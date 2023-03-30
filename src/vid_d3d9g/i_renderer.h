// (C)2006 S2 Games
// i_renderer.h
//
//=============================================================================
#ifndef __I_RENDERER_H__
#define __I_RENDERER_H__

//=============================================================================
// Headers
//=============================================================================
#include "d3d9g_main.h"
#include "d3d9g_model.h"
#include "c_treemodeldef.h"

#include "../k2/c_pool.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EMaterialPhase;
class CCamera;
class CSceneEntity;

enum ERenderType
{
    RT_UNKNOWN,
    RT_MESH
};
//=============================================================================

//=============================================================================
// IRenderer
//=============================================================================
class IRenderer
{
protected:
    //ERenderType       m_eType;

    // Sorting values
    bool            m_bRender;
    bool            m_bTranslucent;
    int             m_iLayer;
    int             m_iEffectLayer;
    int             m_iVertexType;
    int             m_iVertexShaderInstance;
    int             m_iPixelShaderInstance;
    size_t          m_uiVertexBuffer;
    size_t          m_uiIndexBuffer;
    float           m_fDepth;
    bool            m_bRefractive;

    // Shadervars
    D3DXMATRIXA16   m_mWorld;
    D3DXMATRIXA16   m_mWorldViewProj;
    D3DXMATRIXA16   m_mWorldRotate;
    CVec3f          m_vSunColor;
    CVec3f          m_vAmbient;
    const CCamera   *m_pCam;
    const CSceneEntity  *m_pCurrentEntity;
    bool            m_bObjectColor;
    CVec4f          m_vObjectColor;
    CVec3f          m_vPointLightPosition[MAX_POINT_LIGHTS];
    CVec3f          m_vPointLightColor[MAX_POINT_LIGHTS];
    float           m_fPointLightFalloffStart[MAX_POINT_LIGHTS];
    float           m_fPointLightFalloffEnd[MAX_POINT_LIGHTS];
    int             m_iNumActivePointLights;
    float           m_afLeafClusterData[LEAF_CLUSTER_TABLE_SIZE];
    uint            m_uiLeafClusterDataSize;
    D3DXMATRIXA16   m_vBoneData[MAX_BONE_TABLE_SIZE];
    int             m_iNumActiveBones;
    bool            m_bLighting;
    bool            m_bShadows;
    bool            m_bFog;
    bool            m_bDepthFirst;
    int             m_iTexcoords;
    bool            m_bTexkill;

public:
    virtual ~IRenderer();
    IRenderer(ERenderType eType);

    void            SetShaderVars();

    virtual void    Setup(EMaterialPhase ePhase) = 0;
    virtual void    Render(EMaterialPhase ePhase) = 0;

    // Sorting
    bool            GetRender() const               { return m_bRender; }
    bool            IsTranslucent() const           { return m_bTranslucent; }
    int             GetLayer() const                { return m_iLayer; }
    int             GetEffectLayer() const          { return m_iEffectLayer; }
    int             GetVertexType() const           { return m_iVertexType; }
    int             GetVertexShaderInstance() const { return m_iVertexShaderInstance; }
    int             GetPixelShaderInstance() const  { return m_iPixelShaderInstance; }
    size_t          GetVertexBuffer() const         { return m_uiVertexBuffer; }
    size_t          GetIndexBuffer() const          { return m_uiIndexBuffer; }
    float           GetDepth() const                { return m_fDepth; }
    bool            IsRefractive() const            { return m_bRefractive; }
};
//=============================================================================

#endif //__I_RENDERER_H__
