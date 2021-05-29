/*
 * EBson.cpp
 *
 *  Created on: 2013-3-20
 *      Author: cxxjava@163.com
 */

#include "EBson.hh"
#include "ELLong.hh"
#include "EFloat.hh"
#include "EDouble.hh"
#include "ENullPointerException.hh"

namespace efc {

EBson::~EBson() {
	eso_bson_destroy(&m_bson);
}

EBson::EBson(const char *encoding) {
	m_bson = eso_bson_create(encoding);
}

EBson::EBson(const EBson& that) {
	EBson* t = (EBson*)&that;
	m_bson = eso_bson_create(t->c_bson()->head.encoding);
	this->copyFrom(NULL, t, NULL);
}

EBson& EBson::operator= (const EBson& that) {
	if (this == &that) return *this;

	EBson* t = (EBson*)&that;

	//1.
	eso_bson_clear(m_bson);

	//2.
	this->copyFrom(NULL, t, NULL);

	return *this;
}

void EBson::clear() {
	eso_bson_clear(m_bson);
}

boolean EBson::isEmpty() {
	return eso_bson_is_empty(m_bson);
}

void EBson::Import(void *buffer, ulong size) {
	(void)eso_bson_import(m_bson, buffer, size);
}

void EBson::Export(es_buffer_t *buffer, const char *path, boolean useHead) {
	es_status_t stat = eso_bson_export(m_bson, buffer, path, useHead);
	if (stat != ES_SUCCESS) {
		throw EIOException(__FILE__, __LINE__);
	}
}

typedef struct {
	es_ostream_t s;
	EByteBuffer *bb;
} bson_export_bb_ostream;

static es_size_t bbos_stream_write(void *s, const void *buf, es_size_t size)
{
	bson_export_bb_ostream *os = (bson_export_bb_ostream*)s;
	return os->bb->append(buf, size);
}

void EBson::Export(EByteBuffer* bb, const char *path, boolean useHead) {
	bson_export_bb_ostream bbos;
	bbos.s.write = bbos_stream_write;
	bbos.bb = bb;

	es_status_t stat = eso_bson_export2(m_bson, (es_ostream_t*)&bbos, path, useHead);
	if (stat != ES_SUCCESS) {
		throw EIOException(__FILE__, __LINE__);
	}
}

typedef struct {
	es_ostream_t s;
	EOutputStream *os;
} bson_export_os_ostream;

static es_size_t osos_stream_write(void *s, const void *buf, es_size_t size)
{
	bson_export_os_ostream *os = (bson_export_os_ostream*)s;
	try {
		os->os->write(buf, size);
	} catch (...) {
		return 0;
	}
	return size;
}

void EBson::Export(EOutputStream* ostream, const char *path, boolean useHead) {
	bson_export_os_ostream osos;
	osos.s.write = osos_stream_write;
	osos.os = ostream;

	es_status_t stat = eso_bson_export2(m_bson, (es_ostream_t*)&osos, path, useHead);
	if (stat != ES_SUCCESS) {
		throw EIOException(__FILE__, __LINE__);
	}
}

void EBson::copyFrom(const char *toPath, EBson *from, const char *fromPath) THROWS(ERuntimeException)
{
	es_status_t rv = eso_bson_clone(m_bson, toPath, from->c_bson(), fromPath);
	if (rv != ES_SUCCESS) {
		switch (rv) {
		case BSON_EINVAL: throw ERuntimeException(__FILE__, __LINE__, "BSON_EINVAL");
		case BSON_EHEAD: throw ERuntimeException(__FILE__, __LINE__, "BSON_EHEAD");
		case BSON_ENONODE: throw ERuntimeException(__FILE__, __LINE__, "BSON_ENONODE");
		default: throw ERuntimeException(__FILE__, __LINE__);
		}
	}
}

es_bson_node_t* EBson::add(const char *path, const char *str) {
	return eso_bson_add_str(m_bson, path, str);
}

es_bson_node_t* EBson::add(const char *path, const void *data, es_size_t size) {
	return eso_bson_add_bin(m_bson, path, data, size);
}

es_bson_node_t* EBson::addFormat(const char *path, const char *fmt, ...) {
	va_list args;
	es_string_t *value = NULL;
	es_bson_node_t *node;

	va_start(args, fmt);
	eso_mvsprintf(&value, fmt, args);
	va_end(args);

	node = eso_bson_add_str(m_bson, path, value);
	ESO_MFREE(&value);

	return node;
}

es_bson_node_t* EBson::addByte(const char *path, byte v) {
	return eso_bson_add_int8(m_bson, path, v);
}

es_bson_node_t* EBson::addShort(const char *path, short v) {
	return eso_bson_add_int16(m_bson, path, v);
}

es_bson_node_t* EBson::addInt(const char *path, int v) {
	return eso_bson_add_int32(m_bson, path, v);
}

es_bson_node_t* EBson::addLLong(const char *path, llong v) {
	return eso_bson_add_int64(m_bson, path, v);
}

es_bson_node_t* EBson::addFloat(const char *path, float v) {
	return eso_bson_add_float(m_bson, path, v);
}

es_bson_node_t* EBson::addDouble(const char *path, double v) {
	return eso_bson_add_double(m_bson, path, v);
}

es_bson_node_t* EBson::addVarint(const char *path, llong v) {
	return eso_bson_add_varint(m_bson, path, v);
}

es_bson_node_t* EBson::set(const char *path, const char *str) {
	return eso_bson_set_str(m_bson, path, str);
}

es_bson_node_t* EBson::set(const char *path, const void *data, es_size_t size) {
	return eso_bson_set_bin(m_bson, path, data, size);
}

es_bson_node_t* EBson::setFormat(const char *path, const char *fmt, ...) {
	va_list args;
	es_string_t *value = NULL;
	es_bson_node_t *node;

	va_start(args, fmt);
	eso_mvsprintf(&value, fmt, args);
	va_end(args);

	node = eso_bson_set_str(m_bson, path, value);
	ESO_MFREE(&value);

	return node;
}

es_bson_node_t* EBson::setByte(const char *path, byte v) {
	return eso_bson_set_int8(m_bson, path, v);
}

es_bson_node_t* EBson::setShort(const char *path, short v) {
	return eso_bson_set_int16(m_bson, path, v);
}

es_bson_node_t* EBson::setInt(const char *path, int v) {
	return eso_bson_set_int32(m_bson, path, v);
}

es_bson_node_t* EBson::setLLong(const char *path, llong v) {
	return eso_bson_set_int64(m_bson, path, v);
}

es_bson_node_t* EBson::setFloat(const char *path, float v) {
	return eso_bson_set_float(m_bson, path, v);
}

es_bson_node_t* EBson::setDouble(const char *path, double v) {
	return eso_bson_set_double(m_bson, path, v);
}

es_bson_node_t* EBson::setVarint(const char *path, llong v) {
	return eso_bson_set_varint(m_bson, path, v);
}

char* EBson::get(const char *path) {
	return eso_bson_get_str(m_bson, path);
}

void* EBson::get(const char *path, es_size_t *size) {
	return eso_bson_get_bin(m_bson, path, size, NULL);
}

EString EBson::getString(const char *path) {
	es_bson_node_t* node = find(path);
	if (!node) {
		throw ENoSuchElementException(__FILE__, __LINE__);
	}
	return nodeGetString(node);
}

byte EBson::getByte(const char *path) {
	return (byte)getLLong(path);
}

short EBson::getShort(const char *path) {
	return (short)getLLong(path);
}

int EBson::getInt(const char *path) {
	return (int)getLLong(path);
}

llong EBson::getLLong(const char *path) {
	es_bson_node_t* node = find(path);
	if (!node) {
		throw ENoSuchElementException(__FILE__, __LINE__);
	}
	return nodeGetLLong(node);
}

float EBson::getFloat(const char *path) {
	return getDouble(path);
}

double EBson::getDouble(const char *path) {
	es_bson_node_t* node = find(path);
	if (!node) {
		throw ENoSuchElementException(__FILE__, __LINE__);
	}
	return nodeGetDouble(node);
}

EString EBson::getString(const char *path, const char *defaultValue) {
	es_bson_node_t* node = find(path);
	if (!node) {
		return defaultValue;
	}
	try {
		return nodeGetString(node);
	} catch (...) {
		return defaultValue;
	}
	//not reach here.
	ES_ASSERT(false);
	return null;
}

byte EBson::getByte(const char *path, byte defaultValue) {
	es_bson_node_t* node = find(path);
	if (!node) {
		return defaultValue;
	}
	try {
		return (byte)nodeGetLLong(node);
	} catch (...) {
		return defaultValue;
	}
	//not reach here.
	ES_ASSERT(false);
	return 0;
}

short EBson::getShort(const char *path, short defaultValue) {
	es_bson_node_t* node = find(path);
	if (!node) {
		return defaultValue;
	}
	try {
		return (short)nodeGetLLong(node);
	} catch (...) {
		return defaultValue;
	}
	//not reach here.
	ES_ASSERT(false);
	return 0;
}

int EBson::getInt(const char *path, int defaultValue) {
	es_bson_node_t* node = find(path);
	if (!node) {
		return defaultValue;
	}
	try {
		return (int)nodeGetLLong(node);
	} catch (...) {
		return defaultValue;
	}
	//not reach here.
	ES_ASSERT(false);
	return 0;
}

llong EBson::getLLong(const char *path, llong defaultValue) {
	es_bson_node_t* node = find(path);
	if (!node) {
		return defaultValue;
	}
	try {
		return (llong)nodeGetLLong(node);
	} catch (...) {
		return defaultValue;
	}
	//not reach here.
	ES_ASSERT(false);
	return 0;
}

float EBson::getFloat(const char *path, float defaultValue) {
	es_bson_node_t* node = find(path);
	if (!node) {
		return defaultValue;
	}
	try {
		return (float)nodeGetDouble(node);
	} catch (...) {
		return defaultValue;
	}
	//not reach here.
	ES_ASSERT(false);
	return 0;
}

double EBson::getDouble(const char *path, double defaultValue) {
	es_bson_node_t* node = find(path);
	if (!node) {
		return defaultValue;
	}
	try {
		return (double)nodeGetDouble(node);
	} catch (...) {
		return defaultValue;
	}
	//not reach here.
	ES_ASSERT(false);
	return 0;
}

int EBson::count(const char *path) {
	return eso_bson_node_count(m_bson, path);
}

es_status_t EBson::del(const char *path) {
	return eso_bson_del(m_bson, path);
}

es_bson_node_t* EBson::find(const char *path) {
	return eso_bson_node_find(m_bson, path);
}

es_bson_t* EBson::c_bson()
{
	return m_bson;
}

int EBson::indexOf(es_bson_node_t *node) {
	return eso_bson_node_index(node);
}

int EBson::levelOf(es_bson_node_t *node) {
	return eso_bson_node_level(node);
}

char* EBson::pathOf(es_bson_node_t *node, char *path_buf, int buf_size) {
	return eso_bson_node_path(node, path_buf, buf_size);
}

es_status_t EBson::nodeDelete(es_bson_node_t *node) {
	return eso_bson_node_del(node);
}

es_bson_node_t* EBson::childFind(es_bson_node_t *node, const char *path) {
	return eso_bson_node_child_get(node, path);
}

int EBson::childCount(es_bson_node_t *node, const char *path) {
	return eso_bson_node_child_count(node, path);
}

char* EBson::nodeGet(es_bson_node_t *node) {
	return (char*)node->value->data;
}

void* EBson::nodeGet(es_bson_node_t *node, es_size_t *size) {
	if (size) *size = node->value->len;
	return node->value->data;
}

EString EBson::nodeGetString(es_bson_node_t *node) {
	es_size_t size = node->value->len;
	int type = node->dtype;
	void* v = node->value->data;
	if (!v) {
		throw ENullPointerException(__FILE__, __LINE__);
	}
	switch (type) {
	case BSON_NODE_DATA_TYPE_STRING:
	case BSON_NODE_DATA_TYPE_BINARY:
		return (node->value->len == 0) ? null : EString((char*)v, size);
	case BSON_NODE_DATA_TYPE_I8:
		return EString((byte)eso_array2llong((es_byte_t*)v, size));
	case BSON_NODE_DATA_TYPE_I16:
		return EString((short)eso_array2llong((es_byte_t*)v, size));
	case BSON_NODE_DATA_TYPE_I32:
		return EString((int)eso_array2llong((es_byte_t*)v, size));
	case BSON_NODE_DATA_TYPE_I64:
		return EString(eso_array2llong((es_byte_t*)v, size));
	case BSON_NODE_DATA_TYPE_FLOAT:
	{
		es_int32_t i = eso_array2llong((es_byte_t*)v, size);
		EFloat f(eso_intBits2float(i));
		return f.toString();
	}
	case BSON_NODE_DATA_TYPE_DOUBLE:
	{
		es_int64_t i = eso_array2llong((es_byte_t*)v, size);
		EDouble d(eso_llongBits2double(i));
		return d.toString();
	}
	case BSON_NODE_DATA_TYPE_VARINT:
	{
		es_uint64_t l;
		eso_arrayToVarint64((es_uint8_t*)v, &l);
		return EString((llong)l);
	}
	default:
		throw ENumberFormatException(__FILE__, __LINE__);
	}
}

byte EBson::nodeGetByte(es_bson_node_t *node) {
	return (byte)nodeGetLLong(node);
}

short EBson::nodeGetShort(es_bson_node_t *node) {
	return (short)nodeGetLLong(node);
}

int EBson::nodeGetInt(es_bson_node_t *node) {
	return (int)nodeGetLLong(node);
}

llong EBson::nodeGetLLong(es_bson_node_t *node) {
	es_size_t size = node->value->len;
	int type = node->dtype;
	void* v = node->value->data;
	if (!v) {
		throw ENumberFormatException(__FILE__, __LINE__);
	}
	switch (type) {
	case BSON_NODE_DATA_TYPE_STRING:
		return ELLong::parseLLong((char*)v);
	case BSON_NODE_DATA_TYPE_BINARY:
	{
		if (size > sizeof(llong)) {
			throw ENumberFormatException(__FILE__, __LINE__);
		}
		return eso_array2llong((es_byte_t*)v, size);
	}
	case BSON_NODE_DATA_TYPE_I8:
	case BSON_NODE_DATA_TYPE_I16:
	case BSON_NODE_DATA_TYPE_I32:
	case BSON_NODE_DATA_TYPE_I64:
		return eso_array2llong((es_byte_t*)v, size);
	case BSON_NODE_DATA_TYPE_FLOAT:
	{
		es_int32_t i = eso_array2llong((es_byte_t*)v, size);
		return (llong)eso_intBits2float(i);
	}
	case BSON_NODE_DATA_TYPE_DOUBLE:
	{
		es_int64_t i = eso_array2llong((es_byte_t*)v, size);
		return (llong)eso_llongBits2double(i);
	}
	case BSON_NODE_DATA_TYPE_VARINT:
	{
		es_uint64_t l;
		eso_arrayToVarint64((es_uint8_t*)v, &l);
		return (llong)l;
	}
	default:
		throw ENumberFormatException(__FILE__, __LINE__);
	}
}

float EBson::nodeGetFloat(es_bson_node_t *node) {
	return nodeGetDouble(node);
}

double EBson::nodeGetDouble(es_bson_node_t *node) {
	es_size_t size = node->value->len;
	int type = node->dtype;
	void* v = node->value->data;
	if (!v) {
		throw ENumberFormatException(__FILE__, __LINE__);
	}
	switch (type) {
	case BSON_NODE_DATA_TYPE_STRING:
		return EDouble::parseDouble((char*)v);
	case BSON_NODE_DATA_TYPE_BINARY:
	{
		if (size > sizeof(double)) {
			throw ENumberFormatException(__FILE__, __LINE__);
		}
		es_int64_t i = eso_array2llong((es_byte_t*)v, size);
		return eso_llongBits2double(i);
	}
	case BSON_NODE_DATA_TYPE_I8:
	case BSON_NODE_DATA_TYPE_I16:
	case BSON_NODE_DATA_TYPE_I32:
	case BSON_NODE_DATA_TYPE_I64:
		return eso_array2llong((es_byte_t*)v, size);
	case BSON_NODE_DATA_TYPE_FLOAT:
	{
		es_int32_t i = eso_array2llong((es_byte_t*)v, size);
		return eso_intBits2float(i);
	}
	case BSON_NODE_DATA_TYPE_DOUBLE:
	{
		es_int64_t i = eso_array2llong((es_byte_t*)v, size);
		return eso_llongBits2double(i);
	}
	case BSON_NODE_DATA_TYPE_VARINT:
	{
		es_uint64_t l;
		eso_arrayToVarint64((es_uint8_t*)v, &l);
		return l;
	}
	default:
		throw ENumberFormatException(__FILE__, __LINE__);
	}
}

EString EBson::nodeGetString(es_bson_node_t *node, const char* defaultValue) {
	try {
		return nodeGetString(node);
	} catch (...) {
		return defaultValue;
	}
	//not reach here.
	ES_ASSERT(false);
	return null;
}

byte EBson::nodeGetByte(es_bson_node_t *node, byte defaultValue) {
	try {
		return nodeGetByte(node);
	} catch (...) {
		return defaultValue;
	}
	//not reach here.
	ES_ASSERT(false);
	return 0;
}

short EBson::nodeGetShort(es_bson_node_t *node, short defaultValue) {
	try {
		return nodeGetShort(node);
	} catch (...) {
		return defaultValue;
	}
	//not reach here.
	ES_ASSERT(false);
	return 0;
}

int EBson::nodeGetInt(es_bson_node_t *node, int defaultValue) {
	try {
		return nodeGetInt(node);
	} catch (...) {
		return defaultValue;
	}
	//not reach here.
	ES_ASSERT(false);
	return 0;
}

llong EBson::nodeGetLLong(es_bson_node_t *node, llong defaultValue) {
	try {
		return nodeGetLLong(node);
	} catch (...) {
		return defaultValue;
	}
	//not reach here.
	ES_ASSERT(false);
	return 0;
}

float EBson::nodeGetFloat(es_bson_node_t *node, float defaultValue) {
	try {
		return nodeGetFloat(node);
	} catch (...) {
		return defaultValue;
	}
	//not reach here.
	ES_ASSERT(false);
	return 0;
}

double EBson::nodeGetDouble(es_bson_node_t *node, double defaultValue) {
	try {
		return nodeGetDouble(node);
	} catch (...) {
		return defaultValue;
	}
	//not reach here.
	ES_ASSERT(false);
	return 0;
}

es_bson_node_t* EBson::nodeUpdate(es_bson_node_t *node, const char *key,
		const char *str) {
	return eso_bson_node_update_str(node, key, str);
}

es_bson_node_t* EBson::nodeUpdate(es_bson_node_t *node, const char *key,
		const void *data, es_size_t size) {
	return eso_bson_node_update_bin(node, key, data, size);
}

es_bson_node_t* EBson::nodeUpdateFormat(es_bson_node_t *node, const char *key,
		const char *fmt, ...) {
	va_list args;
	es_string_t *value = NULL;

	va_start(args, fmt);
	eso_mvsprintf(&value, fmt, args);
	va_end(args);

	node = eso_bson_node_update_str(node, key, value);
	ESO_MFREE(&value);

	return node;
}

es_bson_node_t* EBson::nodeUpdateByte(es_bson_node_t *node, const char *key, byte v) {
	return eso_bson_node_update_int8(node, key, v);
}

es_bson_node_t* EBson::nodeUpdateShort(es_bson_node_t *node, const char *key, short v) {
	return eso_bson_node_update_int16(node, key, v);
}

es_bson_node_t* EBson::nodeUpdateInt(es_bson_node_t *node, const char *key, int v) {
	return eso_bson_node_update_int32(node, key, v);
}

es_bson_node_t* EBson::nodeUpdateLLong(es_bson_node_t *node, const char *key, llong v) {
	return eso_bson_node_update_int64(node, key, v);
}

es_bson_node_t* EBson::nodeUpdateFloat(es_bson_node_t *node, const char *key, float v) {
	return eso_bson_node_update_float(node, key, v);
}

es_bson_node_t* EBson::nodeUpdateDouble(es_bson_node_t *node, const char *key, double v) {
	return eso_bson_node_update_double(node, key, v);
}

es_bson_node_t* EBson::nodeUpdateVarint(es_bson_node_t *node, const char *key, llong v) {
	return eso_bson_node_update_varint(node, key, v);
}

es_bson_node_t* EBson::attrAdd(es_bson_node_t *node, const char *key,
		const char *str) {
	return eso_bson_attr_add_str(node, key, str);
}

es_bson_node_t* EBson::attrAdd(es_bson_node_t *node, const char *key,
		const void *data, es_size_t size) {
	return eso_bson_attr_add_bin(node, key, data, size);
}

es_bson_node_t* EBson::attrAddFormat(es_bson_node_t *node, const char *key,
		const char *fmt, ...) {
	va_list args;
	es_string_t *value = NULL;
	es_bson_node_t *new_node;

	va_start(args, fmt);
	eso_mvsprintf(&value, fmt, args);
	va_end(args);

	new_node = eso_bson_attr_add_str(node, key, value);
	ESO_MFREE(&value);

	return new_node;
}

es_bson_node_t* attrAddByte(es_bson_node_t *node, const char *key, byte v) {
	return eso_bson_attr_add_int8(node, key, v);
}

es_bson_node_t* attrAddShort(es_bson_node_t *node, const char *key, short v) {
	return eso_bson_attr_add_int16(node, key, v);
}

es_bson_node_t* attrAddInt(es_bson_node_t *node, const char *key, int v) {
	return eso_bson_attr_add_int32(node, key, v);
}

es_bson_node_t* attrAddLLong(es_bson_node_t *node, const char *key, llong v) {
	return eso_bson_attr_add_int64(node, key, v);
}

es_bson_node_t* attrAddFloat(es_bson_node_t *node, const char *key, float v) {
	return eso_bson_attr_add_float(node, key, v);
}

es_bson_node_t* attrAddDouble(es_bson_node_t *node, const char *key, double v) {
	return eso_bson_attr_add_double(node, key, v);
}

es_bson_node_t* attrAddVarint(es_bson_node_t *node, const char *key, llong v) {
	return eso_bson_attr_add_varint(node, key, v);
}

es_bson_node_t* EBson::attrSet(es_bson_node_t *node, const char *key,
		const char *str) {
	return eso_bson_attr_set_str(node, key, str);
}

es_bson_node_t* EBson::attrSet(es_bson_node_t *node, const char *key,
		const void *data, es_size_t size) {
	return eso_bson_attr_set_bin(node, key, data, size);
}

es_bson_node_t* EBson::attrSetFormat(es_bson_node_t *node, const char *key,
		const char *fmt, ...) {
	va_list args;
	es_string_t *value = NULL;
	es_bson_node_t *attr_node;

	va_start(args, fmt);
	eso_mvsprintf(&value, fmt, args);
	va_end(args);

	attr_node = eso_bson_attr_set_str(node, key, value);
	ESO_MFREE(&value);

	return attr_node;
}

es_bson_node_t* EBson::attrSetByte(es_bson_node_t *node, const char *key, byte v) {
	return eso_bson_attr_set_int8(node, key, v);
}

es_bson_node_t* EBson::attrSetShort(es_bson_node_t *node, const char *key, short v) {
	return eso_bson_attr_set_int16(node, key, v);
}

es_bson_node_t* EBson::attrSetInt(es_bson_node_t *node, const char *key, int v) {
	return eso_bson_attr_set_int32(node, key, v);
}

es_bson_node_t* EBson::attrSetLLong(es_bson_node_t *node, const char *key, llong v) {
	return eso_bson_attr_set_int64(node, key, v);
}

es_bson_node_t* EBson::attrSetFloat(es_bson_node_t *node, const char *key, float v) {
	return eso_bson_attr_set_float(node, key, v);
}

es_bson_node_t* EBson::attrSetDouble(es_bson_node_t *node, const char *key, double v) {
	return eso_bson_attr_set_double(node, key, v);
}

es_bson_node_t* EBson::attrSetVarint(es_bson_node_t *node, const char *key, llong v) {
	return eso_bson_attr_set_varint(node, key, v);
}

char* EBson::attrGet(es_bson_node_t *node, const char *key) {
	return eso_bson_attr_get_str(node, key);
}

void* EBson::attrGet(es_bson_node_t *node, const char *key, es_size_t *size) {
	return eso_bson_attr_get_bin(node, key, size, NULL);
}

EString EBson::attrGetString(es_bson_node_t *node, const char *key) {
	es_size_t size;
	int type;
	void* v = eso_bson_attr_get_bin(node, key, &size, &type);
	if (!v) {
		throw ENoSuchElementException(__FILE__, __LINE__);
	}
	switch (type) {
	case BSON_NODE_DATA_TYPE_STRING:
	case BSON_NODE_DATA_TYPE_BINARY:
		return (node->value->len == 0) ? null : (char*)v;
	case BSON_NODE_DATA_TYPE_I8:
		return EString((byte)eso_array2llong((es_byte_t*)v, size));
	case BSON_NODE_DATA_TYPE_I16:
		return EString((short)eso_array2llong((es_byte_t*)v, size));
	case BSON_NODE_DATA_TYPE_I32:
		return EString((int)eso_array2llong((es_byte_t*)v, size));
	case BSON_NODE_DATA_TYPE_I64:
		return EString(eso_array2llong((es_byte_t*)v, size));
	case BSON_NODE_DATA_TYPE_FLOAT:
	{
		es_int32_t i = eso_array2llong((es_byte_t*)v, size);
		EFloat f(eso_intBits2float(i));
		return f.toString();
	}
	case BSON_NODE_DATA_TYPE_DOUBLE:
	{
		es_int64_t i = eso_array2llong((es_byte_t*)v, size);
		EDouble d(eso_llongBits2double(i));
		return d.toString();
	}
	case BSON_NODE_DATA_TYPE_VARINT:
	{
		es_uint64_t l;
		eso_arrayToVarint64((es_uint8_t*)v, &l);
		return EString((llong)l);
	}
	default:
		throw ENumberFormatException(__FILE__, __LINE__);
	}
}

byte EBson::attrGetByte(es_bson_node_t *node, const char *key) {
	return (byte)attrGetLLong(node, key);
}

short EBson::attrGetShort(es_bson_node_t *node, const char *key) {
	return (short)attrGetLLong(node, key);
}

int EBson::attrGetInt(es_bson_node_t *node, const char *key) {
	return (int)attrGetLLong(node, key);
}

llong EBson::attrGetLLong(es_bson_node_t *node, const char *key) {
	es_size_t size;
	int type;
	void* v = eso_bson_attr_get_bin(node, key, &size, &type);
	if (!v) {
		throw ENumberFormatException(__FILE__, __LINE__);
	}
	switch (type) {
	case BSON_NODE_DATA_TYPE_STRING:
		return ELLong::parseLLong((char*)v);
	case BSON_NODE_DATA_TYPE_BINARY:
	{
		if (size > sizeof(llong)) {
			throw ENumberFormatException(__FILE__, __LINE__);
		}
		return eso_array2llong((es_byte_t*)v, size);
	}
	case BSON_NODE_DATA_TYPE_I8:
	case BSON_NODE_DATA_TYPE_I16:
	case BSON_NODE_DATA_TYPE_I32:
	case BSON_NODE_DATA_TYPE_I64:
		return eso_array2llong((es_byte_t*)v, size);
	case BSON_NODE_DATA_TYPE_FLOAT:
	{
		es_int32_t i = eso_array2llong((es_byte_t*)v, size);
		return (llong)eso_intBits2float(i);
	}
	case BSON_NODE_DATA_TYPE_DOUBLE:
	{
		es_int64_t i = eso_array2llong((es_byte_t*)v, size);
		return (llong)eso_llongBits2double(i);
	}
	case BSON_NODE_DATA_TYPE_VARINT:
	{
		es_uint64_t l;
		eso_arrayToVarint64((es_uint8_t*)v, &l);
		return (llong)l;
	}
	default:
		throw ENumberFormatException(__FILE__, __LINE__);
	}
}

float EBson::attrGetFloat(es_bson_node_t *node, const char *key) {
	return attrGetDouble(node, key);
}

double EBson::attrGetDouble(es_bson_node_t *node, const char *key) {
	es_size_t size;
	int type;
	void* v = eso_bson_attr_get_bin(node, key, &size, &type);
	if (!v) {
		throw ENumberFormatException(__FILE__, __LINE__);
	}
	switch (type) {
	case BSON_NODE_DATA_TYPE_STRING:
		return EDouble::parseDouble((char*)v);
	case BSON_NODE_DATA_TYPE_BINARY:
	{
		if (size > sizeof(double)) {
			throw ENumberFormatException(__FILE__, __LINE__);
		}
		es_int64_t i = eso_array2llong((es_byte_t*)v, size);
		return eso_llongBits2double(i);
	}
	case BSON_NODE_DATA_TYPE_I8:
	case BSON_NODE_DATA_TYPE_I16:
	case BSON_NODE_DATA_TYPE_I32:
	case BSON_NODE_DATA_TYPE_I64:
		return eso_array2llong((es_byte_t*)v, size);
	case BSON_NODE_DATA_TYPE_FLOAT:
	{
		es_int32_t i = eso_array2llong((es_byte_t*)v, size);
		return eso_intBits2float(i);
	}
	case BSON_NODE_DATA_TYPE_DOUBLE:
	{
		es_int64_t i = eso_array2llong((es_byte_t*)v, size);
		return eso_llongBits2double(i);
	}
	case BSON_NODE_DATA_TYPE_VARINT:
	{
		es_uint64_t l;
		eso_arrayToVarint64((es_uint8_t*)v, &l);
		return l;
	}
	default:
		throw ENumberFormatException(__FILE__, __LINE__);
	}
}


EString EBson::attrGetString(es_bson_node_t *node, const char *key, const char* defaultValue) {
	try {
		return attrGetString(node, key);
	} catch (...) {
		return defaultValue;
	}
	//not reach here.
	ES_ASSERT(false);
	return null;
}

byte EBson::attrGetByte(es_bson_node_t *node, const char *key, byte defaultValue) {
	try {
		return attrGetByte(node, key);
	} catch (...) {
		return defaultValue;
	}
	//not reach here.
	ES_ASSERT(false);
	return 0;
}

short EBson::attrGetShort(es_bson_node_t *node, const char *key, short defaultValue) {
	try {
		return attrGetShort(node, key);
	} catch (...) {
		return defaultValue;
	}
	//not reach here.
	ES_ASSERT(false);
	return 0;
}

int EBson::attrGetInt(es_bson_node_t *node, const char *key, int defaultValue) {
	try {
		return attrGetInt(node, key);
	} catch (...) {
		return defaultValue;
	}
	//not reach here.
	ES_ASSERT(false);
	return 0;
}

llong EBson::attrGetLLong(es_bson_node_t *node, const char *key, llong defaultValue) {
	try {
		return attrGetLLong(node, key);
	} catch (...) {
		return defaultValue;
	}
	//not reach here.
	ES_ASSERT(false);
	return 0;
}

float EBson::attrGetFloat(es_bson_node_t *node, const char *key, float defaultValue) {
	try {
		return attrGetFloat(node, key);
	} catch (...) {
		return defaultValue;
	}
	//not reach here.
	ES_ASSERT(false);
	return 0;
}

double EBson::attrGetDouble(es_bson_node_t *node, const char *key, double defaultValue) {
	try {
		return attrGetDouble(node, key);
	} catch (...) {
		return defaultValue;
	}
	//not reach here.
	ES_ASSERT(false);
	return 0;
}

es_status_t EBson::attrDelete(es_bson_node_t *node, const char *key) {
	return eso_bson_attr_del(node, key);
}

void EBson::attrClear(es_bson_node_t *node) {
	eso_bson_attr_clear(node);
}

void EBson::load(const char *fname) THROWS2(EIOException,ERuntimeException)
{
	es_status_t rv = eso_bson_load(m_bson, fname);
	if (rv != ES_SUCCESS) {
		if (rv == ES_IOERROR)
			throw EIOException(__FILE__, __LINE__);
		else
			throw ERuntimeException(__FILE__, __LINE__);
	}
}

void EBson::save(const char *fname, const char *node) THROWS2(EIOException,ERuntimeException)
{
	es_status_t rv = eso_bson_save(m_bson, fname, node);
	if (rv != ES_SUCCESS) {
		if (rv == ES_IOERROR)
			throw EIOException(__FILE__, __LINE__);
		else
			throw ERuntimeException(__FILE__, __LINE__);
	}
}

static void bson_to_string(EString& strbuf, es_bson_node_t* node) {
	if (!node) return;

	while (node) {
		for (int i = 1; i < eso_bson_node_level(node); i++) {
			strbuf.append("\t");
		}

		strbuf.append(node->name).append("-->").append(EBson::nodeGetString(node)).append("\n");

		if (node->child0) {
			bson_to_string(strbuf, node->child0);
		}

		node = node->next;
	}
}

EString EBson::toString() {
	EString strbuf;
	es_bson_node_t *node = m_bson->root;
	if (node) {
		node = node->child0;
	}
	bson_to_string(strbuf, node);
	return strbuf;
}

} /* namespace efc */
