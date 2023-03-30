	// (C)2005 S2 Games
// c_cmd.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_cmdprecache.h"
#include "c_consoleregistry.h"
#include "stringutils.h"
//=============================================================================

/*====================
  CCmdPrecache::CCmdPrecache
  ====================*/
CCmdPrecache::CCmdPrecache(const tstring &sName, ConsoleElementFn_t pfnCmd) :
CConsoleElement(sName + _T("_Precache"), 0, ELEMENT_CMDPRECACHE, pfnCmd)
{
	assert(m_pfnCmd != NULL);

	// Check if the associated command has already been registered
	CConsoleElement *pCmd(ConsoleRegistry.GetElement(sName));

	ConsoleRegistry.Register(sName + _T("_Precache"), this);

	if (pCmd != NULL)
		pCmd->AddPrecacheCommand(pfnCmd);

	ConsoleRegistry.AddPrecacheCmd(this);
}


/*====================
  CCmdPrecache::~CCmdPrecache
  ====================*/
CCmdPrecache::~CCmdPrecache()
{
	ConsoleRegistry.Unregister(this);
	ConsoleRegistry.RemovePrecacheCmd(this);
}


/*====================
  CCmdPrecache::operator()

  Calls the associated function directly
  ====================*/
bool	CCmdPrecache::operator()(tstring s0, tstring s1, tstring s2, tstring s3, tstring s4,
						 tstring s5, tstring s6, tstring s7, tstring s8, tstring s9)
{
	tsvector	vArgList;
	if (!s0.empty()) { vArgList.push_back(s0);
	if (!s1.empty()) { vArgList.push_back(s1);
	if (!s2.empty()) { vArgList.push_back(s2);
	if (!s3.empty()) { vArgList.push_back(s3);
	if (!s4.empty()) { vArgList.push_back(s4);
	if (!s5.empty()) { vArgList.push_back(s5);
	if (!s6.empty()) { vArgList.push_back(s6);
	if (!s7.empty()) { vArgList.push_back(s7);
	if (!s8.empty()) { vArgList.push_back(s8);
	if (!s9.empty()) { vArgList.push_back(s9);
	}}}}}}}}}}

	return m_pfnCmd(this, vArgList);
}


/*====================
  CCmdPrecache::operator()

  Calls the associated function even more directly
  ====================*/
bool	CCmdPrecache::operator()(const tsvector &vArgList)
{
	return m_pfnCmd(this, vArgList);
}
