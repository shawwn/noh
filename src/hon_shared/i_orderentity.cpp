// (C)2009 S2 Games
// i_orderentity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "hon_shared_common.h"

#include "i_orderentity.h"

#include "../k2/c_scenemanager.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
uint                IOrderEntity::s_uiBaseType(ENTITY_BASE_TYPE_ORDER);

DEFINE_ENTITY_DESC(IOrderEntity, 1)
{
    s_cDesc.pFieldTypes = K2_NEW(ctx_Game,   TypeVector)();
}
//=============================================================================

/*====================
  IOrderEntity::IOrderEntity
  ====================*/
IOrderEntity::IOrderEntity() :
IGameEntity(nullptr),
m_uiLevel(1),
m_uiOwnerIndex(INVALID_INDEX),
m_fParam(0.0f),
m_bComplete(false),
m_bCancel(false)
{
    for (int i(0); i < 4; ++i)
        m_auiProxyUID[i] = INVALID_INDEX;
}


/*====================
  IOrderEntity::Baseline
  ====================*/
void    IOrderEntity::Baseline()
{
}


/*====================
  IOrderEntity::GetSnapshot
  ====================*/
void    IOrderEntity::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
}


/*====================
  IOrderEntity::ReadSnapshot
  ====================*/
bool    IOrderEntity::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
    try
    {
        Validate();

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("IVisualEntity::ReadSnapshot() - "), NO_THROW);
        return false;
    }

    return true;
}


/*====================
  IOrderEntity::ClientPrecache
  ====================*/
void    IOrderEntity::ClientPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier)
{
    IGameEntity::ClientPrecache(pConfig, eScheme, sModifier);
}


/*====================
  IOrderEntity::ServerPrecache
  ====================*/
void    IOrderEntity::ServerPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier)
{
    IGameEntity::ServerPrecache(pConfig, eScheme, sModifier);
}


/*====================
  IOrderEntity::Spawn
  ====================*/
void    IOrderEntity::Spawn()
{
    IGameEntity::Spawn();

#if 0
    if (Game.IsServer())
    {
        COrderDefinition *pDefinition(GetDefinition<CAffectorDefinition>(GetModifierBits()));
        if (pDefinition != nullptr)
            pDefinition->ExecuteActionScript(ACTION_SCRIPT_SPAWN, this, GetOwner(), this, nullptr, GetPosition(), GetProxy(0), GetLevel());
    }
#endif
}


/*====================
  IOrderEntity::ServerFrameSetup
  ====================*/
bool    IOrderEntity::ServerFrameSetup()
{
    return IGameEntity::ServerFrameSetup();
}


/*====================
  IOrderEntity::ServerFrameMovement
  ====================*/
bool    IOrderEntity::ServerFrameMovement()
{
    return IGameEntity::ServerFrameMovement();
}


/*====================
  IOrderEntity::ServerFrameAction
  ====================*/
bool    IOrderEntity::ServerFrameAction()
{
    return IGameEntity::ServerFrameAction();
}


/*====================
  IOrderEntity::ServerFrameCleanup
  ====================*/
bool    IOrderEntity::ServerFrameCleanup()
{
    return IGameEntity::ServerFrameCleanup();
}


/*====================
  IOrderEntity::ExecuteActionScript
  ====================*/
void    IOrderEntity::ExecuteActionScript(EEntityActionScript eScript, IUnitEntity *pTarget, const CVec3f &v3Target)
{
    COrderDefinition *pDefinition(GetDefinition<COrderDefinition>(GetModifierBits()));
    if (pDefinition == nullptr)
        return;

    pDefinition->ExecuteActionScript(eScript, this, GetOwner(), this, pTarget, v3Target, GetProxy(0), GetLevel());
}


/*====================
  IOrderEntity::UpdateModifiers
  ====================*/
void    IOrderEntity::UpdateModifiers(const uivector &vModifiers)
{
    m_vModifierKeys = vModifiers;

    uint uiModifierBits(0);
    if (m_uiActiveModifierKey != INVALID_INDEX)
        uiModifierBits |= GetModifierBit(m_uiActiveModifierKey);

    SetModifierBits(uiModifierBits | GetModifierBits(vModifiers));

    // Activate conditional modifiers
    IUnitEntity *pOwner(GetOwner());
    if (pOwner == nullptr)
        return;

    CEntityDefinitionResource *pResource(g_ResourceManager.Get<CEntityDefinitionResource>(m_hDefinition));
    if (pResource == nullptr)
        return;
    IEntityDefinition *pDefinition(pResource->GetDefinition<IEntityDefinition>());
    if (pDefinition == nullptr)
        return;

    const EntityModifierMap &mapModifiers(pDefinition->GetModifiers());
    for (EntityModifierMap::const_iterator cit(mapModifiers.begin()), citEnd(mapModifiers.end()); cit != citEnd; ++cit)
    {
        const tstring &sCondition(cit->second->GetCondition());
        if (sCondition.empty())
            continue;

        tsvector vsTypes(TokenizeString(sCondition, _T(' ')));

        tsvector_cit itType(vsTypes.begin()), itTypeEnd(vsTypes.end());
        for (; itType != itTypeEnd; ++itType)
        {
            if (!itType->empty() && (*itType)[0] == _T('!'))
            {
                if (pOwner->IsTargetType(itType->substr(1), pOwner))
                    break;
            }
            else
            {
                if (!pOwner->IsTargetType(*itType, pOwner))
                    break;
            }
        }
        if (itType == itTypeEnd)
            SetModifierBits(GetModifierBits() | cit->first);
    }
}
