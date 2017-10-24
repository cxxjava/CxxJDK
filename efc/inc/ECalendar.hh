#ifndef __ECalendar_H__
#define __ECalendar_H__

#include "EObject.hh"
#include "EString.hh"
#include "EDate.hh"
#include "EIllegalArgumentException.hh"

namespace efc {

/*
 * sample:
 EString getDateString(llong time) {
	EString str;
    ECalendar *calendar = new ECalendar();
	
    calendar->setTimeInMillis(ESystem::currentTimeMillis());
	str = EInteger::toString(calendar->get(ECalendar::YEAR)) + "Äê"
		+ EInteger::toString(calendar->get(ECalendar::MONTH)+1) + "ÔÂ"
		+ EInteger::toString(calendar->get(ECalendar::DATE)+1) + "ÈÕ"
		+ (calendar->get(ECalendar::AM_PM)==ECalendar::PM ? "ÏÂÎç" : "ÉÏÎç")
		+ EInteger::toString(calendar->get(ECalendar::HOUR)) + "Ê±"
		+ EInteger::toString(calendar->get(ECalendar::MINUTE)) + "·Ö"
		+ EInteger::toString(calendar->get(ECalendar::SECOND)) + "Ãë";
	
	delete calendar;

    return str;
 }
 */

class ECalendar : public EObject
{
public:
	enum {
		/**
		 * Field number for <code>get</code> and <code>set</code> indicating the
		 * year. This is a calendar-specific value.
		 */
		YEAR = 1,
		/**
		 * Field number for <code>get</code> and <code>set</code> indicating the
		 * month. This is a calendar-specific value.
		 */
		MONTH = 2,
		/**
		 * Field number for <code>get</code> and <code>set</code> indicating the
		 * week number within the current year.  The first week of the year, as
		 * defined by <code>getFirstDayOfWeek()</code> and
		 * <code>getMinimalDaysInFirstWeek()</code>, has value 1.  Subclasses define
		 * the value of <code>WEEK_OF_YEAR</code> for days before the first week of
		 * the year.
		 *
		 * @see #getFirstDayOfWeek
		 * @see #getMinimalDaysInFirstWeek
		 */
		WEEK_OF_YEAR = 3,
		/**
		 * Field number for <code>get</code> and <code>set</code> indicating the
		 * week number within the current month.  The first week of the month, as
		 * defined by <code>getFirstDayOfWeek()</code> and
		 * <code>getMinimalDaysInFirstWeek()</code>, has value 1.  Subclasses define
		 * the value of <code>WEEK_OF_MONTH</code> for days before the first week of
		 * the month.
		 *
		 * @see #getFirstDayOfWeek
		 * @see #getMinimalDaysInFirstWeek
		 */
		WEEK_OF_MONTH = 4,
		/**
		 * Field number for <code>get</code> and <code>set</code> indicating the
		 * day of the month. This is a synonym for <code>DAY_OF_MONTH</code>.
		 * @see #DAY_OF_MONTH
		 */
		DATE = 5,
		/**
		 * Field number for <code>get</code> and <code>set</code> indicating the
		 * day of the month. This is a synonym for <code>DATE</code>.
		 * @see #DATE
		 */
		DAY_OF_MONTH = 5,
		/**
		 * Field number for <code>get</code> and <code>set</code> indicating the day
		 * number within the current year.  The first day of the year has value 1.
		 */
		DAY_OF_YEAR = 6,
		/**
		 * Field number for <code>get</code> and <code>set</code> indicating the
		 * day of the week.
		 */
		DAY_OF_WEEK = 7,
		/**
		 * Field number for <code>get</code> and <code>set</code> indicating the
		 * ordinal number of the day of the week within the current month. Together
		 * with the <code>DAY_OF_WEEK</code> field, this uniquely specifies a day
		 * within a month.  Unlike <code>WEEK_OF_MONTH</code> and
		 * <code>WEEK_OF_YEAR</code>, this field's value does <em>not</em> depend on
		 * <code>getFirstDayOfWeek()</code> or
		 * <code>getMinimalDaysInFirstWeek()</code>.  <code>DAY_OF_MONTH 1</code>
		 * through <code>7</code> always correspond to <code>DAY_OF_WEEK_IN_MONTH
		 * 1</code>; <code>8</code> through <code>14</code> correspond to
		 * <code>DAY_OF_WEEK_IN_MONTH 2</code>, and so on.
		 * <code>DAY_OF_WEEK_IN_MONTH 0</code> indicates the week before
		 * <code>DAY_OF_WEEK_IN_MONTH 1</code>.  Negative values count back from the
		 * end of the month, so the last Sunday of a month is specified as
		 * <code>DAY_OF_WEEK = SUNDAY, DAY_OF_WEEK_IN_MONTH = -1</code>.  Because
		 * negative values count backward they will usually be aligned differently
		 * within the month than positive values.  For example, if a month has 31
		 * days, <code>DAY_OF_WEEK_IN_MONTH -1</code> will overlap
		 * <code>DAY_OF_WEEK_IN_MONTH 5</code> and the end of <code>4</code>.
		 *
		 * @see #DAY_OF_WEEK
		 * @see #WEEK_OF_MONTH
		 */
		DAY_OF_WEEK_IN_MONTH = 8,
		/**
		 * Field number for <code>get</code> and <code>set</code> indicating
		 * whether the <code>HOUR</code> is before or after noon.
		 * E.g., at 10:04:15.250 PM the <code>AM_PM</code> is <code>PM</code>.
		 * @see #AM
		 * @see #PM
		 * @see #HOUR
		 */
		AM_PM = 9,
		/**
		 * Field number for <code>get</code> and <code>set</code> indicating the
		 * hour of the morning or afternoon. <code>HOUR</code> is used for the
		 * 12-hour clock.
		 * E.g., at 10:04:15.250 PM the <code>HOUR</code> is 10.
		 * @see #AM_PM
		 * @see #HOUR_OF_DAY
		 */
		HOUR = 10,
		/**
		 * Field number for <code>get</code> and <code>set</code> indicating the
		 * hour of the day. <code>HOUR_OF_DAY</code> is used for the 24-hour clock.
		 * E.g., at 10:04:15.250 PM the <code>HOUR_OF_DAY</code> is 22.
		 */
		HOUR_OF_DAY = 11,
		/**
		 * Field number for <code>get</code> and <code>set</code> indicating the
		 * minute within the hour.
		 * E.g., at 10:04:15.250 PM the <code>MINUTE</code> is 4.
		 */
		MINUTE = 12,
		/**
		 * Field number for <code>get</code> and <code>set</code> indicating the
		 * second within the minute.
		 * E.g., at 10:04:15.250 PM the <code>SECOND</code> is 15.
		 */
		SECOND = 13,
		/**
		 * Field number for <code>get</code> and <code>set</code> indicating the
		 * millisecond within the second.
		 * E.g., at 10:04:15.250 PM the <code>MILLISECOND</code> is 250.
		 */
		MILLISECOND = 14,
		/**
		 * Field number for <code>get</code> and <code>set</code>
		 * indicating the raw offset from GMT in milliseconds.
		 * <p>
		 * This field reflects the correct GMT offset value of the time
		 * zone of this <code>Calendar</code> if the
		 * <code>TimeZone</code> implementation subclass supports
		 * historical GMT offset changes.
		 */
		ZONE_OFFSET = 15,
		/**
		 * Field number for <code>get</code> and <code>set</code> indicating the
		 * daylight saving offset in milliseconds.
		 * <p>
		 * This field reflects the correct daylight saving offset value of
		 * the time zone of this <code>Calendar</code> if the
		 * <code>TimeZone</code> implementation subclass supports
		 * historical Daylight Saving Time schedule changes.
		 */
		DST_OFFSET = 16
	};

