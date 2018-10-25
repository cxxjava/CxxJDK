/**
 * @file  eso_conf.c
 * @brief ES Config Routines
 */

#include "eso_conf.h"
#include "eso_string.h" 
#include "eso_file.h" 

static void line_trim(es_string_t *line)
{
	eso_mtrim(line, ' ');
	eso_mtrim(line, '\t');
	eso_mtrim(line, '\r');
	eso_mtrim(line, '\n');
}

static void env_value_parse(es_string_t **parsed, 
                            const char *envstr, 
                            es_bson_t *bson)
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
			char *ptmp = NULL;
			eso_strncpy(envkey, p1+2, ES_MIN(p2-p1-1, (int)sizeof(envkey)));
			ptmp = getenv(envkey);
			if (ptmp)
				eso_mstrcat(parsed, ptmp);
			pstr = p2 + 1;
		}
	}
	eso_mstrcat(parsed, pstr);
	ESO_MFREE(&envstrdup);
}

static void bson_value_parse(es_string_t **parsed,
                            const char *bsonstr,
                            es_bson_t *bson)
{
	es_string_t *bsonstrdup = eso_mstrdup(bsonstr);
	char *pstr = (char *)bsonstrdup;
	char bsonkey[256] = {0};
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
			char *ptmp = NULL;
			eso_strncpy(bsonkey, p1+2, ES_MIN(p2-p1-1, (int)sizeof(bsonkey)));
			ptmp = eso_bson_get_str(bson, bsonkey);
			if (ptmp)
				eso_mstrcat(parsed, ptmp);
			pstr = p2 + 1;
		}
	}
	eso_mstrcat(parsed, pstr);
	ESO_MFREE(&bsonstrdup);
}

es_bool_t eso_conf_load(es_bson_t *bson,
                                     const char *fname)
{
	es_file_t *conf_file = NULL;
	es_string_t *buffer = NULL;
	es_uint16_t buffer_size = 4096;
	es_uint32_t read_pos = 0;
	es_bson_node_t *parent_node = NULL;
	es_bson_node_t *curr_node = NULL;
	es_string_t *full_bsonpath = NULL;
	char *pstart = NULL;
	char *pend = NULL;
	
	if (!bson) return FALSE;
	
	conf_file = eso_fopen(fname, "rb");
	if (!conf_file) {
		return FALSE;
	}
	
	full_bsonpath = eso_mcalloc(4096);
	
	buffer = eso_mmalloc(buffer_size);
	while (!eso_feof(conf_file)) {
		char *p = eso_fgets(buffer + read_pos, eso_mnode_size(buffer) - read_pos, conf_file);
		if (!p) {
			break;
		}
		
		/* Check if we got all of the line. */
		if (!eso_feof(conf_file)) {
				pend = eso_strchr(buffer, '\n');
				if (!pend) {
					read_pos = eso_mnode_size(buffer) - 1;
					buffer = eso_mrealloc(buffer, eso_mnode_size(buffer) * 2);
					
					continue;
				}
		}
		read_pos = 0;
		
		/* 去前后无效字符 */
		line_trim(buffer);
		
		/* 空行或者注释行则忽略 */
		if (buffer[0] == 0 || buffer[0] == '#' || buffer[0] == ';') {
			continue;
		}
		else {
			/* "key = value" or "key" */
			es_string_t *strkey = eso_mcalloc(1);
			es_string_t *strval = eso_mcalloc(1);
			
			if (buffer[0] == '{') {
				parent_node = curr_node;
			}
			else if (buffer[0] == '}') {
				parent_node = (parent_node ? parent_node->parent : NULL);
			}
			else {
				if (parent_node) {
					eso_bson_node_path(parent_node, full_bsonpath, eso_mnode_size(full_bsonpath));
				}
				else {
					eso_mmemfill(full_bsonpath, 0);
				}
				
				eso_mstrsplit(buffer, "=", 1, &strkey);
				line_trim(strkey);
				eso_mstrcat(&full_bsonpath, "/");
				eso_mstrcat(&full_bsonpath, strkey);
				
				eso_mstrsplit(buffer, "=", 2, &strval);
				if ((pend = eso_strpbrk(strval, "#;")) != 0) {
					*pend = 0; //去除尾部注释
				}
				line_trim(strval);
				env_value_parse(&strval, strval, bson);
				bson_value_parse(&strval, strval, bson);
				
				/* 搜索'='后第一个有效字符是否为'"' */
				pstart = strval;
				if (*pstart == '{') {
					curr_node = eso_bson_add_str(bson, full_bsonpath, "");
					parent_node = curr_node;
				}
				else if (*pstart == '"') {
					pend = eso_strrchr(pstart + 1, '"');
					if (pend) {
						pstart++;
						curr_node = eso_bson_add_bin(bson, full_bsonpath, pstart, pend-pstart);
					}
					else {
						curr_node = eso_bson_add_str(bson, full_bsonpath, pstart);
					}
				}
				else {
					curr_node = eso_bson_add_str(bson, full_bsonpath, pstart);
				}
			}
			
			ESO_MFREE(&strkey);
			ESO_MFREE(&strval);
		}
	} //!while
	
	eso_fclose(conf_file);
	ESO_MFREE(&buffer);
	ESO_MFREE(&full_bsonpath);
	
	return TRUE;
}
