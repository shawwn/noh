// (C)2007 S2 Games
// c_skillimppoisoned.h
//
//=============================================================================
#ifndef __C_SKILLIMPPOISONED_H__
#define __C_SKILLIMPPOISONED_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skillitem.h"
//=============================================================================

//=============================================================================
// CSkillImpPoisoned
//=============================================================================
class CSkillImpPoisoned : public ISkillMelee
{
private:
	DECLARE_ENT_ALLOCATOR2(Skill, ImpPoisoned);

public:
	~CSkillImpPoisoned()	{}
	CSkillImpPoisoned() :
	ISkillMelee(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_SKILLIMPPOISONED_H__
