// (C)2008 S2 Games
// game_shared_types.h
//
//=============================================================================
#ifndef __HON_SHARED_TYPES_H__
#define __HON_SHARED_TYPES_H__

//=============================================================================
// Definitions
//=============================================================================
enum EGamePhase
{
    GAME_PHASE_INVALID = -1,

    GAME_PHASE_IDLE = 0,
    GAME_PHASE_WAITING_FOR_PLAYERS,
    GAME_PHASE_HERO_BAN,
    GAME_PHASE_HERO_SELECT,
    GAME_PHASE_HERO_LOADING,
    GAME_PHASE_PRE_MATCH,
    GAME_PHASE_ACTIVE,
    GAME_PHASE_ENDED,

    NUM_GAME_PHASES
};

enum EGameMode
{
    GAME_MODE_NORMAL,
    GAME_MODE_RANDOM_DRAFT,
    GAME_MODE_SINGLE_DRAFT,
    GAME_MODE_DEATHMATCH,
    GAME_MODE_BANNING_DRAFT,
    GAME_MODE_CAPTAINS_DRAFT,
    GAME_MODE_CAPTAINS_MODE,
    GAME_MODE_BANNING_PICK,

    NUM_GAME_MODES
};

// Hero list
const byte HERO_LIST_NOT_AVAILABLE  (0);
const byte HERO_LIST_AVAILABLE_ALL  (-1);
const byte HERO_LIST_BANNED         (-2);
const byte HERO_LIST_PICKED         (-3);
const byte HERO_LIST_UNKNOWN        (-4);

typedef pair<ushort, byte>          HeroListEntry;
typedef vector<HeroListEntry>       HeroList;
typedef HeroList::iterator          HeroList_it;
typedef HeroList::const_iterator    HeroList_cit;

enum EGameResourceType
{
    RES_ENTITY_DEF = NUM_RESOURCE_TYPES,
    RES_GAME_MECHANICS
};
static const int NUM_GAME_RESOURCE_TYPES(RES_GAME_MECHANICS + 1);

enum EAttribute
{
    ATTRIBUTE_STRENGTH,
    ATTRIBUTE_AGILITY,
    ATTRIBUTE_INTELLIGENCE,
};
static const EAttribute ATTRIBUTE_INVALID(EAttribute(ATTRIBUTE_INTELLIGENCE + 1));

enum ETargetSelection
{
    TARGET_SELECT_CLOSEST = 1,
    TARGET_SELECT_FURTHEST,
    TARGET_SELECT_RANDOM,
    TARGET_SELECT_RANDOM_POSITION,
    TARGET_SELECT_RANDOM_ANGLE_DISTANCE,
    TARGET_SELECT_ALL
};
static const ETargetSelection TARGET_SELECT_NONE(ETargetSelection(0));

enum ESuperType
{
    SUPERTYPE_INVALID,

    SUPERTYPE_ATTACK,
    SUPERTYPE_SPELL
};

enum EEntityToolAction
{
    TOOL_ACTION_PASSIVE = 1,
    TOOL_ACTION_TOGGLE,
    TOOL_ACTION_NO_TARGET,
    TOOL_ACTION_TARGET_POSITION,
    TOOL_ACTION_TARGET_ENTITY,
    TOOL_ACTION_GLOBAL,
    TOOL_ACTION_TARGET_SELF,
    TOOL_ACTION_FACING,
    TOOL_ACTION_SELF_POSITION,
    TOOL_ACTION_ATTACK,
    TOOL_ACTION_ATTACK_TOGGLE,
    TOOL_ACTION_TARGET_DUAL,
    TOOL_ACTION_TARGET_DUAL_POSITION,
    TOOL_ACTION_TARGET_VECTOR,
    TOOL_ACTION_TARGET_CURSOR,
};
static const EEntityToolAction TOOL_ACTION_INVALID(EEntityToolAction(0));


enum EActionTarget
{
    ACTION_TARGET_INVALID,

    ACTION_TARGET_SOURCE_ENTITY,
    ACTION_TARGET_SOURCE_POSITION,
    ACTION_TARGET_SOURCE_TARGET_OFFSET,
    ACTION_TARGET_SOURCE_ATTACK_OFFSET,
    ACTION_TARGET_SOURCE_OWNER_ENTITY,
    ACTION_TARGET_SOURCE_OWNER_POSITION,
    ACTION_TARGET_TARGET_ENTITY,
    ACTION_TARGET_TARGET_POSITION,
    ACTION_TARGET_TARGET_TARGET_OFFSET,
    ACTION_TARGET_TARGET_ATTACK_OFFSET,
    ACTION_TARGET_TARGET_OWNER_ENTITY,
    ACTION_TARGET_TARGET_OWNER_POSITION,
    ACTION_TARGET_INFLICTOR_ENTITY,
    ACTION_TARGET_INFLICTOR_POSITION,
    ACTION_TARGET_INFLICTOR_TARGET_OFFSET,
    ACTION_TARGET_INFLICTOR_OWNER_ENTITY,
    ACTION_TARGET_INFLICTOR_OWNER_POSITION,
    ACTION_TARGET_PROXY_ENTITY,
    ACTION_TARGET_PROXY_POSITION,
    ACTION_TARGET_STACK_ENTITY,
    ACTION_TARGET_STACK_POSITION,
    ACTION_TARGET_THIS_ENTITY,
    ACTION_TARGET_THIS_POSITION,
    ACTION_TARGET_THIS_TARGET_OFFSET,
    ACTION_TARGET_THIS_ATTACK_OFFSET,
    ACTION_TARGET_THIS_OWNER_ENTITY,
    ACTION_TARGET_THIS_OWNER_POSITION,
    ACTION_TARGET_THIS_INFLICTOR_ENTITY,
    ACTION_TARGET_THIS_INFLICTOR_POSITION,
    ACTION_TARGET_THIS_SPAWNER_ENTITY,
    ACTION_TARGET_THIS_SPAWNER_POSITION,
    ACTION_TARGET_THIS_TARGET_ENTITY,
    ACTION_TARGET_THIS_TARGET_POSITION,
    ACTION_TARGET_THIS_OWNER_TARGET_ENTITY,
    ACTION_TARGET_THIS_OWNER_TARGET_POSITION,
    ACTION_TARGET_THIS_PROXY_ENTITY,
    ACTION_TARGET_THIS_PROXY_POSITION,
    ACTION_TARGET_THIS_PROXY_ENTITY1,
    ACTION_TARGET_THIS_PROXY_POSITION1,
    ACTION_TARGET_THIS_PROXY_ENTITY2,
    ACTION_TARGET_THIS_PROXY_POSITION2,
    ACTION_TARGET_THIS_PROXY_ENTITY3,
    ACTION_TARGET_THIS_PROXY_POSITION3,
    ACTION_TARGET_DELTA_POSITION,
    ACTION_TARGET_POS0,
    ACTION_TARGET_POS1,
    ACTION_TARGET_POS2,
    ACTION_TARGET_POS3,
    ACTION_TARGET_ENT0,
    ACTION_TARGET_ENT0_POSITION,
    ACTION_TARGET_ENT1,
    ACTION_TARGET_ENT1_POSITION,
    ACTION_TARGET_ENT2,
    ACTION_TARGET_ENT2_POSITION,
    ACTION_TARGET_ENT3,
    ACTION_TARGET_ENT3_POSITION,
};

