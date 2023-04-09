// (C)2005 S2 Games
// stringutils.h
//
// A class of static member functions to perform various data manipulation
//=============================================================================
#ifndef __STRINGUTILS_H__
#define __STRINGUTILS_H__

//=============================================================================
// Definitions
//=============================================================================
class CFontMap;
typedef vector<CVec4f> ColorVector;

typedef list<tstring>   StringList;

const int NUM_TOK_STRINGS = 16;

const TCHAR SPACE(_T(' '));

const tstring   WHITESPACE(_T(" \f\n\r\t\v"));
const tstring   SEARCH_PIPES(_T("\\|"));
const tstring   SEARCH_HYPHENS(_T("\\-"));
const tstring   REPLACE_PIPES(_T("\\!*@&#^$%"));
const tstring   REPLACE_HYPHENS(_T("\\%$^#&@*!"));

typedef pair<size_t, CVec4f>    StrColorKey;
typedef vector<StrColorKey>     StrColorMap;

//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
K2_API tsvector TokenizeString(const tstring &sStr, TCHAR cSeperator);

K2_API tstring      JoinString(tsvector &vStrings, const tstring &sJoinBy);

template <class T> T&   LowerStringRef(T &str)      { if (!str.empty()) transform(str.begin(), str.end(), str.begin(), tolower); return str; }
template <class T> T    LowerString(const T &str)   { T ret(str); return LowerStringRef(ret); }

template <class T> T&   UpperStringRef(T &str)      { if (!str.empty()) transform(str.begin(), str.end(), str.begin(), toupper); return str; }
template <class T> T    UpperString(const T &str)   { T ret(str); return UpperStringRef(ret); }

K2_API bool         IsLineSeparator(int c);
K2_API bool         IsEOL(int c);
K2_API bool         IsTokenSeparator(int c);
K2_API bool         IsAllWhiteSpace(const tstring &sStr);

K2_API tstring      Filename_GetExtension(const tstring &sPath);
K2_API tstring      Filename_GetName(const tstring &sPath);
K2_API tstring      Filename_GetPath(const tstring &sPath);
K2_API tstring      Filename_StripExtension(const tstring &sPath);
K2_API tstring      Filename_StripPath(const tstring &sPath);
K2_API tstring      Filename_AppendSuffix(const tstring &sPath, const tstring &sSuffix);

K2_API tstring      Upgrade_GetType(const tstring &sProductCode);
K2_API tstring      Upgrade_GetName(const tstring &sProductCode);

K2_API char*        GetNextWord(const char *str);
K2_API tstring      GetNextWord(const tstring &sStr, size_t &zStart);
K2_API tstring      GetNextSymbol(const tstring &sStr, size_t &zStart);
K2_API char*        StripEOL(char *str);
K2_API char*        StrChrBackwards (const char *str, int c);
K2_API TCHAR*       SkipSpaces(TCHAR *str);
K2_API size_t       SkipSpaces(const tstring &sStr, size_t zStart = 0);
K2_API char*        FirstToken(const char *str);
K2_API int          SplitArgs(TCHAR *s, TCHAR **argv, int maxargs);
K2_API int          SplitArgs(const tstring &sString, tsvector &vArgList);
K2_API tstring      ConcatinateArgs(const tsvector &vArgList, const tstring &sSeperator = _T(" "));
K2_API tstring      ConcatinateArgs(tsvector::const_iterator begin, tsvector::const_iterator end, const tstring &sSeperator = _T(" "));

K2_API tstring      StripColorCodes(const tstring &sStr);
K2_API tstring      StripColorCodes(const tstring &sStr, StrColorMap &vColors);

K2_API tstring      StripDuplicateSpaces(const tstring &sStr);
K2_API tstring      StripStartingSpaces(const tstring &sStr);

K2_API tstring      GetLastColorCode(const tstring &sStr);

template <class T> int  _CompareNoCase(const T &s1, const T &s2)
{
    typename T::const_iterator it1(s1.begin()), it1End(s1.end());
    typename T::const_iterator it2(s2.begin()), it2End(s2.end());

    while (it1 != it1End && it2 != it2End)
    {
        if (tolower(*it1) < tolower(*it2))
            return -1;
        else if (tolower(*it1) > tolower(*it2))
            return 1;

        ++it1;
        ++it2;
    }

    return it1 == it1End ? (it2 == it2End ? 0 : -1) : 1;
}
inline int  CompareNoCase(const string &s1, const string &s2)   { return _CompareNoCase(s1, s2); }
inline int  CompareNoCase(const wstring &s1, const wstring &s2) { return _CompareNoCase(s1, s2); }

