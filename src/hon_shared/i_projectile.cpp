// (C)2006 S2 Games
// i_projectile.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_projectile.h"
#include "i_unitentity.h"
#include "c_damageevent.h"

#include "../k2/s_traceinfo.h"
#include "../k2/c_world.h"
#include "../k2/c_entitysnapshot.h"
#include "../k2/c_networkresourcemanager.h"
#include "../k2/c_texture.h"
#include "../k2/c_effect.h"
#include "../k2/c_skeleton.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
EXTERN_CVAR_BOOL(d_printPathTimes);

uint                IProjectile::s_uiBaseType(ENTITY_BASE_TYPE_PROJECTILE);
uint                IProjectile::s_uiStartTime;
uint                IProjectile::s_uiFrameTime;
uint                IProjectile::s_uiElapsed;

DEFINE_ENTITY_DESC(IProjectile, 1)
{
    s_cDesc.pFieldTypes = K2_NEW(ctx_Game,   TypeVector)();
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_unVisibilityFlags"), TYPE_SHORT, 16, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiLevel"), TYPE_INT, 5, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_yStatus"), TYPE_CHAR, 3, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_v3Position"), TYPE_DELTAPOS3D, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_v3Angles[PITCH]"), TYPE_ANGLE8, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_v3Angles[YAW]"), TYPE_ANGLE8, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiOriginTime"), TYPE_INT, 32, 0));
}
//=============================================================================

/*====================
  IProjectile::~IProjectile
  ====================*/
IProjectile::~IProjectile()
{
    if (m_hPath != INVALID_POOL_HANDLE)
    {
        Game.FreePath(m_hPath);
        m_hPath = INVALID_POOL_HANDLE;
    }
}


/*====================
  IProjectile::IProjectile
  ====================*/
IProjectile::IProjectile() :
m_uiDirectDamageEntityIndex(INVALID_INDEX),

m_uiLevel(1),
m_uiCharges(0),
m_uiCreationTime(0),
m_uiOriginTime(0),
m_uiLastUpdateTime(INVALID_TIME),
m_uiOwnerIndex(INVALID_INDEX),
m_bReachedTarget(false),
m_bForceImpact(false),
m_fDamageRadius(0.0f),
m_hPath(INVALID_POOL_HANDLE),
m_uiTargetEntityUID(INVALID_INDEX),
m_uiTargetDisjointSequence(uint(-1)),
m_v3TargetPos(V3_ZERO),
m_bIgnoreTargetOffset(false),
m_uiTargetScheme(0),
m_uiEffectType(0),
m_bIgnoreInvulnerable(false),

m_v2TravelPosition(V2_ZERO),
m_v2CurveAxis(V2_ZERO),
m_fCurveVelocity(0.0f),
m_fCurvePos(0.0f),

m_uiBounceCount(0),
m_uiReturnCount(0),
m_uiRedirectCount(0),
m_uiTotalTouchCount(0),
m_hAttackImpactEffect(INVALID_RESOURCE),
m_uiProxyUID(INVALID_INDEX),
m_fParam(0.0f),
m_bCliffTouched(false)
{
}


/*====================
  IProjectile::Baseline
  ====================*/
void    IProjectile::Baseline()
{
    m_unVisibilityFlags = 0;
    m_uiLevel = 1;
    m_yStatus = ENTITY_STATUS_ACTIVE;
    m_v3Position.Clear();
    m_v2TravelPosition.Clear();
    m_v2CurveAxis.Clear();
    m_fCurveVelocity = 0.0f;
    m_fCurvePos = 0.0f;
    m_v3Angles = V3_ZERO;
    m_uiOriginTime = 0;
}


/*====================
  IProjectile::GetSnapshot
  ====================*/
void    IProjectile::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
    snapshot.WriteField(m_unVisibilityFlags);
    snapshot.WriteField(m_uiLevel);
    snapshot.WriteField(m_yStatus);

    if (uiFlags & SNAPSHOT_HIDDEN)
    {
        snapshot.WriteDeltaPos3D(CVec3f(0.0f, 0.0f, 0.0f));
        snapshot.WriteAngle8(0.0f);
        snapshot.WriteAngle8(0.0f);
    }
    else
    {
        snapshot.WriteDeltaPos3D(m_v3Position);
        snapshot.WriteAngle8(m_v3Angles[PITCH]);
        snapshot.WriteAngle8(m_v3Angles[YAW]);
    }
    
    snapshot.WriteField(m_uiOriginTime);
}


/*====================
  IProjectile::ReadSnapshot
  ====================*/
