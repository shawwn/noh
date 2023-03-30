// (C)2005 S2 Games
// c_entityclientinfo.h
//
//=============================================================================
#ifndef __C_ENTITYCLIENTINFO_H__
#define __C_ENTITYCLIENTINFO_H__

//=============================================================================
// Definitions
//=============================================================================
enum EPersistantStats
{
    PLAYER_PERSISTANT_RANK,
    PLAYER_PERSISTANT_EXPERIENCE,
    PLAYER_PERSISTANT_LEVEL,
    PLAYER_PERSISTANT_WINS,
    PLAYER_PERSISTANT_LOSSES,
    PLAYER_PERSISTANT_DISCONNECTS,
    PLAYER_PERSISTANT_KILLS,
    PLAYER_PERSISTANT_DEATHS,
    PLAYER_PERSISTANT_ASSISTS,
    PLAYER_PERSISTANT_SOULS,
    PLAYER_PERSISTANT_RAZED,
    PLAYER_PERSISTANT_PLAYERDAMAGE,
    PLAYER_PERSISTANT_BUILDINGDAMAGE,
    PLAYER_PERSISTANT_NPCKILLS,
    PLAYER_PERSISTANT_HPHEALED,
    PLAYER_PERSISTANT_RESURRECTS,
    PLAYER_PERSISTANT_KARMA,
    PLAYER_PERSISTANT_GOLDEARNED,
    PLAYER_PERSISTANT_HPREPAIRED,
    PLAYER_PERSISTANT_HOURSPLAYED,
    PLAYER_PERSISTANT_MINUTESPLAYED,
    PLAYER_PERSISTANT_SECONDSPLAYED,
    PLAYER_PERSISTANT_KILLSRATIO,
    PLAYER_PERSISTANT_DEATHSRATIO,
    PLAYER_PERSISTANT_SKILLFACTOR,
    PLAYER_PERSISTANT_LOYALTY,

    NUM_PERSISTANT_STATS
};

const EDataType g_eMatchStatType[] =
{
    TYPE_INT,
    TYPE_INT,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_FLOAT,
    TYPE_FLOAT,
    TYPE_INT,
    TYPE_INT,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_INT,
    TYPE_INT,

    TYPE_INT,
    TYPE_FLOAT,
    TYPE_INT,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_FLOAT,
    TYPE_INT,
    TYPE_INT,
    TYPE_INT,
    TYPE_INT,
};

typedef map<int, flint>         imapfi;
typedef imapfi::iterator        imapfi_it;
typedef imapfi::const_iterator  imapfi_cit;

typedef map<ushort, flint>      unmapfi;
typedef unmapfi::iterator       unmapfi_it;
typedef unmapfi::const_iterator unmapfi_cit;

struct SMatchStatEvent
{
    uint    uiTime;
    int     iClientID;
    ushort  unTargetType;
    ushort  unInflictorType;

    SMatchStatEvent(uint time, int client, ushort target, ushort inflictor) :
    uiTime(time),
    iClientID(client),
    unTargetType(target),
    unInflictorType(inflictor)
    {}

};

typedef vector<SMatchStatEvent>                 MatchStatEventVector;
typedef MatchStatEventVector::iterator          MatchStatEventVector_it;
typedef MatchStatEventVector::const_iterator    MatchStatEventVector_cit;

class CMatchStatRecord
{
private:
    MatchStatEventVector    m_vEvents;
    flint                   m_fiTotal;
    imapfi                  m_mapPerClient;
    unmapfi                 m_mapPerInflictorType;
    unmapfi                 m_mapPerTargetType;

public:
    ~CMatchStatRecord() {}
    CMatchStatRecord()  { m_fiTotal.i = 0; }

    int     GetTotalInt() const     { return m_fiTotal.i; }
    float   GetTotalFloat() const   { return m_fiTotal.f; }

    const MatchStatEventVector& GetEvents() const           { return m_vEvents; }
    const imapfi&           GetPerClientData() const    { return m_mapPerClient; }
    const unmapfi&          GetPerTargetData() const    { return m_mapPerTargetType; }

