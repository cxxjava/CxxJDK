/*
 * EBson.hh
 *
 *  Created on: 2013-3-20
 *      Author: cxxjava@163.com
 */

#ifndef EBSON_HH_
#define EBSON_HH_

#include "EByteBuffer.hh"
#include "EOutputStream.hh"
#include "EIOException.hh"
#include "ERuntimeException.hh"
#include "ENumberFormatException.hh"
#include "ENoSuchElementException.hh"

namespace efc {

class EBson: public EObject {
public:
	virtual ~EBson();
	EBson(const char *encoding=NULL);

	EBson(const EBson& that);
	EBson& operator= (const EBson& that);

	void clear();
	boolean isEmpty();

	void Import(void *buffer, ulong size);
	void Export(es_buffer_t *buffer, const char *path, boolean useHead=false) THROWS(EIOException);
	void Export(EByteBuffer* buffer, const char *path, boolean useHead=false) THROWS(EIOException);
	void Export(EOutputStream* ostream, const char *path, boolean useHead=false) THROWS(EIOException);

	void copyFrom(const char *toPath, EBson *from, const char *fromPath) THROWS(ERuntimeException);

	es_bson_node_t* add(const char *path, const char *str);
	es_bson_node_t* add(const char *path, const void *data, es_size_t size);
	es_bson_node_t* addFormat(const char *path, const char *fmt, ...);
	es_bson_node_t* addByte(const char *path, byte v);
	es_bson_node_t* addShort(const char *path, short v);
	es_bson_node_t* addInt(const char *path, int v);
	es_bson_node_t* addLLong(const char *path, llong v);
	es_bson_node_t* addFloat(const char *path, float v);
	es_bson_node_t* addDouble(const char *path, double v);
	es_bson_node_t* addVarint(const char *path, llong v);

	es_bson_node_t* set(const char *path, const char *str);
	es_bson_node_t* set(const char *path, const void *data, es_size_t size);
	es_bson_node_t* setFormat(const char *path, const char *fmt, ...);
	es_bson_node_t* setByte(const char *path, byte v);
	es_bson_node_t* setShort(const char *path, short v);
	es_bson_node_t* setInt(const char *path, int v);
	es_bson_node_t* setLLong(const char *path, llong v);
	es_bson_node_t* setFloat(const char *path, float v);
	es_bson_node_t* setDouble(const char *path, double v);
	es_bson_node_t* setVarint(const char *path, llong v);

	char* get(const char *path);
	void* get(const char *path, es_size_t *size);
	EString getString(const char *path) THROWS(ENoSuchElementException);
	byte getByte(const char *path) THROWS2(ENoSuchElementException, ENumberFormatException);
	short getShort(const char *path) THROWS2(ENoSuchElementException, ENumberFormatException);
	int getInt(const char *path) THROWS2(ENoSuchElementException, ENumberFormatException);
	llong getLLong(const char *path) THROWS2(ENoSuchElementException, ENumberFormatException);
	float getFloat(const char *path) THROWS2(ENoSuchElementException, ENumberFormatException);
	double getDouble(const char *path) THROWS2(ENoSuchElementException, ENumberFormatException);

	EString getString(const char *path, const char *defaultValue) THROWS(ENoSuchElementException);
	byte getByte(const char *path, byte defaultValue) THROWS2(ENoSuchElementException, ENumberFormatException);
	short getShort(const char *path, short defaultValue) THROWS2(ENoSuchElementException, ENumberFormatException);
	int getInt(const char *path, int defaultValue) THROWS2(ENoSuchElementException, ENumberFormatException);
	llong getLLong(const char *path, llong defaultValue) THROWS2(ENoSuchElementException, ENumberFormatException);
	float getFloat(const char *path, float defaultValue) THROWS2(ENoSuchElementException, ENumberFormatException);
	double getDouble(const char *path, double defaultValue) THROWS2(ENoSuchElementException, ENumberFormatException);

	int count(const char *path);
	es_status_t del(const char *path);
	es_bson_node_t* find(const char *path);
	es_bson_t* c_bson();

	int indexOf(es_bson_node_t *node);
	int levelOf(es_bson_node_t *node);
	char *pathOf(es_bson_node_t *node, char *path_buf, int buf_size);

	void load(const char *fname) THROWS2(EIOException,ERuntimeException);

	void save(const char *fname, const char *node) THROWS2(EIOException,ERuntimeException);

	virtual EString toString();

	static es_status_t nodeDelete(es_bson_node_t *node);
	static es_bson_node_t* childFind(es_bson_node_t *node, const char *path);
	static int childCount(es_bson_node_t *node, const char *path);

	static char* nodeGet(es_bson_node_t *node);
	static void* nodeGet(es_bson_node_t *node, es_size_t *size);
	static EString nodeGetString(es_bson_node_t *node);
	static byte nodeGetByte(es_bson_node_t *node) THROWS(ENumberFormatException);
	static short nodeGetShort(es_bson_node_t *node) THROWS(ENumberFormatException);
	static int nodeGetInt(es_bson_node_t *node) THROWS(ENumberFormatException);
	static llong nodeGetLLong(es_bson_node_t *node) THROWS(ENumberFormatException);
	static float nodeGetFloat(es_bson_node_t *node) THROWS(ENumberFormatException);
	static double nodeGetDouble(es_bson_node_t *node) THROWS(ENumberFormatException);

