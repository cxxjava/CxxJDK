/*
 * EPipedInputStream.hh
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#include "EPipedInputStream.hh"
#include "EPipedOutputStream.hh"

namespace efc {

EPipedInputStream::~EPipedInputStream()
{
	eso_pipe_destroy(&mPipe);
}

EPipedInputStream::EPipedInputStream(EPipedOutputStream *src) THROWS(EIOException)
{
	mPipe = eso_pipe_create();
	connected = false;
	connect(src);
}

EPipedInputStream::EPipedInputStream()
{
	mPipe = eso_pipe_create();
	connected = false;
}

void EPipedInputStream::connect(EPipedOutputStream *src) THROWS(EIOException)
{
	src->connect(this);
}

int EPipedInputStream::read(void *b, int len) THROWS(EIOException)
{
	if (!mPipe->in) {
		throw EIOException(__FILE__, __LINE__);
	}
	
	if (eso_feof(mPipe->in)) {
		return -1;
	}
	
	ES_ASSERT(b);
	
	if (len <= 0) {
		return 0;
	}
	
	es_size_t bytesread = eso_fread(b, len, mPipe->in);
	if (bytesread != (es_size_t)len && eso_ferror(mPipe->in)) {
		throw EIOException(__FILE__, __LINE__);
	}
	return bytesread;
}

long EPipedInputStream::available() THROWS(EIOException)
{
	if (!mPipe->in) {
		throw EIOException(__FILE__, __LINE__);
	}
	
	long curpos = eso_ftell(mPipe->in);
	if (curpos == -1) {
		throw EIOException(__FILE__, __LINE__);
	}

	long filesize = eso_fsize(mPipe->in);
	if (filesize == -1 || filesize - curpos < 0) {
		throw EIOException(__FILE__, __LINE__);
	}

	return filesize - curpos;
}

void EPipedInputStream::close() THROWS(EIOException)
{
	if (mPipe->in) {
		if (eso_fclose(mPipe->in) != 0) {
			throw EIOException(__FILE__, __LINE__);
		}
		mPipe->in = NULL;
	}
}

es_file_t* EPipedInputStream::getWriter() THROWS(EIOException)
{
	if (!mPipe->out) {
		throw EIOException(__FILE__, __LINE__);
	}
	return mPipe->out;
}

} /* namespace efc */
