/**
 * @file  eso_datetime.h
 * @brief ES This file defines structure, contant, and function prototypes for date/time interfaces.
 */

#ifndef __ESO_DATETIME_H__
#define __ESO_DATETIME_H__

#include "es_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define ES_UTC_BASE_YEAR 1970
#define ES_MONTH_PER_YEAR 12
#define ES_DAY_PER_YEAR 365
#define ES_SEC_PER_DAY 86400
#define ES_SEC_PER_HOUR 3600
#define ES_SEC_PER_MIN 60
#define ES_MIN_PER_HR 60
#define ES_HRS_PRE_DAY 24

#define ES_MAX_YEAR 2030
#define ES_MIN_YEAR 2000
    
#define ES_YEARFORMATE  2000
#define ES_DAYS_OF_WEEK    7

#define ES_MAX_WEEK_NUM 54

/*
 *	Time
 */
typedef struct
{
	es_uint8_t  tm_hour;        /* hours, range 0 to 23             */
	es_uint8_t  tm_min;         /* minutes, range 0 to 59           */
	es_uint8_t  tm_sec;         /* seconds,  range 0 to 59          */

	es_uint8_t  tm_year;        /* The number of years since 1900   */
	es_uint8_t  tm_mon;         /* month, range 0 to 11             */
	es_uint8_t  tm_mday;        /* day of the month, range 0 to 30  */
	
	es_uint8_t  tm_wday;        /* day of the week, range 0 to 6    */
	es_int8_t   tm_zone;        /* time zone, rang -11 to 12        */
	
	es_uint32_t tm_time_sec;    /* The number of seconds since 1900 */
	es_uint32_t tm_time_usec;   /* microseconds                     */
} es_tm_t;


/**
 * Get localtime.
 */
es_tm_t* eso_dt_now(es_tm_t *tm);

/**
 * Gets the current value of localtime.
 * Accurate to millisecond: 1 second = 1000 milliseconds
 */
es_int64_t eso_dt_millis(void);

/**
 * Gets the current value of the system clock.
 * Accurate to nanosecond: 1 second = 1000*1000*1000 nanoseconds
 */
es_int64_t eso_dt_nano(void);

/**
 * Determine whether it is a leap year.
 */
es_bool_t eso_dt_is_leap_year(es_uint16_t year);

/**
 * Calculate the number of days in a month.
 */
es_uint8_t eso_dt_last_day_of_mon(es_uint8_t month, es_uint16_t year);

/**
 * Calculation week
 */
es_uint8_t eso_dt_dow(es_uint16_t y, es_uint8_t m, es_uint8_t d);

/**
 * Calculate the date is the first few weeks
 */
es_uint8_t eso_dt_get_week_number(es_tm_t *date);

/**
 * UTC to RTC
 */
void eso_dt_utc_to_rtc(es_tm_t *utc, es_tm_t *result);

/**
 * Calculating next time with time, hour and minute to result. 
 */
void eso_dt_cal_time(es_tm_t *time, int hour, int minute, es_tm_t *result);

/**
 * Check time is valid
 */
es_bool_t eso_dt_is_valid(es_tm_t *t);

/**
 * Convert es_tm_t format of time to seconds after 1970/1/1
 * @param gmt_sec	Since 1970-1-1 Greenwich time the total number of seconds
 * @param tm		The output data.
 */
void eso_gm_time(es_uint32_t gmt_sec, es_tm_t *tm);

/**
 * Convert tm to gm seconds.
 * @param tm	es_tm_t data
 * @return Since 1970-1-1 Greenwich time the total number of seconds
 */
es_uint32_t eso_time_gm(es_tm_t *tm);

/**
 * Convert y/m/d/h/m/s data to seconds after 1970/1/1
 * @param  y/m/d/h/m/s
 * @return Since 1970-1-1 Greenwich time the total number of seconds
 */
es_uint32_t eso_time_sec(es_uint16_t year, es_uint8_t mon, es_uint8_t day, 
                         es_uint8_t hour, es_uint8_t min, es_uint8_t sec);

#ifdef __cplusplus
}
#endif

#endif  /*__ESO_DATETIME_H__*/