    void    AddEvent(float fValue, int iClientID, ushort unTargetType, ushort unInflictorType, uint uiTime = INVALID_TIME)
    {
        m_fiTotal.f += fValue;
        if (iClientID != -1)
        {
            if (m_mapPerClient.find(iClientID) != m_mapPerClient.end())
                m_mapPerClient[iClientID].f += fValue;
            else
                m_mapPerClient[iClientID].f = fValue;
        }
        if (unInflictorType != INVALID_ENT_TYPE)
        {
            if (m_mapPerInflictorType.find(unInflictorType) != m_mapPerInflictorType.end())
                m_mapPerInflictorType[unInflictorType].f += fValue;
            else
                m_mapPerInflictorType[unInflictorType].f = fValue;
        }
        if (unTargetType != INVALID_ENT_TYPE)
        {
            if (m_mapPerTargetType.find(unTargetType) != m_mapPerTargetType.end())
                m_mapPerTargetType[unTargetType].f += fValue;
            else
                m_mapPerTargetType[unTargetType].f = fValue;
        }

        m_vEvents.push_back(SMatchStatEvent((uiTime != INVALID_TIME ? uiTime : Game.GetCurrentGameLength()), iClientID, unTargetType, unInflictorType));
    }

    void    AddEvent(int iValue, int iClientID, ushort unTargetType, ushort unInflictorType, uint uiTime = INVALID_TIME)
    {
        m_fiTotal.i += iValue;
        if (iClientID != -1)
        {
            if (m_mapPerClient.find(iClientID) != m_mapPerClient.end())
                m_mapPerClient[iClientID].i += iValue;
            else
                m_mapPerClient[iClientID].i = iValue;
        }
        if (unInflictorType != INVALID_ENT_TYPE)
        {
            if (m_mapPerInflictorType.find(unInflictorType) != m_mapPerInflictorType.end())
                m_mapPerInflictorType[unInflictorType].i += iValue;
            else
                m_mapPerInflictorType[unInflictorType].i = iValue;
        }
        if (unTargetType != INVALID_ENT_TYPE)
        {
            if (m_mapPerTargetType.find(unTargetType) != m_mapPerTargetType.end())
                m_mapPerTargetType[unTargetType].i += iValue;
            else
                m_mapPerTargetType[unTargetType].i = iValue;
        }
        
        m_vEvents.push_back(SMatchStatEvent((uiTime != INVALID_TIME ? uiTime : Game.GetCurrentGameLength()), iClientID, unTargetType, unInflictorType));
    }

    void    Clear()
    {
        m_fiTotal.i = 0;
        m_vEvents.clear();
        m_mapPerClient.clear();
        m_mapPerInflictorType.clear();
        m_mapPerTargetType.clear();
    }

    void    WriteToBuffer(IBuffer &buffer, EDataType eType) const
    {
        if (eType == TYPE_FLOAT)
            buffer << m_fiTotal.f;
        else
            buffer << m_fiTotal.i;

        buffer << INT_SIZE(m_vEvents.size());
        for (MatchStatEventVector_cit cit(m_vEvents.begin()); cit != m_vEvents.end(); ++cit)
            buffer << cit->uiTime << cit->iClientID << cit->unTargetType << cit->unInflictorType;
        
        buffer << INT_SIZE(m_mapPerClient.size());
        for (imapfi_cit cit(m_mapPerClient.begin()); cit != m_mapPerClient.end(); ++cit)
        {
            float fValue(eType == TYPE_FLOAT ? cit->second.f : cit->second.i);
            buffer << cit->first << fValue;
        }

        buffer << INT_SIZE(m_mapPerInflictorType.size());
        for (unmapfi_cit cit(m_mapPerInflictorType.begin()); cit != m_mapPerInflictorType.end(); ++cit)
        {
            float fValue(eType == TYPE_FLOAT ? cit->second.f : cit->second.i);
            buffer << cit->first << fValue;
        }

        buffer << INT_SIZE(m_mapPerTargetType.size());
        for (unmapfi_cit cit(m_mapPerTargetType.begin()); cit != m_mapPerTargetType.end(); ++cit)
        {
            float fValue(eType == TYPE_FLOAT ? cit->second.f : cit->second.i);
            buffer << cit->first << fValue;
        }
    }

