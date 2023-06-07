// (C)2006 S2 Games
// c_teamdefinition.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_teamdefinition.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CTeamDefinition g_teamdefNeutral(_T("Neutral"));
CTeamDefinition g_teamdefHuman(_T("Human"));
CTeamDefinition g_teamdefBeast(_T("Beast"));
//=============================================================================

/*====================
  CTeamDefinition::CTeamDefinition
  ====================*/
CTeamDefinition::CTeamDefinition(const tstring sName) :
m_sName(sName),
INIT_TEAM_DEFINITION_SETTING(String, BaseBuildingName),
INIT_TEAM_DEFINITION_SETTING(String, OfficerSkill),
INIT_TEAM_DEFINITION_SETTING(String, WorkerName),
INIT_TEAM_DEFINITION_SETTING(String, ResableEffect),
INIT_TEAM_DEFINITION_SETTING(String, SquadNames),
INIT_TEAM_DEFINITION_SETTING(String, SquadColors)
{
    Update(true);
}


/*====================
  CTeamDefinition::Update
  ====================*/
void    CTeamDefinition::Update(bool bForceUpdate)
{
    UPDATE_TEAM_DEFINITION_SETTING(BaseBuildingName)
    UPDATE_TEAM_DEFINITION_SETTING(OfficerSkill)
    UPDATE_TEAM_DEFINITION_SETTING(WorkerName)
    UPDATE_TEAM_DEFINITION_SETTING(ResableEffect)

    START_UPDATE_TEAM_DEFINITION_SETTING(SquadNames)
        m_vSquadNames = TokenizeString(m_SquadNames, _T(','));
    END_UPDATE_TEAM_DEFINITION_SETTING(SquadNames)

    START_UPDATE_TEAM_DEFINITION_SETTING(SquadColors)
        m_vSquadColors = TokenizeString(m_SquadColors, _T(','));
    END_UPDATE_TEAM_DEFINITION_SETTING(SquadColors)
}
