// (C)2008 S2 Games
// gl2_shadersamplers.cpp
//
// Renderer supplied shader samplers
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_gfxshaders.h"
#include "c_gfxterrain.h"
#include "c_shadowmap.h"
#include "c_fogofwar.h"
#include "c_shadersampler.h"
#include "c_proceduralregistry.h"
#include "c_scenebuffer.h"

#include "../k2/c_console.h"
#include "../k2/c_system.h"
#include "../k2/c_vec3.h"
#include "../k2/c_mesh.h"
#include "../k2/c_texture.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

/*--------------------
  shadowmap
  --------------------*/
SHADER_SAMPLER(shadowmap)
{
	glActiveTextureARB(GL_TEXTURE0_ARB + iStageIndex);
	glBindTexture(GL_TEXTURE_2D, g_Shadowmap.GetShadowmapIndex());

	if (vid_shadowmapType == SHADOWMAP_R32F)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	else if (vid_shadowmapType == SHADOWMAP_DEPTH)
	{
		if (vid_shadowmapFilterWidth == 0)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	}

	CVec4f borderColor(1.0f, 1.0f, 1.0f, 1.0f);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, (GLfloat *)&borderColor);
	return true;
}


/*--------------------
  fogofwar
  --------------------*/
SHADER_SAMPLER(fogofwar)
{
	glActiveTextureARB(GL_TEXTURE0_ARB + iStageIndex);
	glBindTexture(GL_TEXTURE_2D, g_FogofWar.GetTextureIndex());
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	return true;
}


/*--------------------
  scene
  --------------------*/
SHADER_SAMPLER(scene)
{
	glActiveTextureARB(GL_TEXTURE0_ARB + iStageIndex);
	glBindTexture(GL_TEXTURE_2D, g_SceneBuffer.GetTextureIndex());
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (vid_sceneBufferMipmap)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	return true;
}


/*--------------------
  clouds
  --------------------*/
SHADER_SAMPLER(clouds)
{
	CTexture *pTexture(g_ResourceManager.GetTexture(g_hCloudTexture));

	GLuint uiTexture(pTexture ? pTexture->GetIndex() : 0);

	glActiveTextureARB(GL_TEXTURE0_ARB + iStageIndex);
	glBindTexture(GL_TEXTURE_2D, uiTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	return true;
}


/*--------------------
  alpha
  --------------------*/
SHADER_SAMPLER(alpha)
{
	glActiveTextureARB(GL_TEXTURE0_ARB + iStageIndex);
	glBindTexture(GL_TEXTURE_2D, GfxTerrain->uiTerrainAlphaMap);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	return true;
}


#if 0
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
#endif
