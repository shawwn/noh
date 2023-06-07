// (C)2006 S2 Games
// i_buildingentity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_buildingentity.h"
#include "c_teaminfo.h"
#include "c_entityclientinfo.h"

#include "../k2/c_model.h"
#include "../k2/c_worldentity.h"
#include "../k2/c_sceneentity.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_eventscript.h"
#include "../k2/c_texture.h"
#include "../k2/c_effect.h"
#include "../k2/c_skeleton.h"
#include "../k2/c_uitrigger.h"
#include "../k2/intersection.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
extern CCvarui g_economyInterval;
extern CCvar<float>     p_stepHeight;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
vector<SDataField>* IBuildingEntity::s_pvFields;

CVAR_UINTF(     g_buildingCorpseTime,               10000,  CVAR_GAMECONFIG);
CVAR_FLOATF(    g_buildingProximityTest,            500.0f, CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    g_buildingLowHealthPercent,         0.33f,  CVAR_GAMECONFIG);
CVAR_FLOATF(    g_buildingMaxTerrainVariance,       64.0f,  CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    g_buildingIncompleteArmorFactor,    0.25f,  CVAR_GAMECONFIG);
CVAR_FLOATF(    g_buildingDamageConstructDelay,     2500,   CVAR_GAMECONFIG);
CVAR_FLOATF(    g_buildingUpkeepFailedArmorMult,    0.6f,   CVAR_GAMECONFIG);
CVAR_FLOATF(    g_buildingDestroyedRefund,          1.0f,   CVAR_GAMECONFIG);

CVAR_FLOATF(    g_armorBreakRate,                   0.01f,  CVAR_GAMECONFIG);
CVAR_FLOATF(    g_armorRecoverRate,                 0.1f,   CVAR_GAMECONFIG);
CVAR_FLOATF(    g_armorBreakLimit,                  0.0f,   CVAR_GAMECONFIG);
//=============================================================================

/*====================
  IBuildingEntity::CEntityConfig::CEntityConfig
  ====================*/
IBuildingEntity::CEntityConfig::CEntityConfig(const tstring &sName) :
IVisualEntity::CEntityConfig(sName),
INIT_ENTITY_CVAR(UpkeepCost, 10),
INIT_ENTITY_CVAR(DecayRate, 0.05f),
INIT_ENTITY_CVAR(Foundation, _T("")),
INIT_ENTITY_CVAR(BuildHealth, 0.05f),
INIT_ENTITY_CVAR(BuildTime, 15000),
INIT_ENTITY_CVAR(Armor, 0.9f),
INIT_ENTITY_CVAR(LowHealthEffectPath, _T("")),
INIT_ENTITY_CVAR(IsMajorBuilding, false),
INIT_ENTITY_CVAR(SelectSoundPath, _T("")),
INIT_ENTITY_CVAR(SelectConstructionSoundPath, _T("")),
INIT_ENTITY_CVAR(ConstructionCompleteSoundPath, _T("")),
INIT_ENTITY_CVAR(ConstructionStartedSoundPath, _T("")),
INIT_ENTITY_CVAR(DestroyedSoundPath, _T("")),
INIT_ENTITY_CVAR(DestroyedSoundPathHuman, _T("")),
INIT_ENTITY_CVAR(DestroyedSoundPathBeast, _T("")),
INIT_ENTITY_CVAR(LowHealthSoundPath, _T("")),
INIT_ENTITY_CVAR(StateRadius, 0.f),
INIT_ENTITY_CVAR(StateTargetEnemy, false),
INIT_ENTITY_CVAR(StateTargetAlly, false),
INIT_ENTITY_CVAR(StateDuration, 0),
INIT_ENTITY_CVAR(StateName, _T("")),
INIT_ENTITY_CVAR(Stateselfapply, false),
INIT_ENTITY_CVAR(StatePreviewColor, CVec4f(0.00f, 0.00f, 0.50f, 1.00f)),
INIT_ENTITY_CVAR(UniqueStructureRadius, 0.0f),
INIT_ENTITY_CVAR(MaximumDistanceFromMain, 0.0f),
INIT_ENTITY_CVAR(MaximumDistanceFromMajor, 0.0f),
INIT_ENTITY_CVAR(MaximumDistanceFromCommandStructure, 0.0f),
INIT_ENTITY_CVAR(MinimumDistanceFromCommandStructure, 0.0f),
INIT_ENTITY_CVAR(IsMainBuilding, true),
INIT_ENTITY_CVAR(UniqueRadiusEffectPath, _T("")),
INIT_ENTITY_CVAR(StateEffectPreviewPath, _T("")),
INIT_ENTITY_CVAR(StateRequiresUpkeep, false),
INIT_ENTITY_CVAR(BuildingCheckUpscale, 1.2f),
INIT_ENTITY_CVAR(MaxBuild, 0),
INIT_ENTITY_CVAR(IsCommandStructure, false)
{
}


/*====================
  IBuildingEntity::~IBuildingEntity
  ====================*/
IBuildingEntity::~IBuildingEntity()
{
    if (m_uiWorldIndex != INVALID_INDEX && Game.WorldEntityExists(m_uiWorldIndex))
    {
        Game.UnlinkEntity(m_uiWorldIndex);
        Game.DeleteWorldEntity(m_uiWorldIndex);
    }

    if (m_iTeam != -1)
    {
        CEntityTeamInfo *pTeamInfo(Game.GetTeam(m_iTeam));

        if (pTeamInfo)
        {
            pTeamInfo->RemoveBuildingIndex(m_uiIndex);
            if (IsSpawnLocation())
                pTeamInfo->RemoveSpawnBuildingIndex(m_uiIndex);
        }
    }
}


/*====================
  IBuildingEntity::IBuildingEntity
  ====================*/
IBuildingEntity::IBuildingEntity(CEntityConfig *pConfig) :
IVisualEntity(pConfig),
m_pEntityConfig(pConfig),

m_fUpkeepLevel(1.0f),
m_uiFoundation(INVALID_INDEX),

m_fStartBuildingHealthPool(0.0f),
m_fBuildingHealthPool(0.0f),
m_uiStartBuildTime(0),
m_uiLastGameTime(0),
m_uiLastIncomeTime(0),
m_fStartBuildingHealth(0.0f),
m_uiLastDamageTime(0),
m_bLowHealthEffectActive(false),
m_uiBuildAnimTime(0),
m_fFoundationScale(1.0f),

m_fRepairCostAccumulator(0.0f),
m_iBountyRemaining(0),
m_fArmorBreakPenalty(0.0f),

m_uiPoseTime(0)

{
}


/*====================
  IBuildingEntity::Baseline
  ====================*/
void    IBuildingEntity::Baseline()
{
    IVisualEntity::Baseline();

    m_uiFoundation = INVALID_INDEX;
    m_uiBuildAnimTime = 0;
    m_uiStartBuildTime = 0;
    m_fFoundationScale = 1.0f;
    m_iBountyRemaining = 0;
    m_fArmorBreakPenalty = 0.0f;
}


/*====================
  IBuildingEntity::GetSnapshot
  ====================*/
