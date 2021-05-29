/*
 * EProperties.cpp
 *
 *  Created on: 2017-11-23
 *      Author: cxxjava@163.com
 */

#include "EProperties.hh"
#include "EDate.hh"
#include "EByteBuffer.hh"
#include "EFileInputStream.hh"
#include "EFileOutputStream.hh"

namespace efc {

/* Read in a "logical line" from an InputStream/Reader, skip all comment
 * and blank lines and filter out those leading whitespace characters
 * (\u0020, \u0009 and \u000c) from the beginning of a "natural line".
 * Method returns the char length of the "logical line" and stores
 * the line in "lineBuf".
 */
class LineReader {
public:
	int inLimit;// = 0;
	int inOff;// = 0;
	EInputStream* inStream;
	EA<byte> lineBuf;

	LineReader(EInputStream* inStream):
		inLimit(0), inOff(0),
		inStream(inStream), lineBuf(1024) {
	}

	int readLine() THROWS(EIOException) {
		int len = 0;
		char c = 0;

		boolean skipWhiteSpace = true;
		boolean isCommentLine = false;
		boolean isNewLine = true;
		boolean appendedLineBegin = false;
		boolean precedingBackslash = false;
		boolean skipLF = false;
		char inByteBuf[8192];

		while (true) {
			if (inOff >= inLimit) {
				inLimit = inStream->read(inByteBuf, sizeof(inByteBuf));
				inOff = 0;
				if (inLimit <= 0) {
					if (len == 0 || isCommentLine) {
						return -1;
					}
					if (precedingBackslash) {
						len--;
					}
					return len;
				}
			}
			//The line below is equivalent to calling a
			//ISO8859-1 decoder.
			c = (char) (0xff & inByteBuf[inOff++]);

			if (skipLF) {
				skipLF = false;
				if (c == '\n') {
					continue;
				}
			}
			if (skipWhiteSpace) {
				if (c == ' ' || c == '\t' || c == '\f') {
					continue;
				}
				if (!appendedLineBegin && (c == '\r' || c == '\n')) {
					continue;
				}
				skipWhiteSpace = false;
				appendedLineBegin = false;
			}
			if (isNewLine) {
				isNewLine = false;
				if (c == '#' || c == '!') {
					isCommentLine = true;
					continue;
				}
			}

			if (c != '\n' && c != '\r') {
				lineBuf[len++] = c;
				if (len == lineBuf.length()) {
					int newLength = lineBuf.length() * 2;
					if (newLength < 0) {
						newLength = EInteger::MAX_VALUE;
					}
					lineBuf.setLength(newLength);
				}
				//flip the preceding backslash flag
				if (c == '\\') {
					precedingBackslash = !precedingBackslash;
				} else {
					precedingBackslash = false;
				}
			}
			else {
				// reached EOL
				if (isCommentLine || len == 0) {
					isCommentLine = false;
					isNewLine = true;
					skipWhiteSpace = true;
					len = 0;
					continue;
				}
				if (inOff >= inLimit) {
					inLimit = inStream->read(inByteBuf, sizeof(inByteBuf));
					inOff = 0;
					if (inLimit <= 0) {
						if (precedingBackslash) {
							len--;
						}
						return len;
					}
				}
				if (precedingBackslash) {
					len -= 1;
					//skip the leading whitespace characters in following line
					skipWhiteSpace = true;
					appendedLineBegin = true;
					precedingBackslash = false;
					if (c == '\r') {
						skipLF = true;
					}
				} else {
					return len;
				}
			}
		}
		//not reach here.
		return 0;
	}
};

//=============================================================================

EProperties::~EProperties() {
	if (owned) {
		delete defaults;
	}
}

EProperties::EProperties(EProperties* defaults, boolean owned):
		defaults(defaults), owned(owned) {
}

sp<EString> EProperties::setProperty(EString key, EString value) {
	SYNCHRONIZED(this) {
		return put(new EString(key), new EString(value));
	}}
}

void EProperties::load(EInputStream* inStream) {
	SYNCHRONIZED(this) {
		LineReader lr(inStream);
		load0(&lr);
	}}
}

void EProperties::load(const char* fileName) {
	SYNCHRONIZED(this) {
		EFileInputStream fis(fileName);
		LineReader lr(&fis);
		load0(&lr);
	}}
}

