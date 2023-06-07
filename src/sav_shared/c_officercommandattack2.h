// (C)2007 S2 Games
// c_officercommandattack2.h
//
//=============================================================================
#ifndef __C_OFFICERCOMMANDATTACK2_H__
#define __C_OFFICERCOMMANDATTACK2_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// COfficerCommandAttack2
//=============================================================================
class COfficerCommandAttack2 : public ISpellItem
{
private:
    DECLARE_ENT_ALLOCATOR2(OfficerCommand, Attack2);

public:
    ~COfficerCommandAttack2()   {}
    COfficerCommandAttack2() : ISpellItem(GetEntityConfig())    {}

    virtual bool    ImpactEntity(uint uiTargetIndex, CGameEvent &evImpact, bool bCheckTarget = true);
};
//=============================================================================

#endif //__C_OFFICERCOMMANDATTACK_H__
