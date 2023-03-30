// (C)2005 S2 Games
// c_date.h
//
//=============================================================================
#ifndef __C_DATE_H__
#define __C_DATE_H__

//=============================================================================
// Definitions
//=============================================================================
enum EMonth
{
	JANUARY = 1,
	FEBRUARY,
	MARCH,
	APRIL,
	MAY,
	JUNE,
	JULY,
	AUGUST,
	SEPTEMBER,
	OCTOBER,
	NOVEMBER,
	DECEMBER
};

enum EWeekday
{
	SUNDAY = 0,
	MONDAY,
	TUESDAY,
	WEDNESDAY,
	THURSDAY,
	FRIDAY,
	SATURDAY
};

const uint DATE_SHORT_YEAR	(BIT(0));
const uint DATE_YEAR_LAST	(BIT(1));
const uint DATE_MONTH_FIRST	(BIT(2));

const uint TIME_NO_SECONDS	(BIT(0));
const uint TIME_TWELVE_HOUR	(BIT(1));

const uint WEEKDAY_SHORT	(BIT(0));
//=============================================================================

//=============================================================================
// CDate
//=============================================================================
class K2_API CDate
{
private:
	int		m_iDay, m_iMonth, m_iYear, m_iWeekday;
	int		m_iHour, m_iMinute, m_iSecond;

public:
	CDate() : m_iYear(-1), m_iHour(-1)	{}

	CDate(bool b);

	CDate(int iYear, int iMonth, int iDay, int iHour = -1, int iMinute = 0, int iSecond = 0, int iWeekday = 0) :
	m_iDay(iDay), m_iMonth(iMonth), m_iYear(iYear), m_iWeekday(iWeekday), 
	m_iHour(iHour), m_iMinute(iMinute), m_iSecond(iSecond)
	{}

	CDate(time_t t);

	bool	IsDateValid() const	{ return m_iYear != -1; };
	bool	IsTimeValid() const	{ return m_iHour != -1; };

	int		GetYear() const		{ return m_iYear; }
	int		GetMonth() const	{ return m_iMonth; }
	int		GetDay() const		{ return m_iDay; }
	int		GetHour() const		{ return m_iHour; }
	int		GetMinute() const	{ return m_iMinute; }
	int		GetSecond() const	{ return m_iSecond; }
	int		GetWeekday() const	{ return m_iWeekday; }

	void	SetYear(int iYear)		{ m_iYear = iYear; }
	void	SetMonth(int iMonth)	{ m_iMonth = iMonth; }
	void	SetDay(int iDay)		{ m_iDay = iDay; }
	void	SetHour(int iHour)		{ m_iHour = iHour; }
	void	SetMinute(int iMinute)	{ m_iMinute = iMinute; }
	void	SetSecond(int iSecond)	{ m_iSecond = iSecond; }

	void	Set(int iYear, int iMonth, int iDay, int iHour, int iMinute, int iSecond)
	{
		m_iYear = iYear; m_iMonth = iMonth; m_iDay = iDay;
		m_iHour = iHour; m_iMinute = iMinute; m_iSecond = iSecond;
	}

	bool	IsDay(EMonth eMonth, int iDay);
	bool	IsDay(int iYear, EMonth eMonth, int iDay);

	tstring	GetDateString(uint uiFlags = 0, TCHAR cSeperator = '/');
	tstring GetTimeString(uint uiFlags = 0, TCHAR cSeperator = ':');
	tstring GetWeekdayString(uint uiFlags = 0);
	tstring	GetString();

	bool	operator==(const CDate &B);
	bool	operator>(const CDate &B);
	bool	operator<(const CDate &B);
	bool	operator>=(const CDate &B);
	bool	operator<=(const CDate &B);

	CDate	operator+(const CDate &B);
	CDate	operator-(const CDate &B);

	CDate&	operator+=(const CDate &B);
	CDate&	operator-=(const CDate &B);
};
//=============================================================================
#endif //__C_DATE_H__
