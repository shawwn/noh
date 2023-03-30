// (C)2010 S2 Games
// c_filestream.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include <sys/stat.h>

#include "c_filestream.h"

#include "c_exception.h"
#include "c_mmapunzip.h"
#include <zlib.h>

#pragma warning(disable:4996) // fopen is just fine, thank you very much
//=============================================================================

//=============================================================================
// Constants
//=============================================================================
static const uint   K2_FILE_CHUNK(128 * 1024);
//=============================================================================

/*====================
  CFileStream::CFileStream
  ====================*/
CFileStream::CFileStream()
: m_File(NULL)
, m_pBuffer(NULL)
, m_uiReadSize(0)
, m_pZlib(NULL)
, m_uiCompressedOffset(0)
, m_uiCompressedSize(0)
{
}


/*====================
  CFileStream::EnsureData
  ====================*/
bool    CFileStream::EnsureData(uint uiSize) const
{
    if (!IsOpen())
        return false;

    uiSize = CLAMP<uint>(uiSize, 0, m_uiSize);

    while (m_uiReadSize < uiSize)
    {
        if (ferror(m_File))
            return false;

        if (m_pZlib == NULL)
        {
            // Regular file
            uint uiReadCount(uiSize - m_uiReadSize);
            uint ret(INT_SIZE(fread(m_pBuffer + m_uiReadSize, 1, uiReadCount, m_File)));
            if (ferror(m_File))
                return false;

            assert(ret == uiReadCount);
            m_uiReadSize += uiReadCount;
        }
        else
        {
            // Compressed file
            unsigned char in[K2_FILE_CHUNK];

            m_pZlib->avail_in = INT_SIZE(fread(in, 1, K2_FILE_CHUNK, m_File));
            if (ferror(m_File))
            {
                inflateEnd(m_pZlib);
                return false;
            }

            if (m_pZlib->avail_in == 0)
                break;

            m_pZlib->next_in = in;

            do
            {
                m_pZlib->avail_out = K2_FILE_CHUNK;
                m_pZlib->next_out = (byte*)(m_pBuffer + m_uiReadSize);

                int ret(inflate(m_pZlib, Z_SYNC_FLUSH));
                switch (ret)
                {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;
                    // and fall through
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    inflateEnd(m_pZlib);
                    return false;
                }

                uint uiHave(K2_FILE_CHUNK - m_pZlib->avail_out);
                m_uiReadSize += uiHave;

            } while (m_pZlib->avail_out == 0);
        }
    }

    return true;
}


/*====================
  CFileStream::LocateCompressedFile
  ====================*/
