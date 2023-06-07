// (C)2006 S2 Games
// i_spellitem.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_spellitem.h"
#include "c_spellpolymorph.h"

#include "../k2/s_traceinfo.h"
#include "../k2/c_clientsnapshot.h"
#include "../k2/c_effect.h"
//=============================================================================

/*====================
  ISpellItem::CEntityConfig::CEntityConfig
  ====================*/
ISpellItem::CEntityConfig::CEntityConfig(const tstring &sName) :
IInventoryItem::CEntityConfig(sName),
INIT_ENTITY_CVAR(CastTime, 200),
INIT_ENTITY_CVAR(ImpactTime, 0),
INIT_ENTITY_CVAR(AnimName, _T("")),
INIT_ENTITY_CVAR(AnimChannel, 1),
INIT_ENTITY_CVAR(TargetRadius, 0.0f),
INIT_ENTITY_CVAR(TargetState, _T("")),
INIT_ENTITY_CVAR(TargetStateDuration, 10000),
INIT_ENTITY_CVAR(CursorPath, _T("")),
INIT_ENTITY_CVAR(Snapcast, false),
INIT_ENTITY_CVAR(RadiusCast, false),
INIT_ENTITY_CVAR(Range, 1000.0f),
INIT_ENTITY_CVAR(MinRange, 0.0f),
INIT_ENTITY_CVAR(SnapcastBreakAngle, 25.0f),
INIT_ENTITY_CVAR(SnapcastSelectColor, CVec3f(0.0f, 1.0f, 0.0f)),
INIT_ENTITY_CVAR(TargetMaterialPath, _T("")),
INIT_ENTITY_CVAR(CastEffectPath, _T("")),
INIT_ENTITY_CVAR(ImpactEffectPath, _T("")),
INIT_ENTITY_CVAR(Freeze, true),
INIT_ENTITY_CVAR(TargetStatusLiving, false),
INIT_ENTITY_CVAR(TargetStatusDead, false),
INIT_ENTITY_CVAR(TargetStatusCorpse, false),
INIT_ENTITY_CVAR(TargetTeamAlly, false),
INIT_ENTITY_CVAR(TargetTeamEnemy, false),
INIT_ENTITY_CVAR(TargetTypePlayer, false),
INIT_ENTITY_CVAR(TargetTypeVehicle, false),
INIT_ENTITY_CVAR(TargetTypeBuilding, false),
INIT_ENTITY_CVAR(TargetTypeGadget, false),
INIT_ENTITY_CVAR(TargetTypeHellbourne, false),
INIT_ENTITY_CVAR(TargetTypePet, false),
INIT_ENTITY_CVAR(TargetTypeNPC, false),
INIT_ENTITY_CVAR(TargetTypeSiege, false),
INIT_ENTITY_CVAR(CastExperience, 0.0f),
INIT_ENTITY_CVAR(GadgetName, _T("")),
INIT_ENTITY_CVAR(MaxDeployable, -1)
{
}


/*====================
  ISpellItem::ISpellItem
  ====================*/
ISpellItem::ISpellItem(CEntityConfig *pConfig) :
IInventoryItem(pConfig),
m_pEntityConfig(pConfig),

m_uiTargetIndex(INVALID_INDEX),
m_fManaSpent(0.0f)
{
}

const CSpellPolymorph*      ISpellItem::GetAsSpellPolymorph() const     { if (!IsSpellToggle()) return NULL; else return static_cast<const CSpellPolymorph*>(this); }
CSpellPolymorph*            ISpellItem::GetAsSpellPolymorph()           { if (!IsSpellToggle()) return NULL; else return static_cast<CSpellPolymorph*>(this); }

/*====================
  ISpellItem::GetTargetLocation
  ====================*/
