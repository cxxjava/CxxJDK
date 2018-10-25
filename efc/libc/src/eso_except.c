/**
 * @file  eso_debug.c
 * @brief ES Exception Handling in C Language
 */

#include "eso_except.h"

/*==============================================================================
注意!!!
此异常处理模块仅限于C应用层代码使用，其他地方禁止使用；
C++中使用会导致对象销毁异常。
==============================================================================*/

static es_except_element_t* g_element_link_head = NULL;

/* 把一个异常插入到链表头中 */
void ___except_push(es_except_element_t* es_except_element_t)
{
	if (g_element_link_head) es_except_element_t->next = g_element_link_head;
	g_element_link_head = es_except_element_t;
}

/* 从链表头中，删除并返回一个异常 */
es_except_element_t* ___except_pop(void)
{
	es_except_element_t* ret = NULL;

	ret = g_element_link_head;
	g_element_link_head = g_element_link_head->next;
	
	return ret;
}

/* 从链表头中，返回一个异常 */
es_except_element_t* ___except_top(void)
{
	return g_element_link_head;
}

/* 链表中是否有任何异常 */
int ___except_is_empty(void)
{
	return (!g_element_link_head);
}

/* 缺省的异常处理模块 */
static void ___uncaught_exception_handler(es_except_t *exception) 
{
	eso_log("error! get a uncaught exception：type=%d, code=%d, msg=%s",
		   exception->err_type, exception->err_code, exception->err_msg);
//	Close();
}

/* 抛出异常 */
void ___except_thrower(es_except_t* e) 
{
	es_except_element_t *se;
	
	if (___except_is_empty()) ___uncaught_exception_handler(e);
	
	se = ___except_top();
	se->exception.err_type = e->err_type;
	se->exception.err_code = e->err_code;
	eso_strncpy(se->exception.err_msg, e->err_msg, sizeof(se->exception.err_msg));
	
	longjmp(se->status, 1);
}
