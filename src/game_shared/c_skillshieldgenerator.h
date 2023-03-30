// (C)2006 S2 Games
// c_skillshieldgenerator.h
//
//=============================================================================
#ifndef __C_SKILLSHIELDGENERATOR_H__
#define __C_SKILLSHIELDGENERATOR_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSkillShieldGenerator
//=============================================================================
class CSkillShieldGenerator : public ISpellItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Skill, ShieldGenerator);

public:
	~CSkillShieldGenerator() {}
	CSkillShieldGenerator() :
	ISpellItem(GetEntityConfig()) {}
};
//=============================================================================

#endif //__C_SKILLSHIELDGENERATOR_H__
