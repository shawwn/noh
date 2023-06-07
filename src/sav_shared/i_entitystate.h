// (C)2006 S2 Games
// i_entitystate.h
//
//=============================================================================
#ifndef __I_ENTITYSTATE_H__
#define __I_ENTITYSTATE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gameentity.h"
#include "../k2/c_modifier.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CMeleeAttackEvent;
class IVisualEntity;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EModType
{
    MOD_NULL,
    MOD_MANA,
    MOD_INCOME,
    MOD_EVASION,
    MOD_ARMOR,
    MOD_HEALTH,
    MOD_SPEED,
    MOD_STAMINA,
    MOD_DAMAGE,
    MOD_GUNMANACOST,
    MOD_SPELLRESIST,
    MOD_SKILLRESIST,

    //Regen mods
    MOD_REGEN_NULL,
    MOD_REGEN_HEALTH,
    MOD_REGEN_INCOME,
    MOD_REGEN_MANA,
    MOD_REGEN_STAMINA,

    NUM_MODS
};

const ushort INVALID_ENTITY_STATE(USHRT_MAX);
//=============================================================================

//=============================================================================
// IEntityState
//=============================================================================
class IEntityState : public IGameEntity
{
private:
    static vector<SDataField>   *s_pvFields;

    IEntityState();

protected:
    // Cvar settings
    START_ENTITY_CONFIG(IGameEntity)
        DECLARE_ENTITY_CVAR(tstring, Name)
        DECLARE_ENTITY_CVAR(tstring, Description)
        DECLARE_ENTITY_CVAR(tstring, IconPath)
        DECLARE_ENTITY_CVAR(tstring, EffectPath)
        DECLARE_ENTITY_CVAR(tstring, AnimName)
        DECLARE_ENTITY_CVAR(bool, IsDebuff)
        DECLARE_ENTITY_CVAR(bool, DisplayState)
        DECLARE_ENTITY_CVAR(bool, IsSecret)
        DECLARE_ENTITY_CVAR(tstring, Skin)
        DECLARE_ENTITY_CVAR(tstring, ModelPath)
        DECLARE_ENTITY_CVAR(bool, Stack)
        DECLARE_ENTITY_CVAR(bool, DisplayTimer)
        DECLARE_ENTITY_CVAR(bool, DisableSkills)
        DECLARE_ENTITY_CVAR(bool, IsStealth)
        DECLARE_ENTITY_CVAR(bool, IsInvulnerable)
        DECLARE_ENTITY_CVAR(bool, IsIntangible)
        DECLARE_ENTITY_CVAR(bool, IsDispellable)
        DECLARE_ENTITY_CVAR(bool, RemoveOnDamage)
        DECLARE_ENTITY_CVAR(float, AssistCredit)
        DECLARE_ENTITY_CVAR(bool, AllowDash)
    END_ENTITY_BASE_CONFIG

    CEntityConfig*  m_pEntityConfig;

    uint            m_uiOwnerIndex;
    uint            m_uiInflictorIndex;
    ushort          m_unDamageID;

    bool            m_bValid;

    uint            m_uiStartTime;
    uint            m_uiDuration;
    uint            m_uiLastIncomeTime;

    ResHandle       m_hModel;
    ushort          m_unDisguiseItem;
    int             m_iDisguiseTeam;
    int             m_iDisguiseClient;

    FloatMod        m_modSpeed;
    FloatMod        m_modAttackSpeed;
    FloatMod        m_modDamage;
    FloatMod        m_modHealth;
    FloatMod        m_modMana;
    FloatMod        m_modStamina;
    FloatMod        m_modHealthRegen;
    FloatMod        m_modManaRegen;
    FloatMod        m_modStaminaRegen;
    FloatMod        m_modArmor;
    FloatMod        m_modAmmo;
    FloatMod        m_modGunManaCost;
    FloatMod        m_modSpellResist;
    FloatMod        m_modSkillResist;
    UShortMod       m_modIncome;
    UShortMod       m_modIncomeGen;

public:
    virtual ~IEntityState();
    IEntityState(CEntityConfig *pConfig);

