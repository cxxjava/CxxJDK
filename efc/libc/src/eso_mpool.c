/**
 * @file  eso_mpool.c
 * @brief ES eso memory pool
 */

#include "eso_mpool.h"
#include "eso_libc.h"

#ifdef ES_MEMPOOL_DEBUG
extern void* malloc(es_size_t size);
extern void  free(void *ptr);
static void simple_check_memaddr_overlap(es_mpool_t *mpool, const char *flag);
#endif

//==============================================================================

#define ES_MEM_BLOCK_CONSIZE      24*1024  //每个block内存块为固定值，超过该大小则直接从底层内存池中获取
#define ES_MEM_SIZE_ALIGN(size)   (((size) < 13) ? 12 : (12 + ES_ALIGN_UP((size)-12, 24)))

#define ES_BUF2BLOCK(buf)         (es_mblock_t*)((char*)(buf) - sizeof(es_mblock_t))
#define ES_BUF2NODE(buf)          (es_mnode_t*)((char*)(buf) - sizeof(es_mnode_t))
#define ES_BLOCK2BUF(block)       (void*)((char*)(block) + sizeof(es_mblock_t))
#define ES_NODE2BUF(node)         (void*)((char*)(node) + sizeof(es_mnode_t))

#ifdef ES_MEMPOOL_DEBUG
typedef struct es_mdbg_item_t es_mdbg_item_t;
struct es_mdbg_item_t
{
	void*        m_ptr;
	es_int32_t   m_length;
	es_int8_t    m_file[ES_PATH_MAX];
	es_int32_t   m_line;
	es_mdbg_item_t* m_pNext;
};
#endif //!ES_MEMPOOL_DEBUG

typedef struct es_mblock_t es_mblock_t;
typedef struct es_mnode_t  es_mnode_t;

struct es_mblock_t {
	es_mblock_t *block_next; //point to the next block
	es_size_t    free_size;  //free size for this block
	es_mnode_t  *free_node;  //point to the head free node
};

/*
 * 被拆分后的尺寸总是可以被完整利用，无碎片方案：
 * size: 12+12 12+36 12+60 12+80 ...
 */
struct es_mnode_t {
	union {
		es_mnode_t *node_next;  //if free then point to the next free node
		es_size_t   used_size;  //if used then register the real used size
	} node_ref;
	es_size_t       max_size;   //user malloc aligned size
	es_mpool_t     *mpool;
};

struct es_mpool_t {
	es_mblock_t *first_block;   //point to head block
	es_size_t    total_rsize;   //count of blocks
	es_size_t    total_usize;   //total user malloc size
#ifdef ES_MEMPOOL_DEBUG
	es_mdbg_item_t *debug_malloc_list;
#endif
};

//==============================================================================

static void* alloc_block(es_size_t block_size)
{
	return eso_malloc(block_size);
}

static void free_block(es_mpool_t *mpool, es_mblock_t *block)
{
	es_mblock_t *temp_block;
	
	if (!block) return;
	
	if (mpool->first_block == block) {
		mpool->first_block = mpool->first_block->block_next;
	}
	else {
		temp_block = mpool->first_block;
		while (temp_block->block_next) {
			if (temp_block->block_next == block) {
				temp_block->block_next = block->block_next;
				break;
			}
			temp_block = temp_block->block_next;
		}
	}
	
	eso_free((void*)block);
}

/*
 * 功能：释放节点
 * 注意：1、不重置或修改原节点内容，保证realloc操作时原数据不丢失；
 *       2、加入空闲列表时，节点需按内存地址从小到大排序。
 */
