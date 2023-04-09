// (C)2005 S2 Games
// xtoa.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "xtoa.h"
#include "stringutils.h"

#include <iostream>
#include <iomanip>

using std::ios;
using std::setw;
using std::setprecision;
using std::streamsize;
//=============================================================================

/*====================
  FormatInt
 ====================*/
void    FormatInt(ULONGLONG ull, int flags, size_t width, int base, char sign, string &sStr)
{
    // Determine sign character, unless it's
    // already been determined negative
    if (sign == 0)
    {
        if (flags & FMT_SIGN)
            sign = '+';
        else if (flags & FMT_PADSIGN)
            sign = ' ';
    }

    // Set up Delimeter
    int dspacing(0);
    char dchar(0);
    if (flags & FMT_DELIMIT)
    {
        switch (base)
        {
        case 2:
            dchar = ' ';
            dspacing = 8;
            break;

        case 10:
            dchar = ',';
            dspacing = 3;
        }
    }

    // Build the string of digits
    ULONGLONG n(base);
    int d(1);
    do
    {
        TCHAR c = TCHAR((ull % n) / (n / base));
        c += (c > 9) ? ((c > 35) ? ('A' - 36) : ('a' - 10)) : '0';
        sStr += c;
        ull -= ull % n;
        if (dchar != 0 && ull != 0 && d == dspacing)
        {
            sStr += dchar;
            d = 0;
        }
        ++d;
        n *= base;
    }
    while (ull);

    int padcount(0);

    // Prefixes
    char prefix[3] = { 0, 0, 0 };
    if (!(flags & FMT_NOPREFIX))
    {
        if (base == 2)
        {
            prefix[0] = 'b';
            padcount -= 1;
        }
        else if (base == 8)
        {
            prefix[0] = '0';
            padcount -= 1;
        }
        else if (base == 16)
        {
            //backwards, the string will be reversed
            prefix[0] = 'x';
            prefix[1] = '0';
            padcount -= 2;
        }
    }

    // Prefix is included before non-zero padding
    if (prefix[0] && !(flags & FMT_PADZERO))
    {
        sStr += prefix;
        padcount = 0;
    }

    padcount += int(width) - int(sStr.length()) - ((sign == 0) ? 0 : 1);

    // If padding is space, add sign before padding
    if (!(flags & FMT_PADZERO) && sign != 0)
        sStr += sign;

    // Padding
    char padchar(' ');
    if (flags & FMT_PADZERO && !(flags & FMT_ALIGNLEFT))
        padchar = '0';
    if (padcount < 0)
        padcount = 0;
    string padding(padcount, padchar);
    // special case to delimit leading zeros
    if (dchar != 0 && padchar == '0')
    {
        for (size_t z(0); z < padding.length(); ++z)
        {
            if (z == padding.length() - 1)
                dchar = ' ';    // last delimiter always looks better as a blank

            if (d == dspacing + 1)
            {
                padding[z] = dchar;
                d = 0;
            }
            ++d;
        }
    }
    if (!(flags & FMT_ALIGNLEFT))
        sStr += padding;

    // Add prefix after zero padding
    if (prefix[0] && (flags & FMT_PADZERO))
        sStr += prefix;

    // If padding is 0, add sign after the padding
    if ((flags & FMT_PADZERO) && sign != 0)
        sStr += sign;

    reverse(sStr.begin(), sStr.end());

    // Left aligned padding
    if ((flags & FMT_ALIGNLEFT) && padchar != '0')
        sStr += padding;
}

