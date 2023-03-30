// (C)2005 S2 Games
// c_zip.cpp
//
// Interface to zlib
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#if defined(linux) || defined(__APPLE__)
#include <unistd.h>
#include <fcntl.h>
#endif

#include "k2_common.h"

#include "c_zip.h"
#include "c_filemanager.h"
#include "c_date.h"
#include "c_compressedfile.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint		ZIP_LOCAL_HEADER_SIGNATURE(0x04034b50);
const uint		ZIP_CENTRAL_HEADER_SIGNATURE(0x02014b50);
const uint		ZIP_CENTRAL_DIRECTORY_END_SIGNATURE(0x06054b50);

const ushort	ZIP_VERSION(20);
const ushort	ZIP_CREATOR(20);

const ushort	ZIP_METHOD_STORE	(0);
const ushort	ZIP_METHOD_DEFLATE	(Z_DEFLATED);

class CCompressedFile;
//=============================================================================

#pragma pack(1)
//=============================================================================
// CZipCentralFileHeader
//=============================================================================
class CZipCentralFileHeader
{
private:
	uint	m_uiSignature;
	ushort	m_unCreator;
	ushort	m_unVersion;
	ushort	m_unFlags;
	ushort	m_unMethod;
	ushort	m_unTime;
	ushort	m_unDate;
	uint	m_uiCRC32;
	uint	m_uiStoredSize;
	uint	m_uiRawSize;
	ushort	m_unFileNameLength;
	ushort	m_unExtraFieldLength;
	ushort	m_unCommentLength;
	ushort	m_unStartingVolume;
	ushort	m_unInternalAttributes;
	uint	m_uiExternalAttributes;
	uint	m_uiLocalHeaderOffset;

	string	m_sFileName;
	string	m_sComment;

	char*	m_pData;

	bool	m_bIsValid;
	int		m_iLevel;
	bool	m_bRemoteData;

	CZipCentralFileHeader();

public:
	~CZipCentralFileHeader()	{ ReleaseData(); }
	CZipCentralFileHeader(const string &sFilename, int iLevel = ZIP_DEFAULT_COMPRESSION_LEVEL, const string &sComment = "");
	CZipCentralFileHeader(fstream &file);

	int		GetLevel() const			{ return m_iLevel; }
	ushort	GetMethod() const			{ return m_unMethod; }
	ushort	GetDate() const				{ return m_unDate; }
	ushort	GetTime() const				{ return m_unTime; }
	uint	GetCRC32() const			{ return m_uiCRC32; }
	uint	GetRawSize() const			{ return m_uiRawSize; }
	uint	GetStoredSize() const		{ return m_uiStoredSize; }
	string	GetFileName() const			{ return m_sFileName; }
	ushort	GetFileNameLength() const	{ return m_unFileNameLength; }
	string	GetComment() const			{ return m_sComment; }
	ushort	GetCommentLength() const	{ return m_unCommentLength; }

	ushort	GetLocalHeaderLength() const		{ return 30 + m_unFileNameLength + m_unCommentLength + m_unExtraFieldLength; }
	ushort	GetCentralHeaderLength() const		{ return 46 + m_unFileNameLength + m_unCommentLength + m_unExtraFieldLength; }

	uint	GetLocalHeaderOffset() const		{ return m_uiLocalHeaderOffset; }
	void	SetLocalHeaderOffset(uint uiOffset)	{ m_uiLocalHeaderOffset = uiOffset; }

	void	SetCRC32(uint uiCRC32)				{ m_uiCRC32 = uiCRC32; }
	void	SetRawSize(uint uiSize)				{ m_uiRawSize = uiSize; }
	void	SetStoredSize(uint uiSize)			{ m_uiStoredSize = uiSize; }

	void	CopyData(const char *pData, uint uiSize)	{ ReleaseData(); m_pData = K2_NEW_ARRAY(ctx_FileSystem, char, uiSize); MemManager.Copy(m_pData, pData, uiSize); }
	void	GetFileData(fstream &file);
	void	ReleaseData()								{ if (!m_bRemoteData) { SAFE_DELETE_ARRAY(m_pData); } else m_pData = NULL; }

	bool	WriteLocalData(fstream &file) const;
	bool	WriteCentralHeader(fstream &file) const;

	void	SetModificationTime(time_t t);

	void	SetRemoteData(bool bRemoteData)		{ m_bRemoteData = bRemoteData; } 
};
//=============================================================================

