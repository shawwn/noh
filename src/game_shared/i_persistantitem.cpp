// (C)2006 S2 Games
// i_persistantitem.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_persistantitem.h"
#include "c_statepersistantitem.h"
#include "c_statepersistantreplenish.h"

#include "../k2/c_texture.h"
#include "../k2/c_effect.h"
#include "../k2/i_widget.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
vector<SDataField>* IPersistantItem::s_pvFields;

CPersistantItemsConfig  g_PersistantItemsConfig;
//=============================================================================

/*====================
  CPersistantItemsConfig::~CPersistantItemsConfig
  ====================*/
CPersistantItemsConfig::~CPersistantItemsConfig()
{
    SAFE_DELETE(m_pCvarDefaultIcon);

    // Regen
    for (int i(0); i < NUM_PERSISTANT_REGEN_MODS; ++i)
    {
        SAFE_DELETE(m_pCvarRegenName[i]);
        SAFE_DELETE(m_pCvarRegenMod[i]);
        SAFE_DELETE(m_pCvarRegenDropWeight[i]);
        SAFE_DELETE(m_pCvarRegenIconPath[i]);
    }

    // Type
    for (int i(0); i < NUM_PERSISTANT_ITEM_TYPES; ++i)
    {
        SAFE_DELETE(m_pCvarTypeName[i]);
        SAFE_DELETE(m_pCvarTypeMultiplier[i]);
        SAFE_DELETE(m_pCvarTypeDropWeight[i]);
        SAFE_DELETE(m_pCvarTypeIconPath[i]);
    }

    // Increase
    for (int i(0); i < NUM_PERSISTANT_INCREASE_MODS; ++i)
    {
        SAFE_DELETE(m_pCvarIncreaseName[i]);
        SAFE_DELETE(m_pCvarIncreaseMod[i]);
        SAFE_DELETE(m_pCvarIncreaseDropWeight[i]);
        SAFE_DELETE(m_pCvarIncreaseIconPath[i]);
    }

    // Replenish
    for (int i(0); i < NUM_PERSISTANT_REPLENISH_MODS; ++i)
    {
        SAFE_DELETE(m_pCvarReplenishName[i]);
        SAFE_DELETE(m_pCvarReplenishMod[i]);
        SAFE_DELETE(m_pCvarReplenishDropWeight[i]);
        SAFE_DELETE(m_pCvarReplenishIconPath[i]);
        SAFE_DELETE(m_pCvarReplenishUseEffect[i]);
        SAFE_DELETE(m_pCvarReplenishDuration[i]);
        SAFE_DELETE(m_pCvarReplenishCooldown[i]);
        SAFE_DELETE(m_pCvarReplenishCooldownOnDamage[i]);
    }
}


/*====================
  CPersistantItemsConfig::CPersistantItemsConfig
  ====================*/
