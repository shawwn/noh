// (C)2008 S2 Games
// c_servermanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_servermanager.h"

#include "c_hostserver.h"
#include "c_filehttp.h"
#include "c_updater.h"
#include "c_date.h"
#include "md5.h"
#include "c_phpdata.h"
#include "c_httpmanager.h"
#include "c_httprequest.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_INTR(man_port,                     1135,                           CVAR_SAVECONFIG,    1024,   65535);
CVAR_INTR(man_startServerPort,          DEFAULT_SERVER_PORT,            CVAR_SAVECONFIG,    1024,   65535);
CVAR_INTR(man_endServerPort,            DEFAULT_SERVER_PORT + 100,      CVAR_SAVECONFIG,    1024,   65535);
CVAR_INTF(man_autoServersPerCPU,        0,                              CVAR_SAVECONFIG);
CVAR_INTF(man_maxServers,               -1,                             CVAR_SAVECONFIG);
CVAR_STRINGF(man_allowCPUs,             "",                             CVAR_SAVECONFIG);
CVAR_STRINGF(man_masterLogin,           "",                             CVAR_SAVECONFIG);
CVAR_STRINGF(man_masterPassword,        "",                             CVAR_SAVECONFIG);
CVAR_INTF(man_numSlaveAccounts,         10,                             CVAR_SAVECONFIG);
CVAR_BOOLF(man_broadcastSlaves,         true,                           CVAR_SAVECONFIG);
CVAR_UINTF(man_idleTarget,              1,                              CVAR_SAVECONFIG);
//CVAR_BOOLF(man_requireAuthentication, true,                           CVAR_SAVECONFIG);
CVAR_UINTF(man_logPeriod,               5000,                           CVAR_SAVECONFIG);
#if 0
CVAR_UINTF(man_slaveTimeout,            29000,                          CVAR_SAVECONFIG);
CVAR_UINTF(man_slaveTimeoutLastChance,  1000,                           CVAR_SAVECONFIG);
CVAR_UINTF(man_slaveLoadTimeout,        120000,                         CVAR_SAVECONFIG);
#else
const uint man_slaveTimeout(59000u);
const uint man_slaveTimeoutLastChance(3000u);
const uint man_slaveLoadTimeout(240000u);
#endif

#define STAT_LABEL(name)        (_T(" ") _T(#name) _T(":"))
#define WRITE_STAT(name, value) m_hLogFile << STAT_LABEL(name) << (value)
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CServerManager *g_pServerManager(CServerManager::GetInstance());

SINGLETON_INIT(CServerManager)
//=============================================================================

/*====================
  CManagerLog::Open
  ====================*/
void    CManagerLog::Open()
{
    tstring sLogFilename(FileManager.GetNextFileIncrement(4, _T("~/logs/manager/") + svr_name + _T(" - "), _T("log")));
    m_hLogFile.Open(sLogFilename, FILE_WRITE | FILE_TEXT);
}


/*====================
  CManagerLog::WriteInfo
  ====================*/
void    CManagerLog::WriteInfo(EManLogEvent eEvent, const tstring &sTagA, const tstring &sValueA, const tstring &sTagB, const tstring &sValueB)
{
    if (!m_hLogFile.IsOpen())
        return;

    m_hLogFile << GetManLogEventName(eEvent);
    if (!sTagA.empty() && !sValueA.empty())
        m_hLogFile << SPACE << sTagA << _T(":") << QuoteStr(sValueA);
    if (!sTagB.empty() && !sValueB.empty())
        m_hLogFile << SPACE << sTagB << _T(":") << QuoteStr(sValueB);
    m_hLogFile << newl;
}


/*====================
  CManagerLog::WriteSlave
  ====================*/
void    CManagerLog::WriteSlave(EManLogEvent eEvent, SServerStatus &cSlave, const tstring &sParamA, const tstring &sParamB)
{
    if (!m_hLogFile.IsOpen())
        return;

    m_hLogFile << GetManLogEventName(eEvent);
    WRITE_STAT(time, Host.GetSystemTime());
    WRITE_STAT(slave, cSlave.iSlave);
    switch (eEvent)
    {
    case MAN_LOG_SLAVE_START:
        WRITE_STAT(address, QuoteStr(cSlave.sAddress));
        WRITE_STAT(affinity, cSlave.uiAffinity);
        break;

    case MAN_LOG_INFO_DATE:
    case MAN_LOG_INFO_GAME:
    case MAN_LOG_UPDATE:
    case MAN_LOG_SHUTDOWN_SLAVES:
    case MAN_LOG_SLAVE_INITIALIZED:
    case MAN_LOG_SLAVE_SHUTDOWN:
    case MAN_LOG_SLAVE_WAKE:
    case MAN_LOG_SLAVE_SLEEP:
    case MAN_LOG_SLAVE_UNRESPONSIVE:
    case MAN_LOG_SLAVE_TERMINATE:
    case MAN_LOG_SLAVE_TERMINATE2:
    case MAN_LOG_SLAVE_TERMINATE3:
    case MAN_LOG_SLAVE_LATE_RESPONSE:
    case MAN_LOG_SLAVE_LATE_RESPONSE2:
        break;

    case MAN_LOG_SLAVE_STATUS:
        {
            WRITE_STAT(clients, cSlave.iNumClients);

            uint uiLoad(0);
            for (deque<SServerLoad>::iterator itLoad(cSlave.deqLoad.begin()); itLoad != cSlave.deqLoad.end(); ++itLoad)
                uiLoad += itLoad->uiCpuLoad;

            WRITE_STAT(cpuload, XtoA(uiLoad / 5000000.0f, 0, 0, 4));
            WRITE_STAT(mem_usage, cSlave.cLoad.uiMemUsage);
            WRITE_STAT(out_bytes, cSlave.cLoad.uiBytesSent);
            WRITE_STAT(out_packets, cSlave.cLoad.uiPacketsSent);
            WRITE_STAT(out_bytes_dropped, cSlave.cLoad.uiBytesDropped);
            WRITE_STAT(out_packets_dropped, cSlave.cLoad.uiPacketsDropped);
            WRITE_STAT(in_bytes, cSlave.cLoad.uiBytesReceived);
            WRITE_STAT(in_packets, cSlave.cLoad.uiPacketsReceived);
        }
        break;

    case MAN_LOG_SLAVE_LONG_FRAME:
        WRITE_STAT(length, sParamA);
        break;

    case MAN_LOG_MATCH_START:
        WRITE_STAT(matchid, cSlave.uiMatchID);
        WRITE_STAT(name, QuoteStr(cSlave.sGameName));
        WRITE_STAT(map, cSlave.sMap);
        break;

    case MAN_LOG_MATCH_END:
        break;
    }

    m_hLogFile << newl;
}


