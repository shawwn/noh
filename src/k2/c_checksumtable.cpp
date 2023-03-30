// (C)2010 S2 Games
// c_checksumtable.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_checksumtable.h"
#include "md6.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define CHECKSUM_FILE_VERSION		0
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
static md6_state	g_cMD6State;
static bool			g_bNeedInitChecksum(true);
//=============================================================================

//=============================================================================
// Private Functions
//=============================================================================

/*====================
  InitChecksum
  ====================*/
static bool		InitChecksum(md6_state& st)
{
	if (g_bNeedInitChecksum)
	{
		if (md6_init(&g_cMD6State, 8*CHECKSUM_SIZE) != MD6_SUCCESS)
			return false;
		g_bNeedInitChecksum = false;
	}

	st = g_cMD6State;
	return true;
}


/*====================
  AddToChecksum
  ====================*/
static bool		AddToChecksum(md6_state& st, const byte* pMem, uint uiMemSize)
{
	// add the length of the data to the checksum.
	uint uiSize(LittleInt(uiMemSize)); // make sure the representation is little endian
	if (md6_update(&st, (byte*)&uiSize, 8*sizeof(uint)) != MD6_SUCCESS)
		return false;

	// add the data to the checksum.
	if (md6_update(&st, (byte*)pMem, 8*uiMemSize) != MD6_SUCCESS)
		return false;

	return true;
}


/*====================
  FinalizeChecksum
  ====================*/
static bool		FinalizeChecksum(md6_state& st, byte* pOutChecksum)
{
	// compute the final checksum.
	if (md6_final(&st, pOutChecksum) != MD6_SUCCESS)
		return false;

	return true;
}
//=============================================================================

//=============================================================================
// CChecksumTable::SChecksum
//=============================================================================

/*====================
  SChecksum::SChecksum
  ====================*/
CChecksumTable::SChecksum::SChecksum()
{
	for (int i = 0; i < CHECKSUM_SIZE; ++i)
		val[i] = 0;
}


/*====================
  SChecksum::SChecksum
  ====================*/
CChecksumTable::SChecksum::SChecksum(uint uiIndex, const byte* pChecksum)
: index(uiIndex)
{
	for (int i = 0; i < CHECKSUM_SIZE; ++i)
		val[i] = pChecksum[i];
}


/*====================
  SChecksum::Compare
  ====================*/
bool			CChecksumTable::SChecksum::Compare(const byte* pChecksum)
{
	return (memcmp(val, pChecksum, CHECKSUM_SIZE) == 0);
}


/*====================
  SChecksum::operator <
  ====================*/
bool			CChecksumTable::SChecksum::operator <(const SChecksum &cChecksum) const
{
	return (index < cChecksum.index);
}
//=============================================================================

//=============================================================================
// SChecksumFileHeader
//
//	Do not change the size of this struct!
//=============================================================================
struct SChecksumFileHeader
{
	SChecksumFileHeader()
		: version(CHECKSUM_FILE_VERSION)
		, count(0)
		, padding(0)
	{
		magic[0] = 'C';
		magic[1] = 'S';
		magic[2] = 'U';
		magic[3] = 'M';
	}

	byte		magic[4];	// equal to "CSUM"
	int			version;	// version number.
	int			count;		// number of entries in the checksum table.
	int			padding;	// maintain 16-byte alignment.
};
//=============================================================================

//=============================================================================
// CChecksumTable
//=============================================================================

/*====================
  CChecksumTable::CChecksumTable
  ====================*/
CChecksumTable::CChecksumTable()
{
}


/*====================
  CChecksumTable::~CChecksumTable
  ====================*/
CChecksumTable::~CChecksumTable()
{
}


/*====================
  CChecksumTable::Clear
  ====================*/
void	CChecksumTable::Clear()
{
	m_mapChecksums.clear();
	m_mapLoadedChecksums.clear();
}


/*====================
  CChecksumTable::Add
  ====================*/
void	CChecksumTable::Add(uint uiIndex, const tstring &sFilePath, const byte *pChecksum)
{
	tstring sUsePath(LowerString(sFilePath));
	m_mapChecksums[sUsePath] = SChecksum(uiIndex, pChecksum);
}


/*====================
  CChecksumTable::GetChecksum
  ====================*/
bool	CChecksumTable::GetChecksum(byte *pOutChecksum, const tstring &sFilePath)
{
	tstring sUsePath(LowerString(sFilePath));
	if (sUsePath.empty())
		return false;

	// verify that the path begins with a /
	if (sUsePath[0] != _T('/'))
		sUsePath.insert(sUsePath.begin(), 1, _T('/'));

	ChecksumMap::iterator it(m_mapChecksums.find(sUsePath));

	// if the checksum table does not contain a checksum for that file, return false.
	if (it == m_mapChecksums.end())
		return false;

	SChecksum& sChecksum(it->second);
	MemManager.Copy(pOutChecksum, sChecksum.val, CHECKSUM_SIZE);
	return true;
}


