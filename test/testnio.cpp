#include "es_main.h"
#include "Efc.hh"

#define LOG(fmt,...) ESystem::out->printfln(fmt, ##__VA_ARGS__)

static void handleKey(ESelector *selector, sp<ESelectionKey> selectionKey) {
	sp<EServerSocketChannel> server = null;
	sp<ESocketChannel> client = null;

	EIOByteBuffer receiveBuffer(512);
	EA<EIOByteBuffer*> sends(3);
	sends[0] = EIOByteBuffer::allocate();
	sends[1] = EIOByteBuffer::allocate();
	sends[2] = EIOByteBuffer::allocate();
	int count=0;

	if (selectionKey->isAcceptable()) {
		server = dynamic_pointer_cast<EServerSocketChannel>(selectionKey->channel());
		client = server->accept();
		if (!client) {
			return;
		}
//		client->close();
//		return;

		client->configureBlocking(false );
//		client->register_(selector, ESelectionKey::OP_READ | ESelectionKey::OP_WRITE);
		client->register_(selector, ESelectionKey::OP_READ);
	} else if (selectionKey->isReadable()) {
		client = dynamic_pointer_cast<ESocketChannel>(selectionKey->channel());

//		if (!client) {
//			return;
//		}
//		client->close();
//		return;

		receiveBuffer.clear();

		try {
			count = client->read(&receiveBuffer);
		} catch (...) {
			client->close();
			return;
		}
        if (count == -1) {
        	ESystem::out->println("client closed!");
            client->close();
			return;
        }
		if (count > 0) {
			char ss[6] = {0};
			receiveBuffer.flip();
			eso_strncpy(ss, (char*)receiveBuffer.current(), 3);
//			ESystem::out->printfln("server recv:%s", ss);
//			client->register_(selector, ESelectionKey::OP_WRITE);
			selectionKey->interestOps(ESelectionKey::OP_READ | ESelectionKey::OP_WRITE);
		}
	} else if (selectionKey->isWritable()) {
		//sendBuffer.clear();
		client = dynamic_pointer_cast<ESocketChannel>(selectionKey->channel());
		//sendBuffer.put(sendText.getBytes());
        sends[0]->put("HTTP/1.1 200 OK\r\n", 17);
		sends[0]->flip();
        sends[1]->put("Content-Type: text/html; charset=UTF-8\r\n\r\n", 42);
		sends[1]->flip();
        sends[2]->put("1234567890", 10);
		sends[2]->flip();
		try {
			client->write(&sends);
		} catch (...) {
			client->close();
			return;
		}
//		ESystem::out->printfln("server send:%s", "1234567890");
//		client->register_(selector, ESelectionKey::OP_READ);
        selectionKey->interestOps(ESelectionKey::OP_READ);
//        selectionKey->interestOps(0);

		client->close();
	}
}

static void test_nioserver() {

	sp<EServerSocketChannel> ssc = EServerSocketChannel::open();
	ssc->configureBlocking(false);
	EInetSocketAddress isa(8899);
	ssc->bind(&isa, 4096);

	ESelector *selector = ESelector::open();
	ssc->register_(selector, ESelectionKey::OP_ACCEPT);

	long ccc = 0;

//	while (ccc < 100) {
	while (true) {
		int readyChannels = selector->select();
		if (readyChannels == 0) continue;
//		LOG("readyChannels=%d", readyChannels);
		ESet<sp<ESelectionKey> >* selectionKeys = selector->selectedKeys();
		sp<EIterator<sp<ESelectionKey> > > iterator = selectionKeys->iterator();
		while (iterator->hasNext()) {
			sp<ESelectionKey> selectionKey = iterator->next();
			handleKey(selector, selectionKey);
			iterator->remove();
		}

		ccc++;
	}

	ssc->close(); //!
	selector->close(); //!
	delete selector;

	LOG("test_nioserver() finished.");

//	eso_thread_sleep(1000000);
}

