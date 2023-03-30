// (C)2008 S2 Games
// gl2_shadervars.cpp
//
// Shader Variables
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_gfxmaterials.h"
#include "c_gfxmodels.h"
#include "c_gfxterrain.h"
#include "c_shadowmap.h"
#include "gl2_foliage.h"
#include "c_shadervar.h"
#include "c_treemodeldef.h"

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
//=============================================================================

/*--------------------
  mWorldViewProj
  --------------------*/
SHADER_VAR(mWorldViewProj)
{
    glUniformMatrix4fvARB(iLocation, 1, GL_FALSE, (GLfloat *)&g_mWorldViewProj);
    return true;
}

/*--------------------
  mView
  --------------------*/
SHADER_VAR(mView)
{
    glUniformMatrix4fvARB(iLocation, 1, GL_FALSE, (GLfloat *)&g_mView);
    return true;
}

/*--------------------
  mProj
  --------------------*/
SHADER_VAR(mProj)
{
    glUniformMatrix4fvARB(iLocation, 1, GL_FALSE, (GLfloat *)&g_mProj);
    return true;
}

/*--------------------
  mWorld
  --------------------*/
SHADER_VAR(mWorld)
{
    glUniformMatrix4fvARB(iLocation, 1, GL_FALSE, (GLfloat *)&g_mWorld);
    return true;
}


/*--------------------
  mWorldRotate
  --------------------*/
SHADER_VAR(mWorldRotate)
{
    float mat3[9] = 
    {
        g_mWorldRotate[0], g_mWorldRotate[1], g_mWorldRotate[2],
        g_mWorldRotate[4], g_mWorldRotate[5], g_mWorldRotate[6],
        g_mWorldRotate[8], g_mWorldRotate[9], g_mWorldRotate[10]
    };

    glUniformMatrix3fvARB(iLocation, 1, GL_FALSE, (GLfloat *)&mat3);
    return true;
}


/*--------------------
  mWorldViewRotate
  --------------------*/
SHADER_VAR(mWorldViewRotate)
{
    D3DXMATRIXA16 mWorldViewRotate = g_mWorldRotate * g_mViewRotate;

    float mat3[9] = 
    {
        mWorldViewRotate[0], mWorldViewRotate[1], mWorldViewRotate[2],
        mWorldViewRotate[4], mWorldViewRotate[5], mWorldViewRotate[6],
        mWorldViewRotate[8], mWorldViewRotate[9], mWorldViewRotate[10]
    };

    glUniformMatrix3fvARB(iLocation, 1, GL_FALSE, (GLfloat *)&mat3);
    return true;
}


/*--------------------
  mWorldViewOffset
  --------------------*/
SHADER_VAR(mWorldViewOffset)
{
    D3DXMATRIXA16 mWorldViewOffset = g_mWorld * g_mViewOffset;

    glUniformMatrix4fvARB(iLocation, 1, GL_FALSE, (GLfloat *)&mWorldViewOffset);
    return true;
}


/*--------------------
  fTime
  --------------------*/
SHADER_VAR(fTime)
{
    glUniform1fARB(iLocation, (g_pCam != NULL) ? (g_pCam->GetTime()) : MsToSec(Host.GetSystemTime()));
    return true;
}


/*--------------------
  vColor
  --------------------*/
SHADER_VAR(vColor)
{
    CVec4f vDiffuseColor
    (
        g_pCurrentMaterial->GetDiffuseColor().x,
        g_pCurrentMaterial->GetDiffuseColor().y,
        g_pCurrentMaterial->GetDiffuseColor().z,
        g_pCurrentMaterial->GetOpacity()
    );

    CVec4f  vObjectColor;

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

    CVec4f vColor(vObjectColor * vDiffuseColor);

    glUniform4fvARB(iLocation, 1, (GLfloat *)&vColor);
    return true;
}


/*--------------------
  vTeamColor
  --------------------*/
