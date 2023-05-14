// (C)2005 S2 Games
// c_system.h
//
//=============================================================================
#ifndef __C_SYSTEM_H__
#define __C_SYSTEM_H__

//=============================================================================
// Declarations
//=============================================================================
extern K2_API class CSystem &K2System;

namespace fsw {
    class monitor;
}
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint NUM_SYS_JOYSTICK_AXIS(6);
const uint NUM_JOYSTICK_BUTTONS(32);

// Memory Protection Flags
const uint K2_MEM_PF_EXECUTE                (BIT(0));
const uint K2_MEM_PF_EXECUTE_READ           (BIT(1));
const uint K2_MEM_PF_EXECUTE_READWRITE      (BIT(2));
const uint K2_MEM_PF_EXECUTE_WRITECOPY      (BIT(3));
const uint K2_MEM_PF_NOACCESS               (BIT(4));
const uint K2_MEM_PF_READONLY               (BIT(5));
const uint K2_MEM_PF_READWRITE              (BIT(6));
const uint K2_MEM_PF_WRITECOPY              (BIT(7));
const uint K2_MEM_PF_GUARD                  (BIT(8));
const uint K2_MEM_PF_NOCACHE                (BIT(9));
const uint K2_MEM_PF_WRITECOMBINE           (BIT(10));

enum EMemState
{
    // Indicates committed pages for which physical storage has been
    // allocated, either in memory or in the paging file on disk.
    MEMSTATE_COMMIT,

    // Indicates free pages not accessible to the calling process and
    // available to be allocated. For free pages, the information in
    // the AllocationBase, AllocationProtect, Protect, and Type members
    // is undefined.
    MEMSTATE_FREE,

    // Indicates reserved pages where a range of the process's virtual
    // address space is reserved without any physical storage being
    // allocated. For reserved pages, the information in the Protect
    // member is undefined.
    MEMSTATE_RESERVE
};

enum EMemType
{
    // Indicates that the memory pages within the region are mapped
    // into the view of an image section.
    MEMTYPE_IMAGE,

    // Indicates that the memory pages within the region are mapped
    // into the view of a section.
    MEMTYPE_MAPPED,

    // Indicates that the memory pages within the region are private
    // (that is, not shared by other processes).
    MEMTYPE_PRIVATE
};

enum EDriveType
{
    DRIVETYPE_INVALID,
    DRIVETYPE_REMOVABLE,
    DRIVETYPE_FIXED,
    DRIVETYPE_REMOTE,
    DRIVETYPE_CDROM,
    DRIVETYPE_RAM
};

struct SMemRegion
{
    // A pointer to the base address of the region of pages.
    void*       pBaseAddr;

    // A pointer to the base address of a range of pages allocated
    // by the VirtualAlloc function. The page pointed to by the
    // BaseAddress member is contained within this allocation range.
    void*       pAllocBase;

    // The size of the region beginning at the base address in which
    // all pages have identical attributes, in bytes.
    size_t      uiRegionSize;

    // A combination of MEM_PAGE_* flags (above).
    uint        uiProtectionFlags;

    // The access protection of the pages in the region.
    // This member is one of the values listed for the
    // AllocationProtect member.
    uint        uiProtect;

    // The state of the pages in the region (commit, free, or reserve)
    EMemState   eState;

    // The type of pages in the region (image, mapped, or private)
    EMemType    eType;
};

struct SSysJoystick
{
    float   fAxis[NUM_SYS_JOYSTICK_AXIS];
    uint    uiButtons;
    uint    uiPOV;
};

struct SSysInfo
{
    tstring sMAC;
    tstring sProcessor;
    tstring sRAM;
    tstring sOS;
    tstring sVideo;
};

typedef map<size_t, EButton>    KeyboardMap;
//=============================================================================

//=============================================================================
// CSystem
//=============================================================================
class CSystem
{
    SINGLETON_DEF(CSystem)

private:
    // Game info
    tstring         m_sGameName;
    tstring         m_sVersion;
    tstring         m_sBuildNumber;

    tstring         m_sBuildInfo;
    tstring         m_sBuildOS;
    tstring         m_sBuildOSCode;
    tstring         m_sBuildArch;
    
    string          m_sMasterServerAddress;

#ifdef linux
    char**          m_argv;
#endif

