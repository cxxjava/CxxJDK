
#define TEST_BOOST_SHARED_PTR  0  //0-efc::shared_ptr | 1-boost::shared_ptr
#define _S(s) #s

#if TEST_BOOST_SHARED_PTR
#include <boost/shared_ptr.hpp> //for test boost::shared_ptr
#define spnamespace boost
#else
#define spnamespace efc
#define shared_ptr sp
#endif
//#include <memory> //for test std::shared_ptr

#include "es_main.h"
#include "Efc.hh"

#define LOG(fmt,...) ESystem::out->printfln(fmt, ##__VA_ARGS__)

namespace efc {
es_alogger_t* alogger;
}
#define ALOG(fmt,...) {if (alogger) {eso_alogger_logfmt(alogger, fmt, ##__VA_ARGS__);}}

static EAtomicLLong gLockTestCount1;
static EAtomicLLong gLockTestCount2;

static EAtomicLLong gSumAdd;
static EAtomicLLong gSumPull;

static llong gDeadlineTimestamp = 0L;

class Count {
	static volatile int count1;// = 1;
	static volatile int count2;// = 1;

	public:
	int getcount1() {
		return count1;
	}

	int getcount2() {
		return count2;
	}

	void addcount1() {
		eso_atomic_add_and_fetch32(&count1, 1);
	}

	void addcount2() {
		eso_atomic_add_and_fetch32(&count2, 1);
	}
};
volatile int Count::count1 = 1;
volatile int Count::count2 = 1;

static void test_null(void) {
	EInteger* i1 = NULL;
	if (i1 == null) {
		LOG("i1 == null");
	}
	EInteger* i2 = null;
	if (i2 == NULL) {
		LOG("i2 == NULL");
	}

	if (null == NULL) {
		LOG("null == NULL");
	}
	if (null != NULL) {
		LOG("null != NULL");
	}
	if (NULL == null) {
		LOG("NULL == null");
	}
	if (NULL != null) {
		LOG("NULL != null");
	}
}

static void test_interface() {
	interface IA : virtual public EObject {
		virtual void func() = 0;
	};
	class A : virtual public IA {
	public:
		void func() {
			LOG("A::func()");
		}
		virtual int hashCode() {
			return 1;
		}
		virtual boolean equals(EObject* obj) {
			return true;
		}
	};
	class AA : virtual public EObject, public A {
	public:
		virtual int hashCode() {
			return 2;
		}
		virtual boolean equals(EObject* obj) {
			return false;
		}
	};

	AA aa1, aa2;
	if (aa1.equals(&aa2)) {
		LOG("aa1==aa2");
	}

	LOG("aa1.toString():%s", aa1.toString().c_str());

	A *a1, *a2;
	a1 = &aa1;
	a2 = &aa2;
	if (a1->equals(a2)) {
		LOG("a1==a2");
	}
}

static void test_vc6bug() {
	//class A {
	//};
	class AAAA : public EThread  {
	public:
		AAAA() {

		}
		virtual void run() {

		}
	};
}

template <typename T>
class TraitTest {
public:
	TraitTest(T t): t_(t) {
		LOG("111 - %s", t_.c_str());
	}
	T& get() {
		return t_;
	}
private:
	T t_;
};

template <typename T>
class TraitTest<T*> {
public:
	TraitTest(T* t): t_(t) {
		LOG("222 - %s", t_->c_str());
	}
	T* get() {
		return t_;
	}
private:
	T* t_;
};

template <typename T>
class TraitTest<sp<T> > {
public:
	TraitTest(sp<T> t): t_(t) {
		LOG("333 - %s", t_->c_str());
	}
	sp<T> get() {
		return t_;
	}
private:
	sp<T> t_;
};

static void test_traits() {
	TraitTest<EString*> test1(new EString("p"));
	delete test1.get();
	TraitTest<EString> test2(EString::valueOf("str"));
	TraitTest<sp<EString> > test3(new EString("sp"));
}

static void f_string(const EString& s) {
	LOG("s=%s", s.c_str());
}

static void test_string() {
	{
		EString s("0123456789");
		int n = s.indexOf('\0');
		LOG("n=%d", n);
		ES_ASSERT(n==-1);
		n = s.lastIndexOf('\0');
		LOG("n=%d", n);
		ES_ASSERT(n==-1);

		n = s.indexOf("\0");
		LOG("n=%d", n);
		ES_ASSERT(n==-1);
		n = s.lastIndexOf("\0");
		LOG("n=%d", n);
		ES_ASSERT(n==-1);
	}

	{
		ullong v = -1;
		EString vs(v);
		LOG(vs.c_str());
	}

	LOG((EString("xxx") + 2 + '\t' + "xxxxxxxxx").c_str());

	//string join
	{
		EString x_y("xxx" + EString("|") + 10 + "yyy");
		LOG(x_y.c_str());
		x_y = "ccc" + EString("yyy");
		LOG(x_y.c_str());
		x_y = EString("yyy") + "ccc";
		LOG(x_y.c_str());
		x_y = EString("yyy") + 9;
		LOG(x_y.c_str());
		x_y = EString("yyy") + 'c';
		LOG(x_y.c_str());
		EString x_y2("222 | ");
		x_y2 << &x_y << " | ";
		x_y2 += &x_y;
		EString xx_yy(x_y + "| " + &x_y2);
		LOG(xx_yy.c_str());

		EString s_1("s");
		s_1 << 'x';
		s_1 << 10;
		EString s_2 = s_1 + 'y' + 9;
		LOG(s_2.c_str());
	}

	//as hex
	{
		byte buf[10];
		eso_memcpy(buf + 5, "\5", 1);
		EString hex = EString::toHexString(buf, sizeof(buf));
		LOG("hex=%s", hex.c_str());
	}

	f_string("sssssssssssssss");

	EString str("127.0.0.1");

	{
		EString x1("x");
		x1<<"y"<<1020;
		LOG("x1=%s", x1.c_str());
	}

	int i;

	//1.
	{
		EArray<EString*> arr = EPattern::split("\\.", str.c_str(), 0);
		LOG("arr.size=%d", arr.size());
		for (i=0; i<arr.size(); i++) {
			LOG("s=%s", arr[i]->c_str());
		}

		EString a = str.splitAt(".", 1);
		LOG("a=%s", a.c_str());
		EString b = str.splitAt(".", 4);
		LOG("b=%s", b.c_str());

		EPattern pattern("\\.");
		EArray<EString*> arr2 = pattern.split(str.c_str());
		LOG("arr2.size=%d", arr2.size());
		for (i = 0; i < arr2.size(); i++) {
			LOG("s=%s", arr2[i]->c_str());
		}
	}

	//2.
	{
		EString str2("AAAAAAAAA");
		str << "++++++" << "abc" << str2 << 'X';
		LOG("s=%s", str.c_str());
	}

	//3.
	{
		str.append(88823728, 16);
		LOG("s=%s", str.c_str());
		str.append(9.88327832);
		LOG("s=%s", str.c_str());

		EString s = "abcd'efg +++fdjska";
		s = s.substring(0, 10);
		int n = s.lastIndexOf("cd", 2);
		LOG(s.substring(n).c_str());

		boolean r = s.regionMatches(true, 2, "Cd", 0, 2);
		if (r) {
			LOG("matched 'Cd'!");
		}
	}

	//4.
	{
		EString x("xxx");
		for (int i=0; i<10000; i++) {
			x.append("xxxxxx");
		}
	}

	//5. null
	{
		EString n5(null);
		LOG("n5=%s", n5.c_str());
	}

	//6. pointer
	{
		EString s = "x";
		EString p6(&s);
		LOG("p6=%s", p6.c_str());
	}
}

static void test_simpleMap() {
	ESimpleMap map(true, false);
	map.put("1", new EString("aaa"));
	map.put("1", new EString("bbb"));
	map.put("1", new EString("ccc"));
	map.put("2", new EString("ddd"));

	ESimpleEnumeration* e = map.elements();
	for (; e && e->hasMoreElements();) {
		es_emap_elem_t elem;
		e->nextElement(&elem);
		EString* s = dynamic_cast<EString*>(elem.data);
		LOG("s=%s", s->c_str());
	}

	EString* aaa = dynamic_cast<EString*>(map.get("1"));
	LOG("1=%s", aaa->c_str());
	EString* bbb = dynamic_cast<EString*>(map.get("1", 1));
	LOG("1=%s", bbb->c_str());
	EString* ccc = dynamic_cast<EString*>(map.get("1", -1));
	LOG("1=%s", ccc->c_str());
}

static void test_random() {
	ERandom random;
	int n = random.nextInt();
	LOG("nextInt %d", n);
	n = random.nextInt();
	LOG("nextInt %d", n);
	n = random.nextInt();
	LOG("nextInt %d", n);
	n = random.nextInt();
	LOG("nextInt %d", n);
	n = random.nextInt();
	LOG("nextInt %d", n);
	n = random.nextInt();
	LOG("nextInt %d", n);
	n = random.nextInt();
	LOG("nextInt %d", n);

	double d = random.nextDouble();
	LOG("nextDouble %.20lf", d);

	EA<byte> bb1(10);
	random.nextBytes(&bb1);
	for (int i=0; i<bb1.length(); i++) {
		printf(" %d ", bb1.getAt(i));
	}
	LOG("");
	byte bb2[10] = {0};
	random.nextBytes(bb2, sizeof(bb2));
	for (int i=0; i<sizeof(bb2); i++) {
		printf(" %d ", bb2[i]);
	}
	LOG("");
}

static void test_secureRandom() {
	ESecureRandom sr;
	double d = sr.nextDouble();
	LOG("d=%lf", d);
}

static void test_lock(int flag) {
	//1. ESynchronizeable
	class A : public ESynchronizeable
	{
	public:
		void testLock(int flag) {
			for (int i=0; i<10; i++) {
				SYNCHRONIZED(this) {
					printf("SYNCHRONIZED 1 ##%d\n", flag);
					SYNCHRONIZED(this) {
						printf("SYNCHRONIZED 2 ##%d\n", flag);

						//if (i == 5) continue; //!
						if (i == 5) break; //!
						printf("i=%d\n", i);
					}}
				}}
			}

			printf("end of testLock()\n");
		}
	};

	A a;
	a.testLock(flag);

	printf("\n");

	//2. EReentrantLock
	ELock* lock = new EReentrantLock();
	lock->lock();
	printf("lock1\n");
	lock->lock();
	printf("lock2\n");
	lock->unlock();
	lock->unlock();
	delete lock;

	printf("\n");

	//3. ESpinLock
	lock = new ESpinLock();
	lock->lock();
	printf("lock1\n");
#if 0
	lock->lock(); //blocked!
	printf("lock2\n");
	lock->unlock();
#endif
	lock->unlock();
	delete lock;

	printf("test_lock(int) end.\n");
}


class ErrHandler: public EThread::UncaughtExceptionHandler {
public:
	void uncaughtException(EThread* t, EThrowable* e) {
		LOG("This is:%s,Message:%s", t->getName(), e->getMessage());
		e->printStackTrace();
	}
};

class XXX : public ESynchronizeable {
public:
	void printf() {
//		LOG("xxx");
	}
};

static void test_thread() {
	class TestThread: public EThread {
	public:
		TestThread(const char* name) : EThread(name) {
			v = 0;
		}
		virtual ~TestThread() {
			LOG("TestThread::~TestThread()...");
		}
		virtual void run() {
			while (v < 10) {
				EThread* current = EThread::currentThread();
				if (current != this) {
					printf("current thread: %s\n", current->getName());
				}

				llong n = gLockTestCount1.incrementAndGet();
				if (n % 999999 == 0) {
					printf("TestThread_%s....%lld.\n", getName(), n);
				}

				v++;
			}
//			printf("this thread: %s\n", this->getName());
		}
	private:
		int v;
	};

//	printf("test_thread start.\n");

#if 0
	TestThread t1("#1"), t2("#2"), t3("#3");
	t1.start();
	t2.start();
	t3.start();

	t1.join();
	t2.join();
	t3.join();
#else
	EArray<EThread*> arr;
	int i;
	for (i = 0; i < 1000; i++) {
		TestThread* pct = new TestThread(EString(i).c_str());
		pct->start();
		arr.add(pct);
	}

	for (i = 0; i < arr.length(); i++) {
		arr.getAt(i)->join();
	}
#endif

//	printf("test_thread end.\n");
}

class XX : public EThreadLocal {
public:
    ~XX() {
        //			removeAll();
    }
protected:
    void destructor(EInteger* v) {
        delete v;
    }
};

class TestThread: public EThread
{
public:
	static EReentrantLock *lock_;
	ELock *lock2_;

public:
	TestThread(const char *name, EThreadLocalVariable<XX, EInteger>* threadLocal1, EThreadLocalVariable<XX, EInteger>* threadLocal2, ELock* lock) : EThread(name){
		v = 0;

		this->threadLocal1 = threadLocal1;
		this->threadLocal2 = threadLocal2;

		this->lock2_ = lock;
	}
	virtual ~TestThread() {
	}

	virtual void run() {
		try {
			delete threadLocal1->set(new EInteger(1));
			delete threadLocal2->set(new EInteger(8888888));

			while(v < 100) {
				EThread* current = EThread::currentThread();
//				LOG("current thread: #%d, %s", eso_os_thread_current(), current->getName());

				if (1) {
//					SYNCBLOCK(TestThread::lock_) {
//						ECondition* cond_ = TestThread::lock_->newCondition();
//						delete cond_;
//					}}

					SYNCBLOCK(lock2_) {
						//
					}}

//					XXX xxx;
//					SYNCHRONIZED(&xxx) {
//						try {
//						SYNCHRONIZED(&xxx) {
//							xxx.printf();
//						}}
//						} catch(...) {
//						}
//					}}
				}

				//====

#if 0
#if 1
LOG("thread#%s v=%d", this->getName(), v++);
				if (v%12 == 1 && eso_strcmp(this->getName(), "#1")==0) {
					LOG("thread#%s sleep begin...", this->getName());
					EThread::sleep(2000);
					LOG("thread#%s sleep end.", this->getName());
				}
#else
				test_lock((int)EThread::currentThread());
#endif
#else
				EInteger* i = threadLocal1->get();
				i->value++;
//				LOG("thread#%d i1=%d", eso_os_thread_current(), i->intValue());
				delete threadLocal1->set(new EInteger(i->value));

				EInteger* i2 = threadLocal2->get();
				i2->value++;
//				LOG("thread#%d i2=%d", eso_os_thread_current(), i2->intValue());

//				long c = threadLocal2->get();
//				threadLocal2->set(++c);
//				LOG("thread#%d c=%d", eso_os_thread_current(), c);

				llong n = gLockTestCount1.incrementAndGet();
				if (n % 999999 == 0) {
					printf("XXX_%s....%d.\n", this->getName(), v);
				}

				v++;
#endif
			}

//			threadLocal1->remove(); //!free this thread local object.
		} catch (EThrowable& e) {
			e.printStackTrace();
			throw e;
		}

//		printf("end of thread.\n");
	}

private:
	int v;

	EThreadLocalVariable<XX, EInteger>* threadLocal1;
	EThreadLocalVariable<XX, EInteger>* threadLocal2;
};
EReentrantLock* TestThread::lock_ = new EReentrantLock();

static void test_thread1() {
	EThreadLocalVariable<XX, EInteger>* threadLocal1 = new EThreadLocalVariable<XX, EInteger>();
	EThreadLocalVariable<XX, EInteger>* threadLocal2 = new EThreadLocalVariable<XX, EInteger>();
//	{
	ErrHandler handle;
	EThread::setDefaultUncaughtExceptionHandler(&handle);


	EReentrantLock lock;

#if 0
	TestThread t1("#1", threadLocal1, threadLocal2, &lock), t2("#2", threadLocal1, threadLocal2, &lock), t3("#3", threadLocal1, threadLocal2, &lock);
	t1.setUncaughtExceptionHandler(&handle);
	t1.start();
	t2.start();
	t3.start();

	t1.join();
	t2.join();
	t3.join();
#else
	EArray<EThread*> arr;
	int i;
	for (i = 0; i < 5; i++) {
		TestThread* pct = new TestThread(EString(i).c_str(), threadLocal1, threadLocal2, &lock);
		pct->start();
		arr.add(pct);
	}

	for (i = 0; i < arr.length(); i++) {
		arr.getAt(i)->join();
	}
#endif

//	TestThread t1("#1");
//	t1.start();
//	t1.join();
//	}
	delete threadLocal1;
	delete threadLocal2;

//	LOG("after thread join.");
}

static void test_thread2() {
	class AAAA : public EThread  {
	public:
		AAAA() {

		}
		virtual void run() {
			for (int i=0; i<100; i++) {
				LOG("i=%d", i);
				EThread::sleep(10);
			}
		}
	};

	sp<AAAA> t = new AAAA();
	EThread::setDaemon(t, true); //success!
	t->start();
	//EThread::setDaemon(t, true); //error!

	t->join();
//	EThread::sleep(10);

	LOG("end of test_thread2()");
}

static void test_thread3() {
	struct X {
		EReentrantLock mainLock;
//		ESimpleLock mainLock;
		ECondition* termination;
		EAtomicCounter aliveCount0;
		EAtomicCounter aliveCount1;
		volatile int state; //0=init | 1-shutdown | 2-terminated

		X() {
			termination = mainLock.newCondition();
		}

		~X() {
			delete termination;
		}
	} x;

	class AAAA : public EThread  {
	private:
		X& x;
	public:
		AAAA(X& x) : x(x) {

		}
		virtual ~AAAA() {
			LOG("AAAA::~AAAA()...t=%p", this);
		}
		virtual void run() {
			x.aliveCount1++;

			//			for (int i=0; i<100000; i++) {
////				printf("i=%d\n", i);
////				EThread::sleep(10000);
//			}

			LOG(EString::formatOf("thread %p finished", this).c_str());
		}

		static void test_thread3_exit_callback(void* arg) {
			AAAA *t = static_cast<AAAA *>(arg);

			LOG("thread %p, t->name=%s", t, t->getName());

			SYNCBLOCK(&t->x.mainLock) {
				t->x.aliveCount0--;
				t->x.aliveCount1--;
				int m = t->x.aliveCount0.value();
				int n = t->x.aliveCount1.value();
				if (t->x.state == 1 && m == 0 && n == 0) {
						t->x.state = 2;
						t->x.termination->signalAll();
//						printf("signalAll()...\n");
				}
//				printf("state=%d, m=%d, n=%d\n", t->x.state, m, n);
			}}

//			EThread::sleep(500);
		}
	};

	EArrayList<AAAA*> arr;

	x.state = 0;
	for (int i=0; i<50; i++) {
		AAAA *t = new AAAA(x);
		arr.add(t);
		t->injectExitCallback(AAAA::test_thread3_exit_callback, t);
		x.aliveCount0++;
		t->start();
	}

	SYNCBLOCK(&x.mainLock) {
		x.state = 1;
		for (;;) {
			if (x.aliveCount0.value() == 0) {
				break;
			}
			x.termination->await();
		}
	}}

	for (int i=0; i<arr.size(); i++) {
		AAAA* t = arr.getAt(i);
		t->join();
	}

	llong n = gLockTestCount1.incrementAndGet();
	if (n >= 4096) {
		EThread::sleep(10);
	}

	LOG("end of test_thread3()...");
}

static void test_thread4() {
	struct X {
		EReentrantLock mainLock;
//		ESimpleLock mainLock;
		ECondition* termination;
		EAtomicCounter aliveCount0;
		EAtomicCounter aliveCount1;
		volatile int state; //0=init | 1-shutdown | 2-terminated

		X() {
			termination = mainLock.newCondition();
		}

		~X() {
			delete termination;
		}
	} x;

	class AAAA : public EThread  {
	private:
		X& x;
	public:
		AAAA(X& x) : x(x) {

		}
		virtual ~AAAA() {
			LOG("AAAA::~AAAA()...t=%p", static_cast<EThread*>(this));
		}
		virtual void run() {
			x.aliveCount1++;

//			for (int i=0; i<100000; i++) {
////				printf("i=%d\n", i);
////				EThread::sleep(10000);
//			}

			LOG(EString::formatOf("thread %p finished", static_cast<EThread*>(this)).c_str());

		}

		static void test_thread3_exit_callback(void* arg) {
			AAAA *t = static_cast<AAAA *>(arg);

			LOG("thread %p, t->name=%s", t, t->getName());

			SYNCBLOCK(&t->x.mainLock) {
				t->x.aliveCount0--;
				t->x.aliveCount1--;
				int m = t->x.aliveCount0.value();
				int n = t->x.aliveCount1.value();
				if (t->x.state == 1 && m == 0 && n == 0) {
						t->x.state = 2;
						t->x.termination->signalAll();
//						printf("signalAll()...\n");
				}
//				printf("state=%d, m=%d, n=%d\n", t->x.state, m, n);
			}}

//			EThread::sleep(500);
		}
	};

	x.state = 0;
	for (int i=0; i<5; i++) {
		AAAA* t = new AAAA(x);
		t->injectExitCallback(AAAA::test_thread3_exit_callback, t);
		EThread::setDaemon(t, true);
		x.aliveCount0++;
		t->start();
	}

	SYNCBLOCK(&x.mainLock) {
		x.state = 1;
		for (;;) {
			if (x.aliveCount0.value() == 0) {
				break;
			}
			x.termination->await();
		}
	}}

	EThread::sleep(500); //FIXME: wait all thread destroyed.

	llong n = gLockTestCount1.incrementAndGet();
	if (n >= 4096) {
		EThread::sleep(10);
	}

	LOG("end of test_thread4()...");
}

static void test_thread5() {
	class MyThread : public EThread, public enable_shared_from_this<MyThread> {
	public:
		virtual ~MyThread() {
			LOG("%012s", getName());
		}
		virtual void run() {
			sp<MyThread> self = shared_from_this();

			class MyThread2 : public EThread {
			public:
				sp<MyThread> self;
				virtual ~MyThread2() {
					LOG("%012s", getName());
				}
				MyThread2(sp<MyThread> s) : self(s) {
				}
				virtual void run() {
					self->getName();
				}
			};
			sp<MyThread2> thread = new MyThread2(self);

			thread->setName(EString(gSumAdd.addAndGet(1)).c_str());
			EThread::setDaemon(thread, true); //!!!
			thread->start();
		}
	};

	sp<MyThread> ths1 = new MyThread();
	ths1->setName(EString(gSumAdd.addAndGet(1)).c_str());
	EThread::setDaemon(ths1, true); //!!!
	ths1->start();

	LOG("end of test_thread5().");
}

static void test_threadJoin() {
	class T1 : public EThread {
	public:
		virtual void run() {
//			EThread::sleep(2000);
			LOG("t1 running...");
		}
	};

	T1 t1;
	t1.start();

	class T2 : public EThread {
	public:
		T2(T1* t1) {
			this->t1 = t1;
		}

		virtual void run() {
//            EThread::sleep(500);
			t1->join();
			LOG("after join at t2");
		}
	private:
		T1* t1;
	};
	T2 t2(&t1);
	t2.start();

	t1.join();
	t2.join();

//	printf("t1 cv=%d\n", t1.parkCount.value());
//	printf("t2 cv=%d\n", t2.parkCount.value());

	LOG("after join at main");
}

static void test_threadState() {
	class MyThread: public EThread {
	private:
		ELock* lock;
	public:
		MyThread(ELock* lock) {
			this->lock = lock;
		}
		virtual void run() {
			try {
				LOG("sleep...");
				EThread::sleep(1000);
				LOG("...");

				lock->lock();
				LOG("get lock.");
				lock->unlock();

			} catch (EInterruptedException& e) {
				LOG("%s%s", EThread::currentThread()->getName(),
						" interrupted.");
			}
		}
	};

	EReentrantLock* lock = new EReentrantLock();
	EThread *t1 = new MyThread(lock);
	t1->start();
	LOG("thread stat=%d", t1->getState());

	lock->lock();
	EThread::sleep(10);
	LOG("thread stat0=%d", t1->getState());
	EThread::sleep(2000);
	LOG("thread stat1=%d", t1->getState());
	EThread::sleep(2000);
	t1->interrupt();
	LOG("thread stat2=%d", t1->getState());
	EThread::sleep(2000);
	LOG("thread stat3=%d", t1->getState());
	lock->unlock();

	t1->join();
	delete t1;
	delete lock;
}

/**
 * EThrowable wrapped for based EObject.
 */
class ThrowableType : public virtual EObject {
};

template<typename E>
class ThrowableObject: public ThrowableType {
public:
	ThrowableObject(const E& t) : throwable(t) {
	}

	ThrowableObject(E& t) : throwable(t) {
	}

	ThrowableObject(E* t) : throwable(*t) {
	}

	/**
	 * Get the binded EThrowable object.
	 */
	E* getThrowable() {
		return &throwable;
	}

private:
	E throwable;
};

static void test_exception()
{
	class MyException : public EException {
	public:
		EString xxx;
		virtual ~MyException() {
			LOG("MyException::~MyException()");
		}
		MyException(const char *_file_, int _line_, const char *s) :
			EException(_file_, _line_, s) {
			xxx = "MyException";
		}
//		virtual EThrowable* clone() {
//			return new MyException(*this);
//		}
	};

	LOG("===============");

	{
		EIOException ioe(__FILE__, __LINE__, "11111111");
		EIOException ioe2(__FILE__, __LINE__, "222222222", &ioe);
		LOG("exception msg=%s", ioe2.toString().c_str());
		EIOException ioe3(ioe2);
		LOG("exception msg=%s", ioe3.toString().c_str());
		EFileNotFoundException fbfe(__FILE__, __LINE__, "FileNotFoundException");
		EThrowable* t = ioe3.initCause(&fbfe);
		ES_ASSERT(t == null);
		LOG("exception msg=%s", ioe3.toString().c_str());
		ioe3.printStackTrace();
		EDataFormatException dfe(__FILE__, __LINE__);
		t = dfe.initCause(&fbfe);
		ES_ASSERT(t);
		LOG("exception msg=%s", dfe.toString().c_str());
		dfe.printStackTrace();
//		EFileNotFoundException ioe4(ioe3);
//		LOG("exception msg=%s", ioe4.toString().c_str());
	}

	LOG("===============");

	try {
		MyException me(__FILE__, __LINE__, "ThrowableObject");
		ThrowableObject<MyException> to(me);
		throw to;
	} catch (EObject& t) {
		ThrowableType* to = dynamic_cast<ThrowableType*>(&t);
//		ThrowableObject<MyException>* to = dynamic_cast<ThrowableObject<MyException>*>(&t);
		if (to) {
			ThrowableObject<MyException>* tt = dynamic_cast<ThrowableObject<MyException>*>(&t);
			LOG(tt->getThrowable()->toString().c_str());
		}
	}

	LOG("===============");

	EFileOutputStream sm("/tmp/test_file.txt");
	try {
		try {
			MyException e(__FILE__, __LINE__, "xxx");
			EThrowable* t = new EThrowable(e);
			throw t;
//			throw *(EException*)t;
		} catch (MyException& e) {
			LOG(e.getMessage());
			LOG(e.getStackTrace());
		} catch (MyException* e) {
			LOG(e->getMessage());
			LOG(e->getStackTrace());
		} catch (EThrowable* t) {
			LOG(t->getStackTrace());
			MyException* e = dynamic_cast<MyException*>(t);
			if (e) {
				LOG(e->getMessage());
				LOG(e->xxx.c_str());
			}
			delete t;
		}
		LOG("===============");

		try {
//			throw EIllegalArgumentException(__FILE__, __LINE__, "xx");
			throw EIllegalArgumentException(__FILE__, __LINE__);
		} catch (EIllegalArgumentException &e0) {
			LOG(e0.toString().c_str());
			LOG(e0.getStackTrace());
//			e0.printStackTrace();
//			throw e0;

			EIllegalArgumentException e2(e0);
			LOG(e2.getStackTrace());
		}

		EPrintStream ps(&sm);
		ps.printfln("don't support this mode(%s) for memory stream!", "X");
		ps.write((void*) "jjjjjjjjjjjjjjjjjjjjjxxxg", 25);
		sm.close();
	} catch (EIOException &e) {
		LOG("xxxxxxxxxxxxxxxxxxxxxxxxxxxx");
		printf("errno=%d, errmsg=%s\n", e.getErrno(), e.getMessage());
		e.printStackTrace();
	} catch (EIllegalArgumentException &e2) {
		LOG(e2.toString().c_str());
	} catch (...) {
		LOG("catch all...");
	}
}

class LockConditionTest : public EObject {
public:
	LockConditionTest();
	virtual ~LockConditionTest();

	void put();

	EString* take();

private:
	ELock *lock;
	ECondition *fullCondition;
	ECondition* emptyCondition;
	int maxSize;

	ESimpleStack *bag;

public:
	long number;
};

class Producer: public ERunnable {
public:
	Producer(LockConditionTest *test) {
		m_parent = test;
	}
	virtual ~Producer() {

	}
	virtual void run() {
		try {
			int i = 0;
			do {
				m_parent->put();
//				printf("p\n");
			} while (i++ < 999);
		} catch (EException& e) {
			e.printStackTrace();
			throw e;
		}
	}
private:
	LockConditionTest *m_parent;
};

class Customer: public ERunnable {
public:
	Customer(LockConditionTest *test) {
		m_parent = test;
	}
	virtual ~Customer() {

	}
	virtual void run() {
		do {
			EString* s = m_parent->take();
			if (!s) break;
			delete s;
//			printf("c\n");
		} while (1);
	}
private:
	LockConditionTest *m_parent;
};

LockConditionTest::LockConditionTest() {
	lock = new EReentrantLock();
	fullCondition = lock->newCondition();
	emptyCondition = lock->newCondition();
	maxSize = 1;
	bag = new ESimpleStack();

	number = 0;
}
LockConditionTest::~LockConditionTest() {
	delete bag;
	delete fullCondition;
	delete emptyCondition;
	delete lock;
}

static volatile int az = 0;
static volatile int pz = 0;
static volatile int tz = 0;
static volatile int zz = 0;

void LockConditionTest::put() {
	static int xx = 0;
	ALOG("PUT lock 0");
	lock->lock();
	try {
		while (bag->size() >= maxSize) {
			eso_atomic_add_and_fetch32(&pz, 1);
//			if (pz > 1) {
//				printf("pz=%d, thread=%p\n", pz, EThread::currentThread());
//			}
			fullCondition->await();
		}
		pz = 0;
//		xx++;
//		if (xx == 2) {
//		EThread::sleep(1000);
//		}
		ERandom random;
		int n = random.nextInt();
		bag->push(new EString(EInteger::toString(n)));
//		printf("push: %d,\tthread=%p\n", bag->size(), EThread::currentThread());
//		EThread::sleep(1);
		emptyCondition->signal();
	} catch (EException& ex) {
		ex.printStackTrace();
	}

//	ALOG("PUT lock 1");
//	while ( az >= 10) {
//		ALOG("PUT await 0, az=%d", az);
//		fullCondition->await();
//		ALOG("PUT await 1");
//	}
//	az++;
////	eso_atomic_add_and_fetch32(&az, 1);
////	eso_atomic_synchronize();
////	printf("az=%d, thread=%p\n", az, EThread::currentThread());
//	ALOG("PUT signal 0, az=%d", az);
//	emptyCondition->signal();
//	ALOG("PUT signal 1, unlock 0");

	llong n = gLockTestCount1.incrementAndGet();
	if (n % 999999 == 0) {
		printf("Thread1....%d.\n", n);
	}

	lock->unlock();
	ALOG("PUT unlock 1");
}

EString* LockConditionTest::take() {
	EString* result = null;

//	static int xx = 0;
//	if (xx == 0) {
//	EThread::sleep(1000);
//	}
//	xx++;

	ALOG("GET lock 0");
	lock->lock();
	ALOG("GET lock 1");
	try {
		while (bag->size() == 0) {
			eso_atomic_add_and_fetch32(&tz, 1);
//			if (tz > 1) {
//				printf("tz=%d, thread=%p\n", tz, EThread::currentThread());
//			}
			//emptyCondition->await();
			if (!emptyCondition->await(1, ETimeUnit::SECONDS)) {
				break;
			}
			zz = 0;
		}
		tz = 0;
		result = dynamic_cast<EString*>(bag->pop());
//		printf("pop  : %d,\tthread=%p\n", bag->size(), EThread::currentThread());
		zz++;
//		EThread::sleep(1);
		fullCondition->signal();
//		if (pz > 0 && zz > 1) {
//			printf("zz=%d, bag->size()=%d, thread=%p\n", zz, bag->size(), EThread::currentThread());
//		}
	} catch (EException& e) {
		e.printStackTrace();
	}

//	while (az <= 0) {
//		ALOG("GET await 0, az=%d", az);
//		emptyCondition->await();
//		ALOG("GET await 1");
//	}
//	az--;
////	eso_atomic_sub_and_fetch32(&az, 1);
////	eso_atomic_synchronize();
//	ALOG("GET signal 0");
//	fullCondition->signal();
//	ALOG("GET signal 1, unlock 0");

	llong n = gLockTestCount2.incrementAndGet();
	if (n % 999999 == 0) {
		printf("Thread2....%d.\n", n);
	}

	lock->unlock();
	ALOG("GET unlock 1");

	return result;
}

static void test_condition()
{
	class ProbeThread : public EThread {
	private:
		boolean run_;// = true;
		EArray<EThread*>* arr;

	public:
		ProbeThread(EArray<EThread*>* arr) : run_(true) {
			this->arr = arr;
		}

		virtual void run() {
			while (this->run_) {
				try {
					EThread::sleep(1000 * 2 - 1);
				} catch (EException& ex) {
					LOG("Error[ProbeThread.run]:%s", ex.getMessage());
				}

				for (int i = 0; i < arr->length(); i++) {
//					printf("t cv=%d\n", arr->getAt(i)->parkCount.value());
				}
				printf("\n");
			}
		}
	};

	EArray<EThread*> arr;

	LockConditionTest *tct = new LockConditionTest();

	//producer
	for (int i = 0; i < 200; i++) {
		ERunnable *r1 = new Producer(tct);
		EThread *t1 = new EThread(r1, "PUT");
		t1->start();
		arr.add(t1);
	}

	//customer
	for (int i = 0; i < 200; i++) {
		ERunnable *r1 = new Customer(tct);
		EThread *t1 = new EThread(r1, "GET");
		t1->start();
		arr.add(t1);
	}

//	ProbeThread* pt = new ProbeThread(&arr);
//	pt->start();

	for (int i = 0; i < arr.length(); i++) {
		arr.getAt(i)->join();
//		printf("t cv=%d\n", arr.getAt(i)->parkCount.value());
	}
//	pt->join();
//	delete pt;
	delete tct;

	printf("end of test_condition.\n");
}

static void test_number_int()
{
	int i, l;

	EInteger ll(9);
	EString s = ll.toString();
	LOG("1s=%s", s.c_str());
	llong n = EInteger::parseInt("1234567890");
	LOG("n=%lld", n);

	s  = EInteger::toOctalString(99999);
	LOG("2s=%s", s.c_str());

	s = EInteger::toUnsignedString(-323244, 10);
	LOG("3s=%s", s.c_str()); //4294644052
	s = EInteger::toUnsignedString(-323244, 9);
	LOG("3s=%s", s.c_str()); //12068115084

	ll = EInteger::decode("0x88778");
	ES_ASSERT(ll.value == 558968);
	LOG("ll=%lld", ll.value);
	ll = EInteger::decode("#88778");
	LOG("ll=%lld", ll.value);
	ll = EInteger::decode("027");
	LOG("ll=%lld", ll.value);

	i = EInteger::compareUnsigned(1, -1);
	ES_ASSERT(i == -1);
	LOG("i=%d", i);

	ll = EInteger::divideUnsigned(-995445454, 65);
	ES_ASSERT(ll.value == 50761874);
	LOG("ll=%lld", ll.value);

	ll = EInteger::remainderUnsigned(-995445454, 65);
	LOG("ll=%lld", ll.value);

	i = EInteger::numberOfLeadingZeros(43432134);
	ES_ASSERT(i == 6);
	LOG("i=%d", i);
	i = EInteger::bitCount(43432134);
	ES_ASSERT(i == 13);
	LOG("i=%d", i);
	i = EInteger::numberOfLeadingZeros(0);
	ES_ASSERT(i == 32);
	LOG("i=%d", i);
	i = EInteger::bitCount(0);
	ES_ASSERT(i == 0);
	LOG("i=%d", i);

	i = EInteger::numberOfLeadingZeros(-43432134);
	ES_ASSERT(i == 0);
	LOG("i=%d", i);
	i = EInteger::bitCount(-43432134);
	ES_ASSERT(i == 19);
	LOG("i=%d", i);

	i = EInteger::numberOfTrailingZeros(43432134);
	ES_ASSERT(i == 1);
	LOG("i=%d", i);
	i = EInteger::numberOfTrailingZeros(-43432134);
	ES_ASSERT(i == 1);
	LOG("i=%d", i);
	i = EInteger::numberOfTrailingZeros(0);
	ES_ASSERT(i == 32);
	LOG("i=%d", i);

	l = EInteger::highestOneBit(43432134);
	ES_ASSERT(l == 33554432);
	LOG("l=%lld", l);
	l = EInteger::highestOneBit(-43432134);
	ES_ASSERT(l == -2147483648);
	LOG("l=%lld", l);
	l = EInteger::highestOneBit(0);
	ES_ASSERT(l == 0);
	LOG("l=%lld", l);

	l = EInteger::lowestOneBit(43432134);
	ES_ASSERT(l == 2);
	LOG("l=%lld", l);
	l = EInteger::lowestOneBit(-43432134);
	ES_ASSERT(l == 2);
	LOG("l=%lld", l);
	l = EInteger::lowestOneBit(0);
	ES_ASSERT(l == 0);
	LOG("l=%lld", l);

	l = EInteger::divideUnsigned(-995445454, 65);
	ES_ASSERT(l == 50761874);
	LOG("l=%lld", l);

	l = EInteger::rotateLeft(-995445454, 65);
	ES_ASSERT(l == -1990890907);
	LOG("l=%lld", l);

	l = EInteger::rotateRight(-995445454, 65);
	ES_ASSERT(l == 1649760921);
	LOG("l=%lld", l);

	l = EInteger::reverse(995445454);
	ES_ASSERT(l == 1934797532);
	LOG("l=%lld", l);
	l = EInteger::reverse(-995445454);
	ES_ASSERT(l == 1286427939);
	LOG("l=%lld", l);

	l = EInteger::signum(995445454);
	ES_ASSERT(l == 1);
	LOG("l=%lld", l);
	l = EInteger::signum(-995445454);
	ES_ASSERT(l == -1);
	LOG("l=%lld", l);
	l = EInteger::signum(0);
	ES_ASSERT(l == 0);
	LOG("l=%lld", l);

	l = EInteger::reverseBytes(43432134);
	ES_ASSERT(l == -960981502);
	LOG("l=%lld", l);
	l = EInteger::reverseBytes(-43432134);
	ES_ASSERT(l == 977758717);
	LOG("l=%lld", l);
	l = EInteger::reverseBytes(0);
	ES_ASSERT(l == 0);
	LOG("l=%lld", l);
}