//=============================================================================
// CZipCentralDirectory
//=============================================================================
class CZipCentralDirectory
{
private:
	uint	m_uiSignature;
	ushort	m_unVolumeNumber;
	ushort	m_unVolumeOfDirectory;
	ushort	m_unEntriesOnThisVolume;
	ushort	m_unTotalEntries;
	uint	m_uiDirectoryLength;
	uint	m_uiDirectoryOffset;
	ushort	m_unCommentLength;
	
	string	m_sComment;

	bool	m_bIsValid;

public:
	~CZipCentralDirectory()	{}
	CZipCentralDirectory(const string &sComment = "");
	CZipCentralDirectory(fstream &file);

	bool	Write(fstream &file);

	uint	GetDirectoryOffset() const			{ return m_uiDirectoryOffset; }

	void	SetDirectoryOffset(uint uiOffset)	{ m_uiDirectoryOffset = uiOffset; }
	void	SetDirectoryLength(uint uiLength)	{ m_uiDirectoryLength = uiLength; }
	void	SetNumEntries(ushort unCount)		{ m_unTotalEntries = m_unEntriesOnThisVolume = unCount; }
};
//=============================================================================

//=============================================================================
// CZipLocalFileHeader
//=============================================================================
class CZipLocalFileHeader
{
private:
	// Actual header data
	uint	m_uiSignature;
	ushort	m_unVersion;
	ushort	m_unFlags;
	ushort	m_unMethod;
	ushort	m_unTime;
	ushort	m_unDate;
	uint	m_uiCRC32;
	uint	m_uiStoredSize;
	uint	m_uiRawSize;
	ushort	m_unFileNameLength;
	ushort	m_unExtraFieldLength;
	string	m_sFileName;

	// Other data
	bool	m_bIsValid;
	string	m_sComment;

	CZipLocalFileHeader();

public:
	~CZipLocalFileHeader()	{}
	CZipLocalFileHeader(fstream &file);

	uint	GetStoredSize() const		{ return m_uiStoredSize; }
};
//=============================================================================
#pragma pack()

/*====================
  CZipCentralFileHeader::CZipCentralFileHeader
  ====================*/
CZipCentralFileHeader::CZipCentralFileHeader(const string &sFilename, int iLevel, const string &sComment) :
m_uiSignature(ZIP_CENTRAL_HEADER_SIGNATURE),
m_unCreator(ZIP_CREATOR),
m_unVersion(ZIP_VERSION),
m_unFlags(0),
m_unMethod(iLevel == 0 ? ZIP_METHOD_STORE : ZIP_METHOD_DEFLATE),
m_unTime(0),
m_unDate(0),
m_uiCRC32(0),
m_uiStoredSize(0),
m_uiRawSize(0),
m_unFileNameLength(INT_SIZE(sFilename.length() & 0xffff)),
m_unExtraFieldLength(0),
m_unCommentLength(INT_SIZE(sComment.length() & 0xffff)),
m_unStartingVolume(0),
m_unInternalAttributes(0),
m_uiExternalAttributes(0),
m_uiLocalHeaderOffset(0),

m_sFileName(sFilename),
m_sComment(sComment),
m_pData(NULL),
m_bIsValid(true),
m_iLevel(iLevel),
m_bRemoteData(false)
{
	CDate date(true);
	m_unDate = ((date.GetYear() - 1980) << 9) | (date.GetMonth() << 5) | date.GetDay();
	m_unTime = (date.GetHour() << 11) | (date.GetMinute() << 5) | (date.GetSecond() >> 1);
}

