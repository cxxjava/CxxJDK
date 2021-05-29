/*
 * EDate.cpp
 *
 *  Created on: 2013-3-19
 *      Author: cxxjava@163.com
 */

#include "EDate.hh"
#include "ESystem.hh"
#include "ECalendar.hh"

namespace efc {

EDate::~EDate() {
	delete calendar;
}

EDate::EDate() {
	calendar = new ECalendar();
	calendar->setTimeInMillis(ESystem::currentTimeMillis());
}

EDate::EDate(llong date)
{
	calendar = new ECalendar();
	calendar->setTimeInMillis(date);
}

EDate::EDate(const EDate& that) {
	EDate* t = (EDate*)&that;
	calendar = new ECalendar();
	calendar->setTimeInMillis(t->getTime());
}

EDate& EDate::operator= (const EDate& that) {
	if (this == &that) return *this;
	EDate* t = (EDate*)&that;
	delete calendar;
	calendar = new ECalendar();
	calendar->setTimeInMillis(t->getTime());
	return *this;
}

llong EDate::getTime()
{
	return calendar->getTimeInMillis();
}

void EDate::setTime(llong time)
{
	calendar->setTimeInMillis(time);

}

boolean EDate::before(EDate& when)
{
	return (getTime() < when.getTime());
}

boolean EDate::after(EDate& when)
{
	return (getTime() > when.getTime());
}

boolean EDate::equals(EDate& that)
{
	return (getTime() == that.getTime());
}

EString EDate::toString()
{
	return ECalendar::toString(calendar);
}

EString EDate::toString(const char *format) {
	return calendar->toString(format);
}

boolean EDate::equals(EDate* obj)
 {
	if (this == obj) {
		return true;
	}
	return getTime() == obj->getTime();
}

int  EDate::hashCode() {
	llong ht = this->getTime();
    return (int) ht ^ (int) (ht >> 32);
}

llong EDate::parse(const char *format, const char *source) THROWS(EIllegalArgumentException)
{
	int tz;
	es_uint32_t tm = eso_parse_time(&tz, format, source);
	if (tm == 0) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Parse formated datetime string failed");
	}
	return (llong(tm) + tz * ES_SEC_PER_HOUR) * llong(1000);
}

} /* namespace efc */