SHADER_VAR(vTeamColor)
{
    CVec4f  vTeamColor;

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

    glUniform3fvARB(iLocation, 1, (GLfloat *)&vTeamColor);
    return true;
}


/*--------------------
  vSunPositionView
  --------------------*/
SHADER_VAR(vSunPositionView)
{
    CVec3f v3SunPos(SceneManager.GetSunPos());

    D3DXVec3TransformNormal((D3DXVECTOR3 *)&v3SunPos, (D3DXVECTOR3 *)&v3SunPos, &g_mViewRotate);

    glUniform3fvARB(iLocation, 1, (GLfloat *)&v3SunPos);

    return true;
}


/*--------------------
  vSunPositionWorld
  --------------------*/
SHADER_VAR(vSunPositionWorld)
{
    CVec3f v3SunPos(SceneManager.GetSunPos());

    glUniform3fvARB(iLocation, 1, (GLfloat *)&v3SunPos);
    return true;
}


/*--------------------
  vSunColor
  --------------------*/
SHADER_VAR(vSunColor)
{
    glUniform3fvARB(iLocation, 1, (GLfloat *)&g_v3SunColor);
    return true;
}


/*--------------------
  vSunColorSpec
  --------------------*/
SHADER_VAR(vSunColorSpec)
{
    CVec3f v3SunColorSpec(g_v3SunColor * g_pCurrentMaterial->GetSpecularLevel());

    glUniform3fvARB(iLocation, 1, (GLfloat *)&v3SunColorSpec);
    return true;
}


/*--------------------
  vAmbient
  --------------------*/
SHADER_VAR(vAmbient)
{
    glUniform3fvARB(iLocation, 1, (GLfloat *)&g_v3Ambient);
    return true;
}


/*--------------------
  vGroundAmbient
  --------------------*/
SHADER_VAR(vGroundAmbient)
{
    CVec3f v3GroundAmbient(g_v3Ambient * 0.25f);

    glUniform3fvARB(iLocation, 1, (GLfloat *)&v3GroundAmbient);
    return true;
}


/*--------------------
  vBones
  --------------------*/
SHADER_VAR(vBones)
{
    glUniformMatrix3x4fv(iLocation, g_iNumActiveBones, GL_FALSE, (GLfloat *)&g_vBoneData);
    return true;
}


/*--------------------
  vBones3
  --------------------*/
SHADER_VAR(vBones3)
{
    glUniform4fvARB(iLocation, g_iNumActiveBones * 3, (GLfloat *)&g_vBoneData);
    return true;
}


/*--------------------
  fWorldWidth
  --------------------*/
SHADER_VAR(fWorldWidth)
{
    glUniform1fARB(iLocation, GfxTerrain->pWorld ? GfxTerrain->pWorld->GetWorldWidth() : 1.0f);
    return true;
}


/*--------------------
  fWorldHeight
  --------------------*/
SHADER_VAR(fWorldHeight)
{
    glUniform1fARB(iLocation, GfxTerrain->pWorld ? GfxTerrain->pWorld->GetWorldHeight() : 1.0f);
    return true;
}


/*--------------------
  fWorldTileSize
  --------------------*/
SHADER_VAR(fWorldTileSize)
{
    glUniform1fARB(iLocation, GfxTerrain->pWorld ? GfxTerrain->pWorld->GetScale() : 1.0f);
    return true;
}


/*--------------------
  fWorldTextureScale
  --------------------*/
SHADER_VAR(fWorldTextureScale)
{
    glUniform1fARB(iLocation, GfxTerrain->pWorld ? GfxTerrain->pWorld->GetTextureScale() : 1.0f);
    return true;
}


/*--------------------
  fWorldTexelDensity
  --------------------*/
SHADER_VAR(fWorldTexelDensity)
{
    glUniform1fARB(iLocation, GfxTerrain->pWorld ? GfxTerrain->pWorld->GetTexelDensity() : 1.0f);
    return true;
}


/*--------------------
  fWorldTextureInc
  --------------------*/