static void free_node(es_mpool_t *mpool, es_mnode_t *mnode)
{
	es_mblock_t *mblock;
	
	if (!mpool || !mnode) return;
	
	mpool->total_usize -= mnode->max_size;
	
	if (mnode->max_size > ES_MEM_BLOCK_CONSIZE) {
		mblock = ES_BUF2BLOCK(mnode);
		mpool->total_rsize -= (mnode->max_size + sizeof(es_mnode_t) + sizeof(es_mblock_t));
		free_block(mpool, mblock);
		return;
	}
	else {
		mblock = mpool->first_block;
		while (mblock) {
			if (((char*)mnode > (char*)mblock)
				&& ((char*)mnode < ((char*)mblock + sizeof(es_mblock_t) + ES_MEM_BLOCK_CONSIZE)))
			{
				goto FREE_NODE_TO_LIST;
			}
			mblock = mblock->block_next;
		}
	}
	
#ifdef ES_MEMPOOL_DEBUG
	//reach here is error!
	eso_log("free mnode fail!(-1)");
#endif
	return;
	
FREE_NODE_TO_LIST:
	
	#ifdef ES_MEMPOOL_DEBUG
	{ //检查是否重复释放
		es_mnode_t *mnode2;
		
		mnode2 = mblock->free_node;
		while (mnode2) {
			if (mnode == mnode2) {
				eso_log("error: repeated free of the same memory node!!! paddr=0x%x", mnode);
				return;
			}
			mnode2 = mnode2->node_ref.node_next;
		}
	}
	#endif
	
	{ //1
		if (!mblock->free_node) {
			mnode->node_ref.node_next = NULL;
			mblock->free_node = mnode;
			mblock->free_size += mnode->max_size;
			return;
		}
		else if (mblock->free_node > mnode) {
			if (((char*)(mnode) + sizeof(es_mnode_t) + mnode->max_size) == (char*)mblock->free_node) {
				mnode->max_size += sizeof(es_mnode_t) + mblock->free_node->max_size;
				mnode->node_ref.node_next = mblock->free_node->node_ref.node_next;
				mblock->free_node = mnode;
				mblock->free_size += mnode->max_size + sizeof(es_mnode_t);
			}
			else {
				mnode->node_ref.node_next = mblock->free_node;
				mblock->free_node = mnode;
				mblock->free_size += mnode->max_size;
			}
			return;
		}
		else { //mblock->free_node < mnode
			register es_mnode_t *node_curr, *node_prev;
			
			node_curr = node_prev = mblock->free_node;
			while (node_curr && node_curr < mnode) {
				if (((char*)(node_curr) + sizeof(es_mnode_t) + node_curr->max_size) == (char*)mnode) {
					node_curr->max_size += sizeof(es_mnode_t) + mnode->max_size;
					mblock->free_size += sizeof(es_mnode_t) + mnode->max_size;
					
					if (node_curr->node_ref.node_next &&
						(((char*)(node_curr) + node_curr->max_size) == (char*)node_curr->node_ref.node_next)) 
					{
						node_curr->max_size += sizeof(es_mnode_t) + node_curr->node_ref.node_next->max_size;
						mblock->free_size += sizeof(es_mnode_t) + node_curr->node_ref.node_next->max_size;
						node_curr->node_ref.node_next = node_curr->node_ref.node_next->node_ref.node_next;
					}
					
					return;
				}
				
				node_prev = node_curr;
				node_curr = node_prev->node_ref.node_next;
			}
			
			mnode->node_ref.node_next = node_prev->node_ref.node_next;
			node_prev->node_ref.node_next = mnode;
			mblock->free_size += mnode->max_size;
			
			return;
		} //!else
	} //!1
#ifdef ES_MEMPOOL_DEBUG
	//reach here is error!
	eso_log("free mnode fail!(-2)");
#endif
	return;
}

/**
 * malloc
 */
