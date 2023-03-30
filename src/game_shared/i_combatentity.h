// (C)2007 S2 Games
// i_combatentity.h
//
//=============================================================================
#ifndef __I_COMBATENTITY_H__
#define __I_COMBATENTITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_visualentity.h"
#include "i_inventoryitem.h"
#include "c_meleeattackevent.h"
#include "c_skillactivateevent.h"
#include "c_spellactivateevent.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const int PLAYER_ACTION_IDLE            (0);
const int PLAYER_ACTION_BLOCK           (BIT(1));
const int PLAYER_ACTION_QUICK_ATTACK    (BIT(2));
const int PLAYER_ACTION_STRONG_ATTACK   (BIT(3));
const int PLAYER_ACTION_SKILL_ATTACK    (BIT(4));
const int PLAYER_ACTION_SKILL           (BIT(5));
const int PLAYER_ACTION_IMMOBILE        (BIT(6));
const int PLAYER_ACTION_STUNNED         (BIT(7));
const int PLAYER_ACTION_GUN_SPINUP      (BIT(8));
const int PLAYER_ACTION_GUN_FIRE        (BIT(9));
const int PLAYER_ACTION_GUN_WARM        (BIT(10));
const int PLAYER_ACTION_GUN_COOLDOWN    (BIT(11));
const int PLAYER_ACTION_GUN_CHARGE      (BIT(12));
const int PLAYER_ACTION_GUN_CHARGED     (BIT(13));
const int PLAYER_ACTION_GUN_RELOAD      (BIT(14));
const int PLAYER_ACTION_GUN_DELAY       (BIT(15));
const int PLAYER_ACTION_SPELL           (BIT(16));
const int PLAYER_ACTION_SACRIFICING     (BIT(17));
const int PLAYER_ACTION_TRAJECTORY      (BIT(19));
const int PLAYER_ACTION_UNBLOCK         (BIT(20));
const int PLAYER_ACTION_JUMP_ATTACK     (BIT(21));

const int PLAYER_ACTION_ATTACK          (PLAYER_ACTION_QUICK_ATTACK | PLAYER_ACTION_STRONG_ATTACK | PLAYER_ACTION_SKILL_ATTACK | PLAYER_ACTION_JUMP_ATTACK);

const int PLAYER_ACTION_IDLE_FLAGS      (PLAYER_ACTION_UNBLOCK);
//=============================================================================

//=============================================================================
// ICombatEntity
//=============================================================================
class ICombatEntity : public IVisualEntity
{
private:
    ICombatEntity();
    static vector<SDataField>*  s_pvFields;

protected:
    START_ENTITY_CONFIG(IVisualEntity)
        DECLARE_ENTITY_CVAR(float, BoundsRadius)
        DECLARE_ENTITY_CVAR(float, BoundsHeight)
        DECLARE_ENTITY_CVAR(float, ViewHeight)
        DECLARE_ENTITY_CVAR(float, Speed)
        DECLARE_ENTITY_CVAR(float, MaxMana)
        DECLARE_ENTITY_CVAR(float, ManaRegenRate)
        DECLARE_ENTITY_CVAR(float, MaxStamina)
        DECLARE_ENTITY_CVAR(float, Armor)
        DECLARE_ENTITY_CVAR(tstring, Inventory0)
        DECLARE_ENTITY_CVAR(tstring, Inventory1)
        DECLARE_ENTITY_CVAR(tstring, Inventory2)
        DECLARE_ENTITY_CVAR(tstring, Inventory3)
        DECLARE_ENTITY_CVAR(tstring, Inventory4)
        DECLARE_ENTITY_CVAR(tstring, Inventory5)
        DECLARE_ENTITY_CVAR(tstring, Inventory6)
        DECLARE_ENTITY_CVAR(tstring, Inventory7)
        DECLARE_ENTITY_CVAR(tstring, Inventory8)
        DECLARE_ENTITY_CVAR(tstring, Inventory9)
        DECLARE_ENTITY_CVAR(int, DefaultInventorySlot)
        DECLARE_ENTITY_CVAR(bool, IsSiege)
        DECLARE_ENTITY_CVAR(bool, IsHellbourne)
        DECLARE_ENTITY_CVAR(bool, CanBuild)
        DECLARE_ENTITY_CVAR(float, BuildingRepairRate)
        DECLARE_ENTITY_CVAR(float, SiegeRepairRate)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    float                       m_fFov;
    float                       m_fBaseFov;

