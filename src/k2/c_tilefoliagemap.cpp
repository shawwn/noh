// (C)2006 S2 Games
// c_tilefoliagemap.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_tilefoliagemap.h"
#include "c_world.h"
#include "s_foliagetile.h"
#include "c_texture.h"
#include "c_buffer.h"
#include "c_resourcemanager.h"
//=============================================================================

/*====================
  CTileFoliageMap::~CTileFoliageMap
  ====================*/
CTileFoliageMap::~CTileFoliageMap()
{
	Release();
}


/*====================
  CTileFoliageMap::CTileFoliageMap
  ====================*/
CTileFoliageMap::CTileFoliageMap(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("TileFoliageMap"))
{
	for (int iLayer(0); iLayer < NUM_FOLIAGE_LAYERS; ++iLayer)
		m_pFoliageTiles[iLayer] = NULL;
}


/*====================
  CTileFoliageMap::Load
  ====================*/
bool	CTileFoliageMap::Load(CArchive &archive, const CWorld *pWorld)
{
	try
	{
		Release();
		m_pWorld = pWorld;
		if (m_pWorld == NULL)
			EX_ERROR(_T("Invalid CWorld pointer"));

		CFileHandle	hTileFoliageMap(m_sName, FILE_READ | FILE_BINARY, archive);
		if (!hTileFoliageMap.IsOpen())
			EX_ERROR(_T("Could not find TileFoliageMaterialMap in archive"));

		int iLayers = hTileFoliageMap.ReadInt32();
		int iWidth = hTileFoliageMap.ReadInt32();
		int iHeight = hTileFoliageMap.ReadInt32();

		if (iLayers != NUM_FOLIAGE_LAYERS)
			EX_ERROR(_T("Invalid number of foliage layers"));

		if (iWidth != m_pWorld->GetTileWidth() || iHeight != m_pWorld->GetTileWidth())
			EX_ERROR(_T("Invalid dimensions for TileFoliageMap"));

		size_t zSize(hTileFoliageMap.GetLength() / m_pWorld->GetTileArea() / iLayers);

		// Load tile arrays
		for (int iLayer(0); iLayer < NUM_FOLIAGE_LAYERS; ++iLayer)
		{	
			m_pFoliageTiles[iLayer] = K2_NEW_ARRAY(ctx_World, SFoliageTile, m_pWorld->GetTileArea());
			
			if (m_pFoliageTiles[iLayer] == NULL)
				EX_ERROR(_T("Failed to allocate foliage tile array"));

			if (zSize == 8) // HACK: Old format
			{
				for (int iTile(0); iTile < m_pWorld->GetTileArea(); ++iTile)
				{
					m_pFoliageTiles[iLayer][iTile].iMaterialRef = hTileFoliageMap.ReadInt32();
					m_pFoliageTiles[iLayer][iTile].iTextureRef = hTileFoliageMap.ReadInt32();
					m_pFoliageTiles[iLayer][iTile].yNumCrossQuads = 2;
					m_pFoliageTiles[iLayer][iTile].yFlags = 0;
				}
			}
			else
			{
				for (int iTile(0); iTile < m_pWorld->GetTileArea(); ++iTile)
				{
					m_pFoliageTiles[iLayer][iTile].iMaterialRef = hTileFoliageMap.ReadInt32();
					m_pFoliageTiles[iLayer][iTile].iTextureRef = hTileFoliageMap.ReadInt32();
					m_pFoliageTiles[iLayer][iTile].yNumCrossQuads = hTileFoliageMap.ReadByte();
					m_pFoliageTiles[iLayer][iTile].yFlags = hTileFoliageMap.ReadByte();
				}
			}
		}
		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CTileFoliageMap::Load() - "), NO_THROW);
		return false;
	}
}


/*====================
  CTileFoliageMap::Generate
  ====================*/
