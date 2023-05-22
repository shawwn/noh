// (C)2006 S2 Games
// c_vertexfoliagemap.h
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_vertexfoliagemap.h"
#include "c_world.h"
#include "c_buffer.h"
//=============================================================================

/*====================
  CVertexFoliageMap::~CVertexFoliageMap
  ====================*/
CVertexFoliageMap::~CVertexFoliageMap()
{
    Release();
}


/*====================
  CVertexFoliageMap::CVertexFoliageMap
  ====================*/
CVertexFoliageMap::CVertexFoliageMap(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("VertexFoliageMap"))
{
    for (int iLayer(0); iLayer < NUM_FOLIAGE_LAYERS; ++iLayer)
        m_pFoliageVertices[iLayer] = nullptr;
}


/*====================
  CVertexFoliageMap::Load
  ====================*/
bool    CVertexFoliageMap::Load(CArchive &archive, const CWorld *pWorld)
{
    try
    {
        bool newFileFormat = false; // UTTAR: Filesize-optimized format...
        Release();

        m_pWorld = pWorld;
        if (m_pWorld == nullptr)
            EX_ERROR(_T("Invalid CWorld pointer"));

        CFileHandle hVertexFoliageMap(m_sName, FILE_READ | FILE_BINARY, archive);
        if (!hVertexFoliageMap.IsOpen())
            EX_ERROR(_T("Could not find VertexFoliageMap in archive"));

        // Read the dimensions
        int iLayers = hVertexFoliageMap.ReadInt32();
        int iWidth = hVertexFoliageMap.ReadInt32();
        int iHeight = hVertexFoliageMap.ReadInt32();

        // UTTAR: In order to keep full backward compatibility, I will differentiate
        // between the old and the new file formats based on iWidth being negative.
        if (iWidth < 0)
        {
            iWidth = -iWidth;
            newFileFormat = true;
        }

        if (iLayers != NUM_FOLIAGE_LAYERS)
            EX_ERROR(_T("Invalid number of foliage layers"));

        if (iWidth != m_pWorld->GetGridWidth() || iHeight != m_pWorld->GetGridHeight())
            EX_ERROR(_T("Invalid VertexFoliageMap dimensions"));

        if (hVertexFoliageMap.IsEOF())
            EX_ERROR(_T("Truncated VertexFoliageMap"));

        // Read the VertexFoliageMap
        if (newFileFormat)
        {
            // UTTAR: New optimized file format; use shorts instead of floats!
            for (int iLayer(0); iLayer < NUM_FOLIAGE_LAYERS; ++iLayer)
            {
                m_pFoliageVertices[iLayer] = K2_NEW_ARRAY(ctx_World, SFoliageVertexEntry, m_pWorld->GetGridArea());

                for (int iTile(0); iTile < m_pWorld->GetGridArea(); ++iTile)
                {
                    m_pFoliageVertices[iLayer][iTile].fDensity = (float)(hVertexFoliageMap.ReadInt16())/32.0f;
                    m_pFoliageVertices[iLayer][iTile].v3Size.x = (float)(hVertexFoliageMap.ReadByte())/4.0f;
                    m_pFoliageVertices[iLayer][iTile].v3Size.y = (float)(hVertexFoliageMap.ReadByte())/4.0f;
                    m_pFoliageVertices[iLayer][iTile].v3Size.z = (float)(hVertexFoliageMap.ReadByte())/4.0f;
                    m_pFoliageVertices[iLayer][iTile].v3Variance.x = (float)(hVertexFoliageMap.ReadByte())/4.0f;
                    m_pFoliageVertices[iLayer][iTile].v3Variance.y = (float)(hVertexFoliageMap.ReadByte())/4.0f;
                    m_pFoliageVertices[iLayer][iTile].v3Variance.z = (float)(hVertexFoliageMap.ReadByte())/4.0f;
                    m_pFoliageVertices[iLayer][iTile].v3Color.x = (float)(hVertexFoliageMap.ReadByte())/255.0f;
                    m_pFoliageVertices[iLayer][iTile].v3Color.y = (float)(hVertexFoliageMap.ReadByte())/255.0f;
                    m_pFoliageVertices[iLayer][iTile].v3Color.z = (float)(hVertexFoliageMap.ReadByte())/255.0f;
                }
            }
        }
        else
        {
            for (int iLayer(0); iLayer < NUM_FOLIAGE_LAYERS; ++iLayer)
            {
                m_pFoliageVertices[iLayer] = K2_NEW_ARRAY(ctx_World, SFoliageVertexEntry, m_pWorld->GetGridArea());

                for (int iTile(0); iTile < m_pWorld->GetGridArea(); ++iTile)
                {
                    m_pFoliageVertices[iLayer][iTile].fDensity = hVertexFoliageMap.ReadFloat();
                    m_pFoliageVertices[iLayer][iTile].v3Size.x = hVertexFoliageMap.ReadFloat();
                    m_pFoliageVertices[iLayer][iTile].v3Size.y = hVertexFoliageMap.ReadFloat();
                    m_pFoliageVertices[iLayer][iTile].v3Size.z = hVertexFoliageMap.ReadFloat();
                    m_pFoliageVertices[iLayer][iTile].v3Variance.x = hVertexFoliageMap.ReadFloat();
                    m_pFoliageVertices[iLayer][iTile].v3Variance.y = hVertexFoliageMap.ReadFloat();
                    m_pFoliageVertices[iLayer][iTile].v3Variance.z = hVertexFoliageMap.ReadFloat();
                    m_pFoliageVertices[iLayer][iTile].v3Color.x = hVertexFoliageMap.ReadFloat();
                    m_pFoliageVertices[iLayer][iTile].v3Color.y = hVertexFoliageMap.ReadFloat();
                    m_pFoliageVertices[iLayer][iTile].v3Color.z = hVertexFoliageMap.ReadFloat();
                }
            }
        }

        m_bChanged = false;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CVertexFoliageMap::Load() - "), NO_THROW);
        return false;
    }

    return true;
}