static void test_number_long()
{
	int i;
	llong l;

	ELLong ll(9);
	EString s = ll.toString();
	LOG("1s=%s", s.c_str());
	llong n = ELLong::parseLLong("1234567890");
	LOG("n=%lld", n);

	s  = ELLong::toOctalString(99999);
	LOG("2s=%s", s.c_str());

	s = ELLong::toUnsignedString(-323244, 10);
	LOG("3s=%s", s.c_str()); //18446744073709228372
	s = ELLong::toUnsignedString(-323244, 9);
	LOG("3s=%s", s.c_str()); //145808576354216171387

	ll = ELLong::decode("0x88778");
	LOG("ll=%lld", ll.value);
	ll = ELLong::decode("#88778");
	LOG("ll=%lld", ll.value);
	ll = ELLong::decode("027");
	LOG("ll=%lld", ll.value);

	i = ELLong::compareUnsigned(1, -1);
	LOG("i=%d", i);

	ll = ELLong::divideUnsigned(-995445454, 65);
	LOG("ll=%lld", ll.value);

	ll = ELLong::remainderUnsigned(-995445454, 65);
	LOG("ll=%lld", ll.value);

	i = ELLong::numberOfLeadingZeros(43432134);
	ES_ASSERT(i == 38);
	LOG("i=%d", i);
	i = ELLong::bitCount(43432134);
	ES_ASSERT(i == 13);
	LOG("i=%d", i);
	i = ELLong::numberOfLeadingZeros(0);
	ES_ASSERT(i == 64);
	LOG("i=%d", i);
	i = ELLong::bitCount(0);
	ES_ASSERT(i == 0);
	LOG("i=%d", i);

	i = ELLong::numberOfLeadingZeros(-43432134);
	ES_ASSERT(i == 0);
	LOG("i=%d", i);
	i = ELLong::bitCount(-43432134);
	ES_ASSERT(i == 51);
	LOG("i=%d", i);

	i = ELLong::numberOfTrailingZeros(43432134);
	ES_ASSERT(i == 1);
	LOG("i=%d", i);
	i = ELLong::numberOfTrailingZeros(-43432134);
	ES_ASSERT(i == 1);
	LOG("i=%d", i);
	i = ELLong::numberOfTrailingZeros(0);
	ES_ASSERT(i == 64);
	LOG("i=%d", i);

	l = ELLong::highestOneBit(43432134);
	ES_ASSERT(l == 33554432);
	LOG("l=%lld", l);
	l = ELLong::highestOneBit(-43432134);
	ES_ASSERT(l == -9223372036854775808);
	LOG("l=%lld", l);
	l = ELLong::highestOneBit(0);
	ES_ASSERT(l == 0);
	LOG("l=%lld", l);

	l = ELLong::lowestOneBit(43432134);
	ES_ASSERT(l == 2);
	LOG("l=%lld", l);
	l = ELLong::lowestOneBit(-43432134);
	ES_ASSERT(l == 2);
	LOG("l=%lld", l);
	l = ELLong::lowestOneBit(0);
	ES_ASSERT(l == 0);
	LOG("l=%lld", l);

	l = ELLong::divideUnsigned(-995445454, 65);
	ES_ASSERT(l == 283796062657140094);
	LOG("l=%lld", l);

	l = ELLong::rotateLeft(-995445454, 65);
	ES_ASSERT(l == -1990890907);
	LOG("l=%lld", l);

	l = ELLong::rotateRight(-995445454, 65);
	ES_ASSERT(l == 9223372036357053081);
	LOG("l=%lld", l);

	l = ELLong::reverse(995445454);
	ES_ASSERT(l == 8309892124321513472);
	LOG("l=%lld", l);
	l = ELLong::reverse(-995445454);
	ES_ASSERT(l == 5525165930960650239);
	LOG("l=%lld", l);

	l = ELLong::signum(995445454);
	ES_ASSERT(l == 1);
	LOG("l=%lld", l);
	l = ELLong::signum(-995445454);
	ES_ASSERT(l == -1);
	LOG("l=%lld", l);
	l = ELLong::signum(0);
	ES_ASSERT(l == 0);
	LOG("l=%lld", l);

	l = ELLong::reverseBytes(43432134);
	ES_ASSERT(l == -4127384123150958592);
	LOG("l=%lld", l);
	l = ELLong::reverseBytes(-43432134);
	ES_ASSERT(l == 4199441717188886527);
	LOG("l=%lld", l);
	l = ELLong::reverseBytes(0);
	ES_ASSERT(l == 0);
	LOG("l=%lld", l);
}

static void test_filepath()
{
	LOG("path=%s", ESystem::getCurrentWorkPath());
	LOG("file=%s", ESystem::getExecuteFilename());

	char path[1024];
	LOG("path=%s", eso_realpath("d:\\downloads\\..\\", path, 1024));
	LOG("path=%s", eso_realpath("d:\\downloads\\22.log", path, 1024));
	LOG("path=%s", eso_realpath("makefile", path, 1024));
	LOG("path=%s", eso_realpath("..\\.project", path, 1024));
}

static void test_config()
{
	EConfig config("sample.conf");

	EString str = config.toString();
	LOG(str.c_str());

	const char *p = config.getString("server_name|0");
	printf("conf v1=%s\n", p);
	p = config.getString("proc|0/exec_file|0");
	printf("conf v2=%s\n", p);
	p = config.getString("/proc|1/prv_conf|1/http/server/name");
	printf("conf v3=%s\n", p);
	p = config.getString("/proc/prv_conf/http/server/name");
	printf("conf v4=%s\n", p);

	p = config.getString("/env/LC_ALL");
	printf("conf LC_ALL=%s\n", p);

	EConfig* procConf = config.getConfig("proc|0");
	p = procConf->getString("prv_conf/http/server/listen");
	LOG(procConf->toString().c_str());
	printf("conf v5=%s\n", p);

	EArray<EConfig*> procs = config.getConfigs("/proc/prv_conf");
	for (int i =0; i<procs.length(); i++) {
		LOG(procs.getAt(i)->getString("wait_timeout"));
	}

	//========================

	config.loadFromINI("sample.ini");

	str = config.toString();
	LOG(str.c_str());

	p = config.getString("/BACKEND_REDIS/redis_host");
	printf("conf v6=%s\n", p);

	EArray<EConfig*> filters = config.getConfigs("FILTER");
	for (int i =0; i<filters.length(); i++) {
		LOG(filters.getAt(i)->getString("topic"));
	}
}

static void test_system()
{
	LOG("conf path=%s", ESystem::getConfigFilename());
	LOG("conf::server_name=%s", ESystem::getConfigParameter("server_name"));

	LOG("user.name=%s", ESystem::getProperty("user.name"));
	LOG("user.dir=%s", ESystem::getProperty("user.dir"));
	LOG("os.arch=%s", ESystem::getProperty("os.arch"));
	LOG("os.version=%s", ESystem::getProperty("os.version"));
	LOG("os.name=%s", ESystem::getProperty("os.name"));

	LOG("args::d=%s", ESystem::getProgramArgument("d"));
	LOG("args::key=%s", ESystem::getProgramArgument("key"));

	EString a("hhh");
	EString b("hhh");
	LOG("%d", ESystem::identityHashCode(&a));
	LOG("%d", ESystem::identityHashCode(&b));
	LOG("%d", a.hashCode());
	LOG("%d", b.hashCode());

	{
		sp<EString> v = ESystem::setProperty("xxx", "XXX");
		LOG("0 %s", (v==null) ? "v is null" : v->c_str());

		v = ESystem::setProperty("xxx", "YYY");
		LOG("1 %s", (v==null) ? "v is null" : v->c_str());

		v = ESystem::setProperty("xxx", "ZZZ");
		LOG("2 %s", (v==null) ? "v is null" : v->c_str());
		LOG("3 %s", ESystem::getProperty("xxx"));
	}
}

static void test_strToken() {
	EString s = EString(
			"The=Java=platform=is=the=ideal=platform=for=network=computing");
	EStringTokenizer st = EStringTokenizer(s.c_str(), "=", false);
	LOG("Token Total:%d", st.countTokens());
	//EString x = st.nextToken("is");
	//LOG("is 0: %s", x.c_str());
	LOG(st.nextToken().c_str());
	while (st.hasMoreTokens()) {
		LOG(st.nextToken().c_str());
	}
}

template<typename E>
class ComparatorTst: public EComparator<E> {
public:
	int compare(E o1, E o2) {
		if (o1->llongValue() > o2->llongValue())
			return 1;
		else if (o1->llongValue() < o2->llongValue())
			return -1;
		else
			return 0;
	}
};

class ComparableTst: public EInteger, virtual public EComparable<ComparableTst*> {
public:
	ComparableTst(int i) : EInteger(i) {

	}
	int compareTo(ComparableTst* o) {
		if (this->llongValue() > o->llongValue())
			return 1;
		else if (this->llongValue() < o->llongValue())
			return -1;
		else
			return 0;
	}
};

static void test_arraylist() {
	EArrayList<EInteger*> *x1 = new EArrayList<EInteger*>(true);
	EArrayList<EInteger*> *x2 = new EArrayList<EInteger*>(100);
	delete x1;
	delete x2;

	//0
	EArrayList<EInteger*> *list = new EArrayList<EInteger*>(true, 100);
	sp<EIterator<EInteger*> > iter = null;

	list->add(new EInteger(1));
	list->add(new EInteger(0));
	list->add(new EInteger(20000));
	list->add(new EInteger(20002));
	list->add(new EInteger(-223));
	list->add(new EInteger(220000));
	list->add(new EInteger(200300));
	list->add(new EInteger(202000));
	list->add(new EInteger(203000));
	list->add(new EInteger(2004400));
	list->add(new EInteger(2003200));
	list->add(new EInteger(20000));
	list->add(new EInteger(20002));
	list->add(new EInteger(234000));
	list->add(new EInteger(204200));
	list->add(new EInteger(201300));
	list->add(new EInteger(3420000));
	list->add(new EInteger(2540000));
	list->add(new EInteger(20063700));
	list->add(new EInteger(200400));
	EInteger* old = list->setAt(0, new EInteger(9999999));
	delete old;

	EArrayList<EInteger*> *cc = new EArrayList<EInteger*>(true, 100);
	cc->add(new EInteger(1));
	cc->add(new EInteger(0));
	cc->add(new EInteger(20000));
	list->removeAll(cc);
	delete cc;


	LOG("list.toStrng():%s", list->toString().c_str());

	ECollections::reverse(list);
	ECollections::shuffle(list);

	LOG("list.toStrng():%s", list->toString().c_str());

	ComparatorTst<EInteger*> ct;
	ECollections::sort(list, &ct);
//	ECollections::sort(list);

	iter = list->iterator();
	while (iter->hasNext()) {
		printf("iter->next()=%d\n", iter->next()->intValue());
	}

	sp<EListIterator<EInteger*> > listiter = list->listIterator();
	while (listiter->hasNext()) {
		printf("listiter->next()=%d\n", listiter->next()->intValue());
	}

	EInteger* max = ECollections::max(list, &ct);
	EInteger* min = ECollections::min(list, &ct);
	printf("max=%ld, min=%ld\n", max->llongValue(), min->llongValue());

	EInteger* I = list->getAt(1);
	printf("i=%d\n", I->intValue());

    EInteger t1 = EInteger::valueOf(20000);
	int n = list->indexOf(&t1);
	printf("n=%d\n", n);

    EInteger t2 = EInteger::valueOf(0);
	n = list->indexOf(&t2);
	printf("n=%d\n", n);

	LOG("size=%d", list->size());

	EA<EInteger*> arr = list->toArray();
	for (int i=0; i<arr.length(); i++) {
		LOG("arr[%d]=%d", i, arr[i]->intValue());
	}

	iter = list->iterator(1);
	while (iter->hasNext()) {
		printf("iter->next()=%d\n", iter->next()->intValue());
		iter->remove();
	}

	delete list;

	//1
	EArrayList<EString*> *list1 = new EArrayList<EString*>();

	list1->add(new EString("xxxxxxxxxxxxxxxxxxx"));
	list1->add(new EString("yyyyyyyyyyyyyyyyy"));
	list1->add(new EString("b"));
	list1->add(new EString("a"));
	list1->add(new EString("ccccc"));
	list1->add(new EString("aa"));
	list1->add(new EString("a.c"));
	list1->add(new EString("bbbbb"));
	list1->add(new EString("z_a"));
	EString *s = new EString("xvvxvxvxvxvxv");
	list1->add(s);

//	EArrayList<EString*> listClone = *list1;
//	delete list1;
//	EIterator<EString*> *iterxx = listClone.iterator();
//		while (iterxx->hasNext()) {
//			printf("iterxx->next()=%s\n", iterxx->next()->c_str());
//			iterxx->remove();
//	}
//	delete iterxx;
//	return;

	if (list1->contains(s)) {
		LOG("contains(s)");
	}

	EString *str = list1->getAt(1);
	printf("str=%s\n", str->c_str());

	s = list1->removeAt(2);
	LOG("s=%s", s->c_str());
	delete s;

	ECollections::sort(list1);

	sp<EIterator<EString*> > iter1 = list1->iterator();
	while (iter1->hasNext()) {
		printf("iter->next()=%s\n", iter1->next()->c_str());
		iter1->remove();
	}
	printf("list1->size=%d\n", list1->size());

	delete list1;

	//2
//	EArrayList<double> *list2 = new EArrayList<double>(32, false);
//
//	list2->add(1.0);
//	list2->add(2.99);
//	list2->add(2.43243243);
//	list2->add(2.44499);
//	list2->add(2.93339);
//	list2->add(2.000009900);
//
//	double f = (double) list2->getAt(1);
//	printf("f=%lf\n", f);
//
//	EIterator<double> *iter2 = list2->iterator();
//	while (iter2->hasNext()) {
//		printf("iter->next()=%lf\n", iter2->next());
//		iter2->remove();
//	}
//	delete iter2;
//	delete list2;

	//test comparable
	EArrayList<ComparableTst*> *list3 = new EArrayList<ComparableTst*>();
	list3->add(new ComparableTst(11));
	list3->add(new ComparableTst(2));
	list3->add(new ComparableTst(32));
	list3->add(new ComparableTst(4));
	list3->add(new ComparableTst(52));
	list3->add(new ComparableTst(52));
	list3->add(new ComparableTst(22));
	list3->add(new ComparableTst(72));
	list3->add(new ComparableTst(52));

	ECollections::sort(list3);

	sp<EIterator<ComparableTst*> > iter3 = list3->iterator();
	while (iter3->hasNext()) {
		printf("iter3->next()=%d\n", iter3->next()->intValue());
	}
	delete list3;


	//4
	EArrayList<llong> *list4 = new EArrayList<llong>();

	list4->add(1330);
	list4->add(2343);
	list4->add(243243243);
	list4->add(244499);
	list4->add(293339);
	list4->add(2000009900);

	llong l = (llong) list4->getAt(1);
	printf("l=%lld\n", l);

	sp<EIterator<llong> > iter4 = list4->iterator();
	while (iter4->hasNext()) {
		printf("iter->next()=%lld\n", iter4->next());
		iter4->remove();
	}
	delete list4;

	//TODO...
}

static void test_arraylist2() {
	EArrayList<sp<EInteger> > *x1 = new EArrayList<sp<EInteger> >();
	EArrayList<sp<EInteger> > *x2 = new EArrayList<sp<EInteger> >(100);
	delete x1;
	delete x2;

	//0
	EArrayList<sp<EInteger> > *list = new EArrayList<sp<EInteger> >(100);
	sp<EIterator<sp<EInteger> > > iter = null;

	list->add(new EInteger(1));
	list->add(new EInteger(0));
	list->add(new EInteger(20000));
	list->add(new EInteger(20002));
	list->add(new EInteger(-223));
	list->add(new EInteger(220000));
	list->add(new EInteger(200300));
	list->add(new EInteger(202000));
	list->add(new EInteger(203000));
	list->add(new EInteger(2004400));
	list->add(new EInteger(2003200));
	list->add(new EInteger(20000));
	list->add(new EInteger(20002));
	list->add(new EInteger(234000));
	list->add(new EInteger(204200));
	list->add(new EInteger(201300));
	list->add(new EInteger(3420000));
	list->add(new EInteger(2540000));
	list->add(new EInteger(20063700));
	list->add(new EInteger(200400));
	list->addAt(2, new EInteger(22222222));

	iter = list->iterator();
	while (iter->hasNext()) {
		printf("iter->next()=%d\n", iter->next()->intValue());
	}

	sp<EInteger>  old = list->setAt(0, new EInteger(9999999));

	EArrayList<sp<EInteger> > *cc = new EArrayList<sp<EInteger> >(100);
	cc->add(new EInteger(1));
	cc->add(new EInteger(0));
	cc->add(new EInteger(20000));
	list->removeAll(cc);
	delete cc;


	LOG("list.toStrng():%s", list->toString().c_str());

	ECollections::reverse(list);
	ECollections::shuffle(list);

	LOG("list.toStrng():%s", list->toString().c_str());

	ComparatorTst<sp<EInteger> > ct;
	ECollections::sort(list, &ct);
//	ECollections::sort(list);

	iter = list->iterator();
	while (iter->hasNext()) {
		printf("iter->next()=%d\n", iter->next()->intValue());
	}

	sp<EListIterator<sp<EInteger> > > listiter = list->listIterator();
	while (listiter->hasNext()) {
		printf("listiter->next()=%d\n", listiter->next()->intValue());
	}

	sp<EInteger>  max = ECollections::max(list, &ct);
	sp<EInteger>  min = ECollections::min(list, &ct);
	printf("max=%ld, min=%ld\n", max->llongValue(), min->llongValue());

	sp<EInteger>  I = list->getAt(1);
	printf("i=%d\n", I->intValue());

	EInteger t1 = EInteger::valueOf(20000);
	int n = list->indexOf(&t1);
	printf("n=%d\n", n);

	EInteger t2 = EInteger::valueOf(0);
	n = list->indexOf(&t2);
	printf("n=%d\n", n);

	LOG("size=%d", list->size());

	EA<sp<EInteger> > arr = list->toArray();
	for (int i=0; i<arr.length(); i++) {
		LOG("arr[%d]=%d", i, arr[i]->intValue());
	}

	iter = list->iterator(1);
	while (iter->hasNext()) {
		printf("iter->next()=%d\n", iter->next()->intValue());
		iter->remove();
	}

	delete list;

	//1
	EArrayList<EString*> *list1 = new EArrayList<EString*>();

	list1->add(new EString("xxxxxxxxxxxxxxxxxxx"));
	list1->add(new EString("yyyyyyyyyyyyyyyyy"));
	list1->add(new EString("b"));
	list1->add(new EString("a"));
	list1->add(new EString("ccccc"));
	list1->add(new EString("aa"));
	list1->add(new EString("a.c"));
	list1->add(new EString("bbbbb"));
	list1->add(new EString("z_a"));
	EString *s = new EString("xvvxvxvxvxvxv");
	list1->add(s);

//	EArrayList<EString*> listClone = *list1;
//	delete list1;
//	EIterator<EString*> *iterxx = listClone.iterator();
//		while (iterxx->hasNext()) {
//			printf("iterxx->next()=%s\n", iterxx->next()->c_str());
//			iterxx->remove();
//	}
//	delete iterxx;
//	return;

	if (list1->contains(s)) {
		LOG("contains(s)");
	}

	EString *str = list1->getAt(1);
	printf("str=%s\n", str->c_str());

	s = list1->removeAt(2);
	LOG("s=%s", s->c_str());
	delete s;

	ECollections::sort(list1);

	sp<EIterator<EString*> > iter1 = list1->iterator();
	while (iter1->hasNext()) {
		printf("iter->next()=%s\n", iter1->next()->c_str());
		iter1->remove();
	}
	printf("list1->size=%d\n", list1->size());

	delete list1;

	//2
//	EArrayList<double> *list2 = new EArrayList<double>(32, false);
//
//	list2->add(1.0);
//	list2->add(2.99);
//	list2->add(2.43243243);
//	list2->add(2.44499);
//	list2->add(2.93339);
//	list2->add(2.000009900);
//
//	double f = (double) list2->getAt(1);
//	printf("f=%lf\n", f);
//
//	EIterator<double> *iter2 = list2->iterator();
//	while (iter2->hasNext()) {
//		printf("iter->next()=%lf\n", iter2->next());
//		iter2->remove();
//	}
//	delete iter2;
//	delete list2;

	//test comparable
	EArrayList<ComparableTst*> *list3 = new EArrayList<ComparableTst*>();
	list3->add(new ComparableTst(11));
	list3->add(new ComparableTst(2));
	list3->add(new ComparableTst(32));
	list3->add(new ComparableTst(4));
	list3->add(new ComparableTst(52));
	list3->add(new ComparableTst(52));
	list3->add(new ComparableTst(22));
	list3->add(new ComparableTst(72));
	list3->add(new ComparableTst(52));

	ECollections::sort(list3);

	sp<EIterator<ComparableTst*> > iter3 = list3->iterator();
	while (iter3->hasNext()) {
		printf("iter3->next()=%d\n", iter3->next()->intValue());
	}
	delete list3;


	//4
	EArrayList<llong> *list4 = new EArrayList<llong>();

	list4->add(1330);
	list4->add(2343);
	list4->add(243243243);
	list4->add(244499);
	list4->add(293339);
	list4->add(2000009900);

	llong l = (llong) list4->getAt(1);
	printf("l=%lld\n", l);

	sp<EIterator<llong> > iter4 = list4->iterator();
	while (iter4->hasNext()) {
		printf("iter->next()=%lld\n", iter4->next());
		iter4->remove();
	}
	delete list4;
}

static void test_linkedlist()
{
	ELinkedList<EInteger*> *list0 = new ELinkedList<EInteger*>();

	list0->add(new EInteger(0));
	list0->add(new EInteger(1));
	list0->add(new EInteger(2));
	EInteger* ei = list0->remove();
	int n0 = ei->intValue();
	delete ei;
	ei = list0->remove();
	int n1 = ei->intValue();
	delete ei;
	ei = list0->remove();
	int n2 = ei->intValue();
	delete ei;
	LOG("n0=%d, n1=%d, n2=%d", n0, n1, n2);

	list0->add(new EInteger(1));
	list0->add(new EInteger(0));
	list0->add(new EInteger(20000));

	LOG("list0 2=%d", list0->getAt(2)->intValue());
	LOG("list0.toStrng():%s", list0->toString().c_str());

//	list0->setAutoFree(false);
	ELinkedList<EInteger*> listxx = *list0;
//	listxx.setAutoFree(false);
	LOG("listxx 1=%d", list0->getAt(1)->intValue());

	EInteger* u = null;
	while ((u = listxx.poll()) != null) {
		LOG("list0 u=%d", u->intValue());
        delete u;
	}

	LOG("listxx.toStrng():%s", listxx.toString().c_str());

	list0->clear();

	delete list0;


//	ELinkedList<double> *list2 = new ELinkedList<double>(false);
//
//	list2->add(2222.0009);
//	list2->add(3333.0009);
//	list2->add(44444.0009);
//
//	LOG("list1 2=%lf", list2->get(2));
//
//	delete list2;
//
//
	ELinkedList<EString*> *list1 = new ELinkedList<EString*>();

	list1->add(new EString("xxxxxxxxxxxxxxxxxxx"));
	list1->add(new EString("yyyyyyyyyyyyyyyyy"));
	EString *s = new EString("xvvxvxvxvxvxv");
	list1->add(s);

	if (list1->contains(s)) {
		LOG("contains(s)");
	}

	EString *str = list1->getAt(1);
	printf("str=%s\n", str->c_str());

	s = list1->removeAt(2);
	LOG("s=%s", s->c_str());
	delete s;

//	list1->clear();

	sp<EIterator<EString*> > iter1 = list1->iterator();
	while (iter1->hasNext()) {
		printf("iter->next()=%s\n", iter1->next()->c_str());
		iter1->remove();
	}
	printf("list1->size=%d\n", list1->size());

	delete list1;

	{
		ELinkedList<EString*> list;
		for (int i=0; i<10000; i++) {
			list.add(new EString(i));
		}
		EString* s = list.getAt(10000-1);
		LOG("s=%s", s->c_str());

		sp<EListIterator<EString*> > listiter = list.listIterator(list.size());
		while (listiter->hasPrevious()) {
			EString* s = listiter->previous();
			LOG("s=%s", s->c_str());
		}

		sp<EIterator<EString*> > iter = list.iterator();
		while (iter->hasNext()) {
			EString* s = iter->next();
//			LOG("s=%s", s->c_str());
			iter->remove();
		}
	}
}

static void test_linkedlist2()
{
#if 0
	{
		ELinkedList<sp<EString> > list;

		list.add(new EString("3333"));
		list.add(new EString("4444"));
		list.add(new EString("5555"));
		list.add(new EString("1111"));
		list.add(new EString("2222"));
		list.add(new EString("3333"));
		list.add(new EString("4444"));
		list.add(new EString("5555"));

		EString x("2222");
		list.remove(&x);
	//	list.remove(2);

		sp<EString> s = list.peek();
		LOG("s0=%s", s->c_str());

		sp<EIterator<sp<EString> > > iter = list.iterator();
		while (iter->hasNext()) {
			sp<EString> s = iter->next();
			LOG("s1=%s", s->c_str());
		}

		sp<EIterator<sp<EString> > > iter2 = list.listIterator(2);
		while (iter2->hasNext()) {
			sp<EString> s = iter2->next();
			LOG("s2=%s", s->c_str());
		}
	}
#endif

#if 1
	{
		ELinkedList<ullong> list;

		list.add(22222);
		list.add(3333);
		list.add(44444);
		list.add(1111);
		list.add(3);
		list.add(99999999);

		sp<EIterator<ullong> > iter2 = list.listIterator(2);
		while (iter2->hasNext()) {
			llong l = iter2->next();
			LOG("l=%ld", l);
		}
	}
#endif
}

static void test_stack() {
	EStack<EString*> stack;

	stack.push(new EString("1"));
	stack.push(new EString("2"));
	stack.push(new EString("3"));
	stack.push(new EString("4"));

	LOG(stack.peek()->c_str());

	EString* s = stack.pop();
	LOG(s->c_str()); delete s;
	s = stack.pop();
	LOG(s->c_str()); delete s;
	s = stack.pop();
	LOG(s->c_str()); delete s;
	s = stack.pop();
	LOG(s->c_str()); delete s;
//	s = stack.pop();
}

static void test_arraydeque() {
	EArrayDeque<EString*> ad(1);

	ad.addFirst(new EString("x0"));
	ad.add(new EString("x1"));
	ad.add(new EString("x2"));
	ad.add(new EString("x3"));
	ad.add(new EString("x4"));
	ad.addLast(new EString("x5"));
	ad.add(new EString("x6"));
	ad.add(new EString("x7"));
	ad.add(new EString("x8"));
	ad.add(new EString("x9"));
	ad.add(new EString("x10"));
	ad.add(new EString("x11"));
	ad.add(new EString("x12"));
	ad.add(new EString("x13"));
	ad.add(new EString("x14"));

	EDeque<EString*>* dq = dynamic_cast<EDeque<EString*>*>(&ad);
	dq->push(new EString("x15"));
	dq->offer(new EString("x16"));
	dq->remove();

	EQueue<EString*>* qe = dynamic_cast<EQueue<EString*>*>(&ad);
	qe->offer(new EString("x17"));
	qe->remove();

	EString* s = ad.poll();
	LOG("poll s=%s", s->c_str());
	delete s;

	sp<EIterator<EString*> > iter = ad.iterator();
	while (iter->hasNext()) {
		EString* s = iter->next();
		LOG("s=%s", s->c_str());
	}

	s = ad.pop();
	LOG("pop s=%s", s->c_str());
	delete s;

	EArrayDeque<EString*> ad_(ad);
	iter = ad_.iterator();
	while (iter->hasNext()) {
		EString* s = iter->next();
		LOG("s_=%s", s->c_str());
	}
	ad_.addFirst(new EString("x0"));
	ad_.addLast(new EString("x15"));

	ad = ad_;
	iter = ad.iterator();
	while (iter->hasNext()) {
		EString* s = iter->next();
		LOG("s=%s", s->c_str());
//		iter->remove();
		delete iter->moveOut();
	}
}

class CompratorByLastModified: public EComparator<EFile*> {
public:
	int compare(EFile* f1, EFile* f2) {
		llong diff = f1->lastModified() - f2->lastModified();
		if (diff > 0)
			return 1;
		else if (diff == 0)
			return 0;
		else
			return -1;
	}
};

static void test_file() {
	//1.
	EFile file("/");

	EArray<EString*> names = file.list();
	sp<EIterator<EString*> > iter = names.iterator();
	while (iter->hasNext()) {
		LOG("name=%s", iter->next()->c_str());
	}

	EArray<EString*> names2 = file.list();
	iter = names2.iterator();
	while (iter->hasNext()) {
		LOG("name2=%s", iter->next()->c_str());
	}

	EArray<EFile*> files = file.listFiles();
	sp<EIterator<EFile*> > iter2 = files.iterator();
	while (iter2->hasNext()) {
		LOG("name3=%s", iter2->next()->toString().c_str());
	}
	LOG("\n");
	CompratorByLastModified c;
	ECollections::sort(&files, &c);
	iter2 = files.iterator();
	while (iter2->hasNext()) {
		EFile *f = iter2->next();
		LOG("name3=%s, lastModified=%lld", f->toString().c_str(), f->lastModified());
	}

	LOG("file.toString=%s", file.toString().c_str());

	//2.
	EFile file1("sample.conf");
	LOG("file1.toString=%s", file1.toString().c_str());

	//3.
	EFile file2 = EFile::createTempFile(null, "/tmp");
	LOG("file2.toString=%s", file2.toString().c_str());

	//4.
	EFile file01("./../Debug/logs/error.log");
	EFile file02("./logs/error.log");

	LOG("file01.path=%s", file01.getCanonicalPath().c_str());
	LOG("file02.path=%s", file02.getCanonicalPath().c_str());

	if (file01.equals(&file02)) {
		LOG("file01 == file02");
	}

	//5.
	EFile file05("/Users/xxx/Temp");
	if (!file05.exists()) {
		LOG("File not found!");
	}

	EFile file_(file05);
	LOG("file_.path=%s", file_.getCanonicalPath().c_str());

	file_ = file01;
	LOG("file_.path=%s", file_.getCanonicalPath().c_str());

	//6.
	EFile file06("/tmp");
	LOG("file06 is link: %s", file06.isLink() ? "true" : "false");

	EFile file07("/tmp/all.log");
	LOG("file07 can write: %s", file07.canWrite() ? "true" : "false");
	boolean e = file07.canExecute();
	LOG("file07 can execute: %s", e ? "true" : "false");
	file07.setExecutable(!e, true);
	e = file07.canExecute();
	LOG("file07 can execute: %s", e ? "true" : "false");

	//7.
	class FileFilter : public EFileFilter {
	public:
		virtual boolean accept(EFile *pathname) {
			return (pathname->getName().endsWith(".log"));
		}
	};
	FileFilter filter;
	EArray<EFile*> logsfile = file06.listFiles(&filter);
	for (int i=0; i<logsfile.size(); i++) {
		LOG("file=%s", logsfile.getAt(i)->toString().c_str());
	}
}

class Class : public EObject {
public:
	virtual ~Class() {
		LOG("~Class()");
	}
	Class() {
		LOG("Class()");
	}
	int value() {
		return 1;
	}
	const char* c_str() {
		return "xxxxxx";
	}

	static EString v() {
		static const EString xxx("xxx");
		return xxx;
	}
};

class SubClass : public Class {
public:
	virtual ~SubClass() {
		LOG("~SubClass()");
	}
	SubClass() {
		LOG("SubClass()");
	}
};

static void test_class() {
#define DECLARE_INSTANCEOF(name) virtual boolean instanceof(const char* n) { \
		return eso_strstr(name, n) ? true : false; \
	}

#define IFA _S(A)
	class A {
	public:
		int n;
		A() {
			n = 9;
		}

		EString name() {
			return typeid(*this).name();
		}

		DECLARE_INSTANCEOF(IFA);
#if 0
		virtual boolean instanceof(const char* name) {
			if (eso_strcmp(name, "A") == 0) {
				return true;
			}
			return false;
		}
#endif

	public:
		class B : public EObject {
		int _line;
		public:
			B(int line): _line(line) {
				printf("line=%d\n", _line);
			}
			~B() {
				printf("~line=%d\n", _line);
			}
		};
	};

	A::B *b = new A::B(__LINE__);
	LOG("b->hashcode=%d", b->hashCode());
	delete b;

	//===========================

	{
		A::B xxx(__LINE__);
		{
			{
			A::B xxx(__LINE__);
			{
				printf("xxx\n");
            }}
		}
	}

	//===========================

	LOG("size of EInteger=%d", sizeof(EInteger));

	//===========================

#define IFC _S(C) "," IFA
	class C : public A {
	public:
		DECLARE_INSTANCEOF(IFC);

//		EString name() {
//			return typeid(*this).name();
//		}
#if 0
		virtual boolean instanceof(const char* name) {
			if (eso_strcmp(name, "C") == 0) {
				return true;
			}
			return false;
		}
#endif
	};

	A a;
	C c;

	LOG("A name=%s", a.name().c_str());
	LOG("C name=%s", c.name().c_str());

	LOG("a instanceof A : %d", a.instanceof(_S(A)));
	LOG("a instanceof C : %d", a.instanceof(_S(C)));
	LOG("c instanceof A : %d", c.instanceof(_S(A)));
	LOG("c instanceof C : %d", c.instanceof(_S(C)));

#define DECLARE_PARAMS2(name1,name2) #name1 "," #name2
	char *s = DECLARE_PARAMS2(A,C);
	LOG("s=%s,v=%d", s, eso_strstr(s, _S(A)));

	//===========================

	EString* _s = NEWC(EString)("NEWC EString test.");
	LOG("_s : %s", _s->c_str());
	DELC(_s);

	A* _a = NEWC(A)();
	LOG("_a instanceof A : %d", instanceof<A>(_a));
//	DELC(_a); //gcc error: no matching function for call to 'DELC(test_class()::A*&)'

	Class* c0 = NEWRC(Class)();
	Class* c1 = GETRC(c0);
	DELRC(c0);
	DELRC(c1);
//	DELRC(c0);

	SubClass* sc0 = NEWRC(SubClass)();
	Class* sc1 = (Class*)GETRC(sc0);
	DELRC(sc0);
	DELRC(sc1);

	//===========================

	A::B ab(__LINE__);
	EObject* ax = dynamic_cast<EObject*>(&ab);
	if (ax) {
		LOG("a is instance of class EObject.");
	}

	class X {
	public:
		virtual ~X(){}
	};
	class Y {

	};
	class Z {
	public:
		virtual void funcz() {
		}
	};
	class XY : public X, public Y {

	};
	class XYZ : public X, virtual public Y, public Z {

	};
	XY xy;
	XY* _xy = dynamic_cast<XY*>(&xy);
	if (_xy) {
		LOG("_xy is instance of class XY.");
	}
	X* x = dynamic_cast<X*>(&xy);
	if (x) {
		LOG("x is instance of class X.");
	}
	Y* y = dynamic_cast<Y*>(&xy);
	if (y) {
		LOG("y is instance of class Y.");
	}
	XYZ xyz;
	Z* z = dynamic_cast<Z*>(&xyz);
	if (z) {
		LOG("z is instance of class Z.");
	}
	if (instanceof<Y>(&xyz)) {
		LOG("y is instance of class Y.");
	}
	if (instanceof<Z>(xy)) {
		LOG("z is instance of class Z.");
	}
}

