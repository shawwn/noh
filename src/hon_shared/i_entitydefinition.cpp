// (C)2008 S2 Games
// i_entitydefinition.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "hon_shared_common.h"

#include "i_entitydefinition.h"
#include "i_unitentity.h"
#include "i_gadgetentity.h"
#include "i_projectile.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_DEFINITION_TYPE_INFO(IEntityDefinition, ENTITY_BASE_TYPE_ENTITY, Entity)
//=============================================================================

/*====================
  GetActionScriptName
  ====================*/
tstring GetActionScriptName(uint uiActionScript)
{
    switch (uiActionScript)
    {
    case ACTION_SCRIPT_FRAME:
        return _CTS("FRAME");
    case ACTION_SCRIPT_FRAME_IMPACT:
        return _CTS("FRAME_IMPACT");
    case ACTION_SCRIPT_INTERVAL:
        return _CTS("INTERVAL");
    case ACTION_SCRIPT_START:
        return _CTS("START");
    case ACTION_SCRIPT_PRE_COST:
        return _CTS("PRE_COST");
    case ACTION_SCRIPT_ACTION:
        return _CTS("ACTION");
    case ACTION_SCRIPT_PRE_IMPACT:
        return _CTS("PRE_IMPACT");
    case ACTION_SCRIPT_PRE_DAMAGE:
        return _CTS("PRE_DAMAGE");
    case ACTION_SCRIPT_DAMAGE_EVENT:
        return _CTS("DAMAGE_EVENT");
    case ACTION_SCRIPT_IMPACT:
        return _CTS("IMPACT");
    case ACTION_SCRIPT_IMPACT_INVALID:
        return _CTS("IMPACT_INVALID");
    case ACTION_SCRIPT_COMPLETE:
        return _CTS("COMPLETE");
    case ACTION_SCRIPT_CANCEL:
        return _CTS("CANCEL");
    case ACTION_SCRIPT_ACTIVATE_START:
        return _CTS("ACTIVATE_START");
    case ACTION_SCRIPT_ACTIVATE_PRE_IMPACT:
        return _CTS("ACTIVATE_PRE_IMPACT");
    case ACTION_SCRIPT_ACTIVATE_IMPACT:
        return _CTS("ACTIVATE_IMPACT");
    case ACTION_SCRIPT_ACTIVATE_END:
        return _CTS("ACTIVATE_END");
    case ACTION_SCRIPT_ABILITY_START:
        return _CTS("ABILITY_START");
    case ACTION_SCRIPT_ABILITY_IMPACT:
        return _CTS("ABILITY_IMPACT");
    case ACTION_SCRIPT_ABILITY_FINISH:
        return _CTS("ABILITY_FINISH");
    case ACTION_SCRIPT_ABILITY_END:
        return _CTS("ABILITY_END");
    case ACTION_SCRIPT_TOGGLE_ON:
        return _CTS("TOGGLE_ON");
    case ACTION_SCRIPT_TOGGLE_OFF:
        return _CTS("TOGGLE_OFF");

    case ACTION_SCRIPT_CHANNEL_START:
        return _CTS("CHANNEL_START");
    case ACTION_SCRIPT_CHANNEL_FRAME:
        return _CTS("CHANNEL_FRAME");
    case ACTION_SCRIPT_CHANNEL_BROKEN:
        return _CTS("CHANNEL_BROKEN");
    case ACTION_SCRIPT_CHANNEL_END:
        return _CTS("CHANNEL_END");

    case ACTION_SCRIPT_CHANNELING_START:
        return _CTS("CHANNELING_START");
    case ACTION_SCRIPT_CHANNELING_FRAME:
        return _CTS("CHANNELING_FRAME");
    case ACTION_SCRIPT_CHANNELING_BROKEN:
        return _CTS("CHANNELING_BROKEN");
    case ACTION_SCRIPT_CHANNELING_END:
        return _CTS("CHANNELING_END");

    case ACTION_SCRIPT_ATTACK_START:
        return _CTS("ATTACK_START");
    case ACTION_SCRIPT_ATTACK:
        return _CTS("ATTACK");
    case ACTION_SCRIPT_ATTACK_PRE_IMPACT:
        return _CTS("ATTACK_PRE_IMPACT");
    case ACTION_SCRIPT_ATTACK_PRE_DAMAGE:
        return _CTS("ATTACK_PRE_DAMAGE");
    case ACTION_SCRIPT_ATTACK_DAMAGE_EVENT:
        return _CTS("ATTACK_DAMAGE_EVENT");
    case ACTION_SCRIPT_ATTACK_IMPACT:
        return _CTS("ATTACK_IMPACT");
    case ACTION_SCRIPT_ATTACK_END:
        return _CTS("ATTACK_END");
    case ACTION_SCRIPT_ATTACKED_START:
        return _CTS("ATTACKED_START");
    case ACTION_SCRIPT_ATTACKED_PRE_IMPACT:
        return _CTS("ATTACKED_PRE_IMPACT");
    case ACTION_SCRIPT_ATTACKED_PRE_DAMAGE:
        return _CTS("ATTACKED_PRE_DAMAGE");
    case ACTION_SCRIPT_ATTACKED_DAMAGE_EVENT:
        return _CTS("ATTACKED_DAMAGE_EVENT");
    case ACTION_SCRIPT_ATTACKED_POST_IMPACT:
        return _CTS("ATTACKED_POST_IMPACT");
    case ACTION_SCRIPT_DAMAGE:
        return _CTS("DAMAGE");
    case ACTION_SCRIPT_DAMAGED:
        return _CTS("DAMAGED");
    case ACTION_SCRIPT_STUNNED:
        return _CTS("STUNNED");
    case ACTION_SCRIPT_KILLED:
        return _CTS("KILLED");
    case ACTION_SCRIPT_EXPIRED:
        return _CTS("EXPIRED");
    case ACTION_SCRIPT_DEATH:
        return _CTS("DEATH");
    case ACTION_SCRIPT_KILL:
        return _CTS("KILL");
    case ACTION_SCRIPT_INDIRECT_KILL:
        return _CTS("INDIRECT_KILL");
    case ACTION_SCRIPT_ASSIST:
        return _CTS("ASSIST");
    case ACTION_SCRIPT_SPAWN:
        return _CTS("SPAWN");
    case ACTION_SCRIPT_RESPAWN:
        return _CTS("RESPAWN");
    case ACTION_SCRIPT_INFLICT:
        return _CTS("INFLICT");
    case ACTION_SCRIPT_REFRESH:
        return _CTS("REFRESH");
    case ACTION_SCRIPT_INFLICTED:
        return _CTS("INFLICTED");
    case ACTION_SCRIPT_OWNER_RESPAWN:
        return _CTS("OWNER_RESPAWN");
    case ACTION_SCRIPT_RELEASE:
        return _CTS("RELEASE");
    case ACTION_SCRIPT_TOUCH:
        return _CTS("TOUCH");
    case ACTION_SCRIPT_TOUCHED:
        return _CTS("TOUCHED");
    case ACTION_SCRIPT_THINK:
        return _CTS("THINK");
    case ACTION_SCRIPT_TARGET_ACQUIRED:
        return _CTS("TARGET_ACQUIRED");
    case ACTION_SCRIPT_LEARN:
        return _CTS("LEARN");
    case ACTION_SCRIPT_UPGRADE:
        return _CTS("UPGRADE");
    case ACTION_SCRIPT_ACTIVATE_COST:
        return _CTS("ACTIVATE_COST");
    }

    return TSNULL;
}


