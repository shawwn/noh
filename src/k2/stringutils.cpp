// (C)2005 S2 Games
// stringutils.cpp
//
// string utility functions
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "stringutils.h"
#include "xtoa.h"

#include "c_cmd.h"
#include "c_function.h"
#include "c_fontmap.h"
#include "c_uicmd.h"

#define K2_USE_CODECVT 0
#if K2_USE_CODECVT
#include <codecvt>
#endif
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const string    SNULL("");
const wstring   WSNULL(L"");
const tstring   TSNULL(_T(""));
const tsvector  VSNULL;
const tsmapts   SMAPS_EMPTY;
//=============================================================================

/*====================
  TokenizeString
 ====================*/
tsvector    TokenizeString(const tstring &sStr, TCHAR cSeperator)
{
    tsvector    vReturn;

    size_t  zLast = 0;
    for (size_t zIndex(0); zIndex < sStr.length(); ++zIndex)
    {
        if (sStr[zIndex] == cSeperator)
        {
            vReturn.push_back(sStr.substr(zLast, zIndex - zLast));
            zLast = zIndex + 1;
        }
    }
    if (zLast < sStr.length())
        vReturn.push_back(sStr.substr(zLast));

    return vReturn;
}


/*====================
  JoinString

  returns an string of all the strings in the vector joined by the joinBy string
 ====================*/
tstring JoinString(tsvector &vStrings, const tstring &sJoinBy)
{
    tstring ret;

    for (tsvector::iterator it(vStrings.begin()); it != vStrings.end(); ++it)
        ret += *it + sJoinBy;
    return ret;
}


/*====================
  FormatStringNewline

 replaces \n with newline bytes
 ====================*/
tstring FormatStringNewline(tstring sString)
{
    tstring sReplaceString(sString);
    int     iOffset(0);
    for (uint I(0); I < sString.size() - 1; ++I)
    {
        tstring sSubString(sString.substr(size_t(I), size_t(2)));
        if (sSubString.compare(_T("\\n")) == 0)
        {
            sReplaceString.replace(size_t(I - iOffset), size_t(2), _T("\n"));
            ++iOffset;
        }
    }
    return sReplaceString;
}


/*--------------------
  FormatStringNewline
  --------------------*/
UI_CMD(FormatStringNewline, 1)
{
    if (vArgList.size() < 1)
        return TSNULL;

    tstring sString(vArgList[0]->Evaluate());

    return FormatStringNewline(sString);
}


/*--------------------
  FormatStringNewline
  --------------------*/
FUNCTION(FormatStringNewline)
{
    if (vArgList.size() < 1)
        return TSNULL;
    
    tstring sString(vArgList[0]);

    return FormatStringNewline(sString);
}


/*====================
  IsLineSeparator
 ====================*/
bool    IsLineSeparator(int c)
{
    static TCHAR s_cList[] = { '\x00', '\n', '\r' };

    for (uint n(0); n < sizeof(s_cList) / sizeof(TCHAR); ++n)
        if (c == s_cList[n])
            return true;
    return false;
}


/*====================
  IsTokenSeparator
 ====================*/
bool    IsTokenSeparator(int c)
{
    static TCHAR s_cList[] = { '\n', '\r', ' ', '\t' };

    for (uint n(0); n < sizeof(s_cList) / sizeof(TCHAR); ++n)
        if (c == s_cList[n])
            return true;
    return false;
}


/*====================
  IsAllWhiteSpace
  ====================*/
bool    IsAllWhiteSpace(const tstring &sStr)
{
    return sStr.find_first_not_of(_T("\n\r\t ")) == tstring::npos;
}


/*====================
  IsEOL
  ====================*/
bool    IsEOL(int c)
{
    if (c == 0x00)
        return false;

    static TCHAR s_cList[] = { '\n', '\r' };

    for (uint n(0); n < sizeof(s_cList); ++n)
        if (c == s_cList[n])
            return true;
    return false;
}


/*====================
  IsDigit
  ====================*/
bool    IsDigit(int c)
{
    return isdigit(c) != 0;
}


/*====================
  IsLetter
  ====================*/
bool    IsLetter(int c)
{
    if (c >= _T('A') && c <= _T('Z'))
        return true;
    if (c >= _T('a') && c <= _T('z'))
        return true;

    return false;
}


/*====================
  IsIdentifier
  ====================*/
bool    IsIdentifier(int c)
{
    if (IsDigit(c) || IsLetter(c) || c == _T('_'))
        return true;

    return false;
}


/*====================
  IsEmailAddress
  ====================*/
bool    IsEmailAddress(const tstring &sAddress)
{
    if (sAddress.empty())
        return false;

    size_t zAtPos(sAddress.find(_T('@'), 1));
    if (zAtPos == tstring::npos)
        return false;
    
    if (sAddress.find(_T('@'), zAtPos + 1) != tstring::npos)
        return false;

    size_t zDotPos(sAddress.rfind(_T('.')));
    if (zDotPos == tstring::npos)
        return false;

    if (zDotPos < zAtPos)
        return false;

    if (sAddress.size() - zDotPos > 7 || sAddress.size() - zDotPos <= 2)
        return false;

    return true;
}


/*====================
  IsValidCCNumber
  ====================*/
bool    IsValidCCNumber(const tstring &sInputNumber)
{
    tstring sNumber;
    for (tstring::const_iterator it(sInputNumber.begin()); it != sInputNumber.end(); ++it)
    {
        if (*it == _T('-') || *it == SPACE)
            continue;
        if (*it < _T('0') || *it > _T('9'))
            return false;
        
        sNumber += *it;
    }

    if (sNumber.length() < 13 || sNumber.length() > 16)
        return false;

    if (sNumber[0] == _T('3'))
    {
        if (sNumber[1] == _T('4') || sNumber[1] == _T('7')) // AmEx
        {
            if (sNumber.length() != 15)
                return false;
        }
        else if (sNumber[1] == _T('0') || sNumber[1] == _T('6') || sNumber[1] == _T('8'))   // Diners Club
        {
            if (sNumber[1] == _T('0'))
            {
                if (sNumber[2] < _T('0') || sNumber[2] > _T('5'))
                    return false;
            }

            if (sNumber.length() != 14)
                return false;
        }
        else
        {
            return false;
        }
    }
    else if (sNumber[0] == _T('4')) // Visa
    {
        if (sNumber.length() != 13 && sNumber.length() != 16)
            return false;
    }
    else if (sNumber[0] == _T('5')) // MasterCard
    {
        if (sNumber[1] < _T('1') || sNumber[1] > _T('5'))
            return false;
        
        if (sNumber.length() != 16)
            return false;
    }
    else if (sNumber[0] == _T('6')) // Discover
    {
        if (sNumber.substr(0, 4) == _T("6011"))
            return false;
        
        if (sNumber.length() != 16)
            return false;
    }
    else
    {
        return false;
    }

    uint uiCount(0);
    uint uiSum(0);
    for (tstring::reverse_iterator it(sNumber.rbegin()); it != sNumber.rend(); ++it)
    {
        if (*it == SPACE)
            continue;

        if (*it < _T('0') || *it > _T('9'))
            return false;

        ++uiCount;

        if (uiCount % 2 == 0)
        {
            uint uiNum(*it - _T('0'));
            uiSum += (2 * uiNum) / 10;
            uiSum += (2 * uiNum) % 10;
        }
        else
        {
            uiSum += *it - _T('0');
        }
    }

    return uiSum % 10 == 0;
}


/*====================
  IsIPAddress

  determines whether a given string is a valid ip address in the form (255.255.255.255)
  ====================*/
bool    IsIPAddress(const tstring &sIP)
{
    if (sIP.empty())
        return false;

    // Locate the seperators
    size_t dot1 = sIP.find_first_of(_T('.'));
    size_t dot2 = sIP.find_first_of(_T('.'), dot1 + 1);
    size_t dot3 = sIP.find_first_of(_T('.'), dot2 + 1);
    size_t dot4 = sIP.find_first_of(_T('.'), dot3 + 1); // sanity check for extra dots

    // If the three seperators don't exist,
    // or more than three exist,
    // we know this isn't a valid ip address
    if (dot1 == tstring::npos ||
        dot2 == tstring::npos ||
        dot3 == tstring::npos ||
        dot4 != tstring::npos)
        return false;

    // Each part of the ip must contain only digits
    tstring s1 = sIP.substr(0, dot1);
    tstring s2 = sIP.substr(dot1 + 1, dot2 - dot1 - 1);
    tstring s3 = sIP.substr(dot2 + 1, dot3 - dot2 - 1);
    tstring s4 = sIP.substr(dot3 + 1);

    if (count_if(s1.begin(), s1.end(), IsNotDigit) > 0 ||
        count_if(s2.begin(), s2.end(), IsNotDigit) > 0 ||
        count_if(s3.begin(), s3.end(), IsNotDigit) > 0 ||
        count_if(s4.begin(), s4.end(), IsNotDigit) > 0)
        return false;

    // Each component must be in the range 0 - 255
    int n1 = AtoI(s1);
    int n2 = AtoI(s2);
    int n3 = AtoI(s3);
    int n4 = AtoI(s4);

    if (n1 < 0 || n1 > 255 ||
        n2 < 0 || n2 > 255 ||
        n3 < 0 || n3 > 255 ||
        n4 < 0 || n4 > 255)
        return false;

    return true;
}


/*====================
  SplitBy
  ====================*/
void    SplitBy(tsvector &vSplits, const tstring &sStr, const tstring &sSeparator, size_t uiOffset, uint uiFlags)
{
    bool bSkipIfEmpty((uiFlags & SPLITBY_ERASE_EMPTY_SPLITS) != 0);

    assert(!sSeparator.empty());
    assert(uiOffset <= sStr.size());

    // search for the next separator.
    size_t uiFindPos(sStr.find(sSeparator, uiOffset));

    if (uiFindPos == tstring::npos)
    {
        // if we could not find any more separators, we're done.
        tstring sSplitResult(sStr.substr(uiOffset));
        if (!(bSkipIfEmpty && sSplitResult.empty()))
            vSplits.push_back(sSplitResult);
        return;
    }

    // push the substr before the separator to the results list.
    tstring sSplitResult(sStr.substr(uiOffset, uiFindPos - uiOffset));
    if (!(bSkipIfEmpty && sSplitResult.empty()))
        vSplits.push_back(sSplitResult);

    // recurse for the rest of the string.
    size_t uiNextPos(uiFindPos + sSeparator.size());
    if (uiNextPos >= sStr.length())
        return;

    SplitBy(vSplits, sStr, sSeparator, uiNextPos, uiFlags);
}


/*====================
  EqualsWildcard
  ====================*/
