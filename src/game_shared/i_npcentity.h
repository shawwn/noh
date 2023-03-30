// (C)2007 S2 Games
// i_npcentity.h
//
//=============================================================================
#ifndef __I_NPCENTITY_H__
#define __I_NPCENTITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_npcentity.h"
#include "c_npcability.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
class INpcEntity;

enum ENpcMode
{
    NPCMODE_PASSIVE,
    NPCMODE_AGGRESSIVE,
    NPCMODE_DEFENSIVE
};

enum ENpcState
{
    NPCSTATE_WAITING,
    NPCSTATE_MOVING,
    NPCSTATE_ATTACKING,
    NPCSTATE_REPAIRING
};

enum ENpcJob
{
    NPCJOB_FOLLOW,
    NPCJOB_ATTACK,
    NPCJOB_MOVE,
    NPCJOB_IDLE,
    NPCJOB_GUARDPOS,
    NPCJOB_PATROL,
    NPCJOB_REPAIR,
};

enum ENpcCommand
{
    NPCCMD_INVALID,

    NPCCMD_ATTACK,
    NPCCMD_MOVE,
    NPCCMD_STOP,
    NPCCMD_FOLLOW,
    NPCCMD_SPECIALABILITY,
    NPCCMD_RETURN,
    NPCCMD_TOGGLEAGGRO,
    NPCCMD_BANISH,
    NPCCMD_REPAIR,
    NPCCMD_PATROL,

    NUM_NPCCMDS
};

const int NPC_MOVE_IDLE         (0);
const int NPC_MOVE_FWD          (BIT(0));
const int NPC_MOVE_BACK         (BIT(1));
const int NPC_MOVE_LEFT         (BIT(2));
const int NPC_MOVE_RIGHT        (BIT(3));
const int NPC_MOVE_JUMP         (BIT(5));
const int NPC_MOVE_IMMOBILE     (BIT(6));
const int NPC_MOVE_FWD_LEFT     (NPC_MOVE_FWD | NPC_MOVE_LEFT);
const int NPC_MOVE_FWD_RIGHT    (NPC_MOVE_FWD | NPC_MOVE_RIGHT);
const int NPC_MOVE_BACK_LEFT    (NPC_MOVE_BACK | NPC_MOVE_LEFT);
const int NPC_MOVE_BACK_RIGHT   (NPC_MOVE_BACK | NPC_MOVE_RIGHT);
const int NPC_MOVE_NO_FLAGS     (0x00ff);
const int NPC_MOVE_FLAGS        (0xff00);
const int NPC_MOVE_RESTING      (BIT(13));
const int NPC_MOVE_SPRINT       (BIT(14));
const int NPC_MOVE_TIRED        (BIT(15));
const int NPC_MOVE_IGNORE_FLAGS (NPC_MOVE_RESTING);

typedef vector<CNpcAbility>             AbilityVector;
typedef vector<CNpcAbility>::iterator   AbilityIter;
typedef map<uint, float>                AggroMap;
//=============================================================================

//=============================================================================
// INpcEntity
//=============================================================================
class INpcEntity : public ICombatEntity
{
private:
    static vector<SDataField>   *s_pvFields;

protected:
    void    Accelerate(const CVec3f &v3Intent, float fAcceleration);
    void    Friction(float fFriction);

    void    MoveWalkGround();
    void    MoveWalkAir();

    bool    MoveWalk(float fFrameTime, const CVec3f &v3TargetPos, bool bContinue);

    void    AttackProjectile(const CNpcAbility &cAbility);
    void    AttackTrace(const CNpcAbility &cAbility);
    void    AttackSnap(const CNpcAbility &cAbility);
    void    AttackSelf(const CNpcAbility &cAbility);

    AbilityIter PickNextAbility();

    ResHandle           m_hDefinition;

    tstring             m_sNpcType;
    tstring             m_sNpcDescription;
    int                 m_iLevel;
    float               m_fMaxHealth;
    float               m_fMaxMana;
    float               m_fMaxStamina;
    float               m_fHealthRegen;
    float               m_fManaRegen;
    float               m_fStaminaRegen;
    float               m_fArmor;
    float               m_fSpeed;
    float               m_fExperienceReward;
    int                 m_iGoldReward;
    float               m_fAggroRadius;
    float               m_fMultiAggroProc;
    float               m_fMultiAggroRadius;
    tstring             m_sInitialJob;
    bool                m_bSoul;
    float               m_fItemDrop;
    float               m_fCommanderScale;
    float               m_fEffectScale;
    float               m_fSelectionRadius;
    tstring             m_sCommanderPortraitPath;
    int                 m_iMinimapIconSize;
    tstring             m_sMinimapIconPath;
    tstring             m_sIconPath;
    float               m_fPushMultiplier;
    float               m_fFollowDistance;

    ResHandle           m_hHitByMeleeEffectPath;
    ResHandle           m_hHitByRangedEffectPath;

    ENpcState           m_eNpcState;
    ENpcJob             m_eNpcJob;
    ENpcMode            m_eNpcMode;
    AggroMap            m_mapAggro;
    PoolHandle          m_hPath;

    uint                m_uiTargetUID;
    CVec3f              m_v3TargetPos;
    uint                m_uiNextActionTime;
    uint                m_uiRepathTime;
    CVec3f              m_v3OldTargetPos;
    
    int                 m_iCurrentMovement;
    int                 m_iMoveFlags;
    CVec3f              m_v3FaceAngles;
    
    AbilityVector       m_vAbilities;
    AbilityIter         m_itAbility;
    bool                m_bAbilityActivating;
    bool                m_bAbilityImpacted;

    tstring             m_sController;

