// (C)2007 S2 Games
// i_behavior.h
//
//=============================================================================
#ifndef __I_BEHAVIOR_H__
#define __I_BEHAVIOR_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/c_recyclepool.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class IUnitEntity;
class IActionState;
class CBrain;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
// Behavior State Result
const uint BSR_NEW          (BIT(0));
const uint BSR_MOVING       (BIT(1));
const uint BSR_END          (BIT(2));
const uint BSR_DEFAULT      (BIT(3));
const uint BSR_FORCED       (BIT(4));
const uint BSR_RESTRICTED   (BIT(5));
const uint BSR_SUCCESS      (BIT(6));
const uint BSR_SHARED       (BIT(7));

const uint  BEHAVIOR_UPDATE_MS(500u);
const float PATH_RECALC_DISTANCE(64.0f);

// Behavior type
enum EBehaviorType
{
    EBT_INVALID = 0,
    EBT_ABILITY,
    EBT_ATTACK,
    EBT_EVENT,
    EBT_FOLLOW,
    EBT_GIVEITEM,
    EBT_HOLD,
    EBT_MOVE,
    EBT_SENTRY,
    EBT_STOP,
    EBT_TOUCH,
    EBT_WANDER,

    EBT_AGGRESSIVEWANDER,
    EBT_AGGRO,
    EBT_ASSIST,
    EBT_ATTACKFOLLOW,
    EBT_ATTACKMOVE,
    EBT_DROPITEM,
    EBT_FOLLOWGUARD,
    EBT_GUARD,
    EBT_GUARDFOLLOW,
    EBT_DOUBLE_ACTIVATE_ABILITY,
};
//=============================================================================

//=============================================================================
// IBehavior
//=============================================================================
class IBehavior
{
protected:
    // Behavior
    CBrain*         m_pBrain;
    uint            m_uiFlags;

    int             m_iIssuedClientNumber;
    uint            m_uiEndTime;
    CVec2f          m_v2UpdatedGoal;
    CVec2f          m_v2PathGoal;
    PoolHandle      m_hPath;
    uint            m_uiTargetIndex;
    IUnitEntity*    m_pSelf;
    uint            m_uiLastUpdate;
    CPlane          m_plAvoidPlane;
    bool            m_bInheritMovement;
    float           m_fGoalRange;
    uint            m_uiForcedTime;
    uint            m_uiWaypointUID;
    ushort          m_unOrderEnt;
    uint            m_uiOrderEntUID;
    uint            m_uiLevel;
    CVec2f          m_v2Delta;
    uint            m_uiOrderSequence;
    bool            m_bDirectPathing;
    uint            m_uiTargetOrderDisjointSequence;

    EBehaviorType   m_eType;
    void    SetType( EBehaviorType eType )      { m_eType = eType; }

    void    FindPathToUpdatedGoal();
    void    GetMovement(CVec2f &v2Movement, float &fYawDelta, bool &bAtGoal, float &fGoalYaw);
    void    DebugRender();
    void    SmoothedRender();

    void    SetFlag(uint uiFlags)   { m_uiFlags |= uiFlags; }
    void    ClearFlag(uint uiFlags) { m_uiFlags &= ~uiFlags; }

public:
    virtual ~IBehavior();
    IBehavior( EBehaviorType type ) :
    m_eType(type),
    m_v2UpdatedGoal(V2_ZERO),
    m_v2PathGoal(V2_ZERO),
    m_uiFlags(BSR_NEW),
    m_iIssuedClientNumber(-1),
    m_hPath(INVALID_POOL_HANDLE),
    m_pBrain(nullptr),
    m_pSelf(nullptr),
    m_uiTargetIndex(INVALID_INDEX),
    m_uiLastUpdate(0),
    m_plAvoidPlane(0.0f, 0.0f, 0.0f, 0.0f),
    m_bInheritMovement(false),
    m_uiEndTime(INVALID_TIME),
    m_fGoalRange(0.0f),
    m_uiForcedTime(INVALID_TIME),
    m_uiWaypointUID(INVALID_INDEX),
    m_unOrderEnt(INVALID_ENT_TYPE),
    m_uiOrderEntUID(INVALID_INDEX),
    m_uiLevel(1),
    m_v2Delta(V2_ZERO),
    m_uiOrderSequence(uint(-1)),
    m_bDirectPathing(false),
    m_uiTargetOrderDisjointSequence(uint(-1))
    {}

    EBehaviorType   GetType() const     {   return m_eType;     }

    virtual void    CopyFrom( const IBehavior* pBehavior );

    virtual IBehavior*  Clone(CBrain* pNewBrain, IUnitEntity* pNewSelf) const = 0;

    virtual bool    Validate();                 // 
    virtual void    Update() = 0;               // 
    virtual void    BeginBehavior() = 0;        //
    virtual void    ThinkFrame() = 0;           // 
    virtual void    MovementFrame() = 0;        // 
    virtual void    ActionFrame() = 0;          // 
    virtual void    CleanupFrame() = 0;         // 
    virtual void    EndBehavior();              // 

