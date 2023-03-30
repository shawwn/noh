// (C)2010 S2 Games
// c_filebuffer.cpp
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_filebuffer.h"
//=============================================================================

/*====================
  CFileBuffer::~CFileBuffer
  ====================*/
CFileBuffer::~CFileBuffer()
{
	Close();
}


/*====================
  CFileBuffer::CFileBuffer
  ====================*/
CFileBuffer::CFileBuffer()
{
}


/*====================
  CFileBuffer::Open
  ====================*/
bool	CFileBuffer::Open(const tstring &sPath, int iMode)
{
	m_sPath = sPath;
	m_iMode = iMode;

	if (iMode & FILE_TRUNCATE)
	{
		m_uiSize = 0;
	}
	else
	{
		//m_uiSize = m_pArchive->ReadFile(sPath, m_pBuffer);
		m_uiSize = 0;
		m_pBuffer = NULL;

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
  CFileBuffer::Close
  ====================*/
void	CFileBuffer::Close()
{
	SAFE_DELETE_ARRAY(m_pBuffer);
}


/*====================
  CFileBuffer::IsOpen
  ====================*/
bool	CFileBuffer::IsOpen() const
{
	return true;
}


/*====================
  CFileBuffer::ReadLine
  ====================*/
size_t	CFileBuffer::ReadLine(char* pBuffer, int iBufferSize)
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

tstring	CFileBuffer::ReadLine()
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
  CFileBuffer::WriteString
  ====================*/
bool	CFileBuffer::WriteString(const string &sText)
{
	if (m_iMode & FILE_READ)
	{
		Console.Warn << _T("Cannot write to a READ file ") << m_sPath << newl;
		return 0;
	}

	m_bufferWrite << sText << newl;
	return false;
}

bool	CFileBuffer::WriteString(const wstring &sText)
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
  CFileBuffer::Read
  ====================*/
uint	CFileBuffer::Read(char* pBuffer, uint uiBufferSize) const
{
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
  CFileBuffer::Write
  ====================*/
size_t	CFileBuffer::Write(const void* pBuffer, size_t zBufferSize)
{
	if (!(m_iMode & FILE_WRITE))
		return 0;

	if (!m_bufferWrite.Append(pBuffer, INT_SIZE(zBufferSize)))
		return 0;

	return zBufferSize;
}


/*====================
  CFileBuffer::GetBuffer
  ====================*/
const char	*CFileBuffer::GetBuffer(uint &uiSize)
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
  CFileBuffer::WriteByte
  ====================*/
bool	CFileBuffer::WriteByte(char c)
{
	m_bufferWrite << c;
	return m_bufferWrite.GetFaults() == 0;
}


/*====================
  CFileBuffer::WriteInt16
  ====================*/
bool	CFileBuffer::WriteInt16(short c, bool bUseBigEndian)
{
	m_bufferWrite << (bUseBigEndian ? char((c >> 8) & 0xff) : char(c & 0xff));
	m_bufferWrite << (bUseBigEndian ? char(c & 0xff) : char((c >> 8) & 0xff));
	return m_bufferWrite.GetFaults() == 0;
}


/*====================
  CFileBuffer::WriteInt32
  ====================*/
bool	CFileBuffer::WriteInt32(int c, bool bUseBigEndian)
{
	m_bufferWrite << (bUseBigEndian ? char((c >> 24) & 0xff) : char((c >> 0) & 0xff));
	m_bufferWrite << (bUseBigEndian ? char((c >> 16) & 0xff) : char((c >> 8) & 0xff));
	m_bufferWrite << (bUseBigEndian ? char((c >> 8) & 0xff) : char((c >> 16) & 0xff));
	m_bufferWrite << (bUseBigEndian ? char((c >> 0) & 0xff) : char((c >> 24) & 0xff));
	return m_bufferWrite.GetFaults() == 0;
}


/*====================
  CFileBuffer::WriteInt64
  ====================*/
bool	CFileBuffer::WriteInt64(LONGLONG c, bool bUseBigEndian)
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
