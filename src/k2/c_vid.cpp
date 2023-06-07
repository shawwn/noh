// (C)2005 S2 Games
// c_vid.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_vid.h"
#include "c_cmd.h"
#include "i_resourcelibrary.h"
#include "c_scenestats.h"
#include "c_uicmd.h"
#include "i_widget.h"
#include "i_listwidget.h"
#include "c_uimanager.h"
#include "c_resourcemanager.h"

#undef pVid
#undef Vid
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CVid    *pVid = CVid::GetInstance();

EXTERN_CVAR_STRING(host_vidDriver);
EXTERN_CVAR_STRING(host_startupCfg);

CVAR_STRING     (vid_currentMode,               "");
CVAR_INTF       (vid_mode,                      -1,         CVAR_READONLY);
#ifdef linux
CVAR_STRINGF    (vid_display,                   "",         CVAR_SAVECONFIG);
CVAR_BOOLF      (vid_blankOtherDisplays,        false,      CVAR_SAVECONFIG);
#endif
#ifdef __APPLE__
CVAR_INTF       (vid_display,                   -1,         CVAR_SAVECONFIG);
#endif
ARRAY_CVAR_UINTF(vid_resolution,                _T("0,0"),  CVAR_SAVECONFIG);
CVAR_INTF       (vid_bpp,                       0,          CVAR_SAVECONFIG);
CVAR_INTF       (vid_refreshRate,               0,          CVAR_SAVECONFIG);
CVAR_STRINGF    (vid_aspect,                    "",         CVAR_SAVECONFIG);
CVAR_FLOATF     (vid_gamma,                     1.1f,       CVAR_SAVECONFIG);
ARRAY_CVAR_UINTF(vid_antialiasing,              _T("0,0"),  CVAR_SAVECONFIG);
CVAR_INTR       (vid_textureFiltering,          TEXTUREFILTERING_TRILINEAR, CVAR_SAVECONFIG, 0, NUM_TEXTUREFILTERING_MODES - 1);
CVAR_BOOLF      (vid_fullscreen,                false,      CVAR_SAVECONFIG);
CVAR_BOOL       (vid_windowResize,              false);

typedef void (*_initapis_t)(SVidDriver *vid_api, void *_MainWndProc, void *hInstance);

SINGLETON_INIT(CVid)
//=============================================================================

/*====================
  CVid::CVid

  Set the singleton up for a SetDriver call
  ====================*/
CVid::CVid() :
m_iCurrentMode(-1),
m_bInitialized(false),
m_hVidDLL(nullptr)
{
#ifdef _WIN32
    MemManager.Set(&m_Driver, 0, sizeof(m_Driver));
#else
    MemManager.Set((char*)(&m_Driver) + sizeof(tstring), 0, sizeof(m_Driver) - sizeof(tstring));
#endif
}


/*====================
  CVid::~CVid
  ====================*/
CVid::~CVid()
{
    UnloadVidModule();
}


/*====================
  CVid::SetDriver
  ====================*/
void    CVid::SetDriver(const tstring &sDriverName)
{
    bool reload = false;
    if (!m_Driver.sDriverName.empty()) // Shutdown an old driver if we had one
    {
        if (m_Driver.sDriverName == sDriverName)
            return;

        Shutdown(); // Shutdown old driver
        UnloadVidModule();
        reload = true;

        // UTTAR: Not ideal but best solution I can find for cvars to stick properly.
        // All other alternatives either required redoing some systems or had disadvantages (or crashes...)
        Console.ExecuteScript(host_startupCfg);
    }

    LoadVidModule(sDriverName);

    StartDriver();

    // UTTAR: Reload all the resources the video driver is directly aware of...
    if (reload)
    {
        g_ResourceManager.GetLib(RES_PIXEL_SHADER)->ReloadAll();
        g_ResourceManager.GetLib(RES_VERTEX_SHADER)->ReloadAll();
        g_ResourceManager.GetLib(RES_TEXTURE)->ReloadAll();
        g_ResourceManager.GetLib(RES_MATERIAL)->ReloadAll();
        g_ResourceManager.GetLib(RES_MODEL)->ReloadAll();
    }
    host_vidDriver.SetModified(0);
}


