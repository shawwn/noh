// (C)2005 S2 Games
// k2_protocol.h
//
// Definitions for client/server communication
//=============================================================================
#ifndef __K2_PROTOCOL_H__
#define __K2_PROTOCOL_H__

//=============================================================================
// Definitions
//=============================================================================
const ushort    DEFAULT_SERVER_PORT     (11235);

const uint      SNAPSHOT_HEADER_SIZE                (sizeof(byte) + sizeof(uint));
const uint      SNAPSHOT_FRAGMENT_HEADER_SIZE       (sizeof(byte) + sizeof(uint) + sizeof(byte));
const uint      SNAPSHOT_TERMINATION_HEADER_SIZE    (sizeof(byte) + sizeof(uint) + sizeof(byte) + sizeof(ushort));

const byte PACKET_NORMAL    (BIT(0));
const byte PACKET_RELIABLE  (BIT(1));
const byte PACKET_ACK       (BIT(2));
const byte PACKET_PRE_ACK   (BIT(3));
const byte PACKET_ERROR     (BIT(4));

enum EServerAccess
{
    ACCESS_PUBLIC = 0,
    ACCESS_INVITEONLY
};

const byte HOST_SERVER_NO_LEAVER(BIT(0));
const byte HOST_SERVER_NO_STATS(BIT(1));

// Reserved state strings
enum ECoreStateStrings
{
    STATE_STRING_NULL = 0,
    STATE_STRING_SERVER_INFO,           // The string sent to out of band "info" requests
    STATE_STRING_CVAR_SETTINGS,         // The values of all cvars flagged with the CVAR_TRANSMIT setting
    STATE_STRING_ENTITIES,              // Strings referenced by entity snapshots
    STATE_STRING_RESOURCES,

    STATE_STRING_NUM_RESERVED
};

// Reserved state blocks
enum ECoreStateBlocks
{
    STATE_BLOCK_NULL = 0,
    STATE_BLOCK_BIT,

    STATE_BLOCK_NUM_RESERVED
};

// Client Connection State
enum EClientConnectionState
{
    CLIENT_CONNECTION_STATE_DISCONNECTED,
    CLIENT_CONNECTION_STATE_CONNECTING,
    CLIENT_CONNECTION_STATE_READY,
    CLIENT_CONNECTION_STATE_STANDBY,
    CLIENT_CONNECTION_STATE_IN_GAME
};

// Server to client commands
const byte NETCMD_AUTH_OKAY                             (0x50); // Server authorized the client's connection successfully
const byte NETCMD_KICK                                  (0x51); // Server denied a connection request
const byte NETCMD_START_STATE_DATA                      (0x52); // Server is sending a complete state data update
const byte NETCMD_END_STATE_DATA                        (0x53); // Server finished sending state data updates
const byte NETCMD_STATE_STRING                          (0x54); // Server updated a state string
const byte NETCMD_COMPRESSED_STATE_STRING               (0x55); // Server updated a compressed state string
const byte NETCMD_STATE_STRING_FRAGMENT                 (0x56); // Part of a state string update that was too large for one packet
const byte NETCMD_STATE_STRING_TERMINATION              (0x57); // Last part of a fragmented state string
const byte NETCMD_COMPRESSED_STATE_STRING_TERMINATION   (0x58); // Last part of a compressed fragmented state string
const byte NETCMD_LOAD_WORLD                            (0x59); // Server loaded this world, the client should too
const byte NETCMD_SNAPSHOT                              (0x5a); // A complete update of the world
const byte NETCMD_COMPRESSED_SNAPSHOT                   (0x5b); // A complete update of the world
const byte NETCMD_SNAPSHOT_FRAGMENT                     (0x5c); // Part of a snapshot that was too large for one packet
const byte NETCMD_SNAPSHOT_TERMINATION                  (0x5d); // Last part of a fragmented snapshot
const byte NETCMD_COMPRESSED_SNAPSHOT_TERMINATION       (0x5e); // Last part of a compressed fragmented snapshot
const byte NETCMD_SERVER_GAME_DATA                      (0x5f); // Data to be processed by the client game code
const byte NETCMD_STATE_BLOCK                           (0x60); // Server updated a state block
const byte NETCMD_COMPRESSED_STATE_BLOCK                (0x61); // Server updated a compressed state block
const byte NETCMD_STATE_BLOCK_FRAGMENT                  (0x62); // Part of a state block update that was too large for one packet
const byte NETCMD_STATE_BLOCK_TERMINATION               (0x63); // Last part of a fragmented state block
const byte NETCMD_COMPRESSED_STATE_BLOCK_TERMINATION    (0x64); // Last part of a fragmented state block
const byte NETCMD_CONSOLE_MESSAGE                       (0x65); // 
const byte NETCMD_SERVER_INFO                           (0x66);
const byte NETCMD_SERVER_KEEP_ALIVE                     (0x67); // Loading heartbeat
const byte NETCMD_START_LOADING                         (0x68);
const byte NETCMD_GAME_HOST                             (0x69);
const byte NETCMD_NEW_VOICE_CLIENT                      (0x6a); // Update old voice users on new voice client
const byte NETCMD_UPDATE_VOICE_CLIENT                   (0x6b); // Update new voice user on old voice clients
const byte NETCMD_REMOVE_VOICE_CLIENT                   (0x6c); // Notify clients to remove another voice client
const byte NETCMD_REMOTE_START_LOADING                  (0x6d);
const byte NETCMD_SERVER_INVITE                         (0x6e); // 
const byte NETCMD_RECONNECT_INFO_RESPONSE               (0x6f);

