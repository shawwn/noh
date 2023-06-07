// (C)2007 S2 Games
// c_skillcarnivorous.h
//
//=============================================================================
#ifndef __C_SKILLCARNIVOROUS_H__
#define __C_SKILLCARNIVOROUS_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skillselfbuff.h"
//=============================================================================

//=============================================================================
// CSkillCarnivorous
//=============================================================================
class CSkillCarnivorous : public ISkillSelfBuff
{
private:
    DECLARE_ENT_ALLOCATOR2(Skill, Carnivorous)

public:
    ~CSkillCarnivorous()    {}
    CSkillCarnivorous() :
    ISkillSelfBuff(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_SKILLCARNIVOROUS_H__
