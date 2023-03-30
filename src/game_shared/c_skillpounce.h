// (C)2007 S2 Games
// c_skillpounce.h
//
//=============================================================================
#ifndef __CSKILLPOUNCE_H__
#define __CSKILLPOUNCE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skillmelee.h"
//=============================================================================

//=============================================================================
// CSkillPounce
//=============================================================================
class CSkillPounce : public ISkillMelee
{
private:
	START_ENTITY_CONFIG(ISkillMelee)
		DECLARE_ENTITY_CVAR(float, LeapRange)
		DECLARE_ENTITY_CVAR(uint, LeapTime)
		DECLARE_ENTITY_CVAR(tstring, ImpactAnim)
	END_ENTITY_CONFIG
	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(Skill, Pounce)

public:
	~CSkillPounce()	{}
	CSkillPounce();

	void	FinishedAction(int iAction);

	bool	ActivatePrimary(int iButtonStatus);

	ENTITY_CVAR_ACCESSOR(float, LeapRange, 0.0f)
	ENTITY_CVAR_ACCESSOR(uint, LeapTime, 0)
	ENTITY_CVAR_ACCESSOR(tstring, ImpactAnim, SNULL)
};
//=============================================================================

#endif //__CSKILLPOUNCE_H__
