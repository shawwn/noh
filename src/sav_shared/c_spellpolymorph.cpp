// (C)2007 S2 Games
// c_spellpolymorph.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_spellpolymorph.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Spell, Polymorph)
//=============================================================================

/*====================
  CSpellPolymorph::CEntityConfig::CEntityConfig
  ====================*/
CSpellPolymorph::CEntityConfig::CEntityConfig(const tstring &sName) :
ISpellItem::CEntityConfig(sName),
INIT_ENTITY_CVAR(EndAnimName, _T("")),
INIT_ENTITY_CVAR(FinishTime, 0),
INIT_ENTITY_CVAR(ManaCostPerSecond, 0.0f),
INIT_ENTITY_CVAR(MaxTime, 0)
{
}


/*====================
  CSpellPolymorph::ActiveFrame
  ====================*/
void    CSpellPolymorph::ActiveFrame()
{
    if (Game.IsClient())
        return;

    if (!HasNetFlags(ITEM_NET_FLAG_ACTIVE))
        return;

    ICombatEntity *pOwner(GetOwnerEnt());
    if (pOwner == NULL)
        return;

    float fManaCost(GetManaCostPerSecond() * MsToSec(Game.GetFrameLength()));
    if (!pOwner->SpendMana(fManaCost))
    {
        Deactivate();
        return;
    }

    if (m_pEntityConfig->GetMaxTime() > 0 &&
        Game.GetGameTime() - m_uiActivationTime >= m_pEntityConfig->GetMaxTime())
    {
        Deactivate();
        return;
    }
}


/*====================
  CSpellPolymorph::Deactivate
  ====================*/
void    CSpellPolymorph::Deactivate()
{
    RemoveNetFlags(ITEM_NET_FLAG_ACTIVE);

    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return;

    pOwner->RemoveNetFlags(ENT_NET_FLAG_CHANNELING);

//  Game.SelectItem(pOwner->GetDefaultInventorySlot());
    if (m_iSelfStateSlot != -1)
    {
        pOwner->RemoveState(m_iSelfStateSlot);
        m_iSelfStateSlot = -1;
    }
    SetCooldownTimer(Game.GetGameTime(), GetCooldownTime());
    pOwner->SetAction(PLAYER_ACTION_SKILL, Game.GetGameTime() + GetFinishTime());
    if (GetEndAnimName().empty())
        pOwner->StartAnimation(_T("idle"), 1);
    else
        pOwner->StartAnimation(GetEndAnimName(), 1);
}


/*====================
  CSpellPolymorph::ImpactEntity
  ====================*/
bool    CSpellPolymorph::ImpactEntity(uint uiTargetIndex, CGameEvent &evImpact, bool bCheckTarget)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;

    IVisualEntity *pTarget(Game.GetVisualEntity(uiTargetIndex));
    if (pTarget == NULL)
        return false;
    if (bCheckTarget && !IsValidTarget(pTarget, true))
        return false;

    if (Game.IsServer())
        pOwner->SelectItem(m_ySlot);

    m_uiActivationTime = Game.GetGameTime();
    SetNetFlags(ITEM_NET_FLAG_ACTIVE);

    ushort unStateID(EntityRegistry.LookupID(m_pEntityConfig->GetTargetState()));
    uint uiLength(m_pEntityConfig->GetMaxTime() > 0 ? m_pEntityConfig->GetMaxTime() : INVALID_TIME);
    m_iSelfStateSlot = pOwner->ApplyState(unStateID, Game.GetGameTime(), uiLength);
    IEntityState *pState(pOwner->GetState(m_iSelfStateSlot));
    if (pState != NULL)
    {
        ushort unItemType(INVALID_ENT_TYPE);
        for (int i(0); i < MAX_INVENTORY; ++i)
        {
            IInventoryItem *pItem(pTarget->GetItem(i));
            if (pItem == NULL)
                continue;
            if (!pItem->IsMelee())
                continue;

            unItemType = pItem->GetType();
            break;
        }

        pState->SetModelHandle(pTarget->GetModelHandle());
        pState->SetDisguise(pTarget->GetTeam(), pTarget->IsPlayer() ? pTarget->GetAsPlayerEnt()->GetClientID() : -1, unItemType);
    }

    evImpact.SetSourcePosition(pTarget->GetPosition());
    evImpact.SetSourceAngles(pTarget->GetAngles());

    return true;
}