    uint                m_uiNextSightTime;

    uint                m_uiControllerUID;

    // Jobs
    bool    Follow();
    bool    Attack();
    bool    TryRepair();
    bool    Move();
    bool    Idle();
    bool    Think(bool bMoveAggro);

    // Commands
    void    CommandAttack(uint uiIndex);
    void    CommandMove(const CVec3f &v3Pos);
    void    CommandPatrol(const CVec3f &v3Pos);
    void    CommandStop();
    void    CommandFollow(uint uiIndex);
    void    CommandSpecialAbility();
    void    CommandReturn();
    void    CommandToggleAggro();
    void    CommandBanish();
    void    CommandRepair(uint uiIndex);

    void    DisturbController();

public:
    virtual ~INpcEntity();
    INpcEntity();

    GAME_SHARED_API static const vector<SDataField>&    GetTypeVector();

    virtual bool        IsNpc() const                   { return true; }

    virtual bool        IsSelectable() const            { return true; }

    virtual bool            HasAltInfo() const          { return true; }
    virtual const tstring&  GetAltInfoName() const      { return m_sNpcType; }

    virtual const tstring&  GetEntityName() const           { return m_sNpcType; }
    virtual const tstring&  GetEntityDescription() const    { return m_sNpcDescription; }
    virtual const tstring&  GetEntityIconPath() const   { return m_sIconPath; }

    virtual float       GetBaseMaxHealth() const        { return m_fMaxHealth; }
    virtual float       GetBaseMaxMana() const          { return m_fMaxMana; }
    virtual float       GetBaseMaxStamina() const       { return m_fMaxStamina; }

    virtual float       GetBaseHealthRegen() const      { return m_fHealthRegen; }
    virtual float       GetBaseManaRegen() const        { return m_fManaRegen; }
    virtual float       GetBaseStaminaRegen() const     { return m_fStaminaRegen; }

    virtual float       GetBaseArmor() const            { return m_fArmor; }
    
    virtual void        Baseline();
    virtual void        GetSnapshot(CEntitySnapshot &snapshot) const;
    virtual bool        ReadSnapshot(CEntitySnapshot &snapshot);
    virtual void        Copy(const IGameEntity &B);

    virtual CSkeleton*  AllocateSkeleton();
    virtual void        UpdateSkeleton(bool bPose);

    virtual void        ApplyWorldEntity(const CWorldEntity &ent);

    virtual void        Spawn();
    virtual float       Damage(float fDamage, int iFlags, IVisualEntity *pAttacker = NULL, ushort unDamagingObjectID = INVALID_ENT_TYPE, bool bFeedback = true);
    virtual void        Hit(CVec3f v3Pos, CVec3f v3Angle, EEntityHitByType eHitBy = ENTITY_HIT_BY_RANGED);
    virtual void        KillReward(IGameEntity *pKiller);
    virtual void        Kill(IVisualEntity *pAttacker = NULL, ushort unKillingObjectID = INVALID_ENT_TYPE);

    virtual void        Link();
    virtual void        Unlink();

    virtual bool        ServerFrame();

    virtual bool        AddToScene(const CVec4f &v4Color, int iFlags);

    ENpcState           GetNpcState()                   { return m_eNpcState; }
    void                SetNpcState(ENpcState eState)   { m_eNpcState = eState; }

    const tstring&      GetNpcType() const              { return m_sNpcType; }
    const tstring&      GetNpcDescription() const       { return m_sNpcDescription; }
    float               GetSpeed() const                { return m_fSpeed; }
    float               GetExperienceValue() const      { return m_fExperienceReward; }
    int                 GetBounty() const               { return m_iGoldReward; }
    float               GetAggroRadius() const          { return m_fAggroRadius; }
    float               GetMultiAggroProc() const       { return m_fMultiAggroProc; }
    float               GetMultiAggroRadius() const     { return m_fMultiAggroRadius; }
    const tstring&      GetInitialJob() const           { return m_sInitialJob; }
    int                 GetLevel() const                { return m_iLevel; }

    void                SetDefinition(ResHandle hDefinition)    { m_hDefinition = hDefinition; }
    ResHandle           GetDefinition() const                   { return m_hDefinition; }

    void                SetControllerUID(uint uiControllerUID)  { m_uiControllerUID = uiControllerUID; }
    uint                GetControllerUID() const                { return m_uiControllerUID; }

    // Game setting overrides
    virtual int         GetMinimapIconSize() const      { return m_iMinimapIconSize; }
    virtual tstring     GetMinimapIconPath() const      { return m_sMinimapIconPath; }
    virtual float       GetCommanderScale() const       { return m_fCommanderScale; }
    virtual float       GetEffectScale() const          { return m_fEffectScale; }
    virtual float       GetSelectionRadius() const      { return m_fSelectionRadius; }
    virtual tstring     GetCommanderPortraitPath() const    { return m_sCommanderPortraitPath; }
    virtual float       GetPushMultiplier() const       { return m_fPushMultiplier; }

    // Combat
    float               GetAbilityRange() const;
    uint                GetNextActionTime()             { return m_uiNextActionTime; }
    GAME_SHARED_API void    AddAggro(uint uiIndex, float fAggro, bool bMulti);
    uint                GetMaxAggro();
    float               GetAggro(uint uiIndex);
    void                DecayAggro(float fFrameTime);
    bool                ShouldTarget(IGameEntity *pOther);
    float               GetFollowDistance()             { return m_fFollowDistance; }

    GAME_SHARED_API void    PlayerCommand(ENpcCommand ePetCmd, uint uiIndex, const CVec3f &v3Pos);
};
//=============================================================================

#endif //__I_NPCENTITY_H__
