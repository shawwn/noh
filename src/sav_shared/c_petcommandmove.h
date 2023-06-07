// (C)2007 S2 Games
// c_petcommandmove.h
//
//=============================================================================
#ifndef __C_PETCOMMANDMOVE_H__
#define __C_PETCOMMANDMOVE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CPetCommandMove
//=============================================================================
class CPetCommandMove : public ISpellItem
{
private:
    DECLARE_ENT_ALLOCATOR2(PetCommand, Move);

public:
    ~CPetCommandMove()  {}
    CPetCommandMove() : ISpellItem(GetEntityConfig())   {}

    virtual void            ImpactPosition(const CVec3f &v3target, CGameEvent &evImpact);
};
//=============================================================================

#endif //__C_PETCOMMANDMOVE_H__