    // Stats
    float                       m_fStamina;
    float                       m_fMana;

    // Actions
    int                         m_iCurrentAction;
    uint                        m_uiCurrentActionEndTime;
    uint                        m_uiCurrentActionStartTime;
    uint                        m_uiLastActionTime;
    uint                        m_uiLastMeleeAttackTime;
    uint                        m_uiLastMeleeAttackLength;
    byte                        m_yAttackSequence;

    CMeleeAttackEvent           m_attack;
    CSkillActivateEvent         m_skillActivate;
    CSpellActivateEvent         m_spellActivate;

    // Inventory
    byte                        m_ySelectedItem;
    
    CVec3f                      m_v3ViewAngles;
    uint                        m_uiSpellTargetIndex;
    uint                        m_uiTargetIndex;

    float               TiltTrace(const CVec3f &v3Start, const CVec3f &v3End);

    void                SetTerrainTypeCvar();

public:
    virtual ~ICombatEntity();
    ICombatEntity(CEntityConfig *pConfig);

    virtual bool        IsCombat() const                    { return true; }

    virtual float       GetFov() const                      { return m_fFov; }
    void                SetFov(float fFov)                  { m_fFov = fFov; }
    virtual float       GetBaseFov() const                  { return m_fBaseFov; }
    void                SetBaseFov(float fFov)              { m_fBaseFov = fFov; }
    float               GetFovScale() const                 { return (m_fBaseFov > 0.0f) ? m_fFov / m_fBaseFov : 0.0f; }

    const CVec3f&       GetViewAngles() const               { return m_v3ViewAngles; }
    void                SetViewAngles(const CVec3f &v3Angles)   { m_v3ViewAngles = v3Angles; }
    void                SetTargetIndex(uint uiIndex)        { m_uiTargetIndex = uiIndex; }
    uint                GetTargetIndex() const              { return m_uiTargetIndex; }
    uint                GetSpellTargetIndex() const         { return m_uiSpellTargetIndex; }

    // Attributes
    GAME_SHARED_API virtual float   GetAttributeBoost(int iAttribute) const;
    float                   GetAttackSpeed(bool bApplyAttributes = true) const;

    virtual float           GetBaseMaxHealth() const                            { return m_pEntityConfig->GetMaxHealth(); }
    GAME_SHARED_API float   GetMaxHealth() const;
    virtual GAME_SHARED_API float   GetHealthRegen() const;
    void                    RegenerateHealth(float fFrameTime);
    virtual float           Damage(float fDamage, int iFlags, IVisualEntity *pAttacker = NULL, ushort unDamagingObjectID = INVALID_ENT_TYPE, bool bFeedback = true);

    virtual float           GetBaseArmor() const                                { return m_pEntityConfig->GetArmor(); }
    GAME_SHARED_API float   GetArmor() const;
    GAME_SHARED_API float   GetArmorDamageReduction(float fArmor) const;

    float                   GetViewHeight() const                               { return m_pEntityConfig->GetViewHeight(); }
    GAME_SHARED_API float   GetSpeed() const;

    virtual float           GetBaseSpeed() const                                { return m_pEntityConfig->GetSpeed(); }

