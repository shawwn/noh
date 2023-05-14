//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include <ncurses.h>
// undefine these defines from ncurses that conflict with enums in k2
#undef BUTTON_ALT
#undef BUTTON_SHIFT
#undef BUTTON_CTRL

#include "k2_common.h"

#include "c_system.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <glob.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <dirent.h>
#include <sched.h>
#include <spawn.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <linux/joystick.h>
#include <sys/soundcard.h>
#include <dlfcn.h>
#include <ctype.h>

#include "inotify.h"
#include "inotify-syscalls.h"

#include "c_filemanager.h"
#include "c_input.h"
#include "c_cmd.h"
#include "c_vid.h"
#include "c_soundmanager.h"
#include "c_filehttp.h"
#include "c_uicmd.h"
#include "i_widget.h"
#include "i_listwidget.h"
#include "c_voicemanager.h"
#include "s_x11info.h"
//=============================================================================


//=============================================================================
// Definitions
//=============================================================================
extern CCvar<bool>  host_dynamicResReload;

typedef map<KeySym, EButton>    keyboardMap;
keyboardMap                     g_KeyboardMap;

#define MOD_NONE    0
#define MOD_SHIFT   1
#define MOD_CTRL    2
#define MOD_ALT     4
typedef pair<EButton, byte>         escSequence;
typedef map<tstring, escSequence>   escSequenceMap; // button + modifiers
escSequenceMap                      g_KeyboardEscSequenceMap;

typedef unsigned long           Cardinal;

#define RRScreenChangeNotify    0

//=============================================================================

//=============================================================================
// Globals
//=============================================================================
SINGLETON_INIT(CSystem);
CSystem& K2System(*CSystem::GetInstance());

CVAR_BOOLF  (sys_autoSaveConfig,        true,   CVAR_SAVECONFIG);
CVAR_BOOLF  (sys_fileChangeNotify,      true,   CONEL_DEV);

__attribute__((visibility("default")))
CVAR_BOOLF  (sys_grabInput,             false,  CVAR_SAVECONFIG);
CVAR_BOOLF  (sys_dedicatedConsole,      true,   CVAR_SAVECONFIG);
CVAR_BOOL   (sys_interactive,           true);
#if defined(_DEBUG)
CVAR_BOOL   (sys_debugOutput,           true);
#else
CVAR_BOOL   (sys_debugOutput,           false);
#endif

// Keyboard settings
CVAR_BOOLF  (key_splitShift,            false,  CVAR_SAVECONFIG);
CVAR_BOOLF  (key_splitAlt,              false,  CVAR_SAVECONFIG);
CVAR_BOOLF  (key_splitCtrl,             false,  CVAR_SAVECONFIG);
CVAR_BOOLF  (key_splitWin,              false,  CVAR_SAVECONFIG);
CVAR_BOOLF  (key_splitEnter,            false,  CVAR_SAVECONFIG);
CVAR_BOOL   (key_debugEvents,           false);

// Joystick settings
EXTERN_CVAR_INT(input_joyDeviceID);
// track joysticks by name as their device entries can change if they get unplugged/plugged back in
CVAR_STRINGF(sys_joyName,               "",     CVAR_SAVECONFIG);

Atom    XA_WM_PROTOCOLS;
Atom    XA_WM_DELETE_WINDOW;
Atom    XA__NET_WM_PING;
Atom    XA__NET_WM_USER_TIME;
Atom    XA_CLIPBOARD;
Atom    XA_TARGETS;
Atom    XA_UTF8_STRING;

SX11Info g_X11Info = { 0 };

extern char** environ;
//=============================================================================


/*====================
  CSystem::CSystem
  ====================*/
CSystem::CSystem() :
m_hInstance(nullptr),
m_ConsoleWindowHandle(nullptr),
m_WindowHandle(nullptr),
m_bRestartProcess(false),
m_iInotifyFd(-2),
m_bDedicatedServer(false),
m_bServerManager(false),
m_bHasFocus(false),
m_bMouseClip(false),
m_pfnMainWndProc(nullptr),
m_hJoystick(-1),
m_ullVirtualMemoryLimit(0)
{
    // Set system start time so K2System.Milliseconds counts from 0
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    m_llStartTicks = ts.tv_sec * 1000000ll + ts.tv_nsec / 1000ull;

    // Set process affinity
    SetAffinity(0);

    m_hMainThread = GetCurrentThread();
    
    std::ifstream ProcMeminfo("/proc/meminfo");
    
    if (ProcMeminfo.is_open())
    {
        while (!ProcMeminfo.eof())
        {
            char s[256];
            ProcMeminfo.getline(s, 256);
            string sLine(s);
            if (sLine.find("VmallocTotal", 0) != string::npos)
            {
                size_t zStartPos(13);
                while (sLine[zStartPos] && (sLine[zStartPos] >= '9' || sLine[zStartPos] <= '0')) ++zStartPos;
                size_t zEndPos(zStartPos);
                while (sLine[zEndPos] && sLine[zEndPos] <= '9' && sLine[zEndPos] >= '0') ++zEndPos;
                
                string sMem(sLine.substr(zStartPos, zEndPos - zStartPos));
                m_ullVirtualMemoryLimit = atoll(sMem.c_str());
                break;
            }
        }
        
        ProcMeminfo.close();
    }
}


/*====================
  CSystem::Init
  ====================*/
void    CSystem::Init(const tstring &sGameName, const tstring &sVersion, const tstring &sBuildInfo, const tstring &sBuildNumber, const tstring &sBuildOS, const tstring &sBuildOSCode, const tstring &sBuildArch, const string &sMasterServerAddress, int argc, char *argv[])
{
    m_sGameName = sGameName;
    m_sVersion = sVersion;
    m_sBuildInfo = sBuildInfo;
    m_sBuildNumber = sBuildNumber;
    m_sBuildOS = sBuildOS;
    m_sBuildOSCode = sBuildOSCode;
    m_sBuildArch = sBuildArch;
    m_sMasterServerAddress = sMasterServerAddress;
    
    m_argv = argv;

    srand(uint(m_llStartTicks & UINT_MAX));

    // Set the commandline
    m_sCommandLine.erase();
    for (int i(1); i < argc; i++)
        m_sCommandLine = m_sCommandLine + NativeToTString(argv[i]) + _T(" ");

    // Change to the working directory to the location of hon.bin
    // check for it in the user's path if argv[0] is not a relative/absolute path & then cd to that dir
    if (strchr(argv[0], '/') == nullptr)
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

    // Set the root path
    m_sRootDir = NativeToTString(rootDir);
    m_sRootDir += _T("/");
    free(rootDir);

    // Set the user directory (~/.Heroes of Newerth)
    SetConfig(_T(""));

    // Process the base paths after they have both been initially set,
    // because FileManager could behave poorly if these are not yet set.
    m_sRootDir = FileManager.SanitizePath(m_sRootDir, false);
    m_sUserDir = FileManager.SanitizePath(m_sUserDir, false);
    
    m_hInstance = static_cast<void*>(&g_X11Info);
}


/*====================
  CSystem::InitMore
  ====================*/
