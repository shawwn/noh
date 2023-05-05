// (C)2008 S2 Games
// gl2_main.cpp
//
// Implements the K2 Renderer API
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_gfxinit.h"
#include "c_gfxshaders.h"
#include "c_gfx2d.h"
#include "c_gfxmodels.h"
#include "c_gfxterrain.h"
#include "c_gfxutils.h"
#include "c_gfxtextures.h"
#include "gl2_foliage.h"
#include "c_fogofwar.h"
#include "c_shadowmap.h"
#include "c_treescenemanager.h"
#include "c_scenebuffer.h"
#include "c_postbuffer.h"
#include "../k2/c_resourceinfo.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
const CCamera           *g_pCam;
CVec3f                  g_vCamOrigin;

bool                    g_bFullscreen = false;
bool                    gl_initialized = false;
bool                    g_bValidScene = false;
int                     g_iScreenWidth;
int                     g_iScreenHeight;
byte                    g_dwDrawColor[4];

int                     g_iCurrentVideoMode(-1);
SVidMode                g_CurrentVidMode;
SVidMode                g_VidModes[MAX_VID_MODES];
int                     g_iNumVidModes;

SAAMode                 g_CurrentAAMode;
SAAMode                 g_AAModes[MAX_AA_MODES];
int                     g_iNumAAModes;

// Texture settings
GLint                   g_iMaxTextureImageUnits;
GLint                   g_textureMagFilter(GL_LINEAR);
GLint                   g_textureMinFilter(GL_LINEAR);
GLint                   g_textureMinFilterMipmap(GL_LINEAR_MIPMAP_LINEAR);
GLfloat                 g_textureMaxAnisotropy(1.0f);

const float             DEFAULT_OVERBRIGHT = 1.0f;

ResHandle               g_hCursor(INVALID_RESOURCE);

SDeviceCaps             g_DeviceCaps;
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
CVAR_INT    (terrain_chunkSize,         64);
CVAR_INT    (foliage_chunkSize,         16);

CVAR_INTF   (gl_swapInterval,           0,      CVAR_SAVECONFIG);
//=============================================================================


/*====================
  GL_Init
  ====================*/
int     GL_Global_Init()
{
    GfxInit->Init();
    return 1;
}


/*====================
  GL_GetDeviceCaps

  Determine features supported by this video card in this video mode
  ====================*/
void    GL_GetDeviceCaps()
{
#ifdef __APPLE__
    g_DeviceCaps.bNonSquareMatrix = false;
#else
    g_DeviceCaps.bNonSquareMatrix = (GLEW_VERSION_2_1 == GL_TRUE);
#endif
    
    g_DeviceCaps.bTextureCompression = (GLEW_EXT_texture_compression_s3tc == GL_TRUE);

    glGetIntegerv(GL_MAX_VARYING_FLOATS_ARB, &g_DeviceCaps.iMaxVaryingFloats);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB, &g_iMaxTextureImageUnits);

    if (g_iMaxTextureImageUnits > 32)
        g_iMaxTextureImageUnits = 32;
}


/*====================
  GL_Global_Start
  ====================*/
void    GL_Global_Start()
{
    GfxInit->Start();
    GL_SetupTextureFilterSettings(); 
#ifdef _WIN32
    if (WGLEW_EXT_swap_control)
        wglSwapIntervalEXT(gl_swapInterval);
#endif
}


/*====================
  GL_Create
  ====================*/
void    GL_Create()
{
}


/*====================
  GL_GetMode
  ====================*/
