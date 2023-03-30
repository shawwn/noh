// (C)2005 S2 Games
// c_uitrigger.cpp
//
// Self registering shader variable controllers
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_uitrigger.h"
#include "c_uitriggerregistry.h"
#include "i_widget.h"
#include "c_uimanager.h"
#include "c_cmd.h"
#include "c_uicmd.h"
#include "c_interface.h"
//=============================================================================

/*====================
  CUITrigger::CUITrigger
  ====================*/
CUITrigger::CUITrigger(const tstring &sName) :
m_sName(sName),
m_vlWatchers(10)
{
	UITriggerRegistry.Register(this);
}


/*====================
  CUITrigger::~CUITrigger
  ====================*/
CUITrigger::~CUITrigger()
{
	// If the registry is still valid, unregister the trigger
	if (!UITriggerRegistry.IsReleased())
		UITriggerRegistry.Unregister(this);

	// Notify all widgets that are watching it that it is no longer valid
	for (WatcherList::iterator it(m_lWatchers.begin()); it != m_lWatchers.end(); ++it)
		(*it)->StopWatching(this);
}


/*====================
  CUITrigger::AddWatcher
  ====================*/
void	CUITrigger::AddWatcher(IWidget *pWidget, int iIndex)
{
	if (iIndex == -1)
	{
		m_lWatchers.push_back(pWidget);
		return;
	}

	if (iIndex < 0 || iIndex > 9)
		return;

	m_vlWatchers[iIndex].push_back(pWidget);
}


/*====================
  CUITrigger::RemoveWatcher
  ====================*/
void	CUITrigger::RemoveWatcher(IWidget *pWidget, int iIndex)
{
	if (iIndex == -1)
	{
		m_lWatchers.remove(pWidget);
		return;
	}

	if (iIndex < 0 || iIndex > 9)
		return;

	m_vlWatchers[iIndex].remove(pWidget);
}


/*====================
  CUITrigger::Trigger
  ====================*/
void	CUITrigger::Trigger(const tstring &sParam, bool bForce)
{
	PROFILE("CUITrigger::Trigger");

	m_sLastParam = sParam;

	CInterface *pSavedInterface(UIManager.GetSavedActiveInterface());
	CInterface *pActiveInterface(UIManager.GetActiveInterface());

	for (WatcherList::iterator it(m_lWatchers.begin()), itEnd(m_lWatchers.end()); it != itEnd; ++it)
	{
		IWidget *pWidget(*it);
		if (pWidget == NULL)
			continue;
		if (!bForce &&
			pWidget->GetInterface() != NULL &&
			pWidget->GetInterface() != pSavedInterface &&
			pWidget->GetInterface() != pActiveInterface &&
			!pWidget->GetInterface()->AlwaysUpdate())
			continue;

		pWidget->DoEvent(WEVENT_TRIGGER, sParam);
	}

	for (int i(0); i <= 9; ++i)
	{
		for (WatcherList::iterator it(m_vlWatchers[i].begin()), itEnd(m_vlWatchers[i].end()); it != itEnd; ++it)
		{
			IWidget *pWidget(*it);
			if (pWidget == NULL)
				continue;
			if (!bForce &&
				pWidget->GetInterface() != NULL &&
				pWidget->GetInterface() != pSavedInterface &&
				pWidget->GetInterface() != pActiveInterface &&
				!pWidget->GetInterface()->AlwaysUpdate())
				continue;

			pWidget->DoEvent(EWidgetEvent(WEVENT_TRIGGER0 + i), sParam);
		}
	}
}

void	CUITrigger::Trigger(const tsvector &vParams, bool bForce)
{
	PROFILE("CUITrigger::Trigger");

	CInterface *pSavedInterface(UIManager.GetSavedActiveInterface());
	CInterface *pActiveInterface(UIManager.GetActiveInterface());

	for (WatcherList::iterator it(m_lWatchers.begin()), itEnd(m_lWatchers.end()); it != itEnd; ++it)
	{
		IWidget *pWidget(*it);
		if (pWidget == NULL)
			continue;
		if (!bForce &&
			pWidget->GetInterface() != NULL &&
			pWidget->GetInterface() != pSavedInterface &&
			pWidget->GetInterface() != pActiveInterface &&
			!pWidget->GetInterface()->AlwaysUpdate())
			continue;

		pWidget->DoEvent(WEVENT_TRIGGER, vParams);
	}

	for (int i(0); i <= 9; ++i)
	{
		for (WatcherList::iterator it(m_vlWatchers[i].begin()), itEnd(m_vlWatchers[i].end()); it != itEnd; ++it)
		{
			IWidget *pWidget(*it);
			if (pWidget == NULL)
				continue;
			if (!bForce &&
				pWidget->GetInterface() != NULL &&
				pWidget->GetInterface() != pSavedInterface  &&
				pWidget->GetInterface() != pActiveInterface  &&
				!pWidget->GetInterface()->AlwaysUpdate())
				continue;

			pWidget->DoEvent(EWidgetEvent(WEVENT_TRIGGER0 + i), vParams);
		}
	}
}


