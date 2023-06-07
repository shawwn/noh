// (C)2006 S2 Games
// c_spellimmolate.h
//
//=============================================================================
#ifndef __C_SPELLIMMOLATE_H__
#define __C_SPELLIMMOLATE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSpellImmolate
//=============================================================================
class CSpellImmolate : public ISpellItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Spell, Immolate);

    bool    m_bActive;
    IGameEntity *m_pGadgetEnt;

public:
    ~CSpellImmolate()   {}
    CSpellImmolate();

    bool    ActivatePrimary(int iButtonStatus);
    void    FinishedAction(int iAction);
};
//=============================================================================

#endif //__C_SPELLIMMOLATE_H__
