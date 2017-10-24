/*
 * EDomainServerSocket.hh
 *
 *  Created on: 2017-9-27
 *      Author: cxxjava@163.com
 */

#ifndef EDOMAINSERVERSOCKET_HH_
#define EDOMAINSERVERSOCKET_HH_

#include "Efc.hh"

namespace efc {
namespace utils {

/**
 *
 */

#ifdef WIN32

class EDomainServerSocket: public efc::EServerSocket {
public:
	virtual void bind(const char* pathname) THROWS(EIOException) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}
	using EServerSocket::bind;
};

#else //!

class EDomainServerSocket: public efc::EServerSocket {
public:
	virtual ~EDomainServerSocket();

	EDomainServerSocket();

	//TODO:
	EDomainServerSocket(const EDomainServerSocket& that);
	EDomainServerSocket& operator= (const EDomainServerSocket& that);

	virtual void bind(const char* pathname) THROWS(EIOException);
	virtual void bind(const char* pathname, int /* ignore */, int backlog=50) THROWS(EIOException);

	virtual ESocket* accept() THROWS(EIOException);

private:
	virtual void bind(EInetSocketAddress *endpoint, int backlog=50) THROWS(EIOException) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}
	virtual void bind(int port, int backlog=50) THROWS(EIOException) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}
};

#endif //!WIN32

} /* namespace utils */
} /* namespace efc */
#endif /* EDOMAINSERVERSOCKET_HH_ */