    virtual bool        IsState() const             { return true; }

    // Network
    GAME_SHARED_API virtual void    Baseline();
    GAME_SHARED_API virtual void    GetSnapshot(CEntitySnapshot &snapshot) const;
    GAME_SHARED_API virtual bool    ReadSnapshot(CEntitySnapshot &snapshot);

    static const vector<SDataField>&    GetTypeVector();

    virtual int         GetPrivateClient();

    static void         ClientPrecache(CEntityConfig *pConfig);
    static void         ServerPrecache(CEntityConfig *pConfig);

    void            SetOwner(uint uiIndex)                  { m_uiOwnerIndex = uiIndex; }
    uint            GetOwner()                              { return m_uiOwnerIndex; }
    
    void            SetInflictor(uint uiIndex)              { m_uiInflictorIndex = uiIndex; }
    uint            GetInflictor()                          { return m_uiInflictorIndex; }

    ResHandle       GetModelHandle() const                  { return m_hModel; }
    void            SetModelHandle(ResHandle hModel)        { m_hModel = hModel; }

    void            Invalidate()                            { m_bValid = false; }
    bool            IsValid() const                         { return m_bValid; }

    void            SetStartTime(uint uiTime)               { m_uiStartTime = uiTime; }
    uint            GetStartTime() const                    { return m_uiStartTime; }

    void            SetDuration(uint uiTime)                { m_uiDuration = uiTime; }
    uint            GetDuration() const                     { return m_uiDuration; }

    void            SetDamageID(ushort unID)                { m_unDamageID = unID; }
    ushort          GetDamageID() const                     { return m_unDamageID; }

    uint            GetExpireTime() const                   { if (m_uiStartTime != INVALID_TIME && m_uiDuration != INVALID_TIME) return m_uiStartTime + m_uiDuration; else return INVALID_TIME; }

    virtual void    Activated();
    virtual void    Expired();
    virtual void    Spawn();
    virtual void    StateFrame();
    virtual void    DoAttack(CMeleeAttackEvent &attack)     {}
    virtual void    DoRangedAttack()                        {}
    virtual float   OwnerDamaged(float fDamage, int iFlags, IVisualEntity *pAttacker);
    virtual void    ModifyCamera(CCamera &camera)           {}

    virtual bool    IsDisguised() const                     { return false; }

    int             GetDisguiseTeam() const                 { return m_iDisguiseTeam; }
    int             GetDisguiseClient() const               { return m_iDisguiseClient; }
    ushort          GetDisguiseItem() const                 { return m_unDisguiseItem; }

    void            SetDisguise(int iTeam, int iClient, ushort unItem)  { m_iDisguiseTeam = iTeam; m_iDisguiseClient = iClient; m_unDisguiseItem = unItem; }

    bool            IsBuff() const                          { return !GetIsDebuff(); }
    bool            IsDebuff() const                        { return GetIsDebuff(); }

    virtual bool    IsMatch(ushort unType);

    const FloatMod& GetSpeedMod() const                     { return m_modSpeed; }
    const FloatMod& GetAttackSpeedMod() const               { return m_modAttackSpeed; }
    const FloatMod& GetDamageMod() const                    { return m_modDamage; }
    const FloatMod& GetHealthMod() const                    { return m_modHealth; }
    const FloatMod& GetManaMod() const                      { return m_modMana; }
    const FloatMod& GetStaminaMod() const                   { return m_modStamina; }
    const FloatMod& GetHealthRegenMod() const               { return m_modHealthRegen; }
    const FloatMod& GetManaRegenMod() const                 { return m_modManaRegen; }
    const FloatMod& GetStaminaRegenMod() const              { return m_modStaminaRegen; }
    const FloatMod& GetArmorMod() const                     { return m_modArmor; }
    const FloatMod& GetAmmoMod() const                      { return m_modAmmo; }
    const FloatMod& GetGunManaCostMod() const               { return m_modGunManaCost; }
    const FloatMod& GetSpellResistMod() const               { return m_modSpellResist; }
    const FloatMod& GetSkillResistMod() const               { return m_modSkillResist; }
    const UShortMod&    GetIncomeMod() const                { return m_modIncome; }
    const UShortMod&    GetIncomeGenerationMod() const      { return m_modIncomeGen; }

