/**
 * @file  eso_bson.c
 * @brief Binary Serialized Object Notation.
 */

#include "eso_bson.h"
#include "eso_libc.h"
#include "eso_util.h"
#include "eso_buffer.h"
#include "eso_string.h"

//==============================================================================


#define BSON_VERSION                 3
#define BSON_DEFAULT_ENCODING        "UTF-8"

#define BSON_FIELD_DIV               "\0"
#define BSON_MAX_KEY_LEN             255
#define	BSON_NAMEFLAG                '/'
#define BSON_NAMEFLAG2               "/"
#define	BSON_POSFLAG	             '|'
#define BSON_POSFLAG2                "|"

#define BSON_NODE_INDEX_FIRST         1
#define BSON_NODE_INDEX_LAST          0


/**
 * tail flag
 */
#define BSON_TAIL_FLAG "\x0\x0\xF"
#define BSON_TAIL_SIZE 3

/**
 * node type
 */
#define NODE_ELEMENT   0
#define NODE_ATTRIBUTE 1

/**
 * node flag
 */
#define NODE_TYPE_IS_ATTR  0x80
#define NODE_TYPE_IS_SUB   0x40
#define NODE_TYPE_IS_LAST  0x20
#define NODE_TYPE_HAS_ATTR 0x10


#define ES_IS_ROOT_PATH(path) (!path || !*path || (path[0]==BSON_NAMEFLAG && path[1]==0))

//==============================================================================

static void bson_node_free(es_bson_node_t **self)
{
	es_bson_node_t *node;
	es_bson_node_t *child, *sibling;

	if (!self || !*self)
		return;

	node = *self;

	child = node->child0;
	if (child) {
		bson_node_free(&child);
	}

	sibling = node->next;
	if (sibling) {
		bson_node_free(&sibling);
	}

	ESO_MFREE(&node->name);
	eso_buffer_free(&node->value);
	bson_node_free(&node->attr0);
	ESO_FREE(self);
}

static es_size_t bson_reset_head(es_bson_t *bson, const char *bson_buf)
{
	char *p;
	es_size_t head_len = 0;
	char tmp[32];

	p = eso_strstr(bson_buf, "?>");
	if (p && (eso_memcmp(bson_buf, "<?", 2) == 0)) {
		head_len = p - (char *)bson_buf + 3; //sizeof("?>\0") = 3
		//version
		p = eso_strnstr(bson_buf, head_len, "version");
		if (!p)
			bson->head.version = BSON_VERSION;
		else {
			eso_strsplit(p, "\"", 2, tmp, sizeof(tmp));
			bson->head.version = atoi(tmp);
		}
		//encoding
		p = eso_strnstr(bson_buf, head_len, "encoding");
		if (!p)
			eso_strncpy(bson->head.encoding, BSON_DEFAULT_ENCODING, sizeof(bson->head.encoding));
		else {
			eso_strsplit(p, "\"", 2, tmp, sizeof(tmp));
			eso_strncpy(bson->head.encoding, tmp, sizeof(bson->head.encoding));
		}
	}

	return head_len;
}

static es_bson_node_t* bson_node_get_sibling(es_bson_node_t *child0, const char *key, int index)
{
	es_bson_node_t *node_tmp;
	es_bson_node_t *node_last = NULL;
	int count = 0;

	node_tmp = child0;
	while (node_tmp) {
		if (eso_strcmp(node_tmp->name, key) == 0) {
			count++;

			if (index == BSON_NODE_INDEX_LAST) {
				node_last = node_tmp;
			}
			else if (index == count) {
				return node_tmp;
			}
		}

		node_tmp = node_tmp->next;
	}

	return node_last;
}

static es_bson_node_t* bson_node_get_last(es_bson_node_t* node) {
	es_bson_node_t *node_tmp;
	
	node_tmp = node;
	while (node_tmp) {
		if (!node_tmp->next) {
			return node_tmp;
		}
		
		node_tmp = node_tmp->next;
	}
	
	return NULL;
}

static es_bson_node_t *bson_locate_node(es_bson_t *bson,
                                   const char *path,
                                   int idx_default)
{
	int i, index;
	char *pkey = (char *)path;
	char *p;
	char element[BSON_MAX_KEY_LEN + 10];
	es_bson_node_t *child0 = NULL;
	es_bson_node_t *found = NULL;

	if (!bson) return NULL;

	if (ES_IS_ROOT_PATH(path))
		return bson->root;

	if (*pkey == BSON_NAMEFLAG)
		pkey++;

	i = 0;
	child0 = bson->root->child0;
	while (1) {
		eso_strsplit(pkey, BSON_NAMEFLAG2, ++i, element, sizeof(element));
		if (!*element)
			break;

		p = eso_strchr(element, BSON_POSFLAG);
		if (p) {
			*p = 0;
			index = eso_atol(p+1);
		}
		else {
			index = idx_default;
		}

		found = bson_node_get_sibling(child0, element, index);
		//not found!
		if (!found)
			return NULL;
		else
			child0 = found->child0;
	}

	return found;
}

static es_bson_node_t* find_prev_sib_node(es_bson_node_t *prev_node)
{
	if (!prev_node->_last_) {
		return prev_node;
	}
	else {
		es_bson_node_t *node = prev_node->parent;
		if (node && node->parent) {
			es_bson_node_t *node_last = node;
			while (node) {
				node_last = node;
				node = node->next;
			}
			return find_prev_sib_node(node_last);
		}
	}
	return NULL;
}


static es_bool_t bson_array_to_varint(es_uint8_t** pcurr, es_uint8_t* pend, es_uint32_t* value)
{
	es_uint8_t* ptr = *pcurr;
	es_uint32_t b;
	es_uint32_t result;
	int i;
	
	if (pend > ptr) {
		b = *(ptr++); result  = (b & 0x7F)      ; if (!(b & 0x80)) goto done;
		if (pend > ptr) {
			b = *(ptr++); result |= (b & 0x7F) <<  7; if (!(b & 0x80)) goto done;
			if (pend > ptr) {
				b = *(ptr++); result |= (b & 0x7F) << 14; if (!(b & 0x80)) goto done;
				if (pend > ptr) {
					b = *(ptr++); result |= (b & 0x7F) << 21; if (!(b & 0x80)) goto done;
					if (pend > ptr) {
						b = *(ptr++); result |=  b         << 28; if (!(b & 0x80)) goto done;

						// If the input is larger than 32 bits, we still need to read it all
						// and discard the high-order bits.
						for (i = 0; ((i < MAX_VARINT64_BYTES - MAX_VARINT32_BYTES) && (pend > ptr)); i++) {
							b = *(ptr++); if (!(b & 0x80)) goto done;
						}
					}
				}
			}
		}
	}

	return FALSE;
	
done:
	*value = result;
	*pcurr = ptr;
	return TRUE;
}

/**
 * multi-import, slow but safe.
 */
#define ES_NODE_IMP_RET if (last_parent_node) *last_parent_node = parent_node; \
	                    if (last_prev_node) *last_prev_node = prev_node; \
	                    return bson_data;

