/**
 * @file  eso_buffer.h
 * @brief ES buffer string functions
 */

#ifndef __ESO_BUFFER_H__
#define __ESO_BUFFER_H__

#include "es_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct es_buffer_t es_buffer_t;
struct es_buffer_t
{
	void       *data;
	es_size_t  len;
	
	es_size_t  capacity;
	es_size_t  reserved;
};

/*
 * create one buffer
*/
es_buffer_t* eso_buffer_make(es_size_t size, es_size_t reserved);

/*
 * zero the sized buffer's data with offset
 */
es_buffer_t* eso_buffer_zero(es_buffer_t *buffer,
		                     es_size_t off, es_size_t size);

/*
 * append one data to buffer
*/
es_buffer_t* eso_buffer_append(es_buffer_t *buffer,
                              const void *data, es_size_t size);
/*
 * insert data to buffer
*/
es_buffer_t* eso_buffer_insert(es_buffer_t *buffer, es_int32_t pos, 
                              const void *data, es_size_t size);

/*
 * delete data buffer
*/
es_buffer_t* eso_buffer_delete(es_buffer_t *buffer,
                              es_int32_t pos, es_size_t size);

/*
 * update buffer's data
*/
es_buffer_t* eso_buffer_update(es_buffer_t *buffer, 
                              es_int32_t pos, es_size_t old_size,
                              const void *data, es_size_t new_size);

/*
 * clear buffer
*/
es_buffer_t* eso_buffer_clear(es_buffer_t *buffer);

/*
 * set new capacity
*/
void eso_buffer_resize(es_buffer_t *buffer, es_size_t newsize);

/*
 * destroy one buffer
*/
void eso_buffer_free(es_buffer_t **buffer);


#ifdef __cplusplus
}
#endif

#endif  /* ! __ESO_BUFFER_H__ */
