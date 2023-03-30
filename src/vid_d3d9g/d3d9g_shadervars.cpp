// (C)2005 S2 Games
// d3d9g_shadervars.cpp
//
// Direct3D Shader Variables
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "d3d9g_main.h"
#include "d3d9g_material.h"
#include "d3d9g_model.h"
#include "d3d9g_texture.h"
#include "d3d9g_scene.h"
#include "d3d9g_state.h"
#include "d3d9g_terrain.h"
#include "d3d9g_foliage.h"
#include "d3d9g_shader.h"
#include "c_treemodeldef.h"
#include "c_shadervar.h"
#include "c_shadowmap.h"

#include "../k2/c_vec3.h"
#include "../k2/c_mesh.h"
#include "../k2/c_camera.h"
#include "../k2/c_material.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_host.h"
#include "../k2/c_world.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_VEC3F		(gfx_fogColor,		CVec3f(0.75f, 0.75f, 0.75f),	CVAR_WORLDCONFIG);
CVAR_FLOATF		(gfx_fogNear,		4500.0f,						CVAR_WORLDCONFIG);
CVAR_FLOATF		(gfx_fogFar,		9000.0f,						CVAR_WORLDCONFIG);
CVAR_FLOATF		(gfx_fogDensity,	0.0005f,						CVAR_WORLDCONFIG);
CVAR_FLOATF		(gfx_fogScale,		1.0f,							CVAR_WORLDCONFIG);
//=============================================================================

/*====================
  D3D_SetFloat
  ====================*/
static inline
void	D3D_SetFloat(float *pRegisters, float fFloat)
{
	*pRegisters = fFloat;
}


/*====================
  D3D_SetFloatArray
  ====================*/
static inline
void	D3D_SetFloatArray(float *pRegisters, float *pFloats, uint uiCount)
{
	for (uint ui(0); ui < uiCount; ++ui)
	{
		*pRegisters = *pFloats;
		pRegisters += 4;
		++pFloats;
	}
}


/*====================
  D3D_SetVector
  ====================*/
static inline
void	D3D_SetVector(float *pRegisters, const D3DXVECTOR4 *pVector)
{
	MemManager.Copy(pRegisters, pVector, sizeof(D3DXVECTOR4));
}


/*====================
  D3D_SetVectorArray
  ====================*/
static inline
void	D3D_SetVectorArray(float *pRegisters, const D3DXVECTOR4 *pVector, uint uiCount)
{
	MemManager.Copy(pRegisters, pVector, sizeof(D3DXVECTOR4) * uiCount);
}


/*====================
  D3D_SetMatrix
  ====================*/
static inline
void	D3D_SetMatrix(float *pRegisters, uint uiSize, const D3DXMATRIX *pMatrix)
{
	if (uiSize == 4)
	{
		D3DXMatrixTranspose((D3DXMATRIX *)pRegisters, pMatrix);
	}
	else
	{
		for (uint uiX(0); uiX < uiSize; ++uiX)
		{
			const float *pSrc(&pMatrix->_11 + uiX);

			for (uint uiY(0); uiY < 4; ++uiY)
			{
				*pRegisters = *pSrc;
				++pRegisters;
				pSrc += 4;
			}
		}
	}
}


/*====================
  D3D_SetMatrixArray
  ====================*/
static inline
void	D3D_SetMatrixArray(float *pRegisters, uint uiSize, const D3DXMATRIX *pMatrix, uint uiCount)
{
	if (uiSize == 4)
	{
		MemManager.Copy(pRegisters, pMatrix, sizeof(D3DXVECTOR4) * uiSize * uiSize);
	}
	else
	{
		for (uint ui(0); ui < uiCount; ++ui)
		{
			for (uint uiX(0); uiX < uiSize; ++uiX)
			{
				const float *pSrc(&pMatrix[ui]._11 + uiX);

				for (uint uiY(0); uiY < 4; ++uiY)
				{
					*pRegisters = *pSrc;
					++pRegisters;
					pSrc += 4;
				}
			}
		}
	}
}


/*--------------------
  mWorldViewProj
  --------------------*/
SHADER_VAR(mWorldViewProj)
{
	D3D_SetMatrix(pRegisters, uiSize, &g_mWorldViewProj);
	return true;
}


/*--------------------
  mWorldViewProjInv
  --------------------*/
SHADER_VAR(mWorldViewProjInv)
{
	D3DXMATRIXA16 mWorldViewProjInv;
	D3DXMatrixInverse(&mWorldViewProjInv, NULL, &g_mWorldViewProj);

	D3D_SetMatrix(pRegisters, uiSize, &mWorldViewProjInv);
	return true;
}


/*--------------------
  mWorld
  --------------------*/
SHADER_VAR(mWorld)
{
	D3D_SetMatrix(pRegisters, uiSize, &g_mWorld);
	return true;
}


/*--------------------
  mView
  --------------------*/
SHADER_VAR(mView)
{

	D3D_SetMatrix(pRegisters, uiSize, &g_mView);
	return true;
}

/*--------------------
  mProj
  --------------------*/
SHADER_VAR(mProj)
{
	D3D_SetMatrix(pRegisters, uiSize, &g_mProj);
	return true;
}


/*--------------------
  mWorldView
  --------------------*/
SHADER_VAR(mWorldView)
{
	D3DXMATRIXA16 mWorldView = g_mWorld * g_mView;

	D3D_SetMatrix(pRegisters, uiSize, &mWorldView);
	return true;
}


/*--------------------
  mWorldViewRotate
  --------------------*/
SHADER_VAR(mWorldViewRotate)
{
	D3DXMATRIXA16 mWorldViewRotate = g_mWorldRotate * g_mViewRotate;

	D3D_SetMatrix(pRegisters, uiSize, &mWorldViewRotate);
	return true;
}


/*--------------------
  mWorldViewOffset
  --------------------*/
SHADER_VAR(mWorldViewOffset)
{
	D3DXMATRIXA16 mWorldViewOffset = g_mWorld * g_mViewOffset;

	D3D_SetMatrix(pRegisters, uiSize, &mWorldViewOffset);
	return true;
}


/*--------------------
  mWorldInverse
  --------------------*/
SHADER_VAR(mWorldInverse)
{
	D3DXMATRIXA16 mWorldInverse;

	D3DXMatrixInverse(&mWorldInverse, NULL, &g_mWorld);

	D3D_SetMatrix(pRegisters, uiSize, &mWorldInverse);
	return true;
}


/*--------------------
  mWorldRotate
  --------------------*/
SHADER_VAR(mWorldRotate)
{
	D3D_SetMatrix(pRegisters, uiSize, &g_mWorldRotate);
	return true;
}


/*--------------------
  mWorldInverseRotate
  --------------------*/
SHADER_VAR(mWorldInverseRotate)
{
	D3DXMATRIXA16 mWorldInverseRotate;
	D3DXMatrixTranspose(&mWorldInverseRotate, &g_mWorldRotate);

	D3D_SetMatrix(pRegisters, uiSize, &mWorldInverseRotate);
	return true;
}


