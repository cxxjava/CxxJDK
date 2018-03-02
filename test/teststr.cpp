/*
 * testutils.cpp
 *
 *  Created on: 2017-9-11
 *      Author: cxxjava@163.com
 */
#include "es_main.h"
#include "Efc.hh"
#include <string>
#include <iostream>
#include <cstring>
#include <assert.h>

#define LOG(fmt,...) ESystem::out->printfln(fmt, ##__VA_ARGS__)

static void test_string0() {
	//构造
	{
		EString s("1234567890\0xxx", 14);
		LOG("s=%s", s.c_str());
		assert (s.length() == 14);
	}

	//字符操作
	{
		EString s("1234567890\0xxx", 14);
		char c;
		try {
			c = s.charAt(-1);
			assert(false);
		} catch (EIndexOutOfBoundsException& e) {
			e.printStackTrace();
		}
		c = s.charAt(0);
		assert (c=='1');
		c = s.charAt(9);
		assert (c=='0');
		c = s.charAt(10);
		assert (c=='\0');
		try {
			c = s.charAt(120);
			assert (false);
		} catch (EIndexOutOfBoundsException& e) {
			e.printStackTrace();
		}

		s.concat("0123");
		assert (s.length()==18);
		std::string x1("1234567890\0xxx0123", 18);
		assert (s.equals(x1));

		s.replace('0', '\0');
		assert (s.charAt(9) == '\0');
		assert (s.charAt(14) == '\0');

		s.replace("23", "ZZZZZ");
		std::string x2("1ZZZZZ456789\0\0xxx\0""1ZZZZZ", 24);
		assert (s.endsWith(x2));

		s.replaceFirst("ZZZZ", "888");
		std::string x3("1888Z456789\0\0xxx\0""1ZZZZZ", 23);
		assert (s.endsWith(x3));

		EString ss = s.substring(2);
		std::string x4("88Z456789\0\0xxx\0""1ZZZZZ", 21);
		assert (ss.endsWith(x4));
		ss = s.substring(2, 4);
		assert (ss.endsWith("88"));

		try {
			ss = s.substring(-1);
			assert (false);
		} catch (EIndexOutOfBoundsException& e) {
			e.printStackTrace();
		}
		try {
			ss = s.substring(0, 100);
			assert (false);
		} catch (EIndexOutOfBoundsException& e) {
			e.printStackTrace();
		}

		ss = s.toLowerCase();
        std::string x5("88z456789\0\0xxx\0""1zzzzz", 21);
		assert (ss.endsWith(x5));

		ss = ss.toUpperCase();
        std::string x6("88Z456789\0\0XXX\0""1ZZZZZ", 21);
		assert (ss.endsWith(x6));

		s = "  kk   ";
		s = s.trim();
		assert (s.endsWith("kk"));
	}

	//比较
	{
		EString s1("1234567890\0""xxx", 14);
		EString s2("0123456789\0""aXX", 14);

		if (s1.equals(s2)) {
			assert (false);
		}
		assert (true);

		if (s1.equalsIgnoreCase(s2)) {
			assert (false);
		}
		assert (true);

		s2.reset("0123456789\0""XXX", 14);
		if (s1.equalsIgnoreCase(s2)) {
			assert (true);
		}

		boolean r = false;

		r = s1.startsWith("abc");
		assert (!r);
		r = s2.startsWith("0123");
		assert (r);
		std::string x1("0123456789\0", 11);
		r = s2.startsWith(x1);
		assert (r);
		std::string x2("0123456789\0""XXXXXX", 17);
		r = s2.startsWith(x2);
		assert (!r);

		r = s1.endsWith("abc");
		assert (!r);
		r = s1.endsWith("xxx");
		assert (r);
		std::string x3("0\0xxx", 5);
		r = s1.endsWith(x3);
		assert (r);
		r = s1.startsWith("0", -1);
		assert (!r);
	}

	//搜索
	{
		EString s("1234567890\0""xxx", 14);
		int index = -1;

		std::string x1("\0", 1);
		index = s.indexOf(x1);
		assert (index == 10);
		index = s.indexOf('a');
		assert (index == -1);
		index = s.indexOf('1');
		assert (index == 0);

		index = s.indexOf(x1);
		assert (index > 0);
		index = s.indexOf("a");
		assert (index < 0);
		index = s.indexOf("1");
		assert (index == 0);
		index = s.indexOf("123");
		assert (index == 0);
		index = s.indexOf("xxx");
		assert (index == 11);
		index = s.indexOf("123", 2);
		assert (index == -1);
		index = s.indexOf("456", 2);
		assert (index == 3);

		s.reset("1234567890\0""xxx1234567890", 24);
		index = s.indexOf("123");
		assert (index == 0);

		index = s.lastIndexOf("123");
		assert (index == 14);
		index = s.lastIndexOf("123", 14);
		assert (index == 14);
		index = s.lastIndexOf("aaa", 14);
		assert (index == -1);

		index = s.indexOf('1', -1);
		assert (index == 0);
		index = s.lastIndexOf('1', 100);
		assert (index == 14);
	}

	//其他
	{
		EString s("1234567890\0""xxx", 14);
		assert (!s.isEmpty());
		s = "";
		assert (s.isEmpty());

		assert (s.length() == 0);
	}

	LOG("string test success.");
}

