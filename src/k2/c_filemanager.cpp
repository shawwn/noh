// (C)2005 S2 Games
// c_filemanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include <sys/stat.h>
#ifdef _WIN32
#include <io.h>
#include <direct.h>
#endif

#include <zlib.h>

#include "stringutils.h"
#include "c_archive.h"
#include "c_filemanager.h"
#include "c_system.h"
#include "c_file.h"
#include "c_filedisk.h"
#include "c_filehttp.h"
#include "c_filearchive.h"
#include "c_cmd.h"
#include "c_uicmd.h"
#include "c_function.h"
#include "i_widget.h"
#include "c_xmlmanager.h"
#include "c_updater.h"
#include "c_world.h"
#include "c_resourcemanager.h"

#undef pFileManager
#undef FileManager
//=============================================================================


//=============================================================================
// Definitions
//=============================================================================
CVAR_BOOLF      (fs_trackCustomFiles,       false,              CVAR_SAVECONFIG);
CVAR_BOOLF      (fs_disablemods,            false,              CVAR_SAVECONFIG);
CVAR_BOOL       (fs_smallMMap,              false);
//=============================================================================


//=============================================================================
// Globals
//=============================================================================
CFileManager    *pFileManager(CFileManager::GetInstance());

SINGLETON_INIT(CFileManager)
//=============================================================================

/*====================
  CFileManager::CFileManager
  ====================*/
CFileManager::CFileManager() :
m_sCurrentDir(_T("/")),
m_sEditPath(_T("game")),
m_pWorldArchive(NULL),
m_pCompatArchive(NULL),
m_bCompatDisabled(false),
m_uiCRC32(NULL),
m_uiCompatVersion(0),
m_bUsingCustomFiles(false),
m_bCoreFilesModified(false)
{   
}


/*====================
  CFileManager::Initialize
  ====================*/
void    CFileManager::Initialize()
{
    if (K2System.GetVirtualMemoryLimit() < 2200000000u)
    {
        Console << _T("Limited virtual memory detected, forcing fs_smallMMap") << newl;
        fs_smallMMap = true;
    }

    m_vArchiveList.reserve(32);
    m_vModPathStack.push_back(_T("base"));

    InitCRC32();
}


/*====================
  CFileManager::SanitizePath

  Modifies a string conatining a path to a standard format, and adjusts
  relative paths (those not beginning with a /) to be absolute paths (relative
  to the game's root directory)

  return value is a reference to the pathName that has been modified
  ====================*/
tstring CFileManager::SanitizePath(const tstring &sPath, bool bResolveToRoot)
{
    size_t pos(0);
    tstring sReturn(sPath);

    if (sPath.empty() || sPath[0] == _T('$') || sPath[0] == _T('!') || sPath[0] == _T('*'))
        return sPath;

    // Keep drive letter lower case
    if (sPath[1] == _T(':'))
        sReturn[0] = tolower(sPath[0]);

    // Convert all \ to /
    while ((pos = sReturn.find(_T('\\'), pos)) != tstring::npos)
        sReturn.replace(pos, 1, _T("/"));

    // If the path is not absolute (starts with a /), prepend the current path
    // unless it has a protocol marker in it
    // Other special markers:
    //   ~ means to use the writeable config path
    //   # means to use the top level only of the writeable config path
    //   @ means to use the edit path
    //   : means to use the root path
    if (bResolveToRoot)
    {
        if (sReturn[0] == _T('~') ||
            sReturn[0] == _T('#') ||
            sReturn[0] == _T('@') ||
            sReturn[0] == _T(':'))
        {
            if (sReturn.length() > 1)
            {
                if (sReturn[1] != _T('/'))
                    sReturn = sReturn[0] + m_sCurrentDir + sReturn.substr(1, tstring::npos);
            }
        }
        else if (sReturn[0] != _T('/') &&
            sReturn.find(_T("://")) == tstring::npos)
        {
            sReturn = m_sCurrentDir + sReturn;
        }

    }

    // Remove any instances of consecutive //// (except for protocol markers)
    pos = 0;
    while ((pos = sReturn.find(_T("//"), pos)) != tstring::npos)
    {
        if (pos > 0 && sReturn[pos - 1] == _T(':'))
        {
            pos += 1;
            continue;
        }

        size_t end = sReturn.find_first_not_of(_T("/"), pos);
        if (end > sReturn.length())
            sReturn = sReturn.substr(0, pos + 1);
        else
            sReturn = sReturn.substr(0, pos + 1) + sReturn.substr(end);
    }

    // Clean out all the ".."
    // Mostly, this just shortens the path to a standard format so that it can
    // be safely compared, but it will also discard any /.. that try to go above
    // the root game directory
    pos = 0;
    while ((pos = sReturn.find(_T("/.."))) != tstring::npos)
    {
        // if the directory tries to go above the root, stop it there
        if (sReturn.find(_T("/")) >= pos)
        {
            size_t next = sReturn.find(_T("/"), pos + 1);
            if (next > sReturn.length())
                sReturn = _T("/");
            else
                sReturn = sReturn.substr(next);

            continue;
        }

        size_t end = sReturn.find(_T("/"), pos + 1);
        if (end > sReturn.length())
            sReturn = sReturn.substr(0, sReturn.find_last_of(_T("/"), pos - 1) + 1);
        else
            sReturn = sReturn.substr(0, sReturn.find_last_of(_T("/"), pos - 1) + 1) + sReturn.substr(end + 1);
    }

    return sReturn;
}


/*====================
  CFileManager::IsCleanPath
  ====================*/
bool    CFileManager::IsCleanPath(const tstring &sPath, bool bResolveToRoot)
{
    if (sPath.empty() || sPath[0] == _T('$') || sPath[0] == _T('!'))
        return true;

    size_t pos(0);

    // Drive letter lower case
    if (sPath[1] == _T(':'))
        if (!islower(sPath[0]))
            return false;

    // No backslash
    if (sPath.find(_T('\\'), pos) != tstring::npos)
        return false;

    // Absolute paths
    if (bResolveToRoot)
    {
        if (sPath[0] == _T('~') ||
            sPath[0] == _T('#') ||
            sPath[0] == _T('@') ||
            sPath[0] == _T(':'))
        {
            return false;
        }
        else if (sPath[0] != _T('/') && sPath.find(_T("://")) == tstring::npos)
        {
            return false;
        }

    }

    // Consecutive forward slash (except for protocol markers)
    pos = 0;
    while ((pos = sPath.find(_T("//"), pos)) != tstring::npos)
    {
        if (pos > 0 && sPath[pos - 1] == _T(':'))
        {
            pos += 1;
            continue;
        }

        return false;
    }

    // No /..
    pos = 0;
    if ((pos = sPath.find(_T("/.."))) != tstring::npos)
        return false;

    return true;
}


/*====================
  CFileManager::SetWorkingDirectory
  ====================*/
void    CFileManager::SetWorkingDirectory(const tstring &sPathName)
{
    tstring sNewPath = SanitizePath(sPathName);
    if (!sNewPath.empty())
        m_sCurrentDir = sNewPath;
}


/*====================
  CFileManager::SetEditPath
  ====================*/
void    CFileManager::SetEditPath(const tstring &sPath)
{
    m_sEditPath = SanitizePath(sPath, false);
    Console.Dev << _T("Edit path set to: ") << m_sEditPath << newl;
}


/*====================
  CFileManager::GetEditPath
  ====================*/
const tstring& CFileManager::GetEditPath()
{
    return m_sEditPath;
}


/*====================
  CFileManager::GetModPath
  ====================*/
const tstring&  CFileManager::GetModPath(uint uiDepth)
{
    if (uiDepth >= m_vModPathStack.size())
        return TSNULL;

    return m_vModPathStack[m_vModPathStack.size() - uiDepth - 1];
}


/*====================
  CFileManager::GetTopModPath
  ====================*/
const tstring& CFileManager::GetTopModPath()
{
    return m_vModPathStack.back();
}


/*====================
  CFileManager::GetSystemPath

  Modifies a string containing a path to include the full path
  as the system sees it

  This is where the special path characters are replaced:
    ~ writeable config directory
    @ editing directory
  ====================*/
