// (C)2008 S2 Games
// system_osx.cpp
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

#include <mach/mach.h>
#include <mach/mach_time.h>
#include <unistd.h>
#include <sys/sysctl.h>
#include <Cocoa/Cocoa.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDUsageTables.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/network/IOEthernetController.h>
#include <CoreAudio/CoreAudio.h>
#include <thread>

#include <libfswatch/libfswatch_config.h>
#define HAVE_FSEVENTS_FSEVENTSTREAMSETDISPATCHQUEUE
#include <libfswatch/c++/path_utils.hpp>
#include <libfswatch/c++/monitor_factory.hpp>
#ifdef HAVE_FSEVENTS_FSEVENTSTREAMSETDISPATCHQUEUE
  #include <libfswatch/c++/fsevents_monitor.hpp>
#endif


#include "c_system.h"

#include "c_filemanager.h"
#include "c_input.h"
#include "c_cmd.h"
#include "c_vid.h"
#include "c_filehttp.h"
#include "c_mutex.h"
//=============================================================================


//=============================================================================
// Definitions
//=============================================================================
extern CCvar<bool>  host_dynamicResReload;

typedef map<ushort, EButton>    keyboardMap;
#define MOD_NONE    0
#define MOD_SHIFT   1
#define MOD_CTRL    2
#define MOD_ALT     4
typedef pair<EButton, byte>         escSequence;
typedef map<tstring, escSequence>   escSequenceMap; // button + modifiers

struct CSystem::SJoystickInfo
{
    tstring sName;
    IOHIDDeviceInterface122 **hidDeviceInterface;
    struct
    {
        IOHIDElementCookie cookie;
        EAxis eAxis;
        int iMin, iMax;
    } axis[9];
    struct
    {
        IOHIDElementCookie cookie;
        EButton eButton;
    } button[32];
    struct
    {
        IOHIDElementCookie cookie;
        int iMin, iMax, iUp, iRight, iDown, iLeft;
    } hat;
};
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
SINGLETON_INIT(CSystem);
CSystem& K2System(*CSystem::GetInstance());

CVAR_BOOLF  (sys_autoSaveConfig,        true,   CVAR_SAVECONFIG);
CVAR_BOOLF  (sys_fileChangeNotify,      true,   CONEL_DEV);

CVAR_BOOLF  (sys_dedicatedConsole,      true,   CVAR_SAVECONFIG);
CVAR_BOOL   (sys_interactive,           true);
#if defined(_DEBUG)
CVAR_BOOL   (sys_debugOutput,           true);
#else
CVAR_BOOL   (sys_debugOutput,           false);
#endif

CVAR_BOOLF  (key_splitShift,            false,  CVAR_SAVECONFIG);
CVAR_BOOLF  (key_splitOption,           false,  CVAR_SAVECONFIG);
CVAR_BOOLF  (key_splitControl,          false,  CVAR_SAVECONFIG);
CVAR_BOOLF  (key_splitCommand,          false,  CVAR_SAVECONFIG);
//CVAR_BOOLF    (key_splitEnter,            false,  CVAR_SAVECONFIG);
CVAR_BOOL   (key_debugEvents,           false);

EXTERN_CVAR_INT(input_joyDeviceID);
CVAR_STRINGF(sys_joyName,               "", CVAR_SAVECONFIG);
CVAR_BOOL   (sys_debugHID,              false);
CVAR_BOOLF  (sys_warpCursor,            true,   CVAR_SAVECONFIG);

keyboardMap             g_KeyboardMap;
escSequenceMap          g_KeyboardEscSequenceMap;
CK2Mutex                g_FileMonitorMutex;
std::thread*            g_pFileMonitorThread;
//=============================================================================

#ifndef wcscasecmp
#include <wctype.h>
// this doesn't seem to be present in OS X's libc
int wcscasecmp(const wchar_t *s1, const wchar_t *s2)
{
    wchar_t c1, c2;
    while (*s1 && *s2)
    {
        c1 = towlower(*s1);
        c2 = towlower(*s2);
        if (c1 != c2)
            return c1-c2;
    }
    return (*s1-*s2);
}
#endif

/*====================
 GetModPaths
 ====================*/
vector<string> GetModPaths()
{
    vector<string> vPaths;
    for (const auto& sMod : FileManager.GetModStack()) {
        vPaths.emplace_back(fsw::fsw_realpath(TStringToNative(K2System.GetRootDir() + _T("/") + sMod).c_str(), nullptr));
    }
    return std::move(vPaths);
}


/*====================
 CSystem::CSystem
 ====================*/
CSystem::CSystem() :
m_hInstance(NULL),
m_ConsoleWindowHandle(NULL),
m_WindowHandle(NULL),
m_bRestartProcess(false),
m_bDedicatedServer(false),
m_bServerManager(false),
m_bHasFocus(false),
m_bMouseClip(false),
m_recMouseClip(0, 0, 0, 0),
m_pfnMainWndProc(NULL),
m_ullVirtualMemoryLimit(4294967296ull) // 4GB address space per process on 32 bit Mac OS X (18 exabytes on 64 bit MaC OS X)
{
    // Set system start time so K2System.Milliseconds counts from 0
    m_llStartTicks = mach_absolute_time();
    mach_timebase_info_data_t TimeBaseInfo;
    mach_timebase_info(&TimeBaseInfo);
    m_llFrequency = (long long)(1000000000.0 * TimeBaseInfo.denom / TimeBaseInfo.numer);
    
    m_hMainThread = GetCurrentThread();
}


/*====================
 CSystem::Init
 ====================*/
void    CSystem::Init(const tstring &sGameName, const tstring &sVersion, const tstring &sBuildInfo, const tstring &sBuildNumber, const tstring &sBuildOS, const tstring &sBuildOSCode, const tstring &sBuildArch, const string &sMasterServerAddress)
{
    m_sGameName = sGameName;
    m_sVersion = sVersion;
    m_sBuildInfo = sBuildInfo;
    m_sBuildNumber = sBuildNumber;
    m_sBuildOS = sBuildOS;
    m_sBuildOSCode = sBuildOSCode;
    m_sBuildArch = sBuildArch;
    m_sMasterServerAddress = sMasterServerAddress;
    
    srand(uint(m_llStartTicks & UINT_MAX));
    
    // Set the commandline
    m_sCommandLine.erase();
    NSEnumerator *cmdline = [[[NSProcessInfo processInfo] arguments] objectEnumerator];
    NSString *str = [cmdline nextObject];
    while ((str = [cmdline nextObject]))
    {
        if ([str rangeOfString:@"-psn"].location != NSNotFound)
            break;
        m_sCommandLine += NativeToTString([str UTF8String]);
        m_sCommandLine += _T(" ");
    }
    
    // Set the root path
    m_sRootDir = NativeToTString([[[NSBundle mainBundle] bundlePath] UTF8String]);
    m_sRootDir += _T('/');
    
    // Change the working directory to the proper location
    [[NSFileManager defaultManager] changeCurrentDirectoryPath:[NSString stringWithUTF8String:TStringToNative(m_sRootDir).c_str()]];
    
    // Set the user directory
    SetConfig(_T(""));
    
    // Process the base paths after they have both been initially set,
    // because FileManager could behave poorly if these are not yet set.
    m_sRootDir = FileManager.SanitizePath(m_sRootDir, false);
    m_sUserDir = FileManager.SanitizePath(m_sUserDir, false);
    
    // Shut down screensaver
    
    // Store the current mouse clipping area
    
    m_bHasFocus = true;
}


/*====================
 CSystem::InitMore
 ====================*/