//测试拷贝构造函数、赋值函数等
static void test_class2() {
	//1. EA
	EA<EString*> a1(1);
	a1[0] = new EString("1234567087654321");

	{
		EA<EString*> a2(2);
		a2[0] = new EString("xxxxxxxxxxxxxxx");
		a2[1] = new EString("yyyyyyyyyyyyyyy");

		LOG((char*)a1[0]->c_str());
		a1 = a2;
		LOG((char*)a2[0]->c_str());
		LOG((char*)a2[1]->c_str());
	}

	LOG((char*)a1[0]->c_str());
	LOG((char*)a1[1]->c_str());

	char* s = new char[30];
	strncpy(s, "aaaaaaaaaaaaaaa", 20);
	EA<char> a3(s, strlen(s), true, MEM_NEW);
	{
		char* s = new char[30];
		strncpy(s, "bbbbbbbbbbbbb", 20);
		EA<char> a4(s, strlen(s), true, MEM_NEW);

		LOG((char*)a3.address());
		a3 = a4;
		LOG((char*)a4.address());

		LOG((char*)a3.address());
	}
	LOG((char*)a3.address());

	EA<byte> a5(10);
	EByteBuffer bb;
	a5 = *(bb.reset().get());

	//2.
}

static void test_hashmap() {
#if 1
	EHashMap<EString*, EString*> hashmap(true, true);

	for (int i=0; i<10000; i++) {
//		hashmap.put(new EString("key1"), new EString("value1"));
//		hashmap.put(new EString("key2"), new EString("value2"));
		hashmap.put(new EString(EString::formatOf("key%d", i)), new EString(EString::formatOf("value%d", i)));
	}

	LOG("hashmap.toString():%s", hashmap.toString().c_str());

	EHashMap<EString*, EString*> hashmap__;
	{
		EHashMap<EString*, EString*> hashmap_(hashmap);

		ECollection<EString*>* values = hashmap.values();
		sp<EIterator<EString*> > iterS = values->iterator();
		while(iterS->hasNext()) {
			LOG("v=%s", iterS->next()->c_str());
		}

		//=======================

		for (int i=0; i<10; i++) {
			hashmap__.put(new EString(EString::formatOf("key%d", i)), new EString(EString::formatOf("value%d", i)));
		}
		hashmap__ = hashmap_;
		{
			ECollection<EString*>* values = hashmap__.values();
			sp<EIterator<EString*> > iterS = values->iterator();
			while(iterS->hasNext()) {
				LOG("v=%s", iterS->next()->c_str());
			}
		}

		hashmap__.setAutoFree(false, false);
	}
	hashmap.setAutoFree(true, true);

	LOG("=======================");

    EString t1 = EString("key1");
	boolean hasK1 = hashmap.containsKey(&t1);
	LOG("hasK1=%d", hasK1);
    EString t2 = EString("value2");
	boolean hasV2 = hashmap.containsValue(&t2);
	LOG("hasV2=%d", hasV2);

    EString t3 = EString("key1");
	EString *v1 = hashmap.get(&t3);
	LOG(v1->c_str());

	ECollection<EString*>* values = hashmap.values();
	sp<EIterator<EString*> > iterS = values->iterator();
	while(iterS->hasNext()) {
		LOG("v=%s", iterS->next()->c_str());
	}

	ESet<EMapEntry<EString*, EString*>*>* set = hashmap.entrySet();
	LOG("set size=%d", set->size());
	sp<EIterator<EMapEntry<EString*, EString*>*> > iter = set->iterator();
	while(iter->hasNext()) {
		LOG("key=%s", iter->next()->getKey()->c_str());
		//iter->remove();
		break;
	}

	LOG("map size=%d", hashmap.size());

    EString t4 = EString("key1");
	v1 = hashmap.remove(&t4);
	LOG(v1->c_str());
	delete v1;

    EString t5 = EString("key2");
	EHashMap<EString*, EString*>::Entry *entry = hashmap.removeEntryForKey(&t5);
	LOG(entry->getValue()->c_str());
	delete entry;
	LOG("map size=%d", hashmap.size());

	LOG("=======================");

	hashmap.clear();

	//========================

	EHashMap<int, EString*> hashmapi(10, true);
	hashmapi.put(10, new EString("10"));
	hashmapi.put(19, new EString("19"));

	LOG("hashmapi size=%d", hashmapi.size());

	LOG("hashmapi.toString():%s", hashmapi.toString().c_str());

	ESet<EMapEntry<int, EString*>*>* setI = hashmapi.entrySet();
	LOG("set size=%d", setI->size());
	sp<EIterator<EMapEntry<int, EString*>*> > iterI = setI->iterator();
	while(iterI->hasNext()) {
		LOG("key=%d", iterI->next()->getKey());
		//iterI->remove();
	}
	hashmapi.setAutoFree(false);
	hashmapi.setAutoFree(true);

	EHashMap<llong, EString*> hashmapL;
	hashmapL.put(2222222, new EString("10"));
	hashmapL.put(23232323, new EString("19"));

	LOG("hashmapL size=%d", hashmapL.size());

	ESet<EMapEntry<llong, EString*>*>* setL = hashmapL.entrySet();
	LOG("set size=%d", setL->size());
	sp<EIterator<EMapEntry<llong, EString*>*> > iterL = setL->iterator();
	while(iterL->hasNext()) {
		LOG("key=%lld", iterL->next()->getKey());
		//iterL->remove();
	}

	//test object copy
	{
		EHashMap<int, EString*> hashmapi_(hashmapi);
		hashmapi_.put(10, new EString("10"));
	}
#endif

#if 1
	{
		EHashMap<int, sp<EString> > hm;

		hm.put(0, new EString("0000"));
		hm.put(2, new EString("2222"));
		hm.put(4, new EString("4444"));
		hm.put(0, null);

		sp<EIterator<EMapEntry<int, sp<EString> >*> > iter = hm.entrySet()->iterator();
		while(iter->hasNext()) {
			sp<EString> s = iter->next()->getValue();
			LOG("key=%s", (s != null) ? s->c_str() : "null");
		}

		EMap<int, sp<EString> >* m = &hm;
		m->put(5, new EString("55555"));

		iter = hm.entrySet()->iterator();
		while(iter->hasNext()) {
			sp<EString> s = iter->next()->getValue();
			LOG("key=%s", (s != null) ? s->c_str() : "null");
		}
	}
#endif
}

static void test_hashset() {
	EHashSet<EString*> hashset(true);
	hashset.add(new EString("1"));
	hashset.add(new EString("2"));
	LOG("add 3 ret=%d", hashset.add(new EString("3")));
	LOG("add 2 ret=%d", hashset.add(new EString("2")));

	LOG("hashset.toString():%s", hashset.toString().c_str());
	ESet<EString*>* set = &hashset;
	LOG("set.toString():%s", set->toString().c_str());

	sp<EIterator<EString*> > iter = hashset.iterator();
	while(iter->hasNext()) {
		LOG("set v=%s", iter->next()->c_str());
	}

	EString s3("3");
	EString s4("4");
	LOG("contains 3 ret=%d", hashset.contains(&s3));
	LOG("contains 4 ret=%d", hashset.contains(&s4));

	//test object copy
	EHashSet<EString*> hashset_(hashset);
	iter = hashset_.iterator();
	while(iter->hasNext()) {
		LOG("set v_=%s", iter->next()->c_str());
	}

	//test operator=
	hashset = hashset_;
	iter = hashset.iterator();
	while(iter->hasNext()) {
		LOG("set v=%s", iter->next()->c_str());
	}
}

static void test_treemap() {
	ETreeMap<EInteger*,EString*> tm;

	do {
	tm.put(new EInteger(9), new EString("====9"));
	EInteger n9 = EInteger(9);
	EString* v9 = tm.get(&n9);
	LOG("v9=%s", v9->c_str());

	tm.put(new EInteger(2), new EString("====2"));
	EInteger n2 = EInteger(2);
	EString* v2 = tm.get(&n2);
	LOG("v2=%s", v2->c_str());

	for (int i=10000; i>0; i--) {
		EString* v = tm.put(new EInteger(i), new EString(EString::formatOf("value%d", i)));
		if (v) {
			delete v;
		}
		//test reput.
		v = tm.put(new EInteger(i), new EString(EString::formatOf("value%d", i)));
		if (v) {
			delete v;
		}
	}

	ESet<EMapEntry<EInteger*,EString*>*>* es = tm.entrySet();
	sp<EIterator<EMapEntry<EInteger*,EString*>*> > iter = es->iterator();
	while (iter->hasNext()) {
		EMapEntry<EInteger*,EString*>* me = iter->next();
		LOG("K=%d, V=%s", me->getKey()->intValue(), me->getValue()->c_str());

//		iter->remove();
//		delete iter->moveOut();
	}

//	return;

	v2 = tm.remove(&n2);
	LOG("remove v2=%s", v2->c_str());
	delete v2;

	EMapEntry<EInteger*,EString*>* me = tm.pollFirstEntry();
	LOG("me.k=%d,me.v=%s", me->getKey()->intValue(), me->getValue()->c_str());
	delete me;
	me = tm.pollLastEntry();
	LOG("me.k=%d,me.v=%s", me->getKey()->intValue(), me->getValue()->c_str());
	delete me;

	tm.clear();
	} while (0);
}

static void test_treeset() {
	ETreeSet<EString*> ts;

	ts.add(new EString("x1"));
	ts.add(new EString("x2"));
	ts.add(new EString("x3"));
	ts.add(new EString("x4"));

	sp<EIterator<EString*> > iter = ts.iterator();
	while (iter->hasNext()) {
		EString* s = iter->next();
		LOG("s=%s", s->c_str());
	}
}

static void test_math() {
	double d0 = EMath::abs(-1.01);

	double d1 = EMath::toRadians(0.333);

	LOG("d0=%lf, d1=%lf", d0, d1);
}

static void test_array() {
#if 1
	// native_ptr
	EA<EString*> ea4(32);
	ea4[0] = new EString("0");
	ea4[1] = new EString("1");
	ea4[2] = new EString("c");
	ea4[3] = new EString("A");
	ea4[4] = new EString("2");
	ea4[5] = new EString("b");
	ea4[13] = new EString("aab");
	ea4[15] = new EString("abb");
	ea4[16] = new EString("abc");
	ea4[19] = new EString("aba");
	ea4[10] = new EString("abbb");
	ea4.setLength(100);
	ea4.sort();
	for (int i=0; i<ea4.length(); i++) {
		if (ea4[i]) LOG("ea4[%d]=%s", i, ea4[i]->c_str());
	}

	EA<EString*> ea4_(ea4);
	ea4_.setLength(200);
	for (int i=0; i<ea4_.length(); i++) {
		if (ea4_[i]) LOG("ea4_[%d]=%s", i, ea4_[i]->c_str());
	}

	EA<EString*> eacopy0(32, false);
	ESystem::arraycopy(ea4, 0, eacopy0, 0, 32);
	eacopy0.setLength(300);
	for (int i=0; i<eacopy0.length(); i++) {
		if (eacopy0[i]) LOG("eacopy0[%d]=%s", i, eacopy0[i]->c_str());
	}

	EA<EString*>* eacopy1 = new EA<EString*>(32, false);
	ESystem::arraycopy(&ea4, 0, eacopy1, 0, 32);
	eacopy1->setLength(400);
	for (int i=0; i<eacopy1->length(); i++) {
		if (eacopy1->getAt(i)) LOG("eacopy1[%d]=%s", i, eacopy1->getAt(i)->c_str());
	}
	delete eacopy1;

	EArray<EString*> arr(true, 1);
	arr.add(new EString("1111"));
	arr.add(new EString("2222"));
	arr.add(new EString("3333"));
	for (int i=0; i<arr.length(); i++) {
		LOG("arr[%d]=%s", i, arr[i]->c_str());
	}

	{
		EArray<EString*> *namesxx = new EArray<EString*>();
		namesxx->add(new EString("jjjjjjjjjjj1"));
		namesxx->add(new EString("jjjjjjjjjjj2"));
		namesxx->add(new EString("jjjjjjjjjjj3"));
		EArray<EString*> namesyy = *namesxx;
		delete namesxx;
		LOG("auto=%d\n", namesyy.getAutoFree());
		sp<EIterator<EString*> > iter = namesyy.iterator();
		while (iter->hasNext()) {
			LOG("namesyy=%s", iter->next()->c_str());
		}

		EArray<EString*> nameszz;
		nameszz = namesyy;
		iter = nameszz.iterator();
		while (iter->hasNext()) {
			LOG("nameszz=%s", iter->next()->c_str());
		}
	}
#endif

#if 1
	// shared_ptr
	EA<sp<EString> > shared_ea(20);
	shared_ea[0] = new EString("0");
	shared_ea[1] = new EString("1");
	shared_ea[2] = new EString("c");
	shared_ea[3] = new EString("A");
	shared_ea[4] = new EString("2");
	shared_ea[5] = new EString("b");
	shared_ea[13] = new EString("aab");
	shared_ea[15] = new EString("abb");
	shared_ea[16] = new EString("abc");
	shared_ea[19] = new EString("aba");
	shared_ea[10] = new EString("abbb");
	shared_ea.setLength(20);
	shared_ea.sort();
	for (int i=0; i<shared_ea.length(); i++) {
		if (shared_ea[i] != null) LOG("shared_ea[%d]=%s", i, shared_ea[i]->c_str());
	}

	if (1) {
		EA<sp<EString> > shared_ea_copy(32);
		ESystem::arraycopy(shared_ea, 0, shared_ea_copy, 0, 20);
		for (int i=0; i<shared_ea_copy.length(); i++) {
			if (shared_ea_copy[i] != null) LOG("shared_ea_copy[%d]=%s", i, shared_ea_copy[i]->c_str());
		}
	}

#endif

#if 1
	int i;

	EA<char> ea_char(30);
	ea_char[0] = 'c';

	EA<short> ea_short(30);
	ea_short[0] = 22;

	EA<int> ea(30);
	ea[1] = 999;
	ea[11] = 99999999;
	ea[12] = 999;
	ea[13] = 991;
	ea[14] = 8;
	ea[17] = 0;
	ea[18] = -2;
	ea[19] = -4;
	ea.setLength(20);
	for (i=0; i<ea.length(); i++) {
		LOG("ea[%d]=%d", i, ea[i]);
	}
	ea.sort();
	for (i = 0; i < ea.length(); i++) {
		LOG("ea[%d]=%d", i, ea[i]);
	}

	EA<int> ea_(ea);
	for (i=0; i<ea_.length(); i++) {
		LOG("ea_[%d]=%d", i, ea_[i]);
	}

	EA<long> ea_long(30);
	ea_long[0] = 2222223232;

	EA<float> ea_float(30);
	ea_float[0] = 222222.3232;

	EA<double> *ea2 = new EA<double>(30);
	(*ea2)[1] = 999.99;
	(*ea2)[2] = 999.98;
	(*ea2)[3] = 999.98;
	(*ea2)[4] = 0.0000001;
	(*ea2)[5] = -0.0000001;
	(*ea2)[7] = 0.000000;
	(*ea2)[20] = 999.000001;
	(*ea2)[22] = 999.000002;
	(*ea2)[28] = -999.9100002;
	ea2->setLength(6);
	for (i = 0; i < ea2->length(); i++) {
		LOG("ea2[%d]=%.9f", i, (*ea2)[i]);
	}
	ea2->sort();
	for (i = 0; i < ea2->length(); i++) {
		LOG("ea2[%d]=%.9f", i, (*ea2)[i]);
	}
	EA<double> ea2_(*ea2);
	delete ea2;
	for (i = 0; i < ea2_.length(); i++) {
		LOG("ea2_[%d]=%.9f", i, ea2_[i]);
	}

	EA<char> ea3(32, 'c');
	ea3[31] = 0;
	LOG("ea3=%s", &ea3[0]);


	EA<int>* iea = new EA<int>(10);
	EA<int>& fea = *iea;
	fea[0] = 1;
	fea[1] = 3;
	for (i=0; i<fea.length(); i++) {
		LOG("v=%d", fea[i]);
	}
	delete iea;

	//EA<byte*> bea("abcedfghijklmn");

	char str[32];
	eso_strcpy(str, "abfdsarea;lkdfcedfghijklmn");
	EA<char> bea(str, eso_strlen(str), false, MEM_IGNORE);

	LOG("%s", bea.address());
	LOG("%c", bea[2]);
	bea.sort();
	LOG("%s", bea.address());

	EA<char> bea2 = bea;
	LOG("copy: %s", bea2.address());

	EA<char>* beaClone = bea.clone();
	LOG("clone: %s", beaClone->address());
	delete beaClone;

	int ia[10] = {3,2,1,6,4,3,2,8,9,0};
	EA<int> bint(ia, ES_ARRAY_LEN(ia), false, MEM_IGNORE);
	for (int i=0; i<bint.length(); i++) {
		printf("%d,", bint[i]);
	}
	printf("\n");
	bint.sort();
	for (int i=0; i<bint.length(); i++) {
		printf("%d,", bint[i]);
	}
	printf("\n");

	char *ss = new char[5]; //right!
//	char *ss = (char*)eso_malloc(5); //error!
	ss[0] = 'a';
	ss[1] = 'c';
	ss[2] = 'd';
	ss[3] = 'b';
	ss[4] = '\0';
	EA<char> sseb(ss, eso_strlen(ss), true, MEM_NEW);
	printf("ss=%s\n", sseb.address());
	sseb.sort(0, 4);
	printf("ss=%s\n", sseb.address());
#endif

	byte b1[10] = {0, 1,2,3,4,5,6,7,8,9};
	byte b2[10] = {1,2,3,4,5,6,7,8,9, 0};
	ESystem::arraycopy(b1, 0, b2, 0, 10);
	for (int i=0; i<sizeof(b2); i++) {
		LOG("b2=%d", b2[i]);
	}
	ESystem::arraycopy(b1, 2, b1, 5, 5);
	for (int i=0; i<sizeof(b1); i++) {
		LOG("b1=%d", b1[i]);
	}

#if 1
	EA<EA<int>*> bb(10);
	for (int i=0; i<bb.length(); i++) {
		bb[i] = new EA<int>(2);
		(*bb[i])[0] = i;
		(*bb[i])[1] = i;
	}
	for (int i=0; i<bb.length(); i++) {
		EA<int>* x = bb[i];
		LOG("i=%d, %d:%d", i, x->getAt(0), x->getAt(1));
	}
#endif
}

static void test_buffer() {
	EByteBuffer buf__;
	int len = 0;

	{
	EByteBuffer buf;
	len = buf.append('c');
	len += buf.append(81);
	len += buf.append("1234567890");
	for (int i=0; i<5; i++) {
		len += buf.append("1234567890");
	}
	LOG("len=%d,buf.len=%d .cap=%d .limit=%d,buf.data=%s", len, buf.size(), buf.capacity(), buf.limit(), buf.data());

	len = buf.erase(50, 2);
	LOG("len=%d,buf.len=%d .cap=%d .limit=%d,buf.data=%s", len, buf.size(), buf.capacity(), buf.limit(), buf.data());

	sp<EA<byte> > arr = buf.reset();
	LOG("arr.len=%d,arr.data=%s", arr->length(), arr->address());

	buf.setLimit(20);
	buf.setGrowSize(100);

	len = 0;
	for (int i=0; i<5; i++) {
		len += buf.append("1234567890");
	}
	LOG("len=%d,buf.len=%d .cap=%d .limit=%d,buf.data=%s", len, buf.size(), buf.capacity(), buf.limit(), buf.data());

	len = buf.insert(10, "abcdefg");
	LOG("len=%d,buf.len=%d .cap=%d .limit=%d,buf.data=%s", len, buf.size(), buf.capacity(), buf.limit(), buf.data());

	len = buf.insert(10, "zxcvbnmasdfghjkqwertyu");
	LOG("len=%d,buf.len=%d .cap=%d .limit=%d,buf.data=%s", len, buf.size(), buf.capacity(), buf.limit(), buf.data());

	LOG("buf.limit=%d", buf.limit());

	len = buf.update(18, "mmmm", 4);
	LOG("len=%d,buf.len=%d .cap=%d .limit=%d,buf.data=%s", len, buf.size(), buf.capacity(), buf.limit(), buf.data());

	len = buf.zero(18, 4);
	LOG("len=%d,buf.len=%d .cap=%d .limit=%d,buf.data=%s", len, buf.size(), buf.capacity(), buf.limit(), buf.data());

	buf.setLimit(50);

	for (int i=0; i<5; i++) {
		len += buf.append("1234567890");
	}
	LOG("len=%d,buf.len=%d .cap=%d .limit=%d,buf.data=%s", len, buf.size(), buf.capacity(), buf.limit(), buf.data());

	len = buf.update(18, "xxxx", 4);
	LOG("len=%d,buf.len=%d .cap=%d .limit=%d,buf.data=%s", len, buf.size(), buf.capacity(), buf.limit(), buf.data());

	EByteBuffer buf_(buf);
	LOG("len=%d,buf_.len=%d .cap=%d .limit=%d,buf_.data=%s", len, buf_.size(), buf_.capacity(), buf_.limit(), buf_.data());

	buf_ = buf;
	LOG("len=%d,buf_.len=%d .cap=%d .limit=%d,buf_.data=%s", len, buf_.size(), buf_.capacity(), buf_.limit(), buf_.data());

	buf__ = buf_;
	}
	LOG("len=%d,buf__.len=%d .cap=%d .limit=%d,buf__.data=%s", len, buf__.size(), buf__.capacity(), buf__.limit(), buf__.data());
}

static void test_bits()
{
	EByteBuffer ba(32);
	ba.zero(0, 100);

	EBits::putFloat(ba, 0, 3.009);
	float f = EBits::getFloat(ba, 0);
	LOG("test_bits() f=%f", f);

	EBits::putDouble(ba, 0, 377777777777.009);
	double d = EBits::getDouble(ba, 0);
	LOG("test_bits() d=%.5lf", d);
}

static void test_float()
{
	EFloat f(0.998);
	LOG("f1=%f", f.floatValue());

	LOG("f2=%s", f.toString(4.5544).c_str());

    EFloat t1 = EFloat::valueOf(4.3434);
    LOG("f3 %d", EFloat::valueOf(4.3434).equals(&t1));
}

static void test_double()
{
	EDouble d(0.998);
	LOG("d1=%lf", d.doubleValue());

	LOG("d2=%s", d.toString(4.5544).c_str());

    EDouble t1 = EDouble::valueOf(4.3434);
    LOG("d3 %d", EDouble::valueOf(4.3434).equals(&t1));
}

static void test_filestream()
{
	EFileInputStream fis("/tmp/f.out");
	LOG("fis size1=%d", fis.available());
	fis.skip(10);
	LOG("fis size2=%d", fis.available());

	EFileOutputStream fos("/tmp/f.out");
	fos.write("1234567890", 10);
	fos.flush();
	EFileOutputStream fos2(fos.getFD());
	fos.write("0987654321", 10);
}

static void test_bytearraystream()
{
	EByteArrayOutputStream baos;

	baos.write("123456789", 8);
	EString s = baos.toString();
	LOG("s=%s", s.c_str());
}

static void test_randomaccessfile()
{
	//read
	if (1)
	{
		ERandomAccessFile raf("sample.conf", "rb");

		char buffer[11] = {0};
		raf.seek(-sizeof(buffer), ES_SEEK_END);
		raf.read(buffer, sizeof(buffer) - 1);
		LOG("buffer=[%s]", buffer);
		char *p = eso_new_bytes2hexstr((es_uint8_t*)buffer, sizeof(buffer));
		LOG("hexstr=[%s]", p);
		eso_free(p);

//		raf.setLength(100);
	}

	//read-write
	if (1)
	{
		ERandomAccessFile raf("/tmp/xxx", "rw");

		raf.writeInt(112233);
		raf.seek(0);
		int v = raf.readInt();
		LOG("int v=%d", v);

		raf.seek(0);
		raf.writeDouble(999.989928);
		raf.seek(0);
		double d = raf.readDouble();
		LOG("double d=%lf", d);

		raf.seek(-4, ES_SEEK_END);
		raf.writeInt(332211);
		raf.seek(-4, ES_SEEK_END);
		v = raf.readInt();
		LOG("int v=%d", v);

		raf.setLength(200);
	}
}

static void test_randomaccessfile2() {
	llong readfilelen = 0;
	ERandomAccessFile *brafReadFile, *brafWriteFile;

	brafReadFile = new ERandomAccessFile("/tmp/f1", "r");
	readfilelen = brafReadFile->length();
	brafWriteFile = new ERandomAccessFile("/tmp/f2", "rw");

	byte buf[1024];
	int readcount;

	llong start = ESystem::currentTimeMillis();

	while((readcount = brafReadFile->read(buf, sizeof(buf))) != -1) {
		brafWriteFile->write(buf, readcount);
	}

	brafWriteFile->close();
	brafReadFile->close();

	LOG("ERandomAccessFile Copy & Write File: xxx"
						" FileSize: %d"
						" (KB)    "
						"Spend: %lf"
						"(s)",
					   (int)readfilelen >> 1024,
					   (double)(ESystem::currentTimeMillis()-start) / 1000
					   );
	delete brafWriteFile;
	delete brafReadFile;

	EFileInputStream* fdin = new EFileInputStream("/tmp/f1");
	fdin->setIOBuffered(true);
	EBufferedInputStream* bis = new EBufferedInputStream(fdin, 1024);
	EDataInputStream* dis = new EDataInputStream(bis);

	EFileOutputStream* fdout = new EFileOutputStream("/tmp/f2");
	fdout->setIOBuffered(true);
	EBufferedOutputStream* bos = new EBufferedOutputStream(fdout, 1024);
	EDataOutputStream* dos = new EDataOutputStream(bos);

	start = ESystem::currentTimeMillis();

	char bb[4];
	for (int i = 0; i < readfilelen; i++) {
//		fdin->read();
//		bis->read();
//		dis->readByte();
//		int c=fdin->read(); if (c>=0) fdout->write(c); //readfilelen+1

//		fdout->write(fdin->read());
//		if (i==readfilelen / 2) {
//			fdin->setIOBuffered(false);
//		}

		int n = fdin->read(bb, sizeof(bb));
		if (n < 0) break;
		fdout->write(bb, n);
		if (i % 1014 == 0) {
			fdin->setIOBuffered(!fdin->isIOBuffered());
		}

//		dos->write(dis->readByte());
	}

	dos->close();
	dis->close();

	LOG("DataBufferedios Copy & Write File: xxx"
							" FileSize: %d"
							" (KB)    "
							"Spend: %lf"
							"(s)",
						   (int)readfilelen >> 1024,
						   (double)(ESystem::currentTimeMillis()-start) / 1000
						   );
	delete bis;
	delete dis;
	delete fdin;
	delete bos;
	delete dos;
	delete fdout;
}

static void test_pipedstream() {
	if (0) {
		EPipedInputStream pis;
		EPipedOutputStream pos;
		pos.connect(&pis);
		pos.write("1234567890", 10);
		pos.close();
		char buffer[32];
		pis.read(buffer, sizeof(buffer));
		LOG("buffer=%s", buffer);
	}

	if (1) {
		EPipedInputStream pis;
		EPipedOutputStream pos;
		pos.connect(&pis);

		es_proc_t proc;
		ES_PROC_INIT(&proc);
		es_status_t st = eso_proc_fork(&proc);
		if (st == ES_INCHILD) {
			pis.close();

			LOG("in child");
			pos.write("1234567890", 10);
			pos.close();
			exit(0);
		} else if (st == ES_INPARENT) {
			pos.close();

			LOG("in parent");
			eso_thread_sleep(1000);

			char buffer[32];
			pis.read(buffer, sizeof(buffer));
			LOG("buffer=%s", buffer);
			eso_proc_wait(&proc, NULL, NULL, ES_WAIT);
		}
	}
}

class ChildProcess: public ERunnable {
public:
	ChildProcess() {
	}
	virtual ~ChildProcess() {
	}
	virtual void run() {
		for (int i=0; i<100; i++) {
			LOG("child1 running..., i=%d", i);
		}
		exit(0);
	}
};

class ForkProcess : public EFork
{
public:
	ForkProcess() {
		count = 0;
	}
	ForkProcess(const char *name) : EFork(name) {

	}
	virtual void run() {
//		for (int i=0; i<100; i++) {
//			LOG("child2 running..., i=%d", i);
//		}
//		exit(0);

		while (1) {
			LOG("child2 running..., count=%d", count++);
		}
	}
private:
	int count;
};

static void test_fork()
{
	ChildProcess child;
	EFork child1(&child, "child1");
	child1.start();
	LOG("before wait1.");
	child1.waitFor();
	LOG("after wait1.");

#ifndef WIN32
	ForkProcess child2("child2");
	child2.start();
	LOG("before wait2.");
	EThread::sleep(10000);
	child2.kill(SIGKILL);
	child1.waitFor();
	LOG("after wait2.");
#endif
}

static void test_process() {
	EProcess process;
//	process.commandArguments("/cygdrive/d/Downloads/tail.exe", "/cygdrive/d/Downloads/1.c", NULL);
//	process.commandArguments("/bin/ls", "-l", NULL);
//	process.command("/bin/ls -l");
	process.commandArguments("/bin/ls", "-l", NULL);
	process.environmentList("PATH=/bin", NULL);
	process.start();

	char buffer[512] = {0};
	EInputStream *is = process.getInputStream();
	while ((is->read(buffer, sizeof(buffer) - 1)) > 0) {
		LOG("buffer=%s", buffer);
		eso_memset(buffer, 0, sizeof(buffer));
	}

	process.waitFor();
}

static void test_inetaddress() {
	EInetAddress localAddr = EInetAddress::getLocalHost();
	LOG("hostName=%s", localAddr.getHostName());
	LOG("canonicalHostName=%s", localAddr.getCanonicalHostName());
	LOG("localAddr=%s", localAddr.toString().c_str());

	try {
		//EInetAddress zzqAddr = EInetAddress::getByName("ZZQ"); //EUnknownHostException
		EInetAddress zzqAddr = EInetAddress::getByName("www.baidu.com");
		LOG("hostName=%s", zzqAddr.getHostName());
		LOG("canonicalHostName=%s", zzqAddr.getCanonicalHostName());
	} catch (EUnknownHostException& e) {
		e.printStackTrace();
	}

	try {
		EInetAddress zzqAddr = EInetAddress::getByName("141.146.8.66");
		LOG("hostName=%s", zzqAddr.getHostName());
		LOG("canonicalHostName=%s", zzqAddr.getCanonicalHostName());

		EInetAddress zzqAddr2 = EInetAddress::getByName("1.2.3.4");
		LOG("hostName=%s", zzqAddr2.getHostName());
		LOG("canonicalHostName=%s", zzqAddr2.getCanonicalHostName());
	} catch (EUnknownHostException& e) {
		e.printStackTrace();
	}

//	return;

	EInetAddress ia127 = EInetAddress::getByName("127.0.0.1");
	boolean reachable = ia127.isReachable(3000);
	LOG("ip=%s, reachable=%d", ia127.getHostAddress().c_str(), reachable);

	EInetAddress ia = EInetAddress::getByName("0.0.0.10");
	LOG("ip(int)=%d", ia.getAddress());

	EArray<EInetAddress*> arr1 = EInetAddress::getAllByName("www.baidu.com");
	for (int i = 0; i < arr1.size(); i++) {
		LOG("addr1=%s", arr1[i]->getHostAddress().c_str());
	}

	reachable = arr1[0]->isReachable(3000);
	LOG("ip=%s, reachable=%d", arr1[0]->getHostAddress().c_str(), reachable);

	EArray<EInetAddress*> arr2 = EInetAddress::getAllByName("11.22.33.44");
	LOG("addr2=%s, %d", arr2[0]->getHostAddress().c_str(), arr2[0]->getAddress());
	reachable = arr2[0]->isReachable(3000);
	LOG("ip=%s, reachable=%d", arr2[0]->getHostAddress().c_str(), reachable);

//	return;

	EArray<EInetAddress*> arr3 = EInetAddress::getAllByName(null);
	LOG("addr3=%s", arr3[0]->getHostAddress().c_str());

	try {
		EArray<EInetAddress*> arr4 = EInetAddress::getAllByName("[fe80::f66d:4ff:fe54:985f]");
		LOG("addr4=%s", arr4[0]->getHostAddress().c_str());
	} catch (EException &e) {
		LOG("get address from IPv6 fail.");
		e.printStackTrace();
	}

	//test copy constructor
	{
		EInetAddress localAddr_(localAddr);
		LOG("localAddr_=%s", localAddr_.toString().c_str());
	}
}

static void test_socket() {
	char buffer[4096];
	int ret;
	EInetSocketAddress isa("www.baidu.com", 80);
	//test copy constructor
	{
		EInetSocketAddress isa_(isa);
		LOG("isa_=%s", isa_.toString().c_str());
	}
//	EInetSocketAddress isa("32.3.2.3", 890); //test connect timeout
//	ESocket *socket = new ESocket("www.baidu.com", 80);
//	ESocket *socket = new ESocket(isa.getAddress(), 80);
//	ESocket *socket = new ESocket("www.baidu.com", 80, &localAddr, 8898);
//	ESocket *socket = new ESocket(isa.getAddress(), 80, &localAddr, 8898);
	ESocket *socket = new ESocket();
	socket->setReceiveBufferSize(10240);
//	socket->connect(&isa, 3000);
	socket->connect("www.baidu.com", 80, 3000);
//	LOG("laddr=%s, lport=%d", socket->getLocalAddress()->getHostName(), socket->getLocalPort());
	socket->setSoTimeout(3000);
	char *get_str = "GET / HTTP/1.1\r\n"
					"Accept: image/gif, image/jpeg, image/pjpeg, image/pjpeg, application/x-shockwave-flash, application/msword, application/vnd.ms-excel, application/vnd.ms-powerpoint, application/xaml+xml, application/x-ms-xbap, application/x-ms-application, */*\r\n"
					"Accept-Language: zh-cn\r\n"
					"User-Agent: Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 5.1; Trident/4.0; .NET4.0C; .NET4.0E; .NET CLR 2.0.50727)\r\n"
					"Accept-Encoding: gzip, deflate\r\n"
					"Host: www.baidu.com\r\n"
					"Connection: Keep-Alive\r\n"
					"Cookie: BAIDUID=72CBD0B204EC83BF3C5C0FA7A9C89637:FG=1\r\n\r\n";
//	ESocketOutputStream *sos = (ESocketOutputStream*)socket->getOutputStream();
//	ESocketInputStream *sis = (ESocketInputStream*)socket->getInputStream();
	EOutputStream *sos = socket->getOutputStream();
	EInputStream *sis = socket->getInputStream();
//	socket->shutdownInput();
	sos->write(get_str, strlen(get_str));
	LOG("socket available=[%d]", sis->available());
	try {
		while ((ret = sis->read(buffer, sizeof(buffer))) > 0) {
			//	EThread::sleep(1000);
			LOG("socket available=[%d]", sis->available());
			LOG("socket read=[%s]", buffer);
		}
	} catch (...) {
	}
	sis->close();
	sos->close();
	socket->close();
	delete socket;

	ESocket sock;
}

static void test_serversocket() {
//	EServerSocket *serverSocket = new EServerSocket(8787);
	EServerSocket *serverSocket = new EServerSocket();
	serverSocket->setReuseAddress(true);
	serverSocket->bind(8787);
	LOG("serverSocket=%s", serverSocket->toString().c_str());

#if 0
	// test accept interrupt.
	class CloseThread: public EThread {
	public:
		CloseThread(EServerSocket *ss): serverSocket(ss) {}
		virtual void run() {
			EThread::sleep(3000);
			serverSocket->close();
			LOG("accept socket closed.");
		}
	private:
		EServerSocket *serverSocket;
	};
	CloseThread ct(serverSocket);
	ct.start();
#endif

	// accept
	int count = 0;
	char buffer[11];
	while (count < 10) {
		ESocket *clientSocket;
		try {
			clientSocket = serverSocket->accept();
		} catch (EIOException& e) {
			LOG("accept error.");
			break;
		}
		count++;
		EInetSocketAddress *isar = clientSocket->getRemoteSocketAddress();
		EInetSocketAddress *isal = clientSocket->getLocalSocketAddress();
		LOG("socket rip=[%s], rport=%d", isar->getHostName(), isar->getPort());
		LOG("socket lip=[%s], lport=%d", isal->getHostName(), isal->getPort());
		try {
//			ESocketInputStream *sis = (ESocketInputStream*)clientSocket->getInputStream();
			EInputStream *sis = clientSocket->getInputStream();
			eso_memset(buffer, 0, sizeof(buffer) - 1);
			sis->read(buffer, sizeof(buffer));
			LOG("socket read=[%s]", buffer);
		} catch (EIOException &e) {
			LOG("read e=%s", e.toString().c_str());
		}
		delete clientSocket;
	}
	delete serverSocket;

	EServerSocket serverSock;
}

EString getDateString(llong time) {
	EString str;
    ECalendar *calendar = new ECalendar();

    //calendar->setTimeInMillis(ESystem::currentTimeMillis());
	str = EInteger::toString(calendar->get(ECalendar::YEAR)) + "YEAR "
		+ EInteger::toString(calendar->get(ECalendar::MONTH)+1) + "MONTH "
		+ EInteger::toString(calendar->get(ECalendar::DATE)) + "DATE "
		+ (calendar->get(ECalendar::AM_PM)==ECalendar::PM ? "PM " : "AM ")
		+ EInteger::toString(calendar->get(ECalendar::HOUR)) + "HOUR "
		+ EInteger::toString(calendar->get(ECalendar::MINUTE)) + "MINUTE "
		+ EInteger::toString(calendar->get(ECalendar::SECOND)) + "SECOND";

	delete calendar;

    return str;
}