enum EUnitCommand
{
    UNITCMD_INVALID,

    UNITCMD_ATTACK,
    UNITCMD_MOVE,
    UNITCMD_STOP,
    UNITCMD_HOLD,
    UNITCMD_FOLLOW,
    UNITCMD_PATROL,
    UNITCMD_ABILITY,
    UNITCMD_ABILITY2,
    UNITCMD_ATTACKMOVE,
    UNITCMD_SENTRY,
    UNITCMD_DROPITEM,
    UNITCMD_GIVEITEM,
    UNITCMD_TOUCH,
    UNITCMD_GUARD,
    UNITCMD_WANDER,
    UNITCMD_AGGRESSIVEWANDER,
    UNITCMD_AGGRO,
    UNITCMD_EVENT,
    UNITCMD_GUARDFOLLOW,
    UNITCMD_FOLLOWGUARD,
    UNITCMD_ASSIST,
    UNITCMD_ATTACKFOLLOW,
    UNITCMD_DOUBLE_ACTIVATE_ABILITY,

    NUM_UNITCMDS
};

enum EGameLogEvent
{
    GAME_LOG_INVALID,

    GAME_LOG_INFO_DATE,
    GAME_LOG_INFO_SERVER,
    GAME_LOG_INFO_GAME,
    GAME_LOG_INFO_MATCH,
    GAME_LOG_INFO_MAP,
    GAME_LOG_INFO_SETTINGS,

    GAME_LOG_PLAYER_CONNECT,
    GAME_LOG_PLAYER_DISCONNECT,
    GAME_LOG_PLAYER_TIMEDOUT,
    GAME_LOG_PLAYER_KICKED,
    GAME_LOG_PLAYER_KICKED_VOTE,
    GAME_LOG_PLAYER_KICKED_AFK,
    GAME_LOG_PLAYER_TERMINATED,
    GAME_LOG_PLAYER_TEAM_CHANGE,
    GAME_LOG_PLAYER_CHAT,
    GAME_LOG_PLAYER_ACTIONS,
    GAME_LOG_PLAYER_SELECT,
    GAME_LOG_PLAYER_RANDOM,
    GAME_LOG_PLAYER_REPICK,
    GAME_LOG_PLAYER_BAN,
    GAME_LOG_PLAYER_SWAP,
    GAME_LOG_PLAYER_BUYBACK,
    GAME_LOG_PLAYER_CALL_VOTE,
    GAME_LOG_PLAYER_VOTE,

    GAME_LOG_GAME_START,
    GAME_LOG_GAME_END,
    GAME_LOG_GAME_CONCEDE,
    GAME_LOG_GAME_PAUSE,
    GAME_LOG_GAME_RESUME,

    GAME_LOG_HERO_DEATH,
    GAME_LOG_HERO_SUICIDE,
    GAME_LOG_HERO_ASSIST,
    GAME_LOG_HERO_DENY,
    GAME_LOG_HERO_RESPAWN,
    GAME_LOG_HERO_LEVEL,
    GAME_LOG_HERO_POWERUP,

    GAME_LOG_CREEP_DENY,

    GAME_LOG_BUILDING_DENY,

    GAME_LOG_DAMAGE,
    GAME_LOG_KILL,

    GAME_LOG_ITEM_PURCHASE,
    GAME_LOG_ITEM_SELL,
    GAME_LOG_ITEM_ASSEMBLE,
    GAME_LOG_ITEM_DISASSEMBLE,
    GAME_LOG_ITEM_DROP,
    GAME_LOG_ITEM_PICKUP,
    GAME_LOG_ITEM_TRANSFER,
    GAME_LOG_ITEM_ACTIVATE,

    GAME_LOG_EXP_EARNED,
    GAME_LOG_EXP_DENIED,

    GAME_LOG_GOLD_LOST,
    GAME_LOG_GOLD_EARNED,

    GAME_LOG_ABILITY_UPGRADE,
    GAME_LOG_ABILITY_ACTIVATE,

    GAME_LOG_AWARD_FIRST_BLOOD,
    GAME_LOG_AWARD_KILL_STREAK,
    GAME_LOG_AWARD_MULTI_KILL,
    GAME_LOG_AWARD_KILL_STREAK_BREAK,
    GAME_LOG_AWARD_SMACKDOWN,
    GAME_LOG_AWARD_HUMILIATION,
    GAME_LOG_AWARD_RIVAL,
    GAME_LOG_AWARD_PAYBACK
};

template<> inline EAttribute        GetDefaultEmptyValue<EAttribute>()          { return ATTRIBUTE_INVALID; }
template<> inline ETargetSelection  GetDefaultEmptyValue<ETargetSelection>()    { return TARGET_SELECT_NONE; }
template<> inline EEntityToolAction GetDefaultEmptyValue<EEntityToolAction>()   { return TOOL_ACTION_INVALID; }
template<> inline EActionTarget     GetDefaultEmptyValue<EActionTarget>()       { return ACTION_TARGET_INVALID; }
template<> inline EUnitCommand      GetDefaultEmptyValue<EUnitCommand>()        { return UNITCMD_INVALID; }
template<> inline ESuperType        GetDefaultEmptyValue<ESuperType>()          { return SUPERTYPE_INVALID; }
template<> inline EGamePhase        GetDefaultEmptyValue<EGamePhase>()          { return GAME_PHASE_INVALID; }

inline EGamePhase   GetGamePhaseFromString(const tstring &sGamePhase)
{
    if (CompareNoCase(sGamePhase, _T("idle")) == 0)
        return GAME_PHASE_IDLE;
    else if (CompareNoCase(sGamePhase, _T("waiting_for_players")) == 0)
        return GAME_PHASE_WAITING_FOR_PLAYERS;
    else if (CompareNoCase(sGamePhase, _T("hero_ban")) == 0)
        return GAME_PHASE_HERO_BAN;
    else if (CompareNoCase(sGamePhase, _T("hero_select")) == 0)
        return GAME_PHASE_HERO_SELECT;
    else if (CompareNoCase(sGamePhase, _T("hero_loading")) == 0)
        return GAME_PHASE_HERO_LOADING;
    else if (CompareNoCase(sGamePhase, _T("pre_match")) == 0)
        return GAME_PHASE_PRE_MATCH;
    else if (CompareNoCase(sGamePhase, _T("active")) == 0)
        return GAME_PHASE_ACTIVE;
    else if (CompareNoCase(sGamePhase, _T("ended")) == 0)
        return GAME_PHASE_ENDED;

    return GAME_PHASE_INVALID;
}