CZipCentralFileHeader::CZipCentralFileHeader(fstream &file) :
m_pData(NULL),
m_bIsValid(true),
m_bRemoteData(false)
{
	if (!file.is_open())
		return;

#define READ_INT(var) \
	file.read((char*)&var, 4); \
	var = LittleInt(var);
#define READ_SHORT(var) \
	file.read((char*)&var, 2); \
	var = LittleShort(var);

	READ_INT(m_uiSignature);
	READ_SHORT(m_unCreator);
	READ_SHORT(m_unVersion);
	READ_SHORT(m_unFlags);
	READ_SHORT(m_unMethod);
	READ_SHORT(m_unTime);
	READ_SHORT(m_unDate);
	READ_INT(m_uiCRC32);
	READ_INT(m_uiStoredSize);
	READ_INT(m_uiRawSize);
	READ_SHORT(m_unFileNameLength);
	READ_SHORT(m_unExtraFieldLength);
	READ_SHORT(m_unCommentLength);
	READ_SHORT(m_unStartingVolume);
	READ_SHORT(m_unInternalAttributes);
	READ_INT(m_uiExternalAttributes);
	READ_INT(m_uiLocalHeaderOffset);

#undef READ_INT
#undef READ_SHORT

	char *szFileName(K2_NEW_ARRAY(ctx_FileSystem, char, m_unFileNameLength+1));
	MemManager.Set(szFileName, 0, m_unFileNameLength + 1);
	file.read(szFileName, m_unFileNameLength);
	m_sFileName = szFileName;
	SAFE_DELETE_ARRAY(szFileName);

	file.seekg(m_unExtraFieldLength, std::ios_base::cur);

	char *szComment(K2_NEW_ARRAY(ctx_FileSystem, char, m_unCommentLength+1));
	MemManager.Set(szComment, 0, m_unCommentLength + 1);
	file.read(szComment, m_unCommentLength);
	m_sComment = szComment;
	SAFE_DELETE_ARRAY(szComment);

	if (file.fail() ||
		m_uiSignature != ZIP_CENTRAL_HEADER_SIGNATURE ||
		m_unVersion > ZIP_VERSION ||
		(m_unMethod != ZIP_METHOD_STORE && m_unMethod != ZIP_METHOD_DEFLATE))
		m_bIsValid = false;
}


/*====================
  CZipCentralFileHeader::GetFileData
  ====================*/
void	CZipCentralFileHeader::GetFileData(fstream &file)
{
	if (m_pData != NULL)
		return;
	m_pData = K2_NEW_ARRAY(ctx_FileSystem, char, m_uiStoredSize);
	if (m_pData == NULL)
	{
		Console.Err << _T("CZipCentralFileHeader::GetFileData() - Failed to allocate buffer") << newl;
		return;
	}

	file.seekg(m_uiLocalHeaderOffset + 30 + m_unFileNameLength + m_unExtraFieldLength);
	file.read(m_pData, m_uiStoredSize);
}


/*====================
  CZipCentralFileHeader::WriteLocalData
  ====================*/
bool	CZipCentralFileHeader::WriteLocalData(fstream &file) const
{
	if (!m_bIsValid)
		Console.Warn << _T("Writing invalid CZipLocalFileHeader") << newl;
	
	uint uiVar;
	ushort unVar;
	
#define WRITE_INT(var) \
	uiVar = LittleInt(var); \
	file.write((char*)&uiVar, 4);
#define WRITE_SHORT(var) \
	unVar = LittleShort(var); \
	file.write((char*)&unVar, 2);
	
	WRITE_INT(ZIP_LOCAL_HEADER_SIGNATURE);
	WRITE_SHORT(m_unVersion);
	WRITE_SHORT(m_unFlags);
	WRITE_SHORT(m_unMethod);
	WRITE_SHORT(m_unTime);
	WRITE_SHORT(m_unDate);
	WRITE_INT(m_uiCRC32);
	WRITE_INT(m_uiStoredSize);
	WRITE_INT(m_uiRawSize);
	WRITE_SHORT(m_unFileNameLength);
	WRITE_SHORT(m_unExtraFieldLength);
	
#undef WRITE_INT
#undef WRITE_SHORT

	file.write(m_sFileName.c_str(), m_unFileNameLength);

	file.write(m_pData, m_uiStoredSize);

	return !file.fail();
}


/*====================
  CZipCentralFileHeader::WriteCentralHeader
  ====================*/