    // System configuration
    int             m_iScreenSaverActive;
    dword           m_dwPageSize;

    // Command line
    tstring         m_sCommandLine;

    // Window management
    void*           m_hInstance;
    void*           m_ConsoleWindowHandle;
    void*           m_WindowHandle;
    void*           m_pfnMainWndProc;
    void*           m_pfnConsoleWndProc;
    bool            m_bRestartProcess;
    size_t          m_hMainThread;

    // File system
    tstring         m_sRootDir;
    tstring         m_sUserDir;

#ifdef linux
    int             m_iInotifyFd;
    map<int,tstring>    m_mapInotifyWdPaths;
#elif defined(WIN32)
    bool            m_bMonitoringActive;
    void*           m_pFileMonitorInfo;
    sset            m_setsModifiedFiles;
#elif defined(__APPLE__)
    fsw::monitor*   m_pFileMonitor = nullptr;
    sset            m_setsModifiedFiles;
#endif

    // Input
    CRecti          m_recMouseClip;
    SSysJoystick    m_JoystickState;
#ifdef __APPLE__
    bool            m_bMouseClip;
    struct          SJoystickInfo;
    map <int, SJoystickInfo*> m_mapJoysticks;
#endif
#ifdef linux
    CVec4i          m_v4MousePos; // position and relative motion (x, y, delta x, delta y)
    list<uint>      m_listMotionEventsIgnore; // to keep track of motion events generated by pointer warping (so that that motion can be ignored)
    typedef pair<tstring, tstring>  jsInfo; //device & name
    map <int, jsInfo>   m_mapJoysticks;
    int             m_hJoystick;
    uint            m_uiJoystickAxes;
    tstring         m_sClipboardString;
    bool            m_bMouseClip;
#ifndef Atom
typedef unsigned long Atom;
#endif
    Atom            m_ClipboardTarget;
#endif

    // Status
    bool            m_bHasFocus;

    LONGLONG        m_llStartTicks;
#ifdef __APPLE__
    LONGLONG        m_llFrequency;
#endif

    bool            m_bDirectoryChangeNotified;

    bool            m_bDedicatedServer;
    bool            m_bServerManager;

    tstring         m_sDedicatedBuffer;
    tstring         m_sConsoleInputLine;
    
    KeyboardMap     m_mapKeyboard;

    tstring         m_sConsoleTitle;

    ULONGLONG       m_ullVirtualMemoryLimit;

    bool            PreventSetUnhandledExceptionFilter();

public:
    ~CSystem();

#ifdef linux
    K2_API void             Init(const tstring &sGameName, const tstring &sVersion, const tstring &sBuildInfo, const tstring &sBuildNumber, const tstring &sBuildOS, const tstring &sBuildOSCode, const tstring &sBuildArch, const string &sMasterServerAddress, int argc, char *argv[]);
#else
    K2_API void             Init(const tstring &sGameName, const tstring &sVersion, const tstring &sBuildInfo, const tstring &sBuildNumber, const tstring &sBuildOS, const tstring &sBuildOSCode, const tstring &sBuildArch, const string &sMasterServerAddress);
#endif
    void                    InitMore(); // init that depends on dedicated server being set/not set

    inline const tstring&   GetGameName() const             { return m_sGameName; }
    inline const tstring&   GetVersionString() const        { return m_sVersion; }
    inline const tstring&   GetBuildNumberString() const    { return m_sBuildNumber; }
    inline const tstring&   GetBuildInfoString() const      { return m_sBuildInfo; }
    inline const tstring&   GetBuildOSString() const        { return m_sBuildOS; }
    inline const tstring&   GetBuildOSCodeString() const    { return m_sBuildOSCode; }
    inline const tstring&   GetBuildArchString() const      { return m_sBuildArch; }
    inline const string&    GetMasterServerAddress() const  { return m_sMasterServerAddress; }

    K2_API void             HandleOSMessages();
    K2_API void             Exit(int iErrorLevel);
    K2_API void             RestartOnExit(bool bRestart)        { m_bRestartProcess = bRestart; }

