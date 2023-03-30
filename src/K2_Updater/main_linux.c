// (C)2006 S2 Games
// main.cpp
//
// This project provides a "front end" for the Savage 2 executable, which
// installs updates before the files that are to be updated are loaded.
//=============================================================================

#define _GNU_SOURCE // euidaccess
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <glob.h>

#include "ui_linux.h"
extern int stdio_GetUIAPI(struct UIAPI* api);
extern int x11_GetUIAPI(struct UIAPI* api);

#define MAX_PATH_SIZE FILENAME_MAX

#ifdef _DEBUG
#define SUFFIX "_debug"
#else
#define SUFFIX ""
#endif

#ifdef __x86_64__
#define ARCH "x86_64"
#else
#define ARCH "x86"
#endif
struct UIAPI UI;

struct SFileList
{
    int iSize;
    int iAllocatedSize;
    char **sFiles;
};

inline static void SFileList_Init(struct SFileList* pFileList, int iInitialAllocatedSize)
{
    pFileList->iSize = 0;
    pFileList->iAllocatedSize = iInitialAllocatedSize;
    pFileList->sFiles = malloc(sizeof(char*) * pFileList->iAllocatedSize);
}

inline static void SFileList_Add(struct SFileList* pFileList, char *s)
{
    if (pFileList->iSize >= pFileList->iAllocatedSize)
    {
        pFileList->iAllocatedSize += 32;
        pFileList->sFiles = realloc(pFileList->sFiles, sizeof(char*) * pFileList->iAllocatedSize);
    }
    pFileList->sFiles[pFileList->iSize++] = s;
}

inline static void SFileList_Cleanup(struct SFileList* FileList, bool bFreeElements)
{
    int i;
    if (bFreeElements)
    {
        for (i = 0; i < FileList->iSize; ++i)
        {
            free(FileList->sFiles[i]);
        }
    }
    free(FileList->sFiles);
}

inline static bool IsDir(const char* path)
{
    struct stat s;
    
    stat(path, &s);
    
    return S_ISDIR(s.st_mode);
}

static void GetUpdate(char sDir[MAX_PATH_SIZE], struct SFileList *pFileList, int iIgnoreChars)
{
    char sTargetDir[MAX_PATH_SIZE];
    glob_t globbuf;
    int i;
    
    strncpy(sTargetDir, sDir, MAX_PATH_SIZE-1);
    strncat(sTargetDir, "/*", MAX_PATH_SIZE-1);
    
    glob(sTargetDir, 0, NULL, &globbuf);
    
    for (i = 0; i < globbuf.gl_pathc; ++i)
    {
        if (!IsDir(globbuf.gl_pathv[i]))
        {
            SFileList_Add(pFileList, strdup(&globbuf.gl_pathv[i][iIgnoreChars]));
        }
    }
    
    for (i = 0; i < globbuf.gl_pathc; ++i)
    {
        int j = strlen(sDir)+1;
        if (IsDir(globbuf.gl_pathv[i]) &&
            strcmp(&globbuf.gl_pathv[i][j], ".") &&
            strcmp(&globbuf.gl_pathv[i][j], "..") && 
            strcmp(&globbuf.gl_pathv[i][j], "CVS"))
        {
            GetUpdate(globbuf.gl_pathv[i], pFileList, iIgnoreChars);
        }
    }
    
    globfree(&globbuf);
}

static void DeleteDirectory(char sDir[MAX_PATH_SIZE])
{
    int i;
    
    if (rmdir(sDir) == -1)
    {
        if (errno == ENOTEMPTY)
        {
            char sTargetDir[MAX_PATH_SIZE];
            glob_t globbuf;
            
            strncpy(sTargetDir, sDir, MAX_PATH_SIZE-1);
            strncat(sTargetDir, "/*", MAX_PATH_SIZE-1);
            
            glob(sTargetDir, 0, NULL, &globbuf);
            
            for (i =0; i < globbuf.gl_pathc; ++i)
            {
                DeleteDirectory(globbuf.gl_pathv[i]);
            }
            
            globfree(&globbuf);
            
            rmdir(sDir);
        }
    }
}

