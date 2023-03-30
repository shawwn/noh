// (C)2005 S2 Games
// c_buffer.cpp
//
// A general purpose buffer class and friends
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_buffer.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

/*====================
  IBuffer::~IBuffer
  ====================*/
IBuffer::~IBuffer()
{
	SAFE_DELETE_ARRAY(m_pBuffer);
}


/*====================
  IBuffer::IBuffer
  ====================*/
IBuffer::IBuffer() :
m_pBuffer(NULL),
m_uiSize(0),
m_uiEnd(0),
m_uiRead(0),
m_iFaults(0)
{
}

IBuffer::IBuffer(const IBuffer &buffer) :
m_pBuffer(NULL),
m_uiSize(buffer.m_uiSize),
m_uiEnd(buffer.m_uiEnd),
m_uiRead(buffer.m_uiRead),
m_iFaults(buffer.m_iFaults)
{
	if (m_pBuffer)
		K2_DELETE_ARRAY(m_pBuffer);

	if (buffer.m_pBuffer)
	{
		m_pBuffer = K2_NEW_ARRAY(ctx_Buffers, char, buffer.m_uiSize);
		if (m_pBuffer == NULL)
			m_iFaults |= BUFFER_FAULT_COPY | BUFFER_FAULT_ALLOCATE;
		MemManager.Copy(m_pBuffer, buffer.m_pBuffer, buffer.m_uiSize);
	}
}


/*====================
  IBuffer::Init
  ====================*/
void	IBuffer::Init(uint uiSize)
{
	SAFE_DELETE_ARRAY(m_pBuffer);

	if (uiSize > 0)
	{
		m_pBuffer = K2_NEW_ARRAY(ctx_Buffers, char, uiSize);
		
		if (!m_pBuffer)
			m_iFaults |= BUFFER_FAULT_INITIALIZE | BUFFER_FAULT_ALLOCATE;
		else
			m_uiSize = uiSize;
	}
	else
	{
		m_pBuffer = NULL;
		m_uiSize = 0;
	}
	
	m_uiRead = 0;
}


/*====================
  IBuffer::Resize
  ====================*/
bool	IBuffer::Resize(uint uiSize)
{
	if (uiSize == m_uiSize)
		return true;

	if (uiSize > 0)
	{
		char *pTemp = K2_NEW_ARRAY(ctx_Buffers, char, uiSize);
		if (!pTemp)
		{
			m_iFaults |= BUFFER_FAULT_ALLOCATE;
			return false;
		}

		// copy the old contents, if there is any
		uint uiCopyLen(0);
		if (m_uiEnd > 0)
		{
			uiCopyLen = (uiSize < m_uiEnd) ? uiSize : m_uiEnd;
			MemManager.Copy(pTemp, m_pBuffer, uiCopyLen);
		}

		m_uiEnd = uiCopyLen;

		if (m_pBuffer)
			K2_DELETE_ARRAY(m_pBuffer);

		m_pBuffer = pTemp;
		m_uiSize = uiSize;
	}
	else
	{
		m_uiRead = 0;
		m_uiEnd = 0;

		if (m_pBuffer)
			K2_DELETE_ARRAY(m_pBuffer);

		m_pBuffer = NULL;
		m_uiSize = 0;
	}
	
	return true;
}


/*====================
  IBuffer::Reserve
  ====================*/
bool	IBuffer::Reserve(uint uiSize)
{
	if (uiSize <= m_uiSize)
		return true;

	char *pTemp = K2_NEW_ARRAY(ctx_Buffers, char, uiSize);
	if (!pTemp)
	{
		m_iFaults |= BUFFER_FAULT_ALLOCATE;
		return false;
	}

	// copy the old contents, if there is any
	uint uiCopyLen(0);
	if (m_uiEnd > 0)
	{
		uiCopyLen = (uiSize < m_uiEnd) ? uiSize : m_uiEnd;
		MemManager.Copy(pTemp, m_pBuffer, uiCopyLen);
	}
	if (m_pBuffer)
		K2_DELETE_ARRAY(m_pBuffer);

	m_pBuffer = pTemp;
	m_uiSize = uiSize;
	m_uiEnd = uiCopyLen;
	return true;
}