/*====================
  CServerManager::~CServerManager
  ====================*/
CServerManager::~CServerManager()
{
}


/*====================
  CServerManager::CServerManager
  ====================*/
CServerManager::CServerManager() :
m_pHTTPManager(nullptr),
m_cSocket(_T("SERVER_MANAGER")),
m_uiNumAffinities(0),
m_uiLastUpdateCheck(INVALID_TIME),
m_bUpdateAvailable(false),
m_bUpdating(false),
m_bUpdateComplete(false),
m_bShutdownInstances(false),
m_bReset(false),
m_uiLastLogTime(INVALID_TIME)
{
}


/*====================
  CServerManager::Initialize
  ====================*/
void    CServerManager::Initialize(CHTTPManager *pHTTPManager)
{
    m_pHTTPManager = pHTTPManager;
    
    m_sMasterServerURL = K2System.GetMasterServerAddress() + "/server_requester.php";

    CDate date(true);

    m_Log.Open();

    m_Log.WriteInfo(MAN_LOG_INFO_DATE, _T("date"), date.GetDateString(), _T("time"), date.GetTimeString());
    m_Log.WriteInfo(MAN_LOG_INFO_GAME, _T("name"), K2System.GetGameName(), _T("version"), K2System.GetVersionString());

    if (!m_cSocket.Init(K2_SOCKET_GAME, man_port, false, uint(-1), uint(-1)))
    {
        Console.Err << _T("Couldn't open port ") << man_port << newl;
        return;
    }

    tsvector vsAllowAffinities(TokenizeString(man_allowCPUs, _T(',')));
    uiset setAllowAffinites;

    for (tsvector_it it(vsAllowAffinities.begin()); it != vsAllowAffinities.end(); ++it)
        setAllowAffinites.insert(AtoUI(*it));

    uint uiAffinityMask(K2System.GetAffinityMask());
    for (uint uiCPU(0); uiCPU < 32; ++uiCPU)
    {
        if (!setAllowAffinites.empty() && setAllowAffinites.find(uiCPU) == setAllowAffinites.end())
            continue;

        if (BIT(uiCPU) & uiAffinityMask)
            m_auiAffinity[m_uiNumAffinities++] = uiCPU;
    }

    for (int iInstance(0); iInstance < man_autoServersPerCPU; ++iInstance)
    {
        for (uint ui(0); ui < m_uiNumAffinities; ++ui)
        {
            StartInstance(m_auiAffinity[ui]);
        }
    }

    while (!SendManagerAuth())
        K2System.Sleep(1000);
}


/*====================
  CServerManager::SendManagerAuth
  ====================*/
bool    CServerManager::SendManagerAuth()
{
#ifdef K2_CLIENT
    return true;
#else
    CHTTPRequest *pAuthRequest(m_pHTTPManager->SpawnRequest());
    if (pAuthRequest == nullptr)
        return false;

    pAuthRequest->SetTargetURL(m_sMasterServerURL);
    pAuthRequest->AddVariable(_T("f"), _T("replay_auth"));
    pAuthRequest->AddVariable(_T("login"), man_masterLogin);
    pAuthRequest->AddVariable(_T("pass"), MD5String(TStringToUTF8(man_masterPassword)));
    pAuthRequest->SendPostRequest();
    pAuthRequest->Wait();

    CPHPData phpResponse(pAuthRequest->GetResponse());
    phpResponse.Print();

    return pAuthRequest->WasSuccessful();
#endif
}


/*====================
  CServerManager::Frame
  ====================*/