void CSystem::InitMore()
{
    // Connect to the X server if we are a client
    if (!IsDedicatedServer() && !IsServerManager())
    {
        if (!(g_X11Info.dpy = XOpenDisplay(nullptr)))
            EX_ERROR(_T("Unable to connect to the X server."));

#define INIT_ATOM(x) XA_##x = XInternAtom(g_X11Info.dpy, #x, False)
        INIT_ATOM(WM_PROTOCOLS);
        INIT_ATOM(WM_DELETE_WINDOW);
        INIT_ATOM(_NET_WM_PING);
        INIT_ATOM(_NET_WM_USER_TIME);
        INIT_ATOM(CLIPBOARD);
        INIT_ATOM(TARGETS);
        INIT_ATOM(UTF8_STRING);
#undef INIT_ATOM

        if (getenv("K2_X11_SYNCHRONIZE") && atoi(getenv("K2_X11_SYNCHRONIZE")) > 0)
            XSynchronize(g_X11Info.dpy, True);
        
        // set up res_name and res_class
        // from the ICCCM, section 4.1.2.5:
        // On POSIX-conformant systems, the following conventions are used:
        // * If "-name NAME" is given on the command line, NAME is used as the instance name. -- not using this here as k2 doesn't use this style of arguments
        // * Otherwise, if the environment variable RESOURCE_NAME is set, its value will be used as the instance name.
        // * Otherwise, the trailing part of the name used to invoke the program (argv[0] stripped of any directory names) is used as the instance name.
        char *res_name = getenv("RESOURCE_NAME");
        if (!res_name && (res_name = strrchr(m_argv[0], '/')))
            res_name++;
        else
            res_name = m_argv[0];
        strncpy(g_X11Info.res_name, res_name, 256);
        g_X11Info.res_name[255] = 0;
#ifdef UNICODE
        if (wcstombs(g_X11Info.res_class, m_sGameName.c_str(), 256) == 256)
            g_X11Info.res_class[255] = 0;
#else
        strncpy(g_X11Info.res_class, m_sGameName.c_str(), 256);
        g_X11Info.res_class[255] = 0;
#endif
        
#ifdef UNICODE
        XSetLocaleModifiers("");
        if (!(g_X11Info.im = XOpenIM(g_X11Info.dpy, nullptr, g_X11Info.res_class, g_X11Info.res_name)))
        {
            Console.Warn << _T("CSystem::Init() - Unable to open X11 input method.  Retrying using @im=none") << newl;
            XSetLocaleModifiers("@im=none");
            if (!(g_X11Info.im = XOpenIM(g_X11Info.dpy, nullptr, g_X11Info.res_class, g_X11Info.res_name)))
                Console.Warn << _T("CSystem::Init() - Unable to open X11 input method") << newl;
        }
        // the IC is created in the renderer whenever a window is created
#endif

        // init joystick stuff
        m_mapJoysticks.clear();
        // find all joysticks
        for (int i(0); i < 32; ++i)
        {
            int fd;
            string sDevice;

            // find device
            if ((fd = open((string("/dev/input/js") + XtoS(i)).c_str(), O_RDONLY | O_NONBLOCK)) != -1)
                sDevice = string("/dev/input/js") + XtoS(i);
            else if ((fd = open((string("/dev/js") + XtoS(i)).c_str(), O_RDONLY | O_NONBLOCK)) != -1)
                sDevice = string("/dev/js") + XtoS(i);
            else
                continue;

            // get name
            char name[256] = { 0 };
            ioctl(fd, JSIOCGNAME(sizeof(name)), name);
            Console << _T("Found Joystick: ") << name << newl;
            
            m_mapJoysticks[i] = jsInfo(NativeToTString(sDevice), NativeToTString(name));
            close(fd);
        }

        // set input_joyDeviceID using input_joyName
        if (input_joyDeviceID != -1)
        {
            input_joyDeviceID = -1; // disable if we don't find the device
            for (map<int, jsInfo>::iterator it(m_mapJoysticks.begin()); it != m_mapJoysticks.end(); ++it)
            {
                if (sys_joyName == it->second.second)
                {
                    input_joyDeviceID = it->first;
                    break;
                }
            }
            input_joyDeviceID.SetModified(false);
        }

        // init the joystick
        if (input_joyDeviceID != -1)
        {
            m_hJoystick = open(TStringToNative(m_mapJoysticks[input_joyDeviceID].first).c_str(), O_RDONLY | O_NONBLOCK);
            char axes(0);
            ioctl(m_hJoystick, JSIOCGAXES, &axes);
            m_uiJoystickAxes = MIN(uint(axes), NUM_SYS_JOYSTICK_AXIS);
        }

#define MAP_KEYSYM(keysym, button) \
        { \
            KeySym actualkeysym = XKeycodeToKeysym(g_X11Info.dpy, XKeysymToKeycode(g_X11Info.dpy, keysym), 0); \
            if (key_debugEvents) \
                Console << _T("CSystem::Init() - Mapping ") << INT_HEX_TSTR(keysym) << _T(" ( ") << INT_HEX_TSTR(actualkeysym) << _T(") to ") << INT_HEX_TSTR(button) << newl; \
            actualkeysym = (actualkeysym == NoSymbol)?keysym:actualkeysym; \
            if (g_KeyboardMap.find(actualkeysym) == g_KeyboardMap.end()) \
                g_KeyboardMap[actualkeysym] = button; \
            else \
                Console.Warn << _T("CSystem::Init() - Keysym ") << INT_HEX_TSTR(keysym) << _T(" already mapped. ") << Input.ToString(button) << _T(" unavailable for input.") << newl; \
        }
        
#define MAP_KEYSYM2(keysym, button) \
        { \
            if (key_debugEvents) \
                Console << _T("CSystem::Init() - Mapping ") << INT_HEX_TSTR(keysym) << _T(" to ") << INT_HEX_TSTR(button) << newl; \
            if (g_KeyboardMap.find(keysym) == g_KeyboardMap.end()) \
                g_KeyboardMap[keysym] = button; \
            else \
                Console.Warn << _T("CSystem::Init() - Keysym ") << INT_HEX_TSTR(keysym) << _T(" already mapped. ") << Input.ToString(button) << _T(" unavailable for input.") << newl; \
        }
        
        MAP_KEYSYM(XK_BackSpace,    BUTTON_BACKSPACE)
        MAP_KEYSYM(XK_Tab,          BUTTON_TAB)
        MAP_KEYSYM(XK_Return,       BUTTON_ENTER)
        MAP_KEYSYM(XK_Escape,       BUTTON_ESC)
        MAP_KEYSYM(XK_space,        BUTTON_SPACE)
        
        MAP_KEYSYM(XK_0,            EButton('0'))
        MAP_KEYSYM(XK_1,            EButton('1'))
        MAP_KEYSYM(XK_2,            EButton('2'))
        MAP_KEYSYM(XK_3,            EButton('3'))
        MAP_KEYSYM(XK_4,            EButton('4'))
        MAP_KEYSYM(XK_5,            EButton('5'))
        MAP_KEYSYM(XK_6,            EButton('6'))
        MAP_KEYSYM(XK_7,            EButton('7'))
        MAP_KEYSYM(XK_8,            EButton('8'))
        MAP_KEYSYM(XK_9,            EButton('9'))
        
        MAP_KEYSYM(XK_A,            EButton('A'))
        MAP_KEYSYM(XK_B,            EButton('B'))
        MAP_KEYSYM(XK_C,            EButton('C'))
        MAP_KEYSYM(XK_D,            EButton('D'))
        MAP_KEYSYM(XK_E,            EButton('E'))
        MAP_KEYSYM(XK_F,            EButton('F'))
        MAP_KEYSYM(XK_G,            EButton('G'))
        MAP_KEYSYM(XK_H,            EButton('H'))
        MAP_KEYSYM(XK_I,            EButton('I'))
        MAP_KEYSYM(XK_J,            EButton('J'))
        MAP_KEYSYM(XK_K,            EButton('K'))
        MAP_KEYSYM(XK_L,            EButton('L'))
        MAP_KEYSYM(XK_M,            EButton('M'))
        MAP_KEYSYM(XK_N,            EButton('N'))
        MAP_KEYSYM(XK_O,            EButton('O'))
        MAP_KEYSYM(XK_P,            EButton('P'))
        MAP_KEYSYM(XK_Q,            EButton('Q'))
        MAP_KEYSYM(XK_R,            EButton('R'))
        MAP_KEYSYM(XK_S,            EButton('S'))
        MAP_KEYSYM(XK_T,            EButton('T'))
        MAP_KEYSYM(XK_U,            EButton('U'))
        MAP_KEYSYM(XK_V,            EButton('V'))
        MAP_KEYSYM(XK_W,            EButton('W'))
        MAP_KEYSYM(XK_X,            EButton('X'))
        MAP_KEYSYM(XK_Y,            EButton('Y'))
        MAP_KEYSYM(XK_Z,            EButton('Z'))
        
        MAP_KEYSYM(XK_Caps_Lock,    BUTTON_CAPS_LOCK)
        
        MAP_KEYSYM(XK_F1,           BUTTON_F1)
        MAP_KEYSYM(XK_F2,           BUTTON_F2)
        MAP_KEYSYM(XK_F3,           BUTTON_F3)
        MAP_KEYSYM(XK_F4,           BUTTON_F4)
        MAP_KEYSYM(XK_F5,           BUTTON_F5)
        MAP_KEYSYM(XK_F6,           BUTTON_F6)
        MAP_KEYSYM(XK_F7,           BUTTON_F7)
        MAP_KEYSYM(XK_F8,           BUTTON_F8)
        MAP_KEYSYM(XK_F9,           BUTTON_F9)
        MAP_KEYSYM(XK_F10,          BUTTON_F10)
        MAP_KEYSYM(XK_F11,          BUTTON_F11)
        MAP_KEYSYM(XK_F12,          BUTTON_F12)
        
        MAP_KEYSYM(XK_Shift_L,      BUTTON_LSHIFT)
        MAP_KEYSYM(XK_Shift_R,      BUTTON_RSHIFT)
        
        MAP_KEYSYM(XK_Control_L,    BUTTON_LCTRL)
        MAP_KEYSYM(XK_Control_R,    BUTTON_RCTRL)
        
        MAP_KEYSYM(XK_Alt_L,        BUTTON_LALT)
        MAP_KEYSYM(XK_Alt_R,        BUTTON_RALT)
        
        MAP_KEYSYM(XK_Super_L,      BUTTON_LWIN)
        MAP_KEYSYM(XK_Super_R,      BUTTON_RWIN)
        
        MAP_KEYSYM(XK_Menu,         BUTTON_MENU)
        
        MAP_KEYSYM(XK_Up,           BUTTON_UP)
        MAP_KEYSYM(XK_Left,         BUTTON_LEFT)
        MAP_KEYSYM(XK_Down,         BUTTON_DOWN)
        MAP_KEYSYM(XK_Right,        BUTTON_RIGHT)
        
        MAP_KEYSYM(XK_Insert,       BUTTON_INS)
        MAP_KEYSYM(XK_Delete,       BUTTON_DEL)
        MAP_KEYSYM(XK_Home,         BUTTON_HOME)
        MAP_KEYSYM(XK_End,          BUTTON_END)
        MAP_KEYSYM(XK_Page_Up,      BUTTON_PGUP)
        MAP_KEYSYM(XK_Page_Down,    BUTTON_PGDN)
        
        MAP_KEYSYM(XK_Print,        BUTTON_PRINTSCREEN)
        MAP_KEYSYM(XK_Scroll_Lock,  BUTTON_SCROLL_LOCK)
        MAP_KEYSYM(XK_Pause,        BUTTON_PAUSE)
        
        MAP_KEYSYM2(XK_Num_Lock,    BUTTON_NUM_LOCK)
        MAP_KEYSYM2(XK_KP_Divide,   BUTTON_DIVIDE)
        MAP_KEYSYM2(XK_KP_Multiply, BUTTON_MULTIPLY)
        MAP_KEYSYM2(XK_KP_Add,      BUTTON_ADD)
        MAP_KEYSYM2(XK_KP_Subtract, BUTTON_SUBTRACT)
        MAP_KEYSYM2(XK_KP_Decimal,  BUTTON_DECIMAL)
        MAP_KEYSYM2(XK_KP_Delete,   BUTTON_DECIMAL)
        MAP_KEYSYM2(XK_KP_0,        BUTTON_NUM0)
        MAP_KEYSYM2(XK_KP_Insert,   BUTTON_NUM0)
        MAP_KEYSYM2(XK_KP_1,        BUTTON_NUM1)
        MAP_KEYSYM2(XK_KP_End,      BUTTON_NUM1)
        MAP_KEYSYM2(XK_KP_2,        BUTTON_NUM2)
        MAP_KEYSYM2(XK_KP_Down,     BUTTON_NUM2)
        MAP_KEYSYM2(XK_KP_3,        BUTTON_NUM3)
        MAP_KEYSYM2(XK_KP_Page_Down,BUTTON_NUM3)
        MAP_KEYSYM2(XK_KP_4,        BUTTON_NUM4)
        MAP_KEYSYM2(XK_KP_Left,     BUTTON_NUM4)
        MAP_KEYSYM2(XK_KP_5,        BUTTON_NUM5)
        MAP_KEYSYM2(XK_KP_Begin,    BUTTON_NUM5)
        MAP_KEYSYM2(XK_KP_6,        BUTTON_NUM6)
        MAP_KEYSYM2(XK_KP_Right,    BUTTON_NUM6)
        MAP_KEYSYM2(XK_KP_7,        BUTTON_NUM7)
        MAP_KEYSYM2(XK_KP_Home,     BUTTON_NUM7)
        MAP_KEYSYM2(XK_KP_8,        BUTTON_NUM8)
        MAP_KEYSYM2(XK_KP_Up,       BUTTON_NUM8)
        MAP_KEYSYM2(XK_KP_9,        BUTTON_NUM9)
        MAP_KEYSYM2(XK_KP_Page_Up,  BUTTON_NUM9)
        MAP_KEYSYM2(XK_KP_Enter,    BUTTON_NUM_ENTER)
        
        MAP_KEYSYM(XK_semicolon,    BUTTON_MISC1)
        MAP_KEYSYM(XK_slash,        BUTTON_MISC2)
        MAP_KEYSYM(XK_grave,        BUTTON_MISC3)
        MAP_KEYSYM(XK_bracketleft,  BUTTON_MISC4)
        MAP_KEYSYM(XK_backslash,    BUTTON_MISC5)
        MAP_KEYSYM(XK_bracketright, BUTTON_MISC6)
        MAP_KEYSYM(XK_apostrophe,   BUTTON_MISC7)
        
        MAP_KEYSYM(XK_equal,        BUTTON_PLUS)
        MAP_KEYSYM(XK_minus,        BUTTON_MINUS)
        MAP_KEYSYM(XK_comma,        BUTTON_COMMA)
        MAP_KEYSYM(XK_period,       BUTTON_PERIOD)
        
        g_KeyboardMap[XK_ISO_Level3_Shift] = BUTTON_RALT; // Alt Gr
        g_KeyboardMap[XK_Mode_switch] = BUTTON_RALT; // some keyboard layouts have this for right alt
    }
}


