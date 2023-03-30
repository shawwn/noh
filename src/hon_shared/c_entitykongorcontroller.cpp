// (C)2008 S2 Games
// c_entitykongorcontroller.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_entitykongorcontroller.h"

#include "i_unitentity.h"
#include "i_heroentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Entity, BossController)

CVAR_FLOATF(    g_bossMapIconSize,      0.05f,                              CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_STRINGF(   g_bossMapIcon,          "/shared/icons/minimap_circle.tga", CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_VEC4F(     g_bossMapIconColor,     CVec4f(0.6f, 0.0f, 0.0f, 1.0f),     CVAR_TRANSMIT | CVAR_GAMECONFIG);

ResHandle   CEntityBossController::s_hMinimapIcon(INVALID_RESOURCE);
//=============================================================================

/*====================
  CEntityBossController::CEntityBossController
  ====================*/
CEntityBossController::CEntityBossController() :
m_fMapIconSize(0.0f),
m_hMapIcon(INVALID_RESOURCE),
m_v4MapIconColor(WHITE),
m_uiFirstSpawnTime(INVALID_TIME),
m_uiRespawnInterval(INVALID_TIME),
m_uiNextRespawnTime(INVALID_TIME),
m_uiMaxLevel(1),
m_uiLevel(0)
{
}


/*====================
  CEntityBossController::ClientPrecache
  ====================*/
void    CEntityBossController::ClientPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier)
{
    s_hMinimapIcon = Game.RegisterIcon(g_bossMapIcon);
}


/*====================
  CEntityBossController::ApplyWorldEntity
  ====================*/
void    CEntityBossController::ApplyWorldEntity(const CWorldEntity &ent)
{
    IVisualEntity::ApplyWorldEntity(ent);

    m_fMapIconSize = ent.GetPropertyFloat(_T("iconsize"));
    m_hMapIcon = Game.RegisterIcon(ent.GetProperty(_T("icon")));
    m_v4MapIconColor = GetColorFromString(ent.GetProperty(_T("iconcolor")));
    m_uiFirstSpawnTime = ent.GetPropertyInt(_T("firstspawntime"));
    m_uiRespawnInterval = ent.GetPropertyInt(_T("respawninterval"));
    m_uiLevel = ent.GetPropertyInt(_T("firstlevel"));
    m_uiMaxLevel = ent.GetPropertyInt(_T("maxlevel"));

    for (uint ui(0); ; ++ui)
    {
        tstring sPropertyName(_T("target") + XtoA(ui));
        if (!ent.HasProperty(sPropertyName))
            break;

        const tstring &sValue(ent.GetProperty(sPropertyName));
        if (sValue.empty())
            break;

        m_vSpawnerArray.push_back((TokenizeString(sValue, _T(','))));
        m_vSpawnerUIDArray.push_back(SpawnerUIDList(m_vSpawnerArray.back().size(), INVALID_INDEX));
    }
}


/*====================
  CEntityBossController::Spawn
  ====================*/
void    CEntityBossController::Spawn()
{
    m_uiNextRespawnTime = INVALID_TIME;
}


/*====================
  CEntityBossController::GameStart
  ====================*/
void    CEntityBossController::GameStart()
{
    // Register spawners, assumes the vectors are sync'd
    SpawnerUIDArray_it itUIDArray(m_vSpawnerUIDArray.begin());
    for (SpawnerArray_it itArray(m_vSpawnerArray.begin()), itArrayEnd(m_vSpawnerArray.end()); itArray != itArrayEnd; ++itArray, ++itUIDArray)
    {
        SpawnerUIDList_it itUIDList(itUIDArray->begin());
        for (SpawnerList_it itList(itArray->begin()), itListEnd(itArray->end()); itList != itListEnd; ++itList, ++itUIDList)
        {
            if (itList->empty())
            {
                *itUIDList = INVALID_INDEX;
                continue;
            }

            IVisualEntity *pTarget(Game.GetEntityFromName(*itList));
            if (pTarget != NULL)
            {
                *itUIDList = pTarget->GetUniqueID();
                continue;
            }

            Console.Warn << _T("Target ") << SingleQuoteStr(*itList) << _T(" not found") << newl;
            *itUIDList = INVALID_INDEX;
        }
    }
}


/*====================
  CEntityBossController::ServerFrameThink
  ====================*/
bool    CEntityBossController::ServerFrameThink()
{
    if (m_vSpawnerArray.empty())
        return false;

    if (!IVisualEntity::ServerFrameThink())
        return false;

    // Check for active bosses
    if (!m_setActiveBossUIDs.empty())
    {
        uiset_it it(m_setActiveBossUIDs.begin());
        while (it != m_setActiveBossUIDs.end())
        {
            IUnitEntity *pUnit(Game.GetUnitEntity(Game.GetGameIndexFromUniqueID(*it)));

            if (pUnit == NULL || pUnit->GetStatus() != ENTITY_STATUS_ACTIVE)
            {
                STL_ERASE(m_setActiveBossUIDs, it);
                continue;
            }

            ++it;
        }

        if (m_setActiveBossUIDs.empty())
        {
            m_uiNextRespawnTime = Game.GetMatchTime() + m_uiRespawnInterval;
            SetStatus(ENTITY_STATUS_DORMANT);
        }
        else
        {
            SetStatus(ENTITY_STATUS_ACTIVE);
            return true;
        }
    }


    // Wait for initial spawn timer
    if (Game.GetMatchTime() < m_uiFirstSpawnTime)
        return true;

    if (m_uiNextRespawnTime != INVALID_TIME && m_uiNextRespawnTime > Game.GetMatchTime())
        return true;

    return AttemptSpawn();
}


/*====================
  CEntityBossController::AttemptSpawn
  ====================*/
bool    CEntityBossController::AttemptSpawn()
{
    if (!m_setActiveBossUIDs.empty())
        return true;

    // Respawn
    SpawnerUIDList &vSpawners(m_vSpawnerUIDArray[M_Randnum(0u, INT_SIZE(m_vSpawnerArray.size()) - 1)]);
    for (SpawnerUIDList_it it(vSpawners.begin()); it != vSpawners.end(); ++it)
    {
        IGameEntity *pSpawner(Game.GetEntityFromUniqueID(*it));
        if (pSpawner == NULL)
            continue;

        pSpawner->Trigger(this);
    }

    // Set "revealed", which for camps means the team knowledge
    // of whether this camp is destroyed or not
    RemoveVisibilityFlags(VIS_REVEALED(1));
    RemoveVisibilityFlags(VIS_REVEALED(2));

    m_uiNextRespawnTime = INVALID_TIME;
    if (m_uiLevel < m_uiMaxLevel)
        ++m_uiLevel;
    return true;
}