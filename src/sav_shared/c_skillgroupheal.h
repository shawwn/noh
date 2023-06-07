// (C)2006 S2 Games
// c_skillgroupheal.h
//
//=============================================================================
#ifndef __C_SKILLGROUPHEAL_H__
#define __C_SKILLGROUPHEAL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skillitem.h"
//=============================================================================

//=============================================================================
// CSkillGroupHeal
//=============================================================================
class CSkillGroupHeal : public ISkillItem
{
private:
    DECLARE_ITEM_ALLOCATOR(Skill, GroupHeal)

public:
    ~CSkillGroupHeal();
    CSkillGroupHeal();

    bool    ActivatePrimary(int iButtonStatus);
};
//=============================================================================

#endif //__C_SKILLGROUPHEAL_H__
