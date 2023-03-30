// (C)2006 S2 Games
// game_shared_protocol.h
//
//=============================================================================
#ifndef __GAME_SHARED_PROTOCOL_H__
#define __GAME_SHARED_PROTOCOL_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/k2_protocol.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const byte GAME_CMD_CHANGE_TEAM				(0x01);
const byte GAME_CMD_CHAT_ALL				(0x02);
const byte GAME_CMD_CHAT_TEAM				(0x03);
const byte GAME_CMD_BUILDING_DESTROYED		(0x04);
const byte GAME_CMD_SERVERCHAT_ALL			(0x05);
const byte GAME_CMD_SELECT_HERO				(0x06);
const byte GAME_CMD_RANDOM_HERO				(0x07);
const byte GAME_CMD_SERVER_STATUS			(0x08);
const byte GAME_CMD_PURCHASE				(0x09);
const byte GAME_CMD_SELL					(0x0a);
const byte GAME_CMD_VOTE					(0x0b);
const byte GAME_CMD_END_GAME				(0x0c);
const byte GAME_CMD_MESSAGE					(0x0d);
const byte GAME_CMD_DISASSEMBLE				(0x0e);
const byte GAME_CMD_SET_REPLAY_CLIENT		(0x0f);
const byte GAME_CMD_MINIMAP_DRAW			(0x10);
const byte GAME_CMD_MINIMAP_PING			(0x11);
const byte GAME_CMD_BUILDING_ATTACK_ALERT	(0x12);
const byte GAME_CMD_EXT_ENTITY_DATA			(0x13);
const byte GAME_CMD_REQUEST_EXT_ENTITY_DATA	(0x14);
const byte GAME_CMD_LEVEL_UP				(0x15);
const byte GAME_CMD_SHARE_FULL_CONTROL		(0x16);
const byte GAME_CMD_UNSHARE_FULL_CONTROL	(0x17);
const byte GAME_CMD_SHARE_PARTIAL_CONTROL	(0x18);
const byte GAME_CMD_UNSHARE_PARTIAL_CONTROL	(0x19);
const byte GAME_CMD_CREATE_GAME				(0x1a);
const byte GAME_CMD_ABILITY					(0x1b);
const byte GAME_CMD_ABILITY_POSITION		(0x1c);
const byte GAME_CMD_ABILITY_ENTITY			(0x1d);
const byte GAME_CMD_ORDER_POSITION			(0x1e);
const byte GAME_CMD_ORDER_ENTITY			(0x1f);
const byte GAME_CMD_ORDER_STOP				(0x20);
const byte GAME_CMD_ORDER_HOLD				(0x21);
const byte GAME_CMD_PICKUP_ITEM				(0x22);
const byte GAME_CMD_CYCLE_SHARED_CONTROL	(0x23);
const byte GAME_CMD_SERVER_STATS			(0x24);
const byte GAME_CMD_SUBMIT_KARMA_RATING		(0x25);
const byte GAME_CMD_SUBMIT_MATCH_COMMENT	(0x26);
const byte GAME_CMD_CALLVOTE				(0x27);
const byte GAME_CMD_UNPAUSE					(0x28);
const byte GAME_CMD_ITEM_FAILED				(0x29);
const byte GAME_CMD_ITEM_SUCCEEDED			(0x2a);
const byte GAME_CMD_BLD_HEALTH_LOW			(0x2b);
const byte GAME_CMD_MOVE_ITEM				(0x2c);
const byte GAME_CMD_ORDER_CONFIRMATION		(0x2d);
const byte GAME_CMD_PURCHASE2				(0x2e);
const byte GAME_CMD_READY					(0x2f);
const byte GAME_CMD_REQUEST_MATCH_START		(0x30);
const byte GAME_CMD_BUYBACK					(0x31);
const byte GAME_CMD_REPICK_HERO				(0x32);
const byte GAME_CMD_SWAP_HERO_REQUEST		(0x33);
const byte GAME_CMD_MAP_PING				(0x34);
const byte GAME_CMD_CONFIRM_MOVE			(0x35);
const byte GAME_CMD_CONFIRM_ATTACK			(0x36);
const byte GAME_CMD_KICK					(0x37);
const byte GAME_CMD_PROMOTE_REFEREE			(0x38);
const byte GAME_CMD_DEMOTE_REFEREE			(0x39);
const byte GAME_CMD_BAN_HERO				(0x3a);
const byte GAME_CMD_SET_ATTACK_MOD_SLOT		(0x3b);
const byte GAME_CMD_PREV_ATTACK_MOD_SLOT	(0x3c);
const byte GAME_CMD_NEXT_ATTACK_MOD_SLOT	(0x3d);
const byte GAME_CMD_POPUP					(0x3e);
const byte GAME_CMD_POPUP_VALUE				(0x3f);
const byte GAME_CMD_SELECTION				(0x40);
const byte GAME_CMD_TOUCH					(0x41);
const byte GAME_CMD_LEVELUP_EVENT			(0x42);
const byte GAME_CMD_EXPERIENCE_EVENT		(0x43);
const byte GAME_CMD_GOLD_EVENT				(0x44);
const byte GAME_CMD_INVENTORY_READY_EVENT	(0x45);
const byte GAME_CMD_INVENTORY_UPGRADE_EVENT	(0x46);
const byte GAME_CMD_ABILITY2				(0x47);
const byte GAME_CMD_UNREADY					(0x48);
const byte GAME_CMD_FINISHED_LOADING_HEROES	(0x49);
const byte GAME_CMD_LOADING_PROGRESS		(0x4a);
const byte GAME_CMD_LOCK_SLOT				(0x4b);
const byte GAME_CMD_UNLOCK_SLOT				(0x4c);
const byte GAME_CMD_TOGGLE_SLOT_LOCK		(0x4d);
const byte GAME_CMD_BALANCE_TEAMS			(0x4e);
const byte GAME_CMD_SWAP_PLAYER_SLOT		(0x4f);
const byte GAME_CMD_ASSIGN_SPECTATOR		(0x50);
const byte GAME_CMD_ASSIGN_HOST				(0x51);
const byte GAME_CMD_CHAT_ROLL				(0x52);
const byte GAME_CMD_CHAT_EMOTE				(0x53);
const byte GAME_CMD_REQUEST_MATCH_CANCEL	(0x54);
const byte GAME_CMD_MATCH_CANCEL_MESSAGE	(0x55);