tstring CFileManager::GetSystemPath(const tstring &sPath, const tstring &sMod, bool bWrite, bool bForce, bool bFileOnly, tstring *psBasePathOut)
{
    PROFILE("CFileManager::GetSystemPath");

    if (sPath.empty())
        return TSNULL;

    // If no mod is specified, we should try each one on the stack
    if (sMod.empty())
    {
        tsvector::iterator it(m_vModPathStack.end());
        while (it != m_vModPathStack.begin())
        {
            --it;

            const tstring &sReturn(GetSystemPath(sPath, *it, bWrite, false, bFileOnly, NULL));
            if (!sReturn.empty())
            {
                if (psBasePathOut != NULL)
                {
                    if (sPath[0] == _T(':'))
                        *psBasePathOut = IsCleanPath(sPath) ? sPath : SanitizePath(sPath);
                    else
                        *psBasePathOut = _T(":/") + *it + (IsCleanPath(sPath) ? sPath : SanitizePath(sPath));
                }

                return sReturn;
            }
        }

        if (bWrite)
            return GetSystemPath(sPath, m_vModPathStack.back(), bWrite, true, bFileOnly, NULL);
    }

    // Determine the absolute system path
    tstring sAbsolutePath(IsCleanPath(sPath) ? sPath : SanitizePath(sPath));
    switch (sAbsolutePath[0])
    {
    case '/':
        if (sMod.empty())
            sAbsolutePath = K2System.GetRootDir() + sAbsolutePath.substr(1);
        else
            sAbsolutePath = K2System.GetRootDir() + sMod + sAbsolutePath;
        break;

    case '~':
        // ignore non-top mods while writing to the settings directory
        if (bWrite && sMod != m_vModPathStack.back())
            return _T("");

        if (sAbsolutePath[1] == _T('/'))
            sAbsolutePath = K2System.GetUserDir() + sMod + sAbsolutePath.substr(1);
        else
            sAbsolutePath = K2System.GetUserDir() + sMod + sAbsolutePath;

        break;

    case '#':
        // always ignore non top mods
        if (sMod != m_vModPathStack.back())
            return _T("");

        if (sAbsolutePath[1] == _T('/'))
            sAbsolutePath = K2System.GetUserDir() + sMod + sAbsolutePath.substr(1);
        else
            sAbsolutePath = K2System.GetUserDir() + sMod + sAbsolutePath;

        break;

    case '@':
        sAbsolutePath[0] = _T('/');
#if defined(linux) || defined(__APPLE__) // in the user dir
        sAbsolutePath = K2System.GetUserDir() + m_sEditPath + sAbsolutePath;
#else
        sAbsolutePath = K2System.GetRootDir() + m_sEditPath + sAbsolutePath;
#endif
        break;

    case ':':
        sAbsolutePath[0] = _T('/');
        sAbsolutePath = K2System.GetRootDir() + sAbsolutePath;
        break;

    default:
        Console.Warn << _T("Invalid starting character for path: ") << sAbsolutePath << newl;
        return _T("");
    }
    if (!IsCleanPath(sAbsolutePath, false))
        sAbsolutePath = SanitizePath(sAbsolutePath, false);

    if (!bForce)
    {
        PROFILE("Test");

        // Validate that the file or path exists
        if (sAbsolutePath[sAbsolutePath.length() - 1] == _T('/')) 
        {
            // Directory
            if (_taccess(TStringToNative(sAbsolutePath.substr(0, sAbsolutePath.length() - 1)).c_str(), 0) == -1)
                return _T("");
        }
        else
        {
            if (_taccess(TStringToNative(sAbsolutePath).c_str(), 0) == -1)
                return _T("");
        }
    }

    return sAbsolutePath;
}


/*====================
  CFileManager::Exists
  ====================*/
bool    CFileManager::Exists(const tstring &sPath, int iMode, const tstring &sMod)
{
    PROFILE("CFileManager::Exists");

    if (sPath.empty())
        return false;

    if (!(iMode & FILE_NOARCHIVES))
    {
        // World archive
        if (m_pWorldArchive != NULL)
        {
            tsvector vArchive;

            m_pWorldArchive->GetFileList(vArchive);

            for (tsvector_it it(vArchive.begin()); it != vArchive.end(); it++)
            {
                // Ignore directory entries
                if ((*it)[it->length() - 1] == _T('/'))
                    continue;

                // Ignore files not stored as resources
                if (CompareNoCase(it->substr(0, 10), _T("/resources")) != 0)
                    continue;

                if (CompareNoCase((*it).substr(10), sPath) == 0)
                    return true;
            }
        }
        if (FindInArchives(sPath, sMod))
            return true;
    }

    const tstring &sSystemPath(GetSystemPath(sPath, sMod, false, false, true));

    if (!sSystemPath.empty())
        return true;

    if (sPath[0] != _T('~') && sPath[0] != _T('#') && sPath[0] != _T('@') && sPath[0] != _T(':') && !(iMode & FILE_NOUSERDIR))
    {
        const tstring &sSystemPathUser(GetSystemPath(_TS("~") + sPath, sMod, false, false, true));

        if (!sSystemPathUser.empty())
            return true;
    }
    
    return false;
}


/*====================
  CFileManager::DirectoryExists
  ====================*/
bool    CFileManager::DirectoryExists(const tstring &sPath, int iMode, const tstring &sMod)
{
    if (sPath.empty())
        return false;

    const tstring &sSystemPath(GetSystemPath(sPath, sMod, false, false, false));

    if (!sSystemPath.empty())
        return true;

    const tstring &sSystemPathUser(GetSystemPath(_TS("~") + sPath, sMod, false, false, false));

    if (!sSystemPathUser.empty())
        return true;

    if (FindInArchives(sPath))
        return true;

    return false;
}


/*====================
  CFileManager::Stat
  ====================*/
bool    CFileManager::Stat(const tstring &sPath, struct _stat &stats, const tstring &sMod)
{
    assert(!sPath.empty());
    if (sPath.empty())
        return false;

    const tstring &sFilename(GetSystemPath(sPath, sMod));

    return _tstat(TStringToNative(sFilename).c_str(), &stats) == 0;
}


/*====================
  CFileManager::Delete
  ====================*/
bool    CFileManager::Delete(const tstring &sPath, const tstring &sMod)
{
    tstring sSystemPath(GetSystemPath(sPath, sMod));

    if (_tremove(TStringToNative(sSystemPath).c_str()) == 0)
    {
        return true;
    }
    else
    {
        tstring sReason(_T("unknown error"));
        switch(errno)
        {
        case EACCES:
            sReason = _T("permission denied");
            break;

        case ENOENT:
            sReason = _T("file not found");
            break;
        }
        Console.Err << _T("Can not delete ") << sPath << SPACE << ParenStr(sReason) << newl;
        return false;
    }
}


/*====================
  CFileManager::Rename
  ====================*/
bool    CFileManager::Rename(const tstring &sOldPath, const tstring &sNewPath, const tstring &sMod)
{
    tstring sOldSystemPath(GetSystemPath(sOldPath, sMod));
    tstring sNewSystemPath(GetSystemPath(sNewPath, sMod, true, true));

    if (_trename(TStringToNative(sOldSystemPath).c_str(), TStringToNative(sNewSystemPath).c_str()) == 0)
    {
        return true;
    }
    else
    {
        tstring sReason(_T("unknown error"));
        switch(errno)
        {
        case EACCES:
            sReason = _T("permission denied");
            break;

        case ENOENT:
            sReason = _T("file not found");
            break;

        case EINVAL:
            sReason = _T("invalid characters in file name");
            break;
        }
        Console.Err << _T("Can not rename ") << sOldSystemPath << _T(" to ") << sNewPath << SPACE << ParenStr(sReason) << newl;
        return false;
    }
}


/*====================
  CFileManager::DeleteTree
  ====================*/
bool    CFileManager::DeleteTree(const tstring &sPath, const tstring &sMod)
{
    tstring sSystemPath(GetSystemPath(sPath, sMod));

    if (sSystemPath.empty())
        return true;

    tsvector vDirList;
    GetDirList(sPath, false, vDirList);
    for (tsvector_it it(vDirList.begin()); it != vDirList.end(); ++it)
        DeleteTree(*it, sMod);

    tsvector vFileList;
    GetFileList(sPath, _T("*"), false, vFileList);
    for (tsvector_it it(vFileList.begin()); it != vFileList.end(); ++it)
        Delete(*it, sMod);

    if (_trmdir(TStringToNative(sSystemPath).c_str()) == 0)
    {
        return true;
    }
    else
    {
        tstring sReason;
        switch(errno)
        {
        case EACCES:
            sReason = _T("permission denied");
            break;

        case ENOENT:
            sReason = _T("file not found");
            break;

        default:
            sReason = _T("unknown error");
        }
        Console << _T("Can not delete ") << sPath << ParenStr(sReason) << newl;
        return false;
    }
}


/*====================
  CFileManager::MakeDir
  ====================*/
bool    CFileManager::MakeDir(const tstring &sRequestedPath)
{
    tstring sPath = GetSystemPath(sRequestedPath + _T("/"), _T(""), true);

    return K2System.MakeDir(sPath);
}


/*====================
  CFileManager::SetModStack
  ====================*/
void    CFileManager::SetModStack(const tsvector &sModStack)
{
    for (tsvector_it it(m_vModPathStack.begin()); it != m_vModPathStack.end(); ++it)
        UnregisterArchives(*it);

    m_vModPathStack.clear();
    m_vModPathStack.push_back(_T("base"));
    m_vModPathStack.insert(m_vModPathStack.end(), sModStack.begin(), sModStack.end());

    for (tsvector_it it(m_vModPathStack.begin()); it != m_vModPathStack.end(); ++it)
        RegisterArchives(*it);
}


/*====================
  CFileManager::ChangeModStack
  ====================*/
void    CFileManager::ChangeModStack(const tsvector &sModStack)
{
    SetModStack(sModStack);
}


/*====================
  CFileManager::PushMod
  ====================*/
void    CFileManager::PushMod(const tstring &sMod)
{
    m_vModPathStack.push_back(sMod);
    RegisterArchives(sMod);

    tsvector vFileList;

    // Files in current mod directory
    K2System.GetFileList(_T("/"), _T("*"), true, vFileList, sMod);

    // Check each registered archive
    for (vector<CArchive*>::iterator itArchive(m_vArchiveList.begin()), itEnd(m_vArchiveList.end()); itArchive != itEnd; ++itArchive)
    {
        // Only process the archive on this pass if it belongs to the current mod
        if ((*itArchive)->GetMod() != sMod)
            continue;

        tsvector vArchiveFileList;
        (*itArchive)->GetFileList(vArchiveFileList);

        // Search each file in the archive
        for (tsvector_it itFileName(vArchiveFileList.begin()), itEnd(vArchiveFileList.end()); itFileName != itEnd; ++itFileName)
            vFileList.push_back(*itFileName);
    }

    // Reload any changed files
    for (tsvector_it it(vFileList.begin()), itEnd(vFileList.end()); it != itEnd; ++it)
        g_ResourceManager.Reload(*it);

    g_ResourceManager.GetLib(RES_STRINGTABLE)->ReloadAll();
}


/*====================
  CFileManager::ListMods
  ====================*/
void    CFileManager::ListMods()
{
    Console << _T("Active mods:") << newl
            << _T("------------") << newl;

    tsvector::reverse_iterator it;
    for (it = m_vModPathStack.rbegin(); it != m_vModPathStack.rend(); ++it)
        Console << *it << newl;
}


