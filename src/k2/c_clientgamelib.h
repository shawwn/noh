// (C)2006 S2 Games
// c_clientgamelib.h
//
//=============================================================================
#ifndef __C_CLIENTGAMELIB_H__
#define __C_CLIENTGAMELIB_H__

//=============================================================================
// Declarations
//=============================================================================
class CHostClient;
class CSnapshot;
class CPacket;
class CStateBlock;
//=============================================================================

//=============================================================================
// CClientGameLib
//=============================================================================
class CClientGameLib
{
private:
    void*   m_pGameLib;

    bool    m_bValid;

    tstring m_sName;
    tstring m_sTypeName;
    int     m_iMajorVersion;
    int     m_iMinorVersion;

    typedef void    (FnInitGameDLL)(CClientGameLib&);
    
    typedef void    (FnSetGamePointer)(uint uiIndex);
    typedef bool    (FnInit)(CHostClient *pHostClient);
    typedef void    (FnStartLoadingWorld)();
    typedef void    (FnStartPreloadingWorld)();
    typedef void    (FnFinishedLoadingWorld)();
    typedef void    (FnSpawnNextWorldEntity)();
    typedef void    (FnPrecacheNextWorldEntity)();

    typedef void    (FnStartLoadingResources)();
    typedef void    (FnLoadNextResource)();
    typedef bool    (FnIsFinishedLoadingResources)();
    typedef float   (FnGetResourceLoadingProgress)();

    typedef bool    (FnIsSpawningEntities)();
    typedef bool    (FnIsFinishedSpawningEntities)();
    typedef float   (FnGetEntitySpawningProgress)();

    typedef void    (FnStartLoadingEntityResources)();
    typedef void    (FnLoadNextEntityResource)();
    typedef bool    (FnIsFinishedLoadingEntityResources)();
    typedef float   (FnGetEntityResourceLoadingProgress)();

    typedef void    (FnPreFrame)();
    typedef void    (FnFrame)();
    typedef void    (FnShutdown)();
    typedef bool    (FnGameEvents)(CSnapshot &snapshot);
    typedef bool    (FnSnapshot)(CSnapshot &snapshot);
    typedef bool    (FnGameData)(CPacket &pkt);
    typedef void    (FnDropNotify)(const tsvector &vsFiles);
    typedef void    (FnReinitialize)();
    typedef void    (FnLoadAllResources)();
    typedef void    (FnConnect)(const tstring &sAddr);
    typedef void    (FnStateStringChanged)(uint uiID, const CStateString &ss);
    typedef void    (FnStateBlockChanged)(uint uiID, const CStateBlock &block);
    typedef void    (FnSendCreateGameRequest)(const tstring &sName, const tstring &sOptions);
    typedef tstring (FnGetGameModeName)(uint uiMode);
    typedef uint    (FnGetGameModeFromString)(const tstring &sMode);
    typedef tstring (FnGetGameModeString)(uint uiMode);
    typedef tstring (FnGetGameOptionName)(uint uiOption);
    typedef uint    (FnGetGameOptionFromString)(const tstring &sOption);
    typedef tstring (FnGetGameOptionsString)(uint uiOptions);
    typedef void    (FnStartLoading)(const tstring &sWorldName);

    FnSetGamePointer*               m_fnSetGamePointer;
    FnInit*                         m_fnInit;
    FnStartLoadingWorld*            m_fnStartLoadingWorld;
    FnStartPreloadingWorld*         m_fnStartPreloadingWorld;
    FnFinishedLoadingWorld*         m_fnFinishedLoadingWorld;
    FnSpawnNextWorldEntity*         m_fnSpawnNextWorldEntity;
    FnPrecacheNextWorldEntity*      m_fnPrecacheNextWorldEntity;

    FnStartLoadingResources*        m_fnStartLoadingResources;
    FnLoadNextResource*             m_fnLoadNextResource;
    FnIsFinishedLoadingResources*   m_fnIsFinishedLoadingResources;
    FnGetResourceLoadingProgress*   m_fnGetResourceLoadingProgress;

    FnIsSpawningEntities*           m_fnIsSpawningEntities;
    FnIsFinishedSpawningEntities*   m_fnIsFinishedSpawningEntities;
    FnGetEntitySpawningProgress*    m_fnGetEntitySpawningProgress;

    FnStartLoadingEntityResources*          m_fnStartLoadingEntityResources;
    FnLoadNextEntityResource*               m_fnLoadNextEntityResource;
    FnIsFinishedLoadingEntityResources*     m_fnIsFinishedLoadingEntityResources;
    FnGetEntityResourceLoadingProgress*     m_fnGetEntityResourceLoadingProgress;

