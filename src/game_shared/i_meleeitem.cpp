// (C)2006 S2 Games
// i_meleeitem.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_meleeitem.h"
#include "c_meleeattackevent.h"
#include "c_teaminfo.h"

#include "../k2/s_traceinfo.h"
#include "../k2/c_world.h"
#include "../k2/c_vid.h"
#include "../k2/c_clientsnapshot.h"
#include "../k2/c_effect.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_FLOATF(    g_meleeStaminaRecovery,     1.0f,       CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_BOOLF(     g_meleeNewBlock,            false,      CVAR_GAMECONFIG | CVAR_TRANSMIT);
//=============================================================================

/*====================
  IMeleeItem::CEntityConfig::CEntityConfig
  ====================*/
IMeleeItem::CEntityConfig::CEntityConfig(const tstring &sName) :
IInventoryItem::CEntityConfig(sName),
INIT_ENTITY_CVAR(ApplyAttributes, true),
INIT_ENTITY_CVAR(QuickAttackAnimName, _T("quick_attack")),
INIT_ENTITY_CVAR(QuickAttackAnimNameRunForward, _T("quick_attack")),
INIT_ENTITY_CVAR(QuickAttackAnimNameRunBack, _T("quick_attack")),
INIT_ENTITY_CVAR(QuickAttackAnimNameSprint, _T("quick_attack")),
INIT_ENTITY_CVAR(QuickAttackNumAnims, 1),
INIT_ENTITY_CVAR(QuickAttackTime, _T("400")),
INIT_ENTITY_CVAR(QuickAttackImpactTime, _T("100")),
INIT_ENTITY_CVAR(QuickAttackChainTime, _T("300")),
INIT_ENTITY_CVAR(QuickAttackImmobileTime, _T("0")),
INIT_ENTITY_CVAR(QuickAttackMinDamage, "20.0f"),
INIT_ENTITY_CVAR(QuickAttackMaxDamage, "30.0f"),
INIT_ENTITY_CVAR(QuickAttackHeightMin, 40.0f),
INIT_ENTITY_CVAR(QuickAttackHeightMax, 75.0f),
INIT_ENTITY_CVAR(QuickAttackHeightStep, 15.0f),
INIT_ENTITY_CVAR(QuickAttackAngleMin, -60.0f),
INIT_ENTITY_CVAR(QuickAttackAngleMax, 70.0f),
INIT_ENTITY_CVAR(QuickAttackAngleStep, 20.0f),
INIT_ENTITY_CVAR(QuickAttackRangeMin, 20.0f),
INIT_ENTITY_CVAR(QuickAttackRangeMax, 80.0f),
INIT_ENTITY_CVAR(QuickAttackRangeStep, 20.0f),
INIT_ENTITY_CVAR(QuickAttackPivotHeight, 35.0f),
INIT_ENTITY_CVAR(QuickAttackPivotFactor, 0.4f),
INIT_ENTITY_CVAR(QuickAttackStaminaCost, "12.0f"),
INIT_ENTITY_CVAR(QuickAttackPush, V_ZERO),
INIT_ENTITY_CVAR(QuickAttackLunge, V_ZERO),
INIT_ENTITY_CVAR(QuickAttackBlockable, true),
INIT_ENTITY_CVAR(QuickAttackStaminaRequired, false),

INIT_ENTITY_CVAR(JumpAttackAnimName, _T("jump_attack")),
INIT_ENTITY_CVAR(JumpAttackNumAnims, 1),
INIT_ENTITY_CVAR(JumpAttackTime, _T("400")),
INIT_ENTITY_CVAR(JumpAttackImpactTime, _T("100")),
INIT_ENTITY_CVAR(JumpAttackChainTime, _T("300")),
INIT_ENTITY_CVAR(JumpAttackImmobileTime, 0),
INIT_ENTITY_CVAR(JumpAttackMinDamage, 20.0f),
INIT_ENTITY_CVAR(JumpAttackMaxDamage, 30.0f),
INIT_ENTITY_CVAR(JumpAttackHeightMin, 40.0f),
INIT_ENTITY_CVAR(JumpAttackHeightMax, 75.0f),
INIT_ENTITY_CVAR(JumpAttackHeightStep, 15.0f),
INIT_ENTITY_CVAR(JumpAttackAngleMin, -60.0f),
INIT_ENTITY_CVAR(JumpAttackAngleMax, 70.0f),
INIT_ENTITY_CVAR(JumpAttackAngleStep, 20.0f),
INIT_ENTITY_CVAR(JumpAttackRangeMin, 20.0f),
INIT_ENTITY_CVAR(JumpAttackRangeMax, 80.0f),
INIT_ENTITY_CVAR(JumpAttackRangeStep, 20.0f),
INIT_ENTITY_CVAR(JumpAttackPivotHeight, 35.0f),
INIT_ENTITY_CVAR(JumpAttackPivotFactor, 0.4f),
INIT_ENTITY_CVAR(JumpAttackStaminaCost, 12.0f),
INIT_ENTITY_CVAR(JumpAttackPush, V_ZERO),
INIT_ENTITY_CVAR(JumpAttackLunge, V_ZERO),
INIT_ENTITY_CVAR(JumpAttackBlockable, true),
INIT_ENTITY_CVAR(JumpAttackEnabled, false),
INIT_ENTITY_CVAR(JumpAttackStaminaRequired, false),