bool	CTileFoliageMap::Generate(const CWorld *pWorld)
{
	PROFILE("CTileFoliageMap::Generate");

	try
	{
		Release();
		m_pWorld = pWorld;
		if (m_pWorld == NULL)
			EX_ERROR(_T("Invalid CWorld pointer"));

		uint iMaterialRef(m_pWorld->AddMaterial(g_ResourceManager.Register(_T("/world/foliage/materials/default.material"), RES_MATERIAL)));
		uint iTextureRef(m_pWorld->AddTexture(g_ResourceManager.Register(_T("$checker"), RES_TEXTURE)));

		for (int iLayer(0); iLayer < NUM_FOLIAGE_LAYERS; ++iLayer)
		{
			m_pFoliageTiles[iLayer] = K2_NEW_ARRAY(ctx_World, SFoliageTile, m_pWorld->GetTileArea());
			
			if (m_pFoliageTiles[iLayer] == NULL)
				EX_ERROR(_T("Failed to allocate foliage material array"));

			for (int iTile(0); iTile < m_pWorld->GetTileArea(); ++iTile)
			{
				m_pFoliageTiles[iLayer][iTile].iMaterialRef = iMaterialRef;
				m_pFoliageTiles[iLayer][iTile].iTextureRef = iTextureRef;
				m_pFoliageTiles[iLayer][iTile].yNumCrossQuads = 2;
				m_pFoliageTiles[iLayer][iTile].yFlags = 0;
			}
		}

		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CTileFoliageMap::Generate() - "), NO_THROW);
		return false;
	}
}


/*====================
  CTileFoliageMap::Serialize
  ====================*/
bool	CTileFoliageMap::Serialize(IBuffer *pBuffer)
{
	pBuffer->Clear();
	(*pBuffer) << NUM_FOLIAGE_LAYERS << m_pWorld->GetTileWidth() << m_pWorld->GetTileWidth();

	for (int iLayer(0); iLayer < NUM_FOLIAGE_LAYERS; ++iLayer)
	{
		for (int iTile(0); iTile < m_pWorld->GetTileArea(); ++iTile)
		{
			(*pBuffer) << m_pFoliageTiles[iLayer][iTile].iMaterialRef;
			(*pBuffer) << m_pFoliageTiles[iLayer][iTile].iTextureRef;
			(*pBuffer) << m_pFoliageTiles[iLayer][iTile].yNumCrossQuads;
			(*pBuffer) << m_pFoliageTiles[iLayer][iTile].yFlags;
		}
	}

	if (pBuffer->GetFaults())
		return false;

	return true;
}


/*====================
  CTileFoliageMap::Release
  ====================*/
void	CTileFoliageMap::Release()
{
	for (int iLayer(0); iLayer < NUM_FOLIAGE_LAYERS; ++iLayer)
	{
		if (m_pFoliageTiles[iLayer] != NULL)
			K2_DELETE_ARRAY(m_pFoliageTiles[iLayer]);
		
		m_pFoliageTiles[iLayer] = NULL;
	}
}


/*====================
  CTileFoliageMap::SetUsage
  ====================*/
void	CTileFoliageMap::SetUsage()
{
	for (int iLayer(0); iLayer < NUM_FOLIAGE_LAYERS; ++iLayer)
	{
		for (int iTile(0); iTile < m_pWorld->GetTileArea(); ++iTile)
		{
			m_pFoliageTiles[iLayer][iTile].iTextureRef = m_pWorld->GetTextureID(m_pWorld->GetTextureHandle(m_pFoliageTiles[iLayer][iTile].iTextureRef));

			m_pWorld->SetTextureIDUsed(m_pFoliageTiles[iLayer][iTile].iTextureRef);
		}
	}
}


/*====================
  CTileFoliageMap::GetRegion
  ====================*/
bool	CTileFoliageMap::GetRegion(const CRecti &recArea, void *pDest, int iLayer) const
{
	assert(iLayer >= 0 && iLayer < NUM_FOLIAGE_LAYERS);
	assert(recArea.IsNormalized());
	assert(m_pWorld->IsInBounds(recArea.left, recArea.top, TILE_SPACE));
	assert(m_pWorld->IsInBounds(recArea.right - 1, recArea.bottom - 1, TILE_SPACE));

	SFoliageTile *pTileDest(static_cast<SFoliageTile *>(pDest));
	SFoliageTile *pTileSource(&m_pFoliageTiles[iLayer][m_pWorld->GetTileIndex(recArea.left, recArea.top)]);

	int iDestSpan = 0;
	int iSourceSpan = m_pWorld->GetTileWidth() - recArea.GetWidth();

	for (int y = 0; y < recArea.GetHeight(); ++y, pTileDest += iDestSpan, pTileSource += iSourceSpan)
	{
		for (int x = 0; x < recArea.GetWidth(); ++x, ++pTileDest, ++pTileSource)
		{
			*pTileDest = *pTileSource;

            // Translate Refs to ResHandles
			pTileDest->iMaterialRef = m_pWorld->GetMaterialHandle(pTileDest->iMaterialRef);
			pTileDest->iTextureRef = m_pWorld->GetTextureHandle(pTileDest->iTextureRef);
		}
	}

	return true;
}


