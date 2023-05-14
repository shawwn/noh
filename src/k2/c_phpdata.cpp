// (C)2010 S2 Games
// c_phpdata.cpp
//
// Parses data returned by PHP scripts
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_phpdata.h"
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
CVAR_BOOL(php_printDebugInfo,   false);
//=============================================================================

/*====================
  CPHPData::ReadInteger
  ====================*/
TCHAR*  CPHPData::ReadInteger(TCHAR *p)
{
    if (*p != _T(':') || !*(++p))
        return nullptr;
    
    TCHAR *sValue = p;
    
    if (!(p = _tcschr(p, _T(';'))))
        return nullptr;
    
    *p = 0;
    
    SetValue(int(_tstoi(sValue)));
    
    return p+1;
}


/*====================
  CPHPData::ReadFloat
  ====================*/
TCHAR*  CPHPData::ReadFloat(TCHAR *p)
{
    if (*p != _T(':') || !*(++p))
        return nullptr;
    
    TCHAR *sValue = p;
    
    if (!(p = _tcschr(p, _T(';'))))
        return nullptr;
    
    *p = 0;
    
    SetValue(float(_tstof(sValue)));
    
    return p+1;
}


/*====================
  CPHPData::ReadString
  ====================*/
TCHAR*  CPHPData::ReadString(TCHAR *p)
{
    int size;
    if (*p != _T(':') || !*(++p))
        return nullptr;
    
    size = _tcstol(p, &p, 0);
    if (*p != _T(':') || *(++p) != _T('"') || !*(++p))
        return nullptr;
    
    TCHAR *sValue = p;
    
    while (*p && p < sValue + size) ++p;
    
    if (*p != _T('"') || *(p+1) != _T(';'))
            return nullptr;
    
    *p = 0;
    
    SetValue(sValue);
    
    return p+2;
}


/*====================
  CPHPData::ReadBool
  ====================*/
TCHAR*  CPHPData::ReadBool(TCHAR *p)
{
    if (*p != _T(':') || !*(++p))
        return nullptr;
    
    TCHAR *sValue = p;
    
    if (!(p = _tcschr(p, _T(';'))))
        return nullptr;
    
    *p = 0;
    
    SetValue(AtoB(sValue));
    
    return p+1;
}


/*====================
  CPHPData::ReadArray
  ====================*/
TCHAR*  CPHPData::ReadArray(TCHAR *p)
{
    int i, size;
    if (*p != _T(':') || !*(++p))
        return nullptr;
    
    size = _tcstol(p, &p, 0);
    if (*p != _T(':') || *(++p) != _T('{') || !*(++p))
        return nullptr;
    
    m_eType = PHP_ARRAY;
    m_vArray.reserve(size);
    
    for (i = 0; i < size; ++i)
    {
        CPHPData phpKey;
        if (!(p = phpKey.Deserialize(p)))
            break;
        if (phpKey.GetType() == PHP_ARRAY)
            break;
        
        CPHPData phpData;
        if (!(p = phpData.Deserialize(p)))
            break;
        
        phpData.m_sKey = phpKey.GetString();
        m_vArray.push_back(phpData);
    }
    
    if (!p || *p != _T('}'))
    {
        return nullptr;
    }
    
    return p+1;
}


/*====================
  CPHPData::Deserialize
  ====================*/
TCHAR*  CPHPData::Deserialize(TCHAR *p)
{
    switch (*p)
    {
        case(_T('a')):
            p = ReadArray(p+1);
            break;
        
        case(_T('i')):
            p = ReadInteger(p+1);
            break;
            
        case(_T('d')):
            p = ReadFloat(p+1);
            break;
        
        case(_T('s')):
            p = ReadString(p+1);
            break;
        
        case(_T('b')):
            p = ReadBool(p+1);
            break;
        
        case(_T('N')):
            if (!*(++p) || *p != _T(';'))
                return nullptr;
            ++p;
            break;
        
        default:
            p = nullptr;
            break;
    }
    
    return p;
}


/*====================
  CPHPData::CPHPData
  ====================*/
CPHPData::CPHPData() :
m_eType(PHP_NULL),
m_iValue(0),
m_fValue(0.0f),
m_bValue(false)
{
}

CPHPData::CPHPData(const tstring &sData) :
m_eType(PHP_NULL),
m_iValue(0),
m_fValue(0.0f),
m_bValue(false)
{
    if (sData.empty())
    {
        Console.Err << _T("CPHPData::CPHPData - Empty data") << newl;
        m_eType = PHP_INVALID;
        return;
    }
    
    TCHAR *sBuf = K2_NEW_ARRAY(ctx_Net, TCHAR, sData.length()+1);
    //_tcsncpy(sBuf, sData.c_str(), sData.length());  // this throws warning in Win32 build - JT
    _TCSNCPY_S(sBuf, sData.length()+1, sData.c_str(), _TRUNCATE);   
    sBuf[sData.length()] = 0;
    
    if (!Deserialize(sBuf))
    {
        Console.Err << _T("CPHPData::CPHPData - Bad data: ") << sData << newl;
        m_eType = PHP_INVALID;
    }
    
    K2_DELETE_ARRAY(sBuf);

    if (php_printDebugInfo)
        Print();
}


/*====================
  CPHPData::Print
  ====================*/
void    CPHPData::Print() const
{
    static int s_iIndent(0);
    
    switch (m_eType)
    {
        case PHP_NULL:      Console << _T("nullptr"); break;
        case PHP_INTEGER:   Console << _T("INT: ") << m_iValue; break;
        case PHP_STRING:    Console << _T("STRING: ") << m_sValue; break;
        case PHP_FLOAT:     Console << _T("FLOAT: ") << m_fValue; break;
        case PHP_BOOL:      Console << _T("BOOL: ") << m_bValue; break;
        case PHP_ARRAY:
            Console << _T("ARRAY:\n");
            ++s_iIndent;
            for (vector<CPHPData>::const_iterator cit(m_vArray.begin()); cit != m_vArray.end(); ++cit)
            {
                for (int i(0); i < s_iIndent * 2; ++i)
                    Console << SPACE;
                
                Console << cit->m_sKey << _T(", ");
                cit->Print();
            }
            --s_iIndent;
            return;
        case PHP_INVALID:
            return;
    }
    Console << newl;
}
