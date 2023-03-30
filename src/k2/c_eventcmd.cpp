// (C)2006 S2 Games
// c_eventcmd.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_eventcmd.h"
#include "c_eventcmdregistry.h"

#include "../k2/c_cmd.h"
//=============================================================================

/*====================
  CEventCmd::CEventCmd
  ====================*/
CEventCmd::CEventCmd(const tstring &sName, EventCmdFn_t pfnEventCmd, int iFlags) :
m_sName(sName),
m_pfnEventCmd(pfnEventCmd),
m_iFlags(iFlags)
{
	if (m_pfnEventCmd == NULL)
		K2System.Error(_T("Tried to register an EventCmd with a NULL function."));

	CEventCmdRegistry::GetInstance()->Register(this);
}


/*====================
  CEventCmd::~CEventCmd
  ====================*/
CEventCmd::~CEventCmd()
{
	// If the registry is still valid, unregister the eventcmd
	// This is important for any actions declared in a client dll that
	// is being unloaded
	if (!CEventCmdRegistry::IsReleased())
		CEventCmdRegistry::GetInstance()->Unregister(this);
}


/*====================
  CEventCmd::Execute
  ====================*/
bool	CEventCmd::Execute(const tsvector &vArgList, int iTimeNudge)
{
	return m_pfnEventCmd(vArgList, iTimeNudge);
}


/*====================
  CEventCmd::operator()
  ====================*/
bool	CEventCmd::operator()(const tsvector &vArgList, int iTimeNudge)
{
	return m_pfnEventCmd(vArgList, iTimeNudge);
}


/*--------------------
  cmdEventCmdList

  prints a list of all ui commands
  --------------------*/
CMD(EventCmdList)
{
	int iNumFound(0);

	const EventCmdMap &lCmds = CEventCmdRegistry::GetInstance()->GetEventCmdMap();

	// Print eventcmds
	for (EventCmdMap::const_iterator it(lCmds.begin()); it != lCmds.end(); ++it)
	{
		if (vArgList.size() == 0 || it->second->GetName().find(vArgList[0]) != string::npos)
		{
			Console << it->second->GetName() << newl;
			++iNumFound;
		}
	}

	Console << newl << iNumFound << _T(" matching ui commands found") << newl;

	return true;
}