void    FormatInt(ULONGLONG ull, int flags, size_t width, int base, wchar_t sign, wstring &sStr)
{
    // Determine sign character, unless it's
    // already been determined negative
    if (sign == 0)
    {
        if (flags & FMT_SIGN)
            sign = L'+';
        else if (flags & FMT_PADSIGN)
            sign = L' ';
    }

    // Set up Delimeter
    int dspacing(0);
    wchar_t dchar(0);
    if (flags & FMT_DELIMIT)
    {
        switch (base)
        {
        case 2:
            dchar = L' ';
            dspacing = 8;
            break;

        case 10:
            dchar = L',';
            dspacing = 3;
        }
    }

    // Build the string of digits
    ULONGLONG n(base);
    int d(1);
    do
    {
        TCHAR c = TCHAR((ull % n) / (n / base));
        c += (c > 9) ? ((c > 35) ? (L'A' - 36) : (L'a' - 10)) : L'0';
        sStr += c;
        ull -= ull % n;
        if (dchar != 0 && ull != 0 && d == dspacing)
        {
            sStr += dchar;
            d = 0;
        }
        ++d;
        n *= base;
    }
    while (ull);

    int padcount(0);

    // Prefixes
    wchar_t prefix[3] = { 0, 0, 0 };
    if (!(flags & FMT_NOPREFIX))
    {
        if (base == 2)
        {
            prefix[0] = L'b';
            padcount -= 1;
        }
        else if (base == 8)
        {
            prefix[0] = L'0';
            padcount -= 1;
        }
        else if (base == 16)
        {
            //backwards, the string will be reversed
            prefix[0] = L'x';
            prefix[1] = L'0';
            padcount -= 2;
        }
    }

    // Prefix is included before non-zero padding
    if (prefix[0] && !(flags & FMT_PADZERO))
    {
        sStr += prefix;
        padcount = 0;
    }

    padcount += int(width) - int(sStr.length()) - ((sign == 0) ? 0 : 1);

    // If padding is space, add sign before padding
    if (!(flags & FMT_PADZERO) && sign != 0)
        sStr += sign;

    // Padding
    wchar_t padchar(L' ');
    if (flags & FMT_PADZERO && !(flags & FMT_ALIGNLEFT))
        padchar = L'0';
    if (padcount < 0)
        padcount = 0;
    wstring padding(padcount, padchar);
    // special case to delimit leading zeros
    if (dchar != 0 && padchar == L'0')
    {
        for (size_t z(0); z < padding.length(); ++z)
        {
            if (z == padding.length() - 1)
                dchar = L' ';   // last delimiter always looks better as a blank

            if (d == dspacing + 1)
            {
                padding[z] = dchar;
                d = 0;
            }
            ++d;
        }
    }
    if (!(flags & FMT_ALIGNLEFT))
        sStr += padding;

    // Add prefix after zero padding
    if (prefix[0] && (flags & FMT_PADZERO))
        sStr += prefix;

    // If padding is 0, add sign after the padding
    if ((flags & FMT_PADZERO) && sign != 0)
        sStr += sign;

    reverse(sStr.begin(), sStr.end());

    // Left aligned padding
    if ((flags & FMT_ALIGNLEFT) && padchar != L'0')
        sStr += padding;
}


/*====================
  FormatFloat
 ====================*/
