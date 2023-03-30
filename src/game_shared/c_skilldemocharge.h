// (C)2007 S2 Games
// c_skilldemocharge.h
//
//=============================================================================
#ifndef __C_SKILLDEMOCHARGE_H__
#define __C_SKILLDEMOCHARGE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSkillDemoCharge
//=============================================================================
class CSkillDemoCharge : public ISpellItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Skill, DemoCharge);

public:
    ~CSkillDemoCharge() {}
    CSkillDemoCharge() :
    ISpellItem(GetEntityConfig()) {}
};
//=============================================================================

#endif //__C_SKILLDEMOCHARGE_H__
