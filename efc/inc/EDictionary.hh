#ifndef __EDictionary_H__
#define __EDictionary_H__

#include "EObject.hh"
#include "EEnumeration.hh"

namespace efc {

/**
 * The <code>Dictionary</code> class is the abstract parent of any 
 * class, such as <code>Hashtable</code>, which maps keys to values. 
 * Every key and every value is an object. In any one <tt>Dictionary</tt> 
 * object, every key is associated with at most one value. Given a 
 * <tt>Dictionary</tt> and a key, the associated element can be looked up. 
 * Any non-<code>null</code> object can be used as a key and as a value.
 * <p>
 * As a rule, the <code>equals</code> method should be used by 
 * implementations of this class to decide if two keys are the same. 
 * <p>
 * <strong>NOTE: This class is obsolete.  New implementations should
 * implement the Map interface, rather than extending this class.</strong>
 *
 * @version 1.24, 11/17/05
 * @see	    java.util.Map
 * @see     java.lang.Object#equals(java.lang.Object)
 * @see     java.lang.Object#hashCode()
 * @see     java.util.Hashtable
 * @since   JDK1.0
 */

template<typename K, typename V>
abstract class EDictionary
{
public:
	/**
	 * Sole constructor.  (For invocation by subclass constructors, typically
	 * implicit.)
	 */
	EDictionary() {
	}

	virtual ~EDictionary(){}
	
protected:
	/**
     * Returns the number of entries (distinct keys) in this dictionary.
     *
     * @return  the number of keys in this dictionary.
     */
	virtual int size() = 0;

	/**
     * Tests if this dictionary maps no keys to value. The general contract 
     * for the <tt>isEmpty</tt> method is that the result is true if and only 
     * if this dictionary contains no entries. 
     *
     * @return  <code>true</code> if this dictionary maps no keys to values;
     *          <code>false</code> otherwise.
     */
	virtual boolean isEmpty() = 0;

	/**
     * Returns an enumeration of the keys in this dictionary. The general 
     * contract for the keys method is that an <tt>Enumeration</tt> object 
     * is returned that will generate all the keys for which this dictionary 
     * contains entries. 
     *
     * @return  an enumeration of the keys in this dictionary.
     * @see     java.util.Dictionary#elements()
     * @see     java.util.Enumeration
     */
    virtual EEnumeration<K>* keys() = 0;
    
    /**
     * Returns an enumeration of the values in this dictionary. The general 
     * contract for the <tt>elements</tt> method is that an 
     * <tt>Enumeration</tt> is returned that will generate all the elements 
     * contained in entries in this dictionary.
     *
     * @return  an enumeration of the values in this dictionary.
     * @see     java.util.Dictionary#keys()
     * @see     java.util.Enumeration
     */
    virtual EEnumeration<V>* elements() = 0;
    
	/**
     * Returns the value to which the key is mapped in this dictionary. 
     * The general contract for the <tt>isEmpty</tt> method is that if this 
     * dictionary contains an entry for the specified key, the associated 
     * value is returned; otherwise, <tt>null</tt> is returned. 
     *
     * @return  the value to which the key is mapped in this dictionary;
     * @param   key   a key in this dictionary.
     *          <code>null</code> if the key is not mapped to any value in
     *          this dictionary.
     * @exception NullPointerException if the <tt>key</tt> is <tt>null</tt>.
     * @see     java.util.Dictionary#put(java.lang.Object, java.lang.Object)
     */
	virtual V get(K key) = 0;

	/**
     * Maps the specified <code>key</code> to the specified 
     * <code>value</code> in this dictionary. Neither the key nor the 
     * value can be <code>null</code>.
     * <p>
     * If this dictionary already contains an entry for the specified 
     * <tt>key</tt>, the value already in this dictionary for that 
     * <tt>key</tt> is returned, after modifying the entry to contain the
     *  new element. <p>If this dictionary does not already have an entry 
     *  for the specified <tt>key</tt>, an entry is created for the 
     *  specified <tt>key</tt> and <tt>value</tt>, and <tt>null</tt> is 
     *  returned.
     * <p>
     * The <code>value</code> can be retrieved by calling the 
     * <code>get</code> method with a <code>key</code> that is equal to 
     * the original <code>key</code>. 
     *
     * @param      key     the hashtable key.
     * @param      value   the value.
     * @return     the previous value to which the <code>key</code> was mapped
     *             in this dictionary, or <code>null</code> if the key did not
     *             have a previous mapping.
     * @exception  NullPointerException  if the <code>key</code> or
     *               <code>value</code> is <code>null</code>.
     * @see        java.lang.Object#equals(java.lang.Object)
     * @see        java.util.Dictionary#get(java.lang.Object)
     */
     virtual V put(K key, V value) = 0;
     
	/**
     * Removes the <code>key</code> (and its corresponding 
     * <code>value</code>) from this dictionary. This method does nothing 
     * if the <code>key</code> is not in this dictionary. 
     *
     * @param   key   the key that needs to be removed.
     * @return  the value to which the <code>key</code> had been mapped in this
     *          dictionary, or <code>null</code> if the key did not have a
     *          mapping.
     * @exception NullPointerException if <tt>key</tt> is <tt>null</tt>.
     */
	virtual V remove(K key) = 0;
};

} /* namespace efc */
#endif //!__EDictionary_H__
