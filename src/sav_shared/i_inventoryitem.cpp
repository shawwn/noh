// (C)2006 S2 Games
// i_inventoryitem.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_inventoryitem.h"

#include "i_meleeitem.h"
#include "i_gunitem.h"
#include "i_skillitem.h"
#include "i_spellitem.h"
#include "i_siegeitem.h"
#include "i_consumableitem.h"
#include "i_persistantitem.h"

#include "../k2/c_texture.h"
#include "../k2/c_effect.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
vector<SDataField>* IInventoryItem::s_pvFields;
//=============================================================================

/*====================
  IGameEntity::CEntityConfig::CEntityConfig
  ====================*/
IInventoryItem::CEntityConfig::CEntityConfig(const tstring &sName) :
IGameEntity::CEntityConfig(sName),
INIT_ENTITY_CVAR(Name, _T("NAME")),
INIT_ENTITY_CVAR(Description, _T("DESCRIPTION")),
INIT_ENTITY_CVAR(Race, _T("")),
INIT_ENTITY_CVAR(IconPath, _T("")),
INIT_ENTITY_CVAR(CooldownTime, 0),
INIT_ENTITY_CVAR(CooldownOnDamage, 0),
INIT_ENTITY_CVAR(ManaCost, 0.0f),
INIT_ENTITY_CVAR(AmmoCount, 0),
INIT_ENTITY_CVAR(Model1Path, _T("")),
INIT_ENTITY_CVAR(Model1Bone, _T("")),
INIT_ENTITY_CVAR(Model2Path, _T("")),
INIT_ENTITY_CVAR(Model2Bone, _T("")),
INIT_ENTITY_CVAR(HoldEffect, _T("")),
INIT_ENTITY_CVAR(ImpulseOnly, true),
INIT_ENTITY_CVAR(Cost, 0),
INIT_ENTITY_CVAR(Prerequisite, _T("")),
INIT_ENTITY_CVAR(SpeedMult, 1.0f)
{
}


/*====================
  IInventoryItem::IInventoryItem
  ====================*/
IInventoryItem::IInventoryItem(CEntityConfig *pConfig) :
IGameEntity(pConfig),
m_pEntityConfig(pConfig),

m_uiOwnerIndex(INVALID_INDEX),
m_ySlot(-1),
m_uiStartCooldown(INVALID_TIME),
m_uiCooldownDuration(INVALID_TIME),
m_unAmmo(0),

m_fHandFOV(60.0f)
{
}


/*====================
  IInventoryItem::~IInventoryItem
  ====================*/
IInventoryItem::~IInventoryItem()
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (pOwner)
        pOwner->SetInventorySlot(m_ySlot, NULL);
}

const IMeleeItem*       IInventoryItem::GetAsMelee() const      { if (!IsMelee()) return NULL; else return static_cast<const IMeleeItem*>(this); }
const IGunItem*         IInventoryItem::GetAsGun() const        { if (!IsGun()) return NULL; else return static_cast<const IGunItem*>(this); }
const ISkillItem*       IInventoryItem::GetAsSkill() const      { if (!IsSkill()) return NULL; else return static_cast<const ISkillItem*>(this); }
const ISpellItem*       IInventoryItem::GetAsSpell() const      { if (!IsSpell()) return NULL; else return static_cast<const ISpellItem*>(this); }
const ISiegeItem*       IInventoryItem::GetAsSiege() const      { if (!IsSiege()) return NULL; else return static_cast<const ISiegeItem*>(this); }
const IConsumableItem*  IInventoryItem::GetAsConsumable() const { if (!IsConsumable()) return NULL; else return static_cast<const IConsumableItem*>(this); }
const IPersistantItem*  IInventoryItem::GetAsPersistant() const { if (!IsPersistant()) return NULL; else return static_cast<const IPersistantItem*>(this); }