void    CSystem::InitMore()
{
    if (!IsDedicatedServer() && !IsServerManager())
    {
        // Init the Keyboard map
        g_KeyboardMap[0x33] =   BUTTON_BACKSPACE;
        g_KeyboardMap[0x30] =   BUTTON_TAB;
        g_KeyboardMap[0x24] =   BUTTON_ENTER;
        g_KeyboardMap[0x35] =   BUTTON_ESC;
        g_KeyboardMap[0x31] =   BUTTON_SPACE;
        
        g_KeyboardMap[0x00] =   EButton('A');
        g_KeyboardMap[0x0B] =   EButton('B');
        g_KeyboardMap[0x08] =   EButton('C');
        g_KeyboardMap[0x02] =   EButton('D');
        g_KeyboardMap[0x0E] =   EButton('E');
        g_KeyboardMap[0x03] =   EButton('F');
        g_KeyboardMap[0x05] =   EButton('G');
        g_KeyboardMap[0x04] =   EButton('H');
        g_KeyboardMap[0x22] =   EButton('I');
        g_KeyboardMap[0x26] =   EButton('J');
        g_KeyboardMap[0x28] =   EButton('K');
        g_KeyboardMap[0x25] =   EButton('L');
        g_KeyboardMap[0x2E] =   EButton('M');
        g_KeyboardMap[0x2D] =   EButton('N');
        g_KeyboardMap[0x1F] =   EButton('O');
        g_KeyboardMap[0x23] =   EButton('P');
        g_KeyboardMap[0x0C] =   EButton('Q');
        g_KeyboardMap[0x0F] =   EButton('R');
        g_KeyboardMap[0x01] =   EButton('S');
        g_KeyboardMap[0x11] =   EButton('T');
        g_KeyboardMap[0x20] =   EButton('U');
        g_KeyboardMap[0x09] =   EButton('V');
        g_KeyboardMap[0x0D] =   EButton('W');
        g_KeyboardMap[0x07] =   EButton('X');
        g_KeyboardMap[0x10] =   EButton('Y');
        g_KeyboardMap[0x06] =   EButton('Z');
        
        g_KeyboardMap[0x1D] =   EButton('0');
        g_KeyboardMap[0x12] =   EButton('1');
        g_KeyboardMap[0x13] =   EButton('2');
        g_KeyboardMap[0x14] =   EButton('3');
        g_KeyboardMap[0x15] =   EButton('4');
        g_KeyboardMap[0x17] =   EButton('5');
        g_KeyboardMap[0x16] =   EButton('6');
        g_KeyboardMap[0x1A] =   EButton('7');
        g_KeyboardMap[0x1C] =   EButton('8');
        g_KeyboardMap[0x19] =   EButton('9');
        
        g_KeyboardMap[0x39] =   BUTTON_CAPS_LOCK;
        
        g_KeyboardMap[0x7A] =   BUTTON_F1;
        g_KeyboardMap[0x78] =   BUTTON_F2;
        g_KeyboardMap[0x63] =   BUTTON_F3;
        g_KeyboardMap[0x76] =   BUTTON_F4;
        g_KeyboardMap[0x60] =   BUTTON_F5;
        g_KeyboardMap[0x61] =   BUTTON_F6;
        g_KeyboardMap[0x62] =   BUTTON_F7;
        g_KeyboardMap[0x64] =   BUTTON_F8;
        g_KeyboardMap[0x65] =   BUTTON_F9;
        g_KeyboardMap[0x6D] =   BUTTON_F10;
        g_KeyboardMap[0x67] =   BUTTON_F11;
        g_KeyboardMap[0x6F] =   BUTTON_F12;
        
        g_KeyboardMap[0x38] =   BUTTON_LSHIFT;
        g_KeyboardMap[0x3C] =   BUTTON_RSHIFT;
        
        g_KeyboardMap[0x3B] =   BUTTON_LCTRL;
        g_KeyboardMap[0x3E] =   BUTTON_RCTRL;
        
        g_KeyboardMap[0x37] =   BUTTON_LCMD;
        g_KeyboardMap[0x36] =   BUTTON_RCMD;
        
        g_KeyboardMap[0x3A] =   BUTTON_LOPT;
        g_KeyboardMap[0x3D] =   BUTTON_ROPT;
        
        g_KeyboardMap[0x6E] =   BUTTON_MENU;
        
        g_KeyboardMap[0x7E] =   BUTTON_UP;
        g_KeyboardMap[0x7B] =   BUTTON_LEFT;
        g_KeyboardMap[0x7D] =   BUTTON_DOWN;
        g_KeyboardMap[0x7C] =   BUTTON_RIGHT;
        
        g_KeyboardMap[0x72] =   BUTTON_FN;
        g_KeyboardMap[0x75] =   BUTTON_DEL;
        g_KeyboardMap[0x73] =   BUTTON_HOME;
        g_KeyboardMap[0x77] =   BUTTON_END;
        g_KeyboardMap[0x79] =   BUTTON_PGDN;
        g_KeyboardMap[0x74] =   BUTTON_PGUP;
        
        g_KeyboardMap[0x69] =   BUTTON_F13;
        g_KeyboardMap[0x6B] =   BUTTON_F14;
        g_KeyboardMap[0x71] =   BUTTON_F15;
        
        g_KeyboardMap[0x47] =   BUTTON_CLEAR;
        g_KeyboardMap[0x4B] =   BUTTON_DIVIDE;
        g_KeyboardMap[0x43] =   BUTTON_MULTIPLY;
        g_KeyboardMap[0x45] =   BUTTON_ADD;
        g_KeyboardMap[0x4E] =   BUTTON_SUBTRACT;
        g_KeyboardMap[0x41] =   BUTTON_DECIMAL;
        g_KeyboardMap[0x52] =   BUTTON_NUM0;
        g_KeyboardMap[0x53] =   BUTTON_NUM1;
        g_KeyboardMap[0x54] =   BUTTON_NUM2;
        g_KeyboardMap[0x55] =   BUTTON_NUM3;
        g_KeyboardMap[0x56] =   BUTTON_NUM4;
        g_KeyboardMap[0x57] =   BUTTON_NUM5;
        g_KeyboardMap[0x58] =   BUTTON_NUM6;
        g_KeyboardMap[0x59] =   BUTTON_NUM7;
        g_KeyboardMap[0x5B] =   BUTTON_NUM8;
        g_KeyboardMap[0x5C] =   BUTTON_NUM9;
        g_KeyboardMap[0x4C] =   BUTTON_NUM_ENTER;
        
        g_KeyboardMap[0x29] =   BUTTON_MISC1; // ;:
        g_KeyboardMap[0x2C] =   BUTTON_MISC2; // /?
        g_KeyboardMap[0x32] =   BUTTON_MISC3; // `~
        g_KeyboardMap[0x21] =   BUTTON_MISC4; // [{
        g_KeyboardMap[0x2A] =   BUTTON_MISC5; // \|
        g_KeyboardMap[0x1E] =   BUTTON_MISC6; // ]}
        g_KeyboardMap[0x27] =   BUTTON_MISC7; // '"
        
        g_KeyboardMap[0x18] =   BUTTON_PLUS;
        g_KeyboardMap[0x1B] =   BUTTON_MINUS;
        g_KeyboardMap[0x2B] =   BUTTON_COMMA;
        g_KeyboardMap[0x2F] =   BUTTON_PERIOD;
        
        g_KeyboardMap[0x51] =   BUTTON_EQUALS;
        g_KeyboardMap[0x6a] =   BUTTON_F16;
        g_KeyboardMap[0x40] =   BUTTON_F17;
        g_KeyboardMap[0x4f] =   BUTTON_F18;
        g_KeyboardMap[0x50] =   BUTTON_F19;
        
        // init the joysticks
        CFMutableDictionaryRef hidMatchDictionary;
        hidMatchDictionary = IOServiceMatching(kIOHIDDeviceKey);
        
        io_iterator_t hidObjectIterator;
        IOServiceGetMatchingServices(kIOMasterPortDefault, hidMatchDictionary, &hidObjectIterator);
        
        io_object_t hidDevice;
        int iJoystick(0);
        
        input_joyDeviceID = -1;
        
        while ((hidDevice = IOIteratorNext(hidObjectIterator)))
        {
            IOHIDDeviceInterface122 **hidDeviceInterface(NULL);
            IOCFPlugInInterface **plugInInterface(NULL);
            SInt32 score(0);
            CFTypeRef object;
            CFArrayRef usagePairs;
            CFDictionaryRef usagePair;
            long usage, usagePage;
            bool bIsJoystick(false);
            char productName[256];
            
            object = IORegistryEntryCreateCFProperty(hidDevice, CFSTR(kIOHIDProductKey), kCFAllocatorDefault, kNilOptions);
            if (object == 0 || CFGetTypeID(object) != CFStringGetTypeID())
                continue;
            CFStringGetCString(CFStringRef(object), productName, 256, kCFStringEncodingUTF8);
            CFRelease(object);
            
            // determine if the device is a joystick/gamepad
            if (!(usagePairs = CFArrayRef(IORegistryEntryCreateCFProperty(hidDevice, CFSTR(kIOHIDDeviceUsagePairsKey), kCFAllocatorDefault, kNilOptions))))
                continue;
            for (CFIndex i(0); i < CFArrayGetCount(usagePairs); ++i)
            {
                usagePair = CFDictionaryRef(CFArrayGetValueAtIndex(usagePairs, i));
                
                object = CFDictionaryGetValue(usagePair, CFSTR(kIOHIDDeviceUsageKey));
                if (object == 0 || CFGetTypeID(object) != CFNumberGetTypeID())
                    continue;
                if (!CFNumberGetValue(CFNumberRef(object), kCFNumberLongType, &usage))
                    continue;
                
                object = CFDictionaryGetValue(usagePair, CFSTR(kIOHIDDeviceUsagePageKey));
                if (object == 0 || CFGetTypeID(object) != CFNumberGetTypeID())
                    continue;
                if (!CFNumberGetValue(CFNumberRef(object), kCFNumberLongType, &usagePage))
                    continue;
                
                if (sys_debugHID)
                    Console << _T("Device: ") << productName << _T(" Usage: ") << XtoA(usagePage, 0, 0, 16) << _T(":") << XtoA(usage, 0, 0, 16) << newl;
                
                if (usagePage == kHIDPage_GenericDesktop &&
                    (usage == kHIDUsage_GD_GamePad ||
                     usage == kHIDUsage_GD_Joystick ||
                     usage == kHIDUsage_GD_MultiAxisController))
                    bIsJoystick = true;
            }
            CFRelease(usagePairs);
            
            if (!bIsJoystick)
                continue;
            
            // now get all the information we need and init the joystick
            if (IOCreatePlugInInterfaceForService(hidDevice, kIOHIDDeviceUserClientTypeID, kIOCFPlugInInterfaceID, &plugInInterface, &score) != kIOReturnSuccess)
                continue;
            (*plugInInterface)->QueryInterface(plugInInterface, CFUUIDGetUUIDBytes(kIOHIDDeviceInterfaceID), (LPVOID*) &hidDeviceInterface);
            (*plugInInterface)->Release(plugInInterface);
            
            SJoystickInfo *pInfo = new SJoystickInfo;
            MemManager.Set((char*)(pInfo)+sizeof(tstring), 0, sizeof(SJoystickInfo)-sizeof(tstring));
            
            pInfo->sName = NativeToTString(productName);
            
            CFArrayRef elements;
            CFDictionaryRef element;
            long number;
            int minValue, maxValue;
            IOHIDElementCookie cookie;
            
            if (((*hidDeviceInterface)->copyMatchingElements(hidDeviceInterface, NULL, &elements)) == kIOReturnSuccess)
            {
                for (CFIndex i(0); i < CFArrayGetCount(elements); ++i)
                {
                    element = CFDictionaryRef(CFArrayGetValueAtIndex(elements, i));
                    
                    //Get cookie
                    object = (CFDictionaryGetValue(element, CFSTR(kIOHIDElementCookieKey)));
                    if (object == 0 || CFGetTypeID(object) != CFNumberGetTypeID())
                        continue;
                    if(!CFNumberGetValue(CFNumberRef(object), kCFNumberLongType, &number))
                        continue;
                    cookie = IOHIDElementCookie(number);
                    
                    //Get usage
                    object = CFDictionaryGetValue(element, CFSTR(kIOHIDElementUsageKey));
                    if (object == 0 || CFGetTypeID(object) != CFNumberGetTypeID())
                        continue;
                    if (!CFNumberGetValue(CFNumberRef(object), kCFNumberLongType, &usage))
                        continue;
                    
                    //Get usage page
                    object = CFDictionaryGetValue(element,CFSTR(kIOHIDElementUsagePageKey));
                    if (object == 0 || CFGetTypeID(object) != CFNumberGetTypeID())
                        continue;
                    if (!CFNumberGetValue(CFNumberRef(object), kCFNumberLongType, &usagePage))
                        continue;
                    
                    //Get Min/Max
                    object = CFDictionaryGetValue(element,CFSTR(kIOHIDElementMaxKey));
                    if (object == 0 || CFGetTypeID(object) != CFNumberGetTypeID())
                        continue;
                    if (!CFNumberGetValue((CFNumberRef) object, kCFNumberIntType, &maxValue))
                        continue;
                    object = CFDictionaryGetValue(element,CFSTR(kIOHIDElementMinKey));
                    if (object == 0 || CFGetTypeID(object) != CFNumberGetTypeID())
                        continue;
                    if (!CFNumberGetValue((CFNumberRef) object, kCFNumberIntType, &minValue))
                        continue;
                    
                    if (usagePage == kHIDPage_Button && usage > 0 && usage <= 32)
                    {
                        pInfo->button[usage-1].cookie = cookie;
                        pInfo->button[usage-1].eButton = EButton(BUTTON_JOY1+usage-1);
                    }
                    else if (usagePage == kHIDPage_GenericDesktop && usage >= kHIDUsage_GD_X && usage <= kHIDUsage_GD_Wheel)
                    {
                        pInfo->axis[usage-kHIDUsage_GD_X].cookie = cookie;
                        pInfo->axis[usage-kHIDUsage_GD_X].iMax = maxValue;
                        pInfo->axis[usage-kHIDUsage_GD_X].iMin = minValue;
                    }
                    else if (usagePage == kHIDPage_GenericDesktop && usage == kHIDUsage_GD_Hatswitch)
                    {
                        pInfo->hat.cookie = cookie;
                        int iDelta((maxValue-minValue+1)/4);
                        pInfo->hat.iMin = minValue;
                        pInfo->hat.iMax = maxValue;
                        pInfo->hat.iUp = minValue;
                        pInfo->hat.iLeft = minValue + iDelta;
                        pInfo->hat.iDown = minValue + 2 * iDelta;
                        pInfo->hat.iLeft = minValue + 3 * iDelta;
                    }
                }
            }
            
            // assign axes
            EAxis eAxis(AXIS_JOY_X);
            for (int i(0); i < 9 && eAxis < NUM_AXES; ++i)
            {
                if (pInfo->axis[i].cookie)
                {
                    pInfo->axis[i].eAxis = eAxis;
                    eAxis = EAxis(eAxis+1);
                }
                
            }
            
            (*hidDeviceInterface)->open(hidDeviceInterface, 0);
            pInfo->hidDeviceInterface = hidDeviceInterface;
            
            // set joyDeviceID based on joyName
            if (pInfo->sName == sys_joyName)
            {
                input_joyDeviceID = iJoystick;
                input_joyDeviceID.SetModified(false);
            }
            
            m_mapJoysticks[iJoystick++] = pInfo;
        }
    }
}