/*====================
  IEntityDefinition::IEntityDefinition
  ====================*/
IEntityDefinition::IEntityDefinition(IBaseEntityAllocator *pAllocator) :
m_bPrecaching(false),
m_bPostProcessing(false),
m_unTypeID(INVALID_ENT_TYPE),
m_unModifierBits(0),
m_pAllocator(pAllocator),
m_unModifierMask(0),
m_uiModifierCount(0),

m_uiModifierID(INVALID_INDEX),
m_iPriority(0),
m_bExclusive(false),
m_bHasBaseModifier(false),
m_bAltAvatar(false)
{
    m_vActionScripts.resize(NUM_ACTION_SCRIPTS, nullptr);
}


/*====================
  IEntityDefinition::SetName
  ====================*/
void    IEntityDefinition::SetName(const tstring &sName)
{
    m_sName = sName;
    for (EntityModifierMap::iterator it(m_mapModifiers.begin()); it != m_mapModifiers.end(); ++it)
        it->second->SetName(sName);
}


/*====================
  IEntityDefinition::SetTypeID
  ====================*/
void    IEntityDefinition::SetTypeID(ushort unTypeID)
{
    m_unTypeID = unTypeID;
    for (EntityModifierMap::iterator it(m_mapModifiers.begin()); it != m_mapModifiers.end(); ++it)
        it->second->SetTypeID(unTypeID);
}


