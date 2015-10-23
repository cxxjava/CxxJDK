#ifndef __ESimpleMap_H__
#define __ESimpleMap_H__

#include "EObject.hh"
#include "ESimpleEnumeration.hh"
#include "EUnsupportedOperationException.hh"

namespace efc {

/**
 *	es_emap_elem_t element;
 *	for (EEnumeration* e = map->elements(); e->hasMoreElements();) {
 *		e->nextElement(&element);
 *		//...
 *	}
 */
typedef struct es_emap_elem_t es_emap_elem_t;
struct es_emap_elem_t {
	ES_RING_ENTRY(es_emap_elem_t) link;
	es_int32_t key_int;
	char *key_str;
	EObject *data;
};

typedef struct es_emap_t es_emap_t;

class ESimpleMap : virtual public ESimpleEnumeration {
public:
	ESimpleMap(es_bool_t autofree = TRUE, es_bool_t uniqueKey = TRUE,
			es_uint32_t initialCapacity = 32);
	~ESimpleMap();

public:
	ESimpleEnumeration* elements();

	void setAutoFree(es_bool_t autofree = TRUE);boolean getAutoFree();
	es_int32_t put(const char* key, const EObject* value);
	es_int32_t put(es_int32_t key, const EObject* value);
	EObject* get(const char* key, int index=0); //if (index == -1) then get the last one.
	EObject* get(es_int32_t key, int index=0); //if (index == -1) then get the last one.
	es_int32_t remove(const char* key, int index=0);
	es_int32_t remove(es_int32_t key, int index=0);
	es_int32_t insert(const char* key, const EObject* value, es_int32_t index);
	es_int32_t insert(es_int32_t key, const EObject* value, es_int32_t index);
	es_int32_t update(const char* key, const EObject* value, int index=0);
	es_int32_t update(es_int32_t key, const EObject* value, int index=0);

	EObject* elementAt(es_int32_t index);
	es_int32_t indexOf(es_int32_t key);
	es_int32_t indexOf(const char* key);

	void clear();
	es_bool_t isEmpty();
	es_bool_t isUniqueKey();
	es_int32_t size();
	void sort(es_bool_t onASC = TRUE);

private:
	es_bool_t m_uniqueKey;
	es_bool_t m_autoFree;
	es_uint32_t m_initialCapacity;
	es_int32_t m_items;

	es_emap_t* m_emap;
	es_emap_elem_t* m_emapElem;

	es_bool_t hasMoreElements();
	void nextElement(void* element);
	EObject* nextElement() {
		throw EUNSUPPORTEDOPERATIONEXCEPTION;
	}
};

} /* namespace efc */
#endif //!__ESimpleMap_H__