bool    GL_GetMode(int iMode, SVidMode *pVidMode)
{
    if (!pVidMode || iMode < 0 || iMode >= g_iNumVidModes)
        return false;

#ifdef linux
    pVidMode->sDisplay = g_VidModes[iMode].sDisplay;
#endif
#ifdef __APPLE__
    pVidMode->iDisplay = g_VidModes[iMode].iDisplay;
#endif
    pVidMode->iWidth = g_VidModes[iMode].iWidth;
    pVidMode->iHeight = g_VidModes[iMode].iHeight;
    pVidMode->iBpp = g_VidModes[iMode].iBpp;
    pVidMode->iRefreshRate = g_VidModes[iMode].iRefreshRate;
    pVidMode->sName = g_VidModes[iMode].sName;
    return true;
}


/*====================
  GL_GetCurrentMode
  ====================*/
bool    GL_GetCurrentMode(SVidMode *pVidMode)
{
    pVidMode->iWidth = g_CurrentVidMode.iWidth;
    pVidMode->iHeight = g_CurrentVidMode.iHeight;
    pVidMode->iBpp = g_CurrentVidMode.iBpp;
    pVidMode->iRefreshRate = g_CurrentVidMode.iRefreshRate;
    pVidMode->sName = g_CurrentVidMode.sName;
    return true;
}


/*====================
  GL_IsFullScreen
  ====================*/
bool    GL_IsFullScreen()
{
    return g_bFullscreen;
}


/*====================
  GL_GetAAMode
  ====================*/
bool    GL_GetAAMode(int iMode, SAAMode *pAAMode)
{
    if (!pAAMode || iMode < 0 || iMode >= g_iNumAAModes)
        return false;

    pAAMode->iSamples = g_AAModes[iMode].iSamples;
    pAAMode->iQuality = g_AAModes[iMode].iQuality;
    pAAMode->sName = g_AAModes[iMode].sName;
    return true;
}


/*====================
  GL_GetCurrentAAMode
  ====================*/
bool    GL_GetCurrentAAMode(SAAMode *pAAMode)
{
    pAAMode->iSamples = g_CurrentAAMode.iSamples;
    pAAMode->iQuality = g_CurrentAAMode.iQuality;
    pAAMode->sName = g_CurrentAAMode.sName;
    return true;
}


/*====================
  GL_Global_Shutdown
  ====================*/
void    GL_Global_Shutdown()
{
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgramObjectARB(0);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    Gfx2D->Shutdown();
    Gfx3D->Shutdown();
    GfxModels->Shutdown();
    GfxTextures->Shutdown();
    GfxShaders->Shutdown();
    GfxTerrain->Shutdown();

    GL_DestroyFoliage();

    g_pTreeSceneManager->Destroy();

    g_Shadowmap.Release();
    g_FogofWar.Release();
    g_SceneBuffer.Release();
    g_PostBuffer.Release();

    PRINT_GLERROR_BREAK();
}


/*====================
  GL_TextureFilteringModeAvailable
  ====================*/
bool    GL_TextureFilteringModeAvailable(ETextureFilteringModes eMode)
{
    float fMaxAnisotropy(0);
    
    if (GLEW_EXT_texture_filter_anisotropic)
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fMaxAnisotropy);
    
    switch (eMode)
    {
    case TEXTUREFILTERING_ANISOTROPIC16: return fMaxAnisotropy >= 16.0;
    case TEXTUREFILTERING_ANISOTROPIC12: return fMaxAnisotropy >= 12.0;
    case TEXTUREFILTERING_ANISOTROPIC8: return fMaxAnisotropy >= 8.0;
    case TEXTUREFILTERING_ANISOTROPIC6: return fMaxAnisotropy >= 6.0;
    case TEXTUREFILTERING_ANISOTROPIC4: return fMaxAnisotropy >= 4.0;
    case TEXTUREFILTERING_ANISOTROPIC2: return fMaxAnisotropy >= 2.0;
    default: return true;
    }
    
    return false;
}


/*====================
  GL_SetupTextureFilterSettings
  ====================*/
