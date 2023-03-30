// (C)2006 S2 Games
// i_persistantitem.h
//
//=============================================================================
#ifndef __I_PERSISTANTITEM_H__
#define __I_PERSISTANTITEM_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_inventoryitem.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define MAX_CARRIED_PERSISTANT_ITEMS 2

enum EPersistantRegenModifiers
{
    PERSISTANT_REGEN_NULL,
    PERSISTANT_REGEN_HEALTH,
    PERSISTANT_REGEN_INCOME,
    PERSISTANT_REGEN_MANA,
    PERSISTANT_REGEN_STAMINA,
    PERSISTANT_REGEN_UNUSED1,
    PERSISTANT_REGEN_UNUSED2,
    PERSISTANT_REGEN_UNUSED3,
    PERSISTANT_REGEN_UNUSED4,
    PERSISTANT_REGEN_UNUSED5,

    NUM_PERSISTANT_REGEN_MODS
};

const tstring g_sPersistantRegenModifiers[NUM_PERSISTANT_REGEN_MODS] = 
{
    _T("NULL"),
    _T("Health Regeneration"),
    _T("Income Generation"),
    _T("Mana Regeneration"),
    _T("Stamina Regeneration"),
    _T("NULL"),
    _T("NULL"),
    _T("NULL"),
    _T("NULL"),
    _T("NULL")
};

const tstring g_sPersistantRegenModifierCvars[NUM_PERSISTANT_REGEN_MODS] = 
{
    _T(""),
    _T("Health"),
    _T("Income"),
    _T("Mana"),
    _T("Stamina"),
    _T(""),
    _T(""),
    _T(""),
    _T(""),
    _T("")
};


const tstring g_sPersistantRegenColors[NUM_PERSISTANT_REGEN_MODS] = 
{
    _T("NULL"),
    _T("Red"),
    _T("Yellow"),
    _T("Blue"),
    _T("White"),
    _T("Orange"),
    _T("Green"),
    _T("Turquoise"),
    _T("Pink"),
    _T("Purple")
};

enum EPersistantItemTypes
{
    PERSISTANT_TYPE_NULL,
    PERSISTANT_TYPE_RING,
    PERSISTANT_TYPE_AMULET,
    PERSISTANT_TYPE_JEWEL,
    PERSISTANT_TYPE_RUNE,

    NUM_PERSISTANT_ITEM_TYPES
};

const tstring g_sPersistantItemTypeCvars[NUM_PERSISTANT_ITEM_TYPES] = 
{
    _T(""),
    _T("Ring"),
    _T("Amulet"),
    _T("Jewel"),
    _T("Rune")
};

const tstring g_sPersistantItemTypes[NUM_PERSISTANT_ITEM_TYPES] = 
{
    _T("NULL"),
    _T("Ring"),
    _T("Amulet"),
    _T("Jewel"),
    _T("Rune")
};

const float g_fPersistantItemTypeMultipliers[NUM_PERSISTANT_ITEM_TYPES] = 
{
    1.0f,
    1.05f,
    1.08f,
    1.11f,
    1.15f
};

enum EPersistantIncreaseMods
{
    PERSISTANT_INCREASE_NULL,
    PERSISTANT_INCREASE_MANA,
    PERSISTANT_INCREASE_INCOME,
    PERSISTANT_INCREASE_EVASION,
    PERSISTANT_INCREASE_ARMOR,
    PERSISTANT_INCREASE_HEALTH,
    PERSISTANT_INCREASE_SPEED,
    PERSISTANT_INCREASE_STAMINA,
    PERSISTANT_INCREASE_DAMAGE,

    NUM_PERSISTANT_INCREASE_MODS
};

const tstring g_sPersistantIncreaseCvars[NUM_PERSISTANT_INCREASE_MODS] = 
{
    _T(""),
    _T("Mana"),
    _T("Income"),
    _T("Evasion"),
    _T("Armor"),
    _T("Health"),
    _T("Speed"),
    _T("Stamina"),
    _T("Damage")
};