/*--------------------
  mViewProj
  --------------------*/
SHADER_VAR(mViewProj)
{
	D3D_SetMatrix(pRegisters, uiSize, &g_mViewProj);
	return true;
}


/*--------------------
  fTime
  --------------------*/
SHADER_VAR(fTime)
{
	D3D_SetFloat(pRegisters, (g_pCam != NULL) ? (g_pCam->GetTime()) : MsToSec(Host.GetSystemTime()));
	return true;
}


/*--------------------
  fSpecularLevel
  --------------------*/
SHADER_VAR(fSpecularLevel)
{
	D3D_SetFloat(pRegisters, g_pCurrentMaterial->GetSpecularLevel());
	return true;
}


/*--------------------
  fGlossiness
  --------------------*/
SHADER_VAR(fGlossiness)
{
	D3D_SetFloat(pRegisters, g_pCurrentMaterial->GetGlossiness());
	return true;
}


/*--------------------
  vSpec
  --------------------*/
SHADER_VAR(vSpec)
{
	D3DXVECTOR4 vSpec(g_pCurrentMaterial->GetSpecularLevel(), g_pCurrentMaterial->GetGlossiness(), 0.0f, 0.0f);

	D3D_SetVector(pRegisters, &vSpec);
	return true;
}


/*--------------------
  fBumpLevel
  --------------------*/
SHADER_VAR(fBumpLevel)
{
	D3D_SetFloat(pRegisters, g_pCurrentMaterial->GetBumpLevel());
	return true;
}


/*--------------------
  fReflect
  --------------------*/
SHADER_VAR(fReflect)
{
	D3D_SetFloat(pRegisters, g_pCurrentMaterial->GetReflect());
	return true;
}


/*--------------------
  vDiffuseColor
  --------------------*/
SHADER_VAR(vDiffuseColor)
{
	D3DXVECTOR4 vDiffuseColor
	(
		g_pCurrentMaterial->GetDiffuseColor().x,
		g_pCurrentMaterial->GetDiffuseColor().y,
		g_pCurrentMaterial->GetDiffuseColor().z,
		g_pCurrentMaterial->GetOpacity()
	);

	D3D_SetVector(pRegisters, &vDiffuseColor);
	return true;
}


/*--------------------
  vSunColor
  --------------------*/
SHADER_VAR(vSunColor)
{
	D3DXVECTOR4 vSunColor(g_v3SunColor.x, g_v3SunColor.y, g_v3SunColor.z, 1.0f);

	D3D_SetVector(pRegisters, &vSunColor);
	return true;
}



/*--------------------
  vSunColorSpec
  --------------------*/
SHADER_VAR(vSunColorSpec)
{
	D3DXVECTOR4 vSunColor
	(
		g_v3SunColor.x * g_pCurrentMaterial->GetSpecularLevel(),
		g_v3SunColor.y * g_pCurrentMaterial->GetSpecularLevel(),
		g_v3SunColor.z * g_pCurrentMaterial->GetSpecularLevel(),
		1.0f
	);

	D3D_SetVector(pRegisters, &vSunColor);
	return true;
}


/*--------------------
  vAmbient
  --------------------*/
SHADER_VAR(vAmbient)
{
	D3DXVECTOR4 vAmbient(g_v3Ambient.x, g_v3Ambient.y, g_v3Ambient.z, 1.0f);

	D3D_SetVector(pRegisters, &vAmbient);
	return true;
}


/*--------------------
  vGroundAmbient
  --------------------*/
SHADER_VAR(vGroundAmbient)
{
	D3DXVECTOR4 vGroundAmbient(g_v3Ambient.x * 0.25f, g_v3Ambient.y * 0.25f, g_v3Ambient.z * 0.25f, 1.0f);

	D3D_SetVector(pRegisters, &vGroundAmbient);
	return true;
}


/*--------------------
  vSunPosition
  --------------------*/
SHADER_VAR(vSunPosition)
{
	CVec3f v3SunPos(SceneManager.GetSunPos());
	D3DXVECTOR4	vSunPosition(v3SunPos.x, v3SunPos.y, v3SunPos.z, 1.0f);

	D3DXVec3Normalize((D3DXVECTOR3*)&vSunPosition, (D3DXVECTOR3*)&vSunPosition);

	D3DXMATRIXA16 mWorldInverseRotate;
	D3DXMatrixTranspose(&mWorldInverseRotate, &g_mWorldRotate);

	D3DXVec4Transform(&vSunPosition, &vSunPosition, &mWorldInverseRotate);

	D3D_SetVector(pRegisters, &vSunPosition);
	return true;
}


/*--------------------
  vSunPositionWorld
  --------------------*/
SHADER_VAR(vSunPositionWorld)
{
	CVec3f v3SunPos(SceneManager.GetSunPos());
	D3DXVECTOR4	vSunPosition(v3SunPos.x, v3SunPos.y, v3SunPos.z, 1.0f);

	D3DXVec3Normalize((D3DXVECTOR3*)&vSunPosition, (D3DXVECTOR3*)&vSunPosition);

	D3D_SetVector(pRegisters, &vSunPosition);
	return true;
}


/*--------------------
  vSunPositionView
  --------------------*/
SHADER_VAR(vSunPositionView)
{
	CVec3f v3SunPos(SceneManager.GetSunPos());
	D3DXVECTOR4	vSunPosition(v3SunPos.x, v3SunPos.y, v3SunPos.z, 1.0f);

	D3DXVec3Normalize((D3DXVECTOR3*)&vSunPosition, (D3DXVECTOR3*)&vSunPosition);

	D3DXVec3TransformNormal((D3DXVECTOR3*)&vSunPosition, (D3DXVECTOR3*)&vSunPosition, &g_mViewRotate);

	D3D_SetVector(pRegisters, &vSunPosition);
	return true;
}


/*--------------------
  vCameraPosition
  --------------------*/
SHADER_VAR(vCameraPosition)
{
	D3DXMATRIXA16 mWorldInverse;
	D3DXMatrixInverse(&mWorldInverse, NULL, &g_mWorld);

	D3DXVECTOR4	vCameraPosition;

	if (g_pCam)
	{
		vCameraPosition.x = g_pCam->GetOrigin(X);
		vCameraPosition.y = g_pCam->GetOrigin(Y);
		vCameraPosition.z = g_pCam->GetOrigin(Z);
	}
	else
	{
		vCameraPosition.x = 0.0f;
		vCameraPosition.y = 0.0f;
		vCameraPosition.z = 0.0f;
	}

	vCameraPosition.w = 1.0f;

	D3DXVec4Transform(&vCameraPosition, &vCameraPosition, &mWorldInverse);

	D3D_SetVector(pRegisters, &vCameraPosition);
	return true;
}


/*--------------------
  vCameraPositionWorld
  --------------------*/