void    GL_SetupTextureFilterSettings()
{
    // texture filtering settings
    float fMaxAnisotropy(1.0f);
        
    if (GLEW_EXT_texture_filter_anisotropic)
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fMaxAnisotropy);
        
    switch (ETextureFilteringModes(vid_textureFiltering.GetValue()))
    {
    case TEXTUREFILTERING_NONE:
        g_textureMagFilter = GL_NEAREST;
        g_textureMinFilter = GL_NEAREST;
        g_textureMinFilterMipmap = GL_NEAREST_MIPMAP_NEAREST;
        g_textureMaxAnisotropy = 1.0f;
        break;
    case TEXTUREFILTERING_BILINEAR:
        g_textureMagFilter = GL_LINEAR;
        g_textureMinFilter = GL_LINEAR;
        g_textureMinFilterMipmap = GL_LINEAR_MIPMAP_NEAREST;
        g_textureMaxAnisotropy = 1.0f;
        break;
    case TEXTUREFILTERING_TRILINEAR:
        g_textureMagFilter = GL_LINEAR;
        g_textureMinFilter = GL_LINEAR;
        g_textureMinFilterMipmap = GL_LINEAR_MIPMAP_LINEAR;
        g_textureMaxAnisotropy = 1.0f;
        break;
    case TEXTUREFILTERING_ANISOTROPIC2:
        g_textureMagFilter = GL_LINEAR;
        g_textureMinFilter = GL_LINEAR;
        g_textureMinFilterMipmap = GL_LINEAR_MIPMAP_LINEAR;
        g_textureMaxAnisotropy = MIN(2.0f, fMaxAnisotropy);
        break;
    case TEXTUREFILTERING_ANISOTROPIC4:
        g_textureMagFilter = GL_LINEAR;
        g_textureMinFilter = GL_LINEAR;
        g_textureMinFilterMipmap = GL_LINEAR_MIPMAP_LINEAR;
        g_textureMaxAnisotropy = MIN(4.0f, fMaxAnisotropy);
        break;
    case TEXTUREFILTERING_ANISOTROPIC6:
        g_textureMagFilter = GL_LINEAR;
        g_textureMinFilter = GL_LINEAR;
        g_textureMinFilterMipmap = GL_LINEAR_MIPMAP_LINEAR;
        g_textureMaxAnisotropy = MIN(6.0f, fMaxAnisotropy);
        break;
    case TEXTUREFILTERING_ANISOTROPIC8:
        g_textureMagFilter = GL_LINEAR;
        g_textureMinFilter = GL_LINEAR;
        g_textureMinFilterMipmap = GL_LINEAR_MIPMAP_LINEAR;
        g_textureMaxAnisotropy = MIN(8.0f, fMaxAnisotropy);
        break;
    case TEXTUREFILTERING_ANISOTROPIC12:
        g_textureMagFilter = GL_LINEAR;
        g_textureMinFilter = GL_LINEAR;
        g_textureMinFilterMipmap = GL_LINEAR_MIPMAP_LINEAR;
        g_textureMaxAnisotropy = MIN(12.0f, fMaxAnisotropy);
        break;
    case TEXTUREFILTERING_ANISOTROPIC16:
        g_textureMagFilter = GL_LINEAR;
        g_textureMinFilter = GL_LINEAR;
        g_textureMinFilterMipmap = GL_LINEAR_MIPMAP_LINEAR;
        g_textureMaxAnisotropy = MIN(16.0f, fMaxAnisotropy);
        break;
    case TEXTUREFILTERING_ANISOTROPIC32:
        g_textureMagFilter = GL_LINEAR;
        g_textureMinFilter = GL_LINEAR;
        g_textureMinFilterMipmap = GL_LINEAR_MIPMAP_LINEAR;
        g_textureMaxAnisotropy = MIN(32.0f, fMaxAnisotropy);
        break;
    case NUM_TEXTUREFILTERING_MODES:
        K2_UNREACHABLE();
        break;
    }
}


/*====================
  GL_BeginFrame
  ====================*/