static void test_calendar()
{
	ECalendar cal;

	LOG("ECalendar::getTimeInMillis()=%lld", cal.getTimeInMillis());
	LOG("ESystem::currentTimeMillis()=%lld", ESystem::currentTimeMillis());

	ECalendar cal_(cal);
	LOG("ECalendar::getTimeInMillis()=%lld", cal_.getTimeInMillis());
	cal_ = cal;
	LOG("ECalendar::getTimeInMillis()=%lld", cal_.getTimeInMillis());
	eso_log("cal_.WEEK_OF_MONTH=%d\n", cal_.get(ECalendar::WEEK_OF_MONTH));

	eso_log("\n");

	eso_log("cal.WEEK_OF_MONTH=%d\n", cal.get(ECalendar::WEEK_OF_MONTH));
	eso_log("cal.WEEK_OF_YEAR=%d\n", cal.get(ECalendar::WEEK_OF_YEAR));
	eso_log("cal.DAY_OF_YEAR=%d\n", cal.get(ECalendar::DAY_OF_YEAR));
	eso_log("cal.DAY_OF_WEEK_IN_MONTH=%d\n", cal.get(ECalendar::DAY_OF_WEEK_IN_MONTH));
	eso_log("cal.ZONE_OFFSET=%d\n", cal.get(ECalendar::ZONE_OFFSET));
	eso_log("cal.DST_OFFSET=%d\n", cal.get(ECalendar::DST_OFFSET));

	eso_log("\n");

	eso_log("cal.year=%d\n", cal.get(ECalendar::YEAR));
	eso_log("cal.mon=%d\n", cal.get(ECalendar::MONTH));
	eso_log("cal.date=%d\n", cal.get(ECalendar::DATE));
	eso_log("cal.hour=%d\n", cal.get(ECalendar::HOUR_OF_DAY));
	eso_log("cal.minute=%d\n", cal.get(ECalendar::MINUTE));
	eso_log("cal.second=%d\n", cal.get(ECalendar::SECOND));

	eso_log("cal.toString=%s\n", cal.toString("%Y-%m-%d %H:%M:%S,%z").c_str());

	eso_log("\n");

	cal.set(ECalendar::YEAR, 2002);
	eso_log("cal.year=%d\n", cal.get(ECalendar::YEAR));

	cal.set(ECalendar::MONTH, 33);
	eso_log("cal.year=%d\n", cal.get(ECalendar::YEAR));
	eso_log("cal.mon=%d\n", cal.get(ECalendar::MONTH));

	cal.add(ECalendar::MONTH, 33);
	eso_log("cal.year=%d\n", cal.get(ECalendar::YEAR));
	eso_log("cal.mon=%d\n", cal.get(ECalendar::MONTH));
	eso_log("cal.timezone=%d\n", cal.get(ECalendar::ZONE_OFFSET)/1000/60/60);

	eso_log("\n");

//	cal.setTimeInMillis(ESystem::currentTimeMillis());
	cal.setTimeInMillis(1408413451000);
	eso_log("cal.year=%d\n", cal.get(ECalendar::YEAR));
	eso_log("cal.mon=%d\n", cal.get(ECalendar::MONTH));
	eso_log("cal.date=%d\n", cal.get(ECalendar::DATE));
	eso_log("cal.hour=%d\n", cal.get(ECalendar::HOUR_OF_DAY));
	eso_log("cal.minute=%d\n", cal.get(ECalendar::MINUTE));
	eso_log("cal.second=%d\n", cal.get(ECalendar::SECOND));
	eso_log("cal.toString=%s\n", cal.toString("%Y-%m-%d %H:%M:%S,%z").c_str());

	eso_log("\n");

	cal.setTimeZone(-9);
	eso_log("cal.rawOffset=%d\n", cal.get(ECalendar::ZONE_OFFSET));
	eso_log("cal.year=%d\n", cal.get(ECalendar::YEAR));
	eso_log("cal.mon=%d\n", cal.get(ECalendar::MONTH));
	eso_log("cal.date=%d\n", cal.get(ECalendar::DATE));
	eso_log("cal.hour=%d\n", cal.get(ECalendar::HOUR_OF_DAY));
	eso_log("cal.minute=%d\n", cal.get(ECalendar::MINUTE));
	eso_log("cal.second=%d\n", cal.get(ECalendar::SECOND));
	eso_log("cal.timezone=%d\n", cal.get(ECalendar::ZONE_OFFSET)/1000/60/60);
	eso_log("cal.toString=%s\n", cal.toString("%Y-%m-%d %H:%M:%S,%z").c_str());

	eso_log("\n");

	EString str = getDateString(ESystem::currentTimeMillis());
#ifdef WIN32
	eso_log("tm=%I64d, str=%s\n", ESystem::currentTimeMillis(), str.c_str());
#else
	eso_log("tm=%lld, str=%s\n", ESystem::currentTimeMillis(), str.c_str());
#endif

	cal.set(ECalendar::YEAR, 2002);
	eso_log("cal.year=%d\n", cal.get(ECalendar::YEAR));

	cal.set(ECalendar::MONTH, 33);
	eso_log("cal.year=%d\n", cal.get(ECalendar::YEAR));
	eso_log("cal.mon=%d\n", cal.get(ECalendar::MONTH));

	cal.add(ECalendar::MONTH, 33);
	eso_log("cal.year=%d\n", cal.get(ECalendar::YEAR));
	eso_log("cal.mon=%d\n", cal.get(ECalendar::MONTH));

	EString isoTime = ECalendar::toISO8601String(&cal);
	eso_log("toISO8601String=%s\n", isoTime.c_str());

	eso_log("toString=%s\n", cal.toString().c_str());

	cal.clear();
	eso_log("cal.year=%d\n", cal.get(ECalendar::YEAR));
	eso_log("cal.mon=%d\n", cal.get(ECalendar::MONTH));
	eso_log("cal.date=%d\n", cal.get(ECalendar::DATE));
	eso_log("cal.hour=%d\n", cal.get(ECalendar::HOUR));
	eso_log("cal.hourofday=%d\n", cal.get(ECalendar::HOUR_OF_DAY));
	eso_log("cal.minute=%d\n", cal.get(ECalendar::MINUTE));
	eso_log("cal.second=%d\n", cal.get(ECalendar::SECOND));

	char *datestr = "Y2013 YEAR02 22 DAY12am +08";
	ECalendar cal2 = ECalendar::parseOf("Y%Y YEAR%m %d DAY%H%p %z", datestr);
	eso_log("eso_parse_time...\n");
	eso_log("cal2.year=%d\n", cal2.get(ECalendar::YEAR));
	eso_log("cal2.mon=%d\n", cal2.get(ECalendar::MONTH));
	eso_log("cal2.date=%d\n", cal2.get(ECalendar::DATE));
	eso_log("cal2.hour=%d\n", cal2.get(ECalendar::HOUR_OF_DAY));
	eso_log("cal2.minute=%d\n", cal2.get(ECalendar::MINUTE));
	eso_log("cal2.second=%d\n", cal2.get(ECalendar::SECOND));
	eso_log("cal2.timezone=%d\n", cal2.getTimeZone());
}

static void test_date() {
	EDate d;
	LOG("time=%ld, %s", d.getTime(), d.toString().c_str());

	EDate d_(d);
	LOG("time=%ld, %s", d_.getTime(), d_.toString().c_str());
	d_ = d;
	LOG("time=%ld, %s", d_.getTime(), d_.toString().c_str());

	//testTimestampCastToDateAfterFail()
	EDate d1(1369461400000L);
	EDate d2(1369461400001L);
	LOG("d2 after d1 : %s", d2.after(d1) ? "true" : "false");
}

static void test_bitset()
{
	EBitSet bs1("1001001001111111111111111110011");
	EBitSet bs2("0010001001111111111111111110001");
	LOG("BitSet1=%s", bs1.toString().c_str());
	LOG("BitSet2=%s", bs2.toString().c_str());
	bs1.xor_(&bs2);
	LOG("BitSet1 xor BitSet2 =%s", bs1.toString().c_str());

	EBitSet bs1_(bs1);
	LOG("BitSet1_=%s", bs1_.toString().c_str());

	bs1_ = bs2;
	LOG("BitSet2_=%s", bs1_.toString().c_str());
}

static void test_urlstring()
{
	EURLString *x = new EURLString("xx");
	LOG("x instanceof EURLString : %d", instanceof<EURLString>(x));
	delete x;

	char *s1 = "ABCfdk35+ 67890@#$%^&*()j";
	LOG("s1=%s", s1);
	EString us1 = EURLEncoder::encode(s1);
	LOG("u1=%s", us1.c_str());
	EString ud1 = EURLDecoder::decode(us1.c_str());
	LOG("ud1=%s", ud1.c_str());


	EURLString urls("key=");
	urls.enccat("JKJdfj^&*(dd dfjkdL345624324*").fmtcat("&key2=").enccat("JJ** 9r43").fmtcat("&key3=").enccat("3");
	LOG("urls=%s", urls.c_str());
	EURLString urld("XXX");
	urld.deccat(urls.c_str());
	LOG("urld=%s", urld.c_str());
}

static void test_datastream()
{
	LOG("sizeof llong = %d", sizeof(llong));
	es_byte_t s[8] = {0};
	eso_llong2array(LLONG(344331234567890654), s, 8);
	llong n = (llong)eso_array2llong(s, 8);
	LOG("n=%lld", n);

	es_byte_t t[4] = {0};
	eso_llong2array(34433, t, 4);
	int m = (int)eso_array2llong(t, 4);
	LOG("m=%d", m);

	EFileOutputStream fos("/tmp/gtest.txt");
	EDataOutputStream dos(&fos);
	dos.writeBoolean(true);
	dos.writeByte('a');
	dos.writeShort(2121);
	dos.writeBytes("|afdkaerejkrewkr|\r\n");
	dos.writeDouble(9.99893001);
	dos.write("123456789", 9);
	dos.writeFloat(8.0009);
	dos.writeInt(888888);
	dos.writeLLong(LLONG(333333356757545609));
	dos.close();

	EFileInputStream fis("/tmp/gtest.txt");
	EDataInputStream dis(&fis);
	LOG("readBoolean=%d", dis.readBoolean());
	LOG("readByte=%c", dis.readByte());
	LOG("readShort=%d", dis.readShort());
	LOG("readLine=%s", dis.readLine()->c_str());
	LOG("readDouble=%f", dis.readDouble());
	char buf[512] = {0};
	dis.read(buf, 9);
	LOG("readString=%s", buf);
	LOG("readFloat=%f", dis.readFloat());
	LOG("readInt=%d", dis.readInt());
	LOG("readLLong=%lld", dis.readLLong());

	EFileInputStream fis2("sample.conf");
	EDataInputStream dis2(&fis2);
	sp<EString> line;
	while ((line = dis2.readLine()) != null) {
		LOG("%s", line->c_str());
	}
}

static sp<EA<byte> > zip(byte* data, int offset, int len) {
	try {
		EByteArrayOutputStream baos;
		EGZIPOutputStream gzipOutputStream(&baos);
		gzipOutputStream.write(data + offset, len);
		gzipOutputStream.finish();
		gzipOutputStream.close();

		return baos.reset(false);
	} catch (EIOException& e) {
		//...
	}
	return null;
}

static sp<EA<byte> > unzip(byte* data, int offset, int len) {
	try {
		EByteArrayOutputStream baos;
		EByteArrayInputStream bais(data + offset, len);
		EBufferedInputStream bis(&bais);
		EA<byte> buf(8192);
		int ulen = 0;
		EInputStream* gzis;
		gzis = new EGZIPInputStream(&bis);
		while ((ulen = gzis->read(buf.address(), buf.length())) != -1)
			baos.write(buf.address(), ulen);
		gzis->close();
		delete gzis;
		bis.close();
		baos.flush();
		sp<EA<byte> > out = baos.reset(false);
		baos.close();
		return out;
	} catch(EException& e) {
		//...
	}
	return null;
}

static void test_gzipstream() {
	es_buffer_t* buffer = eso_buffer_make(512, 512);
	eso_file_read_by_filename("/tmp/gtest.zip", 0, buffer);

	sp<EA<byte> > bb1 = zip((byte*)buffer->data, 0, buffer->len);
	eso_file_create("/tmp/gziptst.dat", TRUE, bb1->address(), bb1->length());

	eso_buffer_clear(buffer);
	eso_file_read_by_filename("/tmp/gziptst.dat", 0, buffer);
	sp<EA<byte> > bb2 = unzip((byte*)buffer->data, 0, buffer->len);
	eso_file_create("/tmp/gtest2.zip", TRUE, bb2->address(), bb2->length());

	eso_buffer_free(&buffer);
}

static void test_sequencestream() {
	EByteArrayInputStream bis1((void*)"1234567890", 10);
	EByteArrayInputStream bis2((void*)"0987654321", 10);
	ESequenceInputStream sis(&bis1, &bis2);
	byte c[1];
	while (sis.read(c, 1) > 0) {
		LOG("%d", c[0]);
	}

	LOG("====");

	EVector<EInputStream*> vector(false);
	vector.addElement(&bis1);
	vector.addElement(&bis2);

	sp<EEnumeration<EInputStream*> > e = vector.elements();
	ESequenceInputStream sis2(e.get());
	byte x[1];
	while (sis2.read(x, 1) > 0) {
		LOG("%d", x[0]);
	}
}

static void split(EString& demo){
	EArray<EString*> result = EPattern::split("-", demo.c_str());
	int len = result.size();
	printf("\"%s\" len：%d", demo.c_str(), len);
	if(len >= 0)
	{
		printf(", result: ");
		for(int i=0; i<len; i++)
		{
			printf(" \"%s\"", result[i]->c_str());
		}
	}
	LOG("");
}

static void test_pattern() {
	//1.
	const char* orig = "From:regular.expressions@example.com\r\n"\
            "From:exddd@43434.com\r\n"\
            "From:7853456@exgem.com\r\n";
	EPattern pattern("From:([^@]+)@([^\r]+)");
	EMatcher matcher(&pattern, orig);
	LOG("orig=%s", orig);
	while (matcher.find()) {
		LOG("group count=%d", matcher.groupCount());
		LOG("m0=%s", matcher.group(0).c_str());
		LOG("m1=%s", matcher.group(1).c_str());
		LOG("m2=%s\n", matcher.group(2).c_str());
	}

	//2.
	const char* line = "aXXbcXX";
	EArray<EString*> result = EPattern::split("XX", line);
	LOG("pattern result size=%d", result.size());
	for (int i=0; i<result.size(); i++) {
		LOG("%s", result[i]->c_str());
	}

	EString str1 = "a-b";
	EString str2 = "a-b-";
	EString str3 = "-a-b";
	EString str4 = "-a-b-";
	EString str5 = "a";
	EString str6 = "-";
	EString str7 = "--";
	EString str8 = "";

	split(str1);
	split(str2);
	split(str3);
	split(str4);
	split(str5);
	split(str6);
	split(str7);
	split(str8);
}

class CountAtomicThread: public EThread
{
public:
	CountAtomicThread(const char *name, EAtomicInteger *ai, EAtomicReference<EInteger*>* arp, EAtomicDouble* adp) : EThread(name){
		v = 0;
		a = ai;
		ar = arp;
		ad = adp;
	}
	virtual ~CountAtomicThread() {

	}

	virtual void run() {
		for (int i = 0; i < 100; i++) {
			if (eso_strcmp(this->getName(), "#1")==0) {
				boolean ret = a->compareAndSet(i, 11);
				if (ret) {
					LOG("i=%d, value=%d", i, a->get());
				}
				else {
					v++;
				}

				ad->set(i + 0.0000190001);
			}
			else {
				if (a->get() == 11) {
					LOG("value is 11");
				}

				LOG("ad value: %.10f", ad->get());
			}

			EInteger* o = ar->getAndSet(new EInteger(i));
			if (o) {
				LOG("o=%d", o->intValue());
				delete o;
			}
		}

		if (eso_strcmp(this->getName(), "#1")==0) {
//			LOG("compareAndSet() failed count=%d", v);
		}
	}

private:
	int v;
	EAtomicInteger *a;
	EAtomicReference<EInteger*>* ar;
	EAtomicDouble* ad;
};

static void test_atomic() {
	EAtomicInteger ai(1);
	EAtomicReference<EInteger*> ar;
	EAtomicDouble ad(0.00091);

	CountAtomicThread t1("#1", &ai, &ar, &ad), t2("#2", &ai, &ar, &ad), t3("#3", &ai, &ar, &ad);
	t1.start();
	t2.start();
	t3.start();

	t1.join();
	t2.join();
	t3.join();

	LOG("after thread join. ai value=%d", ai.get());
}

class TestSynchronizedPerformance {
public:

    abstract class TestTemplate {
    public:
    	TestTemplate(const char* name) {
    		_name = name;
    	}

    	virtual void run_() = 0;
    	virtual int value() = 0;

        const char* getName() {
            return _name.c_str();
        }

    private:
        EString _name;
    };

    class AtomicCounterPerformance : public TestTemplate {
	public:
    	EAtomicCounter i;

		AtomicCounterPerformance() : TestTemplate("AtomicCounterPerformance") {
			i = 1;
		}

		void run_() {
			++i;
		}

		int value() {
			return i.value();
		}
	};

    class SynchronizedPerformance : public TestTemplate {
    public:
    	int i;

    	SynchronizedPerformance() : TestTemplate("SynchronizedPerformance") {
    		i = 1;
    	}

        void run_() {
        	eso_atomic_add_and_fetch32(&i, 1);
        }

        int value() {
			return i;
		}
    };

	class LockPerformance: public TestTemplate {
	public:
		int i;
		ELock *lock;

		LockPerformance() : TestTemplate("LockPerformance") {
			i = 1;
			lock = new EReentrantLock();
		}

		~LockPerformance() {
			delete lock;
		}

		void run_() {
			lock->lock();
			i += 1;
			lock->unlock();
		}

		int value() {
			return i;
		}
	};

    void performance(TestTemplate* test, int threadnum, int count) {
        EAtomicLLong *start = new EAtomicLLong();
        EAtomicLLong *end = new EAtomicLLong();
        EAtomicInteger *counter = new EAtomicInteger(0);

        class Counter: public ERunnable {
        public:
        	Counter(TestTemplate* test, EAtomicLLong *start, EAtomicLLong *end, EAtomicInteger *counter, int count) {
        		m_test = test;
        		m_start = start;
        		m_end = end;
        		m_counter = counter;
        		m_count = count;
        	}

			virtual void run() {
//				for (int i=0; i<1000000; i++) {
//					m_test->run_();
//				}

				while (true ) {
					m_test->run_();
					int runCount = m_counter->incrementAndGet();
					if (runCount == 1) {
						m_start->set(ESystem::nanoTime());
					} else if (runCount >= m_count) {
						if (runCount == m_count) {
							m_end->set(ESystem::nanoTime());
							LOG("Test Name: %s Time: %ld",
									m_test->getName(),
									(m_end->llongValue() - m_start->llongValue())
											/ 1000);
						}
						break;
					}
				}
			}

        private:
        	TestTemplate *m_test;
        	EAtomicLLong *m_start;
        	EAtomicLLong *m_end;
        	EAtomicInteger *m_counter;
        	int m_count;
        };

        EArray<EThread*> arr;
		int i;
        for (i = 0; i < threadnum; i++) {
            EThread *t = new EThread(new Counter(test, start, end, counter, count));
            t->start();
            arr.add(t);
        }
        for (i = 0; i < arr.length(); i++) {
        	arr.getAt(i)->join();
        }

        delete counter;
        delete start;
        delete end;
    }

    void dowork() {
        int threadnum = 100;
        int count = 10000;
        AtomicCounterPerformance *a1 = new AtomicCounterPerformance();
        SynchronizedPerformance *a2 = new SynchronizedPerformance();
        LockPerformance *a3 = new LockPerformance();
        performance(a1, threadnum, threadnum * count);
        performance(a2, threadnum, threadnum * count);
        performance(a3, threadnum, threadnum * count);
        LOG("Test Name: %s, value=%d", a1->getName(), a1->value());
        LOG("Test Name: %s, value=%d", a2->getName(), a2->value());
        LOG("Test Name: %s, value=%d", a3->getName(), a3->value());
        delete a1;
        delete a2;
        delete a3;
    }
};

static void test_atomic2() {
	TestSynchronizedPerformance *tsp = new TestSynchronizedPerformance();
	tsp->dowork();
	delete tsp;
}

static void test_atomicReference_sp() {
	EAtomicReference<EInteger> ar;
	ar.set(new EInteger(0));

	class Thread : public EThread {
	private:
		EAtomicReference<EInteger>* ar;
	public:
		Thread(EAtomicReference<EInteger>* ar, const char* name) : EThread(name), ar(ar) {
		}
		virtual void run() {
			for (int i=0; i<1000; i++) {
				if (EString("#1").equals(getName())) {
					sp<EInteger> old = ar->getAndSet(new EInteger(10000+i));
					LOG("#1 i=%d", old->intValue());
				}

				if (EString("#2").equals(getName())) {
					sp<EInteger> old = ar->getAndSet(new EInteger(20000+i));
					LOG("#2 i=%d", old->intValue());
				}
			}
		}
	};

	Thread thread1(&ar, "#1");
	Thread thread2(&ar, "#2");

	thread1.start();
	thread2.start();

	thread1.join();
	thread2.join();

	LOG("end of test_atomicreference_sp().");
}

class CB;
class CA {
public:
	CA();
	~CA();
	CB* createCB();
private:
	friend class CB;
	CB* o;boolean deleteing;
};
class CB {
public:
	CB(CA *ca) :
			deleteing(false ) {
		o = ca;
	}
	~CB() {
		LOG("~CB");
		deleteing = true;
		if (o && !o->deleteing) {
			delete o;
		}
	}
private:
	friend class CA;
	CA* o;boolean deleteing;
};
CA::CA() :
		o(null), deleteing(false ) {
}
CA::~CA() {
	LOG("~CA");
	deleteing = true;
	if (o && !o->deleteing) {
		delete o;
	}
}
CB* CA::createCB() {
	if (!o) {
		o = new CB(this);
	}
	return o;
}

static void test_delete() {
    class AA {
    public:
        ~AA() {
            printf("~AA\n");
        }
    };
    
    AA** aa = new AA*[10];
	int i;
    for (i=0; i<10; i++) {
        aa[i] = new AA();
    }
    for (i=0; i<10; i++) {
        delete aa[i];
    }
    delete[] aa;
    
//    AA* bb = new AA[10];
//    delete[] bb;

    //=================

    CA* ca = new CA();
    CB* cb = ca->createCB();
    delete cb;
}

static void test_collections() {
	//test synchronizedCollection
	{
		ELinkedList<int> ll;
		ECollection<int> *l = ECollections::synchronizedCollection(&ll,
				null, false );
		l->add(100);
		l->add(300);
		l->add(200);

		sp<EIterator<int> > iter = l->iterator();
		while (iter->hasNext()) {
			int n = iter->next();
			printf("n=%d\n", n);
		}

		printf("\n");
		ECollections::sort(&ll);

		iter = l->iterator();
		while (iter->hasNext()) {
			int n = iter->next();
			printf("n=%d\n", n);
		}

		delete l;
	}

	//test synchronizedList
	{
		ELinkedList<int> ll;
		ECollection<int> *l = ECollections::synchronizedList(&ll,
				null, false );
		l->add(100);
		l->add(200);
		l->add(300);

		sp<EIterator<int> > iter = l->iterator();
		while (iter->hasNext()) {
			int n = iter->next();
			printf("n=%d\n", n);
		}

		delete l;
	}

	//test SynchronizedMap
	{
		EHashMap<EInteger*,EString*> hm;
		EMap<EInteger*,EString*> *m = ECollections::synchronizedMap(&hm,
				null, false );
		m->put(new EInteger(1), new EString("#1"));
		m->put(new EInteger(2), new EString("#2"));
		m->put(new EInteger(3), new EString("#3"));

		ESet<EMapEntry<EInteger*,EString*>*>* me = m->entrySet();
		sp<EIterator<EMapEntry<EInteger*,EString*>*> > iter = me->iterator();
		while (iter->hasNext()) {
			EMapEntry<EInteger*,EString*>*mi = iter->next();
			printf("1 n=%d, s=%s\n", mi->getKey()->intValue(), mi->getValue()->c_str());
		}

		//test unmodifiable wrappers
		ECollections::UnmodifiableMap<EInteger*,EString*> um(&hm);
		ESet<EMapEntry<EInteger*,EString*>*>* me2 = um.entrySet();
		sp<EIterator<EMapEntry<EInteger*,EString*>*> > iter2 = me2->iterator();
		while (iter2->hasNext()) {
			EMapEntry<EInteger*,EString*>*mi = iter2->next();
			printf("2 n=%d, s=%s\n", mi->getKey()->intValue(), mi->getValue()->c_str());
		}

		delete m;
	}
}

static void test_instanceof() {
	EHashMap<EString*, EString*>* hm = new EHashMap<EString*, EString*>();
	EObject* o = (EObject*)hm;
	if (instanceof<EHashMap<EString*, EString*> >(o)) {
		LOG("o instanceof EHashMap: true");
	}
	if (instanceof<EMap<EString*, EString*> >(o)) {
		LOG("o instanceof EMap: true");
	}
	if (instanceof<EList<EString*> >(o)) {
		LOG("o instanceof EList: false");
	}
	if (instanceof<EObject>(o)) {
		LOG("o instanceof EObject: true");
	}
	if (instanceof<EAbstractMap<EString*, EString*> >(o)) {
		LOG("o instanceof EAbstractMap: true");
	}
	delete hm;
}

class RcThread1 : public EThread
{
private:
	Class* c;
public:
	RcThread1(Class* c) {
		this->c = c;
	}
	virtual void run() {
		for (int i = 0; i < 10000; i++) {
			Class* cc = GETRC(c);
			for (int j = 0; j < 100; j++) {
				Class* dd = GETRC(c);
				LOG("str=%s", c->c_str());
				DELRC(dd);
			}
			DELRC(cc);
		}
	}
};

class RcThread2 : public EThread
{
private:
	Class* c;
public:
	RcThread2(Class* c) {
		this->c = c;
	}
	virtual void run() {
		for (int i = 0; i < 10000; i++) {
			Class* cc = GETRC(c);
			for (int j = 0; j < 100; j++) {
				Class* dd = GETRC(c);
				LOG("str=%s", c->c_str());
				DELRC(dd);
			}
			DELRC(cc);
		}
	}
};

static void test_NEWRC() {
	Class* c = NEWRC(Class)();

	EArray<EThread*> arr;
	int i;
	for (i = 0; i < 1; i++) {
		RcThread1* sThread1 = new RcThread1(c);
		sThread1->start();// put
		arr.add(sThread1);
	}

	for (i = 0; i < 3; i++) {
		RcThread2* sThread2 = new RcThread2(c);
		sThread2->start(); // get
		arr.add(sThread2);
	}

	for (i = 0; i < arr.length(); i++) {
		arr.getAt(i)->join();
	}

	DELRC(c);
}

static sp<EString> g_sp(new EString("111"));

static void test_sp()
{
	{
		sp<Class> s = new Class();
		{
			sp<Class>* s2 = new sp<Class>(s);
			delete s2;
		}
	}

	{
		sp<Class> c(new Class());

		struct A {
			static void f(sp<Class>& s) {
				sp<Class> t = s;
				LOG("t=%s", (t!=null) ? t->c_str() : "null");
			}
		};
		A::f(c);

	//	A::f(sp<Class>(null)); //
	//	A::f(sp<Class>(new Class()));

		sp<Class> xx(c);
		A::f(xx);
	}

	{
		class spReader : public EThread {
		public:
			virtual void run() {
			   while (true) {
				   sp<EString> s = atomic_load(&g_sp);
				   // do smth
				   LOG("s=%s",  (s!=null) ? s->c_str() : "null");
			   }
			}
		};

		class spWriter : public EThread {
		public:
			virtual void run() {
				while (true) {
					sp<EString> newPtr(new EString("xxx"));
					atomic_store(&g_sp, newPtr);
			   }
			}
		};

		EArray<EThread*> arr;
		int i;
		for (i = 0; i < 2; i++) {
			spReader* sThread1 = new spReader();
			sThread1->start();
			arr.add(sThread1);
		}

		for (i = 0; i < 3; i++) {
			spWriter* sThread2 = new spWriter();
			sThread2->start();
			arr.add(sThread2);
		}

		for (i = 0; i < arr.length(); i++) {
			arr.getAt(i)->join();
		}
	}
}

class ClassX : public EObject {
public:
	virtual ~ClassX() {
		LOG("ClassX(%d)", v);
	}
	ClassX(int v) : v(v) {
		LOG("ClassX(%d)", v);
	}
	int value() {
		return v;
	}
private:
	int v;
};
static void test_array2() {
	EA<sp<ClassX> > sea(10);
	int i;
	for (i=0; i<sea.length(); i++) {
		sea[i] = new ClassX(i);
	}
	for (i=0; i<sea.length(); i++) {
		LOG("v=%d", sea[i]->value());
	}

	EA<sp<ClassX> > sea_(sea);
	sp<ClassX> old = sea_.setAt(1, new ClassX(1111));
	LOG("sea4 old=%d", old->value());
	for (i=0; i<sea_.length(); i++) {
		LOG("v_=%d", sea_[i]->value());
	}

	EA<sp<ClassX> > eacopy(10);
	ESystem::arraycopy(&sea, 0, &eacopy, 0, 10);
	for (i=0; i<eacopy.length(); i++) {
		LOG("eacopy v=%d", eacopy[i]->value());
	}
	ESystem::arraycopy(&eacopy, 2, &eacopy, 0, 8);
	for (i=0; i<eacopy.length(); i++) {
		LOG("eacopy_ v=%d", eacopy[i]->value());
	}
	ESystem::arraycopy(&eacopy, 2, &eacopy, 5, 5);
	for (i=0; i<eacopy.length(); i++) {
		LOG("eacopy__ v=%d", eacopy[i]->value());
	}

	EA<sp<Class> > *sea2 = NEWRC(EA<sp<Class> >)(10);
	DELRC(sea2);

	LOG("test_ea()...0");
	ESystem::arraycopy(sea, 3, sea, 0, 5);
	LOG("test_ea()...1");
	ESystem::arraycopy(sea, 0, sea, 3, 5);
	LOG("test_ea()...2");
	ESystem::arraycopy(sea, 5, sea, 2, 5);
	LOG("test_ea()...3");
	EA<sp<ClassX> > sea3(10);
	ESystem::arraycopy(sea, 0, sea3, 0, 5);
	EA<sp<ClassX> > sea4(10);
	ESystem::arraycopy(sea, 0, sea4, 0, 10);
	for (i=0; i<sea4.length(); i++) {
		sp<ClassX> x = sea4.getAt(i);
		LOG("v=%d", (x != null) ? x->value() : -1);
	}
	sea4.clear();
	LOG("sea4 length=%d", sea4.length());
	for (i=0; i<sea4.length(); i++) {
		sp<ClassX> x = sea4[i];
		LOG("v=%d", (x != null) ? x->value() : -1);
	}

	LOG("end of test_ea().");
}

class SpEntry {
public:
		sp<EString> s;
};
static EA<SpEntry*>* volatile spTestArr = null;

class RCThread1 : public EThread
{
public:
	virtual void run() {
		for (int k = 0; k < 10; k++) {
			EA<SpEntry*>* arr = GETRC(spTestArr);
			for (int i = 0; i < arr->length(); i++) {
				if ((*arr)[i] == null) {
					SpEntry* e = new SpEntry();
					EString* s = new EString("xxxxxxxxxxxxxxxxxx");
					s->fmtcat(",%d", i);
					e->s = s;
					(*arr)[i] = e;
				}
	//			LOG("text1=%s", text->c_str());

	//			EThread::sleep(10);
			}
			DELRC(arr);
		}
	}
};

class RCThread2 : public EThread
{
public:
	virtual void run() {
		for (int i = 0; i < 100000; i++) {
			EA<SpEntry*>* arr = GETRC(spTestArr);
			for (int i = 0; i < arr->length(); i++) {
				SpEntry* e = (*arr)[i];
//				if (e != null && e->s != null)
//					LOG("text2=%s", e->s->c_str());
			}
			DELRC(arr);
		}
	}
};

static void test_rc()
{
//	while (true) {
		spTestArr = NEWRC(EA<SpEntry*>)(100000, false);

		EArray<EThread*> arr;
		int i;
		for (i = 0; i < 1; i++) {
			RCThread1* sThread1 = new RCThread1();
			sThread1->start(); // put
				arr.add(sThread1);
			}

			for (i = 0; i < 3; i++) {
				RCThread2* sThread2 = new RCThread2();
				sThread2->start(); // get
				arr.add(sThread2);
			}

			for (i = 0; i < arr.length(); i++) {
				arr.getAt(i)->join();
			}

			if (spTestArr) {
				spTestArr->setAutoFree(true);
			}
			DELRC(spTestArr);
//	}
}

#if 0
class CopyOnWriteClass : public EObject {
private:
	EArrayList<EString*>* arrayList;
	EArrayList<EString*>* arrayListFree;

	EReentrantLock lock;

	EArrayList<EString*>* tackArray() {
		EArrayList<EString*>* arr;
		SYNCBLOCK(&lock) {
			arr = arrayList;
			arr->refCount++;
			arrayListFree->refCount++;
		}}
		return arr;
	}

	void backArray(EArrayList<EString*>* arr) {
		SYNCBLOCK(&lock) {
			if (arr->refCount-- == 0) {
				delete arr;
			}
			arrayListFree->refCount--;

			if ((arrayListFree->refCount) == 0) {
				arrayListFree->clear();
			}
		}}
	}

public:
	virtual ~CopyOnWriteClass() {
		delete arrayList;
		delete arrayListFree;
		LOG("~CopyOnWriteClass()");
	}
	CopyOnWriteClass() {
		LOG("CopyOnWriteClass()");
		arrayList = new EArrayList<EString*>(32);
		arrayList->add(new EString("0"));
		arrayList->add(new EString("1"));
		arrayList->add(new EString("2"));
		arrayList->add(new EString("3"));
		arrayList->add(new EString("4"));

		arrayListFree = new EArrayList<EString*>(32);
	}

	void read() {
		EArrayList<EString*>* arr = tackArray();
		for (int i=0; i<arr->size(); i++) {
			EString* s = arr->getAt(i);
			LOG("s=%s", s->c_str());
		}
		backArray(arr);
	}

	boolean add(EString* s) {
		SYNCBLOCK(&lock) {
			EArrayList<EString*>* arr = arrayList;
			if ((arr->refCount) == 0) {
				arr->add(s);
			}
			else {
				EArrayList<EString*>* newArr = new EArrayList<EString*>(arr->size()+1, true);
				for (int i=0; i<arr->size(); i++) {
					newArr->add(arr->getAt(i));
				}
				newArr->add(s);
				arr->setAutoFree(false);
				arr->refCount--;
				arrayList = newArr;
			}
			if ((arrayListFree->refCount) == 0) {
				arrayListFree->clear();
			}
		}}
		return true;
	}

	void remove(int index) {
		SYNCBLOCK(&lock) {
			EArrayList<EString*>* arr = arrayList;
			if ((arr->refCount) == 0) {
				arrayListFree->add(arr->removeAt(index));
			}
			else {
				EArrayList<EString*>* newArr = new EArrayList<EString*>(arr->size());
				for (int i=0; i<arr->size(); i++) {
					if (i == index) {
						arrayListFree->add(arr->getAt(i));
					} else {
						newArr->add(arr->getAt(i));
					}
				}
				arr->setAutoFree(false);
				arr->refCount--;
				arrayList = newArr;
			}
			if ((arrayListFree->refCount) == 0) {
				arrayListFree->clear();
			}
		}}
	}

	void set(int index, EString* s) {
		SYNCBLOCK(&lock) {
			EArrayList<EString*>* arr = arrayList;

	//		if (arr->size() >= index) {
	//			LOG("index out of size.");
	//			lock.unlock();
	//			return;
	//		}

			if ((arr->refCount) == 0) {
				EString* old = arr->setAt(index, s);
				arrayListFree->add(old);
			}
			else {
				EString* old = arr->getAt(index);
				if (old != s) {
					EArrayList<EString*>* newArr = new EArrayList<EString*>(arr->size());
					for (int i=0; i<arr->size(); i++) {
						newArr->add(arr->getAt(i));
					}
					newArr->setAt(index, s);
					arr->setAutoFree(false);
					arr->refCount--;
					arrayListFree->add(old);
					arrayList = newArr;
				}
				else {
					delete s;
				}
			}
			if ((arrayListFree->refCount) == 0) {
				arrayListFree->clear();
			}
		}}
	}
};

class Getter: public ERunnable {
public:
	Getter(CopyOnWriteClass* coal) {
		this->coal = coal;
	}
	virtual ~Getter() {

	}
	void run() {
		try {
			while (1) {
				coal->read();
			}
		} catch (EException& e) {
			e.printStackTrace();
			throw e;
		}
	}
private:
	CopyOnWriteClass* coal;
};

class Setter: public ERunnable {
public:
	Setter(CopyOnWriteClass* coal) {
		this->coal = coal;
	}
	virtual ~Setter() {

	}
	void run() {
		try {
			long l = 0;
			while (1) {
				int flag = l % 3;
				int index = l++ % 5;

				if (flag == 0) {
					EString* s = new EString();
					s->format("xxx: %d", index);
					coal->set(index, s);
				} else if (flag == 1) {
					coal->remove(index);
				} else {
					coal->add(new EString("added"));
				}

				EThread::sleep(500);
			}
		} catch (EException& e) {
			e.printStackTrace();
//			throw e;
		}
	}
private:
	CopyOnWriteClass* coal;
};

static void test_copyOnWrite1() {
	int i;
	CopyOnWriteClass* coal = new CopyOnWriteClass();

	EA<EThread*>* threads = new EA<EThread*>(10);
	for (i=0; i<8; i++) {
		(*threads)[i] = new EThread(new Getter(coal));
		(*threads)[i]->start();
	}
	for (i=8; i<10; i++) {
		(*threads)[i] = new EThread(new Setter(coal));
		(*threads)[i]->start();
	}

	for (i=0; i<10; i++) {
		(*threads)[i]->join();
	}

	delete coal;
}
#endif

