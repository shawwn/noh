// (C)2007 S2 Games
// c_triggermanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_server_common.h"

#include "c_gameserver.h"
#include "c_triggermanager.h"

#include "../k2/c_function.h"
#include "../k2/c_script.h"

#include "../hon_shared/c_player.h"
#include "../hon_shared/i_visualentity.h"
#include "../hon_shared/i_entitystate.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CTriggerManager&	TriggerManager(*CTriggerManager::GetInstance());
SINGLETON_INIT(CTriggerManager)
//=============================================================================

/*====================
  CTriggerManager::CTriggerManager
  ====================*/
CTriggerManager::CTriggerManager()
{
}

/*====================
  CTriggerManager::RegisterTriggerParam
  ====================*/
void	CTriggerManager::RegisterTriggerParam(const tstring &sName, const tstring &sValue)
{
	m_mapTriggerParams[sName] = sValue;
}

/*====================
  CTriggerManager::RegisterEntityScript
  ====================*/
void	CTriggerManager::RegisterEntityScript(uint uiIndex, const tstring &sName, const tstring &sScript)
{
	map<uint, tsmapts>::iterator findit(m_mapEntityScripts.find(uiIndex));

	if (findit != m_mapEntityScripts.end())
		findit->second[sName] = sScript;
	else
	{
		tsmapts mapScript;
		mapScript[sName] = sScript;

		m_mapEntityScripts[uiIndex] = mapScript;
	}
}


/*====================
  CTriggerManager::CopyEntityScripts
  ====================*/
void	CTriggerManager::CopyEntityScripts(uint uiFromIndex, uint uiToIndex)
{
	ClearEntityScripts(uiToIndex);

	map<uint, tsmapts>::iterator findit(m_mapEntityScripts.find(uiFromIndex));

	if (findit != m_mapEntityScripts.end())
		m_mapEntityScripts[uiToIndex] = findit->second;
}


/*====================
  CTriggerManager::TriggerEntityScript
  ====================*/
bool	CTriggerManager::TriggerEntityScript(uint uiIndex, const tstring &sName)
{
	return true;
}


/*====================
  CTriggerManager::TriggerGlobalScript
  ====================*/
bool	CTriggerManager::TriggerGlobalScript(const tstring &sName)
{
	return true;
}