bool    EqualsWildcard(const tstring &sWild, const tstring &sComp)
{
    tstring::size_type zStart(0);
    tstring::size_type zEnd(tstring::npos);
    tstring::size_type zPos(0);

    tstring::size_type zLengthWild(sWild.length());
    tstring::size_type zLengthComp(sComp.length());

    // Loop until we hit the end of the string
    while (zStart < zLengthWild && zPos < zLengthComp)
    {
        // Find the first wildcard character
        zEnd = sWild.find_first_of(_T("?*"), zStart);

        // If the text from the current position to the next wildcard character
        // (or the end of the string, whichever is first) doesn't match, return false
        if (sWild.substr(zStart, zEnd - zStart) != sComp.substr(zPos, zEnd - zPos))
            return false;

        // If we hit the end of the string and the text does match, break out
        if (zEnd == tstring::npos)
            break;

        // If it's a single character wildcard
        if (sWild[zEnd] == _T('?'))
        {
            // Skip the current character in both strings, anything will match
            zPos += (zEnd - zStart) + 1;
            zStart = zEnd + 1;
        }
        else // Otherwise, if it's a * (any length) wildcard
        {
            // Set the new positions to directly after where the wildcard occurs
            zPos += (zEnd - zStart);
            zStart = zEnd + 1;

            // If the wildcard ends the string, then anything past here matches - return true
            if (zStart >= zLengthWild)
                return true;

            // Find the next wildcard
            zEnd = sWild.find_first_of(_T("?*"), zStart);

            // Find the string from the current position to the next wildcard in the
            // string we're comparing
            tstring::size_type zFind;
            zFind = sComp.find(sWild.substr(zStart, zEnd - zStart), zPos);

            // While we successfully matched it... (check all matches)
            while (zFind != tstring::npos)
            {
                // Recursively check the remaining text
                if (EqualsWildcard(sWild.substr(zStart), sComp.substr(zFind)))
                    return true;

                // Otherwise, look for the next match
                zFind = sComp.find(sWild.substr(zStart, zEnd - zStart), zFind + 1);
            }
        }
    }

    // If we've hit the end of both strings at this point (or the same amount
    // of length is remaining), they're a match, return true. Otherwise, return false.
    return (zLengthWild - zStart == zLengthComp - zPos);
}


/*====================
  EqualsWildcardNoCase
  ====================*/
bool    EqualsWildcardNoCase(const tstring &sWild, const tstring &sComp)
{
    tstring sLowerWild(LowerString(sWild));
    tstring sLowerComp(LowerString(sComp));
    return EqualsWildcard(sLowerWild, sLowerComp);
}


/*====================
  EqualsWildcards_
  ====================*/
static bool EqualsWildcards_(const tstring &sWild, const tstring &sComp, bool bCaseSensitive)
{
    // split each entry by | to create a list of "wildcards that could match"
    tsvector vWildcards;
    SplitBy(vWildcards, sWild, _T("|"));

    bool bMatches(false);

    for (size_t i(0); i < vWildcards.size(); ++i)
    {
        tstring &sCurWild(vWildcards[i]);
        if (sCurWild.empty())
            continue;

        // negate the wildcard?
        bool bShouldMatch(true);
        if (sCurWild[0] == _T('-'))
        {
            bShouldMatch = false;
            sCurWild.erase(0, 1);
        }

        // ensure that the wildcard begins with *
        if (sCurWild.empty() || sCurWild[0] != _T('*'))
            sCurWild = _TS("*") + sCurWild;

        // ensure that the wildcard ends with *
        if (*sCurWild.rbegin() != _T('*'))
            sCurWild.append(1, _T('*'));

        // test the string against the wildcard.
        bool bDoesMatch;
        if (bCaseSensitive)
            bDoesMatch = EqualsWildcard(sCurWild, sComp);
        else
            bDoesMatch = EqualsWildcardNoCase(sCurWild, sComp);

        if (bDoesMatch)
        {
            // if the wildcard shouldn't match but does, then fail.
            if (!bShouldMatch)
                return false;

            // flag it as a match.
            bMatches = true;
        }
    }

    return bMatches;
}


/*====================
  EqualsWildcards
  ====================*/
bool    EqualsWildcards(const tstring &sWild, const tstring &sComp, bool bCaseSensitive)
{
    // first, split by comma, creating a list of entries.
    tsvector vEntries;
    SplitBy(vEntries, sWild, _T(","));

    for (size_t uiEntry = 0; uiEntry < vEntries.size(); ++uiEntry)
    {
        const tstring &sEntry(vEntries[uiEntry]);

        // if the current entry matched, then finish (success).
        if (EqualsWildcards_(sEntry, sComp, bCaseSensitive))
            return true;
    }

    // if none of the entries matched, then fail.
    return false;
}


/*====================
  CompareNum
  ====================*/
int     CompareNum(const tstring &s1, const tstring &s2, size_t n)
{
    tstring::const_iterator p1 = s1.begin();
    tstring::const_iterator p2 = s2.begin();

    while (p1 != s1.end() && p2 != s2.end() && n != 0)
    {
        if (*p1 < *p2)
            return -1;
        else if (*p1 > *p2)
            return 1;

        ++p1;
        ++p2;
        --n;
    }

    if (n == 0)
        return 0;
    else
        return p1 == s1.end() ? p2 == s2.end() ? 0 : -1 : 1;
}


/*====================
  CompareNum
  ====================*/
int     CompareNum(const tstring &s1, const TCHAR *s2, size_t n)
{
    tstring::const_iterator p1 = s1.begin();
    const TCHAR *p2 = s2;

    while (p1 != s1.end() && *p2 != 0 && n != 0)
    {
        if (*p1 < *p2)
            return -1;
        else if (*p1 > *p2)
            return 1;

        ++p1;
        ++p2;
        --n;
    }

    if (n == 0)
        return 0;
    else
        return p1 == s1.end() ? p2 == nullptr ? 0 : -1 : 1;
}


/*====================
  CompareNoCaseNum
  ====================*/
int     CompareNoCaseNum(const TCHAR *p1, const TCHAR *p2, size_t n)
{
    while (*p1 && *p2 && n != 0)
    {
        TCHAR c1(tolower(*p1));
        TCHAR c2(tolower(*p2));
        if (c1 < c2)
            return -1;
        else if (c1 > c2)
            return 1;

        ++p1;
        ++p2;
        --n;
    }

    if (n == 0)
        return 0;
    else
        return p1 == nullptr ? p2 == nullptr ? 0 : -1 : 1;
}


/*====================
  StripEOL
 ====================*/
char*   StripEOL(char *str)
{
    char *s = str;

    while (*s)
    {
        if (IsLineSeparator(*s))
        {
            *s = 0;
            return str;
        }
        ++s;
    }
    return str;
}


/*====================
  GetNextWord
 ====================*/
char*   GetNextWord(const char *str)
{
    char *s = const_cast<char *>(str);

    while (!IsTokenSeparator(*s) && *s)
        ++s;

    if (!*s)
        return s;

    while (IsTokenSeparator(*s) && *s)
        ++s;

    if (!*s)
        return s;
    return s;
}

tstring GetNextWord(const tstring &sStr, size_t &zStart)
{
    size_t zEnd(sStr.find_first_of(_T("\n\r\t "), zStart));
    tstring sWord(sStr.substr(zStart, zEnd));
    zStart = zEnd;

    return sWord;
}


/*====================
  SkipSpaces
 ====================*/
TCHAR*  SkipSpaces(TCHAR *str)
{
    TCHAR *s = str;

    while (*s) {
        switch (*s) {
            case _T(' '):
            case _T('\n'):
            case _T('\r'):
            case _T('\t'):
                ++s;
                break;

            default:
                return s;
        }
    }

    return s;
}

size_t  SkipSpaces(const tstring &sStr, size_t zStart)
{
    size_t zResult(sStr.find_first_not_of(_T("\n\r\t "), zStart));
    return zResult;
}


/*====================
  Filename_GetExtension
 ====================*/
tstring Filename_GetExtension(const tstring &sPath)
{
    size_t end = sPath.find_last_of(_T('.'));

    if (end == string::npos)
        return TSNULL;
    else
        return sPath.substr(end + 1);
}


/*====================
  Filename_GetName
  ====================*/
tstring Filename_GetName(const tstring &sPath)
{
    if (sPath.empty())
        return TSNULL;

    size_t start = sPath.find_last_of(_T("/"));
    size_t end = sPath.find_last_of(_T("."));

    if (start == string::npos)
    {
        if (sPath.length() > 0 && (
            sPath[0] == '~' ||
            sPath[0] == '#' ||
            sPath[0] == ':'))
            start = 1;
        else
            start = 0;
    }
    else
        start += 1;

    if (start >= end)
        return TSNULL;

    if (end == string::npos)
        return sPath.substr(start, string::npos);
    else
        return sPath.substr(start, end - start);
}


/*====================
  Filename_GetPath
 ====================*/
tstring Filename_GetPath(const tstring &sPath)
{
    size_t end = sPath.find_last_of(_T("/"));

    if (end == string::npos)
    {
        if (sPath.length() > 0 && (
            sPath[0] == '~' ||
            sPath[0] == '#' ||
            sPath[0] == ':'))
            return sPath.substr(0, 1);
        
        return TSNULL;
    }
    else
        return sPath.substr(0, end + 1);
}


/*====================
  Filename_StripExtension
 ====================*/
tstring     Filename_StripExtension(const tstring &sPath)
{
    return sPath.substr(0, sPath.find_last_of(_T(".")));
}


/*====================
  Filename_StripPath
  ====================*/
tstring     Filename_StripPath(const tstring &sPath)
{
    if (sPath.empty())
        return TSNULL;

    size_t start = sPath.find_last_of(_T("/"));

    if (start == string::npos)
    {
        if (sPath.length() > 0 && (
            sPath[0] == '~' ||
            sPath[0] == '#' ||
            sPath[0] == ':'))
            return sPath.substr(1, sPath.length() - 1);

        return sPath;
    }
    else
        return sPath.substr(start + 1, string::npos);
}


/*====================
  Upgrade_GetType
  ====================*/
tstring Upgrade_GetType(const tstring &sProductCode)
{
    if (sProductCode.empty())
        return TSNULL;

    size_t zDot(sProductCode.find_first_of(_T(".")));

    if (zDot == tstring::npos)
        return TSNULL;
    else
        return sProductCode.substr(0, zDot);
}


/*====================
  Upgrade_GetName
  ====================*/
tstring Upgrade_GetName(const tstring &sProductCode)
{
    if (sProductCode.empty())
        return TSNULL;

    size_t zDot(sProductCode.find_first_of(_T(".")));

    if (zDot == tstring::npos)
        return sProductCode;
    else
        return sProductCode.substr(zDot + 1);
}


/*====================
  StrChrBackwards
 ====================*/
char*   StrChrBackwards(const char *str, int c)
{
    int i = int(strlen(str)) - 1;
    for (; i >= 0; --i)
    {
        if (str[i] == c)
            return (char*)&str[i];
    }
    if (i < 0)
        i = 0;
    return const_cast < char *>(&str[i]);
}


/*====================
  FirstToken
 ====================*/
char*   FirstToken(const char *str)
{
    static char empty[1] = { 0 };
    static char buf[NUM_TOK_STRINGS][128];
    static uint marker = 0;
    uint idx = marker;
    char *s = const_cast < char *>(str);
    int pos = 0;

    while(*s) {
        if (IsTokenSeparator(*s))
            break;

        buf[idx][pos] = *s;
        ++pos;
        if (pos >= 128) {
            return empty; // TKTK: was ""
        }
        ++s;
    }
    buf[idx][pos] = 0;
    marker = (marker + 1) % NUM_TOK_STRINGS;
    return buf[idx];
}


/*====================
  SplitArgs
 ====================*/
int SplitArgs(TCHAR *in, TCHAR **argv, int maxargs)
{
    TCHAR *s = in;
    int argc = 0;

    s = SkipSpaces(s);

    while (*s)
    {
        argv[argc] = s;
        ++argc;

        if (argc >= maxargs)
            return argc;

        ++s;

        while (!IsTokenSeparator(*s) && *s)
            ++s;

        if (*s)
        {
            *s = 0;     //null terminate each arg
            s = SkipSpaces(s + 1);
        }
    }
    return argc;
}

