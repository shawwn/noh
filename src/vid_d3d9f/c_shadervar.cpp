// (C)2005 S2 Games
// c_shadervar.cpp
//
// Self registering shader variable controllers
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_shadervar.h"
#include "c_shadervarregistry.h"

#include "../k2/c_cmd.h"
//=============================================================================

/*====================
  CShaderVar::CShaderVar
  ====================*/
CShaderVar::CShaderVar(const tstring &sName, ShaderVarFn_t pfnShaderVarCmd) :
m_sName(sName),
m_pfnShaderVar(pfnShaderVarCmd)
{
	if (m_pfnShaderVar == NULL)
		K2System.Error(_T("Tried to register a ShaderVar with a NULL function."));

	CShaderVarRegistry::GetInstance()->Register(this);
}


/*====================
  CShaderVar::~CShaderVar
  ====================*/
CShaderVar::~CShaderVar()
{
	// If the registry is still valid, unregister the uicmd
	// This is important for any actions declared in a client dll that
	// is being unloaded
	if (!CShaderVarRegistry::IsReleased())
		CShaderVarRegistry::GetInstance()->Unregister(m_sName);
}


/*--------------------
  cmdShaderVarList

  prints a list of all shader variables
  --------------------*/
CMD(ShaderVarList)
{
	int iNumFound(0);

	const ShaderVarMap &lVars = CShaderVarRegistry::GetInstance()->GetShaderVarMap();

	// Print shader variables
	for (ShaderVarMap::const_iterator it(lVars.begin()); it != lVars.end(); ++it)
	{
		if (vArgList.size() == 0 || it->second->GetName().find(vArgList[0]) != tstring::npos)
		{
			Console << it->second->GetName() << newl;
			++iNumFound;
		}
	}

	Console << newl << iNumFound << _T(" matching ShaderVars found") << newl;

	return true;
}