static void test_string1() {
	//构造
	{
		{
			EString s;
			LOG("s=%s", s.c_str());
			assert (s.length() == 0);
		}

		{
			EString s("1234567890");
			LOG("s=%s", s.c_str());
			assert (s.length() == 10);
		}
	}

	//字符操作
	{
		EString s("1234567890");
		char c;
		try {
			c = s.charAt(-1);
			assert(false);
		} catch (EIndexOutOfBoundsException& e) {
			e.printStackTrace();
		}
		c = s.charAt(0);
		assert (c=='1');
		c = s.charAt(9);
		assert (c=='0');
		try {
			c = s.charAt(120);
			assert (false);
		} catch (EIndexOutOfBoundsException& e) {
			e.printStackTrace();
		}

		s.concat("0123");
		assert (s.length()==14);
		assert (s.equals("12345678900123"));

		s.replace('0', 'x');
		assert (s.charAt(9) == 'x');
		assert (s.charAt(10) == 'x');

		s.replace("23", "ZZZZZ");
		assert (s.endsWith("1ZZZZZ456789xx1ZZZZZ"));

		s.replace("1ZZZZZ4", "1888Z4");
		assert (s.endsWith("1888Z456789xx1ZZZZZ"));

		EString ss = s.substring(2);
		assert (ss.endsWith("88Z456789xx1ZZZZZ"));
		ss = s.substring(2, 4);
		assert (ss.endsWith("88"));

		try {
			ss = s.substring(-1);
			assert (false);
		} catch (EIndexOutOfBoundsException& e) {
			e.printStackTrace();
		}
		try {
			ss = s.substring(0, 100);
			assert (false);
		} catch (EIndexOutOfBoundsException& e) {
			e.printStackTrace();
		}

		ss = s.toLowerCase();
		assert (ss.endsWith("88z456789xx1zzzzz"));

		ss = ss.toUpperCase();
		assert (ss.endsWith("88Z456789XX1ZZZZZ"));

		s = "  kk   ";
		s = s.trim();
		assert (s.endsWith("kk"));

		s.fmtcat("%d_%s", 99, "xxx");
		assert (s.equals("kk99_xxx"));

		s.format("012345%d", 6789);
		assert (s.equals("0123456789"));

		s << "abc" << 123 << true << 'x';
		assert (s.equals("0123456789abc123truex"));

		s.erase(10);
		assert (s.equals("0123456789"));
		s.erase(5, 100);
		assert (s.equals("01234"));
		s.erase(4, -1);
		assert (s.equals("0123"));

		try {
			s.eraseCharAt(-1);
			assert (false);
		} catch (EIndexOutOfBoundsException& e) {
			e.printStackTrace();
		}

		s.eraseCharAt(2);
		assert (s.equals("013"));
	}

	//比较
	{
		EString s1("0123456789xxx");
		EString s2("0123456789aXX");

		if (s1.equals(s2)) {
			assert (false);
		}
		assert (true);

		if (s1.equalsIgnoreCase(s2)) {
			assert (false);
		}
		assert (true);

		s2.reset("0123456789XXX");
		if (!s1.equalsIgnoreCase(s2)) {
			assert (false);
		}

		boolean r = false;

		r = s1.startsWith("abc");
		assert (!r);
		r = s2.startsWith("0123");
		assert (r);
		r = s2.startsWith("0123456789");
		assert (r);
		r = s2.startsWith("0123456789XXXXXX");
		assert (!r);

		r = s1.endsWith("abc");
		assert (!r);
		r = s1.endsWith("xxx");
		assert (r);
		r = s1.endsWith("9xxx");
		assert (r);
		r = s1.startsWith("0", -1);
		assert (!r);

		if (s1.compareNoCase(s2) != 0) {
			assert (false);
		}
		assert (s1 == "0123456789xxx");
	}

	//搜索
	{
		EString s("1234567890xxx");
		int index = -1;

		index = s.indexOf("0");
		assert (index == 9);
		index = s.indexOf('a');
		assert (index == -1);
		index = s.indexOf('1');
		assert (index == 0);

		index = s.indexOf("0");
		assert (index > 0);
		index = s.indexOf("a");
		assert (index < 0);
		index = s.indexOf("1");
		assert (index == 0);
		index = s.indexOf("123");
		assert (index == 0);
		index = s.indexOf("xxx");
		assert (index == 10);
		index = s.indexOf("123", 2);
		assert (index == -1);
		index = s.indexOf("456", 2);
		assert (index == 3);

		s.reset("1234567890xxx1234567890");
		index = s.indexOf("123");
		assert (index == 0);

		index = s.lastIndexOf("123");
		assert (index == 13);
		index = s.lastIndexOf("123", 14);
		assert (index == 13);
		index = s.lastIndexOf("aaa", 14);
		assert (index == -1);

		index = s.indexOf('1', 0);
		assert (index == 0);
		index = s.lastIndexOf('1', 100);
		assert (index == 13);
	}

	//其他
	{
		EString s("1234567890xxx");
		for (int i=0; i<10000; i++) {
			s += "xxxx";
		}
		LOG("l=%d", s.data().capacity());
		assert (!s.isEmpty());
		s = "";
		assert (s.isEmpty());
		assert (s.length() == 0);

		s = "xxxxxx";
		s.trimToSize();
		LOG("l=%d", s.data().capacity());
		LOG("s=%s", s.c_str());

		s = "1|  2|3| 444|";
		EString s1 = s.splitAt("|", 1);
		LOG("s1=%s", s1.c_str());
		EString s2 = s.splitAt("|", 2);
		LOG("s2=%s", s2.c_str());
		EString s3 = s.splitAt("| ", 3);
		LOG("s3=%s", s3.c_str());
		EString s5 = s.splitAt("|", 5);
		LOG("s5=%s", s5.c_str());
		EString s6 = s.splitAt("|", 6);
		LOG("s6=%s", s6.c_str());

		s.reverse();
		LOG("s=%s", s.c_str());

		s = "1234567890";
		int h = s.hashCode();
		assert (h == -2054162789);
	}

	LOG("string test success.");
}

MAIN_IMPL(teststr) {
	printf("main()\n");

	ESystem::init(argc, argv);

	LOG("inited.");

	do {
		try {

			test_string0();
//			llong t1 = ESystem::currentTimeMillis();
//			for (int i=0; i<1000000; i++) {
//				test_string0();
//			}
//			llong t2 = ESystem::currentTimeMillis();
//			LOG("t = %ld", t2-t1);
			test_string1();

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
