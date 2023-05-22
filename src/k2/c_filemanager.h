// (C)2005 S2 Games
// c_filemanager.h
//
//=============================================================================
#ifndef __C_FILEMANAGER__
#define __C_FILEMANAGER__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"
#include "stringutils.h"
#include "c_archive.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CFile;
typedef set<tstring>                    StringSet;
typedef set<tstring>::iterator          StringSet_it;
typedef set<tstring>::const_iterator    StringSet_cit;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
struct block_t
{
    char    name[5];
    size_t  pos;        //position in buffer
    uint    length;
    byte*   data;       //data pointer
};

struct SManifestEntry
{
    uint uiSize;
    uint uiChecksum;
    uint uiVersion;
    uint uiZipSize;
};

typedef map<tstring, SManifestEntry>    ManifestEntryMap;

struct SFileManifest
{
    tstring             sVersion;
    tstring             sOS;
    tstring             sArch;

    ManifestEntryMap        mapManifestFiles;
};

typedef map<tstring, tstring>   CompatMap;
//=============================================================================

//=============================================================================
// CFileManager
//=============================================================================
class CFileManager
{
SINGLETON_DEF(CFileManager)

private:
    tstring             m_sCurrentDir;
    tstring             m_sEditPath;

    CArchive*           m_pWorldArchive;
    
    vector<CArchive*>   m_vArchiveList;
    tsvector            m_vModPathStack;

    bool                m_bCompatDisabled;
    CArchive*           m_pCompatArchive;
    tstring             m_sCompatVersion;
    tstring             m_sCompatVersionShort;
    uint                m_uiCompatVersion;

    SFileManifest       m_cBaseManifest;
    SFileManifest       m_cCompatManifest;

    CompatMap           m_mapCompatFiles;
    StringSet           m_setNewCompatFiles;
    StringSet           m_setDeletedCompatFiles;

    StringSet           m_setCustomFiles;
    StringSet           m_setCustomArchives;
    bool                m_bUsingCustomFiles;
    bool                m_bCoreFilesModified;
    tstring             m_sModifiedCoreFile;

    uint*               m_uiCRC32;

    void                InitCRC32();

public:
    ~CFileManager()         { SAFE_DELETE(m_uiCRC32); }

    void                    Initialize();

    K2_API tstring          SanitizePath(const tstring &sPath, bool bResolveToRoot = true);
    K2_API bool             IsCleanPath(const tstring &sPath, bool bResolveToRoot = true);
    K2_API tstring          GetSystemPath(const tstring &sPath, const tstring &sMod = TSNULL, bool bWrite = false, bool bForce = false, bool bFileOnly = false, tstring *psBasePathOut = nullptr);
    K2_API tstring          GetGamePath(const tstring &sSystemPath);
    K2_API tstring          GetLibraryPath(const tstring &sLibFilename);

    const tstring&          GetWorkingDirectory()   { return m_sCurrentDir; }

    K2_API void             SetWorkingDirectory(const tstring &sPathName);

    K2_API void             SetEditPath(const tstring &sPath);
    K2_API const tstring&   GetEditPath();
    K2_API const tstring&   GetModPath(uint uiDepth);
    K2_API const tstring&   GetTopModPath();
    K2_API const tsvector&  GetModStack()           { return m_vModPathStack; }

    K2_API void             SetModStack(const tsvector &sModStack);
    K2_API void             ChangeModStack(const tsvector &sModStack);
    K2_API void             PushMod(const tstring &sMod);
    K2_API void             ListMods();

    K2_API void             SetWorldArchive(CArchive *pWorldArchive)                { m_pWorldArchive = pWorldArchive; }

    K2_API void             RegisterArchives(const tstring &sMod, bool bRecurse = false);
    K2_API void             UnregisterArchives(const tstring &sMod, bool bRecurse = false);
    K2_API CArchive*        FindInArchives(const tstring &sPath, const tstring &sMod = _T(""));
    K2_API tstring          FindFilePath(const tstring &sPath, int iMode, const tstring &sMod = _T(""));

    K2_API bool             RegisterArchive(const tstring &sPath);
    K2_API void             UnregisterArchive(const tstring &sPath);
    K2_API CArchive*        GetArchive(const tstring &sPath, const tstring &sMod);

    K2_API bool             Exists(const tstring &sPath, int iMode = 0, const tstring &sMod = _T(""));
    K2_API bool             DirectoryExists(const tstring &sPath, int iMode = 0, const tstring &sMod = _T(""));
    K2_API bool             Stat(const tstring &sPath, struct _stat &stats, const tstring &sRequestedMod = _T(""));
    K2_API bool             Delete(const tstring &sPath, const tstring &sMod = _T(""));
    K2_API bool             DeleteTree(const tstring &sPath, const tstring &sMod = _T(""));
    K2_API bool             Rename(const tstring &sOldPath, const tstring &sNewPath, const tstring &sMod = _T(""));

    K2_API bool             MakeDir(const tstring &sRequestedPath);

    K2_API bool             GetFileList(const tstring &sPath, const tstring &sFile, bool bRecurse, tsvector &vFileList, bool bCheckOutsideArchives = false);
    K2_API bool             GetDirList(const tstring &sPath, bool bRecurse, tsvector &vDirList);
    K2_API bool             GetFileListCompat(const tstring &sPath, const tstring &sFile, bool bRecurse, tsvector &vFileList);

    K2_API tstring          GetNextFileIncrement(int zNumDigits, const tstring &sBaseName, const tstring &sExt, int iStart = 0);

    K2_API CFile*           GetFile(const tstring &sPath, int iMode, const tstring &sMod = TSNULL);

    K2_API bool             BuildBlockList(const char *pBuffer, size_t iBufLen, vector<block_t> &vBlockList);

    uint                    GetCRC32ForArchive(const tstring &sFile);
    uint                    GetCRC32(const tstring &sFile);
    uint                    GetCRC32(const char *pBuf, size_t zLen);

    K2_API bool             IsCompatVersionSupported(const tstring &sVersion);
    K2_API bool             OpenCompatArchive();
    K2_API bool             CloseCompatArchive();
    K2_API bool             DisableCompatArchive();
    K2_API bool             EnableCompatArchive();

    K2_API bool             SetCompatVersion(const tstring &sVersion);
    const tstring&          GetCompatVersion() const        { return m_sCompatVersion; }

    K2_API CFile*           GetCompatFile(const tstring &sPath, int iMode, bool &bDeleted);
    K2_API tstring          GetGamePathFromCompatPath(const tstring &sCompatPath);

    bool                    GetUsingCustomFiles() const     { return m_bUsingCustomFiles; }
    const StringSet&        GetCustomFilesList() const      { return m_setCustomFiles; }
    const StringSet&        GetCustomArchivesList() const   { return m_setCustomArchives; }

    bool                    GetCoreFilesModified() const    { return m_bCoreFilesModified; }
    const tstring&          GetModifiedCoreFilePath() const { return m_sModifiedCoreFile; }
    void                    CoreFilesModified(const tstring &sModifiedFile)     { m_bCoreFilesModified = true; m_sModifiedCoreFile = sModifiedFile; }
};

extern K2_API CFileManager *pFileManager;
#define FileManager (*pFileManager)
#ifdef K2_EXPORTS
#define pFileManager CFileManager::GetInstance()
#endif
//=============================================================================
#endif