/*====================
  IBuffer::Reallocate
  ====================*/
bool	IBuffer::Reallocate(uint uiSize)
{
	if (uiSize == m_uiSize)
		return true;

	if (uiSize > 0)
	{
		char *pTemp = K2_NEW_ARRAY(ctx_Buffers, char, uiSize);
		if (!pTemp)
		{
			m_iFaults |= BUFFER_FAULT_ALLOCATE;
			return false;
		}

		m_uiEnd = 0;
		m_uiRead = 0;

		if (m_pBuffer)
			K2_DELETE_ARRAY(m_pBuffer);

		m_pBuffer = pTemp;
		m_uiSize = uiSize;
	}
	else
	{
		m_uiRead = 0;
		m_uiEnd = 0;

		if (m_pBuffer)
			K2_DELETE_ARRAY(m_pBuffer);

		m_pBuffer = NULL;
		m_uiSize = 0;
	}
	
	return true;
}


/*====================
  IBuffer::FindNext
  ====================*/
uint	IBuffer::FindNext(char c) const
{
	for (uint uiSeek(m_uiRead); uiSeek < m_uiEnd; ++uiSeek)
	{
		if (m_pBuffer[uiSeek] == c)
			return uiSeek;
	}

	return INVALID_INDEX;
}

uint	IBuffer::FindNext(wchar_t c) const
{
	for (uint uiSeek(m_uiRead); uiSeek < (m_uiEnd - 1); uiSeek += 2)
	{
		if (*(wchar_t*)(&m_pBuffer[uiSeek]) == c)
			return uiSeek;
	}

	return INVALID_INDEX;
}


/*====================
  IBuffer::ReadString
  ====================*/
string	IBuffer::ReadString() const
{
	char *sz(NULL);

	try
	{
		uint uiPos(FindNext('\x00'));
		if (uiPos == INVALID_INDEX)
			EX_ERROR(_T("Attempted to read unterminated string"));

		uint uiRead(uiPos - GetReadPos() + 1);
		sz = K2_NEW_ARRAY(ctx_Buffers, char, uiRead);
		if (sz == NULL)
			EX_ERROR(_T("Failed to allocate temporary string buffer"));

		Read(sz, uiRead);
		if (GetFaults() & BUFFER_FAULT_UNDERRUN)
			EX_ERROR(_T("Attempted to read past end of buffer"));

		string sReturn(sz);
		K2_DELETE_ARRAY(sz);
		return sReturn;
	}
	catch (CException &ex)
	{
		if (sz != NULL)
			K2_DELETE_ARRAY(sz);

		ex.Process(_T("IBuffer::ReadString() - "), NO_THROW);
		return "";
	}
}


/*====================
  IBuffer::ReadWString
  ====================*/
wstring	IBuffer::ReadWString() const
{
	wchar_t *sz(NULL);

	try
	{
		uint uiPos(FindNext(L'\x0000'));
		if (uiPos == INVALID_INDEX)
			EX_ERROR(_T("Attempted to read unterminated string"));

		uint uiRead((uiPos - GetReadPos() + sizeof(wchar_t)));
		sz = K2_NEW_ARRAY(ctx_Buffers, wchar_t, uiRead);
		if (sz == NULL)
			EX_ERROR(_T("Failed to allocate temporary string buffer"));

		Read(sz, uiRead);
		if (GetFaults() & BUFFER_FAULT_UNDERRUN)
			EX_ERROR(_T("Attempted to read past end of packet"));

		wstring sReturn(sz);
		K2_DELETE_ARRAY(sz);
		return sReturn;
	}
	catch (CException &ex)
	{
		if (sz != NULL)
			K2_DELETE_ARRAY(sz);

		ex.Process(_T("IBuffer::ReadString() - "), NO_THROW);
		return L"";
	}
}