/*====================
  CUITrigger::Hide
  ====================*/
void	CUITrigger::Hide()
{
	for (WatcherList::iterator it = m_lWatchers.begin(); it != m_lWatchers.end(); ++it)
		if (*it != NULL)
			(*it)->Hide();

	for (int i(0); i <= 9; ++i)
	{
		for (WatcherList::iterator it = m_vlWatchers[i].begin(); it != m_vlWatchers[i].end(); ++it)
			if (*it != NULL)
				(*it)->Hide();
	}
}


/*====================
  CUITrigger::Show
  ====================*/
void	CUITrigger::Show()
{
	for (WatcherList::iterator it = m_lWatchers.begin(); it != m_lWatchers.end(); ++it)
		if (*it != NULL)
			(*it)->Show();

	for (int i(0); i <= 9; ++i)
	{
		for (WatcherList::iterator it = m_vlWatchers[i].begin(); it != m_vlWatchers[i].end(); ++it)
			if (*it != NULL)
				(*it)->Show();
	}
}


/*====================
  CUITrigger::IsVisible
  ====================*/
bool	CUITrigger::IsVisible()
{
	for (WatcherList::iterator it = m_lWatchers.begin(); it != m_lWatchers.end(); ++it)
	{
		if (*it == NULL || !(*it)->IsAbsoluteVisible() )
			return false;
	}
	return true;
}


/*====================
  CUITrigger::Execute
  ====================*/
void	CUITrigger::Execute(const tstring &sCmd)
{
	for (WatcherList::iterator it(m_lWatchers.begin()), itEnd(m_lWatchers.end()); it != itEnd; ++it)
		if (*it != NULL)
			(*it)->Execute(sCmd);

	for (int i(0); i <= 9; ++i)
	{
		for (WatcherList::iterator it(m_vlWatchers[i].begin()), itEnd(m_vlWatchers[i].end()); it != itEnd; ++it)
			if (*it != NULL)
				(*it)->Execute(sCmd);
	}
}

void	CUITrigger::Execute(const tstring &sCmd, IBuffer &buffer)
{
	for (WatcherList::iterator it(m_lWatchers.begin()), itEnd(m_lWatchers.end()); it != itEnd; ++it)
		if (*it != NULL)
			(*it)->Execute(sCmd, buffer);

	for (int i(0); i <= 9; ++i)
	{
		for (WatcherList::iterator it(m_vlWatchers[i].begin()), itEnd(m_vlWatchers[i].end()); it != itEnd; ++it)
			if (*it != NULL)
				(*it)->Execute(sCmd, buffer);
	}
}


/*====================
  CUIWatcher::~CUIWatcher
  ====================*/
CUIWatcher::~CUIWatcher()
{
	if (m_pTrigger != NULL)
		m_pTrigger->RemoveWatcher(m_pWidget, m_iIndex);

	UITriggerRegistry.Unregister(this);
}


/*====================
  CUIWatcher::CUIWatcher
  ====================*/
CUIWatcher::CUIWatcher(IWidget *pWidget, const tstring &sTriggerName, int iIndex) :
m_sTriggerName(sTriggerName),
m_pTrigger(UITriggerRegistry.GetUITrigger(m_sTriggerName)),
m_pWidget(pWidget),
m_iIndex(iIndex)
{
	if (m_pTrigger != NULL && m_pWidget != NULL)
		m_pTrigger->AddWatcher(m_pWidget, iIndex);

	UITriggerRegistry.Register(this);
}


/*====================
  CUIWatcher::SetTrigger
  ====================*/
void	CUIWatcher::SetTrigger(CUITrigger *pTrigger)
{
	if (m_pTrigger == pTrigger)
		return;

	if (m_pTrigger != NULL)
		m_pTrigger->RemoveWatcher(m_pWidget, m_iIndex);

	m_pTrigger = pTrigger;
	
	if (m_pTrigger != NULL)
		pTrigger->AddWatcher(m_pWidget, m_iIndex);
}


/*--------------------
  UITriggerList
  --------------------*/
CMD(UITriggerList)
{
	const UITriggerMap &lVars(UITriggerRegistry.GetUITriggerMap());

	int iNumFound(0);
	for (UITriggerMap::const_iterator it(lVars.begin()); it != lVars.end(); ++it)
	{
		if (vArgList.size() == 0 || it->second->GetName().find(vArgList[0]) != string::npos)
		{
			if (it->second->GetLastParam().empty())
				Console << it->second->GetName() << newl;
			else
				Console << it->second->GetName() << _T(" = ") << QuoteStr(it->second->GetLastParam()) << newl;

			++iNumFound;
		}
	}

	Console << newl << iNumFound << _T(" matching UITriggers found") << newl;
	return true;
}


