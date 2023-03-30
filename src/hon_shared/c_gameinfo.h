// (C)2007 S2 Games
// c_gameinfo.h
//
//=============================================================================
#ifndef __C_GAMEINFO_H__
#define __C_GAMEINFO_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gameentity.h"
#include "c_gamedefinition.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const byte GAME_FLAG_FIRST_BLOOD        (BIT(0));
const byte GAME_FLAG_FINAL_HERO_SELECT  (BIT(1));
const byte GAME_FLAG_CONCEDED           (BIT(2));
const byte GAME_FLAG_SOLO               (BIT(3));
const byte GAME_FLAG_ARRANGED           (BIT(4));
const byte GAME_FLAG_PAUSED             (BIT(5));

enum EStatsStatus
{
    STATS_NULL = 0,
    STATS_RECORDING,
    STATS_SUBMITTING,
    STATS_SUCCESSFUL,
    STATS_FAILURE
};

const tstring g_aStatsStatusNames[] =
{
    _T("null"),
    _T("recording"),
    _T("submitting"),
    _T("successful"),
    _T("failure")
};
//=============================================================================

//=============================================================================
// CGameInfo
//=============================================================================
class CGameInfo : public IGameEntity
{
    DECLARE_ENTITY_DESC

public:
    typedef CGameInfoDefinition TDefinition;

private:
    ushort      m_unServerDateIndex;
    ushort      m_unServerTimeIndex;
    ushort      m_unServerNameIndex;
    ushort      m_unGameNameIndex;

    byte        m_yFlags;

    uint        m_uiGamePhase;
    uint        m_uiPhaseStartTime;
    uint        m_uiPhaseDuration;
    uint        m_uiMatchID;

    uint        m_uiGameMode;
    uint        m_uiGameOptions;
    uint        m_uiTeamSize;
    uint        m_uiMaxSpectators;
    uint        m_uiMaxReferees;

    uint        m_uiMatchLength;
    
    byte        m_yServerAccess;
    byte        m_yHostFlags;

    byte        m_yActiveVote;
    int         m_iVoteTarget;
    uint        m_uiVoteEndTime;
    byte        m_yVotesRequired;
    byte        m_yYesVotes;
    
    ushort      m_unMinPSR;
    ushort      m_unMaxPSR; 

    uint        m_uiFirstBanTeam;

    byte        m_yStatsStatus;

    map<tstring, tstring>   m_mapScriptValues;

public:
    ~CGameInfo()    {}
    CGameInfo();

    SUB_ENTITY_ACCESSOR(CGameInfo, GameInfo)

