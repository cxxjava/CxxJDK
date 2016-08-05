/*
 * EChannelInputStream.hh
 *
 *  Created on: 2014-2-8
 *      Author: cxxjava@163.com
 */

#ifndef ECHANNELINPUTSTREAM_HH_
#define ECHANNELINPUTSTREAM_HH_

#include "EInputStream.hh"
#include "EByteChannel.hh"
#include "ESynchronizeable.hh"

namespace efc {
namespace nio {

/**
 * This class is defined here rather than in java.nio.channels.Channels
 * so that code can be shared with SocketAdaptor.
 *
 * @since 1.4
 */

class EChannelInputStream: public EInputStream, public ESynchronizeable {
public:
	EChannelInputStream(EByteChannel* ch);

	virtual ~EChannelInputStream();

	virtual synchronized int read() THROWS(EIOException);

	virtual synchronized int read(void *b, int len) THROWS(EIOException);

	virtual void close() THROWS(EIOException);

protected:
	EByteChannel* ch;

	virtual int read(EIOByteBuffer* bb) THROWS(EIOException);

private:
	EIOByteBuffer* bb;// = null;
	void* bs;// = null;           // Invoker's previous array
};

} /* namespace nio */
} /* namespace efc */
#endif /* ECHANNELINPUTSTREAM_HH_ */