static const char* bson_node_import(es_bson_t *bson,
                                   es_bson_node_t *parent_node,
                                   es_bson_node_t *prev_node,
                                   const char *bson_data,
                                   es_size_t bson_size,
                                   es_bson_node_t **last_parent_node,
                                   es_bson_node_t **last_prev_node,
                                   es_bson_stream_t *parser)
{
	char *p, *pcurr, *pend, *key_val, *text_val;
	es_uint32_t size_key, size_text;
	es_byte_t node_flag;
	es_bson_node_t *new_node;
	es_bool_t ret = FALSE;

	if (bson_size == 0) {
		if (parser) parser->_bsondone_ = TRUE;
		ES_NODE_IMP_RET;
	}

	//tail check
	if (bson_size >= BSON_TAIL_SIZE
			&& eso_memcmp(bson_data, BSON_TAIL_FLAG, BSON_TAIL_SIZE) == 0) {
		if (parser) parser->_bsondone_ = TRUE;
		return (char*)bson_data + BSON_TAIL_SIZE;
	}

	pend = (char*)bson_data + bson_size;
	pcurr = key_val = (char*)bson_data;
	
	p = eso_memchr(pcurr, '\0', bson_size);
	if (!p) { //no key name
		ES_NODE_IMP_RET;
	}
	pcurr = ++p;
	if (pend - pcurr < 2) { //1 byte node type
		ES_NODE_IMP_RET;
	}
	size_key = eso_strlen(key_val);
	node_flag = *pcurr;
	pcurr++;
	if (pend - pcurr < 1) {
		ES_NODE_IMP_RET;
	}

	//new node
	new_node = (es_bson_node_t*)eso_calloc(sizeof(es_bson_node_t));
	new_node->name = eso_mstrdup(key_val);
	new_node->dtype = node_flag & 0x0F;
	new_node->isattr = (node_flag & NODE_TYPE_IS_ATTR) ? 1 : 0;
	new_node->_last_ = (node_flag & NODE_TYPE_IS_LAST) ? 1 : 0;

	if (new_node->isattr) {
		ret = bson_array_to_varint((es_uint8_t**)&pcurr, (es_uint8_t*)pend, &size_text);
		text_val = pcurr;
		pcurr += size_text;
		if (!ret || pend - pcurr < 1) {
			bson_node_free(&new_node);
			ES_NODE_IMP_RET;
		}
		new_node->value = eso_buffer_make(size_text, 0);
		eso_buffer_append(new_node->value, text_val, size_text);

		if (new_node->_last_) {
			ret = bson_array_to_varint((es_uint8_t**)&pcurr, (es_uint8_t*)pend, &size_text);
			text_val = pcurr;
			pcurr += size_text;
			if (!ret || pend - pcurr < 0) {
				bson_node_free(&new_node);
				ES_NODE_IMP_RET;
			}
			parent_node->value = eso_buffer_make(size_text, 0);
			eso_buffer_append(parent_node->value, text_val, size_text);
		}

		if (!prev_node) {
			parent_node->attr0 = new_node;
			new_node->parent = parent_node;
		}
		else {
			new_node->prev = prev_node;
			prev_node->next = new_node;
			new_node->parent = parent_node;
		}
		
		if (parser) parser->parsed(parser, bson, new_node);
			
		if (new_node->_last_) {
			
			if (parser) parser->parsed(parser, bson, parent_node);
			
			return bson_node_import(bson, parent_node->parent, parent_node, pcurr, pend-pcurr, last_parent_node, last_prev_node, parser);
		}
		else {
			return bson_node_import(bson, parent_node, new_node, pcurr, pend-pcurr, last_parent_node, last_prev_node, parser);
		}
	}
	else {
		es_bool_t has_attr = node_flag & NODE_TYPE_HAS_ATTR;
		if (has_attr) {
			prev_node->next = new_node;
			new_node->prev = prev_node;
			new_node->parent = prev_node->parent;
			
			return bson_node_import(bson, new_node, NULL, pcurr, pend-pcurr, last_parent_node, last_prev_node, parser);
		}
		else {
			ret = bson_array_to_varint((es_uint8_t**)&pcurr, (es_uint8_t*)pend, &size_text);
			text_val = pcurr;
			pcurr += size_text;
			if (!ret || pend - pcurr < 0) {
				bson_node_free(&new_node);
				ES_NODE_IMP_RET;
			}
			new_node->value = eso_buffer_make(size_text, 0);
			eso_buffer_append(new_node->value, text_val, size_text);
			
			if (!prev_node) {
				parent_node->child0 = new_node;
				new_node->parent = parent_node;
				
				if (parser) parser->parsed(parser, bson, new_node);
				
				return bson_node_import(bson, parent_node, new_node, pcurr, pend-pcurr, last_parent_node, last_prev_node, parser);
			}
			else {
				es_bool_t is_sub = node_flag & NODE_TYPE_IS_SUB;
				if (is_sub) {
					prev_node->child0 = new_node;
					new_node->parent = prev_node;
					
					if (parser) parser->parsed(parser, bson, new_node);
					
					return bson_node_import(bson, prev_node, new_node, pcurr, pend-pcurr, last_parent_node, last_prev_node, parser);
				}
				else {
					es_bson_node_t *sib_node = find_prev_sib_node(prev_node);
					if (sib_node) {
						new_node->prev = sib_node;
						sib_node->next = new_node;
						new_node->parent = sib_node->parent;
						
						if (parser) parser->parsed(parser, bson, new_node);
						
						return bson_node_import(bson, sib_node->parent, new_node, pcurr, pend-pcurr, last_parent_node, last_prev_node, parser);
					}
					else {
						bson_node_free(&new_node);
						if (parser) parser->_bsondone_ = TRUE;
						return bson_data;
					}
				}
			}
		}
	}
	//always not reach here!!!
}

static void bson_node_export(es_bson_node_t *node, int node_type, es_buffer_t *buffer)
{
	es_uint8_t array[MAX_VARINT32_BYTES];
	es_uint8_t n;
	es_byte_t node_flag;

	while (node) {
		//node key
		eso_buffer_append(buffer, node->name, eso_strlen(node->name));
		eso_buffer_append(buffer, BSON_FIELD_DIV, 1);

		//node type
		node_flag = 0x00;
		if (node_type == NODE_ATTRIBUTE) {
			node_flag |= NODE_TYPE_IS_ATTR;
		}
		if (!node->prev) {
			node_flag |= NODE_TYPE_IS_SUB;
		}
		if (!node->next) {
			node_flag |= NODE_TYPE_IS_LAST;
		}
		if (node->attr0) {
			node_flag |= NODE_TYPE_HAS_ATTR;
		}
		node_flag |= node->dtype;
		eso_buffer_append(buffer, &node_flag, 1);

		//attribute
		if (node->attr0 && node_type == NODE_ELEMENT) {
			bson_node_export(node->attr0, NODE_ATTRIBUTE, buffer);
		}

		//varint data size
		n = eso_varint32ToArray(node->value->len, array);
		eso_buffer_append(buffer, array, n);

		//node data
		eso_buffer_append(buffer, node->value->data, node->value->len);

		//sub bson
		if (node->child0 && node_type == NODE_ELEMENT) {
			bson_node_export(node->child0, NODE_ELEMENT, buffer);
		}

		//node next
		node = node->next;
	}
}