FILE*   CFileStream::LocateCompressedFile(const tstring& sZipFilePath, const tstring& sFilePath,
                                               uint& uiCompressedOffset, uint& uiCompressedSize, uint& uiRawSize)
{
    uiCompressedOffset = 0;
    uiCompressedSize = 0;
    uiRawSize = 0;

    // Invalid zip file?
    FILE* fp(tfopen(TStringToNative(sZipFilePath).c_str(), "rb"));
    if (fp != NULL)
    {
        fseek(fp, 0, SEEK_END);
        uint uiZipFileSize(ftell(fp));
        uint uiZipCentralDir(-1);

        // Central directory is at the end of the file
        // right before the global comment. The global
        // comment has a maximum size of 64k

        byte pUnzipBuf[66000];
        uint uiEnd((uint)MAX<int>((int)uiZipFileSize - 66000, 4));
        uint uiSize(uiZipFileSize - uiEnd);
        fseek(fp, uiEnd, SEEK_SET);
        uint uiRead(INT_SIZE(fread(pUnzipBuf, 1, uiSize, fp)));
        assert(uiRead == uiSize);
        int iSize((int)uiSize);

        // Scan for signature
        for (int i(iSize - 1); i >= 3; --i)
        {
            if (pUnzipBuf[i - 0] == 0x06 &&
                pUnzipBuf[i - 1] == 0x05 &&
                pUnzipBuf[i - 2] == 0x4b &&
                pUnzipBuf[i - 3] == 0x50)
            {
                // Signature found; return its position
                uiZipCentralDir = (uint)(uiZipFileSize - iSize + i - 3);
                break;
            }
        }

        if (uiZipCentralDir != (uint)-1)
        {
            fseek(fp, uiZipCentralDir, SEEK_SET);

            SArchiveCentralInfo cCentralInfo;
            if (fread(&cCentralInfo, sizeof(SArchiveCentralInfo), 1, fp) == 1)
            {
                const uint uiCentralInfoSignature(0x06054b50);
                if (LittleInt(cCentralInfo.signature) == uiCentralInfoSignature)
                {
                    // Prepare the various temporary variables for the loop...
                    uint uiPos(LittleInt(cCentralInfo.centralDirOffset));
                    fseek(fp, uiPos, SEEK_SET);

                    string sSearchFilename(TStringToUTF8(sFilePath));
                    char szFilename[1024];

                    while (true)
                    {
                        SArchiveFileInfo cFileInfo;
                        cFileInfo.signature = 0;
                        if (fread(&cFileInfo, sizeof(SArchiveFileInfo), 1, fp) != 1)
                            break;

                        // Check if the signature matches; if not, the file headers are over (normal termination)
                        const uint uiFileInfoSignature(0x02014b50);
                        if (LittleInt(cFileInfo.signature) != uiFileInfoSignature)
                            break;

                        uint uiFilenameLength(MIN<uint>(LittleShort(cFileInfo.filenameLength), 1023));
                        
                        // Early out: different filename length
                        if (uiFilenameLength != sFilePath.size())
                        {
                            fseek(fp, uiFilenameLength, SEEK_CUR);
                        }
                        else
                        {
                            // Copy filename into temporary buffer
                            if (fread(szFilename, 1, uiFilenameLength, fp) != uiFilenameLength)
                                break;
                            szFilename[uiFilenameLength] = 0;

#ifdef _WIN32
                            if (_strnicmp(sSearchFilename.data(), szFilename, uiFilenameLength) == 0)
#else
                            if (strncasecmp(sSearchFilename.data(), szFilename, uiFilenameLength) == 0)
#endif
                            {
                                // Found
                                uiRawSize = LittleInt(cFileInfo.uncompressed);
                                uiCompressedOffset = LittleInt(cFileInfo.relativeOffset) + sizeof(SArchiveLocalInfo) + LittleShort(cFileInfo.filenameLength) + LittleShort(cFileInfo.extraLength);
                                uiCompressedSize = LittleInt(cFileInfo.compressed);
                                fseek(fp, uiCompressedOffset, SEEK_SET);
                                return fp;
                            }
                        }

                        // Advance
                        ushort unExtra(LittleShort(cFileInfo.extraLength));
                        fseek(fp, unExtra, SEEK_CUR);
                    }
                }
            }
        }
    }
    fclose(fp);
    return NULL;
}


/*====================
  CFileStream::AcquireCompressedFile
  ====================*/
bool    CFileStream::AcquireCompressedFile(FILE* fp, uint uiCompressedOffset, uint uiCompressedSize, uint uiRawSize, int iMode)
{
    if (IsOpen())
        return false;

    if ((iMode & FILE_READ) == 0)
    {
        assert(!"CFileStream::AcquireCompressedFile() - does not support writing");
        return false;
    }

    m_File = fp;
    m_iMode = iMode;
    m_uiCompressedOffset = uiCompressedOffset;
    m_uiCompressedSize = uiCompressedSize;

    fseek(m_File, m_uiCompressedOffset, SEEK_SET);

    assert(m_pBuffer == NULL);
    assert(m_pZlib == NULL);
    m_pZlib = new z_stream;
    m_pZlib->zalloc = Z_NULL;
    m_pZlib->zfree = Z_NULL;
    m_pZlib->opaque = Z_NULL;
    m_pZlib->avail_in = 0;
    m_pZlib->next_in = Z_NULL;
    int ret(inflateInit2(m_pZlib, -15));
    if (ret != Z_OK)
    {
        Close();
        return false;
    }

    m_pBuffer = new byte[uiRawSize];
    m_uiSize = uiRawSize;
    m_uiReadSize = 0;
    m_uiPos = 0;

    m_File = fp;
    return true;
}


