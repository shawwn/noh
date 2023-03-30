// (C)2008 S2 Games
// c_gfxmaterials.cpp
//
// Materials
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_gfxmaterials.h"

#include "c_gfxshaders.h"
#include "c_gfxtextures.h"
#include "c_proceduralregistry.h"
#include "c_shaderregistry.h"
#include "c_shadersampler.h"
#include "c_shadervar.h"
#include "c_shadowmap.h"

#include "../k2/c_materialparameter.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
SINGLETON_INIT(CGfxMaterials)
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CGfxMaterials *GfxMaterials(CGfxMaterials::GetInstance());

CMaterial	g_SimpleMaterial3D(TSNULL, _T("simple_material"));
CMaterial	g_SimpleMaterial3DLit(TSNULL, _T("simple_material_lit"));
CMaterial	g_SimpleMaterial3DColored(TSNULL, _T("simple_material_colored"));
CMaterial	g_SimpleMaterial3DColoredBias(TSNULL, _T("simple_material_colored_bias"));
CMaterial	g_MaterialGUI(TSNULL, _T("gui_material"));
CMaterial	g_MaterialGUIGrayScale(TSNULL, _T("gui_grayscale_material"));
CMaterial	g_MaterialGUIBlur(TSNULL, _T("gui_blur_material"));

CVAR_FLOATF	(vid_alphaTestRef,	90, CVAR_SAVECONFIG);
CVAR_FLOAT	(vid_depthBiasScale, -2.0f);
CVAR_BOOLF	(vid_shaderTexkill,				false,	CVAR_SAVECONFIG);
CVAR_BOOL	(vid_shaderTexkillColorOnly,	false);

const CMaterial	*g_pCurrentMaterial;
EMaterialPhase	g_eCurrentPhase;
int				g_iCurrentVertexType;
float			g_fCurrentTime;

int				g_iCurrentVertexShader(-1);
int				g_iCurrentPixelShader(-1);
int				g_iCurrentShaderProgram(-1);

bool			g_bCamShadows;
bool			g_bCamFogofWar;
bool			g_bCamFog;

bool			g_bLighting;
bool			g_bShadows;
bool			g_bFogofWar;
bool			g_bFog;
CVec3f			g_v3SunColor;
CVec3f			g_v3Ambient;
CVec3f			g_vPointLightPosition[MAX_POINT_LIGHTS];
CVec3f			g_vPointLightColor[MAX_POINT_LIGHTS];
float			g_fPointLightFalloffStart[MAX_POINT_LIGHTS];
float			g_fPointLightFalloffEnd[MAX_POINT_LIGHTS];
int				g_iNumActivePointLights;
bool			g_bObjectColor;
CVec4f			g_vObjectColor;
bool			g_bTexkill;

int blendmodeMapping[] =
{
	GL_ZERO,
	GL_ONE,
	GL_SRC_COLOR,
	GL_DST_COLOR,
	GL_ONE_MINUS_SRC_COLOR,
	GL_ONE_MINUS_DST_COLOR,
	GL_SRC_ALPHA,
	GL_DST_ALPHA,
	GL_ONE_MINUS_SRC_ALPHA,
	GL_ONE_MINUS_DST_ALPHA
};
//=============================================================================

/*====================
  CGfxMaterials::~CGfxMaterials
  ====================*/
CGfxMaterials::~CGfxMaterials()
{
}


/*====================
  CGfxMaterials::CGfxMaterials
  ====================*/
CGfxMaterials::CGfxMaterials()
{
}


/*====================
  CGfxMaterials::Init
  ====================*/
