// (C)2005 S2 Games
// c_script.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_script.h"
#include "c_filemanager.h"
#include "c_function.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
K2_API  CScript *g_pCurrentScript(NULL);
//=============================================================================

/*====================
  CScript::~CScript
  ====================*/
CScript::~CScript()
{
}


/*====================
  CScript::CScript
  ====================*/
CScript::CScript(tsmapts *mapParams) :
m_sFilename(_T("")),
m_bLoaded(false),
m_iLineNum(0),
m_dwNextExecuteTime(0),
m_bIncrement(true),
m_bStoringCvars(false)
{
    if (mapParams != NULL)
        m_mapParams = (*mapParams);
}

/*====================
  CScript::LoadFile
  ====================*/
void    CScript::LoadFile(const tstring &sFilename)
{
    if (m_bLoaded)
        return;

    m_bStoringCvars = ICvar::StoreCvars();

    m_sFilename = sFilename;

    CFileHandle hScriptFile(sFilename, FILE_READ | FILE_TEXT | FILE_ALLOW_CUSTOM);
    if (!hScriptFile.IsOpen())
    {
        Console.Warn << _T("Could not read ") << SingleQuoteStr(sFilename) << _T("") << newl;
        return;
    }

    Preprocess(hScriptFile);
    m_bLoaded = true;
}

/*====================
  CScript::LoadScript
  ====================*/
void    CScript::LoadScript(const tstring &sScript)
{
    if (m_bLoaded)
        return;

    m_bStoringCvars = ICvar::StoreCvars();

    Preprocess(sScript);
    m_bLoaded = true;
}

/*====================
  CScript::Preprocess

  Handles all comments and whitespace replacement, as well as mapping the goto labels
  ====================*/
void    CScript::Preprocess(CFileHandle &hScriptFile)
{
    while (!hScriptFile.IsEOF())
    {
        tstring sCmd(hScriptFile.ReadLine());
        if (sCmd.empty())
            continue;

        if (sCmd[0] == '@')
            m_mapGoto[sCmd.substr(1, tstring::npos)] = uint(m_vCmdBuffer.size());
        else
            m_vCmdBuffer.push_back(sCmd);
    }

    m_itCurrentCmd = m_vCmdBuffer.begin();
}


/*====================
  CScript::Preprocess

  Handles all comments and whitespace replacement, as well as mapping the goto labels
  ====================*/
void    CScript::Preprocess(const tstring &sScript)
{
    tstring sNewScript(sScript);
    size_t zPos(0);
    size_t zLastPos(0);

    NormalizeLineBreaks(sNewScript);

    while (zLastPos < sNewScript.length())
    {
        zPos = sNewScript.find(TLINEBREAK, zLastPos);

        if (zPos == tstring::npos)
            zPos = sNewScript.length();

        tstring sCmd(sNewScript.substr(zLastPos, zPos - zLastPos));
        if (sCmd.empty())
        {
            zLastPos = zPos + TLINEBREAK.length();
            continue;
        }

        if (sCmd[0] == '@')
            m_mapGoto[sCmd.substr(1, tstring::npos)] = uint(m_vCmdBuffer.size());
        else
            m_vCmdBuffer.push_back(sCmd);

        zLastPos = zPos + TLINEBREAK.length();
    }

    m_itCurrentCmd = m_vCmdBuffer.begin();
}


/*====================
  CScript::Goto
  ====================*/
void    CScript::Goto(const tstring &sLabel)
{
    ScriptGotoMap::const_iterator findit(m_mapGoto.find(sLabel));

    if (findit != m_mapGoto.end())
    {
        m_itCurrentCmd = m_vCmdBuffer.begin() + findit->second;
        if (m_itCurrentCmd != m_vCmdBuffer.end())
            m_bIncrement = false; // Do not increment next loop, or we will skip the next command
    }
    else
        Console.Dev << _T("Label ") << SingleQuoteStr(sLabel) << _T(" not found.") << newl;
}


/*====================
  CScript::Sleep
  ====================*/
void    CScript::Sleep(dword dwMilliseconds)
{
    m_dwNextExecuteTime = Host.GetTime() + dwMilliseconds;
}


/*====================
  CScript::Reset
  ====================*/
void    CScript::Reset()
{
    m_dwNextExecuteTime = 0;
}


/*====================
  CScript::Execute

  Execute the script.  True is returned if the script actually finishes, otherwise False (on a pause or such)
  ====================*/
bool    CScript::Execute()
{
    tstring     sCmd;
    tstring     sOldDir;
    CScript*    pPrevScript;

    if (m_dwNextExecuteTime && m_dwNextExecuteTime > Host.GetTime())
        return false;

    bool bWasStoring(ICvar::StoreCvars());
    ICvar::StoreCvars(m_bStoringCvars);

    // Track the current script for parameter purposes
    pPrevScript = g_pCurrentScript;
    g_pCurrentScript = this;

    sOldDir = FileManager.GetWorkingDirectory();

    // Change to the directory of the .cfg file
    if (!m_sFilename.empty())
        FileManager.SetWorkingDirectory(Filename_GetPath(m_sFilename));

    while (m_itCurrentCmd != m_vCmdBuffer.end())
    {
        m_bIncrement = true;

        if (m_dwNextExecuteTime && m_dwNextExecuteTime > Host.GetTime())
        {
            // Change back to the original dir
            FileManager.SetWorkingDirectory(sOldDir);
            g_pCurrentScript = pPrevScript;
            ICvar::StoreCvars(bWasStoring);
            return false;
        }

        m_dwNextExecuteTime = 0;

        Console.Execute(*m_itCurrentCmd);

        if (m_itCurrentCmd == m_vCmdBuffer.end())
            break;

        if (m_dwNextExecuteTime && m_dwNextExecuteTime == Host.GetTime())
        {
            if (m_bIncrement)
                ++m_itCurrentCmd;

            // Change back to the original dir
            FileManager.SetWorkingDirectory(sOldDir);
            g_pCurrentScript = pPrevScript;
            ICvar::StoreCvars(bWasStoring);
            return false;
        }

        if (m_bIncrement)
            ++m_itCurrentCmd;
    }

    // Change back to the original dir
    FileManager.SetWorkingDirectory(sOldDir);
    g_pCurrentScript = pPrevScript;
    ICvar::StoreCvars(bWasStoring);
    return true;
}

/*====================
  CScript::GetParameter
  ====================*/
tstring CScript::GetParameter(const tstring &sName)
{
    tsmapts::iterator it(m_mapParams.find(sName));

    if (it != m_mapParams.end())
        return it->second;
    else
        return _T("");
}

/*====================
  CScript::AddParameter
  ====================*/
void    CScript::AddParameter(const tstring &sName, const tstring &sValue)
{
    m_mapParams[sName] = sValue;
}

/*--------------------
  GetScriptParam
  --------------------*/
FUNCTION(GetScriptParam)
{
    if (vArgList.size() < 1 || g_pCurrentScript == NULL)
        return _T("");

    return g_pCurrentScript->GetParameter(vArgList[0]);
}