CVec3f  ISpellItem::GetTargetLocation() const
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return CVec3f(0.0f, 0.0f, 0.0f);

    if (m_pEntityConfig->GetRadiusCast())
        return pOwner->GetPosition() + pOwner->GetBounds().GetMid();

    return pOwner->GetTargetPosition(m_pEntityConfig->GetRange(), m_pEntityConfig->GetMinRange());
}


/*====================
  ISpellItem::TryImpact
  ====================*/
bool    ISpellItem::TryImpact()
{
    // Client does not predict the execution
    if (Game.IsClient())
        return false;

    bool bSucceeded;

    CGameEvent evImpact;
    if (m_pEntityConfig->GetSnapcast())
        bSucceeded = ImpactEntity(m_uiTargetIndex, evImpact);
    else
        bSucceeded = ImpactPosition(m_v3TargetPosition, evImpact);

    m_uiTargetIndex = INVALID_INDEX;
    m_v3TargetPosition.Clear();

    if (bSucceeded)
    {
        // Impact event
        if (!m_pEntityConfig->GetImpactEffectPath().empty())
        {
            evImpact.SetEffect(Game.RegisterEffect(m_pEntityConfig->GetImpactEffectPath()));
            Game.AddEvent(evImpact);
        }

        return true;
    }
    else
    {
        return false;
    }
}


/*====================
  ISpellItem::ImpactEntity
  ====================*/
bool    ISpellItem::ImpactEntity(uint uiTargetIndex, CGameEvent &evImpact, bool bCheckTarget)
{
    ICombatEntity *pOwner(GetOwnerEnt());

    IVisualEntity *pTarget(Game.GetVisualEntity(uiTargetIndex));
    if (pTarget == NULL)
        return false;
    if (bCheckTarget && !IsValidTarget(pTarget, true))
        return false;

    int iStateSlot(-1);

    if (!m_pEntityConfig->GetTargetState().empty())
    {
        iStateSlot = pTarget->ApplyState(EntityRegistry.LookupID(m_pEntityConfig->GetTargetState()), Game.GetGameTime(), m_pEntityConfig->GetTargetStateDuration(), (pOwner != NULL ? pOwner->GetIndex() : INVALID_INDEX));

        // If appropriate, reduce duration by spell resistance
        if (iStateSlot != -1 && pTarget->IsCombat())
        {
            IEntityState *pState(pTarget->GetState(iStateSlot));
            
            if (pState != NULL && pState->IsDebuff())
                pState->SetDuration(INT_CEIL(float(pState->GetDuration()) * pTarget->GetAsCombatEnt()->GetSpellResistance()));
        }

        IPlayerEntity *pOwner(Game.GetPlayerEntity(GetOwner()));
        if (pOwner != NULL)
            pOwner->GiveExperience(m_pEntityConfig->GetCastExperience(), pTarget->GetPosition());
    }

    evImpact.SetSourcePosition(pTarget->GetPosition());
    evImpact.SetSourceAngles(pTarget->GetAngles());

    if (pOwner != NULL && pOwner->IsPlayer() && iStateSlot != -1)
    {
        IEntityState *pState(pTarget->GetState(iStateSlot));
        Game.MatchStatEvent(pOwner->GetAsPlayerEnt()->GetClientID(), (pState->IsBuff() ? COMMANDER_MATCH_BUFFS : COMMANDER_MATCH_DEBUFFS), 1, (pTarget->IsPlayer() ? pTarget->GetAsPlayerEnt()->GetClientID() : -1), GetType(), (pTarget->IsPlayer() ? INVALID_ENT_TYPE : pTarget->GetType()));
    }

    if (pOwner != NULL)
        Game.RegisterTriggerParam(_T("index"), XtoA(pOwner->GetIndex()));

    Game.RegisterTriggerParam(_T("targetindex"), XtoA(uiTargetIndex));
    Game.RegisterTriggerParam(_T("name"), GetName());
    Game.RegisterTriggerParam(_T("type"), GetTypeName());

    Game.TriggerGlobalScript(_T("spellimpact"));

    return true;
}


