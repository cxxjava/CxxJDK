/*
 * EDatagramSocketAdaptor.hh
 *
 *  Created on: 2016-8-23
 *      Author: cxxjava@163.com
 */

#ifndef EDATAGRAMSOCKETADAPTOR_HH_
#define EDATAGRAMSOCKETADAPTOR_HH_

#include "../../inc/EDatagramSocket.hh"
#include "../inc/EDatagramChannel.hh"

namespace efc {
namespace nio {

// Make a datagram-socket channel look like a datagram socket.
//
// The methods in this class are defined in exactly the same order as in
// java.net.DatagramSocket so as to simplify tracking future changes to that
// class.
//

class EDatagramChannel;

class EDatagramSocketAdaptor: public EDatagramSocket {
public:
	virtual ~EDatagramSocketAdaptor();

	static EDatagramSocketAdaptor* create(EDatagramChannel* dc);

	virtual sp<EDatagramChannel> getChannel();

	virtual void bind(EInetSocketAddress* local) THROWS(ESocketException);
	virtual void connect(EInetSocketAddress* remote) THROWS(ESocketException);
	virtual void connect(EInetAddress* address, int port) THROWS(ESocketException);
	virtual void disconnect();

	virtual boolean isBound();
	virtual boolean isConnected();

	virtual EInetAddress* getInetAddress();
	virtual int getPort();

	virtual void send(EDatagramPacket* p) THROWS(EIOException);
	virtual void receive(EDatagramPacket* p) THROWS(EIOException);

	virtual sp<EInetAddress> getLocalAddress();
	virtual int getLocalPort();

	virtual void setSoTimeout(int timeout) THROWS(ESocketException);
	virtual int getSoTimeout() THROWS(ESocketException);

	virtual void close();
	virtual boolean isClosed();

	virtual void setSendBufferSize(int size) THROWS(ESocketException);
	virtual int getSendBufferSize() THROWS(ESocketException);
	virtual void setReceiveBufferSize(int size) THROWS(ESocketException);
	virtual int getReceiveBufferSize() THROWS(ESocketException);
	virtual void setReuseAddress(boolean on) THROWS(ESocketException);
	virtual boolean getReuseAddress() THROWS(ESocketException);
	void setBroadcast(boolean on) THROWS(ESocketException);
	virtual boolean getBroadcast() THROWS(ESocketException);
	virtual void setTrafficClass(int tc) THROWS(ESocketException);
	virtual int getTrafficClass() THROWS(ESocketException);

	virtual int getFD() THROWS(EIOException);

private:
	friend class EDatagramChannel;

	// The channel being adapted
	EDatagramChannel* _dc;

	// Timeout "option" value for receives
	volatile int _timeout;// = 0;

	EDatagramSocketAdaptor(EDatagramChannel* dc) THROWS(EIOException);
};

} /* namespace nio */
} /* namespace efc */
#endif /* EDATAGRAMSOCKETADAPTOR_HH_ */