    void    ReadFromBuffer(IBuffer &buffer, EDataType eType)
    {
        if (eType == TYPE_FLOAT)
            m_fiTotal.f = buffer.ReadFloat();
        else
            m_fiTotal.i = buffer.ReadInt();

        m_vEvents.clear();
        int iNumEvents(buffer.ReadInt());
        for (int i(0); i < iNumEvents; ++i)
        {
            uint uiTime(buffer.ReadInt());
            int iClient(buffer.ReadInt());
            ushort unTarget(buffer.ReadShort());
            ushort unInflictor(buffer.ReadShort());
            m_vEvents.push_back(SMatchStatEvent(uiTime, iClient, unTarget, unInflictor));
        }
        
        m_mapPerClient.clear();
        int iNumClients(buffer.ReadInt());
        for (int i(0); i < iNumClients; ++i)
        {
            int iClient(buffer.ReadInt());
            m_mapPerClient[iClient].f = buffer.ReadFloat();
        }

        m_mapPerInflictorType.clear();
        int iNumInflictors(buffer.ReadInt());
        for (int i(0); i < iNumInflictors; ++i)
        {
            ushort unInflictor(buffer.ReadShort());
            m_mapPerInflictorType[unInflictor].f = buffer.ReadFloat();
        }

        m_mapPerTargetType.clear();
        int iNumTargets(buffer.ReadInt());
        for (int i(0); i < iNumTargets; ++i)
        {
            ushort unTarget(buffer.ReadShort());
            m_mapPerTargetType[unTarget].f = buffer.ReadFloat();
        }
    }
};

const int CLIENT_INFO_WANTS_TO_COMMAND      (BIT(0));
const int CLIENT_INFO_WANTS_TO_BE_OFFICER   (BIT(1));
const int CLIENT_INFO_IS_COMMANDER          (BIT(2));
const int CLIENT_INFO_IS_OFFICER            (BIT(3));
const int CLIENT_INFO_DISCONNECTED          (BIT(4));

const int MAX_DEPLOYED_GADGETS(4);
//=============================================================================

//=============================================================================
// CEntityClientInfo
//=============================================================================
class CEntityClientInfo : public IGameEntity
{
private:
    static vector<SDataField>*  s_pvFields;

    DECLARE_ENT_ALLOCATOR(Entity, ClientInfo);

    uint    m_uiPlayerEntityIndex;

    // Identity
    int     m_iAccountID;
    int     m_iClientNumber;
    tstring m_sName;
    tstring m_sClanName;
    tstring m_sClanRank;
    int     m_iKarma;

    // Affiliations
    int     m_iVote;
    int     m_iFlags;

    int     m_iTeam;
    int     m_iLastTeam;
    byte    m_ySquad;

    // Assets
    float   m_fExperience;
    float   m_fCommExperience;
    ushort  m_unGold;
    float   m_fInitialExperience;
    ushort  m_unInitialGold;
    ushort  m_unSouls;
    int     m_iLevel;
    int     m_iCommLevel;
    ushort  m_unAttributePointsSpent;
    short   m_anAttributes[NUM_PLAYER_ATTRIBUTES];

    uint    m_auiGadgets[MAX_DEPLOYED_GADGETS];

    // Statistics
    ushort  m_unPing;

    uint    m_uiConnectTime;
    uint    m_uiDisconnectTime;
    uint    m_uiPlayTime;
    uint    m_uiCommPlayTime;
    uint    m_uiDiscTime;
    uint    m_uiCommDiscTime;
    uint    m_uiDemoTimeRemaining;

    uint    m_uiLoadoutTime;
    byte    m_ySpawnQueuePosition;

    uint    m_uiLastInputTime;

    // These are just for transmitting match stat totals while the game is in progress
    int     m_iKills;
    int     m_iDeaths;
    int     m_iAssists;
    float   m_fPlayerDamage;
    int     m_iRazes;
    float   m_fBuildingDamage;
    float   m_fHealed;
    int     m_iResurrects;
    float   m_fRepaired;

    CMatchStatRecord    m_aMatchStatRecords[NUM_PLAYER_MATCH_STATS];
    int                 m_iPersistantStats[NUM_PERSISTANT_STATS];

public:
    ~CEntityClientInfo()    {}
    CEntityClientInfo();

    GAME_SHARED_API void    Initialize(class CClientConnection *pClientConnection);
    bool                    ServerFrame();

    // Network
    void    Baseline();
    void    GetSnapshot(CEntitySnapshot &snapshot) const;
    bool    ReadSnapshot(CEntitySnapshot &snapshot);
    
    static const vector<SDataField>&    GetTypeVector();

    int     GetPrivateClient()                          { return m_iClientNumber; }