/*====================
  CTileFoliageMap::SetRegion
  ====================*/
bool	CTileFoliageMap::SetRegion(const CRecti &recArea, void *pSource, int iLayer)
{
	assert(iLayer >= 0 && iLayer < NUM_FOLIAGE_LAYERS);
	assert(m_pWorld->IsInBounds(recArea.left, recArea.top, TILE_SPACE));
	assert(m_pWorld->IsInBounds(recArea.right - 1, recArea.bottom - 1, TILE_SPACE));

	SFoliageTile *pTileSource(static_cast<SFoliageTile *>(pSource));
	SFoliageTile *pTileDest(&m_pFoliageTiles[iLayer][m_pWorld->GetTileIndex(recArea.left, recArea.top)]);

	int iSourceSpan = 0;
	int iDestSpan = m_pWorld->GetTileWidth() - recArea.GetWidth();

	for (int y = 0; y < recArea.GetHeight(); ++y, pTileDest += iDestSpan, pTileSource += iSourceSpan)
	{
		for (int x = 0; x < recArea.GetWidth(); ++x, ++pTileDest, ++pTileSource)
		{
			*pTileDest = *pTileSource;

            // Translate ResHandles to Refs
			pTileDest->iMaterialRef = m_pWorld->AddMaterial(pTileDest->iMaterialRef);
			pTileDest->iTextureRef = m_pWorld->AddTexture(pTileDest->iTextureRef);
		}
	}

	m_bChanged = true;
	return true;
}


/*====================
  CTileFoliageMap::GetTileMaterialID
  ====================*/
uint	CTileFoliageMap::GetTileMaterialID(int iX, int iY, int iLayer)
{
	assert(iLayer >= 0 && iLayer < NUM_FOLIAGE_LAYERS);
	assert(m_pWorld != NULL);
	assert(m_pWorld->IsInBounds(iX, iY, TILE_SPACE));
	return m_pFoliageTiles[iLayer][m_pWorld->GetTileIndex(iX, iY)].iMaterialRef;
}


/*====================
  CTileFoliageMap::GetTileTextureID
  ====================*/
uint	CTileFoliageMap::GetTileTextureID(int iX, int iY, int iLayer)
{
	assert(iLayer >= 0 && iLayer < NUM_TERRAIN_LAYERS);
	assert(m_pWorld != NULL);
	assert(m_pWorld->IsInBounds(iX, iY, TILE_SPACE));
	return m_pFoliageTiles[iLayer][m_pWorld->GetTileIndex(iX, iY)].iTextureRef;
}


/*====================
  CTileFoliageMap::GetNumCrossQuads
  ====================*/
byte	CTileFoliageMap::GetNumCrossQuads(int iX, int iY, int iLayer)
{
	assert(iLayer >= 0 && iLayer < NUM_TERRAIN_LAYERS);
	assert(m_pWorld != NULL);
	assert(m_pWorld->IsInBounds(iX, iY, TILE_SPACE));
	return m_pFoliageTiles[iLayer][m_pWorld->GetTileIndex(iX, iY)].yNumCrossQuads;
}


/*====================
  CTileFoliageMap::GetFlags
  ====================*/
byte	CTileFoliageMap::GetFlags(int iX, int iY, int iLayer)
{
	assert(iLayer >= 0 && iLayer < NUM_TERRAIN_LAYERS);
	assert(m_pWorld != NULL);
	assert(m_pWorld->IsInBounds(iX, iY, TILE_SPACE));
	return m_pFoliageTiles[iLayer][m_pWorld->GetTileIndex(iX, iY)].yFlags;
}
