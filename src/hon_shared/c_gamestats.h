// (C)2008 S2 Games
// c_gamestats.h
//
//=============================================================================
#ifndef __C_GAMESTATS_H__
#define __C_GAMESTATS_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gameentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define GAME_STAT(name) \
private: \
    uint    m_ui##name; \
public: \
    uint    Get##name() const           { return m_ui##name; } \
    void    Set##name(uint ui##name)    { m_ui##name = ui##name; } \
    void    Clear##name()               { m_ui##name = 0; } \
    void    Add##name(uint ui##name)    { m_ui##name += ui##name; }

enum EItemHistoryAction
{
    ITEM_HISTORY_TRANSFER,
    ITEM_HISTORY_PURCHASE,
    ITEM_HISTORY_ASSEMBLE,
    ITEM_HISTORY_SELL,
    ITEM_HISTORY_DISASSEMBLE,
    ITEM_HISTORY_DROP,
    ITEM_HISTORY_PICKUP
};
//=============================================================================

//=============================================================================
// CGameStats
//=============================================================================
class CGameStats : public IGameEntity
{
    DECLARE_ENTITY_DESC

private:
    DECLARE_ENT_ALLOCATOR3(GameStats);

    GAME_STAT(HeroDamage)
    GAME_STAT(HeroKills)
    GAME_STAT(HeroAssists)
    GAME_STAT(HeroExperience)

    GAME_STAT(CreepDamage)
    GAME_STAT(CreepKills)
    GAME_STAT(CreepExperience)

    GAME_STAT(NeutralDamage)
    GAME_STAT(NeutralKills)
    GAME_STAT(NeutralExperience)

    GAME_STAT(BuildingDamage)
    GAME_STAT(BuildingKills)
    GAME_STAT(BuildingExperience)

    GAME_STAT(Denies)
    GAME_STAT(DeniedExperience)
    GAME_STAT(Experience)

    GAME_STAT(HeroBounty)
    GAME_STAT(CreepBounty)
    GAME_STAT(NeutralBounty)
    GAME_STAT(BuildingBounty)
    GAME_STAT(GoldEarned)
    GAME_STAT(GoldSpent)
    GAME_STAT(GoldLost)

    GAME_STAT(Deaths)
    GAME_STAT(TimeDead)
    GAME_STAT(TimePlayed)
    GAME_STAT(BuyBacks)

    GAME_STAT(ActionCount)

    GAME_STAT(WardsPurchased)
    GAME_STAT(ConsumablesPurchased)

    GAME_STAT(WardsPlaced)

    GAME_STAT(ConcedeCalls)

    GAME_STAT(KillStreak)
    GAME_STAT(MultiKillX2)
    GAME_STAT(MultiKillX3)
    GAME_STAT(MultiKillX4)
    GAME_STAT(MultiKillX5)

    struct SSmackdownEntry
    {
        uint    uiTimeStamp;
        int     iVictim;
        byte    ySlot;

        SSmackdownEntry(uint _uiTimeStamp, byte _ySlot, int _iVictim) :
        uiTimeStamp(_uiTimeStamp),
        iVictim(_iVictim),
        ySlot(_ySlot)
        {}
    };
    typedef vector<SSmackdownEntry>                 SmackdownLog;
    typedef SmackdownLog::iterator                  SmackdownLog_it;
    typedef SmackdownLog::const_iterator            SmackdownLog_cit;

    SmackdownLog    m_vSmackdowns;

    struct SHumiliationEntry
    {
        uint    uiTimeStamp;
        int     iVictim;
        byte    ySlot;

        SHumiliationEntry(uint _uiTimeStamp, byte _ySlot, int _iVictim) :
        uiTimeStamp(_uiTimeStamp),
        iVictim(_iVictim),
        ySlot(_ySlot)
        {}
    };
    typedef vector<SHumiliationEntry>               HumiliationLog;
    typedef HumiliationLog::iterator                HumiliationLog_it;
    typedef HumiliationLog::const_iterator          HumiliationLog_cit;

    HumiliationLog  m_vHumiliations;

    struct SRivalEntry
    {
        uint    uiTimeStamp;
        int     iVictim;
        byte    ySlot;
        byte    yCount;

        SRivalEntry(uint _uiTimeStamp, byte _ySlot, int _iVictim, byte _yCount) :
        uiTimeStamp(_uiTimeStamp),
        iVictim(_iVictim),
        ySlot(_ySlot),
        yCount(_yCount)
        {}
    };
    typedef vector<SRivalEntry>             RivalLog;
    typedef RivalLog::iterator              RivalLog_it;
    typedef RivalLog::const_iterator        RivalLog_cit;

    RivalLog    m_vRivals;

    struct SPaybackEntry
    {
        uint    uiTimeStamp;
        int     iVictim;
        byte    ySlot;

        SPaybackEntry(uint _uiTimeStamp, byte _ySlot, int _iVictim) :
        uiTimeStamp(_uiTimeStamp),
        iVictim(_iVictim),
        ySlot(_ySlot)
        {}
    };
    typedef vector<SPaybackEntry>               PaybackLog;
    typedef PaybackLog::iterator                PaybackLog_it;
    typedef PaybackLog::const_iterator          PaybackLog_cit;

    PaybackLog  m_vPaybacks;

    struct SFirstBloodEntry
    {
        uint    uiTimeStamp;
        int     iVictim;
        byte    ySlot;
        bool    bFirstBlood;

        SFirstBloodEntry() :
        uiTimeStamp(0),
        iVictim(0),
        ySlot(0),
        bFirstBlood(false)
        {
        }

