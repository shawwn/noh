// (C)2005 S2 Games
// c_action.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_baseinput.h"
#include "c_actionregistry.h"
//=============================================================================


/*====================
  IBaseInput::IBaseInput
  ====================*/
IBaseInput::IBaseInput(const tstring &sName, EActionType eType, int iFlags) :
m_sName(sName),
m_iFlags(iFlags),
m_eType(eType)
{
	CActionRegistry::GetInstance()->Register(this);
}


/*====================
  IBaseInput::~IBaseInput
  ====================*/
IBaseInput::~IBaseInput()
{
	// If the registry is still valid, unregister the action
	// This is important for any actions declared in a client dll that
	// is being unloaded
	if (!CActionRegistry::IsReleased())
		CActionRegistry::GetInstance()->Unregister(this);
}