/*====================
  IEntityDefinition::NewActionScript
  ====================*/
CCombatActionScript*        IEntityDefinition::NewActionScript(EEntityActionScript eScript, int iPriority, bool bPropagateToIllusions, bool bActivateOnBounces)
{
    CCombatActionScript *pNewScript(K2_NEW(ctx_Game,   CCombatActionScript)(eScript, iPriority, bPropagateToIllusions, bActivateOnBounces, m_uiModifierID));
    if (pNewScript == nullptr)
        return nullptr;

    m_vActionScripts[eScript] = pNewScript;
    return pNewScript;
}


/*====================
  IEntityDefinition::ExecuteActionScript
  ====================*/
float   IEntityDefinition::ExecuteActionScript(
    EEntityActionScript eScript,
    IGameEntity *pEntity,
    IGameEntity *pInitiator,
    IGameEntity *pInflictor,
    IGameEntity *pTarget,
    const CVec3f &v3Target,
    IGameEntity *pProxy,
    uint uiLevel,
    CCombatEvent *pCombatEvent,
    CDamageEvent *pDamageEvent,
    const CVec3f &v3Delta,
    float fDefault)
{
    PROFILE("IEntityDefinition::ExecuteActionScript");

    if (pEntity != nullptr)
    {
        IEntityDefinition *pModifier(GetModifiedDefinition(pEntity->GetModifierBits()));
        if (pModifier != nullptr && pModifier != this)
        {
            return pModifier->ExecuteActionScript(eScript, pEntity, pInitiator, pInflictor, pTarget, v3Target, pProxy, uiLevel);
        }
    }

    CCombatActionScript *pScript(GetActionScript(eScript));
    if (pScript == nullptr)
        return fDefault;

    if (pEntity != nullptr && !pScript->GetPropagateToIllusions())
    {
        IGameEntity *pCheck(pEntity);

        while (pCheck != nullptr)
        {
            if (pCheck->IsUnit() && pCheck->GetAsUnit()->IsIllusion())
                return fDefault;

            pCheck = pCheck->GetOwner();
        }
    }

    pScript->SetThisUID(pEntity->GetUniqueID());
    pScript->SetLevel(uiLevel);
    return pScript->Execute(pInflictor, pInitiator, pTarget, v3Target, pProxy, pCombatEvent, pDamageEvent, nullptr, v3Delta, fDefault);
}


/*====================
  IEntityDefinition::GetActionScript
  ====================*/
CCombatActionScript*    IEntityDefinition::GetActionScript(EEntityActionScript eScript, ushort unModifierBits)
{
    IEntityDefinition *pModifier(GetModifiedDefinition(unModifierBits));
    if (pModifier != nullptr && pModifier != this)
        return pModifier->GetActionScript(eScript);
    else
        return GetActionScript(eScript);
}


/*====================
  IEntityDefinition::GetEffectDescription
  ====================*/
