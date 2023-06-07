// (C)2006 S2 Games
// c_skillattrition.h
//
//=============================================================================
#ifndef __C_SKILLATTRITION_H__
#define __C_SKILLATTRITION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skillitem.h"
//=============================================================================

//=============================================================================
// CSkillAttrition
//=============================================================================
class CSkillAttrition : public ISkillItem
{
private:
    DECLARE_ITEM_ALLOCATOR(Skill, Attrition)

public:
    ~CSkillAttrition();
    CSkillAttrition();

    bool    ActivatePrimary(int iButtonStatus);
};
//=============================================================================

#endif //__C_SKILLATTRITION_H__