static es_size_t bson_node_export2(es_bson_node_t *node, int node_type, es_ostream_t *ostream)
{
	es_uint8_t array[MAX_VARINT32_BYTES];
	es_uint8_t n;
	es_byte_t node_flag;
	es_size_t length = 0;

	while (node) {
		//node key
		length += ostream->write(ostream, node->name, eso_strlen(node->name));
		length += ostream->write(ostream, BSON_FIELD_DIV, 1);

		//node type
		node_flag = 0x00;
		if (node_type == NODE_ATTRIBUTE) {
			node_flag |= NODE_TYPE_IS_ATTR;
		}
		if (!node->prev) {
			node_flag |= NODE_TYPE_IS_SUB;
		}
		if (!node->next) {
			node_flag |= NODE_TYPE_IS_LAST;
		}
		if (node->attr0) {
			node_flag |= NODE_TYPE_HAS_ATTR;
		}
		node_flag |= node->dtype;
		length += ostream->write(ostream, &node_flag, 1);

		//attribute
		if (node->attr0 && node_type == NODE_ELEMENT) {
			length += bson_node_export2(node->attr0, NODE_ATTRIBUTE, ostream);
		}

		//varint data size
		n = eso_varint32ToArray(node->value->len, array);
		length += ostream->write(ostream, array, n);

		//node data
		length += ostream->write(ostream, node->value->data, node->value->len);

		//sub bson
		if (node->child0 && node_type == NODE_ELEMENT) {
			length += bson_node_export2(node->child0, NODE_ELEMENT, ostream);
		}

		//node next
		node = node->next;
	}

	return length;
}

static es_status_t bson_node_path(es_bson_node_t *node,
                                  char *path_buf, 
                                  int buf_size)
{
	char key_idx[BSON_MAX_KEY_LEN] = {0};
	int index, m, n;
	es_status_t stat;
	
	if (!node || !node->parent) {
		return ES_SUCCESS;
	}
	
	stat = bson_node_path(node->parent, path_buf, buf_size);
	if (stat != ES_SUCCESS) {
		return stat;
	}
	
	index = eso_bson_node_index(node);
	eso_snprintf(key_idx, sizeof(key_idx), "/%s|%d", node->name, index);
	m = eso_strlen(path_buf);
	n = eso_strlen(key_idx);
	if (buf_size-m <= n) {
		return ES_FAILURE;
	}
	eso_strncpy(path_buf+m, key_idx, buf_size-m);
	
	return ES_SUCCESS;
}

static es_bson_node_t* bson_add_bin(es_bson_t *bson,
                                          const char *path,
                                          const void *data,
                                          es_size_t size,
                                          int type)
{
	int i, node_count, index;
	char *pkey, *pkey_next, *p;
	es_string_t *path_dup = NULL;
	es_bson_node_t *parent_node, *node;
	es_bson_node_t *new_node = NULL;

	if (!path || !*path) {
		errno = BSON_EINVNODENAME;
		return NULL;
	}

	p = (char *)path;
	if (*p == BSON_NAMEFLAG)
		p++;
	path_dup = eso_mstrdup(p);

	node_count = 1;
	p = eso_strchr(path_dup, BSON_NAMEFLAG);
	while (p) {
		node_count++;
		*p++ = 0;
		p = eso_strchr(p, BSON_NAMEFLAG);
	}

	pkey = path_dup;
	parent_node = bson->root;
	for (i = 1; i <= node_count; i++) {
		p = eso_strchr(pkey, BSON_POSFLAG);
		if (p) {
			*p++ = 0;
			index = eso_atol(p);
			pkey_next = p + eso_strlen(p) + 1;
		}
		else {
			index = BSON_NODE_INDEX_LAST;
			pkey_next = pkey + eso_strlen(pkey) + 1;
		}

		node = bson_node_get_sibling(parent_node->child0, pkey, index);
		if (!node || i==node_count) {
			//new node
			new_node = (es_bson_node_t*)eso_calloc(sizeof(es_bson_node_t));
			new_node->name = eso_mstrdup(pkey);
			new_node->dtype = type;
			new_node->value = eso_buffer_make(0, 0);
			if (data && (i==node_count)) {
				eso_buffer_append(new_node->value, data, size);
			}
			new_node->parent = parent_node;
			new_node->child0 = NULL;
			{
				if (!parent_node->child0) {
					parent_node->child0 = new_node;
					new_node->prev = NULL;
					new_node->next = NULL;
				}
				else if (!node || index == BSON_NODE_INDEX_LAST) {
					es_bson_node_t *node2 = bson_node_get_last(parent_node->child0);
					new_node->prev = node2;
					new_node->next = NULL;
					node2->next = new_node;
				}
				else if (!node->prev) {
					parent_node->child0 = new_node;
					new_node->prev = NULL;
					new_node->next = node;
					node->prev = new_node;
				}
				else {
					new_node->prev = node->prev;
					new_node->next = node;
					node->prev->next = new_node;
					node->prev = new_node;
				}
			}

			parent_node = new_node;
		}
		else {
			parent_node = node;
		}

		//next node
		pkey = pkey_next;
	}
	ESO_MFREE(&path_dup);

	return new_node;
}

static es_bson_node_t* bson_set_bin(es_bson_t *bson,
                                          const char *path,
                                          const void *data,
                                          es_size_t size,
                                          int type)
{
	es_bson_node_t *node;

	node = bson_locate_node(bson, path, BSON_NODE_INDEX_FIRST);
	if (!node) {
		node = bson_add_bin(bson, path, data, size, type);
	}
	else {
		eso_buffer_update(node->value, 0, node->value->len, data, size);
		node->dtype = type;
	}
	return node;
}

static es_bson_node_t* bson_node_update_bin(es_bson_node_t *node,
                                                  const char *key,
                                                  const void *data,
                                                  es_size_t size,
                                                  int type)
{
	if (!node) return NULL;

	if (key) {
		eso_mstrcpy(&node->name, key);
	}
	else {
		eso_mmemfill(node->name, '\0'); //?
	}
	eso_buffer_update(node->value, 0, node->value->len, data, size);

	node->dtype = type;

	return node;
}

static es_bson_node_t* bson_attr_add_bin(es_bson_node_t *node,
                                            const char *key,
                                            const void *data,
                                            es_size_t size,
                                            int type)
{
	es_bson_node_t *new_node;

	if (!node) {
		errno = BSON_EINVNODENAME;
		return NULL;
	}

	new_node = (es_bson_node_t*)eso_calloc(sizeof(es_bson_node_t));
	eso_mstrcpy(&new_node->name, key);
	new_node->dtype = type;
	new_node->isattr = 1;
	new_node->value = eso_buffer_make(0, 0);
	if (data) {
		eso_buffer_append(new_node->value, data, size);
	}
	new_node->parent = node;

	if (node->attr0) {
		es_bson_node_t *node_last = bson_node_get_last(node->attr0);
		node_last->next = new_node;
		new_node->prev = node_last;
	}
	else {
		node->attr0 = new_node;
	}

	return new_node;
}

static es_bson_node_t* bson_attr_set_bin(es_bson_node_t *node,
                                            const char *key,
                                            const void *data,
                                            es_size_t size,
                                            int type)
{
	es_bson_node_t *attr_node;

	attr_node = bson_node_get_sibling(node->attr0, key, BSON_NODE_INDEX_FIRST);
	if (!attr_node) {
		attr_node = bson_attr_add_bin(node, key, data, size, type);
	}
	else {
		eso_buffer_update(attr_node->value, 0, attr_node->value->len, data, size);
		attr_node->dtype = type;
	}

	return attr_node;
}

