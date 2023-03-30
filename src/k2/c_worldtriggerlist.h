// (C)2007 S2 Games
// c_worldtriggerlist.h
//
//=============================================================================
#ifndef __C_WORLDTRIGGERLIST_H__
#define __C_WORLDTRIGGERLIST_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_worldcomponent.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CWorldTrigger;
//=============================================================================

//=============================================================================
// CWorldTriggerList
//=============================================================================
class CWorldTriggerList : public IWorldComponent
{
private:
    tsmapts m_mapScripts;
    map<tstring, bool>  m_mapReserved;

public:
    ~CWorldTriggerList();
    CWorldTriggerList(EWorldComponent eComponent);

    bool    Load(CArchive &archive, const CWorld *pWorld);
    bool    Generate(const CWorld *pWorld);
    void    Release();
    bool    Serialize(IBuffer *pBuffer);

    K2_API void             RegisterNewScript(const tstring &sScriptName, const tstring &sScript);
    K2_API bool             HasScript(const tstring &sScriptName)           { return (m_mapScripts.find(sScriptName) != m_mapScripts.end()); }
    K2_API void             DeleteScript(const tstring &sScriptName)        { if (HasScript(sScriptName)) m_mapScripts.erase(sScriptName); }
    K2_API tstring          GetScript(const tstring &sName)                 { if (HasScript(sName)) return m_mapScripts[sName]; return _T(""); }
    K2_API tsmapts&         GetScriptMap()                                  { return m_mapScripts; }
    K2_API bool             IsScriptReserved(const tstring &sName);

    K2_API void             RegisterReservedScript(const tstring &sScript);
    K2_API void             RegisterReservedScripts();
};
//=============================================================================

#endif //__C_WORLDTRIGGERLIST_H__