static void* __malloc(es_mpool_t *mpool, es_size_t size, es_mfind_e type, const void *pval, es_size_t vlen)
{
	es_mfind_e find_type = type;
	es_size_t   require_size;
	es_size_t   block_size;
	es_size_t   node_size;
	register es_mblock_t *mblock;
	register es_mnode_t  *mnode;
	void *pdata;
	
	if (size == 0) {
		return NULL;
	}
	
	//计算实际申请内存大小
	require_size = ES_MEM_SIZE_ALIGN(size);
	node_size = sizeof(es_mnode_t) + require_size;
	
	if (require_size > ES_MEM_BLOCK_CONSIZE) {
		//如果申请内存大小超出32K，则直接从底层分配内存
		block_size = sizeof(es_mblock_t) + node_size;
		goto NEWBLOCK;
	}
	else {
		//遍历查找最优/最快匹配空闲块
		es_mblock_t *match_block = NULL;
		es_mnode_t  *prev_node, *match_prev_node = NULL, *match_node = NULL;
		es_size_t   find_min_size = 0xFFFFFFFF;
		
		mblock = mpool->first_block;
		while (mblock) {
			if (mblock->free_node && (mblock->free_size >= node_size)) {
				mnode = mblock->free_node;
				prev_node = NULL;
				while (mnode) {
					if (mnode->max_size == require_size) { //最佳匹配
						if (prev_node) {
							prev_node->node_ref.node_next = mnode->node_ref.node_next;
						}
						else {
							mblock->free_node = mnode->node_ref.node_next;
						}
						mnode->node_ref.used_size = size;
						mnode->mpool = mpool;
						mblock->free_size -= node_size;
						mpool->total_usize += mnode->max_size;
						
						//复制原数据
						pdata = ES_NODE2BUF(mnode);
						if (pval) {
							eso_memmove(pdata, pval, ES_MIN(require_size, vlen));
						}
						return pdata;
					}
					else if (mnode->max_size > require_size) {
						if (find_type == ES_MFIND_FAST) {  //最快匹配
							match_block = mblock;
							match_node = mnode;
							match_prev_node = prev_node;
							goto FINDED;
						}
						else {                             //次佳匹配
							if (mnode->max_size < find_min_size) {
								match_block = mblock;
								match_node = mnode;
								match_prev_node = prev_node;
							}
							find_min_size = ES_MIN(mnode->max_size, find_min_size);
						}
					}
					
					prev_node = mnode;
					mnode = mnode->node_ref.node_next;  //!
				} //!while (mnode) {
			}
			
			mblock = mblock->block_next;  //!
		}

FINDED:
		if (match_block && match_node) {
			//空闲池内找到匹配内存块
			es_mnode_t *newnode;
			
			//复制原数据
			pdata = ES_NODE2BUF(match_node);
			if (pval) {
				eso_memmove(pdata, pval, ES_MIN(require_size, vlen));
			}
			
			//拆分新的节点
			newnode = (es_mnode_t*)((char*)match_node + node_size);
			newnode->node_ref.node_next = match_node->node_ref.node_next;
			newnode->max_size = match_node->max_size - require_size - sizeof(es_mnode_t);
			newnode->mpool = mpool;
			
			if (match_prev_node) {
				match_prev_node->node_ref.node_next = newnode;
			}
			else {
				match_block->free_node = newnode;
			}
			
			match_block->free_size -= node_size + sizeof(es_mnode_t);
			match_node->max_size = require_size;
			match_node->node_ref.used_size = size;
			mpool->total_usize += match_node->max_size;
			
			return pdata;
		}
		else {
			//重新申请Block内存块
			block_size = sizeof(es_mblock_t) + ES_MEM_BLOCK_CONSIZE;
			goto NEWBLOCK;
		}
	} //!if (require_size > ES_MEM_BLOCK_CONSIZE)
	
	return NULL;

NEWBLOCK:
	{
		char *paddr;
		
		paddr = (char*)alloc_block(block_size);
		
		mnode = (es_mnode_t*)(paddr + sizeof(es_mblock_t));
		mnode->node_ref.used_size = size;
		mnode->max_size = require_size;
		mnode->mpool = mpool;
		
		mblock = (es_mblock_t*)paddr;
		mblock->block_next = mpool->first_block;
		mblock->free_size = block_size - sizeof(es_mblock_t) - node_size;
		if (mblock->free_size == 0) {
			mblock->free_node = NULL;
		}
		else {
			es_mnode_t *newnode = (es_mnode_t*)((char*)mnode + node_size);
			newnode->node_ref.node_next = NULL;
			mblock->free_size -= sizeof(es_mnode_t);
			newnode->max_size = mblock->free_size;
			mblock->free_node = newnode;
		}
		
		mpool->first_block = mblock;
		mpool->total_usize += mnode->max_size;
		mpool->total_rsize += (mnode->max_size > ES_MEM_BLOCK_CONSIZE) ? 
		                          (sizeof(es_mblock_t) + node_size) :
		                          (sizeof(es_mblock_t) + ES_MEM_BLOCK_CONSIZE);
		
		
		//复制原数据
		pdata = ES_NODE2BUF(mnode);
		if (pval) {
			eso_memmove(pdata, pval, ES_MIN(require_size, vlen));
		}
		return pdata;
	}
	
	return NULL;
}

