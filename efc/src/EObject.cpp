#include <stddef.h>
#include <typeinfo>
#include "EString.hh"

namespace efc {

EObject::EObject()
{
}

EObject::~EObject()
{
}

boolean EObject::equals(EObject* obj)
{
	return obj ? (((void*)this == (void*)obj) ? true : false) : false;
}

/**
 * The general contract of hashCode is:
 * Whenever it is invoked on the same object more than once during an execution of a Java application,
 * the hashCode method must consistently return the same integer, provided no information used in equals
 * comparisons on the object is modified. This integer need not remain consistent from one execution of
 * an application to another execution of the same application.
 * If two objects are equal according to the equals(Object) method, then calling the hashCode method on
 * each of the two objects must produce the same integer result.
 * It is not required that if two objects are unequal according to the equals(java.lang.Object) method,
 * then calling the hashCode method on each of the two objects must produce distinct integer results.
 * However, the programmer should be aware that producing distinct integer results for unequal objects
 * may improve the performance of hashtables.
 */
int EObject::hashCode()
{
	es_size_t v = (es_size_t)this;
	return (int)(v ^ (v >> sizeof(es_size_t)));
}

EString EObject::toString() {
	char s[256] = {};
	eso_snprintf(s, sizeof(s), "%s@%d", typeid(*this).name(), hashCode());
	return s;
}

} /* namespace efc */
