// (C)2006 S2 Games
// c_updater.h
//
// CUpdater
// Provides a class that connects to the patch server,
// checks for updates, and downloads them (If available).
//=============================================================================
#ifndef __C_UPDATER_H__
#define __C_UPDATER_H__

//=============================================================================
// Header
//=============================================================================
#include "c_filehttp.h"
#include "c_filedisk.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CHTTPRequest;
class CPHPData;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define K2Updater (*CUpdater::GetInstance())

struct SUpdateFile
{
    tstring     sFile;
    tstring     sURLFile;
    bool        bDownloading;
    uint        uiDownloaded;
    uint        uiSize;
    bool        bGotSize;
    CFileDisk*  pFile;
    int         iRetries;
    bool        bComplete;
    tstring     sVersion;
    tstring     sOS;
    tstring     sArch;
    uint        uiFullSize;
    uint        uiChecksum;
    bool        bVerified;
    bool        bExtracted;
    bool        bPrimaryFail;
    bool        bSecondaryFail;
    tstring     sOldVersion;
};

struct SDeleteFile
{
    tstring     sFile;
    tstring     sOldVersion;
};

typedef vector<SUpdateFile>     UpdVector;
typedef vector<SDeleteFile>     DelVector;
typedef map<tstring, CArchive>  ZipMap;

#define UPDATER_MAX_RETRIES 5

enum EUpdaterStatus
{
    UPDATE_STATUS_IDLE = 0,
    UPDATE_STATUS_SILENT,
    UPDATE_STATUS_RETRIEVING_FILES,
    UPDATE_STATUS_DOWNLOADING,
    UPDATE_STATUS_RETRIEVING_COMPAT_FILES,
    UPDATE_STATUS_DOWNLOADING_COMPAT,
    UPDATE_STATUS_NO_UPDATE,
    UPDATE_STATUS_NEEDS_UPDATE,
    UPDATE_STATUS_SKIPPED_UPDATE
};
//=============================================================================

//=============================================================================
// CUpdater
//=============================================================================
class CUpdater
{
    SINGLETON_DEF(CUpdater)

private:
    CHTTPRequest*   m_pRequest;
    string          m_sMasterServerURL;
    CFileHTTP       m_fileHTTP;

    tstring     m_sPrimaryServer;
    tstring     m_sSecondaryServer;

    bool        m_bRequireConfirmation;

    uint        m_uiLastSpeedUpdate;
    uint        m_uiCurDownloaded;
    uint        m_uiTotalResumed;
    uint        m_uiTotalSize;
    uint        m_uiTotalFiles;
    uint        m_uiTotalSized;

    uint        m_uiNumActive;
    uint        m_uiNumComplete;
    uint        m_uiNumSpeedUpdates;
    uint        m_uiFirstSpeedUpdate;

    bool        m_bCheckingForUpdate;

    UpdVector   m_vUpdateFiles;
    DelVector   m_vDeleteFiles;

    SFileManifest m_cNewManifest;
    SFileManifest m_cOldManifest;

    tstring     m_sUpdateVersion;
    tstring     m_sCompatVersion;

    bool        m_bDownloadingCompat;

    EUpdaterStatus  m_eStatus;

    void    GetUpdate(const tstring &sFile, const tstring &sVersion, const tstring &sOS, const tstring &sArch, uint uiSize, uint uiFullSize, uint uiChecksum, const tstring &sOldVersion);

    void    ResponseOK(CPHPData &response);
    void    ResponseVersion(CPHPData &response);
    void    ResponseError(CPHPData &response);

    void    CompatResponse(CPHPData &response);
    void    CompatResponseError();

    void    FrameAwaitingResponse();
    void    FrameDownloading();
        
    void    ApplyUpdate();
    
    void    ResponseVersionError();
    void    ErrorDownloading();

    void    CancelApplyUpdate(ZipMap &mapZipFiles);

    void    AutoUpdateCompatArchives(float fLoadingScale, float fLoadingOffset);
    void    FrameAwaitingCompatResponse();

    void    UpdateCompatArchives();

    void    CleanupUpdateFiles();

    void    ReadFileList(const tstring &sFile, sset &setFiles);

public:
    ~CUpdater();

    void        Initialize();

    void        CheckForUpdates(bool bRequireConfirmation);
    K2_API void SilentUpdate();
    void        CancelUpdate();
    void        UpdateConfirm(bool bUpdate);
    void        StartUpdate();
    void        DownloadCompat(const tstring &sVersion);
    void        CancelCompatDownload();
    void        ForceUpdate(const tstring &sVersion, const tstring &sPrimaryServer, const tstring &sSecondaryServer, bool bRequireConfirmation);

    void        GenerateIgnore(const tstring &sFilename);
    void        GenerateResources();

    void        GenerateUpdate(bool bResourceFilesOnly = false);
    void        UploadUpdate(const tstring &sAddress, const tstring &sUser, const tstring &sPass);
    void        UploadPrevious(const tstring &sAddress, const tstring &sUser, const tstring &sPass);
    void        UpdateDB(const tstring &sUsername, const tstring &sPassword);

    void        Frame();

    EUpdaterStatus  GetStatus() const       { return m_eStatus; }

    static inline   tstring     CtoA(uint uiChecksum)       { return XtoA(uiChecksum, FMT_NOPREFIX, 0, 16); }
    static inline   tstring     VtoA(uint uiVersion);
    static inline   tstring     VtoA2(uint uiVersion);
};
//=============================================================================

//=============================================================================
// Inline functions
//=============================================================================

/*====================
  CUpdater::VtoA
  ====================*/
inline
tstring CUpdater::VtoA(uint uiVersion)
{
    tstring sRet;
    sRet += XtoA((uiVersion & 0xff000000) >> 24);
    sRet += _T(".");
    sRet += XtoA((uiVersion & 0x00ff0000) >> 16);
    sRet += _T(".");
    sRet += XtoA((uiVersion & 0x0000ff00) >> 8);
    sRet += _T(".");
    sRet += XtoA((uiVersion & 0x000000ff) >> 0);

    return sRet;
}


/*====================
  CUpdater::VtoA2
  ====================*/
inline
tstring CUpdater::VtoA2(uint uiVersion)
{
    tstring sRet;
    sRet += XtoA((uiVersion & 0xff000000) >> 24);
    sRet += _T(".");
    sRet += XtoA((uiVersion & 0x00ff0000) >> 16);
    sRet += _T(".");
    sRet += XtoA((uiVersion & 0x0000ff00) >> 8);

    if ((uiVersion & 0x000000ff) > 0)
    {
        sRet += _T(".");
        sRet += XtoA((uiVersion & 0x000000ff) >> 0);
    }

    return sRet;
}
//=============================================================================

#endif // __C_UPDATER_H__
