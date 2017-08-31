/**
 * @file  eso_bson.h
 * @brief Binary Serialized Object Notation.
 */

#ifndef __ESO_BSON_H__
#define __ESO_BSON_H__

#include "es_types.h"
#include "es_status.h"
#include "eso_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * bson node data type
 */
#define BSON_NODE_DATA_TYPE_STRING   0x00  //string The encoded string(default is utf-8)
#define BSON_NODE_DATA_TYPE_BINARY   0x01  //binary The origin binary data
#define BSON_NODE_DATA_TYPE_I8       0x02  //i8 8-bits signed integer(int8, uint8, bool, byte)
#define BSON_NODE_DATA_TYPE_I16      0x03  //i16 16-bits signed integer(int16, uint16)
#define BSON_NODE_DATA_TYPE_I32      0x04  //i32 32-bits signed integer(int32, uint32, float)
#define BSON_NODE_DATA_TYPE_I64      0x05  //i64 64-bits signed integer(int64, uint64, double)
#define BSON_NODE_DATA_TYPE_FLOAT    0x06  //float  32-bits float
#define BSON_NODE_DATA_TYPE_DOUBLE   0x07  //double 64-bits float
#define BSON_NODE_DATA_TYPE_VARINT   0x08  //varint variant integer(int32, int64, uint32, uint64, sint32, sint64, bool, enum)
//0x09-0x0F: reserved

typedef struct es_bson_node_t es_bson_node_t;
struct es_bson_node_t {
	es_string_t      *name;     //the node key
	es_buffer_t      *value;    //the node value
	es_bson_node_t   *parent;
	es_bson_node_t   *child0;
	es_bson_node_t   *prev;
	es_bson_node_t   *next;
	es_bson_node_t   *attr0;
	es_uint8_t        dtype:4;  //node data type
	es_uint8_t        isattr:1; //1-attribute node, 0-element node
	es_uint8_t        _last_:1; /*private*/
	es_uint8_t        _xxxx_:2; /*private*/
};

typedef struct es_bson_t      es_bson_t;
struct es_bson_t
{
	struct bson_head_t
	{
		int  version;
		char encoding[20];
	} head;
	es_bson_node_t *root;

	es_size_t length; /*stream data length*/
};

/** Notice:
 * 1. path format: "/NAME1|IDX1/NAME2|IDX2/.../NAMEn|IDXn"
 * 2. node index: 1 is the first node in the same name's nodes, and 0 is the last;
 *                if Read operation, the default is 1;
 *                if Write operation, the default is 0.
 */

/**
 * Create a new bson object.
 * @param encoding The string encode type.
 * @return bson object.
 */
es_bson_t* eso_bson_create(const char *encoding);

/**
 * Destroy bson object.
 * @param bson	The bson object.
 */
void eso_bson_destroy(es_bson_t **bson);

/**
 * Clean bson object.
 * @param bson	The bson object.
 */
void eso_bson_clear(es_bson_t *bson);

/**
 * Returns TRUE if this bson contains some node.
 * @param bson	The bson object.
 * @return TRUE or FALSE.
 */
es_bool_t eso_bson_is_empty(es_bson_t *bson);

/**
 * Import from data buffer.
 * @param bson	The bson object.
 * @param buffer	The data buffer.
 * @param size	The data size.
 * @return ES_SUCCESS or ES_FAILURE,ES_BADARG,BSON_EREAD.
 */
es_status_t eso_bson_import(es_bson_t *bson, void *buffer, es_size_t size);
es_status_t eso_bson_import2(es_bson_t *bson, es_istream_t *istream);

/**
 * Export to buffer with node path.
 * @param bson	The bson object.
 * @param buffer	The dest data buffer.
 * @param path	The node path, like "/persion|2/name|0", default node index is 1; if NULL, then export all.
 * @param use_head	If TRUE then export with bson head.
 * @return ES_SUCCESS or ES_BADARG.
 */
es_status_t eso_bson_export(es_bson_t *bson, es_buffer_t *buffer, const char *path, es_bool_t use_head);
es_status_t eso_bson_export2(es_bson_t *bson, es_ostream_t *ostream, const char *path, es_bool_t use_head);

/**
 * Stream parsed to bson object.
 * @param bson	The bson object.
 * @param parser	Parser function.
 * @return ES_SUCCESS or ES_FAILURE,ES_BADARG,BSON_EREAD.
 */
typedef struct
{
	es_int32_t (*read)(void *p, void *buf, es_size_t *size);
	/* if (input(*size) != 0 && output(*size) == 0) means end_of_stream.
	(output(*size) < input(*size)) is allowed 
	success: ES_SUCCESS
	failure: BSON_EREAD
	*/
	
	es_bool_t finished; /* default is false */

	void (*parsed)(void *p, es_bson_t *bson, es_bson_node_t *node);
	
	/* private, don't to change it. */
	es_buffer_t *_cache_;
	es_bool_t _bsondone_;
} es_bson_stream_t;