void    CServerManager::Frame()
{
    if (m_bUpdating)
        return;

    CPacket pkt;
    while (m_cSocket.ReceivePacket(pkt) > 0)
    {
        // Handle packets
        bool bProcessed(false);
        byte cmd(pkt.ReadByte());
        tstring sAddr(m_cSocket.GetRecvAddrName() + _T(":") + XtoA(m_cSocket.GetRecvPort()));

        SlaveServerMap::iterator itServer(m_mapServers.find(sAddr));
        if (itServer == m_mapServers.end())
        {
            Console.Net << _T("Packet received from unknown server ") << sAddr
                << _T(", cmd ") << XtoA(cmd, FMT_PADZERO, 4, 16) << newl;
            continue;
        }

        switch (cmd)
        {
        case NETCMD_MANAGER_INITIALIZED:
            {
                Console.Std << _T("Slave ") << itServer->first << _T(" initialized") << newl;
                m_Log.WriteSlave(MAN_LOG_SLAVE_INITIALIZED, itServer->second);

                bProcessed = true;
            }
            break;

        case NETCMD_MANAGER_SHUTDOWN:
            {
                Console.Std << _T("Slave ") << itServer->first << _T(" shutdown") << newl;
                m_Log.WriteSlave(MAN_LOG_SLAVE_SHUTDOWN, itServer->second);

                if (itServer->second.yServerStatus != SERVER_STATUS_KILLED)
                    m_mapServers.erase(itServer);

                bProcessed = true;
            }
            break;

        case NETCMD_MANAGER_MATCH_START:
            {
                uint uiMatchID(pkt.ReadInt(-1));
                tstring sMap(pkt.ReadWStringAsTString());
                tstring sGameName(pkt.ReadWStringAsTString());
                tstring sGameMode(pkt.ReadWStringAsTString());
                pkt.ReadByte(0); // byte yTeamSize

                Console.Std << _T("Slave ") << itServer->first << _T(" match start [") << (uiMatchID == uint(-1) ? _T("Local") : XtoA(uiMatchID)) << _T("]") << newl;

                itServer->second.uiMatchID = uiMatchID;
                itServer->second.sMap = sMap;
                itServer->second.sGameName = sGameName;

                m_Log.WriteSlave(MAN_LOG_MATCH_START, itServer->second);

                bProcessed = true;
            }
            break;

        case NETCMD_MANAGER_MATCH_END:
            {               
                Console.Std << _T("Slave ") << itServer->first << _T(" match end [") << (itServer->second.uiMatchID == uint(-1) ? _T("Local") : XtoA(itServer->second.uiMatchID)) << _T("]") << newl;

                tstring sReplayHost(pkt.ReadWStringAsTString());
                tstring sReplayDir(pkt.ReadWStringAsTString());
                tstring sReplayFilename(pkt.ReadWStringAsTString());
                tstring sLogFilename(pkt.ReadWStringAsTString());

                m_Log.WriteSlave(MAN_LOG_MATCH_END, itServer->second);

                {
                    tstring sLogZip(Filename_StripExtension(sLogFilename) + _T(".zip"));

                    CFileHandle hLog(sLogFilename, FILE_READ | FILE_BINARY);
                    if (hLog.IsOpen())
                    {
                        uint uiSize;
                        const char *pBuffer(hLog.GetBuffer(uiSize));
                        CArchive archive;

                        if (archive.Open(sLogZip, ARCHIVE_WRITE | ARCHIVE_MAX_COMPRESS) && archive.WriteFile(Filename_StripPath(sLogFilename), pBuffer, uiSize))
                        {
                            archive.Close();

                            SActiveUpload cUpload;
                            cUpload.sHost = sReplayHost;
                            cUpload.sFilename = sLogZip;
                            cUpload.sTarget = sReplayDir + _T("/") + Filename_StripPath(sLogZip);

                            cUpload.fProgress = 0.0f;
                            cUpload.iRetries = 0;
                            cUpload.bStarted = false;

                            m_deqFileUploads.push_back(cUpload);
                        }
                        else
                        {
                            archive.Close();
                            FileManager.Delete(sLogZip);
                        }
                    }
                }

                {
                    SActiveUpload cUpload;
                    cUpload.sHost = sReplayHost;
                    cUpload.sFilename = sReplayFilename;
                    cUpload.sTarget = sReplayDir + _T("/") + Filename_StripPath(sReplayFilename);

                    cUpload.fProgress = 0.0f;
                    cUpload.iRetries = 0;
                    cUpload.bStarted = false;

                    m_deqFileUploads.push_back(cUpload);
                }

                itServer->second.sGameName.clear();
                itServer->second.sMap.clear();
    
                bProcessed = true;
            }
            break;

        case NETCMD_MANAGER_STATUS:
            {
                //Console.Std << _T("Slave ") << itServer->first << _T(" status") << newl;

                byte yServerStatus(pkt.ReadByte(SERVER_STATUS_SLEEPING));
                uint uiSystemTime(pkt.ReadInt(0));
                uint uiCpuLoad(pkt.ReadInt(0));
                byte yNumClients(pkt.ReadByte(0));
                bool bMatchStarted(pkt.ReadByte(0) != 0);
                uint uiBytesSent(pkt.ReadInt(0));
                uint uiPacketsSent(pkt.ReadInt(0));
                uint uiBytesDropped(pkt.ReadInt(0));
                uint uiPacketsDropped(pkt.ReadInt(0));
                uint uiBytesReceived(pkt.ReadInt(0));
                uint uiPacketsReceived(pkt.ReadInt(0));
                uint uiMemUsage(pkt.ReadInt(0));

                if (itServer->second.yServerStatus == SERVER_STATUS_CRASHED)
                {
                    m_Log.WriteSlave(MAN_LOG_SLAVE_LATE_RESPONSE, itServer->second);

                    Console << _T("Slave #") << itServer->second.iSlave << _T(" responded after being unresponsive, attempting to terminate") << newl;

                    CPacket pkt;
                    pkt.WriteByte(NETCMD_MANAGER_SHUTDOWN_SLAVE);

                    m_cSocket.SetSendAddr(itServer->first);
                    m_cSocket.SendPacket(pkt);

                    itServer->second.yServerStatus = SERVER_STATUS_KILLED;

                    bProcessed = true;
                    break;
                }

                if (itServer->second.yServerStatus == SERVER_STATUS_KILLED || itServer->second.yExpectedServerStatus == SERVER_STATUS_KILLED)
                {
                    Console << _T("Slave #") << itServer->second.iSlave << _T(" responded after being terminated, attempting to terminate") << newl;

                    CPacket pkt;
                    pkt.WriteByte(NETCMD_MANAGER_SHUTDOWN_SLAVE);

                    m_cSocket.SetSendAddr(itServer->first);
                    m_cSocket.SendPacket(pkt);

                    bProcessed = true;
                    break;
                }

                if (itServer->second.yExpectedServerStatus == SERVER_STATUS_IDLE && yServerStatus == SERVER_STATUS_ACTIVE)
                    itServer->second.yExpectedServerStatus = SERVER_STATUS_ACTIVE;
                else if (itServer->second.yExpectedServerStatus == SERVER_STATUS_ACTIVE && yServerStatus == SERVER_STATUS_IDLE)
                    itServer->second.yExpectedServerStatus = SERVER_STATUS_IDLE;

                itServer->second.yServerStatus = yServerStatus;
                itServer->second.bMatchStarted = bMatchStarted;
                itServer->second.iNumClients = yNumClients;

                SServerLoad cLoad;
                cLoad.uiTime = uiSystemTime;
                cLoad.uiCpuLoad = uiCpuLoad;
                cLoad.uiBytesSent = uiBytesSent;
                cLoad.uiPacketsSent = uiPacketsSent;
                cLoad.uiBytesDropped = uiBytesDropped;
                cLoad.uiPacketsDropped = uiPacketsDropped;
                cLoad.uiBytesReceived = uiBytesReceived;
                cLoad.uiPacketsReceived = uiPacketsReceived;
                cLoad.uiMemUsage = uiMemUsage;
                
                itServer->second.deqLoad.push_back(cLoad);

                while (!itServer->second.deqLoad.empty() && itServer->second.deqLoad.front().uiTime + man_logPeriod < uiSystemTime)
                    itServer->second.deqLoad.pop_front();

                itServer->second.cLoad.uiTime = uiSystemTime;
                itServer->second.cLoad.uiBytesSent += uiBytesSent;
                itServer->second.cLoad.uiPacketsSent += uiPacketsSent;
                itServer->second.cLoad.uiBytesDropped += uiBytesDropped;
                itServer->second.cLoad.uiPacketsDropped += uiPacketsDropped;
                itServer->second.cLoad.uiBytesReceived += uiBytesReceived;
                itServer->second.cLoad.uiPacketsReceived += uiPacketsReceived;
                itServer->second.cLoad.uiMemUsage = uiMemUsage;

                itServer->second.uiLastResponse = K2System.Milliseconds();
                itServer->second.uiLastChance = INVALID_TIME;

                //Console.Std << XtoA(yGameStatus) << newl;

                bProcessed = true;
            }
            break;

        case NETCMD_MANAGER_LONG_FRAME:
            {
                uint uiFrameLength(pkt.ReadInt(0));

                Console.Std << _T("Slave #") << itServer->second.iSlave <<  _T(" long frame (") << uiFrameLength << _T(" msec)") << newl;

                m_Log.WriteSlave(MAN_LOG_SLAVE_LONG_FRAME, itServer->second, XtoA(uiFrameLength));

                bProcessed = true;
            }
            break;
        }

        if (!bProcessed)
            Console.Net << _T("Invalid Packet received from slave server ") << sAddr
                << _T(", cmd ") << XtoA(cmd, FMT_PADZERO, 4, 16) << newl;
    }

    if (m_uiLastLogTime == INVALID_TIME || Host.GetSystemTime() >= m_uiLastLogTime + man_logPeriod)
    {
        for (SlaveServerMap::iterator it(m_mapServers.begin()); it != m_mapServers.end(); ++it)
        {
            if (it->second.yServerStatus != SERVER_STATUS_ACTIVE)
                continue;

            m_Log.WriteSlave(MAN_LOG_SLAVE_STATUS, it->second);

            MemManager.Set(&it->second.cLoad, 0, sizeof(SServerLoad));
        }

        m_uiLastLogTime = Host.GetSystemTime();
    }

    // Check for updates... Managers check every 2 minutes
    if (m_uiLastUpdateCheck == INVALID_TIME || Host.GetSystemTime() >= m_uiLastUpdateCheck + 120000)
    {
        K2Updater.SilentUpdate();
        m_uiLastUpdateCheck = Host.GetSystemTime();
    }

    if (!m_deqFileUploads.empty())
    {
        SActiveUpload &cUpload(m_deqFileUploads.front());
        
        if (!cUpload.hFile.IsOpen()) // Start new upload
            cUpload.hFile.Open(cUpload.sFilename, FILE_READ | FILE_BINARY);

        if (!cUpload.hFile.IsOpen())
        {
            Console << _T("Failed to open file for uploading ") << SingleQuoteStr(cUpload.sFilename) << newl;
            m_deqFileUploads.pop_front();
        }
        else
        {
            CFileHTTP fileHTTP;

            if (!cUpload.bStarted)
            {
                Console << _T("Starting upload on ") << cUpload.sFilename << newl;
                cUpload.bStarted = true;
            }

            fileHTTP.SetFileTarget(cUpload.hFile.GetFile());
            fileHTTP.Open(_T("ftp://") + man_masterLogin + _T(":") + MD5String(TStringToUTF8(man_masterPassword)) + _T("@") + cUpload.sHost + cUpload.sTarget, FILE_HTTP_UPLOAD);

            if (fileHTTP.DoneUpload())
            {
                if (fileHTTP.ErrorEncountered())
                {
                    Console << _T("Error uploading ") << cUpload.sFilename << newl;

                    if (cUpload.iRetries < 5)
                    {
                        ++cUpload.iRetries;
                        Console << _T("Retrying upload, attempt ") << cUpload.iRetries << newl;
                    }
                    else
                    {
                        Console << _T("Retries exhausted, upload failed.") << newl;
                        cUpload.hFile.Close();
                        m_deqFileUploads.pop_front();
                    }
                }
                else
                {
                    Console << _T("Finished upload on ") << cUpload.sFilename << newl;
                    cUpload.hFile.Close();
                    m_deqFileUploads.pop_front();
                }
            }
            else
            {
                // Update progress
                SHTTPProgress cProgress(fileHTTP.GetProgress());

                cUpload.fProgress = cProgress.fPercent;
            }
        }
    }

    if (m_bUpdateAvailable || m_bShutdownInstances)
    {
        // Shutdown idle and sleeping servers
        for (SlaveServerMap::iterator it(m_mapServers.begin()); it != m_mapServers.end(); ++it)
        {
            if ((it->second.yExpectedServerStatus == SERVER_STATUS_SLEEPING && it->second.yServerStatus == SERVER_STATUS_SLEEPING) ||
                (it->second.yExpectedServerStatus == SERVER_STATUS_IDLE && it->second.yServerStatus == SERVER_STATUS_IDLE))
            {
                m_Log.WriteSlave(MAN_LOG_SLAVE_TERMINATE3, it->second);

                CPacket pkt;
                pkt.WriteByte(NETCMD_MANAGER_SHUTDOWN_SLAVE);

                m_cSocket.SetSendAddr(it->first);
                m_cSocket.SendPacket(pkt);

                it->second.yExpectedServerStatus = SERVER_STATUS_KILLED;
            }
        }

        uiset setProcesses(K2System.GetRunningProcesses());

        if (m_bUpdateAvailable)
        {
            if (setProcesses.size() == 1 && m_deqFileUploads.empty())
                PerformUpdate();
        }

        if (m_bReset)
        {
            if (setProcesses.size() == 1 && m_deqFileUploads.empty())
            {
                K2System.RestartOnExit(true);
                K2System.Exit(0);
            }
        }

        return;
    }

    uint uiNumLoading(0);
    for (SlaveServerMap::iterator it(m_mapServers.begin()); it != m_mapServers.end(); ++it)
    {
        if (it->second.yServerStatus == SERVER_STATUS_UNKNOWN)
            ++uiNumLoading;
    }

    if (uiNumLoading == 0)
    {
        if (!m_deqServerStart.empty())
        {
            StartInstance(m_deqServerStart.front());
            m_deqServerStart.pop_front();
        }
    }
    
    // Wake or sleep servers
    uint uiNumIdle(0);
    for (SlaveServerMap::iterator it(m_mapServers.begin()); it != m_mapServers.end(); ++it)
    {
        if (it->second.yExpectedServerStatus == SERVER_STATUS_IDLE)
            ++uiNumIdle;
    }

    while (uiNumIdle < man_idleTarget)
    {
        // Find CPU with lowest number of servers running
        uint auiCount[32] = {0};
        uint uiLowestIndex(uint(-1));
        uint uiLowestCount(uint(-1));
        for (SlaveServerMap::iterator it(m_mapServers.begin()); it != m_mapServers.end(); ++it)
        {
            if (it->second.yExpectedServerStatus == SERVER_STATUS_IDLE || it->second.yExpectedServerStatus == SERVER_STATUS_ACTIVE)
                ++auiCount[it->second.uiAffinity];
        }
        for (uint ui(0); ui < m_uiNumAffinities; ++ui)
        {
            if (auiCount[ui] < uiLowestCount)
            {
                uiLowestIndex = ui;
                uiLowestCount = auiCount[ui];
            }
        }

        uint uiAffinity(m_auiAffinity[uiLowestIndex]);

        // Find a sleeping server using this CPU and wake it up
        SlaveServerMap::iterator it(m_mapServers.begin());
        for (; it != m_mapServers.end(); ++it)
        {
            if (it->second.yExpectedServerStatus == SERVER_STATUS_SLEEPING &&
                it->second.yServerStatus == SERVER_STATUS_SLEEPING &&
                it->second.uiAffinity == uiAffinity)
            {
                Console << _T("Waking slave server ") << it->first << newl;

                CPacket pkt;
                pkt.WriteByte(NETCMD_MANAGER_WAKE);

                m_cSocket.SetSendAddr(it->first);
                m_cSocket.SendPacket(pkt);

                it->second.yExpectedServerStatus = 1;

                m_Log.WriteSlave(MAN_LOG_SLAVE_WAKE, it->second);
                
                ++uiNumIdle;
                break;
            }
        }

        // No free sleeping servers
        if (it == m_mapServers.end())
        {
            // Find a sleeping server on any CPU to wake up
            SlaveServerMap::iterator it2(m_mapServers.begin());
            for (; it2 != m_mapServers.end(); ++it2)
            {
                if (it2->second.yExpectedServerStatus == SERVER_STATUS_SLEEPING &&
                    it2->second.yServerStatus == SERVER_STATUS_SLEEPING)
                {
                    Console << _T("Waking alternate slave server ") << it2->first << newl;

                    CPacket pkt;
                    pkt.WriteByte(NETCMD_MANAGER_WAKE);

                    m_cSocket.SetSendAddr(it2->first);
                    m_cSocket.SendPacket(pkt);

                    it2->second.yExpectedServerStatus = SERVER_STATUS_IDLE;

                    m_Log.WriteSlave(MAN_LOG_SLAVE_WAKE, it2->second);
                    
                    ++uiNumIdle;
                    break;
                }
            }

            // No free sleeping servers
            if (it2 == m_mapServers.end())
                break;
        }
    }

    while (uiNumIdle > man_idleTarget)
    {
        // Find CPU with greatest number of servers running
        uint auiCount[32] = {0};
        uint uiGreatestIndex(uint(-1));
        uint uiGreatestCount(0);
        for (SlaveServerMap::iterator it(m_mapServers.begin()); it != m_mapServers.end(); ++it)
        {
            if (it->second.yExpectedServerStatus == SERVER_STATUS_IDLE || it->second.yExpectedServerStatus == SERVER_STATUS_ACTIVE)
                ++auiCount[it->second.uiAffinity];
        }
        for (uint ui(0); ui < m_uiNumAffinities; ++ui)
        {
            if (uiGreatestIndex == uint(-1) || auiCount[ui] > uiGreatestCount)
            {
                uiGreatestIndex = ui;
                uiGreatestCount = auiCount[ui];
            }
        }

        uint uiAffinity(m_auiAffinity[uiGreatestIndex]);

        // Find a idle server using this CPU to put to sleep
        SlaveServerMap::iterator it(m_mapServers.begin());
        for (; it != m_mapServers.end(); ++it)
        {
            if (it->second.yExpectedServerStatus == SERVER_STATUS_IDLE &&
                it->second.yServerStatus == SERVER_STATUS_IDLE &&
                it->second.uiAffinity == uiAffinity)
            {
                Console << _T("Sleeping slave server ") << it->first << newl;

                CPacket pkt;
                pkt.WriteByte(NETCMD_MANAGER_SLEEP);

                m_cSocket.SetSendAddr(it->first);
                m_cSocket.SendPacket(pkt);

                it->second.yExpectedServerStatus = SERVER_STATUS_SLEEPING;

                m_Log.WriteSlave(MAN_LOG_SLAVE_SLEEP, it->second);
                
                --uiNumIdle;
                break;
            }
        }

        // No idle servers on this CPU
        if (it == m_mapServers.end())
        {
            // Find a idle server using on any CPU to put to sleep
            SlaveServerMap::iterator it2(m_mapServers.begin());
            for (; it2 != m_mapServers.end(); ++it2)
            {
                if (it2->second.yExpectedServerStatus == SERVER_STATUS_IDLE &&
                    it2->second.yServerStatus == SERVER_STATUS_IDLE)
                {
                    Console << _T("Sleeping alternate slave server ") << it2->first << newl;

                    CPacket pkt;
                    pkt.WriteByte(NETCMD_MANAGER_SLEEP);

                    m_cSocket.SetSendAddr(it2->first);
                    m_cSocket.SendPacket(pkt);

                    it2->second.yExpectedServerStatus = SERVER_STATUS_SLEEPING;

                    m_Log.WriteSlave(MAN_LOG_SLAVE_SLEEP, it2->second);
                    
                    --uiNumIdle;
                    break;
                }
            }

            // No idle servers to put to sleep
            if (it2 == m_mapServers.end())
                break;
        }
    }
    
    // Mark unresponsive servers
    for (SlaveServerMap::iterator it(m_mapServers.begin()); it != m_mapServers.end(); ++it)
    {
        if (it->second.uiLastChance == INVALID_TIME &&
            K2System.Milliseconds() > it->second.uiLastResponse + man_slaveTimeout &&
            (it->second.yServerStatus < SERVER_STATUS_CRASHED || it->second.yServerStatus == SERVER_STATUS_UNKNOWN))
        {
            Console << _T("Slave #") << it->second.iSlave << _T(" is unresponsive, giving last chance") << newl;
            it->second.uiLastChance = K2System.Milliseconds();

            m_Log.WriteSlave(MAN_LOG_SLAVE_UNRESPONSIVE, it->second);
        }
    }

    // Restart unresponsive servers
    for (SlaveServerMap::iterator it(m_mapServers.begin()); it != m_mapServers.end(); ++it)
    {
        if (it->second.uiLastChance != INVALID_TIME &&
            K2System.Milliseconds() > it->second.uiLastChance + man_slaveTimeoutLastChance &&
            (it->second.yServerStatus < SERVER_STATUS_CRASHED || it->second.yServerStatus == SERVER_STATUS_UNKNOWN))
        {
            Console << _T("Slave #") << it->second.iSlave << _T(" is still unresponsive, terminating") << newl;

            it->second.yServerStatus = SERVER_STATUS_CRASHED;
            it->second.yExpectedServerStatus = SERVER_STATUS_CRASHED;

            m_Log.WriteSlave(MAN_LOG_SLAVE_TERMINATE, it->second);

            m_deqServerStart.push_back(it->second.uiAffinity);
        }
    }
}