/*====================
  CSystem::~CSystem
  ====================*/
CSystem::~CSystem()
{
    if (m_pfnMainWndProc)
        free(m_pfnMainWndProc);
}


inline static EButton KeyCodeToButton(Display* pDisplay, int iCode)
{
    KeySym sym(XKeycodeToKeysym(pDisplay, iCode, 0));
    EButton button(BUTTON_INVALID);

    if (sym >= 'a' && sym <= 'z')
        button = static_cast<EButton>(sym & 0xdf); // convert to uppercase to match the windows client
    else if (sym >= '0' && sym <= '9')
        button = static_cast<EButton>(sym);
    else
    {
        keyboardMap::iterator findit = g_KeyboardMap.find(sym);
        if (findit != g_KeyboardMap.end())
        {
            button = findit->second;

            // handle non-split shift, etc
            switch (button)
            {
                case BUTTON_LSHIFT:
                case BUTTON_RSHIFT:
                    if (!key_splitShift)
                        button = BUTTON_SHIFT;
                    break;
                case BUTTON_LCTRL:
                case BUTTON_RCTRL:
                    if (!key_splitCtrl)
                        button = BUTTON_CTRL;
                    break;
                case BUTTON_LALT:
                case BUTTON_RALT:
                    if (!key_splitAlt)
                        button = BUTTON_ALT;
                    break;
                case BUTTON_LWIN:
                case BUTTON_RWIN:
                    if (!key_splitWin)
                        button = BUTTON_WIN;
                    break;
                case BUTTON_NUM_ENTER:
                    if (!key_splitEnter)
                        button = BUTTON_ENTER;
                    break;
            }
        }
    }

    return button;
}


inline static EButton XButtonToButton(Display* pDisplay, int iButton)
{
    EButton button(BUTTON_INVALID);
    switch (iButton)
    {
        case 1:
            button = BUTTON_MOUSEL;
            break;
        case 2:
            button = BUTTON_MOUSEM;
            break;
        case 3:
            button = BUTTON_MOUSER;
            break;
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
            button = EButton(BUTTON_NUM_ENTER + iButton);
            break;
    }
    return button;
}


inline static bool ProcessCharEvent(int c)
{
    EButton button(BUTTON_INVALID);
    if (key_debugEvents)
        Console << _T("Key: ") << XtoA(c) << newl;
    if (c >= 'a' && c <= 'z')
    {
        button = static_cast<EButton>(c & 0xdf); // convert to uppercase to match the windows client
    }
    else if ((c >= '0' && c <= '9') || (c >= 'A' &&  c <= 'Z'))
    {
        button = static_cast<EButton>(c);
    }
    else
    {
        keyboardMap::iterator findit = g_KeyboardMap.find(c);
        if (findit != g_KeyboardMap.end())
            button = findit->second;
    }
    if (button != BUTTON_INVALID)
    {
        Input.AddEvent(button, true);
        Input.AddEvent(button, false);
    }
    if (c >= 0x20 && c < 0x7f)
        Input.AddEvent(c);
    if (c == KEY_RESIZE)
        return true;
    return false;
}


// handles a subset of VT100 style escape sequences that follow the following pattern:
//  <ESC>[pn   where p is in [0-9,;] and n is in [@-~]
// this should cover all that we need
inline static bool HandleEscapeSequence()
{
    tstring sSeq;
    bool bSequenceFound(false);
    bool bGeometryChanged(false);
    int c = getch();

    if (c != '[')
    {
        ProcessCharEvent(27); //esc
        if (c != ERR)
            bGeometryChanged |= ProcessCharEvent(c);

        return bGeometryChanged;
    }

    sSeq += TCHAR(c);

    while ((c = getch()) != ERR)
    {
        sSeq += TCHAR(c);
        if (c >= '@' && c <= '~') // end of escape sequence
        {
            bSequenceFound = true;
            break;
        }
        else if ((c < '0' || c > '9') && c != ';') // not a sequence we are looking for
            break;
    }

    if (!bSequenceFound)
    {
        ProcessCharEvent(27);
        for (int i(0); i < sSeq.length(); ++i)
        {
            bGeometryChanged |= ProcessCharEvent(sSeq[i]);
        }
    }
    else
    {
        if (key_debugEvents)
            Console << _T("Escape sequence: ") << sSeq << newl;
        escSequenceMap::iterator itFind = g_KeyboardEscSequenceMap.find(sSeq);
        if (itFind != g_KeyboardEscSequenceMap.end())
        {
            if (itFind->second.second & MOD_SHIFT)
            {
                Input.AddEvent(BUTTON_SHIFT, true);
                Input.AddEvent(itFind->second.first, true);
                Input.AddEvent(itFind->second.first, false);
                Input.AddEvent(BUTTON_SHIFT, false);
            }
            else if (itFind->second.second & MOD_CTRL)
            {
                Input.AddEvent(BUTTON_CTRL, true);
                Input.AddEvent(itFind->second.first, true);
                Input.AddEvent(itFind->second.first, false);
                Input.AddEvent(BUTTON_CTRL, false);
            }
            else if (itFind->second.second & MOD_ALT)
            {
                Input.AddEvent(BUTTON_ALT, true);
                Input.AddEvent(itFind->second.first, true);
                Input.AddEvent(itFind->second.first, false);
                Input.AddEvent(BUTTON_ALT, false);
            }
            else
            {
                Input.AddEvent(itFind->second.first, true);
                Input.AddEvent(itFind->second.first, false);
            }
        }
    }

    return bGeometryChanged;
}


/*====================
  CSystem::HandleOSMessages
  ====================*/
