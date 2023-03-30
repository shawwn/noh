// (C)2007 S2 Games
// c_gameinfo.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_gameinfo.h"
#include "c_teaminfo.h"

#include "../k2/c_snapshot.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
uint    CGameInfo::s_uiBaseType(ENTITY_BASE_TYPE_GAME_INFO);

DEFINE_ENTITY_DESC(CGameInfo, 2)
{
    s_cDesc.pFieldTypes = K2_NEW(g_heapTypeVector,    TypeVector)();
    s_cDesc.pFieldTypes->clear();
    const TypeVector &vBase(IGameEntity::GetTypeVector());
    s_cDesc.pFieldTypes->insert(s_cDesc.pFieldTypes->begin(), vBase.begin(), vBase.end());
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_unServerDateIndex"), TYPE_SHORT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_unServerTimeIndex"), TYPE_SHORT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_unServerNameIndex"), TYPE_SHORT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_unGameNameIndex"), TYPE_SHORT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_yFlags"), TYPE_CHAR, 8, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiGamePhase"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiPhaseStartTime"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiPhaseDuration"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiMatchID"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiGameMode"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiGameOptions"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiTeamSize"), TYPE_INT, 4, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiMaxSpectators"), TYPE_INT, 4, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiMaxReferees"), TYPE_INT, 4, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiMatchLength"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_yServerAccess"), TYPE_CHAR, 8, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_yHostFlags"), TYPE_CHAR, 8, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_yActiveVote"), TYPE_CHAR, 8, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_iVoteTarget"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiVoteEndTime"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_yVotesRequired"), TYPE_CHAR, 4, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_yYesVotes"), TYPE_CHAR, 4, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_unMinPSR"), TYPE_SHORT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_unMaxPSR"), TYPE_SHORT, 0, 0));
}
//=============================================================================

/*====================
  CGameInfo::GetGameModeFromString
  ====================*/
uint    CGameInfo::GetGameModeFromString(const tstring &sMode)
{
    if (CompareNoCase(sMode, _T("normal")) == 0)
        return GAME_MODE_NORMAL;
    else if (CompareNoCase(sMode, _T("randomdraft")) == 0 || CompareNoCase(sMode, _T("rd")) == 0)
        return GAME_MODE_RANDOM_DRAFT;
    else if (CompareNoCase(sMode, _T("singledraft")) == 0 || CompareNoCase(sMode, _T("sd")) == 0)
        return GAME_MODE_SINGLE_DRAFT;
    //else if (CompareNoCase(sMode, _T("deathmatch")) == 0 || CompareNoCase(sMode, _T("dm")) == 0)
    //  return GAME_MODE_DEATHMATCH;
    else if (CompareNoCase(sMode, _T("banningdraft")) == 0 || CompareNoCase(sMode, _T("bd")) == 0)
        return GAME_MODE_BANNING_DRAFT;
    else if (CompareNoCase(sMode, _T("captainsdraft")) == 0 || CompareNoCase(sMode, _T("cd")) == 0)
        return GAME_MODE_CAPTAINS_DRAFT;
    else if (CompareNoCase(sMode, _T("captainsmode")) == 0 || CompareNoCase(sMode, _T("cm")) == 0)
        return GAME_MODE_CAPTAINS_MODE;
    else if (CompareNoCase(sMode, _T("banningpick")) == 0 || CompareNoCase(sMode, _T("bp")) == 0)
        return GAME_MODE_BANNING_PICK;
    else
        return GAME_MODE_NORMAL;
}


/*====================
  CGameInfo::GetGameModeName
  ====================*/
tstring CGameInfo::GetGameModeName(uint uiMode)
{
    if (uiMode == GAME_MODE_NORMAL)                 return _T("normal");
    else if (uiMode == GAME_MODE_RANDOM_DRAFT)      return _T("randomdraft");
    else if (uiMode == GAME_MODE_SINGLE_DRAFT)      return _T("singledraft");
    else if (uiMode == GAME_MODE_DEATHMATCH)        return _T("deathmatch");
    else if (uiMode == GAME_MODE_BANNING_DRAFT)     return _T("banningdraft");
    else if (uiMode == GAME_MODE_CAPTAINS_DRAFT)    return _T("captainsdraft");
    else if (uiMode == GAME_MODE_CAPTAINS_MODE)     return _T("captainsmode");
    else if (uiMode == GAME_MODE_BANNING_PICK)      return _T("banningpick");
    else return TSNULL;
}


