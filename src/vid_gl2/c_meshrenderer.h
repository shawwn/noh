// (C)2005 S2 Games
// c_meshrenderer.h
//
//=============================================================================
#ifndef __C_MESHRENDERER_H__
#define __C_MESHRENDERER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_renderer.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
class   CSceneEntity;
class   CMesh;
enum    EMaterialPhase;
//=============================================================================

//=============================================================================
// CMeshRenderer
//=============================================================================
class CMeshRenderer : public IRenderer
{
private:
    const CSceneEntity  &m_cEntity;

    ResHandle       m_hMaterial;
    CMesh           *m_pMesh;
    CMaterial       *m_pMaterial;
    D3DXMATRIXA16   m_mWorldEntity;
    D3DXMATRIXA16   m_mWorldEntityRotation;

    const int       *m_pMapping;
    int             m_iCurrentSkelbone;

    bool    SetupObjectMatrix();
    bool    SetRenderStates(EMaterialPhase ePhase);

    void    DrawMesh(EMaterialPhase ePhase);
    bool    DrawSkinnedMesh();
    bool    DrawSkinnedMeshGPU();
    bool    DrawStaticMesh();

    void    DrawNormals();
    bool    DrawSkinnedNormals();
    bool    DrawStaticNormals();

public:
    static CPool<CMeshRenderer>     s_Pool;
    
    void*   operator new(size_t z, const char *szContext = NULL, const char *szType = NULL, const char *szFile = NULL, short nLine = 0); // Uses CPool of preallocated instances
    void    operator delete(void *p) { }
    void    operator delete(void *p, const char *szContext, const char *szType, const char *szFile, short nLine) { }

    ~CMeshRenderer();
    CMeshRenderer(ResHandle hMaterial, const CSceneEntity &cEntity, CMesh *m_pMesh, const D3DXMATRIXA16 &mWorldEntity, const D3DXMATRIXA16 &mWorldEntityRotation);

    void    Setup(EMaterialPhase ePhase);
    void    Render(EMaterialPhase ePhase);
};
//=============================================================================

#endif //__C_MESHRENDERITEM_H__