inline bool FastCompare(const tstring &s1, const tstring &s2)   
{
    if(s1.length() != s2.length())
        return false;

    for(size_t zStrPos(0); zStrPos < s1.length(); ++zStrPos)
    {
        if(s1[zStrPos] != s1[zStrPos])
            return false;
    }
    return true;
};

K2_API int          CompareNum(const tstring &s1, const tstring &s2, size_t n);
K2_API int          CompareNum(const tstring &s1, const TCHAR *s2, size_t n);
K2_API int          CompareNoCaseNum(const TCHAR *p1, const TCHAR *p2, size_t n);
inline int          CompareNoCaseNum(const tstring &s1, const tstring &s2, size_t n) { return CompareNoCaseNum(s1.c_str(), s2.c_str(), n); }

static const uint SPLITBY_ERASE_EMPTY_SPLITS        = BIT(0);
K2_API void         SplitBy(tsvector &vSplits, const tstring &sStr, const tstring &sSeparator, size_t uiOffset = 0, uint uiFlags = 0);

K2_API bool         EqualsWildcard(const tstring &sWild, const tstring &sComp);
K2_API bool         EqualsWildcardNoCase(const tstring &sWild, const tstring &sComp);

// sWild may be in the format *foo*|-*bar* to match any *foo* but not any *bar*
K2_API bool         EqualsWildcards(const tstring &sWild, const tstring &sComp, bool bCaseSensitive = false);

K2_API wstring      SingleToWide(const string &sIn);
K2_API wchar_t*     SingleToWide(wchar_t *out, const char *in, size_t len);
K2_API string       WideToSingle(const wstring &sIn);
K2_API char*        WideToSingle(char *out, const wchar_t *in, size_t len);

K2_API bool         IsUTF16(bool &bOutLittleEndian, const char *in, size_t len);
K2_API bool         IsUTF8(const char *in, size_t len);

K2_API int          UTF16to8(short *instr, int inlen, char *outstr);

K2_API string       UTF8ToString(const string &sIn);
K2_API wstring      UTF8ToWString(const string &sIn);
K2_API string       WStringToUTF8(const wstring &sIn);
K2_API string       StringToUTF8(const string &sIn);

K2_API string       WCSToMBS(const wstring &sIn);
K2_API string       WCSToMBS(const wchar_t *sIn);
K2_API wstring      MBSToWCS(const string &sIn);
K2_API wstring      MBSToWCS(const char *sIn);

K2_API void         StripNewline(tstring &sStr);
K2_API void         NormalizeLineBreaks(string &sStr, const string &sBreak = LINEBREAK);
K2_API void         NormalizeLineBreaks(wstring &sStr, const wstring &sBreak = WLINEBREAK);
inline string       NormalizeLineBreaks(const string &sStr, const string &sBreak = LINEBREAK)       { string sResult(sStr); NormalizeLineBreaks(sResult); return sResult; }
inline wstring      NormalizeLineBreaks(const wstring &sStr, const wstring &sBreak = WLINEBREAK)    { wstring sResult(sStr); NormalizeLineBreaks(sResult); return sResult; }
K2_API tstring&     EscapeWhiteSpace(tstring &sStr);
inline tstring      EscapeWhiteSpace(const tstring &sStr)   { tstring sReturn(sStr); return EscapeWhiteSpace(sReturn); }

K2_API tstring&     TabPad(tstring &sStr, size_t zTabStop, size_t zSize);
inline tstring      TabPad(const tstring &sStr, size_t zTabStop, size_t zSize)  { tstring sReturn(sStr); return TabPad(sReturn, zTabStop, zSize); }

K2_API bool         IsIPAddress(const tstring &sIP);
K2_API int          GetPortFromAddress(const tstring &sAddr);
K2_API int          ExtractPortFromAddress(tstring &sAddr);

K2_API tstring      GetByteString(unsigned int z);
K2_API tstring      GetByteString(unsigned long z);
K2_API tstring      GetByteString(ULONGLONG z);

K2_API CVec4f       GetColorFromString(const tstring &sIn);

