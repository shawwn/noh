// (C)2006 S2 Games
// c_interfacemanager.h
//
//=============================================================================
#ifndef __C_INTERFACEMANAGER_H__
#define __C_INTERFACEMANAGER_H__

//=============================================================================
// Headers
//=============================================================================
#include "../aba_shared/c_teaminfo.h"

#include "../k2/i_widget.h"
#include "../k2/c_uitrigger.h"
#include "../k2/c_filehttp.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CUITrigger;
class CSmartGameUITrigger;
class CMatchStatRecord;
class CHTTPManager;
class CHTTPRequest;

EXTERN_CVAR_UINT(cg_buildingAttackAlertTime);
EXTERN_CVAR_UINT(cg_endGameInterfaceDelay);
EXTERN_CVAR_BOOL(cg_displayLevelup);
EXTERN_CVAR_BOOL(cg_displayAllies);
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const int MAX_DISPLAY_STATES(10);
const int MAX_DISPLAY_TEAMS(2);
const int MAX_DISPLAY_PLAYERSPERTEAM(5);
const int MAX_DISPLAY_PLAYERS(MAX_DISPLAY_TEAMS * MAX_DISPLAY_PLAYERSPERTEAM);
const int MAX_SELECTED_UNITS(16);
const int MAX_HERO_LIST(90);
const int MAX_SHOPS(12);
const int MAX_SHOP_ITEMS(20);
const uint MAX_RECIPE_COMPONENTS(4);
const int MAX_RECIPE_USEDIN(4);
const int MAX_HERO_ABILITY_INFO(4);
const int MAX_ALLY_HEROES(4);

enum EGameUITrigger
{
    UITRIGGER_HOST_TIME,
    UITRIGGER_CAN_LEAVE,
    UITRIGGER_GAME_PHASE,
    UITRIGGER_LAG,

    UITRIGGER_MAIN_LOGIN_STATUS,
    UITRIGGER_MAIN_UPDATER_STATUS,
    UITRIGGER_MAIN_CHANGE_PASSWORD_STATUS,
    UITRIGGER_MAIN_ACCOUNT_INFO,
    UITRIGGER_MAIN_PLAYER_STATS,
    UITRIGGER_MAIN_LOCAL_PLAYER_STATS,
    UITRIGGER_MAIN_GAMELIST_STATUS,
    UITRIGGER_MAIN_LOCAL_SERVER_AVAILABLE,
    UITRIGGER_MAIN_DEV,
    UITRIGGER_MAIN_PLAYERS_ONLINE,
    UITRIGGER_MAIN_PLAYERS_INGAME,
    
    UITRIGGER_PLAYER_INFO,
    UITRIGGER_PLAYER_GOLD,
    UITRIGGER_PLAYER_CAN_SHOP,
    UITRIGGER_PLAYER_SCORE,
    
    UITRIGGER_SHOP_ACTIVE,
    UITRIGGER_SHOP_TYPE,
    UITRIGGER_SHOP_HEADER,
    UITRIGGER_SHOP_NAME,
    UITRIGGER_SHOP_DESCRIPTION,
    UITRIGGER_SHOP_ICON,
    UITRIGGER_SHOP_KEY,
    UITRIGGER_SHOP_ITEM,
    UITRIGGER_SHOP_ITEM_TOOLTIP,
    UITRIGGER_SHOP_ITEM_TYPE,

    UITRIGGER_RECIPE_ITEM,
    UITRIGGER_RECIPE_ITEM_TYPE,
    UITRIGGER_RECIPE_COMPONENT,
    UITRIGGER_RECIPE_COMPONENT_0_SUB_COMPONENT,
    UITRIGGER_RECIPE_COMPONENT_1_SUB_COMPONENT,
    UITRIGGER_RECIPE_COMPONENT_2_SUB_COMPONENT,
    UITRIGGER_RECIPE_COMPONENT_3_SUB_COMPONENT,
    UITRIGGER_RECIPE_USEDIN,

    UITRIGGER_RECIPE_ITEM_TOOLTIP,
    UITRIGGER_RECIPE_COMPONENT_TOOLTIP,
    UITRIGGER_RECIPE_COMPONENT_0_SUB_TOOLTIP,
    UITRIGGER_RECIPE_COMPONENT_1_SUB_TOOLTIP,
    UITRIGGER_RECIPE_COMPONENT_2_SUB_TOOLTIP,
    UITRIGGER_RECIPE_COMPONENT_3_SUB_TOOLTIP,
    UITRIGGER_RECIPE_USEDIN_TOOLTIP,

    UITRIGGER_RECIPE_HAS_BACK_HISTORY,
    UITRIGGER_RECIPE_HAS_FORWARD_HISTORY,

    UITRIGGER_HERO_INDEX,
    UITRIGGER_HERO_NAME,
    UITRIGGER_HERO_ICON,
    UITRIGGER_HERO_PORTRAIT,
    UITRIGGER_HERO_LEVEL,
    UITRIGGER_HERO_EXPERIENCE,
    UITRIGGER_HERO_HEALTH,
    UITRIGGER_HERO_MANA,
    UITRIGGER_HERO_STAMINA,
    UITRIGGER_HERO_HEALTHREGEN,
    UITRIGGER_HERO_MANAREGEN,
    UITRIGGER_HERO_STAMINAREGEN,
    UITRIGGER_HERO_STATUS,
    UITRIGGER_HERO_RESPAWN,
    UITRIGGER_HERO_BUYBACK,
    UITRIGGER_HERO_BUYBACK_COST,
    UITRIGGER_HERO_INVENTORY_EXISTS,
    UITRIGGER_HERO_INVENTORY_ICON,
    UITRIGGER_HERO_INVENTORY_CAN_ACTIVATE,
    UITRIGGER_HERO_INVENTORY_STATUS,
    UITRIGGER_HERO_INVENTORY_COOLDOWN,
    UITRIGGER_HERO_INVENTORY_CHARGES,
    UITRIGGER_HERO_INVENTORY_DESCRIPTION,
    UITRIGGER_HERO_INVENTORY_HOTKEYS,
    UITRIGGER_HERO_INVENTORY_AURA,
    UITRIGGER_HERO_INVENTORY_STATUS_EFFECT,
    UITRIGGER_HERO_INVENTORY_STATUS_EFFECTB,
    UITRIGGER_HERO_INVENTORY_TRIGGERED_EFFECT,
    UITRIGGER_HERO_INVENTORY_INTERFACE,
    UITRIGGER_HERO_INVENTORY_DURATION,
    UITRIGGER_HERO_INVENTORY_PASSIVE_EFFECT,
    UITRIGGER_HERO_INVENTORY_HAS_TIMER,
    UITRIGGER_HERO_INVENTORY_TIMER,

