/**
 * @file  es_mem.c
 * @brief ES memory manager.
 */

#include "es_comm.h"
#include "eso_mem.h"
#include "eso_libc.h"

#define ES_MEM_SIZE_ALIGN(size)   (((size) < 33) ? 32 : (32 + ES_ALIGN_UP((size)-32, 64)))

#define ES_BUF2NODE(buf)          (es_node_t*)((char*)(buf) - sizeof(es_node_t))
#define ES_NODE2BUF(node)         (void*)((char*)(node) + sizeof(es_node_t))

typedef struct es_node_t  es_node_t;
struct es_node_t {
	es_size_t       max_size;  //user malloc aligned size
};

void* eso_mmalloc(es_size_t size)
{
	es_size_t n = ES_MEM_SIZE_ALIGN(size);
	es_node_t *node = (es_node_t*)eso_malloc(sizeof(es_node_t) + n);
	if (node) {
		node->max_size = n;
		return ES_NODE2BUF(node);
	}
	return NULL;
}

void* eso_mcalloc(es_size_t size)
{
	es_size_t n = ES_MEM_SIZE_ALIGN(size);
	es_node_t *node = (es_node_t*)eso_malloc(sizeof(es_node_t) + n);
	if (node) {
		void *ptr = ES_NODE2BUF(node);
		node->max_size = n;
		eso_memset(ptr, 0, n);
		return ptr;
	}
	return NULL;
}

void* eso_mrealloc(void *ptr, es_size_t newsize)
{
	es_node_t *node;
	
	if (!ptr) {
		return NULL;
	}
	
	node = ES_BUF2NODE(ptr);
	if (node->max_size >= newsize) {
		return ptr;
	}
	else {
		void *newptr = eso_mcalloc(newsize);
		if (newptr) {
			eso_memcpy(newptr, ptr, node->max_size);
		}
		eso_mfree(ptr);
		return newptr;
	}
}

void eso_mfree(void *ptr)
{
	es_node_t *node;
	
	if (!ptr) return;
	
	node = ES_BUF2NODE(ptr);
	eso_free(node);
}

void eso_mfree_and_nil(void **ptr)
{
	if (!ptr || !*ptr) {
		return;
	}
	eso_mfree(*ptr);
	*ptr = NULL;
}

es_size_t eso_mnode_size(void *ptr)
{
	es_node_t  *mnode;
	
	if (!ptr) return 0;
	mnode = ES_BUF2NODE(ptr);
	return mnode->max_size;
}
