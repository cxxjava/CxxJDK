/*
 * EDomainServerSocket.cpp
 *
 *  Created on: 2017-9-27
 *      Author: cxxjava@163.com
 */

#include "../inc/EDomainServerSocket.hh"
#include "../inc/EDomainSocket.hh"

#ifndef WIN32

#include <sys/un.h>
#include <sys/socket.h>
#include <sys/stat.h>

namespace efc {
namespace utils {

EDomainServerSocket::~EDomainServerSocket() {
	//
}

EDomainServerSocket::EDomainServerSocket() : EServerSocket(null) {
	socket = eso_net_socket(PF_LOCAL, SOCK_STREAM, 0);
	if (socket < 0) {
		throw EIOException(__FILE__, __LINE__);
	}
}

void EDomainServerSocket::bind(const char* pathname) {
	this->bind(pathname, 0);
}

void EDomainServerSocket::bind(const char *pathname, int /* ignore */, int backlog) {
	if (!pathname || !*pathname) {
		throw ENullPointerException(__FILE__, __LINE__);
	}

	/* in case it already exists */
	struct stat statbuf;
	if ((::stat(pathname, &statbuf) == 0) && (S_ISSOCK(statbuf.st_mode))) {
		unlink(pathname);
	}

	struct sockaddr_un sau;
	memset(&sau, 0, sizeof(sau));
	sau.sun_family = PF_LOCAL;
	strncpy(sau.sun_path, pathname, sizeof(sau.sun_path));

	if (::bind(socket, (struct sockaddr *)&sau, sizeof(sau)) != 0) {
		throw EBindException(__FILE__, __LINE__, "Bind failed");
	}

	status.bound = 1;

	int ret = eso_net_listen(socket, backlog);
	if (ret != 0) {
		throw EIOException(__FILE__, __LINE__, "Listen failed");
	}
}

ESocket* EDomainServerSocket::accept() {
	if (isClosed())
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	if (!isBound())
		throw ESocketException(__FILE__, __LINE__, "Socket is not bound yet");

	struct sockaddr_un sau;
	socklen_t addrlen = sizeof(sau);
	int newsock = ::accept(socket, (struct sockaddr *)&sau, &addrlen);
	if (newsock < 0) {
		if (newsock == -2) {
			throw ESocketTimeoutException(__FILE__, __LINE__, "Accept timed out");
		} else {
			throw ESocketException(__FILE__, __LINE__, "Accept failed");
		}
	}

	EDomainSocket *newSocket = null;
	try {
		newSocket = new EDomainSocket(newsock, true, true);
	} catch (EIOException& e) {
		eso_net_close(newsock);
		delete newSocket;
		throw ESocketException(__FILE__, __LINE__, "Implement accept socket failed");
	}
	return newSocket;
}

} /* namespace utils */
} /* namespace efc */

#endif //!WIN32