CPersistantItemsConfig::CPersistantItemsConfig()
{
    m_pCvarDefaultIcon = ICvar::CreateString(_T("Persistant_Item_DefaultIcon"), _T("/ui/elements/vault2.tga"), CVAR_GAMECONFIG | CVAR_TRANSMIT);

    // Regen
    for (int i(0); i < NUM_PERSISTANT_REGEN_MODS; ++i)
    {
        if (g_sPersistantRegenModifierCvars[i].empty())
        {
            m_pCvarRegenName[i] = NULL;
            m_pCvarRegenMod[i] = NULL;
            m_pCvarRegenDropWeight[i] = NULL;
            m_pCvarRegenIconPath[i] = NULL;
        }
        else
        {
            m_pCvarRegenName[i] = ICvar::CreateString(_T("Persistant_Regen_") + g_sPersistantRegenModifierCvars[i] + _T("_Name"), g_sPersistantRegenColors[i], CVAR_GAMECONFIG | CVAR_TRANSMIT);
            m_pCvarRegenMod[i] = ICvar::CreateString(_T("Persistant_Regen_") + g_sPersistantRegenModifierCvars[i] + _T("_Modifier"), g_sPersistantRegenModifiers[i], CVAR_GAMECONFIG | CVAR_TRANSMIT);
            m_pCvarRegenDropWeight[i] = ICvar::CreateFloat(_T("Persistant_Regen_") + g_sPersistantRegenModifierCvars[i] + _T("_DropWeight"), 1.0f, CVAR_GAMECONFIG | CVAR_TRANSMIT);
            m_pCvarRegenIconPath[i] = ICvar::CreateString(_T("Persistant_Regen_") + g_sPersistantRegenModifierCvars[i] + _T("_IconPath"), _T("/items/persistant/bg_") + LowerString(g_sPersistantRegenColors[i]) + _T(".tga"), CVAR_GAMECONFIG | CVAR_TRANSMIT);
        }
    }

    // Type
    for (int i(0); i < NUM_PERSISTANT_ITEM_TYPES; ++i)
    {
        if (g_sPersistantItemTypeCvars[i].empty())
        {
            m_pCvarTypeName[i] = NULL;
            m_pCvarTypeMultiplier[i] = NULL;
            m_pCvarTypeDropWeight[i] = NULL;
            m_pCvarTypeIconPath[i] = NULL;
        }
        else
        {
            m_pCvarTypeName[i] = ICvar::CreateString(_T("Persistant_Type_") + g_sPersistantItemTypeCvars[i] + _T("_Name"), g_sPersistantItemTypes[i], CVAR_GAMECONFIG | CVAR_TRANSMIT);
            m_pCvarTypeMultiplier[i] = ICvar::CreateFloat(_T("Persistant_Type_") + g_sPersistantItemTypeCvars[i] + _T("_Multiplier"), g_fPersistantItemTypeMultipliers[i], CVAR_GAMECONFIG | CVAR_TRANSMIT);
            m_pCvarTypeDropWeight[i] = ICvar::CreateFloat(_T("Persistant_Type_") + g_sPersistantItemTypeCvars[i] + _T("_DropWeight"), 1.0f, CVAR_GAMECONFIG | CVAR_TRANSMIT);
            m_pCvarTypeIconPath[i] = ICvar::CreateString(_T("Persistant_Type_") + g_sPersistantItemTypeCvars[i] + _T("_IconPath"), _T("/items/persistant/object_") + LowerString(g_sPersistantItemTypes[i]) + _T(".tga"), CVAR_GAMECONFIG | CVAR_TRANSMIT);
        }
    }

    // Increase
    for (int i(0); i < NUM_PERSISTANT_INCREASE_MODS; ++i)
    {
        if (g_sPersistantIncreaseCvars[i].empty())
        {
            m_pCvarIncreaseName[i] = NULL;
            m_pCvarIncreaseMod[i] = NULL;
            m_pCvarIncreaseDropWeight[i] = NULL;
            m_pCvarIncreaseIconPath[i] = NULL;
        }
        else
        {
            m_pCvarIncreaseName[i] = ICvar::CreateString(_T("Persistant_Increase_") + g_sPersistantIncreaseCvars[i] + _T("_Name"), g_sPersistantIncreaseNames[i], CVAR_GAMECONFIG | CVAR_TRANSMIT);
            m_pCvarIncreaseMod[i] = ICvar::CreateString(_T("Persistant_Increase_") + g_sPersistantIncreaseCvars[i] + _T("_Modifier"), g_sPersistantIncreaseMods[i], CVAR_GAMECONFIG | CVAR_TRANSMIT);
            m_pCvarIncreaseDropWeight[i] = ICvar::CreateFloat(_T("Persistant_Increase_") + g_sPersistantIncreaseCvars[i] + _T("_DropWeight"), 1.0f, CVAR_GAMECONFIG | CVAR_TRANSMIT);
            m_pCvarIncreaseIconPath[i] = ICvar::CreateString(_T("Persistant_Increase_") + g_sPersistantIncreaseCvars[i] + _T("_IconPath"), _T("/items/persistant/animal_") + LowerString(g_sPersistantIncreaseNames[i]) + _T(".tga"), CVAR_GAMECONFIG | CVAR_TRANSMIT);
        }
    }

    // Replenish
    for (int i(0); i < NUM_PERSISTANT_REPLENISH_MODS; ++i)
    {
        if (g_sPersistantReplenishCvars[i].empty())
        {
            m_pCvarReplenishName[i] = NULL;
            m_pCvarReplenishMod[i] = NULL;
            m_pCvarReplenishDropWeight[i] = NULL;
            m_pCvarReplenishIconPath[i] = NULL;
            m_pCvarReplenishUseEffect[i] = NULL;
            m_pCvarReplenishDuration[i] = NULL;
            m_pCvarReplenishCooldown[i] = NULL;
            m_pCvarReplenishCooldownOnDamage[i] = NULL;
        }
        else
        {
            m_pCvarReplenishName[i] = ICvar::CreateString(_T("Persistant_Replenish_") + g_sPersistantReplenishCvars[i] + _T("_Name"), g_sPersistantReplenishNames[i], CVAR_GAMECONFIG | CVAR_TRANSMIT);
            m_pCvarReplenishMod[i] = ICvar::CreateString(_T("Persistant_Replenish_") + g_sPersistantReplenishCvars[i] + _T("_Modifier"), g_sPersistantReplenishMods[i], CVAR_GAMECONFIG | CVAR_TRANSMIT);
            m_pCvarReplenishDropWeight[i] = ICvar::CreateFloat(_T("Persistant_Replenish_") + g_sPersistantReplenishCvars[i] + _T("_DropWeight"), 1.0f, CVAR_GAMECONFIG | CVAR_TRANSMIT);
            m_pCvarReplenishIconPath[i] = ICvar::CreateString(_T("Persistant_Replenish_") + g_sPersistantReplenishCvars[i] + _T("_IconPath"), _T("/items/persistant/effected_") + LowerString(g_sPersistantReplenishNames[i]) + _T(".tga"), CVAR_GAMECONFIG | CVAR_TRANSMIT);
            m_pCvarReplenishUseEffect[i] = ICvar::CreateString(_T("Persistant_Replenish_") + g_sPersistantReplenishCvars[i] + _T("_UseEffect"), _T(""), CVAR_GAMECONFIG | CVAR_TRANSMIT);
            m_pCvarReplenishCooldown[i] = ICvar::CreateUInt(_T("Persistant_Replenish_") + g_sPersistantReplenishCvars[i] + _T("_Cooldown"), 30000, CVAR_GAMECONFIG | CVAR_TRANSMIT);
            m_pCvarReplenishCooldownOnDamage[i] = ICvar::CreateUInt(_T("Persistant_Replenish_") + g_sPersistantReplenishCvars[i] + _T("_CooldownOnDamage"), 10000, CVAR_GAMECONFIG | CVAR_TRANSMIT);
            if (g_bPersistantReplenishBoost[i])
                m_pCvarReplenishDuration[i] = ICvar::CreateUInt(_T("Persistant_Replenish_") + g_sPersistantReplenishCvars[i] + _T("_Duration"), 15000, CVAR_GAMECONFIG | CVAR_TRANSMIT);
        }
    }
}