	/*
	 * The number of fields for the array below.
	 */
	static const int FIELDS = 17;
	
	enum {
		/**
		 * Value of the <code>DAY_OF_WEEK</code> field indicating
		 * Sunday.
		 */
		SUNDAY = 1,
		/**
		 * Value of the <code>DAY_OF_WEEK</code> field indicating
		 * Monday.
		 */
		MONDAY = 2,
		/**
		 * Value of the <code>DAY_OF_WEEK</code> field indicating
		 * Tuesday.
		 */
		TUESDAY = 3,
		/**
		 * Value of the <code>DAY_OF_WEEK</code> field indicating
		 * Wednesday.
		 */
		WEDNESDAY = 4,
		/**
		 * Value of the <code>DAY_OF_WEEK</code> field indicating
		 * Thursday.
		 */
		THURSDAY = 5,
		/**
		 * Value of the <code>DAY_OF_WEEK</code> field indicating
		 * Friday.
		 */
		FRIDAY = 6,
		/**
		 * Value of the <code>DAY_OF_WEEK</code> field indicating
		 * Saturday.
		 */
		SATURDAY = 7
	};
	
	enum {
		/**
		 * Value of the <code>MONTH</code> field indicating the
		 * first month of the year.
		 */
		JANUARY = 0,
		/**
		 * Value of the <code>MONTH</code> field indicating the
		 * second month of the year.
		 */
		FEBRUARY = 1,
		/**
		 * Value of the <code>MONTH</code> field indicating the
		 * third month of the year.
		 */
		MARCH = 2,
		/**
		 * Value of the <code>MONTH</code> field indicating the
		 * fourth month of the year.
		 */
		APRIL = 3,
		/**
		 * Value of the <code>MONTH</code> field indicating the
		 * fifth month of the year.
		 */
		MAY = 4,
		/**
		 * Value of the <code>MONTH</code> field indicating the
		 * sixth month of the year.
		 */
		JUNE = 5,
		/**
		 * Value of the <code>MONTH</code> field indicating the
		 * seventh month of the year.
		 */
		JULY = 6,
		/**
		 * Value of the <code>MONTH</code> field indicating the
		 * eighth month of the year.
		 */
		AUGUST = 7,
		/**
		 * Value of the <code>MONTH</code> field indicating the
		 * ninth month of the year.
		 */
		SEPTEMBER = 8,
		/**
		 * Value of the <code>MONTH</code> field indicating the
		 * tenth month of the year.
		 */
		OCTOBER = 9,
		/**
		 * Value of the <code>MONTH</code> field indicating the
		 * eleventh month of the year.
		 */
		NOVEMBER = 10,
		/**
		 * Value of the <code>MONTH</code> field indicating the
		 * twelfth month of the year.
		 */
		DECEMBER = 11
	};
	
