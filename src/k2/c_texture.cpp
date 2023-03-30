// (C)2005 S2 Games
// c_texture.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_texture.h"
#include "i_resourcelibrary.h"
#include "c_bitmap.h"
#include "c_vid.h"
#include "c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
IResource*	AllocTexture(const tstring &sPath);
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
IResourceLibrary	g_ResLibTexture(RES_TEXTURE, _T("Textures"), CTexture::ResTypeName(), false, AllocTexture);
//=============================================================================

/*====================
  AllocTexture
  ====================*/
IResource*	AllocTexture(const tstring &sPath)
{
	return K2_NEW(ctx_Resources,  CTexture)(sPath);
}


/*====================
  CTexture::CTexture
  ====================*/
CTexture::CTexture(const tstring &sPath) :
IResource(sPath, TSNULL),
m_iIndex(-1),
m_iIndex2(-1),
m_eType(TEXTURE_2D),
m_iTextureFlags(0),
m_eFormat(TEXFMT_A8R8G8B8),
m_pBitmap(NULL),
m_bTranslucent(false)
{
}

CTexture::CTexture(const tstring &sPath, ETextureType eType, int iTextureFlags, ETextureFormat eFormat) :
IResource(sPath, TSNULL),
m_iIndex(-1),
m_iIndex2(-1),
m_eType(eType),
m_iTextureFlags(iTextureFlags),
m_eFormat(eFormat),
m_pBitmap(NULL),
m_bTranslucent(false)
{
}

CTexture::CTexture(const tstring &sPath, const CBitmap *pBitmap, ETextureType eType, int iTextureFlags, ETextureFormat eFormat) :
IResource(sPath, TSNULL),
m_iIndex(-1),
m_iIndex2(-1),
m_eType(eType),
m_iTextureFlags(iTextureFlags),
m_eFormat(eFormat),
m_pBitmap(pBitmap),
m_bTranslucent(false)
{
}


/*====================
  CTexture::Load
  ====================*/
int		CTexture::Load(uint uiIgnoreFlags, const char *pData, uint uiSize)
{
	PROFILE("CTexture::Load");

	try
	{
		// Dedicated servers don't need texture files so skip this and save some memory
		if (K2System.IsDedicatedServer() || K2System.IsServerManager())
			return false;
			
		if (!m_sPath.empty())
			Console.Res << "Loading ^gTexture^* " << SingleQuoteStr(m_sPath) << newl;
		else if (!m_sName.empty())
			Console.Res << "Loading ^gTexture^* " << SingleQuoteStr(m_sName) << newl;
		else
			EX_ERROR(_T("No path and no name!"));

		assert(pData == NULL);

		if (uiIgnoreFlags & RES_TEXTURE_IGNORE_ALL)
			Console.Res << _T(" [IGNORE_ALL]");

		if (~uiIgnoreFlags & RES_TEXTURE_IGNORE_ALL)
			Vid.RegisterTexture(this);
	}
	catch (CException &ex)
	{
		tstring sStr(m_sPath);
		if (m_sPath.empty())
			sStr = m_sName;
		ex.Process(_TS("CTexture::Load(") + sStr + _TS(") - "), THROW);
		return RES_LOAD_FAILED;
	}

	return 0;
}


/*====================
  CTexture::LoadNull

  Loads NULL checker texture
  ====================*/
bool	CTexture::LoadNull()
{
	try
	{
		CTexture *pChecker(g_ResourceManager.GetTexture(g_ResourceManager.GetCheckerTexture()));
		if (pChecker == NULL)
		{
			// Fallback just incase the checker texture hasn't been loaded yet by anything
			ResHandle hCheckerTexture(g_ResourceManager.Register(_T("$checker"), RES_TEXTURE));
			pChecker = g_ResourceManager.GetTexture(hCheckerTexture);
		}

		if (pChecker == NULL)
			EX_ERROR(_T("$checker not registered"));

		m_iIndex = pChecker->GetIndex();
	}
	catch (CException &ex)
	{
		ex.Process(_TS("CTexture::Load(") + m_sName + _TS(") - "), NO_THROW);
		EX_FATAL(_T("Texture NULL resource failure"));
	}

	return true;
}


/*====================
  CTexture::Free
  ====================*/
void	CTexture::Free()
{
	Vid.UnregisterTexture(this);
}