static void test_nioserversocket() {
	sp<EServerSocketChannel> ssc = EServerSocketChannel::open();
	EInetSocketAddress isa(8899);
	ssc->bind(&isa);

	EServerSocket* ss = ssc->socket();
//	EServerSocketChannel* channel = (EServerSocketChannel*)ss->getChannel();
//	ss->setSoTimeout(10000);
	ESocket* s = ss->accept();
	LOG("s=%x", s);

	EInputStream* is = s->getInputStream();
	char b[100] = {0};
	LOG("avaliable size=%d", is->available());
	int len = is->read(b, sizeof(b));
	LOG("len=%d, s=%s", len, b);

	EOutputStream* os = s->getOutputStream();
	os->write(b, 12);

	delete s;
}

//===========================================================================


static void test_nioclient() {
	sp<ESocketChannel> socketChannel = ESocketChannel::open();
	socketChannel->configureBlocking(false );
	ESelector* selector = ESelector::open();
	socketChannel->register_(selector, ESelectionKey::OP_CONNECT);
	EInetSocketAddress SERVER_ADDRESS("localhost", 8096);
//	EInetSocketAddress SERVER_ADDRESS("10.211.55.8", 8899);
	socketChannel->connect(&SERVER_ADDRESS);

	ESet<sp<ESelectionKey> >* selectionKeys;
	sp<EIterator<sp<ESelectionKey> > > iterator;
	sp<ESelectionKey> selectionKey;
	sp<ESocketChannel> client;
	int count = 0;
	EIOByteBuffer sendbuffer(512);
	EIOByteBuffer receivebuffer(512);

	int nn = 0;
	do {
		nn++;

		selector->select();
		selectionKeys = selector->selectedKeys();
		//System.out.println(selectionKeys.size());
		iterator = selectionKeys->iterator();
		while (iterator->hasNext()) {
			selectionKey = iterator->next();
			if (selectionKey->isConnectable()) {
				ESystem::out->println("client connect");
				client = dynamic_pointer_cast<ESocketChannel>(selectionKey->channel());
				if (client->isConnectionPending()) {
					client->finishConnect();
					ESystem::out->println("connect finished!");
					sendbuffer.clear();
					sendbuffer.put("Hello,Server", 12);
					sendbuffer.flip();
					client->write(&sendbuffer);
				}
//				client->register_(selector, ESelectionKey::OP_READ | ESelectionKey::OP_WRITE);
				client->register_(selector, ESelectionKey::OP_WRITE);
			} else if (selectionKey->isReadable()) {
				client = dynamic_pointer_cast<ESocketChannel>(selectionKey->channel());
				receivebuffer.clear();
				try {
					count = client->read(&receivebuffer);
				} catch (...) {
					client->close();
				}
				if(count>0){
					receivebuffer.flip();
					ESystem::out->printfln("recev server:%s", receivebuffer.current());
//					client->register_(selector, ESelectionKey::OP_WRITE);
					selectionKey->interestOps(ESelectionKey::OP_READ | ESelectionKey::OP_WRITE);
				}
			} else if (selectionKey->isWritable()) {
				sendbuffer.clear();
				client = dynamic_pointer_cast<ESocketChannel>(selectionKey->channel());
				sendbuffer.put("xxx", 3);
				sendbuffer.flip();

#if 0
				//test transferTo
				EFileChannel* fc2 = EFileChannel::open("test_file.txt", true, true, true);
				fc2->transferTo(0, 10, client);
				delete fc2;
#endif
				try {
					client->write(&sendbuffer);
				} catch (...) {
					client->close();
				}
				ESystem::out->printfln("sendto server:%s", "xxx");
//				client->register_(selector, ESelectionKey::OP_READ);
				selectionKey->interestOps(ESelectionKey::OP_READ);
			}
		}
		selectionKeys->clear();
	} while (nn < 1);

	selector->close();
	delete selector;

	socketChannel->close();
}

//===========================================================================

