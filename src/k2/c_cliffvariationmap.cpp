// (C)2005 S2 Games
// c_VertexCliffMap.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_cliffvariationmap.h"
#include "c_world.h"
#include "c_buffer.h"
//=============================================================================

/*====================
  CCliffVariationMap::CCliffVariationMap
  ====================*/
CCliffVariationMap::CCliffVariationMap(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("VariationCliffMap")),
m_pTileCliffDefinition(NULL),
m_pTileCliffVariation(NULL)
{
}


/*====================
  CCliffVariationMap::~CCliffVariationMap
  ====================*/
CCliffVariationMap::~CCliffVariationMap()
{
	Release();
}


/*====================
  CCliffVariationMap::Release
  ====================*/
void	CCliffVariationMap::Release()
{
	m_pWorld = NULL;

	if (m_pTileCliffDefinition != NULL)
		K2_DELETE_ARRAY(m_pTileCliffDefinition);
	m_pTileCliffDefinition = NULL;

	if (m_pTileCliffVariation != NULL)
		K2_DELETE_ARRAY(m_pTileCliffVariation);
	m_pTileCliffVariation = NULL;
}


/*====================
  CCliffVariationMap::Load
  ====================*/
bool	CCliffVariationMap::Load(CArchive &archive, const CWorld *pWorld)
{
	try
	{
		Release();
		m_pWorld = pWorld;

		CFileHandle	file(m_sName, FILE_READ | FILE_BINARY, archive);
		if (!file.IsOpen())
			EX_ERROR(_T("No CliffVariationMap found in archive"));

		// Read the dimensions
		int iWidth = file.ReadInt32();
		int iHeight = file.ReadInt32();

		if (iWidth != m_pWorld->GetCliffGridWidth() || iHeight != m_pWorld->GetCliffGridHeight())
			EX_ERROR(_T("Invalid CliffVariationMap dimensions"));

		m_iCliffMapWidth = iWidth;

		if (file.IsEOF())
			EX_ERROR(_T("Truncated CliffVariationMap"));

		// Read the CliffMap
		int iSize(m_pWorld->GetCliffGridArea() * sizeof(uint));
		m_pTileCliffVariation = K2_NEW_ARRAY(ctx_World, uint, m_pWorld->GetCliffGridArea());
		m_pTileCliffDefinition = K2_NEW_ARRAY(ctx_World, uint, m_pWorld->GetCliffGridArea());

		if (file.Read((char*)m_pTileCliffVariation, iSize) < iSize)
			EX_ERROR(_T("Truncated CliffVariationMap"));

		if (file.Read((char*)m_pTileCliffDefinition, iSize) < iSize)
			EX_ERROR(_T("Truncated CliffDefinitionMap"));

		m_bChanged = false;
	}
	catch (CException &ex)
	{
		if (m_pTileCliffVariation != NULL)
			K2_DELETE_ARRAY(m_pTileCliffVariation);
		m_pTileCliffVariation = NULL;

		if (m_pTileCliffDefinition != NULL)
			K2_DELETE_ARRAY(m_pTileCliffDefinition);
		m_pTileCliffDefinition = NULL;

		ex.Process(_T("CVertexCliff::Load() - "), NO_THROW);
		return false;
	}

	return true;
}

/*====================
CCliffVariationMap::Serialize
  ====================*/
bool	CCliffVariationMap::Serialize(IBuffer *pBuffer)
{
	pBuffer->Clear();
	(*pBuffer) << m_pWorld->GetCliffGridWidth() << m_pWorld->GetCliffGridHeight();
	pBuffer->Append(m_pTileCliffVariation, m_pWorld->GetCliffGridArea() * sizeof(uint));
	pBuffer->Append(m_pTileCliffDefinition, m_pWorld->GetCliffGridArea() * sizeof(uint));

	if (pBuffer->GetFaults())
		return false;

	return true;
}

/*====================
  CCliffVariationMap::Generate
  ====================*/
