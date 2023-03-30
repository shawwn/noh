// (C)2006 S2 Games
// game_shared_protocol.h
//
//=============================================================================
#ifndef __GAME_SHARED_PROTOCOL_H__
#define __GAME_SHARED_PROTOCOL_H__

//=============================================================================
// Definitions
//=============================================================================
const byte GAME_CMD_CHANGE_UNIT             (0x01);
const byte GAME_CMD_CHANGE_TEAM             (0x02);
const byte GAME_CMD_CHAT_ALL                (0x03);
const byte GAME_CMD_CHAT_TEAM               (0x04);
const byte GAME_CMD_CHAT_SQUAD              (0x05);
const byte GAME_CMD_SERVERCHAT_ALL          (0x06);
const byte GAME_CMD_SPEND_POINT             (0x07);
const byte GAME_CMD_PLACE_BUILDING          (0x08);
const byte GAME_CMD_POS_COMMAND             (0x09);
const byte GAME_CMD_ENT_COMMAND             (0x0a);
const byte GAME_CMD_SPAWN                   (0x0b);
const byte GAME_CMD_REMOTE                  (0x0c);
const byte GAME_CMD_PURCHASE                (0x0d);
const byte GAME_CMD_SELL                    (0x0e);
//const byte GAME_CMD_TEAM_LIST             (0x0f);
const byte GAME_CMD_SET_UPKEEP              (0x10);
const byte GAME_CMD_PROMOTE_OFFICER         (0x11);
const byte GAME_CMD_DEMOTE_OFFICER          (0x12);
const byte GAME_CMD_VOTE                    (0x13);
const byte GAME_CMD_REQUEST_COMMAND         (0x14);
const byte GAME_CMD_DECLINE_OFFICER         (0x15);
const byte GAME_CMD_SPAWN_SELECT            (0x16);
const byte GAME_CMD_JOIN_SQUAD              (0x17);
const byte GAME_CMD_PURCHASE_PERSISTANT     (0x18);
const byte GAME_CMD_MESSAGE                 (0x19);
const byte GAME_CMD_REWARD                  (0x1a);
const byte GAME_CMD_HITFEEDBACK             (0x1b);
const byte GAME_CMD_MINIMAP_DRAW            (0x1c);
const byte GAME_CMD_SACRIFICE               (0x1d);
const byte GAME_CMD_SPEND_TEAM_POINT        (0x1e);
const byte GAME_CMD_MINIMAP_PING            (0x1f);
const byte GAME_CMD_COMMANDER_RESIGN        (0x20);
const byte GAME_CMD_BUILDING_ATTACK_ALERT   (0x21);
const byte GAME_CMD_PETCMD                  (0x22);
const byte GAME_CMD_PETCMD_ENT              (0x23);
const byte GAME_CMD_PETCMD_POS              (0x24);
const byte GAME_CMD_OFFICERCMD              (0x25);
const byte GAME_CMD_OFFICERCMD_ENT          (0x26);
const byte GAME_CMD_OFFICERCMD_POS          (0x27);
const byte GAME_CMD_END_GAME_TERMINATION    (0x28);
const byte GAME_CMD_END_GAME_FRAGMENT       (0x29);
const byte GAME_CMD_SWAP_INVENTORY          (0x2a);
const byte GAME_CMD_SUBMIT_REPLAY_COMMENT   (0x2b);
const byte GAME_CMD_HELLSHRINE_BUILDING     (0x2c);
const byte GAME_CMD_MALPHAS_SPAWN           (0x2d);
const byte GAME_CMD_CONSTRUCTION_COMPLETE   (0x2e);
const byte GAME_CMD_GOLD_MINE_LOW           (0x2f);
const byte GAME_CMD_GOLD_MINE_DEPLETED      (0x30);
const byte GAME_CMD_PERSISTANT_ITEMS        (0x31);
const byte GAME_CMD_VOICE_DATA              (0x32);
const byte GAME_CMD_VOICE_REMOVECLIENT      (0x33);
const byte GAME_CMD_VOICE_STOPTALKING       (0x34);
const byte GAME_CMD_VOICE_STARTTALKING      (0x35);
const byte GAME_CMD_VOICECOMMAND            (0x36);
const byte GAME_CMD_STOP_BUILDING           (0x37);
const byte GAME_CMD_EXEC_SCRIPT             (0x38);
const byte GAME_CMD_SCRIPT_INPUT            (0x39);
const byte GAME_CMD_PETCMD_ORDERCONFIRMED   (0x3a);
const byte GAME_CMD_SET_REPLAY_CLIENT       (0x3b);
const byte GAME_CMD_END_GAME                (0x3c);
const byte GAME_CMD_CONTRIBUTE              (0x3d);
const byte GAME_CMD_CONSTRUCTION_STARTED    (0x3e);
const byte GAME_CMD_BUILDING_DESTROYED      (0x3f);
const byte GAME_CMD_SPAWNFLAG_PLACED        (0x40);
const byte GAME_CMD_KILL_NOTIFICATION       (0x41);
const byte GAME_CMD_ASSIST_NOTIFICATION     (0x42);
const byte GAME_CMD_RAZED_NOTIFICATION      (0x43);
const byte GAME_CMD_CANCEL_SACRIFICE        (0x44);
const byte GAME_CMD_SERVER_STATUS           (0x45);
const byte GAME_CMD_CONSOLE_MESSAGE         (0x46);
const byte GAME_CMD_DEATH_MESSAGE           (0x47);
const byte GAME_CMD_SPAWN_WORKER            (0x48);
const byte GAME_CMD_PICKUP_ITEM             (0x49);
const byte GAME_CMD_SCRIPT_MESSAGE          (0x4A);
const byte GAME_CMD_START_BUILDING          (0x4B);
const byte GAME_CMD_SERVER_STATS            (0x4C);
const byte GAME_CMD_SUBMIT_COMMANDER_RATING (0x4D);
const byte GAME_CMD_SUBMIT_KARMA_RATING     (0x4E);
const byte GAME_CMD_END_GAME_TIME           (0x4F);
const byte GAME_CMD_GADGET_ACCESSED         (0x50);
const byte GAME_CMD_CANCEL_SPAWN            (0x51);
const byte GAME_CMD_CONSOLE_EXECUTE         (0x52);
const byte GAME_CMD_REPAIRABLE              (0x53);
const byte GAME_CMD_ITEM_FAILED             (0x54);
const byte GAME_CMD_ITEM_SUCCEEDED          (0x55);
const byte GAME_CMD_BLD_HEALTH_LOW          (0x56);
const byte GAME_CMD_KILLSTREAK              (0x57);
const byte GAME_CMD_GADGET_ALARM            (0x58);
//=============================================================================

#endif //__GAME_SHARED_PROTOCOL_H__
