/**
 * @file  eso_json.c
 * @brief ES JSON parser.
 */

#include "eso_json.h"
#include "eso_util.h"
#include "eso_string.h"
#include <math.h>
#include <float.h>
#include <limits.h>

//@see: http://sourceforge.net/projects/cjson/

#define JSON_IS_REFERENCE 256

/* Predeclare these prototypes. */
static const char *parse_number(es_json_t *item, const char *num);
static char *print_number(es_json_t *item);
static const char *parse_string(es_json_t *item, const char *str, const char **return_parse_end);
static char *print_string_ptr(const char *str);
static char *print_string(es_json_t *item);
static const char *parse_value(es_json_t *item, const char *value, const char **return_parse_end);
static char *print_value(es_json_t *item, int depth, int fmt);
static const char *parse_array(es_json_t *item, const char *value, const char **return_parse_end);
static char *print_array(es_json_t *item, int depth, int fmt);
static const char *parse_object(es_json_t *item, const char *value, const char **return_parse_end);
static char *print_object(es_json_t *item, int depth, int fmt);

/* Internal constructor. */
static es_json_t *json_new_item(void)
{
	es_json_t* node = (es_json_t*)eso_malloc(sizeof(es_json_t));
	if (node) {
		eso_memset(node, 0, sizeof(es_json_t));
	}
	return node;
}

/* Utility to jump whitespace and cr/lf */
static const char *skip(const char *in) {
	while (in && *in && (unsigned char)*in<=32) in++;
	return in;
}

/* Utility for array list handling. */
static void suffix_object(es_json_t *prev, es_json_t *item) {
	prev->next = item;
	item->prev = prev;
}

/* Utility for handling references. */
static es_json_t *create_reference(es_json_t *item) {
	es_json_t *ref = json_new_item();
	if (!ref) return 0;
	eso_memcpy(ref, item, sizeof(es_json_t));
	ref->string = 0;
	ref->type |= JSON_IS_REFERENCE;
	ref->next = ref->prev = 0;
	return ref;
}

/* Parse the input text to generate a number, and populate the result into item. */
static const char *parse_number(es_json_t *item, const char *num)
{
	double n=0,sign=1,scale=0;int subscale=0,signsubscale=1;

	/* Could use sscanf for this? */
	if (*num=='-') sign=-1,num++;	/* Has sign? */
	if (*num=='0') num++;			/* is zero */
	if (*num>='1' && *num<='9')	{
		do n=(n*10.0)+(*num++ -'0'); while (*num>='0' && *num<='9');
	}	/* Number? */
	if (*num=='.' && num[1]>='0' && num[1]<='9') {
		num++;
		do n=(n*10.0)+(*num++ -'0'),scale--; while (*num>='0' && *num<='9');
	}	/* Fractional part? */
	if (*num=='e' || *num=='E') {
		num++;
		if (*num=='+') num++; else if (*num=='-') signsubscale=-1,num++;		/* With sign? */
		while (*num>='0' && *num<='9') subscale=(subscale*10)+(*num++ - '0');	/* Number? */
	}	/* Exponent? */

	n = sign*n*pow(10.0,(scale+subscale*signsubscale));	/* number = +/- number.fraction * 10^+/- exponent */
	
	item->valuedouble = n;
	item->valueint = (int)n;
	item->type = ES_JSON_NUMBER;
	return num;
}

/* Render the number nicely from the given item into a string. */
static char *print_number(es_json_t *item)
{
	char *str;
	double d = item->valuedouble;
	if (fabs(((double)item->valueint)-d)<=DBL_EPSILON && d<=INT_MAX && d>=INT_MIN) {
		str = (char*)eso_malloc(21);	/* 2^64+1 can be represented in 21 chars. */
		if (str)                                      eso_sprintf(str,"%d",item->valueint);
	}
	else {
		str=(char*)eso_malloc(64);	/* This is a nice tradeoff. */
		if (str) {
			if (fabs(floor(d)-d)<=DBL_EPSILON && fabs(d)<1.0e60) eso_sprintf(str,"%.0f",d);
			else if (fabs(d)<1.0e-6 || fabs(d)>1.0e9) eso_sprintf(str,"%e",d);
			else                                      eso_sprintf(str,"%f",d);
		}
	}
	return str;
}