/*====================
  CFileStream::Open
  ====================*/
bool    CFileStream::Open(const tstring &sPath, int iMode)
{
    m_sPath = sPath;
    m_iMode = iMode;

    // Set the access mode
    if (m_iMode & FILE_READ)
    {
        m_iMode &= ~(FILE_APPEND | FILE_TRUNCATE);
    }
    else
    {
        return false;
    }

    // Always open the stream as binary so that the engine can
    // handle all cases of linebreaks properly
    if (m_iMode & FILE_TEXT)
    {
        if ((m_iMode & FILE_ASCII) ||
            (m_iMode & FILE_UTF8))
            m_iMode &= ~FILE_ENDIAN_MASK;
    }
    else
    {
        m_iMode |= FILE_BINARY;
        m_iMode &= ~(FILE_UTF8 | FILE_UTF16 | FILE_UTF32);
    }

    // open
    m_File = tfopen(TStringToNative(sPath).c_str(), "rb");

    // default behavior is to buffer
    if (!(m_iMode & FILE_NOBUFFER))
        m_iMode |= FILE_BUFFER;

    // default behavior is to block
    if (!(m_iMode & FILE_NOBLOCK))
        m_iMode |= FILE_BLOCK;

    // validate file
    if (m_File == NULL)
        return false;

    // Get size
    if (m_iMode & FILE_READ)
    {
        fseek(m_File, 0, SEEK_END);
        m_uiSize = ftell(m_File);
        fseek(m_File, 0, SEEK_SET);
    }

    m_pBuffer = new byte[m_uiSize];

    if (m_iMode & FILE_READ)
    {
        if (m_iMode & FILE_BUFFER)
            GetBuffer(m_uiSize);

        m_bEOF = false;
    }

    return true;
}


/*====================
  CFileStream::OpenCompressed
  ====================*/
bool    CFileStream::OpenCompressed(const tstring &sZipFile, const tstring &sDirtyPath, int iMode)
{
    if (IsOpen())
        return false;

    tstring sPath;
    size_t uiStart(sDirtyPath.find_first_not_of(_T('/')));
    if (uiStart != tstring::npos)
        sPath = sDirtyPath.substr(uiStart);

    uint uiRawSize(0);
    uint uiCompressedOffset(0);
    uint uiCompressedSize(0);
    FILE* fp(LocateCompressedFile(sZipFile, sPath, uiCompressedOffset, uiCompressedSize, uiRawSize));
    if (fp == NULL)
    {
        Close();
        return false;
    }

    m_sPath = sZipFile;
    m_sPath.append(1, L'/');
    m_sPath.append(sPath);

    if (!AcquireCompressedFile(fp, uiCompressedOffset, uiCompressedSize, uiRawSize, iMode))
    {
        Close();
        return false;
    }

    return true;
}


/*====================
  CFileStream::Close
  ====================*/
void    CFileStream::Close()
{
    SAFE_DELETE_ARRAY(m_pBuffer);

    if (m_pZlib != NULL)
    {
        inflateEnd(m_pZlib);
        SAFE_DELETE(m_pZlib);
    }

    if (m_File != NULL)
    {
        fclose(m_File);
        m_File = NULL;
    }
}


/*====================
  CFileStream::IsOpen
  ====================*/
bool    CFileStream::IsOpen() const
{
    if (m_pBuffer != NULL)
        return true;

    return false;
}


/*====================
  CFileStream::ReadCharacter
  ====================*/
