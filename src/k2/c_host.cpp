// (C)2005 S2 Games
// c_host.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_host.h"
#include "c_hostserver.h"
#include "c_hostclient.h"

#include "k2_api.h"
#include "c_date.h"
#include "c_vid.h"
#include "c_bitmap.h"
#include "c_fontmap.h"
#include "c_draw2d.h"
#include "c_input.h"
#include "c_movie.h"
#include "c_texture.h"
#include "c_xmlmanager.h"
#include "c_soundmanager.h"
#include "c_netdriver.h"
#include "c_uimanager.h"
#include "c_uitrigger.h"
#include "c_chatmanager.h"
#include "c_world.h"
#include "c_filehttp.h"
#include "c_updater.h"
#include "c_scenestats.h"
#include "c_clientlogin.h"
#include "c_updater.h"
#include "c_netstats.h"
#include "c_eventmanager.h"
#include "c_actionregistry.h"
#include "c_alias.h"
#include "c_gamebind.h"
#include "c_voicemanager.h"
#include "c_servermanager.h"
#include "c_uicmd.h"
#include "c_httpmanager.h"
#include "c_uitextureregistry.h"
#include "c_resourcemanager.h"
#include "c_resourceinfo.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
extern IResourceLibrary g_ResLibFontFace;
extern IResourceLibrary g_ResLibFontMap;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_BOOL       (host_sleep,                    false);
#ifdef _WIN32
CVAR_INT        (host_sleepMS,                  1);
#else
CVAR_INT        (host_sleepMS,                  0);
#endif
CVAR_BOOLF      (host_dynamicResReload,         true,       CVAR_SAVECONFIG);
CVAR_STRINGF    (host_date,                     "",         CVAR_READONLY);
CVAR_STRINGF    (host_time,                     "",         CVAR_READONLY);
CVAR_STRINGF    (host_version,                  "",         CVAR_READONLY);
#ifdef _WIN32
CVAR_STRING     (host_vidDriver,                "vid_gl2");
#else
CVAR_STRING     (host_vidDriver,                "vid_gl2");
#endif
CVAR_FLOATR     (host_timeScale,                1.0f,       CVAR_TRANSMIT,      0.01f,  100.0f);
CVAR_FLOATR     (host_replaySpeed,              1.0f,       0,                  0.0f,   10.0f);
CVAR_STRING     (host_autoexec,                 "");
CVAR_STRING     (host_init,                     "");
CVAR_BOOL       (host_debugInit,                false);
#ifdef _DEBUG
CVAR_INT        (host_drawFPS,                  1);
#else
CVAR_INT        (host_drawFPS,                  0);
#endif
CVAR_BOOL       (host_drawActiveClient,         false);
CVAR_STRINGF    (host_runOnce,                  "",         CVAR_SAVECONFIG);
CVAR_STRINGF    (host_config,                   "",         CVAR_READONLY);
CVAR_INTF       (host_affinity,                 0,          CVAR_SAVECONFIG);
CVAR_STRINGF    (host_language,                 "en",       CVAR_SAVECONFIG);
CVAR_INTR       (host_maxFPS,                   1000,       CVAR_SAVECONFIG,    1,      10000);
#if defined(_WIN32)
CVAR_STRINGF    (host_os,                       "windows",  CVAR_READONLY);
#elif defined(linux)
CVAR_STRINGF    (host_os,                       "linux",    CVAR_READONLY);
#elif defined(__APPLE__)
CVAR_STRINGF    (host_os,                       "macos",    CVAR_READONLY);
#endif

#ifdef _DEBUG
CVAR_STRINGF    (host_startupCfg,               "~/startup_d.cfg",  CVAR_READONLY);
#else
CVAR_STRINGF    (host_startupCfg,               "~/startup.cfg",    CVAR_READONLY);
#endif

const uint  VERSION_STAMP_FADE_TIME(2000);
const float VERSION_STAMP_SPACING(2.0f);
const float VERSION_STAMP_DROP_SHADOW(2.0f);

SINGLETON_INIT(CHost);
CHost *g_pHost(CHost::GetInstance());

CVAR_UINTF(     _system_ms, 0,  CVAR_READONLY);
CVAR_UINTF(     _host_ms,   0,  CVAR_READONLY);

UI_TRIGGER(FPS);
//=============================================================================

/*====================
  CHost::~CHost
  ====================*/
CHost::~CHost()
{
    StopClient();
    StopServer(false);

    m_pHTTPManager->Shutdown();

    K2_DELETE(m_pHTTPManager);
    
    cURL_Shutdown();
}


/*====================
  CHost::CHost
  ====================*/
CHost::CHost() :
m_pHTTPManager(NULL),

m_uiThisFrameEnd(0),
m_fThisFrameEndSeconds(0.0f),
m_uiThisFrameLength(0),
m_uiCurrentFrame(0),

m_uiLastFPSTime(0),
m_uiLastFPS(1),
m_uiFPSCount(0),

m_bReplay(false),

m_bShowVersionStamp(false),
m_uiVersionStampTime(INVALID_TIME),

m_pServer(NULL),
m_pCurrentClient(NULL),
m_pInterfaceExt(NULL),
m_uiActiveClient(-1),
m_uiNextClientID(0),

m_uiID(K2System.GetRandomSeed32()),

m_bResolutionChange(false),
m_bNoConfig(false),
m_wRegisterPort(0),
m_bSleeping(false),
m_bFirstFrame(false),
m_bShouldDisconnect(false)
{
}


/*====================
  CHost::UpdateTime
  ====================*/
void    CHost::UpdateTime()
{
    CDate   date(true);
    host_date = date.GetDateString();
    host_time = date.GetTimeString();
}


/*====================
  CHost::DrawVersionStamp
  ====================*/
