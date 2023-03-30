// (C)2005 S2 Games
// xtoa.h
//
//=============================================================================
#ifndef __XTOA_H__
#define __XTOA_H__

//=============================================================================
// Headers
//=============================================================================
#include "stringutils.h"
#include "c_system.h"
#include "c_profilemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EXtoAFormatFlags
{
	FMT_NONE =		0x0000,
	FMT_SIGN =		0x0001,
	FMT_PADZERO =	0x0002,
	FMT_ALIGNLEFT =	0x0004,
	FMT_PADSIGN =	0x0008,
	FMT_NOPREFIX =	0x0010,
	FMT_DELIMIT =	0x0020,
	FMT_LOWERCASE = 0x0040
};

const uint FMT_TIME_ALPHA		(0x0080);
const uint FMT_TIME_COUNTDOWN	(0x0100);

const int XTOA_DEFAULT_FLOAT_PRECISION(4);

#define BYTE_HEX_TSTR(y)	XtoA((y), FMT_PADZERO, 4, 16)
#define SHORT_HEX_TSTR(n)	XtoA((n), FMT_PADZERO, 6, 16)
#define INT_HEX_TSTR(i)		XtoA((i), FMT_PADZERO, 10, 16)

#define BYTE_HEX_WSTR(y)	XtoW((y), FMT_PADZERO, 4, 16)
#define SHORT_HEX_WSTR(n)	XtoW((n), FMT_PADZERO, 6, 16)
#define INT_HEX_WSTR(i)		XtoW((i), FMT_PADZERO, 10, 16)

#define BYTE_HEX_STR(y)		XtoS((y), FMT_PADZERO, 4, 16)
#define SHORT_HEX_STR(n)	XtoS((n), FMT_PADZERO, 6, 16)
#define INT_HEX_STR(i)		XtoS((i), FMT_PADZERO, 10, 16)

enum ETimeFormatString
{
	TIME_STRING_SEPERATOR,
	TIME_STRING_SECONDS,
	TIME_STRING_MINUTES,
	TIME_STRING_HOURS,
	TIME_STRING_DAYS
};

const string TIME_STRINGS[] = { ":", "s ", "m ", "h ", "d " };
const wstring TIME_STRINGW[] = { L":", L"s ", L"m ", L"h ", L"d " };

template<class T> const T&			GetTimeString(ETimeFormatString e);
template<> inline const string&		GetTimeString<string>(ETimeFormatString e)	{ return TIME_STRINGS[e]; }
template<> inline const wstring&	GetTimeString<wstring>(ETimeFormatString e)	{ return TIME_STRINGW[e]; }

#ifndef _WIN32
#define _wtoi(x) wcstol(x, NULL, 10)
#endif

#if defined(_WIN32)
inline double WTOF(const wstring &s) { return _wtof(s.c_str()); }
#elif defined(__APPLE__)
inline double WTOF(const wstring &s) { return atof(TStringToNative(s).c_str()); }
#elif defined(linux)
inline double WTOF(const wstring &s) { return wcstod(s.c_str(), NULL); }
#endif
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
// Integers
K2_API void	FormatInt(ULONGLONG ull, int flags, size_t width, int base, wchar_t sign, wstring &sStr);
K2_API void	FormatInt(ULONGLONG ull, int flags, size_t width, int base, char sign, string &sStr);

template <class T>
wstring	_ItoW(T i, int flags, int width, int base)
{
	wstring sReturn;
	FormatInt(ABS(i), flags, width, base, (i < 0) ? L'-' : 0, sReturn);
	return sReturn;
}

template <class T>
string	_ItoS(T i, int flags, int width, int base)
{
	string sReturn;
	FormatInt(ABS(i), flags, width, base, (i < 0) ? '-' : 0, sReturn);
	return sReturn;
}

