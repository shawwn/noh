// (C)2007 S2 Games
// c_skillpolymorph.h
//
//=============================================================================
#ifndef __C_SKILLPOLYMORPH_H__
#define __C_SKILLPOLYMORPH_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skillselfbuff.h"
//=============================================================================

//=============================================================================
// CSkillPolymorph
//=============================================================================
class CSkillPolymorph : public ISkillSelfBuff
{
private:
	DECLARE_ITEM_ALLOCATOR(Skill, Polymorph)

public:
	~CSkillPolymorph()	{}
	CSkillPolymorph() :
	ISkillSelfBuff(GetItemConfig())
	{}
};
//=============================================================================

#endif //__C_SKILLPOLYMORPH_H__