INIT_ENTITY_CVAR(StrongAttackAnimName, _T("strong_attack")),
INIT_ENTITY_CVAR(StrongAttackNumAnims, 1),
INIT_ENTITY_CVAR(StrongAttackTime, _T("1200")),
INIT_ENTITY_CVAR(StrongAttackImpactTime, _T("650")),
INIT_ENTITY_CVAR(StrongAttackChainTime, _T("0")),
INIT_ENTITY_CVAR(StrongAttackImmobileTime, 0),
INIT_ENTITY_CVAR(StrongAttackMinDamage, 90.0f),
INIT_ENTITY_CVAR(StrongAttackMaxDamage, 110.0f),
INIT_ENTITY_CVAR(StrongAttackHeightMin, 40.0f),
INIT_ENTITY_CVAR(StrongAttackHeightMax, 75.0f),
INIT_ENTITY_CVAR(StrongAttackHeightStep, 15.0f),
INIT_ENTITY_CVAR(StrongAttackAngleMin, -60.0f),
INIT_ENTITY_CVAR(StrongAttackAngleMax, 70.0f),
INIT_ENTITY_CVAR(StrongAttackAngleStep, 20.0f),
INIT_ENTITY_CVAR(StrongAttackRangeMin, 80.0f),
INIT_ENTITY_CVAR(StrongAttackRangeMax, 20.0f),
INIT_ENTITY_CVAR(StrongAttackRangeStep, 35.0f),
INIT_ENTITY_CVAR(StrongAttackPivotHeight, 0.4f),
INIT_ENTITY_CVAR(StrongAttackPivotFactor, 12.0f),
INIT_ENTITY_CVAR(StrongAttackStaminaCost, 18.0f),
INIT_ENTITY_CVAR(StrongAttackPush, V_ZERO),
INIT_ENTITY_CVAR(StrongAttackLunge, V_ZERO),
INIT_ENTITY_CVAR(StrongAttackStaminaRequired, false),

INIT_ENTITY_CVAR(BlockAnimName, _T("block")),
INIT_ENTITY_CVAR(BlockTime, 500),
INIT_ENTITY_CVAR(BlockImmobileTime, 0),
INIT_ENTITY_CVAR(BlockStaminaCost, 0.0f),
INIT_ENTITY_CVAR(BlockPush, V_ZERO),
INIT_ENTITY_CVAR(BlockLunge, V_ZERO),
INIT_ENTITY_CVAR(BlockImpactTime, 0),
INIT_ENTITY_CVAR(BlockDuration, 0),
INIT_ENTITY_CVAR(BlockStaminaRequired, false),
INIT_ENTITY_CVAR(BlockMaxTime, 0),
INIT_ENTITY_CVAR(BlockRecoverTime, 0),

INIT_ENTITY_CVAR(PierceUnit, 1.0f),
INIT_ENTITY_CVAR(PierceHellbourne, 1.0f),
INIT_ENTITY_CVAR(PierceSiege, 1.0f),
INIT_ENTITY_CVAR(PierceBuilding, 1.0f),
INIT_ENTITY_CVAR(RearMultiplier, 1.0f),

INIT_ENTITY_CVAR(ImpactEffectPath, _T(""))
{
}


/*====================
  IMeleeItem::TraceArc
  ====================*/