	enum {
		/**
		 * Value of the <code>AM_PM</code> field indicating the
		 * period of the day from midnight to just before noon.
		 */
		AM = 0,
		/**
		 * Value of the <code>AM_PM</code> field indicating the
		 * period of the day from noon to just before midnight.
		 */
		PM = 1
	};
	
public:
	virtual ~ECalendar();

	ECalendar();
	ECalendar(llong millis);
	ECalendar(llong millis, int timezone);
	
public:
	/**
	 * Gets this Calendar's current time as a long expressed in milliseconds
	 * after January 1, 1970, 0:00:00 GMT (the epoch).
	 *
	 * @return the current time as UTC milliseconds from the epoch.
	 *
	 * @see #setTimeInMillis
	 */
	llong getTimeInMillis();

	/**
	 * Sets this Calendar's current time from the given long value.
	 * @param millis the new time in UTC milliseconds from the epoch.
	 *
	 * @see #getTimeInMillis
	 */
	void setTimeInMillis(llong millis);

	/**
	 * Returns a <code>Date</code> object representing this
	 * <code>Calendar</code>'s time value (millisecond offset from the <a
	 * href="#Epoch">Epoch</a>").
	 *
	 * @return a <code>Date</code> representing the time value.
	 * @see #setTime(Date)
	 * @see #getTimeInMillis()
	 */
	EDate getTime();

	/**
	 * Sets this Calendar's time with the given <code>Date</code>.
	 * <p>
	 * Note: Calling <code>setTime()</code> with
	 * <code>Date(Long.MAX_VALUE)</code> or <code>Date(Long.MIN_VALUE)</code>
	 * may yield incorrect field values from <code>get()</code>.
	 *
	 * @param date the given Date.
	 * @see #getTime()
	 * @see #setTimeInMillis(long)
	 */
	void setTime(EDate& date);

	/**
	 * Gets the value for a given time field.
	 * @param field the given time field (either YEAR, MONTH, DATE, DAY_OF_WEEK,
	 *                                    HOUR_OF_DAY, HOUR, AM_PM, MINUTE,
	 *                                    SECOND, or MILLISECOND
	 * @return the value for the given time field.
	 * @exception ArrayIndexOutOfBoundsException if the parameter is not
	 * one of the above.
	 */
	int get(int field);

