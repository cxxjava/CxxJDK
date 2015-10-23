/**
 * @file  eso_bon.h
 * @brief Binary Object Notation.
 */

#ifndef __ESO_BON_H__
#define __ESO_BON_H__

#include "es_types.h"
#include "es_status.h"
#include "eso_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct es_bon_node_t es_bon_node_t;
struct es_bon_node_t {
	es_string_t      *name;     //the node key
	es_buffer_t      *value;    //the node value
	es_bon_node_t    *parent;
	es_bon_node_t    *child0;
	es_bon_node_t    *prev;
	es_bon_node_t    *next;
	es_bon_node_t    *attr0;
	es_uint8_t        dtype:4;   /* node data type:
			                      0x00£ºstring The encoded string(default is utf-8)
    		                      0x01£ºbinary The origin binary data
    		                      0x02£ºi8 8-bits signed integer(int8, uint8, bool, byte)
    		                      0x03£ºi16 16-bits signed integer(int16, uint16)
    		                      0x04£ºi32 32-bits signed integer(int32, uint32, float)
    		                      0x05£ºi64 64-bits signed integer(int64, uint64, double)
    		                      0x06£ºfloat  32-bits float
    		                      0x07£ºdouble 64-bits float
    		                      0x08£ºvarint variant integer(int32, int64, uint32, uint64, sint32, sint64, bool, enum)
    		                      */
	es_uint8_t        isattr:1; //1-attribute node, 0-element node
	es_uint8_t        _last_:1; /*private*/
	es_uint8_t        _xxxx_:2; /*private*/
};

typedef struct es_bon_t      es_bon_t;
struct es_bon_t
{
	struct bon_head_t
	{
		int  version;
		char encoding[20];
	} head;
	es_bon_node_t *root;

	es_size_t length; /*stream data length*/
};

/** Notice:
 * 1. path format: "/NAME1|IDX1/NAME2|IDX2/.../NAMEn|IDXn"
 * 2. node index: 1 is the first node in the same name's nodes, and 0 is the last;
 *                if Read operation, the default is 1;
 *                if Write operation, the default is 0.
 */

/**
 * Create a new bon object.
 * @param encoding The string encode type.
 * @return bon object.
 */
es_bon_t* eso_bon_create(const char *encoding);

/**
 * Destroy bon object.
 * @param bon	The bon object.
 */
void eso_bon_destroy(es_bon_t **bon);

/**
 * Clean bon object.
 * @param bon	The bon object.
 */
void eso_bon_clear(es_bon_t *bon);

/**
 * Returns TRUE if this bon contains some node.
 * @param bon	The bon object.
 * @return TRUE or FALSE.
 */
es_bool_t eso_bon_is_empty(es_bon_t *bon);

/**
 * Import from data buffer.
 * @param bon	The bon object.
 * @param buffer	The data buffer.
 * @param size	The data size.
 * @return ES_SUCCESS or ES_FAILURE,ES_BADARG,BON_EREAD.
 */
es_status_t eso_bon_import(es_bon_t *bon, void *buffer, es_size_t size);

/**
 * Export to buffer with node path.
 * @param bon	The bon object.
 * @param buffer	The dest data buffer.
 * @param path	The node path, like "/persion|2/name|0", default node index is 1; if NULL, then export all.
 * @param use_head	If TRUE then export with bon head.
 * @return ES_SUCCESS or ES_BADARG.
 */
es_status_t eso_bon_export(es_bon_t *bon, es_buffer_t *buffer, const char *path, es_bool_t use_head);

/**
 * Stream parsed to bon object.
 * @param bon	The bon object.
 * @param parser	Parser function.
 * @return ES_SUCCESS or ES_FAILURE,ES_BADARG,BON_EREAD.
 */
typedef struct
{
	es_int32_t (*read)(void *p, void *buf, es_size_t *size);
	/* if (input(*size) != 0 && output(*size) == 0) means end_of_stream.
	(output(*size) < input(*size)) is allowed 
	success: ES_SUCCESS
	failure: BON_EREAD
	*/
	
	es_bool_t finished; /* default is false */

	void (*parsed)(void *p, es_bon_t *bon, es_bon_node_t *node);
	
	/* private, don't to change it. */
	es_buffer_t *_cache_;
	es_bool_t _bondone_;
} es_bon_stream_t;

void eso_bon_parser_init(es_bon_stream_t *parser,
		es_int32_t (*read)(void *p, void *buf, es_size_t *size),
		void (*parsed)(void *p, es_bon_t *bon, es_bon_node_t *node));

es_status_t eso_bon_parse(es_bon_t *bon, es_bon_stream_t *parser);

void eso_bon_parser_uninit(es_bon_stream_t *parser);

/**
 * Clone the bon object.
 * @param bon_dst The dest bon object.
 * @param path_dst The dest node path.
 * @param bon_src The source bon object.
 * @param path_src The source node path.
 * @return ES_SUCCESS or ES_FAILURE,ES_BADARG,BON_EHEAD,BON_ENONODE.
 */
