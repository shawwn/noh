// (C)2005 S2 Games
// c_unzip.h
//
// support for unzipping .zip files - based on the zlib minizip package
//=============================================================================
#ifndef __C_UNZIP_H__
#define __C_UNZIP_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_zipfile.h"
//=============================================================================

//=============================================================================
// CUnzip
//=============================================================================
class CUnzip : public CZipFile
{
public:
	inline static int CloseCurrentFile(SZipFile *pFile);
	inline static int Cof(SZipFile *pFile);
	inline static int ReadCurrentFile(SZipFile *pFile, voidp pBuf, uint iLen);

	bool	IsOpen() const	{ return m_pzipFile != NULL; }

private:
	int		GoToFirstFile();
	int		GoToNextFile();
	uLong	SearchCentralDir(FILE *pFileIn);
	int		CheckCurrentFileCoherencyHeader(uint *piSizeVar, uLong *pOffsetLocalExtraField,
		uint *piSizeLocalExtraField);
	int		GetLocalExtraField (voidp pBuf, uint iLen);
	//int		_getGlobalComment (unzFile file, char *szComment, uLong uSizeBuf);
	int		StringFilenameCompare (const tstring &sFilename1, const tstring &sFilename2, int iCaseSensitivity);
	int		OpenCurrentFile();
	//int		_locateFile (string szFilename, int iCaseSensitivity);

public:
	CUnzip(const tstring &sPath);
	~CUnzip();

	SZipFile*			Open(const tstring &sPath);
	int					Close ();
	bool				FileExists (const tstring &sPath);
	/*void				zipSystemDir (void *archive, char *_directory, char *wildcard, bool recurse,
		void(*dirCallback)(const char *dir, void *userdata),
		void(*fileCallback)(const char *filename, void *userdata), void *userdata);*/
	uint		OpenUnzipFile (const tstring &sFilename, char *&pBuffer);
};
//=============================================================================
#endif