/*====================
 CSystem::~CSystem
 ====================*/
CSystem::~CSystem()
{
    // Restore screensaver if it was shut down
    
    // Restore the original mouse clipping area,
    // in case the program has altered it
}

static inline EButton mouseButtonToEButton(int button, uint modifierFlags)
{
    switch (button)
    {
        case 0:
            if (modifierFlags & NSEventModifierFlagCommand)
                return BUTTON_MOUSER;
            else
                return BUTTON_MOUSEL;
        case 1:
            return BUTTON_MOUSER;
        case 2:
            return BUTTON_MOUSEM;
        case 3:
            return BUTTON_MOUSEX1;
        case 4:
            return BUTTON_MOUSEX2;
    }
    return BUTTON_INVALID;
}

static inline EButton keyCodeToEButton(ushort unKeyCode)
{
    EButton button(BUTTON_INVALID);
    keyboardMap::iterator findit(g_KeyboardMap.find(unKeyCode));
    if (findit != g_KeyboardMap.end())
        button = findit->second;
    return button;
}

static inline void handleModifiers(uint uiModifierFlags)
{
    static uint uiOldModifierFlags(0);
    if (key_debugEvents)
        Console << _T("Modifier flags: ") << XtoA(uiModifierFlags, FMT_PADZERO | FMT_DELIMIT, 32, 2) << newl;
#if 1
    uint uiChanged(uiOldModifierFlags ^ uiModifierFlags);
    
    if (key_splitShift)
    {
        if (uiChanged & NX_DEVICELSHIFTKEYMASK)
            Input.AddEvent(BUTTON_LSHIFT, uiModifierFlags & NX_DEVICELSHIFTKEYMASK);
        if (uiChanged & NX_DEVICERSHIFTKEYMASK)
            Input.AddEvent(BUTTON_RSHIFT, uiModifierFlags & NX_DEVICERSHIFTKEYMASK);
    }
    else if (uiChanged & NSEventModifierFlagShift)
        Input.AddEvent(BUTTON_SHIFT, uiModifierFlags & NSEventModifierFlagShift);
    
    if (key_splitControl)
    {
        if (uiChanged & NX_DEVICELCTLKEYMASK)
            Input.AddEvent(BUTTON_LCTRL, uiModifierFlags & NX_DEVICELCTLKEYMASK);
        if (uiChanged & NX_DEVICERCTLKEYMASK)
            Input.AddEvent(BUTTON_RCTRL, uiModifierFlags & NX_DEVICERCTLKEYMASK);
    }
    else if (uiChanged & NSEventModifierFlagControl)
        Input.AddEvent(BUTTON_CTRL, uiModifierFlags & NSEventModifierFlagControl);

    if (key_splitOption)
    {
        if (uiChanged & NX_DEVICELALTKEYMASK)
            Input.AddEvent(BUTTON_LOPT, uiModifierFlags & NX_DEVICELALTKEYMASK);
        if (uiChanged & NX_DEVICERALTKEYMASK)
            Input.AddEvent(BUTTON_ROPT, uiModifierFlags & NX_DEVICERALTKEYMASK);
    }
    else if (uiChanged & NSEventModifierFlagOption)
        Input.AddEvent(BUTTON_OPT, uiModifierFlags & NSEventModifierFlagOption);
    
    if (key_splitCommand)
    {
        if (uiChanged & NX_DEVICELCMDKEYMASK)
            Input.AddEvent(BUTTON_LCMD, uiModifierFlags & NX_DEVICELCMDKEYMASK);
        if (uiChanged & NX_DEVICERCMDKEYMASK)
            Input.AddEvent(BUTTON_RCMD, uiModifierFlags & NX_DEVICERCMDKEYMASK);
    }
    else if (uiChanged & NSEventModifierFlagCommand)
        Input.AddEvent(BUTTON_CMD, uiModifierFlags & NSEventModifierFlagCommand);
        
    if (uiChanged & NSEventModifierFlagCapsLock)
        Input.AddEvent(BUTTON_CAPS_LOCK, uiModifierFlags & NSEventModifierFlagCapsLock);
    
    if (uiChanged & NSEventModifierFlagFunction)
        Input.AddEvent(BUTTON_FN, uiModifierFlags & NSEventModifierFlagFunction);
        
#else
    switch ((uiOldModifierFlags ^ uiModifierFlags) & 0xffff)
    {
            
        case NX_DEVICELSHIFTKEYMASK:
            if (key_splitShift)
                Input.AddEvent(BUTTON_LSHIFT, uiModifierFlags & NX_DEVICELSHIFTKEYMASK);
            else
                Input.AddEvent(BUTTON_SHIFT, uiModifierFlags & NX_DEVICELSHIFTKEYMASK);
            break;
            
        case NX_DEVICERSHIFTKEYMASK:
            if (key_splitShift)
                Input.AddEvent(BUTTON_RSHIFT, uiModifierFlags & NX_DEVICERSHIFTKEYMASK);
            else
                Input.AddEvent(BUTTON_RSHIFT, uiModifierFlags & NX_DEVICERSHIFTKEYMASK);
            break;
            
        case NX_DEVICELCTLKEYMASK:
            if (key_splitControl)
                Input.AddEvent(BUTTON_LCTRL, uiModifierFlags & NX_DEVICELCTLKEYMASK);
            else
                Input.AddEvent(BUTTON_CTRL, uiModifierFlags & NX_DEVICELCTLKEYMASK);
            break;
            
        case NX_DEVICERCTLKEYMASK:
            if (key_splitControl)
                Input.AddEvent(BUTTON_RCTRL, uiModifierFlags & NX_DEVICERCTLKEYMASK);
            else
                Input.AddEvent(BUTTON_CTRL, uiModifierFlags & NX_DEVICERCTLKEYMASK);
            break;
            
        case NX_DEVICELALTKEYMASK:
            if (key_splitOption)
                Input.AddEvent(BUTTON_LOPT, uiModifierFlags & NX_DEVICELALTKEYMASK);
            else
                Input.AddEvent(BUTTON_OPT, uiModifierFlags & NX_DEVICELALTKEYMASK);
            break;
            
        case NX_DEVICERALTKEYMASK:
            if (key_splitOption)
                Input.AddEvent(BUTTON_ROPT, uiModifierFlags & NX_DEVICERALTKEYMASK);
            else
                Input.AddEvent(BUTTON_OPT, uiModifierFlags & NX_DEVICERALTKEYMASK);
            break;
            
        case NX_DEVICELCMDKEYMASK:
            if (key_splitCommand)
                Input.AddEvent(BUTTON_LCMD, uiModifierFlags & NX_DEVICELCMDKEYMASK);
            else
                Input.AddEvent(BUTTON_CMD, uiModifierFlags & NX_DEVICELCMDKEYMASK);
            break;
            
        case NX_DEVICERCMDKEYMASK:
            if (key_splitCommand)
                Input.AddEvent(BUTTON_RCMD, uiModifierFlags & NX_DEVICERCMDKEYMASK);
            else
                Input.AddEvent(BUTTON_CMD, uiModifierFlags & NX_DEVICERCMDKEYMASK);
            break;
            
        default:
            switch ((uiOldModifierFlags ^ uiModifierFlags) & 0xffff0000)
        {
            case NSEventModifierFlagCapsLock:
                Input.AddEvent(BUTTON_CAPS_LOCK, uiModifierFlags & NSEventModifierFlagCapsLock);
                break;
            case NSEventModifierFlagShift:
                Input.AddEvent(BUTTON_SHIFT, uiModifierFlags & NSEventModifierFlagShift);
                break;
            case NSEventModifierFlagControl:
                Input.AddEvent(BUTTON_CTRL, uiModifierFlags & NSEventModifierFlagControl);
                break;
            case NSEventModifierFlagOption:
                Input.AddEvent(BUTTON_OPT, uiModifierFlags & NSEventModifierFlagOption);
                break;
            case NSEventModifierFlagCommand:
                Input.AddEvent(BUTTON_CMD, uiModifierFlags & NSEventModifierFlagCommand);
                break;
                // case NSEventModifierFlagNumericPad: // this is not num lock, it shows up as "clear" keypress
                // case NSEventModifierFlagHelp:
            case NSEventModifierFlagFunction: // INS on a windows keyboard
                Input.AddEvent(BUTTON_FN, uiModifierFlags & NSEventModifierFlagFunction);
                break;
            default:
                break;
        }
            break;
    }
#endif
    uiOldModifierFlags = uiModifierFlags;
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
    if ((c >= 0x20 && c < 0x7f) || (c > 0xa0 && c <= 0xff))
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
    PROFILE("CSystem::HandleOSMessages");
    
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
    
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSEvent *event;
    
    while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES]) != nil)
    {
        NSEventType type = [event type];
        NSWindow *win = [event window];
        if (win == nil && !Vid.IsFullScreen())
        {
            [NSApp sendEvent:event];
            continue;
        }
        switch (type)
        {
            case NSEventTypeKeyDown:
            case NSEventTypeKeyUp:
            {
                ushort unKeyCode = [event keyCode];
                if (key_debugEvents)
                    Console << _T("KeyCode: ") << INT_HEX_STR(unKeyCode) << (type == NSEventTypeKeyDown ? _T(" pressed") : _T(" released")) << newl;
                EButton button(keyCodeToEButton(unKeyCode));
                // Ensure that menu events get passed on
                if (Input.IsCommandDown() &&
                    ((Input.IsShiftDown() && button == EButton('Q'))
                     || (Input.IsShiftDown() && button == EButton('E'))
                     || button == EButton('M')
                     || button == EButton('H')
                     || button == EButton('F')))
                {
                    [NSApp sendEvent:event];
                    break;
                }
                if (button != BUTTON_INVALID)
                    Input.AddEvent(button, type == NSEventTypeKeyDown);
                if (type == NSEventTypeKeyDown && !Input.IsCommandDown() && !Input.IsCtrlDown()) // ignore chars typed when command or control is down
                {
                    NSString *chars = [event characters];
                    string sCharsUTF8([chars UTF8String]);
                    tstring sChars(UTF8ToTString(sCharsUTF8));
                    if (key_debugEvents)
                    {
                        Console << _T("Chars: "); // << sChars; // TKTK: This breaks when pressing F8
                        for (tstring::iterator it(sChars.begin()); it != sChars.end(); ++it)
                            Console << _T(", ") << XtoA(uint(*it));
                        Console << newl;
                    }
                    for (tstring::iterator it(sChars.begin()); it != sChars.end(); ++it)
                        if (unsigned(*it) >= 0x20 && unsigned(*it) != 0x7f // filter out control chars
#ifdef UNICODE
                            && (*it < 0xe000 || *it > 0xf8ff) // filter out the first private use plane (arrow keys, produce values here)
#endif
                            )
                            Input.AddEvent(*it);
                }
            }
                break;
                
            case NSEventTypeLeftMouseDown:
            case NSEventTypeRightMouseDown:
            case NSEventTypeOtherMouseDown:
            case NSEventTypeLeftMouseUp:
            case NSEventTypeRightMouseUp:
            case NSEventTypeOtherMouseUp:
            {
                NSPoint mousePosition;
                if (!win)
                {
                    CRecti viewrec = GetWindowArea();
                    mousePosition = [event locationInWindow];
                    mousePosition.x -= viewrec.left;
                    mousePosition.y = viewrec.bottom - mousePosition.y + viewrec.top;
                    /*CGDirectDisplayID *display = (CGDirectDisplayID*)Vid.GetHWnd();
                     if (display)
                     {
                     CGRect displayBounds(CGDisplayBounds(*display));
                     mousePosition = [event locationInWindow];
                     mousePosition.y = displayBounds.size.height - mousePosition.y;
                     }*/                    
                }
                else
                {
//                    mousePosition = [[win contentView] convertPointToBacking:[event locationInWindow]]; // TKTK TODO: Was this for non-retina screens? [event locationInWindow] seems to work...
                    mousePosition = [event locationInWindow];
                    mousePosition.y = [[win contentView] frame].size.height - mousePosition.y;
                }

                int mouseButton = [event buttonNumber];
                if (key_debugEvents)
                    Console << _T("Mouse button number ") << mouseButton << (type & 1 ? _T(" pressed at ") : _T(" released at ")) << ParenStr(XtoA(mousePosition.x) + _T(",") + XtoA(mousePosition.y)) << newl;
                EButton button(mouseButtonToEButton(mouseButton, [event modifierFlags]));
                if (button != BUTTON_INVALID)
                    Input.AddEvent(button, type & 1, CVec2f(mousePosition.x, mousePosition.y));
                [NSApp sendEvent:event];
            }
                break;
                
            case NSEventTypeScrollWheel:
            {
                NSPoint mousePosition;
                if (!win)
                {
                    CRecti viewrec = GetWindowArea();
                    mousePosition = [event locationInWindow];
                    mousePosition.x -= viewrec.left;
                    mousePosition.y = viewrec.bottom - mousePosition.y + viewrec.top;
                    /*CGDirectDisplayID *display = (CGDirectDisplayID*)Vid.GetHWnd();
                    if (display)
                    {
                        CGRect displayBounds(CGDisplayBounds(*display));
                        mousePosition = [event locationInWindow];
                        mousePosition.y = displayBounds.size.height - mousePosition.y;
                    }*/
                }
                else
                {
                    mousePosition = [[win contentView] convertPoint:[event locationInWindow] fromView:nil];
                    mousePosition.y = [[win contentView] frame].size.height - mousePosition.y;
                }
                
                float dX = [event deltaX];
                float dY = [event deltaY];
                float dZ = [event deltaZ];
                if (key_debugEvents)
                    Console << _T("Mousewheel ") << dX << " " << dY << " " << dZ << newl;
                if (dY > 0)
                {
                    Input.AddEvent(BUTTON_WHEELUP, true, CVec2f(mousePosition.x, mousePosition.y));
                    Input.AddEvent(BUTTON_WHEELUP, false, CVec2f(mousePosition.x, mousePosition.y));
                }
                else if (dY < 0)
                {
                    Input.AddEvent(BUTTON_WHEELDOWN, true, CVec2f(mousePosition.x, mousePosition.y));
                    Input.AddEvent(BUTTON_WHEELDOWN, false, CVec2f(mousePosition.x, mousePosition.y));
                }
                if (dX > 0)
                {
                    Input.AddEvent(BUTTON_WHEELRIGHT, true, CVec2f(mousePosition.x, mousePosition.y));
                    Input.AddEvent(BUTTON_WHEELRIGHT, false, CVec2f(mousePosition.x, mousePosition.y));
                }
                else if (dX < 0)
                {
                    Input.AddEvent(BUTTON_WHEELLEFT, true, CVec2f(mousePosition.x, mousePosition.y));
                    Input.AddEvent(BUTTON_WHEELLEFT, false, CVec2f(mousePosition.x, mousePosition.y));
                }
            }
                break;
                
            case NSEventTypeFlagsChanged:
            {
                handleModifiers([event modifierFlags]);
            }
                break;
            case NSEventTypeAppKitDefined:
                [NSApp sendEvent:event];
                // restore custom cursor in exclusive mode fullscreen, doing it with this event seems to catch all cases where it gets clobbered
                if ([event subtype] == 0)
                    Vid.SetCursor(ResHandle(-2));
                break;
                
            case NSEventTypeMouseMoved:
            case NSEventTypeLeftMouseDragged:
            case NSEventTypeRightMouseDragged:
            case NSEventTypeOtherMouseDragged:
                // using polling -- may want to use events for relative motion
                
            case NSEventTypeMouseEntered:
            case NSEventTypeMouseExited:
                
            case NSEventTypeSystemDefined:
            case NSEventTypeApplicationDefined:
            case NSEventTypePeriodic:
            case NSEventTypeCursorUpdate:
            default:
                [NSApp sendEvent:event];
        }
    }
    
    [pool release];
}


