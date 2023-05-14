// (C)2005 S2 Games
// c_filedisk.cpp
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include <sys/stat.h>
#if defined(linux) || defined(__APPLE__)
#include <unistd.h>
#include <fcntl.h>
#endif

#include "c_filedisk.h"
#include "stringutils.h"
//=============================================================================

/*====================
  CFileDisk::CFileDisk
  ====================*/
CFileDisk::CFileDisk()
{
}


/*====================
  CFileDisk::Open
  ====================*/
bool    CFileDisk::Open(const tstring &sPath, int iMode)
{
    m_sPath = sPath;
    m_iMode = iMode;

    // Set the access mode
    ios_base::openmode iOpenFlags;

    if (m_iMode & FILE_READ)
    {
        m_iMode &= ~(FILE_APPEND | FILE_TRUNCATE);
        iOpenFlags = ios_base::in;
    }
    else if (m_iMode & FILE_WRITE)
    {
        iOpenFlags = ios_base::out;
    }
    else
    {
        Console.Warn << _T("No valid mode specified for file ") << sPath << newl;
        return false;
    }

    if (m_iMode & FILE_APPEND)
    {
        iOpenFlags |= ios_base::app;
    }
    else if (m_iMode & FILE_WRITE)
    {
        m_iMode |= FILE_TRUNCATE;
        iOpenFlags |= ios_base::trunc;
    }

    // Always open the stream as binary so that the engine can
    // handle all cases of linebreaks properly
    iOpenFlags |= ios_base::binary;
    if (m_iMode & FILE_TEXT)
    {
        if ((m_iMode & FILE_WRITE) &&
            (m_iMode & FILE_TEXT_ENCODING_MASK) == 0)
            m_iMode |= FILE_DEFAULT_TEXT_ENCODING;

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
    m_File.open(TStringToNative(m_sPath).c_str(), iOpenFlags);

    // default behavior is to buffer
    if (!(m_iMode & FILE_NOBUFFER))
        m_iMode |= FILE_BUFFER;

    // default behavior is to block
    if (!(m_iMode & FILE_NOBLOCK))
        m_iMode |= FILE_BLOCK;

    // validate file
    if (!m_File.is_open())
    {
        Console.Warn << _T("Failed to open file: ") << sPath << newl;
        return false;
    }
    else
    {
#if TKTK // Just disable this for now, since can't figure out how to access the file descriptor in a cross-platform way as of 2023
#if defined(linux) || defined(__APPLE__)
    // set files to close on exec
#if defined(__APPLE__)
    struct fd_accessor : public std::basic_filebuf<char> { int fd() { return __file_; } }; // error: '__file_' is a private member of 'std::filebuf'
#else
    struct fd_accessor : public std::basic_filebuf<char> { int fd() { return _M_file.fd(); } };
#endif
        long flags;
        if ((flags = fcntl(fd, F_GETFD, 0)) == -1)
            flags = 0;

        if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1)
            EX_ERROR(_T("fcntl() failure: ") + K2System.GetErrorString(errno));
#endif
#endif
        
        // Unicode files get a BOM
        if ((m_iMode & FILE_TEXT) &&
            (m_iMode & FILE_WRITE) &&
            !(m_iMode & FILE_ASCII))
        {
            if (m_iMode & FILE_UTF8)
            {
#ifdef _WIN32
                WriteByte('\xef');
                WriteByte('\xbb');
                WriteByte('\xbf');
#endif
            }
            else
            {
                if (m_iMode & FILE_BIG_ENDIAN)
                {
                    if (m_iMode & FILE_UTF32)
                    {
                        WriteByte('\x00');
                        WriteByte('\x00');
                    }
                    WriteByte('\xfe');
                    WriteByte('\xff');
                }
                else
                {
                    WriteByte('\xff');
                    WriteByte('\xfe');
                    if (m_iMode & FILE_UTF32)
                    {
                        WriteByte('\x00');
                        WriteByte('\x00');
                    }
                }
            }
        }

        if (m_iMode & FILE_READ)
        {
            m_File.seekg(0, std::ios::end);
            m_uiSize = m_File.tellg();
            m_File.seekg(0);
        }

        if ((m_iMode & FILE_READ) &&
            (m_iMode & FILE_BUFFER))
            GetBuffer(m_uiSize);

        if (m_iMode & FILE_READ)
            m_bEOF = false;

        return true;
    }
}


