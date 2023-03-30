// (C)2008 S2 Games
// c_gamebind.h
//
//=============================================================================
#ifndef __C_GAMEBIND_H__
#define __C_GAMEBIND_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"

#include "c_consoleelement.h"
#include "c_action.h"
#include "c_input.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define GAMEBIND_BUTTON(name, table, key1, mod1) \
CGameBindButton gamebind##name(_T(#name), _T(#name), TSNULL, table, key1, mod1, BUTTON_INVALID, BIND_MOD_NONE, 0);

#define GAMEBIND_BUTTON2(name, table, key1, mod1, key2, mod2) \
CGameBindButton gamebind##name(_T(#name), _T(#name), TSNULL, table, key1, mod1, key2, mod2, 0);

#define GAMEBIND_BUTTON_EX(name, action, param, table, key1, mod1) \
CGameBindButton gamebind##name(_T(#name), _T(#action), param, table, key1, mod1, BUTTON_INVALID, BIND_MOD_NONE, 0);

#define GAMEBIND_BUTTON2_EX(name, action, param, table, key1, mod1, key2, mod2) \
CGameBindButton gamebind##name(_T(#name),  _T(#action), param, table, key1, mod1, key2, mod2, 0);

#define GAMEBIND_AXIS(name, table, key1, mod1) \
CGameBindAxis gamebind##name(_T(#name), _T(#name), TSNULL, table, key1, mod1, AXIS_INVALID, BIND_MOD_NONE, 0);

#define GAMEBIND_AXIS2(name, table, key1, mod1, key2, mod2) \
CGameBindAxis gamebind##name(_T(#name), _T(#name), TSNULL, table, key1, mod1, key2, mod2, 0);

#define GAMEBIND_IMPULSE(name, table, key1, mod1) \
CGameBindImpulse gamebind##name(_T(#name), _T(#name), TSNULL, table, key1, mod1, BUTTON_INVALID, BIND_MOD_NONE, 0);

#define GAMEBIND_IMPULSE_EX(name, action, param, table, key1, mod1) \
CGameBindImpulse gamebind##name(_T(#name), _T(#action), param, table, key1, mod1, BUTTON_INVALID, BIND_MOD_NONE, 0);

#define GAMEBIND_IMPULSE2(name, table, key1, mod1, key2, mod2) \
CGameBindImpulse gamebind##name(_T(#name), _T(#name), TSNULL, table, key1, mod1, key2, mod2, 0);

#define GAMEBIND_IMPULSE2_EX(name, action, param, table, key1, mod1, key2, mod2) \
CGameBindImpulse gamebind##name(_T(#name), _T(#action), param, table, key1, mod1, key2, mod2, 0);
//=============================================================================

//=============================================================================
// IGameBind
//=============================================================================
class IGameBind : public CConsoleElement
{
protected:
    tstring         m_sAction;
    tstring         m_sParam;
    EActionType     m_eActionType;
    EBindTable      m_eBindTable;

    bool            m_bInherentValue;
    tstring         m_sInherentKey1;
    tstring         m_sInherentKey2;

    void            Write(CFileHandle &hFile);

public:
    K2_API ~IGameBind();
    IGameBind(const tstring &sName, const tstring &sAction, const tstring &sParam, EActionType eActionType, EBindTable eBindTable, int iFlags = 0);

    tstring             GetString() const       { return TSNULL; }

    const tstring&      GetAction() const       { return m_sAction; }
    const tstring&      GetParam() const        { return m_sParam; }
    EActionType         GetActionType() const   { return m_eActionType; }
    EBindTable          GetBindTable() const    { return m_eBindTable; }

    virtual tstring     GetTypeString() = 0;
    virtual tstring     GetKey1String() = 0;
    virtual tstring     GetKey2String() = 0;

    virtual void        SetKey1(const tstring &sKey, bool bBind) = 0;
    virtual void        SetKey2(const tstring &sKey, bool bBind) = 0;

    virtual void        ResetKey1() = 0;
    virtual void        ResetKey2() = 0;

    static IGameBind*   Find(const tstring &sName);
    static IGameBind*   Create(const tstring &sName, const tstring &sAction, const tstring &sParam, EActionType eActionType, EBindTable eBindTable, const tstring &sKey1 = TSNULL, const tstring &sKey2 = TSNULL, int iFlags = 0);
    static bool         WriteConfigFile(CFileHandle &hFile);
};
//=============================================================================

