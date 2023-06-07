// (C)2005 S2 Games
// c_gameserver.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_server_common.h"

#include "c_gameserver.h"
#include "c_serverentitydirectory.h"
#include "c_serverstatstracker.h"

#include "../game_shared/c_teamdefinition.h"
#include "../game_shared/c_playercommander.h"
#include "../game_shared/c_replaymanager.h"
#include "../game_shared/c_entitygameinfo.h"
#include "../game_shared/c_entityclientinfo.h"
#include "../game_shared/c_teaminfo.h"

#include "../k2/c_clientsnapshot.h"
#include "../k2/c_hostserver.h"
#include "../k2/c_world.h"
#include "../k2/c_worldentitylist.h"
#include "../k2/c_buffer.h"
#include "../k2/c_clientconnection.h"
#include "../k2/c_networkresourcemanager.h"
#include "../k2/c_worldentity.h"
#include "../k2/c_worldlight.h"
#include "../k2/c_filemanager.h"
#include "../k2/c_host.h"
#include "../k2/c_zip.h"
#include "../k2/c_host.h"
#include "../k2/c_model.h"
#include "../k2/c_effect.h"
#include "../k2/c_timermanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
SINGLETON_INIT(CGameServer)

CVAR_BOOLF(     sv_allUnitsAvailable,       false,          CONEL_DEV | CVAR_TRANSMIT);
CVAR_BOOLF(     sv_allItemsAvailable,       false,          CONEL_DEV | CVAR_TRANSMIT);
CVAR_BOOLF(     sv_unitH4x,                 false,          CONEL_DEV | CVAR_TRANSMIT);
CVAR_BOOLF(     sv_itemH4x,                 false,          CONEL_DEV | CVAR_TRANSMIT);
CVAR_BOOLF(     sv_commanderNpcControl,     false,          CONEL_DEV | CVAR_TRANSMIT);
CVAR_STRINGF(   svr_remotePass,             "",             CVAR_SAVECONFIG);

CVAR_INTF(      sv_maxTeamDifference,       1,              CVAR_SAVECONFIG | CVAR_SERVERINFO);
CVAR_UINTF(     sv_setupTimeCommander,      SecToMs(30u),   CVAR_GAMECONFIG);
CVAR_UINTF(     sv_setupTimeOfficers,       SecToMs(30u),   CVAR_GAMECONFIG);
CVAR_UINTF(     sv_setupTimeSquads,         SecToMs(30u),   CVAR_GAMECONFIG);
CVAR_UINTF(     sv_setupTimeCountdown,      SecToMs(5u),    CVAR_GAMECONFIG);
CVAR_UINTF(     sv_gameEndPhaseTime,        SecToMs(120u),  CVAR_GAMECONFIG);
CVAR_BOOLF(     sv_disablePhaseTransitions, false,          CONEL_DEV);
CVAR_BOOLF(     sv_precacheEntities,        true,           CVAR_SAVECONFIG);
CVAR_STRING(    sv_raceString,              "HvB");
CVAR_UINTF(     sv_totalMatches,            0,              CVAR_SAVECONFIG | CVAR_SERVERINFO);
CVAR_BOOLF(     sv_autosaveReplay,          false,          CVAR_SAVECONFIG);
CVAR_BOOLF(     sv_disableVoiceChat,        false,          CVAR_SAVECONFIG);
CVAR_BOOLF(     sv_debugVoiceChat,          false,          CVAR_SAVECONFIG);
CVAR_BOOLF(     sv_autosaveGameLog,         true,           CVAR_SAVECONFIG);
CVAR_UINTF(     sv_statusNotifyTime,        SecToMs(60u),   CVAR_SAVECONFIG);
CVAR_UINTF(     sv_afkTimeout,              MinToMs(5u),    CVAR_SAVECONFIG);

CVAR_BOOLF(     sv_warmup,                  true,           CVAR_SAVECONFIG);

CVAR_STRINGF(   sv_team1Race,               "Human",        CVAR_SAVECONFIG);
CVAR_STRINGF(   sv_team2Race,               "Beast",        CVAR_SAVECONFIG);

CVAR_FLOATF(    g_donationExpReward,        0.5f,           CVAR_GAMECONFIG);

#define END_GAME_LOG Console << _T("[END ") << Host.GetTime() << SPACE << ParenStr(GetGameTime()) << SPACE << ParenStr(GetServerFrame()) << _T("] - ")
//=============================================================================

/*====================
  CGameServer::~CGameServer
  ====================*/
CGameServer::~CGameServer()
{
    Console << _T("Game server released") << newl;
    SAFE_DELETE(m_pServerEntityDirectory);

    SAFE_DELETE(m_pDBManager);

    Console.StopGameLog();
}


/*====================
  CGameServer::CGameServer
  ====================*/
CGameServer::CGameServer() :
m_pServerEntityDirectory(NULL),
m_pHostServer(NULL),

m_pDBManager(NULL),

m_pGameInfo(NULL),

m_uiLastUpdateCheck(0),
m_uiLastGameLength(0),

m_uiLastStatusNotifyTime(INVALID_TIME)
{
    m_pServerEntityDirectory = K2_NEW(global,   CServerEntityDirectory);
}


/*====================
  CGameServer::AllocateEntity
  ====================*/
IGameEntity*    CGameServer::AllocateEntity(const tstring &sName, uint uiMinIndex)
{
    return m_pServerEntityDirectory->Allocate(sName, uiMinIndex);
}

IGameEntity*    CGameServer::AllocateEntity(ushort unType, uint uiMinIndex)
{
    return m_pServerEntityDirectory->Allocate(unType, uiMinIndex);
}


/*====================
  CGameServer::DeleteEntity
  ====================*/
void    CGameServer::DeleteEntity(IGameEntity *pEntity)
{
    if (!pEntity)
        return;
    
    pEntity->SetDelete(true);

    if (pEntity->IsVisual())
        pEntity->GetAsVisualEnt()->Unlink();
}

void    CGameServer::DeleteEntity(uint uiIndex)
{
    DeleteEntity(GetEntity(uiIndex));
}


/*====================
  CGameServer::Initialize
  ====================*/
bool    CGameServer::Initialize(CHostServer *pHostServer)
{
    try
    {
        // Store a pointer to the host server
        m_pHostServer = pHostServer;
        if (m_pHostServer == NULL)
            EX_ERROR(_T("Invalid CHostServer"));

        // Store a pointer to the world
        SetWorldPointer(m_pHostServer->GetWorld());
        SetEntityDirectory(m_pServerEntityDirectory);
        Validate();

        if (Host.IsReplay())
            return true;

        // Load game settings
        ICvar::SetTrackModifications(false);
        Console.ExecuteScript(_T("/game_settings.cfg"));
        ICvar::SetTrackModifications(true);

        InitCensor();

        // Setup a new database manager
        m_pDBManager = K2_NEW(global,   CDBManager)(_T("masterserver.savage2.s2games.com"), _T("/irc_updater/irc_requester.php"));

        if (sv_precacheEntities)
            PrecacheEntities();

        if (sv_warmup)
            SetGamePhase(GAME_PHASE_WARMUP);
        else
            SetGamePhase(GAME_PHASE_WAITING_FOR_PLAYERS);

        m_vItemDrops.clear();

        const EntAllocatorNameMap &mapAllocatorNames(EntityRegistry.GetAllocatorNames());
        for (EntAllocatorNameMap::const_iterator cit(mapAllocatorNames.begin()); cit != mapAllocatorNames.end(); ++cit)
        {
            const IEntityAllocator *pAllocator(cit->second);
            if (!pAllocator)
                continue;

            if (pAllocator->GetName().compare(0, 11, _T("Consumable_")) == 0)
                m_vItemDrops.push_back(pAllocator->GetID());
        }

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CGameServer::Initialize() - "), NO_THROW);
        return false;
    }
}


/*====================
  CGameServer::DatabaseFrame
  ====================*/
void    CGameServer::DatabaseFrame()
{
    PROFILE("CGameServer::DatabaseFrame");

    CDBResponse *pResponse;

    pResponse = m_pDBManager->Frame();

    if (pResponse != NULL)
    {
        if (pResponse->GetVarArray(_T("error")) == NULL && pResponse->GetVarString(_T("error")) == _T(""))
        {
            int iAccountID = AtoI(pResponse->GetResponseName());
            int iClientID = GetClientNumFromAccountID(iAccountID);

            Console.ServerGame << _T("Recieved item list for client #") << iClientID << _T(" (Account ID: ") << iAccountID << _T(")") << newl;

            int iLoop(0);
            CDBResponse *pItemData(NULL);
            SPersistantItemVault vault;
            CBufferDynamic buffer;

            buffer << GAME_CMD_PERSISTANT_ITEMS;

            while ((pItemData = pResponse->GetArrayNum(iLoop)) != NULL)
            {
                vault.unItemType[iLoop] = AtoI(pItemData->GetVarString(_T("type")));
                vault.uiItemID[iLoop] = AtoI(pItemData->GetVarString(_T("item_id")));

                Console.ServerGame << _T("Item #") << iLoop + 1 << _T(" - ID: ") << vault.uiItemID[iLoop]
                                    << _T(", Type: ") << vault.unItemType[iLoop] << _T(", Expiration Date: ")
                                    << pItemData->GetVarString(_T("exp_date")) << newl;

                buffer << vault.unItemType[iLoop] << vault.uiItemID[iLoop];

                iLoop++;
            }

            if (m_mapPersistantVaults.find(iClientID) != m_mapPersistantVaults.end())
                m_mapPersistantVaults.erase(iClientID);

            while (iLoop < MAX_PERSISTANT_ITEMS)
            {
                vault.unItemType[iLoop] = -1;
                vault.uiItemID[iLoop] = -1;
                buffer << ushort(-1) << uint(-1);
                iLoop++;
            }

            m_mapPersistantVaults.insert(PersistantVaultPair(iClientID, vault));
            
            SendGameData(iClientID, buffer, true);
        }
        else
        {
            if (pResponse->GetVarArray(_T("error")) != NULL)
            {
                Console.ServerGame << _T("Error retrieving item list for client #") << pResponse->GetResponseName()
                                << _T(" (") << pResponse->GetVarString(_T("nickname")) << _T("): ")
                                << pResponse->GetVarArray(_T("error"))->GetStringNum(0) << newl;
            }
            
            if (pResponse->GetVarString(_T("error")) != _T(""))
            {
                Console.ServerGame << _T("Error retrieving item list for client #") << pResponse->GetResponseName()
                                << _T(" (") << pResponse->GetVarString(_T("nickname")) << _T("): ")
                                << pResponse->GetVarString(_T("error")) << newl;
            }
        }
    }
}


/*====================
  CGameServer::Frame
  ====================*/