int     SplitArgs(const tstring &sString, tsvector &vArgList)
{
    size_t  zPos = sString.find_first_not_of(_T(" \n\r\t"));

    while (zPos != tstring::npos)
    {
        size_t zNextPos = sString.find_first_of(_T(" \n\r\t"), zPos + 1);

        if (zNextPos != tstring::npos)
            vArgList.push_back(sString.substr(zPos, zNextPos - zPos));
        else if (zPos + 1 < sString.length())
            vArgList.push_back(sString.substr(zPos));

        zPos = sString.find_first_not_of(_T(" \n\r\t"), zNextPos);
    }

    return int(vArgList.size());
}


/*====================
  ConcatinateArgs
 ====================*/
tstring ConcatinateArgs(const tsvector &vArgList, const tstring &sSeperator)
{
    tstring sResult;

    for (tsvector::const_iterator it(vArgList.begin()); it != vArgList.end(); ++it)
    {
        sResult += (*it);

        if (it + 1 != vArgList.end())
            sResult += sSeperator;
    }

    return sResult;
}

tstring ConcatinateArgs(tsvector::const_iterator begin, tsvector::const_iterator end, const tstring &sSeperator)
{
    tstring sResult;

    for (tsvector::const_iterator it(begin); it != end; ++it)
    {
        sResult += (*it);

        if (it + 1 != end)
            sResult += sSeperator;
    }

    return sResult;
}


/*====================
  StripColorCodes
  ====================*/
tstring     StripColorCodes(const tstring &sStr)
{
    tstring sOut;

    for (size_t z(0); z < sStr.length(); ++z)
    {
        if (sStr[z] == _T('^'))
        {
            if (z + 1 < sStr.length())
            {           
                tstring sPCString = sStr.substr(z + 1, 2);

                if (z + 1 < sStr.length() && CompareNoCase(sPCString, _T("?^")) == 0)
                {
                    z += 2;
                }
                
                if (!CompareNoCase(sPCString, _T("!b")) ||
                    !CompareNoCase(sPCString, _T("!t")) ||
                    !CompareNoCase(sPCString, _T("!p")) ||
                    !CompareNoCase(sPCString, _T("!y")) ||
                    !CompareNoCase(sPCString, _T("!o")) ||
                    !CompareNoCase(sPCString, _T("!i")) ||
                    !CompareNoCase(sPCString, _T("!v")) ||
                    !CompareNoCase(sPCString, _T("!l")) ||
                    !CompareNoCase(sPCString, _T("!g")) ||
                    !CompareNoCase(sPCString, _T("!n")))
                {
                    z += 2;
                    continue;
                }
                else if (sStr[z + 1] == _T('c') || sStr[z + 1] == _T('C') ||
                        sStr[z + 1] == _T('m') || sStr[z + 1] == _T('M') ||
                        sStr[z + 1] == _T('y') || sStr[z + 1] == _T('Y') ||
                        sStr[z + 1] == _T('k') || sStr[z + 1] == _T('K') ||
                        sStr[z + 1] == _T('r') || sStr[z + 1] == _T('R') ||
                        sStr[z + 1] == _T('g') || sStr[z + 1] == _T('G') ||
                        sStr[z + 1] == _T('b') || sStr[z + 1] == _T('B') ||
                        sStr[z + 1] == _T('v') || sStr[z + 1] == _T('V') ||
                        sStr[z + 1] == _T('p') || sStr[z + 1] == _T('P') ||
                        sStr[z + 1] == _T('o') || sStr[z + 1] == _T('O') ||
                        sStr[z + 1] == _T('n') || sStr[z + 1] == _T('N') ||
                        sStr[z + 1] == _T('t') || sStr[z + 1] == _T('T') ||
                        sStr[z + 1] == _T('w') || sStr[z + 1] == _T('W') ||
                        sStr[z + 1] == _T('*') || sStr[z + 1] == _T('?'))
                {
                    ++z;
                    continue;
                }
            }

            if (z + 3 < sStr.length())
            {
                if (sStr[z + 1] >= _T('0') && sStr[z + 1] <= _T('9') &&
                    sStr[z + 2] >= _T('0') && sStr[z + 2] <= _T('9') &&
                    sStr[z + 3] >= _T('0') && sStr[z + 3] <= _T('9'))
                {
                    z += 3;
                    continue;
                }
            }
        }

        sOut += sStr[z];
    }

    return sOut;
}

tstring     StripColorCodes(const tstring &sStr, StrColorMap &vColors)
{
    tstring sOut;
    for (size_t z(0); z < sStr.length(); ++z)
    {
        if (sStr[z] == _T('^'))
        {
            
            bool bGlow(false);
            StrColorKey key;
            key.first = sOut.length();
            bool bPlayerColor(false);
            
            if (z + 1 < sStr.length() && CompareNoCase(sStr.substr(z + 1, 2), _T("?^")) == 0)
            {
                bGlow = true;
                z += 2;
                if (!(z < sStr.length()))
                {
                    key.second = CVec4f(0.0f, 0.0f, 0.0f, 3.14f);
                    if (!vColors.empty() && vColors.back().first == key.first)
                        vColors.pop_back();
                    vColors.push_back(key);
                    break;
                }
            }
            else if (sStr[z + 1] == _T('?'))
            {
                ++z;
                key.second = CVec4f(0.0f, 0.0f, 0.0f, 3.14f);
                if (!vColors.empty() && vColors.back().first == key.first)
                    vColors.pop_back();
                vColors.push_back(key);
                continue;
            }


            if (z + 1 < sStr.length())
            {
                CVec4f v4Color(0.0f, 0.0f, 0.0f, 0.0f);
                tstring sPCString = sStr.substr(z + 1, 2);
                
                if (CompareNoCase(sPCString, _T("!b")) == 0)
                {
                    v4Color = PCBLUE;
                    bPlayerColor = true;
                }
                else if (CompareNoCase(sPCString, _T("!t")) == 0)
                {
                    v4Color = PCTEAL;
                    bPlayerColor = true;
                }
                else if (CompareNoCase(sPCString, _T("!p")) == 0)
                {
                    v4Color = PCPURPLE;
                    bPlayerColor = true;
                }
                else if (CompareNoCase(sPCString, _T("!y")) == 0)
                {
                    v4Color = PCYELLOW;
                    bPlayerColor = true;
                }                   
                else if (CompareNoCase(sPCString, _T("!o")) == 0)
                {
                    v4Color = PCORANGE;
                    bPlayerColor = true;
                }                   
                else if (CompareNoCase(sPCString, _T("!i")) == 0)
                {
                    v4Color = PCPINK;
                    bPlayerColor = true;
                }                   
                else if (CompareNoCase(sPCString, _T("!v")) == 0)
                {
                    v4Color = PCGRAY;
                    bPlayerColor = true;
                }   
                else if (CompareNoCase(sPCString, _T("!l")) == 0)
                {
                    v4Color = PCLIGHTBLUE;
                    bPlayerColor = true;
                }   
                else if (CompareNoCase(sPCString, _T("!g")) == 0)
                {
                    v4Color = PCDARKGREEN;
                    bPlayerColor = true;
                }   
                else if (CompareNoCase(sPCString, _T("!n")) == 0)
                {
                    v4Color = PCBROWN;
                    bPlayerColor = true;
                }                   
                else if (sStr[z + 1] == _T('c') || sStr[z + 1] == _T('C'))
                    v4Color = CYAN;
                else if (sStr[z + 1] == _T('m') || sStr[z + 1] == _T('M'))
                    v4Color = MAGENTA;
                else if (sStr[z + 1] == _T('y') || sStr[z + 1] == _T('Y'))
                    v4Color = YELLOW;
                else if (sStr[z + 1] == _T('k') || sStr[z + 1] == _T('K'))
                    v4Color = BLACK;
                else if (sStr[z + 1] == _T('r') || sStr[z + 1] == _T('R'))
                    v4Color = RED;
                else if (sStr[z + 1] == _T('g') || sStr[z + 1] == _T('G'))
                    v4Color = LIME;
                else if (sStr[z + 1] == _T('b') || sStr[z + 1] == _T('B'))
                    v4Color = BLUE;
                else if (sStr[z + 1] == _T('v') || sStr[z + 1] == _T('V'))
                    v4Color = GRAY;
                else if (sStr[z + 1] == _T('p') || sStr[z + 1] == _T('P'))
                    v4Color = PURPLE;
                else if (sStr[z + 1] == _T('o') || sStr[z + 1] == _T('O'))
                    v4Color = ORANGE;
                else if (sStr[z + 1] == _T('n') || sStr[z + 1] == _T('N'))
                    v4Color = BROWN;
                else if (sStr[z + 1] == _T('t') || sStr[z + 1] == _T('T'))
                    v4Color = TEAL;
                else if (sStr[z + 1] == _T('w') || sStr[z + 1] == _T('W'))
                    v4Color = WHITE;
                else if (sStr[z + 1] == _T('*'))
                    v4Color = CVec4f(-1.0f, -1.0f, -1.0f, 1.0f);

                if (v4Color.w > 0.0f)
                {
                    if (z + 2 < sStr.length() && CompareNoCase(sStr.substr(z + 2, 2), _T("^?")) == 0)
                    {
                        bGlow = true;
                        z += 2;
                    }
                    if (bGlow)
                        v4Color.w += 3.14f;

                    key.second = v4Color;
                    if (!vColors.empty() && vColors.back().first == key.first)
                        vColors.pop_back();
                    vColors.push_back(key);
                    if (!bPlayerColor)
                        z += 1;
                    else
                        z += 2;                     
                    continue;
                }
            }

            if (z + 3 < sStr.length())
            {
                if (sStr[z + 1] >= _T('0') && sStr[z + 1] <= _T('9') &&
                    sStr[z + 2] >= _T('0') && sStr[z + 2] <= _T('9') &&
                    sStr[z + 3] >= _T('0') && sStr[z + 3] <= _T('9'))
                {
                    if (z + 2 < sStr.length() && CompareNoCase(sStr.substr(z + 2, 2), _T("^?")) == 0)
                    {
                        bGlow = true;
                        z += 2;
                    }

                    key.second.x = (sStr[z + 1] - _T('0')) / 9.0f;
                    key.second.y = (sStr[z + 2] - _T('0')) / 9.0f;
                    key.second.z = (sStr[z + 3] - _T('0')) / 9.0f;
                    key.second.w = bGlow ? 4.14f : 1.0f;
                    if (!vColors.empty() && vColors.back().first == key.first)
                        vColors.pop_back();
                    vColors.push_back(key);
                    z += 3;
                    continue;
                }
            }
        }

        sOut += sStr[z];
    }

    return sOut;
}


/*====================
  StripStartingSpaces
  ====================*/
tstring     StripStartingSpaces(const tstring &sStr)
{
    tstring sReturn = sStr;
    size_t zEndPos = SkipSpaces(sReturn);

    if (zEndPos == 0)
        return sReturn;

    if (zEndPos != tstring::npos)
        sReturn.erase(0, zEndPos);
    else
        sReturn.erase(0, sReturn.length());

    return sReturn;
}


/*====================
  StripDuplicateSpaces
  ====================*/
tstring     StripDuplicateSpaces(const tstring &sStr)
{
    size_t zPos = 0;
    size_t zEndPos = 0;
    tstring sReturn = sStr;

    while (zPos != tstring::npos)
    {
        zPos = sReturn.find_first_of(_T(" "), zPos);

        if (zPos != tstring::npos)
            zPos++;

        if (zPos < sReturn.length())
        {
            zEndPos = SkipSpaces(sReturn, zPos);

            if (zEndPos == zPos)
                continue;

            if (zEndPos != tstring::npos)
                sReturn.erase(zPos, zEndPos);
            else
                sReturn.erase(zPos, sReturn.length() - zPos);
        }
    }

    return sReturn;
}