static void test_copyOnWrite2() {
	if (1) {
		ECopyOnWriteArrayList<EInteger> col;
		col.add(new EInteger(1));
		sp<EInteger> i0 = col.getAt(0);
		LOG("col[0] = %d", i0->intValue());
		col.remove(new EInteger(2));
		col.addAt(1, new EInteger(4));
		sp<EInteger> i1 = col.getAt(1);
		LOG("col[1] = %d", i1->intValue());

		col.addIfAbsent(new EInteger(4));

		sp<EIterator<sp<EInteger> > > iter = col.listIterator(1);
		while (iter->hasNext()) {
			sp<EInteger> i = iter->next();
			LOG("i=%d", i->intValue());
		}

		ECopyOnWriteArrayList<EInteger>* col2 = col.clone();
		i1 = col2->getAt(1);
		LOG("col2[1] = %d", i1->intValue());
		delete col2;

		col.remove(new EInteger(4));
		col.clear();
		LOG("end.");
	}

	class addThread : public EThread {
	private:
		ECopyOnWriteArrayList<EInteger>* list;// = null;
		Count* c;// = null;

	public:
		addThread(ECopyOnWriteArrayList<EInteger>* list, Count* c) {
			this->list = list;
			this->c = c;
		}

		void run() {
			int i = 1;
			try {
//				while (true) {
				for (i=0; i<2000; i++) {
					boolean r = list->add(new EInteger(i));
					if (!r) {
						LOG("add fail...");
					}
//                    list->addAt(1, new EInteger(i+2));

					c->addcount1(); // add
					i++;

//					EThread::sleep(500);
				}
			} catch (EException& e) {
				e.printStackTrace();
			}
			LOG("end of add thread run().");
		}
	};

	class getThread : public EThread {
	private:
		ECopyOnWriteArrayList<EInteger>* list;// = null;
		Count* c;// = null;

	public:
		getThread(ECopyOnWriteArrayList<EInteger>* list, Count* c) {
			this->list = list;
			this->c = c;
		}

		void run() {
			int i = 0;
			try {
				EThread::sleep(500);

//				while (true) {
				for (i=0; i<20000; i++) {
					try {
						sp<EInteger> si = list->getAt(0);
//						LOG("get si=%d", si->intValue());
					} catch (EIndexOutOfBoundsException& e) {
					}

					sp<EIterator<sp<EInteger> > > iter = list->iterator();
					while (iter->hasNext()) {
						sp<EInteger> x = iter->next();
//						LOG("i=%d", x->intValue());
					}

					sp<EListIterator<sp<EInteger> > > listiter = list->listIterator();
					while (listiter->hasNext()) {
						sp<EInteger> x = listiter->next();
						LOG("i=%d", x->intValue());
					}

					c->addcount2(); // poll
					i++;
				}
			} catch (EException& e) {
				e.printStackTrace();
			}

			EA<sp<EInteger> > arr = list->toArray();
			for (i=0; i<arr.length(); i++) {
				LOG("xxxxxxxxxxxxxxx=%d", arr[i]->intValue());
			}

			LOG("end of get thread run(), i=%d.", i);
		}
	};

	class delThread : public EThread {
	private:
		ECopyOnWriteArrayList<EInteger>* list;// = null;
		Count* c;// = null;

	public:
		delThread(ECopyOnWriteArrayList<EInteger>* list, Count* c) {
			this->list = list;
			this->c = c;
		}

		void run() {
			try {
				EThread::sleep(10);
//				while (true) {
                for (int i=0; i<2000; i++) {
                	try {
						sp<EInteger> si = list->removeAt(0);
                	} catch (EIndexOutOfBoundsException& e) {
					}
//                    list->removeAt(1);
//					LOG("get si=%d", si->intValue());
				}
			} catch (EException& e) {
				e.printStackTrace();
			}
			LOG("end of del thread run().");
		}
	};

	class ProbeThread : public EThread { // 
	private:
		boolean run_;// = true;
		Count* cc;

	public:
		ProbeThread(Count* cc) : run_(true) {
			this->cc = cc;
		}

		void run() {
			int c1 = 0, c2 = 0;
			int cc1 = 0, cc2 = 0;
			while (this->run_) {
				c2 = cc->getcount1();
				cc2 = cc->getcount2();
				LOG("put:[%d/%d]  get:[%d/%d]",
						(c2 - c1) / 2, c2,
						(cc2 - cc1) / 2, cc2);
				c1 = c2;
				cc1 = cc2;

				try {
					EThread::sleep(1000 * 2 - 1);
				} catch (EException& ex) {
					LOG("Error[ProbeThread.run]:%s", ex.getMessage());
				}
			}
		}
	};

	try {
		Count* c = new Count();
		ECopyOnWriteArrayList<EInteger>* list = new ECopyOnWriteArrayList<EInteger>();

		EArray<EThread*> arr;
		int i;
		for (i = 0; i < 2; i++) {
			addThread* pct = new addThread(list, c);
			pct->start();// put
			arr.add(pct);
		}

		for (i = 0; i < 5; i++) {
			getThread* gct = new getThread(list, c);
			gct->start(); // get
			arr.add(gct);
		}

		for (i = 0; i < 1; i++) {
			delThread* dct = new delThread(list, c);
			dct->start(); // del
			arr.add(dct);
		}

//		ProbeThread* pt = new ProbeThread(c); // 
//		pt->start();
//		arr.add(pt);

		for (i = 0; i < arr.length(); i++) {
			arr.getAt(i)->join();
		}

		delete list;
		delete c;

		LOG("test_copyOnWrite2...");

//		EThread::sleep(3000);
	} catch (EException& e) {
		e.printStackTrace();
	}
}

#if 0
static std::shared_ptr<EString> g_shared_ptr(new EString("111"));

static void test_shared_ptr() {
	{
	std::shared_ptr<EString> sx(new EString("x"));
	std::shared_ptr<EString> sy(new EString("y"));
	sx = sy;
	LOG("x=%s, y=%s", sx->c_str(), sy->c_str());
	std::shared_ptr<EString> sn;
	sx = null;
	}

	class spReader : public EThread {
	public:
		virtual void run() {
	       while (true) {
	    	   std::shared_ptr<EString> local(g_shared_ptr);
	           // do smth
	           LOG("s=%s", local->c_str());
	       }
	    }
	};

	class spWriter : public EThread {
	public:
		virtual void run() {
			while (true) {
				std::shared_ptr<EString> newPtr(new EString("xxx"));
				g_shared_ptr = newPtr;
	       }
	    }
	};

	EArray<EThread*> arr;
	int i;
	for (i = 0; i < 2; i++) {
		spReader* sThread1 = new spReader();
		sThread1->start();
		arr.add(sThread1);
	}

	for (i = 0; i < 3; i++) {
		spWriter* sThread2 = new spWriter();
		sThread2->start();
		arr.add(sThread2);
	}

	for (i = 0; i < arr.length(); i++) {
		arr.getAt(i)->join();
	}
}
#endif

#if 1
static spnamespace::shared_ptr<EString> g_shared_ptr(new EString("111"));

static void test_shared_ptr() {
	{
		spnamespace::shared_ptr<EString> sx(new EString("x"));
		spnamespace::shared_ptr<EString> sy(new EString("y"));
		sx = sy;
		LOG("x=%s, y=%s", sx->c_str(), sy->c_str());
		spnamespace::shared_ptr<EString> sn;
		spnamespace::shared_ptr<EString> sm(null);

		sx = null;
		sn = sy;
		sm = new EString("m");
		LOG("n=%s, m=%s", sn->c_str(), sm->c_str());

		spnamespace::shared_ptr<EString> r = spnamespace::atomic_exchange(&sn, sm);
		LOG("r=%s, n=%s, m=%s", r->c_str(), sn->c_str(), sm->c_str());

		spnamespace::shared_ptr<EString>& r2 = r;
		boolean b = spnamespace::atomic_compare_exchange(&r, &r2, sn);
//		boolean b = spnamespace::atomic_compare_exchange(&r, &sn, sm);
		LOG("b=%d, r=%s, n=%s, m=%s", b, r->c_str(), sn->c_str(), sm->c_str());

		spnamespace::shared_ptr<EObject> so = dynamic_pointer_cast<EObject>(sm);
		LOG("so=%s", so->toString().c_str());

		if (so == sm) {
			LOG("so == sm!");
		}
		if (so.get() == sm.get()) {
			LOG("so.px == sm.px! so.px=%p, sm.px=%p", so.get(), sm.get());
		}
	}
	return;

	class spReader : public EThread {
	public:
		virtual void run() {
	       while (true) {
	    	   spnamespace::shared_ptr<EString> local(spnamespace::atomic_load(&g_shared_ptr));
	           // do smth
	           LOG("s=%s", local->c_str());
	       }
	    }
	};

	class spWriter : public EThread {
	public:
		virtual void run() {
			while (true) {
				spnamespace::shared_ptr<EString> newPtr(new EString("xxx"));
				spnamespace::atomic_store(&g_shared_ptr, newPtr);
//				g_shared_ptr = newPtr;
	       }
	    }
	};

	EArray<EThread*> arr;
	int i;
	for (i = 0; i < 2; i++) {
		spReader* sThread1 = new spReader();
		sThread1->start();
		arr.add(sThread1);
	}

	for (i = 0; i < 3; i++) {
		spWriter* sThread2 = new spWriter();
		sThread2->start();
		arr.add(sThread2);
	}

	for (i = 0; i < arr.length(); i++) {
		arr.getAt(i)->join();
	}
}
#endif

static void test_shared_ptr2()
{
	EString* s = null;

	{
		sp<EString> s0(new EString("s0"));

		{
			sp<EString> s1(s0);

			if (s0 == s1) {
				LOG("1");
			}

			sp<EString> s2(new EString("s0"));
			if (s2.equals(s1)) {
				LOG("2");
			}
		}

		s = s0.dismiss();
	}

	if (s) {
		LOG("s=%s", s->c_str());
		delete s;
	}

	//======================

	class X2;
	class X1 {
	sp<X2> x2;
	public:
		virtual ~X1() {
			LOG("~X1()");
		}
		void setX2(sp<X2> x) {
			x2 = x;
		}
		sp<X2> getX2() {
			return x2;
		}
		void print() {
			LOG("print1...");
		}
	};

	class X2 {
	wp<X1> x1;
	public:
		virtual ~X2() {
			LOG("~X2()");
		}
		X2(sp<X1> x) : x1(x) {
		}
		void print() {
			LOG("print2...");
		}
		sp<X1> getX1() {
			return x1.lock();
		}
	};

	sp<X1> x1(new X1());
	{
	sp<X2> x2(new X2(x1));
	x1->setX2(x2);
	}
	sp <X2> xx2 = x1->getX2();
	if (xx2 != null) {
		xx2->print();
	}
	sp <X1> xx1 = xx2->getX1();
	if (xx1 != null) {
		xx1->print();
	}

	//======================

	class Test : public efc::enable_shared_from_this<Test>
	{
	public:
	    ~Test() { LOG("~Test()"); }
	    sp<Test> GetObject()
	    {
	        return shared_from_this();
	    }
	};

	sp<Test> p(new Test());
//	sp<Test> p = new Test();
	sp<Test> q = p->GetObject();

	//======================

	{
		sp<X1> x1(new X1());
		sp<X1> x2(x1);

		LOG("111");
		x1.reset();
		LOG("222");
	}

	//======================

//	class raw_shared: public enable_shared_from_raw {
//	public:
//		raw_shared() {
//			LOG("raw_shared ctor");
//		}
//		~raw_shared() {
//			LOG("raw_shared dtor");
//		}
//	};
//
//	{
//		raw_shared x;
//		assert(!weak_from_raw(&x).use_count());
//		auto px = shared_from_raw(&x);
//		assert(px.use_count() == 2);
//		auto p = new raw_shared;
//		auto wp = weak_from_raw(p);
//		assert(wp.use_count() == 0);
//		decltype(shared_from_raw(p)) spx(p);
//		auto sp = shared_from_raw(p);
//		assert(sp.use_count() == 2);
//		auto sp2 = sp;
//		auto wp2 = weak_from_raw(p);
//		assert(wp2.use_count() == 3);
//	}

	LOG("end of test_shared_ptr2()");
}

static void test_concurrentHashmap()
{
#if 1
	EConcurrentHashMap<EString, EString> *chm = new EConcurrentHashMap<EString, EString>();
	chm->put(new EString("1"), new EString("1111111111"));
	chm->put(new EString("2"), new EString("2222222222"));
	chm->put(new EString("3"), new EString("3333333333"));

	llong t1 = ESystem::currentTimeMillis();
	for (int i=0; i<10000; i++) {
		sp<EString> oldV = chm->put(new EString(i), new EString(i));
	}

	LOG("t = %ld", ESystem::currentTimeMillis() - t1);

	{
		EString* v = new EString("2222222222");
		boolean rv = chm->contains(v);
		LOG("rv=%d", rv);
		delete v;
	}

	EHashMap<EString*, EString*> hm(false, false);
	hm.put(new EString("key"), new EString("value"));
	chm->putAll(&hm);

	{
		EString* k = new EString("key");
		sp<EString> v = chm->get(k);
		LOG("v=%s", v!=null ? v->c_str() : "null");
		delete k;
	}

    ESet<sp<EConcurrentMapEntry<EString,EString> > >* set = chm->entrySet();
    sp<EIterator<sp<EConcurrentMapEntry<EString,EString> > > > it = set->iterator();
	while (it->hasNext()) {
		sp<EConcurrentMapEntry<EString,EString> > me = it->next();
		sp<EString> ki = me->getKey();
		sp<EString> vi = me->getValue();
		LOG("k=%s, v=%s", ki->c_str(), vi->c_str());
	}

	delete chm;

	EConcurrentHashMap<EFile, EString> chm2;
	chm2.put(new EFile("./logs/error.log"), new EString("1"));
	chm2.put(new EFile("./logs/all.log"), new EString("2"));
	sp<EFile> sf = new EFile("./logs/error.log");
	LOG("file=%s", chm2.get(sf.get())->c_str());

//	return;
#endif

	gDeadlineTimestamp = ESystem::currentTimeSeconds() + 60; //30s

	//========================
	class putCHashmapThread : public EThread {
	private:
		EConcurrentHashMap<EInteger, EInteger>* chm;// = null;
		Count* c;// = null;

	public:
		putCHashmapThread(EConcurrentHashMap<EInteger, EInteger>* chm, Count* c) {
			this->chm = chm;
			this->c = c;
		}

		void run() {
			int tt = 13;
			int i = 1;
			try {
                ERandom r;
				while (true) {
//				for (i=0; i<50000; i++) {
					tt = EMath::abs(tt * (tt - i) - 119);
//                    tt = r.nextInt();
					sp<EInteger> oldV = chm->put(new EInteger(tt), new EInteger(tt));
//					LOG("put %d, oldV=%d", i, oldV != null ? oldV->intValue() : -1);

#if 0
					EInteger ei(tt);

					//remove
					if (i % 10 == 0) {
						sp<EInteger> v = chm->remove(&ei);
//						LOG("remove ei=%d, v=%d", ei.intValue(), v==null ? -1 : v->intValue());
					}
#endif
					c->addcount1(); // put
					i++;

					if (ESystem::currentTimeSeconds() - gDeadlineTimestamp > 0) {
						break;
					}

//					EThread::sleep(500);
				}
			} catch (EException& e) {
				e.printStackTrace();
			}
            LOG("end of put thread run().");
		}
	};

	class getCHashmapThread : public EThread { // get
	private:
		EConcurrentHashMap<EInteger, EInteger>* chm;// = null;
		Count* c;// = null;

	public:
		getCHashmapThread(EConcurrentHashMap<EInteger, EInteger>* chm, Count* c) {
			this->chm = chm;
			this->c = c;
		}

		void run() {
			int tt = 13;
			int i = 1;
			try {
				while (true) {
//				for (i=0; i<10000; i++) {
					tt = EMath::abs(tt * (tt - i) - 119);
					EInteger ei(tt);
					sp<EInteger> v = chm->get(&ei);
//					LOG("get i=%d, v=%d", i, v==null ? -1 : v->intValue());

#if 0
					//test containsKey
					if (v!=null) {
						boolean r = chm->containsKey(&ei);
//						LOG("containsKey r=%d", r);
					}

					//test containsValue
					if (v!=null) {
						boolean r = chm->containsValue(v.get());
//						LOG("containsValue r=%d", r);
					}

					//test iterator
                    ESet<sp<EConcurrentMapEntry<EInteger,EInteger> > >* set = chm->entrySet();
                    sp<EIterator<sp<EConcurrentMapEntry<EInteger,EInteger> > > > it = set->iterator();
					while (it->hasNext()) {
						sp<EConcurrentMapEntry<EInteger,EInteger> > me = it->next();
						sp<EInteger> ki = me->getKey();
						sp<EInteger> vi = me->getValue();
//						LOG("k=%d, v=%d", ki->intValue(), vi->intValue());
					}

					//test elements
					sp<EConcurrentEnumeration<EInteger> > ce = chm->elements();
					while (ce->hasMoreElements()) {
						sp<EInteger> o = ce->nextElement();
//						LOG("v=%d", o->intValue());
					}
#endif
					c->addcount2(); // get
					i++;

					if (ESystem::currentTimeSeconds() - gDeadlineTimestamp > 0) {
						break;
					}

//					EThread::sleep(100);
				}
			} catch (EException& e) {
				e.printStackTrace();
			}
			LOG("end of get thread run().");
		}
	};

	class iteCHashmapThread : public EThread { // get操作线程
	private:
		EConcurrentHashMap<EInteger, EInteger>* chm;// = null;

	public:
		iteCHashmapThread(EConcurrentHashMap<EInteger, EInteger>* chm) {
			this->chm = chm;
		}

		void run() {
			try {
				chm->entrySet()->iterator();
			} catch (EException& e) {
				e.printStackTrace();
			}
			LOG("end of ite thread run().");
		}
	};

	class ProbeThread : public EThread { // 
	private:
		boolean run_;// = true;
		Count* cc;

	public:
		ProbeThread(Count* cc) : run_(true) {
			this->cc = cc;
		}

		void run() {
			int c1 = 0, c2 = 0;
			int cc1 = 0, cc2 = 0;
			while (this->run_) {
				c2 = cc->getcount1();
				cc2 = cc->getcount2();
				LOG("put:[%d/%d]  get:[%d/%d]",
						(c2 - c1) / 2, c2,
						(cc2 - cc1) / 2, cc2);
				c1 = c2;
				cc1 = cc2;

				if (ESystem::currentTimeSeconds() - gDeadlineTimestamp > 0) {
					break;
				}

				try {
					EThread::sleep(1000 * 2 - 1);
				} catch (EException& ex) {
					LOG("Error[ProbeThread.run]:%s", ex.getMessage());
				}
			}
		}
	};

	try {
		Count* c = new Count();
		EConcurrentHashMap<EInteger, EInteger>* chm = new EConcurrentHashMap<EInteger, EInteger>();

		EArray<EThread*> arr;
		int i;
		for (i = 0; i < 20; i++) {
			putCHashmapThread* pct = new putCHashmapThread(chm, c);
			pct->start();// put
			arr.add(pct);
		}

		for (i = 0; i < 20; i++) {
			getCHashmapThread* gct = new getCHashmapThread(chm, c);
			gct->start(); // get
			arr.add(gct);
		}

//		for (i = 0; i < 2; i++) {
//			iteCHashmapThread* ict = new iteCHashmapThread(chm);
//			ict->start();// iterator
//			arr.add(ict);
//		}

		ProbeThread* pt = new ProbeThread(c); //
		pt->start();
		arr.add(pt);

		for (i = 0; i < arr.length(); i++) {
			arr.getAt(i)->join();
		}
		delete chm;
		delete c;

		LOG("test_concurrentHashmap...");

//		EThread::sleep(3000);
	} catch (EException& e) {
		e.printStackTrace();
	}
}

static void test_concurrentHashmap2()
{
#define K int

	gDeadlineTimestamp = ESystem::currentTimeSeconds() + 60; //30s

	//========================
	class putCHashmapThread : public EThread {
	private:
		EConcurrentHashMap<K, EInteger>* chm;// = null;
		Count* c;// = null;

	public:
		putCHashmapThread(EConcurrentHashMap<K, EInteger>* chm, Count* c) {
			this->chm = chm;
			this->c = c;
		}

		void run() {
			int tt = 13;
			int i = 1;
			try {
                ERandom r;
				while (true) {
//				for (i=0; i<50000; i++) {
					tt = EMath::abs(tt * (tt - i) - 119);
//                    tt = r.nextInt();
					sp<EInteger> oldV = chm->put(tt, new EInteger(tt));
//					LOG("put %d, oldV=%d", i, oldV != null ? oldV->intValue() : -1);

#if 1
					//remove
					if (i % 10 == 0) {
						sp<EInteger> v = chm->remove(tt);
//						LOG("remove ei=%d, v=%d", ei.intValue(), v==null ? -1 : v->intValue());
					}
#endif
					c->addcount1(); // put
					i++;

					if (ESystem::currentTimeSeconds() - gDeadlineTimestamp > 0) {
						break;
					}

//					EThread::sleep(500);
				}
			} catch (EException& e) {
				e.printStackTrace();
			}
            LOG("end of put thread run().");
		}
	};

	class getCHashmapThread : public EThread { // get
	private:
		EConcurrentHashMap<K, EInteger>* chm;// = null;
		Count* c;// = null;

	public:
		getCHashmapThread(EConcurrentHashMap<K, EInteger>* chm, Count* c) {
			this->chm = chm;
			this->c = c;
		}

		void run() {
			int tt = 13;
			int i = 1;
			try {
				while (true) {
//				for (i=0; i<10000; i++) {
					tt = EMath::abs(tt * (tt - i) - 119);
					sp<EInteger> v = chm->get(tt);
//					LOG("get i=%d, v=%d", i, v==null ? -1 : v->intValue());

#if 0
					//test containsKey
					if (v!=null) {
						boolean r = chm->containsKey(tt);
//						LOG("containsKey r=%d", r);
					}

					//test containsValue
					if (v!=null) {
						boolean r = chm->containsValue(v.get());
//						LOG("containsValue r=%d", r);
					}

					//test iterator
                    ESet<sp<EConcurrentMapEntry<K,EInteger> > >* set = chm->entrySet();
					sp<EIterator<sp<EConcurrentMapEntry<K,EInteger> > > > it = set->iterator();
					while (it->hasNext()) {
						sp<EConcurrentMapEntry<K,EInteger> > me = it->next();
						K ki = me->getKey();
						sp<EInteger> vi = me->getValue();
//						LOG("k=%d, v=%d", ki->intValue(), vi->intValue());
					}

					//test elements
					sp<EConcurrentEnumeration<EInteger> > ce = chm->elements();
					while (ce->hasMoreElements()) {
						sp<EInteger> o = ce->nextElement();
//						LOG("v=%d", o->intValue());
					}
#endif
					c->addcount2(); // get
					i++;

					if (ESystem::currentTimeSeconds() - gDeadlineTimestamp > 0) {
						break;
					}

//					EThread::sleep(100);
				}
			} catch (EException& e) {
				e.printStackTrace();
			}
			LOG("end of get thread run().");
		}
	};

	class iteCHashmapThread : public EThread { // get操作线程
	private:
		EConcurrentHashMap<EInteger, EInteger>* chm;// = null;

	public:
		iteCHashmapThread(EConcurrentHashMap<EInteger, EInteger>* chm) {
			this->chm = chm;
		}

		void run() {
			try {
				chm->entrySet()->iterator();
			} catch (EException& e) {
				e.printStackTrace();
			}
			LOG("end of ite thread run().");
		}
	};

	class ProbeThread : public EThread { //
	private:
		boolean run_;// = true;
		Count* cc;

	public:
		ProbeThread(Count* cc) : run_(true) {
			this->cc = cc;
		}

		void run() {
			int c1 = 0, c2 = 0;
			int cc1 = 0, cc2 = 0;
			while (this->run_) {
				c2 = cc->getcount1();
				cc2 = cc->getcount2();
				LOG("put:[%d/%d]  get:[%d/%d]",
						(c2 - c1) / 2, c2,
						(cc2 - cc1) / 2, cc2);
				c1 = c2;
				cc1 = cc2;

				if (ESystem::currentTimeSeconds() - gDeadlineTimestamp > 0) {
					break;
				}

				try {
					EThread::sleep(1000 * 2 - 1);
				} catch (EException& ex) {
					LOG("Error[ProbeThread.run]:%s", ex.getMessage());
				}
			}
		}
	};

	try {
		Count* c = new Count();
		EConcurrentHashMap<K, EInteger>* chm = new EConcurrentHashMap<K, EInteger>();

		EArray<EThread*> arr;
		int i;
		for (i = 0; i < 20; i++) {
			putCHashmapThread* pct = new putCHashmapThread(chm, c);
			pct->start();// put
			arr.add(pct);
		}

		for (i = 0; i < 20; i++) {
			getCHashmapThread* gct = new getCHashmapThread(chm, c);
			gct->start(); // get
			arr.add(gct);
		}

//		for (i = 0; i < 2; i++) {
//			iteCHashmapThread* ict = new iteCHashmapThread(chm);
//			ict->start();// iterator
//			arr.add(ict);
//		}

		ProbeThread* pt = new ProbeThread(c); //
		pt->start();
		arr.add(pt);

		for (i = 0; i < arr.length(); i++) {
			arr.getAt(i)->join();
		}
		delete chm;
		delete c;

		LOG("test_concurrentHashmap...");

//		EThread::sleep(3000);
	} catch (EException& e) {
		e.printStackTrace();
	}
}

template<typename E, typename LOCK=ESpinLock>
class ConcurrentQueue {
public:
	typedef struct node_t {
	    E* volatile value;
	    node_t* volatile next;

	    node_t(): next(null) {}
	} NODE;

public:
	~ConcurrentQueue() {
		NODE* node = head;
		while (node != null) {
			NODE* n = node->next;
			delete node;
			node = n;
		}
	}

	ConcurrentQueue() {
		NODE* node = new NODE();
		head = tail = node;
	}

	void add(E* e) {
		NODE* node = new NODE();
		node->value = e;
		node->next = null;
		tl.lock();
			tail->next = node;
			tail = node;
		tl.unlock();
	}

	E* poll() {
		E* v;
		NODE* node = null;
		hl.lock();
			node = head;
			NODE* new_head = node->next;
			if (new_head == null) {
				hl.unlock();
				return null;
			}
			v = new_head->value;
			head = new_head;
		hl.unlock();
		delete node;
		return v;
	}

private:
	NODE *head;
	NODE *tail;
	LOCK hl;
	LOCK tl;
};

static ConcurrentQueue<EInteger> g_queue;
static EConcurrentHashMap<EInteger, EInteger> g_queue_datas;
static void test_concurrent_queue() {
#define NUM 1000000

	class Thread: public EThread {
	private:
		int id;
	public:
		Thread(int i): id(i) {}
		virtual void run() {
			int j = NUM * id;
			for (int i = 0; i < NUM; i++) {
				if (id < 2)
					g_queue.add(new EInteger(i));
				else {
					sp<EInteger> n = g_queue.poll();
					//LOG("i=%d", i);
					if (n != null) {
						g_queue_datas.put(n, n);
					}
				}
			}
		}
	};

	es_int64_t ts1, ts2;
	ts1 = ESystem::currentTimeMillis();

	Thread t0(0);
	Thread t1(1);
	Thread t2(2);
	Thread t3(3);
	t0.start();
	t1.start();
	t2.start();
	t3.start();
	t0.join();
	t1.join();
	t2.join();
	t3.join();

	ts2 = ESystem::currentTimeMillis();
//	LOG("%lld", ts2-ts1);

	int n = g_queue_datas.size();
	ES_ASSERT(n == NUM);
	LOG("%s", (n != NUM) ? "error!!!" : "success.");
}

static void test_concurrentLiteQueue() {

#define NUM 10000000

	EConcurrentLiteQueue<EInteger> queue;

	class Thread: public EThread {
	private:
		EConcurrentLiteQueue<EInteger>& queue;
		int id;
	public:
		Thread(EConcurrentLiteQueue<EInteger>& q, int i): queue(q), id(i) {}
		virtual void run() {
			int j = NUM * id;
			for (int i = 0; i < NUM; i++) {
				if (id < 2)
					queue.add(new EInteger(i));
				else {
					sp<EInteger> n = queue.poll();
					LOG("i=%d", i);
				}
			}
		}
	};

	es_int64_t ts1, ts2;
	ts1 = ESystem::currentTimeMillis();

	Thread t0(queue, 0);
	Thread t1(queue, 1);
	Thread t2(queue, 2);
	Thread t3(queue, 3);
	t0.start();
	t1.start();
	t2.start();
	t3.start();
	t0.join();
	t1.join();
	t2.join();
	t3.join();

	LOG("test_concurrentLiteQueue...");
}

static void test_concurrentIntrusiveDeque() {
	class XXX: public EQueueEntry {
	private:
		int i;
	public:
		XXX(int i): i(i) {}
		int intValue() { return i; }
	};

	class addCQueueThread : public EThread {
	private:
		EConcurrentIntrusiveDeque<XXX>* queue;// = null;
		Count* c;// = null;

	public:
		addCQueueThread(EConcurrentIntrusiveDeque<XXX>* queue, Count* c) {
			this->queue = queue;
			this->c = c;
		}

		void run() {
			int i = 1;
			try {
//				while (true) {
				for (i=0; i<10000; i++) {
					boolean r = queue->add(new XXX(i));
					if (!r) {
						LOG("add fail...");
					}

					c->addcount1(); // add
					i++;

//					EThread::sleep(500);
				}
//				queue->add(null);
			} catch (EException& e) {
				e.printStackTrace();
			}
			LOG("end of add thread run().");
		}
	};

	class pollCQueueThread : public EThread {
	private:
		EConcurrentIntrusiveDeque<XXX>* queue;// = null;
		Count* c;// = null;

	public:
		pollCQueueThread(EConcurrentIntrusiveDeque<XXX>* queue, Count* c) {
			this->queue = queue;
			this->c = c;
		}

		void run() {
			int i = 0;
			try {
				while (true) {
					sp<XXX> si = queue->poll();
					if (si == null) {
						break;
					}
//					LOG("poll si=%d", si->intValue());

					if (i % 50) {
						si = queue->peekLast();
						queue->remove(si.get()); // mast be element self!
					}
					if (i % 100) {
						try {
							queue->remove();
						} catch (ENoSuchElementException& e) {
						}
					}

					c->addcount2(); // poll
					i++;

//					EThread::sleep(500);
				}
			} catch (EException& e) {
				e.printStackTrace();
			}
			LOG("end of poll thread run(), i=%d.", i);
		}
	};

	class ProbeThread : public EThread { //
	private:
		boolean run_;// = true;
		Count* cc;

	public:
		ProbeThread(Count* cc) : run_(true) {
			this->cc = cc;
		}

		void run() {
			int c1 = 0, c2 = 0;
			int cc1 = 0, cc2 = 0;
			while (this->run_) {
				c2 = cc->getcount1();
				cc2 = cc->getcount2();
				LOG("put:[%d/%d]  get:[%d/%d]",
						(c2 - c1) / 2, c2,
						(cc2 - cc1) / 2, cc2);
				c1 = c2;
				cc1 = cc2;

				try {
					EThread::sleep(1000 * 2 - 1);
				} catch (EException& ex) {
					LOG("Error[ProbeThread.run]:%s", ex.getMessage());
				}
			}
		}
	};

	try {
		Count* c = new Count();
		EConcurrentIntrusiveDeque<XXX>* queue = new EConcurrentIntrusiveDeque<XXX>();

		EArray<EThread*> arr;
		int i;
		for (i = 0; i < 5; i++) {
			addCQueueThread* pct = new addCQueueThread(queue, c);
			pct->start();// put
			arr.add(pct);
		}

		for (i = 0; i < 5; i++) {
			pollCQueueThread* gct = new pollCQueueThread(queue, c);
			gct->start(); // get
			arr.add(gct);
		}

//		ProbeThread* pt = new ProbeThread(c); //
//		pt->start();
//		arr.add(pt);

		for (i = 0; i < arr.length(); i++) {
			arr.getAt(i)->join();
		}

		delete queue;
		delete c;

		LOG("test_concurrentIntrusiveDeque...");

//		EThread::sleep(3000);
	} catch (EException& e) {
		e.printStackTrace();
	}
}

static void test_mutexLinkedQueue() {
	EArrayList<sp<EInteger> > list;
	list.add(new EInteger(-1));

	EMutexLinkedQueue<EInteger> queue(&list);

	for (int i=0; i<10; i++) {
		queue.add(new EInteger(i));
	}

	LOG("queue size=%d", queue.size());

	for (sp<EIterator<sp<EInteger> > > iter = queue.iterator(); iter->hasNext();) {
		sp<EInteger> e = iter->next();
		LOG("iter i=%d", e->intValue());
	}

	sp<EInteger> e = queue.peek();
	LOG("peek i=%d", e->intValue());

	EInteger x(8);
	if (queue.contains(&x)) {
		LOG("contains 8!");
	}

	if (queue.remove(&x)) {
		LOG("remove 8!");
	}

	LOG("queue size=%d", queue.size());

	queue.remove();

	LOG("queue size=%d", queue.size());

	for (int i=0; i<12; i++) {
		sp<EInteger> e = queue.poll();
		if (e == null) {
			LOG("poll null!");
			break;
		}
		LOG("poll i=%d", e->intValue());
	}

	LOG("queue size=%d", queue.size());

	e = queue.peek();
	if (e == null) {
		LOG("peek null!");
	}

	for (int i=0; i<10; i++) {
		queue.add(new EInteger(i));
	}

	for (sp<EIterator<sp<EInteger> > > iter = queue.iterator(5); iter->hasNext();) {
		iter->remove();
	}

	LOG("queue size=%d", queue.size());

	for (sp<EIterator<sp<EInteger> > > iter = queue.iterator(); iter->hasNext();) {
		sp<EInteger> e = iter->next();
		LOG("iter i=%d", e->intValue());
	}

	LOG("test_mutexLinkedQueue...");
}

static void test_mutexLinkedQueue2() {
	class addCQueueThread : public EThread {
	private:
		EMutexLinkedQueue<EInteger>* queue;// = null;
		Count* c;// = null;

	public:
		addCQueueThread(EMutexLinkedQueue<EInteger>* queue, Count* c) {
			this->queue = queue;
			this->c = c;
		}

		void run() {
			int i = 1;
			try {
				while (true) {
//				for (i=0; i<1000; i++) {
#if 0
					if ((long)eso_os_thread_current() % 3 == 0) {
						sp<EIterator<sp<EInteger> > > iter = queue->iterator();
						while (iter->hasNext()) {
							sp<EInteger> i = iter->next();
							LOG("i=%d", i->intValue());
						}
						LOG("+++++++++++++++++++++++++++++++++++++++++++++++");
					}
#endif
					boolean r = queue->add(new EInteger(i));
					if (!r) {
						LOG("add fail...");
					}

					c->addcount1(); // add
					i++;

//					EThread::sleep(500);
				}

				queue->add(new EInteger(-1));

			} catch (EException& e) {
				e.printStackTrace();
			}
			LOG("end of add thread run().");
		}
	};

	class pollCQueueThread : public EThread {
	private:
		EMutexLinkedQueue<EInteger>* queue;// = null;
		Count* c;// = null;

	public:
		pollCQueueThread(EMutexLinkedQueue<EInteger>* queue, Count* c) {
			this->queue = queue;
			this->c = c;
		}

		void run() {
			int i = 0;
			try {
				while (true) {
					sp<EInteger> si = queue->poll();
//					if (si == null || si->intValue() == -1) {
//						break;
//					}
//					LOG("poll si=%d", si->intValue());

//					if (i % 50) {
//						sp<EInteger> pi(new EInteger(i-1));
//						queue->remove(pi.get());
//					}
//					if (i % 100) {
//						queue->remove();
//					}

					c->addcount2(); // poll
					i++;

//					EThread::sleep(500);
				}
			} catch (EException& e) {
				e.printStackTrace();
			}
			LOG("end of poll thread run(), i=%d.", i);
		}
	};

	class ProbeThread : public EThread { //
	private:
		boolean run_;// = true;
		Count* cc;

	public:
		ProbeThread(Count* cc) : run_(true) {
			this->cc = cc;
		}

		void run() {
			int c1 = 0, c2 = 0;
			int cc1 = 0, cc2 = 0;
			while (this->run_) {
				c2 = cc->getcount1();
				cc2 = cc->getcount2();
				LOG("put:[%d/%d]  get:[%d/%d]",
						(c2 - c1) / 2, c2,
						(cc2 - cc1) / 2, cc2);
				c1 = c2;
				cc1 = cc2;

				try {
					EThread::sleep(1000 * 2 - 1);
				} catch (EException& ex) {
					LOG("Error[ProbeThread.run]:%s", ex.getMessage());
				}
			}
		}
	};

	try {
		Count* c = new Count();
		EMutexLinkedQueue<EInteger>* queue = new EMutexLinkedQueue<EInteger>();

		EArray<EThread*> arr;
		int i;
		for (i = 0; i < 5; i++) {
			addCQueueThread* pct = new addCQueueThread(queue, c);
			pct->start();// put
			arr.add(pct);
		}

		for (i = 0; i < 5; i++) {
			pollCQueueThread* gct = new pollCQueueThread(queue, c);
			gct->start(); // get
			arr.add(gct);
		}

		ProbeThread* pt = new ProbeThread(c); //
		pt->start();
		arr.add(pt);

		for (i = 0; i < arr.length(); i++) {
			arr.getAt(i)->join();
		}

		delete queue;
		delete c;

		LOG("test_mutexLinkedQueue2...");
	} catch (EException& e) {
		e.printStackTrace();
	}
}

