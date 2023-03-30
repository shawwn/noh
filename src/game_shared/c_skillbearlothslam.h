// (C)2007 S2 Games
// c_skillBearlothSlam.h
//
//=============================================================================
#ifndef __C_SKILLBEARLOTHSLAM_H__
#define __C_SKILLBEARLOTHSLAM_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skillitem.h"
//=============================================================================

//=============================================================================
// CSkillBearlothSlam
//=============================================================================
class CSkillBearlothSlam : public ISkillMelee
{
private:
	DECLARE_ENT_ALLOCATOR2(Skill, BearlothSlam);

public:
	~CSkillBearlothSlam()	{}
	CSkillBearlothSlam() :
	ISkillMelee(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_SKILLBEARLOTHSLAM_H__
