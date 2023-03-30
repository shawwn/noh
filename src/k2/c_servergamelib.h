// (C)2008 S2 Games
// c_servergamelib.h
//
//=============================================================================
#include "k2_protocol.h" // enum EClientConnectionState;

//=============================================================================
// Declarations
//=============================================================================
class CClientSnapshot;
class CClientConnection;
class CPacket;
class CHTTPRequest;
class CPHPData;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define FUNCTION(name) \
private: \
    Fn##name    m_fn##name; \
public: \
    void    Assign##name##Fn(Fn##name fn)   { m_fn##name = fn; } \

typedef void    (FnInitServerGameLib)(class CServerGameLib&);
//=============================================================================

//=============================================================================
// CServerGameLib
//=============================================================================
class CServerGameLib
{
private:
    bool    m_bValid;
    void*   m_pGameLib;
    tstring m_sName;
    tstring m_sTypeName;
    int     m_iMajorVersion;
    int     m_iMinorVersion;

    typedef void    (*FnSetGamePointer)();
    typedef bool    (*FnInit)(CHostServer *pHostServer);
    typedef void    (*FnFrame)();
    typedef bool    (*FnLoadWorld)(const tstring &sName, const tstring &sGameSettings);
    typedef bool    (*FnAddClient)(CClientConnection *pClientConnection);
    typedef void    (*FnRemoveClient)(int iClientNum, const tstring &sReason);
    typedef void    (*FnClientTimingOut)(int iClientNum);
    typedef uint    (*FnGetMaxClients)();
    typedef uint    (*FnProcessClientSnapshot)(int iClienNum, CClientSnapshot &snapshot);
    typedef bool    (*FnProcessGameData)(int iClientNum, CPacket &pkt);
    typedef void    (*FnGetSnapshot)(CSnapshot &snapshot);
    typedef void    (*FnShutdown)();
    typedef uint    (*FnGetMatchTime)();
    typedef void    (*FnReauthClient)(CClientConnection *pClientConnection);
    typedef bool    (*FnStartReplay)(const tstring &sFilename);
    typedef void    (*FnStopReplay)();
    typedef void    (*FnStateStringChanged)(uint uiID, const CStateString &ss);
    typedef void    (*FnStateBlockChanged)(uint uiID, const IBuffer &buffer);
    typedef void    (*FnUnloadWorld)();
    typedef void*   (*FnGetEntity)(uint uiIndex);
    typedef void    (*FnGetServerInfo)(CPacket &pkt);
    typedef void    (*FnGetReconnectInfo)(CPacket &pkt, uint uiMatchID, uint uiAccountID, ushort unConnectionID);
    typedef bool    (*FnIsPlayerReconnecting)(int iAccountID);
    typedef bool    (*FnIsDuplicateAccountInGame)(int iAccountID);
    typedef bool    (*FnRemoveDuplicateAccountsInGame)(int iAccountID);
    typedef void    (*FnGetGameStatus)(CPacket &pkt);
    typedef void    (*FnGetHeartbeatInfo)(CHTTPRequest *pHeartbeat);
    typedef void    (*FnProcessAuthData)(int iAccountID, const CPHPData *pData);
    typedef void    (*FnProcessAuxData)(int iClientNum, const CPHPData *pData);
    typedef void    (*FnLongServerFrame)(uint uiFrameLength);
    typedef void    (*FnReset)();
    typedef void    (*FnEndFrame)(PoolHandle hSnapshot);
    typedef void    (*FnClientStateChange)(int iClientNum, EClientConnectionState eState);
    typedef uint    (*FnGetGameInfoInt)(const tstring &sType);
    typedef tstring (*FnGetGameInfoString)(const tstring &sType);
    typedef void    (*FnUpdateUpgrades)(int iClientNum);
    
    FUNCTION(SetGamePointer)
    FUNCTION(Init)
    FUNCTION(Frame)
    FUNCTION(LoadWorld)
    FUNCTION(AddClient)
    FUNCTION(RemoveClient)
    FUNCTION(ClientTimingOut)
    FUNCTION(GetMaxClients)
    FUNCTION(ProcessClientSnapshot)
    FUNCTION(ProcessGameData)
    FUNCTION(GetSnapshot)
    FUNCTION(Shutdown)
    FUNCTION(GetMatchTime)
    FUNCTION(ReauthClient)
    FUNCTION(StartReplay)
    FUNCTION(StopReplay)
    FUNCTION(StateStringChanged)
    FUNCTION(StateBlockChanged)
    FUNCTION(UnloadWorld)
    FUNCTION(GetEntity)
    FUNCTION(GetServerInfo)
    FUNCTION(GetReconnectInfo)
    FUNCTION(IsPlayerReconnecting)
    FUNCTION(IsDuplicateAccountInGame)
    FUNCTION(RemoveDuplicateAccountsInGame)
    FUNCTION(GetGameStatus)
#ifndef K2_CLIENT
    FUNCTION(GetHeartbeatInfo)
    FUNCTION(ProcessAuthData)
    FUNCTION(ProcessAuxData)
#endif
    FUNCTION(LongServerFrame)
    FUNCTION(Reset)
    FUNCTION(EndFrame)
    FUNCTION(ClientStateChange)
    FUNCTION(GetGameInfoInt)
    FUNCTION(GetGameInfoString)
    FUNCTION(UpdateUpgrades)

    CServerGameLib();

public:
    ~CServerGameLib();
    CServerGameLib(const tstring &sLibPath);

    void            Invalidate()                                    { m_bValid = false; }

    void            SetName(const tstring &sName)                   { m_sName = sName; }
    const tstring&  GetName() const                                 { return m_sName; }

