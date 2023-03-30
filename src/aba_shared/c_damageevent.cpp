// (C)2008 S2 Games
// c_damageevent.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_damageevent.h"
#include "i_unitentity.h"
//=============================================================================

/*====================
  CDamageEvent::ApplyDamage
  ====================*/
void    CDamageEvent::ApplyDamage()
{
    IUnitEntity *pAttacker(Game.GetUnitEntity(m_uiAttackerIndex));

    IUnitEntity *pTarget(Game.GetUnitEntity(m_uiTargetIndex));
    if (pTarget == NULL)
        return;

    if (pTarget->GetInvulnerable())
    {
        m_fAppliedDamage = 0.0f;
        return;
    }

    // Immunity
    if (Game.IsImmune(m_uiEffectType, pTarget->GetAdjustedImmunityType()))
    {
        m_fAppliedDamage = 0.0f;
        return;
    }

    // Deflection
    if (m_eSuperType == SUPERTYPE_ATTACK && m_fDeflection > 0.0f && m_fAttemptedDamage > 0.0f)
        Game.SendPopup(POPUP_DEFLECTION, pTarget, pTarget, ushort(MIN(m_fAttemptedDamage, m_fDeflection)));

    // Regular damage
    m_fAppliedDamage = MAX(0.0f, m_fAttemptedDamage - m_fDeflection) * pTarget->GetIncomingDamageMultiplier();

    // Combat type modifications
    if (pAttacker != NULL)
    {
        if (m_eSuperType == SUPERTYPE_ATTACK)
            m_fAppliedDamage *= Game.GetAttackMultiplier(pAttacker->GetCombatTypeIndex(), pTarget->GetCombatTypeIndex());
        else if (m_eSuperType == SUPERTYPE_SPELL)
            m_fAppliedDamage *= Game.GetSpellMultiplier(pAttacker->GetCombatTypeIndex(), pTarget->GetCombatTypeIndex());
        else 
            Console.Warn << _T("Damage event has an invalid super type!") << newl;
    }

    // Armor
    float fDamageAdjustment(1.0f);
    if (Game.GetIsArmorEffective(pTarget->GetArmorType(), m_uiEffectType))
        fDamageAdjustment *= (1.0f - Game.GetArmorDamageAdjustment(pTarget->GetArmorType(), pTarget->GetArmor()));
    if (Game.GetIsArmorEffective(pTarget->GetMagicArmorType(), m_uiEffectType))
        fDamageAdjustment *= (1.0f - Game.GetArmorDamageAdjustment(pTarget->GetMagicArmorType(), pTarget->GetMagicArmor()));
    
    m_fAppliedDamage *= fDamageAdjustment;
    
    pTarget->Damage(*this);
}
