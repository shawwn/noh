// (C)2005 S2 Games
// c_consoleregistry.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "stringutils.h"
#include "c_consoleregistry.h"
#include "c_cmd.h"
#include "c_cvar.h"

#undef pConsoleRegistry
#undef ConsoleRegistry
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
extern CCvar<tstring> con_prompt;

CConsoleRegistry *pConsoleRegistry = CConsoleRegistry::GetInstance();

SINGLETON_INIT(CConsoleRegistry)
//=============================================================================

/*====================
  CConsoleRegistry::CConsoleRegistry
  ====================*/
CConsoleRegistry::CConsoleRegistry()
{
}


/*====================
  CConsoleRegistry::~CConsoleRegistry
  ====================*/
CConsoleRegistry::~CConsoleRegistry()
{
    for (ConsoleElementMap::iterator it = m_mapElements.begin(); it != m_mapElements.end(); ++it)
    {
        if (it->second->HasFlags(CONEL_DYNAMIC))
            K2_DELETE(it->second);
    }
}


/*====================
  CConsoleRegistry::Register
  ====================*/
void    CConsoleRegistry::Register(const tstring &sName, CConsoleElement *pElement)
{
    const tstring sLowerName(LowerString(sName));

    if (m_mapElements.find(sLowerName) != m_mapElements.end())
        Console.Err << "Console element " << QuoteStr(sName) << " already exists" << newl;

    m_mapElements[sLowerName] = pElement;
}


/*====================
  CConsoleRegistry::Unregister
  ====================*/
void    CConsoleRegistry::Unregister(CConsoleElement *pElement)
{
    try
    {
        if (pElement == NULL)
        {
            EX_WARN(_T("CConsoleRegistry::Unregister() - NULL element detected."));
            return;
        }
            
        const tstring sLowerName(LowerString(pElement->GetName()));
        ConsoleElementMap::iterator findit(m_mapElements.find(sLowerName));
        if (findit == m_mapElements.end())
            EX_WARN(_T("Element not found: ") + sLowerName);
        if (findit->second != pElement)
            EX_WARN(_T("Name mismatch: ") + sLowerName + _T(" != ") + findit->second->GetName());

        m_mapElements.erase(sLowerName);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CConsoleRegistry::Unregister() - "), NO_THROW);
    }
}

void    CConsoleRegistry::Unregister(const tstring &sName)
{
    try
    {
        const tstring sLowerName(LowerString(sName));
        ConsoleElementMap::iterator findit(m_mapElements.find(sLowerName));
        if (findit == m_mapElements.end())
            EX_WARN(_T("Element not found: ") + sLowerName);

        m_mapElements.erase(sLowerName);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CConsoleRegistry::Unregister() - "), NO_THROW);
    }
}


/*====================
  CConsoleRegistry::GetElement
  ====================*/
CConsoleElement*    CConsoleRegistry::GetElement(const tstring &sName)
{
    ConsoleElementMap::iterator findit(m_mapElements.find(LowerString(sName)));

    if (findit == m_mapElements.end())
        return NULL;
    else
        return findit->second;
}


/*====================
  CConsoleRegistry::GetCvar
  ====================*/
ICvar*  CConsoleRegistry::GetCvar(const tstring &sName)
{
    CConsoleElement* pElement(GetElement(sName));
    if (pElement == NULL || pElement->GetType() != ELEMENT_CVAR)
        return NULL;
    return static_cast<ICvar*>(pElement);
}


/*====================
 CConsoleRegistry::Exists
 ====================*/
bool    CConsoleRegistry::Exists(const tstring &sName)
{
    return GetElement(sName) != NULL;
}


/*====================
  CConsoleRegistry::CompleteString
  ====================*/
tstring CConsoleRegistry::CompleteString(const tstring &str, bool bPrintMatches, bool &bPerfectMatch)
{
    list<CConsoleElement *> lMatches;

    // build list of all matches
    for (ElementList_it it(m_mapElements.begin()); it != m_mapElements.end(); ++it)
    {
        if (!CompareNoCaseNum(str, it->second->GetName(), str.length()) && !it->second->HasFlags(CVAR_CHILD))
            lMatches.push_back(it->second);
    }

    if (lMatches.empty())
        return tstring();

    if (lMatches.size() == 1)
    {
        if (!(*lMatches.begin())->GetString().empty())
        {
            Console << con_prompt << Console.GetInputLine() << newl
                    << _T("  ")
                    << (*lMatches.begin())->GetName()
                    << _T(" = \"")
                    << (*lMatches.begin())->GetString()
                    << _T("\"") << newl;
        }

        bPerfectMatch = true;

        return (*lMatches.begin())->GetName();
    }

    if (bPrintMatches)
    {
        Console << con_prompt << Console.GetInputLine() << newl;

        for (list<CConsoleElement *>::iterator it = lMatches.begin(); it != lMatches.end(); ++it)
        {
            tstring s = (*it)->GetString();

            if (!s.empty())
                Console << _T("  ") << (*it)->GetName() << _T(" = \"") << s << _T("\"") << newl;
            else
                Console << _T("  ") << (*it)->GetName() << newl;
        }
    }

    // calculate the most common match
    tstring match(LowerString(str));
    bool bDone = false;
    size_t i = str.length();    // start at length of the initial string

    while (!bDone)
    {
        list<CConsoleElement *>::iterator it = lMatches.begin();

        const tstring &s = (*it)->GetName();

        if (i >= s.length())
            break;

        // see if all matches contain this character
        for (; it != lMatches.end(); ++it)
        {
            const tstring &s2 = (*it)->GetName();

            if (i >= s2.length() || tolower(s2[i]) != tolower(s[i]))
                break;
        }

        // if so then append it to the string
        if (it == lMatches.end())
            match += tolower(s[i]);
        else
            bDone = true;

        ++i;
    }

    bPerfectMatch = false;
    return match;
}


