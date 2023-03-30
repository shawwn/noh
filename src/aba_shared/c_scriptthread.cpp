// (C)2010 S2 Games
// c_scriptthread.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_scriptthread.h"
//=============================================================================

/*====================
  CScriptThread::~CScriptThread
  ====================*/
CScriptThread::~CScriptThread()
{
}


/*====================
  CScriptThread::CScriptThread
  ====================*/
CScriptThread::CScriptThread(const tstring &sName) :
m_sName(sName),

m_pvActions(NULL),

m_uiStartTime(0),
m_uiLastUpdateTime(INVALID_TIME),
m_uiWaitTime(0),

m_uiThisUID(INVALID_INDEX),
m_uiLevel(1)
{
}


/*====================
  CScriptThread::CScriptThread
  ====================*/
CScriptThread::CScriptThread(CScriptThread &cDef, uint uiStartTime) :
m_sName(cDef.m_sName),

m_pvActions(&cDef.m_vActions),

m_uiStartTime(uiStartTime),
m_uiLastUpdateTime(INVALID_TIME),
m_uiWaitTime(0),

m_uiThisUID(INVALID_INDEX),
m_uiLevel(1)
{
    IGameEntity *pThis(Game.GetEntityFromUniqueID(m_uiThisUID));
    IGameEntity *pInitiator(Game.GetEntityFromUniqueID(m_uiThisUID));
    IGameEntity *pInflictor(Game.GetEntityFromUniqueID(m_uiThisUID));
    IGameEntity *pTarget(Game.GetEntityFromUniqueID(m_uiThisUID));
    IGameEntity *pProxy(Game.GetEntityFromUniqueID(m_uiThisUID));

    CVec3f v3Target(V_ZERO);
    CVec3f v3Delta(V_ZERO);

    m_cEnv.pThis = pThis;
    m_cEnv.uiLevel = m_uiLevel;
    m_cEnv.pInitiator = pInitiator;
    m_cEnv.pInflictor = pInflictor;
    m_cEnv.pTarget = pTarget;
    m_cEnv.pProxy = pProxy;
    m_cEnv.v3Target = v3Target;
    m_cEnv.pCombatEvent = NULL;
    m_cEnv.pDamageEvent = NULL;
    m_cEnv.v3Delta = v3Delta;
    m_cEnv.pScriptThread = this;

    m_cEnv.citAct = m_pvActions->begin();
    
    m_cEnv.fResult = 0.0f;
    m_cEnv.fVar0 = 0.0f;
    m_cEnv.fVar1 = 0.0f;
    m_cEnv.fVar2 = 0.0f;
    m_cEnv.fVar3 = 0.0f;
    m_cEnv.v3Pos0.Clear();
    m_cEnv.v3Pos1.Clear();
    m_cEnv.v3Pos2.Clear();
    m_cEnv.v3Pos3.Clear();
    m_cEnv.pEnt0 = NULL;
    m_cEnv.pEnt1 = NULL;
    m_cEnv.pEnt2 = NULL;
    m_cEnv.pEnt3 = NULL;

    m_cEnv.uiWaitTime = 0;
    m_cEnv.bStall = false;
    m_cEnv.bTerminate = false;
    m_cEnv.uiRepeated = 0;
    m_cEnv.uiTracker = 0;
    m_cEnv.pNext = NULL;
}


/*====================
  CScriptThread::Execute

  true is returned if the thread finishes, otherwise false (on a pause or such)
  ====================*/
bool    CScriptThread::Execute(uint uiTime)
{
    if (m_uiLastUpdateTime == INVALID_TIME)
    {
        if (uiTime < m_uiStartTime)
            return false;
        else
            m_uiLastUpdateTime = uiTime;
    }

    uint uiDeltaTime(uiTime > m_uiLastUpdateTime ? uiTime - m_uiLastUpdateTime : 0);

    if (m_uiWaitTime > 0)
    {
        if (uiDeltaTime > m_uiWaitTime)
        {
            uiDeltaTime -= m_uiWaitTime;
            m_uiWaitTime = 0;
        }
        else if (uiDeltaTime > 0)
        {
            m_uiWaitTime -= uiDeltaTime;
        }
    }

    // Update environment
    IGameEntity *pThis(Game.GetEntityFromUniqueID(m_uiThisUID));
    IGameEntity *pInitiator(Game.GetEntityFromUniqueID(m_uiThisUID));
    IGameEntity *pInflictor(Game.GetEntityFromUniqueID(m_uiThisUID));
    IGameEntity *pTarget(Game.GetEntityFromUniqueID(m_uiThisUID));
    IGameEntity *pProxy(Game.GetEntityFromUniqueID(m_uiThisUID));

    CVec3f v3Target(V_ZERO);
    CVec3f v3Delta(V_ZERO);

    m_cEnv.pThis = pThis;
    m_cEnv.uiLevel = m_uiLevel;
    m_cEnv.pInitiator = pInitiator;
    m_cEnv.pInflictor = pInflictor;
    m_cEnv.pTarget = pTarget;
    m_cEnv.pProxy = pProxy;
    m_cEnv.v3Target = v3Target;
    m_cEnv.pCombatEvent = NULL;
    m_cEnv.pDamageEvent = NULL;
    m_cEnv.v3Delta = v3Delta;
    m_cEnv.bStall = false;

    if (m_uiWaitTime == 0)
    {
        if (m_cEnv.citAct == m_pvActions->end())
            return true;

        while (m_cEnv.citAct != m_pvActions->end())
        {
            (*m_cEnv.citAct)->SetEnv(&m_cEnv);

            m_cEnv.fResult = (*m_cEnv.citAct)->Execute();

            if (m_cEnv.bTerminate)
                break;

            if (m_cEnv.bStall)
            {
                ++m_cEnv.uiRepeated;
                break;
            }

            ++m_cEnv.citAct;
            
            m_cEnv.uiRepeated = 0;
            m_cEnv.uiTracker = 0;

            if (m_uiWaitTime > 0)
                break;
        }
    }

    if (uiTime > m_uiLastUpdateTime)
        m_uiLastUpdateTime = uiTime;

    return ((m_cEnv.citAct == m_pvActions->end() || m_cEnv.bTerminate) && m_uiWaitTime == 0);
}


/*====================
  CScriptThread::Precache
  ====================*/
void    CScriptThread::Precache(EPrecacheScheme eScheme)
{
    // Script actions
    for (CombatActionScript_cit it(m_vActions.begin()); it != m_vActions.end(); ++it)
        (*it)->Precache(eScheme);
}
