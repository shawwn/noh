// (C)2005 S2 Games
// c_material.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_material.h"
#include "i_resourcelibrary.h"
#include "c_xmlmanager.h"
#include "c_pixelshader.h"
#include "c_vertexshader.h"
#include "c_texture.h"
#include "c_materialparameter.h"
#include "c_vid.h"
#include "c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
IResource*	AllocMaterial(const tstring &sPath);
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
IResourceLibrary	g_ResLibMaterial(RES_MATERIAL, _T("Materials"), CMaterial::ResTypeName(), true, AllocMaterial);
CMaterialSampler	emptySampler;
//=============================================================================

/*====================
  AllocMaterial
  ====================*/
IResource*	AllocMaterial(const tstring &sPath)
{
	return K2_NEW(ctx_Resources,  CMaterial)(sPath, TSNULL);
}


/*====================
  CMaterial::CMaterial
  ====================*/
CMaterial::CMaterial(const tstring &sPath, const tstring &sName) :
IResource(sPath, sName),
m_uiPhases(0),
m_vPhases(NUM_SHADER_PHASES),
m_iPass(0)
{
}


/*====================
  CMaterial::Load
  ====================*/
int		CMaterial::Load(uint uiIgnoreFlags, const char *pData, uint uiSize)
{
	PROFILE("CMaterial::Load");

	try
	{
		// Dedicated servers don't need material files so skip this and save some memory
		if (K2System.IsDedicatedServer() || K2System.IsServerManager())
			return false;
		
		if (!m_sPath.empty())
			Console.Res << "Loading Material " << SingleQuoteStr(m_sPath) << newl;
		else if (!m_sName.empty())
			Console.Res << "Loading Material " << SingleQuoteStr(m_sName) << newl;
		else
			Console.Res << "Loading Unknown Material" << newl;

		// Process the material XML file
		if (!XMLManager.ReadBuffer(pData, uiSize, _T("material"), this))
			throw CException(_TS("CMaterial::Load(") + m_sPath + _T(") - couldn't read XML"), E_WARNING);
	}
	catch (CException &ex)
	{
		ex.Process(_TS("CMaterial::Load(") + m_sName + _TS(") - "), THROW);
		return RES_LOAD_FAILED;
	}

	return 0;
}


/*====================
  CMaterial::LoadNull

  Loads NULL material
  ====================*/
bool	CMaterial::LoadNull()
{
	try
	{
		m_fGlossiness = 16.0f;
		m_fSpecularLevel = 1.0f;
		m_fBumpLevel = 1.0f;
		m_fReflect = 0.0f;
		m_vDiffuseColor = CVec3f(1.0f, 1.0f, 1.0f);
		m_fOpacity = 1.0f;

		//
		// Phase 0 (shadow)
		//
		CMaterialPhase	phaseShadow(PHASE_SHADOW, _T("mesh_shadow"), _T("mesh_shadow"), BLEND_ONE, BLEND_ZERO, CULL_BACK, 0, 0, 0.0f, PHASE_DEFAULT);
		AddPhase(phaseShadow);

		//
		// Phase 1 (color)
		//
		CMaterialPhase	phaseColor(PHASE_COLOR, _T("mesh_color"), _T("mesh_color"), BLEND_ONE, BLEND_ZERO, CULL_BACK, 0, 0, 0.0f, PHASE_DEFAULT);
		CMaterialSampler samplerDiffuse(_T("diffuse"), 15, 0.0f, 0.0f, 1.0f, 1.0f, SAM_REPEAT, _T("$yellow_checker"), TEXTURE_2D, 0);
		CMaterialSampler samplerNormalmap(_T("normalmap"), 15, 0.0f, 0.0f, 1.0f, 1.0f, SAM_REPEAT, _T("$flat"), TEXTURE_2D, 0);
		phaseColor.AddSampler(samplerDiffuse);
		phaseColor.AddSampler(samplerNormalmap);
		AddPhase(phaseColor);
	}
	catch (CException &ex)
	{
		ex.Process(_TS("CMaterial::LoadNull(") + m_sName + _TS(") - "), NO_THROW);
		EX_FATAL(_T("Material NULL resource failure"));
	}

	return true;
}


/*====================
  CMaterial::Free
  ====================*/
void	CMaterial::Free()
{
	for (MaterialParameterMap::iterator it = m_mapParams.begin(); it != m_mapParams.end(); ++it)
		K2_DELETE(it->second);

	m_mapParams.clear();
}


/*====================
  CMaterial::AddPhase
  ====================*/
CMaterialPhase&	CMaterial::AddPhase(const CMaterialPhase &Phase)
{
	EMaterialPhase ePhase(Phase.GetPhase());

	m_vPhases[ePhase] = Phase;
	m_uiPhases |= BIT(ePhase);

	return m_vPhases[ePhase];
}


/*====================
  CMaterialPhase::CMaterialPhase
  ====================*/
CMaterialPhase::CMaterialPhase() :
m_ePhase(PHASE_COLOR),
m_sVertexShaderName(_T("")),
m_sPixelShaderName(_T("")),
m_eSrcBlend(BLEND_ONE),
m_eDstBlend(BLEND_ZERO),
m_iDepthBias(0),
m_eCull(CULL_BACK),
m_iLayer(0),
m_fDepthSortBias(0.0f),
m_iFlags(0)
{
}


/*====================
  CMaterialPhase::CMaterialPhase
  ====================*/