    UITRIGGER_ALLY_DISPLAY,
    UITRIGGER_ALLY_EXISTS,
    UITRIGGER_ALLY_INDEX,
    UITRIGGER_ALLY_PLAYER_INFO,
    UITRIGGER_ALLY_HERO_INFO,
    UITRIGGER_ALLY_HEALTH,
    UITRIGGER_ALLY_MANA,
    UITRIGGER_ALLY_STAMINA,
    UITRIGGER_ALLY_STATUS,
    UITRIGGER_ALLY_RESPAWN,
    UITRIGGER_ALLY_DAMAGE,
    UITRIGGER_ALLY_ARMOR,
    UITRIGGER_ALLY_MAGIC_ARMOR,
    UITRIGGER_ALLY_MOVE_SPEED,
    UITRIGGER_ALLY_ATTACK_SPEED,
    UITRIGGER_ALLY_STRENGTH,
    UITRIGGER_ALLY_AGILITY,
    UITRIGGER_ALLY_INTELLIGENCE,
    UITRIGGER_ALLY_GOLD,
    UITRIGGER_ALLY_DISCONNECTED,
    UITRIGGER_ALLY_DISCONNECT_TIME,
    UITRIGGER_ALLY_AFK,
    UITRIGGER_ALLY_LOADING_PERCENT,
    UITRIGGER_ALLY_CONTROL_SHARING,
    UITRIGGER_ALLY_NO_HELP,
    UITRIGGER_ALLY_VOICE,
    UITRIGGER_ALLY_POWER,

    UITRIGGER_ALLY_ABILITY_0_INFO,
    UITRIGGER_ALLY_ABILITY_0_COOLDOWN,
    UITRIGGER_ALLY_ABILITY_1_INFO,
    UITRIGGER_ALLY_ABILITY_1_COOLDOWN,
    UITRIGGER_ALLY_ABILITY_2_INFO,
    UITRIGGER_ALLY_ABILITY_2_COOLDOWN,
    UITRIGGER_ALLY_ABILITY_3_INFO,
    UITRIGGER_ALLY_ABILITY_3_COOLDOWN,
    UITRIGGER_ALLY_ABILITY_4_INFO,
    UITRIGGER_ALLY_ABILITY_4_COOLDOWN,

    UITRIGGER_ACTIVE_INDEX,
    UITRIGGER_ACTIVE_NAME,
    UITRIGGER_ACTIVE_ICON,
    UITRIGGER_ACTIVE_PORTRAIT,
    UITRIGGER_ACTIVE_MODEL,
    UITRIGGER_ACTIVE_EFFECT,
    UITRIGGER_ACTIVE_STATUS,
    UITRIGGER_ACTIVE_LEVEL,
    UITRIGGER_ACTIVE_EXPERIENCE,
    UITRIGGER_ACTIVE_DAMAGE,
    UITRIGGER_ACTIVE_ARMOR,
    UITRIGGER_ACTIVE_MAGIC_ARMOR,
    UITRIGGER_ACTIVE_MOVE_SPEED,
    UITRIGGER_ACTIVE_ATTACK_SPEED,
    UITRIGGER_ACTIVE_CAST_SPEED,
    UITRIGGER_ACTIVE_ATTACK_RANGE,
    UITRIGGER_ACTIVE_ATTACK_COOLDOWN,
    UITRIGGER_ACTIVE_STRENGTH,
    UITRIGGER_ACTIVE_AGILITY,
    UITRIGGER_ACTIVE_INTELLIGENCE,
    UITRIGGER_ACTIVE_ATTRIBUTES,
    UITRIGGER_ACTIVE_HEALTH,
    UITRIGGER_ACTIVE_MANA,
    UITRIGGER_ACTIVE_STAMINA,
    UITRIGGER_ACTIVE_HEALTHREGEN,
    UITRIGGER_ACTIVE_MANAREGEN,
    UITRIGGER_ACTIVE_STAMINAREGEN,
    UITRIGGER_ACTIVE_LIFETIME,
    UITRIGGER_ACTIVE_HAS_INVENTORY,
    UITRIGGER_ACTIVE_HAS_ATTRIBUTES,
    UITRIGGER_ACTIVE_PLAYER_INFO,
    UITRIGGER_ACTIVE_ILLUSION,
    UITRIGGER_ACTIVE_POWER,

    UITRIGGER_ACTIVE_ATTACK_INFO,
    UITRIGGER_ACTIVE_DEFENSE_INFO,
    UITRIGGER_ACTIVE_ATTRIBUTE_INFO,

