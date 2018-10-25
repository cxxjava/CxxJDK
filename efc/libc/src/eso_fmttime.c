/**
 * @file  eso_fmttime.c
 * @brief ES time format functions
 */
#include "eso_fmttime.h"
#include "eso_datetime.h"
#include "eso_libc.h"
#include "eso_string.h"
#include "eso_util.h"

#define MEMCPY(dst, src, n) ((char *) eso_memcpy(dst, src, n)) + (n)

static char week[7][4]   = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static char months[12][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

static int get_week_num(const char *w)
{
	int i;
	for (i = 0; i < ES_ARRAY_LEN(week); i++) {
		if (eso_strcasecmp(w, week[i]) == 0)
			return i;
	}
	return -1;
}

static int get_month_num(const char *m)
{
	int i;
	for (i = 0; i < ES_ARRAY_LEN(months); i++) {
		if (eso_strcasecmp(m, months[i]) == 0)
			return i;
	}
	return -1;
}

/*******************************************************************************
declaration:int eso_fmt_time(char *DateTime,char *format);
Description:根据不同时间格式类型获取日期和时间
parameter:
 序号 参数名称        变量    类型     参数说明
  1 时间格式描述字符串 format  char*
    %a     locale’s abbreviated weekday name (e.g., Sun)
    %b     locale’s abbreviated month name (e.g., Jan)
    %d     day of month (e.g, 01)
    %H     hour (00..23)
    %I     hour (01..12)
    %m     month (01..12)
    %M     minute (00..59)
    %p     locale’s equivalent of either AM or PM; blank if not known
    %P     like %p, but lower case
    %S     second (00..60)
    %s     millisecond (000..999)
    %u     day of week (1..7); 1 is Monday
    %w     day of week (0..6); 0 is Sunday
    %Y     year
    %y     last two digits of year (00-99)
    %z     +hhmm numeric timezone (e.g., -0400)
  2 es_tm_t *tm =时间数据
return:!=NULL--成功,==NULL--失败
*******************************************************************************/
char *eso_fmt_time(es_string_t **to, const char *fmt, es_tm_t *tm)
{
	char buf[32];
	char *fcurr, *fend;
	int len;

	ES_ASSERT(fmt);
	ES_ASSERT(tm);

	eso_mmeminit((es_data_t**)to);

	fcurr = (char*)fmt;
	fend = fcurr + eso_strlen((const char *)fmt);

	while (fcurr < fend) {
		if (*fcurr == '%') {
			if (++fcurr < fend) {
				switch (*fcurr) {
				case 'a':  //%a
					eso_mstrcat(to, week[tm->tm_wday]);
					break;
				case 'b':  //%b
					eso_mstrcat(to, months[tm->tm_mon]);
					break;
				case 'd':  //%d
					eso_sprintf(buf, "%02d", tm->tm_mday+1);
					eso_mstrcat(to, buf);
					break;
				case 'H':  //%H
					eso_sprintf(buf, "%02d", tm->tm_hour);
					eso_mstrcat(to, buf);
					break;
				case 'I':  //%I
					len = tm->tm_hour;
					if (len >= 12) len -=12;
					eso_sprintf(buf, "%02d", len);
					eso_mstrcat(to, buf);
					break;
				case 'm':  //%m
					eso_sprintf(buf, "%02d", tm->tm_mon+1);
					eso_mstrcat(to, buf);
					break;
				case 'M':  //%M
					eso_sprintf(buf, "%02d", tm->tm_min);
					eso_mstrcat(to, buf);
					break;
				case 'p':  //%p
					eso_sprintf(buf, "%s", tm->tm_hour < 12 ? "AM" : "PM");
					eso_mstrcat(to, buf);
					break;
				case 'P':  //%P
					eso_sprintf(buf, "%s", tm->tm_hour < 12 ? "am" : "pm");
					eso_mstrcat(to, buf);
					break;
				case 'S':  //%S
					eso_sprintf(buf, "%02d", tm->tm_sec);
					eso_mstrcat(to, buf);
					break;
				case 's':  //%s
					eso_sprintf(buf, "%03d", tm->tm_time_usec/1000);
					eso_mstrcat(to, buf);
					break;
				case 'u':  //%u
					eso_sprintf(buf, "%02d", tm->tm_wday ? tm->tm_wday : 7);
					eso_mstrcat(to, buf);
					break;
				case 'w':  //%w
					eso_sprintf(buf, "%02d", tm->tm_wday);
					eso_mstrcat(to, buf);
					break;
				case 'Y':  //%Y
					eso_sprintf(buf, "%04d", tm->tm_year+1900);
					eso_mstrcat(to, buf);
					break;
				case 'y':  //%y
					eso_sprintf(buf, "%02d", (tm->tm_year+1900) % 100);
					eso_mstrcat(to, buf);
					break;
				case 'z': //%z
					eso_sprintf(buf, "%c%02d00", tm->tm_zone < 0? '-' : '+', abs(tm->tm_zone));
					eso_mstrcat(to, buf);
					break;
				default:
					eso_mstrcat(to, "%");
					break;
				}
			}
			fcurr++;
		} else {
			buf[0] = *fcurr;
			buf[1] = 0;
			eso_mstrcat(to, buf);
			fcurr++;
		}
	}

	return *to;
}

/**
 * description: 格式化输出传入的时区时间
 * parameter  : to		输出串
 *              fmt		格式化串，具体格式定义详见format
 *              gmt_sec	从1970-1-1以来的格林威治时间总秒数
 *              tz 		-12~12
 * return     : (*to)
*/
char *eso_fmt_timezone(es_string_t **to, const char *fmt, es_uint64_t ms, es_int32_t tz)
{
	es_tm_t tm = {0,0,0,0,0,0,0,0,0};

	es_uint32_t gmt_sec = ms/1000 + tz*60*60;
	eso_gm_time(gmt_sec, &tm);
	tm.tm_zone = tz;
	tm.tm_time_usec = ms % 1000 * 1000;
	return eso_fmt_time(to, fmt, &tm);
}

/**
 * description: 格式化解析时间串
 * parameter  : tm		输出时区
 *              tz		格式化串，具体格式定义详见format
 *              src		源时间串
 * return     : >0 从1970-1-1以来的格林威治时间总秒数，=0 失败
 */
es_uint32_t eso_parse_time(es_int32_t *tz, const char *fmt, const char *src)
{
	es_tm_t stm, *tm;
	char buf[32];
	char *fcurr, *fend;
	char *scurr, *send;
	int i;

	ES_ASSERT(fmt);
	ES_ASSERT(src);

	tm = &stm;
	eso_memset(tm, 0, sizeof(es_tm_t));
	if (tz) *tz = 0;

	fcurr = (char*) fmt;
	fend = fcurr + eso_strlen((const char *) fmt);
	scurr = (char*)src;
	send = scurr + eso_strlen((const char *) src);

	while (fcurr < fend && scurr < send) {
		eso_memset(buf, 0, sizeof(buf));
		if (*fcurr == '%') {
			if (++fcurr < fend) {
				switch (*fcurr) {
//				case 'a': //%a
//					eso_strncpy(buf, scurr, 3);
//					scurr += 3;
//					if ((i = get_week_num(buf)) < 0) {
//						return 0;
//					}
//					tm->tm_wday = i;
//					break;
				case 'b': //%b
					eso_strncpy(buf, scurr, 3);
					scurr += 3;
					if ((i = get_month_num(buf)) < 0) {
						return 0;
					}
					tm->tm_mon = i;
					break;
				case 'd': //%d
					eso_strncpy(buf, scurr, 2);
					scurr += 2;
					if (!eso_isdigit_string(buf)) {
						return 0;
					}
					i = eso_atol(buf);
					if (i < 1 || i > 31) {
						return 0;
					}
					tm->tm_mday = i - 1;
					break;
				case 'H': //%H
					eso_strncpy(buf, scurr, 2);
					scurr += 2;
					if (!eso_isdigit_string(buf)) {
						return 0;
					}
					i = eso_atol(buf);
					if (i < 0 || i > 23) {
						return 0;
					}
					tm->tm_hour = i;
					break;
				case 'I': //%I
					eso_strncpy(buf, scurr, 2);
					scurr += 2;
					if (!eso_isdigit_string(buf)) {
						return 0;
					}
					i = eso_atol(buf);
					if (i < 0 || i > 11) {
						return 0;
					}
					tm->tm_hour = i;
					break;
				case 'm': //%m
					eso_strncpy(buf, scurr, 2);
					scurr += 2;
					if (!eso_isdigit_string(buf)) {
						return 0;
					}
					i = eso_atol(buf);
					if (i < 1 || i > 12) {
						return 0;
					}
					tm->tm_mon = i -1;
					break;
				case 'M': //%M
					eso_strncpy(buf, scurr, 2);
					scurr += 2;
					if (!eso_isdigit_string(buf)) {
						return 0;
					}
					i = eso_atol(buf);
					if (i < 0 || i > 59) {
						return 0;
					}
					tm->tm_min = i;
					break;
				case 'p': //%p
				case 'P': //%P
					eso_strncpy(buf, scurr, 2);
					scurr += 2;
					if (eso_strcasecmp(buf, "AM") && eso_strcasecmp(buf, "PM")) {
						return 0;
					}
					if (tm->tm_hour < 12) {
						tm->tm_hour += 12;
					}
					break;
				case 'S': //%S
					eso_strncpy(buf, scurr, 2);
					scurr += 2;
					if (!eso_isdigit_string(buf)) {
						return 0;
					}
					i = eso_atol(buf);
					if (i < 0 || i > 59) {
						return 0;
					}
					tm->tm_sec = i;
					break;
//				case 'u': //%u
//					eso_strncpy(buf, scurr, 2);
//					scurr += 2;
//					if (!eso_isdigit_string(buf)) {
//						return 0;
//					}
//					i = eso_atol(buf);
//					if (i < 1 || i > 7) {
//						return 0;
//					}
//					tm->tm_wday = i -1;
//					break;
//				case 'w': //%w
//					eso_strncpy(buf, scurr, 2);
//					scurr += 2;
//					if (!eso_isdigit_string(buf)) {
//						return 0;
//					}
//					i = eso_atol(buf);
//					if (i < 0 || i > 6) {
//						return 0;
//					}
//					tm->tm_wday = i;
//					break;
				case 'Y': //%Y
					eso_strncpy(buf, scurr, 4);
					scurr += 4;
					if (!eso_isdigit_string(buf)) {
						return 0;
					}
					i = eso_atol(buf);
					if (i < 1970) {
						return 0;
					}
					tm->tm_year = i - 1900;
					break;
				case 'y': //%y
					eso_strncpy(buf, scurr, 2);
					scurr += 2;
					if (!eso_isdigit_string(buf)) {
						return 0;
					}
					i = eso_atol(buf);
					{
						es_tm_t t;
						eso_dt_now(&t);
						tm->tm_year = (i + ES_ALIGN_DOWN(t.tm_year+1900, 100)) - 1900;
					}
					break;
				case 'z': //%z
					if (*scurr == '+' || *scurr == '-') {
						eso_strncpy(buf, scurr, 3);
						scurr += 3;
						i = 1;
					}
					else {
						eso_strncpy(buf, scurr, 2);
						scurr += 2;
						i = 0;
					}
					if (!eso_isdigit_string(i ? buf+1 : buf)) {
						return 0;
					}
					i = eso_atol(buf);
					tm->tm_zone = i;
					break;
				default:
					break;
				}
			}
			fcurr++;
		} else {
			fcurr++;
			scurr++;
		}
	}

	if (tz) {
		*tz = tm->tm_zone;
	}

	return eso_time_gm(tm);
}