const byte GAME_CMD_TAUNTED_SOUND			(0x56);
const byte GAME_CMD_TAUNT_KILLED_SOUND		(0x57);

const byte GAME_CMD_DRAFT_HERO				(0x58);

const byte GAME_CMD_KILL_MESSAGE				(0x60);
const byte GAME_CMD_FIRST_BLOOD_MESSAGE			(0x61);
const byte GAME_CMD_KILLSTREAK_MESSAGE			(0x62);
const byte GAME_CMD_END_STREAK_MESSAGE			(0x63);
const byte GAME_CMD_START_GAME_WARNING			(0x64);
const byte GAME_CMD_MULTIKILL_MESSAGE			(0x65);
const byte GAME_CMD_KILL_TOWER_MESSAGE			(0x66);
const byte GAME_CMD_DENY_TOWER_MESSAGE			(0x67);
const byte GAME_CMD_KILL_COURIER_MESSAGE		(0x68);
const byte GAME_CMD_TEAM_KILLSTREAK_MESSAGE		(0x69);
const byte GAME_CMD_TEAM_KILL_MESSAGE			(0x6a);
const byte GAME_CMD_TEAM_KILL_TOWER_MESSAGE		(0x6b);
const byte GAME_CMD_TEAM_DENY_TOWER_MESSAGE		(0x6c);
const byte GAME_CMD_NEUTRAL_KILL_MESSAGE		(0x6d);
const byte GAME_CMD_KONGOR_KILL_MESSAGE			(0x6e);
const byte GAME_CMD_SUICIDE_KILL_MESSAGE		(0x6f);
const byte GAME_CMD_UNKNOWN_KILL_MESSAGE		(0x70);
const byte GAME_CMD_KILL_KONGOR_MESSAGE			(0x71);
const byte GAME_CMD_TEAM_KILL_KONGOR_MESSAGE	(0x72);
const byte GAME_CMD_CONTROL_SHARE_MESSAGE		(0x73);
const byte GAME_CMD_POWERUP_MESSAGE				(0x74);
const byte GAME_CMD_CREEP_UPGRADE_MESSAGE		(0x75);
const byte GAME_CMD_MEGACREEP_MESSAGE			(0x76);
const byte GAME_CMD_SMACKDOWN_MESSAGE			(0x77);
const byte GAME_CMD_HUMILIATION_MESSAGE			(0x78);
const byte GAME_CMD_TEAM_WIPE_MESSAGE			(0x79);
const byte GAME_CMD_NEMESIS_MESSAGE				(0x7a);
const byte GAME_CMD_RETRIBUTION_MESSAGE			(0x7b);
const byte GAME_CMD_TERMINATION_WARNING_MESSAGE	(0x7c);
const byte GAME_CMD_TERMINATED_MESSAGE			(0x7d);
const byte GAME_CMD_CONNECT_MESSAGE				(0x7e);
const byte GAME_CMD_RECONNECT_MESSAGE			(0x7f);
const byte GAME_CMD_DISCONNECT_MESSAGE			(0x80);
const byte GAME_CMD_PICK_HERO_MESSAGE			(0x81);
const byte GAME_CMD_RANDOM_HERO_MESSAGE			(0x82);
const byte GAME_CMD_REPICK_HERO_MESSAGE			(0x83);
const byte GAME_CMD_SWAP_HERO_MESSAGE			(0x84);
const byte GAME_CMD_READY_MESSAGE				(0x85);
const byte GAME_CMD_UNREADY_MESSAGE				(0x86);
const byte GAME_CMD_SWAP_REQUEST_MESSAGE		(0x87);
const byte GAME_CMD_VOTE_CALLED_MESSAGE			(0x88);
const byte GAME_CMD_VOTE_PASSED_MESSAGE			(0x89);
const byte GAME_CMD_VOTE_FAILED_MESSAGE			(0x8a);
const byte GAME_CMD_CONCEDE_MESSAGE				(0x8b);
const byte GAME_CMD_ABANDONED_MESSAGE			(0x8c);
const byte GAME_CMD_AFK_WARNING_MESSAGE			(0x8d);
const byte GAME_CMD_LOBBY_CONNECT_MESSAGE		(0x8e);
const byte GAME_CMD_LOBBY_DISCONNECT_MESSAGE	(0x8f);
const byte GAME_CMD_LOBBY_KICK_MESSAGE			(0x90);
const byte GAME_CMD_LOBBY_BALANCED_MESSAGE		(0x91);
const byte GAME_CMD_GAME_MESSAGE				(0x92);
const byte GAME_CMD_RAGE_QUIT_MESSAGE			(0x93);
const byte GAME_CMD_FEEDER_MESSAGE				(0x94);
const byte GAME_CMD_BUILDING_UNDER_ATTACK_MESSAGE		(0x95);
const byte GAME_CMD_HERO_UNDER_ATTACK_MESSAGE			(0x96);
const byte GAME_CMD_HELLBOURNE_DESTROY_TOWER_MESSAGE	(0x97);
const byte GAME_CMD_LEGION_DESTROY_TOWER_MESSAGE		(0x98);
const byte GAME_CMD_LONG_SERVER_FRAME					(0x99);
const byte GAME_CMD_HERO_DENY_MESSAGE					(0xa0);
const byte GAME_CMD_LOBBY_FORCED_TEAM_SWAP_MESSAGE		(0xa1);
const byte GAME_CMD_BAN_HERO_MESSAGE					(0xa2);
const byte GAME_CMD_ABILITY_VECTOR						(0xa3);
const byte GAME_CMD_WALKING_COURIER_PURCHASED_MESSAGE	(0xa4);
const byte GAME_CMD_FLYING_COURIER_PURCHASED_MESSAGE	(0xa5);
const byte GAME_CMD_UNSTUCK								(0xa6);
const byte GAME_CMD_KILL_RAX_MESSAGE					(0xa7);
const byte GAME_CMD_TIMEDOUT_MESSAGE					(0xa8);
const byte GAME_CMD_SPAWN_CLIENT_THREAD			(0xa9);
const byte GAME_CMD_UI_TRIGGER					(0xaa);
const byte GAME_CMD_SCRIPT_MESSAGE				(0xab);
const byte GAME_CMD_OPEN_SHOP					(0xac);
const byte GAME_CMD_CLOSE_SHOP					(0xad);
const byte GAME_CMD_SET_ACTIVE_SHOP				(0xae);
const byte GAME_CMD_SET_ACTIVE_RECIPE			(0xaf);
const byte GAME_CMD_LOBBY_LOCKED_MESSAGE		(0xb0);
const byte GAME_CMD_LOBBY_UNLOCKED_MESSAGE		(0xb1);
const byte GAME_CMD_LOBBY_ASSIGNED_HOST_MESSAGE	(0xb2);
const byte GAME_CMD_LOBBY_ASSIGNED_SPECTATOR_MESSAGE	(0xb3);
const byte GAME_CMD_LOBBY_ASSIGNED_REFEREE_MESSAGE		(0xb4);
const byte GAME_CMD_LOCK_SHOP					(0xb5);
const byte GAME_CMD_UNLOCK_SHOP					(0xb6);
const byte GAME_CMD_PHASE						(0xb7);
const byte GAME_CMD_ASSIGN_FIRST_BAN_TEAM		(0xb8);
const byte GAME_CMD_LOBBY_ASSIGNED_REFEREE_MESSAGE2		(0xb9);
const byte GAME_CMD_GAMEPLAY_OPTION				(0xba);
const byte GAME_CMD_ORDER_CANCEL_AND_HOLD		(0xbb);
const byte GAME_CMD_DOUBLE_ACTIVATE_ABILITY		(0xbc);
const byte GAME_CMD_SELECT_POTENTIAL_HERO		(0xbd);
const byte GAME_CMD_PICK_POTENTIAL_HERO_MESSAGE	(0xbe);
const byte GAME_CMD_NOTIFICATION_FLAGS			(0xbf);
const byte GAME_CMD_GENERAL_MESSAGE				(0xc0);
const byte GAME_CMD_VERIFY_FILES				(0xc1);
const byte GAME_CMD_KILL_COURIER_MESSAGE2		(0xc2);
const byte GAME_CMD_PET_ADDED					(0xc3);
const byte GAME_CMD_PING_ALL					(0xc4);
const byte GAME_CMD_PING_ALL_MESSAGE			(0xc5);
const byte GAME_CMD_SET_NO_HELP					(0xc6);
const byte GAME_CMD_AFK_MESSAGE					(0xc7);

