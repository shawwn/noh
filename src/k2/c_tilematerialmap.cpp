// (C)2005 S2 Games
// c_tilematerialmap.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_tilematerialmap.h"
#include "c_world.h"
#include "s_tile.h"
#include "c_buffer.h"
#include "c_resourcemanager.h"
//=============================================================================

/*====================
  CTileMaterialMap::~CTileMaterialMap
  ====================*/
CTileMaterialMap::~CTileMaterialMap()
{
    Release();
}


/*====================
  CTileMaterialMap::CTileMaterialMap
  ====================*/
CTileMaterialMap::CTileMaterialMap(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("TileMaterialMap"))
{
    for (int i(0); i < NUM_TERRAIN_LAYERS; ++i)
        m_pTiles[i] = nullptr;
}


/*====================
  CTileMaterialMap::Load
  ====================*/
bool    CTileMaterialMap::Load(CArchive &archive, const CWorld *pWorld)
{
    try
    {
        bool newFileFormat = false; // UTTAR: Filesize-optimized format...
        Release();
        m_pWorld = pWorld;
        if (m_pWorld == nullptr)
            EX_ERROR(_T("Invalid CWorld pointer"));

        CFileHandle hTileMaterialMap(m_sName, FILE_READ | FILE_BINARY, archive);
        if (!hTileMaterialMap.IsOpen())
            EX_ERROR(_T("Could not find TileMaterialMap in archive"));

        int iLayers = hTileMaterialMap.ReadInt32();
        int iWidth = hTileMaterialMap.ReadInt32();
        int iHeight = hTileMaterialMap.ReadInt32();

        // UTTAR: In order to keep full backward compatibility, I will differentiate
        // between the old and the new file formats based on iWidth being negative.
        if (iWidth < 0)
        {
            iWidth = -iWidth;
            newFileFormat = true;
        }

        if (iLayers != NUM_TERRAIN_LAYERS)
            EX_ERROR(_T("Invalid number of terrain layers"));

        if (iWidth != m_pWorld->GetTileWidth() || iHeight != m_pWorld->GetTileWidth())
            EX_ERROR(_T("Invalid dimensions for TileMaterialMap"));

        // Load tile arrays
        for (int iLayer(0); iLayer < NUM_TERRAIN_LAYERS; ++iLayer)
        {
            m_pTiles[iLayer] = K2_NEW_ARRAY(ctx_World, STileInternal, m_pWorld->GetTileArea());
            if (m_pTiles[iLayer] == nullptr)
                EX_ERROR(_T("Failed to allocate tile array for layer ") + XtoA(iLayer));

            if (newFileFormat) // UTTAR
            {
                // UTTAR: New optimized file format; use shorts instead of ints, no more unused vars, etc.
                for (int iTile(0); iTile < m_pWorld->GetTileArea(); ++iTile)
                {
                    m_pTiles[iLayer][iTile].iMaterialRef = hTileMaterialMap.ReadInt16();
                    m_pTiles[iLayer][iTile].iDiffuseRef = hTileMaterialMap.ReadInt16();
                    m_pTiles[iLayer][iTile].iNormalmapRef = hTileMaterialMap.ReadInt16();

                    // Maintain backwards compatibility
                    hTileMaterialMap.ReadFloat(); // fScale
                    hTileMaterialMap.ReadInt16(); // fRotation
                    hTileMaterialMap.ReadFloat(); // v2Offset.x
                    hTileMaterialMap.ReadFloat(); // v2Offset.y
                }
            }
            else
            {
                for (int iTile(0); iTile < m_pWorld->GetTileArea(); ++iTile)
                {
                    m_pTiles[iLayer][iTile].iMaterialRef = hTileMaterialMap.ReadInt32();
                    m_pTiles[iLayer][iTile].iDiffuseRef = hTileMaterialMap.ReadInt32();
                    m_pTiles[iLayer][iTile].iNormalmapRef = hTileMaterialMap.ReadInt32();

                    // Maintain even more backwards compatibility
                    hTileMaterialMap.ReadFloat(); // fScale
                    hTileMaterialMap.ReadFloat(); // fRotation
                    hTileMaterialMap.ReadFloat(); // v2Offset.x
                    hTileMaterialMap.ReadFloat(); // v2Offset.y
                    hTileMaterialMap.ReadFloat(); // v3Proj.x
                    hTileMaterialMap.ReadFloat(); // v3Proj.y
                    hTileMaterialMap.ReadFloat(); // v3Proj.z
                }
            }
        }
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CTileMaterialMap::Load() - "), NO_THROW);
        return false;
    }
}


/*====================
  CTileMaterialMap::Generate
  ====================*/