bool	CZipCentralFileHeader::WriteCentralHeader(fstream &file) const
{
	if (!m_bIsValid)
		Console.Warn << _T("Writing invalid CZipLocalFileHeader") << newl;
	
	uint uiVar;
	ushort unVar;
	
#define WRITE_INT(var) \
		uiVar = LittleInt(var); \
		file.write((char*)&uiVar, 4);
#define WRITE_SHORT(var) \
		unVar = LittleShort(var); \
		file.write((char*)&unVar, 2);
	
	WRITE_INT(ZIP_CENTRAL_HEADER_SIGNATURE);
	WRITE_SHORT(m_unCreator);
	WRITE_SHORT(m_unVersion);
	WRITE_SHORT(m_unFlags);
	WRITE_SHORT(m_unMethod);
	WRITE_SHORT(m_unTime);
	WRITE_SHORT(m_unDate);
	WRITE_INT(m_uiCRC32);
	WRITE_INT(m_uiStoredSize);
	WRITE_INT(m_uiRawSize);
	WRITE_SHORT(m_unFileNameLength);
	WRITE_SHORT(m_unExtraFieldLength);
	WRITE_SHORT(m_unCommentLength);
	WRITE_SHORT(m_unStartingVolume);
	WRITE_SHORT(m_unInternalAttributes);
	WRITE_INT(m_uiExternalAttributes);
	WRITE_INT(m_uiLocalHeaderOffset);
	
#undef WRITE_INT
#undef WRITE_SHORT

	file.write(m_sFileName.c_str(), INT_SIZE(m_sFileName.length()));
	file.write(m_sComment.c_str(), INT_SIZE(m_sComment.length()));

	return !file.fail();
}


/*====================
  CZipCentralFileHeader::SetModificationTime
  ====================*/
void	CZipCentralFileHeader::SetModificationTime(time_t t)
{
	CDate date(t);
	m_unDate = ((date.GetYear() - 1980) << 9) | (date.GetMonth() << 5) | date.GetDay();
	m_unTime = (date.GetHour() << 11) | (date.GetMinute() << 5) | (date.GetSecond() >> 1);
}


/*====================
  CZipCentralDirectory::CZipCentralDirectory
  ====================*/
CZipCentralDirectory::CZipCentralDirectory(const string &sComment) :
m_uiSignature(ZIP_CENTRAL_DIRECTORY_END_SIGNATURE),
m_unVolumeNumber(0),
m_unVolumeOfDirectory(0),
m_unEntriesOnThisVolume(0),
m_unTotalEntries(0),
m_uiDirectoryOffset(0),
m_unCommentLength(INT_SIZE(sComment.length() & 0xffff)),
m_sComment(sComment.substr(0, 0xffff)),

m_bIsValid(true)
{
	m_uiDirectoryLength = 22 + INT_SIZE(m_sComment.length());
}

CZipCentralDirectory::CZipCentralDirectory(fstream &file) :
m_bIsValid(true)
{
	if (!file.is_open())
		return;

#define READ_INT(var) \
	file.read((char*)&var, 4); \
	var = LittleInt(var);
#define READ_SHORT(var) \
	file.read((char*)&var, 2); \
	var = LittleShort(var);

	READ_INT(m_uiSignature);
	READ_SHORT(m_unVolumeNumber);
	READ_SHORT(m_unVolumeOfDirectory);
	READ_SHORT(m_unEntriesOnThisVolume);
	READ_SHORT(m_unTotalEntries);
	READ_INT(m_uiDirectoryLength);
	READ_INT(m_uiDirectoryOffset);
	READ_SHORT(m_unCommentLength);

#undef READ_INT
#undef READ_SHORT

	char *szComment(K2_NEW_ARRAY(ctx_FileSystem, char, m_unCommentLength+1));
	MemManager.Set(szComment, 0, m_unCommentLength + 1);
	file.read(szComment, m_unCommentLength);
	m_sComment = szComment;
	SAFE_DELETE_ARRAY(szComment);

	if (file.fail() || m_uiSignature != ZIP_CENTRAL_DIRECTORY_END_SIGNATURE)
		m_bIsValid = false;
}


/*====================
  CZipCentralDirectory::Write
  ====================*/
bool	CZipCentralDirectory::Write(fstream &file)
{
	if (!m_bIsValid)
		Console.Warn << _T("Writing invalid CZipCentralDirectory") << newl;
	
	uint uiVar;
	ushort unVar;
	
#define WRITE_INT(var) \
	uiVar = LittleInt(var); \
	file.write((char*)&uiVar, 4);
#define WRITE_SHORT(var) \
	unVar = LittleShort(var); \
	file.write((char*)&unVar, 2);
	
	WRITE_INT(m_uiSignature);
	WRITE_SHORT(m_unVolumeNumber);
	WRITE_SHORT(m_unVolumeOfDirectory);
	WRITE_SHORT(m_unEntriesOnThisVolume);
	WRITE_SHORT(m_unTotalEntries);
	WRITE_INT(m_uiDirectoryLength);
	WRITE_INT(m_uiDirectoryOffset);
	WRITE_SHORT(m_unCommentLength);
	
#undef WRITE_INT
#undef WRITE_SHORT

	file.write(m_sComment.c_str(), INT_SIZE(m_sComment.length()));

	return !file.fail();
}