/*====================
  IPersistantItem::IPersistantItem
  ====================*/
IPersistantItem::IPersistantItem() :
IInventoryItem(NULL),
m_uiRegenMod(PERSISTANT_REGEN_NULL),
m_uiIncreaseMod(PERSISTANT_INCREASE_NULL),
m_uiReplenishMod(PERSISTANT_REPLENISH_NULL),
m_uiPersistantType(PERSISTANT_TYPE_NULL),
m_uiItemID(0),
m_unItemData(PERSISTANT_ITEM_NULL),
m_iStateSlot(-1)
{
}


/*====================
  IPersistantItem::~IPersistantItem
  ====================*/
IPersistantItem::~IPersistantItem()
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return;

    if (m_iStateSlot != -1 && pOwner->GetState(m_iStateSlot) != NULL && pOwner->GetState(m_iStateSlot)->GetType() == State_PersistantItem)
    {
        // Track changes in player's health
        float fHealthPercent(1.0f);
        
        if (!pOwner->HasNetFlags(ENT_NET_FLAG_KILLED))
            fHealthPercent = pOwner->GetHealthPercent();

        pOwner->RemoveState(m_iStateSlot);

        pOwner->SetHealth(pOwner->GetMaxHealth() * fHealthPercent);
    }
}


/*====================
  IPersistantItem::GetTypeVector
  ====================*/
const vector<SDataField>&   IPersistantItem::GetTypeVector()
{
    if (!s_pvFields)
    {
        s_pvFields = K2_NEW(global,   vector<SDataField>)();
        s_pvFields->clear();
        const vector<SDataField> &vBase(IInventoryItem::GetTypeVector());
        s_pvFields->insert(s_pvFields->begin(), vBase.begin(), vBase.end());

        s_pvFields->push_back(SDataField(_T("m_unItemData"), FIELD_PUBLIC, TYPE_SHORT));
    }

    return *s_pvFields;
}


