// (C)2010 S2 Games
// c_scriptdirectory.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_scriptdirectory.h"
#include "c_scriptthread.h"
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
CVAR_BOOLF(script_debug,	false,	CONEL_DEV);
//=============================================================================

/*====================
  CScriptDirectory::~CScriptDirectory
  ====================*/
CScriptDirectory::~CScriptDirectory()
{
}


/*====================
  CScriptDirectory::CScriptDirectory
  ====================*/
CScriptDirectory::CScriptDirectory()
{
}


/*====================
  CScriptDirectory::SpawnThread
  ====================*/
CScriptThread*	CScriptDirectory::SpawnThread(const tstring &sName, uint uiTime)
{
	PROFILE("CScriptDirectory::SpawnThread");

	CScriptThread *pDefinition(EntityRegistry.GetScriptDefinition(sName));
	if (pDefinition == NULL)
		return NULL;

	if (script_debug)
		Console.Dev << _T("ThreadStart(") << SingleQuoteStr(pDefinition->GetName()) << _T(", ") << uiTime << _T(")") << newl;

	CScriptThread *pNewThread(K2_NEW(global,    CScriptThread)(*pDefinition, uiTime));

	if (!pNewThread->Execute(uiTime))
	{
		m_lScripts.push_back(pNewThread);
		return pNewThread;
	}
	else
	{
		// Delete thread immediately if it finishes without blocking
		K2_DELETE(pNewThread);
		return NULL;
	}
}


/*====================
  CScriptDirectory::Frame
  ====================*/
void	CScriptDirectory::Frame()
{
	PROFILE("CScriptDirectory::Frame");

	list<CScriptThread *>::iterator it(m_lScripts.begin()), itEnd(m_lScripts.end());
	while (it != itEnd)
	{
		if ((*it)->Execute(Game.GetGameTime()))
		{
			if (script_debug)
				Console.Dev << _T("ThreadFinish(") << SingleQuoteStr((*it)->GetName()) << _T(", ") << Game.GetGameTime() << _T(")") << newl;

			K2_DELETE(*it);
			STL_ERASE(m_lScripts, it);
		}
		else
		{
			++it;
		}
	}
}