void    FormatFloat(double d, int flags, size_t width, int precision, string &sStr)
{
#if defined(__APPLE__)
    // fcvt is buggy when ndigit = 0, so lets just convert this to an int and format it that way
    if (!precision)
    {
        FormatInt((d < 0 ? -d : d), flags, width, 10, (d < 0 ? '-' : 0), sStr);
        return;
    }
#endif
    // Determine sign character
    char sign(0);
    if (d < 0)
    {
        sign = '-';
    }
    else
    {
        if (flags & FMT_SIGN)
            sign = '+';
        else if (flags & FMT_PADSIGN)
            sign = ' ';
    }

    if (_isnan(d))
    {
        sStr += "#NAN";
        return;
    }

    if (!_finite(d))
    {
        sStr += "#INF";
        return;
    }

    char szBuffer[256];
    int iDec;
    int iSign;

#if defined(_WIN32)
    _fcvt_s(szBuffer, 256, d, precision, &iDec, &iSign);
#else
    strncpy(szBuffer, fcvt(d, precision, &iDec, &iSign), 256);
#endif

    // Set up Delimeter
    int dspacing(0);
    char dchar(0);
    if (flags & FMT_DELIMIT)
    {
        dchar = ',';
        dspacing = 3;
    }

    // Build the string of digits
    if (iDec <= 0)
    {
        if (iDec < -precision)
            iDec = -precision;

        sStr += '0';
        if (precision > 0)
            sStr += '.';
        for (int i(iDec); i < 0; ++i)
            sStr += '0';
        for (char *sz(szBuffer); *sz; ++sz)
            sStr += *sz;
    }
    else
    {
        int i(0);

        for (char *sz(szBuffer); *sz; ++sz, ++i)
        {
            if (i == iDec && precision > 0)
                sStr += '.';
            if (dchar != 0 && i > 0 && i < iDec && (iDec - i) % dspacing == 0)
                sStr += dchar;
            sStr += *sz;
        }
    }

    // Create padding
    char padchar(' ');
    int padcount(int(width) - ((sign == 0) ? 0 : 1) - (MAX(iDec, 1)) - ((precision > 0) ? 1 : 0) - precision);
    if (flags & FMT_PADZERO && !(flags & FMT_ALIGNLEFT))
        padchar = '0';
    if (padcount < 0)
        padcount = 0;

    // Assemble the parts
    if (padcount > 0)
    {
        string padding(padcount, padchar);

        if (sign != 0)
        {
            if (flags & FMT_ALIGNLEFT)
            {
                sStr = sign + sStr + padding;
            }
            else
            {
                if (padchar == '0')
                    sStr = sign + padding + sStr;
                else
                    sStr = padding + sign + sStr;
            }
        }
        else
        {
            if (flags & FMT_ALIGNLEFT)
                sStr = sStr + padding;
            else
                sStr = padding + sStr;
        }
    }
    else
    {
        if (sign != 0)
            sStr = sign + sStr;
    }
}

void    FormatFloat(double d, int flags, size_t width, int precision, wstring &sStr)
{
#if defined(__APPLE__)
    // fcvt is buggy when ndigit = 0, so lets just convert this to an int and format it that way
    if (!precision)
    {
        FormatInt((d < 0 ? -d : d), flags, width, 10, (d < 0 ? L'-' : 0), sStr);
        return;
    }
#endif
    if (_isnan(d))
    {
        sStr += L"#NAN";
        return;
    }

    if (!_finite(d))
    {
        sStr += L"#INF";
        return;
    }

    // Determine sign character
    wchar_t sign(0);
    if (d < 0)
    {
        sign = L'-';
    }
    else
    {
        if (flags & FMT_SIGN)
            sign = L'+';
        else if (flags & FMT_PADSIGN)
            sign = L' ';
    }

    char szBuffer[256];
    wchar_t szBufferW[256];
    int iDec;
    int iSign;

#ifdef _WIN32
    _fcvt_s(szBuffer, 256, d, precision, &iDec, &iSign);
#else
    strncpy(szBuffer, fcvt(d, precision, &iDec, &iSign), 256);
#endif
    SingleToWide(szBufferW, szBuffer, 256);

    // Set up Delimeter
    int dspacing(0);
    wchar_t dchar(0);
    if (flags & FMT_DELIMIT)
    {
        dchar = L',';
        dspacing = 3;
    }

    // Build the string of digits
    if (iDec <= 0)
    {
        if (iDec < -precision)
            iDec = -precision;

        sStr += L'0';
        if (precision > 0)
            sStr += L'.';
        for (int i(iDec); i < 0; ++i)
            sStr += L'0';
        for (wchar_t *sz(szBufferW); *sz; ++sz)
            sStr += *sz;
    }
    else
    {
        int i(0);

        for (wchar_t *sz(szBufferW); *sz; ++sz, ++i)
        {
            if (i == iDec && precision > 0)
                sStr += L'.';
            if (dchar != 0 && i > 0 && i < iDec && (iDec - i) % dspacing == 0)
                sStr += dchar;
            sStr += *sz;
        }
    }

    // Create padding
    wchar_t padchar(L' ');
    int padcount(int(width) - ((sign == 0) ? 0 : 1) - (MAX(iDec, 1)) - ((precision > 0) ? 1 : 0) - precision);
    if (flags & FMT_PADZERO && !(flags & FMT_ALIGNLEFT))
        padchar = L'0';
    if (padcount < 0)
        padcount = 0;

    // Assemble the parts
    if (padcount > 0)
    {
        wstring padding(padcount, padchar);

        if (sign != 0)
        {
            if (flags & FMT_ALIGNLEFT)
            {
                sStr = sign + sStr + padding;
            }
            else
            {
                if (padchar == L'0')
                    sStr = sign + padding + sStr;
                else
                    sStr = padding + sign + sStr;
            }
        }
        else
        {
            if (flags & FMT_ALIGNLEFT)
                sStr = sStr + padding;
            else
                sStr = padding + sStr;
        }
    }
    else
    {
        if (sign != 0)
            sStr = sign + sStr;
    }
}

