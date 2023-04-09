// (C)2010 S2 Games
// c_phpdata.h
//
//=============================================================================
#ifndef __C_PHPDATA_H__
#define __C_PHPDATA_H__

//=============================================================================
// Declarations
//=============================================================================
class CPHPData;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef vector<CPHPData>                PHPDataArray;
typedef PHPDataArray::iterator          PHPDataArray_it;
typedef PHPDataArray::const_iterator    PHPDataArray_cit;
typedef map<string, CPHPData*>          PHPDataMap;
typedef pair<string, CPHPData*>         PHPDataPair;
typedef PHPDataMap::iterator            PHPDataMap_it;
typedef PHPDataMap::const_iterator      PHPDataMap_cit;

enum EPHPDataType
{
    PHP_INVALID,
    PHP_NULL,
    PHP_ARRAY,
    PHP_INTEGER,
    PHP_STRING,
    PHP_FLOAT,
    PHP_BOOL
};
//=============================================================================

//=============================================================================
// CPHPData
//=============================================================================
class CPHPData
{
private:
    EPHPDataType        m_eType;
    int                 m_iValue;
    float               m_fValue;
    tstring             m_sValue;
    bool                m_bValue;
    tstring             m_sKey;
    vector<CPHPData>    m_vArray;
    
    TCHAR*  ReadArray(TCHAR *p);
    TCHAR*  ReadInteger(TCHAR *p);
    TCHAR*  ReadFloat(TCHAR *p);
    TCHAR*  ReadString(TCHAR *p);
    TCHAR*  ReadBool(TCHAR *p);
    TCHAR*  Deserialize(TCHAR *p);
    
    void    SetValue(int iValue)            { m_eType = PHP_INTEGER; m_iValue = iValue; }
    void    SetValue(float fValue)          { m_eType = PHP_FLOAT; m_fValue = fValue; }
    void    SetValue(const TCHAR *sValue)   { m_eType = PHP_STRING; m_sValue = sValue; }
    void    SetValue(bool bValue)           { m_eType = PHP_BOOL; m_bValue = bValue; }

public:
    ~CPHPData() {}
    CPHPData();
    K2_API  CPHPData(const tstring &sData);

    bool                IsValid() const                     { return m_eType != PHP_INVALID; }

    EPHPDataType        GetType() const                     { return m_eType; }
    EPHPDataType        GetType(const tstring &sKey) const
    {
        for (vector<CPHPData>::const_iterator cit(m_vArray.begin()), citEnd(m_vArray.end()); cit != citEnd; ++cit)      
        {
            if (sKey == cit->m_sKey)
            {
                return cit->GetType();
            }
        }
        
        return PHP_NULL;
    }

    tstring GetString(const tstring &sKey, const tstring &sDefault = TSNULL) const
    {
        const CPHPData *pNode(GetVar(sKey));
        if (pNode == NULL)
            return sDefault;
        return pNode->GetString();
    }

    tstring GetString() const
    {
        switch (m_eType)
        {
        case PHP_INTEGER:   return XtoA(m_iValue);
        case PHP_FLOAT:     return XtoA(m_fValue);
        case PHP_BOOL:      return XtoA(m_bValue);
        case PHP_STRING:    return m_sValue;
        case PHP_ARRAY:
            {
                tstring sValue;

                for (vector<CPHPData>::const_iterator cit(m_vArray.begin()), citEnd(m_vArray.end()); cit != citEnd; ++cit)      
                {
                    if (!sValue.empty())
                        sValue += _T(",");

                    sValue += cit->GetString();
                }

                return sValue;
            }
        default:            return TSNULL;
        }
    }

    int GetInteger(const tstring &sKey, int iDefault = 0) const
    {
        const CPHPData *pNode(GetVar(sKey));
        if (pNode == NULL)
            return iDefault;
        return pNode->GetInteger();
    }

    int GetInteger() const
    {
        switch (m_eType)
        {
        case PHP_INTEGER:   return m_iValue;
        case PHP_FLOAT:     return INT_ROUND(m_fValue);
        case PHP_BOOL:      return m_bValue ? 1 : 0;
        case PHP_STRING:    return AtoI(m_sValue);
        default:            return 0;
        }
    }

    float   GetFloat(const tstring &sKey, float fDefault = 0.0f) const
    {
        const CPHPData *pNode(GetVar(sKey));
        if (pNode == NULL)
            return fDefault;
        return pNode->GetFloat();
    }

    float   GetFloat() const
    {
        switch (m_eType)
        {
        case PHP_INTEGER:   return float(m_iValue);
        case PHP_FLOAT:     return m_fValue;
        case PHP_BOOL:      return m_bValue ? 1.0f : 0.0f;
        case PHP_STRING:    return AtoF(m_sValue);
        default:            return 0.0f;
        }
    }

    bool    GetBool(const tstring &sKey, bool bDefault = false) const
    {
        const CPHPData *pNode(GetVar(sKey));
        if (pNode == NULL)
            return bDefault;
        return pNode->GetBool();
    }

    bool    GetBool() const
    {
        switch (m_eType)
        {
        case PHP_INTEGER:   return m_iValue != 0;
        case PHP_FLOAT:     return m_fValue != 0.0f;
        case PHP_BOOL:      return m_bValue;
        case PHP_STRING:    return AtoB(m_sValue);
        default:            return false;
        }
    }

    const CPHPData*     GetVar(const tstring &sKey) const
    {
        for (vector<CPHPData>::const_iterator cit(m_vArray.begin()), citEnd(m_vArray.end()); cit != citEnd; ++cit)      
        {
            if (sKey == cit->m_sKey)
            {
                return &(*cit);
            }
        }
        
        return NULL;
    }

    bool                HasVar(const tstring &sKey) const   { return GetVar(sKey) != NULL; }

    size_t              GetSize() const                     { if (m_eType == PHP_ARRAY) return m_vArray.size(); if (m_eType == PHP_NULL) return 0; return 1; }
    const CPHPData*     GetVar(size_t zOffset) const        { if (m_eType != PHP_ARRAY || zOffset >= GetSize()) return NULL; return &(m_vArray[zOffset]); }
    const tstring&      GetKeyName(size_t zOffset) const    { if (m_eType != PHP_ARRAY || zOffset >= GetSize()) return TSNULL; return m_vArray[zOffset].m_sKey; }
    
    K2_API  void        Print() const;
};
//=============================================================================

#endif //__C_PHPDATA_H__