/*====================
  CZipLocalFileHeader::CZipLocalFileHeader
  ====================*/
CZipLocalFileHeader::CZipLocalFileHeader(fstream &file) :
m_bIsValid(true)
{
	if (!file.is_open())
		return;

#define READ_INT(var) \
	file.read((char*)&var, 4); \
	var = LittleInt(var);
#define READ_SHORT(var) \
	file.read((char*)&var, 2); \
	var = LittleShort(var);

	READ_INT(m_uiSignature);
	READ_SHORT(m_unVersion);
	READ_SHORT(m_unFlags);
	READ_SHORT(m_unMethod);
	READ_SHORT(m_unTime);
	READ_SHORT(m_unDate);
	READ_INT(m_uiCRC32);
	READ_INT(m_uiStoredSize);
	READ_INT(m_uiRawSize);
	READ_SHORT(m_unFileNameLength);
	READ_SHORT(m_unExtraFieldLength);

#undef READ_INT
#undef READ_SHORT

	char *szFileName(K2_NEW_ARRAY(ctx_FileSystem, char, m_unFileNameLength+1));
	MemManager.Set(szFileName, 0, m_unFileNameLength + 1);
	file.read(szFileName, m_unFileNameLength);
	m_sFileName = szFileName;
	SAFE_DELETE_ARRAY(szFileName);

	file.seekg(m_unExtraFieldLength, std::ios_base::cur);

	if (file.fail() ||
		m_uiSignature != ZIP_LOCAL_HEADER_SIGNATURE ||
		m_unVersion > ZIP_VERSION ||
		(m_unMethod != ZIP_METHOD_STORE && m_unMethod != ZIP_METHOD_DEFLATE))
		m_bIsValid = false;
}


/*====================
  CZip::~CZip
  ====================*/
CZip::~CZip()
{
	Close();

	for (vector<CZipCentralFileHeader*>::iterator it(m_vCentralHeaders.begin()); it != m_vCentralHeaders.end(); ++it)
		SAFE_DELETE(*it);

	SAFE_DELETE(m_pCentralDirectory);
}


/*====================
  CZip::CZip
  ====================*/
CZip::CZip(const tstring &sFilename, bool bAppend) :
m_sPathName(sFilename),
m_pCentralDirectory(NULL),
m_bWriting(false),
m_fWriteProgress(0.0f)
{
	Open(m_sPathName, bAppend);
}


/*====================
  CZip::ReadCentralDirectory
  ====================*/
void	CZip::ReadCentralDirectory()
{
	m_file.seekg(0);

	while (!m_file.fail() && !m_file.eof())
	{
		uint uiSignature(0);
		m_file.read((char*)&uiSignature, 4);
		m_file.seekg(-4, std::ios_base::cur);
		
		ToLittle(uiSignature);
		
		if (uiSignature == ZIP_LOCAL_HEADER_SIGNATURE)
		{
			CZipLocalFileHeader localHeader(m_file);
			m_file.seekg(localHeader.GetStoredSize(), std::ios_base::cur);
			continue;
		}

		if (uiSignature == ZIP_CENTRAL_HEADER_SIGNATURE)
		{
			m_vCentralHeaders.push_back(K2_NEW(ctx_FileSystem,  CZipCentralFileHeader)(m_file));
			continue;
		}

		if (uiSignature == ZIP_CENTRAL_DIRECTORY_END_SIGNATURE)
		{
			m_pCentralDirectory = K2_NEW(ctx_FileSystem,  CZipCentralDirectory)(m_file);
			return;
		}

		Console.Err << _T("Found an invalid signature while searching for central directory") << newl;
		SAFE_DELETE(m_pCentralDirectory);
		break;
	}
}


/*====================
  CZip::WriteToDisk
  ====================*/