// Client to server commands
const byte NETCMD_CONNECT                   (0xc0); // Client is requesting to connect
const byte NETCMD_CLIENT_NET_SETTINGS       (0xc1); // Client's network parameters
const byte NETCMD_CLIENT_READY              (0xc2); // Client is waiting for the server to do something
const byte NETCMD_CLIENT_DISCONNECT         (0xc3); // Tell the server that this client is disconnecting
const byte NETCMD_CLIENT_KEEP_ALIVE         (0xc4); // Loading heartbeat
const byte NETCMD_CLIENT_IN_GAME            (0xc5); // Client has received a snapshot and switched it's state
const byte NETCMD_CLIENT_REMOTE_COMMAND     (0xc6); // A console command that the client wishes to execute
const byte NETCMD_CLIENT_SNAPSHOT           (0xc7); // Snapshot of the client's input
const byte NETCMD_CLIENT_GAME_DATA          (0xc8); // Data to be processed by the server game code
const byte NETCMD_CLIENT_HEARTBEAT          (0xc9); // Used when there's nothing else to send   
const byte NETCMD_INFO_REQUEST              (0xca); // OOB request for detailed information about the server
const byte NETCMD_FINISHED_LOADING_WORLD    (0xcb); // Client finished loading a world
const byte NETCMD_RECONNECT_INFO_REQUEST    (0xcc); // 
const byte NETCMD_SET_PRIVATE               (0xcd); // 
const byte NETCMD_LOADING_PROGRESS          (0xce); // 
const byte NETCMD_CLIENT_INVITE             (0xcf); // 
const byte NETCMD_CLIENT_COOKIE             (0xd0); // Request the server to update this clients cookie.

// Manager to Server commands
const byte NETCMD_MANAGER_SLEEP             (0x20); // Zzzzz...
const byte NETCMD_MANAGER_WAKE              (0x21); // Huh? Whut?
const byte NETCMD_MANAGER_SHUTDOWN_SLAVE    (0x22); // Shutdown the instance
const byte NETCMD_MANAGER_RESTART_SLAVE     (0x23); // Restart the instance
const byte NETCMD_MANAGER_CHAT              (0x24); // Talky talk

// Server to Manager commands
const byte NETCMD_MANAGER_INITIALIZED       (0x40); // Inform manager we've finished initializing
const byte NETCMD_MANAGER_SHUTDOWN          (0x41); // Inform manager we've shutdown
const byte NETCMD_MANAGER_STATUS            (0x42); // Per-frame status
const byte NETCMD_MANAGER_LONG_FRAME        (0x43); // Per-frame status
const byte NETCMD_MANAGER_MATCH_START       (0x44); // Match start
const byte NETCMD_MANAGER_MATCH_END         (0x45); // Match end
//=============================================================================

#endif //__K2_PROTOCOL_H__
