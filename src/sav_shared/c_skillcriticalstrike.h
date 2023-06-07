// (C)2006 S2 Games
// c_skillcriticalstrike.h
//
//=============================================================================
#ifndef __C_SKILLCRITICALSTRIKE_H__
#define __C_SKILLCRITICALSTRIKE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skillmelee.h"
//=============================================================================

//=============================================================================
// CSkillCriticalStrike
//=============================================================================
class CSkillCriticalStrike : public ISkillMelee
{
private:
    START_ENTITY_CONFIG(ISkillMelee)
    END_ENTITY_CONFIG

    DECLARE_ENT_ALLOCATOR2(Skill, CriticalStrike)

public:
    ~CSkillCriticalStrike() {}
    CSkillCriticalStrike() :
    ISkillMelee(GetEntityConfig())  {}
};
//=============================================================================

#endif //__C_SKILLCRITICALSTRIKE_H__
