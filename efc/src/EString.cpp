#include "EString.hh"
#include "EInteger.hh"
#include "ELLong.hh"
#include "EIllegalArgumentException.hh"
#include "ENullPointerException.hh"
#include "EIndexOutOfBoundsException.hh"

namespace efc {

EString::~EString() {
	//
};

EString::EString() : hash(0) {
}

EString::EString(const char* s) : str_(s ? s : ""), hash(0) {
}

EString::EString(const char* s, int len) : hash(0) {
	if (len < 0) {
		throw EIllegalArgumentException(__FILE__, __LINE__);
	}
	if (s) {
		str_.assign(s, len);
	}
}

EString::EString(const char* s, int off, int len) : hash(0) {
	if (s) {
		if (off < 0 || len < 0) {
			throw EIllegalArgumentException(__FILE__, __LINE__);
		}
		str_.assign(s+off, len);
	}
}

EString::EString(const std::string* s) : str_(s ? (*s) : ""), hash(0) {
}

EString::EString(const std::string& s) : str_(s), hash(0) {
}

EString::EString(const std::string& s, int len) : hash(0) {
	if (len < 0) {
		throw EIllegalArgumentException(__FILE__, __LINE__);
	}
	if (len < s.length()) {
		str_ = s.substr(0, len);
	}
}

EString::EString(const std::string& s, int off, int len) : hash(0) {
	if (off < 0 || off > (s.length() - len)) {
		throw EIllegalArgumentException(__FILE__, __LINE__);
	}
	str_ = s.substr(off, len);
}

EString::EString(const EString* s) : str_(s ? s->str_ : ""), hash(0) {
}

EString::EString(const EString& s) : str_(s.str_), hash(0) {
}

EString::EString(const EString& s, int len) : str_(s.str_), hash(0) {
	if (len < 0) {
		throw EIllegalArgumentException(__FILE__, __LINE__);
	}
	if (len < s.str_.length()) {
		str_ = s.str_.substr(0, len);
	}
}

EString::EString(const EString& s, int off, int len) : hash(0) {
	if (off < 0 || off > (s.str_.length() - len)) {
		throw EIllegalArgumentException(__FILE__, __LINE__);
	}
	str_ = s.str_.substr(off, len);
}

EString::EString(const char c) : str_(1, c), hash(0) {
}

EString::EString(short i, int radix) : hash(0) {
	es_string_t* s = eso_itostring(i, radix);
	str_ = s;
	eso_mfree(s);
}

EString::EString(ushort i, int radix) : hash(0) {
	es_string_t* s = eso_itostring(i, radix);
	str_ = s;
	eso_mfree(s);
}

EString::EString(int i, int radix) : hash(0) {
	es_string_t* s = eso_itostring(i, radix);
	str_ = s;
	eso_mfree(s);
}

EString::EString(uint i, int radix) : hash(0) {
#if (SIZEOF_LONG == 8)
	es_string_t* s = eso_ltostring((llong)i, radix);
#else
	es_string_t* s = eso_itostring(i, radix);
#endif
	str_ = s;
	eso_mfree(s);
}

EString::EString(long l, int radix) : hash(0) {
#if (SIZEOF_LONG == 8)
	es_string_t* s = eso_ltostring((llong)l, radix);
#else
	es_string_t* s = eso_itostring(l, radix);
#endif
	str_ = s;
	eso_mfree(s);
}

EString::EString(ulong ll, int radix) : hash(0) {
#if (SIZEOF_LONG == 8)
	es_string_t* s = eso_ultostring((ullong)ll, radix);
#else
	es_string_t* s = eso_uitostring(ll, radix);
#endif
	str_ = s;
	eso_mfree(s);
}

EString::EString(llong ll, int radix) : hash(0) {
	es_string_t* s = eso_ltostring(ll, radix);
	str_ = s;
	eso_mfree(s);
}

EString::EString(ullong ll, int radix) : hash(0) {
	es_string_t* s = eso_ultostring(ll, radix);
	str_ = s;
	eso_mfree(s);
}

const char EString::charAt(int index) {
	rangeCheck(index);
	return str_[index];
}

boolean EString::endsWith(const char* suffix) {
	if (!suffix) {
		throw ENullPointerException(__FILE__, __LINE__);
	}
	int len = (int)strlen(suffix);
	int pos = ((int)str_.length() - len);
	if (pos < 0) {
		return false;
	}
	return (str_.find(suffix, pos) != std::string::npos);
}

boolean EString::endsWith(EString& suffix) {
	int len = (int)suffix.str_.length();
	int pos = ((int)str_.length() - len);
	if (pos < 0) {
		return false;
	}
	return (str_.find(suffix.str_, pos) != std::string::npos);
}

boolean EString::endsWith(std::string& suffix) {
	int len = (int)suffix.length();
	int pos = ((int)str_.length() - len);
	if (pos < 0) {
		return false;
	}
	return (str_.find(suffix, pos) != std::string::npos);
}

boolean EString::startsWith(const char* prefix, int toffset) {
	if (!prefix) {
		throw ENullPointerException(__FILE__, __LINE__);
	}
	std::string& ta = str_;
	int to = toffset;
	const char* pa = prefix;
	int po = 0;
	int pc = (int)strlen(prefix);
	// Note: toffset might be near -1>>>1.
	if ((toffset < 0) || (toffset > (int)str_.length() - pc)) {
		return false;
	}
	while (--pc >= 0) {
		if (ta[to++] != pa[po++]) {
			return false;
		}
	}
	return true;
}

boolean EString::startsWith(EString& prefix, int toffset) {
	std::string& ta = str_;
	int to = toffset;
	std::string& pa = prefix.str_;
	int po = 0;
	int pc = (int)prefix.str_.length();
	// Note: toffset might be near -1>>>1.
	if ((toffset < 0) || (toffset > (int)str_.length() - pc)) {
		return false;
	}
	while (--pc >= 0) {
		if (ta[to++] != pa[po++]) {
			return false;
		}
	}
	return true;
}

boolean EString::startsWith(std::string& prefix, int toffset) {
	std::string& ta = str_;
	int to = toffset;
	std::string& pa = prefix;
	int po = 0;
	int pc = (int)prefix.length();
	// Note: toffset might be near -1>>>1.
	if ((toffset < 0) || (toffset > str_.length() - pc)) {
		return false;
	}
	while (--pc >= 0) {
		if (ta[to++] != pa[po++]) {
			return false;
		}
	}
	return true;
}

boolean EString::equals(EString& anotherString) {
	if (this == &anotherString) {
		return true;
	}
	return (str_ == anotherString.str_);
}

boolean EString::equals(EString* anotherString) {
	if (this == anotherString) {
		return true;
	}
	return anotherString ? (str_ == anotherString->str_) : false;
}

boolean EString::equals(std::string& anotherString) {
	return (str_ == anotherString);
}

boolean EString::equals(const char* anotherString) {
	return (str_ == anotherString);
}

boolean EString::equalsIgnoreCase(EString& anotherString) {
	if (this == &anotherString) {
		return true;
	}
	return this->equalsIgnoreCase(anotherString.str_);
}

boolean EString::equalsIgnoreCase(EString* anotherString) {
	if (this == anotherString) {
		return true;
	}
	return anotherString ? (this->equalsIgnoreCase(anotherString->str_)) : false;
}

boolean EString::equalsIgnoreCase(std::string& anotherString) {
	if (&this->str_ == &anotherString) {
		return true;
	}

	int len = (int)str_.length();
	if (len != anotherString.length()) {
		return false;
	}

	int i = 0;
	while (len-- != 0) {
		if (eso_toupper(str_[i]) != eso_toupper(anotherString[i])) {
			return false;
		}
		i++;
	}
	return true;
}

boolean EString::equalsIgnoreCase(const char* anotherString) {
	if (!anotherString) {
		return false;
	}

	int len = (int)str_.length();
	if (len != strlen(anotherString)) {
		return false;
	}

	int i = 0;
	while (len-- != 0) {
		if (eso_toupper(str_[i]) != eso_toupper(*anotherString)) {
			return false;
		}
		i++;
		anotherString++;
	}
	return true;
}

boolean EString::regionMatches(boolean ignoreCase, int toffset, EString* other,
		int ooffset, int len) {
	// Note: toffset, ooffset, or len might be near -1>>>1.
	if (!other
			|| (ooffset < 0) || (toffset < 0)
			|| (toffset > str_.length() - len)
			|| (ooffset > other->str_.length() - len)) {
		return false;
	}
	std::string& ta = str_;
	int to = toffset;
	std::string& pa = other->str_;
	int po = ooffset;
	while (len-- > 0) {
		char c1 = ta[to++];
		char c2 = pa[po++];
		if (c1 == c2) {
			continue;
		}
		if (ignoreCase && (eso_toupper(c1) == eso_toupper(c2))) {
			continue;
		}
		return false;
	}
	return true;
}
boolean EString::regionMatches(boolean ignoreCase, int toffset,
		const char* other, int ooffset, int len) {
	// Note: toffset, ooffset, or len might be near -1>>>1.
	if (!other
			|| (ooffset < 0) || (toffset < 0)
			|| (toffset > str_.length() - len)
			|| (ooffset > strlen(other) - len)) {
		return false;
	}
	const char* ta = str_.c_str();
	int to = toffset;
	const char* pa = other;
	int po = ooffset;
	while (len-- > 0) {
		char c1 = ta[to++];
		char c2 = pa[po++];
		if (c1 == c2) {
			continue;
		}
		if (ignoreCase && (eso_toupper(c1) == eso_toupper(c2))) {
			continue;
		}
		return false;
	}
	return true;
}

boolean EString::contains(const char* s) {
    if (!s) return false;
	return str_.find(s) != std::string::npos;
}

boolean EString::contains(const EString* s) {
	if (!s) return false;
	return str_.find(s->str_) != std::string::npos;
}

boolean EString::contains(const EString& s) {
	return str_.find(s.str_) != std::string::npos;
}

boolean EString::contains(const std::string& s) {
	return str_.find(s) != std::string::npos;
}

int EString::indexOf(int ch, int fromIndex) {
	if (fromIndex < 0) fromIndex = 0;
	return (int)str_.find(ch, fromIndex);
}

int EString::indexOf(const EString& s, int fromIndex) {
	if (fromIndex < 0) fromIndex = 0;
	return (int)str_.find(s.str_, fromIndex);
}

int EString::indexOf(const std::string& s, int fromIndex) {
	if (fromIndex < 0) fromIndex = 0;
	return (int)str_.find(s, fromIndex);
}

int EString::indexOf(const char* s, int fromIndex) {
    if (!s) return -1;
	if (fromIndex < 0) fromIndex = 0;
	return (int)str_.find(s, fromIndex);
}

int EString::lastIndexOf(int ch) {
	return (int)str_.rfind(ch);
}

int EString::lastIndexOf(const EString& s) {
	return (int)str_.rfind(s.str_);
}

int EString::lastIndexOf(const std::string& s) {
	return (int)str_.rfind(s);
}

int EString::lastIndexOf(const char* s) {
    if (!s) return -1;
	return (int)str_.rfind(s);
}

int EString::lastIndexOf(int ch, int fromIndex) {
	return (int)str_.rfind(ch, fromIndex);
}

int EString::lastIndexOf(const EString& s, int fromIndex) {
	return (int)str_.rfind(s.str_, fromIndex);
}

int EString::lastIndexOf(const std::string& s, int fromIndex) {
	return (int)str_.rfind(s, fromIndex);
}

int EString::lastIndexOf(const char* s, int fromIndex) {
    if (!s) return -1;
	return (int)str_.rfind(s, fromIndex);
}

EString EString::substring(int beginIndex, int endIndex) {
	if (endIndex < 0) {
		endIndex = (int)str_.length();
	}
	if (endIndex > str_.length()) {
		EString msg("String index out of range: ");
		msg += endIndex;
		throw EIndexOutOfBoundsException(__FILE__, __LINE__, msg.c_str());
	}
	int subLen = endIndex - beginIndex;
	if (subLen < 0) {
		EString msg("String index out of range: ");
		msg += subLen;
		throw EIndexOutOfBoundsException(__FILE__, __LINE__, msg.c_str());
	}
	return ((beginIndex == 0) && (endIndex == str_.length())) ? *this
			: EString(str_, beginIndex, subLen);
}

EString& EString::toLowerCase() {
		int len = (int)str_.length();
		int i = 0;
		while (len-- > 0) {
			str_[i] = eso_tolower(str_[i]);
			i++;
		}
		return *this;
	}

EString& EString::toUpperCase() {
	int len = (int)str_.length();
	int i = 0;
	while (len-- > 0) {
		str_[i] = eso_toupper(str_[i]);
		i++;
	}
	return *this;
}

EString& EString::trim(char c) {
	ltrim(c);
	rtrim(c);
	return *this;
}

EString& EString::trim(char cc[]) {
	ltrim(cc);
	rtrim(cc);
	return *this;
}

EString& EString::ltrim(char c) {
	str_.erase(0, str_.find_first_not_of(c));
	return *this;
}

EString& EString::ltrim(char cc[]) {
	str_.erase(0, str_.find_first_not_of(cc));
	return *this;
}

EString& EString::rtrim(char c) {
	str_.erase(str_.find_last_not_of(c) + 1);
	return *this;
}

EString& EString::rtrim(char cc[]) {
	str_.erase(str_.find_last_not_of(cc) + 1);
	return *this;
}

boolean EString::isEmpty() {
	return str_.empty();
}

int EString::length() const {
	return (int)str_.length();
}

EString EString::valueOf(const char *data) {
	return EString(data);
}

EString EString::valueOf(const char *data, int length) {
	return EString(data, length);
}

EString EString::valueOf(const char *data, int offset, int count) {
	return EString(data, offset, count);
}

EString EString::valueOf(boolean b) {
	return b ? EString("true") : EString("false");
}

EString EString::valueOf(char c) {
	char v[2] = {0};
	v[0] = c;
	return EString(v);
}

EString EString::valueOf(int i, int radix) {
	return EInteger::toString(i, radix);
}

EString EString::valueOf(llong l, int radix) {
	return ELLong::toString(l, radix);
}

EString EString::valueOf(float f) {
	char s[80];
	eso_snprintf(s, sizeof(s), "%f", f);
	return EString(s);
}

EString EString::valueOf(double d) {
	char s[80];
	eso_snprintf(s, sizeof(s), "%lf", d);
	return EString(s);
}

EString EString::formatOf(const char* fmt, ...) {
	va_list args;
	EString s;

	va_start(args, fmt);
	s.vformat(fmt, args);
	va_end(args);

	return s;
}

EString EString::toHexString(const byte* data, int length) {
	EString s;
	char *p = eso_new_bytes2hexstr((es_uint8_t*)data, length);
	s.append(p);
	eso_free(p);
	return s;
}

EString& EString::append(const char* s, int len) {
    if (!s) return *this;
	if (len < 0)
		str_.append(s);
	else
		str_.append(s, len);
	return *this;
}

EString& EString::append(const EString* s) {
	if (!s) return *this;
	str_.append(s->str_);
	return *this;
}

EString& EString::append(const EString& s) {
	str_.append(s.str_);
	return *this;
}

EString& EString::append(const EString& s, int len) {
	if (len < 0) {
		len = s.str_.length();
	}
	str_.append(s.str_, 0, len);
	return *this;
}

EString& EString::append(const EString& s, int offset, int len) {
	if (len < 0 || offset < 0 || offset > (s.str_.length() - len)) {
		EString msg("String index out of range: ");
		msg += offset;
		throw EIndexOutOfBoundsException(__FILE__, __LINE__, msg.c_str());
	}
	str_.append(s.str_, offset, len);
	return *this;
}

EString& EString::append(const std::string* s) {
	if (!s) return *this;
	str_.append(*s);
	return *this;
}

EString& EString::append(const std::string& s) {
	str_.append(s);
	return *this;
}

EString& EString::append(const std::string& s, int len) {
	if (len < 0) {
		len = s.length();
	}
	str_.append(s, 0, len);
	return *this;
}

EString& EString::append(const std::string& s, int offset, int len) {
	if (len < 0 || offset < 0 || offset > (s.length() - len)) {
		EString msg("String index out of range: ");
		msg += offset;
		throw EIndexOutOfBoundsException(__FILE__, __LINE__, msg.c_str());
	}
	str_.append(s, offset, len);
	return *this;
}

EString& EString::append(boolean b) {
	str_.append(b ? "true" : "false");
	return *this;
}

EString& EString::append(const char c) {
	char s[2] = {0};
	s[0] = c;
	str_.append(s);
	return *this;
}

EString& EString::append(short i, int radix) {
	return this->append((int)i, radix);
}

EString& EString::append(ushort i, int radix) {
	return this->append((int)i, radix);
}

EString& EString::append(int i, int radix) {
	EString s = EInteger::toString(i, radix);
	str_.append(s.c_str());
	return (*this);
}

EString& EString::append(uint i, int radix) {
	return this->append((llong)i, radix);
}

EString& EString::append(long l, int radix) {
	return this->append((llong)l, radix);
}

EString& EString::append(ulong l, int radix) {
	return this->append((llong)l, radix);
}

EString& EString::append(llong l, int radix) {
	EString s = ELLong::toString(l, radix);
	str_.append(s.c_str());
	return (*this);
}

EString& EString::append(ullong l, int radix) {
	EString s = ELLong::toUnsignedString(l, radix);
	str_.append(s.c_str());
	return (*this);
}

EString& EString::append(float f) {
	char s[80];
	eso_snprintf(s, sizeof(s), "%f", f);
	str_.append(s);
	return (*this);
}

EString& EString::append(double d) {
	char s[80];
	eso_snprintf(s, sizeof(s), "%lf", d);
	str_.append(s);
	return (*this);
}

EString& EString::insert(int index, const char* s, int len) {
	if (!s) return *this;

	if (len < 0) {
		len = (int)strlen(s);
	}
	str_.insert(index, s, len);

	return *this;
}

EString& EString::insert(int index, const std::string& s, int len) {
	if (len < 0) {
		len = (int)s.length();
	}
	str_.insert(index, s, 0, len);

	return *this;
}

EString& EString::insert(int index, const EString& s, int len) {
	if (len < 0) {
		len = (int)s.str_.length();
	}
	str_.insert(index, s.str_, 0, len);

	return *this;
}

EString& EString::insert(int index, int len, const char c) {
	str_.insert(index, len, c);
	return *this;
}

EString& EString::replace(char oldChar, char newChar) {
	if (oldChar != newChar) {
		int len = (int)str_.length();
		int i = 0;
		while (len-- > 0) {
			if (str_[i] == oldChar) {
				str_[i] = newChar;
			}
			i++;
		}
	}
	return *this;
}

EString& EString::replace(const char *src, const char *dest) {
	if (!src || !dest) {
		throw ENullPointerException(__FILE__, __LINE__);
	}
	if (src != dest) {
		std::string::size_type pos = 0;
		while ((pos = str_.find(src, pos)) != std::string::npos) {
			str_.replace(pos, strlen(src), dest);
			pos += strlen(dest);
		}
	}
	return *this;
}

EString& EString::replaceFirst(const char *src, const char *dest) {
	if (!src || !dest) {
		throw ENullPointerException(__FILE__, __LINE__);
	}
	if (src != dest) {
		std::string::size_type pos = 0;
		if ((pos = str_.find(src, pos)) != std::string::npos) {
			str_.replace(pos, strlen(src), dest);
		}
	}
	return *this;
}

EString& EString::erase(int index, int len) {
	rangeCheck(index);
	str_.erase(index, (len < 0) ? str_.npos : len);
	return *this;
}

EString& EString::eraseCharAt(int index) {
	rangeCheck(index);
	str_.erase(index, 1);
	return *this;
}

EString& EString::setLength(int newLength, char c) {
	if (newLength < 0) {
		throw EIllegalArgumentException(__FILE__, __LINE__);
	}
	str_.resize(newLength, c);
	return *this;
}

EString& EString::reverse() {
	std::reverse(str_.begin(), str_.end());
	return *this;
}

int EString::capacity() {
	return str_.capacity();
}

EString EString::operator+(const char c) const {
	return (EString(this) << c);
}

EString EString::operator+(const boolean b) const {
	return (EString(this) << b);
}

EString EString::operator+(const byte b) const {
	return (EString(this) << b);
}

EString EString::operator+(const int i) const {
	return (EString(this) << i);
}

EString EString::operator+(const long l) const {
	return (EString(this) << l);
}

EString EString::operator+(const llong ll) const {
	return (EString(this) << ll);
}

EString& EString::operator=(const char* s) {
    str_ = (s ? s : "");
	return *this;
}

EString& EString::operator=(const EString &s) {
	str_ = s.str_;
	return *this;
}

EString& EString::operator<<(const char* s) {
    str_.append(s ? s : "");
	return *this;
}

EString& EString::operator<<(const char c) {
	char s[2] = {0};
	s[0] = c;
	str_.append(s);
	return *this;
}

EString& EString::operator<<(const EString* s) {
	if (!s) return (*this);
	str_.append(s->str_);
	return *this;
}

EString& EString::operator<<(const EString& s) {
	str_.append(s.str_);
	return *this;
}

EString& EString::operator<<(const boolean b) {
	str_.append(b ? "true" : "false");
	return *this;
}

EString& EString::operator<<(const byte b) {
	char s[2] = {0};
	s[0] = b;
	str_.append(s);
	return *this;
}

EString& EString::operator<<(const int i) {
	EString s(i);
	str_.append(s.str_);
	return *this;
}

EString& EString::operator<<(const long l) {
	EString s(l);
	str_.append(s.str_);
	return *this;
}

EString& EString::operator<<(const llong ll) {
	EString s(ll);
	str_.append(s.str_);
	return *this;
}

EString& EString::operator+=(const char* s) {
    str_.append(s ? s : "");
	return *this;
}

EString& EString::operator+=(const char c) {
	return this->append(c);
}

EString& EString::operator+=(const EString* s) {
	return this->append(s);
}

EString& EString::operator+=(const EString& s) {
	return this->append(s);
}

EString& EString::operator+=(const boolean b) {
	return this->append(b);
}

EString& EString::operator+=(const byte b) {
	return this->append(b);
}

EString& EString::operator+=(const int i) {
	return this->append(i);
}

EString& EString::operator+=(const long l) {
	return this->append(l);
}

EString& EString::operator+=(const llong ll) {
	return this->append(ll);
}

boolean EString::operator==(const char* s) const {
	return (str_ == s);
}

boolean EString::operator==(const std::string& s) const {
	return (str_ == s);
}

boolean EString::operator==(const EString& s) const {
	return (str_ == s.str_);
}

boolean EString::operator!=(const char* s) const {
	return (str_ != s);
}

boolean EString::operator!=(const std::string& s) const {
	return (str_ != s);
}

boolean EString::operator!=(const EString& s) const {
	return (str_ != s.str_);
}

const char& EString::operator[](int index) const {
	rangeCheck(index);
	return str_[index];
}

EString& EString::concat(const EString& s) {
	return this->append(s);
}

EString& EString::concat(const std::string& s) {
	return this->append(s);
}

EString& EString::concat(const char* s, int len) {
	return this->append(s, len);
}

EString& EString::fmtcat(const char* fmt, ...) {
	va_list args;
	EString s;
	va_start(args, fmt);
	s.vformat(fmt, args);
	va_end(args);
	return this->append(s);
}

EString& EString::vformat(const char* fmt, va_list args) {
	es_string_t* s = NULL;
	eso_mvsprintf(&s, fmt, args);
	str_ = s;
	eso_mfree(s);

	return *this;
}

EString& EString::format(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	es_string_t* s = NULL;
	eso_mvsprintf(&s, fmt, args);
	va_end(args);
	str_ = s;
	eso_mfree(s);

	return *this;
}

int EString::compare(const char* s, int len) const {
	if (!s) {
		return 1;
	}

	if (len < 0) {
		return str_.compare(s);
	}

	return eso_strncmp(str_.c_str(), s, len);
}

int EString::compare(const EString& s) const {
	return str_.compare(s.str_);
}

int EString::compareNoCase(const char* s, int len) const {
	if (!s) {
		return 1;
	}
	if (len < 0)
		return eso_strcasecmp(str_.c_str(), s);
	else
		return eso_strncasecmp(str_.c_str(), s, len);
}

int EString::compareNoCase(const EString& s) const {
	int sp = str_.length();
	int sq = s.str_.length();

	if (sp > sq)
		return 1;
	if (sp < sq)
		return -1;

	int diff = 0;
	for (int i = 0; i < sp; ++i) {
		diff = (eso_tolower(str_[i]) - eso_tolower(s.str_[i]));
		if (diff) {
			return diff;
		}
	}

	return 0;
}

EString& EString::reset(const char* s, int len) {
	if (!s) {
		throw ENullPointerException(__FILE__, __LINE__);
	}
	if (len < 0) {
		len = (int)strlen(s);
	}
	str_.assign(s, len);
	return *this;
}

EString& EString::reset(const EString& s, int offset, int len) {
	if (len < 0) {
		len = (int)s.str_.length() - offset;
	}
	if (len < 0 || offset < 0 || offset > (s.str_.length() - len)) {
		EString msg("String index out of range: ");
		msg += offset;
		throw EIndexOutOfBoundsException(__FILE__, __LINE__, msg.c_str());
	}
	str_.assign(s.str_, offset, len);
	return *this;
}

EString& EString::reset(const std::string& s, int offset, int len) {
	if (len < 0) {
		len = (int)s.length() - offset;
	}
	if (len < 0 || offset < 0 || offset > (s.length() - len)) {
		EString msg("String index out of range: ");
		msg += offset;
		throw EIndexOutOfBoundsException(__FILE__, __LINE__, msg.c_str());
	}
	str_.assign(s, offset, len);
	return *this;
}

EString EString::splitAt(const char *separators, int index) {
	es_string_t *to = NULL;
	to = eso_mstrsplit((char*)str_.c_str(), separators, index, &to);
	if (!to) {
		return null;
	}
	EString o(to);
	eso_mfree(to);
	return o;
}

EString& EString::resize(int len, char c) {
	return this->setLength(len, c);
}

EString& EString::clear() {
	str_.clear();
	return *this;
}

const std::string& EString::data() {
	return str_;
}

const char* EString::c_str(int fromIndex) const {
	return str_.c_str();
}

int EString::compareTo(EString* anotherString) {
	if (!anotherString) {
		return 1;
	}
	return this->compare(*anotherString);
}

int EString::compareToIgnoreCase(EString* anotherString) {
	if (!anotherString) {
		return 1;
	}
	return this->compareNoCase(*anotherString);
}

EString EString::toString() {
	return *this;
}

boolean EString::equals(EObject* obj) {
	if (!obj) return false;
	if (this == obj) return true;
	EString* that = dynamic_cast<EString*>(obj);
	return this->equals(that);
}

int EString::hashCode() {
	int h = hash;
	if (h == 0 && !str_.empty()) {
		for (int i=0; i<str_.length(); i++) {
			h = h * 31 + str_[i];
		}
		hash = h;
	}
	return h;
}

void EString::rangeCheck(int index) const THROWS(EIndexOutOfBoundsException) {
	if (index < 0 || index >= str_.length()) {
		EString msg("String index out of range: ");
		msg += index;
		throw EIndexOutOfBoundsException(__FILE__, __LINE__, msg.c_str());
	}
}

} /* namespace efc */
