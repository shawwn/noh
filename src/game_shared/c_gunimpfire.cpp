// (C)2007 S2 Games
// c_gunimpfire.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_gunimpfire.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Gun, ImpFire);
//=============================================================================

/*====================
  CGunImpFire::FireProjectile
  ====================*/
IProjectile*    CGunImpFire::FireProjectile(const CVec3f &v3Origin, const CVec3f &v3Dir, float fCharge)
{
    IProjectile *pProjectile(IGunItem::FireProjectile(v3Origin, v3Dir, fCharge));
    if (pProjectile == NULL)
        return NULL;

    IPetEntity *pOwner(Game.GetPetEntity(GetOwner()));
    if (pOwner == NULL)
        return pProjectile;

    IGameEntity *pTarget(Game.GetEntityFromUniqueID(pOwner->GetTargetUID()));
    if (pTarget == NULL)
        return pProjectile;
    IVisualEntity *pTargetVis(pTarget->GetAsVisualEnt());
    if (pTargetVis == NULL)
        return pProjectile;

    pProjectile->SetVelocity(pProjectile->GetVelocity() + pTargetVis->GetVelocity());

    return pProjectile;
}
