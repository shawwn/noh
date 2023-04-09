// (C)2008 S2 Games
// c_navigationmap.cpp
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_world.h"
#include "c_navigationmap.h"
#include "c_searchnode.h"
#include "c_navgridUnits.h"
#include "intersection.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define RES_TO_FLAGS_SHIFT 29
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CVAR_FLOATF(g_navmap_terrain_minslope,  0.70f,  CVAR_GAMECONFIG);
//=============================================================================

/*====================
  CNavigationMap::CNavigationMap
  ====================*/
CNavigationMap::CNavigationMap(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("NavigationMap")),
m_uiMapChanges(0),
m_uiReady(0),
m_cBlockers(SHRT_MAX),
m_cGrids(ENAVGRID_MAX),
m_pBuildingGrid(NULL),
m_pTreeGrid(NULL),
m_pUnitGrid(NULL),
m_pAntiGrid(NULL)
{
    MemManager.Set(m_ahGrids, 0xff, sizeof(m_ahGrids));
};


/*====================
  CNavigationMap::FlagsByResolution
  ====================*/
inline uint CNavigationMap::FlagsByResolution(uint uiFlags, uint uiDownRes)
{
    return (uiFlags | (uiDownRes << RES_TO_FLAGS_SHIFT));
}


/*====================
  CNavigationMap::Init
  ====================*/
bool    CNavigationMap::Init(const CWorld* pWorld)
{
    Release();

    m_pWorld = pWorld;

    CNavGridZ::Alloc(m_pWorld->GetNavigationSize());
    m_fNavigationScale = m_pWorld->GetNavigationScale();
    m_fInverseTileScale = 1.0f / m_pWorld->GetNavigationScale();
    m_uiReady = 1;

    for (int x(0); x < NUM_GRID_RESOLUTIONS; ++x)
    {
        for (int i(0); i < ENAVGRID_COUNT; ++i)
        {
            m_ahGrids[x][i] = m_cGrids.New(CNavGridZ(m_pWorld->GetNavigationWidth() >> x, m_pWorld->GetNavigationHeight() >> x, x));

            CNavGridZ *pNavGrid(m_cGrids.GetReferenceByHandle(m_ahGrids[x][i]));
            pNavGrid->Init(1);
        }
    }

    CNavGridZ *pFullBuildingGrid(m_cGrids.GetReferenceByHandle(m_ahGrids[EGRIDRES_FULL][ENAVGRID_BUILDING]));
    m_pBuildingGrid = K2_NEW(ctx_Nav, CNavGridUnits)(m_pWorld->GetNavigationWidth(), m_pWorld->GetNavigationHeight());
    m_pBuildingGrid->LinkTo(pFullBuildingGrid);

    CNavGridZ *pFullTreeGrid(m_cGrids.GetReferenceByHandle(m_ahGrids[EGRIDRES_FULL][ENAVGRID_TREE]));
    m_pTreeGrid = K2_NEW(ctx_Nav, CNavGridUnits)(m_pWorld->GetNavigationWidth(), m_pWorld->GetNavigationHeight());
    m_pTreeGrid->LinkTo(pFullTreeGrid);

    CNavGridZ *pFullAntiGrid(m_cGrids.GetReferenceByHandle(m_ahGrids[EGRIDRES_FULL][ENAVGRID_ANTI]));
    m_pAntiGrid = K2_NEW(ctx_Nav, CNavGridUnits)(m_pWorld->GetNavigationWidth(), m_pWorld->GetNavigationHeight());
    m_pAntiGrid->LinkTo(pFullAntiGrid);

    CNavGridZ *pFullUnitGrid(m_cGrids.GetReferenceByHandle(m_ahGrids[EGRIDRES_FULL][ENAVGRID_UNIT]));
    m_pUnitGrid = K2_NEW(ctx_Nav, CNavGridUnits)(m_pWorld->GetNavigationWidth(), m_pWorld->GetNavigationHeight());
    m_pUnitGrid->LinkTo(pFullUnitGrid);

    m_uiGridFlags[ENAVGRID_SUMMARY] = NAVIGATION_TREE | NAVIGATION_CLIFF | NAVIGATION_BUILDING;
    m_uiGridFlags[ENAVGRID_BUILDING] = NAVIGATION_BUILDING;
    m_uiGridFlags[ENAVGRID_TREE] = NAVIGATION_TREE;
    m_uiGridFlags[ENAVGRID_CLIFF] = NAVIGATION_CLIFF;
    m_uiGridFlags[ENAVGRID_ANTI] = NAVIGATION_ANTI;
    m_uiGridFlags[ENAVGRID_UNIT] = 0;

    return true;
}


