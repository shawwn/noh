// (C)2005 S2 Games
// c_VertexCliffMap.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_vertexcliffmap.h"
#include "c_world.h"
#include "c_buffer.h"
//=============================================================================

/*====================
  CVertexCliffMap::CVertexCliffMap
  ====================*/
CVertexCliffMap::CVertexCliffMap(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("VertexCliffMap")),
m_pVertexCliff(nullptr)
{
}


/*====================
  CVertexCliffMap::~CVertexCliffMap
  ====================*/
CVertexCliffMap::~CVertexCliffMap()
{
    Release();
}


/*====================
  CVertexCliffMap::Release
  ====================*/
void    CVertexCliffMap::Release()
{
    m_pWorld = nullptr;

    if (m_pVertexCliff != nullptr)
        K2_DELETE_ARRAY(m_pVertexCliff);
    m_pVertexCliff = nullptr;
}


/*====================
  CVertexCliffMap::Load
  ====================*/
bool    CVertexCliffMap::Load(CArchive &archive, const CWorld *pWorld)
{
    try
    {
        Release();
        m_pWorld = pWorld;

        CFileHandle file(m_sName, FILE_READ | FILE_BINARY, archive);
        if (!file.IsOpen())
            EX_ERROR(_T("No CliffVertexMap found in archive"));

        // Read the dimensions
        int iWidth = file.ReadInt32();
        int iHeight = file.ReadInt32();

        if (iWidth != m_pWorld->GetCliffGridWidth() || iHeight != m_pWorld->GetCliffGridHeight())
            EX_ERROR(_T("Invalid CliffVertexMap dimensions"));

        m_iCliffMapWidth = iWidth;

        if (file.IsEOF())
            EX_ERROR(_T("Truncated CliffVertexMap"));

        // Read the CliffMap
        int iSize(m_pWorld->GetCliffGridArea() * sizeof(int));
        m_pVertexCliff = K2_NEW_ARRAY(ctx_World, int, m_pWorld->GetCliffGridArea());

        if (file.Read((char*)m_pVertexCliff, iSize) < iSize)
            EX_ERROR(_T("Truncated CliffVertexMap"));

        m_bChanged = false;
    }
    catch (CException &ex)
    {
        if (m_pVertexCliff != nullptr)
            K2_DELETE_ARRAY(m_pVertexCliff);
        m_pVertexCliff = nullptr;

        ex.Process(_T("CVertexCliff::Load() - "), NO_THROW);
        return false;
    }

    return true;
}

/*====================
  CVertexCliffMap::Serialize
  ====================*/
bool    CVertexCliffMap::Serialize(IBuffer *pBuffer)
{
    pBuffer->Clear();
    (*pBuffer) << m_pWorld->GetCliffGridWidth() << m_pWorld->GetCliffGridHeight();
    pBuffer->Append(m_pVertexCliff, m_pWorld->GetCliffGridArea() * sizeof(int));


    /*
    pBuffer->Clear();
    // UTTAR: Negative width shows that this is our new file format.
    // This file format uses 3 BYTES rather than floats to optimize download size.
    (*pBuffer) << m_pWorld->GetCliffGridWidth() << m_pWorld->GetCliffGridHeight();

    float *pHeightMapCopy(K2_NEW(ctx_World) float[m_pWorld->GetGridArea()]);

    for (int iTile(0); iTile < m_pWorld->GetGridArea(); ++iTile)
    {
        pHeightMapCopy[iTile] = m_pHeightMap[iTile] + 32768.0f;
        (*pBuffer) << ((byte)((int)((pHeightMapCopy[iTile]*256.0f)+0.5f)%256));
    }
    for (int iTile(0); iTile < m_pWorld->GetGridArea(); ++iTile)
        (*pBuffer) << ((byte)((int)(pHeightMapCopy[iTile])%256));
    for (int iTile(0); iTile < m_pWorld->GetGridArea(); ++iTile)
        (*pBuffer) << ((byte)((int)(pHeightMapCopy[iTile]/256.0f)%256));

    delete[] pHeightMapCopy;
    */

    if (pBuffer->GetFaults())
        return false;

    return true;
}


/*====================
  CVertexCliffMap::Generate
  ====================*/