/*====================
  GetLastColorCode
  ====================*/
tstring     GetLastColorCode(const tstring &sStr)
{
    tstring sLastCode(TSNULL);

    if (sStr.length() < 2)
        return sLastCode;

    for (size_t z(sStr.length() - 1); z != tstring::npos; --z)
    {
        if (sStr[z] == _T('^'))
        {
            if (z + 1 < sStr.length())
            {
                tstring sPCString = sStr.substr(z + 1, 2);
                
                if (!CompareNoCase(sPCString, _T("!b")) ||
                    !CompareNoCase(sPCString, _T("!t")) ||
                    !CompareNoCase(sPCString, _T("!p")) ||
                    !CompareNoCase(sPCString, _T("!y")) ||
                    !CompareNoCase(sPCString, _T("!o")) ||
                    !CompareNoCase(sPCString, _T("!i")) ||
                    !CompareNoCase(sPCString, _T("!v")) ||
                    !CompareNoCase(sPCString, _T("!l")) ||
                    !CompareNoCase(sPCString, _T("!g")) ||
                    !CompareNoCase(sPCString, _T("!n")))
                {
                    sLastCode = sPCString;
                    break;              
                }
                else if (sStr[z + 1] == _T('c') || sStr[z + 1] == _T('C') ||
                        sStr[z + 1] == _T('m') || sStr[z + 1] == _T('M') ||
                        sStr[z + 1] == _T('y') || sStr[z + 1] == _T('Y') ||
                        sStr[z + 1] == _T('k') || sStr[z + 1] == _T('K') ||
                        sStr[z + 1] == _T('r') || sStr[z + 1] == _T('R') ||
                        sStr[z + 1] == _T('g') || sStr[z + 1] == _T('G') ||
                        sStr[z + 1] == _T('b') || sStr[z + 1] == _T('B') ||
                        sStr[z + 1] == _T('v') || sStr[z + 1] == _T('V') ||
                        sStr[z + 1] == _T('p') || sStr[z + 1] == _T('P') ||
                        sStr[z + 1] == _T('o') || sStr[z + 1] == _T('O') ||
                        sStr[z + 1] == _T('n') || sStr[z + 1] == _T('N') ||
                        sStr[z + 1] == _T('t') || sStr[z + 1] == _T('T') ||
                        sStr[z + 1] == _T('w') || sStr[z + 1] == _T('W') ||
                        sStr[z + 1] == _T('*'))
                {
                    sLastCode = sStr.substr(z, 2);
                    break;
                }
            }

            if (z + 3 < sStr.length())
            {
                if (sStr[z + 1] >= _T('0') && sStr[z + 1] <= _T('9') &&
                    sStr[z + 2] >= _T('0') && sStr[z + 2] <= _T('9') &&
                    sStr[z + 3] >= _T('0') && sStr[z + 3] <= _T('9'))
                {
                    sLastCode = sStr.substr(z, 4);
                    break;
                }
            }
        }
    }

    return sLastCode;
}


/*====================
  SingleToWide

  Returns a wide char version of 'in', without any special conversion.
  'len' is the maximum length of the 'out' buffer
  ====================*/
wchar_t*    SingleToWide(wchar_t *out, const char *in, size_t len)
{
    for (size_t n = 0; n < len; ++n)
    {
        out[n] = in[n];
        if (in[n] == '\x00')
            break;
    }

    return out;
}

wchar_t*    SingleToWide(wchar_t *out, const xmlChar *in, size_t len)
{
    for (size_t n = 0; n < len; ++n)
    {
        if (in[n] == '\x00')
            break;
        out[n] = in[n];
    }

    return out;
}

wstring     SingleToWide(const string &sIn)
{
    wstring sOut;
    for (size_t z(0); z < sIn.length(); ++z)
        sOut += wchar_t(sIn[z]);

    return sOut;
}


/*====================
  WideToSingle

  Returns a single char version of 'in', truncating the wide chars in 'out'.
  'len' is the maximum length of the 'out' buffer
  ====================*/
char*   WideToSingle(char *out, const wchar_t *in, size_t len)
{
    for (size_t n = 0; n < len; ++n)
    {
        if (in[n] == L'\x00')
            break;
        out[n] = char(in[n]);
    }

//  Console.Perf << _T("WideToSingle()") << newl;
    return out;
}

string      WideToSingle(const wstring &sIn)
{
    string sOut;
    for (size_t z(0); z < sIn.length(); ++z)
        sOut += char(sIn[z]);

//  Console.Perf << _T("WideToSingle() : ") << newl;
    return sOut;
}


/*====================
  IsUTF16
  ====================*/
bool            IsUTF16(bool &bOutLittleEndian, const char *in, size_t len)
{
    bOutLittleEndian = true;
    if (len < 2)
        return false;

    byte y0((byte)in[0]);
    byte y1((byte)in[1]);

    if (y0 == 0xFF &&
        y1 == 0xFE)
    {
        bOutLittleEndian = true;
        return true;
    }

    if (y0 == 0xFE &&
        y1 == 0xFF)
    {
        bOutLittleEndian = false;
        return true;
    }
    return false;
}


/*====================
  IsUTF8
  ====================*/
bool        IsUTF8(const char *in, size_t len)
{
    if (len < 3)
        return false;

    char pUTF8BOM[3] = { (char)0xEF, (char)0xBB, (char)0xBF };
    if (in[0] == pUTF8BOM[0] &&
        in[1] == pUTF8BOM[1] &&
        in[2] == pUTF8BOM[2])
    {
        return true;
    }

    return false;
}


/*====================
  UTFChar2Len
    Return the number of bytes the UTF-8 encoding of character "c" takes.
    This does not include composing characters.
  ====================*/
int     UTFChar2Len(int c)
{
    if (c < 0x80)
        return 1;
    if (c < 0x800)
        return 2;
    if (c < 0x10000)
        return 3;
    if (c < 0x200000)
        return 4;
    if (c < 0x4000000)
        return 5;
    return 6;
}


/*====================
  UTFChar2Bytes
    Convert Unicode character "c" to UTF-8 string in "buf[]".
    Returns the number of bytes.
    This does not include composing characters.
  ====================*/
int     UTFChar2Bytes(int c, char* buf)
{
    if (c < 0x80)               /* 7 bits */
    {
        buf[0] = c;
        return 1;
    }
    if (c < 0x800)              /* 11 bits */
    {
        buf[0] = 0xc0 + ((unsigned)c >> 6);
        buf[1] = 0x80 + (c & 0x3f);
        return 2;
    }
    if (c < 0x10000)            /* 16 bits */
    {
        buf[0] = 0xe0 + ((unsigned)c >> 12);
        buf[1] = 0x80 + (((unsigned)c >> 6) & 0x3f);
        buf[2] = 0x80 + (c & 0x3f);
        return 3;
    }
    if (c < 0x200000)           /* 21 bits */
    {
        buf[0] = 0xf0 + ((unsigned)c >> 18);
        buf[1] = 0x80 + (((unsigned)c >> 12) & 0x3f);
        buf[2] = 0x80 + (((unsigned)c >> 6) & 0x3f);
        buf[3] = 0x80 + (c & 0x3f);
        return 4;
    }
    if (c < 0x4000000)          /* 26 bits */
    {
        buf[0] = 0xf8 + ((unsigned)c >> 24);
        buf[1] = 0x80 + (((unsigned)c >> 18) & 0x3f);
        buf[2] = 0x80 + (((unsigned)c >> 12) & 0x3f);
        buf[3] = 0x80 + (((unsigned)c >> 6) & 0x3f);
        buf[4] = 0x80 + (c & 0x3f);
        return 5;
    }
                                /* 31 bits */
    buf[0] = 0xfc + ((unsigned)c >> 30);
    buf[1] = 0x80 + (((unsigned)c >> 24) & 0x3f);
    buf[2] = 0x80 + (((unsigned)c >> 18) & 0x3f);
    buf[3] = 0x80 + (((unsigned)c >> 12) & 0x3f);
    buf[4] = 0x80 + (((unsigned)c >> 6) & 0x3f);
    buf[5] = 0x80 + (c & 0x3f);
    return 6;
}


/*====================
  UTF16to8

    Convert an UTF-16 string to UTF-8.
    The input is "instr[inlen]" with "inlen" in number of UTF-16 words.
    When "outstr" is nullptr only return the required number of bytes.
    Otherwise "outstr" must be a buffer of sufficient size.
    Return the number of bytes produced.
  ====================*/
int     UTF16to8(short *instr, int inlen, char *outstr)
{
    int         outlen = 0;
    int         todo = inlen;
    short       *p = instr;
    int         l;
    int         ch, ch2;

    while (todo > 0)
    {
        ch = *p;
        if (ch >= 0xD800 && ch <= 0xDBFF && todo > 1)
        {
            /* surrogate pairs handling */
            ch2 = p[1];
            if (ch2 >= 0xDC00 && ch2 <= 0xDFFF)
            {
                ch = ((ch - 0xD800) << 10) + (ch2 & 0x3FF) + 0x10000;
                ++p;
                --todo;
            }
        }
        if (outstr != nullptr)
        {
            l = UTFChar2Bytes(ch, outstr);
            outstr += l;
        }
        else
        {
            l = UTFChar2Len(ch);
        }
        ++p;
        outlen += l;
        --todo;
    }

    return outlen;
}


/*====================
  UTF8ToString
  ====================*/
string  UTF8ToString(const string &sIn)
{
    string sReturn;
    size_t zSize(sIn.length());

    for (size_t zPosition(0); zPosition < zSize; )
    {
        uint uiResult(sIn[zPosition]);
        if ((uiResult & 0x80) == 0)
        {
            // Single byte
            sReturn += char(uiResult & 0xff);
            ++zPosition;
            continue;
        }

        // Determine how many bytes are in this character
        uint uiMask(0x80);
        uint uiNumBytes(0);
        while (zPosition < zSize && (sIn[zPosition] & uiMask))
        {
            ++uiNumBytes;
            uiMask >>= 1;
        }

        // Assign the remaining bits of the first byte
        uiMask -= 1;
        uiResult = (uiResult & uiMask) << ((uiNumBytes - 1) * 6);
        ++zPosition;

        // Assemble the remaining bytes
        for (uint ui(2); ui <= uiNumBytes; ++ui)
        {
            uint uiNext(sIn[zPosition]);
            if (uiNext == 0 || (uiNext & 0xc0) != 0x80)
            {
                uiResult = 0;
                break;
            }

            uiResult |= (uiNext & 0x3f) << ((uiNumBytes - ui) * 6);
            ++zPosition;
        }

        if (uiResult == 0)
            continue;

        if (uiResult > 0xff)
            uiResult = '?';

        sReturn += char(uiResult & 0xff);
    }

    return sReturn;
}


/*====================
  UTF8ToWString
  ====================*/
