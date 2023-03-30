// (C)2005 S2 Games
// c_vid.h
//
//=============================================================================
#ifndef __C_VID_H__
#define __C_VID_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"
#include "k2_singleton.h"

#include "../public/vid_driver_t.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const int   MAX_VID_MODES(256);
const int   MAX_AA_MODES(32);
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class IModel;
struct SShader;
class CTexture;
class CVertexShader;
class CPixelShader;

const tstring g_aTextureFilteringNames[NUM_TEXTUREFILTERING_MODES] = 
{
    _T("None"),
    _T("Bilinear"),
    _T("Trilinear"),
    _T("Anisotropic 2x"),
    _T("Anisotropic 4x"),
    _T("Anisotropic 6x"),
    _T("Anisotropic 8x"),
    _T("Anisotropic 12x"),
    _T("Anisotropic 16x"),
    _T("Anisotropic 32x")
};

EXTERN_CVAR_STRING(vid_currentMode);
EXTERN_CVAR_INT(vid_mode);

#ifdef linux
K2_API EXTERN_CVAR_STRING(vid_display);
K2_API EXTERN_CVAR_BOOL(vid_blankOtherDisplays);
#endif
#ifdef __APPLE__
K2_API EXTERN_CVAR_INT(vid_display);
#endif
K2_API EXTERN_CVAR_STRING(vid_aspect);
K2_API EXTERN_ARRAY_CVAR_UINT(vid_resolution);
K2_API EXTERN_CVAR_INT(vid_bpp);
K2_API EXTERN_CVAR_INT(vid_refreshRate);
K2_API EXTERN_CVAR_FLOAT(vid_gamma);
K2_API EXTERN_ARRAY_CVAR_UINT(vid_antialiasing);
K2_API EXTERN_CVAR_INT(vid_textureFiltering);
K2_API EXTERN_CVAR_BOOL(vid_fullscreen);
K2_API EXTERN_CVAR_BOOL(vid_windowResize);
//=============================================================================

//=============================================================================
// SVidMode
//=============================================================================
struct SVidMode
{
    tstring sName;
#ifdef linux
    tstring sDisplay;
#endif
#ifdef __APPLE__
    int     iDisplay;
#endif
    int     iWidth;
    int     iHeight;
    int     iBpp;
    int     iRefreshRate;
};
//=============================================================================

//=============================================================================
// SAAMode
//=============================================================================
struct SAAMode
{
    tstring sName;
    int     iSamples;
    int     iQuality;
};
//=============================================================================

//=============================================================================
// CVid
//=============================================================================
class CVid
{
SINGLETON_DEF(CVid)

private:
    SVidDriver  m_Driver;
    SVidMode    m_VidMode;
    SAAMode     m_AAMode;
    int         m_iCurrentMode;
    bool        m_bInitialized;
    void*       m_hVidDLL;

    void    StartDriver();

public:
    ~CVid();

    K2_API void     SetDriver(const tstring &sDrivername);
    K2_API void     PrintWarnings();
    K2_API void     SetMode();

    K2_API void     Shutdown(); // shuts down the current video driver

    K2_API void     InitScene();

    // All drawing goes through this function, so we call our
    // specific driver drawing functions through here, as opposed to
    // putting them in vid_driver_t or having a function header for them
    // (we don't want any external interface to them; they should only be
    // called by the RenderScene function)
    K2_API void     RenderScene(class CCamera &camera);

    K2_API void     Add2dRect(float x, float y, float w, float h, float s1, float t1, float s2, float t2, ResHandle hTexture, int iFlags);
    K2_API void     Add2dQuad(const CVec2f& v1, const CVec2f& v2, const CVec2f& v3, const CVec2f& v4,
                                    const CVec2f& t1, const CVec2f& t2, const CVec2f& t3, const CVec2f& t4, ResHandle hTexture, int iFlags);
    K2_API void     Add2dLine(const CVec2f& v1, const CVec2f& v2, const CVec4f &v4Color1, const CVec4f &v4Color2, int iFlags);
    K2_API void     AddPoint(const CVec3f &v3Point, const CVec4f &v4Color);
    K2_API void     AddLine(const CVec3f &v3Start, const CVec3f &v3End, const CVec4f &v4Color);
    K2_API void     Clear();

    K2_API void     NormalizeColor(const vec4_t color, vec4_t out);
    K2_API void     SetColor(CVec4f v4Color);

    // Sometimes system functions have to notify the video
    // system that they have been called, such as loading
    // of a new map, modification of terrain data, etc
    K2_API void     Notify(EVidNotifyMessage eMsg, int iParam1, int iParam2, int iParam3, void *pData, const tstring &sResContext = _T("global"));

    K2_API void     BeginFrame();   // called before the client frame to notify the system we may be drawing
    K2_API void     EndFrame();     // called at the end of the client frame to swap buffers (and any other cleanup)

    K2_API int      GetScreenW();
    K2_API int      GetScreenH();
    K2_API float    GetAspect()     { return GetScreenW() / float(GetScreenH()); }

    K2_API void     GetClientOffset(int *x, int *y);

    K2_API void     GetFrameBuffer(CBitmap &bmp);
    K2_API CVec2f   ProjectVertex(const class CCamera &cam, const CVec3f &vecVertex);
    K2_API bool     IsFullScreen();
    K2_API void*    GetHWnd();
    K2_API void     ChangeMode(int iMode);
    K2_API bool     TextureFilteringModeAvailable(ETextureFilteringModes eMode);

    K2_API void     OpenTextureArchive(bool bNoReload = false);
    K2_API void     CloseTextureArchive();
    K2_API void     GetTextureList(const tstring &sPath, const tstring &sSearch, tsvector &vResult);

    K2_API int      RegisterTexture(CTexture *pTexture);
    K2_API void     UnregisterTexture(CTexture *pTexture);

    K2_API int      RegisterVertexShader(CVertexShader *pVertexShader);
    K2_API void     UnregisterVertexShader(CVertexShader *pVertexShader);

    K2_API int      RegisterPixelShader(CPixelShader *pPixelShader);
    K2_API void     UnregisterPixelShader(CPixelShader *pPixelShader);

    K2_API void     RegisterShaderPair(CVertexShader *pVertexShader, CPixelShader *pPixelShader);

    K2_API int      RegisterModel(CModel *pModel);
    K2_API void     UnregisterModel(CModel *pModel);

    K2_API void     LoadVidModule(const tstring &sFilename);
    K2_API void     UnloadVidModule();

    K2_API const CCamera*   GetCamera();

    K2_API void     RenderFogofWar(float fClear, bool bTexture, float fLerp);
    K2_API void     UpdateFogofWar(const CBitmap &cBmp);

    K2_API void     ShowCursor(bool bShow);
    K2_API void     SetCursor(ResHandle hCursor);

    K2_API bool     GetMode(int iMode, SVidMode *pVidMode);
    K2_API bool     GetCurrentMode(SVidMode *pVidMode);

    K2_API bool     GetAAMode(int iMode, SAAMode *pAAMode);
    K2_API bool     GetCurrentAAMode(SAAMode *pAAMode);
    
    K2_API CVec4f   GetTextureColor(CTexture *pTexture);
    K2_API bool     TextureExists(const tstring &sName, uint uiTextureFlags);
};

extern K2_API CVid *pVid;
#define Vid (*pVid)
//=============================================================================
#endif
