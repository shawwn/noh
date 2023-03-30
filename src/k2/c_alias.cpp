// (C)2005 S2 Games
// c_alias.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_alias.h"
#include "c_consoleregistry.h"
#include "c_cmd.h"
#include "c_filehandle.h"
#include "c_xmldoc.h"
#include "stringutils.h"
//=============================================================================


/*====================
  Alias_Cmd
  ====================*/
bool    Alias_Cmd(CConsoleElement *pElem, const tsvector &vArgList)
{
    assert(pElem->GetType() == ELEMENT_ALIAS);  // don't pass non - aliases into this function!

    // TODO: allow dos style command argument relays? (i.e %1 %2 %3, etc)
    Console.Execute(static_cast<CAlias *>(pElem)->GetCmd());

    return 0;
}


/*====================
  CAlias::CAlias
  ====================*/
CAlias::CAlias(const tstring &sName, const tstring &sCmd, int iFlags) :
CConsoleElement(sName, iFlags, ELEMENT_ALIAS, Alias_Cmd),
m_sCmd(sCmd)
{
    ConsoleRegistry.Register(sName, this);
    ConsoleRegistry.AddAlias(this);
}


/*====================
  CAlias::CAlias
  ====================*/
CAlias::~CAlias()
{
    ConsoleRegistry.Unregister(this);
    ConsoleRegistry.RemoveAlias(this);
}


/*====================
  CAlias::Create
  ====================*/
CAlias* CAlias::Create(const tstring &sName, const tstring &sCmd)
{
    CConsoleElement *pElem = ConsoleRegistry.GetElement(sName);

    if (pElem && pElem->GetType() == ELEMENT_ALIAS)
        static_cast<CAlias*>(pElem)->Set(sCmd);
    else if (pElem)
        Console << _T("Console element ") << pElem->GetName() << _T(" already exists") << newl;
    else
        return K2_NEW(ctx_Console,  CAlias)(sName, sCmd, CONEL_DYNAMIC | ALIAS_SAVECONFIG);

    return NULL;
}


/*====================
  CAlias::Find
  ====================*/
CAlias* CAlias::Find(const tstring &sName)
{
    CConsoleElement *pElem = ConsoleRegistry.GetElement(sName);

    if (pElem && pElem->GetType() == ELEMENT_ALIAS)
        return static_cast<CAlias*>(pElem);
    else
        return NULL;
}


/*====================
  CAlias::Write
  ====================*/
void    CAlias::Write(CFileHandle &hFile, const tstring &sWildcard, int iFlags)
{
    if ((!sWildcard.empty() && CompareNum(GetName(), sWildcard, sWildcard.length()) == 0) ||
        HasFlags(iFlags))
    {
        hFile << _T("Alias ") << QuoteStr(AddEscapeChars(GetName())) << _T(" ") << QuoteStr(AddEscapeChars(GetString())) << newl;
    }

}


/*====================
  CAlias::Write
  ====================*/
void    CAlias::Write(CXMLDoc &xmlConfig, const tstring &sWildcard, int iFlags)
{
    if ((!sWildcard.empty() && CompareNum(GetName(), sWildcard, sWildcard.length()) == 0) ||
        HasFlags(iFlags))
    {
        xmlConfig.NewNode("alias");
        xmlConfig.AddProperty("name", GetName());
        xmlConfig.AddProperty("cmd", GetString());
        xmlConfig.EndNode();
    }
}


/*====================
  CAlias::WriteConfigFile
  ====================*/