bool    IProjectile::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
    snapshot.ReadField(m_unVisibilityFlags);
    snapshot.ReadField(m_uiLevel);
    snapshot.ReadField(m_yStatus);
    snapshot.ReadDeltaPos3D(m_v3Position);
    snapshot.ReadAngle8(m_v3Angles[PITCH]);
    snapshot.ReadAngle8(m_v3Angles[YAW]);
    snapshot.ReadField(m_uiOriginTime);

    Validate();
    
    return true;
}


/*====================
  IProjectile::AllocateSkeleton
  ====================*/
CSkeleton*  IProjectile::AllocateSkeleton()
{
    return m_pSkeleton = K2_NEW(ctx_Game, CSkeleton);
}


/*====================
  IProjectile::Spawn
  ====================*/
void    IProjectile::Spawn()
{
    IVisualEntity::Spawn();

    m_yStatus = ENTITY_STATUS_ACTIVE;

    m_fScale = GetModelScale();

    if (GetTrailEffect() != INVALID_RESOURCE)
    {
        SetEffect(EFFECT_CHANNEL_PROJECTILE_TRAIL, GetTrailEffect());
        IncEffectSequence(EFFECT_CHANNEL_PROJECTILE_TRAIL);
    }

    StartAnimation(_T("idle"), -1);
    m_bReachedTarget = false;
    m_bForceImpact = false;
    m_uiCreationTime = m_uiOriginTime;
    m_uiLastUpdateTime = m_uiOriginTime;

    if (GetCanTurn())
        m_v3Angles = V3_ZERO;

    IGameEntity *pTarget(Game.GetEntityFromUniqueID(m_uiTargetEntityUID));
    
    // Update target position
    if (pTarget != NULL)
    {
        if (pTarget->IsUnit() && !m_bIgnoreTargetOffset)
            m_v3TargetPos = pTarget->GetAsUnit()->GetPosition() + pTarget->GetAsUnit()->GetTargetOffset();
        else if (pTarget->IsVisual())
            m_v3TargetPos = pTarget->GetAsVisual()->GetPosition();
    }

    // Set Initial velocity
    float fSpeed(GetSpeed());
    float fTime;
    if (GetUseExactLifetime() && GetLifetime() != 0)
        fTime = MsToSec(GetLifetime());
    else
        fTime = (Distance(m_v3Position.xy(), m_v3TargetPos.xy()) / fSpeed);

    m_v2TravelPosition = m_v3Position.xy();

    m_v3Velocity.Clear();
    m_v2CurveAxis.Clear();
    m_fCurveVelocity = 0.0f;
    m_fCurvePos = 0.0f;

    // calc initial gravity.
    m_v3Velocity.z = (m_v3TargetPos.z - m_v3Position.z) / fTime;
    if (GetArc() != 0.0f)
        m_v3Velocity.z += GetArc() * GetSpeed();
    else
        m_v3Velocity.z += 0.5f * GetGravity() * fTime;

    // calc initial curvature.
    if (GetCurve() != 0.0f)
    {
        CalcPerpendicular(m_v2CurveAxis);
        m_fCurveVelocity = 0.5f * GetCurve() * fTime;
    }

    if (GetFlying())
        m_v3Position.z = Game.GetCameraHeight(m_v3Position.x, m_v3Position.y) + GetFlyHeight();

    m_v3StartPosition = m_v3Position;

    if (m_v3TargetPos.xy() != m_v3Position.xy())
    {
        CVec2f v2Heading(Normalize(m_v3TargetPos.xy() - m_v3Position.xy()));
        m_v3Velocity.x = v2Heading.x * fSpeed;
        m_v3Velocity.y = v2Heading.y * fSpeed;

        if (GetCanTurn() && m_v3Velocity.LengthSq() > 0)
            m_v3Angles = M_GetAnglesFromForwardVec(Normalize(m_v3Velocity + CVec3f(m_v2CurveAxis * m_fCurveVelocity, 0.0f)));
    }

    if (Game.IsServer())
    {
        SetVisibilityFlags(Game.GetVision(GetPosition().x, GetPosition().y));

        ExecuteActionScript(ACTION_SCRIPT_SPAWN, pTarget ? pTarget->GetAsUnit() : NULL, m_v3Position);
    }
}


/*====================
  IProjectile::UpdateTargetPosition
  ====================*/
void    IProjectile::UpdateTargetPosition()
{
    IGameEntity *pTarget(Game.GetEntityFromUniqueID(m_uiTargetEntityUID));
    if (pTarget == NULL)
        return;

    if (pTarget->IsUnit())
    {
        if (pTarget->GetAsUnit()->GetDisjointSequence() == m_uiTargetDisjointSequence)
        {
            if (m_bIgnoreTargetOffset)
                m_v3TargetPos = pTarget->GetAsUnit()->GetPosition();
            else
                m_v3TargetPos = pTarget->GetAsUnit()->GetPosition() + pTarget->GetAsUnit()->GetTargetOffset();
        }
        else
        {
            m_uiTargetEntityUID = INVALID_INDEX;
            pTarget = NULL;
        }
    }
    else if (pTarget->IsVisual())
    {
        m_v3TargetPos = pTarget->GetAsVisual()->GetPosition();
    }
}