SHADER_VAR(vCameraPositionWorld)
{
	D3DXVECTOR4	vCameraPosition;

	if (g_pCam)
	{
		vCameraPosition.x = g_pCam->GetOrigin(X);
		vCameraPosition.y = g_pCam->GetOrigin(Y);
		vCameraPosition.z = g_pCam->GetOrigin(Z);
	}
	else
	{
		vCameraPosition.x = 0.0f;
		vCameraPosition.y = 0.0f;
		vCameraPosition.z = 0.0f;
	}

	vCameraPosition.w = 1.0f;

	D3D_SetVector(pRegisters, &vCameraPosition);
	return true;
}


/*--------------------
  vCameraForward
  --------------------*/
SHADER_VAR(vCameraForward)
{
	D3DXVECTOR4	vCameraForward;

	if (g_pCam)
	{
		vCameraForward.x = g_pCam->GetViewAxis(FORWARD).x;
		vCameraForward.y = g_pCam->GetViewAxis(FORWARD).y;
		vCameraForward.z = g_pCam->GetViewAxis(FORWARD).z;
	}
	else
	{
		vCameraForward.x = 0.0f;
		vCameraForward.y = 0.0f;
		vCameraForward.z = 1.0f;
	}

	vCameraForward.w = 1.0f;

	D3DXMATRIXA16 mWorldInverseRotate;
	D3DXMatrixTranspose(&mWorldInverseRotate, &g_mWorldRotate);

	D3DXVec4Transform(&vCameraForward, &vCameraForward, &mWorldInverseRotate);

	D3D_SetVector(pRegisters, &vCameraForward);
	return true;
}


/*--------------------
  vCameraRight
  --------------------*/
SHADER_VAR(vCameraRight)
{
	D3DXVECTOR4	vCameraRight;

	if (g_pCam)
	{
		vCameraRight.x = g_pCam->GetViewAxis(RIGHT).x;
		vCameraRight.y = g_pCam->GetViewAxis(RIGHT).y;
		vCameraRight.z = g_pCam->GetViewAxis(RIGHT).z;
	}
	else
	{
		vCameraRight.x = 1.0f;
		vCameraRight.y = 0.0f;
		vCameraRight.z = 0.0f;
	}

	vCameraRight.w = 1.0f;

	D3DXMATRIXA16 mWorldInverseRotate;
	D3DXMatrixTranspose(&mWorldInverseRotate, &g_mWorldRotate);

	D3DXVec4Transform(&vCameraRight, &vCameraRight, &mWorldInverseRotate);

	D3D_SetVector(pRegisters, &vCameraRight);
	return true;
}


/*--------------------
  vCameraUp
  --------------------*/
SHADER_VAR(vCameraUp)
{
	D3DXVECTOR4	vCameraUp;

	if (g_pCam)
	{
		vCameraUp.x = g_pCam->GetViewAxis(UP).x;
		vCameraUp.y = g_pCam->GetViewAxis(UP).y;
		vCameraUp.z = g_pCam->GetViewAxis(UP).z;
	}
	else
	{
		vCameraUp.x = 0.0f;
		vCameraUp.y = 0.0f;
		vCameraUp.z = 1.0f;
	}

	vCameraUp.w = 1.0f;

	D3DXMATRIXA16 mWorldInverseRotate;
	D3DXMatrixTranspose(&mWorldInverseRotate, &g_mWorldRotate);

	D3DXVec4Transform(&vCameraUp, &vCameraUp, &mWorldInverseRotate);

	D3D_SetVector(pRegisters, &vCameraUp);
	return true;
}


/*--------------------
  vUp
  --------------------*/
SHADER_VAR(vUp)
{
	D3DXVECTOR4	vUp;

	vUp.x = 0.0f;
	vUp.y = 0.0f;
	vUp.z = 1.0f;
	vUp.w = 1.0f;

	D3DXMATRIXA16 mWorldInverseRotate;
	D3DXMatrixTranspose(&mWorldInverseRotate, &g_mWorldRotate);

	D3DXVec4Transform(&vUp, &vUp, &mWorldInverseRotate);

	D3D_SetVector(pRegisters, &vUp);
	return true;
}


/*--------------------
  vViewUp
  --------------------*/
SHADER_VAR(vViewUp)
{
	D3DXVECTOR4	vUp;

	vUp.x = 0.0f;
	vUp.y = 0.0f;
	vUp.z = 1.0f;
	vUp.w = 1.0f;

	D3DXVec3TransformNormal((D3DXVECTOR3 *)&vUp, (D3DXVECTOR3 *)&vUp, &g_mViewRotate);

	D3D_SetVector(pRegisters, &vUp);
	return true;
}


/*--------------------
  vObjectColor
  --------------------*/
SHADER_VAR(vObjectColor)
{
	D3DXVECTOR4	vObjectColor;

	if (g_pCurrentEntity)
	{
		if (g_pCurrentEntity->flags & SCENEENT_SOLID_COLOR)
		{
			vObjectColor.x = g_pCurrentEntity->color[0];
			vObjectColor.y = g_pCurrentEntity->color[1];
			vObjectColor.z = g_pCurrentEntity->color[2];
			vObjectColor.w = g_pCurrentEntity->color[3];
		}
		else
		{
			vObjectColor.x = 1.0f;
			vObjectColor.y = 1.0f;
			vObjectColor.z = 1.0f;
			vObjectColor.w = 1.0f;
		}
	}
	else if (g_bObjectColor)
	{
		vObjectColor.x = g_vObjectColor.x;
		vObjectColor.y = g_vObjectColor.y;
		vObjectColor.z = g_vObjectColor.z;
		vObjectColor.w = g_vObjectColor.w;
	}
	else
	{
		vObjectColor.x = 1.0f;
		vObjectColor.y = 1.0f;
		vObjectColor.z = 1.0f;
		vObjectColor.w = 1.0f;
	}

	D3D_SetVector(pRegisters, &vObjectColor);
	return true;
}


/*--------------------
  vColor
  --------------------*/
SHADER_VAR(vColor)
{
	D3DXVECTOR4 vDiffuseColor
	(
		g_pCurrentMaterial->GetDiffuseColor().x,
		g_pCurrentMaterial->GetDiffuseColor().y,
		g_pCurrentMaterial->GetDiffuseColor().z,
		g_pCurrentMaterial->GetOpacity()
	);

	D3DXVECTOR4	vObjectColor;

	if (g_pCurrentEntity)
	{
		if (g_pCurrentEntity->flags & SCENEENT_SOLID_COLOR)
		{
			vObjectColor.x = g_pCurrentEntity->color[0];
			vObjectColor.y = g_pCurrentEntity->color[1];
			vObjectColor.z = g_pCurrentEntity->color[2];
			vObjectColor.w = g_pCurrentEntity->color[3];
		}
		else
		{
			vObjectColor.x = 1.0f;
			vObjectColor.y = 1.0f;
			vObjectColor.z = 1.0f;
			vObjectColor.w = 1.0f;
		}
	}
	else if (g_bObjectColor)
	{
		vObjectColor.x = g_vObjectColor.x;
		vObjectColor.y = g_vObjectColor.y;
		vObjectColor.z = g_vObjectColor.z;
		vObjectColor.w = g_vObjectColor.w;
	}
	else
	{
		vObjectColor.x = 1.0f;
		vObjectColor.y = 1.0f;
		vObjectColor.z = 1.0f;
		vObjectColor.w = 1.0f;
	}

	D3DXVECTOR4 vColor;
	vColor.x = vObjectColor.x * vDiffuseColor.x;
	vColor.y = vObjectColor.y * vDiffuseColor.y;
	vColor.z = vObjectColor.z * vDiffuseColor.z;
	vColor.w = vObjectColor.w * vDiffuseColor.w;

	D3D_SetVector(pRegisters, &vColor);
	return true;
}


