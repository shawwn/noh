// (C)2007 S2 Games
// c_spelldecay.h
//
//=============================================================================
#ifndef __C_SPELLDECAY_H__
#define __C_SPELLDECAY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spelltoggle.h"
//=============================================================================

//=============================================================================
// CSpellDecay
//=============================================================================
class CSpellDecay : public ISpellToggle
{
private:
    DECLARE_ENT_ALLOCATOR2(Spell, Decay)

public:
    ~CSpellDecay()  {}
    CSpellDecay() :
    ISpellToggle(GetEntityConfig())
    {}

    void    ActiveFrame();
    void    Activate()              { ActivatePrimary(GAME_BUTTON_STATUS_DOWN | GAME_BUTTON_STATUS_PRESSED); }
    bool    IsValidTarget(IGameEntity *pEntity, bool bImpact)   { return ((pEntity != NULL && pEntity == GetOwnerEnt()) || ISpellToggle::IsValidTarget(pEntity, bImpact)); }
};
//=============================================================================

#endif //__C_SPELLDECAY_H__