CMaterialPhase::CMaterialPhase
(
	EMaterialPhase ePhase,
	const tstring &sVertexShaderName,
	const tstring &sPixelShaderName,
	eBlendMode eSrcBlend,
	eBlendMode eDstBlend,
	eCullMode eCull,
	int iDepthBias,
	int iLayer,
	float fDepthSortBias,
	int iFlags
) :
m_ePhase(ePhase),
m_sVertexShaderName(sVertexShaderName),
m_sPixelShaderName(sPixelShaderName),
m_eSrcBlend(eSrcBlend),
m_eDstBlend(eDstBlend),
m_iDepthBias(iDepthBias),
m_eCull(eCull),
m_iLayer(iLayer),
m_fDepthSortBias(fDepthSortBias),
m_iFlags(iFlags)
{
	m_hVertexShader = g_ResourceManager.Register(K2_NEW(ctx_Resources,  CVertexShader)(sVertexShaderName, m_iFlags & PHASE_SHADER_GUI_PRECACHE ? VS_GUI_PRECACHE : 0), RES_VERTEX_SHADER);
	m_hPixelShader = g_ResourceManager.Register(K2_NEW(ctx_Resources,  CPixelShader)(sPixelShaderName, m_iFlags & PHASE_SHADER_GUI_PRECACHE ? PS_GUI_PRECACHE : 0), RES_PIXEL_SHADER);

	if (m_hVertexShader != INVALID_RESOURCE && m_hPixelShader != INVALID_RESOURCE)
		Vid.RegisterShaderPair(g_ResourceManager.GetVertexShader(m_hVertexShader), g_ResourceManager.GetPixelShader(m_hPixelShader));
}


/*====================
  CMaterialPhase::~CMaterialPhase
  ====================*/
CMaterialPhase::~CMaterialPhase()
{
}


/*====================
  CMaterialSampler::AddSampler

  TODO: Check for duplicate samplers
  ====================*/
void	CMaterialPhase::AddSampler(const CMaterialSampler &cSampler)
{
	m_vSamplers.push_back(cSampler);
}


/*====================
  CMaterialSampler::AddMultiPass
  ====================*/
CMaterialPhase&	CMaterialPhase::AddMultiPass(const CMaterialPhase &cMultiPass)
{
	m_vMultiPass.push_back(cMultiPass);

	return m_vMultiPass.back();
}


/*====================
  CMaterialPhase::GetSampler
  ====================*/
CMaterialSampler&		CMaterialPhase::GetSampler(const tstring &sName)
{
	if (m_vSamplers.empty())
		return emptySampler;

	vector<CMaterialSampler>::iterator itSampler(m_vSamplers.begin());
	for (; itSampler != m_vSamplers.end(); ++itSampler)
	{
		if (itSampler->GetName() == sName)
			return *itSampler;
	}

	return *itSampler;
}


/*====================
  CMaterialPhase::GetSampler
  ====================*/
const CMaterialSampler&		CMaterialPhase::GetSampler(const tstring &sName) const
{
	if (m_vSamplers.empty())
		return emptySampler;

	vector<CMaterialSampler>::const_iterator itSampler(m_vSamplers.begin());
	for (; itSampler != m_vSamplers.end(); ++itSampler)
	{
		if (itSampler->GetName() == sName)
			return *itSampler;
	}

	return *itSampler;
}


/*====================
  CMaterialPhase::GetSamplerIndex
  ====================*/
int		CMaterialPhase::GetSamplerIndex(const tstring &sName) const
{
	int n = 0;
	for (vector<CMaterialSampler>::const_iterator itSampler = m_vSamplers.begin(); itSampler != m_vSamplers.end(); ++itSampler, ++n)
	{
		if (itSampler->GetName() == sName)
			return n;
	}

	return -1;
}


/*====================
  CMaterialSampler::CMaterialSampler
  ====================*/
CMaterialSampler::CMaterialSampler() :
m_sName(_T("")),
m_iFps(15),
m_fOffsetU(0.0f),
m_fOffsetV(0.0f),
m_fScaleU(1.0f),
m_fScaleV(1.0f),
m_iFlags(0),
m_sTexture(_T("")),
m_iTextureFlags(0),
m_hTexture(INVALID_RESOURCE)
{
}


/*====================
  CMaterialSampler::CMaterialSampler
  ====================*/
CMaterialSampler::CMaterialSampler
(
	const tstring &sName,
	int iFps,
	float fOffsetU,
	float fOffsetV,
	float fScaleU,
	float fScaleV,
	int iFlags,
	const tstring &sTexture,
	ETextureType eType,
	int iTextureFlags
) :
m_sName(sName),
m_iFps(iFps),
m_fOffsetU(fOffsetU),
m_fOffsetV(fOffsetV),
m_fScaleU(fScaleU),
m_fScaleV(fScaleV),
m_iFlags(iFlags),
m_sTexture(sTexture),
m_iTextureFlags(iTextureFlags),
m_hTexture(INVALID_RESOURCE)
{
	if (!sTexture.empty())
		m_hTexture = g_ResourceManager.Register(K2_NEW(ctx_Resources,  CTexture)(sTexture, eType, iTextureFlags, (iFlags & SAM_NORMALMAP) ? TEXFMT_NORMALMAP : TEXFMT_A8R8G8B8), RES_TEXTURE);
}


/*====================
  CMaterialSampler::~CMaterialSampler
  ====================*/
CMaterialSampler::~CMaterialSampler()
{
}


/*====================
  CMaterial::AddParameter
  ====================*/
void	CMaterial::AddParameter(const IMaterialParameter *pParam)
{
	m_mapParams[pParam->GetName()] = pParam;
}