/*====================
  IProjectile::Return
  ====================*/
void    IProjectile::Return()
{
    SWAP(m_v3TargetPos, m_v3StartPosition);
}


/*====================
  IProjectile::FindPath
  ====================*/
void    IProjectile::FindPath()
{
    PROFILE("IProjectile::FindPath");

    if (m_hPath != INVALID_POOL_HANDLE)
    {
        Game.FreePath(m_hPath);
        m_hPath = INVALID_POOL_HANDLE;
    }

    uint uiBeginFind(0);
    if (d_printPathTimes)
        uiBeginFind = K2System.Microseconds();

    uint uiNavFlags(NAVIGATION_ALL);

    if (GetUnitwalking())
        uiNavFlags &= ~NAVIGATION_UNIT;
    if (GetTreewalking())
        uiNavFlags &= ~NAVIGATION_TREE;
    if (GetCliffwalking())
        uiNavFlags &= ~NAVIGATION_CLIFF;
    if (GetBuildingwalking())
        uiNavFlags &= ~NAVIGATION_BUILDING;
    
    uiNavFlags &= ~NAVIGATION_ANTI;

    vector<PoolHandle> *pBlockers(NULL);
    if (m_uiTargetEntityUID != INVALID_INDEX)
    {
        IUnitEntity *pTarget(Game.GetUnitFromUniqueID(m_uiTargetEntityUID));
        if (pTarget != NULL && m_v3TargetPos.xy() == pTarget->GetBlockPosition())
            pBlockers = &pTarget->GetPathBlockers();
    }

    if (DistanceSq(GetPosition().xy(), m_v3TargetPos.xy()) <= SQR(0.001f))
        return;
        
    m_hPath = Game.FindPath(GetPosition().xy(), GetBounds().GetDim(X) * 0.5f, uiNavFlags, m_v3TargetPos.xy(), 0.0f, pBlockers);

    if (d_printPathTimes)
    {
        uint uiCompleteFind(K2System.Microseconds());
        Console << _T("(") << m_hPath << _T(") IProjectile::FindPath() - ") << uiCompleteFind - uiBeginFind << _T("us") << newl;

        static uint uiWorst(0);
        if (uiCompleteFind - uiBeginFind > uiWorst)
            uiWorst = uiCompleteFind - uiBeginFind;

        Console << _T("Worst case thusfar: ") << uiWorst << newl;
    }

    CPath *pPath(Game.AccessPath(m_hPath));
    if (pPath == NULL || pPath->GetWaypointCount() == 0)
    {
        if (m_hPath != INVALID_POOL_HANDLE)
        {
            Game.FreePath(m_hPath);
            m_hPath = INVALID_POOL_HANDLE;
        }

        Console.Err << _T("IProjectile::FindPath() -  Failed to find a path") << newl;
    }
}


/*====================
  IProjectile::ApplyGravity
  ====================*/
void    IProjectile::ApplyGravity(float fDeltaTime, float fTimeToGoal)
{
    if (fTimeToGoal > 0.01f)
    {
        // our velocity due to our current position
        float fGravVel = (m_v3Position.z - m_v3TargetPos.z) / fTimeToGoal;

        // our current velocity (includes initial vertical gravity impulse)
        float fVel = m_v3Velocity.z;

        // our acceleration due to position + our acceleration due to current velocity
        float fAccel = -(fGravVel + fVel) / fTimeToGoal;

        // offset our position by gravity.
        m_v3Position.z += (fVel + fAccel * fDeltaTime) * fDeltaTime;
        m_v3Velocity.z += 2.0f * fAccel * fDeltaTime;
    }
    else
    {
        m_v3Position.z = m_v3TargetPos.z;
    }
}


/*====================
  IProjectile::ApplyCurvature
  ====================*/
CVec2f  IProjectile::ApplyCurvature(float fDeltaTime, float fTimeToGoal)
{
    CVec2f v2Offset(V2_ZERO);
    if (GetCurve() != 0.0f)
    {
        if (fTimeToGoal > 0.01f)
        {
            // our velocity due to our current position
            float fCurveVel = m_fCurvePos / fTimeToGoal;

            // our current velocity (includes initial perpendicular curvature impulse)
            float fVel = m_fCurveVelocity;

            // our acceleration due to position + our acceleration due to current velocity
            float fAccel = -(fCurveVel + fVel) / fTimeToGoal;

            // offset our position by gravity.
            float fDeltaCurvePos = (fVel + fAccel * fDeltaTime) * fDeltaTime;
            m_fCurvePos += fDeltaCurvePos;
            m_fCurveVelocity += 2.0f * fAccel * fDeltaTime;
            v2Offset = m_v2CurveAxis * fDeltaCurvePos;
        }
        else
        {
            // move the rest of the way.
            v2Offset = m_v2CurveAxis * -m_fCurvePos;
            m_fCurvePos = 0.0f;
        }
    }

    return v2Offset;
}