/*====================
  CFileDisk::Close
  ====================*/
void    CFileDisk::Close()
{
    m_File.close();
    SAFE_DELETE_ARRAY(m_pBuffer);
}


/*====================
  CFileDisk::IsOpen
  ====================*/
bool    CFileDisk::IsOpen() const
{
    return m_File.is_open();
}


/*====================
  CFileDisk::ReadLine
  ====================*/
tstring CFileDisk::ReadLine()
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
            else if (m_iMode & FILE_UTF16 & FILE_LITTLE_ENDIAN)
            {
#if BYTE_ORDER == LITTLE_ENDIAN
                c = *((ushort*)aBuffer);
#else
                c = ushort(SwapShortEndian(*((ushort*)aBuffer)));
#endif
            }
            else if (m_iMode & FILE_UTF16 & FILE_BIG_ENDIAN)
            {
#if BYTE_ORDER == LITTLE_ENDIAN
                c = ushort(SwapShortEndian(*((ushort*)aBuffer)));
#else
                c = *((ushort*)aBuffer);
#endif
            }
            else if (m_iMode & FILE_UTF32 & FILE_LITTLE_ENDIAN)
            {
#if BYTE_ORDER == LITTLE_ENDIAN
                c = *((uint*)aBuffer);
#else
                c = SwapIntEndian(*((uint*)aBuffer));
#endif
            }
            else if (m_iMode & FILE_UTF32 & FILE_BIG_ENDIAN)
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
                    Console.Warn << _T("CFileDisk::ReadLine() [") + m_sPath + _T("] - Truncated a wide character") << newl;
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
        ex.Process(_T("CFileDisk::ReadLine() [") + m_sPath + _T("] - "), NO_THROW);
        return _T("");
    }
}


/*====================
  CFileDisk::WriteString
  ====================*/
bool    CFileDisk::WriteString(const string &sText)
{
    if (sText.empty())
        return true;

    if (m_iMode & FILE_READ)
    {
        Console.Warn << _T("Cannot write to READ file: ") << m_sPath << newl;
        return false;
    }

    string sTmp(NormalizeLineBreaks(sText));

    // Check to see if the string requires convesion
    if ((m_iMode & FILE_ASCII) || (m_iMode & FILE_UTF8) || (m_iMode & FILE_BINARY))
    {
        m_File.write(sTmp.data(), std::streamsize(sTmp.length() * sizeof(char)));
    }
    else
    {
        // Determine write character size
        size_t zCharSize(2);
        if (m_iMode & FILE_UTF32)
            zCharSize = 4;

        // Determine offsets
        size_t a(0), b(1), c(2), d(3);
        if (m_iMode & 
#if BYTE_ORDER == LITTLE_ENDIAN         
                FILE_BIG_ENDIAN
#else
                FILE_LITTLE_ENDIAN
#endif
                )
        {
            if (m_iMode & FILE_UTF32)
            {
                a = 3; b = 2; c = 1; d = 0;
            }
            else
            {
                a = 1; b = 0;
            }
        }

        // Fill the temporary buffer
        size_t zBufferSize(sTmp.length() * zCharSize);
        char *pBuffer(K2_NEW_ARRAY(ctx_FileSystem, char, zBufferSize));
        for (size_t z(0); z < sTmp.length(); ++z)
        {
            pBuffer[(z * zCharSize) + a] = sTmp[z];
            pBuffer[(z * zCharSize) + b] = 0;
            if (m_iMode & FILE_UTF32)
            {
                pBuffer[(z * zCharSize) + c] = 0;
                pBuffer[(z * zCharSize) + d] = 0;
            }
        }

        m_File.write(pBuffer, std::streamsize(zBufferSize));
        K2_DELETE_ARRAY(pBuffer);
    }

    if (m_iMode & FILE_NOBUFFER)
        m_File.flush();

    return true;
}

