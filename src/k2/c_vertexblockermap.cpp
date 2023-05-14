// (C)2007 S2 Games
// c_vertexblockermap.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_vertexblockermap.h"
#include "c_world.h"
#include "c_buffer.h"
//=============================================================================

/*====================
  CVertexBlockerMap::CVertexBlockerMap
  ====================*/
CVertexBlockerMap::CVertexBlockerMap(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("VertexBlockerMap")),
m_pVertexBlockers(nullptr)
{
}


/*====================
  CVertexBlockerMap::~CVertexBlockerMap
  ====================*/
CVertexBlockerMap::~CVertexBlockerMap()
{
    Release();
}


/*====================
  CVertexBlockerMap::Release
  ====================*/
void    CVertexBlockerMap::Release()
{
    m_pWorld = nullptr;
    SAFE_DELETE_ARRAY(m_pVertexBlockers);
}


/*====================
  CVertexBlockerMap::Load
  ====================*/
bool    CVertexBlockerMap::Load(CArchive &archive, const CWorld *pWorld)
{
    try
    {
        Release();
        m_pWorld = pWorld;

        CFileHandle file(m_sName, FILE_READ | FILE_BINARY, archive);
        if (!file.IsOpen())
            EX_ERROR(_T("No BlockerMap found in archive"));

        // Read the dimensions
        int iWidth = file.ReadInt32();
        int iHeight = file.ReadInt32();

        if (iWidth != m_pWorld->GetGridWidth() || iHeight != m_pWorld->GetGridHeight())
            EX_ERROR(_T("Invalid BlockerMap dimensions"));

        if (file.IsEOF())
            EX_ERROR(_T("Truncated BlockerMap"));

        // Read the BlockerMap
        int iSize(m_pWorld->GetGridArea() * sizeof(byte));
        m_pVertexBlockers = K2_NEW_ARRAY(ctx_World, byte, m_pWorld->GetGridArea());

        if (file.Read((char*)m_pVertexBlockers, iSize) < iSize)
            EX_ERROR(_T("Truncated BlockerMap"));

        m_bChanged = false;
    }
    catch (CException &ex)
    {
        SAFE_DELETE_ARRAY(m_pVertexBlockers);
        ex.Process(_T("CVertexBlockerMap::Load() - "), NO_THROW);
        return false;
    }

    return true;
}


/*====================
  CVertexBlockerMap::Generate
  ====================*/
bool    CVertexBlockerMap::Generate(const CWorld *pWorld)
{
    PROFILE("CVertexBlockerMap::Generate");

    try
    {
        Release();

        m_bChanged = true;
        m_pWorld = pWorld;
        if (m_pWorld == nullptr)
            EX_ERROR(_T("CVertexBlockerMap needs a valid CWorld"));

        m_pVertexBlockers = K2_NEW_ARRAY(ctx_World, byte, m_pWorld->GetGridArea());
        if (m_pVertexBlockers == nullptr)
            EX_ERROR(_T("Failed to allocate memory for BlockerMap"));

        for (int i(0); i < m_pWorld->GetGridArea(); ++i)
            m_pVertexBlockers[i] = 0;

        return true;
    }
    catch (CException &ex)
    {
        Release();
        ex.Process(_T("CVertexBlockerMap::Generate() - "));
        return false;
    }
}


/*====================
  CVertexBlockerMap::Serialize
  ====================*/
bool    CVertexBlockerMap::Serialize(IBuffer *pBuffer)
{
    pBuffer->Clear();
    (*pBuffer) << m_pWorld->GetGridWidth() << m_pWorld->GetGridHeight();
    pBuffer->Append(m_pVertexBlockers, m_pWorld->GetGridArea() * sizeof(byte));

    if (!pBuffer->GetFaults())
        return true;
    else
        return false;
}


/*====================
  CVertexBlockerMap::GetRegion
  ====================*/
