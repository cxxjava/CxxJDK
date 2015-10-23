#ifndef __ESimpleVector_H__
#define __ESimpleVector_H__

#include "EObject.hh"
#include "ESimpleEnumeration.hh"
#include "EUnsupportedOperationException.hh"

namespace efc {

/**
 *	EObject *pobj;
 *	for (EEnumeration* e = vector->elements(); e->hasMoreElements();) {
 *		pobj = e->nextElement();
 *		//...
 *	}
 */

class ESimpleVector : virtual public ESimpleEnumeration {
public:
	ESimpleVector(int size = 10, boolean autofree = TRUE);
	virtual ~ESimpleVector();

public:
	//Returns an enumeration of the components of this vector.
	ESimpleEnumeration* elements();

	void setAutoFree(boolean autofree = TRUE);
	boolean getAutoFree();
	void pushElement(const EObject* obj);
	EObject* popElement();
	boolean contains(const EObject* obj);
	EObject* elementAt(int index);
	void insertElementAt(const EObject* obj, int index);
	boolean isEmpty();
	void removeAllElements();
	boolean removeElement(const EObject* obj);
	EObject* removeElementAt(int index);
	void setElementAt(const EObject* obj, int index);
	void resize(int newSize);
	int size();
	int indexOf(const EObject* obj);
	int indexOf(const EObject* obj, int index);
	int lastIndexOf(const EObject* obj);
	int lastIndexOf(const EObject* obj, int index);

	ESimpleVector& concat(ESimpleVector* vector);

protected:
	es_array_t* m_arrayBuffer;boolean m_autoFree;
	int m_vectorIndex;

private:
	boolean hasMoreElements();
	void nextElement(void* element) {
		throw EUNSUPPORTEDOPERATIONEXCEPTION;
	}
	EObject* nextElement();
};

} /* namespace efc */
#endif //!__ESimpleVector_H__