K2_API tstring      AddEscapeChars(const tstring &sIn);
K2_API tstring      AddUIEscapeChars(const tstring &sIn);
K2_API tstring      Format(const TCHAR *sz, ...);

K2_API tstring      ProcessValueStrings(const tstring &sStr, TCHAR chToken);

K2_API bool         IsValidURLChar(TCHAR c);
K2_API string       URLEncode(const string &sMessage, bool bAllowSlashes = false, bool bEncodeSpaces = false);
inline string       URLEncode(const wstring &sMessage, bool bAllowSlashes = false, bool bEncodeSpaces = false)  { return URLEncode(WStringToUTF8(sMessage), bAllowSlashes, bEncodeSpaces); }
K2_API string       URLDecode(const string &sMessage);

K2_API bool         IsDigit(int c);
K2_API bool         IsLetter(int c);
K2_API bool         IsIdentifier(int c);
inline bool         IsNotDigit(int c)       { return !IsDigit(c); }
inline bool         IsNotLetter(int c)      { return !IsLetter(c); }
inline bool         IsNotIdentifier(int c)  { return !IsIdentifier(c); }

K2_API bool         IsEmailAddress(const tstring &sAddress);
K2_API bool         IsValidCCNumber(const tstring &sInputNumber);

K2_API void         WrapString(const tstring &sStr, CFontMap *pFontMap, float fWidth, tsvector &vsOut, const CVec4f *pColor = NULL, ColorVector *pColors = NULL);
K2_API tsvector     WrapString(const tstring &sStr, CFontMap *pFontMap, float fWidth, bool bWrap, const CVec4f *pColor = NULL, ColorVector *pColors = NULL);
K2_API float        WrapStringCount(const tstring &sStr, CFontMap *pFontMap, float fWidth, bool bWrap, uivector *vWrappingBreakListOut, int m_iDrawFlags, fvector *vCentering);
K2_API float        BiggestStringWidth(const tstring &sStr, CFontMap *pFontMap, float fWidth);

K2_API tstring      ReplaceTokens(tstring &sStr, const tsmapts &mapTokens);
inline tstring      ReplaceTokens(const tstring &sStr, const tsmapts &mapTokens)    { tstring sReturn(sStr); return ReplaceTokens(sReturn, mapTokens); }

K2_API tstring      TrimLeft(const tstring sStr);
K2_API tstring      TrimRight(const tstring sStr);
K2_API tstring      Trim(const tstring sStr);

K2_API tstring      StringReplace(tstring sString, const tstring sSearch, const tstring sReplace, const int iReplaceFlag = 0);

K2_API tsvector     ExplodeString(tstring sList, tstring sSeperator);

// String Replace Flags
const uint STR_REPLACE_ALL      (BIT(0)); // all instances of 'sSearch' are replaced with 'sReplace'
const uint STR_REPLACE_FIRST    (BIT(1)); // only the first instance of 'sSearch' is replaced with 'sReplace'
const uint STR_REPLACE_LAST     (BIT(2)); // only the last instance of 'sSearch' is replaced with 'sReplace'
K2_API uint         StrReplace(tstring& sStr, const tstring &sSearch, const tstring &sReplace, const uint uiReplaceFlags = STR_REPLACE_ALL);

// returns true if 'sStr' ends with 'sEnding'.
K2_API bool         EndsWith(const tstring &sStr, const tstring &sEnding);

// converts '\r\n' to '\n'
K2_API uint         ConvertLineEndings(char *pOutBuf, const char *pInBuf, uint uiInBufSize);

#define TStringCompare(s, sz) (s).compare(0, (s).length(), sz, (sizeof(sz) / sizeof(TCHAR)) - 1)
#define StringCompare(s, sz) (s).compare(0, (s).length(), sz, sizeof(sz) - 1)
#define WStringCompare(s, sz) (s).compare(0, (s).length(), sz, (sizeof(sz) / sizeof(wchar_t)) - 1)
//=============================================================================

/*====================
  QuoteStr
====================*/
tstring    QuoteStr(const tstring& x);
tstring    QuoteStr(const TCHAR *x);
tstring    QuoteStr(TCHAR *x);


/*====================
  ParenStr
====================*/
tstring    ParenStr(const tstring& x);


/*====================
  SingleQuoteStr
====================*/
tstring    SingleQuoteStr(const tstring& x);


#endif // __STRINGUTILS_H__