const tstring&  IEntityDefinition::GetEffectDescription(EEntityActionScript eScript)
{
    if (m_vActionScripts[eScript] == nullptr)
        return TSNULL;

    return m_vActionScripts[eScript]->GetEffectDescription();
}

const tstring&  IEntityDefinition::GetEffectDescription(EEntityActionScript eScript, ushort unModifierBits)
{   
    IEntityDefinition *pModifier(GetModifiedDefinition(unModifierBits));
    if (pModifier != nullptr && pModifier != this)
        return pModifier->GetEffectDescription(eScript);
    else
        return GetEffectDescription(eScript);
}


/*====================
  IEntityDefinition::GetEffectDescriptionIndex
  ====================*/
uint    IEntityDefinition::GetEffectDescriptionIndex(EEntityActionScript eScript)
{
    if (m_vActionScripts[eScript] == nullptr)
        return INVALID_INDEX;

    return m_vActionScripts[eScript]->GetEffectDescriptionIndex();
}


/*====================
  IEntityDefinition::GetActionScriptPriority
  ====================*/
int     IEntityDefinition::GetActionScriptPriority(EEntityActionScript eScript)
{
    if (m_vActionScripts[eScript] == nullptr)
        return 0;

    return m_vActionScripts[eScript]->GetScriptPriority();
}

int     IEntityDefinition::GetActionScriptPriority(EEntityActionScript eScript, ushort unModifierBits)
{
    IEntityDefinition *pModifier(GetModifiedDefinition(unModifierBits));
    if (pModifier != nullptr && pModifier != this)
        return pModifier->GetActionScriptPriority(eScript);
    else
        return GetActionScriptPriority(eScript);
}


/*====================
  IEntityDefinition::ApplyAuras
  ====================*/
void    IEntityDefinition::ApplyAuras(IGameEntity *pSource, uint uiLevel)
{
    PROFILE("IEntityDefinition::ApplyAuras");

    if (pSource == nullptr)
        return;

#if 0
    // Check modifiers
    IEntityDefinition *pModifier(GetModifiedDefinition(pSource->GetModifierBits()));
    if (pModifier != nullptr && pModifier != this)
    {
        pModifier->ApplyAuras(pSource, uiLevel);
        return;
    }
#endif

    if (m_vAuras.empty())
        return;

    // Get owner
    IUnitEntity *pOwner(pSource->GetAsUnit());
    if (pOwner == nullptr)
    {
        if (pSource->GetAsSlave() == nullptr)
            return;

        pOwner = pSource->GetAsSlave()->GetOwner();
        if (pOwner == nullptr)
            return;
    }

    for (AuraList_it itAura(m_vAuras.begin()); itAura != m_vAuras.end(); ++itAura)
    {
        if (!itAura->IsValid())
            continue;

        bool bAuraActive(false);

        itAura->FetchWorkingValues(pSource, uiLevel);

        float fRadius(itAura->GetRadius(uiLevel));

        if (!itAura->CanPropagate(pOwner) || fRadius <= 0.0f)
        {
            // Check target scheme and condition
            if (!itAura->CanApply(pOwner, pOwner))
                continue;

            // Apply state
            if (itAura->ApplyState(pOwner, pOwner, uiLevel, pSource))
                bAuraActive = true;

            // Bind gadgets
            if (itAura->BindGadget(pOwner, pOwner, uiLevel, pSource))
                bAuraActive = true;

            // Apply reflexive state
            if (bAuraActive)
                itAura->ApplyReflexiveState(pOwner, pOwner, uiLevel, pSource);
        }
        else if (fRadius >= 9999.0f)
        {
            const UnitList &lUnits(Game.GetUnitList());
            for (UnitList_cit itEntity(lUnits.begin()), itEntityEnd(lUnits.end()); itEntity != itEntityEnd; ++itEntity)
            {
                // Check target scheme and condition
                if (!itAura->CanApply(pOwner, *itEntity))
                    continue;

                // Apply state
                if (itAura->ApplyState(pOwner, *itEntity, uiLevel, pSource))
                    bAuraActive = true;

                // Bind gadgets
                if (itAura->BindGadget(pOwner, *itEntity, uiLevel, pSource))
                    bAuraActive = true;
            }

            // Apply reflexive state
            if (bAuraActive)
                itAura->ApplyReflexiveState(pOwner, pOwner, uiLevel, pSource);
        }
        else
        {
            static uivector vEntities;
            vEntities.clear();

            Game.GetEntitiesInRadius(vEntities, pOwner->GetPosition().xy(), fRadius, REGION_UNIT);

            for (uivector_it itEntity(vEntities.begin()), itEntityEnd(vEntities.end()); itEntity != itEntityEnd; ++itEntity)
            {
                IUnitEntity *pUnit(Game.GetUnitEntity(Game.GetGameIndexFromWorldIndex(*itEntity)));

                // Check target scheme and condition
                if (!itAura->CanApply(pOwner, pUnit))
                    continue;

                // Apply state
                if (itAura->ApplyState(pOwner, pUnit, uiLevel, pSource))
                    bAuraActive = true;

                // Bind gadgets
                if (itAura->BindGadget(pOwner, pUnit, uiLevel, pSource))
                    bAuraActive = true;
            }

            // Apply reflexive state
            if (bAuraActive)
                itAura->ApplyReflexiveState(pOwner, pOwner, uiLevel, pSource);
        }
    }
}


