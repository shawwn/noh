// (C)2005 S2 Games
// c_system_win32.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"
#ifdef _WIN32

#include "c_system.h"

#include "k2_include_windows.h"
#include <direct.h>
#include <shlobj.h>
#include <mmsystem.h>
#include <winsock2.h>
#include <dbghelp.h>
#include <Psapi.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <Iphlpapi.h>
#include <mmsystem.h>

#include "c_filemanager.h"
#include "c_input.h"
#include "c_cmd.h"
#include "c_function.h"
#include "c_vid.h"
#include "resource.h"
#include "c_soundmanager.h"
#include "c_bitmap.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
EXTERN_CVAR_STRING(host_startupCfg);
EXTERN_CVAR_BOOL(host_dynamicResReload);

struct SFileMonitorInfo
{
    byte                    FileNotifyInfoBuffer[1024];
    OVERLAPPED              Overlapped;
    HANDLE                  hDirectory;
};

typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(
    HANDLE hProcess,
    DWORD dwPid,
    HANDLE hFile,
    MINIDUMP_TYPE DumpType,
    CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
    CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
    CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
    );
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
SINGLETON_INIT(CSystem);
CSystem& K2System(*CSystem::GetInstance());

CVAR_BOOLF  (sys_autoSaveConfig,        true,   CVAR_SAVECONFIG);
CVAR_BOOLF  (sys_fileChangeNotify,      true,   CONEL_DEV);

CVAR_BOOLF  (sys_autoSaveDump,          false,  CVAR_SAVECONFIG);
CVAR_BOOL   (sys_dumpOnFatal,           false);

#if defined(_DEBUG)
CVAR_BOOL   (sys_debugOutput,           true);
#else
CVAR_BOOL   (sys_debugOutput,           false);
#endif

// Keyboard settings
CVAR_BOOLF(key_splitShift,  false, CVAR_SAVECONFIG);
CVAR_BOOLF(key_splitAlt,    false, CVAR_SAVECONFIG);
CVAR_BOOLF(key_splitCtrl,   false, CVAR_SAVECONFIG);
CVAR_BOOLF(key_splitWin,    false, CVAR_SAVECONFIG);
CVAR_BOOLF(key_splitEnter,  false, CVAR_SAVECONFIG);
//=============================================================================

/*====================
  DirectoryChangeNotify
  ====================*/
VOID CALLBACK   DirectoryChangeNotify(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
{
    K2System.SetDirectoryChangeNotified(true);

    if (dwErrorCode != 0)
    {
        Console.Err << _T("DirectoryChangeNotify() - Error: ") << dwErrorCode << newl;
        return;
    }

    SFileMonitorInfo *pFileMonitorInfo(reinterpret_cast<SFileMonitorInfo*>(K2System.GetFileMonitorInfo()));

    FILE_NOTIFY_INFORMATION *pInfo(reinterpret_cast<FILE_NOTIFY_INFORMATION *>(&pFileMonitorInfo->FileNotifyInfoBuffer));

    while (pInfo)
    {
        TCHAR tszFileName[_MAX_PATH + 1];
        MemManager.Set(tszFileName, 0, sizeof(tszFileName));
        WideToTCHAR(tszFileName, _MAX_PATH + 1, pInfo->FileName, pInfo->FileNameLength / sizeof(WCHAR));
        tstring sFileName(tszFileName);

        sFileName = FileManager.SanitizePath(sFileName);
        size_t zPos(sFileName.find_first_of(_T("/"), 1));
        if (zPos != tstring::npos)
            sFileName = sFileName.substr(zPos);
        K2System.AddModifiedPath(sFileName);

        if (sys_fileChangeNotify)
        {
            //Console.Dev << Host.GetFrame() << newl;
            Console.Dev << _T("DirectoryChangeNotify() - File: ") << sFileName << _T(" [") << pInfo->FileNameLength << _T("] ");
            switch (pInfo->Action)
            {
            case FILE_ACTION_ADDED:
                Console.Dev << _T(" ADDED") << newl;
                break;

            case FILE_ACTION_MODIFIED:
                Console.Dev << _T(" MODIFIED") << newl;
                break;

            case FILE_ACTION_REMOVED:
                Console.Dev << _T(" REMOVED") << newl;
                break;

            case FILE_ACTION_RENAMED_NEW_NAME:
                Console.Dev << _T(" NEW NAME") << newl;
                break;

            case FILE_ACTION_RENAMED_OLD_NAME:
                Console.Dev << _T(" OLD NAME") << newl;
                break;
            }
        }

        if (pInfo->NextEntryOffset)
            pInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION *>(reinterpret_cast<byte *>(pInfo) + pInfo->NextEntryOffset);
        else
            pInfo = nullptr;
    }
}


/*====================
  System_ExceptionFilter
  ====================*/
LONG WINAPI System_ExceptionFilter(EXCEPTION_POINTERS *pExceptionInfo)
{
    Vid.Shutdown();
    K2System.ShowMouseCursor();

    if (!sys_autoSaveDump)
    {
        if (MessageBox(nullptr,
            _T("Would you like to save a crash report?"),
            _T("K2 Exception"),
            MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
            return EXCEPTION_EXECUTE_HANDLER;
    }

    HMODULE hDebugLib(LoadLibrary(_T("dbghelp.dll")));
    if (hDebugLib == nullptr)
    {
        MessageBox(nullptr, _T("System_ExceptionFilter() - Could not load dbghelp.dll"), _T("K2 Exception"), MB_OK | MB_ICONEXCLAMATION);
        return EXCEPTION_EXECUTE_HANDLER;
    }

    MINIDUMPWRITEDUMP fnDump((MINIDUMPWRITEDUMP)GetProcAddress(hDebugLib, "MiniDumpWriteDump"));
    if (fnDump == nullptr)
    {
        MessageBox(nullptr, _T("System_ExceptionFilter() - Could not find function MiniDumpWriteDump"), _T("K2 Exception"), MB_OK | MB_ICONEXCLAMATION);
        return EXCEPTION_EXECUTE_HANDLER;
    }

    tstring sDumpPath(FileManager.GetNextFileIncrement(4, _T("~/crash_") + K2System.GetVersionString() + _T("_"), _T("dmp")));
    sDumpPath = FileManager.GetSystemPath(sDumpPath, _T(""), true, true);
    HANDLE hFile(CreateFile(sDumpPath.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr));
    if (hFile == nullptr)
    {
        MessageBox(nullptr, _T("System_ExceptionFilter() - Failed to create dump file"), _T("K2 Exception"), MB_OK | MB_ICONEXCLAMATION);
        return EXCEPTION_EXECUTE_HANDLER;
    }

    MINIDUMP_EXCEPTION_INFORMATION ExInfo;
    ExInfo.ThreadId = GetCurrentThreadId();
    ExInfo.ExceptionPointers = pExceptionInfo;
    ExInfo.ClientPointers = nullptr;

    if (fnDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MINIDUMP_TYPE(MiniDumpWithDataSegs | MiniDumpWithIndirectlyReferencedMemory), &ExInfo, nullptr, nullptr) == FALSE)
    {
        CloseHandle(hFile);
        MessageBox(nullptr, _T("System_ExceptionFilter() - Failed to write dump file"), _T("K2 Exception"), MB_OK | MB_ICONEXCLAMATION);
        return EXCEPTION_EXECUTE_HANDLER;
    }

    if (!sys_autoSaveDump)
    {
        tstring sMsg(_T("Successfully wrote file: ") + sDumpPath);
        MessageBox(nullptr, sMsg.c_str(), _T("K2 Exception"), MB_OK);
    }

    CloseHandle(hFile);
    return EXCEPTION_EXECUTE_HANDLER;
}


/*====================
  System_DummySetUnhandledExceptionFilter
  ====================*/
LPTOP_LEVEL_EXCEPTION_FILTER WINAPI System_DummySetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
    return nullptr;
}


/*====================
  CSystem::PreventSetUnhandledExceptionFilter

  This hooks the system's SetUnhandledExceptionFilter so that
  K2's exception filter cannot be replaced

  This only works on x86
  ====================*/
bool    CSystem::PreventSetUnhandledExceptionFilter()
{
#ifdef _M_IX86
    HMODULE hKernel32(::LoadLibrary(_T("kernel32.dll")));
    if (hKernel32 == nullptr)
        return false;

    void *pOrgEntry(::GetProcAddress(hKernel32, "SetUnhandledExceptionFilter"));
    if (pOrgEntry == nullptr)
        return false;

    unsigned char newJump[100];
    INT_PTR dwOrgEntryAddr((INT_PTR)pOrgEntry);
    dwOrgEntryAddr += 5; // add 5 for 5 op-codes for jmp far
    void *pNewFunc(&System_DummySetUnhandledExceptionFilter);
    INT_PTR dwNewEntryAddr((INT_PTR)pNewFunc);
    INT_PTR dwRelativeAddr(dwNewEntryAddr - dwOrgEntryAddr);

    newJump[0] = 0xE9;  // JMP absolute
    memcpy(&newJump[1], &dwRelativeAddr, sizeof(pNewFunc));
    SIZE_T bytesWritten;
    BOOL bRet = ::WriteProcessMemory(GetCurrentProcess(), pOrgEntry, newJump, sizeof(pNewFunc) + 1, &bytesWritten);
    return bRet != FALSE;
#else
    return false;
#endif
}


/*====================
  CSystem::CSystem
  ====================*/
CSystem::CSystem() :
m_WindowHandle(nullptr),
m_sCommandLine(::GetCommandLine()),
m_hInstance(nullptr),
m_bMonitoringActive(false),
m_bRestartProcess(false),
m_pFileMonitorInfo(nullptr),
m_bDedicatedServer(false),
m_bServerManager(false),
m_ConsoleWindowHandle(nullptr),
m_ullVirtualMemoryLimit(0)
{
    // Set system start time so K2System.Milliseconds counts from 0
    LARGE_INTEGER ll;
    ::QueryPerformanceCounter(&ll);
    m_llStartTicks = ll.QuadPart;

    // Set process affinity
    SetAffinity(0);

    m_hMainThread = GetCurrentThread();

    SYSTEM_INFO sysInfo;
    ::GetSystemInfo(&sysInfo);

    m_dwPageSize = sysInfo.dwAllocationGranularity;

    // Determine total available virtual memory
    MEMORYSTATUSEX memorystatus;
    memset(&memorystatus, 0, sizeof(MEMORYSTATUSEX));
    memorystatus.dwLength = sizeof(MEMORYSTATUSEX);

    if (::GlobalMemoryStatusEx(&memorystatus))
    {
        m_ullVirtualMemoryLimit = memorystatus.ullTotalVirtual;
    }
}


/*====================
  CSystem::Init
  ====================*/
void    CSystem::Init(const tstring &sGameName, const tstring &sVersion, const tstring &sBuildInfo, const tstring &sBuildNumber, const tstring &sBuildOS, const tstring &sBuildOSCode, const tstring &sBuildArch, const string &sMasterServerAddress)
{
    m_sConsoleTitle = m_sGameName = sGameName;
    m_sVersion = sVersion;
    m_sBuildNumber = sBuildNumber;
    m_sBuildInfo = sBuildInfo;
    m_sMasterServerAddress = sMasterServerAddress;
    
    m_sBuildOS = sBuildOS;
    m_sBuildOSCode = sBuildOSCode;
    m_sBuildArch = sBuildArch;

    srand(uint(m_llStartTicks & UINT_MAX));

    // Setup custom error handling
    SetUnhandledExceptionFilter(System_ExceptionFilter);
    PreventSetUnhandledExceptionFilter();

    SetErrorMode(SetErrorMode(0) | SEM_NOGPFAULTERRORBOX);

    // Change the working directory to the location of core.exe
    tstring sStartDir(m_sCommandLine);
    sStartDir = sStartDir.substr(1, sStartDir.find_first_of(_T('"'), 1) - 1);
    sStartDir = sStartDir.substr(0, sStartDir.find_last_of(_T('\\')));
    _tchdir(sStartDir.c_str());

    // Now that we have the executable path, strip out that part of the command line
    m_sCommandLine = m_sCommandLine.substr(m_sCommandLine.find_first_of(_T('"'), 1) + 1);

    // Set the root path
    TCHAR szDir[_MAX_PATH + 1];
    _tgetcwd(szDir, _MAX_PATH);
    m_sRootDir = szDir;
    m_sRootDir += _T('/');

    // Set the user directory (in "My Documents")
    SetConfig(_T(""));

    // Process the base paths after they have both been initially set,
    // because FileManager could behave poorly if these are not yet set.
    m_sRootDir = FileManager.SanitizePath(m_sRootDir, false);
    m_sUserDir = FileManager.SanitizePath(m_sUserDir, false);

    // Shut down screensaver
    SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, &m_iScreenSaverActive, 0);
    SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, FALSE, 0, 0);

    // Store the current mouse clipping area
    RECT mouseclip;
    GetClipCursor(&mouseclip);
    m_recMouseClip.top = mouseclip.top;
    m_recMouseClip.left = mouseclip.left;
    m_recMouseClip.bottom = mouseclip.bottom;
    m_recMouseClip.right = mouseclip.right;

    m_bHasFocus = true;

    timeBeginPeriod(1);
}


/*====================
  CSystem::InitMore
  ====================*/
void    CSystem::InitMore()
{
}


/*====================
  CSystem::~CSystem
  ====================*/
CSystem::~CSystem()
{
    if (m_iScreenSaverActive)
        SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, TRUE, 0, 0);

    // Restore the original mouse clipping area,
    // in case the program has altered it
    RECT mouseclip = { m_recMouseClip.top, m_recMouseClip.left,
                    m_recMouseClip.bottom, m_recMouseClip.right };
    ClipCursor(&mouseclip);

    timeEndPeriod(1);
}


/*====================
  CSystem::HandleOSMessages
  ====================*/
