// (C)2007 S2 Games
// c_skillbeastbuild.h
//
//=============================================================================
#ifndef __C_SKILLBEASTBUILD_H__
#define __C_SKILLBEASTBUILD_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skillitem.h"
//=============================================================================

//=============================================================================
// CSkillBeastBuild
//=============================================================================
class CSkillBeastBuild : public ISkillItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Skill, BeastBuild);

public:
    ~CSkillBeastBuild() {}
    CSkillBeastBuild() :
    ISkillItem(GetEntityConfig())
    {}

    void    Activate()  { ActivatePrimary(GAME_BUTTON_STATUS_DOWN | GAME_BUTTON_STATUS_PRESSED); }
    bool    ActivatePrimary(int iButtonStatus);

    TYPE_NAME("Building")
};
//=============================================================================

#endif //__C_SKILLBEASTBUILD_H__
