/*
 * EBonParser.hh
 *
 *  Created on: 2013-7-30
 *      Author: cxxjava@163.com
 */

#ifndef EBONPARSER_HH_
#define EBONPARSER_HH_

#include "EBon.hh"
#include "EInputStream.hh"
#include "EDataFormatException.hh"

namespace efc {

class EBonParser: public EObject {
public:
	virtual ~EBonParser();

	EBonParser(EInputStream* is);

	// unsupported.
	EBonParser(const EBonParser& that);
	// unsupported.
	EBonParser& operator= (const EBonParser& that);

	boolean nextBon(EBon* bon) THROWS(EDataFormateException);

	virtual void parsing(es_bon_node_t* node) {};

protected:
	EInputStream* _is;
	EBon* _bon;

	struct Stream {
		es_bon_stream_t stm;
		EBonParser* self;
	} _es;

	static es_int32_t stream_read(void *s, void *buf, es_size_t *size);
	static void parsed_node(void *s, es_bon_t *bon, es_bon_node_t *node);
};

} /* namespace efc */
#endif /* EBONPARSER_HH_ */
