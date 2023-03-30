// (C)2005 S2 Games
// c_material.h
//
//=============================================================================
#ifndef __C_MATERIAL_H__
#define __C_MATERIAL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_resource.h"
#include "c_texture.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class IMaterialParameter;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================

// sampler flags
const int SAM_REPEAT_U			(BIT(0));	// texture is a repeated (tiled) texture
const int SAM_REPEAT_V			(BIT(1));	// texture is a repeated (tiled) texture
const int SAM_REPEAT_W			(BIT(2));	// texture is a repeated (tiled) texture
const int SAM_NO_MIPMAPS		(BIT(3));	// don't generate mipmaps for this texture
const int SAM_NO_FILTERING		(BIT(4));	// use nearest-point filtering only
const int SAM_BORDER			(BIT(5));	// use texture border
const int SAM_NORMALMAP			(BIT(6));   // texture is a normalmap

const int SAM_REPEAT			(SAM_REPEAT_U|SAM_REPEAT_V);

// phase flags
const int PHASE_SHADER_GUI_PRECACHE	(BIT(0));
const int PHASE_TRANSLUCENT			(BIT(1));
const int PHASE_ALPHA_TEST			(BIT(2));
const int PHASE_DEPTH_WRITE			(BIT(3));
const int PHASE_DEPTH_READ			(BIT(4));
const int PHASE_DEPTH_SLOPE_BIAS	(BIT(5));
const int PHASE_NO_SHADOWS			(BIT(6));
const int PHASE_NO_LIGHTING			(BIT(7));
const int PHASE_NO_FOG				(BIT(8));
const int PHASE_REFRACTIVE			(BIT(9));
const int PHASE_COLOR_WRITE			(BIT(10));
const int PHASE_ALPHA_WRITE			(BIT(11));
const int PHASE_VAMPIRE				(BIT(12)); // Don't show up in reflections
const int PHASE_WIREFRAME			(BIT(13)); // Don't show up in reflections

const int PHASE_DEFAULT				(PHASE_DEPTH_WRITE | PHASE_DEPTH_READ | PHASE_COLOR_WRITE | PHASE_ALPHA_WRITE);
const int PHASE_DEFAULT_TRANSLUCENT	(PHASE_DEPTH_READ | PHASE_COLOR_WRITE | PHASE_ALPHA_WRITE);
const int PHASE_DEFAULT_GUI			(PHASE_SHADER_GUI_PRECACHE | PHASE_TRANSLUCENT | PHASE_COLOR_WRITE | PHASE_ALPHA_WRITE);

enum eBlendMode
{
	BLEND_ZERO = 0,
	BLEND_ONE,
	BLEND_SRC_COLOR,
	BLEND_DEST_COLOR,
	BLEND_ONE_MINUS_SRC_COLOR,
	BLEND_ONE_MINUS_DEST_COLOR,
	BLEND_SRC_ALPHA,
	BLEND_DEST_ALPHA,
	BLEND_ONE_MINUS_SRC_ALPHA,
	BLEND_ONE_MINUS_DEST_ALPHA,
	NUM_BLEND_MODES
};

enum eCullMode
{
	CULL_BACK = 0,
	CULL_FRONT,
	CULL_NONE,
	NUM_CULL_MODES
};

enum EMaterialPhase
{
	PHASE_COLOR = 0,
	PHASE_SHADOW,
	PHASE_DEPTH,
	PHASE_FADE,
	PHASE_VELOCITY,
	PHASE_REFRACT,
	NUM_SHADER_PHASES
};

enum ELightingScheme
{
	LIGHTING_DEFAULT = 0,
	LIGHTING_TERRAIN,
	LIGHTING_ENTITY,
	NUM_LIGHTING_SCHEMES
};

#define	SHD_CALCULATEFRAME(time, fps, numFrames) (int((ABS(time)) * (fps)) % (numFrames))
#define SHD_CALCULATETIME(frame, fps) (float(frame) / (fps))

typedef map<tstring, const IMaterialParameter *>	MaterialParameterMap;
//=============================================================================

//=============================================================================
// CMaterialSampler
//=============================================================================
class CMaterialSampler
{
private:
	tstring		m_sName;

	int			m_iFps;

	float		m_fOffsetU;
	float		m_fOffsetV;

	float		m_fScaleU;
	float		m_fScaleV;

	int			m_iFlags;

