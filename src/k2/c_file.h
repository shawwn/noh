// (C)2005 S2 Games
// c_file.h
//
//=============================================================================
#ifndef __C_FILE_H__
#define __C_FILE_H__

//=============================================================================
// Definitions
//=============================================================================
enum EFileType
{
	FILE_INVALID = 0,
	FILE_DISK,		// A regular file on the hard drive
	FILE_ARCHIVE,	// A file located inside an s2z archive
	FILE_HTTP,		// A file that is being retrieved from a remote url
	FILE_COMPAT,	// A file overridden by the compatibility archive

	NUM_FILE_TYPES
};

enum ESeekOrigin
{
	SEEK_ORIGIN_START = 0,
	SEEK_ORIGIN_END,
	SEEK_ORIGIN_CURRENT
};

const int FILE_READ				(BIT(0));
const int FILE_WRITE			(BIT(1));
const int FILE_TRUNCATE			(BIT(2));
const int FILE_APPEND			(BIT(3));
const int FILE_BUFFER			(BIT(4));
const int FILE_NOBUFFER			(BIT(5));
const int FILE_BINARY			(BIT(6));
const int FILE_TEXT				(BIT(7));
const int FILE_CREATE			(BIT(8));
const int FILE_BLOCK			(BIT(9));
const int FILE_NOBLOCK			(BIT(10));
const int FILE_ASCII			(BIT(11));
const int FILE_UTF8				(BIT(12));
const int FILE_UTF16			(BIT(13));
const int FILE_UTF32			(BIT(14));
const int FILE_LITTLE_ENDIAN	(BIT(15));
const int FILE_BIG_ENDIAN		(BIT(16));
const int FILE_HTTP_WRITETOFILE	(BIT(17));
const int FILE_HTTP_RESUME		(BIT(18));
const int FILE_NOUSERDIR		(BIT(19));
const int FILE_NOARCHIVES		(BIT(20));
const int FILE_TOPMODONLY		(BIT(21));
const int FILE_HTTP_UPLOAD		(BIT(22));
const int FILE_HTTP_GETSIZE		(BIT(23));
const int FILE_NOWORLDARCHIVE	(BIT(25));
const int FILE_COMPRESS			(BIT(26));
const int FILE_TEST				(BIT(27)); // Test for existance and silently fail if the file doesn't exist
const int FILE_FTP_ACTIVE		(BIT(28));
const int FILE_NOCOMPAT			(BIT(29));
const int FILE_ALLOW_CUSTOM		(BIT(30));

const int FILE_TEXT_ENCODING_MASK	(FILE_ASCII | FILE_UTF8 | FILE_UTF16 | FILE_UTF32);
const int FILE_ENDIAN_MASK			(FILE_LITTLE_ENDIAN | FILE_BIG_ENDIAN);

#ifdef UNICODE
#if defined(linux) || defined(__APPLE__)
const int FILE_DEFAULT_TEXT_ENCODING	(FILE_UTF8);
#else
const int FILE_DEFAULT_TEXT_ENCODING	(FILE_UTF16 | FILE_LITTLE_ENDIAN);
#endif
#else
const int FILE_DEFAULT_TEXT_ENCODING	(FILE_ASCII);
#endif

#ifdef linux
#define _S_IFDIR S_IFDIR
#endif
#if defined(linux) || defined(__APPLE__)
#include <sys/stat.h>
#define _stat stat
#endif
//=============================================================================

//=============================================================================
// CFile
// Base class for different file types
//=============================================================================
class CFile
{
private:

protected:
	char*					m_pBuffer;
	int						m_iMode;
	uint					m_uiSize;
	mutable uint			m_uiPos;				// for the const read functions
	mutable bool			m_bEOF;
	tstring					m_sPath;
	tstring					m_sGamePath;

public:
	K2_API	CFile();
	K2_API	virtual ~CFile();

	virtual bool		Open(const tstring &sPath, int iMode) = 0;
	virtual void		Close() = 0;
	virtual	bool		IsOpen() const = 0;
	K2_API	bool		IsEOF()	const							{ return m_bEOF; };

	K2_API	const tstring&	GetPath() const						{ return m_sPath; }

	virtual tstring		ReadLine() = 0;
	virtual char		Peek(uint uiPos);
	virtual uint		Read(char *pBuffer, uint uiBufferSize) const = 0;
	virtual size_t		Write(const void *pBuffer, size_t iBufferSize) = 0;

	K2_API	virtual const char*	GetBuffer(uint &uiSize) = 0;

	K2_API	byte				ReadByte();
	K2_API	short				ReadInt16(bool bUseBigEndian = false);
	K2_API	int					ReadInt32(bool bUseBigEndian = false);
	K2_API	LONGLONG			ReadInt64(bool bUseBigEndian = false);
	K2_API	float				ReadFloat(bool bUseBigEndian = false);

	K2_API	uint				GetBufferSize()							{ return m_uiSize; }
	K2_API	uint				GetUnreadLength()						{ return m_uiSize - m_uiPos; }

	virtual uint		Tell() const							{ return m_uiPos; }
	virtual bool		Seek(uint uiOffset, ESeekOrigin eOrigin = SEEK_ORIGIN_START);
	virtual size_t		GetLength() const						{ return size_t(m_uiSize); }

	virtual bool		WriteByte(char c) = 0;
	virtual bool		WriteInt16(short c, bool bUseBigEndian) = 0;
	virtual bool		WriteInt32(int c, bool bUseBigEndian) = 0;
	virtual bool		WriteInt64(LONGLONG c, bool bUseBigEndian) = 0;
	virtual bool		WriteString(const string &sText) = 0;
	virtual bool		WriteString(const wstring &sText) = 0;

	virtual void		Flush() {}

	virtual void		SetModificationTime(time_t tModTime) {}
};
//=============================================================================

#endif //__C_FILE_H__
