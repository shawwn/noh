// (C)2006 S2 Games
// i_inventoryitem.h
//
//=============================================================================
#ifndef __I_INVENTORYITEM_H__
#define __I_INVENTORYITEM_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gameentity.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class IGunItem;
class IMeleeItem;
class ISkillItem;
class ISpellItem;
class ISiegeItem;
class IConsumableItem;
class CCamera;

class CClientSnapshot;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint ITEM_NET_FLAG_DISABLED           (BIT(0));
const uint ITEM_NET_FLAG_ACTIVE             (BIT(10));

#define TYPE_NAME(name) virtual const tstring&  GetTypeName()   { static tstring sTypeName(_T(name)); return sTypeName; }
//=============================================================================

//=============================================================================
// IInventoryItem
//=============================================================================
class IInventoryItem : public IGameEntity
{
private:
    static vector<SDataField>   *s_pvFields;

    IInventoryItem();

protected:
    // Cvar settings
    START_ENTITY_CONFIG(IGameEntity)
        DECLARE_ENTITY_CVAR(tstring, Name)
        DECLARE_ENTITY_CVAR(tstring, Description)
        DECLARE_ENTITY_CVAR(tstring, Race);
        DECLARE_ENTITY_CVAR(tstring, IconPath)
        DECLARE_ENTITY_CVAR(uint, CooldownTime)
        DECLARE_ENTITY_CVAR(uint, CooldownOnDamage)
        DECLARE_ENTITY_CVAR(float, ManaCost)
        DECLARE_ENTITY_CVAR(int, AmmoCount)
        DECLARE_ENTITY_CVAR(tstring, Model1Path)
        DECLARE_ENTITY_CVAR(tstring, Model1Bone)
        DECLARE_ENTITY_CVAR(tstring, Model2Path)
        DECLARE_ENTITY_CVAR(tstring, Model2Bone)
        DECLARE_ENTITY_CVAR(tstring, HoldEffect)
        DECLARE_ENTITY_CVAR(bool, ImpulseOnly)
        DECLARE_ENTITY_CVAR(int, Cost)
        DECLARE_ENTITY_CVAR(tstring, Prerequisite)
        DECLARE_ENTITY_CVAR(float, SpeedMult)
    END_ENTITY_BASE_CONFIG

    CEntityConfig*  m_pEntityConfig;

    uint            m_uiOwnerIndex;
    byte            m_ySlot;
    uint            m_uiStartCooldown;
    uint            m_uiCooldownDuration;
    ushort          m_unAmmo;

    float           m_fHandFOV;

public:
    ~IInventoryItem();
    IInventoryItem(CEntityConfig *pConfig);

    // Network
    GAME_SHARED_API virtual void    Baseline();
    GAME_SHARED_API virtual void    GetSnapshot(CEntitySnapshot &snapshot) const;
    GAME_SHARED_API virtual bool    ReadSnapshot(CEntitySnapshot &snapshot);
    static const vector<SDataField>&    GetTypeVector();

    virtual int         GetPrivateClient();

    static void         ClientPrecache(CEntityConfig *pConfig);
    static void         ServerPrecache(CEntityConfig *pConfig);

    void            SetOwner(uint uiIndex)              { m_uiOwnerIndex = uiIndex; }
    uint            GetOwner() const                    { return m_uiOwnerIndex; }

    GAME_SHARED_API ICombatEntity*  GetOwnerEnt() const;

    void            SetSlot(byte ySlot)                 { m_ySlot = ySlot; }
    byte            GetSlot() const                     { return m_ySlot; }

    GAME_SHARED_API void    SetCooldownTimer(uint uiStartCooldown, uint uiCooldownDuration);

    virtual void    SetAmmo(ushort unAmmo)              { m_unAmmo = unAmmo; }
    virtual void    AdjustAmmo(int iAmount)             { m_unAmmo += iAmount; }
    virtual ushort  GetAmmo() const                     { return m_unAmmo; }

    virtual bool    IsInventoryItem() const { return true; }

    virtual bool    IsMelee() const         { return false; }
    virtual bool    IsGun() const           { return false; }
    virtual bool    IsSkill() const         { return false; }
    virtual bool    IsSpell() const         { return false; }
    virtual bool    IsSiege() const         { return false; }
    virtual bool    IsConsumable() const    { return false; }
    virtual bool    IsPersistant() const    { return false; }

