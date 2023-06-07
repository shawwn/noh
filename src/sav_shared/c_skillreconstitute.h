// (C)2006 S2 Games
// c_skillreconstitute.h
//
//=============================================================================
#ifndef __C_SKILLRECONSTITUTE_H__
#define __C_SKILLRECONSTITUTE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skillselfbuff.h"
//=============================================================================

//=============================================================================
// CSkillReconstitute
//=============================================================================
class CSkillReconstitute : public ISkillSelfBuff
{
private:
    DECLARE_ENT_ALLOCATOR2(Skill, Reconstitute);

public:
    ~CSkillReconstitute() {}
    CSkillReconstitute() :
    ISkillSelfBuff(GetEntityConfig()) {}
};
//=============================================================================

#endif //__C_SKILLRECONSTITUTE_H__
