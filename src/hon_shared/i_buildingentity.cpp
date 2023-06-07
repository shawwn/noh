// (C)2006 S2 Games
// i_buildingentity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "hon_shared_common.h"

#include "i_buildingentity.h"
#include "c_teaminfo.h"
#include "c_player.h"
#include "i_entitystate.h"
#include "c_bsentry.h"
#include "i_heroentity.h"

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
#include "../k2/c_resourcemanager.h"
#include "../k2/c_resourceinfo.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
extern CCvarui g_economyInterval;
EXTERN_CVAR(bool, d_drawUnitBounds);
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
ResHandle           IBuildingEntity::s_hMinimapDefendTargetIcon(INVALID_RESOURCE);
ResHandle           IBuildingEntity::s_hMinimapAttackTargetIcon(INVALID_RESOURCE);
uint                IBuildingEntity::s_uiBaseType(ENTITY_BASE_TYPE_BUILDING);

DEFINE_ENTITY_DESC(IBuildingEntity, 1)
{
    s_cDesc.pFieldTypes = K2_NEW(ctx_Game,   TypeVector)();
    s_cDesc.pFieldTypes->clear();
    const TypeVector &vBase(IGameEntity::GetTypeVector());
    s_cDesc.pFieldTypes->insert(s_cDesc.pFieldTypes->begin(), vBase.begin(), vBase.end());

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_v3Position"), TYPE_ROUNDPOS3D, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_v3UnitAngles[YAW]"), TYPE_ANGLE8, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_yStatus"), TYPE_CHAR, 3, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_ySequence"), TYPE_CHAR, 4, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_unVisibilityFlags"), TYPE_SHORT, 16, 0));
    
    for (int i(0); i < 1; ++i)
    {
        s_cDesc.pFieldTypes->push_back(SDataField(_T("m_ayAnim[") + XtoA(i) + _T("]"), TYPE_CHAR, 5, -1));
        s_cDesc.pFieldTypes->push_back(SDataField(_T("m_ayAnimSequence[") + XtoA(i) + _T("]"), TYPE_CHAR, 2, 0));
        s_cDesc.pFieldTypes->push_back(SDataField(_T("m_afAnimSpeed[") + XtoA(i) + _T("]"), TYPE_FLOAT, 32, 0));
    }

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiTeamID"), TYPE_INT, 3, TEAM_INVALID));

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_unUnitFlags"), TYPE_SHORT, 8, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiLevel"), TYPE_INT, 5, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_fHealth"), TYPE_CEIL16, 15, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_fMana"), TYPE_FLOOR16, 15, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_iOwnerClientNumber"), TYPE_INT, 5, -1));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiOwnerEntityIndex"), TYPE_GAMEINDEX, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_fFade"), TYPE_BYTEPERCENT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiSpawnTime"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiLifetime"), TYPE_INT, 32, 0));

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiLastDamageTime"), TYPE_INT, 32, 0));
}
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
CVAR_UINTF  (g_buildingCorpseTime,              10000,  CVAR_GAMECONFIG);
CVAR_FLOATF (g_buildingLowHealthPercent,        0.33f,  CVAR_GAMECONFIG);
CVAR_UINTF  (g_buildingHelpHeroDuration,        2000,   CVAR_GAMECONFIG); 
CVAR_UINTF  (g_buildingTargetMemory,            6000,   CVAR_GAMECONFIG);
CVAR_UINTF  (g_buildingAnnounceAttackTime,      30000,  CVAR_GAMECONFIG);
CVAR_UINTF  (g_buildingAnnounceTeamAttackTime,  10000,  CVAR_GAMECONFIG);
CVAR_UINTF  (g_buildingAnnounceNoHeroRadius,    2000,   CVAR_GAMECONFIG);
CVAR_FLOATF (g_buildingIconSizeScale,           2.0f,   CVAR_GAMECONFIG);
CVAR_STRINGF(g_buildingMapAttackTargetIcon,     "/shared/icons/minimap_ping_enemy.tga",     CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_STRINGF(g_buildingMapDefendTargetIcon,     "/shared/icons/minimap_ping_ally.tga",      CVAR_TRANSMIT | CVAR_GAMECONFIG);
//=============================================================================

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
}


/*====================
  IBuildingEntity::IBuildingEntity
  ====================*/
IBuildingEntity::IBuildingEntity() :
m_uiLastGameTime(INVALID_TIME),
m_uiLastDamageTime(INVALID_TIME),
m_uiLastAttackAnnounce(INVALID_TIME),
m_bLowHealthEffectActive(false)
{
    m_uiLinkFlags = SURF_BUILDING;

    for (uint ui(0); ui < NUM_BUILDING_TARGETS; ++ui)
        m_auiTargetUIDs[ui] = INVALID_INDEX;
}


/*====================
  IBuildingEntity::Baseline
  ====================*/