bool	CZip::WriteToDisk(uint uiMaxTime)
{
	if (!IsOpen())
		return true;

	if (!m_bWriting)
	{
		// Create a temporary file to do the actual write
		m_sTempPath = Filename_GetPath(m_sPathName) + _T(".temp.") + Filename_StripPath(m_sPathName);
		tstring sTempSystemPath(FileManager.GetSystemPath(m_sTempPath, TSNULL, true));
	#if defined(linux) || defined(__APPLE__)
		// need to check ~sPath as well since maps are saved/downloaded to user dir
		if (sTempSystemPath.empty() && m_sTempPath[0] != _T('~') && m_sTempPath[0] != _T('#') && m_sTempPath[0] != _T(':'))
			sTempSystemPath = FileManager.GetSystemPath(_TS("~") + m_sTempPath, TSNULL, true);
	#endif
		m_fileTemp.open(TStringToNative(sTempSystemPath).c_str(), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
		if (!m_fileTemp.is_open())
		{
			Console.Err << _T("CZip::WriteToDisk() - Failed to create temporary file: ") << m_sTempPath << newl;
			return true;
		}

		m_itWrite = m_vCentralHeaders.begin();
		m_uiWritePosition = 0;
		m_bWriting = true;
	}

	uint uiStartTime(uiMaxTime != INVALID_TIME ? K2System.Milliseconds() : INVALID_TIME);

	// Write files and local headers
	vector<CZipCentralFileHeader*>::iterator itEnd(m_vCentralHeaders.end());
	for (; m_itWrite != itEnd; ++m_itWrite)
	{
		CZipCentralFileHeader *pHeader(*m_itWrite);
		pHeader->GetFileData(m_file);
		pHeader->SetLocalHeaderOffset(m_uiWritePosition);
		pHeader->WriteLocalData(m_fileTemp);
		pHeader->ReleaseData();
		m_uiWritePosition += pHeader->GetLocalHeaderLength() + pHeader->GetStoredSize();

		if (uiMaxTime != INVALID_TIME)
		{
			uint uiElapsedTime(K2System.Milliseconds() - uiStartTime);

			if (uiElapsedTime > uiMaxTime)
			{
				++m_itWrite;
				break;
			}
		}
	}

	if (m_itWrite != itEnd)
	{
		m_fWriteProgress = float(m_itWrite - m_vCentralHeaders.begin()) / m_vCentralHeaders.size();
		return false;
	}

	// Write central directory headers
	uint uiDirectoryLength(0);
	for (vector<CZipCentralFileHeader*>::iterator it(m_vCentralHeaders.begin()); it != m_vCentralHeaders.end(); ++it)
	{
		CZipCentralFileHeader *pHeader(*it);
		pHeader->WriteCentralHeader(m_fileTemp);
		uiDirectoryLength += pHeader->GetCentralHeaderLength();
	}

	// Write central directory termination
	m_pCentralDirectory->SetDirectoryOffset(m_uiWritePosition);
	m_pCentralDirectory->SetDirectoryLength(uiDirectoryLength);
	m_pCentralDirectory->SetNumEntries(ushort(m_vCentralHeaders.size() & 0xffff));
	m_pCentralDirectory->Write(m_fileTemp);

	// Replace the original file with the temporary one
	m_fileTemp.close();
	m_file.close();
	FileManager.Delete(m_sPathName);
	FileManager.Rename(m_sTempPath, m_sPathName, FileManager.GetTopModPath());
	return true;
}


/*====================
  CZip::Open
  ====================*/
bool	CZip::Open(const tstring &sPath, bool bAppend)
{
	tstring sSystemPath(FileManager.GetSystemPath(FileManager.SanitizePath(sPath), _T(""), true));
	
#ifdef linux
	// need to check ~sPath as well since maps are saved/downloaded to user dir
	if (sSystemPath.empty() && sPath[0] != _T('~') && sPath[0] != _T('#') && sPath[0] != _T(':'))
		sSystemPath = FileManager.GetSystemPath(_TS("~") + sPath, _T(""), true);
#endif

	// Create missing directories
	FileManager.MakeDir(Filename_GetPath(sPath));
	
	m_file.open(TStringToNative(sSystemPath).c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::binary | (bAppend ? static_cast<std::ios_base::openmode>(0) : std::ios_base::trunc));
	if (!m_file.is_open())
	{
		bAppend = false;
		
		m_file.open(TStringToNative(sSystemPath).c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::binary | (bAppend ? static_cast<std::ios_base::openmode>(0) : std::ios_base::trunc));
		if (!m_file.is_open())
			return false;
	}
	
#if defined(linux) || defined(__APPLE__)
	// set files to close on exec
	struct fd_accessor : public std::basic_filebuf<char> { int fd() { return _M_file.fd(); } };
	int fd = static_cast<fd_accessor*>(m_file.rdbuf())->fd();
	long flags;
	if ((flags = fcntl(fd, F_GETFD, 0)) == -1)
		flags = 0;

	if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1)
		return false;
#endif

	if (bAppend)
		ReadCentralDirectory();
	else
		m_pCentralDirectory = K2_NEW(ctx_FileSystem,  CZipCentralDirectory)();

	return IsOpen();
}