wstring UTF8ToWString(const string &sIn)
{
#if K2_USE_CODECVT && (defined(__APPLE__) || defined(WIN32))
    std::wstring_convert< std::codecvt_utf8_utf16<wchar_t> > myconv;
    return myconv.from_bytes(sIn);
#else // TKTK
    wstring sReturn;
    size_t zSize(sIn.length());

    for (size_t zPosition(0); zPosition < zSize; )
    {
        uint uiResult(sIn[zPosition]);
        if ((uiResult & 0x80) == 0)
        {
            // Single byte
            sReturn += wchar_t(uiResult);
            ++zPosition;
            continue;
        }

        // Determine how many bytes are in this character
        uint uiMask(0x80);
        uint uiNumBytes(0);
        while (zPosition < zSize && (sIn[zPosition] & uiMask))
        {
            ++uiNumBytes;
            uiMask >>= 1;
        }

        // Assign the remaining bits of the first byte
        uiMask -= 1;
        uiResult = (uiResult & uiMask) << ((uiNumBytes - 1) * 6);
        ++zPosition;

        // Assemble the remaining bytes
        for (uint ui(2); ui <= uiNumBytes; ++ui)
        {
            uint uiNext(sIn[zPosition]);
            if (uiNext == 0 || (uiNext & 0xc0) != 0x80)
            {
                uiResult = L'?';
                break;
            }

            uiResult |= (uiNext & 0x3f) << ((uiNumBytes - ui) * 6);
            ++zPosition;
        }

#ifdef _WIN32
        if (uiResult > 0xffff)
            uiResult = L'?';
#endif

        sReturn += wchar_t(uiResult);
    }

    return sReturn;
#endif
}


string  WStringToUTF8(const wstring &in)
{
#if K2_USE_CODECVT && (defined(__APPLE__) || defined(WIN32))
    std::wstring_convert< std::codecvt_utf8_utf16<wchar_t> > myconv;
    return myconv.to_bytes(in);
#else // TKTK

        //MikeG Check for null
    if (in.empty())
        return "";

#ifdef _WIN32
    unsigned long *decoded(K2_NEW(ctx_Strings,  unsigned long[in.length()]));
    size_t count = 0;
    const wchar_t *wat = in.c_str();
#else
    size_t count = in.length();
    const wstring &decoded(in);
#endif
    string out;

#ifdef _WIN32
    // Decode the UTF-16 into an array
    while (*wat != L'\x0000')
    {
        // Watch out for overflow (preliminary check)
        if (count >= in.length())
        {
            Console.Warn << _T("WStringToUTF8(): Truncating input string") << newl;
            break;
        }

        // This is part of a surrogate pair
        if (*wat >= 0xd800 && *wat <= 0xdfff)
        {
            // TODO: Handle endianess?
            // Save the first 10 bits
            unsigned long c = (*wat & 0x03ff) << 10;

            // Advance and verify next byte
            ++wat;
            if (((*wat) & 0xfc00) != 0xdc00)
            {
                Console.Warn << _T("WStringToUTF8(): Invalid UTF-16 string") << newl;
                out.clear();
                K2_DELETE_ARRAY(decoded);
                return out;
            }

            // Get the second set of 10 bits
            c |= (*wat & 0x03ff);
            decoded[count++] = c;
            continue;
        }

        // No decoding necessary
        decoded[count++] = *wat;
        ++wat;
    }
#endif

    // Create the UTF-8 string
    for (size_t o(0); o < count; ++o)
    {
        // Single byte
        if ((decoded[o] & 0xffffff80) == 0)
        {
            out += char(decoded[o] & 0x0000007f);
            continue;
        }

        // Two bytes
        if ((decoded[o] & 0xfffff800) == 0)
        {
            out += 0xc0 | char((decoded[o] >> 6) & 0x1f);
            out += 0x80 | char(decoded[o] & 0x3f);
            continue;
        }

        // Three bytes
        if ((decoded[o] & 0xffff0000) == 0)
        {
            out += 0xe0 | char((decoded[o] >> 12) & 0x0f);
            out += 0x80 | char((decoded[o] >> 6) & 0x3f);
            out += 0x80 | char(decoded[o] & 0x3f);
            continue;
        }

        // Four bytes

        // Check for overflow
        out += 0xf0 | char((decoded[o] >> 18) & 0x07);
        out += 0x80 | char((decoded[o] >> 12) & 0x3f);
        out += 0x80 | char((decoded[o] >> 6) & 0x3f);
        out += 0x80 | char(decoded[o] & 0x3f);
    }

#ifdef _WIN32
    K2_DELETE_ARRAY(decoded);
#endif
    return out;
#endif
}


/*====================
  StringToUTF8
  ====================*/
string  StringToUTF8(const string &sIn)
{
    string sOut;
    size_t zLength(sIn.length());

    for (size_t z(0); z < zLength; ++z)
    {
        // Single byte
        if ((sIn[z] & 0x80) == 0)
        {
            sOut += sIn[z];
            continue;
        }

        uint uiResult(sIn[z]);

        // Determine how many bytes are in this character
        uint uiMask(0x80);
        uint uiNumBytes(0);
        while (uiResult & uiMask)
        {
            ++uiNumBytes;
            uiMask >>= 1;
        }

        if (zLength + uiNumBytes >= zLength)
        {
            sOut += '?';
            continue;
        }

        // Assign the remaining bits of the first byte
        uiMask -= 1;
        uiResult = (uiResult & uiMask) << ((uiNumBytes - 1) * 6);
        ++z;

        // Assemble the remaining bytes
        for (uint ui(2); ui <= uiNumBytes; ++ui)
        {
            uint uiNext(sIn[z]);
            if ((uiNext & 0xc0) != 0x80)
            {
                sOut += '?';
                break;
            }

            uiResult |= (uiNext & 0x3f) << ((uiNumBytes - ui) * 6);
            ++z;
        }
    }

    return sOut;
}

// the behaviour of these when an error is encountered might need to be modified

/*====================
  WCSToMBS
  string version of wcstombs
  ====================*/
string WCSToMBS(const wstring &sIn)
{
    string sOut;
    size_t zLength(sIn.length());
    static char* s(K2_NEW_ARRAY(ctx_Strings, char, MB_CUR_MAX));
    int ret;
    
    for (size_t z(0); z < zLength; ++z)
    {
        WCTOMB_S(ret, s, MB_CUR_MAX, sIn[z]);
        if (ret > 0)
        {
            for (int i(0); i < ret; ++i)
                sOut += s[i];
        }
        else if (ret == 0) // hit NUL
        {
            break;
        }
    }
    
    return sOut;
}

string WCSToMBS(const wchar_t *sIn)
{
    string sOut;
    size_t zLength(wcslen(sIn));
    static char* s(K2_NEW_ARRAY(ctx_Strings, char, MB_CUR_MAX));
    int ret;
    
    for (size_t z(0); z < zLength; ++z)
    {
        WCTOMB_S(ret, s, MB_CUR_MAX, sIn[z]);
        if (ret > 0)
        {
            for (int i(0); i < ret; ++i)
                sOut += s[i];
        }
        else if (ret == 0) // hit NUL
        {
            break;
        }
    }

    return sOut;
}

/*====================
  MBSToWCS
  string version of mbstowcs
  ====================*/
wstring MBSToWCS(const string &sIn)
{
    wstring sOut;
    size_t zLength(sIn.length());
    wchar_t wc;
    int ret;
    
    for (size_t z(0); z < zLength; z += ret)
    {
        if ((ret = mbtowc(&wc, &(sIn.c_str()[z]), zLength - z)) > 0)
            sOut += wc;
        else if (ret == 0) // hit NUL
            break;
        else // error! try next char for start of a multi-byte sequence
            ret = 1;
    }
    
    return sOut;
}

wstring MBSToWCS(const char *sIn)
{
    wstring sOut;
    size_t zLength(strlen(sIn));
    wchar_t wc;
    int ret;
    
    for (size_t z(0); z < zLength; z += ret)
    {
        if ((ret = mbtowc(&wc, &sIn[z], zLength - z)) > 0)
            sOut += wc;
        else if (ret == 0) // hit NUL
            break;
        else // error! try next char for start of a multi-byte sequence
            ret = 1;
    }
    
    return sOut;
}


/*====================
  StripNewline
  ====================*/
void    StripNewline(tstring &sStr)
{
    for (size_t zOffset(0); ;)
    {
        size_t zNextN(sStr.find('\n', zOffset));
        size_t zNextR(sStr.find('\r', zOffset));
        zOffset = MIN(zNextN, zNextR);
        if (zOffset == tstring::npos)
            break;

        sStr.erase(zOffset, 1);
    }
}


/*====================
  NormalizeLineBreaks
  ====================*/
void    NormalizeLineBreaks(string &sStr, const string &sBreak)
{
    for (size_t zOffset(0); ; zOffset += sBreak.length())
    {
        size_t zNextN(sStr.find('\n', zOffset));
        size_t zNextR(sStr.find('\r', zOffset));
        zOffset = MIN(zNextN, zNextR);
        if (zOffset == string::npos)
            break;

        // Check for a windows line break
        if (sStr[zOffset] == '\r' &&
            zOffset + 1 < sStr.length() &&
            sStr[zOffset + 1] == '\n')
        {
            sStr.replace(zOffset, 2, sBreak);
            continue;
        }

        sStr.replace(zOffset, 1, sBreak);
    }
}

void    NormalizeLineBreaks(wstring &sStr, const wstring &sBreak)
{
    for (size_t zOffset(0); ; zOffset += sBreak.length())
    {
        size_t zNextN(sStr.find(L'\n', zOffset));
        size_t zNextR(sStr.find(L'\r', zOffset));
        zOffset = MIN(zNextN, zNextR);
        if (zOffset == wstring::npos)
            break;

        // Check for a windows line break
        if (sStr[zOffset] == L'\r' &&
            zOffset + 1 < sStr.length() &&
            sStr[zOffset + 1] == L'\n')
        {
            sStr.replace(zOffset, 2, sBreak);
            continue;
        }

        sStr.replace(zOffset, 1, sBreak);
    }
}


/*====================
  EscapeWhiteSpace
  ====================*/
tstring&    EscapeWhiteSpace(tstring &sStr)
{
    size_t zPos(0);
    for (;;)
    {
        zPos = sStr.find(_T("\\"), zPos);
        if (zPos == tstring::npos)
            break;

        sStr.replace(zPos, 1, _T("\\\\"));
    }

    for (;;)
    {
        size_t zPos(sStr.find(_T("\n")));
        if (zPos == tstring::npos)
            break;

        sStr.replace(zPos, 1, _T("\\n"));
    }

    for (;;)
    {
        size_t zPos(sStr.find(_T("\r")));
        if (zPos == tstring::npos)
            break;

        sStr.replace(zPos, 1, _T("\\r"));
    }

    for (;;)
    {
        size_t zPos(sStr.find(_T("\t")));
        if (zPos == tstring::npos)
            break;

        sStr.replace(zPos, 1, _T("\\t"));
    }

    return sStr;
}


/*====================
  TabPad
  ====================*/
tstring&    TabPad(tstring &sStr, size_t zTabStop, size_t zSize)
{
    if (sStr.length() >= zSize || zTabStop == 0)
        return sStr;
    
    size_t zCount(INT_CEIL((zSize - sStr.length()) / float(zTabStop)));
    for (size_t z(0); z < zCount; ++z)
        sStr += _T("\t");

    return sStr;
}


/*====================
  GetPortFromAddress
  ====================*/
int     GetPortFromAddress(const tstring &sAddr)
{
    // Skip "protocol://" if the string starts with that
    size_t zMarker(sAddr.find(_T("//")));
    if (zMarker == tstring::npos)
        zMarker = 0;

    // Grab the port number from after the last ':'
    zMarker = sAddr.find_last_of(_T(":"), zMarker);
    if (zMarker == tstring::npos)
        return 0;

    return AtoI(sAddr.substr(zMarker + 1));
}


/*====================
  ExtractPortFromAddress
  ====================*/
int     ExtractPortFromAddress(tstring &sAddr)
{
    // Skip "protocol://" if the string starts with that
    size_t zMarker(sAddr.find(_T("//")));
    if (zMarker == tstring::npos)
        zMarker = 0;

    // Grab the port number from after the last ':'
    zMarker = sAddr.find(_T(":"), zMarker);
    if (zMarker == tstring::npos)
        return -1;

    int iPort(AtoI(sAddr.substr(zMarker + 1)));
    sAddr = sAddr.substr(0, zMarker);
    return iPort;
}