void    IBuildingEntity::Baseline()
{
    IGameEntity::Baseline();

    // Basic data
    m_v3Position = V3_ZERO;
    m_v3UnitAngles = V3_ZERO;
    m_yStatus = ENTITY_STATUS_ACTIVE;
    m_ySequence = 0;
    m_unVisibilityFlags = 0;

    // Anims
    for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
    {
        m_ayAnim[i] = ENTITY_INVALID_ANIM;
        m_ayAnimSequence[i] = 0;
        m_afAnimSpeed[i] = 1.0f;
    }

    m_uiTeamID = TEAM_PASSIVE;

    m_unUnitFlags = 0;
    m_uiLevel = 0;
    m_fHealth = 0.0f;
    m_fMana = 0.0f;

    m_iOwnerClientNumber = -1;
    m_uiOwnerEntityIndex = INVALID_INDEX;

    m_fFade = 0.0f;

    m_uiSpawnTime = INVALID_TIME;
    m_uiLifetime = 0;

    m_uiLastDamageTime = INVALID_TIME;
    m_uiLastAttackAnnounce = INVALID_TIME;
}


/*====================
  IBuildingEntity::GetSnapshot
  ====================*/
void    IBuildingEntity::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
    IGameEntity::GetSnapshot(snapshot, uiFlags);

    // Basic data
    snapshot.WriteRoundPos3D(m_v3Position);
    snapshot.WriteAngle8(m_v3UnitAngles.z);

    snapshot.WriteField(m_yStatus);
    snapshot.WriteField(m_ySequence);
    snapshot.WriteField(m_unVisibilityFlags);

    // Anims
    for (int i(0); i < 1; ++i)
    {
        snapshot.WriteInteger(m_ayAnim[i]);
        snapshot.WriteField(m_ayAnimSequence[i]);
        snapshot.WriteField(m_afAnimSpeed[i]);
    }

    snapshot.WriteInteger(m_uiTeamID);
    snapshot.WriteField(m_unUnitFlags);
    snapshot.WriteField(m_uiLevel);

    if (uiFlags & SNAPSHOT_HIDDEN)
    {
        snapshot.WriteCeil16(0.0f);
        snapshot.WriteFloor16(0.0f);
    }
    else
    {
        snapshot.WriteCeil16(m_fHealth);
        snapshot.WriteFloor16(m_fMana);
    }

    snapshot.WriteInteger(m_iOwnerClientNumber);
    snapshot.WriteGameIndex(m_uiOwnerEntityIndex);
    snapshot.WriteBytePercent(m_fFade);
    snapshot.WriteField(m_uiSpawnTime);
    snapshot.WriteField(m_uiLifetime);

    snapshot.WriteField(m_uiLastDamageTime);
}


/*====================
  IBuildingEntity::ReadSnapshot
  ====================*/