bool    IMeleeItem::TraceArc(const CMeleeAttackEvent &attack, uiset &setIndices, vector<CVec3f> &vImpacts)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return true;

    setIndices.clear();
    CAxis axis(pOwner->GetAngles());

    // Step through each layer of the attack
    for (float fHeight(attack.GetMin(MELEE_HEIGHT)); fHeight < attack.GetMax(MELEE_HEIGHT); fHeight += attack.GetStep(MELEE_HEIGHT))
    {
        // Find the center point along the "up" axis
        CVec3f v3Center(attack.GetCenter(pOwner->GetPosition(), axis, fHeight));

        // Step through each slice of the arc
        for (float fAngle(attack.GetMin(MELEE_ANGLE)); fAngle < attack.GetMax(MELEE_ANGLE); fAngle += attack.GetStep(MELEE_ANGLE))
        {
            // Determine next point in the arc
            float fNextAngle(MIN(fAngle + attack.GetStep(MELEE_ANGLE), attack.GetMax(MELEE_ANGLE)));
            
            // Determine the direction of each angle
            CVec3f v3DirA(attack.GetDir(axis, fAngle));
            CVec3f v3DirB(attack.GetDir(axis, fNextAngle));

            // Step to the maximum range of the attack
            for (float fDist(attack.GetMin(MELEE_RANGE)); fDist < attack.GetMax(MELEE_RANGE); fDist += attack.GetStep(MELEE_RANGE))
            {
                if (fDist == 0.0f)
                    continue;

                CVec3f v3Start(v3Center + v3DirA * fDist);
                CVec3f v3End(v3Center + v3DirB * fDist);
                
                STraceInfo result;
                Game.TraceLine(result, v3Start, v3End, TRACE_MELEE, pOwner->GetWorldIndex());
                if (result.uiEntityIndex != INVALID_INDEX)
                {
                    CWorldEntity *pWorldEnt(Game.GetWorldEntity(result.uiEntityIndex));
                    if (!pWorldEnt)
                        continue;

                    IVisualEntity *pEntity(Game.GetVisualEntity(pWorldEnt->GetGameIndex()));
                    if (!pEntity)
                        continue;

                    setIndices.insert(pEntity->GetIndex());
                    vImpacts.push_back(result.v3EndPos);

                    pWorldEnt->SetSurfFlags(pWorldEnt->GetSurfFlags() | SURF_IGNORE);
                }
            }
        }
    }

    // Reset SURF_IGNORE on all of the entities that were just hit
    for (uiset_it itEntityHit(setIndices.begin()); itEntityHit != setIndices.end(); ++itEntityHit)
    {
        IVisualEntity *pEntity(Game.GetVisualEntity(*itEntityHit));
        if (!pEntity)
            continue;

        CWorldEntity *pWorldEnt(Game.GetWorldEntity(pEntity->GetWorldIndex()));
        if (!pWorldEnt)
            continue;

        pWorldEnt->SetSurfFlags(pWorldEnt->GetSurfFlags() & ~SURF_IGNORE);
    }

    return true;
}


/*====================
  IMeleeItem::StartAttack
  ====================*/
bool    IMeleeItem::StartAttack(bool bJump)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return true;

    CMeleeAttackEvent &attack(pOwner->GetAttackEvent());

    // Check to see if the attack is ready yet
    //if (!IsReady())
    //  return false;

    // Check mana
    if (GetManaCost() > 0.0f &&
        !pOwner->SpendMana(GetManaCost()))
        return false;

    // Check stamina
    if (attack.GetStaminaRequired() &&
        pOwner->GetStamina() < attack.GetStaminaCost())
        return false;
    
    // Notify all states that an attack is occuring
    pOwner->DoAttack(attack);

    // Play the animation
    if (bJump)
    {
        pOwner->StartAnimation(attack.GetAnimName(), -1, pOwner->GetAttackSpeed(GetApplyAttributes()));
        pOwner->LockAnimation(-1, Game.GetGameTime() + attack.GetLength());
    }
    else
    {
        pOwner->StartAnimation(attack.GetAnimName(), 1, pOwner->GetAttackSpeed(GetApplyAttributes()));
    }

    // Lunge
    if (pOwner->IsOnGround() || bJump)
    {
        CAxis axis(pOwner->GetAngles());
        CVec3f v3Lunge(axis.Forward2d());
        v3Lunge *= attack.GetLunge().x;
        v3Lunge += axis.Right() * attack.GetLunge().y;
        v3Lunge.z = attack.GetLunge().z;
        pOwner->ApplyVelocity(v3Lunge);
    }

    pOwner->DrainStamina(attack.GetStaminaCost());
    pOwner->SetLastMeleeAttackTime(Game.GetGameTime());
    pOwner->SetLastMeleeAttackLength(attack.GetLength());
    return true;
}


/*====================
  IMeleeItem::Impact
  ====================*/
