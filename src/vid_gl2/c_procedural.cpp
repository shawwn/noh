// (C)2008 S2 Games
// c_procedural.cpp
//
// Self registering procedural textures
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_procedural.h"
#include "c_proceduralregistry.h"

#include "../k2/c_cmd.h"
//=============================================================================

/*====================
  CProcedural::CProcedural
  ====================*/
CProcedural::CProcedural(const tstring &sName, int iWidth, int iHeight, ETextureFormat eFormat, int iFlags, ProceduralFn_t pfnProceduralCmd) :
m_sName(sName),
m_iWidth(iWidth),
m_iHeight(iHeight),
m_eFormat(eFormat),
m_iFlags(iFlags),
m_pfnProcedural(pfnProceduralCmd),
m_pfnProceduralMipmaps(NULL)
{
	if (m_pfnProcedural == NULL)
		K2System.Error(_T("Tried to register a Procedural with a NULL function."));

	CProceduralRegistry::GetInstance()->Register(this);
}


/*====================
  CProcedural::CProcedural
  ====================*/
CProcedural::CProcedural(const tstring &sName, int iWidth, int iHeight, ETextureFormat eFormat, int iFlags, ProceduralMipmapsFn_t pfnProceduralCmd) :
m_sName(sName),
m_iWidth(iWidth),
m_iHeight(iHeight),
m_eFormat(eFormat),
m_iFlags(iFlags),
m_pfnProcedural(NULL),
m_pfnProceduralMipmaps(pfnProceduralCmd)
{
	if (m_pfnProceduralMipmaps == NULL)
		K2System.Error(_T("Tried to register a Procedural with a NULL function."));

	CProceduralRegistry::GetInstance()->Register(this);
}


/*====================
  CProcedural::~CProcedural
  ====================*/
CProcedural::~CProcedural()
{
	// If the registry is still valid, unregister the uicmd
	// This is important for any actions declared in a client dll that
	// is being unloaded
	if (!CProceduralRegistry::IsReleased())
		CProceduralRegistry::GetInstance()->Unregister(m_sName);
}


/*====================
  CProcedural::Get
  ====================*/
CVec4f	CProcedural::Get(float fU, float fV) const
{
	return m_pfnProcedural(this, fU, fV);
}


/*====================
  CProcedural::Get
  ====================*/
CVec4f	CProcedural::Get(float fU, float fV, int iLevel) const
{
	return m_pfnProceduralMipmaps(this, fU, fV, iLevel);
}


/*--------------------
  cmdProceduralList

  prints a list of all procedurals
  --------------------*/
CMD(ProceduralList)
{
	int iNumFound(0);

	const ProceduralMap &lVars = CProceduralRegistry::GetInstance()->GetProceduralMap();

	// Print shader variables
	for (ProceduralMap::const_iterator it(lVars.begin()); it != lVars.end(); ++it)
	{
		if (vArgList.size() == 0 || it->second->GetName().find(vArgList[0]) != string::npos)
		{
			Console << it->second->GetName() << newl;
			++iNumFound;
		}
	}

	Console << newl << iNumFound << _T(" matching procedurals found") << newl;

	return true;
}