void    CSystem::HandleOSMessages()
{
    PROFILE("CSystem::HandleOSMessages");

    if (m_ConsoleWindowHandle)
    {
        HANDLE hConsoleInput(GetStdHandle(STD_INPUT_HANDLE));
        DWORD cNumRead;
        INPUT_RECORD irInBuf[128];

        if (GetNumberOfConsoleInputEvents(hConsoleInput, &cNumRead))
        {
            if (cNumRead > 0 && ReadConsoleInput(hConsoleInput, irInBuf, 128, &cNumRead))
            {
                // Dispatch the events to the appropriate handler
                for (uint ui(0); ui < cNumRead; ++ui)
                {
                    switch (irInBuf[ui].EventType)
                    {
                    case KEY_EVENT: // keyboard input
                        Input.AddEvent(GetButton(irInBuf[ui].Event.KeyEvent.wVirtualKeyCode, 0), irInBuf[ui].Event.KeyEvent.bKeyDown == TRUE);
                        if (irInBuf[ui].Event.KeyEvent.bKeyDown == TRUE)
                        {
                            Input.AddEvent(irInBuf[ui].Event.KeyEvent.uChar.AsciiChar);
                            //WriteConsole((HANDLE)m_ConsoleWindowHandle, &irInBuf[ui].Event.KeyEvent.uChar.AsciiChar, 1, nullptr, nullptr);
                        }
                        break;

                    case FOCUS_EVENT:  // disregard focus events
                        //Console << _T("FOCUS_EVENT: ") << newl;
                        break;

                    case MENU_EVENT:   // disregard menu events
                        //Console << _T("MENU_EVENT: ") << newl;
                        break;

                    default:
                        break;
                    }
                }
            }
        }
    }

    MSG msg;

    while (PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE))
    {
        // Check for a WM_QUIT message
        if (GetMessage(&msg, nullptr, 0, 0) == 0)
            Exit(0);

        if (m_ConsoleWindowHandle && IsDialogMessage((HWND)m_ConsoleWindowHandle, &msg))
            continue;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}


/*====================
  CSystem::Exit
  ====================*/
void    CSystem::Exit(int iErrorLevel)
{
    Console.FlushLogs();

    if (sys_autoSaveConfig && !Host.GetNoConfig())
    {
        tsvector vArgs;
        vArgs.push_back(host_startupCfg);
        CConsoleElement *pElem(ConsoleRegistry.GetElement(_T("WriteConfigScript")));
        if (pElem != nullptr)
            pElem->Execute(vArgs);
    }

    Vid.Shutdown();
    Host.Shutdown();

    if (m_iScreenSaverActive)
        SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, TRUE, 0, 0);

    if (m_bRestartProcess)
    {
        tstring sCmdLine;
        tstring sFilename;
        TCHAR szCmdLine[32768];
        TCHAR szFilename[32768];

        sCmdLine = m_sCommandLine;
        sFilename = GetRootDir() + _T("./hon_update.exe");

        _TCSNCPY_S(szCmdLine, 32768, sCmdLine.c_str(), _TRUNCATE);
        _TCSNCPY_S(szFilename, 32768, sFilename.c_str(), _TRUNCATE);

        STARTUPINFO startInfo;
        MemManager.Set(&startInfo, 0, sizeof(STARTUPINFO));
        PROCESS_INFORMATION procInfo;
        MemManager.Set(&procInfo, 0, sizeof(PROCESS_INFORMATION));
        CreateProcess(szFilename, szCmdLine, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &startInfo, &procInfo);

        Console << _T("CMD LINE: ") << szCmdLine << newl;
    }

    exit(iErrorLevel);
}


/*====================
  CSystem::DebugOutput
  ====================*/
void    CSystem::DebugOutput(const tstring &sOut)
{
    OutputDebugString(sOut.c_str());

    if (!sys_debugOutput)
        return;
#ifdef UNICODE
    wprintf(_T("%ls"), sOut.c_str());
#else
    printf("%s", sOut.c_str());
#endif
    fflush(stdout);
}


/*====================
  CSystem::Sleep
  ====================*/
void    CSystem::Sleep(uint uiMsecs)
{
    ::Sleep(uiMsecs);
}


/*====================
  CSystem::DebugBreak
  ====================*/
void    CSystem::DebugBreak()
{
    __asm int 0x03;
}


/*====================
  CSystem::GetLastError
  ====================*/
uint    CSystem::GetLastError()
{
    return ::GetLastError();
}


/*====================
  CSystem::GetErrorString
  ====================*/
tstring CSystem::GetErrorString(uint err)
{
    TCHAR szError[1024];
    _tcscpy(szError, _T("Unknown error."));

    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
        err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), szError, 1024, 0);

    // Remove the CR/LF that FormatMessage() tacks on
    tstring sError(szError);
    size_t zEnd(sError.find_last_not_of(_T("\n\r")));
    return sError.substr(0, zEnd == tstring::npos ? zEnd : zEnd + 1);
}


/*====================
  CSystem::GetLastErrorString
  ====================*/
tstring CSystem::GetLastErrorString()
{
    return GetErrorString(GetLastError());
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

        if (_tmkdir(sDir.c_str()) == 0)
        {
            bRet = true;
            continue;
        }

        if (errno != EEXIST)
        {
            DECLARE_TCSERROR_S_BUFFER(szError, 1024);
            _TCSERROR_S(szError, 1024, errno);
            Console.Warn << _T("Failed to create directory ") << QuoteStr(sDir) << _T(", ") << szError << newl;
            bRet = false;
        }
    }

    return bRet;
}


/*====================
  CSystem::StartDirectoryMonitoring
  ====================*/
void    CSystem::StartDirectoryMonitoring()
{
    if (host_dynamicResReload && !m_pFileMonitorInfo)
    {
        // Set up file monitoring
        SFileMonitorInfo *pFileMonitorInfo(K2_NEW(ctx_System,  SFileMonitorInfo));
        if (pFileMonitorInfo == nullptr)
        {
            Console.Err << _T("Failed to allocate FileMonitorInfo") << newl;
            return;
        }

        MemManager.Set(pFileMonitorInfo, 0, sizeof(SFileMonitorInfo));
        m_pFileMonitorInfo = pFileMonitorInfo;

        pFileMonitorInfo->hDirectory = CreateFile(m_sRootDir.c_str(), FILE_LIST_DIRECTORY, FILE_SHARE_READ,
            nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);

        if (!ReadDirectoryChangesW(pFileMonitorInfo->hDirectory, pFileMonitorInfo->FileNotifyInfoBuffer, sizeof(pFileMonitorInfo->FileNotifyInfoBuffer),
            TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_SIZE, nullptr, &pFileMonitorInfo->Overlapped, DirectoryChangeNotify))
            Console.Err << _T("ReadDirectoryChangesW: ") << GetLastErrorString() << newl;

        m_bMonitoringActive = true;
    }
}


/*====================
  CSystem::GetModifiedFileList
  ====================*/
void    CSystem::GetModifiedFileList(tsvector &vFileList)
{
    if (!m_bMonitoringActive)
        return;

    m_setsModifiedFiles.clear();

    m_bDirectoryChangeNotified = false;

    ::SleepEx(0, TRUE);

    if (m_bDirectoryChangeNotified)
    {
        if (host_dynamicResReload)
        {
            // This catches the second event that likes to pop up when a file changes
            // By catching it in the same frame, we avoid multiple reloads because the modified files are stored in a set
            SFileMonitorInfo *pFileMonitorInfo(reinterpret_cast<SFileMonitorInfo*>(m_pFileMonitorInfo));
            if (!ReadDirectoryChangesW(pFileMonitorInfo->hDirectory, pFileMonitorInfo->FileNotifyInfoBuffer, sizeof(pFileMonitorInfo->FileNotifyInfoBuffer),
                TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_SIZE, nullptr, &pFileMonitorInfo->Overlapped, DirectoryChangeNotify))
                Console.Err << _T("ReadDirectoryChangesW: ") << GetLastErrorString() << newl;
            ::SleepEx(0, TRUE);
            if (!ReadDirectoryChangesW(pFileMonitorInfo->hDirectory, pFileMonitorInfo->FileNotifyInfoBuffer, sizeof(pFileMonitorInfo->FileNotifyInfoBuffer),
                TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_SIZE, nullptr, &pFileMonitorInfo->Overlapped, DirectoryChangeNotify))
                Console.Err << _T("ReadDirectoryChangesW: ") << GetLastErrorString() << newl;

            for (sset::iterator it(m_setsModifiedFiles.begin()); it != m_setsModifiedFiles.end(); ++it)
                vFileList.push_back(*it);
        }
        else
        {
            StopDirectoryMonitoring();
        }
    }
}


/*====================
  CSystem::FlushDirectoryMonitoring
  ====================*/
void    CSystem::FlushDirectoryMonitoring()
{
    bool bNotify(sys_fileChangeNotify);
    sys_fileChangeNotify = false;

    m_setsModifiedFiles.clear();

    ::SleepEx(0, TRUE);
    SFileMonitorInfo *pFileMonitorInfo(reinterpret_cast<SFileMonitorInfo*>(m_pFileMonitorInfo));
    if (!ReadDirectoryChangesW(pFileMonitorInfo->hDirectory, pFileMonitorInfo->FileNotifyInfoBuffer, sizeof(pFileMonitorInfo->FileNotifyInfoBuffer),
        TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_SIZE, nullptr, &pFileMonitorInfo->Overlapped, DirectoryChangeNotify))
        Console.Err << _T("ReadDirectoryChangesW: ") << GetLastErrorString() << newl;

    sys_fileChangeNotify = bNotify;
}


/*====================
  CSystem::StopDirectoryMonitoring
  ====================*/
void    CSystem::StopDirectoryMonitoring()
{
    m_bMonitoringActive = false;
    if (m_pFileMonitorInfo != nullptr)
    {
        SFileMonitorInfo *pFileMonitorInfo(reinterpret_cast<SFileMonitorInfo*>(m_pFileMonitorInfo));
        CloseHandle(pFileMonitorInfo->hDirectory);
        SAFE_DELETE(m_pFileMonitorInfo);
    }
}


/*====================
  CSystem::AddModifiedPath
  ====================*/
void    CSystem::AddModifiedPath(const tstring &sPath)
{
    m_setsModifiedFiles.insert(sPath);
}


/*====================
  CSystem::GetHiddenFileList
  ====================*/