void	CGfxMaterials::Init()
{
	for (int i(0); i < g_iMaxTextureImageUnits; ++i)
	{
		m_aTextureUnits[i].eTextureType = GL_NONE;
	}

	//
	// Setup "simple" 3D shader for lines, boxes, and such
	//
	{
		g_SimpleMaterial3D.SetGlossiness(16.0f);
		g_SimpleMaterial3D.SetSpecularLevel(1.0f);
		g_SimpleMaterial3D.SetBumpLevel(1.0f);
		g_SimpleMaterial3D.SetReflect(0.0f);
		g_SimpleMaterial3D.SetDiffuseColor(CVec3f(1.0f, 1.0f, 1.0f));
		g_SimpleMaterial3D.SetOpacity(1.0f);

		//
		// Phase 0 (depth)
		//
		CMaterialPhase	phaseDepth(PHASE_DEPTH, _T("mesh_depth"), _T("mesh_depth"), BLEND_ONE, BLEND_ZERO, CULL_BACK, 0, 0, 0.0f, PHASE_DEFAULT);
		g_SimpleMaterial3D.AddPhase(phaseDepth);

		//
		// Phase 1 (color)
		//
		CMaterialPhase	phaseColor(PHASE_COLOR, _T("simple"), _T("simple"), BLEND_ONE, BLEND_ZERO, CULL_BACK, 0, 0, 0.0f, PHASE_DEFAULT);
		g_SimpleMaterial3D.AddPhase(phaseColor);
	}


	//
	// Setup "simple" 3D shader with lighting
	//
	{
		g_SimpleMaterial3DLit.SetGlossiness(16.0f);
		g_SimpleMaterial3DLit.SetSpecularLevel(1.0f);
		g_SimpleMaterial3DLit.SetBumpLevel(1.0f);
		g_SimpleMaterial3DLit.SetReflect(0.0f);
		g_SimpleMaterial3DLit.SetDiffuseColor(CVec3f(1.0f, 1.0f, 1.0f));
		g_SimpleMaterial3DLit.SetOpacity(1.0f);

		//
		// Phase 0 (depth)
		//
		CMaterialPhase	phaseDepth(PHASE_DEPTH, _T("mesh_depth"), _T("mesh_depth"), BLEND_ONE, BLEND_ZERO, CULL_BACK, 0, 0, 0.0f, PHASE_DEFAULT);
		g_SimpleMaterial3DLit.AddPhase(phaseDepth);

		//
		// Phase 1 (color)
		//
		CMaterialPhase	phaseColor(PHASE_COLOR, _T("simple_color_flat"), _T("simple_color_flat"), BLEND_ONE, BLEND_ZERO, CULL_BACK, 0, 0, 0.0f, PHASE_DEFAULT);
		g_SimpleMaterial3DLit.AddPhase(phaseColor);
	}


	//
	// Setup "simple" 3D shader for lines, boxes, and such
	//
	{
		g_SimpleMaterial3DColored.SetGlossiness(16.0f);
		g_SimpleMaterial3DColored.SetSpecularLevel(1.0f);
		g_SimpleMaterial3DColored.SetBumpLevel(1.0f);
		g_SimpleMaterial3DColored.SetReflect(0.0f);
		g_SimpleMaterial3DColored.SetDiffuseColor(CVec3f(1.0f, 1.0f, 1.0f));
		g_SimpleMaterial3DColored.SetOpacity(1.0f);

		//
		// Phase 0 (color)
		//
		CMaterialPhase	phaseColor(PHASE_COLOR, _T("line"), _T("line"), BLEND_ONE, BLEND_ZERO, CULL_BACK, 0, 0, 0.0f, PHASE_DEFAULT);
		g_SimpleMaterial3DColored.AddPhase(phaseColor);
	}

	//
	// Setup biased 3D shader for lines, boxes, and such
	//
	{
		g_SimpleMaterial3DColoredBias.SetGlossiness(16.0f);
		g_SimpleMaterial3DColoredBias.SetSpecularLevel(1.0f);
		g_SimpleMaterial3DColoredBias.SetBumpLevel(1.0f);
		g_SimpleMaterial3DColoredBias.SetReflect(0.0f);
		g_SimpleMaterial3DColoredBias.SetDiffuseColor(CVec3f(1.0f, 1.0f, 1.0f));
		g_SimpleMaterial3DColoredBias.SetOpacity(1.0f);

		//
		// Phase 0 (color)
		//
		CMaterialPhase	phaseColor(PHASE_COLOR, _T("line"), _T("line"), BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA, CULL_BACK, 2, 0, 0.0f, PHASE_TRANSLUCENT | PHASE_COLOR_WRITE | PHASE_ALPHA_WRITE);
		g_SimpleMaterial3DColoredBias.AddPhase(phaseColor);
	}

	//
	// Setup default GUI material
	//
	{
		g_MaterialGUI.SetGlossiness(16.0f);
		g_MaterialGUI.SetSpecularLevel(1.0f);
		g_MaterialGUI.SetBumpLevel(1.0f);
		g_MaterialGUI.SetReflect(0.0f);
		g_MaterialGUI.SetDiffuseColor(CVec3f(1.0f, 1.0f, 1.0f));
		g_MaterialGUI.SetOpacity(1.0f);

		//
		// Phase 0 (color)
		//
		CMaterialPhase	phaseColor(PHASE_COLOR, _T("gui"), _T("gui"), BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA, CULL_BACK, 0, 0, 0.0f, PHASE_DEFAULT_GUI);
		CMaterialSampler samplerDiffuse(_T("image"), 15, 0.0f, 0.0f, 1.0f, 1.0f, 0, _T(""), TEXTURE_2D, 0);
		phaseColor.AddSampler(samplerDiffuse);

		g_MaterialGUI.AddPhase(phaseColor);
	}

	
	//
	// Setup grayscale GUI material
	//
	{
		g_MaterialGUIGrayScale.SetGlossiness(16.0f);
		g_MaterialGUIGrayScale.SetSpecularLevel(1.0f);
		g_MaterialGUIGrayScale.SetBumpLevel(1.0f);
		g_MaterialGUIGrayScale.SetReflect(0.0f);
		g_MaterialGUIGrayScale.SetDiffuseColor(CVec3f(1.0f, 1.0f, 1.0f));
		g_MaterialGUIGrayScale.SetOpacity(1.0f);

		//
		// Phase 0 (color)
		//
		CMaterialPhase	phaseColor(PHASE_COLOR, _T("gui"), _T("gui_grayscale"), BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA, CULL_BACK, 0, 0, 0.0f, PHASE_DEFAULT_GUI);
		CMaterialSampler samplerDiffuse(_T("image"), 15, 0.0f, 0.0f, 1.0f, 1.0f, 0, _T(""), TEXTURE_2D, 0);
		phaseColor.AddSampler(samplerDiffuse);

		g_MaterialGUIGrayScale.AddPhase(phaseColor);
	}

	//
	// Setup blur GUI material
	//
	{
		g_MaterialGUIBlur.SetGlossiness(16.0f);
		g_MaterialGUIBlur.SetSpecularLevel(1.0f);
		g_MaterialGUIBlur.SetBumpLevel(1.0f);
		g_MaterialGUIBlur.SetReflect(0.0f);
		g_MaterialGUIBlur.SetDiffuseColor(CVec3f(1.0f, 1.0f, 1.0f));
		g_MaterialGUIBlur.SetOpacity(1.0f);

		//
		// Phase 0 (color)
		//
		CMaterialPhase	phaseColor(PHASE_COLOR, _T("gui"), _T("gui_blur"), BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA, CULL_BACK, 0, 0, 0.0f, PHASE_DEFAULT_GUI);
		CMaterialSampler samplerDiffuse(_T("image"), 15, 0.0f, 0.0f, 1.0f, 1.0f, 0, _T(""), TEXTURE_2D, 0);
		phaseColor.AddSampler(samplerDiffuse);

		g_MaterialGUIBlur.AddPhase(phaseColor);
	}
}