//==============================================================================

es_bson_t* eso_bson_create(const char *encoding)
{
	es_bson_t  *new_bson;

	new_bson = (es_bson_t *)eso_calloc(sizeof(es_bson_t));
	if (!new_bson)
		return NULL;

	new_bson->head.version = BSON_VERSION;
	eso_strncpy(new_bson->head.encoding,
	           (encoding ? encoding : BSON_DEFAULT_ENCODING),
	           	sizeof(new_bson->head.encoding));

	new_bson->root = (es_bson_node_t*)eso_calloc(sizeof(es_bson_node_t));

	return new_bson;
}

void eso_bson_destroy(es_bson_t **bson)
{
	if (!bson || !*bson)
		return;

	bson_node_free(&(*bson)->root);
	ESO_FREE(bson);
}

void eso_bson_clear(es_bson_t *bson)
{
	if (bson) {
		bson_node_free(&bson->root);
		bson->root = (es_bson_node_t*)eso_calloc(sizeof(es_bson_node_t));
		bson->length = 0;
	}
}

es_bool_t eso_bson_is_empty(es_bson_t *bson)
{
	return !(bson && bson->root && bson->root->child0);
}

es_status_t eso_bson_import(es_bson_t *bson, void *buffer, es_size_t size)
{
	es_size_t head_len, body_len;
	const char *pend;

	if (!bson) return ES_BADARG;

	eso_bson_clear(bson);

	bson->length = size;

	head_len = bson_reset_head(bson, (char*)buffer);
	if (size <= head_len) {
		return ES_SUCCESS;
	}
	body_len = size - head_len;

	pend = bson_node_import(bson, bson->root, NULL,
	                       (char *)buffer + head_len, body_len, NULL, NULL, NULL);
	if (pend == (char*)buffer + size) {
		return ES_SUCCESS;
	}
	else {
		return BSON_EREAD;
	}
}

es_status_t eso_bson_import2(es_bson_t *bson, es_istream_t *istream)
{
	es_status_t stat;
	es_int32_t ret = 0;
	es_size_t size;
	es_buffer_t *buffer;
	char buf[4096];

	buffer = eso_buffer_make(0, 0);

	while (ret == 0) {
		size = sizeof(buf);
		ret = istream->read(istream, buf, &size);
		if (size == 0) {
			break; // end of stream.
		}
		if (ret != 0) {
			eso_buffer_free(&buffer);
			return BSON_EREAD;
		}
		eso_buffer_append(buffer, buf, size);
	}

	stat = eso_bson_import(bson, buffer->data, buffer->len);

	eso_buffer_free(&buffer);

	return stat;
}

es_status_t eso_bson_export(es_bson_t *bson, es_buffer_t *buffer, const char *path, es_bool_t use_head)
{
	es_string_t *head = NULL;
	es_bson_node_t *node;
	es_size_t length = 0;

	if (!bson) return ES_BADARG;

	length = buffer->len;

	if (use_head) {
		eso_msprintf(&head, "<?bson version=\"%d\" encoding=\"%s\"?>",
				bson->head.version, bson->head.encoding);
		eso_buffer_append(buffer, head, eso_strlen(head));
		eso_buffer_append(buffer, BSON_FIELD_DIV, 1);
		ESO_MFREE(&head);
	}

	node = bson_locate_node(bson, path, BSON_NODE_INDEX_FIRST);
	if (node) {
		if (node->name) {
			es_uint8_t array[MAX_VARINT32_BYTES];
			es_uint8_t n;
			es_byte_t node_flag;
			
			//node key
			eso_buffer_append(buffer, node->name, eso_strlen(node->name));
			eso_buffer_append(buffer, BSON_FIELD_DIV, 1);
			
			//node type
			node_flag = 0x00;
			node_flag |= NODE_TYPE_IS_LAST;
			if (node->attr0) {
				node_flag |= NODE_TYPE_HAS_ATTR;
			}
			node_flag |= node->dtype;
			eso_buffer_append(buffer, &node_flag, 1);
    	
			//attribute
			if (node->attr0) {
				bson_node_export(node->attr0, NODE_ATTRIBUTE, buffer);
			}
    	
			//varint data size
			n = eso_varint32ToArray(node->value->len, array);
			eso_buffer_append(buffer, array, n);
    	
			//node data
			eso_buffer_append(buffer, node->value->data, node->value->len);
		}
		
		node = node->child0;
		bson_node_export(node, NODE_ELEMENT, buffer);
	}

	//add tail
	eso_buffer_append(buffer, BSON_TAIL_FLAG, BSON_TAIL_SIZE);

	//set length
	bson->length = buffer->len - length;

	return ES_SUCCESS;
}

es_status_t eso_bson_export2(es_bson_t *bson, es_ostream_t *ostream, const char *path, es_bool_t use_head)
{
	es_string_t *head = NULL;
	es_bson_node_t *node;
	es_size_t length = 0;

	if (!bson) return ES_BADARG;

	if (use_head) {
		eso_msprintf(&head, "<?bson version=\"%d\" encoding=\"%s\"?>",
				bson->head.version, bson->head.encoding);
		length += ostream->write(ostream, head, eso_strlen(head));
		length += ostream->write(ostream, BSON_FIELD_DIV, 1);
		ESO_MFREE(&head);
	}

	node = bson_locate_node(bson, path, BSON_NODE_INDEX_FIRST);
	if (node) {
		if (node->name) {
			es_uint8_t array[MAX_VARINT32_BYTES];
			es_uint8_t n;
			es_byte_t node_flag;

			//node key
			length += ostream->write(ostream, node->name, eso_strlen(node->name));
			length += ostream->write(ostream, BSON_FIELD_DIV, 1);

			//node type
			node_flag = 0x00;
			node_flag |= NODE_TYPE_IS_LAST;
			if (node->attr0) {
				node_flag |= NODE_TYPE_HAS_ATTR;
			}
			node_flag |= node->dtype;
			length += ostream->write(ostream, &node_flag, 1);

			//attribute
			if (node->attr0) {
				length += bson_node_export2(node->attr0, NODE_ATTRIBUTE, ostream);
			}

			//varint data size
			n = eso_varint32ToArray(node->value->len, array);
			length += ostream->write(ostream, array, n);

			//node data
			length += ostream->write(ostream, node->value->data, node->value->len);
		}

		node = node->child0;
		length += bson_node_export2(node, NODE_ELEMENT, ostream);
	}

	//add tail
	length += ostream->write(ostream, BSON_TAIL_FLAG, BSON_TAIL_SIZE);

	//set length
	bson->length = length;

	return ES_SUCCESS;
}

void eso_bson_parser_init(es_bson_stream_t *parser,
		es_int32_t (*read)(void *p, void *buf, es_size_t *size),
		void (*parsed)(void *p, es_bson_t *bson, es_bson_node_t *node)) {
	parser->read = read;
	parser->finished = FALSE;
	parser->parsed = parsed;
	parser->_cache_ = eso_buffer_make(0, 0);
}

void eso_bson_parser_uninit(es_bson_stream_t *parser) {
	eso_buffer_free(&parser->_cache_);
}