void eso_bson_parser_init(es_bson_stream_t *parser,
		es_int32_t (*read)(void *p, void *buf, es_size_t *size),
		void (*parsed)(void *p, es_bson_t *bson, es_bson_node_t *node));

es_status_t eso_bson_parse(es_bson_t *bson, es_bson_stream_t *parser);

void eso_bson_parser_uninit(es_bson_stream_t *parser);

/**
 * Clone the bson object.
 * @param bson_dst The dest bson object.
 * @param path_dst The dest node path.
 * @param bson_src The source bson object.
 * @param path_src The source node path.
 * @return ES_SUCCESS or ES_FAILURE,ES_BADARG,BSON_EHEAD,BSON_ENONODE.
 */
es_status_t eso_bson_clone(es_bson_t *bson_dst,
                                      const char *path_dst,
                                      es_bson_t *bson_src,
                                      const char *path_src);

//==============================================================================

/**
 * add bson element
 */
es_bson_node_t* eso_bson_add_bin(es_bson_t *bson,
									  const char *path,
									  const void *data,
									  es_size_t size);

es_bson_node_t* eso_bson_add_str(es_bson_t *bson,
									  const char *path,
									  const char *str);

es_bson_node_t* eso_bson_add_fmt(es_bson_t *bson,
                                      const char *path,
                                      const char *fmt, ...);

es_bson_node_t* eso_bson_add_int8(es_bson_t *bson,
									  const char *path,
									  es_int8_t v);
es_bson_node_t* eso_bson_add_int16(es_bson_t *bson,
									  const char *path,
									  es_int16_t v);
es_bson_node_t* eso_bson_add_int32(es_bson_t *bson,
									  const char *path,
									  es_int32_t v);
es_bson_node_t* eso_bson_add_int64(es_bson_t *bson,
									  const char *path,
									  es_int64_t v);
es_bson_node_t* eso_bson_add_float(es_bson_t *bson,
									  const char *path,
									  float v);
es_bson_node_t* eso_bson_add_double(es_bson_t *bson,
									  const char *path,
									  double v);
es_bson_node_t* eso_bson_add_varint(es_bson_t *bson,
									  const char *path,
									  es_int64_t v);
/**
 * set bson element value
 */
es_bson_node_t* eso_bson_set_bin(es_bson_t *bson,
									  const char *path,
									  const void *data,
									  es_size_t size);

es_bson_node_t* eso_bson_set_str(es_bson_t *bson,
									  const char *path,
									  const char *str);

es_bson_node_t* eso_bson_set_fmt(es_bson_t *bson,
									  const char *path,
									  const char *fmt, ...);

es_bson_node_t* eso_bson_set_int8(es_bson_t *bson,
									  const char *path,
									  es_int8_t v);
es_bson_node_t* eso_bson_set_int16(es_bson_t *bson,
									  const char *path,
									  es_int16_t v);
es_bson_node_t* eso_bson_set_int32(es_bson_t *bson,
									  const char *path,
									  es_int32_t v);
es_bson_node_t* eso_bson_set_int64(es_bson_t *bson,
									  const char *path,
									  es_int64_t v);
es_bson_node_t* eso_bson_set_float(es_bson_t *bson,
									  const char *path,
									  float v);
es_bson_node_t* eso_bson_set_double(es_bson_t *bson,
									  const char *path,
									  double v);
es_bson_node_t* eso_bson_set_varint(es_bson_t *bson,
									  const char *path,
									  es_int64_t v);

/*
 * get bson element value
*/
char* eso_bson_get_str(es_bson_t *bson,
                                  const char *path);

void* eso_bson_get_bin(es_bson_t *bson,
                                  const char *path, 
                                  es_size_t *size,
                                  int *type);

/**
 * delete bson element
 */
es_status_t eso_bson_del(es_bson_t *bson,
                                   const char *path);

//==============================================================================

/**
 * Get a bson node.
 * @param bson The bson handle
 * @param path The node path, if the path is like this '/NAME/' then get the fist node from '/NAME' node
 * @return The found bson node or NULL
 */
es_bson_node_t* eso_bson_node_find(es_bson_t *bson,
									const char *path);

/**
 * Count nodes with the path.
 */
int eso_bson_node_count(es_bson_t *bson, const char *path);

/**
 * Index of a bson node.
 */
int eso_bson_node_index(es_bson_node_t *node);

/**
 * Get the level of a bson node.
 */
int eso_bson_node_level(es_bson_node_t *node);

/**
 * Get the path of a bson node.
 */
char* eso_bson_node_path(es_bson_node_t *node,
                                  char *path_buf, 
                                  int buf_size);

/**
 * Delete bson element
 * @param bson The bson handle
 * @param node The bson node for delete
 */
es_status_t eso_bson_node_del(es_bson_node_t *node);

/**
 * Get child node
 */
