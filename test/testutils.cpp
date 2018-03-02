/*
 * testutils.cpp
 *
 *  Created on: 2017-9-11
 *      Author: cxxjava@163.com
 */
#include "es_main.h"
#include "Efc.hh"
#include "EUtils.hh"

#define LOG(fmt,...) ESystem::out->printfln(fmt, ##__VA_ARGS__)

static void test_boundinputstream()
{
	EFileInputStream fis("/tmp/test.log");
	EBoundedInputStream bis(&fis);
	char buf[512];
	int n = bis.read(buf, sizeof(buf));
	LOG("n=%d", n);
}

static void test_domainsocket() {
	EDomainSocket ds;

	//1.
	ds.connect("/tmp/xxx.sock", 0, 10000);
	EOutputStream* os = ds.getOutputStream();
	os->write("0123456789");

	//2.
	char buf[512];
	int size = sizeof(buf);
	sp<ESocket> s = ESocket::createFromFD(ds.recvFD(buf, &size), true, true);
	LOG("buf: %s", buf);
	os = s->getOutputStream();
	const char *get_str = "GET / HTTP/1.1\r\n"
		"Accept: image/gif, image/jpeg, image/pjpeg, image/pjpeg, application/x-shockwave-flash, application/msword, application/vnd.ms-excel, application/vnd.ms-powerpoint, application/xaml+xml, application/x-ms-xbap, application/x-ms-application, */*\r\n"
		"Accept-Language: zh-cn\r\n"
		"User-Agent: Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 5.1; Trident/4.0; .NET4.0C; .NET4.0E; .NET CLR 2.0.50727)\r\n"
		"Accept-Encoding: gzip, deflate\r\n"
		"Host: www.baidu.com\r\n"
		"Connection: Close\r\n" //"Connection: Keep-Alive\r\n"
		"Cookie: BAIDUID=72CBD0B204EC83BF3C5C0FA7A9C89637:FG=1\r\n\r\n";
	os->write(get_str);
	EInputStream* is = s->getInputStream();
	is->read(buf, sizeof(buf));
	LOG("read: %s", buf);
}

static void test_domainserversocket() {
	EDomainServerSocket dss;

	dss.bind("/tmp/xxx.sock");

	sp<ESocket> client;
	while ((client = dss.accept()) != null) {
		//1.
		EInputStream* is = client->getInputStream();
		char buf[512];
		is->read(buf, sizeof(buf));
		LOG("read: %s", buf);

		EInetSocketAddress* ia = client->getRemoteSocketAddress();
		if (ia) {
			LOG("ia=%s", ia->toString().c_str());
		}

		//2.
		ESocket s("www.baidu.com", 80);
		EDomainSocket* ds = dynamic_cast<EDomainSocket*>(client.get());
		const char* ptr = "abc";
		ds->sendFD(s.getFD(), (void*)ptr, strlen(ptr));
		LOG("after sendfd.");
	}
}

MAIN_IMPL(testutils) {
	printf("main()\n");

	ESystem::init(argc, argv);

	LOG("inited.");

	do {
		try {

//			test_boundinputstream();
//			test_domainsocket();
			test_domainserversocket();

		} catch (EException& e) {
			LOG("exception: %s", e.toString().c_str());
		} catch (...) {
			LOG("Catched a exception.");
		}
	} while (0);

	LOG("exit...");

	ESystem::exit(0);

	return 0;
}