/*--------------------
  Trigger
  --------------------*/
CMD(Trigger)
{
	if (vArgList.empty())
	{
		Console << _T("Syntax: Trigger <uitrigger name> [param1] ...") << newl;
		return false;
	}

	CUITrigger *pTrigger(UITriggerRegistry.GetUITrigger(vArgList[0]));
	if (pTrigger == NULL)
	{
		Console << _T("Trigger not found") << newl;
		return false;
	}

	if (vArgList.size() < 2)
		pTrigger->Trigger(TSNULL);
	if (vArgList.size() == 2)
		pTrigger->Trigger(vArgList[1]);
	else
		pTrigger->Trigger(tsvector(vArgList.begin() + 1, vArgList.end()));

	return true;
}

UI_VOID_CMD(Trigger, 1)
{
	CUITrigger *pTrigger(UITriggerRegistry.GetUITrigger(vArgList[0]->Evaluate()));
	if (pTrigger == NULL)
	{
		Console << _T("Trigger not found") << newl;
		return;
	}

	tsvector vParams;
	for (ScriptTokenVector_cit it(vArgList.begin() + 1); it != vArgList.end(); ++it)
		vParams.push_back((*it)->Evaluate());

	pTrigger->Trigger(vParams);
}

UI_CMD(ExplodeTriggerCount, 1)
{
	tstring sSeperator(_T("|"));
	tstring sList(vArgList[0]->Evaluate());

	if (sList.size() == 0)
		return XtoA(0);
	
	if (vArgList.size() >= 2)
		sSeperator = vArgList[1]->Evaluate();

	uint	iFoundTimes(1);
	size_t	zLastPosition(0);
	size_t	zPosition = sList.find(sSeperator, 0);

	while(iFoundTimes < sList.length() + 1)
	{
		if (zPosition == -1)
			break;

		zLastPosition = zPosition + sSeperator.length();
		zPosition = sList.find(sSeperator, zPosition + sSeperator.length());
		++iFoundTimes;
	}

	return XtoA(iFoundTimes);
}

UI_VOID_CMD(ExplodeTrigger, 2)
{

	CUITrigger *pTrigger(UITriggerRegistry.GetUITrigger(vArgList[0]->Evaluate()));
	if (pTrigger == NULL)
	{
		Console << _T("Trigger not found") << newl;
		return;
	}

	tstring sSeperator(_T("|"));
	tstring sSeperator2(_T(":"));
	tstring sList(vArgList[1]->Evaluate());
	tstring sNameTmp(_T(""));
	tstring sValueTmp(_T(""));
	uint	uiTriggerStart(0);
	uint	uiTriggerCount(-1);
	tsvector vParams;

	if (sList.size() == 0)
		return;
	
	if (vArgList.size() >= 3)
		sSeperator = vArgList[2]->Evaluate();

	if (vArgList.size() >= 4)
		sSeperator2 = vArgList[3]->Evaluate();

	if (vArgList.size() >= 5)
		uiTriggerStart = vArgList[4]->EvaluateInteger() + 1;

	if (vArgList.size() >= 6)
		uiTriggerCount = vArgList[5]->EvaluateInteger();

	uint	uiFoundTimes(1);
	size_t	zLastPosition(0);
	size_t	zPosition = sList.find(sSeperator, 0);

	while(uiFoundTimes < sList.length() + 1)
	{
		sValueTmp = sList.substr(zLastPosition, (zPosition == -1) ? (sList.length() - zLastPosition) : (zPosition - zLastPosition));

		vParams.clear();
		vParams.push_back(XtoA(uiFoundTimes));

		uint	uiSubFoundTimes(1);
		size_t	zSubLastPosition(0);
		size_t	zSubPosition = sValueTmp.find(sSeperator2, 0);
		tstring sSubValueTmp(_T(""));
		if(zSubPosition != -1)
		{
			while(uiSubFoundTimes < sValueTmp.length())
			{
				sSubValueTmp = sValueTmp.substr(zSubLastPosition, (zSubPosition == -1) ? (sValueTmp.length() - zSubLastPosition) : (zSubPosition - zSubLastPosition));

				vParams.push_back(sSubValueTmp);

				if (zSubPosition == -1)
					break;

				zSubLastPosition = zSubPosition + sSeperator2.length();
				zSubPosition = sValueTmp.find(sSeperator2, zSubPosition + sSeperator2.length());
				++uiSubFoundTimes;
			}
		}
		else
			vParams.push_back(sValueTmp);

		if (uiFoundTimes >= uiTriggerStart)
			pTrigger->Trigger(vParams);

		if (zPosition == -1)
			break;

		zLastPosition = zPosition + sSeperator.length();
		zPosition = sList.find(sSeperator, zPosition + sSeperator.length());
		++uiFoundTimes;

		if (uiFoundTimes >= uiTriggerCount + uiTriggerStart )
			break;
	}
}