/* Parse the input text into an unescaped cstring, and populate item. */
static const unsigned char firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
static const char *parse_string(es_json_t *item, const char *str, const char **return_parse_end)
{
	const char *ptr=str+1;char *ptr2;char *out;int len=0;unsigned uc,uc2;
	if (*str!='\"') {
		if (return_parse_end) *return_parse_end=str;
		return 0;
	}	/* not a string! */
	
	while (*ptr!='\"' && *ptr && ++len) if (*ptr++ == '\\') ptr++;	/* Skip escaped quotes. */
	
	out = (char*)eso_malloc(len+1);	/* This is how long we need for the string, roughly. */
	if (!out) return 0;
	
	ptr = str+1;
	ptr2 = out;
	while (*ptr!='\"' && *ptr) {
		if (*ptr!='\\') *ptr2++=*ptr++;
		else {
			ptr++;
			switch (*ptr) {
				case 'b': *ptr2++='\b';	break;
				case 'f': *ptr2++='\f';	break;
				case 'n': *ptr2++='\n';	break;
				case 'r': *ptr2++='\r';	break;
				case 't': *ptr2++='\t';	break;
				case 'u':	 /* transcode utf16 to utf8. */
					sscanf(ptr+1,"%4x",&uc);ptr+=4;	/* get the unicode char. */

					if ((uc>=0xDC00 && uc<=0xDFFF) || uc==0) break;	// check for invalid.

					if (uc>=0xD800 && uc<=0xDBFF)	// UTF16 surrogate pairs.
					{
						if (ptr[1]!='\\' || ptr[2]!='u') break;	// missing second-half of surrogate.
						sscanf(ptr+3,"%4x",&uc2);ptr+=6;
						if (uc2<0xDC00 || uc2>0xDFFF) break;	// invalid second-half of surrogate.
						uc=0x10000 + (((uc&0x3FF)<<10) | (uc2&0x3FF));
					}

					len=4;if (uc<0x80) len=1;else if (uc<0x800) len=2;else if (uc<0x10000) len=3; ptr2+=len;
					
					switch (len) {
						case 4: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
						case 3: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
						case 2: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
						case 1: *--ptr2 =(uc | firstByteMark[len]);
					}
					ptr2+=len;
					break;
				default:  *ptr2++=*ptr; break;
			}
			ptr++;
		}
	}
	*ptr2=0;
	if (*ptr=='\"') ptr++;
	item->valuestring = out;
	item->type = ES_JSON_STRING;
	return ptr;
}

/* Render the cstring provided to an escaped version that can be printed. */
static char *print_string_ptr(const char *str)
{
	const char *ptr;char *ptr2,*out;int len=0;unsigned char token;
	
	if (!str) return eso_strdup("");
	ptr = str;
	while ((token=*ptr) && ++len) {if (eso_strchr("\"\\\b\f\n\r\t", token)) len++; else if (token<32) len+=5;ptr++;}
	
	out = (char*)eso_malloc(len+3);
	if (!out) return 0;
	
	ptr2 = out;
	ptr = str;
	*ptr2++ = '\"';
	while (*ptr) {
		if ((unsigned char)*ptr>31 && *ptr!='\"' && *ptr!='\\') {
			*ptr2++ = *ptr++;
		}
		else {
			*ptr2++='\\';
			switch (token=*ptr++)
			{
				case '\\':	*ptr2++='\\';	break;
				case '\"':	*ptr2++='\"';	break;
				case '\b':	*ptr2++='b';	break;
				case '\f':	*ptr2++='f';	break;
				case '\n':	*ptr2++='n';	break;
				case '\r':	*ptr2++='r';	break;
				case '\t':	*ptr2++='t';	break;
				default: eso_sprintf(ptr2,"u%04x",token);ptr2+=5;	break;	/* escape and print */
			}
		}
	}
	*ptr2++ = '\"';
	*ptr2++ = 0;
	return out;
}
/* Invote print_string_ptr (which is useful) on an item. */
static char *print_string(es_json_t *item)
{
	return print_string_ptr(item->valuestring);
}