es_status_t eso_bon_clone(es_bon_t *bon_dst,
                                      const char *path_dst,
                                      es_bon_t *bon_src,
                                      const char *path_src);

//==============================================================================

/**
 * add bon element
 */
es_bon_node_t* eso_bon_add_str(es_bon_t *bon,
                                          const char *path, 
                                          const char *str);

/**
 * add bon element
 */
es_bon_node_t* eso_bon_add_bin(es_bon_t *bon,
                                          const char *path,
                                          const void *data,
                                          es_size_t size);

/**
 * add bon element
 */
es_bon_node_t* eso_bon_add_fmt(es_bon_t *bon,
                                       const char *path,
                                       const char *fmt, ...);

/**
 * set bon element value
 */
es_bon_node_t* eso_bon_set_str(es_bon_t *bon,
                                          const char *path,
                                          const char *str);

/**
 * set bon element value
 */
es_bon_node_t* eso_bon_set_fmt(es_bon_t *bon,
                                          const char *path,
                                          const char *fmt, ...);

/**
 * set bon element value
 */
es_bon_node_t* eso_bon_set_bin(es_bon_t *bon, 
                                          const char *path,
                                          const void *data,
                                          es_size_t size);

/*
 * get bon element value
*/
char* eso_bon_get_str(es_bon_t *bon,
                                  const char *path);

/*
 * get bon element value
*/
void* eso_bon_get_bin(es_bon_t *bon,
                                  const char *path, 
                                  es_size_t *size);

/**
 * delete bon element
 */
es_status_t eso_bon_del(es_bon_t *bon, 
                                   const char *path);

//==============================================================================

/**
 * Get a bon node.
 * @param bon The bon handle
 * @param path The node path, if the path is like this '/NAME/' then get the fist node from '/NAME' node
 * @return The found bon node or NULL
 */
es_bon_node_t* eso_bon_node_find(es_bon_t *bon, 
                                            const char *path);

/**
 * Count nodes with the path.
 */
int eso_bon_node_count(es_bon_t *bon, const char *path);

/**
 * Index of a bon node.
 */
int eso_bon_node_index(es_bon_node_t *node);

/**
 * Get the level of a bon node.
 */
int eso_bon_node_level(es_bon_node_t *node);

/**
 * Get the path of a bon node.
 */
char* eso_bon_node_path(es_bon_node_t *node,
                                  char *path_buf, 
                                  int buf_size);

/**
 * Delete bon element
 * @param bon The bon handle
 * @param node The bon node for delete
 */
es_status_t eso_bon_node_del(es_bon_node_t *node);

/**
 * update bon element node
 */
es_bon_node_t* eso_bon_node_update_str(es_bon_node_t *node,
                                                  const char *key,
                                                  const char *str);

/**
 * update bon element node
 */
es_bon_node_t* eso_bon_node_update_fmt(es_bon_node_t *node,
                                                  const char *key,
                                                  const char *fmt, ...);

/**
 * update bon element node
 */
es_bon_node_t* eso_bon_node_update_bin(es_bon_node_t *node,
                                                  const char *key,
                                                  const void *data,
                                                  es_size_t size);

//==============================================================================

/**
 * add bon element attribute
 */
es_bon_node_t* eso_bon_attr_add_str(es_bon_node_t *node,
                                            const char *key, 
                                            const char *str);

/**
 * add bon element attribute
 */
es_bon_node_t* eso_bon_attr_add_fmt(es_bon_node_t *node,
                                            const char *key, 
                                            const char *fmt, ...);

/**
 * add bon element attribute
 */
es_bon_node_t* eso_bon_attr_add_bin(es_bon_node_t *node,
                                            const char *key,
                                            const void *data,
                                            es_size_t size);

/**
 * set bon element attribute
 */
es_bon_node_t* eso_bon_attr_set_str(es_bon_node_t *node,
                                            const char *key, 
                                            const char *str);

/**
 * set bon element attribute
 */
es_bon_node_t* eso_bon_attr_set_fmt(es_bon_node_t *node,
                                            const char *key, 
                                            const char *fmt, ...);

/**
 * set bon element attribute
 */
es_bon_node_t* eso_bon_attr_set_bin(es_bon_node_t *node,
                                            const char *key,
                                            const void *data,
                                            es_size_t size);

/**
 * get bon element attribute
 */
char* eso_bon_attr_get_str(es_bon_node_t *node,
                                       const char *key);

/**
 * get bon element attribute
 */
void* eso_bon_attr_get_bin(es_bon_node_t *node,
                                       const char *key,
                                       es_size_t *size);

/**
 * del bon element attribute
 */
es_status_t eso_bon_attr_del(es_bon_node_t *node, 
                                        const char *key);

/**
 * clear bon attr data
 */
void eso_bon_attr_clear(es_bon_node_t *node);


#ifdef __cplusplus
}
#endif

#endif /* !__ESO_BON_H__ */

