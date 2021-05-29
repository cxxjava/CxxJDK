/*
 * EPattern.cpp
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#include "EPattern.hh"
#include "EPatternSyntaxException.hh"
#include "EMatcher.hh"

namespace efc {

EPattern::~EPattern()
{
	eso_pcre_free(&_pcre);
}

EPattern::EPattern(const char* regex, int opetion) THROWS(EPatternSyntaxException)
{
	_pattern = regex;

	int v;
	_pcre = eso_pcre_make(regex, opetion, &v);
	if (!_pcre) {
		throw EPatternSyntaxException(__FILE__, __LINE__);
	}
}

EString EPattern::pattern()
{
	return _pattern;
}

EString EPattern::toString()
{
	return _pattern;
}

es_pcre_t* EPattern::c_pcre()
{
	return _pcre;
}

EMatcher* EPattern::newMatcher(const char* input)
{
	return new EMatcher(this, input);
}

EArray<EString*> EPattern::split(const char* input, int limit)
{
	const char *regex = _pattern.c_str();
	EArray<EString*> res;
	EString *s;

	if (!input || !*input) {
		return res;
	}

	if (!regex || !*regex) {
		s = new EString(input);
		res.add(s);
		return res;
	}

	int v, count, length;
	int ovector[3];
	char *psrc = (char*)input;

	count = length = 0;
	while (*psrc && ((limit == 0) || (count < limit))) {
		v = eso_pcre_exec(_pcre, psrc, -1, 0, 0, ovector, 3);
		if (v >= 0) {
			res.add(new EString(psrc, 0, ovector[0]));
			psrc += ovector[1];
			count++;

			continue;
		}
		else if (v == -1) {
			res.add(new EString(psrc));
			break;
		}
		else if (v < -1) {
			throw EPatternSyntaxException(__FILE__, __LINE__);
		}
	}

	return res;
}

EArray<EString*> EPattern::split(const char* regex, const char* input,
		int limit) THROWS(EPatternSyntaxException)
{
	EArray<EString*> res;
	EString *s;

	if (!input) {
		return res;
	}
	
	if (!regex || !*regex) {
		s = new EString(input);
		res.add(s);
		return res;
	}

	//@see: openjdk-8/src/share/classes/java/lang/String.java#L2316

	/* fastpath if the regex is a
	 (1)one-char String and this character is not one of the
		RegEx's meta characters ".$|()[{^?*+\\", or
	 (2)two-char String and the first char is the backslash and
		the second is not the ascii digit or ascii letter.
	 */
	char ch = 0;
	if (((eso_strlen(regex) == 1 &&
			eso_strchr(".$|()[{^?*+\\", (ch = regex[0])) == NULL) ||
			(eso_strlen(regex) == 2 &&
					regex[0] == '\\' &&
					(((ch = regex[1])-'0')|('9'-ch)) < 0 &&
					((ch-'a')|('z'-ch)) < 0 &&
					((ch-'A')|('Z'-ch)) < 0)))
	{
		EString s(input);
		int off = 0;
		int next = 0;
		boolean limited = limit > 0;
		while ((next = s.indexOf(ch, off)) != -1) {
			if (!limited || res.size() < limit - 1) {
				res.add(new EString(s.substring(off, next)));
				off = next + 1;
			} else {    // last one
				//assert (res.size() == limit - 1);
				res.add(new EString(s.substring(off, s.length())));
				off = s.length();
				break;
			}
		}
		// If no match was found, return this
		if (off == 0) {
			res.add(new EString(input));
			return res;
		}

		// Add remaining segment
		if (!limited || res.size() < limit)
			res.add(new EString(s.substring(off, s.length())));

		/* @see:
		// Construct result
		int resultSize = list.size();
		if (limit == 0) {
			while (resultSize > 0 && list.get(resultSize - 1).length() == 0) {
				resultSize--;
			}
		}
		String[] result = new String[resultSize];
		return list.subList(0, resultSize).toArray(result);
		*/
		int resultSize = res.size();
		for (int i=resultSize-1; i>=0; i--) {
			if (res[i]->length() == 0) {
				res.removeAt(i);
			} else {
				break;
			}
		}

		return res;
	}

	//=================================================

	es_pcre_t *pcre;
	int v, count, length;
	int ovector[3];
	char *psrc = (char*)input;

	pcre = eso_pcre_make(regex, 0, &v);
	if (!pcre) {
		throw EPatternSyntaxException(__FILE__, __LINE__);
	}

	count = length = 0;
	while (*psrc && ((limit == 0) || (count < limit))) {
		v = eso_pcre_exec(pcre, psrc, -1, 0, 0, ovector, 3);
		if (v >= 0) {
			res.add(new EString(psrc, 0, ovector[0]));
			psrc += ovector[1];
			count++;

			continue;
		}
		else if (v == -1) {
			res.add(new EString(psrc));
			break;
		}
		else if (v < -1) {
			eso_pcre_free(&pcre);
			throw EPatternSyntaxException(__FILE__, __LINE__);
		}
	}

	eso_pcre_free(&pcre);

	return res;
}

boolean EPattern::matches(const char* regex, const char* input) THROWS(EPatternSyntaxException)
{
	if ((!input || !*input) && (!regex || !*regex)) {
		return true;
	}

	if (!input || !*input) {
		return false;
	}

	if (!regex || !*regex) {
		return false;
	}

	es_pcre_t *pcre;
	int v;
	int ovector[3];
	boolean ret;

	pcre = eso_pcre_make(regex, 0, &v);
	if (!pcre) {
		throw EPatternSyntaxException(__FILE__, __LINE__);
	}

	v = eso_pcre_exec(pcre, input, -1, 0, 0, ovector, 3);
	if (v >= 0) {
		ret = true;
	} else if (v == -1) {
		ret = false;
	} else if (v < -1) {
		eso_pcre_free(&pcre);
		throw EPatternSyntaxException(__FILE__, __LINE__);
	}

	eso_pcre_free(&pcre);

	return ret;
}

} /* namespace efc */