template <class T>
wstring	_UItoW(T ui, int flags, int width, int base)
{
	wstring sReturn;
	FormatInt(ui, flags, width, base, 0, sReturn);
	return sReturn;
}

template <class T>
string	_UItoS(T ui, int flags, int width, int base)
{
	string sReturn;
	FormatInt(ui, flags, width, base, 0, sReturn);
	return sReturn;
}

inline wstring	XtoW(short n, int flags = 0, int width = 0, int base = 10)				{ return _ItoW(n, flags, width, base); }
inline wstring	XtoW(int i, int flags = 0, int width = 0, int base = 10)				{ return _ItoW(i, flags, width, base); }
inline wstring	XtoW(long l, int flags = 0, int width = 0, int base = 10)				{ return _ItoW(l, flags, width, base); }
inline wstring	XtoW(LONGLONG ll, int flags = 0, int width = 0, int base = 10)			{ return _ItoW(ll, flags, width, base); }
inline wstring	XtoW(byte y, int flags = 0, int width = 0, int base = 10)				{ return _UItoW(y, flags, width, base); }
inline wstring	XtoW(unsigned short un, int flags = 0, int width = 0, int base = 10)	{ return _UItoW(un, flags, width, base); }
inline wstring	XtoW(unsigned int ui, int flags = 0, int width = 0, int base = 10)		{ return _UItoW(ui, flags, width, base); }
inline wstring	XtoW(unsigned long ul, int flags = 0, int width = 0, int base = 10)		{ return _UItoW(ul, flags, width, base); }
inline wstring	XtoW(ULONGLONG ull, int flags = 0, int width = 0, int base = 10)		{ return _UItoW(ull, flags, width, base); }

inline string	XtoS(short n, int flags = 0, int width = 0, int base = 10)				{ return _ItoS(n, flags, width, base); }
inline string	XtoS(int i, int flags = 0, int width = 0, int base = 10)				{ return _ItoS(i, flags, width, base); }
inline string	XtoS(long l, int flags = 0, int width = 0, int base = 10)				{ return _ItoS(l, flags, width, base); }
inline string	XtoS(LONGLONG ll, int flags = 0, int width = 0, int base = 10)			{ return _ItoS(ll, flags, width, base); }
inline string	XtoS(byte y, int flags = 0, int width = 0, int base = 10)				{ return _UItoS(y, flags, width, base); }
inline string	XtoS(unsigned short un, int flags = 0, int width = 0, int base = 10)	{ return _UItoS(un, flags, width, base); }
inline string	XtoS(unsigned int ui, int flags = 0, int width = 0, int base = 10)		{ return _UItoS(ui, flags, width, base); }
inline string	XtoS(unsigned long ul, int flags = 0, int width = 0, int base = 10)		{ return _UItoS(ul, flags, width, base); }
inline string	XtoS(ULONGLONG ull, int flags = 0, int width = 0, int base = 10)		{ return _UItoS(ull, flags, width, base); }

// Floating point
K2_API void	FormatFloat(double d, int flags, size_t width, int precision, string &sStr);
K2_API void	FormatFloat(double d, int flags, size_t width, int precision, wstring &sStr);
K2_API void	FormatFloat(double d, int flags, size_t width, int iMinPrecision, int iMaxPrecision, string &sStr);
K2_API void	FormatFloat(double d, int flags, size_t width, int iMinPrecision, int iMaxPrecision, wstring &sStr);

inline wstring	XtoW(float f, int flags = 0, int width = 0, int precision = XTOA_DEFAULT_FLOAT_PRECISION)	{ wstring s; FormatFloat(f, flags, width, precision, s); return s; }
inline wstring	XtoW(double d, int flags = 0, int width = 0, int precision = XTOA_DEFAULT_FLOAT_PRECISION)	{ wstring s; FormatFloat(d, flags, width, precision, s); return s; }