/*--------------------
  vTeamColor
  --------------------*/
SHADER_VAR(vTeamColor)
{
	D3DXVECTOR4	vTeamColor;

	if (g_pCurrentEntity)
	{
		vTeamColor.x = g_pCurrentEntity->teamcolor[0];
		vTeamColor.y = g_pCurrentEntity->teamcolor[1];
		vTeamColor.z = g_pCurrentEntity->teamcolor[2];
		vTeamColor.w = g_pCurrentEntity->teamcolor[3];
	}
	else
	{
		vTeamColor.x = 1.0f;
		vTeamColor.y = 1.0f;
		vTeamColor.z = 1.0f;
		vTeamColor.w = 1.0f;
	}

	D3D_SetVector(pRegisters, &vTeamColor);
	return true;
}


/*--------------------
  mLightWorldViewProjTex
  --------------------*/
SHADER_VAR(mLightWorldViewProjTex)
{
	D3DXMATRIXA16 mLightWorldViewProj = g_mWorld * g_mLightViewProjTex;

	D3D_SetMatrix(pRegisters, uiSize, &mLightWorldViewProj);
	return true;
}


#if 0
/*--------------------
  mLightWorldViewProjTexSplit
  --------------------*/
SHADER_VAR(mLightWorldViewProjTexSplit)
{
	float fOffsetX, fOffsetY;

	if (g_bSplitLightProjection)
	{
		D3DXMATRIX mTexShadowScale(0.25f,  0.0f,  0.0f, 0.0f,
								0.0f,  -0.25f, 0.0f, 0.0f,
								0.0f,   0.0f,  1.0f, 0.0f,
								0.0f,   0.0f,  0.0f, 1.0f);

		D3DXMATRIX mTexShadowOffset1[4];

		// Quadrant I
		fOffsetX = 0.75f;
		fOffsetY = 0.25f;

		mTexShadowOffset1[0] = D3DXMATRIX(1.0f,     0.0f,      0.0f, 0.0f,
										0.0f,     1.0f,      0.0f, 0.0f,
										0.0f,     0.0f,      1.0f, 0.0f,
										fOffsetX, fOffsetY,  0.0f, 1.0f);

		// Quadrant II
		fOffsetX = 0.25f;
		fOffsetY = 0.25f;

		mTexShadowOffset1[1] = D3DXMATRIX(1.0f,     0.0f,      0.0f, 0.0f,
										0.0f,     1.0f,      0.0f, 0.0f,
										0.0f,     0.0f,      1.0f, 0.0f,
										fOffsetX, fOffsetY,  0.0f, 1.0f);

		// Quadrant III
		fOffsetX = 0.25f;
		fOffsetY = 0.75f;

		mTexShadowOffset1[2] = D3DXMATRIX(1.0f,     0.0f,      0.0f, 0.0f,
										0.0f,     1.0f,      0.0f, 0.0f,
										0.0f,     0.0f,      1.0f, 0.0f,
										fOffsetX, fOffsetY,  0.0f, 1.0f);

		// Quadrant IV
		fOffsetX = 0.75f;
		fOffsetY = 0.75f;

		mTexShadowOffset1[3] = D3DXMATRIX(1.0f,     0.0f,      0.0f, 0.0f,
										0.0f,     1.0f,      0.0f, 0.0f,
										0.0f,     0.0f,      1.0f, 0.0f,
										fOffsetX, fOffsetY,  0.0f, 1.0f);

		// Offset by half a texel to make PCF filtering pertier
		fOffsetX = (0.5f / vid_shadowmapSize);
		fOffsetY = (0.5f / vid_shadowmapSize);

		D3DXMATRIX mTexShadowOffset2(1.0f,     0.0f,      0.0f, 0.0f,
									0.0f,     1.0f,      0.0f, 0.0f,
									0.0f,     0.0f,      1.0f, 0.0f,
									fOffsetX, fOffsetY,  0.0f, 1.0f);

		D3DXMATRIXA16 mLightWorldViewProjSplit[4] =
		{
			g_mWorld * g_mLightViewProjSplit[0] * mTexShadowScale * mTexShadowOffset1[0] * mTexShadowOffset2,
			g_mWorld * g_mLightViewProjSplit[1] * mTexShadowScale * mTexShadowOffset1[1] * mTexShadowOffset2,
			g_mWorld * g_mLightViewProjSplit[2] * mTexShadowScale * mTexShadowOffset1[2] * mTexShadowOffset2,
			g_mWorld * g_mLightViewProjSplit[3] * mTexShadowScale * mTexShadowOffset1[3] * mTexShadowOffset2
		};

		D3D_SetMatrixArray(pRegisters, uiSize, mLightWorldViewProjSplit, 4);
		return true;
	}
	else
	{
		float fOffsetX, fOffsetY;

		D3DXMATRIX mTexShadowScale(0.5f,  0.0f, 0.0f, 0.0f,
								0.0f, -0.5f, 0.0f, 0.0f,
								0.0f,  0.0f, 1.0f, 0.0f,
								0.0f,  0.0f, 0.0f, 1.0f);

		fOffsetX = 0.5f;
		fOffsetY = 0.5f;

		D3DXMATRIX mTexShadowOffset1(1.0f,     0.0f,      0.0f, 0.0f,
									0.0f,     1.0f,      0.0f, 0.0f,
									0.0f,     0.0f,      1.0f, 0.0f,
									fOffsetX, fOffsetY,  0.0f, 1.0f);

		fOffsetX = (0.5f / vid_shadowmapSize);
		fOffsetY = (0.5f / vid_shadowmapSize);

		D3DXMATRIX mTexShadowOffset2(1.0f,     0.0f,      0.0f, 0.0f,
									0.0f,     1.0f,      0.0f, 0.0f,
									0.0f,     0.0f,      1.0f, 0.0f,
									fOffsetX, fOffsetY,  0.0f, 1.0f);

		D3DXMATRIXA16 mLightWorldViewProj = g_mWorld * g_mLightViewProj * mTexShadowScale * mTexShadowOffset1 * mTexShadowOffset2;

		D3DXMATRIXA16 mLightWorldViewProjSplit[4] =
		{
			mLightWorldViewProj,
			mLightWorldViewProj,
			mLightWorldViewProj,
			mLightWorldViewProj
		};

		D3D_SetMatrixArray(pRegisters, uiSize, mLightWorldViewProjSplit, 4);
		return true;
	}
}
#endif


