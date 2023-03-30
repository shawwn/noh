// (C)2008 S2 Games
// c_proptree.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_proptree.h"

#include "../k2/c_scenemanager.h"
#include "../k2/c_model.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Prop, Tree);
//=============================================================================

/*====================
  CPropTree::CEntityConfig::CEntityConfig
  ====================*/
CPropTree::CEntityConfig::CEntityConfig(const tstring &sName) :
IBitEntity::CEntityConfig(sName),
INIT_ENTITY_CVAR(DisplayName, _T(""))
{
}


/*====================
  CPropTree::CPropTree
  ====================*/
CPropTree::CPropTree() :
IBitEntity(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}


/*====================
  CPropTree::Spawn
  ====================*/
void    CPropTree::Spawn()
{
    IBitEntity::Spawn();

    if (m_uiWorldIndex == INVALID_INDEX || !Game.WorldEntityExists(m_uiWorldIndex))
        return;

    CWorldEntity *pWorldEnt(Game.GetWorldEntity(m_uiWorldIndex));
    if (pWorldEnt == NULL)
        return;

    float fOcclusionRadius(GetOcclusionRadius());

    m_bbBounds.SetCylinder(GetBoundsRadius(), GetBoundsHeight());

    pWorldEnt->SetBounds(m_bbBounds);
    pWorldEnt->SetModelHandle(Game.RegisterModel(pWorldEnt->GetModelPath()));

    if (Game.IsServer())
        NetworkResourceManager.GetNetIndex(pWorldEnt->GetModelHandle());

    pWorldEnt->SetOcclusionRadius(fOcclusionRadius);

    Game.LinkEntity(pWorldEnt->GetIndex(), LINK_BOUNDS | LINK_RENDER, SURF_TREE | SURF_STATIC);

    Activate();
}


/*====================
  CPropTree::Activate
  ====================*/
void    CPropTree::Activate()
{
    // Unblock pathing
    vector<PoolHandle>::const_iterator citEnd(m_vPathBlockers.end());
    for (vector<PoolHandle>::const_iterator cit(m_vPathBlockers.begin()); cit != citEnd; ++cit)
        Game.ClearPath(*cit);

    m_vPathBlockers.clear();

    CWorldEntity *pWorldEnt(Game.GetWorldEntity(m_uiWorldIndex));
    if (pWorldEnt == NULL)
        return;

    pWorldEnt->SetSurfFlags(pWorldEnt->GetSurfFlags() & ~SURF_IGNORE);

    float fOcclusionRadius(GetOcclusionRadius());

    // Block pathing
    m_vPathBlockers.push_back(Game.BlockPath(NAVIGATION_TREE, pWorldEnt->GetPosition().xy() - CVec2f(fOcclusionRadius), fOcclusionRadius * 2.0f, fOcclusionRadius * 2.0f));

    m_v2BlockPosition = pWorldEnt->GetPosition().xy();

    Game.AddOccludeRegion(pWorldEnt->GetPosition(), fOcclusionRadius);

    SetVisibilityFlags(VIS_SIGHTED(1));
    SetVisibilityFlags(VIS_SIGHTED(2));

    m_yStatus = ENTITY_STATUS_ACTIVE;
}


/*====================
  CPropTree::Deactivate
  ====================*/
void    CPropTree::Deactivate()
{
    CWorldEntity *pWorldEnt(Game.GetWorldEntity(m_uiWorldIndex));
    if (pWorldEnt == NULL)
        return;

    pWorldEnt->SetSurfFlags(pWorldEnt->GetSurfFlags() | SURF_IGNORE);

    // Unblock pathing
    vector<PoolHandle>::const_iterator citEnd(m_vPathBlockers.end());
    for (vector<PoolHandle>::const_iterator cit(m_vPathBlockers.begin()); cit != citEnd; ++cit)
        Game.ClearPath(*cit);

    m_vPathBlockers.clear();

    float fOcclusionRadius(GetOcclusionRadius());

    Game.RemoveOccludeRegion(pWorldEnt->GetPosition(), fOcclusionRadius);

    ClearVisibilityFlags();
    
    m_yStatus = ENTITY_STATUS_DORMANT;

    ReleaseBinds();
}


/*====================
  CPropTree::IsTargetType
  ====================*/
bool    CPropTree::IsTargetType(const CTargetScheme::STestRecord &test, const IUnitEntity *pInitiator) const
{
    if (test.m_eTest == CTargetScheme::TARGET_SCHEME_TEST_TRAIT && test.m_eTrait == TARGET_TRAIT_TREE)
        return true;
    if (test.m_eTest == CTargetScheme::TARGET_SCHEME_TEST_NOT_TRAIT && test.m_eTrait == TARGET_TRAIT_TREE)
        return false;
    
    return IUnitEntity::IsTargetType(test, pInitiator);
}