/*====================
  CGameInfo::GetGameModeString
  ====================*/
tstring CGameInfo::GetGameModeString(uint uiMode)
{
    switch (uiMode)
    {
    case GAME_MODE_NORMAL:          return Game.GetEntityString(_T("Mode_Normal"));
    case GAME_MODE_RANDOM_DRAFT:    return Game.GetEntityString(_T("Mode_RandomDraft"));
    case GAME_MODE_SINGLE_DRAFT:    return Game.GetEntityString(_T("Mode_SingleDraft"));
    case GAME_MODE_DEATHMATCH:      return Game.GetEntityString(_T("Mode_Deathmatch"));
    case GAME_MODE_BANNING_DRAFT:   return Game.GetEntityString(_T("Mode_BanningDraft"));
    case GAME_MODE_CAPTAINS_DRAFT:  return Game.GetEntityString(_T("Mode_CaptainsDraft"));
    case GAME_MODE_CAPTAINS_MODE:   return Game.GetEntityString(_T("Mode_Captains"));
    case GAME_MODE_BANNING_PICK:    return Game.GetEntityString(_T("Mode_BanningPick"));
    }

    return Game.GetEntityString(_T("Mode_Invalid"));
}


/*====================
  CGameInfo::GetGameOptionFromString
  ====================*/
uint    CGameInfo::GetGameOptionFromString(const tstring &sOption)
{
    if (CompareNoCase(sOption, _T("forcerandom")) == 0)
        return GAME_OPTION_FORCE_RANDOM;
    else if (CompareNoCase(sOption, _T("alternatepicks")) == 0)
        return GAME_OPTION_ALTERNATE_SELECTION;
    else if (CompareNoCase(sOption, _T("norepick")) == 0)
        return GAME_OPTION_NO_REPICK;
    else if (CompareNoCase(sOption, _T("noswap")) == 0)
        return GAME_OPTION_NO_SWAP;
    else if (CompareNoCase(sOption, _T("noagility")) == 0)
        return GAME_OPTION_NO_AGILITY;
    else if (CompareNoCase(sOption, _T("nointelligence")) == 0)
        return GAME_OPTION_NO_INTELLIGENCE;
    else if (CompareNoCase(sOption, _T("nostrength")) == 0)
        return GAME_OPTION_NO_STRENGTH;
    else if (CompareNoCase(sOption, _T("norespawntimer")) == 0)
        return GAME_OPTION_NO_RESPAWN_TIMER;
    else if (CompareNoCase(sOption, _T("dropitems")) == 0)
        return GAME_OPTION_DROP_ITEMS;
    else if (CompareNoCase(sOption, _T("nopowerups")) == 0)
        return GAME_OPTION_NO_POWERUPS;
    else if (CompareNoCase(sOption, _T("supercreeps")) == 0)
        return GAME_OPTION_SUPER_CREEPS;
    else if (CompareNoCase(sOption, _T("easymode")) == 0)
        return GAME_OPTION_EASY_MODE;
    else if (CompareNoCase(sOption, _T("allowduplicate")) == 0)
        return GAME_OPTION_DUPLICATE_HEROES;
    else if (CompareNoCase(sOption, _T("reverseselect")) == 0)
        return GAME_OPTION_REVERSE_SELECTION;
    else if (CompareNoCase(sOption, _T("notop")) == 0)
        return GAME_OPTION_NO_TOP_LANE;
    else if (CompareNoCase(sOption, _T("nomiddle")) == 0)
        return GAME_OPTION_NO_MIDDLE_LANE;
    else if (CompareNoCase(sOption, _T("nobottom")) == 0)
        return GAME_OPTION_NO_BOTTOM_LANE;
    else if (CompareNoCase(sOption, _T("allowveto")) == 0)
        return GAME_OPTION_ALLOW_VETO;
    else if (CompareNoCase(sOption, _T("shuffleteams")) == 0)
        return GAME_OPTION_SHUFFLE_TEAMS;
    else if (CompareNoCase(sOption, _T("autobalance")) == 0)
        return GAME_OPTION_AUTOBALANCE_TEAMS;
    else if (CompareNoCase(sOption, _T("allheroes")) == 0)
        return GAME_OPTION_ALL_HEROES;
    else if (CompareNoCase(sOption, _T("tournamentrules")) == 0)
        return GAME_OPTION_TOURNAMENT_RULES;
    else if (CompareNoCase(sOption, _T("hardcore")) == 0)
        return GAME_OPTION_HARDCORE;
    else if (CompareNoCase(sOption, _T("devheroes")) == 0)
        return GAME_OPTION_DEV_HEROES;
    else
        return GAME_OPTION_INVALID;
}