	tstring		m_sTexture;
	int			m_iTextureFlags;
	ResHandle	m_hTexture;

public:
	K2_API ~CMaterialSampler();
	K2_API CMaterialSampler();
	K2_API CMaterialSampler
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
	);

	const tstring&	GetName() const		{ return m_sName; }
	int				GetFps() const		{ return m_iFps; }
	float			GetOffsetU() const	{ return m_fOffsetU; }
	float			GetOffsetV() const	{ return m_fOffsetV; }
	float			GetScaleU() const	{ return m_fScaleU; }
	float			GetScaleV() const	{ return m_fScaleV; }
	ResHandle		GetTexture() const	{ return m_hTexture; }

	void	SetTexture(ResHandle hTexture) { m_hTexture = hTexture; }

	void	AddFlags(int iFlags)			{ m_iFlags |= iFlags; }
	void	ClearFlags(int iFlags)			{ m_iFlags &= ~iFlags; }
	void	ClearAllFlags()					{ m_iFlags = 0; }
	bool	HasFlags(int iFlags) const		{ return (m_iFlags & iFlags) != 0; }
	bool	HasAllFlags(int iFlags) const	{ return (m_iFlags & iFlags) == iFlags; }
};
//=============================================================================


//=============================================================================
// CMaterialPhase
//=============================================================================
class CMaterialPhase
{
private:
	EMaterialPhase	m_ePhase;

	tstring			m_sVertexShaderName;
	ResHandle		m_hVertexShader;

	tstring			m_sPixelShaderName;
	ResHandle		m_hPixelShader;

	eBlendMode		m_eSrcBlend;		// framebuffer blend operation
	eBlendMode		m_eDstBlend;		// framebuffer blend operation

	int				m_iDepthBias;

	eCullMode		m_eCull;

	int				m_iLayer;
	float			m_fDepthSortBias;

	int			m_iFlags;

	vector<CMaterialSampler>	m_vSamplers;