void    CSystem::HandleOSMessages()
{
    if (IsDedicatedServer() || IsServerManager())
    {
        if (sys_dedicatedConsole && m_ConsoleWindowHandle)
        {
            CCursesConsole *pConsole(static_cast<CCursesConsole*>(m_ConsoleWindowHandle));
            // handle input
            int c;
            bool bGeometryChanged(false);
            while ((c = getch()) != ERR)
            {
                if (c == 27)
                    bGeometryChanged |= HandleEscapeSequence();
                else
                    bGeometryChanged |= ProcessCharEvent(c);
            }

            if (bGeometryChanged)
                pConsole->SetGeometryChanged(true);
        }
        return;
    }

    if (!g_X11Info.dpy)
        return;

    /* Handles all input, etc as it is all event driven under X11 */
    XEvent e;
    int iPending = XPending(g_X11Info.dpy);

    m_v4MousePos.w = m_v4MousePos.z = 0;

    bool bWarpMouse(false);
    Cardinal UserInteractionTimestamp(0);
    for (int i(0); i < iPending; ++i)
    {
        XNextEvent(g_X11Info.dpy, &e);
        if (XFilterEvent(&e, g_X11Info.win)) // so dead key composition works
            continue;
        switch (e.type)
        {
            case KeyPress:
            case KeyRelease: {
                UserInteractionTimestamp = e.xkey.time;
                if (key_debugEvents)
                    Console << _T("Keycode: ") << INT_HEX_STR(e.xkey.keycode) << _T(", Keysym: ") << INT_HEX_STR(XLookupKeysym(&e.xkey, 0)) << (e.type == KeyPress ? _T(" Pressed") : _T(" Released")) << newl;


                EButton button(KeyCodeToButton(g_X11Info.dpy, e.xkey.keycode));
                if (sys_grabInput && e.type == KeyPress && button == BUTTON_TAB && e.xkey.state & Mod1Mask)
                {
                    // alt-tab: iconify window
                    XUngrabPointer(g_X11Info.dpy, CurrentTime);
                    XUngrabKeyboard(g_X11Info.dpy, CurrentTime);
                    Window Win = *static_cast<Window*>(Vid.GetHWnd());
                    XIconifyWindow(g_X11Info.dpy, Win, DefaultScreen(g_X11Info.dpy));
                    break;
                    // pass on event
                    //XSendEvent(g_X11Info.dpy, InputFocus, True, e.type == KeyPress ? KeyPressMask : KeyReleaseMask, &e);
                }

                if (e.type == KeyRelease)
                {
                    if (i + 1 < iPending)
                    {
                        XEvent peek;
                        XPeekEvent(g_X11Info.dpy, &peek);
                        if (peek.type == KeyPress && peek.xkey.keycode == e.xkey.keycode && peek.xkey.time == e.xkey.time) // not sure if the time will always be the same
                            break; // repeat event -- throw away the key release
                    }
                }

                if (button != BUTTON_INVALID)
                    Input.AddEvent(button, e.type == KeyPress);

                int chars_count;
#ifdef UNICODE
                if (e.type == KeyPress && g_X11Info.ic)
                {
                    wchar_t wc[32] = { 0 };
                    int status;
                    if ((chars_count = XwcLookupString(g_X11Info.ic, &e.xkey, wc, 32, nullptr, &status)) > 0
                            && (status == XLookupChars || status == XLookupBoth))
                    {
                        if (key_debugEvents)
                            Console << _T("Characters: ") << wc << newl;
    
                        for (int i(0); i < chars_count; ++i)
                        {
                            if ((wc[i] >= 0x00 && wc[i] < 0x20) || (wc[i] >= 0x7f && wc[i] <= 0x9f)) // filter out control chars
                                continue;
                            Input.AddEvent(wc[i]);
                        }
                    }
                }
                else
#endif
                if (e.type == KeyPress)
                {
                    char c[32];
                    if ((chars_count = XLookupString(&e.xkey, c, 32, nullptr, nullptr)) > 0)
                    {
                        if (key_debugEvents)
                            Console << _T("Characters: ") << c << newl;
    
                        for (int i(0); i < chars_count; ++i)
                        {
                            if ((c[i] >= 0x00 && c[i] < 0x20) || c[i] == 0x7f || c[i] <= -0x60) // filter out control chars
                                continue;
                            Input.AddEvent(TCHAR(c[i]));
                        }
                    }
                }

            } break;

            case ButtonPress:
            case ButtonRelease: {
                UserInteractionTimestamp = e.xbutton.time;
                if (key_debugEvents)
                    Console << _T("Button") << XtoA(e.xbutton.button) << _T(": ") << (e.type == ButtonPress ? _T("Pressed") : _T("Released")) << newl;

                EButton button(XButtonToButton(g_X11Info.dpy, e.xbutton.button));
                int x(e.xbutton.x);
                int y(e.xbutton.y);
                if (m_bMouseClip && m_bHasFocus && !m_recMouseClip.Contains(CVec2i(x, y)))
                {
                    x = CLAMP(x, m_recMouseClip.left, m_recMouseClip.right);
                    y = CLAMP(y, m_recMouseClip.top, m_recMouseClip.bottom);
                }
                if (button != BUTTON_INVALID)
                    Input.AddEvent(button, e.type == ButtonPress, CVec2f(x, y));
            } break;

            case MotionNotify: {
                UserInteractionTimestamp = e.xmotion.time;
                bool bIgnoreRelativeMotion(false);
                if (!m_listMotionEventsIgnore.empty())
                {
                    for (list<uint>::iterator it(m_listMotionEventsIgnore.begin()); it != m_listMotionEventsIgnore.end(); ++it)
                    {
                        if (*it == ((e.xmotion.x << 16) + e.xmotion.y))
                        {
                            bIgnoreRelativeMotion = true;
                            m_listMotionEventsIgnore.erase(it);
                            break;
                        }
                    }
                }
                if (!bIgnoreRelativeMotion)
                {
                    m_v4MousePos.w += e.xmotion.x - m_v4MousePos.x;
                    m_v4MousePos.z += e.xmotion.y - m_v4MousePos.y;
                }
                m_v4MousePos.x = e.xmotion.x;
                m_v4MousePos.y = e.xmotion.y;
                if (m_bMouseClip && m_bHasFocus && !m_recMouseClip.Contains(CVec2i(m_v4MousePos.x, m_v4MousePos.y)))
                {
                    m_v4MousePos.x = CLAMP(m_v4MousePos.x, m_recMouseClip.left, m_recMouseClip.right);
                    m_v4MousePos.y = CLAMP(m_v4MousePos.y, m_recMouseClip.top, m_recMouseClip.bottom);
                    bWarpMouse = true;
                }
            } break;

            case EnterNotify: {
            } break;
            case MapNotify:
            case FocusIn: {
                SetFocus(true);
                Window Win = *static_cast<Window*>(Vid.GetHWnd());
                if (sys_grabInput)
                {
                    XGrabPointer(g_X11Info.dpy, Win, True, 0, GrabModeAsync, GrabModeAsync, Win, None, CurrentTime);
                    XGrabKeyboard(g_X11Info.dpy, Win, True, GrabModeAsync, GrabModeAsync, CurrentTime);
                }
                else if (m_bMouseClip)
                {
                    XGrabPointer(g_X11Info.dpy, Win, True, 0, GrabModeAsync, GrabModeAsync, Win, None, CurrentTime);
                }

                // update the status of the keyboard buttons
                char keys[32];
                XQueryKeymap(g_X11Info.dpy, keys);
                for (int i(8); i < 32*8; i++) // valid keycodes are 8-255
                {
                    EButton button(KeyCodeToButton(g_X11Info.dpy, i));
                    bool bState(keys[i/8] & (1 << (i % 8)));
                    if (button == BUTTON_INVALID)
                        continue;

                    if (bState != Input.IsButtonDown(button))
                        Input.SetButton(button, bState);
                }

                // update the status of the mouse buttons we can query
                Window DummyWin;
                int iDummy;
                unsigned int uiMask;
                if (XQueryPointer(g_X11Info.dpy, Win, &DummyWin, &DummyWin, &iDummy, &iDummy, &iDummy, &iDummy, &uiMask))
                {
                    for (int i(0); i < 5; ++i)
                    {
                        EButton button(XButtonToButton(g_X11Info.dpy, i + Button1));
                        if (button == BUTTON_INVALID)
                            continue;

                        bool bState(uiMask & (Button1Mask << i));

                        if (bState != Input.IsButtonDown(button))
                            Input.SetButton(button, bState);
                    }
                }
            } break;

            case LeaveNotify: {
            } break;
            case UnmapNotify:
            case FocusOut: {
                SetFocus(false);
                XUngrabPointer(g_X11Info.dpy, CurrentTime);
                XUngrabKeyboard(g_X11Info.dpy, CurrentTime);
            } break;

            case SelectionRequest: {
                Atom reply_property = e.xselectionrequest.property == None ? e.xselectionrequest.target : e.xselectionrequest.property;
                XEvent reply;
                reply.xselection.type = SelectionNotify;
                reply.xselection.requestor = e.xselectionrequest.requestor;
                reply.xselection.selection = e.xselectionrequest.selection;
                reply.xselection.target = e.xselectionrequest.target;
                reply.xselection.property = None;
                reply.xselection.time = e.xselectionrequest.time;

                // will want to handle XA_UTF8_STRING in a unicode enabled build
                if (e.xselectionrequest.target == XA_TARGETS)
                {
                    Atom targets[] = { XA_TARGETS,
#ifdef UNICODE
                            XA_UTF8_STRING,
#endif
                            XA_STRING };
                    reply.xselection.property = reply_property;
                    XChangeProperty(g_X11Info.dpy, e.xselectionrequest.requestor, reply_property, XA_ATOM, 32, PropModeReplace, (unsigned char*)targets, sizeof(targets) / sizeof(Atom));
                }
#ifdef UNICODE
                else if (e.xselectionrequest.target == XA_UTF8_STRING)
                {
                    reply.xselection.property = reply_property;
                    string sUTF8(WStringToUTF8(m_sClipboardString));
                    XChangeProperty(g_X11Info.dpy, e.xselectionrequest.requestor, reply_property, e.xselectionrequest.target, 8, PropModeReplace, (unsigned char*)sUTF8.c_str(), sUTF8.size());
                }
#endif
                else if (e.xselectionrequest.target == XA_STRING)
                {
                    reply.xselection.property = reply_property;
                    XChangeProperty(g_X11Info.dpy, e.xselectionrequest.requestor, reply_property, e.xselectionrequest.target, 8, PropModeReplace, (unsigned char*)TStringToNative(m_sClipboardString).c_str(), m_sClipboardString.size());
                }

                XSendEvent(g_X11Info.dpy, e.xselectionrequest.requestor, True, 0, &reply);
            } break;

            case ClientMessage: {
                if (e.xclient.message_type == XA_WM_PROTOCOLS)
                {
                    if (e.xclient.data.l[0] == XA_WM_DELETE_WINDOW)
                    {
                        Exit(0);
                    }
                    if (e.xclient.data.l[0] == XA__NET_WM_PING)
                    {
                        Window root = RootWindow(g_X11Info.dpy, DefaultScreen(g_X11Info.dpy));
                        e.xclient.window = root;
                        XSendEvent(g_X11Info.dpy, root, False, SubstructureNotifyMask | SubstructureRedirectMask, &e);
                    }
                }
            } break;

            /*case UnmapNotify: {
                if (sys_grabInput)
                {
                    XUngrabPointer(g_X11Info.dpy, CurrentTime);
                    XUngrabKeyboard(g_X11Info.dpy, CurrentTime);
                }
            } break;
            case MapNotify: {
                if (sys_grabInput)
                {
                    Window Win = *static_cast<Window*>(Vid.GetHWnd());
                    XGrabPointer(g_X11Info.dpy, Win, True, 0, GrabModeAsync, GrabModeAsync, Win, None, CurrentTime);
                    XGrabKeyboard(g_X11Info.dpy, Win, True, GrabModeAsync, GrabModeAsync, CurrentTime);
                }
            } // fall through*/
            case DestroyNotify:
            case ReparentNotify:
            case ConfigureNotify:
            case PropertyNotify:
                Vid.Notify(VID_NOTIFY_X11_EVENT, 0, 0, 0, static_cast<void*>(&e));
                break;

            default:
                Console.Dev << _T("Unhandled X11 Event: ") << XtoA(e.type) << newl;
        }
    }

    if (sys_grabInput.IsModified())
    {
        if (sys_grabInput && m_bHasFocus)
        {
            Window Win = *static_cast<Window*>(Vid.GetHWnd());
            XGrabPointer(g_X11Info.dpy, Win, True, 0, GrabModeAsync, GrabModeAsync, Win, None, CurrentTime);
            XGrabKeyboard(g_X11Info.dpy, Win, True, GrabModeAsync, GrabModeAsync, CurrentTime);
        }
        else
        {
            XUngrabPointer(g_X11Info.dpy, CurrentTime);
            XUngrabKeyboard(g_X11Info.dpy, CurrentTime);
        }
        sys_grabInput.SetModified(false);
    }

    if (bWarpMouse)
    {
        SetMousePos(m_v4MousePos.x, m_v4MousePos.y);
        XFlush(g_X11Info.dpy);
    }

    // timestamp of last user interaction
    if (UserInteractionTimestamp)
    {
        XChangeProperty(g_X11Info.dpy, g_X11Info.win, XA__NET_WM_USER_TIME, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)&UserInteractionTimestamp, 1);
    }
}


/*====================
  CSystem::Exit
  ====================*/