/*====================
  CChecksumTable::Compare
  ====================*/
bool	CChecksumTable::Compare(const tstring &sFilePath, const byte *pChecksum)
{
	tstring sUsePath(LowerString(sFilePath));
	ChecksumMap::iterator it(m_mapChecksums.find(sUsePath));

	// if the checksum table does not contain a checksum for that file, return false.
	if (it == m_mapChecksums.end())
		return false;

	SChecksum& sChecksum(it->second);

	// store the loaded checksum for later.
	m_mapLoadedChecksums[sUsePath] = SChecksum(sChecksum.index, pChecksum);

	if (!sChecksum.Compare(pChecksum))
		return false;

	return true;
}


/*====================
  CChecksumTable::Serialize
  ====================*/
bool	CChecksumTable::Serialize(byte *&pOut, uint &uiOutSize, const tsvector &vFileList)
{
	pOut = NULL;
	uiOutSize = 0;

	CBufferDynamic cBuffer;

	// write the header.
	SChecksumFileHeader cHeader;
	cHeader.count = (int)vFileList.size();
	cBuffer.Append((const void*)&cHeader.magic, sizeof(cHeader.magic));
	cBuffer.WriteInt(cHeader.version);
	cBuffer.WriteInt(cHeader.count);
	cBuffer.WriteInt(cHeader.padding);

	// write each checksum.
	for (tsvector::const_iterator it(vFileList.begin()), itEnd(vFileList.end()); it != itEnd; ++it)
	{
		const tstring &sFilePath(*it);

		ChecksumMap::iterator sumIt(m_mapChecksums.find(sFilePath));
		if (sumIt == m_mapChecksums.end())
			return false;

		const SChecksum& sChecksum(sumIt->second);
		cBuffer.Append(sChecksum.val, CHECKSUM_SIZE);
	}

	// write each file path.
	for (tsvector::const_iterator it(vFileList.begin()), itEnd(vFileList.end()); it != itEnd; ++it)
	{
		const tstring &sFilePath(*it);
		assert(m_mapChecksums.find(sFilePath) != m_mapChecksums.end());
		cBuffer << TStringToUTF8(sFilePath) << byte(0);
	}

	if (cBuffer.GetBufferLength() == 0)
		return false;

	uiOutSize = cBuffer.GetBufferLength();
	pOut = K2_NEW_ARRAY(ctx_FileSystem, byte, uiOutSize);
	MemManager.Copy(pOut, cBuffer.GetBuffer(), uiOutSize);
	return true;
} 


/*====================
  CChecksumTable::Load
  ====================*/
bool	CChecksumTable::Load(const byte *pBuf, uint uiBufSize)
{
	Clear();
	tstring str;

	if (uiBufSize == 0)
		return false;

	CBufferDynamic cBuffer(uiBufSize);
	if (!cBuffer.Write(pBuf, uiBufSize))
		return false;

	// read the header.
	SChecksumFileHeader cHeader;
	if (!cBuffer.Read((void*)&cHeader.magic, sizeof(cHeader.magic)))
		return false;
	
	cHeader.version = cBuffer.ReadInt();
	cHeader.count = cBuffer.ReadInt();
	cHeader.padding = cBuffer.ReadInt();

	// compare the magic value.
	if (memcmp(cHeader.magic, "CSUM", 4) != 0)
		return false;

	// if the version number is greater than our own, then it is a newer format,
	// so don't read it.
	if (cHeader.version > CHECKSUM_FILE_VERSION)
		return false;

	int iNumChecksums(cHeader.count);
	const byte* pChecksums((const byte*)cBuffer.GetBuffer() + cBuffer.GetReadPos());
	cBuffer.Advance(iNumChecksums*CHECKSUM_SIZE);
	if (cBuffer.GetFaults())
		return false;

	// read the file paths.
	for (int i = 0; i < iNumChecksums; ++i)
	{
		tstring sFilePath(cBuffer.ReadTString());

		// if we could not read the file path, abort.
		if (sFilePath == TSNULL || cBuffer.GetFaults() != 0)
		{
			Clear();
			return false;
		}

		m_mapChecksums[sFilePath] = SChecksum((uint)i, pChecksums + i*CHECKSUM_SIZE);
	}

	return true;
}


/*====================
  CChecksumTable::HashChecksums
  ====================*/
