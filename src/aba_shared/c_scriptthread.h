// (C)2010 S2 Games
// c_scriptthread.h
//
//=============================================================================
#ifndef __C_SCRIPTTHREAD_H__
#define __C_SCRIPTTHREAD_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_combataction.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CScriptThread
//=============================================================================
class CScriptThread : public IActionScript
{
private:
    tstring                 m_sName;

    uint                    m_uiStartTime;
    uint                    m_uiLastUpdateTime;
    uint                    m_uiWaitTime;
    
    const CombatActionScript    *m_pvActions;

    uint                    m_uiThisUID;
    uint                    m_uiLevel;

    SCombatActionEnv        m_cEnv;

    CombatActionScript      m_vActions;

public:
    GAME_SHARED_API ~CScriptThread();
    GAME_SHARED_API CScriptThread(const tstring &sName);
    GAME_SHARED_API CScriptThread(CScriptThread &cDef, uint uiStartTime);

    virtual void    AddAction(ICombatAction *pAction)               { m_vActions.push_back(pAction); }
    
    GAME_SHARED_API bool Execute(uint uiTime);

    void                    Wait(uint uiDuration)                   { m_uiWaitTime += uiDuration; }
    uint                    GetWaitTime() const                     { return m_uiWaitTime; }

    GAME_SHARED_API void    Precache(EPrecacheScheme eScheme);

    const tstring&  GetName() const         { return m_sName; }
};
//=============================================================================

#endif //__C_SCRIPTTHREAD_H__
