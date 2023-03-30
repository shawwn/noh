// (C)2005 S2 Games
// c_zipfile.h
//
// file io functions
//=============================================================================
#ifndef __C_ZIPFILE_H__
#define __C_ZIPFILE_H__

//=============================================================================
// Headers
//=============================================================================
#include <zlib.h>
//=============================================================================

#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)
#define BUFREADCOMMENT (0x400)

#define UNZ_OK                  (0)
#define UNZ_END_OF_LIST_OF_FILE (-100)
#define UNZ_ERRNO               (Z_ERRNO)
#define UNZ_EOF                 (0)
#define UNZ_PARAMERROR          (-102)
#define UNZ_BADZIPFILE          (-103)
#define UNZ_INTERNALERROR       (-104)
#define UNZ_CRCERROR            (-105)

#define ZIP_OK                  (0)
#define ZIP_ERRNO               (Z_ERRNO)
#define ZIP_PARAMERROR          (-102)
#define ZIP_INTERNALERROR       (-104)

struct SZipTime
{
    uInt tm_sec;    // seconds after the minute - [0,59]
    uInt tm_min;    // minutes after the hour - [0,59]
    uInt tm_hour;   // hours since midnight - [0,23]
    uInt tm_mday;   // day of the month - [1,31]
    uInt tm_mon;    // months since January - [0, 11]
    uInt tm_year;   // years - [1980..2044]
};

typedef struct zipFileInfo_s
{
    uLong version;              // version made by                 2 bytes
    uLong versionNeeded;        // version needed to extract       2 bytes
    uLong flag;                 // general purpose bit flag        2 bytes
    uLong compressionMethod;    // compression method              2 bytes
    uLong dosDate;              // last mod file date in Dos fmt   4 bytes
    uLong crc;                  // crc - 32                          4 bytes
    uLong compressedSize;       // compressed size                 4 bytes
    uLong uncompressedSize;     // uncompressed size               4 bytes
    uLong sizeFilename;         // filename length                 2 bytes
    uLong sizeFileExtra;        // extra field length              2 bytes
    uLong sizeFileComment;      // file comment length             2 bytes

    uLong diskNumStart;       // disk number start               2 bytes
    uLong internalFA;          // internal file attributes        2 bytes
    uLong externalFA;          // external file attributes        4 bytes

    SZipTime timeInfo;
}
zipFileInfo_t;

typedef struct unz_file_info_internal_s
{
    uLong offsetCurfile;    // relative offset of local header 4 bytes
}
zipFileInfoInternal_t;  // unz_file_info_internal;

// zipGlobalInfo_t structure contain global data about the ZIPfile
// These data comes from the end of central dir
typedef struct zipFileGlobalInfo_s
{
    uLong numberEntry;  // total number of entries in the central dir on this disk
    uLong sizeComment;  // size of the global comment of the zipfile
}
zipFileGlobalInfo_t;

// file_in_zip_read_info_s contain internal information about a file in zipfile,
// when reading and decompress it
struct SZipFileInfo
{
    char *preadBuffer;              // internal buffer for compressed data
    z_stream stream;                // zLib stream structure for inflate
    uLong posInZipfile;             // position in byte on the zipfile, for fseek
    uLong streamInitialised;        // flag set if stream structure is initialised
    uLong offsetLocalExtrafield;    // offset of the local extra field
    uInt sizeLocalExtrafield;       // size of the local extra field
    uLong posLocalExtrafield;       // position in the local extra field in read
    uLong crc32;                    // crc32 of all data uncompressed
    uLong crc32Wait;                // crc32 we must obtain after decompress all
    uLong restReadCompressed;       // number of byte to be decompressed
    uLong restReadUncompressed;     //number of byte to be obtained after decomp
    FILE *pfile;                    // io structore of the zipfile
    uLong compressionMethod;        // compression method (0==store)
    uLong byteBefore;               // byte before the zipfile, (>0 for sfx)
};

typedef hash_map<tstring, uLong> FilenameMap;

// zipFileInternalInfo_s contain internal information about the zipfile
struct SZipFile
{
    FILE* pfile;                                    // io structore of the zipfile
    zipFileGlobalInfo_t globalInfo;                 // public global information
    uLong byteBefore;                               // byte before the zipfile, (>0 for sfx)
    uLong fileNum;                                  // number of the current file in the zipfile
    uLong posCentralDir;                            // pos of the current file in the central dir
    uLong fileOk;                                   // flag about the usability of the current file
    uLong centralPos;                               // position of the beginning of the central dir
    uLong sizeCentralDir;                           // size of the central directory
    uLong offsetCentralDir;                         // offset of start of central directory with
                                                    // respect to the starting disk number
    zipFileInfo_t curFileInfo;                      // public info about the current file in zip
    zipFileInfoInternal_t curFileInfoInternal;      // private info about it
    SZipFileInfo*   pfileInZipRead;                 // structure about the current
                                                    //  file if we are decompressing it
    FilenameMap*    pFilenameMap;
};

//=============================================================================
// CZipFile
//=============================================================================
class CZipFile
{
private:
    void DosDateToTmuDate (uLong ulDosDate, SZipTime *pTime);

protected:
    tstring     m_sPath;
    SZipFile*   m_pzipFile;
    int         m_iFlags;

    int GetByte (FILE *pFileIn, int *iPi);
    int GetShort (FILE *pFileIn, uLong *pX);
    int GetLong (FILE *pFileIn, uLong *pX);
    int GetCurrentFileInfo (zipFileInfo_t *pFileInfo, TCHAR *szFilename, uLong ulFilenameBufferSize,
        void *pExtraField, uLong ulExtraFieldBufferSize, char *szComment, uLong ulCommentBufferSize);
    int GetCurrentFileInfoInternal (zipFileInfo_t *pFileInfo, zipFileInfoInternal_t *pFileInfoInternal,
        TCHAR *szFilename, uLong ulFilenameBufferSize, void *pExtraField, uLong ulExtraFieldBufferSize,
        char *szComment, uLong ulCommentBufferSize);
    //static bool stat (SZipFile *archive, const char *filename, struct stat *stats);

public:
    virtual ~CZipFile()     {}

    virtual bool    IsOpen() const = 0;
};
//=============================================================================
#endif
