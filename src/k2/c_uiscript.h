// (C)2005 S2 Games
// c_uiscript.h
//
//=============================================================================
#ifndef __C_UISCRIPT_H__
#define __C_UISCRIPT_H__

//=============================================================================
// Declarations
//=============================================================================
class CInterface;
class IWidget;
class CUIScriptToken;

extern K2_API class CUIScript *g_pUIScript;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#ifdef K2_EXPORTS
#define UIScript	(*g_pUIScript)
#else
#define UIScript	(*CUIScript::GetInstance())
#endif
//=============================================================================

//=============================================================================
// CUIScript
//=============================================================================
class CUIScript
{
	SINGLETON_DEF(CUIScript)

private:
	IWidget*	m_pActiveWidget;

public:
	~CUIScript()	{}

	IWidget*		GetActiveWidget()	{ return m_pActiveWidget; }

	K2_API tstring		Evaluate(IWidget *pCaller, const tstring &sScript, const tsvector &vParams = VSNULL);
};
//=============================================================================
#endif //__C_UISCRIPT_H__
