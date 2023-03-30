// (C)2005 S2 Games
// c_filearchive.cpp
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_filearchive.h"
//=============================================================================

/*====================
  CFileArchive::~CFileArchive
  ====================*/
CFileArchive::~CFileArchive()
{
	Close();
}


/*====================
  CFileArchive::CFileArchive
  ====================*/
CFileArchive::CFileArchive(CArchive *pArchive) :
m_pArchive(pArchive),
m_tModTime(0)
{
}


/*====================
  CFileArchive::Open
  ====================*/
bool	CFileArchive::Open(const tstring &sPath, int iMode)
{
	if (m_pArchive == NULL)
		return false;

	m_sPath = sPath;
	m_iMode = iMode;

	if (iMode & FILE_TRUNCATE)
	{
		m_uiSize = 0;
	}
	else
	{
		m_uiSize = m_pArchive->ReadFile(sPath, m_pBuffer);

		if (iMode & FILE_WRITE)
			m_bufferWrite.Write(m_pBuffer, m_uiSize);
	}

	m_uiPos = 0;
	m_bEOF = false;

	if (m_pBuffer != NULL || iMode & FILE_WRITE)
		return true;
	else
		return false;
}


/*====================
  CFileArchive::Close
  ====================*/
void	CFileArchive::Close()
{
	SAFE_DELETE_ARRAY(m_pBuffer);
	
	if (m_iMode & FILE_WRITE && m_pArchive != NULL)
	{
		m_pArchive->WriteFile(m_sPath, m_bufferWrite.Get(), m_bufferWrite.GetLength(), m_iMode, m_tModTime);
		m_bufferWrite.Clear();
		m_pArchive = NULL;
	}
}


/*====================
  CFileArchive::IsOpen
  ====================*/
bool	CFileArchive::IsOpen() const
{
	if (m_pArchive == NULL)
		return false;

	if (m_iMode & FILE_WRITE)
		return m_pArchive->IsOpen();
	else
		return m_pBuffer != NULL;
}


/*====================
  CFileArchive::ReadLine
  ====================*/
size_t	CFileArchive::ReadLine(char* pBuffer, int iBufferSize)
{
	if (m_iMode & FILE_WRITE)
	{
		Console.Warn << _T("Cannot read from WRITE file ") << m_sPath << newl;
		return 0;
	}

	if (m_iMode & FILE_BINARY)
	{
		Console.Warn << _T("Cannot ReadLine from BINARY file ") << m_sPath << newl;
		return 0;
	}

	if (iBufferSize < 2)
		return 0;

	if (m_bEOF)
		return 0;

	int iIndex(0);
	do
	{
		pBuffer[iIndex] = *(m_pBuffer + m_uiPos);

		// look for any line break character
		if (pBuffer[iIndex] == '\n' || pBuffer[iIndex] == '\r')
		{
			char stopAt = pBuffer[iIndex];

			pBuffer[iIndex] = 0;

			// move the position marker to the start of the next line
			// we want to any consecutivce line break characters until we reach
			// more text, or another duplicate of the character that initiated
			// the break (a blank line).  This should handle any conceivable
			// text file format
			do
			{
				++m_uiPos;
			}
			while ((*(m_pBuffer + m_uiPos) == '\n' || *(m_pBuffer + m_uiPos) == '\r') && *(m_pBuffer + m_uiPos) != stopAt);
			break;
		}

		++iIndex;
		++m_uiPos;

		if (m_uiPos == m_uiSize)
		{
			pBuffer[iIndex] = 0;
			m_bEOF = true;
			break;
		}
	}
	while (iIndex < iBufferSize - 1);

	return iIndex;
}

tstring	CFileArchive::ReadLine()
{
	if (m_iMode & FILE_WRITE)
	{
		Console.Warn << _T("Cannot read from WRITE file ") << m_sPath << newl;
		return 0;
	}

	if (m_iMode & FILE_BINARY)
	{
		Console.Warn << _T("Cannot ReadLine from BINARY file ") << m_sPath << newl;
		return 0;
	}

	if (m_bEOF)
		return 0;

	tstring sReturn;
	do
	{
		TCHAR c = *(m_pBuffer + m_uiPos);

		// look for any line break character
		if (c == '\n' || c == '\r')
		{
			// move the position marker to the start of the next line
			// we want to any consecutivce line break characters until we reach
			// more text, or another duplicate of the character that initiated
			// the break (a blank line).  This should handle any conceivable
			// text file format
			do
			{
				++m_uiPos;
			}
			while ((*(m_pBuffer + m_uiPos) == '\n' || *(m_pBuffer + m_uiPos) == '\r') && *(m_pBuffer + m_uiPos) != c && m_uiPos < m_uiSize);
			break;
		}
		else
		{
			sReturn += c;
		}

		++m_uiPos;

		if (m_uiPos >= m_uiSize)
		{
			m_bEOF = true;
			break;
		}
	}
	while (true);

	return sReturn;
}