const tstring g_sPersistantIncreaseNames[NUM_PERSISTANT_INCREASE_MODS] = 
{
    _T("NULL"),
    _T("Dolphin"),
    _T("Beaver"),
    _T("Fox"),
    _T("Armadillo"),
    _T("Bear"),
    _T("Cheetah"),
    _T("Rabbit"),
    _T("Lion")
};

const tstring g_sPersistantIncreaseMods[NUM_PERSISTANT_INCREASE_MODS] = 
{
    _T("NULL"),
    _T("Mana"),
    _T("Income"),
    _T("Evasion"),
    _T("Armor"),
    _T("Health"),
    _T("Speed"),
    _T("Stamina"),
    _T("Damage")
};

enum EPersistantReplenishMods
{
    PERSISTANT_REPLENISH_NULL,
    PERSISTANT_REPLENISH_STAMINA,
    PERSISTANT_REPLENISH_HEALTH,
    PERSISTANT_REPLENISH_ARMOR,
    PERSISTANT_REPLENISH_SPEED,
    PERSISTANT_REPLENISH_MANA,
    PERSISTANT_REPLENISH_DAMAGE,

    NUM_PERSISTANT_REPLENISH_MODS
};

const tstring g_sPersistantReplenishCvars[NUM_PERSISTANT_REPLENISH_MODS] = 
{
    _T(""),
    _T("Stamina"),
    _T("Health"),
    _T("Armor"),
    _T("Speed"),
    _T("Mana"),
    _T("Damage")
};

const tstring g_sPersistantReplenishNames[NUM_PERSISTANT_REPLENISH_MODS] = 
{
    _T("NULL"),
    _T("Lungs"),
    _T("Heart"),
    _T("Shield"),
    _T("Feet"),
    _T("Brain"),
    _T("Dagger")
};

const tstring g_sPersistantReplenishMods[NUM_PERSISTANT_REPLENISH_MODS] = 
{
    _T("NULL"),
    _T("Stamina"),
    _T("Health"),
    _T("Armor"),
    _T("Speed"),
    _T("Mana"),
    _T("Damage")
};

const bool g_bPersistantReplenishBoost[NUM_PERSISTANT_REPLENISH_MODS] = 
{
    false,
    false,
    false,
    true,
    true,
    false,
    true
};

const ushort    PERSISTANT_ITEM_NULL    (-1);

#define DECLARE_PERSISTANT_ITEM_CVAR(type, name, size) \
private: \
    CCvar<type, type>   *m_pCvar##name[size]; \
public: \
    type    Get##name(uint uiIndex) const   { return *m_pCvar##name; }
//=============================================================================


//=============================================================================
// CPersistantItemsConfig
//=============================================================================
class CPersistantItemsConfig
{
protected:
    // Defaults
    CCvar<tstring>      *m_pCvarDefaultIcon;

    // Regen
    CCvar<tstring>      *m_pCvarRegenName[NUM_PERSISTANT_REGEN_MODS];
    CCvar<tstring>      *m_pCvarRegenMod[NUM_PERSISTANT_REGEN_MODS];
    CCvar<float>        *m_pCvarRegenDropWeight[NUM_PERSISTANT_REGEN_MODS];
    CCvar<tstring>      *m_pCvarRegenIconPath[NUM_PERSISTANT_REGEN_MODS];

    // Type
    CCvar<tstring>      *m_pCvarTypeName[NUM_PERSISTANT_ITEM_TYPES];
    CCvar<float>        *m_pCvarTypeMultiplier[NUM_PERSISTANT_ITEM_TYPES];
    CCvar<float>        *m_pCvarTypeDropWeight[NUM_PERSISTANT_ITEM_TYPES];
    CCvar<tstring>      *m_pCvarTypeIconPath[NUM_PERSISTANT_ITEM_TYPES];

    // Increase
    CCvar<tstring>      *m_pCvarIncreaseName[NUM_PERSISTANT_INCREASE_MODS];
    CCvar<tstring>      *m_pCvarIncreaseMod[NUM_PERSISTANT_INCREASE_MODS];
    CCvar<float>        *m_pCvarIncreaseDropWeight[NUM_PERSISTANT_INCREASE_MODS];
    CCvar<tstring>      *m_pCvarIncreaseIconPath[NUM_PERSISTANT_INCREASE_MODS];