static void test_bytebuffer1() {
	char addr[11] = {'x', 0};
	EIOByteBuffer *bb = EIOByteBuffer::wrap(addr, 10, 0);

	ESystem::out->printfln("bb text0:[%d]%s", bb->remaining(), bb->current());
	bb->put('a');
	bb->put('b');
	bb->put('c');
	bb->put('d');
	bb->flip();
	ESystem::out->printfln("bb text1:[%d]%s", bb->remaining(), bb->current());
	bb->put('1');
	bb->put('2');
	bb->put('3');
	bb->put('4');
	bb->flip();
	ESystem::out->printfln("bb text2:[%d]%s", bb->remaining(), bb->current());
	ESystem::out->printfln("bb byte1:%c", bb->get());
	ESystem::out->printfln("bb byte2:%c", bb->get());
	ESystem::out->printfln("bb byte3:%c", bb->get());
	ESystem::out->printfln("bb byte4:%c", bb->get());

	ESystem::out->printfln("bb byte0:%c", bb->get(0));
	ESystem::out->printfln("bb byte3:%c", bb->get(3));
//	ESystem::out->printfln("bb byte4:%c", bb->get(4));

	bb->clear();
	bb->put("ABCDEFG12345678", 10);
	bb->flip();
	ESystem::out->printfln("bb text3:[%d]%s", bb->remaining(), bb->current());
	bb->put("ABCDEFG12345678", 10);
	char readBuf[20] = {0};
	bb->flip();
	ESystem::out->printfln("bb text4:[%d]", bb->remaining());
	bb->get(readBuf, sizeof(readBuf), 10);
	ESystem::out->printfln("bb text4:[%d]%s", bb->remaining(), readBuf);

	delete bb;
}

static void test_bytebuffer2() {
	EIOByteBuffer bb(8);
	bb.put((byte) 'h')->put((byte) 'e')->put((byte) 'l')->put((byte) 'l')->put(
			(byte) 'o')->put((byte) '!');
	bb.flip();
	while (bb.hasRemaining()) {
		ESystem::out->printf("%c", (char) bb.get());
	}
	ESystem::out->println();
	bb.put(0, (byte) 'a');
	bb.put(1, (byte) 'b');
	bb.rewind();
	for (int i = 0; i < bb.remaining(); i++) {
		ESystem::out->printf("%c", (char) bb.get(i));
	}
	ESystem::out->println();
	bb.clear();
}

static void test_bytebuffer() {
	byte buf[512];
	sp<EIOByteBuffer> bb = EIOByteBuffer::wrap((void*)buf, 512);
	bb->putLLong(0, 998877665544L);
//	bb->flip();
	llong l = bb->getLLong(0);
	LOG("l=%ld", l);

	sp<EIOByteBuffer> bf = EIOByteBuffer::wrap((void*)"consddss", 8);
	short n = bf->getShort();
	LOG("n=%ld", n);

//    EIOByteBuffer receiveBuffer;
	EA<EIOByteBuffer*> sends(3);
	sends[0] = new EIOByteBuffer((void*)"1234567890", 10, 0);
	sends[1] = new EIOByteBuffer((void*)"abcdefghijk", 10, 0);
	sends[2] = new EIOByteBuffer((void*)"ABCDEFGHIJK", 10, 0);
    
//	try {
		test_bytebuffer1();
//	} catch (...){}

	try {
		test_bytebuffer2();
	} catch (...){}
}

