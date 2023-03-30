// (C)2008 S2 Games
// i_instancer.h
//
//=============================================================================
#ifndef __I_INSTANCER_H__
#define __I_INSTANCER_H__

//=============================================================================
// Headers
//=============================================================================
#include "d3d9_main.h"
#include "d3d9_model.h"
#include "c_treemodeldef.h"

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
// IInstancer
//=============================================================================
class IInstancer
{
	struct SInstanceShaderVars
	{
		D3DXMATRIXA16	mWorld;
		CVec4f			vColor;
	};

protected:
	// Shadervars
	SInstanceShaderVars	m_cInstanceVars[1];
	
	CVec3f			m_vSunColor;
	CVec3f			m_vAmbient;
	const CCamera	*m_pCam;
	CVec3f			m_vPointLightPosition[MAX_POINT_LIGHTS];
	CVec3f			m_vPointLightColor[MAX_POINT_LIGHTS];
	float			m_fPointLightFalloffStart[MAX_POINT_LIGHTS];
	float			m_fPointLightFalloffEnd[MAX_POINT_LIGHTS];
	int				m_iNumActivePointLights;
	float			m_afLeafClusterData[LEAF_CLUSTER_TABLE_SIZE];
	uint			m_uiLeafClusterDataSize;
	D3DXMATRIXA16	m_vBoneData[MAX_BONE_TABLE_SIZE];
	int				m_iNumActiveBones;
	bool			m_bLighting;
	bool			m_bShadows;
	bool			m_bFog;
	bool			m_bDepthFirst;
	int				m_iTexcoords;
	bool			m_bTexkill;

public:
	virtual ~IInstancer();
	IInstancer();

	void			SetShaderVars();

	virtual void	Setup(EMaterialPhase ePhase) = 0;
	virtual void	Render(EMaterialPhase ePhase) = 0;
};
//=============================================================================

#endif //__I_INSTANCER_H__
