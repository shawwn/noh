// (C)2005 S2 Games
// c_shadersampler.h
//
//=============================================================================
#ifndef __C_SHADERSAMPLER_H__
#define __C_SHADERSAMPLER_H__

//=============================================================================
// Headers
//=============================================================================
//=============================================================================

class CShaderSampler;

//=============================================================================
// Definitions
//=============================================================================
typedef bool(*ShaderSamplerFn_t)(CShaderSampler *pThis, int iStageIndex);

// Declaration macros
#define SHADER_SAMPLER(name) \
bool	shaderSampler##name##Fn(CShaderSampler *pThis, int iStageIndex); \
CShaderSampler  shaderSampler##name(_T(#name), shaderSampler##name##Fn); \
bool	shaderSampler##name##Fn(CShaderSampler *pThis, int iStageIndex)
//=============================================================================

//=============================================================================
// CShaderSampler
//=============================================================================
class CShaderSampler
{
private:
	tstring				m_sName;
	ShaderSamplerFn_t	m_pfnShaderSampler;

	// CShaderSamplers should not be copied
	CShaderSampler(CShaderSampler&);
	CShaderSampler& operator=(CShaderSampler&);

public:
	~CShaderSampler();
	CShaderSampler(const tstring &sName, ShaderSamplerFn_t pfnCShaderSamplerCmd);

	const tstring&	GetName()			{ return m_sName; }

	bool	Get(int iStageIndex);
};
//=============================================================================
#endif //__C_SHADERSAMPLER_H__
