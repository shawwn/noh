// (C)2007 S2 Games
// c_texelocclusionmap.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_texelocclusionmap.h"
#include "c_world.h"
#include "c_buffer.h"
#include "s_traceinfo.h"
//=============================================================================

/*====================
  CTexelOcclusionMap::CTexelOcclusionMap
  ====================*/
CTexelOcclusionMap::CTexelOcclusionMap(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("TexelOcclusionMap")),
m_bActive(false),
m_pTexelOcclusion(NULL)
{
}


/*====================
  CTexelOcclusionMap::~CTexelOcclusionMap
  ====================*/
CTexelOcclusionMap::~CTexelOcclusionMap()
{
	Release();
}


/*====================
  CTexelOcclusionMap::Release
  ====================*/
void	CTexelOcclusionMap::Release()
{
	m_pWorld = NULL;
	m_bActive = false;

	if (m_pTexelOcclusion != NULL)
		K2_DELETE_ARRAY(m_pTexelOcclusion);
	m_pTexelOcclusion = NULL;
}


/*====================
  CTexelOcclusionMap::Load
  ====================*/
bool	CTexelOcclusionMap::Load(CArchive &archive, const CWorld *pWorld)
{
	try
	{
		Release();
		m_pWorld = pWorld;

		CFileHandle	file(m_sName, FILE_READ | FILE_BINARY, archive);
		if (!file.IsOpen())
			EX_ERROR(_T("No OcclusionMap found in archive"));

		// Read the dimensions
		int iWidth = file.ReadInt32();
		int iHeight = file.ReadInt32();

		if (iWidth != m_pWorld->GetTexelWidth() || iHeight != m_pWorld->GetTexelHeight())
			EX_ERROR(_T("Invalid OcclusionMap dimensions"));

		if (file.IsEOF())
			EX_ERROR(_T("Truncated OcclusionMap"));

		m_bActive = false;

		// Read the OcclusionMap
		int iSize(m_pWorld->GetTexelArea());
		m_pTexelOcclusion = K2_NEW_ARRAY(ctx_World, byte, iSize);

		if (file.Read((char*)m_pTexelOcclusion, iSize) < iSize)
			EX_ERROR(_T("Truncated OcclusionMap"));

		m_bChanged = false;
	}
	catch (CException &ex)
	{
		if (m_pTexelOcclusion != NULL)
			K2_DELETE_ARRAY(m_pTexelOcclusion);
		m_pTexelOcclusion = NULL;

		ex.Process(_T("CTexelOcclusionMap::Load() - "), NO_THROW);
		return false;
	}

	return true;
}


/*====================
  CTexelOcclusionMap::Generate
  ====================*/
bool	CTexelOcclusionMap::Generate(const CWorld *pWorld)
{
	PROFILE("CTexelOcclusionMap::Generate");

	try
	{
		Release();

		m_bActive = false;
		m_bChanged = true;
		
		m_pWorld = pWorld;
		if (m_pWorld == NULL)
			EX_ERROR(_T("CTexelOcclusionMap needs a valid CWorld"));

		

		return true;
	}
	catch (CException &ex)
	{
		Release();
		ex.Process(_T("CTexelOcclusionMap::Generate() - "));
		return false;
	}
}


/*====================
  CTexelOcclusionMap::Serialize
  ====================*/
bool	CTexelOcclusionMap::Serialize(IBuffer *pBuffer)
{
	pBuffer->Clear();
	(*pBuffer) << m_pWorld->GetTexelWidth() << m_pWorld->GetTexelHeight();
	pBuffer->Append(m_pTexelOcclusion, m_pWorld->GetTexelArea() * sizeof(byte));

	if (!pBuffer->GetFaults())
		return true;
	else
		return false;
}


/*====================
  CTexelOcclusionMap::GetRegion
  ====================*/
