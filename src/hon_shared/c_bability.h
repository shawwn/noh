// (C)2008 S2 Games
// c_bability.h
//
//=============================================================================
#ifndef __C_BABILITY_H__
#define __C_BABILITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_behavior.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class IEntityTool;

const uint BSR_CAST         (BIT(8));
const uint BSR_SECONDARY    (BIT(9));
//=============================================================================

//=============================================================================
// CBAbility
//=============================================================================
class CBAbility : public IBehavior
{
private:
    // Parameters calculated during setup frame
    float           m_fDistSq;      
    CVec2f          m_v2TargetPosition;
    CVec2f          m_v2ApproachPosition;
    float           m_fRange;
    bool            m_bAtGoal;
    bool            m_bSight;
    PoolHandle      m_hRangePath;

    int             m_iInventorySlot;
    IEntityTool*    m_pAbility;

    bool            IsTargeted();
    bool            IsAutoCast();

    CBAbility();

    void            ThinkFramePrimary();
    void            ThinkFrameAutoCast();

    void            BeginCast(const CVec3f &v3TargetPosition);

    bool            ActionFramePrimary(CVec3f &v3TargetPosition);
    bool            ActionFrameAutoCast(CVec3f &v3TargetPosition);

public:
    ~CBAbility() {}
    CBAbility(int iInventorySlot, bool bSecondary) :
    IBehavior(EBT_ABILITY),
    m_iInventorySlot(iInventorySlot),
    m_bAtGoal(false),
    m_hRangePath(INVALID_POOL_HANDLE)
    {
        if (bSecondary)
            SetFlag(BSR_SECONDARY);
    }

    virtual void        CopyFrom( const IBehavior* pBehavior );
    virtual IBehavior*  Clone(CBrain* pNewBrain, IUnitEntity* pNewSelf) const;

    virtual bool    Validate();
    virtual void    Update();
    virtual void    BeginBehavior();
    virtual void    ThinkFrame();
    virtual void    MovementFrame();
    virtual void    ActionFrame();
    virtual void    CleanupFrame();
    virtual void    EndBehavior();

    virtual bool    IsChanneling() const;
    virtual bool    ShouldReset() const     { return (GetFlags() & BSR_CAST) == 0 && IBehavior::ShouldReset(); }
};
//=============================================================================

#endif //__C_BABILITY_H__