void    GL_BeginFrame()
{
    GL_SetGamma(vid_gamma);
    
    g_bValidScene = true;

    GfxShaders->Frame();
}


/*====================
  GL_Global_EndFrame
  ====================*/
void    GL_Global_EndFrame()
{
    Gfx2D->Draw();
    g_bValidScene = false;

    GfxModels->lCustomMappings.clear();
}


/*====================
  GL_RenderScene
  ====================*/
void    GL_RenderScene(CCamera &camera)
{
    Gfx2D->Draw();

    glDepthMask(GL_TRUE);

    if (!(camera.GetFlags() & CAM_NO_DEPTH_CLEAR))
        glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    Gfx3D->Draw(camera);
}


/*====================
  GL_Notify
  ====================*/
void    GL_Notify(EVidNotifyMessage eMsg, int iParam1, int iParam2, int iParam3, void *pData, const tstring &sResContext)
{
    K2_RESOURCE_CONTEXT(sResContext);

    switch (eMsg)
    {
    case VID_NOTIFY_UPDATE_SHADERS:
    case VID_NOTIFY_RELOAD_SHADER_CACHE:
    case VID_NOTIFY_ADD_CLIFF:
    case VID_NOTIFY_REMOVE_CLIFF:
    case VID_NOTIFY_REBUILD_CLIFFS:
    case VID_NOTIFY_FOG_OF_WAR:
        break;
    case VID_NOTIFY_NEW_WORLD:
        GfxTerrain->Rebuild(terrain_chunkSize, static_cast<CWorld*>(pData));
        GL_RebuildFoliage(foliage_chunkSize, static_cast<CWorld*>(pData));
        break;

    case VID_NOTIFY_WORLD_DESTROYED:
        GfxTerrain->Destroy();
        GL_DestroyFoliage();
        break;

    case VID_NOTIFY_TERRAIN_COLOR_MODIFIED:
        if (iParam3)
            GfxTerrain->InvalidateTerrainLayer(TERRAIN_REBUILD_COLORS | TERRAIN_REBUILD_TEXCOORDS);
        else
            GfxTerrain->InvalidateTerrainVertex(iParam1, iParam2, TERRAIN_REBUILD_COLORS);
        break;

    case VID_NOTIFY_TERRAIN_NORMAL_MODIFIED:
        if (iParam3)
            GfxTerrain->InvalidateTerrainLayer(TERRAIN_REBUILD_NORMALS | TERRAIN_REBUILD_TEXCOORDS);
        else
            GfxTerrain->InvalidateTerrainVertex(iParam1, iParam2, TERRAIN_REBUILD_NORMALS);
        break;

    case VID_NOTIFY_TERRAIN_VERTEX_MODIFIED:
        if (iParam3)
            GfxTerrain->InvalidateTerrainLayer(TERRAIN_REBUILD_VERTICES);
        else
            GfxTerrain->InvalidateTerrainVertex(iParam1, iParam2, TERRAIN_REBUILD_VERTICES);
        break;

    case VID_NOTIFY_TERRAIN_SHADER_MODIFIED:
        if (iParam3)
            GfxTerrain->InvalidateTerrainLayer(TERRAIN_REBUILD_SHADERS);
        else
            GfxTerrain->InvalidateTerrainVertex(iParam1, iParam2, TERRAIN_REBUILD_SHADERS);
        break;

    case VID_NOTIFY_TERRAIN_TEXCOORD_MODIFIED:
        if (iParam3)
            GfxTerrain->InvalidateTerrainLayer(TERRAIN_REBUILD_TEXCOORDS);
        else
            GfxTerrain->InvalidateTerrainVertex(iParam1, iParam2, TERRAIN_REBUILD_TEXCOORDS);
        break;
    case VID_NOTIFY_TERRAIN_TEXEL_ALPHA_MODIFIED:
        if (iParam3)
            GfxTerrain->InvalidateTerrainLayer(TERRAIN_REBUILD_SHADERS);
        else
            GfxTerrain->InvalidateTerrainTexel(iParam1, iParam2, TERRAIN_REBUILD_ALPHAMAP);
        break;
    case VID_NOTIFY_TEXTURE_FILTERING_SETTINGS_MODIFIED:
        GL_SetupTextureFilterSettings();
        break;
    case VID_NOTIFY_FOLIAGE_TEXTURE_MODIFIED:
        if (iParam3)
            GL_InvalidateFoliageLayer(FOLIAGE_REBUILD_SHADERS);
        else
            GL_InvalidateFoliageTile(iParam1, iParam2, FOLIAGE_REBUILD_SHADERS);
        break;

    case VID_NOTIFY_FOLIAGE_DENSITY_MODIFIED:
        if (iParam3)
            GL_InvalidateFoliageLayer(FOLIAGE_REBUILD_VERTICES);
        else
            GL_InvalidateFoliageVertex(iParam1, iParam2, FOLIAGE_REBUILD_VERTICES);
        break;

    case VID_NOTIFY_FOLIAGE_SIZE_MODIFIED:
        if (iParam3)
            GL_InvalidateFoliageLayer(FOLIAGE_REBUILD_VERTICES);
        else
            GL_InvalidateFoliageVertex(iParam1, iParam2, FOLIAGE_REBUILD_VERTICES);
        break;
    case VID_NOTIFY_X11_EVENT:
#ifdef linux
        GL_X11_Event(static_cast<XEvent*>(pData));
#else
        break;
#endif
    }
}