/*====================
  CFileManager::RegisterArchives

  Searches sPath for archives matching sFile and adds them to the list of valid
  archives to search if a file is not found on disk.

  The list is sorted before being added, however it is NOT sorted with archives
  that have already been registered, so early registration takes precedence
  ====================*/
void    CFileManager::RegisterArchives(const tstring &sMod, bool bRecurse)
{
    tsvector vFileList;
    
#if defined(linux) || defined(__APPLE__)
    // pick up resources*.s2z files in "~" (so mods can go here instead of the game dir)
    K2System.GetFileList(_T("~/"), _T("resources*.s2z"), bRecurse, vFileList, sMod);
    for (tsvector::iterator it(vFileList.begin()); it != vFileList.end(); ++it)
    {
        // strip the ~
        (*it) = (*it).substr(1);
    }
#endif
    
    K2System.GetFileList(_T("/"), _T("resources*.s2z"), bRecurse, vFileList, sMod);

    sort(vFileList.begin(), vFileList.end());

    while (!vFileList.empty())
    {
        tstring sArchivePath(vFileList.back());
        vFileList.pop_back();

        // determine whether the archive is a resources0 archive.
        const tstring sResources0(_T("resources0.s2z"));
        size_t uiFindPos = LowerString(sArchivePath).find(sResources0);
        bool bResources0(true);
        if (uiFindPos == tstring::npos || (sArchivePath.size() - uiFindPos) != sResources0.size())
            bResources0 = false;

        // if mods are disabled, then only allow resources0 archives.
        if (fs_disablemods)
            if (!bResources0)
                continue;

        // store and compare paths in lowercase
        tstring sPathLower(LowerString(sMod + sArchivePath));

        // only allow each archive to be included once
        if (find(m_vArchiveList.begin(), m_vArchiveList.end(), sPathLower) != m_vArchiveList.end())
            continue;

        // try to open the archive
        CArchive *pNewArchive(K2_NEW(ctx_FileSystem,  CArchive));
        if (bResources0)
            pNewArchive->SetRequireChecksums(true);
        if (!pNewArchive->Open(sArchivePath, ARCHIVE_READ, sMod))
            continue;
        
        // detect custom files.
        if (!bResources0)
        {
            m_bUsingCustomFiles = true;
            if (fs_trackCustomFiles)
                m_setCustomArchives.insert(sArchivePath);
        }

        // store it in the list
        m_vArchiveList.push_back(pNewArchive);
        Console << _T("Registered archive: ") << sMod << sArchivePath << newl;
    }
}


/*====================
  CFileManager::UnregisterArchives

  Searches sPath for archives matching sFile and adds them to the list of valid
  archives to search if a file is not found on disk.

  The list is sorted before being added, however it is NOT sorted with archives
  that have already been registered, so early registration takes precedence
  ====================*/
void    CFileManager::UnregisterArchives(const tstring &sMod, bool bRecurse)
{
    tsvector vFileList;

    K2System.GetFileList(_T("/"), _T("*.s2z"), bRecurse, vFileList, sMod);

    sort(vFileList.begin(), vFileList.end());
    reverse(vFileList.begin(), vFileList.end());
    while (!vFileList.empty())
    {
        tstring sArchivePath(vFileList.back());
        vFileList.pop_back();

        // store and compare paths in lowercase
        tstring sPathLower(LowerString(sMod + sArchivePath));

        vector<CArchive *>::iterator itFind(find(m_vArchiveList.begin(), m_vArchiveList.end(), sPathLower));

        // only allow each archive to be included once
        if (itFind != m_vArchiveList.end())
        {
            (*itFind)->Close();
            K2_DELETE(*itFind);
            m_vArchiveList.erase(itFind);
            Console << _T("Unregistered archive: ") << sMod << sArchivePath << newl;
        }
        else
        {
            Console << _T("Unable to unregister archive: ") << sMod << sArchivePath << newl;
            continue;
        }
    }
}


/*====================
  CFileManager::RegisterArchive
  ====================*/
bool    CFileManager::RegisterArchive(const tstring &sPath)
{
    tstring sAbsolutePath(SanitizePath(sPath));

    // store and compare paths in lowercase
    tstring sPathLower(LowerString(sAbsolutePath));

    // only allow each archive to be included once
    if (find(m_vArchiveList.begin(), m_vArchiveList.end(), sPathLower) != m_vArchiveList.end())
        return true;

    // try to open the archive
    CArchive *pNewArchive(K2_NEW(ctx_FileSystem,  CArchive)(sAbsolutePath));
    if (!pNewArchive->IsOpen())
    {
        K2_DELETE(pNewArchive);
        return false;
    }

    // store it in the list
    m_vArchiveList.push_back(pNewArchive);
    Console << _T("Registered archive: ") << sAbsolutePath << newl;
    return true;
}


/*====================
  CFileManager::UnregisterArchive
  ====================*/
void    CFileManager::UnregisterArchive(const tstring &sPath)
{
    const tstring &sAbsolutePath(SanitizePath(sPath));

    vector<CArchive *>::iterator it(m_vArchiveList.begin());
    while (it != m_vArchiveList.end())
    {
        if ((*it)->GetPath() == sAbsolutePath)
        {
            if ((*it)->IsOpen())
                (*it)->Close();

            K2_DELETE(*it);
            it = m_vArchiveList.erase(it);
        }
        else
            ++it;
    }
}


/*====================
  CFileManager::GetArchive
  ====================*/
CArchive*   CFileManager::GetArchive(const tstring &sPath, const tstring &sMod)
{
    tstring sLowerPath(LowerString(sPath));
    for (vector<CArchive *>::iterator it(m_vArchiveList.begin()), itEnd(m_vArchiveList.end()); it != itEnd; ++it)
    {
        CArchive *pArchive(*it);
        if (pArchive != NULL)
        {
            if (CompareNoCase(pArchive->GetPath(), sLowerPath) == 0)
            {
                if (CompareNoCase(pArchive->GetMod(), sMod) == 0)
                    return pArchive;
            }
        }
    }

    return NULL;
}


/*====================
  CFileManager::FindInArchives

  Returns a pointer to the first archive in the list of registered
  archives that the file appears in, or NULL if it cannot be found
  ====================*/
CArchive*   CFileManager::FindInArchives(const tstring &sPath, const tstring &sMod)
{
    PROFILE("CFileManager::FindInArchives");

    if (sMod.empty())
    {
        for (vector<CArchive *>::iterator it(m_vArchiveList.begin()); it != m_vArchiveList.end(); ++it)
        {
            if ((*it)->ContainsFile(sPath))
                return (*it);
        }
    }
    else
    {
        for (vector<CArchive *>::iterator it(m_vArchiveList.begin()); it != m_vArchiveList.end(); ++it)
        {
            if ((*it)->GetMod() == sMod && (*it)->ContainsFile(sPath))
                return (*it);
        }
    }

    return NULL;
}


/*====================
  CFileManager::FindFilePath
  ====================*/