    uint            GetLastInputTime() const            { return m_uiLastInputTime; }
    void            SetLastInputTime(uint uiTime)       { m_uiLastInputTime = uiTime; }

    // Identity
    int             GetAccountID() const                { return m_iAccountID; }
    int             GetClientNumber() const             { return m_iClientNumber; }
    const tstring&  GetName() const                     { return m_sName; }
    int             GetKarma() const                    { return m_iKarma; }
    
    void            SetClanName(const tstring &sName)   { m_sClanName = sName; }
    const tstring&  GetClanName() const                 { return m_sClanName; }

    void            SetClanRank(const tstring &sRank)   { m_sClanRank = sRank; }
    const tstring&  GetClanRank() const                 { return m_sClanRank; }

    // Affiliations
    GAME_SHARED_API void    ClearAffiliations();

    void            ClearFlags()                        { m_iFlags = 0; }
    void            SetFlags(int iFlags)                { m_iFlags |= iFlags; }
    void            RemoveFlags(int iFlags)             { m_iFlags &= ~iFlags; }
    bool            HasFlags(int iFlags) const          { return (m_iFlags & iFlags) == iFlags; }

    void            SetVote(int iVote)                  { m_iVote = iVote; }
    int             GetVote() const                     { return m_iVote; }

    GAME_SHARED_API void    SetTeam(int iTeam);
    int             GetTeam() const                     { return m_iTeam; }

    int             GetLastTeam() const                 { return m_iLastTeam; }

    GAME_SHARED_API void    SetSquad(byte ySquad);
    byte            GetSquad() const                    { return m_ySquad; }
    
    // Assets
    void                    ResetExperience()               { m_fExperience = 0.0f; m_fCommExperience = 0.0f; m_iLevel = 1; }
    float                   GetExperience() const           { return m_fExperience; }
    float                   GetCommExperience() const       { return m_fExperience; }
    int                     GetLevel() const                { return m_iLevel; }
    int                     GetCommLevel() const            { return m_iCommLevel; }
    GAME_SHARED_API float   GetPercentNextLevel() const;
    void                    GiveCommExperience(float fExperience, const CVec3f &v3Pos);
    GAME_SHARED_API void    GiveCommExperience(float fExperience);
    void                    GiveExperience(float fExperience, const CVec3f &v3Pos);
    GAME_SHARED_API void    GiveExperience(float fExperience);

    GAME_SHARED_API void    SetInitialExperience(float fExperience)     { m_fInitialExperience = fExperience; }
    GAME_SHARED_API float   GetInitialExperience()                      { return m_fInitialExperience; }

    GAME_SHARED_API void    GiveGold(ushort unGold);
    ushort          GetGold() const                     { return m_unGold; }
    bool            SpendGold(ushort unCost)            { if (m_unGold < unCost) return false; m_unGold -= unCost; return true; }

    void            SetInitialGold(ushort unGold)       { m_unInitialGold = unGold; }
    ushort          GetInitialGold()                    { return m_unInitialGold; }

    void            AddSoul()                           { ++m_unSouls; }
    ushort          GetSouls() const                    { return m_unSouls; }
    bool            SpendSouls(int iCount)              { if (iCount > m_unSouls) return false; m_unSouls -= iCount; MatchStatEvent(PLAYER_MATCH_SOULS_SPENT, iCount); return true; }

    ushort                  GetAttributePointsSpent() const         { return m_unAttributePointsSpent; }
    GAME_SHARED_API int     GetAvailablePoints() const;
    GAME_SHARED_API void    SpendPoint(int iStat);
    GAME_SHARED_API float   GetAttributeBoost(int iAttribute) const;
    GAME_SHARED_API float   GetAttributeBoostIncrease(int iAttribute) const;
    GAME_SHARED_API int     GetAttributeCost(int iStat);
    int                     GetAttributeLevel(int iAttribute) const { if (iAttribute <= ATTRIBUTE_NULL || iAttribute >= NUM_PLAYER_ATTRIBUTES) return 0; return m_anAttributes[iAttribute]; }
    GAME_SHARED_API void    ResetAttributes();
    
    // Gadgets
    void            AddGadget(uint uiIndex);
    void            RemoveGadget(uint uiIndex);
    uint            GetGadgetIndex(int iIndex)          { return m_auiGadgets[CLAMP(iIndex, 0, MAX_DEPLOYED_GADGETS - 1)]; }