//==============================================================================

es_mpool_t* eso_mempool_create(void)
{
	es_mpool_t *mpool = eso_malloc(sizeof(es_mpool_t));
	if (mpool) {
		eso_memset(mpool, 0, sizeof(es_mpool_t));
	}
	return mpool;
}

void eso_mempool_free(es_mpool_t **mpool)
{
	es_mblock_t *block;
	
	if (!mpool || !*mpool) return;
	
#ifdef ES_MEMPOOL_DEBUG
	eso_mempool_dump(*mpool, "mpool free");
#endif
	
	block = (*mpool)->first_block;
	while (block) {
		free_block(*mpool, block);
		block = (*mpool)->first_block;
	}
	ESO_FREE(mpool);
}

void eso_mempool_count(es_mpool_t *mpool,
                     es_size_t *user_malloc_size,
                     es_size_t *real_malloc_size)
{
	if (mpool && user_malloc_size) {
		*user_malloc_size = mpool->total_usize;
	}
	if (mpool && real_malloc_size) {
		*real_malloc_size = mpool->total_rsize;
	}
}

void eso_mempool_dump(es_mpool_t *mpool, const char *flag)
{
#ifdef ES_MEMPOOL_DEBUG
	es_int8_t buf[516];
	es_mdbg_item_t *pItem = NULL;
	es_size_t user_malloc_size, real_malloc_size;
	
	eso_log("+++++ memory pool dump begin +++++[%s]", flag ? flag : "");
	
	if (!mpool) return;
	
	eso_mempool_count(mpool, &user_malloc_size, &real_malloc_size);
	eso_log("count: user_malloc_size=%d, real_malloc_size=%d", user_malloc_size, real_malloc_size);
	
	pItem = mpool->debug_malloc_list;
	while(pItem) {
		eso_snprintf(buf, sizeof(buf), "%s:%d\t(ptr=0x%x,length=%d)",pItem->m_file, pItem->m_line, pItem->m_ptr, pItem->m_length);
		eso_log(buf);
		
		pItem = pItem->m_pNext;
	}
	
	eso_log("+++++ memory pool dump end +++++");
#endif
}

void eso_mempool_check(es_mpool_t *mpool, const char *flag)
{
#ifdef ES_MEMPOOL_DEBUG
	simple_check_memaddr_overlap(mpool, flag);
#endif
}

/**
 * realloc
 */
void* eso_mprealloc(void *pold, es_size_t size)
{
	return eso_mprealloc_ext(pold, size, ES_MFIND_SIZE);
}

/**
 * free
 */
void eso_mpfree0(void** ptr)
{
	if (!ptr || !*ptr) return;
	
	eso_mpfree(*ptr);
	*ptr = NULL;
}

/**
 * get memory node max size
 */
es_size_t eso_mpnode_size(void *ptr)
{
	es_mnode_t  *mnode;
	
	if (!ptr) return 0;
	mnode = ES_BUF2NODE(ptr);
	return mnode->max_size;
}

//==============================================================================

#ifdef ES_MEMPOOL_DEBUG

