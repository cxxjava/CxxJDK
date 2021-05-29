/*
 * EPipeWrapper.cpp
 *
 *  Created on: 2014-1-26
 *      Author: cxxjava@163.com
 */

#include "./EPipeWrapper.hh"
#include "../inc/EServerSocketChannel.hh"
#include "../../inc/EIOException.hh"
#include "../../inc/ERandom.hh"
#include "../../inc/EServerSocket.hh"

namespace efc {
namespace nio {

//@see: openjdk-6/jdk/src/windows/classes/sun/nio/ch/PipeImpl.java

EPipeWrapper::EPipeWrapper() {
#ifdef WIN32
	EServerSocketChannel* ssc = null;

	try {
		// loopback address
		EInetAddress lb = EInetAddress::getByName("127.0.0.1");
		ES_ASSERT(lb.isLoopbackAddress());

		// bind ServerSocketChannel to a port on the loopback address
		ssc = EServerSocketChannel::open();
		EInetSocketAddress * isa = new EInetSocketAddress(&lb, 0);
		ssc->bind(isa);
		delete isa;

		// Establish connection (assumes connections are eagerly accepted)
		EInetSocketAddress* sa = new EInetSocketAddress(&lb, ssc->socket()->getLocalPort());
		source= ESocketChannel::open(sa);
		delete sa;

		ERandom rnd;
		EIOByteBuffer* bb = EIOByteBuffer::allocate(sizeof(int));
		int secret = rnd.nextInt();
		bb->put(&secret, sizeof(int));
		bb->flip();
		source->write(bb);

		// Get a connection and verify it is legitimate
		for (;;) {
			sink = ssc->accept();
			bb->clear();
			sink->read(bb);
			bb->rewind();
			int secret2;
			bb->get(&secret2, sizeof(int), sizeof(int));
			if (secret2 == secret)
				break;
			sink->close();
			delete sink;
		}
		delete bb;
	} catch (EIOException& e) {
		finally {
			try {
				if (ssc != null)
					ssc->close();
				delete ssc;
			} catch (EIOException& e) { }
		}

		throw; //!
	} finally {
		try {
			if (ssc != null)
				ssc->close();
			delete ssc;
		} catch (EIOException& e) { }
	}
#else
	pipe = eso_pipe_create();
#endif
}

EPipeWrapper::~EPipeWrapper() {
#ifdef WIN32
	delete source;
	delete sink;
#else
	eso_pipe_destroy(&pipe);
#endif
}

int EPipeWrapper::inFD() {
#ifdef WIN32
	return source->getFDVal();
#else
	return eso_fileno(pipe->in);
#endif
}

int EPipeWrapper::outFD() {
#ifdef WIN32
	return sink->getFDVal();
#else
	return eso_fileno(pipe->out);
#endif
}

} /* namespace nio */
} /* namespace efc */