    UITRIGGER_ACTIVE_INVENTORY_EXISTS,
    UITRIGGER_ACTIVE_INVENTORY_ICON,
    UITRIGGER_ACTIVE_INVENTORY_RECIPE,
    UITRIGGER_ACTIVE_INVENTORY_CAN_ACTIVATE,
    UITRIGGER_ACTIVE_INVENTORY_STATUS,
    UITRIGGER_ACTIVE_INVENTORY_STATE,
    UITRIGGER_ACTIVE_INVENTORY_COOLDOWN,
    UITRIGGER_ACTIVE_INVENTORY_CHARGES,
    UITRIGGER_ACTIVE_INVENTORY_DESCRIPTION,
    UITRIGGER_ACTIVE_INVENTORY_HOTKEYS,
    UITRIGGER_ACTIVE_INVENTORY_AURA,
    UITRIGGER_ACTIVE_INVENTORY_STATUS_EFFECT,
    UITRIGGER_ACTIVE_INVENTORY_STATUS_EFFECTB,
    UITRIGGER_ACTIVE_INVENTORY_TRIGGERED_EFFECT,
    UITRIGGER_ACTIVE_INVENTORY_TRIGGERED_EFFECT_DESCRIPTION,
    UITRIGGER_ACTIVE_INVENTORY_INTERFACE,
    UITRIGGER_ACTIVE_INVENTORY_DURATION,
    UITRIGGER_ACTIVE_INVENTORY_DURATION_PERCENT,
    UITRIGGER_ACTIVE_INVENTORY_PASSIVE_EFFECT,
    UITRIGGER_ACTIVE_INVENTORY_DESCRIPTION_A,
    UITRIGGER_ACTIVE_INVENTORY_DESCRIPTION_B,
    UITRIGGER_ACTIVE_INVENTORY_EFFECT_DESCRIPTION,
    UITRIGGER_ACTIVE_INVENTORY_HAS_TIMER,
    UITRIGGER_ACTIVE_INVENTORY_TIMER,
    UITRIGGER_ACTIVE_INVENTORY_ACTIVATE_COST,
    
    UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_STATUS,
    UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_DESCRIPTION,
    UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_CAN_ACTIVATE,
    UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_COOLDOWN,
    UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_AURA,
    UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_STATUS_EFFECT,
    UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_STATUS_EFFECTB,
    UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_TRIGGERED_EFFECT,
    UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_TRIGGERED_EFFECT_DESCRIPTION,
    UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_DESCRIPTION_A,
    UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_DESCRIPTION_B,
    UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_EFFECT_DESCRIPTION,

    UITRIGGER_ATTACK_MODIFIERS,

    UITRIGGER_STASH_EXISTS,
    UITRIGGER_STASH_ICON,
    UITRIGGER_STASH_RECIPE,
    UITRIGGER_STASH_CAN_ACTIVATE,
    UITRIGGER_STASH_STATUS,
    UITRIGGER_STASH_COOLDOWN,
    UITRIGGER_STASH_CHARGES,
    UITRIGGER_STASH_DESCRIPTION,

    UITRIGGER_SELECTED_VISIBLE,
    UITRIGGER_SELECTED_INDEX,
    UITRIGGER_SELECTED_NAME,
    UITRIGGER_SELECTED_ICON,
    UITRIGGER_SELECTED_COLOR,
    UITRIGGER_SELECTED_ACTIVE,
    UITRIGGER_SELECTED_LEVEL,
    UITRIGGER_SELECTED_TYPE,
    UITRIGGER_SELECTED_EXPERIENCE,
    UITRIGGER_SELECTED_DAMAGE,
    UITRIGGER_SELECTED_ARMOR,
    UITRIGGER_SELECTED_MAGIC_ARMOR,
    UITRIGGER_SELECTED_MOVE_SPEED,
    UITRIGGER_SELECTED_ATTACK_SPEED,
    UITRIGGER_SELECTED_CAST_SPEED,
    UITRIGGER_SELECTED_ATTACK_RANGE,
    UITRIGGER_SELECTED_ATTACK_COOLDOWN,
    UITRIGGER_SELECTED_STRENGTH,
    UITRIGGER_SELECTED_AGILITY,
    UITRIGGER_SELECTED_INTELLIGENCE,
    UITRIGGER_SELECTED_ATTRIBUTES,
    UITRIGGER_SELECTED_HEALTH,
    UITRIGGER_SELECTED_MANA,
    UITRIGGER_SELECTED_STAMINA,
    UITRIGGER_SELECTED_HEALTHREGEN,
    UITRIGGER_SELECTED_MANAREGEN,
    UITRIGGER_SELECTED_STAMINAREGEN,
    UITRIGGER_SELECTED_LIFETIME,
    UITRIGGER_SELECTED_HAS_INVENTORY,
    UITRIGGER_SELECTED_HAS_ATTRIBUTES,
    UITRIGGER_SELECTED_PLAYER_INFO,
    UITRIGGER_SELECTED_ILLUSION,
    UITRIGGER_SELECTED_POWER,

    UITRIGGER_SELECTED_ATTACK_INFO,
    UITRIGGER_SELECTED_DEFENSE_INFO,
    UITRIGGER_SELECTED_ATTRIBUTE_INFO,

    UITRIGGER_SELECTED_INVENTORY_EXISTS,
    UITRIGGER_SELECTED_INVENTORY_ICON,
    UITRIGGER_SELECTED_INVENTORY_RECIPE,
    UITRIGGER_SELECTED_INVENTORY_CAN_ACTIVATE,
    UITRIGGER_SELECTED_INVENTORY_STATUS,
    UITRIGGER_SELECTED_INVENTORY_COOLDOWN,
    UITRIGGER_SELECTED_INVENTORY_CHARGES,
    UITRIGGER_SELECTED_INVENTORY_DESCRIPTION,
    UITRIGGER_SELECTED_INVENTORY_AURA,
    UITRIGGER_SELECTED_INVENTORY_STATUS_EFFECT,
    UITRIGGER_SELECTED_INVENTORY_STATUS_EFFECTB,
    UITRIGGER_SELECTED_INVENTORY_TRIGGERED_EFFECT,
    UITRIGGER_SELECTED_INVENTORY_PASSIVE_EFFECT,
    UITRIGGER_SELECTED_INVENTORY_INTERFACE,
    UITRIGGER_SELECTED_INVENTORY_STATE,
    UITRIGGER_SELECTED_INVENTORY_HAS_TIMER,
    UITRIGGER_SELECTED_INVENTORY_TIMER,
    UITRIGGER_SELECTED_INVENTORY_DURATION,
    UITRIGGER_SELECTED_INVENTORY_DURATION_PERCENT,
    UITRIGGER_SELECTED_INVENTORY_ACTIVATE_COST,