//内存溢出检查
static void simple_check_memaddr_overlap(es_mpool_t *mpool, const char *flag)
{
	register es_mdbg_item_t *pItem= NULL;
	register es_mnode_t  *mnode;
	register es_size_t    require_size;
	
	if (!mpool) return;
	
	pItem = mpool->debug_malloc_list;
	while(pItem) {
		mnode = ES_BUF2NODE(pItem->m_ptr);
		require_size = ES_MEM_SIZE_ALIGN(pItem->m_length);
		
		if (mnode->max_size != require_size) {
			//内存错误
			eso_log("+++++ warning! memory overlap check fail. +++++[%s]", flag ? flag : "");
		}
		
		pItem = pItem->m_pNext;
	}
}

/**
 * malloc
 */
void* eso_mpmalloc_debug(es_mpool_t *mpool, es_size_t size, const es_int8_t *file, es_int32_t line)
{
	return eso_mpmalloc_ext_debug(mpool, size, ES_MFIND_SIZE, file, line);
}

void* eso_mpmalloc_ext_debug(es_mpool_t *mpool, es_size_t size, es_mfind_e type, const es_int8_t *file, es_int32_t line)
{
	es_mdbg_item_t *pItem= NULL;
	void* ptr = NULL;
	
	if (!mpool) return NULL;
	
	ptr = (void*)__malloc(mpool, size, type, NULL, 0);
	if (!ptr) {
		return NULL;
	}
	
	pItem = (es_mdbg_item_t*)malloc(sizeof(es_mdbg_item_t));
	eso_strncpy(pItem->m_file, file, sizeof(pItem->m_file));
	pItem->m_line = line;
	pItem->m_ptr = ptr;
	pItem->m_length = size;
	pItem->m_pNext = mpool->debug_malloc_list;
	mpool->debug_malloc_list = pItem;
	
	//内存溢出检查
	simple_check_memaddr_overlap(mpool, "malloc");
	
	return ptr;
}

/**
 * calloc
 */
void* eso_mpcalloc_debug(es_mpool_t *mpool, es_size_t size, const es_int8_t *file, es_int32_t line)
{
	return eso_mpcalloc_ext_debug(mpool, size, ES_MFIND_SIZE, file, line);
}

void* eso_mpcalloc_ext_debug(es_mpool_t *mpool, es_size_t size, es_mfind_e type, const es_int8_t *file, es_int32_t line)
{
	void *data;
	
	if (!mpool) return NULL;
	
	if ((data = eso_mpmalloc_ext_debug(mpool, size, type, file, line)) != NULL) {
		eso_memset(data, 0, size);
	}
	
	return data;
}

/**
 * realloc
 */
void* eso_mprealloc_ext(void *pold, es_size_t size, es_mfind_e type)
{
	es_mpool_t *mpool;
	es_mnode_t *node_old;
	es_size_t  size_old;
	void *pnew = NULL;
	
	es_mdbg_item_t *pItemPrev = NULL;
	es_mdbg_item_t *pItem;
	es_int8_t    file[ES_PATH_MAX];
	es_int32_t   line;
	
	if (!pold || size==0)
		return NULL;
	
	node_old = ES_BUF2NODE(pold);
	size_old = node_old->max_size;
	
	if (size <= size_old) {
		return pold;
	}
	mpool = node_old->mpool;
	
	//释放原debug信息
	pItem = mpool->debug_malloc_list;
	while(pItem) {
		if(pItem->m_ptr == pold)	{
			if(pItemPrev) {
				pItemPrev->m_pNext = pItem->m_pNext;
				
				eso_strcpy(file, pItem->m_file);
				line = pItem->m_line;
				
				free(pItem);
				break;
			}
			else {
				mpool->debug_malloc_list = pItem->m_pNext;
				
				eso_strcpy(file, pItem->m_file);
				line = pItem->m_line;
				
				free(pItem);
				break;
			}
		}
		pItemPrev = pItem;
		pItem = pItem->m_pNext;
	}
	
	//realloc
	if (size_old > ES_MEM_BLOCK_CONSIZE) {
		pnew = eso_mpmalloc_ext_debug(mpool, size, type, file, line);
		eso_memmove(pnew, pold, size_old);
		free_node(mpool, node_old);
	}
	else {
		es_mdbg_item_t *pItem= NULL;
		
		free_node(mpool, node_old);
		pnew = __malloc(mpool, size, type, pold, size_old);  //申请新内存块同时复制原数据
		
		//同步debug信息
		pItem = (es_mdbg_item_t*)malloc(sizeof(es_mdbg_item_t));
		eso_strncpy(pItem->m_file, file, sizeof(pItem->m_file));
		pItem->m_line = line;
		pItem->m_ptr = pnew;
		pItem->m_length = size;
		pItem->m_pNext = mpool->debug_malloc_list;
		mpool->debug_malloc_list = pItem;
	}
	
	return pnew;
}