/*====================
  CGfxMaterials::SelectMaterial
  ====================*/
bool	CGfxMaterials::SelectMaterial(const CMaterial &material, EMaterialPhase ePhase, float fTime, bool bDepthFirst)
{
	PROFILE("CGfxMaterials::SelectMaterial");

	assert(material.HasPhase(ePhase));
	assert(!(ePhase == PHASE_DEPTH && !bDepthFirst));
	assert(g_iNumActivePointLights <= MAX_POINT_LIGHTS);

	m_cMaterialState.pMaterial = &material;
	m_cMaterialState.ePhase = ePhase;
	m_cMaterialState.iPass = material.GetPass();

	g_pCurrentMaterial = &material;
	g_eCurrentPhase = ePhase;
	g_fCurrentTime = fTime;

	g_ShaderRegistry.SetNumPointLights(g_iNumActivePointLights);
	g_ShaderRegistry.SetNumBones(g_iNumActiveBones);
	g_ShaderRegistry.SetLighting(g_bLighting);
	g_ShaderRegistry.SetShadows(g_bShadows);
	g_ShaderRegistry.SetFogofWar(g_bFogofWar);
	g_ShaderRegistry.SetFog(g_bFog);
	g_ShaderRegistry.SetTexkill(g_bTexkill);

	const CMaterialPhase &phase(material.GetPhase(ePhase));

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
	if (g_iCurrentVertexShader == -1 || !g_aVertexShaderSlots[g_iCurrentVertexShader].uiShader)
	{
		g_iCurrentVertexShader = g_ShaderRegistry.GetVertexShaderInstance(g_hNullMeshVS);
		g_iCurrentPixelShader = g_ShaderRegistry.GetPixelShaderInstance(g_hNullMeshPS);
	}
	else
	{
		if (phase.GetPixelShader() == -1) // a mesh we don't have a shader for
		{
			g_iCurrentPixelShader = g_ShaderRegistry.GetPixelShaderInstance(g_hNullMeshPS);
		}
		else
		{
			g_iCurrentPixelShader = g_ShaderRegistry.GetPixelShaderInstance(phase.GetPixelShader());
		}

		// Use error shader if something went wrong
		if (g_iCurrentPixelShader == -1 || !g_aPixelShaderSlots[g_iCurrentPixelShader].uiShader)
		{
			g_iCurrentPixelShader = g_ShaderRegistry.GetPixelShaderInstance(g_hNullMeshPS);
		}
	}

	g_iCurrentShaderProgram = g_ShaderRegistry.GetShaderProgramInstance(g_iCurrentVertexShader, g_iCurrentPixelShader);

	// Use error shader if something went wrong
	if (g_iCurrentShaderProgram == -1 || !g_aShaderProgramSlots[g_iCurrentShaderProgram].uiProgram)
	{
		g_iCurrentVertexShader = g_ShaderRegistry.GetVertexShaderInstance(g_hNullMeshVS);
		g_iCurrentPixelShader = g_ShaderRegistry.GetPixelShaderInstance(g_hNullMeshPS);
		g_iCurrentShaderProgram = g_ShaderRegistry.GetShaderProgramInstance(g_iCurrentVertexShader, g_iCurrentPixelShader);

		// Use even simplier error shader if something is still wrong
		if (g_iCurrentShaderProgram == -1 || !g_aShaderProgramSlots[g_iCurrentShaderProgram].uiProgram)
		{
			g_ShaderRegistry.SetNumPointLights(0);
			g_ShaderRegistry.SetNumBones(0);
			g_ShaderRegistry.SetLighting(false);
			g_ShaderRegistry.SetShadows(false);
			g_ShaderRegistry.SetFogofWar(false);
			g_ShaderRegistry.SetFog(false);

			g_iCurrentVertexShader = g_ShaderRegistry.GetVertexShaderInstance(g_hNullMeshVS);
			g_iCurrentPixelShader = g_ShaderRegistry.GetPixelShaderInstance(g_hNullMeshPS);
			g_iCurrentShaderProgram = g_ShaderRegistry.GetShaderProgramInstance(g_iCurrentVertexShader, g_iCurrentPixelShader);

			// Use fixed function is something is still wrong
			if (g_iCurrentShaderProgram == -1 || !g_aShaderProgramSlots[g_iCurrentShaderProgram].uiProgram)
				g_iCurrentShaderProgram = -1;
		}
	}

	uint uiProgram(0);

	if (g_iCurrentShaderProgram != -1)
		uiProgram = g_aShaderProgramSlots[g_iCurrentShaderProgram].uiProgram;

	glUseProgramObjectARB(uiProgram);

	glBlendFunc(blendmodeMapping[phase.GetSrcBlend()], blendmodeMapping[phase.GetDstBlend()]);
	glAlphaFunc(GL_GEQUAL, vid_alphaTestRef / 255.0f);

	if (!(vid_shaderTexkill && (!vid_shaderTexkillColorOnly || ePhase == PHASE_COLOR)) && phase.GetAlphaTest())
		glEnable(GL_ALPHA_TEST);
	else
		glDisable(GL_ALPHA_TEST);
	
	if (phase.GetDepthWrite())
		glDepthMask(GL_TRUE);
	else
		glDepthMask(GL_FALSE);
	
	if (phase.GetDepthRead())
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	if (ePhase == PHASE_COLOR || ePhase == PHASE_FADE)
	{
		GLboolean bWriteColor(phase.GetColorWrite() ? GL_TRUE : GL_FALSE);
		GLboolean bWriteAlpha(phase.GetAlphaWrite() ? GL_TRUE : GL_FALSE);

		glColorMask(bWriteColor, bWriteColor, bWriteColor, bWriteAlpha);
	}

	if (phase.GetTranslucent())
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);

	eCullMode eCull(phase.GetCullMode());

	if ((ePhase == PHASE_SHADOW && (vid_shadowBackface || g_Shadowmap.GetType() == SHADOWMAP_R32F)) != g_bInvertedProjection)
	{
		// Backwards culling
		if (eCull == CULL_BACK)
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
		}
		else if (eCull == CULL_NONE)
		{
			glDisable(GL_CULL_FACE);
		}
		else
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
		}
	}
	else
	{
		if (eCull == CULL_BACK)
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
		}
		else if (eCull == CULL_NONE)
		{
			glDisable(GL_CULL_FACE);
		}
		else
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
		}
	}

	glEnable(GL_POLYGON_OFFSET_FILL);

	if (ePhase == PHASE_SHADOW && g_Shadowmap.GetType() == SHADOWMAP_DEPTH)
	{
		glPolygonOffset(vid_shadowSlopeBias + vid_shadowmapFilterWidth, vid_shadowDepthBias * 2);
	}
	else
	{
		glPolygonOffset(phase.GetDepthSlopeBias() ? -1.0f : 0.0f, phase.GetDepthBias() * vid_depthBiasScale);
	}

	SShaderProgramSlot &cShaderProgram(g_aShaderProgramSlots[g_iCurrentShaderProgram]);

	//
	// Shader Parameters
	//
	{
		PROFILE("Shader Parameters");

		for (uint ui(0); ui < cShaderProgram.vUniforms.size(); ++ui)
		{
			SShaderUniform &cUniform(cShaderProgram.vUniforms[ui]);

			if (cUniform.eTextureType != GL_NONE)
			{
				// Find sampler in the shader
				int iSamplerIndex(phase.GetSamplerIndex(cUniform.sName));

				if (iSamplerIndex != -1)
				{
					SetSampler(cUniform.iTextureStage, cUniform.eTextureType, phase.GetSampler(iSamplerIndex), cUniform.uiSubTexture);
				}
				else
				{
					CShaderSampler *pSampler(cUniform.pShaderSampler);

					if (pSampler)
					{
						pSampler->Get(cUniform.iTextureStage);
					}
					else 
					{
						// No valid sampler found, so disable the texture unit
						glActiveTextureARB(GL_TEXTURE0_ARB + cUniform.iTextureStage);

						if (m_aTextureUnits[cUniform.iTextureStage].eTextureType != GL_NONE)
						{
							m_aTextureUnits[cUniform.iTextureStage].eTextureType = GL_NONE;
						}
					}
				}

				glUniform1iARB(cUniform.iLocation, cUniform.iTextureStage);
			}
			else
			{
				CShaderVar *pVar(cUniform.pShaderVar);

				if (pVar)
					pVar->Get(cUniform.eType, cUniform.iLocation);
				else
				{
					// See if this material defines this parameter
					const IMaterialParameter *pParam(material.GetParameter(cUniform.sName));

					if (pParam)
					{
						switch (pParam->GetType())
						{
						case MPT_FLOAT:
							glUniform1fARB(cUniform.iLocation, pParam->GetFloat(fTime));
							break;
						case MPT_VEC2:
							glUniform2fvARB(cUniform.iLocation, 1, (GLfloat *)&pParam->GetVec2(fTime));
							break;
						case MPT_VEC3:
							glUniform3fvARB(cUniform.iLocation, 1, (GLfloat *)&pParam->GetVec3(fTime));
							break;
						case MPT_VEC4:
							glUniform4fvARB(cUniform.iLocation, 1, (GLfloat *)&pParam->GetVec4(fTime));
							break;
						}
					}
					else
					{
						Console.Warn << "Shader parameter " << SingleQuoteStr(cUniform.sName) << " not defined" << newl;
					}
				}
			}
		}
	}

	// turn off unused texture stages
	for (int ui(cShaderProgram.iNumTextureStages); ui < g_iMaxTextureImageUnits; ++ui)
	{
		if (m_aTextureUnits[ui].eTextureType != GL_NONE)
		{
			glActiveTextureARB(GL_TEXTURE0_ARB + ui);
			
			m_aTextureUnits[ui].eTextureType = GL_NONE;
		}
	}

	glActiveTextureARB(GL_TEXTURE0_ARB);

	return true;
}


