// (C)2005 S2 Games
// c_proceduralregistry.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_proceduralregistry.h"
#include "c_procedural.h"
#include "d3d9f_texture.h"

#include "../k2/stringutils.h"
#include "../k2/c_texture.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CProceduralRegistry	*CProceduralRegistry::s_pInstance;
bool			CProceduralRegistry::s_bRequested;
bool			CProceduralRegistry::s_bReleased;

CProceduralRegistry	*pProceduralRegistry = CProceduralRegistry::GetInstance();
//=============================================================================


/*====================
  CProceduralRegistry::GetInstance
  ====================*/
CProceduralRegistry*	CProceduralRegistry::GetInstance()
{
	assert(!s_bReleased);

	if (s_pInstance == NULL)
	{
		assert(!s_bRequested);
		s_bRequested = true;
		s_pInstance = new CProceduralRegistry;
	}

	return s_pInstance;
}


/*====================
  CProceduralRegistry::Release
  ====================*/
void	CProceduralRegistry::Release()
{
	assert(!s_bReleased);

	if (s_pInstance != NULL)
		delete s_pInstance;

	s_bReleased = true;
}


/*====================
  CProceduralRegistry::Register
  ====================*/
void	CProceduralRegistry::Register(CProcedural *pProcedural)
{
	// Make sure there is no name collision
	ProceduralMap::iterator findit = m_mapProcedurals.find(pProcedural->GetName());
	if (findit != m_mapProcedurals.end())
	{
		Console.Err << _T("A procedural named ") << QuoteStr(pProcedural->GetName())
					<< _T(" already exists.") << newl;
		return;
	}

	m_mapProcedurals[pProcedural->GetName()] = pProcedural;
}


/*====================
  CProceduralRegistry::Unregister
  ====================*/
void	CProceduralRegistry::Unregister(const tstring &sName)
{
	ProceduralMap::iterator findit = m_mapProcedurals.find(sName);
	if (findit != m_mapProcedurals.end())
	{
		//Console.Dev << _T("Shader Variable ") << sName << _T(" has been unregistered.") << newl;
		m_mapProcedurals.erase(findit);
	}
}


#include "c_texturecache.h"

/*====================
  CProceduralRegistry::RegisterProcedurals
  ====================*/
void	CProceduralRegistry::RegisterProcedurals()
{
	for (ProceduralMap::iterator it(m_mapProcedurals.begin()); it != m_mapProcedurals.end(); ++it)
	{
		if (it->second->GetFlags() & TEX_ON_DEMAND)
			continue;

		tstring sName(_T("$") + it->second->GetName());

		TextureMap::iterator itFind(g_mapTextures.find(sName));
		if (itFind != g_mapTextures.end())
			continue;

		int iTexture2(-1);
		int iTexture(D3D_Register2DProcedural(*it->second, iTexture2));
		m_mapIndices[it->second->GetName()] = iTexture;
		g_mapTextures[sName] = STextureMapEntry(iTexture, iTexture2, INVALID_RESOURCE);

#if 0
		D3D_TextureDefine("DOWNSIZE", "0");
		D3D_TextureDefine("COMPRESSION", "0");

		if (!it->second->IsMipmaps())
			D3D_TextureDefine("MIPMAPS", "0");
		else
			D3D_TextureDefine("MIPMAPS", "1");

		g_TextureCache.ActivateNode(D3D_GetTextureDefinitionString());

		g_TextureCache.CacheTexture(sName, g_pTextures[iTexture]);
#endif
	}
}


/*====================
  CProceduralRegistry::GetTextureIndex
  ====================*/
int		CProceduralRegistry::GetTextureIndex(const tstring &sName)
{
	ProceduralIndexMap::iterator itFind(m_mapIndices.find(sName));
	if (itFind != m_mapIndices.end())
		return itFind->second;
	else
	{
		// Load on demand procedurals
		ProceduralMap::iterator itFind2(m_mapProcedurals.find(sName));
		if (itFind2 != m_mapProcedurals.end())
		{
			tstring sName(_T("$") + itFind2->second->GetName());

			TextureMap::iterator itFind3(g_mapTextures.find(sName));
			if (itFind3 != g_mapTextures.end())
				return itFind3->second.iIndex;

			int iTexture2(-1);
			int iTexture(D3D_Register2DProcedural(*itFind2->second, iTexture2));
			m_mapIndices[itFind2->second->GetName()] = iTexture;
			g_mapTextures[sName] = STextureMapEntry(iTexture, iTexture2, INVALID_RESOURCE);

			return iTexture;
		}

		return -1;
	}
}
