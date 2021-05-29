/*
 * EBsonParser.cpp
 *
 *  Created on: 2013-7-30
 *      Author: cxxjava@163.com
 */

#include "EBsonParser.hh"

namespace efc {

es_int32_t EBsonParser::stream_read(void *s, void *buf, es_size_t *size)
{
	EBsonParser::Stream *ins = (EBsonParser::Stream*)s;
	int ret = ins->self->_is->read(buf, *size);
	*size = (ret >= 0) ? ret : 0;
	return ret > 0 ?  ES_SUCCESS : BSON_EREAD;
}

void EBsonParser::parsed_node(void *s, es_bson_t *bson, es_bson_node_t *node)
{
	EBsonParser::Stream *ins = (EBsonParser::Stream*)s;
	ins->self->parsing(node);
}

EBsonParser::~EBsonParser() {
	eso_bson_parser_uninit(&_es.stm);
}

EBsonParser::EBsonParser(EInputStream* is) :
	_is(is), _bson(null) {
	eso_bson_parser_init(&_es.stm, EBsonParser::stream_read, EBsonParser::parsed_node);
	_es.self = this;
}

boolean EBsonParser::nextBson(EBson* bson) {
	if (bson) {
		bson->clear();
	}
	_bson = bson;

	if (_es.stm.finished) {
		return false;
	}

	if (eso_bson_parse(_bson->c_bson(), (es_bson_stream_t*) &_es) != ES_SUCCESS) {
		return false;
	}

	return true;
}

} /* namespace efc */