/*====================
  IProjectile::TryTouch
  ====================*/
void    IProjectile::TryTouch(float fTouchRadius, const CVec2f &v2Delta, float fDeltaTime)
{
    if (GetTouchRadiusDirAdjust())
    {
        if (ABS(v2Delta.x) > ABS(v2Delta.y))
        {
            CVec2f v2Dir(Normalize(v2Delta));

            fTouchRadius *= ABS(v2Dir.x);
        }
        else if (ABS(v2Delta.y) > 0.0f)
        {
            CVec2f v2Dir(Normalize(v2Delta));

            fTouchRadius *= ABS(v2Dir.y);
        }
    }

    CBBoxf bbBounds(CVec3f(-fTouchRadius, -fTouchRadius, -FAR_AWAY), CVec3f(fTouchRadius, fTouchRadius, FAR_AWAY));

    CVec3f v3NewPosition(m_v3Position);

    v3NewPosition.x += v2Delta.x;
    v3NewPosition.y += v2Delta.y;

    uivector vIgnore;
    bool bTraceFinished(false);

    do
    {
        uint uiTraceFlags(TRACE_PROJECTILE_TOUCH);

        if (GetTouchCliffs() && !m_bCliffTouched)
            uiTraceFlags &= ~SURF_BLOCKER;

        STraceInfo cTrace;
        Game.TraceBox(cTrace, m_v3Position, v3NewPosition, bbBounds, uiTraceFlags);
        
        if (GetTouchCliffs() && cTrace.uiEntityIndex == INVALID_INDEX && cTrace.uiSurfFlags & SURF_BLOCKER)
        {
            m_v3Position = cTrace.v3EndPos;
            ExecuteActionScript(ACTION_SCRIPT_TOUCH, NULL, m_v3Position);
            m_bCliffTouched = true;
        }
        else if (cTrace.uiEntityIndex != INVALID_INDEX)
        {
            CWorldEntity *pWorldEnt(Game.GetWorldEntity(cTrace.uiEntityIndex));
            if (pWorldEnt == NULL)
                break;

            pWorldEnt->SetSurfFlags(pWorldEnt->GetSurfFlags() | SURF_IGNORE);
            vIgnore.push_back(cTrace.uiEntityIndex);

            if (GetTouchCliffs() && pWorldEnt->GetSurfFlags() & SURF_PROP)
            {
                m_v3Position = cTrace.v3EndPos;
                ExecuteActionScript(ACTION_SCRIPT_TOUCH, NULL, m_v3Position);
                m_bCliffTouched = true;
                continue;
            }

            IUnitEntity *pUnit(Game.GetUnitEntity(pWorldEnt->GetGameIndex()));
            if (pUnit == NULL)
                continue;

            uint uiTargetUID(pUnit->GetUniqueID());
            if (GetMaxTouchesPerTarget() > 0 && m_mapTouches[uiTargetUID] >= GetMaxTouchesPerTarget())
                continue;

            m_v3Position = cTrace.v3EndPos;

            if (Game.IsValidTarget(GetTouchTargetScheme(), GetTouchEffectType(), GetOwner(), pUnit, GetTouchIgnoreInvulnerable()))
            {
                ExecuteActionScript(ACTION_SCRIPT_TOUCH, pUnit, m_v3Position);
                m_mapTouches[uiTargetUID] += 1;
                ++m_uiTotalTouchCount;
                if (GetMaxTouches() > 0 && m_uiTotalTouchCount >= GetMaxTouches())
                    break;
            }
        }
        else
        {
            m_v3Position = cTrace.v3EndPos;
            bTraceFinished = true;
        }
    }
    while (!bTraceFinished && !m_bForceImpact);

    if (m_bForceImpact)
        m_bReachedTarget = true;

    for (uivector_it it(vIgnore.begin()), itEnd(vIgnore.end()); it != itEnd; ++it)
    {
        CWorldEntity *pWorldEnt(Game.GetWorldEntity(*it));
        if (pWorldEnt == NULL)
            continue;

        pWorldEnt->SetSurfFlags(pWorldEnt->GetSurfFlags() & ~SURF_IGNORE);
    }
}


/*====================
  IProjectile::CalcDelta

  Return value is remaining time to reach goal
  ====================*/