bool	CCliffVariationMap::Generate(const CWorld *pWorld)
{
	PROFILE("CCliffVariationMap::Generate");

	try
	{
		Release();
		m_bChanged = true;
		m_pWorld = pWorld;
		if (m_pWorld == NULL)
			EX_ERROR(_T("CCliffVariationMap needs a valid CWorld"));

		int iWorldSize = pow((float)2, (float)m_pWorld->GetSize());
		m_iCliffMapWidth = iWorldSize / pWorld->GetCliffSize();
		
		m_pTileCliffDefinition = K2_NEW_ARRAY(ctx_World, uint, m_iCliffMapWidth*m_iCliffMapWidth);
		m_pTileCliffVariation = K2_NEW_ARRAY(ctx_World, uint, m_iCliffMapWidth*m_iCliffMapWidth);

		if (m_pTileCliffDefinition == NULL)
			EX_ERROR(_T("Failed to allocate memory for map data"));
		if (m_pTileCliffVariation == NULL)
			EX_ERROR(_T("Failed to allocate memory for map data"));

		MemManager.Set(m_pTileCliffDefinition, 0, sizeof(uint) * (m_iCliffMapWidth * m_iCliffMapWidth));
		MemManager.Set(m_pTileCliffVariation, 0, sizeof(uint) * (m_iCliffMapWidth * m_iCliffMapWidth));

	}
	catch (CException &ex)
	{
		ex.Process(_T("CCliffVariationMap::Load() - "), NO_THROW);
		return false;
	}

	return true;
}


/*====================
  CCliffVariationMap::Update
  ====================*/
void	CCliffVariationMap::Update(const CRecti &recArea)
{
	//for (int y(recArea.top); y < recArea.bottom; ++y)
	//{
	//	for (int x(recArea.left); x < recArea.right; ++x)
	//		CalculateVertexNormal(x, y);
	//}
	//
	//CRecti recNormals(recArea.left - 1, recArea.top - 1, recArea.right + 1, recArea.bottom + 1);
	//
	//if (m_pWorld->ClipRect(recNormals, GRID_SPACE))
	//{
	//	m_pWorld->UpdateComponent(WORLD_VERT_TANGENT_MAP, recArea);
	//}
}

/*====================
  CCliffVariationMap::GetTileCliff
  ====================*/
const uint	CCliffVariationMap::GetTileCliff(int x, int y)
{
	assert(x >= 0 && x <= m_iCliffMapWidth);
	assert(y >= 0 && y <= m_iCliffMapWidth);
	return (y * m_iCliffMapWidth) + x;
}

/*====================
  CCliffVariationMap::GetRegion
  ====================*/
bool	CCliffVariationMap::GetRegion(const CRecti &recArea, void *pDest, int iLayer) const
{
	if (!(iLayer == 0 || iLayer == 1))
		return false;

	assert(m_pWorld->IsInBounds(recArea.left, recArea.top, GRID_SPACE));
	assert(m_pWorld->IsInBounds(recArea.right - 1, recArea.bottom - 1, GRID_SPACE));

	uint *pCliffDest(static_cast<uint*>(pDest));
	uint *pCliffSource = 0;
	if (iLayer == 0)
		pCliffSource = &m_pTileCliffDefinition[m_pWorld->GetTileCliff(recArea.left, recArea.top)];
	else if (iLayer == 1)
		pCliffSource = &m_pTileCliffVariation[m_pWorld->GetTileCliff(recArea.left, recArea.top)];

	for (int y(0); y < recArea.GetHeight(); ++y, pCliffSource += m_iCliffMapWidth, pCliffDest += recArea.GetWidth())
		MemManager.Copy(pCliffDest, pCliffSource, sizeof(uint) * recArea.GetWidth());

	return true;
}


/*====================
  CCliffVariationMap::SetRegion
  ====================*/
bool	CCliffVariationMap::SetRegion(const CRecti &recArea, void *pSource, int iLayer)
{
	if (!(iLayer == 0 || iLayer == 1))
		return false;

	assert(m_pWorld->IsInBounds(recArea.left, recArea.top, GRID_SPACE));
	assert(m_pWorld->IsInBounds(recArea.right - 1, recArea.bottom - 1, GRID_SPACE));

	uint *pCliffSource(static_cast<uint*>(pSource));
	uint *pCliffDest = 0;
	if (iLayer == 0)
		pCliffDest = &m_pTileCliffDefinition[m_pWorld->GetTileCliff(recArea.left, recArea.top)];
	else if (iLayer == 1)
		pCliffDest = &m_pTileCliffVariation[m_pWorld->GetTileCliff(recArea.left, recArea.top)];

	for (int y(0); y < recArea.GetHeight(); ++y, pCliffSource += recArea.GetWidth(), pCliffDest += m_iCliffMapWidth)
		MemManager.Copy(pCliffDest, pCliffSource, sizeof(uint) * recArea.GetWidth());

	return true;
}