/*====================
  CFileArchive::WriteString
  ====================*/
bool	CFileArchive::WriteString(const string &sText)
{
	if (m_iMode & FILE_READ)
	{
		Console.Warn << _T("Cannot write to a READ file ") << m_sPath << newl;
		return 0;
	}

	m_bufferWrite << sText << newl;
	return false;
}

bool	CFileArchive::WriteString(const wstring &sText)
{
	if (m_iMode & FILE_READ)
	{
		Console.Warn << _T("Cannot write to a READ file ") << m_sPath << newl;
		return 0;
	}

	m_bufferWrite << sText << newl;
	return false;
}


/*====================
  CFileArchive::Read
  ====================*/
uint	CFileArchive::Read(char* pBuffer, uint uiBufferSize) const
{
	assert(pBuffer != NULL);

	if (m_iMode & FILE_WRITE)
	{
		Console.Warn << _T("Cannot read from WRITE file ") << m_sPath << newl;
		return 0;
	}

	if (uiBufferSize > (m_uiSize - m_uiPos))
	{
		uiBufferSize = m_uiSize - m_uiPos;
		m_bEOF = true;
	}

	MemManager.Copy(pBuffer, m_pBuffer + m_uiPos, uiBufferSize);
	m_uiPos += uiBufferSize;

	return uiBufferSize;
}


/*====================
  CFileArchive::Write
  ====================*/
size_t	CFileArchive::Write(const void* pBuffer, size_t zBufferSize)
{
	if (!(m_iMode & FILE_WRITE))
		return 0;

	if (!m_bufferWrite.Append(pBuffer, INT_SIZE(zBufferSize)))
		return 0;

	return zBufferSize;
}


/*====================
  CFileArchive::GetBuffer
  ====================*/
const char	*CFileArchive::GetBuffer(uint &uiSize)
{
	if (m_iMode & FILE_WRITE)
	{
		uiSize = 0;
		return NULL;
	}

	if (m_pBuffer == NULL)
	{
		uiSize = 0;
		return NULL;
	}
	else
	{
		uiSize = m_uiSize;
		return m_pBuffer;
	}
}


/*====================
  CFileArchive::WriteByte
  ====================*/
bool	CFileArchive::WriteByte(char c)
{
	m_bufferWrite << c;
	return m_bufferWrite.GetFaults() == 0;
}


/*====================
  CFileArchive::WriteInt16
  ====================*/
bool	CFileArchive::WriteInt16(short c, bool bUseBigEndian)
{
	m_bufferWrite << (bUseBigEndian ? char((c >> 8) & 0xff) : char(c & 0xff));
	m_bufferWrite << (bUseBigEndian ? char(c & 0xff) : char((c >> 8) & 0xff));
	return m_bufferWrite.GetFaults() == 0;
}


/*====================
  CFileArchive::WriteInt32
  ====================*/
bool	CFileArchive::WriteInt32(int c, bool bUseBigEndian)
{
	m_bufferWrite << (bUseBigEndian ? char((c >> 24) & 0xff) : char((c >> 0) & 0xff));
	m_bufferWrite << (bUseBigEndian ? char((c >> 16) & 0xff) : char((c >> 8) & 0xff));
	m_bufferWrite << (bUseBigEndian ? char((c >> 8) & 0xff) : char((c >> 16) & 0xff));
	m_bufferWrite << (bUseBigEndian ? char((c >> 0) & 0xff) : char((c >> 24) & 0xff));
	return m_bufferWrite.GetFaults() == 0;
}


/*====================
  CFileArchive::WriteInt64
  ====================*/
bool	CFileArchive::WriteInt64(LONGLONG c, bool bUseBigEndian)
{
	m_bufferWrite << (bUseBigEndian ? char((c >> 56) & 0xff) : char((c >> 0) & 0xff));
	m_bufferWrite << (bUseBigEndian ? char((c >> 48) & 0xff) : char((c >> 8) & 0xff));
	m_bufferWrite << (bUseBigEndian ? char((c >> 40) & 0xff) : char((c >> 16) & 0xff));
	m_bufferWrite << (bUseBigEndian ? char((c >> 32) & 0xff) : char((c >> 24) & 0xff));
	m_bufferWrite << (bUseBigEndian ? char((c >> 24) & 0xff) : char((c >> 32) & 0xff));
	m_bufferWrite << (bUseBigEndian ? char((c >> 16) & 0xff) : char((c >> 40) & 0xff));
	m_bufferWrite << (bUseBigEndian ? char((c >> 8) & 0xff) : char((c >> 48) & 0xff));
	m_bufferWrite << (bUseBigEndian ? char((c >> 0) & 0xff) : char((c >> 56) & 0xff));
	return m_bufferWrite.GetFaults() == 0;
}