SHADER_VAR(fWorldTextureInc)
{
    glUniform1fARB(iLocation, GfxTerrain->pWorld ? 1.0f / GfxTerrain->pWorld->GetTextureScale() : 1.0f);
    return true;
}


/*--------------------
  fWorldTexelInc
  --------------------*/
SHADER_VAR(fWorldTexelInc)
{
    glUniform1fARB(iLocation, GfxTerrain->pWorld ? 1.0f / GfxTerrain->iChunkSize : 1.0f);
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
    glUniform3fARB(iLocation,
        GfxTerrain->pWorld ? GfxTerrain->pWorld->GetScale() : 1.0f,
        GfxTerrain->pWorld ? 1.0f / GfxTerrain->pWorld->GetTextureScale() : 1.0f,
        GfxTerrain->pWorld ? 1.0f / GfxTerrain->iChunkSize : 1.0f);
    return true;
}


/*--------------------
  mLightWorldViewProjTex
  --------------------*/
SHADER_VAR(mLightWorldViewProjTex)
{
    D3DXMATRIXA16 mLightWorldViewProj = g_mWorld * g_mLightViewProjTex;

    glUniformMatrix4fvARB(iLocation, 1, GL_FALSE, (GLfloat *)&mLightWorldViewProj);
    return true;
}


/*--------------------
  fReflect
  --------------------*/
SHADER_VAR(fReflect)
{
    glUniform1fARB(iLocation, g_pCurrentMaterial->GetReflect());
    return true;
}


/*--------------------
  fGlossiness
  --------------------*/
SHADER_VAR(fGlossiness)
{
    glUniform1fARB(iLocation, g_pCurrentMaterial->GetGlossiness());
    return true;
}


/*--------------------
  fSpecularLevel
  --------------------*/
SHADER_VAR(fSpecularLevel)
{
    glUniform1fARB(iLocation, g_pCurrentMaterial->GetSpecularLevel());
    return true;
}


/*--------------------
  vSpec
  --------------------*/
SHADER_VAR(vSpec)
{
    CVec2f vSpec(g_pCurrentMaterial->GetSpecularLevel(), g_pCurrentMaterial->GetGlossiness());

    glUniform2fvARB(iLocation, 1, (GLfloat *)&vSpec);
    return true;
}



/*--------------------
  mFowProj
  --------------------*/
SHADER_VAR(mFowProj)
{
    D3DXMATRIXA16 mWorldFowProj = g_mWorld * g_mFowProj;

    glUniformMatrix4fvARB(iLocation, 1, GL_FALSE, (GLfloat *)&mWorldFowProj);
    return true;
}


/*--------------------
  mCloudProj
  --------------------*/
SHADER_VAR(mCloudProj)
{
    D3DXMATRIXA16 mWorldCloudProj = g_mWorld * g_mCloudProj;

    glUniformMatrix4fvARB(iLocation, 1, GL_FALSE, (GLfloat *)&mWorldCloudProj);
    return true;
}


/*--------------------
  fFogDensity
  --------------------*/
SHADER_VAR(fFogDensity)
{
    glUniform1fARB(iLocation, gfx_fogDensity);
    return true;
}


/*--------------------
  fFogStart
  --------------------*/
SHADER_VAR(fFogStart)
{
    glUniform1fARB(iLocation, gfx_fogNear);
    return true;
}


/*--------------------
  fFogEnd
  --------------------*/
SHADER_VAR(fFogEnd)
{
    glUniform1fARB(iLocation, gfx_fogFar);
    return true;
}


/*--------------------
  fFogDelta
  --------------------*/
SHADER_VAR(fFogDelta)
{
    glUniform1fARB(iLocation, MAX(gfx_fogFar - gfx_fogNear, 0.0f));
    return true;
}



/*--------------------
  fFogScale
  --------------------*/
SHADER_VAR(fFogScale)
{
    glUniform1fARB(iLocation, CLAMP<float>(gfx_fogScale, 0.0f, 1.0f));
    return true;
}


