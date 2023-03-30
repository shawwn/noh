// (C)2006 S2 Games
// c_skilldisembark.h
//
//=============================================================================
#ifndef __C_SKILLDISEMBARK_H__
#define __C_SKILLDISEMBARK_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skillitem.h"
//=============================================================================

//=============================================================================
// CSkillDisembark
//=============================================================================
class CSkillDisembark : public ISkillItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Skill, Disembark);

public:
	~CSkillDisembark()	{}
	CSkillDisembark() :
	ISkillItem(GetEntityConfig())
	{}

	virtual void	Activate()				{ ActivatePrimary(GAME_BUTTON_STATUS_DOWN | GAME_BUTTON_STATUS_PRESSED); }
	virtual bool	ActivatePrimary(int iButtonStatus);

	TYPE_NAME("Vehicle abandonment")
};
//=============================================================================

#endif //__C_SKILLDISEMBARK_H__