/*====================
  CGameInfo::GetGameOptionName
  ====================*/
tstring CGameInfo::GetGameOptionName(uint uiOption)
{
    if (uiOption == GAME_OPTION_FORCE_RANDOM)               return _T("forcerandom");
    else if (uiOption == GAME_OPTION_ALTERNATE_SELECTION)   return _T("alternatepicks");
    else if (uiOption == GAME_OPTION_NO_REPICK)             return _T("norepick");
    else if (uiOption == GAME_OPTION_NO_SWAP)               return _T("noswap");
    else if (uiOption == GAME_OPTION_NO_AGILITY)            return _T("noagility");
    else if (uiOption == GAME_OPTION_NO_INTELLIGENCE)       return _T("nointelligence");
    else if (uiOption == GAME_OPTION_NO_STRENGTH)           return _T("nostrength");
    else if (uiOption == GAME_OPTION_NO_RESPAWN_TIMER)      return _T("norespawntimer");
    else if (uiOption == GAME_OPTION_DROP_ITEMS)            return _T("dropitems");
    else if (uiOption == GAME_OPTION_NO_POWERUPS)           return _T("nopowerups");
    else if (uiOption == GAME_OPTION_SUPER_CREEPS)          return _T("supercreeps");
    else if (uiOption == GAME_OPTION_EASY_MODE)             return _T("easymode");
    else if (uiOption == GAME_OPTION_DUPLICATE_HEROES)      return _T("allowduplicate");
    else if (uiOption == GAME_OPTION_REVERSE_SELECTION)     return _T("reverseselect");
    else if (uiOption == GAME_OPTION_NO_TOP_LANE)           return _T("notop");
    else if (uiOption == GAME_OPTION_NO_MIDDLE_LANE)        return _T("nomiddle");
    else if (uiOption == GAME_OPTION_NO_BOTTOM_LANE)        return _T("nobottom");
    else if (uiOption == GAME_OPTION_ALLOW_VETO)            return _T("allowveto");
    else if (uiOption == GAME_OPTION_SHUFFLE_TEAMS)         return _T("shuffleteams");
    else if (uiOption == GAME_OPTION_AUTOBALANCE_TEAMS)     return _T("autobalance");
    else if (uiOption == GAME_OPTION_ALL_HEROES)            return _T("allheroes");
    else if (uiOption == GAME_OPTION_TOURNAMENT_RULES)      return _T("tournamentrules");
    else if (uiOption == GAME_OPTION_HARDCORE)              return _T("hardcore");
    else if (uiOption == GAME_OPTION_DEV_HEROES)            return _T("devheroes");
    else return TSNULL;
}


/*====================
  CGameInfo::GetGameOptionsString
  ====================*/