IMeleeItem*             IInventoryItem::GetAsMelee()            { if (!IsMelee()) return NULL; else return static_cast<IMeleeItem*>(this); }
IGunItem*               IInventoryItem::GetAsGun()              { if (!IsGun()) return NULL; else return static_cast<IGunItem*>(this); }
ISkillItem*             IInventoryItem::GetAsSkill()            { if (!IsSkill()) return NULL; else return static_cast<ISkillItem*>(this); }
ISpellItem*             IInventoryItem::GetAsSpell()            { if (!IsSpell()) return NULL; else return static_cast<ISpellItem*>(this); }
ISiegeItem*             IInventoryItem::GetAsSiege()            { if (!IsSiege()) return NULL; else return static_cast<ISiegeItem*>(this); }
IConsumableItem*        IInventoryItem::GetAsConsumable()       { if (!IsConsumable()) return NULL; else return static_cast<IConsumableItem*>(this); }
IPersistantItem*        IInventoryItem::GetAsPersistant()       { if (!IsPersistant()) return NULL; else return static_cast<IPersistantItem*>(this); }


/*====================
  IInventoryItem::GetTypeVector
  ====================*/
const vector<SDataField>&   IInventoryItem::GetTypeVector()
{
    if (!s_pvFields)
    {
        s_pvFields = K2_NEW(global,   vector<SDataField>)();
        s_pvFields->clear();
        const vector<SDataField> &vBase(IGameEntity::GetTypeVector());
        s_pvFields->insert(s_pvFields->begin(), vBase.begin(), vBase.end());

        s_pvFields->push_back(SDataField(_T("m_uiOwnerIndex"), FIELD_PUBLIC, TYPE_GAMEINDEX));
        s_pvFields->push_back(SDataField(_T("m_ySlot"), FIELD_PUBLIC, TYPE_CHAR));
        s_pvFields->push_back(SDataField(_T("m_uiStartCooldown"), FIELD_PRIVATE, TYPE_INT));
        s_pvFields->push_back(SDataField(_T("m_uiCooldownDuration"), FIELD_PRIVATE, TYPE_INT));
        s_pvFields->push_back(SDataField(_T("m_unAmmo"), FIELD_PUBLIC, TYPE_SHORT));
    }

    return *s_pvFields;
}


/*====================
  IInventoryItem::GetSnapshot
  ====================*/
void    IInventoryItem::GetSnapshot(CEntitySnapshot &snapshot) const
{
    IGameEntity::GetSnapshot(snapshot);

    snapshot.AddGameIndex(m_uiOwnerIndex);
    snapshot.AddField(m_ySlot);
    snapshot.AddField(m_uiStartCooldown);
    snapshot.AddField(m_uiCooldownDuration);
    snapshot.AddField(m_unAmmo);
}


/*====================
  IInventoryItem::ReadSnapshot
  ====================*/
