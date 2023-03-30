// (C)2008 S2 Games
// c_reflectionmap.h
//
//=============================================================================
#ifndef __C_REFLECTIONMAP_H__
#define __C_REFLECTIONMAP_H__

//=============================================================================
// Headers
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
class CCamera;

EXTERN_CVAR_BOOL(vid_reflections);
const bool REFLECTIONS_MIPMAP(false);
//=============================================================================

//=============================================================================
// CReflectionMap
//=============================================================================
class CReflectionMap
{
private:
    bool                        m_bActive;

    int                         m_iReflectionMap;
    IDirect3DTexture9           *m_pReflectionMap;
    IDirect3DSurface9           *m_pReflectionMapSurface;
    IDirect3DSurface9           *m_pDepthStencil;       // Depth-stencil buffer for rendering to the reflection map

    ResHandle                   m_hReflectionTexture;

    uint                        m_uiWidth;
    uint                        m_uiHeight;

public:
    ~CReflectionMap();
    CReflectionMap();

    int     GetTextureIndex()           { return m_iReflectionMap; }
    bool    GetActive()                 { return m_bActive; }

    void    Initialize(int iWidth, int iHeight);
    void    Release();

    void    Render(const CCamera &cCamera);
};

extern CReflectionMap   g_ReflectionMap;
//=============================================================================

#endif //__C_REFLECTIONMAP_H__
