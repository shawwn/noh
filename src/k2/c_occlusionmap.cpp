// (C)2008 S2 Games
// c_occlusionmap.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_occlusionmap.h"

#include "c_world.h"
#include "c_worldentity.h"
#include "c_heightmap.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
//=============================================================================

/*====================
  COcclusionMap::COcclusionMap
  ====================*/
COcclusionMap::COcclusionMap(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("OcclusionMap")),
m_bInitialized(false),
m_pTerrain(NULL),
m_pCombined(NULL),
m_uiSize(2),
m_fScale(1.0f),
m_bRebuildOcclusion(true)
{
}


/*====================
  COcclusionMap::~COcclusionMap
  ====================*/
COcclusionMap::~COcclusionMap()
{
    Release();
}


/*====================
  COcclusionMap::Release
  ====================*/
void    COcclusionMap::Release()
{
    PROFILE("COcclusionMap::Release");

    SAFE_DELETE_ARRAY(m_pTerrain);
    SAFE_DELETE_ARRAY(m_pCombined);
}


/*====================
  COcclusionMap::Update
  ====================*/
void    COcclusionMap::Update(const CRecti &rect)
{
    try
    {
    }
    catch (CException &ex)
    {
        ex.Process(_T("COcclusionMap::Update() - "), NO_THROW);
    }
}


/*====================
  COcclusionMap::InitTerrainHeight
  ====================*/
void    COcclusionMap::InitTerrainHeight()
{
    try
    {
        uint uiTileWidth(m_pWorld->GetTileWidth() / m_uiSize);
        uint uiTileHeight(m_pWorld->GetTileHeight() / m_uiSize);

        for (uint uiMapY(0); uiMapY < uiTileHeight; ++uiMapY)
        {
            for (uint uiMapX(0); uiMapX < uiTileWidth; ++uiMapX)
            {
                float fTerrainHeightMax(-FAR_AWAY);

                int iBeginX(uiMapX * m_uiSize);
                int iBeginY(uiMapY * m_uiSize);
                int iEndX((uiMapX + 1) * m_uiSize);
                int iEndY((uiMapY + 1) * m_uiSize);

                for (int iY(iBeginY); iY < iEndY; ++iY)
                {
                    for (int iX(iBeginX); iX < iEndX; ++iX)
                    {
                        if (m_pWorld->GetVisBlocker(iX, iY))
                        {
                            fTerrainHeightMax = FAR_AWAY;
                        }
                        else
                        {
                            // The four grid points associated with this terrain tile
                            float p1(m_pWorld->GetGridPoint(iX, iY));
                            float p2(m_pWorld->GetGridPoint(iX, iY + 1));
                            float p3(m_pWorld->GetGridPoint(iX + 1, iY));
                            float p4(m_pWorld->GetGridPoint(iX + 1, iY + 1));

                            fTerrainHeightMax = MAX(fTerrainHeightMax, p1);
                            fTerrainHeightMax = MAX(fTerrainHeightMax, p2);
                            fTerrainHeightMax = MAX(fTerrainHeightMax, p3);
                            fTerrainHeightMax = MAX(fTerrainHeightMax, p4);
                        }
                    }
                }

                m_pTerrain[uiMapY * uiTileWidth + uiMapX] = fTerrainHeightMax;
            }
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("COcclusionMap::InitTerrainHeight() - "), NO_THROW);
    }
}


/*====================
  COcclusionMap::Generate
  ====================*/
bool    COcclusionMap::Generate(const CWorld *pWorld)
{
    PROFILE("COcclusionMap::Generate");

    try
    {
        Release();

        m_pWorld = pWorld;
        if (m_pWorld == NULL)
            EX_ERROR(_T("Invalid CWorld pointer"));

        m_uiSize = 1 << m_pWorld->GetVisibilitySize();

        m_uiTileWidth = m_pWorld->GetTileWidth() / m_uiSize;
        m_uiTileHeight = m_pWorld->GetTileHeight() / m_uiSize;
        m_fScale = m_pWorld->GetScale() * m_uiSize;

        m_pTerrain = K2_NEW_ARRAY(ctx_World, float, m_uiTileWidth*m_uiTileHeight);
        m_pCombined = K2_NEW_ARRAY(ctx_World, float, m_uiTileWidth*m_uiTileHeight);

        InitTerrainHeight();

        MemManager.Copy(m_pCombined, m_pTerrain, m_uiTileWidth * m_uiTileHeight * sizeof(float));

        m_bInitialized = true;
        return true;
    }
    catch (CException &ex)
    {
        m_bInitialized = false;
        ex.Process(_T("COcclusionMap::Generate() - "), NO_THROW);
        return true;
    }
}


