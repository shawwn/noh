// (C)2007 S2 Games
// c_skillsacrifice.h
//
//=============================================================================
#ifndef __C_SKILLSACRIFICE_H__
#define __C_SKILLSACRIFICE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skillselfbuff.h"
//=============================================================================

//=============================================================================
// CSkillSacrifice
//=============================================================================
class CSkillSacrifice : public ISkillSelfBuff
{
private:
	DECLARE_ENT_ALLOCATOR2(Skill, Sacrifice)

public:
	~CSkillSacrifice()	{}
	CSkillSacrifice() :
	ISkillSelfBuff(GetEntityConfig())
	{}

	virtual void	Impact();
};
//=============================================================================

#endif //__C_SKILLSACRIFICE_H__