bool    CTileMaterialMap::Generate(const CWorld *pWorld)
{
    PROFILE("CTileFoliageMap::Generate");

    try
    {
        Release();
        m_pWorld = pWorld;
        if (m_pWorld == nullptr)
            EX_ERROR(_T("Invalid CWorld pointer"));

        uint iMaterialRef(m_pWorld->AddMaterial(g_ResourceManager.Register(_T("/world/terrain/materials/default.material"), RES_MATERIAL)));
        uint iDiffuseRef(m_pWorld->AddTexture(g_ResourceManager.Register(_T("$checker"), RES_TEXTURE)));
        uint iNormalmapRef(m_pWorld->AddTexture(g_ResourceManager.Register(_T("$flat_dull"), RES_TEXTURE)));
        g_ResourceManager.Register(_T("$green_smooth_checker"), RES_TEXTURE);
        g_ResourceManager.Register(_T("$red_smooth_checker"), RES_TEXTURE);
        g_ResourceManager.Register(_T("$yellow_smooth_checker"), RES_TEXTURE);
        g_ResourceManager.Register(_T("$blue_smooth_checker"), RES_TEXTURE);
        g_ResourceManager.Register(_T("$tile_norm"), RES_TEXTURE);
        g_ResourceManager.Register(_T("$pyrmid_norm"), RES_TEXTURE);
        g_ResourceManager.Register(_T("$smooth_checker"), RES_TEXTURE);

        // Allocate tile arrays
        for (int iLayer(0); iLayer < NUM_TERRAIN_LAYERS; ++iLayer)
        {
            m_pTiles[iLayer] = K2_NEW_ARRAY(ctx_World, STileInternal, m_pWorld->GetTileArea());
            if (m_pTiles[iLayer] == nullptr)
                EX_ERROR(_T("Failed to allocate tile array for layer ") + XtoA(iLayer));

            for (int iTile(0); iTile < m_pWorld->GetTileArea(); ++iTile)
            {
                m_pTiles[iLayer][iTile].iMaterialRef = iMaterialRef;
                m_pTiles[iLayer][iTile].iDiffuseRef = iDiffuseRef;
                m_pTiles[iLayer][iTile].iNormalmapRef = iNormalmapRef;
            }
        }

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CTileMaterialMap::Generate() - "), NO_THROW);
        return false;
    }
}


/*====================
  CTileMaterialMap::Serialize
  ====================*/
bool    CTileMaterialMap::Serialize(IBuffer *pBuffer)
{
    pBuffer->Clear();
    // UTTAR: Negative width shows that this is our new file format.
    (*pBuffer) << NUM_TERRAIN_LAYERS << -m_pWorld->GetTileWidth() << m_pWorld->GetTileWidth();

    for (int iLayer(0); iLayer < NUM_TERRAIN_LAYERS; ++iLayer)
    {
        for (int iTile(0); iTile < m_pWorld->GetTileArea(); ++iTile)
        {
            (*pBuffer) << (short)m_pTiles[iLayer][iTile].iMaterialRef;
            (*pBuffer) << (short)m_pTiles[iLayer][iTile].iDiffuseRef;
            (*pBuffer) << (short)m_pTiles[iLayer][iTile].iNormalmapRef;

            // Maintain backwards compatibility
            (*pBuffer) << float(0.0f); // fScale
            (*pBuffer) << ushort(0); // fRotation
            (*pBuffer) << float(0.0f); // v2Offset.x;
            (*pBuffer) << float(0.0f); // v2Offset.y;
        }
    }

    if (pBuffer->GetFaults())
        return false;

    return true;
}


/*====================
  CTileMaterialMap::SetUsage
  ====================*/
void    CTileMaterialMap::SetUsage()
{
    for (int iLayer(0); iLayer < NUM_TERRAIN_LAYERS; ++iLayer)
    {
        for (int iTile(0); iTile < m_pWorld->GetTileArea(); ++iTile)
        {
            m_pTiles[iLayer][iTile].iDiffuseRef = m_pWorld->GetTextureID(m_pWorld->GetTextureHandle(m_pTiles[iLayer][iTile].iDiffuseRef));
            m_pTiles[iLayer][iTile].iNormalmapRef = m_pWorld->GetTextureID(m_pWorld->GetTextureHandle(m_pTiles[iLayer][iTile].iNormalmapRef));

            m_pWorld->SetTextureIDUsed(m_pTiles[iLayer][iTile].iDiffuseRef);
            m_pWorld->SetTextureIDUsed(m_pTiles[iLayer][iTile].iNormalmapRef);
        }
    }
}


/*====================
  CTileMaterialMap::Release
  ====================*/
void    CTileMaterialMap::Release()
{
    for (int i(0); i < NUM_TERRAIN_LAYERS; ++i)
    {
        if (m_pTiles[i] != nullptr)
        {
            K2_DELETE_ARRAY(m_pTiles[i]);
            m_pTiles[i] = nullptr;
        }
    }
}