void    FormatFloat(double d, int flags, size_t width, int iMinPrecision, int iMaxPrecision, string &sStr)
{
#if defined(__APPLE__)
    // fcvt is buggy when ndigit = 0, so lets just convert this to an int and format it that way
    if (!iMaxPrecision)
    {
        FormatInt((d < 0 ? -d : d), flags, width, 10, (d < 0 ? '-' : 0), sStr);
        return;
    }
#endif
    // Determine sign character
    char sign(0);
    if (d < 0)
    {
        sign = '-';
    }
    else
    {
        if (flags & FMT_SIGN)
            sign = '+';
        else if (flags & FMT_PADSIGN)
            sign = ' ';
    }

    if (_isnan(d))
    {
        sStr += "#NAN";
        return;
    }

    if (!_finite(d))
    {
        sStr += "#INF";
        return;
    }

    char szBuffer[256];
    int iDec;
    int iSign;

#ifdef _WIN32
    _fcvt_s(szBuffer, 256, d, iMaxPrecision, &iDec, &iSign);
#else
    strncpy(szBuffer, fcvt(d, iMaxPrecision, &iDec, &iSign), 256);
#endif

    int iLength(int(strlen(szBuffer)));

    // Trim trailing zeros up to min precision
    while (iLength > iDec && szBuffer[iLength - 1] == '0' && iMaxPrecision > iMinPrecision)
    {
        --iLength;
        --iMaxPrecision;
    }

    szBuffer[iLength] = '\0';

    // Set up Delimeter
    int dspacing(0);
    char dchar(0);
    if (flags & FMT_DELIMIT)
    {
        dchar = ',';
        dspacing = 3;
    }

    // Build the string of digits
    if (iDec <= 0)
    {
        if (iDec < -iMaxPrecision)
            iDec = -iMaxPrecision;

        sStr += '0';
        if (iMaxPrecision > 0)
            sStr += '.';
        for (int i(iDec); i < 0; ++i)
            sStr += '0';
        for (char *sz(szBuffer); *sz; ++sz)
            sStr += *sz;
    }
    else
    {
        int i(0);

        for (char *sz(szBuffer); *sz; ++sz, ++i)
        {
            if (i == iDec && iMaxPrecision > 0)
                sStr += '.';
            if (dchar != 0 && i > 0 && i < iDec && (iDec - i) % dspacing == 0)
                sStr += dchar;
            sStr += *sz;
        }
    }

    // Create padding
    char padchar(' ');
    int padcount(int(width) - ((sign == 0) ? 0 : 1) - (MAX(iDec, 1)) - ((iMaxPrecision > 0) ? 1 : 0) - iMaxPrecision);
    if (flags & FMT_PADZERO && !(flags & FMT_ALIGNLEFT))
        padchar = '0';
    if (padcount < 0)
        padcount = 0;

    // Assemble the parts
    if (padcount > 0)
    {
        string padding(padcount, padchar);

        if (sign != 0)
        {
            if (flags & FMT_ALIGNLEFT)
            {
                sStr = sign + sStr + padding;
            }
            else
            {
                if (padchar == '0')
                    sStr = sign + padding + sStr;
                else
                    sStr = padding + sign + sStr;
            }
        }
        else
        {
            if (flags & FMT_ALIGNLEFT)
                sStr = sStr + padding;
            else
                sStr = padding + sStr;
        }
    }
    else
    {
        if (sign != 0)
            sStr = sign + sStr;
    }
}

