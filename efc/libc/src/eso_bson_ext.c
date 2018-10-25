/**
 * @file  eso_bson_ext.c
 * @brief Binary Serialized Object Notation.
 */

#include "eso_bson_ext.h"
#include "eso_libc.h"
#include "eso_buffer.h"
#include "eso_file.h"

es_status_t eso_bson_load(es_bson_t *bson, const char *fname)
{
	es_status_t stat;
	es_file_t *bson_file = NULL;
	es_buffer_t *buffer = NULL;
	es_size_t fsize;
		
	//clear old.
	eso_bson_clear(bson);
	
	bson_file = eso_fopen(fname, "rb");
	if (!bson_file) {
		return ES_IOERROR;
	}
	fsize = eso_fsize(bson_file);
	
	buffer = eso_buffer_make(fsize, 0);
	if (eso_fread(buffer->data, fsize, bson_file) != fsize) {
		stat = ES_IOERROR;
		goto DONE;
	}
	buffer->len = fsize;
	
	stat = eso_bson_import(bson, buffer->data, buffer->len);

DONE:
	eso_buffer_free(&buffer);
	eso_fclose(bson_file);
	
	return stat;
}

es_status_t eso_bson_save(es_bson_t *bson,
                               const char *fname,
                               const char *node)
{
	es_status_t stat;
	es_file_t *bson_file = NULL;
	es_buffer_t *buffer = NULL;
	es_size_t written;
	
	buffer = eso_buffer_make(0, 0);
	stat = eso_bson_export(bson, buffer, node, TRUE);
	if (stat != ES_SUCCESS) {
		eso_buffer_free(&buffer);
		return stat;
	}
	
	bson_file = eso_fopen(fname, "wb+");
	if (!bson_file) {
		eso_buffer_free(&buffer);
		return ES_IOERROR;
	}
	written = eso_fwrite(buffer->data, buffer->len, bson_file);
	if (written != buffer->len) {
		eso_fclose(bson_file);
		eso_buffer_free(&buffer);
		return ES_IOERROR;
	}
	eso_fclose(bson_file);
	eso_buffer_free(&buffer);
	
	return ES_SUCCESS;
}

