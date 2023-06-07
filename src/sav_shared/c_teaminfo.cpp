// (C)2006 S2 Games
// c_teaminfo.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_teaminfo.h"
#include "i_propfoundation.h"
#include "c_teamdefinition.h"
#include "c_entityclientinfo.h"

#include "../k2/c_snapshot.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
extern CCvari g_maxPlayerLevel;
extern CCvarf g_expLevelExponent;
extern CCvarf g_expLevelMultiplier;
extern CCvari g_statPointsPerLevel;
extern CCvarf g_statsCostTotalMult;
extern CCvarf g_statsCostExponent;
extern CCvarf g_statsCostLevelMult;
EXTERN_CVAR_FLOAT(p_maxWalkSlope)
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
vector<SDataField>* CEntityTeamInfo::s_pvFields;

CVAR_UINTF(     g_economyInterval,      5000,           CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_UINTF(     sv_teamStartingGold,    500,            CVAR_GAMECONFIG);
CVAR_UINTF(     sv_squadSize,           5,              CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_UINTF(     g_maxCandidates,        5,              CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_FLOATF(    g_teamManaRegen,        0.5f,           CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_FLOATF(    g_teamMaxMana,          200.0f,         CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_BOOLF(     sv_allowNoCommander,    false,          CVAR_SAVECONFIG);
CVAR_UINTF(     g_economyDeadTime,      MinToMs(5u),    CVAR_GAMECONFIG);

CVAR_UINTF(     g_buildingDamageNotifyInterval,     20000,          CVAR_GAMECONFIG);
CVAR_UINTF(     g_buildingDamageNotifyMinDamage,    200,            CVAR_GAMECONFIG);

CVAR_UINTF(     g_workerCost,                       100,            CVAR_GAMECONFIG);
CVAR_UINTF(     g_workerCoolDownTime,               60000,          CVAR_GAMECONFIG);
CVAR_UINTF(     g_workerMaxActive,                  3,              CVAR_GAMECONFIG);

const tstring g_asSquadNames[] =
{
    _T("Aleph"),
    _T("Beth"),
    _T("Gimel"),
    _T("Daleth"),
    _T("Zayin"),

    _T("Unassigned")
};

const tstring g_asSquadColors[] =
{
    _T("aqua"),
    _T("blue"),
    _T("yellow"),
    _T("green"),
    _T("orange"),

    _T("white")
};

DEFINE_ENT_ALLOCATOR(Entity, TeamInfo)
//=============================================================================

/*====================
  CEntityTeamInfo::CEntityTeamInfo
  ====================*/
CEntityTeamInfo::~CEntityTeamInfo()
{
    if (m_iTeamID != -1)
        Game.SetTeam(m_iTeamID, NULL);
}


/*====================
  CEntityTeamInfo::CEntityTeamInfo
  ====================*/
CEntityTeamInfo::CEntityTeamInfo() :
IGameEntity(NULL),

m_bRosterChanged(false),
m_pDefinition(NULL),

m_iTeamID(-1),
m_iCommanderClientID(-1),
m_iOfficialCommanderClientID(-1),
m_iLastCommanderClientID(-1),

m_fExperience(0.0f),
m_iLevel(0),
m_iPointsSpent(0),

m_uiGold(0),
m_uiNextEconomyInterval(0),

m_fMana(g_teamMaxMana),

m_uiBaseBuildingIndex(INVALID_INDEX),
m_uiLastAttackNotifyTime(0),
m_bDamagedBuildingSetCleared(false),

m_iHellShrineCount(0),
m_bPlayedMalphasSound(false),

m_uiLastWorkerSpawnTime(INVALID_TIME),

m_unWorkerTypeID(INVALID_ENT_TYPE)
{
    for (uint ui(0); ui < MAX_OFFICERS; ++ui)
        m_aiOfficers[ui] = -1;

    for (int i(0); i < NUM_PLAYER_ATTRIBUTES; ++i)
        m_aiStats[i] = 0;
}


/*====================
  CEntityTeamInfo::GetTypeVector
  ====================*/
const vector<SDataField>&   CEntityTeamInfo::GetTypeVector()
{
    if (!s_pvFields)
    {
        s_pvFields = K2_NEW(global,   vector<SDataField>)();
        s_pvFields->clear();
        const vector<SDataField> &vBase(IGameEntity::GetTypeVector());
        s_pvFields->insert(s_pvFields->begin(), vBase.begin(), vBase.end());

        s_pvFields->push_back(SDataField(_T("m_iTeamID"), FIELD_PUBLIC, TYPE_INT));
        s_pvFields->push_back(SDataField(_T("m_uiBaseBuildingIndex"), FIELD_PUBLIC, TYPE_GAMEINDEX));
        s_pvFields->push_back(SDataField(_T("m_iCommanderClientID"), FIELD_PUBLIC, TYPE_INT));
        s_pvFields->push_back(SDataField(_T("m_iOfficialCommanderClientID"), FIELD_PUBLIC, TYPE_INT));
        s_pvFields->push_back(SDataField(_T("m_iLastCommanderClientID"), FIELD_PUBLIC, TYPE_INT));
        s_pvFields->push_back(SDataField(_T("m_uiGold"), FIELD_PUBLIC, TYPE_INT));
        s_pvFields->push_back(SDataField(_T("m_iLevel"), FIELD_PUBLIC, TYPE_INT));
        s_pvFields->push_back(SDataField(_T("m_fExperience"), FIELD_PUBLIC, TYPE_FLOAT));
        s_pvFields->push_back(SDataField(_T("m_fMana"), FIELD_PUBLIC, TYPE_ROUND16));
        s_pvFields->push_back(SDataField(_T("m_iPointsSpent"), FIELD_PUBLIC, TYPE_INT));
        s_pvFields->push_back(SDataField(_T("m_uiNextEconomyInterval"), FIELD_PUBLIC, TYPE_INT));
        s_pvFields->push_back(SDataField(_T("m_uiLastWorkerSpawnTime"), FIELD_PUBLIC, TYPE_INT));
        s_pvFields->push_back(SDataField(_T("m_sName"), FIELD_PUBLIC, TYPE_STRING));
        s_pvFields->push_back(SDataField(_T("m_pDefinition"), FIELD_PUBLIC, TYPE_STRING));
    }

    return *s_pvFields;
}


/*====================
  CEntityTeamInfo::GetSnapshot
  ====================*/
void    CEntityTeamInfo::GetSnapshot(CEntitySnapshot &snapshot) const
{
    IGameEntity::GetSnapshot(snapshot);

    snapshot.AddField(m_iTeamID);
    snapshot.AddGameIndex(m_uiBaseBuildingIndex);
    snapshot.AddField(m_iCommanderClientID);
    snapshot.AddField(m_iOfficialCommanderClientID);
    snapshot.AddField(m_iLastCommanderClientID);
    snapshot.AddField(m_uiGold);
    snapshot.AddField(m_iLevel);
    snapshot.AddField(m_fExperience);
    snapshot.AddRound16(m_fMana);
    snapshot.AddField(m_iPointsSpent);
    snapshot.AddField(m_uiNextEconomyInterval);
    snapshot.AddField(m_uiLastWorkerSpawnTime);
    snapshot.AddField(m_sName);
    snapshot.AddField(m_pDefinition ? m_pDefinition->GetName() : _T(""));
}


/*====================
  CEntityTeamInfo::ReadSnapshot
  ====================*/
bool    CEntityTeamInfo::ReadSnapshot(CEntitySnapshot &snapshot)
{
    try
    {
        bool bUpkeepWasFailed(HasNetFlags(ENT_NET_FLAG_UPKEEP_FAILED));

        if (!IGameEntity::ReadSnapshot(snapshot))
            EX_ERROR(_T("IGameEntity::ReadSnapshot failed"));

        snapshot.ReadNextField(m_iTeamID);
        snapshot.ReadNextGameIndex(m_uiBaseBuildingIndex);
        snapshot.ReadNextField(m_iCommanderClientID);
        snapshot.ReadNextField(m_iOfficialCommanderClientID);
        snapshot.ReadNextField(m_iLastCommanderClientID);
        snapshot.ReadNextField(m_uiGold);
        snapshot.ReadNextField(m_iLevel);
        snapshot.ReadNextField(m_fExperience);
        snapshot.ReadNextRound16(m_fMana);
        snapshot.ReadNextField(m_iPointsSpent);
        snapshot.ReadNextField(m_uiNextEconomyInterval);
        snapshot.ReadNextField(m_uiLastWorkerSpawnTime);
        snapshot.ReadNextField(m_sName);
        tstring sRace(m_pDefinition ? m_pDefinition->GetName() : _T(""));
        snapshot.ReadNextField(sRace);

        sRace = LowerString(sRace);
        if (sRace == _T("human"))
            m_pDefinition = &g_teamdefHuman;
        else if (sRace == _T("beast"))
            m_pDefinition = &g_teamdefBeast;
        else
            m_pDefinition = &g_teamdefNeutral;

        bool bUpkeepIsFailed(HasNetFlags(ENT_NET_FLAG_UPKEEP_FAILED));
        if (bUpkeepWasFailed != bUpkeepIsFailed)
            Game.DoUpkeepEvent(bUpkeepIsFailed, GetTeamID());

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityTeamInfo::ReadSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  CEntityTeamInfo::Baseline
  ====================*/
void    CEntityTeamInfo::Baseline()
{
    IGameEntity::Baseline();

    m_iTeamID = -1;
    m_uiBaseBuildingIndex = INVALID_INDEX;
    m_iCommanderClientID = -1;
    m_iOfficialCommanderClientID = -1;
    m_iLastCommanderClientID = -1;
    m_uiGold = 0;
    m_iLevel = 0;
    m_fExperience = 0.0f;
    m_fMana = 0.0f;
    m_iPointsSpent = 0;
    m_sName = _T("");
    m_pDefinition = NULL;
    m_iHellShrineCount = 0;
    m_bPlayedMalphasSound = false;
    m_uiNextEconomyInterval = 0;
    m_uiLastWorkerSpawnTime = INVALID_TIME;
}


/*====================
  CEntityTeamInfo::IsValid
  ====================*/
bool    CEntityTeamInfo::IsValid()
{
    if ((m_vSpawnPointIndices.empty() && m_setSpawnBuildingIndices.empty()) || m_pDefinition == NULL)
        return false;

    return true;
}


/*====================
  CEntityTeamInfo::Initialize
  ====================*/
void    CEntityTeamInfo::Initialize()
{
    m_uiGold = sv_teamStartingGold;
    m_uiNextEconomyInterval = Game.GetGameTime() + g_economyInterval;
    m_fMana = g_teamMaxMana;
}


/*====================
  CEntityTeamInfo::AddClient
  ====================*/
void    CEntityTeamInfo::AddClient(int iClientNumber)
{
    try
    {
        // Check if they are already on the team
        for (uint ui(0); ui < m_vClients.size(); ++ui)
        {
            if (m_vClients[ui] == iClientNumber)
                return;
        }

        CEntityClientInfo *pClient(Game.GetClientInfo(iClientNumber));
        if (pClient == NULL)
            EX_WARN(_T("Could not retrieve an entity for client: ") + XtoA(iClientNumber));

        // Reset team related data for this player
        pClient->ClearAffiliations();
        pClient->SetTeam(m_iTeamID);

        m_vClients.push_back(iClientNumber);

        SortClientList();

        if (m_iTeamID != 0 && Game.GetGamePhase() > GAME_PHASE_SELECTING_OFFICERS && GetNumOfficers() < GetMaxOfficers())
            PromoteFromTeam();

        UpdateSpawnQueue(pClient);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityTeamInfo::AddClient() - "), NO_THROW);
    }
}


/*====================
  CEntityTeamInfo::RemoveClient
  ====================*/
void    CEntityTeamInfo::RemoveClient(int iClientID)
{
    uint uiTeamIndex = GetTeamIndexFromClientID(iClientID);

    if (IsCommander(uiTeamIndex))
        RemoveCommander();

    byte ySquad = INVALID_SQUAD;
    CEntityClientInfo *pClient(Game.GetClientInfo(iClientID));

    // Check if the removed client was the officer
    if (IsOfficer(uiTeamIndex) && pClient != NULL && Game.GetGamePhase() >= GAME_PHASE_ACTIVE)
        ySquad = pClient->GetSquad();

    ivector_it it(m_vClients.begin());
    while (it != m_vClients.end())
    {
        if (*it == iClientID)
        {
            it = m_vClients.erase(it);
            continue;
        }

        it++;
    }

    ideque_it itSpawnQueue(m_deqSpawnQueue.begin());
    while (itSpawnQueue != m_deqSpawnQueue.end())
    {
        if (*itSpawnQueue == iClientID)
        {
            itSpawnQueue = m_deqSpawnQueue.erase(itSpawnQueue);
            continue;
        }

        ++itSpawnQueue;
    }

    // If he was, promote based on skill factor
    if (ySquad != INVALID_SQUAD && ySquad < MAX_OFFICERS)
    {
        m_aiOfficers[ySquad] = -1;
        m_asetSquads[ySquad].erase(iClientID);

        if (GetSquadSize(ySquad) > 0)
        {
            PromoteFromSquad(ySquad);

            // If we didn't get an officer, then all of them are
            // declining the promote - promote one of them anyways,
            // since we NEED an officer.
            if (m_aiOfficers[ySquad] == -1)
                PromoteFromSquad(ySquad, true);
        }
    }

    // Remove all votes for this user
    for (ivector::iterator it(m_vClients.begin()); it != m_vClients.end(); it++)
    {
        CEntityClientInfo *pClient(Game.GetClientInfo(*it));

        if (pClient == NULL)
            continue;
        
        if (pClient->GetVote() == iClientID)
            pClient->SetVote(-1);
    }

    SortClientList();
}


/*====================
  CEntityTeamInfo::GetClientName
  ====================*/
tstring CEntityTeamInfo::GetClientName(uint uiTeamIndex)
{
    if (uiTeamIndex < 0 || uiTeamIndex >= m_vClients.size())
        return _T("");

    CEntityClientInfo *pClient(Game.GetClientInfo(m_vClients[uiTeamIndex]));
    if (pClient == NULL)
        return _T("");

    return pClient->GetName();
}


/*====================
  CEntityTeamInfo::GetClientIDFromTeamIndex
  ====================*/
int     CEntityTeamInfo::GetClientIDFromTeamIndex(uint uiTeamIndex) const
{
    if (uiTeamIndex < 0 || uiTeamIndex >= m_vClients.size())
        return -1;
    
    return m_vClients[uiTeamIndex];
}


/*====================
  CEntityTeamInfo::GetTeamIndexFromClientID
  ====================*/
uint    CEntityTeamInfo::GetTeamIndexFromClientID(int iClientID) const
{
    for (uint ui(0); ui < m_vClients.size(); ++ui)
    {
        if (m_vClients[ui] == iClientID)
            return ui;
    }

    return -1;
}


/*====================
  CEntityTeamInfo::GetSquadFromClientID
  ====================*/
byte    CEntityTeamInfo::GetSquadFromClientID(int iClientID) const
{
    for (byte ySquad(0); ySquad < MAX_OFFICERS; ++ySquad)
    {
        for (iset_cit itSquad(m_asetSquads[ySquad].begin()); itSquad != m_asetSquads[ySquad].end(); ++itSquad)
        {
            if (*itSquad == iClientID)
                return ySquad;
        }
    }

    return -1;
}


/*====================
  CEntityTeamInfo::GetEconomyIntervalPercent
  ====================*/
float   CEntityTeamInfo::GetEconomyIntervalPercent() const
{
    return (m_uiNextEconomyInterval - Game.GetGameTime()) / float(g_economyInterval);
}


/*====================
  CEntityTeamInfo::GetMaxMana
  ====================*/
float   CEntityTeamInfo::GetMaxMana() const
{
    return g_teamMaxMana;
}


/*====================
  CEntityTeamInfo::UpdateTeamExperience
  ====================*/
void    CEntityTeamInfo::UpdateTeamExperience()
{
    m_fExperience = 0.0f;
    int iCount(0);
    for (ivector_it it(m_vClients.begin()); it != m_vClients.end(); ++it)
    {
        if (*it == m_iCommanderClientID)
            continue;

        CEntityClientInfo *pClient(Game.GetClientInfo(*it));
        if (pClient == NULL)
            continue;

        ++iCount;
        m_fExperience += pClient->GetExperience();
    }

    if (iCount == 0)
        return;

    //m_fExperience /= iCount;

    int i(1);
    float fAccumulator(0.0f);
    for (; i < g_maxPlayerLevel; ++i)
    {
        fAccumulator += pow(float(i - 1), g_expLevelExponent) * g_expLevelMultiplier;
        if (m_fExperience <= fAccumulator)
            break;
    }

    m_iLevel = i;
}


/*====================
  CEntityTeamInfo::GetPercentNextLevel
  ====================*/
float   CEntityTeamInfo::GetPercentNextLevel() const
{
    float fLastLevel(0.0f);
    float fNextLevel(0.0f);
    int i(1);
    for (; i < g_maxPlayerLevel; ++i)
    {
        fNextLevel += pow(float(i - 1), g_expLevelExponent) * g_expLevelMultiplier;
        if (m_fExperience < fNextLevel)
            break;

        fLastLevel = fNextLevel;
    }

    if (m_fExperience >= fNextLevel)
        return 1.0f;

    float fDiff(fNextLevel - fLastLevel);

    return (m_fExperience - fLastLevel) / fDiff;
}


/*====================
  CEntityTeamInfo::GetAvailablePoints
  ====================*/
int     CEntityTeamInfo::GetAvailablePoints() const
{
    return ((m_iLevel - 1) * g_statPointsPerLevel) - m_iPointsSpent;
}


/*====================
  CEntityTeamInfo::GetAttributeCost
  ====================*/
int CEntityTeamInfo::GetAttributeCost(int iStat)
{
    int iPointsRequired = 1;

    iPointsRequired += (m_aiStats[iStat] * g_statsCostLevelMult);
    iPointsRequired = pow(iPointsRequired, g_statsCostExponent);
    iPointsRequired = (iPointsRequired * g_statsCostTotalMult);

    return iPointsRequired;
}


/*====================
  CEntityTeamInfo::SpendPoint
  ====================*/
void    CEntityTeamInfo::SpendPoint(int iStat)
{
    if (iStat <= ATTRIBUTE_NULL || iStat >= NUM_PLAYER_ATTRIBUTES)
        return;

    int iPointsRequired = GetAttributeCost(iStat);

    if (GetAvailablePoints() < iPointsRequired)
        return;

    ++m_aiStats[iStat];
    m_iPointsSpent += iPointsRequired;
}


/*====================
  CEntityTeamInfo::GetVoteCount
  ====================*/
int     CEntityTeamInfo::GetVoteCount(int iClientID)
{
    int iVotes(0);
    for (uint ui(0); ui < m_vClients.size(); ++ui)
    {
        CEntityClientInfo *pClient(Game.GetClientInfo(m_vClients[ui]));
        if (pClient == NULL)
            continue;

        if (pClient->GetVote() == iClientID)
            ++iVotes;
    }

    return iVotes;
}


/*====================
  CEntityTeamInfo::IsMajoritySelection
  ====================*/
bool    CEntityTeamInfo::IsMajoritySelection(float fMajority)
{
    for (ivector_it it(m_vClients.begin()); it != m_vClients.end(); ++it)
    {
        if (GetVotePercent(*it) >= fMajority)
            return true;
    }

    return false;
}


/*====================
  CEntityTeamInfo::RemoveCommander
  ====================*/
void    CEntityTeamInfo::RemoveCommander()
{
    if (m_iCommanderClientID == -1)
        return;

    CEntityClientInfo *pClient(Game.GetClientInfo(m_iCommanderClientID));
    if (pClient == NULL)
        return;

    Console << pClient->GetName() << _T(" has resigned as commander.") << newl;

    pClient->RemoveFlags(CLIENT_INFO_IS_COMMANDER);
    pClient->SetSquad(INVALID_SQUAD);
    
    Game.ChangeUnit(m_iCommanderClientID, Player_Observer);

    m_iCommanderClientID = -1;

    SortClientList();
}


/*====================
  CEntityTeamInfo::HasCommander
  ====================*/
bool    CEntityTeamInfo::HasCommander()
{
    CEntityClientInfo *pClient(Game.GetClientInfo(m_iCommanderClientID));
    if (pClient == NULL ||
        pClient->IsDisconnected() ||
        pClient->GetTeam() != GetTeamID())
        return false;

    return true;
}


/*====================
  CEntityTeamInfo::SetCommander
  ====================*/
void    CEntityTeamInfo::SetCommander(int iClientID)
{
    CEntityClientInfo *pClient(Game.GetClientInfo(iClientID));
    if (pClient == NULL)
        return;

    RemoveCommander();

    pClient->SetFlags(CLIENT_INFO_IS_COMMANDER);
    pClient->RemoveFlags(CLIENT_INFO_IS_OFFICER);
    pClient->SetSquad(INVALID_SQUAD);

    if (Game.GetGamePhase() == GAME_PHASE_ACTIVE)
    {
        IPlayerEntity *pCommander(Game.ChangeUnit(iClientID, Player_Commander, CHANGE_UNIT_REFUND_GOLD));
        if (pCommander == NULL)
            Console.Err << _T("Could not spawn commander for team: ") << m_iTeamID << newl;
        else
        {
            // Assign any existing workers to this commander
            uivector vWorkers(GetWorkerList());
            for (uivector_it it(vWorkers.begin()); it != vWorkers.end(); ++it)
            {
                IPetEntity *pWorker(Game.GetPetEntity(*it));
                if (pWorker == NULL)
                    continue;
                pWorker->SetOwnerUID(pCommander->GetUniqueID());
            }
        }
    }

    SortClientList();
}


/*====================
  CEntityTeamInfo::GetCommanderClient
  ====================*/
CEntityClientInfo*  CEntityTeamInfo::GetCommanderClient()
{
    return Game.GetClientInfo(GetCommanderClientID());
}


/*====================
  CEntityTeamInfo::GetCommanderPlayerEntity
  ====================*/
CPlayerCommander*   CEntityTeamInfo::GetCommanderPlayerEntity()
{
    CEntityClientInfo *pClient(GetCommanderClient());
    if (pClient == NULL)
        return NULL;

    IPlayerEntity *pPlayer(pClient->GetPlayerEntity());
    if (pPlayer == NULL)
        return NULL;

    return pPlayer->GetAsCommander();
}


/*====================
  CEntityTeamInfo::SelectCommander
  ====================*/
bool    CEntityTeamInfo::SelectCommander()
{
    if(m_vClients.empty())
        return false;

    if (m_iCommanderClientID != -1)
        return true;

    int iMostVotes(0);
    ivector vCandidates;

    for (uint uiTeamIndex(0); uiTeamIndex < m_vClients.size(); ++uiTeamIndex)
    {
        CEntityClientInfo *pClient(Game.GetClientInfo(m_vClients[uiTeamIndex]));
        if (pClient == NULL || !pClient->HasFlags(CLIENT_INFO_WANTS_TO_COMMAND))
            continue;

        int iVotes(GetVoteCount(m_vClients[uiTeamIndex]));

        // New leader
        if (iVotes > iMostVotes)
        {
            vCandidates.clear();
            vCandidates.push_back(GetClientIDFromTeamIndex(uiTeamIndex));
            iMostVotes = iVotes;
            continue;
        }

        // Tie
        if (iVotes == iMostVotes)
            vCandidates.push_back(GetClientIDFromTeamIndex(uiTeamIndex));
    }

    // Choose winner
    int iWinner(-1);
    if (!vCandidates.empty())
        iWinner = vCandidates[M_Randnum(0, int(vCandidates.size() - 1))];
    else
        return false;

    SetCommander(iWinner);
    return true;
}


/*====================
  CEntityTeamInfo::GetNumCandidates
  ====================*/
int     CEntityTeamInfo::GetNumCandidates() const
{
    int iCount(0);
    for (ivector_cit cit(m_vClients.begin()); cit != m_vClients.end(); ++cit)
    {
        CEntityClientInfo *pClient(Game.GetClientInfo(*cit));
        if (pClient == NULL)
            continue;

        if (pClient->HasFlags(CLIENT_INFO_WANTS_TO_COMMAND))
            ++iCount;
    }

    return iCount;
}


/*====================
  CEntityTeamInfo::GetMaxCandidates
  ====================*/
int     CEntityTeamInfo::GetMaxCandidates() const
{
    return g_maxCandidates;
}


/*====================
  CEntityTeamInfo::GetNumOfficers
  ====================*/
uint    CEntityTeamInfo::GetNumOfficers() const
{
    uint uiCount(0);
    for (uint ui(0); ui < MAX_OFFICERS; ++ui)
    {
        if (m_aiOfficers[ui] != -1)
            ++uiCount;
    }
    return uiCount;
}


/*====================
  CEntityTeamInfo::GetMaxOfficers
  ====================*/
uint    CEntityTeamInfo::GetMaxOfficers() const
{
    if (m_vClients.size() < 2)
        return 1;

    int iCount(INT_CEIL((m_vClients.size() - 1) / sv_squadSize.GetFloat()));
    return MIN(MAX_OFFICERS, MAX(iCount, 1));
}


/*====================
  CEntityTeamInfo::GetMaxSquadSize
  ====================*/
uint    CEntityTeamInfo::GetMaxSquadSize() const
{
    if (m_vClients.size() < 2)
        return 1;

    size_t zPlayerPool(m_vClients.size());
    if (GetCommanderClientID() != -1 && zPlayerPool > 0)
        --zPlayerPool;

    uint uiNumOfficers(GetNumOfficers());
    if (uiNumOfficers == 0)
        return uint(zPlayerPool);

    return INT_CEIL(zPlayerPool / float(uiNumOfficers));
}


/*====================
  CEntityTeamInfo::PromotePlayer
  ====================*/
void    CEntityTeamInfo::PromotePlayer(int iClientID)
{
    try
    {
        if (GetNumOfficers() >= GetMaxOfficers())
            return;

        CEntityClientInfo *pClient(Game.GetClientInfo(iClientID));
        if (pClient == NULL)
            EX_WARN(_T("Invalid client: ") + XtoA(iClientID));

        if (pClient->GetTeam() != GetTeamID())
            EX_WARN(_T("Target player is not on this team"));
        if (!pClient->HasFlags(CLIENT_INFO_WANTS_TO_BE_OFFICER))
            EX_WARN(_T("Target player is not an officer candidate"));

        byte y(0);
        for (; y < MAX_OFFICERS; ++y)
        {
            if (m_aiOfficers[y] == iClientID)
                return;

            if (m_aiOfficers[y] == -1)
                break;
        }
        if (y == MAX_OFFICERS)
            EX_WARN(_T("MAX_OFFICERS exceeded"));

        pClient->SetSquad(y);
        pClient->SetFlags(CLIENT_INFO_IS_OFFICER);

        SortClientList();
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityTeamInfo::PromotePlayer() - "), NO_THROW);
    }
}


/*====================
  CEntityTeamInfo::PromoteFromTeam
  ====================*/
void    CEntityTeamInfo::PromoteFromTeam(bool bIgnoreDecline)
{
    try
    {
        if (GetNumOfficers() > GetMaxOfficers())
            return;

        int iHighestID(-1);
        int iHighestSF(-1);

        for (ivector_it it(m_vClients.begin()); it != m_vClients.end(); ++it)
        {
            if (*it == m_iCommanderClientID || IsOfficer(GetTeamIndexFromClientID(*it)))
                continue;

            CEntityClientInfo *pClient(Game.GetClientInfo(*it));
            if (pClient != NULL && (pClient->HasFlags(CLIENT_INFO_WANTS_TO_BE_OFFICER) || bIgnoreDecline))
            {
                if (pClient->GetPersistantStat(PLAYER_PERSISTANT_SKILLFACTOR) > iHighestSF)
                {
                    iHighestSF = pClient->GetPersistantStat(PLAYER_PERSISTANT_SKILLFACTOR);
                    iHighestID = (*it);
                }
            }
        }

        if (iHighestID != -1)
            PromotePlayer(iHighestID);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityTeamInfo::PromotePlayer() - "), NO_THROW);
    }
}


/*====================
  CEntityTeamInfo::PromoteFromSquad
  ====================*/
void    CEntityTeamInfo::PromoteFromSquad(byte ySquad, bool bIgnoreDecline)
{
    try
    {
        iset setSquad;

        if (ySquad >= GetMaxOfficers() || GetOfficerGameIndex(ySquad) != INVALID_INDEX)
            return;

        setSquad = m_asetSquads[ySquad];

        iset::iterator it;
        int iHighestSF(-1);
        int iHighestID(-1);

        for (it = setSquad.begin(); it != setSquad.end(); it++)
        {
            if (*it == m_iCommanderClientID)
                continue;

            CEntityClientInfo *pClient(Game.GetClientInfo(*it));
            if (pClient != NULL && (pClient->HasFlags(CLIENT_INFO_WANTS_TO_BE_OFFICER) || bIgnoreDecline))
            {
                if (pClient->GetPersistantStat(PLAYER_PERSISTANT_SKILLFACTOR) > iHighestSF)
                {
                    iHighestSF = pClient->GetPersistantStat(PLAYER_PERSISTANT_SKILLFACTOR);
                    iHighestID = (*it);
                }
            }
        }

        if (iHighestID != -1)
            PromotePlayer(iHighestID);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityTeamInfo::PromotePlayer() - "), NO_THROW);
    }
}


/*====================
  CEntityTeamInfo::DemotePlayer
  ====================*/
void    CEntityTeamInfo::DemotePlayer(byte ySquad)
{
    try
    {
        if (ySquad > MAX_OFFICERS)
            EX_WARN(_T("Invalid squad"));

        int iClientID(m_aiOfficers[ySquad]);
        CEntityClientInfo *pClient(Game.GetClientInfo(iClientID));
        if (pClient == NULL)
            EX_WARN(_T("Invalid client: #") + XtoA(iClientID));

        if (pClient->GetTeam() != GetTeamID())
            EX_WARN(_T("Target player is not on this team"));

        pClient->RemoveFlags(CLIENT_INFO_IS_OFFICER);
        pClient->SetSquad(INVALID_SQUAD);
        SortClientList();
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityTeamInfo::DemotePlayer() - "), NO_THROW);
    }
}


/*====================
  CEntityTeamInfo::DeclineOfficer
  ====================*/
void    CEntityTeamInfo::DeclineOfficer(int iClientID)
{
    CEntityClientInfo *pClient(Game.GetClientInfo(iClientID));
    if (pClient == NULL)
        return;
    if (pClient->GetTeam() != m_iTeamID)
        return;

    pClient->RemoveFlags(CLIENT_INFO_WANTS_TO_BE_OFFICER);
    pClient->RemoveFlags(CLIENT_INFO_IS_OFFICER);
    pClient->SetSquad(INVALID_SQUAD);
    SortClientList();
}


/*====================
  CEntityTeamInfo::IsOfficer
  ====================*/
bool    CEntityTeamInfo::IsOfficer(uint uiTeamIndex) const
{
    int iClientID(GetClientIDFromTeamIndex(uiTeamIndex));
    CEntityClientInfo *pClient(Game.GetClientInfo(iClientID));
    if (pClient == NULL)
        return false;
    
    return pClient->HasFlags(CLIENT_INFO_IS_OFFICER);
}


/*====================
  CEntityTeamInfo::SelectOfficers
  ====================*/
void    CEntityTeamInfo::SelectOfficers()
{
    if (GetNumOfficers() >= GetMaxOfficers())
        return;

    int iNumEligible(0);
    int iNumTotal(0);

    for (uint ui(0); ui < m_vClients.size(); ++ui)
    {
        CEntityClientInfo *pClient(Game.GetClientInfo(m_vClients[ui]));
        if (pClient == NULL)
            continue;
        if (pClient->HasFlags(CLIENT_INFO_IS_COMMANDER | CLIENT_INFO_IS_OFFICER))
            continue;

        ++iNumTotal;

        if (pClient->HasFlags(CLIENT_INFO_WANTS_TO_BE_OFFICER))
            ++iNumEligible;
    }

    while (GetNumOfficers() < GetMaxOfficers() && iNumEligible > 0)
    {
        PromoteFromTeam();
        --iNumEligible;
        --iNumTotal;
    }

    // If we still have no officers, but we have players that declined
    // officer... start ignoring their decline and promote one of them
    if (GetNumOfficers() == 0 && iNumTotal > 0)
        PromoteFromTeam(true);
}


/*====================
  CEntityTeamInfo::JoinSquad
  ====================*/
void    CEntityTeamInfo::JoinSquad(int iClient, byte ySquad)
{
    // Get the requesting client
    CEntityClientInfo *pClient(Game.GetClientInfo(iClient));
    if (pClient == NULL)
        return;
    if (pClient->GetTeam() != GetTeamID())
        return;

    if (pClient->HasFlags(CLIENT_INFO_IS_COMMANDER))
        return;
    if (pClient->HasFlags(CLIENT_INFO_IS_OFFICER))
        return;

    // Make sure the squad isn't full
    if (m_asetSquads[ySquad].size() >= GetMaxSquadSize())
        return;

    pClient->SetSquad(ySquad);
    SortClientList();
}


/*====================
  CEntityTeamInfo::GetSquadColor
  ====================*/
const tstring&  CEntityTeamInfo::GetSquadColor(uint uiSquad)
{
    return m_pDefinition->GetSquadColor(uiSquad);
}


/*====================
  CEntityTeamInfo::GetSquadName
  ====================*/
const tstring&  CEntityTeamInfo::GetSquadName(uint uiSquad)
{
    return m_pDefinition->GetSquadName(uiSquad);
}


/*====================
  CEntityTeamInfo::AreAllPlayersInSquads
  ====================*/
bool    CEntityTeamInfo::AreAllPlayersInSquads() const
{
    for (uint ui(0); ui < m_vClients.size(); ++ui)
    {
        CEntityClientInfo *pClient(Game.GetClientInfo(m_vClients[ui]));
        if (pClient == NULL)
            continue;
        if (pClient->GetSquad() == INVALID_SQUAD && !pClient->HasFlags(CLIENT_INFO_IS_COMMANDER))
            return false;
    }
    return true;
}


/*====================
  CEntityTeamInfo::FillSquads
  ====================*/
void    CEntityTeamInfo::FillSquads()
{
    for (uint ui(0); ui < m_vClients.size(); ++ui)
    {
        IPlayerEntity *pPlayer(Game.GetPlayerEntityFromClientID(m_vClients[ui]));
        if (pPlayer == NULL)
            continue;
        if (pPlayer->GetSquad() != INVALID_SQUAD)
            continue;
        if (pPlayer->GetClientID() == m_iCommanderClientID)
            continue;

        for (byte y(0); y < GetNumOfficers(); ++y)
        {
            if (m_asetSquads[y].size() < GetMaxSquadSize())
            {
                pPlayer->SetSquad(y);
                break;
            }
        }
    }

    SortClientList();
}


/*====================
  CEntityTeamInfo::AddSquadObject
  ====================*/
void    CEntityTeamInfo::AddSquadObject(byte ySquad, uint uiIndex)
{
    if (ySquad >= MAX_OFFICERS)
        return;

    IGameEntity *pEntity(Game.GetEntity(uiIndex));
    if (!pEntity)
        return;

    uiset_it it(m_asetSquadObjects[ySquad].begin());

    while (it != m_asetSquadObjects[ySquad].end())
    {
        IGameEntity *pTestEntity(Game.GetEntityFromUniqueID(*it));
        if (pTestEntity == NULL || pTestEntity->GetType() != pEntity->GetType())
        {
            ++it;
            continue;
        }

        pTestEntity->Kill();
        STL_ERASE(m_asetSquadObjects[ySquad], it);
    }

    m_asetSquadObjects[ySquad].insert(pEntity->GetUniqueID());
}


/*====================
  CEntityTeamInfo::KillSquadObjects
  ====================*/
void    CEntityTeamInfo::KillSquadObjects(byte ySquad)
{
    if (ySquad >= MAX_OFFICERS)
        return;

    for (uiset_it it(m_asetSquadObjects[ySquad].begin()); it != m_asetSquadObjects[ySquad].end(); ++it)
    {
        IGameEntity *pEnt(Game.GetEntityFromUniqueID(*it));
        if (pEnt == NULL)
            continue;

        pEnt->Kill();
    }

    m_asetSquadObjects[ySquad].clear();
}


/*====================
  CEntityTeamInfo::AddSpawnPointIndex
  ====================*/
void    CEntityTeamInfo::AddSpawnPointIndex(uint uiIndex)
{
    m_vSpawnPointIndices.push_back(uiIndex);
}


/*====================
  CEntityTeamInfo::HasBuilding
  ====================*/
bool    CEntityTeamInfo::HasBuilding(const tstring &sName) const
{
    bool bHasBuilding(true);

    svector vList(TokenizeString(sName, ' '));

    for (svector_it it(vList.begin()); it != vList.end(); it++)
    {
        ushort unType(EntityRegistry.LookupID(*it));

        bHasBuilding = false;

        for (uiset_cit cit(m_setBuildingIndices.begin()); cit != m_setBuildingIndices.end(); ++cit)
        {
            IVisualEntity *pEntity(Game.GetVisualEntity(*cit));
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
  CEntityTeamInfo::GetBuildingCount
  ====================*/
uint    CEntityTeamInfo::GetBuildingCount(const tstring &sName) const
{
    uint uiCount(0);

    svector vList(TokenizeString(sName, ' '));

    for (svector_it it(vList.begin()); it != vList.end(); ++it)
    {
        ushort unType(EntityRegistry.LookupID(*it));
        for (uiset_cit cit(m_setBuildingIndices.begin()); cit != m_setBuildingIndices.end(); ++cit)
        {
            IVisualEntity *pEntity(Game.GetVisualEntity(*cit));
            if (pEntity == NULL)
                continue;

            if (pEntity->GetType() == unType && pEntity->GetTeam() == GetTeamID())
                ++uiCount;
        }
    }

    return uiCount;
}


/*====================
  CEntityTeamInfo::BuildingDamaged
  ====================*/
void    CEntityTeamInfo::BuildingDamaged(uint uiIndex, uint uiAttacker, float fDamage)
{
    m_mapDamagedBuildings[uiIndex] += fDamage;

    for (uiset_it it(m_setBuildingIndices.begin()); it != m_setBuildingIndices.end(); ++it)
    {
        IBuildingEntity *pBuilding(Game.GetBuildingEntity(*it));
        if (pBuilding == NULL)
            continue;

        pBuilding->DamageNotification(uiIndex, uiAttacker, fDamage);
    }
}


/*====================
  CEntityTeamInfo::GetNumSpawnBuildings
  ====================*/
uint    CEntityTeamInfo::GetNumSpawnBuildings(IPlayerEntity *pPlayer) const
{
    uint uiCount(0);
    for (uiset_cit cit(m_setSpawnBuildingIndices.begin()); cit != m_setSpawnBuildingIndices.end(); ++cit)
    {
        IVisualEntity *pEntity(Game.GetVisualEntity(*cit));
        if (pEntity == NULL)
            continue;
        if (!pEntity->CanSpawnFrom(pPlayer))
            continue;

        ++uiCount;
    }

    return uiCount;
}


/*====================
  CEntityTeamInfo::Spawn
  ====================*/
void    CEntityTeamInfo::Spawn()
{
    if (m_pDefinition != NULL)
        m_pDefinition->Update(true);

    GetWorkerType(true);
}


/*====================
  CEntityTeamInfo::ServerFrame
  ====================*/
bool    CEntityTeamInfo::ServerFrame()
{
    PROFILE("CEntityTeamInfo::ServerFrame");

    if (m_pDefinition != NULL)
        m_pDefinition->Update();

    if (m_iTeamID == 0)
        return true;

    UpdateTeamExperience();

    if (GetNumOfficers() == 0 && Game.GetGamePhase() >= GAME_PHASE_ACTIVE)
    {
        float fMostXP(0.0f);
        int iBestCandidate(-1);
        for (ivector_it it(m_vClients.begin()); it != m_vClients.end(); ++it)
        {
            CEntityClientInfo *pClient(Game.GetClientInfo(*it));
            if (pClient == NULL)
                continue;
            if (pClient->HasFlags(CLIENT_INFO_IS_COMMANDER))
                continue;

            if (pClient->GetExperience() > fMostXP || iBestCandidate == -1)
            {
                fMostXP = pClient->GetExperience();
                iBestCandidate = *it;
            }
        }

        if (iBestCandidate != -1)
            PromotePlayer(iBestCandidate);
    }

    // Mana
    m_fMana = MIN(g_teamMaxMana.GetValue(), m_fMana + g_teamManaRegen * MsToSec(Game.GetFrameLength()));

    // Economy
    while (m_uiNextEconomyInterval < Game.GetGameTime())
    {
        RemoveNetFlags(ENT_NET_FLAG_UPKEEP_FAILED);

        // Income
        uint uiIncome(0);
        for (uiset_it it(m_setBuildingIndices.begin()); it != m_setBuildingIndices.end(); ++it)
        {
            IBuildingEntity *pBuilding(Game.GetBuildingEntity(*it));
            if (pBuilding == NULL)
                continue;
            
            pBuilding->RemoveNetFlags(ENT_NET_FLAG_UPKEEP_FAILED);
            
            if (pBuilding->GetStatus() != ENTITY_STATUS_ACTIVE)
                continue;

            uiIncome += pBuilding->HarvestGold();
        }
        GiveGold(uiIncome);

        // Failed upkeep
        if (uiIncome == 0 && Game.GetCurrentGameLength() >= g_economyDeadTime)
        {
            SetNetFlags(ENT_NET_FLAG_UPKEEP_FAILED);

            // Use a copy of the set, because a building may be destroyed, invalidating the set
            uiset setBuildingIndices(m_setBuildingIndices);
            for (uiset_it it(setBuildingIndices.begin()); it != setBuildingIndices.end(); ++it)
            {
                IBuildingEntity *pBuilding(Game.GetBuildingEntity(*it));
                if (pBuilding == NULL)
                    continue;

                pBuilding->UpkeepFailed(0.0f);
            }
        }

        // DEPRECATED: Pay upkeep based on individual building costs and decay building health
        // Determine total upkeep
        /*
        uint uiTotalUpkeep(0);
        for (uiset_it it(m_setBuildingIndices.begin()); it != m_setBuildingIndices.end(); ++it)
        {
            IBuildingEntity *pBuilding(Game.GetBuildingEntity(*it));
            if (pBuilding == NULL || pBuilding->GetStatus() != ENTITY_STATUS_ACTIVE)
                continue;

            pBuilding->Upkeep();
            uiTotalUpkeep += pBuilding->GetActiveUpkeepCost();
        }

        // If total cost is more than the team has, distribute decay
        if (!SpendGold(uiTotalUpkeep))
        {
            float fFraction(m_uiGold / float(uiTotalUpkeep));
            m_uiGold = 0;

            // Use a copy of the set, because a building may be destroyed, invalidating the set
            uiset setBuildingIndices(m_setBuildingIndices);
            for (uiset_it it(setBuildingIndices.begin()); it != setBuildingIndices.end(); ++it)
            {
                IGameEntity *pEntity(Game.GetEntity(*it));
                if (pEntity == NULL)
                    continue;
                IBuildingEntity *pBuilding(pEntity->GetAsBuilding());
                if (pBuilding == NULL)
                    continue;

                pBuilding->UpkeepFailed(fFraction);
            }
        }
        */

        m_uiNextEconomyInterval += g_economyInterval;
    }

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

    // Record "official" comm status based on playtime
    if (m_iCommanderClientID != -1 && m_iOfficialCommanderClientID != m_iCommanderClientID)
    {
        CEntityClientInfo *pOfficial(Game.GetClientInfo(m_iOfficialCommanderClientID));
        CEntityClientInfo *pCurrent(Game.GetClientInfo(m_iCommanderClientID));

        if (pOfficial == NULL && pCurrent != NULL)
            m_iOfficialCommanderClientID = m_iCommanderClientID;
        else if (pOfficial != NULL && pCurrent != NULL && pOfficial->GetCommPlayTime() < pCurrent->GetCommPlayTime())
            m_iOfficialCommanderClientID = m_iCommanderClientID;
    }

    // Try to push players out of the spawn queue
    SpawnClientFromQueue();

    return true;
}


/*====================
  CEntityTeamInfo::UpdateRoster
  ====================*/
void    CEntityTeamInfo::UpdateRoster()
{
    // Clear commander/officer/squad data
    m_iCommanderClientID = -1;
    for (int i(0); i < MAX_OFFICERS; ++i)
    {
        m_aiOfficers[i] = -1;
        m_asetSquads[i].clear();
    }

    // Fill in commander/officer/squad data
    for (ivector_it it(m_vClients.begin()); it != m_vClients.end(); ++it)
    {
        CEntityClientInfo *pClient(Game.GetClientInfo(*it));
        if (pClient == NULL)
            continue;

        // Commander
        if (pClient->HasFlags(CLIENT_INFO_IS_COMMANDER))
        {
            if (m_iCommanderClientID != -1)
                Console.Warn << _T("Multiple commanders on team") << newl;

            m_iCommanderClientID = *it;
            m_iLastCommanderClientID = *it;
            continue;
        }

        byte ySquad(pClient->GetSquad());
        if (ySquad >= MAX_OFFICERS)
            continue;

        // Squads
        m_asetSquads[ySquad].insert(*it);

        // Officers
        if (pClient->HasFlags(CLIENT_INFO_IS_OFFICER))
        {
            if (m_aiOfficers[ySquad] != -1)
                Console.Warn << _T("Multiple officers for squad: ") << ySquad << newl;

            m_aiOfficers[ySquad] = *it;
        }
    }
}


/*====================
  CEntityTeamInfo::SortClientList
  ====================*/
void    CEntityTeamInfo::SortClientList()
{
    UpdateRoster();

    // Grab a copy of the current list before wiping it
    ivector vClients(m_vClients);
    m_vClients.clear();

    // List should be grouped by squad, with respective officers at the start of each squad
    for (byte ySquad(0); ySquad < MAX_OFFICERS; ++ySquad)
    {
        ideque deqSquad;
        for (iset_it itSquad(m_asetSquads[ySquad].begin()); itSquad != m_asetSquads[ySquad].end(); ++itSquad)
        {
            byte yOfficer(0);
            for (; yOfficer < MAX_OFFICERS; ++yOfficer)
            {
                if (m_aiOfficers[yOfficer] == *itSquad)
                {
                    deqSquad.push_front(*itSquad);
                    break;
                }
            }
            if (yOfficer == MAX_OFFICERS)
                deqSquad.push_back(*itSquad);
        }

        for (ideque_it itResults(deqSquad.begin()); itResults != deqSquad.end(); ++itResults)
            m_vClients.push_back(*itResults);
    }

    // Throw in anyone who is not the commander or in a squad
    for (ivector_it it(vClients.begin()); it != vClients.end(); ++it)
    {
        if (*it == m_iCommanderClientID)
            continue;

        byte y(0);
        for (; y < m_vClients.size(); ++y)
        {
            if (m_vClients[y] == *it)
                break;
        }
        if (y != m_vClients.size())
            continue;

        m_vClients.push_back(*it);
    }

    // Commander is last so that the team list can skip it without issue
    if (m_iCommanderClientID != -1)
        m_vClients.push_back(m_iCommanderClientID);

    m_bRosterChanged = true;
}


/*====================
  CEntityTeamInfo::GetOfficerGameIndex
  ====================*/
uint    CEntityTeamInfo::GetOfficerGameIndex(byte ySquad)
{
    if (ySquad >= MAX_OFFICERS)
        return INVALID_INDEX;

    int iClientID(m_aiOfficers[ySquad]);

    IPlayerEntity *pPlayer(Game.GetPlayerEntityFromClientID(iClientID));

    if (pPlayer)
        return pPlayer->GetIndex();
    else
        return INVALID_INDEX;
}


/*====================
  CEntityTeamInfo::GetSquadMemberIndex
  ====================*/
uint    CEntityTeamInfo::GetSquadMemberIndex(byte ySquad, uint uiMember)
{
    if (ySquad >= MAX_OFFICERS)
        return INVALID_INDEX;

    if (uiMember >= m_asetSquads[ySquad].size())
        return INVALID_INDEX;

    iset_it it(m_asetSquads[ySquad].begin());
    for (uint ui(0); ui < uiMember; ++ui)
    {
        ++it;
        if (it == m_asetSquads[ySquad].end())
            return INVALID_INDEX;
    }
    int iClientID(*it);

    IPlayerEntity *pPlayer(Game.GetPlayerEntityFromClientID(iClientID));
    if (pPlayer != NULL)
        return pPlayer->GetIndex();
    else
        return INVALID_INDEX;
}


/*====================
  CEntityTeamInfo::GetActiveUpkeep
  ====================*/
uint    CEntityTeamInfo::GetActiveUpkeep() const
{
    uint uiTotalUpkeep(0);

    // Check each building
    for (uiset_cit cit(m_setBuildingIndices.begin()); cit != m_setBuildingIndices.end(); ++cit)
    {
        IBuildingEntity *pBuilding(Game.GetBuildingEntity(*cit));
        if (pBuilding == NULL || pBuilding->GetStatus() != ENTITY_STATUS_ACTIVE)
            continue;

        uiTotalUpkeep += pBuilding->GetActiveUpkeepCost();
    }

    return uiTotalUpkeep;
}


/*====================
  CEntityTeamInfo::GetTotalUpkeep
  ====================*/
uint    CEntityTeamInfo::GetTotalUpkeep() const
{
    uint uiTotalUpkeep(0);

    // Check each building
    for (uiset_cit cit(m_setBuildingIndices.begin()); cit != m_setBuildingIndices.end(); ++cit)
    {
        IBuildingEntity *pBuilding(Game.GetBuildingEntity(*cit));
        if (pBuilding == NULL || pBuilding->GetStatus() != ENTITY_STATUS_ACTIVE || pBuilding->GetTeam() != GetTeamID())
            continue;

        uiTotalUpkeep += pBuilding->GetUpkeepCost();
    }

    return uiTotalUpkeep;
}


/*====================
  CEntityTeamInfo::GetTotalIncome
  ====================*/
uint    CEntityTeamInfo::GetTotalIncome() const
{
    uint uiTotalIncome(0);

    // Check each building
    for (uiset_cit cit(m_setBuildingIndices.begin()); cit != m_setBuildingIndices.end(); ++cit)
    {
        IBuildingEntity *pBuilding(Game.GetBuildingEntity(*cit));
        if (pBuilding == NULL || pBuilding->GetStatus() != ENTITY_STATUS_ACTIVE || pBuilding->GetTeam() != GetTeamID())
            continue;

        uiTotalIncome += pBuilding->GetIncomeAmount();
    }

    return uiTotalIncome;
}


/*====================
  CEntityTeamInfo::GiveGold
  ====================*/
void    CEntityTeamInfo::GiveGold(uint uiGold)
{
    if (Game.GetGamePhase() == GAME_PHASE_WARMUP)
        return;

    m_uiGold += uiGold;

    Game.MatchStatEvent(m_iCommanderClientID, COMMANDER_MATCH_TEAM_GOLD, int(uiGold));
}


/*====================
  CEntityTeamInfo::GetAverageSF
  ====================*/
uint    CEntityTeamInfo::GetAverageSF()
{
    uint uiSF(0);
    int iNumClients(0);

    for (ivector_it it(m_vClients.begin()); it != m_vClients.end(); ++it)
    {
        CEntityClientInfo *pClient(Game.GetClientInfo(*it));
        if (pClient == NULL)
            continue;

        ++iNumClients;
        uiSF += pClient->GetPersistantStat(PLAYER_PERSISTANT_SKILLFACTOR);
    }

    if (iNumClients > 0)
        uiSF = uiSF / iNumClients;

    return uiSF;
}


/*====================
  CEntityTeamInfo::SpawnClientFromQueue
  ====================*/
void    CEntityTeamInfo::SpawnClientFromQueue()
{
    CEntityTeamInfo *pEnemyTeam(Game.GetTeam(m_iTeamID ^ 3));
    if (pEnemyTeam == NULL)
    {
        // REMOVED WARNING: Spam in servers with > 2 teams...
        // Console.Warn << _T("Could not find enemy team for team id: ") << m_iTeamID << newl;
        return;
    }

    while (!m_deqSpawnQueue.empty() && (Game.GetGamePhase() == GAME_PHASE_WARMUP || GetNumActiveClients() - ICvar::GetInteger(_T("sv_maxTeamDifference")) < pEnemyTeam->GetNumClients()))
    {
        int iClientNum(m_deqSpawnQueue.front());
        m_deqSpawnQueue.pop_front();

        CEntityClientInfo *pClient(Game.GetClientInfo(iClientNum));
        if (pClient == NULL)
            continue;

        pClient->RemoveNetFlags(ENT_NET_FLAG_QUEUED);
        pClient->SetSpawnQueuePosition(0);
        return;
    }
}


/*====================
  CEntityTeamInfo::UpdateSpawnQueue
  ====================*/
void    CEntityTeamInfo::UpdateSpawnQueue(CEntityClientInfo *pClient)
{
    if (pClient == NULL || pClient->GetTeam() != GetTeamID() || pClient->GetTeam() == 0)
        return;

    AddClientToSpawnQueue(pClient);
    SpawnClientFromQueue();

    byte yPos(1);
    ideque_it itClient(m_deqSpawnQueue.begin());
    while (itClient != m_deqSpawnQueue.end())
    {
        CEntityClientInfo *pClient(Game.GetClientInfo(*itClient));
        
        if (pClient != NULL)
            pClient->SetSpawnQueuePosition(yPos++);

        ++itClient;
    }
}


/*====================
  CEntityTeamInfo::AddClientToSpawnQueue
  ====================*/
void    CEntityTeamInfo::AddClientToSpawnQueue(CEntityClientInfo *pClient)
{
    pClient->SetNetFlags(ENT_NET_FLAG_QUEUED);

    ideque_it itClient(m_deqSpawnQueue.begin());
    while (itClient != m_deqSpawnQueue.end())
    {
        if ((*itClient) == pClient->GetClientNumber())
            return;

        ++itClient;
    }

    m_deqSpawnQueue.push_back(pClient->GetClientNumber());
}


/*====================
  CEntityTeamInfo::GetNumActiveClients
  ====================*/
int CEntityTeamInfo::GetNumActiveClients()
{
    int iActiveClients(0);

    for (ivector_it it(m_vClients.begin()); it != m_vClients.end(); ++it)
    {
        CEntityClientInfo *pClient(Game.GetClientInfo(*it));
        if (pClient == NULL || pClient->HasNetFlags(ENT_NET_FLAG_QUEUED))
            continue;

        ++iActiveClients;
    }

    return iActiveClients;
}


/*====================
  CEntityTeamInfo::GetWorkerType
  ====================*/
ushort  CEntityTeamInfo::GetWorkerType(bool bForceLookup)
{
    if (!bForceLookup)
        return m_unWorkerTypeID;

    // Lookup worker entity
    tstring sWorkerName(m_pDefinition->GetWorkerName());
    if (sWorkerName.empty())
        m_unWorkerTypeID = INVALID_ENT_TYPE;

    m_unWorkerTypeID = EntityRegistry.LookupID(sWorkerName);
    return m_unWorkerTypeID;
}


/*====================
  CEntityTeamInfo::CanSpawnWorker
  ====================*/
bool    CEntityTeamInfo::CanSpawnWorker(int iClientNum)
{
    // Request must come from commander
    if (GetCommanderClientID() != iClientNum)
        return false;

    // Check cooldown
    if (m_uiLastWorkerSpawnTime != INVALID_TIME && Game.GetGameTime() - m_uiLastWorkerSpawnTime < g_workerCoolDownTime)
        return false;

    // Lookup worker entity
    if (GetWorkerType() == INVALID_ENT_TYPE)
        return false;

    // Count existing
    uivector vWorkers(GetWorkerList());
    if (vWorkers.size() >= g_workerMaxActive)
        return false;

    return true;
}


/*====================
  CEntityTeamInfo::SpawnWorker
  ====================*/
void    CEntityTeamInfo::SpawnWorker(int iClientNum)
{
    if (!CanSpawnWorker(iClientNum))
        return;

    // Lookup worker entity
    ushort unType(GetWorkerType());
    if (unType == INVALID_ENT_TYPE)
        return;

    // Get base buildnig
    IBuildingEntity *pBaseBuilding(Game.GetBuildingEntity(GetBaseBuildingIndex()));
    if (pBaseBuilding == NULL)
        return;

    if (!SpendGold(g_workerCost))
        return;

    // Spawn the pet
    IGameEntity *pNewEnt(Game.AllocateEntity(unType));
    if (pNewEnt == NULL || pNewEnt->GetAsPet() == NULL)
    {
        Console.Warn << _T("Failed to spawn worker: ") << m_pDefinition->GetWorkerName() << newl;
        return;
    }
    IPetEntity *pPet(pNewEnt->GetAsPet());

    // Get spawn location
    CVec3f v3Spawn(V_ZERO);
    CVec3f v3Center(pBaseBuilding->GetPosition());

    float fX(pBaseBuilding->GetBounds().GetDim(X) / 2.0f);
    float fY(pBaseBuilding->GetBounds().GetDim(Y) / 2.0f);
    float fDist(1.0f * sqrt((fX * fX + fY * fY)));

    int iSpawnTries(0);
    float fBaseAngle(pBaseBuilding->GetAngles()[YAW]);
    float fAngle(0.0f);
    float fStep(2.0f);
    while (iSpawnTries < 100)
    {
        CVec3f v3Start(v3Center + CVec3f(0.0f, fDist, 0.0f));
        CVec3f v3PosA(M_RotatePointAroundLine(v3Start, v3Center, v3Center + V_UP, fBaseAngle + fAngle));
        CVec3f v3PosB(M_RotatePointAroundLine(v3Start, v3Center, v3Center + V_UP, fBaseAngle - fAngle));

        STraceInfo trace;
        Game.TraceBox(trace, v3PosA + V_UP * 1000.0f, v3PosA + V_UP * -1000.0f, pPet->GetBounds(), TRACE_PLAYER_MOVEMENT);
        if (!trace.bHit || (trace.plPlane.v3Normal.z > 1.0f - p_maxWalkSlope && (trace.uiSurfFlags & SURF_TERRAIN)))
        {
            v3Spawn = trace.v3EndPos;
            break;
        }

        Game.TraceBox(trace, v3PosB + V_UP * 1000.0f, v3PosB + V_UP * -1000.0f, pPet->GetBounds(), TRACE_PLAYER_MOVEMENT);
        if (!trace.bHit || (trace.plPlane.v3Normal.z > 1.0f - p_maxWalkSlope && (trace.uiSurfFlags & SURF_TERRAIN)))
        {
            v3Spawn = trace.v3EndPos;
            break;
        }

        ++iSpawnTries;
        fAngle += fStep;
    }

    CEntityClientInfo *pClient(Game.GetClientInfo(iClientNum));
    if (pClient != NULL)
    {
        IPlayerEntity *pPlayer(pClient->GetPlayerEntity());
        if (pPlayer != NULL)
            pPet->SetOwnerUID(pPlayer->GetUniqueID());
    }
    pPet->SetTeam(GetTeamID());
    pPet->SetPosition(v3Spawn);
    pPet->SetAngles(pBaseBuilding->GetAngles());
    pPet->Spawn();
    pPet->SetPetMode(PETMODE_PASSIVE);
    pPet->PlayerCommand(PETCMD_STOP, INVALID_INDEX, V3_ZERO);
    
    m_uiLastWorkerSpawnTime = Game.GetGameTime();
}


/*====================
  CEntityTeamInfo::GetWorkerCooldownPercent
  ====================*/
float   CEntityTeamInfo::GetWorkerCooldownPercent() const
{
    if (m_uiLastWorkerSpawnTime == INVALID_TIME)
        return 0.0f;

    return 1.0f - CLAMP((Game.GetGameTime() - m_uiLastWorkerSpawnTime) / g_workerCoolDownTime.GetFloat(), 0.0f, 1.0f);
}


/*====================
  CEntityTeamInfo::GetWorkerList
  ====================*/
uivector    CEntityTeamInfo::GetWorkerList()
{
    uivector vWorkers;

    ushort unType(GetWorkerType());
    if (unType == INVALID_ENT_TYPE)
        return vWorkers;

    IGameEntity *pEnt(Game.GetFirstEntity());
    while (pEnt != NULL)
    {
        IVisualEntity *pVisual(pEnt->GetAsVisualEnt());
        if (pVisual != NULL &&
            pVisual->GetType() == unType &&
            pVisual->GetTeam() == GetTeamID() &&
            pVisual->GetStatus() == ENTITY_STATUS_ACTIVE)
        {
            vWorkers.push_back(pVisual->GetIndex());
        }

        pEnt = Game.GetNextEntity(pEnt);
    }

    return vWorkers;
}
