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
	int key_int;
	char *key_str;
	EObject *data;
};

typedef struct es_emap_t es_emap_t;

class ESimpleMap : virtual public ESimpleEnumeration {
public:
	~ESimpleMap();
	ESimpleMap(boolean autofree = TRUE, boolean uniqueKey = TRUE,
			uint initialCapacity = 32);

	//TODO:
	ESimpleMap(const ESimpleMap& that);
	ESimpleMap& operator= (const ESimpleMap& that);

public:
	ESimpleEnumeration* elements();

	void setAutoFree(boolean autofree = TRUE);
    boolean getAutoFree();
	int put(const char* key, const EObject* value);
	int put(int key, const EObject* value);
	EObject* get(const char* key, int index=0); //if (index == -1) then get the last one.
	EObject* get(int key, int index=0); //if (index == -1) then get the last one.
	int remove(const char* key, int index=0);
	int remove(int key, int index=0);
	int insert(const char* key, const EObject* value, int index);
	int insert(int key, const EObject* value, int index);
	int update(const char* key, const EObject* value, int index=0);
	int update(int key, const EObject* value, int index=0);

	EObject* elementAt(int index);
	int indexOf(int key);
	int indexOf(const char* key);

	void clear();
	boolean isEmpty();
	boolean isUniqueKey();
	int size();
	void sort(boolean onASC = TRUE);

private:
	boolean m_uniqueKey;
	boolean m_autoFree;
	uint m_initialCapacity;
	int m_items;

	es_emap_t* m_emap;
	es_emap_elem_t* m_emapElem;

	boolean hasMoreElements();
	void nextElement(void* element);
	EObject* nextElement() {
		throw EUNSUPPORTEDOPERATIONEXCEPTION;
	}
};

} /* namespace efc */
#endif //!__ESimpleMap_H__
