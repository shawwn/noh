// (C)2005 S2 Games
// c_cvararray.h
//
//=============================================================================
#ifndef __C_CVARARRAY_H__
#define __C_CVARARRAY_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"

#include "c_cvar.h"
//=============================================================================

#define CVAR_ARRAY(t, name, e, def)                     CCvarArray<t> name(_T(#name), e, def)
#define CVAR_ARRAYR(t, name, e, def, fl, lo, hi)        CCvarArray<t> name(_T(#name), e, def, fl, lo, hi)
#define CVAR_ARRAYEX(t, name, e, def, flags, lo, hi, fn) CCvarArray<t> name(_T(#name), e, def, flags, lo, hi, fn)

#define CVAR_ARRAY_INT(name, e, def)                    CCvarArray<int> name(_T(#name), e, def)
#define CVAR_ARRAY_INTF(name, e, def, fl)               CCvarArray<int> name(_T(#name), e, def, fl)
#define CVAR_ARRAY_INTR(name, e, def, fl, lo, hi)       CCvarArray<int> name(_T(#name), e, def, fl, lo, hi)
#define CVAR_ARRAY_INTEX(name, e, def, fl, lo, hi, fn)  CCvarArray<int> name(_T(#name), e, def, fl, lo, hi, fn)

#define CVAR_ARRAY_FLOAT(name, e, def)                  CCvarArray<float> name(_T(#name), e, def)
#define CVAR_ARRAY_FLOATF(name, e, def, fl)             CCvarArray<float> name(_T(#name), e, def, fl)
#define CVAR_ARRAY_FLOATR(name, e, def, fl, lo, hi)     CCvarArray<float> name(_T(#name), e, def, fl, lo, hi)
#define CVAR_ARRAY_FLOATEX(name, e, def, fl, lo, hi, fn) CCvarArray<float> name(_T(#name), e, def, lo, hi, fl, fn)

#define CVAR_ARRAY_STRING(name, e, def)                 CCvarArray<tstring> name(_T(#name), e, _T(def))
#define CVAR_ARRAY_STRINGF(name, e, def, fl)            CCvarArray<tstring> name(_T(#name), e, _T(def), fl)
#define CVAR_ARRAY_STRINGEX(name, e, def, fl, fn)       CCvarArray<tstring> name(_T(#name), e, _T(def), fl, fn)

#define CVAR_ARRAY_BOOL(name, e, def)                   CCvarArray<bool> name(_T(#name), e, def)
#define CVAR_ARRAY_BOOLF(name, e, def, fl)              CCvarArray<bool> name(_T(#name), e, def, fl)
#define CVAR_ARRAY_BOOLEX(name, e, def, fl, fn)         CCvarArray<bool> name(_T(#name), e, def, fl, fn)

#define CVAR_ARRAY_VEC3(name, e, def)                   CCvarArray<CVec3f, float> name(_T(#name), e, def)
#define CVAR_ARRAY_VEC3F(name, e, def, fl)              CCvarArray<CVec3f, float> name(_T(#name), e, def, fl)
#define CVAR_ARRAY_VEC3EX(name, e, def, fl, fn)         CCvarArray<CVec3f, float> name(_T(#name), e, def, fl, fn)

//=============================================================================
// CCvarArray
//=============================================================================
template <class T, class T2 = T>
class CCvarArray
{
private:
    CCvar<T, T2>    **m_pCvars;
    int             m_iNumElements;

public:
    ~CCvarArray();

    CCvarArray(const tstring &sName, int iNumElements, T _Default, int iFlags = 0, ConsoleElementFn_t pfnCmd = nullptr);
    CCvarArray(const tstring &sName, int iNumElements, T _Default, int iFlags, T _Min, T _Max, ConsoleElementFn_t pfnCmd = nullptr);

    CCvar<T, T2>&       operator[](int n)       { return *(m_pCvars[n]); }
};


/*====================
  CCvarArray::CCvarArray
  ====================*/
template <class T, class T2>
CCvarArray<T, T2>::CCvarArray(const tstring &sName, int iNumElements, T _Default, int iFlags, ConsoleElementFn_t pfnCmd) :
m_iNumElements(iNumElements)
{
    typedef CCvar<T,T2>     CCvar_t;
    m_pCvars = K2_NEW_ARRAY(ctx_Console, CCvar_t*, iNumElements);

    for (int n = 0; n < iNumElements; ++n)
    {
        m_pCvars[n] = K2_NEW(ctx_Console,  CCvar_t)(sName + XtoA(n + 1), _Default, iFlags, pfnCmd);
    }
}



/*====================
  CCvarArray::CCvarArray
  ====================*/
template <class T, class T2>
CCvarArray<T, T2>::CCvarArray(const tstring &sName, int iNumElements, T _Default, int iFlags, T _Min, T _Max, ConsoleElementFn_t pfnCmd) :
m_iNumElements(iNumElements)
{
    typedef CCvar<T,T2>     CCvar_t;
    m_pCvars = K2_NEW_ARRAY(ctx_Console, CCvar_t*, iNumElements);

    for (int n = 0; n < iNumElements; ++n)
    {
        m_pCvars[n] = K2_NEW(ctx_Console,  CCvar_t)(sName + XtoA(n + 1), _Default, iFlags, _Min, _Max, pfnCmd);
    }
}


/*====================
  CCvarArray::~CCvarArray
  ====================*/
template <class T, class T2>
CCvarArray<T, T2>::~CCvarArray()
{
    for (int n = 0; n < m_iNumElements; ++n)
    {
        K2_DELETE(m_pCvars[n]);
    }

    K2_DELETE_ARRAY(m_pCvars);
}
//=============================================================================
#endif // __C_CVARARRAY_H__
