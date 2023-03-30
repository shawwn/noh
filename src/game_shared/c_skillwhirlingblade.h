// (C)2006 S2 Games
// c_skillwhirlingblade.h
//
//=============================================================================
#ifndef __C_SKILLWHIRLINGBLADE_H__
#define __C_SKILLWHIRLINGBLADE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skillmelee.h"
//=============================================================================

//=============================================================================
// CSkillWhirlingBlade
//=============================================================================
class CSkillWhirlingBlade : public ISkillMelee
{
private:
    DECLARE_ENT_ALLOCATOR2(Skill, WhirlingBlade);

public:
    ~CSkillWhirlingBlade() {}
    CSkillWhirlingBlade() :
    ISkillMelee(GetEntityConfig()) {}
};
//=============================================================================

#endif //__C_SKILLWHIRLINGBLADE_H__
