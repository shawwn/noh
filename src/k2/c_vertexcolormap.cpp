// (C)2005 S2 Games
// c_vertexcolormap.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_vertexcolormap.h"
#include "c_world.h"
#include "c_buffer.h"
//=============================================================================

/*====================
  CVertexColorMap::CVertexColorMap
  ====================*/
CVertexColorMap::CVertexColorMap(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("VertexColorMap")),
m_pVertexColors(NULL)
{
}


/*====================
  CVertexColorMap::~CVertexColorMap
  ====================*/
CVertexColorMap::~CVertexColorMap()
{
	Release();
}


/*====================
  CVertexColorMap::Release
  ====================*/
void	CVertexColorMap::Release()
{
	m_pWorld = NULL;

	if (m_pVertexColors != NULL)
		K2_DELETE_ARRAY(m_pVertexColors);
	m_pVertexColors = NULL;
}


/*====================
  CVertexColorMap::Load
  ====================*/
bool	CVertexColorMap::Load(CArchive &archive, const CWorld *pWorld)
{
	try
	{
		Release();
		m_pWorld = pWorld;

		CFileHandle	file(m_sName, FILE_READ | FILE_BINARY, archive);
		if (!file.IsOpen())
			EX_ERROR(_T("No ColorMap found in archive"));

		// Read the dimensions
		int iWidth = file.ReadInt32();
		int iHeight = file.ReadInt32();

		if (iWidth != m_pWorld->GetGridWidth() || iHeight != m_pWorld->GetGridHeight())
			EX_ERROR(_T("Invalid ColorMap dimensions"));

		if (file.IsEOF())
			EX_ERROR(_T("Truncated ColorMap"));

		// Read the ColorMap
		int iSize(m_pWorld->GetGridArea() * sizeof(CVec4b));
		m_pVertexColors = K2_NEW_ARRAY(ctx_World, CVec4b, m_pWorld->GetGridArea());

		if (file.Read((char*)m_pVertexColors, iSize) < iSize)
			EX_ERROR(_T("Truncated ColorMap"));

		m_bChanged = false;
	}
	catch (CException &ex)
	{
		if (m_pVertexColors != NULL)
			K2_DELETE_ARRAY(m_pVertexColors);
		m_pVertexColors = NULL;

		ex.Process(_T("CVertexColorMap::Load() - "), NO_THROW);
		return false;
	}

	return true;
}


/*====================
  CVertexColorMap::Generate
  ====================*/
bool	CVertexColorMap::Generate(const CWorld *pWorld)
{
	PROFILE("CVertexColorMap::Generate");

	try
	{
		Release();

		m_bChanged = true;
		m_pWorld = pWorld;
		if (m_pWorld == NULL)
			EX_ERROR(_T("CVertexColorMap needs a valid CWorld"));

		m_pVertexColors = K2_NEW_ARRAY(ctx_World, CVec4b, m_pWorld->GetGridArea());
		if (m_pVertexColors == NULL)
			EX_ERROR(_T("Failed to allocate memory for ColorMap"));

		for (int i(0); i < m_pWorld->GetGridArea(); ++i)
			m_pVertexColors[i].Set(255, 255, 255, 0);

		return true;
	}
	catch (CException &ex)
	{
		Release();
		ex.Process(_T("CVertexColorMap::Generate() - "));
		return false;
	}
}


/*====================
  CVertexColorMap::Serialize
  ====================*/
bool	CVertexColorMap::Serialize(IBuffer *pBuffer)
{
	pBuffer->Clear();
	(*pBuffer) << m_pWorld->GetGridWidth() << m_pWorld->GetGridHeight();
	pBuffer->Append(m_pVertexColors, m_pWorld->GetGridArea() * sizeof(CVec4b));

	if (!pBuffer->GetFaults())
		return true;
	else
		return false;
}


/*====================
  CVertexColorMap::GetRegion
  ====================*/
bool	CVertexColorMap::GetRegion(const CRecti &recArea, void *pDest, int iLayer) const
{
	assert(recArea.IsNormalized());
	assert(m_pWorld->IsInBounds(recArea.left, recArea.top, GRID_SPACE));
	assert(m_pWorld->IsInBounds(recArea.right - 1, recArea.bottom - 1, GRID_SPACE));

	CVec4b *pColorDest(static_cast<CVec4b*>(pDest));
	CVec4b *pColorSource = &m_pVertexColors[m_pWorld->GetGridIndex(recArea.left, recArea.top)];

	for (int y(0); y < recArea.GetHeight(); ++y, pColorDest += recArea.GetWidth(), pColorSource += m_pWorld->GetGridWidth())
		MemManager.Copy(pColorDest, pColorSource, sizeof(CVec4b) * recArea.GetWidth());

	return true;
}


/*====================
  CVertexColorMap::SetRegion
  ====================*/
bool	CVertexColorMap::SetRegion(const CRecti &recArea, void *pSource, int iLayer)
{
	assert(m_pWorld->IsInBounds(recArea.left, recArea.top, GRID_SPACE));
	assert(m_pWorld->IsInBounds(recArea.right - 1, recArea.bottom - 1, GRID_SPACE));

	CVec4b *pColorSource(static_cast<CVec4b*>(pSource));
	CVec4b *pColorDest(&m_pVertexColors[m_pWorld->GetGridIndex(recArea.left, recArea.top)]);

	for (int y(0); y < recArea.GetHeight(); ++y, pColorSource += recArea.GetWidth(), pColorDest += m_pWorld->GetGridWidth())
		MemManager.Copy(pColorDest, pColorSource, sizeof(CVec4b) * recArea.GetWidth());

	m_bChanged = true;
	return true;
}


/*====================
  CVertexColorMap::GetVertexColor
  ====================*/
const CVec4b&	CVertexColorMap::GetVertexColor(int x, int y)
{
	assert(x >= 0 && x < m_pWorld->GetGridWidth());
	assert(y >= 0 && y < m_pWorld->GetGridHeight());
	return m_pVertexColors[m_pWorld->GetGridIndex(x, y)];
}


/*====================
  CVertexColorMap::SetColor
  ====================*/
void	CVertexColorMap::SetColor(int x, int y, CVec3b v3Color)
{
	assert(x >= 0 && x < m_pWorld->GetGridWidth());
	assert(y >= 0 && y < m_pWorld->GetGridHeight());
	CVec4b v4Color(v3Color[R], v3Color[G], v3Color[B], 255);
	m_pVertexColors[m_pWorld->GetGridIndex(x, y)] = v4Color;
}


/*====================
  CVertexColorMap::SetColor
  ====================*/
void	CVertexColorMap::SetColor(int x, int y, CVec4b v4Color)
{
	assert(x >= 0 && x < m_pWorld->GetGridWidth());
	assert(y >= 0 && y < m_pWorld->GetGridHeight());
	m_pVertexColors[m_pWorld->GetGridIndex(x, y)] = v4Color;
}
