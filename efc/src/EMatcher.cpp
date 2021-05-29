/*
 * EMatcher.cpp
 *
 *  Created on: 2013-8-5
 *      Author: cxxjava@163.com
 */

#include "EMatcher.hh"
#include "EPatternSyntaxException.hh"
#include "EUnsupportedOperationException.hh"

namespace efc {

EMatcher::~EMatcher() {
	delete _groups;
}

EMatcher::EMatcher() :
		_parentPattern(null), _groups(null), _text(null) {
	_groups = new EA<int>(32*3);
	reset();
}

EMatcher::EMatcher(EPattern* parent, const char* text) :
		_parentPattern(parent), _groups(null), _text(text) {
	_groups = new EA<int>(32*3);
	reset();
}

EPattern* EMatcher::pattern() {
	return _parentPattern;
}

EMatcher* EMatcher::usePattern(EPattern* newPattern) {
	ES_ASSERT(newPattern);
	_parentPattern = newPattern;
	reset();
	return this;
}

EMatcher* EMatcher::reset() {
	_groups->reset(-1);
	_offset = 0;
	_gcount = 0;
	return this;
}

EMatcher* EMatcher::reset(const char* input) {
	_text = input;
	return reset();
}

EString EMatcher::group() {
	return group(0);
}

EString EMatcher::group(int group) {
	if (group < 0 || group >= _gcount) {
		EString msg = EString::formatOf("No group %d", group);
		throw EIndexOutOfBoundsException(__FILE__, __LINE__, msg.c_str());
	}
	int begin = (*_groups)[group*2];
	int end = (*_groups)[group*2+1];
	return EString(_text, begin, end - begin);
}

int EMatcher::groupCount() {
	return _gcount;
}

boolean EMatcher::matches() {
	boolean ret;
	int ovector[3] = { 0 };
	int v = eso_pcre_exec(_parentPattern->c_pcre(), _text, -1, 0, 0, ovector, 3);
	if (v >= 0) {
		ret = true;
	} else if (v == -1) {
		ret = false;
	} else /*if (v < -1)*/ {
		throw EPatternSyntaxException(__FILE__, __LINE__);
	}
	return ret;
}

int EMatcher::start() {
	throw EUNSUPPORTEDOPERATIONEXCEPTION;
}

int EMatcher::start(int group) {
	throw EUNSUPPORTEDOPERATIONEXCEPTION;
}

int EMatcher::end() {
	throw EUNSUPPORTEDOPERATIONEXCEPTION;
}

int EMatcher::end(int group) {
	throw EUNSUPPORTEDOPERATIONEXCEPTION;
}

boolean EMatcher::find() {
	es_pcre_t *pcre = _parentPattern->c_pcre();
RETRY:
	int v = eso_pcre_exec(pcre, _text, -1, _offset, 0, _groups->address(), _groups->length());
	if (v == 0) { //success, but offsets is not big enough
		EA<int>* old = _groups;
		_groups = new EA<int>(_groups->length() * 2, -1);
		delete old;
		goto RETRY;
	}
	else if (v > 0) { //success; value is the number of elements filled in
		_offset = (*_groups)[1];
		_gcount = v;
		return true;
	}
	else if (v == -1) { //failed to match
		reset();
	}
	else if (v < -1) {
		throw EPatternSyntaxException(__FILE__, __LINE__);
	}

	return false;
}

} /* namespace efc */
