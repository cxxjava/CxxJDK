#include "ECalendar.hh"
#include "ELLong.hh"

namespace efc {

//@see: http://www.iro.umontreal.ca/~lapalme/java/util/GregorianCalendar.java
//@see: jdk1.4.2_src/java/util/GregorianCalendar.java

/* ERA */
static const int BC = 0;
static const int AD = 1;

/* January 1, year 1 (Gregorian) */
static const int JAN_1_1_JULIAN_DAY = 1721426;

/* January 1, 1970 (Gregorian) */
static const int EPOCH_JULIAN_DAY   = 2440588;

/* 0-based, for day-in-year */
static const int NUM_DAYS[] = {0,31,59,90,120,151,181,212,243,273,304,334};

/* 0-based, for day-in-year */
static const int LEAP_NUM_DAYS[] = {0,31,60,91,121,152,182,213,244,274,305,335};

/**
 * Useful millisecond constants.  Although ONE_DAY and ONE_WEEK can fit
 * into ints, they must be longs in order to prevent arithmetic overflow
 * when performing (CR 4173516).
 */
static const int  ONE_SECOND = 1000;
static const int  ONE_MINUTE = 60*ONE_SECOND;
static const int  ONE_HOUR   = 60*ONE_MINUTE;
static const long ONE_DAY= 24*ONE_HOUR;

/**
 * The point at which the Gregorian calendar rules are used, measured in
 * milliseconds from the standard epoch.  Default is October 15, 1582
 * (Gregorian) 00:00:00 UTC or -12219292800000L.  For this value, October 4,
 * 1582 (Julian) is followed by October 15, 1582 (Gregorian).  This
 * corresponds to Julian day number 2299161.
 */
static const llong gregorianCutover = LLONG(-12219292800000);

/**
 * The year of the gregorianCutover, with 0 representing
 * 1 BC, -1 representing 2 BC, etc.
 */
static const int gregorianCutoverYear = 1582;

static const char months[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                          "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
static const char days[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

//==============================================================================

ECalendar::~ECalendar()
{
}

ECalendar::ECalendar()
{
	es_tm_t tm;
	eso_dt_now(&tm);
	this->timezone = tm.tm_zone;
	setTimeInMillis(tm.tm_time_sec*(llong)1000 + tm.tm_time_usec/1000);
}

ECalendar::ECalendar(llong millis)
{
	es_tm_t tm;
	eso_dt_now(&tm);
	this->timezone = tm.tm_zone;
	setTimeInMillis(millis);
}

ECalendar::ECalendar(llong millis, int timezone)
{
	this->timezone = timezone;
	setTimeInMillis(millis);
}

llong ECalendar::getTimeInMillis()
{
    if (!isTimeSet) {
        computeTime();
        isTimeSet = true;
    }
    return this->time;
}

void ECalendar::setTimeInMillis(llong millis)
{
	isTimeSet = true;
    this->fields[DAY_OF_WEEK] = 0;
    this->time = millis;
    computeFields();
}

EDate ECalendar::getTime()
{
	return EDate(getTimeInMillis());
}

void ECalendar::setTime(EDate& date)
{
	setTimeInMillis(date.getTime());
}

int ECalendar::get(int field)
{
	getTimeInMillis();
	computeFields();
	return this->fields[field];
}

void ECalendar::set(int field, int value)
{
	if (field == HOUR_OF_DAY) {
		_isSet[HOUR] = _isSet[AM_PM] = false;
	} else if (field == HOUR) {
		_isSet[HOUR_OF_DAY] = false;
	} else if (field == AM_PM) {
		if (value == AM || value == PM) {
			_isSet[HOUR_OF_DAY] = false;
		}
	} else if (field == DAY_OF_MONTH) {
		_isSet[DAY_OF_WEEK] = false;
	}

	isTimeSet = false;
	this->_isSet[field] = true;
	this->fields[field] = value;
}

void ECalendar::set(int year, int month, int date, int hourOfDay, int minute, int second)
{
	set(YEAR, year);
	set(MONTH, month);
	set(DATE, date);
	if (hourOfDay != 0) {
		set(HOUR_OF_DAY, hourOfDay);
	}
	if (minute != 0) {
		set(MINUTE, minute);
	}
	if (second != 0) {
		set(SECOND, second);
	}
}

void ECalendar::clear()
 {
	for (int i = 0; i < FIELDS;) {
		if (i == YEAR)
			fields[i] = 1970;
		else if (i == DATE)
			fields[i] = 1;
		else
			fields[i] = 0; // UNSET == 0
		_isSet[i++] = false;
	}
	isTimeSet = false;
}

void ECalendar::clear(int field)
 {
	if (field == YEAR)
		fields[field] = 1970;
	else if (field == DATE)
		fields[field] = 1;
	else
		fields[field] = 0; // UNSET == 0
	_isSet[field] = false;
	isTimeSet = false;
}

boolean ECalendar::equals(ECalendar& that)
{
	if (this == &that) {
        return true;
    }
	return getTimeInMillis() == that.getTimeInMillis()
			&& timezone == that.timezone;
}

boolean ECalendar::before(ECalendar& when)
{
	return (getTimeInMillis() < when.getTimeInMillis());
}

boolean ECalendar::after(ECalendar& when)
{
	return (getTimeInMillis() > when.getTimeInMillis());
}

void ECalendar::add(int field, int amount)
{
	if (amount != 0) {
		int n = get(field) + amount;
		set(field, n);
	}
}

void ECalendar::setTimeZone(int value)
{
	timezone = value;
	getTimeInMillis();
	computeFields();
}

int ECalendar::getTimeZone()
{
	return timezone;
}

int ECalendar::getFirstDayOfWeek() {
	//TODO...
	return SUNDAY;
}

int ECalendar::getMinimalDaysInFirstWeek() {
	//TODO...
	return 1;
}

boolean ECalendar::isLeapYear(int year) {
	return year >= gregorianCutoverYear ?
			((year%4 == 0) && ((year%100 != 0) || (year%400 == 0))) : // Gregorian
			(year%4 == 0); // Julian
}

int ECalendar::yearLength() {
	return isLeapYear(this->fields[YEAR]) ? 366 : 365;
}

int ECalendar::yearLength(int year) {
	return isLeapYear(year) ? 366 : 365;
}

int ECalendar::weekNumber(int dayOfPeriod, int dayOfWeek) {
	// Determine the day of the week of the first day of the period
	// in question (either a year or a month).  Zero represents the
	// first day of the week on this calendar.
	int periodStartDayOfWeek = (dayOfWeek - getFirstDayOfWeek() - dayOfPeriod + 1) % 7;
	if (periodStartDayOfWeek < 0) periodStartDayOfWeek += 7;

	// Compute the week number.  Initially, ignore the first week, which
	// may be fractional (or may not be).  We add periodStartDayOfWeek in
	// order to fill out the first week, if it is fractional.
	int weekNo = (dayOfPeriod + periodStartDayOfWeek - 1)/7;

	// If the first week is long enough, then count it.  If
	// the minimal days in the first week is one, or if the period start
	// is zero, we always increment weekNo.
	if ((7 - periodStartDayOfWeek) >= getMinimalDaysInFirstWeek()) ++weekNo;

	return weekNo;
}

void ECalendar::computeFields()
{
	int rawOffset = (timezone * 60 * 60 * 1000); //calculate timezone @cxxjava
    llong localMillis = time + rawOffset;
    
    // Time to fields takes the wall millis (Standard or DST).
    timeToFields(localMillis);

    llong days = (llong)(localMillis / ONE_DAY);
    int millisInDay = (int)(localMillis - (days * ONE_DAY));
    if (millisInDay < 0) millisInDay += ONE_DAY;

    // Call getOffset() to get the TimeZone offset.  The millisInDay value must
	// be standard local millis.
    /**
    int dstOffset = getTimeZone().getOffset(era,year,month,date,dayOfWeek,millisInDay,
                                                    monthLength(month)) - rawOffset;
	*/
	int dstOffset = 0; //TODO...

	// Adjust our millisInDay for DST, if necessary.
	millisInDay += dstOffset;

    // If DST has pushed us into the next day,
    // we must call timeToFields() again.
    // This happens in DST between 12:00 am and 1:00 am every day.
    // The call to timeToFields() will give the wrong day,
    // since the Standard time is in the previous day
    if (millisInDay >= ONE_DAY) {
    	llong dstMillis = localMillis + dstOffset;
        millisInDay -= ONE_DAY;
        // As above, check for and pin extreme values
		if (localMillis > 0 && dstMillis < 0 && dstOffset > 0) {
			dstMillis = ELLong::MAX_VALUE;
		} else if (localMillis < 0 && dstMillis > 0 && dstOffset < 0) {
			dstMillis = ELLong::MIN_VALUE;
		}
        timeToFields(dstMillis);
    }

    // Fill in all time-related fields based on millisInDay.
    // so as not to perturb flags.
    this->fields[MILLISECOND] = millisInDay % 1000;
    millisInDay /= 1000;

    this->fields[SECOND] = millisInDay % 60;
    millisInDay /= 60;

    this->fields[MINUTE] = millisInDay % 60;
    millisInDay /= 60;

    this->fields[HOUR_OF_DAY] = millisInDay;
    this->fields[AM_PM] = millisInDay / 12;
    this->fields[HOUR] = millisInDay % 12;

    this->fields[ZONE_OFFSET] = rawOffset;
    this->fields[DST_OFFSET] = dstOffset;
}

void ECalendar::timeToFields(llong theTime)
{
	int dayOfWeek, dayOfYear, rawYear;
    boolean isLeap;

    // Compute the year, month, and day of month from the given millis
    if (theTime >= gregorianCutover) {

        // The Gregorian epoch day is zero for Monday January 1, year 1.
        llong gregorianEpochDay =
            millisToJulianDay(theTime) - JAN_1_1_JULIAN_DAY;

        // Here we convert from the day number to the multiple radix
        // representation.  We use 400-year, 100-year, and 4-year cycles.
        // For example, the 4-year cycle has 4 years + 1 leap day; giving
        // 1461 == 365*4 + 1 days.
        int rem;

        // 400-year cycle length
        int n400 = floorDivide((int)gregorianEpochDay, 146097, &rem);

        // 100-year cycle length
        int n100 = floorDivide(rem, 36524, &rem);

        // 4-year cycle length
        int n4 = floorDivide(rem, 1461, &rem);

        int n1 = floorDivide(rem, 365, &rem);
        rawYear = 400*n400 + 100*n100 + 4*n4 + n1;

        // zero-based day of year
        dayOfYear = rem;

        // Dec 31 at end of 4- or 400-yr cycle
        if (n100 == 4 || n1 == 4) {
            dayOfYear = 365;
        } else {
            ++rawYear;
        }
		
        // equiv. to (rawYear%4 == 0)
        isLeap =
            ((rawYear&0x3) == 0) && (rawYear%100 != 0 || rawYear%400 == 0);

        // Gregorian day zero is a Monday
        this->fields[DAY_OF_WEEK] = (int)((gregorianEpochDay+1) % 7);
    } else {
        // The Julian epoch day (not the same as Julian Day)
        // is zero on Saturday December 30, 0 (Gregorian).
        llong julianEpochDay =
            millisToJulianDay(theTime) - (JAN_1_1_JULIAN_DAY - 2);

        rawYear = (int) floorDivide(4*julianEpochDay + 1464, 1461);

        // Compute the Julian calendar day number for January 1, year
        llong january1 = 365*(rawYear-1) + floorDivide(rawYear-1, 4);
        dayOfYear = (int)(julianEpochDay - january1); // 0-based

        // Julian leap years occurred historically every 4 years starting
        // with 8 AD.  Before 8 AD the spacing is irregular; every 3 years
        // from 45 BC to 9 BC, and then none until 8 AD.  However, we don't
        // implement this historical detail; instead, we implement the
        // computationally cleaner proleptic calendar, which assumes
        // consistent 4-year cycles throughout time.

        // equiv. to (rawYear%4 == 0)
        isLeap = ((rawYear&0x3) == 0);

        // Julian calendar day zero is a Saturday
        this->fields[DAY_OF_WEEK] = (int)((julianEpochDay-1) % 7);
    }

    // Common Julian/Gregorian calculation
    int correction = 0;

    // zero-based DOY for March 1
    int march1 = isLeap ? 60 : 59;

    if (dayOfYear >= march1) correction = isLeap ? 1 : 2;

    // zero-based month
    int month_field = (12 * (dayOfYear + correction) + 6) / 367;

    // one-based DOM
    int date_field = dayOfYear -
        (isLeap ? LEAP_NUM_DAYS[month_field] : NUM_DAYS[month_field]) + 1;

    // Normalize day of week
    this->fields[DAY_OF_WEEK] += (this->fields[DAY_OF_WEEK] < 0) ? (SUNDAY+7) : SUNDAY;
    dayOfWeek = this->fields[DAY_OF_WEEK];

    this->fields[YEAR] = rawYear;

    // If year is < 1 we are in BC
    if (this->fields[YEAR] < 1) {
        this->fields[YEAR] = 1 - this->fields[YEAR];
    }

    // 0-based
    this->fields[MONTH] = month_field + JANUARY;
    this->fields[DATE] = date_field;

    this->fields[DAY_OF_YEAR] = ++dayOfYear; // Convert from 0-based to 1-based

	// Compute the week of the year.  Valid week numbers run from 1 to 52
	// or 53, depending on the year, the first day of the week, and the
	// minimal days in the first week.  Days at the start of the year may
	// fall into the last week of the previous year; days at the end of
	// the year may fall into the first week of the next year.
	int relDow = (dayOfWeek + 7 - getFirstDayOfWeek()) % 7; // 0..6
	int relDowJan1 = (dayOfWeek - dayOfYear + 701 - getFirstDayOfWeek()) % 7; // 0..6
	int woy = (dayOfYear - 1 + relDowJan1) / 7; // 0..53
	if ((7 - relDowJan1) >= getMinimalDaysInFirstWeek()) {
		++woy;
		// Check to see if we are in the last week; if so, we need
		// to handle the case in which we are the first week of the
		// next year.
		int lastDoy = yearLength();
		int lastRelDow = (relDow + lastDoy - dayOfYear) % 7;
		if (lastRelDow < 0) lastRelDow += 7;
		if (dayOfYear > 359 && // Fast check which eliminates most cases
			(6 - lastRelDow) >= getMinimalDaysInFirstWeek() &&
			(dayOfYear + 7 - relDow) > lastDoy) woy = 1;
	}
	else if (woy == 0) {
		// We are the last week of the previous year.
		int prevDoy = dayOfYear + yearLength(rawYear - 1);
		woy = weekNumber(prevDoy, dayOfWeek);
	}
	this->fields[WEEK_OF_YEAR] = woy;

	this->fields[WEEK_OF_MONTH] = weekNumber(date_field, dayOfWeek);
	this->fields[DAY_OF_WEEK_IN_MONTH] = (date_field-1) / 7 + 1;
}

EString ECalendar::toString()
{
	return ECalendar::toString(this);
}

EString ECalendar::toString(const char *format)
{
	es_string_t *to = NULL;
	to = eso_fmt_timezone(&to, format, getTimeInMillis(), getTimeZone());
	if (to) {
		EString out(to);
		eso_mfree(to);
		return out;
	}
	throw EIllegalArgumentException(__FILE__, __LINE__);
}

EString ECalendar::toString(ECalendar* calendar)
{
	// Printing in the absence of a Calendar
    // implementation class is not supported
    if (calendar == null) {
        return EString("Thu Jan 01 00:00:00 UTC 1970");
    }
	
    int dow = calendar->get(DAY_OF_WEEK);
    int month = calendar->get(MONTH);
    int day = calendar->get(DAY_OF_MONTH);
    int hour_of_day = calendar->get(HOUR_OF_DAY);
    int minute = calendar->get(MINUTE);
    int seconds = calendar->get(SECOND);
    int year = calendar->get(YEAR);
    int timezone = calendar->getTimeZone();

    EString out;
    char digit[10];
    
    out.concat(days[dow-1]).concat(" ");
	out.concat(months[month]).concat(" ");
	eso_sprintf(digit, "%02d", day);
	out.concat(digit).concat(" ");
	eso_sprintf(digit, "%02d", hour_of_day);
	out.concat(digit).concat(":");
	eso_sprintf(digit, "%02d", minute);
	out.concat(digit).concat(":");
	eso_sprintf(digit, "%02d", seconds);
	out.concat(digit).concat(" ");
	eso_sprintf(digit, "%02d", timezone);
	out.concat(digit).concat(" ");
	eso_sprintf(digit, "%04d", year);
	out.concat(digit);
	
    return out;
}

EString ECalendar::toISO8601String(ECalendar* calendar)
{
	// Printing in the absence of a Calendar
    // implementation class is not supported
    if (calendar == null) {
        return EString("0000 00 00 00 00 00 +0000");
    }

    int year = calendar->get(YEAR);
    int month = calendar->get(MONTH) + 1;
    int day = calendar->get(DAY_OF_MONTH);
    int hour_of_day = calendar->get(HOUR_OF_DAY);
    int minute = calendar->get(MINUTE);
    int seconds = calendar->get(SECOND);
    int timezone = calendar->getTimeZone();
	
	EString out;
    char digit[10];
    
    eso_sprintf(digit, "%04d", year);
	out.concat(digit).concat(" ");
    eso_sprintf(digit, "%02d", month);
	out.concat(digit).concat(" ");
	eso_sprintf(digit, "%02d", day);
	out.concat(digit).concat(" ");
	eso_sprintf(digit, "%02d", hour_of_day);
	out.concat(digit).concat(" ");
	eso_sprintf(digit, "%02d", minute);
	out.concat(digit).concat(" ");
	eso_sprintf(digit, "%02d", seconds);
	out.concat(digit).concat(" ");
	
	if (timezone < 0)
		out.concat("-");
	else {
		out.concat("+");
	}
	eso_sprintf(digit, "%02d", abs(timezone));
	out.concat(digit).concat("00");

    return out;
}

void ECalendar::computeTime()
{
	correctTime();

    // This function takes advantage of the fact that unset fields in
    // the time field list have a value of zero.

    // First, use the year to determine whether to use the Gregorian or the
    // Julian calendar. If the year is not the year of the cutover, this
    // computation will be correct. But if the year is the cutover year,
    // this may be incorrect. In that case, assume the Gregorian calendar,
    // make the computation, and then recompute if the resultant millis
    // indicate the wrong calendar has been assumed.

    // A date such as Oct. 10, 1582 does not exist in a Gregorian calendar
    // with the default changeover of Oct. 15, 1582, since in such a
    // calendar Oct. 4 (Julian) is followed by Oct. 15 (Gregorian).  This
    // algorithm will interpret such a date using the Julian calendar,
    // yielding Oct. 20, 1582 (Gregorian).
    int year = this->fields[YEAR];
    boolean isGregorian = year >= gregorianCutoverYear;
    llong julianDay = calculateJulianDay(isGregorian, year);

    //if DAY_OF_WEEK was set more recently than DAY_OF_MONTH and is correct 
    //then time is computed using current week and day of week
    if(_isSet[DAY_OF_WEEK] && fields[DAY_OF_WEEK] >= SUNDAY && fields[DAY_OF_WEEK] <= SATURDAY) {
        julianDay += fields[DAY_OF_WEEK] - julianDayToDayOfWeek(julianDay);
        fields[DATE] += fields[DAY_OF_WEEK] - julianDayToDayOfWeek(julianDay);
    }

    llong millis = julianDayToMillis(julianDay);

    // The following check handles portions of the cutover year BEFORE the
    // cutover itself happens. The check for the julianDate number is for a
    // rare case; it's a hardcoded number, but it's efficient.  The given
    // Julian day number corresponds to Dec 3, 292269055 BC, which
    // corresponds to millis near Long.MIN_VALUE.  The need for the check
    // arises because for extremely negative Julian day numbers, the millis
    // actually overflow to be positive values. Without the check, the
    // initial date is interpreted with the Gregorian calendar, even when
    // the cutover doesn't warrant it.
    if (isGregorian != (millis >= gregorianCutover) &&
        julianDay != LLONG(-106749550580)) { // See above

        julianDay = calculateJulianDay(!isGregorian, year);
        millis = julianDayToMillis(julianDay);
    }

    // Do the time portion of the conversion.

    int millisInDay = 0;

    // Hours
    // Don't normalize here; let overflow bump into the next period.
    // This is consistent with how we handle other fields.
    millisInDay += this->fields[HOUR_OF_DAY];
    millisInDay *= 60;

    // now get minutes
    millisInDay += this->fields[MINUTE];
    millisInDay *= 60;

    // now get seconds
    millisInDay += this->fields[SECOND];
    millisInDay *= 1000;

    // now get millis
    millisInDay += this->fields[MILLISECOND];

   // Now add date and millisInDay together, to make millis contain local wall
    // millis, with no zone or DST adjustments
    millis += millisInDay;

    // Normalize the millisInDay to 0..ONE_DAY-1.  If the millis is out
    // of range, then we must call timeToFields() to recompute our
    // fields.
    int normalizedMillisInDay;
    floorDivide((int)millis, (int)ONE_DAY, &normalizedMillisInDay);

    // Note: Because we pass in wall millisInDay, rather than
    // standard millisInDay, we interpret "1:00 am" on the day
    // of cessation of DST as "1:00 am Std" (assuming the time
    // of cessation is 2:00 am).

    // Store our final computed GMT time, with timezone adjustments.
    int zoneOffset = (timezone * 60 * 60 * 1000);
    time = millis - zoneOffset;//- dstOffset;
}

llong ECalendar::calculateJulianDay(boolean isGregorian, int year)
{
	int month = 0;

    month = this->fields[MONTH] - JANUARY;

    // If the month is out of range, adjust it into range
    if (month < 0 || month > 11) {
        int rem;
        year += floorDivide(month, 12, &rem);
        month = rem;
    }

    boolean isLeap = year%4 == 0;

    llong julianDay =
        LLONG(365)*(year - 1) + floorDivide((year - 1), 4) + (JAN_1_1_JULIAN_DAY - 3);

    if (isGregorian) {
        isLeap = isLeap && ((year%100 != 0) || (year%400 == 0));
        // Add 2 because Gregorian calendar starts 2 days after Julian calendar
        julianDay +=
            floorDivide((year - 1), 400) - floorDivide((year - 1), 100) + 2;
    }

    // At this point julianDay is the 0-based day BEFORE the first day of
    // January 1, year 1 of the given calendar.  If julianDay == 0, it
    // specifies (Jan. 1, 1) - 1, in whatever calendar we are using (Julian
    // or Gregorian).
    julianDay += isLeap ? LEAP_NUM_DAYS[month] : NUM_DAYS[month];
    julianDay += this->fields[DATE];
    return julianDay;
}

void ECalendar::correctTime()
{
	int value;

    if (_isSet[HOUR_OF_DAY]) {
        value = this->fields[HOUR_OF_DAY] % 24;
        this->fields[HOUR_OF_DAY] = value;
        this->fields[AM_PM] = (value < 12) ? AM : PM;
        this->_isSet[HOUR_OF_DAY] = false;
        return;
    }

    if (_isSet[HOUR]||_isSet[AM_PM]) {
        value = this->fields[HOUR];
        if (value > 12) {
            this->fields[HOUR_OF_DAY] = (value % 12) + 12;
            this->fields[HOUR] = value % 12;
            this->fields[AM_PM] = PM;
        } else {
            if (this->fields[AM_PM] == PM) {
                this->fields[HOUR_OF_DAY] = (value % 12) + 12;
            } else {
                this->fields[HOUR_OF_DAY] = value;
            }
        }
        this->_isSet[AM_PM] = false;
        this->_isSet[HOUR] = false;
    }
}

llong ECalendar::millisToJulianDay(llong millis)
{
	return EPOCH_JULIAN_DAY + floorDivide(millis, ONE_DAY);
}

llong ECalendar::julianDayToMillis(llong julian)
{
	return (julian - EPOCH_JULIAN_DAY) * ONE_DAY;
}

int ECalendar::julianDayToDayOfWeek(llong julian)
{
	// If julian is negative, then julian%7 will be negative, so we adjust
	// accordingly.  We add 1 because Julian day 0 is Monday.
	int dayOfWeek = (int)((julian + 1) % 7);
	return dayOfWeek + ((dayOfWeek < 0) ? (7 + SUNDAY) : SUNDAY);
}

llong ECalendar::floorDivide(llong numerator, llong denominator)
{
	// We do this computation in order to handle
    // a numerator of Long.MIN_VALUE correctly
    return (numerator >= 0) ?
        (llong)numerator / (llong)denominator :
        ((llong)(numerator + 1) / (llong)denominator) - 1;
}

int ECalendar::floorDivide(int numerator, int denominator, int *remainder)
{
	if (numerator >= 0) {
		*remainder = numerator % denominator;
		return numerator / denominator;
	}
	int quotient = ((numerator + 1) / denominator) - 1;
	*remainder = numerator - (quotient * denominator);
	return quotient;
}

boolean ECalendar::equals(ECalendar* obj)
{
	if (!obj) return false;
	if (this == obj) {
		return true;
	}
	return getTimeInMillis() == obj->getTimeInMillis()
			&& timezone == obj->timezone;
}

boolean ECalendar::equals(EObject* obj) {
	if (!obj) return false;
	if (this == obj) {
		return true;
	}
	ECalendar* that = dynamic_cast<ECalendar*>(obj);
	if (!that) return false;
	return getTimeInMillis() == that->getTimeInMillis()
			&& timezone == that->timezone;
}

int  ECalendar::hashCode() {
	llong timex = getTimeInMillis();
	return (int) ((timex & 0xffffffffL) ^ (timex >> 32));
}

ECalendar ECalendar::parseOf(const char *format, const char *source) THROWS(EIllegalArgumentException)
{
	int tz;
	es_uint32_t tm = eso_parse_time(&tz, format, source);
	if (tm == 0) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Parse formated datetime string failed");
	}
	return ECalendar(llong(tm) * llong(1000), tz);
}

} /* namespace efc */
