// (C)2006 S2 Games
// c_gameinterfacemanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_client_common.h"

#include "c_gameinterfacemanager.h"
#include "c_clientcommander.h"

#include "../game_shared/c_teaminfo.h"
#include "../game_shared/c_teamdefinition.h"
#include "../game_shared/c_replaymanager.h"
#include "../game_shared/c_entitychest.h"
#include "../game_shared/c_entityclientinfo.h"

#include "../k2/c_uimanager.h"
#include "../k2/c_uitrigger.h"
#include "../k2/c_uitriggerregistry.h"
#include "../k2/c_input.h"
#include "../k2/c_camera.h"
#include "../k2/c_statestring.h"
#include "../k2/c_function.h"
#include "../k2/c_vid.h"
#include "../k2/c_eventmanager.h"
#include "../k2/c_uicmd.h"
#include "../k2/c_interface.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_BOOL   (cg_constrainCursor,            false);
CVAR_STRINGF(cg_lobbyPhaseTimer,            "",     CVAR_READONLY);
CVAR_BOOL   (ui_forceUpdate,                false);

// TODO: Properly integrate these
UI_TRIGGER(HoverInfoPlayer);
UI_TRIGGER(HoverInfoBuilding);
UI_TRIGGER(HoverInfoGadget);
UI_TRIGGER(HoverInfoNpc);
UI_TRIGGER(HoverInfoChest);
UI_TRIGGER(CommSelectionPlayer);
UI_TRIGGER(CommSelectionBuilding);
UI_TRIGGER(CommSelectionGadget);
UI_TRIGGER(CommSelectionNpc);
UI_TRIGGER(CommSelectionChest);
UI_TRIGGER(OfficerInfo);

CVAR_UINTF  (cg_endGameInterfaceDelay,      8000,   CVAR_GAMECONFIG);
//=============================================================================

/*====================
  CompareByStatValuesAsc
  ====================*/
template<class T>
static bool CompareByStatValuesAsc(const pair<int, T> elem1, const pair<int, T> elem2)
{
    return elem1.second < elem2.second;
}


/*====================
  CompareByStatValuesDesc
  ====================*/
template <class T>
static bool CompareByStatValuesDesc(const pair<int, T> elem1, const pair<int, T> elem2)
{
    return elem1.second > elem2.second;
}

/*====================
  CompareByExperience
  ====================*/
static bool CompareByExperience(const CEntityClientInfo *elem1, const CEntityClientInfo *elem2)
{
    return elem1->GetExperience() > elem2->GetExperience();
}

/*====================
  CSmartGameUITrigger::~CSmartGameUITrigger
  ====================*/
CSmartGameUITrigger::~CSmartGameUITrigger()
{
    for (uint ui(0); ui < m_uiCount * m_uiSubCount; ++ui)
    {
        if (m_vTriggerOwner[ui])
            SAFE_DELETE(m_vTriggers[ui]);
    }
}


/*====================
  CSmartGameUITrigger::CSmartGameUITrigger
  ====================*/
CSmartGameUITrigger::CSmartGameUITrigger(const tstring &sName, uint uiCount, uint uiSubCount) :
m_uiCount(uiCount),
m_uiSubCount(uiSubCount),
m_vTriggers(uiCount * uiSubCount, NULL),
m_vTriggerOwner(uiCount * uiSubCount, false),
m_vbValue(uiCount * uiSubCount, false),
m_vunValue(uiCount * uiSubCount, 0),
m_vuiValue(uiCount * uiSubCount, 0),
m_vfValue(uiCount * uiSubCount, 0.0f),
m_vsValue(uiCount * uiSubCount, SNULL),
m_vvValue(uiCount * uiSubCount),
m_vUpdateSequence(uiCount * uiSubCount, -1)
{
    assert((m_uiCount * m_uiSubCount) > 0);

    for (uint ui(0); ui < m_uiCount; ++ui)
    {
        for (uint uiSub(0); uiSub < uiSubCount; ++uiSub)
        {
            uint uiIndex((ui * m_uiSubCount) + uiSub);
            tstring sNameIndex(sName + ((uiCount > 1 || (uiSubCount > 1)) ? XtoA(ui) : _T("")) + ((uiSubCount > 1) ? XtoA(uiSub) : _T("")));
            m_vTriggers[uiIndex] = UITriggerRegistry.GetUITrigger(sNameIndex);
            if (m_vTriggers[uiIndex] == NULL)
            {
                m_vTriggers[uiIndex] = K2_NEW(MemManager.GetHeap(HEAP_CLIENT_GAME),   CUITrigger)(sNameIndex);
                m_vTriggerOwner[uiIndex] = true;
            }
        }
    }
}


/*====================
  CSmartGameUITrigger::Trigger
  ====================*/
