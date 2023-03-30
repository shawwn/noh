// (C)2005 S2 Games
// server_api.h
//
// Contains all functions exported from a standard server dll
//=============================================================================
#ifndef __SERVER_API_H__
#define __SERVER_API_H__

#ifdef SERVER_EXPORTS
#define SERVER_API extern "C" __declspec(dllexport)
#else
#define SERVER_API extern "C"
#endif

//=============================================================================
// Headers
//=============================================================================
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CWorld;
class CPacket;
class IBuffer;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef void (*serverInitialize_t)(CWorld &World); 
typedef void (*serverFrame_t)(int iGameTime);
typedef void (*serverProcessClientInput_t)(CPacket &pkt);
typedef bool (*serverWriteFrameUpdate_t)(CPacket &pkt, int iServerTime);
typedef bool (*serverPacket_t)(byte cmd, CPacket &pkt);
typedef void (*serverShutdown_t)();
typedef void (*serverAddClient_t)(int iClientNum, IBuffer &LoadData);
typedef void (*serverRemoveClient_t)(int iClientNum, IBuffer &SaveData);

struct SServerAPI
{
    serverInitialize_t          Initialize;         // Called when the server starts
    serverFrame_t               Frame;              // Called at a fixed interval while the servver runs
    serverAddClient_t           AddClient;          // Called when a new client connects
    serverRemoveClient_t        RemoveClient;       // Called when a client disconnects
    serverProcessClientInput_t  ProcessClientInput; // Called when an input state is received
    serverWriteFrameUpdate_t    WriteFrameUpdate;   // Called for each client every frame
    serverPacket_t              Packet;             // Called for any packet that core does not handle
    serverShutdown_t            Shutdown;           // Called when the server is killed
};
//=============================================================================
#endif