es_status_t eso_bson_parse(es_bson_t *bson, es_bson_stream_t *parser)
{
	char buffer[4096];
	es_bson_node_t *parent_node;
	es_bson_node_t *prev_node;
	es_size_t bufsize;
	es_int32_t value_size;
	es_int32_t done_size;
	char *value_data;
	const char* pcurr;
	es_bool_t head_need_end_flag = FALSE;

	if (!bson || !parser || !parser->read) {
		return ES_BADARG;
	}
	
	parent_node = bson->root;
	prev_node = NULL;
	
	while (bufsize = sizeof(buffer), parser->read(parser, buffer, &bufsize) == ES_SUCCESS) {
		if (bufsize == 0) { //end_of_stream
			if (parser) parser->finished = TRUE;
			break;
		}
		
		eso_buffer_append(parser->_cache_, buffer, bufsize);
		value_data = parser->_cache_->data;
		value_size = parser->_cache_->len;
		
		if (value_size < 2) {
			continue;
		}
		
		if (eso_strncmp(value_data, "<?", 2) == 0) {
			head_need_end_flag = TRUE;
		}
		
		if (head_need_end_flag) {
			es_size_t head_len;
			char *p;
			
			p = eso_strstr(value_data, "?>");
			if (!p) {
				continue;
			}
			if (value_data + value_size < p+3) {
				continue;
			}
			
			head_len = bson_reset_head(bson, value_data);
			eso_buffer_delete(parser->_cache_, 0, head_len);
			bson->length += head_len;
			if (parser) parser->parsed(parser, bson, NULL);
			
			break;
		}
	}
	
	if (parser->_cache_->len > 0) {
		goto PARSE;
	}
	
	while (bufsize = sizeof(buffer), parser->read(parser, buffer, &bufsize) == ES_SUCCESS) {
		if (bufsize == 0) { //end_of_stream
			if (parser) parser->finished = TRUE;
			return ES_SUCCESS;
		}
		
		eso_buffer_append(parser->_cache_, buffer, bufsize);
		
PARSE:
		value_data = parser->_cache_->data;
		value_size = parser->_cache_->len;
		parser->_bsondone_ = FALSE;
		
		pcurr = bson_node_import(bson, parent_node, prev_node, value_data, value_size, &parent_node, &prev_node, parser);
		done_size = pcurr - value_data;
		eso_buffer_delete(parser->_cache_, 0, done_size);
		bson->length += done_size;
		if (parser->_bsondone_) {
			//get one full bson!
			return ES_SUCCESS;
		}
	}

	return ES_FAILURE;
}

es_status_t eso_bson_clone(es_bson_t *bson_dst,
                                      const char *path_dst,
                                      es_bson_t *bson_src,
                                      const char *path_src)
{
	es_status_t stat;
	es_buffer_t *buffer = NULL;
	es_bson_node_t *node_dst;
	
	if (!bson_dst || !bson_src) {
		return ES_BADARG;
	}
	
	if (eso_strcasecmp(bson_dst->head.encoding, bson_src->head.encoding) != 0) {
		return BSON_EHEAD;
	}
	
	node_dst = eso_bson_node_find(bson_dst, path_dst);
	if (!node_dst && !ES_IS_ROOT_PATH(path_dst)) {
		return BSON_ENONODE;
	}
	
	buffer = eso_buffer_make(0, 0);
	stat = eso_bson_export(bson_src, buffer, path_src, FALSE);
	if (stat != ES_SUCCESS) {
		goto free_all_return;
	}
	
	if (!node_dst && ES_IS_ROOT_PATH(path_dst)) {
		eso_bson_clear(bson_dst);
		stat = eso_bson_import(bson_dst, buffer->data, buffer->len);
		goto free_all_return;
	}
	else {
		es_bson_node_t *node_clone;
		es_bson_t *bson_tmp = eso_bson_create(bson_dst->head.encoding);
		stat = eso_bson_import(bson_tmp, buffer->data, buffer->len);
		if (stat != ES_SUCCESS) {
			goto free_all_return;
		}
		node_clone = bson_tmp->root->child0;
		bson_tmp->root->child0 = NULL;
		eso_bson_destroy(&bson_tmp);
		
		if (!node_dst->child0) {
			node_dst->child0 = node_clone;
		}
		else {
			es_bson_node_t *node_last = bson_node_get_last(node_dst->child0);
			node_last->next = node_clone;
			node_clone->prev = node_last;
		}
		
		while (node_clone) {
			node_clone->parent = node_dst;
			node_clone = node_clone->next;
		}
	}
	
free_all_return:
	eso_buffer_free(&buffer);
	return stat;
}

//==============================================================================

/**
 * add bson element
 */
es_bson_node_t* eso_bson_add_bin(es_bson_t *bson,
                                          const char *path,
                                          const void *data,
                                          es_size_t size)
{
	return bson_add_bin(bson, path, data, size, BSON_NODE_DATA_TYPE_BINARY);
}

es_bson_node_t* eso_bson_add_str(es_bson_t *bson,
                                          const char *path,
                                          const char *str)
{
	return bson_add_bin(bson, path, str, (str ? eso_strlen(str) + 1 : 0), BSON_NODE_DATA_TYPE_STRING);
}

/**
 * add bson element
 */
es_bson_node_t* eso_bson_add_fmt(es_bson_t *bson,
                                       const char *path,
                                       const char *fmt, ...)
{
	va_list args;
	es_string_t *value = NULL;
	es_bson_node_t *node;

	va_start(args, fmt);
	eso_mvsprintf(&value, fmt, args);
	va_end(args);

	node = eso_bson_add_str(bson, path, value);
	ESO_MFREE(&value);

	return node;
}

es_bson_node_t* eso_bson_add_int8(es_bson_t *bson,
                                          const char *path,
                                          es_int8_t v)
{
	return bson_add_bin(bson, path, &v, 1, BSON_NODE_DATA_TYPE_I8);
}

es_bson_node_t* eso_bson_add_int16(es_bson_t *bson,
                                          const char *path,
                                          es_int16_t v)
{
	es_byte_t s[2];
	int l = eso_llong2array(v, s, 2);
	return bson_add_bin(bson, path, s, l, BSON_NODE_DATA_TYPE_I16);
}

es_bson_node_t* eso_bson_add_int32(es_bson_t *bson,
                                          const char *path,
                                          es_int32_t v)
{
	es_byte_t s[4];
	int l = eso_llong2array(v, s, 4);
	return bson_add_bin(bson, path, s, l, BSON_NODE_DATA_TYPE_I32);
}

es_bson_node_t* eso_bson_add_int64(es_bson_t *bson,
                                          const char *path,
                                          es_int64_t v)
{
	es_byte_t s[8];
	int l = eso_llong2array(v, s, 8);
	return bson_add_bin(bson, path, s, l, BSON_NODE_DATA_TYPE_I64);
}

es_bson_node_t* eso_bson_add_float(es_bson_t *bson,
                                          const char *path,
                                          float v)
{
	es_byte_t s[4];
	es_int32_t i = eso_float2intBits(v);
	int l = eso_llong2array(i, s, 4);
	return bson_add_bin(bson, path, s, l, BSON_NODE_DATA_TYPE_FLOAT);
}

es_bson_node_t* eso_bson_add_double(es_bson_t *bson,
                                          const char *path,
                                          double v)
{
	es_byte_t s[8];
	es_int64_t i = eso_double2llongBits(v);
	int l = eso_llong2array(i, s, 8);
	return bson_add_bin(bson, path, s, l, BSON_NODE_DATA_TYPE_DOUBLE);
}

