/*
 * EBsonParser.hh
 *
 *  Created on: 2013-7-30
 *      Author: cxxjava@163.com
 */

#ifndef EBSONPARSER_HH_
#define EBSONPARSER_HH_

#include "EBson.hh"
#include "EInputStream.hh"
#include "EDataFormatException.hh"

namespace efc {

class EBsonParser: public EObject {
public:
	virtual ~EBsonParser();

	EBsonParser(EInputStream* is);

	// unsupported.
	EBsonParser(const EBsonParser& that);
	// unsupported.
	EBsonParser& operator= (const EBsonParser& that);

	boolean nextBson(EBson* bson) THROWS(EDataFormateException);

	virtual void parsing(es_bson_node_t* node) {};

protected:
	EInputStream* _is;
	EBson* _bson;

	struct Stream {
		es_bson_stream_t stm;
		EBsonParser* self;
	} _es;

	static es_int32_t stream_read(void *s, void *buf, es_size_t *size);
	static void parsed_node(void *s, es_bson_t *bson, es_bson_node_t *node);
};

} /* namespace efc */
#endif /* EBSONPARSER_HH_ */