/*====================
  CVid::RegisterModel
  ====================*/
int     CVid::RegisterModel(CModel *pModel)
{
    if (m_bInitialized)
        return m_Driver.RegisterModel(pModel);
    else
        return -1;
}


/*====================
  CVid::UnregisterModel
  ====================*/
void    CVid::UnregisterModel(CModel *pModel)
{
    if (m_bInitialized)
        return m_Driver.UnregisterModel(pModel);
}


/*====================
  CVid::GetCamera
  ====================*/
const CCamera*  CVid::GetCamera()
{
    if (m_bInitialized)
        return m_Driver.GetCamera();
    else
        return nullptr;
}


/*====================
  CVid::StartDriver
  ====================*/
void    CVid::StartDriver()
{
    Console.Video << _T("---------------------------------------------------------") << newl
            << _T("CVid::StartDriver():") << newl;

    m_Driver.Init();

    {
        SVidMode vm;
        for (int i(0); m_Driver.GetMode(i, &vm); ++i)
        {
            // Create two cvars, one for each direction
            ICvar::CreateString(_T("SetVideoMode") + XtoA(i, FMT_PADZERO, 2), vm.sName);
            ICvar::CreateInt(vm.sName, i);
        }
    }

    for (int i(0); i < NUM_TEXTUREFILTERING_MODES; ++i)
    {
        if (!TextureFilteringModeAvailable(ETextureFilteringModes(i)))
            continue;

        ICvar::CreateString(_T("vid_textureFiltering") + XtoA(i, FMT_PADZERO, 1), g_aTextureFilteringNames[i]);
    }

    if (!vid_resolution[0] || !vid_resolution[1] || !vid_bpp
#ifndef __APPLE__
        // some devices report a refresh rate of 0 under OS X
        || !vid_refreshRate
#endif
            )
    {
        SVidMode vm;
        if (m_Driver.GetMode(0, &vm))
        {
            if (vid_resolution.GetSize() != 2)
                vid_resolution.Resize(2, 0);

            if (vid_fullscreen)
            {
                vid_resolution.SetValue(0, vm.iWidth);
                vid_resolution.SetValue(1, vm.iHeight);
            }
            else
            {
                // Subtract a bit of resolution to account for title bar
                vid_resolution.SetValue(0, MAX(720, vm.iWidth - 160));
                vid_resolution.SetValue(1, MAX(480, vm.iHeight - 200));
            }
            vid_bpp = vm.iBpp;
            vid_refreshRate = vm.iRefreshRate;
        }
    }

    vid_mode = m_iCurrentMode = m_Driver.SetMode();
    m_Driver.GetCurrentMode(&m_VidMode);

    m_bInitialized = true;
    m_Driver.Start();

    m_Driver.GetCurrentMode(&m_VidMode);
    Console.Video << _T("Using mode ") << m_iCurrentMode << _T(": ") << m_VidMode.iWidth << _T("x") << m_VidMode.iHeight << _T("x") << m_VidMode.iBpp << newl;
    vid_currentMode = XtoA(m_VidMode.iWidth) + _T("x") + XtoA(m_VidMode.iHeight) + _T("x") + XtoA(m_VidMode.iBpp);
}


/*====================
  CVid::Shutdown
  ====================*/
void    CVid::Shutdown()
{
    if (m_bInitialized)
        m_Driver.Shutdown();
    m_bInitialized = false;
}


/*====================
  CVid::RenderScene
  ====================*/
void    CVid::RenderScene(class CCamera &camera)
{
    if (m_bInitialized)
    {
        PROFILE("CVid::RenderScene");
        m_Driver.RenderScene(camera);
    }
}


/*====================
  CVid::Add2dRect
  ====================*/
void    CVid::Add2dRect(float x, float y, float w, float h, float s1, float t1, float s2, float t2, ResHandle hTexture, int iFlags)
{
    if (m_bInitialized)
        m_Driver.Add2dRect(x, y, w, h, s1, t1, s2, t2, hTexture, iFlags);
}