inline EGamePhase&  AtoX(const tstring &s, EGamePhase &e)   { return e = GetGamePhaseFromString(s); }

inline EAttribute   GetAttributeFromString(const tstring &sAttribute)
{
    if (CompareNoCase(sAttribute, _T("strength")) == 0)
        return ATTRIBUTE_STRENGTH;
    else if (CompareNoCase(sAttribute, _T("agility")) == 0)
        return ATTRIBUTE_AGILITY;
    else if (CompareNoCase(sAttribute, _T("intelligence")) == 0)
        return ATTRIBUTE_INTELLIGENCE;
    else
        return ATTRIBUTE_INVALID;
}

inline EAttribute&  AtoX(const tstring &s, EAttribute &e)   { return e = GetAttributeFromString(s); }

inline ETargetSelection&    AtoX(const tstring &s, ETargetSelection &e)
{
    if (CompareNoCase(s, _T("closest")) == 0)
        return e = TARGET_SELECT_CLOSEST;
    else if (CompareNoCase(s, _T("furthest")) == 0)
        return e = TARGET_SELECT_FURTHEST;
    else if (CompareNoCase(s, _T("random")) == 0)
        return e = TARGET_SELECT_RANDOM;
    else if (CompareNoCase(s, _T("random_position")) == 0)
        return e = TARGET_SELECT_RANDOM_POSITION;
    else if (CompareNoCase(s, _T("random_angle_distance")) == 0)
        return e = TARGET_SELECT_RANDOM_ANGLE_DISTANCE;
    else if (CompareNoCase(s, _T("all")) == 0)
        return e = TARGET_SELECT_ALL;
    else if (CompareNoCase(s, _T("")) == 0)
        return e = TARGET_SELECT_ALL;

    return e = TARGET_SELECT_NONE;
}

inline EEntityToolAction    GetActionTypeFromString(const tstring &sActionType)
{
    if (CompareNoCase(sActionType, _T("passive")) == 0)
        return TOOL_ACTION_PASSIVE;
    else if (CompareNoCase(sActionType, _T("toggle")) == 0)
        return TOOL_ACTION_TOGGLE;
    else if (CompareNoCase(sActionType, _T("no_target")) == 0)
        return TOOL_ACTION_NO_TARGET;
    else if (CompareNoCase(sActionType, _T("target_entity")) == 0)
        return TOOL_ACTION_TARGET_ENTITY;
    else if (CompareNoCase(sActionType, _T("target_position")) == 0)
        return TOOL_ACTION_TARGET_POSITION;
    else if (CompareNoCase(sActionType, _T("global")) == 0)
        return TOOL_ACTION_GLOBAL;
    else if (CompareNoCase(sActionType, _T("target_self")) == 0)
        return TOOL_ACTION_TARGET_SELF;
    else if (CompareNoCase(sActionType, _T("facing")) == 0)
        return TOOL_ACTION_FACING;
    else if (CompareNoCase(sActionType, _T("self_position")) == 0)
        return TOOL_ACTION_SELF_POSITION;
    else if (CompareNoCase(sActionType, _T("attack")) == 0)
        return TOOL_ACTION_ATTACK;
    else if (CompareNoCase(sActionType, _T("attack_toggle")) == 0)
        return TOOL_ACTION_ATTACK_TOGGLE;
    else if (CompareNoCase(sActionType, _T("target_dual")) == 0)
        return TOOL_ACTION_TARGET_DUAL;
    else if (CompareNoCase(sActionType, _T("target_dual_position")) == 0)
        return TOOL_ACTION_TARGET_DUAL_POSITION;
    else if (CompareNoCase(sActionType, _T("target_vector")) == 0)
        return TOOL_ACTION_TARGET_VECTOR;
    else if (CompareNoCase(sActionType, _T("target_cursor")) == 0)
        return TOOL_ACTION_TARGET_CURSOR;

    return TOOL_ACTION_INVALID;
}

inline EEntityToolAction&   AtoX(const tstring &s, EEntityToolAction &e)    { return e = GetActionTypeFromString(s); }