/*====================
  ISpellItem::CreateGadget
  ====================*/
bool    ISpellItem::CreateGadget(const CVec3f &v3Target)
{
    if (m_pEntityConfig->GetGadgetName().empty())
        return false;

    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;

    ushort unType(EntityRegistry.LookupID(m_pEntityConfig->GetGadgetName()));
    if (unType == INVALID_ENT_TYPE)
        return false;

    // Count existing
    IGameEntity *pKillTarget(NULL);
    if (m_pEntityConfig->GetMaxDeployable() > 0 && pOwner->IsPlayer())
    {
        int iCount(0);
        IGameEntity *pEnt(Game.GetFirstEntity());
        IGameEntity *pOldest(NULL);
        uint uiOldestTime(-1);
        while (pEnt != NULL)
        {
            IGadgetEntity *pGadget(pEnt->GetAsGadget());
            if (pGadget != NULL &&
                pGadget->GetType() == unType &&
                pGadget->GetOwnerClientNumber() == pOwner->GetAsPlayerEnt()->GetClientID() &&
                pGadget->GetStatus() == ENTITY_STATUS_ACTIVE)
            {
                ++iCount;
                if (pGadget->GetSpawnTime() < uiOldestTime)
                {
                    uiOldestTime = pGadget->GetSpawnTime();
                    pOldest = pGadget;
                }
            }

            pEnt = Game.GetNextEntity(pEnt);
        }

        if (iCount >= m_pEntityConfig->GetMaxDeployable())
            pKillTarget = pOldest;
    }

    IGameEntity *pNewEnt(Game.AllocateEntity(unType));
    if (pNewEnt == NULL)
        return false;
    IGadgetEntity *pGadget(pNewEnt->GetAsGadget());
    if (pGadget == NULL)
        return false;

    CAxis axis(pOwner->GetAngles());
    CVec3f v3Normal(Game.GetTerrainNormal(v3Target.x, v3Target.y));
    CVec3f v3Angles(
        90.0f + RAD2DEG(acos(DotProduct(v3Normal, CVec3f(axis.Forward2d(), 0.0f)))),
        90.0f + RAD2DEG(acos(DotProduct(v3Normal, CVec3f(axis.Right2d(), 0.0f)))),
        pOwner->GetAngles()[YAW]);

    pGadget->SetOwner(pOwner->GetIndex());
    pGadget->SetTeam(pOwner->GetTeam());
    pGadget->SetSquad(pOwner->GetSquad());
    pGadget->SetPosition(CVec3f(v3Target.x, v3Target.y, Game.GetTerrainHeight(v3Target.x, v3Target.y)));
    pGadget->SetAngles(v3Angles);

    if (pOwner->IsPlayer())
        pGadget->SetOwnerClientNumber(pOwner->GetAsPlayerEnt()->GetClientID());

    if (pGadget->CanSpawn())
    {
        pGadget->Spawn();

        Game.RegisterTriggerParam(_T("index"), XtoA(pOwner->GetIndex()));
        Game.RegisterTriggerParam(_T("gadgetindex"), XtoA(pGadget->GetIndex()));
        Game.RegisterTriggerParam(_T("posx"), XtoA(v3Target.x));
        Game.RegisterTriggerParam(_T("posy"), XtoA(v3Target.y));
        Game.RegisterTriggerParam(_T("posz"), XtoA(pGadget->GetPosition()[Z]));
        Game.RegisterTriggerParam(_T("name"), pGadget->GetName());
        Game.RegisterTriggerParam(_T("type"), pGadget->GetTypeName());

        Game.TriggerGlobalScript(_T("placegadget"));

        if (pKillTarget != NULL)
            pKillTarget->Kill();

        IPlayerEntity *pOwner(Game.GetPlayerEntity(GetOwner()));
        if (pOwner != NULL)
            pOwner->GiveExperience(m_pEntityConfig->GetCastExperience(), pGadget->GetPosition());

        return true;
    }
    else
    {
        Game.DeleteEntity(pGadget);
        pOwner->GiveMana(m_fManaSpent);
        SetCooldownTimer(INVALID_TIME, INVALID_TIME);
        m_fManaSpent = 0.0f;

        return false;
    }
}


