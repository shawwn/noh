// (C)2007 S2 Games
// c_skillbehemothtoughskin.h
//
//=============================================================================
#ifndef __C_SKILLBEHEMOTHTOUGHSKIN_H__
#define __C_SKILLBEHEMOTHTOUGHSKIN_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skilltoggle.h"
//=============================================================================

//=============================================================================
// CSkillBehemothToughSkin
//=============================================================================
class CSkillBehemothToughSkin : public ISkillToggle
{
private:
	DECLARE_ENT_ALLOCATOR2(Skill, BehemothToughSkin)

public:
	~CSkillBehemothToughSkin()	{}
	CSkillBehemothToughSkin() :
	ISkillToggle(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_SKILLBEHEMOTHTOUGHSKIN_H__
