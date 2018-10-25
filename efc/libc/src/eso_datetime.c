#include "eso_datetime.h"
#include "eso_libc.h"

#ifdef WIN32
#include <time.h>
#include <windows.h>
#elif defined(__MTK__)
#include <time.h>
#else //
#include <sys/time.h> /* gethrtime(), gettimeofday() */
#include <time.h>	  /* clock_gettime(), time() */
#include <unistd.h>   /* POSIX flags */
#if defined(__APPLE__)
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif //!__APPLE__
#endif //!WIN32

//==============================================================================

const es_uint8_t g_dt_day_per_mon[ES_MONTH_PER_YEAR] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

//==============================================================================

#ifdef __linux__
__attribute__((weak))
int clock_gettime (clockid_t __clock_id, struct timespec *__tp)
{
	struct timeval tm;

	if (__clock_id == -1 || !__tp) {
		return -1;
	}

	gettimeofday( &tm, NULL );

	__tp->tv_sec = tm.tv_sec;
	__tp->tv_nsec = (es_int64_t)tm.tv_usec * 1000;

	return 0;
}
#endif

/**
 * 获取当前本地系统时间
 */
es_tm_t* eso_dt_now(es_tm_t *tm)
{
	if (tm) {
#if defined(WIN32) || defined(__MTK__)
		long diff_secs;
		time_t t = time(0);
		struct tm *p = localtime(&t);
		t = mktime(p);
		tm->tm_time_usec = 0; // - ?
		eso_gm_time((es_uint32_t)t, tm);
		_get_timezone(&diff_secs);
		tm->tm_zone = -diff_secs/60/60; // - ?
#else
		struct timeval tv;
		struct timezone tz;
		gettimeofday(&tv, &tz);
		tm->tm_time_usec = tv.tv_usec;
		eso_gm_time((es_uint32_t)tv.tv_sec, tm);
		tm->tm_zone = -tz.tz_minuteswest/60; // - ?
#endif
	}
	return tm;
}

/**
 * 获取当前本地系统时间
 * 精确到毫秒
 */
es_int64_t eso_dt_millis(void) {
	es_tm_t tm;
	eso_dt_now(&tm);
	return (es_int64_t) (tm.tm_time_sec * (es_int64_t)1000 + tm.tm_time_usec / (es_int64_t)1000);
}

/**
 * 获取系统运行的时间
 * 精确到纳秒
 */
es_int64_t eso_dt_nano(void)
{
	//@see: http://nadeausoftware.com/articles/2012/04/c_c_tip_how_measure_elapsed_real_time_benchmarking

#if defined(_WIN32)

	FILETIME tm;
	ULONGLONG t;
#if defined(NTDDI_WIN8) && NTDDI_VERSION >= NTDDI_WIN8
	/* Windows 8, Windows Server 2012 and later. ---------------- */
	GetSystemTimePreciseAsFileTime( &tm );
#else
	/* Windows 2000 and later. ---------------------------------- */
	GetSystemTimeAsFileTime( &tm );
#endif
	t = ((ULONGLONG)tm.dwHighDateTime << 32) | (ULONGLONG)tm.dwLowDateTime;
	return (es_int64_t)(t * ES_NANOS_PER_MILLISEC);  // the best we can do.

#elif defined(__MTK__)

	return clock() * ES_NANOS_PER_MILLISEC;

#elif (defined(__hpux) || defined(hpux)) || ((defined(__sun__) || defined(__sun) || defined(sun)) && (defined(__SVR4) || defined(__svr4__)))

	/* HP-UX, Solaris. ------------------------------------------ */
	return (es_int64_t)gethrtime( );

#elif defined(__MACH__) && defined(__APPLE__)

	/* OSX. ----------------------------------------------------- */
	//@see: http://shiftedbits.org/2008/10/01/mach_absolute_time-on-the-iphone/
	static mach_timebase_info_data_t timeBase = {0,0};
	if (timeBase.denom == 0 && timeBase.numer == 0) {
		(void) mach_timebase_info(&timeBase);
	}
	return (es_int64_t)(mach_absolute_time() * timeBase.numer / timeBase.denom);

#elif defined(_POSIX_VERSION)

	/* POSIX. --------------------------------------------------- */
#if defined(_POSIX_TIMERS) && (_POSIX_TIMERS > 0)
	{
		struct timespec ts;
#if defined(CLOCK_MONOTONIC_PRECISE)
		/* BSD. --------------------------------------------- */
		const clockid_t id = CLOCK_MONOTONIC_PRECISE;
//#elif defined(CLOCK_MONOTONIC_RAW)
//		/* Linux. ------------------------------------------- */
//		const clockid_t id = CLOCK_MONOTONIC_RAW;
#elif defined(CLOCK_HIGHRES)
		/* Solaris. ----------------------------------------- */
		const clockid_t id = CLOCK_HIGHRES;
#elif defined(CLOCK_MONOTONIC)
		/* AIX, BSD, Linux, POSIX, Solaris. ----------------- */
		const clockid_t id = CLOCK_MONOTONIC;
#elif defined(CLOCK_REALTIME)
		/* AIX, BSD, HP-UX, Linux, POSIX. ------------------- */
		const clockid_t id = CLOCK_REALTIME;
#else
		const clockid_t id = (clockid_t)-1;	/* Unknown. */
#endif /* CLOCK_* */
		if ( id != (clockid_t)-1 && clock_gettime( id, &ts ) != -1 )
			return (es_int64_t)ts.tv_sec * ES_NANOS_PER_SECOND +
					(es_int64_t)ts.tv_nsec;
		/* Fall thru. */
	}
#endif //!_POSIX_TIMERS

	/* AIX, BSD, Cygwin, HP-UX, Linux, OSX, POSIX, Solaris. ----- */
	struct timeval tm;
	gettimeofday( &tm, NULL );
	return (es_int64_t)tm.tv_sec * ES_NANOS_PER_SECOND + (es_int64_t)tm.tv_usec * 1000;

#else //other

#error "not support!"

#endif //!_POSIX_VERSION
}

