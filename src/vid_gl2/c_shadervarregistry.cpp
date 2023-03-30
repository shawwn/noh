// (C)2005 S2 Games
// c_shadervarregistry.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_shadervarregistry.h"
#include "c_shadervar.h"

#include "../k2/stringutils.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CShaderVarRegistry	*CShaderVarRegistry::s_pInstance;
bool			CShaderVarRegistry::s_bRequested;
bool			CShaderVarRegistry::s_bReleased;

CShaderVarRegistry	*pShaderVarRegistry = CShaderVarRegistry::GetInstance();
//=============================================================================


/*====================
  CShaderVarRegistry::GetInstance
  ====================*/
CShaderVarRegistry*	CShaderVarRegistry::GetInstance()
{
	assert(!s_bReleased);

	if (s_pInstance == NULL)
	{
		assert(!s_bRequested);
		s_bRequested = true;
		s_pInstance = K2_NEW(ctx_GL2,    CShaderVarRegistry);
	}

	return s_pInstance;
}


/*====================
  CShaderVarRegistry::Release
  ====================*/
void	CShaderVarRegistry::Release()
{
	assert(!s_bReleased);

	if (s_pInstance != NULL)
		K2_DELETE(s_pInstance);

	s_bReleased = true;
}


/*====================
  CShaderVarRegistry::Register
  ====================*/
void	CShaderVarRegistry::Register(CShaderVar *pShaderVar)
{
	// Make sure there is no name collision
	ShaderVarMap::iterator findit = m_mapShaderVars.find(pShaderVar->GetName());
	if (findit != m_mapShaderVars.end())
	{
		Console.Err << _T("A shader variable named ") << QuoteStr(pShaderVar->GetName())
					<< _T(" already exists.") << newl;
		return;
	}

	m_mapShaderVars[pShaderVar->GetName()] = pShaderVar;
}


/*====================
  CShaderVarRegistry::Unregister
  ====================*/
void	CShaderVarRegistry::Unregister(const tstring &sName)
{
	ShaderVarMap::iterator findit = m_mapShaderVars.find(sName);
	if (findit != m_mapShaderVars.end())
	{
		//Console.Dev << _T("Shader Variable ") << sName << _T(" has been unregistered.") << newl;
		m_mapShaderVars.erase(findit);
	}
}