void    IMeleeItem::Impact()
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return;

    // Don't predict hits
    if (Game.IsClient())
        return;

    CMeleeAttackEvent& attack(pOwner->GetAttackEvent());
    attack.SetActive();

    // Trace for hits
    uiset setIndices;
    vector<CVec3f> vImpacts;
    TraceArc(attack, setIndices, vImpacts);
    
    // Process each entity that this attack hit
    vector<CVec3f>::iterator itImpacts(vImpacts.begin());
    for (uiset_it itEntIndex(setIndices.begin()); itEntIndex != setIndices.end(); ++itEntIndex, ++itImpacts)
    {
        IVisualEntity *pEntity(Game.GetVisualEntity(*itEntIndex));

        if (pEntity == NULL)
            continue;

        // Melee hits never affect allies
        if (!pOwner->IsEnemy(pEntity))
            continue;

        float fMult(1.0f);

        if (attack.GetDamageFlags() & DAMAGE_FLAG_SKILL && pEntity->IsCombat())
            fMult = pEntity->GetAsCombatEnt()->GetSkillResistance();

        // Apply any states carried by this attack
        const svector &vStates(attack.GetStateVector());
        const uivector &vStateDurations(attack.GetStateDurationVector());
        for (size_t z(0); z < vStates.size(); ++z)
        {
            int iSlot(pEntity->ApplyState(EntityRegistry.LookupID(vStates[z]), Game.GetGameTime(), vStateDurations[z], pOwner->GetIndex()));

            if (iSlot != -1 && pEntity->GetState(iSlot) != NULL && pEntity->GetState(iSlot)->IsDebuff())
                pEntity->GetState(iSlot)->SetDuration(vStateDurations[z] * fMult);
        }

        // Melee push
        attack.Push(pOwner->GetAngles(), pEntity->GetAsCombatEnt());

        // Do the damage
        float fDamage(attack.GetDamage() * fMult);
        IPlayerEntity *pPlayer = pEntity->GetAsPlayerEnt();

        if (pEntity->IsBuilding())
            fDamage *= m_pEntityConfig->GetPierceBuilding();
        else if (pPlayer != NULL && pPlayer->GetIsSiege())
            fDamage *= m_pEntityConfig->GetPierceSiege();
        else if (pPlayer != NULL && pPlayer->GetIsHellbourne())
            fDamage *= m_pEntityConfig->GetPierceHellbourne();
        else if (pEntity->IsPlayer() || pEntity->IsNpc() || pEntity->IsPet())
            fDamage *= m_pEntityConfig->GetPierceUnit();

        fDamage = pEntity->Damage(fDamage, attack.GetDamageFlags(), pOwner, GetType());
        STraceInfo result;
        Game.TraceLine(result, pOwner->GetPosition() + pOwner->GetBounds().GetMid(), pEntity->GetPosition() + pEntity->GetBounds().GetMid(), TRACE_MELEE, pOwner->GetWorldIndex());             
        if (result.uiEntityIndex != INVALID_INDEX)
        {
            pEntity->Hit(result.v3EndPos, V3_ZERO, ENTITY_HIT_BY_MELEE); // TODO: Impact angles
            if (pOwner->IsPlayer() && (pEntity->IsPlayer() || pEntity->IsPet() || pEntity->IsNpc()))
            {
                CBufferFixed<15> buffer;
                buffer << GAME_CMD_HITFEEDBACK << byte(HIT_MELEE_IMPACT) << result.v3EndPos;
                Game.SendGameData(pOwner->GetAsPlayerEnt()->GetClientID(), buffer, false);
            }
            if (pEntity->IsPlayer() && (pOwner->IsPlayer() || pOwner->IsPet() || pOwner->IsNpc()))
            {
                CBufferFixed<15> buffer;
                buffer << GAME_CMD_HITFEEDBACK << byte(HIT_MELEE_IMPACT) << result.v3EndPos;
                Game.SendGameData(pEntity->GetAsPlayerEnt()->GetClientID(), buffer, false);
            }
        }

        if (!m_pEntityConfig->GetImpactEffectPath().empty())
        {
            CGameEvent evImpact;
            evImpact.SetSourcePosition(result.v3EndPos);
            evImpact.SetEffect(Game.RegisterEffect(m_pEntityConfig->GetImpactEffectPath()));
            Game.AddEvent(evImpact);
        }

        if (fDamage > 0.0f && !pEntity->IsBuilding())
        {
            // Stamina recovery
            pOwner->GiveStamina(g_meleeStaminaRecovery * attack.GetStaminaCost());

            // Health recovery
            pOwner->Heal(fDamage * attack.GetHealthLeach(), pOwner);

            // Start blood effect on the weapon if not vehicle or hellbourne
            if (!(pEntity->IsPlayer() && (pEntity->GetAsPlayerEnt()->GetIsVehicle() || pEntity->GetAsPlayerEnt()->GetIsHellbourne())))
            {
                pOwner->SetEffect(EFFECT_CHANNEL_BLOOD_SPRAY, Game.RegisterEffect(_T("/shared/effects/bloodspray_small.effect")));
                pOwner->IncEffectSequence(EFFECT_CHANNEL_BLOOD_SPRAY);
            }
        }
    }

    // Stop attack if it has exceeded its duration
    if (attack.GetImpactEndTime() > 0 && Game.GetGameTime() >= attack.GetImpactEndTime())
        attack.SetInactive();
}


/*====================
  IMeleeItem::ActivatePrimary
  ====================*/
bool    IMeleeItem::ActivatePrimary(int iButtonStatus)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return true;

//  if (!m_bPrimaryAttackRequested)
//  {
        if (!(iButtonStatus & GAME_BUTTON_STATUS_DOWN))
            return false;

        // Make sure non-automatic items don't activate if the button is held
        if (GetImpulseOnly() && !(iButtonStatus & GAME_BUTTON_STATUS_PRESSED))
            return false;