void    CSystem::GetHiddenFileList(const tstring &sPath, const tstring &sFile, bool bRecurse, tsvector &vFileList, const tstring &sMod)
{
    WIN32_FIND_DATA     finddata;
    HANDLE              handle;

    tstring sFullPath = FileManager.GetSystemPath(sPath + _T("/"), sMod);
    if (sFullPath.empty())
        return;
    tstring sSearch(sFullPath + sFile);

    //first search for files only
    handle = FindFirstFile(sSearch.c_str(), &finddata);
    if (handle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((finddata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) &&
                !(finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                vFileList.push_back(FileManager.SanitizePath(sPath + _T("/") + tstring(finddata.cFileName)));
            }

        }
        while (FindNextFile(handle, &finddata));

        FindClose(handle);
    }

    if (!bRecurse)
        return;

    // next search for directories only, and list their contents if we were asked to recurse
    sSearch = sFullPath + _T("*");

    handle = FindFirstFile(sSearch.c_str(), &finddata);
    if (handle == INVALID_HANDLE_VALUE)
        return;

    do
    {
        if ((finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            if (_tcscmp(finddata.cFileName, _T(".")) &&
                _tcscmp(finddata.cFileName, _T("..")))
            {
                tstring sDirectory(sPath + _T("/") + finddata.cFileName + _T("/"));
                GetHiddenFileList(sDirectory, sFile, bRecurse, vFileList, sMod);
            }
        }
    }
    while (FindNextFile(handle, &finddata));

    FindClose(handle);
}


/*====================
  CSystem::GetFileList
  ====================*/
void    CSystem::GetFileList(const tstring &sPath, const tstring &sFile, bool bRecurse, tsvector &vFileList, const tstring &sMod)
{
    WIN32_FIND_DATA     finddata;
    HANDLE              handle;

    tstring sFullPath = FileManager.GetSystemPath(sPath + _T("/"), sMod);
    if (sFullPath.empty())
        return;
    tstring sSearch(sFullPath + sFile);

    //first search for files only
    handle = FindFirstFile(sSearch.c_str(), &finddata);
    if (handle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(finddata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) &&
                !(finddata.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) &&
                !(finddata.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) &&
                !(finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                vFileList.push_back(FileManager.SanitizePath(sPath + _T("/") + tstring(finddata.cFileName)));
            }

        }
        while (FindNextFile(handle, &finddata));

        FindClose(handle);
    }

    if (!bRecurse)
        return;

    // next search for directories only, and list their contents if we were asked to recurse
    sSearch = sFullPath + _T("*");

    handle = FindFirstFile(sSearch.c_str(), &finddata);
    if (handle == INVALID_HANDLE_VALUE)
        return;

    do
    {
        if (!(finddata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) &&
            !(finddata.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) &&
            !(finddata.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) &&
            (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            if (_tcscmp(finddata.cFileName, _T(".")) &&
                _tcscmp(finddata.cFileName, _T("..")) &&
                _tcscmp(finddata.cFileName, _T("CVS")))
            {
                tstring sDirectory(sPath + _T("/") + finddata.cFileName + _T("/"));

                GetFileList(sDirectory, sFile, bRecurse, vFileList, sMod);
            }
        }
    }
    while (FindNextFile(handle, &finddata));

    FindClose(handle);
}


/*====================
  CSystem::GetDirList
  ====================*/
void    CSystem::GetDirList(const tstring &sPath, bool bRecurse, tsvector &vDirList, const tstring &sMod)
{
    WIN32_FIND_DATA     finddata;
    HANDLE              handle;

    tstring sFullPath = FileManager.GetSystemPath(sPath + _T("/"), sMod);

    if (sFullPath.empty())
        return;

    tstring sSearch(sFullPath + _T("*"));

    handle = FindFirstFile(sSearch.c_str(), &finddata);
    if (handle == INVALID_HANDLE_VALUE)
        return;

    do
    {
        if (!(finddata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) &&
            !(finddata.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) &&
            !(finddata.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) &&
            (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            if (_tcscmp(finddata.cFileName, _T(".")) &&
                _tcscmp(finddata.cFileName, _T("..")) &&
                _tcscmp(finddata.cFileName, _T("CVS")))
            {
                vDirList.push_back(FileManager.SanitizePath(sPath + _T("/") + finddata.cFileName));

                if (bRecurse)
                {
                    tstring sDirectory(sPath + _T("/") + finddata.cFileName + _T("/"));
                    GetDirList(sDirectory, bRecurse, vDirList, sMod);
                }
            }
        }
    }
    while (FindNextFile(handle, &finddata));

    FindClose(handle);
}


/*====================
  CSystem::Error
  ====================*/
void    CSystem::Error(const tstring &sMsg)
{
    if (sys_dumpOnFatal)
    {
        RaiseException(0xE0000001, 0, 0, 0);
    }
    else
    {
        Vid.Shutdown();
        MessageBox(nullptr, sMsg.c_str(), _T("K2 - Fatal Error"), MB_OK);
    }

    Exit(-1);
}


/*====================
  CSystem::Milliseconds
  ====================*/
uint    CSystem::Milliseconds()
{
#if 0
    timeBeginPeriod(1);
    uint uiTime(timeGetTime());
    timeEndPeriod(1);
    return uiTime;
#else
    return uint(((GetTicks() - m_llStartTicks)) / (GetFrequency() / 1000));
#endif
}


/*====================
  CSystem::Microseconds
  ====================*/
uint    CSystem::Microseconds()
{
    return uint(((GetTicks() - m_llStartTicks)) / (GetFrequency() / 1000000));
}


/*====================
  CSystem::GetTicks
  ====================*/
LONGLONG    CSystem::GetTicks() const
{
    LARGE_INTEGER ll;
    QueryPerformanceCounter(&ll);
    return ll.QuadPart;
}


/*====================
  CSystem::GetFrequency
  ====================*/
LONGLONG    CSystem::GetFrequency()
{
    LARGE_INTEGER ll;
    QueryPerformanceFrequency(&ll);
    return ll.QuadPart;
}


/*====================
  CSystem::LoadLibrary
  ====================*/
#pragma push_macro("LoadLibrary")
#undef LoadLibrary
void*   CSystem::LoadLibrary(const tstring &sLibFilename)
#pragma pop_macro("LoadLibrary")
{
    PROFILE("CSystem::LoadLibrary");

    tstring sFullPah(FileManager.GetSystemPath(sLibFilename + _T(".dll")));

    _tchdir(Filename_GetPath(sFullPah).c_str());
    void* pLib(::LoadLibrary(sFullPah.c_str()));
    _tchdir(m_sRootDir.c_str());

    if (pLib == nullptr)
        Console.Err << _T("CSystem::LoadLibrary() - ") << GetLastErrorString() << newl;
    return pLib;
}


/*====================
  CSystem::GetProcAddress
  ====================*/
void*   CSystem::GetProcAddress(void *pDll, const tstring &sProcName)
{
    char szProcName[1024];
    MemManager.Set(szProcName, 0, 1024);
    TCHARToSingle(szProcName, 1024, sProcName.c_str(), 1023);

    return ::GetProcAddress(static_cast<HMODULE>(pDll), szProcName);
}


/*====================
  CSystem::FreeLibrary
  ====================*/
bool    CSystem::FreeLibrary(void *pDll)
{
    if (pDll == nullptr)
        return true;

    return ::FreeLibrary(static_cast<HMODULE>(pDll)) != FALSE;
}


/*====================
  CSystem::GetWindowArea
  ====================*/
CRecti  CSystem::GetWindowArea()
{
    RECT full, client;
    int clientw, clienth;
    int style;

    if (Vid.IsFullScreen())
        style = WS_SYSMENU | WS_VISIBLE;
    else
        style = WS_CAPTION | WS_MINIMIZEBOX | WS_VISIBLE | WS_SYSMENU;

    GetClientRect((HWND)m_WindowHandle, &client);
    clientw = client.right;
    clienth = client.bottom;
    AdjustWindowRect(&client, style, false);
    GetWindowRect((HWND)m_WindowHandle, &full);

    return CRecti(full.left - client.left,
                full.top - client.top,
                full.left - client.left + clientw,
                full.top - client.top + clienth);
}


/*====================
  CSystem::SetMousePos
  ====================*/
void    CSystem::SetMousePos(int x, int y)
{
    CRecti winrec = GetWindowArea();
    ::SetCursorPos(winrec.left + x, winrec.top + y);
}


/*====================
  CSystem::GetMousePos
  ====================*/
CVec2i  CSystem::GetMousePos()
{
    CRecti winrec = GetWindowArea();
    POINT   p;
    ::GetCursorPos(&p);
    return CVec2i(p.x - winrec.left, p.y - winrec.top);
}


/*====================
  CSystem::ShowMouseCursor
  ====================*/
void    CSystem::ShowMouseCursor()
{
    while(::ShowCursor(true) < 0);
}


/*====================
  CSystem::HideMouseCursor
  ====================*/
void    CSystem::HideMouseCursor()
{
    while(::ShowCursor(false) >= 0);
}


/*====================
  CSystem::SetMouseClipping
  ====================*/
void    CSystem::SetMouseClipping(const CRecti &recClip)
{
    if (!m_bHasFocus)
        return;

    CRecti winrec(GetWindowArea());
    RECT    rec = { winrec.left + recClip.left, winrec.top + recClip.top, winrec.left + recClip.right, winrec.top + recClip.bottom };
    ::ClipCursor(&rec);
}


/*====================
  CSystem::UnsetMouseClipping
  ====================*/
void    CSystem::UnsetMouseClipping()
{
    ::ClipCursor(nullptr);
}


/*====================
  CSystem::PollMouse
  ====================*/
void    CSystem::PollMouse()
{
    CVec2i v2Point(GetMousePos());

    if (v2Point.x != INT_ROUND(Input.GetAxisState(AXIS_MOUSE_X)))
        Input.AddEvent(AXIS_MOUSE_X, float(v2Point.x), v2Point.x - Input.GetAxisState(AXIS_MOUSE_X));
    
    if (v2Point.y != INT_ROUND(Input.GetAxisState(AXIS_MOUSE_Y)))
        Input.AddEvent(AXIS_MOUSE_Y, float(v2Point.y), v2Point.y - Input.GetAxisState(AXIS_MOUSE_Y));
}


/*====================
  CSystem::PollJoysticks
  ====================*/
void    CSystem::PollJoysticks(uint uiID)
{
    // Check for a valid device
    JOYINFO joystickInfo;
    MemManager.Set(&joystickInfo, 0, sizeof(JOYINFO));
    if (joyGetPos(uiID, &joystickInfo) != JOYERR_NOERROR)
        return;

    // Analog axis
    for (uint uiAxis(0); uiAxis < NUM_SYS_JOYSTICK_AXIS; ++uiAxis)
    {
        if (!JoystickHasAxis(uiID, EAxis(AXIS_JOY_X + uiAxis)))
            continue;

        float fAxisValue(GetJoystickAxis(uiID, EAxis(AXIS_JOY_X + uiAxis)));
        float fAxisDelta(2.0f * (fAxisValue - 0.5f));
        Input.AddEvent(EAxis(AXIS_JOY_X + uiAxis), fAxisValue, fAxisDelta);

        m_JoystickState.fAxis[uiAxis] = fAxisDelta;
    }

    // Buttons
    uint uiButtonStates(GetJoystickButtons(uiID));
    for (uint uiButton(0); uiButton < NUM_JOYSTICK_BUTTONS; ++uiButton)
    {
        if ((uiButtonStates & BIT(uiButton)) != (m_JoystickState.uiButtons & BIT(uiButton)))
            Input.AddEvent(EButton(BUTTON_JOY1 + uiButton), (uiButtonStates & BIT(uiButton)) != 0);
    }
    m_JoystickState.uiButtons = uiButtonStates;

    // POV
#define POV_UP(pov)     ((pov) != JOY_POVCENTERED && (((pov) >= JOY_POVFORWARD && (pov) < JOY_POVRIGHT) || (pov) > JOY_POVLEFT))
#define POV_RIGHT(pov)  ((pov) != JOY_POVCENTERED && (pov) > JOY_POVFORWARD && (pov) < JOY_POVBACKWARD)
#define POV_DOWN(pov)   ((pov) != JOY_POVCENTERED && (pov) > JOY_POVRIGHT && (pov) < JOY_POVLEFT)
#define POV_LEFT(pov)   ((pov) != JOY_POVCENTERED && (pov) > JOY_POVBACKWARD)

    uint uiPOV(GetJoystickPOV(uiID));
    if (POV_UP(uiPOV) && !POV_UP(m_JoystickState.uiPOV))
        Input.AddEvent(BUTTON_JOY_POV_UP, true);
    else if (!POV_UP(uiPOV) && POV_UP(m_JoystickState.uiPOV))
        Input.AddEvent(BUTTON_JOY_POV_UP, false);
    if (POV_RIGHT(uiPOV) && !POV_RIGHT(m_JoystickState.uiPOV))
        Input.AddEvent(BUTTON_JOY_POV_RIGHT, true);
    else if (!POV_RIGHT(uiPOV) && POV_RIGHT(m_JoystickState.uiPOV))
        Input.AddEvent(BUTTON_JOY_POV_RIGHT, false);
    if (POV_DOWN(uiPOV) && !POV_DOWN(m_JoystickState.uiPOV))
        Input.AddEvent(BUTTON_JOY_POV_DOWN, true);
    else if (!POV_DOWN(uiPOV) && POV_DOWN(m_JoystickState.uiPOV))
        Input.AddEvent(BUTTON_JOY_POV_DOWN, false);
    if (POV_LEFT(uiPOV) && !POV_LEFT(m_JoystickState.uiPOV))
        Input.AddEvent(BUTTON_JOY_POV_LEFT, true);
    else if (!POV_LEFT(uiPOV) && POV_LEFT(m_JoystickState.uiPOV))
        Input.AddEvent(BUTTON_JOY_POV_LEFT, false);
    m_JoystickState.uiPOV = uiPOV;

#undef POV_UP
#undef POV_RIGHT
#undef POV_DOWN
#undef POV_LEFT
}


/*====================
  CSystem::GetJoystickAxis
  ====================*/
float   CSystem::GetJoystickAxis(uint uiID, EAxis eAxis)
{
    try
    {
        JOYINFOEX   joystickInfo;
        MemManager.Set(&joystickInfo, 0, sizeof(JOYINFOEX));
        joystickInfo.dwSize = sizeof(JOYINFOEX);
        joystickInfo.dwFlags = JOY_RETURNALL;
        switch (joyGetPosEx(uiID, &joystickInfo))
        {
        case MMSYSERR_NODRIVER: EX_ERROR(_T("Joystick driver is not present"));
        case JOYERR_PARMS:
        case MMSYSERR_INVALPARAM: EX_ERROR(_T("Invalid parameter to joyGetPosEx()"));
        case MMSYSERR_BADDEVICEID: EX_ERROR(_T("Invalid joystick ID"));
        case JOYERR_UNPLUGGED: EX_WARN(_T("No joystick plugged in"));
        default: break;
        }

        switch (eAxis)
        {
        case AXIS_JOY_X: return joystickInfo.dwXpos / float(USHRT_MAX);
        case AXIS_JOY_Y: return joystickInfo.dwYpos / float(USHRT_MAX);
        case AXIS_JOY_Z: return joystickInfo.dwZpos / float(USHRT_MAX);
        case AXIS_JOY_R: return joystickInfo.dwRpos / float(USHRT_MAX);
        case AXIS_JOY_U: return joystickInfo.dwUpos / float(USHRT_MAX);
        case AXIS_JOY_V: return joystickInfo.dwVpos / float(USHRT_MAX);
        default: EX_WARN(_T("Invalid axis specified"));
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CSystem::GetJoystickAxis() - "), NO_THROW);
        return 0.0f;
    }
}


/*====================
  CSystem::GetJoystickButtons
  ====================*/
uint    CSystem::GetJoystickButtons(uint uiID)
{
    try
    {
        JOYINFOEX   joystickInfo;
        MemManager.Set(&joystickInfo, 0, sizeof(JOYINFOEX));
        joystickInfo.dwSize = sizeof(JOYINFOEX);
        joystickInfo.dwFlags = JOY_RETURNALL;
        switch (joyGetPosEx(uiID, &joystickInfo))
        {
        case MMSYSERR_NODRIVER: EX_ERROR(_T("Joystick driver is not present"));
        case JOYERR_PARMS:
        case MMSYSERR_INVALPARAM: EX_ERROR(_T("Invalid parameter to joyGetPosEx()"));
        case MMSYSERR_BADDEVICEID: EX_ERROR(_T("Invalid joystick ID"));
        case JOYERR_UNPLUGGED: EX_WARN(_T("No joystick plugged in"));
        default: break;
        }

        return joystickInfo.dwButtons;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CSystem::GetJoystickButton() - "), NO_THROW);
        return 0;
    }

}


/*====================
  CSystem::GetJoystickPOV
  ====================*/
uint    CSystem::GetJoystickPOV(uint uiID)
{
    try
    {
        JOYINFOEX   joystickInfo;
        MemManager.Set(&joystickInfo, 0, sizeof(JOYINFOEX));
        joystickInfo.dwSize = sizeof(JOYINFOEX);
        joystickInfo.dwFlags = JOY_RETURNPOVCTS;
        switch (joyGetPosEx(uiID, &joystickInfo))
        {
        case MMSYSERR_NODRIVER: EX_ERROR(_T("Joystick driver is not present"));
        case JOYERR_PARMS:
        case MMSYSERR_INVALPARAM: EX_ERROR(_T("Invalid parameter to joyGetPosEx()"));
        case MMSYSERR_BADDEVICEID: EX_ERROR(_T("Invalid joystick ID"));
        case JOYERR_UNPLUGGED: EX_WARN(_T("No joystick plugged in"));
        default: break;
        }

        return joystickInfo.dwPOV;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CSystem::GetJoystickButton() - "), NO_THROW);
        return 0;
    }

}


/*====================
  CSystem::JoystickHasAxis
  ====================*/
bool    CSystem::JoystickHasAxis(uint uiID, EAxis axis)
{
    try
    {
        JOYCAPS joycaps;
        MemManager.Set(&joycaps, 0, sizeof(JOYCAPS));
        switch (joyGetDevCaps(uiID, &joycaps, sizeof(JOYCAPS)))
        {
        case MMSYSERR_NODRIVER: EX_ERROR(_T("Joystick driver is not present"));
        case MMSYSERR_INVALPARAM: EX_ERROR(_T("Invalid parameter to joyGetPosEx()"));
        case JOYERR_NOERROR: break;
        default: break;
        }

        switch (axis)
        {
        case AXIS_JOY_X: return (joycaps.wNumAxes > 0);
        case AXIS_JOY_Y: return (joycaps.wNumAxes > 1);
        case AXIS_JOY_Z: return (joycaps.wCaps & JOYCAPS_HASZ) != 0;
        case AXIS_JOY_R: return (joycaps.wCaps & JOYCAPS_HASR) != 0;
        case AXIS_JOY_U: return (joycaps.wCaps & JOYCAPS_HASU) != 0;
        case AXIS_JOY_V: return (joycaps.wCaps & JOYCAPS_HASV) != 0;
        default: return false;
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CSystem::JoystickHasAxis() - "), NO_THROW);
        return false;
    }
}


/*====================
  CSystem::GetJoystickList
  ====================*/
void    CSystem::GetJoystickList(imaps &mapNames)
{
    uint uiNumDevices(joyGetNumDevs());
    for (uint uiDevice(0); uiDevice < uiNumDevices; ++uiDevice)
    {
        JOYINFO joyinfo;
        MemManager.Set(&joyinfo, 0, sizeof(JOYINFO));
        if (joyGetPos(uiDevice, &joyinfo) != JOYERR_NOERROR)
            continue;

        JOYCAPS joycaps;
        MemManager.Set(&joycaps, 0, sizeof(JOYCAPS));
        switch (joyGetDevCaps(uiDevice, &joycaps, sizeof(JOYCAPS)))
        {
        case MMSYSERR_NODRIVER:
            Console.Err << _T("Joystick driver is not present for device #") << uiDevice << newl;
            break;

        case MMSYSERR_INVALPARAM:
            Console.Err << _T("Invalid parameter to joyGetPosEx() for device #") << uiDevice << newl;
            break;

        case JOYERR_NOERROR:
            mapNames[uiDevice] = joycaps.szPname;

        default:
            break;
        }
    }
}


/*====================
  CSystem::CopyToClipboard
  ====================*/
void    CSystem::CopyToClipboard(const tstring &sText)
{
    HGLOBAL hGlobal(::GlobalAlloc(GHND | GMEM_SHARE, (sText.length() + 1) * sizeof(TCHAR)));
    
    TCHAR *pGlobal(static_cast<TCHAR*>(::GlobalLock(hGlobal)));

    for (tstring::const_iterator it(sText.begin()); it != sText.end(); ++it)
        *pGlobal++ = *it;

    ::GlobalUnlock(hGlobal);

    ::OpenClipboard(static_cast<HWND>(m_WindowHandle));

    EmptyClipboard();

#ifdef UNICODE
    ::SetClipboardData(CF_UNICODETEXT, hGlobal);
#else
    ::SetClipboardData(CF_TEXT, hGlobal);
#endif

    ::CloseClipboard();
}


/*====================
  CSystem::IsClipboardString
  ====================*/
bool    CSystem::IsClipboardString()
{
#ifdef UNICODE
    return ::IsClipboardFormatAvailable(CF_UNICODETEXT) != FALSE;
#else
    return ::IsClipboardFormatAvailable(CF_TEXT) != FALSE;
#endif
}


/*====================
  CSystem::GetClipboardString
  ====================*/
tstring CSystem::GetClipboardString()
{
    ::OpenClipboard(static_cast<HWND>(m_WindowHandle));

#ifdef UNICODE
    HGLOBAL hGlobal(::GetClipboardData(CF_UNICODETEXT));
#else
    HGLOBAL hGlobal(::GetClipboardData(CF_TEXT));
#endif
    if (!hGlobal)
        return TSNULL;

    tstring sReturn;
    TCHAR *pGlobal(static_cast<TCHAR*>(::GlobalLock(hGlobal)));
    while (pGlobal && *pGlobal)
        sReturn += *pGlobal++;

    ::GlobalUnlock(hGlobal);
    ::CloseClipboard();

    return sReturn;
}


/*====================
  CSystem::Beep
  ====================*/
void    CSystem::Beep(dword dwFreq, dword dwDuration)
{
    ::Beep(dwFreq, dwDuration);
}


/*====================
  CSystem::IsDebuggerPresent
  ====================*/
bool    CSystem::IsDebuggerPresent()
{
    return ::IsDebuggerPresent() == TRUE;
}


/*====================
  CSystem::GetClientOffset
  ====================*/
void    CSystem::GetClientOffset(int *x, int *y)
{
    RECT adj;
    int style;

    if (Vid.IsFullScreen())
        style = WS_SYSMENU | WS_VISIBLE;
    else
        style = WS_CAPTION | WS_MINIMIZEBOX | WS_VISIBLE | WS_SYSMENU;

    ::GetClientRect(reinterpret_cast<HWND>(Vid.GetHWnd()), &adj);
    ::AdjustWindowRect(&adj, style, false);

    *x = -adj.left;
    *y = -adj.top;
}


/*====================
  CSystem::GetProcessID
  ====================*/
uint    CSystem::GetProcessID()
{
    return GetCurrentProcessId();
}


/*====================
  CSystem::GetRunningProcesses

  Returns a list of all active processes.
  ====================*/
uiset   CSystem::GetRunningProcesses()
{
    DWORD dwProcesses[2048];
    DWORD dwBytesWritten;
    uint uiNumProcs;
    uiset setProcesses;

    tstring sProcessName(GetProcessBaseName());

    if (!EnumProcesses(dwProcesses, sizeof(DWORD) * 2048, &dwBytesWritten))
        return setProcesses;

    // Calculate how many process identifiers were returned.
    uiNumProcs = dwBytesWritten / sizeof(DWORD);

    for (uint i = 0; i < uiNumProcs; i++)
    {
        if (dwProcesses[i] != 0)
        {
            TCHAR szProcessName[MAX_PATH];

            szProcessName[0] = 0;

            // Get a handle to the process.
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, dwProcesses[i]);

            // Get the process name.
            if (hProcess != nullptr)
            {
                HMODULE hMod;
                DWORD dwTotalModules;

                if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &dwTotalModules))
                    GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));

                CloseHandle(hProcess);
            }

            if (_tcscmp(szProcessName, sProcessName.c_str()) == 0)
                setProcesses.insert(dwProcesses[i]);
        }
    }

    return setProcesses;
}


