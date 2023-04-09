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
#include "c_gamelogevent.h"

#include "../aba_shared/c_replaymanager.h"
#include "../aba_shared/c_gameinfo.h"
#include "../aba_shared/c_gamestats.h"
#include "../aba_shared/c_player.h"
#include "../aba_shared/c_teaminfo.h"
#include "../aba_shared/i_buildingentity.h"
#include "../aba_shared/i_heroentity.h"
#include "../aba_shared/i_entityability.h"
#include "../aba_shared/i_light.h"
#include "../aba_shared/i_entityitem.h"
#include "../aba_shared/c_entitycreepspawner.h"
#include "../aba_shared/i_bitentity.h"
#include "../aba_shared/c_entitypowerupspawner.h"
#include "../aba_shared/c_entitycritterspawner.h"
#include "../aba_shared/c_entitychest.h"
#include "../aba_shared/i_shopentity.h"
#include "../aba_shared/c_shopdefinition.h"
#include "../aba_shared/c_entityneutralcampcontroller.h"
#include "../aba_shared/c_entitykongorcontroller.h"
#include "../aba_shared/c_gamemechanicsresource.h"
#include "../aba_shared/c_shopinfo.h"
#include "../aba_shared/c_shopiteminfo.h"
#include "../aba_shared/c_bhold.h"

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
#include "../k2/c_host.h"
#include "../k2/c_model.h"
#include "../k2/c_effect.h"
#include "../k2/c_texture.h"
#include "../k2/c_timermanager.h"
#include "../k2/c_eventmanager.h"
#include "../k2/c_date.h"
#include "../k2/c_voiceserver.h"
#include "../k2/c_phpdata.h"
#include "../k2/c_httpmanager.h"
#include "../k2/c_httprequest.h"
#include "../k2/c_checksumtable.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
SINGLETON_INIT(CGameServer)

CVAR_STRINGF(       svr_remotePass,                 "",             CVAR_SAVECONFIG);

CVAR_UINTF(         sv_heroSelectTime,              MinToMs(1.0f),  CVAR_GAMECONFIG);
CVAR_UINTF(         sv_heroSelectTimeSD,            MinToMs(1.0f),  CVAR_GAMECONFIG);
CVAR_UINTF(         sv_heroAltSelectInitialTime,    SecToMs(60u),   CVAR_GAMECONFIG);
CVAR_UINTF(         sv_heroAltSelectTurnTime,       SecToMs(25u),   CVAR_GAMECONFIG);
CVAR_UINTF(         sv_heroAltSelectPostTime,       MinToMs(1.0f),  CVAR_GAMECONFIG);
CVAR_UINTF(         sv_heroBanTime,                 SecToMs(20u),   CVAR_GAMECONFIG);
CVAR_UINTF(         sv_heroBanInitialTime,          SecToMs(15u),   CVAR_GAMECONFIG);
CVAR_UINTF(         sv_heroBanTransitionTime,       SecToMs(15u),   CVAR_GAMECONFIG);
CVAR_UINTF(         sv_heroAllBanTime,              SecToMs(40u),   CVAR_GAMECONFIG);
CVAR_UINTF(         sv_heroAllBanInitialTime,       SecToMs(10u),   CVAR_GAMECONFIG);
CVAR_UINTF(         sv_heroAllBanTransitionTime,    SecToMs(15u),   CVAR_GAMECONFIG);
CVAR_UINTF(         sv_heroAllBanExtraTime,         SecToMs(60u),   CVAR_GAMECONFIG);
CVAR_UINTF(         sv_preMatchTime,                MinToMs(2.0f),  CVAR_GAMECONFIG);
CVAR_UINTF(         sv_matchStartAnnounceTime,      SecToMs(10u),   CVAR_GAMECONFIG);
CVAR_UINTF(         sv_gameEndPhaseTime,            MinToMs(5.0f),  CVAR_GAMECONFIG);
CVAR_BOOLF(         sv_precacheEntities,            true,           CVAR_SAVECONFIG);
CVAR_BOOLF(         sv_autosaveReplay,              false,          CVAR_SAVECONFIG);
CVAR_BOOLF(         sv_disableVoiceChat,            false,          CVAR_SAVECONFIG);
CVAR_BOOLF(         sv_debugVoiceChat,              false,          CVAR_SAVECONFIG);
CVAR_BOOLF(         sv_autosaveGameLog,             true,           CVAR_SAVECONFIG);
CVAR_UINTF(         sv_statusNotifyTime,            SecToMs(60u),   CVAR_SAVECONFIG);
CVAR_UINTF(         sv_afkTimeout,                  MinToMs(3u),    CVAR_GAMECONFIG);
CVAR_UINTF(         sv_afkWarningTime,              SecToMs(180u),  CVAR_GAMECONFIG);
CVAR_UINTF(         sv_maxDisconnectedTime,         MinToMs(5u),    CVAR_SAVECONFIG);
CVAR_BOOLF(         sv_gameAutoStart,               false,          CVAR_SAVECONFIG);
CVAR_UINTF(         sv_mapPingDelay,                200,            CVAR_GAMECONFIG);
CVAR_BOOLF(         sv_reliablePopups,              false,          CVAR_SAVECONFIG);
CVAR_BOOLF(         sv_reliablePings,               false,          CVAR_SAVECONFIG);
CVAR_BOOL(          sv_allowSolo,                   false);
CVAR_BOOL(          sv_allowDev,                    false);
CVAR_BOOL(          sv_allowEmpty,                  false);
CVAR_UINTF(         sv_gameStartCountdown,          SecToMs(5u),    CVAR_GAMECONFIG);
CVAR_STRING(        sv_masterName,                  "");

CVAR_UINTF(         sv_gameCountdownLength,         6500,           CVAR_SAVECONFIG);
CVAR_UINTF(         sv_maxHeroLoadingTime,          30000,          CVAR_SAVECONFIG);

CVAR_UINTF(         sv_arrangedMatchWaitTime,       SecToMs(180u),  CVAR_SAVECONFIG);
CVAR_UINTF(         sv_tournMatchWaitTime,          SecToMs(600u),  CVAR_SAVECONFIG);

CVAR_BOOL(          sv_spawnTrees,              true);
CVAR_BOOL(          sv_spawnCreeps,             true);
CVAR_BOOL(          sv_spawnPowerups,           true);
CVAR_BOOL(          sv_spawnCritters,           true);

CVAR_UINTF(         g_creepWaveInterval,            SecToMs(30u),               CVAR_GAMECONFIG);
CVAR_UINTF(         g_creepSiegeInterval,           5,                          CVAR_GAMECONFIG);
ARRAY_CVAR_UINTF(   g_creepFormationIndexes,        _T("0,31,65,87,97,115"),    CVAR_GAMECONFIG);
ARRAY_CVAR_UINTF(   g_creepMeleeCount,              _T("3,4,5,5,6,7"),          CVAR_GAMECONFIG);
ARRAY_CVAR_UINTF(   g_creepRangedCount,             _T("1,1,1,2,2,2"),          CVAR_GAMECONFIG);
ARRAY_CVAR_UINTF(   g_creepSiegeCount,              _T("1,1,1,2,2,2"),          CVAR_GAMECONFIG);

CVAR_UINTF(         g_treeSpawnInterval,            MinToMs(5u),                CVAR_GAMECONFIG);
CVAR_UINTF(         g_powerupSpawnInterval,         MinToMs(2u),                CVAR_GAMECONFIG);
CVAR_UINTF(         g_critterSpawnInterval,         MinToMs(2u),                CVAR_GAMECONFIG);

CVAR_FLOATF(        g_critterNoRespawnProximity,    500.0f,                     CVAR_GAMECONFIG);

CVAR_FLOATF(        g_voiceLaneChatRange,           2000.0f,                    CVAR_GAMECONFIG);

CVAR_UINTF(         g_creepUpgradeInterval,         MinToMs(7u),                CVAR_GAMECONFIG);
CVAR_UINTF(         g_creepMaxUpgrades,             30,                         CVAR_GAMECONFIG);

CVAR_UINTF(         g_voteDuration,                 SecToMs(20u),               CVAR_GAMECONFIG);
CVAR_FLOATF(        g_voteRemakeLeaverRequired,     0.5f,                       CVAR_GAMECONFIG);
CVAR_FLOATF(        g_voteRemakeRequired,           0.7f,                       CVAR_GAMECONFIG);
CVAR_FLOATF(        g_votePauseRequired,            0.7f,                       CVAR_GAMECONFIG);

CVAR_FLOATF(        psf_logisticPredictionScale,    200.0f,                     CVAR_GAMECONFIG | CVAR_TRANSMIT);

CVAR_UINTF(         g_pauseDelayTime,               SecToMs(5u),                CVAR_GAMECONFIG);
CVAR_UINTF(         g_pauseAllowance,               SecToMs(60u),               CVAR_GAMECONFIG);
CVAR_UINTF(         g_pauseMaxTime,                 MinToMs(10u),               CVAR_GAMECONFIG);
//=============================================================================

/*====================
  CGameServer::~CGameServer
  ====================*/
CGameServer::~CGameServer()
{
    Console << _T("Game server released") << newl;
    SAFE_DELETE(m_pServerEntityDirectory);
}


/*====================
  CGameServer::CGameServer
  ====================*/
CGameServer::CGameServer() :
m_pServerEntityDirectory(NULL),
m_pHostServer(NULL),

m_uiLastStatusNotifyTime(INVALID_TIME),
m_uiLastChatCounterDecrement(INVALID_TIME),

m_uiMatchCreationTime(INVALID_TIME),

m_uiDraftRound(0),
m_uiFinalDraftRound(0),
m_iBanRound(0),
m_uiBanningTeam(TEAM_INVALID),

m_uiCreepWaveCount(0),
m_uiNextCreepWaveTime(INVALID_TIME),
m_uiLastCreepUpgradeLevel(0),

m_uiNextTreeSpawnTime(INVALID_TIME),
m_uiNextPowerupSpawnTime(INVALID_TIME),
m_uiNextCritterSpawnTime(INVALID_TIME),

m_cVisRaster(SQR(RASTER_BUFFER_SPAN)),
m_cOccRaster(SQR(RASTER_BUFFER_SPAN)),

m_bSentTrialGame(false),
m_bStartAnnounced(false),
m_bHasForcedRandom(false),
m_iActiveTeam(TEAM_INVALID),
m_iCreatorClientNum(-1),
m_uiPowerupUID(INVALID_INDEX),
m_uiLastVisibilityUpdate(INVALID_TIME),
m_bSolo(false),
m_bEmpty(false),
m_bLocal(false),

m_fAverageRating(0.0f),

m_bAllowedUnpause(false),
m_uiPauseToggleTime(INVALID_TIME),
m_uiPauseStartTime(INVALID_TIME),

m_uiLongServerFrame(0)
{
    m_fTeamAverageRating[0] = 0.0f;
    m_fTeamAverageRating[1] = 0.0f;

    m_pServerEntityDirectory = K2_NEW(g_heapEntity,   CServerEntityDirectory);
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
    {
        pEntity->GetAsVisual()->Unlink();
        pEntity->GetAsVisual()->ReleaseBinds();
    }
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

        SetGameTime(0);
        SetTotalTime(0);
        SetFrameLength(0);

        // Store a pointer to the world
        SetWorldPointer(m_pHostServer->GetWorld());
        SetEntityDirectory(m_pServerEntityDirectory);
        Validate();

        if (Host.IsReplay())
            return true;

        ICvar::UnprotectTransmitCvars();

        // Load game settings
        ICvar::SetTrackModifications(false);
        Console.ExecuteScript(_T("/game_settings.cfg"));
        ICvar::SetTrackModifications(true);

        PrecacheEntities(K2System.IsDedicatedServer());

        NetworkResourceManager.Clear();

        m_pHostServer->AddStateBlock(STATE_BLOCK_ENTITY_TYPES);
        CStateBlock &blockTypes(m_pHostServer->GetStateBlock(STATE_BLOCK_ENTITY_TYPES));
        EntityRegistry.WriteDynamicEntities(blockTypes.GetBuffer());
        blockTypes.Modify();

        m_bSentTrialGame = false;
        m_bStartAnnounced = false;
        m_bHasForcedRandom = false;
        m_iActiveTeam = TEAM_INVALID;

        m_vGuests.clear();

        m_uiPauseToggleTime = INVALID_TIME;
        m_uiPauseStartTime = INVALID_TIME;
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CGameServer::Initialize() - "), NO_THROW);
        return false;
    }
}


/*====================
  CGameServer::ReplayFrame
  ====================*/
void    CGameServer::ReplayFrame()
{
    MapStateString &mapStateString(ReplayManager.GetStateStrings());
    for (MapStateString::iterator it(mapStateString.begin()); it != mapStateString.end(); ++it)
        m_pHostServer->SetStateString(it->first, it->second);

    MapStateBlock &mapStateBlocks(ReplayManager.GetStateBlocks());
    for (MapStateBlock::iterator itBlock(mapStateBlocks.begin()), itEnd(mapStateBlocks.end()); itBlock != itEnd; ++itBlock)
        m_pHostServer->SetStateBlock(itBlock->first, itBlock->second);

#ifdef K2_CLIENT
    CClientConnection *pClient(m_pHostServer->GetClient());

    if (pClient != NULL)
    {
        CBufferDynamic cBufferReliable;
        ReplayManager.GetGameDataReliable(pClient->GetClientNum(), cBufferReliable);
        if (cBufferReliable.GetLength() > 0)
            pClient->SendGameData(cBufferReliable, true);

        CBufferDynamic cBuffer;
        ReplayManager.GetGameData(pClient->GetClientNum(), cBuffer);
        if (cBuffer.GetLength() > 0)
            pClient->SendGameData(cBuffer, false);
    }
#else
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
#endif
}


/*====================
  CGameServer::VoteFrame
  ====================*/
void    CGameServer::VoteFrame()
{
    // Check voting progress
    CGameInfo *pGameInfo(GetGameInfo());
    if (pGameInfo == NULL)
        return;

    switch (pGameInfo->GetActiveVoteType())
    {
    case VOTE_TYPE_CONCEDE:
        {
            // Tally votes
            int iVotesRequired(0);
            int iYesVotes(0);

            for (PlayerMap_it it(m_mapClients.begin()), itEnd(m_mapClients.end()); it != itEnd; ++it)
            {
                if (it->second->GetTeam() != pGameInfo->GetVoteTarget())
                    continue;
                if (it->second->IsDisconnected())
                    continue;

                ++iVotesRequired;
                if (it->second->GetVote() == VOTE_YES)
                    ++iYesVotes;
            }

            pGameInfo->SetVotesRequired(iVotesRequired);
            pGameInfo->SetYesVotes(iYesVotes);

            // Check results
            if (iYesVotes >= iVotesRequired)
            {
                int iTarget(pGameInfo->GetVoteTarget());
                pGameInfo->VotePassed();
                ResetPlayerVotes();
                Concede(iTarget);
            }
        }
        break;

    case VOTE_TYPE_REMAKE:
        {
            // Tally votes
            int iVotesRequired(0);
            int iYesVotes(0);
            bool bHasLeaver(false);

            for (PlayerMap_it it(m_mapClients.begin()), itEnd(m_mapClients.end()); it != itEnd; ++it)
            {
                if (it->second->GetTeam() != TEAM_1 && it->second->GetTeam() != TEAM_2)
                    continue;
                
                if (it->second->HasFlags(PLAYER_FLAG_TERMINATED))
                    bHasLeaver = true;

                if (it->second->IsDisconnected())
                    continue;

                ++iVotesRequired;
                if (it->second->GetVote() == VOTE_YES)
                    ++iYesVotes;
            }

            // Require a 70% majority (50% if there is a termination)
            if (bHasLeaver)
                iVotesRequired = INT_ROUND(iVotesRequired * g_voteRemakeLeaverRequired);
            else
                iVotesRequired = INT_ROUND(iVotesRequired * g_voteRemakeRequired);

            // Require at least 2 votes always, so a 1v1 must be unanimous
            iVotesRequired = MAX(iVotesRequired, 2);

            pGameInfo->SetVotesRequired(iVotesRequired);
            pGameInfo->SetYesVotes(iYesVotes);

            // Check results
            if (iYesVotes >= iVotesRequired)
            {
                ResetPlayerVotes();
                pGameInfo->VotePassed();
                Reset();
                return;
            }
        }
        break;

    case VOTE_TYPE_KICK_AFK:
        {
            CPlayer *pTarget(GetPlayer(pGameInfo->GetVoteTarget()));

            // Make sure kick target is still valid
            if (!IsKickVoteValid(pTarget))
            {
                ResetPlayerVotes();
                pGameInfo->VoteFailed();
                break;
            }

            // Tally votes
            int iVotesRequired(0);
            int iYesVotes(0);

            for (PlayerMap_it it(m_mapClients.begin()), itEnd(m_mapClients.end()); it != itEnd; ++it)
            {
                if (it->first == pGameInfo->GetVoteTarget())
                    continue;
                if (it->second->GetTeam() != pTarget->GetTeam())
                    continue;
                if (it->second->IsDisconnected())
                    continue;

                ++iVotesRequired;
                if (it->second->GetVote() == VOTE_YES)
                    ++iYesVotes;
            }

            pGameInfo->SetVotesRequired(iVotesRequired);
            pGameInfo->SetYesVotes(iYesVotes);

            // Check results
            if (iYesVotes >= iVotesRequired)
            {
                ResetPlayerVotes();
                pGameInfo->VotePassed();
                
                pTarget->SetFlags(PLAYER_FLAG_KICKED);
                m_GameLog.WritePlayer(GAME_LOG_PLAYER_KICKED_VOTE, pTarget);
                m_pHostServer->KickClient(pTarget->GetClientNumber(), _T("disconnect_vote_kicked"));
                pTarget->Terminate();
            }
        }
        break;

    case VOTE_TYPE_PAUSE:
        {
            // Make sure team is valid
            CTeamInfo *pTeam(GetTeam(pGameInfo->GetVoteTarget()));
            if (pTeam == NULL)
            {
                ResetPlayerVotes();
                pGameInfo->VoteFailed();
                break;
            }

            // Tally votes
            int iVotesRequired(0);
            int iYesVotes(0);

            for (PlayerMap_it it(m_mapClients.begin()), itEnd(m_mapClients.end()); it != itEnd; ++it)
            {
                if (it->second->GetTeam() != pTeam->GetTeamID())
                    continue;
                if (it->second->IsDisconnected())
                    continue;

                ++iVotesRequired;
                if (it->second->GetVote() == VOTE_YES)
                    ++iYesVotes;
            }

            // Allow a majority vote to pause
            iVotesRequired = INT_ROUND(iVotesRequired * g_votePauseRequired);
            pGameInfo->SetVotesRequired(iVotesRequired);
            pGameInfo->SetYesVotes(iYesVotes);

            if (iYesVotes >= iVotesRequired)
            {
                pGameInfo->VotePassed();
                ResetPlayerVotes();

                if (!m_pHostServer->GetPaused() && m_uiPauseToggleTime == INVALID_TIME)
                    m_uiPauseToggleTime = GetTotalTime() + g_pauseDelayTime;

                pTeam->Pause();
            }
        }
        break;
    }

    if (pGameInfo->GetVoteEndTime() != INVALID_TIME && pGameInfo->GetVoteEndTime() <= GetTotalTime())
    {
        ResetPlayerVotes();
        pGameInfo->VoteFailed();
    }
}


/*====================
  CGameServer::UpdatePauseStatus
  ====================*/
void    CGameServer::UpdatePauseStatus()
{
    // Update game flag
    if (m_pHostServer->GetPaused())
        SetFlags(GAME_FLAG_PAUSED);
    else
        RemoveFlags(GAME_FLAG_PAUSED);

    // If a player paused the match, check for pause expiration
    // A manual server pause will not have a valid m_uiPauseStartTime
    if (m_pHostServer->GetPaused() && m_uiPauseStartTime != INVALID_TIME)
    {
        // If pause allowance is exceeded, allow all teams to unpause
        if (!m_bAllowedUnpause && GetTotalTime() - m_uiPauseStartTime > g_pauseAllowance)
        {
            const map<uint, CTeamInfo*> mapTeams(GetTeams());
            for (map<uint, CTeamInfo*>::const_iterator itTeam(mapTeams.begin()), itEnd(mapTeams.end()); itTeam != itEnd; ++itTeam)
            {
                if (!itTeam->second->IsActiveTeam())
                    continue;

                itTeam->second->SetFlags(TEAM_FLAG_CAN_UNPAUSE);
            }

            m_bAllowedUnpause = true;
        }

        // If pause exceeds maximum time, force match to restart
        if (GetTotalTime() - m_uiPauseStartTime > g_pauseMaxTime && m_uiPauseToggleTime == INVALID_TIME)
            StartUnpause(NULL);

        // If the game is over or all the players are gone, force an unpause
        bool bHasPlayers(false);

        for (uint uiTeam(TEAM_1); uiTeam <= TEAM_2; ++uiTeam)
        {
            CTeamInfo *pTeam(GetTeam(uiTeam));
            if (pTeam == NULL)
                continue;

            for (uint uiPlayer(0); uiPlayer < pTeam->GetNumClients(); ++uiPlayer)
            {
                CPlayer *pPlayer(pTeam->GetPlayer(uiPlayer));
                if (pPlayer == NULL)
                    continue;

                if (pPlayer->HasFlags(PLAYER_FLAG_TERMINATED))
                    continue;

                bHasPlayers = true;
                break;
            }

            if (bHasPlayers)
                break;
        }

        if (!bHasPlayers || GetGamePhase() < GAME_PHASE_PRE_MATCH || GetGamePhase() > GAME_PHASE_ACTIVE)
        {
            m_pHostServer->SetPaused(false);
            m_uiPauseStartTime = INVALID_TIME;
            m_uiPauseToggleTime = INVALID_TIME;
        }
    }

    // Check for pause transitions
    if (m_uiPauseToggleTime == INVALID_TIME)
        return;

    if (m_uiPauseToggleTime < GetTotalTime())
    {
        m_uiPauseToggleTime = INVALID_TIME;

        if (m_pHostServer->GetPaused())
        {
            m_pHostServer->SetPaused(false);
            m_uiPauseStartTime = INVALID_TIME;
        }
        else
        {
            m_pHostServer->SetPaused(true);
            m_uiPauseStartTime = GetTotalTime();
            m_bAllowedUnpause = false;
        }
    }
    else
    {
        // Little hacky, similar to game start countdown...
        uint uiRemainingTime(m_uiPauseToggleTime - GetTotalTime());
        if (uiRemainingTime / 1000 != (uiRemainingTime + GetFrameLength()) / 1000)
        {
            CBufferDynamic buffer;
            buffer << GAME_CMD_MESSAGE << TStringToUTF8(XtoA((uiRemainingTime / 1000) + 1) + _T("...")) << byte(0);
            BroadcastGameData(buffer, true);
        }
    }
}


/*====================
  CGameServer::Frame
  ====================*/
void    CGameServer::Frame()
{
    FetchGameMechanics();

    ReplayManager.StartFrame(m_pHostServer->GetFrameNumber());

    if (ReplayManager.IsPlaying())
    {
        ReplayFrame();
        return;
    }

    SetGameTime(m_pHostServer->GetTime());
    SetTotalTime(m_pHostServer->GetTime() + m_pHostServer->GetPauseTime());
    SetFrameLength(m_pHostServer->GetFrameLength());

    if (m_uiLongServerFrame != 0)
    {
        CBufferFixed<3> buffer;
        buffer << GAME_CMD_LONG_SERVER_FRAME << ushort(m_uiLongServerFrame);
        BroadcastGameData(buffer, false);

        m_uiLongServerFrame = 0;
    }

    UpdatePauseStatus();

    CGameInfo *pGameInfo(GetGameInfo());
    if (pGameInfo == NULL)
    {
        m_pServerEntityDirectory->BackgroundFrame();
        return;
    }

    bool bDecrementChatCounter(false);
    if (m_uiLastChatCounterDecrement == INVALID_TIME || GetTotalTime() - m_uiLastChatCounterDecrement >= sv_chatCounterDecrementInterval)
    {
        bDecrementChatCounter = true;
        m_uiLastChatCounterDecrement = GetTotalTime();
    }

    // Update client connections
    for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
    {
        CPlayer *pPlayer(it->second);
        if (pPlayer == NULL)
            continue;

        // Ignore terminated clients
        if (pPlayer->HasFlags(PLAYER_FLAG_TERMINATED))
            continue;

        // Update chat flood protection
        pPlayer->SetAllowChat(true);

        if (bDecrementChatCounter)
            pPlayer->DecrementChatCounter();

        // Check for terminating disconnected clients
        if (pPlayer->IsDisconnected())
        {
            if (GetGameTime() >= pPlayer->GetTerminationTime() &&
                !pGameInfo->HasFlags(GAME_FLAG_CONCEDED) &&
                GetGamePhase() < GAME_PHASE_ENDED &&
                pPlayer->GetTeam() != TEAM_SPECTATOR)
            {
                pPlayer->Terminate();
                m_GameLog.WritePlayer(GAME_LOG_PLAYER_TERMINATED, pPlayer);
            }
            else
            {
                if (!m_pHostServer->GetPaused())
                {
                    // Include the time a player is disconnected (but not yet terminated) into 
                    // their total play time, since they are still earning exp and gold.
                    if (GetGamePhase() == GAME_PHASE_ACTIVE)
                        pPlayer->AddPlayTime(GetFrameLength());
                }
            }

            continue;
        }

        CClientConnection *pClientConnection(m_pHostServer->GetClient(pPlayer->GetClientNumber()));
        if (pClientConnection == NULL)
            continue;

        if (pPlayer->GetTeam() == TEAM_1)
            pClientConnection->SetStream(1);
        else if (pPlayer->GetTeam() == TEAM_2)
            pClientConnection->SetStream(2);
        else
            pClientConnection->SetStream(0);

        // Kick idle clients
        //if (!pClientConnection->HasFlags(CLIENT_CONNECTION_LOCAL))
        {
            if (GetGamePhase() < GAME_PHASE_ACTIVE || pPlayer->GetTeam() == TEAM_SPECTATOR || pPlayer->GetTeam() >= TEAM_INVALID)
            {
                pPlayer->SetLastInteractionTime(GetGameTime());
            }
            else if (GetGamePhase() < GAME_PHASE_ENDED)
            {
                uint uiAFKTime(pPlayer->GetLastInteractionTime() == INVALID_TIME ? 0 : GetGameTime() - pPlayer->GetLastInteractionTime());
                if (uiAFKTime >= sv_afkTimeout)
                {
                    if (!pPlayer->HasFlags(PLAYER_FLAG_IS_AFK))
                    {
                        CBufferFixed<1> buffer;
                        buffer << GAME_CMD_AFK_MESSAGE;
                        SendGameData(pClientConnection->GetClientNum(), buffer, true);
                        pPlayer->SetFlags(PLAYER_FLAG_IS_AFK);
                    }
                }
                else
                {
                    pPlayer->RemoveFlags(PLAYER_FLAG_IS_AFK);

                    if (uiAFKTime >= sv_afkWarningTime)
                    {
                        if (!pPlayer->GetAFKWarningSent())
                        {
                            pPlayer->SetAFKWarningSent(true);
                            CBufferFixed<1> buffer;
                            buffer << GAME_CMD_AFK_WARNING_MESSAGE;
                            SendGameData(pClientConnection->GetClientNum(), buffer, true);
                        }
                    }
                    else
                    {
                        pPlayer->SetAFKWarningSent(false);
                    }
                }
            }
        }

        if (pPlayer->HasFlags(PLAYER_FLAG_LOADING))
            pPlayer->SetLoadingProgress(pClientConnection->GetLoadingProgress());

        if (pClientConnection->GetState() == CLIENT_CONNECTION_STATE_IN_GAME && pPlayer->HasFlags(PLAYER_FLAG_LOADING))
            pPlayer->FinishedLoading(GetGameTime(), sv_maxDisconnectedTime);

        if (!m_pHostServer->GetPaused())
        {
            // Track play time
            if (GetGamePhase() == GAME_PHASE_ACTIVE)
                pPlayer->AddPlayTime(GetFrameLength());
        }

        // Update ping
        // TODO: might want to average these out or something so it's a little smoother
        if (m_pHostServer->GetFrameNumber() % 100 == 0) // HACK: Easy way to limit this for now
            pPlayer->SetPing(pClientConnection->GetPing());
    }

    if (GetGamePhase() >= GAME_PHASE_PRE_MATCH && GetGamePhase() < GAME_PHASE_ENDED)
    {
        if (m_bSolo)
        {
            // Check for an entire team leaving
            CTeamInfo *pTeam1(GetTeam(TEAM_1));
            int iNumTeam1(pTeam1 != NULL ? pTeam1->GetNumActiveClients() : 0);

            CTeamInfo *pTeam2(GetTeam(TEAM_2));
            int iNumTeam2(pTeam2 != NULL ? pTeam2->GetNumActiveClients() : 0);

            if (iNumTeam1 == 0 && iNumTeam2 == 0)
            {
                // Fully terminate the game immediately with no stats and no replay
                ReplayManager.StopRecording();

                if (!sv_autosaveReplay && FileManager.Exists(ReplayManager.GetReplayFilename()))
                    FileManager.Delete(ReplayManager.GetReplayFilename());

                m_mapClients.clear();
                m_pServerEntityDirectory->Clear();

                Reset();

                return;
            }
        }
        else if (!m_bEmpty)
        {
            if (!HasFlags(GAME_FLAG_CONCEDED))
            {
                // Check for an entire team leaving
                CTeamInfo *pTeam1(GetTeam(TEAM_1));
                int iNumTeam1(pTeam1 != NULL ? pTeam1->GetNumActiveClients() : 0);
                if (iNumTeam1 == 0)
                    Concede(TEAM_1);

                CTeamInfo *pTeam2(GetTeam(TEAM_2));
                int iNumTeam2(pTeam2 != NULL ? pTeam2->GetNumActiveClients() : 0);
                if (iNumTeam2 == 0)
                    Concede(TEAM_2);

                // Check for enough players leaving that the game is "abandoned"
                if (pTeam1 != NULL && iNumTeam2 - iNumTeam1 >= 2)
                    pTeam1->Abandoned();

                if (pTeam2 != NULL && iNumTeam1 - iNumTeam2 >= 2)
                    pTeam2->Abandoned();
            }
        }

#ifndef K2_CLIENT
        if (m_bSentTrialGame == false)// && Game.GetGameTime() > 300000) // After ~5 mins send trial accounts to increase on the server and client.
        {
            SendTrialGameIncrease();
            m_bSentTrialGame = true;
        }
#endif

        VoteFrame();
    }

    if (!m_pHostServer->GetPaused())
    {
        bool bContinue(true);
        
        ScriptFrame();

        if (GetGamePhase() < GAME_PHASE_PRE_MATCH)
            bContinue = SetupFrame();
        else if (GetGamePhase() < GAME_PHASE_ENDED)
            bContinue = ActiveFrame();
        else if (GetGamePhase() == GAME_PHASE_ENDED)
            bContinue = EndedFrame();

        if (!bContinue)
            return;

        m_bAllowedUnpause = false;
    }

    // Send delayed messages
    for (vector<SDelayedMessage>::iterator it(m_vDelayedMessages.begin()); it != m_vDelayedMessages.end(); )
    {
        if (it->uiSendTime <= Game.GetGameTime())
        {
            if (it->uiTargetClient == -1)
                BroadcastGameData(it->cBuffer, it->bReliable);
            else
                SendGameData(it->uiTargetClient, it->cBuffer, it->bReliable);

            it = m_vDelayedMessages.erase(it);
        }
        else
            ++it;
    }

#ifndef K2_CLIENT
    // Update voice over IP lane targets
    UpdateVoIP();
#endif

    // Prevent Tournament Rules games from ever being public games
    if (GetGamePhase() <= GAME_PHASE_WAITING_FOR_PLAYERS && HasGameOptions(GAME_OPTION_TOURNAMENT_RULES))
        m_pHostServer->SetForceInviteOnly(true);
    else
        m_pHostServer->SetForceInviteOnly(false);

    // Update misc game info
    pGameInfo->SetServerAccess(m_pHostServer->GetServerAccess());
    pGameInfo->SetHostFlags(m_pHostServer->GetHostFlags());
}


/*====================
  CGameServer::AllPlayersReady
  ====================*/
bool    CGameServer::AllPlayersReady() const
{
    for (PlayerMap_cit it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
    {
        if (it->second->IsDisconnected() || it->second->GetTeam() < TEAM_1 || it->second->GetTeam() > TEAM_2)
            continue;

        if (!it->second->HasFlags(PLAYER_FLAG_READY))
            return false;
    }

    return true;
}


/*====================
  CGameServer::AllPlayersFullyConnected
  ====================*/
bool    CGameServer::AllPlayersFullyConnected() const
{
    uint uiTeam1Count(0);
    uint uiTeam2Count(0);

    for (PlayerMap_cit it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
    {
        CPlayer *pPlayer(it->second);
        if (pPlayer->IsDisconnected() || pPlayer->HasFlags(PLAYER_FLAG_LOADING))
            continue;

        if (pPlayer->GetTeam() == TEAM_1)
            ++uiTeam1Count;
        if (pPlayer->GetTeam() == TEAM_2)
            ++uiTeam2Count;
    }

    if (uiTeam1Count == GetTeamSize() && uiTeam2Count == GetTeamSize())
        return true;

    return false;
}


/*====================
  CGameServer::GetClientCount
  ====================*/
uint    CGameServer::GetClientCount(int iTeam)
{
    if (iTeam == -1)
        return GetConnectedClientCount();

    CTeamInfo *pTeam(GetTeam(iTeam));
    if (pTeam == NULL)
        return 0;

    return pTeam->GetNumClients();
}


/*====================
  CGameServer::GetConnectedClientCount
  ====================*/
int     CGameServer::GetConnectedClientCount(int iTeam)
{
    if (iTeam == -1)
    {
        int iNumClients(0);

        for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); it++)
            if (!it->second->IsDisconnected())
                iNumClients++;

        return iNumClients;
    }

    CTeamInfo *pTeam(GetTeam(iTeam));
    if (pTeam == NULL)
        return 0;

    return pTeam->GetNumClients();
}


/*====================
  CGameServer::StartReplayRecording
  ====================*/
bool    CGameServer::StartReplayRecording()
{
    CGameInfo *pGameInfo(GetGameInfo());
    if (pGameInfo == NULL)
        return false;

    if (!sv_autosaveReplay && pGameInfo->GetMatchID() == -1)
        return false;

    tstring sName(_T("~/replays/"));
    if (pGameInfo->GetMatchID() == -1)
    {
        int iLocalMatch(0);

        do
        {
            if (iLocalMatch == 0)
            {
                sName += _T("Local") _T(" - ") + pGameInfo->GetGameName();
                ++iLocalMatch;
            }
            else
            {
                sName += _T("Local") _T(" - ") + pGameInfo->GetGameName() + _T(" ") + ParenStr(XtoA(iLocalMatch++));
            }
        }
        while (FileManager.Exists(sName + _T(".honreplay")) || FileManager.Exists(sName + _T(".tmp")));
    }
    else
    {
        sName += _T("M") + XtoA(pGameInfo->GetMatchID()) /*+ _T(" - ") + pGameInfo->GetGameName()*/;
    }
    
    ReplayManager.StartRecording(sName + _T(".honreplay"));
    return true;
}


/*====================
  CGameServer::RequestStartGame
  ====================*/
void    CGameServer::RequestStartGame(int iClientNum)
{
    if (GetGamePhase() != GAME_PHASE_WAITING_FOR_PLAYERS)
        return;

    if (!m_bLocal && !m_bSolo && iClientNum != -1 && !sv_allowSolo)
    {
        CTeamInfo *pTeam1(GetTeam(TEAM_1));
        int iNumTeam1(pTeam1 != NULL ? pTeam1->GetNumClients() : 0);

        CTeamInfo *pTeam2(GetTeam(TEAM_2));
        int iNumTeam2(pTeam2 != NULL ? pTeam2->GetNumClients() : 0);

        if (iNumTeam1 == 0 || iNumTeam2 == 0)
        {
            CBufferDynamic buffer;
            buffer << GAME_CMD_GAME_MESSAGE << TStringToUTF8(_CWS("nosolo")) << byte(0);
            SendGameData(iClientNum, buffer, true);

            return;
        }

        // when we've hosted a tournament rules game, scan for any player
        // using custom files (mods) and don't allow the game to start.
        if (HasGameOptions(GAME_OPTION_TOURNAMENT_RULES))
        {
            // build a list of players who are using custom files.
            int iPlayerList[64];
            int iSize(0);

            for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
            {
                CPlayer *pPlayer(it->second);
                if (pPlayer == NULL)
                    continue;

                if (pPlayer->GetTeam() < TEAM_1 || pPlayer->GetTeam() > TEAM_2)
                    continue;

                if (pPlayer->HasNotificationFlags(NOTIFY_FLAG_CUSTOM_FILES))
                {
                    iPlayerList[iSize] = pPlayer->GetClientNumber();
                    ++iSize;
                    if (iSize == 64)
                        break;
                }
            }

            if (iSize > 0)
            {
                SendGeneralMessage(_CWS("nomods"), iSize, iPlayerList);
                return;
            }
        }

    }
    
    SetGamePhaseEndTime(GetGameTime() + sv_gameStartCountdown);
}


/*====================
  CGameServer::StartGame
  ====================*/
void    CGameServer::StartGame(bool bAllowSolo, bool bAllowEmpty)
{
    if (GetGamePhase() >= GAME_PHASE_HERO_BAN)
        return;

    // Kick unassigned players
    for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
    {
        CPlayer *pPlayer(it->second);
        if (pPlayer == NULL)
            continue;

        CClientConnection *pClientConnection(m_pHostServer->GetClient(pPlayer->GetClientNumber()));
        if (pClientConnection == NULL)
            continue;

        if (pPlayer->GetTeam() == TEAM_INVALID || pPlayer->GetTeam() == TEAM_PASSIVE)
        {
            pClientConnection->Disconnect(_T("disconnect_unassigned"));
            continue;
        }
    }

    // Teams shuffle/autobalance (before phase update, since ChangeTeam() checks this)
    if (HasGameOptions(GAME_OPTION_AUTOBALANCE_TEAMS))
    {
        for (int iTeam(TEAM_1); iTeam <= TEAM_2; ++iTeam)
        {
            CTeamInfo *pTeam(GetTeam(iTeam));
            if (pTeam == NULL)
                continue;

            for (uint uiSlot(0); uiSlot < GetTeamSize(); ++uiSlot)
            {
                CPlayer *pPlayer(pTeam->GetPlayer(uiSlot));
                if (pPlayer == NULL)
                    continue;

                pTeam->UnlockSlot(uiSlot);
            }
        }                   
        
        BalanceTeams(true);
    }
    else if (HasGameOptions(GAME_OPTION_SHUFFLE_TEAMS))
    {
        vector<CPlayer*> vClients;
        for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
        {
            if (it->second->GetTeam() >= TEAM_1 && it->second->GetTeam() <= TEAM_2)
                vClients.push_back(it->second);
        }

        std::random_shuffle(vClients.begin(), vClients.end());
        for (vector<CPlayer*>::iterator it(vClients.begin()); it != vClients.end(); ++it)
            ChangeTeam((*it)->GetClientNumber(), TEAM_INVALID);

        int i(0);
        for (vector<CPlayer*>::iterator it(vClients.begin()); it != vClients.end(); ++it, ++i)
            ChangeTeam((*it)->GetClientNumber(), (i & 1) + 1);
    }   
    
    // Reveal secret info for each client
    for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
        it->second->RevealSecretInfo();

    // Update prediction and point values
    CTeamInfo *pTeam1(GetTeam(TEAM_1));
    CTeamInfo *pTeam2(GetTeam(TEAM_2));
    if (pTeam1 != NULL && pTeam2 != NULL)
    {
        int iRankDiff(pTeam1->GetRank() - pTeam2->GetRank());
        float fWinProbability(1.0f / (1.0f + pow(M_E, -iRankDiff / psf_logisticPredictionScale)));
        
        pTeam1->SetWinChance(fWinProbability);
        pTeam2->SetWinChance(1.0f - fWinProbability);
    }

    uint uiPlayerCount(0);
    m_fAverageRating = 0.0f;
    m_fTeamAverageRating[0] = 0.0f;
    m_fTeamAverageRating[1] = 0.0f;

    // Lock each players win/loss point values
    for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
    {
        it->second->StoreMatchPointValues();

        if (it->second->GetTeam() > TEAM_SPECTATOR && it->second->GetTeam() < TEAM_INVALID)
        {
            ++uiPlayerCount;
            m_fAverageRating += it->second->GetRank();
            if (it->second->GetTeam() == TEAM_1)
                m_fTeamAverageRating[0] += it->second->GetRank();
            if (it->second->GetTeam() == TEAM_2)
                m_fTeamAverageRating[1] += it->second->GetRank();
        }
    }

    if (uiPlayerCount > 0)
    {
        m_fAverageRating /= uiPlayerCount;
        m_fTeamAverageRating[0] /= uiPlayerCount;
        m_fTeamAverageRating[1] /= uiPlayerCount;
    }

    // Pick captains
    for (int iTeam(TEAM_1); iTeam <= TEAM_2; ++iTeam)
    {
        CTeamInfo *pTeam(GetTeam(iTeam));
        if (pTeam == NULL)
            continue;

        for (uint uiSlot(0); uiSlot < GetTeamSize(); ++uiSlot)
        {
            CPlayer *pPlayer(pTeam->GetPlayer(uiSlot));
            if (pPlayer == NULL)
                continue;

            pPlayer->SetFlags(PLAYER_FLAG_IS_CAPTAIN);
            break;
        }
    }

    int iStartingTeam(M_Randnum(TEAM_1, TEAM_2));

        // assign first ban team.
    CGameInfo *pGameInfo(GetGameInfo());
    if (pGameInfo != NULL)
    {
        if ((GetBanCount() > 0) &&
            (pGameInfo->GetGameOptions() & GAME_OPTION_TOURNAMENT_RULES))
        {
            iStartingTeam = pGameInfo->GetFirstBanTeam();
        }
    }

    if (GetBanCount() > 0)
    {
        m_uiBanningTeam = iStartingTeam;
        m_iBanRound = -1;
    }

    // Update phase and assign draft order
    if (GetAlternatePicks())
    {
        // Team A gets first pick, but only one hero
        CTeamInfo *pTeam(GetTeam(iStartingTeam));
        if (pTeam != NULL)
        {
            uint uiDraftRound(1);
            uint uiCount(0);
            for (uint uiSlot(0); uiSlot < GetTeamSize(); ++uiSlot)
            {
                CPlayer *pPlayer(pTeam->GetPlayer(uiSlot));
                if (pPlayer == NULL)
                    continue;

                pPlayer->SetDraftRound(uiDraftRound);
                m_uiFinalDraftRound = MAX(m_uiFinalDraftRound, uiDraftRound);
                ++uiCount;

                if (uiCount % 2 != 0)
                    uiDraftRound += 2;
            }
        }

        // Team B always gets last pick
        pTeam = GetTeam(iStartingTeam ^ 3);
        if (pTeam != NULL)
        {
            uint uiDraftRound(2);
            uint uiCount(0);
            uint uiNumPlayers(pTeam->GetNumClients());
            for (uint uiSlot(0); uiSlot < GetTeamSize(); ++uiSlot)
            {
                CPlayer *pPlayer(pTeam->GetPlayer(uiSlot));
                if (pPlayer == NULL)
                    continue;

                pPlayer->SetDraftRound(uiDraftRound);
                m_uiFinalDraftRound = MAX(m_uiFinalDraftRound, uiDraftRound);
                ++uiCount;

                if (uiCount % 2 == 0 || (uiNumPlayers % 2 == 0 && uiCount == uiNumPlayers - 1))
                    uiDraftRound += 2;
            }
        }
    }
    else
    {
        SetFlags(GAME_FLAG_FINAL_HERO_SELECT);
        
        // Everyone can pick right away if it is not an alternating pick mode
        for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
            it->second->SetFlags(PLAYER_FLAG_CAN_PICK);
    }

    // Set initial phase and time
    if (pGameInfo == NULL || !pGameInfo->GetNoHeroSelect())
    {
        if (GetBanCount() > 0)
        {
            if (GetGameMode() == GAME_MODE_BANNING_PICK || GetGameMode() == GAME_MODE_CAPTAINS_MODE)
                SetGamePhase(GAME_PHASE_HERO_BAN, sv_heroAllBanInitialTime);
            else
                SetGamePhase(GAME_PHASE_HERO_BAN, sv_heroBanInitialTime);
        }
        else
        {
            if (GetAlternatePicks())
                SetGamePhase(GAME_PHASE_HERO_SELECT, sv_heroAltSelectInitialTime);
            else
                SetGamePhase(GAME_PHASE_HERO_SELECT, GetGameMode() == GAME_MODE_SINGLE_DRAFT ? sv_heroSelectTimeSD : sv_heroSelectTime);
        }
    }
    else
    {
        SetGamePhase(GAME_PHASE_PRE_MATCH);
    }

    // Set extra time
    for (int iTeam(TEAM_1); iTeam <= TEAM_2; ++iTeam)
    {
        CTeamInfo *pTeam(GetTeam(iTeam));
        if (pTeam == NULL)
            continue;

        if (GetGameMode() == GAME_MODE_BANNING_PICK || GetGameMode() == GAME_MODE_CAPTAINS_MODE)
            pTeam->SetExtraTime(sv_heroAllBanExtraTime);
        else
            pTeam->SetExtraTime(0);

        pTeam->SetUsingExtraTime(false);
    }

    if (bAllowSolo)
    {
        CTeamInfo *pTeam1(GetTeam(TEAM_1));
        int iNumTeam1(pTeam1 != NULL ? pTeam1->GetNumClients() : 0);

        CTeamInfo *pTeam2(GetTeam(TEAM_2));
        int iNumTeam2(pTeam2 != NULL ? pTeam2->GetNumClients() : 0);

        if (iNumTeam1 == 0 || iNumTeam2 == 0 && iNumTeam1 != iNumTeam2)
        {
            m_bSolo = true;

            CGameInfo *pGameInfo(GetGameInfo());
            if (pGameInfo != NULL)
                pGameInfo->SetFlags(GAME_FLAG_SOLO);
        }
    }

    if (bAllowEmpty)
    {
        CTeamInfo *pTeam1(GetTeam(TEAM_1));
        int iNumTeam1(pTeam1 != NULL ? pTeam1->GetNumClients() : 0);

        CTeamInfo *pTeam2(GetTeam(TEAM_2));
        int iNumTeam2(pTeam2 != NULL ? pTeam2->GetNumClients() : 0);

        if (iNumTeam1 == 0 && iNumTeam2 == 0)
        {
            m_bSolo = false;
            m_bEmpty = true;

            CGameInfo *pGameInfo(GetGameInfo());
            if (pGameInfo != NULL)
                pGameInfo->SetFlags(GAME_FLAG_SOLO);
        }
    }

    m_pServerEntityDirectory->GameStart();

    m_pHostServer->StartMatch();

    // Remove private flag so that people can rejoin
    m_pHostServer->SetServerAccess(ACCESS_PUBLIC);

    StartReplayRecording();
}


/*====================
  CGameServer::StartPreMatch
  ====================*/
void    CGameServer::StartPreMatch()
{
    if (GetGamePhase() >= GAME_PHASE_PRE_MATCH)
        return;

    for (int iTeam(TEAM_1); iTeam <= TEAM_2; ++iTeam)
    {
        CTeamInfo *pTeam(GetTeam(iTeam));
        if (pTeam == NULL)
            continue;

        pTeam->SetStat(TEAM_STAT_TOWER_KILLS, 0);
        pTeam->SetStat(TEAM_STAT_TOWER_DENIES, 0);
    }

    m_vMatchKillLog.clear();

    for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
    {
        CPlayer* pPlayer(it->second);

        // All players without a hero selected for whatever reason get a random hero
        if (!pPlayer->HasSelectedHero())
            SelectPotentialHeroOrRandomHero(it->first);

        pPlayer->SpawnHero();
        pPlayer->SetLastInteractionTime(GetGameTime());
    }

    SetGamePhase(GAME_PHASE_PRE_MATCH, sv_preMatchTime);
}


/*====================
  CGameServer::StartMatch
  ====================*/
void    CGameServer::StartMatch()
{
    if (GetGamePhase() >= GAME_PHASE_ACTIVE)
        return;

    if (Game.GetWorldPointer() == NULL || !Game.GetWorldPointer()->IsLoaded())
        return;

    // Make sure all players can reconnect
    for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
        m_pHostServer->GenerateInvitation(it->second->GetAccountID());

    m_pServerEntityDirectory->MatchStart();

    SetGamePhase(GAME_PHASE_ACTIVE);
    
    m_GameLog.WriteStatus(GAME_LOG_GAME_START);

    m_uiLastStatusNotifyTime = GetGameTime();
    m_uiLastChatCounterDecrement = GetTotalTime();

    m_uiNextTreeSpawnTime = GetMatchTime() + g_treeSpawnInterval;

    m_uiLastCreepUpgradeLevel = 0;
    m_uiCreepWaveCount = 0;
    if (sv_spawnCreeps)
        SpawnCreeps();
    m_uiNextCreepWaveTime = GetMatchTime() + g_creepWaveInterval;

    if (sv_spawnPowerups && !Game.HasGameOptions(GAME_OPTION_NO_POWERUPS))
        SpawnPowerup();
    m_uiNextPowerupSpawnTime = GetMatchTime() + g_powerupSpawnInterval;
    
    if (sv_spawnCritters)
        SpawnCritters();
    m_uiNextCritterSpawnTime = GetMatchTime() + g_critterSpawnInterval;
}


/*====================
  CGameServer::EndMatch
  ====================*/
void    CGameServer::EndMatch(int iLosingTeam)
{
    if (Game.GetGamePhase() != GAME_PHASE_ACTIVE && Game.GetGamePhase() != GAME_PHASE_PRE_MATCH)
        return;

    if (iLosingTeam <= 0 || iLosingTeam >= 3)
    {
        Console.Warn << _T("EndMatch: Invalid team, game will not end: ") << iLosingTeam << newl;
        return;
    }

    uint uiMatchTime(Game.GetMatchTime());

    CGameInfo *pGameInfo(GetGameInfo());
    if (pGameInfo != NULL && !pGameInfo->HasFlags(GAME_FLAG_CONCEDED))
    {
        pGameInfo->SetMatchLength(Game.GetMatchTime());

        m_pServerEntityDirectory->FlushStats();

        SetWinningTeam(iLosingTeam ^ 3);
        SetFinalMatchTime(uiMatchTime);
        m_GameLog.WriteStatus(GAME_LOG_GAME_END, _T("winner"), XtoA(GetWinningTeam()));
        m_sGameLogPath = FileManager.GetGamePath(m_GameLog.GetPath());
        m_GameLog.Close();

        ParseGameLog(m_sGameLogPath);
    }

    Game.SetGamePhase(GAME_PHASE_ENDED, sv_gameEndPhaseTime);

    // Notify clients who the winner is
    if (pGameInfo == NULL || !pGameInfo->HasFlags(GAME_FLAG_CONCEDED))
    {
        CBufferFixed<9> buffer;
        buffer << GAME_CMD_END_GAME << GetWinningTeam() << uiMatchTime;
        BroadcastGameData(buffer, true);
    }

    // Force base building to die
    CTeamInfo *pLosingTeam(GetTeam(iLosingTeam));
    if (pLosingTeam != NULL)
    {
        IBuildingEntity *pTargetBuilding(GetBuildingEntity(pLosingTeam->GetBaseBuildingIndex()));
        if (pTargetBuilding != NULL)
            pTargetBuilding->Kill();
    }

    // Delete projectiles and affectors
    IGameEntity *pEnt(Game.GetFirstEntity());
    while (pEnt != NULL)
    {
        if (pEnt != NULL)
        {   
            if (pEnt->IsProjectile() || pEnt->IsAffector())
                DeleteEntity(pEnt);
            else if (pEnt->IsUnit() && !pEnt->IsBuilding())
                pEnt->GetAsUnit()->StopAnimation(-1);
        }

        pEnt = Game.GetNextEntity(pEnt);
    }
}


/*====================
  CGameServer::Remake
  ====================*/
void    CGameServer::Remake()
{
    // Remove disconnected/terminated clients
    PlayerMap_it it(m_mapClients.begin());
    while (it != m_mapClients.end())
    {
        if (it->second->IsDisconnected() || it->second->HasFlags(PLAYER_FLAG_TERMINATED))
        {
            m_pHostServer->KickClient(it->first, _T("disconnect_game_over"));
            DeleteEntity(it->second);
            STL_ERASE(m_mapClients, it);
            continue;
        }

        ++it;
    }

    SetWinningTeam(TEAM_INVALID);
    SetFinalMatchTime(INVALID_TIME);

    m_uiCreepWaveCount = 0;
    m_uiLastCreepUpgradeLevel = 0;
    m_uiNextCreepWaveTime = INVALID_TIME;
    m_uiNextTreeSpawnTime = INVALID_TIME;
    m_uiNextPowerupSpawnTime = INVALID_TIME;
    m_uiNextCritterSpawnTime = INVALID_TIME;
    m_bStartAnnounced = false;
    m_bHasForcedRandom = false;
    m_iActiveTeam = TEAM_INVALID;

    m_cVisibilityMap[0].Clear();
    m_cVisibilityMap[1].Clear();

    m_vNeutralCamps.clear();
    m_vKongors.clear();
    
    m_uiPowerupUID = INVALID_INDEX;
    m_uiLastVisibilityUpdate = INVALID_TIME;

    m_GameLog.Close();

    // Reset hero lists
    for (uint ui(0); ui < NUM_HERO_LISTS; ++ui)
        m_vHeroLists[ui].clear();

    for (ushort unBlock(STATE_BLOCK_FIRST_HERO_GROUP); unBlock <= STATE_BLOCK_LAST_HERO_GROUP; ++unBlock)
    {
        CStateBlock &block(m_pHostServer->GetStateBlock(unBlock));
        IBuffer &buffer(block.GetBuffer());
        buffer.Clear();
        block.Modify();
    }

    BuildHeroLists();

    m_uiDraftRound = 0;
    m_uiFinalDraftRound = 0;
    m_iBanRound = -1;
    m_uiBanningTeam = TEAM_INVALID;

    // Delete or reset all entities
    IGameEntity *pEnt(Game.GetFirstEntity());
    while (pEnt != NULL)
    {
        if (pEnt->IsPlayer() || pEnt->IsGameInfo() || pEnt->IsTeamInfo())
        {
            pEnt->MatchRemake();
            pEnt = Game.GetNextEntity(pEnt);
            continue;
        }
        
        uint uiIndex(pEnt->GetIndex());
        pEnt = Game.GetNextEntity(pEnt);
        m_pServerEntityDirectory->Delete(uiIndex);
    }

    m_pServerEntityDirectory->ClearBitEntities();

    // Initialize team visibility maps
    int iSize(1 << int(g_fogofwarSize));

    m_uiVisibilitySize = iSize;
    m_fVisibilityScale = m_uiVisibilitySize * GetWorldPointer()->GetScale(); 
    m_cVisibilityMap[0].Initialize(GetWorldPointer()->GetTileWidth() / m_uiVisibilitySize, GetWorldPointer()->GetTileHeight() / m_uiVisibilitySize);
    m_cVisibilityMap[1].Initialize(GetWorldPointer()->GetTileWidth() / m_uiVisibilitySize, GetWorldPointer()->GetTileHeight() / m_uiVisibilitySize);

    ClearWaterMarkers();

    ResetWorld();

    // Reinitialize shop info
    RegisterShopInfo();

    // Spawn game entities for each world entity that requires one
    WorldEntList &vWorldEnts(GetWorldEntityList());
    for (WorldEntList_it it(vWorldEnts.begin()), itEnd(vWorldEnts.end()); it != itEnd; ++it)
    {
        if (*it == INVALID_POOL_HANDLE)
            continue;

        CWorldEntity *pWorldEntity(GetWorldPointer()->GetEntityByHandle(*it));
        if (pWorldEntity == NULL)
            continue;

        if (pWorldEntity->GetType() == _T("Prop_Cliff"))
        {
            SpawnCliff(pWorldEntity);
            continue;
        }

        if (pWorldEntity->GetType() == _T("Prop_Cliff2"))
        {
            SpawnCliff(pWorldEntity);
            continue;
        }

        if (pWorldEntity->GetType() == _T("Prop_Water"))
        {
            SpawnWater(pWorldEntity);
            continue;
        }

        if (pWorldEntity->GetType() == _T("Prop_Static"))
        {
            SpawnStaticProp(pWorldEntity);
            continue;
        }

        if (pWorldEntity->GetPropertyBool(_CTS("dormant")))
        {
            Precache(pWorldEntity->GetType(), PRECACHE_ALL);
            continue;
        }

        IGameEntity* pNewEnt(m_pServerEntityDirectory->Allocate(pWorldEntity->GetType()));
        if (pNewEnt == NULL)
        {
            Console.Err << _T("Failed to allocate a game entity for world entity #") + XtoA(pWorldEntity->GetIndex()) << _T(" type: ") << pWorldEntity->GetType() << newl;
            continue;
        }

        pWorldEntity->SetGameIndex(pNewEnt->GetIndex());
        pNewEnt->ApplyWorldEntity(*pWorldEntity);
        Precache(pNewEnt->GetType(), PRECACHE_ALL);
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
    }

    m_pServerEntityDirectory->Spawn();

    m_pServerEntityDirectory->WriteBitEntityMap(m_pHostServer->GetStateBlock(STATE_BLOCK_BIT));
    for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
        m_pHostServer->SendStateBlock(it->first, STATE_BLOCK_BIT);

    m_vNeutralCamps.clear();
    m_vKongors.clear();

    Game.GetEntities(m_vNeutralCamps, Entity_NeutralCampController);
    Game.GetEntities(m_vKongors, Entity_BossController);

    m_pHostServer->RestartMatch();
}


/*====================
  CGameServer::Concede
  ====================*/
void    CGameServer::Concede(int iLosingTeam)
{
    if (Game.GetGamePhase() != GAME_PHASE_ACTIVE && Game.GetGamePhase() != GAME_PHASE_PRE_MATCH)
        return;

    if (iLosingTeam <= 0 || iLosingTeam >= 3)
    {
        Console.Warn << _T("EndMatch: Invalid team, game will not end: ") << iLosingTeam << newl;
        return;
    }

    uint uiMatchTime(GetMatchTime());

    CGameInfo *pGameInfo(GetGameInfo());

    if (pGameInfo != NULL)
    {
        pGameInfo->SetMatchLength(Game.GetMatchTime());
        pGameInfo->SetFlags(GAME_FLAG_CONCEDED);
    }

    CBufferFixed<5> buffer;
    buffer << GAME_CMD_CONCEDE_MESSAGE << iLosingTeam;
    BroadcastGameData(buffer, true);

    SetWinningTeam(iLosingTeam ^ 3);
    m_pServerEntityDirectory->FlushStats();

    // Write concession to log, so stats beyond this will not be counted
    m_GameLog.WriteStatus(GAME_LOG_GAME_CONCEDE, _T("winner"), XtoA(GetWinningTeam()));

    // Close game log and parse it, game stats after this point are ignored
    m_sGameLogPath = FileManager.GetGamePath(m_GameLog.GetPath());
    m_GameLog.Close();
    ParseGameLog(m_sGameLogPath);

    CBufferFixed<9> buffer2;
    buffer2 << GAME_CMD_END_GAME << GetWinningTeam() << uiMatchTime;
    BroadcastGameData(buffer2, true);
}


/*====================
  CGameServer::Reset
  ====================*/
void    CGameServer::Reset(bool bFailed)
{
    ReplayManager.StopRecording();

    SetGameTime(INVALID_TIME);

    ClearEventList();

    ClearTeams();

    SetWinningTeam(TEAM_INVALID);
    SetFinalMatchTime(INVALID_TIME);

    ////

    m_pServerEntityDirectory->Clear();
    m_GameLog.Close();
    m_pHostServer->EndGame(m_sReplayHost, m_sReplayDir, ReplayManager.GetReplayFilename(), m_sGameLogPath, bFailed);
    m_mapClients.clear();
    m_sGameLogPath.clear();

    ClearGlobalModifiers();
    SetGameInfo(NULL);

    m_uiLastStatusNotifyTime = INVALID_TIME;
    m_uiLastChatCounterDecrement = INVALID_TIME;

    for (uint ui(0); ui < NUM_HERO_LISTS; ++ui)
        m_vHeroLists[ui].clear();

    m_uiCreepWaveCount = 0;
    m_uiLastCreepUpgradeLevel = 0;
    m_uiNextCreepWaveTime = INVALID_TIME;
    m_uiNextTreeSpawnTime = INVALID_TIME;
    m_uiNextPowerupSpawnTime = INVALID_TIME;
    m_uiNextCritterSpawnTime = INVALID_TIME;
    m_bSentTrialGame = false;
    m_bStartAnnounced = false;
    m_bHasForcedRandom = false;
    m_iActiveTeam = TEAM_INVALID;
    m_uiDraftRound = 0;
    m_uiFinalDraftRound = 0;
    m_iBanRound = -1;
    m_uiBanningTeam = TEAM_INVALID;
    m_bSolo = false;
    m_bEmpty = false;
    m_bLocal = false;
    m_fAverageRating = 0.0f;
    m_fTeamAverageRating[0] = 0.0f;
    m_fTeamAverageRating[1] = 0.0f;

    m_cVisibilityMap[0].Clear();
    m_cVisibilityMap[1].Clear();

    m_vNeutralCamps.clear();
    m_vKongors.clear();

    m_uiPowerupUID = INVALID_INDEX;
    m_uiLastVisibilityUpdate = INVALID_TIME;

    m_pServerEntityDirectory->WriteBitEntityMap(m_pHostServer->GetStateBlock(STATE_BLOCK_BIT));

    for (ushort unBlock(STATE_BLOCK_FIRST_HERO_GROUP); unBlock <= STATE_BLOCK_LAST_HERO_GROUP; ++unBlock)
    {
        CStateBlock &block(m_pHostServer->GetStateBlock(unBlock));
        IBuffer &buffer(block.GetBuffer());
        buffer.Clear();
        block.Modify();
    }

    m_sName.clear();
    m_iCreatorClientNum = -1;
    m_uiPauseToggleTime = INVALID_TIME;

    m_vGuests.clear();
}


/*====================
  ComparePlayerRank
  ====================*/
bool    ComparePlayerRank(CPlayer *pPlayerA, CPlayer *pPlayerB)
{
    if (pPlayerA == NULL)
        return false;

    if (pPlayerB == NULL)
        return true;

    if (pPlayerA->GetRank() > pPlayerB->GetRank())
        return true;

    return false;
}


/*====================
  CGameServer::BalanceTeams
  ====================*/
void    CGameServer::BalanceTeams(const bool bAutoBalanceMode)
{
    CTeamInfo *pTeamA(GetTeam(TEAM_1));
    CTeamInfo *pTeamB(GetTeam(TEAM_2));
    if (pTeamA == NULL || pTeamB == NULL)
        return;

    // Get a list of all players on team, sorted highest to lowest
    ivector vPlayerPool;
    const ivector &vTeamRosterA(pTeamA->GetClientList());
    vPlayerPool.insert(vPlayerPool.end(), vTeamRosterA.begin(), vTeamRosterA.end());
    const ivector &vTeamRosterB(pTeamB->GetClientList());
    vPlayerPool.insert(vPlayerPool.end(), vTeamRosterB.begin(), vTeamRosterB.end());

    deque<CPlayer*> deqPlayers;
    for (ivector_it it(vPlayerPool.begin()); it != vPlayerPool.end(); ++it)
    {
        CPlayer *pPlayer(GetPlayer(*it));
        if (pPlayer == NULL)
            continue;

        CTeamInfo *pTeam(GetTeam(pPlayer->GetTeam()));
        if (pTeam == NULL || pTeam->IsSlotLocked(pTeam->GetTeamIndexFromClientID(pPlayer->GetClientNumber())))
            continue;

        deqPlayers.push_back(pPlayer);
    }

    std::sort(deqPlayers.begin(), deqPlayers.end(), ComparePlayerRank);

    // Clear the teams
    for (deque<CPlayer*>::iterator it(deqPlayers.begin()); it != deqPlayers.end(); ++it)
    {
        CTeamInfo *pTeam(GetTeam((*it)->GetTeam()));
        if (pTeam == NULL || pTeam->IsSlotLocked(pTeam->GetTeamIndexFromClientID((*it)->GetClientNumber())))
            continue;
        
        ChangeTeam((*it)->GetClientNumber(), TEAM_INVALID);
    }

    // Start assigning (first pass)
    int iCurrentTeam(M_Randnum(TEAM_1, TEAM_2));
    CTeamInfo *pCurrentTeam(GetTeam(iCurrentTeam));
    CTeamInfo *pOtherTeam(GetTeam(iCurrentTeam ^ 3));
    while (!deqPlayers.empty())
    {
        while ((pCurrentTeam->GetRank() <= pOtherTeam->GetRank() || pOtherTeam->IsFull()) && !deqPlayers.empty() && !pCurrentTeam->IsFull())
        {
            CPlayer *pPlayer(deqPlayers.front());
            deqPlayers.pop_front();

            ChangeTeam(pPlayer->GetClientNumber(), pCurrentTeam->GetTeamID());
        }

        SWAP(pCurrentTeam, pOtherTeam);
    }

    // Make a pass at each movable player and look for improvements
    for (uint uiTeamA(0); uiTeamA < pTeamA->GetTeamSize(); ++uiTeamA)
    {
        if (pTeamA->IsSlotLocked(uiTeamA))
            continue;

        CPlayer *pPlayer(pTeamA->GetPlayer(uiTeamA));
        if (pPlayer == NULL)
            continue;

        for (uint uiTeamB(0); uiTeamB < pTeamB->GetTeamSize(); ++uiTeamB)
        {
            if (pTeamB->IsSlotLocked(uiTeamB))
                continue;

            int iOldRankDiff(pTeamA->GetRank() - pTeamB->GetRank());
            float fOldProbability(1.0f / (1.0f + pow(M_E, -iOldRankDiff / psf_logisticPredictionScale)));
            SwapPlayerSlots(pTeamA->GetTeamID(), uiTeamA, pTeamB->GetTeamID(), uiTeamB);
            
            int iNewRankDiff(pTeamA->GetRank() - pTeamB->GetRank());
            float fNewProbability(1.0f / (1.0f + pow(M_E, -iNewRankDiff / psf_logisticPredictionScale)));
            if (fabs(fOldProbability - 0.5f) < fabs(fNewProbability - 0.5f))
                SwapPlayerSlots(pTeamA->GetTeamID(), uiTeamA, pTeamB->GetTeamID(), uiTeamB);
        }
    }

    if (!bAutoBalanceMode)
    {
        CBufferFixed<1> buffer;
        buffer << GAME_CMD_LOBBY_BALANCED_MESSAGE;              
        BroadcastGameData(buffer, true);
    }
}


/*====================
  CGameServer::SwapPlayerSlots
  ====================*/
void    CGameServer::SwapPlayerSlots(int iTeamA, uint uiSlotA, int iTeamB, uint uiSlotB)
{
    if (GetGamePhase() != GAME_PHASE_WAITING_FOR_PLAYERS)
        return;

    CTeamInfo *pTeamA(GetTeam(iTeamA));
    CTeamInfo *pTeamB(GetTeam(iTeamB));
    if (pTeamA == NULL || pTeamB == NULL)
        return;

    CPlayer *pPlayerA(pTeamA->GetPlayer(uiSlotA));
    CPlayer *pPlayerB(pTeamB->GetPlayer(uiSlotB));

    if (pPlayerA != NULL)
        ChangeTeam(pPlayerA->GetClientNumber(), TEAM_INVALID);
    if (pPlayerB != NULL)
        ChangeTeam(pPlayerB->GetClientNumber(), iTeamA, uiSlotA);
    if (pPlayerA != NULL)
        ChangeTeam(pPlayerA->GetClientNumber(), iTeamB, uiSlotB);
}


/*====================
  CGameServer::ForceSwapPlayerSlots
  ====================*/
void    CGameServer::ForceSwapPlayerSlots(int iTeamA, uint uiSlotA, int iTeamB, uint uiSlotB)
{
    if (GetGamePhase() != GAME_PHASE_WAITING_FOR_PLAYERS)
        return;

    if (iTeamA == iTeamB && uiSlotA == uiSlotB)
        return;
    if (iTeamA < TEAM_1 || iTeamA > TEAM_2 || iTeamB < TEAM_1 || iTeamB > TEAM_2)
        return;
    
    CTeamInfo *pTeamA(GetTeam(iTeamA));
    CTeamInfo *pTeamB(GetTeam(iTeamB));
    if (pTeamA == NULL || pTeamB == NULL)
        return;

    if (uiSlotA >= pTeamA->GetTeamSize() ||
        uiSlotB >= pTeamB->GetTeamSize())
        return;

    // Stop countdown
    if (GetGamePhase() == GAME_PHASE_WAITING_FOR_PLAYERS)
        SetGamePhaseEndTime(INVALID_TIME);

    bool bSlotALocked(pTeamA->IsSlotLocked(uiSlotA));
    pTeamA->UnlockSlot(uiSlotA);
    bool bSlotBLocked(pTeamB->IsSlotLocked(uiSlotB));
    pTeamB->UnlockSlot(uiSlotB);

    CPlayer *pPlayerA(pTeamA->GetPlayer(uiSlotA));
    CPlayer *pPlayerB(pTeamB->GetPlayer(uiSlotB));

    if (pPlayerA != NULL)
        ChangeTeam(pPlayerA->GetClientNumber(), TEAM_INVALID);
    if (pPlayerB != NULL)
        ChangeTeam(pPlayerB->GetClientNumber(), iTeamA, uiSlotA);
    if (pPlayerA != NULL)
        ChangeTeam(pPlayerA->GetClientNumber(), iTeamB, uiSlotB);

    if (bSlotALocked)
        pTeamA->LockSlot(uiSlotA);
    if (bSlotBLocked)
        pTeamB->LockSlot(uiSlotB);

    CBufferFixed<17> buffer;
    buffer << GAME_CMD_LOBBY_FORCED_TEAM_SWAP_MESSAGE << iTeamA << uiSlotA << iTeamB << uiSlotB;
    BroadcastGameData(buffer, true);
}


/*====================
  CGameServer::LockSlot
  ====================*/
void    CGameServer::LockSlot(int iTeam, uint uiSlot)
{
    CTeamInfo *pTeam(GetTeam(iTeam));
    if (pTeam == NULL)
        return;

    pTeam->LockSlot(uiSlot);    
}


/*====================
  CGameServer::UnlockSlot
  ====================*/
void    CGameServer::UnlockSlot(int iTeam, uint uiSlot)
{
    CTeamInfo *pTeam(GetTeam(iTeam));
    if (pTeam == NULL)
        return;

    pTeam->UnlockSlot(uiSlot);
}


/*====================
  CGameServer::ToggleSlotLock
  ====================*/
void    CGameServer::ToggleSlotLock(int iTeam, uint uiSlot)
{
    CTeamInfo *pTeam(GetTeam(iTeam));
    if (pTeam == NULL)
        return;

    pTeam->ToggleSlotLock(uiSlot);
}


/*====================
  CGameServer::ResetPlayerVotes
  ====================*/
void    CGameServer::ResetPlayerVotes()
{
    for (PlayerMap_it it(m_mapClients.begin()), itEnd(m_mapClients.end()); it != itEnd; ++it)
        it->second->SetVote(VOTE_NONE);
}


/*====================
  CGameServer::SetupFrame
  ====================*/
bool    CGameServer::SetupFrame()
{
    PROFILE("CGameServer::SetupFrame");

    m_pServerEntityDirectory->BackgroundFrame();

    // End the game if the host disconnects
    if (GetGamePhase() <= GAME_PHASE_WAITING_FOR_PLAYERS && m_iCreatorClientNum != -1)
    {
        CClientConnection *pClient(m_pHostServer->GetClient(m_iCreatorClientNum));
        if (pClient == NULL || !pClient->IsConnected())
        {
            Reset();
            return false;
        }
    }

    for (int iTeam(TEAM_1); iTeam <= TEAM_2; ++iTeam)
    {
        CTeamInfo *pTeam(GetTeam(iTeam));
        if (pTeam == NULL)
            continue;

        // Hi-jack field to send the extra time of captains mode
        pTeam->SetStat(TEAM_STAT_TOWER_KILLS, INT_CEIL(pTeam->GetExtraTime() / 1000.0f));
        pTeam->SetStat(TEAM_STAT_TOWER_DENIES, pTeam->GetUsingExtraTime() ? 1 : 0);
    }

    // Check for transitions
    switch (GetGamePhase())
    {
    case GAME_PHASE_IDLE:
        break;

    case GAME_PHASE_WAITING_FOR_PLAYERS:
        {
            CGameInfo *pGameInfo(GetGameInfo());
            if (pGameInfo->GetNoLobby())
            {
                StartGame(true, true);
                return true;
            }

            // Update prediction and point values
            CTeamInfo *pTeam1(GetTeam(TEAM_1));
            CTeamInfo *pTeam2(GetTeam(TEAM_2));
            if (pTeam1 != NULL && pTeam2 != NULL)
            {
                int iRankDiff(pTeam1->GetRank() - pTeam2->GetRank());
                float fWinProbability(1.0f / (1.0f + pow(M_E, -iRankDiff / psf_logisticPredictionScale)));
                
                pTeam1->SetWinChance(fWinProbability);
                pTeam2->SetWinChance(1.0f - fWinProbability);
            }

            for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
                it->second->StoreMatchPointValues();

            // Check for start conditions
            if (AllPlayersFullyConnected() && (sv_gameAutoStart || m_pHostServer->IsArrangedMatch() || m_pHostServer->IsTournMatch()))
                StartGame(sv_allowSolo || m_bLocal, sv_allowEmpty);
            
            if (GetGameTime() >= GetPhaseEndTime())
            {
                // Kill an arranged match that has taken too long to start
                if (m_pHostServer->IsArrangedMatch())
                {
                    Reset();
                    return false;
                }
                
                StartGame(sv_allowSolo || m_bLocal, sv_allowEmpty);
            }

            // Send reminders to players that are not connecting to an arranged match
            if (m_pHostServer->IsArrangedMatch() && !AllPlayersFullyConnected())
                m_pHostServer->SendConnectReminders();

            // Kill a tournament match that has taken too long to start
            if (m_pHostServer->IsTournMatch() &&
                GetGameTime() > (uint)(m_pHostServer->GetTournMatchStartTime()) &&
                GetGameTime() - m_pHostServer->GetTournMatchStartTime() >= sv_tournMatchWaitTime)
            {
                if (GetClientCount(TEAM_1) == GetTeamSize() && GetClientCount(TEAM_2) != GetTeamSize())
                {
                    // Team 1 wins
                }
                else if (GetClientCount(TEAM_1) != GetTeamSize() && GetClientCount(TEAM_2) == GetTeamSize())
                {
                    // Team 2 wins
                }
                else
                {
                    // Draw/double forfeit submission here
                    Reset();
                }

                return false;
            }
            
            // HACK: Countdown
            if (!m_pHostServer->IsArrangedMatch())
            {
                if (GetPhaseEndTime() != INVALID_TIME)
                {
                    uint uiRemainingTime(GetPhaseEndTime() - GetGameTime());
                    if (uiRemainingTime / 1000 != (uiRemainingTime + GetFrameLength()) / 1000)
                        SendMessage(XtoA((uiRemainingTime / 1000) + 1) + _T("..."), -1);
                }
            }
        }
        break;

    case GAME_PHASE_HERO_BAN:
        {
            // Get current team
            CTeamInfo *pTeam(GetTeam(m_uiBanningTeam));
            if (pTeam == NULL)
                break;

            // Instantly end this round when a ban is chosen
            if (m_iBanRound >= 0 &&
                (GetGameMode() == GAME_MODE_CAPTAINS_MODE || GetGameMode() == GAME_MODE_BANNING_PICK) &&
                int(pTeam->GetBanCount()) > m_iBanRound)
                SetGamePhaseEndTime(GetGameTime());

            if (GetGameTime() >= GetPhaseEndTime())
            {
                // Check for the end of the initial phase
                if (m_iBanRound < 0)
                {
                    CTeamInfo *pTeam(GetTeam(m_uiBanningTeam));
                    if (pTeam != NULL)
                    {
                        CPlayer *pCaptain(pTeam->GetCaptain());
                        if (pCaptain != NULL)
                            pCaptain->SetFlags(PLAYER_FLAG_CAN_PICK);
                    }

                    m_iBanRound = 0;

                    if (GetGameMode() == GAME_MODE_CAPTAINS_MODE || GetGameMode() == GAME_MODE_BANNING_PICK)
                        SetGamePhaseEndTime(GetGameTime() + sv_heroAllBanTime);
                    else
                        SetGamePhaseEndTime(GetGameTime() + sv_heroBanTime);
                    break;
                }

                // Get current team
                CTeamInfo *pTeam(GetTeam(m_uiBanningTeam));

                // Use extra time if any is remaining
                if (int(pTeam->GetBanCount()) <= m_iBanRound)
                {
                    if (pTeam->GetExtraTime() >= Game.GetFrameLength())
                    {
                        pTeam->SetExtraTime(pTeam->GetExtraTime() - Game.GetFrameLength());
                        pTeam->SetUsingExtraTime(true);
                        break;
                    }
                    else
                    {
                        pTeam->SetExtraTime(0);
                    }
                }

                pTeam->SetUsingExtraTime(false);

                // Make sure they have used up all of their bans to this point
                while (int(pTeam->GetBanCount()) <= m_iBanRound)
                {
                    // Issue a random ban
                    ushort unHeroID(GetRandomHeroFromPool());
                    SetHeroStatus(unHeroID, HERO_LIST_BANNED);
                    for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
                        ClearPotentialHero(it->second, unHeroID);
                    pTeam->IncrementBanCount();

                    CBufferFixed<7> buffer;
                    buffer << GAME_CMD_BAN_HERO_MESSAGE << -1 << unHeroID;
                    BroadcastGameData(buffer, true);
                }

                CPlayer *pCaptain(pTeam->GetCaptain());
                if (pCaptain != NULL)
                    pCaptain->RemoveFlags(PLAYER_FLAG_CAN_PICK);

                // Check opposing teams ban count
                CTeamInfo *pNextTeam(GetTeam(m_uiBanningTeam ^ 3));
                if (pNextTeam->GetBanCount() < GetBanCount())
                {
                    // Start a new banning round
                    if (pNextTeam->GetBanCount() == pTeam->GetBanCount())
                        ++m_iBanRound;

                    m_uiBanningTeam = pNextTeam->GetTeamID();
                    if (GetGameMode() == GAME_MODE_CAPTAINS_MODE || GetGameMode() == GAME_MODE_BANNING_PICK)
                        SetGamePhaseEndTime(GetGameTime() + sv_heroAllBanTime);
                    else
                        SetGamePhaseEndTime(GetGameTime() + sv_heroBanTime);

                    CPlayer *pCaptain(pNextTeam->GetCaptain());
                    if (pCaptain != NULL)
                        pCaptain->SetFlags(PLAYER_FLAG_CAN_PICK);
                }
                else
                {
                    for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
                        it->second->RemoveFlags(PLAYER_FLAG_CAN_PICK);

                    // Move on to the select phase
                    if (GetAlternatePicks())
                        SetGamePhase(GAME_PHASE_HERO_SELECT, sv_heroBanTransitionTime);
                    else
                        SetGamePhase(GAME_PHASE_HERO_SELECT, sv_heroSelectTime);
                }
            }
            else
            {
                pTeam->SetUsingExtraTime(false);
            }
        }
        break;

    case GAME_PHASE_HERO_SELECT:
        {
            // Force random selection
            if (HasGameOptions(GAME_OPTION_FORCE_RANDOM) && !m_bHasForcedRandom)
            {
                for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
                {
                    it->second->SetFlags(PLAYER_FLAG_CAN_PICK);
                    SelectRandomHero(it->first);
                }

                m_bHasForcedRandom = true;
            }

            // Check if all players are ready
            bool bAllPlayersReady(AllPlayersReady());

            // If all players are ready, shorten the wait time
            if (bAllPlayersReady && (GetPhaseEndTime() - GetGameTime() > sv_gameCountdownLength))
            {
                SetFlags(GAME_FLAG_FINAL_HERO_SELECT);
                SetGamePhaseEndTime(GetGameTime() + sv_gameCountdownLength);
            }

            // Check for all players in the current draft round picking their heroes
            if (GetAlternatePicks() && m_uiDraftRound > 0 && !HasFlags(GAME_FLAG_FINAL_HERO_SELECT))
            {
                bool bAllPlayersPicked(true);

                for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
                {
                    if (it->second->GetDraftRound() != m_uiDraftRound || it->second->HasSelectedHero())
                        continue;
                    
                    bAllPlayersPicked = false;
                }

                if (bAllPlayersPicked)
                    SetGamePhaseEndTime(GetGameTime());
            }

            // Check for time expiring
            if (GetGameTime() >= GetPhaseEndTime())
            {
                bool bStartMatch(true);

                if (GetAlternatePicks())
                {
                    // Use extra time if any is remaining
                    bool bHasNotPicked(false);
                    uint uiTeam(TEAM_INVALID);
                    for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
                    {
                        if (it->second->GetTeam() < TEAM_1 || it->second->GetTeam() > TEAM_2)
                            continue;

                        if (it->second->HasFlags(PLAYER_FLAG_CAN_PICK) && !it->second->HasSelectedHero())
                        {
                            // if the player is disconnected, don't allow them to use extra time.
                            if (it->second->IsDisconnected())
                            {
                                SelectPotentialHeroOrRandomHero(it->first);
                            }
                            else
                            {
                                bHasNotPicked = true;
                                uiTeam = it->second->GetTeam();
                                break;
                            }
                        }
                    }

                    if (bHasNotPicked)
                    {
                        CTeamInfo *pTeam(GetTeam(uiTeam));

                        if (pTeam != NULL)
                        {
                            if (pTeam->GetExtraTime() >= Game.GetFrameLength())
                            {
                                pTeam->SetExtraTime(pTeam->GetExtraTime() - Game.GetFrameLength());
                                pTeam->SetUsingExtraTime(true);
                                break;
                            }
                            else
                            {
                                pTeam->SetExtraTime(0);
                            }
                        }
                    }

                    // Clear "can pick" flag for everyone and random select if anyone didn't pick
                    for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
                    {
                        if (it->second->HasFlags(PLAYER_FLAG_CAN_PICK) && !it->second->HasSelectedHero())
                            SelectPotentialHeroOrRandomHero(it->first);

                        it->second->RemoveFlags(PLAYER_FLAG_CAN_PICK);
                    }

                    if (m_uiDraftRound < m_uiFinalDraftRound)
                    {
                        bStartMatch = false;

                        // Start next draft round
                        ++m_uiDraftRound;

                        for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
                        {
                            if (it->second->GetDraftRound() == m_uiDraftRound)
                            {
                                it->second->SetFlags(PLAYER_FLAG_CAN_PICK);
                            }
                        }

                        SetGamePhaseEndTime(GetGameTime() + sv_heroAltSelectTurnTime);

                        for (int iTeam(TEAM_1); iTeam <= TEAM_2; ++iTeam)
                        {
                            CTeamInfo *pTeam(GetTeam(iTeam));
                            if (pTeam == NULL)
                                continue;

                            pTeam->SetUsingExtraTime(false);
                        }
                    }                   
                    else if (!HasFlags(GAME_FLAG_FINAL_HERO_SELECT))
                    {
                        bStartMatch = false;
                        SetFlags(GAME_FLAG_FINAL_HERO_SELECT);
                        SetGamePhaseEndTime(GetGameTime() + sv_heroAltSelectPostTime);

                        for (int iTeam(TEAM_1); iTeam <= TEAM_2; ++iTeam)
                        {
                            CTeamInfo *pTeam(GetTeam(iTeam));
                            if (pTeam == NULL)
                                continue;

                            pTeam->SetExtraTime(0);
                            pTeam->SetUsingExtraTime(false);
                        }
                    }
                }

                if (bStartMatch)
                {
                    // All players without a hero selected for whatever reason get a random hero
                    for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
                    {
                        CPlayer* pPlayer(it->second);

                        if (!pPlayer->HasSelectedHero())
                            SelectPotentialHeroOrRandomHero(it->first);
                    }

                    SetGamePhase(GAME_PHASE_HERO_LOADING, sv_maxHeroLoadingTime);
                }
            }
        }
        break;

    case GAME_PHASE_HERO_LOADING:
        {
            bool bAllPlayersFinishedLoading(true);
            for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
            {
                if (it->second->IsDisconnected() || it->second->GetTeam() < TEAM_1 || it->second->GetTeam() > TEAM_2)
                    continue;

                if (!it->second->HasFlags(PLAYER_FLAG_LOADED_HEROES))
                {
                    bAllPlayersFinishedLoading = false;
                    break;
                }
            }

            if (GetGameTime() >= GetPhaseEndTime() || bAllPlayersFinishedLoading)
                StartPreMatch();
        }
        break;
    }

    return true;
}


/*====================
  CGameServer::ActiveFrame
  ====================*/
bool    CGameServer::ActiveFrame()
{
    PROFILE("CGameServer::ActiveFrame");

    CTeamInfo *pTeam1(GetTeam(TEAM_1));
    CTeamInfo *pTeam2(GetTeam(TEAM_2));
    if (pTeam1 == NULL || pTeam2 == NULL)
        return true;

    if (sv_statusNotifyTime && GetGameTime() >= m_uiLastStatusNotifyTime + sv_statusNotifyTime)
    {
        Console << GetServerStatus() << newl;
        m_uiLastStatusNotifyTime = GetGameTime(); 
    }

    // Annouce game start
    if (!m_bStartAnnounced &&
        GetGamePhase() == GAME_PHASE_PRE_MATCH &&
        GetRemainingPhaseTime() < sv_matchStartAnnounceTime)
    {
        CBufferFixed<1> buffer;
        buffer << GAME_CMD_START_GAME_WARNING;
        BroadcastGameData(buffer, true);

        m_bStartAnnounced = true;
    }

    // Check for the end of the pre-match phase
    if (GetGamePhase() == GAME_PHASE_PRE_MATCH && GetGameTime() >= GetPhaseEndTime())
        StartMatch();
    
    // Tree spawn
    if (Game.GetMatchTime() >= m_uiNextTreeSpawnTime)
    {
        if (sv_spawnTrees)
            SpawnTrees();

        m_uiNextTreeSpawnTime += g_treeSpawnInterval;
    }

    // Creep waves
    if (Game.GetMatchTime() >= m_uiNextCreepWaveTime)
    {
        if (sv_spawnCreeps)
            SpawnCreeps();

        ++m_uiCreepWaveCount;
        m_uiNextCreepWaveTime += g_creepWaveInterval;
    }

    // Powerup spawn
    if (Game.GetMatchTime() >= m_uiNextPowerupSpawnTime)
    {
        if (sv_spawnPowerups && !Game.HasGameOptions(GAME_OPTION_NO_POWERUPS))
            SpawnPowerup();

        m_uiNextPowerupSpawnTime += g_powerupSpawnInterval;
    }

    // Critter spawn
    if (Game.GetMatchTime() >= m_uiNextCritterSpawnTime)
    {
        if (sv_spawnCritters)
            SpawnCritters();

        m_uiNextCritterSpawnTime += g_critterSpawnInterval;
    }

    //
    // Execute entity frames
    //

    m_pServerEntityDirectory->FrameSetup();
    
    UpdateVisibility();

    m_pServerEntityDirectory->FrameThink();

    UpdateNavigation();
    
    m_pServerEntityDirectory->FrameMovement();
    m_pServerEntityDirectory->FrameAction();
    m_pServerEntityDirectory->FrameCleanup();

    return true;
}


/*====================
  CGameServer::EndedFrame
  ====================*/
bool    CGameServer::EndedFrame()
{
    PROFILE("CGameServer::EndedFrame");

    if (GetGamePhase() != GAME_PHASE_ENDED)
        return true;

    // Update server navigation
    UpdateVisibility();

    // Check for phase transition
    if (GetGameTime() < GetPhaseEndTime())
        return true;

    if (m_pHostServer->HasManager())
    {
        int iOldPriority(K2System.GetPriority());
        K2System.SetPriority(-1);

        ReplayManager.StopRecording();
        SendStats();

        K2System.SetPriority(iOldPriority);
    }
    else
    {
        ReplayManager.StopRecording();
        SendStats();
    }

    // Note that we delete the replay after it's recorded rather than just not
    // record it so that we can submit it with the match summary for stats
    if (!sv_autosaveReplay && !m_pHostServer->HasManager() && FileManager.Exists(ReplayManager.GetReplayFilename()))
        FileManager.Delete(ReplayManager.GetReplayFilename());

    m_mapClients.clear();
    m_pServerEntityDirectory->Clear();

    Reset();
    return true;
}


/*====================
  CGameServer::IsKickVoteValid
  ====================*/
bool    CGameServer::IsKickVoteValid(CPlayer *pTarget)
{
    if (pTarget == NULL)
        return false;
    if (pTarget->HasFlags(PLAYER_FLAG_TERMINATED))
        return false;
    if (!pTarget->HasFlags(PLAYER_FLAG_IS_AFK))
        return false;
    if (pTarget->GetTeam() == TEAM_SPECTATOR)
        return false;
    if (pTarget->HasFlags(PLAYER_FLAG_STAFF))
        return false;

    return true;
}


/*====================
  CGameServer::IsVisible
  ====================*/
bool    CGameServer::IsVisible(IUnitEntity* pFrom, float fX, float fY)
{
    if (pFrom == NULL)
        return false;

    uint uiTeamIndex(-1);
    if (pFrom->GetTeam() == TEAM_1)
        uiTeamIndex = 0;
    else if(pFrom->GetTeam() == TEAM_2)
        uiTeamIndex = 1;
    else
    {
        // for right now, this case should not happen.  Handle this when it does.
        Console.Warn << "CGameServer::IsVisible: Invalid team " << pFrom->GetTeam() << newl;
        assert(!"CGameServer::IsVisible: Invalid team number");
        return false;
    }

    int iVisibilityTileWidth(GetWorldPointer()->GetTileWidth() / m_uiVisibilitySize);
    int iVisibilityTileHeight(GetWorldPointer()->GetTileHeight() / m_uiVisibilitySize);

    uint uiX(uint(CLAMP(INT_FLOOR(fX / m_fVisibilityScale), 0, iVisibilityTileWidth - 1)));
    uint uiY(uint(CLAMP(INT_FLOOR(fY / m_fVisibilityScale), 0, iVisibilityTileHeight - 1)));

    assert(uiTeamIndex == 0 || uiTeamIndex == 1);
    if (!m_cVisibilityMap[uiTeamIndex].IsVisible(uiX, uiY))
        return false;

    return true;
}


/*====================
  CGameServer::UpdateVoIP
  ====================*/
void    CGameServer::UpdateVoIP()
{
    map<int, IHeroEntity*> mapHeroList;

    // Retrieve the list of heroes to test for distance
    for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); it++)
    {
        IHeroEntity *pHero(it->second->GetHero());

        if (pHero == NULL)
            continue;

        mapHeroList.insert(pair<int, IHeroEntity*>(it->first, pHero));
        VoiceServer.RemoveAllVoiceTargets(it->first, VOICE_TARGET_SET_LANE);
    }

    map<int, ivector> mapTargets;

    // Test for distance and store target results in mapTargets. Testing method is:
    // 1 -> 2
    // 1 -> 3
    // 1 -> 4
    // 2 -> 3
    // 2 -> 4
    // 3 -> 4
    // Done.
    for (map<int, IHeroEntity*>::iterator it(mapHeroList.begin()); it != mapHeroList.end(); it++)
    {
        map<int, IHeroEntity*>::iterator itComp(it);
        itComp++;

        while (itComp != mapHeroList.end())
        {
            bool bEnemy1(it->second->IsEnemy(itComp->second));
            bool bEnemy2(itComp->second->IsEnemy(it->second));

            if (bEnemy1 && bEnemy2)
            {
                ++itComp;
                continue;
            }

            if (Distance(it->second->GetPosition(), itComp->second->GetPosition()) > g_voiceLaneChatRange)
            {
                itComp++;
                continue;
            }

            if (!bEnemy1)
                mapTargets[it->first].push_back(itComp->first);

            if (!bEnemy2)
                mapTargets[itComp->first].push_back(it->first);

            itComp++;
        }
    }

    // Add the finalized voice targets to the voice server
    for (map<int, ivector>::iterator it(mapTargets.begin()); it != mapTargets.end(); it++)
        VoiceServer.AddVoiceTargets(it->second, it->first, VOICE_TARGET_SET_LANE);
}

#define WRITE_STAT(type, type_label, stat, value) \
{ \
    itStat = map##type##Stats.find(ev.GetPropertyInt(_T(#type_label), -1)); \
    if (itStat != map##type##Stats.end()) \
        itStat->second->Add##stat(value); \
}

#define WRITE_PLAYER_STAT(stat, value)  WRITE_STAT(Player, player, stat, value)
#define WRITE_TEAM_STAT(stat, value)    WRITE_STAT(Team, team, stat, value)

/*====================
  CGameServer::ParseGameLog
  ====================*/
void    CGameServer::ParseGameLog(const tstring &sPath)
{
    // Clear any existing stat entities
    vector<IGameEntity*> vDelete;
    for (IGameEntity *pEntity(GetFirstEntity()); pEntity != NULL; pEntity = GetNextEntity(pEntity))
    {
        if (!pEntity->IsStats())
            continue;

        vDelete.push_back(pEntity);
    }

    for (vector<IGameEntity*>::iterator it(vDelete.begin()); it != vDelete.end(); ++it)
        DeleteEntity(*it);

    // Create stats entity for each team

    // Open the log file
    CFileHandle hGameLog(sPath, FILE_READ | FILE_TEXT);
    if (!hGameLog.IsOpen())
        return;

    map<int, CGameStats*> mapPlayerStats;
    map<int, CGameStats*> mapTeamStats;

    for (int i(TEAM_1); i <= TEAM_2; ++i)
    {
        CTeamInfo *pTeam(GetTeam(i));
        if (pTeam == NULL)
            continue;

        IGameEntity *pNewEntity(AllocateEntity(Info_Stats));
        if (pNewEntity == NULL)
            continue;

        CGameStats *pStats(pNewEntity->GetAsStats());
        if (pStats == NULL)
            continue;

        mapTeamStats[i] = pStats;
        pTeam->AssignStats(pStats);
    }

    map<int, CGameStats*>::iterator itStat;
    while (!hGameLog.IsEOF())
    {
        // Read a line and generate a property map
        tstring sLine(hGameLog.ReadLine());
        if (sLine.empty())
            continue;

        CGameLogEvent ev(sLine);

        switch (ev.GetEventType())
        {
        case GAME_LOG_PLAYER_CONNECT:
            {
                // Get player number
                int iPlayer(ev.GetPropertyInt(_T("player"), -1));
                if (iPlayer == -1)
                    break;

                // Create a new stat entry for this player if none exists
                map<int, CGameStats*>::iterator itStats(mapPlayerStats.find(iPlayer));
                if (itStats == mapPlayerStats.end())
                {
                    IGameEntity *pNewStats(AllocateEntity(Info_Stats));
                    if (pNewStats != NULL && pNewStats->GetAsStats() != NULL)
                    {
                        mapPlayerStats[iPlayer] = pNewStats->GetAsStats();

                        CPlayer *pPlayer(Game.GetPlayer(iPlayer));
                        if (pPlayer != NULL)
                            pPlayer->AssignStats(pNewStats->GetAsStats());
                    }
                }
            }
            break;

        case GAME_LOG_PLAYER_DISCONNECT:
            break;

        case GAME_LOG_PLAYER_TIMEDOUT:
            break;
            
        case GAME_LOG_PLAYER_TERMINATED:
            break;

        case GAME_LOG_PLAYER_ACTIONS:
            {
                uint uiActionCount(ev.GetPropertyFloat(_T("count")));
                WRITE_PLAYER_STAT(ActionCount, uiActionCount)
                WRITE_TEAM_STAT(ActionCount, uiActionCount)
            }
            break;

        case GAME_LOG_PLAYER_BUYBACK:
            {
                uint uiCost(ev.GetPropertyInt(_T("cost")));
                WRITE_PLAYER_STAT(BuyBacks, 1)
                WRITE_PLAYER_STAT(GoldSpent, uiCost)

                WRITE_TEAM_STAT(BuyBacks, 1)
                WRITE_TEAM_STAT(GoldSpent, uiCost)
            }
            break;

        case GAME_LOG_PLAYER_CALL_VOTE:
            {
                uint uiVoteType(GetVoteTypeFromString(ev.GetProperty(_T("type"))));
                if (uiVoteType == VOTE_TYPE_CONCEDE)
                    WRITE_PLAYER_STAT(ConcedeCalls, 1)
            }
            break;

        case GAME_LOG_GAME_END:
            {
                for (map<int, CGameStats*>::iterator it(mapPlayerStats.begin()); it != mapPlayerStats.end(); ++it)
                {
                    CPlayer *pPlayer(GetPlayer(it->first));
                    if (pPlayer == NULL || pPlayer->HasFlags(PLAYER_FLAG_TERMINATED))
                        continue;

                    it->second->SetTimePlayed(pPlayer->GetPlayTime());
                }
            }
            break;

        case GAME_LOG_GAME_CONCEDE:
            hGameLog.Seek(0, SEEK_ORIGIN_END);
            break;

        case GAME_LOG_HERO_DEATH:
            {
                map<int, CGameStats*>::iterator itStats(mapPlayerStats.find(ev.GetPropertyInt(_T("player"), -1)));
                if (itStats != mapPlayerStats.end())
                {
                    itStats->second->AddDeaths(1);
                    itStats->second->LogDeath(CGameStats::KillLogEvent(ev.GetPropertyInt(_T("time")), ev.GetPropertyInt(_T("owner"))));
                }

                WRITE_TEAM_STAT(Deaths, 1)
            }
            break;

        case GAME_LOG_HERO_ASSIST:
            {
                map<int, CGameStats*>::iterator itStats(mapPlayerStats.find(ev.GetPropertyInt(_T("player"), -1)));
                if (itStats != mapPlayerStats.end())
                {
                    itStats->second->AddHeroAssists(1);
                    itStats->second->LogAssist(CGameStats::KillLogEvent(ev.GetPropertyInt(_T("time")), ev.GetPropertyInt(_T("owner"))));
                }

                WRITE_TEAM_STAT(HeroAssists, 1)
            }
            break;

        case GAME_LOG_HERO_DENY:
            break;

        case GAME_LOG_HERO_RESPAWN:
            {
                map<int, CGameStats*>::iterator itStats(mapPlayerStats.find(ev.GetPropertyInt(_T("player"), -1)));
                if (itStats != mapPlayerStats.end())
                    itStats->second->AddTimeDead(ev.GetPropertyFloat(_T("duration")));
            }
            break;

        case GAME_LOG_HERO_LEVEL:
            break;

        case GAME_LOG_CREEP_DENY:
            {
                float fExperience(ev.GetPropertyFloat(_T("experience")));
                WRITE_PLAYER_STAT(Denies, 1)
                WRITE_PLAYER_STAT(DeniedExperience, fExperience)

                WRITE_TEAM_STAT(Denies, 1)
                WRITE_TEAM_STAT(DeniedExperience, fExperience)
            }
            break;

        case GAME_LOG_BUILDING_DENY:
            break;

        case GAME_LOG_ITEM_PURCHASE:
            {
                ushort unCost(ev.GetPropertyInt(_T("cost")));
                map<int, CGameStats*>::iterator itStats(mapPlayerStats.find(ev.GetPropertyInt(_T("player"), -1)));
                if (itStats != mapPlayerStats.end())
                {
                    CGameStats::SItemHistoryEntry entry(
                        ev.GetPropertyInt(_T("time")),
                        EntityRegistry.LookupID(ev.GetProperty(_T("item"))),
                        byte(ITEM_HISTORY_PURCHASE));
                    itStats->second->AddGoldSpent(unCost);
                    itStats->second->LogItemHistory(entry);

                    CItemDefinition *pItemDef(EntityRegistry.GetDefinition<CItemDefinition>(ev.GetProperty(_T("item"))));
                    if (pItemDef != NULL)
                    {
                        if (pItemDef->GetCategory() == _T("ward"))
                            itStats->second->AddWardsPurchased(1);
                        else if (pItemDef->GetCategory() == _T("consumable"))
                            itStats->second->AddConsumablesPurchased(1);
                    }
                }

                WRITE_TEAM_STAT(GoldSpent, unCost)
            }
            break;

        case GAME_LOG_ITEM_SELL:
            {
                map<int, CGameStats*>::iterator itStats(mapPlayerStats.find(ev.GetPropertyInt(_T("player"), -1)));
                if (itStats != mapPlayerStats.end())
                {
                    CGameStats::SItemHistoryEntry entry(
                        ev.GetPropertyInt(_T("time")),
                        EntityRegistry.LookupID(ev.GetProperty(_T("item"))),
                        byte(ITEM_HISTORY_SELL));
                    itStats->second->LogItemHistory(entry);
                }
            }
            break;

        case GAME_LOG_ITEM_ASSEMBLE:
            {
                map<int, CGameStats*>::iterator itStats(mapPlayerStats.find(ev.GetPropertyInt(_T("player"), -1)));
                if (itStats != mapPlayerStats.end())
                {
                    CGameStats::SItemHistoryEntry entry(
                        ev.GetPropertyInt(_T("time")),
                        EntityRegistry.LookupID(ev.GetProperty(_T("item"))),
                        byte(ITEM_HISTORY_ASSEMBLE));
                    itStats->second->LogItemHistory(entry);
                }
            }
            break;

        case GAME_LOG_ITEM_DISASSEMBLE:
            {
                map<int, CGameStats*>::iterator itStats(mapPlayerStats.find(ev.GetPropertyInt(_T("player"), -1)));
                if (itStats != mapPlayerStats.end())
                {
                    CGameStats::SItemHistoryEntry entry(
                        ev.GetPropertyInt(_T("time")),
                        EntityRegistry.LookupID(ev.GetProperty(_T("item"))),
                        byte(ITEM_HISTORY_DISASSEMBLE));
                    itStats->second->LogItemHistory(entry);
                }
            }
            break;

        case GAME_LOG_DAMAGE:
            {
                float fDamage(ev.GetPropertyInt(_T("damage")));

                switch (EntityRegistry.GetBaseType(ev.GetProperty(_T("target"))))
                {
                case ENTITY_BASE_TYPE_HERO:
                    WRITE_PLAYER_STAT(HeroDamage, fDamage)
                    WRITE_TEAM_STAT(HeroDamage, fDamage)
                    break;
                
                case ENTITY_BASE_TYPE_CREEP:
                    WRITE_PLAYER_STAT(CreepDamage, fDamage)
                    WRITE_TEAM_STAT(CreepDamage, fDamage)
                    break;
                
                case ENTITY_BASE_TYPE_NEUTRAL:
                    WRITE_PLAYER_STAT(NeutralDamage, fDamage)
                    WRITE_TEAM_STAT(NeutralDamage, fDamage)
                    break;
                
                case ENTITY_BASE_TYPE_BUILDING:
                    WRITE_PLAYER_STAT(BuildingDamage, fDamage)
                    WRITE_TEAM_STAT(BuildingDamage, fDamage)
                    break;
                }
            }
            break;

        case GAME_LOG_KILL:
            {
                int iKillerPlayer(ev.GetPropertyInt(_T("player"), -1));
                int iVictimPlayer(ev.GetPropertyInt(_T("owner"), -1));
                uint uiTimeStamp(ev.GetPropertyInt(_T("time")));

                tsvector vAssistString(TokenizeString(ev.GetProperty(_T("assists")), _T(',')));
                ivector vAssistClients;
                for (tsvector_it it(vAssistString.begin()); it != vAssistString.end(); ++it)
                    vAssistClients.push_back(AtoI(*it));

                switch (EntityRegistry.GetBaseType(ev.GetProperty(_T("target"))))
                {
                case ENTITY_BASE_TYPE_HERO:
                    {
                        map<int, CGameStats*>::iterator itStats(mapPlayerStats.find(iKillerPlayer));
                        if (itStats != mapPlayerStats.end())
                        {
                            itStats->second->AddHeroKills(1);
                            
                            CGameStats::SHeroKillHistoryEvent entry(uiTimeStamp, iVictimPlayer, vAssistClients);
                            itStats->second->LogKill(entry);
                        }

                        WRITE_TEAM_STAT(HeroKills, 1)

                        SMatchKillEntry entry;

                        tstring sAttacker(ev.GetProperty(_T("attacker")));
                        uint uiBaseType(EntityRegistry.GetBaseType(sAttacker));
                        if (uiBaseType & ENTITY_BASE_TYPE_HERO)
                            entry.m_eKillCode = HKK_HERO;
                        else if (uiBaseType & ENTITY_BASE_TYPE_NEUTRAL)
                            entry.m_eKillCode = HKK_NEUTRAL;
                        else if (uiBaseType & ENTITY_BASE_TYPE_BUILDING)
                            entry.m_eKillCode = HKK_TOWER;
                        else if (uiBaseType & ENTITY_BASE_TYPE_CREEP)
                            entry.m_eKillCode = HKK_TEAM;
                        else if (sAttacker == _T("Neutral_Kongor"))
                            entry.m_eKillCode = HKK_KONGOR;
                        else
                            entry.m_eKillCode = HKK_UNKNOWN;

                        entry.m_iKillerClientNumber = iKillerPlayer;
                        entry.m_iVictimClientNumber = iVictimPlayer;
                        entry.m_uiTimeStamp = uiTimeStamp;
                        entry.m_vAssists = vAssistClients;
                        m_vMatchKillLog.push_back(entry);
                    }
                    break;

                case ENTITY_BASE_TYPE_CREEP:
                    WRITE_PLAYER_STAT(CreepKills, 1)
                    WRITE_TEAM_STAT(CreepKills, 1)
                    break;

                case ENTITY_BASE_TYPE_NEUTRAL:
                    WRITE_PLAYER_STAT(NeutralKills, 1)
                    WRITE_TEAM_STAT(NeutralKills, 1)
                    break;

                case ENTITY_BASE_TYPE_BUILDING:
                    WRITE_PLAYER_STAT(BuildingKills, 1)
                    WRITE_TEAM_STAT(BuildingKills, 1)
                    break;
                }
            }
            break;

        case GAME_LOG_EXP_EARNED:
            {
                float fExperience(ev.GetPropertyFloat(_T("experience")));
                WRITE_PLAYER_STAT(Experience, fExperience)
                WRITE_TEAM_STAT(Experience, fExperience)

                switch (EntityRegistry.GetBaseType(ev.GetProperty(_T("source"))))
                {
                case ENTITY_BASE_TYPE_HERO:
                    WRITE_PLAYER_STAT(HeroExperience, fExperience)
                    WRITE_TEAM_STAT(HeroExperience, fExperience)
                    break;

                case ENTITY_BASE_TYPE_CREEP:
                    WRITE_PLAYER_STAT(CreepExperience, fExperience)
                    WRITE_TEAM_STAT(CreepExperience, fExperience)
                    break;

                case ENTITY_BASE_TYPE_NEUTRAL:
                    WRITE_PLAYER_STAT(NeutralExperience, fExperience)
                    WRITE_TEAM_STAT(NeutralExperience, fExperience)
                    break;

                case ENTITY_BASE_TYPE_BUILDING:
                    WRITE_PLAYER_STAT(BuildingExperience, fExperience)
                    WRITE_TEAM_STAT(BuildingExperience, fExperience)
                    break;
                }
            }
            break;

        case GAME_LOG_EXP_DENIED:
            break;

        case GAME_LOG_GOLD_LOST:
            {
                ushort unGold(ev.GetPropertyInt(_T("gold")));
                WRITE_PLAYER_STAT(GoldLost, unGold)
                WRITE_TEAM_STAT(GoldLost, unGold)
            }
            break;

        case GAME_LOG_GOLD_EARNED:
            {
                ushort unGold(ev.GetPropertyInt(_T("gold")));
                WRITE_PLAYER_STAT(GoldEarned, unGold)
                WRITE_TEAM_STAT(GoldEarned, unGold)

                switch (EntityRegistry.GetBaseType(ev.GetProperty(_T("source"))))
                {
                case ENTITY_BASE_TYPE_HERO:
                    WRITE_PLAYER_STAT(HeroBounty, unGold)
                    WRITE_TEAM_STAT(HeroBounty, unGold)
                    break;

                case ENTITY_BASE_TYPE_CREEP:
                    WRITE_PLAYER_STAT(CreepBounty, unGold)
                    WRITE_TEAM_STAT(CreepBounty, unGold)
                    break;

                case ENTITY_BASE_TYPE_NEUTRAL:
                    WRITE_PLAYER_STAT(NeutralBounty, unGold)
                    WRITE_TEAM_STAT(NeutralBounty, unGold)
                    break;

                case ENTITY_BASE_TYPE_BUILDING:
                    WRITE_PLAYER_STAT(BuildingBounty, unGold)
                    WRITE_TEAM_STAT(BuildingBounty, unGold)
                    break;
                }
            }
            break;

        case GAME_LOG_ABILITY_UPGRADE:
            {
                map<int, CGameStats*>::iterator itStats(mapPlayerStats.find(ev.GetPropertyInt(_T("player"), -1)));
                if (itStats != mapPlayerStats.end())
                {
                    CGameStats::SAbilityUpgradeEntry entry(
                        ev.GetPropertyInt(_T("time")),
                        EntityRegistry.LookupID(ev.GetProperty(_T("name"))),
                        ev.GetPropertyInt(_T("level")),
                        ev.GetPropertyInt(_T("slot")));
                    itStats->second->LogAbilityUpgrade(entry);
                }
            }
            break;
        }
    }
}

SERVER_CMD(ParseLog)
{
    if (vArgList.empty())
        return false;

    GameServer.ParseGameLog(vArgList[0]);
    return true;
}


/*====================
  CGameServer::GetMatchIDFromMasterServer
  ====================*/
void    CGameServer::GetMatchIDFromMasterServer()
{
#ifdef K2_CLIENT
    CGameInfo *pGameInfo(GetGameInfo());
    if (pGameInfo != NULL)
        pGameInfo->SetMatchID(uint(-1));
#else
    CHTTPRequest *pRequest(Host.GetHTTPManager()->SpawnRequest());
    if (pRequest == NULL)
    {
        CGameInfo *pGameInfo(GetGameInfo());
        if (pGameInfo != NULL)
            pGameInfo->SetMatchID(uint(-1));

        m_sReplayHost = TSNULL;
        m_sReplayDir = TSNULL;
        return;
    }

    pRequest->SetTargetURL(m_pHostServer->GetMasterServerURL());
    pRequest->AddVariable(L"f", L"start_game");
    pRequest->AddVariable(L"session", m_pHostServer->GetSessionCookie());
    pRequest->AddVariable(L"code", TSNULL);
    pRequest->AddVariable(L"extra", TSNULL);
    pRequest->AddVariable(L"map", GetWorldPointer()->GetName());
    pRequest->AddVariable(L"version", K2System.GetVersionString());
    pRequest->AddVariable(L"mname", m_sName);
    pRequest->AddVariable(L"mstr", sv_masterName);
    pRequest->SendPostRequest();
    pRequest->Wait();

    if (pRequest->WasSuccessful())
    {
        CPHPData phpResponse(pRequest->GetResponse());
        
        CGameInfo *pGameInfo(GetGameInfo());
        if (pGameInfo != NULL)
            pGameInfo->SetMatchID(phpResponse.GetInteger(_T("match_id"), -1));

        m_sReplayHost = phpResponse.GetString(_T("file_host"));
        m_sReplayDir = phpResponse.GetString(_T("file_dir"));
    }
    else
    {
        CGameInfo *pGameInfo(GetGameInfo());
        if (pGameInfo != NULL)
            pGameInfo->SetMatchID(uint(-1));

        m_sReplayHost = TSNULL;
        m_sReplayDir = TSNULL;
    }

    Host.GetHTTPManager()->ReleaseRequest(pRequest);
#endif
}

SERVER_CMD(GetMatchID)
{
    GameServer.GetMatchIDFromMasterServer();
    return true;
}


/*====================
  CGameServer::SendTrialGameIncrease
  ====================*/
void    CGameServer::SendTrialGameIncrease()
{

    if (Host.IsReplay() || m_bSolo || m_bLocal)
        return; 

    CHTTPRequest *pRequest(Host.GetHTTPManager()->SpawnRequest());

    pRequest->SetTargetURL(m_pHostServer->GetMasterServerURL());
    pRequest->AddVariable(L"f", L"update_trial");
    pRequest->AddVariable(L"session", m_pHostServer->GetSessionCookie());

    bool bTrialSent(false);
    for (PlayerMap_it itPlayer(m_mapClients.begin()); itPlayer != m_mapClients.end(); ++itPlayer)
    {
        int iAccountId = itPlayer->second->GetAccountID();
        map<int, uint>::iterator itAccountType(m_mapPlayerAccountType.find(iAccountId));
        map<int, uint>::iterator itTrialStatus(m_mapPlayerTrialStatus.find(iAccountId));
        map<int, uint>::iterator itTrialCount(m_mapPlayerTrialCount.find(iAccountId));
        int iClientNumber = itPlayer->second->GetClientNumber();


        if (itAccountType == m_mapPlayerAccountType.end() || itTrialStatus == m_mapPlayerTrialStatus.end() || itTrialCount == m_mapPlayerTrialCount.end())
        {
            Console.Err << _T("Server missing account map information!");
            assert(!"AccountType / TrialStatus / TrialCount map find failed");
            continue;
        } 

        if (itAccountType->second == 1 && itTrialStatus->second > 0)
        {
            ++itTrialCount->second;

            int iTrialGamesCount(itTrialCount->second);

            pRequest->AddVariable(L"trial_ids[]", XtoA(iAccountId));

                //No Client tracking till patch.
            CBufferFixed<5> buffer;
            buffer << GAME_CMD_TRIAL_INC << iTrialGamesCount;
            SendGameData(iClientNumber, buffer, true);

            bTrialSent = true;
        }
    }

    if (bTrialSent)
    {
        pRequest->SendPostRequest();
        pRequest->Wait();
    }

    Host.GetHTTPManager()->ReleaseRequest(pRequest);
}

/*====================
  CGameServer::SendStats
  ====================*/
void    CGameServer::SendStats()
{
    if (GetGameInfo()->GetMatchID() == -1)
        return;

#ifndef K2_CLIENT
    CHTTPRequest *pRequest(Host.GetHTTPManager()->SpawnRequest());
    if (pRequest == NULL)
        return;

    pRequest->SetTargetURL(K2System.GetMasterServerAddress() + "/stats_requester.php");

    tstring sDate(XtoA(m_cMatchDate.GetYear(), FMT_PADZERO, 4) + XtoA(m_cMatchDate.GetMonth(), FMT_PADZERO, 2) + XtoA(m_cMatchDate.GetDay(), FMT_PADZERO, 2));

    tstring sReplayName(ReplayManager.GetReplayFilename());

    uint uiReplaySize(0);
    struct _stat st;
    if (FileManager.Stat(sReplayName, st))
        uiReplaySize = uint(st.st_size);

    if (uiReplaySize > 0)
        sReplayName = _T("/") + sDate + _T("/") + Filename_StripPath(sReplayName);

    uint uiSecondsPlayed(INT_ROUND(MsToSec(GetGameInfo()->GetMatchLength())));

    // Header
    //pRequest->AddVariable(_T("f"), _T("match_stats"));
    pRequest->AddVariable(_T("session"), m_pHostServer->GetSessionCookie());
    pRequest->AddVariable(_T("match_stats[match_id]"), GetGameInfo()->GetMatchID());
    pRequest->AddVariable(_T("match_stats[map]"), GetWorldPointer()->GetName());
    pRequest->AddVariable(_T("match_stats[map_version]"), GetWorldPointer()->GetVersionString());
    pRequest->AddVariable(_T("match_stats[time_played]"), uiSecondsPlayed);
    pRequest->AddVariable(_T("match_stats[file_size]"), uiReplaySize);
    pRequest->AddVariable(_T("match_stats[file_name]"), sReplayName);
    pRequest->AddVariable(_T("match_stats[c_state]"), 5);
    pRequest->AddVariable(_T("match_stats[version]"), K2System.GetVersionString());
    pRequest->AddVariable(_T("match_stats[avgpsr]"), INT_ROUND(m_fAverageRating));
    pRequest->AddVariable(_T("match_stats[avgpsr_team1]"), INT_ROUND(m_fTeamAverageRating[0]));
    pRequest->AddVariable(_T("match_stats[avgpsr_team2]"), INT_ROUND(m_fTeamAverageRating[1]));

    tstring sBannedHeroes(GetBannedHeroesStr());
    if (!sBannedHeroes.empty())
        pRequest->AddVariable(_T("match_stats[banned_heroes]"), sBannedHeroes);

    // Players
    uint uiItemCount(0);
    for (PlayerMap_it itPlayer(m_mapClients.begin()); itPlayer != m_mapClients.end(); ++itPlayer)
    {
        CPlayer *pPlayer(itPlayer->second);
        uint uiSecondsPlayerPlayed(INT_ROUND(MsToSec(pPlayer->GetPlayTime())));
        
        IHeroEntity *pHero(pPlayer->GetHero());
        if (pHero == NULL)
            continue;

        CGameStats *pStats(itPlayer->second->GetStats());
        if (pStats == NULL)
            continue;

        tstring sPlayer(_T("player_stats[") + XtoA(pPlayer->GetAccountID()) + _T("][") + pHero->GetTypeName() + _T("]"));
        pRequest->AddVariable(sPlayer + _T("[clan_id]"), pPlayer->GetClanID());
        pRequest->AddVariable(sPlayer + _T("[team]"), pPlayer->GetTeam());
        pRequest->AddVariable(sPlayer + _T("[position]"), pPlayer->GetTeamIndex() + (pPlayer->GetTeam() - 1) * 5);
        
        bool bTerminated(pPlayer->HasFlags(PLAYER_FLAG_TERMINATED) && !pPlayer->HasFlags(PLAYER_FLAG_EXCUSED));
        bool bWinner(pPlayer->GetTeam() == Game.GetWinningTeam());
        pRequest->AddVariable(sPlayer + _T("[wins]"), (bWinner && !bTerminated) ? 1 : 0);
        pRequest->AddVariable(sPlayer + _T("[losses]"), (!bWinner || bTerminated) ? 1 : 0);
        pRequest->AddVariable(sPlayer + _T("[discos]"), bTerminated ? 1 : 0);
        pRequest->AddVariable(sPlayer + _T("[concedes]"), (!bWinner && HasFlags(GAME_FLAG_CONCEDED)) ? 1 : 0);
        pRequest->AddVariable(sPlayer + _T("[kicked]"), pPlayer->HasFlags(PLAYER_FLAG_KICKED) ? 1 : 0);
        
        if (m_pHostServer->IsArrangedMatch())
        {
            pRequest->AddVariable(sPlayer + _T("[amm_solo_rating]"), bWinner ? pPlayer->GetStoredMatchWinValue() : pPlayer->GetStoredMatchLossValue());
            pRequest->AddVariable(sPlayer + _T("[amm_solo_count]"), 1);
        }
        else
        {
            pRequest->AddVariable(sPlayer + _T("[pub_skill]"), bWinner ? pPlayer->GetStoredMatchWinValue() : pPlayer->GetStoredMatchLossValue());
            pRequest->AddVariable(sPlayer + _T("[pub_count]"), 1);
        }

        pRequest->AddVariable(sPlayer + _T("[concedevotes]"), pStats->GetConcedeCalls());
        
        pRequest->AddVariable(sPlayer + _T("[herokills]"), pStats->GetHeroKills());
        pRequest->AddVariable(sPlayer + _T("[herodmg]"), pStats->GetHeroDamage());
        pRequest->AddVariable(sPlayer + _T("[herokillsgold]"), pStats->GetHeroBounty());
        pRequest->AddVariable(sPlayer + _T("[heroassists]"), pStats->GetHeroAssists());
        pRequest->AddVariable(sPlayer + _T("[heroexp]"), pStats->GetHeroExperience());
        pRequest->AddVariable(sPlayer + _T("[deaths]"), pStats->GetDeaths());
        pRequest->AddVariable(sPlayer + _T("[buybacks]"), pStats->GetBuyBacks());
        pRequest->AddVariable(sPlayer + _T("[goldlost2death]"), pStats->GetGoldLost());
        pRequest->AddVariable(sPlayer + _T("[secs_dead]"), INT_ROUND(MsToSec(pStats->GetTimeDead())));
        pRequest->AddVariable(sPlayer + _T("[teamcreepkills]"), pStats->GetCreepKills());
        pRequest->AddVariable(sPlayer + _T("[teamcreepdmg]"), pStats->GetCreepDamage());
        pRequest->AddVariable(sPlayer + _T("[teamcreepgold]"), pStats->GetCreepBounty());
        pRequest->AddVariable(sPlayer + _T("[teamcreepexp]"), pStats->GetCreepExperience());
        pRequest->AddVariable(sPlayer + _T("[neutralcreepkills]"), pStats->GetNeutralKills());
        pRequest->AddVariable(sPlayer + _T("[neutralcreepdmg]"), pStats->GetNeutralDamage());
        pRequest->AddVariable(sPlayer + _T("[neutralcreepgold]"), pStats->GetNeutralBounty());
        pRequest->AddVariable(sPlayer + _T("[neutralcreepexp]"), pStats->GetNeutralExperience());
        pRequest->AddVariable(sPlayer + _T("[bdmg]"), pStats->GetBuildingDamage());
        pRequest->AddVariable(sPlayer + _T("[razed]"), pStats->GetBuildingKills());
        pRequest->AddVariable(sPlayer + _T("[bdmgexp]"), pStats->GetBuildingExperience());
        pRequest->AddVariable(sPlayer + _T("[bgold]"), pStats->GetBuildingBounty());
        pRequest->AddVariable(sPlayer + _T("[denies]"), pStats->GetDenies());
        pRequest->AddVariable(sPlayer + _T("[exp_denied]"), pStats->GetDeniedExperience());
        pRequest->AddVariable(sPlayer + _T("[gold]"), pStats->GetGoldEarned());
        pRequest->AddVariable(sPlayer + _T("[gold_spent]"), pStats->GetGoldSpent());
        pRequest->AddVariable(sPlayer + _T("[exp]"), pStats->GetExperience());
        pRequest->AddVariable(sPlayer + _T("[actions]"), pStats->GetActionCount());
        pRequest->AddVariable(sPlayer + _T("[secs]"), uiSecondsPlayerPlayed);
        pRequest->AddVariable(sPlayer + _T("[level]"), pHero->GetLevel());
        pRequest->AddVariable(sPlayer + _T("[consumables]"), pStats->GetConsumablesPurchased());
        pRequest->AddVariable(sPlayer + _T("[wards]"), pStats->GetWardsPurchased());

        // exp/min calculation
        if (pHero->GetLevel() == pHero->GetMaxLevel())
        {
            // the hero is max level, so use exp/final_exp_earned_time.
            uint uiSecondsEarningExp(INT_ROUND(MsToSec(pHero->GetFinalExpEarnedTime())));
            pRequest->AddVariable(sPlayer + _T("[time_earning_exp]"), uiSecondsEarningExp);
        }
        else
        {
            // otherwise use exp/time_played.
            pRequest->AddVariable(sPlayer + _T("[time_earning_exp]"), uiSecondsPlayerPlayed);
        }
        
        for (int iSlot(INVENTORY_START_BACKPACK); iSlot <= INVENTORY_END_BACKPACK; ++iSlot)
        {
            IEntityItem *pItem(pHero->GetItem(iSlot));
            if (pItem == NULL)
                continue;

            tstring sInventory(_T("inventory[") + XtoA(pPlayer->GetAccountID()) + _T("][") + XtoA(iSlot - INVENTORY_START_BACKPACK) + _T("]"));
            pRequest->AddVariable(sInventory, pItem->GetTypeName());
        }

        const CGameStats::ItemHistoryLog &logItems(pStats->GetItemHistoryLog());
        for (CGameStats::ItemHistoryLog_cit itItem(logItems.begin()); itItem != logItems.end(); ++itItem)
        {
            tstring sItem(_T("items[") + XtoA(uiItemCount) + _T("]"));
            pRequest->AddVariable(sItem + _T("[account_id]"), pPlayer->GetAccountID());
            pRequest->AddVariable(sItem + _T("[cli_name]"), EntityRegistry.LookupName(itItem->unItemTypeID));
            pRequest->AddVariable(sItem + _T("[secs]"), INT_ROUND(MsToSec(itItem->uiTimeStamp)));
            pRequest->AddVariable(sItem + _T("[action]"), itItem->yAction);
            ++uiItemCount;
        }

        const CGameStats::AbilityUpgradeLog &logAbilities(pStats->GetAbilityUpgradeLog());
        for (CGameStats::AbilityUpgradeLog_cit itAbility(logAbilities.begin()); itAbility != logAbilities.end(); ++itAbility)
        {
            uint uiIndex(itAbility - logAbilities.begin());
            tstring sAbility(_T("abilities[") + XtoA(pPlayer->GetAccountID()) + _T("][") + XtoA(uiIndex) + _T("]"));
            pRequest->AddVariable(sAbility + _T("[hero_cli_name]"), pHero->GetTypeName());
            pRequest->AddVariable(sAbility + _T("[ability_cli_name]"), EntityRegistry.LookupName(itAbility->unAbilityTypeID));
            pRequest->AddVariable(sAbility + _T("[secs]"), INT_ROUND(MsToSec(itAbility->uiTimeStamp)));
            pRequest->AddVariable(sAbility + _T("[slot]"), itAbility->ySlot);
        }

        if (!pPlayer->GetMatchComment().empty())
            pRequest->AddVariable(_T("comments[") + XtoA(pPlayer->GetAccountID()) + _T("]"), pPlayer->GetMatchComment());
    }

    // Kill log
    uint uiKillCount(0);
    for (vector<SMatchKillEntry>::iterator itKill(m_vMatchKillLog.begin()); itKill != m_vMatchKillLog.end(); ++itKill)
    {
        tstring sKill(_T("frags[") + XtoA(uiKillCount) + _T("]"));

        CPlayer *pKiller(GetPlayer(itKill->m_iKillerClientNumber));
        CPlayer *pVictim(GetPlayer(itKill->m_iVictimClientNumber));
        if (pVictim == NULL)
            continue;

        int iKillerID(pKiller ? pKiller->GetAccountID() : -1);
        if (itKill->m_eKillCode != HKK_HERO)
            iKillerID = itKill->m_eKillCode;

        pRequest->AddVariable(sKill + _T("[killer_id]"), iKillerID);
        pRequest->AddVariable(sKill + _T("[target_id]"), pVictim ? pVictim->GetAccountID() : -1);
        pRequest->AddVariable(sKill + _T("[secs]"), INT_ROUND(MsToSec(itKill->m_uiTimeStamp)));
        
        uint uiAssistCount(0);
        for (ivector_cit itAssist(itKill->m_vAssists.begin()); itAssist != itKill->m_vAssists.end(); ++itAssist)
        {
            CPlayer *pAssister(GetPlayer(*itAssist));
            if (pAssister != NULL)
                pRequest->AddVariable(sKill + _T("[assisters][") + XtoA(uiAssistCount) + _T("]"), pAssister->GetAccountID());

            ++uiAssistCount;
        }

        ++uiKillCount;
    }

    pRequest->SendPostRequest();
    pRequest->Wait();
    CPHPData phpResponse(pRequest->GetResponse());
    phpResponse.Print();
#endif
}


/*====================
  CGameServer::TryAddClient
  ====================*/
CPlayer*    CGameServer::TryAddClient(CClientConnection *pClientConnection)
{
    if (pClientConnection == NULL)
        return NULL;

    // If the server is idle (no match has been hosted) only a host is allowed to connect
    if (GetGamePhase() == GAME_PHASE_IDLE && !pClientConnection->HasFlags(CLIENT_CONNECTION_GAME_HOST))
    {
        pClientConnection->Disconnect(_T("disconnect_not_host"));
        return NULL;
    }

    if (GetGamePhase() >= GAME_PHASE_WAITING_FOR_PLAYERS)
    {
        // Trial account restrictions
        if (pClientConnection->HasFlags(CLIENT_CONNECTION_TRIAL) && !m_pHostServer->GetNoStats())
        {
            pClientConnection->Disconnect(L"disconnect_trial_not_allowed");
            return NULL;
        }
    }

    // If the game is in progress, only reconnecting players are allowed
    CPlayer *pPlayer(NULL);
    if (GetGamePhase() > GAME_PHASE_WAITING_FOR_PLAYERS)
    {
        for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
        {
            if (it->second->GetAccountID() == pClientConnection->GetAccountID())
            {
                if (it->second->HasFlags(PLAYER_FLAG_TERMINATED))
                {
                    pClientConnection->Disconnect(_T("disconnect_terminated"));
                    return NULL;
                }

                pPlayer = it->second;
                break;
            }
        }

        if (pPlayer == NULL)
        {
            // Ignore "late" connection from host
            if (!pClientConnection->HasFlags(CLIENT_CONNECTION_GAME_HOST))
            {
                pClientConnection->Disconnect(_T("disconnect_game_in_progress"));
                return NULL;
            }
        }
        else if (pPlayer->GetClientNumber() != pClientConnection->GetClientNum())
        {
            pClientConnection->Disconnect(_T("disconnect_client_number_mismatch"));
            return NULL;
        }
        else
        {
            return pPlayer;
        }
    }

    // Check for a full server (but admins can always connect)
    if (GetGamePhase() > GAME_PHASE_IDLE && GetClientCount() >= GetMaxClients() && !pClientConnection->HasFlags(CLIENT_CONNECTION_ADMIN))
    {
        pClientConnection->Disconnect(_T("disconnect_server_full"));
        return NULL;
    }

    // If the countdown has started, cancel it
    if (GetGamePhase() == GAME_PHASE_WAITING_FOR_PLAYERS && !m_pHostServer->IsArrangedMatch())
        SetGamePhaseEndTime(INVALID_TIME);

    // Allocate a new player
    IGameEntity* pNewPlayer(m_pServerEntityDirectory->Allocate(Player));
    if (pNewPlayer == NULL)
    {
        pClientConnection->Disconnect(_T("disconnect_server_error"));
        return NULL;
    }

    pPlayer = pNewPlayer->GetAsPlayer();
    m_mapClients[pClientConnection->GetClientNum()] = pPlayer;

    pPlayer->Initialize(pClientConnection, m_pHostServer);

    pClientConnection->SetStream(0);

#if 0
    CTeamInfo *pTeam(GetTeam(TEAM_PASSIVE));
    if (pTeam != NULL)
        pTeam->AddClient(pPlayer->GetClientNumber());
#endif

    map<int, float>::iterator itRating(m_mapPlayerRatings.find(pClientConnection->GetAccountID()));
    if (itRating != m_mapPlayerRatings.end())
    {
        if (m_pHostServer->IsArrangedMatch())
            pPlayer->SetSecretRank(itRating->second);
        else
            pPlayer->SetRank(itRating->second);
    }

    map<int, uint>::iterator itWins(m_mapPlayerWins.find(pClientConnection->GetAccountID()));
    if (itWins != m_mapPlayerWins.end())
        pPlayer->SetAccountWins(itWins->second);

    map<int, uint>::iterator itLosses(m_mapPlayerLosses.find(pClientConnection->GetAccountID()));
    if (itLosses != m_mapPlayerLosses.end())
        pPlayer->SetAccountLosses(itLosses->second);

    map<int, uint>::iterator itDisconnects(m_mapPlayerDisconnects.find(pClientConnection->GetAccountID()));
    if (itDisconnects != m_mapPlayerDisconnects.end())
        pPlayer->SetAccountDisconnects(itDisconnects->second);

    map<int, uint>::iterator itKills(m_mapPlayerKills.find(pClientConnection->GetAccountID()));
    if (itKills != m_mapPlayerKills.end())
        pPlayer->SetAccountKills(itKills->second);

    map<int, uint>::iterator itAssists(m_mapPlayerAssists.find(pClientConnection->GetAccountID()));
    if (itAssists != m_mapPlayerAssists.end())
        pPlayer->SetAccountAssists(itAssists->second);
    
    map<int, uint>::iterator itDeaths(m_mapPlayerDeaths.find(pClientConnection->GetAccountID()));
    if (itDeaths != m_mapPlayerDeaths.end())
        pPlayer->SetAccountDeaths(itDeaths->second);

    map<int, float>::iterator itEmPercent(m_mapPlayerEmPercent.find(pClientConnection->GetAccountID()));
    if (itEmPercent != m_mapPlayerEmPercent.end())
        pPlayer->SetAccountEmPercent(itEmPercent->second);

    map<int, float>::iterator itExpMin(m_mapPlayerExpMin.find(pClientConnection->GetAccountID()));
    if (itExpMin != m_mapPlayerExpMin.end())
        pPlayer->SetAccountExpMin(itExpMin->second);

    map<int, float>::iterator itGoldMin(m_mapPlayerGoldMin.find(pClientConnection->GetAccountID()));
    if (itGoldMin != m_mapPlayerGoldMin.end())
        pPlayer->SetAccountGoldMin(itGoldMin->second);

    if (m_pHostServer->IsArrangedMatch() || m_pHostServer->IsTournMatch())
        ChangeTeam(pPlayer->GetClientNumber(), m_pHostServer->GetTeamFromRoster(pClientConnection->GetAccountID(), TEAM_INVALID), m_pHostServer->GetSlotFromRoster(pClientConnection->GetAccountID()));

    CBufferDynamic buffer;
    buffer << GAME_CMD_LOBBY_CONNECT_MESSAGE << TStringToUTF8(pPlayer->GetName()) << byte(0) << ushort(pPlayer->GetFlags());
    BroadcastGameData(buffer, true);

    return pPlayer;
}


/*====================
  CGameServer::IsPlayerReconnecting
  ====================*/
bool    CGameServer::IsPlayerReconnecting(int iAccountID)
{
    for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
    {
        if (it->second->GetAccountID() == iAccountID)               
        {       
            CPlayer *pPlayer(it->second);
            if (pPlayer == NULL)
                continue;
            
            if (pPlayer->HasFlags(PLAYER_FLAG_WAS_CONNECTED))
                return true;                    
        }       
    }
    
    return false;
}


/*====================
  CGameServer::IsDuplicateAccountInGame
  ====================*/
bool    CGameServer::IsDuplicateAccountInGame(int iAccountID)
{
    for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
    {
        if (it->second->GetAccountID() == iAccountID)               
        {           
            CPlayer *pPlayer(it->second);
            if (pPlayer == NULL)
                continue;
            
            // If the account_id matches another client, but the flags say they weren't previously connected
            // they are trying to connect two players to the server at at the same time on the same account_id.
            // Note that the only way they would have this flag is if they were in game and disconnected before, 
            // so if this is their first connection it will boot them.
            if (!pPlayer->HasFlags(PLAYER_FLAG_WAS_CONNECTED))
                return true;
        }       
    }
    
    return false;
}


/*====================
  CGameServer::RemoveDuplicateAccountsInGame
  ====================*/
bool    CGameServer::RemoveDuplicateAccountsInGame(int iAccountID)
{
    if (m_pHostServer == NULL) return false;
    
    for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
    {
        if (it->second->GetAccountID() == iAccountID)               
        {           
            CPlayer *pPlayer(it->second);
            if (pPlayer == NULL)
                continue;
                
            CClientConnection *pClientConnection(m_pHostServer->GetClient(pPlayer->GetClientNumber()));
            if (pClientConnection == NULL)
                continue;
            
            // Disconnect any players in game with this AccountID, precedence is given to the last player connecting, 
            // not the first to avoid problems where players can't reconnect until the first one times out
            pClientConnection->Disconnect(_T("disconnect_duplicate_connect"));          
        }       
    }
    
    return true;
}


/*====================
  CGameServer::AddClient
  ====================*/
bool    CGameServer::AddClient(CClientConnection *pClientConnection)
{
    IGame *pGame(Game.GetCurrentGamePointer());
    Game.SetCurrentGamePointer(this);

    CPlayer *pPlayer(TryAddClient(pClientConnection));
    if (pPlayer == NULL)
        return false;

    if (ReplayManager.IsPlaying())
    {
        Game.SetCurrentGamePointer(pGame);
        pClientConnection->SetFlags(CLIENT_CONNECTION_IN_GAME);
        return true;
    }

    if (pClientConnection->GetClientNum() == m_iCreatorClientNum)
        pClientConnection->SetFlags(CLIENT_CONNECTION_GAME_HOST);

    m_GameLog.WritePlayer(GAME_LOG_PLAYER_CONNECT, pPlayer);

    // TODO: Get rid of this
    if (GetGamePhase() >= GAME_PHASE_ENDED)
    {
        // Notify client who the winner is
        CBufferFixed<9> buffer;
        buffer << GAME_CMD_END_GAME << GetWinningTeam() << GetFinalMatchTime();
        SendGameData(pClientConnection->GetClientNum(), buffer, true);
    }
    //

    pClientConnection->SetFlags(CLIENT_CONNECTION_IN_GAME);
    pPlayer->Connected(GetGameTime());

    CGameInfo *pGameInfo(GetGameInfo());
    if (pGameInfo != NULL)
        pGameInfo->ExecuteActionScript(ACTION_SCRIPT_ADD_PLAYER, pGameInfo, NULL, pPlayer, V3_ZERO);

    Game.SetCurrentGamePointer(pGame);
    return true;
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

        CPlayer *pClient(NULL);
    
        PlayerMap_it itFind(m_mapClients.find(iClientNumber));
        if (itFind == m_mapClients.end())
            return;
        else
            pClient = itFind->second;

        Game.SetCurrentGamePointer(pGame);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CGameServer::ReauthClient() - "), NO_THROW);
    }
}


/*====================
  CGameServer::RemoveClient
  ====================*/
void    CGameServer::RemoveClient(int iClientNum, const tstring &sReason)
{
    try
    {
        PlayerMap_it itClient(m_mapClients.find(iClientNum));
        if (itClient == m_mapClients.end())
            EX_ERROR(_T("Client doesnt exist"));
            
        CPlayer *pPlayer(itClient->second);
        if (pPlayer->IsDisconnected())
            EX_ERROR(_T("Client is already disconnected"));

        if (pPlayer->ShouldMoveUnitsOnDisconnect())
            pPlayer->MoveUnitsToSafety(iClientNum);

        CTeamInfo *pTeam(GetTeam(pPlayer->GetTeam()));

        pPlayer->Disconnected(GetGameTime(), sv_maxDisconnectedTime);

        if (HasFlags(GAME_FLAG_CONCEDED) || (pTeam != NULL && pTeam->HasFlags(TEAM_FLAG_ABANDONED)))
            pPlayer->SetFlags(PLAYER_FLAG_EXCUSED);

        // try to handle the various disconnects, whether they be in lobby, in game or a ragequit
        CBufferFixed<5> buffer;         

        if (!pPlayer->HasFlags(PLAYER_FLAG_KICKED))
        {
            // if they aren't yet in the game and they disconnect, show them the lobby disconnect message but don't write to the gamelog
            // note that this lobby is hardly different from the in game disconnect message, and could probably be consolidated in the future
            if (GetGamePhase() <= GAME_PHASE_WAITING_FOR_PLAYERS)
            {
                buffer << GAME_CMD_LOBBY_DISCONNECT_MESSAGE << pPlayer->GetClientNumber();
                BroadcastGameData(buffer, true);
            }
                        
            if (GetGamePhase() == GAME_PHASE_ACTIVE && !pPlayer->HasFlags(PLAYER_FLAG_EXCUSED))
            {
                IHeroEntity *pHero(pPlayer->GetHero());

                if (pHero != NULL && pHero->GetDeathTimeStamp() != INVALID_TIME)
                {
                    CBufferFixed<1> quitBuffer;
                    quitBuffer << GAME_CMD_RAGE_QUIT_MESSAGE;
                    BroadcastGameData(quitBuffer, true);
                }
            }
        }

        // Handle timeout/disconnect messages in game and log them too (demos now record bans)
        if (GetGamePhase() >= GAME_PHASE_HERO_BAN)
        {
            buffer.Clear();

            if (sReason == _T("Connection timed out"))
            {
                buffer << GAME_CMD_TIMEDOUT_MESSAGE << pPlayer->GetClientNumber();
                m_GameLog.WritePlayer(GAME_LOG_PLAYER_TIMEDOUT, pPlayer);
            }
            else
            {
                buffer << GAME_CMD_DISCONNECT_MESSAGE << pPlayer->GetClientNumber();
                m_GameLog.WritePlayer(GAME_LOG_PLAYER_DISCONNECT, pPlayer); 
            }
            
            BroadcastGameData(buffer, true);
        }

        
        // Purge players who disconnect before the game starts
        if (GetGamePhase() <= GAME_PHASE_WAITING_FOR_PLAYERS)
        {
            // they are disconnecting from the game, remove their host flags just to be safe for the assign a new host functionality
            CClientConnection *pClientConnection(m_pHostServer->GetClient(iClientNum));
            
            pPlayer->RemoveFlags(PLAYER_FLAG_HOST);
            
            if (pClientConnection != NULL)
                pClientConnection->RemoveFlags(CLIENT_CONNECTION_GAME_HOST);                
        
            m_pHostServer->ReleaseClientID(iClientNum);

            if (pTeam != NULL)
                pTeam->RemoveClient(pPlayer->GetClientNumber());

            DeleteEntity(pPlayer);

            m_mapClients.erase(itClient);

            // if the client disconnecting is the host, instead of disbanding the game, try to assign a new host to take over
            CClientConnection *pClient(GetHostServer()->GetClient(iClientNum));
            if (iClientNum == m_iCreatorClientNum || (pClient != NULL && pClient->HasFlags(CLIENT_CONNECTION_LOCAL)))
            {
                bool bFoundNewHost(false);
                CPlayer *pNewPlayerHost(NULL);
                CClientConnection *pNewClientConnection(NULL);

                // try to find a new player to assign as the host
                for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)             
                {
                    pNewPlayerHost = GetPlayer(it->second->GetClientNumber());
                    pNewClientConnection = m_pHostServer->GetClient(it->second->GetClientNumber());
                    
                    if (pNewClientConnection != NULL && pNewPlayerHost != NULL && it->second->GetClientNumber() != -1)
                    {                   
                        pNewPlayerHost->SetFlags(PLAYER_FLAG_HOST);
                        pNewClientConnection->SetFlags(CLIENT_CONNECTION_GAME_HOST);
                        
                        m_iCreatorClientNum = it->second->GetClientNumber();
                        
                        buffer.Clear();                     
                        buffer << GAME_CMD_LOBBY_ASSIGNED_HOST_MESSAGE << it->second->GetClientNumber();
                        BroadcastGameData(buffer, true);
                        
                        bFoundNewHost = true;
                        break;
                    }
                }
                
                // didn't find another player to assign, reset the server
                if (!bFoundNewHost)
                    Reset();
            }
            
            if (!m_pHostServer->IsArrangedMatch())
                SetGamePhaseEndTime(INVALID_TIME);
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CGameServer::RemoveClient() - "), NO_THROW);
    }
}

/*====================
  CGameServer::ClientTimingOut
  ====================*/
void    CGameServer::ClientTimingOut(int iClientNum)
{
    try
    {
        PlayerMap_it itClient(m_mapClients.find(iClientNum));
        if (itClient == m_mapClients.end())
            EX_ERROR(_T("Client doesnt exist"));
            
        CPlayer *pPlayer(itClient->second);
        if (pPlayer->ShouldMoveUnitsOnDisconnect())
            pPlayer->MoveUnitsToSafety(iClientNum);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CGameServer::ClientTimingOut() - "), NO_THROW);
    }
}


/*====================
  CGameServer::GetMaxClients
  ====================*/
uint    CGameServer::GetMaxClients() const
{
#ifdef K2_CLIENT
    return 1;
#else
    return (GetTeamSize() * 2) + GetMaxSpectators() + GetMaxReferees();
#endif
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

        CPlayer *pClient(GetPlayer(iClientNum));
        if (pClient == NULL)
            EX_ERROR(_T("Received snapshot with invalid client number ") + XtoA(iClientNum));

        pClient->SetLastInputTime(Game.GetGameTime());

        uint uiRealGameTime(GetGameTime());
        uint uiRealFrameLength(GetFrameLength());

        SetGameTime(snapshot.GetTimeStamp());
        SetFrameLength(snapshot.GetFrameLength());

        pClient->ReadClientSnapshot(snapshot);

        SetGameTime(uiRealGameTime);
        SetFrameLength(uiRealFrameLength);
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
void    CGameServer::SellItem(int iClientNum, uint uiUnitIndex, int iSlot)
{
    if (m_pHostServer->GetPaused())
        return;

    if (!IS_ITEM_SLOT(iSlot))
        return;

    CPlayer *pPlayer(GetPlayerFromClientNumber(iClientNum));
    if (pPlayer == NULL)
        return;

    IHeroEntity *pHero(pPlayer->GetHero());
    if (pHero == NULL || pHero->GetStatus() != ENTITY_STATUS_ACTIVE)
        return;

    IUnitEntity *pControlUnit(GetUnitEntity(uiUnitIndex));
    if (pControlUnit == NULL)
        return;

    IUnitEntity *pSrcUnit(iSlot >= INVENTORY_START_STASH && iSlot <= INVENTORY_END_STASH ? pHero : pControlUnit);
    if (pSrcUnit == NULL)
        return;

    IEntityItem *pItem(pSrcUnit->GetItem(iSlot));
    if (pItem == NULL)
        return;

    if (!pControlUnit->CanSellItem(pItem, iClientNum))
        return;

    m_GameLog.WriteItem(GAME_LOG_ITEM_SELL, pItem);

    CTeamInfo *pTeam(GetTeam(pPlayer->GetTeam()));

    if (pItem->WasPurchasedRecently())
    {
        if (pTeam != NULL)
        {
            CShopInfo *pShop(pTeam->GetShopInfo());

            if (pShop != NULL)
            {
                uint uiCharges = 1;
                if (pItem->GetInitialCharges() > 0)
                    uiCharges = pItem->GetCharges() / pItem->GetInitialCharges();
                pShop->ReplenishItem(pItem->GetType(), uiCharges);
            }
        }
    }

    if (pItem->GetPurchaserClientNumber() == -1)
    {
        if (pTeam != NULL)
            pTeam->DistributeGold(pItem->GetValue());
    }
    else
    {
        int iValue(pItem->GetValue());

        pPlayer->GiveGold(iValue, pControlUnit);
        pPlayer->AdjustStat(PLAYER_STAT_GOLD_SPENT, -iValue);
    }

    pSrcUnit->RemoveItem(iSlot);
}


/*====================
  CGameServer::MoveItem
  ====================*/
void    CGameServer::MoveItem(int iClientNum, uint uiUnitIndex, int iSlot0, int iSlot1)
{
    if (m_pHostServer->GetPaused())
        return;

    if (!IS_ITEM_SLOT(iSlot0) || ! IS_ITEM_SLOT(iSlot1))
        return;

    CPlayer *pPlayer(GetPlayerFromClientNumber(iClientNum));
    if (pPlayer == NULL)
        return;

    IUnitEntity *pControlUnit(GetUnitEntity(uiUnitIndex));
    if (pControlUnit == NULL || !pControlUnit->CanActOnOrdersFrom(iClientNum) || pControlUnit->IsIllusion() || pControlUnit->HasUnitFlags(UNIT_FLAG_LOCKED_BACKPACK))
        return;
            
    if ((IS_STASH_SLOT(iSlot0) || IS_STASH_SLOT(iSlot1)) && !pControlUnit->GetStashAccess())
        return;

    IUnitEntity *pSrcUnit(IS_STASH_SLOT(iSlot0) ? pPlayer->GetHero() : pControlUnit);
    IUnitEntity *pDstUnit(IS_STASH_SLOT(iSlot1) ? pPlayer->GetHero() : pControlUnit);
    if (pSrcUnit == NULL || pDstUnit == NULL)
        return;

    if (pSrcUnit == pDstUnit)
        pSrcUnit->SwapItem(iClientNum, iSlot0, iSlot1);
    else if (pDstUnit->GetItem(iSlot1) != NULL)
    {
        IEntityItem *pSrcItem(pSrcUnit->GetItem(iSlot0));
        IEntityItem *pDstItem(pDstUnit->GetItem(iSlot1));
        if (pSrcUnit->CanCarryItem(pDstItem) && pDstUnit->CanCarryItem(pSrcItem))
            Game.SwapItem(iClientNum, pSrcItem, pDstItem);
    }
    else
    {
        IEntityItem *pSrcItem(pSrcUnit->GetItem(iSlot0));
        if (pDstUnit->CanCarryItem(pSrcItem))
            pDstUnit->TransferItem(iClientNum, pSrcItem, iSlot1);
    }
}


/*====================
  CGameServer::PurchaseItem
  ====================*/
bool    CGameServer::PurchaseItem(int iClientNum, uint uiUnitIndex, ushort unShop, int iSlot)
{
    try
    {
        if (m_pHostServer->GetPaused())
            return false;

        // Lookup shop
        CShopDefinition *pShop(EntityRegistry.GetDefinition<CShopDefinition>(unShop));
        if (pShop == NULL)
            EX_WARN(_T("Invalid shop type: ") + XtoA(unShop));

        // Lookup item
        ushort unItemID(INVALID_ENT_TYPE);
        const tsvector &vItems(pShop->GetItems());
        if (iSlot >= 0 && iSlot < int(vItems.size()))
            unItemID = EntityRegistry.LookupID(vItems[iSlot]);

        if (unItemID == INVALID_ENT_TYPE)
            EX_ERROR(_T("Invalid item: ") + XtoA(unItemID));

        CItemDefinition *pItemDef(EntityRegistry.GetDefinition<CItemDefinition>(unItemID));
        if (pItemDef == NULL)
            EX_WARN(_T("No definition found for item"));
        if (pItemDef->GetAutoAssemble())
            EX_WARN(_T("This item auto-assembles"));
        
        Console << _T("Purchase request: ") << iClientNum << SPACE << uiUnitIndex << SPACE << pItemDef->GetName() << newl;

        // Validate player
        CPlayer *pPlayer(GetPlayerFromClientNumber(iClientNum));
        if (pPlayer == NULL)
            EX_ERROR(_T("Could not find entity for client: ") + XtoA(iClientNum));

        // Validate unit making the purchase
        IUnitEntity *pUnit(GetUnitEntity(uiUnitIndex));
        if (pUnit == NULL)
            EX_ERROR(_T("Could not find entity for unit: ") + XtoA(uiUnitIndex));

        if (pUnit->HasUnitFlags(UNIT_FLAG_LOCKED_BACKPACK))
            EX_ERROR(_T("This unit has a locked backpack"));

        if (pUnit->IsIllusion())
            EX_ERROR(_T("Illusions cannot shop") + XtoA(uiUnitIndex));

        if (!pUnit->CanReceiveOrdersFrom(iClientNum))
            EX_WARN(_T("Player can not purchase for this unit because they do not own it"));

        if (!pUnit->CanAccessShop(unShop) && (!pShop->GetRecommendedItems() || !pUnit->CanAccessItem(pItemDef->GetName())))
            EX_WARN(_T("Player does not have access to this shop"));

        if (!pUnit->CanAccessItem(pItemDef->GetName()))
            EX_WARN(_T("Player does not have access to this item"));

        // Check for stash access
        IUnitEntity *pStash(pPlayer->GetHero());

        // Find a slot in the inventory
        IUnitEntity *pTarget(NULL);

        bool bUseBackpack(pUnit->CanAccessItemLocal(pItemDef->GetName()));
        bool bUseStash(pStash && (pUnit->GetStashAccess() || !bUseBackpack));
        bool bStacked(false);
        int iTargetSlot(-1);

        if (bUseBackpack)
        {
            for (int iTrySlot(INVENTORY_START_BACKPACK); iTrySlot <= INVENTORY_BACKPACK_PROVISIONAL; ++iTrySlot)
            {
                // Determine what unit's inventory to look at
                pTarget = IS_STASH_SLOT(iTrySlot) ? pStash : pUnit;
                if (pTarget == NULL)
                    continue;

                // Check for an empty slot
                IEntityItem *pItem(pTarget->GetItem(iTrySlot));
                if (pItem == NULL)
                {
                    if (iTargetSlot == -1)
                        iTargetSlot = iTrySlot;

                    continue;
                }

                // Check for a stackable item
                if (pItem->CanStack(unItemID, iClientNum))
                {
                    iTargetSlot = iTrySlot;
                    bStacked = true;
                    break;
                }
            }
        }

        if (bUseStash)
        {
            for (int iTrySlot(INVENTORY_START_STASH); iTrySlot <= INVENTORY_STASH_PROVISIONAL; ++iTrySlot)
            {
                // Determine what unit's inventory to look at
                pTarget = IS_STASH_SLOT(iTrySlot) ? pStash : pUnit;
                if (pTarget == NULL)
                    continue;

                // Check for an empty slot
                IEntityItem *pItem(pTarget->GetItem(iTrySlot));
                if (pItem == NULL)
                {
                    if (iTargetSlot == -1)
                        iTargetSlot = iTrySlot;

                    continue;
                }

                // Check for a stackable item
                if (pItem->CanStack(unItemID, iClientNum))
                {
                    iTargetSlot = iTrySlot;
                    bStacked = true;
                    break;
                }
            }
        }

        if (iTargetSlot == -1)
            EX_WARN(_T("No empty slot for item"));

        // Check cost
        ushort unCost(pItemDef->GetCost());
        if (pPlayer->GetGold() < unCost)
            EX_WARN(_T("Player can not afford this item"));

        // Check whether the item is in stock or not
        bool bInStock(true);
        CTeamInfo *pTeam(Game.GetTeam(pPlayer->GetTeam()));

        if (pTeam != NULL)
        {
            CShopInfo *pShop(pTeam->GetShopInfo());

            if (pShop != NULL)
                bInStock = pShop->PurchaseItem(unItemID);
        }

        if (!bInStock)
            EX_WARN(_T("Item is not in stock"));

        // Spawn the item
        pTarget = IS_STASH_SLOT(iTargetSlot) ? pStash : pUnit;
        IEntityTool *pTool(pTarget->GiveItem(iTargetSlot, unItemID, false));
        if (pTool == NULL)
            EX_WARN(_T("Item generation failed"));
        IEntityItem *pItem(pTool->GetAsItem());
        if (pItem == NULL)
            EX_WARN(_T("Item generation failed"));

        if (!bStacked)
            pItem->SetPurchaseTime(Game.GetGameTime());
        pItem->SetPurchaserClientNumber(iClientNum);

        int iResultSlot(pTarget->CheckRecipes(pItem->GetSlot()));

        // If the item ended up in the provisional slot, try to find a valid place for it
        if (IS_PROVISIONAL_SLOT(iResultSlot))
        {
            iTargetSlot = -1;

            if (bUseBackpack)
            {
                for (int iTrySlot(INVENTORY_START_BACKPACK); iTrySlot <= INVENTORY_BACKPACK_PROVISIONAL; ++iTrySlot)
                {
                    // Skip provisional slots
                    if (IS_PROVISIONAL_SLOT(iTrySlot))
                        continue;

                    // Determine what unit's inventory to look at
                    IUnitEntity *pTestEntity(IS_STASH_SLOT(iTrySlot) ? pStash : pUnit);
                    if (pTestEntity == NULL)
                        continue;

                    if (pTestEntity->GetItem(iTrySlot) == NULL)
                    {
                        iTargetSlot = iTrySlot;
                        pTarget = pTestEntity;
                        break;
                    }
                }
            }

            if (bUseStash && iTargetSlot == -1)
            {
                for (int iTrySlot(INVENTORY_START_STASH); iTrySlot <= INVENTORY_STASH_PROVISIONAL; ++iTrySlot)
                {
                    // Skip provisional slots
                    if (IS_PROVISIONAL_SLOT(iTrySlot))
                        continue;

                    // Determine what unit's inventory to look at
                    IUnitEntity *pTestEntity(IS_STASH_SLOT(iTrySlot) ? pStash : pUnit);
                    if (pTestEntity == NULL)
                        continue;

                    if (pTestEntity->GetItem(iTrySlot) == NULL)
                    {
                        iTargetSlot = iTrySlot;
                        pTarget = pTestEntity;
                        break;
                    }
                }
            }

            if (iTargetSlot == -1)
            {
                pTarget->RemoveItem(iResultSlot);
                EX_WARN(_T("No empty slot for item"));
            }
            
            if (pTarget == pUnit)
                pUnit->SwapItem(iClientNum, iResultSlot, iTargetSlot);
            else
                pTarget->TransferItem(iClientNum, pUnit->GetItem(iResultSlot), iTargetSlot);
        }
        
        IUnitEntity *pOwner(pItem->GetOwner());
        if (pOwner != NULL)     
            pItem->ExecuteActionScript(ACTION_SCRIPT_PURCHASED, pOwner, pOwner->GetPosition());
        
        pPlayer->SpendGold(unCost);
        m_GameLog.WriteItem(GAME_LOG_ITEM_PURCHASE, pItem);

        pItem->UpdateApparentCooldown();

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CGameServer::PurchaseItem() - "), NO_THROW);
        return false;
    }

    return false;
}


/*====================
  CGameServer::RequestChangeTeam
  ====================*/
void    CGameServer::RequestChangeTeam(int iClientID, uint uiTeamID, uint uiSlot, bool bBecomingReferee)
{
    if (GetGamePhase() != GAME_PHASE_WAITING_FOR_PLAYERS ||
        uiTeamID == TEAM_INVALID ||
        m_pHostServer->IsArrangedMatch() ||
        m_pHostServer->IsTournMatch())
        return;

    CPlayer *pPlayer(GetPlayer(iClientID));
    if (pPlayer == NULL)
    {
        Console.Warn << _T("No entity found for client #") << iClientID << newl;
        return;
    }
    
    // We're trying to change this client to TEAM_SPECTATOR
    if (uiTeamID == TEAM_SPECTATOR)
    {
        // No more slots available for spectators
        if (!bBecomingReferee && GetCurrentSpectatorCount() >= MAX_TOTAL_SPECTATORS)
            return;
        
        // No more slots available for referees
        if (bBecomingReferee && GetCurrentRefereeCount() >= MAX_TOTAL_REFEREES)
            return;
    }
    
    // reserve the referee slots.
    if (pPlayer->GetTeam() != TEAM_SPECTATOR && uiTeamID == TEAM_SPECTATOR && !bBecomingReferee)
    {
        CTeamInfo *pNewTeam(GetTeam(uiTeamID));
        if (pNewTeam->GetNumClients() + GetMaxReferees() - GetNumReferees() >= pNewTeam->GetTeamSize())
            return;
    }

    if (GetPhaseEndTime() != INVALID_TIME)
    {
        SetGamePhaseEndTime(INVALID_TIME);
        
        CBufferFixed<1> buffer;
        buffer << GAME_CMD_MATCH_CANCEL_MESSAGE;
        BroadcastGameData(buffer, true);
    }

    ChangeTeam(iClientID, uiTeamID, uiSlot);
}


/*====================
  CGameServer::ChangeTeam
  ====================*/
void    CGameServer::ChangeTeam(int iClientID, uint uiTeamID, uint uiSlot)
{
    CPlayer *pPlayer(GetPlayer(iClientID));
    if (pPlayer == NULL)
    {
        Console.Warn << _T("No entity found for client #") << iClientID << newl;
        return;
    }

    CTeamInfo *pOldTeam(GetTeam(pPlayer->GetTeam()));

    if (pPlayer->IsReferee() && uiTeamID != TEAM_SPECTATOR)
    {
        // demote referee
        CBufferFixed<6> buffer;
        buffer << GAME_CMD_LOBBY_ASSIGNED_REFEREE_MESSAGE2 << byte(0) << iClientID;
        BroadcastGameData(buffer, true);                                            
    }

    if (uiTeamID == TEAM_INVALID || uiTeamID == TEAM_PASSIVE)
    {
        if (pOldTeam != NULL)
            pOldTeam->RemoveClient(iClientID);

        pPlayer->ClearAffiliations();
        pPlayer->SetTeam(uiTeamID);
        return;
    }

    CTeamInfo *pNewTeam(GetTeam(uiTeamID));
    if (pNewTeam == NULL || !pNewTeam->CanJoinTeam(iClientID, uiSlot))
        return;

    if (pOldTeam != NULL)
        pOldTeam->RemoveClient(iClientID);

    pNewTeam->AddClient(iClientID, uiSlot);

    // Stop countdown
    if (GetGamePhase() == GAME_PHASE_WAITING_FOR_PLAYERS && !m_pHostServer->IsArrangedMatch())
        SetGamePhaseEndTime(INVALID_TIME);

    Console << _T("Client #") << iClientID << _T(" joined team: ") << uiTeamID << newl;
    m_GameLog.WritePlayer(GAME_LOG_PLAYER_TEAM_CHANGE, pPlayer);

    CClientConnection *pClientConnection(m_pHostServer->GetClient(pPlayer->GetClientNumber()));
    if (pClientConnection != NULL)
    {
        if (uiTeamID == 1)
            pClientConnection->SetStream(1);
        else if (uiTeamID == 2)
            pClientConnection->SetStream(2);
        else
            pClientConnection->SetStream(0);
    }
}

/*====================
  CGameServer::RequestAssignFirstBanTeam
  ====================*/
void    CGameServer::RequestAssignFirstBanTeam(int iClientID, uint uiTeamID)
{
    if (GetGamePhase() != GAME_PHASE_WAITING_FOR_PLAYERS ||
        (uiTeamID != 1 && uiTeamID != 2) ||
        m_pHostServer->IsArrangedMatch())
        return;

    // only allow the host to assign the first ban team.
    CClientConnection *pClientConnection(m_pHostServer->GetClient(iClientID));
    if (pClientConnection != NULL && pClientConnection->HasFlags(CLIENT_CONNECTION_GAME_HOST))
    {
        if (GetPhaseEndTime() != INVALID_TIME)
        {
            SetGamePhaseEndTime(INVALID_TIME);
            
            CBufferFixed<1> buffer;
            buffer << GAME_CMD_MATCH_CANCEL_MESSAGE;
            BroadcastGameData(buffer, true);
        }

        AssignFirstBanTeam(uiTeamID);
    }
}

/*====================
  CGameServer::AssignFirstBanTeam
  ====================*/
void    CGameServer::AssignFirstBanTeam(uint uiTeamID)
{
    CGameInfo* pGameInfo(GetGameInfo());
    if (pGameInfo != NULL)
    {
        pGameInfo->SetFirstBanTeam(uiTeamID);
            
        CBufferFixed<5> buffer;
        buffer << GAME_CMD_ASSIGN_FIRST_BAN_TEAM << uiTeamID;
        BroadcastGameData(buffer, true);
    }
}


/*====================
  CGameServer::BuyBackHero
  ====================*/
bool    CGameServer::BuyBackHero(int iClientNum, uint uiUnitIndex)
{
    try
    {
        Console << _T("Buy-back request: ") << iClientNum << SPACE << uiUnitIndex << newl;

        if (m_pHostServer->GetPaused())
            return false;

        CPlayer *pPlayer(GetPlayerFromClientNumber(iClientNum));
        if (pPlayer == NULL)
            EX_ERROR(_T("Could not find entity for client: ") + XtoA(iClientNum));

        IUnitEntity *pUnit(GetUnitEntity(uiUnitIndex));
        if (pUnit == NULL)
            EX_ERROR(_T("Could not find entity for unit: ") + XtoA(uiUnitIndex));

        if (!pUnit->IsHero())
            EX_ERROR(_T("Unit is not a hero: ") + XtoA(uiUnitIndex));

        if (pUnit->GetStatus() != ENTITY_STATUS_CORPSE && pUnit->GetStatus() != ENTITY_STATUS_DORMANT)
            EX_ERROR(_T("Unit is not dead: ") + XtoA(uiUnitIndex));

        if (pUnit->GetOwnerClientNumber() != pPlayer->GetClientNumber())
            EX_WARN(_T("Player can not buy-back this unit because they do not own it"));

        ushort unCost(pUnit->GetAsHero()->GetBuyBackCost());
        if (!pPlayer->SpendGold(unCost))
            EX_WARN(_T("Player can not afford to buy-back"));

        m_GameLog.WritePlayer(GAME_LOG_PLAYER_BUYBACK, pPlayer, XtoA(unCost));
        pUnit->GetAsHero()->Respawn();
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CGameServer::BuyBackHero() - "), NO_THROW);
        return false;
    }

    return false;
}


/*====================
  CGameServer::SetSelection
  ====================*/
void    CGameServer::SetSelection(int iClientNum, const uiset &setSelection)
{
    try
    {
        CPlayer *pPlayer(GetPlayerFromClientNumber(iClientNum));
        if (pPlayer == NULL)
            EX_ERROR(_T("Could not find entity for client: ") + XtoA(iClientNum));

        pPlayer->SetSelection(setSelection);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CGameServer::SetSelection() - "), NO_THROW);
    }
}


/*====================
  CGameServer::ProcessCallVoteRequest
  ====================*/
void    CGameServer::ProcessCallVoteRequest(CPlayer *pPlayer, CPacket &pkt)
{
    // Read packet data
    byte yVoteType(pkt.ReadByte(VOTE_TYPE_INVALID));
    int iTarget(pkt.ReadInt(-1));

    // Validate player calling the vote
    if (pPlayer == NULL)
        return;
    if (pPlayer->GetTeam() < TEAM_1 || pPlayer->GetTeam() > TEAM_2)
        return;
    if (pPlayer->GetLastVoteCallTime() != INVALID_TIME && GetTotalTime() - pPlayer->GetLastVoteCallTime() < g_voteCooldownTime)
        return;

    // Check general voting pre-conditions
    CGameInfo *pGameInfo(GetGameInfo());
    if (pGameInfo == NULL)
        return;
    if (pGameInfo->GetActiveVoteType() != VOTE_TYPE_INVALID)
        return;

    if (GetGamePhase() < GAME_PHASE_PRE_MATCH || GetGamePhase() > GAME_PHASE_ACTIVE)
        return;

    // Validate specific vote type
    switch (yVoteType)
    {
    case VOTE_TYPE_CONCEDE:
        if (GetWinningTeam() != TEAM_INVALID || HasFlags(GAME_FLAG_CONCEDED) || GetMatchTime() < g_voteAllowConcedeTime)
            break;

        pGameInfo->StartVote(VOTE_TYPE_CONCEDE, pPlayer->GetTeam(), g_voteDuration, GetTotalTime());
        break;

    case VOTE_TYPE_REMAKE:
        if (GetMatchTime() > g_voteRemakeTimeLimit)
            break;
            
        pGameInfo->StartVote(VOTE_TYPE_REMAKE, iTarget, g_voteDuration, GetTotalTime());
        break;

    case VOTE_TYPE_KICK_AFK:
        {
            CPlayer *pTarget(GetPlayer(iTarget));
            if (!IsKickVoteValid(pTarget) || pPlayer->GetTeam() != pTarget->GetTeam())
                break;
                
            pGameInfo->StartVote(VOTE_TYPE_KICK_AFK, iTarget, g_voteDuration, GetTotalTime());
        }
        break;

    case VOTE_TYPE_PAUSE:
        {
            CTeamInfo *pTeam(GetTeam(pPlayer->GetTeam()));
            if (pTeam == NULL || pTeam->GetRemainingPauses() == 0 || m_pHostServer->GetPaused())
                break;
            
            pGameInfo->StartVote(VOTE_TYPE_PAUSE, pPlayer->GetTeam(), g_voteDuration, GetTotalTime());
        }
        break;

    case VOTE_TYPE_KICK:    // No longer allowed
    default:
        return;
    }

    // If a vote was successfully initiated, notify players and log the event
    if (pGameInfo->GetActiveVoteType() != VOTE_TYPE_INVALID)
    {
        CBufferFixed<10> buffer;
        buffer << GAME_CMD_VOTE_CALLED_MESSAGE << pPlayer->GetClientNumber() << pGameInfo->GetActiveVoteType() << pGameInfo->GetVoteTarget();
        BroadcastGameData(buffer, true);

        m_GameLog.WritePlayer(GAME_LOG_PLAYER_CALL_VOTE, pPlayer, GetVoteTypeName(yVoteType));

        ResetPlayerVotes();
        pPlayer->SetVote(VOTE_YES);
        pPlayer->SetLastVoteCallTime(GetGameTime());
    }
}


/*====================
  CGameServer::StartUnpause
  ====================*/
void    CGameServer::StartUnpause(CPlayer *pPlayer)
{
    const map<uint, CTeamInfo*> mapTeams(GetTeams());
    for (map<uint, CTeamInfo*>::const_iterator it(mapTeams.begin()); it != mapTeams.end(); it++)
    {
        if (!it->second->IsActiveTeam())
            continue;

        it->second->Unpause();
    }

    m_uiPauseToggleTime = GetTotalTime() + g_pauseDelayTime;

    CBufferDynamic buffer;
    buffer << GAME_CMD_UNPAUSE << TStringToUTF8(pPlayer != NULL ? pPlayer->GetName() : _T("Server")) << byte(0);
    BroadcastGameData(buffer, true);
}


/*====================
  CGameServer::GetNumReferees
  ====================*/
uint    CGameServer::GetNumReferees()
{
    uint iCount(0);

    for (PlayerMap_it itPlayer(m_mapClients.begin()), itEnd(m_mapClients.end()); itPlayer != itEnd; ++itPlayer)
    {
        if (itPlayer->second->IsReferee())
            ++iCount;
    }

    return iCount;
}


/*====================
  CGameServer::SendGeneralMessage
  ====================*/
void    CGameServer::SendGeneralMessage(const tstring &sMsg, int iNumPlayers, int *iPlayerList)
{
    CBufferDynamic buffer;
    buffer << GAME_CMD_GENERAL_MESSAGE << TStringToUTF8(sMsg) << byte(0);

    // write the reserved fields.
    buffer << int(1) << byte(0);

    // write the player list.
    buffer.WriteInt(iNumPlayers);
    for (int i = 0; i < iNumPlayers; ++i)
        buffer.WriteInt(iPlayerList[i]);

    BroadcastGameData(buffer, true);
}


/*====================
  CGameServer::ProcessGameData
  ====================*/
bool    CGameServer::ProcessGameData(int iClientNum, CPacket &pkt)
{
    // OMG spam...
    // Console << _T("Message from client #") << iClientNum << _T("...") << newl;

    CPlayer *pPlayer(GetPlayer(iClientNum));
    if (pPlayer != NULL)
        pPlayer->SetLastInputTime(Game.GetGameTime());

    byte yCmd(pkt.ReadByte());
    switch (yCmd)
    {
    case GAME_CMD_PURCHASE:
        {
            uint uiUnitIndex(pkt.ReadInt(INVALID_INDEX));
            ushort unShop(pkt.ReadShort(INVALID_ENT_TYPE));
            int iSlot(pkt.ReadByte());
            PurchaseItem(iClientNum, uiUnitIndex, unShop, iSlot);
            
            return !pkt.HasFaults();
        }

    case GAME_CMD_PURCHASE2:
        {
            uint uiUnitIndex(pkt.ReadInt(INVALID_INDEX));
            ushort unItemID(pkt.ReadShort(INVALID_ENT_TYPE));
            if (pkt.HasFaults())
                return false;

            IUnitEntity *pUnit(GetUnitEntity(uiUnitIndex));
            if (pUnit == NULL)
                return false;

            ushort unShop(INVALID_ENT_TYPE);
            int iSlot(-1);
            if (!pUnit->GetAccessableShop(EntityRegistry.LookupName(unItemID), unShop, iSlot))
                return false;

            PurchaseItem(iClientNum, uiUnitIndex, unShop, iSlot);
            return true;
        }

    case GAME_CMD_SELL:
        {
            uint uiUnitIndex(pkt.ReadInt(INVALID_INDEX));
            int iSlot(pkt.ReadInt(INVALID_INDEX));
            if (pkt.HasFaults())
                return false;

            SellItem(iClientNum, uiUnitIndex, iSlot);
            return true;
        }

    case GAME_CMD_DISASSEMBLE:
        {
            uint uiUnitIndex(pkt.ReadInt(INVALID_INDEX));
            int iSlot(pkt.ReadInt(INVALID_INDEX));
            if (pkt.HasFaults())
                return false;

            IUnitEntity *pUnit(GetUnitEntity(uiUnitIndex));
            if (pUnit == NULL || !pUnit->CanReceiveOrdersFrom(iClientNum) || m_pHostServer->GetPaused())
                return true;

            pUnit->DisassembleItem(iSlot);
            return true;
        }

    case GAME_CMD_LEVEL_UP:
        {
            uint uiUnitIndex(pkt.ReadInt(INVALID_INDEX));
            int iSlot(pkt.ReadByte(MAX_INVENTORY));
            if (pkt.HasFaults())
                return false;

            if (m_pHostServer->GetPaused())
                return true;

            IHeroEntity *pHero(GetHeroEntity(uiUnitIndex));
            if (pHero == NULL)
                break;
            if (pHero->GetOwnerClientNumber() != iClientNum)
                break;

            IEntityAbility *pAbility(pHero->GetAbility(iSlot));
            if (pAbility == NULL)
                break;

            pAbility->LevelUp();
            return true;
        }

    case GAME_CMD_CHANGE_TEAM:
        {
            int iTeam(pkt.ReadInt(-1));
            uint uiSlot(pkt.ReadInt(-1));
            if (iTeam != TEAM_SPECTATOR && iTeam != TEAM_PASSIVE && iTeam != TEAM_1 && iTeam != TEAM_2)
                return !pkt.HasFaults();

            RequestChangeTeam(iClientNum, iTeam, uiSlot);
        }
        return !pkt.HasFaults();

    case GAME_CMD_ASSIGN_FIRST_BAN_TEAM:
        {
            uint uiTeam(pkt.ReadInt(-1));
            RequestAssignFirstBanTeam(iClientNum, uiTeam);
        }
        return !pkt.HasFaults();

    case GAME_CMD_SELECT_HERO:
        SelectHero(iClientNum, pkt.ReadShort(-1), false);
        return !pkt.HasFaults();

    case GAME_CMD_SELECT_POTENTIAL_HERO:
        SelectHero(iClientNum, pkt.ReadShort(-1), true);
        return !pkt.HasFaults();

    case GAME_CMD_RANDOM_HERO:
        SelectRandomHero(iClientNum);
        return true;

    case GAME_CMD_READY:
        if (pPlayer != NULL && GetGamePhase() == GAME_PHASE_HERO_SELECT && pPlayer->HasSelectedHero())
        {
            if (!pPlayer->HasFlags(PLAYER_FLAG_READY))
            {
                CBufferFixed<5> buffer;
                buffer << GAME_CMD_READY_MESSAGE << iClientNum;
                BroadcastGameData(buffer, true);
            }

            pPlayer->SetFlags(PLAYER_FLAG_READY);
        }
        return true;

    case GAME_CMD_UNREADY:
        /*
        if (pPlayer != NULL && GetGamePhase() == GAME_PHASE_HERO_SELECT && !AllPlayersReady())
        {
            if (pPlayer->HasFlags(PLAYER_FLAG_READY))
            {
                CBufferFixed<5> buffer;
                buffer << GAME_CMD_UNREADY_MESSAGE << iClientNum;
                BroadcastGameData(buffer, true);
            }

            pPlayer->RemoveFlags(PLAYER_FLAG_READY);
        }
        */
        return true;

    case GAME_CMD_FINISHED_LOADING_HEROES:
        if (pPlayer != NULL)
            pPlayer->SetFlags(PLAYER_FLAG_LOADED_HEROES);

        return true;

    case GAME_CMD_LOADING_PROGRESS:
        {
            float fLoadingProgress(pkt.ReadFloat());

            if (pPlayer != NULL)
                pPlayer->SetLoadingProgress(fLoadingProgress);
        }
        return !pkt.HasFaults();

    case GAME_CMD_REPICK_HERO:
        if (pPlayer != NULL && pPlayer->CanRepick())
        {
            ushort unOldHero(pPlayer->GetSelectedHero());
            pPlayer->SetFlags(PLAYER_FLAG_HAS_REPICKED);
            pPlayer->TakeGold(GetGameInfo()->GetRepickCost());
            m_GameLog.WritePlayer(GAME_LOG_PLAYER_REPICK, pPlayer, EntityRegistry.LookupName(pPlayer->GetSelectedHero()));
            pPlayer->SelectHero(INVALID_ENT_TYPE);
            if (HasGameOptions(GAME_OPTION_FORCE_RANDOM))
                SelectRandomHero(iClientNum);

            SetHeroStatus(unOldHero, HERO_LIST_AVAILABLE_ALL);

            CBufferFixed<7> buffer;
            buffer << GAME_CMD_REPICK_HERO_MESSAGE << iClientNum << unOldHero;
            BroadcastGameData(buffer, true);
        }
        return true;

    case GAME_CMD_SWAP_HERO_REQUEST:
        {
            byte yTargetPlayer(pkt.ReadByte(byte(-1)));

            if (pPlayer == NULL)
                return true;

            CTeamInfo *pTeam(Game.GetTeam(pPlayer->GetTeam()));

            if (pPlayer->CanSwap() && pTeam != NULL)
            {
                CPlayer *pTargetPlayer(GetPlayer(pTeam->GetClientIDFromTeamIndex(yTargetPlayer)));
                if (pTargetPlayer != NULL && pTargetPlayer->CanSwap())
                {
                    if (pTargetPlayer->HasSwapRequest(pPlayer->GetTeamIndex()))
                    {
                        m_GameLog.WritePlayer(GAME_LOG_PLAYER_SWAP, pPlayer, EntityRegistry.LookupName(pPlayer->GetSelectedHero()));
                        m_GameLog.WritePlayer(GAME_LOG_PLAYER_SWAP, pTargetPlayer, EntityRegistry.LookupName(pTargetPlayer->GetSelectedHero()));

                        ushort unSwapHero(pPlayer->GetSelectedHero());
                        pPlayer->SelectHero(pTargetPlayer->GetSelectedHero());
                        pTargetPlayer->SelectHero(unSwapHero);

                        pPlayer->ClearAllSwapRequests();
                        pTargetPlayer->ClearAllSwapRequests();
        
                        CBufferFixed<9> buffer;
                        buffer << GAME_CMD_SWAP_HERO_MESSAGE << iClientNum << pTargetPlayer->GetClientNumber();
                        BroadcastGameData(buffer, true);
                    }
                    else
                    {
                        pPlayer->SetSwapRequest(yTargetPlayer);

                        CBufferFixed<5> buffer;
                        buffer << GAME_CMD_SWAP_REQUEST_MESSAGE << iClientNum;
                        SendGameData(pTargetPlayer->GetClientNumber(), buffer, true);
                    }
                }
            }
        }
        return !pkt.HasFaults();

    case GAME_CMD_REQUEST_MATCH_START:
        {
            CClientConnection *pClientConnection(m_pHostServer->GetClient(iClientNum));
            if (pClientConnection != NULL && pClientConnection->HasFlags(CLIENT_CONNECTION_GAME_HOST))
                RequestStartGame(iClientNum);
        }
        return true;
        
    case GAME_CMD_REQUEST_MATCH_CANCEL:
        {
            CClientConnection *pClientConnection(m_pHostServer->GetClient(iClientNum));
            if (m_pHostServer->GetPractice() || (pClientConnection != NULL && pClientConnection->HasFlags(CLIENT_CONNECTION_GAME_HOST)))
            {
                if (GetGamePhase() < GAME_PHASE_HERO_BAN)
                {
                    SetGamePhaseEndTime(INVALID_TIME);
                    
                    CBufferFixed<1> buffer;
                    buffer << GAME_CMD_MATCH_CANCEL_MESSAGE;
                    BroadcastGameData(buffer, true);
                }                   
            }
        }
        return true;

    case GAME_CMD_CALLVOTE:
        ProcessCallVoteRequest(pPlayer, pkt);
        return !pkt.HasFaults();

    case GAME_CMD_UNPAUSE:
        {
            if (pPlayer != NULL && (pPlayer->GetTeam() == TEAM_1 || pPlayer->GetTeam() == TEAM_2) && m_pHostServer->GetPaused() && m_uiPauseToggleTime == INVALID_TIME)
            {
                CTeamInfo *pTeam(GetTeam(pPlayer->GetTeam()));

                if (pTeam != NULL && pTeam->HasFlags(TEAM_FLAG_CAN_UNPAUSE))
                    StartUnpause(pPlayer);
            }
        }
        return !pkt.HasFaults();

    case GAME_CMD_VOTE:
        {
            byte yVote(pkt.ReadByte(VOTE_NONE));
            CGameInfo *pGameInfo(GetGameInfo());
            if (pGameInfo != NULL && pGameInfo->GetActiveVoteType() != VOTE_TYPE_INVALID && pPlayer != NULL && yVote != VOTE_NONE)
            {
                pPlayer->SetVote(yVote);
                m_GameLog.WritePlayer(GAME_LOG_PLAYER_VOTE, pPlayer, GetVoteTypeName(pGameInfo->GetActiveVoteType()), GetVoteName(yVote));
            }
        }
        return !pkt.HasFaults();

    case GAME_CMD_CHAT_ALL:
    case GAME_CMD_CHAT_ROLL:
    case GAME_CMD_CHAT_EMOTE:
        try
        {
            wstring sMsg(pkt.ReadWString().substr(0, 150));

            if (pPlayer == NULL)
                EX_ERROR(_T("Invalid client ID"));

            if (pPlayer->GetChatCounter() >= sv_chatCounterFloodThreshold || !pPlayer->GetAllowChat())
                break;

            // Disallow chat before game has started in arranged matches
            if (m_pHostServer->IsArrangedMatch() && GetGamePhase() <= GAME_PHASE_WAITING_FOR_PLAYERS)
                break;

            pPlayer->IncrementChatCounter();
            pPlayer->SetAllowChat(false);

            CBufferDynamic buffer;
            
            switch (yCmd)
            {
            case GAME_CMD_CHAT_ALL:
                Console.Server << _T("[ALL] ") << pPlayer->GetName() << _T(": ") << sMsg << newl;
                buffer << GAME_CMD_CHAT_ALL << iClientNum << TStringToUTF8(sMsg) << byte(0);
                BroadcastGameData(buffer, true);
                m_GameLog.WritePlayer(GAME_LOG_PLAYER_CHAT, pPlayer, _T("all"), sMsg);
                break;

            case GAME_CMD_CHAT_ROLL:
                Console.Server << _T("[ROLL] ") << sMsg << newl;
                buffer << GAME_CMD_CHAT_ROLL << iClientNum << TStringToUTF8(sMsg) << byte(0);
                BroadcastGameData(buffer, true);
                m_GameLog.WritePlayer(GAME_LOG_PLAYER_CHAT, pPlayer, _T("roll"), sMsg);
                break;

            case GAME_CMD_CHAT_EMOTE:
                Console.Server << _T("[EMOTE] ") << sMsg << newl;
                buffer << GAME_CMD_CHAT_EMOTE << iClientNum << TStringToUTF8(sMsg) << byte(0);
                BroadcastGameData(buffer, true);
                m_GameLog.WritePlayer(GAME_LOG_PLAYER_CHAT, pPlayer, _T("emote"), sMsg);
                break;
            }
            
        }
        catch (CException &ex)
        {
            if (yCmd == GAME_CMD_CHAT_ALL)
            {
                ex.Process(_T("CGameServer::ProcessGameData(GAME_CMD_CHAT_ALL) - "), NO_THROW);
            }
            else if (yCmd == GAME_CMD_CHAT_ROLL)
            {
                ex.Process(_T("CGameServer::ProcessGameData(GAME_CMD_CHAT_ROLL) - "), NO_THROW);
            }
            else if (yCmd == GAME_CMD_CHAT_EMOTE)
            {
                ex.Process(_T("CGameServer::ProcessGameData(GAME_CMD_CHAT_EMOTE) - "), NO_THROW);
            }           
        }
        break;

    case GAME_CMD_CHAT_TEAM:
        try
        {
            wstring sMsg(pkt.ReadWString().substr(0, 150));

            if (pPlayer == NULL)
                EX_ERROR(_T("Invalid client ID"));

            if (pPlayer->GetChatCounter() >= sv_chatCounterFloodThreshold || !pPlayer->GetAllowChat())
                break;

            // Disallow chat before game has started in arranged matches
            if (m_pHostServer->IsArrangedMatch() && GetGamePhase() <= GAME_PHASE_WAITING_FOR_PLAYERS)
                break;

            pPlayer->IncrementChatCounter();
            pPlayer->SetAllowChat(false);

            CTeamInfo *pTeam(GameServer.GetTeam(pPlayer->GetTeam()));
            if (pTeam != NULL)
            {
                CBufferDynamic buffer;
                buffer << GAME_CMD_CHAT_TEAM << iClientNum << TStringToUTF8(sMsg) << byte(0);
                Console.Server << _T("[TEAM ") << pPlayer->GetTeam() << _T("] ") << pPlayer->GetName() << _T(": ") << sMsg << newl;
                
                m_GameLog.WritePlayer(GAME_LOG_PLAYER_CHAT, pPlayer, _T("team"), sMsg);

                ivector viClients(pTeam->GetClientList());

                for (ivector::iterator it(viClients.begin()); it != viClients.end(); it++)
                {
                    if (*it == -1)
                        continue;

                    SendGameData(*it, buffer, true);
                }
            }
        }
        catch (CException &ex)
        {
            ex.Process(_T("CGameServer::ProcessGameData(GAME_CMD_CHAT_TEAM) - "), NO_THROW);
        }
        break;

    case GAME_CMD_PING_ALL:
        try
        {
            if (pPlayer == NULL)
                EX_WARN(_T("No entity found for client"));

            CBufferDynamic buffer;
            buffer << GAME_CMD_PING_ALL_MESSAGE;

            // count the number of actual players.
            byte yPlayerCount(0);
            for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
            {
                CPlayer* pPlayer(it->second);
                if (pPlayer == NULL)
                    EX_WARN(_T("No entity found for player"));

                if (pPlayer->GetTeam() == TEAM_1 || pPlayer->GetTeam() == TEAM_2)
                    ++yPlayerCount;
            }

            // add the number of actual players to the packet.
            buffer << (byte)yPlayerCount;

            // add each actual player's ping time to the packet.
            for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
            {
                CPlayer* pPlayer(it->second);
                if (pPlayer == NULL)
                    EX_WARN(_T("No entity found for player"));

                if (pPlayer->GetTeam() == TEAM_1 || pPlayer->GetTeam() == TEAM_2)
                {
                    buffer << int(pPlayer->GetClientNumber()) << ushort(pPlayer->GetPing());
                }
            }
            SendGameData(iClientNum, buffer, true);
        }
        catch (CException &ex)
        {
            ex.Process(_T("CGameServer::ProcessGameData(GAME_CMD_PING_ALL) - "), NO_THROW);
        }
        break;

    case GAME_CMD_ORDER_POSITION:
        try
        {
            ECommanderOrder eOrder(ECommanderOrder(pkt.ReadByte()));
            CVec2f v2Pos(pkt.ReadV2f());
            uint uiParam(pkt.ReadInt());
            byte yQueue(pkt.ReadByte());
            uint uiSingleEntityIndex(pkt.ReadInt());
            byte yFlags(pkt.ReadByte());

            if (pPlayer == NULL)
                EX_WARN(_T("No entity found for client"));

            if (m_pHostServer->GetPaused())
                break;

            pPlayer->IncrementActionCount();

            const uiset &setSelection(pPlayer->GetSelection());
            for (uiset::const_iterator it(setSelection.begin()); it != setSelection.end(); ++it)
            {
                IUnitEntity *pUnit(GetUnitEntity(*it));
                if (pUnit == NULL || !pUnit->CanActOnOrdersFrom(iClientNum))
                    continue;

                if (uiSingleEntityIndex != INVALID_INDEX && *it != uiSingleEntityIndex)
                    continue;
            
                SUnitCommand cmd;
                cmd.v2Dest = v2Pos;
                cmd.yQueue = yQueue;
                cmd.iClientNumber = iClientNum;
                cmd.bShared = setSelection.size() > 1;
                cmd.bDirectPathing = (yFlags & CMDR_ORDER_FLAG_DIRECT_PATHING) != 0;

                switch (eOrder)
                {
                case CMDR_ORDER_ATTACK:
                    cmd.eCommandID = UNITCMD_ATTACKMOVE;
                    break;
                
                default:
                case CMDR_ORDER_MOVE:
                    cmd.eCommandID = UNITCMD_MOVE;
                    break;

                case CMDR_ORDER_GIVEITEM:
                    {
                        IEntityItem *pItem(GetEntityItem(uiParam));
                        if (pItem != NULL && 
                            pItem->GetOwner() == pUnit && 
                            pItem->CanDrop() && 
                            !pItem->GetNoDrop() &&
                            (pItem->BelongsToClient(iClientNum) || pUnit->GetOwnerClientNumber() == iClientNum || pItem->GetAllowTransfer()) && 
                            !pItem->HasFlag(ENTITY_TOOL_FLAG_LOCKED))
                        {
                            cmd.eCommandID = UNITCMD_DROPITEM;
                            cmd.bShared = false;
                            cmd.uiParam = pItem->GetUniqueID();
                        }
                        else
                        {
                            cmd.eCommandID = UNITCMD_INVALID;
                        }
                    }
                    break;
                }

                if (cmd.eCommandID != UNITCMD_INVALID)
                {
                    pUnit->PlayerCommand(cmd);

                    if (it == setSelection.begin())
                    {
                        CBufferFixed<3> buffer;
                        buffer << GAME_CMD_CONFIRM_MOVE << ushort(*it);
                        SendGameData(iClientNum, buffer, true);
                    }
                }
            }
        }
        catch (CException &ex)
        {
            ex.Process(_T("CGameServer::ProcessGameData(GAME_CMD_ORDER_POSITION) - "), NO_THROW);
        }
        break;

    case GAME_CMD_ORDER_ENTITY:
        try
        {
            ECommanderOrder eOrder(ECommanderOrder(pkt.ReadByte()));
            uint uiEntIndex(pkt.ReadInt());
            uint uiParam(pkt.ReadInt());
            byte yQueue(pkt.ReadByte());
            uint uiSingleEntityIndex(pkt.ReadInt());
            byte yFlags(pkt.ReadByte());

            if (pPlayer == NULL)
                EX_WARN(_T("No entity found for client"));

            if (m_pHostServer->GetPaused())
                break;

            pPlayer->IncrementActionCount();

            bool bConfirmation(false);

            const uiset &setSelection(pPlayer->GetSelection());

            SUnitCommand cmd;
            cmd.uiIndex = uiEntIndex;
            cmd.yQueue = yQueue;
            cmd.iClientNumber = iClientNum;
            cmd.bShared = setSelection.size() > 1;
            cmd.bDirectPathing = (yFlags & CMDR_ORDER_FLAG_DIRECT_PATHING) != 0;

            for (uiset::const_iterator it(setSelection.begin()); it != setSelection.end(); ++it)
            {
                IUnitEntity *pUnit(GetUnitEntity(*it));
                if (pUnit == NULL || !pUnit->CanActOnOrdersFrom(iClientNum))
                    continue;

                if (uiSingleEntityIndex != INVALID_INDEX && *it != uiSingleEntityIndex)
                    continue;

                IUnitEntity *pTarget(GetUnitEntity(uiEntIndex));
                if (pTarget == NULL)
                    break;

                bConfirmation = true;

                if (eOrder == CMDR_ORDER_ATTACK)
                {
                    cmd.eCommandID = UNITCMD_ATTACK;
                    pUnit->PlayerCommand(cmd);

                    if (it == setSelection.begin())
                    {
                        CBufferFixed<3> buffer;
                        buffer << GAME_CMD_CONFIRM_ATTACK << ushort(*it);
                        SendGameData(iClientNum, buffer, true);
                    }
                }
                else if (eOrder == CMDR_ORDER_MOVE)
                {
                    cmd.eCommandID = UNITCMD_FOLLOW;
                    pUnit->PlayerCommand(cmd);

                    if (it == setSelection.begin())
                    {
                        CBufferFixed<3> buffer;
                        buffer << GAME_CMD_CONFIRM_MOVE << ushort(*it);
                        SendGameData(iClientNum, buffer, true);
                    }
                }
                else if (eOrder == CMDR_ORDER_FOLLOW)
                {
                    cmd.eCommandID = UNITCMD_FOLLOWGUARD;
                    pUnit->PlayerCommand(cmd);

                    if (it == setSelection.begin())
                    {
                        CBufferFixed<3> buffer;
                        buffer << GAME_CMD_CONFIRM_MOVE << ushort(*it);
                        SendGameData(iClientNum, buffer, true);
                    }
                }
                else if (eOrder == CMDR_ORDER_TOUCH)
                {
                    cmd.eCommandID = UNITCMD_TOUCH;
                    pUnit->PlayerCommand(cmd);

                    if (it == setSelection.begin())
                    {
                        CBufferFixed<3> buffer;
                        buffer << GAME_CMD_CONFIRM_MOVE << ushort(*it);
                        SendGameData(iClientNum, buffer, true);
                    }
                }
                else if (eOrder == CMDR_ORDER_GIVEITEM)
                {
                    IEntityItem *pItem(GetEntityItem(uiParam));
                    if (pItem != NULL &&
                        pItem->GetOwner() == pUnit &&
                        pItem->CanDrop() &&
                        !pItem->GetNoDrop() &&
                        pUnit->GetTeam() == pTarget->GetTeam() &&
                        pTarget != pUnit &&
                        pTarget->GetCanCarryItems() &&
                        (pItem->BelongsToClient(iClientNum) || pUnit->GetOwnerClientNumber() == iClientNum || pItem->GetAllowTransfer()) &&
                        !pItem->HasFlag(ENTITY_TOOL_FLAG_LOCKED))
                    {
                        cmd.eCommandID = UNITCMD_GIVEITEM;
                        cmd.uiParam = pItem->GetUniqueID();
                        cmd.bShared = false;
                        pUnit->PlayerCommand(cmd);

                        if (it == setSelection.begin())
                        {
                            CBufferFixed<3> buffer;
                            buffer << GAME_CMD_CONFIRM_MOVE << ushort(*it);
                            SendGameData(iClientNum, buffer, true);
                        }
                    }
                }
                else
                {
                    if (pUnit->IsEnemy(pTarget))
                    {
                        cmd.eCommandID = UNITCMD_ATTACK;
                        pUnit->PlayerCommand(cmd);

                        if (it == setSelection.begin())
                        {
                            CBufferFixed<3> buffer;
                            buffer << GAME_CMD_CONFIRM_ATTACK << ushort(*it);
                            SendGameData(iClientNum, buffer, true);
                        }
                    }
                    else
                    {
                        cmd.eCommandID = UNITCMD_FOLLOWGUARD;
                        pUnit->PlayerCommand(cmd);

                        if (it == setSelection.begin())
                        {
                            CBufferFixed<3> buffer;
                            buffer << GAME_CMD_CONFIRM_MOVE << ushort(*it);
                            SendGameData(iClientNum, buffer, true);
                        }
                    }
                }
            }

            if (bConfirmation)
            {
                CBufferFixed<3> cBuffer;
                cBuffer << GAME_CMD_ORDER_CONFIRMATION << ushort(uiEntIndex);

                SendGameData(iClientNum, cBuffer, false);
            }
        }
        catch (CException &ex)
        {
            ex.Process(_T("CGameServer::ProcessGameData(GAME_CMD_ORDER_ENTITY) - "), NO_THROW);
        }
        break;

    case GAME_CMD_ORDER_STOP:
        {
            uint uiSingleEntityIndex(pkt.ReadInt());

            if (pPlayer == NULL)
            {
                Console.Warn << _T("GAME_CMD_ORDER_STOP: No entity found for client #") << iClientNum << newl;
                break;
            }

            if (m_pHostServer->GetPaused())
                break;

            pPlayer->IncrementActionCount();

            const uiset &setSelection(pPlayer->GetSelection());
            for (uiset::const_iterator it(setSelection.begin()); it != setSelection.end(); ++it)
            {
                IUnitEntity *pUnit(GetUnitEntity(*it));
                if (pUnit == NULL || !pUnit->CanActOnOrdersFrom(iClientNum))
                    continue;

                if (uiSingleEntityIndex != INVALID_INDEX && *it != uiSingleEntityIndex)
                    continue;
                
                SUnitCommand cmd;
                cmd.eCommandID = UNITCMD_STOP;
                cmd.iClientNumber = iClientNum;
                pUnit->PlayerCommand(cmd);
            }
            break;
        }

    case GAME_CMD_ORDER_HOLD:
    case GAME_CMD_ORDER_CANCEL_AND_HOLD:
        {
            byte yQueue(pkt.ReadByte());
            uint uiSingleEntityIndex(pkt.ReadInt());

            if (pPlayer == NULL)
            {
                Console.Warn << _T("GAME_CMD_ORDER_HOLD: No entity found for client #") << iClientNum << newl;
                break;
            }

            if (m_pHostServer->GetPaused())
                break;

            pPlayer->IncrementActionCount();

            const uiset &setSelection(pPlayer->GetSelection());
            for (uiset::const_iterator it(setSelection.begin()); it != setSelection.end(); ++it)
            {
                IUnitEntity *pUnit(GetUnitEntity(*it));
                if (pUnit == NULL || !pUnit->CanActOnOrdersFrom(iClientNum))
                    continue;

                if (uiSingleEntityIndex != INVALID_INDEX && *it != uiSingleEntityIndex)
                    continue;

                SUnitCommand cmd;
                cmd.eCommandID = UNITCMD_HOLD;
                cmd.yQueue = yQueue;
                // "cancel and hold position" will cancel animations, unless being queued.
                cmd.uiParam = 0;
                if (yCmd == GAME_CMD_ORDER_CANCEL_AND_HOLD && yQueue != QUEUE_BACK)
                    cmd.uiParam |= HOLD_FLAGS_CANCEL_ANIMATIONS;
                cmd.iClientNumber = iClientNum;
                pUnit->PlayerCommand(cmd);
            }
            break;
        }

    case GAME_CMD_ABILITY:
        {
            uint uiUnitIndex(pkt.ReadInt());
            byte ySlot(pkt.ReadByte());
            byte yQueue(pkt.ReadByte());

            if (m_pHostServer->GetPaused())
                break;

            IUnitEntity *pUnit(GameServer.GetUnitEntity(uiUnitIndex));
            if (pUnit == NULL)
                break;

            if (!IS_SHARED_ABILITY(ySlot) && !pUnit->CanActOnOrdersFrom(iClientNum))
                break;

            CPlayer *pPlayer(Game.GetPlayerFromClientNumber(iClientNum));

            if (ySlot >= INVENTORY_START_SHARED_ABILITIES && ySlot <= INVENTORY_END_SHARED_ABILITIES && (pPlayer == NULL || pUnit->GetTeam() != pPlayer->GetTeam()))
                break;

            if (ySlot >= INVENTORY_START_SHARED_ABILITIES && ySlot <= INVENTORY_END_SHARED_ABILITIES)
            {
                CTeamInfo *pTeam(Game.GetTeam(pUnit->GetTeam()));
                if (pTeam != NULL)
                {
                    IUnitEntity *pBase(Game.GetUnitEntity(pTeam->GetBaseBuildingIndex()));
                    if (pBase != NULL)
                        pUnit = pBase;
                }
            }

            IEntityTool *pItem(pUnit->GetTool(ySlot));
            if (pItem == NULL)
                break;
            if (pItem->GetFrontQueue() && yQueue == QUEUE_NONE)
                yQueue = QUEUE_FRONT;

            if (pPlayer != NULL)
                pPlayer->IncrementActionCount();

            SUnitCommand cmd;
            cmd.eCommandID = UNITCMD_ABILITY;
            cmd.uiParam = ySlot;
            cmd.yQueue = yQueue;
            cmd.iClientNumber = iClientNum;
            pUnit->PlayerCommand(cmd);
        }
        break;

    case GAME_CMD_ABILITY2:
        {
            uint uiUnitIndex(pkt.ReadInt());
            byte ySlot(pkt.ReadByte());
            byte yQueue(pkt.ReadByte());

            if (m_pHostServer->GetPaused())
                break;

            IUnitEntity *pUnit(GameServer.GetUnitEntity(uiUnitIndex));
            if (pUnit == NULL)
                break;

            if (!IS_SHARED_ABILITY(ySlot) && !pUnit->CanActOnOrdersFrom(iClientNum))
                break;

            CPlayer *pPlayer(Game.GetPlayerFromClientNumber(iClientNum));

            if (ySlot >= INVENTORY_START_SHARED_ABILITIES && ySlot <= INVENTORY_END_SHARED_ABILITIES && (pPlayer == NULL || pUnit->GetTeam() != pPlayer->GetTeam()))
                break;

            IEntityTool *pItem(pUnit->GetTool(ySlot));
            if (pItem == NULL)
                break;
            if (pItem->GetFrontQueue() && yQueue == QUEUE_NONE)
                yQueue = QUEUE_FRONT;
            if ((pItem->GetAllowAutoCast() || pItem->GetActionType() == TOOL_ACTION_ATTACK) && yQueue == QUEUE_NONE)
                yQueue = QUEUE_FRONT;

            if (pPlayer != NULL)
                pPlayer->IncrementActionCount();

            SUnitCommand cmd;
            cmd.eCommandID = UNITCMD_ABILITY2;
            cmd.uiParam = ySlot;
            cmd.yQueue = yQueue;
            cmd.iClientNumber = iClientNum;
            pUnit->PlayerCommand(cmd);
        }
        break;

    case GAME_CMD_ABILITY_POSITION:
        {
            uint uiUnitIndex(pkt.ReadInt());
            byte ySlot(pkt.ReadByte());
            CVec2f v2Pos(pkt.ReadV2f());
            byte yQueue(pkt.ReadByte());
            byte yFlags(pkt.ReadByte());

            if (m_pHostServer->GetPaused())
                break;

            IUnitEntity *pUnit(GameServer.GetUnitEntity(uiUnitIndex));
            if (pUnit == NULL)
                break;

            if (!IS_SHARED_ABILITY(ySlot) && !pUnit->CanActOnOrdersFrom(iClientNum))
                break;

            CPlayer *pPlayer(Game.GetPlayerFromClientNumber(iClientNum));

            if (ySlot >= INVENTORY_START_SHARED_ABILITIES && ySlot <= INVENTORY_END_SHARED_ABILITIES && (pPlayer == NULL || pUnit->GetTeam() != pPlayer->GetTeam()))
                break;

            IEntityTool *pItem(pUnit->GetTool(ySlot));
            if (pItem == NULL)
                break;

            if (pItem->GetNeedVision() && !IsVisible(pUnit, v2Pos.x, v2Pos.y))
                break;

            if (pItem->GetFrontQueue() && yQueue == QUEUE_NONE)
                yQueue = QUEUE_FRONT;

            if (pPlayer != NULL)
                pPlayer->IncrementActionCount();

            SUnitCommand cmd;
            cmd.eCommandID = UNITCMD_ABILITY;
            cmd.v2Dest = v2Pos;
            cmd.uiParam = ySlot;
            cmd.yQueue = yQueue;
            cmd.iClientNumber = iClientNum;
            cmd.bDirectPathing = (yFlags & CMDR_ORDER_FLAG_DIRECT_PATHING) != 0;
            pUnit->PlayerCommand(cmd);

            CBufferFixed<3> buffer;
            buffer << GAME_CMD_CONFIRM_MOVE << ushort(uiUnitIndex);
            SendGameData(iClientNum, buffer, true);
        }
        break;

    case GAME_CMD_ABILITY_VECTOR:
        {
            uint uiUnitIndex(pkt.ReadInt());
            byte ySlot(pkt.ReadByte());
            CVec2f v2Pos(pkt.ReadV2f());
            CVec2f v2Delta(pkt.ReadV2f());
            byte yQueue(pkt.ReadByte());
            byte yFlags(pkt.ReadByte());

            if (m_pHostServer->GetPaused())
                break;

            IUnitEntity *pUnit(GameServer.GetUnitEntity(uiUnitIndex));
            if (pUnit == NULL)
                break;

            if (!IS_SHARED_ABILITY(ySlot) && !pUnit->CanActOnOrdersFrom(iClientNum))
                break;

            CPlayer *pPlayer(Game.GetPlayerFromClientNumber(iClientNum));

            if (ySlot >= INVENTORY_START_SHARED_ABILITIES && ySlot <= INVENTORY_END_SHARED_ABILITIES && (pPlayer == NULL || pUnit->GetTeam() != pPlayer->GetTeam()))
                break;

            IEntityTool *pItem(pUnit->GetTool(ySlot));
            if (pItem == NULL)
                break;
            if (pItem->GetFrontQueue() && yQueue == QUEUE_NONE)
                yQueue = QUEUE_FRONT;

            SUnitCommand cmd;
            cmd.eCommandID = UNITCMD_ABILITY;
            cmd.v2Dest = v2Pos;
            cmd.uiParam = ySlot;
            cmd.yQueue = yQueue;
            cmd.iClientNumber = iClientNum;
            cmd.v2Delta = v2Delta;
            cmd.bDirectPathing = (yFlags & CMDR_ORDER_FLAG_DIRECT_PATHING) != 0;
            pUnit->PlayerCommand(cmd);

            CBufferFixed<3> buffer;
            buffer << GAME_CMD_CONFIRM_MOVE << ushort(uiUnitIndex);
            SendGameData(iClientNum, buffer, true);
        }
        break;

    case GAME_CMD_ABILITY_ENTITY:
        {
            uint uiUnitIndex(pkt.ReadInt());
            byte ySlot(pkt.ReadByte());
            uint uiTargetIndex(pkt.ReadInt());
            byte yQueue(pkt.ReadByte());
            byte yFlags(pkt.ReadByte());

            if (m_pHostServer->GetPaused())
                break;

            IUnitEntity *pUnit(GameServer.GetUnitEntity(uiUnitIndex));
            if (pUnit == NULL)
                break;

            if (!IS_SHARED_ABILITY(ySlot) && !pUnit->CanActOnOrdersFrom(iClientNum))
                break;

            CPlayer *pPlayer(Game.GetPlayerFromClientNumber(iClientNum));

            if (ySlot >= INVENTORY_START_SHARED_ABILITIES && ySlot <= INVENTORY_END_SHARED_ABILITIES && (pPlayer == NULL || pUnit->GetTeam() != pPlayer->GetTeam()))
                break;

            IEntityTool *pItem(pUnit->GetTool(ySlot));
            if (pItem == NULL)
                break;
            if (pItem->GetFrontQueue() && yQueue == QUEUE_NONE)
                yQueue = QUEUE_FRONT;

            if (pPlayer != NULL)
                pPlayer->IncrementActionCount();

            SUnitCommand cmd;
            cmd.eCommandID = UNITCMD_ABILITY;
            cmd.uiIndex = uiTargetIndex;
            cmd.uiParam = ySlot;
            cmd.yQueue = yQueue;
            cmd.iClientNumber = iClientNum;
            cmd.bDirectPathing = (yFlags & CMDR_ORDER_FLAG_DIRECT_PATHING) != 0;
            pUnit->PlayerCommand(cmd);

            CBufferFixed<3> cBuffer;
            cBuffer << GAME_CMD_ORDER_CONFIRMATION << ushort(uiTargetIndex);
            SendGameData(iClientNum, cBuffer, false);

            IUnitEntity *pTarget(GetUnitEntity(uiTargetIndex));
            if (pTarget != NULL)
            {
                if (pUnit->IsEnemy(pTarget) && !pItem->GetNoVoiceResponse())
                {
                    CBufferFixed<3> buffer;
                    buffer << GAME_CMD_CONFIRM_ATTACK << ushort(uiUnitIndex);
                    SendGameData(iClientNum, buffer, true);
                }
                else
                {
                    CBufferFixed<3> buffer;
                    buffer << GAME_CMD_CONFIRM_MOVE << ushort(uiUnitIndex);
                    SendGameData(iClientNum, buffer, true);
                }
            }
        }
        break;

    case GAME_CMD_DOUBLE_ACTIVATE_ABILITY:
        {
            uint uiUnitIndex(pkt.ReadInt());
            byte ySlot(pkt.ReadByte());
            byte yQueue(pkt.ReadByte());
            byte yFlags(pkt.ReadByte());

            IUnitEntity *pUnit(GameServer.GetUnitEntity(uiUnitIndex));
            if (pUnit == NULL)
                break;

            if (!IS_SHARED_ABILITY(ySlot) && !pUnit->CanActOnOrdersFrom(iClientNum))
                break;

            CPlayer *pPlayer(Game.GetPlayerFromClientNumber(iClientNum));

            if (ySlot >= INVENTORY_START_SHARED_ABILITIES && ySlot <= INVENTORY_END_SHARED_ABILITIES && (pPlayer == NULL || pUnit->GetTeam() != pPlayer->GetTeam()))
                break;

            IEntityTool *pItem(pUnit->GetTool(ySlot));
            if (pItem == NULL)
                break;

            if (pPlayer != NULL)
                pPlayer->IncrementActionCount();

            SUnitCommand cmd;
            cmd.eCommandID = UNITCMD_DOUBLE_ACTIVATE_ABILITY;
            cmd.uiIndex = uiUnitIndex;
            cmd.uiParam = ySlot;
            cmd.yQueue = yQueue;
            cmd.iClientNumber = iClientNum;
            cmd.bDirectPathing = (yFlags & CMDR_ORDER_FLAG_DIRECT_PATHING) != 0;
            pUnit->PlayerCommand(cmd);

            CBufferFixed<3> cBuffer;
            cBuffer << GAME_CMD_ORDER_CONFIRMATION << ushort(uiUnitIndex);
            SendGameData(iClientNum, cBuffer, false);
        }
        break;

#if 0
    case GAME_CMD_MINIMAP_DRAW:
        try
        {
            float fX(pkt.ReadFloat());
            float fY(pkt.ReadFloat());
            
            if (pPlayer == NULL)
                EX_ERROR(_T("Invalid client ID"));

            CTeamInfo *pTeam(GameServer.GetTeam(pPlayer->GetTeam()));
            if (pTeam == NULL)
                EX_ERROR(_T("Invalid Team"));

            CBufferFixed<10> buffer;
            buffer << GAME_CMD_MINIMAP_DRAW << fX << fY << byte(iClientNum);

            const ivector &viClients(pTeam->GetClientList());
            for (ivector::const_iterator it(viClients.begin()); it != viClients.end(); ++it)
            {
                if (*it == -1)
                    continue;

                SendGameData(*it, buffer, false);
            }
        }
        catch (CException &ex)
        {
            ex.Process(_T("CGameServer::ProcessGameData(GAME_CMD_MINIMAP_DRAW) - "), NO_THROW);
        }
        break;
#endif

    case GAME_CMD_MINIMAP_PING:
        try
        {
            byte x(pkt.ReadByte());
            byte y(pkt.ReadByte());

            if (pkt.HasFaults())
            {
                Console.Err << "Bad ping request" << newl;
                return false;
            }

            if (m_pHostServer->GetPaused())
                return true;

            if (pPlayer == NULL)
                EX_ERROR(_T("Invalid client ID"));

            CTeamInfo *pTeam(GameServer.GetTeam(pPlayer->GetTeam()));
            if (pTeam == NULL)
                EX_ERROR(_T("Invalid Team"));

            if (pPlayer->GetLastMapPingTime() == INVALID_TIME || GetGameTime() - pPlayer->GetLastMapPingTime() > sv_mapPingDelay)
            {
                SendPing(GetPing(PING_ALERT), pPlayer->GetHero(), NULL, x, y);
                pPlayer->SetLastMapPingTime(GetGameTime());
            }
        }
        catch (CException &ex)
        {
            ex.Process(_T("CGameServer::ProcessGameData(GAME_CMD_MINIMAP_PING) - "), NO_THROW);
        }
        break;

    case GAME_CMD_SUBMIT_MATCH_COMMENT:
        {
            wstring sComment(pkt.ReadWString());
            if (pPlayer != NULL)
                pPlayer->SetMatchComment(sComment.substr(0, 512));
        }
        break;

#if 0
    case GAME_CMD_SUBMIT_KARMA_RATING:
        {
            bool bAdd(pkt.ReadByte() != 0);
            tstring sComment(pkt.ReadString());
            int iTargetClient(pkt.ReadInt());

            if (StatsTracker.GetMatchID() == -1 || Game.GetGamePhase() != GAME_PHASE_ENDED)
                break;
            
            int iAccountID(-1);
            PlayerMap_it itClient(m_mapClients.find(iClientNum));

            if (itClient == m_mapClients.end())
                break;

            iAccountID = itClient->second->GetAccountID();

            if (m_setKarmaReviewSubmitted.find(iAccountID) != m_setKarmaReviewSubmitted.end())
                break;

            if (itClient->second->GetTeam() < 1)
                break;
            
            CPlayer *pTarget(GetPlayer(iTargetClient));

            if (pTarget == NULL)
                break;

            if (pTarget->GetAccountID() == iAccountID)
                break;

            m_setKarmaReviewSubmitted.insert(iAccountID);
            
            m_pDBManager->AddVariable(_T("f"), _T("upd_karma"));
            m_pDBManager->AddVariable(_T("account_id"), XtoA(iAccountID));
            m_pDBManager->AddVariable(_T("target_id"), XtoA(pTarget->GetAccountID()));
            m_pDBManager->AddVariable(_T("match_id"), XtoA(StatsTracker.GetMatchID()));
            m_pDBManager->AddVariable(_T("do"), bAdd ? _T("add") : _T("remove"));
            m_pDBManager->AddVariable(_T("reason"), sComment);
            m_pDBManager->SendRequest(false);
        }
        break;
#endif

    case GAME_CMD_SHARE_FULL_CONTROL:
        {
            int iShareClient(pkt.ReadInt());
            if (pPlayer != NULL && !pPlayer->HasSharedFullControl(iShareClient))
            {
                pPlayer->ShareFullControl(iShareClient);
                CBufferFixed<6> buffer;
                buffer << GAME_CMD_CONTROL_SHARE_MESSAGE << iClientNum <<
                    byte((pPlayer->HasSharedFullControl(iShareClient) ? 1 : 0) | (pPlayer->HasSharedPartialControl(iShareClient) ? 2 : 0));
                Game.SendGameData(iShareClient, buffer, true);
            }
        }
        break;

    case GAME_CMD_UNSHARE_FULL_CONTROL:
        {
            int iShareClient(pkt.ReadInt());
            if (pPlayer != NULL && pPlayer->HasSharedFullControl(iShareClient))
            {
                pPlayer->UnshareFullControl(iShareClient);
                CBufferFixed<6> buffer;
                buffer << GAME_CMD_CONTROL_SHARE_MESSAGE << iClientNum <<
                    byte((pPlayer->HasSharedFullControl(iShareClient) ? 1 : 0) | (pPlayer->HasSharedPartialControl(iShareClient) ? 2 : 0));
                Game.SendGameData(iShareClient, buffer, true);
            }
        }
        break;

    case GAME_CMD_SHARE_PARTIAL_CONTROL:
        {
            int iShareClient(pkt.ReadInt());
            if (pPlayer != NULL && !pPlayer->HasSharedPartialControl(iShareClient))
            {
                pPlayer->SharePartialControl(iShareClient);
                CBufferFixed<6> buffer;
                buffer << GAME_CMD_CONTROL_SHARE_MESSAGE << iClientNum <<
                    byte((pPlayer->HasSharedFullControl(iShareClient) ? 1 : 0) | (pPlayer->HasSharedPartialControl(iShareClient) ? 2 : 0));
                Game.SendGameData(iShareClient, buffer, true);
            }
        }
        break;

    case GAME_CMD_UNSHARE_PARTIAL_CONTROL:
        {
            int iShareClient(pkt.ReadInt());
            if (pPlayer != NULL && pPlayer->HasSharedPartialControl(iShareClient))
            {
                pPlayer->UnsharePartialControl(iShareClient);
                CBufferFixed<6> buffer;
                buffer << GAME_CMD_CONTROL_SHARE_MESSAGE << iClientNum <<
                    byte((pPlayer->HasSharedFullControl(iShareClient) ? 1 : 0) | (pPlayer->HasSharedPartialControl(iShareClient) ? 2 : 0));
                Game.SendGameData(iShareClient, buffer, true);
            }
        }
        break;

    case GAME_CMD_CYCLE_SHARED_CONTROL:
        {
            int iShareClient(pkt.ReadInt(-1));
            if (pkt.HasFaults())
                return false;

            if (pPlayer == NULL)
                break;
                                
            // we don't want them spamming the button over and over flooding other players with this message,
            // so treat this as a chat message and don't send this update if they are spamming.
            pPlayer->IncrementChatCounter();
            
            if (pPlayer->GetChatCounter() >= sv_chatCounterFloodThreshold)
                break;                          
            
            if (pPlayer->HasSharedFullControl(iShareClient))
                pPlayer->UnsharePartialControl(iShareClient);
            else if (pPlayer->HasSharedPartialControl(iShareClient))
                pPlayer->ShareFullControl(iShareClient);
            else
                pPlayer->SharePartialControl(iShareClient);

            CBufferFixed<6> buffer;
            buffer << GAME_CMD_CONTROL_SHARE_MESSAGE << iClientNum <<
                byte((pPlayer->HasSharedFullControl(iShareClient) ? 1 : 0) | (pPlayer->HasSharedPartialControl(iShareClient) ? 2 : 0));
            Game.SendGameData(iShareClient, buffer, true);
        }
        break;

    case GAME_CMD_SET_NO_HELP:
        {
            int iTargetClient(pkt.ReadInt(-1));
            bool bEnable(pkt.ReadByte() != 0);
            if (pkt.HasFaults())
                return false;

            if (pPlayer != NULL)
                pPlayer->SetNoHelp(GetPlayer(iTargetClient), bEnable);
        }
        break;

    case GAME_CMD_SET_REPLAY_CLIENT:
        {
            int iNewClientNum(pkt.ReadInt());

            if (!ReplayManager.IsPlaying() || iNewClientNum == iClientNum)
                break;

#ifdef K2_CLIENT
            CClientConnection *pClientConnection(m_pHostServer->GetClient());

            if (pClientConnection != NULL)
                pClientConnection->SetClientNumber(iNewClientNum);
#else
            ClientMap &mapClients(m_pHostServer->GetClientMap());
            ClientMap_it it(mapClients.find(iClientNum));
            if (it == mapClients.end())
                break;

            CClientConnection *pClientConnection(it->second);

            mapClients.erase(it);

            pClientConnection->SetClientNumber(iNewClientNum);

            mapClients[iNewClientNum] = pClientConnection;
#endif
        }
        break;

    case GAME_CMD_SERVER_STATUS:
        {
            CBufferDynamic buffer;
            buffer << NETCMD_CONSOLE_MESSAGE << GetServerStatus() << newl << byte(0);

            SendGameData(iClientNum, buffer, true);
        }
        break;

    case GAME_CMD_MOVE_ITEM:
        {
            uint uiUnitIndex(pkt.ReadInt(INVALID_INDEX));
            int iSlot0(pkt.ReadInt(INVALID_INDEX));
            int iSlot1(pkt.ReadInt(INVALID_INDEX));
            
            MoveItem(iClientNum, uiUnitIndex, iSlot0, iSlot1);
            return !pkt.HasFaults();
        }
        break;

    case GAME_CMD_CREATE_GAME:
        {
            wstring sName(pkt.ReadWString());
            wstring sSettings(pkt.ReadWString());

            if (m_pHostServer->GetWorld() != NULL && m_pHostServer->GetWorld()->IsLoaded())
            {
                Console << _T("Game is already started") << newl;
                return true;
            }
            
            CClientConnection *pClientConnection(m_pHostServer->GetClient(iClientNum));
            if (pClientConnection == NULL || !pClientConnection->HasFlags(CLIENT_CONNECTION_GAME_HOST))
            {
                Console << _T("Client is not a host") << newl;
                return true;
            }

            Console << _T("GAME_CMD_CREATE_GAME from client #") << iClientNum << _T(": ") << sSettings << newl;

            CPacket pkt;
            pkt << NETCMD_REMOTE_START_LOADING;
            pClientConnection->SendPacket(pkt);

            wstring sUseSettings(sSettings);
            if (pClientConnection->HasFlags(CLIENT_CONNECTION_TRIAL))
                sUseSettings += _CWS(" nostats:true");
                        
            m_pHostServer->StartGame(sName, sUseSettings);

            m_iCreatorClientNum = iClientNum;
            return !pkt.HasFaults();
        }
        break;

    case GAME_CMD_BUYBACK:
        {
            uint uiUnitIndex(pkt.ReadInt(INVALID_INDEX));
            
            if (BuyBackHero(iClientNum, uiUnitIndex))
            {           
                if (pPlayer == NULL)
                    EX_ERROR(_T("Invalid client ID"));

                CTeamInfo *pTeam(GetTeam(pPlayer->GetTeam()));
                if (pTeam != NULL)
                {
                    CBufferFixed<5> buffer;
                    buffer << GAME_CMD_BUYBACK << iClientNum;

                    ivector viClients(pTeam->GetClientList());

                    for (ivector::iterator it(viClients.begin()); it != viClients.end(); it++)
                    {
                        if (*it == -1)
                            continue;

                        SendGameData(*it, buffer, true);
                    }
                }
            }
            
            return !pkt.HasFaults();                
        }
        break;

    case GAME_CMD_KICK:
        {
            int iTargetClient(pkt.ReadInt(-1));
            if (pkt.HasFaults())
                return false;

            if (pPlayer == NULL || !pPlayer->CanKick())
                return true;

            if (Game.GetGamePhase() > GAME_PHASE_WAITING_FOR_PLAYERS)
                return true;

            CPlayer *pTargetPlayer(GetPlayer(iTargetClient));
            if (pTargetPlayer == NULL || !pTargetPlayer->CanBeKicked())
                return true;

            pTargetPlayer->SetFlags(PLAYER_FLAG_KICKED);
            m_GameLog.WritePlayer(GAME_LOG_PLAYER_KICKED, pTargetPlayer);
            
            CBufferFixed<5> buffer;
            buffer << GAME_CMD_LOBBY_KICK_MESSAGE << iTargetClient;
            BroadcastGameData(buffer, true);

            m_pHostServer->KickClient(iTargetClient, _T("disconnect_kicked"));
            pTargetPlayer->Terminate();

            Console << pTargetPlayer->GetName() << _T(" kicked by ") << pPlayer->GetName() << newl;
            return true;
        }
        break;

    case GAME_CMD_LOCK_SLOT:
        {
            int iTeam(pkt.ReadInt(TEAM_INVALID));
            uint uiSlot(pkt.ReadInt(INVALID_TEAM_SLOT));
            if (pPlayer != NULL && pPlayer->HasFlags(PLAYER_FLAG_HOST) && GetGamePhase() == GAME_PHASE_WAITING_FOR_PLAYERS && !HasGameOptions(GAME_OPTION_AUTOBALANCE_TEAMS))
                LockSlot(iTeam, uiSlot);

            return !pkt.HasFaults();
        }
        break;

    case GAME_CMD_UNLOCK_SLOT:
        {
            int iTeam(pkt.ReadInt(TEAM_INVALID));
            uint uiSlot(pkt.ReadInt(INVALID_TEAM_SLOT));
            if (pPlayer != NULL && pPlayer->HasFlags(PLAYER_FLAG_HOST) && GetGamePhase() == GAME_PHASE_WAITING_FOR_PLAYERS && !HasGameOptions(GAME_OPTION_AUTOBALANCE_TEAMS))
                UnlockSlot(iTeam, uiSlot);

            return !pkt.HasFaults();
        }
        break;

    case GAME_CMD_TOGGLE_SLOT_LOCK:
        {
            int iTeam(pkt.ReadInt(TEAM_INVALID));
            uint uiSlot(pkt.ReadInt(INVALID_TEAM_SLOT));
            if (pPlayer != NULL && pPlayer->HasFlags(PLAYER_FLAG_HOST) && GetGamePhase() == GAME_PHASE_WAITING_FOR_PLAYERS && !HasGameOptions(GAME_OPTION_AUTOBALANCE_TEAMS))
                ToggleSlotLock(iTeam, uiSlot);
                                
            CTeamInfo *pTeam(GetTeam(iTeam));
            if (pTeam == NULL)
                return false;
            
            CBufferFixed<9> buffer;
            
            if (pTeam->IsSlotLocked(uiSlot))
                buffer << GAME_CMD_LOBBY_LOCKED_MESSAGE << iTeam << uiSlot;
            else
                buffer << GAME_CMD_LOBBY_UNLOCKED_MESSAGE << iTeam << uiSlot;
                
            BroadcastGameData(buffer, true);                        
                                                
            return !pkt.HasFaults();
        }
        break;

    case GAME_CMD_BALANCE_TEAMS:
        if (pPlayer != NULL && pPlayer->HasFlags(PLAYER_FLAG_HOST) && GetGamePhase() == GAME_PHASE_WAITING_FOR_PLAYERS && !HasGameOptions(GAME_OPTION_AUTOBALANCE_TEAMS))
            BalanceTeams();
        break;

    case GAME_CMD_SWAP_PLAYER_SLOT:
        {
            int iTeamA(pkt.ReadInt(TEAM_INVALID));
            uint uiSlotA(pkt.ReadInt(INVALID_TEAM_SLOT));
            int iTeamB(pkt.ReadInt(TEAM_INVALID));
            uint uiSlotB(pkt.ReadInt(INVALID_TEAM_SLOT));
            if (pPlayer != NULL && pPlayer->HasFlags(PLAYER_FLAG_HOST) && GetGamePhase() == GAME_PHASE_WAITING_FOR_PLAYERS && !HasGameOptions(GAME_OPTION_AUTOBALANCE_TEAMS))
                ForceSwapPlayerSlots(iTeamA, uiSlotA, iTeamB, uiSlotB);

            return !pkt.HasFaults();
        }
        break;
        
    case GAME_CMD_ASSIGN_SPECTATOR:
        {
            int iTargetClient(pkt.ReadInt(-1));
            if (pkt.HasFaults())
                return false;

            if (pPlayer != NULL && pPlayer->HasFlags(PLAYER_FLAG_HOST) && GetGamePhase() == GAME_PHASE_WAITING_FOR_PLAYERS)
            {
                CPlayer *pTargetPlayer(GetPlayer(iTargetClient));
                if (pTargetPlayer != NULL)
                {
                    RequestChangeTeam(iTargetClient, TEAM_SPECTATOR);
                
                    // We were not able to assign them to TEAM_SPECTATOR, so don't show the assigned spectator message
                    if (pTargetPlayer->GetTeam() != TEAM_SPECTATOR)
                        return true;
                }
                    
                CBufferFixed<5> buffer;
                buffer << GAME_CMD_LOBBY_ASSIGNED_SPECTATOR_MESSAGE << iTargetClient;
                BroadcastGameData(buffer, true);                                            
            }       
            
            return !pkt.HasFaults();
        }
        break;
        
    case GAME_CMD_ASSIGN_HOST:
        {
            uint uiClientNum(pkt.ReadInt(INVALID_TEAM_SLOT));
            
            if (pPlayer != NULL && pPlayer->HasFlags(PLAYER_FLAG_HOST) && GetGamePhase() == GAME_PHASE_WAITING_FOR_PLAYERS)
            {   
                CClientConnection *pClientConnection(m_pHostServer->GetClient(iClientNum));
                CClientConnection *pNewClientConnection(m_pHostServer->GetClient(uiClientNum));
                CPlayer *pNewPlayerHost(GameServer.GetPlayer(uiClientNum));
                
                if (pClientConnection != NULL && pNewClientConnection != NULL && pNewPlayerHost != NULL)
                {               
                    pPlayer->RemoveFlags(PLAYER_FLAG_HOST);
                    pClientConnection->RemoveFlags(CLIENT_CONNECTION_GAME_HOST);                                
                
                    pNewPlayerHost->SetFlags(PLAYER_FLAG_HOST);
                    pNewClientConnection->SetFlags(CLIENT_CONNECTION_GAME_HOST);
                    
                    m_iCreatorClientNum = uiClientNum;
                    
                    CBufferFixed<5> buffer;
                    buffer << GAME_CMD_LOBBY_ASSIGNED_HOST_MESSAGE << uiClientNum;
                    BroadcastGameData(buffer, true);                        
                }               
            }   

            return !pkt.HasFaults();
        }
        break;

    case GAME_CMD_PROMOTE_REFEREE:
        {
            int iTargetClient(pkt.ReadInt(-1));
            if (pkt.HasFaults())
                return false;

            if (pPlayer != NULL && pPlayer->HasFlags(PLAYER_FLAG_HOST) && GetGamePhase() == GAME_PHASE_WAITING_FOR_PLAYERS)
            {
                CPlayer *pTargetPlayer(GetPlayer(iTargetClient));
                if (pTargetPlayer != NULL && !pTargetPlayer->IsReferee())
                {
                    RequestChangeTeam(iTargetClient, TEAM_SPECTATOR, INVALID_TEAM_SLOT, true);
                    
                    // We were not able to assign them to TEAM_SPECTATOR, so don't show the assigned spectator message
                    if (pTargetPlayer->GetTeam() != TEAM_SPECTATOR)
                        return true;

                    pTargetPlayer->SetReferee(true);
                    
                    CBufferFixed<6> buffer;
                    buffer << GAME_CMD_LOBBY_ASSIGNED_REFEREE_MESSAGE2 << byte(1) << iTargetClient;
                    BroadcastGameData(buffer, true);                                            
                }
            }

            return true;
        }
        break;

    case GAME_CMD_DEMOTE_REFEREE:
        {
            int iTargetClient(pkt.ReadInt(-1));
            if (pkt.HasFaults())
                return false;

            if (pPlayer != NULL && pPlayer->HasFlags(PLAYER_FLAG_HOST) && GetGamePhase() == GAME_PHASE_WAITING_FOR_PLAYERS)
            {
                CPlayer *pTargetPlayer(GetPlayer(iTargetClient));
                if (pTargetPlayer != NULL && pTargetPlayer->IsReferee())
                    RequestChangeTeam(iTargetClient, TEAM_PASSIVE);
            }

            return true;
        }
        break;

    case GAME_CMD_BAN_HERO:
        {
            ushort unHeroID(pkt.ReadShort(INVALID_ENT_TYPE));
            if (GetGamePhase() == GAME_PHASE_HERO_BAN && pPlayer != NULL && pPlayer->HasFlags(PLAYER_FLAG_IS_CAPTAIN) && pPlayer->GetTeam() == m_uiBanningTeam && GetHeroStatus(unHeroID) != HERO_LIST_BANNED)
            {
                CTeamInfo *pTeam(GetTeam(pPlayer->GetTeam()));
                if (pTeam != NULL && int(pTeam->GetBanCount()) <= m_iBanRound)
                {
                    SetHeroStatus(unHeroID, HERO_LIST_BANNED);
                    pTeam->IncrementBanCount();
                    for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
                        ClearPotentialHero(it->second, unHeroID);

                    m_GameLog.WritePlayer(GAME_LOG_PLAYER_BAN, pPlayer, EntityRegistry.LookupName(unHeroID));

                    CBufferFixed<7> buffer;
                    buffer << GAME_CMD_BAN_HERO_MESSAGE << pPlayer->GetClientNumber() << unHeroID;
                    BroadcastGameData(buffer, true);
                }
            }
            return true;
        }
        break;

    case GAME_CMD_DRAFT_HERO:
        {
            ushort unHeroID(pkt.ReadShort(INVALID_ENT_TYPE));
            if (GetGamePhase() == GAME_PHASE_HERO_SELECT && pPlayer != NULL && pPlayer->HasFlags(PLAYER_FLAG_IS_CAPTAIN))
                SetHeroStatus(unHeroID, pPlayer->GetTeam());
            return true;
        }
        break;

    case GAME_CMD_REQUEST_EXT_ENTITY_DATA:
        {
            uint uiIndex(pkt.ReadInt(INVALID_INDEX));
            IGameEntity *pEntity(GetEntity(uiIndex));
            if (pEntity != NULL)
                pEntity->SendExtendedData(iClientNum);
        }
        break;

    case GAME_CMD_SET_ATTACK_MOD_SLOT:
        {
            uint uiIndex(pkt.ReadInt(INVALID_INDEX));
            int iSlot(pkt.ReadInt(MAX_INVENTORY));

            IUnitEntity *pUnit(Game.GetUnitEntity(uiIndex));
            if (pUnit != NULL && pUnit->CanActOnOrdersFrom(iClientNum))
                pUnit->SetExclusiveAttackModSlot(iSlot);
        }
        return !pkt.HasFaults();

    case GAME_CMD_PREV_ATTACK_MOD_SLOT:
        {
            uint uiIndex(pkt.ReadInt(INVALID_INDEX));

            IUnitEntity *pUnit(Game.GetUnitEntity(uiIndex));
            if (pUnit != NULL && pUnit->CanActOnOrdersFrom(iClientNum))
                pUnit->SetExclusiveAttackModSlot(pUnit->GetPrevExclusiveAttackModSlot());
        }
        return !pkt.HasFaults();
        
    case GAME_CMD_NEXT_ATTACK_MOD_SLOT:
        {
            uint uiIndex(pkt.ReadInt(INVALID_INDEX));

            IUnitEntity *pUnit(Game.GetUnitEntity(uiIndex));
            if (pUnit != NULL && pUnit->CanActOnOrdersFrom(iClientNum))
                pUnit->SetExclusiveAttackModSlot(pUnit->GetNextExclusiveAttackModSlot());
        }
        return !pkt.HasFaults();

    case GAME_CMD_SELECTION:
        {
            uiset uiSelection;

            ushort unIndex(pkt.ReadShort(ushort(-1)));
            while (unIndex != ushort(-1))
            {
                uiSelection.insert(unIndex);
                unIndex = pkt.ReadShort(ushort(-1));
            }

            SetSelection(iClientNum, uiSelection);
            return !pkt.HasFaults();
        }
        break;
                    
    case GAME_CMD_SCRIPT_MESSAGE:
        {
            tstring sName(WStringToTString(pkt.ReadWString()));
            tstring sValue(WStringToTString(pkt.ReadWString()));

            if (GetWorldPointer() != NULL && GetWorldPointer()->GetName() == _T("tutorial"))
            {
                CGameInfo *pGameInfo(GetGameInfo());
                if (pGameInfo != NULL)
                    pGameInfo->SetScriptValue(sName, sValue);
            }
        }
        break;

    case GAME_CMD_GAMEPLAY_OPTION:
        {
            tstring sName(WStringToTString(pkt.ReadWString()));
            tstring sValue(WStringToTString(pkt.ReadWString()));

            if (pPlayer != NULL)
                pPlayer->ProcessGameplayOption(sName, sValue);

            return !pkt.HasFaults();
        }
        break;

    case GAME_CMD_NOTIFICATION_FLAGS:
        {
            uint uiNotificationFlags(pkt.ReadInt());
            byte ySetOrClear(pkt.ReadByte());

            // since we can't ever change the structure of game messages
            // once we deploy them, it seems like a good idea to include
            // a "reserved" buffer into messages like this one
            // in case we need to extend its functionality in the future.
            uint uiReservedDataLen(CLAMP(pkt.ReadInt(), 0, 64));
            if (uiReservedDataLen > 0)
            {
                char pBuf[65];
                size_t uiLen(pkt.Read(pBuf, uiReservedDataLen));
                uiLen = uiLen;
                // process any future extensions to this command here.
            }

            if (Host.IsReplay())
                return true;

            if (m_pHostServer->GetPractice())
                return true;

            if (pPlayer != NULL)
            {
                if (ySetOrClear != 0)
                    pPlayer->SetNotificationFlags(uiNotificationFlags);
                else
                    pPlayer->ClearNotificationFlags(uiNotificationFlags);

                // disconnect players who have modified their core game files (e.g. resources0.s2z)
                if (pPlayer->HasNotificationFlags(NOTIFY_FLAG_MODIFIED_CORE_FILES))
                {
                    CClientConnection *pClientConnection(m_pHostServer->GetClient(pPlayer->GetClientNumber()));
                    if (pClientConnection != NULL)
                    {
                        pClientConnection->Disconnect(_T("disconnect_modified_core_files"));
                    }
                    else
                    {
                        assert(false);
                        Console << _T("Could not disconnect player ") << pPlayer->GetClientNumber() << _T(" for modifying core game files!") << newl;
                    }
                }

                // disallow mods in Tournament Rules games.
                if (HasGameOptions(GAME_OPTION_TOURNAMENT_RULES))
                {
                    if (pPlayer->HasNotificationFlags(NOTIFY_FLAG_CUSTOM_FILES))
                    {
                        if (GetGamePhase() <= GAME_PHASE_WAITING_FOR_PLAYERS)
                            SendGeneralMessage(_CWS("warn_nomods"), pPlayer->GetClientNumber());
                        else
                        {
                            if (pPlayer->GetTeam() >= TEAM_1 && pPlayer->GetTeam() <= TEAM_2)
                            {
                                CClientConnection *pClientConnection(m_pHostServer->GetClient(pPlayer->GetClientNumber()));
                                if (pClientConnection != NULL)
                                    pClientConnection->Disconnect(_T("disconnect_no_mods_allowed"));
                            }
                        }
                    }
                }
            }

            return !pkt.HasFaults();
        }
        break;

    case GAME_CMD_VERIFY_FILES:
        {
            byte yVersion(pkt.ReadByte());
            if (yVersion == 0)
            {
                byte pBaseResources0Checksum[CHECKSUM_SIZE];
                byte pGameResources0Checksum[CHECKSUM_SIZE];
                tstring sBaseResources0Checksum;
                tstring sGameResources0Checksum;
                memset(pBaseResources0Checksum, 0, CHECKSUM_SIZE);
                memset(pGameResources0Checksum, 0, CHECKSUM_SIZE);
                pkt.Read((char *)pBaseResources0Checksum, CHECKSUM_SIZE);
                pkt.Read((char *)pGameResources0Checksum, CHECKSUM_SIZE);

                if (m_pHostServer->GetPractice())
                    return true;

                if (!pkt.HasFaults())
                {
                    CChecksumTable::ChecksumToString(sBaseResources0Checksum, pBaseResources0Checksum);
                    CChecksumTable::ChecksumToString(sGameResources0Checksum, pGameResources0Checksum);

                    if (pPlayer != NULL)
                    {
                        CClientConnection *pClientConnection(m_pHostServer->GetClient(pPlayer->GetClientNumber()));
                        if (pClientConnection != NULL && pClientConnection->GetConnectionState() != CLIENT_CONNECTION_STATE_DISCONNECTED)
                        {
                            // TODO: Silently detect whether the player's checksums are valid.  If they aren't,
                            // flag the player as a hacker but do NOT disconnect them.
                            Console << _T("Player ") << pPlayer->GetName() << _T(" has the following checksums: ") << newl;
                            Console << _T("     base resources0.s2z: ") << sBaseResources0Checksum << newl;
                            Console << _T("     game resources0.s2z: ") << sGameResources0Checksum << newl;
                        }
                    }
                }
            }

            return !pkt.HasFaults();
        }
        break;

    case GAME_CMD_START_SPRINT:
        {
            if (pPlayer == NULL)
            {
                Console.Warn << _T("GAME_CMD_START_SPRINT: No entity found for client #") << iClientNum << newl;
                break;
            }

            pPlayer->SetSprinting(true);
            break;
        }

    case GAME_CMD_STOP_SPRINT:
        {
            if (pPlayer == NULL)
            {
                Console.Warn << _T("GAME_CMD_STOP_SPRINT: No entity found for client #") << iClientNum << newl;
                break;
            }

            pPlayer->SetSprinting(false);
            break;
        }
                        
    default:
        Console.Warn << _T("Unrecognized message: '") << yCmd << _T("'") << newl;
        return false;
    }

    return true;
}


/*====================
  CGameServer::GetSnapshot
  ====================*/
void    CGameServer::GetSnapshot(CSnapshot &snapshot)
{
    GAME_PROFILE(_T("CGameServer::GetSnapshot"));

    if (ReplayManager.IsPlaying())
    {
        //int iClientNum(m_mapClients.empty() ? -1 : m_mapClients.begin()->first);
        //CClientConnection *pClient(GameServer.GetHostServer()->GetClient(iClientNum));

        ReplayManager.GetSnapshot(snapshot);
    }
    else
    {
        // Entities
        m_pServerEntityDirectory->GetSnapshot(snapshot);

        // Events
        GetEventSnapshot(snapshot);
        ClearEventList();

        m_pHostServer->UpdateStateStrings();
    }
}


/*====================
  CGameServer::EndFrame
  ====================*/
void    CGameServer::EndFrame(PoolHandle hSnapshot)
{
    GAME_PROFILE(_T("CGameServer::EndFrame"));
    
    ReplayManager.EndFrame(hSnapshot);
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
        ReplayManager.StopPlayback();

        CFileHandle hFile(_T("~/game_settings_local.cfg"), FILE_WRITE | FILE_TEXT);
        if (!hFile.IsOpen())
            EX_ERROR(_T("Failed to open config file"));

        tsvector    vsWildCards;
        if (!ICvar::WriteConfigFile(hFile, vsWildCards, CVAR_GAMECONFIG))
            EX_WARN(_T("Error writing cvars"));

        m_pServerEntityDirectory->Clear();
    }
    catch (CException &ex)
    {
        ex.Process(_T("CGameServer::Shutdown() - "), NO_THROW);
    }
}


/*====================
  CGameServer::BuildHeroLists
  ====================*/
void    CGameServer::BuildHeroLists()
{
    srand(uint(K2System.GetTicks() & UINT_MAX));

    // Single draft
    if (GetGameMode() == GAME_MODE_SINGLE_DRAFT)
    {
        // Build a list for each attribute
        vector<ushort> vAgiHeroes;
        EntityRegistry.GetHeroList(_T("Legion"), vAgiHeroes, ATTRIBUTE_AGILITY);
        EntityRegistry.GetHeroList(_T("Hellbourne"), vAgiHeroes, ATTRIBUTE_AGILITY);
        vector<ushort> vIntHeroes;
        EntityRegistry.GetHeroList(_T("Legion"), vIntHeroes, ATTRIBUTE_INTELLIGENCE);
        EntityRegistry.GetHeroList(_T("Hellbourne"), vIntHeroes, ATTRIBUTE_INTELLIGENCE);
        vector<ushort> vStrHeroes;
        EntityRegistry.GetHeroList(_T("Legion"), vStrHeroes, ATTRIBUTE_STRENGTH);
        EntityRegistry.GetHeroList(_T("Hellbourne"), vStrHeroes, ATTRIBUTE_STRENGTH);
        if (HasGameOptions(GAME_OPTION_DEV_HEROES))
        {
            EntityRegistry.GetHeroList(_T("Dev_Legion"), vAgiHeroes, ATTRIBUTE_AGILITY);
            EntityRegistry.GetHeroList(_T("Dev_Hellbourne"), vAgiHeroes, ATTRIBUTE_AGILITY);
            EntityRegistry.GetHeroList(_T("Dev_Legion"), vIntHeroes, ATTRIBUTE_INTELLIGENCE);
            EntityRegistry.GetHeroList(_T("Dev_Hellbourne"), vIntHeroes, ATTRIBUTE_INTELLIGENCE);
            EntityRegistry.GetHeroList(_T("Dev_Legion"), vStrHeroes, ATTRIBUTE_STRENGTH);
            EntityRegistry.GetHeroList(_T("Dev_Hellbourne"), vStrHeroes, ATTRIBUTE_STRENGTH);
        }

        // Randomize lists
        std::random_shuffle(vAgiHeroes.begin(), vAgiHeroes.end());
        std::random_shuffle(vIntHeroes.begin(), vIntHeroes.end());
        std::random_shuffle(vStrHeroes.begin(), vStrHeroes.end());

        // Create a list for each player with one hero from each attribute list
        for (uint ui(0); ui < NUM_HERO_LISTS; ++ui)
        {
            if (vAgiHeroes.size() > 0)
                m_vHeroLists[ui].push_back(HeroListEntry(vAgiHeroes[ui % vAgiHeroes.size()], HERO_LIST_AVAILABLE_ALL));
            if (vIntHeroes.size() > 0)
                m_vHeroLists[ui].push_back(HeroListEntry(vIntHeroes[ui % vIntHeroes.size()], HERO_LIST_AVAILABLE_ALL));
            if (vStrHeroes.size() > 0)
                m_vHeroLists[ui].push_back(HeroListEntry(vStrHeroes[ui % vStrHeroes.size()], HERO_LIST_AVAILABLE_ALL));
        }
    }
    else if (GetGameMode() == GAME_MODE_RANDOM_DRAFT)
    {
        // Get a single list of all heroes
        vector<ushort> vHeroes;
        EntityRegistry.GetHeroList(_T("Legion"), vHeroes);
        EntityRegistry.GetHeroList(_T("Hellbourne"), vHeroes);
        if (HasGameOptions(GAME_OPTION_DEV_HEROES))
        {
            EntityRegistry.GetHeroList(_T("Dev_Legion"), vHeroes);
            EntityRegistry.GetHeroList(_T("Dev_Hellbourne"), vHeroes);
        }

        // Generate a random selection
        std::random_shuffle(vHeroes.begin(), vHeroes.end());
        if (vHeroes.size() > GetGameInfo()->GetHeroPoolSize())
            vHeroes.resize(GetGameInfo()->GetHeroPoolSize());

        for (uint ui(0); ui < MIN(INT_SIZE(vHeroes.size()), GetGameInfo()->GetHeroPoolSize()); ++ui)
            m_vHeroLists[0].push_back(HeroListEntry(vHeroes[ui], HERO_LIST_AVAILABLE_ALL));
    }
    else if (GetGameMode() == GAME_MODE_CAPTAINS_DRAFT || GetGameMode() == GAME_MODE_BANNING_DRAFT)
    {
        // Build a list for each attribute
        vector<ushort> vAgiHeroes;
        EntityRegistry.GetHeroList(_T("Legion"), vAgiHeroes, ATTRIBUTE_AGILITY);
        EntityRegistry.GetHeroList(_T("Hellbourne"), vAgiHeroes, ATTRIBUTE_AGILITY);
        vector<ushort> vIntHeroes;
        EntityRegistry.GetHeroList(_T("Legion"), vIntHeroes, ATTRIBUTE_INTELLIGENCE);
        EntityRegistry.GetHeroList(_T("Hellbourne"), vIntHeroes, ATTRIBUTE_INTELLIGENCE);
        vector<ushort> vStrHeroes;
        EntityRegistry.GetHeroList(_T("Legion"), vStrHeroes, ATTRIBUTE_STRENGTH);
        EntityRegistry.GetHeroList(_T("Hellbourne"), vStrHeroes, ATTRIBUTE_STRENGTH);
        if (HasGameOptions(GAME_OPTION_DEV_HEROES))
        {
            EntityRegistry.GetHeroList(_T("Dev_Legion"), vAgiHeroes, ATTRIBUTE_AGILITY);
            EntityRegistry.GetHeroList(_T("Dev_Hellbourne"), vAgiHeroes, ATTRIBUTE_AGILITY);
            EntityRegistry.GetHeroList(_T("Dev_Legion"), vIntHeroes, ATTRIBUTE_INTELLIGENCE);
            EntityRegistry.GetHeroList(_T("Dev_Hellbourne"), vIntHeroes, ATTRIBUTE_INTELLIGENCE);
            EntityRegistry.GetHeroList(_T("Dev_Legion"), vStrHeroes, ATTRIBUTE_STRENGTH);
            EntityRegistry.GetHeroList(_T("Dev_Hellbourne"), vStrHeroes, ATTRIBUTE_STRENGTH);
        }

        // Randomize lists
        std::random_shuffle(vAgiHeroes.begin(), vAgiHeroes.end());
        std::random_shuffle(vIntHeroes.begin(), vIntHeroes.end());
        std::random_shuffle(vStrHeroes.begin(), vStrHeroes.end());

        for (uint ui(0); ui < MIN(INT_SIZE(vAgiHeroes.size()), GetGameInfo()->GetHeroPoolSize() / 3); ++ui)
            m_vHeroLists[0].push_back(HeroListEntry(vAgiHeroes[ui], HERO_LIST_AVAILABLE_ALL));
        for (uint ui(0); ui < MIN(INT_SIZE(vIntHeroes.size()), GetGameInfo()->GetHeroPoolSize() / 3); ++ui)
            m_vHeroLists[0].push_back(HeroListEntry(vIntHeroes[ui], HERO_LIST_AVAILABLE_ALL));
        for (uint ui(0); ui < MIN(INT_SIZE(vStrHeroes.size()), GetGameInfo()->GetHeroPoolSize() / 3); ++ui)
            m_vHeroLists[0].push_back(HeroListEntry(vStrHeroes[ui], HERO_LIST_AVAILABLE_ALL));
    }
    else
    {
        vector<ushort> vHeroes;

        // Legion Agility
        EntityRegistry.GetHeroList(_T("Legion"), vHeroes, ATTRIBUTE_AGILITY);
        if (HasGameOptions(GAME_OPTION_DEV_HEROES))
            EntityRegistry.GetHeroList(_T("Dev_Legion"), vHeroes, ATTRIBUTE_AGILITY);
        for (vector<ushort>::iterator it(vHeroes.begin()); it != vHeroes.end(); ++it)
            m_vHeroLists[0].push_back(HeroListEntry(*it, HERO_LIST_AVAILABLE_ALL));
        vHeroes.clear();

        // Hellbourne Agility
        EntityRegistry.GetHeroList(_T("Hellbourne"), vHeroes, ATTRIBUTE_AGILITY);
        if (HasGameOptions(GAME_OPTION_DEV_HEROES))
            EntityRegistry.GetHeroList(_T("Dev_Hellbourne"), vHeroes, ATTRIBUTE_AGILITY);
        for (vector<ushort>::iterator it(vHeroes.begin()); it != vHeroes.end(); ++it)
            m_vHeroLists[1].push_back(HeroListEntry(*it, HERO_LIST_AVAILABLE_ALL));
        vHeroes.clear();

        // Legion Intelligence
        EntityRegistry.GetHeroList(_T("Legion"), vHeroes, ATTRIBUTE_INTELLIGENCE);
        if (HasGameOptions(GAME_OPTION_DEV_HEROES))
            EntityRegistry.GetHeroList(_T("Dev_Legion"), vHeroes, ATTRIBUTE_INTELLIGENCE);
        for (vector<ushort>::iterator it(vHeroes.begin()); it != vHeroes.end(); ++it)
            m_vHeroLists[2].push_back(HeroListEntry(*it, HERO_LIST_AVAILABLE_ALL));
        vHeroes.clear();

        // Hellbournce Intelligence
        EntityRegistry.GetHeroList(_T("Hellbourne"), vHeroes, ATTRIBUTE_INTELLIGENCE);
        if (HasGameOptions(GAME_OPTION_DEV_HEROES))
            EntityRegistry.GetHeroList(_T("Dev_Hellbourne"), vHeroes, ATTRIBUTE_INTELLIGENCE);
        for (vector<ushort>::iterator it(vHeroes.begin()); it != vHeroes.end(); ++it)
            m_vHeroLists[3].push_back(HeroListEntry(*it, HERO_LIST_AVAILABLE_ALL));
        vHeroes.clear();

        // Legion Strength
        EntityRegistry.GetHeroList(_T("Legion"), vHeroes, ATTRIBUTE_STRENGTH);
        if (HasGameOptions(GAME_OPTION_DEV_HEROES))
            EntityRegistry.GetHeroList(_T("Dev_Legion"), vHeroes, ATTRIBUTE_STRENGTH);
        for (vector<ushort>::iterator it(vHeroes.begin()); it != vHeroes.end(); ++it)
            m_vHeroLists[4].push_back(HeroListEntry(*it, HERO_LIST_AVAILABLE_ALL));
        vHeroes.clear();

        // Hellbourne Strength
        EntityRegistry.GetHeroList(_T("Hellbourne"), vHeroes, ATTRIBUTE_STRENGTH);
        if (HasGameOptions(GAME_OPTION_DEV_HEROES))
            EntityRegistry.GetHeroList(_T("Dev_Hellbourne"), vHeroes, ATTRIBUTE_STRENGTH);
        for (vector<ushort>::iterator it(vHeroes.begin()); it != vHeroes.end(); ++it)
            m_vHeroLists[5].push_back(HeroListEntry(*it, HERO_LIST_AVAILABLE_ALL));
        vHeroes.clear();
    }

    for (ushort unBlock(STATE_BLOCK_FIRST_HERO_GROUP); unBlock <= STATE_BLOCK_LAST_HERO_GROUP; ++unBlock)
        m_pHostServer->AddStateBlock(unBlock);

    UpdateHeroList();
}


/*====================
  CGameServer::LoadWorld
  ====================*/
bool    CGameServer::LoadWorld(const tstring &sName, const tstring &sGameSettings)
{
    // Save thread priority
    int iOldPriority(m_pHostServer->HasManager() ? K2System.GetPriority() : 0);

    try
    {
        GetStateString(STATE_STRING_RESOURCES).Validate(); // DEBUG

        if (m_pHostServer->HasManager())
            K2System.SetPriority(-1);

        NetworkResourceManager.Clear();

        // Parse out game info
        tsvector vGameSettingPairs(TokenizeString(sGameSettings, _T(' ')));
        tsmapts mapGameSettings;
        for (tsvector_it it(vGameSettingPairs.begin()); it != vGameSettingPairs.end(); ++it)
        {
            size_t zDiv(it->find(_T(':')));
            if (zDiv == tstring::npos)
                continue;

            mapGameSettings[it->substr(0, zDiv)] = it->substr(zDiv + 1);
        }

        tsmapts_it itLocal(mapGameSettings.find(_T("local")));
        m_bLocal = AtoB(itLocal->second);
        
        tsmapts_it itMap(mapGameSettings.find(_CWS("map")));
        if (itMap == mapGameSettings.end())
            EX_ERROR(_T("No map specified"));

        if (sName.empty())
            m_sName = _T("Heroes of Newerth");
        else
            m_sName = sName;

        if (m_sName.length() > 64)
            m_sName = m_sName.substr(0, 64);

        tsmapts_it itPrivate(mapGameSettings.find(_T("private")));
        if (itPrivate != mapGameSettings.end())
            m_pHostServer->SetServerAccess(AtoB(itPrivate->second) ? ACCESS_INVITEONLY : ACCESS_PUBLIC);
        else
            m_pHostServer->SetServerAccess(ACCESS_PUBLIC);
        
        tsmapts_it itTournRules(mapGameSettings.find(_T("tournamentrules")));
        if (itTournRules != mapGameSettings.end())
            m_pHostServer->SetServerAccess(AtoB(itPrivate->second) ? ACCESS_INVITEONLY : m_pHostServer->GetServerAccess());

        // Load the world
        IModalDialog::NextLoadingJob();
        IModalDialog::SetDisplay(LOADING_DISPLAY_INTERFACE);
        IModalDialog::Show();

        GetWorldPointer()->SetVisibilitySize(int(g_fogofwarSize));

        if (!GetWorldPointer()->StartLoad(itMap->second))
            EX_ERROR(_T("Failed loading world"));

        if (!m_bLocal)
        {
            if (GetWorldPointer()->GetDev() && !sv_allowDev)
                EX_ERROR(_T("Dev maps are not allowed on this server"));
        }

        while (GetWorldPointer()->IsLoading())
        {
            if (!GetWorldPointer()->LoadNextComponent())
                EX_ERROR(_T("Failed loading world"));

            IModalDialog::SetProgress(GetWorldPointer()->GetLoadProgress());
            IModalDialog::Update();

            // Play nice with other instances
            if (K2System.IsDedicatedServer())
                K2System.Sleep(0);
        }

        IModalDialog::Hide();

        IModalDialog::NextLoadingJob();

        PrecacheEntities(false);

        if (ReplayManager.IsPlaying())
            return true;

        // Setup a new GameInfo
        IGameEntity *pNewGameInfo(m_pServerEntityDirectory->Allocate(_T("Game_Rules")));
        if (pNewGameInfo == NULL || !pNewGameInfo->IsGameInfo())
            EX_ERROR(_T("Failed to allocate CGameInfo entity"));

        CDate date(true);

        m_cMatchDate = date;

        CGameInfo *pGameInfo(static_cast<CGameInfo*>(pNewGameInfo));
        SetGameInfo(pGameInfo);
        pGameInfo->Initialize();
        pGameInfo->SetServerDate(date.GetDateString(DATE_YEAR_LAST | DATE_MONTH_FIRST | DATE_SHORT_YEAR));
        pGameInfo->SetServerTime(date.GetTimeString(TIME_TWELVE_HOUR | TIME_NO_SECONDS));
        pGameInfo->SetServerName(svr_name);
        pGameInfo->SetGameName(m_sName);
        pGameInfo->SetServerAccess(m_pHostServer->GetServerAccess());
        
        if (m_pHostServer->IsArrangedMatch())
            pGameInfo->SetFlags(GAME_FLAG_ARRANGED);

        tsmapts_it itSolo(mapGameSettings.find(_T("solo")));
        if (itSolo != mapGameSettings.end() && AtoB(itSolo->second))
        {
            pGameInfo->SetFlags(GAME_FLAG_SOLO);
            m_bSolo = true;
        }
        else
        {
            m_bSolo = false;
        }

        // Read game settings
        tsmapts_it itMode(mapGameSettings.find(_T("mode")));
        if (itMode == mapGameSettings.end())
        {
            SetGameMode(GAME_MODE_NORMAL);
            Console << _T("No mode specified, defaulting to normal") << newl;
        }
        else
        {
            SetGameMode(CGameInfo::GetGameModeFromString(itMode->second));
        }

        tsmapts_it itTeamSize(mapGameSettings.find(_T("teamsize")));
        if (itTeamSize == mapGameSettings.end())
        {
            SetTeamSize(GetWorldPointer()->GetMaxPlayers());
            Console << _T("No team size specified, defaulting to maximum for map: ") << GetWorldPointer()->GetMaxPlayers() << newl;
        }
        else
        {
            SetTeamSize(CLAMP(AtoI(itTeamSize->second), 1, GetWorldPointer()->GetMaxPlayers()));
        }

        // Spectators
        tsmapts_it itSpectators(mapGameSettings.find(_T("spectators")));
        if (itSpectators == mapGameSettings.end())
            SetMaxSpectators(0);
        else
            SetMaxSpectators(CLAMP(AtoI(itSpectators->second), 0, MAX_TOTAL_SPECTATORS));

        // Refereees
        tsmapts_it itReferees(mapGameSettings.find(_T("referees")));
        if (itReferees == mapGameSettings.end())
            SetMaxReferees(0);
        else
            SetMaxReferees(CLAMP(AtoI(itReferees->second), 0, MAX_TOTAL_REFEREES));

        // Tier
        tsmapts_it itTier(mapGameSettings.find(_T("tier")));
        if (itTier == mapGameSettings.end())
            m_pHostServer->SetTier(1);
        else
            m_pHostServer->SetTier(CLAMP(AtoI(itTier->second), 0, 2));
            
        // Min/Max PSR
        tsmapts_it itMinPSR(mapGameSettings.find(_T("minpsr")));
        if (itMinPSR == mapGameSettings.end())
            m_pHostServer->SetMinPSR(0);
        else
            m_pHostServer->SetMinPSR(AtoI(itMinPSR->second));
            
        // Min/Max PSR
        tsmapts_it itMaxPSR(mapGameSettings.find(_T("maxpsr")));
        if (itMaxPSR == mapGameSettings.end())
            m_pHostServer->SetMaxPSR(0);
        else
            m_pHostServer->SetMaxPSR(AtoI(itMaxPSR->second));           
            
        pGameInfo->SetMinPSR(m_pHostServer->GetMinPSR());
        pGameInfo->SetMaxPSR(m_pHostServer->GetMaxPSR());           

        // No leaver
        tsmapts_it itNoLeaver(mapGameSettings.find(_T("noleaver")));
        if (itNoLeaver == mapGameSettings.end())
            m_pHostServer->SetNoLeaver(false);
        else
            m_pHostServer->SetNoLeaver(AtoB(itNoLeaver->second));

        // No Stats
        tsmapts_it itNoStats(mapGameSettings.find(_T("nostats")));
        if (itNoStats == mapGameSettings.end())
            m_pHostServer->SetNoStats(false);
        else
            m_pHostServer->SetNoStats(AtoB(itNoStats->second));

        for (tsmapts_it itOption(mapGameSettings.begin()); itOption != mapGameSettings.end(); ++itOption)
        {
            uint uiOption(CGameInfo::GetGameOptionFromString(itOption->first));
            if (uiOption == GAME_OPTION_INVALID)
                continue;
            if (AtoB(itOption->second))
                SetGameOptions(uiOption);
            else
                ClearGameOptions(uiOption);
        }

        pGameInfo->ValidateOptions();

        GetMatchIDFromMasterServer();

        m_pHostServer->StartGame(pGameInfo->GetMatchID());

        if (sv_autosaveGameLog)
            m_GameLog.Open(pGameInfo->GetMatchID());

        m_GameLog.WriteInfo(GAME_LOG_INFO_DATE, _T("date"), date.GetDateString(), _T("time"), date.GetTimeString());
        m_GameLog.WriteInfo(GAME_LOG_INFO_SERVER, _T("name"), svr_name, _T("address"), TSNULL);
        m_GameLog.WriteInfo(GAME_LOG_INFO_GAME, _T("name"), K2System.GetGameName(), _T("version"), K2System.GetVersionString());
        m_GameLog.WriteInfo(GAME_LOG_INFO_MATCH, _T("name"), GetGameName(), _T("id"), XtoA(pGameInfo->GetMatchID()));
        m_GameLog.WriteInfo(GAME_LOG_INFO_MAP, _T("name"), GetWorldPointer()->GetName(), _T("version"), GetWorldPointer()->GetVersionString());
        m_GameLog.WriteInfo(GAME_LOG_INFO_SETTINGS, _T("mode"), pGameInfo->GetGameModeString(pGameInfo->GetGameMode()), _T("options"), pGameInfo->GetGameOptionsString(pGameInfo->GetGameOptions()));

        // Add game mode and options to the global modifier list
        ClearGlobalModifiers();
        AddGlobalModifier(EntityRegistry.RegisterModifier(CGameInfo::GetGameModeName(GetGameMode())));
        for (uint uiBit(1); uiBit != 0; uiBit <<= 1)
        {
            if (HasGameOptions(uiBit))
                AddGlobalModifier(EntityRegistry.RegisterModifier(CGameInfo::GetGameOptionName(uiBit)));
        }
        pGameInfo->SetModifierBits(pGameInfo->GetModifierBits(GetGlobalModifiers()));

        // Add any clients that are already connected
        for (PlayerMap_it itPlayer(m_mapClients.begin()); itPlayer != m_mapClients.end(); ++itPlayer)
            m_GameLog.WritePlayer(GAME_LOG_PLAYER_CONNECT, itPlayer->second);

        // Build a list of heroes
        BuildHeroLists();

        EntityRegistry.GetAutoRecipeList(m_vAutoRecipes);

        GetStateString(STATE_STRING_RESOURCES).Validate(); // DEBUG

        // Builds pathing blockers based on the normals of the map's terrain
        AnalyzeTerrain();

        // Initialize team visibility maps
        int iSize(1 << int(g_fogofwarSize));

        m_uiVisibilitySize = iSize;
        m_fVisibilityScale = m_uiVisibilitySize * GetWorldPointer()->GetScale(); 
        m_cVisibilityMap[0].Initialize(GetWorldPointer()->GetTileWidth() / m_uiVisibilitySize, GetWorldPointer()->GetTileHeight() / m_uiVisibilitySize);
        m_cVisibilityMap[1].Initialize(GetWorldPointer()->GetTileWidth() / m_uiVisibilitySize, GetWorldPointer()->GetTileHeight() / m_uiVisibilitySize);

        // Allocate teams
        ClearTeams();
        CTeamInfo *pTeam(NULL);
        pTeam = AddTeam(_T("Spectators"), WHITE, TEAM_SPECTATOR);
        if (pTeam != NULL)
            pTeam->SetTeamSize(GetMaxSpectators() + GetMaxReferees());
        pTeam = AddTeam(_T("Legion"), RED, TEAM_1);
        if (pTeam != NULL)
            pTeam->SetTeamSize(GetTeamSize());
        pTeam = AddTeam(_T("Hellbourne"), GREEN, TEAM_2);
        if (pTeam != NULL)
            pTeam->SetTeamSize(GetTeamSize());

        // Register item info with teams
        RegisterShopInfo();

        ClearWaterMarkers();

        // Spawn game entities for each world entity that requires one
        WorldEntList &vWorldEnts(GetWorldEntityList());
        for (WorldEntList_it it(vWorldEnts.begin()), itEnd(vWorldEnts.end()); it != itEnd; ++it)
        {
            if (*it == INVALID_POOL_HANDLE)
                continue;

            CWorldEntity *pWorldEntity(GetWorldPointer()->GetEntityByHandle(*it));
            if (pWorldEntity == NULL)
                continue;

            if (pWorldEntity->GetType() == _T("Prop_Cliff"))
            {
                //SpawnCliff(pWorldEntity);
                continue;
            }

            if (pWorldEntity->GetType() == _T("Prop_Cliff2"))
            {
                //SpawnCliff(pWorldEntity);
                continue;
            }

            if (pWorldEntity->GetType() == _T("Prop_Water"))
            {
                SpawnWater(pWorldEntity);
                continue;
            }

            if (pWorldEntity->GetType() == _T("Prop_Static"))
            {
                SpawnStaticProp(pWorldEntity);
                continue;
            }

            if (pWorldEntity->GetPropertyBool(_CTS("dormant")))
            {
                Precache(pWorldEntity->GetType(), PRECACHE_ALL);
                continue;
            }

            IGameEntity* pNewEnt(m_pServerEntityDirectory->Allocate(pWorldEntity->GetType()));
            if (pNewEnt == NULL)
            {
                Console.Err << _T("Failed to allocate a game entity for world entity #") + XtoA(pWorldEntity->GetIndex()) << _T(" type: ") << pWorldEntity->GetType() << newl;
                continue;
            }

            pWorldEntity->SetGameIndex(pNewEnt->GetIndex());
            pNewEnt->ApplyWorldEntity(*pWorldEntity);
            Precache(pNewEnt->GetType(), PRECACHE_ALL);
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
        }

        m_pServerEntityDirectory->Spawn();

        // Validate the teams
        if (!GetTeam(TEAM_1)->IsValid())
            Console.Err << _T("Team 1 is invalid") << newl;
        if (!GetTeam(TEAM_2)->IsValid())
            Console.Err << _T("Team 2 is invalid") << newl;

        m_uiMatchCreationTime = GetGameTime();
        
        if(m_pHostServer->IsArrangedMatch())
            SetGamePhase(GAME_PHASE_WAITING_FOR_PLAYERS, sv_arrangedMatchWaitTime, m_uiMatchCreationTime);
        else
            SetGamePhase(GAME_PHASE_WAITING_FOR_PLAYERS);

        Console << _T("LoadWorld: IsArrangedMatch() == ") << m_pHostServer->IsArrangedMatch() << _T(", phase times: ") << GetPhaseStartTime() << SPACE << GetPhaseEndTime() << SPACE << GetPhaseDuration() << newl;

        m_pServerEntityDirectory->WriteBitEntityMap(m_pHostServer->GetStateBlock(STATE_BLOCK_BIT));

        m_vNeutralCamps.clear();
        m_vKongors.clear();

        Game.GetEntities(m_vNeutralCamps, Entity_NeutralCampController);
        Game.GetEntities(m_vKongors, Entity_BossController);

        // Restore thread priority
        if (m_pHostServer->HasManager())
            K2System.SetPriority(iOldPriority);

        pGameInfo->ExecuteActionScript(ACTION_SCRIPT_LOBBY_START, pGameInfo, NULL, NULL, V3_ZERO);

        GetStateString(STATE_STRING_RESOURCES).Validate(); // DEBUG

        return true;
    }
    catch (CException &ex)
    {
        GetWorldPointer()->Free();

        // Restore thread priority
        if (m_pHostServer->HasManager())
            K2System.SetPriority(iOldPriority);

        ex.Process(_T("CGameServer::LoadWorld() - "), THROW);
        return false;
    }
}


/*====================
  CGameServer::UnloadWorld
  ====================*/
void    CGameServer::UnloadWorld()
{
    m_pServerEntityDirectory->Clear();

    SetGameInfo(NULL);
    SetWorldPointer(NULL);
}


/*====================
  CGameServer::StartReplay
  ====================*/
bool    CGameServer::StartReplay(const tstring &sFilename)
{
    if (!ReplayManager.StartPlayback(sFilename))
        return false;

    tstring sGameSettings(_T("map:") + ReplayManager.GetWorldName());

    return m_pHostServer->StartGame(TSNULL, sGameSettings); // FIXME: Game mode
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
    if (sPath.empty())
        return INVALID_RESOURCE;

    ResHandle hHandle(g_ResourceManager.Register(sPath, RES_MODEL, RES_MODEL_SERVER));
    if (hHandle == INVALID_RESOURCE)
        return INVALID_RESOURCE;

    //NetworkResourceManager.GetNetIndex(hHandle); // Register with NetworkResourceManager

    return hHandle;
}


/*====================
  CGameServer::RegisterEffect
  ====================*/
ResHandle   CGameServer::RegisterEffect(const tstring &sPath)
{
    if (sPath.empty())
        return INVALID_RESOURCE;

    ResHandle hHandle(g_ResourceManager.Register(sPath, RES_EFFECT, RES_EFFECT_IGNORE_ALL));
    if (hHandle == INVALID_RESOURCE)
        return INVALID_RESOURCE;

    NetworkResourceManager.GetNetIndex(hHandle); // Register with NetworkResourceManager

    return hHandle;
}


/*====================
  CGameServer::RegisterIcon
  ====================*/
ResHandle   CGameServer::RegisterIcon(const tstring &sPath)
{
    if (sPath.empty())
        return INVALID_RESOURCE;

    ResHandle hHandle(g_ResourceManager.Register(K2_NEW(global,   CTexture)(sPath, TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE, RES_TEXTURE_IGNORE_ALL));
    if (hHandle == INVALID_RESOURCE)
        return INVALID_RESOURCE;

    //NetworkResourceManager.GetNetIndex(hHandle); // Register with NetworkResourceManager

    return hHandle;
}


/*====================
  CGameServer::GetClientNumFromAccountID
  ====================*/
int     CGameServer::GetClientNumFromAccountID(int iAccountID)
{
    return m_pHostServer->GetClientNumber(iAccountID);
}


/*====================
  CGameServer::GetClientNumFromName
  ====================*/
int     CGameServer::GetClientNumFromName(const tstring &sName)
{
    PlayerMap_it itClient(m_mapClients.begin());

    for (; itClient != m_mapClients.end(); itClient++)
        if (itClient->second->GetName() == sName)
            return itClient->first;

    return -1;
}


/*====================
  CGameServer::SendMessage
  ====================*/
void    CGameServer::SendMessage(const tstring &sMsg, int iClientNum)
{
    CBufferDynamic buffer;

    buffer << GAME_CMD_MESSAGE << TStringToUTF8(sMsg) << byte(0);

    if (iClientNum == -1)
        BroadcastGameData(buffer, true);
    else
        SendGameData(iClientNum, buffer, true);
    
    Console.Server << sMsg << newl;
}


/*====================
  CGameServer::BroadcastGameData
  ====================*/
void    CGameServer::BroadcastGameData(const IBuffer &buffer, bool bReliable, int iExcludeClient, uint uiDelay)
{
    if (uiDelay > 0)
    {
        SDelayedMessage cMessage;
        
        cMessage.cBuffer.Resize(buffer.GetLength());
        cMessage.cBuffer.Write(buffer.Get(), buffer.GetLength());
        
        cMessage.bReliable = bReliable;
        cMessage.uiSendTime = Game.GetGameTime() + uiDelay;
        
        cMessage.uiTargetClient = -1;

        m_vDelayedMessages.push_back(cMessage);

        return;
    }

    if (ReplayManager.IsRecording())
    {

#ifdef K2_CLIENT
        CClientConnection *pClient(m_pHostServer->GetClient());
        if (pClient != NULL && pClient->GetClientNum() != iExcludeClient)
        {
            buffer.Rewind();
            ReplayManager.WriteGameData(pClient->GetClientNum(), buffer, bReliable);
        }
#else
        ClientMap &mapClients(m_pHostServer->GetClientMap());

        for (ClientMap_it it(mapClients.begin()); it != mapClients.end(); ++it)
        {
            if (it->first == iExcludeClient)
                continue;

            CPlayer *pPlayer(GetPlayer(it->first));

            if (pPlayer == NULL || pPlayer->GetTeam() == TEAM_SPECTATOR)
                continue;

            buffer.Rewind();
            ReplayManager.WriteGameData(it->first, buffer, bReliable);
        }
#endif
        // Replay spectator stream
        buffer.Rewind();
        ReplayManager.WriteGameData(-1, buffer, bReliable);
    }

    m_pHostServer->BroadcastGameData(buffer, bReliable, iExcludeClient);
}


/*====================
  CGameServer::BroadcastGameDataToTeam
  ====================*/
void    CGameServer::BroadcastGameDataToTeam(int iTeam, const IBuffer &buffer, bool bReliable, int iExcludeClient)
{
#ifdef K2_CLIENT
    CClientConnection *pClient(m_pHostServer->GetClient());
    if (pClient != NULL && pClient->GetClientNum() != iExcludeClient)
    {
        CPlayer *pPlayer(GetPlayer(pClient->GetClientNum()));

        if (pPlayer != NULL && pPlayer->GetTeam() == iTeam)
        {
            buffer.Rewind();

            if (ReplayManager.IsRecording())
            {
                ReplayManager.WriteGameData(pClient->GetClientNum(), buffer, bReliable);
                buffer.Rewind();
            }

            pClient->SendGameData(buffer, bReliable);
        }
    }
#else
    ClientMap &mapClients(m_pHostServer->GetClientMap());

    for (ClientMap_it it(mapClients.begin()); it != mapClients.end(); ++it)
    {
        if (it->first == iExcludeClient)
            continue;

        CPlayer *pPlayer(GetPlayer(it->first));

        if (pPlayer == NULL || pPlayer->GetTeam() != iTeam)
            continue;

        buffer.Rewind();

        if (ReplayManager.IsRecording())
        {
            ReplayManager.WriteGameData(it->first, buffer, bReliable);
            buffer.Rewind();
        }

        it->second->SendGameData(buffer, bReliable);
    }
#endif

    // Replay spectator stream
    if (iTeam == TEAM_SPECTATOR)
    {
        buffer.Rewind();
        ReplayManager.WriteGameData(-1, buffer, bReliable);
    }
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
  CGameServer::SendGameData
  ====================*/
void    CGameServer::SendGameData(int iClient, const IBuffer &buffer, bool bReliable, uint uiDelay)
{
    if (uiDelay > 0)
    {
        SDelayedMessage cMessage;
        
        cMessage.cBuffer.Resize(buffer.GetLength());
        cMessage.cBuffer.Write(buffer.Get(), buffer.GetLength());
        
        cMessage.bReliable = bReliable;
        cMessage.uiSendTime = Game.GetGameTime() + uiDelay;
        
        cMessage.uiTargetClient = iClient;

        m_vDelayedMessages.push_back(cMessage);

        return;
    }

    if (ReplayManager.IsRecording())
    {
        buffer.Rewind();
        ReplayManager.WriteGameData(iClient, buffer, bReliable);
    }

    m_pHostServer->SendGameData(iClient, buffer, bReliable);
}


/*====================
  CGameServer::SendReliablePacket
  ====================*/
void    CGameServer::SendReliablePacket(int iClient, const IBuffer &buffer)
{
    CClientConnection *pClient(m_pHostServer->GetClient(iClient));
    if (pClient == NULL)
    {
        Console.Warn << _T("CHostServer::SendReliablePacket() - Invalid client: ") << iClient << newl;
        return;
    }

    if (ReplayManager.IsRecording())
    {
        buffer.Rewind();
        ReplayManager.WriteGameData(iClient, buffer, true);
    }

    buffer.Rewind();
    
    CPacket pkt;
    pkt << NETCMD_SERVER_GAME_DATA << buffer;
    pClient->SendReliablePacket(pkt);
}


/*====================
  CGameServer::PrecacheEntity
  ====================*/
void    CGameServer::PrecacheEntity(const tstring &sName)
{
    EntityRegistry.ServerPrecache(EntityRegistry.LookupID(sName), PRECACHE_ALL);
}


/*====================
  CGameServer::PrecacheEntities
  ====================*/
void    CGameServer::PrecacheEntities(bool bHeroes)
{
    IModalDialog::SetDisplay(LOADING_DISPLAY_INTERFACE);
    IModalDialog::Show();

    RegisterGameMechanics(_T("/base.gamemechanics"));
    if (!FetchGameMechanics())
        Console.Err << _T("Missing game mechanics!") << newl;

    if (ReplayManager.IsPlaying())
    {
        ReplayManager.UpdateNetIndexes();
    }
    else
    {
        // Load dynamic entity definitions
        tsvector vFileList;
        FileManager.GetFileList(_T("/"), _T("*.entity"), true, vFileList);
        for (tsvector_it it(vFileList.begin()); it != vFileList.end(); ++it)
        {
            g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(*it, RES_ENTITY_DEF));
            IModalDialog::SetProgress(float(it - vFileList.begin()) / (vFileList.size() * 2.0f));
            IModalDialog::Update();
        }
    }

    m_pHostServer->AddStateBlock(STATE_BLOCK_ENTITY_TYPES);
    CStateBlock &blockTypes(m_pHostServer->GetStateBlock(STATE_BLOCK_ENTITY_TYPES));
    EntityRegistry.WriteDynamicEntities(blockTypes.GetBuffer());
    blockTypes.Modify();

    if (!sv_precacheEntities)
        return;

    tsvector vPrecacheList;

    vPrecacheList.push_back(_T("Game_Rules"));
    vPrecacheList.push_back(_T("Entity_Chest"));

    if (bHeroes)
    {
        vector<ushort> vHeroes;
        EntityRegistry.GetHeroList(_T("Legion"), vHeroes, ATTRIBUTE_AGILITY);
        EntityRegistry.GetHeroList(_T("Hellbourne"), vHeroes, ATTRIBUTE_AGILITY);
        EntityRegistry.GetHeroList(_T("Legion"), vHeroes, ATTRIBUTE_INTELLIGENCE);
        EntityRegistry.GetHeroList(_T("Hellbourne"), vHeroes, ATTRIBUTE_INTELLIGENCE);
        EntityRegistry.GetHeroList(_T("Legion"), vHeroes, ATTRIBUTE_STRENGTH);
        EntityRegistry.GetHeroList(_T("Hellbourne"), vHeroes, ATTRIBUTE_STRENGTH);

        for (vector<ushort>::iterator it(vHeroes.begin()), itEnd(vHeroes.end()); it != itEnd; ++it)
            vPrecacheList.push_back(EntityRegistry.LookupName(*it));
    }

    // Shop items
    vector<ushort> vShops;
    EntityRegistry.GetShopList(vShops);

    for (vector<ushort>::iterator it(vShops.begin()), itEnd(vShops.end()); it != itEnd; ++it)
    {
        CShopDefinition *pShop(EntityRegistry.GetDefinition<CShopDefinition>(*it));
        if (pShop == NULL)
            continue;

        const tsvector &vsItems(pShop->GetItems());
        for (tsvector_cit cit(vsItems.begin()), citEnd(vsItems.end()); cit != citEnd; ++cit)
            vPrecacheList.push_back(*cit);
    }

    // Powerups
    for (uint uiPowerup(0); uiPowerup < g_powerups.GetSize(); ++uiPowerup)
        vPrecacheList.push_back(g_powerups[uiPowerup]);

    // Critters
    for (uint uiCritter(0); uiCritter < g_critters.GetSize(); ++uiCritter)
        vPrecacheList.push_back(g_critters[uiCritter]);

    vPrecacheList.push_back(g_creepTeam1Melee);
    vPrecacheList.push_back(g_creepTeam1Ranged);
    vPrecacheList.push_back(g_creepTeam1Siege);
    vPrecacheList.push_back(g_creepTeam2Melee);
    vPrecacheList.push_back(g_creepTeam2Ranged);
    vPrecacheList.push_back(g_creepTeam2Siege);
    vPrecacheList.push_back(g_waypoint);
    
    std::unique(vPrecacheList.begin(), vPrecacheList.end());

    for (tsvector_it it(vPrecacheList.begin()); it != vPrecacheList.end(); ++it)
    {
        PrecacheEntity(*it);
        IModalDialog::SetProgress(0.5f + float(it - vPrecacheList.begin()) / (vPrecacheList.size() * 2.0f));
        IModalDialog::Update();
    }

    EntityRegistry.PrecacheScripts();

    IModalDialog::Hide();
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
  CGameServer::StateBlockChanged
  ====================*/
void    CGameServer::StateBlockChanged(uint uiID, const IBuffer &buffer)
{
    ReplayManager.WriteStateBlock(uiID, buffer);
}


/*====================
  CGameServer::GetStateString
  ====================*/
CStateString&   CGameServer::GetStateString(uint uiID)
{
    return m_pHostServer->GetStateString(uiID);
}


/*====================
  CGameServer::GetStateBlock
  ====================*/
CStateBlock&    CGameServer::GetStateBlock(uint uiID)
{
    return m_pHostServer->GetStateBlock(uiID);
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
uint    CGameServer::GetServerTime() const
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
CTeamInfo*  CGameServer::AddTeam(const tstring &sName, const CVec4f &v4Color, uint uiTeamID)
{
    CTeamInfo *pNewTeam(static_cast<CTeamInfo *>(m_pServerEntityDirectory->Allocate(Info_Team)));
    if (pNewTeam == NULL)
    {
        Console.Err << _T("CGameServer::AddTeam() - Failed to allocate new CTeamInfo for team: ") << sName << newl;
        return NULL;
    }

    pNewTeam->Initialize();
    pNewTeam->SetTeamID(uiTeamID);
    pNewTeam->SetName(sName);
    pNewTeam->SetColor(v4Color);

    SetTeam(pNewTeam->GetTeamID(), pNewTeam);

    return pNewTeam;
}


/*====================
  CGameServer::GetServerStatus
  ====================*/
tstring     CGameServer::GetServerStatus()
{
    int iActiveClients(0);
    int iDisconnects(0);

    for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
    {
        if (it->second->IsDisconnected())
            ++iDisconnects;
        else
            ++iActiveClients;
    }

    tstring sRet;

    sRet += _T("Server Status:");
    sRet += _T(" Map") + ParenStr(GetWorldPointer()->GetName());
    sRet += _T(" Timestamp") + ParenStr(XtoA(GetGameTime()));
    sRet += _T(" Active Clients") + ParenStr(XtoA(iActiveClients));
    sRet += _T(" Disconnects") + ParenStr(XtoA(iDisconnects));
    sRet += _T(" Entities") + ParenStr(XtoA(m_pServerEntityDirectory->GetNumEntities()));
    sRet += _T(" Snapshots") + ParenStr(XtoA(CEntitySnapshot::GetEntitySnapshotPool()->GetNumAllocated()));
    sRet += _T(" Phase") + ParenStr(XtoA(GetGamePhase()));

    uint uiMatchTime(GetMatchTime());
    
    if (uiMatchTime != INVALID_TIME && uiMatchTime != 0)
    {
        uint uiHours(uiMatchTime / MS_PER_HR);
        uint uiMinutes((uiMatchTime % MS_PER_HR) / MS_PER_MIN);
        uint uiSeconds((uiMatchTime % MS_PER_MIN) / MS_PER_SEC);
        tstring sMatchTime(XtoA(uiHours, FMT_PADZERO, 2) + _T(":") + XtoA(uiMinutes, FMT_PADZERO, 2) + _T(":") + XtoA(uiSeconds, FMT_PADZERO, 2));
        
        sRet += _T(" Match Time") + ParenStr(sMatchTime);
    }

    return sRet;
}


/*====================
  CGameServer::UpdateHeroList
  ====================*/
void    CGameServer::UpdateHeroList()
{
    for (ushort unBlock(STATE_BLOCK_FIRST_HERO_GROUP); unBlock <= STATE_BLOCK_LAST_HERO_GROUP; ++unBlock)
    {
        CStateBlock &block(m_pHostServer->GetStateBlock(unBlock));
        IBuffer &buffer(block.GetBuffer());
        buffer.Clear();
        HeroList &vHeroList(m_vHeroLists[unBlock - STATE_BLOCK_FIRST_HERO_GROUP]);
        for (HeroList_it it(vHeroList.begin()); it != vHeroList.end(); ++it)
            buffer << ushort(it - vHeroList.begin()) << it->first << byte(it->second);

        block.Modify();
        
        for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
            m_pHostServer->SendStateBlock(it->second->GetClientNumber(), unBlock);
    }
}


/*====================
  CGameServer::GetAvailableHeroList
  ====================*/
void    CGameServer::GetAvailableHeroList(CPlayer *pPlayer, vector<ushort> &vHeroes)
{
    if (pPlayer == NULL || pPlayer->GetTeam() < TEAM_1 || pPlayer->GetTeam() > TEAM_2)
        return;

    if (GetGameMode() == GAME_MODE_SINGLE_DRAFT)
    {
        HeroList &vHeroList(m_vHeroLists[pPlayer->GetPlayerIndex()]);
        for (HeroList_it it(vHeroList.begin()); it != vHeroList.end(); ++it)
        {
            if (it->second == pPlayer->GetTeam() || it->second == HERO_LIST_AVAILABLE_ALL)
                vHeroes.push_back(it->first);
        }
    }
    else if (GetGameMode() == GAME_MODE_RANDOM_DRAFT || GetGameMode() == GAME_MODE_CAPTAINS_DRAFT || GetGameMode() == GAME_MODE_BANNING_DRAFT)
    {
        HeroList &vHeroList(m_vHeroLists[0]);
        for (HeroList_it it(vHeroList.begin()); it != vHeroList.end(); ++it)
        {
            if (it->second == pPlayer->GetTeam() || it->second == HERO_LIST_AVAILABLE_ALL)
                vHeroes.push_back(it->first);
        }
    }
    else
    {
        for (uint ui(0); ui < 6; ++ui)
        {
            if (!HasGameOptions(GAME_OPTION_ALL_HEROES) && 
                GetGameMode() != GAME_MODE_BANNING_PICK && 
                ((pPlayer->GetTeam() == TEAM_1 && (ui & BIT(0))) || (pPlayer->GetTeam() == TEAM_2 && !(ui & BIT(0)))))
                continue;

            HeroList &vHeroList(m_vHeroLists[ui]);
            for (HeroList_it it(vHeroList.begin()); it != vHeroList.end(); ++it)
            {
                if (it->second == pPlayer->GetTeam() || it->second == HERO_LIST_AVAILABLE_ALL)
                    vHeroes.push_back(it->first);
            }
        }
    }
}


/*====================
  CGameServer::SelectHero
  ====================*/
bool    CGameServer::SelectHero(int iClientNumber, ushort unHero, bool bPotentialHero)
{
    CPlayer *pPlayer(GetPlayerFromClientNumber(iClientNumber));
    if (pPlayer == NULL)
        return false;

    if (HasGameOptions(GAME_OPTION_FORCE_RANDOM))
        return false;

    if (bPotentialHero)
    {
        if (GetGamePhase() < GAME_PHASE_HERO_BAN)
            return false;
    }
    else
    {
        if (GetGamePhase() < GAME_PHASE_HERO_SELECT)
            return false;

        if (!pPlayer->HasFlags(PLAYER_FLAG_CAN_PICK))
            return false;
    }

    int iTeam(pPlayer->GetTeam());
    if (iTeam == 0)
        return false;
    CTeamInfo *pTeam(GetTeam(iTeam));
    if (pTeam == NULL)
        return false;

    vector<ushort> vAvailableHeroes;
    GetAvailableHeroList(pPlayer, vAvailableHeroes);

    const CDynamicEntityAllocator *pAllocator(EntityRegistry.GetDynamicAllocator(unHero));
    if (pAllocator == NULL)
        return false;

    vector<ushort>::iterator itHero(std::find(vAvailableHeroes.begin(), vAvailableHeroes.end(), unHero));
    if (itHero == vAvailableHeroes.end())
        return false;

    if (pPlayer->CanSelectHero(unHero))
    {           
        if (!bPotentialHero)
        {
            pPlayer->SelectHero(unHero);

            if (!HasGameOptions(GAME_OPTION_DUPLICATE_HEROES))
                SetHeroStatus(unHero, HERO_LIST_PICKED);

            m_GameLog.WritePlayer(GAME_LOG_PLAYER_SELECT, pPlayer);

            CBufferFixed<7> buffer;
            buffer << GAME_CMD_PICK_HERO_MESSAGE << iClientNumber << unHero;
            BroadcastGameData(buffer, true);

            if (GetGamePhase() >= GAME_PHASE_PRE_MATCH && GetGamePhase() <= GAME_PHASE_ACTIVE)
                return pPlayer->SpawnHero();

            // Only clear the potential hero for this player, not all of them
            ClearPotentialHero(pPlayer, unHero);
        }
        else
        {
            pPlayer->SelectPotentialHero(unHero);

            CBufferFixed<7> buffer;
            buffer << GAME_CMD_PICK_POTENTIAL_HERO_MESSAGE << iClientNumber << unHero;
            BroadcastGameDataToTeam(iTeam, buffer, true);
            BroadcastGameDataToTeam(TEAM_SPECTATOR, buffer, true);
        }

        return true;
    }

    return false;
}


/*====================
  CGameServer::SelectRandomHero
  ====================*/
void    CGameServer::SelectRandomHero(int iClientNumber)
{
    CPlayer *pPlayer(GetPlayerFromClientNumber(iClientNumber));
    if (pPlayer == NULL)
        return;
    
    if (GetGamePhase() < GAME_PHASE_HERO_SELECT)
        return;

    if (!pPlayer->HasFlags(PLAYER_FLAG_CAN_PICK))
        return;

    int iTeam(pPlayer->GetTeam());
    if (iTeam == 0)
        return;
    CTeamInfo *pTeam(GetTeam(iTeam));
    if (pTeam == NULL)
        return;

    vector<ushort> vAvailableHeroes;
    GetAvailableHeroList(pPlayer, vAvailableHeroes);
    if (vAvailableHeroes.empty())
        return;

    ushort unHero(vAvailableHeroes[M_Randnum(0u, INT_SIZE(vAvailableHeroes.size() - 1))]);
    const CDynamicEntityAllocator *pAllocator(EntityRegistry.GetDynamicAllocator(unHero));
    if (pAllocator == NULL)
        return;

    if (!pPlayer->CanSelectHero(unHero))
        return;

    pPlayer->SelectHero(unHero);

    if (!HasGameOptions(GAME_OPTION_DUPLICATE_HEROES))
        SetHeroStatus(unHero, HERO_LIST_PICKED);

    if (!pPlayer->HasFlags(PLAYER_FLAG_HAS_REPICKED))
    {
        pPlayer->GiveGold(GetGameInfo()->GetRandomBonus(), NULL);
        pPlayer->AdjustStat(PLAYER_STAT_STARTING_GOLD, GetGameInfo()->GetRandomBonus());
    }

    m_GameLog.WritePlayer(GAME_LOG_PLAYER_RANDOM, pPlayer);
    
    CBufferFixed<7> buffer;
    buffer << GAME_CMD_RANDOM_HERO_MESSAGE << iClientNumber << unHero;
    BroadcastGameData(buffer, true);

    if (GetGamePhase() >= GAME_PHASE_PRE_MATCH && GetGamePhase() <= GAME_PHASE_ACTIVE)
        pPlayer->SpawnHero();
}


/*====================
  CGameServer::SelectPotentialHeroOrRandomHero
  ====================*/
void    CGameServer::SelectPotentialHeroOrRandomHero(int iClientNumber)
{
    CPlayer *pPlayer(GetPlayerFromClientNumber(iClientNumber));
    if (pPlayer == NULL)
        return;
    
    // try to select the potential hero.
    if (!pPlayer->HasSelectedHero() && pPlayer->HasPotentialHero())
        SelectHero(iClientNumber, pPlayer->GetPotentialHero(), false);
        
    // if the potential hero could not be selected, then select a random hero.
    if (!pPlayer->HasSelectedHero())
        SelectRandomHero(iClientNumber);        
}


/*====================
  CGameServer::ClearPotentialHero
  ====================*/
void    CGameServer::ClearPotentialHero(CPlayer *pPlayer, ushort unHero)
{
    if (pPlayer == NULL)
        return;

    if (unHero == INVALID_ENT_TYPE)
        return;
    
    if (pPlayer->GetPotentialHero() == unHero)
    {
        pPlayer->SelectPotentialHero(INVALID_ENT_TYPE);

        CBufferFixed<7> buffer;
        buffer << GAME_CMD_PICK_POTENTIAL_HERO_MESSAGE << pPlayer->GetClientNumber() << INVALID_ENT_TYPE;
        GameServer.BroadcastGameDataToTeam(pPlayer->GetTeam(), buffer, true);
        GameServer.BroadcastGameDataToTeam(TEAM_SPECTATOR, buffer, true);
    }
}


/*====================
  CGameServer::SetHeroStatus
  ====================*/
void    CGameServer::SetHeroStatus(ushort unHeroTypeID, byte yStatus)
{
    bool bNeedsUpdate(false);

    for (uint ui(0); ui < NUM_HERO_LISTS; ++ui)
    {
        HeroList &vHeroList(m_vHeroLists[ui]);
        for (HeroList_it it(vHeroList.begin()); it != vHeroList.end(); ++it)
        {
            if (it->first == unHeroTypeID && it->second != yStatus)
            {
                it->second = yStatus;
                bNeedsUpdate = true;
            }
        }
    }

    if (bNeedsUpdate)
        UpdateHeroList();
}


/*====================
  CGameServer::GetHeroStatus
  ====================*/
byte    CGameServer::GetHeroStatus(ushort unHeroTypeID)
{
    for (uint ui(0); ui < NUM_HERO_LISTS; ++ui)
    {
        HeroList &vHeroList(m_vHeroLists[ui]);
        for (HeroList_it it(vHeroList.begin()); it != vHeroList.end(); ++it)
        {
            if (it->first == unHeroTypeID)
                return it->second;
        }
    }

    return HERO_LIST_UNKNOWN;
}


/*====================
  CGameServer::GetBannedHeroes
  ====================*/
void    CGameServer::GetBannedHeroes(vector<ushort>& vHeroes)
{
    for (uint ui(0); ui < NUM_HERO_LISTS; ++ui)
    {
        HeroList &vHeroList(m_vHeroLists[ui]);
        for (HeroList_it it(vHeroList.begin()); it != vHeroList.end(); ++it)
        {
            if (it->second == HERO_LIST_BANNED)
                vHeroes.push_back(it->first);
        }
    }
}


/*====================
  CGameServer::GetBannedHeroes
  ====================*/
tstring         CGameServer::GetBannedHeroesStr()
{
    tstring sResult;

    // get the banned heroes.
    vector<ushort> vHeroes;
    GetBannedHeroes(vHeroes);
    if (vHeroes.empty())
        return sResult;

    // add the first hero to the string.
    sResult.append(EntityRegistry.LookupName(vHeroes[0]));

    // add the rest of the heroes to the string.
    for (size_t i = 1; i < vHeroes.size(); ++i)
    {
        sResult.append(_T(","));
        sResult.append(EntityRegistry.LookupName(vHeroes[i]));
    }

    return sResult;
}


/*====================
  CGameServer::RemoveHero
  ====================*/
bool    CGameServer::RemoveHero(int iClientNumber)
{
    CPlayer *pPlayer(GetPlayerFromClientNumber(iClientNumber));
    if (pPlayer == NULL)
        return false;

    IHeroEntity *pHero(pPlayer->GetHero());
    if (pHero == NULL)
        return false;

    if (GetGameMode() == GAME_MODE_CAPTAINS_DRAFT || GetGameMode() == GAME_MODE_CAPTAINS_MODE)
        SetHeroStatus(pHero->GetType(), pPlayer->GetTeam());
    else
        SetHeroStatus(pHero->GetType(), HERO_LIST_AVAILABLE_ALL);

    DeleteEntity(pHero);
    pPlayer->SelectHero(INVALID_ENT_TYPE);
    pPlayer->AssignHero(NULL);
    pPlayer->RemoveFlags(PLAYER_FLAG_READY);
    pPlayer->ClearAllSwapRequests();

    if (GetGamePhase() > GAME_PHASE_HERO_SELECT)
        pPlayer->SetFlags(PLAYER_FLAG_CAN_PICK);

    return true;
}


/*====================
  CGameServer::ResetPicks
  ====================*/
void    CGameServer::ResetPicks(int iClientNumber)
{
    RemoveHero(iClientNumber);

    CPlayer *pPlayer(GetPlayerFromClientNumber(iClientNumber));
    if (pPlayer == NULL)
        return;

    if (GetGameMode() == GAME_MODE_CAPTAINS_DRAFT || GetGameMode() == GAME_MODE_CAPTAINS_MODE)
        SetHeroStatus(pPlayer->GetSelectedHero(), HERO_LIST_NOT_AVAILABLE);
    else
        SetHeroStatus(pPlayer->GetSelectedHero(), HERO_LIST_AVAILABLE_ALL);

    pPlayer->SelectHero(INVALID_ENT_TYPE);
    pPlayer->AssignHero(NULL);
    pPlayer->RemoveFlags(PLAYER_FLAG_HAS_REPICKED | PLAYER_FLAG_READY);
    pPlayer->ClearAllSwapRequests();
}


/*====================
  CGameServer::GetRandomHeroFromPool
  ====================*/
ushort  CGameServer::GetRandomHeroFromPool()
{
    vector<ushort> vHeroes;

    for (uint ui(0); ui < NUM_HERO_LISTS; ++ui)
    {
        HeroList &vHeroList(m_vHeroLists[ui]);
        for (HeroList_it it(vHeroList.begin()); it != vHeroList.end(); ++it)
        {
            if (it->second != HERO_LIST_BANNED)
                vHeroes.push_back(it->first);
        }
    }

    if (vHeroes.empty())
        return INVALID_ENT_TYPE;

    return vHeroes[M_Randnum(0u, INT_SIZE(vHeroes.size()) - 1)];
}


/*====================
  CGameServer::HasMegaCreeps
  ====================*/
bool    CGameServer::HasMegaCreeps(uint uiTeam)
{
    // Get a list of all creep spawners
    static vector<uint> vSpawners;
    vSpawners.clear();
    Game.GetEntities(vSpawners, Entity_CreepSpawner);
    uivector_it itEnd(vSpawners.end());

    // Check for mega creeps
    for (uivector_it it(vSpawners.begin()); it != itEnd; ++it)
    {
        CEntityCreepSpawner *pSpawner(Game.GetEntity(*it)->GetAs<CEntityCreepSpawner>());
        if (pSpawner == NULL)
            continue;

        CTeamInfo *pTeam(GetTeam(pSpawner->GetTeam()));
        if (pTeam == NULL || pTeam->GetTeamID() == uiTeam)
            continue;

        IUnitEntity *pTarget1(GetUnitFromUniqueID(pSpawner->GetTargetUID(1)));
        if (pTarget1 != NULL && pTarget1->GetStatus() == ENTITY_STATUS_ACTIVE)
            return false;

        IUnitEntity *pTarget2(GetUnitFromUniqueID(pSpawner->GetTargetUID(2)));
        if (pTarget2 != NULL && pTarget2->GetStatus() == ENTITY_STATUS_ACTIVE)
            return false;
    }

    return true;
}


/*====================
  CGameServer::SpawnCreeps
  ====================*/
void    CGameServer::SpawnCreeps()
{
    PROFILE("CGameServer::SpawnCreeps");

    CTeamInfo *pTeam1(GetTeam(TEAM_1));
    CTeamInfo *pTeam2(GetTeam(TEAM_2));

    if (!pTeam1 || !pTeam2)
        return;

    // Get a list of all creep spawners
    static vector<uint> vSpawners;
    vSpawners.clear();
    Game.GetEntities(vSpawners, Entity_CreepSpawner);
    uivector_it itEnd(vSpawners.end());

    // Check for mega creeps
    bool aMegaCreeps[3] = { false, true, true };
    for (uivector_it it(vSpawners.begin()); it != itEnd; ++it)
    {
        CEntityCreepSpawner *pSpawner(Game.GetEntity(*it)->GetAs<CEntityCreepSpawner>());
        if (pSpawner == NULL)
            continue;

        CTeamInfo *pTeam(GetTeam(pSpawner->GetTeam()));
        if (pTeam == NULL || pTeam->GetTeamID() > 2)
            continue;

        bool bTarget1(true);
        bool bTarget2(true);

        IUnitEntity *pTarget1(GetUnitFromUniqueID(pSpawner->GetTargetUID(1)));
        if (pTarget1 == NULL || pTarget1->GetStatus() != ENTITY_STATUS_ACTIVE)
            bTarget1 = false;
        IUnitEntity *pTarget2(GetUnitFromUniqueID(pSpawner->GetTargetUID(2)));
        if (pTarget2 == NULL || pTarget2->GetStatus() != ENTITY_STATUS_ACTIVE)
            bTarget2 = false;

        if (!bTarget1 && !bTarget2)
            continue;

        aMegaCreeps[pTeam->GetTeamID() ^ 3] = false;
    }

    // Determine upgrade level
    uint uiGlobalCreepUpgradeLevel(MIN(GetMatchTime() / g_creepUpgradeInterval, g_creepMaxUpgrades.GetValue()));

    // Check for an upgrade
    if (uiGlobalCreepUpgradeLevel > m_uiLastCreepUpgradeLevel)
    {
        m_uiLastCreepUpgradeLevel = uiGlobalCreepUpgradeLevel;

        static CBufferFixed<1> buffer;
        buffer.Clear();
        buffer << GAME_CMD_CREEP_UPGRADE_MESSAGE;
        Game.BroadcastGameData(buffer, true);
    }

    uint uiBaseFormationIndex(0);
    for (uint ui(0); ui < g_creepFormationIndexes.GetSize(); ++ui)
    {
        if (m_uiCreepWaveCount >= g_creepFormationIndexes.GetValue(ui))
            uiBaseFormationIndex = ui;
    }

    // Spawn creeps
    for (uivector_it it(vSpawners.begin()); it != itEnd; ++it)
    {
        CEntityCreepSpawner *pSpawner(Game.GetEntity(*it)->GetAs<CEntityCreepSpawner>());
        if (pSpawner == NULL)
            continue;

        CTeamInfo *pTeam(GetTeam(pSpawner->GetTeam()));
        if (pTeam == NULL || (GetWinningTeam() != TEAM_INVALID && pTeam->GetTeamID() != GetWinningTeam()))
            continue;

        bool bMegaCreeps(aMegaCreeps[pTeam->GetTeamID()] || pTeam->GetTeamID() == GetWinningTeam());

        int iMeleeCreepLevel(bMegaCreeps ? 3 : 1);
        int iRangedCreepLevel(bMegaCreeps ? 3 : 1);
        uint uiTeamCreepUpgradeLevel(bMegaCreeps ? g_creepMaxUpgrades : uiGlobalCreepUpgradeLevel);
        
        uint uiFormationIndex(pTeam->GetTeamID() == GetWinningTeam() ? (g_creepFormationIndexes.GetSize() - 1) : uiBaseFormationIndex);

        // Check for upgraded creeps
        if (!aMegaCreeps[pTeam->GetTeamID()])
        {
            IGameEntity *pTarget(Game.GetEntityFromUniqueID(pSpawner->GetTargetUID(3)));
            if (pTarget != NULL)
            {
                CEntityCreepSpawner *pOppositeSpawner(pTarget->GetAs<CEntityCreepSpawner>());
                if (pOppositeSpawner != NULL)
                {
                    if (Game.GetEntityFromUniqueID(pOppositeSpawner->GetTargetUID(1)) == NULL)
                        iMeleeCreepLevel = 2;
                    if (Game.GetEntityFromUniqueID(pOppositeSpawner->GetTargetUID(2)) == NULL)
                        iRangedCreepLevel = 2;
                }
            }
        }
        
        if (pTeam->GetTeamID() == TEAM_1)
        {
            for (uint ui(0); ui < g_creepMeleeCount.GetValue(uiFormationIndex); ++ui)
                pTeam->SpawnCreep(g_creepTeam1Melee, pSpawner, iMeleeCreepLevel, uiTeamCreepUpgradeLevel);

            for (uint ui(0); ui < g_creepRangedCount.GetValue(uiFormationIndex); ++ui)
                pTeam->SpawnCreep(g_creepTeam1Ranged, pSpawner, iRangedCreepLevel, uiTeamCreepUpgradeLevel);

            if ((m_uiCreepWaveCount > 0 && (m_uiCreepWaveCount % g_creepSiegeInterval == 0)) || pTeam->GetTeamID() == GetWinningTeam())
            {
                for (uint ui(0); ui < g_creepSiegeCount.GetValue(uiFormationIndex); ++ui)
                    pTeam->SpawnCreep(g_creepTeam1Siege, pSpawner, 1, uiTeamCreepUpgradeLevel);
            }
        }
        else if (pTeam->GetTeamID() == TEAM_2)
        {
            for (uint ui(0); ui < g_creepMeleeCount.GetValue(uiFormationIndex); ++ui)
                pTeam->SpawnCreep(g_creepTeam2Melee, pSpawner, iMeleeCreepLevel, uiTeamCreepUpgradeLevel);

            for (uint ui(0); ui < g_creepRangedCount.GetValue(uiFormationIndex); ++ui)
                pTeam->SpawnCreep(g_creepTeam2Ranged, pSpawner, iRangedCreepLevel, uiTeamCreepUpgradeLevel);

            if ((m_uiCreepWaveCount > 0 && (m_uiCreepWaveCount % g_creepSiegeInterval == 0)) || pTeam->GetTeamID() == GetWinningTeam())
            {
                for (uint ui(0); ui < g_creepSiegeCount.GetValue(uiFormationIndex); ++ui)
                    pTeam->SpawnCreep(g_creepTeam2Siege, pSpawner, 1, uiTeamCreepUpgradeLevel);
            }
        }
    }
}


/*====================
  CGameServer::SpawnPowerup
  ====================*/
void    CGameServer::SpawnPowerup()
{
    if (Game.GetEntityFromUniqueID(m_uiPowerupUID) != NULL)
        return;

    // Get a list of all powerup spawn locations
    static vector<uint> vSpawnPoints;
    vSpawnPoints.clear();
    Game.GetEntities(vSpawnPoints, Entity_PowerupSpawner);

    // Pick spawn location
    if (vSpawnPoints.size() == 0)
        return;

    uint uiRand(M_Randnum(0u, uint(vSpawnPoints.size() - 1))); 

    CEntityPowerupSpawner *pSpawnPoint(Game.GetEntity(vSpawnPoints[uiRand])->GetAs<CEntityPowerupSpawner>());
    if (pSpawnPoint == NULL)
        return;

    // Pick powerup type
    if (g_powerups.GetSize() == 0)
        return;

    uint uiRandType(M_Randnum(0u, g_powerups.GetSize() - 1));

    ushort unType(EntityRegistry.LookupID(g_powerups[uiRandType]));

    // Spawn the powerup
    IGameEntity *pNewEnt(Game.AllocateEntity(unType));
    if (pNewEnt == NULL || !pNewEnt->IsVisual())
    {
        Console.Warn << _T("Failed to spawn powerup: ") << unType << newl;
        return;
    }
    IVisualEntity *pPowerup(pNewEnt->GetAsVisual());

    pPowerup->SetPosition(pSpawnPoint->GetPosition());
    pPowerup->SetAngles(pSpawnPoint->GetAngles());

    if (pPowerup->IsUnit())
        pPowerup->GetAsUnit()->SetLevel(1);

    pPowerup->Spawn();

    m_uiPowerupUID = pPowerup->GetUniqueID();
}


/*====================
  CGameServer::SpawnCritters
  ====================*/
void    CGameServer::SpawnCritters()
{
    PROFILE("CGameServer::SpawnCritters");

    if (g_critters.GetSize() == 0)
        return;

    uint uiCount(0);

    IVisualEntity *pEntity(Game.GetEntityFromName(_T("_critter")));
    while (pEntity != NULL)
    {
        ++uiCount;
        pEntity = Game.GetNextEntityFromName(pEntity);
    }

    // Get a list of all critter spawn locations
    static vector<uint> vSpawnPoints;
    vSpawnPoints.clear();
    Game.GetEntities(vSpawnPoints, Entity_CritterSpawner);

    if (vSpawnPoints.size() == 0)
        return;

    uint uiTries(0);
    while (uiCount < 3 && uiTries < 10)
    {
        ++uiTries;

        // Pick spawn location      
        uint uiRand(M_Randnum(0u, uint(vSpawnPoints.size() - 1))); 

        CEntityCritterSpawner *pSpawnPoint(Game.GetEntity(vSpawnPoints[uiRand])->GetAs<CEntityCritterSpawner>());
        if (pSpawnPoint == NULL)
            continue;

        // Don't respawn if something is near (including corpses)
        static uivector vEntities;
        vEntities.clear();

        Game.GetEntitiesInRadius(vEntities, pSpawnPoint->GetPosition().xy(), g_critterNoRespawnProximity, REGION_UNIT);
        
        uivector_it it(vEntities.begin()), itEnd(vEntities.end());
        for (; it != itEnd; ++it)
        {
            IUnitEntity *pUnit(Game.GetUnitEntity(Game.GetGameIndexFromWorldIndex(*it)));
            if (pUnit == NULL)
                continue;
            if (pUnit->GetStatus() != ENTITY_STATUS_ACTIVE &&
                pUnit->GetStatus() != ENTITY_STATUS_DEAD &&
                pUnit->GetStatus() != ENTITY_STATUS_CORPSE)
                continue;

            break;
        }
        if (it != itEnd)
            continue; // Try a new spawn point if this one is occupied
        
        // Pick critter type
        uint uiRandType(M_Randnum(0u, g_critters.GetSize() - 1));

        ushort unType(EntityRegistry.LookupID(g_critters[uiRandType]));

        // Spawn the critter
        IGameEntity *pNewEnt(Game.AllocateEntity(unType));
        if (pNewEnt == NULL || !pNewEnt->IsUnit())
        {
            if (pNewEnt != NULL)
                DeleteEntity(pNewEnt);

            Console.Warn << _T("Failed to spawn critter: ") << unType << newl;
            continue;
        }
        IUnitEntity *pCritter(pNewEnt->GetAsUnit());

        pCritter->SetName(_T("_critter"));
        pCritter->SetTeam(TEAM_PASSIVE);
        pCritter->SetPosition(pSpawnPoint->GetPosition());
        pCritter->SetAngles(pSpawnPoint->GetAngles());
        pCritter->Spawn();

        ++uiCount;
    }
}


/*====================
  CGameServer::UpdateNavigation
  ====================*/
void    CGameServer::UpdateNavigation()
{
    GAME_PROFILE(_T("CGameServer::UpdateNavigation"));

    GetWorldPointer()->UpdateNavigation();
}


/*====================
  CGameServer::GetVision
  ====================*/
ushort  CGameServer::GetVision(float fX, float fY) const
{
    uint uiX(uint(CLAMP(INT_FLOOR(fX / m_fVisibilityScale), 0, int(GetWorldPointer()->GetTileWidth() / m_uiVisibilitySize) - 1)));
    uint uiY(uint(CLAMP(INT_FLOOR(fY / m_fVisibilityScale), 0, int(GetWorldPointer()->GetTileHeight() / m_uiVisibilitySize) - 1)));

    return m_cVisibilityMap[0].GetVision(uiX, uiY) | (m_cVisibilityMap[1].GetVision(uiX, uiY) << 8);
}


/*====================
  CGameServer::UpdateUnitVisibility
  ====================*/
void    CGameServer::UpdateUnitVisibility(IUnitEntity *pUnit)
{
    if (pUnit == NULL)
        return;

    int iVisibilityTileWidth(GetWorldPointer()->GetTileWidth() / m_uiVisibilitySize);
    int iVisibilityTileHeight(GetWorldPointer()->GetTileHeight() / m_uiVisibilitySize);

    float fX(pUnit->GetPosition().x);
    float fY(pUnit->GetPosition().y);

    uint uiX(uint(CLAMP(INT_FLOOR(fX / m_fVisibilityScale), 0, iVisibilityTileWidth - 1)));
    uint uiY(uint(CLAMP(INT_FLOOR(fY / m_fVisibilityScale), 0, iVisibilityTileHeight - 1)));

    //
    // Team 1
    //
    if (pUnit->GetLastAggression(TEAM_1) != INVALID_TIME && pUnit->GetLastAggression(TEAM_1) + g_unitAggressionSightTime > Game.GetGameTime())
        pUnit->SetVisibilityFlags(VIS_VISION(TEAM_1));

    if (pUnit->GetTeam() == TEAM_1 || pUnit->HasVisibilityFlags(VIS_VISION(TEAM_1)))
        pUnit->SetVisibilityFlags(VIS_SIGHTED(TEAM_1));

    pUnit->SetVisibilityFlags(m_cVisibilityMap[0].GetVision(uiX, uiY));

    //
    // Team 2
    //
    if (pUnit->GetLastAggression(TEAM_2) != INVALID_TIME && pUnit->GetLastAggression(TEAM_2) + g_unitAggressionSightTime > Game.GetGameTime())
        pUnit->SetVisibilityFlags(VIS_VISION(TEAM_2));

    if (pUnit->GetTeam() == TEAM_2 || pUnit->HasVisibilityFlags(VIS_VISION(TEAM_2)))
        pUnit->SetVisibilityFlags(VIS_SIGHTED(TEAM_2));

    pUnit->SetVisibilityFlags(m_cVisibilityMap[1].GetVision(uiX, uiY) << 8);
}


/*====================
  CGameServer::UpdateVisibility
  ====================*/
void    CGameServer::UpdateVisibility()
{
    GAME_PROFILE(_T("CGameServer::UpdateVisibility"));
    
    const UnitList &lUnits(m_pServerEntityDirectory->GetUnitList());
    UnitList_cit itEnd(lUnits.end());

    if (GetGamePhase() == GAME_PHASE_ENDED)
    {
        for (UnitList_cit it(lUnits.begin()); it != itEnd; ++it)
        {
            IUnitEntity *pUnit(*it);

            pUnit->SetVisibilityFlags(VIS_VISION(1) | VIS_VISION(2) | VIS_REVEALED(1) | VIS_REVEALED(2));
        }
    }

    //
    // Build team visibilty maps
    //
    uint uiUpdateMS(g_fogofwarUpdateTime);

    if (m_uiLastVisibilityUpdate == INVALID_TIME || m_uiLastVisibilityUpdate + uiUpdateMS <= Game.GetGameTime())
    {
        m_cVisibilityMap[0].Clear();
        m_cVisibilityMap[1].Clear();

        for (UnitList_cit it(lUnits.begin()); it != itEnd; ++it)
        {
            IUnitEntity *pUnit(*it);

            if ((pUnit->GetStatus() != ENTITY_STATUS_ACTIVE && pUnit->GetStatus() != ENTITY_STATUS_DEAD) ||
                pUnit->GetTeam() < 1 || pUnit->GetTeam() > 2)
                continue;

            // Add our vision to our teams visibility map
            if (pUnit->GetSightRange() > 0.0f)
            {
                int iMap(pUnit->GetTeam() - 1);

                float fX(pUnit->GetPosition().x);
                float fY(pUnit->GetPosition().y);

                int iX(INT_FLOOR(fX / m_fVisibilityScale));
                int iY(INT_FLOOR(fY / m_fVisibilityScale));
                int iRadius(INT_FLOOR(pUnit->GetSightRange() / m_fVisibilityScale));

                iRadius = MIN(iRadius, RASTER_BUFFER_SPAN / 2 - 1);

                CRecti  recRegion(iX - iRadius, iY - iRadius, iX + iRadius + 1, iY + iRadius + 1);

                if (pUnit->GetClearVision())
                {
                    m_cVisRaster.Clear(0, recRegion.GetWidth() * recRegion.GetWidth());
                    m_cVisRaster.DrawFilledCircle(recRegion.GetWidth(), iRadius, iRadius, iRadius);
                }
                else
                {
                    // Get world occluders
                    float fHeight(pUnit->GetPosition().z + g_occlusionHeight);
                        
                    GetWorldPointer()->GetOcclusion(recRegion, m_cOccRaster.GetBuffer(), fHeight);
                    
                    // Occlude sight range
                    m_cOccRaster.DrawOuterFilledCircle(recRegion.GetWidth(), iRadius, iRadius, iRadius);
                    
                    if (g_fogofwarStyle == 0)
                        m_cVisRaster.CalcVisibilty2(recRegion.GetWidth(), m_cOccRaster.GetBuffer());
                    else
                        m_cVisRaster.CalcVisibilty(recRegion.GetWidth(), m_cOccRaster.GetBuffer());
                }

                CRecti  recBuffer(recRegion);

                byte yVision(SIGHTED_BIT);
                CPlayer *pPlayer(pUnit->GetOwnerPlayer());
                if (pPlayer != NULL)
                    yVision |= PLAYER_SIGHTED_BIT << pPlayer->GetTeamIndex();

                if (GetWorldPointer()->ClipRect(recRegion, VIS_TILE_SPACE))
                    m_cVisibilityMap[iMap].AddVision(recRegion, m_cVisRaster.GetBuffer() + recBuffer.GetWidth() * (recRegion.top - recBuffer.top) + (recRegion.left - recBuffer.left), recBuffer.GetWidth(), yVision);
            }
        }

        m_uiLastVisibilityUpdate = Game.GetGameTime() / uiUpdateMS * uiUpdateMS; // Round down to the nearest uiMS (acts like an accumulator)
    }

    //
    // Update reveal and sighted status
    //
    for (UnitList_cit it(lUnits.begin()); it != itEnd; ++it)
    {
        IUnitEntity *pUnit(*it);

        pUnit->RemoveUnitFlags(UNIT_FLAG_REVEALED);

        if ((pUnit->GetStatus() != ENTITY_STATUS_ACTIVE && pUnit->GetStatus() != ENTITY_STATUS_DEAD) ||
            pUnit->GetTeam() < 1 || pUnit->GetTeam() > 2)
            continue;

        // Apply reveal to units in range (no LoS check)
        if (pUnit->GetRevealRange() > 0.0f && pUnit->GetRevealType() != 0)
        {
            static vector<uint> vEntities;
            vEntities.clear();

            float fRange(pUnit->GetRevealRange());

            Game.GetEntitiesInRegion(vEntities, CBBoxf(-fRange, fRange, pUnit->GetPosition()), REGION_UNIT);
            for (uivector_it it(vEntities.begin()); it != vEntities.end(); ++it)
            {
                IGameEntity *pTargetEntity(GetEntityFromWorldIndex(*it));
                if (pTargetEntity == NULL)
                    continue;

                IUnitEntity *pTargetUnit(pTargetEntity->GetAsUnit());
                if (pTargetUnit == NULL)
                    continue;

                if (pTargetUnit->GetTeam() == pUnit->GetTeam())
                    continue;
                
                if (DistanceSq(pUnit->GetPosition().xy(), pTargetUnit->GetPosition().xy()) > SQR(fRange))
                    continue;
    
                if (IsRevealed(pTargetUnit->GetStealthBits(), pUnit->GetRevealType()))
                    pTargetUnit->SetVisibilityFlags(VIS_REVEALED(pUnit->GetTeam()));
            }
        }

        // Reveal from this unit's slaves
        pUnit->SetAlwaysTransmitData(false);
        for (uint ui(INVENTORY_START_ACTIVE); ui <= INVENTORY_END_ACTIVE; ++ui)
        {
            static vector<uint> vEntities;
            vEntities.clear();

            ISlaveEntity *pSlave(pUnit->GetSlave(ui));
            if (pSlave == NULL)
                continue;

            if (pSlave->IsState())
            {
                if (pSlave->GetSighted())
                {
                    IUnitEntity *pInflictor(pSlave->GetAsState()->GetInflictor());
                    if (pInflictor != NULL)
                    {
                        int iTeam(pInflictor->GetTeam());
                        if (iTeam == TEAM_1 || iTeam == TEAM_2)
                            pUnit->SetVisibilityFlags(VIS_VISION(iTeam));
                    }
                }
            }

            if (pSlave->GetAlwaysTransmitData())
                pUnit->SetAlwaysTransmitData(true);

            if (pSlave->GetRevealed())
                pUnit->SetUnitFlags(UNIT_FLAG_REVEALED);
                
            if (pSlave->GetRevealType() == 0)
                continue;

            float fRange(pSlave->GetRevealRange());
            if (fRange <= 0.0f)
                continue;

            Game.GetEntitiesInRegion(vEntities, CBBoxf(-fRange, fRange, pUnit->GetPosition()), REGION_UNIT);
            for (uivector_it it(vEntities.begin()); it != vEntities.end(); ++it)
            {
                IGameEntity *pTargetEntity(GetEntityFromWorldIndex(*it));
                if (pTargetEntity == NULL)
                    continue;

                IUnitEntity *pTargetUnit(pTargetEntity->GetAsUnit());
                if (pTargetUnit == NULL)
                    continue;

                if (pTargetUnit->GetTeam() == pUnit->GetTeam())
                    continue;
                
                if (DistanceSq(pUnit->GetPosition().xy(), pTargetUnit->GetPosition().xy()) > SQR(fRange))
                    continue;
                
                if (IsRevealed(pTargetUnit->GetStealthBits(), pSlave->GetRevealType()))
                    pTargetUnit->SetVisibilityFlags(VIS_REVEALED(pUnit->GetTeam()));
            }
        }

        // Proximity reveal
        if (pUnit->IsStealth() && pUnit->GetMinStealthProximity() > 0.0f)
        {
            static vector<uint> vEntities;
            vEntities.clear();

            float fRange(pUnit->GetMinStealthProximity());
            const CVec3f &v3Position(pUnit->GetPosition());

            CBBoxf bbRegion(CVec3f(v3Position.xy() - CVec2f(fRange, fRange), -FAR_AWAY), CVec3f(v3Position.xy() + CVec2f(fRange, fRange), FAR_AWAY));

            Game.GetEntitiesInRegion(vEntities, bbRegion, REGION_UNIT);
            for (uivector_it it(vEntities.begin()); it != vEntities.end(); ++it)
            {
                IGameEntity *pTargetEntity(GetEntityFromWorldIndex(*it));
                if (pTargetEntity == NULL)
                    continue;

                IUnitEntity *pTargetUnit(pTargetEntity->GetAsUnit());
                if (pTargetUnit == NULL)
                    continue;

                if (pTargetUnit->GetTeam() == pUnit->GetTeam())
                    continue;
                
                if (DistanceSq(pUnit->GetPosition().xy(), pTargetUnit->GetPosition().xy()) > SQR(fRange))
                    continue;
    
                pUnit->SetVisibilityFlags(VIS_REVEALED(pTargetUnit->GetTeam()));
            }
        }
    }

    //
    // Update sight flags
    //

    int iVisibilityTileWidth(GetWorldPointer()->GetTileWidth() / m_uiVisibilitySize);
    int iVisibilityTileHeight(GetWorldPointer()->GetTileHeight() / m_uiVisibilitySize);

    for (UnitList_cit it(lUnits.begin()); it != itEnd; ++it)
    {
        IUnitEntity *pTarget(*it);

        float fX(pTarget->GetPosition().x);
        float fY(pTarget->GetPosition().y);

        uint uiX(uint(CLAMP(INT_FLOOR(fX / m_fVisibilityScale), 0, iVisibilityTileWidth - 1)));
        uint uiY(uint(CLAMP(INT_FLOOR(fY / m_fVisibilityScale), 0, iVisibilityTileHeight - 1)));

        //
        // Team 1
        //
        if (pTarget->GetLastAggression(TEAM_1) != INVALID_TIME && pTarget->GetLastAggression(TEAM_1) + g_unitAggressionSightTime > Game.GetGameTime())
            pTarget->SetVisibilityFlags(VIS_VISION(TEAM_1));

        if (pTarget->GetTeam() == TEAM_1 || pTarget->HasVisibilityFlags(VIS_VISION(TEAM_1)))
            pTarget->SetVisibilityFlags(VIS_SIGHTED(TEAM_1));

        pTarget->SetVisibilityFlags(m_cVisibilityMap[0].GetVision(uiX, uiY));

        //
        // Team 2
        //
        if (pTarget->GetLastAggression(TEAM_2) != INVALID_TIME && pTarget->GetLastAggression(TEAM_2) + g_unitAggressionSightTime > Game.GetGameTime())
            pTarget->SetVisibilityFlags(VIS_VISION(TEAM_2));

        if (pTarget->GetTeam() == TEAM_2 || pTarget->HasVisibilityFlags(VIS_VISION(TEAM_2)))
            pTarget->SetVisibilityFlags(VIS_SIGHTED(TEAM_2));

        pTarget->SetVisibilityFlags(m_cVisibilityMap[1].GetVision(uiX, uiY) << 8);
    }

    for (uivector_it it(m_vNeutralCamps.begin()), itEnd(m_vNeutralCamps.end()); it != itEnd; ++it)
    {
        CEntityNeutralCampController *pCamp(Game.GetEntityAs<CEntityNeutralCampController>(*it));

        if (pCamp == NULL)
            continue;

        float fX(pCamp->GetPosition().x);
        float fY(pCamp->GetPosition().y);

        uint uiX(uint(CLAMP(INT_FLOOR(fX / m_fVisibilityScale), 0, iVisibilityTileWidth - 1)));
        uint uiY(uint(CLAMP(INT_FLOOR(fY / m_fVisibilityScale), 0, iVisibilityTileHeight - 1)));

        //
        // Team 1
        //
        if (m_cVisibilityMap[0].IsVisible(uiX, uiY))
        {
            pCamp->SetVisibilityFlags(VIS_SIGHTED(1));

            if (!pCamp->GetActive())
                pCamp->SetVisibilityFlags(VIS_REVEALED(1));
            else
                pCamp->RemoveVisibilityFlags(VIS_REVEALED(1));
        }
        else
            pCamp->RemoveVisibilityFlags(VIS_SIGHTED(1));

        //
        // Team 2
        //
        if (m_cVisibilityMap[1].IsVisible(uiX, uiY))
        {
            pCamp->SetVisibilityFlags(VIS_SIGHTED(2));

            if (!pCamp->GetActive())
                pCamp->SetVisibilityFlags(VIS_REVEALED(2));
            else
                pCamp->RemoveVisibilityFlags(VIS_REVEALED(2));
        }
        else
            pCamp->RemoveVisibilityFlags(VIS_SIGHTED(2));
    }

    for (uivector_it it(m_vKongors.begin()), itEnd(m_vKongors.end()); it != itEnd; ++it)
    {
        CEntityBossController *pCamp(Game.GetEntityAs<CEntityBossController>(*it));
        
        if (pCamp == NULL)
            continue;

        float fX(pCamp->GetPosition().x);
        float fY(pCamp->GetPosition().y);

        uint uiX(uint(CLAMP(INT_FLOOR(fX / m_fVisibilityScale), 0, iVisibilityTileWidth - 1)));
        uint uiY(uint(CLAMP(INT_FLOOR(fY / m_fVisibilityScale), 0, iVisibilityTileHeight - 1)));

        //
        // Team 1
        //
        if (m_cVisibilityMap[0].IsVisible(uiX, uiY))
        {
            pCamp->SetVisibilityFlags(VIS_SIGHTED(1));

            if (!pCamp->GetActive())
                pCamp->SetVisibilityFlags(VIS_REVEALED(1));
            else
                pCamp->RemoveVisibilityFlags(VIS_REVEALED(1));
        }
        else
            pCamp->RemoveVisibilityFlags(VIS_SIGHTED(1));

        //
        // Team 2
        //
        if (m_cVisibilityMap[1].IsVisible(uiX, uiY))
        {
            pCamp->SetVisibilityFlags(VIS_SIGHTED(2));

            if (!pCamp->GetActive())
                pCamp->SetVisibilityFlags(VIS_REVEALED(2));
            else
                pCamp->RemoveVisibilityFlags(VIS_REVEALED(2));
        }
        else
            pCamp->RemoveVisibilityFlags(VIS_SIGHTED(2));
    }
}


/*====================
  CGameServer::KillTrees
  ====================*/
void    CGameServer::KillTrees()
{
    m_pServerEntityDirectory->DeactivateBitEntities();
}


/*====================
  CGameServer::SpawnTrees
  ====================*/
void    CGameServer::SpawnTrees()
{
    m_pServerEntityDirectory->ActivateBitEntities();
}


/*====================
  CGameServer::Precache
  ====================*/
void    CGameServer::Precache(const tstring &sName, EPrecacheScheme eScheme)
{
    EntityRegistry.ServerPrecache(EntityRegistry.LookupID(sName), eScheme);
}


/*====================
  CGameServer::Precache
  ====================*/
void    CGameServer::Precache(ushort unType, EPrecacheScheme eScheme)
{
    EntityRegistry.ServerPrecache(unType, eScheme);
}


/*====================
  CGameServer::SendPopup
  ====================*/
void    CGameServer::SendPopup(const CPopup *pPopup, IUnitEntity *pSource, IUnitEntity *pTarget, ushort unValue)
{
    if (pPopup == NULL)
        return;

    if (pTarget == NULL)
        pTarget = pSource;

    if (pSource == NULL || pTarget == NULL)
        return;

    CPlayer *pTargetPlayer(GetPlayer(pTarget->GetOwnerClientNumber()));
    uint uiTargetTeam(pTarget->GetTeam());

    // Assemble the message
    static CBufferFixed<11> buffer;
    buffer.Clear();
    buffer 
        << (pPopup->GetShowValue() ? GAME_CMD_POPUP_VALUE : GAME_CMD_POPUP)
        << pPopup->GetType()
        << byte(pTargetPlayer ? pTargetPlayer->GetClientNumber() : -1)
        << ushort(pSource->GetIndex())
        << GetServerTime();
    if (pPopup->GetShowValue())
        buffer << unValue;

    // Send to target client only
    if (pPopup->GetSelfOnly())
    {
        if (pTargetPlayer == NULL)
            return;

        if (!pTargetPlayer->CanSee(pSource))
            return;

        SendGameData(pTargetPlayer->GetClientNumber(), buffer, sv_reliablePopups);
        return;
    }

    // Send to all clients
    for (PlayerMap_it itPlayer(m_mapClients.begin()); itPlayer != m_mapClients.end(); ++itPlayer)
    {
        CPlayer *pPlayer(itPlayer->second);
        if (pPlayer == NULL)
            continue;
        if (pPlayer->IsDisconnected())
            continue;
        if (pPopup->GetTeamOnly() && pPlayer->GetTeam() != TEAM_SPECTATOR && pPlayer->GetTeam() != uiTargetTeam)
            continue;
        if (!pPlayer->CanSee(pSource))
            continue;
        if (pPopup->GetSpectatorOnly() && pPlayer->GetTeam() != TEAM_SPECTATOR)
            continue;

        SendGameData(pPlayer->GetClientNumber(), buffer, sv_reliablePopups);
    }

    // Replay spectator stream
    SendGameData(-1, buffer, sv_reliablePopups);
}

void    CGameServer::SendPopup(byte yType, IUnitEntity *pSource, IUnitEntity *pTarget, ushort unValue)
{
    SendPopup(Game.GetPopup(yType), pSource, pTarget, unValue);
}

void    CGameServer::SendPopup(EPopup eType, IUnitEntity *pSource, IUnitEntity *pTarget, ushort unValue)
{
    SendPopup(Game.GetPopup(eType), pSource, pTarget, unValue);
}


/*====================
  CGameServer::SendPing
  ====================*/
void    CGameServer::SendPing(const CPing *pPing, IUnitEntity *pSource, IUnitEntity *pTarget, byte yX, byte yY)
{
    if (pPing == NULL)
        return;

    if (pSource == NULL)
        return;

    if (pTarget == NULL)
        pTarget = pSource;

    CPlayer *pSourcePlayer(GetPlayer(pSource->GetOwnerClientNumber()));
    uint uiSourceTeam(pSource->GetTeam());

    // Assemble the message
    CBufferFixed<11> buffer;
    buffer 
        << GAME_CMD_MAP_PING
        << pPing->GetType()
        << byte(pSourcePlayer ? pSourcePlayer->GetClientNumber() : -1)
        << yX
        << yY;

    // Send to source client only
    if (pPing->GetSelfOnly())
    {
        if (pSourcePlayer == NULL)
            return;

        SendGameData(pSourcePlayer->GetClientNumber(), buffer, sv_reliablePings);
        return;
    }

    // Send to target client only
    if (pPing->GetTargetOnly())
    {
        CPlayer *pTargetPlayer(GetPlayer(pTarget->GetOwnerClientNumber()));
        if (pTargetPlayer == NULL)
            return;

        SendGameData(pTargetPlayer->GetClientNumber(), buffer, sv_reliablePings);
        return;
    }

    // Send to all/team clients
    for (PlayerMap_it itPlayer(m_mapClients.begin()); itPlayer != m_mapClients.end(); ++itPlayer)
    {
        CPlayer *pPlayer(itPlayer->second);
        if (pPlayer == NULL)
            continue;
        if (pPlayer->IsDisconnected())
            continue;
        if (pPing->GetTeamOnly() && pPlayer->GetTeam() != TEAM_SPECTATOR && pPlayer->GetTeam() != uiSourceTeam)
            continue;

        SendGameData(pPlayer->GetClientNumber(), buffer, sv_reliablePings);
    }

    // Replay spectator stream
    SendGameData(-1, buffer, sv_reliablePings);
}

void    CGameServer::SendPing(const CPing *pPing, IUnitEntity *pSource, IUnitEntity *pTarget, const CVec2f &v2Pos)
{
    SendPing(pPing, pSource, pTarget, (v2Pos.x / Game.GetWorldWidth()) * UCHAR_MAX, (v2Pos.y / Game.GetWorldHeight()) * UCHAR_MAX);
}

void    CGameServer::SendPing(byte yType, IUnitEntity *pSource, IUnitEntity *pTarget, const CVec2f &v2Pos)
{
    SendPing(Game.GetPing(yType), pSource, pTarget, v2Pos);
}

void    CGameServer::SendPing(EPing eType, IUnitEntity *pSource, IUnitEntity *pTarget, const CVec2f &v2Pos)
{
    SendPing(Game.GetPing(eType), pSource, pTarget, v2Pos);
}


/*====================
  CGameServer::UnitRespawned
  ====================*/
void    CGameServer::UnitRespawned(uint uiIndex)
{
    IUnitEntity *pOwner(Game.GetUnitEntity(uiIndex));

    const UnitList &lUnits(m_pServerEntityDirectory->GetUnitList());
    UnitList_cit itEnd(lUnits.end());
    for (UnitList_cit it(lUnits.begin()); it != itEnd; ++it)
    {
        IUnitEntity *pUnit(*it);
        if (pUnit == NULL ||
            pUnit->GetOwnerIndex() != uiIndex)
            continue;

        pUnit->Action(ACTION_SCRIPT_OWNER_RESPAWN, pOwner, pUnit);
    }
}


/*====================
  CGameServer::UnitKilled
  ====================*/
void    CGameServer::UnitKilled(uint uiIndex)
{
    const UnitList &lUnits(m_pServerEntityDirectory->GetUnitList());
    UnitList_cit itEnd(lUnits.end());
    for (UnitList_cit it(lUnits.begin()); it != itEnd; ++it)
    {
        IUnitEntity *pUnit(*it);
        if (pUnit == NULL ||
            !pUnit->GetDieWithOwner() ||
            pUnit->GetOwnerIndex() != uiIndex)
            continue;

        pUnit->Kill();
    }
}


/*====================
  CGameServer::GetServerInfo
  ====================*/
void    CGameServer::GetServerInfo(CPacket &pkt)
{
    pkt.WriteString(GetGameName());
    pkt.WriteString(CGameInfo::GetGameModeName(GetGameMode()));
    pkt.WriteByte(GetTeamSize());

    CGameInfo *pGameInfo(GetGameInfo());
    if (pGameInfo != NULL)
        pkt.WriteInt(pGameInfo->GetGameOptions());
    else
        pkt.WriteInt(0);
}


/*====================
  CGameServer::GetReconnectInfo
  ====================*/
void    CGameServer::GetReconnectInfo(CPacket &pkt, uint uiMatchID, uint uiAccountID, ushort unConnectionID)
{
    //Console << _CWS("Reconnect info request for match: ") << uiMatchID << _CWS(" account id: ") << uiAccountID << _CWS(" connection id: ") << unConnectionID << newl;

    if (GetGamePhase() <= GAME_PHASE_WAITING_FOR_PLAYERS || GetGamePhase() >= GAME_PHASE_ENDED)
    {
        //Console << _CWS("No current match") << newl;
        pkt.WriteInt(0);
        return;
    }

    CGameInfo *pGameInfo(GetGameInfo());
    if (pGameInfo == NULL)
    {
        //Console << _CWS("Invalid CGameInfo") << newl;
        pkt.WriteInt(0);
        return;
    }

    if (pGameInfo->GetMatchID() == -1)
        uiAccountID = -2 - unConnectionID;

    // Don't send info if game is over
    if (GetWinningTeam() != TEAM_INVALID)
    {
        pkt.WriteInt(0);
        return;
    }

    CPlayer *pPlayer(NULL);
    for (PlayerMap_it itPlayer(m_mapClients.begin()); itPlayer != m_mapClients.end(); ++itPlayer)
    {
        if (itPlayer->second->GetAccountID() != uiAccountID)
            continue;

        pPlayer = itPlayer->second;
        break;
    }

    if (pPlayer == NULL)
    {
        //Console << _CWS("Player does not exist") << newl;
        pkt.WriteInt(0);
        return;
    }

    //Console << _CWS("Current match: ") << pGameInfo->GetMatchID() << newl;
    //Console << _CWS("Player status: ") << (pPlayer->HasFlags(PLAYER_FLAG_TERMINATED) ? _CWS("terminated") : _CWS("active")) << newl;

    if (pGameInfo->GetMatchID() != uiMatchID ||
        pPlayer->HasFlags(PLAYER_FLAG_TERMINATED) ||
        GetGameTime() >= pPlayer->GetTerminationTime())
    {
        pkt.WriteInt(0);
        return;
    }

    pkt.WriteInt(pPlayer->GetTerminationTime() - GetGameTime());
}


/*====================
  CGameServer::GetGameStatus
  ====================*/
void    CGameServer::GetGameStatus(CPacket &pkt)
{
    pkt.WriteByte(GetGamePhase()); // Game phase
}


#ifndef K2_CLIENT
/*====================
  CGameServer::GetHeartbeatInfo
  ====================*/
void    CGameServer::GetHeartbeatInfo(CHTTPRequest *pHeartbeat)
{
    if (pHeartbeat == NULL)
        return;

    pHeartbeat->AddVariable(L"mname", GetGameName());

    CGameInfo *pGameInfo(GetGameInfo());
    if (pGameInfo == NULL)
    {
        Console.Err << L"Invalid game info!" << newl;
        return;
    }

    if (m_pHostServer->IsArrangedMatch())
        pHeartbeat->AddVariable(L"new", 2);
    else if (m_pHostServer->IsTournMatch())
        pHeartbeat->AddVariable(L"new", 3);
    else if (m_pHostServer->IsLeagueMatch())
        pHeartbeat->AddVariable(L"new", 4);
    else
        pHeartbeat->AddVariable(L"new", 1); 
            
    pHeartbeat->AddVariable(L"match_id", pGameInfo->GetMatchID());

    if (pGameInfo->GetMatchID() != -1)
        pHeartbeat->AddVariable(L"option[officl]", 1);

    pHeartbeat->AddVariable(L"max_players", pGameInfo->GetTeamSize());

    switch (pGameInfo->GetGameMode())
    {
    case GAME_MODE_NORMAL:          pHeartbeat->AddVariable(L"mode", L"nm"); break;
    case GAME_MODE_SINGLE_DRAFT:    pHeartbeat->AddVariable(L"mode", L"sd"); break;
    case GAME_MODE_RANDOM_DRAFT:    pHeartbeat->AddVariable(L"mode", L"rd"); break;
    case GAME_MODE_DEATHMATCH:      pHeartbeat->AddVariable(L"mode", L"dm"); break;
    case GAME_MODE_BANNING_DRAFT:   pHeartbeat->AddVariable(L"mode", L"bd"); break;
    case GAME_MODE_CAPTAINS_DRAFT:  pHeartbeat->AddVariable(L"mode", L"cd"); break;
    case GAME_MODE_CAPTAINS_MODE:   pHeartbeat->AddVariable(L"mode", L"cm"); break;
    case GAME_MODE_BANNING_PICK:    pHeartbeat->AddVariable(L"mode", L"bp"); break;
    }
    
    if (pGameInfo->HasGameOptions(GAME_OPTION_ALL_HEROES))
        pHeartbeat->AddVariable(L"option[ap]", 1);
    if (pGameInfo->HasGameOptions(GAME_OPTION_FORCE_RANDOM))
        pHeartbeat->AddVariable(L"option[ar]", 1);
    if (pGameInfo->HasGameOptions(GAME_OPTION_EASY_MODE))
        pHeartbeat->AddVariable(L"option[em]", 1);
    if (pGameInfo->HasGameOptions(GAME_OPTION_SHUFFLE_TEAMS))
        pHeartbeat->AddVariable(L"option[shuf]", 1);
    if (pGameInfo->HasGameOptions(GAME_OPTION_AUTOBALANCE_TEAMS))
        pHeartbeat->AddVariable(L"option[ab]", 1);
    if (pGameInfo->HasGameOptions(GAME_OPTION_NO_REPICK))
        pHeartbeat->AddVariable(L"option[no_repick]", 1);
    if (pGameInfo->HasGameOptions(GAME_OPTION_NO_SWAP))
        pHeartbeat->AddVariable(L"option[no_swap]", 1);
    if (pGameInfo->HasGameOptions(GAME_OPTION_NO_STRENGTH))
        pHeartbeat->AddVariable(L"option[no_str]", 1);
    if (pGameInfo->HasGameOptions(GAME_OPTION_NO_AGILITY))
        pHeartbeat->AddVariable(L"option[no_agi]", 1);
    if (pGameInfo->HasGameOptions(GAME_OPTION_NO_INTELLIGENCE))
        pHeartbeat->AddVariable(L"option[no_int]", 1);
    if (pGameInfo->HasGameOptions(GAME_OPTION_DUPLICATE_HEROES))
        pHeartbeat->AddVariable(L"option[dup_h]", 1);
    if (pGameInfo->HasGameOptions(GAME_OPTION_DROP_ITEMS))
        pHeartbeat->AddVariable(L"option[drp_itm]", 1);
    if (pGameInfo->HasGameOptions(GAME_OPTION_NO_RESPAWN_TIMER))
        pHeartbeat->AddVariable(L"option[no_timer]", 1);
    if (pGameInfo->HasGameOptions(GAME_OPTION_REVERSE_SELECTION))
        pHeartbeat->AddVariable(L"option[rev_hs]", 1);
    if (pGameInfo->HasGameOptions(GAME_OPTION_ALTERNATE_SELECTION))
        pHeartbeat->AddVariable(L"option[alt_pick]", 1);
    if (pGameInfo->HasGameOptions(GAME_OPTION_NO_POWERUPS))
        pHeartbeat->AddVariable(L"option[no_pups]", 1);
    if (pGameInfo->HasGameOptions(GAME_OPTION_ALLOW_VETO))
        pHeartbeat->AddVariable(L"option[veto]", 1);
    if (pGameInfo->HasGameOptions(GAME_OPTION_TOURNAMENT_RULES))
        pHeartbeat->AddVariable(L"option[tr]", 1);
    if (pGameInfo->HasGameOptions(GAME_OPTION_HARDCORE))
        pHeartbeat->AddVariable(L"option[hardcore]", 1);
    if (pGameInfo->HasGameOptions(GAME_OPTION_DEV_HEROES))
        pHeartbeat->AddVariable(L"option[dev_heroes]", 1);
}
#endif


#ifndef K2_CLIENT
/*====================
  CGameServer::ProcessAuthData
  ====================*/
void    CGameServer::ProcessAuthData(int iAccountID, const CPHPData *pData)
{
    float fExp(0.0f), fSecondsGettingExp(0.0f), fGold(0.0f), fSecondsPlayed(0.0f);
    if (m_pHostServer->IsArrangedMatch())
    {
        m_mapPlayerRatings[iAccountID] = pData->GetFloat(_CWS("rnk_amm_solo_rating"), PLAYER_RANK_UNKNOWN);
        m_mapPlayerWins[iAccountID] = pData->GetInteger(_CWS("rnk_wins"), -1);
        m_mapPlayerLosses[iAccountID] = pData->GetInteger(_CWS("rnk_losses"), -1);
        m_mapPlayerKills[iAccountID] = pData->GetInteger(_CWS("rnk_herokills"),-1);
        m_mapPlayerAssists[iAccountID] = pData->GetInteger(_CWS("rnk_heroassists"), -1);
        m_mapPlayerDeaths[iAccountID] = pData->GetInteger(_CWS("rnk_deaths"), -1);

        fSecondsPlayed = pData->GetInteger(_CWS("rnk_secs"), -1);
        fSecondsGettingExp = pData->GetInteger(_CWS("rnk_time_earning_exp"), -1);
        fGold = pData->GetInteger(_CWS("rnk_gold"), -1);
        fExp = pData->GetInteger(_CWS("rnk_exp"), -1);
    }
    else
    {
        m_mapPlayerRatings[iAccountID] = pData->GetFloat(_CWS("acc_pub_skill"), PLAYER_RANK_UNKNOWN);
        m_mapPlayerWins[iAccountID] = pData->GetInteger(_CWS("acc_wins"), -1);
        m_mapPlayerLosses[iAccountID] = pData->GetInteger(_CWS("acc_losses"), -1);
        m_mapPlayerKills[iAccountID] = pData->GetInteger(_CWS("acc_herokills"), -1);
        m_mapPlayerAssists[iAccountID] = pData->GetInteger(_CWS("acc_heroassists"), -1);
        m_mapPlayerDeaths[iAccountID] = pData->GetInteger(_CWS("acc_deaths"), -1);

        fSecondsPlayed = pData->GetInteger(_CWS("acc_secs"), -1);
        fSecondsGettingExp = pData->GetInteger(_CWS("acc_time_earning_exp"), -1);
        fGold = pData->GetInteger(_CWS("acc_gold"), -1);
        fExp = pData->GetInteger(_CWS("acc_exp"), -1);

    }

    float fEzModeGames(pData->GetInteger(_CWS("acc_em_played"), -1));
    float fGamesPlayed(pData->GetInteger(_CWS("acc_games_played"), 0) + pData->GetInteger(_CWS("rnk_games_played"), 0));
    
    m_mapPlayerEmPercent[iAccountID] = ROUND((fEzModeGames / fGamesPlayed) * 100.0f);

    m_mapPlayerDisconnects[iAccountID] = pData->GetInteger(_CWS("rnk_discos"), 0) + pData->GetInteger(_CWS("acc_discos"), 0);
    
    if (fExp == -1.0f || fSecondsGettingExp == -1.0f)
        m_mapPlayerExpMin[iAccountID] = -1.0f;
    else if (fExp != 0.0f && fSecondsGettingExp != 0.0f)
        m_mapPlayerExpMin[iAccountID] = ceil((fExp / (fSecondsGettingExp / 60.0f)) * 10.0f) / 10.0f;
    else
        m_mapPlayerExpMin[iAccountID] = 0.0f;

    if (fGold == -1.0f && fSecondsPlayed == -1.0f)
        m_mapPlayerGoldMin[iAccountID] = -1.0f;
    else if (fGold != 0.0f && fSecondsPlayed != 0.0f)
        m_mapPlayerGoldMin[iAccountID] = ceil((fGold / (fSecondsPlayed / 60.0f)) * 10.0f) / 10.0f;
    else
        m_mapPlayerGoldMin[iAccountID] = 0.0f;

        //Server tracking of trial accounts
    m_mapPlayerAccountType[iAccountID] = pData->GetInteger(_CWS("account_type"), 0);
    m_mapPlayerTrialStatus[iAccountID] = pData->GetInteger(_CWS("trial"), 0);
    m_mapPlayerTrialCount[iAccountID] = pData->GetInteger(_CWS("acc_trial_games_played"), 0);//"acc_no_stats_played"), 0);

}
#endif

/*====================
  CGameServer::RegisterShopInfo
  ====================*/
void    CGameServer::RegisterShopInfo()
{
    const map<uint, CTeamInfo*> mapTeams(GetTeams());

    map<ushort, tstring> mapEntities;
    EntityRegistry.GetEntityList(mapEntities);

    for (map<uint, CTeamInfo*>::const_iterator it(mapTeams.begin()); it != mapTeams.end(); it++)
    {
        if (!it->second->IsActiveTeam())
            continue;

        CShopInfo *pShopInfo = static_cast<CShopInfo *>(m_pServerEntityDirectory->Allocate(Shop_Info));
        it->second->RegisterShopInfo(pShopInfo->GetIndex());

        for (map<ushort, tstring>::iterator itEntity(mapEntities.begin()); itEntity != mapEntities.end(); ++itEntity)
        {
            CItemDefinition *pItemDefinition(EntityRegistry.GetDefinition<CItemDefinition>(itEntity->first));

            if (pItemDefinition == NULL || pItemDefinition->GetMaxStock() == 0)
                continue;

            CShopItemInfo *pShopItem = static_cast<CShopItemInfo *>(m_pServerEntityDirectory->Allocate(Shop_ItemInfo));
            pShopItem->SetItemName(pItemDefinition->GetName());
            pShopItem->SetItemType(pItemDefinition->GetTypeID());
            pShopItem->SetMaxStock(pItemDefinition->GetMaxStock());
            pShopItem->SetRemainingStock(pItemDefinition->GetInitialStock());
            pShopItem->SetRestockTime(pItemDefinition->GetRestockDelay());
            pShopItem->SetTeam(it->first);

            pShopInfo->AddItem(pShopItem);
        }
    }
}


/*====================
  CGameServer::LongServerFrame
  ====================*/
void    CGameServer::LongServerFrame(uint uiFrameLength)
{
    m_uiLongServerFrame = uiFrameLength;
}


/*====================
  CGameServer::ClientStateChange
  ====================*/
void    CGameServer::ClientStateChange(int iClientNum, EClientConnectionState eState)
{
    if (ReplayManager.IsPlaying())
    {
        if (eState == CLIENT_CONNECTION_STATE_IN_GAME)
            ReplayManager.SetPaused(false);

        return;
    }

    if (eState == CLIENT_CONNECTION_STATE_IN_GAME)
    {
        CPlayer *pPlayer(GetPlayer(iClientNum));
        if (pPlayer != NULL)
        {
            CGameInfo *pGameInfo(GetGameInfo());
            if (pGameInfo != NULL)
                pGameInfo->ExecuteActionScript(ACTION_SCRIPT_ENTER_GAME, pGameInfo, NULL, pPlayer, V3_ZERO);
        }
    }
}


