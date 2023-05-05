// (C)2008 S2 Games
// c_servermanager.h
//
//=============================================================================
#ifndef __C_SERVERMANAGER_H__
#define __C_SERVERMANAGER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_socket.h"

#include "chatserver_protocol.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
extern K2_API class CServerManager *g_pServerManager;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#ifdef K2_EXPORTS
#define ServerManager (*CServerManager::GetInstance())
#else
#define ServerManager (*g_pServerManager)
#endif

struct SServerLoad
{
    uint    uiTime;
    uint    uiCpuLoad;
    uint    uiBytesSent;
    uint    uiPacketsSent;
    uint    uiBytesDropped;
    uint    uiPacketsDropped;
    uint    uiBytesReceived;
    uint    uiPacketsReceived;
    uint    uiMemUsage;
};

struct SServerStatus
{
    int                 iSlave;
    tstring             sAddress;
    ushort              wPort;
    uint                uiAffinity;
    byte                yExpectedServerStatus;
    byte                yServerStatus;
    bool                bMatchStarted;
    deque<SServerLoad>  deqLoad;
    SServerLoad         cLoad;
    uint                uiMatchID;
    int                 iNumClients;
    tstring             sMap;
    tstring             sGameName;
    uint                uiLastResponse;
    uint                uiLastChance;
};

struct SActiveUpload
{
    tstring             sHost;
    tstring             sFilename;
    tstring             sTarget;
    CFileHandle         hFile;
    int                 iRetries;
    float               fProgress;
    bool                bStarted;
};

typedef map<tstring, SServerStatus>     SlaveServerMap;
typedef deque<SActiveUpload>            ReplayUploadDeque;

enum EManLogEvent
{
    MAN_LOG_INFO_DATE = 1,
    MAN_LOG_INFO_GAME,

    MAN_LOG_SLAVE_START,
    MAN_LOG_SLAVE_INITIALIZED,
    MAN_LOG_SLAVE_SHUTDOWN,
    MAN_LOG_SLAVE_WAKE,
    MAN_LOG_SLAVE_SLEEP,
    MAN_LOG_SLAVE_UNRESPONSIVE,
    MAN_LOG_SLAVE_TERMINATE,
    MAN_LOG_SLAVE_TERMINATE2,
    MAN_LOG_SLAVE_TERMINATE3,
    MAN_LOG_SLAVE_LATE_RESPONSE,
    MAN_LOG_SLAVE_LATE_RESPONSE2,
    MAN_LOG_SLAVE_STATUS,
    MAN_LOG_SLAVE_LONG_FRAME,
    
    MAN_LOG_MATCH_START,
    MAN_LOG_MATCH_END,

    MAN_LOG_UPDATE,
    MAN_LOG_SHUTDOWN_SLAVES,
};
static const EManLogEvent MAN_LOG_INVALID(EManLogEvent(0));


inline EManLogEvent GetManLogEventFromString(const tstring &sEvent)
{
    if (sEvent == _T("INFO_DATE")) return MAN_LOG_INFO_DATE;
    else if (sEvent == _T("INFO_GAME")) return MAN_LOG_INFO_GAME;

    else if (sEvent == _T("SLAVE_START")) return MAN_LOG_SLAVE_START;
    else if (sEvent == _T("SLAVE_INITIALIZED")) return MAN_LOG_SLAVE_INITIALIZED;
    else if (sEvent == _T("SLAVE_SHUTDOWN")) return MAN_LOG_SLAVE_SHUTDOWN;
    else if (sEvent == _T("SLAVE_WAKE")) return MAN_LOG_SLAVE_WAKE;
    else if (sEvent == _T("SLAVE_SLEEP")) return MAN_LOG_SLAVE_SLEEP;
    else if (sEvent == _T("SLAVE_UNRESPONSIVE")) return MAN_LOG_SLAVE_UNRESPONSIVE;
    else if (sEvent == _T("SLAVE_TERMINATE")) return MAN_LOG_SLAVE_TERMINATE;
    else if (sEvent == _T("SLAVE_TERMINATE2")) return MAN_LOG_SLAVE_TERMINATE2;
    else if (sEvent == _T("SLAVE_TERMINATE3")) return MAN_LOG_SLAVE_TERMINATE3;
    else if (sEvent == _T("SLAVE_LATE_RESPONSE")) return MAN_LOG_SLAVE_LATE_RESPONSE;
    else if (sEvent == _T("SLAVE_LATE_RESPONSE2")) return MAN_LOG_SLAVE_LATE_RESPONSE2;
    else if (sEvent == _T("SLAVE_STATUS")) return MAN_LOG_SLAVE_STATUS;
    else if (sEvent == _T("SLAVE_LONG_FRAME")) return MAN_LOG_SLAVE_LONG_FRAME;

    else if (sEvent == _T("MATCH_START")) return MAN_LOG_MATCH_START;
    else if (sEvent == _T("MATCH_END")) return MAN_LOG_MATCH_END;

    else if (sEvent == _T("UPDATE")) return MAN_LOG_UPDATE;
    else if (sEvent == _T("SHUTDOWN_SLAVES")) return MAN_LOG_SHUTDOWN_SLAVES;

    return MAN_LOG_INVALID;
}

