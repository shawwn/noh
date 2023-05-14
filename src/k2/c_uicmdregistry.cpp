// (C)2005 S2 Games
// c_uicmdregistry.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_uicmdregistry.h"
#include "c_uicmd.h"

#include "../k2/stringutils.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CUICmdRegistry  *CUICmdRegistry::s_pInstance;
bool            CUICmdRegistry::s_bRequested;
bool            CUICmdRegistry::s_bReleased;

CUICmdRegistry  *pUICmdRegistry = CUICmdRegistry::GetInstance();
//=============================================================================


/*====================
  CUICmdRegistry::GetInstance
  ====================*/
CUICmdRegistry* CUICmdRegistry::GetInstance()
{
    assert(!s_bReleased);

    if (s_pInstance == nullptr)
    {
        assert(!s_bRequested);
        s_bRequested = true;
        s_pInstance = K2_NEW(ctx_Console,  CUICmdRegistry);
    }

    return s_pInstance;
}


/*====================
  CUICmdRegistry::Release
  ====================*/
void    CUICmdRegistry::Release()
{
    assert(!s_bReleased);

    if (s_pInstance != nullptr)
        K2_DELETE(s_pInstance);

    s_bReleased = true;
}


/*====================
  CUICmdRegistry::Register
  ====================*/
void    CUICmdRegistry::Register(CUICmd *pUICmd)
{
    // Make sure there is no name collision
    UICmdMap::iterator findit = m_mapUICmds.find(LowerString(pUICmd->GetName()));
    if (findit != m_mapUICmds.end())
    {
        Console.Err << _T("A function named ") << QuoteStr(pUICmd->GetName())
                    << _T(" already exists.") << newl;
        return;
    }

    m_mapUICmds[LowerString(pUICmd->GetName())] = pUICmd;
}


/*====================
  CUICmdRegistry::Unregister
  ====================*/
void    CUICmdRegistry::Unregister(const tstring &sName)
{
    UICmdMap::iterator findit = m_mapUICmds.find(LowerString(sName));
    if(findit != m_mapUICmds.end())
    {
        //Console.Dev << _T("Function ") << sName << _T(" has been unregistered.") << newl;
        m_mapUICmds.erase(findit);
    }
}