inline EActionTarget    GetActionTargetFromString(const tstring &sName)
{
    if (CompareNoCase(sName, _T("source_entity")) == 0)
        return ACTION_TARGET_SOURCE_ENTITY;
    else if (CompareNoCase(sName, _T("source_position")) == 0)
        return ACTION_TARGET_SOURCE_POSITION;
    else if (CompareNoCase(sName, _T("source_target_offset")) == 0)
        return ACTION_TARGET_SOURCE_TARGET_OFFSET;
    else if (CompareNoCase(sName, _T("source_attack_offset")) == 0)
        return ACTION_TARGET_SOURCE_ATTACK_OFFSET;
    else if (CompareNoCase(sName, _T("source_owner_entity")) == 0)
        return ACTION_TARGET_SOURCE_OWNER_ENTITY;
    else if (CompareNoCase(sName, _T("source_owner_position")) == 0)
        return ACTION_TARGET_SOURCE_OWNER_POSITION;
    else if (CompareNoCase(sName, _T("target_entity")) == 0)
        return ACTION_TARGET_TARGET_ENTITY;
    else if (CompareNoCase(sName, _T("target_position")) == 0)
        return ACTION_TARGET_TARGET_POSITION;
    else if (CompareNoCase(sName, _T("target_target_offset")) == 0)
        return ACTION_TARGET_TARGET_TARGET_OFFSET;
    else if (CompareNoCase(sName, _T("target_attack_offset")) == 0)
        return ACTION_TARGET_TARGET_ATTACK_OFFSET;
    else if (CompareNoCase(sName, _T("target_owner_entity")) == 0)
        return ACTION_TARGET_TARGET_OWNER_ENTITY;
    else if (CompareNoCase(sName, _T("target_owner_position")) == 0)
        return ACTION_TARGET_TARGET_OWNER_POSITION;
    else if (CompareNoCase(sName, _T("inflictor_entity")) == 0)
        return ACTION_TARGET_INFLICTOR_ENTITY;
    else if (CompareNoCase(sName, _T("inflictor_position")) == 0)
        return ACTION_TARGET_INFLICTOR_POSITION;
    else if (CompareNoCase(sName, _T("inflictor_target_offset")) == 0)
        return ACTION_TARGET_INFLICTOR_TARGET_OFFSET;
    else if (CompareNoCase(sName, _T("inflictor_owner_entity")) == 0)
        return ACTION_TARGET_INFLICTOR_OWNER_ENTITY;
    else if (CompareNoCase(sName, _T("inflictor_owner_position")) == 0)
        return ACTION_TARGET_INFLICTOR_OWNER_POSITION;
    else if (CompareNoCase(sName, _T("proxy_entity")) == 0)
        return ACTION_TARGET_PROXY_ENTITY;
    else if (CompareNoCase(sName, _T("proxy_position")) == 0)
        return ACTION_TARGET_PROXY_POSITION;
    else if (CompareNoCase(sName, _T("stack_entity")) == 0)
        return ACTION_TARGET_STACK_ENTITY;
    else if (CompareNoCase(sName, _T("stack_position")) == 0)
        return ACTION_TARGET_STACK_POSITION;
    else if (CompareNoCase(sName, _T("this_entity")) == 0)
        return ACTION_TARGET_THIS_ENTITY;
    else if (CompareNoCase(sName, _T("this_position")) == 0)
        return ACTION_TARGET_THIS_POSITION;
    else if (CompareNoCase(sName, _T("this_target_offset")) == 0)
        return ACTION_TARGET_THIS_TARGET_OFFSET;
    else if (CompareNoCase(sName, _T("this_attack_offset")) == 0)
        return ACTION_TARGET_THIS_ATTACK_OFFSET;
    else if (CompareNoCase(sName, _T("this_owner_entity")) == 0)
        return ACTION_TARGET_THIS_OWNER_ENTITY;
    else if (CompareNoCase(sName, _T("this_owner_position")) == 0)
        return ACTION_TARGET_THIS_OWNER_POSITION;
    else if (CompareNoCase(sName, _T("this_inflictor_entity")) == 0)
        return ACTION_TARGET_THIS_INFLICTOR_ENTITY;
    else if (CompareNoCase(sName, _T("this_inflictor_position")) == 0)
        return ACTION_TARGET_THIS_INFLICTOR_POSITION;
    else if (CompareNoCase(sName, _T("this_spawner_entity")) == 0)
        return ACTION_TARGET_THIS_SPAWNER_ENTITY;
    else if (CompareNoCase(sName, _T("this_spawner_position")) == 0)
        return ACTION_TARGET_THIS_SPAWNER_POSITION;
    else if (CompareNoCase(sName, _T("this_target_entity")) == 0)
        return ACTION_TARGET_THIS_TARGET_ENTITY;
    else if (CompareNoCase(sName, _T("this_target_position")) == 0)
        return ACTION_TARGET_THIS_TARGET_POSITION;
    else if (CompareNoCase(sName, _T("this_owner_target_entity")) == 0)
        return ACTION_TARGET_THIS_OWNER_TARGET_ENTITY;
    else if (CompareNoCase(sName, _T("this_owner_target_position")) == 0)
        return ACTION_TARGET_THIS_OWNER_TARGET_POSITION;
    else if (CompareNoCase(sName, _T("this_proxy_entity")) == 0)
        return ACTION_TARGET_THIS_PROXY_ENTITY;
    else if (CompareNoCase(sName, _T("this_proxy_position")) == 0)
        return ACTION_TARGET_THIS_PROXY_POSITION;
    else if (CompareNoCase(sName, _T("this_proxy_entity1")) == 0)
        return ACTION_TARGET_THIS_PROXY_ENTITY1;
    else if (CompareNoCase(sName, _T("this_proxy_position1")) == 0)
        return ACTION_TARGET_THIS_PROXY_POSITION1;
    else if (CompareNoCase(sName, _T("this_proxy_entity2")) == 0)
        return ACTION_TARGET_THIS_PROXY_ENTITY2;
    else if (CompareNoCase(sName, _T("this_proxy_position2")) == 0)
        return ACTION_TARGET_THIS_PROXY_POSITION2;
    else if (CompareNoCase(sName, _T("this_proxy_entity3")) == 0)
        return ACTION_TARGET_THIS_PROXY_ENTITY3;
    else if (CompareNoCase(sName, _T("this_proxy_position3")) == 0)
        return ACTION_TARGET_THIS_PROXY_POSITION3;
    else if (CompareNoCase(sName, _T("delta_position")) == 0)
        return ACTION_TARGET_DELTA_POSITION;
    else if (CompareNoCase(sName, _T("pos0")) == 0)
        return ACTION_TARGET_POS0;
    else if (CompareNoCase(sName, _T("pos1")) == 0)
        return ACTION_TARGET_POS1;
    else if (CompareNoCase(sName, _T("pos2")) == 0)
        return ACTION_TARGET_POS2;
    else if (CompareNoCase(sName, _T("pos3")) == 0)
        return ACTION_TARGET_POS3;
    else if (CompareNoCase(sName, _T("ent0")) == 0)
        return ACTION_TARGET_ENT0;
    else if (CompareNoCase(sName, _T("ent0_position")) == 0)
        return ACTION_TARGET_ENT0_POSITION;
    else if (CompareNoCase(sName, _T("ent1")) == 0)
        return ACTION_TARGET_ENT1;
    else if (CompareNoCase(sName, _T("ent1_position")) == 0)
        return ACTION_TARGET_ENT1_POSITION;
    else if (CompareNoCase(sName, _T("ent2")) == 0)
        return ACTION_TARGET_ENT2;
    else if (CompareNoCase(sName, _T("ent2_position")) == 0)
        return ACTION_TARGET_ENT2_POSITION;
    else if (CompareNoCase(sName, _T("ent3")) == 0)
        return ACTION_TARGET_ENT3;
    else if (CompareNoCase(sName, _T("ent3_position")) == 0)
        return ACTION_TARGET_ENT3_POSITION;

    return ACTION_TARGET_INVALID;
}

inline EActionTarget    AtoX(const tstring &s, EActionTarget &e)    { return e = GetActionTargetFromString(s); }