/*====================
  CVid::Add2dQuad
  ====================*/
void    CVid::Add2dQuad(const CVec2f& v1, const CVec2f& v2, const CVec2f& v3, const CVec2f& v4,
                        const CVec2f& t1, const CVec2f& t2, const CVec2f& t3, const CVec2f& t4, ResHandle hTexture, int iFlags)
{
    if (m_bInitialized)
        m_Driver.Add2dQuad(v1, v2, v3, v4, t1, t2, t3, t4, hTexture, iFlags);
}


/*====================
  CVid:Add2dLine
  ====================*/
void    CVid::Add2dLine(const CVec2f& v1, const CVec2f& v2, const CVec4f& v4Color1, const CVec4f& v4Color2, int iFlags)
{
    if (m_bInitialized)
        m_Driver.Add2dLine(v1, v2, v4Color1, v4Color2, iFlags);
}


/*====================
  CVid::AddPoint
  ====================*/
void    CVid::AddPoint(const CVec3f &v3Point, const CVec4f &v4Color)
{
    if (m_bInitialized)
        m_Driver.AddPoint(v3Point, v4Color);
}


/*====================
  CVid::AddLine
  ====================*/
void    CVid::AddLine(const CVec3f &v3Start, const CVec3f &v3End, const CVec4f &v4Color)
{
    if (m_bInitialized)
        m_Driver.AddLine(v3Start, v3End, v4Color);
}


/*====================
  CVid::Clear
  ====================*/
void    CVid::Clear()
{
    if (m_bInitialized)
        m_Driver.Clear();
}


/*====================
  CVid::NormalizeColor
  ====================*/
void    CVid::NormalizeColor(const vec4_t color, vec4_t out)
{
    //just clamp values brighter than 1
    if (color[0] > 1)
        out[0] = 1;
    else
        out[0] = color[0];

    if (color[1] > 1)
        out[1] = 1;
    else
        out[1] = color[1];

    if (color[2] > 1)
        out[2] = 1;
    else
        out[2] = color[2];

    out[3] = color[3];
}


/*====================
  CVid::SetColor
  ====================*/
void    CVid::SetColor(CVec4f v4Color)
{
    if (m_bInitialized)
        m_Driver.SetColor(v4Color);
}


/*====================
  CVid::Notify
  ====================*/
void    CVid::Notify(EVidNotifyMessage eMsg, int iParam1, int iParam2, int iParam3, void *pData, const tstring &sResContext)
{
    if (m_bInitialized)
        m_Driver.Notify(eMsg, iParam1, iParam2, iParam3, pData, sResContext);
}


/*====================
  CVid::BeginFrame
  ====================*/
void    CVid::BeginFrame()
{
    if (m_bInitialized)
    {
        PROFILE("CVid::BeginFrame");
        m_Driver.BeginFrame();
    }
}


/*====================
  CVid::EndFrame
  ====================*/
void    CVid::EndFrame()
{
    if (m_bInitialized)
    {
        PROFILE("CVid::EndFrame");
        m_Driver.EndFrame();
    }

    SceneStats.ResetFrame();
}


/*====================
  CVid::GetScreenW
  ====================*/
int     CVid::GetScreenW()
{
    return m_VidMode.iWidth;
}


/*====================
  CVid::GetScreenH
  ====================*/
int     CVid::GetScreenH()
{
    return m_VidMode.iHeight;
}


/*====================
  CVid::GetFrameBuffer
  ====================*/
void    CVid::GetFrameBuffer(CBitmap &bmp)
{
    if (m_bInitialized)
        return m_Driver.GetFrameBuffer(bmp);
}


/*====================
  CVid::ProjectVertex
  ====================*/
CVec2f  CVid::ProjectVertex(const class CCamera &cam, const CVec3f &vecVertex)
{
    if (m_bInitialized)
        return m_Driver.ProjectVertex(cam, vecVertex);
    else
        return CVec2f(0,0);
}


/*====================
  CVid::IsFullScreen
  ====================*/
bool    CVid::IsFullScreen()
{
    if (m_bInitialized)
        return m_Driver.IsFullScreen();
    else
        return false;
}