void EProperties::load0 (LineReader* lr_) {
	LineReader& lr = *lr_;

	int limit;
	int keyLen;
	int valueStart;
	char c;
	boolean hasSep;
	boolean precedingBackslash;

	while ((limit = lr.readLine()) >= 0) {
		c = 0;
		keyLen = 0;
		valueStart = limit;
		hasSep = false;

		precedingBackslash = false;
		while (keyLen < limit) {
			c = lr.lineBuf[keyLen];
			//need check if escaped.
			if ((c == '=' ||  c == ':') && !precedingBackslash) {
				valueStart = keyLen + 1;
				hasSep = true;
				break;
			} else if ((c == ' ' || c == '\t' ||  c == '\f') && !precedingBackslash) {
				valueStart = keyLen + 1;
				break;
			}
			if (c == '\\') {
				precedingBackslash = !precedingBackslash;
			} else {
				precedingBackslash = false;
			}
			keyLen++;
		}
		while (valueStart < limit) {
			c = lr.lineBuf[valueStart];
			if (c != ' ' && c != '\t' &&  c != '\f') {
				if (!hasSep && (c == '=' ||  c == ':')) {
					hasSep = true;
				} else {
					break;
				}
			}
			valueStart++;
		}
		/* @see:
		String key = loadConvert(lr.lineBuf, 0, keyLen, convtBuf);
		String value = loadConvert(lr.lineBuf, valueStart, limit - valueStart, convtBuf);
		put(key, value);
		*/
		//put(new EString((const char*)lr.lineBuf.address(), 0, keyLen),
		//		new EString((const char*)lr.lineBuf.address(), valueStart, limit - valueStart));
		sp<EString> key = loadConvert((const char*)lr.lineBuf.address(), 0, keyLen);
		sp<EString> value = loadConvert((const char*)lr.lineBuf.address(), valueStart, limit - valueStart);
		put(key, value);
	}
}

void EProperties::store(EOutputStream* out, EString comments) {
	EPrintStream ps(out);
	store0(&ps, comments);
}

void EProperties::store(const char* fileName, EString comments) {
	EFileOutputStream fos(fileName);
	EPrintStream ps(&fos);
	store0(&ps, comments);
}

void EProperties::store0(EPrintStream* out, EString& comments) {
	if (!comments.isEmpty()) {
		writeComments(out, comments);
	}
	EDate date;
	EString h = "#" + date.toString();
	out->print(h.c_str());
	out->println();
	SYNCHRONIZED(this) {
		sp<EIterator<EMapEntry<sp<EString>, sp<EString> >*> > iter = entrySet()->iterator();
		while (iter->hasNext()) {
			EMapEntry<sp<EString>, sp<EString> >* me = iter->next();
			sp<EString> key = me->getKey();
			sp<EString> val = me->getValue();
			key = saveConvert(key.get(), true);
			val = saveConvert(val.get(), false);
			out->print(key->c_str());
			out->print("=");
			out->print(val->c_str());
			out->println();
		}
	}}
	out->flush();
}

sp<EString> EProperties::getProperty(EString key, const char* defaultValue) {
	sp<EString> sval = get(&key);
	return ((sval == null) && (defaults != null)) ? defaults->getProperty(&key) : sval;
}

sp<ESet<sp<EString> > > EProperties::propertyNames() {
	sp<EHashSet<sp<EString> > > h = new EHashSet<sp<EString> >();
	enumerateKeys(*h);
	return dynamic_pointer_cast<ESet<sp<EString> > >(h);
}

void EProperties::list(EPrintStream* out) {
	out->println("-- listing properties --");
	EHashMap<sp<EString>, sp<EString> > h;
	enumerate(h);
	sp<EIterator<EMapEntry<sp<EString>, sp<EString> >*> > iter = h.entrySet()->iterator();
	while (iter->hasNext()) {
		EMapEntry<sp<EString>, sp<EString> >* me = iter->next();
		sp<EString> key = me->getKey();
		sp<EString> val = me->getValue();
		EString linemsg;
		if (val->length() > 40) {
			linemsg = *key + "=" + val->substring(0, 37).c_str() + "...";
		} else {
			linemsg = *key + "=" + *val;
		}
		out->println(linemsg.c_str());
	}
}

void EProperties::enumerate(EHashMap<sp<EString>, sp<EString> >& h) {
	SYNCHRONIZED(this) {
		if (defaults != null) {
			defaults->enumerate(h);
		}
		sp<EIterator<EMapEntry<sp<EString>, sp<EString> >*> > iter = entrySet()->iterator();
		while (iter->hasNext()) {
			EMapEntry<sp<EString>, sp<EString> >* me = iter->next();
			h.put(me->getKey(), me->getValue());
		}
	}}
}

