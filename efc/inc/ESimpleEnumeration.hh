#ifndef __ESimpleEnumeration_H__
#define __ESimpleEnumeration_H__

#include "EBase.hh"

namespace efc {

/**
 * for (EEnumeration* e = v.elements(); e->hasMoreElements();) {
 *    //e->nextElement();
 * }
 */

interface ESimpleEnumeration : virtual public EObject
{
	virtual ~ESimpleEnumeration() {}

	//Tests if this enumeration contains more elements.
	virtual boolean       hasMoreElements() = 0;
	//Returns the next element of this enumeration if this enumeration object has at least one more element to provide.
	virtual void            nextElement(void* element) = 0;
	virtual EObject*        nextElement() = 0;
};

} /* namespace efc */
#endif //!__ESimpleEnumeration_H__
