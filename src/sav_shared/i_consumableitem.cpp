// (C)2006 S2 Games
// i_consumableitem.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_consumableitem.h"

#include "../k2/c_effect.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
SINGLETON_INIT(CConsumableItemManager)

uint    g_uiNumConsumableItems;

CConsumableItemManager *g_pConsumableItemManager(CConsumableItemManager::GetInstance());
//=============================================================================

/*====================
  CompareItemEntries
  ====================*/
bool CompareItemEntries(CConsumableItemManager::ItemEntry A, CConsumableItemManager::ItemEntry B)
{
    return A.first->GetInteger() < B.first->GetInteger();
}


/*====================
  CConsumableItemManager::CConsumableItemManager
  ====================*/
CConsumableItemManager::CConsumableItemManager()
{
}


/*====================
  CConsumableItemManager::AddItem
  ====================*/
void    CConsumableItemManager::AddItem(const tstring &sName, ICvar *pSortVar)
{
    if (pSortVar == NULL)
        return;

    ItemList_it it(m_lItems.begin());
    while (it != m_lItems.end() && pSortVar->GetInteger() <= it->first->GetInteger())
        ++it;

    m_lItems.insert(it, ItemEntry(pSortVar, sName));
    SortItems();
}


/*====================
  CConsumableItemManager::SortItems
  ====================*/
void    CConsumableItemManager::SortItems()
{
    bool bNeedsUpdate(m_vNames.size() != m_lItems.size());
    for (ItemList_it it(m_lItems.begin()); it != m_lItems.end(); ++it)
    {
        if (it->first->IsModified())
        {
            bNeedsUpdate = true;
            it->first->SetModified(false);
        }
    }
    if (!bNeedsUpdate)
        return;

    m_lItems.sort(CompareItemEntries);

    m_vNames.clear();
    for (ItemList_it it(m_lItems.begin()); it != m_lItems.end(); ++it)
        m_vNames.push_back(it->second);
}


/*====================
  IConsumableItem::CEntityConfig::CEntityConfig
  ====================*/
IConsumableItem::CEntityConfig::CEntityConfig(const tstring &sName) :
IInventoryItem::CEntityConfig(sName),
INIT_ENTITY_CVAR(Passive, true),
INIT_ENTITY_CVAR(State, _T("")),
INIT_ENTITY_CVAR(UseEffect, _T("")),
INIT_ENTITY_CVAR(GadgetName, _T("")),
INIT_ENTITY_CVAR(UniqueCategory, _T("")),
INIT_ENTITY_CVAR(GadgetOffset,V_ZERO),
INIT_ENTITY_CVAR(MaxPerStack, 1),
INIT_ENTITY_CVAR(MaxStacks, 1),
INIT_ENTITY_CVAR(Duration, 0),
INIT_ENTITY_CVAR(AmmoMult, 1.0f),
INIT_ENTITY_CVAR(DropWeight, 1.0f),
INIT_ENTITY_CVAR(SortIndex, 0)
{
    ConsumableItemManager.AddItem(sName, &m_cvarSortIndex);
}


/*====================
  IConsumableItem::~IConsumableItem
  ====================*/
IConsumableItem::~IConsumableItem()
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (pOwner)
    {
        if (m_iStateSlot != -1 && pOwner->GetState(m_iStateSlot) != NULL && pOwner->GetState(m_iStateSlot)->GetTypeName() == GetState())
            pOwner->RemoveState(m_iStateSlot);
    }
}


/*====================
  IConsumableItem::IConsumableItem
  ====================*/
IConsumableItem::IConsumableItem(CEntityConfig *pConfig) :
IInventoryItem(pConfig),
m_pEntityConfig(pConfig),

m_iStateSlot(-1)
{
}


/*====================
  IConsumableItem::ActivatePassive
  ====================*/
