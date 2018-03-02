#include "es_main.h"
#include "Efc.hh"

#define LOG(fmt,...) ESystem::out->printfln(fmt, ##__VA_ARGS__)

#ifdef CPP11_SUPPORT

static void test_scopeExit() {
	EString* s1 = new EString("xxx");
	ON_SCOPE_EXIT(delete s1;);

	EString* s2 = new EString("xxx");
	ON_SCOPE_EXIT( {
		LOG("delete...");
		delete s2;
	} );

	LOG("s1=%s", s1->c_str());
	LOG("s2=%s", s2->c_str());

	auto onFailureRollback = MakeScopeGuard([&] {
		LOG("onFailureRollback...");
	});

	//...

	onFailureRollback.dismiss();
}

static void test_finally() {
	int i = 0;
	for (;;) {
//		try {
		{
			ON_SCOPE_EXIT(LOG("finally. i=%d", i););

			ON_SCOPE_EXIT(
				ON_SCOPE_EXIT(
						LOG("ON_SCOPE_EXIT 2");
				);
				LOG("ON_SCOPE_EXIT 1");
			);

			LOG("run 0...");
			if (++i == 1) {
				LOG("run 1... i=%d", i);
				continue;
			}
		}
//		finally {
//			LOG("finally. i=%d", i);
//		}
		LOG("run 2... i=%d", i);
		if (i++ == 2)
			break;
	}

	ON_FINALLY_NOTHROW(
		LOG("finally.");
		for (int i=0; i<10; i++) {
			LOG("finally i=%d.", i);
		}
	) {
		try {
			LOG("at here 0.");

			throw EException(__FILE__, __LINE__);

			LOG("at here 1.");
		} catch (...) {
			LOG("exception.");
		}
    }}

	//==================================

	LOG("---");

	try {
		LOG("at here 0.");

		throw EException(__FILE__, __LINE__);

		LOG("at here 0.");
	} catch (...) {
		LOG("finally 0.");

		try {
			LOG("at here 1.");

			throw EException(__FILE__, __LINE__);

			LOG("at here 11.");
		} catch (...) {
			LOG("finally 1.");
		}
	}

	LOG("---");

	ON_FINALLY_NOTHROW(
		LOG("finally 0.");

		ON_FINALLY_NOTHROW(
			LOG("finally 1.");
		) {
			LOG("at here 1.");

//			throw EException(__FILE__, __LINE__, "11111"); //error!

			LOG("at here 11.");
        }}
	) {
		LOG("at here 0.");

//		throw EException(__FILE__, __LINE__, "00000");

		LOG("at here 0.");
    }}
}

static void test_threadx() {
	ESynchronizeable sync;

	sp<EThread> ths1 = EThread::executeX([&]() {
		//EThread::sleep(1000); //if wait after notify then will blocked forever.
		SYNCHRONIZED(&sync) {
			sync.wait();
		}}
	});
//	ths1->start(); //exception: Already started.

	sp<EThread> ths2 = EThread::executeX([&]() {
		EThread::sleep(1000);
		SYNCHRONIZED(&sync) {
			LOG("at here.");
		}}
		SYNCHRONIZED(&sync) {
			sync.notify();
		}}
	});
	ths1->join();
	ths2->join();

	LOG("end of test_threadx().");
}

static void test_timerx() {
	ETimer timer;

	sp<ETimerTask> task = timer.scheduleX((llong)0, 100, []() {
		LOG("timer runging...");
	});

	EThread::sleep(20000);

	LOG("end of test_timerx().");
}

static void test_executors() {
	//1.
	{
		sp<EExecutorService> executors = EExecutors::newCachedThreadPool();

		executors->executeX([]() {
			for (int i=0; i<1000; i++) {
				LOG("1 running...");
			}
		});

		executors->shutdown();
		executors->awaitTermination();
	}

	//2.
	{
		sp<EInteger> result(new EInteger(999));
		sp<ECallable<EInteger> > callable = EExecutors::callableX([](){
			LOG("2 running...");
		}, result);
		sp<EFutureTask<EInteger> > future = new EFutureTask<EInteger>(callable);
		sp<EThread> thread = new EThread(future);
		thread->start();
		try {
			EThread::sleep(50);//
			LOG("i=%d", future->get()->intValue());
		} catch (EInterruptedException& e) {
			e.printStackTrace();
		} catch (EExecutionException& e) {
			e.printStackTrace();
		}
	}

	//3.
	{
		EExecutorService* executorService = EExecutors::newCachedThreadPool();
		ECompletionService<EInteger> *cs = new EExecutorCompletionService<EInteger>(executorService, new ELinkedBlockingQueue<EFuture<EInteger> >());

		cs->submitX([]()->sp<EInteger>{
			EThread::sleep(50);//
			return new EInteger(999);
		});

		try {
			sp<EFuture<EInteger> > f = cs->poll();
			if (f != null) {
				LOG("i=%d", f->get()->intValue());
			}
		} catch (EInterruptedException& e) {
			e.printStackTrace();
		} catch (EExecutionException& e) {
			e.printStackTrace();
		}

		executorService->shutdown();
		executorService->awaitTermination();

		delete executorService;
		delete cs;
	}

	LOG("end of test_executors().");
}

static void test_socketpair() {
#ifndef WIN32
	int raw_socks[2];
	::socketpair(AF_UNIX, SOCK_STREAM, 0, raw_socks);

	sp<EFork> process = EFork::executeX([&]() {
		LOG("child process.");

		sp<ESocket> s = ESocket::createFromFD(raw_socks[0], true, true);
		EOutputStream* os = s->getOutputStream();
		os->write("1234567890");

		ESystem::exit(0);
	});
	EThread::sleep(100);
	sp<ESocket> s = ESocket::createFromFD(raw_socks[1], true, true);
	EInputStream* os = s->getInputStream();
	char buf[512];
	os->read(buf, sizeof(buf));
	LOG("read: %s", buf);
	process->waitFor();
	LOG("parent process.");
#endif
}

MAIN_IMPL(testc11) {
	printf("main()\n");

	ESystem::init(argc, argv);

	LOG("inited.");

	do {
		try {
//			test_scopeExit();
//			test_finally();
//			test_threadx();
			test_timerx();
//			test_executors();
//			test_socketpair();
		} catch (EException& e) {
			LOG("exception: %s", e.getMessage());
		} catch (...) {
			LOG("Catched a exception.");
		}
	} while (1);

	LOG("exit...");

	ESystem::exit(0);

	return 0;
}

#endif