/*--------------------
  vFogColor
  --------------------*/
SHADER_VAR(vFogColor)
{
    CVec3f v3FogColor(gfx_fogColor[R], gfx_fogColor[G], gfx_fogColor[B]);

    glUniform3fvARB(iLocation, 1, (GLfloat *)&v3FogColor);
    return true;
}


/*--------------------
  vFog
  --------------------*/
SHADER_VAR(vFog)
{
    CVec3f v3Fog(1.0f / (MAX<float>(gfx_fogFar, 0.0f) - MAX<float>(gfx_fogNear, 0.0f)),
        -MAX<float>(gfx_fogNear, 0.0f) / (MAX<float>(gfx_fogFar, 0.0f) - MAX<float>(gfx_fogNear, 0.0f)),
        CLAMP<float>(gfx_fogScale, 0.0f, 1.0f));

    glUniform3fvARB(iLocation, 1, (GLfloat *)&v3Fog);
    return true;
}


/*--------------------
  vPointLightPosition
  --------------------*/
SHADER_VAR(vPointLightPosition)
{
    glUniform3fvARB(iLocation, g_iNumActivePointLights, (GLfloat *)&g_vPointLightPosition);
    return true;
}


/*--------------------
  vPointLightPositionView
  --------------------*/
SHADER_VAR(vPointLightPositionView)
{
    CVec3f v3ViewPosition[MAX_POINT_LIGHTS];

    for (int i = 0; i < g_iNumActivePointLights; ++i)
        D3DXVec3Transform((D3DXVECTOR3 *)&v3ViewPosition[i], (D3DXVECTOR3 *)&g_vPointLightPosition[i], &g_mView);

    glUniform3fvARB(iLocation, g_iNumActivePointLights, (GLfloat *)&v3ViewPosition);
    return true;
}


/*--------------------
  vPointLightPositionOffset
  --------------------*/
SHADER_VAR(vPointLightPositionOffset)
{
    CVec3f v3Position[MAX_POINT_LIGHTS];

    for (int i = 0; i < g_iNumActivePointLights; ++i)
        D3DXVec3Transform((D3DXVECTOR3 *)&v3Position[i], (D3DXVECTOR3 *)&g_vPointLightPosition[i], &g_mViewOffset);

    glUniform3fvARB(iLocation, g_iNumActivePointLights, (GLfloat *)&v3Position);
    return true;
}


/*--------------------
  vPointLightColor
  --------------------*/
SHADER_VAR(vPointLightColor)
{
    glUniform3fvARB(iLocation, g_iNumActivePointLights, (GLfloat *)&g_vPointLightColor);
    return true;
}


/*--------------------
  vPointLightColorSpec
  --------------------*/
SHADER_VAR(vPointLightColorSpec)
{
    CVec3f vColor[MAX_POINT_LIGHTS];

    for (int i = 0; i < g_iNumActivePointLights; ++i)
        vColor[i] = g_vPointLightColor[i] * g_pCurrentMaterial->GetSpecularLevel();

    glUniform3fvARB(iLocation, g_iNumActivePointLights, (GLfloat *)&vColor);
    return true;
}


/*--------------------
  fPointLightFalloffStart
  --------------------*/
SHADER_VAR(fPointLightFalloffStart)
{
    glUniform1fvARB(iLocation, g_iNumActivePointLights, (GLfloat *)&g_fPointLightFalloffStart);
    return true;
}


/*--------------------
  fPointLightFalloffEnd
  --------------------*/
SHADER_VAR(fPointLightFalloffEnd)
{
    glUniform1fvARB(iLocation, g_iNumActivePointLights, (GLfloat *)&g_fPointLightFalloffEnd);
    return true;
}


/*--------------------
  fPointLightFalloff
  --------------------*/
