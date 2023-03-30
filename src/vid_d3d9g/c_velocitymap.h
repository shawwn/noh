// (C)2008 S2 Games
// c_velocitymap.h
//
//=============================================================================
#ifndef __C_VELOCITYMAP_H__
#define __C_VELOCITYMAP_H__

//=============================================================================
// Headers
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
class CCamera;

EXTERN_CVAR_BOOL(vid_motionBlur);
//=============================================================================

//=============================================================================
// CVelocityMap
//=============================================================================
class CVelocityMap
{
private:
    bool                        m_bActive;

    int                         m_iVelocityMap;
    IDirect3DTexture9           *m_pVelocityMap;
    IDirect3DSurface9           *m_pVelocityMapSurface;
    IDirect3DSurface9           *m_pDepthStencil;       // Depth-stencil buffer for rendering to the reflection map

    ResHandle                   m_hVelocityTexture;
    ResHandle                   m_hVelocityReference;

    uint                        m_uiWidth;
    uint                        m_uiHeight;

public:
    ~CVelocityMap();
    CVelocityMap();

    int     GetTextureIndex()           { return m_iVelocityMap; }
    bool    GetActive()                 { return m_bActive; }

    void    Initialize();
    void    Release();

    void    Render(const CCamera &cCamera);
};

extern CVelocityMap g_VelocityMap;
//=============================================================================

#endif //__C_VELOCITYMAP_H__
