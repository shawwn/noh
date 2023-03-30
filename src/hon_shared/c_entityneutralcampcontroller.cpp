// (C)2008 S2 Games
// c_entityneutralcampcontroller.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_entityneutralcampcontroller.h"
#include "c_entityneutralcampspawner.h"

#include "i_unitentity.h"
#include "i_heroentity.h"

#include "../k2/c_cvararray.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Entity, NeutralCampController)

ResHandle           CEntityNeutralCampController::s_hMinimapIcon(INVALID_RESOURCE);

CVAR_UINTF(     g_neutralFirstSpawn,            SecToMs(30u),   CVAR_GAMECONFIG);
CVAR_UINTF(     g_neutralRespawnInterval,       SecToMs(60u),   CVAR_GAMECONFIG);
CVAR_FLOATF(    g_neutralNoRespawnProximity,    500.0f,         CVAR_GAMECONFIG);
CVAR_ARRAY_VEC3F(g_neutralCampMapIconColor, 4,  CVec3f(0.0f, 0.5f, 0.0f),   CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    g_neutralCampMapIconSize,       0.035f,         CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_STRINGF(   g_neutralCampMapIcon,           "/shared/icons/minimap_circle.tga",     CVAR_TRANSMIT | CVAR_GAMECONFIG);

DEFINE_ENTITY_DESC(CEntityNeutralCampController, 1)
{
    s_cDesc.pFieldTypes = K2_NEW(ctx_Game,    TypeVector)();
    s_cDesc.pFieldTypes->clear();
    const TypeVector &vBase(IVisualEntity::GetTypeVector());
    s_cDesc.pFieldTypes->insert(s_cDesc.pFieldTypes->begin(), vBase.begin(), vBase.end());

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiLevel"), TYPE_INT, 8, 0));
}
//=============================================================================

/*====================
  CEntityNeutralCampController::CEntityNeutralCampController
  ====================*/
CEntityNeutralCampController::CEntityNeutralCampController() :
m_uiLevel(1),
m_uiNextSpawnTime(INVALID_TIME),
m_bInitialSpawnAttempted(false),
m_uiNumActive(0),
m_uiMaxCreepStacks(0),
m_uiNoRespawnRadius(0),
m_uiRespawnInterval(0),

m_uiGuardChaseTime(0),
m_uiGuardChaseDistance(0),
m_uiGuardReaggroChaseTime(0),
m_uiGuardReaggroChaseDistance(0),
m_uiOverrideAggroRange(0)
{
}


/*====================
  CEntityNeutralCampController::ClientPrecache
  ====================*/
void    CEntityNeutralCampController::ClientPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier)
{
    s_hMinimapIcon = Game.RegisterIcon(g_neutralCampMapIcon);
}


/*====================
  CEntityNeutralCampController::ServerPrecache
  ====================*/
void    CEntityNeutralCampController::ServerPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier)
{
}


/*====================
  CEntityNeutralCampController::Baseline
  ====================*/
void    CEntityNeutralCampController::Baseline()
{
    IVisualEntity::Baseline();

    m_uiLevel = 1;
}


/*====================
  CEntityNeutralCampController::GetSnapshot
  ====================*/
void    CEntityNeutralCampController::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
    IVisualEntity::GetSnapshot(snapshot, uiFlags);
    
    snapshot.WriteField(m_uiLevel);
}


/*====================
  CEntityNeutralCampController::ReadSnapshot
  ====================*/
bool    CEntityNeutralCampController::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
    if (!IVisualEntity::ReadSnapshot(snapshot, 1))
        return false;

    snapshot.ReadField(m_uiLevel);
    return true;
}


/*====================
  CEntityNeutralCampController::Copy
  ====================*/
void    CEntityNeutralCampController::Copy(const IGameEntity &B)
{
    IVisualEntity::Copy(B);

    const CEntityNeutralCampController *pB(static_cast<const CEntityNeutralCampController *>(&B));

    if (!pB)    
        return;

    const CEntityNeutralCampController &C(*pB);

    m_uiLevel = C.m_uiLevel;
}


/*====================
  CEntityNeutralCampController::ApplyWorldEntity
  ====================*/
