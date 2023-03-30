// (C)2006 S2 Games
// c_cvarreference.h
//
//=============================================================================
#ifndef __C_CVARREFERENCE_H__
#define __C_CVARREFERENCE_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_cvar.h"
//=============================================================================

//=============================================================================
// CCvarReference
//=============================================================================
class CCvarReference
{
private:
    ICvar*  m_pCvar;
    bool    m_bIgnore;


public:
    ~CCvarReference();
    CCvarReference();
    CCvarReference(const CCvarReference &A);
    CCvarReference(ICvar *pCvar);
    CCvarReference(const tstring &sName);

    void        Assign(ICvar *pCvar);
    void        Assign(const tstring &sName);

    void        Invalidate();
    bool        IsValid() const     { return m_pCvar != NULL; }
    bool        IsIgnored() const   { return m_bIgnore; }
    
    ECvarType   GetType() const     { if (m_pCvar != NULL) return m_pCvar->GetType(); else return CT_INVALID; }
    tstring     GetString() const   { if (m_pCvar != NULL) return m_pCvar->GetString(); else return _T("INVALID_CVAR"); }
    float       GetFloat() const    { if (m_pCvar != NULL) return m_pCvar->GetFloat(); else return 0.0f; }
    int         GetInt() const      { if (m_pCvar != NULL) return m_pCvar->GetInteger(); else return 0; }
    bool        GetBool() const     { if (m_pCvar != NULL) return m_pCvar->GetBool(); else return false; }
    CVec3f      GetVec3() const     { if (m_pCvar != NULL) return m_pCvar->GetVec3(); else return V_ZERO; }
    CVec4f      GetVec4() const     { if (m_pCvar != NULL) return m_pCvar->GetVec4(); else return V4_ZERO; }
    
    void        Set(const tstring &sValue)  { if (m_pCvar != NULL) m_pCvar->Set(sValue); }
    void        Set(float fValue)           { if (m_pCvar != NULL) m_pCvar->SetFloat(fValue); }
    void        Set(int iValue)             { if (m_pCvar != NULL) m_pCvar->SetInteger(iValue); }
    void        Set(bool bValue)            { if (m_pCvar != NULL) m_pCvar->SetBool(bValue); }

    CCvarReference& operator=(const CCvarReference &A);
};
//=============================================================================

#endif //__C_CVARREFERENCE_H__
