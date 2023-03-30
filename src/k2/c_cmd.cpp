// (C)2005 S2 Games
// c_cmd.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_cmd.h"
#include "c_consoleregistry.h"
#include "stringutils.h"
//=============================================================================

//=============================================================================
// Global Declarations
//=============================================================================
//=============================================================================

/*====================
  CCmd::CCmd
  ====================*/
CCmd::CCmd(const tstring &sName, ConsoleElementFn_t pfnCmd, int iFlags) :
CConsoleElement(sName, iFlags, ELEMENT_CMD, pfnCmd)
{
    assert(m_pfnCmd != NULL);
    ConsoleRegistry.Register(sName, this);
    ConsoleRegistry.AddCmd(this);
}


/*====================
  CCmd::~CCmd
  ====================*/
CCmd::~CCmd()
{
    ConsoleRegistry.Unregister(this);
    ConsoleRegistry.RemoveCmd(this);
}


/*====================
  CCmd::operator()

  Calls the associated function directly
  ====================*/
bool    CCmd::operator()(const tstring &s0, const tstring &s1, const tstring &s2, const tstring &s3, const tstring &s4,
                         const tstring &s5, const tstring &s6, const tstring &s7, const tstring &s8, const tstring &s9)
{
    tsvector    vArgList;
    if (!s0.empty()) { vArgList.push_back(s0);
    if (!s1.empty()) { vArgList.push_back(s1);
    if (!s2.empty()) { vArgList.push_back(s2);
    if (!s3.empty()) { vArgList.push_back(s3);
    if (!s4.empty()) { vArgList.push_back(s4);
    if (!s5.empty()) { vArgList.push_back(s5);
    if (!s6.empty()) { vArgList.push_back(s6);
    if (!s7.empty()) { vArgList.push_back(s7);
    if (!s8.empty()) { vArgList.push_back(s8);
    if (!s9.empty()) { vArgList.push_back(s9);
    }}}}}}}}}}

    return m_pfnCmd(this, vArgList);
}


/*====================
  CCmd::operator()

  Calls the associated function even more directly
  ====================*/
bool    CCmd::operator()(const tsvector &vArgList)
{
    return m_pfnCmd(this, vArgList);
}


/*--------------------
  cmdCmdList

  Display a list of all the console commands
  --------------------*/
CMD(CmdList)
{
    int iNumFound(0);
    tstring sSearch;

    if (vArgList.size() > 0)
        sSearch = LowerString(vArgList[0]);

    const ElementList &lCmds = ConsoleRegistry.GetCmdList();

    // loop through the cvar list
    for (ElementList::const_iterator it = lCmds.begin(); it != lCmds.end(); ++it)
    {
        if (sSearch.empty() || LowerString(it->second->GetName()).find(sSearch) != tstring::npos)
        {
            Console << (it->second)->GetName() << newl;
            ++iNumFound;
        }
    }

    Console << newl << iNumFound << _T(" matching commands found") << newl;
    return true;
}