/*====================
  IEntityDefinition::PrecacheV
  ====================*/
void    IEntityDefinition::PrecacheV(EPrecacheScheme eScheme, const tstring &sModifier)
{
    PRECACHE_GUARD

    // Script actions (cast projectiles, affectors, summoned units, etc)
    for (uint uiScript(0); uiScript < NUM_ACTION_SCRIPTS; ++uiScript)
    {
        CCombatActionScript *pScript(m_vActionScripts[uiScript]);
        if (pScript == nullptr || pScript->GetModifierID() != m_uiModifierID)
            continue;

        pScript->Precache(eScheme, sModifier);
    }

    // Aura states
    for (AuraList_it it(m_vAuras.begin()); it != m_vAuras.end(); ++it)
        it->Precache(eScheme, sModifier);

    for (EntityModifierMap::iterator it(m_mapModifiers.begin()); it != m_mapModifiers.end(); ++it)
    {
        if (it->second->GetAltAvatar() && TStringCompare(sModifier, _T("All")) != 0)
            continue;

        it->second->Precache(eScheme, sModifier);
    }

    PRECACHE_GUARD_END
}


/*====================
  IEntityDefinition::GetPrecacheListV
  ====================*/
void    IEntityDefinition::GetPrecacheListV(EPrecacheScheme eScheme, const tstring &sModifier, HeroPrecacheList &deqPrecache)
{
    PRECACHE_GUARD
        // Script actions (cast projectiles, affectors, summoned units, etc)
        for (uint uiScript(0); uiScript < NUM_ACTION_SCRIPTS; ++uiScript)
        {
            CCombatActionScript *pScript(m_vActionScripts[uiScript]);
            if (pScript == nullptr || pScript->GetModifierID() != m_uiModifierID)
                continue;

            pScript->GetPrecacheList(eScheme, sModifier, deqPrecache);
        }

        // Aura states
        for (AuraList_it it(m_vAuras.begin()); it != m_vAuras.end(); ++it)
            it->GetPrecacheList(eScheme, sModifier, deqPrecache);

        for (EntityModifierMap::iterator it(m_mapModifiers.begin()); it != m_mapModifiers.end(); ++it)
        {
            if (it->second->GetAltAvatar() && TStringCompare(sModifier, _T("All")) != 0)
                continue;

            it->second->GetPrecacheList(eScheme, sModifier, deqPrecache);
        }
    PRECACHE_GUARD_END
}


/*====================
  IEntityDefinition::Precache
  ====================*/