/*====================
  CZip::AddFile
  ====================*/
int		CZip::AddFile(const tstring &sFileName, const char *pSourceBuffer, size_t zLength, int iLevel, time_t t, const string &sComment)
{
	try
	{
		if (sFileName.empty())
			EX_WARN(_T("Empty filename"));

		if (!IsOpen())
			EX_ERROR(_T("Archive is not open"));

		string sCleanFileName(TStringToString(sFileName));
		if (sCleanFileName[0] == _T('/'))
			sCleanFileName = sCleanFileName.substr(1, string::npos);

		// Remove the file if it already exists
		vector<CZipCentralFileHeader*>::iterator it(m_vCentralHeaders.begin());
		while (it != m_vCentralHeaders.end())
		{
			if (CompareNoCase((*it)->GetFileName(),sCleanFileName) != 0)
			{
				++it;
				continue;
			}

			K2_DELETE(*it);
			it = m_vCentralHeaders.erase(it);
		}

		// Create a new entry for this file
		CZipCentralFileHeader *pNewFileHeader(K2_NEW(ctx_FileSystem,  CZipCentralFileHeader)(sCleanFileName, iLevel, sComment));
		if (pNewFileHeader == NULL)
			EX_ERROR(_T("Failed to allocate new file entry"));

		if (t != 0)
			pNewFileHeader->SetModificationTime(t);

		// Compress
		uLong ulStoredSize = INT_SIZE(zLength);
		const char *pCopyBuffer(pSourceBuffer);
		byte *pTempBuffer(NULL);
		if (pNewFileHeader->GetMethod() == ZIP_METHOD_DEFLATE)
		{
			ulStoredSize = INT_CEIL(ulStoredSize * 1.001f) + 12;
			pTempBuffer = K2_NEW_ARRAY(ctx_FileSystem, byte, ulStoredSize);
			MemManager.Set(pTempBuffer, 0, ulStoredSize);
			compress2(pTempBuffer, &ulStoredSize, (byte*)pSourceBuffer, INT_SIZE(zLength), iLevel);
			pCopyBuffer = (char*)pTempBuffer + 2;
			ulStoredSize -= 6;
		}

		// Fill in the missing info
		pNewFileHeader->SetRawSize(INT_SIZE(zLength));
		pNewFileHeader->SetCRC32(crc32(0, (byte*)pSourceBuffer, INT_SIZE(zLength)));
		pNewFileHeader->SetStoredSize(ulStoredSize);
		pNewFileHeader->CopyData(pCopyBuffer, ulStoredSize);

		// Store the entry
		m_vCentralHeaders.push_back(pNewFileHeader);

		SAFE_DELETE_ARRAY(pTempBuffer);
		return 0;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CZip::AddFile() - "), NO_THROW);
		return ZIP_INTERNALERROR;
	}
}


/*====================
  CZip::AddCompressedFile
  ====================*/
int		CZip::AddCompressedFile(const tstring &sFileName, const CCompressedFile &cFile, time_t t, const string &sComment)
{
	try
	{
		if (sFileName.empty())
			EX_WARN(_T("Empty filename"));

		if (!IsOpen())
			EX_ERROR(_T("Archive is not open"));

		string sCleanFileName(TStringToString(sFileName));
		if (sCleanFileName[0] == _T('/'))
			sCleanFileName = sCleanFileName.substr(1, string::npos);

		// Remove the file if it already exists
		vector<CZipCentralFileHeader*>::iterator it(m_vCentralHeaders.begin());
		while (it != m_vCentralHeaders.end())
		{
			if (LowerString((*it)->GetFileName()) != sCleanFileName)
			{
				++it;
				continue;
			}

			K2_DELETE(*it);
			it = m_vCentralHeaders.erase(it);
		}

		// Create a new entry for this file
		CZipCentralFileHeader *pNewFileHeader(K2_NEW(ctx_FileSystem,  CZipCentralFileHeader)(sCleanFileName, cFile.GetLevel(), sComment));
		if (pNewFileHeader == NULL)
			EX_ERROR(_T("Failed to allocate new file entry"));

		if (t != 0)
			pNewFileHeader->SetModificationTime(t);

		// Fill in the missing info
		pNewFileHeader->SetRawSize(cFile.GetRawSize());
		pNewFileHeader->SetCRC32(cFile.GetCRC32());
		pNewFileHeader->SetStoredSize(cFile.GetStoredSize());
		pNewFileHeader->CopyData(cFile.GetData(), cFile.GetStoredSize());

		// Store the entry
		m_vCentralHeaders.push_back(pNewFileHeader);

		return 0;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CZip::AddCompressedFile() - "), NO_THROW);
		return ZIP_INTERNALERROR;
	}
}


