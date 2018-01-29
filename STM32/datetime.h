#ifndef GUARD_DATETIME
#define GUARD_DATETIME

#include <cstdint>

/**
    \brief Basic date and time class.

    DateTime is a basic date and time class. It can store an Unix uint32 timestamp
    and convert it into a year-month-day hour-minute-seconds format.
    \remark This class uses UNSIGNED 32-bits timestamps. Unlike real Unix time format,
    dates before 01/01/1970 are invalid and may cause undefined behavior. On the other hand,
    this class will not be affected by the Y2038 overflow, since its timestamp will overflow
    around 2106.
**/
class DateTime
{
    public:

	/**
	\brief Time structure.
	**/
    struct Time
    {
		/**
			\brief Constructor
			\param _h Hours 
			\param _m Minutes (0-60)
			\param _s Seconds (0-60)
		**/
        Time(uint8_t _h=0, uint8_t _m=0, uint8_t _s=0);
        Time(const Time& t);

        Time& operator=(const Time& t);

        static Time fromSeconds(uint32_t s);

        uint8_t s; ///< Seconds
        uint8_t m; ///< Minutes
        uint8_t h; ///< Hours
    };

    /**
    \brief Data structure
    **/
    struct Date
    {
        /**
            \brief Constructor
            \param _d Days
            \param _m Months
            \param _y Years
        **/
        Date(uint8_t _d=0, uint8_t _m=0, uint16_t _y=0);
        Date(const Date& other);

        uint8_t d;  ///< Days
        uint8_t m;  ///< Months
        uint16_t y; ///< Years
    };

    public:
        /**
            \brief Constructor
            \param epoch Seconds since epoch
        **/
        DateTime(uint32_t epoch = 0);

        DateTime(const Date& date, const Time& time = Time());

        bool operator==(const DateTime& other) const;
        bool operator!=(const DateTime& other) const;
        bool operator< (const DateTime& other) const;
        bool operator> (const DateTime& other) const;
        bool operator<=(const DateTime& other) const;
        bool operator>=(const DateTime& other) const;

        DateTime operator+(const DateTime& other) const;
        DateTime operator+(const DateTime::Date& date) const;
        DateTime operator+(const DateTime::Time& time) const;

        DateTime& operator+=(const DateTime& other);
        DateTime& operator+=(const DateTime::Date& date);
        DateTime& operator+=(const DateTime::Time& time);

        /**
            \returns The date converted from Unix time
            \remark GMT time only
        **/
        Date getDate() const;

        /**
            \returns The time of day converted from Unix time
            \remark GMT time only
        **/
        Time getTime() const;

        /**
            \brief Compute a date from an Unix timestamp
            \param e Seconds since epoch
            \returns Date
        **/
        static Date computeDateFromEpoch(uint32_t e);

        /**
            \brief Compute a time of day from an Unix timestamp
            \param e Seconds since epoch
            \returns Time of day
        **/
        static Time computeTimeFromEpoch(uint32_t e);

        /**
            \brief Compute an Unix timestamp from a date and a time
            \param date Date
            \param 
        **/
        static uint32_t computeEpochFromDateTime(Date date, Time time = Time());

        /**
            \param y Year to check
            \returns \c true if year is a leap year, \c false otherwise
        **/
        static bool isLeapYear(uint16_t y);

        /**
            \param month Month to check
            \param year Year to use (for leap years)
            \returns The number of days in the month (inc. leap years), or 0 if month is invalid
        **/
        static uint8_t daysInMonth(uint8_t month, uint16_t year = 1970);

    private:
        uint32_t m_epoch;
};

 #endif