/*====================
  IPersistantItem::GetSnapshot
  ====================*/
void    IPersistantItem::GetSnapshot(CEntitySnapshot &snapshot) const
{
    IInventoryItem::GetSnapshot(snapshot);

    snapshot.AddField(m_unItemData);
}


/*====================
  IPersistantItem::ReadSnapshot
  ====================*/
bool    IPersistantItem::ReadSnapshot(CEntitySnapshot &snapshot)
{
    try
    {
        IInventoryItem::ReadSnapshot(snapshot);

        snapshot.ReadNextField(m_unItemData);

        UpdateItemData();

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("IPersistantItem::ReadSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  IPersistantItem::Baseline
  ====================*/
void    IPersistantItem::Baseline()
{
    IInventoryItem::Baseline();
    m_unItemData = PERSISTANT_ITEM_NULL;
}


/*====================
  IPersistantItem::ActivatePrimary
  ====================*/
bool    IPersistantItem::ActivatePrimary(int iButtonStatus)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;

    if (!IsReady() || IsDisabled())
        return false;

    SetCooldownTimer(Game.GetGameTime(), g_PersistantItemsConfig.GetReplenishCooldown(m_uiReplenishMod));

    const tstring &sUseEffect(g_PersistantItemsConfig.GetReplenishUseEffect(m_uiReplenishMod));

    // Use effect
    if (!sUseEffect.empty())
    {
        CGameEvent evEffect;

        evEffect.SetSourcePosition(pOwner->GetPosition());
        evEffect.SetSourceAngles(pOwner->GetAngles());

        evEffect.SetEffect(Game.RegisterEffect(sUseEffect));
        Game.AddEvent(evEffect);
    }

    switch (m_uiReplenishMod)
    {
    case PERSISTANT_REPLENISH_STAMINA:
        pOwner->SetStamina(pOwner->GetStamina() + (pOwner->GetMaxStamina() * (GetMultiplier() - 1.0f)));
        break;
    case PERSISTANT_REPLENISH_HEALTH:
        pOwner->SetHealth(pOwner->GetHealth() + (pOwner->GetMaxHealth() * (GetMultiplier() - 1.0f)));
        break;
    case PERSISTANT_REPLENISH_ARMOR:
        {
            FloatMod modValue;
            modValue.SetMult(GetMultiplier());

            // Apply state
            int iSlot(pOwner->ApplyState(State_PersistantReplenish, Game.GetGameTime(), g_PersistantItemsConfig.GetReplenishDuration(m_uiReplenishMod)));
            if (iSlot != -1)
            {
                IEntityState *pState(pOwner->GetState(iSlot));
                if (pState)
                {
                    pState->SetMod(MOD_ARMOR, modValue);
                    if (pState->GetType() == State_PersistantReplenish)
                        static_cast<CStatePersistantItem *>(pState)->SetItemData(m_unItemData);
                }
            }       
        } break;
    case PERSISTANT_REPLENISH_SPEED:
        {
            FloatMod modValue;
            modValue.SetMult(GetMultiplier());

            // Apply state
            int iSlot(pOwner->ApplyState(State_PersistantReplenish, Game.GetGameTime(), g_PersistantItemsConfig.GetReplenishDuration(m_uiReplenishMod)));
            if (iSlot != -1)
            {
                IEntityState *pState(pOwner->GetState(iSlot));
                if (pState)
                {
                    pState->SetMod(MOD_SPEED, modValue);
                    if (pState->GetType() == State_PersistantReplenish)
                        static_cast<CStatePersistantItem *>(pState)->SetItemData(m_unItemData);
                }
            }       
        } break;
    case PERSISTANT_REPLENISH_MANA:
        pOwner->SetMana(pOwner->GetMana() + (pOwner->GetMaxMana() * (GetMultiplier() - 1.0f)));
        break;
    case PERSISTANT_REPLENISH_DAMAGE:
        {
            FloatMod modValue;
            modValue.SetMult(GetMultiplier());

            // Apply state
            int iSlot(pOwner->ApplyState(State_PersistantReplenish, Game.GetGameTime(), g_PersistantItemsConfig.GetReplenishDuration(m_uiReplenishMod)));
            if (iSlot != -1)
            {
                IEntityState *pState(pOwner->GetState(iSlot));
                if (pState)
                {
                    pState->SetMod(MOD_DAMAGE, modValue);
                    if (pState->GetType() == State_PersistantReplenish)
                        static_cast<CStatePersistantItem *>(pState)->SetItemData(m_unItemData);
                }
            }       
        } break;
    }

    return true;
}


/*====================
  IPersistantItem::ActivatePassive
  ====================*/
bool    IPersistantItem::ActivatePassive()
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;

    if (pOwner == NULL)
        return false;

    if (IsDisabled())
        return false;

    if (m_iStateSlot != -1 && pOwner->GetState(m_iStateSlot) != NULL && pOwner->GetState(m_iStateSlot)->GetType() == State_PersistantItem)
        pOwner->RemoveState(m_iStateSlot);

    m_iStateSlot = pOwner->ApplyState(State_PersistantItem, INVALID_TIME, INVALID_TIME, pOwner->GetIndex());

    if (m_iStateSlot != -1)
    {
        IEntityState *pState(pOwner->GetState(m_iStateSlot));
        if (pState && pState->GetType() == State_PersistantItem)
            static_cast<CStatePersistantItem *>(pState)->SetItemData(m_unItemData);
    }

    return true;
}


/*====================
  IPersistantItem::UpdateItemData
  ====================*/
void    IPersistantItem::UpdateItemData()
{
    if (m_unItemData == 0 || m_unItemData < 1111)
        m_unItemData = PERSISTANT_ITEM_NULL;

    if (m_unItemData == PERSISTANT_ITEM_NULL)
    {
        m_uiPersistantType = 0;
        m_uiRegenMod = 0;
        m_uiIncreaseMod = 0;
        m_uiReplenishMod = 0;
        return;
    }

    m_uiPersistantType = (m_unItemData / 1000) % 10;
    m_uiRegenMod = (m_unItemData / 100) % 10;
    m_uiIncreaseMod = (m_unItemData / 10) % 10;
    m_uiReplenishMod = (m_unItemData % 10);

    if (m_uiPersistantType == 0 || m_uiRegenMod == 0 || m_uiIncreaseMod == 0 || m_uiReplenishMod == 0)
    {
        m_unItemData = PERSISTANT_ITEM_NULL;
        m_uiPersistantType = 0;
        m_uiRegenMod = 0;
        m_uiIncreaseMod = 0;
        m_uiReplenishMod = 0;
        return;
    }

    if (!Game.IsClient())
        return;

    // Set up the item's name and description...
    if (m_sItemName.empty())
    {
        m_sItemName = g_PersistantItemsConfig.GetRegenName(m_uiRegenMod);
        m_sItemName += _T(" ");
        m_sItemName += g_PersistantItemsConfig.GetTypeName(m_uiPersistantType);
        m_sItemName += _T(" of ");
        m_sItemName += g_PersistantItemsConfig.GetIncreaseName(m_uiIncreaseMod);
        m_sItemName += _T("'s ");
        m_sItemName += g_PersistantItemsConfig.GetReplenishName(m_uiReplenishMod);
    }

    if (m_sItemDescription.empty())
    {
        m_sItemDescription = _T("This ") + LowerString(g_PersistantItemsConfig.GetRegenName(m_uiRegenMod)) + _T(" ") + LowerString(g_PersistantItemsConfig.GetTypeName(m_uiPersistantType));
        m_sItemDescription += _T(" holds the spirit of the ") + g_PersistantItemsConfig.GetIncreaseName(m_uiIncreaseMod) + _T(", providing a ");
        m_sItemDescription += XtoA((g_PersistantItemsConfig.GetTypeMultiplier(m_uiPersistantType) - 1.0f) * 100, 0, 0, 0) + _T(" percent increase to your maximum ");
        m_sItemDescription += LowerString(g_PersistantItemsConfig.GetIncreaseMod(m_uiIncreaseMod)) + _T(" and ") + LowerString(g_PersistantItemsConfig.GetRegenMod(m_uiRegenMod));
        m_sItemDescription += _T(", as well as allowing you to instantly increase your ") + LowerString(g_PersistantItemsConfig.GetReplenishMod(m_uiReplenishMod)) + _T(".");
    }

    if (m_sItemIcon.empty())
    {
        if (m_uiRegenMod != PERSISTANT_REGEN_NULL)
            m_sItemIcon += g_PersistantItemsConfig.GetRegenIconPath(m_uiRegenMod);

        if (m_uiPersistantType != PERSISTANT_TYPE_NULL)
        {
            if (!m_sItemIcon.empty())
                m_sItemIcon += IMAGELIST_SEPERATOR;

            m_sItemIcon += g_PersistantItemsConfig.GetTypeIconPath(m_uiPersistantType);
        }

        if (m_uiIncreaseMod != PERSISTANT_INCREASE_NULL)
        {
            if (!m_sItemIcon.empty())
                m_sItemIcon += IMAGELIST_SEPERATOR;

            m_sItemIcon += g_PersistantItemsConfig.GetIncreaseIconPath(m_uiIncreaseMod);
        }

        if (m_uiReplenishMod != PERSISTANT_REPLENISH_NULL)
        {
            if (!m_sItemIcon.empty())
                m_sItemIcon += IMAGELIST_SEPERATOR;

            m_sItemIcon += g_PersistantItemsConfig.GetReplenishIconPath(m_uiReplenishMod);
        }
    }

    // Set to default icon if we're still empty
    if (m_sItemIcon.empty())
    {
        m_sItemIcon = GetIconPath();
    }
}


/*====================
  IPersistantItem::SetItemData
  ====================*/
void    IPersistantItem::SetItemData(ushort unData)
{
    m_unItemData = unData;
    UpdateItemData();
}

/*====================
  IPersistantItem::GetIconImageList
  ====================*/
const tstring&  IPersistantItem::GetIconImageList()
{
    return m_sItemIcon;
}


/*====================
  IPersistantItem::ClientPrecache
  ====================*/
void    IPersistantItem::ClientPrecache(CEntityConfig *pConfig)
{
    IInventoryItem::ClientPrecache(pConfig);

    for (int i(0); i < NUM_PERSISTANT_REGEN_MODS; ++i)
    {
        if (!g_PersistantItemsConfig.GetRegenIconPath(i).empty())
            g_ResourceManager.Register(K2_NEW(global,   CTexture)(g_PersistantItemsConfig.GetRegenIconPath(i), TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);
    }

    for (int i(0); i < NUM_PERSISTANT_ITEM_TYPES; ++i)
    {
        if (!g_PersistantItemsConfig.GetTypeIconPath(i).empty())
            g_ResourceManager.Register(K2_NEW(global,   CTexture)(g_PersistantItemsConfig.GetTypeIconPath(i), TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);
    }

    for (int i(0); i < NUM_PERSISTANT_INCREASE_MODS; ++i)
    {
        if (!g_PersistantItemsConfig.GetIncreaseIconPath(i).empty())
            g_ResourceManager.Register(K2_NEW(global,   CTexture)(g_PersistantItemsConfig.GetIncreaseIconPath(i), TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);
    }

    for (int i(0); i < NUM_PERSISTANT_REPLENISH_MODS; ++i)
    {
        if (!g_PersistantItemsConfig.GetReplenishIconPath(i).empty())
            g_ResourceManager.Register(K2_NEW(global,   CTexture)(g_PersistantItemsConfig.GetReplenishIconPath(i), TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);

        if (!g_PersistantItemsConfig.GetReplenishUseEffect(i).empty())
            g_ResourceManager.Register(g_PersistantItemsConfig.GetReplenishUseEffect(i), RES_EFFECT);   
    }

    EntityRegistry.ClientPrecache(State_PersistantItem);
    EntityRegistry.ClientPrecache(State_PersistantReplenish);
}


/*====================
  IPersistantItem::ServerPrecache
  ====================*/
void    IPersistantItem::ServerPrecache(CEntityConfig *pConfig)
{
    IInventoryItem::ServerPrecache(pConfig);

    for (int i(0); i < NUM_PERSISTANT_REPLENISH_MODS; ++i)
    {
        if (!g_PersistantItemsConfig.GetReplenishUseEffect(i).empty())
            g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(g_PersistantItemsConfig.GetReplenishUseEffect(i), RES_EFFECT, RES_EFFECT_IGNORE_ALL));
    }

    EntityRegistry.ServerPrecache(State_PersistantItem);
    EntityRegistry.ServerPrecache(State_PersistantReplenish);
}

