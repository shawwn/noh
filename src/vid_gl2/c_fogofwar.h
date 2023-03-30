// (C)2007 S2 Games
// c_fogofwar.h
//
//=============================================================================
#ifndef __C_FOGOFWAR_H__
#define __C_FOGOFWAR_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_gfx3d.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const int   MAX_FOWQUADS = 2;
//=============================================================================

//=============================================================================
// CFogofWar
//=============================================================================
class CFogofWar
{
private:
    GLuint          m_uiFrameBufferObject;
    GLuint          m_uiColorTexture;

    GLuint          m_uiPixelBufferObject;
    GLuint          m_uiDynamicTexture0;
    GLuint          m_uiDynamicTexture1;

    ResHandle       m_hFogofWarTexture;
    ResHandle       m_hFogofWarTexture0;
    ResHandle       m_hFogofWarTexture1;

    bool            m_bValid;

    float           m_fWorldWidth;
    float           m_fWorldHeight;
    int             m_iNextTexturemap;

    GLuint          VBFowQuad;

    void    DrawTexture(float fLerp);

public:
    ~CFogofWar();
    CFogofWar();

    int     GetTextureIndex()       { return m_uiColorTexture; }

    void    Initialize();
    void    Release();

    void    Render(float fClear, bool bTexture, float fLerp);
    void    Update(const CBitmap &cBmp);

    void    AddRect(float x, float y, float w, float h, float s1, float t1, float s2, float t2, ResHandle hTexture);

    bool    IsValid() const         { return m_bValid; }
};

extern CFogofWar        g_FogofWar;
//=============================================================================
#endif //__C_FOGOFWAR_H__
