/*
 * testbon.cpp
 *
 *  Created on: 2014-2-19
 *      Author: cxxjava@163.com
 */
#include "main.hh"
#include "Efc.hh"

MAIN_IMPL(testbon) {
	ESystem::init(argc, argv, "cxxjava.conf");

	const char* inFileName = ESystem::getProgramArgument("i");
	if (!inFileName || eso_strlen(inFileName) == 0) {
		ESystem::out->println("Usage: bonparser -i [filename] {-o [filename]}");
		ESystem::exit(0);
	}
	const char* outFileName = ESystem::getProgramArgument("o");

	EFileInputStream *fis = new EFileInputStream(inFileName);
	EFileOutputStream *fos =
			(outFileName && eso_strlen(outFileName) > 0) ?
					new EFileOutputStream(outFileName) :
					new EFileOutputStream(stdout);

	class BonParser : public EBonParser {
	public:
		BonParser(EFileInputStream *fis,
				EFileOutputStream* fos) :
				EBonParser(fis) {
			_fos = fos;
		}
		void parsing(es_bon_node_t* node) {
			if (!node) return;

			for (int i=1; i<_bon->levelOf(node); i++) {
				_fos->write("\t");
			}
			_fos->write(node->name);
			_fos->write("-->");
			_fos->write(node->value->data, node->value->len);
			_fos->write("\n");
		}
	private:
		EFileOutputStream* _fos;
	};

	BonParser ep(fis, fos);
	EBon bon;
	while (ep.nextBon(&bon)) {
		//
	}

	delete fis;
	delete fos;

	ESystem::exit(0);

	return 0;
}
