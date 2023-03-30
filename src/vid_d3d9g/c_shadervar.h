// (C)2005 S2 Games
// c_shadervar.h
//
//=============================================================================
#ifndef __C_SHADERVAR_H__
#define __C_SHADERVAR_H__

//=============================================================================
// Headers
//=============================================================================
//=============================================================================

class CShaderVar;

//=============================================================================
// Definitions
//=============================================================================
typedef bool(*ShaderVarFn_t)(CShaderVar *pThis, float *pRegisters, uint uiSize);

// Declaration macros
#define SHADER_VAR(name) \
bool	shaderVar##name##Fn(CShaderVar *pThis, float *pRegisters, uint uiSize); \
CShaderVar  shaderVar##name(_T(#name), shaderVar##name##Fn); \
bool	shaderVar##name##Fn(CShaderVar *pThis, float *pRegisters, uint uiSize)
//=============================================================================

//=============================================================================
// CShaderVar
//=============================================================================
class CShaderVar
{
private:
	tstring			m_sName;
	ShaderVarFn_t	m_pfnShaderVar;

	// CShaderVars should not be copied
	CShaderVar(CShaderVar&);
	CShaderVar& operator=(CShaderVar&);

public:
	~CShaderVar();
	CShaderVar(const tstring &sName, ShaderVarFn_t pfnCShaderVarCmd);

	const tstring&	GetName()			{ return m_sName; }

	bool	Get(float *pRegisters, uint uiSize)
	{
		return m_pfnShaderVar(this, pRegisters, uiSize);
	}
};
//=============================================================================
#endif //__C_SHADERVAR_H__