    // Statistics
    uint            GetPlayTime() const                 { return m_uiPlayTime; }
    uint            GetCommPlayTime() const             { return m_uiCommPlayTime; }
    uint            GetDiscTime() const                 { return m_uiDiscTime; }
    uint            GetCommDiscTime() const             { return m_uiCommDiscTime; }

    void            SetPing(ushort unPing)              { m_unPing = unPing; }
    ushort          GetPing() const                     { return m_unPing; }

    GAME_SHARED_API uint            GetDisconnectTime()                 { return m_uiDisconnectTime; }
    GAME_SHARED_API void            SetDisconnectTime(uint uiTime)      { m_uiDisconnectTime = uiTime; }
    GAME_SHARED_API void            SetDisconnected(bool b);
    GAME_SHARED_API bool            IsDisconnected() const              { return HasFlags(CLIENT_INFO_DISCONNECTED); }

    GAME_SHARED_API bool            IsDemoAccount() const               { return (m_uiDemoTimeRemaining != INVALID_TIME); }

    GAME_SHARED_API IPlayerEntity*  GetPlayerEntity();
    GAME_SHARED_API uint            GetPlayerEntityIndex();

    GAME_SHARED_API bool    ChangeUnit(ushort unNewUnitID, int iFlags = 0);

    // Match statistics
    int                     GetKills() const            { return m_iKills; }
    int                     GetDeaths() const           { return m_iDeaths; }
    int                     GetAssists() const          { return m_iAssists; }
    float                   GetPlayerDamage() const     { return m_fPlayerDamage; }
    int                     GetRazes() const            { return m_iRazes; }
    float                   GetBuildingDamage() const   { return m_fBuildingDamage; }
    float                   GetHealed() const           { return m_fHealed; }
    int                     GetResurrects() const       { return m_iResurrects; }
    float                   GetRepaired() const         { return m_fRepaired; }

    GAME_SHARED_API void    WriteMatchStatBuffer(IBuffer &buffer);
    GAME_SHARED_API void    ReadMatchStatBuffer(IBuffer &buffer);

    float                   GetMatchStatTotalFloat(EPlayerMatchStat eStat)      { return m_aMatchStatRecords[eStat].GetTotalFloat(); }
    int                     GetMatchStatTotalInt(EPlayerMatchStat eStat)        { return m_aMatchStatRecords[eStat].GetTotalInt(); }
    CMatchStatRecord&       GetMatchStatRecord(EPlayerMatchStat eStat)          { return m_aMatchStatRecords[eStat]; }

    GAME_SHARED_API void    MatchStatEvent(EPlayerMatchStat eStat, float fValue, int iTargetClientID = -1, ushort unInflictorType = INVALID_ENT_TYPE, ushort unTargetType = INVALID_ENT_TYPE, uint uiTime = INVALID_TIME);
    GAME_SHARED_API void    MatchStatEvent(EPlayerMatchStat eStat, int iValue, int iTargetClientID = -1, ushort unInflictorType = INVALID_ENT_TYPE, ushort unTargetType = INVALID_ENT_TYPE, uint uiTime = INVALID_TIME);

    // Persistant stats
    GAME_SHARED_API void    SetPersistantStat(int iPersistantStat, int iValue)  { m_iPersistantStats[iPersistantStat] = iValue; }
    GAME_SHARED_API int     GetPersistantStat(int iPersistantStat)              { return m_iPersistantStats[iPersistantStat]; }

    virtual void            GameStart();
    virtual bool            Reset();

    GAME_SHARED_API uint    GetLoadoutTime()                                    { return m_uiLoadoutTime; }
    GAME_SHARED_API void    SetLoadoutTime(uint uiDuration);
    GAME_SHARED_API uint    GetRemainingLoadoutTime() const;

    GAME_SHARED_API byte    GetSpawnQueuePosition()                             { return m_ySpawnQueuePosition; }
    GAME_SHARED_API void    SetSpawnQueuePosition(byte yPos)                    { m_ySpawnQueuePosition = yPos; }

    GAME_SHARED_API uint    GetDemoTimeRemaining() const                        { return m_uiDemoTimeRemaining; }
    GAME_SHARED_API void    SetDemoTimeRemaining(uint uiValue)                  { m_uiDemoTimeRemaining = uiValue; }
};
//=============================================================================

#endif //__C_ENTITYCLIENTINFO_H__