/* Build an array from input text. */
static const char *parse_array(es_json_t *item, const char *value, const char **return_parse_end)
{
	es_json_t *child;
	
	if (*value != '[') {
		if (return_parse_end) *return_parse_end=value;
		return 0;
	}	/* not an array! */

	item->type = ES_JSON_ARRAY;
	value = skip(value+1);
	if (*value==']') return value+1;	/* empty array. */

	item->child = child = json_new_item();
	if (!item->child) return 0;		 /* memory fail */
	value = skip(parse_value(child, skip(value), return_parse_end));	/* skip any spacing, get the value. */
	if (!value) return 0;

	while (*value == ',') {
		es_json_t *new_item;
		if (!(new_item = json_new_item())) return 0; 	/* memory fail */
		child->next = new_item;
		new_item->prev = child;
		child = new_item;
		value = skip(parse_value(child, skip(value+1), return_parse_end));
		if (!value) return 0;	/* memory fail */
	}

	if (*value==']') return value+1;	/* end of array */
	if (return_parse_end) *return_parse_end = value;
	return 0;	/* malformed. */
}

/* Render an array to text */
static char *print_array(es_json_t *item, int depth, int fmt)
{
	char **entries;
	char *out=0,*ptr,*ret;int len=5;
	es_json_t *child=item->child;
	int numentries=0,i=0,fail=0;
	
	/* How many entries in the array? */
	while (child) numentries++,child=child->next;
	/* Explicitly handle numentries==0 */
	if (!numentries) {
		out = (char*)eso_malloc(3);
		if (out) eso_strcpy(out,"[]");
		return out;
	}
	/* Allocate an array to hold the values for each */
	entries = (char**)eso_malloc(numentries*sizeof(char*));
	if (!entries) return 0;
	eso_memset(entries, 0, numentries*sizeof(char*));
	/* Retrieve all the results: */
	child = item->child;
	while (child && !fail) {
		ret = print_value(child, depth+1, fmt);
		entries[i++] = ret;
		if (ret) {
			len += eso_strlen(ret)+2+(fmt?1:0);
		}
		else {
			fail = 1;
		}
		child = child->next;
	}
	
	/* If we didn't fail, try to malloc the output string */
	if (!fail) out = (char*)eso_malloc(len);
	/* If that fails, we fail. */
	if (!out) fail = 1;

	/* Handle failure. */
	if (fail) {
		for (i=0; i<numentries; i++) if (entries[i]) eso_free(entries[i]);
		eso_free(entries);
		return 0;
	}
	
	/* Compose the output array. */
	*out = '[';
	ptr = out+1;
	*ptr = 0;
	for (i=0; i<numentries; i++) {
		eso_strcpy(ptr, entries[i]);
		ptr += eso_strlen(entries[i]);
		if (i != numentries-1) {
			*ptr++ = ',';
			if (fmt) *ptr++ = ' ';
			*ptr=0;
		}
		eso_free(entries[i]);
	}
	eso_free(entries);
	*ptr++ = ']';
	*ptr++ = 0;
	return out;	
}

