// (C)2006 S2 Games
// c_teaminfo.h
//
//=============================================================================
#ifndef __C_TEAMINFO_H__
#define __C_TEAMINFO_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gameentity.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CTeamDefinition;

extern GAME_SHARED_API CCvarb sv_allowNoCommander;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const int MAX_OFFICERS(5);
//=============================================================================

//=============================================================================
// CEntityTeamInfo
//=============================================================================
class CEntityTeamInfo : public IGameEntity
{
private:
    static vector<SDataField>   *s_pvFields;

    DECLARE_ENT_ALLOCATOR(Entity, TeamInfo);

    bool                m_bRosterChanged;
    CTeamDefinition*    m_pDefinition;

    int                 m_iTeamID;
    tstring             m_sName;
    int                 m_iCommanderClientID;
    int                 m_iOfficialCommanderClientID;
    int                 m_iLastCommanderClientID;
    ivector             m_vClients;

    float               m_fExperience;
    int                 m_iLevel;
    int                 m_iPointsSpent;
    int                 m_aiStats[NUM_PLAYER_ATTRIBUTES];

    uint                m_uiGold;
    uint                m_uiNextEconomyInterval;

    float               m_fMana;

    int                 m_aiOfficers[MAX_OFFICERS];
    iset                m_asetSquads[MAX_OFFICERS];
    uiset               m_asetSquadObjects[MAX_OFFICERS];

    uint                m_uiBaseBuildingIndex;
    uiset               m_setBuildingIndices;
    uiset               m_setSpawnBuildingIndices;
    uivector            m_vSpawnPointIndices;
    
    map<uint, float>    m_mapDamagedBuildings;
    uint                m_uiLastAttackNotifyTime;
    bool                m_bDamagedBuildingSetCleared;

    int                 m_iHellShrineCount;
    bool                m_bPlayedMalphasSound;

    ideque              m_deqSpawnQueue;    

    uint                m_uiLastWorkerSpawnTime;

    ushort              m_unWorkerTypeID;

    iset                m_setAlliedTeams;

public:
    ~CEntityTeamInfo();
    CEntityTeamInfo();

    // Network
    virtual void    Baseline();
    virtual void    GetSnapshot(CEntitySnapshot &snapshot) const;
    virtual bool    ReadSnapshot(CEntitySnapshot &snapshot);
    
    GAME_SHARED_API static const vector<SDataField>&    GetTypeVector();


    GAME_SHARED_API bool            IsValid();

    void                            SetDefinition(CTeamDefinition *pDef)    { if (pDef != NULL) m_pDefinition = pDef; }
    CTeamDefinition*                GetDefinition() const                   { return m_pDefinition; }
        
    void                            SetTeamID(int iTeamID)                  { m_iTeamID = iTeamID; }
    int                             GetTeamID() const                       { return m_iTeamID; }

    GAME_SHARED_API void            Initialize();

    bool                            RosterChanged()                         { bool b(m_bRosterChanged); m_bRosterChanged = false; return b; }

    // Clients
    void                            SetClientList(const ivector &vClients)  { m_vClients.clear(); m_vClients = vClients; m_bRosterChanged = true; }
    GAME_SHARED_API void            AddClient(int iClientNumber);
    GAME_SHARED_API void            RemoveClient(int iClientID);
    int                             GetNumClients() const                   { return int(m_vClients.size()); }
    const ivector&                  GetClientList() const                   { return m_vClients; }
    ivector&                        GetClientList()                         { return m_vClients; }
    GAME_SHARED_API tstring         GetClientName(uint uiTeamIndex);
    GAME_SHARED_API int             GetClientIDFromTeamIndex(uint uiTeamIndex) const;
    GAME_SHARED_API uint            GetTeamIndexFromClientID(int iClientID) const;
    GAME_SHARED_API byte            GetSquadFromClientID(int iClientID) const;

    GAME_SHARED_API uint            GetAverageSF();

    void                            SetName(const tstring &sName)           { m_sName = sName; }
    tstring                         GetName() const                         { return m_sName; }

    GAME_SHARED_API float           GetEconomyIntervalPercent() const;
    uint                            GetNextEconomyInterval() const          { return m_uiNextEconomyInterval; }

