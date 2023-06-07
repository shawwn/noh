// (C)2006 S2 Games
// c_gadgetshieldgenerator.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_gadgetshieldgenerator.h"
#include "../k2/c_worldentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Gadget, ShieldGenerator)

CVAR_FLOATF(    g_expShieldBlockedProjectile,   5.0f,   CVAR_GAMECONFIG);
//=============================================================================

/*====================
  CGadgetShieldGenerator::CEntityConfig::CEntityConfig
  ====================*/
CGadgetShieldGenerator::CEntityConfig::CEntityConfig(const tstring &sName) :
IGadgetEntity::CEntityConfig(sName),
INIT_ENTITY_CVAR(ShieldSurfaceModelPath, _T(""))
{
}

/*====================
  CGadgetShieldGenerator::~CGadgetShieldGenerator
  ====================*/
CGadgetShieldGenerator::~CGadgetShieldGenerator()
{
    if (m_uiShieldSurfaceWorldIndex != INVALID_INDEX && Game.WorldEntityExists(m_uiShieldSurfaceWorldIndex))
    {
        Game.UnlinkEntity(m_uiShieldSurfaceWorldIndex);
        Game.DeleteWorldEntity(m_uiShieldSurfaceWorldIndex);
    }
}


/*====================
  CGadgetShieldGenerator::CGadgetShieldGenerator
  ====================*/
CGadgetShieldGenerator::CGadgetShieldGenerator() :
IGadgetEntity(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig()),

m_uiShieldSurfaceWorldIndex(INVALID_INDEX)
{
    m_vCounterLabels.push_back(_T("Deflections"));
    m_auiCounter[0] = 0;
}


/*====================
  CGadgetShieldGenerator::Baseline
  ====================*/
void    CGadgetShieldGenerator::Baseline()
{
    IGadgetEntity::Baseline();
    m_auiCounter[0] = 0;
}


/*====================
  CGadgetShieldGenerator::Spawn
  ====================*/
void    CGadgetShieldGenerator::Spawn()
{
    m_uiShieldSurfaceWorldIndex = Game.AllocateNewWorldEntity();
    CWorldEntity *pWorldEnt(Game.GetWorldEntity(m_uiShieldSurfaceWorldIndex));
    if (pWorldEnt == NULL)
    {
        m_uiShieldSurfaceWorldIndex = INVALID_INDEX;
        return;
    }

    m_hShieldSurfaceModel = Game.RegisterModel(m_pEntityConfig->GetShieldSurfaceModelPath());

    pWorldEnt->SetPosition(GetPosition());
    pWorldEnt->SetScale(GetScale());
    pWorldEnt->SetScale2(GetScale2());
    pWorldEnt->SetAngles(GetAngles());
    pWorldEnt->SetGameIndex(GetIndex());
    pWorldEnt->SetModelHandle(m_hShieldSurfaceModel);
    pWorldEnt->SetTeam(GetTeam());

    IGadgetEntity::Spawn();
}


/*====================
  CGadgetShieldGenerator::Link
  ====================*/
void    CGadgetShieldGenerator::Link()
{
    IGadgetEntity::Link();

    if (m_uiShieldSurfaceWorldIndex == INVALID_INDEX)
        return;

    CWorldEntity *pWorldEnt(Game.GetWorldEntity(m_uiShieldSurfaceWorldIndex));
    if (pWorldEnt == NULL)
        return;
    
    pWorldEnt->SetPosition(GetPosition());
    pWorldEnt->SetScale(GetScale());
    pWorldEnt->SetScale2(GetScale2());
    pWorldEnt->SetAngles(GetAngles());
    pWorldEnt->SetModelHandle(m_hShieldSurfaceModel);
    Game.LinkEntity(m_uiShieldSurfaceWorldIndex, LINK_SURFACE, SURF_SHIELD);
}


/*====================
  CGadgetShieldGenerator::Unlink
  ====================*/
void    CGadgetShieldGenerator::Unlink()
{
    IGadgetEntity::Unlink();

    if (m_uiShieldSurfaceWorldIndex != INVALID_INDEX)
        Game.UnlinkEntity(m_uiShieldSurfaceWorldIndex);
}


/*====================
  CGadgetShieldGenerator::Impact
  ====================*/
bool    CGadgetShieldGenerator::Impact(STraceInfo &trace, IVisualEntity *pSource)
{
    if (trace.uiEntityIndex != m_uiShieldSurfaceWorldIndex)
        return true;
    
    if (pSource != NULL && pSource->GetTeam() != GetTeam() && m_setReflected.find(pSource->GetIndex()) == m_setReflected.end())
    {
        IPlayerEntity *pOwner(Game.GetPlayerEntity(m_uiOwnerIndex));
        if (pOwner != NULL)
        {
            pOwner->GiveExperience(g_expShieldBlockedProjectile, trace.v3EndPos);
            m_fTotalExperience += g_expShieldBlockedProjectile;
            ++m_auiCounter[SHIELD_COUNTER_SHOTS_DEFLECTED];
        }

        m_setReflected.insert(pSource->GetIndex());
    }

    ResHandle hHitEffectPath(Game.RegisterEffect(GetHitByRangedEffectPath()));

    if (hHitEffectPath == INVALID_RESOURCE)
        return false;

    //TODO: Set up impact angle properly
    CGameEvent evImpact;
    evImpact.SetSourcePosition(trace.v3EndPos);
    evImpact.SetEffect(hHitEffectPath);
    Game.AddEvent(evImpact);

    return false;
}


/*====================
  CGadgetShieldGenerator::Copy
  ====================*/
void    CGadgetShieldGenerator::Copy(const IGameEntity &B)
{
    IGadgetEntity::Copy(B);

    if (GetType() != B.GetType())
        return;

    const CGadgetShieldGenerator &C(static_cast<const CGadgetShieldGenerator&>(B));
    m_uiShieldSurfaceWorldIndex =   C.m_uiShieldSurfaceWorldIndex;
}


/*====================
  CGadgetShieldGenerator::ClientPrecache
  ====================*/
void    CGadgetShieldGenerator::ClientPrecache(CEntityConfig *pConfig)
{
    IGadgetEntity::ClientPrecache(pConfig);

    if (!pConfig)
        return;

    if (!pConfig->GetShieldSurfaceModelPath().empty())
        g_ResourceManager.Register(pConfig->GetShieldSurfaceModelPath(), RES_MODEL);
}


/*====================
  CGadgetShieldGenerator::ServerPrecache
  ====================*/
void    CGadgetShieldGenerator::ServerPrecache(CEntityConfig *pConfig)
{
    IGadgetEntity::ServerPrecache(pConfig);

    if (!pConfig)
        return;

    if (!pConfig->GetShieldSurfaceModelPath().empty())
        g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetShieldSurfaceModelPath(), RES_MODEL, RES_MODEL_SERVER));
}
