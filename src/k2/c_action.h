// (C)2005 S2 Games
// c_action.h
//
//=============================================================================
#ifndef __C_ACTION_H__
#define __C_ACTION_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"
#include "i_baseinput.h"
#include "c_input.h" // EBindTable
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
K2_API EBindTable   EBindTableFromString(const tstring &sBindTable);
K2_API tstring      EBindTableToString(EBindTable eBindTable);
//=============================================================================


//=============================================================================
// Definitions
//=============================================================================
typedef void(*ActionFn_t)(float fValue, float fDelta, const CVec2f &v2Cursor, const tstring &sParam);

// Declaration macros
#define ACTION(name, type) \
void    act##name##Fn(float fValue, float fDelta, const CVec2f &v2Cursor, const tstring &sParam); \
CAction act##name(_T(#name), type, act##name##Fn, ACTION_HOME); \
void    act##name##Fn(float fValue, float fDelta, const CVec2f &v2Cursor, const tstring &sParam)

#define ACTION_BUTTON(name) \
void    act##name##Fn(float fValue, float fDelta, const CVec2f &v2Cursor, const tstring &sParam); \
CAction act##name(_T(#name), AT_BUTTON, act##name##Fn, ACTION_HOME); \
void    act##name##Fn(float fValue, float fDelta, const CVec2f &v2Cursor, const tstring &sParam)

#define ACTION_BUTTON_EX(name, flags) \
void    act##name##Fn(float fValue, float fDelta, const CVec2f &v2Cursor, const tstring &sParam); \
CAction act##name(_T(#name), AT_BUTTON, act##name##Fn, ACTION_HOME | flags); \
void    act##name##Fn(float fValue, float fDelta, const CVec2f &v2Cursor, const tstring &sParam)

#define ACTION_AXIS(name) \
void    act##name##Fn(float fValue, float fDelta, const CVec2f &v2Cursor, const tstring &sParam); \
CAction act##name(_T(#name), AT_AXIS, act##name##Fn, ACTION_HOME); \
void    act##name##Fn(float fValue, float fDelta, const CVec2f &v2Cursor, const tstring &sParam)

#define ACTION_IMPULSE(name) \
void    act##name##Fn(float fValue, float fDelta, const CVec2f &v2Cursor, const tstring &sParam); \
CAction act##name(_T(#name), AT_IMPULSE, act##name##Fn, ACTION_HOME); \
void    act##name##Fn(float fValue, float fDelta, const CVec2f &v2Cursor, const tstring &sParam)

#define ACTION_IMPULSE_EX(name, flags) \
void    act##name##Fn(float fValue, float fDelta, const CVec2f &v2Cursor, const tstring &sParam); \
CAction act##name(_T(#name), AT_IMPULSE, act##name##Fn, ACTION_HOME | flags); \
void    act##name##Fn(float fValue, float fDelta, const CVec2f &v2Cursor, const tstring &sParam)
//=============================================================================

//=============================================================================
// CAction
// An action consists of a function that can interact directly with the game
// code and can be bound to input events.  It is very much like a console
// command, but only actions are bindable.
//=============================================================================
class K2_API CAction : public IBaseInput
{
private:
    ActionFn_t      m_pfnAction;

    // Actions should not be copied
    CAction(CAction&);
    CAction& operator=(CAction&);

public:
    CAction(const tstring &sName, EActionType eType, ActionFn_t pfnAction, int iFlags = 0);
    virtual ~CAction()  {}

    void    Do(float fValue, float fDelta, const CVec2f &v2Cursor, const tstring &sParam = _T(""));
    void    operator()(float fValue, float fDelta, const CVec2f &v2Cursor, const tstring &sParam = _T(""));

    static  bool    WriteConfigFile(CFileHandle &hFile, const tsvector &wildcards, int iFlags);
};
//=============================================================================
#endif //__C_ACTION_H__