    // Gold
    uint                            GetGold() const                         { return m_uiGold; }
    GAME_SHARED_API void            GiveGold(uint uiGold);
    bool                            SpendGold(uint uiGold)                  { if (m_uiGold < uiGold) return false; m_uiGold -= uiGold; return true; }

    void                            SetMana(float fMana)                    { m_fMana = fMana; }
    void                            GiveMana(float fMana)                   { m_fMana += fMana; }
    float                           GetMana() const                         { return m_fMana; }
    GAME_SHARED_API float           GetMaxMana() const;
    float                           GetManaPercent() const                  { return GetMana() / GetMaxMana(); }
    bool                            SpendMana(float fCost)                  { if (fCost > m_fMana) return false; m_fMana -= fCost; return true; }

    // Experience
    void                            UpdateTeamExperience();
    int                             GetLevel() const                        { return m_iLevel; }
    float                           GetExperience() const                   { return m_fExperience; }
    GAME_SHARED_API float           GetPercentNextLevel() const;
    GAME_SHARED_API int             GetAvailablePoints() const;
    GAME_SHARED_API int             GetAttributeCost(int iStat);
    GAME_SHARED_API void            SpendPoint(int iStat);
    int                             GetStatLevel(int iStat) const           { if (iStat <= ATTRIBUTE_NULL || iStat >= NUM_PLAYER_ATTRIBUTES) return 0; return m_aiStats[iStat]; }

    // Commander
    GAME_SHARED_API int                 GetVoteCount(int iClientID);
    float                               GetVotePercent(int iClientID)           { return GetVoteCount(iClientID) / float(m_vClients.size()); }
    GAME_SHARED_API bool                IsMajoritySelection(float fMajority = 0.67f);
    GAME_SHARED_API bool                SelectCommander();
    bool                                IsCommander(uint uiTeamIndex)           { return m_iCommanderClientID == GetClientIDFromTeamIndex(uiTeamIndex); }
    GAME_SHARED_API int                 GetNumCandidates() const;
    GAME_SHARED_API int                 GetMaxCandidates() const;
    GAME_SHARED_API int                 GetCommanderClientID() const            { return m_iCommanderClientID; }
    GAME_SHARED_API int                 GetOfficialCommanderClientID() const    { return m_iOfficialCommanderClientID; }
    GAME_SHARED_API int                 GetLastCommanderClientID() const        { return m_iLastCommanderClientID; }
    int                                 GetOfficerClientID(byte ySquad) const   { return ySquad < MAX_OFFICERS ? m_aiOfficers[ySquad] : -1; }
    GAME_SHARED_API void                RemoveCommander();
    GAME_SHARED_API bool                HasCommander();
    GAME_SHARED_API void                SetCommander(int iClientID);
    GAME_SHARED_API CEntityClientInfo*  GetCommanderClient();
    GAME_SHARED_API CPlayerCommander*   GetCommanderPlayerEntity();

    // Officers
    GAME_SHARED_API uint            GetNumOfficers() const;
    GAME_SHARED_API uint            GetMaxOfficers() const;
    GAME_SHARED_API void            PromotePlayer(int iClientID);
    GAME_SHARED_API void            PromoteFromSquad(byte ySquad, bool bIgnoreDecline = false);
    GAME_SHARED_API void            PromoteFromTeam(bool bIgnoreDecline = false);
    GAME_SHARED_API void            DemotePlayer(byte ySquad);
    GAME_SHARED_API void            DeclineOfficer(int iClientID);
    GAME_SHARED_API bool            IsOfficer(uint uiTeamIndex) const;
    GAME_SHARED_API void            SelectOfficers();

    // Squads
    GAME_SHARED_API void            JoinSquad(int iClient, byte ySquad);
    GAME_SHARED_API const tstring&  GetSquadColor(uint uiSquad);
    GAME_SHARED_API const tstring&  GetSquadName(uint uiSquad);
    GAME_SHARED_API bool            AreAllPlayersInSquads() const;
    GAME_SHARED_API void            FillSquads();
    GAME_SHARED_API void            AddSquadObject(byte ySquad, uint uiIndex);
    GAME_SHARED_API void            KillSquadObjects(byte ySquad);
    GAME_SHARED_API uint            GetOfficerGameIndex(byte ySquad);
    GAME_SHARED_API uint            GetSquadMemberIndex(byte ySquad, uint uiMember);
    GAME_SHARED_API uint            GetMaxSquadSize() const;
    GAME_SHARED_API uint            GetSquadSize(byte ySquad) const         { if (ySquad > MAX_OFFICERS) return 0; return uint(m_asetSquads[ySquad].size()); }