void    CSystem::Exit(int iErrorLevel)
{
#ifndef _S2_EXPORTER
    if (sys_autoSaveConfig && !Host.GetNoConfig())
    {
        tsvector vArgs;
        vArgs.push_back(_T("~/startup.cfg"));

        CConsoleElement *pElem = ConsoleRegistry.GetElement(_T("WriteConfigScript"));
        if (pElem != nullptr)
            pElem->Execute(vArgs);
    }

    if (m_ConsoleWindowHandle)
    {
        CCursesConsole *pConsole(static_cast<CCursesConsole*>(m_ConsoleWindowHandle));
        K2_DELETE(pConsole);
    }

    K2SoundManager.Stop();
    cURL_Shutdown();
    Vid.Shutdown();

    if (g_X11Info.dpy)
    {
        XCloseDisplay(g_X11Info.dpy);
        g_X11Info.dpy = nullptr;
    }

    Host.Shutdown();
    Console.FlushLogs();

    if (m_bRestartProcess)
    {
#ifdef DEBUG
#ifdef __x86_64__
        m_argv[0] = "./hon_update_debug-x86_64";
#else
        m_argv[0] = "./hon_update_debug-x86";
#endif
#else
#ifdef __x86_64__
        m_argv[0] = "./hon_update-x86_64";
#else
        m_argv[0] = "./hon_update-x86";
#endif
#endif
        execv(m_argv[0], m_argv);
    }
#endif

    exit(iErrorLevel);
}

/*====================
  CSystem::DebugOutput
  ====================*/
void    CSystem::DebugOutput(const tstring &sOut)
{
    if (!sys_debugOutput)
        return;
#ifdef UNICODE
    wprintf(_T("%ls"), sOut.c_str());
#else
    printf("%s", sOut.c_str());
#endif
}


/*====================
  CSystem::GetLastError
  ====================*/
uint    CSystem::GetLastError()
{
    return errno;
}


/*====================
  CSystem::GetErrorString
  ====================*/
tstring CSystem::GetErrorString(uint err)
{
    return NativeToTString(strerror(err));
}


/*====================
  CSystem::GetLastErrorString
  ====================*/
tstring CSystem::GetLastErrorString()
{
    return GetErrorString(errno);
}


/*====================
  CSystem::StartDirectoryMonitoring
  ====================*/
void    CSystem::StartDirectoryMonitoring()
{
    if (m_iInotifyFd > -2  || !host_dynamicResReload) // already initialized otherwise
        return;

    if ((m_iInotifyFd = inotify_init()) == -1)
    {
        Console.Warn << _T("Failed to initialize inotify. Dynamic resource reloading will not work.") << newl;
        return;
    }

    int iFlags = fcntl(m_iInotifyFd, F_GETFL);
    fcntl(m_iInotifyFd, F_SETFL, iFlags | O_NONBLOCK);

    AddDirectoryWatch(m_sRootDir);
}


/*====================
  CSystem::GetModifiedFileList
  ====================*/
void    CSystem::GetModifiedFileList(tsvector &vFileList)
{
    if (m_iInotifyFd < 0)
        return;

    if (!host_dynamicResReload)
    {
        StopDirectoryMonitoring();
        return;
    }

    // Will handle ~ 128 file/dir changes per frame
    char buf[128*(sizeof(inotify_event)+16)];
    inotify_event *pEvent;

    int iLength, i = 0;

    if ((iLength = read(m_iInotifyFd, buf, 128*(sizeof(inotify_event)+16))) > 0)
    {
        while (i < iLength)
        {
            pEvent = (inotify_event*)&buf[i];
            if (pEvent->mask & IN_ISDIR)
            {
                if (pEvent->mask & IN_CREATE || pEvent->mask & IN_MOVED_TO)
                {
                    AddDirectoryWatch(m_mapInotifyWdPaths[pEvent->wd]+NativeToTString(pEvent->name));
                }
            }
            else if (!(pEvent->mask & IN_CREATE))
            {
                tstring sFileName(m_mapInotifyWdPaths[pEvent->wd]+NativeToTString(pEvent->name));
                if (sFileName.length() > m_sRootDir.length())
                {
                    size_t zPos(sFileName.find_first_of(_T("/"), m_sRootDir.length()));
                    if (zPos != tstring::npos)
                        sFileName = sFileName.substr(zPos);
                    vFileList.push_back(sFileName);
                }
            }
            i += sizeof(inotify_event) + pEvent->len;

            if (sys_fileChangeNotify)
            {
                Console.Dev << _T("DirectoryChangeNotify() - File: ") << m_mapInotifyWdPaths[pEvent->wd] << pEvent->name;
                if (pEvent->mask & IN_CLOSE_WRITE)
                    Console.Dev << _T(" CLOSE_WRITE") << newl;
                else if (pEvent->mask & IN_CREATE)
                    Console.Dev << _T(" CREATED") << newl;
                else if (pEvent->mask & IN_MODIFY)
                    Console.Dev << _T(" MODIFIED") << newl;
                else if (pEvent->mask & IN_DELETE)
                    Console.Dev << _T(" DELETED") << newl;
                else if (pEvent->mask & IN_MOVED_TO)
                    Console.Dev << _T(" MOVED TO") << newl;
                else if (pEvent->mask & IN_MOVED_FROM)
                    Console.Dev << _T(" MOVED FROM") << newl;
            }
        }
    }
}


/*====================
  CSystem::StopDirectoryMonitoring
  ====================*/
void    CSystem::StopDirectoryMonitoring()
{
    if (m_iInotifyFd >= 0)
        close(m_iInotifyFd);
    m_iInotifyFd = -2;
    m_mapInotifyWdPaths.clear();
}


inline bool IsDir(const char* path)
{
    struct stat s;

    stat(path, &s);

    return S_ISDIR(s.st_mode);
}


/*====================
  CSystem::AddDirectoryWatch
  ====================*/
void    CSystem::AddDirectoryWatch(const tstring &sPath)
{
    int wd;

    if ((wd = inotify_add_watch(m_iInotifyFd, TStringToNative(sPath).c_str(), IN_CREATE | IN_CLOSE_WRITE | IN_MOVED_TO | IN_MOVED_FROM | IN_DELETE)) == -1)
        return;

    m_mapInotifyWdPaths[wd] = FileManager.SanitizePath(sPath + _T("/"), false);

    svector vDirList;

    glob_t globbuf;
    char *pName;
    tstring sSearch(m_mapInotifyWdPaths[wd] + _T("*"));

    glob(TStringToNative(sSearch).c_str(), GLOB_ONLYDIR, nullptr, &globbuf);

    for (uint i(0); i < globbuf.gl_pathc; i++)
    {
        if ((pName = strrchr(globbuf.gl_pathv[i], '/')))
            pName++;
        else
            pName = globbuf.gl_pathv[i];
        if (pName[0] != '.' && strcmp(pName, "CVS") && IsDir(globbuf.gl_pathv[i]))
        {
            AddDirectoryWatch(NativeToTString(globbuf.gl_pathv[i]));
        }
    }

    globfree(&globbuf);
}


/*====================
  CSystem::Error
  ====================*/
void    CSystem::Error(const tstring &sMsg)
{
#ifdef UNICODE
    fprintf(stderr, "K2 - Fatal Error: %ls\n", sMsg.c_str());
#else
    fprintf(stderr, "K2 - Fatal Error: %s\n", TStringToNative(sMsg).c_str());
#endif
    Exit(-1);
}


/*====================
  CSystem::Milliseconds
  ====================*/
uint    CSystem::Milliseconds()
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return uint((ts.tv_sec * 1000ull + ts.tv_nsec / 1000000ull)-m_llStartTicks / 1000ull);
}


/*====================
  CSystem::Microseconds
  ====================*/
uint    CSystem::Microseconds()
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return uint((ts.tv_sec * 1000000ull + ts.tv_nsec / 1000ull)-m_llStartTicks);
}


/*====================
  CSystem::GetTicks
  ====================*/
LONGLONG    CSystem::GetTicks() const
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (ts.tv_sec * 100000000ull + ts.tv_nsec / 10ull);
}


/*====================
  CSystem::GetFrequency
  ====================*/
LONGLONG    CSystem::GetFrequency()
{
    return 100000000;
}


/*====================
  CSystem::GetWindowArea
  ====================*/
CRecti  CSystem::GetWindowArea()
{
    SVidMode mode;
    Vid.GetCurrentMode(&mode);
    return CRecti(0, 0, mode.iWidth, mode.iHeight);
}


/*====================
  CSystem::SetMousePos
  ====================*/
void    CSystem::SetMousePos(int x, int y)
{
    if (!g_X11Info.dpy || !m_bHasFocus)
        return;

    Window Win = *static_cast<Window*>(Vid.GetHWnd());
    XWarpPointer(g_X11Info.dpy, None, Win, 0, 0, 0, 0, x, y);
    m_listMotionEventsIgnore.push_back((x << 16) + y);
    m_v4MousePos.x = x;
    m_v4MousePos.y = y;
}


/*====================
  CSystem::GetMousePos
  ====================*/
CVec2i  CSystem::GetMousePos()
{
    if (!g_X11Info.dpy)
        return CVec2i(0, 0);

    Window Win = *static_cast<Window*>(Vid.GetHWnd());
    Window child_return, root_return;
    int root_x, root_y, x, y;
    unsigned int mask;
    XQueryPointer(g_X11Info.dpy, Win, &root_return, &child_return, &root_x, &root_y, &x, &y, &mask);

    return CVec2i(x, y);
}


/*====================
  CSystem::ShowMouseCursor
  ====================*/
void    CSystem::ShowMouseCursor()
{
    if (!g_X11Info.dpy)
        return;

    Vid.ShowCursor(true);
}


/*====================
  CSystem::HideMouseCursor
  ====================*/
void    CSystem::HideMouseCursor()
{
    if (!g_X11Info.dpy)
        return;

    Vid.ShowCursor(false);
}


/*====================
  CSystem::SetMouseClipping
  ====================*/
void    CSystem::SetMouseClipping(const CRecti &recClip)
{
    if (!g_X11Info.dpy || !m_bHasFocus)
        return;

    m_recMouseClip = recClip;

    if (!m_bMouseClip && !sys_grabInput)
    {
        Window Win = *static_cast<Window*>(Vid.GetHWnd());
        if (Win != None)
        {
            if (XGrabPointer(g_X11Info.dpy, Win, True, 0, GrabModeAsync, GrabModeAsync, Win, None, CurrentTime) != GrabSuccess)
                return; // try again next frame
        }
    }

    m_bMouseClip = true;
}


/*====================
  CSystem::UnsetMouseClipping
  ====================*/
void    CSystem::UnsetMouseClipping()
{
    if (!g_X11Info.dpy)
        return;

    if (m_bMouseClip && !sys_grabInput)
    {
        XUngrabPointer(g_X11Info.dpy, CurrentTime);
    }

    m_bMouseClip = false;
}