uint    CFileStream::ReadCharacter()
{
    // ASCII
    if (m_iMode & FILE_ASCII)
        return ReadByte();

    // UTF-32
    if (m_iMode & FILE_UTF32)
        return ReadInt32(IsBigEndian());

    // UTF-16
    if (m_iMode & FILE_UTF16)
    {
        uint uiResult(ReadInt16(IsBigEndian()));
        if ((uiResult & 0xfc00) != 0xd800)
            return uiResult;

        uint uiSecond(ReadInt16(IsBigEndian()));
        if ((uiSecond & 0xfc00) != 0xdc00)
            return 0;

        return ((uiResult & 0x03ff) << 10) | (uiSecond & 0x03ff);
    }

    // UTF-8
    char cBuffer[4];
    cBuffer[0] = ReadByte();
    if ((cBuffer[0] & 0x80) == 0)
        return cBuffer[0];

    // Determine how many bytes are in this character
    uint uiMask(0x80);
    uint uiNumBytes(0);
    while (cBuffer[0] & uiMask)
    {
        ++uiNumBytes;
        uiMask >>= 1;
    }

    if (uiNumBytes < 2 || uiNumBytes > 4)
        return 0;

    uint uiBytesRead(Read(&cBuffer[1], uiNumBytes - 1));
    if (uiBytesRead < uiNumBytes - 1)
        return 0;

    // Assign the remaining bits of the first byte
    uiMask -= 1;
    uint uiResult((cBuffer[0] & uiMask) << ((uiNumBytes - 1) * 6));

    // Assemble the remaining bytes
    for (uint ui(1); ui < uiNumBytes; ++ui)
    {
        if (cBuffer[ui] == 0 || (cBuffer[ui] & 0xc0) != 0x80)
            return 0;

        uiResult |= (cBuffer[ui] & 0x3f) << ((uiNumBytes - ui - 1) * 6);
    }

    return uiResult;
}


#if 0
/*====================
  CFileStream::ReadLine
  ====================*/
string  CFileStream::ReadLine()
{
    if (m_iMode & FILE_WRITE)
        return SNULL;

    if (m_iMode & FILE_BINARY)
        return SNULL;

    if (m_bEOF)
        return SNULL;


    string sReturn;
    while (!m_bEOF)
    {
        char c(ReadCharacter() & 0xff);
        if (c == 0)
            continue;

        // Check for a line break
        if (c == '\r')
        {
            int iOldPos(Tell());
            char cNext(ReadByte());
            if (cNext != '\n')
                Seek(iOldPos);
            break;
        }

        if (c == '\n')
            break;

        sReturn += c;
    }

    return sReturn;
}


/*====================
  CFileStream::ReadLineW
  ====================*/
tstring CFileStream::ReadLineW()
{
    if (m_iMode & FILE_WRITE)
        return WSNULL;

    if (m_iMode & FILE_BINARY)
        return WSNULL;

    if (m_bEOF)
        return WSNULL;


    tstring sReturn;
    while (!m_bEOF)
    {
        wchar_t c(ReadCharacter() & 0xffff);
        if (c == 0)
            continue;

        // Check for a line break
        if (c == L'\r')
        {
            int iOldPos(Tell());
            wchar_t cNext(ReadCharacter() & 0xffff);
            if (cNext != L'\n')
                Seek(iOldPos);
            break;
        }

        if (c == L'\n')
            break;

        sReturn += c;
    }

    return sReturn;
}
#endif


/*====================
  CFileStream::ReadLine
  ====================*/
