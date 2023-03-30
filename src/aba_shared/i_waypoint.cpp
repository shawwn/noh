// (C)2008 S2 Games
// i_waypoint.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_waypoint.h"

#include "../k2/c_xmlprocroot.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_skeleton.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
uint        IWaypoint::s_uiBaseType(ENTITY_BASE_TYPE_WAYPOINT);

DEFINE_ENTITY_DESC(IWaypoint, 1)
{
    s_cDesc.pFieldTypes = K2_NEW(g_heapTypeVector,   TypeVector)();
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_v3Position"), TYPE_ROUNDPOS3D, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiOwnerIndex"), TYPE_GAMEINDEX, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiUnitIndex"), TYPE_GAMEINDEX, 0, 0));
}
//=============================================================================

//=============================================================================
// Resource
//=============================================================================
DEFINE_DEFINITION_TYPE_INFO(CWaypointDefinition, ENTITY_BASE_TYPE_WAYPOINT, Waypoint)

START_ENTITY_DEFINITION_XML_PROCESSOR(IWaypoint, Waypoint)
    READ_ENTITY_DEFINITION_PROPERTY(Model, model)
    READ_ENTITY_DEFINITION_PROPERTY_EX(ModelScale, modelscale, 1.0)
END_ENTITY_DEFINITION_XML_PROCESSOR(Waypoint, waypoint)
//=============================================================================

/*====================
  IWaypoint::IWaypoint
  ====================*/
IWaypoint::IWaypoint() :
m_uiOwnerIndex(INVALID_INDEX),
m_uiUnitIndex(INVALID_INDEX)
{
}


/*====================
  IWaypoint::Baseline
  ====================*/
void    IWaypoint::Baseline()
{
    m_v3Position = V3_ZERO;
    m_uiOwnerIndex = INVALID_INDEX;
    m_uiUnitIndex = INVALID_INDEX;
}


/*====================
  IWaypoint::GetSnapshot
  ====================*/
void    IWaypoint::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
    snapshot.WriteRoundPos3D(m_v3Position);
    snapshot.WriteGameIndex(m_uiOwnerIndex);
    snapshot.WriteGameIndex(m_uiUnitIndex);
}


/*====================
  IWaypoint::ReadSnapshot
  ====================*/
bool    IWaypoint::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
    snapshot.ReadRoundPos3D(m_v3Position);
    snapshot.ReadGameIndex(m_uiOwnerIndex);
    snapshot.ReadGameIndex(m_uiUnitIndex);

    Validate();
    
    return true;
}


/*====================
  IWaypoint::Copy
  ====================*/
void    IWaypoint::Copy(const IGameEntity &B)
{
    IVisualEntity::Copy(B);

    const IWaypoint *pB(static_cast<const IWaypoint *>(&B));
    if (!pB)
        return;

    const IWaypoint &C(*pB);

    m_uiOwnerIndex = C.m_uiOwnerIndex;
    m_uiUnitIndex = C.m_uiUnitIndex;
}


/*====================
  IWaypoint::AllocateSkeleton
  ====================*/
CSkeleton*  IWaypoint::AllocateSkeleton()
{
    return m_pSkeleton = K2_NEW(g_heapSkeleton,   CSkeleton);
}


/*====================
  IWaypoint::Spawn
  ====================*/
void    IWaypoint::Spawn()
{
    IVisualEntity::Spawn();

    StartAnimation(_T("idle"), 0);
}


/*====================
  IWaypoint::ServerFrameCleanup
  ====================*/
bool    IWaypoint::ServerFrameCleanup()
{
    IVisualEntity *pAttach(Game.GetVisualEntity(m_uiUnitIndex));
    if (pAttach != NULL)
        m_v3Position = pAttach->GetPosition();

    return true;
}


/*====================
  IWaypoint::AddToScene
  ====================*/
bool    IWaypoint::AddToScene(const CVec4f &v4Color, int iFlags)
{
    if (GetModel() == INVALID_INDEX)
        return false;

    if (m_uiOwnerIndex != Game.GetActiveControlEntity())
        return false;

    CPlayer *pLocalPlayer(Game.GetLocalPlayer());
    if (pLocalPlayer == NULL)
        return false;

    if (m_uiUnitIndex != INVALID_INDEX && !pLocalPlayer->CanSee(Game.GetVisualEntity(m_uiUnitIndex)))
        return false;

    CVec4f v4TintedColor(v4Color);

    if (m_v3AxisAngles != m_v3Angles)
    {
        m_aAxis.Set(m_v3Angles);
        m_v3AxisAngles = m_v3Angles;
    }

    static CSceneEntity sceneEntity;

    sceneEntity.Clear();
    sceneEntity.scale = GetModelScale();
    sceneEntity.SetPosition(m_v3Position);
    sceneEntity.axis = m_aAxis;
    sceneEntity.objtype = OBJTYPE_MODEL;
    sceneEntity.hRes = GetModel();
    sceneEntity.skeleton = m_pSkeleton;
    sceneEntity.color = v4TintedColor;
    sceneEntity.flags = iFlags | SCENEENT_SOLID_COLOR | SCENEENT_USE_AXIS;

    SSceneEntityEntry &cEntry(SceneManager.AddEntity(sceneEntity));

    if (!cEntry.bCull || !cEntry.bCullShadow)
        UpdateSkeleton(true);
    else
        UpdateSkeleton(false);

    return true;
}
