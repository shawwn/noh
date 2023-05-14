// (C)2007 S2 Games
// i_behavior.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_behavior.h"

#include "i_unitentity.h"
#include "i_ActionState.h"
#include "c_asMoving.h"

#include "../k2/c_path.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_BOOL(d_printPathTimes, false);
//=============================================================================

/*====================
  IBehavior::~IBehavior
  ====================*/
IBehavior::~IBehavior()
{
    if (m_hPath != INVALID_POOL_HANDLE)
    {
        Game.FreePath(m_hPath);
        m_hPath = INVALID_POOL_HANDLE;
    }

    if (m_uiWaypointUID != INVALID_INDEX)
        Game.DeleteEntity(Game.GetGameIndexFromUniqueID(m_uiWaypointUID));
}


/*====================
  IBehavior::FindPathToUpdatedGoal
  ====================*/
void    IBehavior::FindPathToUpdatedGoal()
{
    PROFILE("IBehavior::FindPathToUpdatedGoal");

    if (m_hPath != INVALID_POOL_HANDLE)
    {
        Game.FreePath(m_hPath);
        m_hPath = INVALID_POOL_HANDLE;
    }

    if (m_pSelf == nullptr)
    {
        Console.Warn << _T("IBehavior::FindPathToUpdatedGoal() failed because its owner is invalid") << newl;
        return;
    }

    if (m_pSelf->IsImmobilized(true, false))
        return;
    
    uint uiBeginFind(0);

    if (d_printPathTimes)
        uiBeginFind = K2System.Microseconds();

    uint uiNavFlags(m_pSelf->GetFlying() ? 0 : NAVIGATION_ALL);

    if (m_pSelf->GetUnitwalking())
        uiNavFlags &= ~NAVIGATION_UNIT;
    if (m_pSelf->GetTreewalking())
        uiNavFlags &= ~NAVIGATION_TREE;
    if (m_pSelf->GetCliffwalking())
        uiNavFlags &= ~NAVIGATION_CLIFF;
    if (m_pSelf->GetBuildingwalking())
        uiNavFlags &= ~NAVIGATION_BUILDING;
    if (!m_pSelf->GetAntiwalking())
        uiNavFlags &= ~NAVIGATION_ANTI;

    if (uiNavFlags == NAVIGATION_ANTI)
        uiNavFlags = 0;

    if (m_bDirectPathing)
        uiNavFlags &= ~(NAVIGATION_UNIT | NAVIGATION_CLIFF | NAVIGATION_BUILDING);

    vector<PoolHandle> *pBlockers(nullptr);
    if (m_uiTargetIndex != INVALID_INDEX)
    {
        IUnitEntity *pTarget(Game.GetUnitEntity(m_uiTargetIndex));
        if (pTarget != nullptr && m_v2UpdatedGoal == pTarget->GetBlockPosition())
            pBlockers = &pTarget->GetPathBlockers();
    }

    m_v2PathGoal = m_v2UpdatedGoal;

    if (DistanceSq(m_pSelf->GetPosition().xy(), m_v2UpdatedGoal) > SQR(0.001f))
        m_hPath = Game.FindPath(m_pSelf->GetPosition().xy(), m_pSelf->GetBounds().GetDim(X) * 0.5f, uiNavFlags, m_v2UpdatedGoal, 0.0f, pBlockers);
    else
        return;

    if (!m_pBrain->GetMoving())
        Console.Warn << _T("FindPathToUpdatedGoal: Unexpected path created") << newl;

    if (d_printPathTimes)
    {
        uint uiCompleteFind(K2System.Microseconds());
        Console << _T("(") << m_hPath << _T(") IBehavior::FindPathToUpdatedGoal - ") << uiCompleteFind - uiBeginFind << _T("us") << newl;

        static uint uiWorst(0);
        if (uiCompleteFind - uiBeginFind > uiWorst)
            uiWorst = uiCompleteFind - uiBeginFind;

        Console << _T("Worst case thusfar: ") << uiWorst << newl;
    }

    CPath *pPath(Game.AccessPath(m_hPath));
    if (pPath == nullptr || pPath->GetWaypointCount() == 0)
    {
        if (m_hPath != INVALID_POOL_HANDLE)
        {
            Game.FreePath(m_hPath);
            m_hPath = INVALID_POOL_HANDLE;
        }

        Console.Err << _T("FindPathToUpdatedGoal: Failed to find a path") << newl;
    }
}


/*====================
  IBehavior::GetMovement
  ====================*/