bool    IInventoryItem::ReadSnapshot(CEntitySnapshot &snapshot)
{
    try
    {
        IGameEntity::ReadSnapshot(snapshot);

        snapshot.ReadNextGameIndex(m_uiOwnerIndex);
        snapshot.ReadNextField(m_ySlot);
        snapshot.ReadNextField(m_uiStartCooldown);
        snapshot.ReadNextField(m_uiCooldownDuration);
        snapshot.ReadNextField(m_unAmmo);

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("IInventoryItem::ReadSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  IInventoryItem::Baseline
  ====================*/
void    IInventoryItem::Baseline()
{
    IGameEntity::Baseline();

    m_uiOwnerIndex = INVALID_INDEX;
    m_ySlot = -1;
    m_uiStartCooldown = INVALID_TIME;
    m_uiCooldownDuration = INVALID_TIME;
    m_unAmmo = 0;
}


/*====================
  IInventoryItem::GetPrivateClient
  ====================*/
int     IInventoryItem::GetPrivateClient()
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (pOwner && pOwner->IsPlayer())
        return pOwner->GetAsPlayerEnt()->GetClientID();
    else
        return -1;
}


/*====================
  IInventoryItem::GetFov
  ====================*/
float   IInventoryItem::GetFov() const
{
    IPlayerEntity *pOwner(Game.GetPlayerEntity(GetOwner()));
    if (pOwner == NULL)
        return 90.0f;
    return pOwner->GetBaseFov();
}


/*====================
  IInventoryItem::Activate
  ====================*/
void    IInventoryItem::Activate()
{
    if (!IsDisabled())
        Game.SelectItem(m_ySlot);
}


/*====================
  IInventoryItem::FinishedAction
  ====================*/
void    IInventoryItem::FinishedAction(int iAction)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (pOwner)
        pOwner->SetAction(PLAYER_ACTION_IDLE, -1);
}


/*====================
  IInventoryItem::GetAdjustedAmmoCount
  ====================*/
int     IInventoryItem::GetAdjustedAmmoCount() const
{
    float fAmmoMult(1.0f);

    if (IsGun())
    {
        ICombatEntity *pOwner(GetOwnerEnt());
        if (pOwner)
        {
            for (int i(INVENTORY_START_BACKPACK); i < INVENTORY_END_BACKPACK; ++i)
            {
                IInventoryItem *pItem(pOwner->GetItem(i));
                if (pItem == NULL)
                    continue;

                IConsumableItem *pConsumable(pItem->GetAsConsumable());
                if (pConsumable == NULL)
                    continue;

                fAmmoMult *= pConsumable->GetAmmoMult();
            }
        }
    }

    return int(GetAmmoCount() * fAmmoMult);
}


/*====================
  IInventoryItem::OwnerDamaged
  ====================*/
float   IInventoryItem::OwnerDamaged(float fDamage, int iFlags, IVisualEntity *pAttacker)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return fDamage;

    // Reset cooldown timer on damage
    if (fDamage > 0.0f && GetCooldownOnDamage() > 0 && (m_uiStartCooldown == INVALID_TIME || m_uiCooldownDuration == INVALID_TIME ||
        Game.GetGameTime() + GetCooldownOnDamage() > m_uiStartCooldown + m_uiCooldownDuration))
        SetCooldownTimer(Game.GetGameTime(), GetCooldownOnDamage());

    return fDamage;
}


/*====================
  IInventoryItem::GetPrerequisiteDescription
  ====================*/
tstring IInventoryItem::GetPrerequisiteDescription() const
{
    if (m_pEntityConfig == NULL)
        return _T("None");

    const tstring &sPrerequisite(m_pEntityConfig->GetPrerequisite());
    if (sPrerequisite.empty())
        return _T("None");

    ushort unType(EntityRegistry.LookupID(sPrerequisite));
    if (unType == INVALID_ENT_TYPE)
        return _T("None");

    ICvar *pNameVar(EntityRegistry.GetGameSetting(unType, _T("name")));
    if (pNameVar == NULL)
        return _T("<ERROR>");

    return pNameVar->GetString();
}


/*====================
  IInventoryItem::ClientPrecache
  ====================*/
void    IInventoryItem::ClientPrecache(CEntityConfig *pConfig)
{
    IGameEntity::ClientPrecache(pConfig);

    if (!pConfig)
        return;

    if (!pConfig->GetIconPath().empty())
        g_ResourceManager.Register(K2_NEW(global,   CTexture)(pConfig->GetIconPath(), TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);

    if (!pConfig->GetModel1Path().empty())
    {
        ResHandle hModel(g_ResourceManager.Register(pConfig->GetModel1Path(), RES_MODEL));

        if (hModel != INVALID_RESOURCE)
            g_ResourceManager.PrecacheSkin(hModel, -1);
    }

    if (!pConfig->GetModel2Path().empty())
    {
        ResHandle hModel(g_ResourceManager.Register(pConfig->GetModel2Path(), RES_MODEL));
        
        if (hModel != INVALID_RESOURCE)
            g_ResourceManager.PrecacheSkin(hModel, -1);
    }

    if (!pConfig->GetHoldEffect().empty())
        g_ResourceManager.Register(pConfig->GetHoldEffect(), RES_EFFECT);
}


/*====================
  IInventoryItem::ServerPrecache

  Setup network resource handles and anything else the server needs for this entity
  ====================*/
void    IInventoryItem::ServerPrecache(CEntityConfig *pConfig)
{
    IGameEntity::ServerPrecache(pConfig);

    if (!pConfig)
        return;

    if (!pConfig->GetHoldEffect().empty())
        g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetHoldEffect(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));
}


