#ifndef __EObject_H__
#define __EObject_H__

#include "EBase.hh"
#include "EStringBase.hh"

namespace efc {

//=============================================================================

//placement new class: NEWC(classT)(arg1, arg2);
#define NEWC(T) new (eso_calloc(sizeof(T))) T

//placement delete class
template<typename T>
inline void DELC(T*& p)
{
	if (!p)
		return;
	p->~T();
	eso_free(p);
}

//placement new[] class: classT *t; NEWMC(t, m);
template<typename T, typename M>
inline void NEWMC(T*& t, M m)
{
	int msize = sizeof(T) * m;
	int hsize = sizeof(int);
	int i, n;
	T *tmp;

	void *p = eso_calloc(hsize + msize);
	n = (int)m;
	eso_memcpy(p, &n, hsize);
	for (i = 0; i < m; i++) {
		tmp = new ((char *)p + i*sizeof(T) + hsize) T;
	}
	t = (T *)((char *)p + hsize);
}

//placement delete[] class
template<typename T>
inline void DELMC(T*& p)
{
	int i, m;
	void *p0;
	T *tmp;

	if (!p)
		return;

	p0 = (char *)p - sizeof(int);
	eso_memcpy(&m, p0, sizeof(int));
	for (i = 0; i < m; i++) {
		tmp = (T *)((char *)p + i*sizeof(T));
		tmp->~T();
	}

	eso_free(p0);
}

//=============================================================================

#define DELETE(pptr) \
	do { \
		if (pptr && *pptr) {\
			delete (*pptr);\
			*pptr = NULL; \
		} \
	} while (0)

//=============================================================================

template <class Class, typename T>
inline boolean
instanceof(T *object)
{
    return dynamic_cast<Class const *>(object) ? true : false;
}

template <class Class, typename T>
inline boolean
instanceof(T &object)
{
    return dynamic_cast<Class const *>(&object) ? true : false;
}

//=============================================================================

//@see: STL iterator traits && http://blog.csdn.net/xuqingict/article/details/38453929

template<class T>
struct ETraits
{
    typedef typename T::valueType valueType;
    typedef typename T::differenceType differenceType;
    typedef typename T::pointer pointer;
    typedef typename T::reference reference;
};

template<class T>
struct ETraits<T*>
{
    typedef T valueType;
    typedef ptrdiff_t differenceType;
    typedef T* pointer;
    typedef T & reference;
};

template<class T>
struct ETraits<const T*>
{
    typedef T valueType;
    typedef ptrdiff_t differenceType;
    typedef const T* pointer;
    typedef const T & reference;
};

//=============================================================================

class EObject
{
public:
	EObject();
	virtual ~EObject();
	
	/**
	 * Returns a hash code value for the object. This method is
	 * supported for the benefit of hashtables such as those provided by
	 * <code>java.util.Hashtable</code>.
	 * <p>
	 * The general contract of <code>hashCode</code> is:
	 * <ul>
	 * <li>Whenever it is invoked on the same object more than once during
	 *     an execution of a Java application, the <tt>hashCode</tt> method
	 *     must consistently return the same integer, provided no information
	 *     used in <tt>equals</tt> comparisons on the object is modified.
	 *     This integer need not remain consistent from one execution of an
	 *     application to another execution of the same application.
	 * <li>If two objects are equal according to the <tt>equals(Object)</tt>
	 *     method, then calling the <code>hashCode</code> method on each of
	 *     the two objects must produce the same integer result.
	 * <li>It is <em>not</em> required that if two objects are unequal
	 *     according to the {@link java.lang.Object#equals(java.lang.Object)}
	 *     method, then calling the <tt>hashCode</tt> method on each of the
	 *     two objects must produce distinct integer results.  However, the
	 *     programmer should be aware that producing distinct integer results
	 *     for unequal objects may improve the performance of hashtables.
	 * </ul>
	 * <p>
	 * As much as is reasonably practical, the hashCode method defined by
	 * class <tt>Object</tt> does return distinct integers for distinct
	 * objects. (This is typically implemented by converting the internal
	 * address of the object into an integer, but this implementation
	 * technique is not required by the
	 * Java<font size="-2"><sup>TM</sup></font> programming language.)
	 *
	 * @return  a hash code value for this object.
	 * @see     java.lang.Object#equals(java.lang.Object)
	 * @see     java.util.Hashtable
	 */
	virtual int hashCode();

