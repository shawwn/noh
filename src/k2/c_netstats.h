// (C)2007 S2 Games
// c_netstats.h
//
//=============================================================================
#ifndef __C_NETSTATS_H__
#define __C_NETSTATS_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"
#include "c_material.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum ENetStatMode
{
    NS_TOTALS,
    NS_DETAILS,
    NUM_NETSTAT_MODES
};

enum ENetStatSampleType
{
    NETSAMPLE_SNAPSHOT,
    NETSAMPLE_GAMEDATA,
    NUM_NETSAMPLE_TYPES
};

enum ENetStatSocketType
{
    NETSOCKET_INCOMING,
    NETSOCKET_OUTGOING,
    NUM_NETSOCKET_TYPES
};

const int MAX_NETSTAT_FRAMES(1024);
//=============================================================================

//=============================================================================
// CNetStats
//=============================================================================
class CNetStats
{
private:
    bool            m_bActive;
    bool            m_bDraw;
    bool            m_bRecord;

    ENetStatMode    m_eMode;

    int             m_iFrame[NUM_NETSOCKET_TYPES];

    uint            m_auiFrameTimeStamp[MAX_NETSTAT_FRAMES][NUM_NETSOCKET_TYPES];

    int             m_aiSocketBytes[MAX_NETSTAT_FRAMES][NUM_NETSOCKET_TYPES];
    int             m_aiSampleTypeBytes[MAX_NETSTAT_FRAMES][NUM_NETSOCKET_TYPES][NUM_NETSAMPLE_TYPES];

public:
    CNetStats();

    ~CNetStats();

    K2_API void RecordBytes(int iBytes, ENetStatSampleType eSampleType, ENetStatSocketType eSocketType);
    K2_API void RecordPacket(ENetStatSocketType eSocketType, int iBytes);

    bool    IsActive()                  { return m_bActive; }
    void    SetActive(bool bActive)     { m_bActive = bActive; }
    void    SetDraw(bool bDraw)         { m_bDraw = bDraw; }
    bool    GetRecord()                 { return m_bRecord; }
    void    SetRecord(bool bRecord)     { m_bRecord = bRecord; }

    K2_API void Frame();
    K2_API void Draw();
};

extern K2_API CNetStats NetStats;
//=============================================================================

#endif // __C_NETSTATS_H__