tstring CGameInfo::GetGameOptionsString(uint uiOptions)
{
    tstring sReturn;

    if ((uiOptions & GAME_OPTION_FORCE_RANDOM) != 0) { if (!sReturn.empty()) sReturn += _T(", "); sReturn += Game.GetEntityString(_T("Option_ForceRandom")); }
    if ((uiOptions & GAME_OPTION_ALTERNATE_SELECTION) != 0) { if (!sReturn.empty()) sReturn += _T(", "); sReturn += Game.GetEntityString(_T("Option_AlternateSelection")); }
    if ((uiOptions & GAME_OPTION_NO_REPICK) != 0) { if (!sReturn.empty()) sReturn += _T(", "); sReturn += Game.GetEntityString(_T("Option_NoRepick")); }
    if ((uiOptions & GAME_OPTION_NO_SWAP) != 0) { if (!sReturn.empty()) sReturn += _T(", "); sReturn += Game.GetEntityString(_T("Option_NoSwap")); }
    if ((uiOptions & GAME_OPTION_NO_AGILITY) != 0) { if (!sReturn.empty()) sReturn += _T(", "); sReturn += Game.GetEntityString(_T("Option_NoAgility")); }
    if ((uiOptions & GAME_OPTION_NO_INTELLIGENCE) != 0) { if (!sReturn.empty()) sReturn += _T(", "); sReturn += Game.GetEntityString(_T("Option_NoIntelligence")); }
    if ((uiOptions & GAME_OPTION_NO_STRENGTH) != 0) { if (!sReturn.empty()) sReturn += _T(", "); sReturn += Game.GetEntityString(_T("Option_NoStrength")); }
    if ((uiOptions & GAME_OPTION_ALL_HEROES) != 0) { if (!sReturn.empty()) sReturn += _T(", "); sReturn += Game.GetEntityString(_T("Option_AllHeroes")); }
    if ((uiOptions & GAME_OPTION_NO_RESPAWN_TIMER) != 0) { if (!sReturn.empty()) sReturn += _T(", "); sReturn += Game.GetEntityString(_T("Option_NoRespawnTimer")); }
    if ((uiOptions & GAME_OPTION_DROP_ITEMS) != 0) { if (!sReturn.empty()) sReturn += _T(", "); sReturn += Game.GetEntityString(_T("Option_DropItems")); }
    if ((uiOptions & GAME_OPTION_NO_POWERUPS) != 0) { if (!sReturn.empty()) sReturn += _T(", "); sReturn += Game.GetEntityString(_T("Option_NoPowerups")); }
    if ((uiOptions & GAME_OPTION_SUPER_CREEPS) != 0) { if (!sReturn.empty()) sReturn += _T(", "); sReturn += Game.GetEntityString(_T("Option_SuperCreeps")); }
    if ((uiOptions & GAME_OPTION_EASY_MODE) != 0) { if (!sReturn.empty()) sReturn += _T(", "); sReturn += Game.GetEntityString(_T("Option_EasyMode")); }
    if ((uiOptions & GAME_OPTION_DUPLICATE_HEROES) != 0) { if (!sReturn.empty()) sReturn += _T(", "); sReturn += Game.GetEntityString(_T("Option_DuplicateHeroes")); }
    if ((uiOptions & GAME_OPTION_REVERSE_SELECTION) != 0) { if (!sReturn.empty()) sReturn += _T(", "); sReturn += Game.GetEntityString(_T("Option_ReverseSelection")); }
    if ((uiOptions & GAME_OPTION_NO_TOP_LANE) != 0) { if (!sReturn.empty()) sReturn += _T(", "); sReturn += Game.GetEntityString(_T("Option_NoTopLane")); }
    if ((uiOptions & GAME_OPTION_NO_MIDDLE_LANE) != 0) { if (!sReturn.empty()) sReturn += _T(", "); sReturn += Game.GetEntityString(_T("Option_NoMiddleLane")); }
    if ((uiOptions & GAME_OPTION_NO_BOTTOM_LANE) != 0) { if (!sReturn.empty()) sReturn += _T(", "); sReturn += Game.GetEntityString(_T("Option_NoBottomLane")); }
    if ((uiOptions & GAME_OPTION_ALLOW_VETO) != 0) { if (!sReturn.empty()) sReturn += _T(", "); sReturn += Game.GetEntityString(_T("Option_AllowVeto")); }
    if ((uiOptions & GAME_OPTION_SHUFFLE_TEAMS) != 0) { if (!sReturn.empty()) sReturn += _T(", "); sReturn += Game.GetEntityString(_T("Option_ShuffleTeams")); }
    if ((uiOptions & GAME_OPTION_AUTOBALANCE_TEAMS) != 0) { if (!sReturn.empty()) sReturn += _T(", "); sReturn += Game.GetEntityString(_T("Option_AutoBalanceTeams")); }
    if ((uiOptions & GAME_OPTION_TOURNAMENT_RULES) != 0) { if (!sReturn.empty()) sReturn += _T(", "); sReturn += Game.GetEntityString(_T("Option_TournamentRules")); }
    if ((uiOptions & GAME_OPTION_HARDCORE) != 0) { if (!sReturn.empty()) sReturn += _T(", "); sReturn += Game.GetEntityString(_T("Option_Hardcore")); }
    if ((uiOptions & GAME_OPTION_DEV_HEROES) != 0) { if (!sReturn.empty()) sReturn += _T(", "); sReturn += Game.GetEntityString(_T("Option_DevHeroes")); }

    if (sReturn.empty())
        sReturn = Game.GetEntityString(_T("Option_None"));

    return sReturn;
}


/*====================
  CGameInfo::GetGameOptionsNamesString
  ====================*/
tstring CGameInfo::GetGameOptionsNamesString(uint uiOptions)
{
    tstring sReturn;

    for (uint ui(0); ui < 32; ++ui)
    {
        uint uiBit(BIT(ui));

        if ((uiOptions & uiBit) != 0)
        {
            if (!sReturn.empty())
                sReturn += _T(",");
            sReturn += GetGameOptionName(uiBit);
        }
    }

    return sReturn;
}