//  }

    // Jump attack
    if (!pOwner->IsOnGround() && m_pEntityConfig->GetJumpAttackEnabled() && (pOwner->GetMoveFlags() & MOVE_JUMPING))
    {
        if (!pOwner->IsIdle())
        {
    //      m_bPrimaryAttackRequested = true;

            if (!(pOwner->GetAction() & PLAYER_ACTION_JUMP_ATTACK))
                return false;
            if (GetJumpAttackChainTime(pOwner->GetAttackSequence()) == 0)
                return false;
            if (Game.GetGameTime() - pOwner->GetLastMeleeAttackTime() < ModifyAttackTime(pOwner, GetJumpAttackChainTime(pOwner->GetAttackSequence())))
                return false;
            pOwner->IncrementAttackSequence();
        }
        else
        {
            pOwner->StartJumpAttackSequence();
        }
        byte ySequence(pOwner->GetAttackSequence());

    //  m_bPrimaryAttackRequested = false;
    //  m_bSecondaryAttackRequested = false;

        // Get base stats for the attack
        CMeleeAttackEvent &attack(pOwner->GetAttackEvent());
        attack.Clear();
        attack.SetWeaponPointer(this);
        attack.SetAnim(GetJumpAttackAnimName(ySequence), ModifyAttackTime(pOwner, GetJumpAttackTime(ySequence)));
        attack.SetDamage(pOwner->GetJumpAttackMinDamage(m_ySlot), pOwner->GetJumpAttackMaxDamage(m_ySlot));
        attack.SetDamageFlags(GetPrimaryDamageFlags() | (GetJumpAttackBlockable() ? DAMAGE_FLAG_BLOCKABLE : 0));
        attack.SetRearAttackMultiplier(m_pEntityConfig->GetRearMultiplier());
        attack.SetImpactTime(Game.GetGameTime() + ModifyAttackTime(pOwner, GetJumpAttackImpactTime(ySequence)));
        attack.SetImmobileTime(ModifyAttackTime(pOwner, m_pEntityConfig->GetJumpAttackImmobileTime()));
        attack.SetPivot(GetJumpAttackPivotHeight(), GetJumpAttackPivotFactor());
        attack.SetMetric(MELEE_HEIGHT, GetJumpAttackHeightMin(), GetJumpAttackHeightMax(), GetJumpAttackHeightStep());
        attack.SetMetric(MELEE_ANGLE, GetJumpAttackAngleMin(), GetJumpAttackAngleMax(), GetJumpAttackAngleStep());
        attack.SetMetric(MELEE_RANGE, GetJumpAttackRangeMin(), GetJumpAttackRangeMax(), GetJumpAttackRangeStep());
        attack.SetStaminaCost(m_pEntityConfig->GetJumpAttackStaminaCost());
        attack.SetPush(m_pEntityConfig->GetJumpAttackPush());
        attack.SetLunge(m_pEntityConfig->GetJumpAttackLunge());
        attack.SetStaminaRequired(m_pEntityConfig->GetJumpAttackStaminaRequired());
        attack.SetStartTime(Game.GetGameTime());

        if (StartAttack(true))
        {
            if (attack.GetImmobileTime() > 0)
            {
                pOwner->SetAction(PLAYER_ACTION_JUMP_ATTACK | PLAYER_ACTION_IMMOBILE, Game.GetGameTime() + attack.GetImmobileTime());
                pOwner->StopAnimation(0);
            }
            else
                pOwner->SetAction(PLAYER_ACTION_JUMP_ATTACK, Game.GetGameTime() + attack.GetLength());

            return true;
        }
    }
    else
    {
        if (!pOwner->IsIdle())
        {
    //      m_bPrimaryAttackRequested = true;

            if (!(pOwner->GetAction() & PLAYER_ACTION_QUICK_ATTACK))
                return false;
            if (GetQuickAttackChainTime(pOwner->GetAttackSequence()) == 0)
                return false;
            if (Game.GetGameTime() - pOwner->GetLastMeleeAttackTime() < ModifyAttackTime(pOwner, GetQuickAttackChainTime(pOwner->GetAttackSequence())))
                return false;
            pOwner->IncrementAttackSequence();
        }
        else
        {
            pOwner->StartQuickAttackSequence();
        }
        byte ySequence(pOwner->GetAttackSequence());

    //  m_bPrimaryAttackRequested = false;
    //  m_bSecondaryAttackRequested = false;

        // Get base stats for the attack
        CMeleeAttackEvent &attack(pOwner->GetAttackEvent());
        attack.Clear();
        attack.SetWeaponPointer(this);
        attack.SetDamage(GetQuickAttackMinDamage(ySequence), GetQuickAttackMaxDamage(ySequence));
        attack.SetDamageFlags(GetPrimaryDamageFlags() | (GetQuickAttackBlockable() ? DAMAGE_FLAG_BLOCKABLE : 0));
        attack.SetRearAttackMultiplier(m_pEntityConfig->GetRearMultiplier());
        attack.SetImpactTime(Game.GetGameTime() + ModifyAttackTime(pOwner, GetQuickAttackImpactTime(ySequence)));
        attack.SetImmobileTime(ModifyAttackTime(pOwner, GetQuickAttackImmobileTime(ySequence)));
        attack.SetPivot(GetQuickAttackPivotHeight(), GetQuickAttackPivotFactor());
        attack.SetMetric(MELEE_HEIGHT, GetQuickAttackHeightMin(), GetQuickAttackHeightMax(), GetQuickAttackHeightStep());
        attack.SetMetric(MELEE_ANGLE, GetQuickAttackAngleMin(), GetQuickAttackAngleMax(), GetQuickAttackAngleStep());
        attack.SetMetric(MELEE_RANGE, GetQuickAttackRangeMin(), GetQuickAttackRangeMax(), GetQuickAttackRangeStep());
        attack.SetStaminaCost(GetQuickAttackStaminaCost(ySequence));
        attack.SetPush(m_pEntityConfig->GetQuickAttackPush());
        attack.SetLunge(m_pEntityConfig->GetQuickAttackLunge());
        attack.SetStaminaRequired(m_pEntityConfig->GetQuickAttackStaminaRequired());
        attack.SetStartTime(Game.GetGameTime());

        // Anim
        tstring sAnim;
        if (pOwner->IsPlayer())
        {
            IPlayerEntity *pPlayer(pOwner->GetAsPlayerEnt());

            if (pPlayer->GetCurrentMovement() & PLAYER_MOVE_BACK)
            {
                sAnim = GetQuickAttackAnimNameRunBack(ySequence);
            }
            else if (pPlayer->GetCurrentMovement() & (PLAYER_MOVE_FWD | PLAYER_MOVE_LEFT | PLAYER_MOVE_RIGHT))
            {
                if (pPlayer->GetCurrentMovement() & PLAYER_MOVE_SPRINT)
                    sAnim = GetQuickAttackAnimNameSprint(ySequence);
                else
                    sAnim = GetQuickAttackAnimNameRunForward(ySequence);
            }
            else
            {
                sAnim = GetQuickAttackAnimName(ySequence);
            }
        }
        else if (pOwner->IsPet())
        {
            IPetEntity *pPet(pOwner->GetAsPet());

            if (pPet->GetCurrentMovement() & PET_MOVE_BACK)
            {
                sAnim = GetQuickAttackAnimNameRunBack(ySequence);
            }
            else if (pPet->GetCurrentMovement() & (PET_MOVE_FWD | PET_MOVE_LEFT | PET_MOVE_RIGHT))
            {
                if (pPet->GetCurrentMovement() & PET_MOVE_SPRINT)
                    sAnim = GetQuickAttackAnimNameSprint(ySequence);
                else
                    sAnim = GetQuickAttackAnimNameRunForward(ySequence);
            }
            else
            {
                sAnim = GetQuickAttackAnimName(ySequence);
            }
        }
        else
        {
            sAnim = GetQuickAttackAnimName(ySequence);
        }
        
        attack.SetAnim(sAnim, ModifyAttackTime(pOwner, GetQuickAttackTime(ySequence)));

        if (StartAttack(false))
        {
            if (attack.GetImmobileTime() > 0)
            {
                pOwner->SetAction(PLAYER_ACTION_QUICK_ATTACK | PLAYER_ACTION_IMMOBILE, Game.GetGameTime() + attack.GetImmobileTime());
                pOwner->StopAnimation(0);
            }
            else
                pOwner->SetAction(PLAYER_ACTION_QUICK_ATTACK, Game.GetGameTime() + attack.GetLength());

            return true;
        }
    }

    return false;
}