float   IProjectile::CalcDelta(float fDeltaTime, CVec2f &v2Delta)
{
    // Calculate remaining distance
    float fDistance(0.0f);

    CPath *pPath(NULL);
    if (GetPathing())
    {
        pPath = Game.AccessPath(m_hPath);
        if (pPath == NULL)
            return 0.0f;

        fDistance = pPath->GetLength();
    }
    else
    {
        v2Delta = m_v3TargetPos.xy() - m_v2TravelPosition;

        float fDistanceSq(v2Delta.LengthSq());
        if (fDistanceSq > SQR(0.001f))
            fDistance = v2Delta.Normalize();
    }

    // Calculate speed and time remaining
    float fSpeed(GetSpeed());
    float fTimeToGoal;
    if (GetUseExactLifetime() && GetLifetime() != 0)
    {
        if (Game.GetGameTime() - m_uiCreationTime >= GetLifetime())
        {
            fTimeToGoal = fDeltaTime;
            m_bReachedTarget = true;
        }
        else
        {
            fTimeToGoal = MsToSec(GetLifetime() - (Game.GetGameTime() - m_uiCreationTime));
        }
        fSpeed = fDistance / fTimeToGoal;
    }
    else
    {
        fTimeToGoal = (fDistance / fSpeed);
    }

    if (pPath != NULL)
    {
        v2Delta = pPath->CalcNearGoal(m_v3Position.xy(), MAX(fSpeed * fDeltaTime, 1.0f), 0.0f) - m_v3Position.xy();
        v2Delta.Normalize();

        v2Delta *= fSpeed;
        m_v3Velocity.xy() = v2Delta;

        v2Delta *= fDeltaTime;

        fDistance = pPath->GetLength(m_v3Position.xy());
    }
    else
    {
        m_v3Velocity.x = v2Delta.x * fSpeed;
        m_v3Velocity.y = v2Delta.y * fSpeed;
        v2Delta *= fSpeed * fDeltaTime;
    }

    if (fDistance <= fSpeed * fDeltaTime + GetImpactDistance() + 0.001f)
    {
        if (fDistance > 0.001f)
        {
            v2Delta.Normalize();
            v2Delta *= MAX(fDistance - GetImpactDistance() - 0.001f, 0.0f);
        }
        else
        {
            fDistance = 0.0f;
            v2Delta.Clear();
        }

        m_bReachedTarget = true;
    }

    if (GetUseExactLifetime() && GetLifetime() != 0)
    {
        if (Game.GetGameTime() - m_uiCreationTime >= GetLifetime())
            return 0;
        else
            return MsToSec(GetLifetime() - (Game.GetGameTime() - m_uiCreationTime));
    }
    else
        return fSpeed == 0.0f ? 0.0f : fDistance / fSpeed;
}


/*====================
  IProjectile::CalcPerpendicular
  ====================*/
void    IProjectile::CalcPerpendicular(CVec2f &v2Perpendicular)
{
    CVec2f v2Heading( (m_v3TargetPos.xy() - m_v2TravelPosition).Direction() );
    v2Perpendicular.x = v2Heading.y;
    v2Perpendicular.y = -v2Heading.x;
}


/*====================
  IProjectile::EvaluateTrajectory
  ====================*/
void    IProjectile::EvaluateTrajectory()
{
    uint uiOwnerWorldIndex(INVALID_INDEX);
    IGameEntity *pOwner(Game.GetEntity(m_uiOwnerIndex));
    if (pOwner != NULL && pOwner->IsVisual())
        uiOwnerWorldIndex = pOwner->GetAsVisual()->GetWorldIndex();

    float fDeltaTime(MsToSec(Game.GetGameTime() - m_uiLastUpdateTime));

    UpdateTargetPosition();

    if (GetPathing() && m_hPath == INVALID_POOL_HANDLE)
    {
        FindPath();

        if (m_hPath == INVALID_POOL_HANDLE)
        {
            Kill();
            return;
        }
    }

    CVec2f v2Delta(V2_ZERO);
    float fTimeToGoal(CalcDelta(fDeltaTime, v2Delta));

    CVec2f v2CurveDelta(ApplyCurvature(fDeltaTime, fTimeToGoal));
    m_v2TravelPosition += v2Delta;
    v2Delta += v2CurveDelta;

    float fTouchRadius(GetTouchRadius());
    if (fTouchRadius > 0.0f)
    {
        TryTouch(fTouchRadius, v2Delta, fDeltaTime);
    }
    else
    {
        m_v3Position.x += v2Delta.x;
        m_v3Position.y += v2Delta.y;
    }

    if (GetFlying())
    {
        m_v3Position.z = Game.GetCameraHeight(m_v3Position.x, m_v3Position.y) + GetFlyHeight();
    }
    else
    {
        ApplyGravity(fDeltaTime, fTimeToGoal);
    }

    if (GetCanTurn() && m_v3Velocity.LengthSq() > SQR(0.001f))
        m_v3Angles = M_GetAnglesFromForwardVec(Normalize(m_v3Velocity + CVec3f(m_v2CurveAxis * m_fCurveVelocity, 0.0f)));

    m_uiLastUpdateTime = Game.GetGameTime();
}


