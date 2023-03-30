// (C)2005 S2 Games
// c_heightmap.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_heightmap.h"
#include "c_world.h"
#include "c_buffer.h"
//=============================================================================

/*====================
  CHeightMap::CHeightMap
  ====================*/
CHeightMap::CHeightMap(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("HeightMap")),
m_pHeightMap(NULL)
{
}


/*====================
  CHeightMap::~CHeightMap
  ====================*/
CHeightMap::~CHeightMap()
{
	Release();
}


/*====================
  CHeightMap::Release
  ====================*/
void	CHeightMap::Release()
{
	PROFILE("CHeightmap::Release");

	m_pWorld = NULL;

	SAFE_DELETE_ARRAY(m_pHeightMap);
}


/*====================
  CHeightMap::Load
  ====================*/
bool	CHeightMap::Load(CArchive &archive, const CWorld *pWorld)
{
	PROFILE("CHeightmap::Load");

	try
	{
		bool newFileFormat = false; // UTTAR: Filesize-optimized format...
		Release();

		m_pWorld = pWorld;
		if (m_pWorld == NULL)
			EX_ERROR(_T("Invalid CWorld"));

		CFileHandle	hHeightMap(m_sName, FILE_READ | FILE_BINARY, archive);
		if (!hHeightMap.IsOpen())
			EX_ERROR(_T("No HeightMap found in archive"));

		// Read the dimensions
		int iWidth = hHeightMap.ReadInt32();
		int iHeight = hHeightMap.ReadInt32();

		// UTTAR: In order to keep full backward compatibility, I will differentiate
		// between the old and the new file formats based on iWidth being negative.
		if (iWidth < 0)
		{
			iWidth = -iWidth;
			newFileFormat = true;
		}

		if (iWidth != m_pWorld->GetGridWidth() || iHeight != m_pWorld->GetGridHeight())
			EX_ERROR(_T("Invalid HeightMap dimensions"));

		if (hHeightMap.IsEOF())
			EX_ERROR(_T("Truncated HeightMap"));

		// Read the heightmap
		m_pHeightMap = K2_NEW_ARRAY(ctx_World, float, m_pWorld->GetGridArea());

		if (newFileFormat)
		{
			// UTTAR: New optimized file format; uses 3 bytes instead of one float!
			for (int iTile(0); iTile < m_pWorld->GetGridArea(); ++iTile)
				m_pHeightMap[iTile] = (float)(hHeightMap.ReadByte())/256.0f;
			for (int iTile(0); iTile < m_pWorld->GetGridArea(); ++iTile)
				m_pHeightMap[iTile] += (float)(hHeightMap.ReadByte());
			for (int iTile(0); iTile < m_pWorld->GetGridArea(); ++iTile)
			{
				m_pHeightMap[iTile] += (float)(hHeightMap.ReadByte())*256.0f;
				m_pHeightMap[iTile] -= 32768;
			}
		}
		else
		{
			for (int iTile(0); iTile < m_pWorld->GetGridArea(); ++iTile)
				m_pHeightMap[iTile] = hHeightMap.ReadFloat();
		}

		m_bChanged = false;

	}
	catch (CException &ex)
	{
		SAFE_DELETE_ARRAY(m_pHeightMap);
		ex.Process(_T("CHeightMap::Load() - "), NO_THROW);
		return false;
	}

	return true;
}


/*====================
  CHeightMap::Generate
  ====================*/
bool	CHeightMap::Generate(const CWorld *pWorld)
{
	PROFILE("CHeightmap::Generate");

	Release();

	m_bChanged = true;
	m_pWorld = pWorld;

	m_pHeightMap = K2_NEW_ARRAY(ctx_World, float, m_pWorld->GetGridArea());
	if (m_pHeightMap == NULL)
		return false;

	MemManager.Set(m_pHeightMap, 0, sizeof(float) * m_pWorld->GetGridArea());
	return true;
}


/*====================
  CHeightMap::Serialize
  ====================*/
bool	CHeightMap::Serialize(IBuffer *pBuffer)
{
	pBuffer->Clear();
	// UTTAR: Negative width shows that this is our new file format.
	// This file format uses 3 BYTES rather than floats to optimize download size.
	(*pBuffer) << -m_pWorld->GetGridWidth() << m_pWorld->GetGridHeight();

	float *pHeightMapCopy(K2_NEW_ARRAY(ctx_World, float, m_pWorld->GetGridArea()));

	for (int iTile(0); iTile < m_pWorld->GetGridArea(); ++iTile)
	{
		pHeightMapCopy[iTile] = m_pHeightMap[iTile] + 32768.0f;
		(*pBuffer) << ((byte)((int)((pHeightMapCopy[iTile]*256.0f)+0.5f)%256));
	}
	for (int iTile(0); iTile < m_pWorld->GetGridArea(); ++iTile)
		(*pBuffer) << ((byte)((int)(pHeightMapCopy[iTile])%256));
	for (int iTile(0); iTile < m_pWorld->GetGridArea(); ++iTile)
		(*pBuffer) << ((byte)((int)(pHeightMapCopy[iTile]/256.0f)%256));

	K2_DELETE_ARRAY(pHeightMapCopy);

	if (pBuffer->GetFaults())
		return false;

	return true;
}


