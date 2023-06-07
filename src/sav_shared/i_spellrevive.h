// (C)2007 S2 Games
// i_spellrevive.h
//
//=============================================================================
#ifndef __I_SPELLREVIVE_H__
#define __I_SPELLREVIVE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// ISpellRevive
//=============================================================================
class ISpellRevive : public ISpellItem
{
protected:
    START_ENTITY_CONFIG(ISpellItem)
        DECLARE_ENTITY_CVAR(uint, TargetFreezeTime)
        DECLARE_ENTITY_CVAR(float, TargetHealthPercent)
        DECLARE_ENTITY_CVAR(float, TargetStaminaPercent)
        DECLARE_ENTITY_CVAR(float, TargetManaPercent)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

public:
    virtual ~ISpellRevive() {}
    ISpellRevive(CEntityConfig *pConfig) :
    ISpellItem(pConfig),
    m_pEntityConfig(pConfig)
    {}

    bool    IsValidTarget(IGameEntity *pEntity, bool bImpact);
    bool    ImpactEntity(uint uiTargetIndex, CGameEvent &evImpact, bool bCheckTarget = true);

    TYPE_NAME("Reviving Spell")
};
//=============================================================================

#endif //__I_SPELLREVIVE_H__
