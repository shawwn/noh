// (C)2005 S2 Games
// c_stringtable.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_stringtable.h"
#include "i_resourcelibrary.h"
#include "c_soundmanager.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
IResource*  AllocStringTable(const tstring &sPath);
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
IResourceLibrary    g_ResLibStringTable(RES_STRINGTABLE, _T("String Tables"), CStringTable::ResTypeName(), true, AllocStringTable);
//=============================================================================

/*====================
  AllocStringTable
  ====================*/
IResource*  AllocStringTable(const tstring &sPath)
{
    return K2_NEW(ctx_Resources,  CStringTable)(sPath);
}


/*====================
  CStringTable::CStringTable
  ====================*/
CStringTable::CStringTable(const tstring &sPath) :
IResource(sPath, TSNULL)
{
}


/*====================
  CStringTable::ReadCharacter
  ====================*/
uint    CStringTable::ReadCharacter(const char *pBuffer, size_t zSize, size_t &zPosition, size_t zCharSize, bool bBigEndian)
{
    // ASCII
    if (zCharSize == 0)
    {
        if (zPosition >= zSize)
            return 0;

        ++zPosition;
        return pBuffer[zPosition - 1];
    }

    // End of buffer
    if (zPosition + zCharSize >= zSize)
    {
        zPosition = zSize;
        return 0;
    }

    // UTF-16
    if (zCharSize == 2)
    {
        uint uiResult(0);
        uiResult |= byte(pBuffer[zPosition + (bBigEndian ? 0 : 1)]);
        uiResult <<= 8;
        uiResult |= byte(pBuffer[zPosition + (bBigEndian ? 1 : 0)]);
        if ((uiResult & 0xfc00) != 0xd800)
        {
            // Not a surrogate pair
            zPosition += 2;
            return uiResult;
        }

        if (zPosition + 2 >=  zSize)
        {
            zPosition = zSize;
            return 0;
        }

        uint uiSecond(0);
        uiSecond |= byte(pBuffer[zPosition + (bBigEndian ? 0 : 1)]);
        uiSecond <<= 8;
        uiSecond |= byte(pBuffer[zPosition + (bBigEndian ? 1 : 0)]);
        if ((uiSecond & 0xfc00) != 0xdc00)
        {
            // Invalid surrogate
            zPosition += 2;
            return 0;
        }

        zPosition += 4;
        return ((uiResult & 0x03ff) << 10) | (uiSecond & 0x03ff);
    }

    // UTF-32
    if (zCharSize == 4)
    {
        uint uiResult(*((uint*)&pBuffer[zPosition]));
        zPosition += 4;
        return uiResult;
    }

    // UTF-8
    uint uiResult(pBuffer[zPosition]);
    if ((uiResult & 0x80) == 0)
    {
        // Single byte
        ++zPosition;
        return uiResult;
    }


    // Determine how many bytes are in this character
    uint uiMask(0x80);
    uint uiNumBytes(0);
    while (zPosition < zSize && (pBuffer[zPosition] & uiMask))
    {
        ++uiNumBytes;
        uiMask >>= 1;
    }

    // Assign the remaining bits of the first byte
    uiMask -= 1;
    uiResult = (uiResult & uiMask) << ((uiNumBytes - 1) * 6);
    ++zPosition;

    // Assemble the remaining bytes
    for (uint ui(2); ui <= uiNumBytes; ++ui)
    {
        uint uiNext(pBuffer[zPosition]);
        if (uiNext == 0 || (uiNext & 0xc0) != 0x80)
            return 0;

        uiResult |= (uiNext & 0x3f) << ((uiNumBytes - ui) * 6);
        ++zPosition;
    }

    return uiResult;
}


/*====================
  CStringTable::ParseBuffer
  ====================*/