/* Build an object from the text. */
static const char *parse_object(es_json_t *item, const char *value, const char **return_parse_end)
{
	es_json_t *child;
	if (*value!='{') {
		if (return_parse_end) *return_parse_end=value;
		return 0;
	}	/* not an object! */
	
	item->type = ES_JSON_OBJECT;
	value = skip(value+1);
	if (*value=='}') return value+1;	/* empty array. */
	
	item->child = child = json_new_item();
	if (!item->child) return 0;
	value = skip(parse_string(child, skip(value), return_parse_end));
	if (!value) return 0;
	child->string = child->valuestring; child->valuestring = 0;
	if (*value!=':') {
		if (return_parse_end) *return_parse_end=value;
		return 0;
	}	/* fail! */
	value = skip(parse_value(child, skip(value+1), return_parse_end));	/* skip any spacing, get the value. */
	if (!value) return 0;
	
	while (*value == ',') {
		es_json_t *new_item;
		if (!(new_item = json_new_item())) return 0; /* memory fail */
		child->next = new_item;
		new_item->prev = child;
		child = new_item;
		value = skip(parse_string(child, skip(value+1), return_parse_end));
		if (!value) return 0;
		child->string = child->valuestring;
		child->valuestring = 0;
		if (*value!=':') {
			if (return_parse_end) *return_parse_end=value;
			return 0;
		}	/* fail! */
		value = skip(parse_value(child, skip(value+1), return_parse_end));	/* skip any spacing, get the value. */
		if (!value) return 0;
	}
	
	if (*value=='}') {
		return value+1;	/* end of array */
	}
	if (return_parse_end) *return_parse_end = value;
	return 0;	/* malformed. */
}

/* Render an object to text. */
static char *print_object(es_json_t *item, int depth, int fmt)
{
	char **entries=0,**names=0;
	char *out=0,*ptr,*ret,*str;int len=7,i=0,j;
	es_json_t *child=item->child;
	int numentries=0,fail=0;
	
	/* Count the number of entries. */
	while (child) numentries++,child=child->next;
	
	/* Explicitly handle empty object case */
	if (!numentries) {
		out = (char*)eso_malloc(fmt?depth+3:3);
		if (!out) return 0;
		ptr = out;*ptr++ = '{';
		if (fmt) {
			*ptr++ = '\n';
			for (i=0;i<depth-1;i++) *ptr++='\t';
		}
		*ptr++ = '}';
		*ptr++ = 0;
		return out;
	}
	/* Allocate space for the names and the objects */
	entries = (char**)eso_malloc(numentries*sizeof(char*));
	if (!entries) return 0;
	names = (char**)eso_malloc(numentries*sizeof(char*));
	if (!names) {eso_free(entries);return 0;}
	eso_memset(entries, 0, sizeof(char*)*numentries);
	eso_memset(names, 0, sizeof(char*)*numentries);

	/* Collect all the results into our arrays: */
	child = item->child;depth++;
	if (fmt) len += depth;
	while (child) {
		names[i] = str = print_string_ptr(child->string);
		entries[i++] = ret = print_value(child, depth, fmt);
		if (str && ret) {
			len += eso_strlen(ret)+eso_strlen(str)+2+(fmt?2+depth:0);
		}
		else {
			fail = 1;
		}
		child = child->next;
	}
	
	/* Try to allocate the output string */
	if (!fail) out = (char*)eso_malloc(len);
	if (!out) fail = 1;

	/* Handle failure */
	if (fail) {
		for (i=0;i<numentries;i++) {
			if (names[i]) eso_free(names[i]);
			if (entries[i]) eso_free(entries[i]);
		}
		eso_free(names);
		eso_free(entries);
		return 0;
	}
	
	/* Compose the output: */
	*out = '{';
	ptr = out+1;
	if (fmt) *ptr++='\n';
	*ptr=0;
	for (i=0; i<numentries; i++) {
		if (fmt) for (j=0;j<depth;j++) *ptr++='\t';
		eso_strcpy(ptr,names[i]);ptr+=eso_strlen(names[i]);
		*ptr++=':';if (fmt) *ptr++='\t';
		eso_strcpy(ptr,entries[i]);ptr+=eso_strlen(entries[i]);
		if (i!=numentries-1) *ptr++=',';
		if (fmt) *ptr++='\n';*ptr=0;
		eso_free(names[i]);eso_free(entries[i]);
	}
	
	eso_free(names);
	eso_free(entries);
	if (fmt) for (i=0;i<depth-1;i++) *ptr++='\t';
	*ptr++ = '}';
	*ptr++ = 0;
	return out;	
}