    UITRIGGER_ALT_INFO_0_NAME,
    UITRIGGER_ALT_INFO_0_PLAYER,
    UITRIGGER_ALT_INFO_0_TEAM,
    UITRIGGER_ALT_INFO_0_COLOR,
    UITRIGGER_ALT_INFO_0_HAS_HEALTH,
    UITRIGGER_ALT_INFO_0_HEALTH_PERCENT,
    UITRIGGER_ALT_INFO_0_HEALTH_LERP,
    UITRIGGER_ALT_INFO_0_HAS_MANA,
    UITRIGGER_ALT_INFO_0_MANA_PERCENT,
    UITRIGGER_ALT_INFO_0_HAS_STAMINA,
    UITRIGGER_ALT_INFO_0_STAMINA_PERCENT,
    UITRIGGER_ALT_INFO_0_LEVEL,

    UITRIGGER_ALT_INFO_1_NAME,
    UITRIGGER_ALT_INFO_1_PLAYER,
    UITRIGGER_ALT_INFO_1_TEAM,
    UITRIGGER_ALT_INFO_1_COLOR,
    UITRIGGER_ALT_INFO_1_HAS_HEALTH,
    UITRIGGER_ALT_INFO_1_HEALTH_PERCENT,
    UITRIGGER_ALT_INFO_1_HEALTH_LERP,
    UITRIGGER_ALT_INFO_1_HAS_MANA,
    UITRIGGER_ALT_INFO_1_MANA_PERCENT,
    UITRIGGER_ALT_INFO_1_HAS_STAMINA,
    UITRIGGER_ALT_INFO_1_STAMINA_PERCENT,
    UITRIGGER_ALT_INFO_1_LEVEL,

    UITRIGGER_ALT_INFO_2_NAME,
    UITRIGGER_ALT_INFO_2_PLAYER,
    UITRIGGER_ALT_INFO_2_TEAM,
    UITRIGGER_ALT_INFO_2_COLOR,
    UITRIGGER_ALT_INFO_2_HAS_HEALTH,
    UITRIGGER_ALT_INFO_2_HEALTH_PERCENT,
    UITRIGGER_ALT_INFO_2_HEALTH_LERP,
    UITRIGGER_ALT_INFO_2_HAS_MANA,
    UITRIGGER_ALT_INFO_2_MANA_PERCENT,
    UITRIGGER_ALT_INFO_2_HAS_STAMINA,
    UITRIGGER_ALT_INFO_2_STAMINA_PERCENT,
    UITRIGGER_ALT_INFO_2_LEVEL,

    UITRIGGER_ALT_INFO_3_NAME,
    UITRIGGER_ALT_INFO_3_PLAYER,
    UITRIGGER_ALT_INFO_3_TEAM,
    UITRIGGER_ALT_INFO_3_COLOR,
    UITRIGGER_ALT_INFO_3_HAS_HEALTH,
    UITRIGGER_ALT_INFO_3_HEALTH_PERCENT,
    UITRIGGER_ALT_INFO_3_HEALTH_LERP,
    UITRIGGER_ALT_INFO_3_HAS_MANA,
    UITRIGGER_ALT_INFO_3_MANA_PERCENT,
    UITRIGGER_ALT_INFO_3_HAS_STAMINA,
    UITRIGGER_ALT_INFO_3_STAMINA_PERCENT,
    UITRIGGER_ALT_INFO_3_LEVEL,

    UITRIGGER_SCOREBOARD_PLAYER,
    UITRIGGER_SCOREBOARD_PLAYER_RESPAWN,
    UITRIGGER_SCOREBOARD_TEAM,

    UITRIGGER_ITEM_CURSOR_VISIBLE,
    UITRIGGER_ITEM_CURSOR_ICON,
    UITRIGGER_ITEM_CURSOR_POSITION,

    UITRIGGER_TOOL_TARGETING_ENTITY,

    UITRIGGER_BASE_HEALTH,
    UITRIGGER_BASE_HEALTH_VISIBLE,

    UITRIGGER_TIME_OF_DAY,
    UITRIGGER_DAYTIME,
    UITRIGGER_MATCH_TIME,
    UITRIGGER_MATCH_ID,

    UITRIGGER_MENU_PLAYER_INFO,

    UITRIGGER_EVENT_FIRST_KILL,
    UITRIGGER_EVENT_MULTI_KILL,
    UITRIGGER_EVENT_KILL_STREAK,
    UITRIGGER_EVENT_TEAM_WIPE,
    UITRIGGER_EVENT_TOWER_DENY,
    UITRIGGER_EVENT_COURIER_KILL,
    UITRIGGER_EVENT_MEGA_CREEPS,
    UITRIGGER_EVENT_NEMESIS,
    UITRIGGER_EVENT_SMACKDOWN,
    UITRIGGER_EVENT_HUMILIATION,
    UITRIGGER_EVENT_PAYBACK,
    UITRIGGER_EVENT_RAGE_QUIT,
    UITRIGGER_EVENT_FEEDER,
    UITRIGGER_EVENT_VICTORY,
    UITRIGGER_EVENT_DEFEAT,

    UITRIGGER_LOADING_IMAGE,

