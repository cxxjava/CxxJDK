/**
 * @file  eso_lzma.c
 * @brief ES lzma zip api.
 */

#include "./lzma/LzmaDec.h"
#include "./lzma/LzmaEnc.h"
#include "./lzma/LzFind.h"

#include "eso_lzma.h"
#include "eso_file.h"
#include "eso_libc.h"
#include "eso_buffer.h"
#include "eso_util.h"

//==============================================================================
// lzmz
//==============================================================================

#define IN_BUF_SIZE (8192)   //uncompress input buffsize
#define OUT_BUF_SIZE (8192)  //uncompress output buffsize

static void *SzAlloc(void *p, size_t size) { ES_UNUSED(p); return eso_malloc(size);}
static void SzFree(void *p, void *address) { ES_UNUSED(p); eso_free(address);}
static ISzAlloc g_Alloc = {0, 0};

static es_int32_t Decode2(CLzmaDec *state, es_ostream_t *outStream, es_istream_t *inStream,
					es_uint32_t unpackSize, ISzAlloc *alloc)
{
	es_uint8_t *inBuf = NULL;
	es_uint8_t *outBuf = NULL;
	es_size_t inPos = 0, inSize = 0;
	es_int32_t res;
	
	inBuf = (es_uint8_t*)alloc->Alloc(alloc, IN_BUF_SIZE);
	outBuf = (es_uint8_t*)alloc->Alloc(alloc, OUT_BUF_SIZE);
	
	if (!inBuf || !outBuf) {
		alloc->Free(alloc, inBuf);
		alloc->Free(alloc, outBuf);
		return ES_SZ_ERROR_MEM;
	}
	
	LzmaDec_Init(state);
	for (;;) {
		if (inPos == inSize) {
			inSize = IN_BUF_SIZE;
			RINOK(inStream->read(inStream, inBuf, &inSize));
			inPos = 0;
		}
		{
			SizeT inProcessed = inSize - inPos;
			SizeT outProcessed = OUT_BUF_SIZE;
			ELzmaFinishMode finishMode = LZMA_FINISH_ANY;
			ELzmaStatus status;
			if (outProcessed > unpackSize) {
				outProcessed = (es_size_t)unpackSize;
				finishMode = LZMA_FINISH_END;
			}

			res = LzmaDec_DecodeToBuf(state, outBuf, &outProcessed,
									  inBuf + inPos, &inProcessed, finishMode, &status);
			inPos += inProcessed;
			unpackSize -= outProcessed;

			if (outStream) {
				es_size_t writeByte;
				writeByte = outStream->write(outStream, outBuf, outProcessed);
				if (writeByte != outProcessed) {
					res = ES_SZ_ERROR_WRITE;
					goto FINISHED;
				}
			}

			if (res != ES_SZ_OK || unpackSize == 0) {
				goto FINISHED;
			}
			
			if (inProcessed == 0 && outProcessed == 0) {
				if (status != LZMA_STATUS_FINISHED_WITH_MARK) {
					res = ES_SZ_ERROR_DATA;
					goto FINISHED;
				}
				goto FINISHED;
			}
		}
	}
	
	return ES_SZ_ERROR_DATA;
	
FINISHED:
	alloc->Free(alloc, inBuf);
	alloc->Free(alloc, outBuf);
	return res;
}

static void LzmaDec_AllocateInit(ISzAlloc *alloc)
{
	alloc->Alloc = SzAlloc;
	alloc->Free  = SzFree;
}

//==============================================================================

es_int32_t eso_lzma_zip(es_ostream_t *outStream,
                        es_istream_t *inStream,
                        es_size_t dataSize)
{
  CLzmaEncHandle enc;
  SRes res;
  CLzmaEncProps props;

  LzmaDec_AllocateInit(&g_Alloc);
  enc = LzmaEnc_Create(&g_Alloc);
  if (enc == 0)
    return ES_SZ_ERROR_MEM;

  LzmaEncProps_Init(&props);
  res = LzmaEnc_SetProps(enc, &props);

  if (res == ES_SZ_OK)
  {
    Byte header[LZMA_PROPS_SIZE + 8];
    size_t headerSize = LZMA_PROPS_SIZE;
    int i;

    res = LzmaEnc_WriteProperties(enc, header, &headerSize);
    for (i = 0; i < 8; i++)
      header[headerSize++] = (Byte)(dataSize >> (8 * i));
    if (outStream->write(outStream, header, headerSize) != headerSize)
      res = ES_SZ_ERROR_WRITE;
    else
    {
      if (res == ES_SZ_OK)
        res = LzmaEnc_Encode(enc, (ISeqOutStream*)outStream, (ISeqInStream*)inStream, NULL, &g_Alloc, &g_Alloc);
    }
  }
  LzmaEnc_Destroy(enc, &g_Alloc, &g_Alloc);
  return res;
}