/*====================
  IBuffer::ReadTString
  ====================*/
tstring	IBuffer::ReadTString() const
{
	char *sz(NULL);

	try
	{
		uint uiPos(FindNext('\x00'));
		if (uiPos == INVALID_INDEX)
			EX_ERROR(_T("Attempted to read unterminated string"));

		uint uiRead(uiPos - GetReadPos() + 1);
		sz = K2_NEW_ARRAY(ctx_Buffers, char, uiRead);
		if (sz == NULL)
			EX_ERROR(_T("Failed to allocate temporary string buffer"));

		Read(sz, uiRead);
		if (GetFaults() & BUFFER_FAULT_UNDERRUN)
			EX_ERROR(_T("Attempted to read past end of buffer"));

		tstring sReturn(UTF8ToTString(sz));
		K2_DELETE_ARRAY(sz);
		return sReturn;
	}
	catch (CException &ex)
	{
		if (sz != NULL)
			K2_DELETE_ARRAY(sz);

		ex.Process(_T("IBuffer::ReadString() - "), NO_THROW);
		return TSNULL;
	}
}


/*====================
  CBufferStatic::Write

  Copy the input to the start of the buffer, overwriting
  ====================*/
bool	CBufferStatic::Write(const void *pBuffer, uint uiSize)
{
	bool ret(true);

	if (pBuffer == NULL)
		return false;

	uint uiCopyLen(uiSize);
	if (uiSize > m_uiSize)
	{
		uiCopyLen = m_uiSize;
		m_iFaults |= BUFFER_FAULT_OVERRUN;
		ret = false;
	}

	MemManager.Copy(m_pBuffer, pBuffer, uiCopyLen);
	m_uiEnd = uiCopyLen;
	m_uiRead = 0;
	return ret;
}


/*====================
  CBufferStatic::Append

  Copy the input to the end of the buffer 
  ====================*/
bool	CBufferStatic::Append(const void *pBuffer, uint uiSize)
{
	bool ret(true);

	uint uiCopyLen(uiSize);
	if (uiSize > (m_uiSize - m_uiEnd))
	{
		uiCopyLen = m_uiSize - m_uiEnd;
		m_iFaults |= BUFFER_FAULT_OVERRUN;
		ret = false;
	}

	MemManager.Copy(&m_pBuffer[m_uiEnd], pBuffer, uiCopyLen);
	m_uiEnd += uiCopyLen;
	return ret;
}


/*====================
  CBufferStatic::Lock

  return a writable region of the buffer from the current write position
  ====================*/
char*	CBufferStatic::Lock(uint uiSize)
{
	if (uiSize > (m_uiSize - m_uiEnd))
	{
		uiSize = m_uiSize - m_uiEnd;
		m_iFaults |= BUFFER_FAULT_OVERRUN;
	}

	if (uiSize == 0)
		return NULL;

	char *pBuffer(&m_pBuffer[m_uiEnd]);
	m_uiEnd += uiSize;
	
	return pBuffer;
}


/*====================
  CBufferStatic::Seek
  ====================*/
bool	CBufferStatic::Seek(uint uiPos)
{
	if (uiPos <= m_uiEnd)
	{
		m_uiRead = uiPos;
		return true;
	}

	return false;
}


/*====================
  CBufferStatic::Read
  ====================*/
bool	CBufferStatic::Read(void *pBuffer, uint uiSize) const
{
	if (m_uiRead + uiSize > m_uiSize)
	{
		m_iFaults |= BUFFER_FAULT_UNDERRUN;
		return false;
	}

	MemManager.Copy(pBuffer, &m_pBuffer[m_uiRead], uiSize);
	m_uiRead += uiSize;
	return true;
}


/*====================
  CBufferStatic::Advance
  ====================*/