tstring CFileManager::FindFilePath(const tstring &sFilename, int iMode, const tstring &sMod)
{
    PROFILE("CFileManager::FindFilePath");

    tstring sPath(sFilename);

    try
    {
        if (sPath.empty())
            return NULL;

        EFileType eType(FILE_INVALID);

        // Check for a protocol that is supported
        size_t zMarker(sPath.find(_T("://")));
        if (zMarker < sPath.length())
        {
            tstring sProtocol(sPath.substr(0, zMarker));
            if (sProtocol == _T("http"))
                eType = FILE_HTTP;
            else
                EX_ERROR(_T("The protocol ") + QuoteStr(sProtocol) + _T(" is not supported"));
        }

        CArchive *pArchive(NULL);
        for (tsvector::reverse_iterator it(m_vModPathStack.rbegin()); it != m_vModPathStack.rend(); ++it)
        {
            const tstring &sModName(sMod.empty() ? *it : sMod);

            // Search for the file on disk
            if (eType == FILE_INVALID)
            {
                if (iMode & FILE_WRITE)
                {
                    assert(!"todo");
                    return TSNULL;
#if 0
                    tstring sPathToFile(Filename_GetPath(sPath));
                    MakeDir(sPathToFile);
                    sPath = GetSystemPath(sPathToFile); // figure out which directory we're going to write to
                    if (!sPath.empty())
                    {
                        sPath += Filename_StripPath(sFilename);
                        eType = FILE_DISK;
                    }
                    break;
#endif
                }
                else
                {
                    if (!GetCompatVersion().empty() && !(iMode & FILE_NOCOMPAT))
                    {
                        tstring sAbsolutePath(IsCleanPath(sPath) ? sPath : SanitizePath(sPath));

                        if (sAbsolutePath[0] != _T(':'))
                            sAbsolutePath = sModName + sAbsolutePath;
                        else
                            sAbsolutePath = sAbsolutePath.substr(2);

                        tstring sLowerPath(LowerString(sAbsolutePath));
                        if (m_setDeletedCompatFiles.find(sLowerPath) == m_setDeletedCompatFiles.end())
                        {
                            CompatMap::iterator it(m_mapCompatFiles.find(sLowerPath));
                            if (it != m_mapCompatFiles.end())
                                return sLowerPath + _T("<") + m_pCompatArchive->GetCompleteDiskPath();
                        }
                    }

                    if (sPath[0] != _T('~') && sPath[0] != _T('#') && sPath[0] != _T('@') && sPath[0] != _T(':'))
                    {
                        // Let files in the current world archive override files in the game directory
                        if (m_pWorldArchive != NULL && !(iMode & FILE_NOWORLDARCHIVE)) 
                        {
                            const tstring &sTestPath(LowerString(Filename_GetPath(m_pWorldArchive->GetPath()) + _T("resources") + sPath));
                            if (m_pWorldArchive->ContainsFile(sTestPath))
                                return sTestPath + _T("<") + m_pCompatArchive->GetCompleteDiskPath();
                        }

                        if ((iMode & FILE_ALLOW_CUSTOM) || !fs_disablemods)
                        {
                            if (!(iMode & FILE_NOUSERDIR)) // Let files in the user directory override files in the game directory
                            {
                                const tstring &sTestPath(GetSystemPath(sPath[0] == _T('/') ? _TS("~") + sPath : _TS("~/") + sPath, sModName));
                                if (!sTestPath.empty())
                                    return sTestPath;
                            }
                        }
                    }

                    if ((iMode & FILE_ALLOW_CUSTOM) || !fs_disablemods)
                    {
                        const tstring &sTestPath(GetSystemPath(sPath, sModName, false, false, true));
                        if (!sTestPath.empty())
                            return sTestPath;
                    }
                }
            }

            // not on the disk, maybe it's in an archive
            if (!(iMode & FILE_WRITE) && !(iMode & FILE_NOARCHIVES))
            {
                if (!GetCompatVersion().empty() && !(iMode & FILE_NOCOMPAT))
                {
                    tstring sAbsolutePath(IsCleanPath(sPath) ? sPath : SanitizePath(sPath));

                    if (sAbsolutePath[0] == _T('/'))
                    {
                        sAbsolutePath = sModName + _T("/resources0.s2z") + sAbsolutePath;

                        tstring sLowerPath(LowerString(sAbsolutePath));
                        if (m_setDeletedCompatFiles.find(sLowerPath) == m_setDeletedCompatFiles.end())
                        {
                            CompatMap::iterator it(m_mapCompatFiles.find(sLowerPath));
                            if (it != m_mapCompatFiles.end())
                                return sLowerPath + _T("<") + m_pCompatArchive->GetCompleteDiskPath();
                        }
                    }
                }

                pArchive = FindInArchives(sPath, *it);
                if (pArchive != NULL)
                    return sPath + _T("<") + pArchive->GetCompleteDiskPath();
            }

            if ((iMode & FILE_TOPMODONLY) || !sMod.empty())
                break;
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CFileManager::FindFilePath() [") + sPath + _T("] - "), NO_THROW);
    }
    return TSNULL;
}


/*====================
  CFileManager::GetFileList

  Generates a list of all matching files in a given game path from
  all registered archives and mod directories

  Searching in this order:
  current world archive
  files in top mod directory
  archives in top mod directory
  files in next mod directory
  archives in next mod directory
  ...
  ...
  files in root game directory
  archives in root game directory

  TODO: Do we want this in order of search priority?
  ====================*/
bool    CFileManager::GetFileList(const tstring &sPath, const tstring &sFile, bool bRecurse, tsvector &vFileList, bool bCheckOutsideArchives)
{
    if (!m_sCompatVersion.empty())
        return GetFileListCompat(sPath, sFile, bRecurse, vFileList);

    // Setup wildcard search variables
    tstring sFirstPart(sFile);
    tstring sSecondPart;

    size_t zStar(sFile.find(_T("*")));
    if (zStar != tstring::npos)
    {
        sFirstPart.resize(zStar);
        sSecondPart.assign(sFile, zStar + 1, sFile.length() - (zStar + 1));
    }

    // Resolve the full path that will be searched
    tstring sAbsolutePath(IsCleanPath(sPath) ? sPath : SanitizePath(sPath));
    
    // World archive
    if (m_pWorldArchive != NULL)
    {
        tsvector vArchive;

        m_pWorldArchive->GetFileList(vArchive);

        for (tsvector_it it(vArchive.begin()); it != vArchive.end(); it++)
        {
            // Ignore directory entries
            if ((*it)[it->length() - 1] == _T('/'))
                continue;

            // Ignore files not stored as resources
            if (CompareNoCase(it->substr(0, 10), _T("resources/")) != 0)
                continue;

            tstring sFilename(it->substr(9)); // Strip off "resources"

            // Ensure that paths match
            if (sFilename.compare(0, sAbsolutePath.size(), sAbsolutePath) != 0)
                continue;

            // If the path of the file being checked goes beyond the path to search, only continue if recursion is requested
            if (!bRecurse && sFilename.find(_T("/"), sAbsolutePath.size() + 1) != tstring::npos)
                continue;

            // Look for the first part of the filename in the target
            size_t zPos(sFilename.find(sFirstPart, sAbsolutePath.size()));

            // Search from the end of the first match (if it was found) for the second part
            if (zPos != tstring::npos && sFilename.find(sSecondPart, zPos + sFirstPart.size()) != tstring::npos)
                vFileList.push_back(sFilename);
        }
    }

    // Check each mod
    for (tsvector_rit it(m_vModPathStack.rbegin()); it != m_vModPathStack.rend(); ++it)
    {
        if (!fs_disablemods || bCheckOutsideArchives)
        {
            // Files in current mod directory
            K2System.GetFileList(sPath, sFile, bRecurse, vFileList, *it);
        }

        // Check each registered archive
        for (vector<CArchive*>::iterator itArchive(m_vArchiveList.begin()), itEnd(m_vArchiveList.end()); itArchive != itEnd; ++itArchive)
        {
            // Only process the archive on this pass if it belongs to the current mod
            if (!it->empty() && (*itArchive)->GetMod() != *it)
                continue;

            // Only process the archive if the path to the archive overlaps the path to be searched
            const tstring &sPathToArchive((*itArchive)->GetPathToArchive());
            if (sAbsolutePath.compare(0, sPathToArchive.length(), sPathToArchive) != 0)
                return false;

            // Strip away the overlapping part of the path for searching within the archive
            tstring sPathInArchive(sAbsolutePath.substr(sPathToArchive.size()));

            tsvector vArchiveFileList;
            (*itArchive)->GetFileList(vArchiveFileList);

            // Search each file in the archive
            for (tsvector_it itFileName(vArchiveFileList.begin()), itEnd(vArchiveFileList.end()); itFileName != itEnd; ++itFileName)
            {
                // Ensure that paths match
                if (itFileName->compare(0, sPathInArchive.size(), sPathInArchive) != 0)
                    continue;

                // If the path of the file being checked goes beyond the path to search, only continue if recursion is requested
                if (!bRecurse && itFileName->find(_T("/"), sPathInArchive.size() + 1) != tstring::npos)
                    continue;

                // Look for the first part of the filename in the target
                size_t zPos(itFileName->find(sFirstPart, sPathInArchive.size()));

                // Search from the end of the first match (if it was found) for the second part
                if (zPos != tstring::npos && itFileName->find(sSecondPart, zPos + sFirstPart.size()) != tstring::npos)
                    vFileList.push_back(*itFileName);
            }
        }
    }

    std::sort(vFileList.begin(), vFileList.end());
    tsvector_it itSortedEnd(std::unique(vFileList.begin(), vFileList.end()));
    vFileList.erase(itSortedEnd, vFileList.end());
    return true;
}


/*====================
  CFileManager::GetFileListCompat

  Generates a list of all matching files in a given game path from
  all registered archives and mod directories

  Searching in this order:
  current world archive
  files in top mod directory
  archives in top mod directory
  files in next mod directory
  archives in next mod directory
  ...
  ...
  files in root game directory
  archives in root game directory

  TODO: Do we want this in order of search priority?
  ====================*/
bool    CFileManager::GetFileListCompat(const tstring &sPath, const tstring &sFile, bool bRecurse, tsvector &vFileList)
{
    // Setup wildcard search variables
    tstring sFirstPart(sFile);
    tstring sSecondPart;

    size_t zStar(sFile.find(_T("*")));
    if (zStar != tstring::npos)
    {
        sFirstPart.resize(zStar);
        sSecondPart.assign(sFile, zStar + 1, sFile.length() - (zStar + 1));
    }

    // Resolve the full path that will be searched
    tstring sAbsolutePath(IsCleanPath(sPath) ? sPath : SanitizePath(sPath));
    
    // World archive
    if (m_pWorldArchive != NULL)
    {
        tsvector vArchive;

        m_pWorldArchive->GetFileList(vArchive);

        for (tsvector_it it(vArchive.begin()); it != vArchive.end(); it++)
        {
            // Ignore directory entries
            if ((*it)[it->length() - 1] == _T('/'))
                continue;

            // Ignore files not stored as resources
            if (CompareNoCase(it->substr(0, 10), _T("resources/")) != 0)
                continue;

            tstring sFilename(it->substr(9)); // Strip off "resources"

            // Ensure that paths match
            if (sFilename.compare(0, sAbsolutePath.size(), sAbsolutePath) != 0)
                continue;

            // If the path of the file being checked goes beyond the path to search, only continue if recursion is requested
            if (!bRecurse && sFilename.find(_T("/"), sAbsolutePath.size() + 1) != tstring::npos)
                continue;

            // Look for the first part of the filename in the target
            size_t zPos(sFilename.find(sFirstPart, sAbsolutePath.size()));

            // Search from the end of the first match (if it was found) for the second part
            if (zPos != tstring::npos && sFilename.find(sSecondPart, zPos + sFirstPart.size()) != tstring::npos)
                vFileList.push_back(sFilename);
        }
    }

    // Check each mod
    for (tsvector_rit it(m_vModPathStack.rbegin()); it != m_vModPathStack.rend(); ++it)
    {
        tsvector vNewFileList;

        // Files in current mod directory
        K2System.GetFileList(sPath, sFile, bRecurse, vNewFileList, *it);

        // Exclude deleted files
        for (tsvector_it itNew(vNewFileList.begin()), itNewEnd(vNewFileList.end()); itNew != itNewEnd; ++itNew)
        {
            tstring sCompatPath(*it + *itNew);

            if (m_setDeletedCompatFiles.find(sCompatPath) != m_setDeletedCompatFiles.end())
                continue;

            vFileList.push_back(*itNew);
        }

        // Add new compat files
        tstring sCompatHeader(*it);
        tstring sCompatPath(*it + sPath);
        for (StringSet::iterator itNew(m_setNewCompatFiles.begin()), itNewEnd(m_setNewCompatFiles.end()); itNew != itNewEnd; ++itNew)
        {
            if (itNew->find(_CWS(".s2z/")) != tstring::npos)
                continue;

            size_t zPos(itNew->find(sCompatPath));
            if (zPos != tstring::npos && (bRecurse || itNew->find(_T("/"), zPos + 1) == tstring::npos))
            {
                size_t pos3 = itNew->find(sFirstPart, zPos + 1);
                if (pos3 != tstring::npos && itNew->find(sSecondPart, pos3) != tstring::npos)
                    vFileList.push_back(itNew->substr(sCompatHeader.length()));
            }
        }

        // Check each registered archive
        for (vector<CArchive*>::iterator itArchive(m_vArchiveList.begin()), itEnd(m_vArchiveList.end()); itArchive != itEnd; ++itArchive)
        {
            // Only process the archive on this pass if it belongs to the current mod
            if (!it->empty() && (*itArchive)->GetMod() != *it)
                continue;

            // Only process the archive if the path to the archive overlaps the path to be searched
            const tstring &sPathToArchive((*itArchive)->GetPathToArchive());
            if (sAbsolutePath.compare(0, sPathToArchive.length(), sPathToArchive) != 0)
                return false;

            // Strip away the overlapping part of the path for searching within the archive
            tstring sPathInArchive(sAbsolutePath.substr(sPathToArchive.size()));

            tsvector vArchiveFileList;
            (*itArchive)->GetFileList(vArchiveFileList);
            
            // Search each file in the archive
            for (tsvector_it itFileName(vArchiveFileList.begin()), itEnd(vArchiveFileList.end()); itFileName != itEnd; ++itFileName)
            {
                // Ensure that paths match
                if (itFileName->compare(0, sPathInArchive.size(), sPathInArchive) != 0)
                    continue;

                // If the path of the file being checked goes beyond the path to search, only continue if recursion is requested
                if (!bRecurse && itFileName->find(_T("/"), sPathInArchive.size() + 1) != tstring::npos)
                    continue;

                // Look for the first part of the filename in the target
                size_t zPos(itFileName->find(sFirstPart, sPathInArchive.size()));

                // Search from the end of the first match (if it was found) for the second part
                if (zPos != tstring::npos && itFileName->find(sSecondPart, zPos + sFirstPart.size()) != tstring::npos)
                {
                    // Exclude deleted files
                    tstring sCompatPath(*it + (*itArchive)->GetPath() + *itFileName);
                    if (m_setDeletedCompatFiles.find(sCompatPath) == m_setDeletedCompatFiles.end())
                        vFileList.push_back(*itFileName);;
                }
            }

            // Add new compat files
            tstring sCompatHeader(*it + (*itArchive)->GetPath());
            tstring sCompatPath(*it + (*itArchive)->GetPath() + sPath);
            for (StringSet::iterator itNew(m_setNewCompatFiles.begin()), itNewEnd(m_setNewCompatFiles.end()); itNew != itNewEnd; ++itNew)
            {
                size_t pos2 = itNew->find(sCompatPath.c_str());
                if (pos2 != tstring::npos && (bRecurse || itNew->find(_T("/"), pos2 + 1) == tstring::npos))
                {
                    size_t pos3 = itNew->find(sFirstPart, pos2 + 1);
                    if (pos2 != tstring::npos && itNew->find(sSecondPart, pos3) != tstring::npos)
                        vFileList.push_back(itNew->substr(sCompatHeader.length()));
                }
            }
        }
    }

    std::sort(vFileList.begin(), vFileList.end());
    tsvector_it itSortedEnd(std::unique(vFileList.begin(), vFileList.end()));
    vFileList.erase(itSortedEnd, vFileList.end());
    return true;
}


/*====================
  CFileManager::GetDirList

  Generates a list of all matching directories in a given
  game path from all registered archives and mod directories

  TODO: Do we want this in order of search priority?
  ====================*/
bool    CFileManager::GetDirList(const tstring &sPath, bool bRecurse, tsvector &vDirList)
{
    vDirList.clear();

    for (tsvector::reverse_iterator it(m_vModPathStack.rbegin()); it != m_vModPathStack.rend(); ++it)
    {
        // Directories in current mod directory
        K2System.GetDirList(sPath, bRecurse, vDirList, *it);

        // TODO: Archives in current mod directory
        // UTTAR: Is this really necessary?
    }

    std::sort(vDirList.begin(), vDirList.end());
    tsvector_it itSortedEnd(std::unique(vDirList.begin(), vDirList.end()));
    vDirList.erase(itSortedEnd, vDirList.end());
    return true;
}


/*====================
  CFileManager::GetNextFileIncrement
  ====================*/
tstring CFileManager::GetNextFileIncrement(int zNumDigits, const tstring &sBaseName, const tstring &sExt, int iStart)
{
    tstring sFilename;

    int iLimit = M_Power(10, zNumDigits) - 1;

    int i(iStart);
    do
    {
        sFilename = sBaseName + XtoA(i, FMT_PADZERO, zNumDigits) + _T('.') + sExt;
        ++i;
    }
    while (Exists(sFilename) && i <= iLimit);

    return sFilename;
}


/*====================
  CFileManager::GetFile
  ====================*/
CFile*  CFileManager::GetFile(const tstring &sFilename, int iMode, const tstring &sMod)
{
    PROFILE("CFileManager::GetFile");

    CFile *pFile(NULL);
    tstring sPath(sFilename);

    try
    {
        if (sPath.empty())
            return NULL;

        EFileType eType(FILE_INVALID);

        // Check for a protocol that is supported
        size_t zMarker(sPath.find(_T("://")));
        if (zMarker < sPath.length())
        {
            tstring sProtocol(sPath.substr(0, zMarker));
            if (sProtocol == _T("http"))
                eType = FILE_HTTP;
            else
                EX_ERROR(_T("The protocol ") + QuoteStr(sProtocol) + _T(" is not supported"));
        }

        CArchive *pArchive(NULL);
        for (tsvector::reverse_iterator it(m_vModPathStack.rbegin()); it != m_vModPathStack.rend(); ++it)
        {
            const tstring &sModName(sMod.empty() ? *it : sMod);

            // Search for the file on disk
            if (eType == FILE_INVALID)
            {
                if (iMode & FILE_WRITE)
                {
                    tstring sPathToFile(Filename_GetPath(sPath));
                    MakeDir(sPathToFile);
                    sPath = GetSystemPath(sPathToFile); // figure out which directory we're going to write to
                    if (!sPath.empty())
                    {
                        sPath += Filename_StripPath(sFilename);
                        eType = FILE_DISK;
                    }
                    break;
                }
                else
                {
                    if (!GetCompatVersion().empty() && !(iMode & FILE_NOCOMPAT))
                    {
                        tstring sAbsolutePath(IsCleanPath(sPath) ? sPath : SanitizePath(sPath));

                        bool bDeleted(false);

                        if (sAbsolutePath[0] != _T(':'))
                            sAbsolutePath = sModName + sAbsolutePath;
                        else
                            sAbsolutePath = sAbsolutePath.substr(2);

                        pFile = GetCompatFile(sAbsolutePath, iMode, bDeleted);
                        
                        if (bDeleted)
                        {
                            SAFE_DELETE(pFile);
                        }
                        else if (pFile != NULL)
                        {
                            eType = FILE_COMPAT;
                            break;
                        }
                    }

                    if (sPath[0] != _T('~') && sPath[0] != _T('#') && sPath[0] != _T('@') && sPath[0] != _T(':'))
                    {
                        if (m_pWorldArchive != NULL && !(iMode & FILE_NOWORLDARCHIVE)) // Let files in the current world archive override files in the game directory
                        {
                            const tstring &sTestPath(LowerString(Filename_GetPath(m_pWorldArchive->GetPath()) + _T("resources") + sPath));
                            if (m_pWorldArchive->ContainsFile(sTestPath))
                            {
                                g_ResourceManager.RegisterMapResource(sPath);
                                sPath = LowerString(Filename_GetPath(m_pWorldArchive->GetPath()) + _T("resources") + sPath);
                                pArchive = m_pWorldArchive;
                                eType = FILE_ARCHIVE;
                                break;
                            }
                        }

                        if ((iMode & FILE_ALLOW_CUSTOM) || !fs_disablemods)
                        {
                            if (!(iMode & FILE_NOUSERDIR)) // Let files in the user directory override files in the game directory
                            {
                                const tstring &sTestPath(GetSystemPath(sPath[0] == _T('/') ? _TS("~") + sPath : _TS("~/") + sPath, sModName));
                                if (!sTestPath.empty())
                                {
                                    sPath = sTestPath;
                                    eType = FILE_DISK;
                                    break;
                                }
                            }
                        }
                    }

                    if ((iMode & FILE_ALLOW_CUSTOM) || !fs_disablemods)
                    {
                        const tstring &sTestPath(GetSystemPath(sPath, sModName, false, false, true));
                        if (!sTestPath.empty())
                        {
                            sPath = sTestPath;
                            eType = FILE_DISK;
                            break;
                        }
                    }
                }
            }

            // not on the disk, maybe it's in an archive
            if (eType == FILE_INVALID && !(iMode & FILE_WRITE) && !(iMode & FILE_NOARCHIVES))
            {
                if (!GetCompatVersion().empty() && !(iMode & FILE_NOCOMPAT))
                {
                    tstring sAbsolutePath(IsCleanPath(sPath) ? sPath : SanitizePath(sPath));

                    bool bDeleted(false);

                    if (sAbsolutePath[0] == _T('/'))
                    {
                        sAbsolutePath = sModName + _T("/resources0.s2z") + sAbsolutePath;

                        pFile = GetCompatFile(sAbsolutePath, iMode, bDeleted);
                        
                        if (bDeleted)
                        {
                            SAFE_DELETE(pFile);
                        }
                        else if (pFile != NULL)
                        {
                            eType = FILE_COMPAT;
                            break;
                        }
                    }
                }

                pArchive = FindInArchives(sPath, *it);
                if (pArchive != NULL)
                {
                    eType = FILE_ARCHIVE;
                    break;
                }
            }

            if ((iMode & FILE_TOPMODONLY) || !sMod.empty())
                break;
        }

        if (eType == FILE_INVALID && iMode & FILE_TEST)
            return NULL;

        if (eType == FILE_COMPAT)
            return pFile;

        // Allocate an appropirate file class for the handle
        switch (eType)
        {
            case FILE_DISK:
                pFile = K2_NEW(ctx_FileSystem,  CFileDisk);
                break;

            case FILE_ARCHIVE:
                pFile = K2_NEW(ctx_FileSystem,  CFileArchive)(pArchive);
                break;

            case FILE_HTTP:
                pFile = K2_NEW(ctx_FileSystem,  CFileHTTP);
                break;

            default:
            case FILE_INVALID:
                EX_ERROR(_T("Invalid file type"));
                break;
        }
        if (pFile == NULL)
            EX_ERROR(_T("Failed to allocate CFile"));

        // Try to open the file
        if (!pFile->Open(sPath, iMode))
            EX_ERROR(_T("Could not open file"));

        if (eType == FILE_DISK && (iMode & FILE_READ) && !(iMode & FILE_ALLOW_CUSTOM))
        {
            m_bUsingCustomFiles = true;
            if (fs_trackCustomFiles)
                m_setCustomFiles.insert(sPath);
        }

        return pFile;
    }
    catch (CException &ex)
    {
        SAFE_DELETE(pFile);
        ex.Process(_T("CFileManager::GetFile() [") + sPath + _T("] - "), NO_THROW);
        return NULL;
    }
}


/*====================
  CFileManager::BuildBlockList
  ====================*/
bool    CFileManager::BuildBlockList(const char *pBuffer, size_t iBufLen, vector<block_t> &vBlockList)
{
    size_t  pos = 0;

    vBlockList.clear();
    while (pos + 8 < iBufLen)
    {
        block_t block;
        MemManager.Set(&block, 0, sizeof(block_t));

        block.name[0] = pBuffer[pos++];
        block.name[1] = pBuffer[pos++];
        block.name[2] = pBuffer[pos++];
        block.name[3] = pBuffer[pos++];
        block.name[4] = '\0';

        block.length = LittleInt(*(int*)(&pBuffer[pos]));
        if (pos + block.length > iBufLen)
        {
            K2System.Error(_T("BuildBlockList: Truncated block"));
            return false;
        }

        pos += 4;
        block.pos = pos;
        block.data = (byte*)&pBuffer[pos];
        pos += block.length;

        vBlockList.push_back(block);
    }

    if (pos != iBufLen)
    {
        Console.Dev << _T("BuildBlockList: bad filesize") << newl;
        return false;
    }

    return true;
}


/*====================
  CFileManager::GetGamePath

  Translates a system path to a game path
  ====================*/
tstring CFileManager::GetGamePath(const tstring &sSystemPath)
{
    tstring sClean(SanitizePath(sSystemPath, false));
    const tstring &sRoot(K2System.GetRootDir());
    const tstring &sUser(K2System.GetUserDir());

    if (CompareNum(sClean, sRoot, sRoot.length()) == 0)
    {
        size_t zPos(sClean.find_first_of(_T("/"), sRoot.length()));
        return sClean.substr(zPos);
    }
    else if (CompareNum(sClean, sUser, sUser.length()) == 0)
    {
        size_t zPos(sClean.find_first_of(_T("/"), sUser.length()));
        return _T("~") + sClean.substr(zPos);
    }

    return Filename_StripPath(sSystemPath);
}


/*====================
  CFileManager::InitCRC32
  ====================*/
void    CFileManager::InitCRC32()
{
#ifdef _WIN32
    // This is the official polynomial defined in IEEE 802.3.
    // The polynomial is occasionally reversed to 0x04C11DB7.
    uint uiPolynomial = 0xEDB88320;
    int i, j;

    SAFE_DELETE_ARRAY(m_uiCRC32);
    m_uiCRC32 = K2_NEW_ARRAY(ctx_FileSystem, uint, 256);

    MemManager.Set(m_uiCRC32, 0, sizeof(m_uiCRC32));

    uint uiCRC;
    for(i = 0; i < 256; i++)
    {
        uiCRC = i;
        for(j = 8; j > 0; j--)
        {
            if(uiCRC & 1)
                uiCRC = (uiCRC >> 1) ^ uiPolynomial;
            else
                uiCRC >>= 1;
        }

        m_uiCRC32[i] = uiCRC;
    }
#endif
}


/*====================
  CFileManager::GetCRC32ForArchive
  ====================*/
uint    CFileManager::GetCRC32ForArchive(const tstring &sFile)
{
    uint uiFinalCRC(0);

    CArchive archive(sFile);

    if (!archive.IsOpen())
        return uint(-1);

    tsvector vsFileList;
    archive.GetFileList(vsFileList);

    for (tsvector_it it(vsFileList.begin()); it != vsFileList.end(); it++)
    {
        char *pBuffer(NULL);
        uint uiSize;

        uiSize = archive.ReadFile(Filename_GetPath(sFile) + (*it).substr(1), pBuffer);

        if (pBuffer == NULL)
            continue;

        uiFinalCRC += GetCRC32(pBuffer, uiSize);

        SAFE_DELETE_ARRAY(pBuffer);
    }

    archive.Close();

    return uiFinalCRC;
}

#ifdef _WIN32
/*====================
  CFileManager::GetCRC32
  ====================*/
uint    CFileManager::GetCRC32(const tstring &sFile)
{
    CFile *pFile = GetFile(sFile, FILE_NOUSERDIR | FILE_NOARCHIVES | FILE_NOWORLDARCHIVE | FILE_TOPMODONLY | FILE_READ | FILE_BINARY);

    if (pFile == NULL)
        return 0;
    
    uint uiSize;
    const char *pBuf = pFile->GetBuffer(uiSize);
    
    uint uiCRC(0);
    if (pBuf && uiSize > 0)
        uiCRC = ~crc32(0, (const Bytef*)pBuf, uiSize);
    
    pFile->Close();
    SAFE_DELETE(pFile);

    return uiCRC;
}

uint    CFileManager::GetCRC32(const char *pBuf, size_t zLen)
{
    return ~crc32(0, (const Bytef*)pBuf, uInt(zLen));
}
#else
/*====================
  CFileManager::GetCRC32
  ====================*/
uint    CFileManager::GetCRC32(const tstring &sFile)
{
    CFile *pFile = GetFile(sFile, FILE_NOUSERDIR | FILE_NOARCHIVES | FILE_NOWORLDARCHIVE | FILE_TOPMODONLY | FILE_READ | FILE_BINARY);

    if (pFile == NULL)
        return 0;
    
    uint uiSize;
    const char *pBuf = pFile->GetBuffer(uiSize);
    
    uint uiCRC(0);
    if (pBuf && uiSize > 0)
        uiCRC = crc32(0, (const Bytef*)pBuf, uiSize);
    
    pFile->Close();
    SAFE_DELETE(pFile);

    return uiCRC;
}

uint    CFileManager::GetCRC32(const char *pBuf, size_t zLen)
{
    return crc32(0, (const Bytef*)pBuf, zLen);
}
#endif


/*====================
  CFileManager::IsCompatVersionSupported
  ====================*/
bool    CFileManager::IsCompatVersionSupported(const tstring &sVersion)
{
    if (K2_Version3(sVersion) == K2_Version3(K2System.GetVersionString()))
        return true;

    if (m_bCompatDisabled)
        return false;
    if (m_pCompatArchive == NULL)
        OpenCompatArchive();
    if (m_pCompatArchive == NULL)
        return false;

    tstring sShortVersion(K2_Version3(sVersion));

    return m_pCompatArchive->ContainsFile(m_pCompatArchive->GetPathToArchive() + _T("/") + sShortVersion + _T("/manifest.xml"));
}


/*====================
  CFileManager::OpenCompatArchive
  ====================*/
bool    CFileManager::OpenCompatArchive()
{
    if (m_bCompatDisabled)
        return false;

    if (m_pCompatArchive == NULL)
        m_pCompatArchive = K2_NEW(ctx_FileSystem,  CArchive)(_T(":/compat/compat.s2z"), ARCHIVE_READ);
    else if (!m_pCompatArchive->IsOpen())
        m_pCompatArchive->Open(_T(":/compat/compat.s2z"), ARCHIVE_READ);

    CFileHandle hVersion(_T("version"), FILE_READ | FILE_TEXT | FILE_ASCII, *m_pCompatArchive);
    if (!hVersion.IsOpen())
        return false;

    tstring sVersion;

    byte y(hVersion.ReadByte());
    while (y && y != _T('\n') && y != _T('\r'))
    {
        sVersion += TCHAR(y);
        y = hVersion.ReadByte();
    }

    if (sVersion != K2System.GetVersionString())
        return false;

    return m_pCompatArchive->IsOpen();
}


/*====================
  CFileManager::CloseCompatArchive
  ====================*/
bool    CFileManager::CloseCompatArchive()
{
    if (m_bCompatDisabled)
        return false;

    if (m_pCompatArchive != NULL)
    {
        m_pCompatArchive->Close();

        K2_DELETE(m_pCompatArchive);
        m_pCompatArchive = NULL;
    }

    return m_pCompatArchive == NULL || !m_pCompatArchive->IsOpen();
}


/*====================
  CFileManager::DisableCompatArchive
  ====================*/
bool    CFileManager::DisableCompatArchive()
{
    CloseCompatArchive();

    m_bCompatDisabled = true;

    return true;
}


/*====================
  CFileManager::EnableCompatArchive
  ====================*/
bool    CFileManager::EnableCompatArchive()
{
    m_bCompatDisabled = false;

    return true;
}


/*====================
  CFileManager::GetGamePathFromCompatPath
  ====================*/
tstring CFileManager::GetGamePathFromCompatPath(const tstring &sCompatPath)
{
    tstring sGamePath;
    size_t zFindPos(sCompatPath.find(_T(".s2z/")));

    if (zFindPos != tstring::npos)
    {
        if (sCompatPath.find(_T("textures.s2z/")) != tstring::npos)
        {
            // Texture archive
            if (sCompatPath.substr(zFindPos + 5) == _T("descriptor"))
                sGamePath = _T("descriptor");
            else
                sGamePath = sCompatPath.substr(zFindPos + 13);
        }
        else
        {
            // We're working with an archive file
            sGamePath = sCompatPath.substr(zFindPos + 4);
        }
    }
    else
    {
        size_t zPos(sCompatPath.find_first_of(_T('/')));
        sGamePath = zPos != tstring::npos ? sCompatPath.substr(zPos) : TSNULL;
    }

    return sGamePath;
}


/*====================
  CFileManager::SetCompatVersion
  ====================*/
bool    CFileManager::SetCompatVersion(const tstring &sVersion)
{
    if (!OpenCompatArchive())
        return false;

    CompatMap mapOldCompatFiles;

    tstring sNewCompatVersion(sVersion);
    tsvector vsVersion;

    // Add forth component if missing
    if (!sNewCompatVersion.empty())
    {
        vsVersion = TokenizeString(sNewCompatVersion, _T('.'));
        
        while (vsVersion.size() < 4)
            vsVersion.push_back(_T("0"));

        vsVersion[3] = _T("0"); // Force fourth component to 0

        sNewCompatVersion = ConcatinateArgs(vsVersion, _T("."));
    }

    if (K2_Version3(sNewCompatVersion) == K2_Version3(K2System.GetVersionString()))
        sNewCompatVersion.clear();

    if (sNewCompatVersion.empty())
    {
        if (m_sCompatVersion.empty())
            return true;

        m_sCompatVersion.clear();
        m_sCompatVersionShort.clear();
        m_uiCompatVersion = 0;

        // Save old compat files to calculate which files need reloading
        mapOldCompatFiles = m_mapCompatFiles;

        m_mapCompatFiles.clear();
        m_setNewCompatFiles.clear();
        m_setDeletedCompatFiles.clear();

        m_cBaseManifest = SFileManifest();
        m_cCompatManifest = SFileManifest();
    }
    else
    {
        if (sNewCompatVersion == m_sCompatVersion)
            return true;

        m_sCompatVersion = sNewCompatVersion;
        
        m_sCompatVersionShort = K2_Version(m_sCompatVersion);
        
        m_uiCompatVersion = (AtoI(vsVersion[0]) << 24) + (AtoI(vsVersion[1]) << 16) + (AtoI(vsVersion[2]) << 8) + AtoI(vsVersion[3]);

        bool bParsedBase(XMLManager.Process(_T(":/manifest.xml"), _T("manifest"), &m_cBaseManifest));

        if (!bParsedBase)
        {
            m_sCompatVersion.clear();
            m_sCompatVersionShort.clear();
            m_uiCompatVersion = 0;

            return false;
        }

        CFileHandle hCompatManifest(m_sCompatVersionShort + _T("/") + _T("manifest.xml"), FILE_READ | FILE_BINARY | FILE_NOCOMPAT, *m_pCompatArchive);

        bool bParsedCompat(XMLManager.Process(hCompatManifest, _T("manifest"), &m_cCompatManifest));

        if (!bParsedCompat)
        {
            m_sCompatVersion.clear();
            m_sCompatVersionShort.clear();
            m_uiCompatVersion = 0;

            m_cBaseManifest.mapManifestFiles.clear();

            return false;
        }

        // Save old compat files to calculate which files need reloading
        mapOldCompatFiles = m_mapCompatFiles;

        m_mapCompatFiles.clear();
        m_setNewCompatFiles.clear();
        m_setDeletedCompatFiles.clear();

        sset setIgnore;

        CFileHandle hIgnore(_T("/compat_resource_ignore.txt"), FILE_READ | FILE_TEXT | FILE_NOCOMPAT);
        if (hIgnore.IsOpen())
        {
            tstring sFilename;

            while (!hIgnore.IsEOF())
            {
                sFilename = hIgnore.ReadLine();

                StripNewline(sFilename);
                sFilename = LowerString(sFilename);

                if (!sFilename.empty() && setIgnore.find(sFilename) == setIgnore.end())
                    setIgnore.insert(sFilename);
            }
        }

        for (ManifestEntryMap::iterator it(m_cCompatManifest.mapManifestFiles.begin()); it != m_cCompatManifest.mapManifestFiles.end(); ++it)
        {
            tstring sVersion(CUpdater::VtoA2(it->second.uiVersion));

            if (!m_pCompatArchive->ContainsFile(m_pCompatArchive->GetPathToArchive() + _T("/") + sVersion + _T("/") + it->first))
                continue;

            ManifestEntryMap::iterator itOld(m_cBaseManifest.mapManifestFiles.find(it->first));

            if (itOld != m_cBaseManifest.mapManifestFiles.end() &&
                it->second.uiSize == itOld->second.uiSize &&
                it->second.uiChecksum == itOld->second.uiChecksum &&
                it->second.uiVersion == itOld->second.uiVersion)
                continue;

            tstring sGamePath(GetGamePathFromCompatPath(it->first));

            if (sGamePath.empty())
                continue;

            tstring sLower(LowerString(sGamePath));

            bool bIgnored(false);
            for (sset::iterator itIgnore(setIgnore.begin()); itIgnore != setIgnore.end(); ++itIgnore)
            {
                if (EqualsWildcard(*itIgnore, sLower))
                {
                    bIgnored = true;
                    break;
                }
            }

            if (bIgnored)
                continue;

            if (itOld == m_cBaseManifest.mapManifestFiles.end())
                m_setNewCompatFiles.insert(it->first);

            m_mapCompatFiles[it->first] = m_pCompatArchive->GetPathToArchive() + _T("/") + sVersion + _T("/") + it->first;
        }

        for (ManifestEntryMap::iterator it(m_cBaseManifest.mapManifestFiles.begin()); it != m_cBaseManifest.mapManifestFiles.end(); ++it)
        {
            ManifestEntryMap::iterator itOld(m_cCompatManifest.mapManifestFiles.find(it->first));

            if (itOld != m_cCompatManifest.mapManifestFiles.end())
                continue;

            tstring sGamePath(GetGamePathFromCompatPath(it->first));

            if (sGamePath.empty())
                continue;

            tstring sLower(LowerString(sGamePath));

            bool bIgnored(false);
            for (sset::iterator itIgnore(setIgnore.begin()); itIgnore != setIgnore.end(); ++itIgnore)
            {
                if (EqualsWildcard(*itIgnore, sLower))
                {
                    bIgnored = true;
                    break;
                }
            }

            if (bIgnored)
                continue;

            m_setDeletedCompatFiles.insert(it->first);

            m_mapCompatFiles[it->first] = TSNULL;
        }
    }

    vector<tstring> vReloadList;
    vector<tstring> vFreeList;

    CompatMap::iterator itOld(mapOldCompatFiles.begin()), itOldEnd(mapOldCompatFiles.end());
    CompatMap::iterator itNew(m_mapCompatFiles.begin()), itNewEnd(m_mapCompatFiles.end());
    
    while (itOld != itOldEnd || itNew != itNewEnd)
    {
        if (itOld == itOldEnd || (itNew != itNewEnd && itNew->first < itOld->first))
        {
            // No items remaining in base, so this must be new
            // Item wasn't overridden and now it is (Reload)
            tstring sGamePath(GetGamePathFromCompatPath(itNew->first));
            if (itNew->second.empty())
                vFreeList.push_back(sGamePath);
            else
                vReloadList.push_back(sGamePath);

            ++itNew;
        }
        else if (itNew == itNewEnd || itNew->first > itOld->first)
        {
            // This item was removed
            // Item was overridden and now it's not (Reload)
            tstring sGamePath(GetGamePathFromCompatPath(itOld->first));
            vReloadList.push_back(sGamePath);

            ++itOld;
        }
        else
        {
            // Overridden in both version, check for changes
            if (itOld->second != itNew->second)
            {
                // Reload
                tstring sGamePath(GetGamePathFromCompatPath(itNew->first));
                if (itNew->second.empty())
                    vFreeList.push_back(sGamePath);
                else
                    vReloadList.push_back(sGamePath);
            }

            ++itOld;
            ++itNew;
        }
    }

    // Execute free list
    for (vector<tstring>::iterator it(vFreeList.begin()), itEnd(vFreeList.end()); it != itEnd; ++it)
    {
        ResHandle hRes(g_ResourceManager.LookUpPath(*it));
        if (hRes == INVALID_RESOURCE)
            continue;

        g_ResourceManager.Unregister(hRes, UNREG_RESERVE_HANDLE);
    }

    // Execute reload list
    for (vector<tstring>::iterator it(vReloadList.begin()), itEnd(vReloadList.end()); it != itEnd; ++it)
        g_ResourceManager.Reload(*it);

    return true;
}


/*====================
  CFileManager::GetCompatFile
  ====================*/
CFile*  CFileManager::GetCompatFile(const tstring &sPath, int iMode, bool &bDeleted)
{
    if (sPath.empty())
        return NULL;

    tstring sLowerPath(LowerString(sPath));

    if (m_setDeletedCompatFiles.find(sLowerPath) != m_setDeletedCompatFiles.end())
    {
        bDeleted = true;
        return NULL;
    }

    bDeleted = false;

    CompatMap::iterator it(m_mapCompatFiles.find(sLowerPath));
    if (it == m_mapCompatFiles.end())
        return NULL;

    CFile *pFile(K2_NEW(ctx_FileSystem,  CFileArchive)(m_pCompatArchive));
    if (!pFile->Open(it->second, iMode))
    {
        pFile->Close();
        return NULL;
    }

    return pFile;
}


/*--------------------
  SystemPath
  --------------------*/
FUNCTION(SystemPath)
{
    return vArgList.size() > 0 ? CFileManager::GetInstance()->GetGamePath(vArgList[0]) : _T("");
}


/*--------------------
  cmdForEachFile
  --------------------*/
CMD(ForEachFile)
{
    if (vArgList.size() < 5)
    {
        Console.Err << _T("Syntax: ForEachFile <mod> <path> <search string> <recurse> <command>") << newl;
        Console.Err << _T("     NOTE: Replacing <mod> with * means all mods,")  << newl;
        Console.Err << _T("     while ! indicates the current mod only.")   << newl;
        Console.Err << _T("     <recurse> should be indicated as true or false.")   << newl;
        return false;
    }

    tsvector vsFileList;
    bool bRecurse = AtoB(vArgList[3]);

    if (vArgList[0] == _T("*"))
        pFileManager->GetFileList(vArgList[1], vArgList[2], bRecurse, vsFileList);
    else if (vArgList[0] == _T("!"))
        K2System.GetFileList(vArgList[1], vArgList[2], bRecurse, vsFileList, pFileManager->GetTopModPath());
    else
        K2System.GetFileList(vArgList[1], vArgList[2], bRecurse, vsFileList, vArgList[0]);

    for (tsvector::iterator it(vsFileList.begin()); it != vsFileList.end(); it++)
    {
        ICvar::CreateString(_T("file_filename"), Filename_GetName(*it));
        ICvar::CreateString(_T("file_fileext"), Filename_GetExtension(*it));
        ICvar::CreateString(_T("file_file"), Filename_StripPath(*it));
        ICvar::CreateString(_T("file_path"), Filename_GetPath(*it));
        ICvar::CreateString(_T("file_filepath"), *it);

        Console.Execute(ConcatinateArgs(vArgList.begin() + 4, vArgList.end()));
    }

    return true;
}


/*--------------------
  cmdForEachFile
  --------------------*/
UI_VOID_CMD(ForEachFile, 5)
{
    if (pThis == NULL)
        return;

    if (vArgList.size() < 5)
    {
        Console.Err << _T("Syntax: ForEachFile <mod> <path> <search string> <recurse> <command>") << newl;
        Console.Err << _T("     NOTE: Replacing <mod> with * means all mods,")  << newl;
        Console.Err << _T("     while ! indicates the current mod only.")   << newl;
        Console.Err << _T("     <recurse> should be indicated as true or false.")   << newl;
        return;
    }

    tsvector vsFileList;
    bool bRecurse = AtoB(vArgList[3]->Evaluate());

    if (vArgList[0]->Evaluate() == _T("*"))
        pFileManager->GetFileList(vArgList[1]->Evaluate(), vArgList[2]->Evaluate(), bRecurse, vsFileList);
    else if (vArgList[0]->Evaluate() == _T("!"))
        K2System.GetFileList(vArgList[1]->Evaluate(), vArgList[2]->Evaluate(), bRecurse, vsFileList, pFileManager->GetTopModPath());
    else
        K2System.GetFileList(vArgList[1]->Evaluate(), vArgList[2]->Evaluate(), bRecurse, vsFileList, vArgList[0]->Evaluate());

    for (tsvector::iterator it(vsFileList.begin()); it != vsFileList.end(); it++)
    {
        ICvar::CreateString(_T("file_filename"), Filename_GetName(*it));
        ICvar::CreateString(_T("file_fileext"), Filename_GetExtension(*it));
        ICvar::CreateString(_T("file_file"), Filename_StripPath(*it));
        ICvar::CreateString(_T("file_path"), Filename_GetPath(*it));
        ICvar::CreateString(_T("file_filepath"), *it);

        pThis->Execute(vArgList[4]->Evaluate());
    }
}


/*--------------------
  cmdForEachDir
  --------------------*/
CMD(ForEachDir)
{
    if (vArgList.size() < 4)
    {
        Console.Err << _T("Syntax: ForEachDir <mod> <path> <recurse> <command>") << newl;
        Console.Err << _T("     NOTE: Replacing <mod> with * means all mods,")  << newl;
        Console.Err << _T("     while ! indicates the current mod only.")   << newl;
        Console.Err << _T("     <recurse> should be indicated as true or false.")   << newl;
        return false;
    }

    tsvector vsFileList;
    bool bRecurse = AtoB(vArgList[2]);

    if (vArgList[0] == _T("*"))
        pFileManager->GetDirList(vArgList[1], bRecurse, vsFileList);
    else if (vArgList[0] == _T("!"))
        K2System.GetDirList(vArgList[1], bRecurse, vsFileList, pFileManager->GetTopModPath());
    else
        K2System.GetDirList(vArgList[1], bRecurse, vsFileList, vArgList[0]);

    for (tsvector::iterator it(vsFileList.begin()); it != vsFileList.end(); it++)
    {
        ICvar::CreateString(_T("dir_dirname"), Filename_GetName(*it));
        ICvar::CreateString(_T("dir_path"), Filename_StripPath(*it));
        ICvar::CreateString(_T("dir_dirpath"), *it);

        Console.Execute(ConcatinateArgs(vArgList.begin() + 3, vArgList.end()));
    }

    return true;
}


/*--------------------
  cmdCompatVersion
  --------------------*/
CMD(CompatVersion)
{
    if (vArgList.size() < 1)
        return false;

    pFileManager->SetCompatVersion(vArgList[0]);

    return true;
}


/*--------------------
  cmdListModdedFiles
  --------------------*/
CMD(ListModdedFiles)
{
    Console.Client << _T("Checking modded files...") << newl;

    if (!pFileManager->GetUsingCustomFiles())
    {
        Console.Client << _T("You are not using any mods.") << newl;
        return true;
    }

    if (!fs_trackCustomFiles)
    {
        Console.Client << _T("You are using mods, but the list could not be retrieved ") <<
            _T("because fs_trackCustomFiles is not enabled.  Enable it then restart HoN.") << newl;
        return true;
    }

    const StringSet& setCustomArchives(pFileManager->GetCustomArchivesList());
    if (!setCustomArchives.empty())
    {
        Console.Client << _T("There are ") << (unsigned int)setCustomArchives.size() << _T(" custom archive files: ") << newl;
        for (StringSet_cit it = setCustomArchives.begin(); it != setCustomArchives.end(); ++it)
        {
            const tstring &sArchive(*it);
            Console.Client << sArchive << newl;
        }
        Console.Client << newl;
    }

    const StringSet& setCustomFiles(pFileManager->GetCustomFilesList());
    if (!setCustomFiles.empty())
    {
        Console.Client << _T("There are ") << (unsigned int)setCustomFiles.size() << _T(" custom files: ") << newl;
        for (StringSet_cit it = setCustomFiles.begin(); it != setCustomFiles.end(); ++it)
        {
            const tstring &sArchive(*it);
            Console.Client << sArchive << newl;
        }
    }

    return true;
}


/*--------------------
  cmdListCoreFilesModified
  --------------------*/
CMD(ListCoreFilesModified)
{
    if (!pFileManager->GetCoreFilesModified())
    {
        Console << _T("No core files have been modified.") << newl;
        return true;
    }

    tstring sCoreFilePath(pFileManager->GetModifiedCoreFilePath());
    if (sCoreFilePath.empty())
    {
        Console << _T("PROGRAMMER ERROR: Core files have been modified, but the engine does not remember which file!") << newl;
        return true;
    }

    Console << _T("The following core file has been modified: ") << newl;
    Console << _T("     ") << sCoreFilePath << newl;
    return true;
}


/*--------------------
  cmdValidateResources0
  --------------------*/
CMD(ValidateResources0)
{
    CArchive *pBaseResources0(pFileManager->GetArchive(_T("/resources0.s2z"), _T("base")));
    CArchive *pGameResources0(pFileManager->GetArchive(_T("/resources0.s2z"), _T("game")));

    if (pBaseResources0 == NULL)
    {
        Console << _T("Could not find /base/resources0.s2z!") << newl;
        return true;
    }

    if (pGameResources0 == NULL)
    {
        Console << _T("Could not find /game/resources0.s2z!") << newl;
        return true;
    }

    if (!pBaseResources0->HasChecksums())
    {
        Console << _T("/base/resources0.s2z has no checksums!") << newl;
        return true;
    }

    if (!pGameResources0->HasChecksums())
    {
        Console << _T("/game/resources0.s2z has no checksums!") << newl;
        return true;
    }

    CArchive *pArchives[] = {
        pBaseResources0,
        pGameResources0
    };

    const wchar_t* pArchiveNames[] = {
        _T("base"),
        _T("game")
    };

    for (uint i = 0; i < 2; ++i)
    {
        CArchive* pArchive(pArchives[i]);
        const wchar_t* pArchiveName(pArchiveNames[i]);

        tsvector vModifiedFiles;
        pArchive->GetModifiedFilesList(vModifiedFiles);
        if (vModifiedFiles.empty())
        {
            Console << _T("/") << pArchiveName << _T("/resources0.s2z is clean.") << newl;
            continue;
        }

        Console << _T("The following core files have been modified in /") << pArchiveName << _T("/resources0.s2z") << newl;
        for (tsvector_cit it(vModifiedFiles.begin()), itEnd(vModifiedFiles.end()); it != itEnd; ++it)
        {
            const tstring &sFilePath(*it);
            Console << _T("/") << pArchiveName << _T("/resources0.s2z") << sFilePath << newl;
        }
    }

    return true;
}

