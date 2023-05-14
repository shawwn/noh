// (C)2005 S2 Games
// c_vertextangentmap.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_vertextangentmap.h"
#include "c_world.h"
#include "c_buffer.h"
//=============================================================================

/*====================
  CVertexTangentMap::CVertexTangentMap
  ====================*/
CVertexTangentMap::CVertexTangentMap(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("VertexTangentMap")),
m_pVertexTangents(nullptr)
{
}


/*====================
  CVertexTangentMap::~CVertexTangentMap
  ====================*/
CVertexTangentMap::~CVertexTangentMap()
{
    Release();
}


/*====================
  CVertexTangentMap::Release
  ====================*/
void    CVertexTangentMap::Release()
{
    m_pWorld = nullptr;

    if (m_pVertexTangents != nullptr)
        K2_DELETE_ARRAY(m_pVertexTangents);
    m_pVertexTangents = nullptr;
}


/*====================
  CVertexTangentMap::Load
  ====================*/
bool    CVertexTangentMap::Load(CArchive &archive, const CWorld *pWorld)
{
    return Generate(pWorld);
}


/*====================
  CVertexTangentMap::Generate
  ====================*/
bool    CVertexTangentMap::Generate(const CWorld *pWorld)
{
    PROFILE("CVertexTangentMap::Generate");

    try
    {
        Release();
        m_bChanged = true;
        m_pWorld = pWorld;
        if (m_pWorld == nullptr)
            EX_ERROR(_T("CVertexTangentMap needs a valid CWorld"));

        m_pVertexTangents = K2_NEW_ARRAY(ctx_World, CVec3f, m_pWorld->GetGridArea());
        if (m_pVertexTangents == nullptr)
            EX_ERROR(_T("Failed to allocate memory for map data"));

        for (int iY(0); iY < m_pWorld->GetGridHeight(); ++iY)
        {
            for (int iX(0); iX < m_pWorld->GetGridWidth(); ++iX)
                CalculateVertexTangent(iX, iY);
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CVertexTangentMap::Load() - "), NO_THROW);
        return false;
    }

    return true;
}


/*====================
  CVertexTangentMap::Update
  ====================*/
void    CVertexTangentMap::Update(const CRecti &recArea)
{
    for (int y(recArea.top); y < recArea.bottom; ++y)
    {
        for (int x(recArea.left); x < recArea.right; ++x)
            CalculateVertexTangent(x, y);
    }
}


/*====================
  CVertexTangentMap::GetRegion
  ====================*/
bool    CVertexTangentMap::GetRegion(int iStartX, int iStartY, int iWidth, int iHeight, void *pDest, int iDestSize, int iLayer) const
{
    CVec3f *pNormalDest = reinterpret_cast<CVec3f*>(pDest);
    CVec3f *pNormalSource = &m_pVertexTangents[m_pWorld->GetGridIndex(iStartX, iStartY)];

    assert(iWidth >= 0);
    assert(iHeight >= 0);

    assert(iStartX >= 0 && iStartX < m_pWorld->GetGridWidth());
    assert(iStartY >= 0 && iStartY < m_pWorld->GetGridHeight());
    assert(iStartX + iWidth <= m_pWorld->GetGridWidth());
    assert(iStartY + iHeight <= m_pWorld->GetGridHeight());

    for (int y = 0; y < iHeight; ++y, pNormalDest += iDestSize, pNormalSource += m_pWorld->GetGridWidth())
        MemManager.Copy(pNormalDest, pNormalSource, sizeof(CVec3f) * iWidth);

    return true;
}


/*====================
  CVertexTangentMap::SetRegion
  ====================*/
bool    CVertexTangentMap::SetRegion(int iStartX, int iStartY, int iWidth, int iHeight, void *pSource, int iSourceSize, int iLayer)
{
    CVec3f *pNormalSource = reinterpret_cast<CVec3f*>(pSource);
    CVec3f *pNormalDest = &m_pVertexTangents[m_pWorld->GetGridIndex(iStartX, iStartY)];

    assert(iWidth >= 0);
    assert(iHeight >= 0);

    assert(iStartX >= 0 && iStartX < m_pWorld->GetGridWidth());
    assert(iStartY >= 0 && iStartY < m_pWorld->GetGridHeight());
    assert(iStartX + iWidth <= m_pWorld->GetGridWidth());
    assert(iStartY + iHeight <= m_pWorld->GetGridHeight());

    for (int y = 0; y < iHeight; ++y, pNormalSource += iSourceSize, pNormalDest += m_pWorld->GetGridWidth())
        MemManager.Copy(pNormalDest, pNormalSource, sizeof(CVec3f) * iWidth);

    m_bChanged = true;
    return true;
}


/*====================
  CVertexTangentMap::GetVertexTangent
  ====================*/
const CVec3f&   CVertexTangentMap::GetVertexTangent(int x, int y)
{
    assert(x >= 0 && x < m_pWorld->GetGridWidth());
    assert(y >= 0 && y < m_pWorld->GetGridHeight());
    return m_pVertexTangents[m_pWorld->GetGridIndex(x, y)];
}


/*====================
  CVertexTangentMap::GetFaceTangent
  ====================*/
CVec3f  CVertexTangentMap::GetFaceTangent(const CVec3f &v0, const CVec3f &v1, const CVec3f &v2, const CVec2f &t0, const CVec2f t1, const CVec2f t2)
{
    CVec3f v3A(CrossProduct(CVec3f(v0.x, t0.x, t0.y) - CVec3f(v1.x, t1.x, t1.y), CVec3f(v0.x, t0.x, t0.y) - CVec3f(v2.x, t2.x, t2.y)));
    CVec3f v3B(CrossProduct(CVec3f(v0.y, t0.x, t0.y) - CVec3f(v1.y, t1.x, t1.y), CVec3f(v0.y, t0.x, t0.y) - CVec3f(v2.y, t2.x, t2.y)));
    CVec3f v3C(CrossProduct(CVec3f(v0.z, t0.x, t0.y) - CVec3f(v1.z, t1.x, t1.y), CVec3f(v0.z, t0.x, t0.y) - CVec3f(v2.z, t2.x, t2.y)));

    // Return the tangent
    return Normalize(CVec3f(v3A.x == 0.0f ? FAR_AWAY : -v3A.y / v3A.x, v3B.x == 0.0f ? FAR_AWAY : -v3B.y / v3B.x, v3C.x == 0.0f ? FAR_AWAY : -v3C.y / v3C.x));
}


/*====================
  CVertexTangentMap::CalculateVertexTangent
  ====================*/
void    CVertexTangentMap::CalculateVertexTangent(int iX, int iY)
{
    assert(iX >= 0);
    assert(iX < m_pWorld->GetGridWidth());
    assert(iY >= 0);
    assert(iY < m_pWorld->GetGridHeight());
    assert(m_pWorld != nullptr);

    CVec3f &v3Tangent(m_pVertexTangents[m_pWorld->GetGridIndex(iX, iY)]);
    v3Tangent.Clear();

    if (iX > 0 && iY > 0)
    {
        if (m_pWorld->GetTileSplit(iX - 1, iY - 1) == SPLIT_NEG)
        {
            // Triangle (iX, iY) (iX - 1, iY) (iX, iY - 1)
            CVec3f v0(iX * m_pWorld->GetScale(), iY * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX, iY));
            CVec3f v1((iX - 1) * m_pWorld->GetScale(), iY * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX - 1, iY));
            CVec3f v2(iX * m_pWorld->GetScale(), (iY - 1) * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX, iY - 1));

            CVec2f t0(static_cast<float>(iX) / m_pWorld->GetTextureScale(), static_cast<float>(iY) / m_pWorld->GetTextureScale());
            CVec2f t1(static_cast<float>(iX - 1) / m_pWorld->GetTextureScale(), static_cast<float>(iY) / m_pWorld->GetTextureScale());
            CVec2f t2(static_cast<float>(iX) / m_pWorld->GetTextureScale(), static_cast<float>(iY - 1) / m_pWorld->GetTextureScale());

            v3Tangent += GetFaceTangent(v0, v1, v2, t0, t1, t2);
        }
        else
        {
            
            // Triangle (iX, iY) (iX - 1, iY - 1) (iX - 1, iY)
            {
                CVec3f v0(iX * m_pWorld->GetScale(), iY * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX, iY));
                CVec3f v1((iX - 1) * m_pWorld->GetScale(), (iY - 1) * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX - 1, iY - 1));
                CVec3f v2((iX - 1) * m_pWorld->GetScale(), iY * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX - 1, iY));

                CVec2f t0(static_cast<float>(iX) / m_pWorld->GetTextureScale(), static_cast<float>(iY) / m_pWorld->GetTextureScale());
                CVec2f t1(static_cast<float>(iX - 1) / m_pWorld->GetTextureScale(), static_cast<float>(iY - 1) / m_pWorld->GetTextureScale());
                CVec2f t2(static_cast<float>(iX - 1) / m_pWorld->GetTextureScale(), static_cast<float>(iY) / m_pWorld->GetTextureScale());

                v3Tangent += GetFaceTangent(v0, v1, v2, t0, t1, t2) * 0.5f;
            }

            // Triangle (iX, iY) (iX, iY - 1) (iX - 1, iY - 1)
            {
                CVec3f v0(iX * m_pWorld->GetScale(), iY * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX, iY));
                CVec3f v1(iX * m_pWorld->GetScale(), (iY - 1) * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX, iY - 1));
                CVec3f v2((iX - 1) * m_pWorld->GetScale(), (iY - 1) * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX - 1, iY - 1));

                CVec2f t0(static_cast<float>(iX) / m_pWorld->GetTextureScale(), static_cast<float>(iY) / m_pWorld->GetTextureScale());
                CVec2f t1(static_cast<float>(iX) / m_pWorld->GetTextureScale(), static_cast<float>(iY - 1) / m_pWorld->GetTextureScale());
                CVec2f t2(static_cast<float>(iX - 1) / m_pWorld->GetTextureScale(), static_cast<float>(iY - 1) / m_pWorld->GetTextureScale());

                v3Tangent += GetFaceTangent(v0, v1, v2, t0, t1, t2) * 0.5f;
            }
        }
    }

    if (iX < m_pWorld->GetGridWidth() - 1 && iY < m_pWorld->GetGridHeight() - 1)
    {
        if (m_pWorld->GetTileSplit(iX, iY) == SPLIT_NEG)
        {
            // Triangle [(iX, iY) (iX + 1, iY) (iX, iY + 1)]
            CVec3f v0(iX * m_pWorld->GetScale(), iY * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX, iY));
            CVec3f v1((iX + 1) * m_pWorld->GetScale(), iY * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX + 1, iY));
            CVec3f v2(iX * m_pWorld->GetScale(), (iY + 1) * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX, iY + 1));

            CVec2f t0(static_cast<float>(iX) / m_pWorld->GetTextureScale(), static_cast<float>(iY) / m_pWorld->GetTextureScale());
            CVec2f t1(static_cast<float>(iX + 1) / m_pWorld->GetTextureScale(), static_cast<float>(iY) / m_pWorld->GetTextureScale());
            CVec2f t2(static_cast<float>(iX) / m_pWorld->GetTextureScale(), static_cast<float>(iY + 1) / m_pWorld->GetTextureScale());

            v3Tangent += GetFaceTangent(v0, v1, v2, t0, t1, t2);
        }
        else
        {
            // Triangle (iX, iY) (iX + 1, iY + 1) (iX, iY + 1)
            {
                CVec3f v0(iX * m_pWorld->GetScale(), iY * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX, iY));
                CVec3f v1((iX + 1) * m_pWorld->GetScale(), (iY + 1) * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX + 1, iY + 1));
                CVec3f v2(iX * m_pWorld->GetScale(), (iY + 1) * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX, iY + 1));

                CVec2f t0(static_cast<float>(iX) / m_pWorld->GetTextureScale(), static_cast<float>(iY) / m_pWorld->GetTextureScale());
                CVec2f t1(static_cast<float>(iX + 1) / m_pWorld->GetTextureScale(), static_cast<float>(iY + 1) / m_pWorld->GetTextureScale());
                CVec2f t2(static_cast<float>(iX) / m_pWorld->GetTextureScale(), static_cast<float>(iY + 1) / m_pWorld->GetTextureScale());

                v3Tangent += GetFaceTangent(v0, v1, v2, t0, t1, t2) * 0.5f;
            }

            // Triangle (iX, iY) (iX + 1, iY) (iX + 1, iY + 1)
            {
                CVec3f v0(iX * m_pWorld->GetScale(), iY * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX, iY));
                CVec3f v1((iX + 1) * m_pWorld->GetScale(), iY * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX + 1, iY));
                CVec3f v2((iX + 1) * m_pWorld->GetScale(), (iY + 1) * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX + 1, iY + 1));

                CVec2f t0(static_cast<float>(iX) / m_pWorld->GetTextureScale(), static_cast<float>(iY) / m_pWorld->GetTextureScale());
                CVec2f t1(static_cast<float>(iX + 1) / m_pWorld->GetTextureScale(), static_cast<float>(iY) / m_pWorld->GetTextureScale());
                CVec2f t2(static_cast<float>(iX + 1) / m_pWorld->GetTextureScale(), static_cast<float>(iY + 1) / m_pWorld->GetTextureScale());

                v3Tangent += GetFaceTangent(v0, v1, v2, t0, t1, t2) * 0.5f;
            }
        }
    }

    if (iX < m_pWorld->GetGridWidth() - 1 && iY > 0)
    {
        if (m_pWorld->GetTileSplit(iX, iY - 1) == SPLIT_NEG)
        {
            // Triangle [(iX, iY) (iX, iY - 1) (iX + 1, iY - 1)]
            {
                CVec3f v0(iX * m_pWorld->GetScale(), iY * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX, iY));
                CVec3f v1(iX * m_pWorld->GetScale(), (iY - 1) * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX, iY - 1));
                CVec3f v2((iX + 1) * m_pWorld->GetScale(), (iY - 1) * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX + 1, iY - 1));

                CVec2f t0(static_cast<float>(iX) / m_pWorld->GetTextureScale(), static_cast<float>(iY) / m_pWorld->GetTextureScale());
                CVec2f t1(static_cast<float>(iX) / m_pWorld->GetTextureScale(), static_cast<float>(iY - 1) / m_pWorld->GetTextureScale());
                CVec2f t2(static_cast<float>(iX + 1) / m_pWorld->GetTextureScale(), static_cast<float>(iY - 1) / m_pWorld->GetTextureScale());

                v3Tangent += GetFaceTangent(v0, v1, v2, t0, t1, t2) * 0.5f;
            }

            // Triangle [(iX, iY) (iX + 1, iY - 1) (iX + 1, iY)]
            {
                CVec3f v0(iX * m_pWorld->GetScale(), iY * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX, iY));
                CVec3f v1((iX + 1) * m_pWorld->GetScale(), (iY - 1) * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX + 1, iY - 1));
                CVec3f v2((iX + 1) * m_pWorld->GetScale(), iY * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX + 1, iY));

                CVec2f t0(static_cast<float>(iX) / m_pWorld->GetTextureScale(), static_cast<float>(iY) / m_pWorld->GetTextureScale());
                CVec2f t1(static_cast<float>(iX + 1) / m_pWorld->GetTextureScale(), static_cast<float>(iY - 1) / m_pWorld->GetTextureScale());
                CVec2f t2(static_cast<float>(iX + 1) / m_pWorld->GetTextureScale(), static_cast<float>(iY) / m_pWorld->GetTextureScale());

                v3Tangent += GetFaceTangent(v0, v1, v2, t0, t1, t2) * 0.5f;
            }
        }
        else
        {
            // Triangle (iX, iY) (iX, iY - 1) (iX + 1, iY)
            CVec3f v0(iX * m_pWorld->GetScale(), iY * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX, iY));
            CVec3f v1(iX * m_pWorld->GetScale(), (iY - 1) * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX, iY - 1));
            CVec3f v2((iX + 1) * m_pWorld->GetScale(), iY * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX + 1, iY));

            CVec2f t0(static_cast<float>(iX) / m_pWorld->GetTextureScale(), static_cast<float>(iY) / m_pWorld->GetTextureScale());
            CVec2f t1(static_cast<float>(iX) / m_pWorld->GetTextureScale(), static_cast<float>(iY - 1) / m_pWorld->GetTextureScale());
            CVec2f t2(static_cast<float>(iX + 1) / m_pWorld->GetTextureScale(), static_cast<float>(iY) / m_pWorld->GetTextureScale());

            v3Tangent += GetFaceTangent(v0, v1, v2, t0, t1, t2);
        }
    }

    if (iX > 0 && iY < m_pWorld->GetGridHeight() - 1)
    {
        if (m_pWorld->GetTileSplit(iX - 1, iY) == SPLIT_NEG)
        {
            // Triangle [(iX, iY) (iX - 1, iY + 1) (iX - 1, iY)]
            {
                CVec3f v0(iX * m_pWorld->GetScale(), iY * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX, iY));
                CVec3f v1((iX - 1) * m_pWorld->GetScale(), (iY + 1) * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX - 1, iY + 1));
                CVec3f v2((iX - 1) * m_pWorld->GetScale(), iY * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX - 1, iY));

                CVec2f t0(static_cast<float>(iX) / m_pWorld->GetTextureScale(), static_cast<float>(iY) / m_pWorld->GetTextureScale());
                CVec2f t1(static_cast<float>(iX - 1) / m_pWorld->GetTextureScale(), static_cast<float>(iY + 1) / m_pWorld->GetTextureScale());
                CVec2f t2(static_cast<float>(iX - 1) / m_pWorld->GetTextureScale(), static_cast<float>(iY) / m_pWorld->GetTextureScale());

                v3Tangent += GetFaceTangent(v0, v1, v2, t0, t1, t2) * 0.5f;
            }

            // Triangle [(iX, iY) (iX, iY + 1) (iX - 1, iY + 1)]
            {
                CVec3f v0(iX * m_pWorld->GetScale(), iY * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX, iY));
                CVec3f v1(iX * m_pWorld->GetScale(), (iY + 1) * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX, iY + 1));
                CVec3f v2((iX - 1) * m_pWorld->GetScale(), (iY + 1) * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX - 1, iY + 1));

                CVec2f t0(static_cast<float>(iX) / m_pWorld->GetTextureScale(), static_cast<float>(iY) / m_pWorld->GetTextureScale());
                CVec2f t1(static_cast<float>(iX) / m_pWorld->GetTextureScale(), static_cast<float>(iY + 1) / m_pWorld->GetTextureScale());
                CVec2f t2(static_cast<float>(iX - 1) / m_pWorld->GetTextureScale(), static_cast<float>(iY + 1) / m_pWorld->GetTextureScale());

                v3Tangent += GetFaceTangent(v0, v1, v2, t0, t1, t2) * 0.5f;
            }
        }
        else
        {
            // Triangle (iX, iY) (iX, iY + 1) (iX - 1, iY)
            CVec3f v0(iX * m_pWorld->GetScale(), iY * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX, iY));
            CVec3f v1(iX * m_pWorld->GetScale(), (iY + 1) * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX, iY + 1));
            CVec3f v2((iX - 1) * m_pWorld->GetScale(), iY * m_pWorld->GetScale(), m_pWorld->GetGridPoint(iX - 1, iY));

            CVec2f t0(static_cast<float>(iX) / m_pWorld->GetTextureScale(), static_cast<float>(iY) / m_pWorld->GetTextureScale());
            CVec2f t1(static_cast<float>(iX) / m_pWorld->GetTextureScale(), static_cast<float>(iY + 1) / m_pWorld->GetTextureScale());
            CVec2f t2(static_cast<float>(iX - 1) / m_pWorld->GetTextureScale(), static_cast<float>(iY) / m_pWorld->GetTextureScale());

            v3Tangent += GetFaceTangent(v0, v1, v2, t0, t1, t2);
        }
    }

    v3Tangent.Normalize();
}
