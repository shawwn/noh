// (C)2006 S2 Games
// c_texelalphamap.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_texelalphamap.h"
#include "c_world.h"
#include "c_buffer.h"
//=============================================================================

/*====================
  CTexelAlphaMap::CTexelAlphaMap
  ====================*/
CTexelAlphaMap::CTexelAlphaMap(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("TexelAlphaMap")),
m_pTexelAlpha(NULL)
{
}


/*====================
  CTexelAlphaMap::~CTexelAlphaMap
  ====================*/
CTexelAlphaMap::~CTexelAlphaMap()
{
	Release();
}


/*====================
  CTexelAlphaMap::Release
  ====================*/
void	CTexelAlphaMap::Release()
{
	m_pWorld = NULL;

	if (m_pTexelAlpha != NULL)
		K2_DELETE_ARRAY(m_pTexelAlpha);
	m_pTexelAlpha = NULL;
}


/*====================
  CTexelAlphaMap::Load
  ====================*/
bool	CTexelAlphaMap::Load(CArchive &archive, const CWorld *pWorld)
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

		if (iWidth != m_pWorld->GetTexelWidth() || iHeight != m_pWorld->GetTexelHeight())
			EX_ERROR(_T("Invalid AlphaMap dimensions"));

		if (file.IsEOF())
			EX_ERROR(_T("Truncated AlphaMap"));

		// Read the ColorMap
		int iSize(m_pWorld->GetTexelArea());
		m_pTexelAlpha = K2_NEW_ARRAY(ctx_World, byte, iSize);

		if (file.Read((char*)m_pTexelAlpha, iSize) < iSize)
			EX_ERROR(_T("Truncated ColorMap"));

		m_bChanged = false;
	}
	catch (CException &ex)
	{
		if (m_pTexelAlpha != NULL)
			K2_DELETE_ARRAY(m_pTexelAlpha);
		m_pTexelAlpha = NULL;

		ex.Process(_T("CTexelAlphaMap::Load() - "), NO_THROW);
		return false;
	}

	return true;
}


/*====================
  CTexelAlphaMap::Generate
  ====================*/
bool	CTexelAlphaMap::Generate(const CWorld *pWorld)
{
	PROFILE("CTexelAlphaMap::Generate");

	try
	{
		Release();

		m_bChanged = true;
		m_pWorld = pWorld;
		if (m_pWorld == NULL)
			EX_ERROR(_T("CTexelAlphaMap needs a valid CWorld"));

		m_pTexelAlpha = K2_NEW_ARRAY(ctx_World, byte, m_pWorld->GetTexelArea());
		if (m_pTexelAlpha == NULL)
			EX_ERROR(_T("Failed to allocate memory for ColorMap"));

		for (int i(0); i < m_pWorld->GetTexelArea(); ++i)
			m_pTexelAlpha[i] = 255;

		return true;
	}
	catch (CException &ex)
	{
		Release();
		ex.Process(_T("CTexelAlphaMap::Generate() - "));
		return false;
	}
}


/*====================
  CTexelAlphaMap::Serialize
  ====================*/
bool	CTexelAlphaMap::Serialize(IBuffer *pBuffer)
{
	pBuffer->Clear();
	(*pBuffer) << m_pWorld->GetTexelWidth() << m_pWorld->GetTexelHeight();
	pBuffer->Append(m_pTexelAlpha, m_pWorld->GetTexelArea() * sizeof(byte));

	if (!pBuffer->GetFaults())
		return true;
	else
		return false;
}


/*====================
  CTexelAlphaMap::GetRegion
  ====================*/
bool	CTexelAlphaMap::GetRegion(const CRecti &recArea, void *pDest, int iLayer) const
{
	assert(recArea.IsNormalized());
	assert(m_pWorld->IsInBounds(recArea.left, recArea.top, TEXEL_SPACE));
	assert(m_pWorld->IsInBounds(recArea.right - 1, recArea.bottom - 1, TEXEL_SPACE));

	byte *pAlphaDest(static_cast<byte *>(pDest));
	byte *pAlphaSource(&m_pTexelAlpha[m_pWorld->GetTexelIndex(recArea.left, recArea.top)]);

	for (int y(0); y < recArea.GetHeight(); ++y, pAlphaDest += recArea.GetWidth(), pAlphaSource += m_pWorld->GetTexelWidth())
		MemManager.Copy(pAlphaDest, pAlphaSource, sizeof(byte) * recArea.GetWidth());

	return true;
}


/*====================
  CTexelAlphaMap::SetRegion
  ====================*/
bool	CTexelAlphaMap::SetRegion(const CRecti &recArea, void *pSource, int iLayer)
{
	assert(m_pWorld->IsInBounds(recArea.left, recArea.top, TEXEL_SPACE));
	assert(m_pWorld->IsInBounds(recArea.right - 1, recArea.bottom - 1, TEXEL_SPACE));

	byte *pAlphaSource(static_cast<byte *>(pSource));
	byte *pAlphaDest(&m_pTexelAlpha[m_pWorld->GetTexelIndex(recArea.left, recArea.top)]);

	for (int y(0); y < recArea.GetHeight(); ++y, pAlphaSource += recArea.GetWidth(), pAlphaDest += m_pWorld->GetTexelWidth())
		MemManager.Copy(pAlphaDest, pAlphaSource, sizeof(byte) * recArea.GetWidth());

	m_bChanged = true;
	return true;
}


/*====================
  CTexelAlphaMap::GetTexelAlpha
  ====================*/
byte	CTexelAlphaMap::GetTexelAlpha(int iX, int iY)
{
	assert(iX >= 0 && iX < m_pWorld->GetTexelWidth());
	assert(iY >= 0 && iY < m_pWorld->GetTexelHeight());
	return m_pTexelAlpha[m_pWorld->GetTexelIndex(iX, iY)];
}


/*====================
  CTexelAlphaMap::SetAlpha
  ====================*/
void	CTexelAlphaMap::SetAlpha(int iX, int iY, byte yAlpha)
{
	assert(iX >= 0 && iX < m_pWorld->GetTexelWidth());
	assert(iY >= 0 && iY < m_pWorld->GetTexelHeight());
	m_pTexelAlpha[m_pWorld->GetTexelIndex(iX, iY)] = yAlpha;
}
