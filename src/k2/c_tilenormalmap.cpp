// (C)2005 S2 Games
// c_tilenormalmap.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_tilenormalmap.h"
#include "c_world.h"
//=============================================================================

/*====================
  CTileNormalMap::CTileNormalMap
  ====================*/
CTileNormalMap::CTileNormalMap(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("TileNormalMap"))
{
    m_pTileNormalMap[TRIANGLE_LEFT] = nullptr;
    m_pTileNormalMap[TRIANGLE_RIGHT] = nullptr;
}


/*====================
  CTileNormalMap::~CTileNormalMap
  ====================*/
CTileNormalMap::~CTileNormalMap()
{
    Release();
}


/*====================
  CTileNormalMap::Release
  ====================*/
void    CTileNormalMap::Release()
{
    m_pWorld = nullptr;

    if (m_pTileNormalMap[TRIANGLE_LEFT] != nullptr)
        K2_DELETE_ARRAY(m_pTileNormalMap[TRIANGLE_LEFT]);
    m_pTileNormalMap[TRIANGLE_LEFT] = nullptr;

    if (m_pTileNormalMap[TRIANGLE_RIGHT] != nullptr)
        K2_DELETE_ARRAY(m_pTileNormalMap[TRIANGLE_RIGHT]);
    m_pTileNormalMap[TRIANGLE_RIGHT] = nullptr;
}


/*====================
  CTileNormalMap::Load
  ====================*/
bool    CTileNormalMap::Load(CArchive &archive, const CWorld *pWorld)
{
    return Generate(pWorld);
}


/*====================
  CTileNormalMap::Generate
  ====================*/
bool    CTileNormalMap::Generate(const CWorld *pWorld)
{
    PROFILE("CTileNormalMap::Generate");

    try
    {
        Release();
        m_bChanged = false;
        m_pWorld = pWorld;
        if (m_pWorld == nullptr)
            EX_ERROR(_T("CTileNormalMap needs a valid CWorld"));

        m_pTileNormalMap[TRIANGLE_LEFT] = K2_NEW_ARRAY(ctx_World, CPlane, m_pWorld->GetTileArea());
        m_pTileNormalMap[TRIANGLE_RIGHT] = K2_NEW_ARRAY(ctx_World, CPlane, m_pWorld->GetTileArea());
        if (m_pTileNormalMap[TRIANGLE_LEFT] == nullptr || m_pTileNormalMap[TRIANGLE_RIGHT] == nullptr)
            EX_ERROR(_T("Failed to allocate memory for map data"));

        for (int y(0); y < m_pWorld->GetTileHeight(); ++y)
        {
            for (int x(0); x < m_pWorld->GetTileWidth(); ++x)
                CalculateTileNormals(x, y);
        }

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CTileNormalMap::Load() - "), NO_THROW);
        return false;
    }
}


/*====================
  CTileNormalMap::Update
  ====================*/
void    CTileNormalMap::Update(const CRecti &recArea)
{
    for (int y(recArea.top); y < recArea.bottom; ++y)
    {
        for (int x(recArea.left); x < recArea.right; ++x)
            CalculateTileNormals(x, y);
    }

    CRecti recNormals(recArea.left - 1, recArea.top - 1, recArea.right + 1, recArea.bottom + 1);

    if (m_pWorld->ClipRect(recNormals, GRID_SPACE))
    {
        m_pWorld->UpdateComponent(WORLD_VERT_NORMAL_MAP, recArea);
    }
}


/*====================
  CTileNormalMap::CalculateTileNormals
  ====================*/
void    CTileNormalMap::CalculateTileNormals(int iX, int iY)
{
    try
    {
        float h1(m_pWorld->GetGridPoint(iX, iY));
        float h2(m_pWorld->GetGridPoint(iX, iY + 1));
        float h3(m_pWorld->GetGridPoint(iX + 1, iY));
        float h4(m_pWorld->GetGridPoint(iX + 1, iY + 1));

        int index(m_pWorld->GetTileIndex(iX, iY));

        CVec3f a(iX * m_pWorld->GetScale(), iY * m_pWorld->GetScale(), h1);
        CVec3f b(iX * m_pWorld->GetScale(), (iY + 1) * m_pWorld->GetScale(), h2);
        CVec3f c((iX + 1) * m_pWorld->GetScale(), iY * m_pWorld->GetScale(), h3);
        CVec3f d((iX + 1) * m_pWorld->GetScale(), (iY + 1) * m_pWorld->GetScale(), h4);

        if (m_pWorld->GetTileSplit(iX, iY) == SPLIT_NEG)
        {
            m_pTileNormalMap[TRIANGLE_LEFT][index].CalcPlaneNormalized(c, b, a);
            m_pTileNormalMap[TRIANGLE_RIGHT][index].CalcPlaneNormalized(d, b, c);
        }
        else
        {
            m_pTileNormalMap[TRIANGLE_LEFT][index].CalcPlaneNormalized(a, d, b);
            m_pTileNormalMap[TRIANGLE_RIGHT][index].CalcPlaneNormalized(a, c, d);
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CTileNormalMap::CalculateTileNormals() - "), NO_THROW);
    }
}


/*====================
  CTileNormalMap::GetTileNormal
  ====================*/
const CVec3f&   CTileNormalMap::GetTileNormal(int iX, int iY, EGridTriangles tri) const
{
    assert(iX >= 0 && iX < m_pWorld->GetTileWidth());
    assert(iY >= 0 && iY < m_pWorld->GetTileHeight());
    assert(m_pTileNormalMap[tri] != nullptr);

    return m_pTileNormalMap[tri][m_pWorld->GetTileIndex(iX, iY)].v3Normal;
}


/*====================
  CTileNormalMap::GetTilePlane
  ====================*/
const CPlane&   CTileNormalMap::GetTilePlane(int iX, int iY, EGridTriangles tri) const
{
    assert(iX >= 0 && iX < m_pWorld->GetTileWidth());
    assert(iY >= 0 && iY < m_pWorld->GetTileHeight());
    assert(m_pTileNormalMap[tri] != nullptr);

    return m_pTileNormalMap[tri][m_pWorld->GetTileIndex(iX, iY)];
}
