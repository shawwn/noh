// (C)2008 S2 Games
// gl2_win32.cpp
//
// Win32 specific renderer API functions
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "../k2/c_uimanager.h"
#include "../k2/c_cursor.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
WNDPROC     MainWndProc;
HINSTANCE   g_hInstance;
HWND        g_hWnd;
HDC         g_hDC;
HGLRC       g_hglrc;
ICONINFO    g_IconInfo = {0};
HICON       g_hCursorIcon = NULL;

CVAR_BOOL   (win_changeDisplayMode,     false);
//=============================================================================

/*====================
  InitAPIs

  This is what's exported by the DLL and called by the rest of
  the engine. It's a wrapper to do the windows-specific stuff
  and call the shared API initialization function, basically.
  ====================*/
extern "C" __declspec(dllexport)
void    InitAPIs(SVidDriver *vid_api, WNDPROC _MainWndProc, HINSTANCE hInstance)
{
    g_hInstance = hInstance;
    MainWndProc = _MainWndProc;

    InitAPIs_Global(vid_api);
}


/*====================
  GL_GetHWnd
  ====================*/
void*   GL_GetHWnd()
{
    return g_hWnd;
}


/*====================
  GL_Init
  ====================*/
int     GL_Init()
{
    int modenum = 0;
    BOOL bSuccess(FALSE);
    DEVMODE devmode;
    
    gl_initialized = false;

    ZeroMemory(&devmode, sizeof(devmode));
    devmode.dmSize = sizeof(DEVMODE);

    // Fill in the default mode (Desktop)
    if (EnumDisplaySettings(NULL, ENUM_REGISTRY_SETTINGS, &devmode))
    {
        g_VidModes[0].iWidth = devmode.dmPelsWidth;
        g_VidModes[0].iHeight = devmode.dmPelsHeight;
        g_VidModes[0].iBpp = devmode.dmBitsPerPel;
        g_VidModes[0].iRefreshRate = devmode.dmDisplayFrequency;
        g_VidModes[0].sName = _T("Desktop ") + ParenStr(XtoA(g_VidModes[0].iWidth) + _T("x") + XtoA(g_VidModes[0].iHeight) + _T("x") + XtoA(g_VidModes[0].iBpp) + _T("@") + XtoA(g_VidModes[0].iRefreshRate));
    }
    else
        K2System.Error(_T("EnumDisplaySettings failed on mode 0"));

    g_iNumVidModes = 1;  // Always keep mode 0 open

    do
    {
        bool bModeExists(false);
        bSuccess = EnumDisplaySettings(NULL, modenum, &devmode);
        if (!bSuccess)
        {
            if (modenum == 0)
                K2System.Error(_T("EnumDisplaySettings failed on mode 0"));
            break;
        }

        if (devmode.dmBitsPerPel >= 15 && devmode.dmPelsWidth >= 1024 && devmode.dmPelsHeight >= 720 && g_iNumVidModes < MAX_VID_MODES)
        {
            // See if the mode is valid
            devmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
            if (ChangeDisplaySettings(&devmode, CDS_TEST | CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL)
            {
                // Make sure the mode doesn't already exist
                for (int n = 1; n < g_iNumVidModes; ++n)
                {
                    if (g_VidModes[n].iWidth == int(devmode.dmPelsWidth) &&
                        g_VidModes[n].iHeight == int(devmode.dmPelsHeight) &&
                        g_VidModes[n].iBpp == int(devmode.dmBitsPerPel) &&
                        g_VidModes[n].iRefreshRate == int(devmode.dmDisplayFrequency))
                        bModeExists = true;
                }
                if (!bModeExists)
                {
                    // The mode is valid, so add it to g_VidModes
                    g_VidModes[g_iNumVidModes].iWidth = int(devmode.dmPelsWidth);
                    g_VidModes[g_iNumVidModes].iHeight = int(devmode.dmPelsHeight);
                    g_VidModes[g_iNumVidModes].iBpp = int(devmode.dmBitsPerPel);
                    g_VidModes[g_iNumVidModes].iRefreshRate = int(devmode.dmDisplayFrequency);
                    g_VidModes[g_iNumVidModes].sName = XtoA(devmode.dmPelsWidth) + _T("x") + XtoA(devmode.dmPelsHeight) + _T("x") + XtoA(devmode.dmBitsPerPel) + _T(" @ ") + XtoA(devmode.dmDisplayFrequency) + _T(" Hz");

                    Console.Video << _T("Vid mode ") << g_iNumVidModes << _T(": ")
                                << _T("Width: ") << devmode.dmPelsWidth
                                << _T(", Height: ") << devmode.dmPelsHeight
                                << _T(", Bpp: ") << devmode.dmBitsPerPel
                                << _T(", Refresh rate: ") << devmode.dmDisplayFrequency << newl;
                    ++g_iNumVidModes;
                }
            }
        }
        ++modenum;
    }
    while (bSuccess);
    
    return GL_Global_Init();
}


/*====================
  GL_SetupPixelFormat
  ====================*/
void    GL_SetupPixelFormat()
{
    int pixelformat;
    static PIXELFORMATDESCRIPTOR pfd = {
    sizeof(PIXELFORMATDESCRIPTOR),  // size of this pfd
    1,                      // version number
    PFD_DRAW_TO_WINDOW      // support window
    | PFD_SUPPORT_OPENGL    // support OpenGL
    | PFD_DOUBLEBUFFER,     // double buffered
    PFD_TYPE_RGBA,          // RGBA type
    24,                     // 24-bit color depth
    0, 0, 0, 0, 0, 0,       // color bits ignored
    0,                      // no alpha buffer
    0,                      // shift bit ignored
    0,                      // no accumulation buffer
    0, 0, 0, 0,             // accum bits ignored
    24,                     // 24-bit z-buffer  
    0,                      // no stencil buffer
    0,                      // no auxiliary buffer
    PFD_MAIN_PLANE,         // main layer
    0,                      // reserved
    0, 0, 0                 // layer masks ignored
    };
    
    if (!(pixelformat = ChoosePixelFormat(g_hDC, &pfd)))    
       K2System.Error(_T("GL_SetupPixelFormat: ChoosePixelFormat failed"));            

    if (!SetPixelFormat(g_hDC, pixelformat, &pfd))
        K2System.Error(_T("SetPixelFormat failed"));
}


/*====================
  GL_Start
  ====================*/
void    GL_Start()
{
    static bool class_registered = false;

    // Register window class
    if (!class_registered)
    {
        WNDCLASS wc;

        MemManager.Set(&wc, 0, sizeof(WNDCLASS));       
        wc.lpfnWndProc = MainWndProc;
        wc.hInstance = g_hInstance;
        wc.lpszClassName = _T("K2_OpenGL");
        wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1)); 

        if (!RegisterClass(&wc))
            K2System.Error(_T("GL_Start: RegisterClass() failed"));
        class_registered = true;
    }

    // Create the window
    HWND hWnd = CreateWindowEx
    (
        WS_EX_ACCEPTFILES,
        _T("K2_OpenGL"),
        K2System.GetGameName().c_str(),
        WS_MAXIMIZE | WS_POPUP,
        0, 
        0, 
        g_CurrentVidMode.iWidth,
        g_CurrentVidMode.iHeight,
        NULL,
        NULL,
        g_hInstance,
        NULL
    );

    if (!hWnd)
        K2System.Error(_T("GL_Start: CreateWindow() failed"));
    K2System.SetWindowHandle(hWnd);

    g_hWnd = hWnd;
    g_hDC = GetDC(g_hWnd);

    GL_SetMode();

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    PatBlt(g_hDC, 0, 0, g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight, BLACKNESS);

    GL_SetupPixelFormat();

    if (!(g_hglrc = wglCreateContext(g_hDC)))
        K2System.Error(_T("GL_Start: wglCreateContext() failed"));

    if (!wglMakeCurrent(g_hDC, g_hglrc))
        K2System.Error(_T("GL_Start: wglMakeCurrent() failed"));

    gl_initialized = true;

    PatBlt(g_hDC, 0, 0, g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight, BLACKNESS);

    GL_Global_Start();

    // Build anti-aliasing mode list
    g_AAModes[0].iSamples = 0;
    g_AAModes[0].iQuality = 0;
    g_AAModes[0].sName = _T("None");
    g_iNumAAModes = 1; // Always keep mode 0 open

    for (int i(2); i <= 16; ++i)
    {
        int iAttributes[] =
        {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
            WGL_COLOR_BITS_ARB, 24,
            WGL_ALPHA_BITS_ARB, 8,
            WGL_DEPTH_BITS_ARB, 16,
            WGL_STENCIL_BITS_ARB, 0,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
            WGL_SAMPLES_ARB, i,
            0, 0
        };

        float fAttributes[] = {0, 0};
        int iPixelFormat;
        UINT uiNumFormats;

        BOOL bValid(wglChoosePixelFormatARB(g_hDC, iAttributes, fAttributes, 1, &iPixelFormat, &uiNumFormats));
     
        if (!bValid || uiNumFormats == 0)
            continue;

        g_AAModes[g_iNumAAModes].iSamples = i;
        g_AAModes[g_iNumAAModes].iQuality = 0;
        g_AAModes[g_iNumAAModes].sName = XtoA(i) + _T("x");

        ++g_iNumAAModes;
    }

    GL_SetGamma(DEFAULT_OVERBRIGHT);
}


