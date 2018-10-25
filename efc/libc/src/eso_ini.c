/**
 * @file  eso_ini.h
 * @brief ES Inifile Routines
 */

#include "eso_ini.h"
#include "eso_libc.h"
#include "eso_util.h"
#include "eso_string.h"

struct es_inifile_t
{
	char          fname[ES_PATH_MAX];
	es_ini_sec_t *top;
	es_int32_t    flag;   /* ES_INI_READ or ES_INI_WRITE */
	es_bool_t     updated;   /* has updated */
};

struct es_ini_sec_t
{
	char          name[32];
	es_ini_key_t *keys;
	es_ini_sec_t *next;
};

struct es_ini_key_t
{
	char          name[32];
	es_string_t  *value;
	es_ini_key_t *next;
};

static void ini_line_trim(es_string_t *line)
{
	char *p;
	int count = 0;
	int str_len = eso_strlen(line);
	
	p = line + str_len - 1;
	while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') {
		*p = '\0';
		p--;
		count++;
	}
	
	p = line;
	while (*p == ' ' || *p == '\t') {
		p++;
		count++;
	}
	if (p != line) {
		eso_memmove(line, p, str_len-count);
		line[str_len-count] = '\0';
	}
}

static void pfree_key(es_ini_key_t *key)
{
	es_ini_key_t *ref_curr = key;
	es_ini_key_t *ref_next = NULL;
	
	if (!key)
		return;
	
	while (ref_curr) {
		ref_next = ref_curr->next;
		
		eso_free(ref_curr->value);
		eso_free(ref_curr);
		
		ref_curr = ref_next;
	}
}

static void pfree_sec(es_ini_sec_t *sec)
{
	es_ini_sec_t *ref_curr = sec;
	es_ini_sec_t *ref_next = NULL;
	
	if (!sec)
		return;
	
	while (ref_curr) {
		ref_next = ref_curr->next;
		
		if (ref_curr->keys) {
			pfree_key(ref_curr->keys);
			ref_curr->keys = NULL;
		}
		
		eso_free(ref_curr);
		
		ref_curr = ref_next;
	}
}

static int ini_update(es_file_t *file, es_inifile_t *ini)
{
	es_ini_sec_t *sec;
	es_ini_key_t *key_;
	
	/* section */
	for (sec = eso_ini_sec_first(ini); sec; sec = eso_ini_sec_next(sec)) {
		/* section name */
		eso_fputs("[", file);
		eso_fputs(sec->name, file);
		eso_fputs("]\n", file);
		
		/*key*/		
		for (key_ = eso_ini_key_first(sec); key_; key_ = eso_ini_key_next(key_)) {
			/* key = value */
			eso_fputs(key_->name, file);
			eso_fputs(" = \"", file);
			eso_fputs(key_->value, file);
			eso_fputs("\"\n", file);
		}
		eso_fputs("\n", file);
	}
	return 0;
}

