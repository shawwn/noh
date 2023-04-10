// (C)2008 S2 Games
// c_mmapunzip.h
// Memory mapped based unzipper
//=============================================================================
#ifndef __C_MMAPUNZIP_H__
#define __C_MMAPUNZIP_H__

//=============================================================================
// Headers
//=============================================================================
#ifdef _WIN32
#include "k2_include_windows.h"
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
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

struct SArchiveLocalInfo
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
};

struct SArchiveCentralInfo
{
    unsigned int signature;
    unsigned short diskNum;
    unsigned short diskNum2;
    unsigned short numEntries;
    unsigned short numEntries2;
    unsigned int centralDirSize;
    unsigned int centralDirOffset;
    unsigned short commentLength;
};

struct SArchiveFileInfo
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
};

#ifdef _WIN32
#pragma pack(pop, ali1)
#else
#pragma pack(pop)
#endif

// This structure is what's referenced in our hashmap
struct SZippedFile
{
    uint        uiPos;
    bool        bCompressed;
    uint        uiSize;
    uint        uiRawSize;
    uint        uiCRC32;
};

typedef vector<SZippedFile>         ZFVector;
typedef ZFVector::iterator          ZFVector_it;
typedef ZFVector::const_iterator    ZFVector_cit;
typedef ZFVector::reverse_iterator  ZFVector_rit;

typedef hash_map<tstring, SZippedFile*> ZFMap;
typedef pair<tstring, SZippedFile*>     ZFMap_pair;
typedef ZFMap::iterator                 ZFMap_it;
typedef ZFMap::const_iterator           ZFMap_cit;

#define MAX_KEEPINMEM_SIZE  0

class CCompressedFile;
//=============================================================================

//=============================================================================
// CMMapUnzip
//=============================================================================
class CMMapUnzip
{
private:
    // Here's our hashmap and the associated private function to add files to it
    ZFMap       m_mapFiles;
    tsvector    m_vFileNames;

    // The Operating System-specific variables and functions because HANDLE is used under Windows...
    #ifdef _WIN32
    HANDLE          m_hFile;
    HANDLE          m_hMappedFile;
    #else
    int             m_iFile;
    #endif

    const char*     m_pData; // NULL for "small mapped" resource archived
    uint            m_uiSize;

    bool            m_bMemory;
    bool            m_bInitialized;

    void    Initialize();
    uint    SearchCentralDir(SArchiveCentralInfo *pCentralInfo);
    void    AddZippedFile(const tstring &sFilename, SArchiveFileInfo* info);

public:
    CMMapUnzip(const tstring &sPath);
    CMMapUnzip(const char *pBuffer, uint uiSize);
    ~CMMapUnzip();

    // Check if a specific file exists, or prevent it from being preloaded, etc.
    inline const tsvector&  GetFileList()                           { return m_vFileNames; }
    inline bool             FileExists(const tstring &sFilename)    { return m_mapFiles.find(LowerString(sFilename)) != m_mapFiles.end(); }

    // Possibly the most important function of them all: the one which actually opens files!
    uint        OpenUnzipFile(const tstring &sFilename, char *&pBuffer);

    uint        GetCompressedFile(const tstring &sFilename, CCompressedFile &cFile);
};
//=============================================================================

#endif //__C_MMAPUNZIP_H__