	static EString nodeGetString(es_bson_node_t *node, const char* defaultValue);
	static byte nodeGetByte(es_bson_node_t *node, byte defaultValue) THROWS(ENumberFormatException);
	static short nodeGetShort(es_bson_node_t *node, short defaultValue) THROWS(ENumberFormatException);
	static int nodeGetInt(es_bson_node_t *node, int defaultValue) THROWS(ENumberFormatException);
	static llong nodeGetLLong(es_bson_node_t *node, llong defaultValue) THROWS(ENumberFormatException);
	static float nodeGetFloat(es_bson_node_t *node, float defaultValue) THROWS(ENumberFormatException);
	static double nodeGetDouble(es_bson_node_t *node, double defaultValue) THROWS(ENumberFormatException);

	static es_bson_node_t* nodeUpdate(es_bson_node_t *node, const char *key, const char *str);
	static es_bson_node_t* nodeUpdate(es_bson_node_t *node, const char *key, const void *data, es_size_t size);
	static es_bson_node_t* nodeUpdateFormat(es_bson_node_t *node, const char *key, const char *fmt, ...);
	static es_bson_node_t* nodeUpdateByte(es_bson_node_t *node, const char *key, byte v);
	static es_bson_node_t* nodeUpdateShort(es_bson_node_t *node, const char *key, short v);
	static es_bson_node_t* nodeUpdateInt(es_bson_node_t *node, const char *key, int v);
	static es_bson_node_t* nodeUpdateLLong(es_bson_node_t *node, const char *key, llong v);
	static es_bson_node_t* nodeUpdateFloat(es_bson_node_t *node, const char *key, float v);
	static es_bson_node_t* nodeUpdateDouble(es_bson_node_t *node, const char *key, double v);
	static es_bson_node_t* nodeUpdateVarint(es_bson_node_t *node, const char *key, llong v);

	static es_bson_node_t* attrAdd(es_bson_node_t *node, const char *key, const char *str);
	static es_bson_node_t* attrAdd(es_bson_node_t *node, const char *key, const void *data, es_size_t size);
	static es_bson_node_t* attrAddFormat(es_bson_node_t *node, const char *key, const char *fmt, ...);
	static es_bson_node_t* attrAddByte(es_bson_node_t *node, const char *key, byte v);
	static es_bson_node_t* attrAddShort(es_bson_node_t *node, const char *key, short v);
	static es_bson_node_t* attrAddInt(es_bson_node_t *node, const char *key, int v);
	static es_bson_node_t* attrAddLLong(es_bson_node_t *node, const char *key, llong v);
	static es_bson_node_t* attrAddFloat(es_bson_node_t *node, const char *key, float v);
	static es_bson_node_t* attrAddDouble(es_bson_node_t *node, const char *key, double v);
	static es_bson_node_t* attrAddVarint(es_bson_node_t *node, const char *key, llong v);

	static es_bson_node_t* attrSet(es_bson_node_t *node, const char *key, const char *str);
	static es_bson_node_t* attrSet(es_bson_node_t *node, const char *key, const void *data, es_size_t size);
	static es_bson_node_t* attrSetFormat(es_bson_node_t *node, const char *key, const char *fmt, ...);
	static es_bson_node_t* attrSetByte(es_bson_node_t *node, const char *key, byte v);
	static es_bson_node_t* attrSetShort(es_bson_node_t *node, const char *key, short v);
	static es_bson_node_t* attrSetInt(es_bson_node_t *node, const char *key, int v);
	static es_bson_node_t* attrSetLLong(es_bson_node_t *node, const char *key, llong v);
	static es_bson_node_t* attrSetFloat(es_bson_node_t *node, const char *key, float v);
	static es_bson_node_t* attrSetDouble(es_bson_node_t *node, const char *key, double v);
	static es_bson_node_t* attrSetVarint(es_bson_node_t *node, const char *key, llong v);

	static char* attrGet(es_bson_node_t *node, const char *key);
	static void* attrGet(es_bson_node_t *node, const char *key, es_size_t *size);
	static EString attrGetString(es_bson_node_t *node, const char *key) THROWS(ENoSuchElementException);
	static byte attrGetByte(es_bson_node_t *node, const char *key) THROWS(ENumberFormatException);
	static short attrGetShort(es_bson_node_t *node, const char *key) THROWS(ENumberFormatException);
	static int attrGetInt(es_bson_node_t *node, const char *key) THROWS(ENumberFormatException);
	static llong attrGetLLong(es_bson_node_t *node, const char *key) THROWS(ENumberFormatException);
	static float attrGetFloat(es_bson_node_t *node, const char *key) THROWS(ENumberFormatException);
	static double attrGetDouble(es_bson_node_t *node, const char *key) THROWS(ENumberFormatException);

	static EString attrGetString(es_bson_node_t *node, const char *key, const char* defaultValue) THROWS(ENoSuchElementException);
	static byte attrGetByte(es_bson_node_t *node, const char *key, byte defaultValue) THROWS(ENumberFormatException);
	static short attrGetShort(es_bson_node_t *node, const char *key, short defaultValue) THROWS(ENumberFormatException);
	static int attrGetInt(es_bson_node_t *node, const char *key, int defaultValue) THROWS(ENumberFormatException);
	static llong attrGetLLong(es_bson_node_t *node, const char *key, llong defaultValue) THROWS(ENumberFormatException);
	static float attrGetFloat(es_bson_node_t *node, const char *key, float defaultValue) THROWS(ENumberFormatException);
	static double attrGetDouble(es_bson_node_t *node, const char *key, double defaultValue) THROWS(ENumberFormatException);

	static es_status_t attrDelete(es_bson_node_t *node, const char *key);
	static void attrClear(es_bson_node_t *node);

private:
	es_bson_t *m_bson;
};

} /* namespace efc */
#endif /* EBSON_HH_ */
