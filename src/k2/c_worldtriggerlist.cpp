// (C)2005 S2 Games
// c_worldentitylist.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_worldtriggerlist.h"
#include "c_xmlmanager.h"
#include "c_xmldoc.h"
#include "c_world.h"
//=============================================================================

/*====================
  CWorldTriggerList::~CWorldTriggerList
  ====================*/
CWorldTriggerList::~CWorldTriggerList()
{
    Release();
}


/*====================
  CWorldTriggerList::CWorldTriggerList
  ====================*/
CWorldTriggerList::CWorldTriggerList(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("TriggerList"))
{
}

/*====================
  CWorldTriggerList::RegisterReservedScripts
  ====================*/
void    CWorldTriggerList::RegisterReservedScripts()
{
    RegisterReservedScript(_T("frame"));
    RegisterReservedScript(_T("gamestart"));
    RegisterReservedScript(_T("gameend"));
    RegisterReservedScript(_T("setupframe"));
    RegisterReservedScript(_T("activeframe"));
    RegisterReservedScript(_T("endedframe"));
    RegisterReservedScript(_T("playerjoin"));
    RegisterReservedScript(_T("playerleave"));
    RegisterReservedScript(_T("sellitem"));
    RegisterReservedScript(_T("buyitem"));
    RegisterReservedScript(_T("changeteam"));
    RegisterReservedScript(_T("changeunit"));
    RegisterReservedScript(_T("maploaded"));
    RegisterReservedScript(_T("mapunloaded"));
    RegisterReservedScript(_T("mapreset"));
    RegisterReservedScript(_T("spawn"));
    RegisterReservedScript(_T("loadout"));
    RegisterReservedScript(_T("scriptinput"));
    RegisterReservedScript(_T("activateprimary"));
    RegisterReservedScript(_T("activatesecondary"));
    RegisterReservedScript(_T("activatetertiary"));
    RegisterReservedScript(_T("phasechange"));
    RegisterReservedScript(_T("spellimpact"));
    RegisterReservedScript(_T("warmupstart"));
    RegisterReservedScript(_T("warmupframe"));
    RegisterReservedScript(_T("buildingplaced"));
    RegisterReservedScript(_T("placegadget"));
}

/*====================
  CWorldTriggerList::Load
  ====================*/
bool    CWorldTriggerList::Load(CArchive &archive, const CWorld *pWorld)
{
    try
    {
        CFileHandle hEntList(m_sName, FILE_READ | FILE_BINARY, archive);
        if (!hEntList.IsOpen())
            EX_ERROR(_T("Couldn't open file"));

        XMLManager.Process(hEntList, _T("triggerlist"), this);
        m_bChanged = true;

        RegisterReservedScripts();

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldTriggerList::Load() - "), NO_THROW);
        return false;
    }
}


/*====================
  CWorldTriggerList::Generate
  ====================*/
bool    CWorldTriggerList::Generate(const CWorld *pWorld)
{
    RegisterReservedScripts();

    return true;
}

/*====================
  CWorldTriggerList::RegisterReservedScript
  ====================*/
void    CWorldTriggerList::RegisterReservedScript(const tstring &sScript)
{
    if (!HasScript(sScript))
        RegisterNewScript(sScript, _T(""));

    m_mapReserved[sScript] = true;
}


/*====================
  CWorldTriggerList::Release
  ====================*/
void    CWorldTriggerList::Release()
{
    m_mapScripts.clear();
}

/*====================
  CWorldTriggerList::IsScriptReserved
  ====================*/
bool    CWorldTriggerList::IsScriptReserved(const tstring &sName)
{
    map<tstring, bool>::iterator findit(m_mapReserved.find(sName));

    if (findit != m_mapReserved.end())
        return findit->second;

    return false;
}

/*====================
  CWorldTriggerList::Serialize
  ====================*/
bool    CWorldTriggerList::Serialize(IBuffer *pBuffer)
{
    CXMLDoc xml;
    xml.NewNode("triggerlist");
    for (tsmapts::iterator it(m_mapScripts.begin()); it != m_mapScripts.end(); ++it)
    {
        xml.NewNode("trigger");

        // Add script properties
        xml.AddProperty("name", it->first);
        xml.AddProperty("script", it->second);
        
        xml.EndNode();
    }
    xml.EndNode();

    pBuffer->Clear();
    pBuffer->Write(xml.GetBuffer()->Get(), xml.GetBuffer()->GetLength());
    return true;
}


/*====================
  CWorldTriggerList::RegisterNewScript
  ====================*/
void    CWorldTriggerList::RegisterNewScript(const tstring &sScriptName, const tstring &sScript)
{
    if (sScript.empty() && m_mapScripts.find(sScriptName) != m_mapScripts.end() && !IsScriptReserved(sScriptName))
        m_mapScripts.erase(sScriptName);
    else if (!sScript.empty() || IsScriptReserved(sScriptName))
        m_mapScripts[sScriptName] = sScript;

    m_bChanged = true;
}
