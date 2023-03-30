// (C)2005 S2 Games
// c_consoleregistry.h
//
//=============================================================================
#ifndef __C_CONSOLEREGISTRY_H__
#define __C_CONSOLEREGISTRY_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_singleton.h"
#include "c_consoleelement.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class ICvar;
class CCmd;
class CAlias;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef map<tstring, CConsoleElement*>			ConsoleElementMap;

typedef map<tstring, CConsoleElement*>			ElementList;
typedef ElementList::iterator					ElementList_it;
typedef ElementList::const_iterator				ElementList_cit;

typedef map<uint, ElementList>					ElementListMap;
typedef ElementListMap::iterator				ElementListMap_it;

typedef map<tstring, ICvar*>					CvarList;
typedef CvarList::iterator						CvarList_it;
typedef CvarList::const_iterator				CvarList_cit;
typedef map<uint, CvarList>						CvarListMap;
typedef CvarListMap::iterator					CvarListMap_it;
//=============================================================================

//=============================================================================
// CConsoleRegistry
//=============================================================================
class CConsoleRegistry
{
SINGLETON_DEF(CConsoleRegistry)

private:
	ConsoleElementMap	m_mapElements;

	CvarList			m_lCvars;
	CvarListMap			m_mapCvarLists;

	ElementList			m_lCmds;
	ElementList			m_lCmdPrecaches;
	ElementList			m_lAliases;
	ElementList			m_lFunctions;
	ElementList			m_lGameBinds;

public:
	~CConsoleRegistry();

	void						Register(const tstring &sName, CConsoleElement *pElement);
	K2_API void					Unregister(CConsoleElement *pElement);
	K2_API void					Unregister(const tstring &sName);

	void						AddCvar(ICvar *cvar);
	void						RemoveCvar(ICvar *cvar);

	void						AddCmd(CConsoleElement *cmd);
	void						RemoveCmd(CConsoleElement *cmd);

	void						AddPrecacheCmd(CConsoleElement *cmd);
	void						RemovePrecacheCmd(CConsoleElement *cmd);

	void						AddAlias(CConsoleElement *alias);
	void						RemoveAlias(CConsoleElement *alias);

	void						AddFunction(CConsoleElement *function);
	void						RemoveFunction(CConsoleElement *function);

	void						AddGameBind(CConsoleElement *pGameBind);
	void						RemoveGameBind(CConsoleElement *pGameBind);

	const CvarList&				GetCvarList()				{ return m_lCvars; }
	const CvarList&				GetCvarList(EConsoleElementFlag eFlag);
	const ElementList&			GetCmdList()				{ return m_lCmds; }
	const ElementList&			GetPrecacheCmdList()		{ return m_lCmdPrecaches; }
	const ElementList&			GetAliasList()				{ return m_lAliases; }
	const ElementList&			GetFunctionList()			{ return m_lFunctions; }
	const ElementList&			GetGameBindList()			{ return m_lGameBinds; }
	const ConsoleElementMap&	GetConsoleElementMap()		{ return m_mapElements; }

	K2_API bool				Exists(const tstring &sName);

	K2_API CConsoleElement*	GetElement(const tstring &sName);
	K2_API ICvar*			GetCvar(const tstring &sName);

	tstring						CompleteString(const tstring &str, bool bPrintMatches, bool &bPerfectMatch);
};
//=============================================================================

extern K2_API CConsoleRegistry *pConsoleRegistry;
#define ConsoleRegistry (*pConsoleRegistry)
#ifdef K2_EXPORTS
#define pConsoleRegistry CConsoleRegistry::GetInstance()
#endif

#endif //__C_CONSOLEREGISTRY_H__