/*====================
  GL_SetMode
  ====================*/
int     GL_SetMode()
{
    int iMode(-1);

    // Try to match a valid mode
    for (int i(0); i < g_iNumVidModes; ++i)
    {
        if (g_VidModes[i].iWidth == vid_resolution[0] &&
            g_VidModes[i].iHeight == vid_resolution[1] &&
            g_VidModes[i].iBpp == vid_bpp &&
            g_VidModes[i].iRefreshRate == vid_refreshRate)
        {
            g_CurrentVidMode = g_VidModes[i];
            iMode = i;
            break;
        }
    }

    if (iMode == -1)
    {
        if (vid_fullscreen)
        {
            g_CurrentVidMode = g_VidModes[0];
            iMode = 0;
        }
        else
        {
            g_CurrentVidMode.iWidth = vid_resolution[0];
            g_CurrentVidMode.iHeight = vid_resolution[1];
            g_CurrentVidMode.iBpp = vid_bpp;
            g_CurrentVidMode.iRefreshRate = vid_refreshRate;
        }
    }
    
    g_iCurrentVideoMode = iMode;

    if (vid_antialiasing.GetSize() != 2)
        vid_antialiasing.Resize(2, 0);

    g_CurrentAAMode.iSamples = vid_antialiasing[0];
    g_CurrentAAMode.iQuality = vid_antialiasing[1];
    g_CurrentAAMode.sName = _T("");

    if (vid_fullscreen)
    {
        if (iMode != 0 || win_changeDisplayMode)
        {
            DEVMODE devmode;
            MemManager.Set(&devmode, 0, sizeof(devmode));
            devmode.dmPelsWidth = g_CurrentVidMode.iWidth;
            devmode.dmPelsHeight = g_CurrentVidMode.iHeight;
            devmode.dmBitsPerPel = g_CurrentVidMode.iBpp;
            devmode.dmDisplayFrequency = g_CurrentVidMode.iRefreshRate;
            devmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;

            devmode.dmSize = sizeof(devmode);

            bool bSuccess(false);

            bSuccess = ChangeDisplaySettings(&devmode, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL;

            if (!bSuccess && iMode != 0)
            {
                if (win_changeDisplayMode)
                {
                    DEVMODE devmode0;
                    MemManager.Set(&devmode0, 0, sizeof(devmode0));
                    devmode0.dmPelsWidth = g_VidModes[0].iWidth;
                    devmode0.dmPelsHeight = g_VidModes[0].iHeight;
                    devmode0.dmBitsPerPel = g_VidModes[0].iBpp;
                    devmode0.dmDisplayFrequency = g_VidModes[0].iRefreshRate;
                    devmode0.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

                    bSuccess = ChangeDisplaySettings(&devmode0, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL;

                    devmode = devmode0;
                }
                else
                {
                    bSuccess = ChangeDisplaySettings(NULL, 0) == DISP_CHANGE_SUCCESSFUL;
                }

                g_iCurrentVideoMode = 0;
            }

            if (!bSuccess)
            {
                K2System.Error(_T("GL_SetMode: Unable to set Mode 0"));
            }
        }
        else
        {
            if (ChangeDisplaySettings(NULL, 0) != DISP_CHANGE_SUCCESSFUL)
                K2System.Error(_T("GL_SetMode: Unable to set desktop video mode"));
        }

        g_bFullscreen = true;
    }
    else
    {
        g_bFullscreen = false;

        if (ChangeDisplaySettings(NULL, 0) != DISP_CHANGE_SUCCESSFUL)
            K2System.Error(_T("GL_SetMode: Unable to set desktop video mode"));
    }

    if (g_hWnd)
    {
        if (!vid_fullscreen)
        {
            DWORD dwWindowStyle(WS_CAPTION | WS_MINIMIZEBOX | WS_VISIBLE | WS_SYSMENU);

            if (vid_windowResize)
                dwWindowStyle |= WS_SIZEBOX;

            if (!SetWindowLong(g_hWnd, GWL_STYLE, dwWindowStyle))
                K2System.Error(_TS("SetWindowLong() - ") + K2System.GetLastErrorString());

            RECT    winsize;
            winsize.left = 0;
            winsize.right = g_CurrentVidMode.iWidth;
            winsize.top = 0;
            winsize.bottom = g_CurrentVidMode.iHeight;
            AdjustWindowRect(&winsize, dwWindowStyle, false);
            if (!SetWindowPos(g_hWnd, HWND_TOP, 0, 0, winsize.right - winsize.left, winsize.bottom - winsize.top, SWP_SHOWWINDOW))
                K2System.Error(_TS("SetWindowPos() - ") + K2System.GetLastErrorString());
            GetClientRect(g_hWnd, &winsize);
    
            // Resize render surface to match new window size (if it was clipped)
            g_CurrentVidMode.iWidth = MIN<int>(g_CurrentVidMode.iWidth, winsize.right - winsize.left);
            g_CurrentVidMode.iHeight = MIN<int>(g_CurrentVidMode.iHeight, winsize.bottom - winsize.top);
        }
        else
        {
            if (!SetWindowLong(g_hWnd, GWL_STYLE, WS_MAXIMIZE))
                K2System.Error(_TS("SetWindowLong() - ") + K2System.GetLastErrorString());

            if (!SetWindowPos(g_hWnd, HWND_TOP, 0, 0, g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight, SWP_SHOWWINDOW))
                K2System.Error(_TS("SetWindowPos() - ") + K2System.GetLastErrorString());
        }

        ShowWindow(g_hWnd, SW_SHOW);
        UpdateWindow(g_hWnd);

        PatBlt(g_hDC, 0, 0, g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight, BLACKNESS);
    }

    return g_iCurrentVideoMode;
}


/*====================
  GL_SetGamma
  ====================*/
void    GL_SetGamma(float gamma)
{
}


/*====================
  GL_ShowCursor
  ====================*/
void    GL_ShowCursor(bool bShow)
{
    if (bShow)
        while(ShowCursor(true) < 0);
    else
        while(::ShowCursor(false) >= 0);
}


/*====================
  GL_SetCursor
  ====================*/
void    GL_SetCursor(ResHandle hCursor)
{
    if (hCursor == g_hCursor)
        return;
    g_hCursor = hCursor;

    if (g_hCursor == INVALID_RESOURCE)
    {
        ShowCursor(false);
        return;
    }

    CCursor *pCursor(g_ResourceManager.GetCursor(hCursor));
    if (pCursor == NULL)
    {
        ShowCursor(false);
        return;
    }

    CBitmap *pBitmap(pCursor->GetBitmapPointer());
    if (pBitmap == NULL)
    {
        ShowCursor(false);
        return;
    }

    const int WIDTH(32), HEIGHT(32), BYTEPP(4);
    
    if (pBitmap->GetWidth() != WIDTH || pBitmap->GetHeight() != HEIGHT || pBitmap->GetBMPType() != BYTEPP)
        return;

    byte yBuffer[WIDTH * HEIGHT * 4];

    const byte *pSrcData(static_cast<const byte *>(pBitmap->GetBuffer()));
    byte *pDstData(yBuffer);
    int iDeltaPitch(0);

    // Invert
    pSrcData += pBitmap->GetWidth() * 4 * (pBitmap->GetHeight() - 1);
    iDeltaPitch -= pBitmap->GetWidth() * 4 * 2;

    for (int y(0); y < pBitmap->GetHeight(); ++y)
    {
        for (int x(0); x < pBitmap->GetWidth(); ++x)
        {
            pDstData[0] = pSrcData[2];
            pDstData[1] = pSrcData[1];
            pDstData[2] = pSrcData[0];
            pDstData[3] = pSrcData[3];

            pDstData += 4;
            pSrcData += 4;
        }

        pSrcData += iDeltaPitch;
    }

    CVec2i v2Hotspot(pCursor->GetHotspot());

    if (g_hCursorIcon != NULL) {
        DeleteObject(g_IconInfo.hbmMask);
        DeleteObject(g_IconInfo.hbmColor);
        DestroyIcon(g_hCursorIcon);
    }

    g_IconInfo.fIcon = false;
    g_IconInfo.xHotspot = v2Hotspot.x;
    g_IconInfo.yHotspot = v2Hotspot.y;
    g_IconInfo.hbmMask = CreateBitmap(WIDTH, HEIGHT, 1, 1, NULL);
    g_IconInfo.hbmColor = CreateBitmap(WIDTH, HEIGHT, 1, BYTEPP * 8, yBuffer);

    g_hCursorIcon = CreateIconIndirect(&g_IconInfo);

    SetClassLongPtr(g_hWnd, GCL_HCURSOR, LONG_PTR(g_hCursorIcon));
}


/*====================
  GL_Shutdown
  ====================*/
void    GL_Shutdown()
{
    if (!gl_initialized)
        return;

    GL_Global_Shutdown();

    wglMakeCurrent(NULL, NULL);
    
    GL_SetGamma(1);

    if (g_hglrc)
    {
        wglDeleteContext(g_hglrc);
        g_hglrc = 0;
    }

    if (g_hDC)
    {
        ReleaseDC(g_hWnd, g_hDC);
        g_hDC = 0;
    }

    if (g_hCursorIcon != NULL)
        DestroyIcon(g_hCursorIcon);

    DestroyWindow(g_hWnd);
    UnregisterClass(_T("K2_OpenGL"), g_hInstance);

    ChangeDisplaySettings(NULL, 0);

    ShowCursor(TRUE);
    DestroyIcon(g_hCursorIcon);
    DeleteObject(g_IconInfo.hbmMask);
    DeleteObject(g_IconInfo.hbmColor);

    gl_initialized = false;
    g_bValidScene = false;
}


/*====================
  GL_EndFrame
  ====================*/
void    GL_EndFrame()
{
    GL_Global_EndFrame();
    SwapBuffers(g_hDC);
    
    PRINT_GLERROR_BREAK();
}


/*====================
  GL_Break
  ====================*/
void    GL_Break()
{
    __asm int 0x03;
}
