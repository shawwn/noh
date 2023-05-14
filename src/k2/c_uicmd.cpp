// (C)2005 S2 Games
// c_uicmd.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_uicmd.h"
#include "c_uicmdregistry.h"

#include "../k2/c_cmd.h"
//=============================================================================

/*====================
  CUICmd::CUICmd
  ====================*/
CUICmd::CUICmd(const tstring &sName, UICmdFn_t pfnUICmd, int iMinArgs, int iFlags) :
m_sName(sName),
m_pfnUICmd(pfnUICmd),
m_iMinArgs(iMinArgs),
m_iFlags(iFlags)
{
    if (m_pfnUICmd == nullptr)
        K2System.Error(_T("Tried to register an UICmd with a nullptr function."));

    CUICmdRegistry::GetInstance()->Register(this);
}


/*====================
  CUICmd::~CUICmd
  ====================*/
CUICmd::~CUICmd()
{
    // If the registry is still valid, unregister the uicmd
    // This is important for any actions declared in a client dll that
    // is being unloaded
    if (!CUICmdRegistry::IsReleased())
        CUICmdRegistry::GetInstance()->Unregister(m_sName);
}


/*====================
  CUICmd::Execute
  ====================*/
tstring CUICmd::Execute(IWidget *pCaller, const ScriptTokenVector &vArgList)
{
    if (int(vArgList.size()) < m_iMinArgs)
    {
        Console.Warn << _T("Insufficient arg count to command ") << m_sName << newl;
        return TSNULL;
    }

    return m_pfnUICmd(pCaller, vArgList);
}


/*--------------------
  UICmdList

  prints a list of all ui commands
  --------------------*/
CMD(UICmdList)
{
    int iNumFound(0);
    tstring sFind;

    if (!vArgList.empty())
        sFind = LowerString(vArgList[0]);

    const UICmdMap &lCmds = CUICmdRegistry::GetInstance()->GetUICmdMap();

    // Print uicmds
    for (UICmdMap::const_iterator it(lCmds.begin()); it != lCmds.end(); ++it)
    {
        if (sFind.empty() || LowerString(it->second->GetName()).find(sFind) != string::npos)
        {
            Console << it->second->GetName() << newl;
            ++iNumFound;
        }
    }

    Console << newl << iNumFound << _T(" matching ui commands found") << newl;

    return true;
}