bool    CFileDisk::WriteString(const wstring &sText)
{
    if (sText.empty())
        return true;

    if (m_iMode & FILE_READ)
    {
        Console.Warn << _T("Cannot write to READ file: ") << m_sPath << newl;
        return false;
    }

    wstring sTmp(NormalizeLineBreaks(sText));

    if (m_iMode & FILE_BINARY)
    {
        m_File.write(reinterpret_cast<const char*>(sTmp.data()), std::streamsize(sTmp.length() * sizeof(wchar_t)));
    }
    else
    {
        // Determine write character size
        size_t zCharSize(1);
        if (m_iMode & FILE_UTF16)
            zCharSize = 2;
        else if (m_iMode & FILE_UTF32)
            zCharSize = 4;

        // Determine offsets
        size_t a(0), b(1), c(2), d(3);
        if (m_iMode &
#if BYTE_ORDER == LITTLE_ENDIAN         
                FILE_BIG_ENDIAN
#else
                FILE_LITTLE_ENDIAN
#endif
                )
        {
            if (m_iMode & FILE_UTF32)
            {
                a = 1; b = 0;
            }
            else
            {
                a = 3; b = 2; c = 1; d = 0;
            }
        }

        if (m_iMode & FILE_UTF8)
        {
            string sBuffer(WStringToUTF8(sTmp));
            m_File.write(sBuffer.c_str(), std::streamsize(sBuffer.size()));
        }
        else
        {
            // Fill the temporary buffer
            size_t zBufferSize(sTmp.length() * zCharSize);
            char *pBuffer(K2_NEW_ARRAY(ctx_FileSystem, char, zBufferSize));
            for (size_t z(0); z < sTmp.length(); ++z)
            {
                pBuffer[(z * zCharSize) + a] = (sTmp[z] & 0xff);
                if (zCharSize > 1)
                    pBuffer[(z * zCharSize) + b] = ((sTmp[z] >> 8) & 0xff);
#ifdef _WIN32
                if (zCharSize > 2)
                {
                    pBuffer[(z * zCharSize) + c] = 0;
                    pBuffer[(z * zCharSize) + d] = 0;
                }
#else
                if (zCharSize > 2)
                    pBuffer[(z * zCharSize) + c] = ((sTmp[z] >> 16) & 0xff);
                if (zCharSize > 3)
                    pBuffer[(z * zCharSize) + d] = ((sTmp[z] >> 24) & 0xff);
#endif
            }
    
            m_File.write(pBuffer, std::streamsize(zBufferSize));
            K2_DELETE_ARRAY(pBuffer);
        }
    }

    if (m_iMode & FILE_NOBUFFER)
        m_File.flush();

    return true;
}


/*====================
  CFileDisk::Read
  ====================*/
uint    CFileDisk::Read(char* pBuffer, uint uiBufferSize) const
{
    if (m_iMode & FILE_WRITE)
    {
        Console.Warn << _T("Cannot read from WRITE file ") << m_sPath << newl;
        return 0;
    }

    // Check for a buffer
    if (m_pBuffer)
    {
        // read from the buffer, and advance the position
        uint uiSize(uiBufferSize);

        if (m_uiSize - m_uiPos < uiBufferSize)
        {
            uiSize = m_uiSize - m_uiPos;
            m_bEOF = true;
        }

        MemManager.Copy(pBuffer, m_pBuffer + m_uiPos, uiSize);
        m_uiPos += uiSize;
        return uiSize;
    }
    else
    {
        // read directly from the disk
        m_File.read(pBuffer, uiBufferSize);
        uint uiCount(m_File.gcount());
        if (uiCount < uiBufferSize)
            m_bEOF = true;
        return uiCount;
    }
}


/*====================
  CFileDisk::Write
  ====================*/