/*====================
  GL_Add2dRect
  ====================*/
void    GL_Add2dRect(float x, float y, float w, float h, float s1, float t1, float s2, float t2, ResHandle hTexture, int iFlags)
{
    Gfx2D->AddRect(x,y,w,h,s1,t1,s2,t2,hTexture,iFlags);
}


/*====================
  GL_Add2dQuad
  ====================*/
void    GL_Add2dQuad(const CVec2f& v1, const CVec2f& v2, const CVec2f&      v3, const CVec2f& v4, const CVec2f& t1, const CVec2f& t2, const CVec2f& t3, const CVec2f& t4, ResHandle hTexture, int iFlags)
{
    Gfx2D->AddQuad(v1,v2,v3,v4,t1,t2,t3,t4,hTexture,iFlags);
}


/*====================
  GL_Add2dLine
  ====================*/
void    GL_Add2dLine(const CVec2f& v1, const CVec2f& v2, const CVec4f &v4Color1, const CVec4f &v4Color2, int iFlags)
{
    Gfx2D->AddLine(v1,v2,v4Color1,v4Color2,iFlags);
}


/*====================
  GL_AddPoint
  ====================*/
void    GL_AddPoint(const CVec3f &v3Point, const CVec4f &v4Color)
{
    Gfx3D->AddPoint(v3Point, v4Color);
}


/*====================
  GL_AddLine
  ====================*/
void    GL_AddLine(const CVec3f &v3Start, const CVec3f &v3End, const CVec4f &v4Color)
{
    Gfx3D->AddLine(v3Start, v3End, v4Color);
}


/*====================
  GL_SetColor
  ====================*/
void    GL_SetColor(CVec4f v4Color)
{
    GfxUtils->SetCurrentColor(v4Color);
}


/*====================
  GL_GetFrameBuffer
  ====================*/
void    GL_GetFrameBuffer(CBitmap &bmp)
{
    bmp.Alloc(g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight, BITMAP_RGB);

    glReadPixels(0, 0, g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight, GL_RGB, GL_UNSIGNED_BYTE, bmp.GetBuffer());

    bmp.Flip();
}


/*====================
  GL_ProjectVertex
  ====================*/
CVec2f  GL_ProjectVertex(const CCamera &cam, const CVec3f &vecVertex)
{
    return CVec2f(0, 0);
}


/*====================
  GL_Clear
  ====================*/