bool    IBuildingEntity::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
    try
    {
        // Base entity info
        if (!IGameEntity::ReadSnapshot(snapshot, 1))
            return false;
        
        snapshot.ReadRoundPos3D(m_v3Position);
        snapshot.ReadAngle8(m_v3UnitAngles.z);
        snapshot.ReadField(m_yStatus);
        snapshot.ReadField(m_ySequence);
        snapshot.ReadField(m_unVisibilityFlags);

        // Anims
        for (int i(0); i < 1; ++i)
        {
            snapshot.ReadInteger(m_ayAnim[i]);
            snapshot.ReadField(m_ayAnimSequence[i]);
            snapshot.ReadField(m_afAnimSpeed[i]);
        }

        snapshot.ReadInteger(m_uiTeamID);

        snapshot.ReadField(m_unUnitFlags);
        snapshot.ReadField(m_uiLevel);
        snapshot.ReadFloat16(m_fHealth);
        snapshot.ReadFloat16(m_fMana);
        snapshot.ReadInteger(m_iOwnerClientNumber);
        snapshot.ReadGameIndex(m_uiOwnerEntityIndex);
        snapshot.ReadBytePercent(m_fFade);
        snapshot.ReadField(m_uiSpawnTime);
        snapshot.ReadField(m_uiLifetime);

        snapshot.ReadField(m_uiLastDamageTime);
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("IBuildingEntity::ReadSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  IBuildingEntity::AllocateSkeleton
  ====================*/
CSkeleton*  IBuildingEntity::AllocateSkeleton()
{
    m_pSkeleton = K2_NEW(ctx_Game,   CSkeleton);
    return m_pSkeleton;
}


/*====================
  IBuildingEntity::GetLinkFlags
  ====================*/
uint    IBuildingEntity::GetLinkFlags()
{
    uint uiLinkFlags(m_uiLinkFlags | SURF_DYNAMIC);

    if (GetFlying())
        uiLinkFlags |= SURF_FLYING;
    if (m_uiLinkFlags & SURF_UNIT && GetUnitwalking())
        uiLinkFlags |= SURF_UNITWALKING;
    if (m_uiLinkFlags & (SURF_UNIT | SURF_BUILDING) && GetBoundsRadius() == 0.0f)
        uiLinkFlags |= SURF_NOBLOCK;

    if (!GetIsSelectable())
        uiLinkFlags |= SURF_NOSELECT;

    return uiLinkFlags;
}

/*====================
  IBuildingEntity::DrawOnMap
  ====================*/
void    IBuildingEntity::DrawOnMap(CUITrigger &minimap, CPlayer *pLocalPlayer) const
{
    IUnitEntity* pUnit((IUnitEntity*)GetAsUnit());

    bool bTarget(pLocalPlayer->GetTeamInfo()->IsTeamTarget(pUnit->GetIndex()) && s_hMinimapAttackTargetIcon != INVALID_RESOURCE && s_hMinimapDefendTargetIcon != INVALID_RESOURCE);

    if (bTarget)
    {
        if (!IsVisibleOnMap(pLocalPlayer))
            return;

        if (s_hMinimapAttackTargetIcon == INVALID_RESOURCE)
            return;

        if (s_hMinimapDefendTargetIcon == INVALID_RESOURCE)
            return;

        CBufferFixed<40> buffer;
        
        buffer << GetPosition().x / Game.GetWorldWidth();
        buffer << 1.0f - (GetPosition().y / Game.GetWorldHeight());
        
        buffer << GetMapIconSize(pLocalPlayer) * g_buildingIconSizeScale << GetMapIconSize(pLocalPlayer) * g_buildingIconSizeScale;
    
        CVec4f v4Color(1.0f, 1.0f, 1.0f, 1.0f);//IUnitEntity::GetMapIconColor(pLocalPlayer));
        buffer << v4Color[R];
        buffer << v4Color[G];
        buffer << v4Color[B];
        buffer << v4Color[A];
        
        if (pLocalPlayer->IsEnemy(pUnit))
            buffer << s_hMinimapAttackTargetIcon;
        else
            buffer << s_hMinimapDefendTargetIcon;

        buffer << (GetHoverOnMap() ? GetIndex() : uint(-1));

        minimap.Execute(_T("icon"), buffer);
    }
    else
    {
        if (!IsVisibleOnMap(pLocalPlayer))
            return;

        if (s_hMinimapAttackTargetIcon == INVALID_RESOURCE)
            return;

        if (s_hMinimapDefendTargetIcon == INVALID_RESOURCE)
            return;

        CBufferFixed<40> buffer;
    
        buffer << GetPosition().x / Game.GetWorldWidth();
        buffer << 1.0f - (GetPosition().y / Game.GetWorldHeight());
        
        buffer << GetMapIconSize(pLocalPlayer) << GetMapIconSize(pLocalPlayer);
        
        CVec4f v4Color(GetMapIconColor(pLocalPlayer));
        buffer << v4Color[R];
        buffer << v4Color[G];
        buffer << v4Color[B];
        buffer << v4Color[A];

        buffer << GetMapIcon(pLocalPlayer);

        buffer << (GetHoverOnMap() ? GetIndex() : uint(-1));

        minimap.Execute(_T("icon"), buffer);
    }
}

/*====================
  IBuildingEntity::DrawOutlineOnMap
  ====================*/
void    IBuildingEntity::DrawOutlineOnMap(CUITrigger &minimap, CPlayer *pLocalPlayer) const
{
    return;

    if (!IsVisibleOnMap(pLocalPlayer))
        return;
    if (GetMapIcon(pLocalPlayer) == INVALID_RESOURCE)
        return;

    CBufferFixed<24> buffer;
    
    buffer << GetPosition().x / Game.GetWorldWidth();
    buffer << 1.0f - (GetPosition().y / Game.GetWorldHeight());
    buffer << GetMapIconSize(pLocalPlayer) << GetMapIconSize(pLocalPlayer);
    buffer << GetMapIcon(pLocalPlayer);
    buffer << (GetHoverOnMap() ? GetIndex() : uint(-1));

    minimap.Execute(_T("outline"), buffer);
}


/*====================
  IBuildingEntity::Link
  ====================*/
void    IBuildingEntity::Link()
{
    if (m_uiWorldIndex == INVALID_INDEX)
        return;
    
    CWorldEntity *pWorldEnt(Game.GetWorldEntity(m_uiWorldIndex));
    if (pWorldEnt == nullptr)
        return;

    pWorldEnt->SetPosition(GetPosition());
    pWorldEnt->SetScale(GetBaseScale() * GetScale());
    pWorldEnt->SetScale2(1.0f); // Change this to GetScale() if we want bounds to resize with scale changes caused by states
    pWorldEnt->SetAngles(GetAngles());
    pWorldEnt->SetBounds(GetBounds());
    pWorldEnt->SetModelHandle(GetModel());
    pWorldEnt->SetGameIndex(GetIndex());

    uint uiLinkFlags(GetLinkFlags());

    if (GetStatus() != ENTITY_STATUS_CORPSE)
    {
        if (GetBoundsRadius() > 0.0f)
        {
            Game.LinkEntity(m_uiWorldIndex, LINK_BOUNDS | LINK_MODEL, uiLinkFlags);

            m_vPathBlockers.push_back(Game.BlockPath(NAVIGATION_BUILDING, m_bbBounds.GetMin().xy() + m_v3Position.xy() + CVec2f(-g_pathPad, -g_pathPad), m_bbBounds.GetDim(X) + g_pathPad * 2.0f, m_bbBounds.GetDim(Y) + g_pathPad * 2.0f));
        }
        else
        {
            Game.LinkEntity(m_uiWorldIndex, LINK_SURFACE | LINK_MODEL, uiLinkFlags);

            const vector<CConvexPolyhedron> &cWorldSurfs(pWorldEnt->GetWorldSurfsRef());
            for (vector<CConvexPolyhedron>::const_iterator cit(cWorldSurfs.begin()); cit != cWorldSurfs.end(); ++cit)
                Game.BlockPath(m_vPathBlockers, NAVIGATION_BUILDING, *cit, 0.0f);
        }

        m_v2BlockPosition = m_v3Position.xy();
    }

    m_uiLinkedFlags = uiLinkFlags;
}


/*====================
  IBuildingEntity::Unlink
  ====================*/
void    IBuildingEntity::Unlink()
{
    if (m_uiWorldIndex != INVALID_INDEX)
    {
        CWorldEntity *pWorldEnt(Game.GetWorldEntity(m_uiWorldIndex));
        
        if (pWorldEnt != nullptr)
        {
            vector<PoolHandle>::const_iterator citEnd(m_vPathBlockers.end());
            for (vector<PoolHandle>::const_iterator cit(m_vPathBlockers.begin()); cit != citEnd; ++cit)
                Game.ClearPath(*cit);

            m_vPathBlockers.clear();
        }

        Game.UnlinkEntity(m_uiWorldIndex);
    }
}


/*====================
  IBuildingEntity::ApplyWorldEntity
  ====================*/
void    IBuildingEntity::ApplyWorldEntity(const CWorldEntity &ent)
{
    IUnitEntity::ApplyWorldEntity(ent);

    for (uint ui(0); ui < NUM_BUILDING_TARGETS; ++ui)
        m_asTargets[ui] = ent.GetProperty(_T("target") + XtoA(ui), TSNULL);
}


/*====================
  IBuildingEntity::Spawn
  ====================*/
void    IBuildingEntity::Spawn()
{
    Game.Precache(m_unType, PRECACHE_ALL, TSNULL); // TODO: Make this smarter...
    
    IUnitEntity::Spawn();

    if (GetBoundsRadius() > 0.0f)
    {
        m_bbBounds.SetCylinder(GetBoundsRadius(), GetBoundsHeight());
    }
    else
    {
        m_bbBounds = g_ResourceManager.GetModelSurfaceBounds(GetModel());

        float fRadius(0.0f);

        fRadius = MAX(fRadius, ABS(m_bbBounds.GetMin(X)));
        fRadius = MAX(fRadius, ABS(m_bbBounds.GetMin(Y)));
        fRadius = MAX(fRadius, ABS(m_bbBounds.GetMax(X)));
        fRadius = MAX(fRadius, ABS(m_bbBounds.GetMax(Y)));

        m_bbBounds.SetCylinder(fRadius, m_bbBounds.GetMax(Z));

        m_bbBounds *= (GetBaseScale() * m_fScale);
    }

    if (m_pSkeleton)
        m_pSkeleton->SetModel(GetModel());

    StartAnimation(GetIdleAnim(), 0);

    Unlink();
    Link();

    if (GetAttackType() != INVALID_ATTACK_TYPE)
        m_cBrain.AddBehavior(K2_NEW(ctx_Game,   CBSentry)(), false);

    if (GetIsBase())
    {
        CTeamInfo *pTeam(Game.GetTeam(GetTeam()));
        if (pTeam != nullptr)
            pTeam->SetBaseBuildingIndex(GetIndex());
    }
}


/*====================
  IBuildingEntity::Die
  ====================*/
void    IBuildingEntity::Die(IUnitEntity *pAttacker, ushort unKillingObjectID)
{
    if (GetStatus() != ENTITY_STATUS_ACTIVE)
        return;

    IUnitEntity::Die(pAttacker, unKillingObjectID);

    if (pAttacker != nullptr && GetIsTower())
    {
        CPlayer *pPlayerKiller(Game.GetPlayerFromClientNumber(pAttacker->GetOwnerClientNumber()));
        
        if (pPlayerKiller != nullptr)
        {
            if (pAttacker->GetTeam() != GetTeam())
            {
                CBufferFixed<15> buffer;
                buffer << GAME_CMD_KILL_TOWER_MESSAGE << pPlayerKiller->GetClientNumber() << pAttacker->GetTeam() << ushort(GetGoldBountyTeam()) << GetIndex();
                Game.BroadcastGameData(buffer, true);

                CTeamInfo *pTeam(Game.GetTeam(GetTeam()));
                if (pTeam != nullptr)
                {
                    if (CompareNoCase(pTeam->GetName(), _T("Hellbourne")) == 0)
                    {
                        CBufferFixed<1> buffer;
                        buffer << GAME_CMD_LEGION_DESTROY_TOWER_MESSAGE;
                        Game.BroadcastGameData(buffer, true);
                    }
                    else if (CompareNoCase(pTeam->GetName(), _T("Legion")) == 0)
                    {
                        CBufferFixed<1> buffer;
                        buffer << GAME_CMD_HELLBOURNE_DESTROY_TOWER_MESSAGE;
                        Game.BroadcastGameData(buffer, true);
                    }
                }
            }
            else
            {
                if (pPlayerKiller->GetAnnouncerVoice() != INVALID_INDEX)
                {
                    CBufferFixed<10> buffer;
                    buffer << GAME_CMD_DENY_TOWER_MESSAGE2 << pPlayerKiller->GetClientNumber() << GetIndex() << byte(pPlayerKiller->GetAnnouncerVoice());
                    Game.BroadcastGameData(buffer, true);
                }
                else
                {   
                    CBufferFixed<9> buffer;
                    buffer << GAME_CMD_DENY_TOWER_MESSAGE << pPlayerKiller->GetClientNumber() << GetIndex();
                    Game.BroadcastGameData(buffer, true);
                }                                   
            }
        }
        else if (pAttacker->GetTeam() == TEAM_1 || pAttacker->GetTeam() == TEAM_2)
        {
            if (pAttacker->GetTeam() != GetTeam())
            {
                CBufferFixed<11> buffer;
                buffer << GAME_CMD_TEAM_KILL_TOWER_MESSAGE << pAttacker->GetTeam() << ushort(GetGoldBountyConsolation()) << GetIndex();
                Game.BroadcastGameData(buffer, true);

                CTeamInfo *pTeam(Game.GetTeam(GetTeam()));

                if (pTeam != nullptr)
                {
                    if (CompareNoCase(pTeam->GetName(), _T("Hellbourne")) == 0)
                    {
                        CBufferFixed<1> buffer;
                        buffer << GAME_CMD_LEGION_DESTROY_TOWER_MESSAGE;
                        Game.BroadcastGameData(buffer, true);
                    }
                    else if (CompareNoCase(pTeam->GetName(), _T("Legion")) == 0)
                    {
                        CBufferFixed<1> buffer;
                        buffer << GAME_CMD_HELLBOURNE_DESTROY_TOWER_MESSAGE;
                        Game.BroadcastGameData(buffer, true);
                    }
                }
            }
            else
            {
                CBufferFixed<9> buffer;
                buffer << GAME_CMD_TEAM_DENY_TOWER_MESSAGE << pAttacker->GetTeam() << GetIndex();
                Game.BroadcastGameData(buffer, true);
            }
        }

        if (pAttacker->GetTeam() != GetTeam())
        {
            CTeamInfo *pAttackerTeam(Game.GetTeam(pAttacker->GetTeam()));
            if (pAttackerTeam != nullptr)
                pAttackerTeam->AdjustStat(TEAM_STAT_TOWER_KILLS, 1);
        }
        else
        {
            CTeamInfo *pAttackerTeam(Game.GetTeam(pAttacker->GetTeam()));
            if (pAttackerTeam != nullptr)
                pAttackerTeam->AdjustStat(TEAM_STAT_TOWER_DENIES, 1);
        }
    }
    
    if (pAttacker != nullptr && GetIsRax())
    {       
        // send the message, along with the victim team, building index and type indicator
        CBufferFixed<13> buffer;            
        
        const tstring sUnitTypeName(GetUnitType().begin()->c_str());
        
        if (CompareNoCase(sUnitTypeName, _T("MeleeRax")) == 0)
        {
            const uint uiMeleeRaxFlag(1);
            buffer << GAME_CMD_KILL_RAX_MESSAGE << GetTeam() <<  GetIndex() << uiMeleeRaxFlag;
            Game.BroadcastGameData(buffer, true);
        }
        else if (CompareNoCase(sUnitTypeName, _T("RangedRax")) == 0)
        {
            const uint uiRangedRaxFlag(2);
            buffer << GAME_CMD_KILL_RAX_MESSAGE << GetTeam() <<  GetIndex() << uiRangedRaxFlag;
            Game.BroadcastGameData(buffer, true);
        }               
    }

    CTeamInfo *pTeamInfo(Game.GetTeam(GetTeam()));
    if (pTeamInfo != nullptr)
    {
        if (pTeamInfo->GetBaseBuildingIndex() == GetIndex())
            Game.EndMatch(GetTeam());

        if (Game.HasMegaCreeps(GetTeam() ^ 3))
            pTeamInfo->SendMegaCreepMessage(pAttacker ? pAttacker->GetTeam() : TEAM_INVALID);
    }
}


/*====================
  IBuildingEntity::GameStart
  ====================*/
void    IBuildingEntity::GameStart()
{
    // Register targets
    for (uint ui(0); ui < NUM_BUILDING_TARGETS; ++ui)
    {
        if (m_asTargets[ui].empty())
            m_auiTargetUIDs[ui] = INVALID_INDEX;
        else
        {
            IVisualEntity *pTarget(Game.GetEntityFromName(m_asTargets[ui]));
            if (pTarget)
                m_auiTargetUIDs[ui] = pTarget->GetUniqueID();
            else
            {
                Console.Warn << _T("Target ") << SingleQuoteStr(m_asTargets[ui]) << _T(" not found") << newl;
                m_auiTargetUIDs[ui] = INVALID_INDEX;
            }
        }
    }
}


/*====================
  IBuildingEntity::ServerFrameSetup
  ====================*/
bool    IBuildingEntity::ServerFrameSetup()
{
    PROFILE("IBuildingEntity::ServerFrameSetup");

    if (!IUnitEntity::ServerFrameSetup())
        return false;

    if (Game.GetWinningTeam() == TEAM_INVALID)
    {
        // Assume it is vulnerable once the match has started
        if (Game.GetGamePhase() < GAME_PHASE_ACTIVE)
            SetUnitFlags(UNIT_FLAG_INVULNERABLE);
        else
            RemoveUnitFlags(UNIT_FLAG_INVULNERABLE);

        // If any targets > 1 are set, assume invulnerability
        for (uint ui(2); ui < NUM_BUILDING_TARGETS; ++ui)
        {
            if (m_auiTargetUIDs[ui] == INVALID_INDEX)
                continue;

            SetUnitFlags(UNIT_FLAG_INVULNERABLE);
            break;
        }

        // If any target > 1 is set, but not active, remove invulnerability
        for (uint ui(2); ui < NUM_BUILDING_TARGETS; ++ui)
        {
            if (m_auiTargetUIDs[ui] == INVALID_INDEX)
                continue;

            IGameEntity *pTarget(Game.GetEntityFromUniqueID(m_auiTargetUIDs[ui]));
            if (pTarget == nullptr || !pTarget->IsVisual() || pTarget->GetAsVisual()->GetStatus() != ENTITY_STATUS_ACTIVE)
            {
                RemoveUnitFlags(UNIT_FLAG_INVULNERABLE);
                break;
            }
        }

        // If any target < 2 is set and exists, always grant invulnerability
        for (uint ui(0); ui < 2; ++ui)
        {
            if (m_auiTargetUIDs[ui] == INVALID_INDEX)
                continue;

            IGameEntity *pTarget(Game.GetEntityFromUniqueID(m_auiTargetUIDs[ui]));
            if (pTarget == nullptr || !pTarget->IsVisual() || pTarget->GetAsVisual()->GetStatus() != ENTITY_STATUS_ACTIVE)
                continue;

            SetUnitFlags(UNIT_FLAG_INVULNERABLE);
            break;
        }
    }
    else
    {
        if (Game.GetWinningTeam() == GetTeam())
            SetUnitFlags(UNIT_FLAG_INVULNERABLE);
        else
            RemoveUnitFlags(UNIT_FLAG_INVULNERABLE);
    }

    return true;
}


/*====================
  IBuildingEntity::ServerFrameThink
  ====================*/
bool    IBuildingEntity::ServerFrameThink()
{
    if (GetAttackType() != INVALID_ATTACK_TYPE)
    {
        // Issue default behavior (Sentry)
        if (m_cBrain.IsEmpty() && m_cBrain.GetCommandsPending() == 0)
            m_cBrain.AddBehavior(K2_NEW(ctx_Game,   CBSentry), false);
    }

    return IUnitEntity::ServerFrameThink();
}


/*====================
  IBuildingEntity::ServerFrameMovement
  ====================*/
bool    IBuildingEntity::ServerFrameMovement()
{
    if (!IUnitEntity::ServerFrameMovement())
        return false;

    // Check for low health
    if (GetHealth() / GetMaxHealth() <= g_buildingLowHealthPercent && GetStatus() != ENTITY_STATUS_CORPSE && !m_bLowHealthEffectActive)
    {
        m_bLowHealthEffectActive = true;
        ResHandle hEffect(GetLowHealthEffect());
        if (hEffect != INVALID_RESOURCE)
        {
            SetEffect(EFFECT_CHANNEL_BUILDING_LOW_HEALTH, hEffect);
            IncEffectSequence(EFFECT_CHANNEL_BUILDING_LOW_HEALTH);
        }
        
        if (GetLowHealthSound() != INVALID_RESOURCE)
        {
            ivector vClients(Game.GetTeam(GetTeam())->GetClientList());

            CBufferFixed<3> buffer;
        
            buffer << GAME_CMD_BLD_HEALTH_LOW << GetType();
            for (ivector_cit it(vClients.begin()); it != vClients.end(); ++it)
                Game.SendGameData(*it, buffer, true);
        }
    }
    
    if (((GetHealth() / GetMaxHealth() > g_buildingLowHealthPercent) || GetStatus() == ENTITY_STATUS_CORPSE) && m_bLowHealthEffectActive)
    {
        m_bLowHealthEffectActive = false;
        SetEffect(EFFECT_CHANNEL_BUILDING_LOW_HEALTH, INVALID_RESOURCE);
        IncEffectSequence(EFFECT_CHANNEL_BUILDING_LOW_HEALTH);
    }

    m_uiLastGameTime = Game.GetGameTime();
    return true;
}


/*====================
  IBuildingEntity::Damage
  ====================*/
void    IBuildingEntity::Damage(CDamageEvent &damage)
{
    IUnitEntity::Damage(damage);

    bool bDamaged(m_fLethalDamageAccumulator > 0.0f || m_fNonLethalDamageAccumulator > 0.0f);

    if (bDamaged)
    {
        CTeamInfo *pTeam(Game.GetTeam(GetTeam()));

        if (pTeam != nullptr && (pTeam->GetLastBuildingAttackAnnouncement() == INVALID_TIME || pTeam->GetLastBuildingAttackAnnouncement() + g_buildingAnnounceTeamAttackTime < Game.GetGameTime()) && (m_uiLastAttackAnnounce == INVALID_TIME || m_uiLastAttackAnnounce + g_buildingAnnounceAttackTime < Game.GetGameTime()))
        {
            // Only announce building under attack if no heroes are in g_buildingAnnounceNoHeroRadius
            uivector vResult;
            Game.GetEntitiesInRadius(vResult, GetPosition().xy(), g_buildingAnnounceNoHeroRadius, 0);
            bool bAnnounce(true);

            uivector vBuildings;

            for (uivector_it it(vResult.begin()); it != vResult.end(); it++)
            {
                IUnitEntity *pUnit(Game.GetUnitEntity(Game.GetGameIndexFromWorldIndex(*it)));

                if (pUnit == nullptr)
                    continue;

                if (pUnit->IsBuilding())
                {
                    vBuildings.push_back(*it);
                    continue;
                }

                if(!pUnit->IsHero() || pUnit->GetTeam() != GetTeam())
                    continue;

                bAnnounce = false;
            }

            if (bAnnounce)
            {
                CBufferFixed<5> buffer;
                buffer << GAME_CMD_BUILDING_UNDER_ATTACK_MESSAGE << GetIndex();
                Game.BroadcastGameDataToTeam(GetTeam(), buffer, true);

                m_uiLastAttackAnnounce = Game.GetGameTime();

                for (uivector_it it(vBuildings.begin()); it != vBuildings.end(); it++)
                {
                    IUnitEntity *pUnit(Game.GetUnitEntity(Game.GetGameIndexFromWorldIndex(*it)));

                    if (pUnit == nullptr || !pUnit->IsBuilding())
                        continue;

                    pUnit->GetAsBuilding()->SetLastAttackAnnouncement(m_uiLastAttackAnnounce);
                }

                pTeam->SetLastBuildingAttackAnnouncement(m_uiLastAttackAnnounce);
            }
        }

        if (GetIsBase())
            m_uiLastDamageTime = Game.GetGameTime();
    }
}


/*====================
  AddDebugLine
  ====================*/
static void AddDebugLine(const CVec3f &v3Start, const CVec3f &v3End)
{
    CSceneEntity sceneEntity;

    sceneEntity.objtype = OBJTYPE_BEAM;
    sceneEntity.SetPosition(v3Start);
    sceneEntity.angle = v3End;
    sceneEntity.scale = 2.0f;
    sceneEntity.height = 1.0f;
    sceneEntity.color = BLUE;

    K2_WITH_GAME_RESOURCE_SCOPE()
        sceneEntity.hRes = g_ResourceManager.Register(_T("/core/materials/effect_solid.material"), RES_MATERIAL);

    SceneManager.AddEntity(sceneEntity);
}


/*====================
  IBuildingEntity::AddToScene
  ====================*/
bool    IBuildingEntity::AddToScene(const CVec4f &v4Color, int iFlags)
{
    //PROFILE("IBuildingEntity::AddToScene");

    if (GetModel() == INVALID_INDEX)
        return false;

    CVec4f v4TintedColor(v4Color);

    if (m_v3AxisAngles != m_v3UnitAngles)
    {
        m_aAxis.Set(m_v3UnitAngles);
        m_v3AxisAngles = m_v3UnitAngles;
    }

    static CSceneEntity sceneEntity;
    sceneEntity.Clear();

    sceneEntity.scale = GetBaseScale() * GetScale();
    sceneEntity.SetPosition(m_v3Position);
    sceneEntity.axis = m_aAxis;
    sceneEntity.objtype = OBJTYPE_MODEL;
    sceneEntity.hRes = GetModel();
    sceneEntity.skeleton = m_pSkeleton;
    sceneEntity.color = v4TintedColor;
    sceneEntity.flags = iFlags | SCENEENT_SOLID_COLOR | SCENEENT_USE_AXIS;

    if (IsHighlighted())
        sceneEntity.color *= m_v4HighlightColor;

    if (m_uiClientRenderFlags & ECRF_HALFTRANSPARENT)
        sceneEntity.color[A] *= 0.5f;

    SSceneEntityEntry &cEntry(SceneManager.AddEntity(sceneEntity));

    if (!cEntry.bCull || !cEntry.bCullShadow)
    {
        AddSelectionRingToScene();
        UpdateSkeleton(true);

        if (d_drawUnitBounds)
        {
            CBBoxf bbBoundsWorld(GetBounds());
            bbBoundsWorld.Offset(m_v3Position);
            vector<CVec3f> v3Points(bbBoundsWorld.GetCorners());

            AddDebugLine(v3Points[0], v3Points[1]);
            AddDebugLine(v3Points[1], v3Points[3]);
            AddDebugLine(v3Points[3], v3Points[2]);
            AddDebugLine(v3Points[2], v3Points[0]);

            AddDebugLine(v3Points[0], v3Points[4]);
            AddDebugLine(v3Points[1], v3Points[5]);
            AddDebugLine(v3Points[2], v3Points[6]);
            AddDebugLine(v3Points[3], v3Points[7]);

            AddDebugLine(v3Points[4], v3Points[5]);
            AddDebugLine(v3Points[5], v3Points[7]);
            AddDebugLine(v3Points[7], v3Points[6]);
            AddDebugLine(v3Points[6], v3Points[4]);
        }
    }
    else
    {
        UpdateSkeleton(false);
    }

    return true;
}


/*====================
  IBuildingEntity::UpdateSkeleton
  ====================*/
void    IBuildingEntity::UpdateSkeleton(bool bPose)
{
    PROFILE("IBuildingEntity::UpdateSkeleton");

    if (m_pSkeleton == nullptr)
        return;

    m_pSkeleton->SetModel(GetModel());

    if (GetModel() == INVALID_RESOURCE)
        return;

    // Pose skeleton
    if (bPose)
        m_pSkeleton->Pose(Game.GetGameTime());
    else
        m_pSkeleton->PoseLite(Game.GetGameTime());

    // Process animation events
    if (m_pSkeleton->CheckEvents())
    {
        tstring sOldDir(FileManager.GetWorkingDirectory());
        FileManager.SetWorkingDirectory(Filename_GetPath(g_ResourceManager.GetPath(GetModel())));

        const vector<SAnimEventCmd> &vEventCmds(m_pSkeleton->GetEventCmds());

        for (vector<SAnimEventCmd>::const_iterator it(vEventCmds.begin()); it != vEventCmds.end(); ++it)
            EventScript.Execute(it->sCmd, it->iTimeNudge);

        m_pSkeleton->ClearEvents();

        FileManager.SetWorkingDirectory(sOldDir);
    }
}


/*====================
  IBuildingEntity::SetTeam
  ====================*/
void    IBuildingEntity::SetTeam(uint uiTeam)
{
    if (uiTeam == GetTeam())
        return;
    
    IUnitEntity::SetTeam(uiTeam);

    CTeamInfo *pTeamInfo(Game.GetTeam(uiTeam));
    if (pTeamInfo)
        pTeamInfo->AddBuildingUID(GetUniqueID());
}


/*====================
  IBuildingEntity::Copy
  ====================*/
void    IBuildingEntity::Copy(const IGameEntity &B)
{
    IUnitEntity::Copy(B);

    const IBuildingEntity *pB(B.GetAsBuilding());

    if (!pB)    
        return;

    const IBuildingEntity &C(*pB);

    m_uiLastDamageTime = C.m_uiLastDamageTime;
    m_uiLastAttackAnnounce = C.m_uiLastAttackAnnounce;
}


/*====================
  IBuildingEntity::ClientPrecache
  ====================*/
void    IBuildingEntity::ClientPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier)
{
    IUnitEntity::ClientPrecache(pConfig, eScheme, sModifier);
    s_hMinimapAttackTargetIcon = Game.RegisterIcon(g_buildingMapAttackTargetIcon);
    s_hMinimapDefendTargetIcon = Game.RegisterIcon(g_buildingMapDefendTargetIcon);
}


/*====================
  IBuildingEntity::ServerPrecache
  ====================*/
void    IBuildingEntity::ServerPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier)
{
    IUnitEntity::ServerPrecache(pConfig, eScheme, sModifier);
}