    // Replenish
    CCvar<tstring>      *m_pCvarReplenishName[NUM_PERSISTANT_REPLENISH_MODS];
    CCvar<tstring>      *m_pCvarReplenishMod[NUM_PERSISTANT_REPLENISH_MODS];
    CCvar<float>        *m_pCvarReplenishDropWeight[NUM_PERSISTANT_REPLENISH_MODS];
    CCvar<tstring>      *m_pCvarReplenishIconPath[NUM_PERSISTANT_REPLENISH_MODS];
    CCvar<tstring>      *m_pCvarReplenishUseEffect[NUM_PERSISTANT_REPLENISH_MODS];
    CCvar<uint>         *m_pCvarReplenishDuration[NUM_PERSISTANT_REPLENISH_MODS];
    CCvar<uint>         *m_pCvarReplenishCooldown[NUM_PERSISTANT_REPLENISH_MODS];
    CCvar<uint>         *m_pCvarReplenishCooldownOnDamage[NUM_PERSISTANT_REPLENISH_MODS];

public:
    ~CPersistantItemsConfig();
    CPersistantItemsConfig();

    const tstring&  GetRegenName(uint ui)           { return m_pCvarRegenName[ui] ? m_pCvarRegenName[ui]->GetValue() : SNULL; }
    const tstring&  GetRegenMod(uint ui)            { return m_pCvarRegenMod[ui] ? m_pCvarRegenMod[ui]->GetValue() : SNULL; }
    float           GetRegenDropWeight(uint ui)     { return m_pCvarRegenDropWeight[ui] ? m_pCvarRegenDropWeight[ui]->GetValue() : 0.0f; }
    const tstring&  GetRegenIconPath(uint ui)       { return m_pCvarRegenIconPath[ui] ? m_pCvarRegenIconPath[ui]->GetValue() : SNULL; }

    const tstring&  GetTypeName(uint ui)            { return m_pCvarTypeName[ui] ? m_pCvarTypeName[ui]->GetValue() : SNULL; }
    float           GetTypeMultiplier(uint ui)      { return m_pCvarTypeMultiplier[ui] ? m_pCvarTypeMultiplier[ui]->GetValue() : 0.0f; }
    float           GetTypeDropWeight(uint ui)      { return m_pCvarTypeDropWeight[ui] ? m_pCvarTypeDropWeight[ui]->GetValue() : 0.0f; }
    const tstring&  GetTypeIconPath(uint ui)        { return m_pCvarTypeIconPath[ui] ? m_pCvarTypeIconPath[ui]->GetValue() : SNULL; }

    const tstring&  GetIncreaseName(uint ui)        { return m_pCvarIncreaseName[ui] ? m_pCvarIncreaseName[ui]->GetValue() : SNULL; }
    const tstring&  GetIncreaseMod(uint ui)         { return m_pCvarIncreaseMod[ui] ? m_pCvarIncreaseMod[ui]->GetValue() : SNULL; }
    float           GetIncreaseDropWeight(uint ui)  { return m_pCvarIncreaseDropWeight[ui] ? m_pCvarIncreaseDropWeight[ui]->GetValue() : 0.0f; }
    const tstring&  GetIncreaseIconPath(uint ui)    { return m_pCvarIncreaseIconPath[ui] ? m_pCvarIncreaseIconPath[ui]->GetValue() : SNULL; }

