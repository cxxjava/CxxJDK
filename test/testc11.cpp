#include "es_main.h"
#include "Efc.hh"

#define LOG(fmt,...) ESystem::out->printfln(fmt, ##__VA_ARGS__)

#ifdef CPP11_SUPPORT

#ifndef WIN32
#include <mutex>
#endif

static void thread_worker()
{
    LOG("thread_worker running...");
}

static void thread_worker2(int v)
{
    LOG("thread_worker2 [v=%d] running...", v);
}

#define LOCKED_WORK_THREADS 10
#define MAX_LOCKED_COUNTE 10000000
static long locked_counter = 0L;
static EReentrantLock locked_counter_lock;
#ifndef WIN32
static std::mutex locked_counter_stdmutex;
static pthread_mutex_t locked_counter_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
static void test_lock_benchmark() {
	class Thread: public EThread {
	public:
		virtual void run() {
			while (true) {
				//1. use ELockPool
				//mac: 10 thread run 10000000 times, cost 1494 ms
				//linux: 10 thread run 10000000 times, cost 1737 ms
				//linux2: 10 thread run 10000000 times, cost 3554 ms/1 thread run 10000000 times, cost 160 ms/2 thread run 10000000 times, cost 2582 ms
				if (0) {
					SCOPED_SLOCK5(&locked_counter) {
						locked_counter++;
						if (locked_counter > MAX_LOCKED_COUNTE) {
							break;
						}
					}}
				}

				//2. use EReentrantLock
				//mac: 10 thread run 10000000 times, cost 3276 ms
				//linux: 10 thread run 10000000 times, cost 1512 ms
				//linux2: 10 thread run 10000000 times, cost 1588 ms/1 thread run 10000000 times, cost 595 ms/2 thread run 10000000 times, cost 6031 ms
				if (1) {
					SYNCBLOCK(&locked_counter_lock) {
						locked_counter++;
						if (locked_counter > MAX_LOCKED_COUNTE) {
							break;
						}
					}}
				}

#ifndef WIN32
				//3. use std::mutex
				if (0) {
					std::lock_guard<std::mutex> lock(locked_counter_stdmutex);
					locked_counter++;
					if (locked_counter > MAX_LOCKED_COUNTE) {
						break;
					}
				}

				//4. pthread mutex
				//mac: 10 thread run 10000000 times, cost 46415 ms
				//linux: 10 thread run 10000000 times, cost 975 ms
				//linux2: 10 thread run 10000000 times, cost 1096 ms/1 thread run 10000000 times, cost 241 ms/2 thread run 10000000 times, cost 1047 ms
				if (0) {
					pthread_mutex_lock( &locked_counter_mutex );
					locked_counter++;
					if (locked_counter > MAX_LOCKED_COUNTE) {
						pthread_mutex_unlock( &locked_counter_mutex );
						break;
					}
					pthread_mutex_unlock( &locked_counter_mutex );
				}
#endif
			}
		}
	};

	llong t1 = ESystem::currentTimeMillis();

	EArrayList<sp<Thread> > arrs;
	for (int i=0; i<LOCKED_WORK_THREADS; i++) {
		sp<Thread> t = new Thread();
		arrs.add(t);
		t->start();
	}

	for (int i=0; i<LOCKED_WORK_THREADS; i++) {
		arrs.getAt(i)->join();
	}

	llong t2 = ESystem::currentTimeMillis();

	LOG("%d thread run %ld times, cost %ld ms\nper second op times: %f",
			LOCKED_WORK_THREADS, MAX_LOCKED_COUNTE, t2 - t1, ((double)MAX_LOCKED_COUNTE)/(t2-t1)*1000);
}

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

	auto ths1 = EThread::executeX([&]() {
		//EThread::sleep(1000); //if wait after notify then will blocked forever.
		SYNCHRONIZED(&sync) {
			sync.wait();
		}}
	});
//	ths1->start(); //exception: Already started.

	auto ths2 = EThread::executeX([&]() {
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

	auto ths3 = EThread::executeX(thread_worker);
	ths3->join();

	auto ths4 = EThread::executeX(std::bind(thread_worker2, 4));
	ths4->join();

	sp<EThread> ths5 = new EEThreadTarget([](){
		EThread* t = EThread::currentThread();
		LOG("%s running...", t->getName());
	});
	ths5->setName("thread5");
	ths5->start();
	ths5->join();

	LOG("end of test_threadx().");
}

static void test_timerx() {
	ETimer timer;

	auto task = timer.scheduleX((llong)0, 100, []() {
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

static void test_foreach() {
#if !defined(_MSC_VER) || (_MSC_VER>=1800) //VS2013
	EArray<EString*> arr;
	arr.add(new EString("1"));
	arr.add(new EString("2"));
	arr.add(new EString("3"));
	for (auto s : arr) {
		LOG("s=%s", s->c_str());
	}

	EConcurrentHashMap<int, EInteger> hm;
	hm.put(1, new EInteger(1));
	hm.put(2, new EInteger(2));
	hm.put(3, new EInteger(3));
	for (auto i : *hm.entrySet()) {
		LOG("i=%d", i->getValue()->intValue());
	}
#endif
}

static void test_initializer_list() {
#if !defined(_MSC_VER) || (_MSC_VER>=1800) //VS2013
	EA<int> arr = {1,2,3};

	for (int i=0; i<arr.length(); i++) {
		LOG("i=%d", arr[i]);
	}
	LOG("===");
	sp<EIterator<int> > iter = arr.iterator(1);
	while (iter->hasNext()) {
		LOG("i=%d", iter->next());
	}
	LOG("===");
	for (int i : arr) {
		LOG("i=%d", i);
	}
	LOG("===");

	EA<EString> arr2 = {"a", "b", "c"};
	arr2.setAt(0, "aa");
	for (EString s : arr2) {
		LOG("s=%s", s.c_str());
	}
	LOG("===");

//	EA<EString*> arr3 = {new EString("a"), new EString("b"), new EString("c")};
	EA<EString*> arr3 = {"a", "b", "c"};
	arr3.setAt(0, "aa");
	for (EString* s : arr3) {
		LOG("s=%s", s->c_str());
	}
	LOG("===");

//	EA<sp<EString>> arr4 = {new EString("a"), new EString("b"), new EString("c")};
	EA<sp<EString>> arr4 = {"a", "b", "c"};
	arr4.setAt(0, "aa");
	for (auto s : arr4) {
		LOG("s=%s", s->c_str());
	}
	LOG("===");
#endif
}

MAIN_IMPL(testc11) {
	printf("main()\n");

	ESystem::init(argc, argv);

	LOG("inited.");

	do {
		try {
//			test_lock_benchmark();
//			test_scopeExit();
//			test_finally();
//			test_threadx();
//			test_timerx();
//			test_executors();
//			test_socketpair();
//			test_foreach();
			test_initializer_list();
		} catch (EException& e) {
			LOG("exception: %s", e.getMessage());
		} catch (...) {
			LOG("Catched a exception.");
		}
	} while (0);

	LOG("exit...");

	ESystem::exit(0);

	return 0;
}

#endif
