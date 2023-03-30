// (C)2006 S2 Games
// c_skillbackstab.h
//
//=============================================================================
#ifndef __C_SKILLEXORCISE_H__
#define __C_SKILLEXORCISE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skillitem.h"
//=============================================================================

//=============================================================================
// CSkillExorcise
//=============================================================================
class CSkillExorcise : public ISkillItem
{
private:
    DECLARE_ITEM_ALLOCATOR(Skill, Exorcise)

public:
    ~CSkillExorcise();
    CSkillExorcise();

    bool    ActivatePrimary(int iButtonStatus);
};
//=============================================================================

#endif //__C_SKILLEXORCISE_H__
