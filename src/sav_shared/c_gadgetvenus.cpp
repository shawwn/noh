// (C)2007 S2 Games
// c_gadgetvenus.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_gadgetvenus.h"

#include "../k2/intersection.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Gadget, Venus)
//=============================================================================

/*====================
  CGadgetVenus::CEntityConfig::CEntityConfig
  ====================*/
CGadgetVenus::CEntityConfig::CEntityConfig(const tstring &sName) :
IGadgetEntity::CEntityConfig(sName),
INIT_ENTITY_CVAR(Range, 500.0f),
INIT_ENTITY_CVAR(AttackTime, 1500),
INIT_ENTITY_CVAR(AttackOffset, V_ZERO),
INIT_ENTITY_CVAR(ProjectileName, _T("")),
INIT_ENTITY_CVAR(SpreadX, 0.0f),
INIT_ENTITY_CVAR(SpreadY, 0.0f),
INIT_ENTITY_CVAR(MinDamage, 50.0f),
INIT_ENTITY_CVAR(MaxDamage, 50.0f),
INIT_ENTITY_CVAR(DamageRadius, 0.0f),
INIT_ENTITY_CVAR(PierceUnit, 1.0f),
INIT_ENTITY_CVAR(PierceHellbourne, 1.0f),
INIT_ENTITY_CVAR(PierceSiege, 1.0f),
INIT_ENTITY_CVAR(PierceBuilding, 1.0f)
{
}


/*====================
  CGadgetVenus::ValidateTarget
  ====================*/
bool    CGadgetVenus::ValidateTarget(uint uiTargetIndex)
{
    if (uiTargetIndex == INVALID_INDEX)
        return false;

    IVisualEntity *pTarget(Game.GetVisualEntity(uiTargetIndex));
    if (pTarget == NULL)
        return false;

    // Check status
    if (pTarget->GetStatus() != ENTITY_STATUS_ACTIVE)
        return false;
    if (!IsEnemy(pTarget))
        return false;
    if (!LooksLikeEnemy(pTarget) && !pTarget->HasNetFlags(ENT_NET_FLAG_EYED))
        return false;
    if (pTarget->IsStealthed() || pTarget->IsIntangible())
        return false;
    if (!pTarget->AIShouldTarget())
        return false;

    // Check range
    CBBoxf bbBoundsWorld(pTarget->GetBounds());
    bbBoundsWorld.Transform(pTarget->GetPosition(), CAxis(pTarget->GetAngles()), pTarget->GetScale());
    if (!I_SphereBoundsIntersect(CSphere(GetPosition(), m_pEntityConfig->GetRange()), bbBoundsWorld))
        return false;

    // Check vision
    CVec3f v3Start(m_v3Position + m_pEntityConfig->GetAttackOffset());
    CVec3f v3End(pTarget->GetPosition() + pTarget->GetBounds().GetMid());
    STraceInfo trace;
    Game.TraceLine(trace, v3Start, v3End, 0, GetWorldIndex());

    if (trace.uiEntityIndex == INVALID_INDEX)
        return false;

    IVisualEntity *pActualTarget(Game.GetEntityFromWorldIndex(trace.uiEntityIndex));

    if (pActualTarget == NULL || pActualTarget->GetIndex() != pTarget->GetIndex())
        return false;

    return true;
}


/*====================
  CGadgetVenus::ServerFrame
  ====================*/
bool    CGadgetVenus::ServerFrame()
{
    if (!IGadgetEntity::ServerFrame())
        return false;

    if (GetStatus() != ENTITY_STATUS_ACTIVE)
        return true;

    // Select target
    float fShortestDistance(FAR_AWAY);
    if (!ValidateTarget(m_uiTarget))
    {
        m_uiTarget = INVALID_INDEX;

        uivector vSight;
        Game.GetEntitiesInRadius(vSight, CSphere(GetPosition(), m_pEntityConfig->GetRange()), 0);
        for (uivector_it it(vSight.begin()); it != vSight.end(); ++it)
        {
            IVisualEntity *pTarget(Game.GetEntityFromWorldIndex(*it));
            if (pTarget == NULL)
                continue;
            if (!ValidateTarget(pTarget->GetIndex()))
                continue;

            CVec3f v3Dist(pTarget->GetPosition() - GetPosition());
            float fDistance(v3Dist.Length());
            if (fDistance < fShortestDistance)
            {
                m_uiTarget = pTarget->GetIndex();
                fShortestDistance = fDistance;
            }
        }
    }

    IVisualEntity *pTarget(Game.GetVisualEntity(m_uiTarget));
    if (pTarget == NULL)
        return true;

    // Face current target
    CVec3f v3Start(m_v3Position + m_pEntityConfig->GetAttackOffset());
    CVec3f v3Aim(pTarget->GetPosition() + pTarget->GetBounds().GetMid());
    CVec3f v3Forward(Normalize(v3Aim - v3Start));
    CVec3f v3Angles(M_GetAnglesFromForwardVec(v3Forward));
    v3Angles[YAW] += 90.0f;     // HACK: Model doesn't face forward
    SetAngles(v3Angles);

    if (m_uiNextAttackTime > Game.GetGameTime())
        return true;

    // Spawn a projectile
    IGameEntity *pNewEnt(Game.AllocateEntity(m_pEntityConfig->GetProjectileName()));
    if (pNewEnt == NULL || pNewEnt->GetAsProjectile() == NULL)
        return true;
    IProjectile *pProjectile(pNewEnt->GetAsProjectile());

    // Attack the target
    CAxis axis(GetAxisFromForwardVec(v3Forward));
    const CVec3f &v3Right(axis.Right());
    const CVec3f &v3Up(axis.Up());

    CVec3f v3Rand;
    do
    {
        v3Rand.x = M_Randnum(-0.5f, 0.5f) + M_Randnum(-0.5f, 0.5f);
        v3Rand.y = M_Randnum(-0.5f, 0.5f) + M_Randnum(-0.5f, 0.5f);
        v3Rand.z = v3Rand.x * v3Rand.x + v3Rand.y * v3Rand.y;
    }
    while (v3Rand.z > 1.0f);

    CVec2f v2Spread(m_pEntityConfig->GetSpreadX(), m_pEntityConfig->GetSpreadY());
    CVec3f v3Dir(Normalize(v3Forward + v3Right * (v3Rand.x * v2Spread.x) + v3Up * (v3Rand.y * v2Spread.y)));

    float fTime(Distance(v3Start, v3Aim) / pProjectile->GetSpeed());
    
    pProjectile->SetOwner(GetIndex());
    pProjectile->SetWeaponOrigin(Spell_Venus);
    pProjectile->SetOrigin(v3Start);
    pProjectile->SetAngles(M_GetAnglesFromForwardVec(v3Forward));
    pProjectile->SetVelocity(v3Dir * pProjectile->GetSpeed() + CVec3f(0.0f, 0.0f, (p_gravity * pProjectile->GetGravity()) * (fTime / 2.0f)));
    pProjectile->SetOriginTime(Game.GetGameTime());
    pProjectile->SetDamage(m_pEntityConfig->GetMinDamage(), m_pEntityConfig->GetMaxDamage(), m_pEntityConfig->GetDamageRadius(),
                        m_pEntityConfig->GetPierceUnit(), m_pEntityConfig->GetPierceHellbourne(),
                        m_pEntityConfig->GetPierceSiege(), m_pEntityConfig->GetPierceBuilding());
    pProjectile->Spawn();

    m_uiNextAttackTime = Game.GetGameTime() + m_pEntityConfig->GetAttackTime();
    
    StartAnimation(_T("spore"), 0);
    return true;
}