/* Parser core - when encountering text, process appropriately. */
static const char *parse_value(es_json_t *item, const char *value, const char **return_parse_end)
{
	if (!value)                     return 0;	/* Fail on null. */
	if (!eso_strncmp(value,"null",4))   { item->type=ES_JSON_NULL;  return value+4; }
	if (!eso_strncmp(value,"false",5))  { item->type=ES_JSON_FALSE; return value+5; }
	if (!eso_strncmp(value,"true",4))   { item->type=ES_JSON_TRUE; item->valueint=1; return value+4; }
	if (*value=='\"')               { return parse_string(item, value, return_parse_end); }
	if (*value=='-' || (*value>='0' && *value<='9')) { return parse_number(item,value); }
	if (*value=='[')                { return parse_array(item, value, return_parse_end); }
	if (*value=='{')                { return parse_object(item, value, return_parse_end); }

	if (return_parse_end) *return_parse_end = value;
	return 0;	/* failure. */
}

/* Render a value to text. */
static char *print_value(es_json_t *item, int depth, int fmt)
{
	char *out=0;
	if (!item) return 0;
	switch ((item->type)&255) {
		case ES_JSON_NULL:   out=eso_strdup("null"); break;
		case ES_JSON_FALSE:  out=eso_strdup("false"); break;
		case ES_JSON_TRUE:   out=eso_strdup("true"); break;
		case ES_JSON_NUMBER: out=print_number(item); break;
		case ES_JSON_STRING: out=print_string(item); break;
		case ES_JSON_ARRAY:  out=print_array(item,depth,fmt); break;
		case ES_JSON_OBJECT: out=print_object(item,depth,fmt); break;
	}
	return out;
}

/* Delete a es_json_t structure. */
static void json_free(es_json_t *c)
{
	es_json_t *next;
	while (c)
	{
		next = c->next;
		if (!(c->type & JSON_IS_REFERENCE) && c->child) {
			json_free(c->child);
		}
		if (!(c->type & JSON_IS_REFERENCE) && c->valuestring) {
			eso_free(c->valuestring);
		}
		if (c->string) {
			eso_free(c->string);
		}
		eso_free(c);
		c=next;
	}
}

//==============================================================================

/* Parse an object - create a new root, and populate. */
es_json_t* eso_json_make(const char *value)
{
	return eso_json_make2(value, NULL, FALSE);
}

es_json_t* eso_json_make2(const char *value,
                          const char **return_parse_end,
                          es_bool_t require_null_terminated)
{
	const char *end = NULL;
	es_json_t *c= json_new_item();
	
	if (return_parse_end) {
		*return_parse_end = NULL;
	}
	if (!c) return NULL;       /* memory fail */

	end = parse_value(c, skip(value), return_parse_end);
	if (!end) {
		json_free(c);
		return NULL;
	}	/* parse failure. */
	
	/* if we require null-terminated JSON without appended garbage, skip and then check for a null terminator */
	if (require_null_terminated) {
		end = skip(end);
		if (*end) {
			json_free(c);
			if (return_parse_end) *return_parse_end=end;
			return NULL;
		}
	}
	if (return_parse_end) {
		*return_parse_end = end;
	}
	
	return c;
}

/* Delete a es_json_t structure. */
void eso_json_free(es_json_t **json)
{
	if (!json || !*json) return;
	json_free(*json);
	*json = NULL;
}

/* Render a es_json_t entity to text for transfer/storage. Free the char* when finished. */
char *eso_json_sprintf(es_json_t *item, es_string_t **print_str, es_bool_t formated)
{
	char *out;
	
	if (!print_str) return NULL;
	
	out = print_value(item, 0, formated);
	if (out) {
		eso_mstrcpy(print_str, out);
		eso_free(out);
	}
	
	return *print_str;
}

