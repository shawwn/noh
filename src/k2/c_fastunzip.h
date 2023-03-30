// (C)2007 S2 Games
// c_fastunzip.h
// UTTAR
//=============================================================================
#ifndef __C_FASTUNZIP_H__
#define __C_FASTUNZIP_H__

//=============================================================================
// Headers
//=============================================================================
#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <aio.h>
#endif
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
// This is required for a proper fread, but lowers performance very slightly.
// The following structures are read directly from the zip/s2z
//-----------------------------------------------------------------------------
#ifdef _WIN32
#pragma pack(push, ali1, 1)
#else
#pragma pack(push, 1)
#endif
//-----------------------------------------------------------------------------
typedef struct archiveLocalInfo_s
{
	unsigned int signature;
	unsigned short minVersion;
	unsigned short generalFlags;
	unsigned short compression;
	unsigned short lastModTime;
	unsigned short lastModDate;
	unsigned int crc32;
	unsigned int compressed;
	unsigned int uncompressed;
	unsigned short filenameLength;
	unsigned short extraLength;
} archiveLocalInfo_t;
//-----------------------------------------------------------------------------
typedef struct archiveCentralInfo_s
{
	unsigned int signature;
	unsigned short diskNum;
	unsigned short diskNum2;
	unsigned short numEntries;
	unsigned short numEntries2;
	unsigned int centralDirSize;
	unsigned int centralDirOffset;
	unsigned short commentLength;
} archiveCentralInfo_t;
//-----------------------------------------------------------------------------
typedef struct archiveFileInfo_s
{
	unsigned int signature;
	unsigned short version;
	unsigned short version2;
	unsigned short bitflags;
	unsigned short compression;
	unsigned short lastmodTime;
	unsigned short lastmodDate;
	unsigned int crc32;
	unsigned int compressed;
	unsigned int uncompressed;
	unsigned short filenameLength;
	unsigned short extraLength;
	unsigned short commentLength;
	unsigned short diskNumberStart;
	unsigned short internalAttrib;
	unsigned int externalAttrib;
	unsigned int relativeOffset;
} archiveFileInfo_t;
//-----------------------------------------------------------------------------
#ifdef _WIN32
#pragma pack(pop, ali1)
#else
#pragma pack(pop)
#endif

// This structure is what's referenced in our hashmap
struct SZippedFile
{
	// Both of these members are dynamically allocated
	tstring sFileName;
	byte *buf;

	// Generic not-so-file-specific data
	bool compressed;
	bool preload;
	bool loading;
	// loaded=2 if it has already been loaded once
	byte loaded;

	// This includes all the data needed to inflate
	unsigned int atPos;
	unsigned int size;
	unsigned int rawSize;
	unsigned int adler;
};

typedef vector<SZippedFile>			ZFVector;
typedef ZFVector::iterator			ZFVector_it;
typedef ZFVector::const_iterator	ZFVector_cit;
typedef ZFVector::reverse_iterator	ZFVector_rit;

typedef hash_map<tstring, SZippedFile*> ZFMap;
typedef pair<tstring, SZippedFile*>		ZFMap_pair;
typedef ZFMap::iterator					ZFMap_it;
typedef ZFMap::const_iterator			ZFMap_cit;

#define MAX_KEEPINMEM_SIZE	0
//=============================================================================

//=============================================================================
// CFastUnzip
//=============================================================================
class CFastUnzip
{
private:
	// Here's our hashmap and the associated private function to add files to it
	ZFVector	m_vFiles;
	ZFMap		m_mapFiles;
	void AddZippedFile(const tstring &sFileName, archiveFileInfo_s* info, unsigned int atPos);

	// The Operating System-specific variables and functions because HANDLE is used under Windows...
	#ifdef _WIN32
	HANDLE			file;
	HANDLE			fileAsynch;
	OVERLAPPED		asynchInfo;
	HANDLE			threadHandle;
	uint			SearchCentralDir(HANDLE hFileIn);
	#else
	FILE*			file;
	struct aiocb	asynchInfo;
	uint			SearchCentralDir(FILE *pFileIn);
	#endif

	// And here's the various stuff we need to make preloading work
	ZFVector_it		preloadIter;
	ZFVector_it	preloadIterThread;
	uint	numToPreload2;
	uint	numToPreload;
	bool	preloading;
	tstring path;

public:
	CFastUnzip(const tstring &sPath);
	~CFastUnzip();

	#ifdef _WIN32
	// This function is called by FastUnzipAsynchIOThread
	void		FastUnzipAsynchIO();
	#endif

	// Preloading-specific functions affecting all files
	void		UnloadAll();
	void		PreloadAll();
	void		StopPreload();
	bool		PreloadFrame();

	// Check if a specific file exists, or prevent it from being preloaded, etc.
	void		GetFileList(svector &vFileList);
	bool		FileExists(const tstring &sFilename);
	void		StopFilePreload(const tstring &sFilename);

	// Possibly the most important function of them all: the one which actually opens files!
	uint		OpenUnzipFile(const tstring &sFilename, char *&pBuffer);
};
//=============================================================================

#endif //__C_FASTUNZIP_H__