bool	CBufferStatic::Advance(uint uiSize) const
{
	if (uiSize == 0)
		return true;

	bool ret(true);

	if (m_uiRead + uiSize > m_uiEnd)
	{
		m_iFaults |= BUFFER_FAULT_OVERRUN;
		Console.Err << _T("IBuffer::Advance - Overrun") << newl;
		ret = false;
	}
	else
	{
		m_uiRead += uiSize;
	}

	return ret;
}


/*====================
  CBufferDynamic::Write
  ====================*/
bool	CBufferDynamic::Write(const void *pBuffer, uint uiSize)
{
	try
	{
		if (uiSize == 0)
		{
			m_uiRead = 0;
			m_uiEnd = 0;
			return true;
		}

		bool ret(true);

		if (pBuffer == NULL)
			EX_ERROR(_T("Invalid source"));

		uint uiCopyLen(uiSize);
		if (uiSize > m_uiSize)
		{
			uint uiNewSize(MAX(m_uiSize, 1u));
			while (uiNewSize < uiSize)
				uiNewSize <<= 1;
			if (!Resize(uiNewSize))
			{
				m_iFaults |= BUFFER_FAULT_OVERRUN;
				uiCopyLen = m_uiSize;
				ret = false;
			}
		}

		MemManager.Copy(m_pBuffer, pBuffer, uiCopyLen);
		m_uiEnd = uiCopyLen;
		m_uiRead = 0;
		return ret;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CBufferDynamic::Write() - "));
		return false;
	}
}


/*====================
  CBufferDynamic::WriteBytes
  ====================*/
bool	CBufferDynamic::WriteBytes(byte yValue, uint uiSize)
{
	bool	ret(true);

	if (uiSize == 0)
		return true;

	uint uiCopyLen(uiSize);
	if (uiSize > (m_uiSize - m_uiEnd))
	{
		uint uiNewSize(MAX(m_uiSize, 1u));
		while (uiNewSize < (m_uiEnd + uiSize))
			uiNewSize <<= 1;
		if (!Resize(uiNewSize))
		{
			m_iFaults |= BUFFER_FAULT_OVERRUN;
			ret = false;
			uiCopyLen = m_uiSize - m_uiEnd;
		}
	}

	MemManager.Set(&m_pBuffer[m_uiEnd], yValue, uiCopyLen);
	m_uiEnd += uiCopyLen;
	return ret;
}


/*====================
  CBufferDynamic::Seek
  ====================*/
bool	CBufferDynamic::Seek(uint uiPos)
{
	if (uiPos < m_uiEnd)
	{
		m_uiRead = uiPos;
		return true;
	}

	return false;
}


/*====================
  CBufferDynamic::Read
  ====================*/
bool	CBufferDynamic::Read(void *pBuffer, uint uiSize) const
{
	if (m_uiRead + uiSize > m_uiEnd)
	{
		m_iFaults |= BUFFER_FAULT_UNDERRUN;
		return false;
	}

	MemManager.Copy(pBuffer, &m_pBuffer[m_uiRead], uiSize);
	m_uiRead += uiSize;
	return true;
}


/*====================
  CBufferDynamic::Overwrite
  ====================*/
bool	CBufferDynamic::Overwrite(const void *pBuffer, uint uiSize)
{
	if (uiSize == 0)
		return true;

	bool ret(true);

	if (pBuffer == NULL)
	{
		Console.Err << _T("CBufferDynamic::Overwrite - Invalid source") << newl;
		ret = false;
	}
	else
	{
		uint uiCopyLen(uiSize);
		if (m_uiRead + uiSize > m_uiEnd)
		{
			m_iFaults |= BUFFER_FAULT_OVERRUN;
			Console.Err << _T("CBufferDynamic::Overwrite - Overrun") << newl;
			ret = false;
		}
		else
		{
			MemManager.Copy(&m_pBuffer[m_uiRead], pBuffer, uiCopyLen);
			m_uiRead += uiSize;
		}
	}

	return ret;
}