es_bson_node_t* eso_bson_add_varint(es_bson_t *bson,
                                          const char *path,
                                          es_int64_t v)
{
	es_uint8_t s[32];
	es_uint8_t l = eso_varint64ToArray(v, s);
	return bson_add_bin(bson, path, s, l, BSON_NODE_DATA_TYPE_VARINT);
}

/**
 * set bson element value
 */
es_bson_node_t* eso_bson_set_bin(es_bson_t *bson,
                                          const char *path,
                                          const void *data,
                                          es_size_t size)
{
	return bson_set_bin(bson, path, data, size, BSON_NODE_DATA_TYPE_BINARY);
}

es_bson_node_t* eso_bson_set_str(es_bson_t *bson,
                                          const char *path,
                                          const char *str)
{
	return bson_set_bin(bson, path, str, (str ? eso_strlen(str)+1 : 0), BSON_NODE_DATA_TYPE_STRING);
}

es_bson_node_t* eso_bson_set_fmt(es_bson_t *bson,
                                          const char *path,
                                          const char *fmt, ...)
{
	va_list args;
	es_string_t *value = NULL;
	es_bson_node_t *node;
	
	va_start(args, fmt);
	eso_mvsprintf(&value, fmt, args);
	va_end(args);
	
	node = eso_bson_set_str(bson, path, value);
	ESO_MFREE(&value);
	
	return node;
}

es_bson_node_t* eso_bson_set_int8(es_bson_t *bson,
                                          const char *path,
                                          es_int8_t v)
{
	return bson_set_bin(bson, path, &v, 1, BSON_NODE_DATA_TYPE_I8);
}

es_bson_node_t* eso_bson_set_int16(es_bson_t *bson,
                                          const char *path,
                                          es_int16_t v)
{
	es_byte_t s[2];
	int l = eso_llong2array(v, s, 2);
	return bson_set_bin(bson, path, s, l, BSON_NODE_DATA_TYPE_I16);
}

es_bson_node_t* eso_bson_set_int32(es_bson_t *bson,
                                          const char *path,
                                          es_int32_t v)
{
	es_byte_t s[4];
	int l = eso_llong2array(v, s, 4);
	return bson_set_bin(bson, path, s, l, BSON_NODE_DATA_TYPE_I32);
}

es_bson_node_t* eso_bson_set_int64(es_bson_t *bson,
                                          const char *path,
                                          es_int64_t v)
{
	es_byte_t s[8];
	int l = eso_llong2array(v, s, 8);
	return bson_set_bin(bson, path, s, l, BSON_NODE_DATA_TYPE_I64);
}

es_bson_node_t* eso_bson_set_float(es_bson_t *bson,
                                          const char *path,
                                          float v)
{
	es_byte_t s[4];
	es_int32_t i = eso_float2intBits(v);
	int l = eso_llong2array(i, s, 4);
	return bson_set_bin(bson, path, s, l, BSON_NODE_DATA_TYPE_FLOAT);
}

es_bson_node_t* eso_bson_set_double(es_bson_t *bson,
                                          const char *path,
                                          double v)
{
	es_byte_t s[8];
	es_int64_t i = eso_double2llongBits(v);
	int l = eso_llong2array(i, s, 8);
	return bson_set_bin(bson, path, s, l, BSON_NODE_DATA_TYPE_DOUBLE);
}

es_bson_node_t* eso_bson_set_varint(es_bson_t *bson,
                                          const char *path,
                                          es_int64_t v)
{
	es_uint8_t s[32];
	es_uint8_t l = eso_varint64ToArray(v, s);
	return bson_set_bin(bson, path, s, l, BSON_NODE_DATA_TYPE_VARINT);
}

/*
 * get bson element value
*/
char* eso_bson_get_str(es_bson_t *bson,
                                  const char *path)
{
	return (char *)eso_bson_get_bin(bson, path, NULL, NULL);
}

/*
 * get bson element value
*/
void* eso_bson_get_bin(es_bson_t *bson,
                                  const char *path, 
                                  es_size_t *size,
                                  int *type)
{
	es_bson_node_t *node;
	
	node = bson_locate_node(bson, path, BSON_NODE_INDEX_FIRST);
	if (!node) {
		if (size) *size = 0;
		return NULL;
	}
	else {
		if (size) *size = node->value->len;
		if (type) *type = node->dtype;
		return node->value->data;
	}
}

/**
 * delete bson element
 */
es_status_t eso_bson_del(es_bson_t *bson,
                                   const char *path)
{
	es_bson_node_t *node;
	
	node = bson_locate_node(bson, path, BSON_NODE_INDEX_FIRST);
	if (!node) {
		return BSON_EINVNODENAME;
	}
	
	return eso_bson_node_del(node);
}

/**
 * Get a bson node.
 */
es_bson_node_t* eso_bson_node_find(es_bson_t *bson,
                                            const char *path)
{
	return (es_bson_node_t*)bson_locate_node(bson, path, BSON_NODE_INDEX_FIRST);
}

int eso_bson_node_count(es_bson_t *bson, const char *path)
{
	int count = 0;
	es_bson_node_t* first = bson_locate_node(bson, path, BSON_NODE_INDEX_FIRST);
	if (first) {
		const char* name = first->name;
		es_bson_node_t* node = first->next;
		count = 1;
		while (node) {
			if (eso_strcmp(name, node->name) == 0)
				count++;
			node = node->next;
		}
	}
	return count;
}

/**
 * Index of a bson node.
 */
int eso_bson_node_index(es_bson_node_t *node)
{
	es_bson_node_t *node_tmp;
	int index = 1;
	
	if (node) {
		node_tmp = node->prev;
		while (node_tmp) {
			if (eso_strcmp(node_tmp->name, node->name) == 0) {
				index++;
			} 
				
			node_tmp = node_tmp->prev;
		}
		
		return index;
	}
	
	return -1;
}

/**
 * Get the level of a bson node.
 */
int eso_bson_node_level(es_bson_node_t *node)
{
	int level = 0;

	es_bson_node_t *parent = node->parent;
	while (parent) {
		parent = parent->parent;
		level++;
	}

	return level;
}

/**
 * Get the path of a bson node.
 */
char* eso_bson_node_path(es_bson_node_t *node,
                                  char *path_buf, 
                                  int buf_size)
{
	es_status_t stat;
	
	eso_memset(path_buf, 0, buf_size);
	stat = bson_node_path(node, path_buf, buf_size);
	
	return (stat == ES_SUCCESS) ? path_buf : NULL;
}

/**
 * Delete bson element
 */
es_status_t eso_bson_node_del(es_bson_node_t *node)
{
	if (!node) {
		return ES_BADARG;
	}
	
	if (!node->prev) {
		if (!node->parent) {
			return BSON_ESYSTEM;
		}
		else {
			node->parent->child0 = node->next;
		}
	}
	else {
		node->prev->next = node->next;
	}
	if (node->next) {
		node->next->prev = node->prev;
	}
	
	node->prev = node->next = node->parent = NULL;
	bson_node_free(&node);
	
	return ES_SUCCESS;
}

/**
 * Get child node
 */
