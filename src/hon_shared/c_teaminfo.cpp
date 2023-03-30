// (C)2006 S2 Games
// c_teaminfo.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_teaminfo.h"

#include "c_player.h"
#include "i_buildingentity.h"
#include "i_heroentity.h"
#include "i_creepentity.h"
#include "c_triggerspawnpoint.h"
#include "c_gamestats.h"
#include "c_shopinfo.h"
#include "i_entityitem.h"

#include "../k2/c_snapshot.h"
#include "../k2/c_voiceserver.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
uint    CTeamInfo::s_uiBaseType(ENTITY_BASE_TYPE_TEAM_INFO);

CVAR_UINTF(     g_buildingDamageNotifyInterval,     20000,  CVAR_GAMECONFIG);
CVAR_UINTF(     g_buildingDamageNotifyMinDamage,    200,    CVAR_GAMECONFIG);

// NOTE: If g_teamAllowedPauses is increased past 3, you must increase the # of bits transmitted as well
CVAR_UINTF(     g_teamAllowedPauses,                3,      CVAR_GAMECONFIG);
CVAR_UINTF(     g_teamTargetTime,               10000,      CVAR_GAMECONFIG);

CVAR_FLOATF(    psf_teamRankWeighting,              2.0f,   CVAR_GAMECONFIG | CVAR_TRANSMIT);

DEFINE_ENT_ALLOCATOR3(TeamInfo, Info_Team)

DEFINE_ENTITY_DESC(CTeamInfo, 2)
{
    s_cDesc.pFieldTypes = K2_NEW(ctx_Game,    TypeVector)();
    s_cDesc.pFieldTypes->clear();
    const TypeVector &vBase(IGameEntity::GetTypeVector());
    s_cDesc.pFieldTypes->insert(s_cDesc.pFieldTypes->begin(), vBase.begin(), vBase.end());

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiTeamID"), TYPE_INT, 3, TEAM_INVALID));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiRosterChangeSequence"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiTeamSize"), TYPE_INT, 5, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_fWinChance"), TYPE_BYTEPERCENT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiBaseBuildingIndex"), TYPE_GAMEINDEX, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiStartingTowerCount"), TYPE_INT, 4, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiCurrentTowerCount"), TYPE_INT, 4, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiCurrentRangedCount"), TYPE_INT, 4, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiCurrentMeleeCount"), TYPE_INT, 4, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_unNameIndex"), TYPE_SHORT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_v4Color[0]"), TYPE_BYTEPERCENT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_v4Color[1]"), TYPE_BYTEPERCENT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_v4Color[2]"), TYPE_BYTEPERCENT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiStatsIndex"), TYPE_GAMEINDEX, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_yFlags"), TYPE_CHAR, 4, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_ySlotLocks"), TYPE_CHAR, 5, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_fBaseHealthPercent"), TYPE_FLOAT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_yRemainingPauses"), TYPE_CHAR, 2, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiBanCount"), TYPE_INT, 4, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiShopInfoIndex"), TYPE_INT, 32, 0));

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiTeamTarget"), TYPE_GAMEINDEX, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiTeamTargetTime"), TYPE_INT, 32, 0));

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_aStatTotals[TEAM_STAT_TOWER_KILLS]"), TYPE_INT, 16, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_aStatTotals[TEAM_STAT_TOWER_DENIES]"), TYPE_INT, 16, 0));
}
//=============================================================================

/*====================
  CTeamInfo::~CTeamInfo
  ====================*/
CTeamInfo::~CTeamInfo()
{
}


/*====================
  CTeamInfo::CTeamInfo
  ====================*/
CTeamInfo::CTeamInfo() :
IGameEntity(NULL),

m_bRosterChanged(true),
m_uiRosterChangeSequence(0),

m_uiTeamID(TEAM_INVALID),
m_unNameIndex(INVALID_NETWORK_STRING),
m_uiTeamSize(0),
m_ySlotLocks(0),

m_fWinChance(0.0f),

m_uiBaseBuildingIndex(INVALID_INDEX),
m_uiStartingTowerCount(0),
m_uiCurrentTowerCount(0),
m_uiCurrentRangedCount(0),
m_uiCurrentMeleeCount(0),
m_uiLastAttackNotifyTime(0),
m_bDamagedBuildingSetCleared(false),

m_yKillStreak(0),
m_bSentMegaCreepMessage(false),
m_bAllKilled(false),

m_yRemainingPauses(g_teamAllowedPauses),
m_uiBanCount(0),

m_yFlags(0),

m_uiLastBuildingAttackAnnouncement(INVALID_TIME),

m_uiShopInfoIndex(INVALID_INDEX),

m_uiExtraTime(0),
m_bUsingExtraTime(false),

m_uiLastIncomeTime(INVALID_TIME),

m_uiTeamTarget(INVALID_INDEX),
m_uiTeamTargetTime(INVALID_TIME)
{
    for (int i(0); i < NUM_TEAM_STATS; ++i)
        m_aStatTotals[i] = 0;
}


/*====================
  CTeamInfo::Baseline
  ====================*/
