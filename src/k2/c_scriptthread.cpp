// (C)2005 S2 Games
// c_scriptthread.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "k2_api.h"

#include "c_scriptthread.h"
#include "c_script.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
//=============================================================================

/*====================
  CScriptThread::CScriptThread
  ====================*/
CScriptThread::CScriptThread() :
m_bFinished(false)
{
}


/*====================
  CScriptThread::~CScriptThread
  ====================*/
CScriptThread::~CScriptThread()
{
}


/*====================
  CScriptThread::Frame
  ====================*/
void	CScriptThread::Frame()
{
	while (!m_qActiveScripts.empty())
	{
		CScript *pScript = m_qActiveScripts.back();

		if (pScript->Execute())
		{
			m_qActiveScripts.pop_back();
			K2_DELETE(pScript);
		}
		else
		{
			break;
		}
	}

	if (m_qActiveScripts.empty())
		m_bFinished = true;
}


/*====================
  CScriptThread::ExecuteScript
  ====================*/
void	CScriptThread::ExecuteScript(const tstring &sData, bool bFile, tsmapts *mapParams)
{
	CScript	*pNewScript = K2_NEW(ctx_Script,  CScript)(mapParams);
	CScript	*pOldScript = !m_qActiveScripts.empty() ? m_qActiveScripts.back() : NULL;

	if (bFile)
		pNewScript->LoadFile(sData);
	else
		pNewScript->LoadScript(sData);

	if (!pNewScript->IsLoaded())
	{
		K2_DELETE(pNewScript);
		return;
	}

	m_qActiveScripts.push_back(pNewScript);

	if (pNewScript->Execute())
	{
		K2_DELETE(pNewScript);
		m_qActiveScripts.pop_back();
	}
	else if (pOldScript)
	{
		pOldScript->Sleep(0); // Pause the script that called this script
	}
}

/*====================
  CScriptThread::GotoScriptLabel
  ====================*/
void	CScriptThread::GotoScriptLabel(const tstring &sLabel)
{
	if (!m_qActiveScripts.empty())
		m_qActiveScripts.back()->Goto(sLabel);
}


/*====================
  CScriptThread::PauseScript
  ====================*/
void	CScriptThread::PauseScript(dword dwMilliseconds)
{
	if (!m_qActiveScripts.empty())
		m_qActiveScripts.back()->Sleep(dwMilliseconds);
}

