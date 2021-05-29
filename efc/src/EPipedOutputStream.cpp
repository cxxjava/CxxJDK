/*
 * EPipedOutputStream.hh
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#include "EPipedOutputStream.hh"
#include "EPipedInputStream.hh"

namespace efc {

EPipedOutputStream::~EPipedOutputStream()
{
	/* 20180207: removed for destroy error when owned object freed before!
    try {
        close();
    } catch (...) {
    }
    */
}

EPipedOutputStream::EPipedOutputStream(EPipedInputStream *snk) THROWS(EIOException)
{
	sink = snk;
}

EPipedOutputStream::EPipedOutputStream()
{
	sink = null;
}

void EPipedOutputStream::connect(EPipedInputStream *snk) THROWS(EIOException)
{
	if (!snk) {
		throw ENullPointerException(__FILE__, __LINE__);
	}
	
	if (sink || snk->connected) {
		throw EIOException(__FILE__, __LINE__, "Already connected");
	}
	
	sink = snk;
	snk->connected = true;
}

void EPipedOutputStream::write(const void *b, int len) THROWS(EIOException)
{
	if (sink == null) {
		throw EIOException(__FILE__, __LINE__, "Pipe not connected");
	} else if (b == null) {
	    throw ENullPointerException(__FILE__, __LINE__);
	} else if (len <= 0) {
	    return;
	}
	
	if (eso_fwriten(b, len, sink->getWriter()) != (es_size_t)len) {
		throw EIOException(__FILE__, __LINE__);
	}
}

void EPipedOutputStream::write(const char *s)
{
	EOutputStream::write(s);
}

void EPipedOutputStream::write(int b)
{
	EOutputStream::write(b);
}

void EPipedOutputStream::close() THROWS(EIOException)
{
	if (sink && sink->mPipe->out) {
		if (eso_fclose(sink->mPipe->out) != 0) {
			throw EIOException(__FILE__, __LINE__);
		}
		sink->mPipe->out = NULL;
	}
}

} /* namespace efc */