    K2_API void             DebugOutput(const tstring &sOut);
    K2_API void             Sleep(uint uiMsecs);
    K2_API void             DebugBreak();
    K2_API void             Error(const tstring &sMsg);
    K2_API uint             GetLastError();
    K2_API tstring          GetErrorString(uint err);
    K2_API tstring          GetLastErrorString();
    K2_API const tstring&   GetCommandLine()            { return m_sCommandLine; }

    K2_API void             SetFocus(bool bFocus)       { m_bHasFocus = bFocus; }
    K2_API bool             HasFocus()                  { return m_bHasFocus; }

    // File system
    K2_API bool             MakeDir(const tstring &sPath);
    K2_API bool             DeleteFile();
    K2_API void             SetRootDir(const tstring &sPath)    { m_sRootDir = sPath; }
    K2_API const tstring&   GetRootDir()                        { return m_sRootDir; }
    K2_API const tstring&   GetUserDir()                        { return m_sUserDir; }

    K2_API void             GetDirList(const tstring &sPath, bool bRecurse, tsvector &vDirList, const tstring &sMod);
    K2_API void             GetFileList(const tstring &sPath, const tstring &sFile, bool bRecurse, tsvector &vFileList, const tstring &sMod);
    K2_API void             GetHiddenFileList(const tstring &sPath, const tstring &sFile, bool bRecurse, tsvector &vFileList, const tstring &sMod);

    K2_API void             StartDirectoryMonitoring();
    K2_API void             GetModifiedFileList(tsvector &vFileList);
    K2_API void             FlushDirectoryMonitoring();
    K2_API void             StopDirectoryMonitoring();
#ifdef linux
    bool                    IsDirectoryMonitoring() const                   { return m_iInotifyFd >= 0; }
#elif defined(WIN32)
    bool                    IsDirectoryMonitoring() const                   { return m_bMonitoringActive; }
#elif defined(__APPLE__)
    bool                    IsDirectoryMonitoring() const                   { return m_pFileMonitor != nullptr; }
#endif

#ifdef WIN32
    void*                   GetFileMonitorInfo() const                      { return m_pFileMonitorInfo; }
    void                    SetDirectoryChangeNotified(bool bValue)         { m_bDirectoryChangeNotified = bValue; }
    void                    AddModifiedPath(const tstring &sPath);
#elif defined(__APPLE__)
    void                    AddModifiedPath(const tstring &sPath);
#endif
#ifdef linux
    void                    AddDirectoryWatch(const tstring &sPath);
#endif

    // Timers
    K2_API uint             Milliseconds();
    K2_API uint             Microseconds();
    K2_API LONGLONG         GetTicks() const;
    K2_API LONGLONG         GetFrequency();

    // Dynamic librarys
    K2_API void*            LoadLibrary(const tstring &sLibFilename);
    K2_API void*            GetProcAddress(void *pDll, const tstring &sProcName);
    K2_API bool             FreeLibrary(void *pDll);

    // General process management
    K2_API uiset            GetRunningProcesses();
    K2_API uint             GetProcessID();

    // Windowing
    K2_API void             SetWindowHandle(void *pHandle)  { m_WindowHandle = pHandle; }
    K2_API void*            GetWindowHandle()               { return m_WindowHandle; }
    K2_API CRecti           GetWindowArea();
    K2_API void             GetClientOffset(int *x, int *y);

    K2_API void             SetConsoleWindowHandle(void *pHandle)   { m_ConsoleWindowHandle = pHandle; }
    K2_API void*            GetConsoleWindowHandle()                { return m_ConsoleWindowHandle; }

    K2_API void             SetInstanceHandle(void *pInstance)  { m_hInstance = pInstance; }
    K2_API void*            GetInstanceHandle()                 { return m_hInstance; }

    K2_API void             SetMainWndProc(void *pfnMainWndProc)    { m_pfnMainWndProc = pfnMainWndProc; }
    K2_API void*            GetMainWndProc()                        { return m_pfnMainWndProc; }

    K2_API void             SetConsoleWndProc(void *pfnConsoleWndProc)  { m_pfnConsoleWndProc = pfnConsoleWndProc; }
    K2_API void*            GetConsoleWndProc()                         { return m_pfnConsoleWndProc; }

    K2_API size_t           GetMainThread()                     { return m_hMainThread; }
    K2_API size_t           GetCurrentThread();

