// (C)2005 S2 Games
// c_interfaceresource.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_interfaceresource.h"
#include "c_interface.h"
#include "c_xmlproc_interface.h"
#include "c_uimanager.h"

#include "../k2/i_resourcelibrary.h"
#include "../k2/c_xmlmanager.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
IResource*	AllocInterfaceResource(const tstring &sPath);
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
IResourceLibrary	g_ResLibInterfaceResource(RES_INTERFACE, _T("Interfaces"), CInterfaceResource::ResTypeName(), true, AllocInterfaceResource);
//=============================================================================

/*====================
  AllocInterfaceResource
  ====================*/
IResource*	AllocInterfaceResource(const tstring &sPath)
{
	return K2_NEW(ctx_Resources,  CInterfaceResource)(sPath);
}


/*====================
  CInterfaceResource::CInterfaceResource
  ====================*/
CInterfaceResource::CInterfaceResource(const tstring &sPath) :
IResource(sPath, TSNULL),
m_pInterface(NULL)
{
}


/*====================
  CInterfaceResource::Load
  ====================*/
int		CInterfaceResource::Load(uint uiIgnoreFlags, const char *pData, uint uiSize)
{
	PROFILE("CInterfaceResource::Load");

	// Dedicated servers don't need interface files so skip this and save some memory
	if (K2System.IsDedicatedServer() || K2System.IsServerManager())
		return false;

	int iResult(0);

	if (!m_sPath.empty())
		Console.Res << "Loading ^mInterface^* " << SingleQuoteStr(m_sPath) << newl;
	else if (!m_sName.empty())
		Console.Res << "Loading ^mInterface^* " << SingleQuoteStr(m_sName) << newl;
	else
		Console.Res << "Loading ^mUnknown Interface^*" << newl;

	if (!XMLManager.ReadBuffer(pData, uiSize, _T("interface"), this))
		iResult = RES_LOAD_FAILED;

	if (m_pInterface != NULL)
	{
		m_pInterface->SetFilename(m_sPath);
		m_pInterface->DoEvent(WEVENT_LOAD);
		m_pInterface->NeedsRefresh(true);
	}

	return iResult;
}


/*====================
  CInterfaceResource::Free
  ====================*/
void	CInterfaceResource::Free()
{
	SAFE_DELETE(m_pInterface);
}


/*====================
  CInterfaceResource::Allocate
  ====================*/
bool	CInterfaceResource::Allocate(const CWidgetStyle& style)
{
	m_pInterface = K2_NEW(ctx_Resources,  CInterface)(style);

	return m_pInterface != NULL;
}


/*====================
  CInterfaceResource::Reloaded
  ====================*/
void	CInterfaceResource::Reloaded()
{
	if (m_pInterface != NULL)
		m_pInterface->DoEvent(WEVENT_RELOAD);

	IResource::Reloaded();
}