/*====================
  IMeleeItem::ActivateSecondary
  ====================*/
bool    IMeleeItem::ActivateSecondary(int iButtonStatus)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return true;

//  if (!m_bSecondaryAttackRequested)
//  {
        if (!(iButtonStatus & GAME_BUTTON_STATUS_DOWN))
            return false;

        // Make sure non-automatic items don't activate if the button is held
        if (GetImpulseOnly() && !(iButtonStatus & GAME_BUTTON_STATUS_PRESSED))
            return false;
//  }

    if (!pOwner->IsIdle())
    {
//      m_bSecondaryAttackRequested = true;

        if (!(pOwner->GetAction() & PLAYER_ACTION_STRONG_ATTACK))
            return false;
        if (GetStrongAttackChainTime(pOwner->GetAttackSequence()) == 0)
            return false;
        if (Game.GetGameTime() - pOwner->GetLastMeleeAttackTime() < ModifyAttackTime(pOwner, GetStrongAttackChainTime(pOwner->GetAttackSequence())))
            return false;
        pOwner->IncrementAttackSequence();
    }
    else
    {
        pOwner->StartStrongAttackSequence();
    }
    byte ySequence(pOwner->GetAttackSequence());


    // Get base stats for the attack
    CMeleeAttackEvent   &attack(pOwner->GetAttackEvent());
    attack.Clear();
    attack.SetWeaponPointer(this);
    attack.SetAnim(GetStrongAttackAnimName(ySequence), ModifyAttackTime(pOwner, GetStrongAttackTime(ySequence)));
    attack.SetDamage(pOwner->GetStrongAttackMinDamage(m_ySlot), pOwner->GetStrongAttackMaxDamage(m_ySlot));
    attack.SetDamageFlags(GetSecondaryDamageFlags());
    attack.SetImpactTime(Game.GetGameTime() + ModifyAttackTime(pOwner, GetStrongAttackImpactTime(ySequence)));
    attack.SetImmobileTime(ModifyAttackTime(pOwner, m_pEntityConfig->GetStrongAttackImmobileTime()));
    attack.SetPivot(m_pEntityConfig->GetStrongAttackPivotHeight(), m_pEntityConfig->GetStrongAttackPivotFactor());
    attack.SetMetric(MELEE_HEIGHT, GetStrongAttackHeightMin(), GetStrongAttackHeightMax(), GetStrongAttackHeightStep());
    attack.SetMetric(MELEE_ANGLE, GetStrongAttackAngleMin(), GetStrongAttackAngleMax(), GetStrongAttackAngleStep());
    attack.SetMetric(MELEE_RANGE, GetStrongAttackRangeMin(), GetStrongAttackRangeMax(), GetStrongAttackRangeStep());
    attack.SetStaminaCost(m_pEntityConfig->GetStrongAttackStaminaCost());
    attack.SetPush(m_pEntityConfig->GetStrongAttackPush());
    attack.SetLunge(m_pEntityConfig->GetStrongAttackLunge());
    attack.SetStaminaRequired(m_pEntityConfig->GetStrongAttackStaminaRequired());
    attack.SetStartTime(Game.GetGameTime());

    if (StartAttack(false))
    {
        if (attack.GetImmobileTime() > 0)
        {
            pOwner->SetAction(PLAYER_ACTION_STRONG_ATTACK | PLAYER_ACTION_IMMOBILE, Game.GetGameTime() + attack.GetImmobileTime());
            pOwner->StopAnimation(0);
        }
        else
            pOwner->SetAction(PLAYER_ACTION_STRONG_ATTACK, Game.GetGameTime() + attack.GetLength());

        return true;
    }

    return false;
}