/*--------------------
  fShadowmapSize
  --------------------*/
SHADER_VAR(fShadowmapSize)
{
	D3D_SetFloat(pRegisters, static_cast<float>(vid_shadowmapSize));
	return true;
}


/*--------------------
  fShadowmapSizeInv
  --------------------*/
SHADER_VAR(fShadowmapSizeInv)
{
	D3D_SetFloat(pRegisters, 1.0f/vid_shadowmapSize);
	return true;
}


/*--------------------
  fTranslucent
  --------------------*/
SHADER_VAR(fTranslucent)
{
	D3D_SetFloat(pRegisters, g_bAlpha ? 1.0f : 0.0f);
	return true;
}


/*--------------------
  fFogDensity
  --------------------*/
SHADER_VAR(fFogDensity)
{
	D3D_SetFloat(pRegisters, gfx_fogDensity);
	return true;
}


/*--------------------
  fFogStart
  --------------------*/
SHADER_VAR(fFogStart)
{
	D3D_SetFloat(pRegisters, gfx_fogNear);
	return true;
}


/*--------------------
  fFogEnd
  --------------------*/
SHADER_VAR(fFogEnd)
{
	D3D_SetFloat(pRegisters, gfx_fogFar);
	return true;
}


/*--------------------
  fFogDelta
  --------------------*/
SHADER_VAR(fFogDelta)
{
	D3D_SetFloat(pRegisters, max(gfx_fogFar - gfx_fogNear, 0.0f));
	return true;
}



/*--------------------
  fFogScale
  --------------------*/
SHADER_VAR(fFogScale)
{
	D3D_SetFloat(pRegisters, CLAMP<float>(gfx_fogScale, 0.0f, 1.0f));
	return true;
}


/*--------------------
  vFogColor
  --------------------*/
SHADER_VAR(vFogColor)
{
	D3DXVECTOR4 vFogColor(gfx_fogColor[R], gfx_fogColor[G], gfx_fogColor[B], 1.0f);

	D3D_SetVector(pRegisters, &vFogColor);
	return true;
}


/*--------------------
  vFog
  --------------------*/
SHADER_VAR(vFog)
{
	D3DXVECTOR4 vFog
	(
		1.0f / (MAX<float>(gfx_fogFar, 0.0f) - MAX<float>(gfx_fogNear, 0.0f)),
		-MAX<float>(gfx_fogNear, 0.0f) / (MAX<float>(gfx_fogFar, 0.0f) - MAX<float>(gfx_fogNear, 0.0f)),
		CLAMP<float>(gfx_fogScale, 0.0f, 1.0f),
		0.0f
	);

	D3D_SetVector(pRegisters, &vFog);
	return true;
}


/*--------------------
  fShadowLeak
  --------------------*/
SHADER_VAR(fShadowLeak)
{
	D3D_SetFloat(pRegisters, vid_shadowLeak);
	return true;
}


/*--------------------
  fOneMinusShadowLeak
  --------------------*/
SHADER_VAR(fOneMinusShadowLeak)
{
	D3D_SetFloat(pRegisters, 1.0f - vid_shadowLeak);
	return true;
}


/*--------------------
  vShadowLeak
  --------------------*/
SHADER_VAR(vShadowLeak)
{
	D3DXVECTOR4 vShadowLeak(1.0f - vid_shadowLeak, vid_shadowLeak, 0.0f, 0.0f);

	D3D_SetVector(pRegisters, &vShadowLeak);
	return true;
}


/*--------------------
  fShadowDepthBias
  --------------------*/
SHADER_VAR(fShadowDepthBias)
{
	D3D_SetFloat(pRegisters, vid_shadowDepthBias / 16777215.f);
	return true;
}


/*--------------------
  fShadowSlopeBias
  --------------------*/
SHADER_VAR(fShadowSlopeBias)
{
	D3D_SetFloat(pRegisters, vid_shadowSlopeBias + vid_shadowmapFilterWidth);
	return true;
}



/*--------------------
  vPointLightPosition
  --------------------*/
SHADER_VAR(vPointLightPosition)
{
	D3DXVECTOR4 vPosition[MAX_POINT_LIGHTS];

	for (int i = 0; i < g_iNumActivePointLights; ++i)
		vPosition[i] = D3DXVECTOR4(g_vPointLightPosition[i].x, g_vPointLightPosition[i].y, g_vPointLightPosition[i].z, 1.0f);

	D3D_SetVectorArray(pRegisters, vPosition, g_iNumActivePointLights);
	return true;
}


/*--------------------
  vPointLightPositionView
  --------------------*/
SHADER_VAR(vPointLightPositionView)
{
	D3DXVECTOR4 vPosition[MAX_POINT_LIGHTS];

	for (int i = 0; i < g_iNumActivePointLights; ++i)
	{
		vPosition[i] = D3DXVECTOR4(g_vPointLightPosition[i].x, g_vPointLightPosition[i].y, g_vPointLightPosition[i].z, 1.0f);

		D3DXVec4Transform(&vPosition[i], &vPosition[i], &g_mView);
	}

	D3D_SetVectorArray(pRegisters, vPosition, g_iNumActivePointLights);
	return true;
}


/*--------------------
  vPointLightPositionOffset
  --------------------*/
SHADER_VAR(vPointLightPositionOffset)
{
	D3DXVECTOR4 vPosition[MAX_POINT_LIGHTS];

	for (int i = 0; i < g_iNumActivePointLights; ++i)
	{
		vPosition[i] = D3DXVECTOR4(g_vPointLightPosition[i].x, g_vPointLightPosition[i].y, g_vPointLightPosition[i].z, 1.0f);

		D3DXVec4Transform(&vPosition[i], &vPosition[i], &g_mViewOffset);
	}

	D3D_SetVectorArray(pRegisters, vPosition, g_iNumActivePointLights);
	return true;
}


/*--------------------
  vPointLightColor
  --------------------*/
SHADER_VAR(vPointLightColor)
{
	D3DXVECTOR4 vColor[MAX_POINT_LIGHTS];

	for (int i = 0; i < g_iNumActivePointLights; ++i)
		vColor[i] = D3DXVECTOR4(g_vPointLightColor[i].x, g_vPointLightColor[i].y, g_vPointLightColor[i].z, 1.0f);

	D3D_SetVectorArray(pRegisters, vColor, g_iNumActivePointLights);
	return true;
}


/*--------------------
  vPointLightColorSpec
  --------------------*/
SHADER_VAR(vPointLightColorSpec)
{
	D3DXVECTOR4 vColor[MAX_POINT_LIGHTS];

	for (int i = 0; i < g_iNumActivePointLights; ++i)
		vColor[i] = D3DXVECTOR4
		(
			g_vPointLightColor[i].x * g_pCurrentMaterial->GetSpecularLevel(),
			g_vPointLightColor[i].y * g_pCurrentMaterial->GetSpecularLevel(),
			g_vPointLightColor[i].z * g_pCurrentMaterial->GetSpecularLevel(),
			1.0f
		);

	D3D_SetVectorArray(pRegisters, vColor, g_iNumActivePointLights);
	return true;
}