/*====================
  CGfxMaterials::SetSampler
  ====================*/
void	CGfxMaterials::SetSampler(int iTextureStage, GLenum eTextureType, const CMaterialSampler &sampler, uint uiSubTexture)
{
	glActiveTextureARB(GL_TEXTURE0_ARB + iTextureStage);

	// Enable the appropriate texture unit
	if (m_aTextureUnits[iTextureStage].eTextureType != eTextureType)
	{
		m_aTextureUnits[iTextureStage].eTextureType = eTextureType;
	}

	if (eTextureType == GL_NONE)
		return;

	ResHandle hTexture(sampler.GetTexture());
	if (hTexture != INVALID_RESOURCE)
	{
		CTexture *pTexture(g_ResourceManager.GetTexture(hTexture));
		uint uiTextureID(pTexture != NULL ? uiSubTexture > 0 ? pTexture->GetIndex2() : pTexture->GetIndex() : 0);
		
		glBindTexture(eTextureType, uiTextureID);

		if (sampler.HasFlags(SAM_BORDER))
		{
			CVec4f borderColor(1.0f, 1.0f, 1.0f, 1.0f);
			glTexParameteri(eTextureType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(eTextureType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTexParameterfv(eTextureType, GL_TEXTURE_BORDER_COLOR, (GLfloat *)&borderColor);
		}
		else
		{
			if (eTextureType == GL_TEXTURE_CUBE_MAP)
			{
				glTexParameteri(eTextureType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(eTextureType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(eTextureType, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			}
			else
			{
				if (sampler.HasFlags(SAM_REPEAT_U))
					glTexParameteri(eTextureType, GL_TEXTURE_WRAP_S, GL_REPEAT);
				else
					glTexParameteri(eTextureType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

				if (eTextureType != GL_TEXTURE_1D)
				{
					if (sampler.HasFlags(SAM_REPEAT_V))
						glTexParameteri(eTextureType, GL_TEXTURE_WRAP_T, GL_REPEAT);
					else
						glTexParameteri(eTextureType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				}

				if (eTextureType == GL_TEXTURE_3D)
				{
					if (sampler.HasFlags(SAM_REPEAT_W))
						glTexParameteri(eTextureType, GL_TEXTURE_WRAP_R, GL_REPEAT);
					else
						glTexParameteri(eTextureType, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				}
			}
		}

		if (sampler.HasFlags(SAM_NO_MIPMAPS) || (pTexture && pTexture->GetTextureFlags() & TEX_NO_MIPMAPS))
		{
			if (sampler.HasFlags(SAM_NO_FILTERING))
			{
				glTexParameteri(eTextureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(eTextureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			}
			else
			{
				glTexParameteri(eTextureType, GL_TEXTURE_MAG_FILTER, g_textureMagFilter);
				glTexParameteri(eTextureType, GL_TEXTURE_MIN_FILTER, g_textureMinFilter);
				if (GLEW_EXT_texture_filter_anisotropic && eTextureType == GL_TEXTURE_2D)
					glTexParameterf(eTextureType, GL_TEXTURE_MAX_ANISOTROPY_EXT, g_textureMaxAnisotropy);
			}
		}
		else
		{
			if (sampler.HasFlags(SAM_NO_FILTERING))
			{
				glTexParameteri(eTextureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(eTextureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			}
			else
			{
				glTexParameteri(eTextureType, GL_TEXTURE_MAG_FILTER, g_textureMagFilter);
				glTexParameteri(eTextureType, GL_TEXTURE_MIN_FILTER, g_textureMinFilterMipmap);
				if (GLEW_EXT_texture_filter_anisotropic && eTextureType == GL_TEXTURE_2D)
					glTexParameterf(eTextureType, GL_TEXTURE_MAX_ANISOTROPY_EXT, g_textureMaxAnisotropy);
			}
		}
	}
	else
	{
		glBindTexture(eTextureType, GfxTextures->GetWhiteTexture());
	}
}


/*====================
  CGfxMaterials::BindAttributes
  ====================*/
void	CGfxMaterials::BindAttributes(const AttributeMap &mapAttributes, int iStride)
{
	PROFILE("CGfxMaterials::BindAttributes");

	SShaderProgramSlot &cShaderProgram(g_aShaderProgramSlots[g_iCurrentShaderProgram]);

	for (uint ui(0); ui < cShaderProgram.vAttributes.size(); ++ui)
	{
		SShaderAttribute &cAttribute(cShaderProgram.vAttributes[ui]);

		AttributeMap::const_iterator itFind(mapAttributes.find(cAttribute.sName));
		if (itFind != mapAttributes.end())
		{
			glEnableVertexAttribArrayARB(cAttribute.iLocation);
			glVertexAttribPointerARB(cAttribute.iLocation, itFind->second.iSize, itFind->second.uiType, itFind->second.bNormalized, iStride, BUFFER_OFFSET(itFind->second.iOffset));
		}
		else
		{
			glDisableVertexAttribArrayARB(cAttribute.iLocation);
		}
	}
}


/*====================
  CGfxMaterials::UnbindAttributes
  ====================*/
void	CGfxMaterials::UnbindAttributes()
{
	PROFILE("CGfxMaterials::UnbindAttributes");

	SShaderProgramSlot &cShaderProgram(g_aShaderProgramSlots[g_iCurrentShaderProgram]);

	for (uint ui(0); ui < cShaderProgram.vAttributes.size(); ++ui)
	{
		glDisableVertexAttribArrayARB(cShaderProgram.vAttributes[ui].iLocation);
	}
}


/*====================
  CGfxMaterials::GetTextureStage
  ====================*/
int		CGfxMaterials::GetTextureStage(const tstring &sSampler)
{
	SShaderProgramSlot &cShaderProgram(g_aShaderProgramSlots[g_iCurrentShaderProgram]);

	for (uint ui(0); ui < cShaderProgram.vUniforms.size(); ++ui)
	{
		SShaderUniform &cUniform(cShaderProgram.vUniforms[ui]);

		if (cUniform.eTextureType == GL_NONE || cUniform.sName != sSampler)
			continue;

		return cUniform.iTextureStage;
	}

	return -1;
}


/*====================
  CGfxMaterials::UpdateShaderTexture
  ====================*/
bool	CGfxMaterials::UpdateShaderTexture(int iTextureStage, ResHandle hTexture, uint uiSubTexture)
{
	GLenum eTextureType(m_aTextureUnits[iTextureStage].eTextureType);

	glActiveTextureARB(GL_TEXTURE0_ARB + iTextureStage);

	if (hTexture != INVALID_RESOURCE)
	{
		CTexture *pTexture(g_ResourceManager.GetTexture(hTexture));
		uint uiTextureID(pTexture != NULL ? uiSubTexture > 0 ? pTexture->GetIndex2() : pTexture->GetIndex() : 0);
		glBindTexture(eTextureType, uiTextureID);

		return true;
	}
	else
	{
		m_aTextureUnits[iTextureStage].eTextureType = GL_NONE;

		return false;
	}
}


/*====================
  CGfxMaterials::UpdateShaderTexture
  ====================*/
bool	CGfxMaterials::UpdateShaderTexture(const tstring &sSampler, ResHandle hTexture, uint uiFlags, uint uiSubTexture)
{
	SShaderProgramSlot &cShaderProgram(g_aShaderProgramSlots[g_iCurrentShaderProgram]);

	for (uint ui(0); ui < cShaderProgram.vUniforms.size(); ++ui)
	{
		SShaderUniform &cUniform(cShaderProgram.vUniforms[ui]);

		if (cUniform.eTextureType == GL_NONE || cUniform.sName != sSampler || cUniform.uiSubTexture != uiSubTexture)
			continue;

		glActiveTextureARB(GL_TEXTURE0_ARB + cUniform.iTextureStage);

		int iTextureStage(cUniform.iTextureStage);
		GLenum eTextureType(cUniform.eTextureType);

		if (hTexture != INVALID_RESOURCE)
		{
			// make sure the appropriate texture unit is enabled
			if (m_aTextureUnits[iTextureStage].eTextureType != eTextureType)
				m_aTextureUnits[iTextureStage].eTextureType = eTextureType;

			CTexture *pTexture(g_ResourceManager.GetTexture(hTexture));
			uint uiTextureID(pTexture != NULL ? uiSubTexture > 0 ? pTexture->GetIndex2() : pTexture->GetIndex() : 0);
			
			glBindTexture(eTextureType, uiTextureID);
		
			if (uiFlags & SAM_BORDER)
			{
				CVec4f borderColor(1.0f, 1.0f, 1.0f, 1.0f);
				glTexParameteri(eTextureType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
				glTexParameteri(eTextureType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
				glTexParameterfv(eTextureType, GL_TEXTURE_BORDER_COLOR, (GLfloat *)&borderColor);
			}
			else
			{
				if (eTextureType == GL_TEXTURE_CUBE_MAP)
				{
					glTexParameteri(eTextureType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(eTextureType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					glTexParameteri(eTextureType, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				}
				else
				{
					if (uiFlags & SAM_REPEAT_U)
						glTexParameteri(eTextureType, GL_TEXTURE_WRAP_S, GL_REPEAT);
					else
						glTexParameteri(eTextureType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

					if (uiFlags & SAM_REPEAT_V)
						glTexParameteri(eTextureType, GL_TEXTURE_WRAP_T, GL_REPEAT);
					else
						glTexParameteri(eTextureType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

					if (eTextureType == GL_TEXTURE_3D)
					{
						if (uiFlags & SAM_REPEAT_W)
							glTexParameteri(eTextureType, GL_TEXTURE_WRAP_R, GL_REPEAT);
						else
							glTexParameteri(eTextureType, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
					}
				}
			}

			if (uiFlags & SAM_NO_MIPMAPS || (pTexture && pTexture->GetTextureFlags() & TEX_NO_MIPMAPS))
			{
				if (uiFlags & SAM_NO_FILTERING)
				{
					glTexParameteri(eTextureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTexParameteri(eTextureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				}
				else
				{
					glTexParameteri(eTextureType, GL_TEXTURE_MAG_FILTER, g_textureMagFilter);
					glTexParameteri(eTextureType, GL_TEXTURE_MIN_FILTER, g_textureMinFilter);
					if (GLEW_EXT_texture_filter_anisotropic && eTextureType == GL_TEXTURE_2D)
						glTexParameterf(eTextureType, GL_TEXTURE_MAX_ANISOTROPY_EXT, g_textureMaxAnisotropy);
				}
			}
			else
			{
				if (uiFlags & SAM_NO_FILTERING)
				{
					glTexParameteri(eTextureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTexParameteri(eTextureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				}
				else
				{
					glTexParameteri(eTextureType, GL_TEXTURE_MAG_FILTER, g_textureMagFilter);
					glTexParameteri(eTextureType, GL_TEXTURE_MIN_FILTER, g_textureMinFilterMipmap);
					if (GLEW_EXT_texture_filter_anisotropic && eTextureType == GL_TEXTURE_2D)
						glTexParameterf(eTextureType, GL_TEXTURE_MAX_ANISOTROPY_EXT, g_textureMaxAnisotropy);
				}
			}
		}
		else
		{
			// Disable the current texture unit
			if (m_aTextureUnits[iTextureStage].eTextureType != GL_NONE)
			{
				//glDisable(m_aTextureUnits[iTextureStage].eTextureType);
				m_aTextureUnits[iTextureStage].eTextureType = GL_NONE;
			}
		}

		return true;
	}

	return false;
}


/*====================
  CGfxMaterials::UpdateShaderParams

  This does not update samplers!
  ====================*/
bool	CGfxMaterials::UpdateShaderParams(const CMaterial &material, float fTime)
{
	g_fCurrentTime = fTime;

	SShaderProgramSlot &cShaderProgram(g_aShaderProgramSlots[g_iCurrentShaderProgram]);

	//
	// Shader Parameters
	//
	{
		PROFILE("Shader Parameters");

		for (uint ui(0); ui < cShaderProgram.vUniforms.size(); ++ui)
		{
			SShaderUniform &cUniform(cShaderProgram.vUniforms[ui]);

			if (cUniform.eTextureType != GL_NONE)
			{
				continue;
			}
			else
			{
				CShaderVar *pVar(cUniform.pShaderVar);

				if (pVar)
					pVar->Get(cUniform.eType, cUniform.iLocation);
				else
				{
					// See if this material defines this parameter
					const IMaterialParameter *pParam(material.GetParameter(cUniform.sName));

					if (pParam)
					{
						switch (pParam->GetType())
						{
						case MPT_FLOAT:
							glUniform1fARB(cUniform.iLocation, pParam->GetFloat(fTime));
							break;
						case MPT_VEC2:
							glUniform2fvARB(cUniform.iLocation, 1, (GLfloat *)&pParam->GetVec2(fTime));
							break;
						case MPT_VEC3:
							glUniform3fvARB(cUniform.iLocation, 1, (GLfloat *)&pParam->GetVec3(fTime));
							break;
						case MPT_VEC4:
							glUniform4fvARB(cUniform.iLocation, 1, (GLfloat *)&pParam->GetVec4(fTime));
							break;
						}
					}
					else
					{
						Console.Warn << "Shader parameter " << SingleQuoteStr(cUniform.sName) << " not defined" << newl;
					}
				}
			}
		}
	}

	return true;
}