/*====================
  CVid::GetHWnd
  ====================*/
void*   CVid::GetHWnd()
{
    if (m_bInitialized)
        return m_Driver.GetHWnd();
    else
        return nullptr;
}


/*====================
  CVid::OpenTextureArchive
  ====================*/
void    CVid::OpenTextureArchive(bool bNoReload)
{
    if (m_bInitialized)
        m_Driver.OpenTextureArchive(bNoReload);
}


/*====================
  CVid::CloseTextureArchive
  ====================*/
void    CVid::CloseTextureArchive()
{
    if (m_bInitialized)
        m_Driver.CloseTextureArchive();
}


/*====================
  CVid::GetTextureList
  ====================*/
void    CVid::GetTextureList(const tstring &sPath, const tstring &sSearch, tsvector &vResult)
{
    if (m_bInitialized)
        m_Driver.GetTextureList(sPath, sSearch, vResult);
}


/*====================
  CVid::RegisterTexture
  ====================*/
int     CVid::RegisterTexture(CTexture *pTexture)
{
    if (m_bInitialized)
        return m_Driver.RegisterTexture(pTexture);
    else
        return -1;
}


/*====================
  CVid::UnregisterTexture
  ====================*/
void    CVid::UnregisterTexture(CTexture *pTexture)
{
    if (m_bInitialized)
        m_Driver.UnregisterTexture(pTexture);
}


/*====================
  CVid::RegisterVertexShader
  ====================*/
int     CVid::RegisterVertexShader(CVertexShader *pVertexShader)
{
    if (m_bInitialized)
        return m_Driver.RegisterVertexShader(pVertexShader);
    else
        return -1;
}


/*====================
  CVid::UnregisterVertexShader
  ====================*/
void    CVid::UnregisterVertexShader(CVertexShader *pVertexShader)
{
    if (m_bInitialized)
        m_Driver.UnregisterVertexShader(pVertexShader);
}


/*====================
  CVid::RegisterPixelShader
  ====================*/
int     CVid::RegisterPixelShader(CPixelShader *pPixelShader)
{
    if (m_bInitialized)
        return m_Driver.RegisterPixelShader(pPixelShader);
    else
        return -1;
}


/*====================
  CVid::UnregisterPixelShader
  ====================*/
void    CVid::UnregisterPixelShader(CPixelShader *pPixelShader)
{
    if (m_bInitialized)
        m_Driver.UnregisterPixelShader(pPixelShader);
}


/*====================
  CVid::RegisterShaderPair
  ====================*/
void    CVid::RegisterShaderPair(CVertexShader *pVertexShader, CPixelShader *pPixelShader)
{
    if (m_bInitialized)
        m_Driver.RegisterShaderPair(pVertexShader, pPixelShader);
}


/*====================
  CVid::RenderFogofWar
  ====================*/
void    CVid::RenderFogofWar(float fClear, bool bTexture, float fLerp)
{
    if (m_bInitialized)
    {
        PROFILE("CVid::RenderFogofWar");
        m_Driver.RenderFogofWar(fClear, bTexture, fLerp);
    }
}


/*====================
  CVid::UpdateFogofWar
  ====================*/
void    CVid::UpdateFogofWar(const CBitmap &cBmp)
{
    if (m_bInitialized)
    {
        PROFILE("CVid::UpdateFogofWar");
        m_Driver.UpdateFogofWar(cBmp);
    }
}


/*====================
  CVid::ShowCursor
  ====================*/
void    CVid::ShowCursor(bool bShow)
{
    if (m_bInitialized)
    {
        m_Driver.ShowCursor(bShow);
    }
}


/*====================
  CVid::SetCursor
  ====================*/
void    CVid::SetCursor(ResHandle hCursor)
{
    if (m_bInitialized)
    {
        m_Driver.SetCursor(hCursor);
    }
}


/*====================
  CVid::GetMode
  ====================*/
bool    CVid::GetMode(int iMode, SVidMode *pVidMode)
{
    if (m_bInitialized)
    {
        return m_Driver.GetMode(iMode, pVidMode);
    }

    return false;
}