bool    IConsumableItem::ActivatePassive()
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;

    // If this is a passive item, apply the state indefinitely
    if (!IsDisabled() && GetPassive() && GetState() != _T(""))
    {
        ushort unID(EntityRegistry.LookupID(GetState()));

        if (unID == INVALID_ENT_TYPE)
            return false;

        if (m_iStateSlot != -1 && pOwner->GetState(m_iStateSlot) != NULL && pOwner->GetState(m_iStateSlot)->GetType() == unID)
            pOwner->RemoveState(m_iStateSlot);

        m_iStateSlot = pOwner->ApplyState(unID, INVALID_TIME, INVALID_TIME, pOwner->GetIndex());
        return true;
    }

    return false;
}


/*====================
  IConsumableItem::ActivatePrimary
  ====================*/
bool    IConsumableItem::ActivatePrimary(int iButtonStatus)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;

    if (!GetPassive() && IsReady() && pOwner->GetAmmoCount(m_ySlot) > 0)
    {
        if (GetState() != _T(""))
            pOwner->ApplyState(EntityRegistry.LookupID(GetState()), Game.GetGameTime(), GetDuration());

        if (GetGadgetName() != _T("") && Game.IsServer())
        {
            IGameEntity *pNewEnt(Game.AllocateEntity(GetGadgetName()));
            if (pNewEnt == NULL || pNewEnt->GetAsGadget() == NULL)
                Console.Warn << _T("Failed to spawn gadget: ") << GetGadgetName() << newl;
            else
            {
                CVec3f v3Angles(pOwner->GetAngles());
                v3Angles[PITCH] = 0.0f;
                CAxis axis(v3Angles);
                CVec3f v3Offset(GetGadgetOffset());
                v3Offset[Z] = 0.0f;
                v3Offset = TransformPoint(v3Offset, axis, pOwner->GetPosition());
                v3Offset[Z] = Game.GetTerrainHeight(v3Offset[X], v3Offset[Y]) + GetGadgetOffset().z;
                pNewEnt->GetAsGadget()->SetOwner(pOwner->GetIndex());
                pNewEnt->GetAsGadget()->SetTeam(pOwner->GetTeam());
                pNewEnt->GetAsGadget()->SetPosition(v3Offset);
                pNewEnt->GetAsGadget()->SetAngles(0.0f, 0.0f, v3Angles[YAW]);
                pNewEnt->Spawn();
            }
        }

        SetCooldownTimer(Game.GetGameTime(), GetCooldownTime());
        pOwner->UseItem(m_ySlot, 1);

        // Use effect
        if (!m_pEntityConfig->GetUseEffect().empty())
        {
            CGameEvent evEffect;
            evEffect.SetSourceEntity(pOwner->GetIndex());
            evEffect.SetEffect(Game.RegisterEffect(m_pEntityConfig->GetUseEffect()));
            Game.AddEvent(evEffect);
        }

        return true;
    }

    return false;
}


/*====================
  IConsumableItem::ClientPrecache
  ====================*/
void    IConsumableItem::ClientPrecache(CEntityConfig *pConfig)
{
    IInventoryItem::ClientPrecache(pConfig);

    if (!pConfig)
        return;

    if (!pConfig->GetUseEffect().empty())
        g_ResourceManager.Register(pConfig->GetUseEffect(), RES_EFFECT);    

    if (!pConfig->GetGadgetName().empty())
        EntityRegistry.ClientPrecache(EntityRegistry.LookupID(pConfig->GetGadgetName()));

    if (!pConfig->GetState().empty())
        EntityRegistry.ClientPrecache(EntityRegistry.LookupID(pConfig->GetState()));
}


/*====================
  IConsumableItem::ServerPrecache
  ====================*/
void    IConsumableItem::ServerPrecache(CEntityConfig *pConfig)
{
    IInventoryItem::ServerPrecache(pConfig);

    if (!pConfig)
        return;

    if (!pConfig->GetUseEffect().empty())
        g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetUseEffect(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));

    if (!pConfig->GetGadgetName().empty())
        EntityRegistry.ServerPrecache(EntityRegistry.LookupID(pConfig->GetGadgetName()));

    if (!pConfig->GetState().empty())
        EntityRegistry.ServerPrecache(EntityRegistry.LookupID(pConfig->GetState()));
}