/*====================
  CTileMaterialMap::GetRegion
  ====================*/
bool    CTileMaterialMap::GetRegion(const CRecti &recArea, void *pDest, int iLayer) const
{
    assert(iLayer >= 0 && iLayer < NUM_TERRAIN_LAYERS);
    assert(recArea.IsNormalized());
    assert(m_pWorld->IsInBounds(recArea.left, recArea.top, TILE_SPACE));
    assert(m_pWorld->IsInBounds(recArea.right - 1, recArea.bottom - 1, TILE_SPACE));

    STile *pTileDest(static_cast<STile *>(pDest));
    STileInternal *pTileSource = &m_pTiles[iLayer][m_pWorld->GetTileIndex(recArea.left, recArea.top)];

    int iDestSpan = 0;
    int iSourceSpan = m_pWorld->GetTileWidth() - recArea.GetWidth();

    for (int y = 0; y < recArea.GetHeight(); ++y, pTileDest += iDestSpan, pTileSource += iSourceSpan)
    {
        for (int x = 0; x < recArea.GetWidth(); ++x, ++pTileDest, ++pTileSource)
        {
            // Translate Refs to ResHandles
            pTileDest->hMaterial = m_pWorld->GetMaterialHandle(pTileSource->iMaterialRef);
            pTileDest->hDiffuse = m_pWorld->GetTextureHandle(pTileSource->iDiffuseRef);
            pTileDest->hNormalmap = m_pWorld->GetTextureHandle(pTileSource->iNormalmapRef);
        }
    }

    return true;
}


/*====================
  CTileMaterialMap::SetRegion
  ====================*/
bool    CTileMaterialMap::SetRegion(const CRecti &recArea, void *pSource, int iLayer)
{
    assert(iLayer >= 0 && iLayer < NUM_TERRAIN_LAYERS);
    assert(m_pWorld->IsInBounds(recArea.left, recArea.top, TILE_SPACE));
    assert(m_pWorld->IsInBounds(recArea.right - 1, recArea.bottom - 1, TILE_SPACE));

    STile *pTileSource(static_cast<STile *>(pSource));
    STileInternal *pTileDest(&m_pTiles[iLayer][m_pWorld->GetTileIndex(recArea.left, recArea.top)]);

    int iSourceSpan = 0;
    int iDestSpan = m_pWorld->GetTileWidth() - recArea.GetWidth();

    for (int y = 0; y < recArea.GetHeight(); ++y, pTileDest += iDestSpan, pTileSource += iSourceSpan)
    {
        for (int x = 0; x < recArea.GetWidth(); ++x, ++pTileDest, ++pTileSource)
        {
            // Translate ResHandles to Refs
            pTileDest->iMaterialRef = (ushort)m_pWorld->AddMaterial(pTileSource->hMaterial);
            pTileDest->iDiffuseRef = (ushort)m_pWorld->AddTexture(pTileSource->hDiffuse);
            pTileDest->iNormalmapRef = (ushort)m_pWorld->AddTexture(pTileSource->hNormalmap);
        }
    }

    m_bChanged = true;
    return true;
}


/*====================
  CTileMaterialMap::GetTileMaterialID
  ====================*/
uint    CTileMaterialMap::GetTileMaterialID(int iX, int iY, int iLayer)
{
    assert(m_pWorld != nullptr);
    assert(m_pWorld->IsInBounds(iX, iY, TILE_SPACE));
    assert(iLayer >= 0 && iLayer < NUM_TERRAIN_LAYERS);
    return m_pTiles[iLayer][m_pWorld->GetTileIndex(iX, iY)].iMaterialRef;
}


/*====================
  CTileMaterialMap::GetTileDiffuseTextureID
  ====================*/
uint    CTileMaterialMap::GetTileDiffuseTextureID(int iX, int iY, int iLayer)
{
    assert(m_pWorld != nullptr);
    assert(m_pWorld->IsInBounds(iX, iY, TILE_SPACE));
    assert(iLayer >= 0 && iLayer < NUM_TERRAIN_LAYERS);
    return m_pTiles[iLayer][m_pWorld->GetTileIndex(iX, iY)].iDiffuseRef;
}


/*====================
  CTileMaterialMap::GetTileNormalmapTextureID
  ====================*/
uint    CTileMaterialMap::GetTileNormalmapTextureID(int iX, int iY, int iLayer)
{
    assert(m_pWorld != nullptr);
    assert(m_pWorld->IsInBounds(iX, iY, TILE_SPACE));
    assert(iLayer >= 0 && iLayer < NUM_TERRAIN_LAYERS);
    return m_pTiles[iLayer][m_pWorld->GetTileIndex(iX, iY)].iNormalmapRef;
}