/*====================
  IProjectile::TryImpact
  ====================*/
void    IProjectile::TryImpact()
{
    if (Game.GetGameTime() < m_uiCreationTime)
        return;
    if (GetUseExactLifetime() && GetLifetime() != 0 && Game.GetGameTime() - m_uiCreationTime < GetLifetime())
        return;
    if (!m_bReachedTarget)
        return;

    IUnitEntity *pTarget(Game.GetUnitFromUniqueID(m_uiTargetEntityUID));
    IUnitEntity *pOwner(GetOwner());

    if (pTarget != NULL)
        m_mapImpacts[m_uiTargetEntityUID] += 1;

    UpdateTargetPosition();
    
    Kill();

    bool bStealthMiss(m_uiTargetEntityUID != INVALID_INDEX && pTarget != NULL && !GetImpactStealth() && pOwner != NULL && pTarget->IsStealth() && !pTarget->HasVisibilityFlags(VIS_REVEALED(pOwner->GetTeam())) && pOwner->GetTeam() != pTarget->GetTeam());

    if (bStealthMiss)
        Game.SendPopup(POPUP_MISS, pOwner);

    ExecuteActionScript(ACTION_SCRIPT_IMPACT, pTarget, m_v3Position);

    if (bStealthMiss ||
        (m_uiTargetEntityUID != INVALID_INDEX && !Game.IsValidTarget(m_uiTargetScheme, m_uiEffectType, Game.GetUnitEntity(m_uiOwnerIndex), pTarget, m_bIgnoreInvulnerable)))
    {
        m_combat.SetTarget(pTarget ? pTarget->GetIndex() : INVALID_INDEX);
        m_combat.SetTarget(m_v3Position);
        m_combat.ProcessInvalid();

        if (pTarget != NULL && GetInvalidEffect() != INVALID_RESOURCE)
        {
            CGameEvent ev;
            ev.SetSourceEntity(pTarget->GetIndex());
            ev.SetEffect(GetInvalidEffect());
            Game.AddEvent(ev);
        }

        return;
    }

    m_combat.SetTarget(pTarget ? pTarget->GetIndex() : INVALID_INDEX);
    m_combat.SetTarget(m_v3Position);
    m_combat.SetEvasion(m_combat.GetSuperType() == SUPERTYPE_ATTACK && pTarget != NULL ? pTarget->GetEvasionRanged() : 0.0f);
    m_combat.Process();
    
    if (m_combat.GetSuccessful() &&
        GetImpactEffect() != INVALID_RESOURCE &&
        pTarget != NULL)
    {
        CGameEvent ev;
        ev.SetSourceEntity(pTarget->GetIndex());
        ev.SetEffect(GetImpactEffect());
        Game.AddEvent(ev);
    }

    if (m_combat.GetSuccessful() &&
        m_hAttackImpactEffect != INVALID_RESOURCE &&
        pTarget != NULL)
    {
        CGameEvent ev;
        ev.SetSourceEntity(pTarget->GetIndex());
        ev.SetEffect(m_hAttackImpactEffect);
        Game.AddEvent(ev);
    }
}


/*====================
  IProjectile::ServerFrameSetup
  ====================*/
bool    IProjectile::ServerFrameSetup()
{
    if (HasLocalFlags(ENT_LOCAL_DELETE_NEXT_FRAME))
        return false;

    return IVisualEntity::ServerFrameSetup();
}


/*====================
  IProjectile::ServerFrameMovement
  ====================*/
bool    IProjectile::ServerFrameMovement()
{
    // Update visibility
    SetVisibilityFlags(Game.GetVision(GetPosition().x, GetPosition().y));

    // Reset trajectory variables
    s_uiElapsed = 0;
    s_uiFrameTime = MAX(int(Game.GetGameTime() - m_uiLastUpdateTime), 0);

    if (m_uiLastUpdateTime <= Game.GetGameTime())
        EvaluateTrajectory();

    return IVisualEntity::ServerFrameMovementEnd();
}


/*====================
  IProjectile::ServerFrameAction
  ====================*/
bool    IProjectile::ServerFrameAction()
{
    if (!IVisualEntity::ServerFrameAction())
        return false;

    TryImpact();
    return true;
}


/*====================
  IProjectile::ServerFrameCleanup
  ====================*/
