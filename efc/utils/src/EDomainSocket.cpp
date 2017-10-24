/*
 * EDomainSocket.cpp
 *
 *  Created on: 2017-9-26
 *      Author: cxxjava@163.com
 */

#include "../inc/EDomainSocket.hh"

#ifndef WIN32

#include <sys/un.h>
#include <sys/socket.h>
#include <sys/stat.h>

namespace efc {
namespace utils {

EDomainSocket::~EDomainSocket() {
	//
}

EDomainSocket::EDomainSocket() : ESocket(-1, false, false) {
	socket = eso_net_socket(PF_LOCAL, SOCK_STREAM, 0);
	if (socket < 0) {
		throw EIOException(__FILE__, __LINE__);
	}
}

EDomainSocket::EDomainSocket(const int fd, boolean connected, boolean bound) :
		ESocket(fd, connected, bound, 0, 0) {
	//
}

EDomainSocket* EDomainSocket::createFromFD(int fd, boolean connected, boolean bound) {
	return new EDomainSocket(fd, connected, bound);
}

void EDomainSocket::connect(const char *pathname, int /* ignore */, int timeout) {
	if (isClosed())
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");

	if (isConnected())
		throw ESocketException(__FILE__, __LINE__, "already connected");

	struct sockaddr_un sau;
	memset(&sau, 0, sizeof(struct sockaddr_un));
	sau.sun_family = PF_LOCAL;
	strncpy(sau.sun_path, pathname, sizeof(sau.sun_path));

	RETRY:

	int ret;
	RESTARTABLE(::connect(socket, (struct sockaddr *)&sau, sizeof(sau)), ret);
	if (ret != 0) {
		if ((errno == ENOENT || errno == ECONNREFUSED)
							&& (timeout > 0)) {
			timeout -= 100; //100 millisecond
			if (timeout > 0) {
				EThread::sleep(ES_MIN(timeout, 100));
				goto RETRY;
			}
		}
		throw EPortUnreachableException(__FILE__, __LINE__, "connect failed");
	}

	status.connected = 1;

	/*
	 * If the socket was not bound before the connect, it is now because
	 * the kernel will have picked an ephemeral port & a local address
	 */
	status.bound = 1;
}

void EDomainSocket::sendFD(int fd, void *ptr, int nbytes) {
	struct msghdr hdr;
	struct iovec data;

	char cmsgbuf[CMSG_SPACE(sizeof(int))];

	if (!ptr) {
		const char* dummy = "";
		data.iov_base = &dummy;
		data.iov_len = 1;
	} else {
		data.iov_base = ptr;
		data.iov_len = nbytes;
	}

	memset(&hdr, 0, sizeof(hdr));
	hdr.msg_name = NULL;
	hdr.msg_namelen = 0;
	hdr.msg_iov = &data;
	hdr.msg_iovlen = 1;
	hdr.msg_flags = 0;

	hdr.msg_control = cmsgbuf;
	hdr.msg_controllen = CMSG_LEN(sizeof(int));

	struct cmsghdr* cmsg = CMSG_FIRSTHDR(&hdr);
	cmsg->cmsg_len   = CMSG_LEN(sizeof(int));
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type  = SCM_RIGHTS;

	*(int*)(CMSG_DATA(cmsg)) = fd;

	ssize_t ret = ::sendmsg(socket, &hdr, 0 /*MSG_DONTWAIT*/);
	if (ret <= 0) {
		throw ESocketException(__FILE__, __LINE__, "sendmsg failed");
	}
}

int EDomainSocket::recvFD(void *ptr, int *nbytes) {
	struct msghdr msg;
	struct iovec iov[1];
	struct cmsghdr *cmsg = NULL;
	char ctrl_buf[CMSG_SPACE(sizeof(int))];
	char data[1];

	memset(&msg, 0, sizeof(struct msghdr));
	memset(ctrl_buf, 0, CMSG_SPACE(sizeof(int)));

	/* For the dummy data */
	if (ptr && nbytes) {
		iov[0].iov_base = ptr;
		iov[0].iov_len = (*nbytes);
	} else {
		iov[0].iov_base = data;
		iov[0].iov_len = sizeof(data);
	}

	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_control = ctrl_buf;
	msg.msg_controllen = CMSG_SPACE(sizeof(int));
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	ssize_t rv;
	if ((rv = ::recvmsg(socket, &msg, 0 /*MSG_DONTWAIT*/)) == -1) {
		throw ESocketException(__FILE__, __LINE__, "recvmsg failed");
	}
	if (nbytes) (*nbytes) = rv;

	if ((msg.msg_flags & MSG_TRUNC) || (msg.msg_flags & MSG_CTRUNC)) {
		throw ESocketException(__FILE__, __LINE__);
	}

	int fd = -1;
	if ((cmsg = CMSG_FIRSTHDR(&msg))
		&& cmsg->cmsg_len == CMSG_LEN(sizeof(int))
		&& cmsg->cmsg_level == SOL_SOCKET
		&& cmsg->cmsg_type == SCM_RIGHTS)
	{
		fd = *(int*) CMSG_DATA(cmsg);
	}

	return fd;
}

} /* namespace utils */
} /* namespace efc */

#endif //!WIN32
