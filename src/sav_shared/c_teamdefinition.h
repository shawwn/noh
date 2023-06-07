// (C)2006 S2 Games
// c_teamdefinition.h
//
//=============================================================================
#ifndef __C_TEAMDEFINITION_H__
#define __C_TEAMDEFINITION_H__

//=============================================================================
// Declarations
//=============================================================================
class CTeamDefinition;
extern GAME_SHARED_API CTeamDefinition  g_teamdefNeutral;
extern GAME_SHARED_API CTeamDefinition  g_teamdefHuman;
extern GAME_SHARED_API CTeamDefinition  g_teamdefBeast;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define DECLARE_TEAM_DEFINITION_SETTING(type, name) \
private: \
    CCvar<type>*    m_p##name; \
    type            m_##name; \
public: \
    const type&     Get##name() const   { return m_##name; }

#define INIT_TEAM_DEFINITION_SETTING(type, name) \
    m_p##name(ICvar::Create##type(_T("team_") + m_sName + _T("_") _T(#name), _T(""), CVAR_GAMECONFIG|CVAR_TRANSMIT))

#define START_UPDATE_TEAM_DEFINITION_SETTING(name) \
if (m_p##name != NULL && (m_p##name->IsModified() || bForceUpdate)) \
{ \
    m_##name = m_p##name->GetValue(); \
    m_p##name->SetModified(false);

#define END_UPDATE_TEAM_DEFINITION_SETTING(name) \
}

#define UPDATE_TEAM_DEFINITION_SETTING(name) \
    START_UPDATE_TEAM_DEFINITION_SETTING(name) \
    END_UPDATE_TEAM_DEFINITION_SETTING(name)
//=============================================================================

//=============================================================================
// CTeamDefintion
//=============================================================================
class CTeamDefinition
{
private:
    tstring     m_sName;

    DECLARE_TEAM_DEFINITION_SETTING(tstring, BaseBuildingName)
    DECLARE_TEAM_DEFINITION_SETTING(tstring, OfficerSkill)
    DECLARE_TEAM_DEFINITION_SETTING(tstring, WorkerName)
    DECLARE_TEAM_DEFINITION_SETTING(tstring, ResableEffect)
    DECLARE_TEAM_DEFINITION_SETTING(tstring, SquadNames)
    DECLARE_TEAM_DEFINITION_SETTING(tstring, SquadColors)

    svector m_vSquadNames;
    svector m_vSquadColors;

public:
    ~CTeamDefinition()  {}
    GAME_SHARED_API CTeamDefinition(const tstring sName);

    const tstring&  GetName() const     { return m_sName; }

    void    Update(bool bForceUpdate = false);

    const tstring&  GetSquadName(uint ui)   { if (m_vSquadNames.empty()) return SNULL; return m_vSquadNames[CLAMP(ui, 0u, INT_SIZE(m_vSquadNames.size() - 1))]; }
    const tstring&  GetSquadColor(uint ui)  { if (m_vSquadColors.empty()) return SNULL; return m_vSquadColors[CLAMP(ui, 0u, INT_SIZE(m_vSquadColors.size() - 1))]; }
};
//=============================================================================

#endif //__C_TEAMDEFINITION_H__