#if 0 // Looks like a memory leak to me
/*====================
  CBufferDynamic::Overwrite
  ====================*/
bool	CBufferDynamic::Overwrite(tstring &sNewString)
{
	if (m_uiRead >= m_uiEnd)
		return false;

	uint readPosition(m_uiRead);

	// Advance through the buffer, past the current string
	while (ReadByte() && m_uiRead <= m_uiEnd)
		/* do nothing */;

	char *pTemp(NULL);
	uint uiRemainingBuffer(m_uiEnd - m_uiRead);
	if (uiRemainingBuffer)
	{
		pTemp = K2_NEW_ARRAY(ctx_Buffers, char, m_uiEnd-m_uiRead);
		MemManager.Copy(pTemp, &m_pBuffer[m_uiRead], uiRemainingBuffer);
	}

	m_uiEnd = readPosition;
	Append(sNewString.c_str(), sizeof(TCHAR) * INT_SIZE(sNewString.size()));
	byte terminate(0);
	Append(&terminate, sizeof(byte));
	// Store the position of the string terminator
	readPosition = m_uiEnd;
	if (pTemp)
		Append(pTemp, uiRemainingBuffer);

	// Set the read position
	m_uiRead = readPosition;

	return true;
}
#endif


/*====================
  CBufferDynamic::Lock

  return a writable region of the buffer from the current write position
  ====================*/
char*	CBufferDynamic::Lock(uint uiSize)
{
	if (uiSize > (m_uiSize - m_uiEnd))
	{
		uiSize = m_uiSize - m_uiEnd;
		m_iFaults |= BUFFER_FAULT_OVERRUN;
	}

	if (uiSize == 0)
		return NULL;

	char *pBuffer(&m_pBuffer[m_uiEnd]);
	m_uiEnd += uiSize;
	
	return pBuffer;
}


/*====================
  CBufferCircular::CBufferCircular
  ====================*/
CBufferCircular::CBufferCircular(const CBufferCircular &buffer) :
IBuffer(buffer),
m_uiLength(buffer.m_uiLength)
{
}


/*====================
  CBufferCircular::Write
  ====================*/
bool	CBufferCircular::Write(const void *pBuffer, uint uiSize)
{
	bool ret(true);

	uint uiCopyLen(uiSize);
	uint uiReadStart(0);

	// Data is larger than the size of the buffer, so we
	// just write the maximum possible portion of the data,
	// aligned to the end rather than the start
	if (uiSize > m_uiSize)
	{
		uiReadStart = uiSize - m_uiSize;
		uiCopyLen = m_uiSize;
		m_iFaults |= BUFFER_FAULT_OVERRUN;
		ret = false;
	}

	MemManager.Copy(m_pBuffer, &((const char*)pBuffer)[uiReadStart], uiCopyLen);
	m_uiLength = uiCopyLen;
	m_uiEnd = uiCopyLen;
	m_uiRead = 0;
	return ret;
}


/*====================
  CBufferCircular::Append
  ====================*/
bool	CBufferCircular::Append(const void *pBuffer, uint uiSize)
{
	// If the data is longer than the buffer, treating
	// it as a 'write' will handle the case properly
	if (uiSize > m_uiSize)
		return Write(pBuffer, uiSize);

	uint uiReadStart(0);
	uint uiTailsize(m_uiSize - m_uiEnd);

	// check for hitting the physical end of the buffer,
	// which will require two seperate writes
	if (uiSize > uiTailsize)
	{
		// Write as much of the data as possible, then set up
		// for a regular copy at the start of the buffer
		MemManager.Copy(&m_pBuffer[m_uiEnd], pBuffer, uiTailsize);
		uiReadStart = uiTailsize;
		m_uiEnd = 0;
	}

	MemManager.Copy(&m_pBuffer[m_uiEnd], &(((char*)pBuffer)[uiReadStart]), uiSize - uiReadStart);
	m_uiEnd += uiSize - uiReadStart;
	m_uiLength = MIN(m_uiSize, m_uiLength + uiSize);
	return true;
}


