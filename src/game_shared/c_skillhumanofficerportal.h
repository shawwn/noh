// (C)2007 S2 Games
// c_skillhumanofficerportal.h
//
//=============================================================================
#ifndef __C_SKILLHUMANOFFICERPORTAL_H__
#define __C_SKILLHUMANOFFICERPORTAL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSkillHumanOfficerPortal
//=============================================================================
class CSkillHumanOfficerPortal : public ISpellItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Skill, HumanOfficerPortal);

public:
	~CSkillHumanOfficerPortal() {}
	CSkillHumanOfficerPortal() : ISpellItem(GetEntityConfig()) {}
};
//=============================================================================

#endif //__C_SKILLOFFICERPORTAL_H__