/**
 * 判断是否是闰年
 */
es_bool_t eso_dt_is_leap_year(es_uint16_t year)
{
	if ((year % 400) == 0) {
		return TRUE;
	}
	else if ((year % 100) == 0) {
		return FALSE;
	}
	else if ((year % 4) == 0) {
		return TRUE;
	}
	return FALSE;
}

/**
 * 计算指定月份的天数
 */
es_uint8_t eso_dt_last_day_of_mon(es_uint8_t month, es_uint16_t year)
{
	if (month != 2) {
		return g_dt_day_per_mon[month - 1];
	}
	else {
		return g_dt_day_per_mon[1] + eso_dt_is_leap_year(year);
	}
}

/**
 * 计算星期几
 */
es_uint8_t eso_dt_dow(es_uint16_t y, es_uint8_t m, es_uint8_t d)
{
	if (m < 3) {
		m += 13;
		y--;
	}
	else {
		m++;
	}
	
	return (d + 26 * m / 10 + y + y / 4 - y / 100 + y / 400 + 6) % 7;
}

/**
 * 计算是第几个星期
 */
es_uint8_t eso_dt_get_week_number(es_tm_t *date)
{
	es_int32_t i, no_of_days, week_num, numdays_inmonth, day_on_firstjan;
	
	no_of_days = date->tm_mday;
	
	for (i = date->tm_mon; i > 0; i--) {

		/* get the num of days in todays month */
		numdays_inmonth = eso_dt_last_day_of_mon((es_uint8_t) i, date->tm_year);
		no_of_days += numdays_inmonth;
	}

	day_on_firstjan = eso_dt_dow(date->tm_year, 1, 1);
	
	/* Total no of days/ weekdays */
	no_of_days += day_on_firstjan;
	
	week_num = (no_of_days) / ES_DAYS_OF_WEEK;
	if ((no_of_days) % ES_DAYS_OF_WEEK > 0) {
		week_num += 1;
	}
	
	return week_num;
}

/**
 * UTC时间到RTC时间转换
 */
void eso_dt_utc_to_rtc(es_tm_t *utc, es_tm_t *result)
{
	if (!utc || !result) return;
	eso_dt_cal_time(utc, utc->tm_zone, 0, result);
	utc->tm_zone = 0;
}

/**
 * 时间偏移计算
 */
void eso_dt_cal_time(es_tm_t *time, int hour, int minute, es_tm_t *result)
{
	es_uint32_t seconds;
	
	if (!time || !result) return;
	
	seconds = time->tm_time_sec + hour*ES_SEC_PER_HOUR + minute*ES_SEC_PER_MIN;
	eso_gm_time(seconds, result);
}

/**
 * 校验时间是否有效
 */