/*====================
  CBufferCircular::Seek
  ====================*/
bool	CBufferCircular::Seek(uint uiPos)
{
	if (uiPos < m_uiSize)
	{
		m_uiRead = uiPos;
		return true;
	}

	return false;
}


/*====================
  CBufferCircular::Rewind
  ====================*/
void	CBufferCircular::Rewind()
{
	m_uiRead = m_uiEnd + 1;
	if (m_uiRead >= m_uiSize)
		m_uiRead -= m_uiSize;
}


/*====================
  CBufferCircular::Read
  ====================*/
bool	CBufferCircular::Read(void *pBuffer, uint uiSize) const
{
	if (uiSize > m_uiSize)
	{
		m_iFaults |= BUFFER_FAULT_UNDERRUN;
		return false;
	}

	if (m_uiRead + uiSize > m_uiSize)
	{
		MemManager.Copy(pBuffer, &m_pBuffer[m_uiRead], m_uiSize - m_uiRead);
		m_uiRead = 0;
		uiSize -= (m_uiSize - m_uiRead);
	}

	MemManager.Copy(pBuffer, &m_pBuffer[m_uiRead], uiSize);
	m_uiRead += uiSize;
	return true;
}


/*====================
  CBufferCircular::FindNext
  ====================*/
uint	CBufferCircular::FindNext(char c) const
{
	for (uint uiSeek(m_uiRead); uiSeek != m_uiEnd; ++uiSeek)
	{
		if (uiSeek == m_uiSize)
			uiSeek = 0;

		if (m_pBuffer[uiSeek] == c)
			return uiSeek;
	}

	return INVALID_INDEX;
}

uint	CBufferCircular::FindNext(wchar_t c) const
{
	for (uint uiSeek(m_uiRead); uiSeek != m_uiEnd && uiSeek != m_uiEnd - 1; uiSeek += 2)
	{
		if (uiSeek == m_uiSize - 1)
		{
			wchar_t t((m_pBuffer[uiSeek] << 8) | m_pBuffer[0]);
			if (t == c)
				return uiSeek;
			uiSeek = 1;
		}
		else if (uiSeek == m_uiSize)
		{
			uiSeek = 0;
		}

		if (*((wchar_t*)&m_pBuffer[uiSeek]) == c)
			return uiSeek;
	}

	return INVALID_INDEX;
}


/*====================
  CBufferBit::ReadBits
  ====================*/
uint	CBufferBit::ReadBits(uint uiNumBits) const
{
	uint uiValue(0);

	if (m_uiRead >= m_uiEnd)
	{
		m_iFaults |= BUFFER_FAULT_UNDERRUN;
		return 0;
	}

	// Cast sign away from data buffer
	const byte* const pBytes(reinterpret_cast<byte*>(m_pBuffer));

	if (m_uiReadBit != 0)
	{
		if (uiNumBits + m_uiReadBit > 8)
		{
			uint uiValueShift(0);

			// Read the rest of the current byte
			uiValue |= uint(pBytes[m_uiRead] >> m_uiReadBit);
			uiNumBits -= 8 - m_uiReadBit;
			uiValueShift += 8 - m_uiReadBit;

			++m_uiRead;
			m_uiReadBit = 0;

			while (uiNumBits > 8)
			{
				uiValue |= uint(pBytes[m_uiRead]) << uiValueShift;
				uiValueShift += 8;
				uiNumBits -= 8;

				++m_uiRead;
			}

			uiValue |= uint(pBytes[m_uiRead] & (byte(-1) >> (8 - uiNumBits))) << uiValueShift;
			m_uiReadBit += uiNumBits;
		}
		else
		{
			uiValue |= uint(pBytes[m_uiRead] >> m_uiReadBit) & (0xffu >> (8 - uiNumBits));
			m_uiReadBit += uiNumBits;
		}
	}
	else
	{
		uint uiValueShift(0);

		while (uiNumBits > 8)
		{
			uiValue |= uint(pBytes[m_uiRead]) << uiValueShift;
			uiValueShift += 8;
			uiNumBits -= 8;

			++m_uiRead;
		}

		uiValue |= uint(pBytes[m_uiRead] & (byte(-1) >> (8 - uiNumBits))) << uiValueShift;
		m_uiReadBit += uiNumBits;
	}

	if (m_uiReadBit == 8)
	{
		++m_uiRead;
		m_uiReadBit = 0;
	}

	return uiValue;
}


