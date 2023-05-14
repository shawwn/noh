// (C)2005 S2 Games
// c_archive.h
//
//=============================================================================
#ifndef __C_ARCHIVE_H__
#define __C_ARCHIVE_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_file.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CZip;
class CMMapUnzip;
class CCompressedFile;
class CChecksumTable;

enum EArchiveMode
{
    ARCHIVE_READ = 1,
    ARCHIVE_WRITE = 2,
    ARCHIVE_TRUNCATE = 4,
    ARCHIVE_APPEND = 8,
    ARCHIVE_NO_COMPRESS = 16,
    ARCHIVE_MAX_COMPRESS = 32,
    ARCHIVE_SPEED_COMPRESS = 64,
};
//=============================================================================

//=============================================================================
// CArchive
// Interface to .s2z archive files, provides handles to it's internal files
//=============================================================================
class K2_API CArchive
{
private:
    tstring         m_sPath;
    tstring         m_sPathToArchive;
    tstring         m_sMod;
    tstring         m_sBasePath;
    tstring         m_sCompleteDiskPath;
    int             m_iMode = 0;
    CZip*           m_pZipFile = nullptr;
    CMMapUnzip*     m_pUnzipFile = nullptr;
    CChecksumTable* m_pChecksums = nullptr;
    bool            m_bRequireChecksums = false;

    bool            ValidateFileChecksum(const tstring &sPath);

    // public static members.
public:
    static bool     ExamineChecksums;

public:
    CArchive(); // TKTK NOTE: Don't use = default constructors! It causes symbol not found errors in dylib imports.
    CArchive(const tstring &sPath, int iMode = ARCHIVE_READ);
    ~CArchive();

    bool            Open(const tstring &sPath, int iMode = ARCHIVE_READ, const tstring &sMod = TSNULL);
    bool            Close(uint uiMaxTime = INVALID_TIME);
    bool            CancelWrite();
    bool            IsOpen() const;

    void            StopFilePreload(const tstring &sFilename);
    bool            ContainsFile(const tstring &sPath);
    void            GetFileList(tsvector &vFileList) const;
    bool            GetModifiedFilesList(tsvector &vFileList);

    int             ReadFile(const tstring &sPath, char *&pBuffer);
    bool            WriteFile(const tstring &sPath, const char *pBuffer, size_t size, int iMode = -1, time_t t = 0);

    int             GetCompressedFile(const tstring &sPath, CCompressedFile &cFile);
    bool            WriteCompressedFile(const tstring &sPath, const CCompressedFile &cFile, time_t t = 0);

    const tstring&  GetPath() const                     { return m_sPath; }
    const tstring&  GetPathToArchive() const            { return m_sPathToArchive; }
    const tstring&  GetBasePath() const                 { return m_sBasePath; }
    const tstring&  GetCompleteDiskPath() const         { return m_sCompleteDiskPath; }

    const tstring&  GetMod() const                      { return m_sMod; }
    void            SetMod(const tstring &sMod)         { m_sMod = sMod; }

    void            SetPath(const tstring &sPath)       { m_sPath = sPath; }

    bool            operator==(const tstring &s) const  { return m_sPath == s; }
    bool            operator!=(const tstring &s) const  { return m_sPath != s; }

    float           GetWriteProgress() const;
    bool            DeleteCompressedFile(const tstring &sPath);

    const tsvector& GetFileList() const;
    bool            HasChecksums() const                { return (m_pChecksums != nullptr); }
    bool            ComputeChecksums(CChecksumTable &cChecksums, const tsvector &vFileList);

    // compute the checksum of all checksums (determines whether the client has loaded any files behind our back)
    bool            HashChecksums(byte* pOutChecksum);

    bool            GetRequireChecksums() const         { return m_bRequireChecksums; }
    void            SetRequireChecksums(bool bSet)      { m_bRequireChecksums = true; }
};
//=============================================================================

/*====================
  operator==
  ====================*/
inline
bool    operator==(const CArchive *pArchive, const tstring &s)
{
    return pArchive->GetPath() == s;
}
//=============================================================================

#endif //__C_ARCHIVE_H__