    // Network
    virtual void    Baseline();
    virtual void    GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
    virtual bool    ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);
    
    GAME_SHARED_API void    Initialize();
    GAME_SHARED_API void    ValidateOptions();
    GAME_SHARED_API void    MatchRemake();

    void                    SetMatchID(uint uiMatchID)                  { m_uiMatchID = uiMatchID; }

    GAME_SHARED_API void    SetGamePhase(EGamePhase eGamePhase, uint uiDuration = INVALID_TIME, uint uiStartTime = INVALID_TIME);

    tstring         GetServerDate() const                   { return NetworkResourceManager.GetString(m_unServerDateIndex); }
    void            SetServerDate(const tstring &sString)   { NetworkResourceManager.SetString(m_unServerDateIndex, sString); }

    tstring         GetServerTime() const                   { return NetworkResourceManager.GetString(m_unServerTimeIndex); }
    void            SetServerTime(const tstring &sString)   { NetworkResourceManager.SetString(m_unServerTimeIndex, sString); }

    tstring         GetServerName() const                   { return NetworkResourceManager.GetString(m_unServerNameIndex); }
    void            SetServerName(const tstring &sString)   { NetworkResourceManager.SetString(m_unServerNameIndex, sString); }

    tstring         GetGameName() const                     { return NetworkResourceManager.GetString(m_unGameNameIndex); }
    void            SetGameName(const tstring &sString)     { NetworkResourceManager.SetString(m_unGameNameIndex, sString); }

    EServerAccess   GetServerAccess() const                 { return EServerAccess(m_yServerAccess); }
    void            SetServerAccess(EServerAccess eAccess)  { m_yServerAccess = byte(eAccess); }

    byte            GetHostFlags() const                    { return m_yHostFlags; }
    void            SetHostFlags(byte yHostFlags)           { m_yHostFlags = yHostFlags; }

    EStatsStatus    GetStatsStatus() const                  { return EStatsStatus(m_yStatsStatus); }
    void            SetStatsStatus(EStatsStatus eStatsStatus)   { m_yStatsStatus = byte(eStatsStatus); }

    // Voting
    GAME_SHARED_API void    StartVote(EVoteType eType, int iTarget, uint uiDuration, uint uiStartTime);
    GAME_SHARED_API void    VoteFailed();
    GAME_SHARED_API void    VotePassed();
    void                    ResetVote()                     { m_yActiveVote = VOTE_TYPE_INVALID; m_iVoteTarget = -1; m_uiVoteEndTime = INVALID_TIME; m_yVotesRequired = 0; m_yYesVotes = 0; }
    byte                    GetActiveVoteType() const       { return m_yActiveVote; }
    int                     GetVoteTarget() const           { return m_iVoteTarget; }
    uint                    GetVoteEndTime() const          { return m_uiVoteEndTime; }

    void                    SetVotesRequired(byte yVotes)   { m_yVotesRequired = yVotes; }
    byte                    GetVotesRequired() const        { return m_yVotesRequired; }

    void                    SetYesVotes(byte yVotes)        { m_yYesVotes = yVotes; }
    byte                    GetYesVotes() const             { return m_yYesVotes; }

    void        ClearFlags()                                { m_yFlags = 0; }
    void        SetFlags(byte yFlags)                       { m_yFlags |= yFlags; }
    void        RemoveFlags(byte yFlags)                    { m_yFlags &= ~yFlags; }
    bool        HasFlags(byte yFlags) const                 { return (m_yFlags & yFlags) == yFlags; }

    void        SetGamePhaseEndTime(uint uiTime)            { if (m_uiPhaseStartTime > uiTime) m_uiPhaseDuration = 0; else m_uiPhaseDuration = uiTime - m_uiPhaseStartTime; }
    uint        GetGamePhase() const                        { return m_uiGamePhase; }
    uint        GetPhaseStartTime() const                   { return m_uiPhaseStartTime; }
    uint        GetPhaseDuration() const                    { return m_uiPhaseDuration; }
    uint        GetPhaseEndTime() const                     { if (m_uiPhaseStartTime == INVALID_TIME || m_uiPhaseDuration == INVALID_TIME) return INVALID_TIME; return m_uiPhaseStartTime + m_uiPhaseDuration; }
    uint        GetMatchID() const                          { return m_uiMatchID; }

    void        SetGameMode(uint uiMode)                    { m_uiGameMode = uiMode; }
    uint        GetGameMode() const                         { return m_uiGameMode; }

    void        SetTeamSize(uint uiTeamSize)                { m_uiTeamSize = uiTeamSize; }
    uint        GetTeamSize() const                         { return m_uiTeamSize; }
    
    GAME_SHARED_API uint    GetCurrentSpectatorCount() const;
    GAME_SHARED_API uint    GetCurrentRefereeCount() const; 

    void        SetMaxSpectators(uint uiMaxSpectators)      { m_uiMaxSpectators = uiMaxSpectators; }
    uint        GetMaxSpectators() const                    { return m_uiMaxSpectators; }

    void        SetMaxReferees(uint uiMaxReferees)          { m_uiMaxReferees = uiMaxReferees; }
    uint        GetMaxReferees() const                      { return m_uiMaxReferees; }

    uint        GetMatchLength() const                      { return m_uiMatchLength; }
    void        SetMatchLength(uint uiLength)               { m_uiMatchLength = uiLength; }
    
    ushort      GetMinPSR() const                           { return m_unMinPSR; }
    ushort      GetMaxPSR() const                           { return m_unMaxPSR; }
    void        SetMinPSR(const ushort unMinPSR)            { m_unMinPSR = unMinPSR; }
    void        SetMaxPSR(const ushort unMaxPSR)            { m_unMaxPSR = unMaxPSR; }  

    uint        GetFirstBanTeam() const                     { return m_uiFirstBanTeam; }
    void        SetFirstBanTeam(uint uiFirstBanTeam)        { m_uiFirstBanTeam = uiFirstBanTeam; }

    GAME_SHARED_API void        ExecuteActionScript(EEntityActionScript eScript, IGameEntity *pInitiator, IGameEntity *pInflictor, IGameEntity *pTarget, const CVec3f &v3Target);

    GAME_SHARED_API void        SetGameOptions(const tstring &sOptions);
    void                        SetGameOptions(uint uiOption)               { m_uiGameOptions |= uiOption; }
    bool                        HasGameOptions(uint uiOption) const         { return (m_uiGameOptions & uiOption) == uiOption; }
    void                        ClearGameOptions(uint uiOption)             { m_uiGameOptions &= ~uiOption; }
    uint                        GetGameOptions() const                      { return m_uiGameOptions; }

    GAME_SHARED_API static uint     GetGameOptionFromString(const tstring &sOption);
    GAME_SHARED_API static tstring  GetGameOptionName(uint uiOption);
    GAME_SHARED_API static uint     GetGameModeFromString(const tstring &sMode);
    GAME_SHARED_API static tstring  GetGameModeName(uint uiMode);
    GAME_SHARED_API static tstring  GetGameModeString(uint uiMode);
    GAME_SHARED_API static tstring  GetGameOptionsString(uint uiOptions);
    GAME_SHARED_API static tstring  GetGameOptionsNamesString(uint uiOptions);

    ENTITY_DEFINITION_ACCESSOR(uint, StartingGold)
    ENTITY_DEFINITION_ACCESSOR(uint, RepickCost)
    ENTITY_DEFINITION_ACCESSOR(uint, RandomBonus)
    ENTITY_DEFINITION_ACCESSOR(uint, HeroPoolSize)
    ENTITY_DEFINITION_ACCESSOR(uint, BanCount)
    ENTITY_DEFINITION_ACCESSOR(uint, GoldPerTick)
    ENTITY_DEFINITION_ACCESSOR(uint, IncomeInterval)
    ENTITY_DEFINITION_ACCESSOR(float, ExperienceMultiplier)
    ENTITY_DEFINITION_ACCESSOR(float, TowerDenyGoldMultiplier)
    ENTITY_DEFINITION_ACCESSOR(uint, ExtraTime)

    ENTITY_DEFINITION_ACCESSOR(bool, AlternatePicks)
    ENTITY_DEFINITION_ACCESSOR(bool, NoLobby)
    ENTITY_DEFINITION_ACCESSOR(bool, NoHeroSelect)
    ENTITY_DEFINITION_ACCESSOR(bool, NoDev)

    GAME_SHARED_API static void WriteStringTable(CFileHandle &hFile, size_t zTabStop, size_t zColumnOffset);

    bool        CanLeave(uint uiTeam) const;

    bool        HasScriptValue(const tstring &sName) const                              { return m_mapScriptValues.find(sName) != m_mapScriptValues.end(); }
    void        SetScriptValue(const tstring &sName, const tstring &sValue)             { m_mapScriptValues[sName] = sValue; }
    int         GetScriptValueInt(const tstring &sName, int iDefault = 0) const         { if (HasScriptValue(sName)) return AtoI(GetScriptValue(sName)); return iDefault; }
    float       GetScriptValueFloat(const tstring &sName, float fDefault = 0.0f) const  { if (HasScriptValue(sName)) return AtoF(GetScriptValue(sName)); return fDefault; }
    bool        GetScriptValueBool(const tstring &sName, bool bDefault = false) const   { if (HasScriptValue(sName)) return AtoB(GetScriptValue(sName)); return bDefault; }
    tstring     GetScriptValue(const tstring &sName, const tstring &sDefault = TSNULL) const
    {
        map<tstring, tstring>::const_iterator findit(m_mapScriptValues.find(sName));
        if (findit != m_mapScriptValues.end())
            return findit->second;
        else
            return sDefault;
    }
};
//=============================================================================

#endif //__C_GAMEINFO_H__