/*====================
  CGameInfo::CGameInfo
  ====================*/
CGameInfo::CGameInfo() :
IGameEntity(NULL),
m_unServerDateIndex(INVALID_NETWORK_STRING),
m_unServerTimeIndex(INVALID_NETWORK_STRING),
m_unServerNameIndex(INVALID_NETWORK_STRING),
m_unGameNameIndex(INVALID_NETWORK_STRING),
m_uiGamePhase(GAME_PHASE_IDLE),
m_uiPhaseStartTime(INVALID_TIME),
m_uiPhaseDuration(INVALID_TIME),
m_uiMatchID(-1),

m_uiGameMode(GAME_MODE_NORMAL),
m_uiGameOptions(0),
m_uiTeamSize(0),
m_uiMaxSpectators(0),
m_uiMaxReferees(0),
m_uiMatchLength(INVALID_TIME),
m_yServerAccess(byte(ACCESS_PUBLIC)),
m_yHostFlags(0),

m_yActiveVote(VOTE_TYPE_INVALID),
m_iVoteTarget(-1),
m_uiVoteEndTime(INVALID_TIME),
m_yVotesRequired(0),
m_yYesVotes(0),

m_unMinPSR(0),
m_unMaxPSR(0),
m_uiFirstBanTeam(TEAM_1)
{
}


/*====================
  CGameInfo::GetSnapshot
  ====================*/
void    CGameInfo::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
    IGameEntity::GetSnapshot(snapshot, uiFlags);

    snapshot.WriteField(m_unServerDateIndex);
    snapshot.WriteField(m_unServerTimeIndex);
    snapshot.WriteField(m_unServerNameIndex);
    snapshot.WriteField(m_unGameNameIndex);

    snapshot.WriteField(m_yFlags);
    snapshot.WriteField(m_uiGamePhase);
    snapshot.WriteField(m_uiPhaseStartTime);
    snapshot.WriteField(m_uiPhaseDuration);
    snapshot.WriteField(m_uiMatchID);
    snapshot.WriteField(m_uiGameMode);
    snapshot.WriteField(m_uiGameOptions);
    snapshot.WriteField(m_uiTeamSize);
    snapshot.WriteField(m_uiMaxSpectators);
    snapshot.WriteField(m_uiMaxReferees);
    snapshot.WriteField(m_uiMatchLength);
    snapshot.WriteField(m_yServerAccess);
    snapshot.WriteField(m_yHostFlags);

    snapshot.WriteField(m_yActiveVote);
    snapshot.WriteField(m_iVoteTarget);
    snapshot.WriteField(m_uiVoteEndTime);
    snapshot.WriteField(m_yVotesRequired);
    snapshot.WriteField(m_yYesVotes);
    
    snapshot.WriteField(m_unMinPSR);
    snapshot.WriteField(m_unMaxPSR);
    
}


/*====================
  CGameInfo::ReadSnapshot
  ====================*/
