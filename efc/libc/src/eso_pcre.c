/**
 * @file  eso_pcre.c
 * @brief ES pcre wrapper.
 */

#include "eso_pcre.h"
#include "eso_libc.h"
#include "eso_util.h"

#include "./pcre/pcre.h"

/**
 * @see: pcredemo.c
 */

struct es_pcre_t {
	pcre *pcre;
};

/**
 * 构建并编译正则表达式
 * Params: pattern	正则表达式
 *         options  参数选项，一般使用0
 *         erroffset 保存错误位置
 * Return: 编译好的模式结构
 */
es_pcre_t* eso_pcre_make(const char *pattern, 
                         int options,
                         int *erroffset)
{
	es_pcre_t *pcre = NULL;
	const char *errptr = NULL;
	
	if (erroffset) {
		*erroffset = 0;
	}
	
	if (!pattern || !*pattern) {
		return NULL;
	}
	
	pcre = (es_pcre_t*)eso_malloc(sizeof(es_pcre_t));
	if (!pcre) {
		return NULL;
	}
	
	pcre->pcre = pcre_compile(pattern, options, &errptr, erroffset, NULL);
	if (!pcre->pcre) {
		if (errptr) {
			//...
		}
		eso_free(pcre);
		return NULL;
	}
	
	return pcre;
}

/**
 * 使用编译好的模式进行匹配
 * Params: pcre	编译好的模式结构
 *         subject 需要匹配的字符串
 *         length  匹配的字符串长度，-1表示整个subject字符串长度
 *         startoffset	匹配的开始位置
 *         options  参数选项，一般使用0
 *         ovector	结果的整型数组，长度为3的倍数
 *         ovecsize	数组大小
 * Return: > 0 => success; value is the number of elements filled in
           = 0 => success, but offsets is not big enough
           -1 => failed to match
           < -1 => some kind of unexpected problem
 */
int eso_pcre_exec(es_pcre_t *pcre, 
                  const char *subject, 
                  int length, 
                  int startoffset, 
                  int options, 
                  int ovector[], 
                  int ovecsize)
{
	int ret;

	if (!subject) {
		return -2;
	}

	ret = pcre_exec(pcre->pcre,
	                    NULL,
	                    subject,
	                    (length == -1) ? (int)eso_strlen(subject) : length,
	                    startoffset,
	                    options,
	                    ovector,
	                    ovecsize);
	return ret;
}

/**
 * 资源释放
 */
void eso_pcre_free(es_pcre_t **pcre)
{
	if (!pcre || !*pcre) {
		return;
	}
	pcre_free((*pcre)->pcre);
	eso_free(*pcre);
	*pcre = NULL;
}

//=============================================================================

#if 0

void test_pcre()
{
	es_pcre_t *pcre;
	int v;
	int ovector[15];
	
	//pcre = eso_pcre_make("(\\w+)  ***&&&:(\\d+)", 0, &v);
	pcre = eso_pcre_make("(\\w+):(\\d+)", 0, &v);
	if (!pcre) {
		printf("pcre make off=%d\n", v);
		return;
	}
	v = eso_pcre_exec(pcre, "123 hello:1234", 14, 0, 0, ovector, 15);
	printf("pcre exec v=%d\n", v);
	if (v >= 0) {
		printf("matched start pos=%d\n", ovector[0]);
	}
	eso_pcre_free(&pcre);
}

#endif
