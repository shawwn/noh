// (C)2006 S2 Games
// c_eventcmdregistry.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_eventcmdregistry.h"
#include "c_eventcmd.h"
#include "stringutils.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CEventCmdRegistry   *CEventCmdRegistry::s_pInstance;
bool            CEventCmdRegistry::s_bRequested;
bool            CEventCmdRegistry::s_bReleased;

CEventCmdRegistry   *pEventCmdRegistry = CEventCmdRegistry::GetInstance();
//=============================================================================


/*====================
  CEventCmdRegistry::GetInstance
  ====================*/
CEventCmdRegistry*  CEventCmdRegistry::GetInstance()
{
    assert(!s_bReleased);

    if (s_pInstance == nullptr)
    {
        assert(!s_bRequested);
        s_bRequested = true;
        s_pInstance = K2_NEW(ctx_Models,  CEventCmdRegistry);
    }

    return s_pInstance;
}


/*====================
  CEventCmdRegistry::Release
  ====================*/
void    CEventCmdRegistry::Release()
{
    assert(!s_bReleased);

    if (s_pInstance != nullptr)
        K2_DELETE(s_pInstance);

    s_bReleased = true;
}


/*====================
  CEventCmdRegistry::Register
  ====================*/
void    CEventCmdRegistry::Register(CEventCmd *pEventCmd)
{
    // Make sure there is no name collision
    EventCmdMap::iterator findit = m_mapEventCmds.find(LowerString(pEventCmd->GetName()));
    if (findit != m_mapEventCmds.end())
    {
        Console.Err << _T("A function named ") << QuoteStr(pEventCmd->GetName())
                    << _T(" already exists.") << newl;
        return;
    }

    m_mapEventCmds[LowerString(pEventCmd->GetName())] = pEventCmd;
}


/*====================
  CEventCmdRegistry::Unregister
  ====================*/
void    CEventCmdRegistry::Unregister(CEventCmd *pEventCmd)
{
    EventCmdMap::iterator findit = m_mapEventCmds.find(LowerString(pEventCmd->GetName()));
    if(findit != m_mapEventCmds.end() && findit->second == pEventCmd)
    {
        //Console.Dev << _T("Function ") << sName << _T(" has been unregistered.") << newl;
        m_mapEventCmds.erase(findit);
    }
}