    UITRIGGER_LOBBY_STATUS,
    UITRIGGER_LOBBY_TEAM_INFO,
    UITRIGGER_LOBBY_SPECTATORS,
    UITRIGGER_LOBBY_PLAYER_INFO,
    UITRIGGER_LOBBY_BUDDY,
    UITRIGGER_LOBBY_VOICE,
    UITRIGGER_LOBBY_GAME_INFO,
    UITRIGGER_LOBBY_REFEREE,
    UITRIGGER_LOBBY_PLAYER_LIST,
    UITRIGGER_LOBBY_PLAYER_LIST_SIZE,
    UITRIGGER_LOBBY_COUNTDOWN,
    UITRIGGER_LOBBY_PRIVATE,
    UITRIGGER_LOBBY_NO_STATS,
    UITRIGGER_LOBBY_NO_LEAVERS,
    
    UITRIGGER_MATCHMAKER_PLAYER_INFO,
    UITRIGGER_MATCHMAKER_STATUS,
    UITRIGGER_MATCHMAKER_PROGRESS,
    UITRIGGER_MATCHMAKER_TIMEOUT,

    UITRIGGER_HERO_SELECT_HERO_LIST,
    UITRIGGER_HERO_SELECT_PLAYER_INFO,
    UITRIGGER_HERO_SELECT_HERO_INFO,
    UITRIGGER_HERO_SELECT_HERO_ABILITY_INFO,
    UITRIGGER_HERO_SELECT_TIMER,
    UITRIGGER_HERO_SELECT_INFO,
    UITRIGGER_HERO_SELECT_HAS_EXTRA_TIME,
    UITRIGGER_HERO_SELECT_EXTRA_TIME,
    UITRIGGER_HERO_SELECT_USING_EXTRA_TIME,

    UITRIGGER_ENDGAME,
    UITRIGGER_ENDGAME_INTERFACE_DISPLAY,
    UITRIGGER_ENDGAME_MATCH_INFO,
    UITRIGGER_ENDGAME_PLAYER_STATS,
    UITRIGGER_ENDGAME_PLAYER_DETAIL_STATS,
    UITRIGGER_ENDGAME_PLAYER_DETAIL_INVENTORY,
    UITRIGGER_ENDGAME_PLAYER_DETAIL_ABILITY_HISTORY,
    UITRIGGER_ENDGAME_PLAYER_DETAIL_KILLS,
    UITRIGGER_ENDGAME_PLAYER_DETAIL_ASSISTS,
    UITRIGGER_ENDGAME_PLAYER_DETAIL_DEATHS,
    UITRIGGER_ENDGAME_TEAM_STATS,
    UITRIGGER_ENDGAME_TIMER,
    
    UITRIGGER_CONNECTION_STATUS,

    UITRIGGER_FOLLOW_STATUS,

    UITRIGGER_SCOREBOARD_CHANGE,

    UITRIGGER_VOTE_TYPE,
    UITRIGGER_VOTE_TIME,
    UITRIGGER_VOTE_SHOW,
    UITRIGGER_VOTE_PROGRESS,
    UITRIGGER_VOTED,
    UITRIGGER_VOTE_PERMISSIONS,
    UITRIGGER_VOTE_KICK_PERMISSIONS,
    
    UITRIGGER_SPECTATOR_TEAMINFO,
    UITRIGGER_SPECTATOR_PLAYER,
    UITRIGGER_SPECTATOR_PLAYER_HEALTH_PERCENT,

    UITRIGGER_SPECTATOR_HERO_EXISTS,
    UITRIGGER_SPECTATOR_HERO_INDEX,
    UITRIGGER_SPECTATOR_HERO_PLAYER_INFO,
    UITRIGGER_SPECTATOR_HERO_HERO_INFO,
    UITRIGGER_SPECTATOR_HERO_HEALTH,
    UITRIGGER_SPECTATOR_HERO_MANA,
    UITRIGGER_SPECTATOR_HERO_STATUS,
    UITRIGGER_SPECTATOR_HERO_RESPAWN,
    UITRIGGER_SPECTATOR_HERO_DAMAGE,
    UITRIGGER_SPECTATOR_HERO_ARMOR,
    UITRIGGER_SPECTATOR_HERO_MAGIC_ARMOR,
    UITRIGGER_SPECTATOR_HERO_MOVE_SPEED,
    UITRIGGER_SPECTATOR_HERO_ATTACK_SPEED,
    UITRIGGER_SPECTATOR_HERO_STRENGTH,
    UITRIGGER_SPECTATOR_HERO_AGILITY,
    UITRIGGER_SPECTATOR_HERO_INTELLIGENCE,
    UITRIGGER_SPECTATOR_HERO_GOLD,
    UITRIGGER_SPECTATOR_HERO_DISCONNECTED,
    UITRIGGER_SPECTATOR_HERO_DISCONNECT_TIME,
    UITRIGGER_SPECTATOR_HERO_LOADING_PERCENT,
    UITRIGGER_SPECTATOR_HERO_POWER,

    UITRIGGER_SPECTATOR_HERO_ABILITY_0_INFO,
    UITRIGGER_SPECTATOR_HERO_ABILITY_0_COOLDOWN,
    UITRIGGER_SPECTATOR_HERO_ABILITY_1_INFO,
    UITRIGGER_SPECTATOR_HERO_ABILITY_1_COOLDOWN,
    UITRIGGER_SPECTATOR_HERO_ABILITY_2_INFO,
    UITRIGGER_SPECTATOR_HERO_ABILITY_2_COOLDOWN,
    UITRIGGER_SPECTATOR_HERO_ABILITY_3_INFO,
    UITRIGGER_SPECTATOR_HERO_ABILITY_3_COOLDOWN,
    UITRIGGER_SPECTATOR_HERO_ABILITY_4_INFO,
    UITRIGGER_SPECTATOR_HERO_ABILITY_4_COOLDOWN,

