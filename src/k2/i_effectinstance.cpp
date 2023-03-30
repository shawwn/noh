// (C)2007 S2 Games
// i_effectinstance.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_effectinstance.h"
#include "i_emitter.h"
#include "c_effectthread.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
//=============================================================================


/*====================
  IEffectInstance::~IEffectInstance
  ====================*/
IEffectInstance::~IEffectInstance()
{
}


/*====================
  IEffectInstance::IEffectInstance
  ====================*/
IEffectInstance::IEffectInstance() :
m_pEffectThread(NULL)
{
}


/*====================
  IEffectInstance::IEffectInstance
  ====================*/
IEffectInstance::IEffectInstance(CEffectThread *pEffectThread) :
m_pEffectThread(pEffectThread)
{
}


/*====================
  IEffectInstance::GetActive
  ====================*/
bool	IEffectInstance::GetActive() const
{
	if (m_pEffectThread != NULL)
		return m_pEffectThread->GetActive();
	else
		return false;
}


/*====================
  IEffectInstance::GetExpire
  ====================*/
bool	IEffectInstance::GetExpire() const
{
	if (m_pEffectThread != NULL)
		return m_pEffectThread->GetExpire();
	else
		return false;
}


/*====================
  IEffectInstance::GetEffect
  ====================*/
CEffect*	IEffectInstance::GetEffect()
{
	if (m_pEffectThread != NULL)
		return m_pEffectThread->GetEffect();
	else
		return NULL;
}