bool    CVertexCliffMap::Generate(const CWorld *pWorld)
{
    PROFILE("CVertexCliffMap::Generate");

    try
    {
        Release();
        m_bChanged = true;
        m_pWorld = pWorld;
        if (m_pWorld == nullptr)
            EX_ERROR(_T("CVertexCliffMap needs a valid CWorld"));

        int iWorldSize = pow((float)2, (float)m_pWorld->GetSize());
        m_iCliffMapWidth = iWorldSize / pWorld->GetCliffSize() + 1;
        m_iCliffMapHeight = iWorldSize / pWorld->GetCliffSize() + 1;
        
        m_pVertexCliff = K2_NEW_ARRAY(ctx_World, int, m_iCliffMapWidth*m_iCliffMapHeight);

        if (m_pVertexCliff == nullptr)
            EX_ERROR(_T("Failed to allocate memory for map data"));

        MemManager.Set(m_pVertexCliff, 0, sizeof(int) * (m_iCliffMapWidth * m_iCliffMapWidth));
    }
    catch (CException &ex)
    {
        ex.Process(_T("CVertexCliffMap::Load() - "), NO_THROW);
        return false;
    }

    return true;
}


/*====================
  CVertexCliffMap::Update
  ====================*/
void    CVertexCliffMap::Update(const CRecti &recArea)
{
    //for (int y(recArea.top); y < recArea.bottom; ++y)
    //{
    //  for (int x(recArea.left); x < recArea.right; ++x)
    //      CalculateVertexNormal(x, y);
    //}
    //
    //CRecti recNormals(recArea.left - 1, recArea.top - 1, recArea.right + 1, recArea.bottom + 1);
    //
    //if (m_pWorld->ClipRect(recNormals, GRID_SPACE))
    //{
    //  m_pWorld->UpdateComponent(WORLD_VERT_TANGENT_MAP, recArea);
    //}
}


/*====================
  CVertexCliffMap::GetVertexCliff
  ====================*/
const uint  CVertexCliffMap::GetVertexCliff(int x, int y)
{
    assert(x >= 0 && x <= m_iCliffMapWidth);
    assert(y >= 0 && y <= m_iCliffMapWidth);
    return (y * m_iCliffMapWidth) + x;
}


/*====================
  CVertexCliffMap::GetRegion
  ====================*/
bool    CVertexCliffMap::GetRegion(const CRecti &recArea, void *pDest, int iLayer) const
{
    assert(recArea.right <= m_pWorld->GetCliffGridWidth() + 1);
    assert(recArea.left >= 0);
    assert(recArea.bottom <= m_pWorld->GetCliffTileHeight() + 1);
    assert(recArea.top >= 0);
    assert(recArea.GetArea() > 0);

    int *pCliffDest(static_cast<int*>(pDest));
    int *pCliffSource = &m_pVertexCliff[m_pWorld->GetVertCliff(recArea.left, recArea.top)];

    for (int y(0); y < recArea.GetHeight(); ++y, pCliffSource += m_iCliffMapWidth, pCliffDest += recArea.GetWidth())
        MemManager.Copy(pCliffDest, pCliffSource, sizeof(int) * recArea.GetWidth());

    return true;
}


/*====================
  CVertexCliffMap::SetRegion
  ====================*/
bool    CVertexCliffMap::SetRegion(const CRecti &recArea, void *pSource, int iLayer)
{
    assert(recArea.right <= m_pWorld->GetCliffGridWidth() + 1);
    assert(recArea.left >= 0);
    assert(recArea.bottom <= m_pWorld->GetCliffTileHeight() + 1);
    assert(recArea.top >= 0);
    assert(recArea.GetHeight() > 0);
    assert(recArea.GetWidth() > 0);
    assert(recArea.GetArea() > 0);

    int *pCliffSource(static_cast<int*>(pSource));
    int *pCliffDest(&m_pVertexCliff[m_pWorld->GetVertCliff(recArea.left, recArea.top)]);

    for (int y(0); y < recArea.GetHeight(); ++y, pCliffSource += recArea.GetWidth(), pCliffDest += m_iCliffMapWidth)
        MemManager.Copy(pCliffDest, pCliffSource, sizeof(int) * recArea.GetWidth());

    return true;
}