/*====================
  CNavigationMap::Release
  ====================*/
void    CNavigationMap::Release()
{
    m_cBlockers.Clear();
    m_cGrids.Clear();

    m_perFrameCustom.clear();
    m_mapCustom.clear();
    m_uiMapChanges = 0xffffffff;

    SAFE_DELETE(m_pBuildingGrid);
    SAFE_DELETE(m_pTreeGrid);
    SAFE_DELETE(m_pAntiGrid);
    SAFE_DELETE(m_pUnitGrid);

    MemManager.Set(m_ahGrids, 0xff, sizeof(m_ahGrids));

    CNavGridZ::ReleaseAlloc();

    m_uiReady = 0;
}


/*====================
  CNavigationMap::AnalyzeTerrain
  ====================*/
void    CNavigationMap::AnalyzeTerrain()
{
    const CWorld &cWorld(*m_pWorld);

#if 0
    const CTileNormalMap &cTileMap(cWorld.GetTileNormalMap());
#endif

    uint uiTileWidth(cWorld.GetTileWidth()), uiTileHeight(cWorld.GetTileHeight());
    uint uiTerrainBlockers(0);

    for (uint uiX(0); uiX < uiTileWidth; ++uiX)
    {
        for (uint uiY(0); uiY < uiTileHeight; ++uiY)
        {
#if 0
            float fLeftSlope(cTileMap.GetTileNormal(uiX, uiY, TRIANGLE_LEFT).z);
            float fRightSlope(cTileMap.GetTileNormal(uiX, uiY, TRIANGLE_RIGHT).z);
            float fMinSlope(MIN(fLeftSlope, fRightSlope));

            byte y1(m_pWorld->GetBlockers(uiX, uiY));
            byte y2(m_pWorld->GetBlockers(uiX, uiY + 1));
            byte y3(m_pWorld->GetBlockers(uiX + 1, uiY));
            byte y4(m_pWorld->GetBlockers(uiX + 1, uiY + 1));
            byte yBlockers((m_pWorld->GetTileSplit(uiX, uiY) == SPLIT_NEG) ? ((y1 & y2 & y3) | (y2 & y3 & y4)) : ((y1 & y2 & y4) | (y1 & y3 & y4)));

            if (fMinSlope < g_navmap_terrain_minslope || yBlockers)
            {
                AddBlocker2(NAVIGATION_CLIFF, cWorld.ScaleGridCoord(uiX), cWorld.ScaleGridCoord(uiY), cWorld.GetScale(), cWorld.GetScale());
                ++uiTerrainBlockers;
            }
#else
            byte y1(m_pWorld->GetBlockers(uiX, uiY));
            byte y2(m_pWorld->GetBlockers(uiX, uiY + 1));
            byte y3(m_pWorld->GetBlockers(uiX + 1, uiY));
            byte y4(m_pWorld->GetBlockers(uiX + 1, uiY + 1));
            byte yBlockers(y1 & y2 & y3 & y4);

            if (yBlockers)
            {
                AddBlocker2(NAVIGATION_CLIFF, cWorld.ScaleGridCoord(uiX), cWorld.ScaleGridCoord(uiY), cWorld.GetScale(), cWorld.GetScale());
                ++uiTerrainBlockers;
            }
#endif
        }
    }

    Console << uiTerrainBlockers << _T(" Terrain Blockers were added to the map navigation.") << newl;
}


/*====================
  CNavigationMap::AddBlocker
  ====================*/
