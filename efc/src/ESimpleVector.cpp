#include "ESimpleVector.hh"

namespace efc {

ESimpleVector::ESimpleVector(int size, boolean autofree)
{
	m_arrayBuffer = eso_ptrarray_make(size);
	m_autoFree = autofree;
	m_vectorIndex = 0;
}

ESimpleVector::~ESimpleVector()
{
	removeAllElements();
	eso_ptrarray_free(&m_arrayBuffer);
}

void ESimpleVector::setAutoFree(boolean autofree)
{
	m_autoFree = autofree;
}

boolean ESimpleVector::getAutoFree()
{
	return m_autoFree;
}

void ESimpleVector::pushElement(const EObject* obj)
{
	eso_ptrarray_push(m_arrayBuffer, (void*)obj);
}

EObject* ESimpleVector::popElement()
{
	return (EObject*)eso_ptrarray_pop(m_arrayBuffer);
}

boolean ESimpleVector::contains(const EObject* obj)
{
	return indexOf(obj, 0) >= 0;
}

EObject* ESimpleVector::elementAt(int index)
{
	return (EObject *)eso_ptrarray_get(m_arrayBuffer, index);
}

void ESimpleVector::insertElementAt(const EObject* obj, int index)
{
	eso_ptrarray_insert(m_arrayBuffer, index, (void *)obj);
}

boolean ESimpleVector::isEmpty()
{
	return eso_ptrarray_is_empty(m_arrayBuffer);
}

void ESimpleVector::removeAllElements()
{
	if (m_autoFree) {
		for (int i=0; i<size(); i++) {
			EObject *pObj = (EObject *)eso_ptrarray_get(m_arrayBuffer, i);
			delete pObj;
		}
	}
	eso_ptrarray_clear(m_arrayBuffer);
}

boolean ESimpleVector::removeElement(const EObject* obj)
{
	int i = indexOf(obj);
	if (i >= 0) {
		delete removeElementAt(i);
		return TRUE;
	}
	return FALSE;
}

EObject* ESimpleVector::removeElementAt(int index)
{
	EObject *pObj = (EObject *)eso_ptrarray_get(m_arrayBuffer, index);
	eso_ptrarray_delete(m_arrayBuffer, index);
	return pObj;
}

void ESimpleVector::setElementAt(const EObject* obj, int index)
{
	if (m_autoFree) {
		EObject *pObj = (EObject *)eso_ptrarray_get(m_arrayBuffer, index);
		delete pObj;
	}
	eso_ptrarray_set(m_arrayBuffer, index, (void *)obj);
}

void ESimpleVector::resize(int newSize)
{
	if (newSize > size()) {
		if (m_autoFree) {
			for (int i=newSize; i<size(); i++) {
				EObject *pObj = (EObject *)eso_ptrarray_get(m_arrayBuffer, i);
				delete pObj;
			}
		}
		eso_ptrarray_resize(m_arrayBuffer, newSize);
	}
}

int ESimpleVector::size()
{
	return eso_ptrarray_count(m_arrayBuffer);
}

int ESimpleVector::indexOf(const EObject* obj)
{
	return indexOf(obj, 0);
}

int ESimpleVector::indexOf(const EObject* obj, int index)
{
	if (obj == NULL) {
		for (int i = index; i < size(); i++)
			if (eso_ptrarray_get(m_arrayBuffer, i) == NULL)
				return i;
	}
	else {
		for (int i = index; i < size(); i++)
			if ((EObject *)eso_ptrarray_get(m_arrayBuffer, i) == obj)
				return i;
	}
	return -1;
}

int ESimpleVector::lastIndexOf(const EObject* obj)
{
	return lastIndexOf(obj, size()-1);
}

int ESimpleVector::lastIndexOf(const EObject* obj, int index)
{
	if (index < 0 || index >= size()) return -1;
	
	if (obj == NULL) {
		for (int i = index; i >= 0; i--)
			if (eso_ptrarray_get(m_arrayBuffer, i) == NULL)
				return i;
	}
	else {
		for (int i = index; i >= 0; i--)
			if ((EObject *)eso_ptrarray_get(m_arrayBuffer, i) == obj)
				return i;
	}
	return -1;
}

boolean ESimpleVector::hasMoreElements()
{
	return (m_vectorIndex < size()) ? TRUE : FALSE;
}

EObject* ESimpleVector::nextElement()
{
	return elementAt(m_vectorIndex++);
}

ESimpleEnumeration* ESimpleVector::elements()
{
	m_vectorIndex = 0;
	return (ESimpleEnumeration*)this;
}

ESimpleVector& ESimpleVector::concat(ESimpleVector* vector)
{
	if (vector) {
		eso_ptrarray_append(m_arrayBuffer, vector->m_arrayBuffer);
	}
	return (*this);
}

} /* namespace efc */
