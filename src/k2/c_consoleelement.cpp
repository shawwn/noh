// (C)2005 S2 Games
// c_consoleelement.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_consoleelement.h"
//=============================================================================

/*====================
  CConsoleElement::~ConsoleElement
  ====================*/
CConsoleElement::~CConsoleElement()
{
}


/*====================
  CConsoleElement::CConsoleElement
 ====================*/
CConsoleElement::CConsoleElement(const tstring &sName, int iFlags, EElementType eType, ConsoleElementFn_t pfnCmd) :
m_sName(sName),
m_iFlags(iFlags),
m_eType(eType),
m_pfnCmd(pfnCmd),
m_pfnPrecacheCmd(nullptr)
{
}