//==============================================================================

es_int32_t eso_lzma_unzip(es_ostream_t *outStream,
                          es_istream_t *inStream)
{
	es_uint32_t unpackSize;
	int i;
	es_int32_t res = 0;
	es_size_t inSize;

	CLzmaDec state;

	/* header: 5 bytes of LZMA properties and 8 bytes of uncompressed size */
	unsigned char header[LZMA_PROPS_SIZE + 8];

	/* read and parse header */
	inSize = sizeof(header);
	RINOK(inStream->read(inStream, header, &inSize));
	if (inSize != sizeof(header))
		return ES_SZ_ERROR_DATA;

	unpackSize = 0;
	for (i = 0; i < 8; i++)
		unpackSize += (es_uint32_t)header[LZMA_PROPS_SIZE + i] << (i * 8);

	LzmaDec_Construct(&state);
	LzmaDec_AllocateInit(&g_Alloc);
	RINOK(LzmaDec_Allocate(&state, header, LZMA_PROPS_SIZE, &g_Alloc));
	res = Decode2(&state, outStream, inStream, unpackSize, &g_Alloc);
	LzmaDec_Free(&state, &g_Alloc);
	
	return res;
}

//==============================================================================

typedef struct {
	es_istream_t s;
	es_file_t *file;
} CFileInStream;

typedef struct {
	es_istream_t s;
	const char *buff;
	int   size;
	char *ppos;
} CBuffInStream;

typedef struct {
	es_ostream_t s;
	const char *root; //Extract target root path
	es_bool_t overwrite;//Does the same name file overwrite directly?
	es_bool_t ignore;   //Flags are ignored when existing files are not covered.
	es_buffer_t *buf; //Extract fragment data cache
	es_file_t *file;  //Unzip target file handle
	es_int32_t fsize; //Unzip target file size
	es_int32_t count; //Extract the processed length of the destination file
} CMemOutStream;

#define ES_RESET_OUTSTREAM(stream) do { \
	if (stream->count >= stream->fsize) { \
		if (stream->file) { \
			eso_fclose(stream->file); \
			stream->file = NULL; \
		} \
		stream->fsize = 0; \
		stream->count = 0; \
		stream->ignore = FALSE; \
	} \
} while(0);

static es_int32_t stream_read_file(void *s, void *buf, es_size_t *size)
{
	CFileInStream *inStream = (CFileInStream*)s;
	es_size_t ret = eso_fread(buf, *size, inStream->file);
	*size = ret;
	return ret >= 0 ?  ES_SZ_OK : ES_SZ_ERROR_READ;
}

static es_int32_t stream_read_buff(void *s, void *buf, es_size_t *size)
{
	CBuffInStream *inStream = (CBuffInStream*)s;
	*size = ES_MIN(*size, (es_size_t)(inStream->size - (inStream->ppos - inStream->buff)));
	eso_memcpy(buf, inStream->ppos, *size);
	inStream->ppos += *size;
	return *size >= 0 ?  ES_SZ_OK : ES_SZ_ERROR_READ;
}

static es_size_t stream_write_file(void *s, const void *buf, es_size_t size)
{
	CMemOutStream *outStream = (CMemOutStream*)s;
	char *p;
	es_size_t len = 0;
	
	ES_RESET_OUTSTREAM(outStream);
	
	if (outStream->file || outStream->ignore) {
		//Continue to decompress the previous file
		p = (char*)buf;
		
		if (outStream->count < outStream->fsize) {
			len = ES_MIN((es_size_t)(outStream->fsize-outStream->count), size);
			if (outStream->file && eso_fwrite(p, len, outStream->file) != len) {
				return ES_SZ_ERROR_WRITE;
			}
			outStream->count += len;
			p += len;
		}
		
		ES_RESET_OUTSTREAM(outStream);
		
		eso_buffer_append(outStream->buf, p, size-len);
	}
	else {
		eso_buffer_append(outStream->buf, buf, size);
	}
	
	if (outStream->buf->len > 0) {
		//Prepare to unzip the new file
		while (strlen((char*)outStream->buf->data) < outStream->buf->len) {
			es_size_t ifilename;
			int done;
			char filename[ES_PATH_MAX];
			char *pdata;

			pdata = (char*)outStream->buf->data;
			ifilename = eso_strlen(pdata);

			if (outStream->buf->len < ifilename + 5) {
				return size; //Only part of the file length is received
			}
			
			p = pdata + ifilename + 1;
			outStream->fsize = eso_array2llong((es_byte_t*)p, 4);
			done = ES_MIN(outStream->fsize, (int)(outStream->buf->len - ifilename - 5));
			outStream->count += done;
			eso_sprintf(filename, "%s%s", outStream->root, (char*)outStream->buf->data);
			p = filename;
			while (*p) {
				if (*p == '/') *p = '\\';
				p++;
			}
			eso_mkdir(filename); //Create directory
			if (filename[strlen(filename)-1] != '\\') {
				if (outStream->overwrite || !eso_fexist(filename)) {
					outStream->ignore = FALSE;
					outStream->file = eso_fopen(filename, "wb");
					if (!outStream->file) return ES_SZ_ERROR_WRITE;
					if (eso_fwrite(pdata+ifilename+5, outStream->count, outStream->file) != (es_size_t)outStream->count) {
						return ES_SZ_ERROR_WRITE;
					}  
				}
				else {
					outStream->ignore = TRUE;
				}
			}
			
			eso_buffer_delete(outStream->buf, 0, ifilename + 5 + done);
			
			ES_RESET_OUTSTREAM(outStream);
		}
	}
	
	return size;
}

