// (C)2006 S2 Games
// c_skillbuild.h
//
//=============================================================================
#ifndef __C_SKILLBUILD_H__
#define __C_SKILLBUILD_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skillitem.h"
//=============================================================================

//=============================================================================
// CSkillBuild
//=============================================================================
class CSkillBuild : public ISkillItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Skill, Build);

public:
	~CSkillBuild() {}
	CSkillBuild() :
	ISkillItem(GetEntityConfig()) {}

	virtual void	Activate()	{ ActivatePrimary(GAME_BUTTON_STATUS_DOWN | GAME_BUTTON_STATUS_PRESSED); }
	virtual bool	ActivatePrimary(int iButtonStatus);
	virtual bool	Cancel(int iButtonStatus);

	TYPE_NAME("Building")
};
//=============================================================================

#endif //__C_SKILLBUILD_H__