    void            SetTypeName(const tstring &sTypeName)           { m_sTypeName = sTypeName; }
    const tstring&  GetTypeName() const                             { return m_sTypeName; }
    
    void            SetVersion(int iMajor, int iMinor)              { m_iMajorVersion = iMajor; m_iMinorVersion = iMinor; }
    int             GetMajorVersion() const                         { return m_iMajorVersion; }
    int             GetMinorVersion() const                         { return m_iMinorVersion; }

    void            SetGamePointer() const                                                  { if (m_bValid) m_fnSetGamePointer(); }
    bool            Init(CHostServer *pHostServer) const                                    { if (m_bValid) return m_fnInit(pHostServer); return false; }
    void            Frame() const                                                           { if (m_bValid) m_fnFrame(); }
    bool            LoadWorld(const tstring &sName, const tstring &sGameSettings) const     { if (m_bValid) return m_fnLoadWorld(sName, sGameSettings); return false; }
    bool            AddClient(CClientConnection *pClientConnection) const                   { if (m_bValid) return m_fnAddClient(pClientConnection); return false; }
    void            RemoveClient(int iClientNum, const tstring &sReason) const              { if (m_bValid) m_fnRemoveClient(iClientNum, sReason); }
    void            ClientTimingOut(int iClientNum) const                                   { if (m_bValid) m_fnClientTimingOut(iClientNum); }
    uint            GetMaxClients() const                                                   { if (m_bValid) return m_fnGetMaxClients(); return 0; }
    uint            ProcessClientSnapshot(int iClientNum, CClientSnapshot &snapshot) const  { if (m_bValid) return m_fnProcessClientSnapshot(iClientNum, snapshot); return 0; }
    bool            ProcessGameData(int iClientNum, CPacket &pkt) const                     { if (m_bValid) return m_fnProcessGameData(iClientNum, pkt); return false; }
    void            GetSnapshot(CSnapshot &snapshot) const                                  { if (m_bValid) m_fnGetSnapshot(snapshot); }
    void            Shutdown() const                                                        { if (m_bValid) m_fnShutdown(); }
    uint            GetMatchTime() const                                                    { if (m_bValid) return m_fnGetMatchTime(); return INVALID_TIME; }
    void            ReauthClient(CClientConnection *pClientConnection) const                { if (m_bValid) m_fnReauthClient(pClientConnection); }
    bool            StartReplay(const tstring &sFilename) const                             { if (m_bValid) return m_fnStartReplay(sFilename); return false; }
    void            StopReplay() const                                                      { if (m_bValid) m_fnStopReplay(); }
    void            StateStringChanged(uint uiID, const CStateString &ss) const             { if (m_bValid) m_fnStateStringChanged(uiID, ss); }
    void            StateBlockChanged(uint uiID, const IBuffer &buffer) const               { if (m_bValid) m_fnStateBlockChanged(uiID, buffer); }
    void            UnloadWorld() const                                                     { if (m_bValid) m_fnUnloadWorld(); }
    void*           GetEntity(uint uiIndex) const                                           { if (m_bValid) return m_fnGetEntity(uiIndex); return NULL; }
    void            GetServerInfo(CPacket &pkt)                                             { if (m_bValid) m_fnGetServerInfo(pkt); }
    void            GetReconnectInfo(CPacket &pkt, uint uiMatchID, uint uiAccountID, ushort unConnectionID) { if (m_bValid) m_fnGetReconnectInfo(pkt, uiMatchID, uiAccountID, unConnectionID); }
    bool            IsPlayerReconnecting(int iAccountID)                                    { if (m_bValid) return m_fnIsPlayerReconnecting(iAccountID); return false; }
    bool            IsDuplicateAccountInGame(int iAccountID)                                { if (m_bValid) return m_fnIsDuplicateAccountInGame(iAccountID); return false; }
    bool            RemoveDuplicateAccountsInGame(int iAccountID)                           { if (m_bValid) return m_fnRemoveDuplicateAccountsInGame(iAccountID); return false; }
    void            GetGameStatus(CPacket &pkt)                                             { if (m_bValid) m_fnGetGameStatus(pkt); }
#ifndef K2_CLIENT
    void            GetHeartbeatInfo(CHTTPRequest *pHeartbeat)                              { if (m_bValid) m_fnGetHeartbeatInfo(pHeartbeat); }
    void            ProcessAuthData(int iAccountID, const CPHPData *pData)                  { if (m_bValid) m_fnProcessAuthData(iAccountID, pData); }
    void            ProcessAuxData(int iClientNum, const CPHPData *pData)                   { if (m_bValid) m_fnProcessAuxData(iClientNum, pData); }
#endif
    void            LongServerFrame(uint uiFrameLength)                                     { if (m_bValid) m_fnLongServerFrame(uiFrameLength); }
    void            Reset()                                                                 { if (m_bValid) m_fnReset(); }
    void            EndFrame(PoolHandle hSnapshot) const                                    { if (m_bValid) m_fnEndFrame(hSnapshot); }
    void            ClientStateChange(int iClientNum, EClientConnectionState eState) const  { if (m_bValid) m_fnClientStateChange(iClientNum, eState); }
    uint            GetGameInfoInt(const tstring &sType) const                              { if (m_bValid) return m_fnGetGameInfoInt(sType); return 0; }   
    tstring         GetGameInfoString(const tstring &sType) const                           { if (m_bValid) return m_fnGetGameInfoString(sType); return TSNULL; }   
    void            UpdateUpgrades(int iClientNum) const                                    { if (m_bValid) return m_fnUpdateUpgrades(iClientNum); }    
};
//=============================================================================