/* Returns the number of items in an array (or object). */
int	eso_json_get_array_size(es_json_t *array)
{
	es_json_t *c = array->child;
	int i=0;
	while(c) i++,c=c->next; return i;
}

/* Retrieve item number "item" from array "array". Returns NULL if unsuccessful. */
es_json_t* eso_json_get_array_item(es_json_t *array, int item)
{
	es_json_t *c = array->child;
	while (c && item>0) item--,c=c->next; return c;
}

/* Get item "string" from object. Case insensitive. */
es_json_t* eso_json_get_object_item(es_json_t *object, const char *string)
{
	es_json_t *c = object->child;
	while (c && eso_strcasecmp(c->string,string)) c=c->next; return c;
}

es_json_t* eso_json_create_null(void)
{
	es_json_t* item = json_new_item();
	if (item) item->type = ES_JSON_NULL;
	return item;
}

es_json_t* eso_json_create_true(void)
{
	es_json_t* item = json_new_item();
	if (item) item->type = ES_JSON_TRUE;
	return item;
}

es_json_t* eso_json_create_false(void)
{
	es_json_t* item = json_new_item();
	if (item) item->type = ES_JSON_FALSE;
	return item;
}

es_json_t* eso_json_create_bool(int b)
{
	es_json_t* item = json_new_item();
	if (item) item->type = b ? ES_JSON_TRUE : ES_JSON_FALSE;
	return item;
}

es_json_t* eso_json_create_number(double num)
{
	es_json_t* item = json_new_item();
	if (item) {
		item->type = ES_JSON_NUMBER;
		item->valuedouble = num;
		item->valueint = (int)num;
	}
	return item;
}

es_json_t* eso_json_create_string(const char *string)
{
	es_json_t* item = json_new_item();
	if (item) {
		item->type = ES_JSON_STRING;
		item->valuestring = eso_strdup(string);
	}
	return item;
}

es_json_t* eso_json_create_array(void)
{
	es_json_t* item = json_new_item();
	if (item) item->type = ES_JSON_ARRAY;
	return item;
}

es_json_t* eso_json_create_object(void)
{
	es_json_t* item = json_new_item();
	if (item) item->type = ES_JSON_OBJECT;
	return item;
}

es_json_t* eso_json_create_intArray(int numbers[], int count)
{
	int i;
	es_json_t *n=0,*p=0,*a=eso_json_create_array();
	for (i=0;a && i<count;i++) {
		n = eso_json_create_number(numbers[i]);
		if(!i)a->child=n;else suffix_object(p,n);p=n;
	}
	return a;
}

es_json_t* eso_json_create_floatArray(float numbers[], int count)
{
	int i;
	es_json_t *n=0,*p=0,*a=eso_json_create_array();
	for (i=0;a && i<count;i++) {
		n = eso_json_create_number(numbers[i]);
		if(!i)a->child=n;else suffix_object(p,n);p=n;
	}
	return a;
}

es_json_t* eso_json_create_doubleArray(double numbers[], int count)
{
	int i;
	es_json_t *n=0,*p=0,*a=eso_json_create_array();
	for (i=0;a && i<count;i++) {
		n = eso_json_create_number(numbers[i]);
		if(!i)a->child=n;else suffix_object(p,n);p=n;
	}
	return a;
}

es_json_t* eso_json_create_stringArray(const char *strings[], int count)
{
	int i;
	es_json_t *n=0,*p=0,*a=eso_json_create_array();
	for (i=0;a && i<count;i++) {
		n = eso_json_create_string(strings[i]);
		if(!i)a->child=n;else suffix_object(p,n);p=n;
	}
	return a;
}

void eso_json_add_item_to_array(es_json_t *array, es_json_t *item)
{
	es_json_t *c = array->child;
	if (!item) return;
	if (!c) {
		array->child = item;
	} else {
		while (c && c->next) c = c->next;
		suffix_object(c, item);
	}
}

