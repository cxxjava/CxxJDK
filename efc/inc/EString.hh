#ifndef __ESTRING_HH__
#define __ESTRING_HH__

#include "EComparable.hh"
#include <string>

namespace efc {

/**
 * The {@code String} class represents character strings. All
 * string literals in Java programs, such as {@code "abc"}, are
 * implemented as instances of this class.
 * <p>
 * Strings are constant; their values cannot be changed after they
 * are created. String buffers support mutable strings.
 * Because String objects are immutable they can be shared. For example:
 * <blockquote><pre>
 *     String str = "abc";
 * </pre></blockquote><p>
 * is equivalent to:
 * <blockquote><pre>
 *     char data[] = {'a', 'b', 'c'};
 *     String str = new String(data);
 * </pre></blockquote><p>
 * Here are some more examples of how strings can be used:
 * <blockquote><pre>
 *     System.out.println("abc");
 *     String cde = "cde";
 *     System.out.println("abc" + cde);
 *     String c = "abc".substring(2,3);
 *     String d = cde.substring(1, 2);
 * </pre></blockquote>
 * <p>
 * The class {@code String} includes methods for examining
 * individual characters of the sequence, for comparing strings, for
 * searching strings, for extracting substrings, and for creating a
 * copy of a string with all characters translated to uppercase or to
 * lowercase. Case mapping is based on the Unicode Standard version
 * specified by the {@link java.lang.Character Character} class.
 * <p>
 * The Java language provides special support for the string
 * concatenation operator (&nbsp;+&nbsp;), and for conversion of
 * other objects to strings. String concatenation is implemented
 * through the {@code StringBuilder}(or {@code StringBuffer})
 * class and its {@code append} method.
 * String conversions are implemented through the method
 * {@code toString}, defined by {@code Object} and
 * inherited by all classes in Java. For additional information on
 * string concatenation and conversion, see Gosling, Joy, and Steele,
 * <i>The Java Language Specification</i>.
 *
 * <p> Unless otherwise noted, passing a <tt>null</tt> argument to a constructor
 * or method in this class will cause a {@link NullPointerException} to be
 * thrown.
 *
 * <p>A {@code String} represents a string in the UTF-16 format
 * in which <em>supplementary characters</em> are represented by <em>surrogate
 * pairs</em> (see the section <a href="Character.html#unicode">Unicode
 * Character Representations</a> in the {@code Character} class for
 * more information).
 * Index values refer to {@code char} code units, so a supplementary
 * character uses two positions in a {@code String}.
 * <p>The {@code String} class provides methods for dealing with
 * Unicode code points (i.e., characters), in addition to those for
 * dealing with Unicode code units (i.e., {@code char} values).
 *
 * @see     java.lang.Object#toString()
 * @see     java.lang.StringBuffer
 * @see     java.lang.StringBuilder
 * @see     java.nio.charset.Charset
 * @since   JDK1.0
 */

/**
 * A not-thread-safe, mutable sequence of characters.
 * A string buffer is like a {@link String}, but can be modified. At any
 * point in time it contains some particular sequence of characters, but
 * the length and content of the sequence can be changed through certain
 * method calls.
 * <p>
 * String buffers are safe for use by multiple threads. The methods
 * are synchronized where necessary so that all the operations on any
 * particular instance behave as if they occur in some serial order
 * that is consistent with the order of the method calls made by each of
 * the individual threads involved.
 * <p>
 * The principal operations on a {@code StringBuffer} are the
 * {@code append} and {@code insert} methods, which are
 * overloaded so as to accept data of any type. Each effectively
 * converts a given datum to a string and then appends or inserts the
 * characters of that string to the string buffer. The
 * {@code append} method always adds these characters at the end
 * of the buffer; the {@code insert} method adds the characters at
 * a specified point.
 * <p>
 * For example, if {@code z} refers to a string buffer object
 * whose current contents are {@code "start"}, then
 * the method call {@code z.append("le")} would cause the string
 * buffer to contain {@code "startle"}, whereas
 * {@code z.insert(4, "le")} would alter the string buffer to
 * contain {@code "starlet"}.
 * <p>
 * In general, if sb refers to an instance of a {@code StringBuffer},
 * then {@code sb.append(x)} has the same effect as
 * {@code sb.insert(sb.length(), x)}.
 * <p>
 * Whenever an operation occurs involving a source sequence (such as
 * appending or inserting from a source sequence), this class synchronizes
 * only on the string buffer performing the operation, not on the source.
 * Note that while {@code StringBuffer} is designed to be safe to use
 * concurrently from multiple threads, if the constructor or the
 * {@code append} or {@code insert} operation is passed a source sequence
 * that is shared across threads, the calling code must ensure
 * that the operation has a consistent and unchanging view of the source
 * sequence for the duration of the operation.
 * This could be satisfied by the caller holding a lock during the
 * operation's call, by using an immutable source sequence, or by not
 * sharing the source sequence across threads.
 * <p>
 * Every string buffer has a capacity. As long as the length of the
 * character sequence contained in the string buffer does not exceed
 * the capacity, it is not necessary to allocate a new internal
 * buffer array. If the internal buffer overflows, it is
 * automatically made larger.
 * <p>
 * Unless otherwise noted, passing a {@code null} argument to a constructor
 * or method in this class will cause a {@link NullPointerException} to be
 * thrown.
 * <p>
 * As of  release JDK 5, this class has been supplemented with an equivalent
 * class designed for use by a single thread, {@link StringBuilder}.  The
 * {@code StringBuilder} class should generally be used in preference to
 * this one, as it supports all of the same operations but it is faster, as
 * it performs no synchronization.
 *
 * @see     java.lang.StringBuilder
 * @see     java.lang.String
 * @since   JDK1.0
 */

class EString : public EComparable<EString*>
{
public:
	virtual ~EString();

