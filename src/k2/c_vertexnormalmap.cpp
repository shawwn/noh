// (C)2005 S2 Games
// c_vertexnormalmap.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_vertexnormalmap.h"
#include "c_world.h"
#include "c_buffer.h"
//=============================================================================

/*====================
  CVertexNormalMap::CVertexNormalMap
  ====================*/
CVertexNormalMap::CVertexNormalMap(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("VertexNormalMap")),
m_pVertexNormals(NULL)
{
}


/*====================
  CVertexNormalMap::~CVertexNormalMap
  ====================*/
CVertexNormalMap::~CVertexNormalMap()
{
	Release();
}


/*====================
  CVertexNormalMap::Release
  ====================*/
void	CVertexNormalMap::Release()
{
	m_pWorld = NULL;

	if (m_pVertexNormals != NULL)
		K2_DELETE_ARRAY(m_pVertexNormals);
	m_pVertexNormals = NULL;
}


/*====================
  CVertexNormalMap::Load
  ====================*/
bool	CVertexNormalMap::Load(CArchive &archive, const CWorld *pWorld)
{
	return Generate(pWorld);
}


/*====================
  CVertexNormalMap::Generate
  ====================*/
bool	CVertexNormalMap::Generate(const CWorld *pWorld)
{
	PROFILE("CVertexNormalMap::Generate");

	try
	{
		Release();
		m_bChanged = true;
		m_pWorld = pWorld;
		if (m_pWorld == NULL)
			EX_ERROR(_T("CVertexNormalMap needs a valid CWorld"));

		m_pVertexNormals = K2_NEW_ARRAY(ctx_World, CVec3f, m_pWorld->GetGridArea());
		if (m_pVertexNormals == NULL)
			EX_ERROR(_T("Failed to allocate memory for map data"));

		for (int iY(0); iY < m_pWorld->GetGridHeight(); ++iY)
		{
			for (int iX(0); iX < m_pWorld->GetGridWidth(); ++iX)
				CalculateVertexNormal(iX, iY);
		}
	}
	catch (CException &ex)
	{
		ex.Process(_T("CVertexNormalMap::Load() - "), NO_THROW);
		return false;
	}

	return true;
}


/*====================
  CVertexNormalMap::Update
  ====================*/
void	CVertexNormalMap::Update(const CRecti &recArea)
{
	for (int y(recArea.top); y < recArea.bottom; ++y)
	{
		for (int x(recArea.left); x < recArea.right; ++x)
			CalculateVertexNormal(x, y);
	}

	CRecti recNormals(recArea.left - 1, recArea.top - 1, recArea.right + 1, recArea.bottom + 1);

	if (m_pWorld->ClipRect(recNormals, GRID_SPACE))
	{
		m_pWorld->UpdateComponent(WORLD_VERT_TANGENT_MAP, recArea);
	}
}


/*====================
  CVertexNormalMap::GetRegion
  ====================*/
bool	CVertexNormalMap::GetRegion(int iStartX, int iStartY, int iWidth, int iHeight, void *pDest, int iDestSize, int iLayer) const
{
	CVec3f *pNormalDest = reinterpret_cast<CVec3f*>(pDest);
	CVec3f *pNormalSource = &m_pVertexNormals[m_pWorld->GetGridIndex(iStartX, iStartY)];

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
  CVertexNormalMap::SetRegion
  ====================*/
bool	CVertexNormalMap::SetRegion(int iStartX, int iStartY, int iWidth, int iHeight, void *pSource, int iSourceSize, int iLayer)
{
	CVec3f *pNormalSource = reinterpret_cast<CVec3f*>(pSource);
	CVec3f *pNormalDest = &m_pVertexNormals[m_pWorld->GetGridIndex(iStartX, iStartY)];

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
  CVertexNormalMap::GetVertexNormal
  ====================*/
const CVec3f&	CVertexNormalMap::GetVertexNormal(int x, int y)
{
	assert(x >= 0 && x < m_pWorld->GetGridWidth());
	assert(y >= 0 && y < m_pWorld->GetGridHeight());
	return m_pVertexNormals[m_pWorld->GetGridIndex(x, y)];
}


/*====================
  CVertexNormalMap::CalculateVertexNormal
  ====================*/
void	CVertexNormalMap::CalculateVertexNormal(int iX, int iY)
{
	assert(iX >= 0);
	assert(iX < m_pWorld->GetGridWidth());
	assert(iY >= 0);
	assert(iY < m_pWorld->GetGridHeight());
	assert(m_pWorld != NULL);

	CVec3f &v3Normal(m_pVertexNormals[m_pWorld->GetGridIndex(iX, iY)]);
	v3Normal.Clear();

	if (iX > 0 && iY > 0)
	{
		if (!m_pWorld->GetCliff(iX - 1, iY - 1))
		{
			if (m_pWorld->GetTileSplit(iX - 1, iY - 1) == SPLIT_NEG)
				v3Normal += m_pWorld->GetTileNormal(iX - 1, iY - 1, TRIANGLE_RIGHT);
			else
			{
				v3Normal += m_pWorld->GetTileNormal(iX - 1, iY - 1, TRIANGLE_LEFT) * 0.5f;
				v3Normal += m_pWorld->GetTileNormal(iX - 1, iY - 1, TRIANGLE_RIGHT) * 0.5f;
			}
		}
	}

	if (iX < m_pWorld->GetTileWidth() && iY < m_pWorld->GetTileHeight())
	{
		if (!m_pWorld->GetCliff(iX, iY))
		{
			if (m_pWorld->GetTileSplit(iX, iY) == SPLIT_NEG)
				v3Normal += m_pWorld->GetTileNormal(iX, iY, TRIANGLE_LEFT);
			else
			{
				v3Normal += m_pWorld->GetTileNormal(iX, iY, TRIANGLE_LEFT) * 0.5f;
				v3Normal += m_pWorld->GetTileNormal(iX, iY, TRIANGLE_RIGHT) * 0.5f;
			}
		}
	}

	if (iX < m_pWorld->GetTileWidth() && iY > 0)
	{
		if (!m_pWorld->GetCliff(iX, iY - 1))
		{
			if (m_pWorld->GetTileSplit(iX, iY - 1) == SPLIT_NEG)
			{
				v3Normal += m_pWorld->GetTileNormal(iX, iY - 1, TRIANGLE_LEFT) * 0.5f;
				v3Normal += m_pWorld->GetTileNormal(iX, iY - 1, TRIANGLE_RIGHT) * 0.5f;
			}
			else
				v3Normal += m_pWorld->GetTileNormal(iX, iY - 1, TRIANGLE_LEFT);
		}

	}

	if (iX > 0 && iY < m_pWorld->GetTileHeight())
	{
		if (!m_pWorld->GetCliff(iX - 1, iY))
		{
			if (m_pWorld->GetTileSplit(iX - 1, iY) == SPLIT_NEG)
			{
				v3Normal += m_pWorld->GetTileNormal(iX - 1, iY, TRIANGLE_LEFT) * 0.5f;
				v3Normal += m_pWorld->GetTileNormal(iX - 1, iY, TRIANGLE_RIGHT) * 0.5f;
			}
			else
				v3Normal += m_pWorld->GetTileNormal(iX - 1, iY, TRIANGLE_RIGHT);
		}
	}

	if (v3Normal.LengthSq() == 0.0)
		v3Normal = CVec3f(0.0f, 0.0f, 1.0f);
	else
		v3Normal.Normalize();
}