void eso_json_add_item_to_object(es_json_t *object, const char *string, es_json_t *item)
{
	if (!item) return;
	if (item->string) eso_free(item->string);
	item->string = eso_strdup(string);
	eso_json_add_item_to_array(object, item);
}

void eso_json_add_item_ref_to_array(es_json_t *array, es_json_t *item)
{
	eso_json_add_item_to_array(array, create_reference(item));
}

void eso_json_add_item_ref_to_object(es_json_t *object, const char *string, es_json_t *item)
{
	eso_json_add_item_to_object(object,string, create_reference(item));
}

es_json_t* eso_json_detach_item_from_array(es_json_t *array, int which)
{
	es_json_t *c = array->child;
	while (c && which>0) c=c->next,which--;
	if (!c) return 0;
	if (c->prev) c->prev->next = c->next;
	if (c->next) c->next->prev = c->prev;
	if (c == array->child) array->child = c->next;
	c->prev = c->next = 0;
	return c;
}

void eso_json_delete_item_from_array(es_json_t *array, int which)
{
	json_free(eso_json_detach_item_from_array(array, which));
}

es_json_t* eso_json_detach_item_from_object(es_json_t *object, const char *string)
{
	int i = 0;
	es_json_t *c = object->child;
	while (c && eso_strcasecmp(c->string,string)) i++,c=c->next;
	if (c) return eso_json_detach_item_from_array(object, i);
	return 0;
}

void eso_json_delete_item_from_object(es_json_t *object, const char *string)
{
	json_free(eso_json_detach_item_from_object(object, string));
}

void eso_json_replace_item_in_array(es_json_t *array, int which, es_json_t *newitem)
{
	es_json_t *c = array->child;
	while (c && which>0) c=c->next,which--;
	if (!c) return;
	newitem->next = c->next;
	newitem->prev = c->prev;
	if (newitem->next) newitem->next->prev = newitem;
	if (c == array->child) {
		array->child = newitem;
	}
	else {
		newitem->prev->next = newitem;
	}
	c->next = c->prev = 0;
	json_free(c);
}

void eso_json_replace_item_in_object(es_json_t *object, const char *string, es_json_t *newitem)
{
	int i=0;
	es_json_t *c = object->child;
	while (c && eso_strcasecmp(c->string,string)) i++,c=c->next;
	if (c) {
		newitem->string = eso_strdup(string);
		eso_json_replace_item_in_array(object, i, newitem);
	}
}

es_json_t* eso_json_dup(es_json_t *item, es_bool_t recurse)
{
	es_json_t *newitem,*cptr,*nptr=0,*newchild;
	
	/* Bail on bad ptr */
	if (!item) return 0;
	/* Create new item */
	newitem = json_new_item();
	if (!newitem) return 0;
	/* Copy over all vars */
	newitem->type = item->type & (~JSON_IS_REFERENCE);
	newitem->valueint = item->valueint;
	newitem->valuedouble = item->valuedouble;
	if (item->valuestring) {
		newitem->valuestring = eso_strdup(item->valuestring);
		if (!newitem->valuestring) {
			json_free(newitem);
			return 0;
		}
	}
	if (item->string) {
		newitem->string = eso_strdup(item->string);
		if (!newitem->string) {
			json_free(newitem);
			return 0;
		}
	}
	
	/* If non-recursive, then we're done! */
	if (!recurse) return newitem;
	
	/* Walk the ->next chain for the child. */
	cptr = item->child;
	while (cptr) {
		newchild = eso_json_dup(cptr, TRUE);		/* Duplicate (with recurse) each item in the ->next chain */
		if (!newchild) {
			json_free(newitem);
			return 0;
		}
		if (nptr) {
			nptr->next = newchild;
			newchild->prev = nptr;
			nptr = newchild;
		}	/* If newitem->child already set, then crosswire ->prev and ->next and move on */
		else {
			newitem->child = newchild;
			nptr = newchild;
		}					/* Set newitem->child and move to it */
		cptr = cptr->next;
	}
	return newitem;
}