/*====================
  COcclusionMap::OccludeRegion
  ====================*/
void    COcclusionMap::OccludeRegion(const CVec3f &v3Pos, float fRadius, float fHeight)
{
    int iBeginX(INT_FLOOR((v3Pos.x - fRadius) / m_fScale));
    int iBeginY(INT_FLOOR((v3Pos.y - fRadius) / m_fScale));
    int iEndX(INT_FLOOR((v3Pos.x + fRadius) / m_fScale) + 1);
    int iEndY(INT_FLOOR((v3Pos.y + fRadius) / m_fScale) + 1);

    iBeginX = CLAMP<int>(iBeginX, 0, m_uiTileWidth);
    iBeginY = CLAMP<int>(iBeginY, 0, m_uiTileHeight);
    iEndX = CLAMP<int>(iEndX, 0, m_uiTileWidth);
    iEndY = CLAMP<int>(iEndY, 0, m_uiTileHeight);

    float fTop(v3Pos.z + fHeight);

    for (int iY(iBeginY); iY != iEndY; ++iY)
        for (int iX(iBeginX); iX != iEndX; ++iX)
            m_pCombined[GetCellIndex(iX, iY)] = MAX(m_pCombined[GetCellIndex(iX, iY)], fTop);
}


/*====================
  COcclusionMap::AddOccludeRegion
  ====================*/
void    COcclusionMap::AddOccludeRegion(const CVec3f &v3Pos, float fRadius)
{
    m_bRebuildOcclusion = true;
}


/*====================
  COcclusionMap::RemoveOccludeRegion
  ====================*/
void    COcclusionMap::RemoveOccludeRegion(const CVec3f &v3Pos, float fRadius)
{
    m_bRebuildOcclusion = true;
}


/*====================
  COcclusionMap::GetRegion
  ====================*/
bool    COcclusionMap::GetRegion(const CRecti &recArea, byte *pDst, float fHeight)
{
    if (m_bRebuildOcclusion)
    {
        MemManager.Copy(m_pCombined, m_pTerrain, m_uiTileWidth * m_uiTileHeight * sizeof(float));

        WorldEntList &vEntities(m_pWorld->GetEntityList());
        for (WorldEntList_it it(vEntities.begin()), itEnd(vEntities.end()); it != itEnd; ++it)
        {
            CWorldEntity *pWorldEnt(m_pWorld->GetEntityByHandle(*it));
            if (pWorldEnt == NULL)
                continue;

            if (pWorldEnt->GetOcclusionRadius() > 0.0f && ~pWorldEnt->GetSurfFlags() & SURF_IGNORE)
                OccludeRegion(pWorldEnt->GetPosition(), pWorldEnt->GetOcclusionRadius(), 250.0f);
        }

        m_bRebuildOcclusion = false;
    }

    CRecti recClippedArea(recArea);
    recClippedArea.left = CLAMP<int>(recClippedArea.left, 0, m_uiTileWidth);
    recClippedArea.right = CLAMP<int>(recClippedArea.right, 0, m_uiTileWidth);
    recClippedArea.top = CLAMP<int>(recClippedArea.top, 0, m_uiTileHeight);
    recClippedArea.bottom = CLAMP<int>(recClippedArea.bottom, 0, m_uiTileHeight);

    float *pSrc(&m_pCombined[GetCellIndex(recClippedArea.left, recClippedArea.top)]);
    pDst += (recClippedArea.top - recArea.top) * recArea.GetWidth() + (recClippedArea.left - recArea.left);

    uint uiSrcSpan(m_uiTileWidth - recClippedArea.GetWidth());
    uint uiDstSpan(recArea.GetWidth() - recClippedArea.GetWidth());

    for (int iY(0); iY < recClippedArea.GetHeight(); ++iY, pSrc += uiSrcSpan, pDst += uiDstSpan)
    {
        for (int iX(0); iX < recClippedArea.GetWidth(); ++iX, ++pDst, ++pSrc)
        {
            *pDst = *pSrc > fHeight ? 255 : 0;
        }
    }

    return true;
}
