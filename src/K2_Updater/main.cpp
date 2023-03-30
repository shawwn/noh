// (C)2006 S2 Games
// main.cpp
//
// This project provides a "front end" for the Savage 2 executable, which
// installs updates before the files that are to be updated are loaded.
//=============================================================================

#include <Windows.h>
#include <Psapi.h>

#include "resource.h"
#include "commctrl.h"

#include <vector>

using namespace std;

#define MAX_PATH_SIZE 2048

HWND hWndDlg = NULL;
wchar_t g_sErrorFile[MAX_PATH_SIZE];
//char g_sArgs[MAX_PATH_SIZE];

bool m_bDone = false;

void Execute()
{
    PROCESS_INFORMATION pInfo;  //Process information, for starting Savage 2 again
    STARTUPINFO         sInfo;  //Start up information, for starting Savage 2 again
//  wchar_t             wsArgs[MAX_PATH_SIZE];

    memset(&sInfo, 0, sizeof(STARTUPINFO));
    memset(&pInfo, 0, sizeof(PROCESS_INFORMATION));

//  mbstowcs_s(NULL, wsArgs, MAX_PATH_SIZE, sArgs, _TRUNCATE);

    CreateProcess(L"./hon.exe", GetCommandLine(), NULL, NULL, FALSE, 0, NULL, NULL, &sInfo, &pInfo);
}

void GetUpdate(wchar_t sDir[MAX_PATH_SIZE], vector<wchar_t *> &vFileList)
{
    WIN32_FIND_DATA     finddata;
    HANDLE              handle;
    wchar_t             sTargetDir[MAX_PATH_SIZE];

    wcsncpy_s(sTargetDir, MAX_PATH_SIZE, L"./Update/", _TRUNCATE);
    wcsncat_s(sTargetDir, MAX_PATH_SIZE, sDir, _TRUNCATE);
    wcsncat_s(sTargetDir, MAX_PATH_SIZE, L"*", _TRUNCATE);

    //first search for files only
    handle = FindFirstFile(sTargetDir, &finddata);
    if (handle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                wchar_t *sFile = (wchar_t *)malloc(sizeof(wchar_t) * MAX_PATH_SIZE);

                wcsncpy_s(sFile, MAX_PATH_SIZE, sDir, _TRUNCATE);
                wcsncat_s(sFile, MAX_PATH_SIZE, finddata.cFileName, _TRUNCATE);
                vFileList.push_back(sFile);
            }

        }
        while (FindNextFile(handle, &finddata));

        FindClose(handle);
    }

    // next search for directories only, and list their contents

    handle = FindFirstFile(sTargetDir, &finddata);
    if (handle == INVALID_HANDLE_VALUE)
        return;

    do
    {
        if ((finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            if (wcscmp(finddata.cFileName, L".") &&
                wcscmp(finddata.cFileName, L"..") &&
                wcscmp(finddata.cFileName, L"CVS"))
            {
                wchar_t sFile[MAX_PATH_SIZE];

                wcsncpy_s(sFile, MAX_PATH_SIZE, sDir, _TRUNCATE);
                wcsncat_s(sFile, MAX_PATH_SIZE, finddata.cFileName, _TRUNCATE);
                wcsncat_s(sFile, MAX_PATH_SIZE, L"/", _TRUNCATE);

                GetUpdate(sFile, vFileList);
            }
        }
    }
    while (FindNextFile(handle, &finddata));

    FindClose(handle);
}

