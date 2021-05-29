#include "ESimpleStack.hh"

namespace efc {

void ESimpleStack::push(EObject* item)
{
	pushElement(item);
}

EObject* ESimpleStack::pop()
{
	return popElement();
}

EObject* ESimpleStack::peek()
{
	int len = size();

	if (len == 0) {
	    return NULL;
	}
	
	return elementAt(len - 1);
}

boolean ESimpleStack::empty()
{
	return size() == 0;
}

int ESimpleStack::search(EObject *o)
{
	int i = lastIndexOf(o);

	if (i >= 0) {
	    return size() - i - 1;
	}

	return -1;
}

} /* namespace efc */