/*====================
  CVid::GetCurrentMode
  ====================*/
bool    CVid::GetCurrentMode(SVidMode *pVidMode)
{
    if (m_bInitialized)
    {
        return m_Driver.GetCurrentMode(pVidMode);
    }

    return false;
}


/*====================
  CVid::GetTextureColor
  ====================*/
CVec4f  CVid::GetTextureColor(CTexture *pTexture)
{
    if (m_bInitialized)
    {
        return m_Driver.GetTextureColor(pTexture);
    }

    return WHITE;
}


/*====================
  CVid::GetAAMode
  ====================*/
bool    CVid::GetAAMode(int iMode, SAAMode *pAAMode)
{
    if (!m_bInitialized)
        return false;

    return m_Driver.GetAAMode(iMode, pAAMode);
}


/*====================
  CVid::GetCurrentAAMode
  ====================*/
bool    CVid::GetCurrentAAMode(SAAMode *pAAMode)
{
    if (!m_bInitialized)
        return false;

    return m_Driver.GetCurrentAAMode(pAAMode);
}


/*====================
  CVid::TextureExists
  ====================*/
bool    CVid::TextureExists(const tstring &sFilename, uint uiTextureFlags)
{
    if (m_bInitialized)
    {
#if TKTK
        return m_Driver.TextureExists(sFilename, uiTextureFlags);
#else
        if (m_Driver.TextureExists(sFilename, uiTextureFlags))
            return true;
        // try each image fallback extension.
        for (auto sFallbackExt : {_T("png"), _T("dds"), _T("tga")})
        {
            if (CompareNoCase(Filename_GetExtension(sFilename), sFallbackExt) == 0)
                continue;
            tstring sNewFilename(Filename_StripExtension(sFilename) + _T(".") + sFallbackExt);
            if (m_Driver.TextureExists(sNewFilename, uiTextureFlags))
                return true;
        }
        return false;
#endif
    }
    else
        return false;
}


/*====================
  CVid::ChangeMode
  ====================*/
void    CVid::ChangeMode(int iMode)
{
    if (!m_bInitialized)
        return;

    int iWidth0(m_VidMode.iWidth);
    int iHeight0(m_VidMode.iHeight);

    if (iMode != -1)
    {
        SVidMode vm;
        if (m_Driver.GetMode(iMode, &vm))
        {
            if (vid_resolution.GetSize() != 2)
                vid_resolution.Resize(2, 0);

            vid_resolution.SetValue(0, vm.iWidth);
            vid_resolution.SetValue(1, vm.iHeight);
            vid_bpp = vm.iBpp;
            vid_refreshRate = vm.iRefreshRate;
        }
        else
            return;
    }

    vid_mode = m_iCurrentMode = m_Driver.SetMode();
    m_Driver.GetCurrentMode(&m_VidMode);
    m_Driver.GetCurrentAAMode(&m_AAMode);
    Console.Video << _T("Video Mode: width=") << m_VidMode.iWidth << _T(", height=") << m_VidMode.iHeight << _T(", bpp=") << m_VidMode.iBpp << _T(", refreshrate=") << m_VidMode.iRefreshRate << _T(", samples=") << m_AAMode.iSamples << newl;
    vid_currentMode = XtoA(m_VidMode.iWidth) + _T("x") + XtoA(m_VidMode.iHeight) + _T("x") + XtoA(m_VidMode.iBpp);

    if (iWidth0 != m_VidMode.iWidth || iHeight0 != m_VidMode.iHeight)
    {
        Host.SetResolutionChange(true);

        UIManager.ResizeInterface(_T("loading"), m_VidMode.iWidth, m_VidMode.iHeight);

        SetColor(BLACK);
        Clear();
    }
}


/*====================
  CVid::TextureFilteringModeAvailable
  ====================*/
bool    CVid::TextureFilteringModeAvailable(ETextureFilteringModes eMode)
{
    if (!m_bInitialized)
        return false;

    return m_Driver.TextureFilteringModeAvailable(eMode);
}


/*====================
  CVid::LoadVidModule
  ====================*/