inline EUnitCommand GetUnitCommandFromString(const tstring &sName)
{
    if (CompareNoCase(sName, _T("attack")) == 0)
        return UNITCMD_ATTACK;
    else if (CompareNoCase(sName, _T("move")) == 0)
        return UNITCMD_MOVE;
    else if (CompareNoCase(sName, _T("stop")) == 0)
        return UNITCMD_STOP;
    else if (CompareNoCase(sName, _T("hold")) == 0)
        return UNITCMD_HOLD;
    else if (CompareNoCase(sName, _T("attack_move")) == 0)
        return UNITCMD_ATTACKMOVE;
    else if (CompareNoCase(sName, _T("follow")) == 0)
        return UNITCMD_FOLLOW;
    else if (CompareNoCase(sName, _T("ability")) == 0)
        return UNITCMD_ABILITY;
    else if (CompareNoCase(sName, _T("ability2")) == 0)
        return UNITCMD_ABILITY2;
    else if (CompareNoCase(sName, _T("drop")) == 0)
        return UNITCMD_DROPITEM;
    else if (CompareNoCase(sName, _T("give")) == 0)
        return UNITCMD_GIVEITEM;
    else if (CompareNoCase(sName, _T("guard")) == 0)
        return UNITCMD_GUARD;
    else if (CompareNoCase(sName, _T("patrol")) == 0)
        return UNITCMD_PATROL;
    else if (CompareNoCase(sName, _T("sentry")) == 0)
        return UNITCMD_SENTRY;
    else if (CompareNoCase(sName, _T("touch")) == 0)
        return UNITCMD_TOUCH;
    else if (CompareNoCase(sName, _T("wander")) == 0)
        return UNITCMD_WANDER;
    else if (CompareNoCase(sName, _T("aggressive_wander")) == 0)
        return UNITCMD_AGGRESSIVEWANDER;
    else if (CompareNoCase(sName, _T("aggro")) == 0)
        return UNITCMD_AGGRO;
    else if (CompareNoCase(sName, _T("event")) == 0)
        return UNITCMD_EVENT;
    else if (CompareNoCase(sName, _T("guard_follow")) == 0)
        return UNITCMD_GUARDFOLLOW;
    else if (CompareNoCase(sName, _T("follow_guard")) == 0)
        return UNITCMD_FOLLOWGUARD;
    else if (CompareNoCase(sName, _T("assist")) == 0)
        return UNITCMD_ASSIST;
    else if (CompareNoCase(sName, _T("attack_follow")) == 0)
        return UNITCMD_ATTACKFOLLOW;

    return UNITCMD_INVALID;
}

inline EUnitCommand AtoX(const tstring &s, EUnitCommand &e) { return e = GetUnitCommandFromString(s); }

inline EGameLogEvent    GetGameLogEventFromString(const tstring &sEvent)
{
    if (sEvent == _T("INFO_DATE")) return GAME_LOG_INFO_DATE;
    else if (sEvent == _T("INFO_SERVER")) return GAME_LOG_INFO_SERVER;
    else if (sEvent == _T("INFO_GAME")) return GAME_LOG_INFO_GAME;
    else if (sEvent == _T("INFO_MATCH")) return GAME_LOG_INFO_MATCH;
    else if (sEvent == _T("INFO_MAP")) return GAME_LOG_INFO_MAP;
    else if (sEvent == _T("INFO_SETTINGS")) return GAME_LOG_INFO_SETTINGS;

    else if (sEvent == _T("PLAYER_CONNECT")) return GAME_LOG_PLAYER_CONNECT;
    else if (sEvent == _T("PLAYER_DISCONNECT")) return GAME_LOG_PLAYER_DISCONNECT;
    else if (sEvent == _T("PLAYER_TIMEDOUT")) return GAME_LOG_PLAYER_TIMEDOUT;
    else if (sEvent == _T("PLAYER_KICKED")) return GAME_LOG_PLAYER_KICKED;
    else if (sEvent == _T("PLAYER_KICKED_AFK")) return GAME_LOG_PLAYER_KICKED_AFK;
    else if (sEvent == _T("PLAYER_KICKED_VOTE")) return GAME_LOG_PLAYER_KICKED_VOTE;
    else if (sEvent == _T("PLAYER_TERMINATED")) return GAME_LOG_PLAYER_TERMINATED;
    else if (sEvent == _T("PLAYER_TEAM_CHANGE")) return GAME_LOG_PLAYER_TEAM_CHANGE;
    else if (sEvent == _T("PLAYER_CHAT")) return GAME_LOG_PLAYER_CHAT;
    else if (sEvent == _T("PLAYER_ACTIONS")) return GAME_LOG_PLAYER_ACTIONS;
    else if (sEvent == _T("PLAYER_SELECT")) return GAME_LOG_PLAYER_SELECT;
    else if (sEvent == _T("PLAYER_RANDOM")) return GAME_LOG_PLAYER_RANDOM;
    else if (sEvent == _T("PLAYER_REPICK")) return GAME_LOG_PLAYER_REPICK;
    else if (sEvent == _T("PLAYER_BAN")) return GAME_LOG_PLAYER_BAN;
    else if (sEvent == _T("PLAYER_SWAP")) return GAME_LOG_PLAYER_SWAP;
    else if (sEvent == _T("PLAYER_BUYBACK")) return GAME_LOG_PLAYER_BUYBACK;
    else if (sEvent == _T("PLAYER_CALL_VOTE")) return GAME_LOG_PLAYER_CALL_VOTE;
    else if (sEvent == _T("PLAYER_VOTE")) return GAME_LOG_PLAYER_VOTE;

    else if (sEvent == _T("GAME_START")) return GAME_LOG_GAME_START;
    else if (sEvent == _T("GAME_END")) return GAME_LOG_GAME_END;
    else if (sEvent == _T("GAME_CONCEDE")) return GAME_LOG_GAME_CONCEDE;
    else if (sEvent == _T("GAME_PAUSE")) return GAME_LOG_GAME_PAUSE;
    else if (sEvent == _T("GAME_RESUME")) return GAME_LOG_GAME_RESUME;

    else if (sEvent == _T("HERO_DEATH")) return GAME_LOG_HERO_DEATH;
    else if (sEvent == _T("HERO_SUICIDE")) return GAME_LOG_HERO_SUICIDE;
    else if (sEvent == _T("HERO_ASSIST")) return GAME_LOG_HERO_ASSIST;
    else if (sEvent == _T("HERO_DENY")) return GAME_LOG_HERO_DENY;
    else if (sEvent == _T("HERO_RESPAWN")) return GAME_LOG_HERO_RESPAWN;
    else if (sEvent == _T("HERO_LEVEL")) return GAME_LOG_HERO_LEVEL;
    else if (sEvent == _T("HERO_POWERUP")) return GAME_LOG_HERO_POWERUP;

    else if (sEvent == _T("CREEP_DENY")) return GAME_LOG_CREEP_DENY;

    else if (sEvent == _T("BUILDING_DENY")) return GAME_LOG_BUILDING_DENY;

    else if (sEvent == _T("DAMAGE")) return GAME_LOG_DAMAGE;
    else if (sEvent == _T("KILL")) return GAME_LOG_KILL;

    else if (sEvent == _T("ITEM_PURCHASE")) return GAME_LOG_ITEM_PURCHASE;
    else if (sEvent == _T("ITEM_SELL")) return GAME_LOG_ITEM_SELL;
    else if (sEvent == _T("ITEM_ASSEMBLE")) return GAME_LOG_ITEM_ASSEMBLE;
    else if (sEvent == _T("ITEM_DISASSEMBLE")) return GAME_LOG_ITEM_DISASSEMBLE;
    else if (sEvent == _T("ITEM_DROP")) return GAME_LOG_ITEM_DROP;
    else if (sEvent == _T("ITEM_PICKUP")) return GAME_LOG_ITEM_PICKUP;
    else if (sEvent == _T("ITEM_TRANSFER")) return GAME_LOG_ITEM_TRANSFER;
    else if (sEvent == _T("ITEM_ACTIVATE")) return GAME_LOG_ITEM_ACTIVATE;

    else if (sEvent == _T("EXP_EARNED")) return GAME_LOG_EXP_EARNED;
    else if (sEvent == _T("EXP_DENIED")) return GAME_LOG_EXP_DENIED;

    else if (sEvent == _T("GOLD_LOST")) return GAME_LOG_GOLD_LOST;
    else if (sEvent == _T("GOLD_EARNED")) return GAME_LOG_GOLD_EARNED;

    else if (sEvent == _T("ABILITY_UPGRADE")) return GAME_LOG_ABILITY_UPGRADE;
    else if (sEvent == _T("ABILITY_ACTIVATE")) return GAME_LOG_ABILITY_ACTIVATE;

    else if (sEvent == _T("AWARD_FIRST_BLOOD")) return GAME_LOG_AWARD_FIRST_BLOOD;
    else if (sEvent == _T("AWARD_KILL_STREAK")) return GAME_LOG_AWARD_KILL_STREAK;
    else if (sEvent == _T("AWARD_MULTI_KILL")) return GAME_LOG_AWARD_MULTI_KILL;
    else if (sEvent == _T("AWARD_KILL_STREAK_BREAK")) return GAME_LOG_AWARD_KILL_STREAK_BREAK;
    else if (sEvent == _T("AWARD_SMACKDOWN")) return GAME_LOG_AWARD_SMACKDOWN;
    else if (sEvent == _T("AWARD_HUMILIATION")) return GAME_LOG_AWARD_HUMILIATION;
    else if (sEvent == _T("AWARD_RIVAL")) return GAME_LOG_AWARD_RIVAL;
    else if (sEvent == _T("AWARD_PAYBACK")) return GAME_LOG_AWARD_PAYBACK;

    return GAME_LOG_INVALID;
}

