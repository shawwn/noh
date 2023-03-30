// (C)2006 S2 Games
// c_resourcereference.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_resourcereference.h"
#include "i_resourcelibrary.h"
#include "c_vid.h"
#include "c_resourceinfo.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
IResource*	AllocResourceReference(const tstring &sPath);
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
IResourceLibrary	g_ResLibResourceReference(RES_REFERENCE, _T("Resource References"), _T("{reference}"), false, AllocResourceReference);
//=============================================================================

/*====================
  AllocResourceReference
  ====================*/
IResource*	AllocResourceReference(const tstring &sPath)
{
	return K2_NEW(ctx_Resources,  CResourceReference)(sPath);
}


/*====================
  CResourceReference::~CResourceReference
  ====================*/
CResourceReference::~CResourceReference()
{
	//Free();
}


/*====================
  CResourceReference::CResourceReference
  ====================*/
CResourceReference::CResourceReference(const tstring &sPath) :
IResource(sPath, TSNULL),
m_hReference(INVALID_RESOURCE)
{
}

CResourceReference::CResourceReference(const tstring &sName, ResHandle hReference) :
IResource(TSNULL, sName),
m_hReference(hReference)
{
}


/*====================
  CResourceReference::SetReference
  ====================*/
void	CResourceReference::SetReference(ResHandle hReference)
{
	m_hReference = hReference;
}


/*====================
  CResourceReference::Load
  ====================*/
int		CResourceReference::Load(uint uiIgnoreFlags, const char *pData, uint uiSize)
{
	PROFILE("CResourceReference::Load");

	try
	{
		if (!m_sPath.empty())
			Console.Res << "Loading ResourceReference " << SingleQuoteStr(m_sPath) << newl;
		else if (!m_sName.empty())
			Console.Res << "Loading ResourceReference " << SingleQuoteStr(m_sName) << newl;
		else
			Console.Res << "Loading Unknown ResourceReference" << newl;

		if (m_hReference == INVALID_RESOURCE)
		{
			// TODO: Load a NULL Resource instead...
		}
	}
	catch (CException &ex)
	{
		SAFE_DELETE_ARRAY(m_pData);
		ex.Process(_TS("CResourceReference::Load(") + m_sName + _TS(") - "), NO_THROW);
		return RES_LOAD_FAILED;
	}

	SAFE_DELETE_ARRAY(m_pData);

	return 0;
}


/*====================
  CResourceReference::Free
  ====================*/
void	CResourceReference::Free()
{
}