inline EManLogEvent&    AtoX(const tstring &s, EManLogEvent &e) { return e = GetManLogEventFromString(s); }

inline tstring  GetManLogEventName(EManLogEvent eEvent)
{
    switch (eEvent)
    {
    case MAN_LOG_INFO_DATE: return _T("INFO_DATE");
    case MAN_LOG_INFO_GAME: return _T("INFO_GAME");

    case MAN_LOG_SLAVE_START: return _T("SLAVE_START");
    case MAN_LOG_SLAVE_INITIALIZED: return _T("SLAVE_INITIALIZED");
    case MAN_LOG_SLAVE_SHUTDOWN: return _T("SLAVE_SHUTDOWN");
    case MAN_LOG_SLAVE_WAKE: return _T("SLAVE_WAKE");
    case MAN_LOG_SLAVE_SLEEP: return _T("SLAVE_SLEEP");
    case MAN_LOG_SLAVE_UNRESPONSIVE: return _T("SLAVE_UNRESPONSIVE");
    case MAN_LOG_SLAVE_TERMINATE: return _T("SLAVE_TERMINATE");
    case MAN_LOG_SLAVE_TERMINATE2: return _T("SLAVE_TERMINATE2");
    case MAN_LOG_SLAVE_TERMINATE3: return _T("SLAVE_TERMINATE3");
    case MAN_LOG_SLAVE_LATE_RESPONSE: return _T("SLAVE_LATE_RESPONSE");
    case MAN_LOG_SLAVE_LATE_RESPONSE2: return _T("SLAVE_LATE_RESPONSE2");
    case MAN_LOG_SLAVE_STATUS: return _T("SLAVE_STATUS");
    case MAN_LOG_SLAVE_LONG_FRAME: return _T("SLAVE_LONG_FRAME");

    case MAN_LOG_MATCH_START: return _T("MATCH_START");
    case MAN_LOG_MATCH_END: return _T("MATCH_END");

    case MAN_LOG_UPDATE: return _T("UPDATE");
    case MAN_LOG_SHUTDOWN_SLAVES: return _T("SHUTDOWN_SLAVES");

    default: return _T("INVALID");
    }
}
//=============================================================================

//=============================================================================
// CManagerLog
//=============================================================================
class CManagerLog
{
private:
    CFileHandle     m_hLogFile;

public:
    ~CManagerLog()  {}
    CManagerLog()   {}

    void    Open();
    void    Close()                     { m_hLogFile.Close(); }

    const tstring&  GetPath() const     { return m_hLogFile.GetPath(); }

    void    WriteInfo(EManLogEvent eEvent, const tstring &sTagA = TSNULL, const tstring &sValueA = TSNULL, const tstring &sTagB = TSNULL, const tstring &sValueB = TSNULL);
    void    WriteSlave(EManLogEvent eEvent, SServerStatus &cSlave, const tstring &sParamA = TSNULL, const tstring &sParamB = TSNULL);
};
//=============================================================================

//=============================================================================
// CServerManager
//=============================================================================
class CServerManager
{
    SINGLETON_DEF(CServerManager)

private:
    CHTTPManager*       m_pHTTPManager;
    CSocket             m_cSocket;

    string              m_sMasterServerURL;

    SlaveServerMap      m_mapServers;
    ReplayUploadDeque   m_deqFileUploads;

    uint                m_auiAffinity[32];
    uint                m_uiNumAffinities;

    uint                m_uiLastUpdateCheck;

    bool                m_bUpdateAvailable;
    bool                m_bUpdating;
    bool                m_bUpdateComplete;
    tstring             m_sUpdateVersion;
    bool                m_bShutdownInstances;
    bool                m_bReset;
    uint                m_uiLastLogTime;
    deque<uint>         m_deqServerStart;

    CManagerLog         m_Log;

    void        SetCvar(tstring &sConfig, const tstring &sName, const tstring &sValue);

    void        PerformUpdate();
    bool        SendManagerAuth();
    
public:
    ~CServerManager();

    void        Initialize(CHTTPManager *pHTTPManager);
    void        Frame();
    void        Shutdown();

    void        StartInstance(uint uiAffinity);
    void        PrintStatus();
    void        PrintUploadStatus();

    void        UpdateAvailable(const tstring &sVersion);
    void        UpdateComplete();

    void        ShutdownInstances();
    void        StartShutdown();
    void        StartReset();

    void        Chat(const tstring &sMsg);

    void        UploadFile(const tstring &sHost, const tstring &sFilename, const tstring &sTarget);
};
//=============================================================================

#endif //__C_SERVERMANAGER_H__