	vector<CMaterialPhase>		m_vMultiPass;	// Leeloo Dallas Multipass

public:
	K2_API ~CMaterialPhase();
	K2_API CMaterialPhase();
	K2_API CMaterialPhase
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
	);

	EMaterialPhase	GetPhase() const			{ return m_ePhase; }
	const tstring&	GetVertexShaderName() const	{ return m_sVertexShaderName; }
	ResHandle		GetVertexShader() const		{ return m_hVertexShader; }
	const tstring&	GetPixelShaderName() const	{ return m_sPixelShaderName; }
	ResHandle		GetPixelShader() const		{ return m_hPixelShader; }
	eBlendMode		GetSrcBlend() const			{ return m_eSrcBlend; }
	eBlendMode		GetDstBlend() const			{ return m_eDstBlend; }
	bool			GetTranslucent() const		{ return (m_iFlags & PHASE_TRANSLUCENT) != 0; }
	bool			GetAlphaTest() const		{ return (m_iFlags & PHASE_ALPHA_TEST) != 0; }
	eCullMode		GetCullMode() const			{ return m_eCull; }
	bool			GetDepthWrite() const		{ return (m_iFlags & PHASE_DEPTH_WRITE) != 0; }
	bool			GetDepthRead() const		{ return (m_iFlags & PHASE_DEPTH_READ) != 0; }
	int				GetDepthBias() const		{ return m_iDepthBias; }
	bool			GetDepthSlopeBias() const	{ return (m_iFlags & PHASE_DEPTH_SLOPE_BIAS) != 0; }
	bool			GetLighting() const			{ return (m_iFlags & PHASE_NO_LIGHTING) == 0; }
	bool			GetShadows() const			{ return (m_iFlags & PHASE_NO_SHADOWS) == 0; }
	bool			GetFog() const				{ return (m_iFlags & PHASE_NO_FOG) == 0; }
	int				GetLayer() const			{ return m_iLayer; }
	float			GetDepthSortBias() const	{ return m_fDepthSortBias; }
	bool			GetRefractive() const		{ return (m_iFlags & PHASE_REFRACTIVE) != 0; }
	bool			GetColorWrite() const		{ return (m_iFlags & PHASE_COLOR_WRITE) != 0; }
	bool			GetAlphaWrite() const		{ return (m_iFlags & PHASE_ALPHA_WRITE) != 0; }
	bool			GetVampire() const			{ return (m_iFlags & PHASE_VAMPIRE) != 0; }
	bool			GetWireframe() const		{ return (m_iFlags & PHASE_WIREFRAME) != 0; }

	void	SetVertexShader(ResHandle hVertexShader)	{ m_hVertexShader = hVertexShader; }
	void	SetPixelShader(ResHandle hPixelShader)		{ m_hPixelShader = hPixelShader; }
	void	SetSrcBlend(eBlendMode eSrcBlend)			{ m_eSrcBlend = eSrcBlend; }
	void	SetDstBlend(eBlendMode eDstBlend)			{ m_eDstBlend = eDstBlend; }
	void	SetTranslucent(bool bTranslucent)			{ if (bTranslucent) m_iFlags |= PHASE_TRANSLUCENT; else m_iFlags &= ~PHASE_TRANSLUCENT; }
	void	SetAlphaTest(bool bAlphaTest)				{ if (bAlphaTest) m_iFlags |= PHASE_ALPHA_TEST; else m_iFlags &= ~PHASE_ALPHA_TEST; }
	void	SetCullMode(eCullMode eCull)				{ m_eCull = eCull; }
	void	SetDepthWrite(bool bDepthWrite)				{ if (bDepthWrite) m_iFlags |= PHASE_DEPTH_WRITE; else m_iFlags &= ~PHASE_DEPTH_WRITE; }
	void	SetLayer(int iLayer)						{ m_iLayer = iLayer; }
	void	SetDepthSortBias(float fDepthSortBias)		{ m_fDepthSortBias = fDepthSortBias; }
	void	SetRefractive(bool bRefractive)				{ if (bRefractive) m_iFlags |= PHASE_REFRACTIVE; else m_iFlags &= ~PHASE_REFRACTIVE; }

	void	SetFlags(int iFlags)			{ m_iFlags |= iFlags; }
	void	UnsetFlags(int iFlags)			{ m_iFlags &= ~iFlags; }
	void	ClearFlags()					{ m_iFlags = 0; }
	bool	HasFlags(int iFlags) const		{ return (m_iFlags & iFlags) != 0; }
	bool	HasAllFlags(int iFlags) const	{ return (m_iFlags & iFlags) == iFlags; }

	K2_API CMaterialSampler&		GetSampler(int n);
	K2_API CMaterialSampler&		GetSampler(const tstring &sName);
	K2_API const CMaterialSampler&	GetSampler(const tstring &sName) const;
	K2_API const CMaterialSampler&	GetSampler(int n) const;
	K2_API int						GetSamplerIndex(const tstring &sName) const;

	int		GetNumSamplers() const	{ return int(m_vSamplers.size()); }
	K2_API void	AddSampler(const CMaterialSampler &cSampler);

	K2_API CMaterialPhase&			GetMultiPass(int n);
	K2_API const CMaterialPhase&	GetMultiPass(int n) const;

	int		GetNumMultiPass() const	{ return int(m_vMultiPass.size()); }
	K2_API CMaterialPhase&	AddMultiPass(const CMaterialPhase &cMultiPass);
};
//=============================================================================

//=============================================================================
// CMaterial
//=============================================================================
class 
#if defined(linux) || defined(__APPLE__)
	__attribute__((visibility("default")))
