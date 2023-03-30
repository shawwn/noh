// (C)2010 S2 Games
// c_mmapunzip.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_mmapunzip.h"

#include "c_sample.h"
#include "c_texture.h"
#include "c_material.h"
#include "c_compressedfile.h"

#include <zlib.h>
//=============================================================================

EXTERN_CVAR_BOOL(fs_smallMMap);

/*====================
  CMMapUnzip::AddZippedFile
  ====================*/
void	CMMapUnzip::AddZippedFile(const tstring &sFilename, SArchiveFileInfo* info)
{
	if (sFilename.empty())
		return;

	// Ignore directory entries
	if (sFilename[sFilename.length() - 1] == _T('/'))
		return;

	SZippedFile *pFile(K2_NEW(ctx_FileSystem,  SZippedFile));
	pFile->uiPos = LittleInt(info->relativeOffset) + sizeof(SArchiveLocalInfo) + LittleShort(info->filenameLength) + LittleShort(info->extraLength);
	pFile->bCompressed = (LittleShort(info->compression) != 0);
	pFile->uiSize = LittleInt(info->compressed);
	pFile->uiRawSize = LittleInt(info->uncompressed);
	pFile->uiCRC32 = LittleInt(info->crc32);

#if 0
	if (!pFile->bCompressed || pFile->uiRawSize == 1)
		K2System.DebugBreak();
#endif

	if (pFile->uiSize == -1)
	{
		pFile->uiSize = 0;
		Console.Warn << _T("CMMapUnzip::AddZippedFile() - Invalid file length detected") << newl;
	}

	if (pFile->uiRawSize == -1)
	{
		pFile->uiRawSize = 0;
		Console.Warn << _T("CMMapUnzip::AddZippedFile() - Invalid file length detected") << newl;
	}

	m_mapFiles.insert(ZFMap_pair(sFilename, pFile));
	m_vFileNames.push_back(sFilename);
}


/*====================
  CMMapUnzip::SearchCentralDir
  ====================*/
uint	CMMapUnzip::SearchCentralDir(SArchiveCentralInfo *pCentralInfo)
{
	// Central directory is at the end of the file
	// right before the global comment. The global
	// comment has a maximum size of 64k

	int iEnd(MAX<int>(m_uiSize - 66000, 3));

	dword dwStartOffset(0);
	bool bReleaseData(false);
#ifndef _WIN32
	size_t length(0);
#endif

	if (m_pData == NULL)
	{
		bReleaseData = true;
		dwStartOffset = (iEnd / K2System.GetPageSize()) * K2System.GetPageSize();
		dword dwOffset = iEnd - dwStartOffset;

#ifdef _WIN32
		m_pData = static_cast<const char *>(MapViewOfFile(m_hMappedFile, FILE_MAP_READ, 0, dwStartOffset, dwOffset + (m_uiSize - iEnd)));

		if (m_pData == NULL)
		{
			Console.Warn << _T("CMMapUnzip::SearchCentralDir - Error mapping file to memory: ") << K2System.GetLastErrorString() << newl;
			return 0;
		}
#else
		length = dwOffset + (m_uiSize - iEnd);
		m_pData = static_cast<const char *>(mmap(NULL, length, PROT_READ, MAP_PRIVATE, m_iFile, dwStartOffset));

		if (m_pData == static_cast<const char *>(MAP_FAILED))
		{
			m_pData = NULL;
			Console.Warn << _T("CMMapUnzip::SearchCentralDir - Error mapping file to memory: ") << K2System.GetLastErrorString() << newl;
			return 0;
		}
#endif
	}

	// Scan for signature
	for (int i(m_uiSize - 1); i >= iEnd; --i)
	{
		if (m_pData[i - (0 + dwStartOffset)] == 0x06 &&
			m_pData[i - (1 + dwStartOffset)] == 0x05 &&
			m_pData[i - (2 + dwStartOffset)] == 0x4b &&
			m_pData[i - (3 + dwStartOffset)] == 0x50)
		{
			// Signature found; return its position

			// Check that the file hasn't been truncated
			if (m_uiSize - (i - 3) < sizeof(SArchiveCentralInfo))
			{
				if (bReleaseData)
				{
#ifdef _WIN32
					UnmapViewOfFile(m_pData);
#else
					munmap((void*)m_pData, length);
#endif
					m_pData = NULL;
				}

				return 0;
			}

			// Initialize the central directory
			MemManager.Copy(pCentralInfo, &m_pData[i - (3 + dwStartOffset)], sizeof(SArchiveCentralInfo));

			if (bReleaseData)
			{
#ifdef _WIN32
				UnmapViewOfFile(m_pData);
#else
				munmap((void*)m_pData, length);
#endif
				m_pData = NULL;
			}

			return i - 3;
		}
	}

	if (bReleaseData)
	{
#ifdef _WIN32
		UnmapViewOfFile(m_pData);
#else
		munmap((void*)m_pData, length);
#endif
		m_pData = NULL;
	}

	return 0;
}