void DeleteDirectory(wchar_t sDir[MAX_PATH_SIZE])
{
    WIN32_FIND_DATA     finddata;
    HANDLE              handle;
    int                 iRetryTime(0);

    wchar_t sTargetDir[MAX_PATH_SIZE];

    wcsncpy_s(sTargetDir, MAX_PATH_SIZE, sDir, _TRUNCATE);
    wcsncat_s(sTargetDir, MAX_PATH_SIZE, L"*", _TRUNCATE);

    //first search for files only
    handle = FindFirstFile(sTargetDir, &finddata);
    if (handle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                wchar_t sFile[MAX_PATH_SIZE];

                wcsncpy_s(sFile, MAX_PATH_SIZE, sDir, _TRUNCATE);
                wcsncat_s(sFile, MAX_PATH_SIZE, finddata.cFileName, _TRUNCATE);

                iRetryTime = 0;

                while (DeleteFile(sFile) == ERROR_ACCESS_DENIED && iRetryTime < 2500)
                {
                    iRetryTime += 50;
                    Sleep(50);
                }
            }

        }
        while (FindNextFile(handle, &finddata));

        FindClose(handle);
    }


    // next search for directories only
    handle = FindFirstFile(sTargetDir, &finddata);
    if (handle == INVALID_HANDLE_VALUE)
        return;

    do
    {
        if ((finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            if (wcscmp(finddata.cFileName, L".") &&
                wcscmp(finddata.cFileName, L".."))
            {
                wchar_t sFile[1024];

                wcsncpy_s(sFile, MAX_PATH_SIZE, sDir, _TRUNCATE);
                wcsncat_s(sFile, MAX_PATH_SIZE, finddata.cFileName, _TRUNCATE);
                wcsncat_s(sFile, MAX_PATH_SIZE, L"/", _TRUNCATE);

                DeleteDirectory(sFile);
            }
        }
    }
    while (FindNextFile(handle, &finddata));

    FindClose(handle);

    iRetryTime = 0;

    if (!RemoveDirectory(sDir) && iRetryTime < 2500)
    {
        iRetryTime += 50;
        Sleep(50);
    }
}

bool InstallUpdate(vector<wchar_t *> &vFileList)
{
    int iFilesInstalled(0);
    vector<wchar_t *>::iterator it;
    size_t zPos;
    int iRetryTime(0);

    for (it = vFileList.begin(); it != vFileList.end(); it++)
    {
        // Update progress
        SendMessage(GetDlgItem(hWndDlg, IDC_PROGRESS1), PBM_SETPOS, (int)((float(iFilesInstalled) / float(vFileList.size())) * 1000), NULL);
        UpdateWindow(hWndDlg);

        wchar_t sSource[MAX_PATH_SIZE];

        wcsncpy_s(sSource, MAX_PATH_SIZE, L"./Update/", _TRUNCATE);
        wcsncat_s(sSource, MAX_PATH_SIZE, *it, _TRUNCATE);

        DeleteFile(*it);

        while (!MoveFile(sSource, *it) && iRetryTime < 25000)
        {
            int iError = GetLastError();

            if (iError == ERROR_ACCESS_DENIED)
            {
                Sleep(25);
                DeleteFile(*it);
            }
            else if (iError == ERROR_PATH_NOT_FOUND)
            {
                Sleep(25);
                // Attempt to create directory structure
                zPos = 0;
                while (zPos < wcslen(*it))
                {
                    if ((*it)[zPos] == '/')
                    {
                        iRetryTime = 0;

                        (*it)[zPos] = 0;

                        if (wcscmp(*it, L".") &&
                            wcscmp(*it, L".."))
                        {
                            DWORD dwReturn(0);

                            while (!CreateDirectory(*it, NULL) && iRetryTime < 250)
                            {
                                dwReturn = GetLastError();

                                if (dwReturn == ERROR_ALREADY_EXISTS || dwReturn == ERROR_PATH_NOT_FOUND)
                                    break;

                                iRetryTime += 50;
                                Sleep(50);
                            }
                        }

                        (*it)[zPos] = '/';
                    }

                    zPos++;
                }
            }
            else
                Sleep(25);

            iRetryTime += 25;
        }

        if (iRetryTime >= 25000)
        {
            wcsncpy_s(g_sErrorFile, MAX_PATH_SIZE, L"Error updating file: ", _TRUNCATE);
            wcsncat_s(g_sErrorFile, MAX_PATH_SIZE, *it, _TRUNCATE);
            return false;
        }

        iFilesInstalled++;

        MSG msg;

        while(PeekMessage(&msg,0,0,0,PM_REMOVE))
        {
            if(msg.message == WM_QUIT)
                m_bDone = true;

            if(!IsDialogMessage(hWndDlg,&msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        if (m_bDone)
            return false;
    }

    //Delete the update files...
    DeleteDirectory(L"./Update/");

    return true;
}

bool WaitUntilLowestProcess(wchar_t *sProcName)
{
    DWORD dwProcesses[2048];
    DWORD dwBytesWritten;
    int iNumProcs;
    bool bProcessFound(true);
    DWORD lpExitCode(0);

    while (bProcessFound)
    {
        bProcessFound = false;
        
        if (!EnumProcesses(dwProcesses, sizeof(DWORD) * 2048, &dwBytesWritten))
            return false;

        // Calculate how many process identifiers were returned.
        iNumProcs = dwBytesWritten / sizeof(DWORD);

        for (int i = 0; i < iNumProcs; i++)
        {
            if (dwProcesses[i] == GetCurrentProcessId())
                continue;

            if (dwProcesses[i] != 0)
            {
                wchar_t szProcessName[MAX_PATH];

                szProcessName[0] = 0;

                // Get a handle to the process.
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, dwProcesses[i]);

                // Get the process name.
                if (hProcess != NULL)
                {
                    HMODULE hMod;
                    DWORD dwTotalModules;

                    if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &dwTotalModules))
                        GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
                }

                if (wcscmp(szProcessName, sProcName) == 0 && dwProcesses[i] < GetCurrentProcessId())
                {
                    SetDlgItemText(hWndDlg, IDC_LABEL, L"Waiting for Heroes of Newerth to close...");
                    bProcessFound = true;

                    GetExitCodeProcess(hProcess, &lpExitCode);

                    while (lpExitCode == STILL_ACTIVE)
                    {
                        Sleep(1);

                        MSG msg;

                        while(PeekMessage(&msg,0,0,0,PM_REMOVE))
                        {
                            if(msg.message == WM_QUIT)
                                m_bDone = true;

                            if(!IsDialogMessage(hWndDlg,&msg))
                            {
                                TranslateMessage(&msg);
                                DispatchMessage(&msg);
                            }
                        }

                        if (m_bDone)
                            return false;

                        GetExitCodeProcess(hProcess, &lpExitCode);
                    }
                }

                if (hProcess != NULL)
                    CloseHandle(hProcess);
            }
        }
    }

    return true;
}