void    CTeamInfo::Baseline()
{
    IGameEntity::Baseline();

    m_uiTeamID = TEAM_INVALID;
    m_unNameIndex = INVALID_NETWORK_STRING;
    m_uiRosterChangeSequence = 0;
    m_uiTeamSize = 0;
    m_ySlotLocks = 0;
    m_fWinChance = 0.0f;
    m_uiBaseBuildingIndex = INVALID_INDEX;
    m_uiStartingTowerCount = 0;
    m_uiCurrentTowerCount = 0;
    m_uiCurrentRangedCount = 0;
    m_uiCurrentMeleeCount = 0;
    m_v4Color = WHITE;
    m_uiStatsIndex = INVALID_INDEX;
    m_yFlags = 0;
    m_fBaseHealthPercent = 1.0f;
    m_yRemainingPauses = g_teamAllowedPauses;
    m_uiBanCount = 0;
    m_uiLastBuildingAttackAnnouncement = INVALID_TIME;
    m_uiShopInfoIndex = INVALID_INDEX;

    m_uiTeamTarget = INVALID_INDEX;
    m_uiTeamTargetTime = INVALID_TIME;

    for (int i(0); i < NUM_TEAM_STATS; ++i)
        m_aStatTotals[i] = 0;
}


/*====================
  CTeamInfo::GetSnapshot
  ====================*/
void    CTeamInfo::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
    IGameEntity::GetSnapshot(snapshot, uiFlags);

    snapshot.WriteInteger(m_uiTeamID);
    snapshot.WriteField(m_uiRosterChangeSequence);
    snapshot.WriteField(m_uiTeamSize);
    snapshot.WriteBytePercent(m_fWinChance);
    snapshot.WriteGameIndex(m_uiBaseBuildingIndex);
    snapshot.WriteField(m_uiStartingTowerCount);
    snapshot.WriteField(m_uiCurrentTowerCount);
    snapshot.WriteField(m_uiCurrentRangedCount);
    snapshot.WriteField(m_uiCurrentMeleeCount);
    snapshot.WriteField(m_unNameIndex);
    snapshot.WriteBytePercent(m_v4Color[0]);
    snapshot.WriteBytePercent(m_v4Color[1]);
    snapshot.WriteBytePercent(m_v4Color[2]);
    snapshot.WriteGameIndex(m_uiStatsIndex);
    snapshot.WriteField(m_yFlags);
    snapshot.WriteField(m_ySlotLocks);
    snapshot.WriteField(m_fBaseHealthPercent);
    snapshot.WriteField(m_yRemainingPauses);
    snapshot.WriteField(m_uiBanCount);
    snapshot.WriteField(m_uiShopInfoIndex);

    if (uiFlags & SNAPSHOT_HIDDEN)
    {
    
        snapshot.WriteGameIndex(INVALID_INDEX);
        snapshot.WriteField(INVALID_TIME);

        for (int i(0); i < NUM_TEAM_STATS; ++i)
            snapshot.WriteField(uint(0));
    }
    else
    {

        snapshot.WriteGameIndex(m_uiTeamTarget);
        snapshot.WriteField(m_uiTeamTargetTime);

        for (int i(0); i < NUM_TEAM_STATS; ++i)
            snapshot.WriteField(m_aStatTotals[i]);
    }
}


/*====================
  CTeamInfo::ReadSnapshot
  ====================*/