/*====================
  IBuildingEntity::GetThreatLevel
  ====================*/
float   IBuildingEntity::GetThreatLevel(IUnitEntity *pOther, bool bCurrentTarget)
{
    float fThreatLevel(IUnitEntity::GetThreatLevel(pOther, bCurrentTarget));

    if (pOther->GetCombatType() != _T("Siege"))
    {
        if (pOther->GetLastHeroAttackTime() != INVALID_TIME &&
            pOther->GetLastHeroAttackTime() + g_buildingHelpHeroDuration > Game.GetGameTime())
        {
            if (pOther->IsHero())
                fThreatLevel += 1100.0f;
            else
                fThreatLevel += 1000.0f;

            if (bCurrentTarget)
                fThreatLevel += 50.0f;
        }
        else if (bCurrentTarget ||
            pOther->GetCurrentAttackStateTarget() == GetIndex() ||
            pOther->GetCurrentAttackBehaviorTarget() == GetIndex() ||
            (pOther->GetLastAttackTargetUID() == GetUniqueID() && pOther->GetLastAttackTargetTime() + g_buildingTargetMemory >= Game.GetGameTime()))
        {
            fThreatLevel += 900.0f;
        }
    }

    return fThreatLevel;
}


/*====================
  IBuildingEntity::GetBaseArmor
  ====================*/
float   IBuildingEntity::GetBaseArmor() const
{
    if (Game.GetTowerHeroArmorReduction() && !GetNoHeroArmorReduction())
    {
        CTeamInfo *pTeam(Game.GetTeam(GetTeam()));
        if (pTeam == nullptr)
            return IUnitEntity::GetBaseArmor();
        else
            return IUnitEntity::GetBaseArmor() * pTeam->GetHeroAlivePercent();
    }
    else
    {
        return IUnitEntity::GetBaseArmor();
    }
}