/*====================
  CZip::RemoveFile
  ====================*/
int		CZip::RemoveFile(const tstring &sFileName)
{
	try
	{
		if (sFileName.empty())
			EX_WARN(_T("Empty filename"));

		if (!IsOpen())
			EX_ERROR(_T("Archive is not open"));

		string sCleanFileName(TStringToString(sFileName));
		if (sCleanFileName[0] == _T('/'))
			sCleanFileName = sCleanFileName.substr(1, string::npos);

		// Remove the file
		vector<CZipCentralFileHeader*>::iterator it(m_vCentralHeaders.begin());
		while (it != m_vCentralHeaders.end())
		{
			if (LowerString((*it)->GetFileName()) != sCleanFileName)
			{
				++it;
				continue;
			}
			
			K2_DELETE(*it);
			it = m_vCentralHeaders.erase(it);
		}

		return 0;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CZip::DeleteFile() - "), NO_THROW);
		return ZIP_INTERNALERROR;
	}
}


/*====================
  CZip::Close
  ====================*/
bool	CZip::Close(uint uiMaxTime)
{
	if (!WriteToDisk(uiMaxTime))
		return false;
	
	SAFE_DELETE(m_pCentralDirectory);
	m_file.close();
	
	return true;
}


/*====================
  CZip::CancelWrite
  ====================*/
bool	CZip::CancelWrite()
{
	SAFE_DELETE(m_pCentralDirectory);
	m_file.close();
	return true;
}


/*====================
  CZip::Compress
  ====================*/
uint	CZip::Compress(const byte *pSource, uint uiSourceLength, byte *&pDest, int iLevel)
{
	uint uiDestLength(uiSourceLength + INT_CEIL((uiSourceLength) * 0.001f) + 12);
	pDest = K2_NEW_ARRAY(ctx_FileSystem, byte, uiDestLength);
	long unsigned int uiResultLength(uiDestLength);
	switch (compress2(pDest, &uiResultLength, pSource, uiSourceLength, CLAMP(iLevel, 0, 9)))
	{
	case Z_OK:
		break;
	case Z_BUF_ERROR:
		Console.Err << _T("Output buffer too small") << newl;
		break;
	case Z_MEM_ERROR:
		Console.Err << _T("Out of memory") << newl;
		break;
	}

	return uiResultLength;
}


/*====================
  CZip::Decompress
  ====================*/
bool	CZip::Decompress(const byte *pSource, uint uiSourceLength, byte *pDest, uint uiDestLength)
{
	long unsigned int uiResultLength(uiDestLength);
	switch(uncompress(pDest, &uiResultLength, pSource, uiSourceLength))
	{
	case Z_OK:
		return true;

	case Z_BUF_ERROR:
		Console.Err << _T("CZip::Decompress() - Output buffer too small") << newl;
		break;
	
	case Z_MEM_ERROR:
		Console.Err << _T("CZip::Decompress() - Out of memory") << newl;
		break;
	
	case Z_DATA_ERROR:
		Console.Err << _T("CZip::Decompress() - Corrupt data") << newl;
		break;

	default:
		Console.Err << _T("CZip::Decompress() - Unknown error") << newl;
		break;
	}

	return false;
}


/*====================
  CZip::FileExists
  ====================*/
bool	CZip::FileExists(const tstring &sFilename)
{
	string sCleanFilename(TStringToString(LowerString(sFilename)));
	
	for (vector<CZipCentralFileHeader*>::iterator it(m_vCentralHeaders.begin()); it != m_vCentralHeaders.end(); ++it)
	{
		if (LowerString((*it)->GetFileName()) == sCleanFilename)
			return true;
	}

	return false;
}
