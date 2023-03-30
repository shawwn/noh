// (C)2005 S2 Games
// c_actionregistry.h
//
//=============================================================================
#ifndef __C_ACTIONREGISTRY_H__
#define __C_ACTIONREGISTRY_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_input.h"
#include "c_bind.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
extern K2_API class CActionRegistry* g_pActionRegistry;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#ifdef K2_EXPORTS
#define ActionRegistry  (*CActionRegistry::GetInstance())
#else
#define ActionRegistry  (*g_pActionRegistry)
#endif

class IBaseInput;

typedef map<tstring, IBaseInput*>   ActionMap;

typedef map<int, CBind>             BindModMap;
typedef map<EButton, BindModMap>    ButtonActionMap;
typedef map<EAxis, BindModMap>      AxisActionMap;
//=============================================================================

//=============================================================================
// CActionRegistry
//=============================================================================
class CActionRegistry
{
    SINGLETON_DEF(CActionRegistry)

private:
    ActionMap       m_mapActions;

    ButtonActionMap m_mapButtons[NUM_BINDTABLES];
    AxisActionMap   m_mapAxes[NUM_BINDTABLES];

public:
    ~CActionRegistry()  {}

    void                Register(IBaseInput *pAction);
    void                Unregister(IBaseInput *pAction);

    K2_API void     BindButton(EBindTable eTable, EButton eButton, int iModifier, const tstring &sAction, const tstring &sParam = _T(""), int iFlags = 0);
    K2_API void     BindAxis(EBindTable eTable, EAxis eAxis, int iModifier, const tstring &sAction, const tstring &sParam = _T(""), int iFlags = 0);
    K2_API void     BindImpulse(EBindTable eTable, EButton eButton, int iModifier, const tstring &sAction, const tstring &sParam = _T(""), int iFlags = 0);

    K2_API void     UnbindButton(EBindTable eTable, EButton button, int iModifier);
    K2_API void     UnbindAxis(EBindTable eTable, EAxis axis, int iModifier);
    K2_API void     UnbindImpulse(EBindTable eTable, EButton button, int iModifier);

    K2_API void     UnbindAll();
    K2_API void     UnbindTable(EBindTable eTable);

    K2_API CBind*   GetBind(EBindTable eTable, EButton button, int iModifier);
    K2_API CBind*   GetBind(EBindTable eTable, EAxis axis, int iModifier);

    K2_API inline   IBaseInput* GetAction(const tstring &sAction);

    const ActionMap&        GetActionMap() const                        { return m_mapActions; }
    const ButtonActionMap&  GetButtonActionMap(EBindTable eTable) const { return m_mapButtons[eTable]; }
    const AxisActionMap&    GetAxisActionMap(EBindTable eTable) const   { return m_mapAxes[eTable]; }

    K2_API inline bool  Exists(const tstring &sAction);
};

bool    CActionRegistry::Exists(const tstring &sAction)
{
    ActionMap::iterator find = m_mapActions.find(sAction);

    if (find == m_mapActions.end())
        return false;

    return true;
}

IBaseInput* CActionRegistry::GetAction(const tstring &sAction)
{
    ActionMap::iterator find = m_mapActions.find(sAction);

    if (find == m_mapActions.end())
        return NULL;
    else
        return find->second;
}
//=============================================================================
#endif //__C_ACTIONREGISTRY_H__