    // Mouse
    K2_API void             SetMousePos(int x, int y);
    K2_API CVec2i           GetMousePos();
    K2_API void             ShowMouseCursor();
    K2_API void             HideMouseCursor();
    K2_API void             SetMouseClipping(const CRecti &recClip);
    K2_API void             UnsetMouseClipping();
    K2_API void             PollMouse();

    // Joystick
    K2_API void             PollJoysticks(uint uiID);
    K2_API float            GetJoystickAxis(uint uiID, EAxis eAxis);
    K2_API uint             GetJoystickButtons(uint uiID);
    K2_API uint             GetJoystickPOV(uint uiID);
    K2_API bool             JoystickHasAxis(uint uiID, EAxis axis);
    K2_API void             GetJoystickList(imaps &mapNames);

    // Clipboard
    K2_API void             CopyToClipboard(const tstring &sText);
    K2_API bool             IsClipboardString();
    K2_API tstring          GetClipboardString();

    // Beep
    K2_API void             Beep(dword dwFreq, dword dwDuration);

    K2_API bool             IsDebuggerPresent();

    K2_API bool             IsDedicatedServer()                     { return m_bDedicatedServer; }
    K2_API void             SetDedicatedServer(bool bValue)         { m_bDedicatedServer = bValue; }
    K2_API bool             IsServerManager()                       { return m_bServerManager; }
    K2_API void             SetServerManager(bool bValue)           { m_bServerManager = bValue; }
    
    K2_API void             AddDedicatedConsoleText(const tstring &sText);
    K2_API void             UpdateDedicatedConsoleText();

    // System information
    K2_API dword            GetPageSize()                           { return m_dwPageSize; }
    K2_API SSysInfo         GetSystemInfo();
    K2_API ULONGLONG        GetTotalPhysicalMemory() const;
    K2_API ULONGLONG        GetFreePhysicalMemory() const;
    K2_API ULONGLONG        GetTotalVirtualMemory() const;
    K2_API ULONGLONG        GetFreeVirtualMemory() const;
    K2_API ULONGLONG        GetTotalPageFile() const;
    K2_API ULONGLONG        GetFreePageFile() const;
    K2_API ULONGLONG        GetProcessMemoryUsage() const;
    K2_API ULONGLONG        GetProcessVirtualMemoryUsage() const;
    K2_API ULONGLONG        GetVirtualMemoryLimit() const;

    K2_API void             GetDriveList(tsvector &vNames) const;
    K2_API EDriveType       GetDriveType(const tstring &sName) const;
    K2_API size_t           GetDriveSize(const tstring &sName) const;
    K2_API size_t           GetDriveFreeSpace(const tstring &sName) const;
    K2_API ULONGLONG        GetDriveSizeEx(const tstring &sName) const;
    K2_API ULONGLONG        GetDriveFreeSpaceEx(const tstring &sName) const;

    K2_API void             SetConfig(const tstring &sConfig);
    K2_API void             SetAffinity(int iCPU);
    K2_API uint             GetAffinityMask() const;

    K2_API void             SetPriority(int iPriority);
    K2_API int              GetPriority() const;

    static bool             IsInitialized()                         { return s_pInstance != nullptr; }

    K2_API tstring          GetProcessFilename();
    K2_API tstring          GetProcessBaseName();
    K2_API bool             Shell(const tstring &sCommand, bool bWait = false);

    // This function should return an integer that uniquely identifies a machine across multiple
    // sessions of the game.  It isn't meant to be secure by any means, but to serve the purpose
    // of identifying a client reconnecting to a non-official server, for instance
    // Windows derives this from the MAC address of the first network adapter
    K2_API uint             GetUniqueID() const;

    K2_API void             InitDedicatedConsole();

    K2_API void             InitKeyboardMap();
    K2_API EButton          GetButton(size_t vk, size_t lParam);
    K2_API void             SetupKeystates();

    // Voice functions
#ifdef _WIN32
    K2_API void             SetMicBoost(bool bValue);
    K2_API void             SetMicVolume(float fValue);

    K2_API bool             GetMicBoost();
    K2_API float            GetMicVolume();
#endif
    
    K2_API void             SetTitle(const tstring &sTitle);

    // Memory analysis
    K2_API void             AnalyzeMemory();

    K2_API uint             GetRandomSeed32();
};
//=============================================================================

#endif  //__C_SYSTEM_H__