#define TRIGGERFN(type, pre) \
void    CSmartGameUITrigger::Trigger(type pre##Value, uint uiIndex, uint uiSubIndex, uint uiUpdateSequence) \
{ \
    uint uiAbsoluteIndex(uiSubIndex + (uiIndex * m_uiSubCount)); \
    if (uiAbsoluteIndex >= m_vTriggers.size() || m_vTriggers[uiAbsoluteIndex] == NULL) \
        return; \
\
    if (m_v##pre##Value[uiAbsoluteIndex] == pre##Value && m_vUpdateSequence[uiAbsoluteIndex] == uiUpdateSequence) \
        return; \
\
    m_v##pre##Value[uiAbsoluteIndex] = pre##Value; \
    m_vTriggers[uiAbsoluteIndex]->Trigger(XtoA(pre##Value)); \
    m_vUpdateSequence[uiAbsoluteIndex] = uiUpdateSequence; \
}

TRIGGERFN(bool, b)
TRIGGERFN(short, un)
TRIGGERFN(ushort, un)
TRIGGERFN(int, ui)
TRIGGERFN(uint, ui)
TRIGGERFN(float, f)
TRIGGERFN(const tstring&, s)
#undef TRIGGERFN

void    CSmartGameUITrigger::Trigger(const svector& vValue, uint uiIndex, uint uiSubIndex, uint uiUpdateSequence)
{
    uint uiAbsoluteIndex(uiSubIndex + (uiIndex * m_uiSubCount));
    if (uiAbsoluteIndex >= m_vTriggers.size() || m_vTriggers[uiAbsoluteIndex] == NULL)
        return;

    if (m_vvValue[uiAbsoluteIndex] == vValue && m_vUpdateSequence[uiAbsoluteIndex] == uiUpdateSequence)
        return;

    m_vvValue[uiAbsoluteIndex] = vValue;
    m_vTriggers[uiAbsoluteIndex]->Trigger(vValue);
    m_vUpdateSequence[uiAbsoluteIndex] = uiUpdateSequence;
}


/*====================
  CSmartGameUITrigger::Execute
  ====================*/
void    CSmartGameUITrigger::Execute(const tstring &sScript, uint uiIndex, uint uiSubIndex)
{
    uint uiAbsoluteIndex(uiSubIndex + (uiIndex * m_uiSubCount));
    if (uiAbsoluteIndex >= m_vTriggers.size() || m_vTriggers[uiAbsoluteIndex] == NULL)
        return;

    m_vTriggers[uiAbsoluteIndex]->Execute(sScript);
}


/*====================
  CSmartGameUITrigger::Show
  ====================*/
void    CSmartGameUITrigger::Show(uint uiIndex, uint uiSubIndex)
{
    uint uiAbsoluteIndex(uiSubIndex + (uiIndex * m_uiSubCount));
    if (uiAbsoluteIndex >= m_vTriggers.size() || m_vTriggers[uiAbsoluteIndex] == NULL)
        return;

    m_vTriggers[uiAbsoluteIndex]->Show();
}


/*====================
  CSmartGameUITrigger::Hide
  ====================*/
void    CSmartGameUITrigger::Hide(uint uiIndex, uint uiSubIndex)
{
    uint uiAbsoluteIndex(uiSubIndex + (uiIndex * m_uiSubCount));
    if (uiAbsoluteIndex >= m_vTriggers.size() || m_vTriggers[uiAbsoluteIndex] == NULL)
        return;

    m_vTriggers[uiAbsoluteIndex]->Hide();
}


/*====================
  CGameInterfaceManager::~CGameInterfaceManager
  ====================*/
CGameInterfaceManager::~CGameInterfaceManager()
{
    UIManager.SetOverlayInterface(_T(""));

    for (int i(0); i < NUM_UITRIGGERS; ++i)
        SAFE_DELETE(m_vTriggers[i]);

    // HACK: These should be tracked by their resource handle
    // TODO: Add a generic flag for resources to automatically unload
    // when the module that loaded them is shutdown
    UIManager.UnloadInterface(_T("game"));
    UIManager.UnloadInterface(_T("game_dead"));
    UIManager.UnloadInterface(_T("game_info"));
    UIManager.UnloadInterface(_T("game_loadout"));
    UIManager.UnloadInterface(_T("game_commander"));
    UIManager.UnloadInterface(_T("game_lobby"));
    UIManager.UnloadInterface(_T("game_spawn"));
    UIManager.UnloadInterface(_T("game_sacrificed"));
    UIManager.UnloadInterface(_T("game_player_build"));
    UIManager.UnloadInterface(_T("game_end_stats"));
    //UIManager.UnloadInterface(_T("game_menu")); // Stop crashing please :(
    UIManager.UnloadInterface(_T("game_observer"));
    //UIManager.UnloadInterface(_T("game_replay_control")); :)
    UIManager.UnloadInterface(_T("game_score_overlay"));
    UIManager.UnloadInterface(_T("game_standby"));
}


/*====================
  CGameInterfaceManager::CGameInterfaceManager
  ====================*/
CGameInterfaceManager::CGameInterfaceManager() :
m_vTriggers(NUM_UITRIGGERS, NULL),
m_uiUpdateSequence(0),
m_bCursor(false),
m_uiCursorUpdateSequence(0),
m_eCurrentInterface(CG_INTERFACE_NONE),
m_uiLastBuildingAttackAlertTime(0),
m_bShowScoreOverlay(false),

m_CanBuild(_T("CanBuild"))
{
    AddTrigger(UITRIGGER_GAME_TIP_VISIBLE, _T("GameTipVisible"));
    AddTrigger(UITRIGGER_GAME_TIP_TEXT, _T("GameTipText"));

    AddTrigger(UITRIGGER_COMMANDER_ORDERTIPVISIBLE, _T("OrderTipVisible"));
    AddTrigger(UITRIGGER_COMMANDER_ORDERTIP, _T("OrderTipText"));
    AddTrigger(UITRIGGER_COMMANDER_ORDERTYPE, _T("CommanderOrderType"));
    AddTrigger(UITRIGGER_COMMANDER_ORDERDIRECTION, _T("CommanderOrderDirection"));

    AddTrigger(UITRIGGER_HEALTH_VALUE, _T("HealthValue"));
    AddTrigger(UITRIGGER_MANA_VALUE, _T("ManaValue"));
    AddTrigger(UITRIGGER_STAMINA_VALUE, _T("StaminaValue"));
    AddTrigger(UITRIGGER_HEALTH_PERCENT, _T("HealthPercent"));
    AddTrigger(UITRIGGER_MANA_PERCENT, _T("ManaPercent"));
    AddTrigger(UITRIGGER_STAMINA_PERCENT, _T("StaminaPercent"));
    AddTrigger(UITRIGGER_SOUL_COUNT, _T("SoulCount"));

    AddTrigger(UITRIGGER_SELECTED_ITEM, _T("SelectedItem"));

    AddTrigger(UITRIGGER_PLAYER_NAME_PERSISTANT, _T("PlayerNamePersistant"));

    AddTrigger(UITRIGGER_HAS_COMMANDER, _T("HasCommander"));

    AddTrigger(UITRIGGER_DASH_REMAINING, _T("DashRemaining"));
    AddTrigger(UITRIGGER_DASH_COOLDOWN, _T("DashCooldown"));

    AddTrigger(UITRIGGER_PERSISTANT_STAT, _T("Persistant"), NUM_PERSISTANT_STATS);
    
    AddTrigger(UITRIGGER_INVENTORY_COUNT, _T("InventoryCount"));
    AddTrigger(UITRIGGER_INVENTORY_EXISTS, _T("InventoryExists"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_INVENTORY_ENTITY, _T("InventoryEntity"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_INVENTORY_ICON, _T("InventoryIcon"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_INVENTORY_TIMER, _T("InventoryTimer"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_INVENTORY_ENABLED, _T("InventoryEnabled"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_INVENTORY_AMMO_PERCENT, _T("InventoryAmmoPercent"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_INVENTORY_AMMO_COUNT, _T("InventoryAmmoCount"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_INVENTORY_LOW_MANA, _T("InventoryLowMana"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_INVENTORY_NO_ACCESS, _T("InventoryNoAccess"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_INVENTORY_DISABLED, _T("InventoryDisabled"), INVENTORY_END_BACKPACK);

    AddTrigger(UITRIGGER_INVENTORY_MELEE, _T("InventoryMelee"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_INVENTORY_RANGED_AMMO, _T("InventoryRangedAmmo"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_INVENTORY_RANGED_MANA, _T("InventoryRangedMana"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_INVENTORY_RANGED_ATTACK_SPEED, _T("InventoryRangedAttackSpeed"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_INVENTORY_ABILITY, _T("InventoryAbility"), INVENTORY_END_BACKPACK);

    AddTrigger(UITRIGGER_INVENTORY_NAME, _T("InventoryName"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_INVENTORY_DESCRIPTION, _T("InventoryDescription"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_INVENTORY_MANA_COST, _T("InventoryManaCost"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_INVENTORY_COOLDOWN, _T("InventoryCooldown"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_INVENTORY_RANGED_DAMAGE, _T("InventoryRangedDamage"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_INVENTORY_AMMO, _T("InventoryAmmo"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_INVENTORY_QUICK_ATTACK_DAMAGE, _T("InventoryQuickAttackDamage"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_INVENTORY_QUICK_ATTACK_SPEED, _T("InventoryQuickAttackSpeed"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_INVENTORY_STRONG_ATTACK_DAMAGE, _T("InventoryStrongAttackDamage"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_INVENTORY_STRONG_ATTACK_SPEED, _T("InventoryStrongAttackSpeed"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_INVENTORY_TYPE, _T("InventoryType"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_INVENTORY_PREREQUISITE, _T("InventoryPrerequisite"), INVENTORY_END_BACKPACK);

    AddTrigger(UITRIGGER_CONSUMABLE_ITEM_ICON, _T("ConsumableItemIcon"), MAX_DISPLAY_ITEM_SHOP);
    AddTrigger(UITRIGGER_CONSUMABLE_ITEM_NAME, _T("ConsumableItemName"), MAX_DISPLAY_ITEM_SHOP);
    AddTrigger(UITRIGGER_CONSUMABLE_ITEM_AVAILABLE, _T("ConsumableItemAvailable"), MAX_DISPLAY_ITEM_SHOP);

    AddTrigger(UITRIGGER_PERSISTANT_VAULT_ICON, _T("PersistantVaultIcon"), MAX_PERSISTANT_ITEMS);

    AddTrigger(UITRIGGER_BUFF_ACTIVE, _T("BuffActive"), MAX_DISPLAY_BUFFS);
    AddTrigger(UITRIGGER_BUFF_ICON, _T("BuffIcon"), MAX_DISPLAY_BUFFS);
    AddTrigger(UITRIGGER_BUFF_TIME, _T("BuffTime"), MAX_DISPLAY_BUFFS);
    AddTrigger(UITRIGGER_BUFF_TIME_PERCENT, _T("BuffTimePercent"), MAX_DISPLAY_BUFFS);

    AddTrigger(UITRIGGER_DEBUFF_ACTIVE, _T("DebuffActive"), MAX_DISPLAY_BUFFS);
    AddTrigger(UITRIGGER_DEBUFF_ICON, _T("DebuffIcon"), MAX_DISPLAY_DEBUFFS);
    AddTrigger(UITRIGGER_DEBUFF_TIME, _T("DebuffTime"), MAX_DISPLAY_BUFFS);
    AddTrigger(UITRIGGER_DEBUFF_TIME_PERCENT, _T("DebuffTimePercent"), MAX_DISPLAY_BUFFS);

    AddTrigger(UITRIGGER_CURSOR, _T("Cursor"));

    AddTrigger(UITRIGGER_LEVEL, _T("Level"));
    AddTrigger(UITRIGGER_EXPERIENCE, _T("Experience"));
    AddTrigger(UITRIGGER_GOLD, _T("Gold"));

    AddTrigger(UITRIGGER_NAME, _T("Name"));
    AddTrigger(UITRIGGER_ENTITY, _T("Entity"));
    AddTrigger(UITRIGGER_CLASS, _T("Class"));
    AddTrigger(UITRIGGER_PLAYER_DESCRIPTION, _T("PlayerDescription"));
    AddTrigger(UITRIGGER_SQUAD_COLOR, _T("SquadColor"));
    AddTrigger(UITRIGGER_SQUAD_COUNT, _T("SquadCount"));
    AddTrigger(UITRIGGER_SQUAD_OFFICER, _T("SquadOfficer"));

    AddTrigger(UITRIGGER_GADGET_ACTIVE, _T("GadgetActive"), MAX_DISPLAY_GADGETS);
    AddTrigger(UITRIGGER_GADGET_ICON, _T("GadgetIcon"), MAX_DISPLAY_GADGETS);
    AddTrigger(UITRIGGER_GADGET_TIMER, _T("GadgetTime"), MAX_DISPLAY_GADGETS);
    AddTrigger(UITRIGGER_GADGET_LIFETIME_PERCENT, _T("GadgetLifetimePercent"), MAX_DISPLAY_GADGETS);
    AddTrigger(UITRIGGER_GADGET_HEALTH_PERCENT, _T("GadgetHealthPercent"), MAX_DISPLAY_GADGETS);
    AddTrigger(UITRIGGER_GADGET_EXPERIENCE, _T("GadgetExperience"), MAX_DISPLAY_GADGETS);
    AddTrigger(UITRIGGER_GADGET_COUNTER_LABEL, _T("GadgetCounterLabel"), MAX_DISPLAY_GADGETS, 3);
    AddTrigger(UITRIGGER_GADGET_COUNTER_VALUE, _T("GadgetCounterValue"), MAX_DISPLAY_GADGETS, 3);
    AddTrigger(UITRIGGER_GADGET_ALERT, _T("GadgetAlert"), MAX_DISPLAY_GADGETS);

    AddTrigger(UITRIGGER_TEAM_GOLD, _T("TeamGold"));
    AddTrigger(UITRIGGER_TEAM_MANA, _T("TeamMana"));
    AddTrigger(UITRIGGER_TEAM_MANA_PERCENT, _T("TeamManaPercent"));
    AddTrigger(UITRIGGER_TEAM_MAX_MANA, _T("TeamMaxMana"));
    AddTrigger(UITRIGGER_TEAM_EXPERIENCE, _T("TeamExperience"), 2);
    AddTrigger(UITRIGGER_INCOME, _T("Income"));
    AddTrigger(UITRIGGER_UPKEEP, _T("Upkeep"));
    AddTrigger(UITRIGGER_TOTAL_UPKEEP, _T("TotalUpkeep"));
    AddTrigger(UITRIGGER_COMMAND_CENTER_HEALTH_PERCENT, _T("CommandCenterHealthPercent"));
    AddTrigger(UITRIGGER_ENEMY_COMMAND_CENTER_HEALTH_PERCENT, _T("EnemyCommandCenterHealthPercent"));
    AddTrigger(UITRIGGER_BUILDING_ATTACK_ALERT, _T("BuildingAttackAlert"));
    AddTrigger(UITRIGGER_SUDDEN_DEATH_ALERT, _T("SuddenDeathAlert"));

    AddTrigger(UITRIGGER_STAT_POINTS, _T("StatPoints"));
    AddTrigger(UITRIGGER_STAT_NAME, _T("StatName"), NUM_PLAYER_ATTRIBUTES);
    AddTrigger(UITRIGGER_STAT_PERCENT, _T("StatPercent"), NUM_PLAYER_ATTRIBUTES);
    AddTrigger(UITRIGGER_STAT_COST, _T("StatCost"), NUM_PLAYER_ATTRIBUTES);
    AddTrigger(UITRIGGER_STAT_BUTTON, _T("StatButton"), NUM_PLAYER_ATTRIBUTES);
    AddTrigger(UITRIGGER_STAT_LEVEL, _T("StatLevel"), NUM_PLAYER_ATTRIBUTES);
    AddTrigger(UITRIGGER_STAT_EFFECT, _T("StatEffect"), NUM_PLAYER_ATTRIBUTES);

    AddTrigger(UITRIGGER_BASE_MAX_HEALTH, _T("BaseMaxHealth"));
    AddTrigger(UITRIGGER_BASE_MAX_MANA, _T("BaseMaxMana"));
    AddTrigger(UITRIGGER_BASE_MAX_STAMINA, _T("BaseMaxStamina"));
    AddTrigger(UITRIGGER_ADJUSTED_MAX_HEALTH, _T("AdjustedMaxHealth"));
    AddTrigger(UITRIGGER_ADJUSTED_MAX_MANA, _T("AdjustedMaxMana"));
    AddTrigger(UITRIGGER_ADJUSTED_MAX_STAMINA, _T("AdjustedMaxStamina"));
    AddTrigger(UITRIGGER_MAX_HEALTH_BONUS, _T("MaxHealthBonus"));
    AddTrigger(UITRIGGER_MAX_MANA_BONUS, _T("MaxManaBonus"));
    AddTrigger(UITRIGGER_MAX_STAMINA_BONUS, _T("MaxStaminaBonus"));

    AddTrigger(UITRIGGER_BASE_HEALTH_REGEN, _T("BaseHealthRegen"));
    AddTrigger(UITRIGGER_BASE_MANA_REGEN, _T("BaseManaRegen"));
    AddTrigger(UITRIGGER_BASE_STAMINA_REGEN, _T("BaseStaminaRegen"));
    AddTrigger(UITRIGGER_ADJUSTED_HEALTH_REGEN, _T("AdjustedHealthRegen"));
    AddTrigger(UITRIGGER_ADJUSTED_MANA_REGEN, _T("AdjustedManaRegen"));
    AddTrigger(UITRIGGER_ADJUSTED_STAMINA_REGEN, _T("AdjustedStaminaRegen"));
    AddTrigger(UITRIGGER_HEALTH_REGEN_BONUS, _T("HealthRegenBonus"));
    AddTrigger(UITRIGGER_MANA_REGEN_BONUS, _T("ManaRegenBonus"));
    AddTrigger(UITRIGGER_STAMINA_REGEN_BONUS, _T("StaminaRegenBonus"));

    AddTrigger(UITRIGGER_MELEE_QUICK_DAMAGE, _T("MeleeQuickDamage"));
    AddTrigger(UITRIGGER_MELEE_STRONG_DAMAGE, _T("MeleeStrongDamage"));
    AddTrigger(UITRIGGER_RANGED1_DAMAGE, _T("RangedDamage"));
    AddTrigger(UITRIGGER_RANGED2_DAMAGE, _T("RangedDamage"));
    AddTrigger(UITRIGGER_RANGED1_SPEED, _T("RangedSpeed"));
    AddTrigger(UITRIGGER_RANGED2_SPEED, _T("RangedSpeed"));

    AddTrigger(UITRIGGER_BASE_ARMOR, _T("BaseArmor"));
    AddTrigger(UITRIGGER_BASE_ARMOR_REDUCTION, ("BaseArmorReduction"));
    AddTrigger(UITRIGGER_ADJUSTED_ARMOR, _T("AdjustedArmor"));
    AddTrigger(UITRIGGER_ADJUSTED_ARMOR_REDUCTION, _T("AdjustedArmorReduction"));
    AddTrigger(UITRIGGER_ARMOR_BONUS, _T("ArmorBonus"));
    AddTrigger(UITRIGGER_ARMOR_REDUCTION_BONUS, _T("ArmorReductionBonus"));
    
    AddTrigger(UITRIGGER_BASE_SPEED, _T("BaseSpeed"));
    AddTrigger(UITRIGGER_CURRENT_SPEED, _T("CurrentSpeed"));

    AddTrigger(UITRIGGER_RACE, _T("Race"));
    AddTrigger(UITRIGGER_TEAM, _T("Team"));

    AddTrigger(UITRIGGER_LOADOUT_TIME, _T("LoadoutTime"));
    AddTrigger(UITRIGGER_DEATH_TIME, _T("DeathTime"));
    AddTrigger(UITRIGGER_DEATH_PERCENT, _T("DeathPercent"));
    AddTrigger(UITRIGGER_ECONOMY_INTERVAL, _T("EconomyInterval"));

    AddTrigger(UITRIGGER_HOVER_INDEX, _T("HoverIndex"));
    AddTrigger(UITRIGGER_HOVER_NAME, _T("HoverName"));
    AddTrigger(UITRIGGER_HOVER_RACE, _T("HoverRace"));
    AddTrigger(UITRIGGER_HOVER_CLASS, _T("HoverClass"));
    AddTrigger(UITRIGGER_HOVER_DESCRIPTION, _T("HoverDescription"));
    AddTrigger(UITRIGGER_HOVER_ICON, _T("HoverIcon"));
    AddTrigger(UITRIGGER_HOVER_LEVEL, _T("HoverLevel"));
    AddTrigger(UITRIGGER_HOVER_EXPERIENCE, _T("HoverExperience"));
    AddTrigger(UITRIGGER_HOVER_GOLD, _T("HoverGold"));
    AddTrigger(UITRIGGER_HOVER_COLORS, _T("HoverColors"));

    AddTrigger(UITRIGGER_HOVER_HEALTH_VALUE, _T("HoverHealthValue"));
    AddTrigger(UITRIGGER_HOVER_HEALTH_PERCENT, _T("HoverHealthPercent"));
    AddTrigger(UITRIGGER_HOVER_MAX_HEALTH, _T("HoverMaxHealth"));

    AddTrigger(UITRIGGER_HOVER_MANA_VALUE, _T("HoverManaValue"));
    AddTrigger(UITRIGGER_HOVER_MANA_PERCENT, _T("HoverManaPercent"));
    AddTrigger(UITRIGGER_HOVER_MAX_MANA, _T("HoverMaxMana"));

    AddTrigger(UITRIGGER_HOVER_STAMINA_VALUE, _T("HoverStaminaValue"));
    AddTrigger(UITRIGGER_HOVER_STAMINA_PERCENT, _T("HoverStaminaPercent"));
    AddTrigger(UITRIGGER_HOVER_MAX_STAMINA, _T("HoverMaxStamina"));

    AddTrigger(UITRIGGER_HOVER_LIFETIME_VALUE, _T("HoverLifetimeValue"));
    AddTrigger(UITRIGGER_HOVER_LIFETIME_PERCENT, _T("HoverLifetimePercent"));

    AddTrigger(UITRIGGER_HOVER_SOUL_COUNT, _T("HoverSoulCount"));

    AddTrigger(UITRIGGER_HOVER_INVENTORY_EXISTS, _T("HoverInventoryExists"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_HOVER_INVENTORY_ICON, _T("HoverInventoryIcon"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_HOVER_INVENTORY_AMMO_COUNT, _T("HoverInventoryAmmoCount"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_HOVER_INVENTORY_OVERLAY, _T("HoverInventoryOverlay"), INVENTORY_END_BACKPACK);

    AddTrigger(UITRIGGER_HOVER_BUFF_ACTIVE, _T("HoverBuffActive"), MAX_DISPLAY_BUFFS);
    AddTrigger(UITRIGGER_HOVER_BUFF_ICON, _T("HoverBuffIcon"), MAX_DISPLAY_BUFFS);
    AddTrigger(UITRIGGER_HOVER_DEBUFF_ACTIVE, _T("HoverDebuffActive"), MAX_DISPLAY_BUFFS);
    AddTrigger(UITRIGGER_HOVER_DEBUFF_ICON, _T("HoverDebuffIcon"), MAX_DISPLAY_DEBUFFS);

    AddTrigger(UITRIGGER_HOVER_MELEE_QUICK_DAMAGE, _T("HoverMeleeQuickDamage"));
    AddTrigger(UITRIGGER_HOVER_MELEE_STRONG_DAMAGE, _T("HoverMeleeStrongDamage"));
    AddTrigger(UITRIGGER_HOVER_RANGED1_DAMAGE, _T("HoverRanged1Damage"));
    AddTrigger(UITRIGGER_HOVER_RANGED2_DAMAGE, _T("HoverRanged2Damage"));

    AddTrigger(UITRIGGER_HOVER_ARMOR, _T("HoverArmor"));
    AddTrigger(UITRIGGER_HOVER_ARMOR_REDUCTION, _T("HoverArmorReduction"));

    AddTrigger(UITRIGGER_HOVER_MODEL, _T("HoverModel"));
    AddTrigger(UITRIGGER_HOVER_COMMANDER_PORTRAIT, _T("HoverCommanderPortrait"));

    AddTrigger(UITRIGGER_HOVER_BUILD_PERCENT, _T("HoverBuildPercent"));

    AddTrigger(UITRIGGER_HOVER_GOLD_PERCENT, _T("HoverGoldPercent"));
    AddTrigger(UITRIGGER_HOVER_MAX_GOLD, _T("HoverMaxGold"));
    AddTrigger(UITRIGGER_HOVER_GOLD_VALUE, _T("HoverGoldValue"));
    AddTrigger(UITRIGGER_HOVER_EXTRACTION_RATE, _T("HoverExtractionRate"));
    AddTrigger(UITRIGGER_HOVER_UPKEEP_COST, _T("HoverUpkeepCost"));
    AddTrigger(UITRIGGER_HOVER_UPKEEP_ACTIVE, _T("HoverUpkeepActive"));

    //CommSelection triggers
    AddTrigger(UITRIGGER_COMMSELECTION_INDEX, _T("CommSelectionIndex"));
    AddTrigger(UITRIGGER_COMMSELECTION_NAME, _T("CommSelectionName"));
    AddTrigger(UITRIGGER_COMMSELECTION_RACE, _T("CommSelectionRace"));
    AddTrigger(UITRIGGER_COMMSELECTION_CLASS, _T("CommSelectionClass"));
    AddTrigger(UITRIGGER_COMMSELECTION_DESCRIPTION, _T("CommSelectionDescription"));
    AddTrigger(UITRIGGER_COMMSELECTION_ICON, _T("CommSelectionIcon"));
    AddTrigger(UITRIGGER_COMMSELECTION_LEVEL, _T("CommSelectionLevel"));
    AddTrigger(UITRIGGER_COMMSELECTION_EXPERIENCE, _T("CommSelectionExperience"));
    AddTrigger(UITRIGGER_COMMSELECTION_GOLD, _T("CommSelectionGold"));
    AddTrigger(UITRIGGER_COMMSELECTION_COLORS, _T("CommSelectionColors"));

    AddTrigger(UITRIGGER_COMMSELECTION_HEALTH_VALUE, _T("CommSelectionHealthValue"));
    AddTrigger(UITRIGGER_COMMSELECTION_HEALTH_PERCENT, _T("CommSelectionHealthPercent"));
    AddTrigger(UITRIGGER_COMMSELECTION_MAX_HEALTH, _T("CommSelectionMaxHealth"));

    AddTrigger(UITRIGGER_COMMSELECTION_MANA_VALUE, _T("CommSelectionManaValue"));
    AddTrigger(UITRIGGER_COMMSELECTION_MANA_PERCENT, _T("CommSelectionManaPercent"));
    AddTrigger(UITRIGGER_COMMSELECTION_MAX_MANA, _T("CommSelectionMaxMana"));

    AddTrigger(UITRIGGER_COMMSELECTION_STAMINA_VALUE, _T("CommSelectionStaminaValue"));
    AddTrigger(UITRIGGER_COMMSELECTION_STAMINA_PERCENT, _T("CommSelectionStaminaPercent"));
    AddTrigger(UITRIGGER_COMMSELECTION_MAX_STAMINA, _T("CommSelectionMaxStamina"));

    AddTrigger(UITRIGGER_COMMSELECTION_LIFETIME_VALUE, _T("CommSelectionLifetimeValue"));
    AddTrigger(UITRIGGER_COMMSELECTION_LIFETIME_PERCENT, _T("CommSelectionLifetimePercent"));

    AddTrigger(UITRIGGER_COMMSELECTION_SOUL_COUNT, _T("CommSelectionSoulCount"));

    AddTrigger(UITRIGGER_COMMSELECTION_INVENTORY_EXISTS, _T("CommSelectionInventoryExists"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_COMMSELECTION_INVENTORY_ICON, _T("CommSelectionInventoryIcon"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_COMMSELECTION_INVENTORY_AMMO_COUNT, _T("CommSelectionInventoryAmmoCount"), INVENTORY_END_BACKPACK);
    AddTrigger(UITRIGGER_COMMSELECTION_INVENTORY_OVERLAY, _T("CommSelectionInventoryOverlay"), INVENTORY_END_BACKPACK);

    AddTrigger(UITRIGGER_COMMSELECTION_BUFF_ACTIVE, _T("CommSelectionBuffActive"), MAX_DISPLAY_BUFFS);
    AddTrigger(UITRIGGER_COMMSELECTION_BUFF_ICON, _T("CommSelectionBuffIcon"), MAX_DISPLAY_BUFFS);
    AddTrigger(UITRIGGER_COMMSELECTION_DEBUFF_ACTIVE, _T("CommSelectionDebuffActive"), MAX_DISPLAY_BUFFS);
    AddTrigger(UITRIGGER_COMMSELECTION_DEBUFF_ICON, _T("CommSelectionDebuffIcon"), MAX_DISPLAY_DEBUFFS);

    AddTrigger(UITRIGGER_COMMSELECTION_MELEE_QUICK_DAMAGE, _T("CommSelectionMeleeQuickDamage"));
    AddTrigger(UITRIGGER_COMMSELECTION_MELEE_STRONG_DAMAGE, _T("CommSelectionMeleeStrongDamage"));
    AddTrigger(UITRIGGER_COMMSELECTION_RANGED1_DAMAGE, _T("CommSelectionRanged1Damage"));
    AddTrigger(UITRIGGER_COMMSELECTION_RANGED2_DAMAGE, _T("CommSelectionRanged2Damage"));

    AddTrigger(UITRIGGER_COMMSELECTION_ARMOR, _T("CommSelectionArmor"));
    AddTrigger(UITRIGGER_COMMSELECTION_ARMOR_REDUCTION, _T("CommSelectionArmorReduction"));

    AddTrigger(UITRIGGER_COMMSELECTION_MODEL, _T("CommSelectionModel"));
    AddTrigger(UITRIGGER_COMMSELECTION_COMMANDER_PORTRAIT, _T("CommSelectionCommanderPortrait"));

    AddTrigger(UITRIGGER_COMMSELECTION_BUILD_PERCENT, _T("CommSelectionBuildPercent"));

    AddTrigger(UITRIGGER_COMMSELECTION_GOLD_PERCENT, _T("CommSelectionGoldPercent"));
    AddTrigger(UITRIGGER_COMMSELECTION_MAX_GOLD, _T("CommSelectionMaxGold"));
    AddTrigger(UITRIGGER_COMMSELECTION_GOLD_VALUE, _T("CommSelectionGoldValue"));
    AddTrigger(UITRIGGER_COMMSELECTION_EXTRACTION_RATE, _T("CommSelectionExtractionRate"));
    AddTrigger(UITRIGGER_COMMSELECTION_UPKEEP_COST, _T("CommSelectionUpkeepCost"));
    AddTrigger(UITRIGGER_COMMSELECTION_UPKEEP_ACTIVE, _T("CommSelectionUpkeepActive"));

    AddTrigger(UITRIGGER_ALT_INFO_ALLY, _T("AltInfoAlly"));
    AddTrigger(UITRIGGER_ALT_INFO_NAME, _T("AltInfoName"));
    AddTrigger(UITRIGGER_ALT_INFO_DISTANCE, _T("AltInfoDistance"));
    AddTrigger(UITRIGGER_ALT_INFO_HEALTH_PERCENT, _T("AltInfoHealthPercent"));
    AddTrigger(UITRIGGER_ALT_INFO_UPKEEP, _T("AltInfoUpkeep"));
    AddTrigger(UITRIGGER_ALT_INFO_BUILD_PERCENT, _T("AltInfoBuildPercent"));
    AddTrigger(UITRIGGER_ALT_INFO_BUFF_ACTIVE, _T("AltInfoBuffActive"), MAX_DISPLAY_BUFFS);
    AddTrigger(UITRIGGER_ALT_INFO_BUFF_ICON, _T("AltInfoBuffIcon"), MAX_DISPLAY_BUFFS);
    AddTrigger(UITRIGGER_ALT_INFO_DEBUFF_ACTIVE, _T("AltInfoDebuffActive"), MAX_DISPLAY_BUFFS);
    AddTrigger(UITRIGGER_ALT_INFO_DEBUFF_ICON, _T("AltInfoDebuffIcon"), MAX_DISPLAY_BUFFS);

    AddTrigger(UITRIGGER_PET_NAME, _T("PetName"));
    AddTrigger(UITRIGGER_PET_ICON, _T("PetIcon"));

    AddTrigger(UITRIGGER_PET_HEALTH_VALUE, _T("PetHealthValue"));
    AddTrigger(UITRIGGER_PET_MANA_VALUE, _T("PetManaValue"));
    AddTrigger(UITRIGGER_PET_HEALTH_PERCENT, _T("PetHealthPercent"));
    AddTrigger(UITRIGGER_PET_MANA_PERCENT, _T("PetManaPercent"));
    
    AddTrigger(UITRIGGER_PET_MAX_HEALTH, _T("PetMaxHealth"));
    AddTrigger(UITRIGGER_PET_MAX_MANA, _T("PetMaxMana"));
    AddTrigger(UITRIGGER_PET_MAX_STAMINA, _T("PetMaxStamina"));

    AddTrigger(UITRIGGER_PET_ARMOR, _T("PetArmor"));
    AddTrigger(UITRIGGER_PET_ARMOR_REDUCTION, _T("PetArmorReduction"));

    AddTrigger(UITRIGGER_OFFICER_SQUAD_NAME, _T("OfficerSquadName"));

    AddTrigger(UITRIGGER_REPAIRABLE, _T("Repairable"));

    // Lobby
    AddTrigger(UITRIGGER_LOBBY_PHASE, _T("LobbyPhase"));
    AddTrigger(UITRIGGER_LOBBY_STATUS, _T("LobbyStatus"));
    AddTrigger(UITRIGGER_LOBBY_PHASE_DESCRIPTION, _T("LobbyPhaseDescription"));
    AddTrigger(UITRIGGER_LOBBY_PHASE_TIMER, _T("LobbyPhaseTimer"));
    AddTrigger(UITRIGGER_LOBBY_PHASE_INSTRUCTION, _T("LobbyPhaseInstruction"));
    AddTrigger(UITRIGGER_LOBBY_REQUEST_BUTTON, _T("LobbyRequestButton"));
    AddTrigger(UITRIGGER_LOBBY_DECLINE_BUTTON, _T("LobbyDeclineButton"));
    AddTrigger(UITRIGGER_LOBBY_TEAM_LIST, _T("LobbyTeamList"), 2);
    AddTrigger(UITRIGGER_LOBBY_TEAM_CANDIDATE_LIST, _T("LobbyCandidateList"), 2);
    AddTrigger(UITRIGGER_LOBBY_TEAM_BUTTON, _T("LobbyTeamButton"), 2);
    AddTrigger(UITRIGGER_LOBBY_TEAM_OFFICER_DEMOTE_BUTTON, _T("LobbyOfficerDemoteButton"), 2, MAX_OFFICERS);
    
    AddTrigger(UITRIGGER_LOBBY_COMMANDER_VISIBLE, _T("LobbyCommanderVisible"), 2);
    AddTrigger(UITRIGGER_LOBBY_CANDIDATE_LIST_VISIBLE, _T("LobbyCandidateListVisible"), 2);
    AddTrigger(UITRIGGER_LOBBY_CANDIDATE_LIST_SIZE, _T("LobbyCandidateListSize"), 2);
    AddTrigger(UITRIGGER_LOBBY_TEAM_LIST_VISIBLE, _T("LobbyTeamListVisible"), 2);
    AddTrigger(UITRIGGER_LOBBY_TEAM_LIST_SIZE, _T("LobbyTeamListSize"), 2);
    AddTrigger(UITRIGGER_LOBBY_TEAM_PLAYER_COUNT, _T("LobbyTeamPlayerCount"), 2);
    AddTrigger(UITRIGGER_LOBBY_TEAM_AVERAGE_SF, _T("LobbyTeamAverageSF"), 2);
    AddTrigger(UITRIGGER_LOBBY_TEAM_AVERAGE_LEVEL, _T("LobbyTeamAverageLevel"), 2);
    AddTrigger(UITRIGGER_LOBBY_TEAM_RACE, _T("LobbyTeamRace"), 2);
    AddTrigger(UITRIGGER_LOBBY_SPECTATOR_LIST, _T("LobbySpectatorList"));
    AddTrigger(UITRIGGER_LOBBY_SPECTATOR_BUTTON, _T("LobbySpectateButton"));

    AddTrigger(UITRIGGER_LOBBY_OFFICER_VISIBLE, _T("LobbyOfficerVisible"), 2, MAX_OFFICERS);
    AddTrigger(UITRIGGER_LOBBY_OFFICER_NAME, _T("LobbyOfficerName"), 2, MAX_OFFICERS);
    AddTrigger(UITRIGGER_LOBBY_SQUAD_LIST, _T("LobbySquadList"), 2, MAX_OFFICERS + 1);
    AddTrigger(UITRIGGER_LOBBY_SQUAD_LIST_SIZE, _T("LobbySquadListSize"), 2, MAX_OFFICERS + 1);
    AddTrigger(UITRIGGER_LOBBY_SQUAD_LIST_VISIBLE, _T("LobbySquadListVisible"), 2, MAX_OFFICERS + 1);
    AddTrigger(UITRIGGER_LOBBY_SQUAD_LIST_NAME, _T("LobbySquadListName"), 2, MAX_OFFICERS + 1);
    AddTrigger(UITRIGGER_LOBBY_SQUAD_LIST_COLOR, _T("LobbySquadListColor"), 2, MAX_OFFICERS + 1);
    AddTrigger(UITRIGGER_LOBBY_SQUAD_JOIN_BUTTON, _T("LobbySquadJoinButton"), 2, MAX_OFFICERS + 1);
    AddTrigger(UITRIGGER_LOBBY_COMMANDER_NAME, _T("LobbyCommanderName"), 2);

    AddTrigger(UITRIGGER_LOBBY_SERVER_NAME, _T("LobbyServerName"));
    AddTrigger(UITRIGGER_LOBBY_SERVER_LOCATION, _T("LobbyServerLocation"));
    AddTrigger(UITRIGGER_LOBBY_SERVER_MAP, _T("LobbyServerMap"));
    AddTrigger(UITRIGGER_LOBBY_SERVER_STATUS, _T("LobbyServerStatus"));
    AddTrigger(UITRIGGER_LOBBY_SERVER_PLAYERS, _T("LobbyServerPlayers"));
    AddTrigger(UITRIGGER_LOBBY_SERVER_MIN_PLAYERS, _T("LobbyServerMinPlayers"));
    AddTrigger(UITRIGGER_LOBBY_SERVER_PING, _T("LobbyServerPing"));
    AddTrigger(UITRIGGER_LOBBY_SERVER_MAP_VERSION, _T("LobbyServerMapVersion"));
    AddTrigger(UITRIGGER_LOBBY_SERVER_MATCHES_PLAYED, _T("LobbyServerMatchesPlayed"));
    AddTrigger(UITRIGGER_LOBBY_SERVER_MIN_KARMA, _T("LobbyServerMinKarma"));

    // Scoreboard
    AddTrigger(UITRIGGER_SCOREBOARD_PLAYER_ACTIVE, _T("ScoreboardPlayerActive"), 2, MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SCOREBOARD_PLAYER_SQUAD, _T("ScoreboardPlayerSquad"), 2, MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SCOREBOARD_PLAYER_OFFICER, _T("ScoreboardPlayerOfficer"), 2, MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SCOREBOARD_PLAYER_LEVEL, _T("ScoreboardPlayerLevel"), 2, MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SCOREBOARD_PLAYER_NAME, _T("ScoreboardPlayerName"), 2, MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SCOREBOARD_PLAYER_PING, _T("ScoreboardPlayerPing"), 2, MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SCOREBOARD_PLAYER_EXP, _T("ScoreboardPlayerExp"), 2, MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SCOREBOARD_PLAYER_KILLS, _T("ScoreboardPlayerKills"), 2, MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SCOREBOARD_PLAYER_DEATHS, _T("ScoreboardPlayerDeaths"), 2, MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SCOREBOARD_PLAYER_ASSISTS, _T("ScoreboardPlayerAssists"), 2, MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SCOREBOARD_PLAYER_MUTED, _T("ScoreboardPlayerMuted"), 2, MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SCOREBOARD_PLAYER_SF, _T("ScoreboardPlayerSF"), 2, MAX_DISPLAY_PLAYERS);

    AddTrigger(UITRIGGER_SCOREBOARD_COMMAND_CENTER_HEALTH_PERCENT, _T("ScoreboardCommandCenterHealthPercent"), 2);
    AddTrigger(UITRIGGER_SCOREBOARD_COMMAND_CENTER_HEALTH_VALUE, _T("ScoreboardCommandCenterHealthValue"), 2);
    AddTrigger(UITRIGGER_SCOREBOARD_COMMAND_CENTER_HEALTH_MAX, _T("ScoreboardCommandCenterHealthMax"), 2);

    AddTrigger(UITRIGGER_SCOREBOARD_COMMANDER, _T("ScoreboardCommander"), 2);
    AddTrigger(UITRIGGER_SCOREBOARD_COMMANDER_RECORD, _T("ScoreboardCommanderRecord"), 2);
    AddTrigger(UITRIGGER_SCOREBOARD_COMMANDER_SF, _T("ScoreboardCommanderSF"), 2);

    AddTrigger(UITRIGGER_SCOREBOARD_NUM_PLAYERS, _T("ScoreboardNumPlayers"), 2);

    AddTrigger(UITRIGGER_GAME_MATCH_ID, _T("GameMatchID"));

    AddTrigger(UITRIGGER_SQUAD_INFO, _T("SquadInfo"), MAX_DISPLAY_SQUAD);
    AddTrigger(UITRIGGER_SQUAD_ICON, _T("SquadIcon"), MAX_DISPLAY_SQUAD);
    AddTrigger(UITRIGGER_SQUAD_NAME, _T("SquadName"), MAX_DISPLAY_SQUAD);
    AddTrigger(UITRIGGER_SQUAD_HEALTH_PERCENT, _T("SquadHealthPercent"), MAX_DISPLAY_SQUAD);
    AddTrigger(UITRIGGER_SQUAD_MANA_PERCENT, _T("SquadManaPercent"), MAX_DISPLAY_SQUAD);

    AddTrigger(UITRIGGER_ENDGAME_TEAM, _T("EndGameTeam"), 2);

    AddTrigger(UITRIGGER_ENDGAME_TIME, _T("EndGameTime"));

    AddTrigger(UITRIGGER_ENDGAME_AWARD, _T("EndGameAward"), NUM_END_GAME_AWARDS);
    AddTrigger(UITRIGGER_ENDGAME_LIST, _T("EndGameList"), NUM_END_GAME_AWARDS);

    AddTrigger(UITRIGGER_ENDGAME_DATA_EXPERIENCE, _T("EndGameDataExperience"));
    AddTrigger(UITRIGGER_ENDGAME_DATA_KILLS, _T("EndGameDataKills"));
    AddTrigger(UITRIGGER_ENDGAME_DATA_ASSISTS, _T("EndGameDataAssists"));
    AddTrigger(UITRIGGER_ENDGAME_DATA_DEATHS, _T("EndGameDataDeaths"));
    AddTrigger(UITRIGGER_ENDGAME_DATA_KILL_DEATH_RATIO, _T("EndGameDataKillDeathRatio"));
    AddTrigger(UITRIGGER_ENDGAME_DATA_HP_HEALED, _T("EndGameDataHPHealed"));
    AddTrigger(UITRIGGER_ENDGAME_DATA_RESURRECTS, _T("EndGameDataResurrects"));
    AddTrigger(UITRIGGER_ENDGAME_DATA_GOLD_EARNED, _T("EndGameDataGoldEarned"));
    AddTrigger(UITRIGGER_ENDGAME_DATA_HP_REPAIRED, _T("EndGameDataHPRepaired"));
    AddTrigger(UITRIGGER_ENDGAME_DATA_BUILDINGS_RAZED, _T("EndGameDataBuildingsRazed"));
    AddTrigger(UITRIGGER_ENDGAME_DATA_PLAYER_DAMAGE, _T("EndGameDataPlayerDamage"));
    AddTrigger(UITRIGGER_ENDGAME_DATA_BUILDING_DAMAGE, _T("EndGameDataBuildingDamage"));

    AddTrigger(UITRIGGER_ENDGAME_DATA_EXPERIENCE_PAGES, _T("EndGameDataExperiencePages"));
    AddTrigger(UITRIGGER_ENDGAME_DATA_KILLS_PAGES, _T("EndGameDataKillsPages"));
    AddTrigger(UITRIGGER_ENDGAME_DATA_ASSISTS_PAGES, _T("EndGameDataAssistsPages"));
    AddTrigger(UITRIGGER_ENDGAME_DATA_DEATHS_PAGES, _T("EndGameDataDeathsPages"));
    AddTrigger(UITRIGGER_ENDGAME_DATA_KILL_DEATH_RATIO_PAGES, _T("EndGameDataKillDeathRatioPages"));
    AddTrigger(UITRIGGER_ENDGAME_DATA_HP_HEALED_PAGES, _T("EndGameDataHPHealedPages"));
    AddTrigger(UITRIGGER_ENDGAME_DATA_RESURRECTS_PAGES, _T("EndGameDataResurrectsPages"));
    AddTrigger(UITRIGGER_ENDGAME_DATA_GOLD_EARNED_PAGES, _T("EndGameDataGoldEarnedPages"));
    AddTrigger(UITRIGGER_ENDGAME_DATA_HP_REPAIRED_PAGES, _T("EndGameDataHPRepairedPages"));
    AddTrigger(UITRIGGER_ENDGAME_DATA_BUILDINGS_RAZED_PAGES, _T("EndGameDataBuildingsRazedPages"));
    AddTrigger(UITRIGGER_ENDGAME_DATA_PLAYER_DAMAGE_PAGES, _T("EndGameDataPlayerDamagePages"));
    AddTrigger(UITRIGGER_ENDGAME_DATA_BUILDING_DAMAGE_PAGES, _T("EndGameDataBuildingDamagePages"));

    AddTrigger(UITRIGGER_ENDGAME_RANK, _T("EndGameRank"));
    AddTrigger(UITRIGGER_ENDGAME_NPC_KILLS, _T("EndGameNPCKills"));
    AddTrigger(UITRIGGER_ENDGAME_EXPERIENCE, _T("EndGameExperience"));
    AddTrigger(UITRIGGER_ENDGAME_HP_HEALED, _T("EndGameHPHealed"));
    AddTrigger(UITRIGGER_ENDGAME_KILLS, _T("EndGameKills"));
    AddTrigger(UITRIGGER_ENDGAME_RESURRECTS, _T("EndGameResurrects"));
    AddTrigger(UITRIGGER_ENDGAME_DEATHS, _T("EndGameDeaths"));
    AddTrigger(UITRIGGER_ENDGAME_BUILDING_DAMAGE, _T("EndGameBuildingDamage"));
    AddTrigger(UITRIGGER_ENDGAME_KILL_DEATH_RATIO, _T("EndGameKillDeathRatio"));
    AddTrigger(UITRIGGER_ENDGAME_RAZED, _T("EndGameRazed"));
    AddTrigger(UITRIGGER_ENDGAME_ASSISTS, _T("EndGameAssists"));
    AddTrigger(UITRIGGER_ENDGAME_HP_REPAIRED, _T("EndGameHPRepaired"));
    AddTrigger(UITRIGGER_ENDGAME_PLAYER_DAMAGE, _T("EndGamePlayerDamage"));
    AddTrigger(UITRIGGER_ENDGAME_GOLD_EARNED, _T("EndGameGoldEarned"));
    AddTrigger(UITRIGGER_ENDGAME_SOULS_SPENT, _T("EndGameSoulsSpent"));
    AddTrigger(UITRIGGER_ENDGAME_TIME_PLAYED, _T("EndGameTimePlayed"));
    AddTrigger(UITRIGGER_ENDGAME_VICTORY, _T("EndGameVictory"));
    AddTrigger(UITRIGGER_ENDGAME_SHOW_STATS, _T("EndGameShowStats"));
    AddTrigger(UITRIGGER_ENDGAME_MATCH_ID, _T("EndGameMatchID"));

    AddTrigger(UITRIGGER_BUILDING_INFO_EXISTS, _T("BuildingInfoExists"), MAX_BUILDING_INFO);
    AddTrigger(UITRIGGER_BUILDING_INFO_INDEX, _T("BuildingInfoIndex"), MAX_BUILDING_INFO);
    AddTrigger(UITRIGGER_BUILDING_INFO_NAME, _T("BuildingInfoName"), MAX_BUILDING_INFO);
    AddTrigger(UITRIGGER_BUILDING_INFO_ARMOR, _T("BuildingInfoArmor"), MAX_BUILDING_INFO);
    AddTrigger(UITRIGGER_BUILDING_INFO_ICON, _T("BuildingInfoIcon"), MAX_BUILDING_INFO);
    AddTrigger(UITRIGGER_BUILDING_INFO_HEALTH_PERCENT, _T("BuildingInfoHealthPercent"), MAX_BUILDING_INFO);
    AddTrigger(UITRIGGER_BUILDING_INFO_HEALTH_VALUE, _T("BuildingInfoHealthValue"), MAX_BUILDING_INFO);
    AddTrigger(UITRIGGER_BUILDING_INFO_MAX_HEALTH, _T("BuildingInfoMaxHealth"), MAX_BUILDING_INFO);
    AddTrigger(UITRIGGER_BUILDING_INFO_UPKEEP_COST, _T("BuildingInfoUpkeepCost"), MAX_BUILDING_INFO);
    AddTrigger(UITRIGGER_BUILDING_INFO_UPKEEP_ENABLED, _T("BuildingInfoUpkeepEnabled"), MAX_BUILDING_INFO);
    AddTrigger(UITRIGGER_BUILDING_INFO_EXTRACTION_RATE, _T("BuildingInfoExtractionRate"), MAX_BUILDING_INFO);
    AddTrigger(UITRIGGER_BUILDING_INFO_GOLD_VALUE, _T("BuildingInfoGoldValue"), MAX_BUILDING_INFO);
    AddTrigger(UITRIGGER_BUILDING_INFO_GOLD_PERCENT, _T("BuildingInfoGoldPercent"), MAX_BUILDING_INFO);
    AddTrigger(UITRIGGER_BUILDING_INFO_MAX_GOLD, _T("BuildingInfoMaxGold"), MAX_BUILDING_INFO);
    AddTrigger(UITRIGGER_BUILDING_INFO_BUILD_PERCENT, _T("BuildingInfoBuildPercent"), MAX_BUILDING_INFO);
    AddTrigger(UITRIGGER_BUILDING_INFO_REPAIRABLE, _T("BuildingInfoRepairable"), MAX_BUILDING_INFO);

    AddTrigger(UITRIGGER_MULTI_SELECT_ACTIVE, _T("MultiSelectActive"), MAX_SELECTED_UNITS);
    AddTrigger(UITRIGGER_MULTI_SELECT_ICON, _T("MultiSelectIcon"), MAX_SELECTED_UNITS);
    AddTrigger(UITRIGGER_MULTI_SELECT_DEAD, _T("MultiSelectDead"), MAX_SELECTED_UNITS);
    AddTrigger(UITRIGGER_MULTI_SELECT_HEALTH_PERCENT, _T("MultiSelectHealthPercent"), MAX_SELECTED_UNITS);
    AddTrigger(UITRIGGER_MULTI_SELECT_MANA_PERCENT, _T("MultiSelectManaPercent"), MAX_SELECTED_UNITS);
    AddTrigger(UITRIGGER_MULTI_SELECT_SQUAD_COLOR, _T("MultiSelectSquadColor"), MAX_SELECTED_UNITS);
    AddTrigger(UITRIGGER_MULTI_SELECT_INDEX, _T("MultiSelectIndex"), MAX_SELECTED_UNITS);

    AddTrigger(UITRIGGER_COMMANDER_SQUAD_EXISTS, _T("CommanderSquadExists"), MAX_OFFICERS);
    AddTrigger(UITRIGGER_COMMANDER_SQUAD_COLOR, _T("CommanderSquadColor"), MAX_OFFICERS);
    AddTrigger(UITRIGGER_COMMANDER_SQUAD_SIZE, _T("CommanderSquadSize"), MAX_OFFICERS);
    AddTrigger(UITRIGGER_COMMANDER_SQUAD_OFFICER_INDEX, _T("CommanderSquadOfficerIndex"), MAX_OFFICERS);
    AddTrigger(UITRIGGER_COMMANDER_SQUAD_OFFICER_ICON, _T("CommanderSquadOfficerIcon"), MAX_OFFICERS);
    AddTrigger(UITRIGGER_COMMANDER_SQUAD_OFFICER_HEALTH_PERCENT, _T("CommanderSquadOfficerHealthPercent"), MAX_OFFICERS);
    AddTrigger(UITRIGGER_COMMANDER_SQUAD_OFFICER_MANA_PERCENT, _T("CommanderSquadOfficerManaPercent"), MAX_OFFICERS);
    AddTrigger(UITRIGGER_COMMANDER_SQUAD_OFFICER_DEAD, _T("CommanderSquadOfficerDead"), MAX_OFFICERS);
    
    AddTrigger(UITRIGGER_COMMANDER_SQUAD_MEMBER_ACTIVE, _T("CommanderSquadMemberActive"), MAX_OFFICERS, MAX_DISPLAY_SQUAD);
    AddTrigger(UITRIGGER_COMMANDER_SQUAD_MEMBER_INDEX, _T("CommanderSquadMemberIndex"), MAX_OFFICERS, MAX_DISPLAY_SQUAD);
    AddTrigger(UITRIGGER_COMMANDER_SQUAD_MEMBER_ICON, _T("CommanderSquadMemberIcon"), MAX_OFFICERS, MAX_DISPLAY_SQUAD);
    AddTrigger(UITRIGGER_COMMANDER_SQUAD_MEMBER_HEALTH_PERCENT, _T("CommanderSquadMemberHealthPercent"), MAX_OFFICERS, MAX_DISPLAY_SQUAD);
    AddTrigger(UITRIGGER_COMMANDER_SQUAD_MEMBER_MANA_PERCENT, _T("CommanderSquadMemberManaPercent"), MAX_OFFICERS, MAX_DISPLAY_SQUAD);
    AddTrigger(UITRIGGER_COMMANDER_SQUAD_MEMBER_DEAD, _T("CommanderSquadMemberDead"), MAX_OFFICERS, MAX_DISPLAY_SQUAD);

    AddTrigger(UITRIGGER_COMMANDER_WORKER_COOLDOWN, _T("WorkerCooldownTimer"));
    AddTrigger(UITRIGGER_COMMANDER_WORKER_AVAILABLE, _T("WorkerAvailable"));
    AddTrigger(UITRIGGER_COMMANDER_WORKER_INDEX, _T("WorkerIndex"), MAX_WORKER_ICONS);
    AddTrigger(UITRIGGER_COMMANDER_WORKER_ICON, _T("WorkerIcon"), MAX_WORKER_ICONS);
    AddTrigger(UITRIGGER_COMMANDER_WORKER_ACTION, _T("WorkerAction"), MAX_WORKER_ICONS);
    AddTrigger(UITRIGGER_COMMANDER_WORKER_HEALTH, _T("WorkerHealth"), MAX_WORKER_ICONS);

    AddTrigger(UITRIGGER_BUILD_MODE_ACTIVE, _T("BuildModeActive"));
    AddTrigger(UITRIGGER_BUILD_FAIL_REASON, _T("BuildFailReason"));
    AddTrigger(UITRIGGER_KILL_NOTIFICATION, _T("KillNotification"));

    // Voice commands
    AddTrigger(UITRIGGER_VOICE_COMMAND_MAIN, _T("VoiceCommandMain"));
    AddTrigger(UITRIGGER_VOICE_COMMAND_SUB, _T("VoiceCommandSub"));
    AddTrigger(UITRIGGER_VOICE_COMMAND_CATEGORIES, _T("VoiceCommandCategories"));
    AddTrigger(UITRIGGER_VOICE_COMMAND_SUB_ITEMS, _T("VoiceCommandSubItems"));
    AddTrigger(UITRIGGER_VOICE_COMMAND_SUB_TITLE, _T("VoiceCommandSubTitle"));

    AddTrigger(UITRIGGER_LOADOUT_TOOLTIP_VISIBLE, _T("LoadoutTooltipVisible"));
    AddTrigger(UITRIGGER_LOADOUT_TOOLTIP_TITLE, _T("LoadoutTooltipTitle"));
    AddTrigger(UITRIGGER_LOADOUT_TOOLTIP_MESSAGE, _T("LoadoutTooltipMessage"));
    AddTrigger(UITRIGGER_LOADOUT_SPAWN_QUEUE_POSITION, _T("TeamQueuePosition"));
    AddTrigger(UITRIGGER_LOADOUT_SPAWN_QUEUE_SIZE, _T("TeamQueueSize"));

    AddTrigger(UITRIGGER_REPLAY_NAME, _T("ReplayName"));
    AddTrigger(UITRIGGER_REPLAY_TIME, _T("ReplayTime"));
    AddTrigger(UITRIGGER_REPLAY_ENDTIME, _T("ReplayEndTime"));
    AddTrigger(UITRIGGER_REPLAY_FRAME, _T("ReplayFrame"));
    AddTrigger(UITRIGGER_REPLAY_ENDFRAME, _T("ReplayEndFrame"));
    AddTrigger(UITRIGGER_REPLAY_SPEED, _T("ReplaySpeed"));

    AddTrigger(UITRIGGER_WARMUP_PLAYERS_REQUIRED, _T("WarmupPlayersRequired"));

    AddTrigger(UITRIGGER_DEMO_ACCOUNT, _T("DemoAccount"));

    // Initialize m_mapCanBuild
    const EntAllocatorNameMap &mapAllocatorNames(EntityRegistry.GetAllocatorNames());
    for (EntAllocatorNameMap::const_iterator cit(mapAllocatorNames.begin()); cit != mapAllocatorNames.end(); ++cit)
    {
        const IEntityAllocator *pAllocator(cit->second);
        if (!pAllocator)
            continue;

        if (pAllocator->GetName().compare(0, 9, _T("Building_")) == 0)
            m_mapCanBuild[pAllocator] = -1;
    }
}


/*====================
  CGameInterfaceManager::AddTrigger
  ====================*/
void    CGameInterfaceManager::AddTrigger(uint uiTriggerID, const tstring &sName, uint uiCount, uint uiSubCount)
{
    assert(uiTriggerID < m_vTriggers.size());

    SAFE_DELETE(m_vTriggers[uiTriggerID]);
    m_vTriggers[uiTriggerID] = K2_NEW(MemManager.GetHeap(HEAP_CLIENT_GAME),   CSmartGameUITrigger)(sName, uiCount, uiSubCount);
}


/*====================
  CGameInterfaceManager::Show
  ====================*/
void    CGameInterfaceManager::Show(uint uiTriggerID, uint uiIndex, uint uiSubIndex)
{
    assert(uiTriggerID < NUM_UITRIGGERS);
    m_vTriggers[uiTriggerID]->Show(uiIndex, uiSubIndex);
}


/*====================
  CGameInterfaceManager::Hide
  ====================*/
void    CGameInterfaceManager::Hide(uint uiTriggerID, uint uiIndex, uint uiSubIndex)
{
    assert(uiTriggerID < NUM_UITRIGGERS);
    m_vTriggers[uiTriggerID]->Hide(uiIndex, uiSubIndex);
}


/*====================
  CGameInterfaceManager::Init
  ====================*/
void    CGameInterfaceManager::Init()
{
    PROFILE("CGameInterfaceManager::Init");

    svector vInterfaceList;
    vInterfaceList.push_back(_T("/ui/game.xml"));
    vInterfaceList.push_back(_T("/ui/game_dead.xml"));
    vInterfaceList.push_back(_T("/ui/game_info.xml"));
    vInterfaceList.push_back(_T("/ui/game_loadout.xml"));
    vInterfaceList.push_back(_T("/ui/game_commander.xml"));
    vInterfaceList.push_back(_T("/ui/game_lobby.xml"));
    vInterfaceList.push_back(_T("/ui/game_spawn.xml"));
    vInterfaceList.push_back(_T("/ui/game_sacrificed.xml"));
    vInterfaceList.push_back(_T("/ui/game_player_build.xml"));
    vInterfaceList.push_back(_T("/ui/game_browser.xml"));
    vInterfaceList.push_back(_T("/ui/game_end_stats.xml"));
    vInterfaceList.push_back(_T("/ui/game_menu.xml"));
    vInterfaceList.push_back(_T("/ui/credit_card.xml"));
    vInterfaceList.push_back(_T("/ui/game_observer.xml"));
    vInterfaceList.push_back(_T("/ui/game_score_overlay.xml"));
    vInterfaceList.push_back(_T("/ui/game_standby.xml"));

    if (ReplayManager.IsPlaying())
        vInterfaceList.push_back(_T("/ui/game_replay_control.xml"));

    class CLoadInterfaceFunctions : public CLoadJob<svector, tstring>::IFunctions
    {
    public:
        float   Frame(svector_it &it, float f) const
        {
            UIManager.SetActiveInterface(_T("host_connecting"));
            SetTitle(_T("Loading interfaces"));
            SetDescription(*it);
            SetProgress(f);
            return 0.0f;
        }
        float   PostFrame(svector_it &it, float f) const    { UIManager.LoadInterface(*it); ++it; return 1.0f; }
    };
    CLoadInterfaceFunctions fnLoadInterface;
    CLoadJob<svector, tstring>  job(vInterfaceList, &fnLoadInterface, false);
    job.Execute(vInterfaceList.size());

    SetCursor(_T("crosshair"));
}


/*====================
  CGameInterfaceManager::SetPersistantStat
  ====================*/
void    CGameInterfaceManager::SetPersistantStat(int iStat, int iValue)
{
    if (iStat < 0 || iStat >= NUM_PERSISTANT_STATS)
        return;

    Trigger(UITRIGGER_PERSISTANT_STAT, iValue, iStat);
}


/*====================
  CGameInterfaceManager::ShowCursor
  ====================*/
void    CGameInterfaceManager::ShowCursor(bool bShow)
{
    if (m_uiCursorUpdateSequence == m_uiUpdateSequence && m_bCursor == bShow)
        return;

    m_bCursor = bShow;
    m_uiCursorUpdateSequence = m_uiUpdateSequence;

    if (bShow)
        Execute(UITRIGGER_CURSOR, _T("ShowOnly('") + m_sCurrentCursor + _T("');"));
    else
        Execute(UITRIGGER_CURSOR, _T("HideGroup('cursors');"));
}


/*====================
  CGameInterfaceManager::SetHoverMeleeQuickDamage
  ====================*/
void    CGameInterfaceManager::SetHoverMeleeQuickDamage(float fMin, float fMax)
{
    tstring sValue(XtoA(fMin, 0, 0, 0) + _T(" - ") + XtoA(fMax, 0, 0, 0));

    Trigger(UITRIGGER_HOVER_MELEE_QUICK_DAMAGE, sValue);
}


/*====================
  CGameInterfaceManager::SetHoverMeleeStrongDamage
  ====================*/
void    CGameInterfaceManager::SetHoverMeleeStrongDamage(float fMin, float fMax)
{
    tstring sValue(XtoA(fMin, 0, 0, 0) + _T(" - ") + XtoA(fMax, 0, 0, 0));
    Trigger(UITRIGGER_HOVER_MELEE_STRONG_DAMAGE, sValue);
}


/*====================
  CGameInterfaceManager::SetHoverRanged1Damage
  ====================*/
void    CGameInterfaceManager::SetHoverRanged1Damage(float fMin, float fMax)
{
    tstring sValue(XtoA(fMin, 0, 0, 0) + _T(" - ") + XtoA(fMax, 0, 0, 0));
    Trigger(UITRIGGER_HOVER_RANGED1_DAMAGE, sValue);
}


/*====================
  CGameInterfaceManager::SetHoverRanged2Damage
  ====================*/
void    CGameInterfaceManager::SetHoverRanged2Damage(float fMin, float fMax)
{
    tstring sValue(XtoA(fMin, 0, 0, 0) + _T(" - ") + XtoA(fMax, 0, 0, 0));
    Trigger(UITRIGGER_HOVER_RANGED2_DAMAGE, sValue);
}


/*====================
  CGameInterfaceManager::SetPersistantVaultIcon
  ====================*/
void    CGameInterfaceManager::SetPersistantVaultIcon(int iVaultNum)
{
    PROFILE("CGameInterfaceManager::SetPersistantVaultIcon");

    ushort unData(GameClient.GetPersistantItemType(iVaultNum));

    if (unData != PERSISTANT_ITEM_NULL)
    {
        uint uiPersistantType = (unData / 1000) % 10;
        uint uiRegenMod = (unData / 100) % 10;
        uint uiIncreaseMod = (unData / 10) % 10;
        uint uiReplenishMod = (unData % 10);

        tstring sImageList;

        if (uiRegenMod != PERSISTANT_REGEN_NULL)
            sImageList += g_PersistantItemsConfig.GetRegenIconPath(uiRegenMod);

        if (uiPersistantType != PERSISTANT_TYPE_NULL)
        {
            if (!sImageList.empty())
                sImageList += IMAGELIST_SEPERATOR;

            sImageList += g_PersistantItemsConfig.GetTypeIconPath(uiPersistantType);
        }

        if (uiIncreaseMod != PERSISTANT_INCREASE_NULL)
        {
            if (!sImageList.empty())
                sImageList += IMAGELIST_SEPERATOR;

            sImageList += g_PersistantItemsConfig.GetIncreaseIconPath(uiIncreaseMod);
        }

        if (uiReplenishMod != PERSISTANT_REPLENISH_NULL)
        {
            if (!sImageList.empty())
                sImageList += IMAGELIST_SEPERATOR;

            sImageList += g_PersistantItemsConfig.GetReplenishIconPath(uiReplenishMod);
        }

        Trigger(UITRIGGER_PERSISTANT_VAULT_ICON, sImageList, iVaultNum);
    }
    else
        Trigger(UITRIGGER_PERSISTANT_VAULT_ICON, g_PersistantItemsConfig.GetDefaultIcon(), iVaultNum);
}


/*====================
  CGameInterfaceManager::ShowStats
  ====================*/
void    CGameInterfaceManager::ShowStats(int iClientNum)
{
    CEntityClientInfo* pClient(GameClient.GetClientInfo(iClientNum));
    if (pClient == NULL)
        return;

    bool bShowStats(false);

    if (!pClient->IsDemoAccount() || (GameClient.GetLocalClient() != NULL && !GameClient.GetLocalClient()->IsDemoAccount()))
        bShowStats = true;

    ForceUpdate();

    Trigger(UITRIGGER_PLAYER_NAME_PERSISTANT, pClient->GetName());
    for (int i(0); i < NUM_PERSISTANT_STATS; ++i)
    {
        if (bShowStats)
            SetPersistantStat(i, pClient->GetPersistantStat(i));
        else
            SetPersistantStat(i, 0);
    }
}


/*====================
  CGameInterfaceManager::ShowStatsByTeam
  ====================*/
void    CGameInterfaceManager::ShowStatsByTeam(int iTeam, int iPlayer)
{
    CEntityTeamInfo *pTeam;

    pTeam = GameClient.GetTeam(iTeam);

    if (pTeam == NULL)
        return;

    ShowStats(pTeam->GetClientIDFromTeamIndex(iPlayer));
}


/*====================
  CGameInterfaceManager::LobbyRequiresUpdate
  ====================*/
bool    CGameInterfaceManager::LobbyRequiresUpdate()
{
    if (m_eGamePhase != GameClient.GetGamePhase())
        return true;

    CEntityClientInfo *pLocalClient(GameClient.GetLocalClient());

    if (pLocalClient != NULL)
    {
        if (GameClient.GetGamePhase() == GAME_PHASE_ACTIVE &&
            (pLocalClient->GetTeam() < 1 ||
            (pLocalClient->GetSquad() == INVALID_SQUAD && !pLocalClient->HasFlags(CLIENT_INFO_IS_COMMANDER))))
            return true;
    }

    for (int i(0); i < Game.GetNumTeams(); ++i)
        if (GameClient.GetTeam(i) != NULL && GameClient.GetTeam(i)->RosterChanged())
            return true;

    return false;
}


/*====================
  CGameInterfaceManager::UpdateLoadout
  ====================*/
void    CGameInterfaceManager::UpdateLoadout()
{
    CEntityClientInfo *pLocalClient(GameClient.GetLocalClient());
    if (pLocalClient == NULL)
        return;
    IPlayerEntity *pPlayer(pLocalClient->GetPlayerEntity());
    if (pPlayer == NULL)
        return;

    ConsumableItemManager.SortItems();

    CEntityTeamInfo *pTeam(GameClient.GetTeam(pPlayer->GetTeam()));
    if (pTeam != NULL)
    {
        int iItemCount(0);
        for (uint ui(0); iItemCount < MAX_DISPLAY_ITEM_SHOP && ui < ConsumableItemManager.GetNumItems(); ++ui)
        {
            tstring sName(ConsumableItemManager.GetItemName(ui));
            ushort unType(EntityRegistry.LookupID(sName));
            if (unType == INVALID_ENT_TYPE)
                continue;

            ICvar *pRace(EntityRegistry.GetGameSetting(unType, _T("Race")));
            if (pRace == NULL)
                continue;
            if (!pRace->GetString().empty() && CompareNoCase(pRace->GetString(), pTeam->GetDefinition()->GetName()) != 0)
                continue;

            ICvar *pIcon(EntityRegistry.GetGameSetting(unType, _T("IconPath")));
            if (pIcon == NULL)
                continue;

            // Basic info
            Trigger(UITRIGGER_CONSUMABLE_ITEM_ICON, pIcon->GetString(), iItemCount);
            Trigger(UITRIGGER_CONSUMABLE_ITEM_NAME, sName, iItemCount);

            // Check availabiltity
            bool bAvailable(true);
            if (!pPlayer->GetCanPurchase())
                bAvailable = false;

            ICvar *pCost(EntityRegistry.GetGameSetting(unType, _T("Cost")));
            if (bAvailable && !ICvar::GetBool(_T("sv_allItemsAvailable")) && pCost != NULL)
            {
                if (pLocalClient->GetGold() < pCost->GetInteger())
                    bAvailable = false;
            }

            ICvar *pCategory(EntityRegistry.GetGameSetting(unType, _T("UniqueCategory")));
            if (bAvailable && pCategory != NULL && !pCategory->GetString().empty())
            {
                for (int iSlot(INVENTORY_START_BACKPACK); iSlot < INVENTORY_END_BACKPACK; ++iSlot)
                {
                    IInventoryItem *pItem(pPlayer->GetItem(iSlot));
                    if (pItem == NULL)
                        continue;
                    if (pItem->GetAsConsumable() != NULL && !pItem->GetAsConsumable()->GetUniqueCategory().empty())
                    {
                        bAvailable = (CompareNoCase(pItem->GetAsConsumable()->GetUniqueCategory(), pCategory->GetString()) != 0);

                        if (!bAvailable)
                            break;
                    }
                }
            }

            ICvar *pPrerequisite(EntityRegistry.GetGameSetting(unType, _T("Prerequisite")));
            if (bAvailable && pPrerequisite != NULL && !pPrerequisite->GetString().empty() && !pTeam->HasBuilding(pPrerequisite->GetString()))
                bAvailable = false;

            Trigger(UITRIGGER_CONSUMABLE_ITEM_AVAILABLE, (bAvailable || ICvar::GetBool(_T("sv_itemH4x"))), iItemCount);

            ++iItemCount;
        }
    }

    bool bTooltipVisible(false);
    if (!GameClient.GetLoadoutUnitMouseover().empty())
    {
        tstring sPrereq(ICvar::GetString(_T("Player_") + GameClient.GetLoadoutUnitMouseover() + _T("_Prerequisite")));

        if (!sPrereq.empty() && !GameClient.TeamHasBuilding(pLocalClient->GetTeam(), sPrereq))
        {
            bTooltipVisible = true;
            Trigger(UITRIGGER_LOADOUT_TOOLTIP_TITLE, _T("Not Researched"));
            Trigger(UITRIGGER_LOADOUT_TOOLTIP_MESSAGE, _T("This unit requires a ") + ICvar::GetString(sPrereq + _T("_Name")) + _T("."));
        }
    }
    else if (pLocalClient->HasNetFlags(ENT_NET_FLAG_QUEUED))
    {
        bTooltipVisible = true;
        Trigger(UITRIGGER_LOADOUT_TOOLTIP_TITLE, _T("Spawn Queued"));
        Trigger(UITRIGGER_LOADOUT_TOOLTIP_MESSAGE, _T("Your team has more players than the other team, so you must wait for a teammate to die before you can spawn."));
    }

    Trigger(UITRIGGER_LOADOUT_SPAWN_QUEUE_POSITION, pLocalClient->GetSpawnQueuePosition());

    if (pTeam != NULL)
    {
        ivector vClients(pTeam->GetClientList());
        int iNumQueued(0);

        for (ivector_it it(vClients.begin()); it != vClients.end(); it++)
        {
            CEntityClientInfo *pTeamClient(GameClient.GetClientInfo(*it));

            if (pTeamClient != NULL && pTeamClient->HasNetFlags(ENT_NET_FLAG_QUEUED))
                iNumQueued++;
        }

        Trigger(UITRIGGER_LOADOUT_SPAWN_QUEUE_SIZE, iNumQueued);
    }

    Trigger(UITRIGGER_LOADOUT_TOOLTIP_VISIBLE, bTooltipVisible);
}


/*====================
  CGameInterfaceManager::UpdateLobby
  ====================*/
void    CGameInterfaceManager::UpdateLobby()
{
    // Update phase timer cvar
    cg_lobbyPhaseTimer =
        XtoA(INT_FLOOR(MsToMin(GameClient.GetRemainingPhaseTime())), FMT_PADZERO, 2) +
        _T(":") +
        XtoA(INT_FLOOR(MsToSec(GameClient.GetRemainingPhaseTime() % 60000)), FMT_PADZERO, 2);

    CEntityClientInfo *pLocalClient(GameClient.GetLocalClient());
    if (pLocalClient == NULL)
        return;

    CEntityTeamInfo *pLocalTeam(Game.GetTeam(pLocalClient->GetTeam()));

    // Check for phase changes
    EGamePhase eCurrentPhase(GameClient.GetGamePhase());
    Trigger(UITRIGGER_LOBBY_PHASE, eCurrentPhase);

    bool bLobbyRequiresUpdate(LobbyRequiresUpdate());

    if (bLobbyRequiresUpdate)
        ForceUpdate();

    //if (m_eGamePhase != eCurrentPhase || m_bForceUpdate)
    {
        m_eGamePhase = eCurrentPhase;
        switch (eCurrentPhase)
        {
        case GAME_PHASE_WAITING_FOR_PLAYERS:
            Trigger(UITRIGGER_LOBBY_STATUS, _T("Phase I: ^wWaiting for players"));
            if (pLocalClient->GetTeam() < 1)
                Trigger(UITRIGGER_LOBBY_PHASE_INSTRUCTION, _T("Join a team"));
            else
                Trigger(UITRIGGER_LOBBY_PHASE_INSTRUCTION, _T(""));
            Trigger(UITRIGGER_LOBBY_PHASE_DESCRIPTION,
                _T("The server is waiting for each team to reach the minimum player requirement. ")
                _T("If you would like to be a commander, press the button below. ")
                _T("In the next phase each team will elect a commander."));
            Trigger(UITRIGGER_LOBBY_PHASE_TIMER, false);
            Trigger(UITRIGGER_LOBBY_DECLINE_BUTTON, false);
            break;

        case GAME_PHASE_SELECTING_COMMANDER:
            Trigger(UITRIGGER_LOBBY_STATUS, _T("Phase II: ^wElecting commanders"));
            if (pLocalClient->GetVote() == -1 && pLocalTeam != NULL && pLocalTeam->GetCommanderClientID() == -1)
                Trigger(UITRIGGER_LOBBY_PHASE_INSTRUCTION, _T("Vote for your commander"));
            else
                Trigger(UITRIGGER_LOBBY_PHASE_INSTRUCTION, _T(""));

            if (pLocalTeam == NULL || pLocalTeam->GetCommanderClientID() == -1)
            {
                Trigger(UITRIGGER_LOBBY_PHASE_DESCRIPTION,
                    _T("To vote for a commander, simply check the box to the left of the candidate's name. ")
                    _T("You may change your vote at any time during this phase."));
                Trigger(UITRIGGER_LOBBY_PHASE_TIMER, true);
                Trigger(UITRIGGER_LOBBY_DECLINE_BUTTON, false);
            }
            else
            {
                Trigger(UITRIGGER_LOBBY_PHASE_DESCRIPTION,
                    _T("Your team already has a commander, be patient while the other team elects one."));
                Trigger(UITRIGGER_LOBBY_PHASE_TIMER, true);
                Trigger(UITRIGGER_LOBBY_DECLINE_BUTTON, false);
            }

            break;

        case GAME_PHASE_SELECTING_OFFICERS:
            Trigger(UITRIGGER_LOBBY_STATUS, _T("Phase III: ^wSelecting Officers"));
            if (pLocalClient->HasFlags(CLIENT_INFO_IS_COMMANDER) &&
                pLocalTeam != NULL)
            {
                if (pLocalTeam->GetNumOfficers() < pLocalTeam->GetMaxOfficers())
                    Trigger(UITRIGGER_LOBBY_PHASE_INSTRUCTION, _T("Select your officers"));
                else
                    Trigger(UITRIGGER_LOBBY_PHASE_INSTRUCTION, _T(""));
                Trigger(UITRIGGER_LOBBY_PHASE_DESCRIPTION,
                    _T("Select players from your team to be officers. ")
                    _T("Officers are leaders in the field with special responsibilities. "));
            }
            else if (pLocalClient->HasFlags(CLIENT_INFO_IS_OFFICER))
            {
                Trigger(UITRIGGER_LOBBY_PHASE_INSTRUCTION, _T("Do you accept?"));
                Trigger(UITRIGGER_LOBBY_PHASE_DESCRIPTION,
                    _T("Your commander has selected you to be an officer. ")
                    _T("Officers are leaders in the field with special responsibilities. ")
                    _T("If you do not want this responsibility, press the \"Decline Officer\" button below. "));
            }
            else
            {
                Trigger(UITRIGGER_LOBBY_PHASE_INSTRUCTION, _T(""));
                Trigger(UITRIGGER_LOBBY_PHASE_DESCRIPTION,
                    _T("Your commander is currently selecting players to be officers. ")
                    _T("If all officer positions are not filled by the end of this phase, they will be randomly assigned."));
            }
            Trigger(UITRIGGER_LOBBY_PHASE_TIMER, true);
            if (pLocalClient->HasFlags(CLIENT_INFO_IS_OFFICER))
                Trigger(UITRIGGER_LOBBY_DECLINE_BUTTON, true);
            else
                Trigger(UITRIGGER_LOBBY_DECLINE_BUTTON, false);
            break;

        case GAME_PHASE_FORMING_SQUADS:
            Trigger(UITRIGGER_LOBBY_STATUS, _T("Phase IV: ^wForming Squads"));
            if (pLocalClient->GetSquad() == INVALID_SQUAD && !pLocalClient->HasFlags(CLIENT_INFO_IS_COMMANDER))
                Trigger(UITRIGGER_LOBBY_PHASE_INSTRUCTION, _T("Join a squad"));
            else
                Trigger(UITRIGGER_LOBBY_PHASE_INSTRUCTION, _T(""));
            Trigger(UITRIGGER_LOBBY_PHASE_DESCRIPTION,
                _T("In the final phase players choose battle groups to join. You do so by clicking an ")
                _T("available join button from the various battle groups. If players fail to choose a group they ")
                _T("will be randomly assigned to one after the countdown. This is the last phase and the game will ")
                _T("begin immediately after."));
            Trigger(UITRIGGER_LOBBY_PHASE_TIMER, true);
            Trigger(UITRIGGER_LOBBY_DECLINE_BUTTON, false);
            break;

        case GAME_PHASE_ACTIVE:
            Trigger(UITRIGGER_LOBBY_STATUS, _T("Game in progress"));
            if (pLocalClient->GetTeam() < 1)
                Trigger(UITRIGGER_LOBBY_PHASE_INSTRUCTION, _T("Join a team"));
            else if (pLocalClient->GetSquad() == INVALID_SQUAD && !pLocalClient->HasFlags(CLIENT_INFO_IS_COMMANDER))
                Trigger(UITRIGGER_LOBBY_PHASE_INSTRUCTION, _T("Join a squad"));
            else
                Trigger(UITRIGGER_LOBBY_PHASE_INSTRUCTION, _T(""));
            Trigger(UITRIGGER_LOBBY_PHASE_DESCRIPTION,
                _T("A game is currently in progress.  After you join a team and then a sqaud on that team, you ")
                _T("will automatically enter the game."));
            Trigger(UITRIGGER_LOBBY_PHASE_TIMER, false);
            Trigger(UITRIGGER_LOBBY_DECLINE_BUTTON, false);
            break;

        case GAME_PHASE_WARMUP:
            Trigger(UITRIGGER_LOBBY_STATUS, _T("Warmup"));
            if (pLocalClient->GetTeam() < 1)
                Trigger(UITRIGGER_LOBBY_PHASE_INSTRUCTION, _T("Join a team"));
            else if (pLocalClient->GetSquad() == INVALID_SQUAD && !pLocalClient->HasFlags(CLIENT_INFO_IS_COMMANDER))
                Trigger(UITRIGGER_LOBBY_PHASE_INSTRUCTION, _T("Join a squad"));
            else
                Trigger(UITRIGGER_LOBBY_PHASE_INSTRUCTION, _T(""));
            Trigger(UITRIGGER_LOBBY_PHASE_DESCRIPTION,
                _T("Warmup message."));
            Trigger(UITRIGGER_LOBBY_PHASE_TIMER, false);
            Trigger(UITRIGGER_LOBBY_DECLINE_BUTTON, false);
            break;
        }
    }

    // Spectators
    CEntityTeamInfo *pSpectators(GameClient.GetTeam(0));
    if (pSpectators != NULL && 
        (pSpectators->RosterChanged() || bLobbyRequiresUpdate))
    {
        Execute(UITRIGGER_LOBBY_SPECTATOR_LIST, _T("ListBoxCmd('clear');"));

        const ivector &vSpectators(pSpectators->GetClientList());
        for (ivector_cit cit(vSpectators.begin()); cit != vSpectators.end(); ++cit)
        {
            CEntityClientInfo *pClient(GameClient.GetClientInfo(*cit));
            if (pClient == NULL)
                continue;

            Execute(UITRIGGER_LOBBY_SPECTATOR_LIST, _T("ListBoxCmd('add ") + pClient->GetName() + _T("');"));
        }
    }

    if (pLocalClient->GetTeam() == 0)
    {
        Trigger(UITRIGGER_LOBBY_REQUEST_BUTTON, false);
        Trigger(UITRIGGER_LOBBY_SPECTATOR_BUTTON, (eCurrentPhase == GAME_PHASE_ACTIVE || eCurrentPhase == GAME_PHASE_WARMUP));
    }
    else
        Trigger(UITRIGGER_LOBBY_SPECTATOR_BUTTON, true);

    for (int iTeam(1); iTeam <= 2; ++iTeam)
    {
        uint uiNumUnassigned(1);

        CEntityTeamInfo *pTeam(GameClient.GetTeam(iTeam));
        if (pTeam == NULL)
            continue;

        if (!GameClient.CanJoinTeam(iTeam))
            Trigger(UITRIGGER_LOBBY_TEAM_BUTTON, false, iTeam - 1);
        else
            Trigger(UITRIGGER_LOBBY_TEAM_BUTTON, true, iTeam - 1);

        if (pLocalClient->GetTeam() == iTeam)
        {
            if (eCurrentPhase <= GAME_PHASE_SELECTING_COMMANDER &&
                pTeam->GetNumCandidates() < pTeam->GetMaxCandidates() &&
                !pLocalClient->HasFlags(CLIENT_INFO_WANTS_TO_COMMAND) &&
                pTeam->GetCommanderClientID() == -1)
                Trigger(UITRIGGER_LOBBY_REQUEST_BUTTON, true);
            else
                Trigger(UITRIGGER_LOBBY_REQUEST_BUTTON, false);
        }

        if (pLocalClient->HasFlags(CLIENT_INFO_IS_COMMANDER) && pLocalClient->GetTeam() == iTeam)
            Trigger(UITRIGGER_LOBBY_TEAM_OFFICER_DEMOTE_BUTTON, true, iTeam - 1);
        else
            Trigger(UITRIGGER_LOBBY_TEAM_OFFICER_DEMOTE_BUTTON, false, iTeam - 1);

        IPlayerEntity *pCommander(GameClient.GetPlayer(pTeam->GetCommanderClientID()));
        if (pCommander != NULL)
            Trigger(UITRIGGER_LOBBY_COMMANDER_NAME, pCommander->GetClientName(), iTeam - 1);
        else
            Trigger(UITRIGGER_LOBBY_COMMANDER_NAME, _T(""), iTeam - 1);

        Execute(UITRIGGER_LOBBY_TEAM_LIST, _T("ClearData();"), iTeam - 1);
        Execute(UITRIGGER_LOBBY_TEAM_CANDIDATE_LIST, _T("ClearData();"), iTeam - 1);
        
        // Hide officer/squads by default
        for (uint ui(0); ui <= MAX_OFFICERS; ++ui)
        {
            Execute(UITRIGGER_LOBBY_SQUAD_LIST, _T("ClearData();"), iTeam - 1, ui);
            Trigger(UITRIGGER_LOBBY_OFFICER_VISIBLE, 0, iTeam - 1, ui);
            Trigger(UITRIGGER_LOBBY_SQUAD_LIST_NAME, pTeam->GetSquadName(ui), iTeam - 1, ui);
            Trigger(UITRIGGER_LOBBY_SQUAD_LIST_COLOR, pTeam->GetSquadColor(ui), iTeam - 1, ui);

            if (eCurrentPhase >= GAME_PHASE_FORMING_SQUADS && ui < pTeam->GetNumOfficers())
                Trigger(UITRIGGER_LOBBY_SQUAD_LIST_VISIBLE, true, iTeam - 1, ui);
            else
                Trigger(UITRIGGER_LOBBY_SQUAD_LIST_VISIBLE, false, iTeam - 1, ui);

            if (eCurrentPhase >= GAME_PHASE_FORMING_SQUADS)
                Trigger(UITRIGGER_LOBBY_SQUAD_LIST_SIZE, pTeam->GetSquadSize(ui), iTeam - 1, ui);

            if (pTeam->GetSquadSize(ui) >= pTeam->GetMaxSquadSize() ||
                pLocalClient->GetSquad() == ui ||
                pLocalClient->HasFlags(CLIENT_INFO_IS_COMMANDER) ||
                pLocalClient->HasFlags(CLIENT_INFO_IS_OFFICER) ||
                pLocalClient->GetTeam() != iTeam)
                Trigger(UITRIGGER_LOBBY_SQUAD_JOIN_BUTTON, false, iTeam - 1, ui);
            else
                Trigger(UITRIGGER_LOBBY_SQUAD_JOIN_BUTTON, true, iTeam - 1, ui);
        }

        // Unassigned lists
        Execute(UITRIGGER_LOBBY_SQUAD_LIST, _T("ClearData();"), iTeam - 1, MAX_OFFICERS);
        Trigger(UITRIGGER_LOBBY_SQUAD_LIST_VISIBLE, false, iTeam - 1, MAX_OFFICERS);
        Trigger(UITRIGGER_LOBBY_SQUAD_JOIN_BUTTON, false, iTeam - 1, MAX_OFFICERS);
        Trigger(UITRIGGER_LOBBY_SQUAD_LIST_NAME, pTeam->GetSquadName(MAX_OFFICERS), iTeam - 1, MAX_OFFICERS);
        Trigger(UITRIGGER_LOBBY_SQUAD_LIST_COLOR, pTeam->GetSquadColor(MAX_OFFICERS), iTeam - 1, MAX_OFFICERS);

        int iCandidateCount(0);
        int iPlayerCount(0);
        int iAverageSF(0);
        int iAverageLevel(0);
        for (int iClient(0); iClient < pTeam->GetNumClients(); ++iClient)
        {
            CEntityClientInfo *pClient(GameClient.GetClientInfo(pTeam->GetClientIDFromTeamIndex(iClient)));
            if (pClient == NULL)
                continue;

            iAverageSF += pClient->GetPersistantStat(PLAYER_PERSISTANT_SKILLFACTOR);
            iAverageLevel += pClient->GetPersistantStat(PLAYER_PERSISTANT_LEVEL);

            if (pClient->HasFlags(CLIENT_INFO_IS_COMMANDER))
                continue;

            if (pClient->HasFlags(CLIENT_INFO_IS_OFFICER))
            {
                Trigger(UITRIGGER_LOBBY_OFFICER_NAME, pClient->GetName(), iTeam - 1, pClient->GetSquad());
                if (eCurrentPhase == GAME_PHASE_SELECTING_OFFICERS)
                    Trigger(UITRIGGER_LOBBY_OFFICER_VISIBLE, true, iTeam - 1, pClient->GetSquad());
                continue;
            }

            svector vParams;
            vParams.push_back(XtoA(pClient->GetClientNumber()));
            
            if (eCurrentPhase == GAME_PHASE_SELECTING_COMMANDER)
            {
                if (pClient->GetTeam() != pLocalClient->GetTeam())
                    vParams.push_back(_T(""));
                else if (pClient->HasFlags(CLIENT_INFO_WANTS_TO_COMMAND))
                {
                    if (pClient->GetClientNumber() == pLocalClient->GetVote())
                        vParams.push_back(_T("!!/ui/chatgui/checkbox_down2.tga"));
                    else
                        vParams.push_back(_T("!!/ui/chatgui/checkbox_down.tga"));
                }
                else
                {
                    if (pClient->GetVote() == -1)
                        vParams.push_back(_T("!!/ui/elements/squad_red_icon.tga"));
                    else
                        vParams.push_back(_T("!!/ui/elements/squad_green_icon.tga"));
                }
            }
            else if (eCurrentPhase == GAME_PHASE_SELECTING_OFFICERS &&
                pLocalClient->HasFlags(CLIENT_INFO_IS_COMMANDER) &&
                pLocalClient->GetTeam() == iTeam &&
                pClient->HasFlags(CLIENT_INFO_WANTS_TO_COMMAND) &&
                pTeam->GetNumOfficers() < pTeam->GetMaxOfficers())
            {
                vParams.push_back(_T("!!/ui/elements/plus_up.tga"));
            }
            else
            {
                vParams.push_back(_T(""));
            }
            
            vParams.push_back(pTeam->GetClientName(iClient));
            vParams.push_back(XtoA(pClient->GetPersistantStat(PLAYER_PERSISTANT_SKILLFACTOR)));
            vParams.push_back(
                XtoA(pClient->GetPersistantStat(PLAYER_PERSISTANT_WINS)) + _T("-") +
                XtoA(pClient->GetPersistantStat(PLAYER_PERSISTANT_LOSSES)) + _T("-") +
                XtoA(pClient->GetPersistantStat(PLAYER_PERSISTANT_DISCONNECTS)));
            vParams.push_back(XtoA(pClient->GetPing()));
            vParams.push_back(XtoA(pClient->GetPersistantStat(PLAYER_PERSISTANT_LEVEL)));
            if (pClient->HasFlags(CLIENT_INFO_WANTS_TO_COMMAND) && eCurrentPhase <= GAME_PHASE_SELECTING_COMMANDER)
            {
                Execute(UITRIGGER_LOBBY_TEAM_CANDIDATE_LIST, _T("Data('") + ConcatinateArgs(vParams, _T("','")) + _T("');"), iTeam - 1);
                ++iCandidateCount;
            }
            else if (eCurrentPhase >= GAME_PHASE_FORMING_SQUADS && pClient->GetSquad() != INVALID_SQUAD)
            {
                Execute(UITRIGGER_LOBBY_SQUAD_LIST,
                    _T("AppendData('") +
                    XtoA(pClient->GetClientNumber()) + _T("','") +
                    pTeam->GetClientName(iClient) + _T("','") +
                    XtoA(pClient->GetPersistantStat(PLAYER_PERSISTANT_SKILLFACTOR)) + _T("');"),
                    iTeam - 1, pClient->GetSquad());
            }
            else if (eCurrentPhase >= GAME_PHASE_FORMING_SQUADS && pClient->GetSquad() == INVALID_SQUAD)
            {
                Trigger(UITRIGGER_LOBBY_SQUAD_LIST_VISIBLE, true, iTeam - 1, MAX_OFFICERS);
                Trigger(UITRIGGER_LOBBY_SQUAD_LIST_SIZE, ++uiNumUnassigned, iTeam - 1, MAX_OFFICERS);
                Execute(UITRIGGER_LOBBY_SQUAD_LIST,
                    _T("AppendData('") +
                    XtoA(pClient->GetClientNumber()) + _T("','") +
                    pTeam->GetClientName(iClient) + _T("','") +
                    XtoA(pClient->GetPersistantStat(PLAYER_PERSISTANT_SKILLFACTOR)) + _T("');"),
                    iTeam - 1, MAX_OFFICERS);
            }
            else
            {
                Execute(UITRIGGER_LOBBY_TEAM_LIST, _T("Data('") + ConcatinateArgs(vParams, _T("','")) + _T("');"), iTeam - 1);
                ++iPlayerCount;
            }
        }

        if (pTeam->GetNumClients() > 0)
        {
            iAverageSF /= pTeam->GetNumClients();
            iAverageLevel /= pTeam->GetNumClients();
        }

        Trigger(UITRIGGER_LOBBY_TEAM_AVERAGE_SF, iAverageSF, iTeam - 1);
        Trigger(UITRIGGER_LOBBY_TEAM_AVERAGE_LEVEL, iAverageLevel, iTeam - 1);

        if (eCurrentPhase <= GAME_PHASE_SELECTING_COMMANDER)
            Trigger(UITRIGGER_LOBBY_CANDIDATE_LIST_SIZE, iCandidateCount, iTeam - 1);
        if (eCurrentPhase < GAME_PHASE_FORMING_SQUADS)
            Trigger(UITRIGGER_LOBBY_TEAM_LIST_SIZE, iPlayerCount, iTeam - 1);

        Trigger(UITRIGGER_LOBBY_TEAM_PLAYER_COUNT, pTeam->GetNumClients(), iTeam - 1);

        switch (eCurrentPhase)
        {
        case GAME_PHASE_WAITING_FOR_PLAYERS:
            Trigger(UITRIGGER_LOBBY_COMMANDER_VISIBLE, false, iTeam - 1);
            Trigger(UITRIGGER_LOBBY_CANDIDATE_LIST_VISIBLE, true, iTeam - 1);
            Trigger(UITRIGGER_LOBBY_TEAM_LIST_VISIBLE, true, iTeam - 1);
            break;

        case GAME_PHASE_SELECTING_COMMANDER:
            if (pTeam->GetCommanderClientID() == -1)
            {
            Trigger(UITRIGGER_LOBBY_COMMANDER_VISIBLE, false, iTeam - 1);
            Trigger(UITRIGGER_LOBBY_CANDIDATE_LIST_VISIBLE, true, iTeam - 1);
            Trigger(UITRIGGER_LOBBY_TEAM_LIST_VISIBLE, true, iTeam - 1);
            }
            else
            {
            Trigger(UITRIGGER_LOBBY_COMMANDER_VISIBLE, true, iTeam - 1);
            Trigger(UITRIGGER_LOBBY_CANDIDATE_LIST_VISIBLE, false, iTeam - 1);
            Trigger(UITRIGGER_LOBBY_TEAM_LIST_VISIBLE, true, iTeam - 1);
            }

            break;

        case GAME_PHASE_SELECTING_OFFICERS:
            Trigger(UITRIGGER_LOBBY_COMMANDER_VISIBLE, true, iTeam - 1);
            Trigger(UITRIGGER_LOBBY_CANDIDATE_LIST_VISIBLE, false, iTeam - 1);
            Trigger(UITRIGGER_LOBBY_TEAM_LIST_VISIBLE, true, iTeam - 1);
            break;

        case GAME_PHASE_FORMING_SQUADS:
            Trigger(UITRIGGER_LOBBY_COMMANDER_VISIBLE, true, iTeam - 1);
            Trigger(UITRIGGER_LOBBY_CANDIDATE_LIST_VISIBLE, false, iTeam - 1);
            Trigger(UITRIGGER_LOBBY_TEAM_LIST_VISIBLE, false, iTeam - 1);
            break;

        case GAME_PHASE_ACTIVE:
            Trigger(UITRIGGER_LOBBY_COMMANDER_VISIBLE, true, iTeam - 1);
            Trigger(UITRIGGER_LOBBY_CANDIDATE_LIST_VISIBLE, false, iTeam - 1);
            Trigger(UITRIGGER_LOBBY_TEAM_LIST_VISIBLE, false, iTeam - 1);
            break;

        case GAME_PHASE_WARMUP:
            Trigger(UITRIGGER_LOBBY_COMMANDER_VISIBLE, true, iTeam - 1);
            Trigger(UITRIGGER_LOBBY_CANDIDATE_LIST_VISIBLE, false, iTeam - 1);
            Trigger(UITRIGGER_LOBBY_TEAM_LIST_VISIBLE, false, iTeam - 1);
            break;
        }
    }

    CStateString *pState = Host.GetStateString(1);  //STATE_STRING_SERVER_INFO

    if (pState != NULL)
    {
        Trigger(UITRIGGER_LOBBY_SERVER_NAME, pState->GetString(_T("svr_name")));
        Trigger(UITRIGGER_LOBBY_SERVER_LOCATION, pState->GetString(_T("svr_location")));
        Trigger(UITRIGGER_LOBBY_SERVER_MAP, GameClient.GetWorldPointer()->GetName());
        Trigger(UITRIGGER_LOBBY_SERVER_PLAYERS, XtoA(GameClient.GetNumClients()) + _T("/") + XtoA(pState->GetString(_T("svr_maxPlayers"))));
        Trigger(UITRIGGER_LOBBY_SERVER_MIN_PLAYERS, GameClient.GetWorldPointer()->GetMinPlayersPerTeam());
        Trigger(UITRIGGER_LOBBY_SERVER_PING, XtoA(pLocalClient->GetPing()));
        Trigger(UITRIGGER_LOBBY_SERVER_MAP_VERSION, K2System.GetVersionString());
        Trigger(UITRIGGER_LOBBY_SERVER_MATCHES_PLAYED, pState->GetString(_T("sv_totalMatches")));
        Trigger(UITRIGGER_LOBBY_SERVER_MIN_KARMA, pState->GetString(_T("svr_minKarma")));

        switch (pState->GetInt(_T("svr_status")))
        {
        case 0:
            Trigger(UITRIGGER_LOBBY_SERVER_STATUS, _T("Beginner"));
            break;

        case 1:
            Trigger(UITRIGGER_LOBBY_SERVER_STATUS, _T("General"));
            break;

        case 2:
            Trigger(UITRIGGER_LOBBY_SERVER_STATUS, _T("Veteran"));
            break;

        default:
            Trigger(UITRIGGER_LOBBY_SERVER_STATUS, _T("All"));
            break;
        }
    }
}


/*====================
  CGameInterfaceManager::UpdateScores
  ====================*/
void    CGameInterfaceManager::UpdateScores()
{
    for (int iTeam(0); iTeam < 2; ++iTeam)
    {
        CEntityTeamInfo *pTeam(GameClient.GetTeam(iTeam + 1));
        if (pTeam == NULL)
            continue;

        Trigger(UITRIGGER_LOBBY_TEAM_RACE, pTeam->GetDefinition()->GetName(), iTeam);
        Trigger(UITRIGGER_TEAM_EXPERIENCE, pTeam->GetExperience(), iTeam);
        Trigger(UITRIGGER_LOBBY_TEAM_AVERAGE_SF, pTeam->GetAverageSF(), iTeam);

        IBuildingEntity *pCommandCenter(GameClient.GetBuildingEntity(pTeam->GetBaseBuildingIndex()));
        if (pCommandCenter != NULL)
        {
            Trigger(UITRIGGER_SCOREBOARD_COMMAND_CENTER_HEALTH_PERCENT, pCommandCenter->GetHealthPercent(), iTeam);
            Trigger(UITRIGGER_SCOREBOARD_COMMAND_CENTER_HEALTH_VALUE, pCommandCenter->GetHealth(), iTeam);
            Trigger(UITRIGGER_SCOREBOARD_COMMAND_CENTER_HEALTH_MAX, pCommandCenter->GetMaxHealth(), iTeam);
        }

        CEntityClientInfo *pCommanderClient(GameClient.GetClientInfo(pTeam->GetCommanderClientID()));
        if (pCommanderClient != NULL)
        {
            Trigger(UITRIGGER_SCOREBOARD_COMMANDER, pCommanderClient->GetName(), iTeam);
            Trigger(UITRIGGER_SCOREBOARD_COMMANDER_RECORD, XtoA(pCommanderClient->GetPersistantStat(PLAYER_PERSISTANT_WINS)) + _T("/") + XtoA(pCommanderClient->GetPersistantStat(PLAYER_PERSISTANT_LOSSES)) + _T("/") + XtoA(pCommanderClient->GetPersistantStat(PLAYER_PERSISTANT_DISCONNECTS)), iTeam);
            Trigger(UITRIGGER_SCOREBOARD_COMMANDER_SF, XtoA(pCommanderClient->GetPersistantStat(PLAYER_PERSISTANT_SKILLFACTOR)), iTeam);
        }
        else
        {
            Trigger(UITRIGGER_SCOREBOARD_COMMANDER, _T(""), iTeam);
            Trigger(UITRIGGER_SCOREBOARD_COMMANDER_RECORD, _T(""), iTeam);
            Trigger(UITRIGGER_SCOREBOARD_COMMANDER_SF, _T(""), iTeam);
        }

        Trigger(UITRIGGER_SCOREBOARD_NUM_PLAYERS, pTeam->GetNumActiveClients(), iTeam);

        vector<CEntityClientInfo*> vSorted;
        int iPlayer(0);

        for (; iPlayer < pTeam->GetNumClients(); ++iPlayer)
        {
            CEntityClientInfo *pClient(GameClient.GetClientInfo(pTeam->GetClientIDFromTeamIndex(iPlayer)));
            
            if (pClient == NULL)
                continue;

            vSorted.push_back(pClient);
        }

        sort(vSorted.begin(), vSorted.end(), CompareByExperience);

        iPlayer = 0;

        for (vector<CEntityClientInfo*>::iterator it(vSorted.begin()); it != vSorted.end(); it++)
        {
            CEntityClientInfo *pClient(*it);

            if (pTeam->GetCommanderClientID() == pClient->GetClientNumber())
                continue;

            Trigger(UITRIGGER_SCOREBOARD_PLAYER_ACTIVE, true, iTeam, iPlayer);
            Trigger(UITRIGGER_SCOREBOARD_PLAYER_SQUAD, pTeam->GetSquadColor(pClient->GetSquad()), iTeam, iPlayer);
            Trigger(UITRIGGER_SCOREBOARD_PLAYER_LEVEL, pClient->GetLevel(), iTeam, iPlayer);
            Trigger(UITRIGGER_SCOREBOARD_PLAYER_NAME, pClient->GetName(), iTeam, iPlayer);
            Trigger(UITRIGGER_SCOREBOARD_PLAYER_PING, pClient->GetPing(), iTeam, iPlayer);
            Trigger(UITRIGGER_SCOREBOARD_PLAYER_EXP, INT_ROUND(pClient->GetExperience()), iTeam, iPlayer);
            Trigger(UITRIGGER_SCOREBOARD_PLAYER_KILLS, pClient->GetKills(), iTeam, iPlayer);
            Trigger(UITRIGGER_SCOREBOARD_PLAYER_DEATHS, pClient->GetDeaths(), iTeam, iPlayer);
            Trigger(UITRIGGER_SCOREBOARD_PLAYER_ASSISTS, pClient->GetAssists(), iTeam, iPlayer);
            Trigger(UITRIGGER_SCOREBOARD_PLAYER_OFFICER, pTeam->IsOfficer(iPlayer), iTeam, iPlayer);
            Trigger(UITRIGGER_SCOREBOARD_PLAYER_SF, pClient->GetPersistantStat(PLAYER_PERSISTANT_SKILLFACTOR), iTeam, iPlayer);

            if (pTeam->GetClientIDFromTeamIndex(iPlayer) == GameClient.GetLocalClientNum())
                Trigger(UITRIGGER_SCOREBOARD_PLAYER_MUTED, iTeam, iPlayer, 2);      // 2 signifies "Hide the mute button"
            else
                Trigger(UITRIGGER_SCOREBOARD_PLAYER_MUTED, iTeam, iPlayer, GameClient.IsMuted(pTeam->GetClientIDFromTeamIndex(iPlayer)));

            if (pClient->GetClientNumber() == GameClient.GetLocalClientNum())
            {
                Trigger(UITRIGGER_ENDGAME_DATA_KILLS, pClient->GetKills());
                Trigger(UITRIGGER_ENDGAME_DATA_ASSISTS, pClient->GetAssists());
                Trigger(UITRIGGER_ENDGAME_DATA_DEATHS, pClient->GetDeaths());
                Trigger(UITRIGGER_ENDGAME_DATA_PLAYER_DAMAGE, pClient->GetPlayerDamage());
                Trigger(UITRIGGER_ENDGAME_DATA_BUILDINGS_RAZED, pClient->GetRazes());
                Trigger(UITRIGGER_ENDGAME_DATA_BUILDING_DAMAGE, pClient->GetBuildingDamage());
                Trigger(UITRIGGER_ENDGAME_DATA_HP_HEALED, pClient->GetHealed());
                Trigger(UITRIGGER_ENDGAME_DATA_RESURRECTS, pClient->GetResurrects());
                Trigger(UITRIGGER_ENDGAME_DATA_HP_REPAIRED, pClient->GetRepaired());
            }

            iPlayer++;
        }

        for (; iPlayer < MAX_DISPLAY_PLAYERS; ++iPlayer)
            Trigger(UITRIGGER_SCOREBOARD_PLAYER_ACTIVE, false, iTeam, iPlayer);
    }
}


/*====================
  CGameInterfaceManager::UpdateVoiceCommands
  ====================*/
void    CGameInterfaceManager::UpdateVoiceCommands()
{
    if (GameClient.GetLocalClient() == NULL)
        return;

    CEntityTeamInfo *pTeam(GameClient.GetTeam(GameClient.GetLocalClient()->GetTeam()));

    if (pTeam == NULL)
        return;

    if (!GameClient.IsVCMenuActive())
    {
        Trigger(UITRIGGER_VOICE_COMMAND_MAIN, false);
        Trigger(UITRIGGER_VOICE_COMMAND_SUB, false);
        return;
    }

    VCMap *pMap(GameClient.GetVCMap());
    VCCategory *pCategory(GameClient.GetActiveVCCategory());
    svector vsParams;

    if (pMap == NULL)
    {
        Trigger(UITRIGGER_VOICE_COMMAND_MAIN, false);
        Trigger(UITRIGGER_VOICE_COMMAND_SUB, false);
        return;
    }

    Trigger(UITRIGGER_VOICE_COMMAND_MAIN, true);
    Execute(UITRIGGER_VOICE_COMMAND_CATEGORIES, _T("ClearData();"));

    int iCategoryCount(0);
    for (VCMap_it it(pMap->begin()); it != pMap->end(); it++)
    {
        vsParams.push_back(XtoA(iCategoryCount++));
        vsParams.push_back(it->second.sDesc);
        vsParams.push_back(Input.ToString(it->first));

        Trigger(UITRIGGER_VOICE_COMMAND_CATEGORIES, vsParams); 

        vsParams.clear();
    }

    vsParams.push_back(_T("Cancel"));
    vsParams.push_back(_T("Cancel"));
    vsParams.push_back(_T("ESC"));

    Trigger(UITRIGGER_VOICE_COMMAND_CATEGORIES, vsParams);
    
    vsParams.clear();

    if (!GameClient.IsVCSubMenuActive() || pCategory == NULL)
    {
        Trigger(UITRIGGER_VOICE_COMMAND_SUB, false);
        return;
    }

    Trigger(UITRIGGER_VOICE_COMMAND_SUB, true);
    Execute(UITRIGGER_VOICE_COMMAND_SUB_ITEMS, _T("ClearData();"));

    iCategoryCount = 0;
    tstring sRace(LowerString(pTeam->GetDefinition()->GetName()));
    for (map<EButton, VCSubMap>::iterator it(pCategory->mapSubItems.begin()); it != pCategory->mapSubItems.end(); it++)
    {
        VCSubMap_it findit(it->second.find(sRace));

        if (findit != it->second.end())
        {
            vsParams.push_back(XtoA(iCategoryCount++));
            vsParams.push_back(findit->second.sDesc);
            vsParams.push_back(Input.ToString(it->first));

            Trigger(UITRIGGER_VOICE_COMMAND_SUB_ITEMS, vsParams);

            vsParams.clear();
        }
    }

    vsParams.push_back(_T("Cancel"));
    vsParams.push_back(_T("Cancel"));
    vsParams.push_back(_T("ESC"));

    Trigger(UITRIGGER_VOICE_COMMAND_SUB_ITEMS, vsParams);
    Trigger(UITRIGGER_VOICE_COMMAND_SUB_TITLE, pCategory->sDesc);
}


/*====================
  CGameInterfaceManager::Update
  ====================*/
void    CGameInterfaceManager::Update()
{
    PROFILE("CGameInterfaceManager::Update");

    if (ui_forceUpdate)
        ForceUpdate();

    // Default mouse behavior
    Input.SetCursorRecenter(CURSOR_GAME, BOOL_FALSE);
    Input.SetCursorConstrained(CURSOR_GAME, cg_constrainCursor ? BOOL_TRUE : BOOL_FALSE);
    Input.SetCursorHidden(CURSOR_GAME, BOOL_FALSE);
    Input.SetCursorFrozen(CURSOR_GAME, BOOL_FALSE);

    tstring sRace(_T(""));
    int iTeam(0);
    CEntityClientInfo *pLocalClient(GameClient.GetLocalClient());
    if (pLocalClient != NULL)
    {
        CEntityTeamInfo *pTeam(GameClient.GetTeam(pLocalClient->GetTeam()));

        if (pTeam != NULL && pTeam->GetDefinition() != NULL)
        {
            sRace = LowerString(pTeam->GetDefinition()->GetName());
            iTeam = pTeam->GetTeamID();
            Trigger(UITRIGGER_ECONOMY_INTERVAL, pTeam->GetEconomyIntervalPercent());
        }
        else
        {
            Console.Warn << _T("WARNING: Player is using an invalid team!") << newl;
        }
    }

    if (m_eCurrentInterface != GameClient.GetCurrentInterface() || GameClient.InterfaceNeedsUpdate())
        ForceUpdate();

    m_eCurrentInterface = GameClient.GetCurrentInterface();

    switch (m_eCurrentInterface)
    {
    case CG_INTERFACE_MESSAGE:
        UIManager.SetActiveInterface(_T("game_message"));
        break;

    case CG_INTERFACE_LOBBY:
        UIManager.SetActiveInterface(_T("game_lobby"));
        UpdateLobby();
        UpdateVoiceCommands();
        break;

    case CG_INTERFACE_LOADOUT:
        UIManager.SetActiveInterface(_T("game_loadout"));
        UpdatePlayer();
        UpdatePlayerDetails();
        UpdateInventory();
        UpdateLoadout();
        UpdateVoiceCommands();
        break;

    case CG_INTERFACE_SACRIFICED:
        UIManager.SetActiveInterface(_T("game_sacrificed"));
        UpdateVoiceCommands();
        break;

    case CG_INTERFACE_SPAWN:
        UIManager.SetActiveInterface(_T("game_spawn"));
        UpdateVoiceCommands();
        break;

    case CG_INTERFACE_PLAYER:
        UIManager.SetActiveInterface(_T("game"));

        Input.SetCursorRecenter(CURSOR_GAME, BOOL_TRUE);
        Input.SetCursorHidden(CURSOR_GAME, BOOL_TRUE);
        Input.SetCursorConstrained(CURSOR_GAME, BOOL_TRUE);

        UpdatePlayer();
        UpdateOfficer();
        UpdateVoiceCommands();
        break;

    case CG_INTERFACE_DEAD:
        UIManager.SetActiveInterface(_T("game_dead"));

        if (pLocalClient != NULL && pLocalClient->IsDemoAccount())
        {
            Input.SetCursorRecenter(CURSOR_GAME, BOOL_FALSE);
            Input.SetCursorHidden(CURSOR_GAME, BOOL_FALSE);
            Input.SetCursorConstrained(CURSOR_GAME, BOOL_FALSE);
        }
        else
        {
            Input.SetCursorRecenter(CURSOR_GAME, BOOL_TRUE);
            Input.SetCursorHidden(CURSOR_GAME, BOOL_TRUE);
            Input.SetCursorConstrained(CURSOR_GAME, BOOL_TRUE);
        }

        UpdatePlayer();
        UpdateVoiceCommands();
        break;

    case CG_INTERFACE_COMMANDER_INFO:   // TODO: Might want something slightly diffrent here, since the "player" tab is useless to the commander
    case CG_INTERFACE_PLAYER_INFO:
        UIManager.SetActiveInterface(_T("game_info"));
        UpdateScores();
        UpdatePlayer();
        UpdatePlayerDetails();
        UpdateBuildings();
        UpdateVoiceCommands();
        break;

    case CG_INTERFACE_PLAYER_BUILD:
        UIManager.SetActiveInterface(_T("game_player_build"));
        if (GameClient.GetBuildingRotate() && !ReplayManager.IsPlaying())
        {
            Input.SetCursorFrozen(CURSOR_GAME, BOOL_TRUE);
            Input.SetCursorHidden(CURSOR_GAME, BOOL_TRUE);
        }
        UpdatePlayer();
        UpdateVoiceCommands();
        UpdateBuildButtons();
        break;

    case CG_INTERFACE_PLAYER_OFFICER:
        UIManager.SetActiveInterface(_T("game_officer"));
        UpdateVoiceCommands();
        break;

    case CG_INTERFACE_COMMANDER:
        UIManager.SetActiveInterface(_T("game_commander"));
        if (!ReplayManager.IsPlaying() &&
            (GameClient.GetCurrentSnapshot()->IsButtonDown(GAME_CMDR_BUTTON_DRAGSCROLL) || GameClient.GetBuildingRotate()))
        {
            Input.SetCursorFrozen(CURSOR_GAME, BOOL_TRUE);
            Input.SetCursorHidden(CURSOR_GAME, BOOL_TRUE);
        }
        UpdateCommander();
        UpdateCommanderSquads();
        UpdateVoiceCommands();
        UpdateBuildButtons();
        break;

    case CG_INTERFACE_GAME_OVER:
        UIManager.SetActiveInterface(_T("game_end_stats"));
        UpdateGameOver();
        UpdateVoiceCommands();
        break;

    case CG_INTERFACE_MENU:
        UIManager.SetActiveInterface(_T("game_menu"));
        UpdateVoiceCommands();
        break;

    case CG_INTERFACE_PURCHASE:
        UIManager.SetActiveInterface(_T("credit_card"));
        break;

    case CG_INTERFACE_NONE:
        Host.DisplayActiveInterface();
        break;

    case CG_INTERFACE_STANDBY:
        UIManager.SetActiveInterface(_T("game_standby"));
        break;

    case CG_INTERFACE_OBSERVER:
        UIManager.SetActiveInterface(_T("game_observer"));

        if (pLocalClient != NULL && pLocalClient->IsDemoAccount() && pLocalClient->GetDemoTimeRemaining() < GameClient.GetCurrentGameLength())
        {
            Input.SetCursorRecenter(CURSOR_GAME, BOOL_FALSE);
            Input.SetCursorHidden(CURSOR_GAME, BOOL_FALSE);
            Input.SetCursorConstrained(CURSOR_GAME, BOOL_FALSE);
        }
        else
        {
            Input.SetCursorRecenter(CURSOR_GAME, BOOL_TRUE);
            Input.SetCursorHidden(CURSOR_GAME, BOOL_TRUE);
            Input.SetCursorConstrained(CURSOR_GAME, BOOL_TRUE);
        }

        UpdateVoiceCommands();
        break;

    default:
        Console.Warn << _T("CGameClient::Frame() - Invalid interface") << newl;
        break;
    }

    if (pLocalClient != NULL)
        Trigger(UITRIGGER_DEMO_ACCOUNT, pLocalClient->IsDemoAccount());

    if (Game.GetGameTime() - m_uiLastBuildingAttackAlertTime > cg_buildingAttackAlertTime)
    {
        svector vParams;
        vParams.push_back(_T("false"));
        vParams.push_back(_T(""));
        Trigger(UITRIGGER_BUILDING_ATTACK_ALERT, vParams);
    }

    if (GameClient.ForceMouseHidden())
        Input.SetCursorHidden(CURSOR_GAME, ECursorBool(GameClient.IsMouseHidden()));

    if (GameClient.ForceMouseCentered())
        Input.SetCursorRecenter(CURSOR_GAME, ECursorBool(GameClient.IsMouseCentered()));

    // Check for phase changes
    EGamePhase eCurrentPhase(GameClient.GetGamePhase());
    Trigger(UITRIGGER_LOBBY_PHASE, eCurrentPhase);

    if (m_bShowScoreOverlay)
    {
        UIManager.SetOverlayInterface(_T("game_score_overlay"));
        if (UIManager.GetOverlayInterface())
            UIManager.GetOverlayInterface()->SetAlwaysUpdate(true);
        UpdateScores();
    }
    else if (ReplayManager.IsPlaying() && m_eCurrentInterface != CG_INTERFACE_NONE)
    {
        Input.SetCursorRecenter(CURSOR_GAME, BOOL_FALSE);
        Input.SetCursorConstrained(CURSOR_GAME, cg_constrainCursor ? BOOL_TRUE : BOOL_FALSE);
        Input.SetCursorHidden(CURSOR_GAME, BOOL_FALSE);
        Input.SetCursorFrozen(CURSOR_GAME, BOOL_FALSE);

        UIManager.SetOverlayInterface(_T("game_replay_control"));

        if (UIManager.GetOverlayInterface())
            UIManager.GetOverlayInterface()->SetAlwaysUpdate(true);

        UpdateReplay();
    }
    else
    {
        UIManager.ClearOverlayInterface();
    }

    Trigger(UITRIGGER_RACE, sRace);
    Trigger(UITRIGGER_TEAM, iTeam);
}


/*====================
  CGameInterfaceManager::GetEndGameDetailStats
  ====================*/
void    CGameInterfaceManager::GetEndGameDetailStats(EGameUITrigger eTrigger, EGameUITrigger ePageTrigger, const CMatchStatRecord &record, uint uiStart, uint uiEnd)
{
    const MatchStatEventVector &vEvents(record.GetEvents());

    Trigger(ePageTrigger, INT_FLOOR((vEvents.size() / 20) + 1));
    Execute(eTrigger, _T("ClearData();"));

    if (uiEnd < uiStart)
        return;
    if (uiStart >= vEvents.size())
        return;

    MatchStatEventVector_cit citEvent(vEvents.begin() + uiStart);

    tstring sScript;
    uint uiCount(uiStart);
    while (citEvent != vEvents.end() && uiCount < uiEnd)
    {
        uint uiTime(citEvent->uiTime);
        uint uiHoursPlayed = uiTime / MS_PER_HR;
        uint uiMinutesPlayed = (uiTime % MS_PER_HR) / MS_PER_MIN;
        uint uiSecondsPlayed = (uiTime % MS_PER_MIN) / MS_PER_SEC;

        tstring sName(_T("Unknown"));

        if (citEvent->iClientID != -1 && GameClient.GetClientInfo(citEvent->iClientID) != NULL)
            sName = GameClient.GetClientInfo(citEvent->iClientID)->GetName();
        else
        {
            ICvar *pName(g_EntityRegistry.GetGameSetting(citEvent->unTargetType, _T("name")));
            
            if (pName != NULL)
                sName = pName->GetString();
        }

        ICvar *pInflictor(g_EntityRegistry.GetGameSetting(citEvent->unInflictorType, _T("name")));
        tstring sInflictor(_T("Unknown"));

        if (pInflictor != NULL)
            sInflictor = pInflictor->GetString();

        sScript = _T("Data('") + XtoA(++uiCount) + _T("','") +
            XtoA(uiHoursPlayed, FMT_PADZERO, 2) + _T(":") +
            XtoA(uiMinutesPlayed, FMT_PADZERO, 2) + _T(":") +
            XtoA(uiSecondsPlayed, FMT_PADZERO, 2) + _T("','") +
            sName + _T("','") +
            sInflictor + _T("');");

        Execute(eTrigger, sScript);
        ++citEvent;
    }
}

/*====================
  CGameInterfaceManager::GetEndGameTimeStats
  ====================*/
void    CGameInterfaceManager::GetEndGameTimeStats(EGameUITrigger eTrigger, EGameUITrigger ePageTrigger, const CMatchStatRecord &record, uint uiStart, uint uiEnd)
{
    const MatchStatEventVector &vEvents(record.GetEvents());

    Trigger(ePageTrigger, INT_FLOOR((vEvents.size() / 20) + 1));
    Execute(eTrigger, _T("ClearData();"));

    if (uiEnd < uiStart)
        return;
    if (uiStart >= vEvents.size())
        return;

    MatchStatEventVector_cit citEvent(vEvents.begin() + uiStart);

    tstring sScript;
    uint uiCount(uiStart);
    while (citEvent != vEvents.end() && uiCount < uiEnd)
    {
        uint uiTime(citEvent->uiTime);
        uint uiHoursPlayed = uiTime / MS_PER_HR;
        uint uiMinutesPlayed = (uiTime % MS_PER_HR) / MS_PER_MIN;
        uint uiSecondsPlayed = (uiTime % MS_PER_MIN) / MS_PER_SEC;

        tstring sName(_T("Unknown"));

        if (citEvent->iClientID != -1 && GameClient.GetClientInfo(citEvent->iClientID) != NULL)
            sName = GameClient.GetClientInfo(citEvent->iClientID)->GetName();
        else
        {
            ICvar *pName(g_EntityRegistry.GetGameSetting(citEvent->unTargetType, _T("name")));
            
            if (pName != NULL)
                sName = pName->GetString();
        }

        sScript = _T("Data('") + XtoA(++uiCount) + _T("','") +
            XtoA(uiHoursPlayed, FMT_PADZERO, 2) + _T(":") +
            XtoA(uiMinutesPlayed, FMT_PADZERO, 2) + _T(":") +
            XtoA(uiSecondsPlayed, FMT_PADZERO, 2) + _T("','") +
            sName + _T("');");

        Execute(eTrigger, sScript);
        ++citEvent;
    }
}

/*====================
  CGameInterfaceManager::GetEndGameValueStats
  ====================*/
void    CGameInterfaceManager::GetEndGameValueStats(EGameUITrigger eTrigger, EGameUITrigger ePageTrigger, const CMatchStatRecord &record, uint uiStart, uint uiEnd)
{
    const imapfi &mapClientData(record.GetPerClientData());
    const unmapfi &mapTargetData(record.GetPerTargetData());

    Trigger(ePageTrigger, INT_FLOOR(((mapClientData.size() + mapTargetData.size()) / 20) + 1));
    Execute(eTrigger, _T("ClearData();"));

    if (uiEnd < uiStart)
        return;
    if (uiStart >= mapClientData.size() + mapTargetData.size())
        return;

    uint uiCount(0);
    imapfi_cit citClient(mapClientData.begin());
    while (uiCount < uiStart && citClient != mapClientData.end())
    {
        ++uiCount;
        ++citClient;
    }

    tstring sScript;
    while (citClient != mapClientData.end() && uiCount < uiEnd)
    {
        if (citClient->first != -1 && GameClient.GetClientInfo(citClient->first) != NULL)
        {
            tstring sName = GameClient.GetClientInfo(citClient->first)->GetName();

            sScript = _T("Data('") + XtoA(++uiCount) + _T("','") +
                sName + _T("','") +
                XtoA(citClient->second.f) + _T("');");

            Execute(eTrigger, sScript);
        }

        ++citClient;
    }

    unmapfi_cit citTarget(mapTargetData.begin());
    while (uiCount < uiStart && citTarget != mapTargetData.end())
    {
        ++uiCount;
        ++citTarget;
    }

    while (citTarget != mapTargetData.end() && uiCount < uiEnd)
    {
        tstring sName(_T("Unknown"));
        ICvar *pName(g_EntityRegistry.GetGameSetting(citTarget->first, _T("name")));

        if (pName != NULL)
            sName = pName->GetString();

        sScript = _T("Data('") + XtoA(++uiCount) + _T("','") +
            sName + _T("','") +
            XtoA(citTarget->second.f, 0, 0, 0) + _T("');");

        Execute(eTrigger, sScript);

        ++citTarget;
    }
}


/*====================
  CGameInterfaceManager::ShowEndGameStats
  ====================*/
void    CGameInterfaceManager::ShowEndGameStats(int iClientID, int iStart, int iEnd)
{
    CEntityClientInfo *pClient(GameClient.GetClientInfo(iClientID));
    if (pClient == NULL)
        return;

    //GetEndGameDetailStats(UITRIGGER_ENDGAME_DATA_EXPERIENCE, UITRIGGER_ENDGAME_DATA_EXPERIENCE_PAGES, pClient->GetMatchStatRecord(PLAYER_MATCH_EXPERIENCE), iStart, iEnd);
    GetEndGameDetailStats(UITRIGGER_ENDGAME_DATA_KILLS, UITRIGGER_ENDGAME_DATA_KILLS_PAGES, pClient->GetMatchStatRecord(PLAYER_MATCH_KILLS), iStart, iEnd);
    GetEndGameTimeStats(UITRIGGER_ENDGAME_DATA_ASSISTS, UITRIGGER_ENDGAME_DATA_ASSISTS_PAGES, pClient->GetMatchStatRecord(PLAYER_MATCH_ASSISTS), iStart, iEnd);
    GetEndGameDetailStats(UITRIGGER_ENDGAME_DATA_DEATHS, UITRIGGER_ENDGAME_DATA_DEATHS_PAGES, pClient->GetMatchStatRecord(PLAYER_MATCH_DEATHS), iStart, iEnd);
    //GetEndGameDetailStats(UITRIGGER_ENDGAME_DATA_KILL_DEATH_RATIO, UITRIGGER_ENDGAME_DATA_KILL_DEATH_RATIO_PAGES, mapKillsAndDeaths, iStart, iEnd);
    GetEndGameValueStats(UITRIGGER_ENDGAME_DATA_HP_HEALED, UITRIGGER_ENDGAME_DATA_HP_HEALED_PAGES, pClient->GetMatchStatRecord(PLAYER_MATCH_HEALED), iStart, iEnd);
    GetEndGameTimeStats(UITRIGGER_ENDGAME_DATA_RESURRECTS, UITRIGGER_ENDGAME_DATA_RESURRECTS_PAGES, pClient->GetMatchStatRecord(PLAYER_MATCH_RESURRECTS), iStart, iEnd);
    GetEndGameValueStats(UITRIGGER_ENDGAME_DATA_GOLD_EARNED, UITRIGGER_ENDGAME_DATA_GOLD_EARNED_PAGES, pClient->GetMatchStatRecord(PLAYER_MATCH_GOLD_EARNED), iStart, iEnd);
    GetEndGameValueStats(UITRIGGER_ENDGAME_DATA_HP_REPAIRED, UITRIGGER_ENDGAME_DATA_HP_REPAIRED_PAGES, pClient->GetMatchStatRecord(PLAYER_MATCH_REPAIRED), iStart, iEnd);
    GetEndGameDetailStats(UITRIGGER_ENDGAME_DATA_BUILDINGS_RAZED, UITRIGGER_ENDGAME_DATA_BUILDINGS_RAZED_PAGES, pClient->GetMatchStatRecord(PLAYER_MATCH_RAZED), iStart, iEnd);
    GetEndGameValueStats(UITRIGGER_ENDGAME_DATA_PLAYER_DAMAGE, UITRIGGER_ENDGAME_DATA_PLAYER_DAMAGE_PAGES, pClient->GetMatchStatRecord(PLAYER_MATCH_PLAYER_DAMAGE), iStart, iEnd);
    GetEndGameValueStats(UITRIGGER_ENDGAME_DATA_BUILDING_DAMAGE, UITRIGGER_ENDGAME_DATA_BUILDING_DAMAGE_PAGES, pClient->GetMatchStatRecord(PLAYER_MATCH_BUILDING_DAMAGE), iStart, iEnd);
}

/*====================
  CGameInterfaceManager::UpdateGameOver
  ====================*/
void    CGameInterfaceManager::UpdateGameOver()
{
    // Timer
    int iTimeRemaining(int(Game.GetRemainingPhaseTime()) / 1000);
    Trigger(UITRIGGER_ENDGAME_TIME, XtoA(MAX(iTimeRemaining, 0)));

    // Delay before stats show up
    Trigger(UITRIGGER_ENDGAME_SHOW_STATS, GameClient.GetGameTime() - Game.GetPhaseStartTime() >= cg_endGameInterfaceDelay);

    // Client related stats
    CEntityClientInfo *pLocalClient(GameClient.GetLocalClient());
    if (pLocalClient == NULL)
        return;

    // Play time
    uint uiTimePlayed(pLocalClient->GetPlayTime());
    uint uiHoursPlayed(uiTimePlayed / MS_PER_HR);
    uint uiMinutesPlayed((uiTimePlayed % MS_PER_HR) / MS_PER_MIN);
    uint uiSecondsPlayed((uiTimePlayed % MS_PER_MIN) / MS_PER_SEC);
    tstring sTimePlayed(XtoA(uiHoursPlayed, FMT_PADZERO, 2) + _T(":") + XtoA(uiMinutesPlayed, FMT_PADZERO, 2) + _T(":") + XtoA(uiSecondsPlayed, FMT_PADZERO, 2));

    // Match statistics
    float fKillDeathRatio(pLocalClient->GetMatchStatTotalInt(PLAYER_MATCH_KILLS) / float(pLocalClient->GetMatchStatTotalInt(PLAYER_MATCH_DEATHS)));
    Trigger(UITRIGGER_ENDGAME_NPC_KILLS, pLocalClient->GetMatchStatTotalInt(PLAYER_MATCH_NPC_KILLS));
    Trigger(UITRIGGER_ENDGAME_EXPERIENCE, INT_ROUND(pLocalClient->GetExperience() - pLocalClient->GetInitialExperience()));
    Trigger(UITRIGGER_ENDGAME_HP_HEALED, INT_ROUND(pLocalClient->GetMatchStatTotalFloat(PLAYER_MATCH_HEALED)));
    Trigger(UITRIGGER_ENDGAME_KILLS, pLocalClient->GetMatchStatTotalInt(PLAYER_MATCH_KILLS));
    Trigger(UITRIGGER_ENDGAME_RESURRECTS, pLocalClient->GetMatchStatTotalInt(PLAYER_MATCH_RESURRECTS));
    Trigger(UITRIGGER_ENDGAME_DEATHS, pLocalClient->GetMatchStatTotalInt(PLAYER_MATCH_DEATHS));
    Trigger(UITRIGGER_ENDGAME_BUILDING_DAMAGE, INT_ROUND(pLocalClient->GetMatchStatTotalFloat(PLAYER_MATCH_BUILDING_DAMAGE)));
    Trigger(UITRIGGER_ENDGAME_KILL_DEATH_RATIO, _isnan(fKillDeathRatio) ? _T("-") : XtoA(fKillDeathRatio, 0, 0, 2));
    Trigger(UITRIGGER_ENDGAME_RAZED, pLocalClient->GetMatchStatTotalInt(PLAYER_MATCH_RAZED));
    Trigger(UITRIGGER_ENDGAME_ASSISTS, pLocalClient->GetMatchStatTotalInt(PLAYER_MATCH_ASSISTS));
    Trigger(UITRIGGER_ENDGAME_HP_REPAIRED, INT_ROUND(pLocalClient->GetMatchStatTotalFloat(PLAYER_MATCH_REPAIRED)));
    Trigger(UITRIGGER_ENDGAME_PLAYER_DAMAGE, INT_ROUND(pLocalClient->GetMatchStatTotalFloat(PLAYER_MATCH_PLAYER_DAMAGE)));
    Trigger(UITRIGGER_ENDGAME_GOLD_EARNED, pLocalClient->GetMatchStatTotalInt(PLAYER_MATCH_GOLD_EARNED));
    Trigger(UITRIGGER_ENDGAME_SOULS_SPENT, pLocalClient->GetMatchStatTotalInt(PLAYER_MATCH_SOULS_SPENT));
    Trigger(UITRIGGER_ENDGAME_TIME_PLAYED, sTimePlayed);

    // Victory/Defeat
    if (GameClient.GetWinningTeam() == pLocalClient->GetTeam())
        Trigger(UITRIGGER_ENDGAME_VICTORY, true);
    else if (pLocalClient->GetTeam() != 0)
        Trigger(UITRIGGER_ENDGAME_VICTORY, false);

    // Match ID
    if (GameClient.GetGameMatchID() != -1)
        Trigger(UITRIGGER_ENDGAME_MATCH_ID, GameClient.GetGameMatchID());
    else
        Trigger(UITRIGGER_ENDGAME_MATCH_ID, -1);

    vector<pair<int, int> > vAwardKills;
    vector<pair<int, float> > vAwardExp;
    vector<pair<int, float> > vAwardBuildingDamage;
    vector<pair<int, int> > vAwardOrdersFollowed;
    vector<pair<int, float> > vAwardKDRatio;
    vector<pair<int, float> > vAwardHPHealed;
    vector<pair<int, int> > vAwardNPCKills;
    vector<pair<int, int> > vAwardDeaths;
    vector<pair<int, int> > vAwardGoldEarned;
    vector<pair<int, float> > vAwardHero;

    bool bClientProcessed(false);
    for (int i(1); i < Game.GetNumTeams(); ++i)
    {
        CEntityTeamInfo *pTeam(GameClient.GetTeam(i));
        if (pTeam == NULL)
            continue;

        Execute(UITRIGGER_ENDGAME_TEAM, _T("ClearData();"), i - 1);

        const ivector &vTeamList(pTeam->GetClientList());
        for (ivector_cit it(vTeamList.begin()); it != vTeamList.end(); ++it)
        {
            int iClientNumber(*it);
            CEntityClientInfo *pClient(GameClient.GetClientInfo(iClientNumber));
            if (pClient == NULL)
                continue;

            bClientProcessed = true;

            float fKillDeathRatio(pClient->GetMatchStatTotalInt(PLAYER_MATCH_KILLS) / float(pClient->GetMatchStatTotalInt(PLAYER_MATCH_DEATHS)));
            Execute(UITRIGGER_ENDGAME_TEAM, _T("Data('") +
                XtoA(iClientNumber) + _T("','") +
                XtoA(pClient->GetName()) + _T("','") +
                XtoA(INT_ROUND(pClient->GetExperience() - pClient->GetInitialExperience())) + _T("','") +
                XtoA(pClient->GetMatchStatTotalInt(PLAYER_MATCH_KILLS)) + _T("','") +
                XtoA(pClient->GetMatchStatTotalInt(PLAYER_MATCH_ASSISTS)) + _T("','") +
                XtoA(pClient->GetMatchStatTotalInt(PLAYER_MATCH_DEATHS)) + _T("','") +
                (_isnan(fKillDeathRatio) ? _T("-") : XtoA(fKillDeathRatio, 0, 0, 2)) + _T("','") +
                XtoA(INT_ROUND(pClient->GetMatchStatTotalFloat(PLAYER_MATCH_HEALED))) + _T("','") +
                XtoA(pClient->GetMatchStatTotalInt(PLAYER_MATCH_RESURRECTS)) + _T("','") +
                XtoA(pClient->GetMatchStatTotalInt(PLAYER_MATCH_GOLD_EARNED)) + _T("','") +
                XtoA(INT_ROUND(pClient->GetMatchStatTotalFloat(PLAYER_MATCH_REPAIRED))) + _T("','") +
                XtoA(pClient->GetMatchStatTotalInt(PLAYER_MATCH_RAZED)) + _T("','") +
                XtoA(INT_ROUND(pClient->GetMatchStatTotalFloat(PLAYER_MATCH_PLAYER_DAMAGE))) + _T("','") +
                XtoA(INT_ROUND(pClient->GetMatchStatTotalFloat(PLAYER_MATCH_BUILDING_DAMAGE))) + _T("');"),
                i - 1);

            vAwardKills.push_back(pair<int, int>(iClientNumber, pClient->GetMatchStatTotalInt(PLAYER_MATCH_KILLS)));
            vAwardExp.push_back(pair<int, float>(iClientNumber, pClient->GetExperience() - pClient->GetInitialExperience()));
            vAwardBuildingDamage.push_back(pair<int, float>(iClientNumber, pClient->GetMatchStatTotalFloat(PLAYER_MATCH_BUILDING_DAMAGE)));
            vAwardOrdersFollowed.push_back(pair<int, int>(iClientNumber, 0));
            vAwardHPHealed.push_back(pair<int, float>(iClientNumber, pClient->GetMatchStatTotalFloat(PLAYER_MATCH_HEALED)));
            vAwardNPCKills.push_back(pair<int, int>(iClientNumber, pClient->GetMatchStatTotalInt(PLAYER_MATCH_NPC_KILLS)));
            vAwardDeaths.push_back(pair<int, int>(iClientNumber, pClient->GetMatchStatTotalInt(PLAYER_MATCH_DEATHS)));
            vAwardGoldEarned.push_back(pair<int, int>(iClientNumber, pClient->GetMatchStatTotalInt(PLAYER_MATCH_GOLD_EARNED)));
            vAwardKDRatio.push_back(pair<int, float>(iClientNumber, fKillDeathRatio));
        }
    }

    if (!bClientProcessed)
        return;

    // Awards
#define TRIGGER_GAME_AWARD(type, name, stat, high, index) \
    { \
        type _Best(high ? -999999 : 999999); \
        int iWinner(-1); \
    \
        for (vector<pair<int, type> >::iterator it(vAward##stat.begin()); it != vAward##stat.end(); ++it) \
        { \
            if (it->second == _Best) \
            { \
                iWinner = -1; \
                continue; \
            } \
    \
            if ((high && it->second > _Best) || (!high && it->second < _Best)) \
            { \
                iWinner = it->first; \
                _Best = it->second; \
            } \
        } \
    \
        CEntityClientInfo *pWinner(GameClient.GetClientInfo(iWinner)); \
        if (pWinner != NULL) \
            Trigger(UITRIGGER_ENDGAME_AWARD, ((pWinner->GetTeam() == pLocalClient->GetTeam()) ? _T("^g") : _T("^r")) + pWinner->GetName(), index); \
    }

    //TRIGGER_GAME_AWARD(float, _T("Hero"), Hero, true, 0)
    TRIGGER_GAME_AWARD(int, _T("Sadist"), Kills, true, 1)
    TRIGGER_GAME_AWARD(float, _T("Veteran"), Exp, true, 2)
    TRIGGER_GAME_AWARD(float, _T("Homewrecker"), BuildingDamage, true, 3)
    //TRIGGER_GAME_AWARD(int, _T("TeachersPet"), OrdersFollowed, true, 4)
    //TRIGGER_GAME_AWARD(float, _T("Newbie"), KDRatio, false, 5)
    TRIGGER_GAME_AWARD(float, _T("MotherTeresa"), HPHealed, true, 6)
    TRIGGER_GAME_AWARD(int, _T("MMORPGFan"), NPCKills, true, 7)
    TRIGGER_GAME_AWARD(int, _T("Vegan"), NPCKills, false, 8)
    TRIGGER_GAME_AWARD(int, _T("Feeder"), Deaths, true, 9)
    TRIGGER_GAME_AWARD(int, _T("Ghandi"), Kills, false, 10)
    TRIGGER_GAME_AWARD(int, _T("Entrepreneur"), GoldEarned, true, 11)
#undef TRIGGER_GAME_AWARD

    for (int i(0); i < NUM_END_GAME_AWARDS; ++i)
        Execute(UITRIGGER_ENDGAME_LIST, _T("ClearData();"), i);

#define TRIGGER_GAME_AWARD_LIST(type, stat, ascend, index) \
    { \
        int iRank(0); \
        type _Prev(ascend ? -999999 : 999999); \
        \
        if (ascend) \
            sort(vAward##stat.begin(), vAward##stat.end(), CompareByStatValuesAsc<type>); \
        else \
            sort(vAward##stat.begin(), vAward##stat.end(), CompareByStatValuesDesc<type>); \
        \
        for (vector<pair<int, type> >::iterator it(vAward##stat.begin()); it != vAward##stat.end(); ++it) \
        { \
            CEntityClientInfo *pClient(GameClient.GetClientInfo(it->first)); \
            if (pClient == NULL) \
                continue; \
        \
            if ((ascend && it->second > _Prev) || (!ascend && it->second < _Prev)) \
                ++iRank; \
        \
            _Prev = it->second; \
        \
            Execute(UITRIGGER_ENDGAME_LIST, \
                _T("Data('") + \
                XtoA(it->first) + _T("','") + \
                XtoA(iRank) + _T("','") + \
                pClient->GetName() + _T("','") + \
                XtoA(INT_ROUND(it->second)) + _T("');"), \
                index); \
        } \
    }

    //TRIGGER_GAME_AWARD_LIST(float, Hero, false, 0)
    TRIGGER_GAME_AWARD_LIST(int, Kills, false, 1)
    TRIGGER_GAME_AWARD_LIST(float, Exp, false, 2)
    TRIGGER_GAME_AWARD_LIST(float, BuildingDamage, false, 3)
    //TRIGGER_GAME_AWARD_LIST(int, OrdersFollowed, false, 4)
    //TRIGGER_GAME_AWARD_LIST(float, KDRatio, true, 5)
    TRIGGER_GAME_AWARD_LIST(float, HPHealed, false, 6)
    TRIGGER_GAME_AWARD_LIST(int, NPCKills, false, 7)
    TRIGGER_GAME_AWARD_LIST(int, NPCKills, true, 8)
    TRIGGER_GAME_AWARD_LIST(int, Deaths, false, 9)
    TRIGGER_GAME_AWARD_LIST(int, Kills, true, 10)
    TRIGGER_GAME_AWARD_LIST(int, GoldEarned, false, 11)
#undef TRIGGER_GAME_AWARD_LIST
}


/*====================
  CGameInterfaceManager::UpdateInventory
  ====================*/
void    CGameInterfaceManager::UpdateInventory()
{
    PROFILE("CGameInterfaceManager::UpdateInventory");

    CEntityClientInfo *pLocalClient(GameClient.GetLocalClient());
    if (pLocalClient == NULL)
        return;
    IPlayerEntity *pPlayer(pLocalClient->GetPlayerEntity());
    if (pPlayer == NULL)
        return;

    if (GameClient.GetCurrentSnapshot() != NULL)
        Trigger(UITRIGGER_SELECTED_ITEM, GameClient.GetCurrentSnapshot()->GetSelectedItem());

    int iItemCount(0);
    for (int iSlot(0); iSlot < INVENTORY_END_BACKPACK; ++iSlot)
    {
        PROFILE("Inventory Loop");

        IInventoryItem *pItem(pPlayer->GetItem(iSlot));
        if (pItem == NULL)
        {
            Trigger(UITRIGGER_INVENTORY_EXISTS, false, iSlot);
            continue;
        }

        if (iSlot < INVENTORY_START_BACKPACK)
            ++iItemCount;

        Trigger(UITRIGGER_INVENTORY_EXISTS, true, iSlot);
        Trigger(UITRIGGER_INVENTORY_ENTITY, pItem->GetTypeName(), iSlot);
        Trigger(UITRIGGER_INVENTORY_ICON, pItem->GetIconImageList(), iSlot);
        Trigger(UITRIGGER_INVENTORY_TIMER, pItem->GetCooldownPercent(), iSlot);
        Trigger(UITRIGGER_INVENTORY_AMMO_COUNT, pItem->GetAmmo(), iSlot);
        Trigger(UITRIGGER_INVENTORY_AMMO_PERCENT, pItem->GetAmmoPercent(), iSlot);

        if ((pPlayer->GetAction() & PLAYER_ACTION_STUNNED || pItem->IsSilenced()) &&
            !(pItem->IsConsumable() && pItem->GetAsConsumable()->GetPassive()))
        {
            Trigger(UITRIGGER_INVENTORY_ENABLED, false, iSlot);
            Trigger(UITRIGGER_INVENTORY_LOW_MANA, false, iSlot);
            Trigger(UITRIGGER_INVENTORY_NO_ACCESS, false, iSlot);
            Trigger(UITRIGGER_INVENTORY_DISABLED, true, iSlot);
        }       
        else if (pItem->IsDisabled())
        {
            Trigger(UITRIGGER_INVENTORY_ENABLED, false, iSlot);
            Trigger(UITRIGGER_INVENTORY_LOW_MANA, false, iSlot);
            Trigger(UITRIGGER_INVENTORY_NO_ACCESS, false, iSlot);
            Trigger(UITRIGGER_INVENTORY_DISABLED, false, iSlot);
        }
        else if(!pPlayer->CanAccess(iSlot))
        {
            Trigger(UITRIGGER_INVENTORY_ENABLED, false, iSlot);
            Trigger(UITRIGGER_INVENTORY_LOW_MANA, false, iSlot);
            Trigger(UITRIGGER_INVENTORY_NO_ACCESS, true, iSlot);
            Trigger(UITRIGGER_INVENTORY_DISABLED, false, iSlot);
        }
        else if(pItem->GetManaCost() > pPlayer->GetMana())
        {
            Trigger(UITRIGGER_INVENTORY_ENABLED, false, iSlot);
            Trigger(UITRIGGER_INVENTORY_LOW_MANA, true, iSlot);
            Trigger(UITRIGGER_INVENTORY_NO_ACCESS, false, iSlot);
            Trigger(UITRIGGER_INVENTORY_DISABLED, false, iSlot);
        }
        else
        {
            Trigger(UITRIGGER_INVENTORY_ENABLED, true, iSlot);
            Trigger(UITRIGGER_INVENTORY_LOW_MANA, false, iSlot);
            Trigger(UITRIGGER_INVENTORY_NO_ACCESS, false, iSlot);
            Trigger(UITRIGGER_INVENTORY_DISABLED, false, iSlot);
        }
    }
    Trigger(UITRIGGER_INVENTORY_COUNT, iItemCount);

    for (int i(PERSISTANT_VAULT_START); i < MAX_PERSISTANT_ITEMS; ++i)
        SetPersistantVaultIcon(i);
}


/*====================
  CGameInterfaceManager::UpdatePlayer
  ====================*/
void    CGameInterfaceManager::UpdatePlayer()
{
    PROFILE("CGameInterfaceManager::UpdatePlayer");

    CEntityClientInfo *pClient(GameClient.GetClientInfo(GameClient.GetLocalClientNum()));
    if (pClient == NULL)
        return;
    IPlayerEntity *pPlayer(GameClient.GetLocalPlayer());

    if (pPlayer != NULL)
    {
        Trigger(UITRIGGER_DEATH_TIME, pPlayer->GetRemainingDeathTime());
        Trigger(UITRIGGER_DEATH_PERCENT, pPlayer->GetDeathPercent());
    }

    // Basic stats
    Trigger(UITRIGGER_NAME, pClient->GetName());
    Trigger(UITRIGGER_SOUL_COUNT, pClient->GetSouls());
    Trigger(UITRIGGER_LEVEL, pClient->GetLevel());
    Trigger(UITRIGGER_GOLD, pClient->GetGold());
    Trigger(UITRIGGER_STAT_POINTS, pClient->GetAvailablePoints());
    Trigger(UITRIGGER_EXPERIENCE, pClient->GetPercentNextLevel());
    
    if (pPlayer != NULL)
    {
        Trigger(UITRIGGER_DASH_REMAINING, pPlayer->GetDashRemaining());
        Trigger(UITRIGGER_DASH_COOLDOWN, pPlayer->GetDashCooldownPercent());

        Trigger(UITRIGGER_ENTITY, pPlayer->GetTypeName());
        Trigger(UITRIGGER_CLASS, pPlayer->GetEntityName());
        Trigger(UITRIGGER_HEALTH_VALUE, pPlayer->GetHealth());
        Trigger(UITRIGGER_MANA_VALUE, pPlayer->GetMana());
        Trigger(UITRIGGER_STAMINA_VALUE, pPlayer->GetStamina());
        Trigger(UITRIGGER_HEALTH_PERCENT, pPlayer->GetHealthPercent());
        Trigger(UITRIGGER_MANA_PERCENT, pPlayer->GetManaPercent());
        Trigger(UITRIGGER_STAMINA_PERCENT, pPlayer->GetStaminaPercent());
        Trigger(UITRIGGER_ADJUSTED_MAX_HEALTH, pPlayer->GetMaxHealth());
        Trigger(UITRIGGER_ADJUSTED_MAX_MANA, pPlayer->GetMaxMana());
        Trigger(UITRIGGER_ADJUSTED_MAX_STAMINA, pPlayer->GetMaxStamina());
        Trigger(UITRIGGER_ADJUSTED_ARMOR, pPlayer->GetArmor());
        Trigger(UITRIGGER_ADJUSTED_ARMOR_REDUCTION, pPlayer->GetArmorDamageReduction(pPlayer->GetArmor()));
        Trigger(UITRIGGER_CURRENT_SPEED, pPlayer->GetCurrentSpeed());

        tstring sMoveType;
        if (pPlayer->GetCurrentOrder() != CMDR_ORDER_CLEAR)
        {
            if (pPlayer->GetCurrentOrder() == CMDR_ORDER_ATTACK)
                Trigger(UITRIGGER_COMMANDER_ORDERTYPE, _T("Attack"));
            else if (pPlayer->GetCurrentOrder() == CMDR_ORDER_MOVE)
                Trigger(UITRIGGER_COMMANDER_ORDERTYPE, _T("Move"));
            else
                Trigger(UITRIGGER_COMMANDER_ORDERTYPE, SNULL);

            CVec3f v3OrderPos(pPlayer->GetCurrentOrderPos());
            if (pPlayer->GetCurrentOrderEntIndex() != INVALID_INDEX)
            {
                IVisualEntity *pTarget(GameClient.GetVisualEntity(pPlayer->GetCurrentOrderEntIndex()));
                if (pTarget != NULL)
                    v3OrderPos = pTarget->GetPosition();
            }

            CVec3f v3Offset(pPlayer->GetPosition() - v3OrderPos);
            float fDot(DotProduct(v3Offset, CAxis(pPlayer->GetViewAngles()).Right()));
            Trigger(UITRIGGER_COMMANDER_ORDERDIRECTION, fDot);
        }
        else if (pPlayer->GetOfficerOrder() != OFFICERCMD_INVALID)
        {
            switch (pPlayer->GetOfficerOrder())
            {
            case OFFICERCMD_ATTACK: Trigger(UITRIGGER_COMMANDER_ORDERTYPE, _T("Attack")); break;
            case OFFICERCMD_MOVE: Trigger(UITRIGGER_COMMANDER_ORDERTYPE, _T("Move")); break;
            case OFFICERCMD_FOLLOW: Trigger(UITRIGGER_COMMANDER_ORDERTYPE, _T("Follow")); break;
            case OFFICERCMD_DEFEND: Trigger(UITRIGGER_COMMANDER_ORDERTYPE, _T("Defend")); break;
            case OFFICERCMD_RALLY: Trigger(UITRIGGER_COMMANDER_ORDERTYPE, _T("Rally")); break;
            default: Trigger(UITRIGGER_COMMANDER_ORDERTYPE, SNULL); break;
            }

            CVec3f v3OrderPos(pPlayer->GetOfficerOrderPos());
            if (pPlayer->GetOfficerOrderEntIndex() != INVALID_INDEX)
            {
                IVisualEntity *pTarget(GameClient.GetVisualEntity(pPlayer->GetOfficerOrderEntIndex()));
                if (pTarget != NULL)
                    v3OrderPos = pTarget->GetPosition();
            }

            CVec3f v3Offset(pPlayer->GetPosition() - v3OrderPos);
            float fDot(DotProduct(v3Offset, CAxis(pPlayer->GetViewAngles()).Right()));
            Trigger(UITRIGGER_COMMANDER_ORDERDIRECTION, fDot);
        }
        else
        {
            Trigger(UITRIGGER_COMMANDER_ORDERTYPE, SNULL);
            Trigger(UITRIGGER_COMMANDER_ORDERDIRECTION, 0.0f);
        }
    }

    // Camera
    if (GameClient.GetCamera()->HasFlags(CAM_FIRST_PERSON) || ReplayManager.IsPlaying())
        ShowCursor(true);
    else
        ShowCursor(false);

    UpdateInventory();
    UpdateSquad();

    // Buffs/Debuffs
    if (pPlayer != NULL)
    {
        int iBuffCount(0);
        int iDebuffCount(0);
        for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
        {
            IEntityState *pState(pPlayer->GetState(i));
            if (pState == NULL)
                continue;
            if (!pState->GetDisplayState())
                continue;

            if (pState->IsBuff() && iBuffCount < MAX_DISPLAY_BUFFS)
            {
                Trigger(UITRIGGER_BUFF_ACTIVE, true, iBuffCount);
                Trigger(UITRIGGER_BUFF_ICON, pState->GetIconPath(), iBuffCount);
                if (pState->GetExpireTime() == INVALID_TIME || !pState->GetDisplayTimer())
                {
                    Trigger(UITRIGGER_BUFF_TIME, -1, iBuffCount);
                    Trigger(UITRIGGER_BUFF_TIME_PERCENT, 0.0f, iBuffCount);
                }
                else
                {
                    Trigger(UITRIGGER_BUFF_TIME, pState->GetExpireTime() - Game.GetServerTime(), iBuffCount);
                    Trigger(UITRIGGER_BUFF_TIME_PERCENT, pPlayer->GetStateExpirePercent(i), iBuffCount);
                }
                ++iBuffCount;
            }
            if (pState->IsDebuff() && iDebuffCount < MAX_DISPLAY_DEBUFFS)
            {
                Trigger(UITRIGGER_DEBUFF_ACTIVE, true, iDebuffCount);
                Trigger(UITRIGGER_DEBUFF_ICON, pState->GetIconPath(), iDebuffCount);
                if (pState->GetExpireTime() == INVALID_TIME || !pState->GetDisplayTimer())
                {
                    Trigger(UITRIGGER_DEBUFF_TIME, -1, iDebuffCount);
                    Trigger(UITRIGGER_DEBUFF_TIME_PERCENT, 0.0f, iDebuffCount);
                }
                else
                {
                    Trigger(UITRIGGER_DEBUFF_TIME, pState->GetExpireTime() - Game.GetGameTime(), iDebuffCount);
                    Trigger(UITRIGGER_DEBUFF_TIME_PERCENT, pPlayer->GetStateExpirePercent(i), iDebuffCount);
                }
                ++iDebuffCount;
            }
        }
        for (int i(iBuffCount); i < MAX_DISPLAY_BUFFS; ++i)
            Trigger(UITRIGGER_BUFF_ACTIVE, false, i);
        for (int i(iDebuffCount); i < MAX_DISPLAY_DEBUFFS; ++i)
            Trigger(UITRIGGER_DEBUFF_ACTIVE, false, i);
    }

    // Team info
    CEntityTeamInfo *pTeam(GameClient.GetTeam(pClient->GetTeam()));
    if (pTeam == NULL)
        return;

    Trigger(UITRIGGER_TEAM_GOLD, pTeam->GetGold());
    //Trigger(UITRIGGER_UPKEEP, pTeam->GetActiveUpkeep());
    Trigger(UITRIGGER_UPKEEP, !pTeam->HasNetFlags(ENT_NET_FLAG_UPKEEP_FAILED));
    Trigger(UITRIGGER_INCOME, pTeam->GetTotalIncome());
    Trigger(UITRIGGER_HAS_COMMANDER, pTeam->GetCommanderClientID() != -1);

    IBuildingEntity *pCommandCenter(GameClient.GetBuildingEntity(pTeam->GetBaseBuildingIndex()));
    if (pCommandCenter != NULL)
        Trigger(UITRIGGER_COMMAND_CENTER_HEALTH_PERCENT, pCommandCenter->GetHealthPercent());

    // Enemy team
    pTeam = GameClient.GetTeam(pClient->GetTeam() ^ 3);
    if (pTeam == NULL)
        return;
    pCommandCenter = GameClient.GetBuildingEntity(pTeam->GetBaseBuildingIndex());
    if (pCommandCenter != NULL)
        Trigger(UITRIGGER_ENEMY_COMMAND_CENTER_HEALTH_PERCENT, pCommandCenter->GetHealthPercent());

    if (GameClient.GetGamePhase() == GAME_PHASE_WARMUP)
    {
        int iPlayersRequired(0);
        int iMinPlayers(0);
        
        if (Game.GetWorldPointer() != NULL)
            iMinPlayers = Game.GetWorldPointer()->GetMinPlayersPerTeam();

        for (int i(1); i < Game.GetNumTeams(); i++)
            if (Game.GetTeam(i) != NULL && Game.GetTeam(i)->GetNumClients() < iMinPlayers)
                iPlayersRequired += (iMinPlayers - Game.GetTeam(i)->GetNumClients());

        Trigger(UITRIGGER_WARMUP_PLAYERS_REQUIRED, iPlayersRequired);
    }

    // Gadgets
    for (int i(0); i < MAX_DEPLOYED_GADGETS; ++i)
    {
        IGadgetEntity *pGadget(GameClient.GetGadgetEntity(pClient->GetGadgetIndex(i)));
        if (pGadget == NULL)
        {
            Trigger(UITRIGGER_GADGET_ACTIVE, false, i);
            continue;
        }

        Trigger(UITRIGGER_GADGET_ACTIVE, true, i);
        Trigger(UITRIGGER_GADGET_ICON, pGadget->GetEntityIconPath(), i);
        Trigger(UITRIGGER_GADGET_TIMER, GameClient.GetGameTime() - pGadget->GetSpawnTime(), i);
        Trigger(UITRIGGER_GADGET_EXPERIENCE, pGadget->GetExperienceAccumulator(), i);
        for (int n(0); n < 3; ++n)
        {
            Trigger(UITRIGGER_GADGET_COUNTER_LABEL, pGadget->GetCounterLabel(n), i, n);
            Trigger(UITRIGGER_GADGET_COUNTER_VALUE, int(pGadget->GetCounterValue(n)), i, n);
        }
        Trigger(UITRIGGER_GADGET_LIFETIME_PERCENT, pGadget->GetRemainingLifetimePercent(), i);
        Trigger(UITRIGGER_GADGET_HEALTH_PERCENT, pGadget->GetHealthPercent(), i);
    }
}


/*====================
  CGameInterfaceManager::UpdatePlayerDetails
  ====================*/
void    CGameInterfaceManager::UpdatePlayerDetails()
{
    PROFILE("CGameInterfaceManager::UpdatePlayerDetails");

    CEntityClientInfo *pLocalClient(GameClient.GetLocalClient());
    if (pLocalClient == NULL)
        return;
    IPlayerEntity *pPlayer(pLocalClient->GetPlayerEntity());
    if (pPlayer == NULL)
        return;

    if (pLocalClient->HasNetFlags(ENT_NET_FLAG_QUEUED))
        Trigger(UITRIGGER_LOADOUT_TIME, _T("Queued"));
    else
        Trigger(UITRIGGER_LOADOUT_TIME, INT_CEIL(pLocalClient->GetRemainingLoadoutTime() / 1000.0f));

    int iMaxStats(ICvar::GetInteger(_T("g_maxAttributeBoost")));
    for (int i(0); i < NUM_PLAYER_ATTRIBUTES; ++i)
    {
        Trigger(UITRIGGER_STAT_NAME, g_asPlayerAttibutes[i], i);
        Trigger(UITRIGGER_STAT_COST, pLocalClient->GetAttributeCost(i), i);
        Trigger(UITRIGGER_STAT_BUTTON, pLocalClient->GetAvailablePoints() >= pLocalClient->GetAttributeCost(i) && (iMaxStats == -1 || pLocalClient->GetAttributeLevel(i) < iMaxStats), i);
        Trigger(UITRIGGER_STAT_LEVEL, pLocalClient->GetAttributeLevel(i), i);
        Trigger(UITRIGGER_STAT_PERCENT, pPlayer->GetAttributeBoost(i), i);
        Trigger(UITRIGGER_STAT_EFFECT, pPlayer->GetAttributeBoostDescription(i), i);
    }

    Trigger(UITRIGGER_BASE_MAX_HEALTH, pPlayer->GetBaseMaxHealth());
    Trigger(UITRIGGER_BASE_MAX_MANA, pPlayer->GetBaseMaxMana());
    Trigger(UITRIGGER_BASE_MAX_STAMINA, pPlayer->GetBaseMaxStamina());
    Trigger(UITRIGGER_ADJUSTED_MAX_HEALTH, pPlayer->GetMaxHealth());
    Trigger(UITRIGGER_ADJUSTED_MAX_MANA, pPlayer->GetMaxMana());
    Trigger(UITRIGGER_ADJUSTED_MAX_STAMINA, pPlayer->GetMaxStamina());
    Trigger(UITRIGGER_MAX_HEALTH_BONUS, pPlayer->GetMaxHealth() - pPlayer->GetBaseMaxHealth());
    Trigger(UITRIGGER_MAX_MANA_BONUS, pPlayer->GetMaxMana() - pPlayer->GetBaseMaxMana());
    Trigger(UITRIGGER_MAX_STAMINA_BONUS, pPlayer->GetMaxStamina() - pPlayer->GetBaseMaxStamina());

    Trigger(UITRIGGER_BASE_HEALTH_REGEN, pPlayer->GetBaseHealthRegen());
    Trigger(UITRIGGER_BASE_MANA_REGEN, pPlayer->GetBaseManaRegen());
    Trigger(UITRIGGER_BASE_STAMINA_REGEN, pPlayer->GetBaseStaminaRegen());
    Trigger(UITRIGGER_ADJUSTED_HEALTH_REGEN, pPlayer->GetHealthRegen());
    Trigger(UITRIGGER_ADJUSTED_MANA_REGEN, pPlayer->GetManaRegen());
    Trigger(UITRIGGER_ADJUSTED_STAMINA_REGEN, pPlayer->GetStaminaRegen());
    Trigger(UITRIGGER_HEALTH_REGEN_BONUS, pPlayer->GetHealthRegen() - pPlayer->GetBaseHealthRegen());
    Trigger(UITRIGGER_MANA_REGEN_BONUS, pPlayer->GetManaRegen() - pPlayer->GetBaseManaRegen());
    Trigger(UITRIGGER_STAMINA_REGEN_BONUS, pPlayer->GetStaminaRegen() - pPlayer->GetBaseStaminaRegen());

    Trigger(UITRIGGER_BASE_ARMOR, pPlayer->GetBaseArmor());
    Trigger(UITRIGGER_BASE_ARMOR_REDUCTION, pPlayer->GetArmorDamageReduction(pPlayer->GetBaseArmor()));
    Trigger(UITRIGGER_ARMOR_BONUS, pPlayer->GetArmor() - pPlayer->GetBaseArmor());
    Trigger(UITRIGGER_ARMOR_REDUCTION_BONUS, pPlayer->GetArmorDamageReduction(pPlayer->GetArmor()) - pPlayer->GetArmorDamageReduction(pPlayer->GetBaseArmor()));

    Trigger(UITRIGGER_BASE_SPEED, pPlayer->GetBaseSpeed());

    Trigger(UITRIGGER_PLAYER_DESCRIPTION, pPlayer->GetEntityDescription());

    for (int iSlot(0); iSlot < INVENTORY_END_BACKPACK; ++iSlot)
    {
        IInventoryItem *pItem(pPlayer->GetItem(iSlot));
        if (!pItem)
            continue;

        Trigger(UITRIGGER_INVENTORY_NAME, pItem->GetName(), iSlot);
        Trigger(UITRIGGER_INVENTORY_DESCRIPTION, pItem->GetDescription(), iSlot);
        Trigger(UITRIGGER_INVENTORY_MANA_COST, INT_ROUND(pItem->GetManaCost()), iSlot);
        Trigger(UITRIGGER_INVENTORY_COOLDOWN, INT_ROUND(pItem->GetCooldownTime() / 1000), iSlot);
        Trigger(UITRIGGER_INVENTORY_TYPE, pItem->GetTypeName(), iSlot);
        Trigger(UITRIGGER_INVENTORY_PREREQUISITE, pItem->GetPrerequisiteDescription(), iSlot);

        if (pItem->IsMelee())
        {
            Show(UITRIGGER_INVENTORY_MELEE, iSlot);
            Hide(UITRIGGER_INVENTORY_RANGED_MANA, iSlot);
            Hide(UITRIGGER_INVENTORY_RANGED_AMMO, iSlot);
            Hide(UITRIGGER_INVENTORY_ABILITY, iSlot);

            IMeleeItem *pMelee(pItem->GetAsMelee());
            Trigger(UITRIGGER_INVENTORY_QUICK_ATTACK_DAMAGE, XtoA(INT_ROUND(pMelee->GetQuickAttackMinDamage(0))) + _T(" - ") + XtoA(INT_ROUND(pMelee->GetQuickAttackMaxDamage(0))), iSlot);
            Trigger(UITRIGGER_INVENTORY_QUICK_ATTACK_SPEED, pMelee->GetQuickAttackTime(0), iSlot);
            Trigger(UITRIGGER_INVENTORY_STRONG_ATTACK_DAMAGE, XtoA(INT_ROUND(pMelee->GetStrongAttackMinDamage())) + _T(" - ") + XtoA(INT_ROUND(pMelee->GetStrongAttackMaxDamage())), iSlot);
            Trigger(UITRIGGER_INVENTORY_STRONG_ATTACK_SPEED, pMelee->GetStrongAttackTime(0), iSlot);
        }
        else if (pItem->IsGun() && pItem->GetManaCost())
        {
            Hide(UITRIGGER_INVENTORY_MELEE, iSlot);
            Show(UITRIGGER_INVENTORY_RANGED_MANA, iSlot);
            Hide(UITRIGGER_INVENTORY_RANGED_AMMO, iSlot);
            Hide(UITRIGGER_INVENTORY_ABILITY, iSlot);

            IGunItem *pGun(pItem->GetAsGun());
            Trigger(UITRIGGER_INVENTORY_RANGED_DAMAGE, XtoA(INT_ROUND(pGun->GetMinDamage())) + _T(" - ") + XtoA(INT_ROUND(pGun->GetMaxDamage())), iSlot);
            Trigger(UITRIGGER_INVENTORY_AMMO, pGun->GetAmmoCount(), iSlot);
            Trigger(UITRIGGER_INVENTORY_RANGED_ATTACK_SPEED, pGun->GetAttackTime() + pGun->GetCooldownTime(), iSlot);
        }
        else if (pItem->IsGun())
        {
            Hide(UITRIGGER_INVENTORY_MELEE, iSlot);
            Hide(UITRIGGER_INVENTORY_RANGED_MANA, iSlot);
            Show(UITRIGGER_INVENTORY_RANGED_AMMO, iSlot);
            Hide(UITRIGGER_INVENTORY_ABILITY, iSlot);

            IGunItem *pGun(pItem->GetAsGun());
            Trigger(UITRIGGER_INVENTORY_RANGED_DAMAGE, XtoA(INT_ROUND(pGun->GetMinDamage())) + _T(" - ") + XtoA(INT_ROUND(pGun->GetMaxDamage())), iSlot);
            Trigger(UITRIGGER_INVENTORY_AMMO, pGun->GetAmmoCount(), iSlot);
            Trigger(UITRIGGER_INVENTORY_RANGED_ATTACK_SPEED, pGun->GetAttackTime() + pGun->GetCooldownTime(), iSlot);
        }
        else if (pItem->IsSkill())
        {
            Hide(UITRIGGER_INVENTORY_MELEE, iSlot);
            Hide(UITRIGGER_INVENTORY_RANGED_MANA, iSlot);
            Hide(UITRIGGER_INVENTORY_RANGED_AMMO, iSlot);
            Show(UITRIGGER_INVENTORY_ABILITY, iSlot);
        }
        else if (pItem->IsSpell())
        {
            Hide(UITRIGGER_INVENTORY_MELEE, iSlot);
            Hide(UITRIGGER_INVENTORY_RANGED_MANA, iSlot);
            Hide(UITRIGGER_INVENTORY_RANGED_AMMO, iSlot);
            Show(UITRIGGER_INVENTORY_ABILITY, iSlot);
        }
    }
}


/*====================
  CGameInterfaceManager::UpdateCommander
  ====================*/
void    CGameInterfaceManager::UpdateCommander()
{
    PROFILE("CGameInterfaceManager::UpdateCommander");

    CEntityClientInfo *pLocalClient(GameClient.GetLocalClient());
    if (pLocalClient == NULL)
        return;

    IPlayerEntity *pPlayer(pLocalClient->GetPlayerEntity());
    if (pPlayer == NULL)
        return;
    CClientCommander *pCommander(GameClient.GetClientCommander());
    if (pCommander == NULL)
        return;

    CEntityTeamInfo *pTeam(GameClient.GetTeam(pPlayer->GetTeam()));
    if (pTeam == NULL)
        return;

    Trigger(UITRIGGER_BUILD_MODE_ACTIVE, GameClient.IsBuildMenuVisible());

    //Trigger(UITRIGGER_UPKEEP, pTeam->GetActiveUpkeep());
    Trigger(UITRIGGER_UPKEEP, !pTeam->HasNetFlags(ENT_NET_FLAG_UPKEEP_FAILED));
    Trigger(UITRIGGER_INCOME, pTeam->GetTotalIncome());
    Trigger(UITRIGGER_TEAM_GOLD, pTeam->GetGold());
    Trigger(UITRIGGER_TEAM_MANA, pTeam->GetMana());
    Trigger(UITRIGGER_TEAM_MANA_PERCENT, pTeam->GetManaPercent());
    Trigger(UITRIGGER_TEAM_MAX_MANA, pTeam->GetMaxMana());
    Trigger(UITRIGGER_LEVEL, pTeam->GetLevel());
    Trigger(UITRIGGER_EXPERIENCE, pTeam->GetPercentNextLevel());
    
    Trigger(UITRIGGER_COMMANDER_WORKER_COOLDOWN, pTeam->GetWorkerCooldownPercent());
    Trigger(UITRIGGER_COMMANDER_WORKER_AVAILABLE, pTeam->CanSpawnWorker(pLocalClient->GetClientNumber()));
    
    uivector vWorkers(pTeam->GetWorkerList());
    for (uint ui(0); ui < MAX_WORKER_ICONS; ++ui)
    {
        IVisualEntity *pWorker(NULL);
        if (ui >= vWorkers.size() ||
            (pWorker = GameClient.GetVisualEntity(vWorkers[ui])) == NULL)
        {
            Trigger(UITRIGGER_COMMANDER_WORKER_INDEX, 0, ui);
            Trigger(UITRIGGER_COMMANDER_WORKER_HEALTH, 0.f, ui);
            continue;
        }

        Trigger(UITRIGGER_COMMANDER_WORKER_INDEX, pWorker->GetIndex(), ui);
        Trigger(UITRIGGER_COMMANDER_WORKER_ICON, pWorker->GetEntityIconPath(), ui);

        IPetEntity *pPetWorker(pWorker->GetAsPet());
        if (pPetWorker == NULL)
        {
            Trigger(UITRIGGER_COMMANDER_WORKER_ACTION, _T("idle"), ui);
            Trigger(UITRIGGER_COMMANDER_WORKER_HEALTH, 0.f, ui);
            continue;
        }

        Trigger(UITRIGGER_COMMANDER_WORKER_ACTION, pPetWorker->GetCurrentJob(), ui);
        Trigger(UITRIGGER_COMMANDER_WORKER_HEALTH, pPetWorker->GetHealthPercent(), ui);
    }

    IBuildingEntity *pCommandCenter(GameClient.GetBuildingEntity(pTeam->GetBaseBuildingIndex()));
    if (pCommandCenter != NULL)
        Trigger(UITRIGGER_COMMAND_CENTER_HEALTH_PERCENT, pCommandCenter->GetHealthPercent());

    UpdateInventory();

    // Multi-selection
    const uiset &setSelection(pCommander->GetSelectedEntities());
    int iCount(0);
    if (!setSelection.empty())
    {
        for (uiset_cit it(setSelection.begin()); it != setSelection.end(); ++it)
        {
            IVisualEntity *pEntity(GameClient.GetClientEntityCurrent(*it));
            if (pEntity == NULL)
                continue;

            Trigger(UITRIGGER_MULTI_SELECT_ACTIVE, true, iCount);
            Trigger(UITRIGGER_MULTI_SELECT_ICON, pEntity->GetEntityIconPath(), iCount);
            Trigger(UITRIGGER_MULTI_SELECT_HEALTH_PERCENT, pEntity->GetHealthPercent(), iCount);
            Trigger(UITRIGGER_MULTI_SELECT_INDEX, pEntity->GetIndex(), iCount);
            
            IPlayerEntity *pPlayer(pEntity->GetAsPlayerEnt());
            if (pPlayer == NULL)
            {
                Trigger(UITRIGGER_MULTI_SELECT_MANA_PERCENT, 0.0f, iCount);
                Trigger(UITRIGGER_MULTI_SELECT_SQUAD_COLOR, _T("white"), iCount);
            }
            else
            {
                Trigger(UITRIGGER_MULTI_SELECT_MANA_PERCENT, pPlayer->GetManaPercent(), iCount);
                Trigger(UITRIGGER_MULTI_SELECT_SQUAD_COLOR, pTeam->GetSquadColor(pPlayer->GetSquad()), iCount);
            }

            ++iCount;
        }
    }
    for (int i(iCount); i < MAX_SELECTED_UNITS; ++i)
        Trigger(UITRIGGER_MULTI_SELECT_ACTIVE, false, i);

    // Enemy team
    pTeam = GameClient.GetTeam(pPlayer->GetTeam() ^ 3);
    if (pTeam == NULL)
        return;
    pCommandCenter = GameClient.GetBuildingEntity(pTeam->GetBaseBuildingIndex());
    if (pCommandCenter != NULL)
        Trigger(UITRIGGER_ENEMY_COMMAND_CENTER_HEALTH_PERCENT, pCommandCenter->GetHealthPercent());

    // Ability tooltips
    for (int iSlot(0); iSlot < INVENTORY_START_BACKPACK; ++iSlot)
    {
        IInventoryItem *pItem(pPlayer->GetItem(iSlot));
        if (!pItem)
            continue;

        Trigger(UITRIGGER_INVENTORY_NAME, pItem->GetName(), iSlot);
        Trigger(UITRIGGER_INVENTORY_DESCRIPTION, pItem->GetDescription(), iSlot);
        Trigger(UITRIGGER_INVENTORY_MANA_COST, INT_ROUND(pItem->GetManaCost()), iSlot);
        Trigger(UITRIGGER_INVENTORY_COOLDOWN, INT_ROUND(pItem->GetCooldownTime() / 1000), iSlot);
        Trigger(UITRIGGER_INVENTORY_TYPE, pItem->GetTypeName(), iSlot);
        Trigger(UITRIGGER_INVENTORY_PREREQUISITE, pItem->GetPrerequisiteDescription(), iSlot);
    }
}


/*====================
  CGameInterfaceManager::BuildingAttackAlert
  ====================*/
void    CGameInterfaceManager::BuildingAttackAlert(const tstring &sName)
{
    m_uiLastBuildingAttackAlertTime = Game.GetGameTime();
    svector vParams;
    vParams.push_back(_T("true"));
    vParams.push_back(sName);
    Trigger(UITRIGGER_BUILDING_ATTACK_ALERT, vParams);
}


/*====================
  CGameInterfaceManager::UpdateSquad
  ====================*/
void    CGameInterfaceManager::UpdateSquad()
{
    PROFILE("CGameInterfaceManager::UpdateSquad");

    CEntityClientInfo *pLocalClient(GameClient.GetLocalClient());
    if (pLocalClient == NULL)
    {
        // Hide all squad info panels
        for (int i(0); i < MAX_DISPLAY_SQUAD; ++i)
            Trigger(UITRIGGER_SQUAD_INFO, i, false);
        return;
    }

    CEntityTeamInfo *pTeam(GameClient.GetTeam(pLocalClient->GetTeam()));
    int iSquad(pLocalClient->GetSquad());

    if (pTeam == NULL || iSquad == INVALID_SQUAD)
    {
        // Hide all squad info panels
        for (int i(0); i < MAX_DISPLAY_SQUAD; ++i)
            Trigger(UITRIGGER_SQUAD_INFO, false, i);
        return;
    }

    IPlayerEntity *pLocalPlayer(GameClient.GetLocalPlayer());
    Trigger(UITRIGGER_SQUAD_OFFICER, pLocalPlayer == NULL ? false : pLocalPlayer->IsOfficer());

    Trigger(UITRIGGER_SQUAD_COLOR, pTeam->GetSquadColor(iSquad));

    const ivector &vTeammates(pTeam->GetClientList());
    ivector_cit cit(vTeammates.begin());
    int i(0);


    while (i < MAX_DISPLAY_SQUAD && cit != vTeammates.end())
    {
        IPlayerEntity *pOther(GameClient.GetPlayer(*cit));
        ++cit;
        if (pOther == NULL)
            continue;

        if (pOther->GetSquad() == iSquad && pOther->GetIndex() != pLocalClient->GetPlayerEntityIndex())
        {
            Trigger(UITRIGGER_SQUAD_INFO, true, i);
            Trigger(UITRIGGER_SQUAD_ICON, pOther->GetEntityIconPath(), i);
            Trigger(UITRIGGER_SQUAD_NAME, pOther->GetClientName(), i);
            Trigger(UITRIGGER_SQUAD_HEALTH_PERCENT, pOther->GetHealthPercent(), i);
            Trigger(UITRIGGER_SQUAD_MANA_PERCENT, pOther->GetManaPercent(), i);
            ++i;
        }
    }

    Trigger(UITRIGGER_SQUAD_COUNT, i);

    // Hide remaining squad info panels
    for (; i < MAX_DISPLAY_SQUAD; ++i)
        Trigger(UITRIGGER_SQUAD_INFO, false, i);
}


/*====================
  CGameInterfaceManager::UpdateCommanderSquads
  ====================*/
void    CGameInterfaceManager::UpdateCommanderSquads()
{
    PROFILE("CGameInterfaceManager::UpdateCommanderSquads");

    CEntityClientInfo *pLocalClient(GameClient.GetLocalClient());
    if (pLocalClient == NULL)
        return;

    if (!pLocalClient->HasFlags(CLIENT_INFO_IS_COMMANDER))
        return;

    CEntityTeamInfo *pTeam(GameClient.GetTeam(pLocalClient->GetTeam()));
    if (pTeam == NULL)
        return;

    uint uiSquad(0);
    for (; uiSquad < pTeam->GetNumOfficers(); ++uiSquad)
    {
        Trigger(UITRIGGER_COMMANDER_SQUAD_EXISTS, true, uiSquad);
        Trigger(UITRIGGER_COMMANDER_SQUAD_COLOR, pTeam->GetSquadColor(uiSquad), uiSquad);
        Trigger(UITRIGGER_COMMANDER_SQUAD_SIZE, pTeam->GetSquadSize(uiSquad), uiSquad);

        uint uiOfficeIndex(pTeam->GetOfficerGameIndex(uiSquad));
        IPlayerEntity *pOfficer(GameClient.GetPlayerEntity(uiOfficeIndex));
        if (pOfficer != NULL)
        {
            Trigger(UITRIGGER_COMMANDER_SQUAD_OFFICER_INDEX, pOfficer->GetIndex(), uiSquad);
            Trigger(UITRIGGER_COMMANDER_SQUAD_OFFICER_ICON, pOfficer->GetEntityIconPath(), uiSquad);
            Trigger(UITRIGGER_COMMANDER_SQUAD_OFFICER_HEALTH_PERCENT, pOfficer->GetHealthPercent(), uiSquad);
            Trigger(UITRIGGER_COMMANDER_SQUAD_OFFICER_MANA_PERCENT, pOfficer->GetManaPercent(), uiSquad);
            Trigger(UITRIGGER_COMMANDER_SQUAD_OFFICER_DEAD, pOfficer->GetStatus() == ENTITY_STATUS_DEAD, uiSquad);
        }

        uint uiIndex(0);
        for (uint uiMember(0); uiMember < pTeam->GetSquadSize(uiSquad) && uiIndex < MAX_DISPLAY_SQUAD; ++uiMember)
        {
            uint uiMemberIndex(pTeam->GetSquadMemberIndex(uiSquad, uiMember));
            IPlayerEntity *pMember(GameClient.GetPlayerEntity(uiMemberIndex));
            if (pMember == NULL)
                continue;
            if (pMember == pOfficer)
                continue;

            Trigger(UITRIGGER_COMMANDER_SQUAD_MEMBER_ACTIVE, true, uiIndex, uiSquad);
            Trigger(UITRIGGER_COMMANDER_SQUAD_MEMBER_INDEX, pMember->GetIndex(), uiIndex, uiSquad);
            Trigger(UITRIGGER_COMMANDER_SQUAD_MEMBER_ICON, pMember->GetEntityIconPath(), uiIndex, uiSquad);
            Trigger(UITRIGGER_COMMANDER_SQUAD_MEMBER_HEALTH_PERCENT, pMember->GetHealthPercent(), uiIndex, uiSquad);
            Trigger(UITRIGGER_COMMANDER_SQUAD_MEMBER_MANA_PERCENT, pMember->GetManaPercent(), uiIndex, uiSquad);
            Trigger(UITRIGGER_COMMANDER_SQUAD_MEMBER_DEAD, pMember->GetStatus() == ENTITY_STATUS_DEAD, uiIndex, uiSquad);
            ++uiIndex;
        }
        for ( ; uiIndex < MAX_DISPLAY_SQUAD; ++uiIndex)
        {
            Trigger(UITRIGGER_COMMANDER_SQUAD_MEMBER_ACTIVE, false, uiIndex, uiSquad);
            Trigger(UITRIGGER_COMMANDER_SQUAD_MEMBER_INDEX, INVALID_INDEX, uiIndex, uiSquad);
            Trigger(UITRIGGER_COMMANDER_SQUAD_MEMBER_ICON, _T(""), uiIndex, uiSquad);
            Trigger(UITRIGGER_COMMANDER_SQUAD_MEMBER_HEALTH_PERCENT, 0.0f, uiIndex, uiSquad);
            Trigger(UITRIGGER_COMMANDER_SQUAD_MEMBER_MANA_PERCENT, 0.0f, uiIndex, uiSquad);
            Trigger(UITRIGGER_COMMANDER_SQUAD_MEMBER_DEAD, false, uiIndex, uiSquad);
        }
    }
    for (; uiSquad < MAX_OFFICERS; ++uiSquad)
    {
        Trigger(UITRIGGER_COMMANDER_SQUAD_EXISTS, false, uiSquad);
    }
}


/*====================
  CGameInterfaceManager::UpdateBuildings
  ====================*/
void    CGameInterfaceManager::UpdateBuildings()
{
    PROFILE("CGameInterfaceManager::UpdateBuildings");

    CEntityClientInfo *pLocalClient(GameClient.GetLocalClient());
    if (pLocalClient == NULL)
        return;

    CEntityTeamInfo *pTeam(Game.GetTeam(pLocalClient->GetTeam()));
    if (pTeam == NULL)
        return;

    uint uiMainIndex(0);
    uint uiMajorIndex(0);
    uint uiMinorIndex(0);
    uint uiMineIndex(0);
    const uiset &setBuildings(pTeam->GetBuildingSet());
    for (uiset_cit it(setBuildings.begin()); it != setBuildings.end(); ++it)
    {
        IBuildingEntity *pBuilding(GameClient.GetBuildingEntity(*it));
        if (pBuilding == NULL)
            continue;
        if (pBuilding->GetStatus() != ENTITY_STATUS_ACTIVE && pBuilding->GetStatus() != ENTITY_STATUS_SPAWNING)
            continue;
        if (pBuilding->IsCommandCenter())
            continue;

        uint uiIndex(0);
        if (pBuilding->IsMine())
        {
            if (uiMineIndex >= MAX_MINE_BUILDING_INFO)
                continue;
            uiIndex = START_MINE_BUILDING_INFO + uiMineIndex;
            ++uiMineIndex;

            IPropEntity *pProp(GameClient.GetPropEntity(pBuilding->GetFoundation()));
            IPropFoundation *pFoundation(NULL);
            if (pProp != NULL && (pFoundation = pProp->GetAsFoundation()) != NULL)
            {
                Trigger(UITRIGGER_BUILDING_INFO_GOLD_PERCENT, pFoundation->GetRemainingGoldPercent(), uiIndex);
                Trigger(UITRIGGER_BUILDING_INFO_GOLD_VALUE, pFoundation->GetRemainingGold(), uiIndex);
                Trigger(UITRIGGER_BUILDING_INFO_MAX_GOLD, pFoundation->GetTotalGold(), uiIndex);
                Trigger(UITRIGGER_BUILDING_INFO_EXTRACTION_RATE, pFoundation->GetRemainingGold() > 0 ? pFoundation->GetHarvestRate() : 0, uiIndex);
            }
        }
        else if (pBuilding->GetIsMainBuilding())
        {
            if (uiMainIndex >= MAX_MAIN_BUILDING_INFO)
                continue;
            uiIndex = START_MAIN_BUILDING_INFO + uiMainIndex;
            ++uiMainIndex;
        }
        else if (pBuilding->GetIsMajorBuilding())
        {
            if (uiMajorIndex >= MAX_MAJOR_BUILDING_INFO)
                continue;
            uiIndex = START_MAJOR_BUILDING_INFO + uiMajorIndex;
            ++uiMajorIndex;
        }
        else
        {
            if (uiMinorIndex >= MAX_MINOR_BUILDING_INFO)
                continue;
            uiIndex = START_MINOR_BUILDING_INFO + uiMinorIndex;
            ++uiMinorIndex;
        }

        Trigger(UITRIGGER_BUILDING_INFO_EXISTS, true, uiIndex);
        Trigger(UITRIGGER_BUILDING_INFO_INDEX, pBuilding->GetIndex(), uiIndex);
        Trigger(UITRIGGER_BUILDING_INFO_NAME, pBuilding->GetEntityName(), uiIndex);
        Trigger(UITRIGGER_BUILDING_INFO_ARMOR, pBuilding->GetArmor(), uiIndex);
        Trigger(UITRIGGER_BUILDING_INFO_ICON, pBuilding->GetEntityIconPath(), uiIndex);
        Trigger(UITRIGGER_BUILDING_INFO_HEALTH_PERCENT, pBuilding->GetHealthPercent(), uiIndex);
        Trigger(UITRIGGER_BUILDING_INFO_HEALTH_VALUE, pBuilding->GetHealth(), uiIndex);
        Trigger(UITRIGGER_BUILDING_INFO_MAX_HEALTH, pBuilding->GetMaxHealth(), uiIndex);
        Trigger(UITRIGGER_BUILDING_INFO_UPKEEP_COST, pBuilding->GetActiveUpkeepCost(), uiIndex);
        Trigger(UITRIGGER_BUILDING_INFO_UPKEEP_ENABLED, pBuilding->GetUpkeepLevel() > 0.0f, uiIndex);
        Trigger(UITRIGGER_BUILDING_INFO_BUILD_PERCENT, pBuilding->GetBuildPercent(), uiIndex);
        Trigger(UITRIGGER_BUILDING_INFO_REPAIRABLE, pBuilding->HasNetFlags(ENT_NET_FLAG_NO_REPAIR), uiIndex);
    }
    for ( ; uiMainIndex < MAX_MAIN_BUILDING_INFO; ++uiMainIndex)
        Trigger(UITRIGGER_BUILDING_INFO_EXISTS, false, START_MAIN_BUILDING_INFO + uiMainIndex);
    for ( ; uiMajorIndex < MAX_MAJOR_BUILDING_INFO; ++uiMajorIndex)
        Trigger(UITRIGGER_BUILDING_INFO_EXISTS, false, START_MAJOR_BUILDING_INFO + uiMajorIndex);
    for ( ; uiMinorIndex < MAX_MINOR_BUILDING_INFO; ++uiMinorIndex)
        Trigger(UITRIGGER_BUILDING_INFO_EXISTS, false, START_MINOR_BUILDING_INFO + uiMinorIndex);
    for ( ; uiMineIndex < MAX_MINE_BUILDING_INFO; ++uiMineIndex)
        Trigger(UITRIGGER_BUILDING_INFO_EXISTS, false, START_MINE_BUILDING_INFO + uiMineIndex);
}


/*====================
  CGameInterfaceManager::UpdateBuildButtons
  ====================*/
void    CGameInterfaceManager::UpdateBuildButtons()
{
    // Get local player
    CEntityClientInfo *pLocalClient(GameClient.GetLocalClient());
    if (pLocalClient == NULL)
        return;

    // Get team info
    CEntityTeamInfo *pTeam(GameClient.GetTeam(pLocalClient->GetTeam()));
    if (pTeam == NULL)
        return;

    svector vParams(2);

    for (map<const IEntityAllocator *, int>::iterator it(m_mapCanBuild.begin()); it != m_mapCanBuild.end(); ++it)
    {
        const IBuildingEntity::CEntityConfig *pConfig(static_cast<const IBuildingEntity::CEntityConfig *>(it->first->GetEntityConfig()));

        if (!pConfig)
            continue;

        const tstring &sBuilding(it->first->GetName());

        // Check cost
        if (pConfig->GetCost() > int(pTeam->GetGold()))
        {
            if (it->second != 0)
            {
                it->second = 0;
                
                vParams[0] = sBuilding;
                vParams[1] = _T("false");
                m_CanBuild.Trigger(vParams);
            }
            continue;
        }

        // Check count
        if (pConfig->GetMaxBuild() > 0 && pTeam->GetBuildingCount(sBuilding) >= pConfig->GetMaxBuild())
        {
            if (it->second != 0)
            {
                it->second = 0;
                
                vParams[0] = sBuilding;
                vParams[1] = _T("false");
                m_CanBuild.Trigger(vParams);
            }
            continue;
        }

        // Check prerequisites
        svector vPrerequisites(TokenizeString(pConfig->GetPrerequisite(), _T(' '))); // Bad
        svector_it itPrereq(vPrerequisites.begin());
        for (; itPrereq != vPrerequisites.end(); ++itPrereq)
        {
            if (!pTeam->HasBuilding(*itPrereq))
                break;
        }

        if (itPrereq != vPrerequisites.end())
        {
            if (it->second != 0)
            {
                it->second = 0;
                
                vParams[0] = sBuilding;
                vParams[1] = _T("false");
                m_CanBuild.Trigger(vParams);
            }
            continue;
        }

        // Passed all checks
        if (it->second != 1)
        {
            it->second = 1;

            vParams[0] = it->first->GetName();
            vParams[1] = _T("true");
            m_CanBuild.Trigger(vParams);
        }
    }
}


/*====================
  CGameInterfaceManager::UpdateHoverInfo
  ====================*/
void    CGameInterfaceManager::UpdateHoverInfo(uint uiTargetIndex, bool bShow)
{
    PROFILE("CGameInterfaceManager::UpdateHoverInfo");

    CEntityClientInfo *pLocalClient(GameClient.GetLocalClient());
    if (pLocalClient == NULL)
        return;

    IVisualEntity *pEntity(GameClient.GetClientEntityCurrent(uiTargetIndex));
    if (uiTargetIndex == INVALID_INDEX || pEntity == NULL)
    {
        HoverInfoPlayer.Hide();
        HoverInfoNpc.Hide();
        HoverInfoBuilding.Hide();
        HoverInfoGadget.Hide();
        HoverInfoChest.Hide();
        return;
    }
    
    if (pEntity->IsStealthed()) // Why here and not in UpdateHoverEntity and the commander selection code?
        return;

    if (pEntity->IsPlayer() && (pEntity->GetStatus() == ENTITY_STATUS_ACTIVE || Game.IsCommander()))
    {
        if (bShow)
            HoverInfoPlayer.Show();
        HoverInfoNpc.Hide();
        HoverInfoBuilding.Hide();
        HoverInfoGadget.Hide();
        HoverInfoChest.Hide();

        IPlayerEntity *pPlayer(pEntity->GetAsPlayerEnt());
        CEntityClientInfo *pClient(GameClient.GetClientInfo(pPlayer->GetDisguiseClient()));
        if (pClient == NULL)
            return;
        IPlayerEntity *pApparentPlayer(pClient->GetPlayerEntity());
        if (pApparentPlayer == NULL)
            pApparentPlayer = pPlayer;

        Trigger(UITRIGGER_HOVER_NAME, pClient->GetName());
        
        CEntityTeamInfo *pTeam(GameClient.GetTeam(pClient->GetTeam()));
        if (pTeam != NULL)
            Trigger(UITRIGGER_HOVER_RACE, pTeam->GetDefinition()->GetName());
        
        Trigger(UITRIGGER_HOVER_CLASS, pApparentPlayer->GetTypeName());
        Trigger(UITRIGGER_HOVER_ICON, pApparentPlayer->GetEntityIconPath());

        Trigger(UITRIGGER_HOVER_HEALTH_VALUE, pPlayer->GetHealthPercent() * pApparentPlayer->GetMaxHealth());
        Trigger(UITRIGGER_HOVER_MANA_VALUE, pPlayer->GetManaPercent() * pApparentPlayer->GetMaxMana());
        Trigger(UITRIGGER_HOVER_STAMINA_VALUE, pPlayer->GetStaminaPercent() * pApparentPlayer->GetMaxStamina());
        Trigger(UITRIGGER_HOVER_HEALTH_PERCENT, pPlayer->GetHealthPercent());
        Trigger(UITRIGGER_HOVER_MANA_PERCENT, pPlayer->GetManaPercent());
        Trigger(UITRIGGER_HOVER_STAMINA_PERCENT, pPlayer->GetStaminaPercent());
        Trigger(UITRIGGER_HOVER_SOUL_COUNT, pClient->GetSouls());

        for (int iSlot(INVENTORY_START_BACKPACK); iSlot < INVENTORY_END_BACKPACK; ++iSlot)
        {
            IInventoryItem *pItem(pPlayer->GetItem(iSlot));
            if (pItem == NULL)
            {
                Trigger(UITRIGGER_HOVER_INVENTORY_EXISTS, false, iSlot);
                continue;
            }

            Trigger(UITRIGGER_HOVER_INVENTORY_EXISTS, true, iSlot);
            Trigger(UITRIGGER_HOVER_INVENTORY_ICON, pItem->GetIconImageList(), iSlot);
            Trigger(UITRIGGER_HOVER_INVENTORY_AMMO_COUNT, pItem->GetAmmo(), iSlot);
        }

        int iBuffCount(0);
        int iDebuffCount(0);
        for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
        {
            IEntityState *pState(pPlayer->GetState(i));
            if (pState == NULL)
                continue;
            if (!pState->GetDisplayState())
                continue;
            if (pState->GetIsSecret() && pLocalClient->GetTeam() != pPlayer->GetTeam())
                continue;

            if (pState->IsBuff() && iBuffCount < MAX_DISPLAY_BUFFS)
            {
                Trigger(UITRIGGER_HOVER_BUFF_ACTIVE, true, iBuffCount);
                Trigger(UITRIGGER_HOVER_BUFF_ICON, pState->GetIconPath(), iBuffCount);
                ++iBuffCount;
            }
            if (pState->IsDebuff() && iDebuffCount < MAX_DISPLAY_DEBUFFS)
            {
                Trigger(UITRIGGER_HOVER_DEBUFF_ACTIVE, true, iDebuffCount);
                Trigger(UITRIGGER_HOVER_DEBUFF_ICON, pState->GetIconPath(), iDebuffCount);
                ++iDebuffCount;
            }
        }
        for (int i(iBuffCount); i < MAX_DISPLAY_BUFFS; ++i)
            Trigger(UITRIGGER_HOVER_BUFF_ACTIVE, false, i);
        for (int i(iDebuffCount); i < MAX_DISPLAY_DEBUFFS; ++i)
            Trigger(UITRIGGER_HOVER_DEBUFF_ACTIVE, false, i);

        Trigger(UITRIGGER_HOVER_LEVEL, pClient->GetLevel());
        Trigger(UITRIGGER_HOVER_EXPERIENCE, pClient->GetPercentNextLevel());
        Trigger(UITRIGGER_HOVER_GOLD, pClient->GetGold());

        Trigger(UITRIGGER_HOVER_MAX_HEALTH, pApparentPlayer->GetMaxHealth());
        Trigger(UITRIGGER_HOVER_MAX_MANA, pApparentPlayer->GetMaxMana());
        Trigger(UITRIGGER_HOVER_MAX_STAMINA, pApparentPlayer->GetMaxStamina());

        // FIXME
        //SetHoverMeleeQuickDamage(pPlayer->GetQuickAttackMinDamage(0), pPlayer->GetQuickAttackMaxDamage(0));
        //SetHoverMeleeStrongDamage(pPlayer->GetStrongAttackMinDamage(0), pPlayer->GetStrongAttackMaxDamage(0));
        //SetHoverRanged1Damage(pPlayer->GetRangedMinDamage(), pPlayer->GetRangedMaxDamage());
        //SetHoverRanged2Damage(pPlayer->GetRangedMinDamage(INVENTORY_RANGED2), pPlayer->GetRangedMaxDamage(INVENTORY_RANGED2));

        Trigger(UITRIGGER_HOVER_ARMOR, pApparentPlayer->GetArmor());
        Trigger(UITRIGGER_HOVER_ARMOR_REDUCTION, pApparentPlayer->GetArmorDamageReduction(pPlayer->GetArmor()));

        Trigger(UITRIGGER_HOVER_MODEL, g_ResourceManager.GetPath(pPlayer->GetModelHandle()));
        Trigger(UITRIGGER_HOVER_COMMANDER_PORTRAIT, pApparentPlayer->GetCommanderPortraitPath());
    }
    else if (pEntity->IsBuilding()) 
    {
        HoverInfoPlayer.Hide();
        HoverInfoNpc.Hide();
        if (bShow)
            HoverInfoBuilding.Show();
        HoverInfoGadget.Hide();
        HoverInfoChest.Hide();

        IBuildingEntity *pBuilding(pEntity->GetAsBuilding());

        svector vParams;
        if (pBuilding->GetTeam() == pLocalClient->GetTeam())
        {
            vParams.push_back(_T("#00CC00CC"));
            vParams.push_back(_T("white"));
        }
        else
        {
            vParams.push_back(_T("#AA0000CC"));
            vParams.push_back(_T("yellow"));
        }

        Trigger(UITRIGGER_HOVER_INDEX, pBuilding->GetIndex());
        Trigger(UITRIGGER_HOVER_COLORS, vParams);
        Trigger(UITRIGGER_HOVER_NAME, pBuilding->GetEntityName());
        Trigger(UITRIGGER_HOVER_DESCRIPTION, pBuilding->GetEntityDescription());
        Trigger(UITRIGGER_HOVER_ICON, pBuilding->GetEntityIconPath());

        Trigger(UITRIGGER_HOVER_HEALTH_VALUE, pBuilding->GetHealth());
        Trigger(UITRIGGER_HOVER_HEALTH_PERCENT, pBuilding->GetHealthPercent());
        Trigger(UITRIGGER_HOVER_MAX_HEALTH, pBuilding->GetMaxHealth());

        Trigger(UITRIGGER_HOVER_ARMOR, pBuilding->GetArmor());
        Trigger(UITRIGGER_HOVER_ARMOR_REDUCTION, pBuilding->GetArmorDamageReduction());

        Trigger(UITRIGGER_HOVER_MODEL, g_ResourceManager.GetPath(pBuilding->GetModelHandle()));
        Trigger(UITRIGGER_HOVER_COMMANDER_PORTRAIT, pBuilding->GetCommanderPortraitPath());

        Trigger(UITRIGGER_HOVER_BUILD_PERCENT, pBuilding->GetBuildPercent());

        Trigger(UITRIGGER_REPAIRABLE, pBuilding->HasNetFlags(ENT_NET_FLAG_NO_REPAIR));

        if (pBuilding->IsMine())
        {
            IPropEntity *pProp(GameClient.GetPropEntity(pBuilding->GetFoundation()));
            IPropFoundation *pFoundation(NULL);
            if (pProp != NULL &&
                (pFoundation = pProp->GetAsFoundation()) != NULL)
            {
                Trigger(UITRIGGER_HOVER_GOLD_VALUE, pFoundation->GetRemainingGold());
                Trigger(UITRIGGER_HOVER_GOLD_PERCENT, pFoundation->GetRemainingGoldPercent());
                Trigger(UITRIGGER_HOVER_MAX_GOLD, pFoundation->GetTotalGold());
            }
            Trigger(UITRIGGER_HOVER_EXTRACTION_RATE, pBuilding->GetIncomeAmount());
            Trigger(UITRIGGER_HOVER_UPKEEP_COST, -1);
        }
        else
        {
            Trigger(UITRIGGER_HOVER_GOLD_VALUE, -1);
            Trigger(UITRIGGER_HOVER_GOLD_PERCENT, -1.0f);
            Trigger(UITRIGGER_HOVER_MAX_GOLD, -1);
            Trigger(UITRIGGER_HOVER_EXTRACTION_RATE, -1);
            Trigger(UITRIGGER_HOVER_UPKEEP_COST, pBuilding->GetUpkeepCost());
            Trigger(UITRIGGER_HOVER_UPKEEP_ACTIVE, pBuilding->GetUpkeepLevel() == 1.0f);
        }
    }
    else if (pEntity->IsNpc() || pEntity->IsPet())
    {
        HoverInfoPlayer.Hide();
        if (bShow)
            HoverInfoNpc.Show();
        HoverInfoBuilding.Hide();
        HoverInfoGadget.Hide();
        HoverInfoChest.Hide();

        ICombatEntity *pCombatEnt(pEntity->GetAsCombatEnt());

        Trigger(UITRIGGER_HOVER_NAME, pCombatEnt->GetEntityName());
        Trigger(UITRIGGER_HOVER_DESCRIPTION, pCombatEnt->GetEntityDescription());
        Trigger(UITRIGGER_HOVER_ICON, pCombatEnt->GetEntityIconPath());
        if (pCombatEnt->IsNpc())
            Trigger(UITRIGGER_HOVER_LEVEL, pCombatEnt->GetAsNpc()->GetLevel());
        else
            Trigger(UITRIGGER_HOVER_LEVEL, -1);

        Trigger(UITRIGGER_HOVER_HEALTH_VALUE, pCombatEnt->GetHealth());
        Trigger(UITRIGGER_HOVER_HEALTH_PERCENT, pCombatEnt->GetHealthPercent());
        Trigger(UITRIGGER_HOVER_MAX_HEALTH, pCombatEnt->GetMaxHealth());

        Trigger(UITRIGGER_HOVER_MANA_VALUE, pCombatEnt->GetMana());
        Trigger(UITRIGGER_HOVER_MANA_PERCENT, pCombatEnt->GetManaPercent());
        Trigger(UITRIGGER_HOVER_MAX_MANA, pCombatEnt->GetMaxMana());

        Trigger(UITRIGGER_HOVER_ARMOR, pCombatEnt->GetArmor());
        Trigger(UITRIGGER_HOVER_ARMOR_REDUCTION, pCombatEnt->GetArmorDamageReduction(pCombatEnt->GetArmor()));

        Trigger(UITRIGGER_HOVER_MODEL, g_ResourceManager.GetPath(pCombatEnt->GetModelHandle()));
        Trigger(UITRIGGER_HOVER_COMMANDER_PORTRAIT, pCombatEnt->GetCommanderPortraitPath());
    }
    else if (pEntity->IsGadget()) 
    {
        HoverInfoPlayer.Hide();
        HoverInfoNpc.Hide();
        HoverInfoBuilding.Hide();
        if (bShow)
            HoverInfoGadget.Show();
        HoverInfoChest.Hide();

        IGadgetEntity *pGadget(pEntity->GetAsGadget());

        svector vParams;
        if (pGadget->GetTeam() == pLocalClient->GetTeam())
        {
            vParams.push_back(_T("#00CC00CC"));
            vParams.push_back(_T("white"));
        }
        else
        {
            vParams.push_back(_T("#AA0000CC"));
            vParams.push_back(_T("yellow"));
        }

        Trigger(UITRIGGER_HOVER_COLORS, vParams);
        Trigger(UITRIGGER_HOVER_NAME, pGadget->GetEntityName());
        Trigger(UITRIGGER_HOVER_DESCRIPTION, pGadget->GetEntityDescription());
        Trigger(UITRIGGER_HOVER_ICON, pGadget->GetEntityIconPath());

        Trigger(UITRIGGER_HOVER_HEALTH_VALUE, pGadget->GetHealth());
        Trigger(UITRIGGER_HOVER_HEALTH_PERCENT, pGadget->GetHealthPercent());
        Trigger(UITRIGGER_HOVER_MAX_HEALTH, pGadget->GetMaxHealth());

        Trigger(UITRIGGER_HOVER_LIFETIME_VALUE, pGadget->GetRemainingLifetime());
        Trigger(UITRIGGER_HOVER_LIFETIME_PERCENT, pGadget->GetRemainingLifetimePercent());

        Trigger(UITRIGGER_HOVER_COMMANDER_PORTRAIT, pGadget->GetCommanderPortraitPath());
    }
    else if (pEntity->IsProp())
    {
        IPropEntity *pProp(pEntity->GetAsProp());
        IPropFoundation *pFoundation(pProp->GetAsFoundation());
        if (pFoundation != NULL)
        {
            HoverInfoPlayer.Hide();
            HoverInfoNpc.Hide();
            if (bShow)
                HoverInfoBuilding.Show();
            HoverInfoGadget.Hide();
            HoverInfoChest.Hide();

            svector vParams;
            vParams.push_back(_T("#00CC00CC"));
            vParams.push_back(_T("white"));
            Trigger(UITRIGGER_HOVER_COLORS, vParams);

            Trigger(UITRIGGER_HOVER_NAME, pProp->GetEntityName());
            Trigger(UITRIGGER_HOVER_DESCRIPTION, pProp->GetEntityDescription());
            Trigger(UITRIGGER_HOVER_ICON, pProp->GetEntityIconPath());

            if (pFoundation->IsMine())
            {
                Trigger(UITRIGGER_HOVER_GOLD_VALUE, pFoundation->GetRemainingGold());
                Trigger(UITRIGGER_HOVER_GOLD_PERCENT, pFoundation->GetRemainingGoldPercent());
                Trigger(UITRIGGER_HOVER_MAX_GOLD, pFoundation->GetTotalGold());
                Trigger(UITRIGGER_HOVER_EXTRACTION_RATE, pFoundation->GetHarvestRate());
            }
            else
            {
                Trigger(UITRIGGER_HOVER_GOLD_VALUE, -1);
                Trigger(UITRIGGER_HOVER_GOLD_PERCENT, -1.0f);
                Trigger(UITRIGGER_HOVER_MAX_GOLD, -1);
                Trigger(UITRIGGER_HOVER_EXTRACTION_RATE, -1);
            }

            Trigger(UITRIGGER_HOVER_UPKEEP_COST, -1);
            Trigger(UITRIGGER_HOVER_HEALTH_VALUE, -1.0f);
            Trigger(UITRIGGER_HOVER_HEALTH_PERCENT, -1.0f);
            Trigger(UITRIGGER_HOVER_MAX_HEALTH, -1.0f);
            Trigger(UITRIGGER_HOVER_ARMOR, -1.0f);
            Trigger(UITRIGGER_HOVER_BUILD_PERCENT, 1.0f);

            Trigger(UITRIGGER_HOVER_COMMANDER_PORTRAIT, pProp->GetCommanderPortraitPath());
        }
    }
    else if (pEntity->GetType() == Entity_Chest)
    {
        HoverInfoPlayer.Hide();
        HoverInfoNpc.Hide();
        HoverInfoBuilding.Hide();
        HoverInfoGadget.Hide();
        if (bShow)
            HoverInfoChest.Show();

        CEntityChest *pChest(static_cast<CEntityChest*>(pEntity));

        svector vParams;
        vParams.push_back(_T("silver"));
        vParams.push_back(_T("white"));
        Trigger(UITRIGGER_HOVER_COLORS, vParams);
        
        IInventoryItem *pItem(pChest->GetItem(0));
        if (!pItem)
        {
            Trigger(UITRIGGER_HOVER_NAME, _T("Empty"));
            Trigger(UITRIGGER_HOVER_DESCRIPTION, _T(""));

            Trigger(UITRIGGER_HOVER_INVENTORY_EXISTS, false, 0);
            Trigger(UITRIGGER_HOVER_INVENTORY_AMMO_COUNT, 0, 0);
        }
        else
        {
            Trigger(UITRIGGER_HOVER_NAME, pItem->GetEntityName());
            Trigger(UITRIGGER_HOVER_DESCRIPTION, pItem->GetEntityDescription());

            Trigger(UITRIGGER_HOVER_INVENTORY_EXISTS, true, 0);
            Trigger(UITRIGGER_HOVER_INVENTORY_ICON, pItem->GetIconImageList(), 0);
            Trigger(UITRIGGER_HOVER_INVENTORY_AMMO_COUNT, pItem->GetAmmoCount(), 0);
        }

        Trigger(UITRIGGER_HOVER_COMMANDER_PORTRAIT, pChest->GetCommanderPortraitPath());
    }
    else
    {
        HoverInfoPlayer.Hide();
        HoverInfoNpc.Hide();
        HoverInfoBuilding.Hide();
        HoverInfoGadget.Hide();
    }
}


/*====================
  CGameInterfaceManager::UpdateCommSelectionInfo
  ====================*/
void    CGameInterfaceManager::UpdateCommSelectionInfo(uint uiTargetIndex, bool bShow)
{
    PROFILE("CGameInterfaceManager::UpdateCommSelectionInfo");

    if (!Game.IsCommander())
        return;

    CEntityClientInfo *pLocalClient(GameClient.GetLocalClient());
    if (pLocalClient == NULL)
        return;

    IVisualEntity *pEntity(GameClient.GetClientEntityCurrent(uiTargetIndex));
    if (uiTargetIndex == INVALID_INDEX || pEntity == NULL)
    {
        CommSelectionPlayer.Hide();
        CommSelectionNpc.Hide();
        CommSelectionBuilding.Hide();
        CommSelectionGadget.Hide();
        CommSelectionChest.Hide();
        return;
    }

    if (pEntity->IsStealthed())
        return;

    if (pEntity->IsPlayer())
    {
        if (bShow)
            CommSelectionPlayer.Show();
        CommSelectionNpc.Hide();
        CommSelectionBuilding.Hide();
        CommSelectionGadget.Hide();
        CommSelectionChest.Hide();

        IPlayerEntity *pPlayer(pEntity->GetAsPlayerEnt());
        CEntityClientInfo *pClient(GameClient.GetClientInfo(pPlayer->GetDisguiseClient()));
        if (pClient == NULL)
            return;
        IPlayerEntity *pApparentPlayer(pClient->GetPlayerEntity());
        if (pApparentPlayer == NULL)
            pApparentPlayer = pPlayer;

        Trigger(UITRIGGER_COMMSELECTION_NAME, pClient->GetName());
        
        CEntityTeamInfo *pTeam(GameClient.GetTeam(pClient->GetTeam()));
        if (pTeam != NULL)
            Trigger(UITRIGGER_COMMSELECTION_RACE, pTeam->GetDefinition()->GetName());
        
        Trigger(UITRIGGER_COMMSELECTION_CLASS, pApparentPlayer->GetTypeName());
        Trigger(UITRIGGER_COMMSELECTION_ICON, pApparentPlayer->GetEntityIconPath());

        Trigger(UITRIGGER_COMMSELECTION_HEALTH_VALUE, pPlayer->GetHealthPercent() * pApparentPlayer->GetMaxHealth());
        Trigger(UITRIGGER_COMMSELECTION_MANA_VALUE, pPlayer->GetManaPercent() * pApparentPlayer->GetMaxMana());
        Trigger(UITRIGGER_COMMSELECTION_STAMINA_VALUE, pPlayer->GetStaminaPercent() * pApparentPlayer->GetMaxStamina());
        Trigger(UITRIGGER_COMMSELECTION_HEALTH_PERCENT, pPlayer->GetHealthPercent());
        Trigger(UITRIGGER_COMMSELECTION_MANA_PERCENT, pPlayer->GetManaPercent());
        Trigger(UITRIGGER_COMMSELECTION_STAMINA_PERCENT, pPlayer->GetStaminaPercent());
        Trigger(UITRIGGER_COMMSELECTION_SOUL_COUNT, pClient->GetSouls());

        for (int iSlot(INVENTORY_START_BACKPACK); iSlot < INVENTORY_END_BACKPACK; ++iSlot)
        {
            IInventoryItem *pItem(pPlayer->GetItem(iSlot));
            if (pItem == NULL)
            {
                Trigger(UITRIGGER_COMMSELECTION_INVENTORY_EXISTS, false, iSlot);
                continue;
            }

            Trigger(UITRIGGER_COMMSELECTION_INVENTORY_EXISTS, true, iSlot);
            Trigger(UITRIGGER_COMMSELECTION_INVENTORY_ICON, pItem->GetIconImageList(), iSlot);
            Trigger(UITRIGGER_COMMSELECTION_INVENTORY_AMMO_COUNT, pItem->GetAmmo(), iSlot);
        }

        int iBuffCount(0);
        int iDebuffCount(0);
        for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
        {
            IEntityState *pState(pPlayer->GetState(i));
            if (pState == NULL)
                continue;
            if (!pState->GetDisplayState())
                continue;
            if (pState->GetIsSecret() && pLocalClient->GetTeam() != pPlayer->GetTeam())
                continue;

            if (pState->IsBuff() && iBuffCount < MAX_DISPLAY_BUFFS)
            {
                Trigger(UITRIGGER_COMMSELECTION_BUFF_ACTIVE, true, iBuffCount);
                Trigger(UITRIGGER_COMMSELECTION_BUFF_ICON, pState->GetIconPath(), iBuffCount);
                ++iBuffCount;
            }
            if (pState->IsDebuff() && iDebuffCount < MAX_DISPLAY_DEBUFFS)
            {
                Trigger(UITRIGGER_COMMSELECTION_DEBUFF_ACTIVE, true, iDebuffCount);
                Trigger(UITRIGGER_COMMSELECTION_DEBUFF_ICON, pState->GetIconPath(), iDebuffCount);
                ++iDebuffCount;
            }
        }
        for (int i(iBuffCount); i < MAX_DISPLAY_BUFFS; ++i)
            Trigger(UITRIGGER_COMMSELECTION_BUFF_ACTIVE, false, i);
        for (int i(iDebuffCount); i < MAX_DISPLAY_DEBUFFS; ++i)
            Trigger(UITRIGGER_COMMSELECTION_DEBUFF_ACTIVE, false, i);

        Trigger(UITRIGGER_COMMSELECTION_LEVEL, pClient->GetLevel());
        Trigger(UITRIGGER_COMMSELECTION_EXPERIENCE, pClient->GetPercentNextLevel());
        Trigger(UITRIGGER_COMMSELECTION_GOLD, pClient->GetGold());

        Trigger(UITRIGGER_COMMSELECTION_MAX_HEALTH, pApparentPlayer->GetMaxHealth());
        Trigger(UITRIGGER_COMMSELECTION_MAX_MANA, pApparentPlayer->GetMaxMana());
        Trigger(UITRIGGER_COMMSELECTION_MAX_STAMINA, pApparentPlayer->GetMaxStamina());

        // FIXME
        //SetHoverMeleeQuickDamage(pPlayer->GetQuickAttackMinDamage(0), pPlayer->GetQuickAttackMaxDamage(0));
        //SetHoverMeleeStrongDamage(pPlayer->GetStrongAttackMinDamage(0), pPlayer->GetStrongAttackMaxDamage(0));
        //SetHoverRanged1Damage(pPlayer->GetRangedMinDamage(), pPlayer->GetRangedMaxDamage());
        //SetHoverRanged2Damage(pPlayer->GetRangedMinDamage(INVENTORY_RANGED2), pPlayer->GetRangedMaxDamage(INVENTORY_RANGED2));

        Trigger(UITRIGGER_COMMSELECTION_ARMOR, pApparentPlayer->GetArmor());
        Trigger(UITRIGGER_COMMSELECTION_ARMOR_REDUCTION, pApparentPlayer->GetArmorDamageReduction(pPlayer->GetArmor()));

        Trigger(UITRIGGER_COMMSELECTION_MODEL, g_ResourceManager.GetPath(pPlayer->GetModelHandle()));
        Trigger(UITRIGGER_COMMSELECTION_COMMANDER_PORTRAIT, pApparentPlayer->GetCommanderPortraitPath());
    }
    else if (pEntity->IsBuilding()) 
    {
        CommSelectionPlayer.Hide();
        CommSelectionNpc.Hide();
        if (bShow)
            CommSelectionBuilding.Show();
        CommSelectionGadget.Hide();
        CommSelectionChest.Hide();

        IBuildingEntity *pBuilding(pEntity->GetAsBuilding());

        svector vParams;
        if (pBuilding->GetTeam() == pLocalClient->GetTeam())
        {
            vParams.push_back(_T("#00CC00CC"));
            vParams.push_back(_T("white"));
        }
        else
        {
            vParams.push_back(_T("#AA0000CC"));
            vParams.push_back(_T("yellow"));
        }

        Trigger(UITRIGGER_COMMSELECTION_INDEX, pBuilding->GetIndex());
        Trigger(UITRIGGER_COMMSELECTION_COLORS, vParams);
        Trigger(UITRIGGER_COMMSELECTION_NAME, pBuilding->GetEntityName());
        Trigger(UITRIGGER_COMMSELECTION_DESCRIPTION, pBuilding->GetEntityDescription());
        Trigger(UITRIGGER_COMMSELECTION_ICON, pBuilding->GetEntityIconPath());

        Trigger(UITRIGGER_COMMSELECTION_HEALTH_VALUE, pBuilding->GetHealth());
        Trigger(UITRIGGER_COMMSELECTION_HEALTH_PERCENT, pBuilding->GetHealthPercent());
        Trigger(UITRIGGER_COMMSELECTION_MAX_HEALTH, pBuilding->GetMaxHealth());

        Trigger(UITRIGGER_COMMSELECTION_ARMOR, pBuilding->GetArmor());
        Trigger(UITRIGGER_COMMSELECTION_ARMOR_REDUCTION, pBuilding->GetArmorDamageReduction());

        Trigger(UITRIGGER_COMMSELECTION_MODEL, g_ResourceManager.GetPath(pBuilding->GetModelHandle()));
        Trigger(UITRIGGER_COMMSELECTION_COMMANDER_PORTRAIT, pBuilding->GetCommanderPortraitPath());

        Trigger(UITRIGGER_COMMSELECTION_BUILD_PERCENT, pBuilding->GetBuildPercent());

        Trigger(UITRIGGER_REPAIRABLE, pBuilding->HasNetFlags(ENT_NET_FLAG_NO_REPAIR));

        if (pBuilding->IsMine())
        {
            IPropEntity *pProp(GameClient.GetPropEntity(pBuilding->GetFoundation()));
            IPropFoundation *pFoundation(NULL);
            if (pProp != NULL &&
                (pFoundation = pProp->GetAsFoundation()) != NULL)
            {
                Trigger(UITRIGGER_COMMSELECTION_GOLD_VALUE, pFoundation->GetRemainingGold());
                Trigger(UITRIGGER_COMMSELECTION_GOLD_PERCENT, pFoundation->GetRemainingGoldPercent());
                Trigger(UITRIGGER_COMMSELECTION_MAX_GOLD, pFoundation->GetTotalGold());
            }
            Trigger(UITRIGGER_COMMSELECTION_EXTRACTION_RATE, pBuilding->GetIncomeAmount());
            Trigger(UITRIGGER_COMMSELECTION_UPKEEP_COST, -1);
        }
        else
        {
            Trigger(UITRIGGER_COMMSELECTION_GOLD_VALUE, -1);
            Trigger(UITRIGGER_COMMSELECTION_GOLD_PERCENT, -1.0f);
            Trigger(UITRIGGER_COMMSELECTION_MAX_GOLD, -1);
            Trigger(UITRIGGER_COMMSELECTION_EXTRACTION_RATE, -1);
            Trigger(UITRIGGER_COMMSELECTION_UPKEEP_COST, pBuilding->GetUpkeepCost());
            Trigger(UITRIGGER_COMMSELECTION_UPKEEP_ACTIVE, pBuilding->GetUpkeepLevel() == 1.0f);
        }
    }
    else if (pEntity->IsNpc() || pEntity->IsPet())
    {
        CommSelectionPlayer.Hide();
        if (bShow)
            CommSelectionNpc.Show();
        CommSelectionBuilding.Hide();
        CommSelectionGadget.Hide();
        CommSelectionChest.Hide();

        ICombatEntity *pCombatEnt(pEntity->GetAsCombatEnt());

        Trigger(UITRIGGER_COMMSELECTION_NAME, pCombatEnt->GetEntityName());
        Trigger(UITRIGGER_COMMSELECTION_DESCRIPTION, pCombatEnt->GetEntityDescription());
        Trigger(UITRIGGER_COMMSELECTION_ICON, pCombatEnt->GetEntityIconPath());
        if (pCombatEnt->IsNpc())
            Trigger(UITRIGGER_COMMSELECTION_LEVEL, pCombatEnt->GetAsNpc()->GetLevel());
        else
            Trigger(UITRIGGER_COMMSELECTION_LEVEL, -1);

        Trigger(UITRIGGER_COMMSELECTION_HEALTH_VALUE, pCombatEnt->GetHealth());
        Trigger(UITRIGGER_COMMSELECTION_HEALTH_PERCENT, pCombatEnt->GetHealthPercent());
        Trigger(UITRIGGER_COMMSELECTION_MAX_HEALTH, pCombatEnt->GetMaxHealth());

        Trigger(UITRIGGER_COMMSELECTION_MANA_VALUE, pCombatEnt->GetMana());
        Trigger(UITRIGGER_COMMSELECTION_MANA_PERCENT, pCombatEnt->GetManaPercent());
        Trigger(UITRIGGER_COMMSELECTION_MAX_MANA, pCombatEnt->GetMaxMana());

        Trigger(UITRIGGER_COMMSELECTION_ARMOR, pCombatEnt->GetArmor());
        Trigger(UITRIGGER_COMMSELECTION_ARMOR_REDUCTION, pCombatEnt->GetArmorDamageReduction(pCombatEnt->GetArmor()));

        Trigger(UITRIGGER_COMMSELECTION_MODEL, g_ResourceManager.GetPath(pCombatEnt->GetModelHandle()));
        Trigger(UITRIGGER_COMMSELECTION_COMMANDER_PORTRAIT, pCombatEnt->GetCommanderPortraitPath());
    }
    else if (pEntity->IsGadget()) 
    {
        CommSelectionPlayer.Hide();
        CommSelectionNpc.Hide();
        CommSelectionBuilding.Hide();
        if (bShow)
            CommSelectionGadget.Show();
        CommSelectionChest.Hide();

        IGadgetEntity *pGadget(pEntity->GetAsGadget());

        svector vParams;
        if (pGadget->GetTeam() == pLocalClient->GetTeam())
        {
            vParams.push_back(_T("#00CC00CC"));
            vParams.push_back(_T("white"));
        }
        else
        {
            vParams.push_back(_T("#AA0000CC"));
            vParams.push_back(_T("yellow"));
        }

        Trigger(UITRIGGER_COMMSELECTION_COLORS, vParams);
        Trigger(UITRIGGER_COMMSELECTION_NAME, pGadget->GetEntityName());
        Trigger(UITRIGGER_COMMSELECTION_DESCRIPTION, pGadget->GetEntityDescription());
        Trigger(UITRIGGER_COMMSELECTION_ICON, pGadget->GetEntityIconPath());

        Trigger(UITRIGGER_COMMSELECTION_HEALTH_VALUE, pGadget->GetHealth());
        Trigger(UITRIGGER_COMMSELECTION_HEALTH_PERCENT, pGadget->GetHealthPercent());
        Trigger(UITRIGGER_COMMSELECTION_MAX_HEALTH, pGadget->GetMaxHealth());

        Trigger(UITRIGGER_COMMSELECTION_LIFETIME_VALUE, pGadget->GetRemainingLifetime());
        Trigger(UITRIGGER_COMMSELECTION_LIFETIME_PERCENT, pGadget->GetRemainingLifetimePercent());

        Trigger(UITRIGGER_COMMSELECTION_COMMANDER_PORTRAIT, pGadget->GetCommanderPortraitPath());
    }
    else if (pEntity->IsProp())
    {
        IPropEntity *pProp(pEntity->GetAsProp());
        IPropFoundation *pFoundation(pProp->GetAsFoundation());
        if (pFoundation != NULL)
        {
            CommSelectionPlayer.Hide();
            CommSelectionNpc.Hide();
            if (bShow)
                CommSelectionBuilding.Show();
            CommSelectionGadget.Hide();
            CommSelectionChest.Hide();

            svector vParams;
            vParams.push_back(_T("#00CC00CC"));
            vParams.push_back(_T("white"));
            Trigger(UITRIGGER_COMMSELECTION_COLORS, vParams);

            Trigger(UITRIGGER_COMMSELECTION_NAME, pProp->GetEntityName());
            Trigger(UITRIGGER_COMMSELECTION_DESCRIPTION, pProp->GetEntityDescription());
            Trigger(UITRIGGER_COMMSELECTION_ICON, pProp->GetEntityIconPath());

            if (pFoundation->IsMine())
            {
                Trigger(UITRIGGER_COMMSELECTION_GOLD_VALUE, pFoundation->GetRemainingGold());
                Trigger(UITRIGGER_COMMSELECTION_GOLD_PERCENT, pFoundation->GetRemainingGoldPercent());
                Trigger(UITRIGGER_COMMSELECTION_MAX_GOLD, pFoundation->GetTotalGold());
                Trigger(UITRIGGER_COMMSELECTION_EXTRACTION_RATE, pFoundation->GetHarvestRate());
            }
            else
            {
                Trigger(UITRIGGER_COMMSELECTION_GOLD_VALUE, -1);
                Trigger(UITRIGGER_COMMSELECTION_GOLD_PERCENT, -1.0f);
                Trigger(UITRIGGER_COMMSELECTION_MAX_GOLD, -1);
                Trigger(UITRIGGER_COMMSELECTION_EXTRACTION_RATE, -1);
            }

            Trigger(UITRIGGER_COMMSELECTION_UPKEEP_COST, -1);
            Trigger(UITRIGGER_COMMSELECTION_HEALTH_VALUE, -1.0f);
            Trigger(UITRIGGER_COMMSELECTION_HEALTH_PERCENT, -1.0f);
            Trigger(UITRIGGER_COMMSELECTION_MAX_HEALTH, -1.0f);
            Trigger(UITRIGGER_COMMSELECTION_ARMOR, -1.0f);
            Trigger(UITRIGGER_COMMSELECTION_BUILD_PERCENT, 1.0f);

            Trigger(UITRIGGER_COMMSELECTION_COMMANDER_PORTRAIT, pProp->GetCommanderPortraitPath());
        }
    }
    else
    {
        CommSelectionPlayer.Hide();
        CommSelectionNpc.Hide();
        CommSelectionBuilding.Hide();
        CommSelectionGadget.Hide();
    }
}


/*====================
  CGameInterfaceManager::UpdateReplay
  ====================*/
void    CGameInterfaceManager::UpdateReplay()
{
    PROFILE("CGameInterfaceManager::UpdateReplay");

    Trigger(UITRIGGER_REPLAY_TIME, GameClient.GetGameTime() - ReplayManager.GetBeginTime());
    Trigger(UITRIGGER_REPLAY_ENDTIME, ReplayManager.GetEndTime() - ReplayManager.GetBeginTime());
    Trigger(UITRIGGER_REPLAY_FRAME, ReplayManager.GetFrame());
    Trigger(UITRIGGER_REPLAY_ENDFRAME, ReplayManager.GetEndFrame());
    Trigger(UITRIGGER_REPLAY_SPEED, ReplayManager.GetPlaybackSpeed());

    CEntityClientInfo *pClient(GameClient.GetClientInfo(GameClient.GetLocalClientNum()));
    if (pClient == NULL)
        return;

    Trigger(UITRIGGER_REPLAY_NAME, pClient->GetName());
}


/*====================
  CGameInterfaceManager::UpdateOfficer
  ====================*/
void    CGameInterfaceManager::UpdateOfficer()
{
    CEntityClientInfo *pClient(GameClient.GetLocalClient());
    if (pClient == NULL)
    {
        OfficerInfo.Hide();
        return;
    }

    if (pClient->HasFlags(CLIENT_INFO_IS_OFFICER))
        OfficerInfo.Show();
    else
        OfficerInfo.Hide();

    CEntityTeamInfo *pTeam(GameClient.GetTeam(pClient->GetTeam()));
    Trigger(UITRIGGER_OFFICER_SQUAD_NAME, pTeam == NULL ? _T("Unassigned") : pTeam->GetSquadName(pClient->GetSquad()));
}


/*====================
  CGameInterfaceManager::ShowGameTip
  ====================*/
void    CGameInterfaceManager::ShowGameTip(const tstring &sMessage)
{
    Trigger(UITRIGGER_GAME_TIP_VISIBLE, true);
    Trigger(UITRIGGER_GAME_TIP_TEXT, sMessage);
}


/*====================
  CGameInterfaceManager::HideGameTip
  ====================*/
void    CGameInterfaceManager::HideGameTip()
{
    Trigger(UITRIGGER_GAME_TIP_VISIBLE, false);
}


/*====================
  CGameInterfaceManager::ForceUpdate
  ====================*/
void    CGameInterfaceManager::ForceUpdate()
{
    ++m_uiUpdateSequence;

    for (map<const IEntityAllocator *, int>::iterator it(m_mapCanBuild.begin()); it != m_mapCanBuild.end(); ++it)
        it->second = -1;
}


/*--------------------
  MultiSelectHoverEntity
  --------------------*/
UI_VOID_CMD(MultiSelectHoverEntity, 1)
{
    uint uiTarget(AtoI(vArgList[0]->Evaluate()));
    
    IVisualEntity *pEntity(GameClient.GetClientEntityCurrent(uiTarget));
    if (pEntity == NULL || !pEntity->HasAltInfo())
        uiTarget = INVALID_INDEX;

    GameClient.SetHoverEntity(uiTarget);
    return;
}


/*--------------------
  CanBuild
  --------------------*/
FUNCTION(CanBuild)
{
    if (vArgList.empty())
        return _T("false");

    return XtoA(GameClient.CanBuild(vArgList[0], vArgList.size() > 1 ? AtoB(vArgList[1]) : false));
}


/*--------------------
  CanBuild
  --------------------*/
UI_CMD(CanBuild, 0)
{
    if (vArgList.empty())
        return _T("false");

    return XtoA(GameClient.CanBuild(vArgList[0]->Evaluate(), vArgList.size() > 1 ? AtoB(vArgList[1]->Evaluate()) : false));
}
