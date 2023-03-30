// (C)2006 S2 Games
// c_skillresurrect.h
//
//=============================================================================
#ifndef __C_SKILLRESURRECT_H__
#define __C_SKILLRESURRECT_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skillitem.h"
//=============================================================================

//=============================================================================
// CSkillResurrect
//=============================================================================
class CSkillResurrect : public ISkillItem
{
private:
	DECLARE_ITEM_ALLOCATOR(Skill, Resurrect)

public:
	~CSkillResurrect();
	CSkillResurrect();

	bool	ActivatePrimary(int iButtonStatus);
};
//=============================================================================

#endif //__C_SKILLRESURRECT_H__