	/**
	 * Sets the time field with the given value.
	 *
	 * @param field the given time field.
	 * @param value the value to be set for the given time field.
	 *
	 * @exception ArrayIndexOutOfBoundsException if an illegal field
	 * parameter is received.
	 */
	void set(int field, int value);

	/**
	 * Sets the values for the fields <code>YEAR</code>, <code>MONTH</code>,
	 * <code>DAY_OF_MONTH</code>, <code>HOUR</code>, <code>MINUTE</code>, and
	 * <code>SECOND</code>.
	 * Previous values of other fields are retained.  If this is not desired,
	 * call {@link #clear()} first.
	 *
	 * @param year the value used to set the <code>YEAR</code> calendar field.
	 * @param month the value used to set the <code>MONTH</code> calendar field.
	 * Month value is 0-based. e.g., 0 for January.
	 * @param date the value used to set the <code>DAY_OF_MONTH</code> calendar field.
	 * @param hourOfDay the value used to set the <code>HOUR_OF_DAY</code> calendar field.
	 * @param minute the value used to set the <code>MINUTE</code> calendar field.
	 * @param second the value used to set the <code>SECOND</code> calendar field.
	 * @see #set(int,int)
	 * @see #set(int,int,int)
	 * @see #set(int,int,int,int,int)
	 */
	void set(int year, int month, int date, int hourOfDay = 0, int minute = 0,
			int second = 0);

	/**
	 * Sets all the calendar field values and the time value
	 * (millisecond offset from the <a href="#Epoch">Epoch</a>) of
	 * this <code>Calendar</code> undefined. This means that {@link
	 * #isSet(int) isSet()} will return <code>false</code> for all the
	 * calendar fields, and the date and time calculations will treat
	 * the fields as if they had never been set. A
	 * <code>Calendar</code> implementation class may use its specific
	 * default field values for date/time calculations. For example,
	 * <code>GregorianCalendar</code> uses 1970 if the
	 * <code>YEAR</code> field value is undefined.
	 *
	 * @see #clear(int)
	 */
	void clear();

	/**
	 * Sets the given calendar field value and the time value
	 * (millisecond offset from the <a href="#Epoch">Epoch</a>) of
	 * this <code>Calendar</code> undefined. This means that {@link
	 * #isSet(int) isSet(field)} will return <code>false</code>, and
	 * the date and time calculations will treat the field as if it
	 * had never been set. A <code>Calendar</code> implementation
	 * class may use the field's specific default value for date and
	 * time calculations.
	 *
	 * <p>The {@link #HOUR_OF_DAY}, {@link #HOUR} and {@link #AM_PM}
	 * fields are handled independently and the <a
	 * href="#time_resolution">the resolution rule for the time of
	 * day</a> is applied. Clearing one of the fields doesn't reset
	 * the hour of day value of this <code>Calendar</code>. Use {@link
	 * #set(int,int) set(Calendar.HOUR_OF_DAY, 0)} to reset the hour
	 * value.
	 *
	 * @param field the calendar field to be cleared.
	 * @see #clear()
	 */
	void clear(int field);

	/**
	 * Sets the time zone with the given time zone value.
	 * @param value the given time zone.
	 *
	 * @see #getTimeZone
	 */
	void setTimeZone(int value);

	/**
	 * Gets the time zone.
	 * @return the time zone object associated with this calendar.
	 *
	 * @see #setTimeZone
	 */
	int getTimeZone();

	/**
	 * Gets the first day of the week for this {@code Calendar}.
	 *
	 * @return the first day of the week.
	 */
	int getFirstDayOfWeek();

	/**
	 * Gets the minimal days in the first week of the year.
	 *
	 * @return the minimal days in the first week of the year.
	 */
	int getMinimalDaysInFirstWeek();

	/**
	 * Determines if the given year is a leap year. Returns true if the
	 * given year is a leap year.
	 * @param year the given year.
	 * @return true if the given year is a leap year; false otherwise.
	 */
	boolean isLeapYear(int year);

