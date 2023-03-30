// (C)2005 S2 Games
// client_api.h
//
// Contains all functions exported from a standard client dll
//=============================================================================
#ifndef __CLIENT_API_H__
#define __CLIENT_API_H__

#ifdef CLIENT_EXPORTS
#define CLIENT_API extern "C" __declspec(dllexport)
#else
#define CLIENT_API extern "C"
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
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef void (*clientInitialize_t)(void); 
typedef void (*clientStart_t)(CWorld &World);
typedef void (*clientFrame_t)(int iGameTime);
typedef bool (*clientReadFrameUpdate_t)(CPacket &pkt, int iServerTime);
typedef bool (*clientPacket_t)(byte cmd, CPacket &pkt);
typedef void (*clientShutdown_t)();

struct SClientAPI
{
    clientInitialize_t      Initialize;
    clientStart_t           Start;
    clientFrame_t           Frame;
    clientReadFrameUpdate_t ReadFrameUpdate;
    clientPacket_t          Packet;
    clientShutdown_t        Shutdown;
};
//=============================================================================
#endif
