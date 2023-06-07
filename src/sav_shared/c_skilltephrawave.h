// (C)2006 S2 Games
// c_skilltephrawave.h
//
//=============================================================================
#ifndef __C_SKILLTEPHRAWAVE_H__
#define __C_SKILLTEPHRAWAVE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skillmelee.h"
//=============================================================================

//=============================================================================
// CSkillTephraWave
//=============================================================================
class CSkillTephraWave : public ISkillMelee
{
private:
    DECLARE_ENT_ALLOCATOR2(Skill, TephraWave);

public:
    ~CSkillTephraWave() {}
    CSkillTephraWave() :
    ISkillMelee(GetEntityConfig())
    {}

};
//=============================================================================

#endif //__C_SKILLTEPHRAWAVE_H__