/*====================
  CServerManager::Shutdown
  ====================*/
void    CServerManager::Shutdown()
{
    ShutdownInstances();

    m_Log.Close();
}


/*====================
  CServerManager::SetCvar
  ====================*/
void    CServerManager::SetCvar(tstring &sConfig, const tstring &sName, const tstring &sValue)
{
    sConfig += _T("Set ") + sName + _T(" ") + sValue + _T("; ");
}


/*====================
  CServerManager::StartInstance
  ====================*/
void    CServerManager::StartInstance(uint uiAffinity)
{
    int iNumServers(0);
    for (SlaveServerMap::iterator it(m_mapServers.begin()); it != m_mapServers.end(); ++it)
    {
        // Ignore crashed or killed slaves
        if (it->second.yServerStatus != SERVER_STATUS_UNKNOWN && it->second.yServerStatus >= SERVER_STATUS_CRASHED)
            continue;

        ++iNumServers;
    }

    if (man_maxServers != -1 && iNumServers >= man_maxServers)
        return;

    // Find an available local port
    word wPort(man_startServerPort);
    bool bFreePort(false);
    while (!bFreePort && wPort <= man_endServerPort)
    {
        SlaveServerMap::iterator itFind(m_mapServers.find(_T("127.0.0.1:") + XtoA(wPort)));
        if (itFind == m_mapServers.end())
        {
            bFreePort = true;
            break;
        }

        ++wPort;
    }

    if (!bFreePort)
    {
        Console << _T("No free ports found") << newl;
        return;
    }

    // Find CPU with lowest number of servers running
    if (uiAffinity == uint(-1))
    {
        uint auiCount[32] = {0};
        uint uiLowestIndex(uint(-1));
        uint uiLowestCount(uint(-1));
        for (SlaveServerMap::iterator it(m_mapServers.begin()); it != m_mapServers.end(); ++it)
        {
            // Ignore crashed or killed slaves
            if (it->second.yServerStatus != SERVER_STATUS_UNKNOWN && it->second.yServerStatus >= SERVER_STATUS_CRASHED)
                continue;

            ++auiCount[it->second.uiAffinity];
        }
        for (uint ui(0); ui < m_uiNumAffinities; ++ui)
        {
            if (auiCount[ui] < uiLowestCount)
            {
                uiLowestIndex = ui;
                uiLowestCount = auiCount[ui];
            }
        }

        uiAffinity = m_auiAffinity[uiLowestIndex];
    }

    // Find first available slave account
    int iSlave(1);
    while (iSlave <= man_numSlaveAccounts)
    {
        SlaveServerMap::iterator it(m_mapServers.begin());
        for (; it != m_mapServers.end(); ++it)
        {
            // Ignore crashed or killed slaves
            if (it->second.yServerStatus != SERVER_STATUS_UNKNOWN && it->second.yServerStatus >= SERVER_STATUS_CRASHED)
                continue;

            if (it->second.iSlave == iSlave)
                break;
        }

        if (it == m_mapServers.end())
            break;

        ++iSlave;
    }

    if (iSlave > man_numSlaveAccounts)
    {
        Console << _T("No free slave account found") << newl;
        return;
    }

    SServerStatus cServer;

    tstring sCommandline(K2System.GetProcessFilename());

    sCommandline += _T(" -dedicated -noconfig -sleep");

    tstring sConfig;
    SetCvar(sConfig, _T("svr_login"), man_masterLogin + XtoA(iSlave));
    SetCvar(sConfig, _T("svr_password"), man_masterPassword);
    SetCvar(sConfig, _T("svr_name"), svr_name);
    SetCvar(sConfig, _T("svr_slave"), XtoA(iSlave));
    SetCvar(sConfig, _T("svr_port"), XtoA(wPort));
    SetCvar(sConfig, _T("svr_location"), svr_location);
    SetCvar(sConfig, _T("svr_ip"), svr_ip);
    SetCvar(sConfig, _T("host_affinity"), XtoA(uiAffinity));
    SetCvar(sConfig, _T("svr_broadcast"), XtoA(man_broadcastSlaves));
    SetCvar(sConfig, _T("upd_checkForUpdates"), XtoA(false));
    SetCvar(sConfig, _T("sv_autosaveReplay"), XtoA(true));
    //SetCvar(sConfig, _T("svr_requireAuthentication"), XtoA(man_requireAuthentication));
    SetCvar(sConfig, _T("sys_autoSaveDump"), XtoA(true));
    SetCvar(sConfig, _T("sys_dumpOnFatal"), XtoA(true));
    SetCvar(sConfig, _T("con_writeLog"), XtoA(false));
    SetCvar(sConfig, _T("sv_masterName"), man_masterLogin);
    SetCvar(sConfig, _T("svr_chatAddress"), svr_chatAddress);
    SetCvar(sConfig, _T("svr_chatPort"), XtoA(svr_chatPort));
#ifndef _WIN32
    SetCvar(sConfig, _T("sys_interactive"), XtoA(false));
#endif
    
    sCommandline += _T(" -execute ");
    sCommandline += _T("\"");
    sCommandline += sConfig;
    sCommandline += _T("\"");

    sCommandline += _T(" -register 127.0.0.1:") + XtoA(man_port);

    if (!K2System.Shell(sCommandline, false))
        return;

    cServer.iSlave = iSlave;
    cServer.sAddress = _T("127.0.0.1:") + XtoA(wPort);
    cServer.wPort = wPort;
    cServer.uiAffinity = uiAffinity;
    cServer.yExpectedServerStatus = SERVER_STATUS_SLEEPING;
    cServer.yServerStatus = SERVER_STATUS_UNKNOWN;
    cServer.bMatchStarted = false;
    cServer.iNumClients = 0;
    cServer.uiLastResponse = K2System.Milliseconds() + man_slaveLoadTimeout;
    cServer.uiLastChance = INVALID_TIME;

    MemManager.Set(&cServer.cLoad, 0, sizeof(SServerLoad));

    m_mapServers[cServer.sAddress] = cServer;

    m_Log.WriteSlave(MAN_LOG_SLAVE_START, cServer);
}


