// (C)2006 S2 Games
// c_skillelectriceye.h
//
//=============================================================================
#ifndef __C_SKILLELECTRICEYE_H__
#define __C_SKILLELECTRICEYE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSkillElectricEye
//=============================================================================
class CSkillElectricEye : public ISpellItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Skill, ElectricEye)

public:
	~CSkillElectricEye() {}
	CSkillElectricEye() :
	ISpellItem(GetEntityConfig()) {}
};
//=============================================================================

#endif //__C_SKILLELECTRICEYE_H__