    UITRIGGER_KEY_MODIFIER1,
    UITRIGGER_KEY_MODIFIER2,

    // OLD AND BUSTED
    UITRIGGER_BUILDING_ATTACK_ALERT,

    UITRIGGER_TEAM,

    UITRIGGER_GAME_MATCH_ID,

    UITRIGGER_KILL_NOTIFICATION,

    UITRIGGER_VOICECHAT_TALKING,

    UITRIGGER_REPLAY_NAME,
    UITRIGGER_REPLAY_TIME,
    UITRIGGER_REPLAY_ENDTIME,
    UITRIGGER_REPLAY_FRAME,
    UITRIGGER_REPLAY_ENDFRAME,
    UITRIGGER_REPLAY_SPEED,
    UITRIGGER_REPLAY_PLAYING,
    UITRIGGER_REPLAY_PAUSED,

    UITRIGGER_REPLAY_INFO_GAME,
    UITRIGGER_REPLAY_INFO_PLAYER,

    UITRIGGER_REPLAY_URL_STATUS,

    UITRIGGER_PREVIEW_MAP_NAME,
    UITRIGGER_PREVIEW_MAP_SIZE,

    UITRIGGER_COMMAND_ENABLED_MOVE,
    UITRIGGER_COMMAND_ENABLED_ATTACK,
    UITRIGGER_COMMAND_ENABLED_STOP,
    UITRIGGER_COMMAND_ENABLED_HOLD,
    UITRIGGER_COMMAND_ENABLED_PATROL,

    UITRIGGER_MATCH_INFO_SUMMARY,
    UITRIGGER_MATCH_INFO_PLAYER,
    UITRIGGER_MATCH_INFO_ENTRY,
    UITRIGGER_MATCH_INFO_ENTRY_FINISHED,

    UITRIGGER_TOURNAMENT_INFO,
    UITRIGGER_TOURNAMENTS_FOR_ACCOUNT,

    UITRIGGER_TOURNAMENT_INFO_RETURN,
    UITRIGGER_TOURNAMENT_INFO_TEAM1_PLAYERS_RETURN,
    UITRIGGER_TOURNAMENT_INFO_TEAM2_PLAYERS_RETURN,
    UITRIGGER_TOURNAMENTS_FOR_ACCOUNT_RETURN,

    UITRIGGER_ENTITY_DEFINITIONS_LOADED,
    UITRIGGER_ENTITY_DEFINITIONS_PROGRESS,

    UITRIGGER_COMPENDIUM_CLEAR_INFO,
    UITRIGGER_COMPENDIUM_HERO_INFO,
    UITRIGGER_COMPENDIUM_DETAILED_HERO_INFO,

    UITRIGGER_SYSTEM_DATE,
    UITRIGGER_SYSTEM_WEEKDAY,
    UITRIGGER_SYSTEM_TIME,

    NUM_UITRIGGERS
};

typedef vector<CSmartGameUITrigger*>    SmartUITriggerVector;
typedef SmartUITriggerVector::iterator  SmartUITriggerVector_it;
//=============================================================================

//=============================================================================
// CBuildText
//=============================================================================
class CBuildText
{
private:
    uint    m_uiData;

public:
    CBuildText(uint uiString, uint uiIndex) :
    m_uiData((uiString << 8) + uiIndex) {}

    uint    GetString() const       { return m_uiData >> 8; }
    uint    GetIndex() const        { return m_uiData & 0xff; }

    bool operator==(const CBuildText &b) const  { return m_uiData == b.m_uiData; }
};
//=============================================================================

//=============================================================================
// CBuildMultiLevelText
//=============================================================================
class CBuildMultiLevelText
{
private:
    uint    m_uiData;

public:
    CBuildMultiLevelText(uint uiString, uint uiMarkIndex, uint uiMaxIndex) :
    m_uiData((uiString << 8) + (uiMaxIndex << 4) + uiMarkIndex) {}

    uint    GetString() const       { return m_uiData >> 8; }
    uint    GetMarkIndex() const    { return m_uiData & 0xf; }
    uint    GetMaxIndex() const     { return (m_uiData >> 4) & 0xf; }

    bool operator==(const CBuildMultiLevelText &b) const    { return m_uiData == b.m_uiData; }
};
//=============================================================================

//=============================================================================
// CSmartGameUITrigger
//=============================================================================
class CSmartGameUITrigger
{
private:
    uint                m_uiCount;
    bool                m_bDumb;

    vector<CUITrigger*>     m_vTriggers;
    vector<bool>            m_vTriggerOwner;
    vector<bool>            m_vbValue;
    vector<ushort>          m_vunValue;
    uivector                m_vuiValue;
    fvector                 m_vfValue;
    tsvector                m_vsValue;
    vector<CBuildText>      m_vbtValue;
    vector<CBuildMultiLevelText>    m_vmltValue;
    vector<tsvector>        m_vvValue;
    
    uivector            m_vUpdateSequence;

    CSmartGameUITrigger();

public:
    ~CSmartGameUITrigger();
    CSmartGameUITrigger(const tstring &sName, uint uiCount);

    void    MakeDumb()  { m_bDumb = true; }

    void    Trigger(bool bValue, uint uiIndex, uint uiUpdateSequence);
    void    Trigger(short nValue, uint uiIndex, uint uiUpdateSequence);
    void    Trigger(ushort unValue, uint uiIndex, uint uiUpdateSequence);
    void    Trigger(int uiValue, uint uiIndex, uint uiUpdateSequence);
    void    Trigger(uint uiValue, uint uiIndex, uint uiUpdateSequence);
    void    Trigger(float fValue, uint uiIndex, uint uiUpdateSequence);
    void    Trigger(const tstring &sValue, uint uiIndex, uint uiUpdateSequence);
    void    Trigger(const CBuildText &cValue, uint uiIndex, uint uiUpdateSequence);
    void    Trigger(const CBuildMultiLevelText &cValue, uint uiIndex, uint uiUpdateSequence);
    void    Trigger(const TCHAR *szValue, uint uiIndex, uint uiUpdateSequence)      { Trigger(tstring(szValue), uiIndex, uiUpdateSequence); }
    void    Execute(const tstring &sScript, uint uiIndex);
    void    Trigger(const tsvector &vParams, uint uiIndex, uint uiUpdateSequence);
};
//=============================================================================

