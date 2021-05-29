/*
 * EConfig.cpp
 *
 *  Created on: 2013-3-23
 *      Author: cxxjava@163.com
 */

#include "EConfig.hh"
#include "EArray.hh"
#include "EInteger.hh"
#include "ELLong.hh"
#include "EBoolean.hh"
#include "EFloat.hh"
#include "EDouble.hh"
#include "EPattern.hh"
#include "EFileInputStream.hh"
#include "EDataInputStream.hh"

namespace efc {

#define TRIM_CHARS " \t\r\n\f\v"

#define CFG_MAX_KEY_LEN             255              /* EMP节点关键字最长限制 */
#define	CFG_NAMEFLAG                '/'	             /* 名称和名称之间的分隔符 */
#define CFG_NAMEFLAG2               "/"
#define	CFG_POSFLAG	                '|'              /* 名称和位置之间的分隔符 */
#define CFG_POSFLAG2                "|"

#define CFG_NODE_INDEX_FIRST         1               /* 第一个符合要求的节点索引 */
#define CFG_NODE_INDEX_LAST          0               /* 最后一个符合要求的节点索引 */

static void env_value_parse(es_string_t **parsed,
                            const char *envstr,
                            EConfig *cfg)
{
	es_string_t *envstrdup = eso_mstrdup(envstr);
	char *pstr = (char *)envstrdup;
	char envkey[256] = {0};
	char *p1 = NULL;
	char *p2 = NULL;

	if (parsed) eso_mmemfill(*parsed, 0);

	while (TRUE) {
		p1 = eso_strstr(pstr, "$(");
		if (!p1)
			break;
		eso_mstrncat(parsed, pstr, p1-pstr);
		p2 = eso_strchr(p1+2, ')');
		if (!p2)
			break;
		else {
			eso_strncpy(envkey, p1+2, ES_MIN(p2-p1-2, (int)sizeof(envkey)));
			const char *ptmp = getenv(envkey);
			if (ptmp)
				eso_mstrcat(parsed, ptmp);
			pstr = p2 + 1;
		}
	}
	eso_mstrcat(parsed, pstr);
	ESO_MFREE(&envstrdup);
}

static void cfg_value_parse(es_string_t **parsed,
                            const char *value,
                            EConfig *cfg)
{
	es_string_t *strdup = eso_mstrdup(value);
	char *pstr = (char *)strdup;
	char key[256] = {0};
	char *p1 = NULL;
	char *p2 = NULL;

	if (parsed) eso_mmemfill(*parsed, 0);

	while (TRUE) {
		p1 = eso_strstr(pstr, "@(");
		if (!p1)
			break;
		eso_mstrncat(parsed, pstr, p1-pstr);
		p2 = eso_strchr(p1+2, ')');
		if (!p2)
			break;
		else {
			eso_strncpy(key, p1+2, ES_MIN(p2-p1-2, (int)sizeof(key)));
			const char *ptmp = cfg->getString(key);
			if (ptmp)
				eso_mstrcat(parsed, ptmp);
			pstr = p2 + 1;
		}
	}
	eso_mstrcat(parsed, pstr);
	ESO_MFREE(&strdup);
}

EConfig::EConfig() : m_name("ROOT"), m_parent(null)
{
	m_vmap = new ESimpleMap(true, false);
	m_cmap = new ESimpleMap(true, false);
}

EConfig::EConfig(const char *file) : m_name("ROOT"), m_parent(null)
{
	m_vmap = new ESimpleMap(true, false);
	m_cmap = new ESimpleMap(true, false);

	this->load(file);
}

EConfig::EConfig(const char* name, EConfig* parent) : m_name(name), m_parent(parent)
{
	m_vmap = new ESimpleMap(true, false);
	m_cmap = new ESimpleMap(true, false);
}

EConfig::~EConfig()
{
	delete m_vmap;
	delete m_cmap;
}

void EConfig::clear() {
	m_vmap->clear();
	m_cmap->clear();
	m_name.clear();
	m_parent = null;
}

boolean EConfig::isEmpty() {
	return (!m_vmap->size() && !m_cmap->size());
}

void EConfig::load(const char* file) {
	EFileInputStream fs(file);
	load(&fs);
}

void EConfig::load(EFile *file) {
	EFileInputStream fs(file);
	load(&fs);
}

void EConfig::load(EInputStream* in) {
	EDataInputStream dis(in);
	this->clear();

	EConfig *parent = null;
	EConfig *current = this;

	es_string_t *strkey = (es_string_t*)eso_mcalloc(1);
	es_string_t *strval = (es_string_t*)eso_mcalloc(1);

	sp<EString> line;
	while ((line = dis.readLine()) != null) {
		/* trim left&right invalid char */
		line->trim(TRIM_CHARS);
        
        if (line->isEmpty()) {
            continue;
        }

		/* ignore space line and annotate line */
		char c0 = line->charAt(0);
		if (c0 == 0 || c0 == '#' || c0 == ';') {
			continue;
		}
		else {
			/* "key = value" or "key" */

			if (c0 == '{') {
				parent = current;
				current = new EConfig(strkey, parent);
				parent->m_cmap->put(strkey, current);
			}
			else if (c0 == '}') {
				current = parent;
				parent = (parent ? parent->m_parent : null);
			}
			else {
				char *pstart = NULL;
				char *pend = NULL;

				int pos = line->indexOf('=');
				if (pos >= 0) {
					eso_mstrncpy(&strkey, line->c_str(), pos);
					eso_mstrcpy(&strval, line->c_str() + pos + 1);
				} else {
					eso_mstrcpy(&strkey, line->c_str());
					eso_mstrcpy(&strval, "");
				}
				eso_mtrims(strkey, TRIM_CHARS);
				eso_mtrims(strval, TRIM_CHARS);

				/* 搜索'='后第一个有效字符是否为'"' */
				pstart = strval;
				if (*pstart == '{') {
					parent = current;
					current = new EConfig(strkey, parent);
					parent->m_cmap->put(strkey, current);
				}
				else if (*pstart == '"') {
					pend = eso_strrchr(pstart + 1, '"');
					if (pend) {
						pstart++;
						current->m_vmap->put(strkey, new EString(pstart, 0, pend-pstart));
					}
					else {
						current->m_vmap->put(strkey, new EString(pstart));
					}
				}
				else if (*pstart) {
					if ((pend = eso_strpbrk(strval, "#;")) != 0) {
						*pend = 0; //去除尾部注释
					}

					env_value_parse(&strval, strval, this);
					cfg_value_parse(&strval, strval, this);

					current->m_vmap->put(strkey, new EString(strval));
				}
				else { //仅key值
					current->m_vmap->put(strkey, null);
				}
			}
		}
	}
	ESO_MFREE(&strkey);
	ESO_MFREE(&strval);
}

void EConfig::loadFromINI(const char* file) {
	EFileInputStream fs(file);
	loadFromINI(&fs);
}

void EConfig::loadFromINI(EFile* file) {
	EFileInputStream fs(file);
	loadFromINI(&fs);
}

void EConfig::loadFromINI(EInputStream* in) {
	EDataInputStream dis(in);
	this->clear();

	EConfig* current = this;

	es_string_t *strkey = (es_string_t*)eso_mcalloc(1);
	es_string_t *strval = (es_string_t*)eso_mcalloc(1);

	sp<EString> line;
	while ((line = dis.readLine()) != null) {
		/* trim left&right invalid char */
		line->trim(TRIM_CHARS);
        
        if (line->isEmpty()) {
            continue;
        }

		/* ignore space line and annotate line */
		char c0 = line->charAt(0);
		if (c0 == 0 || c0 == '#' || c0 == ';') {
			continue;
		}
		else if (c0 == '[') {
			int index = line->lastIndexOf(']');
			if (index > 0) {
				EString key = line->substring(1, index);
				current = new EConfig(key.c_str(), this);
				this->m_cmap->put(key.c_str(), current);
			}
		}
		else {
			/* "key = value" or "key" */

			char *pstart = NULL;
			char *pend = NULL;

			int pos = line->indexOf('=');
			if (pos >= 0) {
				eso_mstrncpy(&strkey, line->c_str(), pos);
				eso_mstrcpy(&strval, line->c_str() + pos + 1);
			} else {
				eso_mstrcpy(&strkey, line->c_str());
				eso_mstrcpy(&strval, "");
			}
			eso_mtrims(strkey, TRIM_CHARS);
			eso_mtrims(strval, TRIM_CHARS);

			/* 搜索'='后第一个有效字符是否为'"' */
			pstart = strval;
			if (*pstart == '"') {
				pend = eso_strrchr(pstart + 1, '"');
				if (pend) {
					pstart++;
					current->m_vmap->put(strkey, new EString(pstart, 0, pend-pstart));
				}
				else {
					current->m_vmap->put(strkey, new EString(pstart));
				}
			}
			else if (*pstart) {
				if ((pend = eso_strpbrk(strval, "#;")) != 0) {
					*pend = 0; //去除尾部注释
				}

				env_value_parse(&strval, strval, this);
				cfg_value_parse(&strval, strval, this);

				current->m_vmap->put(strkey, new EString(strval));
			}
			else { //仅key值
				current->m_vmap->put(strkey, null);
			}
		}
	}
	ESO_MFREE(&strval);
	ESO_MFREE(&strkey);
}

const char* EConfig::getString(const char* path, const char* defval) {
	EString* str = findString(path);
	return str ? str->c_str() : defval;
}

int EConfig::getInt(const char* path, int defval) {
	EString* str = findString(path);
	return str ? EInteger::parseInt(str->c_str()) : defval;
}

llong EConfig::getLLong(const char* path, llong defval) {
	EString* str = findString(path);
	return str ? ELLong::parseLLong(str->c_str()) : defval;
}

boolean EConfig::getBoolean(const char* path, boolean defval) {
	EString* str = findString(path);
	return str ? EBoolean::parseBoolean(str->c_str()) : defval;
}

float EConfig::getFloat(const char* path, float defval) {
	EString* str = findString(path);
	return str ? EFloat::parseFloat(str->c_str()) : defval;
}

double EConfig::getFloat(const char* path, double defval) {
	EString* str = findString(path);
	return str ? EDouble::parseDouble(str->c_str()) : defval;
}

EConfig* EConfig::getConfig(const char* path) {
	if (!path || !*path) {
		return null;
	}

	EString paths(path);
	paths.trim('/');

	EConfig* config = this;
	EArray<EString*> keys = EPattern::split("/", paths.c_str(), 0);
	int index = 0;
	for (; config && (index < keys.length()); index++) {
		config = config->findConfig(keys[index]->c_str());
	}
	return config;
}

EArray<EConfig*> EConfig::getConfigs(const char *path) {
	EArray<EConfig*> list(false);

	if (!path || !*path) {
		return list;
	}

	EString paths(path);
	paths.trim('/');

	EConfig* config = this;
	EArray<EString*> keys = EPattern::split("/", paths.c_str(), 0);
	int index = 0;
	for (; config && (index < keys.length() - 1); index++) { //-1
		config = config->findConfig(keys[index]->c_str());
	}
	if (config) {
		const char* element = keys[index]->c_str();
		if (!element) {
			return list;
		}

		char* p = (char*)eso_strchr(element, CFG_POSFLAG);
		if (p) {
			config = config->findConfig(element);
			if (config) list.add(config);
		}
		else if (config->m_cmap) {
			ESimpleEnumeration* e = config->m_cmap->elements();
			for (; e && e->hasMoreElements();) {
				es_emap_elem_t elem;
				e->nextElement(&elem);

				if (eso_strcmp(elem.key_str, element) == 0) {
					config = dynamic_cast<EConfig*>(elem.data);
					if (config) list.add(config);
				}
			}
		}
	}
	return list;
}

EArray<EString*> EConfig::keyNames(KeyType type) {
	EArray<EString*> arr(true);
	if (type == ALL || type == KV) {
		ESimpleEnumeration* e = m_vmap->elements();
		for (; e && e->hasMoreElements();) {
			es_emap_elem_t elem;
			e->nextElement(&elem);
			arr.add(new EString(elem.key_str));
		}
	}
	if (type == ALL || type == KC) {
		ESimpleEnumeration* e = m_cmap->elements();
		for (; e && e->hasMoreElements();) {
			es_emap_elem_t elem;
			e->nextElement(&elem);
			arr.add(new EString(elem.key_str));
		}
	}
	return arr;
}

const char* EConfig::getName() {
	return m_name.c_str();
}

EString EConfig::toString() {
	EString out;

	if (m_parent == null) {
		out.append("## Config output:\n\n");
	}

	ESimpleEnumeration *e = m_vmap->elements();
	for (; e && e->hasMoreElements();) {
		es_emap_elem_t elem;
		e->nextElement(&elem);
		out.append(elem.key_str);
		EString* s = dynamic_cast<EString*>(elem.data);
		if (s) {
			out.append(" = ").append(s);
		}
		out.append("\n");
	}
	out.rtrim("\n");

	e = m_cmap->elements();
	for (; e && e->hasMoreElements();) {
		es_emap_elem_t elem;
		e->nextElement(&elem);
		EConfig* c = dynamic_cast<EConfig*>(elem.data);
		if (c) {
			out.append("\n\n").append(elem.key_str).append(" = {\n\t").append(c->toString().replace("\n", "\n\t").c_str()).append("\n}");
		}
		else {
			out.append(elem.key_str).append(" = ").append("{}").append("\n");
		}
	}

	return out;
}

EString* EConfig::findString(const char* path) {
	if (!path || !*path) {
		return null;
	}

	EString paths(path);
	paths.trim('/');

	EConfig* config = this;
	EArray<EString*> keys = EPattern::split("/", paths.c_str(), 0);
	int index = 0;
	for (; config && (index < keys.length() - 1); index++) {
		config = config->findConfig(keys[index]->c_str());
	}
	if (!config) return null;

	const char* key = keys[index]->c_str();
	int idx = CFG_NODE_INDEX_FIRST;
	char* p = (char*)eso_strchr(key, CFG_POSFLAG);
	if (p) {
		*p = 0;
		idx = eso_atol(p+1);
	}
	return dynamic_cast<EString*>(config->m_vmap->get(key, idx-1));
}

EConfig* EConfig::findConfig(const char* element) {
	if (!element) {
		return null;
	}

	const char* key = element;
	int idx = CFG_NODE_INDEX_FIRST;
	char* p = (char*)eso_strchr(key, CFG_POSFLAG);
	if (p) {
		*p = 0;
		idx = eso_atol(p+1);
	}
	return dynamic_cast<EConfig*>(m_cmap->get(key, idx-1));
}

} /* namespace efc */