    virtual float           GetBaseMaxStamina() const                           { return m_pEntityConfig->GetMaxStamina(); }
    GAME_SHARED_API float   GetMaxStamina() const;
    virtual GAME_SHARED_API float   GetBaseStaminaRegen() const;
    GAME_SHARED_API virtual float   GetStaminaRegen(int iMoveState) const;
    bool                    IsExhausted() const;
    float                   GetStamina() const                                  { return m_fStamina; }
    float                   GetStaminaPercent() const                           { return m_fStamina / GetMaxStamina(); }
    virtual void            RegenerateStamina(float fFrameTime, int iMoveState);
    virtual void            DrainStamina(float fAmount)                         { m_fStamina = MAX(m_fStamina - fAmount, 0.0f); }
    void                    SetStamina(float fValue)                            { m_fStamina = CLAMP(fValue, 0.0f, GetMaxStamina()); }
    void                    GiveStamina(float fValue)                           { m_fStamina = CLAMP(m_fStamina + fValue, 0.0f, GetMaxStamina()); }

    virtual float                   GetBaseMaxMana() const                  { return m_pEntityConfig->GetMaxMana(); }
    virtual GAME_SHARED_API float   GetMaxMana() const;
    float                           GetManaPercent() const                  { return GetMana() / GetMaxMana(); }
    virtual float                   GetMana() const                         { return m_fMana; }
    void                            SetMana(float fValue)                   { m_fMana = CLAMP(fValue, 0.0f, GetMaxMana()); }
    void                            GiveMana(float fValue)                  { m_fMana = CLAMP(m_fMana + fValue, 0.0f, GetMaxMana()); }
    virtual bool                    SpendMana(float fCost)                  { if (fCost > m_fMana) return false; else m_fMana -= fCost; return true; }
    virtual float                   GetBaseManaRegen() const                { return m_pEntityConfig->GetManaRegenRate(); }
    GAME_SHARED_API float           GetManaRegen() const;
    void                            RegenerateMana(float fFrameTime);
    
    float                           GetSpellResistance() const;
    float                           GetSkillResistance() const;

    GAME_SHARED_API void    SetAction(int iAction, uint uiEndTime);
    void                    AddAction(int iAction)                          { m_iCurrentAction |= iAction; }
    uint                    GetActionTime()                                 { return m_uiCurrentActionEndTime; }
    uint                    GetActionStartTime()                            { return m_uiCurrentActionStartTime; }
    int                     GetAction() const                               { return m_iCurrentAction; }
    bool                    IsIdle() const                                  { return (m_iCurrentAction & ~PLAYER_ACTION_IDLE_FLAGS) == PLAYER_ACTION_IDLE; }
    GAME_SHARED_API void    Stun(uint uiEndTime);

    virtual void            GiveExperience(float fExperience, const CVec3f &v3Pos)  {}
    virtual void            GiveExperience(float fExperience)                       {}

    // Network
    virtual void            Baseline();
    virtual void            GetSnapshot(CEntitySnapshot &snapshot) const;
    virtual bool            ReadSnapshot(CEntitySnapshot &snapshot);
    GAME_SHARED_API static const vector<SDataField>&    GetTypeVector();

    static void             ClientPrecache(CEntityConfig *pConfig);
    static void             ServerPrecache(CEntityConfig *pConfig);

    virtual bool            ServerFrame();

    // Actions
    virtual bool            ActivatePrimary(int iSlot, int iButtonStatus)   { if (GetItem(iSlot) != NULL) return GetItem(iSlot)->ActivatePrimary(iButtonStatus); return false; }
    virtual bool            ActivateSecondary(int iSlot, int iButtonStatus) { if (GetItem(iSlot) != NULL) return GetItem(iSlot)->ActivateSecondary(iButtonStatus); return false; }
    virtual bool            ActivateTertiary(int iSlot, int iButtonStatus)  { if (GetItem(iSlot) != NULL) return GetItem(iSlot)->ActivateTertiary(iButtonStatus); return false; }
    virtual bool            Cancel(int iSlot, int iButtonStatus)            { if (GetItem(iSlot) != NULL) return GetItem(iSlot)->Cancel(iButtonStatus); return false; }
    virtual bool            UseItem(int iSlot, int iAmount = 1);
    void                    DoAttack(CMeleeAttackEvent &attack);
    void                    DoRangedAttack();
    void                    SetLastMeleeAttackTime(uint uiTime)                 { m_uiLastMeleeAttackTime = uiTime; }
    void                    SetLastMeleeAttackLength(uint uiLength)             { m_uiLastMeleeAttackLength = uiLength; }
    uint                    GetLastMeleeAttackTime() const                      { return m_uiLastMeleeAttackTime; }
    uint                    GetLastMeleeAttackLength() const                    { return m_uiLastMeleeAttackLength; }
    void                    IncrementAttackSequence()                           { ++m_yAttackSequence; }
    void                    StartQuickAttackSequence()                          { m_yAttackSequence = 0; }
    void                    StartJumpAttackSequence()                           { m_yAttackSequence = 0x80; }
    void                    StartStrongAttackSequence()                         { m_yAttackSequence = 0x40; }
    uint                    GetAttackSequence() const                           { return m_yAttackSequence & ~0xC0; }
    bool                    IsQuickAttackSequence() const                       { return (m_yAttackSequence & 0XC0) == 0; }
    bool                    IsStrongAttackSequence() const                      { return (m_yAttackSequence & 0X40) == 0x40; }
    bool                    IsJumpAttackSequence() const                        { return (m_yAttackSequence & 0X80) == 0x80; }