	/**
	 * Compares this calendar to the specified object.
	 * The result is <code>true</code> if and only if the argument is
	 * not <code>null</code> and is a <code>Calendar</code> object that
	 * represents the same calendar as this object.
	 * @param obj the object to compare with.
	 * @return <code>true</code> if the objects are the same;
	 * <code>false</code> otherwise.
	 */
	boolean equals(ECalendar& that);

	/**
	 * Compares the time field records.
	 * Equivalent to comparing result of conversion to UTC.
	 * @param when the Calendar to be compared with this Calendar.
	 * @return true if the current time of this Calendar is before
	 * the time of Calendar when; false otherwise.
	 */
	boolean before(ECalendar& when);

	/**
	 * Compares the time field records.
	 * Equivalent to comparing result of conversion to UTC.
	 * @param when the Calendar to be compared with this Calendar.
	 * @return true if the current time of this Calendar is after
	 * the time of Calendar when; false otherwise.
	 */
	boolean after(ECalendar& when);

	/**
	 * Adds or subtracts the specified amount of time to the given calendar field,
	 * based on the calendar's rules. For example, to subtract 5 days from
	 * the current time of the calendar, you can achieve it by calling:
	 * <p><code>add(Calendar.DAY_OF_MONTH, -5)</code>.
	 *
	 * @param field the calendar field.
	 * @param amount the amount of date or time to be added to the field.
	 * @see #roll(int,int)
	 * @see #set(int,int)
	 */
	void add(int field, int amount);

	/**
	 * Returns a hash code for this calendar.
	 *
	 * @return a hash code value for this object.
	 * @since 1.2
	 */
	virtual int hashCode();

	/**
	 * Compares this <code>Calendar</code> to the specified
	 * <code>Object</code>.  The result is <code>true</code> if and only if
	 * the argument is a <code>Calendar</code> object of the same calendar
	 * system that represents the same time value (millisecond offset from the
	 * <a href="#Epoch">Epoch</a>) under the same
	 * <code>Calendar</code> parameters as this object.
	 *
	 * <p>The <code>Calendar</code> parameters are the values represented
	 * by the <code>isLenient</code>, <code>getFirstDayOfWeek</code>,
	 * <code>getMinimalDaysInFirstWeek</code> and <code>getTimeZone</code>
	 * methods. If there is any difference in those parameters
	 * between the two <code>Calendar</code>s, this method returns
	 * <code>false</code>.
	 *
	 * <p>Use the {@link #compareTo(Calendar) compareTo} method to
	 * compare only the time values.
	 *
	 * @param obj the object to compare with.
	 * @return <code>true</code> if this object is equal to <code>obj</code>;
	 * <code>false</code> otherwise.
	 */
	boolean equals(ECalendar *obj);
	virtual boolean equals(EObject* obj);

	/**
	 * Return a string representation of this calendar. This method
	 * is intended to be used only for debugging purposes, and the
	 * format of the returned string may vary between implementations.
	 * The returned string may be empty but may not be <code>null</code>.
	 *
	 * @return  a string representation of this calendar.
	 */
	virtual EStringBase toString();

	/**
	 * Return a string representation of this calendar.
	 * @param   format   a string to be parsed as a date.
	 * @format:
	 *	%a     locale¡¯s abbreviated weekday name (e.g., Sun)
	 *	%b     locale¡¯s abbreviated month name (e.g., Jan)
	 *	%d     day of month (01..31)
	 *	%H     hour (00..23)
	 *	%I     hour (01..12)
	 *	%m     month (01..12)
	 *	%M     minute (00..59)
	 *	%S     second (00..60)
	 *	%s     millisecond (000..999)
	 *	%p     locale¡¯s equivalent of either AM or PM; blank if not known
	 *	%P     like %p, but lower case
	 *	%u     day of week (1..7); 1 is Monday
	 *	%w     day of week (0..6); 0 is Sunday
	 *	%Y     year
	 *	%y     last two digits of year (00-99)
	 *	%z     +hhmm numeric timezone (e.g., -0400)
	 */
	EString toString(const char *format);

