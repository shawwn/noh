// (C)2008 S2 Games
// c_gamestats.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "hon_shared_common.h"

#include "c_gamestats.h"

#include "../k2/c_packet.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
uint        CGameStats::s_uiBaseType(ENTITY_BASE_TYPE_GAME_STATS);

DEFINE_ENT_ALLOCATOR3(GameStats, Info_Stats)

DEFINE_ENTITY_DESC(CGameStats, 3)
{
    s_cDesc.pFieldTypes = K2_NEW(ctx_Game,    TypeVector)();
    s_cDesc.pFieldTypes->clear();
    const TypeVector &vBase(IGameEntity::GetTypeVector());
    s_cDesc.pFieldTypes->insert(s_cDesc.pFieldTypes->begin(), vBase.begin(), vBase.end());

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiHeroDamage"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiHeroKills"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiHeroAssists"), TYPE_INT, 32, 0));

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiCreepDamage"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiCreepKills"), TYPE_INT, 32, 0));

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiNeutralDamage"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiNeutralKills"), TYPE_INT, 32, 0));

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiBuildingDamage"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiBuildingKills"), TYPE_INT, 32, 0));

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiDenies"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiDeniedExperience"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiExperience"), TYPE_INT, 32, 0));

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiHeroBounty"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiCreepBounty"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiNeutralBounty"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiBuildingBounty"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiGoldEarned"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiGoldSpent"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiGoldLost"), TYPE_INT, 32, 0));

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiHeroExperience"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiCreepExperience"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiNeutralExperience"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiBuildingExperience"), TYPE_INT, 32, 0));

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiDeaths"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiTimeDead"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiTimePlayed"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiBuyBacks"), TYPE_INT, 32, 0));

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiActionCount"), TYPE_INT, 32, 0));

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiWardsPurchased"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiConsumablesPurchased"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiWardsPlaced"), TYPE_INT, 32, 0));
}
//=============================================================================

/*====================
  CGameStats::CGameStats
  ====================*/
CGameStats::CGameStats() :
IGameEntity(nullptr),
m_uiHeroDamage(0),
m_uiHeroKills(0),
m_uiHeroAssists(0),

m_uiCreepDamage(0),
m_uiCreepKills(0),

m_uiNeutralDamage(0),
m_uiNeutralKills(0),

m_uiBuildingDamage(0),
m_uiBuildingKills(0),

m_uiDenies(0),
m_uiDeniedExperience(0),
m_uiExperience(0),

m_uiHeroBounty(0),
m_uiCreepBounty(0),
m_uiNeutralBounty(0),
m_uiBuildingBounty(0),
m_uiGoldEarned(0),
m_uiGoldSpent(0),
m_uiGoldLost(0),

m_uiHeroExperience(0),
m_uiCreepExperience(0),
m_uiNeutralExperience(0),
m_uiBuildingExperience(0),

m_uiDeaths(0),
m_uiTimeDead(0),
m_uiTimePlayed(0),
m_uiBuyBacks(0),

m_uiActionCount(0),

m_uiWardsPurchased(0),
m_uiConsumablesPurchased(0),
m_uiWardsPlaced(0),

m_uiConcedeCalls(0),

m_uiKillStreak(0),
m_uiMultiKillX2(0),
m_uiMultiKillX3(0),
m_uiMultiKillX4(0),
m_uiMultiKillX5(0),

m_uiClientID(-1)
{
}


/*====================
  CGameStats::GetSnapshot
  ====================*/
