// (C)2010 S2 Games
// c_tilerampmap.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_tilerampmap.h"
#include "c_world.h"
#include "c_buffer.h"
//=============================================================================

/*====================
  CTileRampMap::CTileRampMap
  ====================*/
CTileRampMap::CTileRampMap(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("TileRampMap")),
m_pTileRamps(NULL)
{
}


/*====================
  CTileRampMap::~CTileRampMap
  ====================*/
CTileRampMap::~CTileRampMap()
{
	Release();
}


/*====================
  CTileRampMap::Release
  ====================*/
void	CTileRampMap::Release()
{
	m_pWorld = NULL;

	if (m_pTileRamps != NULL)
		K2_DELETE_ARRAY(m_pTileRamps);
	m_pTileRamps = NULL;
}


/*====================
  CTileRampMap::Load
  ====================*/
bool	CTileRampMap::Load(CArchive &archive, const CWorld *pWorld)
{
	try
	{
		Release();
		m_pWorld = pWorld;

		CFileHandle	file(m_sName, FILE_READ | FILE_BINARY, archive);
		if (!file.IsOpen())
			EX_ERROR(_T("No RampMap found in archive"));

		// Read the dimensions
		int iWidth = file.ReadInt32();
		int iHeight = file.ReadInt32();

		if (iWidth != m_pWorld->GetCliffTileWidth() || iHeight != m_pWorld->GetCliffTileHeight())
			EX_ERROR(_T("Invalid RampMap dimensions"));

		if (file.IsEOF())
			EX_ERROR(_T("Truncated RampMap"));

		// Read the CliffMap
		int iSize(m_pWorld->GetCliffTileArea() * sizeof(byte));
		m_pTileRamps = K2_NEW_ARRAY(ctx_World, byte, m_pWorld->GetCliffTileArea());

		if (file.Read((char*)m_pTileRamps, iSize) < iSize)
			EX_ERROR(_T("Truncated RampMap"));

		m_bChanged = false;
	}
	catch (CException &ex)
	{
		if (m_pTileRamps != NULL)
			K2_DELETE_ARRAY(m_pTileRamps);
		m_pTileRamps = NULL;

		ex.Process(_T("CTileCliffMap::Load() - "), NO_THROW);
		return false;
	}

	return true;
}


/*====================
  CTileRampMap::Generate
  ====================*/
bool	CTileRampMap::Generate(const CWorld *pWorld)
{
	PROFILE("CTileCliffMap::Generate");

	try
	{
		Release();

		m_bChanged = true;
		m_pWorld = pWorld;
		if (m_pWorld == NULL)
			EX_ERROR(_T("CTileRampMap needs a valid CWorld"));

		m_pTileRamps = K2_NEW_ARRAY(ctx_World, byte, m_pWorld->GetCliffTileArea());
		if (m_pTileRamps == NULL)
			EX_ERROR(_T("Failed to allocate memory for RampMap"));

		for (int i(0); i < m_pWorld->GetCliffTileArea(); ++i)
			m_pTileRamps[i] = 0;

		return true;
	}
	catch (CException &ex)
	{
		Release();
		ex.Process(_T("CTileCliffMap::Generate() - "));
		return false;
	}
}


/*====================
  CTileRampMap::Serialize
  ====================*/
bool	CTileRampMap::Serialize(IBuffer *pBuffer)
{
	pBuffer->Clear();
	(*pBuffer) << m_pWorld->GetCliffTileWidth() << m_pWorld->GetCliffTileHeight();
	pBuffer->Append(m_pTileRamps, m_pWorld->GetCliffTileArea() * sizeof(byte));

	if (!pBuffer->GetFaults())
		return true;
	else
		return false;
}


/*====================
  CTileRampMap::GetRegion
  ====================*/
bool	CTileRampMap::GetRegion(const CRecti &recArea, void *pDest, int iLayer) const
{
	assert(recArea.IsNormalized());
	assert(recArea.left >= 0 && recArea.right < m_pWorld->GetCliffTileWidth());
	assert(recArea.top >= 0 && recArea.bottom < m_pWorld->GetCliffTileHeight());

	byte *pRampDest(static_cast<byte*>(pDest));
	byte *pRampSource = &m_pTileRamps[m_pWorld->GetCliffTileIndex(recArea.left, recArea.top)];

	for (int y(0); y < recArea.GetHeight(); ++y, pRampDest += recArea.GetWidth(), pRampSource += m_pWorld->GetCliffTileWidth())
		MemManager.Copy(pRampDest, pRampSource, sizeof(byte) * recArea.GetWidth());

	return true;
}


/*====================
  CTileRampMap::SetRegion
  ====================*/
bool	CTileRampMap::SetRegion(const CRecti &recArea, void *pSource, int iLayer)
{
	assert(recArea.left >= 0 && recArea.right < m_pWorld->GetCliffTileWidth());
	assert(recArea.top >= 0 && recArea.bottom < m_pWorld->GetCliffTileHeight());

	byte *pRampSource(static_cast<byte*>(pSource));
	byte *pRampDest(&m_pTileRamps[m_pWorld->GetCliffTileIndex(recArea.left, recArea.top)]);

	for (int y(0); y < recArea.GetHeight(); ++y, pRampSource += recArea.GetWidth(), pRampDest += m_pWorld->GetCliffTileWidth())
		MemManager.Copy(pRampDest, pRampSource, sizeof(byte) * recArea.GetWidth());
	
	CRecti recNormals(recArea.left - 1, recArea.top - 1, recArea.right + 1, recArea.bottom + 1);

	if (m_pWorld->ClipRect(recNormals, GRID_SPACE))
		m_pWorld->UpdateComponent(WORLD_VERT_NORMAL_MAP, recArea);

	m_pWorld->UpdateComponent(WORLD_TREE, recArea);

	m_bChanged = true;
	return true;
}


/*====================
  CTileRampMap::GetRamp
  ====================*/
byte	CTileRampMap::GetRamp(int x, int y)
{
	assert(x >= 0 && x < m_pWorld->GetCliffTileWidth());
	assert(y >= 0 && y < m_pWorld->GetCliffTileHeight());
	return m_pTileRamps[m_pWorld->GetCliffTileIndex(x, y)];
}

/*====================
  CTileRampMap::GetRampByTile
  ====================*/
byte	CTileRampMap::GetRampByTile(int x, int y)
{
	x /= m_pWorld->GetCliffSize();
	y /= m_pWorld->GetCliffSize();
	assert(x >= 0 && x < m_pWorld->GetCliffTileWidth());
	assert(y >= 0 && y < m_pWorld->GetCliffTileHeight());
	return m_pTileRamps[m_pWorld->GetCliffTileIndex(x, y)];
}


/*====================
  CTileRampMap::SetRamp
  ====================*/
void	CTileRampMap::SetRamp(int x, int y, byte yRamp)
{
	assert(x >= 0 && x < m_pWorld->GetCliffTileWidth());
	assert(y >= 0 && y < m_pWorld->GetCliffTileHeight());
	m_pTileRamps[m_pWorld->GetCliffTileIndex(x, y)] = yRamp;
}


/*====================
  CTileRampMap::SetRampByTile
  ====================*/
void	CTileRampMap::SetRampByTile(int x, int y, byte yRamp)
{
	x /= m_pWorld->GetCliffSize();
	y /= m_pWorld->GetCliffSize();
	assert(x >= 0 && x < m_pWorld->GetCliffTileWidth());
	assert(y >= 0 && y < m_pWorld->GetCliffTileHeight());
	m_pTileRamps[m_pWorld->GetCliffTileIndex(x, y)] = yRamp;
}
