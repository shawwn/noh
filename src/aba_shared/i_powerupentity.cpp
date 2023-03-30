// (C)2008 S2 Games
// i_powerupentity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_powerupentity.h"
#include "i_heroentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
uint                IPowerupEntity::s_uiBaseType(ENTITY_BASE_TYPE_POWERUP);

CVAR_FLOATF(    g_powerupAnnounceRadius,    2000.0f,    CVAR_GAMECONFIG);

DEFINE_ENTITY_DESC(IPowerupEntity, 1)
{
    s_cDesc.pFieldTypes = K2_NEW(g_heapTypeVector,   TypeVector)();
    s_cDesc.pFieldTypes->clear();
    const TypeVector &vBase(IUnitEntity::GetTypeVector());
    s_cDesc.pFieldTypes->insert(s_cDesc.pFieldTypes->begin(), vBase.begin(), vBase.end());
}
//=============================================================================

/*====================
  IPowerupEntity::IPowerupEntity
  ====================*/
IPowerupEntity::IPowerupEntity()
{
    m_uiLinkFlags = SURF_ITEM;
}


/*====================
  IPowerupEntity::Baseline
  ====================*/
void    IPowerupEntity::Baseline()
{
    IUnitEntity::Baseline();
}


/*====================
  IPowerupEntity::GetSnapshot
  ====================*/
void    IPowerupEntity::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
    // Base entity info
    IUnitEntity::GetSnapshot(snapshot, uiFlags);
}


/*====================
  IPowerupEntity::ReadSnapshot
  ====================*/
bool    IPowerupEntity::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
    try
    {
        // Base entity info
        if (!IUnitEntity::ReadSnapshot(snapshot, 1))
            return false;

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("IPowerupEntity::ReadSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  IPowerupEntity::Copy
  ====================*/
void    IPowerupEntity::Copy(const IGameEntity &B)
{
    IUnitEntity::Copy(B);

    const IPowerupEntity *pB(B.GetAsPowerup());

    if (!pB)    
        return;

    //const IPowerupEntity &C(*pB);
}


/*====================
  IPowerupEntity::Spawn
  ====================*/
void    IPowerupEntity::Spawn()
{
    IUnitEntity::Spawn();
}


/*====================
  IPowerupEntity::Touch
  ====================*/
void    IPowerupEntity::Touch(IGameEntity *pActivator)
{
    if (!Game.IsServer() || pActivator == NULL)
        return;

    if (m_yStatus != ENTITY_STATUS_ACTIVE)
        return;

    IHeroEntity *pHero(pActivator->GetAsHero());
    if (pHero == NULL)
        return;

    uint uiTargetScheme(GetTouchTargetScheme());
    if (uiTargetScheme != INVALID_TARGET_SCHEME)
    {
        if (!Game.IsValidTarget(uiTargetScheme, 0, pHero, pHero))
            return;
    }

    Action(ACTION_SCRIPT_TOUCHED, pHero->GetAsUnit(), pHero->GetAsUnit());

    iset setClients;
    uivector vEntities;
    Game.GetEntitiesInRadius(vEntities, GetPosition().xy(), g_powerupAnnounceRadius, 0);
    for (uivector_it itEntity(vEntities.begin()); itEntity != vEntities.end(); ++itEntity)
    {
        IUnitEntity *pOther(Game.GetUnitEntity(Game.GetGameIndexFromWorldIndex(*itEntity)));
        if (pOther == NULL)
            continue;
        if (pOther->GetTeam() != pHero->GetTeam())
            continue;
        if (pOther->GetOwnerClientNumber() == -1)
            continue;

        setClients.insert(pOther->GetOwnerClientNumber());
    }

    static CBufferFixed<7> buffer;
    buffer.Clear();
    buffer << GAME_CMD_POWERUP_MESSAGE << pHero->GetOwnerClientNumber() << GetType();

    for (iset_it itClient(setClients.begin()); itClient != setClients.end(); ++itClient)
        Game.SendGameData(*itClient, buffer, true);

    Game.LogHero(GAME_LOG_HERO_POWERUP, pHero, GetTypeName());
    SetDelete(true);
    SetStatus(ENTITY_STATUS_DORMANT);
}


/*====================
  IPowerupEntity::ClientPrecache
  ====================*/
void    IPowerupEntity::ClientPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme)
{
    IUnitEntity::ClientPrecache(pConfig, eScheme);
}


/*====================
  IPowerupEntity::ServerPrecache
  ====================*/
void    IPowerupEntity::ServerPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme)
{
    IUnitEntity::ServerPrecache(pConfig, eScheme);
}