static void test_concurrentLinkedQueue() {
#if 0 //c11, for test this bug: http://bugs.java.com/view_bug.do?bug_id=6785442
	{
		EConcurrentLinkedQueue<EString> queue;
		volatile boolean removeWorked = false;

		EString x("B");

		queue.add(new EString("A"));
		queue.add(new EString("B"));

		sp<EThread> t1 = EThread::executeX([&]() {
			removeWorked = queue.remove(&x);
		});

//		EThread::sleep(100);
		auto a = queue.poll();
		LOG("First poll returned: %s", (!!a) ? a->toString().c_str() : "null");
		auto b = queue.poll();
		LOG("Second poll returned: %s", (!!b) ?  b->toString().c_str() : "null");

		t1->join();

		LOG(("Async remove(\"B\") returned: " + EString(removeWorked)).c_str());
		if (removeWorked ^ x.equals(b.get())) {
			LOG("PASS");
		} else {
			LOG("FAIL");
			ES_ASSERT(false);
		}

//		return ;
	}
#endif

#if 1
	{
		EConcurrentLinkedQueue<EString> queue;
		queue.add(new EString("111111"));
		LOG("size=%d", queue.size());
		LOG("empty=%s", queue.isEmpty() ? "true" : "false");
		sp<EString> x = queue.poll();
		LOG("poll: %s", x->c_str());
		LOG("size=%d", queue.size());
		LOG("empty=%s", queue.isEmpty() ? "true" : "false");
		//queue.add(null);
		queue.add(new EString("222222"));
		x = queue.peek();
		LOG("peek=%s", x->c_str());
		LOG("size=%d", queue.size());
		LOG("poll: %s", x->c_str());
		EString z("111111");
		boolean r = queue.remove(&z);
		LOG("remove=%s", r ? "true" : "false");
		queue.add(new EString("111111"));
		r = queue.contains(&z);
		LOG("contains=%s", r ? "true" : "false");
		r = queue.remove(&z);
		LOG("remove=%s", r ? "true" : "false");

//		return;
	}
#endif

	//=====================================================

	#define ADD_SIZE 5
	#define POLL_SIZE 5

	class addCQueueThread : public EThread {
	private:
		EConcurrentLinkedQueue<EInteger>* queue;// = null;
		Count* c;// = null;

	public:
		addCQueueThread(EConcurrentLinkedQueue<EInteger>* queue, Count* c) {
			this->queue = queue;
			this->c = c;
		}

		void run() {
			int i = 1;
			try {
#if 0
				if ((long)eso_os_thread_current() % 3 == 0) {
					sp<EIterator<sp<EInteger> > > iter = queue->iterator();
					while (iter->hasNext()) {
						sp<EInteger> i = iter->next();
						LOG("i=%d", i->intValue());
					}
				}
#endif

//				while (true) {
				for (i=0; i<10000; i++) {
					gSumAdd.addAndGet(i);
					boolean r = queue->add(new EInteger(i));
					if (!r) {
						LOG("add fail...");
					}

					c->addcount1(); // add
					i++;

//					EThread::sleep(500);
				}

				queue->add(new EInteger(-1));

			} catch (EException& e) {
				e.printStackTrace();
			}
			LOG("end of add thread run().");
		}
	};

	class pollCQueueThread : public EThread {
	private:
		EConcurrentLinkedQueue<EInteger>* queue;// = null;
		Count* c;// = null;

	public:
		pollCQueueThread(EConcurrentLinkedQueue<EInteger>* queue, Count* c) {
			this->queue = queue;
			this->c = c;
		}

		void run() {
			int i = 0;
			try {
				while (true) {
					sp<EInteger> si = queue->poll();
					if (!!si && si->intValue() == -1) {
						break;
					}

					if (!!si) {
						gSumPull.addAndGet(si->intValue());
					}

//					LOG("poll si=%d", si->intValue());

//					if (i % 50) {
//						sp<EInteger> pi(new EInteger(i-1));
//						queue->remove(pi.get());
//					}
//					if (i % 100) {
//						queue->remove();
//					}

					c->addcount2(); // poll
					i++;

//					EThread::sleep(500);
				}
			} catch (EException& e) {
				e.printStackTrace();
			}
			LOG("end of poll thread run(), i=%d.", i);
		}
	};

	class ProbeThread : public EThread { //
	private:
		boolean run_;// = true;
		Count* cc;

	public:
		ProbeThread(Count* cc) : run_(true) {
			this->cc = cc;
		}

		void run() {
			int c1 = 0, c2 = 0;
			int cc1 = 0, cc2 = 0;
			while (this->run_) {
				c2 = cc->getcount1();
				cc2 = cc->getcount2();
				LOG("put:[%d/%d]  get:[%d/%d]",
						(c2 - c1) / 2, c2,
						(cc2 - cc1) / 2, cc2);
				c1 = c2;
				cc1 = cc2;

				try {
					EThread::sleep(1000 * 2 - 1);
				} catch (EException& ex) {
					LOG("Error[ProbeThread.run]:%s", ex.getMessage());
				}
			}
		}
	};

	try {
		Count* c = new Count();
		EConcurrentLinkedQueue<EInteger>* queue = new EConcurrentLinkedQueue<EInteger>();

		EArray<EThread*> arr;
		int i;
		for (i = 0; i < ADD_SIZE; i++) {
			addCQueueThread* pct = new addCQueueThread(queue, c);
			pct->start();// put
			arr.add(pct);
		}

		for (i = 0; i < POLL_SIZE; i++) {
			pollCQueueThread* gct = new pollCQueueThread(queue, c);
			gct->start(); // get
			arr.add(gct);
		}

//		ProbeThread* pt = new ProbeThread(c); //
//		pt->start();
//		arr.add(pt);

		for (i = 0; i < arr.length(); i++) {
			arr.getAt(i)->join();
		}

		ES_ASSERT(gSumAdd.llongValue() == gSumPull.llongValue());
		gSumAdd.set(0);
		gSumPull.set(0);


//		LOG("queue size=%d", queue->size());
//
//		sp<EIterator<sp<EInteger> > > iter = queue->iterator();
//		while (iter->hasNext()) {
//			sp<EInteger> i = iter->next();
//			LOG("i=%d", i->intValue());
//		}
//
//		sp<EInteger> I;
//		while ((I = queue->poll()) != null) {
//			LOG("I=%d", I->intValue());
//		}

		delete queue;
		delete c;

		LOG("test_concurrentLinkedQueue...");

//		EThread::sleep(3000);
	} catch (EException& e) {
		e.printStackTrace();
	}
}

static void test_concurrentLinkedQueue2() {

#define NUM 10000000

#if 1
	EConcurrentLinkedQueue<EInteger> queue;

	class Thread: public EThread {
	private:
		EConcurrentLinkedQueue<EInteger>& queue;
		int id;
	public:
		Thread(EConcurrentLinkedQueue<EInteger>& q, int i): queue(q), id(i) {}
		virtual void run() {
			int j = NUM * id;
			for (int i = 0; i < NUM; i++) {
				if (id < 2)
					queue.add(new EInteger(i));
				else {
					sp<EInteger> n = queue.poll();
					LOG("i=%d", i);
				}
			}
		}
	};

	es_int64_t ts1, ts2;
	ts1 = ESystem::currentTimeMillis();

	Thread t0(queue, 0);
	Thread t1(queue, 1);
	Thread t2(queue, 2);
	Thread t3(queue, 3);
	t0.start();
	t1.start();
	t2.start();
	t3.start();
	t0.join();
	t1.join();
	t2.join();
	t3.join();
#else
	EConcurrentLinkedQueue<EInteger> queue;

	EInteger* ii = new EInteger(999);
	queue.add(ii);

	class Thread: public EThread {
	private:
		EConcurrentLinkedQueue<EInteger>& queue;
		int id;
	public:
		Thread(EConcurrentLinkedQueue<EInteger>& q, int i): queue(q), id(i) {}
		virtual void run() {
			int j = NUM * id;
			for (int i = 0; i < NUM; i++) {
				sp<EInteger> n = queue.poll();
				queue.add(n);
			}
		}
	};

	es_int64_t ts1, ts2;
	ts1 = ESystem::currentTimeMillis();

	Thread t0(queue, 0);
	t0.start();
	t0.join();
#endif

	ts2 = ESystem::currentTimeMillis();
	LOG("%lld", ts2-ts1);
}

static void test_concurrentSkipListMap() {
	EConcurrentSkipListMap<EString, EString> slm;

	for (int i=0; i<10000; i++) {
		if (i==3) continue;
		slm.put(new EString(i), new EString(i));
	}

	sp<EString> k2(new EString("2"));
	sp<EString> v2(new EString("v2"));
	slm.put(k2, v2);
	sp<EString> v = slm.get(k2.get());
	LOG("v=%s", v->c_str());

	ESet<sp<EString> >* keyset = slm.keySet();
	sp<EIterator<sp<EString> > > iter1 = keyset->iterator();
	while (iter1->hasNext()) {
		sp<EString> s = iter1->next();
//		LOG("k=%s", s->c_str());
	}

	ECollection<sp<EString> >* values = slm.values();
	sp<EIterator<sp<EString> > > iter2 = values->iterator();
	while (iter2->hasNext()) {
		sp<EString> s = iter2->next();
//		LOG("v=%s", s->c_str());
	}

	ESet<sp<EConcurrentMapEntry<EString,EString> > >* entrySet = slm.entrySet();
	sp<EIterator<sp<EConcurrentMapEntry<EString,EString> > > > iter3 = entrySet->iterator();
	while (iter3->hasNext()) {
		sp<EConcurrentMapEntry<EString,EString> > e = iter3->next();
//		LOG("k=%s, v=%s", e->getKey()->c_str(), e->getValue()->c_str());
	}

	//test ctor from sortedmap.
	{
		ETreeMap<EInteger*,EString*> tm;
		for (int i=10000; i>0; i--) {
			EString* v = tm.put(new EInteger(i), new EString(EString::formatOf("value%d", i)));
			if (v) {
				delete v;
			}
		}

		tm.setAutoFree(false, false);

		if (0) {
			EConcurrentSkipListMap<EInteger, EString> slm_from_sortedmap(&tm);

			ESet<sp<EConcurrentMapEntry<EInteger,EString> > >* entrySet = slm_from_sortedmap.entrySet();
			sp<EIterator<sp<EConcurrentMapEntry<EInteger,EString> > > > iter = entrySet->iterator();
			while (iter->hasNext()) {
				sp<EConcurrentMapEntry<EInteger,EString> > e = iter->next();
				LOG("ki=%d, vs=%s", e->getKey()->intValue(), e->getValue()->c_str());
			}
		}
		else {
			EMap<EInteger*,EString*>* map = dynamic_cast<EMap<EInteger*,EString*>*>(&tm);
			EConcurrentSkipListMap<EInteger, EString> slm_from_map(map);

			ESet<sp<EConcurrentMapEntry<EInteger,EString> > >* entrySet = slm_from_map.entrySet();
			sp<EIterator<sp<EConcurrentMapEntry<EInteger,EString> > > > iter = entrySet->iterator();
			while (iter->hasNext()) {
				sp<EConcurrentMapEntry<EInteger,EString> > e = iter->next();
				LOG("ki_=%d, vs_=%s", e->getKey()->intValue(), e->getValue()->c_str());
			}
		}
	}

	//get
	sp<EString> k3(new EString("3"));
	sp<EConcurrentMapEntry<EString,EString> > e = slm.ceilingEntry(k3.get());
	LOG("ceilingEntry: k=%s, v=%s", e->getKey()->c_str(), e->getValue()->c_str());

	e = slm.firstEntry();
	LOG("firstEntry: k=%s, v=%s", e->getKey()->c_str(), e->getValue()->c_str());
	e = slm.lastEntry();
	LOG("lastEntry: k=%s, v=%s", e->getKey()->c_str(), e->getValue()->c_str());

	//remove
	LOG("slm.size = %d", slm.size());
	slm.remove(k2.get());
	LOG("slm.size = %d", slm.size());

	sp<EString> k4(new EString("4"));
	sp<EString> v4(new EString("4"));
	slm.remove(k4.get(), v4.get());
	LOG("slm.size = %d", slm.size());

	//replace
	sp<EString> k5(new EString("5"));
	slm.replace(k5.get(), new EString("xxxxxxxxxxxxx"));
	LOG("v5=%s", slm.get(k5.get())->c_str());
	sp<EString> v5(new EString("xxxxxxxxxxxxx"));
	LOG("containsValue v5=%d", slm.containsValue(v5.get()));
	slm.replace(k5.get(), v5.get(), new EString("zzzzzzzzzzzzzzzzzz"));
	LOG("v5=%s", slm.get(k5.get())->c_str());
	LOG("containsKey k5=%d", slm.containsKey(k5.get()));
	LOG("containsValue v5=%d", slm.containsValue(v5.get()));

	//clear
	slm.clear();
	LOG("slm.size = %d", slm.size());
}

static void test_concurrentSkipListMap2() {
	class putCHashmapThread : public EThread {
	private:
		EConcurrentSkipListMap<EInteger, EInteger>* chm;// = null;
		Count* c;// = null;

	public:
		putCHashmapThread(EConcurrentSkipListMap<EInteger, EInteger>* chm, Count* c) {
			this->chm = chm;
			this->c = c;
		}

		void run() {
			int tt = 13;
			int i = 1;
			try {
				ERandom r;
//				while (true) {
				for (i=0; i<100000; i++) {
					tt = EMath::abs(tt * (tt - i) - 119);
//                    tt = r.nextInt();
					sp<EInteger> oldV = chm->put(new EInteger(tt), new EInteger(0-tt));
//					LOG("put %d, oldV=%d", i, oldV != null ? oldV->intValue() : -1);

#if 1
					tt = i;
					EInteger ei(tt);

					//remove
					if (i % 10 == 0) {
						sp<EInteger> v = chm->remove(&ei);
//						if (v!=null) LOG("remove ei=%d, v=%d, tt=%d", ei.intValue(), v->intValue(), tt);
					}

					//replace
//					chm->replace(&ei, new EInteger(9999999));
//					LOG("ei=%d", chm->get(&ei)->intValue());
#endif
					c->addcount1(); // put
					i++;

//					EThread::sleep(500);
				}
			} catch (EException& e) {
				e.printStackTrace();
			}
			LOG("end of put thread run().");
		}
	};

	class getCHashmapThread : public EThread { // get
	private:
		EConcurrentSkipListMap<EInteger, EInteger>* chm;// = null;
		Count* c;// = null;

	public:
		getCHashmapThread(EConcurrentSkipListMap<EInteger, EInteger>* chm, Count* c) {
			this->chm = chm;
			this->c = c;
		}

		void run() {
			int tt = 13;
			int i = 1;
			try {
//				while (true) {
				for (i=0; i<100000; i++) {
					tt = EMath::abs(tt * (tt - i) - 119);
					EInteger ei(tt);
					sp<EInteger> v = chm->get(&ei);
//					LOG("get i=%d, v=%d", i, v==null ? -1 : v->intValue());

#if 0
					//test containsKey
					if (v!=null) {
						boolean r = chm->containsKey(&ei);
//						LOG("containsKey r=%d", r);
					}

					//test containsValue
					if (v!=null) {
						boolean r = chm->containsValue(v.get());
//						LOG("containsValue r=%d", r);
					}
#if 1
					//test iterator
					ESet<sp<EConcurrentMapEntry<EInteger,EInteger> > >* set = chm->entrySet();
					sp<EIterator<sp<EConcurrentMapEntry<EInteger,EInteger> > > > it = set->iterator();
					while (it->hasNext()) {
						sp<EConcurrentMapEntry<EInteger,EInteger> > me = it->next();
						sp<EInteger> ki = me->getKey();
						sp<EInteger> vi = me->getValue();
						if (ki != null && vi != null) {
//							LOG("k=%d, v=%d", ki->intValue(), vi->intValue());
						}
					}
#endif
#endif
					c->addcount2(); // get
					i++;

//					EThread::sleep(100);
				}
			} catch (EException& e) {
				e.printStackTrace();
			}
			LOG("end of get thread run().");
		}
	};

	class iteCHashmapThread : public EThread { // get操作线程
	private:
		EConcurrentSkipListMap<EInteger, EInteger>* chm;// = null;

	public:
		iteCHashmapThread(EConcurrentSkipListMap<EInteger, EInteger>* chm) {
			this->chm = chm;
		}

		void run() {
			try {
				chm->entrySet()->iterator();
			} catch (EException& e) {
				e.printStackTrace();
			}
			LOG("end of ite thread run().");
		}
	};

	class ProbeThread : public EThread { //
	private:
		boolean run_;// = true;
		Count* cc;

	public:
		ProbeThread(Count* cc) : run_(true) {
			this->cc = cc;
		}

		void run() {
			int c1 = 0, c2 = 0;
			int cc1 = 0, cc2 = 0;
			while (this->run_) {
				c2 = cc->getcount1();
				cc2 = cc->getcount2();
				LOG("put:[%d/%d]  get:[%d/%d]",
						(c2 - c1) / 2, c2,
						(cc2 - cc1) / 2, cc2);
				c1 = c2;
				cc1 = cc2;

				try {
					EThread::sleep(1000 * 2 - 1);
				} catch (EException& ex) {
					LOG("Error[ProbeThread.run]:%s", ex.getMessage());
				}
			}
		}
	};

	try {
		Count* c = new Count();
		EConcurrentSkipListMap<EInteger, EInteger>* chm = new EConcurrentSkipListMap<EInteger, EInteger>();

		llong t1 = ESystem::currentTimeMillis();
		for (int i=0; i<200000; i++) {
			sp<EInteger> oldV = chm->put(new EInteger(i), new EInteger(0-i));
		}

		LOG("t = %ld", ESystem::currentTimeMillis() - t1);

#if 1
		EArray<EThread*> arr;
		int i;
		for (i = 0; i < 2; i++) {
			putCHashmapThread* pct = new putCHashmapThread(chm, c);
			pct->start();// put
			arr.add(pct);
		}

		for (i = 0; i < 5; i++) {
			getCHashmapThread* gct = new getCHashmapThread(chm, c);
			gct->start(); // get
			arr.add(gct);
		}

//		for (i = 0; i < 2; i++) {
//			iteCHashmapThread* ict = new iteCHashmapThread(chm);
//			ict->start();// iterator
//			arr.add(ict);
//		}

//		ProbeThread* pt = new ProbeThread(c); //
//		pt->start();
//		arr.add(pt);

		for (i = 0; i < arr.length(); i++) {
			arr.getAt(i)->join();
		}
#endif
		delete chm;
		delete c;

		LOG("test_concurrentSkipListMap2...");

//		EThread::sleep(3000);
	} catch (EException& e) {
		e.printStackTrace();
	}
}

static void test_linkedTransferQueue()
{
	ELinkedTransferQueue<EString> ltq;

	ltq.add(new EString("1"));
	ltq.add(new EString("2"));
	ltq.add(new EString("3"));
	ltq.add(new EString("4"));

	sp<EString> a = ltq.peek();
	LOG("a=%s", a->c_str());
	while ((a = ltq.poll()) != null) {
		LOG("s=%s", a->c_str());
	}

	//============================

	class RunnerOffer: public ERunnable {
	private:
		ELinkedTransferQueue<EString>* queue;
	public:
		RunnerOffer(ELinkedTransferQueue<EString>* q): queue(q) {
		}
		virtual void run() {
			while (true) {
				EString* string = new EString("a");
				string->append(EMath::random());
				LOG("offer s=%s", string->c_str());
				queue->offer(string);
//				try {
//					EThread::sleep(100);
//				} catch (EInterruptedException& e) {
//					e.printStackTrace();
//				}
			}
		}
	};

	class RunnerTake: public ERunnable {
	private:
		ELinkedTransferQueue<EString>* queue;
	public:
		RunnerTake(ELinkedTransferQueue<EString>* q): queue(q) {
		}
		virtual void run() {
			while (true) {
				try {
					sp<EString> s = queue->take();
					LOG("take s=%s", s->c_str());
				} catch (EInterruptedException& e) {
					e.printStackTrace();
				}
			}
		}
	};

	EThread t1(new RunnerOffer(&ltq)); t1.start();
	EThread t2(new RunnerTake(&ltq)); t2.start();
	t1.join();
	t2.join();

	LOG("end of test_linkedTransferQueue...");
}

static void test_timeunit() {
	ETimeUnit* unit = ETimeUnit::DAYS;
	llong h = unit->toHours(33);
	LOG("h=%ld", h);
}

static void test_linkedBlockingQueue() {
	class addCQueueThread : public EThread {
	private:
		ELinkedBlockingQueue<EInteger>* queue;// = null;
		Count* c;// = null;

	public:
		addCQueueThread(ELinkedBlockingQueue<EInteger>* queue, Count* c) {
			this->queue = queue;
			this->c = c;
		}

		void run() {
			int i = 1;
			try {
				while (true) {
//				for (i=0; i<10000; i++) {
					queue->put(new EInteger(i));
//					boolean r = queue->add(new EInteger(i));
//					boolean r = queue->offer(new EInteger(i));
//					if (!r) {
//						LOG("add fail...");
//					}

					c->addcount1(); // add
					i++;

//					EThread::sleep(500);
				}
			} catch (EException& e) {
				e.printStackTrace();
			}
			LOG("end of add thread run().");
		}
	};

	class pollCQueueThread : public EThread {
	private:
		ELinkedBlockingQueue<EInteger>* queue;// = null;
		Count* c;// = null;

	public:
		pollCQueueThread(ELinkedBlockingQueue<EInteger>* queue, Count* c) {
			this->queue = queue;
			this->c = c;
		}

		void run() {
			int i = 0;
			try {
				while (true) {
					try {
						queue->remove();
					} catch(...) {
					}

					sp<EInteger> si = queue->take();
//					sp<EInteger> si = queue->poll();
//					sp<EInteger> si = (i %3) == 0 ? queue->poll() : queue->take();
					if (si == null) {
//						continue;
						break;
					}
//					LOG("poll si=%d", si->intValue());

					c->addcount2(); // poll
					i++;

//					if (i > 9000) {
//						break;
//					}

//					EThread::sleep(500);
				}
			} catch (EException& e) {
				e.printStackTrace();
			}
			LOG("end of poll thread run(), i=%d.", i);
		}
	};

	class ClearThread : public EThread { // 
		private:
			ELinkedBlockingQueue<EInteger>* queue;

		public:
			ClearThread(ELinkedBlockingQueue<EInteger>* queue) : queue(queue) {
			}

			void run() {
				EThread::sleep(100);
				queue->clear();
			}
	};

	class ProbeThread : public EThread { // 
	private:
		boolean run_;// = true;
		Count* cc;

	public:
		ProbeThread(Count* cc) : run_(true) {
			this->cc = cc;
		}

		void run() {
			int c1 = 0, c2 = 0;
			int cc1 = 0, cc2 = 0;
			while (this->run_) {
				c2 = cc->getcount1();
				cc2 = cc->getcount2();
				LOG("put:[%d/%d]  get:[%d/%d]",
						(c2 - c1) / 2, c2,
						(cc2 - cc1) / 2, cc2);
				c1 = c2;
				cc1 = cc2;

				try {
					EThread::sleep(1000 * 2 - 1);
				} catch (EException& ex) {
					LOG("Error[ProbeThread.run]:%s", ex.getMessage());
				}
			}
		}
	};

	try {
		Count* c = new Count();
		ELinkedBlockingQueue<EInteger>* queue = new ELinkedBlockingQueue<EInteger>(10000);

		EArray<EThread*> arr;
		int i;
		for (i = 0; i < 2; i++) {
			addCQueueThread* pct = new addCQueueThread(queue, c);
			pct->start();// put
			arr.add(pct);
		}

		for (i = 0; i < 5; i++) {
			pollCQueueThread* gct = new pollCQueueThread(queue, c);
			gct->start(); // get
			arr.add(gct);
		}

//		ClearThread* ct = new ClearThread(queue);
//		ct->start();
//		arr.add(ct);

		ProbeThread* pt = new ProbeThread(c); //
		pt->start();
		arr.add(pt);
//
//		LOG("queue size=%d", queue->size());
//
//		sp<EIterator<sp<EInteger> > > iter = queue->iterator();
//		while (iter->hasNext()) {
//			sp<EInteger> i = iter->next();
//			LOG("i=%d", i->intValue());
//		}
//
//		sp<EInteger> I;
//		while ((I = queue->poll()) != null) {
//			LOG("I=%d", I->intValue());
//		}

		for (i = 0; i < arr.length(); i++) {
			arr.getAt(i)->join();
		}

		delete queue;
		delete c;

		LOG("test_linkedBlockingQueue...");

//		EThread::sleep(3000);
	} catch (EException& e) {
		e.printStackTrace();
	}
}

static llong gLockTestCount = 0;
static void test_lock() {

	ALOG("test_lock...0");

	class Thread1 : public EThread {
	private:
		ELock* lock;
		Count* c;// = null;

	public:
		Thread1(const char*name, Count* c, ELock* lock) : EThread(name) {
			this->lock = lock;
			this->c = c;
		}

		void run() {
			try {
				int i=0;
				do {

					SYNCBLOCK(lock) {
					c->addcount1(); // add

//					ERandom a;
//					int n = a.nextInt();
//					if (n > 100) {
//						EThread::sleep(10);
//					}

					gLockTestCount++;

					}}
					llong n = gLockTestCount1.incrementAndGet();
					if (n % 99999 == 0) {
						printf("Thread1_%s....%d.\n", getName(), n);
//						gLockTestCount1.set(0);
					}
					ALOG("Thread1:%d", i);
				} while (i++ < 1000000);
//				} while (1);
//				printf("Thread1....end.\n");
			} catch (EException& e) {
				e.printStackTrace();
			}

//			EThread::sleep(1000);
		}
	};

	class Thread2 : public EThread {
	private:
		ELock* lock;
		Count* c;// = null;

	public:
		Thread2(const char*name, Count* c, ELock* lock) : EThread(name) {
			this->lock = lock;
			this->c = c;
		}

		void run() {
			try {
				int i=0;
				do {
					SYNCBLOCK(lock) {
					c->addcount2(); // poll
					gLockTestCount++;
					}}
					llong n = gLockTestCount1.incrementAndGet();
					if (n % 99999 == 0) {
						printf("Thread2_%s....%d.\n", getName(), n);
//						gLockTestCount2.set(0);
					}
					ALOG("Thread2:%d", i);
				} while (i++ < 1000000);
//				} while (1);
//				printf("Thread2....end.\n");
			} catch (EException& e) {
				e.printStackTrace();
			}

//			EThread::sleep(1000);
		}
	};

	class ProbeThread : public EThread { // 
	private:
		boolean run_;// = true;
		Count* cc;
		EArray<EThread*>* arr;

	public:
		ProbeThread(Count* cc, EArray<EThread*>* arr) : run_(true) {
			this->cc = cc;
			this->arr = arr;
		}

		void run() {
			int c1 = 0, c2 = 0;
			int cc1 = 0, cc2 = 0;
			while (this->run_) {
				c2 = cc->getcount1();
				cc2 = cc->getcount2();
				LOG("put:[%d/%d]  get:[%d/%d]",
						(c2 - c1) / 2, c2,
						(cc2 - cc1) / 2, cc2);
				c1 = c2;
				cc1 = cc2;

				try {
					EThread::sleep(1000 * 2 - 1);
				} catch (EException& ex) {
					LOG("Error[ProbeThread.run]:%s", ex.getMessage());
				}

				for (int i = 0; i < arr->length(); i++) {
//					int n = arr->getAt(i)->parkCount.value();
//					printf("t cv=%d\n", n);
//					ES_ASSERT(n <= 0);
				}
			}
		}
	};

	try {
//		ESimpleLock rLock;
		EReentrantLock rLock;
//		ESpinLock rLock;
		Count* c = new Count();

		EArray<EThread*> arr;
		int i;
		for (i = 0; i < 100; i++) {
			Thread1* pct = new Thread1(EString(i).c_str(), c, &rLock);
			pct->start();// put
			arr.add(pct);
		}

		for (i = 0; i < 100; i++) {
			Thread2* gct = new Thread2(EString(i).c_str(), c, &rLock);
			gct->start(); // get
			arr.add(gct);
		}

//		ProbeThread* pt = new ProbeThread(c, &arr); //
//		pt->start();
//		arr.add(pt);

		for (i = 0; i < arr.length(); i++) {
//			printf("t park=%d, unpark=%d, wait=%d, signal=%d, diff=%d\n",
//					arr.getAt(i)->parkCount0.value(),
//					arr.getAt(i)->unparkCount0.value(),
//					arr.getAt(i)->unparkCount.value() + arr.getAt(i)->parkCount.value(),
//					arr.getAt(i)->unparkCount.value(),
//					arr.getAt(i)->parkCount.value());
			arr.getAt(i)->join();
		}

		ES_ASSERT(gLockTestCount == gLockTestCount1.llongValue());

		delete c;
	} catch (EException& e) {
		e.printStackTrace();
	}

	ALOG("test_lock...1");
//	printf("end of test_lock().\n");
}

static void test_tryLock() {
	class MyThread : public EThread {
	private:
		ELock* lock;
	public:
		MyThread(ELock* lock) {
			this->lock = lock;
		}
		void run() {
			LOG("tryLock...");
//			boolean rv = lock->tryLock();
			boolean rv = lock->tryLock(2, ETimeUnit::SECONDS);
			LOG("tryLock rv=%d", rv);
			if (rv) lock->unlock();
		}
	};

//	ELock* lock = new EReentrantLock();
//	ELock* lock = new ESimpleLock();
	ELock* lock = new ESpinLock();
	EThread *t1 = new MyThread(lock);
	t1->start();
	lock->lock();
	EThread::sleep(30000);
	lock->unlock();
	delete t1;
	delete lock;
}

static void test_lockInterruptibly() {
	class MyThread : public EThread {
	private:
		ELock* lock;
	public:
		MyThread(ELock* lock) {
			this->lock = lock;
		}
		void run() {
			try {
				LOG("sleep...");
				EThread::sleep(10000);
//				lock->lockInterruptibly();
				LOG("...");
			} catch (EInterruptedException& e) {
				LOG("%s%s", EThread::currentThread()->getName(), " interrupted.");
			}
		}
	};

	EReentrantLock* lock = new EReentrantLock();
	LOG("lock is fair? %s", lock->isFair() ? "yes" : "no");
	EThread *t1 = new MyThread(lock);
	t1->start();
	EThread::sleep(5000);
	t1->interrupt();
	EThread::sleep(20000);
	delete t1;
	delete lock;
}

static void test_readWriteLock() {
	class Queue3{
	private:
		EInteger* data;// = null;
	    EReentrantReadWriteLock* rwl;// = new EReentrantReadWriteLock();
	    EAtomicCounter readCount;
	    EAtomicCounter writeCount;
	public:
	    Queue3() : data(null) {
	    	rwl = new EReentrantReadWriteLock();
	    }
	    ~Queue3() {
	    	delete data;
	    	delete rwl;
	    }
	    void get(){
	        rwl->readLock()->lock();
	        readCount++;
//	        LOG("%s be ready to read data! count=%d", EThread::currentThread()->getName(), readCount.value());
	        EThread::sleep((long)(EMath::random()*1000));
//	        LOG("%s have read data : %d", EThread::currentThread()->getName(), data ? data->value : -1);
	        readCount--;
	        rwl->readLock()->unlock(); //
	    }

	    void put(EInteger* data){

	        rwl->writeLock()->lock();//
	        writeCount++;
	        ES_ASSERT(writeCount.value() == 1);
//	        LOG("%s be ready to write data!", EThread::currentThread()->getName());
	        EThread::sleep((long)(EMath::random()*1000));
	        EInteger* old = this->data;
	        this->data = data;
	        delete old;
//	        LOG("%s have write data: %d", EThread::currentThread()->getName(), data->value);
	        ES_ASSERT(writeCount.value() == 1);
	        writeCount--;
	        rwl->writeLock()->unlock();//
	    }
	};

	class MyThread1 : public EThread {
	private:
		Queue3 *q;
	public:
		MyThread1(Queue3* q3) : q(q3) {
		}
		void run() {
			int i=0;
			do {
				q->get();
			} while(i++ < 5);
		}
	};

	class MyThread2 : public EThread {
	private:
		Queue3 *q;
	public:
		MyThread2(Queue3* q3) : q(q3) {
		}
		void run() {
			int i=0;
			do {
				ERandom r;
				q->put(new EInteger(r.nextInt(10000)));
			} while(i++ < 5);
		}
	};

	Queue3 q3;
	EArray<EThread*> arr;
	int i;

	for(i=0;i<10;i++)
	{
		MyThread1* mt = new MyThread1(&q3);
		arr.add(mt);
		mt->start();
	}
	for(i=0;i<5;i++)
	{
		MyThread2* mt = new MyThread2(&q3);
		arr.add(mt);
		mt->start();
	}
	for (i = 0; i < arr.length(); i++) {
		arr.getAt(i)->join();
//		printf("t cv=%d\n", arr.getAt(i)->parkCount.value());
	}

	LOG("end of test_readWriteLock.");
}

static void test_identityHashMap() {
	EIdentityHashMap<EString*, EString*>* ihm = new EIdentityHashMap<EString*, EString*>();

	EString* first = new EString("xx");
	EString* second = new EString("xx");
	ihm->put(first, new EString("first"));
	ihm->put(second, new EString("second"));

	LOG(ihm->get(first)->c_str());
	LOG(ihm->get(second)->c_str());
	EString* third = new EString("xx");
	EString* v = ihm->get(third);
	LOG(v ? v->c_str() : "null");
	delete third;

	EString kxx("xx");
	LOG("containsKey(xx) = %d", ihm->containsKey(&kxx));
	LOG("get(xx) = %s", ihm->get(&kxx));

	ESet<EString*>* kset = ihm->keySet();
	LOG("set size=%d", kset->size());
	sp<EIterator<EString*> > iterK = kset->iterator();
	while(iterK->hasNext()) {
		LOG("key=%s", iterK->next()->c_str());
	}

	ECollection<EString*>* vset = ihm->values();
	sp<EIterator<EString*> > iterV = vset->iterator();
	while(iterV->hasNext()) {
		LOG("val=%s", iterV->next()->c_str());
	}

	ESet<EMapEntry<EString*, EString*>*>* eset = ihm->entrySet();
	LOG("set size=%d", eset->size());
	sp<EIterator<EMapEntry<EString*, EString*>*> > iterE = eset->iterator();
	while(iterE->hasNext()) {
		EMapEntry<EString*, EString*>* e = iterE->next();
		LOG("key=%s, val=%s", e->getKey()->c_str(), e->getValue()->c_str());
	}
	LOG("iterE::hashCode=%d", iterE->hashCode());

	delete ihm;
}

#ifndef WIN32
#define LOCK_IF_MP(mp) "cmp $0, " #mp "; je 1f; lock; 1: "
static int     cmpxchg    (int     exchange_value, volatile int*     dest, int     compare_value) {
  int mp = 1;
  __asm__ volatile (LOCK_IF_MP(%4) "cmpxchgl %1,(%3)"
                    : "=a" (exchange_value)
                    : "r" (exchange_value), "a" (compare_value), "r" (dest), "r" (mp)
                    : "cc", "memory");
  return exchange_value;
}

static void test_cmpxchg() {
	volatile int v = 1;
	int r = cmpxchg(2, &v, 0);
	printf("r=%d\n", r);
}
#endif

EThreadLocalVariable<EThreadLocal, ELLong> gThreadLocal;

static void test_threadlocal1() {
	class TestThread: public EThread {
	public:
		TestThread(const char* name) : EThread(name) {
		}
		void run() {
			ulong id = eso_os_thread_current_id();

			for (int i=0; i<1000; i++) {
				ELLong* l = gThreadLocal.get();
//				printf("tid=%d, l=%ld\n", id, l ? l->value : -1);
				delete gThreadLocal.set(new ELLong(id+i));
			}
			printf("this thread: %s\n", this->getName());
		}
	};

	printf("test_thread start.\n");

	TestThread t1("#1"), t2("#2"), t3("#3");
	t1.start();
	t2.start();
	t3.start();

	t1.join();
	t2.join();
	t3.join();

	printf("test_thread end.\n");
}

static void test_threadlocal2() {
	class TestThread: public EThread {
	public:
		TestThread(const char* name) : EThread(name) {
		}
		void run() {
			EThreadLocalVariable<EThreadLocal, ELLong> local;
			ulong id = eso_os_thread_current_id();

			for (int i=0; i<1000; i++) {
				ELLong* l = local.get();
//				printf("tid=%d, l=%ld\n", id, l ? l->value : -1);
				delete local.set(new ELLong(id+i));
			}
			printf("this thread: %s\n", this->getName());
		}
	};

	printf("test_thread start.\n");

	TestThread t1("#1"), t2("#2"), t3("#3");
	t1.start();
	t2.start();
	t3.start();

	t1.join();
	t2.join();
	t3.join();

	printf("test_thread end.\n");
}