PoolHandle  CNavigationMap::AddBlocker(uint uiFlags, float fPosX, float fPosY, float fWidth, float fHeight)
{
    if (!m_uiReady)
    {
        Console.Err << _T("Blocker added before world was initialized.") << newl;
        return INVALID_INDEX;
    }

    if (fWidth == 0.0f || fHeight == 0.0f)
    {
        Console.Err << _T("Invalid blocker dimensions.") << newl;
        return INVALID_INDEX;
    }

    int iNavigationWidth(m_pWorld->GetNavigationWidth()), iNavigationHeight(m_pWorld->GetNavigationHeight());
    SBlocker cBlocker;

    // Set ID/Flags
    cBlocker.uiFlags = uiFlags;

    // Calc Position in segments
    cBlocker.iBeginX = INT_FLOOR(fPosX * m_fInverseTileScale);
    cBlocker.iBeginY = INT_FLOOR(fPosY * m_fInverseTileScale);
    cBlocker.iEndX = INT_CEIL((fPosX + fWidth) * m_fInverseTileScale);
    cBlocker.iEndY = INT_CEIL((fPosY + fHeight) * m_fInverseTileScale);

    cBlocker.iBeginX = CLAMP(cBlocker.iBeginX, 0, iNavigationWidth - 1);
    cBlocker.iBeginY = CLAMP(cBlocker.iBeginY, 0, iNavigationHeight - 1);
    cBlocker.iEndX = CLAMP(cBlocker.iEndX, 0, iNavigationWidth);
    cBlocker.iEndY = CLAMP(cBlocker.iEndY, 0, iNavigationHeight);

    if (cBlocker.iEndX == 0 || cBlocker.iEndY == 0)
        return INVALID_INDEX;

    if (uiFlags & NAVIGATION_BUILDING)
    {
        m_pBuildingGrid->AddBlocker(cBlocker.iBeginX, cBlocker.iEndX, cBlocker.iBeginY, cBlocker.iEndY);
        m_uiMapChanges |= NAVIGATION_BUILDING;
    }

    if (uiFlags & NAVIGATION_TREE)
    {
        m_pTreeGrid->AddBlocker(cBlocker.iBeginX, cBlocker.iEndX, cBlocker.iBeginY, cBlocker.iEndY);
        m_uiMapChanges |= NAVIGATION_TREE;
    }

    if (uiFlags & NAVIGATION_CLIFF)
    {
        CNavGridZ *pCliffGrid(m_cGrids.GetReferenceByHandle(m_ahGrids[EGRIDRES_FULL][ENAVGRID_CLIFF]));

        pCliffGrid->AddBlocker(cBlocker.iBeginX, cBlocker.iEndX, cBlocker.iBeginY, cBlocker.iEndY);
        m_uiMapChanges |= NAVIGATION_CLIFF;
    }

    if (uiFlags & NAVIGATION_ANTI)
    {
        m_pAntiGrid->AddBlocker(cBlocker.iBeginX, cBlocker.iEndX, cBlocker.iBeginY, cBlocker.iEndY);
        m_uiMapChanges |= NAVIGATION_ANTI;
    }

    if (uiFlags & NAVIGATION_UNIT)
    {       
        m_pUnitGrid->AddBlocker(cBlocker.iBeginX, cBlocker.iEndX, cBlocker.iBeginY, cBlocker.iEndY);
        m_uiMapChanges |= NAVIGATION_UNIT;
    }

    return m_cBlockers.New(cBlocker);
}


/*====================
  CNavigationMap::AddBlocker2
  ====================*/
void    CNavigationMap::AddBlocker2(uint uiFlags, float fPosX, float fPosY, float fWidth, float fHeight)
{
    if (!m_uiReady)
    {
        Console.Err << _T("Blocker added before world was initialized.") << newl;
    }

    int iNavigationWidth(m_pWorld->GetNavigationWidth()), iNavigationHeight(m_pWorld->GetNavigationHeight());

    // Calc Position in segments
    int iBeginX(INT_FLOOR(fPosX * m_fInverseTileScale));
    int iBeginY(INT_FLOOR(fPosY * m_fInverseTileScale));
    int iEndX(INT_CEIL((fPosX + fWidth) * m_fInverseTileScale));
    int iEndY(INT_CEIL((fPosY + fHeight) * m_fInverseTileScale));

    iBeginX = CLAMP(iBeginX, 0, iNavigationWidth - 1);
    iBeginY = CLAMP(iBeginY, 0, iNavigationHeight - 1);
    iEndX = CLAMP(iEndX, 0, iNavigationWidth);
    iEndY = CLAMP(iEndY, 0, iNavigationHeight);

    if (iEndX == 0 || iEndY == 0)
        return;

    if (uiFlags & NAVIGATION_BUILDING)
    {
        m_pBuildingGrid->AddBlocker(iBeginX, iEndX, iBeginY, iEndY);
        m_uiMapChanges |= NAVIGATION_BUILDING;
    }

    if (uiFlags & NAVIGATION_TREE)
    {
        m_pTreeGrid->AddBlocker(iBeginX, iEndX, iBeginY, iEndY);
        m_uiMapChanges |= NAVIGATION_TREE;
    }

    if (uiFlags & NAVIGATION_CLIFF)
    {
        CNavGridZ *pCliffGrid(m_cGrids.GetReferenceByHandle(m_ahGrids[EGRIDRES_FULL][ENAVGRID_CLIFF]));

        pCliffGrid->AddBlocker(iBeginX, iEndX, iBeginY, iEndY);
        m_uiMapChanges |= NAVIGATION_CLIFF;
    }

    if (uiFlags & NAVIGATION_ANTI)
    {
        m_pAntiGrid->AddBlocker(iBeginX, iEndX, iBeginY, iEndY);
        m_uiMapChanges |= NAVIGATION_ANTI;
    }

    if (uiFlags & NAVIGATION_UNIT)
    {       
        m_pUnitGrid->AddBlocker(iBeginX, iEndX, iBeginY, iEndY);
        m_uiMapChanges |= NAVIGATION_UNIT;
    }
}


