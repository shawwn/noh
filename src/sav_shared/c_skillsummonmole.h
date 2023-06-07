// (C)2007 S2 Games
// c_skillsummonmole.h
//
//=============================================================================
#ifndef __C_SKILLSUMMONMOLE_H__
#define __C_SKILLSUMMONMOLE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skilldeploy.h"
//=============================================================================

//=============================================================================
// CSkillSummonMole
//=============================================================================
class CSkillSummonMole : public ISkillDeploy
{
private:
    DECLARE_ENT_ALLOCATOR2(Skill, SummonMole);

public:
    ~CSkillSummonMole() {}
    CSkillSummonMole() :
    ISkillDeploy(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_SKILLSUMMONMOLE_H__