#if 0
/*====================
  CSystem::AddDedicatedConsoleText
  ====================*/
void    CSystem::AddDedicatedConsoleText(const tstring &sText)
{
    m_sDedicatedBuffer += sText;
}


/*====================
  CSystem::UpdateDedicatedConsoleText
  ====================*/
void    CSystem::UpdateDedicatedConsoleText()
{
    HWND hConsoleText(GetDlgItem((HWND)GetConsoleWindowHandle(), IDC_OUTPUT));

    SendMessage(hConsoleText, EM_SETSEL, 0, -1);
    SendMessage(hConsoleText, EM_SETSEL, -1, -1);
    SendMessage(hConsoleText, EM_REPLACESEL, FALSE, (LONG_PTR)(m_sDedicatedBuffer.c_str()));

    m_sDedicatedBuffer.clear();
}
#else
/*====================
  CSystem::AddDedicatedConsoleText
  ====================*/
void    CSystem::AddDedicatedConsoleText(const tstring &sText)
{
    DWORD dwNumberOfCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo; 
    GetConsoleScreenBufferInfo((HANDLE)m_ConsoleWindowHandle, &csbiInfo);

    COORD coord;
    coord.X = 0;
    coord.Y = csbiInfo.dwCursorPosition.Y;

    if (!m_sConsoleInputLine.empty())
    {
        FillConsoleOutputCharacter((HANDLE)m_ConsoleWindowHandle, ' ', csbiInfo.dwSize.X, coord, &dwNumberOfCharsWritten);
        m_sConsoleInputLine.clear();
    }

    SetConsoleCursorPosition((HANDLE)m_ConsoleWindowHandle, coord);
    
    WriteConsole((HANDLE)m_ConsoleWindowHandle, sText.c_str(), DWORD(sText.length()), &dwNumberOfCharsWritten, nullptr);
}


/*====================
  CSystem::UpdateDedicatedConsoleText
  ====================*/
void    CSystem::UpdateDedicatedConsoleText()
{
    DWORD dwNumberOfCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo; 
    GetConsoleScreenBufferInfo((HANDLE)m_ConsoleWindowHandle, &csbiInfo);

    tstring sInputLine(_T(">") + Console.GetInputLine());
    uint uiInputPos(Console.GetInputPos() + 1);

    if (sInputLine != m_sConsoleInputLine)
    {
        COORD coord;
        coord.X = 0;
        coord.Y = csbiInfo.dwCursorPosition.Y;

        FillConsoleOutputCharacter((HANDLE)m_ConsoleWindowHandle, ' ', csbiInfo.dwSize.X, coord, &dwNumberOfCharsWritten);

        SetConsoleCursorPosition((HANDLE)m_ConsoleWindowHandle, coord);

        WriteConsole((HANDLE)m_ConsoleWindowHandle, sInputLine.c_str(), DWORD(sInputLine.length()), &dwNumberOfCharsWritten, nullptr);

        m_sConsoleInputLine = sInputLine;
    }

    if (csbiInfo.dwCursorPosition.X != uiInputPos)
    {
        COORD coord;
        coord.X = uiInputPos;
        coord.Y = csbiInfo.dwCursorPosition.Y;

        SetConsoleCursorPosition((HANDLE)m_ConsoleWindowHandle, coord);
    }
}
#endif


/*====================
  CSystem::GetSystemInfo
  ====================*/
SSysInfo    CSystem::GetSystemInfo()
{
    HRESULT hres;
    SSysInfo structInfo;
    
#define GET_SYS_STR(x) StripDuplicateSpaces(StripStartingSpaces(WStringToTString(x)))

    try
    {
        IWbemLocator *pLoc = nullptr;

        hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)&pLoc);
     
        if (hres < 0)
            EX_ERROR(_T("Failed to initialize IWbemLocator object, error code ") + INT_HEX_TSTR(hres));

        IWbemServices *pSvc = nullptr;
        
        hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &pSvc);
        
        if (hres < 0)
        {
            pLoc->Release();
            EX_ERROR(_T("Failed to initialize IWbemServices object, error code ") + INT_HEX_TSTR(hres));
        }

        hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);

        if (hres < 0)
        {
            pSvc->Release();
            pLoc->Release();
            EX_ERROR(_T("Failed to set proxy blanket, error code ") + INT_HEX_TSTR(hres));
        }

        IEnumWbemClassObject* pEnumerator = nullptr;
        IWbemClassObject *pclsObj = nullptr;
        ULONG uReturn = 0;

        // Grab the name and version of the operating system
        hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT Name, Version FROM Win32_OperatingSystem"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator);
        
        if (hres < 0)
        {
            pSvc->Release();
            pLoc->Release();
            EX_ERROR(_T("Query for operating system name failed, error code ") + INT_HEX_TSTR(hres));
        }

        while (pEnumerator)
        {
            HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

            if (uReturn == 0)
                break;

            VARIANT vtProp;

            hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);

            tstring sOS(GET_SYS_STR(vtProp.bstrVal));
            size_t zFind = sOS.find_first_of(_T("|"));

            if (zFind != tstring::npos)
                sOS = sOS.substr(0, zFind);

            if (structInfo.sOS.empty())
                structInfo.sOS = sOS;
            else
                structInfo.sOS = structInfo.sOS + _T("; ") + sOS;

            VariantClear(&vtProp);

            hr = pclsObj->Get(L"Version", 0, &vtProp, 0, 0);
            structInfo.sOS = structInfo.sOS + _T(", Version ") + GET_SYS_STR(vtProp.bstrVal);
            VariantClear(&vtProp);

            pclsObj->Release();
        }

        pEnumerator->Release();

        // Grab the total physical memory from the computer system info
        hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT TotalPhysicalMemory FROM Win32_ComputerSystem"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator);
        
        if (hres < 0)
        {
            pSvc->Release();
            pLoc->Release();
            EX_ERROR(_T("Query for computer system info failed, error code ") + INT_HEX_TSTR(hres));
        }
  
        while (pEnumerator)
        {
            HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

            if (uReturn == 0)
                break;

            VARIANT vtProp;

            hr = pclsObj->Get(L"TotalPhysicalMemory", 0, &vtProp, 0, 0);

            if (structInfo.sRAM.empty())
                structInfo.sRAM = GetByteString((uint)AtoI(GET_SYS_STR(vtProp.bstrVal)));
            else
                structInfo.sRAM = structInfo.sRAM + _T("; ") + GetByteString((uint)AtoI(GET_SYS_STR(vtProp.bstrVal)));

            VariantClear(&vtProp);

            pclsObj->Release();
        }

        pEnumerator->Release();

        // Grab video card availability, name and RAM
        hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT Availability, Name, AdapterRAM FROM Win32_VideoController"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator);
        
        if (hres < 0)
        {
            pSvc->Release();
            pLoc->Release();
            EX_ERROR(_T("Query for video card info failed, error code ") + INT_HEX_TSTR(hres));
        }
 
        while (pEnumerator)
        {
            HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

            if (uReturn == 0)
                break;

            VARIANT vtProp;

            hr = pclsObj->Get(L"Availability", 0, &vtProp, 0, 0);

            if (vtProp.uintVal != 3) // Not in use or not available
            {
                VariantClear(&vtProp);
                pclsObj->Release();
                continue;
            }

            VariantClear(&vtProp);

            hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);

            if (structInfo.sVideo.empty())
                structInfo.sVideo = GET_SYS_STR(vtProp.bstrVal);
            else
                structInfo.sVideo = structInfo.sVideo + _T("; ") + GET_SYS_STR(vtProp.bstrVal);

            VariantClear(&vtProp);

            hr = pclsObj->Get(L"AdapterRAM", 0, &vtProp, 0, 0);
            structInfo.sVideo = structInfo.sVideo + _T("(") + GetByteString(vtProp.uintVal) + _T(")");
            VariantClear(&vtProp);

            pclsObj->Release();
        }

        pEnumerator->Release();

        // Get network adapter connection status and MAC address
        IP_ADAPTER_INFO *pAdapterInfo(nullptr);
        ULONG ulSize(0);
        if (GetAdaptersInfo(pAdapterInfo, &ulSize) == ERROR_BUFFER_OVERFLOW)
        {
            pAdapterInfo = reinterpret_cast<IP_ADAPTER_INFO*>(K2_NEW_ARRAY(ctx_System, byte, ulSize));
        }

        if (pAdapterInfo != nullptr && GetAdaptersInfo(pAdapterInfo, &ulSize) == ERROR_SUCCESS)
        {
            IP_ADAPTER_INFO *pCurrentAdapter(pAdapterInfo);
            while (pCurrentAdapter != nullptr)
            {
                structInfo.sMAC.clear();
                for (uint ui(0); ui < pCurrentAdapter->AddressLength; ++ui)
                    structInfo.sMAC += ((ui > 0) ? _T("-") : TSNULL) + XtoA(pCurrentAdapter->Address[ui], FMT_PADZERO | FMT_NOPREFIX, 2, 16);
                pCurrentAdapter = pCurrentAdapter->Next;
                
                // FIXME: Handle multiple NIC info?
                break;
            }
        }

        SAFE_DELETE_ARRAY(pAdapterInfo)

        // Get processor availability, status, type and speed
        hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT Availability, CpuStatus, StatusInfo, ProcessorType, CurrentClockSpeed, Name FROM Win32_Processor"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator);
        
        if (hres < 0)
        {
            pSvc->Release();
            pLoc->Release();
            EX_ERROR(_T("Query for processor info failed, error code ") + INT_HEX_TSTR(hres));
        }

        while (pEnumerator)
        {
            HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

            if (uReturn == 0)
                break;

            VARIANT vtProp;

            hr = pclsObj->Get(L"Availability", 0, &vtProp, 0, 0);

            if (vtProp.uintVal != 3) // CPU is not available for use
            {
                pclsObj->Release();
                VariantClear(&vtProp);
                continue;
            }

            VariantClear(&vtProp);

            hr = pclsObj->Get(L"CpuStatus", 0, &vtProp, 0, 0);

            if (vtProp.uintVal != 1) // CPU is not enabled
            {
                pclsObj->Release();
                VariantClear(&vtProp);
                continue;
            }

            VariantClear(&vtProp);

            hr = pclsObj->Get(L"StatusInfo", 0, &vtProp, 0, 0);

            if (vtProp.uintVal != 3) // CPU is not enabled
            {
                pclsObj->Release();
                VariantClear(&vtProp);
                continue;
            }

            VariantClear(&vtProp);

            hr = pclsObj->Get(L"ProcessorType", 0, &vtProp, 0, 0);

            if (vtProp.uintVal != 3) // We only want to pay attention to the central processor
            {
                pclsObj->Release();
                VariantClear(&vtProp);
                continue;
            }

            VariantClear(&vtProp);

            hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);

            if (structInfo.sProcessor.empty())
                structInfo.sProcessor = GET_SYS_STR(vtProp.bstrVal);
            else
                structInfo.sProcessor = structInfo.sProcessor + _T("; ") + GET_SYS_STR(vtProp.bstrVal);

            VariantClear(&vtProp);

            hr = pclsObj->Get(L"CurrentClockSpeed", 0, &vtProp, 0, 0);
            structInfo.sProcessor = structInfo.sProcessor + _T(" (") + XtoA(vtProp.uintVal) + _T(" MHz)");
            VariantClear(&vtProp);

            pclsObj->Release();

            break;
        }

        pEnumerator->Release();
        
        pSvc->Release();
        pLoc->Release();

        return structInfo;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CSystem::GetSystemInfo() - "), NO_THROW);
        return structInfo;
    }