/*====================
  CServerManager::PrintStatus
  ====================*/
void    CServerManager::PrintStatus()
{
    uint auiLoad[32] = {0};
    uint iNumClients(0);

    Console << _T(" #  Port   Core  Status       Load  Clients  Mem Usage   Game") << newl;
    Console << _T("========================================================================") << newl;

    for (SlaveServerMap::iterator it(m_mapServers.begin()); it != m_mapServers.end(); ++it)
    {
        Console << XtoA(it->second.iSlave, 0, 2) << _T("  ");
        Console << XtoA(it->second.wPort, 0, 5) << _T("  ");
        Console << XtoA(it->second.uiAffinity, 0, 4) << _T("  ");

        switch (it->second.yServerStatus)
        {
        case SERVER_STATUS_SLEEPING:
            Console << XtoA(_T("Sleeping"), FMT_ALIGNLEFT, 8) << _T("  ");
            break;
        case SERVER_STATUS_IDLE:
            Console << XtoA(_T("Idle"), FMT_ALIGNLEFT, 8) << _T("  ");
            break;
        case SERVER_STATUS_ACTIVE:
            if (!it->second.bMatchStarted)
                Console << XtoA(_T("Lobby"), FMT_ALIGNLEFT, 8) << _T("  ");
            else
                Console << XtoA(_T("Active"), FMT_ALIGNLEFT, 8) << _T("  ");
            break;
        case SERVER_STATUS_CRASHED:
            Console << XtoA(_T("Crashed"), FMT_ALIGNLEFT, 8) << _T("  ");
            break;
        case SERVER_STATUS_KILLED:
            Console << XtoA(_T("Killed"), FMT_ALIGNLEFT, 8) << _T("  ");
            break;
        default:
            Console << XtoA(_T("Unknown"), FMT_ALIGNLEFT, 8) << _T("  ");
            break;
        }

        uint uiLoad(0);
        for (deque<SServerLoad>::iterator itLoad(it->second.deqLoad.begin()); itLoad != it->second.deqLoad.end(); ++itLoad)
            uiLoad += itLoad->uiCpuLoad;

        uint uiMemUsage(!it->second.deqLoad.empty() ? it->second.deqLoad.back().uiMemUsage : 0);

        Console << XtoA(uiLoad / 50000.0f, 0, 6, 2) << _T("%") << _T("  ");

        Console << XtoA(it->second.iNumClients, 0, 7) << _T("  ");

        Console << XtoA(GetByteString(uiMemUsage), FMT_ALIGNLEFT, 11) << _T("  ");
        
        Console << it->second.sGameName;

        Console << newl;

        if (it->second.yServerStatus != SERVER_STATUS_UNKNOWN && it->second.yServerStatus < SERVER_STATUS_CRASHED)
        {
            auiLoad[it->second.uiAffinity] += uiLoad;
            iNumClients += it->second.iNumClients;
        }
    }

    for (uint ui(0); ui < m_uiNumAffinities; ++ui)
        Console << _T("Core ") << ui << _T(": ") << XtoA(auiLoad[ui] / 50000.0f, 0, 0, 2) << _T("%") << newl;

    // Disk space
    tsvector vDrives;
    K2System.GetDriveList(vDrives);
    for (tsvector_it itDrive(vDrives.begin()), itEnd(vDrives.end()); itDrive != itEnd; ++itDrive)
    {
        if (K2System.GetDriveType(*itDrive) != DRIVETYPE_FIXED)
            continue;

        ULONGLONG ulFree(K2System.GetDriveFreeSpaceEx(*itDrive));
        ULONGLONG ulTotal(K2System.GetDriveSizeEx(*itDrive));
        Console << *itDrive << _T(" ") << GetByteString(ulFree) << _T(" / ") << GetByteString(ulTotal) << _T(" (") << INT_ROUND(100.0f * (ulFree / float(ulTotal))) << _T("%)") << newl;
    }

    // Memory
    ULONGLONG ullFreePhysical(K2System.GetFreePhysicalMemory());
    ULONGLONG ullTotalPhysical(K2System.GetTotalPhysicalMemory());
    Console << GetByteString(ullFreePhysical) << _T(" / ") << GetByteString(ullTotalPhysical) << _T(" (") << INT_ROUND(100.0f * (ullFreePhysical / float(ullTotalPhysical))) << _T("%)") << newl;

    Console << iNumClients << _T(" clients") << newl;
}