/*--------------------
  fPointLightFalloffStart
  --------------------*/
SHADER_VAR(fPointLightFalloffStart)
{
	D3D_SetFloatArray(pRegisters, g_fPointLightFalloffStart, g_iNumActivePointLights);
	return true;
}


/*--------------------
  fPointLightFalloffEnd
  --------------------*/
SHADER_VAR(fPointLightFalloffEnd)
{
	D3D_SetFloatArray(pRegisters, g_fPointLightFalloffEnd, g_iNumActivePointLights);
	return true;
}


/*--------------------
  fPointLightFalloff
  --------------------*/
SHADER_VAR(fPointLightFalloff)
{
	D3DXVECTOR4 vFalloff[MAX_POINT_LIGHTS];

	for (int i = 0; i < g_iNumActivePointLights; ++i)
		vFalloff[i] = D3DXVECTOR4(g_fPointLightFalloffStart[i], g_fPointLightFalloffEnd[i] - g_fPointLightFalloffStart[i], 0.0f, 0.0f);

	D3D_SetVectorArray(pRegisters, vFalloff, g_iNumActivePointLights);
	return true;
}


/*--------------------
  vPointLightFalloff
  --------------------*/
SHADER_VAR(vPointLightFalloff)
{
	D3DXVECTOR4 vFalloff[MAX_POINT_LIGHTS];

	for (int i = 0; i < g_iNumActivePointLights; ++i)
		vFalloff[i] = D3DXVECTOR4(1.0f / (MAX<float>(g_fPointLightFalloffEnd[i], 0.0f) - MAX<float>(g_fPointLightFalloffStart[i], 0.0f)),
			-MAX<float>(g_fPointLightFalloffStart[i], 0.0f) / (MAX<float>(g_fPointLightFalloffEnd[i], 0.0f) - MAX<float>(g_fPointLightFalloffStart[i], 0.0f)),
			g_fPointLightFalloffEnd[i] * g_fPointLightFalloffEnd[i], 0.0f);

	D3D_SetVectorArray(pRegisters, vFalloff, g_iNumActivePointLights);
	return true;

}


/*--------------------
  vLeafClusters
  --------------------*/
SHADER_VAR(vLeafClusters)
{
	D3D_SetVectorArray(pRegisters, (D3DXVECTOR4*)&g_afLeafClusterData, MAX_LEAF_CLUSTER_INDEX);
	return true;
}


/*--------------------
  fFoliageFalloffStart
  --------------------*/
SHADER_VAR(fFoliageFalloffStart)
{
	D3D_SetFloat(pRegisters, MAX(SceneManager.GetFoliageDrawDistance() - vid_foliageFalloffDistance, 0.0f));
	return true;
}


/*--------------------
  fFoliageFalloffStart
  --------------------*/
SHADER_VAR(fFoliageFalloffEnd)
{
	D3D_SetFloat(pRegisters, MAX(SceneManager.GetFoliageDrawDistance(), 0.0f));
	return true;
}


/*--------------------
  fFoliageFalloffLength
  --------------------*/
SHADER_VAR(fFoliageFalloffLength)
{
	D3D_SetFloat(pRegisters, MAX(SceneManager.GetFoliageDrawDistance(), 0.0f) - MAX(SceneManager.GetFoliageDrawDistance() - vid_foliageFalloffDistance, 0.0f));
	return true;
}


/*--------------------
  fFoliageAnimateSpeed
  --------------------*/
SHADER_VAR(fFoliageAnimateSpeed)
{
	D3D_SetFloat(pRegisters, vid_foliageAnimateSpeed);
	return true;
}


/*--------------------
  fFoliageAnimateSpeed
  --------------------*/
SHADER_VAR(fFoliageAnimateStrength)
{
	D3D_SetFloat(pRegisters, vid_foliageAnimateStrength);
	return true;
}


/*--------------------
  vFoliage
  --------------------*/
SHADER_VAR(vFoliage)
{
	D3DXVECTOR4 vFoliage(1.0f / (MAX(SceneManager.GetFoliageDrawDistance(), 0.0f) - MAX(SceneManager.GetFoliageDrawDistance() - vid_foliageFalloffDistance, 0.0f)),
		-MAX(SceneManager.GetFoliageDrawDistance() - vid_foliageFalloffDistance, 0.0f) / (MAX(SceneManager.GetFoliageDrawDistance(), 0.0f) - MAX(SceneManager.GetFoliageDrawDistance() - vid_foliageFalloffDistance, 0.0f)),
		vid_foliageAnimateSpeed,
		vid_foliageAnimateStrength);

	D3D_SetVector(pRegisters, &vFoliage);
	return true;
}



/*--------------------
  fAlphaTest
  --------------------*/
SHADER_VAR(fAlphaTest)
{
	D3D_SetFloat(pRegisters, (D3D_GetRenderState(D3DRS_ALPHAREF) + 1) / 255.0f);
	return true;
}


/*--------------------
  vBones
  --------------------*/
SHADER_VAR(vBones)
{
	D3D_SetMatrixArray(pRegisters, uiSize, g_vBoneData, g_iNumActiveBones);
	return true;
}


/*--------------------
  mCloudProj
  --------------------*/
SHADER_VAR(mCloudProj)
{
	D3D_SetMatrix(pRegisters, uiSize, &(g_mWorld * g_mCloudProj));
	return true;
}


/*--------------------
  mFowProj
  --------------------*/
SHADER_VAR(mFowProj)
{
	D3D_SetMatrix(pRegisters, uiSize, &(g_mWorld * g_mFowProj));
	return true;
}



/*--------------------
  fShadowFalloffStart
  --------------------*/
SHADER_VAR(fShadowFalloffStart)
{
	if (g_pCam && g_pCam->HasFlags(CAM_SHADOW_NO_FALLOFF))
		D3D_SetFloat(pRegisters, g_pCam->GetZFar());
	else
		D3D_SetFloat(pRegisters, MAX(vid_shadowDrawDistance - vid_shadowFalloffDistance, 0.0f));
	return true;
}


/*--------------------
  fShadowFalloffStart
  --------------------*/
SHADER_VAR(fShadowFalloffEnd)
{
	if (g_pCam && g_pCam->HasFlags(CAM_SHADOW_NO_FALLOFF))
		D3D_SetFloat(pRegisters, g_pCam->GetZFar() + 1.0f);
	else
		D3D_SetFloat(pRegisters, MAX(float(vid_shadowDrawDistance), 0.0f));
	return true;
}


/*--------------------
  fShadowFalloffLength
  --------------------*/
