// (C)2007 S2 Games
// c_skillmanaward.h
//
//=============================================================================
#ifndef __C_SKILLMANAWARD_H__
#define __C_SKILLMANAWARD_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skilltoggle.h"
//=============================================================================

//=============================================================================
// CSkillManaWard
//=============================================================================
class CSkillManaWard : public ISkillToggle
{
private:
    DECLARE_ENT_ALLOCATOR2(Skill, ManaWard)

public:
    ~CSkillManaWard()   {}
    CSkillManaWard() :
    ISkillToggle(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_SKILLMANAWARD_H__