/*====================
  GetByteString
  ====================*/
tstring GetByteString(unsigned int z)
{
    if (z < 1024)
        return XtoA(z) + _T(" bytes");
    else if (z < 1024 * 1024)
        return XtoA(z / float(1024), 0, 0, 2) + _T(" KB");
    else if (z < 1024 * 1024 * 1024)
        return XtoA(z / float(1024 * 1024), 0, 0, 2) + _T(" MB");
    else
        return XtoA(z / float(1024 * 1024 * 1024), 0, 0, 2) + _T(" GB");
}

tstring GetByteString(unsigned long z)
{
    if (z < 1024)
        return XtoA(z) + _T(" bytes");
    else if (z < 1024 * 1024)
        return XtoA(z / float(1024), 0, 0, 2) + _T(" KB");
    else if (z < 1024 * 1024 * 1024)
        return XtoA(z / float(1024 * 1024), 0, 0, 2) + _T(" MB");
    else
        return XtoA(z / float(1024 * 1024 * 1024), 0, 0, 2) + _T(" GB");
}

tstring GetByteString(ULONGLONG z)
{
    if (z < 1024)
        return XtoA(z) + _T(" bytes");
    else if (z < 1024 * 1024)
        return XtoA(z / float(1024), 0, 0, 2) + _T(" KB");
    else if (z < 1024 * 1024 * 1024)
        return XtoA(z / float(1024 * 1024), 0, 0, 2) + _T(" MB");
    else
        return XtoA(z / float(1024 * 1024 * 1024), 0, 0, 2) + _T(" GB");
}


/*====================
  Filename_AppendSuffix
  ====================*/
tstring     Filename_AppendSuffix(const tstring &sPath, const tstring &sSuffix)
{
    tstring sWorking(Filename_StripExtension(sPath));
    if (sWorking.empty())
        return TSNULL;

    if (sPath.find(_T('.')) == tstring::npos)
        sWorking += sSuffix;
    else
        sWorking += sSuffix + _T(".") + Filename_GetExtension(sPath);

    return sWorking;
}


/*====================
  GetColorFromString
  ====================*/
CVec4f  GetColorFromString(const tstring &sIn)
{
    // Quick checks
    if (sIn.empty())
        return BLACK;

    if (sIn[0] == _T('^'))
    {
        if (sIn.length() < 4)
        {
            Console.Warn << _T("Invalid color string: ") << sIn << newl;
            return BLACK;
        }

        return CVec4f((sIn[1] - _T('0')) / 9.0f, (sIn[2] - _T('0')) / 9.0f, (sIn[3] - _T('0')) / 9.0f, 1.0f);
    }
    else if (sIn[0] == _T('#'))
    {
        // Make sure this is a valid hex value
        if (sIn.length() < 7)
        {
            Console.Warn << _T("Invalid color string: ") << sIn << newl;
            return BLACK;
        }

        // Set alpha value
        int iRGBA[4] = { 0, 0, 0, 255 };
        if (sIn.length() == 9)
            iRGBA[3] = HexAtoI(sIn.substr(7, 2));
        else if (sIn.length() != 7)
            Console.Warn << _T("Invalid color string: ") << sIn << newl;

        // Read color
        for (int i(0); i < 3; ++i)
            iRGBA[i] = HexAtoI(sIn.substr((i * 2) + 1, 2));

        return CVec4f(iRGBA[0] / 255.0f, iRGBA[1] / 255.0f, iRGBA[2] / 255.0f, iRGBA[3] / 255.0f);
    }

    // Check for names
    tstring sColor(LowerString(sIn));
    
    // Check if this is a player color (PC) or a normal use color
    if (sColor.substr(0, 2) == _T("pc"))
    {
        switch (sColor.length())
        {
        case 6:
            if (sColor == _T("pcblue"))
                return PCBLUE;
            else if (sColor == _T("pcgray"))
                return PCGRAY;              
            else if (sColor == _T("pcpink"))
                return PCPINK;
            else if (sColor == _T("pcteal"))
                return PCTEAL;
            break;

        case 7:
            if (sColor == _T("pcbrown"))
                return PCBROWN;
            break;

        case 8: 
            if (sColor == _T("pcorange"))
                return PCORANGE;
            else if (sColor == _T("pcpurple"))
                return PCPURPLE;
            else if (sColor == _T("pcyellow"))
                return PCYELLOW;
            break;
            
        case 11:    
            if (sColor == _T("pcdarkgreen"))
                return PCDARKGREEN;
            else if (sColor == _T("pclightblue"))
                return PCLIGHTBLUE;
                
        default:
            return WHITE;
        }   
    }
    else
    {   
        switch (sColor.length())
        {
        case 3:
            if (sColor == _T("red"))
                return RED;
            break;

        case 4:
            if (sColor == _T("aqua") || sColor == _T("cyan"))
                return CYAN;
            else if (sColor == _T("gray"))
                return GRAY;
            else if (sColor == _T("navy"))
                return NAVY;
            else if (sColor == _T("teal"))
                return TEAL;
            else if (sColor == _T("blue"))
                return BLUE;
            else if (sColor == _T("lime"))
                return LIME;
            break;

        case 5:
            if (sColor == _T("black"))
                return BLACK;
            else if (sColor == _T("brown"))
                return BROWN;
            else if (sColor == _T("green"))
                return GREEN;
            else if (sColor == _T("olive"))
                return OLIVE;
            else if (sColor == _T("white"))
                return WHITE;
            break;

        case 6:
            if (sColor == _T("silver"))
                return SILVER;
            else if (sColor == _T("purple"))
                return PURPLE;
            else if (sColor == _T("maroon"))
                return MAROON;
            else if (sColor == _T("yellow"))
                return YELLOW;
            else if (sColor == _T("orange"))
                return ORANGE;
            break;

        case 7:
            if (sColor == _T("fuchsia") || sColor == _T("magenta"))
                return MAGENTA;
            break;

        case 9:
            if (sColor == _T("invisible"))
                return CLEAR;
            break;
        }
    }

    // Last resort, try a vector
    tsvector vColor(TokenizeString(sIn, _T(' ')));
    if (vColor.empty())
        return WHITE;

    return CVec4f(
        AtoF(vColor[0]),
        vColor.size() > 1 ? AtoF(vColor[1]) : 0.0f,
        vColor.size() > 2 ? AtoF(vColor[2]) : 0.0f,
        vColor.size() > 3 ? AtoF(vColor[3]) : 1.0f);
}


/*====================
  AddEscapeChars
  ====================*/
tstring AddEscapeChars(const tstring &sIn)
{
    tstring sRet;
    tstring::const_iterator it(sIn.begin());

    while (it != sIn.end())
    {
        switch (*it)
        {
            case _T('"'):
            case _T('\\'):
            case _T('$'):
                sRet += _T('\\');
                sRet += *it;
                break;
            default:
                sRet += *it;
                break;
        }

        ++it;
    }

    return sRet;
}


/*====================
  AddUIEscapeChars
  ====================*/
tstring AddUIEscapeChars(const tstring &sIn)
{
    tstring sRet;
    tstring::const_iterator it(sIn.begin());

    while (it != sIn.end())
    {
        switch (*it)
        {
            case _T('\''):
            case _T('\\'):
                sRet += _T('\\');
                sRet += *it;
                break;
            default:
                sRet += *it;
                break;
        }

        ++it;
    }

    return sRet;
}


/*====================
  Format

  C-style formatted strings
  ====================*/
tstring Format(const TCHAR *sz, ...)
{
    static TCHAR szBuffer[8192];
    
    va_list argptr;

    va_start(argptr, sz);

#ifdef USE_SECURE_CRT
    if (_vsntprintf_s(szBuffer, 8192, 8191, sz, argptr) == -1)
#else
    if (_vsntprintf(szBuffer, 8192, sz, argptr) >= 8192)
#endif
        szBuffer[8191] = 0;

    va_end(argptr);

    return tstring(szBuffer);
}


/*====================
  ProcessValueStrings
  ====================*/
tstring     ProcessValueStrings(const tstring &sStr, TCHAR chToken)
{
    tstring sOut;
    tstring sValueString;

    for (tstring::const_iterator it(sStr.begin()); it != sStr.end(); ++it)
    {
        if (*it == chToken)
        {
            ++it;
            if (it != sStr.end() && *it == chToken)
            {
                sOut += chToken;
                continue;
            }
            else
            {
                sValueString.clear();

                for (; it != sStr.end() && *it != chToken; ++it)
                    sValueString += *it;

                if (it == sStr.end())
                    break;

                if (sValueString.empty())
                    continue;

                sOut += Cmd_GetObjectValueString2(sValueString);
                continue;
            }
        }

        sOut += *it;
    }

    return sOut;
}


/*====================
  IsValidURLChar
  ====================*/
bool    IsValidURLChar(TCHAR c)
{
    if (c >= _T('A') && c <= _T('Z'))
        return true;
    if (c >= _T('a') && c <= _T('z'))
        return true;
    if (c >= _T('0') && c <= _T('9'))
        return true;
    if (c == _T('_') || c == _T('.') || c == _T('-'))
        return true;

    return false;
}


/*====================
  URLEncode
  ====================*/
string  URLEncode(const string &sMessage, bool bAllowSlashes, bool bEncodeSpaces)
{
    string sNewMessage;
    uint uStringPos(0);

    while (uStringPos < sMessage.length())
    {
        if (!IsValidURLChar(sMessage[uStringPos]))
        {
            if ((sMessage[uStringPos] == '/' || sMessage[uStringPos] == '\\') && bAllowSlashes)
                sNewMessage = sNewMessage + sMessage[uStringPos];
            else if (sMessage[uStringPos] == ' ' && !bEncodeSpaces)
                sNewMessage = sNewMessage + "+";
            else
                sNewMessage = sNewMessage + "%" + (BYTE_HEX_STR(sMessage[uStringPos]).substr(2,2));

            uStringPos++;
        }
        else
            sNewMessage = sNewMessage + sMessage[uStringPos++];
    }

    return sNewMessage;
}


/*====================
  URLDecode
  ====================*/
string  URLDecode(const string &sMessage)
{
    string sNewMessage;

    for (size_t z(0); z < sMessage.length(); ++z)
    {
        if (sMessage[z] == _T('%'))
        {
            sNewMessage += char(HexAtoI(sMessage.substr(z + 1, 2)));
            z += 2;
            continue;
        }

        if (sMessage[z] == _T('+'))
            sNewMessage += _T(' ');
        else
            sNewMessage += sMessage[z];
    }

    return sNewMessage;
}

/*====================
  WrapStringCount
  ====================*/
