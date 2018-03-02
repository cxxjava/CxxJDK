/*
 * testbson.cpp
 *
 *  Created on: 2014-2-19
 *      Author: cxxjava@163.com
 */
#include "es_main.h"
#include "Efc.hh"

#define LOG(fmt,...) ESystem::out->printfln(fmt, ##__VA_ARGS__)

static void test_bson()
{
	EBson bson;
	bson.add("/root", "root");
	bson.addInt("/root/n0", 999);
	bson.addShort("/root/n1", 66);
	bson.addFloat("/root/n2", 9.55);
	bson.addDouble("/root/n3", 6.544445);
	bson.addVarint("/root/n4", 66999888);
	bson.add("/root/n5", "887878");
	for (int i = 0; i < 3; i++) {
		bson.add("/root/field", NULL);
		bson.add("/root/field/name", "name");
		bson.add("/root/field/type", "type");
		bson.add("/root/field/length", "length");
		bson.add("/root/field/precision", "precision");
		bson.add("/root/field/scale", "scale");
	}

	//export
	EByteBuffer bb;
	bson.Export(&bb, NULL);

	EByteArrayOutputStream baos;
	bson.Export(&baos, NULL);

	//import
	EBson bson2;
	bson2.Import(bb.data(), bb.size());
	bson2.set("/root/n0", "1111");
	//re import
	bson2.Import(baos.data(), baos.size());
	bson2.set("/root/n0", "2222");

	LOG("n0=%d", bson2.getInt("/root/n0"));
	LOG("n0=%s", bson2.getString("/root/n0").c_str());
	LOG("n1=%d", bson2.getShort("/root/n1"));
	LOG("n2=%f", bson2.getFloat("/root/n2"));
	LOG("n3=%lf", bson2.getDouble("/root/n3"));
	LOG("n3=%s", bson2.getString("/root/n3").c_str());
	LOG("n4=%ld", bson2.getLLong("/root/n4"));
	LOG("n5=%ld", bson2.getLLong("/root/n5"));
	LOG("xxx=%ld", bson2.getLLong("/root/xxx", 88888888));

	//parser
	EByteArrayInputStream bais(baos.data(), baos.size());
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
			LOG(EBson::nodeGetString(node).c_str());
		}
	};

	BsonParser ep(&bais);
	EBson bson_;
	while (ep.nextBson(&bson_)) {
		//
	}
}

static void test_bson_parser()
{
	const char* inFileName = ESystem::getProgramArgument("i");
	if (!inFileName || eso_strlen(inFileName) == 0) {
		ESystem::out->println("Usage: bsonparser -i [filename] {-o [filename]}");
		ESystem::exit(0);
	}
	const char* outFileName = ESystem::getProgramArgument("o");

	EFileInputStream *fis = new EFileInputStream(inFileName);
	EFileOutputStream *fos =
			(outFileName && eso_strlen(outFileName) > 0) ?
					new EFileOutputStream(outFileName) :
					new EFileOutputStream(stdout);

	class BsonParser : public EBsonParser {
	public:
		BsonParser(EFileInputStream *fis,
				EFileOutputStream* fos) :
				EBsonParser(fis) {
			_fos = fos;
		}
		void parsing(es_bson_node_t* node) {
			if (!node) return;

			for (int i=1; i<_bson->levelOf(node); i++) {
				_fos->write("\t");
			}
			_fos->write(node->name);
			_fos->write("-->");
			_fos->write(node->value->data, node->value->len);
			_fos->write("\n");
		}
	private:
		EFileOutputStream* _fos;
	};

	BsonParser ep(fis, fos);
	EBson bson;
	while (ep.nextBson(&bson)) {
		//
	}

	delete fis;
	delete fos;
}

static void test_performance() {
	llong startTime = ESystem::currentTimeMillis();
	sp<EA<byte> > array;
	for (int i = 0; i < 50000; i++) {
		EByteArrayOutputStream baos(32, false);
		EBson bs;
		bs.addInt("ID", 10);
		bs.addLLong("GoodID", 100);
		bs.add("Url", "http://xxx.jpg");
		bs.add("Guid", "11111-22222-3333-444");
		bs.add("Type", "ITEM");
		bs.addInt("Order", 0);
		bs.Export(&baos, null);
		array = baos.toByteArray();
	}
	llong endTime = ESystem::currentTimeMillis();
	LOG("serialized size:%d, time used:%dms", array->length(),
			(endTime - startTime));

	for (int i = 0; i < 50000; i++) {
		EByteArrayInputStream bais(array->address(), array->length(), false);
		EBsonParser bp(&bais);
		EBson bs;
		bp.nextBson(&bs);
	}
	llong endTime2 = ESystem::currentTimeMillis();
	LOG("deserialize time used:%dms", (endTime2 - endTime));
}

MAIN_IMPL(testbson) {
	printf("main()\n");

	ESystem::init(argc, argv);

	LOG("inited.");

	do {
		try {

//		test_bson_parser();
//		test_bson_parser();
		test_performance();

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
