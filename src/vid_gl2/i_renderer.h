// (C)2006 S2 Games
// i_renderer.h
//
//=============================================================================
#ifndef __I_RENDERER_H__
#define __I_RENDERER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_treemodeldef.h"
#include "c_gfxmodels.h"

#include "../k2/c_pool.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EMaterialPhase;
class CCamera;
class CSceneEntity;
//=============================================================================

//=============================================================================
// IRenderer
//=============================================================================
class IRenderer
{
protected:
	// Sorting values
	bool			m_bRender;
	bool			m_bTranslucent;
	int				m_iLayer;
	int				m_iEffectLayer;
	int				m_iShaderProgramInstance;
	size_t			m_uiVertexBuffer;
	size_t			m_uiIndexBuffer;
	float			m_fDepth;
	bool			m_bRefractive;

	// Shadervars
	D3DXMATRIXA16	m_mWorld;
	D3DXMATRIXA16	m_mWorldViewProj;
	D3DXMATRIXA16	m_mWorldRotate;
	CVec3f			m_vSunColor;
	CVec3f			m_vAmbient;
	const CCamera	*m_pCam;
	const CSceneEntity	*m_pCurrentEntity;
	bool			m_bObjectColor;
	CVec4f			m_vObjectColor;
	CVec3f			m_vPointLightPosition[MAX_POINT_LIGHTS];
	CVec3f			m_vPointLightColor[MAX_POINT_LIGHTS];
	float			m_fPointLightFalloffStart[MAX_POINT_LIGHTS];
	float			m_fPointLightFalloffEnd[MAX_POINT_LIGHTS];
	int				m_iNumActivePointLights;
	float			m_afLeafClusterData[LEAF_CLUSTER_TABLE_SIZE];
	uint			m_uiLeafClusterDataSize;
	CVec4f			m_vBoneData[MAX_BONE_TABLE_SIZE];
	int				m_iNumActiveBones;
	bool			m_bLighting;
	bool			m_bShadows;
	bool			m_bFog;
	bool			m_bDepthFirst;
	bool			m_bTexkill;

public:
	virtual ~IRenderer();
	IRenderer();

	void			SetShaderVars();

	virtual void	Setup(EMaterialPhase ePhase) = 0;
	virtual void	Render(EMaterialPhase ePhase) = 0;

	// Sorting
	bool			GetRender() const					{ return m_bRender; }
	bool			IsTranslucent() const				{ return m_bTranslucent; }
	int				GetLayer() const					{ return m_iLayer; }
	int				GetEffectLayer() const				{ return m_iEffectLayer; }
	int				GetShaderProgramInstance() const	{ return m_iShaderProgramInstance; }
	size_t			GetVertexBuffer() const				{ return m_uiVertexBuffer; }
	size_t			GetIndexBuffer() const				{ return m_uiIndexBuffer; }
	float			GetDepth() const					{ return m_fDepth; }
	bool			IsRefractive() const				{ return m_bRefractive; }
};
//=============================================================================

#endif //__I_RENDERER_H__
