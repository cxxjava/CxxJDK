/**
 * @file  eso_fmttime.h
 * @brief ES time format functions
 */

#ifndef __ESO_FMT_TIME_H__
#define __ESO_FMT_TIME_H__

#include "es_types.h"
#include "eso_datetime.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @description: Time format.
 * @parameter  : to		output formated string
 *               fmt	format string, see @format
 *               tm 	es_tm_t time
 * @return     : (*to)
 * @format: 
  %a     locale’s abbreviated weekday name (e.g., Sun)
  %b     locale’s abbreviated month name (e.g., Jan)
  %d     day of month (01..31)
  %H     hour (00..23)
  %I     hour (01..12)
  %m     month (01..12)
  %M     minute (00..59)
  %S     second (00..60)
  %s     millisecond (000..999)
  %p     locale’s equivalent of either AM or PM; blank if not known
  %P     like %p, but lower case
  %u     day of week (1..7); 1 is Monday
  %w     day of week (0..6); 0 is Sunday
  %Y     year
  %y     last two digits of year (00-99)
  %z     +hhmm numeric timezone (e.g., -0400)
 * example:
  %Y-%m-%d %H:%M:%S %p
  %m/%d/%y 
  %a %b %d %H:%M:%S %z %Y
*/
char *eso_fmt_time(es_string_t **to, const char *fmt, es_tm_t *tm);

/**
 * description: Time format
 * parameter  : to		output formated string
 *              fmt		format string, see @format
 *              ms		Since 1970-1-1 Greenwich time the total number of microseconds
 *              tz 		-12~12
 * return     : (*to)
*/
char *eso_fmt_timezone(es_string_t **to, const char *fmt, es_uint64_t ms, es_int32_t tz);

/**
 * description: Parse a time string.
 * parameter  : tz		output parsed time zone
 *              fmt		format string, see @format
 *              src		source time string
 * return     : >0 since 1970-1-1 Greenwich time the total number of seconds
                =0 failure
 */
es_uint32_t eso_parse_time(es_int32_t *tz, const char *fmt, const char *src);

#ifdef __cplusplus
}
#endif

#endif  /*__ESO_FMT_TIME_H__*/