size_t  CFileDisk::Write(const void* pBuffer, size_t zBufferSize)
{
    if (!IsOpen())
        return 0;

    if (m_iMode & FILE_READ)
    {
        Console.Warn << _T("Cannot write to READ file ") << m_sPath << newl;
        return 0;
    }

    m_File.write(reinterpret_cast<const char*>(pBuffer), std::streamsize(zBufferSize));

    // flush the file if it was opened unbuffered
    if (m_iMode & FILE_NOBUFFER)
        m_File.flush();

    return zBufferSize;
}


/*====================
  CFileDisk::GetBuffer
  ====================*/
const char  *CFileDisk::GetBuffer(uint &uiSize)
{
    try
    {
        if (!IsOpen())
            EX_ERROR(_T("Trying to retrieve buffer without opening file"));

        // Check if the buffer has already been filled
        if (m_pBuffer != nullptr)
        {
            uiSize = m_uiSize;
            return m_pBuffer;
        }
        
        m_pBuffer = K2_NEW_ARRAY(ctx_FileSystem, char, m_uiSize);
        if (m_pBuffer == nullptr)
            EX_ERROR(_T("Failed to allocate buffer for file: ") + m_sPath);

        // Allocate and fill the buffer, but preserve any existing read position
        m_uiPos = m_File.tellg();

        if (m_uiPos != 0)
            m_File.seekg(0);

        m_File.read(m_pBuffer, std::streamsize(m_uiSize));
        m_File.seekg(m_uiPos);

        // Check for a unicode BOM
        if (m_iMode & FILE_TEXT &&
            (m_iMode & FILE_TEXT_ENCODING_MASK) == 0)
        {
            if (m_uiSize >= 4)
            {
                if (m_pBuffer[0] == char(0xff) &&
                    m_pBuffer[1] == char(0xfe) &&
                    m_pBuffer[2] == char(0x00) &&
                    m_pBuffer[3] == char(0x00))
                {
                    m_iMode |= (FILE_UTF32 | FILE_LITTLE_ENDIAN);
                    m_uiPos = 4;
                }
                if (m_pBuffer[0] == char(0x00) &&
                    m_pBuffer[1] == char(0x00) &&
                    m_pBuffer[2] == char(0xfe) &&
                    m_pBuffer[3] == char(0xff))
                {
                    m_iMode |= (FILE_UTF32 | FILE_BIG_ENDIAN);
                    m_uiPos = 4;
                }
            }
            if (m_uiSize >= 3)
            {
                if (m_pBuffer[0] == char(0xef) &&
                    m_pBuffer[1] == char(0xbb) &&
                    m_pBuffer[2] == char(0xbf))
                {
                    m_iMode |= FILE_UTF8;
                    m_uiPos = 3;
                }
            }
            if (m_uiSize >= 2)
            {
                if (m_pBuffer[0] == char(0xff) &&
                    m_pBuffer[1] == char(0xfe))
                {
                    m_iMode |= (FILE_UTF16 | FILE_LITTLE_ENDIAN);
                    m_uiPos = 2;
                }
                if (m_pBuffer[0] == char(0xfe) &&
                    m_pBuffer[1] == char(0xff))
                {
                    m_iMode |= (FILE_UTF16 | FILE_BIG_ENDIAN);
                    m_uiPos = 2;
                }
            }
            if ((m_iMode & FILE_TEXT_ENCODING_MASK) == 0)
#ifdef UNICODE
                m_iMode |= FILE_UTF8;
#else
                m_iMode |= FILE_ASCII;
#endif
        }

        // Return values
        uiSize = m_uiSize;
        return m_pBuffer;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CFileDisk::GetBuffer() - "), NO_THROW);
        return nullptr;
    }
}


/*====================
  CFileDisk::WriteByte
  ====================*/
bool    CFileDisk::WriteByte(char c)
{
    if (!Write(&c, 1))
    {
        Console.Err << _T("WriteByte() - Write failed") << m_sPath << newl;
        return false;
    }

    return true;
}


/*====================
  CFileDisk::WriteInt16
  ====================*/
