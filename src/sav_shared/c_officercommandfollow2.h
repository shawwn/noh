// (C)2007 S2 Games
// c_officercommandfollow2.h
//
//=============================================================================
#ifndef __C_OFFICERCOMMANDFOLLOW2_H__
#define __C_OFFICERCOMMANDFOLLOW2_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// COfficerCommandFollow2
//=============================================================================
class COfficerCommandFollow2 : public ISpellItem
{
private:
    DECLARE_ENT_ALLOCATOR2(OfficerCommand, Follow2);

public:
    ~COfficerCommandFollow2()   {}
    COfficerCommandFollow2() : ISpellItem(GetEntityConfig())    {}

    virtual bool    ImpactEntity(uint uiTargetIndex, CGameEvent &evImpact, bool bCheckTarget = true);
};
//=============================================================================

#endif //__C_OFFICERCOMMANDFOLLOW_H__
