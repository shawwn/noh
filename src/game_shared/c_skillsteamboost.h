// (C)2006 S2 Games
// c_skillsteamboost.h
//
//=============================================================================
#ifndef __C_SKILLSTEAMBOOST_H__
#define __C_SKILLSTEAMBOOST_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skillselfbuff.h"
//=============================================================================

//=============================================================================
// CSkillSteamBoost
//=============================================================================
class CSkillSteamBoost : public ISkillSelfBuff
{
private:
    DECLARE_ENT_ALLOCATOR2(Skill, SteamBoost);

public:
    ~CSkillSteamBoost() {}
    CSkillSteamBoost() :
    ISkillSelfBuff(GetEntityConfig())   {}
};
//=============================================================================

#endif //__C_SKILLSTEAMBOOST_H__
