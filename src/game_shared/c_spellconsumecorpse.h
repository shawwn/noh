// (C)2006 S2 Games
// c_spellconsumecorpse.h
//
//=============================================================================
#ifndef __C_SPELLCONSUMECORPSE_H__
#define __C_SPELLCONSUMECORPSE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSpellConsumeCorpse
//=============================================================================
class CSpellConsumeCorpse : public ISpellItem
{
private:
    START_ENTITY_CONFIG(ISpellItem)
        DECLARE_ENTITY_CVAR(float, HealthRestored)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(Spell, ConsumeCorpse);

    uint    m_uiStateSlot;

public:
    ~CSpellConsumeCorpse();
    CSpellConsumeCorpse();

    bool    ImpactEntity(uint uiTargetIndex, CGameEvent &evImpact, bool bCheckTarget = true);
};
//=============================================================================

#endif //__C_SPELLCONSUMECORPSE_H__