void    IEntityDefinition::Precache(EPrecacheScheme eScheme, const tstring &sModifier)
{
    if (!m_bPrecaching)
    {
        if (!sModifier.empty() && TStringCompare(sModifier, _T("All")) != 0)
        {
            uint uiModifierID(EntityRegistry.LookupModifierKey(sModifier));

            IEntityDefinition *pModifiedDefinition(GetModifiedDefinition(GetModifierBit(uiModifierID)));

            if (pModifiedDefinition != nullptr && pModifiedDefinition != this && pModifiedDefinition->GetAltAvatar())
            {
                pModifiedDefinition->Precache(eScheme, sModifier);
                return;
            }
        }
    }
    else
    {
        return;
    }

    PrecacheV(eScheme, sModifier);
}


/*====================
  IEntityDefinition::GetPrecacheList
  ====================*/
void    IEntityDefinition::GetPrecacheList(EPrecacheScheme eScheme, const tstring &sModifier, HeroPrecacheList &deqPrecache)
{
    if (!m_bPrecaching)
    {
        if (!sModifier.empty() && TStringCompare(sModifier, _T("All")) != 0)
        {
            uint uiModifierID(EntityRegistry.LookupModifierKey(sModifier));

            IEntityDefinition *pModifiedDefinition(GetModifiedDefinition(GetModifierBit(uiModifierID)));

            if (pModifiedDefinition != nullptr && pModifiedDefinition != this && pModifiedDefinition->GetAltAvatar())
            {
                pModifiedDefinition->GetPrecacheList(eScheme, sModifier, deqPrecache);
                return;
            }
        }
    }
    else
    {
        return;
    }

    GetPrecacheListV(eScheme, sModifier, deqPrecache);

    deqPrecache.push_back(SHeroPrecache(GetName(), eScheme, sModifier));
}


/*====================
  IEntityDefinition::PostProcess

  TODO: Use this function to convert entity name references to integer indexes
  ====================*/
void    IEntityDefinition::PostProcess()
{
    if (m_bPostProcessing)
        return;

    m_bPostProcessing = true;

    // Script actions (cast projectiles, affectors, summoned units, etc)
    for (uint uiScript(0); uiScript < NUM_ACTION_SCRIPTS; ++uiScript)
    {
        CCombatActionScript *pScript(m_vActionScripts[uiScript]);
        if (pScript == nullptr)
            continue;

        pScript->FetchEffectDescription(m_sName);
    }

    for (EntityModifierMap::iterator it(m_mapModifiers.begin()); it != m_mapModifiers.end(); ++it)
        it->second->PostProcess();

    m_bPostProcessing = false;
}


/*====================
  IEntityDefinition::AddModifier

  This supports blank modifier keys which are only active
  if no other modifiers in the group are active.
  ====================*/
void    IEntityDefinition::AddModifier(const tstring &sModifierKey, IEntityDefinition *pDef)
{
    // Register this key, returns existing bit flag if it already exists
    uint uiModifierID(EntityRegistry.RegisterModifier(sModifierKey));

    ushort unModBits(RegisterModifier(uiModifierID));
    m_unModifierMask |= unModBits;

    pDef->SetModifierID(uiModifierID);

    // Clear the definitions modifier map
    pDef->m_unModifierMask = 0;
    pDef->m_uiModifierCount = 0;
    pDef->m_mapModifiers.clear();
    pDef->m_mapModifierIDs.clear();
    pDef->m_unModifierBits = unModBits;

    // Add the new definition to the modifier map
    m_mapModifiers[unModBits] = pDef;

    if (unModBits == 0)
        m_bHasBaseModifier = true;
}


/*====================
  IEntityDefinition::ImportDefinition
  ====================*/
