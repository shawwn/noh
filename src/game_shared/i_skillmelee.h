// (C)2006 S2 Games
// i_skillmelee.h
//
//=============================================================================
#ifndef __I_SKILLMELEE_H__
#define __I_SKILLMELEE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skillitem.h"
//=============================================================================

//=============================================================================
// ISkillMelee
//=============================================================================
class ISkillMelee : public ISkillItem
{
protected:
	START_ENTITY_CONFIG(ISkillItem)
		DECLARE_ENTITY_CVAR(float, MinDamage)
		DECLARE_ENTITY_CVAR(float, MaxDamage)
		DECLARE_ENTITY_CVAR(uint, ImpactTime)
		DECLARE_ENTITY_CVAR(float, MinAngle)
		DECLARE_ENTITY_CVAR(float, MaxAngle)
		DECLARE_ENTITY_CVAR(float, AngleStep)
		DECLARE_ENTITY_CVAR(float, MinRange)
		DECLARE_ENTITY_CVAR(float, MaxRange)
		DECLARE_ENTITY_CVAR(float, RangeStep)
		DECLARE_ENTITY_CVAR(float, MinHeight)
		DECLARE_ENTITY_CVAR(float, MaxHeight)
		DECLARE_ENTITY_CVAR(float, HeightStep)
		DECLARE_ENTITY_CVAR(float, RearMultiplier)
		DECLARE_ENTITY_CVAR(tstring, TargetState)
		DECLARE_ENTITY_CVAR(uint, TargetStateDuration)
		DECLARE_ENTITY_CVAR(CVec3f, Push)
		DECLARE_ENTITY_CVAR(CVec3f, Lunge)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

public:
	virtual ~ISkillMelee()	{}
	ISkillMelee(CEntityConfig *pConfig) :
	ISkillItem(pConfig),
	m_pEntityConfig(pConfig)
	{}

	bool			IsMeleeSkill() const	{ return true; }

	virtual void	Activate()				{ ActivatePrimary(GAME_BUTTON_STATUS_DOWN | GAME_BUTTON_STATUS_PRESSED); }
	virtual bool	ActivatePrimary(int iButtonStatus);

	static void		ClientPrecache(CEntityConfig *pConfig);
	static void		ServerPrecache(CEntityConfig *pConfig);

	// Settings
	ENTITY_CVAR_ACCESSOR(float, MinDamage, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, MaxDamage, 0.0f)
	ENTITY_CVAR_ACCESSOR(uint, ImpactTime, 0)
	ENTITY_CVAR_ACCESSOR(float, MinAngle, -30.0f)
	ENTITY_CVAR_ACCESSOR(float, MaxAngle, 30.0f)
	ENTITY_CVAR_ACCESSOR(float, AngleStep, 15.0f)
	ENTITY_CVAR_ACCESSOR(float, MinRange, 10.0f)
	ENTITY_CVAR_ACCESSOR(float, MaxRange, 60.0f)
	ENTITY_CVAR_ACCESSOR(float, RangeStep, 20.0f)
	ENTITY_CVAR_ACCESSOR(float, MinHeight, 20.0f)
	ENTITY_CVAR_ACCESSOR(float, MaxHeight, 80.0f)
	ENTITY_CVAR_ACCESSOR(float, HeightStep, 20.0f)
	ENTITY_CVAR_ACCESSOR(float, RearMultiplier, 1.0f)
	ENTITY_CVAR_ACCESSOR(tstring, TargetState, _T(""))
	ENTITY_CVAR_ACCESSOR(uint, TargetStateDuration, 0)
	ENTITY_CVAR_ACCESSOR(CVec3f, Push, V_ZERO)
	ENTITY_CVAR_ACCESSOR(CVec3f, Lunge, V_ZERO)

	TYPE_NAME("Melee Attack")
};
//=============================================================================

#endif //__I_SKILLMELEE_H__