void    CEntityNeutralCampController::ApplyWorldEntity(const CWorldEntity &ent)
{
    IVisualEntity::ApplyWorldEntity(ent);

    m_uiLevel = ent.GetPropertyInt(_T("level"), 1);

    for (uint ui(0); ; ++ui)
    {
        tstring sPropertyName(_T("target") + XtoA(ui));
        if (!ent.HasProperty(sPropertyName))
            break;

        const tstring &sValue(ent.GetProperty(sPropertyName));
        if (sValue.empty())
            break;

        m_vSpawnerArray.push_back((TokenizeString(sValue, _T(','))));
        m_vSpawnerUIDArray.push_back(NeutralSpawnerUIDList(m_vSpawnerArray.back().size(), INVALID_INDEX));
    }

    if (ent.HasProperty(_T("config")))
    {
        const tstring &sValue(ent.GetProperty(_T("config")));
        if (!sValue.empty())
        {
            tsvector tsSettings(TokenizeString(LowerString(sValue),_T(',')));
            bool bWarnInvalid(false);
            for (tsvector::iterator it = tsSettings.begin(); it != tsSettings.end(); ++it)
            {
                tsvector tsKeyValue(TokenizeString(*it, _T(':')));
                if (tsKeyValue.size() != 2)
                {
                    bWarnInvalid = true;
                    continue;
                }
                
                tstring sKey(Trim(tsKeyValue[0]));
                tstring sValue(Trim(tsKeyValue[1]));
                if (sValue.empty())
                {
                    bWarnInvalid = true;
                    continue;
                }

                if (sKey == _T("maxstacks"))
                {
                    m_uiMaxCreepStacks = AtoI(sValue);
                }
                else if (sKey == _T("radius"))
                {
                    m_uiNoRespawnRadius = AtoI(sValue);
                }
                else if (sKey == _T("respawn"))
                {
                    m_uiRespawnInterval = AtoI(sValue);
                }
                else if (sKey == _T("chasetime"))
                {
                    m_uiGuardChaseTime = AtoI(sValue);
                }
                else if (sKey == _T("chasedistance"))
                {
                    m_uiGuardChaseDistance = AtoI(sValue);
                }
                else if (sKey == _T("reaggrochasetime"))
                {
                    m_uiGuardReaggroChaseTime = AtoI(sValue);
                }
                else if (sKey == _T("reaggrochasedistance"))
                {
                    m_uiGuardReaggroChaseDistance = AtoI(sValue);
                }
                else if (sKey == _T("aggrorange"))
                {
                    m_uiOverrideAggroRange = AtoI(sValue);
                }
                else
                {
                    Console.Warn << _T("Neutral camp controller config string has an unknown key: ") << sKey << newl;
                }
            }

            if (bWarnInvalid)
            {
                Console.Warn << _T("Neutral camp controller config string is not valid: ") << sValue << newl;
            }
        }
    }
}


/*====================
  CEntityNeutralCampController::Spawn
  ====================*/
void    CEntityNeutralCampController::Spawn()
{
    uint uiRespawnInterval(m_uiRespawnInterval);
    if (uiRespawnInterval == 0)
        uiRespawnInterval = g_neutralRespawnInterval;
    m_uiNextSpawnTime = uiRespawnInterval;
    m_bInitialSpawnAttempted = false;
    m_uiNumActive = 0;
}


/*====================
  CEntityNeutralCampController::GameStart
  ====================*/
void    CEntityNeutralCampController::GameStart()
{
    // Register spawners, assumes the vectors are sync'd
    NeutralSpawnerUIDArray_it itUIDArray(m_vSpawnerUIDArray.begin());
    for (NeutralSpawnerArray_it itArray(m_vSpawnerArray.begin()), itArrayEnd(m_vSpawnerArray.end()); itArray != itArrayEnd; ++itArray, ++itUIDArray)
    {
        NeutralSpawnerUIDList_it itUIDList(itUIDArray->begin());
        for (NeutralSpawnerList_it itList(itArray->begin()), itListEnd(itArray->end()); itList != itListEnd; ++itList, ++itUIDList)
        {
            if (itList->empty())
                *itUIDList = INVALID_INDEX;
            else
            {
                IVisualEntity *pTarget(Game.GetEntityFromName(*itList));
                if (pTarget != NULL)
                    *itUIDList = pTarget->GetUniqueID();
                else
                {
                    Console.Warn << _T("Target ") << SingleQuoteStr(*itList) << _T(" not found") << newl;
                    *itUIDList = INVALID_INDEX;
                }
            }
        }
    }
}


/*====================
  CEntityNeutralCampController::ServerFrameThink
  ====================*/
bool    CEntityNeutralCampController::ServerFrameThink()
{
    if (m_vSpawnerArray.empty())
        return false;

    if (!IVisualEntity::ServerFrameThink())
        return false;

    // Wait for initial spawn timer
    if (Game.GetMatchTime() >= g_neutralFirstSpawn && !m_bInitialSpawnAttempted)
    {
        AttemptSpawn();
        m_bInitialSpawnAttempted = true;
    }
    
    // Check respawn interval
    uint uiRespawnInterval(m_uiRespawnInterval);
    if (uiRespawnInterval == 0)
        uiRespawnInterval = g_neutralRespawnInterval;
    if (m_uiNextSpawnTime != INVALID_TIME && Game.GetMatchTime() - m_uiNextSpawnTime < uiRespawnInterval)
    {
        AttemptSpawn();
        m_uiNextSpawnTime += uiRespawnInterval;
    }

    return true;
}