float   WrapStringCount(const tstring &sStr, CFontMap *pFontMap, float fWidth, bool bWrap, uivector *vWrappingBreakListOut, int iFlags, fvector *vCentering)// tsvector &vsOut, const CVec4f *pColor, ColorVector *pColors)
{
    if(sStr.length() == 0 || pFontMap == 0)
        return 1.0f;

    if(vWrappingBreakListOut != nullptr)
        vWrappingBreakListOut->clear();
    
    if(vCentering != nullptr)
        vCentering->clear();

        //Used for String Wrapping
    //const int DRAW_STRING_WRAP            (BIT(0));
    //const int DRAW_STRING_ANCHOR_BOTTOM   (BIT(1));
    const int DRAW_STRING_CENTER        (BIT(2));
    //const int DRAW_STRING_VCENTER     (BIT(3));
    //const int DRAW_STRING_SMILEYS     (BIT(4));
    //const int DRAW_STRING_NOCOLORCODES    (BIT(5));
    const int DRAW_STRING_RIGHT         (BIT(6));
    //const int DRAW_STRING_BOTTOM      (BIT(7));

    size_t  zLastStrPos(0);
    size_t  zStrPos(0);
    size_t  zLastStrBreak(0);
    float   fWrapCount(0.0f);
    float   fCurrentWidth(0.0f);

    for (zStrPos = 0; zStrPos < sStr.length(); zStrPos++)
    {
        tstring sSubStr(sStr.substr(zLastStrPos, (zStrPos - zLastStrPos) + 1));
        fCurrentWidth = pFontMap->GetStringWidth(sSubStr);

        if (sStr[zStrPos] == '\n')
        {   
            if(vWrappingBreakListOut != nullptr)
                vWrappingBreakListOut->push_back(zStrPos);
            if (vCentering != nullptr && (iFlags & DRAW_STRING_CENTER || iFlags & DRAW_STRING_RIGHT))
                vCentering->push_back(fCurrentWidth);
            fWrapCount++;
            zLastStrPos = zLastStrBreak  = zStrPos + 1;
        }
        else if (fCurrentWidth > fWidth && bWrap)
        {
            if(vWrappingBreakListOut != nullptr)
                vWrappingBreakListOut->push_back(zLastStrBreak);
            if (vCentering != nullptr && (iFlags & DRAW_STRING_CENTER || iFlags & DRAW_STRING_RIGHT))
            {
                sSubStr = sStr.substr(zLastStrPos, (zLastStrBreak - zLastStrPos) + 1);
                fCurrentWidth = pFontMap->GetStringWidth(sSubStr);
                vCentering->push_back(fCurrentWidth);
            }
            fWrapCount++;
            zLastStrPos = zLastStrBreak + 1;
        }
        else if (sStr[zStrPos] == ' ' || sStr[zStrPos] == '\t')
        {
            zLastStrBreak = zStrPos;
        }
    }
    
    if (zLastStrPos != sStr.length())
    {
        tstring sSubStr(sStr.substr(zLastStrPos));
        fCurrentWidth = pFontMap->GetStringWidth(sSubStr);
        fWrapCount++;
        if (vCentering != nullptr && (iFlags & DRAW_STRING_CENTER || iFlags & DRAW_STRING_RIGHT))
            vCentering->push_back(fCurrentWidth);
    }

    return fWrapCount;
}

/*====================
  float BiggestStringWidth
  ====================*/
float       BiggestStringWidth(const tstring &sStr, CFontMap *pFontMap, float fWidth)
{
    if(sStr.length() == 0 || pFontMap == 0)
        return 1.0f;

    float fBiggestWidth = pFontMap->GetMaxAdvance();

    size_t  zLastStrPos(0);
    size_t  zStrPos(0);
    size_t  zLastStrBreak(0);
    float   fCurrentWidth(0.0f);

    for (zStrPos = 0; zStrPos < sStr.length(); zStrPos++)
    {
        fCurrentWidth = pFontMap->GetStringWidth(sStr.substr(zLastStrPos, (zStrPos - zLastStrPos) + 1));

        if(fBiggestWidth < fCurrentWidth)
                fBiggestWidth = fCurrentWidth;

        if (sStr[zStrPos] == '\n')
            zLastStrPos = zLastStrBreak  = zStrPos + 1;
        //else if (fCurrentWidth > fWidth)
        //  zLastStrPos = zLastStrBreak + 1;
        //else if (sStr[zStrPos] == ' ' || sStr[zStrPos] == '\t')
        //  zLastStrBreak = zStrPos;
    }
    
    if (zLastStrPos != sStr.length())
        if(fBiggestWidth < fCurrentWidth)
                fBiggestWidth = fCurrentWidth;

    return fBiggestWidth;
}

/*====================
  tsvector WrapString
  ====================*/
tsvector    WrapString(const tstring &sStr, CFontMap *pFontMap, float fWidth, bool bWrap, const CVec4f *pColor, ColorVector *pColors)
{
    tsvector vsOut;
    vsOut.clear();

    if (sStr.length() == 0 || pFontMap == 0)
    {
        vsOut.push_back(TSNULL);
        return vsOut;
    }

    size_t  zLastStrPos(0);
    size_t  zStrPos(0);
    size_t  zLastStrBreak(0);

    for (zStrPos = 0; zStrPos < sStr.length(); ++zStrPos)
    {
        float fCurrentWidth = pFontMap->GetStringWidth(sStr.substr(zLastStrPos, (zStrPos - zLastStrPos) + 1));

        if (sStr[zStrPos] == '\n')
        {   
            vsOut.push_back (sStr.substr(zLastStrPos, (zStrPos - zLastStrPos) + 1));
            if (pColors != nullptr)
                    pColors->push_back(*pColor);
            zLastStrPos = zLastStrBreak  = zStrPos + 1;
        }
        else if (fCurrentWidth > fWidth && bWrap)
        {
            vsOut.push_back(sStr.substr(zLastStrPos, (zLastStrBreak - zLastStrPos) + 1));
            if (pColors != nullptr)
                pColors->push_back(*pColor);
            zLastStrPos = zLastStrBreak + 1;
        }
        else if ((sStr[zStrPos] == ' ' || sStr[zStrPos] == '\t')  && bWrap)
        {
            zLastStrBreak = zStrPos;
        }
    }
    
    if (zLastStrPos != sStr.length())
    {
        vsOut.push_back(sStr.substr(zLastStrPos));
        if (pColors != nullptr)
            pColors->push_back(*pColor);
    }

    return vsOut;
}

//CVAR_BOOL(_strQuickReject,    true);
//CVAR_BOOL(_strReject,     true);
//CVAR_BOOL(_strBreakPredict,   true);
/*====================
  WrapString
  ====================*/
void    WrapString(const tstring &sStr, CFontMap *pFontMap, float fWidth, tsvector &vsOut, const CVec4f *pColor, ColorVector *pColors)
{
    if (sStr.length() == 0 || fWidth < pFontMap->GetMaxAdvance())
        return;

    // Do a simple worst case check to quickly reject some strings
    //if (_strQuickReject)
    {
        if (pFontMap->GetMaxAdvance() * sStr.length() < fWidth)
        {
            vsOut.push_back(sStr);
            if(pColor != nullptr)
                pColors->push_back(*pColor);
            return;
        }
    }

    // Check for short strings that will fit
    if (pFontMap->GetStringWidth(sStr) < fWidth)
    {
        vsOut.push_back(sStr);
        if(pColor != nullptr)
            pColors->push_back(*pColor);
        return;
    }

    // Search backwards from the character that would overflow in a worst case
    // scenario (all characters are maximum width) for the first white space
    // character, to avoid some unnecesary comparisons
    size_t zStartSearch(1);//INT_FLOOR(fWidth / pFontMap->GetMaxAdvance()));
    //if (_strBreakPredict)
    {
        zStartSearch = INT_FLOOR(fWidth / pFontMap->GetMaxAdvance());
        for ( ; zStartSearch != 0; --zStartSearch)
        {
            if (IsTokenSeparator(sStr[zStartSearch]) && !IsTokenSeparator(sStr[zStartSearch - 1]))
                break;
        }
    }

    // Find a good place to break
    size_t zBreakPos(0);
    size_t zOverflowPos(0);
    float fCurrentWidth(pFontMap->GetStringWidth(sStr.substr(0, zStartSearch)));
    for (size_t z(zStartSearch); z < sStr.length(); ++z)
    {
        if (z == 0 || (IsTokenSeparator(sStr[z]) && !IsTokenSeparator(sStr[z - 1])))
            zBreakPos = z;

        fCurrentWidth += pFontMap->GetCharMapInfo(sStr[z])->m_fAdvance;
        if (fCurrentWidth >= fWidth)
        {
            zOverflowPos = z - 1;
            break;
        }
    }

    if (zBreakPos == 0)
    {
        // Didn't find any white space to break on, so just split mid-token
        vsOut.push_back(sStr.substr(0, zOverflowPos));
        if (pColors != nullptr)
            pColors->push_back(*pColor);
        WrapString(sStr.substr(zOverflowPos + 1), pFontMap, fWidth, vsOut, pColor, pColors);
    }
    else
    {
        // Break in the middle of some white space
        vsOut.push_back(sStr.substr(0, zBreakPos));
        if (pColors != nullptr)
            pColors->push_back(*pColor);
        WrapString(sStr.substr(zBreakPos + 1), pFontMap, fWidth, vsOut, pColor, pColors);
    }
}


/*====================
  ReplaceTokens
  ====================*/
tstring ReplaceTokens(tstring &sStr, const tsmapts &mapTokens)
{
    if (mapTokens.empty())
        return sStr;

    size_t zOffset(0);
    while (zOffset != tstring::npos)
    {
        size_t zStart(sStr.find(_T('{'), zOffset));
        if (zStart == tstring::npos)
            break;
        size_t zEnd(sStr.find(_T('}'), zStart));
        if (zEnd == tstring::npos)
            break;

        // Default parameter
        size_t zMid(sStr.find(_T('='), zStart));
        if (zMid < zEnd)
        {
            const tstring &sToken(sStr.substr(zStart + 1, zMid - zStart - 1));
            tsmapts_cit itFind(mapTokens.find(sToken));

            if (itFind != mapTokens.end())
            {
                const tstring &sValue(itFind->second);
                zOffset = zStart + sValue.length();
                sStr.replace(zStart, zEnd - zStart + 1, sValue);
            }
            else
            {
                const tstring &sValue(sStr.substr(zMid + 1, zEnd - zMid - 1));
                zOffset = zStart + sValue.length();
                sStr.replace(zStart, zEnd - zStart + 1, sValue);
            }
            continue;
        }

        const tstring &sToken(sStr.substr(zStart + 1, zEnd - zStart - 1));

        tsmapts_cit itFind(mapTokens.find(sToken));
        const tstring &sValue(itFind == mapTokens.end() ? TSNULL : itFind->second);
        zOffset = zStart + sValue.length();
        sStr.replace(zStart, zEnd - zStart + 1, sValue);
    }

    return sStr;
}


/*====================
  TrimLeft
  ====================*/
tstring TrimLeft(const tstring& sString, const tstring& sChars)
{
    tstring sStr(sString);
    tstring::size_type pos = sStr.find_first_not_of(sChars);
    sStr.erase(0, pos);
    return sStr;
}


/*====================
  TrimRight
  ====================*/
tstring TrimRight(const tstring& sString, const tstring& sChars)
{
    tstring sStr(sString);
    tstring::size_type pos = sStr.find_last_not_of(sChars);
    sStr.erase(pos + 1);
    return sStr;
}


/*====================
  Trim
  ====================*/
tstring Trim(const tstring& sStr, const tstring& sChars)
{
   return TrimLeft(TrimRight(sStr, sChars), sChars);
}


/*--------------------
  TrimLeft
  --------------------*/
UI_CMD(TrimLeft, 1)
{
    if (vArgList.size() < 1)
        return TSNULL;

    return TrimLeft(vArgList[0]->Evaluate());
}


/*--------------------
  TrimRight
  --------------------*/
UI_CMD(TrimRight, 1)
{
    if (vArgList.size() < 1)
        return TSNULL;

    return XtoA(TrimRight(vArgList[0]->Evaluate()));
}


/*--------------------
  Trim
  --------------------*/
UI_CMD(Trim, 1)
{
    if (vArgList.size() < 1)
        return TSNULL;

    return XtoA(Trim(vArgList[0]->Evaluate()));
}