/*====================
  CNavigationMap::AddBlocker
  ====================*/
void    CNavigationMap::AddBlocker(vector<PoolHandle> &vecBlockers, uint uiFlags, const CConvexPolyhedron &cSurf, float fStepHeight)
{
    if (!m_uiReady)
    {
        Console.Err << _T("Blocker added before world was initialized.") << newl;
        return;
    }

    int iNavigationWidth(m_pWorld->GetNavigationWidth() - 1), iNavigationHeight(m_pWorld->GetNavigationHeight() - 1);
    uint uiStartSize(uint(vecBlockers.size()));

    // build coordinate range
    float fPosX(cSurf.GetBounds().GetMin().x);
    float fPosY(cSurf.GetBounds().GetMin().y);
    float fWidth(cSurf.GetBounds().GetDim(X));
    float fHeight(cSurf.GetBounds().GetDim(Y));

    int iMinX, iMinY, iWidth, iHeight;
    iMinX = INT_FLOOR(fPosX * m_fInverseTileScale);
    iMinY = INT_FLOOR(fPosY * m_fInverseTileScale);
    iWidth = INT_CEIL(fWidth * m_fInverseTileScale);
    iHeight = INT_CEIL(fHeight * m_fInverseTileScale);

    iMinX = CLAMP(iMinX, 0, iNavigationWidth);
    iMinY = CLAMP(iMinY, 0, iNavigationHeight);

    iWidth = MIN(iWidth, iNavigationWidth - iMinX);
    iHeight = MIN(iHeight, iNavigationHeight - iMinY);

    // find blocker spans based on the polyhedron
    float fHalfScale(m_fNavigationScale * 0.5f);
    for (int iHeightOffset(0); iHeightOffset <= iHeight; ++iHeightOffset)
    {
        float fX((iMinX + 0.5f) * m_fNavigationScale);
        float fY((iMinY + iHeightOffset + 0.5f) * m_fNavigationScale);
        
        int iStartOffset(0);
        for (; iStartOffset <= iWidth; ++iStartOffset, fX += m_fNavigationScale)
        {
            float fFraction(1.0f);
            float fZ(m_pWorld->GetTerrainHeight(fX, fY) + fStepHeight);
            CBBoxf bbBounds(CVec3f(fX - fHalfScale, fY - fHalfScale, fZ), CVec3f(fX + fHalfScale, fY + fHalfScale, fZ + 1.0f));
            
            if (I_MovingBoundsSurfaceIntersect(CVec3f(fX, fY, fZ), CVec3f(fX, fY, fZ + 100.0f), bbBounds, cSurf, fFraction))
                break;
        }

        fX += m_fNavigationScale;

        int iStopOffset(iStartOffset + 1);
        for (; iStopOffset <= iWidth; ++iStopOffset, fX += m_fNavigationScale)
        {
            float fFraction(1.0f);
            float fZ(m_pWorld->GetTerrainHeight(fX, fY) + fStepHeight);
            CBBoxf bbBounds(CVec3f(fX - fHalfScale, fY - fHalfScale, fZ), CVec3f(fX + fHalfScale, fY + fHalfScale, fZ + 1.0f));

            if (I_MovingBoundsSurfaceIntersect(CVec3f(fX, fY, fZ), CVec3f(fX, fY, fZ + 100.0f), bbBounds, cSurf, fFraction) == false)
                break;
        }

        if (iStartOffset <= iWidth)
        {
            // new blocker for the vector
            SBlocker cBlocker;

            cBlocker.iBeginX = iMinX + iStartOffset;
            cBlocker.iEndX = iMinX + iStopOffset;
            cBlocker.iBeginY = iMinY + iHeightOffset;
            cBlocker.iEndY = iMinY + iHeightOffset + 1;
            cBlocker.uiFlags = uiFlags;

            vecBlockers.push_back(m_cBlockers.New(cBlocker));
        }
    }

    // add all elements between uiStartSize and finalsize of the vector-1
    uint uiEndSize(uint(vecBlockers.size()));
    if (uiEndSize > uiStartSize)
    {
        if (uiFlags & NAVIGATION_BUILDING)
        {
            for (uint uiIndex(uiStartSize); uiIndex < uiEndSize; ++uiIndex)
            {
                SBlocker *pBlocker(m_cBlockers.GetReferenceByHandle(vecBlockers[uiIndex]));

                m_pBuildingGrid->AddBlocker(pBlocker->iBeginX, pBlocker->iEndX, pBlocker->iBeginY, pBlocker->iEndY);
            }

            m_uiMapChanges |= NAVIGATION_BUILDING;
        }

        if (uiFlags & NAVIGATION_TREE)
        {
            for (uint uiIndex(uiStartSize); uiIndex < uiEndSize; ++uiIndex)
            {
                SBlocker *pBlocker(m_cBlockers.GetReferenceByHandle(vecBlockers[uiIndex]));

                m_pTreeGrid->AddBlocker(pBlocker->iBeginX, pBlocker->iEndX, pBlocker->iBeginY, pBlocker->iEndY);
            }

            m_uiMapChanges |= NAVIGATION_TREE;
        }

        if (uiFlags & NAVIGATION_CLIFF)
        {
            CNavGridZ *pCliffGrid(m_cGrids.GetReferenceByHandle(m_ahGrids[EGRIDRES_FULL][ENAVGRID_CLIFF]));
            for (uint uiIndex(uiStartSize); uiIndex < uiEndSize; ++uiIndex)
            {
                SBlocker *pBlocker(m_cBlockers.GetReferenceByHandle(vecBlockers[uiIndex]));

                pCliffGrid->AddBlocker(pBlocker->iBeginX, pBlocker->iEndX, pBlocker->iBeginY, pBlocker->iEndY);
            }

            m_uiMapChanges |= NAVIGATION_CLIFF;
        }

        if (uiFlags & NAVIGATION_ANTI)
        {
            for (uint uiIndex(uiStartSize); uiIndex < uiEndSize; ++uiIndex)
            {
                SBlocker *pBlocker(m_cBlockers.GetReferenceByHandle(vecBlockers[uiIndex]));

                m_pAntiGrid->AddBlocker(pBlocker->iBeginX, pBlocker->iEndX, pBlocker->iBeginY, pBlocker->iEndY);
            }

            m_uiMapChanges |= NAVIGATION_ANTI;
        }

        if (uiFlags & NAVIGATION_UNIT)
        {
            for (uint uiIndex(uiStartSize); uiIndex < uiEndSize; ++uiIndex)
            {
                SBlocker *pBlocker(m_cBlockers.GetReferenceByHandle(vecBlockers[uiIndex]));

                m_pUnitGrid->AddBlocker(pBlocker->iBeginX, pBlocker->iEndX, pBlocker->iBeginY, pBlocker->iEndY);
            }

            m_uiMapChanges |= NAVIGATION_UNIT;
        }
    }
}


