// (C)2005 S2 Games
// c_cmd.h
//
//=============================================================================
#ifndef __C_CMD_H__
#define __C_CMD_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_consoleelement.h"

#include "c_profilemanager.h"
//=============================================================================

#ifdef K2_PROFILE // START K2_PROFILE
#define CMD(name) \
bool	_cmd##name##Fn(CConsoleElement *pElem, const tsvector &vArgList);\
bool	cmd##name##Fn(CConsoleElement *pElem, const tsvector &vArgList);\
CCmd cmd##name(_T(#name), _cmd##name##Fn);\
bool	_cmd##name##Fn(CConsoleElement *pElem, const tsvector &vArgList)\
{\
	PROFILE(#name);\
	return cmd##name##Fn(pElem, vArgList);\
}\
bool	cmd##name##Fn(CConsoleElement *pElem, const tsvector &vArgList)
#else	// NOT K2_PROFILE
#define CMD(name) \
bool	cmd##name##Fn(CConsoleElement *pElem, const tsvector &vArgList);\
CCmd cmd##name(_T(#name), cmd##name##Fn);\
bool	cmd##name##Fn(CConsoleElement *pElem, const tsvector &vArgList)
#endif  // END K2_PROFILE

#define CMD_EX(name, flags) \
bool	cmd##name##Fn(CConsoleElement *pElem, const tsvector &vArgList);\
CCmd cmd##name(_T(#name), cmd##name##Fn, flags);\
bool	cmd##name##Fn(CConsoleElement *pElem, const tsvector &vArgList)

#define EXTERN_CMD(name) extern CCmd cmd##name;

#define EMPTY_ARG_LIST	tsvector()

//=============================================================================
// CCmd
//=============================================================================
class K2_API CCmd : public CConsoleElement
{
private:

public:
	CCmd(const tstring &sName, ConsoleElementFn_t pfnCmd, int iFlags = 0);
	~CCmd();

	bool	operator()(const tsvector &vArgList);

	bool	operator()(const tstring &s0 = TSNULL, const tstring &s1 = TSNULL, const tstring &s2 = TSNULL, const tstring &s3 = TSNULL,
					const tstring &s4 = TSNULL, const tstring &s5 = TSNULL, const tstring &s6 = TSNULL, const tstring &s7 = TSNULL,
					const tstring &s8 = TSNULL, const tstring &s9 = TSNULL);

	tstring GetString() const		{ return TSNULL; }
};
//=============================================================================
#endif // __C_CMD_H__
