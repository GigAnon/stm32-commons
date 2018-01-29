#include "datetime.h"

#include <cstdio>

DateTime::Time::Time(uint8_t _h, uint8_t _m, uint8_t _s):s(_s),m(_m),h(_h)
{
    m += (s/60);
    s %= 60;
    h += (m/60);
    m %= 60;
}

DateTime::Time& DateTime::Time::operator=(const Time& t)
{
    s = t.s;
    m = t.m;
    h = t.h;
    return *this;
}

DateTime::Time DateTime::Time::fromSeconds(uint32_t s)
{
    Time t;
    t.s = s%60;

    s /= 60;
    t.m = s;

    s /= 60;
    t.h = s;

    return t;
}

DateTime::Time::Time(const Time& t)
: Time(t.h, t.m, t.s)
{
}

DateTime::Date::Date(uint8_t _d, uint8_t _m, uint16_t _y)
    :d(_d),m(_m),y(_y)
 { }

DateTime::Date::Date(const Date& other)
    : Date(other.d, other.m, other.y)
{ }

DateTime::DateTime(uint32_t epoch): m_epoch(epoch)
{}

DateTime::DateTime(const Date& date, const Time& time): DateTime(computeEpochFromDateTime(date, time))
{}

bool DateTime::operator==(const DateTime& other) const { return other.m_epoch == m_epoch; }
bool DateTime::operator!=(const DateTime& other) const { return !(*this == other); }
bool DateTime::operator<(const DateTime& other)  const { return other.m_epoch <  m_epoch; }
bool DateTime::operator>(const DateTime& other)  const { return other.m_epoch >  m_epoch; }
bool DateTime::operator<=(const DateTime& other) const { return !(*this > other); }
bool DateTime::operator>=(const DateTime& other) const { return !(*this < other); }

DateTime DateTime::operator+(const DateTime& other) const       { return DateTime(m_epoch+other.m_epoch); }
DateTime DateTime::operator+(const DateTime::Date& date) const  { return DateTime(m_epoch+computeEpochFromDateTime(date)); }
DateTime DateTime::operator+(const DateTime::Time& time) const  { return DateTime(m_epoch+computeEpochFromDateTime(Date(), time)); }

DateTime& DateTime::operator+=(const DateTime& other)        {  return (*this = *this + other); }
DateTime& DateTime::operator+=(const DateTime::Date& date)   {  return (*this = *this + date);  }
DateTime& DateTime::operator+=(const DateTime::Time& time)   {  return (*this = *this + time);  }

DateTime::Date DateTime::getDate() const
{
    return computeDateFromEpoch(m_epoch);
}

DateTime::Time DateTime::getTime() const
{
    return computeTimeFromEpoch(m_epoch);
}

DateTime::Date DateTime::computeDateFromEpoch(uint32_t e)
{
    Date d(1, 1, 1970);

    e /= (60*60*24);

    while(true)
    {
        uint16_t days = isLeapYear(d.y)?366:365;

        if(e >= days)
            e -= days;
        else
            break;

        ++d.y;
    }

    for(d.m = 1; d.m < 12; ++d.m)
    {
        uint8_t days = daysInMonth(d.m, d.y);
        if(e >= days)
            e -= days;
        else
            break;
    }

    d.d = e + 1;

    return d;
}

DateTime::Time DateTime::computeTimeFromEpoch(uint32_t e)
{
    Time t;

    t.s = e%60;

    e /= 60;
    t.m = e%60;

    e /= 60;
    t.h = e%24;
    
    return t;
}

uint32_t DateTime::computeEpochFromDateTime(Date date, Time time)
{
    uint32_t days = 0;

    while(date.y > 1970)
        days += (isLeapYear(date.y--)?366:365);

    while(date.m > 1)
        days += daysInMonth(--date.m);

    days += (date.d-1);

    return days*86400 + (uint32_t)(time.h)*3600 
                      + (uint32_t)(time.m)*60
                      + (uint32_t)(time.s);
}

bool DateTime::isLeapYear(uint16_t y)
{
    if(y%4)     return false;
    if(y%100)   return true;
    if(y%400)   return false;
    
    return true;
}

uint8_t DateTime::daysInMonth(uint8_t month, uint16_t year)
{
    switch(month)
    {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            return 31;
        case 4:
        case 6:
        case 9:
        case 11:
            return 30;
        case 2:
            return isLeapYear(year)?29:28;
        default:
            break;
    }
    return 0;
}