/*====================
  ISpellItem::ImpactPosition
  ====================*/
bool    ISpellItem::ImpactPosition(const CVec3f &v3Target, CGameEvent &evImpact)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;
    
    if (!m_pEntityConfig->GetGadgetName().empty())
    {
        if (!CreateGadget(v3Target))
            return false;
    }

    if (m_pEntityConfig->GetTargetRadius() > 0.0f)
    {
        static uivector vResult;
        vResult.clear();
        Game.GetEntitiesInRadius(vResult, CSphere(v3Target, m_pEntityConfig->GetTargetRadius()), 0);
        for (uivector_it it(vResult.begin()); it != vResult.end(); ++it)
            ImpactEntity(Game.GetGameIndexFromWorldIndex(*it), evImpact);
    }

    evImpact.SetSourcePosition(v3Target);
    evImpact.SetSourceAngles(CVec3f(0.0f, 0.0f, pOwner->GetAngles()[YAW]));

    return true;
}


/*====================
  ISpellItem::ActivatePrimary
  ====================*/
bool    ISpellItem::ActivatePrimary(int iButtonStatus)
{
    if (Game.IsClient())
        return false;

    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;

    // Check to see if the spell is ready yet
    if (!IsReady())
        return false;

    // If the spell is not selected, this was a request to activate it
    if (pOwner->GetSelectedItem() != m_ySlot && !m_pEntityConfig->GetRadiusCast())
    {
        Console << _T("Priming spell: ") << m_ySlot << _T(" [") << pOwner->GetSelectedItem() << _T("]") << newl;
        pOwner->SelectItem(m_ySlot);
    }

    // Only activate on an impulse (otherwise it would always fire right away)
    if (!(iButtonStatus & GAME_BUTTON_STATUS_PRESSED))
        return false;

    // Validate the target
    if (m_pEntityConfig->GetSnapcast())
    {
        IGameEntity *pTarget(Game.GetEntity(pOwner->GetSpellTargetIndex()));
        if (pTarget == NULL)
            return false;
    }

    // Information for cast event
    CVec3f v3CastPos;
    CVec3f v3CastAngles(V_ZERO);

    // Determine target position and angles
    if (m_pEntityConfig->GetSnapcast())
    {
        IVisualEntity *pTarget(Game.GetVisualEntity(pOwner->GetSpellTargetIndex()));
        if (pTarget == NULL)
            return false;

        m_uiTargetIndex = pTarget->GetIndex();

        v3CastPos = pTarget->GetPosition();
        v3CastAngles = pTarget->GetAngles();
    }
    else if (m_pEntityConfig->GetRadiusCast())
    {
        v3CastPos = m_v3TargetPosition = pOwner->GetPosition();
        v3CastAngles[YAW] = pOwner->GetAngles()[YAW];
    }
    else
    {
        v3CastPos = m_v3TargetPosition = GetTargetLocation();
        v3CastAngles[YAW] = pOwner->GetAngles()[YAW];
    }

    if (!m_pEntityConfig->GetGadgetName().empty())
    {
        ushort unType(EntityRegistry.LookupID(m_pEntityConfig->GetGadgetName()));
        if (unType == INVALID_ENT_TYPE)
            return false;

        CAxis axis(pOwner->GetAngles());
        CVec3f v3Normal(Game.GetTerrainNormal(v3CastPos.x, v3CastPos.y));
        CVec3f v3Angles(
            90.0f + RAD2DEG(acos(DotProduct(v3Normal, CVec3f(axis.Forward2d(), 0.0f)))),
            90.0f + RAD2DEG(acos(DotProduct(v3Normal, CVec3f(axis.Right2d(), 0.0f)))),
            pOwner->GetAngles()[YAW]);

        bool bCanSpawn(IGadgetEntity::TestSpawn(Game.RegisterModel(EntityRegistry.GetGameSettingString(unType, _T("ModelPath"))),
            CVec3f(v3CastPos.x, v3CastPos.y, Game.GetTerrainHeight(v3CastPos.x, v3CastPos.y)), 
            v3Angles, (EntityRegistry.GetGameSettingFloat(unType, _T("Scale"), 1.0f))));

        if (!bCanSpawn)
            return false;
    }

    // Check mana
    m_fManaSpent = m_pEntityConfig->GetManaCost();
    if (!pOwner->SpendMana(m_fManaSpent))
    {
        m_fManaSpent = 0.0f;
        return false;
    }

    // Cast event
    if (!m_pEntityConfig->GetCastEffectPath().empty())
    {
        CGameEvent evCast;
        evCast.SetSourcePosition(v3CastPos);
        evCast.SetSourceAngles(v3CastAngles);
        evCast.SetEffect(Game.RegisterEffect(m_pEntityConfig->GetCastEffectPath()));
        Game.AddEvent(evCast);
    }

    int iAction(PLAYER_ACTION_SPELL);
    if (m_pEntityConfig->GetFreeze())
    {
        pOwner->StopAnimation(0);
        iAction |= PLAYER_ACTION_IMMOBILE;
    }

    CSpellActivateEvent &spellEvent(pOwner->GetSpellActivateEvent());
    spellEvent.Clear();
    spellEvent.SetOwner(pOwner);
    spellEvent.SetSlot(m_ySlot);
    spellEvent.SetActivateTime(Game.GetGameTime() + m_pEntityConfig->GetImpactTime());

    pOwner->SetAction(iAction, Game.GetGameTime() + m_pEntityConfig->GetCastTime());
    pOwner->StartAnimation(m_pEntityConfig->GetAnimName(), m_pEntityConfig->GetAnimChannel());
    SetCooldownTimer(Game.GetGameTime(), m_pEntityConfig->GetCooldownTime());
    return true;
}