/*====================
  CVertexFoliageMap::Generate
  ====================*/
bool    CVertexFoliageMap::Generate(const CWorld *pWorld)
{
    PROFILE("CVertexFoliageMap::Generate");

    try
    {
        Release();

        m_pWorld = pWorld;
        if (m_pWorld == nullptr)
            EX_ERROR(_T("Invalid CWorld pointer"));

        for (int iLayer(0); iLayer < NUM_FOLIAGE_LAYERS; ++iLayer)
        {
            m_pFoliageVertices[iLayer] = K2_NEW_ARRAY(ctx_World, SFoliageVertexEntry, m_pWorld->GetGridArea());
            if (m_pFoliageVertices[iLayer] == nullptr)
                EX_ERROR(_T("Failed to allocate foliage vert map for layer") + XtoA(iLayer));
            MemManager.Set(m_pFoliageVertices[iLayer], 0, m_pWorld->GetGridArea() * sizeof(SFoliageVertexEntry));

            for (int iTile(0); iTile < m_pWorld->GetGridArea(); ++iTile)
            {
                m_pFoliageVertices[iLayer][iTile].fDensity = 0.0f;
                m_pFoliageVertices[iLayer][iTile].v3Size = CVec3f(32.0f, 32.0f, 0.0f);
                m_pFoliageVertices[iLayer][iTile].v3Variance = CVec3f(0.0f, 0.0f, 0.0f);
                m_pFoliageVertices[iLayer][iTile].v3Color = CVec3f(1.0f, 1.0f, 1.0f);
            }
        }

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CVertexFoliageMap::Generate() - "), NO_THROW);
        return false;
    }
}


/*====================
  CVertexFoliageMap::Serialize
  ====================*/
bool    CVertexFoliageMap::Serialize(IBuffer *pBuffer)
{
    pBuffer->Clear();
    // UTTAR: Negative width shows that this is our new file format.
    // This file format uses SHORTS rather than floats to optimize download size.
    (*pBuffer) << NUM_FOLIAGE_LAYERS << -m_pWorld->GetGridWidth() << m_pWorld->GetGridWidth();

    for (int iLayer(0); iLayer < NUM_FOLIAGE_LAYERS; ++iLayer)
    {
        for (int iTile(0); iTile < m_pWorld->GetGridArea(); ++iTile)
        {
            (*pBuffer) << (short)MIN(65535.0f, (m_pFoliageVertices[iLayer][iTile].fDensity*32.0f));
            (*pBuffer) << (byte)MIN(255.0f, MAX(0.0f, m_pFoliageVertices[iLayer][iTile].v3Size.x*4.0f));
            (*pBuffer) << (byte)MIN(255.0f, MAX(0.0f, m_pFoliageVertices[iLayer][iTile].v3Size.y*4.0f));
            (*pBuffer) << (byte)MIN(255.0f, MAX(0.0f, m_pFoliageVertices[iLayer][iTile].v3Size.z*4.0f));
            (*pBuffer) << (byte)MIN(255.0f, ABS(m_pFoliageVertices[iLayer][iTile].v3Variance.x*4.0f));
            (*pBuffer) << (byte)MIN(255.0f, ABS(m_pFoliageVertices[iLayer][iTile].v3Variance.y*4.0f));
            (*pBuffer) << (byte)MIN(255.0f, ABS(m_pFoliageVertices[iLayer][iTile].v3Variance.z*4.0f));
            (*pBuffer) << (byte)MIN(255.0f, MAX(0.0f, m_pFoliageVertices[iLayer][iTile].v3Color.x*255.0f));
            (*pBuffer) << (byte)MIN(255.0f, MAX(0.0f, m_pFoliageVertices[iLayer][iTile].v3Color.y*255.0f));
            (*pBuffer) << (byte)MIN(255.0f, MAX(0.0f, m_pFoliageVertices[iLayer][iTile].v3Color.z*255.0f));
        }
    }

    if (pBuffer->GetFaults())
        return false;

    return true;
}


/*====================
  CVertexFoliageMap::Release
  ====================*/