es_inifile_t* eso_inifile_open(const char *fname, es_int32_t flag)
{
	es_inifile_t *pini;
	es_file_t *ini_file = NULL;
	es_ini_sec_t *curr_sec = NULL, *last_sec = NULL;
	es_ini_key_t *curr_key = NULL, *last_key = NULL;
	char *p = NULL;
	char *end = NULL;
	es_uint16_t buffer_size = 256;
	es_uint16_t read_pos = 0;
	es_string_t *buffer = NULL;
	
	pini = (es_inifile_t *)eso_malloc(sizeof(es_inifile_t));
	if (!pini)
		return NULL;
	pini->top = NULL;
	pini->flag = flag;
	pini->updated = FALSE;
	eso_strncpy(pini->fname, fname, sizeof(pini->fname));
	
	ini_file = eso_fopen(fname, "rb");
	if (!ini_file && flag == ES_INI_RDWR) {
		ini_file = eso_fopen(fname, "wb");
	}
	if (!ini_file) {
		goto free_all_return;
	}
	
	buffer = eso_malloc(buffer_size);
	while (!eso_feof(ini_file)) {
		p = eso_fgets(buffer + read_pos, buffer_size-read_pos-1, ini_file);
		if (p == NULL || eso_strlen(buffer) == 0) {
			continue;
		}
		else {
			/* Check if we got all the line. */
			if (!eso_feof(ini_file)) {
				end = eso_strchr(buffer, '\n');
				if (!end) {
					buffer_size += 256;
					buffer = eso_realloc(buffer, buffer_size);
					read_pos = eso_strlen(buffer);
				
					continue;
				}
			}
		}
		read_pos = 0;
		
		/* 去前后无效字符 */
		ini_line_trim(buffer);
		
		/* 空行则忽略 */
		if (buffer[0] == 0)
			continue;
		
		if (buffer[0] == '#' || buffer[0] == ';') {
			//ignore!
		}
		else if (buffer[0] == '[') {
			end = eso_strchr(buffer, ']');
			if (!end) {
				goto free_all_return;
			}
			curr_sec = (es_ini_sec_t *)eso_calloc(sizeof(es_ini_sec_t));;
			curr_sec->keys = NULL;
			eso_strncpy(curr_sec->name, buffer + 1, ES_MIN(end-buffer-1, (int)sizeof(curr_sec->name)));

			if (!pini->top) {
				pini->top = curr_sec;
			}
			else {
				last_sec->next = curr_sec;
			}
			last_sec = curr_sec;
			curr_key = NULL;
			
			/* [sec]后面还有'#' or ';' ? */
			if ((end = eso_strpbrk(end, "#;")) != 0) {
				//ignore!
			}
		}
		else {
			/* "key = value" or "key" */
			char *pEqual = NULL;
			
			pEqual = eso_strchr(buffer, '=');
			
			/* 忽略最前面无效的行 */
			if (!curr_sec)
				continue;
			
			curr_key = (es_ini_key_t *)eso_calloc(sizeof(es_ini_key_t));
			if (!curr_sec->keys) {
				curr_sec->keys = curr_key;
			}
			else {
				last_key->next = curr_key;
			}
			last_key = curr_key;
			
			if (pEqual) {
				eso_strncpy(curr_key->name, buffer, ES_MIN(pEqual-buffer, (int)sizeof(curr_key->name)));
				ini_line_trim(curr_key->name);
				
				/* 搜索'='后第一个有效字符是否为'"' */
				p = pEqual + 1;
				while (*p == ' ' || *p == '\t') {
					p++;
				}
				if (*p == '"') {
					end = eso_strrchr(p + 1, '"');
					if (end) {
						curr_key->value = eso_calloc(end - p);
						eso_strncpy(curr_key->value, p + 1, end - p - 1);
						
						/* 后面还有'#' or ';' ? */
						if ((end = eso_strpbrk(end, "#;")) != 0) {
							//ignore!
						}
					}
					else {
						curr_key->value = eso_calloc(eso_strlen(p)+1);
						eso_strcpy(curr_key->value, p + 1);
					}
				}
				else {
					/* 后面有'#' or ';' ? */
					if ((end = eso_strpbrk(p, "#;")) != 0) {
						curr_key->value = eso_calloc(end - p + 1);
						eso_strncpy(curr_key->value, p, end - p);
					}
					else {
						curr_key->value = eso_calloc(eso_strlen(p)+1);
						eso_strcpy(curr_key->value, p);
					}
					
					ini_line_trim(curr_key->value);
				}
			}
			else {
				//only "key"
				
				/* 后面有'#' or ';' ? */
				if ((end = eso_strpbrk(buffer, "#;")) != 0) {
					eso_strncpy(curr_key->name, buffer, ES_MIN(end-buffer, (int)sizeof(curr_key->name)));
				}
				else {
					eso_strncpy(curr_key->name, buffer, sizeof(curr_key->name));
				}
				
				ini_line_trim(curr_key->name);
			}
		}
	} /* end of while() */
	
	eso_free(buffer);
	eso_fclose(ini_file);
	return pini;
	
free_all_return:
	if (ini_file) eso_fclose(ini_file);
	eso_free(buffer);
	//free top
	pfree_sec(pini->top);
	eso_free(pini);
	
	return NULL;
}

void eso_inifile_close(es_inifile_t **ini)
{
	es_inifile_t *theini = *ini;
	es_file_t *ini_file = NULL;
	
	if (!theini)
		return;
	
	if ((theini->flag == ES_INI_RDWR)
		&& theini->updated) {
		ini_file = eso_fopen(theini->fname, "wb");
		if (!ini_file) {
			goto FAIL;
		}
		
		ini_update(ini_file, theini);
		eso_fclose(ini_file);
	}
	
	if (theini->top) {
		pfree_sec(theini->top);
		theini->top = NULL;
	}
	
FAIL:
	eso_free(theini);
	*ini = NULL;
}