bool    CVertexBlockerMap::GetRegion(const CRecti &recArea, void *pDest, int iLayer) const
{
    assert(recArea.IsNormalized());
    assert(m_pWorld->IsInBounds(recArea.left, recArea.top, GRID_SPACE));
    assert(m_pWorld->IsInBounds(recArea.right - 1, recArea.bottom - 1, GRID_SPACE));

    byte *pBlockerDest(static_cast<byte*>(pDest));
    byte *pBlockerSource = &m_pVertexBlockers[m_pWorld->GetGridIndex(recArea.left, recArea.top)];

    for (int y(0); y < recArea.GetHeight(); ++y, pBlockerDest += recArea.GetWidth(), pBlockerSource += m_pWorld->GetGridWidth())
        MemManager.Copy(pBlockerDest, pBlockerSource, sizeof(byte) * recArea.GetWidth());

    return true;
}


/*====================
  CVertexBlockerMap::SetRegion
  ====================*/
bool    CVertexBlockerMap::SetRegion(const CRecti &recArea, void *pSource, int iLayer)
{
    assert(m_pWorld->IsInBounds(recArea.left, recArea.top, GRID_SPACE));
    assert(m_pWorld->IsInBounds(recArea.right - 1, recArea.bottom - 1, GRID_SPACE));

    byte *pBlockerSource(static_cast<byte*>(pSource));
    byte *pBlockerDest(&m_pVertexBlockers[m_pWorld->GetGridIndex(recArea.left, recArea.top)]);

    for (int y(0); y < recArea.GetHeight(); ++y, pBlockerSource += recArea.GetWidth(), pBlockerDest += m_pWorld->GetGridWidth())
        MemManager.Copy(pBlockerDest, pBlockerSource, sizeof(byte) * recArea.GetWidth());

    CRecti recTiles(recArea.left - 1, recArea.top - 1, recArea.right, recArea.bottom);

    if (m_pWorld->ClipRect(recTiles, TILE_SPACE))
        m_pWorld->UpdateComponent(WORLD_TREE, recTiles);

    m_bChanged = true;
    return true;
}


/*====================
  CVertexBlockerMap::GetBlockers
  ====================*/
byte    CVertexBlockerMap::GetBlockers(int x, int y)
{
    assert(x >= 0 && x < m_pWorld->GetGridWidth());
    assert(y >= 0 && y < m_pWorld->GetGridHeight());
    return m_pVertexBlockers[m_pWorld->GetGridIndex(x, y)];
}


/*====================
  CVertexBlockerMap::SetBlockers
  ====================*/
void    CVertexBlockerMap::SetBlockers(int x, int y, byte yBlockers)
{
    assert(x >= 0 && x < m_pWorld->GetGridWidth());
    assert(y >= 0 && y < m_pWorld->GetGridHeight());
    m_pVertexBlockers[m_pWorld->GetGridIndex(x, y)] = yBlockers;
}


/*====================
  CVertexBlockerMap::CalcBlocked
  ====================*/
bool    CVertexBlockerMap::CalcBlocked(const CRecti &recArea) const
{
    byte *pSplitMap(m_pWorld->GetSplitMap());
    int iTileWidth(m_pWorld->GetTileWidth());

    for (int iY(recArea.top); iY <= recArea.bottom; ++iY)
    {
        for (int iX(recArea.left); iX <= recArea.right; ++iX)
        {
            byte yBlocker1(m_pVertexBlockers[m_pWorld->GetGridIndex(iX, iY)]);
            byte yBlocker2(m_pVertexBlockers[m_pWorld->GetGridIndex(iX, iY + 1)]);
            byte yBlocker3(m_pVertexBlockers[m_pWorld->GetGridIndex(iX + 1, iY)]);
            byte yBlocker4(m_pVertexBlockers[m_pWorld->GetGridIndex(iX + 1, iY + 1)]);

            if (pSplitMap[iY * iTileWidth + iX] == SPLIT_NEG)
            {
                if ((yBlocker1 & yBlocker2 & yBlocker3) || (yBlocker2 & yBlocker3 & yBlocker4))
                    return true;
            }
            else
            {
                if ((yBlocker1 & yBlocker2 & yBlocker4) || (yBlocker1 & yBlocker3 & yBlocker4))
                    return true;
            }
        }
    }

    return false;
}