SHADER_VAR(fShadowFalloffLength)
{
	if (g_pCam && g_pCam->HasFlags(CAM_SHADOW_NO_FALLOFF))
		D3D_SetFloat(pRegisters, 1.0f);
	else
		D3D_SetFloat(pRegisters, MAX(float(vid_shadowDrawDistance), 0.0f) - MAX(vid_shadowDrawDistance - vid_shadowFalloffDistance, 0.0f));
	return true;
}


/*--------------------
  vShadowFalloff
  --------------------*/
SHADER_VAR(vShadowFalloff)
{
	if (g_pCam && g_pCam->HasFlags(CAM_SHADOW_NO_FALLOFF))
	{
		D3DXVECTOR4 vShadowFalloff(1.0f, -g_pCam->GetZFar(), 0.0f, 0.0f);
		D3D_SetVector(pRegisters, &vShadowFalloff);
	}
	else
	{
		D3DXVECTOR4 vShadowFalloff(1.0f / (MAX(float(vid_shadowDrawDistance), 0.0f) - MAX(vid_shadowDrawDistance - vid_shadowFalloffDistance, 0.0f)),
			-MAX(vid_shadowDrawDistance - vid_shadowFalloffDistance, 0.0f) / (MAX(float(vid_shadowDrawDistance), 0.0f) - MAX(vid_shadowDrawDistance - vid_shadowFalloffDistance, 0.0f)),
			0.0f, 0.0f);

		D3D_SetVector(pRegisters, &vShadowFalloff);
	}

	return true;
}


/*--------------------
  fWorldWidth
  --------------------*/
SHADER_VAR(fWorldWidth)
{
	D3D_SetFloat(pRegisters, terrain.pWorld ? terrain.pWorld->GetWorldWidth() : 1.0f);
	return true;
}


/*--------------------
  fWorldHeight
  --------------------*/
SHADER_VAR(fWorldHeight)
{
	D3D_SetFloat(pRegisters, terrain.pWorld ? terrain.pWorld->GetWorldHeight() : 1.0f);
	return true;
}


/*--------------------
  fWorldTileSize
  --------------------*/
SHADER_VAR(fWorldTileSize)
{
	D3D_SetFloat(pRegisters, terrain.pWorld ? terrain.pWorld->GetScale() : 1.0f);
	return true;
}


/*--------------------
  fWorldTextureScale
  --------------------*/
SHADER_VAR(fWorldTextureScale)
{
	D3D_SetFloat(pRegisters, terrain.pWorld ? terrain.pWorld->GetTextureScale() : 1.0f);
	return true;
}


/*--------------------
  fWorldTexelDensity
  --------------------*/
SHADER_VAR(fWorldTexelDensity)
{
	D3D_SetFloat(pRegisters, terrain.pWorld ? terrain.pWorld->GetTexelDensity() : 1.0f);
	return true;
}


/*--------------------
  fWorldTextureInc
  --------------------*/
SHADER_VAR(fWorldTextureInc)
{
	D3D_SetFloat(pRegisters, terrain.pWorld ? 1.0f / terrain.pWorld->GetTextureScale() : 1.0f);
	return true;
}


/*--------------------
  fWorldTexelInc
  --------------------*/
SHADER_VAR(fWorldTexelInc)
{
	D3D_SetFloat(pRegisters, terrain.pWorld ? 1.0f / terrain.iChunkSize : 1.0f);
	return true;
}


/*--------------------
  fWorldTextureProjection
  --------------------*/
SHADER_VAR(fWorldTextureProjection)
{
	D3D_SetFloat(pRegisters, terrain.pWorld ? 1.0f / (terrain.pWorld->GetScale() * terrain.pWorld->GetTextureScale()) : 1.0f / (64.0f * 4.0f));
	return true;
}



/*--------------------
  vWorldSizes

  x = fWorldTileSize
  y = fWorldTextureInc
  z = fWorldTexelInc
  --------------------*/
SHADER_VAR(vWorldSizes)
{
	D3DXVECTOR4 vWorldSizes
	(
		terrain.pWorld ? terrain.pWorld->GetScale() : 1.0f,
		terrain.pWorld ? 1.0f / terrain.pWorld->GetTextureScale() : 1.0f,
		terrain.pWorld ? 1.0f / terrain.iChunkSize : 1.0f,
		0.0f
	);

	D3D_SetVector(pRegisters, &vWorldSizes);
	return true;
}


/*--------------------
  fSkyEpsilon
  --------------------*/
SHADER_VAR(fSkyEpsilon)
{
	D3D_SetFloat(pRegisters, vid_skyEpsilon);
	return true;
}


/*--------------------
  fHalfTexelSize
  --------------------*/
SHADER_VAR(fHalfTexelSize)
{
	D3D_SetFloat(pRegisters, 0.5f / g_uiImageWidth);
	return true;
}


/*--------------------
  fTexelSize
  --------------------*/
SHADER_VAR(fTexelSize)
{
	D3D_SetFloat(pRegisters, 1.0f / g_uiImageWidth);
	return true;
}


/*--------------------
  vHalfTexelSize
  --------------------*/
SHADER_VAR(vHalfTexelSize)
{
	D3DXVECTOR4 vTexelSizes
	(
		0.5f / g_uiImageWidth,
		0.5f / g_uiImageHeight,
		float(g_Viewport.Width) / g_CurrentVidMode.iWidth,
		float(g_Viewport.Height) / g_CurrentVidMode.iHeight
	);

	D3D_SetVector(pRegisters, &vTexelSizes);
	return true;
}


/*--------------------
  vTexelSize
  --------------------*/
SHADER_VAR(vTexelSize)
{
	D3DXVECTOR4 vTexelSizes
	(
		1.0f / g_uiImageWidth,
		1.0f / g_uiImageHeight,
		1.0f,
		1.0f
	);

	D3D_SetVector(pRegisters, &vTexelSizes);
	return true;
}


/*--------------------
  vPostExtents
  --------------------*/
SHADER_VAR(vPostExtents)
{
	D3DXVECTOR4 vPostExtents
	(
		float(g_Viewport.Width - 1) / g_CurrentVidMode.iWidth,
		float(g_Viewport.Height - 1) / g_CurrentVidMode.iHeight,
		0.0f,
		0.0f
	);

	D3D_SetVector(pRegisters, &vPostExtents);
	return true;
}


/*--------------------
  vGaussian13TapsX
  --------------------*/