void    GL_Clear()
{
    Gfx3D->Clear();
}


/*====================
  GL_RegisterTexture
  ====================*/
int     GL_RegisterTexture(CTexture *pTexture)
{
    return GfxTextures->RegisterTexture(pTexture);
}


/*====================
  GL_UnregisterTexture
  ====================*/
void    GL_UnregisterTexture(CTexture *pTexture)
{
    GfxTextures->UnregisterTexture(pTexture->GetPath());
}


/*====================
  GL_RegisterModel
  ====================*/
int     GL_RegisterModel(CModel *pModel)
{
    return GfxModels->RegisterModel(pModel);
}


/*====================
  GL_UnregisterModel
  ====================*/
void    GL_UnregisterModel(CModel *pModel)
{
    GfxModels->UnregisterModel(pModel);
}


/*====================
  GL_RegisterVertexShader
  ====================*/
int     GL_RegisterVertexShader(CVertexShader *pVertexShader)
{
    return GfxShaders->RegisterVertexShader(pVertexShader);
}


/*====================
  GL_UnregisterVertexShader
  ====================*/
void    GL_UnregisterVertexShader(CVertexShader *pVertexShader)
{
    GfxShaders->UnregisterVertexShader(pVertexShader);
}


/*====================
  GL_RegisterPixelShader
  ====================*/
int     GL_RegisterPixelShader(CPixelShader *pPixelShader)
{
    return GfxShaders->RegisterPixelShader(pPixelShader);
}


/*====================
  GL_UnregisterPixelShader
  ====================*/
void    GL_UnregisterPixelShader(CPixelShader *pPixelShader)
{
    return GfxShaders->UnregisterPixelShader(pPixelShader);
}


/*====================
  GL_RegisterShaderPair
  ====================*/
void    GL_RegisterShaderPair(CVertexShader *pVertexShader, CPixelShader *pPixelShader)
{   
    GfxShaders->RegisterShaderPair(pVertexShader, pPixelShader);
}


/*====================
  GL_GetCamera
  ====================*/
const CCamera*  GL_GetCamera()
{
    return g_pCam;
}


/*====================
  GL_RenderFogofWar
  ====================*/
void    GL_RenderFogofWar(float fClear, bool bTexture, float fLerp)
{
    g_FogofWar.Render(fClear, bTexture, fLerp);
}


/*====================
  GL_UpdateFogofWar
  ====================*/
void    GL_UpdateFogofWar(const CBitmap &cBmp)
{
    g_FogofWar.Update(cBmp);
}


/*====================
  GL_GetTextureColor
  ====================*/
CVec4f  GL_GetTextureColor(CTexture *pResource)
{
    glPushAttrib(GL_ENABLE_BIT);

    glActiveTextureARB(GL_TEXTURE0_ARB);

    glBindTexture(GL_TEXTURE_2D, pResource->GetIndex());

    int iMaxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)(&iMaxTextureSize));

    int iMaxLevel(INT_ROUND(log(float(iMaxTextureSize)) / log(2.0f)));

    int iLevel(0);
    
    GLint iTextureWidth;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, iLevel, GL_TEXTURE_WIDTH, &iTextureWidth);
    
    GLint iTextureHeight;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, iLevel, GL_TEXTURE_HEIGHT, &iTextureHeight);

    while (iLevel < iMaxLevel)
    {
        int iNextLevel(iLevel + 1);

        GLint iNextTextureWidth;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, iNextLevel, GL_TEXTURE_WIDTH, &iNextTextureWidth);
        
        GLint iNextTextureHeight;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, iNextLevel, GL_TEXTURE_WIDTH, &iNextTextureHeight);

        if (iNextTextureWidth == 0 || iNextTextureHeight == 0)
            break;

        iTextureWidth = iNextTextureWidth;
        iTextureHeight = iNextTextureHeight;
        iLevel = iNextLevel;
    }

    if (iTextureWidth == 1 && iTextureHeight == 1)
    {
        byte color[4];
        glGetTexImage(GL_TEXTURE_2D, iLevel, GL_RGBA, GL_UNSIGNED_BYTE, &color);

        glPopAttrib();

        return CVec4f(color[0] / 255.0f, color[1] / 255.0f, color[2] / 255.0f, color[3] / 255.0f);
    }
    else
    {
        glPopAttrib();

        return WHITE;
    }
    
}