/*====================
  ISpellItem::ActivateSecondary
  ====================*/
bool    ISpellItem::ActivateSecondary(int iButtonStatus)
{
    return Cancel(iButtonStatus);
}


/*====================
  ISpellItem::Cancel
  ====================*/
bool    ISpellItem::Cancel(int iButtonStatus)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;

    if (!(iButtonStatus & GAME_BUTTON_STATUS_PRESSED))
        return false;

    if (pOwner != NULL)
        Game.SelectItem(pOwner->GetDefaultInventorySlot());
    
    return true;
}


/*====================
  ISpellItem::IsValidTarget
  ====================*/
bool    ISpellItem::IsValidTarget(IGameEntity *pEntity, bool bImpact)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;

    if (!pEntity || !pEntity->IsVisual())
        return false;

    IVisualEntity *pVisual(pEntity->GetAsVisualEnt());

    bool bValidStatus(false);
    if (m_pEntityConfig->GetTargetStatusLiving() && (pVisual->GetStatus() == ENTITY_STATUS_ACTIVE || pVisual->GetStatus() == ENTITY_STATUS_SPAWNING))
        bValidStatus = true;
    else if (m_pEntityConfig->GetTargetStatusDead() && pVisual->GetStatus() == ENTITY_STATUS_DEAD && !pVisual->HasNetFlags(ENT_NET_FLAG_NO_CORPSE))
        bValidStatus = true;
    else if (m_pEntityConfig->GetTargetStatusCorpse() && pVisual->GetStatus() == ENTITY_STATUS_CORPSE)
        bValidStatus = true;

    // When the target wants an ally, if the target is an ally or even just looks like an ally, they are valid
    // This means a player can be healed while disguised by either team, for instance
    // If the target wants an enemy, a disguised player is only valid when the effect is actually applied
    // This means they can't be snapped to, but an area effect will hit them, even though they don't "light up" as a target
    bool bValidTeam(false);
    if (m_pEntityConfig->GetTargetTeamAlly() && (!pOwner->LooksLikeEnemy(pVisual) || !pOwner->IsEnemy(pVisual)))
        bValidTeam = true;
    else if (m_pEntityConfig->GetTargetTeamEnemy() && pOwner->IsEnemy(pVisual) && (bImpact ?  true : pOwner->LooksLikeEnemy(pVisual)))
        bValidTeam = true;

    IPlayerEntity *pPlayer(pVisual->GetAsPlayerEnt());

    bool bValidType(false);
    if (m_pEntityConfig->GetTargetTypePlayer() && (pPlayer != NULL && !pPlayer->GetIsVehicle() && !pPlayer->GetIsHellbourne() && !pPlayer->GetIsSiege() && !pPlayer->IsObserver()))
        bValidType = true;
    else if (m_pEntityConfig->GetTargetTypePet() && pVisual->IsPet())
        bValidType = true;
    else if (m_pEntityConfig->GetTargetTypeNPC() && pVisual->IsNpc())
        bValidType = true;
    else if (m_pEntityConfig->GetTargetTypeVehicle() && pPlayer != NULL && pPlayer->GetIsVehicle())
        bValidType = true;
    else if (m_pEntityConfig->GetTargetTypeBuilding() && pVisual->IsBuilding())
        bValidType = true;
    else if (m_pEntityConfig->GetTargetTypeGadget() && pVisual->IsGadget())
        bValidType = true;
    else if (m_pEntityConfig->GetTargetTypeHellbourne() && pPlayer != NULL && pPlayer->GetIsHellbourne())
        bValidType = true;
    else if (m_pEntityConfig->GetTargetTypeSiege() && pPlayer != NULL && pPlayer->GetIsSiege())
        bValidType = true;

    return bValidStatus && bValidTeam && bValidType;
}