inline EGameLogEvent    AtoX(const tstring &s, EGameLogEvent &e)    { return e = GetGameLogEventFromString(s); }

inline tstring  GetGameLogEventName(EGameLogEvent eEvent)
{
    switch (eEvent)
    {
    case GAME_LOG_INFO_DATE: return _T("INFO_DATE");
    case GAME_LOG_INFO_SERVER: return _T("INFO_SERVER");
    case GAME_LOG_INFO_GAME: return _T("INFO_GAME");
    case GAME_LOG_INFO_MATCH: return _T("INFO_MATCH");
    case GAME_LOG_INFO_MAP: return _T("INFO_MAP");
    case GAME_LOG_INFO_SETTINGS: return _T("INFO_SETTINGS");

    case GAME_LOG_PLAYER_CONNECT: return _T("PLAYER_CONNECT");
    case GAME_LOG_PLAYER_DISCONNECT: return _T("PLAYER_DISCONNECT");
    case GAME_LOG_PLAYER_TIMEDOUT: return _T("PLAYER_TIMEDOUT");
    case GAME_LOG_PLAYER_KICKED: return _T("PLAYER_KICKED");
    case GAME_LOG_PLAYER_KICKED_AFK: return _T("PLAYER_KICKED_AFK");
    case GAME_LOG_PLAYER_KICKED_VOTE: return _T("PLAYER_KICKED_VOTE");
    case GAME_LOG_PLAYER_TERMINATED: return _T("PLAYER_TERMINATED");
    case GAME_LOG_PLAYER_TEAM_CHANGE: return _T("PLAYER_TEAM_CHANGE");
    case GAME_LOG_PLAYER_CHAT: return _T("PLAYER_CHAT");
    case GAME_LOG_PLAYER_ACTIONS: return _T("PLAYER_ACTIONS");
    case GAME_LOG_PLAYER_SELECT: return _T("PLAYER_SELECT");
    case GAME_LOG_PLAYER_RANDOM: return _T("PLAYER_RANDOM");
    case GAME_LOG_PLAYER_REPICK: return _T("PLAYER_REPICK");
    case GAME_LOG_PLAYER_BAN: return _T("PLAYER_BAN");
    case GAME_LOG_PLAYER_SWAP: return _T("PLAYER_SWAP");
    case GAME_LOG_PLAYER_BUYBACK: return _T("PLAYER_BUYBACK");
    case GAME_LOG_PLAYER_CALL_VOTE: return _T("PLAYER_CALL_VOTE");
    case GAME_LOG_PLAYER_VOTE: return _T("PLAYER_VOTE");

    case GAME_LOG_GAME_START: return _T("GAME_START");
    case GAME_LOG_GAME_END: return _T("GAME_END");
    case GAME_LOG_GAME_CONCEDE: return _T("GAME_CONCEDE");
    case GAME_LOG_GAME_PAUSE: return _T("GAME_PAUSE");
    case GAME_LOG_GAME_RESUME: return _T("GAME_RESUME");

    case GAME_LOG_HERO_DEATH: return _T("HERO_DEATH");
    case GAME_LOG_HERO_SUICIDE: return _T("HERO_SUICIDE");
    case GAME_LOG_HERO_ASSIST: return _T("HERO_ASSIST");
    case GAME_LOG_HERO_DENY: return _T("HERO_DENY");
    case GAME_LOG_HERO_RESPAWN: return _T("HERO_RESPAWN");
    case GAME_LOG_HERO_LEVEL: return _T("HERO_LEVEL");
    case GAME_LOG_HERO_POWERUP: return _T("HERO_POWERUP");

    case GAME_LOG_CREEP_DENY: return _T("CREEP_DENY");

    case GAME_LOG_BUILDING_DENY: return _T("BUILDING_DENY");

    case GAME_LOG_DAMAGE: return _T("DAMAGE");
    case GAME_LOG_KILL: return _T("KILL");

    case GAME_LOG_ITEM_PURCHASE: return _T("ITEM_PURCHASE");
    case GAME_LOG_ITEM_SELL: return _T("ITEM_SELL");
    case GAME_LOG_ITEM_ASSEMBLE: return _T("ITEM_ASSEMBLE");
    case GAME_LOG_ITEM_DISASSEMBLE: return _T("ITEM_DISASSEMBLE");
    case GAME_LOG_ITEM_DROP: return _T("ITEM_DROP");
    case GAME_LOG_ITEM_PICKUP: return _T("ITEM_PICKUP");
    case GAME_LOG_ITEM_TRANSFER: return _T("ITEM_TRANSFER");
    case GAME_LOG_ITEM_ACTIVATE: return _T("ITEM_ACTIVATE");

    case GAME_LOG_EXP_EARNED: return _T("EXP_EARNED");
    case GAME_LOG_EXP_DENIED: return _T("EXP_DENIED");

    case GAME_LOG_GOLD_LOST: return _T("GOLD_LOST");
    case GAME_LOG_GOLD_EARNED: return _T("GOLD_EARNED");

    case GAME_LOG_ABILITY_UPGRADE: return _T("ABILITY_UPGRADE");
    case GAME_LOG_ABILITY_ACTIVATE: return _T("ABILITY_ACTIVATE");

    case GAME_LOG_AWARD_FIRST_BLOOD: return _T("AWARD_FIRST_BLOOD");
    case GAME_LOG_AWARD_KILL_STREAK: return _T("AWARD_KILL_STREAK");
    case GAME_LOG_AWARD_MULTI_KILL: return _T("AWARD_MULTI_KILL");
    case GAME_LOG_AWARD_KILL_STREAK_BREAK: return _T("AWARD_KILL_STREAK_BREAK");
    case GAME_LOG_AWARD_SMACKDOWN: return _T("AWARD_SMACKDOWN");
    case GAME_LOG_AWARD_HUMILIATION: return _T("AWARD_HUMILIATION");
    case GAME_LOG_AWARD_RIVAL: return _T("AWARD_RIVAL");
    case GAME_LOG_AWARD_PAYBACK: return _T("AWARD_PAYBACK");
    

    default: return _T("INVALID");
    }
}