/*====================
  CMMapUnzip::CMMapUnzip
  ====================*/
CMMapUnzip::CMMapUnzip(const tstring &sPath) :
m_bMemory(false),
m_bInitialized(false)
{
	// Initialize required members incase we bail early
	m_pData = NULL;

	// Setup memory mapping
#ifdef _WIN32
	// Windows version
	m_hFile = INVALID_HANDLE_VALUE;
	m_hMappedFile = INVALID_HANDLE_VALUE;
	m_hFile = CreateFile(sPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
	if	(m_hFile == INVALID_HANDLE_VALUE)
		return;

	// Determine the archive's filesize
	m_uiSize = GetFileSize(m_hFile, NULL);

	if (m_uiSize == 0)
		return;

	m_hMappedFile = CreateFileMapping(m_hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (m_hMappedFile == INVALID_HANDLE_VALUE)
		return;

	if (!fs_smallMMap)
	{
		m_pData = static_cast<const char *>(MapViewOfFile(m_hMappedFile, FILE_MAP_READ, 0, 0, 0));
	}
#else
	// Linux version
	m_iFile = open(TStringToNative(sPath).c_str(), O_RDONLY);
	if (m_iFile == -1)
		return;
	
	// set files to close on exec
	long flags;
	if ((flags = fcntl(m_iFile, F_GETFD, 0)) == -1)
		flags = 0;

	if (fcntl(m_iFile, F_SETFD, flags | FD_CLOEXEC) == -1)
		return;
	
	// Determine the archive's filesize
	struct stat buf;
	if (fstat(m_iFile, &buf) == -1)
		return;
	
	m_uiSize = buf.st_size;
	
	if (m_uiSize == 0)
		return;
	
	m_pData = static_cast<const char *>(mmap(NULL, m_uiSize, PROT_READ, MAP_PRIVATE, m_iFile, 0));
	if (m_pData == static_cast<const char *>(MAP_FAILED))
	{
		m_pData = NULL;
		return;
	}
#endif

	Initialize();
}

CMMapUnzip::CMMapUnzip(const char *pBuffer, uint uiSize) :
m_bMemory(true)
{
	m_pData = pBuffer;
	m_uiSize = uiSize;

	if (m_pData == NULL || m_uiSize == 0)
		return;

	Initialize();
}


/*====================
  CMMapUnzip::Initialize
  ====================*/
void	CMMapUnzip::Initialize()
{
	// Locate the the central directory
	SArchiveCentralInfo centralInfo;
	uint uiDirPos(SearchCentralDir(&centralInfo));
	if (uiDirPos == 0)
		return;

	// Check that the file hasn't been truncated
	if (m_uiSize - uiDirPos < sizeof(SArchiveCentralInfo))
		return;

	// Prepare the various temporary variables for the loop...
	uint uiPos(LittleInt(centralInfo.centralDirOffset));
	SArchiveFileInfo *pInfo(NULL);
	char szFilename[1024];
	uint uiMaxLength(m_uiSize);
#ifndef _WIN32
	size_t length(0);
#endif

	bool bReleaseData(false);

	if (m_pData == NULL)
	{
		bReleaseData = true;
		
		dword dwStartOffset = (uiPos / K2System.GetPageSize()) * K2System.GetPageSize();
		dword dwOffset = uiPos - dwStartOffset;

#ifdef _WIN32
		m_pData = static_cast<const char *>(MapViewOfFile(m_hMappedFile, FILE_MAP_READ, 0, dwStartOffset, dwOffset + (centralInfo.centralDirSize)));

		if (m_pData == NULL)
		{
			Console.Warn << _T("CMMapUnzip::SearchCentralDir - Error mapping file to memory: ") << K2System.GetLastErrorString() << newl;
			return;
		}
#else
		length = dwOffset + (centralInfo.centralDirSize);
		m_pData = static_cast<const char *>(mmap(NULL, length, PROT_READ, MAP_PRIVATE, m_iFile, dwStartOffset));

		if (m_pData == static_cast<const char *>(MAP_FAILED))
		{
			m_pData = NULL;
			Console.Warn << _T("CMMapUnzip::CMMapUnzip - Error mapping file to memory: ") << K2System.GetLastErrorString() << newl;
			return;
		}
#endif
		uiPos = dwOffset;
		uiMaxLength = centralInfo.centralDirSize + dwOffset;
	}

	while (true)
	{
		// Check that the file hasn't been truncated
		if (uiMaxLength - uiPos < sizeof(SArchiveFileInfo))
			break;

		// Point to individual file info
		pInfo = (SArchiveFileInfo*)&m_pData[uiPos];
		uiPos += sizeof(SArchiveFileInfo);

		// Check if the signature matches; if not, the file headers are over (normal termination)
		if (LittleInt(pInfo->signature) != 0x02014b50)
			break;

		uint uiFilenameLength(MIN<uint>(LittleShort(pInfo->filenameLength), 1023));
		
		// Copy filename into temporary buffer
		MemManager.Copy(szFilename, &m_pData[uiPos], uiFilenameLength);
	
		// Convert the filename to lowercase
		szFilename[uiFilenameLength] = 0;
		for (uint ui(0); ui < uiFilenameLength; ++ui)
			szFilename[ui] = tolower(szFilename[ui]);

		// Advance
		uiPos += LittleShort(pInfo->filenameLength);
		uiPos += LittleShort(pInfo->extraLength);

		// Add file to the FileSystem (if some criterias are met, see function)
		AddZippedFile(StringToTString(szFilename), pInfo);
	}

	if (bReleaseData)
	{
#ifdef _WIN32
		UnmapViewOfFile(m_pData);
#else
		munmap((void*)m_pData, length);
#endif
		m_pData = NULL;
	}

	m_bInitialized = true;
}


/*====================
  CMMapUnzip::~CMMapUnzip
  ====================*/
CMMapUnzip::~CMMapUnzip()
{
	if (m_bMemory)
		return;

	for (ZFMap_it it(m_mapFiles.begin()), itEnd(m_mapFiles.end()); it != itEnd; ++it)
		K2_DELETE(it->second);

	// Cleanup file mapping and handles
#ifdef _WIN32
	if (m_pData != NULL)
		UnmapViewOfFile(m_pData);
	if (m_hMappedFile != INVALID_HANDLE_VALUE)
		CloseHandle(m_hMappedFile);
	if (m_hFile != INVALID_HANDLE_VALUE)
		CloseHandle(m_hFile);
#else
	if(m_pData != NULL)
		munmap((void*)m_pData, m_uiSize);
	if (m_iFile != -1)
		close(m_iFile);
#endif
}


/*====================
  CMMapUnzip::OpenUnzipFile
  ====================*/
uint	CMMapUnzip::OpenUnzipFile(const tstring &sFilename, char *&pBuffer)
{
	try
	{
		pBuffer = NULL;

		if (sFilename.empty())
			return 0;

		// Check the archive actually loaded properly
		if (!m_bInitialized)
			return 0;

		// Convert the filename to lowercase and look it up in our hashmap
		tstring sLowerFilename(LowerString(sFilename));
		ZFMap_it file_entry(m_mapFiles.find(sLowerFilename));
		if (file_entry == m_mapFiles.end())
			return 0;

		// Ensure that we don't exceed the maximum array size somehow...
		if (file_entry->second->uiRawSize >= 0x7FFFFFFF)
		{
			Console.Warn << _T("Unzip exceeded maximum array size!") << newl;
			return 0;
		}

		// Allocate the buffer based on the uncompressed file size
		pBuffer = K2_NEW_ARRAY(ctx_FileSystem, char, file_entry->second->uiRawSize);

		const char *pFileEntryBuffer(NULL);

		bool bReleaseData(false);
		
#ifndef _WIN32
		size_t length(0);
#endif

		if (m_pData == NULL)
		{
			bReleaseData = true;

			dword dwStartOffset = (file_entry->second->uiPos / K2System.GetPageSize()) * K2System.GetPageSize();
			dword dwOffset = file_entry->second->uiPos - dwStartOffset;

#ifdef _WIN32
			m_pData = static_cast<const char *>(MapViewOfFile(m_hMappedFile, FILE_MAP_READ, 0, dwStartOffset, file_entry->second->uiSize + dwOffset));

			if (m_pData == NULL)
			{
				SAFE_DELETE_ARRAY(pBuffer);
				return 0;
			}
#else
			size_t length(file_entry->second->uiSize + dwOffset);
			m_pData = static_cast<const char *>(mmap(NULL, length, PROT_READ, MAP_PRIVATE, m_iFile, dwStartOffset));
			if (m_pData == static_cast<const char *>(MAP_FAILED))
			{
				SAFE_DELETE_ARRAY(pBuffer);
				m_pData = NULL;
				return 0;
			}
#endif
			// Point to memory mapped data
			pFileEntryBuffer = (const char *)&m_pData[dwOffset];
		}
		else
		{
			pFileEntryBuffer = &m_pData[file_entry->second->uiPos];
		}

		// If the data is compressed, we need to uncompress it now (only zlib is supported)
		if (file_entry->second->bCompressed)
		{
			int ret;
			z_stream_s Dstrm;

			// Initialize the decompression stream's structure
			Dstrm.zalloc = Z_NULL;
			Dstrm.zfree = Z_NULL;
			Dstrm.opaque = Z_NULL;
			inflateInit2(&Dstrm, -15);
			ret = inflateReset(&Dstrm);
			if (ret < 0 || ret == 2)
			{
				// Failure, abort
				inflateEnd(&Dstrm);
				SAFE_DELETE_ARRAY(pBuffer);
				return 0;
			}

			// Give the structure information over the data's size and location
			Dstrm.next_in = (Bytef*)pFileEntryBuffer;
			Dstrm.next_out = (Bytef*)pBuffer;
			Dstrm.avail_in = file_entry->second->uiSize;
			Dstrm.avail_out = file_entry->second->uiRawSize;

			// This function will do the actual decompression
			ret = inflate(&Dstrm, Z_SYNC_FLUSH);
			if (ret < 0 || ret == 2)
			{
				// Failure, abort
				inflateEnd(&Dstrm);
				SAFE_DELETE_ARRAY(pBuffer);
				return 0;
			}

			// Finished decompressing data
			inflateEnd(&Dstrm);
		}
		else
		{
			// If the data isn't compressed, just copy it from buffer to buffer
			MemManager.Copy(pBuffer, pFileEntryBuffer, file_entry->second->uiRawSize);
		}

		if (bReleaseData)
		{
#ifdef _WIN32
			UnmapViewOfFile(m_pData);
#else
			munmap((void*)m_pData, length);
#endif
			m_pData = NULL;
		}

		// And finally, return the file's rawsize for the data in pBuffer
		return file_entry->second->uiRawSize;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CMMapUnzip::OpenUnzipFile() - "), NO_THROW);
		return 0;
	}
}


/*====================
  CMMapUnzip::GetCompressedFile
  ====================*/
uint	CMMapUnzip::GetCompressedFile(const tstring &sFilename, CCompressedFile &cFile)
{
	try
	{
		if (sFilename.empty())
			return 0;

		// Check the archive actually loaded properly
		if (!m_bInitialized)
			return 0;

		// Force the full file to map if we're currently using a small mapping
		if (m_pData == NULL)
#ifdef _WIN32
			m_pData = static_cast<const char *>(MapViewOfFile(m_hMappedFile, FILE_MAP_READ, 0, 0, 0));
#else
			m_pData = static_cast<const char *>(mmap(NULL, m_uiSize, PROT_READ, MAP_PRIVATE, m_iFile, 0));
#endif

		// Convert the filename to lowercase and look it up in our hashmap
		tstring sLowerFilename(LowerString(sFilename));
		if (sLowerFilename[0] == _T('/'))
			sLowerFilename = sLowerFilename.substr(1);
		ZFMap_it file_entry(m_mapFiles.find(sLowerFilename));
		if (file_entry == m_mapFiles.end())
			return 0;

		// Ensure that we don't exceed the maximum array size somehow...
		if (file_entry->second->uiRawSize >= 0x7FFFFFFF)
		{
			Console.Warn << _T("Unzip exceeded maximum array size!") << newl;
			return 0;
		}

		cFile = CCompressedFile
		(
			&m_pData[file_entry->second->uiPos],
			file_entry->second->uiCRC32,
			file_entry->second->uiSize,
			file_entry->second->uiRawSize,
			file_entry->second->bCompressed ? 1 : 0
		);

		return file_entry->second->uiRawSize;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CMMapUnzip::GetCompressedFile() - "), NO_THROW);
		return 0;
	}
}