SHADER_VAR(fPointLightFalloff)
{
    CVec2f vFalloff[MAX_POINT_LIGHTS];

    for (int i = 0; i < g_iNumActivePointLights; ++i)
        vFalloff[i] = CVec2f(g_fPointLightFalloffStart[i], g_fPointLightFalloffEnd[i] - g_fPointLightFalloffStart[i]);

    glUniform2fvARB(iLocation, g_iNumActivePointLights, (GLfloat *)&vFalloff);
    return true;
}


/*--------------------
  vPointLightFalloff
  --------------------*/
SHADER_VAR(vPointLightFalloff)
{
    CVec2f vFalloff[MAX_POINT_LIGHTS];

    for (int i = 0; i < g_iNumActivePointLights; ++i)
        vFalloff[i] = CVec2f(1.0f / (MAX<float>(g_fPointLightFalloffEnd[i], 0.0f) - MAX<float>(g_fPointLightFalloffStart[i], 0.0f)),
            -MAX<float>(g_fPointLightFalloffStart[i], 0.0f) / (MAX<float>(g_fPointLightFalloffEnd[i], 0.0f) - MAX<float>(g_fPointLightFalloffStart[i], 0.0f)));

    glUniform2fvARB(iLocation, g_iNumActivePointLights, (GLfloat *)&vFalloff);
    return true;

}


/*--------------------
  fShadowLeak
  --------------------*/
SHADER_VAR(fShadowLeak)
{
    glUniform1fARB(iLocation, vid_shadowLeak);
    return true;
}


/*--------------------
  fOneMinusShadowLeak
  --------------------*/
SHADER_VAR(fOneMinusShadowLeak)
{
    glUniform1fARB(iLocation, 1.0f - vid_shadowLeak);
    return true;
}


/*--------------------
  vShadowLeak
  --------------------*/
SHADER_VAR(vShadowLeak)
{
    CVec2f vShadowLeak(1.0f - vid_shadowLeak, vid_shadowLeak);

    glUniform2fvARB(iLocation, 1, (GLfloat *)&vShadowLeak);
    return true;
}


/*--------------------
  fShadowmapSize
  --------------------*/
SHADER_VAR(fShadowmapSize)
{
    glUniform1fARB(iLocation, float(vid_shadowmapSize));
    return true;
}


/*--------------------
  fShadowmapSizeInv
  --------------------*/
SHADER_VAR(fShadowmapSizeInv)
{
    glUniform1fARB(iLocation, 1.0f / vid_shadowmapSize);
    return true;
}


/*--------------------
  fShadowFalloffStart
  --------------------*/
SHADER_VAR(fShadowFalloffStart)
{
    if (g_pCam && g_pCam->HasFlags(CAM_SHADOW_NO_FALLOFF))
        glUniform1fARB(iLocation, g_pCam->GetZFar());
    else
        glUniform1fARB(iLocation, MAX(vid_shadowDrawDistance - vid_shadowFalloffDistance, 0.0f));
    return true;
}


/*--------------------
  fShadowFalloffEnd
  --------------------*/
SHADER_VAR(fShadowFalloffEnd)
{
    if (g_pCam && g_pCam->HasFlags(CAM_SHADOW_NO_FALLOFF))
        glUniform1fARB(iLocation, g_pCam->GetZFar() + 1.0f);
    else
        glUniform1fARB(iLocation, MAX(float(vid_shadowDrawDistance), 0.0f));
    return true;
}


/*--------------------
  fShadowFalloffLength
  --------------------*/
SHADER_VAR(fShadowFalloffLength)
{
    if (g_pCam && g_pCam->HasFlags(CAM_SHADOW_NO_FALLOFF))
        glUniform1fARB(iLocation, 1.0f);
    else
        glUniform1fARB(iLocation, MAX(float(vid_shadowDrawDistance), 0.0f) - MAX(vid_shadowDrawDistance - vid_shadowFalloffDistance, 0.0f));
    return true;
}


/*--------------------
  vShadowFalloff
  --------------------*/
