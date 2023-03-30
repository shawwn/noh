// (C)2007 S2 Games
// c_officercommandmove2.h
//
//=============================================================================
#ifndef __C_OFFICERCOMMANDMOVE2_H__
#define __C_OFFICERCOMMANDMOVE2_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// COfficerCommandMove2
//=============================================================================
class COfficerCommandMove2 : public ISpellItem
{
private:
    DECLARE_ENT_ALLOCATOR2(OfficerCommand, Move2);

public:
    ~COfficerCommandMove2() {}
    COfficerCommandMove2() : ISpellItem(GetEntityConfig())  {}

    virtual bool            ImpactPosition(const CVec3f &v3target, CGameEvent &evImpact);
};
//=============================================================================

#endif //__C_OFFICERCOMMANDMOVE_H__
