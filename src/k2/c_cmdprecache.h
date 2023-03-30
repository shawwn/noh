// (C)2007 S2 Games
// c_cmdprecache.h
//
//=============================================================================
#ifndef __C_CMDPRECACHE_H__
#define __C_CMDPRECACHE_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_consoleelement.h"
//=============================================================================

//#ifdef K2_PROFILE // START K2_PROFILE
#if 0
#define CMD(name) \
bool    _cmd##name##_PrecacheFn(CConsoleElement *pElem, const tsvector &vArgList);\
bool    cmd##name##_PrecacheFn(CConsoleElement *pElem, const tsvector &vArgList);\
CCmdPrecache cmd##name##_Precache(_T(#name), cmd##name##_PrecacheFn);\
bool    _cmd##name##_PrecacheFn(CConsoleElement *pElem, const tsvector &vArgList)
{\
    PROFILE(_T(#name) + _T("_Precache"));\
    return cmd##name##_PrecacheFn(pElem, vArgList);\
}\
bool    cmd##name##_PrecacheFn(CConsoleElement *pElem, const tsvector &vArgList)

#else   // NOT K2_PROFILE

#define CMD_PRECACHE(name) \
bool    cmd##name##_PrecacheFn(CConsoleElement *pElem, const tsvector &vArgList);\
CCmdPrecache cmd##name##_Precache(_T(#name), cmd##name##_PrecacheFn);\
bool    cmd##name##_PrecacheFn(CConsoleElement *pElem, const tsvector &vArgList)
#endif

//=============================================================================
// CCmdPrecache
//=============================================================================
class K2_API CCmdPrecache : public CConsoleElement
{
private:

public:
    CCmdPrecache(const tstring &sName, ConsoleElementFn_t pfnCmd);
    ~CCmdPrecache();

    bool    operator()(const tsvector &vArgList);

    bool    operator()(tstring s0 = _T(""), tstring s1 = _T(""), tstring s2 = _T(""), tstring s3 = _T(""),
                    tstring s4 = _T(""), tstring s5 = _T(""), tstring s6 = _T(""), tstring s7 = _T(""),
                    tstring s8 = _T(""), tstring s9 = _T(""));

    tstring GetString() const       { return _T(""); }
};
//=============================================================================
#endif // __C_CMDPRECACHE_H__
