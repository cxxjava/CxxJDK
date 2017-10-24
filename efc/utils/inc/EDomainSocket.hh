/*
 * EDomainSocket.hh
 *
 *  Created on: 2017-9-26
 *      Author: cxxjava@163.com
 */

#ifndef EDOMAINSOCKET_HH_
#define EDOMAINSOCKET_HH_

#include "Efc.hh"

namespace efc {
namespace utils {

#ifdef WIN32

class EDomainSocket: public efc::ESocket {
public:
	virtual void sendFD(int fd, void *ptr=0, int nbytes=0) THROWS(EIOException) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}
	virtual int recvFD(void *ptr=0, int *nbytes=0) THROWS(EIOException) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}

	static EDomainSocket* createFromFD(int fd, boolean connected, boolean bound) THROWS(EIOException) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}
};

#else //!

class EDomainSocket: public efc::ESocket {
public:
	virtual ~EDomainSocket();

	EDomainSocket() THROWS(EIOException);

	//TODO:
	EDomainSocket(const EDomainSocket& that);
	EDomainSocket& operator= (const EDomainSocket& that);

	virtual void connect(const char *pathname, int /* ignore */, int timeout=0) THROWS(EIOException);

	virtual void sendFD(int fd, void *ptr=0, int nbytes=0) THROWS(EIOException);
	virtual int recvFD(void *ptr=0, int *nbytes=0) THROWS(EIOException);

	static EDomainSocket* createFromFD(int fd, boolean connected, boolean bound) THROWS(EIOException);

protected:
	friend class EDomainServerSocket;

	/**
	 * Initializes a new instance of this class from fd
	 */
	EDomainSocket(const int fd, boolean connected, boolean bound) THROWS(EIOException);

private:
	EString pathname_;

	virtual void connect(EInetSocketAddress *endpoint, int timeout=0) THROWS(EIOException) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}
	virtual void bind(EInetSocketAddress *bindpoint) THROWS(EIOException) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}
	virtual void bind(const char *hostname, int port) THROWS(EIOException) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}
};

#endif //!WIN32

} /* namespace utils */
} /* namespace efc */
#endif /* EDOMAINSOCKET_HH_ */