es_bson_node_t* eso_bson_node_child_get(es_bson_node_t *node,
										const char *path) {
	int i, index;
	char *pkey = (char *)path;
	char *p;
	char element[BSON_MAX_KEY_LEN + 10];
	es_bson_node_t *child0 = NULL;
	es_bson_node_t *found = NULL;

	if (!node) return NULL;

	if (ES_IS_ROOT_PATH(path))
		return node;

	if (*pkey == BSON_NAMEFLAG)
		pkey++;

	i = 0;
	child0 = node->child0;
	while (1) {
		eso_strsplit(pkey, BSON_NAMEFLAG2, ++i, element, sizeof(element));
		if (!*element)
			break;

		p = eso_strchr(element, BSON_POSFLAG);
		if (p) {
			*p = 0;
			index = eso_atol(p+1);
		}
		else {
			index = BSON_NODE_INDEX_FIRST;
		}

		found = bson_node_get_sibling(child0, element, index);
		//not found!
		if (!found)
			return NULL;
		else
			child0 = found->child0;
	}

	return found;
}

/**
 * Count child node
 */
int eso_bson_node_child_count(es_bson_node_t *node, const char *path)
{
	int count = 0;
	char *name;

	node = eso_bson_node_child_get(node, path);
	if (!node) {
		return 0;
	}
	name = node->name;

	while (node) {
		if (eso_strcmp(name, node->name) == 0)
			count++;

		node = node->next;
	}

	return count;
}

/**
 * update bson element node
 */
es_bson_node_t* eso_bson_node_update_str(es_bson_node_t *node,
                                                  const char *key,
                                                  const char *str)
{
	return bson_node_update_bin(node, key, str, (str ? eso_strlen(str)+1 : 0), BSON_NODE_DATA_TYPE_STRING);
}

es_bson_node_t* eso_bson_node_update_fmt(es_bson_node_t *node,
                                                  const char *key,
                                                  const char *fmt, ...)
{
	va_list args;
	es_string_t *value = NULL;
	
	va_start(args, fmt);
	eso_mvsprintf(&value, fmt, args);
	va_end(args);
	
	node = eso_bson_node_update_str(node, key, value);
	ESO_MFREE(&value);
	
	return node;
}

es_bson_node_t* eso_bson_node_update_bin(es_bson_node_t *node,
                                                  const char *key,
                                                  const void *data,
                                                  es_size_t size)
{
	return bson_node_update_bin(node, key, data, size, BSON_NODE_DATA_TYPE_BINARY);
}

es_bson_node_t* eso_bson_node_update_int8(es_bson_node_t *node,
										  const char *key,
										  es_int8_t v)
{
	return bson_node_update_bin(node, key, &v, 1, BSON_NODE_DATA_TYPE_I8);
}

es_bson_node_t* eso_bson_node_update_int16(es_bson_node_t *node,
										  const char *key,
										  es_int16_t v)
{
	es_byte_t s[2];
	int l = eso_llong2array(v, s, 2);
	return bson_node_update_bin(node, key, s, l, BSON_NODE_DATA_TYPE_I16);
}

es_bson_node_t* eso_bson_node_update_int32(es_bson_node_t *node,
										  const char *key,
										  es_int32_t v)
{
	es_byte_t s[4];
	int l = eso_llong2array(v, s, 4);
	return bson_node_update_bin(node, key, s, l, BSON_NODE_DATA_TYPE_I32);
}

es_bson_node_t* eso_bson_node_update_int64(es_bson_node_t *node,
										  const char *key,
										  es_int64_t v)
{
	es_byte_t s[8];
	int l = eso_llong2array(v, s, 8);
	return bson_node_update_bin(node, key, s, l, BSON_NODE_DATA_TYPE_I64);
}

es_bson_node_t* eso_bson_node_update_float(es_bson_node_t *node,
                                          const char *key,
                                          float v)
{
	es_byte_t s[4];
	es_int32_t i = eso_float2intBits(v);
	int l = eso_llong2array(i, s, 4);
	return bson_node_update_bin(node, key, s, l, BSON_NODE_DATA_TYPE_FLOAT);
}

es_bson_node_t* eso_bson_node_update_double(es_bson_node_t *node,
                                          const char *key,
                                          double v)
{
	es_byte_t s[8];
	es_int64_t i = eso_double2llongBits(v);
	int l = eso_llong2array(i, s, 8);
	return bson_node_update_bin(node, key, s, l, BSON_NODE_DATA_TYPE_DOUBLE);
}

es_bson_node_t* eso_bson_node_update_varint(es_bson_node_t *node,
                                          const char *key,
                                          es_int64_t v)
{
	es_uint8_t s[32];
	es_uint8_t l = eso_varint64ToArray(v, s);
	return bson_node_update_bin(node, key, s, l, BSON_NODE_DATA_TYPE_VARINT);
}

/**
 * add bson element attribute
 */
es_bson_node_t* eso_bson_attr_add_str(es_bson_node_t *node,
                                            const char *key, 
                                            const char *str)
{
	return bson_attr_add_bin(node, key, str, (str ? eso_strlen(str)+1 : 0), BSON_NODE_DATA_TYPE_STRING);
}

es_bson_node_t* eso_bson_attr_add_fmt(es_bson_node_t *node,
                                            const char *key, 
                                            const char *fmt, ...)
{
	va_list args;
	es_string_t *value = NULL;
	es_bson_node_t *new_node;
	
	va_start(args, fmt);
	eso_mvsprintf(&value, fmt, args);
	va_end(args);
	
	new_node = eso_bson_attr_add_str(node, key, value);
	ESO_MFREE(&value);
	
	return new_node;
}

es_bson_node_t* eso_bson_attr_add_bin(es_bson_node_t *node,
                                            const char *key,
                                            const void *data,
                                            es_size_t size)
{
	return bson_attr_add_bin(node, key, data, size, BSON_NODE_DATA_TYPE_BINARY);
}

es_bson_node_t* eso_bson_attr_add_int8(es_bson_node_t *node,
									  const char *key,
									  es_int8_t v)
{
	return bson_attr_add_bin(node, key, &v, 1, BSON_NODE_DATA_TYPE_I8);
}

es_bson_node_t* eso_bson_attr_add_int16(es_bson_node_t *node,
									  const char *key,
									  es_int16_t v)
{
	es_byte_t s[2];
	int l = eso_llong2array(v, s, 2);
	return bson_attr_add_bin(node, key, s, l, BSON_NODE_DATA_TYPE_I16);
}

es_bson_node_t* eso_bson_attr_add_int32(es_bson_node_t *node,
									  const char *key,
									  es_int32_t v)
{
	es_byte_t s[4];
	int l = eso_llong2array(v, s, 4);
	return bson_attr_add_bin(node, key, s, l, BSON_NODE_DATA_TYPE_I32);
}

es_bson_node_t* eso_bson_attr_add_int64(es_bson_node_t *node,
									  const char *key,
									  es_int64_t v)
{
	es_byte_t s[8];
	int l = eso_llong2array(v, s, 8);
	return bson_attr_add_bin(node, key, s, l, BSON_NODE_DATA_TYPE_I64);
}

es_bson_node_t* eso_bson_attr_add_float(es_bson_node_t *node,
									  const char *key,
									  float v)
{
	es_byte_t s[4];
	es_int32_t i = eso_float2intBits(v);
	int l = eso_llong2array(i, s, 4);
	return bson_attr_add_bin(node, key, s, l, BSON_NODE_DATA_TYPE_FLOAT);
}