void    CVid::LoadVidModule(const tstring &sFileName)
{
    _initapis_t InitAPIs;

    m_hVidDLL = K2System.LoadLibrary(sFileName);
    if (m_hVidDLL == nullptr)
        K2System.Error(_T("Couldn't load ") + sFileName);

    //find and call CL_InitAPIs to get function pointers to the client game functions
    InitAPIs = (_initapis_t)K2System.GetProcAddress(m_hVidDLL, _T("InitAPIs"));
    if (!InitAPIs)
        K2System.Error(_T("Couldn't find entry function InitAPIs()"));

    InitAPIs(&m_Driver, K2System.GetMainWndProc(), K2System.GetInstanceHandle());
}


/*====================
  CVid::UnloadVidModule
  ====================*/
void    CVid::UnloadVidModule()
{
    if (m_hVidDLL)
    {
        K2System.FreeLibrary(m_hVidDLL);
        m_hVidDLL = nullptr;

#ifdef _WIN32
        MemManager.Set(&m_Driver, 0, sizeof(m_Driver));
#else
        MemManager.Set((char*)(&m_Driver) + sizeof(tstring), 0, sizeof(m_Driver) - sizeof(tstring));
        m_Driver.sDriverName.clear();
#endif
    }
}


/*--------------------
  cmdSetVideoMode
  --------------------*/
CMD(SetVideoMode)
{
    CVid::GetInstance()->ChangeMode(vArgList.size() > 0 ? AtoI(vArgList[0]) : -1);
    return true;
}

#if defined(linux) || defined(__APPLE__)
/*--------------------
  AddDisplays
  --------------------*/
UI_VOID_CMD(AddDisplays, 1)
{
    if (!pThis || !pThis->HasFlags(WFLAG_LIST))
        return;

    IListWidget *pList(static_cast<IListWidget *>(pThis));
    if (pList == nullptr)
        return;

    CXMLNode::PropertyMap mapParams;
    for (ScriptTokenVector_cit cit(vArgList.begin() + 1); cit != vArgList.end(); ++cit)
    {
        CXMLNode::Key sKey((*cit)->Evaluate());
        ++cit;
        if (cit == vArgList.end())
            break;
        mapParams[sKey] = (*cit)->Evaluate();
    }
    
    mapParams[_T("label")] = _T("Automatic");
#ifdef linux
    pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), _T(""), mapParams);

    set<tstring> setNames;

    SVidMode vm;
    for (int i(1); CVid::GetInstance()->GetMode(i, &vm); ++i)
    {
        if (vm.sDisplay.empty() || setNames.find(vm.sDisplay) != setNames.end())
            continue;

        setNames.insert(vm.sDisplay);

        mapParams[_T("label")] = vm.sDisplay;

        pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), vm.sDisplay, mapParams);
    }
#else
    pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), _T("-1"), mapParams);
    
    set<int> setDpys;
    
    SVidMode vm;
    for (int i(1); CVid::GetInstance()->GetMode(i, &vm); ++i)
    {
        if (setDpys.find(vm.iDisplay) != setDpys.end())
            continue;
        
        setDpys.insert(vm.iDisplay);
        
        mapParams[_T("label")] = XtoA(vm.iDisplay);
        
        pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), XtoA(vm.iDisplay), mapParams);
    }
#endif
}
#endif


/*--------------------
  AddVideoModes
  --------------------*/
UI_VOID_CMD(AddVideoModes, 1)
{
    if (!pThis || !pThis->HasFlags(WFLAG_LIST))
        return;

    IListWidget *pList(static_cast<IListWidget *>(pThis));
    if (pList == nullptr)
        return;

    CXMLNode::PropertyMap mapParams;
    for (ScriptTokenVector_cit cit(vArgList.begin() + 1); cit != vArgList.end(); ++cit)
    {
        CXMLNode::Key sKey((*cit)->Evaluate());
        ++cit;
        if (cit == vArgList.end())
            break;
        mapParams[sKey] = (*cit)->Evaluate();
    }

    tsvector vVidModeList;

    SVidMode vm;
    for (int i(0); CVid::GetInstance()->GetMode(i, &vm); ++i)
    {
        vVidModeList.push_back(vm.sName);
    }

    for (int i(0); i < int(vVidModeList.size()); ++i)
    {
        mapParams[_T("label")] = vVidModeList[i];

        pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), XtoA(i), mapParams);
    }
}


