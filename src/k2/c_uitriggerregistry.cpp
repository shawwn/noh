// (C)2005 S2 Games
// c_uitriggerregistry.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_uitriggerregistry.h"
#include "c_uitrigger.h"

#include "../k2/stringutils.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
SINGLETON_INIT(CUITriggerRegistry);

CUITriggerRegistry	*g_pUITriggerRegistry(CUITriggerRegistry::GetInstance());
//=============================================================================

/*====================
  CUITriggerRegistry::CUITriggerRegistry
  ====================*/
CUITriggerRegistry::CUITriggerRegistry()
{
}


/*====================
  CUITriggerRegistry::Register
  ====================*/
void	CUITriggerRegistry::Register(CUITrigger *pUITrigger)
{
	// Make sure there is no name collision
	UITriggerMap_it itFindTrigger(m_mapUITriggers.find(pUITrigger->GetName()));
	if (itFindTrigger != m_mapUITriggers.end())
	{
		Console.Err << _T("A UI trigger named ") << QuoteStr(pUITrigger->GetName()) << _T(" already exists.") << newl;
		return;
	}

	// Tell watchers that this trigger exists now
	UIWatcherMap_it itFindWatcherList(m_mapWatchers.find(pUITrigger->GetName()));
	if (itFindWatcherList != m_mapWatchers.end())
	{
		for (UIWatcherList_it itWatcher(itFindWatcherList->second.begin()); itWatcher != itFindWatcherList->second.end(); ++itWatcher)
			(*itWatcher)->SetTrigger(pUITrigger);
	}

	m_mapUITriggers[pUITrigger->GetName()] = pUITrigger;
}

void	CUITriggerRegistry::Register(CUIWatcher *pUIWatcher)
{
	m_mapWatchers[pUIWatcher->GetTriggerName()].push_back(pUIWatcher);
}


/*====================
  CUITriggerRegistry::Unregister
  ====================*/
void	CUITriggerRegistry::Unregister(CUITrigger *pUITrigger)
{
	UITriggerMap_it itFindTrigger(m_mapUITriggers.find(pUITrigger->GetName()));
	if (itFindTrigger != m_mapUITriggers.end())
		m_mapUITriggers.erase(itFindTrigger);

	UIWatcherMap_it itFindWatcherList(m_mapWatchers.find(pUITrigger->GetName()));
	if (itFindWatcherList != m_mapWatchers.end())
	{
		for (UIWatcherList_it it(itFindWatcherList->second.begin()); it != itFindWatcherList->second.end(); ++it)
			(*it)->SetTrigger(NULL);
	}

	// Console.Dev << _T("UI Trigger ") << QuoteStr(pUITrigger->GetName()) << _T(" has been unregistered.") << newl;
}

void	CUITriggerRegistry::Unregister(CUIWatcher *pWatcher)
{
	UIWatcherMap_it itFind(m_mapWatchers.find(pWatcher->GetTriggerName()));
	if (itFind == m_mapWatchers.end())
		return;

	itFind->second.remove(pWatcher);
}
