// (C)2007 S2 Games
// c_skillentangletrap.h
//
//=============================================================================
#ifndef __C_ENTANGLETRAP_H__
#define __C_ENTANGLETRAP_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skilldeploy.h"
//=============================================================================

//=============================================================================
// CSkillEntangleTrap
//=============================================================================
class CSkillEntangleTrap : public ISkillDeploy
{
private:
    DECLARE_ITEM_ALLOCATOR(Skill, EntangleTrap);

public:
    ~CSkillEntangleTrap()   {}
    CSkillEntangleTrap() :
    ISkillDeploy(GetItemConfig())
    {}
};
//=============================================================================

#endif //__C_ENTANGLETRAP_H__