inline wstring	XtoW(float f, int flags, int width, int iMinPrecision, int iMaxPrecision)					{ wstring s; FormatFloat(f, flags, width, iMinPrecision, iMaxPrecision, s); return s; }
inline wstring	XtoW(double d, int flags, int width, int iMinPrecision, int iMaxPrecision)					{ wstring s; FormatFloat(d, flags, width, iMinPrecision, iMaxPrecision, s); return s; }

inline string	XtoS(float f, int flags = 0, int width = 0, int precision = XTOA_DEFAULT_FLOAT_PRECISION)	{ string s; FormatFloat(f, flags, width, precision, s); return s; }
inline string	XtoS(double d, int flags = 0, int width = 0, int precision = XTOA_DEFAULT_FLOAT_PRECISION)	{ string s; FormatFloat(d, flags, width, precision, s); return s; }

inline string	XtoS(float f, int flags, int width, int iMinPrecision, int iMaxPrecision)					{ string s; FormatFloat(f, flags, width, iMinPrecision, iMaxPrecision, s); return s; }
inline string	XtoS(double d, int flags, int width, int iMinPrecision, int iMaxPrecision)					{ string s; FormatFloat(d, flags, width, iMinPrecision, iMaxPrecision, s); return s; }

// Vectors
K2_API wstring	XtoW(const CVec2f &vec, int flags = 0, int width = 0, int precision = XTOA_DEFAULT_FLOAT_PRECISION);
K2_API wstring	XtoW(const CVec3f &vec, int flags = 0, int width = 0, int precision = XTOA_DEFAULT_FLOAT_PRECISION);
K2_API wstring	XtoW(const CVec3<double> &vec, int flags = 0, int width = 0, int precision = XTOA_DEFAULT_FLOAT_PRECISION);
K2_API wstring	XtoW(const CVec4f &vec, int flags = 0, int width = 0, int precision = XTOA_DEFAULT_FLOAT_PRECISION);

K2_API string	XtoS(const CVec2f &vec, int flags = 0, int width = 0, int precision = XTOA_DEFAULT_FLOAT_PRECISION);
K2_API string	XtoS(const CVec3f &vec, int flags = 0, int width = 0, int precision = XTOA_DEFAULT_FLOAT_PRECISION);
K2_API string	XtoS(const CVec3<double> &vec, int flags = 0, int width = 0, int precision = XTOA_DEFAULT_FLOAT_PRECISION);
K2_API string	XtoS(const CVec4f &vec, int flags = 0, int width = 0, int precision = XTOA_DEFAULT_FLOAT_PRECISION);

// Boolean
inline wstring	XtoW(bool b, bool bNum = false)	{ return (bNum ? (b ? L"1" : L"0") : (b ? L"true" : L"false")); }
inline string	XtoS(bool b, bool bNum = false)	{ return (bNum ? (b ? "1" : "0") : (b ? "true" : "false")); }

// Pointers
inline wstring	XtoW(const void *p, bool bLower = false)	{ wstring sReturn; FormatInt(reinterpret_cast<size_t>(p), FMT_PADZERO, sizeof(size_t) * 2, 16, L'\0', sReturn); return sReturn; }
inline string	XtoS(const void *p, bool bLower = false)	{ string sReturn; FormatInt(reinterpret_cast<size_t>(p), FMT_PADZERO, sizeof(size_t) * 2, 16, '\0', sReturn); return sReturn; }

// Text
K2_API wstring	XtoW(const wstring &s, int flags, size_t width = 0);
K2_API string	XtoS(const string &s, int flags, size_t width = 0);
inline const wstring&	XtoW(const wstring &s)	{ return s; }
inline const string&	XtoS(const string &s)	{ return s; }
inline wstring			XtoW(wchar_t c)			{ return wstring(1, c); }
inline string			XtoS(char c)			{ return string(1, c); }