	EString();
	EString(const char* s);
	EString(const char* s, int len);
	EString(const char* s, int off, int len);
	EString(const std::string* s);
	EString(const std::string& s);
	EString(const std::string& s, int len);
	EString(const std::string& s, int off, int len);
	EString(const EString* s);
	EString(const EString& s);
	EString(const EString& s, int len);
	EString(const EString& s, int off, int len);
	EString(es_nullptr_t) {};

	explicit EString(const char c);
	explicit EString(short i, int radix = 10);
	explicit EString(ushort i, int radix = 10);
	explicit EString(int i, int radix = 10);
	explicit EString(uint i, int radix = 10);
	explicit EString(long l, int radix = 10);
	explicit EString(ulong ll, int radix = 10);
	explicit EString(llong ll, int radix = 10);
	explicit EString(ullong ll, int radix = 10);

public:
	// java String
	const char charAt(int index) THROWS(EIndexOutOfBoundsException);

	boolean endsWith(const char* suffix);
	boolean endsWith(EString& suffix);
	boolean endsWith(std::string& suffix);

	boolean startsWith(const char* prefix, int toffset = 0);
	boolean startsWith(EString& prefix, int toffset = 0);
	boolean startsWith(std::string& prefix, int toffset = 0);

	boolean equals(EString& anotherString);
	boolean equals(EString* anotherString);
	boolean equals(std::string& anotherString);
	boolean equals(const char* anotherString);

	boolean equalsIgnoreCase(EString& anotherString);
	boolean equalsIgnoreCase(EString* anotherString);
	boolean equalsIgnoreCase(std::string& anotherString);
	boolean equalsIgnoreCase(const char* anotherString);

	boolean regionMatches(boolean ignoreCase,
			int toffset, EString* other, int ooffset, int len);
	boolean regionMatches(boolean ignoreCase,
			int toffset, const char* other, int ooffset, int len);

	boolean contains(const char* s);
	boolean contains(const EString* s);
	boolean contains(const EString& s);
	boolean contains(const std::string& s);

	int indexOf(int ch, int fromIndex = 0);
	int indexOf(const EString& s, int fromIndex = 0);
	int indexOf(const std::string& s, int fromIndex = 0);
	int indexOf(const char* s, int fromIndex = 0);

	int lastIndexOf(int ch);
	int lastIndexOf(const EString& s);
	int lastIndexOf(const std::string& s);
	int lastIndexOf(const char* s);
	int lastIndexOf(int ch, int fromIndex);
	int lastIndexOf(const EString& s, int fromIndex);
	int lastIndexOf(const std::string& s, int fromIndex);
	int lastIndexOf(const char* s, int fromIndex);

	EString substring(int beginIndex, int endIndex = -1);

	EString& toLowerCase();
	EString& toUpperCase();

	EString& trim(char c = ' ');
	EString& trim(char cc[]);
	EString& ltrim(char c = ' ');
	EString& ltrim(char cc[]);
	EString& rtrim(char c = ' ');
	EString& rtrim(char cc[]);

	boolean isEmpty();

	int length() const;

	static EString valueOf(const char *data);
	static EString valueOf(const char *data, int length);
	static EString valueOf(const char *data, int offset, int count);
	static EString valueOf(boolean b);
	static EString valueOf(char c);
	static EString valueOf(int i, int radix = 10);
	static EString valueOf(llong l, int radix = 10);
	static EString valueOf(float f);
	static EString valueOf(double d);

	static EString formatOf(const char* fmt, ...);

	static EString toHexString(const byte* data, int length);

public:
	// java StringBuffer
	EString& append(const char* s, int len = -1);
	EString& append(const EString* s);
	EString& append(const EString& s);
	EString& append(const EString& s, int len);
	EString& append(const EString& s, int offset, int len);
	EString& append(const std::string* s);
	EString& append(const std::string& s);
	EString& append(const std::string& s, int len);
	EString& append(const std::string& s, int offset, int len);
	EString& append(boolean b);
	EString& append(const char c);
	EString& append(short i, int radix = 10);
	EString& append(ushort i, int radix = 10);
	EString& append(int i, int radix = 10);
	EString& append(uint i, int radix = 10);
	EString& append(long l, int radix = 10);
	EString& append(ulong l, int radix = 10);
	EString& append(llong l, int radix = 10);
	EString& append(ullong l, int radix = 10);
	EString& append(float f);
	EString& append(double d);

