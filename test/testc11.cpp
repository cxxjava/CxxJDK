#include "main.hh"
#include "Efc.hh"
#include "ELog.hh"

#ifdef WIN32
#define LOG ESystem::out->println
#else
//#define LOG(fmt,args...) ESystem::out->println(fmt, ##args)
#define LOG(fmt,...) ESystem::out->println(fmt, ##__VA_ARGS__)
//#define LOG(fmt,...)
#endif

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
	int n = 999;

	EThreadX* tx = EThreadX::execute([&](){
		LOG("execute: n=%d", n);
	});

//	tx->start(); //exception: Already started.

	tx->join();
	delete tx;
}

MAIN_IMPL(testc11) {
	printf("main()\n");

	ESystem::init(argc, argv);

	LOG("inited.");

	do {
		try {
//		test_scopeExit();
//		test_finally();
		test_threadx();
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