es_ini_sec_t* eso_ini_sec_first(es_inifile_t *ini)
{
    if (!ini)
    	return NULL;
    else
		return ini->top;
}

es_ini_sec_t* eso_ini_sec_next(es_ini_sec_t *sec)
{
    if (!sec)
    	return NULL;
    else
		return sec->next;
}

char* eso_ini_sec_get_name(es_ini_sec_t *sec)
{
    if (!sec)
    	return NULL;
    else
		return sec->name;
}

es_ini_key_t* eso_ini_key_first(es_ini_sec_t *sec)
{
    if (!sec)
    	return NULL;
    else
		return sec->keys;
}

es_ini_key_t* eso_ini_key_next(es_ini_key_t *key)
{
    if (!key)
    	return NULL;
    else
		return key->next;
}

char* eso_ini_key_get_name(es_ini_key_t *key)
{
    if (!key)
    	return NULL;
    else
		return key->name;
}

char* eso_ini_key_get_value(es_ini_key_t *key)
{
    if (!key)
    	return NULL;
    else
		return key->value;
}

es_ini_sec_t* eso_ini_get_section(es_inifile_t *ini,
                                              const char *section,
                                              es_int32_t idx_sec)
{
	es_ini_sec_t *sec;
	int count = 0;
	
	if (!ini || !section || !*section)
		return NULL;
	
	for (sec = eso_ini_sec_first(ini); sec; sec = eso_ini_sec_next(sec)) {
		if (eso_strcmp(sec->name, section) == 0) {
			count++;
		}
		if (idx_sec == count)
			return sec;
	}
	
	return NULL;
}

es_ini_key_t* eso_ini_get_key(es_ini_sec_t *sec,
                                              const char *key,
                                              es_int32_t idx_key)
{
	es_ini_key_t *key_;
	int count = 0;
	
	if (!sec || !key || !*key)
		return NULL;
	
	for (key_ = eso_ini_key_first(sec); key_; key_ = eso_ini_key_next(key_)) {
		if (eso_strcmp(key_->name, key) == 0) {
			count++;
		}
		if (idx_key == count)
			return key_;
	}
	
	return NULL;
}

char* eso_ini_read_string(es_inifile_t *ini,
                                      const char *section,
                                      const char *key,
                                      char *def)
{
	return 	eso_ini_read_string2(ini, section, 1, key, 1, def);
}

char* eso_ini_read_string2(es_inifile_t *ini,
                                      const char *section,
                                      es_int32_t idx_sec,
                                      const char *key,
                                      es_int32_t idx_key,
                                      char *def)
{
	es_ini_sec_t *sec_;
	es_ini_key_t *key_;
	
	if (!ini || !section || !*section || !key || !*key)
		return NULL;
	
	sec_ = eso_ini_get_section(ini, section, idx_sec);
	if (!sec_)
		return def;
	
	key_ = eso_ini_get_key(sec_, key, idx_key);
	if (!key_)
		return def;
	
	return eso_ini_key_get_value(key_);
}

char* eso_ini_read_string3(es_ini_sec_t *sec,
                                      const char *key,
                                      char *def)
{
	es_ini_key_t *key_;
	
	if (!sec || !key || !*key)
		return NULL;
	
	key_ = eso_ini_get_key(sec, key, 1);
	if (!key_)
		return def;
	
	return eso_ini_key_get_value(key_);
}

es_ini_sec_t* eso_ini_add_sec(es_inifile_t *ini, const char *section)
{
	es_ini_sec_t *sec;
	
	if (!ini || !section || !*section)
		return NULL;
	
	sec = (es_ini_sec_t *)eso_calloc(sizeof(es_ini_sec_t));
	if (!sec)
		return NULL;
	eso_strncpy(sec->name, section, sizeof(sec->name));
	sec->next = ini->top;
	ini->top = sec;
	
	ini->updated = TRUE;
	
	return sec;
}

/**
 * add new es_ini_key_t append to index section, index begin 1
 * return new es_ini_key_t
 */