es_bson_node_t* eso_bson_node_child_get(es_bson_node_t *node,
										const char *path);

/**
 * Count child node
 */
int eso_bson_node_child_count(es_bson_node_t *node, const char *path);

/**
 * update bson element node
 */
es_bson_node_t* eso_bson_node_update_str(es_bson_node_t *node,
										  const char *key,
										  const char *str);

es_bson_node_t* eso_bson_node_update_fmt(es_bson_node_t *node,
										  const char *key,
										  const char *fmt, ...);

es_bson_node_t* eso_bson_node_update_bin(es_bson_node_t *node,
										  const char *key,
										  const void *data,
										  es_size_t size);

es_bson_node_t* eso_bson_node_update_int8(es_bson_node_t *node,
										  const char *key,
										  es_int8_t v);
es_bson_node_t* eso_bson_node_update_int16(es_bson_node_t *node,
										  const char *key,
										  es_int16_t v);
es_bson_node_t* eso_bson_node_update_int32(es_bson_node_t *node,
										  const char *key,
										  es_int32_t v);
es_bson_node_t* eso_bson_node_update_int64(es_bson_node_t *node,
										  const char *key,
										  es_int64_t v);
es_bson_node_t* eso_bson_node_update_float(es_bson_node_t *node,
                                          const char *key,
                                          float v);
es_bson_node_t* eso_bson_node_update_double(es_bson_node_t *node,
                                          const char *key,
                                          double v);
es_bson_node_t* eso_bson_node_update_varint(es_bson_node_t *node,
                                          const char *key,
                                          es_int64_t v);

//==============================================================================

/**
 * add bson element attribute
 */
es_bson_node_t* eso_bson_attr_add_str(es_bson_node_t *node,
									  const char *key,
									  const char *str);

es_bson_node_t* eso_bson_attr_add_fmt(es_bson_node_t *node,
									  const char *key,
									  const char *fmt, ...);

es_bson_node_t* eso_bson_attr_add_bin(es_bson_node_t *node,
									  const char *key,
									  const void *data,
									  es_size_t size);

es_bson_node_t* eso_bson_attr_add_int8(es_bson_node_t *node,
									  const char *key,
									  es_int8_t v);
es_bson_node_t* eso_bson_attr_add_int16(es_bson_node_t *node,
									  const char *key,
									  es_int16_t v);
es_bson_node_t* eso_bson_attr_add_int32(es_bson_node_t *node,
									  const char *key,
									  es_int32_t v);
es_bson_node_t* eso_bson_attr_add_int64(es_bson_node_t *node,
									  const char *key,
									  es_int64_t v);
es_bson_node_t* eso_bson_attr_add_float(es_bson_node_t *node,
									  const char *key,
									  float v);
es_bson_node_t* eso_bson_attr_add_double(es_bson_node_t *node,
									  const char *key,
									  double v);
es_bson_node_t* eso_bson_attr_add_varint(es_bson_node_t *node,
									  const char *key,
									  es_int64_t v);

/**
 * set bson element attribute
 */
es_bson_node_t* eso_bson_attr_set_str(es_bson_node_t *node,
									const char *key,
									const char *str);

es_bson_node_t* eso_bson_attr_set_fmt(es_bson_node_t *node,
                                            const char *key, 
                                            const char *fmt, ...);

es_bson_node_t* eso_bson_attr_set_bin(es_bson_node_t *node,
									const char *key,
									const void *data,
									es_size_t size);

es_bson_node_t* eso_bson_attr_set_int8(es_bson_node_t *node,
									const char *key,
									es_int8_t v);
es_bson_node_t* eso_bson_attr_set_int16(es_bson_node_t *node,
									const char *key,
									es_int16_t v);
es_bson_node_t* eso_bson_attr_set_int32(es_bson_node_t *node,
									const char *key,
									es_int32_t v);
es_bson_node_t* eso_bson_attr_set_int64(es_bson_node_t *node,
									const char *key,
									es_int64_t v);
es_bson_node_t* eso_bson_attr_set_float(es_bson_node_t *node,
									const char *key,
									float v);
es_bson_node_t* eso_bson_attr_set_double(es_bson_node_t *node,
									const char *key,
									double v);
es_bson_node_t* eso_bson_attr_set_varint(es_bson_node_t *node,
									const char *key,
									es_int64_t v);

/**
 * get bson element attribute
 */
char* eso_bson_attr_get_str(es_bson_node_t *node,
                                       const char *key);

/**
 * get bson element attribute
 */
void* eso_bson_attr_get_bin(es_bson_node_t *node,
                                       const char *key,
                                       es_size_t *size,
                                       int *type);

/**
 * del bson element attribute
 */
es_status_t eso_bson_attr_del(es_bson_node_t *node,
                                        const char *key);

/**
 * clear bson attr data
 */
void eso_bson_attr_clear(es_bson_node_t *node);


#ifdef __cplusplus
}
#endif

#endif /* !__ESO_BSON_H__ */