bool    CFileDisk::WriteInt16(short n, bool bUseBigEndian)
{
    char    buffer[2];
    if (bUseBigEndian)
    {
        buffer[1] = n & 0xff;
        buffer[0] = (n >> 8) & 0xff;
    }
    else
    {
        buffer[0] = n & 0xff;
        buffer[1] = (n >> 8) & 0xff;
    }

    if (!Write(buffer, 2))
    {
        Console.Err << _T("WriteInt16() - Write failed") << m_sPath << newl;
        return false;
    }

    return true;
}


/*====================
  CFileDisk::WriteInt32
  ====================*/
bool    CFileDisk::WriteInt32(int i, bool bUseBigEndian)
{
    char buffer[4];
    if (bUseBigEndian)
    {
        buffer[3] = i & 0xff;
        buffer[2] = (i >> 8) & 0xff;
        buffer[1] = (i >> 16) & 0xff;
        buffer[0] = (i >> 24) & 0xff;
    }
    else
    {
        buffer[0] = i & 0xff;
        buffer[1] = (i >> 8) & 0xff;
        buffer[2] = (i >> 16) & 0xff;
        buffer[3] = (i >> 24) & 0xff;
    }


    if (!Write(buffer, 4))
    {
        Console.Err << _T("WriteInt32() - Write failed") << m_sPath << newl;
        return false;
    }

    return true;
}


/*====================
  CFileDisk::WriteInt64
  ====================*/
bool    CFileDisk::WriteInt64(LONGLONG ll, bool bUseBigEndian)
{
    char buffer[8];
    if (bUseBigEndian)
    {
        buffer[7] = char(ll & 0xff);
        buffer[6] = char((ll >> 8) & 0xff);
        buffer[5] = char((ll >> 16) & 0xff);
        buffer[4] = char((ll >> 24) & 0xff);
        buffer[3] = char((ll >> 32) & 0xff);
        buffer[2] = char((ll >> 40) & 0xff);
        buffer[1] = char((ll >> 48) & 0xff);
        buffer[0] = char((ll >> 56) & 0xff);
    }
    else
    {
        buffer[0] = char(ll & 0xff);
        buffer[1] = char((ll >> 8) & 0xff);
        buffer[2] = char((ll >> 16) & 0xff);
        buffer[3] = char((ll >> 24) & 0xff);
        buffer[4] = char((ll >> 32) & 0xff);
        buffer[5] = char((ll >> 40) & 0xff);
        buffer[6] = char((ll >> 48) & 0xff);
        buffer[7] = char((ll >> 56) & 0xff);
    }

    if (!Write(buffer, 8))
    {
        Console.Err << _T("WriteInt64() - Write failed") << m_sPath << newl;
        return false;
    }

    return true;
}


/*====================
  CFileDisk::Tell
  ====================*/
uint    CFileDisk::Tell() const
{
    if (m_pBuffer == nullptr)
        return m_File.tellg();
    else
        return m_uiPos;
}


/*====================
  CFileDisk::Seek
  ====================*/
bool    CFileDisk::Seek(uint uiOffset, ESeekOrigin eOrigin)
{
    bool bResult;

    if (m_pBuffer != nullptr)
    {
        bResult = CFile::Seek(uiOffset, eOrigin);

        if (m_uiPos == GetBufferSize())
            m_bEOF = true;
        else
            m_bEOF = false;

        return bResult;
    }

    switch (eOrigin)
    {
    case SEEK_ORIGIN_CURRENT:
        m_File.seekg(uiOffset, ios_base::cur);

        if (uiOffset + m_uiPos < GetBufferSize())
            m_bEOF = false;
        else
            m_bEOF = true;

        break;

    case SEEK_ORIGIN_END:
        m_File.seekg(uiOffset, ios_base::end);

        if (uiOffset > 0)
            m_bEOF = false;
        else
            m_bEOF = true;

        break;

    case SEEK_ORIGIN_START:
        m_File.seekg(uiOffset, ios_base::beg);

        if (uiOffset < GetBufferSize())
            m_bEOF = false;
        else
            m_bEOF = true;

        break;
    }

    bResult = (!m_File.fail());
    m_File.clear();
    return bResult;
}


/*====================
  CFileDisk::Flush
  ====================*/
void    CFileDisk::Flush()
{
    m_File.flush();
}
