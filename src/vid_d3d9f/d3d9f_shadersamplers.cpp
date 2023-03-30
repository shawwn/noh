// (C)2005 S2 Games
// d3d9f_shadervars.cpp
//
// Direct3D Shader Variables
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "d3d9f_main.h"
#include "d3d9f_material.h"
#include "d3d9f_texture.h"
#include "d3d9f_state.h"
#include "d3d9f_scene.h"
#include "d3d9f_terrain.h"
#include "c_shadersampler.h"
#include "c_proceduralregistry.h"
#include "c_shadowmap.h"
#include "c_fogofwar.h"
#include "c_scenebuffer.h"
#include "c_reflectionmap.h"

#include "../k2/c_console.h"
#include "../k2/c_system.h"
#include "../k2/c_vec3.h"
#include "../k2/c_mesh.h"
#include "../k2/c_texture.h"
//=============================================================================

/*--------------------
  shadowmap
  --------------------*/
SHADER_SAMPLER(shadowmap)
{
    D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

    if (vid_shadowmapType == SHADOWMAP_R32F)
    {
        if (vid_shadowmapMagFilter)
        {
            D3D_SetSamplerState(iStageIndex, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
            D3D_SetSamplerState(iStageIndex, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        }
        else
        {
            D3D_SetSamplerState(iStageIndex, D3DSAMP_MINFILTER, D3DTEXF_POINT);
            D3D_SetSamplerState(iStageIndex, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
        }

    }
    else if (vid_shadowmapType == SHADOWMAP_DEPTH)
    {
        if (vid_shadowmapFilterWidth == 0)
        {
            D3D_SetSamplerState(iStageIndex, D3DSAMP_MINFILTER, D3DTEXF_POINT);
            D3D_SetSamplerState(iStageIndex, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
        }
        else
        {
            D3D_SetSamplerState(iStageIndex, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
            D3D_SetSamplerState(iStageIndex, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        }
    }
    if (vid_shadowmapType == SHADOWMAP_VARIANCE)
    {
        D3D_SetSamplerState(iStageIndex, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        D3D_SetSamplerState(iStageIndex, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    }

    D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_BORDERCOLOR, D3DCOLOR_ARGB(255, 255, 255, 255));

    D3D_SetTexture(iStageIndex, g_pTextures[g_Shadowmap.GetShadowmapIndex()]);
    return true;
}


/*--------------------
  specularLookup
  --------------------*/
SHADER_SAMPLER(specularLookup)
{
    D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

    D3D_SetTexture(iStageIndex, g_pTextures[CProceduralRegistry::GetInstance()->GetTextureIndex(_T("specularLookup"))]);
    return true;
}


/*--------------------
  clouds
  --------------------*/
SHADER_SAMPLER(clouds)
{
    D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

    CTexture *pTexture(g_ResourceManager.GetTexture(g_hCloudTexture));

    int iTexture(pTexture ? pTexture->GetIndex() : -1);

    D3D_SetTexture(iStageIndex, iTexture == -1 ? NULL : g_pTextures[iTexture]);
    return true;
}


/*--------------------
  terrainFudge
  --------------------*/
SHADER_SAMPLER(terrainFudge)
{
    D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_MAGFILTER, D3DTEXF_POINT);

    D3D_SetTexture(iStageIndex, g_pTextures[CProceduralRegistry::GetInstance()->GetTextureIndex(_T("terrainFudge"))]);
    return true;
}


/*--------------------
  alpha
  --------------------*/
SHADER_SAMPLER(alpha)
{
    D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

    D3D_SetTexture(iStageIndex, g_pTextures[g_iTerrainAlphaMap]);
    return true;
}


/*--------------------
  fogofwar
  --------------------*/
SHADER_SAMPLER(fogofwar)
{
    D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

    if (g_FogofWar.IsValid())
        D3D_SetTexture(iStageIndex, g_pTextures[g_FogofWar.GetTextureIndex()]);
    else
        D3D_SetTexture(iStageIndex, g_pTextures[g_iWhite]);

    return true;
}


/*--------------------
  scene
  --------------------*/
SHADER_SAMPLER(scene)
{
    D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, vid_sceneBufferMipmap ? D3DTEXF_LINEAR : D3DTEXF_NONE);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

    D3D_SetTexture(iStageIndex, g_pTextures[g_SceneBuffer.GetTextureIndex()]);
    return true;
}


/*--------------------
  reflection
  --------------------*/
SHADER_SAMPLER(reflection)
{
    D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, REFLECTIONS_MIPMAP ? D3DTEXF_LINEAR : D3DTEXF_NONE);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

    D3D_SetTexture(iStageIndex, g_pTextures[g_ReflectionMap.GetTextureIndex()]);
    return true;
}