#undef GET_SYS_STR
}


/*====================
  CSystem::GetTotalPhysicalMemory
  ====================*/
ULONGLONG   CSystem::GetTotalPhysicalMemory() const
{
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(MEMORYSTATUSEX);
    ::GlobalMemoryStatusEx(&status);
    return status.ullTotalPhys;
}


/*====================
  CSystem::GetFreePhysicalMemory
  ====================*/
ULONGLONG   CSystem::GetFreePhysicalMemory() const
{
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(MEMORYSTATUSEX);
    ::GlobalMemoryStatusEx(&status);
    return status.ullAvailPhys;
}


/*====================
  CSystem::GetTotalVirtualMemory
  ====================*/
ULONGLONG   CSystem::GetTotalVirtualMemory() const
{
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(MEMORYSTATUSEX);
    ::GlobalMemoryStatusEx(&status);
    return status.ullTotalVirtual;
}


/*====================
  CSystem::GetFreeVirtualMemory
  ====================*/
ULONGLONG   CSystem::GetFreeVirtualMemory() const
{
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(MEMORYSTATUSEX);
    ::GlobalMemoryStatusEx(&status);
    return status.ullAvailVirtual;
}


/*====================
  CSystem::GetTotalPageFile
  ====================*/
ULONGLONG   CSystem::GetTotalPageFile() const
{
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(MEMORYSTATUSEX);
    ::GlobalMemoryStatusEx(&status);
    return status.ullTotalPageFile;
}


/*====================
  CSystem::GetFreePageFile
  ====================*/
ULONGLONG   CSystem::GetFreePageFile() const
{
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(MEMORYSTATUSEX);
    ::GlobalMemoryStatusEx(&status);
    return status.ullAvailPageFile;
}


/*====================
  CSystem::GetProcessMemoryUsage
  ====================*/
ULONGLONG   CSystem::GetProcessMemoryUsage() const
{
    PROCESS_MEMORY_COUNTERS procMemory;
    ::GetProcessMemoryInfo(GetCurrentProcess(), &procMemory, sizeof(PROCESS_MEMORY_COUNTERS));
    return procMemory.WorkingSetSize;
}


/*====================
  CSystem::GetProcessVirtualMemoryUsage
  ====================*/
ULONGLONG   CSystem::GetProcessVirtualMemoryUsage() const
{
    PROCESS_MEMORY_COUNTERS procMemory;
    ::GetProcessMemoryInfo(GetCurrentProcess(), &procMemory, sizeof(PROCESS_MEMORY_COUNTERS));
    return procMemory.PagefileUsage;
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
void    CSystem::GetDriveList(tsvector &vNames) const
{
    static TCHAR szBuffer[2048];
    uint uiLength(::GetLogicalDriveStrings(2048, szBuffer));

    uint uiCharIndex(0);
    while (uiCharIndex < uiLength && szBuffer[uiCharIndex] != 0)
    {
        tstring sName;
        while (uiCharIndex < uiLength && szBuffer[uiCharIndex] != 0)
        {
            sName += szBuffer[uiCharIndex];
            ++uiCharIndex;
        }

        vNames.push_back(sName);
        sName.clear();
        ++uiCharIndex;
    }
}


/*====================
  CSystem::GetDriveType
  ====================*/
#pragma push_macro("GetDriveType")
#undef GetDriveType
EDriveType  CSystem::GetDriveType(const tstring &sName) const
#pragma pop_macro("GetDriveType")
{
    switch (::GetDriveType(TStringToWString(sName).c_str()))
    {
    case DRIVE_REMOVABLE:
        return DRIVETYPE_REMOVABLE;

    case DRIVE_FIXED:
        return DRIVETYPE_FIXED;

    case DRIVE_REMOTE:
        return DRIVETYPE_REMOTE;

    case DRIVE_CDROM:
        return DRIVETYPE_CDROM;

    case DRIVE_RAMDISK:
        return DRIVETYPE_RAM;

    default:
    case DRIVE_UNKNOWN:
    case DRIVE_NO_ROOT_DIR:
        return DRIVETYPE_INVALID;
    }
}


/*====================
  CSystem::GetDriveSize
  ====================*/
size_t  CSystem::GetDriveSize(const tstring &sName) const
{
    DWORD uiSectorsPerCluster(0);
    DWORD uiBytesPerSector(0);
    DWORD uiFreeClusters(0);
    DWORD uiTotalClusters(0);

    if (!::GetDiskFreeSpace(TStringToWString(sName).c_str(), &uiSectorsPerCluster, &uiBytesPerSector, &uiFreeClusters, &uiTotalClusters))
        return 0;

    return uiSectorsPerCluster * uiBytesPerSector * uiTotalClusters;
}


/*====================
  CSystem::GetDriveFreeSpace
  ====================*/
size_t  CSystem::GetDriveFreeSpace(const tstring &sName) const
{
    DWORD uiSectorsPerCluster(0);
    DWORD uiBytesPerSector(0);
    DWORD uiFreeClusters(0);
    DWORD uiTotalClusters(0);

    if (!::GetDiskFreeSpace(TStringToWString(sName).c_str(), &uiSectorsPerCluster, &uiBytesPerSector, &uiFreeClusters, &uiTotalClusters))
        return 0;

    return uiSectorsPerCluster * uiBytesPerSector * uiFreeClusters;
}


/*====================
  CSystem::GetDriveSizeEx
  ====================*/
ULONGLONG   CSystem::GetDriveSizeEx(const tstring &sName) const
{
    ULARGE_INTEGER uliFreeSpace;
    ULARGE_INTEGER uliTotalSpace;
    ULARGE_INTEGER uliTotalFreeSpace;

    if (!::GetDiskFreeSpaceEx(TStringToWString(sName).c_str(), &uliFreeSpace, &uliTotalSpace, &uliTotalFreeSpace))
        return 0;
    
    return uliTotalSpace.QuadPart;
}


/*====================
  CSystem::GetDriveFreeSpaceEx
  ====================*/
ULONGLONG   CSystem::GetDriveFreeSpaceEx(const tstring &sName) const
{
    ULARGE_INTEGER uliFreeSpace;
    ULARGE_INTEGER uliTotalSpace;
    ULARGE_INTEGER uliTotalFreeSpace;

    if (!::GetDiskFreeSpaceEx(TStringToWString(sName).c_str(), &uliFreeSpace, &uliTotalSpace, &uliTotalFreeSpace))
        return 0;

    return uliTotalFreeSpace.QuadPart;
}


/*====================
  CSystem::GetCurrentThread
  ====================*/
size_t   CSystem::GetCurrentThread()
{
    return ::GetCurrentThreadId();
}


/*====================
  CSystem::SetConfig
  ====================*/
void    CSystem::SetConfig(const tstring &sConfig)
{
    TCHAR szDir[_MAX_PATH + 1];

    SHGetFolderPath(nullptr, CSIDL_PERSONAL, nullptr, SHGFP_TYPE_CURRENT, szDir);
    m_sUserDir = szDir;

    m_sUserDir += _T("/") + m_sGameName + _T("/");
    
    if (!sConfig.empty())
        m_sUserDir += sConfig + _T("/");

    if (_tmkdir(m_sUserDir.c_str()) != 0)
    {
        if (errno != EEXIST)
            Console.Err << _T("Failed to create user directory, settings will not be saved!") << newl;
    }
}


/*====================
  CSystem::SetAffinity
  ====================*/
void    CSystem::SetAffinity(int iCPU)
{
    DWORD dwProcessAffinityMask, dwSystemAffinityMask;
    if (::GetProcessAffinityMask(::GetCurrentProcess(), &dwProcessAffinityMask, &dwSystemAffinityMask))
    {
        if (iCPU < 0 || iCPU >= 32 || (BIT(iCPU) & dwSystemAffinityMask) == 0 || (BIT(iCPU) & dwProcessAffinityMask) == 0)
        {
            iCPU = 0;

            // Pick first available CPU
            for (; iCPU < 32; ++iCPU)
            {
                if ((BIT(iCPU) & dwSystemAffinityMask) && (BIT(iCPU) & dwProcessAffinityMask))
                    break;
            }
        }

        if (iCPU != 32)
            ::SetThreadAffinityMask(::GetCurrentThread(), BIT(iCPU));
    }
}


/*====================
  CSystem::GetAffinityMask
  ====================*/
uint    CSystem::GetAffinityMask() const
{
    DWORD dwProcessAffinityMask, dwSystemAffinityMask;
    if (::GetProcessAffinityMask(::GetCurrentProcess(), &dwProcessAffinityMask, &dwSystemAffinityMask))
        return uint(dwProcessAffinityMask & dwSystemAffinityMask);
    else
        return 0;
}


/*====================
  CSystem::SetPriority
  ====================*/
void    CSystem::SetPriority(int iPriority)
{
    ::SetThreadPriority(::GetCurrentThread(), iPriority);
}


/*====================
  CSystem::GetPriority
  ====================*/
int     CSystem::GetPriority() const
{
    return ::GetThreadPriority(::GetCurrentThread());
}


/*====================
  CSystem::GetProcessFilename
  ====================*/
tstring CSystem::GetProcessFilename()
{
    HMODULE hModule(GetModuleHandle(nullptr));
    TCHAR szFilename[MAX_PATH];

    GetModuleFileName(hModule, szFilename, sizeof(szFilename));
    
    return tstring(szFilename);
}


/*====================
  CSystem::GetProcessBaseName
  ====================*/
tstring CSystem::GetProcessBaseName()
{
    HANDLE hProcess(GetCurrentProcess());
    HMODULE hModule(GetModuleHandle(nullptr));
    TCHAR szFilename[MAX_PATH];

    GetModuleBaseName(hProcess, hModule, szFilename, sizeof(szFilename));
    
    return tstring(szFilename);
}


/*====================
  CSystem::Shell
  ====================*/
bool    CSystem::Shell(const tstring &sCommand, bool bWait)
{
    STARTUPINFO SI;
    PROCESS_INFORMATION PI;
    
    ZeroMemory(&SI,sizeof(STARTUPINFO));
    ZeroMemory(&PI,sizeof(PROCESS_INFORMATION));
    SI.cb = sizeof(STARTUPINFO);

    if (!CreateProcess(
        nullptr,
        (LPTSTR)sCommand.c_str(),
        nullptr,
        nullptr,
        FALSE,
        CREATE_SUSPENDED,
        nullptr,
        nullptr,
        &SI,
        &PI))
    {
        Console.Err << _T("Error while creating process") << newl;
        return false;
    }

    // Let the main thread run
    ResumeThread(PI.hThread);
    
    if (bWait)
        WaitForSingleObject(PI.hProcess, INFINITE);

    // Cleanup
    CloseHandle(PI.hProcess);
    CloseHandle(PI.hThread);
    
    return true;
}


/*====================
  CSystem::GetUniqueID
  ====================*/
uint    CSystem::GetUniqueID() const
{
    uint uiReturn(GetTicks() & UINT_MAX);

    // Get network adapter connection status and MAC address
    IP_ADAPTER_INFO *pAdapterInfo(nullptr);
    ULONG ulSize(0);
    if (GetAdaptersInfo(pAdapterInfo, &ulSize) == ERROR_BUFFER_OVERFLOW)
        pAdapterInfo = reinterpret_cast<IP_ADAPTER_INFO*>(K2_NEW_ARRAY(ctx_System, byte, ulSize));

    if (pAdapterInfo != nullptr && GetAdaptersInfo(pAdapterInfo, &ulSize) == ERROR_SUCCESS)
    {
        for (uint ui(0); ui < MIN(pAdapterInfo->AddressLength, 4u); ++ui)
            uiReturn |= pAdapterInfo->Address[ui] << (8 * ui);
    }

    SAFE_DELETE_ARRAY(pAdapterInfo)
    return uiReturn;
}


#if 0
/*====================
  CSystem::InitDedicatedConsole
  ====================*/
void    CSystem::InitDedicatedConsole()
{
    // Start the dedicated server console
    SetConsoleWindowHandle(::CreateDialog((HINSTANCE)GetInstanceHandle(),
                                    MAKEINTRESOURCE(IDD_CONSOLE),
                                    0,
                                    (DLGPROC)GetConsoleWndProc()));
            
    if (GetConsoleWindowHandle())
        ShowWindow((HWND)GetConsoleWindowHandle(), SW_SHOWNORMAL);
    else
        EX_ERROR(_T("CreateDialog Failed: ") + GetLastErrorString());

    // Set Dedicated console font
    HDC hDisplayDC(GetDC(nullptr));
    if (!hDisplayDC)
        return;

    int iPointSize(90);

    LOGFONT logFont;
    MemManager.Set(&logFont, 0, sizeof(LOGFONT));
    _tcscpy_s(logFont.lfFaceName, LF_FACESIZE, _T("Lucida Console"));
    logFont.lfHeight = -MulDiv(iPointSize, GetDeviceCaps(hDisplayDC, LOGPIXELSY), 720);

    HFONT hFont(CreateFontIndirect(&logFont));

    if (hFont)
    {
        HWND hConsoleText(GetDlgItem((HWND)GetConsoleWindowHandle(), IDC_OUTPUT));

        SendMessage(hConsoleText, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));
    }

    ReleaseDC(nullptr, hDisplayDC);
}
#else
/*====================
  CSystem::InitDedicatedConsole
  ====================*/