void EProperties::enumerateKeys(EHashSet<sp<EString> >& h) {
	SYNCHRONIZED(this) {
		if (defaults != null) {
			defaults->enumerateKeys(h);
		}
		sp<EIterator<EMapEntry<sp<EString>, sp<EString> >*> > iter = entrySet()->iterator();
		while (iter->hasNext()) {
			EMapEntry<sp<EString>, sp<EString> >* me = iter->next();
			h.add(me->getKey());
		}
	}}
}

sp<EString> EProperties::loadConvert(const char* in, int off, int len) {
	EByteBuffer out;
	char aChar;
	int end = off + len;

	while (off < end) {
		aChar = in[off++];
		if (aChar == '\\') {
			aChar = in[off++];
			if (aChar == 'u') {
				// Read the xxxx
				es_uint16_t value = 0;
				for (int i = 0; i < 4; i++) {
					aChar = in[off++];
					switch (aChar) {
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						value = (value << 4) + aChar - '0';
						break;
					case 'a':
					case 'b':
					case 'c':
					case 'd':
					case 'e':
					case 'f':
						value = (value << 4) + 10 + aChar - 'a';
						break;
					case 'A':
					case 'B':
					case 'C':
					case 'D':
					case 'E':
					case 'F':
						value = (value << 4) + 10 + aChar - 'A';
						break;
					default:
						throw EIllegalArgumentException(__FILE__, __LINE__,
								"Malformed \\uxxxx encoding.");
					}
				}
				//ucs2 to utf8
				es_uint8_t u8[5];
				int n = eso_ucs2_to_utf8(u8, value);
				out.append(u8, n);
			} else {
				if (aChar == 't')
					aChar = '\t';
				else if (aChar == 'r')
					aChar = '\r';
				else if (aChar == 'n')
					aChar = '\n';
				else if (aChar == 'f')
					aChar = '\f';
				out.append((byte) aChar);
			}
		} else {
			out.append((byte) aChar);
		}
	}
	return new EString((char*)out.data(), 0, out.size());
}

sp<EString> EProperties::saveConvert(EString* theString, boolean escapeSpace) {
	sp<EString> out = new EString();
	EString& outBuffer = *out;
	int len = theString->length();

	for(int x=0; x<len; x++) {
		char aChar = theString->charAt(x);
		// Handle common case first, selecting largest block that
		// avoids the specials below
		if ((aChar > 61) && (aChar < 127)) {
			if (aChar == '\\') {
				outBuffer.append('\\'); outBuffer.append('\\');
				continue;
			}
			outBuffer.append(aChar);
			continue;
		}
		switch(aChar) {
			case ' ':
				if (x == 0 || escapeSpace)
					outBuffer.append('\\');
				outBuffer.append(' ');
				break;
			case '\t':outBuffer.append('\\'); outBuffer.append('t');
					  break;
			case '\n':outBuffer.append('\\'); outBuffer.append('n');
					  break;
			case '\r':outBuffer.append('\\'); outBuffer.append('r');
					  break;
			case '\f':outBuffer.append('\\'); outBuffer.append('f');
					  break;
			case '=': // Fall through
			case ':': // Fall through
			case '#': // Fall through
			case '!':
				outBuffer.append('\\'); outBuffer.append(aChar);
				break;
			default:
				outBuffer.append(aChar);
				break;
		}
	}

	return out;
}

void EProperties::writeComments(EPrintStream* out, EString& comments) {
	out->print("#");
	int len = comments.length();
	int current = 0;
	int last = 0;
	char uu[6];
	uu[0] = '\\';
	uu[1] = 'u';
	while (current < len) {
		char c = comments.charAt(current);
		if (c == '\n' || c == '\r') {
			if (last != current)
				out->print(comments.substring(last, current).c_str());

			out->println();

			if (c == '\r' &&
				current != len - 1 &&
				comments.charAt(current + 1) == '\n') {
				current++;
			}
			if (current == len - 1 ||
				(comments.charAt(current + 1) != '#' &&
				comments.charAt(current + 1) != '!'))
				out->print("#");

			last = current + 1;
		}
		current++;
	}
	if (last != current)
		out->print(comments.substring(last, current).c_str());
	out->println();
}

} /* namespace efc */
