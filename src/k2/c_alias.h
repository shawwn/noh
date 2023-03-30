// (C)2005 S2 Games
// c_alias.h
//
//=============================================================================
#ifndef __C_ALIAS_H__
#define __C_ALIAS_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_consoleelement.h"
//=============================================================================

class CFileHandle;
class CXMLDoc;

//=============================================================================
// CAlias
//=============================================================================
class K2_API CAlias : public CConsoleElement
{
private:
    tstring m_sCmd;

public:
    CAlias(const tstring &sName, const tstring &sCmd, int iFlags = 0);
    ~CAlias();

    tstring         GetString() const           { return m_sCmd; }
    const tstring&  GetCmd() const              { return m_sCmd; }
    void            Set(const tstring &sCmd)    { m_sCmd = sCmd; }

    void            Write(CFileHandle &hFile, const tstring &sWildcard, int iFlags);
    void            Write(CXMLDoc &xmlConfig, const tstring &sWildcard, int iFlags);
    static bool     WriteConfigFile(CFileHandle &hFile, const tsvector &wildcards, int iFlags);
    static bool     WriteConfigFile(CXMLDoc &xmlConfig, const tsvector &wildcards, int iFlags);

    static CAlias   *Create(const tstring &sName, const tstring &sCmd);
    static CAlias   *Find(const tstring &sName);
};
//=============================================================================
#endif