	/**
	 * Converts this <code>Date</code> object to a <code>String</code>
	 * of the form:
	 * <blockquote><pre>
	 * dow mon dd hh:mm:ss zzz yyyy</pre></blockquote>
	 * where:<ul>
	 * <li><tt>dow</tt> is the day of the week (<tt>Sun, Mon, Tue, Wed,
	 *     Thu, Fri, Sat</tt>).
	 * <li><tt>mon</tt> is the month (<tt>Jan, Feb, Mar, Apr, May, Jun,
	 *     Jul, Aug, Sep, Oct, Nov, Dec</tt>).
	 * <li><tt>dd</tt> is the day of the month (<tt>01</tt> through
	 *     <tt>31</tt>), as two decimal digits.
	 * <li><tt>hh</tt> is the hour of the day (<tt>00</tt> through
	 *     <tt>23</tt>), as two decimal digits.
	 * <li><tt>mm</tt> is the minute within the hour (<tt>00</tt> through
	 *     <tt>59</tt>), as two decimal digits.
	 * <li><tt>ss</tt> is the second within the minute (<tt>00</tt> through
	 *     <tt>61</tt>, as two decimal digits.
	 * <li><tt>zzz</tt> is the time zone (and may reflect daylight savings
	 *     time). If time zone information is not available,
	 *     then <tt>zzz</tt> is empty - that is, it consists
	 *     of no characters at all.
	 * <li><tt>yyyy</tt> is the year, as four decimal digits.
	 * </ul>
	 *
	 * @return  a string representation of this date.
	 */
	static EString toString(ECalendar* calendar);

	/**
	 * Converts this <code>Date</code> object to a <code>String</code>.
	 * The output format is as follows:
	 * <blockquote><pre>yyyy MM dd hh mm ss +zzzz</pre></blockquote>
	 * where:<ul>
	 * <li><dd>yyyy</dd> is the year, as four decimal digits.
	 *    Year values larger than <dd>9999</dd> will be truncated
	 *    to <dd>9999</dd>.
	 * <li><dd>MM</dd> is the month (<dd>01</dd> through <dd>12</dd>),
	 *    as two decimal digits.
	 * <li><dd>dd</dd> is the day of the month (<dd>01</dd> through
	 *    <dd>31</dd>), as two decimal digits.
	 * <li><dd>hh</dd> is the hour of the day (<dd>00</dd> through
	 *    <dd>23</dd>), as two decimal digits.
	 * <li><dd>mm</dd> is the minute within the hour (<dd>00</dd>
	 *     through <dd>59</dd>), as two decimal digits.
	 * <li><dd>ss</dd> is the second within the minute (<dd>00</dd>
	 *    through <dd>59</dd>), as two decimal digits.
	 * <li><dd>zzzz</dd> is the time zone offset in hours and minutes
	 *      (four decimal digits <dd>"hhmm"</dd>) relative to GMT,
	 *      preceded by a "+" or "-" character (<dd>-1200</dd>
	 *      through <dd>+1200</dd>).
	 *      For instance, Pacific Standard Time zone is printed
	 *      as <dd>-0800</dd>.  GMT is printed as <dd>+0000</dd>.
	 * </ul>
	 *
	 * @return  a string representation of this date.
	 */
	static EString toISO8601String(ECalendar* calendar);

	/**
	 * Attempts to interpret the string <tt>s</tt> as a representation
	 * of a date and time.
	 *
	 * @param   s   a string to be parsed as a date.
	 * @return  the Date representing formated date string.
	 * @see     java.text.DateFormat
	 * @format:
	 *	%a     locale¡¯s abbreviated weekday name (e.g., Sun)
	 *	%b     locale¡¯s abbreviated month name (e.g., Jan)
	 *	%d     day of month (01..31)
	 *	%H     hour (00..23)
	 *	%I     hour (01..12)
	 *	%m     month (01..12)
	 *	%M     minute (00..59)
	 *	%S     second (00..60)
	 *	%s     millisecond (000..999)
	 *	%p     locale¡¯s equivalent of either AM or PM; blank if not known
	 *	%P     like %p, but lower case
	 *	%u     day of week (1..7); 1 is Monday
	 *	%w     day of week (0..6); 0 is Sunday
	 *	%Y     year
	 *	%y     last two digits of year (00-99)
	 *	%z     +hhmm numeric timezone (e.g., -0400)
	 */
	static ECalendar parseOf(const char *format, const char *source)
			THROWS(EIllegalArgumentException);

protected:
	/**
	 * Converts UTC as milliseconds to time field values.
	 */
	void computeFields();