bool    CTeamInfo::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
    try
    {
        if (!IGameEntity::ReadSnapshot(snapshot, uiVersion))
            EX_ERROR(_T("IGameEntity::ReadSnapshot failed"));

        snapshot.ReadInteger(m_uiTeamID);
        uint uiOldRosterSequence(m_uiRosterChangeSequence);
        snapshot.ReadField(m_uiRosterChangeSequence);
        if (uiOldRosterSequence != m_uiRosterChangeSequence)
            m_bRosterChanged = true;
        snapshot.ReadField(m_uiTeamSize);
        snapshot.ReadBytePercent(m_fWinChance);
        snapshot.ReadGameIndex(m_uiBaseBuildingIndex);
        snapshot.ReadField(m_uiStartingTowerCount);
        snapshot.ReadField(m_uiCurrentTowerCount);
        snapshot.ReadField(m_uiCurrentRangedCount);
        snapshot.ReadField(m_uiCurrentMeleeCount);
        snapshot.ReadField(m_unNameIndex);
        snapshot.ReadBytePercent(m_v4Color[0]);
        snapshot.ReadBytePercent(m_v4Color[1]);
        snapshot.ReadBytePercent(m_v4Color[2]);
        snapshot.ReadGameIndex(m_uiStatsIndex);
        snapshot.ReadField(m_yFlags);
        snapshot.ReadField(m_ySlotLocks);
        snapshot.ReadField(m_fBaseHealthPercent);
        snapshot.ReadField(m_yRemainingPauses);
        snapshot.ReadField(m_uiBanCount);
        snapshot.ReadField(m_uiShopInfoIndex);

        if (uiVersion >= 2)
        {
            snapshot.ReadGameIndex(m_uiTeamTarget);
            snapshot.ReadField(m_uiTeamTargetTime);
        }

        for (int i(0); i < NUM_TEAM_STATS; ++i)
            snapshot.ReadField(m_aStatTotals[i]);

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CTeamInfo::ReadSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  CTeamInfo::Initialize
  ====================*/
void    CTeamInfo::Initialize()
{
    m_unNameIndex = NetworkResourceManager.ReserveString();
    m_uiLastIncomeTime = INVALID_TIME;
    
    m_uiTeamTarget = INVALID_INDEX;
    m_uiTeamTargetTime = INVALID_TIME;

    for (int i(0); i < NUM_TEAM_STATS; ++i)
        m_aStatTotals[i] = 0;
}

/*====================
  CTeamInfo::SetTeamTarget
  ====================*/
void    CTeamInfo::SetTeamTarget(uint uiIndex)
{
    if (uiIndex == -1)
        return;

    m_uiTeamTarget = uiIndex;

    m_uiTeamTargetTime = Host.GetTime() + g_teamTargetTime;
}


/*====================
  CTeamInfo::MatchRemake
  ====================*/
void    CTeamInfo::MatchRemake()
{
    m_bRosterChanged = true;
    m_uiRosterChangeSequence = 0;

    for (uint ui(0); ui < m_vClients.size(); ++ui)
        m_vClients[ui] = -1;

    m_ySlotLocks = 0;

    m_fWinChance = 0.0f;

    m_uiBaseBuildingIndex = INVALID_INDEX;
    m_setBuildingUIDs.clear();
    m_uiStartingTowerCount = 0;
    m_uiCurrentTowerCount = 0;
    m_uiCurrentRangedCount = 0;
    m_uiCurrentMeleeCount = 0;

    m_mapDamagedBuildings.clear();
    m_uiLastAttackNotifyTime = INVALID_TIME;
    m_bDamagedBuildingSetCleared = false;

    m_setAlliedTeams.clear();

    m_yKillStreak = 0;
    m_bSentMegaCreepMessage = false;
    m_bAllKilled = false;

    m_uiStatsIndex = INVALID_INDEX;
    m_yFlags = 0;

    m_yRemainingPauses = g_teamAllowedPauses;
    m_uiBanCount = 0;

    m_uiLastBuildingAttackAnnouncement = INVALID_TIME;

    m_uiShopInfoIndex = INVALID_INDEX;

    m_uiLastIncomeTime = INVALID_TIME;

    m_uiTeamTarget = INVALID_INDEX;
    m_uiTeamTargetTime = INVALID_TIME;
}


/*====================
  CTeamInfo::GameStart
  ====================*/
void    CTeamInfo::GameStart()
{
    m_uiStartingTowerCount = m_uiCurrentTowerCount = GetBuildingCount(_T("Tower"));
    m_uiCurrentRangedCount = GetBuildingCount(_T("RangedRax"));
    m_uiCurrentMeleeCount = GetBuildingCount(_T("MeleeRax"));
    m_bSentMegaCreepMessage = false;
    m_uiLastIncomeTime = INVALID_TIME;
    
    m_uiTeamTarget = INVALID_INDEX;
    m_uiTeamTargetTime = INVALID_TIME;

    if (GetNumActiveClients() == 0)
        return;

    ushort unGoldPerPlayer(Game.GetStartingGold() * GetTeamSize() / GetNumActiveClients());
    for (ivector_it it(m_vClients.begin()), itEnd(m_vClients.end()); it != itEnd; ++it)
    {
        CPlayer *pClient(Game.GetPlayer(*it));
        if (pClient == NULL)
            continue;

        pClient->GiveGold(unGoldPerPlayer, NULL);
        pClient->AdjustStat(PLAYER_STAT_STARTING_GOLD, unGoldPerPlayer);
    }

    m_yRemainingPauses = g_teamAllowedPauses;
}


/*====================
  CTeamInfo::Abandoned
  ====================*/
void    CTeamInfo::Abandoned()
{
    if (!HasFlags(TEAM_FLAG_ABANDONED))
    {
        CBufferFixed<1> buffer;
        buffer << GAME_CMD_ABANDONED_MESSAGE;
    
        for (ivector_it it(m_vClients.begin()), itEnd(m_vClients.end()); it != itEnd; ++it)
            Game.SendGameData(*it, buffer, true);
    }

    SetFlags(TEAM_FLAG_ABANDONED);
}


/*====================
  CTeamInfo::UpdateVoiceTargets
  ====================*/
void    CTeamInfo::UpdateVoiceTargets(int iClientNumber)
{
    if (IsActiveTeam() && Game.IsServer())
    {
        VoiceServer.AddVoiceTarget(iClientNumber, m_vClients, VOICE_TARGET_SET_TEAM);
        VoiceServer.AddVoiceTargets(m_vClients, iClientNumber, VOICE_TARGET_SET_TEAM);
    }
}


/*====================
  CTeamInfo::AddClient
  ====================*/
void    CTeamInfo::AddClient(int iClientNumber, uint uiSlot)
{
    try
    {
        if (uiSlot != INVALID_TEAM_SLOT && uiSlot >= m_uiTeamSize)
            return;

        // Check if they are already on the team
        bool bTeamChange(true);
        for (uint ui(0); ui < m_vClients.size(); ++ui)
        {
            if (m_vClients[ui] == iClientNumber)
            {
                if (ui == uiSlot)
                    return;
                bTeamChange = false;
            }
        }

        // Check if the requested slot is free
        if (uiSlot != INVALID_TEAM_SLOT && m_vClients[uiSlot] != -1)
            return;

        // Find a free slot
        if (uiSlot == INVALID_TEAM_SLOT)
        {
            for (uint ui(0); ui < m_vClients.size(); ++ui)
            {
                if (m_vClients[ui] == -1 && !IsSlotLocked(ui))
                {
                    uiSlot = ui;
                    break;
                }
            }
        }

        if (uiSlot == INVALID_TEAM_SLOT)
            return;

        CPlayer *pClient(Game.GetPlayer(iClientNumber));
        if (pClient == NULL)
            EX_WARN(_T("Could not retrieve an entity for client: ") + XtoA(iClientNumber));

        // Reset team related data for this player
        if (bTeamChange)
        {
            pClient->ClearAffiliations();
            pClient->SetTeam(m_uiTeamID);
        }

        if (IsActiveTeam() && Game.IsServer())
        {
            VoiceServer.AddVoiceTarget(iClientNumber, m_vClients, VOICE_TARGET_SET_TEAM);
            VoiceServer.AddVoiceTargets(m_vClients, iClientNumber, VOICE_TARGET_SET_TEAM);
        }

        pClient->SetTeamIndex(uiSlot);
        m_vClients[uiSlot] = iClientNumber;

        ++m_uiRosterChangeSequence;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CTeamInfo::AddClient() - "), NO_THROW);
    }
}


/*====================
  CTeamInfo::AssignStats
  ====================*/
void    CTeamInfo::AssignStats(CGameStats *pStats)
{
    if (pStats == NULL)
    {
        m_uiStatsIndex = INVALID_INDEX;
        return;
    }

    m_uiStatsIndex = pStats->GetIndex();
}


/*====================
  CTeamInfo::RemoveClient
  ====================*/
void    CTeamInfo::RemoveClient(int iClientID)
{
    for (uint ui(0); ui < m_vClients.size(); ++ui)
    {
        if (m_vClients[ui] == iClientID)
        {
            UnlockSlot(ui);
            m_vClients[ui] = -1;
            ++m_uiRosterChangeSequence;
        }
    }

    if (IsActiveTeam() && Game.IsServer())
    {
        VoiceServer.RemoveVoiceTarget(iClientID, m_vClients, VOICE_TARGET_SET_TEAM);
        VoiceServer.RemoveAllVoiceTargets(iClientID, VOICE_TARGET_SET_TEAM);
    }
}


/*====================
  CTeamInfo::GetNumClients
  ====================*/
uint    CTeamInfo::GetNumClients() const
{
    uint uiCount(0);
    for (ivector_cit it(m_vClients.begin()); it != m_vClients.end(); ++it)
    {
        if (Game.GetPlayer(*it) != NULL)
            ++uiCount;
    }
    
    return uiCount;
}


/*====================
  CTeamInfo::GetClientName
  ====================*/
tstring CTeamInfo::GetClientName(uint uiTeamIndex)
{
    if (uiTeamIndex < 0 || uiTeamIndex >= m_vClients.size())
        return _T("");

    CPlayer *pClient(Game.GetPlayer(m_vClients[uiTeamIndex]));
    if (pClient == NULL)
        return _T("");

    return pClient->GetName();
}


/*====================
  CTeamInfo::GetClientIDFromTeamIndex
  ====================*/
int     CTeamInfo::GetClientIDFromTeamIndex(uint uiTeamIndex) const
{
    if (uiTeamIndex < 0 || uiTeamIndex >= m_vClients.size())
        return -1;
    
    return m_vClients[uiTeamIndex];
}


/*====================
  CTeamInfo::GetTeamIndexFromClientID
  ====================*/
uint    CTeamInfo::GetTeamIndexFromClientID(int iClientID) const
{
    for (uint ui(0); ui < m_vClients.size(); ++ui)
    {
        if (m_vClients[ui] == iClientID)
            return ui;
    }

    return -1;
}


/*====================
  CTeamInfo::GetCaptain
  ====================*/
CPlayer*    CTeamInfo::GetCaptain() const
{
    for (uint ui(0); ui < m_vClients.size(); ++ui)
    {
        CPlayer *pPlayer(Game.GetPlayer(m_vClients[ui]));
        if (pPlayer == NULL)
            continue;
        
        if (pPlayer->HasFlags(PLAYER_FLAG_IS_CAPTAIN))
            return pPlayer;
    }

    return NULL;
}


/*====================
  CTeamInfo::SetTeamSize
  ====================*/
void    CTeamInfo::SetTeamSize(uint uiSize)
{
    m_uiTeamSize = uiSize;
    while (m_vClients.size() < uiSize)
        m_vClients.push_back(-1);
}


/*====================
  CTeamInfo::HasBuilding
  ====================*/
bool    CTeamInfo::HasBuilding(const tstring &sName) const
{
    bool bHasBuilding(true);

    tsvector vList(TokenizeString(sName, ' '));

    for (tsvector_it it(vList.begin()); it != vList.end(); it++)
    {
        ushort unType(EntityRegistry.LookupID(*it));

        bHasBuilding = false;

        for (uiset_cit cit(m_setBuildingUIDs.begin()); cit != m_setBuildingUIDs.end(); ++cit)
        {
            IUnitEntity *pEntity(Game.GetUnitFromUniqueID(*cit));
            if (pEntity == NULL)
                continue;

            if (pEntity->GetType() == unType && pEntity->GetStatus() == ENTITY_STATUS_ACTIVE && pEntity->GetTeam() == GetTeamID())
                bHasBuilding = true;
        }

        if (!bHasBuilding)
            break;
    }

    return bHasBuilding;
}


/*====================
  CTeamInfo::GetBuildingCount
  ====================*/
uint    CTeamInfo::GetBuildingCount(const tstring &sUnitType) const
{
    uint uiCount(0);

    for (uiset_cit cit(m_setBuildingUIDs.begin()); cit != m_setBuildingUIDs.end(); ++cit)
    {
        IUnitEntity *pEntity(Game.GetUnitFromUniqueID(*cit));
        if (pEntity == NULL)
            continue;
        if (pEntity->GetTeam() != GetTeamID())
            continue;

        const tsvector &vUnitType(pEntity->GetUnitType());
        for (tsvector_cit itType(vUnitType.begin()); itType != vUnitType.end(); ++itType)
        {
            if (sUnitType == *itType)
                ++uiCount;
        }
    }

    return uiCount;
}


/*====================
  CTeamInfo::ServerFrameSetup
  ====================*/
bool    CTeamInfo::ServerFrameSetup()
{
    m_uiCurrentTowerCount = GetBuildingCount(_T("Tower"));
    m_uiCurrentRangedCount = GetBuildingCount(_T("RangedRax"));
    m_uiCurrentMeleeCount = GetBuildingCount(_T("MeleeRax"));
    
    if (AreAllAlive())
        m_bAllKilled = false;

    return IGameEntity::ServerFrameSetup();
}


/*====================
  CTeamInfo::PassiveIncome
  ====================*/
void    CTeamInfo::PassiveIncome()
{
    // Only real teams get passive income
    if (m_uiTeamID == TEAM_SPECTATOR || m_uiTeamID >= TEAM_INVALID)
        return;

    // Only give passive income during active phase
    if (Game.GetGamePhase() != GAME_PHASE_ACTIVE)
        return;
    
    // Initialize on the first call
    if (m_uiLastIncomeTime == INVALID_TIME)
    {
        m_uiLastIncomeTime = Game.GetGameTime();
        return;
    }

    // Retrieve game info
    CGameInfo *pGameInfo(Game.GetGameInfo());
    if (pGameInfo == NULL)
        return;

    // Count number of players on the team (all and currently active)
    uint uiTeamSize(0);
    uint uiNonTerminated(0);
    for (ivector_it itClient(m_vClients.begin()), itEnd(m_vClients.end()); itClient != itEnd; ++itClient)
    {
        CPlayer *pPlayer(Game.GetPlayer(*itClient));
        if (pPlayer == NULL)
            continue;

        ++uiTeamSize;

         if (!pPlayer->HasFlags(PLAYER_FLAG_TERMINATED))
             ++uiNonTerminated;
    }

    // Get income interval, adjusted by number of missing players
    uint uiInterval(pGameInfo->GetIncomeInterval());
    uiInterval *= uiNonTerminated / float(uiTeamSize);

    if (uiInterval == 0)
    {
        m_uiLastIncomeTime = Game.GetGameTime();
        return;
    }

    while (Game.GetGameTime() - m_uiLastIncomeTime >= uiInterval)
    {
        for (ivector_it itClient(m_vClients.begin()), itEnd(m_vClients.end()); itClient != itEnd; ++itClient)
        {
            CPlayer *pPlayer(Game.GetPlayer(*itClient));
            if (pPlayer == NULL || pPlayer->HasFlags(PLAYER_FLAG_TERMINATED))
                continue;

            pPlayer->GiveGold(pGameInfo->GetGoldPerTick(), NULL);
            pPlayer->GetGoldReport()->AddPassiveGoldEarned(pGameInfo->GetGoldPerTick());
        }

        m_uiLastIncomeTime += uiInterval;
    }
}


/*====================
  CTeamInfo::ServerFrameMovement
  ====================*/
bool    CTeamInfo::ServerFrameMovement()
{
    PROFILE("CTeamInfo::ServerFrameMovement");

    if (m_uiTeamID == 0)
        return true;

    // Clear the building damage half way to the next notification interval
    if (!m_bDamagedBuildingSetCleared &&
        Game.GetGameTime() - m_uiLastAttackNotifyTime > (g_buildingDamageNotifyInterval / 2))
    {
        m_mapDamagedBuildings.clear();
        m_bDamagedBuildingSetCleared = true;
    }

    float fDamage(0.0f);
    for (map<uint, float>::iterator it(m_mapDamagedBuildings.begin()); it != m_mapDamagedBuildings.end(); ++it)
        fDamage += it->second;

    // Building attack notifications
    if (Game.GetGameTime() - m_uiLastAttackNotifyTime > g_buildingDamageNotifyInterval &&
        fDamage >= g_buildingDamageNotifyMinDamage)
    {
        CBufferDynamic buffer(3 + 4 * INT_SIZE(m_mapDamagedBuildings.size()));
        buffer << GAME_CMD_BUILDING_ATTACK_ALERT << byte(m_mapDamagedBuildings.size() & 0xff);
        for (map<uint, float>::iterator it(m_mapDamagedBuildings.begin()); it != m_mapDamagedBuildings.end(); ++it)
            buffer << it->second;

        for (ivector_cit it(m_vClients.begin()); it != m_vClients.end(); ++it)
            Game.SendGameData(*it, buffer, true);

        m_uiLastAttackNotifyTime = Game.GetGameTime();
        m_bDamagedBuildingSetCleared = false;
    }

    // Passive income
    PassiveIncome();

    if (HasTeamTarget() && IsTeamTargetTimeUp())
    {
        ClearTeamTarget();
    }

    return true;
}


/*====================
  CTeamInfo::ServerFrameCleanup
  ====================*/
bool    CTeamInfo::ServerFrameCleanup()
{
    IBuildingEntity *pBuilding(Game.GetBuildingEntity(m_uiBaseBuildingIndex));
    if (pBuilding != NULL)
        m_fBaseHealthPercent = pBuilding->GetHealthPercent();

    return true;
}


/*====================
  CTeamInfo::IsFullyLoaded
  ====================*/
bool    CTeamInfo::IsFullyLoaded() const
{
    uint uiFullyLoadedPlayers(0);

    for (uint ui(0); ui < m_vClients.size(); ++ui)
    {
        CPlayer *pPlayer(Game.GetPlayerFromClientNumber(m_vClients[ui]));
        if (pPlayer == NULL)
            continue;

        if (pPlayer->HasFlags(PLAYER_FLAG_LOADING | PLAYER_FLAG_DISCONNECTED))
            continue;

        ++uiFullyLoadedPlayers;
    }

    return uiFullyLoadedPlayers == m_uiTeamSize;
}


/*====================
  CTeamInfo::CanJoinTeam
  ====================*/
bool    CTeamInfo::CanJoinTeam(int iClientNumber, uint uiSlot)
{
    if (IsFull())
        return false;

    CPlayer *pPlayer(Game.GetPlayer(iClientNumber));
    if (pPlayer == NULL)
        return false;

    // Check if the requested slot
    if (uiSlot != INVALID_TEAM_SLOT)
    {
        if (uiSlot >= m_vClients.size() || m_vClients[uiSlot] != -1 || IsSlotLocked(uiSlot))
            return false;
    }
    else
    {
        for (uint ui(0); ui < m_vClients.size(); ++ui)
        {
            if (m_vClients[ui] == -1 && (!IsSlotLocked(ui) || pPlayer->HasFlags(PLAYER_FLAG_HOST)))
            {
                uiSlot = ui;
                break;
            }
        }
    }

    if (pPlayer->GetTeam() == GetTeamID() && (uiSlot == INVALID_TEAM_SLOT || pPlayer->GetTeamIndex() == uiSlot))
        return false;

    return true;
}


/*====================
  CTeamInfo::GetNumActiveClients
  ====================*/
int     CTeamInfo::GetNumActiveClients()
{
    int iActiveClients(0);

    for (ivector_it it(m_vClients.begin()), itEnd(m_vClients.end()); it != itEnd; ++it)
    {
        CPlayer *pClient(Game.GetPlayer(*it));
        if (pClient == NULL || pClient->HasFlags(PLAYER_FLAG_TERMINATED))
            continue;

        ++iActiveClients;
    }

    return iActiveClients;
}


/*====================
  CTeamInfo::GetNumConnectedClients
  ====================*/
int     CTeamInfo::GetNumConnectedClients()
{
    int iConnectedClients(0);

    for (ivector_it it(m_vClients.begin()), itEnd(m_vClients.end()); it != itEnd; ++it)
    {
        CPlayer *pClient(Game.GetPlayer(*it));
        if (pClient == NULL || pClient->HasFlags(PLAYER_FLAG_TERMINATED) || pClient->HasFlags(PLAYER_FLAG_DISCONNECTED))
            continue;

        ++iConnectedClients;
    }

    return iConnectedClients;
}


/*====================
  CTeamInfo::SetHeroSpawnPosition
  ====================*/
void    CTeamInfo::SetHeroSpawnPosition(IUnitEntity *pUnit) const
{
    if (pUnit == NULL)
        return;

    // Get a list of all active spawn points
    static vector<uint> vSpawnPoints;
    vSpawnPoints.clear();
    Game.GetEntities(vSpawnPoints, Trigger_SpawnPoint);

    static vector<uint> vActiveSpawnPoints;
    vActiveSpawnPoints.clear();

    for (uivector_it it(vSpawnPoints.begin()), itEnd(vSpawnPoints.end()); it != itEnd; ++it)
    {
        CTriggerSpawnPoint *pSpawnPoint(Game.GetEntityAs<CTriggerSpawnPoint>(*it));

        if (pSpawnPoint == NULL)
            continue;
        if (pSpawnPoint->GetTeam() != GetTeamID())
            continue;
        vActiveSpawnPoints.push_back(*it);
    }

    CBBoxf bbBounds;
    bbBounds.SetCylinder(pUnit->GetBoundsRadius(), pUnit->GetBoundsHeight());

    CVec3f v3Center;
    float fDist;
    CVec3f v3Angles;

    if (vActiveSpawnPoints.empty())
    {
        // Get base building
        IBuildingEntity *pBaseBuilding(Game.GetBuildingEntity(GetBaseBuildingIndex()));
        if (pBaseBuilding == NULL)
            return;

        v3Center = pBaseBuilding->GetPosition();
        fDist = pBaseBuilding->GetBounds().GetMax(Y) + MAX(bbBounds.GetDim(X) * DIAG, 32.0f) + 32.0f;
        v3Angles = pBaseBuilding->GetAngles();
    }
    else
    {
        uint uiRand(M_Randnum(0u, uint(vActiveSpawnPoints.size() - 1)));

        CTriggerSpawnPoint *pSpawnPoint(Game.GetEntityAs<CTriggerSpawnPoint>(vActiveSpawnPoints[uiRand]));

        v3Center = pSpawnPoint->GetPosition();
        fDist = 0.0f;
        v3Angles = pSpawnPoint->GetAngles();
    }

    // Get spawn location
    CVec3f v3Spawn(V_ZERO);

    int iSpawnTries(0);
    float fShift(0.0f);
    const float fStep(M_PI * 8.0f);
    while (iSpawnTries < 100)
    {
        CVec3f v3StartA(v3Center + CVec3f(fShift, fDist, 0.0f));
        CVec3f v3StartB(v3Center + CVec3f(-fShift, fDist, 0.0f));
        CVec3f v3PosA(M_RotatePointAroundLine(v3StartA, v3Center, v3Center + V_UP, v3Angles[YAW]));
        CVec3f v3PosB(M_RotatePointAroundLine(v3StartB, v3Center, v3Center + V_UP, v3Angles[YAW]));

        STraceInfo trace;
        Game.TraceBox(trace, v3PosA + V_UP * 1000.0f, v3PosA + V_UP * -1000.0f, bbBounds, TRACE_UNIT_MOVEMENT & ~SURF_TERRAIN);
        if (!trace.bHit || (trace.uiSurfFlags & SURF_TERRAIN))
        {
            v3Spawn = trace.v3EndPos;
            break;
        }

        Game.TraceBox(trace, v3PosB + V_UP * 1000.0f, v3PosB + V_UP * -1000.0f, bbBounds, TRACE_UNIT_MOVEMENT & ~SURF_TERRAIN);
        if (!trace.bHit || (trace.uiSurfFlags & SURF_TERRAIN))
        {
            v3Spawn = trace.v3EndPos;
            break;
        }

        ++iSpawnTries;
        fShift += fStep;
    }

    pUnit->SetPosition(v3Spawn);
    pUnit->SetAngles(v3Angles);
    pUnit->SetUnitAngles(v3Angles);
    pUnit->SetAttentionAngles(v3Angles);
    pUnit->IncNoInterpolateSequence();
}

/*====================
  CTeamInfo::GetHeroSpawnPosition
  ====================*/
CVec3f  CTeamInfo::GetHeroSpawnPosition() const
{
    // Get a list of all active spawn points
    static vector<uint> vSpawnPoints;
    vSpawnPoints.clear();
    Game.GetEntities(vSpawnPoints, Trigger_SpawnPoint);

    static vector<uint> vActiveSpawnPoints;
    vActiveSpawnPoints.clear();

    for (uivector_it it(vSpawnPoints.begin()), itEnd(vSpawnPoints.end()); it != itEnd; ++it)
    {
        CTriggerSpawnPoint *pSpawnPoint(Game.GetEntityAs<CTriggerSpawnPoint>(*it));

        if (pSpawnPoint == NULL)
            continue;
        if (pSpawnPoint->GetTeam() != GetTeamID())
            continue;
        vActiveSpawnPoints.push_back(*it);
    }

    CVec3f v3Center;

    if (vActiveSpawnPoints.empty())
    {
        // Get base building
        IBuildingEntity *pBaseBuilding(Game.GetBuildingEntity(GetBaseBuildingIndex()));
        if (pBaseBuilding == NULL)
        {
            assert(false);
            Console.Warn << "GetHeroSpawnPosition() failed: Failed to locate the base building!" << newl;
            return V_ZERO;
        }

        return pBaseBuilding->GetPosition();
    }
    else
    {
        uint uiRand(M_Randnum(0u, uint(vActiveSpawnPoints.size() - 1)));

        CTriggerSpawnPoint *pSpawnPoint(Game.GetEntityAs<CTriggerSpawnPoint>(vActiveSpawnPoints[uiRand]));

        return pSpawnPoint->GetPosition();
    }
}

/*====================
  CTeamInfo::SpawnCreep
  ====================*/
IUnitEntity*    CTeamInfo::SpawnCreep(const tstring &sName, IVisualEntity *pSpawnPoint, int iLevel, uint uiUpgradeLevel)
{
    PROFILE("CTeamInfo::SpawnCreep");

    // Lookup entity
    if (sName.empty())
        return NULL;

    if (!pSpawnPoint)
        return NULL;

    // Spawn the unit
    ICreepEntity *pCreep(Game.AllocateDynamicEntity<ICreepEntity>(sName));
    if (pCreep == NULL)
    {
        Console.Warn << _T("Failed to spawn creep: ") << sName << newl;
        return NULL;
    }

    CVec3f v3Angles(pSpawnPoint->GetAngles());

    pCreep->SetTeam(GetTeamID());
    pCreep->SetPosition(pSpawnPoint->GetPosition());
    pCreep->SetAngles(v3Angles);
    pCreep->SetUnitAngles(v3Angles);
    pCreep->SetAttentionAngles(v3Angles);
    pCreep->SetLevel(iLevel);
    pCreep->AddCharges(uiUpgradeLevel);
    pCreep->Spawn();
    pCreep->ValidatePosition(TRACE_UNIT_SPAWN);
    pCreep->SetController(pSpawnPoint->GetUniqueID());

    return pCreep;
}


/*====================
  CTeamInfo::SendMegaCreepMessage
  ====================*/
void    CTeamInfo::SendMegaCreepMessage(uint uiAttackingTeam)
{
    if (m_bSentMegaCreepMessage)
        return;

    m_bSentMegaCreepMessage = true;
    CBufferFixed<9> buffer;
    buffer << GAME_CMD_MEGACREEP_MESSAGE << uiAttackingTeam << GetTeamID();
    Game.BroadcastGameData(buffer, true);
}


/*====================
  CTeamInfo::RewardKill
  ====================*/
void    CTeamInfo::RewardKill()
{
    ++m_yKillStreak;

    if (m_yKillStreak == 5)
    {
        CBufferFixed<5> buffer;
        buffer << GAME_CMD_TEAM_KILLSTREAK_MESSAGE << m_uiTeamID;
        Game.BroadcastGameData(buffer, true, -1, 4000);
    }
}


/*====================
  CTeamInfo::AreAllDead
  ====================*/
bool    CTeamInfo::AreAllDead() const
{
    for (ivector_cit itPlayer(m_vClients.begin()); itPlayer != m_vClients.end(); ++itPlayer)
    {
        CPlayer *pPlayer(Game.GetPlayer(*itPlayer));
        if (pPlayer == NULL)
            continue;

        IHeroEntity *pHero(pPlayer->GetHero());
        if (pHero == NULL || pHero->GetStatus() != ENTITY_STATUS_ACTIVE)
            continue;

        return false;
    }

    return true;
}


/*====================
  CTeamInfo::AreAllAlive
  ====================*/
bool    CTeamInfo::AreAllAlive() const
{
    for (ivector_cit itPlayer(m_vClients.begin()); itPlayer != m_vClients.end(); ++itPlayer)
    {
        CPlayer *pPlayer(Game.GetPlayer(*itPlayer));
        if (pPlayer == NULL)
            continue;

        IHeroEntity *pHero(pPlayer->GetHero());
        if (pHero == NULL || pHero->GetStatus() == ENTITY_STATUS_ACTIVE)
            continue;

        return false;
    }

    return true;
}


/*====================
  CTeamInfo::GiveGold
  ====================*/
void    CTeamInfo::GiveGold(ushort unGold)
{
    for (ivector_it itClient(m_vClients.begin()); itClient != m_vClients.end(); ++itClient)
    {
        CPlayer *pPlayer(Game.GetPlayer(*itClient));
        if (pPlayer == NULL || pPlayer->HasFlags(PLAYER_FLAG_TERMINATED))
            continue;

        pPlayer->GiveGold(unGold, pPlayer->GetHero());
    }
}


/*====================
  CTeamInfo::DistributeGold
  ====================*/
void    CTeamInfo::DistributeGold(ushort unGold)
{
    GiveGold(INT_FLOOR(unGold / float(GetNumActiveClients())));
}


/*====================
  CTeamInfo::GetRank
  ====================*/
float   CTeamInfo::GetRank() const
{
    float fRank(0.0f);
    for (ivector_cit itClient(m_vClients.begin()); itClient != m_vClients.end(); ++itClient)
    {
        CPlayer *pPlayer(Game.GetPlayer(*itClient));
        if (pPlayer == NULL)
            continue;

        fRank += pow(MAX(pPlayer->GetRank(), 0.01f), psf_teamRankWeighting);
    }

    if (fRank == 0.0f)
        return 0.0f;
    else
        return pow(fRank, 1.0f / psf_teamRankWeighting);
}


/*====================
  CTeamInfo::GetAverageRank
  ====================*/
float   CTeamInfo::GetAverageRank() const
{
    float fRank(0.0f);
    int iCount(0);
    for (ivector_cit itClient(m_vClients.begin()); itClient != m_vClients.end(); ++itClient)
    {
        CPlayer *pPlayer(Game.GetPlayer(*itClient));
        if (pPlayer == NULL)
            continue;

        fRank += pPlayer->GetRank();
        ++iCount;
    }

    return (iCount == 0) ? 0.0f : fRank / iCount;
}


/*====================
  CTeamInfo::CanSee
  ====================*/
bool    CTeamInfo::CanSee(const IVisualEntity *pTarget) const
{
    if (pTarget == NULL)
        return false;

    const IUnitEntity *pUnit(pTarget->GetAsUnit());

    if (pTarget->GetTeam() == GetTeamID())
        return true;
    if (pUnit != NULL && pUnit->GetAlwaysTransmitData())
        return true;
    if (pUnit != NULL && pUnit->GetTeam() != GetTeamID() && pUnit->GetHidden())
        return false;
    if (pUnit != NULL && pUnit->GetAlwaysVisible())
        return true;
    if (pUnit != NULL && pUnit->IsStealth() && !pTarget->HasVisibilityFlags(VIS_REVEALED(GetTeamID())))
        return false;
    if (pTarget->HasVisibilityFlags(VIS_SIGHTED(GetTeamID())))
        return true;

    return false;
}


/*====================
  CTeamInfo::RegisterShopInfo
  ====================*/
void    CTeamInfo::RegisterShopInfo(uint uiIndex)
{
    m_uiShopInfoIndex = uiIndex;
}


/*====================
  CTeamInfo::GetShopInfo
  ====================*/
CShopInfo*  CTeamInfo::GetShopInfo()
{
    if (m_uiShopInfoIndex == INVALID_INDEX)
        return NULL;

    return Game.GetEntityAs<CShopInfo>(m_uiShopInfoIndex);
}


/*====================
  CTeamInfo::GetTeamStat
  ====================*/
uint    CTeamInfo::GetTeamStat(EPlayerStat eStat) const
{
    uint uiTotal(0);

    for (ivector_cit it(m_vClients.begin()), itEnd(m_vClients.end()); it != itEnd; ++it)
    {
        CPlayer *pPlayer(Game.GetPlayer(*it));
        if (pPlayer == NULL)
            continue;

        uiTotal += pPlayer->GetStat(eStat);
    }

    return uiTotal;
}


/*====================
  CTeamInfo::GetExperienceEarned
  ====================*/
float   CTeamInfo::GetExperienceEarned() const
{
    float fTotal(0.0f);

    for (ivector_cit it(m_vClients.begin()), itEnd(m_vClients.end()); it != itEnd; ++it)
    {
        CPlayer *pPlayer(Game.GetPlayer(*it));
        if (pPlayer == NULL)
            continue;

        IHeroEntity *pHero(pPlayer->GetHero());
        if (pHero == NULL)
            continue;

        fTotal += pHero->GetExperience();
    }

    return fTotal;
}


/*====================
  CTeamInfo::GetGoldEarned
  ====================*/
uint    CTeamInfo::GetGoldEarned() const
{
    uint uiTotal(0);

    for (ivector_cit it(m_vClients.begin()), itEnd(m_vClients.end()); it != itEnd; ++it)
    {
        CPlayer *pPlayer(Game.GetPlayer(*it));
        if (pPlayer == NULL)
            continue;

        uiTotal += pPlayer->GetGoldEarned();
    }

    return uiTotal;
}


/*====================
  CTeamInfo::GetHeroAlivePercent
  ====================*/
float   CTeamInfo::GetHeroAlivePercent()
{
    int iTotalHeroes(0);
    int iAliveHeroes(0);

    for (ivector_cit it(m_vClients.begin()), itEnd(m_vClients.end()); it != itEnd; ++it)
    {
        CPlayer *pPlayer(Game.GetPlayer(*it));
        if (pPlayer == NULL)
            continue;

        IHeroEntity *pHero(pPlayer->GetHero());
        if (pHero == NULL)
            continue;

        ++iTotalHeroes;

        if (pHero->GetStatus() == ENTITY_STATUS_ACTIVE)
            ++iAliveHeroes;
    }

    if (iTotalHeroes == 0)
        return 1.0f;
    else
        return float(iAliveHeroes) / float(iTotalHeroes);
}