void    CGameServer::Frame()
{
    ReplayManager.StartFrame(m_pHostServer->GetFrameNumber());

    if (ReplayManager.IsPlaying())
    {
        MapStateString &mapStateString(ReplayManager.GetStateStrings());
        for (MapStateString::iterator it(mapStateString.begin()); it != mapStateString.end(); ++it)
        {
            m_pHostServer->SetStateString(it->first, it->second);
        }

        ClientMap &mapClients(m_pHostServer->GetClientMap());

        for (ClientMap_it it(mapClients.begin()); it != mapClients.end(); ++it)
        {
            CBufferDynamic cBufferReliable;
            ReplayManager.GetGameDataReliable(it->first, cBufferReliable);
            if (cBufferReliable.GetLength() > 0)
                it->second->SendGameData(cBufferReliable, true);

            CBufferDynamic cBuffer;
            ReplayManager.GetGameData(it->first, cBuffer);
            if (cBuffer.GetLength() > 0)
                it->second->SendGameData(cBuffer, false);
        }

        {
            CBufferDynamic cBufferReliable;
            ReplayManager.GetGameDataReliable(-1, cBufferReliable);
            if (cBufferReliable.GetLength() > 0)
                BroadcastGameData(cBufferReliable, true);

            CBufferDynamic cBuffer;
            ReplayManager.GetGameData(-1, cBuffer);
            if (cBuffer.GetLength() > 0)
                BroadcastGameData(cBuffer, false);
        }

        return;
    }

    // Check for updates... Servers check every 2 minutes
    if (K2System.IsDedicatedServer() && K2System.Milliseconds() - m_uiLastUpdateCheck >= 120000)
    {
        m_pHostServer->SilentUpdate();
        m_uiLastUpdateCheck = K2System.Milliseconds();
    }

    for (ClientInfoMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
    {
        if (it->second->IsDisconnected())
            continue;

        CClientConnection *pClientConnection(m_pHostServer->GetClient(it->second->GetClientNumber()));
        if (pClientConnection == NULL)
            continue;

        // Update ping
        // TODO: might want to average these out or something so it's a little smoother
        if (m_pHostServer->GetFrameNumber() % 100 != 0) // HACK: Easy way to limit this for now
            continue;

        it->second->SetPing(pClientConnection->GetPing());

        // Kick idle clients
        if (GetGamePhase() == GAME_PHASE_WARMUP)
            it->second->SetLastInputTime(Game.GetGameTime());

        if (!pClientConnection->IsLocalClient() && Game.GetGameTime() - it->second->GetLastInputTime() > sv_afkTimeout)
        {
            m_pHostServer->KickClient(it->first, _T("You were disconnected for being idle"));
            continue;
        }
    }

    if (GetGamePhase() != GAME_PHASE_STANDBY)
    {
        if (GetGamePhase() < GAME_PHASE_ACTIVE)
            SetupFrame();
        else if (GetGamePhase() == GAME_PHASE_ACTIVE || GetGamePhase() == GAME_PHASE_WARMUP)
            ActiveFrame();
        else if (GetGamePhase() == GAME_PHASE_ENDED)
            EndedFrame();
    }

    DatabaseFrame();

    TriggerManager.TriggerGlobalScript(_T("frame"));
}


/*====================
  CGameServer::GetClientCount
  ====================*/
int CGameServer::GetClientCount(int iTeam)
{
    if (iTeam == -1)
        return int(m_mapClients.size());

    CEntityTeamInfo *pTeam(GetTeam(iTeam));
    if (pTeam == NULL)
        return 0;

    return pTeam->GetNumClients();
}


/*====================
  CGameServer::GetConnectedClientCount
  ====================*/
int CGameServer::GetConnectedClientCount(int iTeam)
{
    if (iTeam == -1)
    {
        int iNumClients(0);

        for (ClientInfoMap_it it(m_mapClients.begin()); it != m_mapClients.end(); it++)
            if (!it->second->IsDisconnected())
                iNumClients++;

        return iNumClients;
    }

    CEntityTeamInfo *pTeam(GetTeam(iTeam));
    if (pTeam == NULL)
        return 0;

    return pTeam->GetNumClients();
}


/*====================
  CGameServer::StartWarmup
  ====================*/
void    CGameServer::StartWarmup()
{
    m_pServerEntityDirectory->WarmupStart();

    SetGamePhase(GAME_PHASE_WARMUP);

    TriggerManager.TriggerGlobalScript(_T("warmupstart"));
}


/*====================
  CGameServer::StartGame
  ====================*/
void    CGameServer::StartGame()
{
    if (GetGamePhase() >= GAME_PHASE_ACTIVE && GetGamePhase() != GAME_PHASE_WARMUP)
        return;

    if (GetGamePhase() == GAME_PHASE_WAITING_FOR_PLAYERS)
        RemoveAllVoiceClients();

    if (GetGamePhase() == GAME_PHASE_WARMUP)
        ResetWorld();

    if (sv_autosaveGameLog)
    {
        tstring sLogFilename(FileManager.GetNextFileIncrement(6, _T("~/logs/") + K2System.GetVersionString() + _T("/game_"), _T("txt")));
        Console.StartGameLog(sLogFilename);
    }

    m_pServerEntityDirectory->GameStart();

    SetGamePhase(GAME_PHASE_ACTIVE);
    const vector<CEntityTeamInfo*> &vTeams(GetTeams());
    for (vector<CEntityTeamInfo*>::const_iterator cit(vTeams.begin()); cit != vTeams.end(); ++cit)
    {
        (*cit)->Initialize();

        if ((*cit)->GetTeamID() > 0 && (*cit)->GetNumOfficers() < (*cit)->GetMaxOfficers())
            (*cit)->PromoteFromTeam();
    }

    if (StatsTracker.HasMatchStarted())
        StatsTracker.EndMatch();

    tstring sMap;

    if (Game.GetWorldPointer() != NULL && Game.GetWorldPointer()->IsLoaded())
        sMap = Game.GetWorldPointer()->GetName();

    StatsTracker.StartMatch(m_pHostServer->GetServerLogin(), m_pHostServer->GetServerPassword(), false, sMap, m_pHostServer->GetServerPort(), m_mapClients);

    if (sv_autosaveReplay || StatsTracker.HasMatchStarted())
    {
        tstring sFilename(FileManager.GetNextFileIncrement(5, _T("~/replays/") + m_pHostServer->GetWorld()->GetName(), _T("s2r")));
        ReplayManager.StartRecording(sFilename);
    }

    // Spawn commanders
    for (int i(1); i < GetNumTeams(); i++)
    {
        CEntityTeamInfo *pTeam(GetTeam(i));

        if (pTeam == NULL)
            continue;

        if (pTeam->GetCommanderClientID() == -1)
            continue;

        IPlayerEntity *pCommander(Game.ChangeUnit(pTeam->GetCommanderClientID(), Player_Commander, CHANGE_UNIT_REFUND_GOLD));

        if (pCommander == NULL)
            continue;

        pCommander->Spawn();
    }

    TriggerManager.TriggerGlobalScript(_T("gamestart"));

    m_uiLastStatusNotifyTime = GetGameTime();
}


/*====================
  CGameServer::EndGame
  ====================*/
void    CGameServer::EndGame(int iLosingTeam)
{
    if (Game.GetGamePhase() == GAME_PHASE_ENDED)
        return;

    if (iLosingTeam <= 0 || iLosingTeam >= 3)
    {
        Console.Warn << _T("EndGame: Invalid team, game will not end: ") << iLosingTeam << newl;
        return;
    }

    m_uiLastGameLength = GetCurrentGameLength();

    Game.SetGamePhase(GAME_PHASE_ENDED, sv_gameEndPhaseTime);
    SetWinningTeam(iLosingTeam ^ 3);
    
    // Notify clients who the winner is
    CBufferDynamic buffer;
    buffer << GAME_CMD_END_GAME << GetWinningTeam() << sv_gameEndPhaseTime.GetUnsignedInteger() << Host.GetNextMap() << byte(0);
    
    BroadcastGameData(buffer, true);

    // Tell every player entity to play it's victory or defeat anim
    for (ClientInfoMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
    {
        IPlayerEntity *pPlayer(it->second->GetPlayerEntity());
        if (pPlayer == NULL)
            continue;

        if (pPlayer->GetTeam() == GetWinningTeam())
            pPlayer->StartAnimation(_T("victory"), 0);
        else
            pPlayer->StartAnimation(_T("defeat"), 0);
    }

    SendStats();

    TriggerManager.RegisterTriggerParam(_T("winningteam"), XtoA(GetWinningTeam()));
    TriggerManager.RegisterTriggerParam(_T("losingteam"), XtoA(iLosingTeam));
    TriggerManager.TriggerGlobalScript(_T("gameend"));

    Console.StopGameLog();
}


/*====================
  CGameServer::SetupFrame
  ====================*/
void    CGameServer::SetupFrame()
{
    PROFILE("CGameServer::SetupFrame");

    SetGameTime(m_pHostServer->GetTime());
    SetFrameLength(m_pHostServer->GetFrameLength());

    m_pServerEntityDirectory->BackgroundFrame();

    // Stats
    StatsTracker.Frame();

    CEntityTeamInfo *pTeam1(GetTeam(1));
    CEntityTeamInfo *pTeam2(GetTeam(2));
    if (pTeam1 == NULL || pTeam2 == NULL)
        return;

    // Check for transitions
    switch (GetGamePhase())
    {
    case GAME_PHASE_WAITING_FOR_PLAYERS:
        // When minimum player requirement is met, select a commander
        if (pTeam1->GetNumClients() >= GetWorldPointer()->GetMinPlayersPerTeam() &&
            pTeam2->GetNumClients() >= GetWorldPointer()->GetMinPlayersPerTeam() &&
            !sv_disablePhaseTransitions)
        {
            // Remove all voice clients, as everyone was allowed to talk to everyone else
            // before this phase - they will be readded (but limited) from here on out.
            RemoveAllVoiceClients();

            SetGamePhase(GAME_PHASE_SELECTING_COMMANDER, GetGamePhase() == GAME_PHASE_WAITING_FOR_PLAYERS ? sv_setupTimeCommander : sv_setupTimeCommander + SecToMs(30u));
        }
        break;

    case GAME_PHASE_SELECTING_COMMANDER:
        {
            bool bReady(true);
            for (int iTeam(1); iTeam <= 2; ++iTeam)
            {
                CEntityTeamInfo *pTeam(Game.GetTeam(iTeam));

                if ((pTeam->IsMajoritySelection() && !sv_disablePhaseTransitions) || Game.GetGameTime() >= GetPhaseEndTime())
                    continue;

                bReady = false;
            }

            //If commanders are selected, the selecting officers phase has normal time. otherwise it has 0 seconds
            if (bReady)
            {
                if (pTeam1->SelectCommander())
                    SetGamePhase(GAME_PHASE_SELECTING_OFFICERS, sv_setupTimeOfficers);
                if (pTeam2->SelectCommander())
                    SetGamePhase(GAME_PHASE_SELECTING_OFFICERS, sv_setupTimeOfficers);
                if (!pTeam1->SelectCommander() && !pTeam2->SelectCommander())
                    SetGamePhase(GAME_PHASE_SELECTING_OFFICERS, SecToMs(0u));
            }
        }
        break;

    case GAME_PHASE_SELECTING_OFFICERS:
        {
            bool bReady(true);
            for (int iTeam(1); iTeam <= 2; ++iTeam)
            {
                CEntityTeamInfo *pTeam(Game.GetTeam(iTeam));

                // When all available officer positions are filled, go to next phase
                if (pTeam->GetNumOfficers() == pTeam->GetMaxOfficers() && !sv_disablePhaseTransitions)
                    continue;

                // If the commander takes too long, just assign some officers
                if (Game.GetGameTime() >= GetPhaseEndTime())
                    continue;

                bReady = false;
            }

            if (bReady)
            {
                SetGamePhase(GAME_PHASE_FORMING_SQUADS, sv_setupTimeSquads);
                pTeam1->SelectOfficers();
                pTeam2->SelectOfficers();
            }
        }
        break;

    case GAME_PHASE_FORMING_SQUADS:
        // If people take too long, just assign them to squads
        if (Game.GetGameTime() >= GetPhaseEndTime())
        {
            pTeam1->FillSquads();
            pTeam2->FillSquads();
            StartGame();
            break;
        }

        // If everyone is on a squad, start the countdown
        if (pTeam1->AreAllPlayersInSquads() &&
            pTeam2->AreAllPlayersInSquads() &&
            GetPhaseEndTime() - Game.GetGameTime() > sv_setupTimeCountdown &&
            !sv_disablePhaseTransitions)
        {
            SetGamePhase(GAME_PHASE_FORMING_SQUADS, sv_setupTimeCountdown);
        }
        break;
    }

    if (sv_disablePhaseTransitions)
        return;

    // If a team is missing officers, revert to Phase III
    if ((pTeam1->GetNumOfficers() < pTeam1->GetMaxOfficers() || pTeam2->GetNumOfficers() < pTeam2->GetMaxOfficers()) &&
        (GetGamePhase() > GAME_PHASE_SELECTING_OFFICERS))
        SetGamePhase(GAME_PHASE_SELECTING_OFFICERS, sv_setupTimeOfficers);

    // If either team is missing a commander, revert to Phase II. COMMENTED OUT due to changes no longer forcing players to take comm unless they select it
/*  if (!sv_allowNoCommander)
    {
        if ((pTeam1->GetCommanderClientID() == -1 || pTeam2->GetCommanderClientID() == -1) &&
            (GetGamePhase() > GAME_PHASE_SELECTING_COMMANDER))
            SetGamePhase(GAME_PHASE_SELECTING_COMMANDER, sv_setupTimeCommander);
    }*/

    // If the number of players drops below the minimum, revert to Phase I
    if ((pTeam1->GetNumClients() < GetWorldPointer()->GetMinPlayersPerTeam() || pTeam2->GetNumClients() < GetWorldPointer()->GetMinPlayersPerTeam()) &&
        (GetGamePhase() > GAME_PHASE_WAITING_FOR_PLAYERS))
    {
        if (sv_warmup)
        {
            ResetWorld();
            StartWarmup();
        }
        else
            SetGamePhase(GAME_PHASE_WAITING_FOR_PLAYERS);

        ivector vClients1(pTeam1->GetClientList());
        ivector vClients2(pTeam2->GetClientList());
        for (ivector_it it(vClients1.begin()); it != vClients1.end(); it++)
        {
            CEntityClientInfo *pClient(Game.GetClientInfo(*it));
            if (pClient != NULL)
            {
                pClient->RemoveFlags(CLIENT_INFO_IS_COMMANDER);
                pClient->RemoveFlags(CLIENT_INFO_WANTS_TO_COMMAND);
                pClient->RemoveFlags(CLIENT_INFO_IS_OFFICER);
            }

        }
        for (ivector_it it(vClients2.begin()); it != vClients2.end(); it++)
        {
            CEntityClientInfo *pClient(Game.GetClientInfo(*it));
            if (pClient != NULL)
            {
                pClient->RemoveFlags(CLIENT_INFO_IS_COMMANDER);
                pClient->RemoveFlags(CLIENT_INFO_WANTS_TO_COMMAND);
                pClient->RemoveFlags(CLIENT_INFO_IS_OFFICER);
            }

        }
    }

    TriggerManager.TriggerGlobalScript(_T("setupframe"));
}


/*====================
  CGameServer::ActiveFrame
  ====================*/
void    CGameServer::ActiveFrame()
{
    PROFILE("CGameServer::ActiveFrame");

    CEntityTeamInfo *pTeam1(GetTeam(1));
    CEntityTeamInfo *pTeam2(GetTeam(2));
    if (pTeam1 == NULL || pTeam2 == NULL)
        return;

    if (GetGamePhase() == GAME_PHASE_WARMUP && GetWorldPointer()->IsLoaded())
    {
        // When minimum player requirement is met, select a commander
        if (pTeam1->GetNumClients() >= GetWorldPointer()->GetMinPlayersPerTeam() &&
            pTeam2->GetNumClients() >= GetWorldPointer()->GetMinPlayersPerTeam() &&
            !sv_disablePhaseTransitions)
        {
            // Remove all voice clients, as everyone was allowed to talk to everyone else
            // before this phase - they will be readded (but limited) from here on out.
            RemoveAllVoiceClients();

            SetGamePhase(GAME_PHASE_SELECTING_COMMANDER, sv_setupTimeCommander + SecToMs(30u));

            ResetWorld();
        }
    }

    if (GetGamePhase() == GAME_PHASE_ACTIVE)
    {
        ClientInfoMap_it it(m_mapClients.begin());

        while (it != m_mapClients.end())
        {
            if (it->second->IsDisconnected() || !it->second->IsDemoAccount())
            {
                it++;
                continue;
            }

            if (it->second->GetDemoTimeRemaining() < GetCurrentGameLength() && it->second->GetTeam() != 0)
            {
                // Force player to go into spectate once their demo account expires
                ChangeTeam(it->first, 0);
                
                if (it->second->GetPlayerEntity() != NULL)
                    it->second->GetPlayerEntity()->Spawn2();

                //Kick(it->first, _T("Your demo account has expired"));
            }

            it++;
        }
    }

    if (sv_statusNotifyTime && GetGameTime() >= m_uiLastStatusNotifyTime + sv_statusNotifyTime)
    {
        Console << GetServerStatus() << newl;
        m_uiLastStatusNotifyTime = GetGameTime(); 
    }

    SetGameTime(m_pHostServer->GetTime());
    SetFrameLength(m_pHostServer->GetFrameLength());

    // Update server navigation
    UpdateNavigation();

    // Execute entity frames
    m_pServerEntityDirectory->Frame();

    StatsTracker.Frame();

    // Check for sudden death
    bool bSuddenDeath(true);
    IGameEntity *pNextEntity(GetFirstEntity());
    while (pNextEntity != NULL)
    {
        IGameEntity *pEntity(pNextEntity);
        pNextEntity = GetNextEntity(pEntity);

        IPropEntity *pProp(pEntity->GetAsProp());
        if (pProp == NULL)
            continue;
        IPropFoundation *pFoundation(pProp->GetAsFoundation());
        if (pFoundation == NULL)
            continue;
        if (pFoundation->GetRemainingGold() > 0)
        {
            bSuddenDeath = false;
            break;
        }
    }
    SetSuddenDeath(bSuddenDeath);

    if (GetGamePhase() == GAME_PHASE_ACTIVE)
        TriggerManager.TriggerGlobalScript(_T("activeframe"));
    else
        TriggerManager.TriggerGlobalScript(_T("warmupframe"));
}


/*====================
  CGameServer::EndedFrame
  ====================*/
void    CGameServer::EndedFrame()
{
    PROFILE("CGameServer::EndedFrame");

    SetGameTime(m_pHostServer->GetTime());
    SetFrameLength(m_pHostServer->GetFrameLength());

    m_pServerEntityDirectory->Frame();

    StatsTracker.Frame();

    TriggerManager.TriggerGlobalScript(_T("endedframe"));

    // Check for phase transition
    if (GetGameTime() >= GetPhaseEndTime() && GetGamePhase() == GAME_PHASE_ENDED)
    {
        Restart();
        return;
    }
}


/*====================
  CGameServer::SendStats
  ====================*/
void    CGameServer::SendStats()
{
    // Submit detailed end-game stats to clients
    PROFILE("CGameServer::SendStats()");

    // Fill a buffer with all the stat data
    CBufferDynamic bufferStats;
    bufferStats << byte(m_mapClients.size());

    for (ClientInfoMap_it itClient(m_mapClients.begin()); itClient != m_mapClients.end(); ++itClient)
        itClient->second->WriteMatchStatBuffer(bufferStats);

    uint uiOriginalSize(INT_SIZE(bufferStats.GetLength()));
    byte* pCompressed(NULL);

    uint uiCompressedSize(CZip::Compress((const byte*)bufferStats.Get(), uiOriginalSize, pCompressed));
    Console.ServerGame << _T("Sending end game statistics... Size: ") << uiOriginalSize << _T(", compressed size: ") << uiCompressedSize << newl;

    CBufferStatic bufferSend(uiCompressedSize + 8);
    bufferSend << uiCompressedSize << uiOriginalSize;
    bufferSend.Append(pCompressed, uiCompressedSize);
    SAFE_DELETE_ARRAY(pCompressed);

    m_pHostServer->BroadcastEndGameStats(bufferSend, GAME_CMD_END_GAME_TERMINATION, GAME_CMD_END_GAME_FRAGMENT);
}


/*====================
  CGameServer::Restart
  ====================*/
void    CGameServer::Restart()
{
    Console << _T("Restart") << newl;

    ReplayManager.StopRecording();

    StatsTracker.SubmitStats(m_mapClients, m_pHostServer->GetWorld()->GetName());
    StatsTracker.SubmitAllData();
    StatsTracker.EndMatch();


    m_setCommanderReviewSubmitted.clear();
    m_setKarmaReviewSubmitted.clear();

    ++sv_totalMatches;

    // Note that we delete the replay after it's recorded rather than just not
    // record it so that we can submit it with the match summary for stats
    if (!sv_autosaveReplay && FileManager.Exists(ReplayManager.GetReplayFilename()))
        FileManager.Delete(ReplayManager.GetReplayFilename());

    ////

    m_pHostServer->GameEnded();

    m_mapClients.clear();
    m_pServerEntityDirectory->Clear();

    if (!LoadNextMap())
    {
        Console.Err << _T("Server could not load the next map!") << newl;
        Shutdown();
        return;
    }

    m_pHostServer->NewGameStarted();
}


/*====================
  CGameServer::LoadNextMap
  ====================*/
bool    CGameServer::LoadNextMap()
{
    if (!m_pHostServer->LoadWorld(Host.GetNextMap()))
        return false;

    return true;
}


/*====================
  CGameServer::SetRace
  ====================*/
void    CGameServer::SetRace(int iTeam, const tstring &sRaceName)
{
    if (GetGamePhase() >= GAME_PHASE_ACTIVE && GetGamePhase() != GAME_PHASE_WARMUP)
        return;

    CEntityTeamInfo *pTeam(GetTeam(iTeam));
    if (pTeam == NULL)
        return;
    if (CompareNoCase(pTeam->GetDefinition()->GetName(), sRaceName) == 0)
        return;

    if (CompareNoCase(sRaceName, _T("Human")) == 0)
        pTeam->SetDefinition(&g_teamdefHuman);
    else if (CompareNoCase(sRaceName, _T("Beast")) == 0)
        pTeam->SetDefinition(&g_teamdefBeast);
}


/*====================
  CGameServer::AddClient
  ====================*/
void    CGameServer::AddClient(CClientConnection *pClientConnection)
{
    try
    {
        if (pClientConnection == NULL)
            EX_ERROR(_T("Invalid CClientConnection from host"));

        IGame *pGame(Game.GetCurrentGamePointer());
        Game.SetCurrentGamePointer(this);

        int iClientNumber(pClientConnection->GetClientNum());

        CEntityClientInfo *pClient(NULL);
    
        ClientInfoMap_it itFind(m_mapClients.find(iClientNumber));
        if (itFind == m_mapClients.end())
        {
            // Allocate entities for this client
            IGameEntity* pNewClient(m_pServerEntityDirectory->Allocate(Entity_ClientInfo));
            if (pNewClient == NULL)
                EX_ERROR(_T("Failed to allocate new client"));
            pClient = static_cast<CEntityClientInfo*>(pNewClient);
            m_mapClients[iClientNumber] = pClient;
        }
        else
        {
            pClient = itFind->second;
        }

        pClient->Initialize(pClientConnection);

        if (pClientConnection->IsDemoAccount())
            pClient->SetDemoTimeRemaining(pClientConnection->GetDemoTimeRemaining() + GetCurrentGameLength());
        else
            pClient->SetDemoTimeRemaining(INVALID_TIME);

        if (ReplayManager.IsPlaying())
        {
            Game.SetCurrentGamePointer(pGame);
            return;
        }

        int iAccountID(pClient->GetAccountID());

        if (iAccountID != -1)
        {
            Console << _T("Getting persistant stats for client ") << iClientNumber << _T(" (Account ID: ") << iAccountID << _T(").") << newl;
            StatsTracker.RetrieveStats(iAccountID);

            Console.ServerGame << _T("Requesting item list for client #") << iClientNumber << _T(" (ID: ") << iAccountID << _T(")") << newl;
            m_pDBManager->AddRequestVariable(_T("f"), _T("item_list"));
            m_pDBManager->AddRequestVariable(_T("account_id"), XtoA(iAccountID));

            // Set the request name to their account number, so we know
            // who we're referring to.
            m_pDBManager->SendRequest(XtoA(iAccountID));
        }

        SendMessage(pClient->GetName() + _T(" has connected."), -1);

        CBufferDynamic buffer(32);
        buffer << GAME_CMD_SERVERCHAT_ALL << _T("^226") << pClient->GetName() << _T(" ^900has connected.") << byte(0);
        BroadcastGameData(buffer, false);

        ChangeTeam(iClientNumber, 0);

        if (GetGamePhase() >= GAME_PHASE_ENDED)
        {
            // Notify client who the winner is
            buffer.Clear();
            buffer << GAME_CMD_END_GAME << GetWinningTeam() << GetRemainingPhaseTime() << Host.GetNextMap() << byte(0);
            SendGameData(iClientNumber, buffer, true);
        }

        Console.ServerGame << _T("Spawned new client #") << pClient->GetClientNumber() << newl;

        TriggerManager.RegisterTriggerParam(_T("name"), pClient->GetName());
        TriggerManager.RegisterTriggerParam(_T("accountid"), XtoA(pClient->GetAccountID()));
        TriggerManager.RegisterTriggerParam(_T("clientid"), XtoA(pClient->GetClientNumber()));
        TriggerManager.RegisterTriggerParam(_T("index"), XtoA(pClient->GetPlayerEntityIndex()));
        TriggerManager.TriggerGlobalScript(_T("playerjoin"));

        Game.SetCurrentGamePointer(pGame);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CGameServer::AddClient() - "), NO_THROW);
    }
}

/*====================
  CGameServer::ReauthClient
  ====================*/
void    CGameServer::ReauthClient(CClientConnection *pClientConnection)
{
    try
    {
        if (pClientConnection == NULL)
            EX_ERROR(_T("Invalid CClientConnection from host"));

        IGame *pGame(Game.GetCurrentGamePointer());
        Game.SetCurrentGamePointer(this);

        int iClientNumber(pClientConnection->GetClientNum());

        CEntityClientInfo *pClient(NULL);
    
        ClientInfoMap_it itFind(m_mapClients.find(iClientNumber));
        if (itFind == m_mapClients.end())
            return;
        else
            pClient = itFind->second;

        if (pClientConnection->IsDemoAccount())
            pClient->SetDemoTimeRemaining(pClientConnection->GetDemoTimeRemaining() + GetCurrentGameLength());
        else
            pClient->SetDemoTimeRemaining(INVALID_TIME);

        Game.SetCurrentGamePointer(pGame);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CGameServer::AddClient() - "), NO_THROW);
    }
}

/*====================
  CGameServer::RemoveClient
  ====================*/
void    CGameServer::RemoveClient(int iClientNum)
{
    try
    {
        ClientInfoMap_it itClient(m_mapClients.find(iClientNum));
        if (itClient == m_mapClients.end())
            EX_ERROR(_T("Client doesnt exist"));
        CEntityClientInfo *pClient(itClient->second);

        if (pClient->IsDisconnected())
            EX_ERROR(_T("Client is already disconnected"));

        int iTeam(pClient->GetTeam());

        TriggerManager.RegisterTriggerParam(_T("name"), pClient->GetName());
        TriggerManager.RegisterTriggerParam(_T("accountid"), XtoA(pClient->GetAccountID()));
        TriggerManager.RegisterTriggerParam(_T("clientid"), XtoA(pClient->GetClientNumber()));
        TriggerManager.RegisterTriggerParam(_T("index"), XtoA(pClient->GetPlayerEntityIndex()));
        TriggerManager.TriggerGlobalScript(_T("playerleave"));
        
        pClient->SetDisconnected(true);
        pClient->SetDisconnectTime(GetGameTime());

        IPlayerEntity *pPlayer(pClient->GetPlayerEntity());
        if (pPlayer != NULL)
        {
            pPlayer->CancelBuild();
            m_pServerEntityDirectory->Delete(pPlayer->GetIndex());
        }

        SendMessage(pClient->GetName() + _T(" has disconnected."), -1);

        CBufferDynamic buffer;
        if (pClient->GetTeam() > 0)
            buffer << GAME_CMD_SERVERCHAT_ALL << _T("^226") << pClient->GetName() << _T(" (Team ") << XtoA(iTeam) << _T(") ^900has disconnected.") << byte(0);
        else
            buffer << GAME_CMD_SERVERCHAT_ALL << _T("^226") << pClient->GetName() << _T(" ^900has disconnected.") << byte(0);
        BroadcastGameData(buffer, false);

        RemoveVoiceClient(iClientNum);

        CEntityTeamInfo *pTeam(GetTeam(iTeam));
        if (pTeam != NULL)
            pTeam->RemoveClient(iClientNum);

        pClient->SetTeam(-1);
        pClient->SetSquad(INVALID_SQUAD);

        if (m_mapPersistantVaults.find(iClientNum) != m_mapPersistantVaults.end())
            m_mapPersistantVaults.erase(iClientNum);

        Console.ServerGame << _T("Removed client #") << iClientNum << newl;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CGameServer::RemoveClient() - "), NO_THROW);
    }
}


/*====================
  CGameServer::ProcessClientSnapshot
  ====================*/
bool    CGameServer::ProcessClientSnapshot(int iClientNum, CClientSnapshot &snapshot)
{
    PROFILE("CGameServer::ProcessClientSnapshot");

    try
    {
        if (ReplayManager.IsPlaying())
            return true;

        CEntityClientInfo *pClient(GetClientInfo(iClientNum));
        if (pClient == NULL)
            EX_ERROR(_T("Received snapshot with invalid client number ") + XtoA(iClientNum));

        IPlayerEntity *pPlayer(pClient->GetPlayerEntity());
        if (pPlayer != NULL)
        {
            if (snapshot.HasInput(pPlayer->GetViewAngles(), pPlayer->GetCursorPos(), pPlayer->GetSelectedItem()))
                pClient->SetLastInputTime(Game.GetGameTime());

            uint uiRealGameTime(GetGameTime());
            uint uiRealFrameLength(GetFrameLength());

            SetGameTime(snapshot.GetTimeStamp());
            SetFrameLength(snapshot.GetFrameLength());
            SeedRand(snapshot.GetTimeStamp() * snapshot.GetServerFrame());

            pPlayer->ReadClientSnapshot(snapshot);

            SetGameTime(uiRealGameTime);
            SetFrameLength(uiRealFrameLength);
        }
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CGameServer::ProcessClientSnapshot() - "));
        return false;
    }
}


/*====================
  CGameServer::SellItem
  ====================*/
void    CGameServer::SellItem(int iSlot, IPlayerEntity *pPlayer, int iNumSold)
{
    if (iSlot < INVENTORY_START_BACKPACK || iSlot >= INVENTORY_END_BACKPACK)
        return;

    IInventoryItem *pItem(pPlayer->GetItem(iSlot));

    if (pItem != NULL)
    {
        TriggerManager.RegisterTriggerParam(_T("slot"), XtoA(iSlot + 1));
        TriggerManager.RegisterTriggerParam(_T("numsold"), XtoA(iNumSold));
        TriggerManager.RegisterTriggerParam(_T("itemname"), pItem->GetName());
        TriggerManager.RegisterTriggerParam(_T("name"), pPlayer->GetClientName());
        TriggerManager.RegisterTriggerParam(_T("clientid"), XtoA(pPlayer->GetClientID()));
        TriggerManager.RegisterTriggerParam(_T("index"), XtoA(pPlayer->GetIndex()));
        TriggerManager.TriggerGlobalScript(_T("sellitem"));

        if (pPlayer->GetAmmoCount(iSlot) >= iNumSold)
        {
            if (!sv_itemH4x)
                pPlayer->GiveGold(pItem->GetCost() * iNumSold);

            pPlayer->SetAmmo(iSlot, pPlayer->GetAmmoCount(iSlot) - iNumSold);
        }
        else
        {
            if (!sv_itemH4x)
                pPlayer->GiveGold(pItem->GetCost() * pPlayer->GetAmmoCount(iSlot));

            pPlayer->SetAmmo(iSlot, 0);
        }

        if (pPlayer->GetAmmoCount(iSlot) <= 0)
            pPlayer->RemoveItem(iSlot);
    }
}


/*====================
  CGameServer::PurchasePersistantItem
  ====================*/
int     CGameServer::PurchasePersistantItem(int iClientNum, int iVaultNum)
{
    IPlayerEntity *pPlayer;
    int iSlot(-1);
    int iNumPersItemsCarried(0);
    uint uiID;
    ushort unType;
    bool bSuccess(false);

    Console << _T("Client #") << iClientNum << _T(" requested purchase of persistant item, vault slot: ") << iVaultNum + 1 << newl;

    if (iVaultNum < 0 || iVaultNum > MAX_PERSISTANT_ITEMS)
        return -1;

    // Demo accounts cannot use persistant items
    if (GetClientInfo(iClientNum) != NULL && GetClientInfo(iClientNum)->IsDemoAccount())
        return -1;

    pPlayer = GetPlayerFromClientNum(iClientNum);

    if (pPlayer == NULL)
        return -1;

    if (pPlayer->GetStatus() != ENTITY_STATUS_DORMANT)
    {
        Console << _T("Cannot purchase items from this status.") << newl;
        return -1;
    }

    if (!pPlayer->GetCanPurchase())
    {
        Console << _T("Cannot purchase items with this class.") << newl;
        return -1;
    }

    unType = GetPersistantItemType(iClientNum, iVaultNum);
    uiID = GetPersistantItemID(iClientNum, iVaultNum);

    if (unType == PERSISTANT_ITEM_NULL)
    {
        Console << _T("No persistant item in that vault slot.") << newl;
        return -1;
    }

    // Check for duplicate persistant items
    for (int i(INVENTORY_START_BACKPACK); i < INVENTORY_END_BACKPACK; ++i)
    {
        if (pPlayer->GetItem(i) != NULL && pPlayer->GetItem(i)->IsPersistant())
        {
            iNumPersItemsCarried++;

            if (pPlayer->GetItem(i)->GetAsPersistant()->GetItemID() == uiID)
            {
                Console << _T("That persistant item is already being held.") << newl;
                return -1;
            }

            if (iNumPersItemsCarried >= MAX_CARRIED_PERSISTANT_ITEMS)
            {
                Console << _T("Already carrying the maximum number of persistant items.") << newl;
                return -1;
            }
        }
    }

    for (int i(INVENTORY_START_BACKPACK); i < INVENTORY_END_BACKPACK && !bSuccess; i++)
    {
        if (pPlayer->GetItem(i) == NULL)
        {
            pPlayer->GiveItem(i, Persistant_Item);

            if (pPlayer->GetItem(i) != NULL)
            {
                if (pPlayer->GetItem(i)->GetAsPersistant() != NULL)
                {
                    pPlayer->GetItem(i)->GetAsPersistant()->SetItemData(unType);
                    pPlayer->GetItem(i)->GetAsPersistant()->SetItemID(uiID);
                    pPlayer->GetItem(i)->GetAsPersistant()->ActivatePassive();
                    iSlot = i;
                    bSuccess = true;
                }
                else
                    pPlayer->RemoveItem(i);
            }
        }
    }

    if (iSlot == -1)
    {
        Console << _T("Could not find an empty inventory space.") << newl;
        return -1;
    }

    Console << _T("Persistant item purchased and placed in slot ") << iSlot << newl;

    TriggerManager.RegisterTriggerParam(_T("slot"), XtoA(iSlot + 1));
    TriggerManager.RegisterTriggerParam(_T("itemname"), pPlayer->GetItem(iSlot)->GetName());
    TriggerManager.RegisterTriggerParam(_T("name"), pPlayer->GetClientName());
    TriggerManager.RegisterTriggerParam(_T("clientid"), XtoA(pPlayer->GetClientID()));
    TriggerManager.RegisterTriggerParam(_T("index"), XtoA(pPlayer->GetIndex()));
    TriggerManager.TriggerGlobalScript(_T("buyitem"));

    return iSlot;
}


/*====================
  CGameServer::GivePersistantItem
  ====================*/
int     CGameServer::GivePersistantItem(int iClientNum, ushort unItemData)
{
    IPlayerEntity *pPlayer(GetPlayerFromClientNum(iClientNum));
    if (pPlayer == NULL)
        return -1;

    int iSlot(pPlayer->GiveItem(INVENTORY_AUTO_BACKPACK, Persistant_Item));

    if (iSlot != -1)
    {
        if (pPlayer->GetItem(iSlot)->IsPersistant())
        {
            pPlayer->GetItem(iSlot)->GetAsPersistant()->SetItemData(unItemData);
            pPlayer->GetItem(iSlot)->GetAsPersistant()->SetItemID(-1);
        }
        else
            pPlayer->RemoveItem(iSlot);
    }

    if (iSlot == -1)
    {
        Console << _T("Could not find an empty inventory space.") << newl;
        return -1;
    }

    return iSlot;
}


/*====================
  CGameServer::GiveItem
  ====================*/
int     CGameServer::GiveItem(int iClientNum, const tstring &sItem)
{
    IPlayerEntity *pPlayer(GetPlayerFromClientNum(iClientNum));
    if (pPlayer == NULL)
        return -1;

    return pPlayer->GiveItem(INVENTORY_AUTO_BACKPACK, EntityRegistry.LookupID(sItem));
}


/*====================
  CGameServer::PurchaseItem
  ====================*/
bool    CGameServer::PurchaseItem(int iClientNum, const tstring &sItem, bool bCheckPurchaseRules)
{
    try
    {
        IPlayerEntity *pPlayer(GetPlayerFromClientNum(iClientNum));
        if (pPlayer == NULL)
            EX_WARN(_T("Could not find entity for client: ") + XtoA(iClientNum));

        CEntityClientInfo *pClient(GetClientInfo(iClientNum));
        if (pClient == NULL)
            EX_WARN(_T("Could not find entity for client: ") + XtoA(iClientNum));

        ushort unItem(EntityRegistry.LookupID(sItem));
        Console << _T("Client #") << iClientNum << _T(" requested purchase of: ") << EntityRegistry.LookupName(unItem) << newl;
        if (unItem == INVALID_ENT_TYPE)
            return false;

        ushort unCost(0);
        ICvar *pCost(EntityRegistry.GetGameSetting(unItem, _T("Cost")));
        if (pCost == NULL)
            Console.Warn << _T("Could not retrieve item cost") << newl;
        else
            unCost = pCost->GetInteger();

        if (bCheckPurchaseRules)
        {
            // Check current status
            if (pPlayer->GetStatus() != ENTITY_STATUS_DORMANT)
            {
                Console << _T("Player is not allowed to purchase items from this status") << newl;
                return false;
            }

            // Check current unit
            if (!pPlayer->GetCanPurchase())
            {
                Console << _T("Player is not allowed to purchase items with this class") << newl;
                return false;
            }

            // Check prerequisites
            if (!sv_allItemsAvailable)
            {
                ICvar *pPrerequisite(EntityRegistry.GetGameSetting(unItem, _T("Prerequisite")));
                if (pPrerequisite)
                {
                    const tstring &sPrerequisite(pPrerequisite->GetString());

                    if (!sPrerequisite.empty())
                    {
                        CEntityTeamInfo *pTeamInfo(Game.GetTeam(pPlayer->GetTeam()));
                        if (pTeamInfo == NULL)
                            EX_ERROR(_T("Invalid team: ") + XtoA(pPlayer->GetTeam()));

                        if (!pTeamInfo->HasBuilding(sPrerequisite))
                        {
                            Console << _T("Item ") << sItem << _T(" requires a ") << sPrerequisite << newl;
                            return false;
                        }
                    }
                }
            }

            if (!pClient->SpendGold(unCost))
            {
                Console << _T("Item ") << sItem << _T(" costs ") << unCost << _T(" gold, the player has ") << pClient->GetGold() << _T(" gold, item cannot be purchased.") << newl;
                return false;
            }
        }

        bool bSuccess(false);
        bool bCanBuy(true);
        uint uNumStacks(0);
        int iSlot(-1);
        tstring sUniqueCategory;
        ICvar *pUniqueCategory(EntityRegistry.GetGameSetting(unItem, _T("UniqueCategory")));
        
        if (pUniqueCategory != NULL)
            sUniqueCategory = pUniqueCategory->GetString();

        for (int i = INVENTORY_START_BACKPACK; i < INVENTORY_END_BACKPACK && !bSuccess; i++)
        {
            if (pPlayer->GetItem(i) == NULL && iSlot == -1)
            {
                iSlot = i;
            }
            else if (pPlayer->GetItem(i) != NULL && pPlayer->GetItem(i)->GetAsConsumable() != NULL)
            {
                if (!sUniqueCategory.empty() && pPlayer->GetItem(i)->GetType() != unItem && CompareNoCase(pPlayer->GetItem(i)->GetAsConsumable()->GetUniqueCategory(), sUniqueCategory) == 0)
                {
                    Console << _T("Player already has an item of this category") << newl;
                    bSuccess = false;
                    bCanBuy = false;
                    break;
                }
                else if (pPlayer->GetItem(i)->GetType() == unItem)
                {
                    if (pPlayer->GetAmmoCount(i) < int(pPlayer->GetItem(i)->GetAsConsumable()->GetMaxPerStack()))
                    {
                        Console << _T("Item ") << sItem << _T(" purchased, placed in slot ") << i << newl;
                        pPlayer->SetAmmo(i, pPlayer->GetAmmoCount(i) + 1);
                        iSlot = i;
                        bSuccess = true;
                        break;
                    }
                    else
                    {
                        uNumStacks++;
                    }
                }
            }
        }

        ICvar *pMaxStacks(EntityRegistry.GetGameSetting(unItem, _T("MaxStacks")));
        if (bCanBuy && !bSuccess && (!pMaxStacks || uNumStacks < pMaxStacks->GetUnsignedInteger()) && iSlot != -1)
        {
            // Create a new stack of this item type
            pPlayer->GiveItem(iSlot, unItem);
            pPlayer->SetAmmo(iSlot, 1);
            Console << _T("Item ") << sItem << _T(" purchased, placed in slot ") << iSlot << newl;
            bSuccess = true;
        }
        else if (!bSuccess || !bCanBuy)
        {
            // No room, refund their gold
            if (!bSuccess && bCanBuy)
                Console << _T("Item ") << sItem << _T(" could not be purchased, no inventory space.") << newl;

            if (bCheckPurchaseRules)
                pPlayer->GiveGold(unCost);
        }

        if (bSuccess && bCanBuy)
        {
            TriggerManager.RegisterTriggerParam(_T("slot"), XtoA(iSlot + 1));
            TriggerManager.RegisterTriggerParam(_T("itemname"), pPlayer->GetItem(iSlot)->GetName());
            TriggerManager.RegisterTriggerParam(_T("name"), pPlayer->GetClientName());
            TriggerManager.RegisterTriggerParam(_T("clientid"), XtoA(pPlayer->GetClientID()));
            TriggerManager.RegisterTriggerParam(_T("index"), XtoA(pPlayer->GetIndex()));
            TriggerManager.TriggerGlobalScript(_T("buyitem"));
        }

        return (bSuccess && bCanBuy);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CGameServer::ProcessGameData(GAME_CMD_PURCHASE) - "), NO_THROW);
        return false;
    }
}


/*====================
  CGameServer::ChangeTeam
  ====================*/
void    CGameServer::ChangeTeam(int iClientID, int iTeam)
{
    Console << _T("Client #") << iClientID << _T(" requested to join team: ") << iTeam << newl;

    CEntityClientInfo *pClient(GetClientInfo(iClientID));
    if (pClient == NULL)
    {
        Console.Warn << _T("No entity found for client #") << iClientID << newl;
        return;
    }

    if (pClient->IsDemoAccount() && pClient->GetDemoTimeRemaining() < GetCurrentGameLength() && iTeam != 0)
        return;
    
    if (pClient->GetTeam() == iTeam || iTeam < 0 || iTeam >= Game.GetNumTeams())
        return;

    int iOldTeam(pClient->GetTeam());

    CEntityTeamInfo *pOldTeam(GetTeam(iOldTeam));
    CEntityTeamInfo *pNewTeam(GetTeam(iTeam));

    if (pNewTeam == NULL || iTeam == iOldTeam)
        return;

    if (pNewTeam->GetNumClients() >= (m_pHostServer->GetMaxPlayers() + 1) / 2)
        return;

    int iNewSF(pNewTeam->GetAverageSF());

    if (iTeam != 0)
    {
        for (int i(1); i < Game.GetNumTeams(); ++i)
        {
            CEntityTeamInfo *pTeam(Game.GetTeam(i));
            int iTeamCount;
            int iSF;

            if (pTeam == NULL || i == iTeam)
                continue;

            if (i == iOldTeam)
            {
                iTeamCount = pTeam->GetNumClients() - 1;
                iSF = ((pTeam->GetAverageSF() * pTeam->GetNumClients()) - pClient->GetPersistantStat(PLAYER_PERSISTANT_SKILLFACTOR)) * (pTeam->GetNumClients() - 1);
            }
            else
            {
                iTeamCount = pTeam->GetNumClients();
                iSF = pTeam->GetAverageSF();
            }

            if (iTeamCount < (pNewTeam->GetNumClients() + 1) - sv_maxTeamDifference.GetInteger())
                return;

            if (iTeamCount == pNewTeam->GetNumClients() && iSF < iNewSF)
                return;
        }
    }

    RemoveVoiceClient(iClientID);

    if (pOldTeam != NULL)
        pOldTeam->RemoveClient(iClientID);

    pNewTeam->AddClient(iClientID);

    IPlayerEntity *pNewPlayer;
    pNewPlayer = ChangeUnit(iClientID, Player_Observer, CHANGE_UNIT_KILL | CHANGE_UNIT_REFUND_GOLD);

    if (pNewPlayer != NULL)
        pNewPlayer->SetStatus(ENTITY_STATUS_DORMANT);

    TriggerManager.RegisterTriggerParam(_T("oldteam"), XtoA(iOldTeam));
    TriggerManager.RegisterTriggerParam(_T("newteam"), XtoA(iTeam));
    TriggerManager.RegisterTriggerParam(_T("name"), pClient->GetName());
    TriggerManager.RegisterTriggerParam(_T("clientid"), XtoA(pClient->GetClientNumber()));
    TriggerManager.RegisterTriggerParam(_T("index"), XtoA(pClient->GetPlayerEntityIndex()));
    TriggerManager.TriggerGlobalScript(_T("changeteam"));
}


/*====================
  CGameServer::ChangeUnit
  ====================*/
IPlayerEntity*  CGameServer::ChangeUnit(int iClientNum, ushort unNewUnitID, int iFlags)
{
    try
    {
        Console << _T("Client #") << iClientNum << _T(" requested change to: ") << EntityRegistry.LookupName(unNewUnitID) << newl;
        CEntityClientInfo *pClient(GetClientInfo(iClientNum));
        if (pClient == NULL)
            EX_ERROR(_T("Invalid client"));

        if (unNewUnitID == INVALID_ENT_TYPE)
            EX_ERROR(_T("Invalid entity type"));

        if (sv_unitH4x || Game.GetGamePhase() == GAME_PHASE_WARMUP)
            iFlags &= ~CHANGE_UNIT_CHECK_RULES;
        if (sv_allUnitsAvailable)
            iFlags &= ~CHANGE_UNIT_CHECK_PREREQUISITES;

        IPlayerEntity *pOldPlayer(pClient->GetPlayerEntity());

        if (pOldPlayer != NULL && unNewUnitID == pOldPlayer->GetType())
            return pOldPlayer;

        IPlayerEntity *pNewPlayer(NULL);
        if (pClient->ChangeUnit(unNewUnitID, iFlags))
            pNewPlayer = pClient->GetPlayerEntity();
        else
            return pOldPlayer;

        TriggerManager.RegisterTriggerParam(_T("oldunitname"), (pOldPlayer == NULL) ? SNULL : pOldPlayer->GetEntityName());
        TriggerManager.RegisterTriggerParam(_T("newunitname"), (pNewPlayer == NULL) ? SNULL : pNewPlayer->GetEntityName());
        TriggerManager.RegisterTriggerParam(_T("oldunittype"), (pOldPlayer == NULL) ? SNULL : pOldPlayer->GetTypeName());
        TriggerManager.RegisterTriggerParam(_T("newunittype"), (pNewPlayer == NULL) ? SNULL : pNewPlayer->GetTypeName());
        TriggerManager.RegisterTriggerParam(_T("name"), pClient->GetName());
        TriggerManager.RegisterTriggerParam(_T("clientid"), XtoA(pClient->GetClientNumber()));
        TriggerManager.RegisterTriggerParam(_T("index"), XtoA((pNewPlayer == NULL) ? INVALID_INDEX : pNewPlayer->GetIndex()));
        TriggerManager.RegisterTriggerParam(_T("oldindex"), XtoA((pOldPlayer == NULL) ? INVALID_INDEX : pOldPlayer->GetIndex()));
        TriggerManager.TriggerGlobalScript(_T("changeunit"));

        // Kill or delete old entity, if there is one
        if (pOldPlayer != NULL)
        {
            pOldPlayer->CancelBuild();

            if (iFlags & CHANGE_UNIT_KILL)
            {
                pOldPlayer->SetLocalFlags(ENT_LOCAL_FLAG_CHANGING_UNIT);
                pOldPlayer->Kill(NULL);
                pOldPlayer->SetNetFlags(ENT_NET_FLAG_NO_RESURRECT);
                pOldPlayer->RemoveLocalFlags(ENT_LOCAL_FLAG_CHANGING_UNIT);
            }
            else
            {
                if (!(iFlags & CHANGE_UNIT_NO_DELETE))
                    DeleteEntity(pOldPlayer->GetIndex());
            }
        }

        return pNewPlayer;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CGameServer::ChangeUnit() - "), NO_THROW);
        return NULL;
    }
}


/*====================
  CGameServer::CancelSacrifice
  ====================*/
void    CGameServer::CancelSacrifice(int iClientNum)
{
    IPlayerEntity *pPlayer(GetPlayerFromClientNum(iClientNum));

    if (pPlayer != NULL && pPlayer->HasNetFlags(ENT_NET_FLAG_SACRIFICE_MENU))
        pPlayer->RemoveNetFlags(ENT_NET_FLAG_SACRIFICE_MENU);
}


/*====================
  CGameServer::SacrificeUnit
  ====================*/
void    CGameServer::SacrificeUnit(int iClientNum, ushort unNewUnitID)
{
    try
    {
        Console << _T("Client #") << iClientNum << _T(" requested sacrifice to: ") << EntityRegistry.LookupName(unNewUnitID) << newl;

        CEntityClientInfo *pClient(GetClientInfo(iClientNum));
        if (pClient == NULL)
            EX_ERROR(_T("Invalid client"));

        if (pClient->IsDemoAccount())
            EX_ERROR(_T("Player is using a demo account"));

        IPlayerEntity *pOldPlayer(GetPlayerFromClientNum(iClientNum));
        if (pOldPlayer == NULL)
            EX_WARN(_T("Old entity is not a player type"));

        if (!pOldPlayer->HasNetFlags(ENT_NET_FLAG_SACRIFICE_MENU))
            EX_WARN(_T("Player was not at a sacrificial shrine"));

        if (pOldPlayer->GetIsHellbourne())
            EX_WARN(_T("Player is already a hellbourne"));

        IGameEntity* pNewEnt(m_pServerEntityDirectory->Allocate(unNewUnitID));
        if (pNewEnt == NULL)
            EX_ERROR(_T("Failed to allocate a new entity"));

        IPlayerEntity *pNewPlayer(pNewEnt->GetAsPlayerEnt());
        if (pNewPlayer == NULL)
        {
            m_pServerEntityDirectory->Delete(pNewEnt->GetIndex());
            EX_WARN(_T("New entity is not a player type"));
        }

        if (!pNewPlayer->GetAsPlayerEnt()->GetIsHellbourne())
        {
            m_pServerEntityDirectory->Delete(pNewEnt->GetIndex());
            EX_WARN(_T("New entity is not a Hellbourne unit"));
        }

        if (pClient->SpendSouls(pNewPlayer->GetSoulCost()))
        {
            pOldPlayer->SetAction(PLAYER_ACTION_SACRIFICING | PLAYER_ACTION_IMMOBILE, GetGameTime() + 200);
            pOldPlayer->StartAnimation(_T("sacrificed"), -1);
            pOldPlayer->SetNextUnit(unNewUnitID);
            pOldPlayer->RemoveNetFlags(ENT_NET_FLAG_SACRIFICED | ENT_NET_FLAG_SACRIFICE_MENU);
        }

        m_pServerEntityDirectory->Delete(pNewPlayer->GetIndex());
    }
    catch (CException &ex)
    {
        ex.Process(_T("CGameServer::SacrificeUnit() - "), NO_THROW);
    }
}


/*====================
  CGameServer::StartBuilding
  ====================*/
void    CGameServer::StartBuilding(int iClientNum, CPacket &pkt)
{
    ushort unType(pkt.ReadShort());

    IPlayerEntity *pPlayer(GetPlayerEntityFromClientID(iClientNum));
    if (pPlayer == NULL || !pPlayer->GetCanBuild())
        return;

    // Delete any existing preview building
    if (pPlayer->GetPreviewBuildingIndex() != INVALID_INDEX)
    {
        IBuildingEntity *pPreview(Game.GetBuildingEntity(pPlayer->GetPreviewBuildingIndex()));
        if (pPreview->GetType() == unType)
            return;
        else
            DeleteEntity(pPlayer->GetPreviewBuildingIndex());
    }

    // Allocate a new building preview
    IGameEntity *pNewEntity(AllocateEntity(unType));
    IBuildingEntity *pNewBuilding(pNewEntity == NULL ? NULL : pNewEntity->GetAsBuilding());
    if (pNewBuilding == NULL)
    {
        DeleteEntity(pNewBuilding);
        return;
    }

    pPlayer->SetPreviewBuildingIndex(pNewBuilding->GetIndex());

    pNewBuilding->SpawnPreview();
    pNewBuilding->SetPosition(pPlayer->GetTargetPosition(FAR_AWAY));
    pNewBuilding->SetTeam(pPlayer->GetTeam());
}


/*====================
  CGameServer::PlaceBuilding
  ====================*/
void    CGameServer::PlaceBuilding(int iClientNum, CPacket &pkt)
{
    IGameEntity *pNewEnt(NULL);

    try
    {
        ushort unType(pkt.ReadShort(-1));
        CVec3f v3Pos(pkt.ReadV3f());
        CVec3f v3Angles(pkt.ReadV3f());
        uint uiFoundation(pkt.ReadInt(INVALID_INDEX));

        if (pkt.HasFaults())
            EX_ERROR(_T("Bad data from packet"));

        // Get entity for player that sent the request
        CEntityClientInfo *pClient(GameServer.GetClientInfo(iClientNum));
        if (pClient == NULL)
            EX_WARN(_T("No entity found for client"));

        // Get players team
        CEntityTeamInfo *pTeamInfo(GetTeam(pClient->GetTeam()));
        if (pTeamInfo == NULL)
            EX_WARN(_T("Requesting client has an invalid team: ") + XtoA(pClient->GetTeam()));

        // Create the building
        pNewEnt = Game.AllocateEntity(unType);
        if (pNewEnt == NULL)
            EX_WARN(_T("Failed to spawn building type: ") + SHORT_HEX_STR(unType));

        IBuildingEntity *pNewBuilding(pNewEnt->GetAsBuilding());
        if (pNewBuilding == NULL)
            EX_WARN(_T("Entity is not a building: ") + SHORT_HEX_STR(unType));

        pNewBuilding->SetTeam(pTeamInfo->GetTeamID());
        pNewBuilding->SetSquad(pClient->GetSquad());

        IPropFoundation *pFoundation(NULL);
        if (uiFoundation != INVALID_INDEX)
        {
            IPropEntity *pProp(GetPropEntity(uiFoundation));
            if (pProp == NULL)
                EX_WARN(_T("NULL foundation"));
            pFoundation = pProp->GetAsFoundation();
            if (pFoundation == NULL)
                EX_WARN(_T("NULL foundation"));
            
            if (pFoundation->GetStatus() != ENTITY_STATUS_ACTIVE)
                EX_WARN(_T("Non-active foundation"));
            if (!pFoundation->CanSupportBuilding())
                EX_WARN(_T("Foundation can't support a building"));
            
            pNewBuilding->SetPosition(pFoundation->GetPosition());
            pNewBuilding->SetAngles(pFoundation->GetAngles());
            pNewBuilding->SetFoundationScale(pFoundation->GetScale());
            pNewBuilding->SetFoundation(uiFoundation);
        }
        else
        {
            pNewBuilding->SetPosition(v3Pos);
            pNewBuilding->SetAngles(v3Angles);
        }

        pNewBuilding->SetModelHandle(Game.RegisterModel(pNewBuilding->GetModelPath()));
        pNewBuilding->SetScale(pNewBuilding->GetBuildingScale());

        // Validate placement
        tstring sTemp;
        if (!pNewBuilding->CanBuild(pClient->GetPlayerEntity(), sTemp) ||
            !pTeamInfo->SpendGold(pNewBuilding->GetCost()))
            EX_WARN(_T("Can not build"));
    
        Console << _T("Place building: ") << EntityRegistry.LookupName(unType) << _T(" at ") << v3Pos << newl;

        // Add this building to "buildings built" if the player is a commander
        MatchStatEvent(pClient->GetClientNumber(), COMMANDER_MATCH_BUILDINGS, 1, -1, INVALID_ENT_TYPE, unType);

        if (pFoundation != NULL)
        {
            pFoundation->SetStatus(ENTITY_STATUS_HIDDEN);
            pFoundation->SetBuildingIndex(pNewBuilding->GetIndex());
            pFoundation->Unlink();
            pFoundation->Link();
        }

        pNewEnt->Spawn();

        StopBuilding(iClientNum);
    }
    catch (CException &ex)
    {
        if (pNewEnt != NULL)
            m_pServerEntityDirectory->Delete(pNewEnt->GetIndex());

        ex.Process(_T("CGameServer::ProcessGameData(GAME_CMD_PLACE_BUILDING) - "), NO_THROW);
    }
}


/*====================
  CGameServer::StopBuilding
  ====================*/
void    CGameServer::StopBuilding(int iClientNum)
{
    // Get entity for player that sent the request
    IPlayerEntity *pPlayer(GameServer.GetPlayerFromClientNum(iClientNum));
    if (pPlayer == NULL)
        return;

    DeleteEntity(pPlayer->GetPreviewBuildingIndex());
    pPlayer->SetPreviewBuildingIndex(INVALID_INDEX);
    pPlayer->RemoveNetFlags(ENT_NET_FLAG_BUILD_MODE);
}


/*====================
  CGameServer::ProcessGameData
  ====================*/
bool    CGameServer::ProcessGameData(int iClientNum, CPacket &pkt)
{
    // OMG spam...
    // Console << _T("Message from client #") << iClientNum << _T("...") << newl;

    CEntityClientInfo *pClient(GetClientInfo(iClientNum));
    if (pClient != NULL)
        pClient->SetLastInputTime(Game.GetGameTime());

    byte yCmd(pkt.ReadByte());
    switch (yCmd)
    {
    case GAME_CMD_CHANGE_UNIT:
        ChangeUnit(iClientNum, pkt.ReadShort(-1), CHANGE_UNIT_CHECK_RULES | CHANGE_UNIT_INHERIT_HP | CHANGE_UNIT_INHERIT_DAMAGE_RECORD | CHANGE_UNIT_REFUND_GOLD | CHANGE_UNIT_TRANSFER_ITEMS);
        return !pkt.HasFaults();

    case GAME_CMD_SACRIFICE:
        SacrificeUnit(iClientNum, pkt.ReadShort(-1));
        return !pkt.HasFaults();

    case GAME_CMD_CANCEL_SACRIFICE:
        CancelSacrifice(iClientNum);
        return !pkt.HasFaults();

    case GAME_CMD_CANCEL_SPAWN:
        {
            IPlayerEntity *pPlayer(GetPlayerFromClientNum(iClientNum));

            if (pPlayer == NULL)
                EX_WARN(_T("No entity found for player"));

            if (pPlayer->GetStatus() == ENTITY_STATUS_SPAWNING)
                pPlayer->SetStatus(ENTITY_STATUS_DORMANT);
        }
        return !pkt.HasFaults();

    case GAME_CMD_CHANGE_TEAM:
        ChangeTeam(iClientNum, pkt.ReadShort(-1));
        return !pkt.HasFaults();

    case GAME_CMD_SPEND_POINT:
        {
            int iAttrib(pkt.ReadInt(0));
            CEntityClientInfo *pClient(GetClientInfo(iClientNum));
            if (pClient == NULL)
                EX_WARN(_T("No entity found for client"));
            pClient->SpendPoint(iAttrib);
            return !pkt.HasFaults();
        }

    case GAME_CMD_SPEND_TEAM_POINT:
        {
            IPlayerEntity *pPlayer(GetPlayerEntityFromClientID(iClientNum));
            if (pPlayer == NULL)
                EX_WARN(_T("No entity found for client"));
            CEntityTeamInfo *pTeam(GetTeam(pPlayer->GetTeam()));
            if (pTeam == NULL)
                EX_WARN(_T("Invalid team"));
            pTeam->SpendPoint(pkt.ReadInt(0));
            return !pkt.HasFaults();
        }

    case GAME_CMD_SPAWN_WORKER:
        {
            CEntityClientInfo *pClient(GetClientInfo(iClientNum));
            if (pClient == NULL)
                break;
            CEntityTeamInfo *pTeam(GetTeam(pClient->GetTeam()));
            if (pTeam == NULL)
                break;
            pTeam->SpawnWorker(iClientNum);
        }
        break;

    case GAME_CMD_CHAT_ALL:
        try
        {
            CBufferDynamic buffer;
            tstring sMsg(CensorChat(pkt.ReadString()));

            CEntityClientInfo *pClient(GameServer.GetClientInfo(iClientNum));
            if (pClient == NULL)
                EX_ERROR(_T("Invalid client ID"));

            buffer << GAME_CMD_CHAT_ALL << iClientNum << sMsg << byte(0);
            BroadcastGameData(buffer, true);
            Console.Server << _T("[ALL] ") << pClient->GetName() << _T(": ") << sMsg << newl;
        }
        catch (CException &ex)
        {
            ex.Process(_T("CGameServer::ProcessGameData(GAME_CMD_CHAT_ALL) - "), NO_THROW);
        }
        break;

    case GAME_CMD_CHAT_TEAM:
        try
        {
            CBufferDynamic buffer;
            tstring sMsg(CensorChat(pkt.ReadString()));

            CEntityClientInfo *pClient(GameServer.GetClientInfo(iClientNum));
            if (pClient == NULL)
                EX_ERROR(_T("Invalid client ID"));

            buffer << GAME_CMD_CHAT_TEAM << iClientNum << sMsg << byte(0);
            Console.Server << _T("[TEAM ") << pClient->GetTeam() << _T("] ") << pClient->GetName() << _T(": ") << sMsg << newl;

            CEntityTeamInfo *pTeam(GameServer.GetTeam(pClient->GetTeam()));
            if (pTeam != NULL)
            {
                ivector viClients(pTeam->GetClientList());

                for (ivector::iterator it(viClients.begin()); it != viClients.end(); it++)
                    SendGameData((*it), buffer, true);
            }
        }
        catch (CException &ex)
        {
            ex.Process(_T("CGameServer::ProcessGameData(GAME_CMD_CHAT_TEAM) - "), NO_THROW);
        }
        break;

    case GAME_CMD_CHAT_SQUAD:
        try
        {
            CBufferDynamic buffer;
            tstring sMsg(CensorChat(pkt.ReadString()));

            CEntityClientInfo *pClient(GameServer.GetClientInfo(iClientNum));
            if (pClient == NULL)
                EX_ERROR(_T("Invalid client ID"));

            buffer << GAME_CMD_CHAT_SQUAD << iClientNum << sMsg << byte(0);
            Console.Server << _T("[SQUAD] ") << pClient->GetName() << _T(": ") << sMsg << newl;

            byte ySquad(pClient->GetSquad());

            CEntityTeamInfo *pTeam(GameServer.GetTeam(pClient->GetTeam()));
            if (pTeam != NULL)
            {
                ivector viClients(pTeam->GetClientList());

                for (ivector::iterator it(viClients.begin()); it != viClients.end(); ++it)
                {
                    if (GameServer.GetPlayerEntityFromClientID((*it))->GetSquad() == ySquad)
                        SendGameData((*it), buffer, true);
                }
            }
        }
        catch (CException &ex)
        {
            ex.Process(_T("CGameServer::ProcessGameData(GAME_CMD_CHAT_SQUAD) - "), NO_THROW);
        }
        break;

    case GAME_CMD_REMOTE:
        try
        {
            tstring sPass(pkt.ReadString());
            tstring sCmd(pkt.ReadString());

            if (svr_remotePass != _T("") && sPass == svr_remotePass)
            {
                CEntityClientInfo *pInfo(GetClientInfo(iClientNum));
                if (pInfo)
                    Console << _T("Remote<") << pInfo->GetName() << _T(">: ") << sCmd << newl;
                else
                    Console << _T("Remote<") << iClientNum << _T(">: ") << sCmd << newl;

                Console.StartWatch();

                Console.Execute(sCmd);

                const tstring &sWatch(Console.GetWatchBuffer());
                if (!sWatch.empty())
                {
                    CBufferDynamic buffer;

                    buffer << GAME_CMD_CONSOLE_MESSAGE << sWatch << byte(0);

                    SendGameData(iClientNum, buffer, true);
                }

                Console.EndWatch();
            }
        }
        catch (CException &ex)
        {
            ex.Process(_T("CGameServer::ProcessGameData(GAME_CMD_REMOTE) - "), NO_THROW);
        }
        break;

    case GAME_CMD_START_BUILDING:
        StartBuilding(iClientNum, pkt);
        return !pkt.HasFaults();

    case GAME_CMD_PLACE_BUILDING:
        PlaceBuilding(iClientNum, pkt);
        return !pkt.HasFaults();

    case GAME_CMD_STOP_BUILDING:
        StopBuilding(iClientNum);
        return !pkt.HasFaults();

    case GAME_CMD_POS_COMMAND:
        try
        {
            ECommanderOrder eOrder(ECommanderOrder(pkt.ReadByte()));
            CVec3f v3Position(pkt.ReadV3f());
            bool bQueued(pkt.ReadByte() != 0);

            IPlayerEntity *pPlayer(GetPlayerFromClientNum(iClientNum));
            if (pPlayer == NULL)
                EX_WARN(_T("No entity found for client"));

            CPlayerCommander *pCommander(pPlayer->GetAsCommander());
            if (pCommander == NULL)
                EX_WARN(_T("Not a commander"));

            const uiset &setSelection(pCommander->GetSelection());
            for (uiset::const_iterator it(setSelection.begin()); it != setSelection.end(); ++it)
            {
                IVisualEntity *pEnt(GetVisualEntity(*it));

                if (!pEnt || pEnt->GetTeam() != pCommander->GetTeam())
                    continue;

                if (pEnt->IsPlayer() && pEnt->GetTeam() == pCommander->GetTeam())
                {
                    IPlayerEntity *pPlayer(pEnt->GetAsPlayerEnt());

                    if (!bQueued)
                        pPlayer->ClearOrders();

                    pPlayer->AddOrder(eOrder, INVALID_INDEX, v3Position);
                }
                else if (pEnt->IsNpc() && sv_commanderNpcControl)
                {
                    INpcEntity *pNpc(pEnt->GetAsNpc());

                    pNpc->PlayerCommand(NPCCMD_MOVE, INVALID_INDEX, v3Position);
                }
                else if (pEnt->IsPet() && pEnt->GetAsPet()->GetOwnerUID() == pCommander->GetUniqueID())
                {
                    IPetEntity *pPet(pEnt->GetAsPet());

                    pPet->PlayerCommand(PETCMD_MOVE, INVALID_INDEX, v3Position);
                }
            }

            if (!setSelection.empty())
                MatchStatEvent(pCommander->GetClientID(), COMMANDER_MATCH_ORDERS, 1);
        }
        catch (CException &ex)
        {
            ex.Process(_T("CGameServer::ProcessGameData(GAME_CMD_POS_COMMAND) - "), NO_THROW);
        }
        break;

    case GAME_CMD_ENT_COMMAND:
        try
        {
            ECommanderOrder eOrder(ECommanderOrder(pkt.ReadByte()));
            uint uiEntIndex(pkt.ReadInt());
            bool bQueued(pkt.ReadByte() != 0);

            IPlayerEntity *pPlayer(GetPlayerFromClientNum(iClientNum));
            if (pPlayer == NULL)
                EX_WARN(_T("No entity found for client"));

            CPlayerCommander *pCommander(pPlayer->GetAsCommander());
            if (pCommander == NULL)
                EX_WARN(_T("Not a commander"));

            const uiset &setSelection(pCommander->GetSelection());
            for (uiset::const_iterator it(setSelection.begin()); it != setSelection.end(); ++it)
            {
                IVisualEntity *pEnt(GetVisualEntity(*it));
                if (!pEnt || pEnt->GetTeam() != pCommander->GetTeam())
                    continue;

                if (pEnt->IsPlayer())
                {
                    IPlayerEntity *pPlayer(pEnt->GetAsPlayerEnt());

                    if (!bQueued)
                        pPlayer->ClearOrders();

                    pPlayer->AddOrder(eOrder, uiEntIndex, V3_ZERO);
                }
                else if (pEnt->IsNpc() && sv_commanderNpcControl)
                {
                    IVisualEntity *pTarget(GetVisualEntity(uiEntIndex));
                    if (pTarget == NULL)
                        break;

                    if (pEnt->IsEnemy(pTarget))
                        pEnt->GetAsNpc()->PlayerCommand(NPCCMD_ATTACK, uiEntIndex, V3_ZERO);
                    else if (pTarget->IsBuilding() && (pTarget->GetStatus() == ENTITY_STATUS_SPAWNING || pTarget->GetHealthPercent() < 1.0f))
                        pEnt->GetAsNpc()->PlayerCommand(NPCCMD_REPAIR, uiEntIndex, V3_ZERO);
                    else
                        pEnt->GetAsNpc()->PlayerCommand(NPCCMD_FOLLOW, uiEntIndex, V3_ZERO);
                }
                else if (pEnt->IsPet() && pEnt->GetAsPet()->GetOwnerUID() == pCommander->GetUniqueID())
                {
                    IVisualEntity *pTarget(GetVisualEntity(uiEntIndex));
                    if (pTarget == NULL)
                        break;

                    if (pEnt->IsEnemy(pTarget))
                        pEnt->GetAsPet()->PlayerCommand(PETCMD_ATTACK, uiEntIndex, V3_ZERO);
                    else if (pTarget->IsBuilding() && (pTarget->GetStatus() == ENTITY_STATUS_SPAWNING || pTarget->GetHealthPercent() < 1.0f))
                        pEnt->GetAsPet()->PlayerCommand(PETCMD_REPAIR, uiEntIndex, V3_ZERO);
                    else
                        pEnt->GetAsPet()->PlayerCommand(PETCMD_FOLLOW, uiEntIndex, V3_ZERO);
                }
            }

            if (!setSelection.empty())
                MatchStatEvent(pCommander->GetClientID(), COMMANDER_MATCH_ORDERS, 1);
        }
        catch (CException &ex)
        {
            ex.Process(_T("CGameServer::ProcessGameData(GAME_CMD_POS_COMMAND) - "), NO_THROW);
        }
        break;

    case GAME_CMD_SPAWN:
        {
            try
            {
                CEntityClientInfo *pClient(GetClientInfo(iClientNum));
                if (pClient == NULL)
                    EX_WARN(_T("Client entity not found"));

                if (pClient->HasNetFlags(ENT_NET_FLAG_QUEUED))
                    EX_WARN(_T("Client is currently queued"));

                if (pClient->HasFlags(CLIENT_INFO_IS_COMMANDER))
                    ChangeUnit(iClientNum, Player_Commander, CHANGE_UNIT_CHECK_RULES | CHANGE_UNIT_REFUND_GOLD);

                if (pClient->GetTeam() < 1 && (pClient->GetPlayerEntity() == NULL || !pClient->GetPlayerEntity()->IsObserver()))
                    EX_WARN(_T("Client has not joined a team"));

                IPlayerEntity *pPlayer(pClient->GetPlayerEntity());

                if (pPlayer == NULL || (pPlayer->IsObserver() && pClient->GetTeam() != 0))
                    EX_WARN(_T("Client does not have a valid player entity"));
                if (pPlayer->GetStatus() != ENTITY_STATUS_DORMANT && pPlayer->GetStatus() != ENTITY_STATUS_SPAWNING)
                    EX_WARN(_T("Client attempting to spawn from a non-dormant state"));

                pPlayer->Spawn2();
            }
            catch (CException &ex)
            {
                ex.Process(_T("CGameServer::ProcessGameData(GAME_CMD_SPAWN) - "), NO_THROW);
            }
        }
        break;

    case GAME_CMD_SPAWN_SELECT:
        {
            try
            {
                uint uiSpawnLocation(pkt.ReadInt());

                IPlayerEntity *pPlayer(GetPlayerFromClientNum(iClientNum));
                if (pPlayer == NULL)
                    EX_WARN(_T("Entity is not a player type"));
                if (pPlayer->GetStatus() != ENTITY_STATUS_DORMANT && pPlayer->GetStatus() != ENTITY_STATUS_SPAWNING && !sv_unitH4x)
                    EX_WARN(_T("Player attempting to respawn from non-dormant state"));

                if (pPlayer->SetSpawnLocation(uiSpawnLocation))
                    pPlayer->Spawn3();
            }
            catch (CException &ex)
            {
                ex.Process(_T("CGameServer::ProcessGameData(GAME_CMD_SPAWN) - "), NO_THROW);
            }
        }
        break;

    case GAME_CMD_PURCHASE:
        {
            ushort unItem(pkt.ReadShort());
            PurchaseItem(iClientNum, EntityRegistry.LookupName(unItem), !sv_itemH4x);
            return true;
        }
        break;

    case GAME_CMD_PURCHASE_PERSISTANT:
        {
            int iVaultNum(pkt.ReadInt());
                        
            PurchasePersistantItem(iClientNum, iVaultNum);
            return true;
        }
        break;

    case GAME_CMD_SELL:
        {
            int iSlot(pkt.ReadInt());
            int iAmount(pkt.ReadInt());

            IPlayerEntity *pPlayer(GetPlayerFromClientNum(iClientNum));
            if (pPlayer == NULL)
                EX_WARN(_T("Old entity is not a player type"));

            SellItem(iSlot, pPlayer, iAmount);
            return true;
        }
        break;
    
    case GAME_CMD_SET_UPKEEP:
        {
            uint uiIndex(pkt.ReadInt());
            float fLevel(pkt.ReadFloat());

            CEntityClientInfo *pClient(GetClientInfo(iClientNum));
            if (pClient == NULL)
                break;

            IGameEntity *pEntity(GetEntity(uiIndex));
            if (pEntity == NULL)
                break;
            IBuildingEntity *pBuilding(pEntity->GetAsBuilding());
            if (pBuilding == NULL)
                break;

            if (!pClient->HasFlags(CLIENT_INFO_IS_COMMANDER))
                break;
            if (pClient->GetTeam() != pBuilding->GetTeam())
                break;

            pBuilding->SetUpkeepLevel(fLevel);
            return true;
        }
        break;

    case GAME_CMD_PROMOTE_OFFICER:
        {
            int iTargetClientID(pkt.ReadInt());

            CEntityClientInfo *pClient(GetClientInfo(iClientNum));
            if (pClient == NULL)
                break;

            if (!pClient->HasFlags(CLIENT_INFO_IS_COMMANDER))
                break;

            CEntityTeamInfo *pTeam(Game.GetTeam(pClient->GetTeam()));
            if (pTeam != NULL)
                pTeam->PromotePlayer(iTargetClientID);
            return true;
        }
        break;

    case GAME_CMD_DEMOTE_OFFICER:
        {
            byte ySquad(pkt.ReadByte());

            CEntityClientInfo *pClient(GetClientInfo(iClientNum));
            if (pClient == NULL)
                break;

            if (!pClient->HasFlags(CLIENT_INFO_IS_COMMANDER))
                break;

            CEntityTeamInfo *pTeam(Game.GetTeam(pClient->GetTeam()));
            pTeam->DemotePlayer(ySquad);
            return true;
        }
        break;

    case GAME_CMD_VOTE:
        {
            int iVote(pkt.ReadInt());
            if (pkt.HasFaults())
                return false;

            if (GetGamePhase() != GAME_PHASE_SELECTING_COMMANDER)
                return true;

            CEntityClientInfo *pClient(GetClientInfo(iClientNum));
            if (pClient == NULL)
                return true;

            CEntityClientInfo *pTargetClient(GetClientInfo(iVote));
            if (pTargetClient == NULL)
                return true;

            if (pClient->GetTeam() != pTargetClient->GetTeam())
                return true;

            pClient->SetVote(iVote);
            return true;
        }
        return true;

    case GAME_CMD_REQUEST_COMMAND:
        {
            CEntityClientInfo *pClient(GetClientInfo(iClientNum));
            if (pClient == NULL)
                break;

            if (pClient->IsDemoAccount())
                break;

            CEntityTeamInfo *pTeam(GetTeam(pClient->GetTeam()));
            if (pTeam == NULL)
                break;

            if (GetGamePhase() == GAME_PHASE_ACTIVE)
            {
                if (pTeam->HasCommander())
                    return true;
                pTeam->SetCommander(iClientNum);
            }
            else if (GetGamePhase() < GAME_PHASE_ACTIVE)
            {
                if (pTeam->GetNumCandidates() < pTeam->GetMaxCandidates())
                {
                    pClient->SetFlags(CLIENT_INFO_WANTS_TO_COMMAND);
                    pClient->SetVote(iClientNum);
                }
                return true;
            }
        }
        break;

    case GAME_CMD_COMMANDER_RESIGN:
        {
            IPlayerEntity *pPlayer(GetPlayerFromClientNum(iClientNum));
            if (pPlayer == NULL)
                break;

            CEntityTeamInfo *pTeam(GetTeam(pPlayer->GetTeam()));
            if (pTeam == NULL)
                break;

            if (pTeam->IsCommander(pTeam->GetTeamIndexFromClientID(iClientNum)))
                pTeam->RemoveCommander();

            return true;
        }
        break;

    case GAME_CMD_DECLINE_OFFICER:
        {
            CEntityClientInfo *pClient(GetClientInfo(iClientNum));
            if (pClient == NULL)
                break;

            CEntityTeamInfo *pTeam(GetTeam(pClient->GetTeam()));
            if (pTeam == NULL)
                break;

            pTeam->DeclineOfficer(iClientNum);
            return true;
        }
        break;

    case GAME_CMD_JOIN_SQUAD:
        {
            byte ySquad(pkt.ReadByte());

            CEntityClientInfo *pClient(GetClientInfo(iClientNum));
            if (pClient == NULL)
                break;
            CEntityTeamInfo *pTeam(GetTeam(pClient->GetTeam()));
            if (pTeam == NULL)
                break;

            RemoveVoiceClient(iClientNum);
            
            pTeam->JoinSquad(iClientNum, ySquad);
            return true;
        }
        break;

    case GAME_CMD_CONTRIBUTE:
        {
            ushort unAmount(pkt.ReadShort());

            CEntityClientInfo *pClient(GetClientInfo(iClientNum));
            if (pClient == NULL)
                break;

            CEntityTeamInfo *pTeam(GetTeam(pClient->GetTeam()));
            if (pTeam == NULL)
                break;

            if (pClient->SpendGold(unAmount))
            {
                pTeam->GiveGold(unAmount);
                pClient->GiveExperience(unAmount * g_donationExpReward);
            }
        }
        break;

    case GAME_CMD_MINIMAP_DRAW:
        try
        {
            float fX(pkt.ReadFloat());
            float fY(pkt.ReadFloat());
            byte ySquad(pkt.ReadByte());
            
            CEntityClientInfo *pClient(GameServer.GetClientInfo(iClientNum));
            if (pClient == NULL)
                EX_ERROR(_T("Invalid client ID"));

            if (!pClient->HasFlags(CLIENT_INFO_IS_COMMANDER))
                break;

            CEntityTeamInfo *pTeam(GameServer.GetTeam(pClient->GetTeam()));
            if (pTeam == NULL)
                EX_ERROR(_T("Invalid Team"));

            CBufferFixed<10> buffer;
            buffer << GAME_CMD_MINIMAP_DRAW << fX << fY;

            const ivector &viClients(pTeam->GetClientList());

            if (ySquad == byte(-1))
            {
                for (ivector::const_iterator it(viClients.begin()); it != viClients.end(); ++it)
                    SendGameData((*it), buffer, false);
            }
            else
            {
                for (ivector::const_iterator it(viClients.begin()); it != viClients.end(); ++it)
                {
                    if (GameServer.GetPlayerEntityFromClientID((*it))->GetSquad() == ySquad)
                        SendGameData((*it), buffer, false);
                }
            }
        }
        catch (CException &ex)
        {
            ex.Process(_T("CGameServer::ProcessGameData(GAME_CMD_MINIMAP_DRAW) - "), NO_THROW);
        }
        break;

    case GAME_CMD_MINIMAP_PING:
        try
        {
            float fX(pkt.ReadFloat());
            float fY(pkt.ReadFloat());
            byte ySquad(pkt.ReadByte());
            
            CEntityClientInfo *pClient(GameServer.GetClientInfo(iClientNum));
            if (pClient == NULL)
                EX_ERROR(_T("Invalid client ID"));

            CBufferFixed<9> buffer;
            buffer << GAME_CMD_MINIMAP_PING << fX << fY;

            CEntityTeamInfo *pTeam(GameServer.GetTeam(pClient->GetTeam()));
            if (pTeam == NULL)
                EX_ERROR(_T("Invalid Team"));

            const ivector &viClients(pTeam->GetClientList());

            if (ySquad == byte(-1))
            {
                for (ivector::const_iterator it(viClients.begin()); it != viClients.end(); ++it)
                    SendGameData((*it), buffer, false);
            }
            else
            {
                for (ivector::const_iterator it(viClients.begin()); it != viClients.end(); ++it)
                {
                    if (GameServer.GetPlayerEntityFromClientID((*it))->GetSquad() == ySquad)
                        SendGameData((*it), buffer, false);
                }
            }
        }
        catch (CException &ex)
        {
            ex.Process(_T("CGameServer::ProcessGameData(GAME_CMD_MINIMAP_DRAW) - "), NO_THROW);
        }
        break;

    case GAME_CMD_PETCMD:
        {
            byte yPetCmd(pkt.ReadByte());

            IPlayerEntity *pPlayer(GetPlayerFromClientNum(iClientNum));
            if (pPlayer == NULL)
                break;

            pPlayer->PetCommand(EPetCommand(yPetCmd), INVALID_INDEX, V3_ZERO);
            return true;
        }
        break;

    case GAME_CMD_PETCMD_ENT:
        {
            byte yPetCmd(pkt.ReadByte());
            uint uiIndex(pkt.ReadInt());

            IPlayerEntity *pPlayer(GetPlayerFromClientNum(iClientNum));
            if (pPlayer == NULL)
                break;

            pPlayer->PetCommand(EPetCommand(yPetCmd), uiIndex, V3_ZERO);
            return true;
        }
        break;

    case GAME_CMD_PETCMD_POS:
        {
            byte yPetCmd(pkt.ReadByte());
            CVec3f v3Pos(pkt.ReadV3f());

            IPlayerEntity *pPlayer(GetPlayerFromClientNum(iClientNum));
            if (pPlayer == NULL)
                break;

            pPlayer->PetCommand(EPetCommand(yPetCmd), INVALID_INDEX, v3Pos);
            return true;
        }
        break;

    case GAME_CMD_OFFICERCMD:
        {
            byte yOfficerCmd(pkt.ReadByte());

            IPlayerEntity *pPlayer(GetPlayerFromClientNum(iClientNum));
            if (pPlayer == NULL)
                break;

            if (!pPlayer->IsOfficer())
                break;

            pPlayer->OfficerCommand(EOfficerCommand(yOfficerCmd), INVALID_INDEX, V3_ZERO);
            return true;
        }
        break;

    case GAME_CMD_OFFICERCMD_ENT:
        {
            byte yOfficerCmd(pkt.ReadByte());
            uint uiIndex(pkt.ReadInt());

            IPlayerEntity *pPlayer(GetPlayerFromClientNum(iClientNum));
            if (pPlayer == NULL)
                break;

            if (!pPlayer->IsOfficer())
                break;

            pPlayer->OfficerCommand(EOfficerCommand(yOfficerCmd), uiIndex, V3_ZERO);
            return true;
        }
        break;

    case GAME_CMD_OFFICERCMD_POS:
        {
            byte yOfficerCmd(pkt.ReadByte());
            CVec3f v3Pos(pkt.ReadV3f());

            IPlayerEntity *pPlayer(GetPlayerFromClientNum(iClientNum));
            if (pPlayer == NULL)
                break;

            if (!pPlayer->IsOfficer())
                break;

            pPlayer->OfficerCommand(EOfficerCommand(yOfficerCmd), INVALID_INDEX, v3Pos);
            return true;
        }
        break;

    case GAME_CMD_SWAP_INVENTORY:
        {
            int iSlot1(pkt.ReadInt());
            int iSlot2(pkt.ReadInt());

            IPlayerEntity *pPlayer(GetPlayerFromClientNum(iClientNum));
            if (pPlayer == NULL)
                break;

            pPlayer->SwapItem(iSlot1, iSlot2);
        }
        break;

    case GAME_CMD_SUBMIT_REPLAY_COMMENT:
        {
            int iRating(pkt.ReadInt());
            tstring sComment(pkt.ReadString());
            
            int iAccountID(-1);
            ClientInfoMap_it itClient(m_mapClients.find(iClientNum));
            if (itClient != m_mapClients.end())
                iAccountID = itClient->second->GetAccountID();

            StatsTracker.SubmitRating(iAccountID, iRating, sComment);
        }
        break;

    case GAME_CMD_SUBMIT_COMMANDER_RATING:
        {
            int iRating(pkt.ReadInt());
            tstring sComment(pkt.ReadString());

            if (StatsTracker.GetMatchID() == -1 || Game.GetGamePhase() != GAME_PHASE_ENDED)
                break;

            if (iRating < 1 || iRating > 5)
                break;
            
            int iAccountID(-1);
            ClientInfoMap_it itClient(m_mapClients.find(iClientNum));

            if (itClient == m_mapClients.end())
                break;

            if (itClient->second->GetTeam() < 1 || itClient->second->IsDemoAccount())
                break;

            if (itClient->second->GetPlayTime() < SecToMs(ICvar::GetUnsignedInteger(_T("svr_minStatsTime"))) && itClient->second->GetCommPlayTime() < SecToMs(ICvar::GetUnsignedInteger(_T("svr_minStatsTimeComm"))))
                break;

            iAccountID = itClient->second->GetAccountID();

            if (m_setCommanderReviewSubmitted.find(iAccountID) != m_setCommanderReviewSubmitted.end())
                break;

            CEntityTeamInfo *pTeam(GetTeam(itClient->second->GetTeam()));

            if (pTeam == NULL)
                break;

            CEntityClientInfo *pComm(GetClientInfo(pTeam->GetOfficialCommanderClientID()));

            if (pComm == NULL)
                break;
            
            if (pComm->GetAccountID() == iAccountID)
                break;

            m_setCommanderReviewSubmitted.insert(iAccountID);
            
            m_pDBManager->AddRequestVariable(_T("f"), _T("cr_vote"));
            m_pDBManager->AddRequestVariable(_T("account_id"), XtoA(iAccountID));
            m_pDBManager->AddRequestVariable(_T("comm_id"), XtoA(pComm->GetAccountID()));
            m_pDBManager->AddRequestVariable(_T("match_id"), XtoA(StatsTracker.GetMatchID()));
            m_pDBManager->AddRequestVariable(_T("vote"), XtoA(iRating));
            m_pDBManager->AddRequestVariable(_T("reason"), sComment);
            m_pDBManager->SendRequest(_T("CommanderRating"), false);
        }
        break;

    case GAME_CMD_SUBMIT_KARMA_RATING:
        {
            bool bAdd(pkt.ReadByte() != 0);
            tstring sComment(pkt.ReadString());
            int iTargetClient(pkt.ReadInt());

            if (StatsTracker.GetMatchID() == -1 || Game.GetGamePhase() != GAME_PHASE_ENDED)
                break;
            
            int iAccountID(-1);
            ClientInfoMap_it itClient(m_mapClients.find(iClientNum));

            if (itClient == m_mapClients.end())
                break;

            iAccountID = itClient->second->GetAccountID();

            if (m_setKarmaReviewSubmitted.find(iAccountID) != m_setKarmaReviewSubmitted.end())
                break;

            if (itClient->second->GetTeam() < 1 || itClient->second->IsDemoAccount())
                break;
            
            if (itClient->second->GetPlayTime() < SecToMs(ICvar::GetUnsignedInteger(_T("svr_minStatsTime"))) && itClient->second->GetCommPlayTime() < SecToMs(ICvar::GetUnsignedInteger(_T("svr_minStatsTimeComm"))))
                break;

            CEntityClientInfo *pTarget(GetClientInfo(iTargetClient));

            if (pTarget == NULL)
                break;

            if (pTarget->GetAccountID() == iAccountID)
                break;

            m_setKarmaReviewSubmitted.insert(iAccountID);
            
            m_pDBManager->AddRequestVariable(_T("f"), _T("upd_karma"));
            m_pDBManager->AddRequestVariable(_T("account_id"), XtoA(iAccountID));
            m_pDBManager->AddRequestVariable(_T("target_id"), XtoA(pTarget->GetAccountID()));
            m_pDBManager->AddRequestVariable(_T("match_id"), XtoA(StatsTracker.GetMatchID()));
            m_pDBManager->AddRequestVariable(_T("do"), bAdd ? _T("add") : _T("remove"));
            m_pDBManager->AddRequestVariable(_T("reason"), sComment);
            m_pDBManager->SendRequest(_T("KarmaRating"), false);
        }
        break;

    case GAME_CMD_VOICE_DATA:
        {
            uint uiLength(pkt.ReadInt(-1));
            if (uiLength == -1)
                return false;

            char *pEncodedData(K2_NEW_ARRAY(global, char, uiLength));
            pkt.Read(pEncodedData, uiLength);
            if (pkt.HasFaults())
            {
                SAFE_DELETE(pEncodedData);
                return false;
            }

            SendVoiceData(iClientNum, uiLength, pEncodedData);
            SAFE_DELETE(pEncodedData);
        }
        break;

    case GAME_CMD_VOICE_STOPTALKING:
        {
            StoppedTalking(iClientNum);
        }
        break;

    case GAME_CMD_VOICE_STARTTALKING:
        {
            StartedTalking(iClientNum);
        }
        break;

    case GAME_CMD_VOICECOMMAND:
        {
            EVCType eType(EVCType(pkt.ReadByte()));
            tstring sRace(pkt.ReadString());
            tstring sCategory(pkt.ReadString());
            tstring sButton(pkt.ReadString());

            IPlayerEntity *pSourcePlayer(GetPlayerFromClientNum(iClientNum));
            CBufferDynamic buffer;

            if (pSourcePlayer == NULL)
                break;

            buffer << GAME_CMD_VOICECOMMAND << iClientNum << sRace << byte(0) << sCategory << byte(0) << sButton << byte(0);

            for (ClientInfoMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
            {
                IPlayerEntity *pPlayer(it->second->GetPlayerEntity());
                if (pPlayer == NULL)
                    continue;
                if (pPlayer->GetClientID() == iClientNum)
                    continue;

                if (eType == VC_ALL ||
                    (eType == VC_TEAM && pPlayer->GetTeam() == pSourcePlayer->GetTeam()) ||
                    (eType == VC_COMMANDER && pPlayer->GetTeam() == pSourcePlayer->GetTeam() && pPlayer->GetAsCommander()) ||
                    (eType == VC_SQUAD && pPlayer->GetTeam() == pSourcePlayer->GetTeam() && pPlayer->GetSquad() == pSourcePlayer->GetSquad()))
                    SendGameData(pPlayer->GetClientID(), buffer, true);
            }
        }
        break;

    case GAME_CMD_SCRIPT_INPUT:
        {
            short nNumArgs(pkt.ReadShort());

            for (int i(0); i < nNumArgs; i++)
            {
                tstring sParam(pkt.ReadString());
                tstring sData(pkt.ReadString());

                TriggerManager.RegisterTriggerParam(sParam, sData);
            }

            TriggerManager.RegisterTriggerParam(_T("clientid"), XtoA(iClientNum));
            
            ClientInfoMap_it itClient(m_mapClients.find(iClientNum));
            if (itClient != m_mapClients.end())
            {
                TriggerManager.RegisterTriggerParam(_T("name"), itClient->second->GetName());
                TriggerManager.RegisterTriggerParam(_T("accountid"), XtoA(itClient->second->GetAccountID()));
                TriggerManager.RegisterTriggerParam(_T("index"), XtoA(itClient->second->GetPlayerEntityIndex()));
            }

            TriggerManager.TriggerGlobalScript(_T("scriptinput"));
        }
        break;

    case GAME_CMD_SET_REPLAY_CLIENT:
        {
            int iNewClientNum(pkt.ReadInt());

            if (!ReplayManager.IsPlaying() || iNewClientNum == iClientNum)
                break;

            ClientMap   &mapClients(m_pHostServer->GetClientMap());
            ClientMap_it it(mapClients.find(iClientNum));
            if (it == mapClients.end())
                break;

            CClientConnection *pClientConnection(it->second);

            mapClients.erase(it);

            pClientConnection->SetClientNum(iNewClientNum);

            mapClients[iNewClientNum] = pClientConnection;
        }
        break;

    case GAME_CMD_SERVER_STATUS:
        {
            CBufferDynamic buffer;
            buffer << GAME_CMD_CONSOLE_MESSAGE << GetServerStatus() << newl << byte(0);

            SendGameData(iClientNum, buffer, true);
        }
        break;

    case GAME_CMD_REPAIRABLE:
        {
            IPlayerEntity *pPlayer(GetPlayerFromClientNum(iClientNum));

            if (pPlayer == NULL)
                break;

            CPlayerCommander *pComm(pPlayer->GetAsCommander());  
            if (pComm == NULL)  
                break;

            uiset setSelection(pComm->GetSelection());  
            for (uiset::iterator it(setSelection.begin()); it != setSelection.end(); it++)  
            { 

                IVisualEntity *pEnt(Game.GetVisualEntity(*it)); 

                if (pEnt == NULL)  
                    continue; 
    
                IBuildingEntity *pBuilding(pEnt->GetAsBuilding()); 
                if (pBuilding == NULL)  
                    continue; 

                if (pBuilding->GetTeam() != pComm->GetTeam())
                    continue;

                if (pBuilding->HasNetFlags(ENT_NET_FLAG_NO_REPAIR)) 
                    pBuilding->SetRepairable(true); 
                else 
                    pBuilding->SetRepairable(false); 
            }


        }
        break;

    default:
        Console.Warn << _T("Unrecognized message") << newl;
        return false;
    }

    return true;
}


/*====================
  CGameServer::GetSnapshot
  ====================*/
void    CGameServer::GetSnapshot(CSnapshot &snapshot)
{
    PROFILE("CGameServer::GetSnapshot");

    if (ReplayManager.IsPlaying())
    {
        ReplayManager.GetSnapshot(snapshot);
    }
    else
    {
        // Update game info entity
        if (m_pGameInfo)
        {
            m_pGameInfo->SetGamePhase(GetGamePhase());
            m_pGameInfo->SetPhaseStartTime(GetPhaseStartTime());
            m_pGameInfo->SetPhaseDuration(GetPhaseDuration());
            m_pGameInfo->SetGameMatchID(StatsTracker.GetMatchID());
            m_pGameInfo->SetSuddenDeath(IsSuddenDeathActive());
        }

        // Entities
        m_pServerEntityDirectory->GetSnapshot(snapshot);

        // Events
        GetEventSnapshot(snapshot);
        ClearEventList();

        ReplayManager.WriteSnapshot(snapshot);

        m_pHostServer->UpdateStateStrings();
    }
    
    ReplayManager.EndFrame();
}


/*====================
  CGameServer::Shutdown
  ====================*/
void    CGameServer::Shutdown()
{
    try
    {
        Console << _T("Shutting down server...") << newl;

        ReplayManager.StopRecording();

        CFileHandle hFile(_T("~/game_settings_local.cfg"), FILE_WRITE | FILE_TEXT);
        if (!hFile.IsOpen())
            EX_ERROR(_T("Failed to open config file"));

        svector vsWildCards;
        if (!ICvar::WriteConfigFile(hFile, vsWildCards, CVAR_GAMECONFIG))
            EX_WARN(_T("Error writing cvars"));

        m_pServerEntityDirectory->Clear();

        Console.StopGameLog();
    }
    catch (CException &ex)
    {
        ex.Process(_T("CGameServer::Shutdown() - "), NO_THROW);
    }
}


/*====================
  CGameServer::LoadWorld
  ====================*/
bool    CGameServer::LoadWorld()
{
    try
    {
        if (!IsWorldLoaded())
            EX_ERROR(_T("Host has not loaded the world yet"));

        if (ReplayManager.IsPlaying())
            return true;

        // Builds pathing blockers based on the normals of the map's terrain
        AnalyzeTerrain();

        TriggerManager.TriggerGlobalScript(_T("mapunloaded"));

        // Clear scripts from previous world
        TriggerManager.ClearAllEntityScripts();
        TriggerManager.ClearGlobalScripts();

        // Register global scripts
        smaps &mapWorldScripts(GetWorldScriptMap());
        for (smaps::iterator it(mapWorldScripts.begin()); it != mapWorldScripts.end(); it++)
            TriggerManager.RegisterGlobalScript(it->first, it->second);

        if (m_pGameInfo)
        {
            m_pServerEntityDirectory->Delete(m_pGameInfo->GetIndex());
            m_pGameInfo = NULL;
        }

        m_pGameInfo = static_cast<CEntityGameInfo *>(m_pServerEntityDirectory->Allocate(Entity_GameInfo));

        // Allocate teams
        ClearTeams();
        AddTeam(_T("Neutral"), g_teamdefNeutral);

        tstring sRace[2];

        sRace[0] = LowerString(sv_team1Race.GetString());
        sRace[1] = LowerString(sv_team2Race.GetString());
        
        for (int i(0); i < 2; i++)
        {
            if (sRace[i] == _T("beast") || sRace[i] == _T("b"))
                AddTeam(_T("Team ") + XtoA(i + 1), g_teamdefBeast);
            else
                AddTeam(_T("Team ") + XtoA(i + 1), g_teamdefHuman);
        }

        if (!sRace[0].empty() && !sRace[1].empty())
            sv_raceString.Set(UpperString(sRace[0].substr(0,1)) + _T("v") + UpperString(sRace[1].substr(0,1)));

        // Spawn game entities for each world entity that requires one
        WorldEntMap &mapWorldEnts(GetWorldEntityMap());

        for (WorldEntMap_it it(mapWorldEnts.begin()); it != mapWorldEnts.end(); ++it)
        {
            CWorldEntity *pWorldEntity(GetWorldEntity(it->first));
            if (!pWorldEntity)
            {
                Console.Err << _T("Failed world entity lookup on #") + XtoA(it->first) << newl;
                continue;
            }

            ushort uiID(EntityRegistry.LookupID(pWorldEntity->GetType()));
            if (uiID == ushort(-1))
                continue;

            IGameEntity* pNewEnt(m_pServerEntityDirectory->Allocate(uiID));
            if (pNewEnt == NULL)
            {
                Console.Err << _T("Failed to allocate a game entity for world entity #") + XtoA(it->first) << newl;
                continue;
            }

            pWorldEntity->SetGameIndex(pNewEnt->GetIndex());

            pNewEnt->ApplyWorldEntity(*pWorldEntity);

            if (pNewEnt->IsVisual())
            {
                Console.ServerGame << _T("Spawned new entity #") << pNewEnt->GetIndex() << _T(" (") + EntityRegistry.LookupName(uiID) + _T(") @ ") << pNewEnt->GetAsVisualEnt()->GetPosition()
                            << SPACE << pNewEnt->GetAsVisualEnt()->GetAngles() << newl;
            }
            else
            {
                Console.ServerGame << _T("Spawned new entity #") << pNewEnt->GetIndex() << _T(" (") + EntityRegistry.LookupName(uiID) + _T(")") << newl;
            }
        }

        // Spawn lights
        WorldLightsMap &mapWorldLights(GetWorldLightsMap());
        for (WorldLightsMap_it it(mapWorldLights.begin()); it != mapWorldLights.end(); ++it)
        {
            IGameEntity* pNewEnt(m_pServerEntityDirectory->Allocate(Light_Static));
            if (pNewEnt == NULL)
            {
                Console.Err << _T("Failed to allocate a light for world light #") + XtoA(it->first) << newl;
                continue;
            }

            ILight *pLight(pNewEnt->GetAsLight());
            if (pLight == NULL)
            {
                Console.Err << _T("Allocated game entity is not the correct type") << newl;
                continue;
            }

            pLight->SetWorldIndex(it->second->GetIndex());
            Console.ServerGame << _T("Spawned new light #") << pLight->GetIndex() << _T(" @ ") << pLight->GetPosition() << newl;
        }

        m_pServerEntityDirectory->Spawn();

        // Validate the teams
        if (!GetTeam(1)->IsValid())
            Console.Err << _T("Team 1 is invalid") << newl;
        if (!GetTeam(2)->IsValid())
            Console.Err << _T("Team 2 is invalid") << newl;

        if (sv_warmup)
            StartWarmup();
        else
            SetGamePhase(GAME_PHASE_WAITING_FOR_PLAYERS);

        TriggerManager.TriggerGlobalScript(_T("maploaded"));

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CGameServer::LoadWorld() - "), THROW);
        return false;
    }
}


/*====================
  CGameServer::ResetWorld
  ====================*/
bool    CGameServer::ResetWorld()
{
    try
    {
        if (!IsWorldLoaded())
            EX_ERROR(_T("Host has not loaded the world yet"));

        if (ReplayManager.IsPlaying())
            return true;

        // Reset entities
        m_pServerEntityDirectory->Reset();

        // Reset world
        GetWorldPointer()->Reset();

        // Spawn game entities for each world entity that requires one
        WorldEntMap &mapWorldEnts(GetWorldEntityMap());

        for (WorldEntMap_it it(mapWorldEnts.begin()); it != mapWorldEnts.end(); ++it)
        {
            CWorldEntity *pWorldEntity(GetWorldEntity(it->first));
            if (!pWorldEntity)
            {
                Console.Err << _T("Failed world entity lookup on #") + XtoA(it->first) << newl;
                continue;
            }

            ushort uiID(EntityRegistry.LookupID(pWorldEntity->GetType()));
            if (uiID == ushort(-1))
                continue;

            IGameEntity* pNewEnt(m_pServerEntityDirectory->Allocate(uiID));
            if (pNewEnt == NULL)
            {
                Console.Err << _T("Failed to allocate a game entity for world entity #") + XtoA(it->first) << newl;
                continue;
            }

            pWorldEntity->SetGameIndex(pNewEnt->GetIndex());

            pNewEnt->ApplyWorldEntity(*pWorldEntity);

            if (pNewEnt->IsVisual())
            {
                Console.ServerGame << _T("Spawned new entity #") << pNewEnt->GetIndex() << _T(" (") + EntityRegistry.LookupName(uiID) + _T(") @ ") << pNewEnt->GetAsVisualEnt()->GetPosition()
                            << SPACE << pNewEnt->GetAsVisualEnt()->GetAngles() << newl;
            }
            else
            {
                Console.ServerGame << _T("Spawned new entity #") << pNewEnt->GetIndex() << _T(" (") + EntityRegistry.LookupName(uiID) + _T(")") << newl;
            }
        }

        // Spawn lights
        WorldLightsMap &mapWorldLights(GetWorldLightsMap());
        for (WorldLightsMap_it it(mapWorldLights.begin()); it != mapWorldLights.end(); ++it)
        {
            IGameEntity* pNewEnt(m_pServerEntityDirectory->Allocate(Light_Static));
            if (pNewEnt == NULL)
            {
                Console.Err << _T("Failed to allocate a light for world light #") + XtoA(it->first) << newl;
                continue;
            }

            ILight *pLight(pNewEnt->GetAsLight());
            if (pLight == NULL)
            {
                Console.Err << _T("Allocated game entity is not the correct type") << newl;
                continue;
            }

            pLight->SetWorldIndex(it->second->GetIndex());
            Console.ServerGame << _T("Spawned new light #") << pLight->GetIndex() << _T(" @ ") << pLight->GetPosition() << newl;
        }

        m_pServerEntityDirectory->Spawn();

        // Reset team lists to remove commanders, officers, etc.
        for (ClientInfoMap_it itClient(m_mapClients.begin()); itClient != m_mapClients.end(); ++itClient)
        {
            if (itClient->second->IsDisconnected())
                continue;

            CEntityTeamInfo *pTeam(GetTeam(itClient->second->GetTeam()));

            if (pTeam != NULL)
            {
                pTeam->RemoveClient(itClient->first);
                pTeam->AddClient(itClient->first);
            }

            ChangeUnit(itClient->first, Player_Observer);
        }

        TriggerManager.TriggerGlobalScript(_T("mapreset"));

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CGameServer::LoadWorld() - "), THROW);
        return false;
    }
}


/*====================
  CGameServer::UnloadWorld
  ====================*/
void    CGameServer::UnloadWorld()
{
    TriggerManager.TriggerGlobalScript(_T("mapunloaded"));

    // Clear scripts from previous world
    TriggerManager.ClearAllEntityScripts();
    TriggerManager.ClearGlobalScripts();

    m_pServerEntityDirectory->Clear();

    m_pGameInfo = NULL;

    SetWorldPointer(NULL);
}


/*====================
  CGameServer::StartReplay
  ====================*/
bool    CGameServer::StartReplay(const tstring &sFilename)
{
    if (!ReplayManager.StartPlayback(sFilename))
        return false;

    MapStateString &mapStateString(ReplayManager.GetStateStrings());
    for (MapStateString::iterator it(mapStateString.begin()); it != mapStateString.end(); ++it)
        m_pHostServer->SetStateString(it->first, it->second);

    mapStateString.clear();

    return m_pHostServer->LoadWorld(ReplayManager.GetWorldName());
}


/*====================
  CGameServer::StopReplay
  ====================*/
void    CGameServer::StopReplay()
{
    ReplayManager.StopPlayback();
}


/*====================
  CGameServer::RegisterModel
  ====================*/
ResHandle   CGameServer::RegisterModel(const tstring &sPath)
{
    ResHandle hHandle(g_ResourceManager.Register(sPath, RES_MODEL, RES_MODEL_SERVER));
    if (hHandle == INVALID_RESOURCE)
        return INVALID_RESOURCE;

    NetworkResourceManager.GetNetIndex(hHandle); // Register with NetworkResourceManager

    return hHandle;
}


/*====================
  CGameServer::RegisterEffect
  ====================*/
ResHandle   CGameServer::RegisterEffect(const tstring &sPath)
{
    ResHandle hHandle(g_ResourceManager.Register(sPath, RES_EFFECT, RES_EFFECT_IGNORE_ALL));
    if (hHandle == INVALID_RESOURCE)
        return INVALID_RESOURCE;

    NetworkResourceManager.GetNetIndex(hHandle); // Register with NetworkResourceManager

    return hHandle;
}


/*====================
  CGameServer::GetClientNumFromAccountID
  ====================*/
int     CGameServer::GetClientNumFromAccountID(int iAccountID)
{
    return m_pHostServer->GetClientNumFromAccountID(iAccountID);
}

/*====================
  CGameServer::GetClientNumFromName
  ====================*/
int     CGameServer::GetClientNumFromName(const tstring &sName)
{
    ClientInfoMap_it itClient(m_mapClients.begin());

    for (; itClient != m_mapClients.end(); itClient++)
        if (itClient->second->GetName() == sName)
            return itClient->first;

    return -1;
}

/*====================
  CGameServer::GetPlayerFromClientNum
  ====================*/
IPlayerEntity*  CGameServer::GetPlayerFromClientNum(int iClientNum)
{
    ClientInfoMap_it itClient(m_mapClients.find(iClientNum));
    if (itClient != m_mapClients.end())
        return itClient->second->GetPlayerEntity();

    return NULL;
}


/*====================
  CGameServer::SendMessage
  ====================*/
void    CGameServer::SendMessage(const tstring &sMsg, int iClientNum)
{
    CBufferDynamic buffer;

    buffer << GAME_CMD_MESSAGE << sMsg << byte(0);

    if (iClientNum == -1)
        BroadcastGameData(buffer, true);
    else
        SendGameData(iClientNum, buffer, true);
    
    Console.Server << sMsg << newl;
}


/*====================
  CGameServer::BroadcastGameData
  ====================*/
void    CGameServer::BroadcastGameData(const IBuffer &buffer, bool bReliable, int iExcludeClient)
{
    if (ReplayManager.IsRecording())
    {
        ClientMap &mapClients(m_pHostServer->GetClientMap());

        for (ClientMap_it it(mapClients.begin()); it != mapClients.end(); ++it)
        {
            if (it->first == iExcludeClient)
                continue;

            buffer.Rewind();
            ReplayManager.WriteGameData(it->first, buffer, bReliable);
        }
    }

    m_pHostServer->BroadcastGameData(buffer, bReliable, iExcludeClient);
}


/*====================
  CGameServer::SendGameData
  ====================*/
void    CGameServer::SendGameData(int iClient, const IBuffer &buffer, bool bReliable)
{
    if (ReplayManager.IsRecording())
    {
        buffer.Rewind();
        ReplayManager.WriteGameData(iClient, buffer, bReliable);
    }

    m_pHostServer->SendGameData(iClient, buffer, bReliable);
}


/*====================
  CGameServer::PrecacheEntity
  ====================*/
void    CGameServer::PrecacheEntity(const tstring &sName)
{
    EntityRegistry.ServerPrecache(EntityRegistry.LookupID(sName));
}


/*====================
  CGameServer::PrecacheEntities
  ====================*/
void    CGameServer::PrecacheEntities()
{
    // Human buildings
    PrecacheEntity(_T("Building_Academy"));
    PrecacheEntity(_T("Building_Armory"));
    PrecacheEntity(_T("Building_ArrowTower"));
    PrecacheEntity(_T("Building_CannonTower"));
    PrecacheEntity(_T("Building_Garrison"));
    PrecacheEntity(_T("Building_HumanHellShrine"));
    PrecacheEntity(_T("Building_Monastery"));
    PrecacheEntity(_T("Building_ShieldTower"));
    PrecacheEntity(_T("Building_SiegeWorkshop"));
    PrecacheEntity(_T("Building_SteamMine"));
    PrecacheEntity(_T("Building_Stronghold"));

    // Human units
    PrecacheEntity(_T("Player_BatteringRam"));
    PrecacheEntity(_T("Player_Chaplain"));
    PrecacheEntity(_T("Player_Engineer"));
    PrecacheEntity(_T("Player_Legionnaire"));
    PrecacheEntity(_T("Player_Marksman"));
    PrecacheEntity(_T("Player_Savage"));
    PrecacheEntity(_T("Player_Steambuchet"));

    PrecacheEntity(_T("Pet_HumanWorker"));

    // Beast buildings
    PrecacheEntity(_T("Building_PredatorDen"));
    PrecacheEntity(_T("Building_Nexus"));
    PrecacheEntity(_T("Building_StrataSpire"));
    PrecacheEntity(_T("Building_EntangleSpire"));
    PrecacheEntity(_T("Building_Sublair"));
    PrecacheEntity(_T("Building_HumanHellShrine"));
    PrecacheEntity(_T("Building_Sanctuary"));
    PrecacheEntity(_T("Building_ChlorophilicSpire"));
    PrecacheEntity(_T("Building_CharmShrine"));
    PrecacheEntity(_T("Building_GroveMine"));
    PrecacheEntity(_T("Building_Lair"));

    // Beast units
    PrecacheEntity(_T("Player_Conjurer"));
    PrecacheEntity(_T("Player_Shapeshifter"));
    PrecacheEntity(_T("Player_Summoner"));
    PrecacheEntity(_T("Player_Shaman"));
    PrecacheEntity(_T("Player_Predator"));
    PrecacheEntity(_T("Player_Behemoth"));
    PrecacheEntity(_T("Player_Tempest"));

    PrecacheEntity(_T("Pet_BeastWorker"));

    // Hellbourne units
    PrecacheEntity(_T("Player_Revenant"));
    PrecacheEntity(_T("Player_Malphas"));

    // Observer
    PrecacheEntity(_T("Player_Observer"));

    // Commander
    PrecacheEntity(_T("Player_Commander"));

    // Officer
    PrecacheEntity(_T("State_Officer"));
    PrecacheEntity(_T("State_OfficerAura"));
    PrecacheEntity(_T("Gadget_HumanOfficerSpawnFlag"));
    PrecacheEntity(_T("Gadget_BeastSpawnPortal"));

    // Persistant items
    PrecacheEntity(_T("Persistant_Item"));

    // Consumable items
    PrecacheEntity(_T("Consumable_AmmoPack"));
    PrecacheEntity(_T("Consumable_AmmoSatchel"));
    PrecacheEntity(_T("Consumable_Chainmail"));
    PrecacheEntity(_T("Consumable_HealthMajor"));
    PrecacheEntity(_T("Consumable_HealthMinor"));
    PrecacheEntity(_T("Consumable_HealthReplenish"));
    PrecacheEntity(_T("Consumable_HealthShrine"));
    PrecacheEntity(_T("Consumable_ManaClarity"));
    PrecacheEntity(_T("Consumable_ManaMajor"));
    PrecacheEntity(_T("Consumable_ManaMinor"));
    PrecacheEntity(_T("Consumable_ManaShrine"));
    PrecacheEntity(_T("Consumable_Platemail"));
    PrecacheEntity(_T("Consumable_StoneHide"));
    PrecacheEntity(_T("Consumable_ToughSkin"));
    PrecacheEntity(_T("Consumable_ManaCrystal"));
    PrecacheEntity(_T("Consumable_ManaStone"));
    PrecacheEntity(_T("Consumable_SpeedBoost"));
    PrecacheEntity(_T("Consumable_LynxFeet"));
    PrecacheEntity(_T("Consumable_StaminaMajor"));
    PrecacheEntity(_T("Consumable_StaminaMinor"));

    // Misc
    PrecacheEntity(_T("Entity_Soul"));
    PrecacheEntity(_T("Entity_Chest"));
    PrecacheEntity(_T("State_Dash"));
}


/*====================
  CGameServer::StateStringChanged
  ====================*/
void    CGameServer::StateStringChanged(uint uiID, const CStateString &ss)
{
    Console << _T("CGameServer::StateStringChanged ") << uiID << newl;

    ReplayManager.WriteStateString(uiID, ss);
}


/*====================
  CGameServer::GetStateString
  ====================*/
CStateString&   CGameServer::GetStateString(uint uiID)
{
    return m_pHostServer->GetStateString(uiID);
}


/*====================
  CGameServer::GetServerFrame
  ====================*/
uint    CGameServer::GetServerFrame()
{
    return m_pHostServer->GetFrameNumber();
}


/*====================
  CGameServer::GetServerTime
  ====================*/
uint    CGameServer::GetServerTime()
{
    return m_pHostServer->GetTime();
}


/*====================
  CGameServer::GetPrevServerTime
  ====================*/
uint    CGameServer::GetPrevServerTime()
{
    return m_pHostServer->GetTime() - m_pHostServer->GetFrameLength();
}


/*====================
  CGameServer::GetServerFrameLength
  ====================*/
uint    CGameServer::GetServerFrameLength()
{
    return m_pHostServer->GetFrameLength();
}


/*====================
  CGameServer::AddTeam
  ====================*/
int     CGameServer::AddTeam(const tstring &sName, CTeamDefinition &teamdef)
{
    CEntityTeamInfo *pNewTeam(static_cast<CEntityTeamInfo *>(m_pServerEntityDirectory->Allocate(Entity_TeamInfo)));
    if (pNewTeam == NULL)
    {
        Console.Err << _T("IGame::AddTeam() - Failed to allocate new CEntityTeamInfo for team: ") << sName << newl;
        return -1;
    }

    pNewTeam->SetTeamID(GetNumTeams());
    pNewTeam->SetName(sName);
    pNewTeam->SetDefinition(&teamdef);

    SetTeam(pNewTeam->GetTeamID(), pNewTeam);

    return pNewTeam->GetTeamID();
}


/*====================
  CGameServer::StoppedTalking
  ====================*/
void    CGameServer::StoppedTalking(int iClientNum)
{
    if (sv_disableVoiceChat)
        return;

    CBufferDynamic buffer;
    buffer << GAME_CMD_VOICE_STOPTALKING << iClientNum;

    iset setTargets(GetVoiceTargets(iClientNum));
    for (iset::iterator it(setTargets.begin()); it != setTargets.end(); ++it)
        SendGameData(*it, buffer, true);
}


/*====================
  CGameServer::StartedTalking
  ====================*/
void    CGameServer::StartedTalking(int iClientNum)
{
    if (sv_disableVoiceChat)
        return;

    CBufferDynamic buffer;
    buffer << GAME_CMD_VOICE_STARTTALKING << iClientNum;

    iset setTargets(GetVoiceTargets(iClientNum));
    for (iset::iterator it(setTargets.begin()); it != setTargets.end(); ++it)
        SendGameData(*it, buffer, true);
}


/*====================
  CGameServer::GetVoiceTargets
  ====================*/
iset    CGameServer::GetVoiceTargets(int iClientNum)
{
    iset setTargets;
    if (sv_disableVoiceChat)
        return setTargets;

    IPlayerEntity *pPlayer(GetPlayerFromClientNum(iClientNum));
    if (pPlayer == NULL)
        return setTargets;

    if (GetGamePhase() == GAME_PHASE_ENDED ||
        GetGamePhase() == GAME_PHASE_WAITING_FOR_PLAYERS)
    {
        // Voice chat goes to everyone during "game ended" and "waiting for player" phases
        for (ClientInfoMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
        {
            if (it->first != iClientNum)
                setTargets.insert(it->first);
        }

        return setTargets;
    }

    CEntityTeamInfo *pTeam(GetTeam(pPlayer->GetTeam()));
    if (pTeam == NULL)
        return setTargets;

    uint uiTeamIndex(pTeam->GetTeamIndexFromClientID(iClientNum));
    if (uiTeamIndex == -1)
        return setTargets;

    const ivector &vTeamList(pTeam->GetClientList());

    // HACK: Just send commander voice for now
    if (pTeam->IsCommander(uiTeamIndex) /*|| pTeam->GetTeamID() == 0*/)
    {
        // Commander sends to entire team
        for (ivector_cit it(vTeamList.begin()); it != vTeamList.end(); ++it)
        {
            if (*it != iClientNum)
                setTargets.insert(*it);
        }
    }
    /*
    else if (pTeam->IsOfficer(uiTeamIndex))
    {
        // Officer sends to commander, other officers and his own squad
        for (ivector_cit it(vTeamList.begin()); it != vTeamList.end(); ++it)
        {
            if (*it == iClientNum)
                continue;

            uint uiIndex(pTeam->GetTeamIndexFromClientID(iClientNum));
            IPlayerEntity *pTarget(GetPlayerFromClientNum(iClientNum));
            if (pTeam->IsOfficer(uiIndex) || pTeam->IsCommander(uiIndex) || pTarget->GetSquad() == pPlayer->GetSquad())
                setTargets.insert(*it);
        }
    }
    else if (pPlayer->GetSquad() != -1)
    {
        // Others only send to their squadmates and officer
        for (ivector_cit it(vTeamList.begin()); it != vTeamList.end(); ++it)
        {
            if (*it == iClientNum)
                continue;

            IPlayerEntity *pTarget(GetPlayerFromClientNum(iClientNum));
            if (pTarget->GetSquad() == pPlayer->GetSquad())
                setTargets.insert(*it);
        }
    }
    */
    
    return setTargets;
}


/*====================
  CGameServer::SendVoiceData
  ====================*/
void    CGameServer::SendVoiceData(int iClientNum, uint uiLength, char *pData)
{
    if (sv_disableVoiceChat)
        return;

    CBufferDynamic buffer;
    buffer << GAME_CMD_VOICE_DATA << iClientNum << uiLength;
    buffer.Append(pData, uiLength);

    iset setTargets(GetVoiceTargets(iClientNum));
    for (iset::iterator it(setTargets.begin()); it != setTargets.end(); ++it)
        SendGameData(*it, buffer, false);
}


/*====================
  CGameServer::RemoveVoiceClient
  ====================*/
void    CGameServer::RemoveVoiceClient(int iClientNum)
{
    if (sv_disableVoiceChat)
        return;

    CBufferDynamic buffer;
    buffer << GAME_CMD_VOICE_REMOVECLIENT << iClientNum;

    iset setTargets(GetVoiceTargets(iClientNum));

    for (iset::iterator it(setTargets.begin()); it != setTargets.end(); ++it)
        SendGameData(*it, buffer, true);
}


/*====================
  CGameServer::RemoveAllVoiceClients
  ====================*/
void    CGameServer::RemoveAllVoiceClients()
{
    if (sv_disableVoiceChat)
        return;

    for (ClientInfoMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
        RemoveVoiceClient(it->first);
}


/*====================
  CGameServer::InitCensor
  ====================*/
void CGameServer::InitCensor()
{
/*  CFile *pFile = FileManager.GetFile(_T(":/censor.txt"), FILE_READ | FILE_TEXT);

    if (pFile != NULL)
    {
        tstring sLine;
        tstring sFirst;
        tstring sLast;
        tstring::size_type pos;

        while (!pFile->IsEOF())
        {
            sLine = pFile->ReadLine();

            StripNewline(sLine);
            sLine = LowerString(sLine);

            if (!sLine.empty())
            {
                pos = sLine.find(_T(":"));

                if (pos == tstring::npos)
                    continue;

                sFirst = sLine.substr(0, pos);
                sLast = sLine.substr(pos + 1, sLine.length() - pos - 1);

                m_mapCensor.insert(pair<tstring, tstring>(sFirst, sLast));
            }
        }
    }*/

    m_mapCensor.insert(pair<tstring, tstring>(_T("niggers"), _T("fluffy bunnies")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("nigger"), _T("fluffy bunny")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("nigga"), _T("fluffy bunny")));

    m_mapCensor.insert(pair<tstring, tstring>(_T("fuck"), _T("hug")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("suck"), _T("tickle")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("newb"), _T("silly person")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("noob"), _T("silly person")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("n00b"), _T("silly person")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("dick"), _T("doorknob")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("cock"), _T("doorknob")));
}


/*====================
  CGameServer::CensorChat

  TODO: Make this function more thorough!
  ====================*/
tstring CGameServer::CensorChat(const tstring &sMessage)
{
    bool bFound(true);
    tstring::size_type pos(0);
    tstring sLower(LowerString(sMessage));
    tstring sNewMessage(sMessage);

    while (bFound)
    {
        bFound = false;

        for (smaps::reverse_iterator it(m_mapCensor.rbegin()); it != m_mapCensor.rend(); it++)
        {
            pos = sLower.find(it->first);

            if (pos != tstring::npos)
            {
                // Only censor if it is not part of a larger word
                if ((pos == 0 || sLower[pos - 1] == _T(' ') || (IsNotDigit(sLower[pos - 1]) && !IsLetter(sLower[pos - 1]))) &&
                    (pos + it->first.length() == sLower.length() || sLower[pos + it->first.length()] == _T(' ') || (IsNotDigit(sLower[pos + it->first.length()]) && !IsLetter(sLower[pos + it->first.length()]))))
                {
                    sLower.erase(pos, it->first.length());
                    sNewMessage.erase(pos, it->first.length());

                    sLower.insert(pos, it->second);
                    sNewMessage.insert(pos, it->second);

                    bFound = true;
                }
            }
        }
    }

    return sNewMessage;
}


/*====================
  CGameServer::GetPersistantItemType
  ====================*/
ushort  CGameServer::GetPersistantItemType(int iClientNum, int iVaultNum)
{
    PersistantVaultMap_it findit(m_mapPersistantVaults.find(iClientNum));

    if (findit == m_mapPersistantVaults.end() || iVaultNum < 0 || iVaultNum >= MAX_PERSISTANT_ITEMS)
        return -1;

    return findit->second.unItemType[iVaultNum];
}


/*====================
  CGameServer::GetPersistantItemID
  ====================*/
uint    CGameServer::GetPersistantItemID(int iClientNum, int iVaultNum)
{
    PersistantVaultMap_it findit(m_mapPersistantVaults.find(iClientNum));

    if (findit == m_mapPersistantVaults.end() || iVaultNum < 0 || iVaultNum >= MAX_PERSISTANT_ITEMS)
        return -1;

    return findit->second.uiItemID[iVaultNum];
}


/*====================
  CGameServer::GetRandomPersistantItem
  ====================*/
ushort  CGameServer::GetRandomPersistantItem()
{
    //
    // Regen
    //
    uint uiRegenMod(PERSISTANT_REGEN_NULL);
    {
        float fSelectionWeightRange(0.0f);

        for (uint ui(0); ui < NUM_PERSISTANT_REGEN_MODS; ++ui)
            fSelectionWeightRange += g_PersistantItemsConfig.GetRegenDropWeight(ui);
        
        float   fRand(M_Randnum(0.0f, fSelectionWeightRange));
        for (;;)
        {
            fRand -= g_PersistantItemsConfig.GetRegenDropWeight(uiRegenMod);

            if (fRand > 0.0f)
            {
                ++uiRegenMod;
                if (uiRegenMod == NUM_PERSISTANT_REGEN_MODS)
                {
                    uiRegenMod = PERSISTANT_REGEN_NULL;
                    break;
                }
            }
            else
            {
                break;
            }
        }

        if (uiRegenMod == NUM_PERSISTANT_REGEN_MODS)
            uiRegenMod = PERSISTANT_REGEN_NULL;
    }

    //
    // Type
    //
    uint uiPersistantType(PERSISTANT_TYPE_NULL);
    {
        float fSelectionWeightRange(0.0f);

        for (uint ui(0); ui < NUM_PERSISTANT_ITEM_TYPES; ++ui)
            fSelectionWeightRange += g_PersistantItemsConfig.GetTypeDropWeight(ui);
        
        float   fRand(M_Randnum(0.0f, fSelectionWeightRange));
        for (;;)
        {
            fRand -= g_PersistantItemsConfig.GetTypeDropWeight(uiPersistantType);

            if (fRand > 0.0f)
            {
                ++uiPersistantType;
                if (uiPersistantType == NUM_PERSISTANT_ITEM_TYPES)
                {
                    uiPersistantType = PERSISTANT_TYPE_NULL;
                    break;
                }
            }
            else
            {
                break;
            }
        }

        if (uiPersistantType == NUM_PERSISTANT_ITEM_TYPES)
            uiPersistantType = PERSISTANT_TYPE_NULL;
    }

    //
    // Increase
    //
    uint uiIncreaseMod(PERSISTANT_INCREASE_NULL);
    {
        float fSelectionWeightRange(0.0f);

        for (uint ui(0); ui < NUM_PERSISTANT_INCREASE_MODS; ++ui)
            fSelectionWeightRange += g_PersistantItemsConfig.GetIncreaseDropWeight(ui);
        
        float   fRand(M_Randnum(0.0f, fSelectionWeightRange));
        for (;;)
        {
            fRand -= g_PersistantItemsConfig.GetIncreaseDropWeight(uiIncreaseMod);

            if (fRand > 0.0f)
            {
                ++uiIncreaseMod;
                if (uiIncreaseMod == NUM_PERSISTANT_INCREASE_MODS)
                {
                    uiIncreaseMod = PERSISTANT_INCREASE_NULL;
                    break;
                }
            }
            else
            {
                break;
            }
        }

        if (uiIncreaseMod == NUM_PERSISTANT_INCREASE_MODS)
            uiIncreaseMod = PERSISTANT_INCREASE_NULL;
    }

    //
    // Replenish
    //
    uint uiReplenishMod(PERSISTANT_REPLENISH_NULL);
    {
        float fSelectionWeightRange(0.0f);

        for (uint ui(0); ui < NUM_PERSISTANT_REPLENISH_MODS; ++ui)
            fSelectionWeightRange += g_PersistantItemsConfig.GetReplenishDropWeight(ui);
        
        float   fRand(M_Randnum(0.0f, fSelectionWeightRange));
        for (;;)
        {
            fRand -= g_PersistantItemsConfig.GetReplenishDropWeight(uiReplenishMod);

            if (fRand > 0.0f)
            {
                ++uiReplenishMod;
                if (uiReplenishMod == NUM_PERSISTANT_REPLENISH_MODS)
                {
                    uiReplenishMod = PERSISTANT_REPLENISH_NULL;
                    break;
                }
            }
            else
            {
                break;
            }
        }

        if (uiReplenishMod == NUM_PERSISTANT_REPLENISH_MODS)
            uiReplenishMod = PERSISTANT_REPLENISH_NULL;
    }

    short unItemData(0);

    unItemData += uiPersistantType * 1000;
    unItemData += uiRegenMod * 100;
    unItemData += uiIncreaseMod * 10;
    unItemData += uiReplenishMod;

    return unItemData;
}


/*====================
  CGameServer::GetRandomItem
  ====================*/
ushort  CGameServer::GetRandomItem(IVisualEntity *pEntity)
{
    if (m_vItemDrops.empty())
        return INVALID_ENT_TYPE;

    float fSelectionWeightRange(0.0f);

    for (vector<ushort>::iterator it(m_vItemDrops.begin()); it != m_vItemDrops.end(); ++it)
    {
        if (pEntity)
        {
            ICvar *pPrerequisite(EntityRegistry.GetGameSetting(*it, _T("Prerequisite")));
            if (pEntity->GetTeam() > 0 && pPrerequisite && !pPrerequisite->GetString().empty())
            {
                CEntityTeamInfo *pTeamInfo(Game.GetTeam(pEntity->GetTeam()));
                if (pTeamInfo && !pTeamInfo->HasBuilding(pPrerequisite->GetString()))
                    continue;
            }
        }

        fSelectionWeightRange += EntityRegistry.GetGameSettingFloat(*it, _T("DropWeight"), 0.0f);
    }

    uint uiItem(0);
    
    float   fRand(M_Randnum(0.0f, fSelectionWeightRange));
    for (;;)
    {
        ICvar *pPrerequisite(EntityRegistry.GetGameSetting(m_vItemDrops[uiItem], _T("Prerequisite")));
        if (pEntity->GetTeam() > 0 && pPrerequisite && !pPrerequisite->GetString().empty())
        {
            CEntityTeamInfo *pTeamInfo(Game.GetTeam(pEntity->GetTeam()));
            if (pTeamInfo && !pTeamInfo->HasBuilding(pPrerequisite->GetString()))
            {
                ++uiItem;
                continue;
            }
        }

        fRand -= EntityRegistry.GetGameSettingFloat(m_vItemDrops[uiItem], _T("DropWeight"), 0.0f);

        if (fRand > 0.0f)
        {
            ++uiItem;
            if (uiItem == m_vItemDrops.size())
            {
                uiItem = 0;
                break;
            }
        }
        else
        {
            break;
        }
    }

    if (uiItem == m_vItemDrops.size())
        return INVALID_ENT_TYPE;

    return m_vItemDrops[uiItem];
}


/*====================
  CGameServer::MatchStatEvent
  ====================*/
void    CGameServer::MatchStatEvent(int iClientNumber, EPlayerMatchStat eStat, float fValue, int iTargetClientID, ushort unInflictorType, ushort unTargetType, uint uiTime)
{
    if (GetGamePhase() != GAME_PHASE_ACTIVE)
        return;

    ClientInfoMap_it itClient(m_mapClients.find(iClientNumber));
    if (itClient == m_mapClients.end())
        return;

    if (eStat >= COMMANDER_MATCH_START && !itClient->second->HasFlags(CLIENT_INFO_IS_COMMANDER))
        return;
    
    itClient->second->MatchStatEvent(eStat, fValue, iTargetClientID, unInflictorType, unTargetType, uiTime);
}

void    CGameServer::MatchStatEvent(int iClientNumber, EPlayerMatchStat eStat, int iValue, int iTargetClientID, ushort unInflictorType, ushort unTargetType, uint uiTime)
{
    if (GetGamePhase() != GAME_PHASE_ACTIVE)
        return;

    ClientInfoMap_it itClient(m_mapClients.find(iClientNumber));
    if (itClient == m_mapClients.end())
        return;

    if (eStat >= COMMANDER_MATCH_START && !itClient->second->HasFlags(CLIENT_INFO_IS_COMMANDER))
        return;
    
    itClient->second->MatchStatEvent(eStat, iValue, iTargetClientID, unInflictorType, unTargetType, uiTime);
}


/*====================
  CGameServer::GetServerStatus
  ====================*/
tstring     CGameServer::GetServerStatus()
{
    int iActiveClients(0);
    int iDisconnects(0);

    for (ClientInfoMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
    {
        if (it->second->IsDisconnected())
            ++iDisconnects;
        else
            ++iActiveClients;
    }

    tstring sRet;

    sRet += _T("Server Status:");
    sRet += _T(" Timestamp") + ParenStr(XtoA(GetGameTime()));
    sRet += _T(" Active Clients") + ParenStr(XtoA(iActiveClients));
    sRet += _T(" Disconnects") + ParenStr(XtoA(iDisconnects));
    sRet += _T(" Entities") + ParenStr(XtoA(m_pServerEntityDirectory->GetNumEntities()));
    sRet += _T(" Snapshots") + ParenStr(XtoA(CEntitySnapshot::GetEntitySnapshotPool()->GetNumAllocated()));

    return sRet;
}


/*====================
  CGameServer::Kick
  ====================*/
void    CGameServer::Kick(int iClientNum, const tstring sReason)
{
    if (m_pHostServer != NULL)
        m_pHostServer->KickClient(iClientNum, sReason);
}


/*====================
  CGameServer::Ban
  ====================*/
void    CGameServer::Ban(int iClientNum, int iLength, const tstring sReason)
{
    if (m_pHostServer != NULL)
        m_pHostServer->BanClient(iClientNum, iLength, sReason);
}