tstring CFileStream::ReadLine()
{
    try
    {
        if (m_iMode & FILE_WRITE)
            EX_ERROR(_T("File is open for write access"));

        if (m_iMode & FILE_BINARY)
            EX_ERROR(_T("File is open for binary access"));

        if (m_bEOF)
            return _T("");

        size_t zOutCharSize(sizeof(TCHAR));
        uint uiInCharSize(1);
        if (m_iMode & FILE_UTF16)
            uiInCharSize = 2;
        else if (m_iMode & FILE_UTF32)
            uiInCharSize = 4;

        char aBuffer[6];
        tstring sReturn;
        for (;;)
        {
            // Get the next character
            if (Read(aBuffer, uiInCharSize) < uiInCharSize)
                return sReturn;
            
            // Read additional UTF-8 characters if needed
            if ((m_iMode & FILE_UTF8) && (aBuffer[0] & 0x80))
            {
                // Determine number of bytes to read
                char cMask('\x80');
                int iCount(0);
                while ((aBuffer[0] & cMask) && iCount <= 6)
                {
                    ++iCount;
                    cMask >>= 1;
                }

                if (iCount > 1)
                {
                    Read(&aBuffer[1], iCount - 1);
                }
            }

            // Convert the character
            uint c;
            if (m_iMode & FILE_ASCII)
            {
                c = byte(aBuffer[0]);
            }
            else if (m_iMode & FILE_UTF8)
            {
                if ((aBuffer[0] & 0x80) == 0)
                {
                    // anything in the ASCII equivalent range
                    c = byte(aBuffer[0]);
                }
                else
                {
                    // determine how many bytes are in this character
                    uint mask(0x80);
                    int numbytes = 0;
                    while (aBuffer[0] & mask)
                    {
                        ++numbytes;
                        mask >>= 1;
                    }
                    
                    if (numbytes <= 4)
                    {
                        // read the rest of the bytes
                        if (int(Read(&aBuffer[1], numbytes-1)) < numbytes-1)
                            return sReturn;
                        
                        // assign the remaining bits of the first byte
                        mask -= 1;
                        c = (aBuffer[0] & mask) << ((numbytes - 1) * 6);
                        
                        // assemble the remaining bytes
                        for (int b = 2; b <= numbytes; ++b)
                        {
                            if ((aBuffer[b-1] == '\x00') || (aBuffer[b-1] & 0xc0) != 0x80)
                            {
                                Console.Warn << _T("CFileDisk::ReadLine(): Invalid UTF-8 string!") << newl;
                                c = 0;
                                break;
                            }
                            c |= (aBuffer[b-1] & 0x3f) << ((numbytes - b) * 6);
                        }
                    }
                    else
                    {
                        Console.Warn << _T("CFileDisk::ReadLine(): Invalid UTF-8 string!") << newl;
                        c = 0;
                    }
                }
            }
            else if (m_iMode & FILE_UTF16 && FILE_LITTLE_ENDIAN)
            {
#if BYTE_ORDER == LITTLE_ENDIAN
                c = *((ushort*)aBuffer);
#else
                c = ushort(SwapShortEndian(*((ushort*)aBuffer)));
#endif
            }
            else if (m_iMode & FILE_UTF16 && FILE_BIG_ENDIAN)
            {
#if BYTE_ORDER == LITTLE_ENDIAN
                c = ushort(SwapShortEndian(*((ushort*)aBuffer)));
#else
                c = *((ushort*)aBuffer);
#endif
            }
            else if (m_iMode & FILE_UTF32 && FILE_LITTLE_ENDIAN)
            {
#if BYTE_ORDER == LITTLE_ENDIAN
                c = *((uint*)aBuffer);
#else
                c = SwapIntEndian(*((uint*)aBuffer));
#endif
            }
            else if (m_iMode & FILE_UTF32 && FILE_BIG_ENDIAN)
            {
#if BYTE_ORDER == LITTLE_ENDIAN
                c = SwapIntEndian(*((uint*)aBuffer));
#else
                c = *((uint*)aBuffer);
#endif
            }
            else
            {
                c = 0;
            }
            

            // Check for a line break
            if (zOutCharSize == 1 || (m_iMode & FILE_ASCII))
            {
                if (c == _T('\r'))
                {
                    int iOldPos(Tell());
                    char cNext(ReadByte());
                    if (cNext != _T('\n'))
                        Seek(iOldPos);
                    break;
                }
                if (c == _T('\n'))
                    break;
            }
            else
            {
                if (c == L'\r')
                {
                    int iOldPos(Tell());
                    wchar_t cNext(ReadInt16());
                    if (cNext != L'\n')
                        Seek(iOldPos);
                    break;
                }
                if (c == L'\n')
                    break;
            }

            // Append this character to the string
            if (zOutCharSize == 1)
            {
                if (c & 0xffffff00)
                    Console.Warn << _T("CFileStream::ReadLine() [") + m_sPath + _T("] - Truncated a wide character") << newl;
                sReturn += char(c & 0xff);
            }
            else
            {
                TCHAR cMask(-1);        // This shouldn't ever actually do anything, but it prevents a warning
                sReturn += TCHAR(c & cMask);
            }
        }

        return sReturn;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CFileStream::ReadLine() [") + m_sPath + _T("] - "), NO_THROW);
        return _T("");
    }
}