/*====================
  CNavigationMap::ClearBlocker
  
  Change the gridR blocker to have a bitval (0 in the case of clearing)
  ====================*/
void    CNavigationMap::ClearBlocker(PoolHandle hBlockerID)
{
    if (hBlockerID == INVALID_INDEX)
        return;

    SBlocker *pBlocker(m_cBlockers.GetReferenceByHandle(hBlockerID));

    if (pBlocker)
    {
        if (pBlocker->uiFlags & NAVIGATION_BUILDING)
        {
            m_pBuildingGrid->ClearBlocker(pBlocker->iBeginX, pBlocker->iEndX, pBlocker->iBeginY, pBlocker->iEndY);
            m_uiMapChanges |= NAVIGATION_BUILDING;
        }

        if (pBlocker->uiFlags & NAVIGATION_TREE)
        {
            m_pTreeGrid->ClearBlocker(pBlocker->iBeginX, pBlocker->iEndX, pBlocker->iBeginY, pBlocker->iEndY);
            m_uiMapChanges |= NAVIGATION_TREE;
        }

        if (pBlocker->uiFlags & NAVIGATION_CLIFF)
        {
            CNavGridZ *pCliffGrid(m_cGrids.GetReferenceByHandle(m_ahGrids[EGRIDRES_FULL][ENAVGRID_CLIFF]));

            pCliffGrid->ClearBlocker(pBlocker->iBeginX, pBlocker->iEndX, pBlocker->iBeginY, pBlocker->iEndY);
            m_uiMapChanges |= NAVIGATION_CLIFF;
        }

        if (pBlocker->uiFlags & NAVIGATION_ANTI)
        {
            m_pAntiGrid->ClearBlocker(pBlocker->iBeginX, pBlocker->iEndX, pBlocker->iBeginY, pBlocker->iEndY);
            m_uiMapChanges |= NAVIGATION_ANTI;
        }

        if (pBlocker->uiFlags & NAVIGATION_UNIT)
        {
            m_pUnitGrid->ClearBlocker(pBlocker->iBeginX, pBlocker->iEndX, pBlocker->iBeginY, pBlocker->iEndY);
            m_uiMapChanges |= NAVIGATION_UNIT;
        }
    }
    else
    {
        Console.Err << _T("Blocker removal failed - invalid handle") << newl;
    }
}