// AtoX
inline int&		AtoX(const string &s, int &i)			{ i = atoi(s.c_str()); return i; }
inline int&		AtoX(const wstring &s, int &i)			{ i = _wtoi(s.c_str()); return i; }
inline uint&	AtoX(const string &s, uint &ui)			{ ui = strtoul(s.c_str(), NULL, 10); return ui; }
inline uint&	AtoX(const wstring &s, uint &ui)		{ ui = wcstoul(s.c_str(), NULL, 10); return ui; }
inline float&	AtoX(const string &s, float &f)			{ f = float(atof(s.c_str())); return f; }
inline float&	AtoX(const wstring &s, float &f)		{ f = float(WTOF(s)); return f; }
inline double&	AtoX(const string &s, double &d)		{ d = atof(s.c_str()); return d; }
inline double&	AtoX(const wstring &s, double &d)		{ d = WTOF(s); return d; }
inline string&	AtoX(const string &s, string &str)		{ str = s; return str; }
inline wstring&	AtoX(const wstring &s, wstring &str)	{ str = s; return str; }
inline bool&	AtoX(const string &s, bool &b)			{ if (CompareNoCase(s, "true") == 0) b = true; else b = (atof(s.c_str()) != 0.0f); return b; }
inline bool&	AtoX(const wstring &s, bool &b)			{ if (CompareNoCase(s, L"true") == 0) b = true; else b = (WTOF(s) != 0.0f);  return b; }

K2_API CVec2f&	AtoX(const tstring &s, CVec2f &vec);
K2_API CVec3f&	AtoX(const tstring &s, CVec3f &vec);
K2_API CVec4f&	AtoX(const tstring &s, CVec4f &vec);

// Non-ref versions
inline int					AtoI(const string &s)	{ return atoi(s.c_str()); }
inline int					AtoI(const wstring &s)	{ return _wtoi(s.c_str()); }
template <class T> short	AtoN(const T &s)		{ return short(AtoI(s) & USHRT_MAX); }

inline uint					AtoUI(const string &s)	{ return strtoul(s.c_str(), NULL, 10); }
inline uint					AtoUI(const wstring &s)	{ return wcstoul(s.c_str(), NULL, 10); }

inline float				AtoF(const string &s)	{ return float(atof(s.c_str())); }
inline float				AtoF(const wstring &s)	{ return float(WTOF(s)); }
inline double				AtoD(const string &s)	{ return atof(s.c_str()); }
inline double				AtoD(const wstring &s)	{ return WTOF(s); }

inline bool					AtoB(const string &s)	{ return (CompareNoCase(s, "true") == 0) ? true : (atof(s.c_str()) != 0); }
inline bool					AtoB(const wstring &s)	{ return (CompareNoCase(s, L"true") == 0) ? true : (WTOF(s) != 0); }

K2_API CVec2f	AtoV2(const tstring &s);
K2_API CVec3f	AtoV3(const tstring &s);
K2_API CVec4f	AtoV4(const tstring &s);

// Percentages
K2_API int		PtoI(const tstring &s);
K2_API float	PtoF(const tstring &s);
K2_API float	P2toF(const tstring &s);