bool	CTexelOcclusionMap::GetRegion(const CRecti &recArea, void *pDest, int iLayer) const
{
	assert(recArea.IsNormalized());
	assert(m_pWorld->IsInBounds(recArea.left, recArea.top, TEXEL_SPACE));
	assert(m_pWorld->IsInBounds(recArea.right - 1, recArea.bottom - 1, TEXEL_SPACE));

	byte *pAlphaDest(static_cast<byte *>(pDest));
	byte *pAlphaSource(&m_pTexelOcclusion[m_pWorld->GetTexelIndex(recArea.left, recArea.top)]);

	for (int y(0); y < recArea.GetHeight(); ++y, pAlphaDest += recArea.GetWidth(), pAlphaSource += m_pWorld->GetTexelWidth())
		MemManager.Copy(pAlphaDest, pAlphaSource, sizeof(byte) * recArea.GetWidth());

	return true;
}


/*====================
  CTexelOcclusionMap::SetRegion
  ====================*/
bool	CTexelOcclusionMap::SetRegion(const CRecti &recArea, void *pSource, int iLayer)
{
	assert(m_pWorld->IsInBounds(recArea.left, recArea.top, TEXEL_SPACE));
	assert(m_pWorld->IsInBounds(recArea.right - 1, recArea.bottom - 1, TEXEL_SPACE));

	byte *pAlphaSource(static_cast<byte *>(pSource));
	byte *pAlphaDest(&m_pTexelOcclusion[m_pWorld->GetTexelIndex(recArea.left, recArea.top)]);

	for (int y(0); y < recArea.GetHeight(); ++y, pAlphaSource += recArea.GetWidth(), pAlphaDest += m_pWorld->GetTexelWidth())
		MemManager.Copy(pAlphaDest, pAlphaSource, sizeof(byte) * recArea.GetWidth());

	m_bChanged = true;
	return true;
}


/*====================
  CTexelOcclusionMap::GetTexelOcclusion
  ====================*/
byte	CTexelOcclusionMap::GetTexelOcclusion(int iX, int iY)
{
	assert(iX >= 0 && iX < m_pWorld->GetTexelWidth());
	assert(iY >= 0 && iY < m_pWorld->GetTexelHeight());
	return m_pTexelOcclusion[m_pWorld->GetTexelIndex(iX, iY)];
}


/*====================
  CTexelOcclusionMap::SetOcclusion
  ====================*/
void	CTexelOcclusionMap::SetOcclusion(int iX, int iY, byte yAlpha)
{
	assert(iX >= 0 && iX < m_pWorld->GetTexelWidth());
	assert(iY >= 0 && iY < m_pWorld->GetTexelHeight());
	m_pTexelOcclusion[m_pWorld->GetTexelIndex(iX, iY)] = yAlpha;
}


/*====================
  CTexelOcclusionMap::Calculate
  ====================*/
void	CTexelOcclusionMap::Calculate(int iSamples)
{
	m_pTexelOcclusion = K2_NEW_ARRAY(ctx_World, byte, m_pWorld->GetTexelArea());
	if (m_pTexelOcclusion == NULL)
		return;

	float fTexelSize(m_pWorld->GetScale() / m_pWorld->GetTexelDensity());

	for (int iY(0); iY < m_pWorld->GetTexelHeight(); ++iY)
	{
		for (int iX(0); iX < m_pWorld->GetTexelWidth(); ++iX)
		{
			CVec3f v3Texel((iX + 0.5f) * fTexelSize, (iY + 0.5f) * fTexelSize, 0.0f);
			v3Texel.z = m_pWorld->GetTerrainHeight(v3Texel.x, v3Texel.y) + 1.0f;

			int iVisible(0);

			for (int i(0); i < iSamples; ++i)
			{
				CVec3f v3Dir(M_RandomDirection(V_UP, 90.0f));

				STraceInfo cTrace;

				if (!m_pWorld->GetWorldTree().TraceLine(cTrace, v3Texel, v3Texel + v3Dir * FAR_AWAY, TRACE_TERRAIN, uint(-1)))
					++iVisible;
			}

			m_pTexelOcclusion[m_pWorld->GetTexelIndex(iX, iY)] = BYTE_ROUND(float(iVisible) / iSamples * 255.0f);
		}
	}

	m_bActive = true;
}