        SFirstBloodEntry(uint _uiTimeStamp, byte _ySlot, int _iVictim, bool _bFirstBlood = true) :
        uiTimeStamp(_uiTimeStamp),
        iVictim(_iVictim),
        ySlot(_ySlot),
        bFirstBlood(_bFirstBlood)
        {}
    };

    SFirstBloodEntry        m_FirstBloodEntry;

    struct SAbilityUpgradeEntry
    {
        uint    uiTimeStamp;
        ushort  unAbilityTypeID;
        byte    yLevel;
        byte    ySlot;

        SAbilityUpgradeEntry(uint _uiTimeStamp, ushort _unAbilityTypeID, byte _yLevel, byte _ySlot) :
        uiTimeStamp(_uiTimeStamp),
        unAbilityTypeID(_unAbilityTypeID),
        yLevel(_yLevel),
        ySlot(_ySlot)
        {}
    };
    typedef vector<SAbilityUpgradeEntry>        AbilityUpgradeLog;
    typedef AbilityUpgradeLog::iterator         AbilityUpgradeLog_it;
    typedef AbilityUpgradeLog::const_iterator   AbilityUpgradeLog_cit;

    AbilityUpgradeLog   m_vAbilityUpgrades;
    
    struct SItemHistoryEntry
    {
        uint    uiTimeStamp;
        ushort  unItemTypeID;
        byte    yAction;

        SItemHistoryEntry(uint _uiTimeStamp, ushort _unItemTypeID, byte _yAction) :
        uiTimeStamp(_uiTimeStamp),
        unItemTypeID(_unItemTypeID),
        yAction(_yAction)
        {}
    };
    typedef vector<SItemHistoryEntry>       ItemHistoryLog;
    typedef ItemHistoryLog::iterator        ItemHistoryLog_it;
    typedef ItemHistoryLog::const_iterator  ItemHistoryLog_cit;

    ItemHistoryLog  m_vItemHistory;

    struct SHeroKillHistoryEvent
    {
        uint    uiTimeStamp;
        int     iVictim;
        ivector vAssists;

        SHeroKillHistoryEvent(uint _uiTimeStamp, int _iVictim, const ivector &_vAssists) :
        uiTimeStamp(_uiTimeStamp),
        iVictim(_iVictim),
        vAssists(_vAssists)
        {}

    };
    typedef vector<SHeroKillHistoryEvent>   HeroKillLog;
    typedef HeroKillLog::iterator           HeroKillLog_it;
    typedef HeroKillLog::const_iterator     HeroKillLog_cit;

    HeroKillLog m_vKills;

    typedef pair<uint, int>                 KillLogEvent;
    typedef vector<KillLogEvent>            KillLogVector;
    typedef KillLogVector::iterator         KillLogVector_it;
    typedef KillLogVector::const_iterator   KillLogVector_cit;

    KillLogVector   m_vDeaths;
    KillLogVector   m_vAssists;

    uint            m_uiClientID;

protected:

public:
    ~CGameStats()   {}
    CGameStats();

    SUB_ENTITY_ACCESSOR(CGameStats, Stats)

    virtual bool    IsServerEntity() const              { return true; }

    // Network
    virtual void    Baseline();
    virtual void    GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
    virtual bool    ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);
    
    void                            LogAbilityUpgrade(SAbilityUpgradeEntry &entry)  { m_vAbilityUpgrades.push_back(entry); }
    const AbilityUpgradeLog&        GetAbilityUpgradeLog() const                    { return m_vAbilityUpgrades; }

    void                            LogSmackdown(SSmackdownEntry &entry)                { m_vSmackdowns.push_back(entry); }
    const SmackdownLog&             GetSmackdownLog() const                             { return m_vSmackdowns; }

    void                            LogHumiliation(SHumiliationEntry &entry)            { m_vHumiliations.push_back(entry); }
    const HumiliationLog&           GetHumiliationLog() const                           { return m_vHumiliations; }

    void                            LogRival(SRivalEntry &entry)                { m_vRivals.push_back(entry); }
    const RivalLog&                 GetRivalLog() const                         { return m_vRivals; }
    GAME_SHARED_API const byte      GetRivalLogCount();

    void                            LogPayback(SPaybackEntry &entry)                    { m_vPaybacks.push_back(entry); }
    const PaybackLog&               GetPaybackLog() const                               { return m_vPaybacks; }

    void                            LogKill(const SHeroKillHistoryEvent &entry)         { m_vKills.push_back(entry); }
    const HeroKillLog&              GetKillLog() const                                  { return m_vKills; }

    void                            LogDeath(KillLogEvent entry)                        { m_vDeaths.push_back(entry); }
    const KillLogVector&            GetDeathLog() const                                 { return m_vDeaths; }

    void                            LogAssist(KillLogEvent entry)                       { m_vAssists.push_back(entry); }
    const KillLogVector&            GetAssistLog() const                                { return m_vAssists; }

    void                            LogItemHistory(SItemHistoryEntry &entry)            { m_vItemHistory.push_back(entry); }
    const ItemHistoryLog&           GetItemHistoryLog() const                           { return m_vItemHistory; }

    void                            LogFirstBlood(SFirstBloodEntry &entry)              { m_FirstBloodEntry = entry; }
    const SFirstBloodEntry&         GetFirstBloodEntry() const                          { return m_FirstBloodEntry; }

    GAME_SHARED_API void            GetExtendedData(IBuffer &buffer) const;
    GAME_SHARED_API void            ReadExtendedData(CPacket &pkt);

    void                            SetPlayerClientID(uint uiID)                        { m_uiClientID = uiID; }
    GAME_SHARED_API uint            GetPlayerClientID() const                           { return m_uiClientID; }
};
//=============================================================================

#endif //__C_GAMESTATS_H__