enum EVoteType
{
    VOTE_TYPE_INVALID,
    VOTE_TYPE_REMAKE,
    VOTE_TYPE_CONCEDE,
    VOTE_TYPE_KICK,
    VOTE_TYPE_KICK_AFK,
    VOTE_TYPE_PAUSE,

    NUM_VOTE_TYPES
};

inline EVoteType    GetVoteTypeFromString(const tstring &sAttribute)
{
    if (CompareNoCase(sAttribute, _T("remake")) == 0)
        return VOTE_TYPE_REMAKE;
    else if (CompareNoCase(sAttribute, _T("concede")) == 0)
        return VOTE_TYPE_CONCEDE;
    else if (CompareNoCase(sAttribute, _T("kick")) == 0)
        return VOTE_TYPE_KICK;
    else if (CompareNoCase(sAttribute, _T("kick_afk")) == 0)
        return VOTE_TYPE_KICK_AFK;
    else if (CompareNoCase(sAttribute, _T("pause")) == 0)
        return VOTE_TYPE_PAUSE;
    else
        return VOTE_TYPE_INVALID;
}

inline tstring  GetVoteTypeName(uint uiVoteType)
{
    switch (uiVoteType)
    {
    case VOTE_TYPE_REMAKE:      return _T("remake");
    case VOTE_TYPE_CONCEDE:     return _T("concede");
    case VOTE_TYPE_KICK:        return _T("kick");
    case VOTE_TYPE_KICK_AFK:    return _T("kick_afk");
    case VOTE_TYPE_PAUSE:       return _T("pause");
    default:                    return _T("INVALID");
    }
}

const byte VOTE_NONE    (0);
const byte VOTE_YES     (1);
const byte VOTE_NO      (2);

inline tstring GetVoteName(uint uiVote)
{
    switch (uiVote)
    {
    case VOTE_YES:  return _T("yes");
    case VOTE_NO:   return _T("no");
    case VOTE_NONE: return _T("none");
    default:        return _T("invalid");
    }
}

inline EVoteType&   AtoX(const tstring &s, EVoteType &e)    { return e = GetVoteTypeFromString(s); }

inline ESuperType   GetSuperTypeFromString(const tstring &sSuperType)
{
    if (TStringCompare(sSuperType, _T("attack")) == 0)
        return SUPERTYPE_ATTACK;
    else if (TStringCompare(sSuperType, _T("spell")) == 0)
        return SUPERTYPE_SPELL;
    else
        return SUPERTYPE_INVALID;
}

inline ESuperType   AtoX(const tstring &s, ESuperType &e)   { return e = GetSuperTypeFromString(s); }