    CMeleeAttackEvent&      GetAttackEvent()                                    { return m_attack; }
    CSkillActivateEvent&    GetSkillActivateEvent()                             { return m_skillActivate; }
    CSpellActivateEvent&    GetSpellActivateEvent()                             { return m_spellActivate; }
    
    virtual CVec3f          GetTargetPosition(float fRange, float fMinRange = 0.0f);

    // Inventory
    GAME_SHARED_API virtual int     GiveItem(int iSlot, ushort unID, bool bEnabled = true);
    IInventoryItem*                 GetCurrentItem()                            { return GetItem(m_ySelectedItem); }
    int                             GetSelectedItem() const                     { return m_ySelectedItem; }
    GAME_SHARED_API void            SelectItem(int iSlot);
    void                            UnselectItem()                              { m_ySelectedItem = NO_SELECTION; }
    bool                            UseAmmo(int iSlot, int iAmount = 1);
    GAME_SHARED_API void            SetAmmo(int iSlot, int iAmount);
    GAME_SHARED_API float           GetAmmoPercent(int iSlot) const;
    GAME_SHARED_API int             GetAmmoCount(int iSlot) const;
    GAME_SHARED_API int             GetTotalAmmo(int iSlot) const;
    GAME_SHARED_API bool            RefillAmmo(int iSlot = -1);
    GAME_SHARED_API bool            CanAccess(int iSlot);

    GAME_SHARED_API float           GetModifiedDamage(float fDamage);
    GAME_SHARED_API float           GetJumpAttackMinDamage(int iSlot);
    GAME_SHARED_API float           GetJumpAttackMaxDamage(int iSlot);
    GAME_SHARED_API float           GetStrongAttackMinDamage(int iSlot);
    GAME_SHARED_API float           GetStrongAttackMaxDamage(int iSlot);
    GAME_SHARED_API float           GetRangedMinDamage(int iSlot);
    GAME_SHARED_API float           GetRangedMaxDamage(int iSlot);

    virtual void                    AttachModel(const tstring &sBoneName, ResHandle hModel);

    virtual bool                    IsVisibleOnMinimap(IPlayerEntity *pPlayer, bool bLargeMap);

    virtual void                    Repair(bool bActivate);

    // Operators
    GAME_SHARED_API virtual void    Copy(const IGameEntity &B);

    ENTITY_CVAR_ACCESSOR(int, DefaultInventorySlot, 0)
    ENTITY_CVAR_ACCESSOR(bool, IsHellbourne, false)
    ENTITY_CVAR_ACCESSOR(bool, IsSiege, false)
    ENTITY_CVAR_ACCESSOR(bool, CanBuild, false)
    ENTITY_CVAR_ACCESSOR(float, BuildingRepairRate, 0.0f)
    ENTITY_CVAR_ACCESSOR(float, SiegeRepairRate, 0.0f)
};
//=============================================================================

#endif //__I_PLAYERENTITY_H__