//#include <fcntl.h>
static void test_filechannel() {
	es_file_t* pfile = eso_fopen("./test_file.txt", "r+");
//	int fd = ::open("./test_file.txt", O_RDWR);
//	int r = ::write(eso_fileno(pfile), (void*)"1234567890", 10);

	EFileChannel* fc = EFileChannel::open(eso_fileno(pfile), true, true, false);

	//test write
	EIOByteBuffer* src = new EIOByteBuffer((void*)"vvvxxxx234567890", 10, 0);
	fc->write(src);
	delete src;

	//test writes
	EA<EIOByteBuffer*>* srcs = new EA<EIOByteBuffer*>(3);
	(*srcs)[0] = new EIOByteBuffer((void*)"000000000000", 10, 0);
	(*srcs)[1] = new EIOByteBuffer((void*)"111111111111", 10, 0);
	(*srcs)[2] = new EIOByteBuffer((void*)"222222222222", 10, 0);
	fc->write(srcs, 0, 2);
	fc->force(true);
	fc->position(0);
	EIOByteBuffer* dst = new EIOByteBuffer(32);
	fc->read(dst);
	dst->flip();
	LOG("read=%s", (char*)dst->current());
	delete dst;
	delete srcs;

	//test EMappedByteBuffer
	EMappedByteBuffer* mbb = fc->map(READ_WRITE, 0, 20);
	mbb->put((void*)"kkkkkkkkkk", 5);
	mbb->flip();
	fc->read(mbb);
	mbb->flip();
	LOG("read=%s", (char*)mbb->current());
	delete mbb;

	//test flock
	EFileLock* fl = fc->lock();
	//TODO...
	delete fl;

	//test transferTo
	EFileChannel* fc2 = EFileChannel::open("test_file2.txt", true, true, false);
	fc->transferTo(0, 10, fc2);
	delete fc2;

	delete fc;

//	::close(fd);
	eso_fclose(pfile);
}

static void sendFile(EDatagramChannel* ch, EInetSocketAddress* remote) {
	EFileChannel* fc = EFileChannel::open("/tmp/b11.cpp", true, false, false);

	// if not connect then read or write, channel will throw NotYetConnectedException.
	ch->connect(remote);

	fc->transferTo(0, fc->size(), ch);

	delete fc;
}

static void test_nioudpserver() {
	sp<EDatagramChannel> channel;
	ESelector* selector;
	EInetSocketAddress isa(9001);

	try {
		channel = EDatagramChannel::open();
		selector = ESelector::open();

		channel->configureBlocking(false);
		channel->socket()->setReuseAddress(true);
		channel->socket()->bind(&isa);
		channel->register_(selector, ESelectionKey::OP_READ);
	} catch (EException& e) {
		e.printStackTrace();
	}

	int timeout = 3000;
	EIOByteBuffer* buf = EIOByteBuffer::allocate(65536);
	while (true) {
		try {
			int n = selector->select(timeout);
			if (n > 0) {
				sp<EIterator<sp<ESelectionKey> > > iter = selector->selectedKeys()->iterator();
				while (iter->hasNext()) {
					sp<ESelectionKey> key = iter->next();
					iter->remove();

					sp<EDatagramChannel> ch = dynamic_pointer_cast<EDatagramChannel>(key->channel());

					if (key->isReadable()) {
						buf->clear();

						sp<EInetSocketAddress> raddr = ch->receive(buf);
						LOG("addr=%d, port=%d, buf=%s", raddr->getAddress()->getAddress(), raddr->getPort(), (char*)buf->address());

						if (0) {
							buf->flip();
							ch->send(buf, raddr.get());
						} else {
							sendFile(ch.get(), raddr.get());
						}
					}
				}
			}
		} catch (EException& e) {
			e.printStackTrace();
		}
	}

	channel->close();
	selector->close();

	delete buf;
	delete selector;
}

MAIN_IMPL(testnio) {
	ESystem::init(argc, argv);

	try {
		do {

//		test_bytebuffer();
		test_nioserver();
//		test_nioclient();
//		test_nioserversocket();
//		test_filechannel();
//		test_nioudpserver();
//		test_nioudpclient();

		} while (1);
	}
	catch (EException& e) {
		ESystem::out->printfln("e=%s", e.toString().c_str());
		e.printStackTrace();
	}
	catch (...) {
		ESystem::out->println("catch all...");
	}

	ESystem::exit(0);

	return 0;
}