enum EEntityActionScript
{
    ACTION_SCRIPT_FRAME,                    // Processed once each frame
    ACTION_SCRIPT_FRAME_IMPACT,             // Processed once each frame per target (affectors)
    ACTION_SCRIPT_INTERVAL,                 // Processed once each interval (affectors)
    ACTION_SCRIPT_BEGIN,                    // The owner has activated this entity
    ACTION_SCRIPT_START,                    // Tool has begun to activate
    ACTION_SCRIPT_PRE_COST,                 // A chance for the tool to modifier its activation cost
    ACTION_SCRIPT_ACTION,                   // This entity has reached its action time (called once per activatation)
    ACTION_SCRIPT_PRE_IMPACT,
    ACTION_SCRIPT_PRE_DAMAGE,
    ACTION_SCRIPT_DAMAGE_EVENT,
    ACTION_SCRIPT_DOUBLE_ACTIVATE,          // Tool has been double-activated
    ACTION_SCRIPT_IMPACT,                   // This entity has reached its action time (called once per target)
    ACTION_SCRIPT_IMPACT_INVALID,           // This entity has reached its action but the impact was invalid
    ACTION_SCRIPT_COMPLETE,                 // Tool successfully activates
    ACTION_SCRIPT_CANCEL,                   // Tool failed to activate
    ACTION_SCRIPT_ACTIVATE_START,           // The owner has activated this entity
    ACTION_SCRIPT_ACTIVATE_PRE_COST,
    ACTION_SCRIPT_ACTIVATE_PRE_IMPACT,      // This entity has reached its action time
    ACTION_SCRIPT_ACTIVATE_IMPACT,          // This entity has reached its action time
    ACTION_SCRIPT_ACTIVATE_END,             // Processed after an activated tool reaches the end of its cast time
    ACTION_SCRIPT_ABILITY_START,            // The owner has activated an ability other than this entity
    ACTION_SCRIPT_ABILITY_IMPACT,           // An ability other than this entity has reached its action time
    ACTION_SCRIPT_ABILITY_FINISH,
    ACTION_SCRIPT_ABILITY_END,              // Processed after an activated ability other than this entity reaches the end of its cast time
    ACTION_SCRIPT_TOGGLE_ON,                // This tool with action type "toggle" has been turned on
    ACTION_SCRIPT_TOGGLE_OFF,               // This tool with action type "toggle" has been turned off
    ACTION_SCRIPT_CHANNEL_START,            // The owner of this entity has activated a channeling tool
    ACTION_SCRIPT_CHANNEL_FRAME,            // Processed each frame the owner is channeling this tool
    ACTION_SCRIPT_CHANNEL_BROKEN,           // The owner has stopped channeling before the channel time completed
    ACTION_SCRIPT_CHANNEL_END,              // The owner has completed the full channel time of this entity
    ACTION_SCRIPT_CHANNELING_START,         // The owner of this entity has activated any channeling tool
    ACTION_SCRIPT_CHANNELING_FRAME,         // Processed each frame the owner is channeling any tool
    ACTION_SCRIPT_CHANNELING_BROKEN,        // The owner has stopped channeling his "tool" before the channel time completed
    ACTION_SCRIPT_CHANNELING_END,           // The owner has completed the full channel time of any tool
    ACTION_SCRIPT_ATTACK_START,             // Processed when the owner begins an attack
    ACTION_SCRIPT_ATTACK,                   // Processed when the owner reaches the attack action time
    ACTION_SCRIPT_ATTACK_PRE_IMPACT,        // Processed when the owner successfully attacks another entity, before damage and evasion
    ACTION_SCRIPT_ATTACK_PRE_DAMAGE,        // Processed when the owner successfully attacks another entity, before damage
    ACTION_SCRIPT_ATTACK_DAMAGE_EVENT,
    ACTION_SCRIPT_ATTACK_IMPACT,            // Processed when the owner successfully attacks another entity and impacts
    ACTION_SCRIPT_ATTACK_IMPACT_INVALID,
    ACTION_SCRIPT_ATTACK_END,               // Processed when the owner completes an attack, successful or not
    ACTION_SCRIPT_ATTACKED_START,           // An attack has been started on this entity
    ACTION_SCRIPT_ATTACKED_PRE_IMPACT,      // This entity has been attacked, but damage/effects/evasion have not yet been applied
    ACTION_SCRIPT_ATTACKED_PRE_DAMAGE,      // This entity has been attacked, but damage/effects have not yet been applied
    ACTION_SCRIPT_ATTACKED_DAMAGE_EVENT,
    ACTION_SCRIPT_ATTACKED_POST_IMPACT,     // This entity has been attacked and damage/effects have been applied (does not trigger on a miss/evade)
    ACTION_SCRIPT_ATTACKING_START,
    ACTION_SCRIPT_ATTACKING_PRE_IMPACT,
    ACTION_SCRIPT_ATTACKING_PRE_DAMAGE,
    ACTION_SCRIPT_ATTACKING_DAMAGE_EVENT,
    ACTION_SCRIPT_ATTACKING_POST_IMPACT,

    ACTION_SCRIPT_DAMAGE,                   // This entity has dealt some amount of damage
    ACTION_SCRIPT_DAMAGED,                  // This entity has taken some amount of damage
    ACTION_SCRIPT_STUNNED,                  // This entity has been stunned
    ACTION_SCRIPT_KILLED,                   // This entity has been killed
    ACTION_SCRIPT_EXPIRED,                  // This entity has died due to its lifetime expiring
    ACTION_SCRIPT_DEATH,                    // This entity has dieded :(
    ACTION_SCRIPT_KILL,                     // This entity killed another entity
    ACTION_SCRIPT_INDIRECT_KILL,            // This entity indirectly killed another entity
    ACTION_SCRIPT_ASSIST,                   // This entity assisted in killing another entity

    ACTION_SCRIPT_SPAWN,                    // Processed when spawn is called
    ACTION_SCRIPT_RESPAWN,                  // Processed when respawn is called
    ACTION_SCRIPT_LEVELUP,                  // Hero levelup

    ACTION_SCRIPT_INFLICT,                  // New state applied to an entity
    ACTION_SCRIPT_REFRESH,                  // Resfreshed state on an entity
    ACTION_SCRIPT_INFLICTED,                // Entity has had a new state applied to it

    ACTION_SCRIPT_OWNER_RESPAWN,            // This entity's owner has respawned

    ACTION_SCRIPT_RELEASE,                  // This entity has lost its binding

    ACTION_SCRIPT_TOUCH,                    // This entity collided with another entity
    ACTION_SCRIPT_TOUCHED,                  // This entity has been touched by another entity

    ACTION_SCRIPT_THINK,                    // Executed before neutral NPC "thinking" occurs

    ACTION_SCRIPT_TARGET_ACQUIRED,          // Executed when a unit changes or acquires a target

    ACTION_SCRIPT_LEARN,                    // This ability was just learned for the first time
    ACTION_SCRIPT_UPGRADE,                  // This tool was leveluped
    ACTION_SCRIPT_CREATE,                   // Item was created
    ACTION_SCRIPT_PURCHASED,                // Item was just purchased
    ACTION_SCRIPT_TIMER,                    // Timer trigger
    ACTION_SCRIPT_PICKUP,                   // Item was moved into a new inventory
    ACTION_SCRIPT_READY,                    // Tool has become ready (off cooldown)

    ACTION_SCRIPT_LEASH,                    // A neutral has leashed

    ACTION_SCRIPT_CHECK_COST,
    ACTION_SCRIPT_CHECK_TRIGGERED_COST,
    ACTION_SCRIPT_ACTIVATE_COST,
    ACTION_SCRIPT_GET_THREAT_LEVEL,

    ACTION_SCRIPT_LOBBY_START,
    ACTION_SCRIPT_ADD_PLAYER,
    ACTION_SCRIPT_ENTER_GAME,

    NUM_ACTION_SCRIPTS
};

const ushort SIGHTED_BIT                    (BIT(0));
const ushort REVEALED_BIT                   (BIT(6));
const ushort VISION_BIT                     (BIT(7));
const ushort PLAYER_SIGHTED_BIT             (BIT(1));

#define VIS_SIGHTED(team)                   ushort(SIGHTED_BIT << (((team) - 1) * 8))
#define VIS_REVEALED(team)                  ushort(REVEALED_BIT << (((team) - 1) * 8))
#define VIS_VISION(team)                    ushort(VISION_BIT << (((team) - 1) * 8))
#define VIS_PLAYER_SIGHTED(team, player)    ushort(PLAYER_SIGHTED_BIT << ((((team) - 1) * 8) + (player)))

const ushort SIGHTED_FLAG_MASK              (SIGHTED_BIT | SIGHTED_BIT << 8);
const ushort REVEALED_FLAG_MASK             (REVEALED_BIT | REVEALED_BIT << 8);
const ushort VISION_FLAG_MASK               (VISION_BIT | VISION_BIT << 8);
const ushort PLAYER_SIGHTED_FLAG_MASK       ((ushort)~(SIGHTED_FLAG_MASK | REVEALED_FLAG_MASK | VISION_FLAG_MASK));
//=============================================================================

#endif //__HON_SHARED_TYPES_H__
