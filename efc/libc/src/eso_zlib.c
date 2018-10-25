/**
 * @file  eso_zlib.c
 * @brief ES zlib zip api.
 */

#include "./zlib/zlib.h"

#include "eso_zlib.h"
#include "eso_libc.h"
#include "eso_util.h"

/**
 * @see: zlib-1.2.5.tar.gz::examples/zpipe.c
 */

#define CHUNK 16384

//ZLIB compress
es_int32_t eso_zlib_deflate(es_ostream_t *outStream, 
                            es_istream_t *inStream)
{
	int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

	if (!outStream || !inStream) {
		return ES_Z_ERRNO;
	}

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
    if (ret != ES_Z_OK)
        return ret;

    /* compress until end of file */
    do {
    	strm.avail_in = CHUNK;
        ret = inStream->read(inStream, in, (es_size_t*)&strm.avail_in);
        if (ret != ES_Z_OK) {
            (void)deflateEnd(&strm);
            return ES_Z_ERRNO;
        }
        flush = (strm.avail_in==0) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            ES_ASSERT(ret != ES_Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
            if (outStream->write(outStream, out, have) != have) {
                (void)deflateEnd(&strm);
                return ES_Z_ERRNO;
            }
        } while (strm.avail_out == 0);
        ES_ASSERT(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    ES_ASSERT(ret == ES_Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
    return ES_Z_OK;
}

//ZLIB decompression
es_int32_t eso_zlib_inflate(es_ostream_t *outStream,
                          es_istream_t *inStream)
{
	int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];
	
	if (!outStream || !inStream) {
		return ES_Z_ERRNO;
	}
	
    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != ES_Z_OK)
        return ret;

    /* decompress until deflate stream ends or end of file */
    do {
    	strm.avail_in = CHUNK;
        ret = inStream->read(inStream, in, (es_size_t*)&strm.avail_in);
        if (ret != ES_Z_OK) {
            (void)inflateEnd(&strm);
            return ES_Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            ES_ASSERT(ret != ES_Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
            case ES_Z_NEED_DICT:
                ret = ES_Z_DATA_ERROR;     /* and fall through */
            case ES_Z_DATA_ERROR:
            case ES_Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return ret;
            }
            have = CHUNK - strm.avail_out;
            if (outStream->write(outStream, out, have) != have) {
                (void)inflateEnd(&strm);
                return ES_Z_ERRNO;
            }
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != ES_Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == ES_Z_STREAM_END ? ES_Z_OK : ES_Z_DATA_ERROR;
}

//==============================================================================

//#define ES_ZIP_TEST

#ifdef ES_ZIP_TEST

typedef struct {
	es_istream_t s;
	es_file_t *file;
} CFileInStream;

typedef struct {
	es_ostream_t s;
	es_file_t *file;
} CFileOutStream;

static es_int32_t stream_read_file(void *p, void *buf, es_size_t *size)
{
	CFileInStream *inStream = (CFileInStream*)p;
	return (*size = eso_fread(buf, *size, inStream->file)) >= 0 ? 0 : -1;
}

static es_size_t stream_write_file(void *p, const void *buf, es_size_t size)
{
	CFileOutStream *outStream = (CFileOutStream*)p;
	return eso_fwrite(buf, size, outStream->file);
}


typedef struct {
	es_istream_t s;
	es_byte_t *ptr;
	es_int32_t size;
	es_int32_t pos;
} CMemInStream;

typedef struct {
	es_ostream_t s;
	es_byte_t *ptr;
	es_int32_t size;
	es_int32_t pos;
} CMemOutStream;

static es_int32_t stream_read_mem(void *p, void *buf, es_size_t *size)
{
	CMemInStream *inStream = (CMemInStream*)p;
	
	if (inStream->pos > inStream->size) {
		return -1;
	}
	
	*size = ES_MIN(*size, inStream->size - inStream->pos);
	memcpy(buf, inStream->ptr+inStream->pos, *size);
	inStream->pos += *size;
	
	return 0;
}

static es_size_t stream_write_mem(void *p, const void *buf, es_size_t size)
{
	CMemOutStream *outStream = (CMemOutStream*)p;
	
	if (!outStream->ptr) {
		outStream->ptr = (es_byte_t *)eso_malloc(size);
		if (!outStream->ptr) {
			return -1;
		}
		outStream->size = size;
	}
	else {
		//realloc
		es_byte_t *p = outStream->ptr;
		outStream->ptr = (es_byte_t *)eso_malloc(outStream->size + size);
		memcpy(outStream->ptr, p, outStream->size);
		eso_free(p);
		outStream->size += size;
	}
	memcpy(outStream->ptr + outStream->pos, buf, size);
	outStream->pos += size;
	
	return size;
}

void eso_unzip_test(void)
{
#if 0
	CFileInStream inStream;
	CFileOutStream outStream;
	
	inStream.s.read = stream_read_file;
	outStream.s.write = stream_write_file;

	inStream.file = eso_fopen("d:\\downloads\\tst.z", "rb");
	outStream.file = eso_fopen("d:\\downloads\\tst.u", "wb");
	
	eso_zlib_inflate((es_ostream_t*)&outStream, (es_istream_t*)&inStream);
	
	eso_fclose(outStream.file);
	eso_fclose(inStream.file);
#else
	es_file_t *pfI, *pfO;
	CMemInStream inStream;
	CMemOutStream outStream;
	
	memset(&inStream, 0, sizeof(CMemInStream));
	memset(&outStream, 0, sizeof(CMemOutStream));
	
	inStream.s.read = stream_read_mem;
	outStream.s.write = stream_write_mem;
	
	pfI = eso_fopen("d:\\downloads\\tst.z", "rb");
	inStream.size = eso_fsize(pfI);
	inStream.ptr = (es_byte_t *)eso_malloc(inStream.size);
	eso_fread(inStream.ptr, inStream.size, pfI);
	eso_fclose(pfI);
	
	eso_zlib_inflate((es_ostream_t*)&outStream, (es_istream_t*)&inStream);
	
	pfO = eso_fopen("d:\\downloads\\tst.u", "wb");
	eso_fwrite(outStream.ptr, outStream.size, pfO);
	eso_free(outStream.ptr);
	eso_fclose(pfO);
#endif
}

void eso_zip_test(void)
{
#if 0
	CFileInStream inStream;
	CFileOutStream outStream;
	
	inStream.s.read = stream_read_file;
	outStream.s.write = stream_write_file;

	inStream.file = eso_fopen("d:\\downloads\\tst.u", "rb");
	outStream.file = eso_fopen("d:\\downloads\\tst.z", "wb");
	
	eso_zlib_deflate((es_ostream_t*)&outStream, (es_istream_t*)&inStream);
	
	eso_fclose(outStream.file);
	eso_fclose(inStream.file);
#else
	es_file_t *pfI, *pfO;
	CMemInStream inStream;
	CMemOutStream outStream;
	
	memset(&inStream, 0, sizeof(CMemInStream));
	memset(&outStream, 0, sizeof(CMemOutStream));
	
	inStream.s.read = stream_read_mem;
	outStream.s.write = stream_write_mem;
	
	pfI = eso_fopen("d:\\downloads\\tst.u", "rb");
	inStream.size = eso_fsize(pfI);
	inStream.ptr = (es_byte_t *)eso_malloc(inStream.size);
	eso_fread(inStream.ptr, inStream.size, pfI);
	eso_fclose(pfI);
	
	eso_zlib_deflate((es_ostream_t*)&outStream, (es_istream_t*)&inStream);
	
	pfO = eso_fopen("d:\\downloads\\tst.z", "wb");
	eso_fwrite(outStream.ptr, outStream.size, pfO);
	eso_free(outStream.ptr);
	eso_fclose(pfO);
#endif
}

#endif //!ES_ZIP_TEST
