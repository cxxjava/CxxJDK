/**
 * @file  eso_json.h
 * @brief ES JSON parser.
 */

#ifndef __ESO_JSON_H__
#define __ESO_JSON_H__

#include "es_comm.h"
#include "es_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* JSON Types: */
#define ES_JSON_FALSE 0
#define ES_JSON_TRUE 1
#define ES_JSON_NULL 2
#define ES_JSON_NUMBER 3
#define ES_JSON_STRING 4
#define ES_JSON_ARRAY 5
#define ES_JSON_OBJECT 6


typedef struct es_json_t es_json_t;
struct es_json_t {
	es_json_t *next, *prev;	/* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
	es_json_t *child;		/* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */

	int type;					/* The type of the item, as above. */

	char *valuestring;			/* The item's string, if type==ES_JSON_STRING */
	int valueint;				/* The item's number, if type==ES_JSON_NUMBER */
	double valuedouble;			/* The item's number, if type==ES_JSON_NUMBER */

	char *string;				/* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
};

/* Parse an object - create a new root, and populate. */
es_json_t* eso_json_make(const char *value);
es_json_t* eso_json_make2(const char *value,
                          const char **return_parse_end,
                          es_bool_t require_null_terminated);

/* Duplicate a es_json_t item */
es_json_t* eso_json_dup(es_json_t *item, es_bool_t recurse);
/* Duplicate will create a new, identical es_json_t item to the one you pass, in new memory that will
need to be released. With recurse==TRUE, it will duplicate any children connected to the item.
The item->next and ->prev pointers are always zero on return from Duplicate. */

/* Render a es_json_t entity to text for transfer/storage. */
char *eso_json_sprintf(es_json_t *item, es_string_t **print_str, es_bool_t formated);

/* Returns the number of items in an array (or object). */
int	eso_json_get_array_size(es_json_t *array);
/* Retrieve item number "item" from array "array". Returns NULL if unsuccessful. */
es_json_t* eso_json_get_array_item(es_json_t *array, int item);
/* Get item "string" from object. Case insensitive. */
es_json_t* eso_json_get_object_item(es_json_t *object, const char *string);

/* These calls create a es_json_t item of the appropriate type. */
es_json_t* eso_json_create_null(void);
es_json_t* eso_json_create_true(void);
es_json_t* eso_json_create_false(void);
es_json_t* eso_json_create_bool(int b);
es_json_t* eso_json_create_number(double num);
es_json_t* eso_json_create_string(const char *string);
es_json_t* eso_json_create_array(void);
es_json_t* eso_json_create_object(void);

/* These utilities create an Array of count items. */
es_json_t* eso_json_create_intArray(int numbers[], int count);
es_json_t* eso_json_create_floatArray(float numbers[], int count);
es_json_t* eso_json_create_doubleArray(double numbers[], int count);
es_json_t* eso_json_create_stringArray(const char *strings[], int count);

/* Append item to the specified array/object. */
void eso_json_add_item_to_array(es_json_t *array, es_json_t *item);
void eso_json_add_item_to_object(es_json_t *object, const char *string, es_json_t *item);
/* Append reference to item to the specified array/object. 
 * Use this when you want to add an existing es_json_t to a new es_json_t, 
 * but don't want to corrupt your existing es_json_t.
 */
void eso_json_add_item_ref_to_array(es_json_t *array, es_json_t *item);
void eso_json_add_item_ref_to_object(es_json_t *object, const char *string, es_json_t *item);

/* Remove/Detatch items from Arrays/Objects. */
es_json_t* eso_json_detach_item_from_array(es_json_t *array, int which);
void eso_json_delete_item_from_array(es_json_t *array, int which);
es_json_t* eso_json_detach_item_from_object(es_json_t *object, const char *string);
void eso_json_delete_item_from_object(es_json_t *object, const char *string);

/* Update array items. */
void eso_json_replace_item_in_array(es_json_t *array, int which, es_json_t *newitem);
void eso_json_replace_item_in_object(es_json_t *object, const char *string, es_json_t *newitem);

/* Delete a es_json_t structure. */
void eso_json_free(es_json_t **json);

#ifdef __cplusplus
}
#endif

#endif /* __ESO_JSON_H__ */
