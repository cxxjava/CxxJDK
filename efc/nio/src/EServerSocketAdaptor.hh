/*
 * EServerSocketAdaptor.hh
 *
 *  Created on: 2013-9-4
 *      Author: cxxjava@163.com
 */

#ifndef ESERVERSOCKETADAPTOR_HH_
#define ESERVERSOCKETADAPTOR_HH_

#include "../../inc/EServerSocket.hh"
#include "../inc/EServerSocketChannel.hh"
#include "../../inc/EIOException.hh"

namespace efc {
namespace nio {

// Make a server-socket channel look like a server socket.
//
// The methods in this class are defined in exactly the same order as in
// java.net.ServerSocket so as to simplify tracking future changes to that
// class.
//

class EServerSocketAdaptor : public EServerSocket {
public:
	virtual ~EServerSocketAdaptor();

	static EServerSocketAdaptor* create(EServerSocketChannel* ssc);

	virtual void bind(EInetSocketAddress* local, int backlog=50) THROWS(EIOException);
	virtual void bind(int port, int backlog=50) THROWS(EIOException);

	virtual EInetAddress* getInetAddress();
	virtual int getLocalPort();

	virtual EInetSocketAddress* getLocalSocketAddress();

	virtual ESocket* accept() THROWS(EIOException);

	virtual void close() THROWS(EIOException);

	virtual sp<EServerSocketChannel> getChannel();

	virtual boolean isBound();
	virtual boolean isClosed();

	virtual void setSoTimeout(int timeout);
	virtual int getSoTimeout();

	virtual void setReuseAddress(boolean on) THROWS(ESocketException);
	virtual boolean getReuseAddress() THROWS(ESocketException);

	virtual void setReceiveBufferSize(int size) THROWS(ESocketException);
	virtual int getReceiveBufferSize() THROWS(ESocketException);

	virtual EString toString();

	virtual void setOption(int optID, const void* optval, int optlen)
				THROWS(ESocketException);
	virtual void getOption(int optID, void* optval, int* optlen)
				THROWS(ESocketException);

	virtual int getFD() THROWS(EIOException);

private:
	friend class EServerSocketChannel;

	// The channel being adapted
	EServerSocketChannel* _ssc;

	// Option adaptor object, created on demand
//	OptionAdaptor opts = null;

	// Timeout "option" value for accepts
	volatile int _timeout; // = 0;

	EServerSocketAdaptor(EServerSocketChannel* ssc);
};

} /* namespace nio */
} /* namespace efc */

#endif /* ESERVERSOCKETADAPTOR_HH_ */