void    FormatFloat(double d, int flags, size_t width, int iMinPrecision, int iMaxPrecision, wstring &sStr)
{
#if defined(__APPLE__)
    // fcvt is buggy when ndigit = 0, so lets just convert this to an int and format it that way
    if (!iMaxPrecision)
    {
        FormatInt((d < 0 ? -d : d), flags, width, 10, (d < 0 ? L'-' : 0), sStr);
        return;
    }
#endif
    // Determine sign character
    wchar_t sign(0);
    if (d < 0)
    {
        sign = L'-';
    }
    else
    {
        if (flags & FMT_SIGN)
            sign = L'+';
        else if (flags & FMT_PADSIGN)
            sign = L' ';
    }

    if (_isnan(d))
    {
        sStr += L"#NAN";
        return;
    }

    if (!_finite(d))
    {
        sStr += L"#INF";
        return;
    }

    char szBuffer[256];
    wchar_t szBufferW[256];
    int iDec;
    int iSign;

#ifdef _WIN32
    _fcvt_s(szBuffer, 256, d, iMaxPrecision, &iDec, &iSign);
#else
    strncpy((char*)szBuffer, fcvt(d, iMaxPrecision, &iDec, &iSign), 256);
#endif
    SingleToWide(szBufferW, szBuffer, 256);

    int iLength(int(wcslen(szBufferW)));

    // Trim trailing zeros up to min precision
    while (iLength > iDec && szBufferW[iLength - 1] == L'0' && iMaxPrecision > iMinPrecision)
    {
        --iLength;
        --iMaxPrecision;
    }

    szBufferW[iLength] = L'\0';

    // Set up Delimeter
    int dspacing(0);
    wchar_t dchar(0);
    if (flags & FMT_DELIMIT)
    {
        dchar = L',';
        dspacing = 3;
    }

    // Build the string of digits
    if (iDec <= 0)
    {
        if (iDec < -iMaxPrecision)
            iDec = -iMaxPrecision;

        sStr += L'0';
        if (iMaxPrecision > 0)
            sStr += L'.';
        for (int i(iDec); i < 0; ++i)
            sStr += L'0';
        for (wchar_t *sz(szBufferW); *sz; ++sz)
            sStr += *sz;
    }
    else
    {
        int i(0);

        for (wchar_t *sz(szBufferW); *sz; ++sz, ++i)
        {
            if (i == iDec && iMaxPrecision > 0)
                sStr += L'.';
            if (dchar != 0 && i > 0 && i < iDec && (iDec - i) % dspacing == 0)
                sStr += dchar;
            sStr += *sz;
        }
    }

    // Create padding
    wchar_t padchar(L' ');
    int padcount(int(width) - ((sign == 0) ? 0 : 1) - (MAX(iDec, 1)) - ((iMaxPrecision > 0) ? 1 : 0) - iMaxPrecision);
    if (flags & FMT_PADZERO && !(flags & FMT_ALIGNLEFT))
        padchar = L'0';
    if (padcount < 0)
        padcount = 0;

    // Assemble the parts
    if (padcount > 0)
    {
        wstring padding(padcount, padchar);

        if (sign != 0)
        {
            if (flags & FMT_ALIGNLEFT)
            {
                sStr = sign + sStr + padding;
            }
            else
            {
                if (padchar == L'0')
                    sStr = sign + padding + sStr;
                else
                    sStr = padding + sign + sStr;
            }
        }
        else
        {
            if (flags & FMT_ALIGNLEFT)
                sStr = sStr + padding;
            else
                sStr = padding + sStr;
        }
    }
    else
    {
        if (sign != 0)
            sStr = sign + sStr;
    }
}


/*====================
  XtoA
 ====================*/
wstring XtoW(const CVec2f &vec, int flags, int width, int precision)
{
    wstring sX, sY;

    FormatFloat(vec.x, flags, 0, precision, sX);
    FormatFloat(vec.y, flags, 0, precision, sY);

    wstring str(sX + L" " + sY);

    return XtoW(str, flags, width);
}

