/**
 * @file  eso_object.h
 * @brief ES Object base manager.
 */

#ifndef __ESO_OBJECT_H__
#define __ESO_OBJECT_H__

#include "eso_libc.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct es_object_t es_object_t;

typedef void es_object_destroy_t (void* cthis);

struct es_object_t
{
	es_object_destroy_t* destroy;
	es_object_t*         parent;
};

extern es_object_t* NULL_create(const es_int8_t* file, int line);

#define ES_CLASS_DECLARE(_class_) \
	typedef struct _class_ _class_;

#define ES_CLASS_DEFINE_BEGIN(_class_) \
_class_* _class_##_create(void); \
void _class_##_destroy(_class_* cthis); \
struct _class_ { \
	es_object_t _this;
#define ES_CLASS_DEFINE_END };

#define ES_OBJ_SUPER(pobj) (pobj)->_this.parent

#define ES_OBJ_CTOR_BEGIN(_class_, _parent_) \
	_class_* _class_##_create(void) { \
		_class_ *cthis = (_class_*)eso_pcalloc(sizeof(_class_)); \
		if (!cthis) return NULL; \
		cthis->_this.destroy = _class_##_destroy; \
		cthis->_this.parent = (es_object_t*)_parent_##_create(); \
		{
#define ES_OBJ_NEW(_class_) _class_##_create()

#define ES_OBJ_CTOR_END \
		} \
		return cthis; \
	}

#define ES_OBJ_DTOR_BEGIN(_class_) \
	void _class_##_destroy(_class_* cthis) {

#define ES_OBJ_DTOR_END \
		if (cthis->_this.parent && cthis->_this.parent->destroy) { \
			cthis->_this.parent->destroy(cthis->_this.parent); \
		} \
		eso_pfree(cthis); \
	}

#define ES_OBJ_DEL(pobj) do { \
	es_object_t* cthis = &(pobj)->_this; \
	if (cthis->destroy) { \
		cthis->destroy(pobj); \
	} \
} while(0);
#define ES_OBJ_DEL2(ppobj) if (ppobj && *ppobj) { \
	ES_OBJ_DEL(*ppobj);\
	*ppobj = NULL; \
}

#ifdef __cplusplus
}
#endif

#endif	/* ! __ESO_OBJECT_H__ */
