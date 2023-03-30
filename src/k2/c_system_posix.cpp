// (C)2008 S2 Games
// system_osx.cpp
//
//=============================================================================

//=============================================================================
// Headers  
//=============================================================================
#include "k2_common.h"

#include "c_system.h"

#include <dlfcn.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <glob.h>
#include <string.h>
//=============================================================================


//=============================================================================
// Definitions
//=============================================================================
#ifndef __USE_GNU
#define GLOB_ONLYDIR 0
#endif
//=============================================================================



/*====================
 CSystem::Sleep
 ====================*/
void    CSystem::Sleep(uint uiMsecs)
{
    usleep(1000 * uiMsecs);
}


/*====================
  CSystem::DebugBreak
  ====================*/
void    CSystem::DebugBreak()
{
#ifdef __ppc__
    // TODO
#else
    asm ("int $0x03");
#endif
}


/*====================
 CSystem::MakeDir
 ====================*/
bool    CSystem::MakeDir(const tstring &sPath)
{
    bool bRet = true;
    
    size_t pos = sPath.find_first_of(_T("/"));
    
    for (;;)
    {
        pos = sPath.find_first_of(_T("/"), pos + 1);
        
        if (pos == tstring::npos)
            break;
        
        tstring sDir = sPath.substr(0, pos + 1);
        
        if (mkdir(TStringToNative(sDir).c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == 0) //permissions of 755 (rwxr-xr-x)
        {
            bRet = true;
            continue;
        }
        
        if (errno != EEXIST)
        {
            tstring sError = GetErrorString(errno);
            Console.Warn << _T("Failed to create directory ") << QuoteStr(sDir) << _T(", ") << sError << newl;
            bRet = false;
        }
    }
    
    return bRet;
}


inline static bool IsDir(const char* path)
{
    struct stat s;
    
    stat(path, &s);
    
    return S_ISDIR(s.st_mode);
}


/*====================
 CSystem::GetHiddenFileList
 ====================*/
void    CSystem::GetHiddenFileList(const tstring &sPath, const tstring &sFile, bool bRecurse, tsvector &vFileList, const tstring &sMod)
{
    glob_t globbuf;
    char *pName;
    
    string  sFullPath = TStringToNative(FileManager.GetSystemPath(sPath + _T("/"), sMod));
    
    if (sFullPath.empty())
        return;
    
    string sSearch(sFullPath + TStringToNative(sFile));
    
    glob(sSearch.c_str(), 0, NULL, &globbuf);
    
    for (uint i(0); i < globbuf.gl_pathc; i++)
    {
        if ((pName = strrchr(globbuf.gl_pathv[i], '/')))
            pName++;
        else
            pName = globbuf.gl_pathv[i];
        if (!IsDir(globbuf.gl_pathv[i]) &&
            pName[0] == '.')
        {
            vFileList.push_back(FileManager.SanitizePath(sPath + _T("/") + NativeToTString(pName)));
        }
    }
    
    globfree(&globbuf);
    
    if (!bRecurse)
        return;
    
    sSearch = sFullPath + "*";
    
    glob(sSearch.c_str(), GLOB_ONLYDIR, NULL, &globbuf);
    
    for (uint i(0); i < globbuf.gl_pathc; i++)
    {
        if ((pName = strrchr(globbuf.gl_pathv[i], '/')))
            pName++;
        else
            pName = globbuf.gl_pathv[i];
        if (strcmp(pName, ".") &&
            strcmp(pName, "..") &&
            strcmp(pName, "CVS") &&
            IsDir(globbuf.gl_pathv[i]))
        {
            tstring sDirectory(sPath + _T("/") + NativeToTString(pName));
            
            GetFileList(sDirectory, sFile, bRecurse, vFileList, sMod);
        }
    }
    
    globfree(&globbuf);
}


/*====================
 CSystem::GetFileList
 ====================*/
void    CSystem::GetFileList(const tstring &sPath, const tstring &sFile, bool bRecurse, tsvector &vFileList, const tstring &sMod)
{
    glob_t globbuf;
    char *pName;
    
    string  sFullPath = TStringToNative(FileManager.GetSystemPath(sPath + _T("/"), sMod));
    
    if (sFullPath.empty())
        return;
    
    string sSearch(sFullPath + TStringToNative(sFile));
    
    glob(sSearch.c_str(), 0, NULL, &globbuf);
    
    for (uint i(0); i < globbuf.gl_pathc; i++)
    {
        if ((pName = strrchr(globbuf.gl_pathv[i], '/')))
            pName++;
        else
            pName = globbuf.gl_pathv[i];
        if (!IsDir(globbuf.gl_pathv[i]) &&
            pName[0] != '.')
        {
            vFileList.push_back(FileManager.SanitizePath(sPath + _T("/") + NativeToTString(pName)));
        }
    }
    
    globfree(&globbuf);
    
    if (!bRecurse)
        return;
    
    sSearch = sFullPath + "*";
    
    glob(sSearch.c_str(), GLOB_ONLYDIR, NULL, &globbuf);
    
    for (uint i(0); i < globbuf.gl_pathc; i++)
    {
        if ((pName = strrchr(globbuf.gl_pathv[i], '/')))
            pName++;
        else
            pName = globbuf.gl_pathv[i];
        if (pName[0] != '.' &&
            strcmp(pName, "CVS")
            && IsDir(globbuf.gl_pathv[i]))
        {
            tstring sDirectory(sPath + _T("/") + NativeToTString(pName));
            
            GetFileList(sDirectory, sFile, bRecurse, vFileList, sMod);
        }
    }
    
    globfree(&globbuf);
}


/*====================
 CSystem::GetDirList
 ====================*/
void    CSystem::GetDirList(const tstring &sPath, bool bRecurse, tsvector &vDirList, const tstring &sMod)
{
    glob_t globbuf;
    char *pName;
    
    tstring sFullPath = FileManager.GetSystemPath(sPath + _T("/"), sMod);
    
    if (sFullPath.empty())
        return;
    
    string sSearch(TStringToNative(sFullPath) + "*");
    
    glob(sSearch.c_str(), GLOB_ONLYDIR, NULL, &globbuf);
    
    for (uint i(0); i < globbuf.gl_pathc; i++)
    {
        if ((pName = strrchr(globbuf.gl_pathv[i], '/')))
            pName++;
        else
            pName = globbuf.gl_pathv[i];
        if (pName[0] != '.' &&
            strcmp(pName, "CVS") &&
            IsDir(globbuf.gl_pathv[i]))
        {
            vDirList.push_back(FileManager.SanitizePath(sPath + _T("/") + NativeToTString(pName)));
            
            if (bRecurse)
            {
                tstring sDirectory(sPath + _T("/") + NativeToTString(pName));
                GetDirList(sDirectory, bRecurse, vDirList, sMod);
            }
        }
    }
    
    globfree(&globbuf);
}


/*====================
 CSystem::LoadLibrary
 ====================*/
void*   CSystem::LoadLibrary(const tstring &sLibFilename)
{
    PROFILE("CSystem::LoadLibrary");
    
#ifdef __APPLE__
    tstring sFullPath(FileManager.GetSystemPath(sLibFilename + _T(".dylib")));
#else
#ifdef __x86_64__
    tstring sFullPath(FileManager.GetSystemPath(sLibFilename + _T("-x86_64.so")));
#else
    tstring sFullPath(FileManager.GetSystemPath(sLibFilename + _T("-x86.so")));
#endif
#endif
    
    if (sFullPath == TSNULL)
        return NULL;
    else
    {
        void* pLib = dlopen(TStringToNative(sFullPath).c_str(), RTLD_NOW);
        if (!pLib)
        {
            string sError(dlerror());
            Console.Err << _T("CSystem::LoadLibrary() - ") << NativeToTString(sError) << newl;
        }
        return pLib;
    }
}


/*====================
 CSystem::GetProcAddress
 ====================*/
void*   CSystem::GetProcAddress(void *pDll, const tstring &sProcName)
{
    return dlsym(pDll, TStringToNative(sProcName).c_str());
}


/*====================
 CSystem::FreeLibrary
 ====================*/
bool    CSystem::FreeLibrary(void *pDll)
{
    return (dlclose(pDll) == 0);
}


/*====================
 CSystem::GetProcessID
 ====================*/
uint    CSystem::GetProcessID()
{
    return getpid();
}


/*====================
  CSystem::GetVirtualMemoryLimit
  ====================*/
ULONGLONG   CSystem::GetVirtualMemoryLimit() const
{
    return m_ullVirtualMemoryLimit;
}


/*====================
  CSystem::GetDriveList
  ====================*/
void    CSystem::GetDriveList(wsvector &vNames) const
{
    return;
}


/*====================
  CSystem::GetDriveType
  ====================*/
EDriveType  CSystem::GetDriveType(const wstring &sName) const
{
    return DRIVETYPE_INVALID;
}


/*====================
  CSystem::GetDriveSize
  ====================*/
size_t  CSystem::GetDriveSize(const wstring &sName) const
{
    return 0;
}


/*====================
  CSystem::GetDriveFreeSpace
  ====================*/
size_t  CSystem::GetDriveFreeSpace(const wstring &sName) const
{
    return 0;
}


/*====================
  CSystem::GetDriveSizeEx
  ====================*/
ULONGLONG   CSystem::GetDriveSizeEx(const wstring &sName) const
{
    return 0;
}


/*====================
  CSystem::GetDriveFreeSpace
  ====================*/
ULONGLONG   CSystem::GetDriveFreeSpaceEx(const wstring &sName) const
{
    return 0;
}



/*====================
 CSystem::GetCurrentThread
 ====================*/
dword   CSystem::GetCurrentThread()
{
    return (dword)pthread_self();
}


/*====================
  CSystem::Shell
  ====================*/
bool    CSystem::Shell(const tstring &sCommand, bool bWait)
{
    pid_t pid;
    char *command = strdup(TStringToNative(sCommand).c_str());
    char *args[2048];
    char **pargs = args, *s = command;
    
    while (*s && pargs < &args[2047])
    {
        while (*s && IsTokenSeparator(*s)) ++s;
        
        if (*s == '"')
        {
            *pargs++ = s++;
            
            while (*s && *s != '"') ++s;
            if (*++s) *s++ = 0;
            continue;
        }
        
        *pargs++ = s;
        
        while (*s && !IsTokenSeparator(*s)) ++s;
        if (*s) *s++ = 0;
    }
    
    *pargs = NULL;
    
    if ((pid = fork()) == -1)
    {
        Console.Err << _T("Error while spawning process") << newl;
        return false;
    }
    else if (pid == 0)
    {
        execv(args[0], args);
    }
    
    free(command);
    
    if (bWait)
        waitpid(pid, NULL, 0);
    
    return true;
}


/*====================
  CSystem::GetRandomSeed32
  ====================*/
uint    CSystem::GetRandomSeed32()
{
    uint uiSeed;
    int fd(open("/dev/urandom", O_RDONLY));
    read(fd, &uiSeed, sizeof(uiSeed));
    close(fd);

    return uiSeed;
}
