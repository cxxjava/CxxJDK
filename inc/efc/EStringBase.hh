#ifndef __EStringBase_HH__
#define __EStringBase_HH__

#include "EBase.hh"

namespace efc {

class EStringBase {
public:
	virtual ~EStringBase();

	EStringBase();
	EStringBase(const char* cstr);
	EStringBase(const char* cstr, uint off, uint len);
	EStringBase(const EStringBase* estr);
	EStringBase(const EStringBase& estr);
	EStringBase(const EStringBase& estr, uint off, uint len);
	explicit EStringBase(const char c);
	explicit EStringBase(int i, int radix = 10);
	explicit EStringBase(long l, int radix = 10);
	explicit EStringBase(llong ll, int radix = 10);

public:
	EStringBase(es_nullptr_t) : m_string(eso_mstrdup("")), hash(0) {}

public:
	//c++ style
	const char* c_str(uint fromIndex = 0) const;

	uint length() const;uint countChars() const;

	EStringBase& resize(uint newlen);

	EStringBase& vformat(const char* fmt, va_list args);
	EStringBase& format(const char* fmt, ...);

	EStringBase& erase(uint index = 0, uint len = 0);
	EStringBase& insert(uint index, const char* cstr, int len = -1);
	EStringBase& insert(uint index, int len, const char c);
	EStringBase& replace(const char *from, const char *to);
	EStringBase& clear();

	EStringBase substr(uint index, int len = -1) const;
	EStringBase splitAt(const char *separators, uint index) THROWS(EIndexOutOfBoundsException);

	int compare(const char* cstr, int len = -1) const;
	int compare(const EStringBase& estr) const;
	int compareNoCase(const char* cstr, int len = -1) const;
	int compareNoCase(const EStringBase& estr) const;

	EStringBase& operator=(const char* cstr);
	EStringBase& operator=(const EStringBase& estr);

	EStringBase& operator<<(const char* cstr);
	EStringBase& operator<<(const char c);
	EStringBase& operator<<(const EStringBase* estr);
	EStringBase& operator<<(const EStringBase& estr);
	EStringBase& operator<<(const boolean b);
	EStringBase& operator<<(const byte b);
	EStringBase& operator<<(const int i);
	EStringBase& operator<<(const long l);
	EStringBase& operator<<(const llong ll);

	EStringBase operator+(const char c) const;
	EStringBase operator+(const boolean b) const;
	EStringBase operator+(const byte b) const;
	EStringBase operator+(const int i) const;
	EStringBase operator+(const long l) const;
	EStringBase operator+(const llong ll) const;

	EStringBase& operator+=(const char* cstr);
	EStringBase& operator+=(const char c);
	EStringBase& operator+=(const EStringBase* estr);
	EStringBase& operator+=(const EStringBase& estr);
	EStringBase& operator+=(const boolean b);
	EStringBase& operator+=(const byte b);
	EStringBase& operator+=(const int i);
	EStringBase& operator+=(const long l);
	EStringBase& operator+=(const llong ll);

	boolean operator==(const char* cstr) const;
	boolean operator==(const EStringBase& estr) const;

	boolean operator!=(const char* cstr) const;
	boolean operator!=(const EStringBase& estr) const;

	char& operator[](uint index) const;

	//java style
	char charAt(uint index);
	boolean endsWith(EStringBase& suffix);
	boolean endsWith(const char* suffix);
	boolean startsWith(EStringBase& prefix, int toffset = 0);
	boolean startsWith(const char* prefix, int toffset = 0);
	boolean equals(EStringBase& anotherString);
	boolean equalsIgnoreCase(EStringBase& anotherString);
	boolean equals(EStringBase* anotherString);
	boolean equalsIgnoreCase(EStringBase* anotherString);
	boolean equals(const char* anotherString);
	boolean equalsIgnoreCase(const char* anotherString);
	boolean regionMatches(boolean ignoreCase, uint toffset, EStringBase* other, uint ooffset, uint len);
	boolean regionMatches(boolean ignoreCase, uint toffset, const char* other, uint ooffset, uint len);
	boolean contains(const char* s);
	int indexOf(int ch, uint fromIndex = 0);
	int indexOf(EStringBase& str, uint fromIndex = 0);
	int indexOf(const char* cstr, uint fromIndex = 0);
	int lastIndexOf(int ch);
	int lastIndexOf(EStringBase& str);
	int lastIndexOf(const char* cstr);
	int lastIndexOf(int ch, uint fromIndex);
	int lastIndexOf(EStringBase& str, uint fromIndex);
	int lastIndexOf(const char* cstr, uint fromIndex);
	EStringBase& concat(EStringBase& estr);
	EStringBase& concat(const char* cstr, int len = -1);
	EStringBase& fmtcat(const char* fmt, ...);
	EStringBase& replace(char oldChar, char newChar);
	EStringBase& toLowerCase();
	EStringBase& toUpperCase();
	EStringBase& trim(char c = ' ');
	EStringBase& trim(char cc[]);
	EStringBase& ltrim(char c = ' ');
	EStringBase& ltrim(char cc[]);
	EStringBase& rtrim(char c = ' ');
	EStringBase& rtrim(char cc[]);

	EStringBase substring(uint beginIndex, int endIndex = -1);

	//StringBuffer
	EStringBase& append(const EStringBase* estr);
	EStringBase& append(const char* cstr, int len = -1);
	EStringBase& append(const EStringBase& estr, uint offset = 0, int len = -1);
	EStringBase& append(const char c);
	EStringBase& append(int i, int radix = 10);
	EStringBase& append(llong l, int radix = 10);
	EStringBase& append(float f);
	EStringBase& append(double d);
	EStringBase& setLength(uint newLength);
	EStringBase& reset(const char* cstr, int len = -1);
	EStringBase& reset(const EStringBase& estr, uint offset = 0, int len = -1);

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

	static EStringBase formatOf(const char* fmt, ...);
	static EStringBase valueOf(const char *data);
	static EStringBase valueOf(const char *data, uint offset, uint count);
	static EStringBase valueOf(boolean b);
	static EStringBase valueOf(char c);
	static EStringBase valueOf(int i, int radix = 10);
	static EStringBase valueOf(llong l, int radix = 10);
	static EStringBase valueOf(float f);
	static EStringBase valueOf(double d);
	static EStringBase toHexString(const byte* data, int length);

protected:
	es_string_t* m_string;

	/** Cache the hash code for the string */
	int hash; // Default to 0
};


// ============================================================================
// Non-member functions.

// Operator+

inline
EStringBase operator+(const EStringBase& x, const EStringBase& y) {
	EStringBase result(x);
	result.append(y);
	return result;
}

inline
EStringBase operator+(const EStringBase& x, const EStringBase* y) {
	EStringBase result(x);
	result.append(y);
	return result;
}

inline
EStringBase operator+(const EStringBase* x, const EStringBase& y) {
	EStringBase result(x);
	result.append(y);
	return result;
}

inline
EStringBase operator+(const char* s, const EStringBase& y) {
	EStringBase result(s);
	result.append(y);
	return result;
}

inline
EStringBase operator+(const EStringBase& x, const char* s) {
	EStringBase result(x);
	result.append(s);
	return result;
}

} /* namespace efc */
#endif //!__EStringBase_HH__