/*====================
  CEntityNeutralCampController::AttemptSpawn
  ====================*/
bool    CEntityNeutralCampController::AttemptSpawn()
{
    PROFILE("CEntityNeutralCampController::AttemptSpawn");

    // Don't respawn if something is near (including corpses)
    static uivector vEntities;
    uint uiNoRespawnRadius(m_uiNoRespawnRadius);
    if (uiNoRespawnRadius == 0)
        uiNoRespawnRadius = g_neutralNoRespawnProximity;
    Game.GetEntitiesInRadius(vEntities, GetPosition().xy(), uiNoRespawnRadius, REGION_UNIT);
    for (uivector_it it(vEntities.begin()); it != vEntities.end(); ++it)
    {
        IUnitEntity *pUnit(Game.GetUnitEntity(Game.GetGameIndexFromWorldIndex(*it)));
        if (pUnit == NULL)
            continue;
        if (pUnit->GetNoBlockNeutralSpawn())
            continue;
        if (pUnit->GetStatus() != ENTITY_STATUS_ACTIVE &&
            pUnit->GetStatus() != ENTITY_STATUS_DEAD &&
            pUnit->GetStatus() != ENTITY_STATUS_CORPSE)
            continue;

        return true;
    }

    // if there are too many creep stacks spawned, then don't spawn more.
    if (m_uiMaxCreepStacks > 0 && NumCreepStacks() >= m_uiMaxCreepStacks)
        return true;

    // Respawn
    m_vCreepStacks.push_back(NeutralCreepStack());
    NeutralCreepStack& creepStack(*m_vCreepStacks.rbegin());
    NeutralSpawnerUIDList &vSpawners(m_vSpawnerUIDArray[M_Randnum(0u, INT_SIZE(m_vSpawnerArray.size()) - 1)]);
    for (NeutralSpawnerUIDList_it it(vSpawners.begin()); it != vSpawners.end(); ++it)
    {
        IGameEntity *pSpawner(Game.GetEntityFromUniqueID(*it));
        assert(pSpawner);
        if (pSpawner == NULL)
            continue;

        CEntityNeutralCampSpawner *pNeutralSpawner(pSpawner->GetAsNeutralCampSpawner());
        assert(pNeutralSpawner);
        if (pNeutralSpawner == NULL)
            continue;

        pSpawner->Trigger(this);

        IUnitEntity *pUnit(pNeutralSpawner->GetSpawnedUnit());
        if (pUnit == NULL)
            continue;

        assert(pUnit->GetUniqueID() != INVALID_INDEX);
        creepStack.push_back(pUnit->GetUniqueID());

        pUnit->SetGuardChaseTime(m_uiGuardChaseTime);
        pUnit->SetGuardChaseDistance(m_uiGuardChaseDistance);
        pUnit->SetGuardReaggroChaseTime(m_uiGuardReaggroChaseTime);
        pUnit->SetGuardReaggroChaseDistance(m_uiGuardReaggroChaseDistance);
        pUnit->OverrideAggroRange(m_uiOverrideAggroRange);

        ++m_uiNumActive;
    }

    // Set "revealed", which for camps means the team has knowledge
    // of whether this camp is destroyed or not
    RemoveVisibilityFlags(VIS_REVEALED(1));
    RemoveVisibilityFlags(VIS_REVEALED(2));

    return true;
}

/*====================
  CEntityNeutralCampController::CreepDied
  ====================*/
void    CEntityNeutralCampController::CreepDied(uint iCreepID)
{
    --m_uiNumActive;

    for (NeutralCreepStackList::iterator it = m_vCreepStacks.begin(); it != m_vCreepStacks.end();)
    {
        NeutralCreepStack& creepStack(*it);
        creepStack.remove(iCreepID);
        if (creepStack.empty())
            it = m_vCreepStacks.erase(it);
        else
            ++it;
    }
}


/*====================
  CEntityNeutralCampController::GetMapIconColor
  ====================*/
CVec4f  CEntityNeutralCampController::GetMapIconColor(CPlayer *pLocalPlayer) const
{
    uint uiIndex(CLAMP(m_uiLevel, 1u, 4u) - 1);

    return CVec4f(g_neutralCampMapIconColor[uiIndex], 1.0f);
}