es_bool_t eso_dt_is_valid(es_tm_t *t)
{
	/* error handling */
//    if (t->tm_year > ES_MAX_YEAR || t->tm_year < ES_MIN_YEAR) {
//	    return FALSE;
//	}

    if (t->tm_mon > 12 || t->tm_hour > 23 || t->tm_min > 59 || t->tm_sec > 59) {
	    return FALSE;
	}
	
    if (t->tm_mon == 2) {
	    if (t->tm_mday > (g_dt_day_per_mon[t->tm_mon - 1] + eso_dt_is_leap_year(t->tm_year))) {
		    return FALSE;
		}
	}
    else {
	    if (t->tm_mday > g_dt_day_per_mon[t->tm_mon - 1]) {
		    return FALSE;
		}
	}
	
    return TRUE;
}

/**
 * convert from UTC second to es_tm_t struct
 * declaration: eso_gm_time
 * description: 将秒数转化为tm结构数据
 * parameter  : es_uint32_t gmt_sec = 从1970-1-1以来的格林威治时间总秒数
 * return     : 返回tm结构数据
 */
void eso_gm_time(es_uint32_t gmt_sec, es_tm_t *tm)
{
	int  sec, min, hour, mday, mon, year, wday, yday, days;
	
	tm->tm_time_sec = gmt_sec; //added 20101220
	
	days = gmt_sec / 86400;

	/* Jaunary 1, 1970 was Thursday */
	wday = (4 + days) % 7;

	gmt_sec %= 86400;
	hour = gmt_sec / 3600;
	gmt_sec %= 3600;
	min = gmt_sec / 60;
	sec = gmt_sec % 60;

	/* the algorithm based on Gauss's formula */

	days = days - (31 + 28) + 719527;

	year = days * 400 / (365 * 400 + 100 - 4 + 1);
	yday = days - (365 * year + year / 4 - year / 100 + year / 400);

	mon = (yday + 31) * 12 / 367; 
	mday = yday - (mon * 367 / 12 - 31);

	mon += 2;

	if (yday >= 306) {
		year++;
		mon -= 12;

		if (mday == 0) {
			/* January 31 */
			mon = 1;
			mday = 31;

		} else if (mon == 2) {

			if ((year % 4 == 0) && (year % 100 || (year % 400 == 0))) {
				if (mday > 29) {
					mon = 3;
					mday -= 29;
				}

			} else if (mday > 28) {
				mon = 3;
				mday -= 28;
			}
		}
	}

	tm->tm_sec  = (es_uint8_t)sec;
	tm->tm_min  = (es_uint8_t)min;
	tm->tm_hour = (es_uint8_t)hour;
	tm->tm_mday = (es_uint8_t)mday-1;
	tm->tm_mon  = (es_uint8_t)mon-1;
	tm->tm_year = year-1900;
	tm->tm_wday = (es_uint8_t)wday;
	tm->tm_zone = 0;
}

/**
 * convert es_tm_t format of time to seconds after 1970/1/1
 * declaration: eso_time_gm
 * description: 将tm结构数据转化为秒数
 * parameter  : tm结构数据
 * return     : 返回从1970-1-1以来的格林威治时间总秒数
 */
es_uint32_t eso_time_gm(es_tm_t *tm)
{
	int year = tm->tm_year;
    int days;
    static const int dayoffset[12] =
    {306, 337, 0, 31, 61, 92, 122, 153, 184, 214, 245, 275};

    /* shift new year to 1st March in order to make leap year calc easy */
	
    if (tm->tm_mon < 2)
        year--;
	
    /* Find number of days since 1st March 1900 (in the Gregorian calendar). */
	
    days = year * 365 + year / 4 - year / 100 + (year / 100 + 3) / 4;
    days += dayoffset[tm->tm_mon] + tm->tm_mday;// - 1; fixbug on 20130704
    days -= 25508;              /* 1 jan 1970 is 25508 days since 1 mar 1900 */
    return ((days * 24 + tm->tm_hour) * 60 + tm->tm_min) * 60 + tm->tm_sec;
}

/**
 * convert y/m/d/h/m/s data to seconds after 1970/1/1
 * declaration: eso_time_sec
 * description: 将年月日数据转化为秒数
 * parameter  : y/m/d/h/m/s
 * return     : 返回从1970-1-1以来的格林威治时间总秒数
 */
es_uint32_t eso_time_sec(es_uint16_t year, es_uint8_t mon, es_uint8_t day, 
                         es_uint8_t hour, es_uint8_t min, es_uint8_t sec)
{
	es_tm_t tm = {0,0,0,0,0,0,0,0,0};
	
	tm.tm_year = year - 1900;
	tm.tm_mon = mon - 1;
	tm.tm_mday = day - 1;
	tm.tm_hour = hour;
	tm.tm_min = min;
	tm.tm_sec = sec;
	
	return eso_time_gm(&tm);
}