wstring XtoW(const CVec3f &vec, int flags, int width, int precision)
{
    wstring sX, sY, sZ;

    FormatFloat(vec.x, flags, 0, precision, sX);
    FormatFloat(vec.y, flags, 0, precision, sY);
    FormatFloat(vec.z, flags, 0, precision, sZ);

    wstring str(sX + L" " + sY + L" " + sZ);

    return XtoW(str, flags, width);
}

wstring XtoW(const CVec3<double> &vec, int flags, int width, int precision)
{
    wstring sX, sY, sZ;

    FormatFloat(vec.x, flags, 0, precision, sX);
    FormatFloat(vec.y, flags, 0, precision, sY);
    FormatFloat(vec.z, flags, 0, precision, sZ);

    wstring str(sX + L" " + sY + L" " + sZ);

    return XtoW(str, flags, width);
}

wstring XtoW(const CVec4f &v4, int flags, int width, int precision)
{
    wstring sX, sY, sZ, sW;

    FormatFloat(v4.x, flags, 0, precision, sX);
    FormatFloat(v4.y, flags, 0, precision, sY);
    FormatFloat(v4.z, flags, 0, precision, sZ);
    FormatFloat(v4.w, flags, 0, precision, sW);

    wstring str(sX + L" " + sY + L" " + sZ + L" " + sW);

    return XtoW(str, flags, width);
}

string  XtoS(const CVec2f &vec, int flags, int width, int precision)
{
    string sX, sY;

    FormatFloat(vec.x, flags, 0, precision, sX);
    FormatFloat(vec.y, flags, 0, precision, sY);

    string str(sX + " " + sY);

    return XtoS(str, flags, width);
}

string  XtoS(const CVec3f &vec, int flags, int width, int precision)
{
    string sX, sY, sZ;

    FormatFloat(vec.x, flags, 0, precision, sX);
    FormatFloat(vec.y, flags, 0, precision, sY);
    FormatFloat(vec.z, flags, 0, precision, sZ);

    string str(sX + " " + sY + " " + sZ);

    return XtoS(str, flags, width);
}

string  XtoS(const CVec3<double> &vec, int flags, int width, int precision)
{
    string sX, sY, sZ;

    FormatFloat(vec.x, flags, 0, precision, sX);
    FormatFloat(vec.y, flags, 0, precision, sY);
    FormatFloat(vec.z, flags, 0, precision, sZ);

    string str(sX + " " + sY + " " + sZ);

    return XtoS(str, flags, width);
}

string  XtoS(const CVec4f &v4, int flags, int width, int precision)
{
    string sX, sY, sZ, sW;

    FormatFloat(v4.x, flags, 0, precision, sX);
    FormatFloat(v4.y, flags, 0, precision, sY);
    FormatFloat(v4.z, flags, 0, precision, sZ);
    FormatFloat(v4.w, flags, 0, precision, sW);

    string str(sX + " " + sY + " " + sZ + " " + sW);

    return XtoS(str, flags, width);
}

wstring XtoW(const wstring &s, int flags, size_t width)
{
    if (s.length() >= width)
        return s;

    if (flags & FMT_ALIGNLEFT)
        return s + wstring(width - s.length(), L' ');
    else
        return wstring(width - s.length(), L' ') + s;
}

string XtoS(const string &s, int flags, size_t width)
{
    if (s.length() >= width)
        return s;

    if (flags & FMT_ALIGNLEFT)
        return s + string(width - s.length(), ' ');
    else
        return string(width - s.length(), ' ') + s;
}


/*====================
  AtoX
 ====================*/
CVec2f& AtoX(const tstring &s, CVec2f &v2)
{
    const tsvector &vComponents = TokenizeString(s, _T(' '));
    float   x, y;
    if (vComponents.size() < 2)
    {
        x = y = 0.0f;
    }
    else
    {
        AtoX(vComponents[0], x);
        AtoX(vComponents[1], y);
    }
    v2.Set(x, y);
    return v2;
}

