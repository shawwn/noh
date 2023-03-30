// (C)2008 S2 Games
// c_tilecliffmap.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_tilecliffmap.h"
#include "c_world.h"
#include "c_buffer.h"
//=============================================================================

/*====================
  CTileCliffMap::CTileCliffMap
  ====================*/
CTileCliffMap::CTileCliffMap(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("TileCliffMap")),
m_pTileCliffs(NULL)
{
}


/*====================
  CTileCliffMap::~CTileCliffMap
  ====================*/
CTileCliffMap::~CTileCliffMap()
{
	Release();
}


/*====================
  CTileCliffMap::Release
  ====================*/
void	CTileCliffMap::Release()
{
	m_pWorld = NULL;

	if (m_pTileCliffs != NULL)
		K2_DELETE_ARRAY(m_pTileCliffs);
	m_pTileCliffs = NULL;
}


/*====================
  CTileCliffMap::Load
  ====================*/
bool	CTileCliffMap::Load(CArchive &archive, const CWorld *pWorld)
{
	try
	{
		Release();
		m_pWorld = pWorld;

		CFileHandle	file(m_sName, FILE_READ | FILE_BINARY, archive);
		if (!file.IsOpen())
			EX_ERROR(_T("No CliffMap found in archive"));

		// Read the dimensions
		int iWidth = file.ReadInt32();
		int iHeight = file.ReadInt32();

		if (iWidth != m_pWorld->GetTileWidth() || iHeight != m_pWorld->GetTileHeight())
			EX_ERROR(_T("Invalid CliffMap dimensions"));

		if (file.IsEOF())
			EX_ERROR(_T("Truncated CliffMap"));

		// Read the CliffMap
		int iSize(m_pWorld->GetTileArea() * sizeof(byte));
		m_pTileCliffs = K2_NEW_ARRAY(ctx_World, byte, m_pWorld->GetTileArea());

		if (file.Read((char*)m_pTileCliffs, iSize) < iSize)
			EX_ERROR(_T("Truncated CliffMap"));

		m_bChanged = false;
	}
	catch (CException &ex)
	{
		if (m_pTileCliffs != NULL)
			K2_DELETE_ARRAY(m_pTileCliffs);
		m_pTileCliffs = NULL;

		ex.Process(_T("CTileCliffMap::Load() - "), NO_THROW);
		return false;
	}

	return true;
}


/*====================
  CTileCliffMap::Generate
  ====================*/
bool	CTileCliffMap::Generate(const CWorld *pWorld)
{
	PROFILE("CTileCliffMap::Generate");

	try
	{
		Release();

		m_bChanged = true;
		m_pWorld = pWorld;
		if (m_pWorld == NULL)
			EX_ERROR(_T("CTileCliffMap needs a valid CWorld"));

		m_pTileCliffs = K2_NEW_ARRAY(ctx_World, byte, m_pWorld->GetTileArea());
		if (m_pTileCliffs == NULL)
			EX_ERROR(_T("Failed to allocate memory for CliffMap"));

		for (int i(0); i < m_pWorld->GetTileArea(); ++i)
			m_pTileCliffs[i] = 0;

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
  CTileCliffMap::Serialize
  ====================*/
bool	CTileCliffMap::Serialize(IBuffer *pBuffer)
{
	pBuffer->Clear();
	(*pBuffer) << m_pWorld->GetTileWidth() << m_pWorld->GetTileHeight();
	pBuffer->Append(m_pTileCliffs, m_pWorld->GetTileArea() * sizeof(byte));

	if (!pBuffer->GetFaults())
		return true;
	else
		return false;
}


/*====================
  CTileCliffMap::GetRegion
  ====================*/
bool	CTileCliffMap::GetRegion(const CRecti &recArea, void *pDest, int iLayer) const
{
	assert(recArea.IsNormalized());
	assert(m_pWorld->IsInBounds(recArea.left, recArea.top, TILE_SPACE));
	assert(m_pWorld->IsInBounds(recArea.right - 1, recArea.bottom - 1, TILE_SPACE));

	byte *pCliffDest(static_cast<byte*>(pDest));
	byte *pCliffSource = &m_pTileCliffs[m_pWorld->GetTileIndex(recArea.left, recArea.top)];

	for (int y(0); y < recArea.GetHeight(); ++y, pCliffDest += recArea.GetWidth(), pCliffSource += m_pWorld->GetTileWidth())
		MemManager.Copy(pCliffDest, pCliffSource, sizeof(byte) * recArea.GetWidth());

	return true;
}


/*====================
  CTileCliffMap::SetRegion
  ====================*/
bool	CTileCliffMap::SetRegion(const CRecti &recArea, void *pSource, int iLayer)
{
	assert(m_pWorld->IsInBounds(recArea.left, recArea.top, TILE_SPACE));
	assert(m_pWorld->IsInBounds(recArea.right - 1, recArea.bottom - 1, TILE_SPACE));

	byte *pCliffSource(static_cast<byte*>(pSource));
	byte *pCliffDest(&m_pTileCliffs[m_pWorld->GetTileIndex(recArea.left, recArea.top)]);

	for (int y(0); y < recArea.GetHeight(); ++y, pCliffSource += recArea.GetWidth(), pCliffDest += m_pWorld->GetTileWidth())
		MemManager.Copy(pCliffDest, pCliffSource, sizeof(byte) * recArea.GetWidth());
	
	CRecti recNormals(recArea.left - 1, recArea.top - 1, recArea.right + 1, recArea.bottom + 1);

	if (m_pWorld->ClipRect(recNormals, GRID_SPACE))
		m_pWorld->UpdateComponent(WORLD_VERT_NORMAL_MAP, recArea);

	m_pWorld->UpdateComponent(WORLD_TREE, recArea);

	m_bChanged = true;
	return true;
}


/*====================
  CTileCliffMap::GetCliff
  ====================*/
byte	CTileCliffMap::GetCliff(int x, int y)
{
	assert(x >= 0 && x < m_pWorld->GetTileWidth());
	assert(y >= 0 && y < m_pWorld->GetTileHeight());
	return m_pTileCliffs[m_pWorld->GetTileIndex(x, y)];
}


/*====================
  CTileCliffMap::SetCliff
  ====================*/
void	CTileCliffMap::SetCliff(int x, int y, byte yCliff)
{
	assert(x >= 0 && x < m_pWorld->GetTileWidth());
	assert(y >= 0 && y < m_pWorld->GetTileHeight());
	m_pTileCliffs[m_pWorld->GetTileIndex(x, y)] = yCliff;
}