#endif
	CMaterial : public IResource
{
private:
	uint						m_uiPhases;
	vector<CMaterialPhase>		m_vPhases;
	MaterialParameterMap		m_mapParams;

	CVec3f			m_vDiffuseColor;
	float			m_fOpacity;
	float			m_fSpecularLevel;
	float			m_fGlossiness;
	float			m_fBumpLevel;
	float			m_fReflect;
	float			m_fTreeScale;
	ELightingScheme	m_eLightingScheme;
	mutable int		m_iPass;

public:
	K2_API ~CMaterial()	{}
	K2_API CMaterial(const tstring &sPath, const tstring &sName);

	K2_API	virtual uint			GetResType() const			{ return RES_MATERIAL; }
	K2_API	virtual const tstring&	GetResTypeName() const		{ return ResTypeName(); }
	K2_API	static const tstring&	ResTypeName()				{ static tstring sTypeName(_T("{material}")); return sTypeName; }

	int		Load(uint uiIgnoreFlags, const char *pData, uint uiSize);
	void	Free();

	bool	LoadNull();

	int		GetNumPhases()											{ return int(m_vPhases.size()); }
	
	CMaterialPhase&		GetPhase(EMaterialPhase ePhase)
	{
		if (m_iPass > 0)
			return m_vPhases[ePhase].GetMultiPass(m_iPass - 1);
		else
			return m_vPhases[ePhase];
	}

	const CMaterialPhase&	GetPhase(EMaterialPhase ePhase) const
	{
		if (m_iPass > 0)
			return m_vPhases[ePhase].GetMultiPass(m_iPass - 1);
		else
			return m_vPhases[ePhase];
	}

	K2_API bool	HasPhase(EMaterialPhase ePhase) const;

	CVec3f			GetDiffuseColor() const		{ return m_vDiffuseColor; }
	float			GetOpacity() const			{ return m_fOpacity; }
	float			GetSpecularLevel() const	{ return m_fSpecularLevel; }
	float			GetReflect() const			{ return m_fReflect; }
	float			GetGlossiness() const		{ return m_fGlossiness; }
	float			GetBumpLevel() const		{ return m_fBumpLevel; }
	float			GetTreeScale() const		{ return m_fTreeScale; }
	ELightingScheme	GetLightingScheme() const	{ return m_eLightingScheme; }

	void	SetDiffuseColor(const CVec3f &vColor)	{ m_vDiffuseColor = vColor;	}
	void	SetOpacity(float fOpacity)				{ m_fOpacity = fOpacity; }
	void	SetSpecularLevel(float fSpecularLevel)	{ m_fSpecularLevel = fSpecularLevel; }
	void	SetReflect(float fReflect)				{ m_fReflect = fReflect; }
	void	SetLightingScheme(ELightingScheme eScheme) { m_eLightingScheme = eScheme; }
	void	SetTreeScale(float fTScale)				{ m_fTreeScale = fTScale; }

	void	SetGlossiness(float fGlossiness)
	{
		m_fGlossiness = fGlossiness;

		if (!m_fGlossiness)
			m_fGlossiness = 16.0f;
	}

	void	SetBumpLevel(float fBumpLevel)
	{
		m_fBumpLevel = fBumpLevel;

		const float BUMP_EPSILON = 0.0001f;
		if (m_fBumpLevel >= 0.0f && m_fBumpLevel < BUMP_EPSILON)
			m_fBumpLevel = BUMP_EPSILON;
		else if(m_fBumpLevel <= 0.0f && m_fBumpLevel > -BUMP_EPSILON)
			m_fBumpLevel = -BUMP_EPSILON;
	}

	K2_API CMaterialPhase&	AddPhase(const CMaterialPhase &Phase);

	K2_API void		AddParameter(const IMaterialParameter *pParam);
	K2_API const	IMaterialParameter* GetParameter(const tstring &sName) const;

	void	SetPass(int iPass)				{ m_iPass = iPass; }
	int		GetPass() const					{ return m_iPass; }
};
//=============================================================================

//=============================================================================
// Inline functions
//=============================================================================

/*====================
  CMaterialPhase::GetSampler
  ====================*/
inline
CMaterialSampler&		CMaterialPhase::GetSampler(int n)
{
	return m_vSamplers[n];
}


/*====================
  CMaterialPhase::GetSampler
  ====================*/
inline
const CMaterialSampler&		CMaterialPhase::GetSampler(int n) const
{
	return m_vSamplers[n];
}


/*====================
  CMaterialPhase::GetMultiPass
  ====================*/
inline
CMaterialPhase&		CMaterialPhase::GetMultiPass(int n)
{
	return m_vMultiPass[n];
}


/*====================
  CMaterialPhase::GetMultiPass
  ====================*/
inline
const CMaterialPhase&		CMaterialPhase::GetMultiPass(int n) const
{
	return m_vMultiPass[n];
}


/*====================
  CMaterial::HasPhase
  ====================*/
inline
bool	CMaterial::HasPhase(EMaterialPhase ePhase) const
{
	return (m_uiPhases & BIT(ePhase)) != 0;
}


/*====================
  CMaterial::GetParameter
  ====================*/
inline
const IMaterialParameter*	CMaterial::GetParameter(const tstring &sName) const
{
	const MaterialParameterMap::const_iterator findit(m_mapParams.find(sName));

	if (findit != m_mapParams.end())
		return findit->second;
	else
		return NULL;
}
//=============================================================================
#endif //__C_MATERIAL_H__