void    CSystem::InitDedicatedConsole()
{
    AllocConsole();

    m_ConsoleWindowHandle = ::GetStdHandle(STD_OUTPUT_HANDLE);

    //SetConsoleTextAttribute((HANDLE)m_ConsoleWindowHandle, FOREGROUND_INTENSITY | FOREGROUND_GREEN);

    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_PROCESSED_INPUT);
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT);

    // Update console window title
    if (K2System.IsServerManager())
    {
        ::SetConsoleTitle(_T("K2 Server Manager"));
    }
    else
    {
        ::SetConsoleTitle(m_sConsoleTitle.c_str());
    }
}
#endif


/*====================
  CSystem::InitKeyboardMap
  ====================*/
void    CSystem::InitKeyboardMap()
{
    m_mapKeyboard[VK_BACK] =        BUTTON_BACKSPACE;
    m_mapKeyboard[VK_TAB] =         BUTTON_TAB;
    m_mapKeyboard[VK_RETURN] =      BUTTON_ENTER;
    m_mapKeyboard[VK_ESCAPE] =      BUTTON_ESC;
    m_mapKeyboard[VK_SPACE] =       BUTTON_SPACE;

    m_mapKeyboard[VK_CONTROL] =     BUTTON_CTRL;
    m_mapKeyboard[VK_MENU] =        BUTTON_ALT;
    m_mapKeyboard[VK_SHIFT] =       BUTTON_SHIFT;

    m_mapKeyboard[VK_INSERT] =      BUTTON_INS;
    m_mapKeyboard[VK_DELETE] =      BUTTON_DEL;
    m_mapKeyboard[VK_HOME] =        BUTTON_HOME;
    m_mapKeyboard[VK_END] =         BUTTON_END;
    m_mapKeyboard[VK_NEXT] =        BUTTON_PGDN;
    m_mapKeyboard[VK_PRIOR] =       BUTTON_PGUP;

    m_mapKeyboard[VK_UP] =          BUTTON_UP;
    m_mapKeyboard[VK_DOWN] =        BUTTON_DOWN;
    m_mapKeyboard[VK_LEFT] =        BUTTON_LEFT;
    m_mapKeyboard[VK_RIGHT] =       BUTTON_RIGHT;

    m_mapKeyboard[VK_CAPITAL] =     BUTTON_CAPS_LOCK;

    m_mapKeyboard[VK_LWIN] =        BUTTON_LWIN;
    m_mapKeyboard[VK_RWIN] =        BUTTON_RWIN;
    m_mapKeyboard[VK_APPS] =        BUTTON_MENU;

    m_mapKeyboard[VK_SNAPSHOT] =    BUTTON_PRINTSCREEN;
    m_mapKeyboard[VK_SCROLL] =      BUTTON_SCROLL_LOCK;
    m_mapKeyboard[VK_PAUSE] =       BUTTON_PAUSE;

    m_mapKeyboard[VK_NUMLOCK] =     BUTTON_NUM_LOCK;
    m_mapKeyboard[VK_MULTIPLY] =    BUTTON_MULTIPLY;
    m_mapKeyboard[VK_DIVIDE] =      BUTTON_DIVIDE;
    m_mapKeyboard[VK_ADD] =         BUTTON_ADD;
    m_mapKeyboard[VK_SUBTRACT] =    BUTTON_SUBTRACT;
    m_mapKeyboard[VK_DECIMAL] =     BUTTON_DECIMAL;
    m_mapKeyboard[VK_NUMPAD0] =     BUTTON_NUM0;
    m_mapKeyboard[VK_NUMPAD1] =     BUTTON_NUM1;
    m_mapKeyboard[VK_NUMPAD2] =     BUTTON_NUM2;
    m_mapKeyboard[VK_NUMPAD3] =     BUTTON_NUM3;
    m_mapKeyboard[VK_NUMPAD4] =     BUTTON_NUM4;
    m_mapKeyboard[VK_NUMPAD5] =     BUTTON_NUM5;
    m_mapKeyboard[VK_NUMPAD6] =     BUTTON_NUM6;
    m_mapKeyboard[VK_NUMPAD7] =     BUTTON_NUM7;
    m_mapKeyboard[VK_NUMPAD8] =     BUTTON_NUM8;
    m_mapKeyboard[VK_NUMPAD9] =     BUTTON_NUM9;

    m_mapKeyboard[VK_F1] =          BUTTON_F1;
    m_mapKeyboard[VK_F2] =          BUTTON_F2;
    m_mapKeyboard[VK_F3] =          BUTTON_F3;
    m_mapKeyboard[VK_F4] =          BUTTON_F4;
    m_mapKeyboard[VK_F5] =          BUTTON_F5;
    m_mapKeyboard[VK_F6] =          BUTTON_F6;
    m_mapKeyboard[VK_F7] =          BUTTON_F7;
    m_mapKeyboard[VK_F8] =          BUTTON_F8;
    m_mapKeyboard[VK_F9] =          BUTTON_F9;
    m_mapKeyboard[VK_F10] =         BUTTON_F10;
    m_mapKeyboard[VK_F11] =         BUTTON_F11;
    m_mapKeyboard[VK_F12] =         BUTTON_F12;

    m_mapKeyboard[VK_OEM_1] =       BUTTON_MISC1;
    m_mapKeyboard[VK_OEM_2] =       BUTTON_MISC2;
    m_mapKeyboard[VK_OEM_3] =       BUTTON_MISC3;
    m_mapKeyboard[VK_OEM_4] =       BUTTON_MISC4;
    m_mapKeyboard[VK_OEM_5] =       BUTTON_MISC5;
    m_mapKeyboard[VK_OEM_6] =       BUTTON_MISC6;
    m_mapKeyboard[VK_OEM_7] =       BUTTON_MISC7;

    m_mapKeyboard[VK_OEM_PLUS] =    BUTTON_PLUS;
    m_mapKeyboard[VK_OEM_MINUS] =   BUTTON_MINUS;
    m_mapKeyboard[VK_OEM_COMMA] =   BUTTON_COMMA;
    m_mapKeyboard[VK_OEM_PERIOD] =  BUTTON_PERIOD;
}


/*====================
  CSystem::GetButton
  ====================*/
EButton CSystem::GetButton(size_t vk, size_t lParam)
{
    // Numbers and letters don't need to map
    if ((vk >= 'A' && vk <= 'Z') ||
        (vk >= '0' && vk <= '9'))
        return static_cast<EButton>(vk);

    // Handle system keys
    EButton button;
    KeyboardMap::iterator findit(m_mapKeyboard.find(vk));
    if (findit == m_mapKeyboard.end())
        return BUTTON_INVALID;
    else
        button = findit->second;

    // Special case adjustments
    switch (button)
    {
    case BUTTON_ENTER:
        if (key_splitEnter && (lParam & (1 << 24)))
            return BUTTON_NUM_ENTER;
        else
            return BUTTON_ENTER;

    case BUTTON_RWIN:
    case BUTTON_LWIN:
        if (key_splitWin)
            return button;
        else
            return BUTTON_WIN;

    case BUTTON_ALT:
        if (!key_splitAlt)
            return BUTTON_ALT;
        else if (lParam & (1 << 24))
            return BUTTON_RALT;
        else
            return BUTTON_LALT;

    case BUTTON_CTRL:
        if (!key_splitCtrl)
            return BUTTON_CTRL;
        else if (lParam & (1 << 24))
            return BUTTON_RCTRL;
        else
            return BUTTON_LCTRL;

    // The right shift key isn't considered "extended" like
    // the right alt and control, but it does have a different
    // scancode (hopefully this scan code is universal)
    case BUTTON_SHIFT:
        if (!key_splitShift)
            return BUTTON_SHIFT;
        else if (((lParam >> 16) & 0xff) == 0x2a)
            return BUTTON_LSHIFT;
        else
            return BUTTON_RSHIFT;
    }

    return button;
}


/*====================
  CSystem::SetupKeystates

  This setups the keystates on init & refocus. See the WM_ACTIVATE comment.
  ====================*/
void    CSystem::SetupKeystates()
{
    char keyStates[256] = {0};
    //GetKeyboardState((unsigned char*)keyStates);
    memset(keyStates, 0, sizeof(keyStates));
    
    // ENTER and NUMPAD ENTER both return as VK_RETURN
    // As such, they cannot be differentiated; force as off.
    Input.SetButton(BUTTON_NUM_ENTER, false);

    // These keys are not part of the primary loop, so handle them here.
    // ALT
    if (key_splitAlt)
    {
        Input.SetButton(BUTTON_ALT, false);
        Input.SetButton(BUTTON_RALT, (keyStates[VK_RMENU] & 0x80) != 0);
        Input.SetButton(BUTTON_LALT, (keyStates[VK_LMENU] & 0x80) != 0);
    }
    else
    {
        Input.SetButton(BUTTON_ALT, (keyStates[VK_MENU] & 0x80) != 0);
        Input.SetButton(BUTTON_RALT, false);
        Input.SetButton(BUTTON_LALT, false);
    }
    // CTRL
    if (key_splitCtrl)
    {
        Input.SetButton(BUTTON_CTRL, false);
        Input.SetButton(BUTTON_RCTRL, (keyStates[VK_RCONTROL] & 0x80) != 0);
        Input.SetButton(BUTTON_LCTRL, (keyStates[VK_LCONTROL] & 0x80) != 0);
    }
    else
    {
        Input.SetButton(BUTTON_CTRL, (keyStates[VK_CONTROL] & 0x80) != 0);
        Input.SetButton(BUTTON_RCTRL, false);
        Input.SetButton(BUTTON_LCTRL, false);
    }
    // SHIFT
    if (key_splitShift)
    {
        Input.SetButton(BUTTON_SHIFT, false);
        Input.SetButton(BUTTON_RSHIFT, (keyStates[VK_RSHIFT] & 0x80) != 0);
        Input.SetButton(BUTTON_LSHIFT, (keyStates[VK_LSHIFT] & 0x80) != 0);
    }
    else
    {
        Input.SetButton(BUTTON_SHIFT, (keyStates[VK_SHIFT] & 0x80) != 0);
        Input.SetButton(BUTTON_RSHIFT, false);
        Input.SetButton(BUTTON_LSHIFT, false);
    }

    // Main loop... Most keys are handled here.
    for (int vk(0); vk < 256; vk++)
    {
        if ((vk >= 'A' && vk <= 'Z') || (vk >= '0' && vk <= '9'))
            Input.SetButton(static_cast<EButton>(vk), (keyStates[vk] & 0x80) != 0);
        else if(vk != VK_SHIFT && vk != VK_CONTROL && vk != VK_MENU)
        {
            EButton button;
            KeyboardMap::iterator findit(m_mapKeyboard.find(vk));
            if (findit != m_mapKeyboard.end())
            {
                button = findit->second;
                if (!key_splitWin && (button == BUTTON_RWIN || button == BUTTON_LWIN))
                    button = BUTTON_WIN;
                Input.SetButton(button, (keyStates[vk] & 0x80) != 0);
            }
        }
    }
}

/*====================
  CSystem::SetMicVolume

  This function controls the mic volume setting on microphone input
  ====================*/
void    CSystem::SetMicVolume(float fValue)
{
    HMIXER hMixer(nullptr);                    // Mixer handle for current device
    MMRESULT result;                        // Return code.
    MIXERLINE mixerLine;                    // Holds the mixer line data.
    MIXERLINECONTROLS mixerLineControl;     // Obtains the mixer control.
    MIXERCAPS mixerCaps;                    // Holds the capabilities for the driver.

    bool bDone(false);
    uint uiDevice(0);

    while (!bDone)
    {
        result = mixerOpen(&hMixer, uiDevice, 0, 0, MIXER_OBJECTF_MIXER);

        if (result != MMSYSERR_NOERROR)
            break;

#pragma warning (disable : 4311)
        result = mixerGetDevCaps(reinterpret_cast<UINT>(hMixer), &mixerCaps, sizeof(MIXERCAPS));
#pragma warning (default : 4311)

        if (result != MMSYSERR_NOERROR)
            break;

        tstring sDeviceName(mixerCaps.szPname);

        if (CompareNoCase(K2SoundManager.GetRecordDriverName(), sDeviceName) != 0)
        {
            uiDevice++;
            continue;
        }

        for (dword dwLine(0); dwLine < mixerCaps.cDestinations; dwLine++)
        {
            // Initialize MIXERLINE structure.
            MemManager.Set(&mixerLine, 0, sizeof(mixerLine));
            mixerLine.cbStruct = sizeof(mixerLine);
            mixerLine.dwDestination = dwLine;

            result = mixerGetLineInfo((HMIXEROBJ)hMixer, &mixerLine, MIXER_GETLINEINFOF_DESTINATION | MIXER_OBJECTF_HMIXER);

            if (result != MMSYSERR_NOERROR)
                continue;

            uint uiConnections(mixerLine.cConnections);

            for (uint uiSource(0); uiSource <= uiConnections; uiSource++)
            {
                if (uiSource > 0)
                {
                    MemManager.Set(&mixerLine, 0, sizeof(mixerLine));
                    mixerLine.cbStruct = sizeof(mixerLine);
                    mixerLine.dwDestination = dwLine;
                    mixerLine.dwSource = uiSource - 1;

                    result = mixerGetLineInfo((HMIXEROBJ)hMixer, &mixerLine, MIXER_GETLINEINFOF_SOURCE | MIXER_OBJECTF_HMIXER);

                    if (result != MMSYSERR_NOERROR)
                        break;
                }

                if (mixerLine.cControls == 0)
                    continue;

                if (mixerLine.dwComponentType != MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE &&
                    mixerLine.dwComponentType != MIXERLINE_COMPONENTTYPE_DST_WAVEIN &&
                    mixerLine.dwComponentType != MIXERLINE_COMPONENTTYPE_DST_VOICEIN)
                    continue;

                MIXERCONTROL *mixerControl;             // Holds the mixer control data.

                mixerControl = K2_NEW_ARRAY(ctx_System, MIXERCONTROL, mixerLine.cControls);

                // Get the control
                MemManager.Set(&mixerLineControl, 0, sizeof(mixerLineControl));

                mixerLineControl.cbStruct = sizeof(mixerLineControl);
                mixerLineControl.dwLineID = mixerLine.dwLineID;
                mixerLineControl.cControls = mixerLine.cControls;
                mixerLineControl.cbmxctrl = sizeof(MIXERCONTROL);
                mixerLineControl.pamxctrl = mixerControl;

                MemManager.Set(mixerControl, 0, sizeof(MIXERCONTROL) * mixerLine.cControls);

                result = mixerGetLineControls((HMIXEROBJ)hMixer, &mixerLineControl, MIXER_GETLINECONTROLSF_ALL);

                if (result != MMSYSERR_NOERROR)
                {
                    SAFE_DELETE_ARRAY(mixerControl);
                    continue;
                }

                MIXERCONTROLDETAILS mixerControlData;   // Gets the control values.
                MIXERCONTROLDETAILS_UNSIGNED uiData;

                // Initialize the MIXERCONTROLDETAILS structure
                MemManager.Set(&mixerControlData, 0, sizeof(mixerControlData));
                mixerControlData.cbStruct = sizeof(mixerControlData);
                mixerControlData.cbDetails = sizeof(uiData);
                mixerControlData.paDetails = &uiData;
                mixerControlData.cChannels = 1;

                for (uint i(0); i < mixerLine.cControls; i++)
                {
                    if (mixerControl[i].dwControlType != MIXERCONTROL_CONTROLTYPE_VOLUME)
                        continue;

                    uiData.dwValue = ((mixerControl[i].Bounds.dwMaximum - mixerControl[i].Bounds.dwMinimum) * fValue) + mixerControl[i].Bounds.dwMinimum;

                    mixerControlData.dwControlID = mixerControl[i].dwControlID;
                    result = mixerSetControlDetails((HMIXEROBJ)hMixer, &mixerControlData, MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE);
                }

                SAFE_DELETE_ARRAY(mixerControl);
            }
        }

        mixerClose(hMixer);
        uiDevice++;
    }
}