    virtual void    Damaged(IUnitEntity *pAttacker) {}
    virtual void    Assist(IUnitEntity *pAlly, IUnitEntity *pAttacker) {}
    virtual void    Moved();
    virtual void    Aggro(IUnitEntity *pTarget, uint uiDuration, uint uiDelay, bool bReaggroBlock) {}
    virtual bool    IsIdle() const          { return false; }
    virtual bool    IsTraveling() const     { return false; }
    virtual bool    IsChanneling() const    { return false; }
    virtual uint    GetAttackTarget() const { return INVALID_INDEX; }

    void    SetSelf(IUnitEntity *pSelf)     { m_pSelf = pSelf; }
    void    SetBrain(CBrain *pBrain)        { m_pBrain = pBrain; }

    void    SetGoal(const CVec2f &v2Goal)   { m_v2UpdatedGoal = v2Goal; }
    void    ForceUpdate()                   { m_uiLastUpdate = INVALID_TIME; }
    
    const CVec2f&   GetGoal()               { return m_v2UpdatedGoal; }
    
    void    SetIssuedClientNumber(int iClient)  { m_iIssuedClientNumber = iClient; }
    int     GetIssuedClientNumber() const       { return m_iIssuedClientNumber; }

    void    SetTarget(uint uiTargetIndex)   { m_uiTargetIndex = uiTargetIndex; }
    uint    GetTarget() const               { return m_uiTargetIndex; }
    
    void    SetMoving(bool bMoving)         { if (bMoving) m_uiFlags |= BSR_MOVING; else m_uiFlags &= ~BSR_MOVING; }
    bool    GetMoving() const               { return (m_uiFlags & BSR_MOVING) != 0; }

    void    SetDefault(bool bDefault)       { if (bDefault) m_uiFlags |= BSR_DEFAULT; else m_uiFlags &= ~BSR_DEFAULT; }
    bool    GetDefault() const              { return (m_uiFlags & BSR_DEFAULT) != 0; }

    void    SetShared(bool bShared)         { if (bShared) m_uiFlags |= BSR_SHARED; else m_uiFlags &= ~BSR_SHARED; }
    bool    GetShared() const               { return (m_uiFlags & BSR_SHARED) != 0; }

    void    SetEndTime(uint uiEndTime)      { m_uiEndTime = uiEndTime; }

    void    SetForced(bool bValue)          { if (bValue) SetFlag(BSR_FORCED); else ClearFlag(BSR_FORCED); }
    void    SetForcedTime(uint uiEndTime)   { m_uiForcedTime = uiEndTime; }

    bool    IsForced() const                { return GetFlags() & BSR_FORCED && m_uiForcedTime > Game.GetGameTime(); }

    void    SetRestricted(bool bValue)      { if (bValue) SetFlag(BSR_RESTRICTED); else ClearFlag(BSR_RESTRICTED); }

    bool    IsRestricted() const            { return (GetFlags() & BSR_RESTRICTED) != 0; }

    uint    GetFlags() const                { return m_uiFlags; }

    void    Reset()                         { m_uiFlags = BSR_NEW; }
    virtual bool    ShouldReset() const     { return (m_uiFlags & BSR_END) == 0; }
    
    virtual PoolHandle  GetPath() const     { return m_hPath; }

    void    SetInheritMovement(bool bInheritMovement)   { m_bInheritMovement = bInheritMovement; }
    bool    GetInheritMovement() const                  { return m_bInheritMovement; }

    void    SetWaypointUID(uint uiWaypointUID)          { m_uiWaypointUID = uiWaypointUID; }

    void    SetOrderEnt(ushort unOrderEnt)              { m_unOrderEnt = unOrderEnt; }
    void    SetOrderEntUID(uint uiOrderEntUID)          { m_uiOrderEntUID = uiOrderEntUID; }
    
    void    SetLevel(uint uiLevel)                      { m_uiLevel = uiLevel; }
    uint    GetLevel() const                            { return m_uiLevel; }

    void            SetDelta(const CVec2f &v2Delta)     { m_v2Delta = v2Delta; }
    const CVec2f&   GetDelta() const                    { return m_v2Delta; }

    void            SetOrderSequence(uint uiOrderSequence)  { m_uiOrderSequence = uiOrderSequence; }
    uint            GetOrderSequence() const                { return m_uiOrderSequence; }

    void            SetDirectPathing(bool b)            { m_bDirectPathing = b; }
    bool            GetDirectPathing() const            { return m_bDirectPathing; }

    void            SetTargetOrderDisjointSequence(uint uiOrderDisjointSequence)    { m_uiTargetOrderDisjointSequence = uiOrderDisjointSequence; }
    uint            GetTargetOrderDisjointSequence() const                  { return m_uiTargetOrderDisjointSequence; }

    virtual void    SetValue0(float fValue0)            {}
    
    IOrderEntity*   GetOrder() const
    {
        if (m_uiOrderEntUID == INVALID_INDEX)
            return nullptr;

        IGameEntity *pEntity(Game.GetEntityFromUniqueID(m_uiOrderEntUID));
        if (pEntity != nullptr && pEntity->IsOrder())
            return pEntity->GetAsOrder();
        else
            return nullptr;
    }
};
//=============================================================================

#endif //__I_BEHAVIOR_H__