/*====================
  CBufferBit::WriteBits
  ====================*/
void	CBufferBit::WriteBits(uint uiValue, uint uiNumBits)
{
	// Check if we have any space left to write
	if (m_uiWrite == m_uiEnd)
		WriteByte(0);

	// Mask unused high part of the value to write
	// This is needed because values are or'd into the existing
	// byte, higher bits would bleed
	uiValue &= uint(-1) >> (32u - uiNumBits);

	if (m_uiWriteBit != 0)
	{
		if (uiNumBits + m_uiWriteBit > 8)
		{
			// Write the rest of the current byte
			m_pBuffer[m_uiWrite] |= (uiValue << m_uiWriteBit) & 0xff;
			uiValue >>= 8 - m_uiWriteBit;
			uiNumBits -= 8 - m_uiWriteBit;

			WriteByte(0);
			++m_uiWrite;
			m_uiWriteBit = 0;

			while (uiNumBits > 8)
			{
				m_pBuffer[m_uiWrite] |= uiValue & 0xff;
				uiValue >>= 8;
				uiNumBits -= 8;

				WriteByte(0);
				++m_uiWrite;
			}

			m_pBuffer[m_uiWrite] |= uiValue & 0xff;
			m_uiWriteBit += uiNumBits;
		}
		else
		{
			m_pBuffer[m_uiWrite] |= (uiValue << m_uiWriteBit) & 0xff;
			m_uiWriteBit += uiNumBits;
		}
	}
	else
	{
		while (uiNumBits > 8)
		{
			m_pBuffer[m_uiWrite] |= uiValue & 0xff;
			uiValue >>= 8;
			uiNumBits -= 8;

			WriteByte(0);
			++m_uiWrite;
		}

		m_pBuffer[m_uiWrite] |= uiValue & 0xff;
		m_uiWriteBit += uiNumBits;
	}

	if (m_uiWriteBit == 8)
	{
		++m_uiWrite;
		m_uiWriteBit = 0;
	}
}


/*====================
  CBufferBit::ReadBit
  ====================*/
uint	CBufferBit::ReadBit() const
{
	if (m_uiRead >= m_uiEnd)
	{
		m_iFaults |= BUFFER_FAULT_UNDERRUN;
		return 0;
	}

	// Cast sign away from data buffer
	const byte* const pBytes(reinterpret_cast<byte*>(m_pBuffer));

	uint uiValue(uint(pBytes[m_uiRead] >> m_uiReadBit) & 0x1);
	++m_uiReadBit;

	if (m_uiReadBit == 8)
	{
		++m_uiRead;
		m_uiReadBit = 0;
	}

	return uiValue;
}


/*====================
  CBufferBit::WriteBit
  ====================*/
void	CBufferBit::WriteBit(uint uiValue)
{
	// Check if we have any space left to write
	if (m_uiWrite == m_uiEnd)
		WriteByte(0);

	m_pBuffer[m_uiWrite] |= ((uiValue & 0x1) << m_uiWriteBit);
	++m_uiWriteBit;
	
	if (m_uiWriteBit == 8)
	{
		++m_uiWrite;
		m_uiWriteBit = 0;
	}
}