/*====================
 CSystem::Exit
 ====================*/
void    CSystem::Exit(int iErrorLevel)
{
#ifndef _S2_EXPORTER
    if (sys_autoSaveConfig &&!Host.GetNoConfig())
    {
        tsvector vArgs;
        vArgs.push_back(_T("~/startup.cfg"));
        
        CConsoleElement *pElem = ConsoleRegistry.GetElement(_T("WriteConfigScript"));
        if (pElem != NULL)
            pElem->Execute(vArgs);
    }
    
    if (m_ConsoleWindowHandle)
    {
        CCursesConsole *pConsole(static_cast<CCursesConsole*>(m_ConsoleWindowHandle));
        delete pConsole;
    }
    

    Vid.Shutdown();
    Host.Shutdown();
    Console.CloseLog();
    
    if (m_bRestartProcess)
    {
        // Restart the game via the updater
        const char *argv[] = { "HoN_Update", NULL };
        // need to fork() first since under OS X can't exec in an app that has multiple threads...
        if (fork() == 0)
            execv(argv[0], (char**)argv);
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
    fflush(stdout);
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
 ProcessFSEvents
 ====================*/
void ProcessFSEvents(const std::vector<fsw::event>& events, void *)
{
    CScopedLock cLock(g_FileMonitorMutex);
    for (auto& event : events)
    {
        K2System.AddModifiedPath(NativeToTString(event.get_path()));
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
 CSystem::StartDirectoryMonitoring
 ====================*/
void    CSystem::StartDirectoryMonitoring()
{
    if (!host_dynamicResReload && m_pFileMonitor)
        StopDirectoryMonitoring();
    if (host_dynamicResReload && !m_pFileMonitor)
    {
        // Set up file monitoring
        m_pFileMonitor = fsw::monitor_factory::create_monitor(
                fsw_monitor_type::system_default_monitor_type,
                GetModPaths(),
                ProcessFSEvents);
        m_pFileMonitor->set_latency(0.25);
        m_pFileMonitor->set_recursive(true);
        m_pFileMonitor->set_follow_symlinks(true);
#if defined(HAVE_FSEVENTS_FSEVENTSTREAMSETDISPATCHQUEUE)
        m_pFileMonitor->set_property(std::string(fsw::fsevents_monitor::DARWIN_EVENTSTREAM_NO_DEFER), "true");
#endif
        std::vector<fsw_event_type_filter> vEventFilters;
        for (const auto& item : FSW_ALL_EVENT_FLAGS)
        {
            vEventFilters.push_back({item});
        }
        m_pFileMonitor->set_event_type_filters(vEventFilters);
        assert(g_pFileMonitorThread == nullptr);
        g_pFileMonitorThread = K2_NEW(ctx_System, std::thread)([=](){
            m_pFileMonitor->start();
        });
        Console << _T("Directory monitoring started") << newl;
    }
}


/*====================
 CSystem::GetModifiedFileList
 ====================*/
void    CSystem::GetModifiedFileList(tsvector &vFileList)
{
    CScopedLock cLock(g_FileMonitorMutex);

    if (!m_setsModifiedFiles.empty()) {
        auto vModPaths = GetModPaths();

        for (auto& sFullPath : m_setsModifiedFiles)
        {
            for (const auto& sModPath : vModPaths) {
                tstring tsModPath = NativeToTString(sModPath) + _T("/");
                if (sFullPath.find(tsModPath) == 0) {
                    auto sPath = sFullPath.substr(tsModPath.size() - 1);
                    vFileList.push_back(sPath);
                }
            }
        }
        m_setsModifiedFiles.clear();
    }
}


/*====================
 CSystem::StopDirectoryMonitoring
 ====================*/
void    CSystem::StopDirectoryMonitoring()
{
    if (m_pFileMonitor)
        m_pFileMonitor->stop();
    if (g_pFileMonitorThread)
        g_pFileMonitorThread->join();
    if (m_pFileMonitor)
        Console << _T("Directory monitoring stopped") << newl;
    SAFE_DELETE(g_pFileMonitorThread);
    SAFE_DELETE(m_pFileMonitor);
}


/*====================
 CSystem::Error
 ====================*/
void    CSystem::Error(const tstring &sMsg)
{
    Vid.Shutdown(); // shutdown video so that error message will always appear (would be hidden behind fullscreen window otherwise)
    
    NSAlert *pAlert = [[NSAlert alloc] init];
    [pAlert setMessageText:@"Heroes of Newerth - Fatal Error"];
    [pAlert setInformativeText:[NSString stringWithUTF8String:TStringToNative(sMsg).c_str()]];
    [pAlert runModal];
    [pAlert release];
    
    Exit(-1);
}


/*====================
 CSystem::Milliseconds
 ====================*/
uint    CSystem::Milliseconds()
{
    return uint((GetTicks() - m_llStartTicks) / (GetFrequency() / 1000));
}


/*====================
 CSystem::Microseconds
 ====================*/
uint    CSystem::Microseconds()
{
    return uint((GetTicks() - m_llStartTicks) / (GetFrequency() / 1000000));
}


/*====================
 CSystem::GetTicks
 ====================*/
LONGLONG    CSystem::GetTicks() const
{
    return mach_absolute_time();
}


/*====================
 CSystem::GetFrequency
 ====================*/
LONGLONG    CSystem::GetFrequency()
{
    return m_llFrequency;
}

/*====================
 CSystem::GetWindowArea - returns origin of window (bot left) in NS coordinates: (0,1) origin at bot left primary display
 ====================*/
CRecti  CSystem::GetWindowArea()
{
    if (!m_WindowHandle)
    {
        CGDirectDisplayID *display = (CGDirectDisplayID*)Vid.GetHWnd();
        CGRect primaryDisplayBounds(CGDisplayBounds(kCGDirectMainDisplay));
        if (display)
        {
            CGRect displayBounds(CGDisplayBounds(*display));
            return CRecti(displayBounds.origin.x,
                          primaryDisplayBounds.size.height - displayBounds.size.height - displayBounds.origin.y,
                          displayBounds.size.width, displayBounds.size.height);
        }
        else
        {
            return CRecti(0, 0, primaryDisplayBounds.size.width, primaryDisplayBounds.size.height);
        }
    }
    else
    {
        NSRect windowRect = [(NSWindow*)m_WindowHandle frame];
        NSRect viewRect = [[(NSWindow*)m_WindowHandle contentView] frame];
        return CRecti(windowRect.origin.x + viewRect.origin.x, windowRect.origin.y + viewRect.origin.y,
                      viewRect.size.width, viewRect.size.height);
    }
}


/*====================
 CSystem::SetMousePos
 ====================*/
void    CSystem::SetMousePos(int x, int y)
{
    CGPoint newCursorPosition;
    CGRect displayBounds(CGDisplayBounds(kCGDirectMainDisplay));
    CRecti viewrec = GetWindowArea();
    newCursorPosition.x = x + viewrec.left;
    newCursorPosition.y = y + displayBounds.size.height - viewrec.bottom - viewrec.top;
    /*if (!m_WindowHandle)
    {
        CGDirectDisplayID *display = (CGDirectDisplayID*)Vid.GetHWnd();
    
        if (display)
            displayBounds = CGDisplayBounds(*display);
        else
            displayBounds = CGDisplayBounds(kCGDirectMainDisplay);
        CRecti viewrec = GetWindowArea();
        newCursorPosition.x = viewrec.left + x;
        newCursorPosition.y = displayBounds.size.height - (viewrec.top + (viewrec.bottom - y));
    }
    else
    {
        displayBounds = CGDisplayBounds(kCGDirectMainDisplay);
        newCursorPosition = [(NSWindow*)m_WindowHandle convertBaseToScreen:NSMakePoint(x, y)];
        CRecti viewrec = GetWindowArea();
        newCursorPosition.y = displayBounds.size.height - newCursorPosition.y;
    }*/

#if TKTK || 1 // This is deprecated
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    CGSetLocalEventsSuppressionInterval(0.0);
    CGWarpMouseCursorPosition(*(CGPoint*)&newCursorPosition);
#pragma clang diagnostic pop
#else
    // https://stackoverflow.com/questions/10196603/using-cgeventsourcesetlocaleventssuppressioninterval-instead-of-the-deprecated
    CGWarpMouseCursorPosition(*(CGPoint*)&newCursorPosition);
    CGAssociateMouseAndMouseCursorPosition(true);
#endif
}


/*====================
 CSystem::GetMousePos
 ====================*/
CVec2i  CSystem::GetMousePos()
{
    CRecti viewrec = GetWindowArea();
    NSPoint point = [NSEvent mouseLocation];
    return CVec2i(point.x - viewrec.left, viewrec.bottom + viewrec.top - point.y);
}


/*====================
 CSystem::ShowMouseCursor
 ====================*/
void    CSystem::ShowMouseCursor()
{
    Vid.ShowCursor(true);
}


/*====================
 CSystem::HideMouseCursor
 ====================*/
void    CSystem::HideMouseCursor()
{
    Vid.ShowCursor(false);
}


/*====================
 CSystem::SetMouseClipping
 ====================*/
void    CSystem::SetMouseClipping(const CRecti &recClip)
{
    m_recMouseClip = recClip;
    m_recMouseClip.left = CLAMP(recClip.left, 0, Vid.GetScreenW()-1);
    m_recMouseClip.right = CLAMP(recClip.right, 0, Vid.GetScreenW()-1);
    m_recMouseClip.top = CLAMP(recClip.top, 0, Vid.GetScreenH()-1);
    m_recMouseClip.bottom = CLAMP(recClip.bottom, 0, Vid.GetScreenH()-1);
    m_bMouseClip = true;
    
}


/*====================
 CSystem::UnsetMouseClipping
 ====================*/
void    CSystem::UnsetMouseClipping()
{
    m_bMouseClip = false;
}


/*====================
 CSystem::PollMouse
 ====================*/
void    CSystem::PollMouse()
{
    CVec2i v2Point(GetMousePos());
    int x(v2Point.x);
    int y(v2Point.y);
    
    /*if (sys_constrainCursor && !m_bMouseClip)
    {
        CRecti viewrec = GetWindowArea();
        m_recMouseClip = CRecti(0, 0, viewrec.right, viewrec.bottom);
    }*/
    
    if (m_bHasFocus && m_bMouseClip)
    {
        if (!m_recMouseClip.Contains(v2Point))
        {
            x = CLAMP(x, m_recMouseClip.left, m_recMouseClip.right);
            y = CLAMP(y, m_recMouseClip.top, m_recMouseClip.bottom);
        }
        
        if (sys_warpCursor && (x != v2Point.x || y != v2Point.y))
        {
            SetMousePos(x, y);
        }
    }
    
    if (v2Point.x != INT_ROUND(Input.GetAxisState(AXIS_MOUSE_X)))
        Input.AddEvent(AXIS_MOUSE_X, x, v2Point.x - Input.GetAxisState(AXIS_MOUSE_X));
    
    if (v2Point.y != INT_ROUND(Input.GetAxisState(AXIS_MOUSE_Y)))
        Input.AddEvent(AXIS_MOUSE_Y, y, v2Point.y - Input.GetAxisState(AXIS_MOUSE_Y));
}


/*====================
 CSystem::PollJoysticks
 ====================*/
void    CSystem::PollJoysticks(uint uiID)
{
    if (input_joyDeviceID.IsModified())
    {
        if (input_joyDeviceID != -1)
        {
            if (m_mapJoysticks.count(input_joyDeviceID))
                sys_joyName = m_mapJoysticks[input_joyDeviceID]->sName;
            else
                input_joyDeviceID = -1;
        }
        input_joyDeviceID.SetModified(false);
    }
    
    if (m_mapJoysticks.count(uiID) == 0)
        return;
    
    SJoystickInfo *pInfo = m_mapJoysticks[uiID];
    
    // Analog Axes
    for (int i(0); i < 9; ++i)
    {
        if (pInfo->axis[i].eAxis != AXIS_INVALID)
        {
            IOHIDEventStruct event;
            (*(pInfo->hidDeviceInterface))->queryElementValue(pInfo->hidDeviceInterface, pInfo->axis[i].cookie, &event, 0, NULL, NULL, NULL);
            
            float fAxisValue(CLAMP(event.value / (float)pInfo->axis[i].iMax, -1.0f, 1.0f));
            float fAxisDelta;
            if (pInfo->axis[i].iMin < 0) // centred at 0, [-1,1]
                fAxisDelta = fAxisValue;
            else // centred at iMax/2, [0,1]
                fAxisDelta = 2.0f * (fAxisValue - 0.5f);
            Input.AddEvent(pInfo->axis[i].eAxis, fAxisValue, fAxisDelta);
            
            m_JoystickState.fAxis[pInfo->axis[i].eAxis-AXIS_JOY_X] = fAxisDelta;
        }
    }
    
    // Buttons
    uint uiButtonStates(0);
    for (int i(0); i < 32; ++i)
    {
        if (pInfo->button[i].eButton != BUTTON_INVALID)
        {
            IOHIDEventStruct event;
            (*(pInfo->hidDeviceInterface))->queryElementValue(pInfo->hidDeviceInterface, pInfo->button[i].cookie, &event, 0, NULL, NULL, NULL);
            
            if (event.value != 0)
                uiButtonStates |= BIT(i);
            
            if ((m_JoystickState.uiButtons & BIT(i)) != (uiButtonStates & BIT(i)))
                Input.AddEvent(pInfo->button[i].eButton, event.value != 0);
        }
    }
    m_JoystickState.uiButtons = uiButtonStates;
    
    // Hat
    if (pInfo->hat.cookie)
    {
#define POV_UP(pov)     (((pov) <= pInfo->hat.iMax || (pov) >= pInfo->hat.iMin) && (((pov) >= pInfo->hat.iUp && (pov) < pInfo->hat.iRight) || (pov) > pInfo->hat.iLeft))
#define POV_RIGHT(pov)  (((pov) <= pInfo->hat.iMax || (pov) >= pInfo->hat.iMin) && ((pov) > pInfo->hat.iUp && (pov) < pInfo->hat.iDown))
#define POV_DOWN(pov)   (((pov) <= pInfo->hat.iMax || (pov) >= pInfo->hat.iMin) && ((pov) > pInfo->hat.iRight && (pov) < pInfo->hat.iLeft))
#define POV_LEFT(pov)   (((pov) <= pInfo->hat.iMax || (pov) >= pInfo->hat.iMin) && ((pov) > pInfo->hat.iDown))
        
        IOHIDEventStruct event;
        (*(pInfo->hidDeviceInterface))->queryElementValue(pInfo->hidDeviceInterface, pInfo->hat.cookie, &event, 0, NULL, NULL, NULL);
        if (POV_UP(event.value) && !POV_UP(m_JoystickState.uiPOV))
            Input.AddEvent(BUTTON_JOY_POV_UP, true);
        else if (!POV_UP(event.value) && POV_UP(m_JoystickState.uiPOV))
            Input.AddEvent(BUTTON_JOY_POV_UP, false);
        if (POV_RIGHT(event.value) && !POV_RIGHT(m_JoystickState.uiPOV))
            Input.AddEvent(BUTTON_JOY_POV_RIGHT, true);
        else if (!POV_RIGHT(event.value) && POV_RIGHT(m_JoystickState.uiPOV))
            Input.AddEvent(BUTTON_JOY_POV_RIGHT, false);
        if (POV_DOWN(event.value) && !POV_DOWN(m_JoystickState.uiPOV))
            Input.AddEvent(BUTTON_JOY_POV_DOWN, true);
        else if (!POV_DOWN(event.value) && POV_DOWN(m_JoystickState.uiPOV))
            Input.AddEvent(BUTTON_JOY_POV_DOWN, false);
        if (POV_LEFT(event.value) && !POV_LEFT(m_JoystickState.uiPOV))
            Input.AddEvent(BUTTON_JOY_POV_LEFT, true);
        else if (!POV_LEFT(event.value) && POV_LEFT(m_JoystickState.uiPOV))
            Input.AddEvent(BUTTON_JOY_POV_LEFT, false);
        m_JoystickState.uiPOV = event.value;
        
#undef POV_UP
#undef POV_RIGHT
#undef POV_DOWN
#undef POV_LEFT
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
    if (m_mapJoysticks.count(uiID) == 0)
        return false;
    
    SJoystickInfo *pInfo = m_mapJoysticks[uiID];
    
    for (int i(0); i < 9; ++i)
    {
        if (pInfo->axis[i].eAxis == axis)
            return true;
    }
    
    return false;
}


/*====================
 CSystem::GetJoystickList
 ====================*/
void    CSystem::GetJoystickList(imaps &mapNames)
{
    for (map<int, SJoystickInfo*>::iterator it(m_mapJoysticks.begin()); it != m_mapJoysticks.end(); ++it)
    {
        mapNames[it->first] = it->second->sName;
    }
}


/*====================
 CSystem::CopyToClipboard
 ====================*/
void    CSystem::CopyToClipboard(const tstring &sText)
{
    NSPasteboard *pClipboard = [NSPasteboard generalPasteboard];
    NSArray *pTypes = [NSArray arrayWithObjects:NSPasteboardTypeString, nil];
    [pClipboard declareTypes:pTypes owner:nil];
    [pClipboard setString:[NSString stringWithUTF8String:(TStringToNative(sText).c_str())] forType:NSPasteboardTypeString];
}


/*====================
 CSystem::IsClipboardString
 ====================*/
bool    CSystem::IsClipboardString()
{
    NSArray *pTypes = [NSArray arrayWithObjects:NSPasteboardTypeString, nil];
    NSString *pBestType = [[NSPasteboard generalPasteboard] availableTypeFromArray:pTypes];
    
    return (pBestType != nil);
}


/*====================
 CSystem::GetClipboardString
 ====================*/
tstring CSystem::GetClipboardString()
{
    NSString *pString = [[NSPasteboard generalPasteboard] stringForType:NSPasteboardTypeString];
    
    if (pString == nil)
        return _T("");
    
    return NativeToTString([pString UTF8String]);
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
    int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid() };
    struct kinfo_proc info;
    size_t size = sizeof(info);
    
    info.kp_proc.p_flag = 0;

    sysctl(mib, 4, &info, &size, NULL, 0);
    
    return (info.kp_proc.p_flag & P_TRACED) != 0;
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
 
 Returns a list of all active Savage 2 processes.
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
    printf("%ls", sText.c_str());
#else
    printf("%s", sText.c_str());
#endif
    fflush(stdout);
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
#if 1
    try
    {
        // OS
        SInt32 version[3] = { 0, 0, 0 };
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        Gestalt(gestaltSystemVersionMajor, &version[0]);
        Gestalt(gestaltSystemVersionMinor, &version[1]);
        Gestalt(gestaltSystemVersionBugFix, &version[2]);
#pragma clang diagnostic pop
        structInfo.sOS = _T("Mac OS X ") + XtoA(version[0]) + _T(".") + XtoA(version[1]) + _T(".") + XtoA(version[2]);
        
        // Memory
        int64_t mem;
        int mib[4] = { CTL_HW, HW_MEMSIZE, 0, 0 };
        size_t size = sizeof(int64_t);
        sysctl(mib, 2, &mem, &size, NULL, 0);
        structInfo.sRAM = XtoA(mem/(1024*1024)) + _T(" MB");
        
        // Processor
        char cpu[1024];
        int cputype;
        size = sizeof(char) * 1024;
        if (sysctlbyname("machdep.cpu.brand_string", cpu, &size, NULL, 0) == 0)
        {
            structInfo.sProcessor = UTF8ToTString(cpu);
        }
        else if (size = sizeof(int), sysctlbyname("hw.cputype", &cputype, &size, NULL, 0) == 0)
        {
            if (cputype == 18)
            {
                structInfo.sProcessor = _T("Power PC");
                if (size = sizeof(int), sysctlbyname("hw.cpusubtype", &cputype, &size, NULL, 0) == 0)
                {
                    if (cputype == 100)
                    {
                        structInfo.sProcessor += _T(" G5");
                    }
                    else if (cputype == 10 || cputype == 11)
                    {
                        structInfo.sProcessor += _T(" G4");
                    }
                    else if (cputype == 9)
                    {
                        structInfo.sProcessor += _T(" G3");
                    }
                }
            }
            else if (cputype == 7)
            {
                structInfo.sProcessor = _T("Intel");
            }
            uint64_t cpufreq;
            size = sizeof(uint64_t);
            if (sysctlbyname("hw.cpufrequency", &cpufreq, &size, NULL, 0) == 0)
            {
                structInfo.sProcessor = XtoA(float(cpufreq) / 1000000000u, 0, 0, 2) + _T(" GHz ") + structInfo.sProcessor;
            }
        }
        else // just return the arch
        {
            mib[1] = HW_MACHINE;
            size = sizeof(char) * 1024;
            sysctl(mib, 2, cpu, &size, NULL, 0);
            structInfo.sProcessor = UTF8ToTString(cpu);
        }
        
        // Video Card
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        io_registry_entry_t displayport = CGDisplayIOServicePort(kCGDirectMainDisplay);
#pragma clang diagnostic pop
        CFDataRef model = (CFDataRef)IORegistryEntrySearchCFProperty(displayport, kIOServicePlane, CFSTR("model"), kCFAllocatorDefault, kIORegistryIterateRecursively | kIORegistryIterateParents);
        if (model)
        {
            memcpy(cpu, CFDataGetBytePtr(model), MIN(CFDataGetLength(model), 1024));
            cpu[MIN(CFDataGetLength(model), 1023)] = 0;
            CFRelease(model);
            structInfo.sVideo = UTF8ToTString(cpu);
        }
        CFTypeRef num;
        num = IORegistryEntrySearchCFProperty(displayport, kIOServicePlane, CFSTR(kIOFBMemorySizeKey), kCFAllocatorDefault, kIORegistryIterateRecursively | kIORegistryIterateParents);
        if (num)
        {
            if (CFGetTypeID(num) == CFNumberGetTypeID())
            {
                CFNumberGetValue((CFNumberRef)num, kCFNumberSInt64Type, &mem);
                structInfo.sVideo += ParenStr(XtoA(mem / (1024 * 1024)) + _T(" MB"));
            }
            CFRelease(num);
        }
        
        // MAC Address
        // based on http://developer.apple.com/mac/library/samplecode/GetPrimaryMACAddress sample
        CFMutableDictionaryRef matchingDict = IOServiceMatching(kIOEthernetInterfaceClass);
        io_iterator_t matchingServices;
        if (matchingDict)
        {
            CFMutableDictionaryRef propertyMatchDict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
            if (propertyMatchDict)
            {
                CFDictionarySetValue(propertyMatchDict, CFSTR(kIOPrimaryInterface), kCFBooleanTrue);
                CFDictionarySetValue(matchingDict, CFSTR(kIOPropertyMatchKey), propertyMatchDict);
                CFRelease(propertyMatchDict);
                if (IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, &matchingServices) == KERN_SUCCESS)
                {
                    io_object_t intfService, controllerService;
                    while ((intfService = IOIteratorNext(matchingServices)))
                    {
                        CFTypeRef MACAddressAsCFData;
                        if (IORegistryEntryGetParentEntry(intfService, kIOServicePlane, &controllerService) == KERN_SUCCESS)
                        {
                            if ((MACAddressAsCFData = IORegistryEntryCreateCFProperty(controllerService, CFSTR(kIOMACAddress), kCFAllocatorDefault, 0)))
                            {
                                UInt8 macaddr[kIOEthernetAddressSize] = { 0 };
                                CFDataGetBytes((CFDataRef)MACAddressAsCFData, CFRangeMake(0, kIOEthernetAddressSize), macaddr);
                                CFRelease(MACAddressAsCFData);
                                structInfo.sMAC = UpperString(XtoA(macaddr[0], FMT_PADZERO | FMT_NOPREFIX, 2, 16));
                                for (int i(1); i < kIOEthernetAddressSize; ++i)
                                    structInfo.sMAC += _T(":") + UpperString(XtoA(macaddr[i], FMT_PADZERO | FMT_NOPREFIX, 2, 16));
                            }
                            IOObjectRelease(controllerService);
                        }
                    }
                    IOObjectRelease(intfService);
                    IOObjectRelease(matchingServices);
                }
            }
        }
        
        
        
        return structInfo;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CSystem::GetSystemInfo() - "), NO_THROW);
        return structInfo;
    }
#else
    return structInfo;
#endif
}


/*====================
 CSystem::GetTotalPhysicalMemory
 ====================*/
ULONGLONG   CSystem::GetTotalPhysicalMemory() const
{
    // TODO
    return 0;
}


/*====================
 CSystem::GetFreePhysicalMemory
 ====================*/
ULONGLONG   CSystem::GetFreePhysicalMemory() const
{
    // TODO
    return 0;
}


/*====================
 CSystem::GetTotalVirtualMemory
 ====================*/
ULONGLONG   CSystem::GetTotalVirtualMemory() const
{
    // TODO
    return 0;
}


/*====================
 CSystem::GetFreeVirtualMemory
 ====================*/
ULONGLONG   CSystem::GetFreeVirtualMemory() const
{
    // TODO
    return 0;
}


/*====================
 CSystem::GetTotalPageFile
 ====================*/
ULONGLONG   CSystem::GetTotalPageFile() const
{
    // TODO
    return 0;
}


/*====================
 CSystem::GetFreePageFile
 ====================*/
ULONGLONG   CSystem::GetFreePageFile() const
{
    // TODO
    return 0;
}


/*====================
 CSystem::GetProcessMemoryUsage
 ====================*/
ULONGLONG   CSystem::GetProcessMemoryUsage() const
{
    struct task_basic_info ti;
    mach_msg_type_number_t count(TASK_BASIC_INFO_COUNT);
    task_info(current_task(), TASK_BASIC_INFO, (task_info_t)&ti, &count);
    return ti.resident_size;
}


/*====================
 CSystem::GetProcessVirtualMemoryUsage
 ====================*/
ULONGLONG   CSystem::GetProcessVirtualMemoryUsage() const
{
    struct task_basic_info ti;
    mach_msg_type_number_t count(TASK_BASIC_INFO_COUNT);
    task_info(current_task(), TASK_BASIC_INFO, (task_info_t)&ti, &count);
    return ti.virtual_size;
}


/*====================
 CSystem::SetConfig
 ====================*/
void    CSystem::SetConfig(const tstring &sConfig)
{
    try
    {
        NSFileManager *pMgr = [NSFileManager defaultManager];
        NSArray *pPaths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
        if ([pPaths count] == 0)
            EX_WARN(_T("Unable to locate user Application Support Directory, settings will not be saved!"));
        
        m_sUserDir = NativeToTString([[pPaths objectAtIndex:0] UTF8String]);
        m_sUserDir += _T("/") + m_sGameName + _T("/");
        
        if (!sConfig.empty())
            m_sUserDir += sConfig + _T("/");
        
        NSString *pPath = [NSString stringWithUTF8String:TStringToNative(m_sUserDir).c_str()];
        if (![pMgr fileExistsAtPath:pPath])
            if (![pMgr createDirectoryAtPath:pPath withIntermediateDirectories:YES attributes:nil error:nil])
                EX_WARN(_T("Failed to create user directory, settings will not be saved!"));
    }
    catch (CException &ex)
    {
        ex.Process(_T("CSystem::SetConfig() - "), NO_THROW);
    }
}


/*====================
 CSystem::SetAffinity
 
 Set process affinity
 ====================*/
void    CSystem::SetAffinity(int iCPU)
{
    // Not supported under OS X:  Only can hint it at thread creation (and that was only introduced in OS X 10.5)
}


/*====================
 CSystem::GetAffinityMask
 ====================*/
uint    CSystem::GetAffinityMask() const
{
    // Not supported under OS X:  Only can hint it at thread creation (and that was only introduced in OS X 10.5)
    return 0;
}


/*====================
  CSystem::SetPriority
  ====================*/
void    CSystem::SetPriority(int iPriority)
{
    // TODO
}


/*====================
  CSystem::GetPriority
  ====================*/
int     CSystem::GetPriority() const
{
    // TODO
    return 0;
}


/*====================
 CSystem::GetProcessFilename
 ====================*/
tstring CSystem::GetProcessFilename()
{
    ProcessSerialNumber psn = { 0, kCurrentProcess };
    ProcessInfoRec pir;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    CFDictionaryRef pProcessInfo(ProcessInformationCopyDictionary(&psn, kProcessDictionaryIncludeAllInformationMask));
#pragma clang diagnostic pop
    tstring sReturn(TSNULL);
    if (CFDictionaryContainsKey(pProcessInfo, kCFBundleExecutableKey))
    {
        CFStringRef pName(CFStringRef(CFDictionaryGetValue(pProcessInfo, kCFBundleExecutableKey)));
        if (pName)
        {
            char szBuf[PATH_MAX];
            if (CFStringGetCString(pName, szBuf, PATH_MAX, kCFStringEncodingUTF8))
                sReturn = NativeToTString(szBuf);
        }
    }
    CFRelease(pProcessInfo);
    return sReturn;
}


/*====================
 CSystem::InitDedicatedConsole
 ====================*/
void    CSystem::InitDedicatedConsole()
{
    if (sys_dedicatedConsole && sys_interactive)
    {
        m_ConsoleWindowHandle = static_cast<void*>(new CCursesConsole());
        
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
}


/*====================
  CSystem::AnalyzeMemory
  ====================*/
void    CSystem::AnalyzeMemory()
{
    
}

//=============================================================================

