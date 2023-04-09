// (C)2005 S2 Games
// c_filehandle.cpp
//=============================================================================


//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_filemanager.h"
#include "c_filehandle.h"
#include "c_filearchive.h"
#include "c_filedisk.h"
#include "c_filehttp.h"
//=============================================================================


/*====================
  CFileHandle::CFileHandle
  ====================*/
CFileHandle::CFileHandle() :
m_pFile(NULL)
{
}


/*====================
  CFileHandle::CFileHandle
  ====================*/
CFileHandle::CFileHandle(const tstring &sPath, int iMode, const tstring &sMod) :
m_pFile(NULL)
{
    Open(sPath, iMode, sMod);
}


/*====================
  CFileHandle::CFileHandle
  ====================*/
CFileHandle::CFileHandle(const tstring &sPath, int iMode, CArchive& archive) :
m_pFile(NULL)
{
    Open(sPath, iMode, archive);
}


/*====================
  CFileHandle::~CFileHandle
  ====================*/
CFileHandle::~CFileHandle()
{
    if (m_pFile)
        m_pFile->Close();
    SAFE_DELETE(m_pFile);
}


/*====================
  CFileHandle::Open

  Interprets sPath to determine what type of file to open, then allocates
  a CFileHandle* child class of the apropriate type and attempts to open it
  ====================*/
bool    CFileHandle::Open(const tstring &sFilePath, int iMode, const tstring &sMod)
{
    tstring sPath(FileManager.SanitizePath(sFilePath));

    if (IsOpen())
    {
        Console.Warn << _T("Open file handle tried to open a new file") << newl;
        return false;
    }

    bool bBadModeFlags(false);
    if (iMode & FILE_TEXT &&
        (iMode & FILE_TEXT_ENCODING_MASK) != FILE_ASCII &&
        (iMode & FILE_TEXT_ENCODING_MASK) != FILE_UTF8 &&
        (iMode & FILE_TEXT_ENCODING_MASK) != FILE_UTF16 &&
        (iMode & FILE_TEXT_ENCODING_MASK) != FILE_UTF32 &&
        (iMode & FILE_TEXT_ENCODING_MASK) != 0)
        bBadModeFlags = true;

    // Check for conflicting mode flags
    if (((iMode & FILE_READ) && (iMode & FILE_WRITE)) ||
        ((iMode & FILE_TEXT) &&  (iMode & FILE_BINARY)) ||
        ((iMode & FILE_APPEND) && (iMode & FILE_TRUNCATE)) ||
        ((iMode & FILE_BUFFER) && (iMode & FILE_NOBUFFER)) ||
        ((iMode & FILE_BLOCK) && (iMode & FILE_NOBLOCK)))
        bBadModeFlags = true;

    if (bBadModeFlags)
    {
        Console.Warn << _T("Conflicting mode flags opening file: ") << sPath << newl;
        return false;
    }

    m_pFile = FileManager.GetFile(sPath, iMode, sMod);
    return IsOpen();
}


/*====================
  CFileHandle::Open

  If the file is already known to be in an archive, or only a file in a specific
  archive is desired to be opened, this skips the registered archives list and
  disk check and opens directly from the specified archive.

  Also, this is the only method available to open a file with write access
  inside of an archive.
  ====================*/