bool	CChecksumTable::HashChecksums(byte *pOutChecksum)
{
	for (int i = 0; i < CHECKSUM_SIZE; ++i)
		pOutChecksum[i] = 0;

	if (m_mapChecksums.size() == 0)
		return true;

	vector<SChecksum> vChecksums;
	vChecksums.reserve(m_mapChecksums.size());
	for (ChecksumMap::iterator it(m_mapChecksums.begin()), itEnd(m_mapChecksums.end()); it != itEnd; ++it)
	{
		const tstring &sFilePath(it->first);
		SChecksum &cChecksum(it->second);

		ChecksumMap::iterator findIt(m_mapLoadedChecksums.find(sFilePath));
		if (findIt == m_mapLoadedChecksums.end())
		{
			vChecksums.push_back(cChecksum);
		}
		else
		{
			SChecksum &cUseChecksum(findIt->second);
			assert(cUseChecksum.index == cChecksum.index);
			if (cUseChecksum.index != cChecksum.index)
			{
				Console.Err << _T("Invalid checksum index for ") << sFilePath << newl;
				cUseChecksum.index = cChecksum.index;
			}
			vChecksums.push_back(cUseChecksum);
		}
	}

	// sort the checksums.
	std::sort(vChecksums.begin(), vChecksums.end());

	// compute the checksum of all the checksums.
	md6_state st;
	if (!InitChecksum(st))
		return false;

	uint uiLastIndex(-1);
	for (vector<SChecksum>::iterator it(vChecksums.begin()), itEnd(vChecksums.end()); it != itEnd; ++it)
	{
		SChecksum& cChecksum(*it);
		if (uiLastIndex != uint(-1))
		{
			assert(cChecksum.index != uiLastIndex);
			if (cChecksum.index == uiLastIndex)
				Console.Err << _T("Duplicate checksum index detected: ") << cChecksum.index << newl;
			assert(cChecksum.index >= uiLastIndex);
			if (cChecksum.index < uiLastIndex)
				Console.Err << _T("Unsorted checksum index detected: ") << cChecksum.index << newl;
		}

		if (!AddToChecksum(st, cChecksum.val, CHECKSUM_SIZE))
			return false;

		uiLastIndex = cChecksum.index;
	}

	if (!FinalizeChecksum(st, pOutChecksum))
		return false;

	return true;
}


/*====================
  CChecksumTable::ComputeChecksum
  ====================*/
bool	CChecksumTable::ComputeChecksum(byte *pOutChecksum, const byte *pMem, uint uiMemSize)
{
	for (int i = 0; i < CHECKSUM_SIZE; ++i)
		pOutChecksum[i] = 0;

	// for empty data, return a null checksum.
	if (uiMemSize == 0)
		return true;

	md6_state st;
	if (!InitChecksum(st))
		return false;

	if (!AddToChecksum(st, pMem, uiMemSize))
		return false;

	if (!FinalizeChecksum(st, pOutChecksum))
		return false;

	return true;
}


/*====================
  CChecksumTable::ChecksumToString
  ====================*/
void	CChecksumTable::ChecksumToString(tstring &sOutResult, byte *pChecksum)
{
	sOutResult.clear();
	for (size_t i = 0; i < CHECKSUM_SIZE; ++i)
	{
		byte yByte(pChecksum[i]);
		TCHAR cChar[2];
		for (size_t j = 0; j < 2; ++j)
		{
			byte yCur(yByte & 0xf);
			switch(yCur)
			{
			case 0x0:		cChar[j] = _T('0'); break;
			case 0x1:		cChar[j] = _T('1'); break;
			case 0x2:		cChar[j] = _T('2'); break;
			case 0x3:		cChar[j] = _T('3'); break;
			case 0x4:		cChar[j] = _T('4'); break;
			case 0x5:		cChar[j] = _T('5'); break;
			case 0x6:		cChar[j] = _T('6'); break;
			case 0x7:		cChar[j] = _T('7'); break;
			case 0x8:		cChar[j] = _T('8'); break;
			case 0x9:		cChar[j] = _T('9'); break;
			case 0xa:		cChar[j] = _T('a'); break;
			case 0xb:		cChar[j] = _T('b'); break;
			case 0xc:		cChar[j] = _T('c'); break;
			case 0xd:		cChar[j] = _T('d'); break;
			case 0xe:		cChar[j] = _T('e'); break;
			case 0xf:		cChar[j] = _T('f'); break;
			default:
				assert(false);
				break;
			}

			yByte >>= 4;
		}
		sOutResult.append(1, cChar[1]);
		sOutResult.append(1, cChar[0]);
	}
}
//=============================================================================

