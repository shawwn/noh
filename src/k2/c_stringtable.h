// (C)2005 S2 Games
// c_stringtable.h
//
//=============================================================================
#ifndef __C_STRINGTABLE__
#define __C_STRINGTABLE__

//=============================================================================
// Headers
//=============================================================================
#include "i_resource.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CStringTable
//=============================================================================
class CStringTable : public IResource
{
private:
    typedef hash_map<tstring, uint> Keys2EntriesMap;

    Keys2EntriesMap     m_mapKeys2Entries;
    tsvector            m_vStrings;

    uint    ReadCharacter(const char *pBuffer, size_t zSize, size_t &zPosition, size_t zCharSize, bool bBigEndian);
    bool    ParseBuffer(const char *pBuffer, size_t zSize);

public:
    ~CStringTable() {}
    K2_API CStringTable(const tstring &sPath);

    K2_API  virtual uint            GetResType() const          { return RES_STRINGTABLE; }
    K2_API  virtual const tstring&  GetResTypeName() const      { return ResTypeName(); }
    K2_API  static const tstring&   ResTypeName()               { static tstring sTypeName(_T("{stringtable}")); return sTypeName; }

    int     Load(uint uiIgnoreFlags, const char *pData, uint uiSize);
    void    Free();

    K2_API  const tstring&          Get(const tstring &sKey) const;
    inline  const tstring&          Get(uint uiIndex) const
    {
        if (uiIndex >= m_vStrings.size())
            return TSNULL;
        return m_vStrings[uiIndex];
    }
    inline  uint                    GetIndex(const tstring &sKey) const
    {
        Keys2EntriesMap::const_iterator itFind(m_mapKeys2Entries.find(sKey));
        if (itFind != m_mapKeys2Entries.end())
            return itFind->second;
        return INVALID_INDEX;
    }

    K2_API  uint                    GetValues(TStringSet& setOutStrings);
};
//=============================================================================
#endif //__C_SAMPLE__