    void    SetSpeedMod(const FloatMod &modSpeed)               { m_modSpeed = modSpeed; }
    void    SetAttackSpeedMod(const FloatMod &modAttackSpeed)   { m_modAttackSpeed = modAttackSpeed; }
    void    SetDamageMod(const FloatMod &modDamage)             { m_modDamage = modDamage; }
    void    SetHealthMod(const FloatMod &modHealth)             { m_modHealth = modHealth; }
    void    SetManaMod(const FloatMod &modMana)                 { m_modMana = modMana; }
    void    SetStaminaMod(const FloatMod &modStamina)           { m_modStamina = modStamina; }
    void    SetHealthRegenMod(const FloatMod &modHealthRegen)   { m_modHealthRegen = modHealthRegen; }
    void    SetManaRegenMod(const FloatMod &modManaRegen)       { m_modManaRegen = modManaRegen; }
    void    SetStaminaRegenMod(const FloatMod &modStaminaRegen) { m_modStaminaRegen = modStaminaRegen; }
    void    SetArmorMod(const FloatMod &modArmor)               { m_modArmor = modArmor; }
    void    SetAmmoMod(const FloatMod &modAmmo)                 { m_modAmmo = modAmmo; }
    void    SetGunManaCostMod(const FloatMod &modGunManaCost)   { m_modGunManaCost = modGunManaCost; }
    void    SetSpellResistMod(const FloatMod &modSpellResist)   { m_modSpellResist = modSpellResist; }
    void    SetSkillResistMod(const FloatMod &modSkillResist)   { m_modSkillResist = modSkillResist; }
    void    SetIncomeMod(const UShortMod &modIncome)            { m_modIncome = modIncome; }
    void    SetIncomeGenerationMod(const UShortMod &modIncomeGen)   { m_modIncomeGen = modIncomeGen; }

    void    SetMod(uint uiMod, const FloatMod &modValue);

    virtual ENTITY_CVAR_ACCESSOR(tstring, Name, _T(""))
    virtual ENTITY_CVAR_ACCESSOR(tstring, Description, _T(""))
    virtual ENTITY_CVAR_ACCESSOR(tstring, IconPath, _T(""))
    virtual ENTITY_CVAR_ACCESSOR(tstring, EffectPath, _T(""))
    virtual ENTITY_CVAR_ACCESSOR(tstring, AnimName, _T(""))
    virtual ENTITY_CVAR_ACCESSOR(bool, IsDebuff, false)
    virtual ENTITY_CVAR_ACCESSOR(bool, DisplayState, false)
    virtual ENTITY_CVAR_ACCESSOR(bool, IsSecret, false)
    virtual ENTITY_CVAR_ACCESSOR(tstring, Skin, _T(""))
    virtual ENTITY_CVAR_ACCESSOR(tstring, ModelPath, _T(""))
    virtual ENTITY_CVAR_ACCESSOR(bool, Stack, false)
    virtual ENTITY_CVAR_ACCESSOR(bool, DisplayTimer, true)
    virtual ENTITY_CVAR_ACCESSOR(bool, DisableSkills, false)
    virtual ENTITY_CVAR_ACCESSOR(bool, IsStealth, false)
    virtual ENTITY_CVAR_ACCESSOR(bool, IsIntangible, false)
    virtual ENTITY_CVAR_ACCESSOR(bool, IsInvulnerable, false)
    virtual ENTITY_CVAR_ACCESSOR(bool, IsDispellable, true)
    virtual ENTITY_CVAR_ACCESSOR(bool, RemoveOnDamage, false)
    virtual ENTITY_CVAR_ACCESSOR(float, AssistCredit, 0.0f)
    virtual ENTITY_CVAR_ACCESSOR(bool, AllowDash, true)
};
//=============================================================================

#endif //__I_ENTITYSTATE_H__