static void test_threadlocal3() {
	class XXLocal : public EThreadLocal {
	protected:
		virtual EObject* initialValue() {
			return new ELLong(-1);
		}
	};
	class TestThread: public EThread {
	public:
		TestThread(const char* name, EThreadLocalVariable<XXLocal, ELLong>* local) : EThread(name) {
			this->local = local;
		}
		void run() {
			ulong id = eso_os_thread_current_id();

			ELLong* l = local->get();
			ES_ASSERT(l && l->value == -1);

			for (int i=0; i<1000; i++) {
				ELLong* l = local->get();
				printf("tid=%lu, l=%lld\n", id, l ? l->value : -1);
				delete local->set(new ELLong(id+i));
			}
			printf("this thread: %s\n", this->getName());
		}
	private:
		EThreadLocalVariable<XXLocal, ELLong>* local;
	};

	printf("test_thread start.\n");

	EThreadLocalVariable<XXLocal, ELLong> local;
	TestThread t1("#1", &local), t2("#2", &local), t3("#3", &local);
	t1.start();
	t2.start();
	t3.start();

	t1.join();
	t2.join();
	t3.join();

	printf("test_thread end.\n");
}

static void test_threadlocal4() {
	class XXLocal : public EThreadLocal {
	protected:
		virtual EObject* initialValue() {
			return new ELLong(-1);
		}
	};
	class TestThread: public EThread {
	public:
		TestThread(const char* name, EThreadLocalVariable<XXLocal, ELLong>* local, EReentrantLock* lock) : EThread(name) {
			this->local = local;
			this->lock = lock;
		}
		void run() {
			ulong id = eso_os_thread_current_id();

			int i = 0;
			do {
			EThreadLocalVariable<XXLocal, ELLong> xl;

//			LOG("1111!");

			//1)
			for (int i=0; i<1000; i++) {
//				LOG("tid=%ld, i=%ld", id, local->get()->value);
				if (i%3 == 0) {
					local->remove();
				}
				else {
					delete local->set(new ELLong(id+i));
				}
//				printf("get: %d\n", xl.get()->value);
				delete xl.set(new ELLong(i));
			}
			//2)
			SYNCBLOCK(lock) {
//				printf("x\n");
			}}

			for (int i=0; i<10000; i++) {
				EReentrantLock llock;
				SYNCBLOCK(&llock) {
//					printf("y\n");
				}}
			}
			} while (i++ < 1000);
//			LOG("success!\n");
		}
	private:
		EThreadLocalVariable<XXLocal, ELLong>* local;
		EReentrantLock* lock;
	};

//	printf("test_thread start.\n");

	EThreadLocalVariable<XXLocal, ELLong> local;
	EReentrantLock lock;
	TestThread t1("#1", &local, &lock), t2("#2", &local, &lock), t3("#3", &local, &lock);
	t1.start();
	t2.start();
	t3.start();

	t1.join();
	t2.join();
	t3.join();

	printf("test_thread end.\n");
}

static void test_threadlocal5() {
	EThreadLocalVariable<EThreadLocal, EString> tl;

	tl.set(new EString("ThreadLocal-VAL"));
	LOG("Main-1:%s", tl.get()->c_str());

	class Thread1: public EThread {
	private:
		EThreadLocalVariable<EThreadLocal, EString>* tl;
	public:
		Thread1(EThreadLocalVariable<EThreadLocal, EString>* t): tl(t) {
		}
		virtual void run() {
			EString* s = tl->get();
			LOG("Child-1:%s", s ? s->c_str() : "null");
		}
	};

	EThreadLocalVariable<EInheritableThreadLocal, EString> itl;

	itl.set(new EString("InheritableThreadLocal-VAL"));
	LOG("Main-2:%s", tl.get()->c_str());

	class Thread2: public EThread {
	private:
		EThreadLocalVariable<EInheritableThreadLocal, EString>* tl;
	public:
		Thread2(EThreadLocalVariable<EInheritableThreadLocal, EString>* t): tl(t) {
		}
		virtual void run() {
			EString* s = tl->get();
			LOG("Child-2:%s", s ? s->c_str() : "null"); //FIXME:
		}
	};

	Thread1 t1(&tl);
	Thread2 t2(&itl);
	t1.start();
	t2.start();
	t1.join();
	t2.join();
}

static void test_vector() {
	EVector<EString*> *vector = new EVector<EString*>();
	vector->setThreadSafe(false);
	EString *string1 = new EString("abcdefg");
	EString *string2 = new EString("advdsafds");
	EString *string3 = new EString("12243");
	vector->addElement(string1);
	vector->addElement(string2);
	vector->addElement(string3);
	sp<EEnumeration<EString*> > e = vector->elements();
	for (; e != null && e->hasMoreElements();) {
		EString *string = (EString*)e->nextElement();
		eso_log("next element, string=[%s]\n", string->c_str());
	}
	int n = vector->lastIndexOf(string2);
	eso_log("n=%d\n", n);
	int m = vector->lastIndexOf(string2, 1);
	eso_log("m=%d\n", m);
	int l = vector->lastIndexOf(string1, 1);
	eso_log("l=%d\n", l);

	//test copy constructor
	{
		EVector<EString*> vcopy(*vector);

		e = vector->elements();
		for (; e != null && e->hasMoreElements();) {
			EString *string = (EString*)e->nextElement();
			LOG("next element, string_=[%s]", string->c_str());
		}

		vcopy.setAutoFree(false);
		vector->setAutoFree(true);
	}

	delete vector;
}

static void test_vector2() {
	EVector<EString*> *vector = new EVector<EString*>();

	class Thread : public EThread {
	private:
		EVector<EString*> *vector;
	public:
		Thread(EVector<EString*> *v) : vector(v) {
		}

		void run() {
			EString *string1 = new EString("abcdefg");
			EString *string2 = new EString("advdsafds");
			EString *string3 = new EString("12243");
			vector->addElement(string1);
			vector->addElement(string2);
			vector->addElement(string3);
			sp<EEnumeration<EString*> > e = vector->elements();
			for (; e != null && e->hasMoreElements();) {
				EString *string = (EString*)e->nextElement();
				eso_log("next element, string=[%s]\n", string->c_str());
			}
			int n = vector->lastIndexOf(string2);
			eso_log("n=%d\n", n);
			int m = vector->lastIndexOf(string2, 1);
			eso_log("m=%d\n", m);
			int l = vector->lastIndexOf(string1, 1);
			eso_log("l=%d\n", l);
		}
	};

	EArray<EThread*> arr;
	int i;

	for (i = 0; i < 10; i++) {
		Thread* t = new Thread(vector);
		arr.add(t);
		t->start();
	}

	for (i = 0; i < arr.length(); i++) {
		arr.getAt(i)->join();
	}

	delete vector;
}

class Publisher : public EObservable {
private:
	EString magazineName;

public:
	EString getMagazineName() {
		return magazineName;
	}

	void publish(EString magazineName) {
		this->magazineName = magazineName;
		setChanged();
		notifyObservers(this);
	}
};

class Reader : public EObserver {
public:
	void update(EObservable* o, void* arg) {
		Publisher* p = (Publisher*) o;

		EThread::sleep(100);

		LOG("reader:%s", p->getMagazineName().c_str());
	}
};

static void test_observable() {

	sp<EObserver> reader(new Reader());

	Publisher* publisher = new Publisher();
	publisher->addObserver(reader);
	publisher->publish("publish");

	delete publisher;
}

static void test_observable2()
{
	class Thread : public EThread {
	private:
		Publisher* publisher;
	public:
		Thread(Publisher* p) : publisher(p) {

		}
		void run() {
			sp<EObserver> reader(new Reader());
			publisher->addObserver(reader);
		}
	};

	Publisher* publisher = new Publisher();

	EArray<EThread*> arr;
	for (int i=0; i<10; i++) {
		Thread* t = new Thread(publisher);
		arr.add(t);
		t->start();
	}

	publisher->publish("publish");

	for (int y=0; y<arr.length(); y++) {
		arr.getAt(y)->join();
	}

	delete publisher;
}

static void test_countDownLatch()
{
	int LATCH_SIZE = 50;
	ECountDownLatch doneSignal(LATCH_SIZE);

	class InnerThread : public EThread {
	private:
		ECountDownLatch *doneSignal;
	public:
		InnerThread(int id, ECountDownLatch *o) : doneSignal(o) {
			this->setName(EString(id).c_str());
		}
		void run() {
			try {
				EThread::sleep(1000);
				LOG("%s sleep 1000ms.", EThread::currentThread()->getName());

				doneSignal->countDown();
			} catch (EInterruptedException& e) {
				e.printStackTrace();
			}
		}
	};

	EArray<EThread*> arr;

	try {
		for (int i = 0; i < LATCH_SIZE; i++) {
			InnerThread *t = new InnerThread(i, &doneSignal);
			t->start();
			arr.add(t);
		}
		LOG("main await begin.");

		doneSignal.await();

//		for (int i = 0; i < LATCH_SIZE; i++) {
//			arr.getAt(i)->join();
//		}

		LOG("main await finished.");
	} catch (EInterruptedException& e) {
		e.printStackTrace();
	}
}

static void test_cyclicBarrier() {
	int LATCH_SIZE = 50;
	ECyclicBarrier cb(LATCH_SIZE);

	class InnerThread : public EThread {
	private:
		ECyclicBarrier *cb;
	public:
		InnerThread(int id, ECyclicBarrier *o) : cb(o) {
			this->setName(EString(id).c_str());
		}
		void run() {
			try {
				LOG("%s wait for CyclicBarrier.", this->getName());

				cb->await();

				LOG("%s continued.", this->getName());
			} catch (EBrokenBarrierException& e) {
				e.printStackTrace();
			} catch (EInterruptedException& e) {
				e.printStackTrace();
			}
		}
	};

	EArray<EThread*> arr;

	for (int i = 0; i < LATCH_SIZE; i++) {
		InnerThread *t = new InnerThread(i, &cb);
		t->start();
		arr.add(t);
	}

	for (int i = 0; i < LATCH_SIZE; i++) {
		arr.getAt(i)->join();
	}

//	LOG("main here.");
}

static void test_semaphore() {
	int LATCH_SIZE = 4;
	ESemaphore semaphore(2);

	class InnerThread : public EThread {
	private:
		ESemaphore *semaphore;
	public:
		InnerThread(int id, ESemaphore *o) : semaphore(o) {
			this->setName(EString(id).c_str());
		}
		void run() {
			SEMAPHORE_SYNCBLOCK(semaphore) {
				LOG("Hello %s", this->getName());
				EThread::sleep(2000);
			}}
			LOG("Goodbye %s", this->getName());

		}
	};

	EArray<EThread*> arr;

	for (int i = 0; i < LATCH_SIZE; i++) {
		InnerThread *t = new InnerThread(i, &semaphore);
		t->start();
		arr.add(t);
	}

	for (int i = 0; i < LATCH_SIZE; i++) {
		arr.getAt(i)->join();
	}

	//	LOG("main here.");
}

static void test_runtime() {
	ERuntime* runtime = ERuntime::getRuntime();

	int cpus = runtime->availableProcessors();
	ullong freeMemory = runtime->freeMemory();
	ullong totalMemory = runtime->totalMemory();
	ullong maxMemory = runtime->maxMemory();

	LOG("cpus=%d", cpus);
	LOG("freeMemory=%lld", freeMemory);
	LOG("totalMemory=%lld", totalMemory);
	LOG("maxMemory=%lld", maxMemory);

	EProcess* proc = runtime->exec("/bin/ls -l");
	EInputStream* is = proc->getInputStream();
	char x[4096] = {0};
	is->read(x, sizeof(x));
	LOG(x);
	delete proc;
}

static void test_exchanger() {

#define EXCHANGER_TIMES  10

	class RunnerA : public ERunnable {
	public:
		EExchanger<EInteger>* exchanger;
		EAtomicReference<EInteger*>* last;
		EInteger* ei;

		RunnerA(EExchanger<EInteger>* exchanger) {
			this->exchanger = exchanger;
			ei = new EInteger(5);
			last = new EAtomicReference<EInteger*>(ei);
		}

		~RunnerA() {
			last->set(null); // clear the last one!
			delete last;
			delete ei;
		}

		void run() {
			try {
				int i = 0;
				while (i++ < EXCHANGER_TIMES) {
					last->set(exchanger->exchange(last->get()));

					llong n = gLockTestCount1.incrementAndGet();
					if (n % 999999 == 0) {
						printf("RunnerA....%lld.\n", n);
					}

					LOG(" After calling exchange. Thread A has value: %d", last->get()->value);
//					EThread::sleep(2000);
				}
			} catch (EInterruptedException& e) {
				e.printStackTrace();
			}
		}
	};

	class RunnerB : public ERunnable {
	public:
		EExchanger<EInteger>* exchanger;
		EAtomicReference<EInteger*>* last;
		EInteger* ei;

		RunnerB(EExchanger<EInteger>* exchanger) {
			this->exchanger = exchanger;
			ei = new EInteger(10);
			last = new EAtomicReference<EInteger*>(ei);
		}

		~RunnerB() {
			last->set(null); // clear the last one!
			delete last;
			delete ei;
		}

		void run() {
			try {
				int i = 0;
				while (i++ < EXCHANGER_TIMES) {
					last->set(exchanger->exchange(last->get()));

					llong n = gLockTestCount2.incrementAndGet();
					if (n % 999999 == 0) {
						printf("RunnerB....%lld.\n", n);
					}

					LOG(" After calling exchange. Thread B has value: %d", last->get()->value);
//					EThread::sleep(2000);
				}
			} catch (EInterruptedException& e) {
				e.printStackTrace();
			}
		}
	};

	EExchanger<EInteger> exchanger;

#if 0
	EArray<EThread*> arr1;
	EArray<EThread*> arr2;

	int LATCH_SIZE = 2; // deadlock on thread.join() when >1!!!

	for (int i = 0; i < LATCH_SIZE; i++) {
		RunnerA* ra = new RunnerA(&exchanger);
		RunnerB* rb = new RunnerB(&exchanger);
		EThread *t1 = new EThread(ra);
		EThread *t2 = new EThread(rb);
		t1->start();
		t2->start();
		arr1.add(t1);
		arr2.add(t2);
	}

	for (int i = 0; i < LATCH_SIZE; i++) {
		arr1.getAt(i)->join();
		arr2.getAt(i)->join();
	}
#else
	EThread* t1 = new EThread(new RunnerA(&exchanger));
	t1->start();
	EThread* t2 = new EThread(new RunnerB(&exchanger));
	t2->start();

	t1->join();
	t2->join();

	delete t1;
	delete t2;
#endif
}

struct VolatileLong {
	volatile long value;
	VolatileLong() {
		value = 0L;
	}
};

struct VolatileLong2 {
	volatile long p0, p1, p2, p3, p4, p5, p6;
	volatile long value;// = 0L;
	volatile long q0, q1, q2, q3, q4, q5, q6;
	VolatileLong2() {
		value = 0L;
	}
};

static void test_falseSharing() {
	static const int NUM_THREADS = 4;
	static const long ITERATIONS = 500L * 1000L * 1000L;

	class FalseSharing: public ERunnable {
	public:
		int arrayIndex;
		FalseSharing(int arrayIndex) {
		    this->arrayIndex = arrayIndex;
		}

		void run() {
			// select VolatileLong or VolatileLong2
			EA<VolatileLong*> longs(NUM_THREADS);
			for (int i = 0; i < longs.length(); i++) {
				longs[i] = new VolatileLong();
			}

			long i = ITERATIONS + 1;
			while (0 != --i) {
				longs[arrayIndex]->value = i;
			}
		}
	};

	EArray<EThread*> arr1;
	llong start = ESystem::nanoTime();

	for (int i = 0; i < NUM_THREADS; i++) {
		FalseSharing* fs = new FalseSharing(i);
		EThread *t1 = new EThread(fs);
		t1->start();
		arr1.add(t1);
	}

	for (int i = 0; i < NUM_THREADS; i++) {
		arr1.getAt(i)->join();
	}

	LOG("duration = %lld", (ESystem::nanoTime() - start));
}

static void test_timer() {
	class timerTask : public ETimerTask {
	private:
		int n;
	public:
		timerTask(int n) {
			this->n = n;
		}
		void run() {
			LOG("timer running, n=%d...", n);
		}
	};

	ETimer* timer = new ETimer();
	timer->schedule(new timerTask(1), 100);
	timer->schedule(new timerTask(1), 10, 200);
	timer->schedule(new timerTask(2), 10, 100);

	for (int i=0; i<10; i++) {
		timer->schedule(new timerTask(i), 10, 100);
	}

	EThread::sleep(2000);
//	timer->cancel();
//	EThread::sleep(1000);

	LOG("xxxxxxxxxxxxxxxx");

	delete timer;

	//TODO: memory leak!!!
}

static void test_priorityQueue() {
	class MyComparator : public EComparator<EInteger*> {
	public:
		int compare(EInteger* i, EInteger* j) {
			int result = i->intValue() % 2 - j->intValue() % 2;
			if (result == 0) {
				result = i->intValue() - j->intValue();
			}
			return result;
		}
	};

	const int QUEUE_SIZE = 10000;

	MyComparator* cmp = new MyComparator();
	EPriorityQueue<EInteger*>* pq = new EPriorityQueue<EInteger*>(QUEUE_SIZE, cmp, true);

	for (int i = 0; i < QUEUE_SIZE; i++) {
		pq->offer(new EInteger(QUEUE_SIZE - i));
		pq->offer(new EInteger(i*3 % QUEUE_SIZE));
	}

	//test copy constructor
	{
		EPriorityQueue<EInteger*> pq_(*pq);

		sp<EIterator<EInteger*> > iter = pq->iterator();
		while (iter->hasNext()) {
			EInteger* i = iter->next();
//			LOG("i0_=%d", i->intValue());
		}

		pq_.setAutoFree(false);
	}
	pq->setAutoFree(true);

	int n = 0;
	sp<EIterator<EInteger*> > iter = pq->iterator();
	while (iter->hasNext()) {
		EInteger* i = iter->next();
//		LOG("i0=%d", i->intValue());
		if (n == 0 || n == QUEUE_SIZE || n == 2 || n == 55 || n == QUEUE_SIZE-1) {
			iter->remove();
		}
		n++;
	}

	LOG("================= 0");

    iter = pq->iterator();
	while (iter->hasNext()) {
		EInteger* i = iter->next();
//		LOG("i1=%d", i->intValue());
	}

    LOG("================= 1");
    
    ERandom r;
    int idx = r.nextInt();
	EInteger* m = new EInteger(idx);
	pq->remove(m);
	delete m;

    n = pq->size();
	for (int i = 0; i < n; i++) {
		EInteger* e = pq->poll();
//		LOG("i2=%d", e->intValue());
		delete e;
	}

	delete pq;
	delete cmp;

	LOG("================= 2\n");
}

static void test_bson() {
	class BsonParser : public EBsonParser {
	public:
		BsonParser(EInputStream *is) :
			EBsonParser(is) {
		}
		void parsing(es_bson_node_t* node) {
			if (!node) return;

			for (int i=1; i<_bson->levelOf(node); i++) {
				printf("\t");
			}
			printf(node->name);
			printf("-->");
			EString s((char*)node->value->data, 0, node->value->len);
			printf(s.c_str());
			printf("\n");
		}
	};

	EBson bson;
	bson.add("/node/a", "a");
	bson.add("/node/b", "b");
	bson.add("/node", "0001");
	bson.add("/node/A", "1");
	bson.add("/node/B", "B");
	bson.add("/node/B", "B2");
	bson.add("/node/B", "B3");
	bson.add("/node/B", "B4");
	bson.add("/node/C", "0.99");
	bson.add("/node/A", "A");
	bson.add("/node/A|2", "a");
	bson.add("/node/B/B1", "B1");
	bson.add("/node/B/B3", "B3");
	bson.add("/node/B/B3|1", "B2");
	bson.add("/node", "0002");

	int count = eso_bson_node_count(bson.c_bson(), "node/B");
	LOG("node(/node/B) count=%d", count);

	while (count > 1) {
		bson.del("/node/B|0");
		count--;
	}

	es_buffer_t *buffer = eso_buffer_make(32, 32);
	bson.Export(buffer, null, false);

	EBson bsonx;
	bsonx.Import(buffer->data, buffer->len);
	bsonx.Export(buffer, null, false);

	EBson bson2;
	bson2.add("/aaa/a", "a");
	bson2.add("/aaa", "aaa");

	eso_buffer_clear(buffer);
	bson.Export(buffer, null, false);
	bson2.Export(buffer, null, false);

	EByteArrayInputStream bais(buffer->data, buffer->len);

	BsonParser ep(&bais);
	EBson bson_;
	while (ep.nextBson(&bson_)) {
		LOG("get one bson.");
	}
	eso_buffer_free(&buffer);

	EBson bson__(bson);
	bson__.save("/tmp/bson.txt", NULL);

	bson__ = bson;
	bson__.save("/tmp/bson2.txt", NULL);
}

static void test_threadPoolExecutor()
{
	class XXX : public ERunnable {
	private:
		int id;
	public:
		XXX(int id) {
			this->id = id;
		}
		~XXX() {
//			LOG("~XXX()");
		}
		virtual void run() {
//			ERandom r;
//			int n = r.nextInt() % 10;
//			for (int i=0; i<n; i++) {
//				EThread::sleep(100);
//			}
			if (id % 10000 == 0)
				LOG(EString::formatOf("thread %d finished", id).c_str());
		}
	};

	EThreadPoolExecutor* executor = new EThreadPoolExecutor(100, 200, 10,
			ETimeUnit::SECONDS, new ELinkedBlockingQueue<ERunnable>());
	executor->allowCoreThreadTimeOut(true);
	for (int i = 0; i < 200000000; i++) {
		executor->execute(new XXX(i));
	}
//	for (int i = 0; i < 20; i++) {
//		EThread::sleep(200);
//		executor->execute(new XXX());
//	}
	executor->shutdown();
	executor->awaitTermination();

//	EThread::sleep(500);
//	printf("thread=%d, sleep wakeup...\n", EThread::currentThread()->getId());

	delete executor;

//	EThread::sleep(500);

	LOG("end of test_threadPoolExecutor.\n");
}

static void test_file_read_write(const char* name)
{
	EFile file(name);

	class Writer : public ERunnable {
	private:
		EFile* file;
	public:
		Writer(EFile* f) : file(f) {
		}
		virtual void run() {
			EFileOutputStream* fos;
			try {
				fos = new EFileOutputStream(file);
//				fos->setIOBuffered(true);
//				fos->setIOBuffered(false);

				for (int i=0; i<10; i++) {
					fos->write(EString(i).c_str());
					fos->write("\n");
//					fos->flush();

					EThread::sleep(1000);
				}

				fos->close();
				delete fos;
			} catch (EFileNotFoundException& e) {
				e.printStackTrace();
			} catch (EIOException& e) {
				e.printStackTrace();
			} catch (EInterruptedException& e) {
				e.printStackTrace();
			}
		}
	};
	ERunnable* wr = new Writer(&file);
	EThread* writer = new EThread(wr);

	class Reader : public ERunnable {
	private:
		EFile* file;
	public:
		Reader(EFile* f) : file(f) {
		}
		virtual void run() {
			EFileInputStream* fis;
			try {
				fis = new EFileInputStream(file);

				if (0) {
					for (int i=0; i<100; i++) {
						char b[32] = {0};
						int n = fis->read(b, sizeof(b));
						if (n > 0) LOG(b);

						EThread::sleep(100);
					}
				}

				if (1) {
					for (int i=0; i<6; i++) {
						EThread::sleep(2000);
						int avaliable = fis->available();
						LOG("avaliable=%d", avaliable);
						fis->skip(2);
						char b[32] = {0};
						int n = fis->read(b, sizeof(b));
						if (n > 0) LOG(b);
					}
				}

				fis->close();
				delete fis;
			} catch (EFileNotFoundException& e) {
				e.printStackTrace();
			} catch (EIOException& e) {
				e.printStackTrace();
			} catch (EInterruptedException& e) {
				e.printStackTrace();
			}
		}
	};
	EThread* reader = new EThread(new Reader(&file));

	writer->start();
	reader->start();

	writer->join();
	reader->join();

	delete writer;
	delete reader;
}

static void test_buffered_stream()
{
	EFile file("/tmp/1.txt");

	EFileOutputStream fos(&file);
	fos.write("11111111111111111111111111111111111111111111111111111111111110");
	fos.close();

	EFileInputStream fis(&file);
	EBufferedInputStream bis(&fis);
	bis.mark(9000);
	byte x[4096];
	int n;
	while ((n = bis.read(x, sizeof(x))) > 0) {
		LOG("%s", x);
	}
}

#define TEST_SYNCHRONOUS_QUEUE_MODE 0
#define TEST_SYNCHRONOUS_QUEUE_MULTITHREAD 1

static void test_synchronousQueue()
{
	class Product : public EThread {
	private:
		ESynchronousQueue<EInteger>* queue;
	public:
		Product(ESynchronousQueue<EInteger>* queue) {
			this->queue = queue;
		}

		virtual void run() {
			long l = 0;
			while (true) {
				ERandom r;
				int rand = r.nextInt(1000);

				boolean ret = false;
				EInteger* ni = new EInteger(rand);
#if (TEST_SYNCHRONOUS_QUEUE_MODE == 0 || TEST_SYNCHRONOUS_QUEUE_MODE == 1)
//				try {
//					ETimeUnit::MILLISECONDS->sleep(1);
//				} catch (EInterruptedException& e) {
//					e.printStackTrace();
//				}
				ret = queue->offer(ni);
#elif (TEST_SYNCHRONOUS_QUEUE_MODE == 2)
				ret = queue->offer(new EInteger(rand), 50, ETimeUnit::MILLISECONDS);
#endif
				if (!ret) delete ni;
//				LOG(".........................");

#if (TEST_SYNCHRONOUS_QUEUE_MODE == 1)
				if (l++ > 100) break;
#endif
			}
		}
	};

	class Customer : public EThread {
	private:
		ESynchronousQueue<EInteger>* queue;
	public:
		Customer(ESynchronousQueue<EInteger>* queue) {
			this->queue = queue;
		}

		virtual void run() {
			while (true) {
				try {
#if (TEST_SYNCHRONOUS_QUEUE_MODE == 0 || TEST_SYNCHRONOUS_QUEUE_MODE == 2)
					int n = queue->take()->intValue();
//					LOG("n=%d", n);
#endif

#if (TEST_SYNCHRONOUS_QUEUE_MODE == 1)
					sp<EInteger> i = queue->poll(50, ETimeUnit::MILLISECONDS);
					if (i != null) {
						LOG("consumer:%d", i->intValue());
					}
					else {
						break;
					}
#endif
				} catch (EInterruptedException& e) {
					e.printStackTrace();
				}
			}
		}
	};

	ESynchronousQueue<EInteger> sq;
//	ESynchronousQueue<EInteger> sq(true);

	Customer* c = new Customer(&sq);
	c->start();
	Product* p = new Product(&sq);
	p->start();
#if TEST_SYNCHRONOUS_QUEUE_MULTITHREAD
	Customer* c1 = new Customer(&sq);
	c1->start();
//	Product* p1 = new Product(&sq);
//	p1->start();
#endif

	c->join();
	p->join();

#if TEST_SYNCHRONOUS_QUEUE_MULTITHREAD
	c1->join();
//	p1->join();

	delete c1;
//	delete p1;
#endif
	delete c;
	delete p;

	LOG("------------------------------------------");
}

class Worker : public ERunnable {
private:
	int idx;
public:
	virtual ~Worker() {
//			LOG("~Worker()");
	}
	Worker(int i) : idx(i) {}

	virtual void run() {
		LOG("worker[%d] run...", idx);
	}
};

class XCallable : public ECallable<EInteger> {
private:
	int idx;
public:
	XCallable() : idx(-1) {}
	XCallable(int i) : idx(i) {}

	virtual ~XCallable() {
//		LOG("~XCallable()");
	}

	virtual sp<EInteger> call() {
		if (idx == -1) {
			ERandom r;
			return new EInteger(r.nextInt(100));
		}
		else {
			return new EInteger(idx);
		}
//		throw EInterruptedException(__FILE__, __LINE__);
	}
};

static void test_executors()
{
	EExecutorService* executorService;

	//=============================================================

	LOG("test newFixedThreadPool(n)");

	executorService = EExecutors::newFixedThreadPool(10);

	for (int i=0; i<10; i++) {
		executorService->execute(new Worker(i));
	}
	executorService->shutdown();
	executorService->awaitTermination();

	delete executorService;

	LOG("===========================");

	//=============================================================

	LOG("test newFixedThreadPool(n, ThreadFactory)");

	executorService = EExecutors::newFixedThreadPool(10, EExecutors::defaultThreadFactory());

	for (int i=0; i<10; i++) {
		executorService->execute(new Worker(i));
	}
	executorService->shutdown();
	executorService->awaitTermination();

	delete executorService;

	LOG("===========================");

	//=============================================================

	LOG("test newSingleThreadExecutor()");

	executorService = EExecutors::newSingleThreadExecutor();

	for (int i=0; i<10; i++) {
		executorService->execute(new Worker(i));
	}
	executorService->shutdown();
	executorService->awaitTermination();

	delete executorService;

	LOG("===========================");

	//=============================================================

	LOG("test newCachedThreadPool()");

	executorService = EExecutors::newCachedThreadPool();

	for (int i=0; i<10; i++) {
		executorService->execute(new Worker(i));
	}
	executorService->shutdown();
	executorService->awaitTermination();

	delete executorService;

	LOG("===========================");

	//=============================================================

	LOG("test EXecutors::callable()");

	sp<EInteger> result(new EInteger(999));
	sp<ECallable<EInteger> > callable = EExecutors::callable(sp<ERunnable>(new Worker(0)), result);
	sp<EFutureTask<EInteger> > future = new EFutureTask<EInteger>(callable);
	EThread* thread = new EThread(future);
	thread->start();
	try {
		EThread::sleep(50);//
		LOG("i=%d", future->get()->intValue());
	} catch (EInterruptedException& e) {
		e.printStackTrace();
	} catch (EExecutionException& e) {
		e.printStackTrace();
	}
	delete thread;

	LOG("===========================");

	//=============================================================

}

static void test_callable_and_future()
{
	EExecutorService* executorService;
	sp<EFuture<EInteger> > future;

	//=============================================================

	sp<XCallable> callable = new XCallable();
	sp<EFutureTask<EInteger> > futureTask = new EFutureTask<EInteger>(callable);
	EThread* thread = new EThread(futureTask);
	thread->start();
	try {
		EThread::sleep(50);//
		while (!futureTask->isDone()){}
		LOG("i=%d", futureTask->get()->intValue());
	} catch (EInterruptedException& e) {
		e.printStackTrace();
	} catch (EExecutionException& e) {
		e.printStackTrace();
	}
	delete thread;

	LOG("===========================");

	//=============================================================

	executorService = EExecutors::newSingleThreadExecutor();
	callable = new XCallable();
	future = executorService->submit(dynamic_pointer_cast<ECallable<EInteger> >(callable));
	try {
		EThread::sleep(50);
		while (!future->isDone()){}
		LOG("i=%d", future->get()->intValue());
	} catch (EInterruptedException& e) {
		e.printStackTrace();
	} catch (EExecutionException& e) {
		e.printStackTrace();
	}
	delete executorService;

	LOG("===========================");

	//=============================================================

	executorService = EExecutors::newSingleThreadExecutor();
	sp<EInteger> result(new EInteger(888));
	future = executorService->submit(new Worker(111), result);
	try {
		EThread::sleep(50);
		while (!future->isDone()){}
		LOG("i=%d", future->get()->intValue());
	} catch (EInterruptedException& e) {
		e.printStackTrace();
	} catch (EExecutionException& e) {
		e.printStackTrace();
	}
	delete executorService;

	LOG("===========================");

	executorService = EExecutors::newCachedThreadPool();

	EArrayList<sp<ECallable<EInteger> > >* callers = new EArrayList<sp<ECallable<EInteger> > >();

	for (int i = 0; i < 20; i++) {
		callers->add(new XCallable());
	}

//	eal<EFuture<EInteger> > futures = executorService->invokeAll(callers);
	EArrayList<sp<EFuture<EInteger> > > futures = executorService->invokeAll(callers, 1, ETimeUnit::MILLISECONDS);
	executorService->shutdown();

	for (int i = 0; i < futures.size(); i++) {
		try {
			sp<EFuture<EInteger> > future = futures.getAt(i);
			LOG("i=%d", future->get()->intValue());
		} catch (ECancellationException& e) {
			e.printStackTrace();
		}
	}

	delete callers;
	delete executorService;

	LOG("===========================");

	executorService = EExecutors::newCachedThreadPool();
//	ECompletionService<EInteger> *cs = new EExecutorCompletionService<EInteger>(executorService);
	ECompletionService<EInteger> *cs = new EExecutorCompletionService<EInteger>(executorService, new ELinkedBlockingQueue<EFuture<EInteger> >());
	EArrayList<sp<XCallable> > al;

	for(int i = 1; i < 5; i++) {
		sp<XCallable> task(new XCallable());
		al.add(task);
		cs->submit(task);
	}
	// ...
	for(int i = 1; i < 5; i++) {
		try {
			//LOG("i=%d", cs->take()->get()->intValue());
			sp<EFuture<EInteger> > f = cs->poll();
			if (f != null) {
				LOG("i=%d", f->get()->intValue());
			}
		} catch (EInterruptedException& e) {
			e.printStackTrace();
		} catch (EExecutionException& e) {
			e.printStackTrace();
		}
	}

	delete executorService;
	delete cs;

	LOG("===========================");

	callers = new EArrayList<sp<ECallable<EInteger> > >();
	executorService = EExecutors::newCachedThreadPool();

	for (int i = 0; i < 20; i++) {
		callers->add(new XCallable(i));
	}

	sp<EInteger> result2;
//	result2 = executorService->invokeAny(callers);
	result2 = executorService->invokeAny(callers, 1, ETimeUnit::MILLISECONDS);
	executorService->shutdown();

	LOG("i=%d", result2->intValue());

	delete executorService;
	delete callers;
}

static void test_threadLocalRandom() {
	EThreadLocalRandom tlr;

	class XXX : public EThread {
	private:
		EThreadLocalRandom* tlr;
	public:
		XXX(EThreadLocalRandom* tlr) {
			this->tlr = tlr;
		}
		virtual void run() {
			LOG("random n=%d", tlr->nextInt(22));
		}
	};

	XXX x1(&tlr);
	XXX x2(&tlr);
	XXX x3(&tlr);

	x1.start();
	x2.start();
	x3.start();

	x1.join();
	x2.join();
	x3.join();
}

static void test_uri() {
//	EString src("http://java2s.com/");
//	EString src("https://[fe80::20e:c6ff:fef5:7276]/%F0%9F%8D%A3%F0%9F%8D%BA");
//	EString src("https://url.spec.whatwg.org/%F0%9F%8D%A3%F0%9F%8D%BA");
	EString src("foo://username:password@example.com:8042/over/there/index.dtb?type=animal&name=narwhal#nose");
//	EString src("eXAMPLE://a/./b/../b/%63/%7bfoo%7d");
//	EString src("example://a/b/c/%7Bfoo%7D");
//	EString src("http://a/b/c/g;x?y#s");
//	EString src("http://a/b/c/d;p?q#s");
//	EString src("urn:oasis:names:specification:docbook:dtd:xml:4.1.2");
//	EString src("telnet://192.0.2.16:80/");
//	EString src("tel:+1-816-555-1212");
//	EString src("news:comp.infosystems.www.servers.unix");
//	EString src("ldap://[2001:db8::7]/c=GB?objectClass?one");
//	EString src("file:///~/calendar");
//	EString src("../../../demo/../jfc/SwingSet2/../../src/SwingSet2.java");
//	EString src("../../../demo/jfc/SwingSet2/src/SwingSet2.java");
//	EString src("docs/guide/collections/designfaq.html#28");
//	EString src("urn:isbn:096139210x");
//	EString src("mailto:java-net@java.sun.com");
//	EString src("http://java.sun.com/j2se/1.3/");

	EURI uri(src.c_str());

	LOG("src=%s\n", src.c_str());

	LOG("scheme=%s", uri.getScheme());
	LOG("raw schemeSpecificPart=%s", uri.getRawSchemeSpecificPart());
	LOG("fragment=%s", uri.getFragment());
	LOG("raw fragment=%s", uri.getRawFragment());
	LOG("authority=%s", uri.getAuthority());
	LOG("raw authority=%s", uri.getRawAuthority());
	LOG("userInfo=%s", uri.getUserInfo());
	LOG("raw userInfo=%s", uri.getRawUserInfo());
	LOG("host=%s", uri.getHost());
	LOG("port=%d", uri.getPort());
	LOG("path=%s", uri.getPath());
	LOG("raw path=%s", uri.getRawPath());
	LOG("query=%s", uri.getQuery());
	LOG("raw query=%s", uri.getRawQuery());

	LOG("string=%s", uri.toString().c_str());
	LOG("ascii string=%s", uri.toASCIIString().c_str());

	// normalize

	sp<EURI> uri2 = uri.normalize();

	LOG("\n\nscheme=%s", uri2->getScheme());
	LOG("raw schemeSpecificPart=%s", uri2->getRawSchemeSpecificPart());
	LOG("fragment=%s", uri2->getFragment());
	LOG("raw fragment=%s", uri2->getRawFragment());
	LOG("authority=%s", uri2->getAuthority());
	LOG("raw authority=%s", uri2->getRawAuthority());
	LOG("userInfo=%s", uri2->getUserInfo());
	LOG("raw userInfo=%s", uri2->getRawUserInfo());
	LOG("host=%s", uri2->getHost());
	LOG("port=%d", uri2->getPort());
	LOG("path=%s", uri2->getPath());
	LOG("raw path=%s", uri2->getRawPath());
	LOG("query=%s", uri2->getQuery());
	LOG("raw query=%s", uri2->getRawQuery());

	LOG("string=%s", uri2->toString().c_str());
	LOG("ascii string=%s", uri2->toASCIIString().c_str());

	if (uri2->equals(&uri)) {
		LOG("2==1");
	}

	// relativize(URI uri)

	EURI rel("http://java2s.com/index.htm?key1=v1&key2=v2&key3=&key4=%E4%B8%AD%E6%96%87&key5&key6=");
	sp<EURI> uri3(uri.relativize(&rel));

	LOG("\n\nscheme=%s", uri3->getScheme());
	LOG("raw schemeSpecificPart=%s", uri3->getRawSchemeSpecificPart());
	LOG("fragment=%s", uri3->getFragment());
	LOG("raw fragment=%s", uri3->getRawFragment());
	LOG("authority=%s", uri3->getAuthority());
	LOG("raw authority=%s", uri3->getRawAuthority());
	LOG("userInfo=%s", uri3->getUserInfo());
	LOG("raw userInfo=%s", uri3->getRawUserInfo());
	LOG("host=%s", uri3->getHost());
	LOG("port=%d", uri3->getPort());
	LOG("path=%s", uri3->getPath());
	LOG("raw path=%s", uri3->getRawPath());
	LOG("query=%s", uri3->getQuery());
	LOG("raw query=%s", uri3->getRawQuery());

	LOG("string=%s", uri3->toString().c_str());
	LOG("ascii string=%s", uri3->toASCIIString().c_str());

	sp<EMap<EString*, EString*> > paramsMap = uri3->getParameterMap();
	sp<EIterator<EMapEntry<EString*, EString*>*> > iter = paramsMap->entrySet()->iterator();
	while (iter->hasNext()) {
		EMapEntry<EString*, EString*>* me = iter->next();
		LOG("==> %s=%s", me->getKey()->c_str(), me->getValue()->c_str());
	}
	LOG("key4=%s", uri3->getParameter("key4").c_str());
}