	/**
	 * Convert the time as milliseconds to the date fields.  Millis must be
	 * given as local wall millis to get the correct local day.  For example,
	 * if it is 11:30 pm Standard, and DST is in effect, the correct DST millis
	 * must be passed in to get the right date.
	 * <p>
	 * Fields that are completed by this method: YEAR, MONTH, DATE, DAY_OF_WEEK.
	 * @param theTime the time in wall millis (either Standard or DST),
	 * whichever is in effect
	 */
	void timeToFields(llong theTime);

	/**
	 * Converts time field values to UTC as milliseconds.
	 * @exception IllegalArgumentException if any fields are invalid.
	 */
	void computeTime();
	
private:
	int fields[FIELDS];
	boolean _isSet[FIELDS];
	boolean isTimeSet; // NOTE: Make transient when possible
	/**
	 * The currently set time for this calendar, expressed in milliseconds after
	 * January 1, 1970, 0:00:00 GMT.
	 */
	llong time;
	int timezone;

	/**
	 * Compute the Julian day number under either the Gregorian or the
	 * Julian calendar, using the given year and the remaining fields.
	 * @param isGregorian if true, use the Gregorian calendar
	 * @param year the adjusted year number, with 0 indicating the
	 * year 1 BC, -1 indicating 2 BC, etc.
	 * @return the Julian day number
	 */
	llong calculateJulianDay(boolean isGregorian, int year);

	/**
	 * Validates the field values for HOUR_OF_DAY, AM_PM and HOUR
	 * The calendar will give preference in the following order
	 * HOUR_OF_DAY, AM_PM, HOUR
	 */
	void correctTime();

	/**
	 *
	 */
	int yearLength();
	int yearLength(int year);

	/**
	 * Return the week number of a day, within a period. This may be the week number in
	 * a year, or the week number in a month. Usually this will be a value >= 1, but if
	 * some initial days of the period are excluded from week 1, because
	 * minimalDaysInFirstWeek is > 1, then the week number will be zero for those
	 * initial days. Requires the day of week for the given date in order to determine
	 * the day of week of the first day of the period.
	 *
	 * @param dayOfPeriod  Day-of-year or day-of-month. Should be 1 for first day of period.
	 * @param day   Day-of-week for given dayOfPeriod. 1-based with 1=Sunday.
	 * @return      Week number, one-based, or zero if the day falls in part of the
	 *              month before the first week, when there are days before the first
	 *              week because the minimum days in the first week is more than one.
	 */
	int weekNumber(int dayOfPeriod, int dayOfWeek);

	/**
	 * Converts time as milliseconds to Julian day.
	 * @param millis the given milliseconds.
	 * @return the Julian day number.
	 */
	static llong millisToJulianDay(llong millis);

	/**
	 * Converts Julian day to time as milliseconds.
	 * @param julian the given Julian day number.
	 * @return time as milliseconds.
	 */
	static llong julianDayToMillis(llong julian);

	static int julianDayToDayOfWeek(llong julian);

	/**
	 * Divide two long integers, returning the floor of the quotient.
	 * <p>
	 * Unlike the built-in division, this is mathematically well-behaved.
	 * E.g., <code>-1/4</code> => 0
	 * but <code>floorDivide(-1,4)</code> => -1.
	 * @param numerator the numerator
	 * @param denominator a divisor which must be > 0
	 * @return the floor of the quotient.
	 */
	static llong floorDivide(llong numerator, llong denominator);

	/**
	 * Divide two integers, returning the floor of the quotient, and
	 * the modulus remainder.
	 * <p>
	 * Unlike the built-in division, this is mathematically well-behaved.
	 * E.g., <code>-1/4</code> => 0 and <code>-1%4</code> => -1,
	 * but <code>floorDivide(-1,4)</code> => -1 with <code>remainder[0]</code> => 3.
	 * @param numerator the numerator
	 * @param denominator a divisor which must be > 0
	 * @param remainder an array of at least one element in which the value
	 * <code>numerator mod denominator</code> is returned. Unlike <code>numerator
	 * % denominator</code>, this will always be non-negative.
	 * @return the floor of the quotient.
	 */
	static int floorDivide(int numerator, int denominator, int *remainder);
};

} /* namespace efc */
#endif //!__ECalendar_H__