bool WaitForProcess(wchar_t *sProcName)
{
    DWORD dwProcesses[2048];
    DWORD dwBytesWritten;
    int iNumProcs;
    bool bProcessFound(true);
    DWORD lpExitCode(0);

    while (bProcessFound)
    {
        bProcessFound = false;
        
        if (!EnumProcesses(dwProcesses, sizeof(DWORD) * 2048, &dwBytesWritten))
            return false;

        // Calculate how many process identifiers were returned.
        iNumProcs = dwBytesWritten / sizeof(DWORD);

        for (int i = 0; i < iNumProcs; i++)
        {
            if (dwProcesses[i] == GetCurrentProcessId())
                continue;

            if (dwProcesses[i] != 0)
            {
                wchar_t szProcessName[MAX_PATH];

                szProcessName[0] = 0;

                // Get a handle to the process.
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, dwProcesses[i]);

                // Get the process name.
                if (hProcess != NULL)
                {
                    HMODULE hMod;
                    DWORD dwTotalModules;

                    if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &dwTotalModules))
                        GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
                }

                if (wcscmp(szProcessName, sProcName) == 0)
                {
                    SetDlgItemText(hWndDlg, IDC_LABEL, L"Waiting for Heroes of Newerth to close...");
                    bProcessFound = true;

                    GetExitCodeProcess(hProcess, &lpExitCode);

                    while (lpExitCode == STILL_ACTIVE)
                    {
                        Sleep(1);

                        MSG msg;

                        while(PeekMessage(&msg,0,0,0,PM_REMOVE))
                        {
                            if(msg.message == WM_QUIT)
                                m_bDone = true;

                            if(!IsDialogMessage(hWndDlg,&msg))
                            {
                                TranslateMessage(&msg);
                                DispatchMessage(&msg);
                            }
                        }

                        if (m_bDone)
                            return false;

                        GetExitCodeProcess(hProcess, &lpExitCode);
                    }

                    // Give the process a bit of time to clean up
                    Sleep(500);
                }

                if (hProcess != NULL)
                    CloseHandle(hProcess);
            }
        }
    }

    // Give the process a bit of time to clean up
    Sleep(500);

    return true;
}