bool    IProjectile::ServerFrameCleanup()
{
    if (!IVisualEntity::ServerFrameCleanup())
        return false;

    // Check for expired timer
    if (GetLifetime() == 0 || Game.GetGameTime() < m_uiCreationTime)
        return true;
    
    if (Game.GetGameTime() - m_uiCreationTime >= GetLifetime())
    {
        ExecuteActionScript(ACTION_SCRIPT_EXPIRED, Game.GetUnitFromUniqueID(m_uiTargetEntityUID), m_v3Position);
        Kill();
    }

    return true;
}


/*====================
  IProjectile::Kill
  ====================*/
void    IProjectile::Kill(IVisualEntity *pAttacker, ushort unKillingObjectID)
{
    m_yStatus = ENTITY_STATUS_DEAD;
    SetLocalFlags(ENT_LOCAL_DELETE_NEXT_FRAME);

    ExecuteActionScript(ACTION_SCRIPT_DEATH, pAttacker ? pAttacker->GetAsUnit() : NULL, m_v3Position);

    // Death event
    if (GetDeathEffect() != INVALID_RESOURCE)
    {
        CGameEvent evDeath;

        // Occlude
        evDeath.RemoveVisibilityFlags(~Game.GetVision(m_v3Position.x, m_v3Position.y));

        IUnitEntity *pOwner(GetOwner());
        if (pOwner != NULL)
            evDeath.SetVisibilityFlags(VIS_SIGHTED(pOwner->GetTeam()));

        evDeath.SetSourcePosition(m_v3Position);
        evDeath.SetEffect(GetDeathEffect());
        Game.AddEvent(evDeath);
    }

    ReleaseBinds();
}


/*====================
  IProjectile::Revive
  ====================*/
void    IProjectile::Revive()
{
    m_yStatus = ENTITY_STATUS_ACTIVE;
    m_bReachedTarget = false;
    m_bForceImpact = false;
    RemoveLocalFlags(ENT_LOCAL_DELETE_NEXT_FRAME);
}


/*====================
  IProjectile::Redirect
  ====================*/
void    IProjectile::Redirect(IUnitEntity *pSource, IGameEntity *pInflictor, IUnitEntity *pTarget)
{
    if (pSource == NULL || pTarget == NULL)
        return;

    Revive();

    SetOwner(pSource->GetIndex());
    SetTargetEntityUID(pTarget->GetUniqueID());
    SetTargetDisjointSequence(pTarget->GetDisjointSequence());
    SetTargetPos(pTarget->GetPosition() + pTarget->GetTargetOffset());

    m_combat.Redirect();
    m_combat.SetTarget(pTarget->GetIndex());
    m_combat.SetEvasion(pTarget->GetEvasionRanged());
    m_combat.SetInitiatorIndex(pSource->GetIndex());
    m_combat.SetInflictorIndex(pInflictor ? pInflictor->GetIndex() : INVALID_INDEX);

    CVec2f v2Pos(m_v2TravelPosition);

    // Set Initial velocity
    float fSpeed(GetSpeed());
    float fTime;
    if (GetUseExactLifetime() && GetLifetime() != 0)
        fTime = MsToSec(GetLifetime());
    else
        fTime = (Distance(v2Pos, m_v3TargetPos.xy()) / fSpeed);

    m_v3Velocity.Clear();

    // calc initial gravity.
    m_v3Velocity.z = (m_v3TargetPos.z - m_v3Position.z) / fTime;
    if (GetArc() != 0.0f)
        m_v3Velocity.z += GetSpeed() * GetArc();
    else
        m_v3Velocity.z += (0.5f * GetGravity() * fTime);

    if (GetFlying())
        m_v3Position.z = Game.GetCameraHeight(m_v3Position.x, m_v3Position.y) + GetFlyHeight();

    IncrementRedirectCount();
}

void    IProjectile::Redirect(IUnitEntity *pSource, IGameEntity *pInflictor, const CVec3f &v3Target)
{
    if (pSource == NULL)
        return;

    Revive();

    SetOwner(pSource->GetIndex());
    SetTargetEntityUID(INVALID_INDEX);
    SetTargetDisjointSequence(0);
    SetTargetPos(v3Target);

    m_combat.Redirect();
    m_combat.SetTarget(INVALID_INDEX);
    m_combat.SetEvasion(0);
    m_combat.SetInitiatorIndex(pSource->GetIndex());
    m_combat.SetInflictorIndex(pInflictor ? pInflictor->GetIndex() : INVALID_INDEX);

    IncrementRedirectCount();
}


/*====================
  IProjectile::Copy
  ====================*/
void    IProjectile::Copy(const IGameEntity &B)
{
    IVisualEntity::Copy(B);

    const IProjectile *pB(B.GetAsProjectile());
    if (!pB)
        return;
    const IProjectile &C(*pB);
    
    m_uiCreationTime =  C.m_uiCreationTime;
    m_uiOriginTime =    C.m_uiOriginTime;
    m_uiOwnerIndex =    C.m_uiOwnerIndex;
}


