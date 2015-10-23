#ifndef __ESimpleStack_H__
#define __ESimpleStack_H__

#include "ESimpleVector.hh"

namespace efc {

class ESimpleStack : public ESimpleVector
{
public:
	//virtual ~ESimpleStack(){}

	/**
     * Pushes an item onto the top of this stack.
     */
	void                    push(EObject* item);
	
	/**
     * Removes the object at the top of this stack and returns that 
     * object as the value of this function. 
     */
	EObject*                pop();
	
	/**
     * Looks at the object at the top of this stack without removing it 
     * from the stack. 
     */
    EObject*                peek();
    
    /**
     * Tests if this stack is empty.
     */
    es_bool_t               empty();
    
    /**
     * Returns the 0-based position where an object is on this stack. 
     * If the object <tt>o</tt> occurs as an item in this stack, this 
     * method returns the distance from the top of the stack of the 
     * occurrence nearest the top of the stack; the topmost item on the 
     * stack is considered to be at distance <tt>0</tt>. The <tt>equals</tt> 
     * method is used to compare <tt>o</tt> to the 
     * items in this stack.
     *
     * @param   o   the desired object.
     * @return  the 1-based position from the top of the stack where 
     *          the object is located; the return value <code>-1</code>
     *          indicates that the object is not on the stack.
     */
    es_int32_t              search(EObject *o);
};

} /* namespace efc */
#endif //!__ESimpleStack_H__