/*====================
  CSystem::GetMicVolume

  This function returns the average volume of all recording channels
  ====================*/
float   CSystem::GetMicVolume()
{
    HMIXER hMixer(nullptr);                    // Mixer handle for current device
    MMRESULT result;                        // Return code.
    MIXERLINE mixerLine;                    // Holds the mixer line data.
    MIXERLINECONTROLS mixerLineControl;     // Obtains the mixer control.
    MIXERCAPS mixerCaps;                    // Holds the capabilities for the driver.

    float fValue(0.0f);
    float fMult(1.0f);
    uint uiNumValues(0);

    bool bDone(false);
    uint uiDevice(0);

    while (!bDone)
    {
        result = mixerOpen(&hMixer, uiDevice, 0, 0, MIXER_OBJECTF_MIXER);

        if (result != MMSYSERR_NOERROR)
            break;

#pragma warning (disable : 4311)
        result = mixerGetDevCaps(reinterpret_cast<UINT>(hMixer), &mixerCaps, sizeof(MIXERCAPS));
#pragma warning (default : 4311)

        if (result != MMSYSERR_NOERROR)
            break;

        tstring sDeviceName(mixerCaps.szPname);

        if (CompareNoCase(K2SoundManager.GetRecordDriverName(), sDeviceName) != 0)
        {
            uiDevice++;
            continue;
        }

        for (dword dwLine(0); dwLine < mixerCaps.cDestinations; dwLine++)
        {
            // Initialize MIXERLINE structure.
            MemManager.Set(&mixerLine, 0, sizeof(mixerLine));
            mixerLine.cbStruct = sizeof(mixerLine);
            mixerLine.dwDestination = dwLine;

            result = mixerGetLineInfo((HMIXEROBJ)hMixer, &mixerLine, MIXER_GETLINEINFOF_DESTINATION | MIXER_OBJECTF_HMIXER);

            if (result != MMSYSERR_NOERROR)
                continue;

            uint uiConnections(mixerLine.cConnections);

            for (uint uiSource(0); uiSource <= uiConnections; uiSource++)
            {
                if (uiSource > 0)
                {
                    MemManager.Set(&mixerLine, 0, sizeof(mixerLine));
                    mixerLine.cbStruct = sizeof(mixerLine);
                    mixerLine.dwDestination = dwLine;
                    mixerLine.dwSource = uiSource - 1;

                    result = mixerGetLineInfo((HMIXEROBJ)hMixer, &mixerLine, MIXER_GETLINEINFOF_SOURCE | MIXER_OBJECTF_HMIXER);

                    if (result != MMSYSERR_NOERROR)
                        break;
                }

                if (mixerLine.cControls == 0)
                    continue;

                if (mixerLine.dwComponentType != MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE &&
                    mixerLine.dwComponentType != MIXERLINE_COMPONENTTYPE_DST_WAVEIN &&
                    mixerLine.dwComponentType != MIXERLINE_COMPONENTTYPE_DST_VOICEIN)
                    continue;

                MIXERCONTROL *mixerControl;             // Holds the mixer control data.

                mixerControl = K2_NEW_ARRAY(ctx_System, MIXERCONTROL, mixerLine.cControls);

                // Get the control
                MemManager.Set(&mixerLineControl, 0, sizeof(mixerLineControl));

                mixerLineControl.cbStruct = sizeof(mixerLineControl);
                mixerLineControl.dwLineID = mixerLine.dwLineID;
                mixerLineControl.cControls = mixerLine.cControls;
                mixerLineControl.cbmxctrl = sizeof(MIXERCONTROL);
                mixerLineControl.pamxctrl = mixerControl;

                MemManager.Set(mixerControl, 0, sizeof(MIXERCONTROL) * mixerLine.cControls);

                result = mixerGetLineControls((HMIXEROBJ)hMixer, &mixerLineControl, MIXER_GETLINECONTROLSF_ALL);

                if (result != MMSYSERR_NOERROR)
                {
                    SAFE_DELETE_ARRAY(mixerControl);
                    continue;
                }

                MIXERCONTROLDETAILS mixerControlData;   // Gets the control values.
                MIXERCONTROLDETAILS_UNSIGNED uiData;

                // Initialize the MIXERCONTROLDETAILS structure
                MemManager.Set(&mixerControlData, 0, sizeof(mixerControlData));
                mixerControlData.cbStruct = sizeof(mixerControlData);
                mixerControlData.cbDetails = sizeof(uiData);
                mixerControlData.paDetails = &uiData;
                mixerControlData.cChannels = 1;

                for (uint i(0); i < mixerLine.cControls; i++)
                {
                    if (mixerControl[i].dwControlType != MIXERCONTROL_CONTROLTYPE_VOLUME)
                        continue;

                    mixerControlData.dwControlID = mixerControl[i].dwControlID;
                    result = mixerGetControlDetails((HMIXEROBJ)hMixer, &mixerControlData, MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE);

                    if (mixerLine.dwComponentType != MIXERLINE_COMPONENTTYPE_DST_WAVEIN && mixerLine.dwComponentType != MIXERLINE_COMPONENTTYPE_DST_VOICEIN)
                    {
                        fValue += (uiData.dwValue - mixerControl[i].Bounds.dwMinimum) / float(mixerControl[i].Bounds.dwMaximum - mixerControl[i].Bounds.dwMinimum);
                        uiNumValues++;
                    }
                    else
                    {
                        fMult *= (uiData.dwValue - mixerControl[i].Bounds.dwMinimum) / float(mixerControl[i].Bounds.dwMaximum - mixerControl[i].Bounds.dwMinimum);
                    }
                }

                SAFE_DELETE_ARRAY(mixerControl);
            }
        }

        mixerClose(hMixer);
        uiDevice++;
    }

    if (uiNumValues > 0)
        return ((fValue / float(uiNumValues)) * fMult);

    return fMult;
}

/*====================
  CSystem::SetMicBoost

  This function controls the mic boost setting on microphone input
  ====================*/
void    CSystem::SetMicBoost(bool bValue)
{
    HMIXER hMixer(nullptr);                    // Mixer handle for current device
    MMRESULT result;                        // Return code.
    MIXERLINE mixerLine;                    // Holds the mixer line data.
    MIXERLINECONTROLS mixerLineControl;     // Obtains the mixer control.
    MIXERCAPS mixerCaps;                    // Holds the capabilities for the driver.

    bool bDone(false);
    uint uiDevice(0);

    while (!bDone)
    {
        result = mixerOpen(&hMixer, uiDevice, 0, 0, MIXER_OBJECTF_MIXER);

        if (result != MMSYSERR_NOERROR)
            break;

#pragma warning (disable : 4311)
        result = mixerGetDevCaps(reinterpret_cast<UINT>(hMixer), &mixerCaps, sizeof(MIXERCAPS));
#pragma warning (default : 4311)

        if (result != MMSYSERR_NOERROR)
            break;

        for (dword dwLine(0); dwLine < mixerCaps.cDestinations; dwLine++)
        {
            // Initialize MIXERLINE structure.
            MemManager.Set(&mixerLine, 0, sizeof(mixerLine));
            mixerLine.cbStruct = sizeof(mixerLine);
            mixerLine.dwDestination = dwLine;

            result = mixerGetLineInfo((HMIXEROBJ)hMixer, &mixerLine, MIXER_GETLINEINFOF_DESTINATION | MIXER_OBJECTF_HMIXER);

            if (result != MMSYSERR_NOERROR)
                continue;

            uint uiConnections(mixerLine.cConnections);

            for (uint uiSource(0); uiSource <= uiConnections; uiSource++)
            {
                if (uiSource > 0)
                {
                    MemManager.Set(&mixerLine, 0, sizeof(mixerLine));
                    mixerLine.cbStruct = sizeof(mixerLine);
                    mixerLine.dwDestination = dwLine;
                    mixerLine.dwSource = uiSource - 1;

                    result = mixerGetLineInfo((HMIXEROBJ)hMixer, &mixerLine, MIXER_GETLINEINFOF_SOURCE | MIXER_OBJECTF_HMIXER);

                    if (result != MMSYSERR_NOERROR)
                        break;
                }

                if (mixerLine.cControls == 0)
                    continue;

                if (mixerLine.dwComponentType != MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE &&
                    mixerLine.dwComponentType != MIXERLINE_COMPONENTTYPE_DST_WAVEIN &&
                    mixerLine.dwComponentType != MIXERLINE_COMPONENTTYPE_DST_VOICEIN)
                    continue;

                MIXERCONTROL *mixerControl;             // Holds the mixer control data.

                mixerControl = K2_NEW_ARRAY(ctx_System, MIXERCONTROL, mixerLine.cControls);

                // Get the control
                MemManager.Set(&mixerLineControl, 0, sizeof(mixerLineControl));

                mixerLineControl.cbStruct = sizeof(mixerLineControl);
                mixerLineControl.dwLineID = mixerLine.dwLineID;
                mixerLineControl.cControls = mixerLine.cControls;
                mixerLineControl.cbmxctrl = sizeof(MIXERCONTROL);
                mixerLineControl.pamxctrl = mixerControl;

                MemManager.Set(mixerControl, 0, sizeof(MIXERCONTROL) * mixerLine.cControls);

                result = mixerGetLineControls((HMIXEROBJ)hMixer, &mixerLineControl, MIXER_GETLINECONTROLSF_ALL);

                if (result != MMSYSERR_NOERROR)
                {
                    SAFE_DELETE_ARRAY(mixerControl);
                    continue;
                }

                MIXERCONTROLDETAILS mixerControlData;   // Gets the control values.
                MIXERCONTROLDETAILS_BOOLEAN bData;

                // Initialize the MIXERCONTROLDETAILS structure
                MemManager.Set(&mixerControlData, 0, sizeof(mixerControlData));
                mixerControlData.cbStruct = sizeof(mixerControlData);
                mixerControlData.cbDetails = sizeof(bData);
                mixerControlData.paDetails = &bData;
                mixerControlData.cChannels = 1;

                bData.fValue = bValue;

                for (uint i(0); i < mixerLine.cControls; i++)
                {
                    if (mixerControl[i].dwControlType != MIXERCONTROL_CONTROLTYPE_ONOFF)
                        continue;

                    tstring sName(mixerControl[i].szName);

                    sName = LowerString(sName);

                    if (sName.find(_T("boost")) == tstring::npos)
                        continue;

                    mixerControlData.dwControlID = mixerControl[i].dwControlID;
                    result = mixerSetControlDetails((HMIXEROBJ)hMixer, &mixerControlData, MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE);
                }

                SAFE_DELETE_ARRAY(mixerControl);
            }
        }

        mixerClose(hMixer);
        uiDevice++;
    }
}


/*====================
  CSystem::GetMicBoost

  This function controls the mic boost setting on microphone input
  ====================*/
bool    CSystem::GetMicBoost()
{
    HMIXER hMixer(nullptr);                    // Mixer handle for current device
    MMRESULT result;                        // Return code.
    MIXERLINE mixerLine;                    // Holds the mixer line data.
    MIXERLINECONTROLS mixerLineControl;     // Obtains the mixer control.
    MIXERCAPS mixerCaps;                    // Holds the capabilities for the driver.

    bool bDone(false);
    bool bValue(true);
    uint uiDevice(0);

    while (!bDone)
    {
        result = mixerOpen(&hMixer, uiDevice, 0, 0, MIXER_OBJECTF_MIXER);

        if (result != MMSYSERR_NOERROR)
            break;

#pragma warning (disable : 4311)
        result = mixerGetDevCaps(reinterpret_cast<UINT>(hMixer), &mixerCaps, sizeof(MIXERCAPS));
#pragma warning (default : 4311)

        if (result != MMSYSERR_NOERROR)
            break;

        for (dword dwLine(0); dwLine < mixerCaps.cDestinations; dwLine++)
        {
            // Initialize MIXERLINE structure.
            MemManager.Set(&mixerLine, 0, sizeof(mixerLine));
            mixerLine.cbStruct = sizeof(mixerLine);
            mixerLine.dwDestination = dwLine;

            result = mixerGetLineInfo((HMIXEROBJ)hMixer, &mixerLine, MIXER_GETLINEINFOF_DESTINATION | MIXER_OBJECTF_HMIXER);

            if (result != MMSYSERR_NOERROR)
                continue;

            uint uiConnections(mixerLine.cConnections);

            for (uint uiSource(0); uiSource <= uiConnections; uiSource++)
            {
                if (uiSource > 0)
                {
                    MemManager.Set(&mixerLine, 0, sizeof(mixerLine));
                    mixerLine.cbStruct = sizeof(mixerLine);
                    mixerLine.dwDestination = dwLine;
                    mixerLine.dwSource = uiSource - 1;

                    result = mixerGetLineInfo((HMIXEROBJ)hMixer, &mixerLine, MIXER_GETLINEINFOF_SOURCE | MIXER_OBJECTF_HMIXER);

                    if (result != MMSYSERR_NOERROR)
                        break;
                }

                if (mixerLine.cControls == 0)
                    continue;

                if (mixerLine.dwComponentType != MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE &&
                    mixerLine.dwComponentType != MIXERLINE_COMPONENTTYPE_DST_WAVEIN &&
                    mixerLine.dwComponentType != MIXERLINE_COMPONENTTYPE_DST_VOICEIN)
                    continue;

                MIXERCONTROL *mixerControl;             // Holds the mixer control data.

                mixerControl = K2_NEW_ARRAY(ctx_System, MIXERCONTROL, mixerLine.cControls);

                // Get the control
                MemManager.Set(&mixerLineControl, 0, sizeof(mixerLineControl));

                mixerLineControl.cbStruct = sizeof(mixerLineControl);
                mixerLineControl.dwLineID = mixerLine.dwLineID;
                mixerLineControl.cControls = mixerLine.cControls;
                mixerLineControl.cbmxctrl = sizeof(MIXERCONTROL);
                mixerLineControl.pamxctrl = mixerControl;

                MemManager.Set(mixerControl, 0, sizeof(MIXERCONTROL) * mixerLine.cControls);

                result = mixerGetLineControls((HMIXEROBJ)hMixer, &mixerLineControl, MIXER_GETLINECONTROLSF_ALL);

                if (result != MMSYSERR_NOERROR)
                {
                    SAFE_DELETE_ARRAY(mixerControl);
                    continue;
                }

                MIXERCONTROLDETAILS mixerControlData;   // Gets the control values.
                MIXERCONTROLDETAILS_BOOLEAN bData;

                // Initialize the MIXERCONTROLDETAILS structure
                MemManager.Set(&mixerControlData, 0, sizeof(mixerControlData));
                mixerControlData.cbStruct = sizeof(mixerControlData);
                mixerControlData.cbDetails = sizeof(bData);
                mixerControlData.paDetails = &bData;
                mixerControlData.cChannels = 1;

                for (uint i(0); i < mixerLine.cControls; i++)
                {
                    if (mixerControl[i].dwControlType != MIXERCONTROL_CONTROLTYPE_ONOFF)
                        continue;

                    tstring sName(mixerControl[i].szName);

                    sName = LowerString(sName);

                    if (sName.find(_T("boost")) == tstring::npos)
                        continue;

                    mixerControlData.dwControlID = mixerControl[i].dwControlID;
                    result = mixerGetControlDetails((HMIXEROBJ)hMixer, &mixerControlData, MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE);

                    bValue &= (bData.fValue != 0.0f);
                }

                SAFE_DELETE_ARRAY(mixerControl);
            }
        }

        mixerClose(hMixer);
        uiDevice++;
    }

    return bValue;
}


