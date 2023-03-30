// (C)2006 S2 Games
// c_eventscript.h
//
//=============================================================================
#ifndef __C_EVENTSCRIPT_H__
#define __C_EVENTSCRIPT_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CInterface;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================

//=============================================================================

//=============================================================================
// CEventScript
//=============================================================================
class CEventScript
{
private:
	static CEventScript	*s_pInstance;
	static bool			s_bReleased;

	CEventScript();
	CEventScript(CEventScript&);
	CEventScript operator=(CEventScript&);

public:
	~CEventScript();

	static CEventScript*	GetInstance();
	static void			Release();

	K2_API bool			DoCommand(const tsvector &vArgList, int iTimeNudge);
	K2_API bool			Execute(const tstring &sScript, int iTimeNudge);
};

extern K2_API CEventScript *pEventScript;
#define EventScript (*pEventScript)
#ifdef K2_EXPORTS
#define pEventScript CEventScript::GetInstance()
#endif //K2_EXPORTS
//=============================================================================
#endif //__C_EVENTSCRIPT_H__