bool    CFileHandle::Open(const tstring &sFilePath, int iMode, CArchive &hArchive)
{
    if (IsOpen())
    {
        Console.Warn << _T("Open file handle tried to open a new file") << newl;
        return false;
    }

    if (!hArchive.IsOpen() || sFilePath.empty())
        return false;

    // check for conflicting mode flags
    if (((iMode & FILE_READ) && (iMode & FILE_WRITE)) ||
        ((iMode & FILE_TEXT) &&  (iMode & FILE_BINARY)) ||
        ((iMode & FILE_APPEND) && (iMode & FILE_TRUNCATE)) ||
        ((iMode & FILE_BUFFER) && (iMode & FILE_NOBUFFER)) ||
        ((iMode & FILE_BLOCK) && (iMode & FILE_NOBLOCK)))
    {
        Console.Warn << _T("Conflicting mode flags opening file ") << sFilePath << newl;
        return false;
    }

    if (iMode & FILE_WRITE)
    {
        m_pFile = K2_NEW(ctx_FileSystem,  CFileArchive)(&hArchive);

        tstring sPath(hArchive.GetPath());
        sPath = Filename_GetPath(sPath) + sFilePath;

        if (!m_pFile->Open(sPath, iMode))
        {
            Close();
            return false;
        }
    }
    else
    {
        {
            tstring sAbsolutePath(FileManager.IsCleanPath(sFilePath, false) ? sFilePath : FileManager.SanitizePath(sFilePath, false));
            tstring sPath(Filename_StripExtension(hArchive.GetPath()) + _T("/") + sAbsolutePath);
            if (Open(sPath, iMode, hArchive.GetMod()))
                return true;
        }

        if (!FileManager.GetCompatVersion().empty())
        {
            bool bDeleted(false);

            // Strip leading bits
            tstring sPath(hArchive.GetBasePath());
            if (sPath.size() >= 2 && sPath[0] == _T(':') && sPath[1] == _T('/'))
                sPath = sPath.substr(2);
            else if (sPath.size() >= 1 && sPath[0] == _T('/'))
                sPath = sPath.substr(1);

            if (sFilePath[0] == _T('/'))
                sPath += sFilePath;
            else
                sPath += _T("/") + sFilePath;

            m_pFile = FileManager.GetCompatFile(sPath, iMode, bDeleted);
            
            if (bDeleted)
            {
                Close();
                return false;
            }
        }

        if (m_pFile == NULL)
        {
            m_pFile = K2_NEW(ctx_FileSystem,  CFileArchive)(&hArchive);

            tstring sPath(hArchive.GetPath());
            sPath = Filename_GetPath(sPath) + sFilePath;

            if (!m_pFile->Open(sPath, iMode))
            {
                Close();
                return false;
            }
        }
    }

    return true;
}


/*====================
  CFileHandle::Close
  ====================*/
void    CFileHandle::Close()
{
    if (m_pFile != NULL)
        m_pFile->Close();
    SAFE_DELETE(m_pFile);
}


/*====================
  CFileHandle::IsOpen
  ====================*/
bool    CFileHandle::IsOpen() const
{
    if (m_pFile == NULL)
        return false;
    return m_pFile->IsOpen();
}


/*====================
  CFileHandle::IsEOF
  ====================*/
bool    CFileHandle::IsEOF() const
{
    if (m_pFile == NULL)
        return true;
    return m_pFile->IsEOF();
}


/*====================
  CFileHandle::ReadLine
  ====================*/
tstring CFileHandle::ReadLine()
{
    if (m_pFile == NULL)
        return 0;
    return m_pFile->ReadLine();
}


/*====================
  CFileHandle::WriteString
  ====================*/
bool    CFileHandle::WriteString(const string &sText)
{
    if (m_pFile)
        return m_pFile->WriteString(sText);
    else
        return false;
}

bool    CFileHandle::WriteString(const wstring &sText)
{
    if (m_pFile)
        return m_pFile->WriteString(sText);
    else
        return false;
}


/*====================
  CFileHandle::Read
  ====================*/
int CFileHandle::Read(char *pBuffer, int iSize) const
{
    if (m_pFile)
        return m_pFile->Read(pBuffer, iSize);
    else
        return 0;
}


/*====================
  CFileHandle::Write
  ====================*/
size_t  CFileHandle::Write(const void *pBuffer, size_t size)
{
    if (m_pFile)
        return m_pFile->Write(pBuffer, size);
    else
        return 0;
}


/*====================
  CFileHandle::GetBuffer
  ====================*/
const char* CFileHandle::GetBuffer(uint &uiSize)
{
    if (m_pFile)
        return m_pFile->GetBuffer(uiSize);
    else
        return NULL;
}