/*====================
  CSystem::PollMouse
  ====================*/
void    CSystem::PollMouse()
{
    CVec2i v2Pos(GetMousePos());
    if (v2Pos.x != INT_ROUND(Input.GetAxisState(AXIS_MOUSE_X)) || m_v4MousePos.w != 0)
        Input.AddEvent(AXIS_MOUSE_X, v2Pos.x, m_v4MousePos.w);
    if (v2Pos.y != INT_ROUND(Input.GetAxisState(AXIS_MOUSE_Y)) || m_v4MousePos.z != 0)
        Input.AddEvent(AXIS_MOUSE_Y, v2Pos.y, m_v4MousePos.z);
}


/*====================
  CSystem::PollJoysticks
  ====================*/
void    CSystem::PollJoysticks(uint uiID)
{
    // selected joystick changed?
    if (input_joyDeviceID.IsModified())
    {
        if (m_hJoystick != -1)
        {
            close(m_hJoystick);
            m_hJoystick = -1;
        }

        if (input_joyDeviceID != -1)
        {
            m_hJoystick = open(TStringToNative(m_mapJoysticks[input_joyDeviceID].first).c_str(), O_RDONLY | O_NONBLOCK);
            sys_joyName = m_mapJoysticks[input_joyDeviceID].second;
            char axes(0);
            ioctl(m_hJoystick, JSIOCGAXES, &axes);
            m_uiJoystickAxes = MIN(uint(axes), NUM_SYS_JOYSTICK_AXIS);
        }

        input_joyDeviceID.SetModified(false);
    }


    if (m_hJoystick != -1 && input_joyDeviceID != -1)
    {
        struct js_event e;
        while(read(m_hJoystick, &e, sizeof(js_event)) > 0)
        {
            if (e.type & JS_EVENT_BUTTON)
            {
                if (e.number < NUM_JOYSTICK_BUTTONS)
                    Input.AddEvent(EButton(BUTTON_JOY1 + e.number), e.value == 1);
            }
            if (e.type & JS_EVENT_AXIS)
            {
                if (e.number < NUM_SYS_JOYSTICK_AXIS)
                {
                    float fValue(e.value / 32767.0f);
                    m_JoystickState.fAxis[e.number] = fValue;
                }
            }
        }
    }

    for (uint uiAxis(0); uiAxis < m_uiJoystickAxes; ++uiAxis)
    {
        Input.AddEvent(EAxis(AXIS_JOY_X + uiAxis), m_JoystickState.fAxis[uiAxis], m_JoystickState.fAxis[uiAxis]);
    }
}


/*====================
  CSystem::GetJoystickAxis
  ====================*/
float   CSystem::GetJoystickAxis(uint uiID, EAxis eAxis)
{
    return 0.0f; // unneeded
}


/*====================
  CSystem::GetJoystickButtons
  ====================*/
uint    CSystem::GetJoystickButtons(uint uiStick)
{
    return 0; // unneeded
}


/*====================
  CSystem::GetJoystickPOV
  ====================*/
uint    CSystem::GetJoystickPOV(uint uiStick)
{
    return 0; // unneeded
}


/*====================
  CSystem::JoystickHasAxis
  ====================*/
bool    CSystem::JoystickHasAxis(uint uiID, EAxis axis)
{
    if (m_hJoystick)
        if (m_uiJoystickAxes + AXIS_JOY_X > axis)
            return true;

    return false;
}


/*====================
  CSystem::GetJoystickList
  ====================*/
void    CSystem::GetJoystickList(imaps &mapNames)
{
    for (map<int, jsInfo>::iterator it(m_mapJoysticks.begin()); it != m_mapJoysticks.end(); ++it)
    {
        mapNames[it->first] = it->second.second;
    }
}


/*====================
  CSystem::CopyToClipboard
  ====================*/
void    CSystem::CopyToClipboard(const tstring &sText)
{
    if (!g_X11Info.dpy)
        return;

    Window Win = *static_cast<Window*>(Vid.GetHWnd());
    m_sClipboardString = sText;
    XSetSelectionOwner(g_X11Info.dpy, XA_CLIPBOARD, Win, CurrentTime);
}


/*====================
  CSystem::IsClipboardString
  ====================*/
bool    CSystem::IsClipboardString()
{
    if (!g_X11Info.dpy)
        return false;

    Window Win = *static_cast<Window*>(Vid.GetHWnd());

    if (Win == XGetSelectionOwner(g_X11Info.dpy, XA_CLIPBOARD))
        return true;

    XConvertSelection(g_X11Info.dpy, XA_CLIPBOARD, XA_TARGETS, XA_CLIPBOARD, Win, CurrentTime);
    m_ClipboardTarget = None;

    uint uiTimeout(Milliseconds() + 200);
    while (Milliseconds() < uiTimeout)
    {
        XEvent e;
        if (XCheckTypedEvent(g_X11Info.dpy, SelectionNotify, &e))
        {
            if (e.xselection.property == None)
                return false;
            
            Atom returnedType;
            int ReturnedFormat;
            unsigned long ulNumItems;
            unsigned long ulBytesUnread = 0;
            unsigned char* pProp = nullptr;
            unsigned long ulReadLength = 1024; // in 32 bit multiples

            do {
                if (pProp) XFree(pProp);
                XGetWindowProperty(g_X11Info.dpy, Win, e.xselection.property, 0, ulReadLength, False, AnyPropertyType, &returnedType, &ReturnedFormat, &ulNumItems, &ulBytesUnread, &pProp);
                ulReadLength += (ulBytesUnread+3)/4; // do a larger read if we didn't get everything
            } while (ulBytesUnread);

            if (returnedType == XA_ATOM && ReturnedFormat == 32)
            {
                Atom* pAtomList = (Atom*)pProp;
#ifdef UNICODE
                // prefer UTF8
                for (int i(0); i < ulNumItems; ++i)
                {
                    if (pAtomList[i] == XA_UTF8_STRING)
                    {
                        m_ClipboardTarget = XA_UTF8_STRING;
                        return true;
                    }
                }
#endif
                for (int i(0); i < ulNumItems; ++i)
                {
                    if (pAtomList[i] == XA_STRING)
                    {
                        m_ClipboardTarget = XA_STRING;
                        return true;
                    }
                }
            }
        }
    }

    return false;
}


/*====================
  CSystem::GetClipboardString
  ====================*/
tstring CSystem::GetClipboardString()
{
    if (!g_X11Info.dpy)
        return _T("");

    // Note: Assumes that this is called right after a check to IsClipboardString

    Window Win = *static_cast<Window*>(Vid.GetHWnd());

    if (Win == XGetSelectionOwner(g_X11Info.dpy, XA_CLIPBOARD))
        return m_sClipboardString;

     if (m_ClipboardTarget == None)
         return _T("");

    XConvertSelection(g_X11Info.dpy, XA_CLIPBOARD, m_ClipboardTarget, XA_CLIPBOARD, Win, CurrentTime);
    m_ClipboardTarget = None;

    uint uiTimeout(Milliseconds() + 200);
    while (Milliseconds() < uiTimeout)
    {
        XEvent e;
        if (XCheckTypedEvent(g_X11Info.dpy, SelectionNotify, &e))
        {
            if (e.xselection.property == None)
                return _T("");
            
            Atom returnedType;
            int ReturnedFormat;
            unsigned long ulNumItems;
            unsigned long ulBytesUnread = 0;
            unsigned char* pProp = nullptr;
            unsigned long ulReadLength = 1024; // in 32 bit multiples

            do {
                if (pProp) XFree(pProp);
                XGetWindowProperty(g_X11Info.dpy, Win, e.xselection.property, 0, ulReadLength, False, AnyPropertyType, &returnedType, &ReturnedFormat, &ulNumItems, &ulBytesUnread, &pProp);
                ulReadLength += (ulBytesUnread+3)/4; // do a larger read if we didn't get everything
            } while (ulBytesUnread);

#ifdef UNICODE
            if (returnedType == XA_UTF8_STRING && ReturnedFormat == 8)
                return UTF8ToWString(string((char*)pProp));
#endif
            if (returnedType == XA_STRING && ReturnedFormat == 8)
                return StringToTString(string((char*)pProp));
        }
    }

    return _T("");
}


/*====================
  CSystem::Beep
  ====================*/
void    CSystem::Beep(dword dwFreq, dword dwDuration)
{

}


/*====================
  CSystem::IsDebuggerPresent
  ====================*/
bool    CSystem::IsDebuggerPresent()
{
    return false;
}


/*====================
  CSystem::GetClientOffset
  ====================*/
void    CSystem::GetClientOffset(int *x, int *y)
{
    *x = *y = 0; // working in windows coordinates always
}


/*====================
  CSystem::GetRunningProcesses

  Returns a list of all active HoN processes.
  ====================*/
uiset   CSystem::GetRunningProcesses()
{
    uiset setProcesses;
    return setProcesses;
}


/*====================
  CSystem::AddDedicatedConsoleText
  ====================*/
void    CSystem::AddDedicatedConsoleText(const tstring &sText)
{
    if ((sys_dedicatedConsole && m_ConsoleWindowHandle) || !sys_interactive)
        return;

#ifdef UNICODE
    wprintf(_T("%ls"), sText.c_str());
#else
    printf("%s", sText.c_str());
#endif
}


/*====================
  CSystem::UpdateDedicatedConsoleText
  ====================*/
void    CSystem::UpdateDedicatedConsoleText()
{
    if (sys_dedicatedConsole && m_ConsoleWindowHandle)
    {
        CCursesConsole *pConsole(static_cast<CCursesConsole*>(m_ConsoleWindowHandle));
        pConsole->Draw();
    }
}


/*====================
  CSystem::GetSystemInfo
  ====================*/
