/*
 * testssl.cpp
 *
 *  Created on: 2017-2-19
 *      Author: cxxjava@163.com
 */
#include "es_main.h"
#include "Efc.hh"

#ifdef HAVE_OPENSSL

#define LOG(fmt,...) ESystem::out->println(fmt, ##__VA_ARGS__)

#define SSL_FILE_PATH ""

static void test_sslsocket() {
	char buffer[4096];
	int ret;
	ESSLSocket *socket = new ESSLSocket();
	socket->setReceiveBufferSize(10240);
	socket->connect("www.baidu.com", 443, 3000);
	socket->setSoTimeout(3000);
	char *get_str = "GET / HTTP/1.1\r\n"
					"Accept: image/gif, image/jpeg, image/pjpeg, image/pjpeg, application/x-shockwave-flash, application/msword, application/vnd.ms-excel, application/vnd.ms-powerpoint, application/xaml+xml, application/x-ms-xbap, application/x-ms-application, */*\r\n"
					"Accept-Language: zh-cn\r\n"
					"User-Agent: Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 5.1; Trident/4.0; .NET4.0C; .NET4.0E; .NET CLR 2.0.50727)\r\n"
					"Accept-Encoding: gzip, deflate\r\n"
					"Host: www.baidu.com\r\n"
					"Connection: Close\r\n" //"Connection: Keep-Alive\r\n"
					"Cookie: BAIDUID=72CBD0B204EC83BF3C5C0FA7A9C89637:FG=1\r\n\r\n";
	EOutputStream *sos = socket->getOutputStream();
	EInputStream *sis = socket->getInputStream();
	sos->write(get_str, strlen(get_str));
	LOG("socket available=[%d]", sis->available());
	try {
		while ((ret = sis->read(buffer, sizeof(buffer))) > 0) {
			LOG("socket ret=[%d], available=[%d]", ret, sis->available());
			LOG("socket read=[%s]", buffer);
		}
	} catch (...) {
	}
	sis->close();
	sos->close();
	socket->close();
	delete socket;
}

static void test_sslserversocket() {
	ESSLServerSocket *serverSocket = new ESSLServerSocket();
	serverSocket->setSSLParameters(null,
			SSL_FILE_PATH "./certs/tests-cert.pem",
			SSL_FILE_PATH "./certs/tests-key.pem",
			null, null);
	serverSocket->setReuseAddress(true);
	serverSocket->setSoTimeout(3000);
	serverSocket->bind(8443);
	LOG("serverSocket=%s", serverSocket->toString().c_str());
	int count = 0;
	char buffer[11];
	while (count < 10) {
		try {
			ESSLSocket *clientSocket = serverSocket->accept();
			if (!clientSocket) {
				continue;
			}
			count++;
			EInetSocketAddress *isar = clientSocket->getRemoteSocketAddress();
			EInetSocketAddress *isal = clientSocket->getLocalSocketAddress();
			LOG("socket rip=[%s], rport=%d", isar->getHostName(), isar->getPort());
			LOG("socket lip=[%s], lport=%d", isal->getHostName(), isal->getPort());
			for (int i=0; i<100; i++) {
				try {
					EInputStream *sis = clientSocket->getInputStream();
					eso_memset(buffer, 0, sizeof(buffer) - 1);
					int n = sis->read(buffer, sizeof(buffer));
					if (n == -1) {
						LOG("EOF");
						break;
					}
					LOG("socket read=[%s]", buffer);
				} catch (EIOException &e) {
					LOG("read e=%s", e.toString().c_str());
				}
				try {
					EOutputStream *sis = clientSocket->getOutputStream();
					#define TEST_HTTP_DATA "HTTP/1.1 200 OK\r\nContent-Length: 3\r\n\r\nOK!"
					sis->write(TEST_HTTP_DATA, strlen(TEST_HTTP_DATA));
				} catch (EIOException &e) {
					LOG("write e=%s", e.toString().c_str());
				}
			}
			delete clientSocket;
		} catch (EThrowable& t) {
			LOG(t.getMessage());
		} catch (...) {
			LOG("accept error.");
		}
	}
	delete serverSocket;
}

MAIN_IMPL(testssl) {
	printf("main()\n");

	ESystem::init(argc, argv);

	LOG("inited.");

	int i = 0;
	try {
		do {
//			test_sslsocket();
			test_sslserversocket();

//		} while (++i < 5);
		} while (0);
	}
	catch (EException& e) {
		e.printStackTrace();
	}
	catch (...) {
		LOG("catch all...");
	}

	LOG("exit...");

	ESystem::exit(0);

	return 0;
}

#endif //!HAVE_OPENSSL