/*====================
  CSystem::SetTitle
  ====================*/
void    CSystem::SetTitle(const tstring &sTitle)
{
    if (sTitle == m_sConsoleTitle)
        return;

    m_sConsoleTitle = sTitle;

    if (m_ConsoleWindowHandle)
    {
        if (K2System.IsServerManager())
            ::SetConsoleTitle(_T("K2 Server Manager"));
        else
            ::SetConsoleTitle(m_sConsoleTitle.c_str());
    }
}


/*====================
  CSystem::GetRandomSeed32
  ====================*/
uint    CSystem::GetRandomSeed32()
{
    uint uiSeed(0);

    rand_s(&uiSeed);

    return uiSeed;
}


//=============================================================================
// class CMemMap
//=============================================================================
class CMemMap
{
private:
    //*********************
    // Private Definitions
    //*********************
    static const size_t MEM_PAGE_SIZE = (4096);
    //static const size_t MEM_HEAP_SIZE = (2 * 1024 * 1024 * 1024);
    static const size_t MEM_PAGES_2GB = (524288);

    // SMemoryPage
    struct SMemoryPage
    {
        ushort      uiChannels[3];

        SMemoryPage()
        {
            uiChannels[0] = 0;
            uiChannels[1] = 0;
            uiChannels[2] = 0;
        }
    };
    typedef vector<SMemoryPage>     MemPageVec;

    //*********************
    // Member Variables
    //*********************

    MemPageVec      m_vPages;

public:
    ~CMemMap();
    CMemMap();

    void            AddMem(const void* pBaseAddr, size_t uiSize, uint uiChannel);
    void            SaveImage(const tstring &sFilename);
};


/*====================
  CMemMap::~CMemMap
  ====================*/
CMemMap::~CMemMap()
{
}


/*====================
  CMemMap::CMemMap
  ====================*/
CMemMap::CMemMap()
{
    m_vPages.resize(MEM_PAGES_2GB);
}


/*====================
  CMemMap::AddMem
  ====================*/
void    CMemMap::AddMem(const void* pBaseAddr, size_t uiSize, uint uiChannel)
{
    assert(uiChannel < 3);
    uiChannel = CLAMP<uint>(uiChannel, 0, 2);

    size_t uiMemAddr((size_t)pBaseAddr);
    size_t uiMemEnd(uiMemAddr + uiSize);

    size_t uiMinPage((size_t)pBaseAddr / MEM_PAGE_SIZE);
    size_t uiMaxPage(((size_t)pBaseAddr + uiSize - 1) / MEM_PAGE_SIZE);

    // expand to 4GB if necessary.
    if (uiMaxPage >= m_vPages.size())
        m_vPages.resize(m_vPages.size() + MEM_PAGES_2GB);

    for (size_t uiPageIdx(uiMinPage); uiPageIdx <= uiMaxPage; ++uiPageIdx)
    {
        size_t uiPageAddr(uiPageIdx * MEM_PAGE_SIZE);
        size_t uiPageEnd(uiPageAddr + MEM_PAGE_SIZE);

        size_t uiUseAddr(uiMemAddr);
        if (uiUseAddr > uiPageEnd)  uiUseAddr = uiPageEnd;
        if (uiUseAddr < uiPageAddr) uiUseAddr = uiPageAddr;
        size_t uiUseEnd(uiMemEnd);
        if (uiUseEnd > uiPageEnd)   uiUseEnd = uiPageEnd;
        if (uiUseEnd < uiPageAddr)  uiUseEnd = uiPageAddr;

        size_t uiBytes(uiUseEnd - uiUseAddr);
        assert(uiBytes <= (ushort)~0);
        assert(uiBytes > 0);

        SMemoryPage& sMemPage(m_vPages[uiPageIdx]);
        sMemPage.uiChannels[uiChannel] += (ushort)uiBytes;

        assert(sMemPage.uiChannels[0] <= MEM_PAGE_SIZE);
        assert(sMemPage.uiChannels[1] <= MEM_PAGE_SIZE);
        assert(sMemPage.uiChannels[2] <= MEM_PAGE_SIZE);
    }
}


/*====================
  CMemMap::SaveImage
  ====================*/
void    CMemMap::SaveImage(const tstring &sFilename)
{
    uint uiImgW(1024);
    uint uiImgH(512);

    if (m_vPages.size() > MEM_PAGES_2GB)
        uiImgH = 2*512;
    if (m_vPages.size() > 2*MEM_PAGES_2GB)
        uiImgH = 3*512;
    if (m_vPages.size() > 3*MEM_PAGES_2GB)
        uiImgH = 4*512;

    CBitmap cImage(uiImgW, uiImgH, BITMAP_RGB);

    // initialize image to magenta.
    for (uint uiY(0); uiY < uiImgH; ++uiY)
        for (uint uiX(0); uiX < uiImgW; ++uiX)
            cImage.SetPixel4b((int)uiX, (int)uiY, 255, 0, 255, 255);

    // represent each memory page as a color.
    for (size_t uiPageIdx(0); uiPageIdx < m_vPages.size(); ++uiPageIdx)
    {
        SMemoryPage& sMemPage(m_vPages[uiPageIdx]);

        float fR(sMemPage.uiChannels[0] / (float)MEM_PAGE_SIZE);
        float fG(sMemPage.uiChannels[1] / (float)MEM_PAGE_SIZE);
        float fB(sMemPage.uiChannels[2] / (float)MEM_PAGE_SIZE);

        fR = CLAMP(fR, 0.0f, 1.0f);
        fG = 0.0f;
        fB = 0.0f;

        int iPelY((int)(uiPageIdx / uiImgW));
        int iPelX((int)(uiPageIdx % uiImgW));
        cImage.SetPixel4b(iPelX, iPelY, (byte)(255.0f * fR), (byte)(255.0f * fG), (byte)(255.0f * fB), 255);
    }

    // save the image.
    bool bSuccess(cImage.WritePNG(sFilename));
    bSuccess = bSuccess;
    assert(bSuccess);
}

//=============================================================================


/*====================
  CSystem::AnalyzeMemory
  ====================*/
void    CSystem::AnalyzeMemory()
{
    // Get all the heaps in the process
    HANDLE hHeaps[100];
    DWORD uiNumHeaps = ::GetProcessHeaps(100, hHeaps);
    Console.Dev << _T("The process has ") << uiNumHeaps << _T(" heaps.") << newl;

    // Get the default heap and the CRT heap (both are among those retrieved above)
    const HANDLE hHeapDefault   (::GetProcessHeap());
    const HANDLE hHeapCrt       ((HANDLE)_get_heap_handle());

    CMemMap cMemImg;

    size_t uiTotalBytes(0);
    size_t uiTotalOverhead(0);
    size_t uiTotalCommittedBytes(0);
    size_t uiTotalUncommittedBytes(0);
    size_t uiTotalFreeBytes(0);
    size_t uiTotalAllocatedBytes(0);

    for (unsigned int uiHeap(0); uiHeap < uiNumHeaps; uiHeap++)
    {
        ULONG uiHeapInfo(0);
        SIZE_T uiRetSize(0);

        // Query the heap attributes
        if (::HeapQueryInformation(hHeaps[uiHeap],
            HeapCompatibilityInformation,
            &uiHeapInfo,
            sizeof(uiHeapInfo),
            &uiRetSize))
        {
            // Show the heap attributes
            switch (uiHeapInfo)
            {
            case 0:
                Console.Dev << _T("Heap ") << uiHeap << _T(" is a regular heap.") << newl;
                break;
            case 1:
                Console.Dev << _T("Heap ") << uiHeap << _T(" is a heap with look-asides (fast heap).") << newl;
                break;
            case 2:
                Console.Dev << _T("Heap ") << uiHeap << _T(" is a LFH (low-fragmentation) heap.") << newl;
                break;
            default:
                Console.Dev << _T("Heap ") << uiHeap << _T(" is of unknown type.") << newl;
                break;
            }

            if (hHeaps[uiHeap] == hHeapDefault)
            {
                Console.Dev << _T(" This the DEFAULT process heap.") << newl;
            }
            if (hHeaps[uiHeap] == hHeapCrt)
            {
                Console.Dev << _T(" This the heap used by the CRT.") << newl;
            }

            // Walk the heap and show each allocated block inside it (the attributes of each entry
            // will differ between DEBUG and RELEASE builds)
            vector<PROCESS_HEAP_ENTRY> vHeapEntries;
            PROCESS_HEAP_ENTRY sHeapEntry;
            memset(&sHeapEntry, 0, sizeof(sHeapEntry));
            while (::HeapWalk(hHeaps[uiHeap], &sHeapEntry))
                vHeapEntries.push_back(sHeapEntry);

            size_t uiHeapAllocations(0);
            size_t uiHeapTotalBytes(0);
            size_t uiHeapTotalOverhead(0);
            size_t uiHeapCommittedBytes(0);
            size_t uiHeapUncommittedBytes(0);
            size_t uiHeapAllocatedBytes(0);
            size_t uiHeapFreeBytes(0);

            // Print out info about each entry
            for (size_t uiEntryIdx(0); uiEntryIdx < vHeapEntries.size(); ++uiEntryIdx)
            {
                const PROCESS_HEAP_ENTRY& sHeapEntry(vHeapEntries[uiEntryIdx]);

                // The entry is an allocated block in a heap. The number of bytes in the block
                // is obtained by subtracting the starting address [virtual address] of the next
                // block from the starting address [virtual address] of the present block
                if (sHeapEntry.wFlags & PROCESS_HEAP_ENTRY_BUSY)
                {
                    size_t uiBytesAllocated((size_t)sHeapEntry.cbData + (size_t)sHeapEntry.cbOverhead);
                    ++uiHeapAllocations;

                    uiHeapCommittedBytes += uiBytesAllocated;
                    uiHeapAllocatedBytes += uiBytesAllocated;

                    uiTotalCommittedBytes += uiBytesAllocated;
                    uiTotalAllocatedBytes += uiBytesAllocated;

                    cMemImg.AddMem(sHeapEntry.lpData, sHeapEntry.cbData, 0);
                }

                // The entry is a committed block which is free, i.e. not being allocated or not
                // being used as control structure. Data member cbData represents the size in bytes
                // for this range of free block
                if (sHeapEntry.wFlags == 0)
                {
                    uiHeapCommittedBytes += sHeapEntry.cbData;
                    uiHeapFreeBytes += sHeapEntry.cbData;

                    uiTotalCommittedBytes += sHeapEntry.cbData;
                    uiTotalFreeBytes += sHeapEntry.cbData;
                }

                // The entry is an uncommitted block of memory
                if (sHeapEntry.wFlags & PROCESS_HEAP_UNCOMMITTED_RANGE)
                {
                    uiTotalUncommittedBytes += sHeapEntry.cbData;
                    uiHeapUncommittedBytes += sHeapEntry.cbData;
                }

                uiHeapTotalBytes += sHeapEntry.cbData;
                uiHeapTotalOverhead += sHeapEntry.cbOverhead;
                uiTotalBytes += sHeapEntry.cbData;
                uiTotalOverhead += sHeapEntry.cbOverhead;
            }

            Console.Dev << _T("Total       ") << (uint)uiHeapTotalBytes << _T(" + overhead of ") << (uint)uiHeapTotalOverhead << newl;
            Console.Dev << _T("Committed   ") << (uint)uiHeapCommittedBytes << newl;
            Console.Dev << _T("Uncommitted ") << (uint)uiHeapUncommittedBytes << newl;
            Console.Dev << _T("Allocated   ") << (uint)uiHeapAllocatedBytes << newl;
            Console.Dev << _T("Free        ") << (uint)uiHeapFreeBytes << newl;
            Console.Dev << newl;
        }
    }

    Console.Dev << newl;
    Console << _T("Total       ") << (uint)uiTotalBytes << newl;
    Console << _T("Committed   ") << (uint)uiTotalCommittedBytes << newl;
    Console << _T("Uncommitted ") << (uint)uiTotalUncommittedBytes << newl;
    Console << _T("Allocated   ") << (uint)uiTotalAllocatedBytes << newl;
    Console << _T("Free        ") << (uint)uiTotalFreeBytes << newl;

    cMemImg.SaveImage(_T("memory.png"));
}


CMD(AnalyzeMemory)
{
    K2System.AnalyzeMemory();
    return true;
}


CMD(TestMicBoost)
{
    if (vArgList.size() < 1)
        return false;

    K2System.SetMicBoost(AtoB(vArgList[0]));

    return true;
}


CMD(TestMicVolume)
{
    if (vArgList.size() < 1)
        return false;

    K2System.SetMicVolume(AtoI(vArgList[0]));

    return true;
}


FUNCTION(GetMicBoost)
{
    return XtoA(K2System.GetMicBoost());
}


FUNCTION(GetMicVolume)
{
    return XtoA(K2System.GetMicVolume());
}
//=============================================================================

#endif  //_WIN32