	EString& insert(int index, const char* s, int len = -1);
	EString& insert(int index, const std::string& s, int len = -1);
	EString& insert(int index, const EString& s, int len = -1);
	EString& insert(int index, int len, const char c);

	EString& replace(char oldChar, char newChar);
	EString& replace(const char *src, const char *dest);

	EString& replaceFirst(const char *src, const char *dest);

	EString& erase(int index = 0, int len = -1);

	EString& eraseCharAt(int index);

	EString& trimToSize() {
#ifdef CPP11_SUPPORT
		str_.shrink_to_fit();
#else
		std::string x;
		x.assign(str_, 0, str_.length());
		str_.swap(x);
#endif
		return *this;
	}

	EString& setLength(int newLength, char c=0);

	EString& reverse();

	int capacity();

public:
	EString operator+(const char c) const;
	EString operator+(const boolean b) const;
	EString operator+(const byte b) const;
	EString operator+(const int i) const;
	EString operator+(const long l) const;
	EString operator+(const llong ll) const;

	EString& operator=(const char* s);
	EString& operator=(const EString &s);
	EString& operator<<(const char* s);
	EString& operator<<(const char c);
	EString& operator<<(const EString* s);
	EString& operator<<(const EString& s);
	EString& operator<<(const boolean b);
	EString& operator<<(const byte b);
	EString& operator<<(const int i);
	EString& operator<<(const long l);
	EString& operator<<(const llong ll);

	EString& operator+=(const char* s);
	EString& operator+=(const char c);
	EString& operator+=(const EString* s);
	EString& operator+=(const EString& s);
	EString& operator+=(const boolean b);
	EString& operator+=(const byte b);
	EString& operator+=(const int i);
	EString& operator+=(const long l);
	EString& operator+=(const llong ll);
	boolean operator==(const char* s) const;
	boolean operator==(const std::string& s) const;
	boolean operator==(const EString& s) const;

	boolean operator!=(const char* s) const;
	boolean operator!=(const std::string& s) const;
	boolean operator!=(const EString& s) const;

	const char& operator[](int index) const THROWS(EIndexOutOfBoundsException);

	EString& concat(const EString& s);
	EString& concat(const std::string& s);
	EString& concat(const char* s, int len = -1);

	EString& fmtcat(const char* fmt, ...);
	EString& vformat(const char* fmt, va_list args);
	EString& format(const char* fmt, ...);

	int compare(const char* s, int len = -1) const;
	int compare(const EString& s) const;
	int compareNoCase(const char* s, int len = -1) const;
	int compareNoCase(const EString& s) const;

	EString& reset(const char* s, int len = -1);
	EString& reset(const EString& s, int offset = 0, int len = -1);
	EString& reset(const std::string& s, int offset = 0, int len = -1);

	EString splitAt(const char *separators, int index);

	EString& resize(int len, char c);

	EString& clear();

	const std::string& data();

	const char* c_str(int fromIndex = 0) const;

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

	/**
	 * Compares this string to the specified object.  The result is {@code
	 * true} if and only if the argument is not {@code null} and is a {@code
	 * String} object that represents the same sequence of characters as this
	 * object.
	 *
	 * @param  anObject
	 *         The object to compare this {@code String} against
	 *
	 * @return  {@code true} if the given object represents a {@code String}
	 *          equivalent to this string, {@code false} otherwise
	 *
	 * @see  #compareTo(String)
	 * @see  #equalsIgnoreCase(String)
	 */
	virtual boolean equals(EObject* obj);

	/**
	 * This object (which is already a string!) is itself returned.
	 *
	 * @return  the string itself.
	 */
	virtual EString toString();

	/**
	 * Returns a hash code for this string. The hash code for a
	 * {@code String} object is computed as
	 * <blockquote><pre>
	 * s[0]*31^(n-1) + s[1]*31^(n-2) + ... + s[n-1]
	 * </pre></blockquote>
	 * using {@code int} arithmetic, where {@code s[i]} is the
	 * <i>i</i>th character of the string, {@code n} is the length of
	 * the string, and {@code ^} indicates exponentiation.
	 * (The hash value of the empty string is zero.)
	 *
	 * @return  a hash code value for this object.
	 */
	virtual int hashCode();

protected:
	std::string str_;

	/** Cache the hash code for the string */
	int hash; // Default to 0

private:
	void rangeCheck(int index) const THROWS(EIndexOutOfBoundsException);
};

// ============================================================================
// Non-member functions.

// Operator+

inline
EString operator+(const EString& x, const EString& y) {
	EString result(x);
	result.append(y);
	return result;
}

inline
EString operator+(const EString& x, const EString* y) {
	EString result(x);
	result.append(y);
	return result;
}

inline
EString operator+(const EString* x, const EString& y) {
	EString result(x);
	result.append(y);
	return result;
}

inline
EString operator+(const char* s, const EString& y) {
	EString result(s);
	result.append(y);
	return result;
}

inline
EString operator+(const EString& x, const char* s) {
	EString result(x);
	result.append(s);
	return result;
}

} /* namespace efc */
#endif //!__ESTRING_HH__