es_int32_t eso_lzma_unzip_arfile(const char *ifname,
                                 es_size_t offset, 
                                 const char *opath,
                                 es_bool_t overwrite)
{
	char zip_head[32];
	CFileInStream inStream;
	CMemOutStream outStream;
	es_int32_t ret;
	
	if (!ifname || !opath) {
		return -1;
	}
	
	if (eso_strchr(opath, ':') == NULL) {
		eso_log("output dir error(no driver)!");
		return -2;
	}
	eso_mkdir(opath);
	
	inStream.s.read = stream_read_file;
	inStream.file = eso_fopen(ifname, "rb");
	if (!inStream.file) {
		return -3;
	}
	
	if (eso_fseek(inStream.file, offset, ES_SEEK_SET) != 0) {
		eso_fclose(inStream.file);
		return -4;
	}
	
	outStream.s.write = stream_write_file;
	outStream.root = opath;
	outStream.file = NULL;
	outStream.fsize = 0;
	outStream.count = 0;
	outStream.overwrite = overwrite;
	outStream.ignore = FALSE;
	
	//Enter the file integrity / validity check
	if (eso_fread(zip_head, sizeof(zip_head), inStream.file) != 32) {
		eso_fclose(inStream.file);
		return -5;
	}
	if (eso_strncmp(zip_head, "EZ", 2) != 0) {
		eso_fclose(inStream.file);
		return -6;
	}
	//TODO: crc check
	
	outStream.buf = eso_buffer_make(32, 0);
	
	ret = eso_lzma_unzip((es_ostream_t*)&outStream, (es_istream_t*)&inStream);
	
	eso_buffer_free(&outStream.buf);
	if (outStream.file) {
		eso_fclose(outStream.file);
	}
	eso_fclose(inStream.file);
	
	return ret;
}

es_int32_t eso_lzma_unzip_arbuff(const char *ibuff, 
                                 es_size_t size, 
                                 const char *opath, 
                                 es_bool_t overwrite)
{
	char zip_head[32];
	CBuffInStream inStream;
	CMemOutStream outStream;
	es_int32_t ret;
	
	if (!ibuff || size<32 || !opath) {
		return -1;
	}
	
	if (eso_strchr(opath, ':') == NULL) {
		eso_log("output dir error(no driver)!");
		return -2;
	}
	eso_mkdir(opath);
	
	inStream.s.read = stream_read_buff;
	inStream.buff = ibuff;
	inStream.size = size;
	inStream.ppos = (char*)ibuff;
	
	outStream.s.write = stream_write_file;
	outStream.root = opath;
	outStream.file = NULL;
	outStream.fsize = 0;
	outStream.count = 0;
	outStream.overwrite = overwrite;
	outStream.ignore = FALSE;
	
	//Enter the file integrity / validity check
	eso_memcpy(zip_head, ibuff, sizeof(zip_head));
	inStream.ppos += sizeof(zip_head);
	if (eso_strncmp(zip_head, "EZ", 2) != 0) {
		return -3;
	}
	//TODO: crc check
	outStream.buf = eso_buffer_make(32, 0);
	
	ret = eso_lzma_unzip((es_ostream_t*)&outStream, (es_istream_t*)&inStream);
	
	eso_buffer_free(&outStream.buf);
	if (outStream.file) {
		eso_fclose(outStream.file);
	}
	
	return ret;
}
