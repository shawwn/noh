// (C)2005 S2 Games
// c_function.h
//
//=============================================================================
#ifndef __C_FUNCTION_H__
#define __C_FUNCTION_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_consoleelement.h"

#include "c_profilemanager.h"
//=============================================================================

typedef tstring (*FunctionFn_t)(const tsvector &vArgList);

#define FUNCTION(name) \
tstring	fn##name##Fn(const tsvector &vArgList);\
CFunction fn##name(_T(#name), fn##name##Fn);\
tstring	fn##name##Fn(const tsvector &vArgList)

//=============================================================================
// CFunction
//=============================================================================
class K2_API CFunction : public CConsoleElement
{
private:
	FunctionFn_t	m_pfnFunction;

public:
	~CFunction();
	CFunction(const tstring &sName, FunctionFn_t pfnFunction);

	tstring Evaluate(const tsvector &vArgList)	{ return m_pfnFunction(vArgList); }

	tstring	operator()(const tsvector &vArgList);

	tstring	operator()(tstring s0 = _T(""), tstring s1 = _T(""), tstring s2 = _T(""), tstring s3 = _T(""),
					tstring s4 = _T(""), tstring s5 = _T(""), tstring s6 = _T(""), tstring s7 = _T(""),
					tstring s8 = _T(""), tstring s9 = _T(""));

	tstring GetString() const		{ return _T(""); }

	static bool		IsFunction(const tstring &sString);
	static bool		Parse(const tstring &sString, tstring &sName, tsvector &vArgList);
};
//=============================================================================
#endif // __C_FUNCTION_H__