LRESULT CALLBACK    MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_COMMAND:
            //control messages
            switch (LOWORD(wParam))
            {
                case IDC_BUTTON2:
                    if (HIWORD(wParam) == BN_CLICKED)
                        m_bDone = true;

                    break;
            }

            return TRUE;

        default:
            return FALSE;//DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

/*====================
  WinMain
  ====================*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    bool bSuccess = true;       //Specifies whether Savage 2 closed successfully.
    vector<wchar_t *> vFiles;   //Files to be updated

    hWndDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOGBAR), 0, (DLGPROC)MainWndProc);

    if (hWndDlg == NULL)
        return 0;

    INITCOMMONCONTROLSEX lpInitControls;
    lpInitControls.dwICC = ICC_PROGRESS_CLASS;
    lpInitControls.dwSize = sizeof(lpInitControls);

    InitCommonControlsEx(&lpInitControls);

    SendMessage(GetDlgItem(hWndDlg, IDC_PROGRESS1), PBM_SETRANGE, NULL, MAKELPARAM(0, 1000));

    ShowWindow(hWndDlg, SW_SHOWNORMAL);
    UpdateWindow(hWndDlg);

    bSuccess = WaitForProcess(L"hon.exe");

    if (bSuccess)
    {
        //strncpy_s(g_sArgs, MAX_PATH_SIZE, GetCommandLine(), _TRUNCATE);

        bSuccess = WaitUntilLowestProcess(L"hon_update.exe");

        if (bSuccess)
        {
            //Grab the file list for the update
            GetUpdate(L"./", vFiles);

            bool bValidUpdate(false);
            bool bMovedEXE(false);
            vector<wchar_t *>::iterator it(vFiles.begin());

            while (it != vFiles.end())
            {
                if (wcscmp(*it, L"./hon.exe") == 0 && !bMovedEXE)
                {
                    // Make sure that hon.exe is the last file that's updated.
                    it = vFiles.erase(it);
                    vFiles.push_back(L"./hon.exe");
                    bMovedEXE = true;
                    continue;
                }

                if (wcscmp(*it, L"./verify") == 0)
                {
                    bValidUpdate = true;
                    it = vFiles.erase(it);
                    continue;
                }

                it++;
            }

            for (it = vFiles.begin(); it != vFiles.end(); it++)
            {
                if (wcscmp(*it, L"./hon_update.exe") == 0)
                {
                    wcsncpy_s(g_sErrorFile, MAX_PATH_SIZE, L"Update not downloaded properly! Please rerun the in-game updater.", _TRUNCATE);
                    wcsncat_s(g_sErrorFile, MAX_PATH_SIZE, *it, _TRUNCATE);
                    DeleteFile(L"./Update/hon_update.exe");
                    DeleteFile(L"./Update/hon.exe");
                    bValidUpdate = false;
                    bSuccess = false;
                    vFiles.erase(it);
                    break;
                }
            }

            //If there is an update, keep going.
            if (bValidUpdate)
            {
                SetDlgItemText(hWndDlg, IDC_LABEL, L"Installing update...");
                bSuccess = InstallUpdate(vFiles);

                vFiles.clear();
            }
        }
    }


    if (bSuccess)
    {
        Execute();
        m_bDone = true;
    }
    else if (hWndDlg != NULL)
    {
        SetDlgItemText(hWndDlg, IDC_LABEL, g_sErrorFile);
        SendMessage(GetDlgItem(hWndDlg, IDC_PROGRESS1), PBM_SETPOS, 0, NULL);
    }

    while (!m_bDone && hWndDlg != NULL)
    {
        Sleep(1);

        MSG msg;
  
        WaitMessage();
        
        while(PeekMessage(&msg,0,0,0,PM_REMOVE))
        {
            if(msg.message == WM_QUIT)
            {
                m_bDone = true;
                PostMessage(NULL,WM_QUIT,0,0);
                break;
            }

            if(!IsDialogMessage(hWndDlg,&msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    if (hWndDlg != NULL)
        DestroyWindow(hWndDlg);

    Sleep(10000);

    return 0;   
}