void    CGameStats::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
    IGameEntity::GetSnapshot(snapshot, uiFlags);

    snapshot.WriteField(m_uiHeroDamage);
    snapshot.WriteField(m_uiHeroKills);
    snapshot.WriteField(m_uiHeroAssists);

    snapshot.WriteField(m_uiCreepDamage);
    snapshot.WriteField(m_uiCreepKills);

    snapshot.WriteField(m_uiNeutralDamage);
    snapshot.WriteField(m_uiNeutralKills);

    snapshot.WriteField(m_uiBuildingDamage);
    snapshot.WriteField(m_uiBuildingKills);

    snapshot.WriteField(m_uiDenies);
    snapshot.WriteField(m_uiDeniedExperience);
    snapshot.WriteField(m_uiExperience);

    snapshot.WriteField(m_uiHeroBounty);
    snapshot.WriteField(m_uiCreepBounty);
    snapshot.WriteField(m_uiNeutralBounty);
    snapshot.WriteField(m_uiBuildingBounty);
    snapshot.WriteField(m_uiGoldEarned);
    snapshot.WriteField(m_uiGoldSpent);
    snapshot.WriteField(m_uiGoldLost);

    snapshot.WriteField(m_uiHeroExperience);
    snapshot.WriteField(m_uiCreepExperience);
    snapshot.WriteField(m_uiNeutralExperience);
    snapshot.WriteField(m_uiBuildingExperience);

    snapshot.WriteField(m_uiDeaths);
    snapshot.WriteField(m_uiTimeDead);
    snapshot.WriteField(m_uiTimePlayed);
    snapshot.WriteField(m_uiBuyBacks);

    snapshot.WriteField(m_uiActionCount);

    snapshot.WriteField(m_uiWardsPurchased);
    snapshot.WriteField(m_uiConsumablesPurchased);
    snapshot.WriteField(m_uiWardsPlaced);
}


/*====================
  CGameStats::ReadSnapshot
  ====================*/
