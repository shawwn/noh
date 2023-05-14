// (C)2008 S2 Games
// c_tilevisblockermap.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_tilevisionmap.h"

#include "c_world.h"
#include "c_buffer.h"
//=============================================================================

/*====================
  CTileVisBlockerMap::CTileVisBlockerMap
  ====================*/
CTileVisBlockerMap::CTileVisBlockerMap(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("TileVisBlockerMap")),
m_pTileVisBlockers(nullptr)
{
}


/*====================
  CTileVisBlockerMap::~CTileVisBlockerMap
  ====================*/
CTileVisBlockerMap::~CTileVisBlockerMap()
{
    Release();
}


/*====================
  CTileVisBlockerMap::Release
  ====================*/
void    CTileVisBlockerMap::Release()
{
    m_pWorld = nullptr;

    if (m_pTileVisBlockers != nullptr)
        K2_DELETE_ARRAY(m_pTileVisBlockers);
    m_pTileVisBlockers = nullptr;
}


/*====================
  CTileVisBlockerMap::Load
  ====================*/
bool    CTileVisBlockerMap::Load(CArchive &archive, const CWorld *pWorld)
{
    try
    {
        Release();
        m_pWorld = pWorld;

        CFileHandle file(m_sName, FILE_READ | FILE_BINARY, archive);
        if (!file.IsOpen())
            EX_ERROR(_T("No VisibilityMap found in archive"));

        // Read the dimensions
        int iWidth = file.ReadInt32();
        int iHeight = file.ReadInt32();

        if (iWidth != m_pWorld->GetTileWidth() || iHeight != m_pWorld->GetTileHeight())
            EX_ERROR(_T("Invalid VisibilityMap dimensions"));

        if (file.IsEOF())
            EX_ERROR(_T("Truncated VisibilityMap"));

        // Read the VisibilityMap
        int iSize(m_pWorld->GetTileArea() * sizeof(byte));
        m_pTileVisBlockers = K2_NEW_ARRAY(ctx_World, byte, m_pWorld->GetTileArea());

        if (file.Read((char*)m_pTileVisBlockers, iSize) < iSize)
            EX_ERROR(_T("Truncated VisibilityMap"));

        m_bChanged = false;
    }
    catch (CException &ex)
    {
        if (m_pTileVisBlockers != nullptr)
            K2_DELETE_ARRAY(m_pTileVisBlockers);
        m_pTileVisBlockers = nullptr;

        ex.Process(_T("CTileVisBlockerMap::Load() - "), NO_THROW);
        return false;
    }

    return true;
}


/*====================
  CTileVisBlockerMap::Generate
  ====================*/
bool    CTileVisBlockerMap::Generate(const CWorld *pWorld)
{
    PROFILE("CTileVisBlockerMap::Generate");

    try
    {
        Release();

        m_bChanged = true;
        m_pWorld = pWorld;
        if (m_pWorld == nullptr)
            EX_ERROR(_T("CTileVisBlockerMap needs a valid CWorld"));

        m_pTileVisBlockers = K2_NEW_ARRAY(ctx_World, byte, m_pWorld->GetTileArea());
        if (m_pTileVisBlockers == nullptr)
            EX_ERROR(_T("Failed to allocate memory for VisibilityMap"));

        for (int i(0); i < m_pWorld->GetTileArea(); ++i)
            m_pTileVisBlockers[i] = 0;

        return true;
    }
    catch (CException &ex)
    {
        Release();
        ex.Process(_T("CTileVisBlockerMap::Generate() - "));
        return false;
    }
}


/*====================
  CTileVisBlockerMap::Serialize
  ====================*/
bool    CTileVisBlockerMap::Serialize(IBuffer *pBuffer)
{
    pBuffer->Clear();
    (*pBuffer) << m_pWorld->GetTileWidth() << m_pWorld->GetTileHeight();
    pBuffer->Append(m_pTileVisBlockers, m_pWorld->GetTileArea() * sizeof(byte));

    if (!pBuffer->GetFaults())
        return true;
    else
        return false;
}


/*====================
  CTileVisBlockerMap::GetRegion
  ====================*/
bool    CTileVisBlockerMap::GetRegion(const CRecti &recArea, void *pDest, int iLayer) const
{
    assert(recArea.IsNormalized());
    assert(m_pWorld->IsInBounds(recArea.left, recArea.top, TILE_SPACE));
    assert(m_pWorld->IsInBounds(recArea.right - 1, recArea.bottom - 1, TILE_SPACE));

    byte *pVisBlockerDest(static_cast<byte*>(pDest));
    byte *pVisBlockerSource = &m_pTileVisBlockers[m_pWorld->GetTileIndex(recArea.left, recArea.top)];

    for (int y(0); y < recArea.GetHeight(); ++y, pVisBlockerDest += recArea.GetWidth(), pVisBlockerSource += m_pWorld->GetTileWidth())
        MemManager.Copy(pVisBlockerDest, pVisBlockerSource, sizeof(byte) * recArea.GetWidth());

    return true;
}


/*====================
  CTileVisBlockerMap::SetRegion
  ====================*/
bool    CTileVisBlockerMap::SetRegion(const CRecti &recArea, void *pSource, int iLayer)
{
    assert(m_pWorld->IsInBounds(recArea.left, recArea.top, TILE_SPACE));
    assert(m_pWorld->IsInBounds(recArea.right - 1, recArea.bottom - 1, TILE_SPACE));

    byte *pVisBlockerSource(static_cast<byte*>(pSource));
    byte *pVisBlockerDest(&m_pTileVisBlockers[m_pWorld->GetTileIndex(recArea.left, recArea.top)]);

    for (int y(0); y < recArea.GetHeight(); ++y, pVisBlockerSource += recArea.GetWidth(), pVisBlockerDest += m_pWorld->GetTileWidth())
        MemManager.Copy(pVisBlockerDest, pVisBlockerSource, sizeof(byte) * recArea.GetWidth());
    
    CRecti recNormals(recArea.left - 1, recArea.top - 1, recArea.right + 1, recArea.bottom + 1);

    if (m_pWorld->ClipRect(recNormals, GRID_SPACE))
        m_pWorld->UpdateComponent(WORLD_VERT_NORMAL_MAP, recArea);

    m_pWorld->UpdateComponent(WORLD_TREE, recArea);

    m_bChanged = true;
    return true;
}


/*====================
  CTileVisBlockerMap::GetVisBlocker
  ====================*/
byte    CTileVisBlockerMap::GetVisBlocker(int x, int y)
{
    assert(x >= 0 && x < m_pWorld->GetTileWidth());
    assert(y >= 0 && y < m_pWorld->GetTileHeight());
    return m_pTileVisBlockers[m_pWorld->GetTileIndex(x, y)];
}


/*====================
  CTileVisBlockerMap::SetVisBlocker
  ====================*/
void    CTileVisBlockerMap::SetVisBlocker(int x, int y, byte yVisBlocker)
{
    assert(x >= 0 && x < m_pWorld->GetTileWidth());
    assert(y >= 0 && y < m_pWorld->GetTileHeight());
    m_pTileVisBlockers[m_pWorld->GetTileIndex(x, y)] = yVisBlocker;
}