/*--------------------
  AddTextureFilteringModes
  --------------------*/
UI_VOID_CMD(AddTextureFilteringModes, 1)
{
    if (!pThis || !pThis->HasFlags(WFLAG_LIST))
        return;

    IListWidget *pList(static_cast<IListWidget *>(pThis));
    if (pList == nullptr)
        return;

    CXMLNode::PropertyMap mapParams;
    for (ScriptTokenVector_cit cit(vArgList.begin() + 1); cit != vArgList.end(); ++cit)
    {
        CXMLNode::Key sKey((*cit)->Evaluate());
        ++cit;
        if (cit == vArgList.end())
            break;
        mapParams[sKey] = (*cit)->Evaluate();
    }

    for (int i(0); i < NUM_TEXTUREFILTERING_MODES; ++i)
    {
        if (!CVid::GetInstance()->TextureFilteringModeAvailable(ETextureFilteringModes(i)))
            continue;
        
        mapParams[_T("label")] = g_aTextureFilteringNames[i];

        pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), XtoA(i), mapParams);
    }
}


/*--------------------
  AddAntiAliasingModes
  --------------------*/
UI_VOID_CMD(AddAntiAliasingModes, 1)
{
    if (!pThis || !pThis->HasFlags(WFLAG_LIST))
        return;

    IListWidget *pList(static_cast<IListWidget *>(pThis));
    if (pList == nullptr)
        return;

    CXMLNode::PropertyMap mapParams;
    for (ScriptTokenVector_cit cit(vArgList.begin() + 1); cit != vArgList.end(); ++cit)
    {
        CXMLNode::Key sKey((*cit)->Evaluate());
        ++cit;
        if (cit == vArgList.end())
            break;
        mapParams[sKey] = (*cit)->Evaluate();
    }

    SAAMode cMode;
    for (int i(0); CVid::GetInstance()->GetAAMode(i, &cMode); ++i)
    {
        mapParams[_T("label")] = cMode.sName;

        pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), XtoA(cMode.iSamples) + _T(",") + XtoA(cMode.iQuality), mapParams);
    }
}


/*--------------------
  AddAspectModes
  --------------------*/
UI_VOID_CMD(AddAspectModes, 1)
{
    if (!pThis || !pThis->HasFlags(WFLAG_LIST))
        return;

    IListWidget *pList(static_cast<IListWidget *>(pThis));
    if (pList == nullptr)
        return;

    CXMLNode::PropertyMap mapParams;
    for (ScriptTokenVector_cit cit(vArgList.begin() + 1); cit != vArgList.end(); ++cit)
    {
        CXMLNode::Key sKey((*cit)->Evaluate());
        ++cit;
        if (cit == vArgList.end())
            break;
        mapParams[sKey] = (*cit)->Evaluate();
    }

    mapParams[_T("label")] = _T("Automatic");
    pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), _T(""), mapParams);

    mapParams[_T("label")] = _T("4:3");
    pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), _T("4:3"), mapParams);

    mapParams[_T("label")] = _T("16:10");
    pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), _T("16:10"), mapParams);

    mapParams[_T("label")] = _T("16:9");
    pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), _T("16:9"), mapParams);

    mapParams[_T("label")] = _T("5:4");
    pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), _T("5:4"), mapParams);
}


#ifdef linux
EXTERN_CVAR_STRING(options_display);
#endif
#ifdef __APPLE__
EXTERN_CVAR_INT(options_display);
#endif
/*--------------------
  AddResolutions
  --------------------*/
