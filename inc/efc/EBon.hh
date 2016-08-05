/*
 * EBon.hh
 *
 *  Created on: 2013-3-20
 *      Author: cxxjava@163.com
 */

#ifndef EBON_HH_
#define EBON_HH_

#include "EObject.hh"
#include "ERuntimeException.hh"
#include "EIllegalArgumentException.hh"
#include "EIOException.hh"

namespace efc {

class EBon: public EObject {
public:
	virtual ~EBon();
	EBon(const char *encoding=NULL);

	EBon(const EBon& that);
	EBon& operator= (const EBon& that);

	void clear();
	boolean isEmpty();

	void Import(void *buffer, ulong size);
	void Export(es_buffer_t *buffer, const char *path, boolean useHead);

	void copy(const char *toPath, EBon& from, const char *fromPath) THROWS(ERuntimeException);

	es_bon_node_t* add(const char *path, const char *str);
	es_bon_node_t* add(const char *path, const void *data, es_size_t size);
	es_bon_node_t* addFormat(const char *path, const char *fmt, ...);

	es_bon_node_t* set(const char *path, const char *str);
	es_bon_node_t* set(const char *path, const void *data, es_size_t size);
	es_bon_node_t* setFormat(const char *path, const char *fmt, ...);

	char* get(const char *path);
	void* get(const char *path, es_size_t *size);

	es_status_t del(const char *path);
	es_bon_node_t* find(const char *path);
	es_bon_t* c_bon();

	int indexOf(es_bon_node_t *node);
	int levelOf(es_bon_node_t *node);
	char *pathOf(es_bon_node_t *node, char *path_buf, int buf_size);

	void load(const char *fname) THROWS2(EIOException,ERuntimeException);

	void save(const char *fname, const char *node) THROWS2(EIOException,ERuntimeException);

	static es_status_t nodeDelete(es_bon_node_t *node);
	static es_bon_node_t* nodeUpdate(es_bon_node_t *node, const char *key, const char *str);
	static es_bon_node_t* nodeUpdate(es_bon_node_t *node, const char *key, const void *data, es_size_t size);
	static es_bon_node_t* nodeUpdateFormat(es_bon_node_t *node, const char *key, const char *fmt, ...);

	static es_bon_node_t* attrAdd(es_bon_node_t *node, const char *key, const char *str);
	static es_bon_node_t* attrAdd(es_bon_node_t *node, const char *key, const void *data, es_size_t size);
	static es_bon_node_t* attrAddFormat(es_bon_node_t *node, const char *key, const char *fmt, ...);

	static es_bon_node_t* attrSet(es_bon_node_t *node, const char *key, const char *str);
	static es_bon_node_t* attrSet(es_bon_node_t *node, const char *key, const void *data, es_size_t size);
	static es_bon_node_t* attrSetFormat(es_bon_node_t *node, const char *key, const char *fmt, ...);

	static char* attrGet(es_bon_node_t *node, const char *key);
	static void* attrGet(es_bon_node_t *node, const char *key, es_size_t *size);

	static es_status_t attrDelete(es_bon_node_t *node, const char *key);
	static void attrClear(es_bon_node_t *node);

private:
	es_bon_t *m_bon;
};

} /* namespace efc */
#endif /* EBON_HH_ */