//=============================================================================
// CGameBindButton
//=============================================================================
#if defined(linux) || defined(__APPLE__)
class __attribute__((visibility("default"))) CGameBindButton : public IGameBind
#else
class CGameBindButton : public IGameBind
#endif
{
protected:
    EButton     m_eButton1;
    int         m_iModifier1;
    
    EButton     m_eButton2;
    int         m_iModifier2;

    EButton     m_eButton1Default;
    int         m_iModifier1Default;
    
    EButton     m_eButton2Default;
    int         m_iModifier2Default;

    // Prevent copies
    CGameBindButton();
    CGameBindButton(const CGameBindButton &);

public:
    ~CGameBindButton() {}
    K2_API CGameBindButton
    (
        const tstring &sName,
        const tstring &sAction,
        const tstring &sParam,
        EBindTable eBindTable,
        EButton eButton1,
        int iModifier1,
        EButton eButton2,
        int iModifier2,
        int iFlags = 0
    );

    virtual tstring     GetTypeString()     { return _T("button"); }
    virtual tstring     GetKey1String()     { return Input.GetBindString(m_eButton1, m_iModifier1); }
    virtual tstring     GetKey2String()     { return Input.GetBindString(m_eButton2, m_iModifier2); }

    K2_API virtual void     SetKey1(const tstring &sKey, bool bBind);
    K2_API virtual void     SetKey2(const tstring &sKey, bool bBind);

    K2_API virtual  int         GetBothModifiers() const    { return m_iModifier1 | m_iModifier2; }

    K2_API virtual  bool        GetButton1Down() const      { return m_eButton1 != 0 ? Input.IsButtonDown(m_eButton1) : true; }
    K2_API virtual  bool        GetButton2Down() const      { return m_eButton2 != 0 ? Input.IsButtonDown(m_eButton2) : true; }

    K2_API virtual  bool        GetModifier1Down() const        
    {
        bool bReturn(m_eButton1 == 0 ? false : true);
        bReturn = m_iModifier1 & BIND_MOD_CTRL ? Input.IsCtrlDown() : bReturn; 
        bReturn = m_iModifier1 & BIND_MOD_ALT ? Input.IsAltDown() : bReturn; 
        bReturn = m_iModifier1 & BIND_MOD_SHIFT ? Input.IsShiftDown() : bReturn; 
#ifdef __APPLE__
        bReturn = m_iModifier1 & BIND_MOD_CMD ? Input.IsCommandDown() : bReturn; 
#endif
        return bReturn; 
    }

    K2_API virtual  bool        GetModifier2Down() const        
    {
        bool bReturn(m_eButton2 == 0 ? false : true);
        bReturn = m_iModifier2 & BIND_MOD_CTRL ? Input.IsCtrlDown() : bReturn; 
        bReturn = m_iModifier2 & BIND_MOD_ALT ? Input.IsAltDown() : bReturn; 
        bReturn = m_iModifier2 & BIND_MOD_SHIFT ? Input.IsShiftDown() : bReturn; 
#ifdef __APPLE__
        bReturn = m_iModifier2 & BIND_MOD_CMD ? Input.IsCommandDown() : bReturn; 
#endif
        return bReturn; 
    }

    K2_API virtual void     ResetKey1();
    K2_API virtual void     ResetKey2();
};
//=============================================================================

//=============================================================================
// CGameBindAxis
//=============================================================================
class CGameBindAxis : public IGameBind
{
protected:
    EAxis       m_eAxis1;
    int         m_iModifier1;

    EAxis       m_eAxis2;
    int         m_iModifier2;

    EAxis       m_eAxis1Default;
    int         m_iModifier1Default;

    EAxis       m_eAxis2Default;
    int         m_iModifier2Default;

    // Prevent copies
    CGameBindAxis();
    CGameBindAxis(const CGameBindAxis &);

public:
    ~CGameBindAxis() {}
    CGameBindAxis
    (
        const tstring &sName,
        const tstring &sAction,
        const tstring &sParam,
        EBindTable eBindTable,
        EAxis eAxis1,
        int iModifier1,
        EAxis eAxis2,
        int iModifier2,
        int iFlags = 0
    );

    virtual tstring     GetTypeString()     { return _T("axis"); }
    virtual tstring     GetKey1String()     { return Input.GetBindString(m_eAxis1, m_iModifier1); }
    virtual tstring     GetKey2String()     { return Input.GetBindString(m_eAxis2, m_iModifier2); }

    K2_API virtual void     SetKey1(const tstring &sKey, bool bBind);
    K2_API virtual void     SetKey2(const tstring &sKey, bool bBind);

    K2_API virtual  int         GetBothModifiers() const    { return m_iModifier1 | m_iModifier2; }