es_ini_key_t* eso_ini_add_key(es_inifile_t *ini,
                                      const char *section,
                                      es_int32_t idx_sec,
                                      const char *key,
                                      const char *value)
{
	es_ini_sec_t *sec;
	es_ini_key_t *key_;
	
	if (!ini || !section || !*section || !key || !*key)
		return NULL;
	
	sec = eso_ini_get_section(ini, section, idx_sec);
	if (!sec) {
		sec = eso_ini_add_sec(ini, section);
		if (!sec)
			return NULL;
	}
	
	key_ = (es_ini_key_t *)eso_calloc(sizeof(es_ini_key_t));
	if (!key_)
		return NULL;
	eso_strncpy(key_->name, key, sizeof(key_->name));
	if (value)
		key_->value = eso_mstrdup(value);
	key_->next = sec->keys;
	sec->keys = key_;
	
	ini->updated = TRUE;
	
	return key_;
}

/**
 * delete first section in inifile
 */
void eso_ini_del_sec(es_inifile_t *ini,
                                      const char *section)
{
	eso_ini_del_sec2(ini, section, 1);
}

/**
 * delete es_ini_sec_t in inifile by section name and index,
 * index begin 1
 */
void eso_ini_del_sec2(es_inifile_t *ini,
                                      const char *section,
                                      es_int32_t idx_sec)
{
	es_ini_sec_t *sec, *ref;
	
	if (!ini || !section || !*section)
		return;
	
	sec = eso_ini_get_section(ini, section, idx_sec);
	if (!sec)
		return;
	
	if (ini->top == sec)
		ini->top = sec->next;
	else
	for (ref = eso_ini_sec_first(ini); ref; ref = eso_ini_sec_next(ref)) {
		if (ref->next == sec) {
			ref->next = sec->next;
			break;
		}
	}
	
	sec->next = NULL;
	pfree_sec(sec);
	
	ini->updated = TRUE;
	
	return;
}

/**
 * delete first key in first section
 */
void eso_ini_del_key(es_inifile_t *ini,
                                      const char *section,
                                      const char *key)
{
	eso_ini_del_key2(ini, section, 1, key, 1);
}

/**
 * delete key by key index and section index,
 * index begin 1
 */
void eso_ini_del_key2(es_inifile_t *ini,
                                      const char *section,
                                      es_int32_t idx_sec,
                                      const char *key,
                                      es_int32_t idx_key)
{
	es_ini_sec_t *sec;
	es_ini_key_t *key_, *ref;
	
	if (!ini || !section || !*section || !key || !*key)
		return;
	
	sec = eso_ini_get_section(ini, section, idx_sec);
	if (!sec)
		return;
	
	key_ = eso_ini_get_key(sec, key, idx_key);
	if (!key_)
		return;
	
	if (sec->keys == key_)
		sec->keys = key_->next;
	else
	for (ref = eso_ini_key_first(sec); ref; ref = eso_ini_key_next(ref)) {
		if (ref->next == key_) {
			ref->next = key_->next;
			break;
		}
	}
	
	key_->next = NULL;
	pfree_key(key_);
	
	ini->updated = TRUE;
	
	return;
}

/**
 * update first key value in first section
 */
es_bool_t eso_ini_upd_key(es_inifile_t *ini,
                                      const char *section,
                                      const char *key,
                                      const char *value)
{
	return eso_ini_upd_key2(ini, section, 1, key, 1, value);
}

/**
 * update key value by key index and section index,
 * index begin 1
 */
es_bool_t eso_ini_upd_key2(es_inifile_t *ini,
                                      const char *section,
                                      es_int32_t idx_sec,
                                      const char *key,
                                      es_int32_t idx_key,
                                      const char *value)
{
	es_ini_sec_t *sec;
	es_ini_key_t *key_;
	
	if (!ini || !section || !*section || !key || !*key)
		return FALSE;
	
	sec = eso_ini_get_section(ini, section, idx_sec);
	if (!sec) {
		sec = eso_ini_add_sec(ini, section);
		if (!sec)
			return FALSE;
	}

	key_ = eso_ini_get_key(sec, key, idx_key);
	if (!key_) {
		key_ = eso_ini_add_key(ini, section, idx_sec, key, "");
		if (!key_)
			return FALSE;
	}
	
	if (value) {
		eso_mstrcpy(&key_->value, value);
	}
	
	ini->updated = TRUE;
	
	return TRUE;
}
