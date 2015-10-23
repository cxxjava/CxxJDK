#ifndef __EString_H__
#define __EString_H__

#include "EObject.hh"
#include "EComparable.hh"

namespace efc {

class EString : virtual public EComparable<EString*>
{
public:
	virtual ~EString();

	EString();
	EString(const char* cstr);
	EString(const char* cstr, uint off, uint len);
	EString(const EString& estr);
	EString(const EString& estr, uint off, uint len);
	explicit EString(const char c);
	explicit EString(int i, int radix=10);
	explicit EString(long l, int radix=10);
	explicit EString(llong ll, int radix=10);

public:
	//c++ style
	const char*             c_str(uint fromIndex=0) const;
	
	uint                    length() const;
	uint                    countChars() const;
	
	EString&                resize(uint newlen);
	
	EString&                vformat(const char* fmt, va_list args);
	EString&                format(const char* fmt, ...);
	
	EString& 	            erase(uint index = 0, uint len = 0);
	EString& 	            insert(uint index, const char* cstr, int len = -1);
	EString& 	            insert(uint index, int len, const char c);
	EString& 	            replace(const char *from, const char *to);
	EString& 	            clear();
	
	EString                 substr(uint index, int len = -1) const;
	EString                 splitAt(const char *separators, uint index) THROWS(EIndexOutOfBoundsException);
	
	int	                    compare(const char* cstr, int len = -1) const;
	int	                    compare(const EString& estr) const;
	int	                    compareNoCase(const char* cstr, int len = -1) const;
	int	                    compareNoCase(const EString& estr) const;
	
	EString&                operator=(const char* cstr);
	EString&                operator=(const EString& estr);
	
	EString&                operator<<(const char* cstr);
	EString&                operator<<(const char c);
	EString&                operator<<(const EString& estr);
	
	EString&                operator+=(const char* cstr);
	EString&                operator+=(const char c);
	EString&                operator+=(const EString& estr);

	EString                 operator+(const char* cstr) const;
	EString                 operator+(const char c) const;
	EString                 operator+(const EString& estr) const;
	EString                 operator+(const boolean b) const;
	EString                 operator+(const int i) const;
	EString                 operator+(const long l) const;
	EString                 operator+(const llong ll) const;

	es_bool_t               operator==(const char* cstr) const;
	es_bool_t               operator==(const EString& estr) const;

	es_bool_t               operator!=(const char* cstr) const;
	es_bool_t               operator!=(const EString& estr) const;
	
	char                    operator[](uint index) const;
	
	//java style
	char                    charAt(uint index);
	boolean                 endsWith(EString& suffix);
	boolean                 endsWith(const char* suffix);
	boolean                 startsWith(EString& prefix, int toffset=0);
	boolean                 startsWith(const char* prefix, int toffset=0);
	boolean                 equals(EString& anotherString);
	boolean                 equalsIgnoreCase(EString& anotherString);
	boolean                 equals(EString* anotherString);
	boolean                 equalsIgnoreCase(EString* anotherString);
	boolean                 equals(const char* anotherString);
	boolean                 equalsIgnoreCase(const char* anotherString);
	boolean                 regionMatches(boolean ignoreCase, uint toffset, EString& other, uint ooffset, uint len);
	int                     indexOf(int ch, uint fromIndex=0);
	int                     indexOf(EString& str, uint fromIndex=0);
	int                     indexOf(const char* cstr, uint fromIndex=0);
	int                     lastIndexOf(int ch);
	int                     lastIndexOf(EString& str);
	int                     lastIndexOf(const char* cstr);
	int                     lastIndexOf(int ch, uint fromIndex);
	int                     lastIndexOf(EString& str, uint fromIndex);
	int                     lastIndexOf(const char* cstr, uint fromIndex);
	EString&                concat(EString& estr);
	EString&                concat(const char* cstr, int len=-1);
	EString&                fmtcat(const char* fmt, ...);
	EString&                replace(char oldChar, char newChar);
	EString&                toLowerCase();
	EString&                toUpperCase();
	EString&                trim(char c=' ');
	EString&                trim(char cc[]);
	EString&                ltrim(char c=' ');
	EString&                ltrim(char cc[]);
	EString&                rtrim(char c=' ');
	EString&                rtrim(char cc[]);

	EString                 substring(uint beginIndex, int endIndex=-1);

	int                     hashCode();

	static EString          formatOf(const char* fmt, ...);
	static EString          valueOf(char *data);
	static EString          valueOf(char *data, uint offset, uint count);
	static EString          valueOf(boolean b);
	static EString          valueOf(char c);
	static EString          valueOf(int i, int radix=10);
	static EString          valueOf(llong l, int radix=10);
	static EString          valueOf(float f);
	static EString          valueOf(double d);

	//StringBuffer
	EString&                append(const char* cstr, int len=-1);
	EString&                append(const EString& estr, uint offset=0, int len = -1);
	EString&                append(const char c);
	EString&                append(int i, int radix=10);
	EString&                append(llong l, int radix=10);
	EString&                append(float f);
	EString&                append(double d);
	EString&                setLength(uint newLength);
	EString&                reset(const char* cstr, int len=-1);
	EString&                reset(const EString& estr, uint offset=0, int len = -1);

	/**
	 * Returns <tt>true</tt> if, and only if, {@link #length()} is <tt>0</tt>.
	 *
	 * @return <tt>true</tt> if {@link #length()} is <tt>0</tt>, otherwise
	 * <tt>false</tt>
	 *
	 * @since 1.6
	 */
	boolean isEmpty();
	boolean isNotEmpty();

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
	int compareTo(EString* anotherString);

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
	int compareToIgnoreCase(EString* str);

protected:
	es_string_t*            m_string;
	
	/** Cache the hash code for the string */
	int hash; // Default to 0
};

} /* namespace efc */
#endif //!__EString_H__