bool    CGameInfo::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
    try
    {
        IGameEntity::ReadSnapshot(snapshot, 1);

        snapshot.ReadField(m_unServerDateIndex);
        snapshot.ReadField(m_unServerTimeIndex);
        snapshot.ReadField(m_unServerNameIndex);
        snapshot.ReadField(m_unGameNameIndex);

        snapshot.ReadField(m_yFlags);
        snapshot.ReadField(m_uiGamePhase);
        snapshot.ReadField(m_uiPhaseStartTime);
        snapshot.ReadField(m_uiPhaseDuration);
        snapshot.ReadField(m_uiMatchID);
        snapshot.ReadField(m_uiGameMode);
        snapshot.ReadField(m_uiGameOptions);
        snapshot.ReadField(m_uiTeamSize);
        snapshot.ReadField(m_uiMaxSpectators);
        snapshot.ReadField(m_uiMaxReferees);
        snapshot.ReadField(m_uiMatchLength);
        snapshot.ReadField(m_yServerAccess);
        snapshot.ReadField(m_yHostFlags);
        
        snapshot.ReadField(m_yActiveVote);
        snapshot.ReadField(m_iVoteTarget);
        snapshot.ReadField(m_uiVoteEndTime);
        snapshot.ReadField(m_yVotesRequired);
        snapshot.ReadField(m_yYesVotes);
        
        if (uiVersion >= 2)
        {
            snapshot.ReadField(m_unMinPSR);
            snapshot.ReadField(m_unMaxPSR);
        }

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CGameInfo::ReadSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  CGameInfo::Initialize
  ====================*/
void    CGameInfo::Initialize()
{
    if (!Game.IsServer())
        return;

    m_unServerDateIndex = NetworkResourceManager.ReserveString();
    m_unServerTimeIndex = NetworkResourceManager.ReserveString();
    m_unServerNameIndex = NetworkResourceManager.ReserveString();
    m_unGameNameIndex = NetworkResourceManager.ReserveString();
}


/*====================
  CGameInfo::ValidateOptions()
  ====================*/
void    CGameInfo::ValidateOptions()
{
    if (!Game.IsServer())
        return;

    if (HasGameOptions(GAME_OPTION_NO_AGILITY | GAME_OPTION_NO_INTELLIGENCE | GAME_OPTION_NO_STRENGTH))
        ClearGameOptions(GAME_OPTION_NO_AGILITY | GAME_OPTION_NO_INTELLIGENCE | GAME_OPTION_NO_STRENGTH);

    if (HasGameOptions(GAME_OPTION_NO_TOP_LANE | GAME_OPTION_NO_MIDDLE_LANE | GAME_OPTION_NO_BOTTOM_LANE))
        ClearGameOptions(GAME_OPTION_NO_TOP_LANE | GAME_OPTION_NO_MIDDLE_LANE | GAME_OPTION_NO_BOTTOM_LANE);

    if (GetAlternatePicks())
        m_uiGameOptions |= GAME_OPTION_ALTERNATE_SELECTION;

    // Force all heroes of Abaddon
    m_uiGameOptions |= GAME_OPTION_ALL_HEROES;
}


/*====================
  CGameInfo::MatchRemake
  ====================*/
void    CGameInfo::MatchRemake()
{
    SetGamePhase(GAME_PHASE_WAITING_FOR_PLAYERS);
    ClearFlags();
}


/*====================
  CGameInfo::Baseline
  ====================*/
void    CGameInfo::Baseline()
{
    IGameEntity::Baseline();

    m_unServerDateIndex = INVALID_NETWORK_STRING;
    m_unServerTimeIndex = INVALID_NETWORK_STRING;
    m_unServerNameIndex = INVALID_NETWORK_STRING;
    m_unGameNameIndex = INVALID_NETWORK_STRING;

    m_yFlags = 0;
    m_uiGamePhase = GAME_PHASE_IDLE;
    m_uiPhaseStartTime = INVALID_TIME;
    m_uiPhaseDuration = INVALID_TIME;
    m_uiMatchID = -1;
    m_uiGameMode = GAME_MODE_NORMAL;
    m_uiGameOptions = 0;
    m_uiTeamSize = 0;
    m_uiMaxSpectators = 0;
    m_uiMaxReferees = 0;
    m_uiMatchLength = INVALID_TIME;
    
    m_yServerAccess = byte(ACCESS_PUBLIC);
    m_yHostFlags = 0;

    m_yActiveVote = VOTE_TYPE_INVALID;
    m_iVoteTarget = -1;
    m_uiVoteEndTime = INVALID_TIME;
    m_yVotesRequired = 0;
    m_yYesVotes = 0;
    
    m_unMinPSR = 0;
    m_unMaxPSR = 0; 
}


/*====================
  CGameInfo::SetGamePhase
  ====================*/
void    CGameInfo::SetGamePhase(EGamePhase eGamePhase, uint uiDuration, uint uiStartTime)
{
    m_uiGamePhase = eGamePhase;
    m_uiPhaseStartTime = uiStartTime == INVALID_TIME ? Game.GetGameTime() : uiStartTime;
    m_uiPhaseDuration = uiDuration;

    if (Game.IsServer())
    {
        CBufferFixed<10> buffer;
        buffer << GAME_CMD_PHASE << byte(eGamePhase) << uiDuration << uiStartTime;
        Game.BroadcastGameData(buffer, true);
    }
}


/*====================
  CGameInfo::StartVote
  ====================*/
void    CGameInfo::StartVote(EVoteType eType, int iTarget, uint uiDuration, uint uiStartTime)
{
    m_yActiveVote = byte(eType);
    m_iVoteTarget = iTarget;
    m_uiVoteEndTime = uiStartTime + uiDuration;
}


/*====================
  CGameInfo::VoteFailed
  ====================*/
void    CGameInfo::VoteFailed()
{
    ResetVote();
    CBufferFixed<1> buffer;
    buffer << GAME_CMD_VOTE_FAILED_MESSAGE;
    Game.BroadcastGameData(buffer, true);
}


/*====================
  CGameInfo::VotePassed
  ====================*/
void    CGameInfo::VotePassed()
{
    ResetVote();
    CBufferFixed<1> buffer;
    buffer << GAME_CMD_VOTE_PASSED_MESSAGE;
    Game.BroadcastGameData(buffer, true);
}


/*====================
  CGameInfo::SetGameOptions
  ====================*/
void    CGameInfo::SetGameOptions(const tstring &sOptions)
{
    if (GetGamePhase() != GAME_PHASE_IDLE)
        return;

    tsvector vOptions(TokenizeString(sOptions, _T(',')));
    for (tsvector_it it(vOptions.begin()); it != vOptions.end(); ++it)
    {
        if (it->empty())
            continue;
        if ((*it)[0] == _T('+'))
            SetGameOptions(GetGameOptionFromString(it->substr(1)));
        else if ((*it)[1] == _T('-'))
            ClearGameOptions(GetGameOptionFromString(it->substr(1)));
    }
}


/*====================
  CGameInfo::WriteStringTable
  ====================*/
void    CGameInfo::WriteStringTable(CFileHandle &hFile, size_t zTabStop, size_t zColumnOffset)
{
    hFile << _CWS("// Game Info") << newl;

    hFile << TabPad(_CWS("Mode_Normal"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameModeString(GAME_MODE_NORMAL)) << newl;
    hFile << TabPad(_CWS("Mode_RandomDraft"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameModeString(GAME_MODE_RANDOM_DRAFT)) << newl;
    hFile << TabPad(_CWS("Mode_SingleDraft"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameModeString(GAME_MODE_SINGLE_DRAFT)) << newl;
    hFile << TabPad(_CWS("Mode_Deathmatch"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameModeString(GAME_MODE_DEATHMATCH)) << newl;
    hFile << TabPad(_CWS("Mode_BanningDraft"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameModeString(GAME_MODE_BANNING_DRAFT)) << newl;
    hFile << TabPad(_CWS("Mode_CaptainsDraft"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameModeString(GAME_MODE_CAPTAINS_DRAFT)) << newl;
    hFile << TabPad(_CWS("Mode_CaptainsMode"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameModeString(GAME_MODE_CAPTAINS_MODE)) << newl;
    hFile << TabPad(_CWS("Mode_BanningPick"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameModeString(GAME_MODE_BANNING_PICK)) << newl;
    hFile << TabPad(_CWS("Mode_Invalid"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameModeString(NUM_GAME_MODES)) << newl;

    hFile << newl;

    hFile << TabPad(_CWS("Option_None"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameOptionsString(GAME_OPTION_INVALID)) << newl;
    hFile << TabPad(_CWS("Option_ForceRandom"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameOptionsString(GAME_OPTION_FORCE_RANDOM)) << newl;
    hFile << TabPad(_CWS("Option_AlternateSelection"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameOptionsString(GAME_OPTION_ALTERNATE_SELECTION)) << newl;
    hFile << TabPad(_CWS("Option_NoRepick"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameOptionsString(GAME_OPTION_NO_REPICK)) << newl;
    hFile << TabPad(_CWS("Option_NoSwap"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameOptionsString(GAME_OPTION_NO_SWAP)) << newl;
    hFile << TabPad(_CWS("Option_NoAgility"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameOptionsString(GAME_OPTION_NO_AGILITY)) << newl;
    hFile << TabPad(_CWS("Option_NoIntelligence"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameOptionsString(GAME_OPTION_NO_INTELLIGENCE)) << newl;
    hFile << TabPad(_CWS("Option_NoStrength"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameOptionsString(GAME_OPTION_NO_STRENGTH)) << newl;
    hFile << TabPad(_CWS("Option_AllHeroes"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameOptionsString(GAME_OPTION_ALL_HEROES)) << newl;
    hFile << TabPad(_CWS("Option_NoRespawnTimer"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameOptionsString(GAME_OPTION_NO_RESPAWN_TIMER)) << newl;
    hFile << TabPad(_CWS("Option_DropItems"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameOptionsString(GAME_OPTION_DROP_ITEMS)) << newl;
    hFile << TabPad(_CWS("Option_NoPowerups"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameOptionsString(GAME_OPTION_NO_POWERUPS)) << newl;
    hFile << TabPad(_CWS("Option_SuperCreeps"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameOptionsString(GAME_OPTION_SUPER_CREEPS)) << newl;
    hFile << TabPad(_CWS("Option_EasyMode"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameOptionsString(GAME_OPTION_EASY_MODE)) << newl;
    hFile << TabPad(_CWS("Option_DuplicateHeroes"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameOptionsString(GAME_OPTION_DUPLICATE_HEROES)) << newl;
    hFile << TabPad(_CWS("Option_ReverseSelection"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameOptionsString(GAME_OPTION_REVERSE_SELECTION)) << newl;
    hFile << TabPad(_CWS("Option_NoTopLane"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameOptionsString(GAME_OPTION_NO_TOP_LANE)) << newl;
    hFile << TabPad(_CWS("Option_NoMiddleLane"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameOptionsString(GAME_OPTION_NO_MIDDLE_LANE)) << newl;
    hFile << TabPad(_CWS("Option_NoBottomLane"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameOptionsString(GAME_OPTION_NO_BOTTOM_LANE)) << newl;
    hFile << TabPad(_CWS("Option_AllowVeto"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameOptionsString(GAME_OPTION_ALLOW_VETO)) << newl;
    hFile << TabPad(_CWS("Option_ShuffleTeams"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameOptionsString(GAME_OPTION_SHUFFLE_TEAMS)) << newl;
    hFile << TabPad(_CWS("Option_AutoBalanceTeams"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameOptionsString(GAME_OPTION_AUTOBALANCE_TEAMS)) << newl;
    hFile << TabPad(_CWS("Option_TournamentRules"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameOptionsString(GAME_OPTION_TOURNAMENT_RULES)) << newl;
    hFile << TabPad(_CWS("Option_Hardcore"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameOptionsString(GAME_OPTION_HARDCORE)) << newl;
    hFile << TabPad(_CWS("Option_DevHeroes"), zTabStop, zColumnOffset) << EscapeWhiteSpace(GetGameOptionsString(GAME_OPTION_DEV_HEROES)) << newl;

    hFile << newl;
}


/*====================
  CGameInfo::CanLeave
  ====================*/
bool    CGameInfo::CanLeave(uint uiTeam) const
{
    if (m_uiMatchID == INVALID_INDEX)
        return true;
    else if (m_uiGamePhase <= GAME_PHASE_WAITING_FOR_PLAYERS)
        return true;
    else if (Game.GetWinningTeam() != TEAM_INVALID)
        return true;
    else if (HasFlags(GAME_FLAG_CONCEDED))
        return true;

    CTeamInfo *pTeam(Game.GetTeam(uiTeam));
    if (pTeam != NULL && pTeam->HasFlags(TEAM_FLAG_ABANDONED))
        return true;

    if (HasFlags(GAME_FLAG_SOLO))
        return true;

    return false;
}


/*====================
  CGameInfo::ExecuteActionScript
  ====================*/
void    CGameInfo::ExecuteActionScript(EEntityActionScript eScript, IGameEntity *pInitiator, IGameEntity *pInflictor, IGameEntity *pTarget, const CVec3f &v3Target)
{
    if (m_pDefinition == NULL)
        return;
        
    m_pDefinition->ExecuteActionScript(eScript, this, pInitiator, pInflictor, pTarget, v3Target, NULL, 1);
}


/*====================
  CGameInfo::GetCurrentSpectatorCount
  ====================*/
uint CGameInfo::GetCurrentSpectatorCount() const
{
    const PlayerMap &mapPlayers(Game.GetPlayerMap());
    uint uiSpectatorCount(0);
    
    for (PlayerMap_cit it(mapPlayers.begin()); it != mapPlayers.end(); ++it)
    {
        if (it->second->GetTeam() == TEAM_SPECTATOR && !it->second->IsReferee())
            uiSpectatorCount++;
    }
    
    return uiSpectatorCount;                                
}


/*====================
  CGameInfo::GetCurrentRefereeCount
  ====================*/
uint CGameInfo::GetCurrentRefereeCount() const
{
    const PlayerMap &mapPlayers(Game.GetPlayerMap());
    uint uiRefereeCount(0);
    
    for (PlayerMap_cit it(mapPlayers.begin()); it != mapPlayers.end(); ++it)
    {
        if (it->second->IsReferee())
            uiRefereeCount++;
    }
    
    return uiRefereeCount;                              
}