static bool CreateDirectory(char* pDir)
{
    int i;
    for (i = 0; i < strlen(pDir); i++)
    {
        if (pDir[i] == '/')
        {
            pDir[i] = 0;
            if ((mkdir(pDir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == -1) && (errno != EEXIST))
            {
                pDir[i] = '/';
                return false;
            }
            pDir[i] = '/';
        }
    }
    return true;
}

static bool InstallUpdate(struct SFileList* pFileList)
{
    int i;
    for (i = 0; i < pFileList->iSize-1; ++i)
    {
        char sSource[MAX_PATH_SIZE];
        
        strncpy(sSource, "Update/", MAX_PATH_SIZE-1);
        strncat(sSource, pFileList->sFiles[i], MAX_PATH_SIZE-1);
        
        if (rename(sSource, pFileList->sFiles[i]) == -1)
        {
            if (errno != ENOENT || !CreateDirectory(pFileList->sFiles[i]) || (rename(sSource, pFileList->sFiles[i]) == -1))
            {
                char sError[MAX_PATH_SIZE+21] = "Error updating file: ";
                strncat(sError, pFileList->sFiles[i], MAX_PATH_SIZE+20);
                UI.ErrorMessage(sError);
                return false;
            }
        }
        
        UI.SetProgress((float)(i+1) / pFileList->iSize);
        if (UI.Update())
        {
            return false; // canceled
        }
    }
    
    struct stat buf;
    if (stat("hon" SUFFIX "-x86", &buf) == 0)
        chmod("hon" SUFFIX "-x86", buf.st_mode & 07777);
    if (stat("hon" SUFFIX "-x86_64", &buf) == 0)
        chmod("hon" SUFFIX "-x86_64", buf.st_mode & 07777);
    
    DeleteDirectory("Update");
    return true;
}


int main(int argc, char *argv[])
{
    struct SFileList FileList; // Files to be updated
    int i;
    bool bSuccess = true;
    bool bValidUpdate = false;
    bool bMovedBinary = false;
    
    mode_t previous = umask(0);
    umask(previous | S_IWGRP | S_IWOTH);
    
    // Change to the working directory to the location of savage2_update.bin
    // check for it in the user's path if argv[0] is not a relative/absolute path & then cd to that dir
    if (strchr(argv[0], '/') == NULL)
    {
        char *p, **paths, *path = strdup(getenv("PATH"));
        int i, n = 1;
        
        p = path;
        
        while ((p = strchr(p, ':'))) n++, p++;
        
        paths = (char**)malloc(sizeof(char*) * n);
        
        paths[0] = path;
        
        for (i = n - 1; i > 0; i--)
        {
            paths[i] = strrchr(path, ':') + 1;
            paths[i][-1] = '\0';
        }
        
        for (i = 0; i < n; i++)
        {
            struct stat s;
            char sBuf[FILENAME_MAX];
            snprintf(sBuf, FILENAME_MAX, "%s/%s", *paths[i] ? paths[i] : ".", argv[0]);
            if (lstat(sBuf, &s) == 0)
            {
                chdir(paths[i]);
                break;
            }
        }
        
        free(path);
        free(paths);
    }
    
    // resolve symlinks & cd
    char *p, *rootDir = canonicalize_file_name(argv[0]);
    if ((p = strrchr(rootDir, '/')))
        *p = '\0';
    chdir(rootDir);

    // wait for process to stop?
    
    // init x11 ui
    x11_GetUIAPI(&UI);
    if (UI.Init("Heroes of Newerth Updater", NULL) == -1)
    {
        // use stdio
        stdio_GetUIAPI(&UI);
        UI.Init("Heroes of Newerth Updater", NULL);
    }
    
    // get the file list
    SFileList_Init(&FileList, 64);
    UI.SetMessage("Checking for updates...");
    GetUpdate("Update", &FileList, strlen("Update")+1);
    
    for (i = 0; i < FileList.iSize; ++i)
    {
        if (!bMovedBinary && strcmp(FileList.sFiles[i], "hon" SUFFIX "-" ARCH) == 0)
        {
            // Make sure that hon-x86* is the last file that's updated.
            char* tmp = FileList.sFiles[i];
            bMovedBinary = true;
            FileList.sFiles[i] = FileList.sFiles[FileList.iSize-2];
            FileList.sFiles[FileList.iSize-2] = tmp;
        }
        
        if (strcmp(FileList.sFiles[i], "verify") == 0)
        {
            // verify gets moved to the end of the list (and then never installed)
            char* tmp = FileList.sFiles[i];
            bValidUpdate = true;
            FileList.sFiles[i] = FileList.sFiles[FileList.iSize-1];
            FileList.sFiles[FileList.iSize-1] = tmp;
            remove("Update/verify");
        }
    }
    
    if (bValidUpdate)
    {
        if (euidaccess(".", W_OK) != 0)
        {
            bSuccess = false;
            UI.ErrorMessage("Unable to write to game directory. The game must be run by the game directory's owner in order to update.");
        }
        
        if (bSuccess)
        {
            for (i = 0; i < FileList.iSize-1; ++i)
            {
                if (strcmp(FileList.sFiles[i], "hon_update" SUFFIX "-" ARCH) == 0)
                {
                    UI.ErrorMessage("Update not downloaded properly! Please rerun the in-game updater.");
                    remove("Update/hon_update" SUFFIX "-x86");
                    remove("Update/hon_update" SUFFIX "-x86_64");
                    remove("Update/hon" SUFFIX "-x86");
                    remove("Update/hon" SUFFIX "-x86_64");
                    bSuccess = false;
                    break;
                }
            }
        }
    
        if (bSuccess)
        {
            UI.SetMessage("Installing update...");
            bSuccess = InstallUpdate(&FileList);
        }
        
        struct stat buf;
        if (stat("hon.sh", &buf) == 0)
            chmod("hon.sh", (buf.st_mode & 07777) | S_IXUSR | S_IXGRP | S_IXOTH);
        if (stat("hon" SUFFIX "-x86", &buf) == 0)
            chmod("hon" SUFFIX "-x86", (buf.st_mode & 07777) | S_IXUSR | S_IXGRP | S_IXOTH);
        if (stat("hon" SUFFIX "-x86_64", &buf) == 0)
            chmod("hon" SUFFIX "-x86_64", (buf.st_mode & 07777) | S_IXUSR | S_IXGRP | S_IXOTH);
        if (stat("editor.sh", &buf) == 0)
            chmod("editor.sh", (buf.st_mode & 07777) | S_IXUSR | S_IXGRP | S_IXOTH);
        
        if (bSuccess)
        {
            UI.SetMessage("Update installed successfully.");
            UI.Update();
        }
    }
    
    SFileList_Cleanup(&FileList, true);
    
    UI.Cleanup();
    
    if (bSuccess)
    {
        argv[0] = "./hon" SUFFIX "-" ARCH;
        execv(argv[0], argv);
    }

    return 0;
}