void    CVertexFoliageMap::Release()
{
    m_pWorld = nullptr;

    for (int iLayer(0); iLayer < NUM_FOLIAGE_LAYERS; ++iLayer)
    {
        if (m_pFoliageVertices[iLayer] != nullptr)
            K2_DELETE_ARRAY(m_pFoliageVertices[iLayer]);
        m_pFoliageVertices[iLayer] = nullptr;
    }
}


/*====================
  CVertexFoliageMap::GetRegion
  ====================*/
bool    CVertexFoliageMap::GetRegion(const CRecti &recArea, void *pDest, int iLayer) const
{
    assert(iLayer >= 0 && iLayer < NUM_FOLIAGE_LAYERS);
    assert(recArea.IsNormalized());
    assert(m_pWorld->IsInBounds(recArea.left, recArea.top, GRID_SPACE));
    assert(m_pWorld->IsInBounds(recArea.right - 1, recArea.bottom - 1, GRID_SPACE));

    SFoliageVertexEntry *pTileDest(static_cast<SFoliageVertexEntry *>(pDest));
    SFoliageVertexEntry *pTileSource(&m_pFoliageVertices[iLayer][m_pWorld->GetGridIndex(recArea.left, recArea.top)]);

    int iDestSpan = 0;
    int iSourceSpan = m_pWorld->GetGridWidth() - recArea.GetWidth();

    for (int y = 0; y < recArea.GetHeight(); ++y, pTileDest += iDestSpan, pTileSource += iSourceSpan)
    {
        for (int x = 0; x < recArea.GetWidth(); ++x, ++pTileDest, ++pTileSource)
        {
            *pTileDest = *pTileSource;
        }
    }

    return true;
}


/*====================
  CVertexFoliageMap::SetRegion
  ====================*/
bool    CVertexFoliageMap::SetRegion(const CRecti &recArea, void *pSource, int iLayer)
{
    assert(iLayer >= 0 && iLayer < NUM_FOLIAGE_LAYERS);
    assert(m_pWorld->IsInBounds(recArea.left, recArea.top, GRID_SPACE));
    assert(m_pWorld->IsInBounds(recArea.right - 1, recArea.bottom - 1, GRID_SPACE));

    SFoliageVertexEntry *pTileSource(static_cast<SFoliageVertexEntry *>(pSource));
    SFoliageVertexEntry *pTileDest(&m_pFoliageVertices[iLayer][m_pWorld->GetGridIndex(recArea.left, recArea.top)]);

    int iSourceSpan = 0;
    int iDestSpan = m_pWorld->GetGridWidth() - recArea.GetWidth();

    for (int y = 0; y < recArea.GetHeight(); ++y, pTileDest += iDestSpan, pTileSource += iSourceSpan)
    {
        for (int x = 0; x < recArea.GetWidth(); ++x, ++pTileDest, ++pTileSource)
        {
            *pTileDest = *pTileSource;
        }
    }

    m_bChanged = true;
    return true;
}


/*====================
  CVertexFoliageMap::GetFoliageDensity
  ====================*/
float   CVertexFoliageMap::GetFoliageDensity(int iX, int iY, int iLayer)
{
    assert(iLayer >= 0 && iLayer < NUM_FOLIAGE_LAYERS);
    assert(m_pFoliageVertices[iLayer] != nullptr);
    assert(m_pWorld->IsInBounds(iX, iY, GRID_SPACE));

    return m_pFoliageVertices[iLayer][m_pWorld->GetGridIndex(iX, iY)].fDensity;
}


/*====================
  CVertexFoliageMap::GetFoliageSize
  ====================*/
const CVec3f&   CVertexFoliageMap::GetFoliageSize(int iX, int iY, int iLayer)
{
    assert(iLayer >= 0 && iLayer < NUM_FOLIAGE_LAYERS);
    assert(m_pFoliageVertices[iLayer] != nullptr);
    assert(m_pWorld->IsInBounds(iX, iY, GRID_SPACE));

    return m_pFoliageVertices[iLayer][m_pWorld->GetGridIndex(iX, iY)].v3Size;
}


/*====================
  CVertexFoliageMap::GetFoliageVariance
  ====================*/
const CVec3f&   CVertexFoliageMap::GetFoliageVariance(int iX, int iY, int iLayer)
{
    assert(iLayer >= 0 && iLayer < NUM_FOLIAGE_LAYERS);
    assert(m_pFoliageVertices[iLayer] != nullptr);
    assert(m_pWorld->IsInBounds(iX, iY, GRID_SPACE));

    return m_pFoliageVertices[iLayer][m_pWorld->GetGridIndex(iX, iY)].v3Variance;
}


/*====================
  CVertexFoliageMap::GetFoliageColor
  ====================*/
const CVec3f&   CVertexFoliageMap::GetFoliageColor(int iX, int iY, int iLayer)
{
    assert(iLayer >= 0 && iLayer < NUM_FOLIAGE_LAYERS);
    assert(m_pFoliageVertices[iLayer] != nullptr);
    assert(m_pWorld->IsInBounds(iX, iY, GRID_SPACE));

    return m_pFoliageVertices[iLayer][m_pWorld->GetGridIndex(iX, iY)].v3Color;
}