/*====================
  StringReplace
  ====================*/
tstring StringReplace(tstring sString, const tstring sSearch, const tstring sReplace, const int iReplaceFlag)
{
    tstring::size_type pos = tstring::npos;
    
    if (iReplaceFlag == 0)
    {       
        // replace all occurences of the search string
        pos = sString.find(sSearch);

        while (pos != tstring::npos) 
        {
            sString.replace(pos, sSearch.size(), sReplace);
            pos = sString.find(sSearch, pos + sSearch.size());
        }
       
        return sString;
    }
    else if (iReplaceFlag == 1)
    {
        // replace only the first occurence
        if((pos = sString.find(sSearch)) != tstring::npos)
            return sString.replace(pos, sSearch.length(), sReplace);
        else
            return sString;
    }
    else
        return TSNULL;      
}


/*====================
  StrSubstitute
  ====================*/
static bool     StrSubstitute(tstring& sStr, size_t uiFindPos, size_t uiFindLen, const tstring &sReplace)
{
    if (uiFindPos == tstring::npos)
        return false;

    sStr = sStr.substr(0, uiFindPos).append(sReplace).append(sStr.substr(uiFindPos + uiFindLen));
    return true;
}


/*====================
  StrReplace
  ====================*/
uint    StrReplace(tstring& sStr, const tstring &sSearch, const tstring &sReplace, const uint uiReplaceFlags)
{
    uint uiReplacements(0);

    if ((uiReplaceFlags & STR_REPLACE_ALL) != 0)
    {
        while (StrSubstitute(sStr, sStr.find(sSearch), sSearch.size(), sReplace))
            ++uiReplacements;
    }
    else
    {
        if ((uiReplaceFlags & STR_REPLACE_FIRST) != 0)
        {
            if (StrSubstitute(sStr, sStr.find(sSearch), sSearch.size(), sReplace))
                ++uiReplacements;
        }

        if ((uiReplaceFlags & STR_REPLACE_LAST) != 0)
        {
            if (StrSubstitute(sStr, sStr.find_last_of(sSearch), sSearch.size(), sReplace))
                ++uiReplacements;
        }
    }

    return uiReplacements;
}


/*====================
  StringReplace
  ====================*/
bool    EndsWith(const tstring &sStr, const tstring &sEnding)
{
    if (sStr.size() < sEnding.size())
        return false;

    for (size_t i = sStr.size() - sEnding.size(), j = 0; i < sStr.size(); ++i, ++j)
    {
        char cA(sStr[i]);
        char cB(sEnding[j]);
        if (cA != cB)
            return false;
    }

    return true;
}


#if 1
/*====================
  ConvertLineEndings
  ====================*/
K2_API uint         ConvertLineEndings(char *pOutBuf, const char *pInBuf, uint uiInBufSize)
{
    char *pOut(pOutBuf);
    const char *pCur(pInBuf);
    const char *pEnd(pInBuf + uiInBufSize);
    while (pCur != pEnd)
    {
        // skip all '\r'
        if (*pCur == '\r')
        {
            ++pCur;
            continue;
        }

        *pOut = *pCur;
        ++pCur;
        ++pOut;
    }
    return (pOut - pOutBuf);
}
#else
/*====================
  ConvertLineEndings
  ====================*/
K2_API uint         ConvertLineEndings(char *pOutBuf, const char *pInBuf, uint uiInBufSize)
{
    // skip all '\r'
    int iFindCharSize(1);
    char pFindChar[2] = { '\r', 0 };

    // detect UTF-16.
    if (uiInBufSize >= 2)
    {
        if (pInBuf[0] == 0xFF && pInBuf[1] == 0xFE)
        {
            pFindChar[0] = 0x0D;
            pFindChar[1] = 0x00;
            iFindCharSize = 2;
        }
        if (pInBuf[0] == 0xFE && pInBuf[1] == 0xFF)
        {
            pFindChar[0] = 0x00;
            pFindChar[1] = 0x0D;
            iFindCharSize = 2;
        }
    }

    char *pOut(pOutBuf);
    const char *pCur(pInBuf);
    const char *pEnd(pInBuf + uiInBufSize);

    switch (iFindCharSize)
    {
    case 1:
        {
            while (pCur != pEnd)
            {
                // skip all '\r'
                if (*pCur == '\r')
                {
                    ++pCur;
                    continue;
                }

                *pOut = *pCur;
                ++pCur;
                ++pOut;
            }
        }
        break;

    case 2:
        {
            // we're using UTF-16, but somehow the input buffer has an odd number of bytes.
            // Ignore the last byte.
            if (size_t(pEnd - pCur) % 2 == 1)
            {
                pEnd -= 1;
            }

            while (pCur != pEnd)
            {
                // skip all '\r'
                if (pCur[0] == pFindChar[0] &&
                    pCur[1] == pFindChar[1])
                {
                    pCur += iFindCharSize;
                    continue;
                }

                pOut[0] = pCur[0];
                pOut[1] = pCur[1];
                pCur += 2;
                pOut += 2;
            }
        }
        break;

    default:
        assert(!"Programmer error");
        MemManager.Copy(pOutBuf, pInBuf, uiInBufSize);
        return uiInBufSize;
    }

    return (pOut - pOutBuf);
}
#endif


/*--------------------
  StringReplace
  --------------------*/
UI_CMD(StringReplace, 2)
{
    if (vArgList.size() < 2)
        return TSNULL;
    
    tstring sString(vArgList[0]->Evaluate());
    tstring sSearch(vArgList[1]->Evaluate());
    tstring sReplace(vArgList[2]->Evaluate());
    
    int iReplaceFlag(0);
    
    if (vArgList.size() == 4)
        iReplaceFlag = AtoI(vArgList[3]->Evaluate());
    
    return StringReplace(sString, sSearch, sReplace, iReplaceFlag);
}


/*--------------------
  StrCmp
  --------------------*/
CMD(StrCmp)
{
    if (vArgList.size() < 3)
    {
        Console << _T("Syntax: StrCcmp <string 1> <string 2> <result variable>") << newl;
        return false;
    }

    ICvar *pCvar(ICvar::Find(vArgList[2]));
    if (pCvar == nullptr)
    {
        Console << _T("Unknown cvar: ") << vArgList[2] << newl;
        return false;
    }

    pCvar->SetBool(CompareNoCase(vArgList[0], vArgList[1]) == 0);
    return true;
}

/*--------------------
  StringEquals
  --------------------*/
FUNCTION(StringEquals)
{
    if (vArgList.size() < 2)
        return _T("0");

    return XtoA(CompareNoCase(vArgList[0], vArgList[1]) == 0, true);
}


/*--------------------
  StringLength
  --------------------*/
FUNCTION(StringLength)
{
    if (vArgList.size() < 1)
        return TSNULL;

    return XtoA(INT_SIZE(vArgList[0].length()));
}


/*--------------------
  Substring
  --------------------*/
FUNCTION(Substring)
{
    if (vArgList.size() < 3)
        return TSNULL;

    tstring sString(vArgList[0]);
    uint uiPos(AtoI(vArgList[1]));
    uint uiLen(AtoI(vArgList[2]));

    if (sString.length() < uiPos)
        return TSNULL;

    return sString.substr(uiPos, uiLen);
}


/*--------------------
  SearchString
  --------------------*/
FUNCTION(SearchString)
{
    if (vArgList.size() < 3)
        return TSNULL;

    tstring sString(vArgList[0]);
    tstring sFind(vArgList[1]);
    uint uiPos(AtoI(vArgList[2]));

    if (sString.length() < uiPos)
        return _T("-1");

    return XtoA(int(sString.find(sFind, uiPos)));
}


/*--------------------
  LowerString
  --------------------*/
FUNCTION(LowerString)
{
    if (vArgList.size() < 1)
        return TSNULL;

    return LowerString(vArgList[0]);
}


/*--------------------
  UpperString
  --------------------*/
FUNCTION(UpperString)
{
    if (vArgList.size() < 1)
        return TSNULL;

    return UpperString(vArgList[0]);
}


/*--------------------
  StripChar
  --------------------*/
FUNCTION(StripChar)
{
    if (vArgList.size() < 2)
        return TSNULL;

    tstring sNewString(ConcatinateArgs(vArgList.begin() + 1, vArgList.end()));

    size_t i(0);

    while (i < sNewString.length())
    {
        if (vArgList[0].find(sNewString[i]) != tstring::npos)
            sNewString.erase(i);
        else
            i++;
    }

    return sNewString;
}


/*--------------------
  StringContains
  --------------------*/
UI_CMD(StringContains, 2)
{
    if (vArgList[0]->Evaluate().find_first_of(vArgList[1]->Evaluate()) != tstring::npos)
        return _T("true");

    return _T("false");
}


/*--------------------
  StringContainsOnly
  --------------------*/
UI_CMD(StringContainsOnly, 2)
{
    tstring sSearch(vArgList[1]->Evaluate());
    if (vArgList.size() > 2)
    {
        const tstring &sFlags(vArgList[2]->Evaluate());
        if (sFlags.find(_T('a')) != tstring::npos)
            sSearch += _T("abcdefghijklmnopqrstuvwxyz");
        if (sFlags.find(_T('A')) != tstring::npos)
            sSearch += _T("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        if (sFlags.find(_T('0')) != tstring::npos)
            sSearch += _T("0123456789");
    }

    if (vArgList[0]->Evaluate().find_first_not_of(sSearch) == tstring::npos)
        return _T("true");

    return _T("false");
}


/*--------------------
  IsEmailAddress
  --------------------*/
UI_CMD(IsEmailAddress, 1)
{
    if (IsEmailAddress(vArgList[0]->Evaluate()))
        return _T("true");

    return _T("false");
}


/*--------------------
  IsValidCCNumber
  --------------------*/
UI_CMD(IsValidCCNumber, 1)
{
    if (IsValidCCNumber(vArgList[0]->Evaluate()))
        return _T("true");

    return _T("false");
}


/*--------------------
  ExplodeString
  --------------------*/
tsvector ExplodeString(tstring sList, tstring sSeperator)
{
    tsvector vOut;

    vOut.clear();

    tstring sNameTmp(_T(""));
    tstring sValueTmp(_T(""));

    uint    iFoundTimes(1);
    size_t  stLastPosition(0);
    size_t  stPosition(sList.find(sSeperator, 0));

    while (iFoundTimes < sList.length() + 1)
    {
        sValueTmp = sList.substr(stLastPosition, (stPosition == -1) ? (sList.length() - stLastPosition) : (stPosition - stLastPosition));

        vOut.push_back(sValueTmp);

        if (stPosition == -1)
            break;

        stLastPosition = stPosition + sSeperator.length();
        stPosition = sList.find(sSeperator, stPosition + sSeperator.length());
        iFoundTimes++;
    }

    return vOut;
}

/*====================
  QuoteStr
====================*/
K2_API tstring    QuoteStr(const tstring& x)
{
    return _T("\"") + x + _T("\"");
}

K2_API tstring    QuoteStr(const TCHAR *x)
{
    return _T("\"") + tstring(x) + _T("\"");
}

K2_API tstring    QuoteStr(TCHAR *x)
{
    return _T("\"") + tstring(x) + _T("\"");
}

/*====================
  ParenStr
====================*/
K2_API tstring    ParenStr(const tstring& x)
{
    return _T("(") + x + _T(")");
}


/*====================
  SingleQuoteStr
====================*/
K2_API tstring    SingleQuoteStr(const tstring& x)
{
    return _T("'") + XtoA(x) + _T("'");
}