    FnPreFrame*                     m_fnPreFrame;
    FnFrame*                        m_fnFrame;
    FnShutdown*                     m_fnShutdown;
    FnGameEvents*                   m_fnProcessGameEvents;
    FnSnapshot*                     m_fnProcessSnapshot;
    FnGameData*                     m_fnProcessGameData;
    FnDropNotify*                   m_fnDropNotify;
    FnReinitialize*                 m_fnReinitialize;
    FnLoadAllResources*             m_fnLoadAllResources;
    FnConnect*                      m_fnConnect;
    FnStateStringChanged*           m_fnStateStringChanged;
    FnStateBlockChanged*            m_fnStateBlockChanged;
    FnSendCreateGameRequest*        m_fnSendCreateGameRequest;
    FnGetGameModeName*              m_fnGetGameModeName;
    FnGetGameModeFromString*        m_fnGetGameModeFromString;
    FnGetGameModeString*            m_fnGetGameModeString;
    FnGetGameOptionName*            m_fnGetGameOptionName;
    FnGetGameOptionFromString*      m_fnGetGameOptionFromString;
    FnGetGameOptionsString*         m_fnGetGameOptionsString;
    FnStartLoading*                 m_fnStartLoading;
    
    CClientGameLib();

public:
    ~CClientGameLib();
    CClientGameLib(const tstring &sLibName);

    bool    IsValid() const                                             { return m_bValid; }
    
    void    SetName(const tstring &sName)                               { m_sName = sName; }
    tstring GetName() const                                             { return m_sName; }

    void    SetTypeName(const tstring &sTypeName)                       { m_sTypeName = sTypeName; }
    const tstring&  GetTypeName() const                                 { return m_sTypeName; }

    void    SetMajorVersion(int iVer)                                   { m_iMajorVersion = iVer; }
    int     GetMajorVersion() const                                     { return m_iMajorVersion; }

    void    SetMinorVersion(int iVer)                                   { m_iMinorVersion = iVer; }
    int     GetMinorVersion() const                                     { return m_iMinorVersion; }

    void    AssignSetGamePointerFn(FnSetGamePointer *fnSetGamePointer)              { m_fnSetGamePointer = fnSetGamePointer; }
    void    AssignInitFn(FnInit *fnInit)                                            { m_fnInit = fnInit; }
    void    AssignStartLoadingWorldFn(FnStartLoadingWorld *fn)                      { m_fnStartLoadingWorld = fn; }
    void    AssignStartPreloadingWorldFn(FnStartPreloadingWorld *fn)                { m_fnStartPreloadingWorld = fn; }
    void    AssignFinishedLoadingWorldFn(FnFinishedLoadingWorld *fn)                { m_fnFinishedLoadingWorld = fn; }
    void    AssignSpawnNextWorldEntityFn(FnSpawnNextWorldEntity *fn)                { m_fnSpawnNextWorldEntity = fn; }
    void    AssignPrecacheNextWorldEntityFn(FnPrecacheNextWorldEntity *fn)          { m_fnPrecacheNextWorldEntity = fn; }

    void    AssignStartLoadingResourcesFn(FnStartLoadingResources *fn)              { m_fnStartLoadingResources = fn; }
    void    AssignLoadNextResourceFn(FnLoadNextResource *fn)                        { m_fnLoadNextResource = fn; }
    void    AssignIsFinishedLoadingResourcesFn(FnIsFinishedLoadingResources *fn)    { m_fnIsFinishedLoadingResources = fn; }
    void    AssignGetResourceLoadingProgressFn(FnGetResourceLoadingProgress *fn)    { m_fnGetResourceLoadingProgress = fn; }

    void    AssignIsSpawningEntitiesFn(FnIsSpawningEntities *fn)                    { m_fnIsSpawningEntities = fn; }
    void    AssignIsFinishedSpawningEntitiesFn(FnIsFinishedSpawningEntities *fn)    { m_fnIsFinishedSpawningEntities = fn; }
    void    AssignGetEntitySpawningProgressFn(FnGetEntitySpawningProgress *fn)      { m_fnGetEntitySpawningProgress = fn; }

    void    AssignStartLoadingEntityResourcesFn(FnStartLoadingEntityResources *fn)              { m_fnStartLoadingEntityResources = fn; }
    void    AssignLoadNextEntityResourceFn(FnLoadNextEntityResource *fn)                        { m_fnLoadNextEntityResource = fn; }
    void    AssignIsFinishedLoadingEntityResourcesFn(FnIsFinishedLoadingEntityResources *fn)    { m_fnIsFinishedLoadingEntityResources = fn; }
    void    AssignGetEntityResourceLoadingProgressFn(FnGetEntityResourceLoadingProgress *fn)    { m_fnGetEntityResourceLoadingProgress = fn; }