/*====================
  CFileHandle::GetPath
  ====================*/
const tstring&  CFileHandle::GetPath() const
{
    if (m_pFile)
        return m_pFile->GetPath();
    else
        return TSNULL;
}


/*====================
  CFileHandle::ReadByte
  ====================*/
byte    CFileHandle::ReadByte()
{
    if (m_pFile)
        return m_pFile->ReadByte();
    else
        return 0;
}


/*====================
  CFileHandle::ReadInt16
  ====================*/
short   CFileHandle::ReadInt16(bool bUseBigEndian)
{
    if (m_pFile)
        return m_pFile->ReadInt16(bUseBigEndian);
    else
        return 0;
}


/*====================
  CFileHandle::ReadInt32
  ====================*/
int     CFileHandle::ReadInt32(bool bUseBigEndian)
{
    if (m_pFile == NULL)
        return 0;
    return m_pFile->ReadInt32(bUseBigEndian);
}


/*====================
  CFileHandle::ReadInt64
  ====================*/
LONGLONG    CFileHandle::ReadInt64(bool bUseBigEndian)
{
    if (m_pFile == NULL)
        return 0;
    return m_pFile->ReadInt64(bUseBigEndian);
}



/*====================
  CFileHandle::ReadFloat
  ====================*/
float       CFileHandle::ReadFloat(bool bUseBigEndian)
{
    if (m_pFile)
        return m_pFile->ReadFloat(bUseBigEndian);
    else
        return 0;
}


/*====================
  CFileHandle::Tell
  ====================*/
size_t  CFileHandle::Tell()
{
    if (m_pFile)
        return m_pFile->Tell();
    else
        return 0;
}


/*====================
  CFileHandle::Seek
  ====================*/
bool    CFileHandle::Seek(int iOffset, ESeekOrigin eOrigin)
{
    if (m_pFile == NULL)
        return false;
    return m_pFile->Seek(iOffset, eOrigin);
}


/*====================
  CFileHandle::GetLength
  ====================*/
size_t  CFileHandle::GetLength()
{
    if (m_pFile)
        return m_pFile->GetLength();
    else
        return 0;
}


/*====================
  CFileHandle::WriteByte
  ====================*/
bool    CFileHandle::WriteByte(char c)
{
    if (m_pFile == NULL)
        return false;
    return m_pFile->WriteByte(c);
}


/*====================
  CFileHandle::ReadInt16
  ====================*/
bool    CFileHandle::WriteInt16(short c, bool bUseBigEndian)
{
    if (m_pFile == NULL)
        return false;
    return m_pFile->WriteInt16(c, bUseBigEndian);
}


/*====================
  CFileHandle::WriteInt32
  ====================*/
bool    CFileHandle::WriteInt32(int c, bool bUseBigEndian)
{
    if (m_pFile == NULL)
        return false;
    return m_pFile->WriteInt32(c, bUseBigEndian);
}


/*====================
  CFileHandle::WriteInt64
  ====================*/
bool    CFileHandle::WriteInt64(LONGLONG ll, bool bUseBigEndian)
{
    if (m_pFile == NULL)
        return false;
    return m_pFile->WriteInt64(ll, bUseBigEndian);
}


/*====================
  CFileHandle::Flush
  ====================*/
void    CFileHandle::Flush()
{
    if (m_pFile == NULL)
        return;
    m_pFile->Flush();
}


/*====================
  CFileHandle::ReadWString
  ====================*/
wstring CFileHandle::ReadWString()
{
    if (m_pFile == NULL)
        return wstring();

    wstring sStr;

    wchar_t wChar(m_pFile->ReadInt16());
    while (wChar)
    {
        sStr += wChar;
        wChar = m_pFile->ReadInt16();
    }

    return sStr;
}


/*====================
  CFileHandle::SetModificationTime
  ====================*/
void    CFileHandle::SetModificationTime(time_t tModTime)
{
    if (m_pFile == NULL)
        return;
    m_pFile->SetModificationTime(tModTime);
}