/*====================
  CServerManager::PrintUploadStatus
  ====================*/
void    CServerManager::PrintUploadStatus()
{
}


/*====================
  CServerManager::UploadFile
  ====================*/
void    CServerManager::UploadFile(const tstring &sHost, const tstring &sFilename, const tstring &sTarget)
{
    SActiveUpload cUpload;
    cUpload.sHost = sHost;
    cUpload.sFilename = sFilename;
    cUpload.sTarget = sTarget;

    cUpload.fProgress = 0.0f;
    cUpload.iRetries = 0;

    cUpload.bStarted = false;

    m_deqFileUploads.push_back(cUpload);
}


/*====================
  CServerManager::UpdateAvailable
  ====================*/
void    CServerManager::UpdateAvailable(const tstring &sVersion)
{
    if (m_bUpdateAvailable || m_bUpdating || m_bUpdateComplete)
        return;

    m_Log.WriteInfo(MAN_LOG_UPDATE, _T("version"), sVersion);

    Console << _T("Version ") << sVersion << _T(" is available, initiating slave shutdown sequence") << newl;

    m_bUpdateAvailable = true;
}


/*====================
  CServerManager::PerformUpdate
  ====================*/
void    CServerManager::PerformUpdate()
{
    if (m_bUpdating || !m_bUpdateAvailable)
        return;
    
    m_bUpdating = true;
    K2Updater.CheckForUpdates(false);
}