//=============================================================================
// CGameInterfaceManager
//=============================================================================
class CGameInterfaceManager
{
private:
    enum EHTTPRequestType
    {
        GET_TOURNAMENT_INFO,
        GET_TOURNAMENTS_FOR_ACCOUNT
    };

    static map<ResHandle, uint> s_mapInterfaceReferenceCount;

    bool            m_bCursorHidden;

    SmartUITriggerVector    m_vTriggers;

    tstring         m_sMainInterface;

    uint            m_uiUpdateSequence;
    uint            m_uiLastUpdateSequence;

    bool            m_bDisplayShop;
    bool            m_bLockShop;

    uint            m_uiScoreState;

    // Cached values
    EGamePhase      m_eGamePhase;
    EGameInterface  m_eCurrentInterface;
    tstring         m_sReplayPreview;
    CXMLNode        m_cGameInfoProperties;
    CXMLNode        m_cPlayerInfoProperties[MAX_DISPLAY_PLAYERS];
    bool            m_bIsLoggedIn;

    uint            m_uiLastBuildingAttackAlertTime;
    uint            m_uiLastUpdateTime;

    tstring         m_sPreviewMapName;
    int             m_iPreviewMapSize;

    tsvector        m_vLastGameStatsSummary;
    tsvector        m_vLastGameStatsPlayers[MAX_DISPLAY_TEAMS][MAX_DISPLAY_PLAYERSPERTEAM];
    tsvector        m_vCurrentGameStatsSummary;
    vector<tsvector>    m_vCurrentGameStatsPlayers[MAX_DISPLAY_TEAMS];

    tsvector        m_vSavedPlayer[MAX_DISPLAY_PLAYERS];

    bool            m_bEntitiesLoaded;
    tsvector        m_vLoadQueue;
    uint            m_uiLoadPos;

    tstring         m_sTestReplayURL;
    int             m_iReplayURLTesting;
    bool            m_bReplayURLValid;
    CFileHTTP       m_fileTestReplayURL;
    uint            m_uiTestReplayURLSize;
    
    int             m_iRequestedStatsAccountID;
    CHTTPRequest*   m_pStatsRequest;
    CHTTPRequest*   m_pMatchInfoRequest;
    CHTTPRequest*   m_pTournamentRequest;
    CHTTPRequest*   m_pRecentMatchesRequest;

    uint            m_uiPrevPhase;
    uint            m_uiPrevPhaseTime;

    void    UpdateLogin();
    void    UpdateChangePassword();
    void    UpdateAccountInfo();
    void    UpdateGameOver();
    void    UpdateGameMenu();
    void    UpdateActiveInventory(IUnitEntity *pUnit, int iStartSlot, int iEndSlot);
    void    UpdateActiveAttackModifiers(IUnitEntity *pUnit);
    void    UpdateLevelUp(IUnitEntity *pUnit, int iStartSlot, int iEndSlot);
    void    UpdateSelectedInventory(IUnitEntity *pUnit, int iStartSlot, int iEndSlot);
    void    UpdateLobby();
    void    UpdateHeroSelect();
    void    UpdateScores();
    void    UpdatePlayer();
    void    UpdateHero();
    void    UpdateHeroInventory(IUnitEntity *pUnit, int iStartSlot, int iEndSlot);
    void    UpdateAllies();
    void    UpdateAllyAbility(IUnitEntity *pUnit, int iSlot, EGameUITrigger eTrigger, uint uiDisplaySlot);
    void    UpdateActiveUnit(IUnitEntity *pUnit);
    void    UpdateSelectedUnit(IUnitEntity *pUnit, uint uiIndex = 0);
    void    UpdateSelectedUnits();
    void    UpdateVoiceChat();
    void    UpdateReplayInfo();
    void    UpdateReplay();
    void    UpdateCursor();
    void    UpdateItemCursor();
    void    UpdateShop();
    void    UpdateShopRecipe(const tstring &sRecipe, IUnitEntity *pControlUnit, tsvector &vItem, uint uiSubIndex);
    void    UpdateShopItem(const tstring &sItem, bool bAccess, bool bLocal, uint uiStockRemaining, uint uiCooldownRemaining, bool bPurchaseRecipe, bool bOwned, bool bComponent, bool bCarry, bool bUsedIn, ushort unShop, int iSlot, tsvector &vItem);
    void    UpdateShopItemTooltip(const tstring &sItem, IUnitEntity *pControlUnit, uint uiTrigger, uint uiSlot, bool bPurchaseRecipe, bool bOwned, bool bCarry, bool bAccess, bool bLocal);
    void    UpdateStash(IUnitEntity *pUnit, IUnitEntity *pControlUnit);
    void    UpdateVote();
    void    UpdateCommander();
    void    UpdateMatchMaker();

    void    UpdateSpectatorTeams();
    void    UpdateSpectatorTeam(uint uiTeam, uint uiIndex);

    void    UpdateSpectatorPlayers();
    void    UpdateSpectatorHeroes();
    void    UpdateSpectatorSelectedUnits();
    void    UpdateSpectatorVoiceChat();

    bool    IsFirstBanButtonVisible() const;

public:
    ~CGameInterfaceManager();
    CGameInterfaceManager();

    void    SetMainInterface(const tstring &sName)  { m_sMainInterface = sName; ForceUpdate(); }
    