void    IEntityDefinition::ImportDefinition(IEntityDefinition *pDefinition)
{
    for (uint uiScript(0); uiScript < NUM_ACTION_SCRIPTS; ++uiScript)
    {
        if (pDefinition->m_vActionScripts[uiScript] == nullptr)
            continue;

        if (m_vActionScripts[uiScript] != nullptr && GetPriority() > pDefinition->GetPriority())
            continue;

        m_vActionScripts[uiScript] = pDefinition->m_vActionScripts[uiScript];
    }

    const AuraList &cAuras(pDefinition->GetAuraList());
    for (AuraList_cit citAura(cAuras.begin()); citAura != cAuras.end(); ++citAura)
        m_vAuras.push_back(*citAura);
}


/*====================
  IEntityDefinition::GenerateMergedModifier
  ====================*/
IEntityDefinition*  IEntityDefinition::GenerateMergedModifier(ushort unModifierBits)
{
    if (unModifierBits == 0)
        return nullptr;

    IEntityDefinition *pMergedDef(GetCopy());

    // Clear the definitions modifier map
    pMergedDef->m_unModifierMask = 0;
    pMergedDef->m_uiModifierCount = 0;
    pMergedDef->m_mapModifiers.clear();
    pMergedDef->m_mapModifierIDs.clear();
    pMergedDef->m_unModifierBits = unModifierBits;

    for (ushort un(1); un != 0; un <<= 1)
    {
        if ((un & unModifierBits) == 0)
            continue;

        EntityModifierMap::iterator itModifier(m_mapModifiers.find(un));
        if (itModifier == m_mapModifiers.end())
            continue;

        pMergedDef->ImportDefinition(itModifier->second);
    }
    
    m_mapModifiers[unModifierBits] = pMergedDef;
    return pMergedDef;
}


/*====================
  IEntityDefinition::GetModifiedDefinition
  ====================*/
IEntityDefinition*  IEntityDefinition::GetModifiedDefinition(ushort unModifierBits)
{
    if (unModifierBits == m_unModifierBits && !(m_unModifierBits == 0 && m_bHasBaseModifier))
        return this;

    unModifierBits &= m_unModifierMask;

    EntityModifierMap::iterator itFind(m_mapModifiers.find(unModifierBits));
    if (itFind == m_mapModifiers.end())
        return GenerateMergedModifier(unModifierBits);

    return itFind->second;
}


/*====================
  IEntityDefinition::GetModifiedDefinition
  ====================*/
IEntityDefinition*  IEntityDefinition::GetModifiedDefinition(const uivector &vModifiers)
{
    return GetModifiedDefinition(GetModifierBits(vModifiers));
}


/*====================
  IEntityDefinition::RegisterModifier
  ====================*/
ushort  IEntityDefinition::RegisterModifier(uint uiModifierID)
{
#if TKTK // Why does this assert limit the count to 16?
    assert(m_uiModifierCount < sizeof(ushort) * 8);
#endif
    

    if (uiModifierID == INVALID_INDEX || uiModifierID == 0)
        return 0;

    map<uint, ushort>::iterator itFind(m_mapModifierIDs.find(uiModifierID));
    if (itFind != m_mapModifierIDs.end())
        return itFind->second;

    ushort unModifierBits(1 << m_uiModifierCount);
    m_mapModifierIDs[uiModifierID] = unModifierBits;
    ++m_uiModifierCount;
    return unModifierBits;
}


/*====================
  IEntityDefinition::GetModifierBit
  ====================*/
ushort  IEntityDefinition::GetModifierBit(uint uiModifierID)
{
    if (uiModifierID == INVALID_INDEX || uiModifierID == 0)
        return 0;

    map<uint, ushort>::iterator itFind(m_mapModifierIDs.find(uiModifierID));
    if (itFind != m_mapModifierIDs.end())
        return itFind->second;

    return 0;
}


/*====================
  IEntityDefinition::GetModifierBits
  ====================*/
ushort  IEntityDefinition::GetModifierBits(const uivector &vModifiers)
{
    ushort unModifierBits(0);

    for (uivector_cit cit(vModifiers.begin()), citEnd(vModifiers.end()); cit != citEnd; ++cit)
        unModifierBits |= GetModifierBit(*cit);

    return unModifierBits;
}