/*====================
  CServerManager::UpdateComplete
  ====================*/
void    CServerManager::UpdateComplete()
{
    if (m_bUpdateAvailable && m_bUpdating)
        m_bUpdateComplete = true;

    K2System.RestartOnExit(true);
    K2System.Exit(0);
}


/*====================
  CServerManager::ShutdownInstances
  ====================*/
void    CServerManager::ShutdownInstances()
{
    for (SlaveServerMap::iterator it(m_mapServers.begin()); it != m_mapServers.end(); ++it)
    {
        m_Log.WriteSlave(MAN_LOG_SLAVE_TERMINATE2, it->second);

        CPacket pkt;
        pkt.WriteByte(NETCMD_MANAGER_SHUTDOWN_SLAVE);

        m_cSocket.SetSendAddr(it->first);
        m_cSocket.SendPacket(pkt);
    }
}


/*====================
  CServerManager::StartShutdown
  ====================*/
void    CServerManager::StartShutdown()
{
    Console << _T("Initiating slave shutdown sequence") << newl;

    m_Log.WriteInfo(MAN_LOG_SHUTDOWN_SLAVES);

    m_bShutdownInstances = true;
    m_bReset = false;

    K2System.SetPriority(-1);
}


/*====================
  CServerManager::StartReset
  ====================*/