    void    ToggleShopInterface();
    bool    IsShopVisible() const   { return m_bDisplayShop; }
    void    SetShopVisible(bool b, bool bForce = false);
    void    SetShopLock(bool bLocked)   { m_bLockShop = bLocked; }
    bool    GetShopLock() const         { return m_bLockShop; }

    void    ToggleLevelupInterface()    { cg_displayLevelup.Toggle(); }
    bool    IsLevelupVisible() const    { return cg_displayLevelup; }
    void    SetLevelupVisible(bool b)   { cg_displayLevelup = b; }

    void    ToggleAlliesInterface()     { cg_displayAllies.Toggle(); }
    bool    IsAlliesVisible() const     { return cg_displayAllies; }
    void    SetAlliesVisible(bool b)    { cg_displayAllies = b; }

    void    SetScoreState(uint uiState) { m_uiScoreState = uiState; }
    uint    GetScoreState()             { return m_uiScoreState; }
    
    void        LoadMainInterfaces();
    void        LoadGameInterfaces();
    ResHandle   LoadGameInterface(const tstring &sName);
    void        ForceUpdate();
    bool        IsFullUpdate()          { return m_uiLastUpdateSequence != m_uiUpdateSequence; }

    // Trigger management
    void    AddTrigger(uint uiTriggerID, const tstring &sName, uint uiCount = 1);
    void    AddDumbTrigger(uint uiTriggerID, const tstring &sName, uint uiCount = 1);

    void    Execute(uint uiTriggerID, const tstring &sScript, uint uiIndex = 0)
    {
        PROFILE("CGameInterfaceManager::Execute");
        assert(uiTriggerID < NUM_UITRIGGERS);
        assert(m_vTriggers[uiTriggerID] != NULL);
        m_vTriggers[uiTriggerID]->Execute(sScript, uiIndex);
    }

    template<class T>
    void    Trigger(uint uiTriggerID, T _Param, uint uiIndex = 0)
    {
        PROFILE("CGameInterfaceManager::Trigger");
        assert(uiTriggerID < NUM_UITRIGGERS);
        assert(m_vTriggers[uiTriggerID] != NULL);
        m_vTriggers[uiTriggerID]->Trigger(_Param, uiIndex, m_uiUpdateSequence);
    }

    void    Trigger(uint uiTriggerID, const tstring &sParam, uint uiIndex = 0)
    {
        PROFILE("CGameInterfaceManager::Trigger");
        assert(uiTriggerID < NUM_UITRIGGERS);
        assert(m_vTriggers[uiTriggerID] != NULL);
        m_vTriggers[uiTriggerID]->Trigger(sParam, uiIndex, m_uiUpdateSequence);
    }

    void    Trigger(uint uiTriggerID, const tsvector &vParams, uint uiIndex = 0)
    {
        PROFILE("CGameInterfaceManager::Trigger");
        assert(uiTriggerID < NUM_UITRIGGERS);
        assert(m_vTriggers[uiTriggerID] != NULL);
        m_vTriggers[uiTriggerID]->Trigger(vParams, uiIndex, m_uiUpdateSequence);
    }

    // Update functions
    void    SetPersistantStat(int iStat, int iValue);

    void    Update();
    
    void    BuildingAttackAlert(const tstring &sName);

    void    RequestPlayerStats(int iAccountID);
    void    RequestPlayerStats(const tstring &sName);
    void    RequestMatchInfo(uint uiMatchID);
    void    RequestTournamentInfo(uint uiTournamentID);
    void    RequestTournamentsForAccount(uint uiAccountID);
    void    RequestRecentMatches();
    void    ClearMatchInfo();

    void    ProcessStatsRequest();
    void    ProcessMatchInfoRequest();
    void    ProcessTournamentRequest();
    void    ProcessRecentMatchesRequest();

    void    SortStatsAsc(int iParam, int iTeam);
    void    SortStatsDesc(int iParam, int iTeam);
    void    SortStatsByValueAsc(int iParam, int iTeam);
    void    SortStatsByValueDesc(int iParam, int iTeam);

    void    SetCursorHidden(bool bHidden)       { m_bCursorHidden = bHidden; }

    void    SetReplayInfo(const tstring &sReplay);
    void    SetReplayGameInfo(const CXMLNode &cNode);
    void    SetReplayPlayerInfo(const CXMLNode &cNode);
    void    SetPreviewMap(const tstring &sMap);

    void    StoreEndGameStats();
    void    StoreEndGamePlayerStats(CPlayer *pPlayer);
    void    StoreMatchLength(uint uiTime)       { if (m_vLastGameStatsSummary.size() > 5) m_vLastGameStatsSummary[5] = XtoA(uiTime); }
    void    ClearEndGameStats();
    
    void    SaveSpectatorPlayers();

    void    RefreshReplayList();

    void    RegisterEntityDefinitions();

    void    UpdateHeroCompendium();
    void    ShowHeroCompendiumInfo(const tstring &sHero);

    bool    IsReplayCompatible();
    tstring GetReplayVersion();

    static void     BuildText(const tstring &sIn, uint uiIndex, tstring &sOut);
    static void     BuildMultiLevelText(const tstring &sIn, uint uiMarkIndex, uint uiMaxIndex, tstring &sOut);
    static void     BuildBonusesString(ISlaveEntity *pSlave, tstring &sStr, int &iLines);
    static void     BuildBonusesString(CStateDefinition *pDefinition, uint uiLevel, uivector &vModifierKeys, tstring &sStr, int &iLines);
    static void     BuildMultiLevelBonusesString(ISlaveEntity *pSlave, tstring &sStr, int &iLines);
    static void     BuildMultiLevelBonusesString(ISlaveDefinition *pDefinition, uint uiLevel, uint uiMaxLevel, uivector &vModifierKeys, tstring &sStr, int &iLines);
};
//=============================================================================

#endif //__C_INTERFACEMANAGER_H__