/*====================
  IProjectile::Interpolate
  ====================*/
void    IProjectile::Interpolate(float fLerp, IVisualEntity *pPrevState, IVisualEntity *pNextState)
{
    m_v3Position = LERP(fLerp, pPrevState->GetPosition(), pNextState->GetPosition());
    m_v3Angles = M_LerpAngles(fLerp, pPrevState->GetAngles(), pNextState->GetAngles());
    m_fScale = LERP(fLerp, pPrevState->GetScale(), pNextState->GetScale());
    m_unVisibilityFlags = pPrevState->GetVisibilityFlags() & pNextState->GetVisibilityFlags();
}


/*====================
  IProjectile::AddToScene
  ====================*/
bool    IProjectile::AddToScene(const CVec4f &v4Color, int iFlags)
{
    return IVisualEntity::AddToScene(v4Color, iFlags);
}


/*====================
  IProjectile::UpdateModifiers
  ====================*/
void    IProjectile::UpdateModifiers(const uivector &vModifiers)
{
    m_vModifierKeys = vModifiers;

    uint uiModifierBits(GetModifierBits(vModifiers));
    if (m_uiActiveModifierKey != INVALID_INDEX)
        uiModifierBits |= GetModifierBit(m_uiActiveModifierKey);

    // Activate conditional modifiers
    IUnitEntity *pOwner(GetOwner());
    if (pOwner == NULL)
        return;

    // Grab base definition
    IEntityDefinition *pDefinition(GetBaseDefinition<IEntityDefinition>());
    if (pDefinition == NULL)
        return;

    const EntityModifierMap &mapModifiers(pDefinition->GetModifiers());
    for (EntityModifierMap::const_iterator cit(mapModifiers.begin()), citEnd(mapModifiers.end()); cit != citEnd; ++cit)
    {
        const tstring &sCondition(cit->second->GetCondition());
        if (sCondition.empty())
            continue;

        tsvector vsTypes(TokenizeString(sCondition, _T(' ')));

        tsvector_cit itType(vsTypes.begin()), itTypeEnd(vsTypes.end());
        for (; itType != itTypeEnd; ++itType)
        {
            if (!itType->empty() && (*itType)[0] == _T('!'))
            {
                if (pOwner->IsTargetType(itType->substr(1), pOwner))
                    break;
            }
            else
            {
                if (!pOwner->IsTargetType(*itType, pOwner))
                    break;
            }
        }
        if (itType == itTypeEnd)
            uiModifierBits |= cit->first;
    }

    SetModifierBits(uiModifierBits);
}


/*====================
  IProjectile::ExecuteActionScript
  ====================*/
void    IProjectile::ExecuteActionScript(EEntityActionScript eScript, IUnitEntity *pTarget, const CVec3f &v3Target)
{
    if (m_pDefinition == NULL)
        return;
        
    m_pDefinition->ExecuteActionScript(eScript, this, GetOwner(), this, pTarget, v3Target, GetProxy(0), GetLevel());
}


/*====================
  IProjectile::Clone
  ====================*/
IProjectile*    IProjectile::Clone()
{
    IGameEntity *pNewEntity(Game.AllocateEntity(GetType()));
    if (pNewEntity == NULL)
    {
        Console.Err << _T("Failed to spawn projectile: ") << GetTypeName() << newl;
        return NULL;
    }

    IProjectile *pProjectile(pNewEntity->GetAsProjectile());
    if (pProjectile == NULL)
    {
        Console.Err << _T("Entity is not a projectile: ") << GetTypeName() << newl;
        Game.DeleteEntity(pNewEntity);
        return NULL;
    }

    pProjectile->SetPosition(m_v3Position);
    pProjectile->SetAngles(m_v3Angles);
    pProjectile->SetOwner(m_uiOwnerIndex);
    pProjectile->SetOriginTime(m_uiOriginTime);
    pProjectile->UpdateModifiers(m_vModifierKeys);
    pProjectile->SetLevel(m_uiLevel);

    pProjectile->GetCombatEvent() = m_combat;
    
    pProjectile->SetTargetEntityUID(m_uiTargetEntityUID);
    pProjectile->SetTargetDisjointSequence(m_uiTargetDisjointSequence);

    pProjectile->Spawn();
    
    return pProjectile;
}


/*====================
  IProjectile::GetTarget
  ====================*/
IUnitEntity*    IProjectile::GetTarget() const
{
    IGameEntity *pTarget(Game.GetEntityFromUniqueID(m_uiTargetEntityUID));
    if (pTarget != NULL)
        return pTarget->GetAsUnit();
    else
        return NULL;
}


/*====================
  IProjectile::ForceImpact
  ====================*/
void    IProjectile::ForceImpact()
{
    m_bForceImpact = true;
}