SHADER_VAR(vGaussian13TapsX)
{
	float fTexelSize(1.0f / g_uiImageWidth);

	D3DXVECTOR4 avTaps[13] =
	{
		D3DXVECTOR4(-11.5f * fTexelSize, 0.0f, 0.0f, 0.0f),
		D3DXVECTOR4(-9.5f * fTexelSize, 0.0f, 0.0f, 0.0f),
		D3DXVECTOR4(-7.5f * fTexelSize, 0.0f, 0.0f, 0.0f),
		D3DXVECTOR4(-5.5f * fTexelSize, 0.0f, 0.0f, 0.0f),
		D3DXVECTOR4(-3.5f * fTexelSize, 0.0f, 0.0f, 0.0f),
		D3DXVECTOR4(-1.5f * fTexelSize, 0.0f, 0.0f, 0.0f),
		D3DXVECTOR4(0.0f * fTexelSize, 0.0f, 0.0f, 0.0f),
		D3DXVECTOR4(1.5f * fTexelSize, 0.0f, 0.0f, 0.0f),
		D3DXVECTOR4(3.5f * fTexelSize, 0.0f, 0.0f, 0.0f),
		D3DXVECTOR4(5.5f * fTexelSize, 0.0f, 0.0f, 0.0f),
		D3DXVECTOR4(7.5f * fTexelSize, 0.0f, 0.0f, 0.0f),
		D3DXVECTOR4(9.5f * fTexelSize, 0.0f, 0.0f, 0.0f),
		D3DXVECTOR4(11.5f * fTexelSize, 0.0f, 0.0f, 0.0f)
	};

	D3D_SetVectorArray(pRegisters, avTaps, 13);
	return true;
}


/*--------------------
  vGaussian13TapsY
  --------------------*/
SHADER_VAR(vGaussian13TapsY)
{
	float fTexelSize(1.0f / g_uiImageHeight);

	D3DXVECTOR4 avTaps[13] =
	{
		D3DXVECTOR4(0.0f, -11.5f * fTexelSize, 0.0f, 0.0f),
		D3DXVECTOR4(0.0f, -9.5f * fTexelSize, 0.0f, 0.0f),
		D3DXVECTOR4(0.0f, -7.5f * fTexelSize, 0.0f, 0.0f),
		D3DXVECTOR4(0.0f, -5.5f * fTexelSize, 0.0f, 0.0f),
		D3DXVECTOR4(0.0f, -3.5f * fTexelSize, 0.0f, 0.0f),
		D3DXVECTOR4(0.0f, -1.5f * fTexelSize, 0.0f, 0.0f),
		D3DXVECTOR4(0.0f, 0.0f * fTexelSize, 0.0f, 0.0f),
		D3DXVECTOR4(0.0f, 1.5f * fTexelSize, 0.0f, 0.0f),
		D3DXVECTOR4(0.0f, 3.5f * fTexelSize, 0.0f, 0.0f),
		D3DXVECTOR4(0.0f, 5.5f * fTexelSize, 0.0f, 0.0f),
		D3DXVECTOR4(0.0f, 7.5f * fTexelSize, 0.0f, 0.0f),
		D3DXVECTOR4(0.0f, 9.5f * fTexelSize, 0.0f, 0.0f),
		D3DXVECTOR4(0.0f, 11.5f * fTexelSize, 0.0f, 0.0f)
	};

	D3D_SetVectorArray(pRegisters, avTaps, 13);
	return true;
}


/*--------------------
  mSceneProj
  --------------------*/
SHADER_VAR(mSceneProj)
{
	float fOffsetX;
	float fOffsetY;

	float fScaleX;
	float fScaleY;

	fScaleX = 0.5f;
	fScaleY = -0.5f;

	D3DXMATRIXA16 mTexScale1(fScaleX, 0.0f,    0.0f, 0.0f,
	                         0.0f,    fScaleY, 0.0f, 0.0f,
	                         0.0f,    0.0f,    1.0f, 0.0f,
	                         0.0f,    0.0f,    0.0f, 1.0f);

	fOffsetX = 0.5f;
	fOffsetY = 0.5f;

	D3DXMATRIXA16 mTexOffset1(1.0f,     0.0f,     0.0f, 0.0f,
	                          0.0f,     1.0f,     0.0f, 0.0f,
	                          0.0f,     0.0f,     1.0f, 0.0f,
	                          fOffsetX, fOffsetY, 0.0f, 1.0f);

	fOffsetX = (0.5f / g_uiImageWidth);
	fOffsetY = (0.5f / g_uiImageHeight);

	D3DXMATRIXA16 mTexOffset2(1.0f,     0.0f,     0.0f, 0.0f,
	                          0.0f,     1.0f,     0.0f, 0.0f,
	                          0.0f,     0.0f,     1.0f, 0.0f,
	                          fOffsetX, fOffsetY, 0.0f, 1.0f);
	
	D3DXMATRIXA16 mScreenProj = mTexScale1 * mTexOffset1 * mTexOffset2;

	D3D_SetMatrix(pRegisters, uiSize, &mScreenProj);
	return true;
}


/*--------------------
  vScene
  --------------------*/
SHADER_VAR(vScene)
{
	float fCamFovX(g_pCam->GetFovX());
	float fCamFovY(g_pCam->GetFovY());
	float fCamAspect(g_pCam->GetAspect());

	float A = tan(DEG2RAD(fCamFovX * 0.5f)) * tan(DEG2RAD(fCamFovY * 0.5f)) * 4.0f;
	float S = sqrt(A);
	float Y = sqrt(4.0f * fCamAspect * A) / (2.0f * fCamAspect);
	float X = A / Y;

	D3DXVECTOR4 vScene
	(
		S / X,
		S / Y,
		0.0f,
		0.0f
	);

	D3D_SetVector(pRegisters, &vScene);
	return true;
}


/*--------------------
  vBright
  --------------------*/
SHADER_VAR(vBright)
{
	D3DXVECTOR4 vBright
	(
		scene_brightMin,
		scene_brightMax,
		scene_brightScale,
		0.0f
	);

	D3D_SetVector(pRegisters, &vBright);
	return true;
}


/*--------------------
  vLinearBright
  --------------------*/
SHADER_VAR(vLinearBright)
{
	D3DXVECTOR4 vBright
	(
		1.0f / (MAX<float>(scene_brightMax, 0.0f) - MAX<float>(scene_brightMin, 0.0f)),
		-MAX<float>(scene_brightMin, 0.0f) / (MAX<float>(scene_brightMax, 0.0f) - MAX<float>(scene_brightMin, 0.0f)),
		scene_brightScale,
		0.0f
	);

	D3D_SetVector(pRegisters, &vBright);
	return true;
}


/*--------------------
  vParam
  --------------------*/
SHADER_VAR(vParam)
{
	if (g_pCurrentEntity != NULL)
	{
		D3DXVECTOR4 vParam
		(
			g_pCurrentEntity->s1,
			g_pCurrentEntity->t1,
			g_pCurrentEntity->s2,
			g_pCurrentEntity->t2
		);

		D3D_SetVector(pRegisters, &vParam);
	}
	else
	{
		D3DXVECTOR4 vParam(0.0f, 0.0f, 0.0f, 0.0f);
		D3D_SetVector(pRegisters, &vParam);
	}

	return true;
}


/*--------------------
  fDistance
  --------------------*/
SHADER_VAR(fDistance)
{
	if (g_pCurrentEntity != NULL)
		D3D_SetFloat(pRegisters, Distance(g_pCurrentEntity->GetPosition(), g_pCam->GetOrigin()));
	else
		D3D_SetFloat(pRegisters, 0.0f);

	return true;
}