/*====================
  CHeightMap::GetRegion
  ====================*/
bool	CHeightMap::GetRegion(const CRecti &recArea, void *pDest, int iLayer) const
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
  CHeightMap::SetRegion
  ====================*/
bool	CHeightMap::SetRegion(const CRecti &recArea, void *pSource, int iLayer)
{
	assert(recArea.GetArea() > 0);
	assert(m_pWorld->IsInBounds(recArea.left, recArea.top, GRID_SPACE));
	assert(m_pWorld->IsInBounds(recArea.right - 1, recArea.bottom - 1, GRID_SPACE));

	float *pHeightSource(static_cast<float*>(pSource));
	float *pHeightDest(&m_pHeightMap[m_pWorld->GetGridIndex(recArea.left, recArea.top)]);

	for (int y(0); y < recArea.GetHeight(); ++y, pHeightSource += recArea.GetWidth(), pHeightDest += m_pWorld->GetGridWidth())
		MemManager.Copy(pHeightDest, pHeightSource, sizeof(float) * recArea.GetWidth());

	CRecti recTiles(recArea.left - 1, recArea.top - 1, recArea.right, recArea.bottom);

	if (m_pWorld->ClipRect(recTiles, TILE_SPACE))
	{
		m_pWorld->UpdateComponent(WORLD_TILE_SPLIT_MAP, recTiles);
		m_pWorld->UpdateComponent(WORLD_TILE_NORMAL_MAP, recTiles);
		m_pWorld->UpdateComponent(WORLD_TREE, recTiles);
	}

	m_bChanged = true;
	return true;
}


/*====================
  CHeightMap::GetGridPoint
  ====================*/
float	CHeightMap::GetGridPoint(int iX, int iY)
{
	if (!m_pWorld->IsInBounds(iX, iY, GRID_SPACE))
	{
		iX = CLAMP(iX, 0, m_pWorld->GetGridWidth() - 1);
		iY = CLAMP(iY, 0, m_pWorld->GetGridHeight() - 1);
		Console.Warn << _T("CHeightMap::GetGridPoint() - Out of bound coordinate") << newl;
	}

	return m_pHeightMap[m_pWorld->GetGridIndex(iX, iY)];
}

float	CHeightMap::GetGridPoint(int iIndex)
{
	assert(iIndex >= 0 && iIndex < m_pWorld->GetGridArea());
	return m_pHeightMap[iIndex];
}


/*====================
  CHeightMap::CalcMaxHeight
  ====================*/
float	CHeightMap::CalcMaxHeight(const CRecti &recArea) const
{
	float fMaxHeight(-FAR_AWAY);

	for (int iY(recArea.top); iY <= recArea.bottom; ++iY)
	{
		for (int iX(recArea.left); iX <= recArea.right; ++iX)
		{
			uint uiGridIndex(m_pWorld->GetGridIndex(iX, iY));

			if (uiGridIndex >= (uint)m_pWorld->GetGridArea())
			{
				Console.Warn << _T("CHeightMap::CalcMaxHeight() - Grid Index exceeded max array size") << newl;
				return fMaxHeight;
			}

			fMaxHeight = MAX(fMaxHeight, m_pHeightMap[uiGridIndex]);
		}
	}

	return fMaxHeight;
}


/*====================
  CHeightMap::CalcMinHeight
  ====================*/
float	CHeightMap::CalcMinHeight(const CRecti &recArea) const
{
	float fMinHeight(FAR_AWAY);

	for (int iY(recArea.top); iY <= recArea.bottom; ++iY)
	{
		for (int iX(recArea.left); iX <= recArea.right; ++iX)
		{
			uint uiGridIndex(m_pWorld->GetGridIndex(iX, iY));

			if (uiGridIndex >= (uint)m_pWorld->GetGridArea())
			{
				Console.Warn << _T("CHeightMap::CalcMinHeight() - Grid Index exceeded max array size") << newl;
				return fMinHeight;
			}

			fMinHeight = MIN(fMinHeight, m_pHeightMap[uiGridIndex]);
		}
	}

	return fMinHeight;
}
