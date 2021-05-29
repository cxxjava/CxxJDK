#include "ESimpleMap.hh"

namespace efc {

ES_RING_HEAD(es_emap_t, es_emap_elem_t);

ESimpleMap::ESimpleMap(boolean autofree, boolean uniqueKey, uint initialCapacity)
{
	m_uniqueKey = uniqueKey;
	m_autoFree = autofree;
	m_initialCapacity = initialCapacity;
	m_items = 0;
	m_emapElem = NULL;
	
	m_emap = (es_emap_t*)eso_calloc(sizeof(es_emap_t));
	ES_RING_INIT(m_emap, es_emap_elem_t, link);
	m_emap->data = 0;
}

ESimpleMap::~ESimpleMap()
{
	clear();
	eso_free(m_emap);
}

void ESimpleMap::setAutoFree(boolean autofree)
{
	m_autoFree = autofree;
}

boolean ESimpleMap::getAutoFree()
{
	return m_autoFree;
}

int ESimpleMap::put(const char* key, const EObject* value)
{
	es_emap_elem_t *ep;
	
	if (!key || !*key) return -1;
	
	if (m_uniqueKey && indexOf(key)>=0) {
		return update(key, value);
	}
	else {
		ep = (es_emap_elem_t *)eso_calloc(sizeof(es_emap_elem_t));
    	ES_RING_ELEM_INIT(ep, link);
    	
    	ep->key_str = (char *)eso_calloc(eso_strlen(key)+1);
    	eso_strcpy(ep->key_str, key);
    	ep->data = (EObject*)value;
    	ES_RING_INSERT_TAIL(m_emap, ep, es_emap_elem_t, link);
    	
    	m_items++;
    	
    	return 0;
    }
}

int ESimpleMap::put(int key, const EObject* value)
{
	es_emap_elem_t *ep;
	
	if (m_uniqueKey && indexOf(key)>=0) {
		return update(key, value);
	}
	else {
		ep = (es_emap_elem_t *)eso_calloc(sizeof(es_emap_elem_t));
    	ES_RING_ELEM_INIT(ep, link);
    	
    	ep->key_int = key;
    	ep->data = (EObject*)value;
    	ES_RING_INSERT_TAIL(m_emap, ep, es_emap_elem_t, link);
    	
    	m_items++;
    	
    	return 0;
    }
}

EObject* ESimpleMap::get(const char* key, int index)
{
	es_emap_elem_t *ep;
	EObject *data = NULL;
	int n = 0;
	
	if (!key || !*key || index < -1) return NULL;
	
	if (!ES_RING_EMPTY(m_emap, es_emap_elem_t, link)) {
		if (index == -1) { //get the last
			ep = ES_RING_LAST(m_emap);
			while (ep != ES_RING_SENTINEL(m_emap, es_emap_elem_t, link)) {
				if (ep->key_str && (eso_strcmp(ep->key_str, key) == 0)) {
					data = ep->data;
					break;
				}
				ep = ES_RING_PREV(ep, link);
			}
		}
		else {
			ep = ES_RING_FIRST(m_emap);
			while (ep != ES_RING_SENTINEL(m_emap, es_emap_elem_t, link)) {
				if (ep->key_str && (eso_strcmp(ep->key_str, key) == 0) && (n++ == index)) {
					data = ep->data;
					break;
				}
				ep = ES_RING_NEXT(ep, link);
			}
		}
    }
    
    return data;
}

EObject* ESimpleMap::get(int key, int index)
{
	es_emap_elem_t *ep;
	EObject *data = NULL;
	int n = 0;
	
	if (index < -1) return NULL;

	if (!ES_RING_EMPTY(m_emap, es_emap_elem_t, link)) {
		if (index == -1) { //get the last
			ep = ES_RING_LAST(m_emap);
			while (ep != ES_RING_SENTINEL(m_emap, es_emap_elem_t, link)) {
				if (!ep->key_str && ep->key_int == key) {
					data = ep->data;
					break;
				}
				ep = ES_RING_PREV(ep, link);
			}
		}
		else {
			ep = ES_RING_FIRST(m_emap);
			while (ep != ES_RING_SENTINEL(m_emap, es_emap_elem_t, link)) {
				if (!ep->key_str && ep->key_int == key && (n++ == index)) {
					data = ep->data;
					break;
				}
				ep = ES_RING_NEXT(ep, link);
			}
		}
    }
    
    return data;
}

int ESimpleMap::remove(const char* key, int index)
{
	es_emap_elem_t *ep;
	int n = 0;
	
	if (!key || !*key || index < 0) return -1;
	
	if (isEmpty()) return 0;
	
	ep = ES_RING_FIRST(m_emap);
	while (ep != ES_RING_SENTINEL(m_emap, es_emap_elem_t, link)) {
		if (ep->key_str && (eso_strcmp(ep->key_str, key) == 0) && (n++ == index)) {
			if (m_autoFree) {
				if (ep->data) delete ep->data;
			}
			eso_free(ep->key_str); //free key
			ES_RING_REMOVE(ep, link);
            
            es_emap_elem_t *t = ep;
            ep = ES_RING_NEXT(ep, link);
			eso_free(t);
			
			m_items--;
			
			if (m_uniqueKey) break;
		} else {
            ep = ES_RING_NEXT(ep, link);
        }
	}
	
	return 0;
}

int ESimpleMap::remove(int key, int index)
{
	es_emap_elem_t *ep;
	int n = 0;
	
	if (index < 0) return -1;
	if (isEmpty()) return 0;
	
	ep = ES_RING_FIRST(m_emap);
	while (ep != ES_RING_SENTINEL(m_emap, es_emap_elem_t, link)) {
		if (!ep->key_str && ep->key_int == key && (n++ == index)) {
			if (m_autoFree) {
				if (ep->data) delete ep->data;
			}
            
            ES_RING_REMOVE(ep, link);
            
            es_emap_elem_t *t = ep;
            ep = ES_RING_NEXT(ep, link);
			eso_free(t);
			
			m_items--;
			
			if (m_uniqueKey) break;
		} else {
            ep = ES_RING_NEXT(ep, link);
        }
	}
	
	return 0;
}

int ESimpleMap::insert(const char* key, const EObject* value, int index)
{
	es_emap_elem_t *ep, *lep;
	int count = 0;
	
	if (!key || !*key || index<0 || index>=size()) {
		return -1;
	}
	
	if (m_uniqueKey && indexOf(key) >= 0) {
		return -1;
	}
	
	ep = ES_RING_FIRST(m_emap);
	while (ep != ES_RING_SENTINEL(m_emap, es_emap_elem_t, link)) {
		if (index == count) {
			lep = ep;

			ep = (es_emap_elem_t *)eso_calloc(sizeof(es_emap_elem_t));
			ES_RING_ELEM_INIT(ep, link);
			
			ep->key_str = (char*)eso_calloc(eso_strlen(key)+1);
			eso_strcpy(ep->key_str, key);
			ep->data = (EObject *)value;
			ES_RING_INSERT_BEFORE(lep, ep, link);
			
			m_items++;
			
			break;
		}
		ep = ES_RING_NEXT(ep, link);
		count++;
	}
	
	return 0;
}

int ESimpleMap::insert(int key, const EObject* value, int index)
{
	es_emap_elem_t *ep, *lep, *nep;
	int count = 0;
	
	if (index<0 || index>=size()) {
		return -1;
	}
	
	if (m_uniqueKey && indexOf(key) >= 0) {
		return -1;
	}
	
	ep = ES_RING_FIRST(m_emap);
	while (ep != ES_RING_SENTINEL(m_emap, es_emap_elem_t, link)) {
		if (index == count) {
			lep = ep;
			nep = ES_RING_NEXT(ep, link);
			
			ep = (es_emap_elem_t *)eso_calloc(sizeof(es_emap_elem_t));
			ES_RING_ELEM_INIT(ep, link);
			
			ep->key_int = key;
			ep->data = (EObject *)value;
			ES_RING_INSERT_BEFORE(lep, nep, link);
			
			m_items++;
			
			break;
		}
		ep = ES_RING_NEXT(ep, link);
		count++;
	}
	
	return 0;
}

int ESimpleMap::update(const char* key, const EObject* value, int index)
{
	es_emap_elem_t *ep;
	int updated = -1;
	int n = 0;
	
	if (!key || !*key || index<0) {
		return -1;
	}
	
	ep = ES_RING_FIRST(m_emap);
	while (ep != ES_RING_SENTINEL(m_emap, es_emap_elem_t, link)) {
		if (ep->key_str && (eso_strcmp(ep->key_str, key) == 0) && (n++ == index)) {
			if (m_autoFree) {
				if (ep->data) delete ep->data;
			}
			ep->data = (EObject*)value;
			
			updated = 0;
			
			break;
		}
		ep = ES_RING_NEXT(ep, link);
	}
	
	return updated;
}

int ESimpleMap::update(int key, const EObject* value, int index)
{
	es_emap_elem_t *ep;
	int updated;
	int n = 0;
	
	if (index<0) return -1;

	updated = -1;
	ep = ES_RING_FIRST(m_emap);
	while (ep != ES_RING_SENTINEL(m_emap, es_emap_elem_t, link)) {
		if (!ep->key_str && ep->key_int == key && (n++ == index)) {
			if (m_autoFree) {
				if (ep->data) delete ep->data;
			}
			ep->data = (EObject*)value;
			
			updated = 0;
			
			break;
		}
		ep = ES_RING_NEXT(ep, link);
	}
	
	return updated;
}

EObject* ESimpleMap::elementAt(int index)
{
	es_emap_elem_t *ep;
	int count = 0;
	
	if (index<0 || index>=size()) {
		return NULL;
	}
	
	ep = ES_RING_FIRST(m_emap);
	while (ep != ES_RING_SENTINEL(m_emap, es_emap_elem_t, link)) {
		if (index == count) {
			return ep->data;
		}
		ep = ES_RING_NEXT(ep, link);
		count++;
	}
	
	return NULL;
}

int ESimpleMap::indexOf(const char* key)
{
	es_emap_elem_t *ep;
	int count = 0;
	
	if (!key || !*key) return -1;
	
	ep = ES_RING_FIRST(m_emap);
	while (ep != ES_RING_SENTINEL(m_emap, es_emap_elem_t, link)) {
		if (ep->key_str && (eso_strcmp(ep->key_str, key) == 0)) {
			return count;
		}
		ep = ES_RING_NEXT(ep, link);
		count++;
	}
	
	return -1;
}

int ESimpleMap::indexOf(int key)
{
	es_emap_elem_t *ep;
	int count = 0;
	
	ep = ES_RING_FIRST(m_emap);
	while (ep != ES_RING_SENTINEL(m_emap, es_emap_elem_t, link)) {
		if (!ep->key_str && ep->key_int == key) {
			return count;
		}
		ep = ES_RING_NEXT(ep, link);
		count++;
	}
	
	return -1;
}

void ESimpleMap::clear()
{
	es_emap_elem_t *ep, *eptmp;
	
	if (!ES_RING_EMPTY(m_emap, es_emap_elem_t, link)) {
        ep = ES_RING_FIRST(m_emap);
        while (ep != ES_RING_SENTINEL(m_emap, es_emap_elem_t, link)) {
			eptmp = ES_RING_NEXT(ep, link);
			if (m_autoFree) {
				if (ep->data) delete ep->data;
			}
			eso_free(ep->key_str); //free key
			ES_RING_REMOVE(ep, link);
			eso_free(ep);
			ep = eptmp;
		}
    }
    m_items = 0;
}

boolean ESimpleMap::isEmpty()
{
	return ES_RING_EMPTY(m_emap, es_emap_elem_t, link);
}

boolean ESimpleMap::isUniqueKey()
{
	return m_uniqueKey;
}

int ESimpleMap::size()
{
	return m_items;
}

void ESimpleMap::sort(boolean onASC)
{
	if (onASC) {
		//TODO...
	}
	else {
		//TODO...
	}
}

boolean ESimpleMap::hasMoreElements()
{
	return m_emapElem ? TRUE : FALSE;
}

void ESimpleMap::nextElement(void* element)
{
	if (m_emapElem && element) {
		es_emap_elem_t *item = (es_emap_elem_t*)element;
		item->data = m_emapElem->data;
		item->key_int = m_emapElem->key_int;
		item->key_str = m_emapElem->key_str;
		item->link.next = m_emapElem->link.next;
		item->link.prev = m_emapElem->link.prev;

		//next
		m_emapElem = ES_RING_NEXT(m_emapElem, link);
		if (m_emapElem == ES_RING_SENTINEL(m_emap, es_emap_elem_t, link)) {
			m_emapElem = NULL;
		}
	}
}

ESimpleEnumeration* ESimpleMap::elements()
{
	if (ES_RING_EMPTY(m_emap, es_emap_elem_t, link)) {
		m_emapElem = NULL;
	}
	else {
		m_emapElem = ES_RING_FIRST(m_emap);
	}
	return (ESimpleEnumeration*)this;
}

} /* namespace efc */
