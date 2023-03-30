// (C)2006 S2 Games
// c_eventcmdregistry.h
//
//=============================================================================
#ifndef __C_EVENTCMDREGISTRY_H__
#define __C_EVENTCMDREGISTRY_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
class CEventCmd;

typedef map<tstring, CEventCmd *>           EventCmdMap;
//=============================================================================

//=============================================================================
// CEventCmdRegistry
//=============================================================================
class CEventCmdRegistry
{
private:
    static CEventCmdRegistry    *s_pInstance;
    static bool             s_bRequested, s_bReleased;

    CEventCmdRegistry() {}
    CEventCmdRegistry(CEventCmdRegistry&);
    CEventCmdRegistry& operator=(CEventCmdRegistry&);

    EventCmdMap     m_mapEventCmds;

public:
    static CEventCmdRegistry*   GetInstance();
    static void             Release();
    static bool             IsReleased()    { return s_bReleased; }

    void                    Register(CEventCmd *pEventCmd);
    void                    Unregister(CEventCmd *pEventCmd);

    K2_API inline CEventCmd*    GetEventCmd(const tstring &sEventCmd);

    const EventCmdMap&          GetEventCmdMap()    { return m_mapEventCmds; }

    K2_API inline bool      Exists(const tstring &sEventCmd);
};


/*====================
  CEventCmdRegistry::Exists
  ====================*/
bool    CEventCmdRegistry::Exists(const tstring &sEventCmd)
{
    EventCmdMap::iterator find = m_mapEventCmds.find(LowerString(sEventCmd));

    if (find == m_mapEventCmds.end())
        return false;

    return true;
}


/*====================
  CEventCmdRegistry::GetEventCmd
  ====================*/
CEventCmd   *CEventCmdRegistry::GetEventCmd(const tstring &sEventCmd)
{
    EventCmdMap::iterator find = m_mapEventCmds.find(LowerString(sEventCmd));

    if (find == m_mapEventCmds.end())
        return NULL;
    else
        return find->second;
}

extern K2_API CEventCmdRegistry *pEventCmdRegistry;
//=============================================================================
#endif //__C_EVENTCMDREGISTRY_H__
