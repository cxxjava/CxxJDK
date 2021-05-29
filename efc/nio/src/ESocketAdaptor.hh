/*
 * ESocketAdaptor.hh
 *
 *  Created on: 2013-9-5
 *      Author: cxxjava@163.com
 */

#ifndef ESOCKETADAPTOR_HH_
#define ESOCKETADAPTOR_HH_

#include "../../inc/ESocket.hh"
#include "../inc/ESocketChannel.hh"
#include "../../inc/EInetSocketAddress.hh"
#include "../../inc/EIOException.hh"
#include "../../inc/EIllegalArgumentException.hh"
#include "../inc/EClosedChannelException.hh"
#include "../../inc/ESocketTimeoutException.hh"

namespace efc {
namespace nio {

// Make a socket channel look like a socket.
//
// The only aspects of java.net.Socket-hood that we don't attempt to emulate
// here are the interrupted-I/O exceptions (which our Solaris implementations
// attempt to support) and the sending of urgent data.  Otherwise an adapted
// socket should look enough like a real java.net.Socket to fool most of the
// developers most of the time, right down to the exception message strings.
//
// The methods in this class are defined in exactly the same order as in
// java.net.Socket so as to simplify tracking future changes to that class.
//

class ESocketAdaptor: public ESocket {
public:
	virtual ~ESocketAdaptor();

	static ESocketAdaptor* create(ESocketChannel* sc);

	virtual sp<ESocketChannel> getChannel();

	// Override this method just to protect against changes in the superclass
	//
	virtual void connect(EInetSocketAddress* remote, int timeout=0) THROWS(EIOException);

	virtual void bind(EInetSocketAddress* local) THROWS(EIOException);

	virtual EInetAddress* getInetAddress();
	virtual EInetAddress* getLocalAddress();
	virtual int getPort();
	virtual int getLocalPort();

	virtual EInetSocketAddress* getRemoteSocketAddress();
	virtual EInetSocketAddress* getLocalSocketAddress();

	virtual EInputStream* getInputStream() THROWS(EIOException);
	virtual EOutputStream* getOutputStream() THROWS(EIOException);

	virtual void setTcpNoDelay(boolean on) THROWS(ESocketException);
	virtual boolean getTcpNoDelay() THROWS(ESocketException);

	virtual void setSoLinger(boolean on, int linger) THROWS(ESocketException);
	virtual int getSoLinger() THROWS(ESocketException);

	virtual void sendUrgentData(int data) THROWS(EIOException);

	virtual void setOOBInline(boolean on) THROWS(ESocketException);
	virtual boolean getOOBInline() THROWS(ESocketException);

	virtual void setSoTimeout(int timeout) THROWS(ESocketException);
	virtual int getSoTimeout() THROWS(ESocketException);

	virtual void setSendBufferSize(int size) THROWS(ESocketException);
	virtual int getSendBufferSize() THROWS(ESocketException);

	virtual void setReceiveBufferSize(int size) THROWS(ESocketException);
	virtual int getReceiveBufferSize() THROWS(ESocketException);

	virtual void setKeepAlive(boolean on) THROWS(ESocketException);
	virtual boolean getKeepAlive() THROWS(ESocketException);

	virtual void setTrafficClass(int tc) THROWS(ESocketException);
	virtual int getTrafficClass() THROWS(ESocketException);

	virtual void setReuseAddress(boolean on) THROWS(ESocketException);
	virtual boolean getReuseAddress() THROWS(ESocketException);

	virtual void close() THROWS(EIOException);

	virtual void shutdownInput() THROWS(EIOException);
	virtual void shutdownOutput() THROWS(EIOException);

	virtual EString toString();

	virtual boolean isConnected();
	virtual boolean isBound();
	virtual boolean isClosed();
	virtual boolean isInputShutdown();
	virtual boolean isOutputShutdown();

	virtual void setOption(int optID, const void* optval, int optlen) THROWS(ESocketException);
	virtual void getOption(int optID, void* optval, int* optlen) THROWS(ESocketException);

	virtual int getFD() THROWS(EIOException);

private:
	friend class ESocketChannel;
	friend class EServerSocketAdaptor;

	// The channel being adapted
	ESocketChannel* _sc;

	// Option adaptor object, created on demand
//	volatile OptionAdaptor opts = null;

	// Timeout "option" value for accepts
	volatile int _timeout;// = 0;

	 // Traffic-class/Type-of-service
	volatile int _trafficClass;// = 0;

//	EInputStream* _socketInputStream;// = null;
//	EOutputStream *_socketOutputStream;// = null;

	ESocketAdaptor(ESocketChannel* sc);
};

} /* namespace nio */
} /* namespace efc */
#endif /* ESOCKETADAPTOR_HH_ */