void    IBehavior::GetMovement(CVec2f &v2Movement, float &fYawDelta, bool &bAtGoal, float &fGoalYaw)
{
    CVec3f v3Angles(m_pSelf->GetAngles());

    CPath *pPath(Game.AccessPath(m_hPath));
    if (pPath == nullptr || !pPath->Exists())
    {
        if (m_hPath != INVALID_POOL_HANDLE)
            Console.Err << _T("m_hPath == INVALID_POOL_HANDLE") << newl;

        bAtGoal = false;
        v2Movement = V2_ZERO;
        fYawDelta = 0.0f;
        fGoalYaw = v3Angles[YAW];
        return;
    }

    // Degenerate path
    if (pPath->GetStart() == pPath->GetGoal())
    {
        bAtGoal = true;
        v2Movement = V2_ZERO;
        fYawDelta = 0.0f;
        fGoalYaw = v3Angles[YAW];
        return;
    }

    CASMoving *pMoving(static_cast<CASMoving *>(m_pBrain->GetActionState(ASID_MOVING)));
    
    if (pMoving->IsBlocked() && m_bDirectPathing)
    {
        bAtGoal = true;
        v2Movement = V2_ZERO;
        fYawDelta = 0.0f;
        fGoalYaw = v3Angles[YAW];
        return;
    }

    float fDeltaTime(MsToSec(Game.GetFrameLength()));
    float fProjectionLength(m_pSelf->GetMoveSpeed() * fDeltaTime);
    CVec2f v2UnitPosition(m_pSelf->GetPosition().xy());
    
    CVec2f v2NearGoal(pPath->CalcNearGoal(v2UnitPosition, fProjectionLength, m_pSelf->GetBoundsRadius() + 0.5f));
    CVec2f v2DirectionOfMovement(v2NearGoal - v2UnitPosition);

    if (v2DirectionOfMovement.LengthSq() > SQR(fProjectionLength))
        v2DirectionOfMovement = Normalize(v2DirectionOfMovement) * fProjectionLength;

    v2Movement = v2DirectionOfMovement;

    if (pMoving->GetImpactPlane() != CPlane(0.0f, 0.0f, 0.0f, 0.0f))
        m_plAvoidPlane = pMoving->GetImpactPlane();

    if (m_plAvoidPlane != CPlane(0.0f, 0.0f, 0.0f, 0.0f))
    {
        if (!m_pSelf->TestSlide(v2Movement, m_pSelf->GetSlideThreshold(), m_bDirectPathing))
        {
            float fLength(v2DirectionOfMovement.Length());
            v2DirectionOfMovement.Clip(pMoving->GetImpactPlane().v3Normal.xy());
            v2DirectionOfMovement.Normalize();
            v2Movement = v2DirectionOfMovement * fLength;
        }
        else
        {
            m_plAvoidPlane = CPlane(0.0f, 0.0f, 0.0f, 0.0f);
        }
    }

    bool bAtPosition(pPath->AtGoal(v2UnitPosition, m_pSelf->GetBoundsRadius() + 0.5f + m_pSelf->GetMoveSpeed() * fDeltaTime));

    // Turn the unit to face the direction of movement as we progress
    if (bAtPosition && DistanceSq(v2UnitPosition, m_v2PathGoal) > SQR(0.001f))
        fGoalYaw = M_YawToPosition(v2UnitPosition, m_v2PathGoal);
    else if (v2DirectionOfMovement.LengthSq() >= SQR(0.001f))
        fGoalYaw = M_GetYawFromForwardVec2(v2DirectionOfMovement);
    else if (DistanceSq(v2UnitPosition, pPath->GetGoal()) > SQR(0.001f))
        fGoalYaw = M_YawToPosition(v2UnitPosition, pPath->GetGoal());
    else
        fGoalYaw = v3Angles[YAW];

    fYawDelta = M_ChangeAngle(m_pSelf->GetTurnRate() * fDeltaTime, v3Angles[YAW], fGoalYaw) - v3Angles[YAW];

    if (bAtPosition)
        bAtGoal = M_DiffAngle(v3Angles[YAW], fGoalYaw) <= m_pSelf->GetTurnRate() * fDeltaTime;
    else
        bAtGoal = false;

    if (v2Movement.LengthSq() < SQR(0.001f))
        v2Movement.Clear();
}


/*====================
  IBehavior::DebugRender
  ====================*/