/*====================
  CConsoleRegistry::AddCvar
  ====================*/
void    CConsoleRegistry::AddCvar(ICvar *cvar)
{
    assert(cvar);

    if (!cvar->HasFlags(CVAR_CHILD))
    {
        for (uint ui(0); ui < sizeof(uint) * 8; ++ui)
        {
            if (cvar->HasFlags(BIT(ui)))
                m_mapCvarLists[BIT(ui)][LowerString(cvar->GetName())] = cvar;
        }
    }

    m_lCvars[LowerString(cvar->GetName())] = cvar;
}


/*====================
  CConsoleRegistry::RemoveCvar
  ====================*/
void    CConsoleRegistry::RemoveCvar(ICvar *cvar)
{
    assert(cvar);

    if (!cvar->HasFlags(CVAR_CHILD))
    {
        for (uint ui(0); ui < sizeof(uint) * 8; ++ui)
        {
            if (cvar->HasFlags(BIT(ui)))
                m_mapCvarLists[BIT(ui)].erase(LowerString(cvar->GetName()));
        }
    }
    m_lCvars.erase(LowerString(cvar->GetName()));
}


/*====================
  CConsoleRegistry::GetCvarList
  ====================*/
const CvarList& CConsoleRegistry::GetCvarList(EConsoleElementFlag eFlag)
{
    return m_mapCvarLists[eFlag];
}


/*====================
  CConsoleRegistry::AddCmd
  ====================*/
void    CConsoleRegistry::AddCmd(CConsoleElement *cmd)
{
    assert(cmd->GetType() == ELEMENT_CMD);
    m_lCmds[LowerString(cmd->GetName())] = cmd;
}

/*====================
  CConsoleRegistry::RemoveCmd
  ====================*/
void    CConsoleRegistry::RemoveCmd(CConsoleElement *cmd)
{
    assert(cmd->GetType() == ELEMENT_CMD);

    tstring sLowerName(LowerString(cmd->GetName()));

    m_lCmds.erase(sLowerName);
}

/*====================
  CConsoleRegistry::AddPrecacheCmd
  ====================*/
void    CConsoleRegistry::AddPrecacheCmd(CConsoleElement *cmd)
{
    assert(cmd->GetType() == ELEMENT_CMDPRECACHE);
    m_lCmdPrecaches[LowerString(cmd->GetName())] = cmd;
}

/*====================
  CConsoleRegistry::RemovePrecacheCmd
  ====================*/
void    CConsoleRegistry::RemovePrecacheCmd(CConsoleElement *cmd)
{
    assert(cmd->GetType() == ELEMENT_CMDPRECACHE);

    tstring sLowerName(LowerString(cmd->GetName()));
    m_lCmdPrecaches.erase(sLowerName);
}

/*====================
  CConsoleRegistry::AddAlias
  ====================*/
void    CConsoleRegistry::AddAlias(CConsoleElement *alias)
{
    assert(alias->GetType() == ELEMENT_ALIAS);
    m_lAliases[LowerString(alias->GetName())] = alias;
}


/*====================
  CConsoleRegistry::RemoveAlias
  ====================*/
void    CConsoleRegistry::RemoveAlias(CConsoleElement *alias)
{
    assert(alias->GetType() == ELEMENT_ALIAS);

    tstring sLowerName(alias->GetName());
    m_lAliases.erase(sLowerName);
}


/*====================
  CConsoleRegistry::AddFunction
  ====================*/
void    CConsoleRegistry::AddFunction(CConsoleElement *function)
{
    assert(function->GetType() == ELEMENT_FUNCTION);
    m_lFunctions[LowerString(function->GetName())] = function;
}


/*====================
  CConsoleRegistry::RemoveFunction
  ====================*/
void    CConsoleRegistry::RemoveFunction(CConsoleElement *function)
{
    assert(function->GetType() == ELEMENT_FUNCTION);

    tstring sLowerName(LowerString(function->GetName()));
    m_lFunctions.erase(sLowerName);
}


/*====================
  CConsoleRegistry::AddGameBind
  ====================*/
void    CConsoleRegistry::AddGameBind(CConsoleElement *pGameBind)
{
    assert(pGameBind->GetType() == ELEMENT_GAMEBIND);
    m_lGameBinds[LowerString(pGameBind->GetName())] = pGameBind;
}


/*====================
  CConsoleRegistry::RemoveGameBind
  ====================*/
void    CConsoleRegistry::RemoveGameBind(CConsoleElement *pGameBind)
{
    assert(pGameBind->GetType() == ELEMENT_GAMEBIND);

    tstring sLowerName(LowerString(pGameBind->GetName()));
    m_lGameBinds.erase(sLowerName);
}


/*--------------------
  cmdConsoleElementList

  Display a list of all the console elements
  --------------------*/
CMD(ConsoleElementList)
{
    int iNumFound(0);

    const ConsoleElementMap &mapElements = CConsoleRegistry::GetInstance()->GetConsoleElementMap();

    // loop through the cvar list
    for (ConsoleElementMap::const_iterator it = mapElements.begin(); it != mapElements.end(); ++it)
    {
        if (vArgList.size() == 0 || it->second->GetName().find(vArgList[0]) != tstring::npos)
        {
            Console << it->second->GetName() << newl;
            ++iNumFound;
        }
    }

    Console << newl << iNumFound << _T(" matching console elements found") << newl;
    return true;
}