// Time
template <class T>
T	FormatTime(uint uiTime, int iSeperations, uint uiPrecision, uint uiFlags)
{
	bool bPadZero((uiFlags & FMT_PADZERO) != 0);

	if (uiPrecision == 0)
	{
		uint uiMs(uiTime % MS_PER_SEC);
		uiTime -= uiMs;
		if ((uiFlags & FMT_TIME_COUNTDOWN) && uiMs > 0)
			uiTime += MS_PER_SEC;
	}

	T _Result;
	bool bShowHours(iSeperations == 2 || (iSeperations == -1 && uiTime >= HrToMs(1u)));
	if (bShowHours)
	{
		uint uiHours(INT_FLOOR(MsToHr(uiTime)));
		uiTime %= MS_PER_HR;

		T _Hours;
		FormatInt(uiHours, FMT_NONE, 0, 10, 0, _Hours);
		_Result += _Hours;
		_Result += (uiFlags & FMT_TIME_ALPHA) ? GetTimeString<T>(TIME_STRING_HOURS) : GetTimeString<T>(TIME_STRING_SEPERATOR);
		
		if (!(uiFlags & FMT_TIME_ALPHA))
			bPadZero = true;
	}

	bool bShowMins(bShowHours || iSeperations >= 1 || (iSeperations == -1 && uiTime >= MinToMs(1u)));
	if (bShowMins)
	{
		uint uiMins(INT_FLOOR(MsToMin(uiTime)));
		uiTime %= MS_PER_MIN;

		T _Minutes;
		FormatInt(uiMins, bPadZero ? FMT_PADZERO : FMT_NONE, bPadZero ? 2 : 0, 10, 0, _Minutes);
		_Result += _Minutes;
		_Result += (uiFlags & FMT_TIME_ALPHA) ? GetTimeString<T>(TIME_STRING_MINUTES) : GetTimeString<T>(TIME_STRING_SEPERATOR);
		
		if (!(uiFlags & FMT_TIME_ALPHA))
			bPadZero = true;
	}

	float fSec(MsToSec(uiTime));
	float fPow(pow(10.0f, float(uiPrecision)));

	// Chop off remaining decimals past precision point
	fSec = (fSec * fPow) / fPow;

	T _Seconds;
	if (bPadZero)
		FormatFloat(fSec, FMT_PADZERO, 2 + uiPrecision + (uiPrecision > 0 ? 1 : 0), uiPrecision, _Seconds);
	else
		FormatFloat(fSec, FMT_NONE, 0, uiPrecision, _Seconds);

	_Result += _Seconds;

	if (uiFlags & FMT_TIME_ALPHA)
		_Result += GetTimeString<T>(TIME_STRING_SECONDS);

	return _Result;
}

inline string	FormatTimeS(uint uiTime, int iSeperations = -1, uint uiPrecision = 0, uint uiFlags = FMT_NONE)	{ return FormatTime<string>(uiTime, iSeperations, uiPrecision, uiFlags); }
inline wstring	FormatTimeW(uint uiTime, int iSeperations= - 1, uint uiPrecision = 0, uint uiFlags = FMT_NONE)	{ return FormatTime<wstring>(uiTime, iSeperations, uiPrecision, uiFlags); }

template<class T> T			GetInlineColorString(const CVec4f &v4Color);
template<> inline string	GetInlineColorString<string>(const CVec4f &v4Color)		{ return "^" + XtoS(INT_ROUND(v4Color[0] * 9.0f)) + XtoS(INT_ROUND(v4Color[1] * 9.0f)) + XtoS(INT_ROUND(v4Color[2] * 9.0f)); }
template<> inline wstring	GetInlineColorString<wstring>(const CVec4f &v4Color)	{ return L"^" + XtoW(INT_ROUND(v4Color[0] * 9.0f)) + XtoW(INT_ROUND(v4Color[1] * 9.0f)) + XtoW(INT_ROUND(v4Color[2] * 9.0f)); }

// Hex
template <class T>
int		HexAtoI(const T &s)
{
	if (s.empty())
		return 0;

	int iResult(0);
	int iMultiple(1);
	for (typename T::const_reverse_iterator cit(s.rbegin()); cit != s.rend(); ++cit)
	{
		if (*cit >= _T('0') && *cit <= _T('9'))
			iResult += (*cit - _T('0')) * iMultiple;
		else if (*cit >= _T('a') && *cit <= _T('f'))
			iResult += (*cit - _T('a') + 10) * iMultiple;
		else if (*cit >= _T('A') && *cit <= _T('F'))
			iResult += (*cit - _T('A') + 10) * iMultiple;
		else
			return 0;

		iMultiple *= 16;
	}

	return iResult;
}
//=============================================================================

#endif	//__XTOA_H__