CVec3f& AtoX(const tstring &s, CVec3f &v3)
{
    const tsvector &vComponents = TokenizeString(s, _T(' '));
    float   x, y, z;
    if (vComponents.size() < 3)
    {
        x = y = z = 0.0f;
    }
    else
    {
        AtoX(vComponents[0], x);
        AtoX(vComponents[1], y);
        AtoX(vComponents[2], z);
    }
    v3.Set(x, y, z);
    return v3;
}

CVec4f& AtoX(const tstring &s, CVec4f &v4)
{
    const tsvector &vComponents = TokenizeString(s, _T(' '));
    float   x, y, z, w;
    if (vComponents.size() < 4)
    {
        x = y = z = w = 0.0f;
    }
    else
    {
        AtoX(vComponents[0], x);
        AtoX(vComponents[1], y);
        AtoX(vComponents[2], z);
        AtoX(vComponents[3], w);
    }
    v4.Set(x, y, z, w);
    return v4;
}


/*====================
  AtoV2
 ====================*/
K2_API CVec2f   AtoV2(const tstring &s)
{
    CVec2f vRet;

    _STSCANF_S_BEGIN(s.c_str(), _T("%f %f"))
        &vRet.x, &vRet.y
    _STSCANF_S_END;

    return vRet;
}


/*====================
  AtoV3
 ====================*/
K2_API CVec3f   AtoV3(const tstring &s)
{
    CVec3f vRet;

    int iFields(_STSCANF_S_BEGIN(s.c_str(), _T("%f %f %f"))
        &vRet.x, &vRet.y, &vRet.z
    _STSCANF_S_END);

    if (iFields == 1)
        vRet.z = vRet.y = vRet.x;

    return vRet;
}


/*====================
  AtoV4
 ====================*/
CVec4f  AtoV4(const tstring &s)
{
    CVec4f vRet;

    _STSCANF_S_BEGIN(s.c_str(), _T("%f %f %f %f"))
        &vRet.x, &vRet.y, &vRet.z, &vRet.w
    _STSCANF_S_END;

    return vRet;
}


/*====================
  PtoI
  ====================*/
int     PtoI(const tstring &s)
{
    if (s[s.length() - 1] != _T('%'))
        return 0;

    int iRet(AtoI(s.substr(0, s.length() - 1)));
    CLAMP(iRet, 0, 100);
    return iRet;
}


/*====================
  PtoF
  ====================*/
float   PtoF(const tstring &s)
{
    if (s[s.length() - 1] != _T('%'))
        return 0.0f;

    float fRet(AtoF(s.substr(0, s.length() - 1)));
    return fRet;
}


/*====================
  P2toF
  ====================*/
float   P2toF(const tstring &s)
{
    if (s[s.length() - 1] != _T('@'))
        return 0.0f;

    float fRet(AtoF(s.substr(0, s.length() - 1)));
    return fRet;
}


/*====================
  BytesToHexString
  ====================*/
tstring         BytesToHexString(const byte* pData, size_t uiLen)
{
    // https://stackoverflow.com/questions/14050452/how-to-convert-byte-array-to-hex-string-in-visual-c
    std::stringstream ss;
    ss << std::hex;

    for( size_t i(0) ; i < uiLen; ++i ) {
        ss << std::setw(2) << std::setfill('0') << (int)pData[i];
    }

    return StringToTString(ss.str());
}

/*====================
  HexStringToBytes
  ====================*/
vector<byte>    HexStringToBytes(const tstring& sData)
{
    // https://stackoverflow.com/questions/17261798/converting-a-hex-string-to-a-byte-array
    std::stringstream ss;
    ss << TStringToUTF8(sData);

    std::vector<byte> resBytes;
    size_t count = 0;
    const auto len = sData.size();
    while(ss.good() && count < len)
    {
        unsigned short num;
        char hexNum[2];
        ss.read(hexNum, 2);
        sscanf(hexNum, "%2hX", &num);
        assert(num >= 0 && num <= 255);
        resBytes.push_back(static_cast<byte>(num));
        count += 2;
    }
    return resBytes;
}