bool    CStringTable::ParseBuffer(const char *pBuffer, size_t zSize)
{
    // Check for a BOM at the start
    if (zSize < 4)
        return false;

    size_t zAt(0);

    size_t zCharSize(1); // Load as UTF-8 without BOM by default.
    bool bBigEndian(false);
    if (pBuffer[0] == char(0xff) &&
        pBuffer[1] == char(0xfe) &&
        pBuffer[2] == char(0x00) &&
        pBuffer[3] == char(0x00))
    {
        zCharSize = 4;
        zAt = 4;
    }
    else if (pBuffer[0] == char(0x00) &&
        pBuffer[1] == char(0x00) &&
        pBuffer[2] == char(0xfe) &&
        pBuffer[3] == char(0xff))
    {
        zCharSize = 4;
        bBigEndian = true;
        zAt = 4;
    }
    else if (pBuffer[0] == char(0xef) && pBuffer[1] == char(0xbb) && pBuffer[2] == char(0xbf))
    {
        zCharSize = 1;
        zAt = 3;
    }
    else if (pBuffer[0] == char(0xff) && pBuffer[1] == char(0xfe))
    {
        zCharSize = 2;
        zAt = 2;
    }
    else if (pBuffer[0] == char(0xfe) && pBuffer[1] == char(0xff))
    {
        zCharSize = 2;
        bBigEndian = true;
        zAt = 2;
    }

    // Read the text
    while (zAt < zSize)
    {
        size_t zStart(zAt);

        // Skip leading spaces and trailing line breaks
        while (zAt < zSize)
        {
            uint uiChar(ReadCharacter(pBuffer, zSize, zAt, zCharSize, bBigEndian));
            if (uiChar != '\n' && uiChar != '\r' && uiChar != ' ' && uiChar != '\t')
                break;

            zStart = zAt;
        }
        zAt = zStart;
        if (zAt >= zSize)
            break;

        wstring sString;

        // Find end of line
        bool bEscaped(false);
        while (zAt < zSize)
        {
            uint uiChar(ReadCharacter(pBuffer, zSize, zAt, zCharSize, bBigEndian));
            if (uiChar == '\n' || uiChar == '\r')
                break;

            if (bEscaped)
            {
                bEscaped = false;

                switch (uiChar)
                {
                case 'n':   uiChar = '\n'; break;
                case 'r':   uiChar = '\r'; break;
                case 't':   uiChar = '\t'; break;
                case '\n':  continue;
                case '\r':  continue;
                }
            }
            else if (uiChar == '\\')
            {
                bEscaped = true;
                continue;
            }

            sString += wchar_t(uiChar & 0xffff);
        }
        if (zAt >= zSize)
            break;

        // Check for comments
        if (sString.length() < 2 || (sString[0] == L'/' && sString[1] == L'/'))
            continue;

        size_t zEnd(sString.find_first_of(L"\t "));
        size_t zSecond(sString.find_first_not_of(L"\t ", zEnd));
        if (zSecond == wstring::npos)
        {
            m_mapKeys2Entries[WStringToTString(sString.substr(0, zEnd))] = INVALID_INDEX;
        }
        else
        {
            m_vStrings.push_back(WStringToTString(sString.substr(zSecond)));
            m_mapKeys2Entries[WStringToTString(sString.substr(0, zEnd))] = INT_SIZE(m_vStrings.size() - 1);
        }
    }

    return true;
}


/*====================
  CStringTable::Load
  ====================*/
int     CStringTable::Load(uint uiIgnoreFlags, const char *pData, uint uiSize)
{
    PROFILE("CStringTable::Load");

    try
    {
        if (!m_sPath.empty())
            Console.Res << "Loading StringTable " << SingleQuoteStr(m_sPath) << newl;
        else if (!m_sName.empty())
            Console.Res << "Loading StringTable " << SingleQuoteStr(m_sName) << newl;
        else
            Console.Res << "Loading Unknown StringTable" << newl;

        if (!ParseBuffer(pData, uiSize))
            return RES_LOAD_FAILED;

        tstring sExtended;
        if (HasFlags(RES_FLAG_LOCALIZED))
            sExtended = Filename_GetName(m_sPath) + _T("*") + m_sLocalizedPath.substr(Filename_StripExtension(m_sPath).length());
        else
            sExtended = Filename_GetName(m_sPath) + _T("*.") + Filename_GetExtension(m_sPath);

        tsvector vFileList;
        FileManager.GetFileList(Filename_GetPath(m_sPath), sExtended, false, vFileList);

        for (tsvector::iterator it(vFileList.begin()), itEnd(vFileList.end()); it != itEnd; ++it)
        {
            if (*it == m_sLocalizedPath || *it == m_sPath)
                continue;

            CFileHandle hFile(*it, FILE_READ | FILE_BINARY);

            if (!hFile.IsOpen())
                continue;
            
            const char *pData(nullptr);
            uint uiSize(0);

            pData = hFile.GetBuffer(uiSize);

            ParseBuffer(pData, uiSize);
        }
    }
    catch (CException &ex)
    {
        ex.Process(_TS("CStringTable::Load(") + m_sName + _TS(") - "), NO_THROW);
        return RES_LOAD_FAILED;
    }

    return 0;
}


/*====================
  CStringTable::Free
  ====================*/
void    CStringTable::Free()
{
    m_mapKeys2Entries.clear();
    m_vStrings.clear();
}


/*====================
  CStringTable::Get
  ====================*/
const tstring&  CStringTable::Get(const tstring &sKey) const
{
    Keys2EntriesMap::const_iterator itFind(m_mapKeys2Entries.find(sKey));

    if (itFind != m_mapKeys2Entries.end())
    {
        if (itFind->second == INVALID_INDEX)
            return TSNULL;

        uint uiIdx(itFind->second);
        return m_vStrings[uiIdx];
    }
    
    return sKey;
}


/*====================
  CStringTable::GetValues
  ====================*/
uint    CStringTable::GetValues(TStringSet& setOutStrings)
{
    uint uiAdded(0);
    for (size_t i(0); i < m_vStrings.size(); ++i)
    {
        const tstring &sValue(m_vStrings[i]);

        // skip empty values.
        if (sValue == TSNULL)
            continue;

        if (setOutStrings.insert(sValue).second)
            ++uiAdded;
    }
    return uiAdded;
}