/*====================
  CFileStream::WriteString
  ====================*/
bool    CFileStream::WriteString(const string &sText)
{
    assert(false);
    return false;
}

bool    CFileStream::WriteString(const tstring &sText)
{
    assert(false);
    return false;
}


/*====================
  CFileStream::Read
  ====================*/
uint    CFileStream::Read(char* pBuffer, uint uiBufferSize) const
{
    if (m_iMode & FILE_WRITE)
    {
        Console.Err << _T("Cannot read from WRITE file ") << m_sPath << newl;
        return 0;
    }

    if (!EnsureData(m_uiPos + uiBufferSize))
    {
        assert(false);
        return 0;
    }

    // Check for a buffer
    if (m_pBuffer)
    {
        // read from the buffer, and advance the position
        uint uiSize(uiBufferSize);

        if (m_uiSize - m_uiPos <= uiBufferSize)
        {
            uiSize = m_uiSize - m_uiPos;
            m_bEOF = true;
        }

        memcpy(pBuffer, m_pBuffer + m_uiPos, uiSize);
        m_uiPos += uiSize;
        return uiSize;
    }
    else
    {
        assert(false);
        return 0;
    }
}


/*====================
  CFileStream::Write
  ====================*/
size_t  CFileStream::Write(const void* pBuffer, size_t zBufferSize)
{
    assert(false);
    return 0;
}


/*====================
  CFileStream::GetBuffer
  ====================*/
const char  *CFileStream::GetBuffer(uint &uiSize)
{
    if (!EnsureData(m_uiSize))
    {
        uiSize = 0;
        return NULL;
    }

    uiSize = m_uiSize;
    return (const char*)m_pBuffer;
}


/*====================
  CFileStream::WriteByte
  ====================*/
bool    CFileStream::WriteByte(char c)
{
    assert(false);
    return false;
}


/*====================
  CFileStream::WriteInt16
  ====================*/
bool    CFileStream::WriteInt16(short n, bool bUseBigEndian)
{
    assert(false);
    return false;
}


/*====================
  CFileStream::WriteInt32
  ====================*/
bool    CFileStream::WriteInt32(int i, bool bUseBigEndian)
{
    assert(false);
    return false;
}


/*====================
  CFileStream::WriteInt64
  ====================*/
bool    CFileStream::WriteInt64(LONGLONG ll, bool bUseBigEndian)
{
    assert(false);
    return false;
}


/*====================
  CFileStream::Tell
  ====================*/
uint    CFileStream::Tell() const
{
    return m_uiPos;
}


/*====================
  CFileStream::Seek
  ====================*/
bool    CFileStream::Seek(uint uiOffset, ESeekOrigin eOrigin)
{
    bool bResult;

    if (m_pBuffer != NULL)
    {
        bResult = CFile::Seek(uiOffset, eOrigin);
        EnsureData(m_uiPos);

        if (m_uiPos == GetBufferSize())
            m_bEOF = true;
        else
            m_bEOF = false;

        return bResult;
    }

    return false;
}


/*====================
  CFileStream::Flush
  ====================*/
void    CFileStream::Flush()
{
}
