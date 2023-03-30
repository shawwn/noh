// (C)2005 S2 Games
// c_script.h
//
//=============================================================================
#ifndef __C_SCRIPT_H__
#define __C_SCRIPT_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef map<tstring, uint>  ScriptGotoMap;
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CScript;
extern K2_API CScript *g_pCurrentScript;
//=============================================================================

//=============================================================================
// CScript
//=============================================================================
class CScript
{
private:
    tstring         m_sFilename;
    tsvector            m_vCmdBuffer;
    bool            m_bLoaded;
    bool            m_bPreprocessed;

    bool            m_bStoringCvars;

    tsvector_cit        m_itCurrentCmd;

    int             m_iLineNum;
    dword           m_dwNextExecuteTime;

    tsmapts         m_mapParams;

    ScriptGotoMap   m_mapGoto;

    bool            m_bIncrement;

    void    Preprocess(CFileHandle &hScriptFile);
    void    Preprocess(const tstring &sScript);

public:
    ~CScript();
    CScript(tsmapts *mapParams = NULL);

    void    LoadFile(const tstring &sFilename);
    void    LoadScript(const tstring &sScript);

    bool    Execute();
    void    Reset();

    bool    IsLoaded()      { return m_bLoaded; }

    void    Goto(const tstring &sLabel);
    void    Sleep(dword dwMilliseconds);

    K2_API  tstring GetParameter(const tstring &sName);
    K2_API  void    AddParameter(const tstring &sName, const tstring &sValue);
};
//=============================================================================
#endif // __C_SCRIPT_H__