// Server Client Trial tracking.
const byte GAME_CMD_TRIAL_INC					(0xc8);

const byte GAME_CMD_START_SPRINT				(0xc9);
const byte GAME_CMD_STOP_SPRINT					(0xca);

const byte QUEUE_NONE	(0);
const byte QUEUE_BACK	(1);
const byte QUEUE_FRONT	(2);

enum EGameStateBlocks
{
	STATE_BLOCK_ENTITY_TYPES = STATE_BLOCK_NUM_RESERVED,

	STATE_BLOCK_FIRST_HERO_GROUP,
	STATE_BLOCK_HERO_GROUP0 = STATE_BLOCK_FIRST_HERO_GROUP,
	STATE_BLOCK_HERO_GROUP1,
	STATE_BLOCK_HERO_GROUP2,
	STATE_BLOCK_HERO_GROUP3,
	STATE_BLOCK_HERO_GROUP4,
	STATE_BLOCK_HERO_GROUP5,
	STATE_BLOCK_HERO_GROUP6,
	STATE_BLOCK_HERO_GROUP7,
	STATE_BLOCK_HERO_GROUP8,
	STATE_BLOCK_HERO_GROUP9,
	STATE_BLOCK_LAST_HERO_GROUP = STATE_BLOCK_HERO_GROUP9
};

const uint NUM_HERO_LISTS(STATE_BLOCK_LAST_HERO_GROUP - STATE_BLOCK_FIRST_HERO_GROUP + 1);
//=============================================================================

#endif //__GAME_SHARED_PROTOCOL_H__