void    IBuildingEntity::GetSnapshot(CEntitySnapshot &snapshot) const
{
    IVisualEntity::GetSnapshot(snapshot);

    snapshot.AddGameIndex(m_uiFoundation);
    snapshot.AddField(m_uiBuildAnimTime);
    snapshot.AddField(m_uiStartBuildTime);
    snapshot.AddField(m_fFoundationScale);
    snapshot.AddBytePercent(m_fArmorBreakPenalty);
}


/*====================
  IBuildingEntity::ReadSnapshot
  ====================*/
bool    IBuildingEntity::ReadSnapshot(CEntitySnapshot &snapshot)
{
    try
    {
        // Base entity info
        if (!IVisualEntity::ReadSnapshot(snapshot))
            return false;

        snapshot.ReadNextGameIndex(m_uiFoundation);
        snapshot.ReadNextField(m_uiBuildAnimTime);
        snapshot.ReadNextField(m_uiStartBuildTime);
        snapshot.ReadNextField(m_fFoundationScale);
        snapshot.ReadNextBytePercent(m_fArmorBreakPenalty);
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("IPlayerEntity::ReadSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  IBuildingEntity::GetTypeVector
  ====================*/
const vector<SDataField>&   IBuildingEntity::GetTypeVector()
{
    if (!s_pvFields)
    {
        s_pvFields = K2_NEW(global,   vector<SDataField>)();
        s_pvFields->clear();
        const vector<SDataField> &vBase(IVisualEntity::GetTypeVector());
        s_pvFields->insert(s_pvFields->begin(), vBase.begin(), vBase.end());

        s_pvFields->push_back(SDataField(_T("m_uiFoundation"), FIELD_PUBLIC, TYPE_GAMEINDEX));
        s_pvFields->push_back(SDataField(_T("m_uiBuildAnimTime"), FIELD_PUBLIC, TYPE_INT));
        s_pvFields->push_back(SDataField(_T("m_uiStartBuildTime"), FIELD_PUBLIC, TYPE_INT));
        s_pvFields->push_back(SDataField(_T("m_fFoundationScale"), FIELD_PUBLIC, TYPE_FLOAT));
        s_pvFields->push_back(SDataField(_T("m_fArmorBreakPenalty"), FIELD_PUBLIC, TYPE_BYTEPERCENT));
    }

    return *s_pvFields;
}


/*====================
  IBuildingEntity::AllocateSkeleton
  ====================*/
CSkeleton*  IBuildingEntity::AllocateSkeleton()
{
    m_pSkeleton = K2_NEW(global,   CSkeleton);
    return m_pSkeleton;
}


/*====================
  IBuildingEntity::Link
  ====================*/
void    IBuildingEntity::Link()
{
    if (m_uiWorldIndex != INVALID_INDEX)
    {
        CWorldEntity *pWorldEnt(Game.GetWorldEntity(m_uiWorldIndex));
        
        if (pWorldEnt != NULL)
        {
            pWorldEnt->SetPosition(GetPosition());
            pWorldEnt->SetScale(GetScale());
            pWorldEnt->SetScale2(GetScale2());
            pWorldEnt->SetAngles(GetAngles());
            pWorldEnt->SetModelHandle(GetModelHandle());
            pWorldEnt->SetGameIndex(GetIndex());

            uint uiLinkFlags(SURF_BUILDING);

            if (IsIntangible())
                uiLinkFlags |= SURF_INTANGIBLE;

            if (GetStatus() != ENTITY_STATUS_DEAD)
                Game.LinkEntity(m_uiWorldIndex, LINK_SURFACE | LINK_MODEL, uiLinkFlags);

            const vector<CConvexPolyhedron> &cWorldSurfs(pWorldEnt->GetWorldSurfsRef());
            for (vector<CConvexPolyhedron>::const_iterator cit(cWorldSurfs.begin()); cit != cWorldSurfs.end(); ++cit)
                Game.BlockPath(NAVIGATION_BUILDING, *cit, p_stepHeight);
        }
    }
}


/*====================
  IBuildingEntity::Unlink
  ====================*/
void    IBuildingEntity::Unlink()
{
    if (m_uiWorldIndex != INVALID_INDEX)
    {
        CWorldEntity *pWorldEnt(Game.GetWorldEntity(m_uiWorldIndex));
        
        if (pWorldEnt != NULL)
        {
            const vector<CConvexPolyhedron> &cWorldSurfs(pWorldEnt->GetWorldSurfsRef());
            for (vector<CConvexPolyhedron>::const_iterator cit(cWorldSurfs.begin()); cit != cWorldSurfs.end(); ++cit)
                Game.ClearPath(NAVIGATION_BUILDING, *cit);
        }

        Game.UnlinkEntity(m_uiWorldIndex);
    }
}


/*====================
  IBuildingEntity::GetGameTip
  ====================*/
bool    IBuildingEntity::GetGameTip(IPlayerEntity *pPlayer, tstring &sMessage)
{
    if (pPlayer == NULL)
        return false;

    if (GetTeam() != pPlayer->GetTeam())
        return false;

    CEntityTeamInfo *pTeam(Game.GetTeam(pPlayer->GetTeam()));
    if (GetHealth() < GetMaxHealth() && pPlayer->GetCanBuild())
    {
        if (GetStatus() == ENTITY_STATUS_SPAWNING)
            sMessage = _T("This building is under construction.  Hold down the 'R' key to help build it.");
        else if (Game.IsSuddenDeathActive())
            sMessage = _T("Sudden death is in effect, you cannot repair this structure.");
        else if (HasNetFlags(ENT_NET_FLAG_NO_REPAIR))
            sMessage = _T("Your commander has disabled the repairing of this structure.");
        else if (pTeam != NULL && pTeam->GetGold() <= 0)
            sMessage = _T("Your team is out of gold and you cannot afford to repair this structure.");
        else
            sMessage = _T("This building is damaged.  Hold down the 'R' key to repair it.");
        return true;
    }

    return IVisualEntity::GetGameTip(pPlayer, sMessage);
}


/*====================
  IBuildingEntity::CanBuild
  ====================*/
bool    IBuildingEntity::CanBuild(const IPlayerEntity *pBuilder, tstring &sCanBuild)
{
    PROFILE("IBuildingEntity::CanBuild");

    if (pBuilder == NULL || !pBuilder->GetCanBuild())
        return false;

    CEntityTeamInfo *pTeam(Game.GetTeam(pBuilder->GetTeam()));
    if (pTeam == NULL)
        return false;

    if (Game.GetGamePhase() != GAME_PHASE_ACTIVE)
    {
        sCanBuild = _T("No building is allowed during warmup");
        return false;
    }

    if (pTeam->GetGold() < GetCost())
    {
        sCanBuild = _T("The team can not afford this building");
        return false;
    }

    // Prerequisites
    svector vPrerequisites(TokenizeString(m_pEntityConfig->GetPrerequisite(), _T(' ')));
    for (svector_it it(vPrerequisites.begin()); it != vPrerequisites.end(); ++it)
    {
        if (!pTeam->HasBuilding(*it))
        {
            ICvar *pCvar(g_EntityRegistry.GetGameSetting(g_EntityRegistry.LookupID(*it), _T("Name")));
            if (pCvar == NULL)
                sCanBuild = _T("Missing prerequisite: ") + *it;
            else
                sCanBuild = _T("Missing prerequisite: ") + pCvar->GetString();
            return false;
        }
    }

    if (m_uiFoundation == INVALID_INDEX)
    {
        // Terrain
        CBBoxf bbBounds(g_ResourceManager.GetModelSurfaceBounds(m_hModel));
        bbBounds.Transform(m_v3Position, CAxis(m_v3Angles), GetBuildingScale());

        float fScale(Game.GetWorldPointer()->GetScale());
        CRecti recArea(INT_FLOOR(bbBounds.GetMin().x / fScale), INT_FLOOR(bbBounds.GetMin().y / fScale), INT_CEIL(bbBounds.GetMax().x / fScale), INT_CEIL(bbBounds.GetMax().y / fScale));
        float fMinTerrain(Game.CalcMinTerrainHeight(recArea));
        float fMaxTerrain(Game.CalcMaxTerrainHeight(recArea));

        if (fMaxTerrain - fMinTerrain > g_buildingMaxTerrainVariance)
        {
            sCanBuild = _T("The terrain varies too much to place the structure here");
            return false;
        }

        bool bBlocked(Game.CalcBlocked(recArea));

        if (bBlocked)
        {
            sCanBuild = _T("The terrain is blocked here");
            return false;
        }
    }

    // Loop to check that building being placed is within MaximumDistanceFromBase radius. If radius is 0 or less, restriction does not apply.
    if (m_pEntityConfig != NULL && (GetMaximumDistanceFromMain() > 0 || GetMaximumDistanceFromMajor() > 0 || GetMaximumDistanceFromCommandStructure() > 0))
    {
        uivector vResultCommandStructure;
        uivector vResultMain;
        uivector vResultMajor;
        Game.GetEntitiesInRadius(vResultCommandStructure, CSphere(GetPosition(), GetMaximumDistanceFromCommandStructure()), 0);
        Game.GetEntitiesInRadius(vResultMain, CSphere(GetPosition(), GetMaximumDistanceFromMain()), 0);
        Game.GetEntitiesInRadius(vResultMajor, CSphere(GetPosition(), GetMaximumDistanceFromMajor()), 0);
        bool bMainCheck(GetMaximumDistanceFromMain() != 0);
        bool bMainPassed(false);
        bool bMajorCheck(GetMaximumDistanceFromMajor() != 0);
        bool bMajorPassed(false);
        bool bCommandCheck(GetMaximumDistanceFromCommandStructure() != 0);
        bool bCommandPassed(false);

        //checks for command structure within command structure radius
        if (bCommandCheck)
        {
            for (uivector_it it(vResultCommandStructure.begin()); it != vResultCommandStructure.end(); ++it)
            {
                IVisualEntity *pEntprev(Game.GetEntityFromWorldIndex(*it));
                IBuildingEntity *pBuilding(pEntprev->GetAsBuilding());
                if (pEntprev->IsBuilding() && pEntprev->GetTeam() == pBuilder->GetTeam() && pBuilding->GetStatus() == ENTITY_STATUS_ACTIVE)
                {

                    if (pBuilding->GetIsCommandStructure())
                        bCommandPassed = true;
                }

                if (bCommandPassed)
                    break;
                
            }
        }

        //checks for main structure within main structure radius
        if (bMainCheck)
        {
            for (uivector_it it(vResultMain.begin()); it != vResultMain.end(); ++it)
            {
                IVisualEntity *pEntprev(Game.GetEntityFromWorldIndex(*it));
                IBuildingEntity *pBuilding(pEntprev->GetAsBuilding());
                if (pEntprev->IsBuilding() && pEntprev->GetTeam() == pBuilder->GetTeam() && pBuilding->GetStatus() == ENTITY_STATUS_ACTIVE)
                {

                    if (pBuilding->GetIsMainBuilding())
                        bMainPassed = true;
                }

                if (bMainPassed)
                    break;
            }
        }
        //checks for major structure within major structure radius
        if (bMajorCheck)
        {
            for (uivector_it it(vResultMajor.begin()); it != vResultMajor.end(); ++it)
            {
                IVisualEntity *pEntprev(Game.GetEntityFromWorldIndex(*it));
                IBuildingEntity *pBuilding(pEntprev->GetAsBuilding());
                if (pEntprev->IsBuilding() && pEntprev->GetTeam() == pBuilder->GetTeam() && pBuilding->GetStatus() == ENTITY_STATUS_ACTIVE)
                {

                    if (pBuilding->GetIsMajorBuilding())
                        bMajorPassed = true;
                }

                if (bMajorPassed)
                    break;
            }
        }


        if(!bMainPassed && !bMajorPassed && !bCommandPassed)
        {
            if (!bMainPassed && bMainCheck)
                sCanBuild = _T("This building is outside its maximum distance from a main allied structure");

            if(!bMajorPassed && bMainCheck)
                sCanBuild = _T("This building is outside its maximum distance from a major allied structure");

            if(!bCommandPassed && bCommandCheck)
                sCanBuild = _T("This building is outside its maximum distance from your command structure");
            if(!bCommandPassed && bCommandCheck && !bMainPassed && bMainCheck)
                sCanBuild = _T("This building is outside its maximum distance from an allied garrison or sublair");

            
            return false;
        }
    }

    // Proximity
    bool bHasProximity(false);
    uivector vEntities;
    Game.GetEntitiesInRadius(vEntities, CSphere(GetPosition(), g_buildingProximityTest), 0);
    for (uivector_it it(vEntities.begin()); it != vEntities.end(); ++it)
    {
        IVisualEntity *pEntity(Game.GetEntityFromWorldIndex(*it));
        if (pEntity == NULL)
            continue;
        if (pEntity->GetTeam() != pBuilder->GetTeam())
            continue;
        if (!pEntity->IsPlayer() && !pEntity->IsBuilding() && !pEntity->IsPet())
            continue;

        bHasProximity = true;
        break;
    }
    if (!bHasProximity)
    {
        sCanBuild = _T("No player or building is in proximity to this building");
        return false;
    }

    uint uiCollisionFlags(m_uiFoundation == INVALID_INDEX ?
        SURF_MODEL | SURF_CORPSE | SURF_SHIELD | SURF_PROJECTILE:
        SURF_MODEL | SURF_CORPSE | SURF_SHIELD | SURF_PROJECTILE | SURF_FOUNDATION);

    // Collision
    vEntities.clear();
    const SurfVector &vSurfs(g_ResourceManager.GetModelSurfaces(m_hModel));
    for (SurfVector::const_iterator it(vSurfs.begin()); it != vSurfs.end(); ++it)
    {
        CConvexPolyhedron cSurf(*it);
        cSurf.Transform(m_v3Position, CAxis(m_v3Angles), GetBuildingScale() * m_pEntityConfig->GetBuildingCheckUpscale());

        Game.GetEntitiesInSurface(vEntities, cSurf, uiCollisionFlags);
        for (uivector_it it(vEntities.begin()); it != vEntities.end(); ++it)
        {
            IVisualEntity *pEntity(Game.GetEntityFromWorldIndex(*it));
            if (pEntity == NULL)
                continue;
            if (pEntity->GetIndex() == m_uiFoundation)
                continue;

            pEntity->AddClientRenderFlags(ECRF_SNAPSELECTED);
            pEntity->SetSelectColor(CVec4f(2.0f, 0.0f, 0.0f, 1.0f));

            sCanBuild = _T("Collision with a nearby object");
            return false;
        }
    }

    // If a building is being placed, check that no other buildings of same type within UniqueStructureRadius
    uivector vResult;
    Game.GetEntitiesInRadius(vResult, CSphere(GetPosition(), m_pEntityConfig->GetUniqueStructureRadius()), 0);

    for (uivector_it it(vResult.begin()); it != vResult.end(); ++it)
    {
        IVisualEntity *pEntprev(Game.GetEntityFromWorldIndex(*it));
        if (pEntprev == NULL)
            continue;               
        if (pEntprev->GetStatus() == ENTITY_STATUS_DEAD)
            continue;
        if (pEntprev->GetType() != GetType())
            continue;
        
        sCanBuild = _T("There is another building of the same type within allowable radius");
        return false;
    }

    if (m_pEntityConfig != NULL && (GetMinimumDistanceFromCommandStructure() > 0))
    {
        // Checks structure is not within the minimum distance to the SH
        Game.GetEntitiesInRadius(vResult, CSphere(GetPosition(), m_pEntityConfig->GetMinimumDistanceFromCommandStructure()), 0);

        for (uivector_it it(vResult.begin()); it != vResult.end(); ++it)
        {
            IVisualEntity *pEntprev(Game.GetEntityFromWorldIndex(*it));
            if (pEntprev == NULL)
                continue;               
            if (pEntprev->GetStatus() == ENTITY_STATUS_DEAD)
                continue;
            
            IBuildingEntity *pBuilding(pEntprev->GetAsBuilding());
            
            if (pBuilding == NULL)
                continue;

            if (!pBuilding->GetIsCommandStructure() || pBuilding->GetTeam() != pBuilder->GetTeam())
                continue;
            
            sCanBuild = _T("The structure you are attempting to place is too close to your command structure.");
            return false;
        }
    }

    sCanBuild.clear();
    return true;
}


/*====================
  IBuildingEntity::Spawn
  ====================*/
void    IBuildingEntity::Spawn()
{
    if (GetStatus() == ENTITY_STATUS_PREVIEW)
    {
        SpawnPreview();
        return;
    }

    IVisualEntity::Spawn();

    // Most buildings shouldn't belong to a squad
    SetSquad(-1);

    SetModelHandle(Game.RegisterModel(GetModelPath()));
    m_bbBounds = g_ResourceManager.GetModelSurfaceBounds(GetModelHandle());
    m_fScale = GetBuildingScale() * m_fFoundationScale;
    m_bbBounds *= m_fScale;

    if (m_pSkeleton)
        m_pSkeleton->SetModel(m_hModel);

    if (m_pEntityConfig->GetBuildHealth() >= GetMaxHealth() || m_pEntityConfig->GetBuildTime() == 0)
    {
        SetStatus(ENTITY_STATUS_ACTIVE);
        m_fStartBuildingHealth = m_fHealth = GetMaxHealth();
    }
    else
    {
        SetStatus(ENTITY_STATUS_SPAWNING);
        m_fStartBuildingHealth = m_fHealth = GetMaxHealth() * m_pEntityConfig->GetBuildHealth();
    }
    
    m_fStartBuildingHealthPool = m_fBuildingHealthPool = GetMaxHealth() - m_fStartBuildingHealth;
    m_uiLastIncomeTime = m_uiLastGameTime = m_uiStartBuildTime = Game.GetGameTime();
    m_iBountyRemaining = m_pEntityConfig->GetBounty();

    if (m_uiWorldIndex == INVALID_INDEX)
        m_uiWorldIndex = Game.AllocateNewWorldEntity();

    StartAnimation(_T("idle"), 0);
    m_yDefaultAnim = m_ayAnim[0];
    
    if (GetStatus() == ENTITY_STATUS_SPAWNING)
        StartAnimation(_T("build"), 0, 1.0f, GetBuildTime());
    
    Link();

    if (Game.IsServer())
    {
        ivector vClients(Game.GetTeam(GetTeam())->GetClientList());

        CBufferFixed<3> buffer;

        if (GetStatus() == ENTITY_STATUS_ACTIVE)
        {
            if (!GetConstructionCompleteSoundPath().empty())
            {
                buffer << GAME_CMD_CONSTRUCTION_COMPLETE << GetType();
                for (ivector_cit it(vClients.begin()); it != vClients.end(); ++it)
                    Game.SendGameData(*it, buffer, true);
            }
        }
        else
        {
            if (!GetConstructionStartedSoundPath().empty())
            {
                buffer << GAME_CMD_CONSTRUCTION_STARTED << GetType();
                for (ivector_cit it(vClients.begin()); it != vClients.end(); ++it)
                    Game.SendGameData(*it, buffer, true);
            }
        }

        Game.RegisterTriggerParam(_T("index"), XtoA(GetIndex()));
        Game.RegisterTriggerParam(_T("posx"), XtoA(GetPosition()[X]));
        Game.RegisterTriggerParam(_T("posy"), XtoA(GetPosition()[Y]));
        Game.RegisterTriggerParam(_T("posz"), XtoA(GetPosition()[Z]));
        Game.RegisterTriggerParam(_T("name"), GetName());
        Game.RegisterTriggerParam(_T("type"), GetTypeName());

        Game.TriggerGlobalScript(_T("buildingplaced"));
    }
}


/*====================
  IBuildingEntity::SpawnPreview
  ====================*/
void    IBuildingEntity::SpawnPreview()
{
    SetModelHandle(Game.RegisterModel(GetModelPath()));
    m_fScale = GetBuildingScale() * m_fFoundationScale;

    if (m_pSkeleton)
    {
        m_pSkeleton->SetModel(m_hModel);
        m_pSkeleton->SetDefaultAnim(_T("place"));
        m_pSkeleton->StartAnim(_T("place"), Game.GetGameTime(), 0);
        m_pSkeleton->Pose(Game.GetGameTime());
    }

    SetStatus(ENTITY_STATUS_PREVIEW);

    StartAnimation(_T("place"), 0);
    m_yDefaultAnim = m_ayAnim[0];

    m_bSighted = true;
    m_bPrevSighted = true;
}


/*====================
  IBuildingEntity::Damage
  ====================*/
float   IBuildingEntity::Damage(float fDamage, int iFlags, IVisualEntity *pAttacker, ushort unDamagingObjectID, bool bFeedback)
{
    if (iFlags & DAMAGE_FLAG_DIRECT)
        return IVisualEntity::Damage(fDamage, iFlags, pAttacker, unDamagingObjectID, bFeedback);

    // Buildings never take team damage
    if (pAttacker != NULL &&
        pAttacker->IsVisual() &&
        pAttacker->GetAsVisualEnt()->GetTeam() == GetTeam())
        return 0.0f;

    // Adjust for damage type
    float fArmor(GetArmor());
    if (iFlags & DAMAGE_FLAG_SIEGE)
        fArmor = 0.0f;
    else if (iFlags & DAMAGE_FLAG_EXPLOSIVE)
        fArmor *= 0.5f;
    else if (iFlags & (DAMAGE_FLAG_CRUSH | DAMAGE_FLAG_FIRE))
        fArmor *= 0.75;

    fDamage *= (1.0f - fArmor);

    fDamage = IVisualEntity::Damage(fDamage, iFlags, pAttacker, unDamagingObjectID, bFeedback);
    if (fDamage > 0.0f && GetStatus() == ENTITY_STATUS_SPAWNING)
    {
        uint uiDelay(g_buildingDamageConstructDelay);
        if (m_uiLastDamageTime != 0 && Game.GetGameTime() - m_uiLastDamageTime <= g_buildingDamageConstructDelay)
            uiDelay = Game.GetGameTime() - m_uiLastDamageTime;
        m_uiLastDamageTime = Game.GetGameTime();
    }

    // Apply armor breakage
    m_fArmorBreakPenalty += fDamage * g_armorBreakRate;
    m_fArmorBreakPenalty = MIN(m_fArmorBreakPenalty, GetBaseArmor() - (GetBaseArmor() * g_armorBreakLimit));

    // Notify the team that this building has been damaged
    CEntityTeamInfo *pTeam(Game.GetTeam(GetTeam()));
    if (pTeam != NULL)
        pTeam->BuildingDamaged(GetIndex(), pAttacker == NULL ? INVALID_INDEX : pAttacker->GetIndex(), fDamage);

    // Give reward as damage is dealt
    IPlayerEntity *pRewarded(NULL);
    if (pAttacker != NULL && pAttacker->IsPet())
    {
        IGameEntity *pEnt(Game.GetEntityFromUniqueID(pAttacker->GetAsPet()->GetOwnerUID()));
        if (pEnt != NULL)
            pRewarded = pEnt->GetAsPlayerEnt();
    }
    else if (pAttacker != NULL)
    {
        pRewarded = pAttacker->GetAsPlayerEnt();
    }

    if (pRewarded != NULL)
    {
        pRewarded->GiveExperience(GetExperienceValue() * (fDamage / GetMaxHealth()));
        int iCurrentBountyValue(INT_ROUND(GetBounty() * GetHealthPercent()));
        
        int iReward(MAX(m_iBountyRemaining - iCurrentBountyValue, 0));
        m_iBountyRemaining = iCurrentBountyValue;
        pRewarded->GiveGold(iReward, true, true);
        
        Game.MatchStatEvent(pRewarded->GetClientID(), PLAYER_MATCH_GOLD_EARNED, iReward, -1, unDamagingObjectID, GetType());
    }

    return fDamage;
}


/*====================
  IBuildingEntity::Kill
  ====================*/
void    IBuildingEntity::Kill(IVisualEntity *pAttacker, ushort unKillingObjectID)
{
    m_fHealth = 0.0f;

    CEntityTeamInfo *pTeamInfo(Game.GetTeam(m_iTeam));
    if (pTeamInfo != NULL)
    {
        CBufferFixed<4> buffer;
        buffer << GAME_CMD_BUILDING_DESTROYED << GetType() << byte(GetTeam());
        Game.BroadcastGameData(buffer, true, 0);

        pTeamInfo->RemoveBuildingIndex(m_uiIndex);

        if (IsSpawnLocation())
            pTeamInfo->RemoveSpawnBuildingIndex(m_uiIndex);

        // Refund
        pTeamInfo->GiveGold(GetCost() * (1.0f - GetBuildPercent()) * g_buildingDestroyedRefund);
    }

    SetStatus(ENTITY_STATUS_DEAD);
    m_uiCorpseTime = Game.GetGameTime() + g_buildingCorpseTime;
    StartAnimation(_T("death"), 0);
    Unlink();

    if (m_uiFoundation != INVALID_INDEX)
    {
        IVisualEntity *pFoundation(Game.GetVisualEntity(m_uiFoundation));
        if (pFoundation && pFoundation->IsProp() && pFoundation->GetAsProp()->GetAsFoundation())
        {
            pFoundation->GetAsProp()->GetAsFoundation()->ClearBuildingIndex();
            pFoundation->SetStatus(ENTITY_STATUS_ACTIVE);
            pFoundation->Link();
        }
    }

    IGameEntity *pEnt(Game.GetFirstEntity());
    while (pEnt)
    {
        if (pEnt->IsPlayer())
        {
            IPlayerEntity *pPlayer(pEnt->GetAsPlayerEnt());

            // Clear this order for all players
            for (byte ySeq(0); ySeq < pPlayer->GetNumOrders(); ySeq++)
                if (pPlayer->GetOrderEntIndex(ySeq) == m_uiIndex)
                    pPlayer->DeleteOrder(ySeq);
            
            if (pPlayer->GetOfficerOrderEntIndex() == m_uiIndex)
                pPlayer->SetOfficerOrder(OFFICERCMD_INVALID);
        }

        pEnt = Game.GetNextEntity(pEnt);
    }

    // Clients don't predict game ends
    if (Game.IsClient())
        return;

    tstring sMethod(_T("Unknown"));
    if (unKillingObjectID != INVALID_ENT_TYPE)
    {
        ICvar *pCvar(EntityRegistry.GetGameSetting(unKillingObjectID, _T("Name")));

        if (pCvar != NULL)
            sMethod = pCvar->GetString();
    }

    Game.RegisterTriggerParam(_T("index"), XtoA(GetIndex()));
    Game.RegisterTriggerParam(_T("attackingindex"), XtoA(pAttacker != NULL ? pAttacker->GetIndex() : INVALID_INDEX));
    Game.RegisterTriggerParam(_T("method"), sMethod);
    Game.TriggerEntityScript(GetIndex(), _T("death"));

    CEntityTeamInfo *pTeam(Game.GetTeam(GetTeam()));
    if (pTeam == NULL)
        return;

    if (pTeam->GetBaseBuildingIndex() == GetIndex())
        Game.EndGame(GetTeam());
}


/*====================
  IBuildingEntity::PlayerEnter
  ====================*/
bool    IBuildingEntity::PlayerEnter(IGameEntity *pActivator)
{
    if (GetStatus() != ENTITY_STATUS_ACTIVE)
    {
        IBuildingEntity::Use(pActivator);
        return false;
    }

    if (Game.IsServer())
    {
        IPlayerEntity *pPlayer(pActivator->GetAsPlayerEnt());
        if (pPlayer != NULL && ((pPlayer->GetTeam() == m_iTeam && pPlayer->GetCanEnterLoadout()) || Game.GetGamePhase() == GAME_PHASE_WARMUP))
            pPlayer->EnterLoadout();
    }

    return true;
}


/*====================
  IBuildingEntity::Upkeep
  ====================*/
void    IBuildingEntity::Upkeep()
{
    RemoveNetFlags(ENT_NET_FLAG_UPKEEP_FAILED);
    if (m_pEntityConfig->GetUpkeepCost() == 0)
        return;

    if (m_fUpkeepLevel < 1.0f)
        SetNetFlags(ENT_NET_FLAG_UPKEEP_FAILED);

    // DEPRECATED: Adjustable upkeep levels
    // float fDamage(m_pEntityConfig->GetDecayRate() * (1.0f - m_fUpkeepLevel) * m_pEntityConfig->GetMaxHealth());
    float fDamage(m_pEntityConfig->GetDecayRate() * m_pEntityConfig->GetMaxHealth());
    Damage(fDamage, DAMAGE_FLAG_DIRECT);
}


/*====================
  IBuildingEntity::UpkeepFailed
  ====================*/
void    IBuildingEntity::UpkeepFailed(float fFraction)
{
    SetNetFlags(ENT_NET_FLAG_UPKEEP_FAILED);

    if (m_pEntityConfig->GetUpkeepCost() == 0)
        return;

    // DEPRECATED: Adjustable upkeep levels
    // float fDamage(m_pEntityConfig->GetDecayRate() * m_fUpkeepLevel * (1.0f - fFraction) * m_pEntityConfig->GetMaxHealth());
    float fDamage(m_pEntityConfig->GetDecayRate() * (1.0f - fFraction) * m_pEntityConfig->GetMaxHealth());
    Damage(fDamage, DAMAGE_FLAG_DIRECT);
}


/*====================
  IBuildingEntity::ServerFrame
  ====================*/
bool    IBuildingEntity::ServerFrame()
{
    if (GetStatus() == ENTITY_STATUS_PREVIEW)
        return true;

    if (!IVisualEntity::ServerFrame())
        return false;

    // Recover from armor breakage
    m_fArmorBreakPenalty = MAX(0.0f, m_fArmorBreakPenalty - ((Game.GetFrameLength() / 1000.0f) * g_armorRecoverRate));

    // Check for low health
    if ((GetHealth() + m_fBuildingHealthPool) / GetMaxHealth() <= g_buildingLowHealthPercent && GetStatus() != ENTITY_STATUS_DEAD && !m_bLowHealthEffectActive)
    {
        m_bLowHealthEffectActive = true;
        ResHandle hEffect(Game.RegisterEffect(m_pEntityConfig->GetLowHealthEffectPath()));
        if (hEffect != INVALID_RESOURCE)
        {
            SetEffect(EFFECT_CHANNEL_BUILDING_LOW_HEALTH, hEffect);
            IncEffectSequence(EFFECT_CHANNEL_BUILDING_LOW_HEALTH);
        }
        
        if (!GetLowHealthSoundPath().empty())
        {
            ivector vClients(Game.GetTeam(GetTeam())->GetClientList());

            CBufferFixed<3> buffer;
        
            buffer << GAME_CMD_BLD_HEALTH_LOW << GetType();
            for (ivector_cit it(vClients.begin()); it != vClients.end(); ++it)
                Game.SendGameData(*it, buffer, true);
        }
    }
    
    if ((((GetHealth() + m_fBuildingHealthPool) / GetMaxHealth() > g_buildingLowHealthPercent)  || GetStatus() == ENTITY_STATUS_DEAD) && m_bLowHealthEffectActive)
    {
        m_bLowHealthEffectActive = false;
        SetEffect(EFFECT_CHANNEL_BUILDING_LOW_HEALTH, INVALID_RESOURCE);
        IncEffectSequence(EFFECT_CHANNEL_BUILDING_LOW_HEALTH);
    }

    // Check for an expiring corpse (Command centers do not expire)
    if (GetStatus() == ENTITY_STATUS_DEAD && Game.GetGameTime() >= m_uiCorpseTime && !IsCommandCenter())
        return false;

    // Apply radius state (to buildings only)
    ushort unStateID(INVALID_ENT_TYPE);
    if (!m_pEntityConfig->GetStateName().empty())
        unStateID = EntityRegistry.LookupID(m_pEntityConfig->GetStateName());

    if (unStateID != INVALID_ENT_TYPE &&
        GetStatus() == ENTITY_STATUS_ACTIVE &&
        !HasNetFlags(ENT_NET_FLAG_UPKEEP_FAILED))
    {
        static uivector vResult;
        vResult.clear();
        Game.GetEntitiesInRadius(vResult, CSphere(GetPosition(), m_pEntityConfig->GetStateRadius()), SURF_MODEL);

        for (uivector_it it(vResult.begin()); it != vResult.end(); ++it)
        {
            IVisualEntity *pEnt(Game.GetEntityFromWorldIndex(*it));
            if (pEnt == NULL)
                continue;

            IBuildingEntity *pBuilding(pEnt->GetAsBuilding());
            if (pBuilding == NULL)
                continue;
            if (pBuilding->GetStatus() != ENTITY_STATUS_ACTIVE)
                continue;
            if (pBuilding->GetTeam() == GetTeam() && !m_pEntityConfig->GetStateTargetAlly())
                continue;
            if (pBuilding->GetTeam() != GetTeam() && !m_pEntityConfig->GetStateTargetEnemy())
                continue;
            if (pBuilding->GetType() == GetType() && !m_pEntityConfig->GetStateselfapply())
                continue;

            pEnt->ApplyState(unStateID, Game.GetGameTime(), m_pEntityConfig->GetStateDuration(), m_uiOwnerIndex);
        }
    }

    // Increment building health over time as it builds
    if (GetStatus() == ENTITY_STATUS_SPAWNING && Game.GetGameTime() > m_uiLastGameTime)
    {
        float fHealthDelta((GetMaxHealth() - m_fStartBuildingHealth) * float(Game.GetGameTime() - m_uiLastGameTime) / GetBuildTime());

        if (fHealthDelta > m_fBuildingHealthPool)
            fHealthDelta = m_fBuildingHealthPool;

        if (Game.GetGameTime() - m_uiLastDamageTime >= g_buildingDamageConstructDelay)
        {
            m_fHealth += fHealthDelta;
            m_fBuildingHealthPool -= fHealthDelta;
        }

        if (m_fHealth >= GetMaxHealth())
        {
            m_fHealth = GetMaxHealth();
            m_fBuildingHealthPool = 0.0f;
        }

        if (m_fBuildingHealthPool <= 0.0f)
        {
            SetStatus(ENTITY_STATUS_ACTIVE);
            StartAnimation(_T("idle"), 0);
            
            if (!GetConstructionCompleteSoundPath().empty())
            {
                ivector vClients(Game.GetTeam(GetTeam())->GetClientList());

                CBufferFixed<3> buffer;
                buffer << GAME_CMD_CONSTRUCTION_COMPLETE << GetType();

                for (ivector_cit it(vClients.begin()); it != vClients.end(); ++it)
                    Game.SendGameData(*it, buffer, true);
            }
        }

        float fPercentComplete(1.0 - (m_fBuildingHealthPool / m_fStartBuildingHealthPool));
        uint uiBuiltTime(MAX(INT_ROUND(fPercentComplete * GetBuildTime()), 0));
        uint uiExpectedTime(Game.GetGameTime() - m_uiStartBuildTime);
        m_uiBuildAnimTime = uiBuiltTime - uiExpectedTime;
    }

    m_uiLastGameTime = Game.GetGameTime();
    return true;
}


/*====================
  IBuildingEntity::Interpolate
  ====================*/
void    IBuildingEntity::Interpolate(float fLerp, IVisualEntity *pPrevState, IVisualEntity *pNextState)
{
    IVisualEntity::Interpolate(fLerp, pPrevState, pNextState);

    // Use client information if local client is the one placing this building
    if (GetStatus() == ENTITY_STATUS_PREVIEW)
        Game.UpdateBuildingPlacement(this);
}


/*====================
  IBuildingEntity::GetArmor
  ====================*/
float   IBuildingEntity::GetArmor() const
{
    FloatMod modArmor;
    for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
    {
        if (m_apState[i] == NULL)
            continue;

        modArmor += m_apState[i]->GetArmorMod();
    }

    // Adjust for under construction
    float fModifiedArmor(modArmor.Modify(GetBaseArmor()));
    if (GetStatus() == ENTITY_STATUS_SPAWNING)
        fModifiedArmor *= g_buildingIncompleteArmorFactor;
    
    // Adjust for upkeep penalty
    if (HasNetFlags(ENT_NET_FLAG_UPKEEP_FAILED))
        fModifiedArmor *= g_buildingUpkeepFailedArmorMult;

    // Adjust for breakage
    fModifiedArmor -= m_fArmorBreakPenalty;

    return  MAX(0.0f, fModifiedArmor);
}


/*====================
  IBuildingEntity::GetArmorDamageReduction
  ====================*/
float   IBuildingEntity::GetArmorDamageReduction() const
{
    float fArmor(GetArmor());
    return fArmor;
}


/*====================
  IBuildingEntity::AddToScene
  ====================*/
bool    IBuildingEntity::AddToScene(const CVec4f &v4Color, int iFlags)
{
    PROFILE("IBuildingEntity::AddToScene");

    if (m_hModel == INVALID_INDEX)
        return false;

    CEntityClientInfo *pLocalClient(Game.GetClientInfo(Game.GetLocalClientNum()));
    if (GetStatus() == ENTITY_STATUS_PREVIEW &&
        pLocalClient != NULL &&
        pLocalClient->GetTeam() != GetTeam())
        return false;

    CVec4f v4TintedColor(v4Color);

    if (Game.IsCommander() && GetStatus() != ENTITY_STATUS_PREVIEW)
    {
        if (!(m_bSighted || m_bPrevSighted))
            return false;
    
        if (!m_bSighted)
        {
            //v4TintedColor[R] *= 0.333f;
            //v4TintedColor[G] *= 0.333f;
            //v4TintedColor[B] *= 0.333f;
        }
    }

    if (m_v3AxisAngles != m_v3Angles)
    {
        m_aAxis.Set(m_v3Angles);
        m_v3AxisAngles = m_v3Angles;
    }

    static CSceneEntity sceneEntity;

    sceneEntity.Clear();

    sceneEntity.scale = GetScale() * GetScale2();
    sceneEntity.SetPosition(m_v3Position);
    sceneEntity.axis = m_aAxis;
    sceneEntity.objtype = OBJTYPE_MODEL;
    sceneEntity.hModel = m_hModel;
    sceneEntity.skeleton = m_pSkeleton;
    sceneEntity.color = v4TintedColor;
    sceneEntity.flags = iFlags | SCENEOBJ_SOLID_COLOR | SCENEOBJ_USE_AXIS;

    if (Game.LooksLikeEnemy(m_uiIndex))
        sceneEntity.hSkin = g_ResourceManager.GetSkin(GetModelHandle(), _T("red"));

    if (m_uiClientRenderFlags & ECRF_SNAPSELECTED)
        sceneEntity.color *= m_v4SelectColor;

    if (m_uiClientRenderFlags & ECRF_HALFTRANSPARENT)
        sceneEntity.color[A] *= 0.5f;

    SSceneEntityEntry &cEntry(SceneManager.AddEntity(sceneEntity));

    if (!cEntry.bCull || !cEntry.bCullShadow)
    {
        if (GetStatus() != ENTITY_STATUS_PREVIEW)
            AddSelectionRingToScene();
        
        UpdateSkeleton(true);
    }
    else
    {
        UpdateSkeleton(false);
    }

    return true;
}


/*====================
  IBuildingEntity::IsVisibleOnMinimap
  ====================*/
bool    IBuildingEntity::IsVisibleOnMinimap(IPlayerEntity *pLocalPlayer, bool bLargeMap)
{
    if ((bLargeMap && GetLargeMapIcon() == INVALID_INDEX && GetMinimapIcon() == INVALID_INDEX) ||
        (!bLargeMap && GetMinimapIcon() == INVALID_INDEX))
        return false;

    if (GetStatus() != ENTITY_STATUS_ACTIVE &&
        GetStatus() != ENTITY_STATUS_DEAD &&
        GetStatus() != ENTITY_STATUS_SPAWNING)
        return false;

    int iTeam(pLocalPlayer == NULL ? -1 : pLocalPlayer->GetTeam());

    if (IsStealthed() && GetTeam() != iTeam)
        return false;
    if (GetIsHidden() && GetTeam() != iTeam)
        return false;

    return m_bPrevSighted;
}


/*====================
  IBuildingEntity::GetMapIconColor
  ====================*/
CVec4f  IBuildingEntity::GetMapIconColor(IPlayerEntity *pLocalPlayer, bool bLargeMap)
{
    if (!bLargeMap)
        return IVisualEntity::GetMapIconColor(pLocalPlayer, bLargeMap);

    if (CanSpawnFrom(pLocalPlayer))
        return WHITE;

    if (pLocalPlayer != NULL && pLocalPlayer->LooksLikeEnemy(this))
        return RED;

    return GRAY;
}


/*====================
  IBuildingEntity::UpdateSkeleton
  ====================*/
void    IBuildingEntity::UpdateSkeleton(bool bPose)
{
    PROFILE("IBuildingEntity::UpdateSkeleton");

    if (m_pSkeleton == NULL)
        return;

    m_pSkeleton->SetModel(GetModelHandle());

    if (GetModelHandle() == INVALID_RESOURCE)
        return;

    // Pose skeleton
    uint uiAnimTime(Game.GetGameTime());
    if (GetStatus() == ENTITY_STATUS_SPAWNING)
    {
        uiAnimTime += m_uiBuildAnimTime;
        m_uiPoseTime = MAX(uiAnimTime, m_uiPoseTime);
    }
    else
    {
        m_uiPoseTime = uiAnimTime;
    }

    if (bPose)
        m_pSkeleton->Pose(m_uiPoseTime);
    else
        m_pSkeleton->PoseLite(m_uiPoseTime);

    // Process animation events
    if (m_pSkeleton->CheckEvents())
    {
        tstring sOldDir(FileManager.GetWorkingDirectory());
        FileManager.SetWorkingDirectory(Filename_GetPath(g_ResourceManager.GetPath(GetModelHandle())));

        const vector<SAnimEventCmd> &vEventCmds(m_pSkeleton->GetEventCmds());

        for (vector<SAnimEventCmd>::const_iterator it(vEventCmds.begin()); it != vEventCmds.end(); ++it)
            EventScript.Execute(it->sCmd, it->iTimeNudge);

        m_pSkeleton->ClearEvents();

        FileManager.SetWorkingDirectory(sOldDir);
    }
}


/*====================
  IBuildingEntity::Heal
  ====================*/
float   IBuildingEntity::Heal(float fHealth, IVisualEntity *pSource)
{
    //if (HasNetFlags(ENT_NET_FLAG_UPKEEP_FAILED) && GetStatus() != ENTITY_STATUS_SPAWNING)
    //  return 0.0f;

    fHealth = MIN(fHealth, GetMaxHealth() - GetHealth());
    SetHealth(CLAMP(GetHealth() + fHealth, 0.0f, GetMaxHealth()));

    if (GetStatus() == ENTITY_STATUS_SPAWNING && m_fBuildingHealthPool > 0.0)
        m_fBuildingHealthPool = MAX(m_fBuildingHealthPool - fHealth, 0.0f);
    else
        m_iBountyRemaining = INT_ROUND(GetBounty() * GetHealthPercent());

    if (pSource != NULL && pSource->GetAsPlayerEnt() != NULL)
        Game.MatchStatEvent(pSource->GetAsPlayerEnt()->GetClientID(), PLAYER_MATCH_REPAIRED, fHealth, -1, (pSource != NULL ? pSource->GetType() : INVALID_ENT_TYPE), GetType());

    return fHealth;
}


/*====================
  IBuildingEntity::GetBuildPercent
  ====================*/
float   IBuildingEntity::GetBuildPercent()
{
    if (GetStatus() != ENTITY_STATUS_SPAWNING)
        return 1.0f;

    if (Game.IsServer())
        return (1.0 - (m_fBuildingHealthPool / m_fStartBuildingHealthPool));

    float fPercent(ILERP(m_uiPoseTime, m_uiStartBuildTime, m_uiStartBuildTime + GetBuildTime()));
    return CLAMP(fPercent, 0.0f, 1.0f);
}


/*====================
  IBuildingEntity::SetTeam
  ====================*/
void    IBuildingEntity::SetTeam(char cTeam)
{
    CEntityTeamInfo *pTeamInfo;

    if (cTeam == m_iTeam)
        return;
    
    if (m_iTeam != -1)
    {
        pTeamInfo = Game.GetTeam(m_iTeam);

        if (pTeamInfo)
        {
            pTeamInfo->RemoveBuildingIndex(m_uiIndex);
            if (IsSpawnLocation())
                pTeamInfo->RemoveSpawnBuildingIndex(m_uiIndex);
        }
    }

    IVisualEntity::SetTeam(cTeam);

    pTeamInfo = Game.GetTeam(cTeam);

    if (pTeamInfo)
    {
        pTeamInfo->AddBuildingIndex(m_uiIndex);
        if (IsSpawnLocation())
            pTeamInfo->AddSpawnBuildingIndex(m_uiIndex);
    }
}


/*====================
  IBuildingEntity::GetApproachPosition
  ====================*/
CVec3f  IBuildingEntity::GetApproachPosition(const CVec3f &v3Start, const CBBoxf &bbBounds)
{
    CWorldEntity *pWorldEnt(Game.GetWorldEntity(m_uiWorldIndex));
        
    if (pWorldEnt != NULL)
    {
        CBBoxf bbBoundsWorld(bbBounds);
        bbBoundsWorld.Offset(v3Start);

        float fFraction(1.0f);

        const vector<CConvexPolyhedron> &cWorldSurfs(pWorldEnt->GetWorldSurfsRef());
        for (vector<CConvexPolyhedron>::const_iterator cit(cWorldSurfs.begin()); cit != cWorldSurfs.end(); ++cit)
            I_MovingBoundsSurfaceIntersect(v3Start, m_v3Position, bbBoundsWorld, *cit, fFraction);

        return LERP(fFraction, v3Start, m_v3Position);
    }
    else
        return m_v3Position;
}


/*====================
  IBuildingEntity::Copy
  ====================*/
void    IBuildingEntity::Copy(const IGameEntity &B)
{
    IVisualEntity::Copy(B);

    const IBuildingEntity *pB(B.GetAsBuilding());

    if (!pB)    
        return;

    const IBuildingEntity &C(*pB);

    m_uiFoundation =        C.m_uiFoundation;
    m_uiBuildAnimTime =     C.m_uiBuildAnimTime;
    m_uiStartBuildTime =    C.m_uiStartBuildTime;
    m_iBountyRemaining =    C.m_iBountyRemaining;
    m_fArmorBreakPenalty =  C.m_fArmorBreakPenalty;
}


/*====================
  IBuildingEntity::ClientPrecache
  ====================*/
void    IBuildingEntity::ClientPrecache(CEntityConfig *pConfig)
{
    IVisualEntity::ClientPrecache(pConfig);

    if (!pConfig)
        return;

    if (!pConfig->GetLowHealthEffectPath().empty())
        g_ResourceManager.Register(pConfig->GetLowHealthEffectPath(), RES_EFFECT);
    if (!pConfig->GetSelectSoundPath().empty())
        g_ResourceManager.Register(pConfig->GetSelectSoundPath(), RES_SAMPLE);
    if (!pConfig->GetSelectConstructionSoundPath().empty())
        g_ResourceManager.Register(pConfig->GetSelectSoundPath(), RES_SAMPLE);
    if (!pConfig->GetConstructionCompleteSoundPath().empty())
        g_ResourceManager.Register(pConfig->GetConstructionCompleteSoundPath(), RES_SAMPLE);
    if (!pConfig->GetConstructionStartedSoundPath().empty())
        g_ResourceManager.Register(pConfig->GetConstructionStartedSoundPath(), RES_SAMPLE);
    if (!pConfig->GetDestroyedSoundPath().empty())
        g_ResourceManager.Register(pConfig->GetDestroyedSoundPath(), RES_SAMPLE);
    if (!pConfig->GetDestroyedSoundPathHuman().empty())
        g_ResourceManager.Register(pConfig->GetDestroyedSoundPathHuman(), RES_SAMPLE);
    if (!pConfig->GetDestroyedSoundPathBeast().empty())
        g_ResourceManager.Register(pConfig->GetDestroyedSoundPathBeast(), RES_SAMPLE);
    if (!pConfig->GetLowHealthSoundPath().empty())
        g_ResourceManager.Register(pConfig->GetLowHealthSoundPath(), RES_SAMPLE);

    if (!pConfig->GetStateName().empty())
        EntityRegistry.ClientPrecache(EntityRegistry.LookupID(pConfig->GetStateName()));
}


/*====================
  IBuildingEntity::ServerPrecache
  ====================*/
void    IBuildingEntity::ServerPrecache(CEntityConfig *pConfig)
{
    IVisualEntity::ServerPrecache(pConfig);

    if (!pConfig)
        return;

    if (!pConfig->GetLowHealthEffectPath().empty())
        g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetLowHealthEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));

    if (!pConfig->GetStateName().empty())
        EntityRegistry.ServerPrecache(EntityRegistry.LookupID(pConfig->GetStateName()));
}
