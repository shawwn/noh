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
    if (pTarget == nullptr)
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
    if (pAttacker != nullptr)
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
    {
        float fArmor(pTarget->GetArmor());

        if (fArmor > 0.0f)
            fArmor = MAX(fArmor - m_fArmorPierce, 0.0f);

        fDamageAdjustment *= (1.0f - Game.GetArmorDamageAdjustment(pTarget->GetArmorType(), fArmor));
    }
    if (Game.GetIsArmorEffective(pTarget->GetMagicArmorType(), m_uiEffectType))
    {
        float fMagicArmor(pTarget->GetMagicArmor());

        if (fMagicArmor > 0.0f)
            fMagicArmor = MAX(fMagicArmor - m_fMagicArmorPierce, 0.0f);

        fDamageAdjustment *= (1.0f - Game.GetArmorDamageAdjustment(pTarget->GetMagicArmorType(), fMagicArmor));
    }
    
    m_fAppliedDamage *= fDamageAdjustment;
    
    pTarget->Damage(*this);
}