void    IBehavior::DebugRender()
{
#if 0
    CPath &cPath(*Game.AccessPath(m_hPath));
    PathResult &vecInfoPath(cPath.SimpleResult());
    ushort unCount(vecInfoPath.size());

    for (ushort i(0); i+1<unCount; ++i)
    {
        CVec2f v2Src(vecInfoPath[i].Path());
        CVec2f v2End(vecInfoPath[i+1].Path());

#if 1
        CVec2f v2PosGate, v2NegGate;
        CVec2f v2DirVectors[SD_COUNT] =
        {
            CVec2f(-1.0f, 0.0f),
            CVec2f(0.0f, 1.0f),
            CVec2f(0.0f, -1.0f),
            CVec2f(1.0f, 0.0f)
        };

        v2PosGate = v2DirVectors[vecInfoPath[i].Direction()] * vecInfoPath[i].GetRadiusPositive();
        v2PosGate += v2Src;

        v2NegGate = v2DirVectors[vecInfoPath[i].Direction()] * -vecInfoPath[i].GetRadiusNegative();
        v2NegGate += v2Src;

        DebugRenderer.AddLine(
            CVec3f(v2PosGate.x, v2PosGate.y, Game.GetTerrainHeight(v2PosGate.x, v2PosGate.y)),
            CVec3f(v2NegGate.x, v2NegGate.y, Game.GetTerrainHeight(v2NegGate.x, v2NegGate.y)),
            vecInfoPath[i].Color());
#endif

        {
            CVec4f v4PathLineColor;

            v4PathLineColor = CVec4f(0.0f, 1.0f, 0.0f, 1.0f);

            DebugRenderer.AddLine(
                CVec3f(v2Src.x, v2Src.y, Game.GetTerrainHeight(v2Src.x, v2Src.y)),
                CVec3f(v2End.x, v2End.y, Game.GetTerrainHeight(v2End.x, v2End.y)),
                v4PathLineColor);
        }

    }
#endif
}


/*====================
  IBehavior::SmoothedRender
  ====================*/
void    IBehavior::SmoothedRender()
{
#if 0
    CPath &cPath(*Game.AccessPath(m_hPath));
    PathResult &vecPath(cPath.SmoothResult());
    ushort unCount(vecPath.size());

    for (ushort unIndex(0); unIndex+1< unCount; ++unIndex)
    {
        CVec2f v2Src(vecPath[unIndex].Path());
        CVec2f v2Dst(vecPath[unIndex+1].Path());

        DebugRenderer.AddLine(
            CVec3f(v2Src.x, v2Src.y, Game.GetTerrainHeight(v2Src)),
            CVec3f(v2Dst.x, v2Dst.y, Game.GetTerrainHeight(v2Dst)),
            CVec4f(1.0f, 1.0f, 1.0f, 1.0f));
    }
#endif
}

/*====================
  IBehavior::CopyFrom
  ====================*/
void    IBehavior::CopyFrom( const IBehavior* pBehavior )
{
    m_uiFlags = pBehavior->m_uiFlags;

    m_iIssuedClientNumber = pBehavior->m_iIssuedClientNumber;
    m_uiEndTime = pBehavior->m_uiEndTime;
    m_v2UpdatedGoal = pBehavior->m_v2UpdatedGoal;
    m_hPath = Game.ClonePath(pBehavior->m_hPath);
    m_uiTargetIndex = pBehavior->m_uiTargetIndex;
    m_uiLastUpdate = pBehavior->m_uiLastUpdate;
    m_plAvoidPlane = pBehavior->m_plAvoidPlane;
    m_bInheritMovement = pBehavior->m_bInheritMovement;
    m_fGoalRange = pBehavior->m_fGoalRange;
    m_uiForcedTime = pBehavior->m_uiForcedTime;
    m_uiWaypointUID = pBehavior->m_uiWaypointUID;
    m_unOrderEnt = pBehavior->m_unOrderEnt;
    m_uiOrderEntUID = pBehavior->m_uiOrderEntUID;
    m_uiLevel = pBehavior->m_uiLevel;
    m_v2Delta = pBehavior->m_v2Delta;
    m_uiOrderSequence = pBehavior->m_uiOrderSequence;
    m_bDirectPathing = pBehavior->m_bDirectPathing;
    m_uiTargetOrderDisjointSequence = pBehavior->m_uiTargetOrderDisjointSequence;
}


/*====================
  IBehavior::Validate
  ====================*/
bool    IBehavior::Validate()
{
    if (m_pBrain == nullptr || m_pSelf == nullptr || GetFlags() & BSR_END)
    {
        SetFlag(BSR_END);
        return false;
    }

    // Check for disjointing
    assert(m_uiTargetIndex == -1 || m_uiTargetOrderDisjointSequence != -1);
    IUnitEntity *pTargetUnit = Game.GetUnitEntity(m_uiTargetIndex);
    if (pTargetUnit != nullptr && m_uiTargetOrderDisjointSequence != -1 && pTargetUnit->GetOrderDisjointSequence() != m_uiTargetOrderDisjointSequence)
    {
        SetFlag(BSR_END);
        return false;
    }

    return true;
}


/*====================
  IBehavior::EndBehavior
  ====================*/
void    IBehavior::EndBehavior()
{
    if (m_hPath != INVALID_POOL_HANDLE)
    {
        Game.FreePath(m_hPath);
        m_hPath = INVALID_POOL_HANDLE;
    }
}


/*====================
  IBehavior::Moved

  Old path is no longer valid after being moved
  ====================*/
void    IBehavior::Moved()
{
    if (m_hPath != INVALID_POOL_HANDLE)
    {
        Game.FreePath(m_hPath);
        m_hPath = INVALID_POOL_HANDLE;
    }
}