bool    CAlias::WriteConfigFile(CFileHandle &hFile, const tsvector &wildcards, int iFlags)
{
    if (wildcards.empty())
    {
        // write variables
        const ElementList &lAliases = ConsoleRegistry.GetAliasList();

        // loop through the cvar list
        for (ElementList::const_iterator it(lAliases.begin()); it != lAliases.end(); ++it)
            static_cast<CAlias *>(it->second)->Write(hFile, _T(""), iFlags);
    }
    else
    {
        const ElementList &lAliases = ConsoleRegistry.GetAliasList();

        for (size_t i(0); i < wildcards.size(); ++i)
        {
            //write variables
            for (ElementList::const_iterator it(lAliases.begin()); it != lAliases.end(); ++it)
                static_cast<CAlias *>(it->second)->Write(hFile, wildcards[i], iFlags);
        }
    }

    return true;
}


/*====================
  CAlias::WriteConfigFile
  ====================*/
bool    CAlias::WriteConfigFile(CXMLDoc &xmlConfig, const tsvector &wildcards, int iFlags)
{
    if (wildcards.empty())
    {
        //write variables
        const ElementList &lAliases = ConsoleRegistry.GetAliasList();

        // loop through the cvar list
        for (ElementList::const_iterator it(lAliases.begin()); it != lAliases.end(); ++it)
            static_cast<CAlias *>(it->second)->Write(xmlConfig, _T(""), iFlags);
    }
    else
    {
        const ElementList &lAliases = ConsoleRegistry.GetAliasList();

        for (size_t i(0); i < wildcards.size(); ++i)
        {
            //write variables
            for (ElementList::const_iterator it(lAliases.begin()); it != lAliases.end(); ++it)
                static_cast<CAlias *>(it->second)->Write(xmlConfig, wildcards[i], iFlags);
        }
    }

    return true;
}


/*--------------------
  cmdAlias

  aliases a new console element to a new command string
  --------------------*/
CMD(Alias)
{
    if (vArgList.size() < 2)
    {
        Console << _T("syntax: alias <alias name> <command>") << newl;
        return false;
    }

    CConsoleElement *pConsoleElement = ConsoleRegistry.GetElement(vArgList[0]);
    CAlias *pAlias = NULL;

    if (pConsoleElement && pConsoleElement->GetType() != ELEMENT_ALIAS)
    {
        Console << _T("Console element ") << pElem->GetName() << _T(" already exists") << newl;
        return false;
    }
    else if (pConsoleElement && pConsoleElement->GetType() == ELEMENT_ALIAS)
    {
        pAlias = static_cast<CAlias *>(pConsoleElement);
    }
    else
    {
        pAlias = CAlias::Create(vArgList[0], _T(""));
    }

    if (pAlias)
        pAlias->Set(ConcatinateArgs(vArgList.begin() + 1, vArgList.end()));

    return true;
}


/*--------------------
  cmdUnalias

  deletes an existing command alias
  --------------------*/
CMD(Unalias)
{
    if (vArgList.size() < 1)
    {
        Console << _T("syntax: unalias <alias name>") << newl;
        return true;
    }

    CAlias *pAlias = CAlias::Find(vArgList[0]);

    if (pAlias)
        K2_DELETE(pAlias);

    return false;
}


/*--------------------
  cmdUnaliasAll

  deletes all existing command alias
  --------------------*/
CMD(UnaliasAll)
{
    const ElementList &lAliases = ConsoleRegistry.GetAliasList();

    // loop through the alias list
    for (ElementList::const_iterator it = lAliases.begin(); it != lAliases.end(); ++it)
        K2_DELETE(it->second);

    return false;
}


/*--------------------
  cmdAliasList

  Display a list of all the aliased commands
  --------------------*/
CMD(AliasList)
{
    int iNumFound(0);

    const ElementList &lAliases = ConsoleRegistry.GetAliasList();

    // loop through the cvar list
    for (ElementList::const_iterator it = lAliases.begin(); it != lAliases.end(); ++it)
    {
        if (vArgList.size() == 0 || (it->second)->GetName().find(vArgList[0]) != tstring::npos)
        {
            Console << (it->second)->GetName() << _T(" : ") << (it->second)->GetString() << newl;
            ++iNumFound;
        }
    }

    Console << newl << iNumFound << _T(" matching aliases found") << newl;
    return true;
}