/*====================
  ISpellItem::ClientPrecache
  ====================*/
void    ISpellItem::ClientPrecache(CEntityConfig *pConfig)
{
    IInventoryItem::ClientPrecache(pConfig);

    if (!pConfig)
        return;

    if (!pConfig->GetGadgetName().empty())
        EntityRegistry.ClientPrecache(EntityRegistry.LookupID(pConfig->GetGadgetName()));

    if (!pConfig->GetTargetMaterialPath().empty())
        g_ResourceManager.Register(pConfig->GetTargetMaterialPath(), RES_MATERIAL);

    if (!pConfig->GetTargetState().empty())
        EntityRegistry.ClientPrecache(EntityRegistry.LookupID(pConfig->GetTargetState()));

    if (!pConfig->GetCastEffectPath().empty())
        g_ResourceManager.Register(pConfig->GetCastEffectPath(), RES_EFFECT);
    
    if (!pConfig->GetImpactEffectPath().empty())
        g_ResourceManager.Register(pConfig->GetImpactEffectPath(), RES_EFFECT);
}


/*====================
  ISpellItem::ServerPrecache
  ====================*/
void    ISpellItem::ServerPrecache(CEntityConfig *pConfig)
{
    IInventoryItem::ServerPrecache(pConfig);

    if (!pConfig)
        return;

    if (!pConfig->GetGadgetName().empty())
        EntityRegistry.ServerPrecache(EntityRegistry.LookupID(pConfig->GetGadgetName()));

    if (!pConfig->GetTargetState().empty())
        EntityRegistry.ServerPrecache(EntityRegistry.LookupID(pConfig->GetTargetState()));

    if (!pConfig->GetCastEffectPath().empty())
        g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetCastEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));
    
    if (!pConfig->GetImpactEffectPath().empty())
        g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetImpactEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));
}