    GAME_SHARED_API const IMeleeItem*       GetAsMelee() const;
    GAME_SHARED_API const IGunItem*         GetAsGun() const;
    GAME_SHARED_API const ISkillItem*       GetAsSkill() const;
    GAME_SHARED_API const ISpellItem*       GetAsSpell() const;
    GAME_SHARED_API const ISiegeItem*       GetAsSiege() const;
    GAME_SHARED_API const IConsumableItem*  GetAsConsumable() const;

    GAME_SHARED_API IMeleeItem*             GetAsMelee();
    GAME_SHARED_API IGunItem*               GetAsGun();
    GAME_SHARED_API ISkillItem*             GetAsSkill();
    GAME_SHARED_API ISpellItem*             GetAsSpell();
    GAME_SHARED_API ISiegeItem*             GetAsSiege();
    GAME_SHARED_API IConsumableItem*        GetAsConsumable();

    GAME_SHARED_API virtual const tstring&      GetIconImageList();

    void                    Disable()                   { m_uiNetFlags |= ITEM_NET_FLAG_DISABLED; }
    void                    Enable()                    { m_uiNetFlags &= ~ITEM_NET_FLAG_DISABLED; }
    GAME_SHARED_API bool    IsDisabled() const;
    GAME_SHARED_API bool    IsReady() const;
    GAME_SHARED_API bool    IsSilenced() const;

    virtual bool    IsFirstPerson() const               { return false; }
    virtual float   GetFov() const;
    virtual void    ApplyDrift(CVec3f &v3Angles)        {}

    virtual void    Selected()                          {}
    virtual void    Unselected()                        {}

    GAME_SHARED_API virtual void    Activate();
    virtual bool    ActivatePrimary(int iButtonStatus) = 0;
    virtual bool    ActivateSecondary(int iButtonStatus)    { return false; }
    virtual bool    ActivateTertiary(int iButtonStatus)     { return false; }
    virtual bool    Cancel(int iButtonStatus)               { return false; }
    virtual bool    ActivatePassive()                       { return false; }
    GAME_SHARED_API virtual void    FinishedAction(int iAction);
    GAME_SHARED_API virtual float   OwnerDamaged(float fDamage, int iFlags, IVisualEntity *pAttacker = NULL);
    virtual void    LocalClientFrame()                      {}

    virtual const tstring&  GetTypeName() = 0;
    
    GAME_SHARED_API tstring GetPrerequisiteDescription() const;

    // Settings
    ENTITY_CVAR_ACCESSOR(tstring, Name, _T(""))
    ENTITY_CVAR_ACCESSOR(tstring, Description, _T(""))
    ENTITY_CVAR_ACCESSOR(tstring, IconPath, _T(""))
    virtual ENTITY_CVAR_ACCESSOR(uint, CooldownTime, 0)
    virtual ENTITY_CVAR_ACCESSOR(uint, CooldownOnDamage, 0)
    ENTITY_CVAR_ACCESSOR(float, ManaCost, 0.0f)
    ENTITY_CVAR_ACCESSOR(int, AmmoCount, 0)
    ENTITY_CVAR_ACCESSOR(tstring, Model1Path, _T(""))
    ENTITY_CVAR_ACCESSOR(tstring, Model1Bone, _T(""))
    ENTITY_CVAR_ACCESSOR(tstring, Model2Path, _T(""))
    ENTITY_CVAR_ACCESSOR(tstring, Model2Bone, _T(""))
    ENTITY_CVAR_ACCESSOR(tstring, HoldEffect, _T(""))
    ENTITY_CVAR_ACCESSOR(bool, ImpulseOnly, false)
    ENTITY_CVAR_ACCESSOR(int, Cost, 0)
    ENTITY_CVAR_ACCESSOR(tstring, Prerequisite, _T(""))
    ENTITY_CVAR_ACCESSOR(float, SpeedMult, 1.0f)

    ResHandle       GetModel1Handle() const             { return g_ResourceManager.Register(GetModel1Path(), RES_MODEL); }
    ResHandle       GetModel2Handle() const             { return g_ResourceManager.Register(GetModel2Path(), RES_MODEL); }

    GAME_SHARED_API int     GetAdjustedAmmoCount() const;

    GAME_SHARED_API float   GetAmmoPercent() const;
    GAME_SHARED_API float   GetCooldownPercent() const;

    virtual void            DoRangedAttack()            {}
};
//=============================================================================

#endif //__I_INVENTORYITEM_H__