UI_VOID_CMD(AddResolutions, 1)
{
    if (!pThis || !pThis->HasFlags(WFLAG_LIST))
        return;

    IListWidget *pList(static_cast<IListWidget *>(pThis));
    if (pList == nullptr)
        return;

    CXMLNode::PropertyMap mapParams;
    for (ScriptTokenVector_cit cit(vArgList.begin() + 1); cit != vArgList.end(); ++cit)
    {
        CXMLNode::Key sKey((*cit)->Evaluate());
        ++cit;
        if (cit == vArgList.end())
            break;
        mapParams[sKey] = (*cit)->Evaluate();
    }

    set<tstring> setNames;

    SVidMode vm;
    for (int i(1); CVid::GetInstance()->GetMode(i, &vm); ++i)
    {
#ifdef linux
        if (!options_display.GetValue().empty() && !vm.sDisplay.empty() && options_display.GetValue() != vm.sDisplay)
            continue;
#endif
#ifdef __APPLE__
        if (options_display > -1 && options_display != vm.iDisplay)
            continue;
#endif
        
        tstring sName(XtoA(vm.iWidth) + _T("x") + XtoA(vm.iHeight));

        if (setNames.find(sName) != setNames.end())
            continue;

        setNames.insert(sName);

        mapParams[_T("label")] = sName;

        pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), XtoA(vm.iWidth) + _T(",") + XtoA(vm.iHeight), mapParams);
    }
}


/*--------------------
  AddColorDepths
  --------------------*/
UI_VOID_CMD(AddColorDepths, 1)
{
    if (!pThis || !pThis->HasFlags(WFLAG_LIST))
        return;

    IListWidget *pList(static_cast<IListWidget *>(pThis));
    if (pList == nullptr)
        return;

    CXMLNode::PropertyMap mapParams;
    for (ScriptTokenVector_cit cit(vArgList.begin() + 1); cit != vArgList.end(); ++cit)
    {
        CXMLNode::Key sKey((*cit)->Evaluate());
        ++cit;
        if (cit == vArgList.end())
            break;
        mapParams[sKey] = (*cit)->Evaluate();
    }

    set<tstring> setNames;

    SVidMode vm;
    for (int i(1); CVid::GetInstance()->GetMode(i, &vm); ++i)
    {
        tstring sName(XtoA(vm.iBpp) + _T(" bit"));

        if (setNames.find(sName) != setNames.end())
            continue;

        setNames.insert(sName);

        mapParams[_T("label")] = sName;

        pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), XtoA(vm.iBpp), mapParams);
    }
}


EXTERN_ARRAY_CVAR_UINT(options_resolution);
/*--------------------
  AddRefreshRates
  --------------------*/
UI_VOID_CMD(AddRefreshRates, 1)
{
    if (!pThis || !pThis->HasFlags(WFLAG_LIST))
        return;

    IListWidget *pList(static_cast<IListWidget *>(pThis));
    if (pList == nullptr)
        return;

    CXMLNode::PropertyMap mapParams;
    for (ScriptTokenVector_cit cit(vArgList.begin() + 1); cit != vArgList.end(); ++cit)
    {
        CXMLNode::Key sKey((*cit)->Evaluate());
        ++cit;
        if (cit == vArgList.end())
            break;
        mapParams[sKey] = (*cit)->Evaluate();
    }

    set<tstring> setNames;

    SVidMode vm;
    for (int i(1); CVid::GetInstance()->GetMode(i, &vm); ++i)
    {
#ifdef linux
        if (!options_display.GetValue().empty() && options_display.GetValue() != vm.sDisplay)
            continue;
        
        if (vm.iWidth != options_resolution[0] || vm.iHeight != options_resolution[1])
            continue;
#endif
#ifdef __APPLE__
        if (options_display > -1 && options_display != vm.iDisplay)
            continue;
        
        if (vm.iWidth != options_resolution[0] || vm.iHeight != options_resolution[1])
            continue;
#endif
        
        tstring sName(XtoA(vm.iRefreshRate) + _T(" Hertz"));
        if (vm.iRefreshRate == 0)
            sName = _T("Automatic");

        if (setNames.find(sName) != setNames.end())
            continue;

        setNames.insert(sName);

        mapParams[_T("label")] = sName;

        pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), XtoA(vm.iRefreshRate), mapParams);
    }
}