/*====================
  IInventoryItem::GetIconImageList
  ====================*/
const tstring&  IInventoryItem::GetIconImageList()
{
    if (m_pEntityConfig)
        return m_pEntityConfig->GetIconPath().GetValue();
    else
        return SNULL;
}


/*====================
  IInventoryItem::GetOwnerEnt
  ====================*/
ICombatEntity*  IInventoryItem::GetOwnerEnt() const
{
    if (m_uiOwnerIndex == INVALID_INDEX)
        return NULL;

    IGameEntity *pEntity(Game.GetEntity(m_uiOwnerIndex));
    if (pEntity && pEntity->IsCombat())
        return pEntity->GetAsCombatEnt();
    else
        return NULL;
}


/*====================
  IInventoryItem::IsDisabled
  ====================*/
bool    IInventoryItem::IsDisabled() const
{
    return HasNetFlags(ITEM_NET_FLAG_DISABLED);
}


/*====================
  IInventoryItem::IsSilenced
  ====================*/
bool    IInventoryItem::IsSilenced() const
{
    IVisualEntity *pOwner(Game.GetVisualEntity(m_uiOwnerIndex));
    if (pOwner != NULL &&
        pOwner->IsSilenced() &&
        !IsMelee() &&
        !IsGun() &&
        !HasNetFlags(ITEM_NET_FLAG_ACTIVE))
        return true;

    return false;
}


/*====================
  IInventoryItem::IsReady
  ====================*/
bool    IInventoryItem::IsReady() const
{
    if (IsSilenced())
        return false;

    IVisualEntity *pOwner(Game.GetVisualEntity(m_uiOwnerIndex));
    if (pOwner && pOwner->GetStatus() != ENTITY_STATUS_ACTIVE)
        return false;

    return m_uiStartCooldown == INVALID_TIME || m_uiCooldownDuration == INVALID_TIME || Game.GetGameTime() >= m_uiStartCooldown + m_uiCooldownDuration;
}


/*====================
  IInventoryItem::SetCooldownTimer
  ====================*/
void    IInventoryItem::SetCooldownTimer(uint uiStartCooldown, uint uiCooldownDuration)
{
    if (uiCooldownDuration == 0 || uiCooldownDuration == INVALID_TIME || uiStartCooldown == INVALID_TIME)
    {
        m_uiStartCooldown = INVALID_TIME;
    }
    else
    {
        m_uiStartCooldown = uiStartCooldown;
        m_uiCooldownDuration = uiCooldownDuration;
    }
}


/*====================
  IInventoryItem::GetAmmoPercent
  ====================*/
float   IInventoryItem::GetAmmoPercent() const
{
    if (GetAdjustedAmmoCount() == 0)
        return -1.0f;

    if (IsDisabled())
        return -1.0f;

    return GetAmmo() / float(GetAdjustedAmmoCount());
}


/*====================
  IInventoryItem::GetCooldownPercent
  ====================*/
float   IInventoryItem::GetCooldownPercent() const
{
    if (m_uiStartCooldown == INVALID_TIME || m_uiCooldownDuration == INVALID_TIME || Game.GetGameTime() >= m_uiStartCooldown + m_uiCooldownDuration)
        return 0.0f;

    uint uiTimeLeft(m_uiStartCooldown + m_uiCooldownDuration - Game.GetGameTime());

    return CLAMP(uiTimeLeft / float(m_uiCooldownDuration), 0.0f, 1.0f);
}