    K2_API virtual  bool        GetModifier1Down() const        
    {
        bool bReturn(m_eAxis1 == 0 ? false : true);
        bReturn = m_iModifier1 & BIND_MOD_CTRL ? Input.IsCtrlDown() : bReturn; 
        bReturn = m_iModifier1 & BIND_MOD_ALT ? Input.IsAltDown() : bReturn; 
        bReturn = m_iModifier1 & BIND_MOD_SHIFT ? Input.IsShiftDown() : bReturn; 
#ifdef __APPLE__
        bReturn = m_iModifier1 & BIND_MOD_CMD ? Input.IsCommandDown() : bReturn; 
#endif
        return bReturn; 
    }

    K2_API virtual  bool        GetModifier2Down() const        
    {
        bool bReturn(m_eAxis2 == 0 ? false : true);
        bReturn = m_iModifier2 & BIND_MOD_CTRL ? Input.IsCtrlDown() : bReturn; 
        bReturn = m_iModifier2 & BIND_MOD_ALT ? Input.IsAltDown() : bReturn; 
        bReturn = m_iModifier2 & BIND_MOD_SHIFT ? Input.IsShiftDown() : bReturn; 
#ifdef __APPLE__
        bReturn = m_iModifier2 & BIND_MOD_CMD ? Input.IsCommandDown() : bReturn; 
#endif
        return bReturn; 
    }

    K2_API virtual void     ResetKey1();
    K2_API virtual void     ResetKey2();
};
//=============================================================================

//=============================================================================
// CGameBindImpulse
//=============================================================================
#if defined(linux) || defined(__APPLE__)
class __attribute__((visibility("default"))) CGameBindImpulse : public IGameBind
#else
class CGameBindImpulse : public IGameBind
#endif
{
protected:
    EButton     m_eButton1;
    int         m_iModifier1;

    EButton     m_eButton2;
    int         m_iModifier2;

    EButton     m_eButton1Default;
    int         m_iModifier1Default;

    EButton     m_eButton2Default;
    int         m_iModifier2Default;

    // Prevent copies
    CGameBindImpulse();
    CGameBindImpulse(const CGameBindImpulse &);

public:
    ~CGameBindImpulse() {}
    K2_API CGameBindImpulse
    (
        const tstring &sName,
        const tstring &sAction,
        const tstring &sParam,
        EBindTable eBindTable,
        EButton eButton1,
        int iModifier1,
        EButton eButton2,
        int iModifier2,
        int iFlags = 0
    );

    virtual tstring     GetTypeString()     { return _T("impulse"); }
    virtual tstring     GetKey1String()     { return Input.GetBindString(m_eButton1, m_iModifier1); }
    virtual tstring     GetKey2String()     { return Input.GetBindString(m_eButton2, m_iModifier2); }

    K2_API virtual void     SetKey1(const tstring &sKey, bool bBind);
    K2_API virtual void     SetKey2(const tstring &sKey, bool bBind);

    K2_API virtual  bool        GetButton1Down() const      { return m_eButton1 != 0 ? Input.IsButtonDown(m_eButton1) : true; }
    K2_API virtual  bool        GetButton2Down() const      { return m_eButton2 != 0 ? Input.IsButtonDown(m_eButton2) : true; }

    K2_API virtual  int         GetBothModifiers() const    { return m_iModifier1 | m_iModifier2; }
    
    K2_API virtual  bool        GetModifier1Down() const        
    {
        bool bReturn(m_eButton1 == 0 ? false : true);
        bReturn = m_iModifier1 & BIND_MOD_CTRL ? Input.IsCtrlDown() : bReturn; 
        bReturn = m_iModifier1 & BIND_MOD_ALT ? Input.IsAltDown() : bReturn; 
        bReturn = m_iModifier1 & BIND_MOD_SHIFT ? Input.IsShiftDown() : bReturn; 
#ifdef __APPLE__
        bReturn = m_iModifier1 & BIND_MOD_CMD ? Input.IsCommandDown() : bReturn; 
#endif
        return bReturn; 
    }

    K2_API virtual  bool        GetModifier2Down() const        
    {
        bool bReturn(m_eButton2 == 0 ? false : true);
        bReturn = m_iModifier2 & BIND_MOD_CTRL ? Input.IsCtrlDown() : bReturn; 
        bReturn = m_iModifier2 & BIND_MOD_ALT ? Input.IsAltDown() : bReturn; 
        bReturn = m_iModifier2 & BIND_MOD_SHIFT ? Input.IsShiftDown() : bReturn; 
#ifdef __APPLE__
        bReturn = m_iModifier2 & BIND_MOD_CMD ? Input.IsCommandDown() : bReturn; 
#endif
        return bReturn; 
    }

    K2_API virtual void     ResetKey1();
    K2_API virtual void     ResetKey2();
};
//=============================================================================

#endif //__C_GAMEBIND_H__
