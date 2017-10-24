#ifndef __EString_HH__
#define __EString_HH__

#include "EStringBase.hh"
#include "EComparable.hh"

namespace efc {

class EString : public EStringBase, virtual public EComparable<EString*>
{
public:
	virtual ~EString() {};

	EString() : EStringBase() {};
	EString(es_nullptr_t p) : EStringBase(p) {};
	EString(const char* cstr) : EStringBase(cstr) {};
	EString(const char* cstr, uint off, uint len) : EStringBase(cstr, off, len) {};
	EString(const EString* estr) : EStringBase(estr) {};
	EString(const EString& estr) : EStringBase(estr) {};
	EString(const EString& estr, uint off, uint len) : EStringBase(estr, off, len) {};
	explicit EString(const char c) : EStringBase(c) {};
	explicit EString(short i, int radix = 10) : EStringBase(i, radix) {};
	explicit EString(ushort i, int radix = 10) : EStringBase(i, radix) {};
	explicit EString(int i, int radix = 10) : EStringBase(i, radix) {};
	explicit EString(uint i, int radix = 10) : EStringBase(i, radix) {};
	explicit EString(long l, int radix = 10) : EStringBase(l, radix) {};
	explicit EString(ulong l, int radix = 10) : EStringBase(l, radix) {};
	explicit EString(llong l, int radix = 10) : EStringBase(l, radix) {};
	explicit EString(ullong l, int radix = 10) : EStringBase(l, radix) {};

	EString(const EStringBase& estr) : EStringBase(estr) {};

public:
	/**
	 * Compares two strings lexicographically.
	 * The comparison is based on the Unicode value of each character in
	 * the strings. The character sequence represented by this
	 * <code>String</code> object is compared lexicographically to the
	 * character sequence represented by the argument string. The result is
	 * a negative integer if this <code>String</code> object
	 * lexicographically precedes the argument string. The result is a
	 * positive integer if this <code>String</code> object lexicographically
	 * follows the argument string. The result is zero if the strings
	 * are equal; <code>compareTo</code> returns <code>0</code> exactly when
	 * the {@link #equals(Object)} method would return <code>true</code>.
	 * <p>
	 * This is the definition of lexicographic ordering. If two strings are
	 * different, then either they have different characters at some index
	 * that is a valid index for both strings, or their lengths are different,
	 * or both. If they have different characters at one or more index
	 * positions, let <i>k</i> be the smallest such index; then the string
	 * whose character at position <i>k</i> has the smaller value, as
	 * determined by using the &lt; operator, lexicographically precedes the
	 * other string. In this case, <code>compareTo</code> returns the
	 * difference of the two character values at position <code>k</code> in
	 * the two string -- that is, the value:
	 * <blockquote><pre>
	 * this.charAt(k)-anotherString.charAt(k)
	 * </pre></blockquote>
	 * If there is no index position at which they differ, then the shorter
	 * string lexicographically precedes the longer string. In this case,
	 * <code>compareTo</code> returns the difference of the lengths of the
	 * strings -- that is, the value:
	 * <blockquote><pre>
	 * this.length()-anotherString.length()
	 * </pre></blockquote>
	 *
	 * @param   anotherString   the <code>String</code> to be compared.
	 * @return  the value <code>0</code> if the argument string is equal to
	 *          this string; a value less than <code>0</code> if this string
	 *          is lexicographically less than the string argument; and a
	 *          value greater than <code>0</code> if this string is
	 *          lexicographically greater than the string argument.
	 */
	virtual int compareTo(EString* anotherString);

	/**
	 * Compares two strings lexicographically, ignoring case
	 * differences. This method returns an integer whose sign is that of
	 * calling <code>compareTo</code> with normalized versions of the strings
	 * where case differences have been eliminated by calling
	 * <code>Character.toLowerCase(Character.toUpperCase(character))</code> on
	 * each character.
	 * <p>
	 * Note that this method does <em>not</em> take locale into account,
	 * and will result in an unsatisfactory ordering for certain locales.
	 * The java.text package provides <em>collators</em> to allow
	 * locale-sensitive ordering.
	 *
	 * @param   str   the <code>String</code> to be compared.
	 * @return  a negative integer, zero, or a positive integer as the
	 *          specified String is greater than, equal to, or less
	 *          than this String, ignoring case considerations.
	 * @see     java.text.Collator#compare(String, String)
	 * @since   1.2
	 */
	virtual int compareToIgnoreCase(EString* str);

	boolean equals(EString& anotherString);
	boolean equals(EString* anotherString);
	boolean equals(const char* anotherString);
	virtual boolean equals(EObject* obj);

	virtual EStringBase toString();
	virtual int hashCode();
};

} /* namespace efc */
#endif //!__EString_HH__
