// (C)2008 S2 Games
// c_vertexcameraheightmap.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_vertexcameraheightmap.h"

#include "c_world.h"
#include "c_buffer.h"
//=============================================================================

/*====================
  CVertexCameraHeightMap::CVertexCameraHeightMap
  ====================*/
CVertexCameraHeightMap::CVertexCameraHeightMap(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("VertexCameraHeightMap")),
m_pHeightMap(NULL)
{
}


/*====================
  CVertexCameraHeightMap::~CVertexCameraHeightMap
  ====================*/
CVertexCameraHeightMap::~CVertexCameraHeightMap()
{
    Release();
}


/*====================
  CVertexCameraHeightMap::Release
  ====================*/
void    CVertexCameraHeightMap::Release()
{
    m_pWorld = NULL;

    SAFE_DELETE_ARRAY(m_pHeightMap);
}


/*====================
  CVertexCameraHeightMap::Load
  ====================*/
bool    CVertexCameraHeightMap::Load(CArchive &archive, const CWorld *pWorld)
{
    return Generate(pWorld);
}

const int KERNEL_SIZE(25);
const float BLUR_WEIGHTS[KERNEL_SIZE] =
{
    0.001131f,
    0.002312f,
    0.004442f,
    0.008019f,
    0.013603f,
    0.021685f,
    0.032484f,
    0.045729f,
    0.060492f,
    0.075199f,
    0.087845f,
    0.096432f,
    0.099476f,
    0.096432f,
    0.087845f,
    0.075199f,
    0.060492f,
    0.045729f,
    0.032484f,
    0.021685f,
    0.013603f,
    0.008019f,
    0.004442f,
    0.002312f,
    0.001131f
};

/*====================
  CVertexCameraHeightMap::Generate
  ====================*/
bool    CVertexCameraHeightMap::Generate(const CWorld *pWorld)
{
    PROFILE("CVertexCameraHeightMap::Generate");

    try
    {
        Release();
        m_bChanged = true;
        m_pWorld = pWorld;
        if (m_pWorld == NULL)
            EX_ERROR(_T("CVertexCameraHeightMap needs a valid CWorld"));

        m_pHeightMap = K2_NEW_ARRAY(ctx_World, float, m_pWorld->GetGridArea());
        if (m_pHeightMap == NULL)
            EX_ERROR(_T("Failed to allocate memory for map data"));

        float *pHeightMap(m_pWorld->GetHeightMap());
        int iGridWidth(m_pWorld->GetGridWidth());
        int iGridHeight(m_pWorld->GetGridHeight());

        float *pHeightMapH(K2_NEW_ARRAY(ctx_World, float, m_pWorld->GetGridArea()));
        
        // 25-tap seperable gaussian filter
        float *p;

        // Blur horizontially from pHeightMap into pHeightMapH
        p = pHeightMapH;
        int iSrc(0);
        for (int iY(0); iY < iGridHeight; ++iY, iSrc += iGridWidth)
        {
            for (int iX(0); iX < iGridWidth; ++iX, ++p)
            {
                float fHeight(0.0f);
                
                for (int iSample(0); iSample < KERNEL_SIZE; ++iSample)
                {
                    int iSampleX(CLAMP(iX + iSample - KERNEL_SIZE / 2, 0, iGridWidth));

                    fHeight += pHeightMap[iSrc + iSampleX] * BLUR_WEIGHTS[iSample];
                }

                *p = fHeight;
            }
        }

        // Blur vertically from pHeightMapH into m_pHeightMap
        p = m_pHeightMap;
        for (int iY(0); iY < iGridHeight; ++iY)
        {
            for (int iX(0); iX < iGridWidth; ++iX, ++p)
            {
                float fHeight(0.0f);
                
                for (int iSample(0); iSample < KERNEL_SIZE; ++iSample)
                {
                    int iSampleY(CLAMP(iY + iSample - KERNEL_SIZE / 2, 0, iGridHeight));

                    fHeight += pHeightMapH[iSampleY * iGridWidth + iX] * BLUR_WEIGHTS[iSample];
                }

                *p = fHeight;
            }
        }

        K2_DELETE_ARRAY(pHeightMapH);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CVertexCameraHeightMap::Load() - "), NO_THROW);
        return false;
    }

    return true;
}


/*====================
  CVertexCameraHeightMap::Update
  ====================*/
void    CVertexCameraHeightMap::Update(const CRecti &recArea)
{
}


/*====================
  CVertexCameraHeightMap::GetRegion
  ====================*/
bool    CVertexCameraHeightMap::GetRegion(const CRecti &recArea, void *pDest, int iLayer) const
{
    assert(recArea.GetArea() > 0);
    assert(m_pWorld->IsInBounds(recArea.left, recArea.top, GRID_SPACE));
    assert(m_pWorld->IsInBounds(recArea.right - 1, recArea.bottom - 1, GRID_SPACE));

    float *pHeightDest(static_cast<float*>(pDest));
    float *pHeightSource = &m_pHeightMap[m_pWorld->GetGridIndex(recArea.left, recArea.top)];

    for (int y(0); y < recArea.GetHeight(); ++y, pHeightDest += recArea.GetWidth(), pHeightSource += m_pWorld->GetGridWidth())
        MemManager.Copy(pHeightDest, pHeightSource, sizeof(float) * recArea.GetWidth());

    return true;
}


/*====================
  CVertexCameraHeightMap::SetRegion
  ====================*/
bool    CVertexCameraHeightMap::SetRegion(const CRecti &recArea, void *pSource, int iLayer)
{
    assert(recArea.GetArea() > 0);
    assert(m_pWorld->IsInBounds(recArea.left, recArea.top, GRID_SPACE));
    assert(m_pWorld->IsInBounds(recArea.right - 1, recArea.bottom - 1, GRID_SPACE));

    float *pHeightSource(static_cast<float*>(pSource));
    float *pHeightDest(&m_pHeightMap[m_pWorld->GetGridIndex(recArea.left, recArea.top)]);

    for (int y(0); y < recArea.GetHeight(); ++y, pHeightSource += recArea.GetWidth(), pHeightDest += m_pWorld->GetGridWidth())
        MemManager.Copy(pHeightDest, pHeightSource, sizeof(float) * recArea.GetWidth());

    m_bChanged = true;
    return true;
}


/*====================
  CVertexCameraHeightMap::GetGridPoint
  ====================*/
float   CVertexCameraHeightMap::GetGridPoint(int iX, int iY)
{
    if (!m_pWorld->IsInBounds(iX, iY, GRID_SPACE))
    {
        iX = CLAMP(iX, 0, m_pWorld->GetGridWidth() - 1);
        iY = CLAMP(iY, 0, m_pWorld->GetGridHeight() - 1);
        Console.Warn << _T("CHeightMap::GetGridPoint() - Out of bound coordinate") << newl;
    }

    return m_pHeightMap[m_pWorld->GetGridIndex(iX, iY)];
}

float   CVertexCameraHeightMap::GetGridPoint(int iIndex)
{
    assert(iIndex >= 0 && iIndex < m_pWorld->GetGridArea());
    return m_pHeightMap[iIndex];
}