void    CServerManager::StartReset()
{
    Console << _T("Initiating slave restart sequence") << newl;

    m_Log.WriteInfo(MAN_LOG_SHUTDOWN_SLAVES);

    m_bShutdownInstances = true;
    m_bReset = true;

    K2System.SetPriority(-1);
}


/*====================
  CServerManager::Chat
  ====================*/
void    CServerManager::Chat(const tstring &sMsg)
{
    for (SlaveServerMap::iterator it(m_mapServers.begin()); it != m_mapServers.end(); ++it)
    {
        CPacket pkt;
        pkt.WriteByte(NETCMD_MANAGER_CHAT);
        pkt.WriteString(sMsg);

        m_cSocket.SetSendAddr(it->first);
        m_cSocket.SendPacket(pkt);
    }
}


/*--------------------
  ManagerStartInstance
  --------------------*/
CMD(ManagerStartInstance)
{
    CServerManager::GetInstance()->StartInstance(uint(-1));

    return true;
}


/*--------------------
  ManagerStatus
  --------------------*/
CMD(ManagerStatus)
{
    CServerManager::GetInstance()->PrintStatus();

    return true;
}


/*--------------------
  ManagerUploadStatus
  --------------------*/
CMD(ManagerUploadStatus)
{
    CServerManager::GetInstance()->PrintUploadStatus();

    return true;
}


/*--------------------
  ManagerUploadFile
  --------------------*/
CMD(ManagerUploadFile)
{
    if (vArgList.size() < 2)
        return false;

    CServerManager::GetInstance()->UploadFile(vArgList[0], vArgList[1], vArgList[2]);

    return true;
}


/*--------------------
  ManagerShutdownInstances
  --------------------*/
CMD(ManagerShutdownInstances)
{
    CServerManager::GetInstance()->ShutdownInstances();

    return true;
}


/*--------------------
  ManagerStartShutdown
  --------------------*/
CMD(ManagerStartShutdown)
{
    CServerManager::GetInstance()->StartShutdown();

    return true;
}


/*--------------------
  ManagerStartReset
  --------------------*/
CMD(ManagerStartReset)
{
    CServerManager::GetInstance()->StartReset();

    return true;
}


/*--------------------
  ManagerChat
  --------------------*/
CMD(ManagerChat)
{
    tstring sMsg(ConcatinateArgs(vArgList, _T(" ")));

    CServerManager::GetInstance()->Chat(sMsg);

    return true;
}