/*====================
  IMeleeItem::ActivateTertiary
  ====================*/
bool    IMeleeItem::ActivateTertiary(int iButtonStatus)
{
    if (g_meleeNewBlock)
    {
        ICombatEntity *pOwner(GetOwnerEnt());
        if (!pOwner)
            return true;

        if (!(iButtonStatus & GAME_BUTTON_STATUS_DOWN))
            return false;

        // Make sure non-automatic items don't activate if the button is held
        if (GetImpulseOnly() && !(iButtonStatus & GAME_BUTTON_STATUS_PRESSED))
            return false;

        if (!pOwner->IsIdle())
            return false;

        // Get base stats for the attack
        CMeleeAttackEvent   &attack(pOwner->GetAttackEvent());
        attack.Clear();
        attack.SetWeaponPointer(this);
        attack.SetAnim(m_pEntityConfig->GetBlockAnimName(), ModifyAttackTime(pOwner, GetBlockTime()));
        attack.SetImmobileTime(ModifyAttackTime(pOwner, m_pEntityConfig->GetBlockImmobileTime()));
        attack.SetStaminaCost(m_pEntityConfig->GetBlockStaminaCost());
        attack.SetLunge(m_pEntityConfig->GetBlockLunge());
        attack.SetPush(m_pEntityConfig->GetBlockPush());
        attack.SetImpactTime(Game.GetGameTime() + GetBlockImpactTime());
        attack.SetImpactEndTime(attack.GetImpactTime() + GetBlockDuration());
        attack.SetStaminaRequired(m_pEntityConfig->GetBlockStaminaRequired());
        attack.SetStartTime(Game.GetGameTime());

        if (StartAttack(false))
        {
            if (attack.GetImmobileTime() > 0)
            {
                pOwner->SetAction(PLAYER_ACTION_BLOCK | PLAYER_ACTION_IMMOBILE, Game.GetGameTime() + attack.GetImmobileTime());
                pOwner->StopAnimation(0);
            }
            else
                pOwner->SetAction(PLAYER_ACTION_BLOCK, Game.GetGameTime() + attack.GetLength());

            return true;
        }

        return false;
    }
    else
    {
        ICombatEntity *pOwner(GetOwnerEnt());
        if (!pOwner)
            return true;

        if (iButtonStatus & GAME_BUTTON_STATUS_UP || iButtonStatus & GAME_BUTTON_STATUS_RELEASED)
        {
            if (pOwner->GetAction() & PLAYER_ACTION_UNBLOCK)
                pOwner->SetAction(PLAYER_ACTION_IDLE, INVALID_TIME);
            else if (pOwner->GetAction() & (PLAYER_ACTION_BLOCK) && iButtonStatus & GAME_BUTTON_STATUS_RELEASED)
                pOwner->SetAction(PLAYER_ACTION_BLOCK, Game.GetGameTime() + m_pEntityConfig->GetBlockRecoverTime());

            return false;
        }

        if (!pOwner->IsIdle() && !(pOwner->GetAction() & PLAYER_ACTION_BLOCK))
            return false;

        if (pOwner->GetAction() & PLAYER_ACTION_UNBLOCK)
            return false;
            
        if (!(pOwner->GetAction() & PLAYER_ACTION_BLOCK))
        {
            //Console << _T("Start block") << newl;
            //pOwner->StartAnimation(m_pEntityConfig->GetBlockAnimName(), 1);
            //pOwner->SetAction(PLAYER_ACTION_BLOCK, uiEndTime);

            CMeleeAttackEvent &attack(pOwner->GetAttackEvent());
            attack.Clear();
            attack.SetWeaponPointer(this);
            attack.SetAnim(m_pEntityConfig->GetBlockAnimName(), ModifyAttackTime(pOwner, GetBlockTime()));
            attack.SetStaminaCost(m_pEntityConfig->GetBlockStaminaCost());
            attack.SetLunge(m_pEntityConfig->GetBlockLunge());
            attack.SetPush(m_pEntityConfig->GetBlockPush());
            attack.SetImpactTime(Game.GetGameTime() + GetBlockImpactTime());
            attack.SetStartTime(Game.GetGameTime());

            if (StartAttack(false))
            {
                uint uiEndTime(m_pEntityConfig->GetBlockMaxTime() == 0 ? INVALID_TIME : Game.GetGameTime() + m_pEntityConfig->GetBlockMaxTime());
                pOwner->SetAction(PLAYER_ACTION_BLOCK, uiEndTime);
            }
        }
        return true;
    }
}