    // Base buildings
    void                            SetBaseBuildingIndex(uint uiIndex)      { m_uiBaseBuildingIndex = uiIndex; }
    uint                            GetBaseBuildingIndex() const            { return m_uiBaseBuildingIndex; }

    // Spawn points
    void                            AddSpawnPointIndex(uint uiIndex);

    // Buildings
    void                            AddBuildingIndex(uint uiIndex)          { m_setBuildingIndices.insert(uiIndex); }
    void                            RemoveBuildingIndex(uint uiIndex)       { m_setBuildingIndices.erase(uiIndex); }
    GAME_SHARED_API bool            HasBuilding(const tstring &sName) const;
    GAME_SHARED_API uint            GetBuildingCount(const tstring &sName) const;
    const uiset&                    GetBuildingSet() const                  { return m_setBuildingIndices; }
    void                            BuildingDamaged(uint uiIndex, uint uiAttacker, float fDamage);

    // Spawn buildings
    void                            AddSpawnBuildingIndex(uint uiIndex)     { m_setSpawnBuildingIndices.insert(uiIndex); }
    void                            RemoveSpawnBuildingIndex(uint uiIndex)  { m_setSpawnBuildingIndices.erase(uiIndex); }
    bool                            IsSpawnBuilding(uint uiIndex)           { return m_setSpawnBuildingIndices.find(uiIndex) != m_setSpawnBuildingIndices.end(); }
    GAME_SHARED_API uint            GetNumSpawnBuildings(IPlayerEntity *pPlayer) const;

    // Malphas/Hellshrine notifications
    void                            HellShrineConstructionStarted()         { m_iHellShrineCount++; }
    void                            HellShrineDestroyed()                   { if (--m_iHellShrineCount == 0) m_bPlayedMalphasSound = false; }
    void                            SetPlayedMalphasSound(bool bPlayed)     { m_bPlayedMalphasSound = bPlayed; }
    bool                            GetPlayedMalphasSound()                 { return m_bPlayedMalphasSound; }

    void                            Spawn();
    bool                            ServerFrame();
    GAME_SHARED_API void            UpdateRoster();
    GAME_SHARED_API void            SortClientList();

    GAME_SHARED_API uint            GetActiveUpkeep() const;
    GAME_SHARED_API uint            GetTotalUpkeep() const;
    GAME_SHARED_API uint            GetTotalIncome() const;

    GAME_SHARED_API bool            IsAlliedTeam(int iTeam)                 { return (m_setAlliedTeams.find(iTeam) != m_setAlliedTeams.end()); }
    GAME_SHARED_API void            AddAlliedTeam(int iTeam)                { m_setAlliedTeams.insert(iTeam); }
    GAME_SHARED_API void            RemoveAlliedTeam(int iTeam)             { m_setAlliedTeams.erase(iTeam); }

    // Spawn queue
    GAME_SHARED_API void            UpdateSpawnQueue(CEntityClientInfo *pClient);
    GAME_SHARED_API void            AddClientToSpawnQueue(CEntityClientInfo *pClient);
    GAME_SHARED_API void            SpawnClientFromQueue();
    GAME_SHARED_API int             GetNumActiveClients();

    // Workers
    GAME_SHARED_API ushort          GetWorkerType(bool bForceLookup = false);
    GAME_SHARED_API bool            CanSpawnWorker(int iClientNum);
    GAME_SHARED_API void            SpawnWorker(int iClientNum);
    GAME_SHARED_API float           GetWorkerCooldownPercent() const;
    GAME_SHARED_API uivector        GetWorkerList();

    bool        Reset()         { return true; }
};
//=============================================================================

#endif //__C_TEAMINFO_H__
