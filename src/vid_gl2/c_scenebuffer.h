// (C)2009 S2 Games
// c_scenebuffer.h
//
//=============================================================================
#ifndef __C_SCENEBUFFER_H__
#define __C_SCENEBUFFER_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/c_material.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
EXTERN_CVAR_BOOL(vid_sceneBuffer);

#if 1
EXTERN_CVAR_BOOL(vid_sceneBufferMipmap);
#else
const bool vid_sceneBufferMipmap(true);
#endif
//=============================================================================

//=============================================================================
// CSceneBuffer
//=============================================================================
class CSceneBuffer
{
private:
    bool                    m_bActive;

    uint                    m_uiWidth;
    uint                    m_uiHeight;
    
    uint                    m_uiSceneBuffer;

    ResHandle               m_hSceneBufferTexture;

public:
    ~CSceneBuffer();
    CSceneBuffer();

    bool    GetActive()             { return m_bActive; }
    uint    GetWidth()              { return m_uiWidth; }
    uint    GetHeight()             { return m_uiHeight; }
    uint    GetTextureIndex()       { return m_uiSceneBuffer; }

    void    Initialize(int iWidth, int iHeight);
    void    Release();
    
    void    Render();
};

extern CSceneBuffer g_SceneBuffer;
//=============================================================================

#endif //__C_SCENEBUFFER_H__