    void    AssignPreFrameFn(FnPreFrame *fnPreFrame)                                { m_fnPreFrame = fnPreFrame; }
    void    AssignFrameFn(FnFrame *fnFrame)                                         { m_fnFrame = fnFrame; }
    void    AssignShutdownFn(FnShutdown *fnShutdown)                                { m_fnShutdown = fnShutdown; }
    void    AssignGameEventFn(FnSnapshot *fnGameEvents)                             { m_fnProcessGameEvents = fnGameEvents; }
    void    AssignSnapshotFn(FnSnapshot *fnSnapshot)                                { m_fnProcessSnapshot = fnSnapshot; }
    void    AssignGameDataFn(FnGameData *fnGameData)                                { m_fnProcessGameData = fnGameData; }
    void    AssignDropNotifyFn(FnDropNotify *fnDropNotify)                          { m_fnDropNotify = fnDropNotify; }
    void    AssignReinitializeFn(FnReinitialize *fnReinitialize)                    { m_fnReinitialize = fnReinitialize; }
    void    AssignLoadAllResourcesFn(FnLoadAllResources *fnLoadAllResources)        { m_fnLoadAllResources = fnLoadAllResources; }
    void    AssignConnectFn(FnConnect *fnConnect)                                   { m_fnConnect = fnConnect; }
    void    AssignStateStringChangedFn(FnStateStringChanged *fnStateStringChanged)  { m_fnStateStringChanged = fnStateStringChanged; }
    void    AssignStateBlockChangedFn(FnStateBlockChanged *fnStateBlockChanged)     { m_fnStateBlockChanged = fnStateBlockChanged; }
    void    AssignSendCreateGameRequestFn(FnSendCreateGameRequest *fn)              { m_fnSendCreateGameRequest = fn; }
    void    AssignGetGameModeNameFn(FnGetGameModeName *fn)                          { m_fnGetGameModeName = fn; }
    void    AssignGetGameModeFromStringFn(FnGetGameModeFromString *fn)              { m_fnGetGameModeFromString = fn; }
    void    AssignGetGameModeStringFn(FnGetGameModeString *fn)                      { m_fnGetGameModeString = fn; }
    void    AssignGetGameOptionNameFn(FnGetGameOptionName *fn)                      { m_fnGetGameOptionName = fn; }
    void    AssignGetGameOptionFromStringFn(FnGetGameOptionFromString *fn)          { m_fnGetGameOptionFromString = fn; }
    void    AssignGetGameOptionsStringFn(FnGetGameOptionsString *fn)                { m_fnGetGameOptionsString = fn; }
    void    AssignStartLoadingFn(FnStartLoading *fnStartLoading)                    { m_fnStartLoading = fnStartLoading; }

    void    SetGamePointer(uint uiIndex) const                                      { if (m_fnSetGamePointer != NULL) m_fnSetGamePointer(uiIndex); }
    bool    Init(CHostClient *pHostClient)                                          { if (m_fnInit == NULL) return false; return m_fnInit(pHostClient); }

    void    StartLoadingResources()                                                 { if (m_fnStartLoadingResources != NULL) m_fnStartLoadingResources(); }
    void    LoadNextResource()                                                      { if (m_fnLoadNextResource != NULL) m_fnLoadNextResource(); }
    bool    IsFinishedLoadingResources()                                            { if (m_fnIsFinishedLoadingResources == NULL) return true; return m_fnIsFinishedLoadingResources(); }
    float   GetResourceLoadingProgress()                                            { if (m_fnGetResourceLoadingProgress == NULL) return 0.0f; return m_fnGetResourceLoadingProgress(); }

    bool    IsSpawningEntities()                                                    { if (m_fnIsSpawningEntities == NULL) return true; return m_fnIsSpawningEntities(); }
    bool    IsFinishedSpawningEntities()                                            { if (m_fnIsFinishedSpawningEntities == NULL) return true; return m_fnIsFinishedSpawningEntities(); }
    float   GetEntitySpawningProgress()                                             { if (m_fnGetEntitySpawningProgress == NULL) return 0.0f; return m_fnGetEntitySpawningProgress(); }