    const tstring&  GetReplenishName(uint ui)       { return m_pCvarReplenishName[ui] ? m_pCvarReplenishName[ui]->GetValue() : SNULL; }
    const tstring&  GetReplenishMod(uint ui)        { return m_pCvarReplenishMod[ui] ? m_pCvarReplenishMod[ui]->GetValue() : SNULL; }
    float           GetReplenishDropWeight(uint ui) { return m_pCvarReplenishDropWeight[ui] ? m_pCvarReplenishDropWeight[ui]->GetValue() : 0.0f; }
    const tstring&  GetReplenishIconPath(uint ui)   { return m_pCvarReplenishIconPath[ui] ? m_pCvarReplenishIconPath[ui]->GetValue() : SNULL; }
    const tstring&  GetReplenishUseEffect(uint ui)  { return m_pCvarReplenishUseEffect[ui] ? m_pCvarReplenishUseEffect[ui]->GetValue() : SNULL; }
    uint            GetReplenishDuration(uint ui)   { return m_pCvarReplenishDuration[ui] ? m_pCvarReplenishDuration[ui]->GetValue() : 0; }
    uint            GetReplenishCooldown(uint ui)   { return m_pCvarReplenishCooldown[ui] ? m_pCvarReplenishCooldown[ui]->GetValue() : 0; }
    uint            GetReplenishCooldownOnDamage(uint ui)   { return m_pCvarReplenishCooldownOnDamage[ui] ? m_pCvarReplenishCooldownOnDamage[ui]->GetValue() : 0; }

    const tstring&  GetDefaultIcon()                { return m_pCvarDefaultIcon ? m_pCvarDefaultIcon->GetValue() : SNULL; }
};

extern GAME_SHARED_API CPersistantItemsConfig g_PersistantItemsConfig;
//=============================================================================


//=============================================================================
// IPersistantItem
//=============================================================================
class IPersistantItem : public IInventoryItem
{
private:
    static vector<SDataField>   *s_pvFields;

protected:
    uint        m_uiRegenMod;
    uint        m_uiIncreaseMod;
    uint        m_uiReplenishMod;
    uint        m_uiPersistantType;
    uint        m_uiItemID;
    ushort      m_unItemData;
    int         m_iStateSlot;
    tstring     m_sItemName;
    tstring     m_sItemDescription;
    tstring     m_sItemIcon;

    void        UpdateItemData();

public:
    ~IPersistantItem();
    IPersistantItem();

    bool            IsPersistant() const                { return true; }

    virtual const tstring&  GetEntityName() const           { return m_sItemName; }
    virtual const tstring&  GetEntityDescription() const    { return m_sItemDescription; }

    // Network
    GAME_SHARED_API virtual void    Baseline();
    GAME_SHARED_API virtual void    GetSnapshot(CEntitySnapshot &snapshot) const;
    GAME_SHARED_API virtual bool    ReadSnapshot(CEntitySnapshot &snapshot);
    static const vector<SDataField>&    GetTypeVector();

    virtual bool    ActivatePrimary(int iButtonStatus);
    virtual bool    ActivatePassive();
    virtual void    Activate()  { ActivatePrimary(GAME_BUTTON_STATUS_DOWN | GAME_BUTTON_STATUS_PRESSED); }

    uint            GetMaxPerStack() const              { return 1; }
    uint            GetMaxStacks() const                { return 1; }

    int             GetAmmoCount() const                { return 0; }
    uint            GetTotalAmmo() const                { return 0; }
    uint            GetAmmoPercent() const              { return 1; }

    uint            GetItemID()                         { return m_uiItemID; }
    void            SetItemID(uint uiID)                { m_uiItemID = uiID; }

    float           GetMultiplier()                     { return g_fPersistantItemTypeMultipliers[m_uiPersistantType]; }

    GAME_SHARED_API void    SetItemData(ushort unData);
    ushort                  GetItemData()               { return m_unItemData; }

    GAME_SHARED_API const tstring&  GetIconImageList();

    static void     ClientPrecache(CEntityConfig *pConfig);
    static void     ServerPrecache(CEntityConfig *pConfig);

    virtual uint    GetCooldownTime() const             { return g_PersistantItemsConfig.GetReplenishCooldown(m_uiReplenishMod); }
    virtual uint    GetCooldownOnDamage() const         { return g_PersistantItemsConfig.GetReplenishCooldownOnDamage(m_uiReplenishMod); }
};
//=============================================================================

#endif //__I_PERSISTANTITEM_H__