SSysInfo    CSystem::GetSystemInfo()
{
    SSysInfo structInfo;

    try
    {
        // Get OS -- maybe use /etc/*[-_]{release,version} later on?
        struct utsname uts;
        uname(&uts);

        structInfo.sOS = NativeToTString(uts.sysname) + _T(" ") + NativeToTString(uts.release) + _T(" ") + ParenStr(NativeToTString(uts.machine));

        // Memory
        struct sysinfo info;
        sysinfo(&info);

        // rounding this to a multiple of 64 MB should match it to the actual amount in the RAM chips
        structInfo.sRAM = XtoA(INT_CEIL(float(info.totalram*info.mem_unit)/(1024*1024*64))*64) + _T(" MB");

        // Processor
        structInfo.sProcessor = _T("Unknown");
        std::ifstream ProcCpuinfo("/proc/cpuinfo");
        if (ProcCpuinfo.is_open())
        {
            while (!ProcCpuinfo.eof())
            {
                char s[256];
                ProcCpuinfo.getline(s, 256);
                string sLine(s);
                if (sLine.find("model name", 0) != string::npos)
                {
                    structInfo.sProcessor = NativeToTString(sLine.substr(sLine.find(": ") + 2));
                    break;
                }
            }

            ProcCpuinfo.close();
        }

        // Video Card
        structInfo.sVideo = _T("Unknown");
        DIR *d = opendir("/sys/bus/pci/devices");
        ushort unVendor(0xffff);
        ushort unDevice(0xffff);
        ushort unSubsystemVendor(0xffff);
        ushort unSubsystemDevice(0xffff);
        if (d)
        {
            struct dirent *dir;
            unsigned int uiClass;
            string sPciId;
            string sPath;
            while ((dir = readdir(d)))
            {
                if (dir->d_name[0] == '.') continue;
                sPath = string("/sys/bus/pci/devices/") + dir->d_name + "/class";
                std::ifstream DeviceClass(sPath.c_str());
                if (DeviceClass.is_open())
                {
                    DeviceClass >> std::hex >> uiClass;
                    if ((uiClass & 0xff0000) == 0x030000)
                    {
                        sPciId = dir->d_name;
                        break;
                    }
                    DeviceClass.close();
                }
            }
            closedir(d);

            if (!sPciId.empty())
            {
                sPath = string("/sys/bus/pci/devices/") + sPciId + "/vendor";
                std::ifstream Vendor(sPath.c_str());
                if (Vendor.is_open())
                {
                    Vendor >> std::hex >> unVendor;
                    Vendor.close();
                }

                sPath = string("/sys/bus/pci/devices/") + sPciId + "/device";
                std::ifstream Device(sPath.c_str());
                if (Device.is_open())
                {
                    Device >> std::hex >> unDevice;
                    Device.close();
                }

                sPath = string("/sys/bus/pci/devices/") + sPciId + "/subsystem_vendor";
                std::ifstream SubsystemVendor(sPath.c_str());
                if (SubsystemVendor.is_open())
                {
                    SubsystemVendor >> std::hex >> unSubsystemVendor;
                    SubsystemVendor.close();
                }

                sPath = string("/sys/bus/pci/devices/") + sPciId + "/subsystem_device";
                std::ifstream SubsystemDevice(sPath.c_str());
                if (SubsystemDevice.is_open())
                {
                    SubsystemDevice >> std::hex >> unSubsystemVendor;
                    SubsystemDevice.close();
                }
            }
        }
        else
        {
            std::ifstream ProcBusPciDevices("/proc/bus/pci/devices");
            if (ProcBusPciDevices.is_open())
            {
                union { ushort bdf; struct { ushort fun:3; ushort dev:5; ushort bus:8; }; } bdf;
                union { uint vd; struct { ushort device; ushort vendor; }; } vd;
                while (!ProcBusPciDevices.eof())
                {
                    ProcBusPciDevices >> std::hex >> bdf.bdf >> vd.vd;
                    ProcBusPciDevices.ignore(512, '\n');
                    if (ProcBusPciDevices.fail())
                        break;
                    string sPath(string("/proc/bus/pci/") + XtoS(bdf.bus, FMT_PADZERO | FMT_NOPREFIX, 2, 16) + "/" +
                            XtoS(bdf.dev, FMT_PADZERO | FMT_NOPREFIX, 2, 16) + "." + XtoS(bdf.fun, FMT_PADZERO | FMT_NOPREFIX, 1, 16));
                    std::ifstream PciDevice(sPath.c_str());
                    if (PciDevice.is_open())
                    {
                        byte b;
                        PciDevice.seekg(0xb, std::ios::beg);
                        PciDevice.read((char*)&b, 1);
                        if (b == 0x03)
                        {
                            unVendor = vd.vendor;
                            unDevice = vd.device;
                            PciDevice.seekg(0x2c, std::ios::beg);
                            PciDevice.read((char*)&unSubsystemVendor, 2);
                            PciDevice.read((char*)&unSubsystemDevice, 2);
                            PciDevice.close();
                            Console << XtoA(unSubsystemVendor, FMT_PADZERO | FMT_NOPREFIX, 4, 16) << _T(":") << XtoA(unSubsystemDevice, FMT_PADZERO | FMT_NOPREFIX, 4, 16);
                            break;
                        }
                        PciDevice.close();
                    }
                }
                ProcBusPciDevices.close();
            }
        }

        // use the http://pciids.sourceforge.net pci.ids to identify the device
        CFile *pPciIds = FileManager.GetFile(_T(":/pci.ids"), FILE_READ | FILE_TEXT | FILE_ALLOW_CUSTOM);
        bool bFoundVendor(false);
        if (pPciIds)
        {
            tstring sLine;
            while (!pPciIds->IsEOF())
            {
                sLine = pPciIds->ReadLine();
                if (sLine[0] == _T('#') || sLine.length() < 7 || sLine[0] == _T('\t'))
                    continue;
                ushort unId(strtoul(TStringToNative(sLine).c_str(), nullptr, 16));
                if (unId == 0xffff)
                {
                    break; // end of list
                }
                if (unId == unVendor)
                {
                    bool bFoundDevice(false);
                    tstring sVendor(sLine.substr(6));
                    bFoundVendor = true;
                    while(!pPciIds->IsEOF())
                    {
                        sLine = pPciIds->ReadLine();
                        if (sLine[0] == _T('#') || sLine.length() < 7 || (sLine[0] == _T('\t') && sLine[1] == _T('\t')))
                            continue;
                        if (sLine[0] == _T('\t'))
                        {
                            unId = strtoul(TStringToNative(sLine.substr(1)).c_str(), nullptr, 16);
                            if (unId == unDevice)
                            {
                                bFoundDevice = true;
                                tstring sDevice(sLine.substr(7));
                                tstring::size_type startPos(sDevice.find('['));
                                tstring::size_type endPos(sDevice.find(']'));
                                if (startPos != string::npos && endPos != string::npos && startPos < endPos)
                                    sDevice = sDevice.substr(startPos+1, endPos-startPos-1);
                                while(!pPciIds->IsEOF())
                                {
                                    sLine = pPciIds->ReadLine();
                                    if (sLine[0] == _T('#') || sLine.length() < 7)
                                        continue;
                                    if (sLine[0] == _T('\t') && sLine[1] == _T('\t'))
                                    {
                                        unId = strtoul(TStringToNative(sLine.substr(2)).c_str(), nullptr, 16);
                                        ushort unId2 = strtoul(TStringToNative(sLine.substr(7)).c_str(), nullptr, 16);
                                        if (unId == unSubsystemVendor && unId2 == unSubsystemDevice)
                                        {
                                            tstring sSubsystem(sLine.substr(13));
                                            tstring::size_type startPos(sSubsystem.find('['));
                                            tstring::size_type endPos(sSubsystem.find(']'));
                                            if (startPos != string::npos && endPos != string::npos && startPos+2 < endPos)
                                                sSubsystem = sSubsystem.substr(startPos+1, endPos-startPos-2);
                                            structInfo.sVideo = sSubsystem;
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        structInfo.sVideo = sDevice;
                                        break; // end of subdevices for this device
                                    }
                                }
                            }
                        }
                        else
                        {
                            break; // end of devices for this vendor
                        }
                    }
                    if (!bFoundDevice)
                    {
                        structInfo.sVideo = sVendor + _T(" - Unknown Device (") + XtoA(unDevice, FMT_PADZERO | FMT_NOPREFIX, 4, 16) + _T(")");
                    }
                    break;
                }
            }
            K2_DELETE(pPciIds);
        }

        if (unVendor != 0xffff && !bFoundVendor)
        {
            structInfo.sVideo = _T("Unknown (") + XtoA(unVendor, FMT_PADZERO | FMT_NOPREFIX, 4, 16) +
                                _T(":") + XtoA(unDevice, FMT_PADZERO | FMT_NOPREFIX, 4, 16) + _T(")");
        }

        // MAC address
        struct ifreq ifr;
        string sInterface("eth0");
        int sock;

        sock = socket(PF_INET, SOCK_DGRAM, 0);
        if (sock != -1)
        {
            // get interfaces in the system
            std::ifstream ProcNetDev("/proc/net/dev");
            if (ProcNetDev.is_open())
            {
                while (!ProcNetDev.eof())
                {
                    string::size_type endPos, startPos;
                    char s[256];
                    ProcNetDev.getline(s, 256);
                    string sLine(s);

                    endPos = sLine.find(':', 0);
                    if (endPos == string::npos)
                        continue;

                    startPos = sLine.find(' ', endPos);
                    if (startPos == string::npos)
                        continue;

                    sLine = sLine.substr(startPos + 1, endPos - startPos - 1);

                    // check if this interface is active
                    MemManager.Set(&ifr, 0, sizeof(struct ifreq));
                    strncpy(ifr.ifr_name, sLine.c_str(), IFNAMSIZ);
                    if (ioctl(sock, SIOCGIFFLAGS, &ifr) == -1)
                        continue;

                    if ((ifr.ifr_flags & IFF_UP) && (ifr.ifr_flags & IFF_RUNNING) && !(ifr.ifr_flags & IFF_LOOPBACK))
                    {
                        sInterface = sLine;
                        break;
                    }
                }

                ProcNetDev.close();
            }

            // get the actual Mac address
            MemManager.Set(&ifr, 0, sizeof(struct ifreq));
            strncpy(ifr.ifr_name, sInterface.c_str(), IFNAMSIZ);
            ifr.ifr_addr.sa_family = AF_INET;
            if (ioctl(sock, SIOCGIFHWADDR, &ifr) == -1)
            {
                EX_ERROR(_T("ioctl SIOCGIFHWADDR failed"));
            }
            else
            {
                structInfo.sMAC = UpperString(XtoA((unsigned char)ifr.ifr_hwaddr.sa_data[0], FMT_PADZERO | FMT_NOPREFIX, 2, 16));
                for (int i(1); i < 6; ++i)
                    structInfo.sMAC += _T(":") + UpperString(XtoA((unsigned char)ifr.ifr_hwaddr.sa_data[i], FMT_PADZERO | FMT_NOPREFIX, 2, 16));
            }
        }
        else
        {
            EX_ERROR(_T("Unable to open socket"));
        }

        return structInfo;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CSystem::GetSystemInfo() - "), NO_THROW);
        return structInfo;
    }
}


/*====================
  CSystem::GetTotalPhysicalMemory
  ====================*/
ULONGLONG   CSystem::GetTotalPhysicalMemory() const
{
    struct sysinfo info;
    sysinfo(&info);
    return info.totalram * info.mem_unit;
}


/*====================
  CSystem::GetFreePhysicalMemory
  ====================*/
ULONGLONG   CSystem::GetFreePhysicalMemory() const
{
    struct sysinfo info;
    sysinfo(&info);
    return info.freeram * info.mem_unit;
}


/*====================
  CSystem::GetTotalVirtualMemory
  ====================*/
ULONGLONG   CSystem::GetTotalVirtualMemory() const
{
    ULONGLONG ull(0);
    string sPath("/proc/meminfo");
    std::ifstream ProcMeminfo(sPath.c_str());
    if (ProcMeminfo.is_open())
    {
        while (!ProcMeminfo.eof() && !ProcMeminfo.bad())
        {
            char s[512];
            ProcMeminfo.getline(s, 512);
            if (strncmp(s, "VmallocTotal", 12) == 0)
            {
                ull = atoll(&s[13]);
                break;
            }
        }
        ProcMeminfo.close();
    }
    return ull * 1024;
}


/*====================
  CSystem::GetFreeVirtualMemory
  ====================*/
ULONGLONG   CSystem::GetFreeVirtualMemory() const
{
    ULONGLONG ull(0);
    string sPath("/proc/meminfo");
    std::ifstream ProcMeminfo(sPath.c_str());
    if (ProcMeminfo.is_open())
    {
        while (!ProcMeminfo.eof() && !ProcMeminfo.bad())
        {
            char s[512];
            ProcMeminfo.getline(s, 512);
            if (strncmp(s, "VmallocUsed", 11) == 0)
            {
                ull = atoll(&s[12]);
                break;
            }
        }
        ProcMeminfo.close();
    }
    return GetTotalVirtualMemory() - ull * 1024;
}


/*====================
  CSystem::GetTotalPageFile
  ====================*/
ULONGLONG   CSystem::GetTotalPageFile() const
{
    struct sysinfo info;
    sysinfo(&info);
    return info.totalswap * info.mem_unit;
}


/*====================
  CSystem::GetFreePageFile
  ====================*/
ULONGLONG   CSystem::GetFreePageFile() const
{
    struct sysinfo info;
    sysinfo(&info);
    return info.freeswap * info.mem_unit;
}


/*====================
  CSystem::GetProcessMemoryUsage
  ====================*/
ULONGLONG   CSystem::GetProcessMemoryUsage() const
{
    ULONGLONG ull(0);
    string sPath("/proc/"+XtoS(getpid())+"/statm");
    std::ifstream ProcPidStatm(sPath.c_str());
    if (ProcPidStatm.is_open())
    {
        ProcPidStatm >> ull >> ull;
        ProcPidStatm.close();
    }
    return ull * getpagesize();
}


/*====================
  CSystem::GetProcessVirtualMemoryUsage
  ====================*/
ULONGLONG   CSystem::GetProcessVirtualMemoryUsage() const
{
    ULONGLONG ull(0);
    string sPath("/proc/"+XtoS(getpid())+"/statm");
    std::ifstream ProcPidStatm(sPath.c_str());
    if (ProcPidStatm.is_open())
    {
        ProcPidStatm >> ull;
        ProcPidStatm.close();
    }
    return ull * getpagesize();
}


/*====================
  CSystem::SetConfig
  ====================*/
void    CSystem::SetConfig(const tstring &sConfig)
{
    m_sUserDir = NativeToTString(getenv("HOME"));
    m_sUserDir += _T("/.") + GetGameName() + _T("/");

    if (!sConfig.empty())
        m_sUserDir += sConfig + _T("/");

    if (mkdir(TStringToNative(m_sUserDir).c_str(), S_IRUSR | S_IWUSR | S_IXUSR) != 0) //permissions of 700 (rwx------)
    {
        if (errno != EEXIST)
            Console.Err << _T("Failed to create user directory, settings will not be saved!") << newl;
    }
}


/*====================
  CSystem::SetAffinity

  Set process affinity
  ====================*/
void    CSystem::SetAffinity(int iCPU)
{
    cpu_set_t mask;
    CPU_ZERO(&mask);
    if (iCPU < 0)
    {
        /* all avaialble */
        for (int i(0); i < 32; ++i)
        {
            CPU_SET(i, &mask);
        }
        sched_setaffinity(0, sizeof(cpu_set_t), &mask);
    }
    else if (iCPU < CPU_SETSIZE)
    {
        CPU_SET(iCPU, &mask);
        sched_setaffinity(0, sizeof(cpu_set_t), &mask);
    }
}


/*====================
  CSystem::GetAffinityMask
  ====================*/
uint    CSystem::GetAffinityMask() const
{
    uint uiMask(0);
    cpu_set_t oldmask, mask;
    
    if (sched_getaffinity(0, sizeof(cpu_set_t), &oldmask) == -1)
        return 0;
    
    CPU_ZERO(&mask);
    for (int i(0); i < 32; ++i)
    {
        CPU_SET(i, &mask);
    }
    
    sched_setaffinity(0, sizeof(cpu_set_t), &mask);
    sched_getaffinity(0, sizeof(cpu_set_t), &mask);
    
    for (int i(0); i < 32; ++i)
    {
        if (CPU_ISSET(i, &mask))
            uiMask |= BIT(i);
    }
    
    sched_setaffinity(0, sizeof(cpu_set_t), &oldmask);
    
    return uiMask;
}


/*====================
  CSystem::SetPriority
  ====================*/
void    CSystem::SetPriority(int iPriority)
{
    //TODO
}


/*====================
  CSystem::GetPriority
  ====================*/
int     CSystem::GetPriority() const
{
    //TODO
    return 0;
}


/*====================
  CSystem::GetProcessFilename
  ====================*/
tstring CSystem::GetProcessFilename()
{
    string sFilename("./");
    char *s = strrchr(m_argv[0], '/');
    if (s)
        sFilename += (s+1);
    else
        sFilename += m_argv[0];
    return NativeToTString(sFilename);
}


/*====================
  CSystem::InitDedicatedConsole
  ====================*/
void    CSystem::InitDedicatedConsole()
{
    if (sys_dedicatedConsole && sys_interactive)
    {
        m_ConsoleWindowHandle = static_cast<void*>(K2_NEW(global,  CCursesConsole)());

        // keys and escape sequences for buttons used by the console
        g_KeyboardMap[8] =              BUTTON_BACKSPACE;
        g_KeyboardMap[127] =            BUTTON_BACKSPACE;
        g_KeyboardMap[9] =              BUTTON_TAB;
        g_KeyboardMap[10] =             BUTTON_ENTER;
        g_KeyboardMap[27] =             BUTTON_ESC;
        g_KeyboardMap[32] =             BUTTON_SPACE;

        g_KeyboardMap[59] =             BUTTON_MISC1;
        g_KeyboardMap[47] =             BUTTON_MISC2;
        g_KeyboardMap[96] =             BUTTON_MISC3;
        g_KeyboardMap[91] =             BUTTON_MISC4;
        g_KeyboardMap[92] =             BUTTON_MISC5;
        g_KeyboardMap[93] =             BUTTON_MISC6;
        g_KeyboardMap[39] =             BUTTON_MISC7;

        g_KeyboardMap[75] =             BUTTON_PLUS;
        g_KeyboardMap[45] =             BUTTON_MINUS;
        g_KeyboardMap[44] =             BUTTON_COMMA;
        g_KeyboardMap[46] =             BUTTON_PERIOD;

        g_KeyboardEscSequenceMap[_T("[A")] =    escSequence(BUTTON_UP, MOD_NONE);
        g_KeyboardEscSequenceMap[_T("[1;5A")] = escSequence(BUTTON_UP, MOD_CTRL);
        g_KeyboardEscSequenceMap[_T("[B")] =    escSequence(BUTTON_DOWN, MOD_NONE);
        g_KeyboardEscSequenceMap[_T("[1;5B")] = escSequence(BUTTON_DOWN, MOD_CTRL);
        g_KeyboardEscSequenceMap[_T("[C")] =    escSequence(BUTTON_RIGHT, MOD_NONE);
        g_KeyboardEscSequenceMap[_T("[D")] =    escSequence(BUTTON_LEFT, MOD_NONE);

        g_KeyboardEscSequenceMap[_T("[2~")] =   escSequence(BUTTON_INS, MOD_NONE);
        g_KeyboardEscSequenceMap[_T("[3~")] =   escSequence(BUTTON_DEL, MOD_NONE);
        g_KeyboardEscSequenceMap[_T("[1~")] =   escSequence(BUTTON_HOME, MOD_NONE);
        g_KeyboardEscSequenceMap[_T("[H")] =    escSequence(BUTTON_HOME, MOD_NONE);
        g_KeyboardEscSequenceMap[_T("[1;5H")] = escSequence(BUTTON_HOME, MOD_CTRL);
        g_KeyboardEscSequenceMap[_T("[4~")] =   escSequence(BUTTON_END, MOD_NONE);
        g_KeyboardEscSequenceMap[_T("[F")] =    escSequence(BUTTON_END, MOD_NONE);
        g_KeyboardEscSequenceMap[_T("[1;5F")] = escSequence(BUTTON_END, MOD_CTRL);
        g_KeyboardEscSequenceMap[_T("[5~")] =   escSequence(BUTTON_PGUP, MOD_NONE);
        g_KeyboardEscSequenceMap[_T("[6~")] =   escSequence(BUTTON_PGDN, MOD_NONE);
    }
}


/*====================
  CSystem::SetTitle
  ====================*/
void    CSystem::SetTitle(const tstring &sTitle)
{
    if (sTitle == m_sConsoleTitle)
        return;

    m_sConsoleTitle = sTitle;

    // TODO
    
    /*if (m_ConsoleWindowHandle)
    {
        if (K2System.IsServerManager())
            ::SetConsoleTitle(_T("K2 Server Manager"));
        else
            ::SetConsoleTitle(m_sConsoleTitle.c_str());
    }*/
}


/*====================
  CSystem::AnalyzeMemory
  ====================*/
void    CSystem::AnalyzeMemory()
{
    
}


//=============================================================================