bool    CGameStats::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
    try
    {
        IGameEntity::ReadSnapshot(snapshot, 1);

        snapshot.ReadField(m_uiHeroDamage);
        snapshot.ReadField(m_uiHeroKills);
        snapshot.ReadField(m_uiHeroAssists);

        snapshot.ReadField(m_uiCreepDamage);
        snapshot.ReadField(m_uiCreepKills);

        snapshot.ReadField(m_uiNeutralDamage);
        snapshot.ReadField(m_uiNeutralKills);

        snapshot.ReadField(m_uiBuildingDamage);
        snapshot.ReadField(m_uiBuildingKills);

        snapshot.ReadField(m_uiDenies);
        snapshot.ReadField(m_uiDeniedExperience);
        snapshot.ReadField(m_uiExperience);

        snapshot.ReadField(m_uiHeroBounty);
        snapshot.ReadField(m_uiCreepBounty);
        snapshot.ReadField(m_uiNeutralBounty);
        snapshot.ReadField(m_uiBuildingBounty);
        snapshot.ReadField(m_uiGoldEarned);
        snapshot.ReadField(m_uiGoldSpent);
        snapshot.ReadField(m_uiGoldLost);

        snapshot.ReadField(m_uiHeroExperience);
        snapshot.ReadField(m_uiCreepExperience);
        snapshot.ReadField(m_uiNeutralExperience);
        snapshot.ReadField(m_uiBuildingExperience);

        snapshot.ReadField(m_uiDeaths);
        snapshot.ReadField(m_uiTimeDead);
        snapshot.ReadField(m_uiTimePlayed);
        snapshot.ReadField(m_uiBuyBacks);

        snapshot.ReadField(m_uiActionCount);

        if (uiVersion >= 2)
        {
            snapshot.ReadField(m_uiWardsPurchased);
            snapshot.ReadField(m_uiConsumablesPurchased);

            if (uiVersion >= 3)
            {
                snapshot.ReadField(m_uiWardsPlaced);
            }
        }

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CGameStats::ReadSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  CGameStats::Baseline
  ====================*/
void    CGameStats::Baseline()
{
    IGameEntity::Baseline();

    m_uiHeroDamage = 0;
    m_uiHeroKills = 0;
    m_uiHeroAssists = 0;

    m_uiCreepDamage = 0;
    m_uiCreepKills = 0;

    m_uiNeutralDamage = 0;
    m_uiNeutralKills = 0;

    m_uiBuildingDamage = 0;
    m_uiBuildingKills = 0;

    m_uiDenies = 0;
    m_uiDeniedExperience = 0;
    m_uiExperience = 0;

    m_uiHeroBounty = 0;
    m_uiCreepBounty = 0;
    m_uiNeutralBounty = 0;
    m_uiBuildingBounty = 0;
    m_uiGoldEarned = 0;
    m_uiGoldSpent = 0;
    m_uiGoldLost = 0;

    m_uiHeroExperience = 0;
    m_uiCreepExperience = 0;
    m_uiNeutralExperience = 0;
    m_uiBuildingExperience = 0;

    m_uiDeaths = 0;
    m_uiTimeDead = 0;
    m_uiTimePlayed = 0;
    m_uiBuyBacks = 0;

    m_uiActionCount = 0;

    m_uiWardsPurchased = 0;
    m_uiConsumablesPurchased = 0;
    m_uiWardsPlaced = 0;

    m_uiClientID = -1;
}


/*====================
  CGameStats::GetRivalLogCount
  ====================*/
const byte  CGameStats::GetRivalLogCount()
{
    if (m_vRivals.size() == 0) 
        return byte(0); 

    return m_vRivals.back().yCount;
}


/*====================
  CGameStats::GetExtendedData
  ====================*/
void    CGameStats::GetExtendedData(IBuffer &buffer) const
{
    // Ability upgrade history
    buffer << byte(m_vAbilityUpgrades.size());
    for (AbilityUpgradeLog_cit it(m_vAbilityUpgrades.begin()); it != m_vAbilityUpgrades.end(); ++it)
        buffer << it->uiTimeStamp << it->unAbilityTypeID << it->yLevel << it->ySlot;

    // Kill history
    buffer << byte(m_vKills.size());
    for (HeroKillLog_cit it(m_vKills.begin()); it != m_vKills.end(); ++it)
    {
        buffer << it->uiTimeStamp << it->iVictim << byte(it->vAssists.size());
        for (ivector_cit itAssist(it->vAssists.begin()); itAssist != it->vAssists.end(); ++itAssist)
            buffer << *itAssist;
    }

    // Assist history
    buffer << byte(m_vAssists.size());
    for (KillLogVector_cit it(m_vAssists.begin()); it != m_vAssists.end(); ++it)
        buffer << it->first << it->second;

    // Death history
    buffer << byte(m_vDeaths.size());
    for (KillLogVector_cit it(m_vDeaths.begin()); it != m_vDeaths.end(); ++it)
        buffer << it->first << it->second;

    // Item history
    buffer << byte(m_vItemHistory.size());
    for (ItemHistoryLog_cit it(m_vItemHistory.begin()); it != m_vItemHistory.end(); ++it)
        buffer << it->uiTimeStamp << it->unItemTypeID << it->yAction;

    buffer << m_uiClientID;

        //ToDo enable these to the ex data.
    /*buffer << byte(m_vSmackdowns.size());
    for (SmackdownLog_cit it(m_vSmackdowns.begin()); it != m_vSmackdowns.end(); ++it)
        buffer << it->uiTimeStamp << it->iVictim << it->ySlot;

    buffer << byte(m_vHumiliations.size());
    for (HumiliationLog_cit it(m_vHumiliations.begin()); it != m_vHumiliations.end(); ++it)
        buffer << it->uiTimeStamp << it->iVictim << it->ySlot;

    buffer << byte(m_vRivals.size());
    for (RivalLog_cit it(m_vRivals.begin()); it != m_vRivals.end(); ++it)
        buffer << it->uiTimeStamp << it->iVictim << it->ySlot;

    buffer << byte(m_vPaybacks.size());
    for (PaybackLog_cit it(m_vPaybacks.begin()); it != m_vPaybacks.end(); ++it)
        buffer << it->uiTimeStamp << it->iVictim << it->ySlot;

    buffer << m_FirstBloodEntry.uiTimeStamp << m_FirstBloodEntry.iVictim << m_FirstBloodEntry.ySlot << (byte)m_FirstBloodEntry.bFirstBlood;*/
}


/*====================
  CGameStats::ReadExtendedData
  ====================*/
void    CGameStats::ReadExtendedData(CPacket &pkt)
{
    IGameEntity::ReadExtendedData(pkt);

    // Ability upgrades
    m_vAbilityUpgrades.clear();
    byte yNumAbilityEntries(pkt.ReadByte());
    for (byte y(0); y < yNumAbilityEntries; ++y)
    {
        uint uiTimeStamp(pkt.ReadInt());
        ushort unAbilityTypeID(pkt.ReadShort());
        byte yLevel(pkt.ReadByte());
        byte ySlot(pkt.ReadByte());
        m_vAbilityUpgrades.push_back(SAbilityUpgradeEntry(uiTimeStamp, unAbilityTypeID, yLevel, ySlot));
    }

    // Kills
    m_vKills.clear();
    byte yNumKillEntries(pkt.ReadByte());
    for (byte y(0); y < yNumKillEntries; ++y)
    {
        uint uiTimeStamp(pkt.ReadInt());
        int iClient(pkt.ReadInt());
        byte yAssists(pkt.ReadByte());
        
        ivector vAssists;
        for (byte yAssist(0); yAssist < yAssists; ++yAssist)
            vAssists.push_back(pkt.ReadInt());

        SHeroKillHistoryEvent entry(uiTimeStamp, iClient, vAssists);
        m_vKills.push_back(entry);
    }

    // Assists
    m_vAssists.clear();
    byte yNumAssistEntries(pkt.ReadByte());
    for (byte y(0); y < yNumAssistEntries; ++y)
    {
        uint uiTimeStamp(pkt.ReadInt());
        int iClient(pkt.ReadInt());
        m_vAssists.push_back(KillLogEvent(uiTimeStamp, iClient));
    }

    // Deaths
    m_vDeaths.clear();
    byte yNumDeathEntries(pkt.ReadByte());
    for (byte y(0); y < yNumDeathEntries; ++y)
    {
        uint uiTimeStamp(pkt.ReadInt());
        int iClient(pkt.ReadInt());
        m_vDeaths.push_back(KillLogEvent(uiTimeStamp, iClient));
    }

    // Item history
    m_vItemHistory.clear();
    byte yNumItemHistoryEntries(pkt.ReadByte());
    for (byte y(0); y < yNumItemHistoryEntries; ++y)
    {
        uint uiTimeStamp(pkt.ReadInt());
        ushort unItemTypeID(pkt.ReadShort());
        byte yAction(pkt.ReadByte());
        m_vItemHistory.push_back(SItemHistoryEntry(uiTimeStamp, unItemTypeID, yAction));
    }

    m_uiClientID = pkt.ReadInt();

        //ToDo Enable these to the ex data.
    m_vSmackdowns.clear();
    /*byte yNumSmackdownEntries(pkt.ReadByte());
    for (byte y(0); y < yNumSmackdownEntries; ++y)
    {
        uint uiTimeStamp(pkt.ReadInt());
        int iVictim(pkt.ReadShort());
        byte ySlot(pkt.ReadByte());
        m_vSmackdowns.push_back(SSmackdownEntry(uiTimeStamp, iVictim, ySlot));
    }*/

    m_vHumiliations.clear();
    /*byte yNumHumiliationEntries(pkt.ReadByte());
    for (byte y(0); y < yNumHumiliationEntries; ++y)
    {
        uint uiTimeStamp(pkt.ReadInt());
        int iVictim(pkt.ReadShort());
        byte ySlot(pkt.ReadByte());
        m_vHumiliations.push_back(SHumiliationEntry(uiTimeStamp, iVictim, ySlot));
    }*/

    m_vRivals.clear();
    /*byte yNumRivalEntries(pkt.ReadByte());
    for (byte y(0); y < yNumRivalEntries; ++y)
    {
        uint uiTimeStamp(pkt.ReadInt());
        int iVictim(pkt.ReadShort());
        byte ySlot(pkt.ReadByte());
        m_vRivals.push_back(SRivalEntry(uiTimeStamp, iVictim, ySlot));
    }*/

    m_vPaybacks.clear();
    /*byte yNumPaybackEntries(pkt.ReadByte());
    for (byte y(0); y < yNumPaybackEntries; ++y)
    {
        uint uiTimeStamp(pkt.ReadInt());
        int iVictim(pkt.ReadShort());
        byte ySlot(pkt.ReadByte());
        m_vPaybacks.push_back(SPaybackEntry(uiTimeStamp, iVictim, ySlot));
    }*/
    
    /*uint uiTimeStamp(pkt.ReadInt());
    int iVictim(pkt.ReadShort());
    byte ySlot(pkt.ReadByte());
    bool bFirstBlood(pkt.ReadByte());

    m_FirstBloodEntry = SFirstBloodEntry(uiTimeStamp, iVictim, ySlot, bFirstBlood);*/
    m_FirstBloodEntry = SFirstBloodEntry();
}
