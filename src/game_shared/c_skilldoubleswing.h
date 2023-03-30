// (C)2006 S2 Games
// c_skilldoubleswing.h
//
//=============================================================================
#ifndef __C_SKILLDOUBLESWING_H__
#define __C_SKILLDOUBLESWING_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skillmelee.h"
//=============================================================================

//=============================================================================
// CSkillDoubleSwing
//=============================================================================
class CSkillDoubleSwing : public ISkillMelee
{
private:
	DECLARE_ENT_ALLOCATOR2(Skill, DoubleSwing);

public:
	~CSkillDoubleSwing()	{}
	CSkillDoubleSwing() :
	ISkillMelee(GetEntityConfig())	{}
};
//=============================================================================

#endif //__C_SKILLDOUBLESWING_H__
