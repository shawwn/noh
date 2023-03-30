// (C)2007 S2 Games
// c_skillhealingward.h
//
//=============================================================================
#ifndef __C_SKILLHEALINGWARD_H__
#define __C_SKILLHEALINGWARD_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skilltoggle.h"
//=============================================================================

//=============================================================================
// CSkillHealingWard
//=============================================================================
class CSkillHealingWard : public ISkillToggle
{
private:
	DECLARE_ENT_ALLOCATOR2(Skill, HealingWard)

public:
	~CSkillHealingWard()	{}
	CSkillHealingWard() :
	ISkillToggle(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_SKILLHEALINGWARD_H__
