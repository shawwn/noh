// (C)2010 S2 Games
// c_replayinfo.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_replayinfo.h"
#include "../k2/c_xmlprocroot.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================


/*====================
  CReplayInfo::Reset
  ====================*/
void    CReplayInfo::Reset()
{
    m_cGameInfoProperties = g_NullXMLNode;
    m_vPlayerInfo.clear();
    m_vPlayerInfo.resize(10, g_NullXMLNode);
}


/*====================
  CReplayInfo::SetReplayGameInfo
  ====================*/
void    CReplayInfo::SetReplayGameInfo(const CXMLNode& cNode)
{
    m_cGameInfoProperties = cNode;
}


/*====================
  CReplayInfo::SetReplayPlayerInfo
  ====================*/
void    CReplayInfo::SetReplayPlayerInfo(const CXMLNode& cNode)
{
    uint uiTeam(cNode.GetPropertyInt(_T("team")));
    if (uiTeam != 1 && uiTeam != 2)
        return;

    uint uiPlayerIndex((uiTeam - 1) * 5  + cNode.GetPropertyInt(_T("teamindex")));

    uiPlayerIndex = CLAMP(uiPlayerIndex, 0u, 9u);

    m_vPlayerInfo[uiPlayerIndex] = cNode;
}


// <replayinfo>
DECLARE_XML_PROCESSOR(replayinfo);
BEGIN_XML_REGISTRATION(replayinfo)
    REGISTER_XML_PROCESSOR(root)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(replayinfo, CReplayInfo)
    pObject->Reset();
    pObject->SetReplayGameInfo(node);
END_XML_PROCESSOR(pObject)

// <player>
DECLARE_XML_PROCESSOR(player)
BEGIN_XML_REGISTRATION(player)
    REGISTER_XML_PROCESSOR(replayinfo)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(player, CReplayInfo)
    pObject->SetReplayPlayerInfo(node);
END_XML_PROCESSOR_NO_CHILDREN
