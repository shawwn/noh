// (C)2008 S2 Games
// c_posteffect.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_posteffect.h"

#include "i_resourcelibrary.h"
#include "c_xmlmanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
IResource*	AllocPostEffect(const tstring &sPath)
{
	return K2_NEW(ctx_Resources,  CPostEffect)(sPath);
}

IResourceLibrary	g_ResLibPostEffect(RES_POST_EFFECT, _T("Post Effects"), CPostEffect::ResTypeName(), true, AllocPostEffect);
//=============================================================================

/*====================
  CPostEffect::Load
  ====================*/
int		CPostEffect::Load(uint uiIgnoreFlags, const char *pData, uint uiSize)
{
	PROFILE("CPostEffect::Load");

	Console.Res << _T("Loading ^970Post effect^*: ") << m_sPath << newl;

	// Process the XML
	if (!XMLManager.ReadBuffer(pData, uiSize, _T("posteffect"), this))
	{
		Console.Warn << _T("CPostEffect::Load(") + m_sPath + _T(") - couldn't read XML") << newl;
		return RES_LOAD_FAILED;
	}

	return 0;
}