void    CHost::DrawVersionStamp()
{
    try
    {
        uint uiTime(GetTime() - m_uiVersionStampTime);
        if (m_uiVersionStampTime > GetTime())
            uiTime = 0;
        if (!m_bShowVersionStamp &&
            (m_uiVersionStampTime == INVALID_TIME || uiTime >= VERSION_STAMP_FADE_TIME))
            return;

        // Get the font
        ResHandle hFont(g_ResourceManager.LookUpName(_T("system_medium"), RES_FONTMAP));
        CFontMap *pFontMap(g_ResourceManager.GetFontMap(hFont));
        if (pFontMap == NULL)
            return;

        float fAlpha(1.0f - (uiTime / float(VERSION_STAMP_FADE_TIME)));
        if (m_bShowVersionStamp)
            fAlpha = 1.0f;
        float fYPos(Draw2D.GetScreenH() - ((pFontMap->GetMaxHeight() + VERSION_STAMP_SPACING) * m_vsVersionStampText.size()));

        // Draw the gradiant
        Draw2D.SetColor(0.8f, 0.8f, 0.8f, fAlpha * 0.8f);
        Draw2D.Rect(Draw2D.GetScreenW() / 2.0f, fYPos - 2.0f,
            Draw2D.GetScreenW(), Draw2D.GetScreenH() - 2.0f, g_ResourceManager.LookUpName(_T("alphagrad"), RES_TEXTURE));

        // Draw the strings with "shadows"
        for (tsvector::iterator it(m_vsVersionStampText.begin()); it != m_vsVersionStampText.end(); ++it)
        {
            Draw2D.SetColor(CVec4f(0.0f, 0.0f, 0.0f, fAlpha / 1.2f));
            Draw2D.String(Draw2D.GetScreenW() - pFontMap->GetStringWidth(*it), fYPos + VERSION_STAMP_DROP_SHADOW, *it, hFont);
            Draw2D.SetColor(CVec4f(1.0f, 1.0f, 1.0f, fAlpha));
            Draw2D.String(Draw2D.GetScreenW() - pFontMap->GetStringWidth(*it) - VERSION_STAMP_DROP_SHADOW, fYPos, *it, hFont);
            fYPos += (pFontMap->GetMaxHeight() + VERSION_STAMP_SPACING);
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CHost::DrawVersionStamp() - "), NO_THROW);
    }
}


/*====================
  CHost::DrawInfoStrings
  ====================*/
void    CHost::DrawInfoStrings()
{
    ResHandle hFont(INVALID_RESOURCE);
    CFontMap *pFontMap(NULL);

    if (host_drawFPS || host_drawActiveClient)
    {
        // Get the font
        hFont = g_ResourceManager.LookUpName(_T("system_large"), RES_FONTMAP);
        pFontMap = g_ResourceManager.GetFontMap(hFont);
    }

    // FPS
    if (host_drawFPS > 2 && pFontMap != NULL)
    {
        tstring sString;
        if (host_drawFPS == 4)
        {
            int iMS(m_uiSystemTime - m_uiLastSystemTime);
            sString = XtoA(iMS) + _T(" MS");
        }
        else
        {
            int iFps(INT_ROUND(1.0f / ((m_uiSystemTime - m_uiLastSystemTime) / 1000.0f)));
            sString = XtoA(iFps) + _T(" FPS");
        }
        
        float fWidth(pFontMap->GetStringWidth(sString));
        Draw2D.SetColor(BLACK);
        Draw2D.String(Draw2D.GetScreenW() - fWidth - 4.0f, 4.0f, fWidth, pFontMap->GetMaxHeight(), sString, hFont);
        Draw2D.SetColor(WHITE);
        Draw2D.String(Draw2D.GetScreenW() - fWidth - 6.0f, 2.0f, fWidth, pFontMap->GetMaxHeight(), sString, hFont);
    }
    else if (host_drawFPS && pFontMap != NULL)
    {
        tstring sString;
        if (host_drawFPS == 2)
            sString = XtoA(1000 / MAX(1u, m_uiLastFPS)) + _T(" MS");
        else
            sString = XtoA(m_uiLastFPS) + _T(" FPS");
        
        float fWidth(pFontMap->GetStringWidth(sString));
        Draw2D.SetColor(BLACK);
        Draw2D.String(Draw2D.GetScreenW() - fWidth - 4.0f, 4.0f, fWidth, pFontMap->GetMaxHeight(), sString, hFont);
        Draw2D.SetColor(WHITE);
        Draw2D.String(Draw2D.GetScreenW() - fWidth - 6.0f, 2.0f, fWidth, pFontMap->GetMaxHeight(), sString, hFont);
    }

    m_uiFPSCount++;

    if (m_uiLastFPSTime < m_uiSystemTime - 1000)
    {
        m_uiLastFPS = (m_uiFPSCount / float(m_uiSystemTime - m_uiLastFPSTime)) * 1000;
        m_uiFPSCount = 0;
        m_uiLastFPSTime = m_uiSystemTime;

        FPS.Trigger(XtoA(m_uiLastFPS));
    }

    // Active client
    if (host_drawActiveClient && pFontMap != NULL)
    {
        tstring sString(XtoA(signed(GetActiveClientIndex()), FMT_PADZERO, 2));
        float fWidth(pFontMap->GetStringWidth(sString));
        Draw2D.SetColor(BLACK);
        Draw2D.String(4.0f, 4.0f, fWidth, pFontMap->GetMaxHeight(), sString, hFont);
        Draw2D.SetColor(WHITE);
        Draw2D.String(2.0f, 2.0f, fWidth, pFontMap->GetMaxHeight(), sString, hFont);
    }
}


/*====================
  CHost::ShowVersionStamp
  ====================*/
void    CHost::ShowVersionStamp()
{
    Console << _T("VERSION INFO") << newl
            << _T("=====================================") << newl;
    for (tsvector::iterator it(m_vsVersionStampText.begin()); it != m_vsVersionStampText.end(); ++it)
            Console << *it << newl;
    Console << _T("=====================================") << newl;

    m_bShowVersionStamp = true;
}


/*====================
  CHost::HideVersionStamp
  ====================*/
void    CHost::HideVersionStamp()
{
    m_bShowVersionStamp = false;
    m_uiVersionStampTime = GetTime();
}


/*====================
  CHost::Init
  ====================*/
void    CHost::Init(const tstring &sGame)
{
    PROFILE_EX("CHost::Init", PROFILE_INIT);

    try
    {   
        // at this point, host_startupCfg might be either "startup_d.cfg" or "startup.cfg",
        // depending on whether we're in Debug or Release mode respectively.  We need to check
        // whether that file exists, and if not, default to "startup.cfg".
        if (!FileManager.Exists(host_startupCfg, 0, sGame))
            host_startupCfg.Set(_T("~/startup.cfg"));

#ifdef K2_TRACK_MEM
        physx::shdfnd2::createMemoryTracker("MemoryTracker");
#ifdef WIN32
        assert(physx::shdfnd2::gMemoryTracker != NULL);
#endif
        if (physx::shdfnd2::gMemoryTracker)
            physx::shdfnd2::gMemoryTracker->setLogLevel(false, false);
#endif

        cURL_Initialize();

        // New core object initialization paradigm
        m_pHTTPManager = K2_NEW(ctx_Host,  CHTTPManager);       
        m_pHTTPManager->Initialize();
        
        FileManager.Initialize();

        FileManager.SetModStack(TokenizeString(sGame, _T(';')));

        // Startup info
        UpdateTime();
        host_version = K2_Version(K2System.GetVersionString());
        
        Console.OpenLog();
        
        Console << _T("************************************************************") << newl
                << _T("K2 Engine start up...") << newl
                << _T("[") << host_date << _T("]") << newl
                << _T("[") << host_time << _T("]") << newl
                << _T("[") << K2System.GetVersionString() << _T("]") << newl
                << _T("[") << K2System.GetBuildOSString() + _T(" ") + K2System.GetBuildArchString() << _T("]") << newl
                << K2System.GetBuildInfoString() << newl
                << _T("Build date: ") _T(__DATE__) _T(" ") _T(__TIME__) << newl;

        if (FileManager.GetUsingCustomFiles() == true)
            Console << _T("Modded files are in use.") << newl;
        
        m_vsVersionStampText.push_back(K2System.GetGameName());
        m_vsVersionStampText.push_back(K2_Version(K2System.GetVersionString()));
        m_vsVersionStampText.push_back(K2System.GetBuildOSString() + _T(" ") + K2System.GetBuildArchString());
        m_vsVersionStampText.push_back(K2System.GetBuildInfoString());
        m_vsVersionStampText.push_back(_T("Build date: ") _T(__DATE__));

        XMLManager.PrintVersion();
        
        K2Updater.Initialize();

        PrintInitDebugInfo(_T("Processing Command Line Options..."));
        // Execute configs
        ProcessCommandLine();
        PrintInitDebugInfo(_T("Finished Processing Command Line Options..."));

        if (!m_bNoConfig)
        {
            Console.ExecuteScript(host_startupCfg);
            Console.ExecuteScript(_T("~/login.cfg"));
        }

        if (!host_init.empty())
            Console.Execute(host_init);

        if (FileManager.Exists(_T("/init.cfg")))
            Console.ExecuteScript(_T("/init.cfg"));

        // Stuff that needs settings from startup.cfg loaded
        K2System.InitMore();

        PrintInitDebugInfo(_T("Setting Host Affinity..."));
        K2System.SetAffinity(host_affinity);
        PrintInitDebugInfo(_T("Finished Setting Host Affinity..."));

        if (K2System.IsDedicatedServer() || K2System.IsServerManager())
        {
            K2System.InitDedicatedConsole();

            if (K2System.IsDedicatedServer())
            {
                if (svr_slave != -1)
                    K2System.SetTitle(XtoA(svr_slave) + _T(" - ") + svr_name);
                else
                    K2System.SetTitle(svr_name);
            }
        }
        
        // Initialize components
        
        PrintInitDebugInfo(_T("Initializing UI Manager..."));
        UIManager.Initialize();
        PrintInitDebugInfo(_T("Finished Initializing UI Manager..."));
        
        PrintInitDebugInfo(_T("Initializing Net Driver..."));   
        NetDriver.Initialize();
        PrintInitDebugInfo(_T("Finished Initializing Net Driver..."));

        PrintInitDebugInfo(_T("Initializing Chat Manager...")); 
        ChatManager.Init(m_pHTTPManager);
        PrintInitDebugInfo(_T("Finished Initializing Chat Manager..."));

        PrintInitDebugInfo(_T("Calculating System Ticks..."));  
        M_Init(K2System.GetRandomSeed32());
        PrintInitDebugInfo(_T("Finished Calculating System Ticks..."));

        if (!K2System.IsDedicatedServer() && !K2System.IsServerManager())
        {
            PrintInitDebugInfo(_T("Initializing Input..."));    
            Input.Init();
            PrintInitDebugInfo(_T("Finished Initializing Input..."));
            
            PrintInitDebugInfo(_T("Initializing Sound..."));    
            K2SoundManager.Start();
            PrintInitDebugInfo(_T("Finished Initializing Sound..."));
            
            PrintInitDebugInfo(_T("Initializing Voice Manager..."));    
            VoiceManager.Init();
            PrintInitDebugInfo(_T("Finished Initializing Voice Manager..."));

            // TODO: Review all of these... I think most of them can be handled better
            Bitmap_Init();
            MoviePlayer_Initialize();

            PrintInitDebugInfo(_T("Initializing Vid Driver..."));   
            // Initialize graphics
            Vid.SetDriver(host_vidDriver);
            PrintInitDebugInfo(_T("Finished Initializing Vid Driver..."));  
        }

        PrintInitDebugInfo(_T("Loading Standard Resources..."));    
        // Load NULL model and other standard resources
        g_ResourceManager.RegisterStandardResources();
        PrintInitDebugInfo(_T("Finished Loading Standard Resources..."));

        XMLManager.Process(_T("/base.upgrades"), _T("upgrades"), &m_cUpgrades);

        if (K2System.IsServerManager())
        {
            ServerManager.Initialize(m_pHTTPManager);
        }
        else if (K2System.IsDedicatedServer())
        {
            StartServer();
        }
        else
        {
            PrintInitDebugInfo(_T("Starting Vid Frame..."));        
            // Draw loading logo
            Vid.BeginFrame();
            
            Draw2D.SetColor(BLACK);
            Draw2D.Clear();
            
            ResHandle hLogo(g_ResourceManager.LookUpName(_T("logo"), RES_TEXTURE));
            if (hLogo != INVALID_RESOURCE)
            {
                Draw2D.SetColor(WHITE);
                Draw2D.Rect((Draw2D.GetScreenW() - 512.0f) * 0.5f, (Draw2D.GetScreenH() - 512.0f) * 0.5f, 512.0f, 512.0f, 0.0f, 0.0f, 1.0f, 1.0f, hLogo);
            }

            Vid.EndFrame();
            PrintInitDebugInfo(_T("Ending Vid Frame..."));      

            PrintInitDebugInfo(_T("Setting Cursor..."));        
            Input.SetCursor(CURSOR_BASE, g_ResourceManager.LookUpName(_T("cursor"), RES_K2CURSOR));
            Input.SetCursorConstrained(CURSOR_BASE, BOOL_FALSE);
            Input.SetCursorConstraint(CURSOR_BASE, CRectf(0.0f, 0.0f, 0.0f, 0.0f));
            Input.SetCursorFrozen(CURSOR_BASE, BOOL_FALSE);
            Input.SetCursorHidden(CURSOR_BASE, BOOL_FALSE);
            Input.SetCursorRecenter(CURSOR_BASE, BOOL_FALSE);
            PrintInitDebugInfo(_T("Finished Setting Cursor..."));       

            PrintInitDebugInfo(_T("Starting The Client..."));       
            StartClient();
            PrintInitDebugInfo(_T("Finished Starting The Client..."));      
            
            // We can only grab this info once the client has initialized their Net/Video, so putting it here is 
            // the most straightforward approach. This way we can have a more thorough understanding of the client 
            // setup from just a glance at the console.log.
            const SSysInfo info(K2System.GetSystemInfo());
            Console
                << _T("Client System Details:") << newl
                << _T("OS: ") << info.sOS << newl
                << _T("Processor: ") << info.sProcessor << newl
                << _T("Video: ") << info.sVideo << newl
                << _T("RAM: ") << GetByteString(K2System.GetTotalPhysicalMemory()) << newl;
        }

        PrintInitDebugInfo(_T("Executing Scripts..."));     
        // Autoexec commands, after everything has finished loading
        if (FileManager.Exists(_T("/autoexec.cfg")))
            Console.ExecuteScript(_T("/autoexec.cfg"));
        if (!host_autoexec.empty())
            Console.AddCmdBuffer(host_autoexec);

        if (!host_runOnce.empty())
        {
            Console.AddCmdBuffer(host_runOnce);
            host_runOnce = _T("");
        }
        PrintInitDebugInfo(_T("Finished Executing Scripts..."));

        m_bFirstFrame = true;
        host_language.SetModified(false);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CHost::Init() - "), NO_THROW);
    }
}


/*====================
  CHost::PrintInitDebugInfo
  ====================*/
void    CHost::PrintInitDebugInfo(const tstring &sMessage)
{
    if (host_debugInit)
        Console << _T("[DEBUG] ") << sMessage << newl;
}


/*====================
  CHost::ProcessCommandLine
  ====================*/
void    CHost::ProcessCommandLine()
{
    Console << _T("Command line: ") << K2System.GetCommandLine() << newl;
    const tstring &sCommandLine(K2System.GetCommandLine());
    size_t zPos(sCommandLine.find(_T('-')));
    while (zPos != tstring::npos && zPos < sCommandLine.length())
    {
        ++zPos;
        tstring sCommand(sCommandLine.substr(zPos, sCommandLine.find(_T(' '), zPos) - zPos));
        zPos += sCommand.length();

        if (sCommand == _T("dedicated"))
        {
            K2System.SetDedicatedServer(true);

            EXTERN_CVAR_BOOL(con_showDev);
            EXTERN_CVAR_BOOL(con_showNet);
            EXTERN_CVAR_BOOL(con_showAI);

            con_showDev = false;
            con_showNet = false;
            con_showAI = false;
        }
        else if (sCommand == _T("config"))
        {
            zPos = sCommandLine.find_first_not_of(_T(' '), zPos);
            size_t zEnd(sCommandLine.find(_T(' '), zPos));
            tstring sConfig(sCommandLine.substr(zPos, zEnd - zPos));

            K2System.SetConfig(sConfig);
            host_config = sConfig;

            zPos = zEnd;
        }
        else if (sCommand == _T("mod"))
        {
            zPos = sCommandLine.find_first_not_of(_T(' '), zPos);
            size_t zEnd(sCommandLine.find(_T(' '), zPos));
            tsvector vModStack(TokenizeString(sCommandLine.substr(zPos, zEnd - zPos), _T(';')));
            FileManager.SetModStack(vModStack);
            zPos = zEnd;
        }
        else if (sCommand == _T("execute"))
        {
            zPos = sCommandLine.find_first_not_of(_T(' '), zPos);
            if (zPos != tstring::npos && sCommandLine[zPos] == _T('"'))
            {
                ++zPos;
                size_t zEnd(zPos);
                while (zEnd != tstring::npos && zEnd < sCommandLine.length())
                {
                    zEnd = sCommandLine.find(_T('"'), zEnd);
                    if (zEnd == tstring::npos || sCommandLine[zEnd - 1] != _T('\\'))
                        break;
                    ++zEnd;
                }

                Console.Execute(sCommandLine.substr(zPos, zEnd - zPos));
                zPos = zEnd;
            }
        }
        else if (sCommand == _T("manager"))
        {
            K2System.SetServerManager(true);

            EXTERN_CVAR_BOOL(con_showNet);

            con_showNet = false;
        }
        else if (sCommand == _T("noconfig"))
        {
            m_bNoConfig = true;
        }
        else if (sCommand == _T("register"))
        {
            zPos = sCommandLine.find_first_not_of(_T(' '), zPos);
            size_t zEnd(sCommandLine.find(_T(' '), zPos));
            tstring sAddr(sCommandLine.substr(zPos, zEnd - zPos));

            word wPort(0);

            // Extract port and base ip address from sAddr
            size_t zColon(sAddr.find_first_of(_T(':')));
            if (zColon != tstring::npos)
            {
                uint uiPort(AtoI(sAddr.substr(zColon + 1)));
                if (uiPort > USHRT_MAX)
                    EX_ERROR(_T("Invalid port"));
                wPort = ushort(uiPort);
                sAddr = sAddr.substr(0, zColon);
            }

            m_sRegisterName = sAddr;
            m_wRegisterPort = wPort;
        }
        else if (sCommand == _T("sleep"))
        {
            m_bSleeping = true;
        }
        else if (sCommand == _T("vid"))
        {
            zPos = sCommandLine.find_first_not_of(_T(' '), zPos);
            size_t zEnd(sCommandLine.find(_T(' '), zPos));
            tstring sValue(sCommandLine.substr(zPos, zEnd - zPos));

            host_vidDriver = sValue;
        }
        else if (sCommand == _T("dev"))
        {
            ICvar::CreateBool(_T("cg_dev"), true);
        }
        else if (sCommand == _T("autoexec"))
        {
            zPos = sCommandLine.find_first_not_of(_T(' '), zPos);
            if (zPos != tstring::npos && sCommandLine[zPos] == _T('"'))
            {
                ++zPos;
                size_t zEnd(zPos);
                while (zEnd != tstring::npos && zEnd < sCommandLine.length())
                {
                    zEnd = sCommandLine.find(_T('"'), zEnd);
                    if (zEnd == tstring::npos || sCommandLine[zEnd - 1] != _T('\\'))
                        break;
                    ++zEnd;
                }

                host_autoexec = sCommandLine.substr(zPos, zEnd - zPos);
                zPos = zEnd;
            }
        }
        else if (sCommand == _T("compat"))
        {
            zPos = sCommandLine.find_first_not_of(_T(' '), zPos);
            size_t zEnd(sCommandLine.find(_T(' '), zPos));
            tstring sValue(sCommandLine.substr(zPos, zEnd - zPos));

            FileManager.SetCompatVersion(sValue);
        }
        else if (sCommand == _T("editpath"))
        {
            zPos = sCommandLine.find_first_not_of(_T(' '), zPos);
            size_t zEnd(sCommandLine.find(_T(' '), zPos));
            tstring sValue(sCommandLine.substr(zPos, zEnd - zPos));

            FileManager.SetEditPath(sValue);
        }
#ifdef K2_GARENA
        else if (sCommand.substr(0, 3) == _T("key"))
        {               
            m_sGarenaToken = sCommand.substr(3, sCommandLine.find(_T(' '), zPos));          
        }
#endif
        else
        {
            Console << _T("Unknown command line switch: ") << sCommand << newl;
        }

        zPos = sCommandLine.find(_T('-'), zPos);
    }
}


/*====================
  CHost::Frame
  ====================*/
void    CHost::Frame(uint uiTickLength)
{
    PROFILE_EX("CHost::Frame", PROFILE_GAMELOOP);

    try
    {
        if (m_bShouldDisconnect)
        {
            Disconnect(m_sShouldDisconnectReason);
            m_bShouldDisconnect = false;
            m_sShouldDisconnectReason.clear();
            return;
        }

        MemManager.Frame();

#if 0 // For finding floating-point divide by zero's
        _controlfp(uint(-1) & ~_EM_ZERODIVIDE, _MCW_EM);
#endif

        // New random seed
        srand(K2System.GetRandomSeed32());

        if (host_affinity.IsModified())
        {
            K2System.SetAffinity(host_affinity);
            host_affinity.SetModified(false);
        }

        UpdateTime();

        // Timing
        if (host_timeScale != 1.0f)
            m_uiThisFrameLength = INT_CEIL(uiTickLength * host_timeScale);
        else
            m_uiThisFrameLength = uiTickLength;

        if (m_bReplay)
            m_uiThisFrameLength = INT_CEIL(m_uiThisFrameLength * MAX(host_replaySpeed.GetFloat(), 0.0001f));

        m_uiThisFrameEnd += m_uiThisFrameLength;

        m_fThisFrameEndSeconds = MsToSec(m_uiThisFrameEnd);

        // Reload any resources that changed
        {
            K2System.StartDirectoryMonitoring();

            tsvector vsUpdatedFiles;
            K2System.GetModifiedFileList(vsUpdatedFiles);
            if (!vsUpdatedFiles.empty())
            {
                for (tsvector_it it(vsUpdatedFiles.begin()); it != vsUpdatedFiles.end(); ++it)
                    g_ResourceManager.Reload(*it);
            }
        }

        // Update language
        if (host_language.IsModified())
        {
            host_language.SetModified(false);
            g_ResLibFontFace.SetReplaceResources(true);
            g_ResLibFontMap.SetReplaceResources(true);
            XMLManager.Process(_T("/core_") + host_language + _T(".resources"), _T("resourcelist"));
            g_ResourceManager.ReloadByFlag(RES_FLAG_LOCALIZED);
            g_ResourceManager.GetLib(RES_INTERFACE)->ReloadAll();
            g_ResLibFontFace.SetReplaceResources(false);
            g_ResLibFontMap.SetReplaceResources(false);
        }

        if (m_bResolutionChange)
        {
            m_bResolutionChange = false;
            
            UIManager.ClearOverlayInterfaces();

            Vid.BeginFrame();
            Vid.SetColor(BLACK);
            Vid.Clear();
            UIManager.Frame(Host.GetFrameLength());
            Vid.EndFrame();

            g_ResourceManager.GetLib(RES_FONTMAP)->ReloadAll();
            g_ResourceManager.GetLib(RES_INTERFACE)->ReloadAll();
        }

        if (m_bFirstFrame)
        {
            m_bShowVersionStamp = false;
            m_uiVersionStampTime = GetTime() + 3000;

            m_bFirstFrame = false;
        }

        // Process window events (like keyboard/mouse input)
        K2System.HandleOSMessages();

        if (!K2System.IsDedicatedServer() && !K2System.IsServerManager())
            Input.Frame();

        ProfileManager.Frame();
        SceneStats.Frame();
        NetStats.Frame();
        Console.Frame();

        // Server Frame
        if (m_pServer != NULL)
        {
            bool worldLoaded(false);
            for (map<uint, CHostClient*>::iterator it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
            {
                if(it->second->GetWorld() && it->second->GetWorld()->IsLoaded())
                    worldLoaded = true;
            }

            Console.SetDefaultStream(Console.Server);
            m_pServer->SetGamePointer();
            m_pServer->Frame(m_uiThisFrameLength, K2System.IsDedicatedServer() || worldLoaded || Host.IsReplay());
            Console.SetDefaultStream(Console.Std);
        }

        // Update Chat.
        ChatManager.Frame();

        // Update VoIP
        VoiceManager.Frame();

        // Give the updater time to process
        K2Updater.Frame();

        // Allow cURL time to operate...
        cURL_Frame();
        m_pHTTPManager->Frame();

        if (K2System.IsServerManager())
        {
            ServerManager.Frame();
        }
        else if (!K2System.IsDedicatedServer())
        {
            // Drawing
            Vid.BeginFrame();

            // Client Frame
            if (m_mapClients.empty())
            {
                // Clear the screen
                Draw2D.SetColor(BLACK);
                Draw2D.Clear();
                Console.Enable();
            }
            else
            {
                for (map<uint, CHostClient*>::iterator it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
                {
                    m_pCurrentClient = it->second;
                    it->second->SetGamePointer();
                    it->second->PreFrame();
                    if (it->first == m_uiActiveClient)
                        UIManager.ProcessInput();

                    Console.SetDefaultStream(Console.Client);
                    it->second->Frame();
                    Console.SetDefaultStream(Console.Std);
                }
                map<uint, CHostClient*>::iterator itActive(m_mapClients.find(m_uiActiveClient));
                if (itActive != m_mapClients.end())
                    itActive->second->SetGamePointer();

                m_pCurrentClient = NULL;
            }

            UIManager.Frame(uiTickLength);

            UITextureRegistry.Frame();

            Console.Draw();
            MemManager.Draw();
            ProfileManager.Draw();
            NetStats.Draw();
            SceneStats.Draw();
            DrawVersionStamp();
            DrawInfoStrings();
            
            Vid.EndFrame();

            K2SoundManager.Frame();

            Input.Flush();

            // Process stop requests from clients
            while (!m_stackClientStopRequests.empty())
            {
                StopClient(m_stackClientStopRequests.top());
                m_stackClientStopRequests.pop();
            }
        }

        ++m_uiCurrentFrame;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CHost::Frame() - "));
    }
}


/*====================
  CHost::Execute
  ====================*/
void    CHost::Execute()
{
    try
    {
        uint uiLastTick(K2System.Milliseconds());
        uint uiThisTick(0);
        int iTickLength(0);
        uint uiLastTickTrack(0);
        int iTickLengthTrack(0);

        for (;;) // Infinite game loop
        {
            uint uiFrameMS(SecToMs(1.0f / host_maxFPS));

            // Allow time for other processes
            if (IsSleeping())
                K2System.Sleep(25);
            else if (K2System.IsDedicatedServer() || K2System.IsServerManager() || host_sleep)
            {
                K2System.Sleep(host_sleepMS);
                uiFrameMS = 1;
            }

            // Don't allow frames with a time less than one ms
            do
            {
                if (!K2System.HasFocus() || !IsConnected())
                    K2System.Sleep(host_sleepMS);
                uiThisTick = K2System.Milliseconds();
                iTickLength = uiThisTick - uiLastTick;
                iTickLengthTrack = uiThisTick - uiLastTickTrack;
            }
            while (iTickLength <= 0 || iTickLengthTrack < int(uiFrameMS));
            uiLastTick = uiThisTick;

            uiLastTickTrack = uiThisTick / uiFrameMS * uiFrameMS; // Round down to the nearest uiFrameMS (acts like an accumulator)

            _system_ms = m_uiSystemTime = uiThisTick;
            _host_ms = Host.GetTime();
            Frame(uint(iTickLength));
            m_uiLastSystemTime = m_uiSystemTime;
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CHost::Execute() - "));
    }
}


/*====================
  CHost::Shutdown
  ====================*/
void    CHost::Shutdown()
{
    if (K2System.IsServerManager())
        ServerManager.Shutdown();

    StopClient();
    StopServer(false);

    cURL_Shutdown();
}


/*====================
  CHost::StartServer
  ====================*/
bool    CHost::StartServer(const tstring &sName, const tstring &sGameSettings, bool bPractice, bool bLocal)
{
    try
    {
        if (m_pServer != NULL)
        {
            Console.Warn << _T("Shutting down active server...") << newl;
            StopServer(true);
        }

        m_pServer = K2_NEW(ctx_Host,  CHostServer)(m_pHTTPManager);
        if (!m_pServer->Init(bPractice, bLocal))
            EX_ERROR(_T("Failed to initialize server"));

        if (!sGameSettings.empty())
            m_pServer->StartGame(sName, sGameSettings);

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CHost::StartServer() - "), NO_THROW);

        if (m_pServer != NULL)
            StopServer(true);

        return false;
    }
}


/*====================
  CHost::StartReplay
  ====================*/
bool    CHost::StartReplay(const tstring &sFilename)
{
    try
    {
        if (!FileManager.Exists(sFilename))
        {
            Console << _T("Replay ") << SingleQuoteStr(sFilename) << _T(" not found") << newl;
            return false;
        }

        if (m_pServer != NULL)
        {
            Console.Warn << _T("Shutting down active server...") << newl;
            StopServer(true);
        }

        m_bReplay = true;

        m_pServer = K2_NEW(ctx_Host,  CHostServer)(m_pHTTPManager);
        if (!m_pServer->Init(true, true))
            EX_ERROR(_T("Failed to initialize server"));

        bool bRet(true);

        if (!sFilename.empty())
            bRet = m_pServer->StartReplay(sFilename);

        return bRet;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CHost::StartReplay() - "), NO_THROW);
        return false;
    }
}


/*====================
  CHost::StopReplay
  ====================*/
void    CHost::StopReplay()
{
    if (!m_bReplay)
        return;

    if (m_pServer != NULL)
    {
        m_pServer->StopReplay();
        Console.Warn << _T("Shutting down active server...") << newl;
        StopServer(true);
    }

    m_bReplay = false;

    //StopClient(-2);

    return;
}


/*====================
  CHost::StartClient
  ====================*/
bool    CHost::StartClient(const tstring &sModStack)
{
    PROFILE_EX("CHost::StartClient", PROFILE_STARTCLIENT);

    try
    {
        if (!sModStack.empty())
        {
            // Save current config
            cmdWriteConfigScript(host_startupCfg);

            // Unbind all game tables
            for (EBindTable eTable(BINDTABLE_GAME); eTable != NUM_BINDTABLES; eTable = EBindTable(int(eTable) + 1))
                ActionRegistry.UnbindTable(eTable);

            tsvector vModStack(TokenizeString(sModStack, _T(';')));
            FileManager.SetModStack(vModStack);

            // Execute configs
            Console.ExecuteScript(host_startupCfg);

            if (!host_init.empty())
                Console.Execute(host_init);

            if (FileManager.Exists(_T("/init.cfg")))
                Console.ExecuteScript(_T("/init.cfg"));

            Vid.ChangeMode(-1);
            Vid.Notify(VID_NOTIFY_RELOAD_SHADER_CACHE, 0, 0, 0, NULL);
        }

        CHostClient *pNewClient(K2_NEW(ctx_Host,  CHostClient)(m_uiNextClientID, m_pHTTPManager));
        if (pNewClient == NULL)
            EX_ERROR(_T("Failed to allocate a client"));

        Vid.OpenTextureArchive(true);   // TODO: intelligently reload textures here

        pNewClient->Init();
        if (!m_mapClients.empty())
            pNewClient->RegenerateConnectionID();

        m_mapClients[m_uiNextClientID] = pNewClient;
        m_uiActiveClient = m_uiNextClientID;
        ++m_uiNextClientID;

        Input.Flush();
        Console << _T("Created new client instance #") << m_uiActiveClient << newl;
        Console.Disable();
        return true;
    }
    catch (CException &ex)
    {
        StopClient();
        ex.Process(_T("CHost::StartClient() - "), NO_THROW);
        return false;
    }
}


/*====================
  CHost::NextClient
  ====================*/
void    CHost::NextClient()
{
    if (m_mapClients.empty())
    {
        m_uiActiveClient = -1;
        return;
    }

    map<uint, CHostClient*>::iterator it(m_mapClients.find(m_uiActiveClient));
    if (it == m_mapClients.end())
    {
        m_uiActiveClient = m_mapClients.begin()->first;
    } 
    else 
    {
        ++it;
        if (it == m_mapClients.end())
            it = m_mapClients.begin();

        m_uiActiveClient = it->first;
    }

    Console.Execute(_T("UpdateInterface"));
}


/*====================
  CHost::PrevClient
  ====================*/
void    CHost::PrevClient()
{
    if (m_mapClients.empty())
    {
        m_uiActiveClient = -1;
        return;
    }

    map<uint, CHostClient*>::iterator it(m_mapClients.find(m_uiActiveClient));

    if (it == m_mapClients.begin())
    {
        it = m_mapClients.end();
    }

    --it;
    m_uiActiveClient = it->first;
    Console.Execute(_T("UpdateInterface"));
}


/*====================
  CHost::StopServer
  ====================*/
void    CHost::StopServer(bool bFreeResources)
{
    if (m_pServer != NULL)
    {
        m_pServer->SetGamePointer();
        SAFE_DELETE(m_pServer);

        if (bFreeResources)
            g_ResourceInfo.ExecCommandLine(_T("context delete server:*"));
    }

    m_bReplay = false;

}


/*====================
  CHost::StopClient
  ====================*/
void    CHost::StopClient(uint uiIndex)
{
    if (uiIndex == uint(-2))
    {
        for (map<uint, CHostClient*>::iterator it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
        {
            it->second->SetGamePointer();
            SAFE_DELETE(it->second);
        }
        m_mapClients.clear();
        m_uiActiveClient = -1;
    }

    if (uiIndex == uint(-1))
    {
        map<uint, CHostClient*>::iterator it(m_mapClients.find(m_uiActiveClient));
        if (it != m_mapClients.end())
        {
            it->second->SetGamePointer();
            SAFE_DELETE(it->second);
            m_mapClients.erase(it);
            if (m_mapClients.empty())
                m_uiActiveClient = -1;
            else
                m_uiActiveClient = m_mapClients.begin()->first;
        }
    }
    else
    {
        map<uint, CHostClient*>::iterator it(m_mapClients.find(uiIndex));
        if (it != m_mapClients.end())
        {
            it->second->SetGamePointer();
            SAFE_DELETE(it->second);
            uint uiOldIndex(it->first);
            m_mapClients.erase(it);
            if (m_uiActiveClient == uiOldIndex)
            {
                if (m_mapClients.empty())
                    m_uiActiveClient = -1;
                else
                    m_uiActiveClient = m_mapClients.begin()->first;
            }
        }
    }
}


/*====================
  CHost::ResetLocalClientTimeouts
  ====================*/
void    CHost::ResetLocalClientTimeouts()
{
    for (map<uint, CHostClient*>::iterator it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
        it->second->UpdateServerTimeout();
}


/*====================
  CHost::Connect
  ====================*/
void    CHost::Connect(const tstring &sAddr, bool bSilent, bool bPractice, const tstring &sLoadingInterface)
{
    PROFILE_EX("CHost::Connect", PROFILE_CONNECT);

    try
    {
        if (sAddr.empty())
            return;

        // Clean up an address in the format of "game_name://x.x.x.x:xxx",
        // so that connect will work with an APP
        // see: http://msdn.microsoft.com/workshop/networking/pluggable/overview/appendix_a.asp
        tstring sCleanAddr(sAddr);
        if (sCleanAddr.substr(0, 8) == _T("hon:"))
            sCleanAddr = sCleanAddr.substr(8, sCleanAddr.find_first_not_of('/'));

        m_sServerAddr = sCleanAddr;

        // Connect to the server
        map<uint, CHostClient*>::iterator it(m_mapClients.find(m_uiActiveClient));
        if (it == m_mapClients.end())
        {
            if (!StartClient())
                EX_ERROR(_T("Failed to start client"));
        }

        m_mapClients[m_uiActiveClient]->SetGamePointer();
        m_mapClients[m_uiActiveClient]->Connect(m_sServerAddr, bSilent, bPractice, sLoadingInterface);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CHost::Connect() - "), NO_THROW);
    }
}


/*====================
  CHost::Disconnect
  ====================*/
void    CHost::Disconnect(const tstring &sReason)
{
    bool bClientDisconnected(false);

    map<uint, CHostClient*>::iterator it(m_mapClients.find(m_uiActiveClient));
    if (it != m_mapClients.end())
    {
        if (it->second->GetState() >= CLIENT_STATE_CONNECTING)
            bClientDisconnected = true;

        it->second->Disconnect(sReason);
    }

    StopServer();
}


/*====================
  CHost::Reconnect
  ====================*/
void    CHost::Reconnect()
{
    if (m_sServerAddr.empty())
        return;

    Connect(m_sServerAddr);
}


/*====================
  CHost::LoadAllClientResources
  ====================*/
void    CHost::LoadAllClientResources()
{
    CHostClient *pActiveClient(GetActiveClient());
    if (pActiveClient != NULL)
    {
        pActiveClient->SetGamePointer();
        pActiveClient->LoadAllResources();
    }
    else
    {
        StartClient();
        m_mapClients[m_uiActiveClient]->SetGamePointer();
        m_mapClients[m_uiActiveClient]->LoadAllResources();
        StopClient();
    }
}


/*====================
  CHost::StartGame
  ====================*/
void    CHost::StartGame(const tstring &sType, const tstring &sName, const tstring &sOptions)
{
#ifndef K2_CLIENT
    if (CompareNoCase(sType, _T("local")) == 0)
    {
        FileManager.SetCompatVersion(K2System.GetVersionString());

        IModalDialog::SetNumLoadingJobs(9);

        if (!StartServer(sName, sOptions, false, true))
        {
            Console << _T("Failed to start server") << newl;
            return;
        }

        Connect(_T("localhost"));
    }
    else 
#endif
    if (CompareNoCase(sType, _T("practice")) == 0)
    {
        FileManager.SetCompatVersion(K2System.GetVersionString());

        IModalDialog::SetNumLoadingJobs(9);

        if (!StartServer(sName, sOptions, true))
        {
            Console << _T("Failed to start server") << newl;
            return;
        }

        Connect(_T("localhost"), false, true);
    }
    else if (CompareNoCase(sType, _T("local_automatic")) == 0)
    {
        // Connect to the server
        map<uint, CHostClient*>::iterator it(m_mapClients.find(m_uiActiveClient));
        if (it == m_mapClients.end())
        {
            if (!StartClient())
                EX_ERROR(_T("Failed to start client"));
        }

        m_mapClients[m_uiActiveClient]->SetGamePointer();
        m_mapClients[m_uiActiveClient]->StartAutoLocalGame(sName, sOptions);
    }
    else if (CompareNoCase(sType, _T("remote_automatic")) == 0)
    {
        // Connect to the server
        map<uint, CHostClient*>::iterator it(m_mapClients.find(m_uiActiveClient));
        if (it == m_mapClients.end())
        {
            if (!StartClient())
                EX_ERROR(_T("Failed to start client"));
        }

        m_mapClients[m_uiActiveClient]->SetGamePointer();
        m_mapClients[m_uiActiveClient]->StartAutoRemoteGame(sName, sOptions);
    }
    else
    {
        Console.Warn << _T("Invalid game type: ") << sType << newl;
    }
}


/*====================
  CHost::PreloadWorld
  ====================*/
void    CHost::PreloadWorld(const tstring &sWorldName)
{
    CHostClient *pActiveClient(GetActiveClient());
    if (pActiveClient == NULL)
        return;

    pActiveClient->SetGamePointer();
    pActiveClient->PreloadWorld(sWorldName);
}



/*====================
  CHost::BanClient
  ====================*/
void    CHost::BanClient(int iClientNum, int iTime, const tstring &sReason)
{
    if (m_pServer == NULL)
    {
        Console << _T("No active server") << newl;
        return;
    }

    m_pServer->BanClient(iClientNum, iTime, sReason);
}


/*====================
  CHost::FileDropNotify
  ====================*/
void    CHost::FileDropNotify(const tsvector &vsFiles)
{
    map<uint, CHostClient*>::iterator it(m_mapClients.find(m_uiActiveClient));
    if (it != m_mapClients.end())
        it->second->FileDropNotify(vsFiles);
}


/*====================
  CHost::GetAccountIDs
  ====================*/
iset    CHost::GetAccountIDs()
{
    iset setIDs;

    setIDs.clear();

    if (m_pServer == NULL)
    {
        Console << _T("No active server") << newl;
        return setIDs;
    }
    
    return m_pServer->GetAccountIDs();
}


/*====================
  CHost::GetClientNumFromAccountID
  ====================*/
int CHost::GetClientNumFromAccountID(int iAccountID)
{
    if (m_pServer == NULL)
    {
        Console << _T("No active server") << newl;
        return -1;
    }
    
    return m_pServer->GetClientNumber(iAccountID);
}


/*====================
  CHost::GetNumActiveClients
  ====================*/
int     CHost::GetNumActiveClients()
{
    if (m_pServer == NULL)
    {
        Console << _T("No active server") << newl;
        return -1;
    }

    return m_pServer->GetNumActiveClients();
}


/*====================
  CHost::IsConnected
  ====================*/
bool    CHost::IsConnected()
{
/*  if (HasServer())
        return true;*/

    map<uint, CHostClient*>::iterator findit;

    findit = m_mapClients.find(GetActiveClientIndex());

    if (!HasClient() || findit == m_mapClients.end())
        return false;
    
    return (findit->second->GetState() != CLIENT_STATE_IDLE);
}


/*====================
  CHost::IsInGame
  ====================*/
bool    CHost::IsInGame()
{
    HostClientMap_it itClient(m_mapClients.find(GetActiveClientIndex()));
    if (itClient == m_mapClients.end())
        return false;
    
    return (itClient->second->GetState() == CLIENT_STATE_IN_GAME);
}


/*====================
  CHost::GetConnectedAddress
  ====================*/
tstring CHost::GetConnectedAddress()
{
/*  if (HasServer())
        return _T("127.0.0.1");//(m_pServer->GetServerAddress() + _T(":") + XtoA(m_pServer->GetServerPort()));*/

    map<uint, CHostClient*>::iterator findit;

    findit = m_mapClients.find(GetActiveClientIndex());

    if (!HasClient() || findit == m_mapClients.end())
        return _T("");
    
    return findit->second->GetConnectedAddress();
}


/*====================
  CHost::GetServerWorldName
  ====================*/
const tstring&  CHost::GetServerWorldName()
{
    if (m_pServer && m_pServer->GetWorld())
        return m_pServer->GetWorld()->GetName();
    else
        return TSNULL;
}


/*====================
  CHost::UpdateAvailable
  ====================*/
void    CHost::UpdateAvailable(const tstring &sVersion)
{
    if (K2System.IsServerManager())
        ServerManager.UpdateAvailable(sVersion);

    // Only notify server of an update if it's a dedicated server
    if (HasServer() && K2System.IsDedicatedServer())
        m_pServer->UpdateAvailable(sVersion);

    map<uint, CHostClient*>::iterator itFind(m_mapClients.find(GetActiveClientIndex()));
    if (itFind != m_mapClients.end())
        itFind->second->UpdateAvailable(sVersion);
}


/*====================
  CHost::UpdateComplete
  ====================*/
void    CHost::UpdateComplete()
{
    if (K2System.IsServerManager())
        ServerManager.UpdateComplete();

    // If we're on a dedicated, notify it that we're done
    if (HasServer() && K2System.IsDedicatedServer())
        m_pServer->UpdateComplete();
}


/*====================
  CHost::IsIgnored
  ====================*/
bool    CHost::IsIgnored(const tstring &sUser)
{
    return ChatManager.IsIgnored(sUser);
}


/*====================
  CHost::SaveConfig
  ====================*/
void    CHost::SaveConfig(const tstring &sFileName, const tstring &sFilter) const
{
    CFileHandle hFile(sFileName.empty() ? host_startupCfg : sFileName, FILE_WRITE | FILE_TEXT);
    if (!hFile.IsOpen())
    {
        Console.Err << _T("Failed to open config file") << newl;
        return;
    }

    tsvector vsWildcards(TokenizeString(sFilter, SPACE));

    ICvar::WriteConfigFile(hFile, vsWildcards, CVAR_SAVECONFIG);
    IGameBind::WriteConfigFile(hFile);
    CAction::WriteConfigFile(hFile, vsWildcards, BIND_SAVECONFIG);
    CAlias::WriteConfigFile(hFile, vsWildcards, ALIAS_SAVECONFIG);
    Console.WriteConsoleHistory(hFile);

    Console << _T("Wrote: ") << hFile.GetPath() << newl;
}


/*--------------------
  SetConfig
  --------------------*/
CMD(SetConfig)
{
    if (vArgList.empty())
        return false;

    host_config = vArgList[0];
    K2System.SetConfig(vArgList[0]);

    return true;
}


/*--------------------
  SetLanguage
  --------------------*/
UI_VOID_CMD(SetLanguage, 1)
{
    const tstring &sLanguage(vArgList[0]->Evaluate());
    if (sLanguage == host_language)
        return;

    host_language = sLanguage;
}


/*--------------------
  GetFPS
  --------------------*/
UI_CMD(GetFPS, 0)
{   
    return XtoA(INT_ROUND(1.0f / ((Host.GetSystemTime() - Host.GetLastSystemTime()) / 1000.0f))) + _T(" FPS");
}