es_bson_node_t* eso_bson_attr_add_double(es_bson_node_t *node,
									  const char *key,
									  double v)
{
	es_byte_t s[8];
	es_int64_t i = eso_double2llongBits(v);
	int l = eso_llong2array(i, s, 8);
	return bson_attr_add_bin(node, key, s, l, BSON_NODE_DATA_TYPE_DOUBLE);
}

es_bson_node_t* eso_bson_attr_add_varint(es_bson_node_t *node,
									  const char *key,
									  es_int64_t v)
{
	es_uint8_t s[32];
	es_uint8_t l = eso_varint64ToArray(v, s);
	return bson_attr_add_bin(node, key, s, l, BSON_NODE_DATA_TYPE_VARINT);
}

/**
 * set bson element attribute
 */
es_bson_node_t* eso_bson_attr_set_str(es_bson_node_t *node,
                                            const char *key, 
                                            const char *str)
{
	return bson_attr_set_bin(node, key, str, (str ? eso_strlen(str)+1 : 0), BSON_NODE_DATA_TYPE_STRING);
}

es_bson_node_t* eso_bson_attr_set_fmt(es_bson_node_t *node,
                                            const char *key, 
                                            const char *fmt, ...)
{
	va_list args;
	es_string_t *value = NULL;
	es_bson_node_t *attr_node;
	
	va_start(args, fmt);
	eso_mvsprintf(&value, fmt, args);
	va_end(args);
	
	attr_node = eso_bson_attr_set_str(node, key, value);
	ESO_MFREE(&value);
	
	return attr_node;
}

es_bson_node_t* eso_bson_attr_set_bin(es_bson_node_t *node,
									const char *key,
									const void *data,
									es_size_t size)
{
	return bson_attr_set_bin(node, key, data, size, BSON_NODE_DATA_TYPE_BINARY);
}

es_bson_node_t* eso_bson_attr_set_int8(es_bson_node_t *node,
									const char *key,
									es_int8_t v)
{
	return bson_attr_set_bin(node, key, &v, 1, BSON_NODE_DATA_TYPE_I8);
}

es_bson_node_t* eso_bson_attr_set_int16(es_bson_node_t *node,
									const char *key,
									es_int16_t v)
{
	es_byte_t s[2];
	int l = eso_llong2array(v, s, 2);
	return bson_attr_set_bin(node, key, s, l, BSON_NODE_DATA_TYPE_I16);
}

es_bson_node_t* eso_bson_attr_set_int32(es_bson_node_t *node,
									const char *key,
									es_int32_t v)
{
	es_byte_t s[4];
	int l = eso_llong2array(v, s, 4);
	return bson_attr_set_bin(node, key, s, l, BSON_NODE_DATA_TYPE_I32);
}

es_bson_node_t* eso_bson_attr_set_int64(es_bson_node_t *node,
									const char *key,
									es_int64_t v)
{
	es_byte_t s[8];
	int l = eso_llong2array(v, s, 8);
	return bson_attr_set_bin(node, key, s, l, BSON_NODE_DATA_TYPE_I64);
}

es_bson_node_t* eso_bson_attr_set_float(es_bson_node_t *node,
									const char *key,
									float v)
{
	es_byte_t s[4];
	es_int32_t i = eso_float2intBits(v);
	int l = eso_llong2array(i, s, 4);
	return bson_attr_set_bin(node, key, s, l, BSON_NODE_DATA_TYPE_FLOAT);
}

es_bson_node_t* eso_bson_attr_set_double(es_bson_node_t *node,
									const char *key,
									double v)
{
	es_byte_t s[8];
	es_int64_t i = eso_double2llongBits(v);
	int l = eso_llong2array(i, s, 8);
	return bson_attr_set_bin(node, key, s, l, BSON_NODE_DATA_TYPE_DOUBLE);
}

es_bson_node_t* eso_bson_attr_set_varint(es_bson_node_t *node,
									const char *key,
									es_int64_t v)
{
	es_uint8_t s[32];
	es_uint8_t l = eso_varint64ToArray(v, s);
	return bson_attr_set_bin(node, key, s, l, BSON_NODE_DATA_TYPE_VARINT);
}

/**
 * get bson element attribute
 */
char* eso_bson_attr_get_str(es_bson_node_t *node,
                                       const char *key)
{
	return (char *)eso_bson_attr_get_bin(node, key, NULL, NULL);
}

/**
 * get bson element attribute
 */
void* eso_bson_attr_get_bin(es_bson_node_t *node,
                                       const char *key,
                                       es_size_t *size,
                                       int *type)
{
	es_bson_node_t *attr_node;
	
	attr_node = bson_node_get_sibling(node->attr0, key, BSON_NODE_INDEX_FIRST);
	if (!attr_node) {
		if (size) *size = 0;
		return NULL;
	}
	else {
		if (size) *size = attr_node->value->len;
		if (type) *type = attr_node->dtype;
		return attr_node->value->data;
	}
}

/**
 * del bson element attribute
 */
es_status_t eso_bson_attr_del(es_bson_node_t *node,
                                        const char *key)
{
	es_bson_node_t *attr_node;
	
	if (!node || !key || !*key) {
		return ES_BADARG;
	}
	
	attr_node = bson_node_get_sibling(node->attr0, key, BSON_NODE_INDEX_FIRST);
	if (!attr_node) {
		return BSON_ENOATTR;
	}
	
	if (!attr_node->prev) {
		node->attr0 = attr_node->next;
	}
	else {
		attr_node->prev->next = attr_node->next;
	}
	if (node->next) {
		node->next->prev = attr_node->prev;
	}
	
	attr_node->prev = attr_node->next = attr_node->parent = NULL;
	bson_node_free(&attr_node);
	
	return ES_SUCCESS;
}

/**
 * clear bson attr data
 */
void eso_bson_attr_clear(es_bson_node_t *node)
{
	if (node && node->attr0) {
		bson_node_free(&node->attr0);
	}
}

//==============================================================================

#if 0
/**
 * sample for eso_bson_parse()
 */
typedef struct {
	es_bson_stream_t s;
	es_file_t *file;
} CFileInStream;

static es_int32_t stream_read_file(void *s, void *buf, es_size_t *size)
{
	CFileInStream *inStream = (CFileInStream*)s;
	es_size_t ret = eso_fread(buf, *size, inStream->file);
	*size = ret;
	return ret > 0 ?  ES_SUCCESS : BSON_ERROR_READ;
}

static void stream_parsed_node(void *p, es_bson_t *bson, es_bson_node_t *node)
{
	printf("name=[%s]\n", node->name);
}

int main(int argc, const char **argv)
{
	es_bson_t *bson;
	CFileInStream inStream;
	es_status_t stat;
	
	bson = eso_bson_create(BSON_DEFAULT_ENCODING);
	
	eso_bson_parser_init(&inStream.s, stream_read_file, stream_parsed_node);

	inStream.file = eso_fopen(argv[1], "rb");
	if (!inStream.file) {
		return -1;
	}
	stat = eso_bson_parse(bson, (es_bson_stream_t*)&inStream);
	eso_fclose(inStream.file);
	
	eso_bson_destroy(&bson);
	
	return 0;
}
#endif