SHADER_VAR(vShadowFalloff)
{
    if (g_pCam && g_pCam->HasFlags(CAM_SHADOW_NO_FALLOFF))
    {
        CVec2f vShadowFalloff(1.0f, -g_pCam->GetZFar());
        glUniform2fvARB(iLocation, 1, (GLfloat *)&vShadowFalloff);
    }
    else
    {
        CVec2f vShadowFalloff(1.0f / (MAX(float(vid_shadowDrawDistance), 0.0f) - MAX(vid_shadowDrawDistance - vid_shadowFalloffDistance, 0.0f)),
            -MAX(vid_shadowDrawDistance - vid_shadowFalloffDistance, 0.0f) / (MAX(float(vid_shadowDrawDistance), 0.0f) - MAX(vid_shadowDrawDistance - vid_shadowFalloffDistance, 0.0f)));

        glUniform2fvARB(iLocation, 1, (GLfloat *)&vShadowFalloff);
    }

    return true;
}


/*--------------------
  vLeafClusters
  --------------------*/
SHADER_VAR(vLeafClusters)
{
    glUniform4fvARB(iLocation, MAX_LEAF_CLUSTER_INDEX, (GLfloat *)&g_afLeafClusterData);
    return true;
}


/*--------------------
  fFoliageFalloffStart
  --------------------*/
SHADER_VAR(fFoliageFalloffStart)
{
    glUniform1fARB(iLocation, MAX(SceneManager.GetFoliageDrawDistance() - vid_foliageFalloffDistance, 0.0f));
    return true;
}


/*--------------------
  fFoliageFalloffStart
  --------------------*/
SHADER_VAR(fFoliageFalloffEnd)
{
    glUniform1fARB(iLocation,  MAX(SceneManager.GetFoliageDrawDistance(), 0.0f));
    return true;
}


/*--------------------
  fFoliageFalloffLength
  --------------------*/
SHADER_VAR(fFoliageFalloffLength)
{
    glUniform1fARB(iLocation, MAX(SceneManager.GetFoliageDrawDistance(), 0.0f) - MAX(SceneManager.GetFoliageDrawDistance() - vid_foliageFalloffDistance, 0.0f));
    return true;
}


/*--------------------
  fFoliageAnimateSpeed
  --------------------*/
SHADER_VAR(fFoliageAnimateSpeed)
{
    glUniform1fARB(iLocation, vid_foliageAnimateSpeed);
    return true;
}


/*--------------------
  fFoliageAnimateSpeed
  --------------------*/
SHADER_VAR(fFoliageAnimateStrength)
{
    glUniform1fARB(iLocation, vid_foliageAnimateStrength);
    return true;
}


/*--------------------
  vFoliage
  --------------------*/
SHADER_VAR(vFoliage)
{
    CVec4f vFoliage(1.0f / (MAX(SceneManager.GetFoliageDrawDistance(), 0.0f) - MAX(SceneManager.GetFoliageDrawDistance() - vid_foliageFalloffDistance, 0.0f)),
        -MAX(SceneManager.GetFoliageDrawDistance() - vid_foliageFalloffDistance, 0.0f) / (MAX(SceneManager.GetFoliageDrawDistance(), 0.0f) - MAX(SceneManager.GetFoliageDrawDistance() - vid_foliageFalloffDistance, 0.0f)),
        vid_foliageAnimateSpeed,
        vid_foliageAnimateStrength);

    glUniform4fvARB(iLocation, 1, (GLfloat *)&vFoliage);
    return true;
}


/*--------------------
  fSkyEpsilon
  --------------------*/
SHADER_VAR(fSkyEpsilon)
{
    glUniform1fARB(iLocation, vid_skyEpsilon);
    return true;
}


/*--------------------
  vCameraPositionWorld
  --------------------*/
SHADER_VAR(vCameraPositionWorld)
{
    CVec3f  v3CameraPosition(g_pCam ? g_pCam->GetOrigin() : V3_ZERO);

    glUniform3fvARB(iLocation, 1, (GLfloat *)&v3CameraPosition);
    return true;
}