/*====================
  IMeleeItem::FinishedAction
  ====================*/
void    IMeleeItem::FinishedAction(int iAction)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return;

    CMeleeAttackEvent &attack(pOwner->GetAttackEvent());

    // If they are immobile due to an attack and the attack
    // should still be occuring, remove the immobile flag and
    // continue, otherwise end the action altogether.
    if ((iAction & PLAYER_ACTION_IMMOBILE) && (
        (iAction & PLAYER_ACTION_QUICK_ATTACK) ||
        (iAction & PLAYER_ACTION_STRONG_ATTACK) ||
        (iAction & PLAYER_ACTION_BLOCK) ||
        (iAction & PLAYER_ACTION_JUMP_ATTACK)))
    {
        if (attack.GetImmobileTime() < attack.GetLength())
        {
            pOwner->SetAction(iAction & ~PLAYER_ACTION_IMMOBILE, (attack.GetLength() + Game.GetGameTime()) - attack.GetImmobileTime());
            return;
        }
    }

    attack.Clear();

    if (iAction == PLAYER_ACTION_BLOCK && !g_meleeNewBlock)
    {
        //Console << _T("Stop block") << newl;
        pOwner->StopAnimation(1);
        pOwner->SetAction(PLAYER_ACTION_UNBLOCK, INVALID_TIME);
    }
    else
    {
        pOwner->SetAction(PLAYER_ACTION_IDLE, INVALID_TIME);
    }
}


/*====================
  IMeleeItem::Selected
  ====================*/
void    IMeleeItem::Selected()
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return;

    pOwner->SetFov(pOwner->GetBaseFov());
    pOwner->StartAnimation(_T("melee_draw"), 1);
    pOwner->SetDefaultAnimation(_T("idle"));
}


/*====================
  IMeleeItem::Unselected
  ====================*/
void    IMeleeItem::Unselected()
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return;

    if (pOwner->GetAction() == PLAYER_ACTION_UNBLOCK || pOwner->GetAction() == PLAYER_ACTION_BLOCK)
        pOwner->SetAction(PLAYER_ACTION_IDLE, INVALID_TIME);
}


/*====================
  IMeleeItem::ClientPrecache
  ====================*/
void    IMeleeItem::ClientPrecache(CEntityConfig *pConfig)
{
    g_ResourceManager.Register(_T("/shared/effects/bloodspray_small.effect"), RES_EFFECT);

    IInventoryItem::ClientPrecache(pConfig);

    if (!pConfig)
        return;

    if (!pConfig->GetImpactEffectPath().empty())
        g_ResourceManager.Register(pConfig->GetImpactEffectPath(), RES_EFFECT);
}


/*====================
  IMeleeItem::ServerPrecache
  ====================*/
void    IMeleeItem::ServerPrecache(CEntityConfig *pConfig)
{
    g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(_T("/shared/effects/bloodspray_small.effect"), RES_EFFECT, RES_EFFECT_IGNORE_ALL));

    IInventoryItem::ServerPrecache(pConfig);

    if (!pConfig)
        return;

    if (!pConfig->GetImpactEffectPath().empty())
        g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetImpactEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));
}


/*====================
  IMeleeItem::GetQuickAttackMinDamage
  ====================*/
float   IMeleeItem::GetQuickAttackMinDamage(byte ySequence) const
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (pOwner == NULL)
        return 0.0f;

    float fDamage(GetQuickAttackMinDamage(ySequence));
    if (GetApplyAttributes())
        fDamage *= (1.0f + GetOwnerEnt()->GetAttributeBoost(ATTRIBUTE_STRENGTH));

    return pOwner->GetModifiedDamage(fDamage);
}


/*====================
  IMeleeItem::GetQuickAttackMaxDamage
  ====================*/
float   IMeleeItem::GetQuickAttackMaxDamage(byte ySequence) const
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (pOwner == NULL)
        return 0.0f;

    float fDamage(GetQuickAttackMaxDamage(ySequence));
    if (GetApplyAttributes())
        fDamage *= (1.0f + GetOwnerEnt()->GetAttributeBoost(ATTRIBUTE_STRENGTH));

    return pOwner->GetModifiedDamage(fDamage);
}
