// (C)2007 S2 Games
// c_tilesplitmap.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_tilesplitmap.h"
#include "c_world.h"
//=============================================================================

/*====================
  CTileSplitMap::CTileSplitMap
  ====================*/
CTileSplitMap::CTileSplitMap(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("TileSplitMap")),
m_pTileSplitMap(NULL)
{
}


/*====================
  CTileSplitMap::~CTileSplitMap
  ====================*/
CTileSplitMap::~CTileSplitMap()
{
    Release();
}


/*====================
  CTileSplitMap::Release
  ====================*/
void    CTileSplitMap::Release()
{
    m_pWorld = NULL;
    SAFE_DELETE_ARRAY(m_pTileSplitMap);
}


/*====================
  CTileSplitMap::Load
  ====================*/
bool    CTileSplitMap::Load(CArchive &archive, const CWorld *pWorld)
{
    return Generate(pWorld);
}


/*====================
  CTileSplitMap::Generate
  ====================*/
bool    CTileSplitMap::Generate(const CWorld *pWorld)
{
    PROFILE("CTileSplitMap::Generate");

    try
    {
        Release();
        m_bChanged = false;
        m_pWorld = pWorld;
        if (m_pWorld == NULL)
            EX_ERROR(_T("CTileSplitMap needs a valid CWorld"));

        m_pTileSplitMap = K2_NEW_ARRAY(ctx_World, byte, m_pWorld->GetTileArea());
        if (m_pTileSplitMap == NULL)
            EX_ERROR(_T("Failed to allocate memory for map data"));

        for (int y(0); y < m_pWorld->GetTileHeight(); ++y)
        {
            for (int x(0); x < m_pWorld->GetTileWidth(); ++x)
                CalculateTileSplit(x, y);
        }

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CTileSplitMap::Load() - "), NO_THROW);
        return false;
    }
}


/*====================
  CTileSplitMap::Update
  ====================*/
void    CTileSplitMap::Update(const CRecti &recArea)
{
    for (int y(recArea.top); y < recArea.bottom; ++y)
    {
        for (int x(recArea.left); x < recArea.right; ++x)
            CalculateTileSplit(x, y);
    }

    m_pWorld->UpdateComponent(WORLD_TILE_MATERIAL_MAP, recArea);
}


/*====================
  CTileSplitMap::CalculateTileSplit
  ====================*/
void    CTileSplitMap::CalculateTileSplit(int iX, int iY)
{
    try
    {
        float h1(m_pWorld->GetGridPoint(iX, iY));
        float h2(m_pWorld->GetGridPoint(iX, iY + 1));
        float h3(m_pWorld->GetGridPoint(iX + 1, iY));
        float h4(m_pWorld->GetGridPoint(iX + 1, iY + 1));

        if (fabs(h1 - h4) == fabs(h2 - h3))
            m_pTileSplitMap[m_pWorld->GetTileIndex(iX, iY)] = (iX % 2 + iY % 2) % 2;
        else if (fabs(h1 - h4) < fabs(h2 - h3))
            m_pTileSplitMap[m_pWorld->GetTileIndex(iX, iY)] = SPLIT_POS;
        else
            m_pTileSplitMap[m_pWorld->GetTileIndex(iX, iY)] = SPLIT_NEG;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CTileSplitMap::CalculateTileSplit() - "), NO_THROW);
    }
}


/*====================
  CTileSplitMap::GetTileSplit
  ====================*/
ETileSplitType  CTileSplitMap::GetTileSplit(int iX, int iY) const
{
    assert(iX >= 0 && iX < m_pWorld->GetTileWidth());
    assert(iY >= 0 && iY < m_pWorld->GetTileHeight());
    assert(m_pTileSplitMap != NULL);

    return ETileSplitType(m_pTileSplitMap[m_pWorld->GetTileIndex(iX, iY)]);
}