    void    StartLoadingEntityResources()                                           { if (m_fnStartLoadingEntityResources != NULL) m_fnStartLoadingEntityResources(); }
    void    LoadNextEntityResource()                                                { if (m_fnLoadNextEntityResource != NULL) m_fnLoadNextEntityResource(); }
    bool    IsFinishedLoadingEntityResources()                                      { if (m_fnIsFinishedLoadingEntityResources == NULL) return true; return m_fnIsFinishedLoadingEntityResources(); }
    float   GetEntityResourceLoadingProgress()                                      { if (m_fnGetEntityResourceLoadingProgress == NULL) return 0.0f; return m_fnGetEntityResourceLoadingProgress(); }

    void    StartLoadingWorld()                                                     { if (m_fnStartLoadingWorld != NULL) m_fnStartLoadingWorld(); }
    void    StartPreloadingWorld()                                                  { if (m_fnStartPreloadingWorld != NULL) m_fnStartPreloadingWorld(); }
    void    FinishedLoadingWorld()                                                  { if (m_fnFinishedLoadingWorld != NULL) m_fnFinishedLoadingWorld(); }
    void    SpawnNextWorldEntity()                                                  { if (m_fnSpawnNextWorldEntity != NULL) m_fnSpawnNextWorldEntity(); }
    void    PrecacheNextWorldEntity()                                               { if (m_fnPrecacheNextWorldEntity != NULL) m_fnPrecacheNextWorldEntity(); }

    void    PreFrame()                                                              { if (m_fnPreFrame != NULL) m_fnPreFrame(); }
    void    Frame()                                                                 { if (m_fnFrame != NULL) m_fnFrame(); }
    void    Shutdown()                                                              { if (m_fnShutdown != NULL) m_fnShutdown(); }
    bool    ProcessGameEvents(CSnapshot &snapshot)                                  { if (m_fnProcessGameEvents != NULL) return m_fnProcessGameEvents(snapshot); return false; }
    bool    ProcessSnapshot(CSnapshot &snapshot)                                    { if (m_fnProcessSnapshot != NULL) return m_fnProcessSnapshot(snapshot); return false; }
    bool    ProcessGameData(CPacket &pkt)                                           { if (m_fnProcessGameData != NULL) return m_fnProcessGameData(pkt); return false; }
    void    FileDropNotify(const tsvector &vsFiles)                                 { if (m_fnDropNotify != NULL) m_fnDropNotify(vsFiles); }
    void    Reinitialize()                                                          { if (m_fnReinitialize != NULL) m_fnReinitialize(); }
    void    LoadAllResources()                                                      { if (m_fnLoadAllResources != NULL) m_fnLoadAllResources(); }
    void    Connect(const tstring &sAddr)                                           { if (m_fnConnect != NULL) m_fnConnect(sAddr); }
    void    StateStringChanged(uint uiID, const CStateString &ss)                   { if (m_fnStateStringChanged != NULL) m_fnStateStringChanged(uiID, ss); }
    void    StateBlockChanged(uint uiID, const CStateBlock &block)                  { if (m_fnStateBlockChanged != NULL) m_fnStateBlockChanged(uiID, block); }
    void    SendCreateGameRequest(const tstring &sName, const tstring &sOptions)    { if (m_fnSendCreateGameRequest != NULL) m_fnSendCreateGameRequest(sName, sOptions); }
    tstring GetGameModeName(uint uiMode)                                            { if (m_fnGetGameModeName != NULL) return m_fnGetGameModeName(uiMode); else return TSNULL; }
    uint    GetGameModeFromString(const tstring &sMode)                             { if (m_fnGetGameModeFromString != NULL) return m_fnGetGameModeFromString(sMode); else return 0; }
    tstring GetGameModeString(uint uiMode)                                          { if (m_fnGetGameModeString != NULL) return m_fnGetGameModeString(uiMode); else return TSNULL; }
    tstring GetGameOptionName(uint uiOption)                                        { if (m_fnGetGameOptionName != NULL) return m_fnGetGameOptionName(uiOption); else return TSNULL; }
    uint    GetGameOptionFromString(const tstring &sOption)                         { if (m_fnGetGameOptionFromString != NULL) return m_fnGetGameOptionFromString(sOption); else return 0; }
    tstring GetGameOptionsString(uint uiOptions)                                    { if (m_fnGetGameOptionsString != NULL) return m_fnGetGameOptionsString(uiOptions); else return TSNULL; }
    void    StartLoading(const tstring &sWorldName)                                     { if (m_fnStartLoading != NULL) m_fnStartLoading(sWorldName); }
};
//=============================================================================

#endif //__C_CLIENTGAMELIB_H__