/*====================
  CNavigationMap::UpdateNavigation
  ====================*/
void    CNavigationMap::UpdateNavigation()
{
    PROFILE("CNavigationMap::UpdateNavigation");

    if (!m_uiReady)
        return;
    
    // Free any cached grids that included unit blocker information
    CustomGridMap::const_iterator cit(m_perFrameCustom.begin()), citEnd(m_perFrameCustom.end());
    for (; cit != citEnd; ++cit)
    {
        CNavGridZ *pGrid(m_cGrids.GetReferenceByHandle(cit->second));

        pGrid->Done();
        m_cGrids.Free(cit->second);
    }
    m_perFrameCustom.clear();

    // Update the half/quad unit blocker grids based on changes
    if (m_uiMapChanges & NAVIGATION_UNIT)
    {
        UpdateGrid(ENAVGRID_UNIT);
        m_uiMapChanges &= ~NAVIGATION_UNIT;
    }

    if (m_uiMapChanges)
    {
        {
            CustomGridMap::iterator it, itEnd;

            it = m_mapCustom.begin();
            itEnd = m_mapCustom.end();
            while (it != itEnd)
            {
                if (it->first & m_uiMapChanges)
                {
                    CNavGridZ *pGrid(m_cGrids.GetReferenceByHandle(it->second));

                    pGrid->Done();
                    m_cGrids.Free(it->second);
                    STL_ERASE(m_mapCustom, it);
                }
                else
                {
                    ++it;
                }
            }
        }

        if (m_uiMapChanges & NAVIGATION_BUILDING)
        {
            UpdateGrid(ENAVGRID_BUILDING);
            m_uiMapChanges &= ~NAVIGATION_BUILDING;
        }

        if (m_uiMapChanges & NAVIGATION_TREE)
        {
            UpdateGrid(ENAVGRID_TREE);
            m_uiMapChanges &= ~NAVIGATION_TREE;
        }

        if (m_uiMapChanges & NAVIGATION_CLIFF)
        {
            UpdateGrid(ENAVGRID_CLIFF);
            m_uiMapChanges &= ~NAVIGATION_TREE;
        }

        if (m_uiMapChanges & NAVIGATION_ANTI)
        {
            UpdateGrid(ENAVGRID_ANTI);
            m_uiMapChanges &= ~NAVIGATION_ANTI;
        }

        m_uiMapChanges = 0;
        BuildSummary();
        UpdateGrid(ENAVGRID_SUMMARY);
    }
}


/*====================
  CNavigationMap::GetGrid
  ====================*/
