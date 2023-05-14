// (C)2005 S2 Games
// c_file.cpp
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_file.h"
//=============================================================================

/*====================
  CFile::CFile
  ====================*/
CFile::CFile() :
m_pBuffer(nullptr),
m_iMode(0),
m_uiSize(0),
m_uiPos(0),
m_bEOF(true),
m_sPath(_T(""))
{
}


/*====================
  CFile::~CFile
  ====================*/
CFile::~CFile()
{
}


/*====================
  CFile::Peek
  ====================*/
char    CFile::Peek(uint uiPos)
{
    try
    {
        if (uiPos < 0 || uiPos >= m_uiSize)
            EX_ERROR(_T("Index is out of bounds: ") + XtoA(uiPos));

        if (m_pBuffer == nullptr)
        {
            int iOldPos(Tell());
            char c;
            Read(&c, 1);
            Seek(iOldPos);
            return c;
        }

        return m_pBuffer[uiPos];
    }
    catch (CException &ex)
    {
        ex.Process(_T("CFile::Peek() [") + m_sPath + _T("] - "), NO_THROW);
        return 0;
    }
}


/*====================
  CFile::ReadInt32
  ====================*/
int     CFile::ReadInt32(bool bUseBigEndian)
{
    byte    buffer[4];

    if (Read((char *)buffer, 4) < 4)
    {
        Console.Warn << _T("Hit end of file in ReadInt32() on file ") << m_sPath << newl;
        return 0;
    }


    if (bUseBigEndian)
        return ((buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3]);
    else
        return ((buffer[3] << 24) + (buffer[2] << 16) + (buffer[1] << 8) + buffer[0]);
}


/*====================
  CFile::ReadInt16
  ====================*/
short   CFile::ReadInt16(bool bUseBigEndian)
{
    byte    buffer[2];

    if (Read((char*)buffer, 2) < 2)
    {
        Console.Warn << _T("Hit end of file in ReadInt16() on file ") << m_sPath << newl;
        return 0;
    }

    if (bUseBigEndian)
        return ((buffer[0] << 8) + buffer[1]);
    else
        return ((buffer[1] << 8) + buffer[0]);
}


/*====================
  CFile::ReadByte
  ====================*/
byte    CFile::ReadByte()
{
    byte    b;
    if (!Read((char *)&b, 1))
    {
        Console.Warn << _T("Hit end of file in ReadByte() on file ") << m_sPath << newl;
        return 0;
    }

    return b;
}


/*====================
  CFile::ReadFloat
  ====================*/
float       CFile::ReadFloat(bool bUseBigEndian)
{
    byte    buffer[4];

    if (Read((char *)buffer, 4) < 4)
    {
        Console.Warn << _T("Hit end of file in ReadFloat() on file ") << m_sPath << newl;
        return 0;
    }

    uint uiValue;

    if (bUseBigEndian)
        uiValue = ((buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3]);
    else
        uiValue = ((buffer[3] << 24) + (buffer[2] << 16) + (buffer[1] << 8) + buffer[0]);

    return FLOAT_CAST(uiValue);
}


/*====================
  CFile::Seek
  ====================*/
bool    CFile::Seek(uint uiOffset, ESeekOrigin eOrigin)
{
    try
    {
        uint uiNewPos(0);
        switch (eOrigin)
        {
        case SEEK_ORIGIN_CURRENT:
            uiNewPos = m_uiPos + uiOffset;
            break;
        case SEEK_ORIGIN_END:
            uiNewPos = m_uiSize - uiOffset - 1;
            break;
        case SEEK_ORIGIN_START:
            uiNewPos = uiOffset;
            break;
        default:
            EX_ERROR(_T("Invalid origin type: ") + XtoA(eOrigin));
        }

        if (uiNewPos < 0 || uiNewPos >= m_uiSize)
            EX_ERROR(_T("Position out of bounds: ") + XtoA(uiNewPos) + _T(" (Size: ") + XtoA(m_uiSize) + _T(")"));

        m_uiPos = uiNewPos;

        m_bEOF = (m_uiPos == m_uiSize);
        
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CFile::Seek() [") + m_sPath + _T("] - "), NO_THROW);
        return false;
    }
}


/*====================
  CFile::ReadInt64
  ====================*/
LONGLONG    CFile::ReadInt64(bool bUseBigEndian)
{
    byte    buffer[8];

    if (Read((char *)buffer, 8) < 8)
    {
        Console.Warn << _T("Hit end of file in ReadInt64() on file ") << m_sPath << newl;
        return 0;
    }

    if (bUseBigEndian)
        return ((LONGLONG(buffer[0]) << 56) +
            (LONGLONG(buffer[1]) << 48) +
            (LONGLONG(buffer[2]) << 40) +
            (LONGLONG(buffer[3]) << 32) +
            (LONGLONG(buffer[4]) << 24) +
            (LONGLONG(buffer[5]) << 16) +
            (LONGLONG(buffer[6]) << 8) +
            LONGLONG(buffer[7]));
    else
        return ((LONGLONG(buffer[7]) << 56) +
            (LONGLONG(buffer[6]) << 48) +
            (LONGLONG(buffer[5]) << 40) +
            (LONGLONG(buffer[4]) << 32) +
            (LONGLONG(buffer[3]) << 24) +
            (LONGLONG(buffer[2]) << 16) +
            (LONGLONG(buffer[1]) << 8) +
            LONGLONG(buffer[0]));
}