/*--------------------
  vCameraPosition
  --------------------*/
SHADER_VAR(vCameraPosition)
{
    D3DXMATRIXA16 mWorldInverse;
    D3DXMatrixInverse(&mWorldInverse, NULL, &g_mWorld);

    CVec3f  v3CameraPosition(g_pCam ? g_pCam->GetOrigin() : V3_ZERO);

    D3DXVec3Transform((D3DXVECTOR3 *)&v3CameraPosition, (D3DXVECTOR3 *)&v3CameraPosition, &mWorldInverse);

    glUniform3fvARB(iLocation, 1, (GLfloat *)&v3CameraPosition);
    return true;
}


/*--------------------
  fShadowDepthBias
  --------------------*/
SHADER_VAR(fShadowDepthBias)
{
    glUniform1fARB(iLocation, vid_shadowDepthBias / 16777215.f);
    return true;
}


/*--------------------
  vViewUp
  --------------------*/
SHADER_VAR(vViewUp)
{
    D3DXVECTOR4 vUp;

    vUp.x = 0.0f;
    vUp.y = 0.0f;
    vUp.z = 1.0f;
    vUp.w = 1.0f;

    D3DXVec3TransformNormal((D3DXVECTOR3 *)&vUp, (D3DXVECTOR3 *)&vUp, &g_mViewRotate);

    glUniform3fvARB(iLocation, 1, (GLfloat *)&vUp);
    return true;
}


/*--------------------
  fWorldTextureProjection
  --------------------*/
SHADER_VAR(fWorldTextureProjection)
{
    glUniform1fARB(iLocation, GfxTerrain->pWorld ? 1.0f / (GfxTerrain->pWorld->GetScale() * GfxTerrain->pWorld->GetTextureScale()) : 1.0f / (64.0f * 4.0f));
    return true;
}


/*--------------------
  fTexelSize
  --------------------*/
SHADER_VAR(fTexelSize)
{
    glUniform1fARB(iLocation, 1.0f / g_uiImageWidth);
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
    fScaleY = 0.5f;

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

    fOffsetX = (0.0f / g_uiImageWidth);
    fOffsetY = (0.0f / g_uiImageHeight);

    D3DXMATRIXA16 mTexOffset2(1.0f,     0.0f,     0.0f, 0.0f,
                              0.0f,     1.0f,     0.0f, 0.0f,
                              0.0f,     0.0f,     1.0f, 0.0f,
                              fOffsetX, fOffsetY, 0.0f, 1.0f);

    D3DXMATRIXA16 mScreenProj = mTexScale1 * mTexOffset1 * mTexOffset2;

    glUniformMatrix4fvARB(iLocation, 1, GL_FALSE, (GLfloat *)&mScreenProj);
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
        -S / Y,
        0.0f,
        0.0f
    );

    glUniform2fvARB(iLocation, 1, (GLfloat *)&vScene);
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

    glUniform3fvARB(iLocation, 1, (GLfloat *)&vBright);
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

    glUniform3fvARB(iLocation, 1, (GLfloat *)&vBright);
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

    glUniform4fvARB(iLocation, 1, (GLfloat *)&vTexelSizes);
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

        glUniform4fvARB(iLocation, 1, (GLfloat *)&vParam);
    }
    else
    {
        D3DXVECTOR4 vParam(0.0f, 0.0f, 0.0f, 0.0f);
        glUniform4fvARB(iLocation, 1, (GLfloat *)&vParam);
    }

    return true;
}


/*--------------------
  fDistance
  --------------------*/
SHADER_VAR(fDistance)
{
    if (g_pCurrentEntity != NULL)
        glUniform1fARB(iLocation, Distance(g_pCurrentEntity->GetPosition(), g_pCam->GetOrigin()));
    else
        glUniform1fARB(iLocation, 0.0f);

    return true;
}