static void test_networkInferface(void) {
	sp<EEnumeration<ENetworkInterface*> > e = ENetworkInterface::getNetworkInterfaces();
	for (; e != null && e->hasMoreElements();) {
		ENetworkInterface* ni = e->nextElement();
		LOG("\r\nni_=%s", ni->toString().c_str());

		LOG("isLoopback=%d", ni->isLoopback());
		LOG("isPointToPoint=%d", ni->isPointToPoint());
		LOG("isUp=%d", ni->isUp());
		LOG("isVirtual=%d", ni->isVirtual());
		LOG("supportsMulticast=%d", ni->supportsMulticast());
		LOG("index=%d", ni->getIndex());
		LOG("mtu=%d", ni->getMTU());

		sp<EList<EInterfaceAddress*> >ifas = ni->getInterfaceAddresses();
		sp<EIterator<EInterfaceAddress*> > iter = ifas->iterator();
		while (iter->hasNext()) {
			EInterfaceAddress* ifa = iter->next();
			EInetAddress* ia = ifa->getAddress();
			if (ia) {
				LOG("ip=%s", ia->toString().c_str());
			}
		}

		sp<EList<EInterfaceAddress*> > ifs =
				ni->getInterfaceAddresses();
		if (ifs != null) {
			sp<EIterator<EInterfaceAddress*> > iter = ifs->iterator();
			while (iter->hasNext()) {
				EInterfaceAddress* ia = iter->next();
				LOG("ia: %s", ia->toString().c_str());
			}
		}

		llong mac = ni->getHardwareAddress();
		ubyte* bmac = MAC6_LLONG2ARR(mac);
		for (int i = 0; i < MAC6_LEN; i++) {
			LOG("MAC[%d]=%x", i, bmac[i]);
		}
	}

	sp<ENetworkInterface> ni = ENetworkInterface::getByIndex(4);
	if (ni != null) LOG("\r\nni=%s", ni->toString().c_str());

	//	EInetAddress addr = EInetAddress::getByName("127.0.0.1");
	EInetAddress addr = EInetAddress::getByName("localhost");
	ni = ENetworkInterface::getByInetAddress(&addr);
	LOG("\r\nni=%s", ni->toString().c_str());
}

static void test_datagramSocket(void) {
	class Server: public EThread {
	public:
		virtual void run() {
			while (1) {
			try {
				EDatagramSocket socket(9001);
				socket.setReuseAddress(true);
				EA<byte> buf(1024);
				EDatagramPacket packet(buf);

				//sets
				socket.setBroadcast(true);

				//recv
				socket.receive(&packet);
				LOG("ip=%s", packet.getAddress()->toString().c_str());
				LOG("port=%d", packet.getPort());
				EA<byte>* data = packet.getData();
				int offset = packet.getOffset();
				int length = packet.getLength();
				LOG("data=%*s", length, (char*)data->address() + offset);

				//echo
				socket.send(&packet);

				//close
				socket.close();
			} catch (ESocketException& se) {
				se.printStackTrace();
			} catch (EIOException& ie) {
				ie.printStackTrace();
			}
			}
		}
	};

	class Client: public EThread {
	public:
		virtual void run() {
			while (1) {
			try {
				EDatagramSocket socket(9002);
				EA<byte> buf(1024);
				memcpy(buf.address(), "1234567890", 10);
				EInetAddress ia = EInetAddress::getByName("127.0.0.1");
				EDatagramPacket packet(buf, 10, &ia, 9001);

				//connect
				socket.connect(&ia, 9001);
				sp<EInetSocketAddress> isa = socket.getRemoteSocketAddress();
				LOG("ip=%s, port=%d", isa->getHostName(), isa->getPort());

				//send
				socket.send(&packet);

				if (0) {
				//recv
				socket.receive(&packet);
				LOG("ip=%s", packet.getAddress()->toString().c_str());
				LOG("port=%d", packet.getPort());
				EA<byte>* data = packet.getData();
				LOG("data=%s", (char*)data->address());
				}

				//disconnect
				socket.disconnect();

				//close
				socket.close();
			} catch (ESocketException& se) {
				se.printStackTrace();
			} catch (EIOException& ie) {
				ie.printStackTrace();
			}
			}
		}
	};

	Server server;
	Client client;

	server.start();

	EThread::sleep(500);

	client.start();

	server.join();
	client.join();
}

static void test_multicastSocket(void) {
	class Receiver: public EThread {
	public:
		virtual void run() {
			try {
				EMulticastSocket socket(9000);
				EInetAddress ia = EInetAddress::getByName("230.0.0.1");

				socket.joinGroup(&ia);
				socket.setLoopbackMode(false);

				EA<byte> buf(1024);
				EDatagramPacket packet(buf);

				while (1) {
					//recv
					socket.receive(&packet);
					LOG("ip=%s", packet.getAddress()->toString().c_str());
					LOG("port=%d", packet.getPort());
					EA<byte>* data = packet.getData();
					int offset = packet.getOffset();
					int length = packet.getLength();
					LOG("data=%*s", length, (char*)data->address() + offset);
				}

				//close
				socket.close();
			} catch (ESocketException& se) {
				se.printStackTrace();
			} catch (EIOException& ie) {
				ie.printStackTrace();
			}
		}
	};

	class Sender: public EThread {
	public:
		virtual void run() {
			try {
				EMulticastSocket socket(9000);
				EInetAddress ia = EInetAddress::getByName("230.0.0.1");

				socket.joinGroup(&ia);
				socket.setLoopbackMode(false);

				EA<byte> buf(1024);
				memcpy(buf.address(), "1234567890", 10);
				EDatagramPacket packet(buf, 10, &ia, 9000);

				while (1) {
					//send
					socket.send(&packet);

					EThread::sleep(1); //?
				}

				//close
				socket.close();
			} catch (ESocketException& se) {
				se.printStackTrace();
			} catch (EIOException& ie) {
				ie.printStackTrace();
			}
		}
	};

	Receiver receiver;
	Sender sender;

//	receiver.start();

	EThread::sleep(500);

	sender.start();

	receiver.join();
	sender.join();
}

static void* execute_c_thread(es_thread_t* t) {
	try {
		EThread* thread = EThread::currentThread();
		LOG("current is %s thread, name: %s", thread->isMainThread() ? "main" : "sub", thread->getName());
		LOG("current is %s thread, name: %s", thread->isCThread() ? "c" : "c++", thread->getName());

		for (int i=0; i<100; i++) {
			test_executors();
		}
	} catch (EThrowable& t) {
		LOG(t.getStackTrace());
	}

	return NULL;
}

static void test_c_thread() {
	EThread* thread = EThread::currentThread();
	LOG("current is %s thread, name: %s", thread->isMainThread() ? "main" : "sub", thread->getName());
	LOG("current is %s thread, name: %s", thread->isCThread() ? "c" : "c++", thread->getName());

	int size = 10;
	es_array_t* arr = eso_ptrarray_make(size);

	for (int i=0; i<size; i++) {
		es_thread_t* thread = eso_thread_create(NULL, execute_c_thread, NULL);
		eso_ptrarray_push(arr, thread);
	}

	for (int i=0; i<size; i++) {
		es_thread_t* thread = (es_thread_t*)eso_ptrarray_get(arr, i);

		eso_thread_join(NULL, thread);
		eso_thread_destroy(&thread);
	}

	eso_ptrarray_free(&arr);
}

static void test_biginteger() {
	EBigInteger bi1("59435345123452345458998928392839247844353457");
	EBigInteger bi2("-8928392839247844353457223888888");
	EBigInteger bi3("0");
	EBigInteger bi4("646534");
	EBigInteger bi5("59435345123452345458998928392839247844353457");
	EBigInteger bi6("0");

	LOG("bi1 = %s", bi1.toString().c_str());

	EBigInteger bi;

	bi = bi1 - bi2;
	LOG("- %s", bi.toString().c_str());

	if (bi2 > bi1) {
		LOG("bi2 > bi1");
	}

	bi = bi2 - bi1;
	LOG("- %s", bi.toString().c_str());

	bi = -bi3;
	LOG("- %s", bi.toString().c_str());

	bi = bi2 * bi1;
	LOG("* %s", bi.toString().c_str());
	bi = bi1 * -1000;
	LOG("* %s", bi.toString().c_str());
	bi = EBigInteger(0LL) * -1;
	LOG("* %s", bi.toString().c_str());
	bi = bi1 * -1;
	LOG("* %s", bi.toString().c_str());

	LOG("bi1=%s", bi1.toString().c_str());
	LOG("bi2=%s", bi2.toString().c_str());
	bi = bi1 / bi2;
	LOG("/ %s", bi.toString().c_str());

	bi = bi1 % bi2;
	LOG("%% %s", bi.toString().c_str());

	EA<EBigInteger*> dr = bi1.divideAndRemainder(bi2);
	LOG("d=%s, r=%s", dr[0]->toString().c_str(), dr[1]->toString().c_str());

	LOG("bi4 intValue=%d", bi4.intValue());

	LOG("bi1 ? bi2: %d", bi3.compareTo(&bi6));

	LOG("abs %s", bi1.abs().toString().c_str());

	LOG("pow10(3) %s", bi1.pow10(3).toString().c_str());
	LOG("pow10(-3) %s", bi1.pow10(-3).toString().c_str());

	LOG("hashcode=%d", bi1.hashCode());
}

static void test_bigdecimal() {
	EBigDecimal d1("+9999343343.2004384010000");
	EBigDecimal d2("9999343343.9004384340000");
	EBigDecimal d3("0.900438");
	EBigDecimal d4("0.0043343000");

	EBigDecimal d;

	LOG("d1=%s", d1.toString().c_str());
	LOG("d2=%s", d2.toString().c_str());

	d = d1 + d2;
	LOG("d1+d2 = %s", d.toString().c_str());

	d = d1 - d2;
	LOG("d1-d2 = %s", d.toString().c_str());

	d = d2 - d1;
	LOG("d2-d1 = %s", d.toString().c_str());

	d = d2 * d1;
	LOG("d2*d1 = %s", d.toString().c_str());

	d = d3 * d4;
	LOG("d3*d4 = %s", d.toString().c_str());

	d = d2 / d1;
	LOG("d2/d1 = %s", d.toString().c_str());

	d = d3 / d4;
	LOG("d3/d4 = %s", d.toString().c_str());

	d = d1 / 8888;
	LOG("d1/8888 = %s", d.toString().c_str());

	d = d2.divide(d1, 3);
	LOG("d2.divide(d1) = %s", d.toString().c_str());

	d = d3.divide(d4, 3);
	LOG("d3.divide(d4) = %s", d.toString().c_str());

	d = d1.divide(-8888, 3);
	LOG("d1.divide(-8888) = %s", d.toString().c_str());

	LOG("d1 toBigInteger=%s", d1.toBigInteger().toString().c_str());
	LOG("d1 llongValue=%lld", d1.llongValue());
	LOG("d1 doubleValue=%lf", d1.doubleValue());

	d = d3 % d4;
	LOG("d3%%d4 = %s", d.toString().c_str());

	d = d1 % 88779;
	LOG("d1%%88779 = %s", d.toString().c_str());

	EA<EBigDecimal*> r2 = d3.divideAndRemainder(d4);
	LOG("d3%%d4 div=%s, rem=%s", r2[0]->toString().c_str(), r2[1]->toString().c_str());

	//round test
	{
		//ROUND_UP
		d = EBigDecimal("5.5").divide(1, 0, EBigDecimal::ROUND_UP);
		LOG("round_up(5.5): %s", d.toString().c_str());

		d = EBigDecimal("2.5").divide(1, 0, EBigDecimal::ROUND_UP);
		LOG("round_up(2.5): %s", d.toString().c_str());

		d = EBigDecimal("1.6").divide(1, 0, EBigDecimal::ROUND_UP);
		LOG("round_up(1.6): %s", d.toString().c_str());

		d = EBigDecimal("1.1").divide(1, 0, EBigDecimal::ROUND_UP);
		LOG("round_up(1.1): %s", d.toString().c_str());

		d = EBigDecimal("1.0").divide(1, 0, EBigDecimal::ROUND_UP);
		LOG("round_up(1.0): %s", d.toString().c_str());

		d = EBigDecimal("-1.0").divide(1, 0, EBigDecimal::ROUND_UP);
		LOG("round_up(-1.0): %s", d.toString().c_str());

		d = EBigDecimal("-1.1").divide(1, 0, EBigDecimal::ROUND_UP);
		LOG("round_up(-1.1): %s", d.toString().c_str());

		d = EBigDecimal("-1.6").divide(1, 0, EBigDecimal::ROUND_UP);
		LOG("round_up(-1.6): %s", d.toString().c_str());

		d = EBigDecimal("-2.5").divide(1, 0, EBigDecimal::ROUND_UP);
		LOG("round_up(-2.5): %s", d.toString().c_str());

		d = EBigDecimal("-5.5").divide(1, 0, EBigDecimal::ROUND_UP);
		LOG("round_up(-5.5): %s", d.toString().c_str());

		//ROUND_DOWN
		d = EBigDecimal("5.5").divide(1, 0, EBigDecimal::ROUND_DOWN);
		LOG("round_down(5.5): %s", d.toString().c_str());

		d = EBigDecimal("2.5").divide(1, 0, EBigDecimal::ROUND_DOWN);
		LOG("round_down(2.5): %s", d.toString().c_str());

		d = EBigDecimal("1.6").divide(1, 0, EBigDecimal::ROUND_DOWN);
		LOG("round_down(1.6): %s", d.toString().c_str());

		d = EBigDecimal("1.1").divide(1, 0, EBigDecimal::ROUND_DOWN);
		LOG("round_down(1.1): %s", d.toString().c_str());

		d = EBigDecimal("1.0").divide(1, 0, EBigDecimal::ROUND_DOWN);
		LOG("round_down(1.0): %s", d.toString().c_str());

		d = EBigDecimal("-1.0").divide(1, 0, EBigDecimal::ROUND_DOWN);
		LOG("round_down(-1.0): %s", d.toString().c_str());

		d = EBigDecimal("-1.1").divide(1, 0, EBigDecimal::ROUND_DOWN);
		LOG("round_down(-1.1): %s", d.toString().c_str());

		d = EBigDecimal("-1.6").divide(1, 0, EBigDecimal::ROUND_DOWN);
		LOG("round_down(-1.6): %s", d.toString().c_str());

		d = EBigDecimal("-2.5").divide(1, 0, EBigDecimal::ROUND_DOWN);
		LOG("round_down(-2.5): %s", d.toString().c_str());

		d = EBigDecimal("-5.5").divide(1, 0, EBigDecimal::ROUND_DOWN);
		LOG("round_down(-5.5): %s", d.toString().c_str());

		//ROUND_CEILING
		d = EBigDecimal("5.5").divide(1, 0, EBigDecimal::ROUND_CEILING);
		LOG("round_ceiling(5.5): %s", d.toString().c_str());

		d = EBigDecimal("2.5").divide(1, 0, EBigDecimal::ROUND_CEILING);
		LOG("round_ceiling(2.5): %s", d.toString().c_str());

		d = EBigDecimal("1.6").divide(1, 0, EBigDecimal::ROUND_CEILING);
		LOG("round_ceiling(1.6): %s", d.toString().c_str());

		d = EBigDecimal("1.1").divide(1, 0, EBigDecimal::ROUND_CEILING);
		LOG("round_ceiling(1.1): %s", d.toString().c_str());

		d = EBigDecimal("1.0").divide(1, 0, EBigDecimal::ROUND_CEILING);
		LOG("round_ceiling(1.0): %s", d.toString().c_str());

		d = EBigDecimal("-1.0").divide(1, 0, EBigDecimal::ROUND_CEILING);
		LOG("round_ceiling(-1.0): %s", d.toString().c_str());

		d = EBigDecimal("-1.1").divide(1, 0, EBigDecimal::ROUND_CEILING);
		LOG("round_ceiling(-1.1): %s", d.toString().c_str());

		d = EBigDecimal("-1.6").divide(1, 0, EBigDecimal::ROUND_CEILING);
		LOG("round_ceiling(-1.6): %s", d.toString().c_str());

		d = EBigDecimal("-2.5").divide(1, 0, EBigDecimal::ROUND_CEILING);
		LOG("round_ceiling(-2.5): %s", d.toString().c_str());

		d = EBigDecimal("-5.5").divide(1, 0, EBigDecimal::ROUND_CEILING);
		LOG("round_ceiling(-5.5): %s", d.toString().c_str());

		//ROUND_FLOOR
		d = EBigDecimal("5.5").divide(1, 0, EBigDecimal::ROUND_FLOOR);
		LOG("round_floor(5.5): %s", d.toString().c_str());

		d = EBigDecimal("2.5").divide(1, 0, EBigDecimal::ROUND_FLOOR);
		LOG("round_floor(2.5): %s", d.toString().c_str());

		d = EBigDecimal("1.6").divide(1, 0, EBigDecimal::ROUND_FLOOR);
		LOG("round_floor(1.6): %s", d.toString().c_str());

		d = EBigDecimal("1.1").divide(1, 0, EBigDecimal::ROUND_FLOOR);
		LOG("round_floor(1.1): %s", d.toString().c_str());

		d = EBigDecimal("1.0").divide(1, 0, EBigDecimal::ROUND_FLOOR);
		LOG("round_floor(1.0): %s", d.toString().c_str());

		d = EBigDecimal("-1.0").divide(1, 0, EBigDecimal::ROUND_FLOOR);
		LOG("round_floor(-1.0): %s", d.toString().c_str());

		d = EBigDecimal("-1.1").divide(1, 0, EBigDecimal::ROUND_FLOOR);
		LOG("round_floor(-1.1): %s", d.toString().c_str());

		d = EBigDecimal("-1.6").divide(1, 0, EBigDecimal::ROUND_FLOOR);
		LOG("round_floor(-1.6): %s", d.toString().c_str());

		d = EBigDecimal("-2.5").divide(1, 0, EBigDecimal::ROUND_FLOOR);
		LOG("round_floor(-2.5): %s", d.toString().c_str());

		d = EBigDecimal("-5.5").divide(1, 0, EBigDecimal::ROUND_FLOOR);
		LOG("round_floor(-5.5): %s", d.toString().c_str());

		//ROUND_HALF_UP
		d = EBigDecimal("5.5").divide(1, 0, EBigDecimal::ROUND_HALF_UP);
		LOG("round_half_up(5.5): %s", d.toString().c_str());

		d = EBigDecimal("2.5").divide(1, 0, EBigDecimal::ROUND_HALF_UP);
		LOG("round_half_up(2.5): %s", d.toString().c_str());

		d = EBigDecimal("1.6").divide(1, 0, EBigDecimal::ROUND_HALF_UP);
		LOG("round_half_up(1.6): %s", d.toString().c_str());

		d = EBigDecimal("1.1").divide(1, 0, EBigDecimal::ROUND_HALF_UP);
		LOG("round_half_up(1.1): %s", d.toString().c_str());

		d = EBigDecimal("1.0").divide(1, 0, EBigDecimal::ROUND_HALF_UP);
		LOG("round_half_up(1.0): %s", d.toString().c_str());

		d = EBigDecimal("-1.0").divide(1, 0, EBigDecimal::ROUND_HALF_UP);
		LOG("round_half_up(-1.0): %s", d.toString().c_str());

		d = EBigDecimal("-1.1").divide(1, 0, EBigDecimal::ROUND_HALF_UP);
		LOG("round_half_up(-1.1): %s", d.toString().c_str());

		d = EBigDecimal("-1.6").divide(1, 0, EBigDecimal::ROUND_HALF_UP);
		LOG("round_half_up(-1.6): %s", d.toString().c_str());

		d = EBigDecimal("-2.5").divide(1, 0, EBigDecimal::ROUND_HALF_UP);
		LOG("round_half_up(-2.5): %s", d.toString().c_str());

		d = EBigDecimal("-5.5").divide(1, 0, EBigDecimal::ROUND_HALF_UP);
		LOG("round_half_up(-5.5): %s", d.toString().c_str());

		//ROUND_HALF_DOWN
		d = EBigDecimal("5.5").divide(1, 0, EBigDecimal::ROUND_HALF_DOWN);
		LOG("round_half_down(5.5): %s", d.toString().c_str());

		d = EBigDecimal("2.5").divide(1, 0, EBigDecimal::ROUND_HALF_DOWN);
		LOG("round_half_down(2.5): %s", d.toString().c_str());

		d = EBigDecimal("1.6").divide(1, 0, EBigDecimal::ROUND_HALF_DOWN);
		LOG("round_half_down(1.6): %s", d.toString().c_str());

		d = EBigDecimal("1.1").divide(1, 0, EBigDecimal::ROUND_HALF_DOWN);
		LOG("round_half_down(1.1): %s", d.toString().c_str());

		d = EBigDecimal("1.0").divide(1, 0, EBigDecimal::ROUND_HALF_DOWN);
		LOG("round_half_down(1.0): %s", d.toString().c_str());

		d = EBigDecimal("-1.0").divide(1, 0, EBigDecimal::ROUND_HALF_DOWN);
		LOG("round_half_down(-1.0): %s", d.toString().c_str());

		d = EBigDecimal("-1.1").divide(1, 0, EBigDecimal::ROUND_HALF_DOWN);
		LOG("round_half_down(-1.1): %s", d.toString().c_str());

		d = EBigDecimal("-1.6").divide(1, 0, EBigDecimal::ROUND_HALF_DOWN);
		LOG("round_half_down(-1.6): %s", d.toString().c_str());

		d = EBigDecimal("-2.5").divide(1, 0, EBigDecimal::ROUND_HALF_DOWN);
		LOG("round_half_down(-2.5): %s", d.toString().c_str());

		d = EBigDecimal("-5.5").divide(1, 0, EBigDecimal::ROUND_HALF_DOWN);
		LOG("round_half_down(-5.5): %s", d.toString().c_str());

		//ROUND_HALF_EVEN
		d = EBigDecimal("5.5").divide(1, 0, EBigDecimal::ROUND_HALF_EVEN);
		LOG("round_half_even(5.5): %s", d.toString().c_str());

		d = EBigDecimal("2.5").divide(1, 0, EBigDecimal::ROUND_HALF_EVEN);
		LOG("round_half_even(2.5): %s", d.toString().c_str());

		d = EBigDecimal("1.6").divide(1, 0, EBigDecimal::ROUND_HALF_EVEN);
		LOG("round_half_even(1.6): %s", d.toString().c_str());

		d = EBigDecimal("1.1").divide(1, 0, EBigDecimal::ROUND_HALF_EVEN);
		LOG("round_half_even(1.1): %s", d.toString().c_str());

		d = EBigDecimal("1.0").divide(1, 0, EBigDecimal::ROUND_HALF_EVEN);
		LOG("round_half_even(1.0): %s", d.toString().c_str());

		d = EBigDecimal("-1.0").divide(1, 0, EBigDecimal::ROUND_HALF_EVEN);
		LOG("round_half_even(-1.0): %s", d.toString().c_str());

		d = EBigDecimal("-1.1").divide(1, 0, EBigDecimal::ROUND_HALF_EVEN);
		LOG("round_half_even(-1.1): %s", d.toString().c_str());

		d = EBigDecimal("-1.6").divide(1, 0, EBigDecimal::ROUND_HALF_EVEN);
		LOG("round_half_even(-1.6): %s", d.toString().c_str());

		d = EBigDecimal("-2.5").divide(1, 0, EBigDecimal::ROUND_HALF_EVEN);
		LOG("round_half_even(-2.5): %s", d.toString().c_str());

		d = EBigDecimal("-5.5").divide(1, 0, EBigDecimal::ROUND_HALF_EVEN);
		LOG("round_half_even(-5.5): %s", d.toString().c_str());

		//ROUND_UNNECESSARY
		if (0) {
		try {
			d = EBigDecimal("5.5").divide(1, 0, EBigDecimal::ROUND_UNNECESSARY);
			LOG("round_unnecessary(5.5): %s", d.toString().c_str());
		} catch (EArithmeticException& e) {
			e.printStackTrace();
		}
		try {
			d = EBigDecimal("2.5").divide(1, 0, EBigDecimal::ROUND_UNNECESSARY);
			LOG("round_unnecessary(2.5): %s", d.toString().c_str());
		} catch (EArithmeticException& e) {
			e.printStackTrace();
		}
		try {
			d = EBigDecimal("1.6").divide(1, 0, EBigDecimal::ROUND_UNNECESSARY);
			LOG("round_unnecessary(1.6): %s", d.toString().c_str());
		} catch (EArithmeticException& e) {
			e.printStackTrace();
		}
		try {
			d = EBigDecimal("1.1").divide(1, 0, EBigDecimal::ROUND_UNNECESSARY);
			LOG("round_unnecessary(1.1): %s", d.toString().c_str());
		} catch (EArithmeticException& e) {
			e.printStackTrace();
		}
		try {
			d = EBigDecimal("1.0").divide(1, 0, EBigDecimal::ROUND_UNNECESSARY);
			LOG("round_unnecessary(1.0): %s", d.toString().c_str());
		} catch (EArithmeticException& e) {
			e.printStackTrace();
		}
		try {
			d = EBigDecimal("-1.0").divide(1, 0, EBigDecimal::ROUND_UNNECESSARY);
			LOG("round_unnecessary(-1.0): %s", d.toString().c_str());
		} catch (EArithmeticException& e) {
			e.printStackTrace();
		}
		try {
			d = EBigDecimal("-1.1").divide(1, 0, EBigDecimal::ROUND_UNNECESSARY);
			LOG("round_unnecessary(-1.1): %s", d.toString().c_str());
		} catch (EArithmeticException& e) {
			e.printStackTrace();
		}
		try {
			d = EBigDecimal("-1.6").divide(1, 0, EBigDecimal::ROUND_UNNECESSARY);
			LOG("round_unnecessary(-1.6): %s", d.toString().c_str());
		} catch (EArithmeticException& e) {
			e.printStackTrace();
		}
		try {
			d = EBigDecimal("-2.5").divide(1, 0, EBigDecimal::ROUND_UNNECESSARY);
			LOG("round_unnecessary(-2.5): %s", d.toString().c_str());
		} catch (EArithmeticException& e) {
			e.printStackTrace();
		}
		try {
			d = EBigDecimal("-5.5").divide(1, 0, EBigDecimal::ROUND_UNNECESSARY);
			LOG("round_unnecessary(-5.5): %s", d.toString().c_str());
		} catch (EArithmeticException& e) {
			e.printStackTrace();
		}
		}
	}
}

static void test_pushbackInputStream() {
	EString str = "www.baidu.com";
	EByteArrayInputStream bai((void*)str.c_str(), str.length());
	EPushbackInputStream push(&bai);
	int temp = 0;
	while ((temp = push.read()) != -1) {
		if (temp == '.') {
			push.unread(temp);
			temp = push.read();
			printf("%c", (char) temp);
		} else {
			printf("%c", (char) temp);
		}
	}

	LOG("\nend of test_pushbackInputStream().");
}

static void test_proterties() {
	EProperties defaults;

	defaults.setProperty("a", "11");
	defaults.setProperty("b", "22");
	defaults.setProperty("c", "33");

	EProperties prop(&defaults);

	prop.setProperty("a", "中文");
	prop.setProperty("b", "2");

	sp<EString> v;
	v = prop.getProperty("a");
	if (v != null) {
		LOG("v=%s", v->c_str());
	}
	v = prop.getProperty("c");
	if (v != null) {
		LOG("v=%s", v->c_str());
	}

	LOG("===");

	prop.load("./log4e.properties");

	prop.list(ESystem::out);

	LOG("===");

	sp<ESet<sp<EString> > > keys = prop.propertyNames();
	LOG(keys->toString().c_str());

	LOG("===");

	prop.store("./prop.txt", "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
}

static void test_arrays() {
	{
		EA<long> la(10);
		EArrays::fill(&la, (long)9);
		la[5] = 8;
		EString las = EArrays::toString(&la);
		LOG(las.c_str());

		EArrays::sort(&la);

		ES_ASSERT(EArrays::binarySearch(&la, (long)7) < 0);
		ES_ASSERT(EArrays::binarySearch(&la, (long)8) >= 0);
		ES_ASSERT(EArrays::binarySearch(&la, (long)9) >= 0);
	}

	{
		EA<double> la(10);
		EArrays::fill(&la, 9.98888);
		la[5] = 8.08;
		EString las = EArrays::toString(&la);
		LOG(las.c_str());

		EArrays::sort(&la);

		ES_ASSERT(EArrays::binarySearch(&la, 7) < 0);
		ES_ASSERT(EArrays::binarySearch(&la, 8.08) >= 0);
		ES_ASSERT(EArrays::binarySearch(&la, 9.98888) >= 0);
	}

	{
		EA<EComparable<EString*>*> la(10);
		for (int i=0; i<la.length(); i++) {
			la.setAt(i, new EString(i));
		}
		EString las = EArrays::toString(&la);
		LOG(las.c_str());
	}

}

static void test_arrayBlockingQueue() {
	EArrayBlockingQueue<EInteger> abq(3);

	class PutThread : public EThread {
	private:
		EArrayBlockingQueue<EInteger>& queue;
	public:
		PutThread(EArrayBlockingQueue<EInteger>& abq) : queue(abq) {
		}
		virtual void run() {
			while (true) {
				try {
					int millis = (int)(EMath::random()*10);
					EThread::sleep(millis);
					LOG((EString(EThread::currentThread()->getName()) + " put ready.").c_str());
					queue.put(new EInteger(millis)); //blocked if queue full.
					LOG((EString(EThread::currentThread()->getName()) + " putted, " + "queue size=" + queue.size()).c_str());
				} catch (EInterruptedException& e) {
					e.printStackTrace();
				}
			}
		}
	};

	class GetThread : public EThread {
	private:
		EArrayBlockingQueue<EInteger>& queue;
	public:
		GetThread(EArrayBlockingQueue<EInteger>& abq) : queue(abq) {
		}
		virtual void run() {
			while (true) {
				try {
					EThread::sleep(10);
					LOG((EString(EThread::currentThread()->getName()) + " get ready.").c_str());
					queue.take();
					LOG((EString(EThread::currentThread()->getName()) + " getted, " + "queue size=" + queue.size()).c_str());
				} catch (EInterruptedException& e) {
					e.printStackTrace();
				}

				sp<EIterator<sp<EInteger> > > iter = queue.iterator();
				while (iter->hasNext()) {
					sp<EInteger> i = iter->next();
					LOG("i=%d", i->intValue());
				}
			}
		}
	};

	PutThread pt1(abq);
	PutThread pt2(abq);
	PutThread pt3(abq);
	GetThread gt1(abq);
	GetThread gt2(abq);
	pt1.start();
	pt2.start();
	pt3.start();
	gt1.start();
	gt2.start();
	pt1.join();
	pt2.join();
	pt3.join();
	gt1.join();
	gt2.join();
}

static void test_threadGroup() {
	sp<EThreadGroup> tg = EThread::currentThread()->getThreadGroup();
	EString name = tg->getName();
	LOG("thread group name: %s", name.c_str());

	EThread t1(tg, new Worker(1));
	t1.start();
	EA<EThread*> threads(tg->activeCount());
	int n = tg->enumerate(&threads);
	for (int i=0; i<n; i++) {
		EThread* t = threads[i];
		LOG("thread name=%s", t->getName());
	}
	t1.join();

	class Worker2 : public ERunnable {
	public:
		virtual void run() {
			throw EException(__FILE__, __LINE__);
		}
	};

	class ThreadGroup : public EThreadGroup {
	public:
		ThreadGroup() : EThreadGroup("mygroup") {}
		void uncaughtException(EThread* t, EThrowable* e) {
			LOG("thread %s : %s", t->getName(), e->getMessage());
		}
	};
	sp<ThreadGroup> tg2 = new ThreadGroup();
	EThread t2(tg2, new Worker2());
	t2.start();
	t2.join();
}

static void test_test(int argc, const char** argv) {
//	test_null();
//	test_cmpxchg();
//	test_interface();
//	test_vc6bug();
//	test_delete();
//	test_traits();
//	test_string();
//	test_simpleMap();
//	test_random();
//	test_secureRandom();
//	test_thread();
//	test_thread1();
//	test_thread2();
//	test_thread3();
//	test_thread4();
//	test_thread5();
//	test_threadlocal1();
//	test_threadlocal2();
//	test_threadlocal3();
//	test_threadlocal4();
//	test_threadlocal5();
//	test_threadJoin();
//	test_threadState();
//	test_lock(0);
//	test_lock();
//	test_tryLock();
//	test_lockInterruptibly();
//	test_condition();
//	test_readWriteLock();
//	test_exception();
//	test_number_int();
//	test_number_long();
//	test_filepath();
//	test_config();
//	test_system();
//	test_strToken();
//	test_arraylist();
//	test_arraylist2();
//	test_linkedlist();
//	test_linkedlist2();
//	test_stack();
//	test_arraydeque();
//	test_file();
//	test_class();
//	test_class2();
//	test_hashmap();
//	test_hashset();
//	test_treemap();
//	test_treeset();
//	test_math();
//	test_array();
//	test_array2();
//	test_buffer();
//	test_bits();
//	test_float();
//	test_double();
//	test_filestream();
//	test_bytearraystream();
//	test_randomaccessfile();
//	test_randomaccessfile2();
//	test_pipedstream();
//	test_fork();
//	test_process();
//	test_runtime();
//	test_inetaddress();
//	test_socket();
//	test_serversocket();
//	test_calendar();
//	test_date();
//	test_bitset();
//	test_urlstring();
//	test_datastream();
//	test_gzipstream();
//	test_sequencestream();
//	test_pattern();
//	test_atomic();
//	test_atomic2();
//	test_atomicReference_sp();
//	test_collections();
//	test_instanceof();
//	test_NEWRC();
//	test_sp();
//	test_rc();
//	test_shared_ptr();
//	test_shared_ptr2();
//	test_timeunit();
//	test_copyOnWrite1();
//	test_copyOnWrite2();
//	test_concurrentHashmap();
//	test_concurrentHashmap2();
//	test_concurrent_queue();
//	test_concurrentLiteQueue();
//	test_concurrentIntrusiveDeque();
//	test_mutexLinkedQueue();
//	test_mutexLinkedQueue2();
//	test_concurrentLinkedQueue();
//	test_concurrentLinkedQueue2();
//	test_concurrentSkipListMap();
//	test_concurrentSkipListMap2();
//	test_linkedTransferQueue();
//	test_linkedBlockingQueue();
//	test_identityHashMap();
//	test_vector();
//	test_vector2();
//	test_observable();
//	test_observable2();
//	test_countDownLatch();
//	test_cyclicBarrier();
//	test_semaphore();
//	test_exchanger();
//	test_falseSharing();
//	test_timer();
//	test_priorityQueue();
//	test_bson();
//	test_threadPoolExecutor();
//	test_file_read_write(argc > 1 ? argv[1] : "/tmp/f.out");
//	test_buffered_stream();
//	test_synchronousQueue();
//	test_executors();
//	test_callable_and_future();
//	test_threadLocalRandom();
//	test_uri();
//	test_networkInferface();
//	test_datagramSocket();
//	test_multicastSocket();
	test_c_thread();
//	test_biginteger();
//	test_bigdecimal();
//	test_pushbackInputStream();
//	test_proterties();
//	test_arrays();
//	test_arrayBlockingQueue();
//	test_threadGroup();
//
//	EThread::sleep(3000);
}

MAIN_IMPL(testefc) {
	printf("main()\n");

	ESystem::init(argc, argv);

	LOG("inited.");

//	alogger = eso_alogger_create("/tmp/alog.txt", 1024000000);

	int i = 0;
	try {
		boolean loop = EBoolean::parseBoolean(ESystem::getProgramArgument("loop"));
		do {
			test_test(argc, argv);


//		} while (++i < 5);
		} while (1);
	}
	catch (EException& e) {
//		LOG("e=%s", e.toString().c_str());
		e.printStackTrace();
	}
	catch (...) {
		LOG("catch all...");
	}

	LOG("exit...");

	ESystem::exit(0);

	return 0;
}