/**
 * free
 */
void eso_mpfree(void* ptr)
{
	es_mpool_t *mpool;
	es_mnode_t *mnode;
	es_mdbg_item_t *pItemPrev = NULL;
	es_mdbg_item_t *pItem;
	
	if (!ptr) return;
	
	mnode = ES_BUF2NODE(ptr);
	mpool = mnode->mpool;
	
	//内存溢出检查
	simple_check_memaddr_overlap(mpool, "free");
	
	free_node(mpool, mnode);
	
	pItem = mpool->debug_malloc_list;
	while(pItem) {
		if(pItem->m_ptr == ptr)	{
			if(pItemPrev) {
				pItemPrev->m_pNext = pItem->m_pNext;
				free(pItem);
				break;
			}
			else {
				mpool->debug_malloc_list = pItem->m_pNext;
				free(pItem);
				break;
			}
		}
		pItemPrev = pItem;
		pItem = pItem->m_pNext;
	}
}

#else //ES_MEMPOOL_DEBUG

/**
 * malloc
 */
void* eso_mpmalloc_ext(es_mpool_t *mpool, es_size_t size, es_mfind_e type)
{
	return __malloc(mpool, size, type, NULL, 0);
}

/**
 * malloc
 */
void* eso_mpmalloc(es_mpool_t *mpool, es_size_t size)
{
	return __malloc(mpool, size, ES_MFIND_SIZE, NULL, 0);
}

/**
 * calloc
 */
void* eso_mpcalloc_ext(es_mpool_t *mpool, es_size_t size, es_mfind_e type)
{
	void *paddr = __malloc(mpool, size, type, NULL, 0);
	if (paddr) {
		eso_memset(paddr, 0, size);
	}
	return paddr;
}

/**
 * calloc
 */
void* eso_mpcalloc(es_mpool_t *mpool, es_size_t size)
{
	return eso_mpcalloc_ext(mpool, size, ES_MFIND_SIZE);
}

/**
 * realloc
 */
void* eso_mprealloc_ext(void *pold, es_size_t size, es_mfind_e type)
{
	es_mpool_t *mpool;
	es_mnode_t *node_old;
	es_size_t  size_old;
	void *pnew = NULL;
	
	if (!pold || size==0)
		return NULL;
	
	node_old = ES_BUF2NODE(pold);
	size_old = node_old->max_size;
	mpool = node_old->mpool;
	
	if (size <= size_old) {
		return pold;
	}
	
	if (size_old > ES_MEM_BLOCK_CONSIZE) {
		pnew = __malloc(mpool, size, type, NULL, 0);
		eso_memmove(pnew, pold, size_old);
		free_node(mpool, node_old);
	}
	else {
		free_node(mpool, node_old);
		pnew = __malloc(mpool, size, type, pold, size_old);  //申请新内存块同时复制原数据
	}
	
	return pnew;
}

/**
 * free
 */
void eso_mpfree(void* ptr)
{
	es_mnode_t *mnode;
	es_mpool_t *mpool;
	
	if (!ptr) return;
	
	mnode = ES_BUF2NODE(ptr);
	mpool = mnode->mpool;
	free_node(mpool, mnode);
}

#endif //!ES_MEMPOOL_DEBUG

//==============================================================================