/*====================
  InitAPIs

  Sets up calls for core engine into vid code
  ====================*/
void    InitAPIs_Global(SVidDriver *vid_api)
{
    vid_api->sDriverName = _T("OpenGL 2.1 [GLSL]");

    vid_api->Init = GL_Init;
    vid_api->Start = GL_Start;
    vid_api->SetMode = GL_SetMode;
    vid_api->GetMode = GL_GetMode;
    vid_api->GetCurrentMode = GL_GetCurrentMode;
    vid_api->IsFullScreen = GL_IsFullScreen;
    vid_api->Shutdown = GL_Shutdown;
    
    vid_api->TextureFilteringModeAvailable = GL_TextureFilteringModeAvailable;

    vid_api->BeginFrame = GL_BeginFrame;
    vid_api->EndFrame = GL_EndFrame;
    vid_api->RenderScene = GL_RenderScene;
    vid_api->Add2dRect = GL_Add2dRect;
    vid_api->Add2dQuad = GL_Add2dQuad;
    vid_api->Add2dLine = GL_Add2dLine;
    vid_api->AddPoint = GL_AddPoint;
    vid_api->AddLine = GL_AddLine;
    vid_api->SetColor = GL_SetColor;
    vid_api->Notify = GL_Notify;
    vid_api->GetFrameBuffer = GL_GetFrameBuffer;
    vid_api->ProjectVertex = GL_ProjectVertex;
    
    vid_api->GetHWnd = GL_GetHWnd;
    vid_api->Clear = GL_Clear;

    vid_api->OpenTextureArchive = GL_OpenTextureArchive;
    vid_api->CloseTextureArchive = GL_CloseTextureArchive;
    vid_api->GetTextureList = GL_GetTextureList;
    vid_api->TextureExists = GL_TextureExists;
    
    vid_api->RegisterTexture = GL_RegisterTexture;
    vid_api->UnregisterTexture = GL_UnregisterTexture;
    
    vid_api->RegisterVertexShader = GL_RegisterVertexShader;
    vid_api->UnregisterVertexShader = GL_UnregisterVertexShader;
    
    vid_api->RegisterPixelShader = GL_RegisterPixelShader;
    vid_api->UnregisterPixelShader = GL_UnregisterPixelShader;
    
    vid_api->RegisterShaderPair = GL_RegisterShaderPair;
    
    vid_api->RegisterModel = GL_RegisterModel;
    vid_api->UnregisterModel = GL_UnregisterModel;
    
    vid_api->GetCamera = GL_GetCamera;
    
    vid_api->RenderFogofWar = GL_RenderFogofWar;
    vid_api->UpdateFogofWar = GL_UpdateFogofWar;

    vid_api->ShowCursor = GL_ShowCursor;
    vid_api->SetCursor = GL_SetCursor;
    
    vid_api->GetTextureColor = GL_GetTextureColor;
    
    vid_api->GetAAMode = GL_GetAAMode;
    vid_api->GetCurrentAAMode = GL_GetCurrentAAMode;
}


/*--------------------
  cmdVidReset
  --------------------*/
CMD(VidReset)
{
    GL_SetMode();

    return true;
}


/*--------------------
  ListVidModes
  --------------------*/
CMD(ListVidModes)
{
    for (int i(0); i < g_iNumVidModes; ++i)
        Console << XtoA(i, 2) << _T(": ") << g_VidModes[i].sName << newl;
    return true;
}