CNavGridZ *CNavigationMap::GetGrid(uint uiFlags, uint uiResolution)
{
    int i;

    if (uiFlags)
    {
        i = ENAVGRID_SUMMARY;
        for (i = 0; i < ENAVGRID_COUNT; ++i)
        {
            if (m_uiGridFlags[i] == uiFlags)
                break;
        }

        if (i == ENAVGRID_COUNT)
        {
            CustomGridMap::iterator itFind, itEnd(m_mapCustom.end());

            uiFlags = FlagsByResolution(uiFlags, uiResolution);
            itFind = m_mapCustom.find(uiFlags);

            if (itFind != itEnd)
            {
                return m_cGrids.GetReferenceByHandle(itFind->second);
            }
            else
            {
                uint uiStart, uiStop;
                uint uiMerged(0);

                uiStart = K2System.Microseconds();
                PoolHandle hNewGrid(m_cGrids.New(CNavGridZ(m_pWorld->GetNavigationWidth() >> uiResolution, m_pWorld->GetNavigationHeight() >> uiResolution, uiResolution)));
                CNavGridZ *pNewGrid(m_cGrids.GetReferenceByHandle(hNewGrid));
                pNewGrid->Init(0);

                // build custom grid
                PoolHandle *pGridHandles(&m_ahGrids[uiResolution][0]);
                while (--i > ENAVGRID_SUMMARY)
                {
                    if (uiFlags & m_uiGridFlags[i])
                    {
                        CNavGridZ *pCurrent(m_cGrids.GetReferenceByHandle(pGridHandles[i]));

                        if (uiMerged == 0)
                            pNewGrid->CopyFrom(*pCurrent);
                        else if (m_uiGridFlags[i] == NAVIGATION_ANTI)
                            pNewGrid->AntiMerge(*pCurrent);
                        else
                            pNewGrid->Merge(*pCurrent);

                        ++uiMerged;
                    }
                }

                m_mapCustom.insert(CustomGridPair(uiFlags, hNewGrid));
                uiStop = K2System.Microseconds();

                Console << _T("Custom navgrid created for flags: ") << uiFlags << _T(" in ") << uiStop - uiStart << _T("us") << newl;
                return pNewGrid;
            }
        }
        else
        {
            return m_cGrids.GetReferenceByHandle(m_ahGrids[uiResolution][i]);
        }
    }

    return NULL;
}


/*====================
  CNavigationMap::PrepForSearch
  ====================*/
CNavGridZ *CNavigationMap::PrepForSearch(uint uiFlags, uint uiResolution)
{
    PROFILE("CNavigationMap::PrepForSearch");

    if (m_uiReady == 0)
        return NULL;

    CNavGridZ *pRet;

    uiFlags &= ((1 << NAVIGATION_TYPES) - 1);
    if (uiFlags & NAVIGATION_UNIT)
    {
        uint uiBaseFlags(uiFlags & ~NAVIGATION_UNIT);
    
        uiFlags = FlagsByResolution(uiFlags, uiResolution);

        CustomGridMap::const_iterator cit(m_perFrameCustom.find(uiFlags)), citEnd(m_perFrameCustom.end());

        if (cit != citEnd)
        {
            pRet = m_cGrids.GetReferenceByHandle(cit->second);
        }
        else
        {
            //uint uiStart(K2System.Microseconds()), uiEnd;
            PoolHandle hNewGrid(m_cGrids.New(CNavGridZ(m_pWorld->GetNavigationWidth() >> uiResolution, m_pWorld->GetNavigationHeight() >> uiResolution, uiResolution)));

            pRet = m_cGrids.GetReferenceByHandle(hNewGrid);
            pRet->Init(0);

            CNavGridZ *pBase(GetGrid(uiBaseFlags, uiResolution));

            if (!pBase)
            {
                K2System.Error(_T("Error finding or allocating base grid"));
                return NULL;
            }

            CNavGridZ *pUnitGrid(m_cGrids.GetReferenceByHandle(m_ahGrids[uiResolution][ENAVGRID_UNIT]));
            pRet->CopyFrom(*pBase);
            pRet->Merge(*pUnitGrid);

            m_perFrameCustom.insert(CustomGridPair(uiFlags, hNewGrid));
            //uiEnd = K2System.Microseconds();
            //Console << _T("Merged unit-based grid for current frame searches: ") << uiEnd - uiStart << newl;
        }
    }
    else
    {
        pRet = GetGrid(uiFlags, uiResolution);
    }

    return pRet;
}