	/**
	 * Indicates whether some other object is "equal to" this one.
	 * <p>
	 * The <code>equals</code> method implements an equivalence relation
	 * on non-null object references:
	 * <ul>
	 * <li>It is <i>reflexive</i>: for any non-null reference value
	 *     <code>x</code>, <code>x.equals(x)</code> should return
	 *     <code>true</code>.
	 * <li>It is <i>symmetric</i>: for any non-null reference values
	 *     <code>x</code> and <code>y</code>, <code>x.equals(y)</code>
	 *     should return <code>true</code> if and only if
	 *     <code>y.equals(x)</code> returns <code>true</code>.
	 * <li>It is <i>transitive</i>: for any non-null reference values
	 *     <code>x</code>, <code>y</code>, and <code>z</code>, if
	 *     <code>x.equals(y)</code> returns <code>true</code> and
	 *     <code>y.equals(z)</code> returns <code>true</code>, then
	 *     <code>x.equals(z)</code> should return <code>true</code>.
	 * <li>It is <i>consistent</i>: for any non-null reference values
	 *     <code>x</code> and <code>y</code>, multiple invocations of
	 *     <tt>x.equals(y)</tt> consistently return <code>true</code>
	 *     or consistently return <code>false</code>, provided no
	 *     information used in <code>equals</code> comparisons on the
	 *     objects is modified.
	 * <li>For any non-null reference value <code>x</code>,
	 *     <code>x.equals(null)</code> should return <code>false</code>.
	 * </ul>
	 * <p>
	 * The <tt>equals</tt> method for class <code>Object</code> implements
	 * the most discriminating possible equivalence relation on objects;
	 * that is, for any non-null reference values <code>x</code> and
	 * <code>y</code>, this method returns <code>true</code> if and only
	 * if <code>x</code> and <code>y</code> refer to the same object
	 * (<code>x == y</code> has the value <code>true</code>).
	 * <p>
	 * Note that it is generally necessary to override the <tt>hashCode</tt>
	 * method whenever this method is overridden, so as to maintain the
	 * general contract for the <tt>hashCode</tt> method, which states
	 * that equal objects must have equal hash codes.
	 *
	 * @param   obj   the reference object with which to compare.
	 * @return  <code>true</code> if this object is the same as the obj
	 *          argument; <code>false</code> otherwise.
	 * @see     #hashCode()
	 * @see     java.util.Hashtable
	 */
	virtual boolean equals(EObject* obj);

	/**
	 * Returns a string representation of the object. In general, the
	 * {@code toString} method returns a string that
	 * "textually represents" this object. The result should
	 * be a concise but informative representation that is easy for a
	 * person to read.
	 * It is recommended that all subclasses override this method.
	 * <p>
	 * The {@code toString} method for class {@code Object}
	 * returns a string consisting of the name of the class of which the
	 * object is an instance, the at-sign character `{@code @}', and
	 * the unsigned hexadecimal representation of the hash code of the
	 * object. In other words, this method returns a string equal to the
	 * value of:
	 * <blockquote>
	 * <pre>
	 * getClass().getName() + '@' + Integer.toHexString(hashCode())
	 * </pre></blockquote>
	 *
	 * @return  a string representation of the object.
	 */
	virtual EStringBase toString();
};

} /* namespace efc */


//=============================================================================

#if 0
inline void *operator new(size_t size)
{
	return (void*)eso_calloc(size);
}

inline void *operator new[](size_t size)
{
	return eso_calloc(size);
}

inline void operator delete(void *p)
{
	if (p) eso_free(p);
}

inline void operator delete[](void *p)
{
	 if (p) eso_free(p);
}

inline void operator delete(void *p, const char *file, int line)
{
	ES_UNUSED(file);
	ES_UNUSED(line);
    if (p) eso_free(p);
}

inline void operator delete[](void *p, const char *file, int line)
{
	ES_UNUSED(file);
	ES_UNUSED(line);
    if (p) eso_free(p);
}
#endif

#endif //!__EObject_H__
