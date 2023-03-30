// (C)2005 S2 Games
// d3d9f_material.cpp
//
// Direct3D shader functions
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "d3d9f_main.h"
#include "d3d9f_material.h"
#include "d3d9f_state.h"
#include "d3d9f_util.h"
#include "d3d9f_shader.h"
#include "d3d9f_scene.h"
#include "d3d9f_texture.h"
#include "c_procedural.h"
#include "c_proceduralregistry.h"
#include "c_shadervar.h"
#include "c_shadersampler.h"
#include "c_shaderpreprocessor.h"
#include "c_shaderregistry.h"
#include "c_shadowmap.h"

#include "../k2/c_material.h"
#include "../k2/c_materialparameter.h"
#include "../k2/c_resourcemanager.h"
#include "../k2/c_scenemanager.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CVAR_BOOL	(vid_alphaTest,		true);
CVAR_INTF	(vid_alphaTestRef,	90, CVAR_SAVECONFIG);
CVAR_FLOAT	(vid_depthBiasScale, -1.0f / 65536);


const CMaterial				*g_pCurrentMaterial;
EMaterialPhase				g_ePhase(EMaterialPhase(-1));
int							g_iVertexType;
float						g_fTime;

int							g_iCurrentVertexShader;
int							g_iCurrentPixelShader;

bool						g_bCamShadows;
bool						g_bCamFogofWar;
bool						g_bCamFog;

bool						g_bReflectPass;

// Shader Globals
bool						g_bLighting;
bool						g_bShadows;
bool						g_bFogofWar;
bool						g_bFog;
CVec3f						g_v3SunColor;
CVec3f						g_v3Ambient;
CVec3f						g_vPointLightPosition[MAX_POINT_LIGHTS];
CVec3f						g_vPointLightColor[MAX_POINT_LIGHTS];
float						g_fPointLightFalloffStart[MAX_POINT_LIGHTS];
float						g_fPointLightFalloffEnd[MAX_POINT_LIGHTS];
int							g_iNumActivePointLights;
bool						g_bObjectColor;
CVec4f						g_vObjectColor;
int							g_iTexcoords;
bool						g_bTexkill;

float						g_afRegisterBuffer[256 * 4];

int blendmodeMapping[] =
{
	D3DBLEND_ZERO,
	D3DBLEND_ONE,
	D3DBLEND_SRCCOLOR,
	D3DBLEND_DESTCOLOR,
	D3DBLEND_INVSRCCOLOR,
	D3DBLEND_INVDESTCOLOR,
	D3DBLEND_SRCALPHA,
	D3DBLEND_DESTALPHA,
	D3DBLEND_INVSRCALPHA,
	D3DBLEND_INVDESTCOLOR
};

int cullmodeMapping[] =
{
	D3DCULL_CW,
	D3DCULL_CCW,
	D3DCULL_NONE
};

int cullmodeMappingInv[] =
{
	D3DCULL_CCW,
	D3DCULL_CW,
	D3DCULL_NONE
};

int cullmodeMappingBack[] =
{
	D3DCULL_CCW,
	D3DCULL_CW,
	D3DCULL_NONE
};

int cullmodeMappingInvBack[] =
{
	D3DCULL_CW,
	D3DCULL_CCW,
	D3DCULL_NONE
};

static vector<SMaterialState>		g_MaterialState(8);
uint g_uiMaterialState(0);
//=============================================================================

/*====================
  D3D_SetVertexShaderConstantFloat
  ====================*/
void	D3D_SetVertexShaderConstantFloat(const string &sName, float fValue)
{
	D3DXHANDLE hHandle(g_aVertexShaderSlots[g_iCurrentVertexShader].pConstantTable->GetConstantByName(NULL, sName.c_str()));

	if (!hHandle)
		return;

	g_aVertexShaderSlots[g_iCurrentVertexShader].pConstantTable->SetFloat(g_pd3dDevice, hHandle, fValue);
}


/*====================
  D3D_SetPixelShaderConstantFloat
  ====================*/
void	D3D_SetPixelShaderConstantFloat(const string &sName, float fValue)
{
	D3DXHANDLE hHandle(g_aPixelShaderSlots[g_iCurrentPixelShader].pConstantTable->GetConstantByName(NULL, sName.c_str()));

	if (!hHandle)
		return;

	g_aPixelShaderSlots[g_iCurrentPixelShader].pConstantTable->SetFloat(g_pd3dDevice, hHandle, fValue);
}


/*====================
  D3D_SetSampler
  ====================*/
void	D3D_SetSampler(int iStageIndex, const CMaterialSampler &sampler, uint uiSubTexture)
{
	PROFILE("D3D_SetSampler");

	if (iStageIndex == -1)
		return;

	ResHandle hTexture(sampler.GetTexture());
	CTexture *pTexture(g_ResourceManager.GetTexture(hTexture));
	
	int iTextureFlags(pTexture != NULL ? pTexture->GetTextureFlags() : 0);

	if (sampler.HasFlags(SAM_BORDER))
	{
		D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
		D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
		D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSW, D3DTADDRESS_BORDER);

		D3D_SetSamplerState(iStageIndex, D3DSAMP_BORDERCOLOR, D3DCOLOR_ARGB(255, 255, 255, 255));
	}
	else
	{
		if (sampler.HasFlags(SAM_REPEAT_U))
			D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		else
			D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);

		if (sampler.HasFlags(SAM_REPEAT_V))
			D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		else
			D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

		if (sampler.HasFlags(SAM_REPEAT_W))
			D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
		else
			D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP);
	}

	// Setup texture mipmaps
	if (sampler.HasFlags(SAM_NO_MIPMAPS) || iTextureFlags & TEX_NO_MIPMAPS)
	{
		D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	}
	else
	{
		if (vid_textureFiltering >= TEXTUREFILTERING_ANISOTROPIC2)
			D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
		else if (vid_textureFiltering == TEXTUREFILTERING_TRILINEAR)
			D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
		else
			D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
		
	}

	if (sampler.HasFlags(SAM_NO_FILTERING))
	{
		D3D_SetSamplerState(iStageIndex, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		D3D_SetSamplerState(iStageIndex, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	}
	else
	{
		if (vid_textureFiltering >= TEXTUREFILTERING_ANISOTROPIC2) 
		{
			D3D_SetSamplerState(iStageIndex, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
			D3D_SetSamplerState(iStageIndex, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		}
		else if (vid_textureFiltering >= TEXTUREFILTERING_TRILINEAR || vid_textureFiltering >= TEXTUREFILTERING_BILINEAR)
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

	if ((gfx_textures == 1 || gfx_textures == 3) && sampler.GetName().compare(0, 7, _T("diffuse")) == 0)
	{
		int iTexture(CProceduralRegistry::GetInstance()->GetTextureIndex(_T("white")));

		D3D_SetTexture(iStageIndex, iTexture == -1 ? NULL : g_pTextures[iTexture]);
	}
	else if ((gfx_textures == 2 || gfx_textures == 3) && sampler.GetName().compare(0, 9, _T("normalmap")) == 0)
	{
		int iTexture(CProceduralRegistry::GetInstance()->GetTextureIndex(_T("flat_dull")));

		D3D_SetTexture(iStageIndex, iTexture == -1 ? NULL : g_pTextures[iTexture]);
	}
	else if (gfx_textures == 4 && sampler.GetName().compare(0, 7, _T("diffuse")) == 0)
	{
		int iTexture(CProceduralRegistry::GetInstance()->GetTextureIndex(_T("size_hint")));

		D3D_SetTexture(iStageIndex, iTexture == -1 ? NULL : g_pTextures[iTexture]);
	}
	else
	{
		int iTexture(pTexture != NULL ? uiSubTexture > 0 ? pTexture->GetIndex2() : pTexture->GetIndex() : -1);

		D3D_SetTexture(iStageIndex, iTexture == -1 ? NULL : g_pTextures[iTexture]);
	}
}


/*====================
  D3D_PushMaterial

  Push all the material properties onto a nice stack that can be
  later popped to restore the old values
  ====================*/
void	D3D_PushMaterial(const CMaterial &material, EMaterialPhase ePhase, int iVertexType, float fTime, bool bDepthFirst)
{
	// Push a place holder state onto the stack
	// D3D_SelectMaterial will replace these with the proper new values
	// while preserving the previous states
	if (g_uiMaterialState < 8 - 1)
		++g_uiMaterialState;

	D3D_SelectMaterial(material, ePhase, iVertexType, fTime, bDepthFirst);
}


/*====================
  D3D_PopMaterial

  use the back of the stack to restore old material values
  ====================*/
void	D3D_PopMaterial()
{
	if (g_uiMaterialState > 0)
	{
		--g_uiMaterialState;

		SMaterialState	oMaterialState = g_MaterialState[g_uiMaterialState];
		D3D_SelectMaterial(*oMaterialState.pMaterial, oMaterialState.ePhase, oMaterialState.iVertexType, oMaterialState.fTime, oMaterialState.bDepthFirst);
	}
}


/*====================
  D3D_GetMaterialState
  ====================*/
const SMaterialState&	D3D_GetMaterialState()
{
	return g_MaterialState[g_uiMaterialState];
}


#if 0
/*====================
  D3D_SelectMaterial
  ====================*/
bool	D3D_SelectMaterial(const CMaterial &material, EMaterialPhase ePhase, int iVertexType, float fTime, bool bDepthFirst)
{
	PROFILE("D3D_SelectMaterial");

	assert(material.HasPhase(ePhase));
	assert(!(ePhase == PHASE_DEPTH && !bDepthFirst));
	assert(g_iNumActivePointLights <= MAX_POINT_LIGHTS);

	g_pCurrentMaterial = &material;
	g_ePhase = ePhase;
	g_iVertexType = iVertexType;
	g_fTime = fTime;

	// Make the back of the deque matches our current material state
	SMaterialState	oMaterialState;

	oMaterialState.pMaterial = &material;
	oMaterialState.ePhase = ePhase;
	oMaterialState.iVertexType = iVertexType;
	oMaterialState.fTime = fTime;
	oMaterialState.bDepthFirst = bDepthFirst;
	oMaterialState.iPass = material.GetPass();

	g_MaterialState[g_uiMaterialState] = oMaterialState;

	g_ShaderRegistry.SetNumPointLights(g_iNumActivePointLights);
	g_ShaderRegistry.SetNumBones(g_iNumActiveBones);
	g_ShaderRegistry.SetLighting(g_bLighting);
	g_ShaderRegistry.SetShadows(g_bShadows);
	g_ShaderRegistry.SetFogofWar(g_bFogofWar);
	g_ShaderRegistry.SetFog(g_bFog);
	g_ShaderRegistry.SetTexcoords(g_iTexcoords);
	g_ShaderRegistry.SetTexkill(g_bTexkill);

	const CMaterialPhase &phase = material.GetPhase(ePhase);

	D3D_SetVertexDeclaration(g_pVertexDeclarations[iVertexType]);

	// turn off unused texture stages
	for (uint ui(phase.GetNumSamplers()); ui < g_dwNumSamplers; ++ui)
		D3D_SetTexture(ui, NULL); 

	if (bDepthFirst && material.HasPhase(PHASE_DEPTH))
	{
		switch (ePhase)
		{
		case PHASE_COLOR:
			D3D_SetRenderState(D3DRS_ZFUNC, D3DCMP_EQUAL);
			D3D_SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
			D3D_SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
			break;
		case PHASE_DEPTH:
			D3D_SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);
			D3D_SetRenderState(D3DRS_ALPHATESTENABLE, vid_alphaTest ? phase.GetAlphaTest() : FALSE);
			D3D_SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			break;
		default:
		case PHASE_SHADOW:
			D3D_SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
			D3D_SetRenderState(D3DRS_ALPHATESTENABLE, vid_alphaTest ? phase.GetAlphaTest() : FALSE);
			D3D_SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			break;
		}
	}
	else
	{
		D3D_SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
		D3D_SetRenderState(D3DRS_ZWRITEENABLE, phase.GetDepthWrite() ? TRUE : FALSE);
		D3D_SetRenderState(D3DRS_ALPHATESTENABLE, !(vid_shaderTexkill && (!vid_shaderTexkillColorOnly || ePhase == PHASE_COLOR)) && phase.GetAlphaTest());
	}

	D3D_SetRenderState(D3DRS_ZENABLE, phase.GetDepthRead() ? TRUE : FALSE);

	if (ePhase == PHASE_COLOR || ePhase == PHASE_FADE)
	{
		uint uiWriteMask(0);

		if (phase.GetColorWrite())
			uiWriteMask |= D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE;
		if (phase.GetAlphaWrite())
			uiWriteMask |= D3DCOLORWRITEENABLE_ALPHA;

		D3D_SetRenderState(D3DRS_COLORWRITEENABLE, uiWriteMask);
	}

	if (phase.GetTranslucent())
		D3D_SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	else
		D3D_SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

	D3D_SetRenderState(D3DRS_SRCBLEND, blendmodeMapping[phase.GetSrcBlend()]);
	D3D_SetRenderState(D3DRS_DESTBLEND, blendmodeMapping[phase.GetDstBlend()]);

	if (ePhase == PHASE_SHADOW && (vid_shadowBackface || g_Shadowmap.GetType() == SHADOWMAP_R32F))
	{
		if (g_bInvertedProjection)
			D3D_SetRenderState(D3DRS_CULLMODE, cullmodeMappingInvBack[phase.GetCullMode()]);
		else
			D3D_SetRenderState(D3DRS_CULLMODE, cullmodeMappingBack[phase.GetCullMode()]);
	}
	else
	{
		if (g_bInvertedProjection)
			D3D_SetRenderState(D3DRS_CULLMODE, cullmodeMappingInv[phase.GetCullMode()]);
		else
			D3D_SetRenderState(D3DRS_CULLMODE, cullmodeMapping[phase.GetCullMode()]);
	}

	if (ePhase == PHASE_SHADOW && g_Shadowmap.GetType() == SHADOWMAP_DEPTH)
	{
		if (g_DeviceCaps.bSlopeScaledDepthBias)
			D3D_SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, D3D_DWORD(vid_shadowSlopeBias + vid_shadowmapFilterWidth));
		if (g_DeviceCaps.bDepthBias)
			D3D_SetRenderState(D3DRS_DEPTHBIAS, D3D_DWORD(vid_shadowDepthBias / 16777215.f));
	}
	else
	{
		if (g_DeviceCaps.bSlopeScaledDepthBias)
			D3D_SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, phase.GetDepthSlopeBias() ? D3D_DWORD(-1.0f) : D3D_DWORD(0.0));
		if (g_DeviceCaps.bDepthBias)
			D3D_SetRenderState(D3DRS_DEPTHBIAS, D3D_DWORD(phase.GetDepthBias() * vid_depthBiasScale));
	}


	//
	// Vertex Shader Parameters
	//
	{
		PROFILE("Vertex Shader Parameters");

		SVertexShaderSlot &VertexShaderSlot(g_aVertexShaderSlots[g_iCurrentVertexShader]);

		for (uint ui(0); ui < VertexShaderSlot.uiNumConstants; ++ui)
		{
			SVertexShaderConstant &VertexShaderConstant(VertexShaderSlot.vConstant[ui]);
			
			CShaderVar *pVar(VertexShaderConstant.pShaderVar);

			if (pVar)
				pVar->Get(&g_afRegisterBuffer[VertexShaderConstant.uiRegisterIndex << 2], VertexShaderConstant.uiSize);
			else
			{
				const tstring &sName(VertexShaderConstant.sName);

				// See if this material defines this parameter
				const IMaterialParameter *pParam(material.GetParameter(sName));

				if (pParam)
				{
					switch (pParam->GetType())
					{
					case MPT_FLOAT:
						g_afRegisterBuffer[VertexShaderConstant.uiRegisterIndex << 2] = pParam->GetFloat(fTime);
						break;
					case MPT_VEC2:
						MemManager.Copy(&g_afRegisterBuffer[VertexShaderConstant.uiRegisterIndex << 2], (float *)&pParam->GetVec2(fTime), sizeof(CVec2f));
						break;
					case MPT_VEC3:
						MemManager.Copy(&g_afRegisterBuffer[VertexShaderConstant.uiRegisterIndex << 2], (float *)&pParam->GetVec3(fTime), sizeof(CVec3f));
						break;
					case MPT_VEC4:
						MemManager.Copy(&g_afRegisterBuffer[VertexShaderConstant.uiRegisterIndex << 2], (float *)&pParam->GetVec4(fTime), sizeof(CVec4f));
						break;
					}
				}
				else
				{
					Console.Warn << "Shader parameter " << SingleQuoteStr(sName) << " not defined" << newl;
				}
			}
		}

		if (VertexShaderSlot.uiNumRegisters > 0)
			g_pd3dDevice->SetVertexShaderConstantF(0, g_afRegisterBuffer, VertexShaderSlot.uiNumRegisters);
	}


	//
	// Pixel Shader Parameters
	//
	{
		PROFILE("Pixel Shader Parameters");

		SPixelShaderSlot &PixelShaderSlot(g_aPixelShaderSlots[g_iCurrentPixelShader]);

		for (uint ui(0); ui < PixelShaderSlot.uiNumConstants; ++ui)
		{
			SPixelShaderConstant &PixelShaderConstant(PixelShaderSlot.vConstant[ui]);

			switch (PixelShaderConstant.eRegisterSet)
			{
			case D3DXRS_SAMPLER:
				{
					// Find sampler in the shader
					int iSamplerIndex(phase.GetSamplerIndex(PixelShaderConstant.sName));

					if (iSamplerIndex != -1)
					{
						D3D_SetSampler(PixelShaderConstant.uiRegisterIndex, phase.GetSampler(iSamplerIndex), PixelShaderConstant.uiSubTexture);
					}
					else
					{
						CShaderSampler *pSampler(PixelShaderConstant.pShaderSamplers);

						if (pSampler)
						{
							pSampler->Get(PixelShaderConstant.uiRegisterIndex);
						}
						else // No valid sampler found, so use white
						{
							int iStageIndex(PixelShaderConstant.uiRegisterIndex);

							D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
							D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
							D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
							D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
							D3D_SetSamplerState(iStageIndex, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
							D3D_SetSamplerState(iStageIndex, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
							D3D_SetTexture(iStageIndex, g_pTextures[g_iWhite]);
						}
					}
				} break;
			case D3DXRS_INT4:
			case D3DXRS_FLOAT4:
			case D3DXRS_BOOL:
				{
					CShaderVar *pVar(PixelShaderConstant.pShaderVar);

					if (pVar)
						pVar->Get(&g_afRegisterBuffer[PixelShaderConstant.uiRegisterIndex << 2], PixelShaderConstant.uiSize);
					else
					{
						// See if this material defines this parameter
						const IMaterialParameter *pParam(material.GetParameter(PixelShaderConstant.sName));

						if (pParam)
						{
							switch (pParam->GetType())
							{
							case MPT_FLOAT:
								g_afRegisterBuffer[PixelShaderConstant.uiRegisterIndex << 2] = pParam->GetFloat(fTime);
								break;
							case MPT_VEC2:
								MemManager.Copy(&g_afRegisterBuffer[PixelShaderConstant.uiRegisterIndex << 2], (float *)&pParam->GetVec2(fTime), sizeof(CVec2f));
								break;
							case MPT_VEC3:
								MemManager.Copy(&g_afRegisterBuffer[PixelShaderConstant.uiRegisterIndex << 2], (float *)&pParam->GetVec3(fTime), sizeof(CVec3f));
								break;
							case MPT_VEC4:
								MemManager.Copy(&g_afRegisterBuffer[PixelShaderConstant.uiRegisterIndex << 2], (float *)&pParam->GetVec4(fTime), sizeof(CVec4f));
								break;
							}
						}
						else
						{
							Console.Warn << "Shader parameter " << SingleQuoteStr(PixelShaderConstant.sName) << " not defined" << newl;
						}
					}
				} break;
			}
		}

		if (PixelShaderSlot.uiNumRegisters > 0)
			g_pd3dDevice->SetPixelShaderConstantF(0, g_afRegisterBuffer, PixelShaderSlot.uiNumRegisters);
	}

	return true;
}
#endif


/*====================
  D3D_SelectVertexShader
  ====================*/
bool	D3D_SelectVertexShader(const CMaterial &material, EMaterialPhase ePhase, int iVertexType, float fTime)
{
	PROFILE("D3D_SelectVertexShader");

	assert(material.HasPhase(ePhase));
	assert(g_iNumActivePointLights <= MAX_POINT_LIGHTS);

	g_pCurrentMaterial = &material;
	g_ePhase = ePhase;
	g_iVertexType = iVertexType;
	g_fTime = fTime;

	g_ShaderRegistry.SetNumPointLights(g_iNumActivePointLights);
	g_ShaderRegistry.SetNumBones(g_iNumActiveBones);
	g_ShaderRegistry.SetLighting(g_bLighting);
	g_ShaderRegistry.SetShadows(g_bShadows);
	g_ShaderRegistry.SetFogofWar(g_bFogofWar);
	g_ShaderRegistry.SetFog(g_bFog);
	g_ShaderRegistry.SetTexcoords(g_iTexcoords);
	g_ShaderRegistry.SetTexkill(g_bTexkill);

	const CMaterialPhase &phase = material.GetPhase(ePhase);

	D3D_SetVertexDeclaration(g_pVertexDeclarations[iVertexType]);

	// Use error shader if something went wrong
	if (g_iCurrentPixelShader == -1 ||
		!g_aPixelShaderSlots[g_iCurrentPixelShader].pShader || !g_aPixelShaderSlots[g_iCurrentPixelShader].pConstantTable)
	{
		g_iCurrentPixelShader = g_ShaderRegistry.GetPixelShaderInstance(g_hNullMeshPS);
	}

	if (phase.GetVertexShader() == -1) // a mesh we don't have a shader for
	{
		g_iCurrentVertexShader = g_ShaderRegistry.GetVertexShaderInstance(g_hNullMeshVS);
		g_iCurrentPixelShader = g_ShaderRegistry.GetPixelShaderInstance(g_hNullMeshPS);
	}
	else
	{
		g_iCurrentVertexShader = g_ShaderRegistry.GetVertexShaderInstance(phase.GetVertexShader());
	}

	// Use error shader if something went wrong
	if (g_iCurrentVertexShader == -1 || g_iCurrentPixelShader == -1 ||
		!g_aVertexShaderSlots[g_iCurrentVertexShader].pShader || !g_aVertexShaderSlots[g_iCurrentVertexShader].pConstantTable ||
		!g_aPixelShaderSlots[g_iCurrentPixelShader].pShader || !g_aPixelShaderSlots[g_iCurrentPixelShader].pConstantTable)
	{
		g_iCurrentVertexShader = g_ShaderRegistry.GetVertexShaderInstance(g_hNullMeshVS);
		g_iCurrentPixelShader = g_ShaderRegistry.GetPixelShaderInstance(g_hNullMeshPS);

		// Use even simplier error shader if something is still wrong
		if (g_iCurrentVertexShader == -1 || g_iCurrentPixelShader == -1 ||
			!g_aVertexShaderSlots[g_iCurrentVertexShader].pShader || !g_aVertexShaderSlots[g_iCurrentVertexShader].pConstantTable ||
			!g_aPixelShaderSlots[g_iCurrentPixelShader].pShader || !g_aPixelShaderSlots[g_iCurrentPixelShader].pConstantTable)
		{
			g_ShaderRegistry.SetNumPointLights(0);
			g_ShaderRegistry.SetNumBones(0);
			g_ShaderRegistry.SetLighting(false);
			g_ShaderRegistry.SetShadows(false);
			g_ShaderRegistry.SetFogofWar(false);
			g_ShaderRegistry.SetFog(false);

			g_iCurrentVertexShader = g_ShaderRegistry.GetVertexShaderInstance(g_hNullMeshVS);
			g_iCurrentPixelShader = g_ShaderRegistry.GetPixelShaderInstance(g_hNullMeshPS);

			// Fatal if something is still wrong with the error shader
			if (g_iCurrentVertexShader == -1 || g_iCurrentPixelShader == -1 ||
				!g_aVertexShaderSlots[g_iCurrentVertexShader].pShader || !g_aVertexShaderSlots[g_iCurrentVertexShader].pConstantTable ||
				!g_aPixelShaderSlots[g_iCurrentPixelShader].pShader || !g_aPixelShaderSlots[g_iCurrentPixelShader].pConstantTable)
			{
				tstring sError;

				if (g_iCurrentVertexShader == -1)
				{
					sError += (sError.empty() ? _T("") : _T(", "));
					sError += _T("g_iCurrentVertexShader invalid");
				}

				if (g_iCurrentPixelShader == -1)
				{
					sError += (sError.empty() ? _T("") : _T(", "));
					sError += _T("g_iCurrentPixelShader invalid");
				}

				if (g_iCurrentVertexShader != -1 && !g_aVertexShaderSlots[g_iCurrentVertexShader].pShader)
				{
					sError += (sError.empty() ? _T("") : _T(", "));
					sError += _T("g_aVertexShaderSlots.pShader invalid");
				}

				if (g_iCurrentVertexShader != -1 && !g_aVertexShaderSlots[g_iCurrentVertexShader].pConstantTable)
				{
					sError += (sError.empty() ? _T("") : _T(", "));
					sError += _T("g_aVertexShaderSlots.pConstantTable invalid");
				}

				if (g_iCurrentPixelShader != -1 && !g_aPixelShaderSlots[g_iCurrentPixelShader].pShader)
				{
					sError += (sError.empty() ? _T("") : _T(", "));
					sError += _T("g_aPixelShaderSlots.pShader invalid");
				}

				if (g_iCurrentPixelShader != -1 && !g_aPixelShaderSlots[g_iCurrentPixelShader].pConstantTable)
				{
					sError += (sError.empty() ? _T("") : _T(", "));
					sError += _T("g_aPixelShaderSlots.pConstantTable invalid");
				}

				EX_FATAL(_T("D3D_SelectMaterial: problem with NULL shader: ") + sError);
			}
		}
	}

	//
	// Vertex Shader Parameters
	//
	{
		PROFILE("Vertex Shader Parameters");

		SVertexShaderSlot &VertexShaderSlot(g_aVertexShaderSlots[g_iCurrentVertexShader]);

		for (uint ui(0); ui < VertexShaderSlot.uiNumConstants; ++ui)
		{
			SVertexShaderConstant &VertexShaderConstant(VertexShaderSlot.vConstant[ui]);
			
			CShaderVar *pVar(VertexShaderConstant.pShaderVar);

			if (pVar)
				pVar->Get(&g_afRegisterBuffer[VertexShaderConstant.uiRegisterIndex << 2], VertexShaderConstant.uiSize);
			else
			{
				const tstring &sName(VertexShaderConstant.sName);

				// See if this material defines this parameter
				const IMaterialParameter *pParam(material.GetParameter(sName));

				if (pParam)
				{
					switch (pParam->GetType())
					{
					case MPT_FLOAT:
						g_afRegisterBuffer[VertexShaderConstant.uiRegisterIndex << 2] = pParam->GetFloat(fTime);
						break;
					case MPT_VEC2:
						MemManager.Copy(&g_afRegisterBuffer[VertexShaderConstant.uiRegisterIndex << 2], (float *)&pParam->GetVec2(fTime), sizeof(CVec2f));
						break;
					case MPT_VEC3:
						MemManager.Copy(&g_afRegisterBuffer[VertexShaderConstant.uiRegisterIndex << 2], (float *)&pParam->GetVec3(fTime), sizeof(CVec3f));
						break;
					case MPT_VEC4:
						MemManager.Copy(&g_afRegisterBuffer[VertexShaderConstant.uiRegisterIndex << 2], (float *)&pParam->GetVec4(fTime), sizeof(CVec4f));
						break;
					}
				}
				else
				{
					Console.Warn << "Shader parameter " << SingleQuoteStr(sName) << " not defined" << newl;
				}
			}
		}

		if (VertexShaderSlot.uiNumRegisters > 0)
			g_pd3dDevice->SetVertexShaderConstantF(0, g_afRegisterBuffer, VertexShaderSlot.uiNumRegisters);
	}

	return true;
}


/*====================
  D3D_SelectPixelShader
  ====================*/
bool	D3D_SelectPixelShader(const CMaterial &material, EMaterialPhase ePhase, float fTime)
{
	PROFILE("D3D_SelectPixelShader");

	assert(material.HasPhase(ePhase));
	assert(g_iNumActivePointLights <= MAX_POINT_LIGHTS);

	g_pCurrentMaterial = &material;
	g_ePhase = ePhase;
	g_fTime = fTime;

	g_ShaderRegistry.SetNumPointLights(g_iNumActivePointLights);
	g_ShaderRegistry.SetNumBones(g_iNumActiveBones);
	g_ShaderRegistry.SetLighting(g_bLighting);
	g_ShaderRegistry.SetShadows(g_bShadows);
	g_ShaderRegistry.SetFogofWar(g_bFogofWar);
	g_ShaderRegistry.SetFog(g_bFog);
	g_ShaderRegistry.SetTexcoords(g_iTexcoords);
	g_ShaderRegistry.SetTexkill(g_bTexkill);

	const CMaterialPhase &phase = material.GetPhase(ePhase);

	if (phase.GetPixelShader() == -1) // a mesh we don't have a shader for
	{
		g_iCurrentPixelShader = g_ShaderRegistry.GetPixelShaderInstance(g_hNullMeshPS);
	}
	else
	{
		g_iCurrentPixelShader = g_ShaderRegistry.GetPixelShaderInstance(phase.GetPixelShader());
	}

	// turn off unused texture stages
	for (uint ui(phase.GetNumSamplers()); ui < g_dwNumSamplers; ++ui)
		D3D_SetTexture(ui, NULL);

	D3D_SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	D3D_SetRenderState(D3DRS_ALPHATESTENABLE, vid_alphaTest ? phase.GetAlphaTest() : FALSE);
	D3D_SetRenderState(D3DRS_ZWRITEENABLE, phase.GetDepthWrite() ? TRUE : FALSE);

	D3D_SetRenderState(D3DRS_ZENABLE, phase.GetDepthRead() ? TRUE : FALSE);

	if (ePhase == PHASE_COLOR || ePhase == PHASE_FADE)
	{
		uint uiWriteMask(0);

		if (phase.GetColorWrite())
			uiWriteMask |= D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE;
		if (phase.GetAlphaWrite())
			uiWriteMask |= D3DCOLORWRITEENABLE_ALPHA;

		D3D_SetRenderState(D3DRS_COLORWRITEENABLE, uiWriteMask);
	}

	if (phase.GetTranslucent())
		D3D_SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	else
		D3D_SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

	D3D_SetRenderState(D3DRS_SRCBLEND, blendmodeMapping[phase.GetSrcBlend()]);
	D3D_SetRenderState(D3DRS_DESTBLEND, blendmodeMapping[phase.GetDstBlend()]);

	if (ePhase == PHASE_SHADOW && (vid_shadowBackface || g_Shadowmap.GetType() == SHADOWMAP_R32F))
	{
		if (g_bInvertedProjection)
			D3D_SetRenderState(D3DRS_CULLMODE, cullmodeMappingInvBack[phase.GetCullMode()]);
		else
			D3D_SetRenderState(D3DRS_CULLMODE, cullmodeMappingBack[phase.GetCullMode()]);
	}
	else
	{
		if (g_bInvertedProjection)
			D3D_SetRenderState(D3DRS_CULLMODE, cullmodeMappingInv[phase.GetCullMode()]);
		else
			D3D_SetRenderState(D3DRS_CULLMODE, cullmodeMapping[phase.GetCullMode()]);
	}

	if (ePhase == PHASE_SHADOW && g_Shadowmap.GetType() == SHADOWMAP_DEPTH)
	{
		if (g_DeviceCaps.bSlopeScaledDepthBias)
			D3D_SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, D3D_DWORD(vid_shadowSlopeBias + vid_shadowmapFilterWidth));
		if (g_DeviceCaps.bDepthBias)
			D3D_SetRenderState(D3DRS_DEPTHBIAS, D3D_DWORD(vid_shadowDepthBias / 16777215.f));
	}
	else
	{
		if (g_DeviceCaps.bSlopeScaledDepthBias)
			D3D_SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, phase.GetDepthSlopeBias() ? D3D_DWORD(-1.0f) : D3D_DWORD(0.0));
		if (g_DeviceCaps.bDepthBias)
			D3D_SetRenderState(D3DRS_DEPTHBIAS, D3D_DWORD(phase.GetDepthBias() * vid_depthBiasScale));
	}


	//
	// Pixel Shader Parameters
	//
	{
		PROFILE("Pixel Shader Parameters");

		SPixelShaderSlot &PixelShaderSlot(g_aPixelShaderSlots[g_iCurrentPixelShader]);

		for (uint ui(0); ui < PixelShaderSlot.uiNumConstants; ++ui)
		{
			SPixelShaderConstant &PixelShaderConstant(PixelShaderSlot.vConstant[ui]);

			switch (PixelShaderConstant.eRegisterSet)
			{
			case D3DXRS_SAMPLER:
				{
					// Find sampler in the shader
					int iSamplerIndex(phase.GetSamplerIndex(PixelShaderConstant.sName));

					if (iSamplerIndex != -1)
					{
						D3D_SetSampler(PixelShaderConstant.uiRegisterIndex, phase.GetSampler(iSamplerIndex), PixelShaderConstant.uiSubTexture);
					}
					else
					{
						CShaderSampler *pSampler(PixelShaderConstant.pShaderSamplers);

						if (pSampler)
						{
							pSampler->Get(PixelShaderConstant.uiRegisterIndex);
						}
						else // No valid sampler found, so use white
						{
							int iStageIndex(PixelShaderConstant.uiRegisterIndex);

							D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
							D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
							D3D_SetSamplerState(iStageIndex, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
							D3D_SetSamplerState(iStageIndex, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
							D3D_SetSamplerState(iStageIndex, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
							D3D_SetSamplerState(iStageIndex, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
							D3D_SetTexture(iStageIndex, g_pTextures[g_iWhite]);
						}
					}
				} break;
			case D3DXRS_INT4:
			case D3DXRS_FLOAT4:
			case D3DXRS_BOOL:
				{
					CShaderVar *pVar(PixelShaderConstant.pShaderVar);

					if (pVar)
						pVar->Get(&g_afRegisterBuffer[PixelShaderConstant.uiRegisterIndex << 2], PixelShaderConstant.uiSize);
					else
					{
						// See if this material defines this parameter
						const IMaterialParameter *pParam(material.GetParameter(PixelShaderConstant.sName));

						if (pParam)
						{
							switch (pParam->GetType())
							{
							case MPT_FLOAT:
								g_afRegisterBuffer[PixelShaderConstant.uiRegisterIndex << 2] = pParam->GetFloat(fTime);
								break;
							case MPT_VEC2:
								MemManager.Copy(&g_afRegisterBuffer[PixelShaderConstant.uiRegisterIndex << 2], (float *)&pParam->GetVec2(fTime), sizeof(CVec2f));
								break;
							case MPT_VEC3:
								MemManager.Copy(&g_afRegisterBuffer[PixelShaderConstant.uiRegisterIndex << 2], (float *)&pParam->GetVec3(fTime), sizeof(CVec3f));
								break;
							case MPT_VEC4:
								MemManager.Copy(&g_afRegisterBuffer[PixelShaderConstant.uiRegisterIndex << 2], (float *)&pParam->GetVec4(fTime), sizeof(CVec4f));
								break;
							}
						}
						else
						{
							Console.Warn << "Shader parameter " << SingleQuoteStr(PixelShaderConstant.sName) << " not defined" << newl;
						}
					}
				} break;
			}
		}

		if (PixelShaderSlot.uiNumRegisters > 0)
			g_pd3dDevice->SetPixelShaderConstantF(0, g_afRegisterBuffer, PixelShaderSlot.uiNumRegisters);
	}

	return true;
}


/*====================
  D3D_UpdateShaderTexture
  ====================*/
bool	D3D_UpdateShaderTexture(int iStageIndex, ResHandle hTexture, uint uiSubTexture)
{
	if (iStageIndex == -1)
		return false;

	CTexture *pTexture(g_ResourceManager.GetTexture(hTexture));
	int iTexture(pTexture != NULL ? uiSubTexture > 0 ? pTexture->GetIndex2() : pTexture->GetIndex() : -1);

	D3D_SetTexture(iStageIndex, iTexture == -1 ? NULL : g_pTextures[iTexture]);

	return true;
}


/*====================
  D3D_UpdateShaderTexture
  ====================*/
bool	D3D_UpdateShaderTexture(const tstring &sSampler, ResHandle hTexture)
{
	SPixelShaderSlot &PixelShaderSlot(g_aPixelShaderSlots[g_iCurrentPixelShader]);

	for (uint ui(0); ui < PixelShaderSlot.uiNumConstants; ++ui)
	{
		SPixelShaderConstant &PixelShaderConstant(PixelShaderSlot.vConstant[ui]);

		if (PixelShaderConstant.eRegisterSet != D3DXRS_SAMPLER ||
			PixelShaderConstant.sName != sSampler)
			continue;

		int iStageIndex(PixelShaderConstant.uiRegisterIndex);
		CTexture *pTexture = g_ResourceManager.GetTexture(hTexture);

		if ((gfx_textures == 1 || gfx_textures == 3) && sSampler.compare(0, 7, _T("diffuse")) == 0)
		{
			int iTexture = CProceduralRegistry::GetInstance()->GetTextureIndex(_T("white"));

			D3D_SetTexture(iStageIndex, iTexture == -1 ? NULL : g_pTextures[iTexture]);
		}
		else if ((gfx_textures == 2 || gfx_textures == 3) && sSampler.compare(0, 9, _T("normalmap")) == 0)
		{
			int iTexture = CProceduralRegistry::GetInstance()->GetTextureIndex(_T("flat_dull"));

			D3D_SetTexture(iStageIndex, iTexture == -1 ? NULL : g_pTextures[iTexture]);
		}
		else if (gfx_textures == 4 && sSampler.compare(0, 7, _T("diffuse")) == 0)
		{
			int iTexture = CProceduralRegistry::GetInstance()->GetTextureIndex(_T("size_hint"));

			D3D_SetTexture(iStageIndex, iTexture == -1 ? NULL : g_pTextures[iTexture]);
		}
		else
		{
			int iTexture = pTexture ? pTexture->GetIndex() : -1;

			D3D_SetTexture(iStageIndex, iTexture == -1 ? NULL : g_pTextures[iTexture]);
		}
	}

	return false;
}


/*====================
  D3D_GetSamplerStageIndex
  ====================*/
int		D3D_GetSamplerStageIndex(const tstring &sSampler, uint uiSubTexture)
{
	if (TStringCompare(sSampler, _T("diffuse")) == 0)
		return 0;
	else if (TStringCompare(sSampler, _T("diffuse0")) == 0)
		return 0;
	else if (TStringCompare(sSampler, _T("image")) == 0)
		return 0;
	else
		return -1;
}


/*====================
  D3D_UpdateShaderTextureIndex
  ====================*/
bool	D3D_UpdateShaderTextureIndex(const tstring &sSampler, int iTexture)
{
	SPixelShaderSlot &PixelShaderSlot(g_aPixelShaderSlots[g_iCurrentPixelShader]);

	for (uint ui(0); ui < PixelShaderSlot.uiNumConstants; ++ui)
	{
		SPixelShaderConstant &PixelShaderConstant(PixelShaderSlot.vConstant[ui]);

		if (PixelShaderConstant.eRegisterSet != D3DXRS_SAMPLER ||
			PixelShaderConstant.sName != sSampler)
			continue;

		int iStageIndex(PixelShaderConstant.uiRegisterIndex);

		if ((gfx_textures == 1 || gfx_textures == 3) && sSampler.compare(0, 7, _T("diffuse")) == 0)
		{
			int iTexture = CProceduralRegistry::GetInstance()->GetTextureIndex(_T("white"));

			D3D_SetTexture(iStageIndex, iTexture == -1 ? NULL : g_pTextures[iTexture]);
		}
		else if ((gfx_textures == 2 || gfx_textures == 3) && sSampler.compare(0, 9, _T("normalmap")) == 0)
		{
			int iTexture = CProceduralRegistry::GetInstance()->GetTextureIndex(_T("flat_dull"));

			D3D_SetTexture(iStageIndex, iTexture == -1 ? NULL : g_pTextures[iTexture]);
		}
		else if (gfx_textures == 4 && sSampler.compare(0, 7, _T("diffuse")) == 0)
		{
			int iTexture = CProceduralRegistry::GetInstance()->GetTextureIndex(_T("size_hint"));

			D3D_SetTexture(iStageIndex, iTexture == -1 ? NULL : g_pTextures[iTexture]);
		}
		else
		{
			D3D_SetTexture(iStageIndex, iTexture == -1 ? NULL : g_pTextures[iTexture]);
		}

		return true;
	}

	return false;
}


/*====================
  D3D_UpdateShaderTextureIndex
  ====================*/
bool	D3D_UpdateShaderTextureIndex(int iStageIndex, int iTexture)
{
	if (iStageIndex == -1)
		return false;

	D3D_SetTexture(iStageIndex, iTexture == -1 ? NULL : g_pTextures[iTexture]);

	return true;
}


/*====================
  D3D_UpdateVertexShaderParams
  ====================*/
bool	D3D_UpdateVertexShaderParams(const CMaterial &material, float fTime)
{
	PROFILE("D3D_UpdateVertexShaderParams");

	assert(g_pCurrentMaterial == &material);

	//
	// Vertex Shader Parameters
	//
	{
	}

	return true;
}


/*====================
  D3D_UpdatePixelShaderParams

  This does not update samplers!
  ====================*/
bool	D3D_UpdatePixelShaderParams(const CMaterial &material, float fTime)
{
	PROFILE("D3D_UpdatePixelShaderParams");

	assert(g_pCurrentMaterial == &material);

	//
	// Pixel Shader Parameters
	//
	{
	}

	return true;
}


/*====================
  D3D_SetSrcBlendMode
  ====================*/
void	D3D_SetSrcBlendMode(eBlendMode eSrcBlend)
{
	D3D_SetRenderState(D3DRS_SRCBLEND, blendmodeMapping[eSrcBlend]);
}


/*====================
  D3D_SetDstBlendMode
  ====================*/
void	D3D_SetDstBlendMode(eBlendMode eDstBlend)
{
	D3D_SetRenderState(D3DRS_DESTBLEND, blendmodeMapping[eDstBlend]);
}


/*====================
  D3D_SelectMaterial
  ====================*/
bool	D3D_SelectMaterial(const CMaterial &material, EMaterialPhase ePhase, int iVertexType, float fTime, bool bDepthFirst)
{
	PROFILE("D3D_SelectMaterialSoftware");

	assert(material.HasPhase(ePhase));
	assert(!(ePhase == PHASE_DEPTH && !bDepthFirst));
	assert(g_iNumActivePointLights <= MAX_POINT_LIGHTS);

	g_pCurrentMaterial = &material;
	g_ePhase = ePhase;
	g_iVertexType = iVertexType;
	g_fTime = fTime;

	// Make the back of the deque matches our current material state
	SMaterialState	oMaterialState;

	oMaterialState.pMaterial = &material;
	oMaterialState.ePhase = ePhase;
	oMaterialState.iVertexType = iVertexType;
	oMaterialState.fTime = fTime;
	oMaterialState.bDepthFirst = bDepthFirst;

	g_MaterialState[g_uiMaterialState] = oMaterialState;

	g_ShaderRegistry.SetNumPointLights(g_iNumActivePointLights);
	g_ShaderRegistry.SetNumBones(g_iNumActiveBones);
	g_ShaderRegistry.SetLighting(g_bLighting);
	g_ShaderRegistry.SetShadows(g_bShadows);
	g_ShaderRegistry.SetFogofWar(g_bFogofWar);

	const CMaterialPhase &phase = material.GetPhase(ePhase);

	D3D_SetVertexDeclaration(g_pVertexDeclarations[iVertexType]);

	int iNumSamplers(MIN(1, phase.GetNumSamplers()));

	if (dword(phase.GetNumSamplers()) < g_dwNumSamplers)
		D3D_SetTexture(iNumSamplers, NULL); // turn off unused texture stages

	if (bDepthFirst && material.HasPhase(PHASE_DEPTH))
	{
		switch (ePhase)
		{
		case PHASE_COLOR:
			D3D_SetRenderState(D3DRS_ZFUNC, D3DCMP_EQUAL);
			D3D_SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
			D3D_SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
			break;
		case PHASE_DEPTH:
			D3D_SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);
			D3D_SetRenderState(D3DRS_ALPHATESTENABLE, vid_alphaTest ? phase.GetAlphaTest() : FALSE);
			D3D_SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			break;
		default:
		case PHASE_SHADOW:
			D3D_SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
			D3D_SetRenderState(D3DRS_ALPHATESTENABLE, vid_alphaTest ? phase.GetAlphaTest() : FALSE);
			D3D_SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			break;
		}
	}
	else
	{
		D3D_SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
		D3D_SetRenderState(D3DRS_ALPHATESTENABLE, vid_alphaTest ? phase.GetAlphaTest() : FALSE);
		D3D_SetRenderState(D3DRS_ZWRITEENABLE, phase.GetDepthWrite() ? TRUE : FALSE);
	}

	D3D_SetRenderState(D3DRS_ZENABLE, phase.GetDepthRead() ? TRUE : FALSE);

	if (ePhase == PHASE_COLOR || ePhase == PHASE_FADE)
	{
		uint uiWriteMask(0);

		if (phase.GetColorWrite())
			uiWriteMask |= D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE;
		if (phase.GetAlphaWrite())
			uiWriteMask |= D3DCOLORWRITEENABLE_ALPHA;

		D3D_SetRenderState(D3DRS_COLORWRITEENABLE, uiWriteMask);
	}

	if (phase.GetTranslucent())
		D3D_SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	else
		D3D_SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

	D3D_SetRenderState(D3DRS_SRCBLEND, blendmodeMapping[phase.GetSrcBlend()]);
	D3D_SetRenderState(D3DRS_DESTBLEND, blendmodeMapping[phase.GetDstBlend()]);

	if (ePhase == PHASE_SHADOW && (vid_shadowBackface || g_Shadowmap.GetType() == SHADOWMAP_R32F))
	{
		if (g_bInvertedProjection)
			D3D_SetRenderState(D3DRS_CULLMODE, cullmodeMappingInvBack[phase.GetCullMode()]);
		else
			D3D_SetRenderState(D3DRS_CULLMODE, cullmodeMappingBack[phase.GetCullMode()]);
	}
	else
	{
		if (g_bInvertedProjection)
			D3D_SetRenderState(D3DRS_CULLMODE, cullmodeMappingInv[phase.GetCullMode()]);
		else
			D3D_SetRenderState(D3DRS_CULLMODE, cullmodeMapping[phase.GetCullMode()]);
	}

	if (ePhase == PHASE_SHADOW && g_Shadowmap.GetType() == SHADOWMAP_DEPTH)
	{
		if (g_DeviceCaps.bSlopeScaledDepthBias)
			D3D_SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, D3D_DWORD(vid_shadowSlopeBias + vid_shadowmapFilterWidth));
		if (g_DeviceCaps.bDepthBias)
			D3D_SetRenderState(D3DRS_DEPTHBIAS, D3D_DWORD(vid_shadowDepthBias / 16777215.f));
	}
	else
	{
		if (g_DeviceCaps.bSlopeScaledDepthBias)
			D3D_SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, phase.GetDepthSlopeBias() ? D3D_DWORD(-1.0f) : D3D_DWORD(0.0));
		if (g_DeviceCaps.bDepthBias)
			D3D_SetRenderState(D3DRS_DEPTHBIAS, D3D_DWORD(phase.GetDepthBias() * vid_depthBiasScale));
	}

	g_pd3dDevice->SetTransform(D3DTS_WORLDMATRIX(0), &g_mWorld);

	if (phase.GetPixelShaderName() == _T("gui"))
		D3D_SetSampler(0, phase.GetSampler(_T("image")), 0);
	else if (phase.GetSamplerIndex(_T("diffuse")) != -1)
		D3D_SetSampler(0, phase.GetSampler(_T("diffuse")), 0);
	else if (phase.GetSamplerIndex(_T("diffuse0")) != -1)
		D3D_SetSampler(0, phase.GetSampler(_T("diffuse0")), 0);

	if (g_bLighting)
	{
		D3D_SetRenderState(D3DRS_LIGHTING, TRUE);
		D3D_SetRenderState(D3DRS_AMBIENT, D3DCOLOR_ARGB
		(
			0,
			CLAMP(INT_ROUND(g_v3Ambient[R] * 255.0f), 0, 255),
			CLAMP(INT_ROUND(g_v3Ambient[G] * 255.0f), 0, 255),
			CLAMP(INT_ROUND(g_v3Ambient[B] * 255.0f), 0, 255)
		));

		D3DLIGHT9 light;
		ZeroMemory(&light, sizeof(light));
		light.Type = D3DLIGHT_DIRECTIONAL;
		
		light.Ambient.r = 0.0f;
		light.Ambient.g = 0.0f;
		light.Ambient.b = 0.0f;
		light.Ambient.a = 1.0f;

		light.Diffuse.r = g_v3SunColor[R];
		light.Diffuse.g = g_v3SunColor[G];
		light.Diffuse.b = g_v3SunColor[B];
		light.Diffuse.a = 1.0f;

		light.Direction.x = -SceneManager.GetSunPos().x;
		light.Direction.y = -SceneManager.GetSunPos().y;
		light.Direction.z = -SceneManager.GetSunPos().z;

		g_pd3dDevice->SetLight(0, &light);
		g_pd3dDevice->LightEnable(0, TRUE);
	}
	else
	{
		D3D_SetRenderState(D3DRS_LIGHTING, FALSE);
	}

	D3DMATERIAL9 mtrl;
	ZeroMemory(&mtrl, sizeof(mtrl));
	mtrl.Diffuse.r = mtrl.Ambient.r = material.GetDiffuseColor()[R];
	mtrl.Diffuse.g = mtrl.Ambient.g = material.GetDiffuseColor()[G];
	mtrl.Diffuse.b = mtrl.Ambient.b = material.GetDiffuseColor()[B];
	mtrl.Diffuse.a = mtrl.Ambient.a = material.GetDiffuseColor()[A];
	g_pd3dDevice->SetMaterial(&mtrl);

	return true;
}
