// (C)2005 S2 Games
// c_entitytool.cpp
//
// Entity creation
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "editor_common.h"

#include "editor.h"

#include "c_entitytool.h"
#include "c_toolbox.h"
#include "c_treedefinitionresource.h"

#include "../hon_shared/c_entityregistry.h"
#include "../hon_shared/c_entitydefinitionresource.h"
#include "../hon_shared/i_unitdefinition.h"

#include "../k2/c_brush.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_input.h"
#include "../k2/c_vid.h"
#include "../k2/c_draw2d.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_world.h"
#include "../k2/c_worldentity.h"
#include "../k2/c_uimanager.h"
#include "../k2/i_widget.h"
#include "../k2/c_uicmd.h"
#include "../k2/c_uitrigger.h"
#include "../k2/c_function.h"
#include "../k2/c_fontmap.h"
#include "../k2/c_model.h"
#include "../k2/c_skin.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_INT    (le_entityEditMode,             ENTITY_CREATE);
CVAR_INT    (le_entityCenterMode,           CENTER_AVERAGE);
CVAR_BOOL   (le_entityHoverDelete,          true);
CVAR_FLOAT  (le_entityRaiseSensitivity,     2.5f);
CVAR_FLOAT  (le_entityRotateSensitivity,    0.2f);
CVAR_FLOAT  (le_entityScaleSensitivity,     0.02f);
CVAR_FLOAT  (le_entityPositionSnap,         32.0f);
CVAR_FLOAT  (le_entityHeightSnap,           16.0f);
CVAR_FLOAT  (le_entityAngleSnap,            45.0f);
CVAR_FLOAT  (le_entityScaleSnap,            0.5f);
CVAR_BOOL   (le_entitySnap,                 false);
CVAR_BOOL   (le_entitySnapAbsolute,         true);
CVAR_BOOL   (le_entitySelectionLock,        false);
CVAR_STRING (le_entityModel,                "/core/null/null.mdf");
CVAR_STRING (le_entityTreeDefinition,       "/world/props/trees/t_1/tree.tree");
CVAR_STRING (le_entityModelDelta,           "/world/props/trees/t_1/model.mdf");
CVAR_STRING (le_entitySkin,                 "default");
CVAR_STRING (le_entityType,                 "Prop_Scenery");
CVAR_INT    (le_entityTeam,                 0);
CVAR_INT    (le_entityLane,                 0);
CVAR_INT    (le_entityValue,                0);
CVAR_BOOLF  (le_entityDrawBrushCoords,      true,                   CVAR_SAVECONFIG);
CVAR_BOOL   (le_entityDrawBrushInfluence,   true);
CVAR_FLOAT  (le_entityBrushInfluenceAlpha,  1.0f);
CVAR_INT    (le_treeMode,                   0);

UI_TRIGGER(EntityEditMode);
UI_TRIGGER(EntitySelection);
UI_TRIGGER(EntitySelectionModel);
UI_TRIGGER(EntitySelectionSkin);
UI_TRIGGER(EnittySelectionTeam);
UI_TRIGGER(EntitySelectionType);

UI_TRIGGER(EntityTypeList);
//=============================================================================

/*====================
  CEntityTool::~CEntityTool
  ====================*/
CEntityTool::~CEntityTool()
{
}


/*====================
  CEntityTool::CEntityTool()
  ====================*/
CEntityTool::CEntityTool() :
ITool(TOOL_ENTITY, _T("entity")),
m_uiHoverEnt(INVALID_INDEX),
m_iState(STATE_HOVERING),
m_vTranslate(0.0f, 0.0f, 0.0f),
m_vTrueTranslate(0.0f, 0.0f, 0.0f),
m_fScale(1.0f),
m_fTrueScale(1.0f),
m_vRotation(0.0f, 0.0f, 0.0f),
m_vTrueRotation(0.0f, 0.0f, 0.0f),
m_bSnapCursor(false),
m_bCloning(false),
m_iOverMode(0),
m_hModel(INVALID_RESOURCE),
m_hLineMaterial(g_ResourceManager.Register(_T("/core/materials/line.material"), RES_MATERIAL)),
m_bValidPosition(false),
m_hFont(g_ResourceManager.LookUpName(_T("system_medium"), RES_FONTMAP))
{
    EntityEditMode.Trigger(_T("Create"));

    map<ushort, tstring> mapEntities;
    EntityRegistry.GetEntityList(mapEntities);
    tsvector vEntities;
    for (map<ushort, tstring>::iterator it(mapEntities.begin()); it != mapEntities.end(); ++it)
    {
        if (it->first <= Entity_Tangible)
            continue;

        const CDynamicEntityAllocator *pAllocator(EntityRegistry.GetDynamicAllocator(it->first));
        if (pAllocator != NULL && GET_ENTITY_BASE_TYPE1(pAllocator->GetBaseType()) != ENTITY_BASE_TYPE1_UNIT &&
            GET_ENTITY_BASE_TYPE1(pAllocator->GetBaseType()) != ENTITY_BASE_TYPE1_AFFECTOR)
            continue;

        vEntities.push_back(it->second);
    }

    sort(vEntities.begin(), vEntities.end());
    for (tsvector_it it(vEntities.begin()); it != vEntities.end(); ++it)
        EntityTypeList.Trigger(*it);
}


/*====================
  CEntityTool::PrimaryUp
  ====================*/
void    CEntityTool::PrimaryUp()
{
    if (m_bCloning && m_vStartCursorPos == Input.GetCursorPos())
    {
        m_bCloning = false;
        Delete();

        // Cancel current action
        m_vTranslate = m_vTrueTranslate = V_ZERO;
        m_vRotation = m_vTrueRotation = V_ZERO;
        m_fScale = m_fTrueScale = 1.0f;

        m_setSelection = m_setOldSelection;
        m_iState = STATE_HOVERING;
        m_vStartCursorPos.Clear();

        EntitySelection.Trigger(XtoA(!m_setSelection.empty()));
        return;
    }

    // Apply the operation
    switch (m_iOverMode)
    {
    case ENTITY_MODE_NORMAL:
        switch(m_iState)
        {
        case STATE_SELECT:
            ApplySelect();
            break;

        case STATE_TRANSLATE_XY:
            ApplyTranslateXY();
            break;

        case STATE_TRANSLATE_Z:
            ApplyTranslateZ();
            break;

        case STATE_SCALE_UNIFORM:
            ApplyScaleUniform();
            break;

        case STATE_ROTATE_YAW:
            ApplyRotation(YAW);
            break;

        case STATE_ROTATE_PITCH:
            ApplyRotation(PITCH);
            break;

        case STATE_ROTATE_ROLL:
            ApplyRotation(ROLL);
            break;
        }
        break;
    case ENTITY_MODE_TREE:
        switch(m_iState)
        {
        case STATE_SELECT:
            ApplyTreeSelect();
            break;

        case STATE_TRANSLATE_XY:
            ApplyTreeTranslateXY();
            break;

        case STATE_TRANSLATE_Z:
            ApplyTranslateZ();
            break;

        case STATE_SCALE_UNIFORM:
            ApplyScaleUniform();
            break;

        case STATE_ROTATE_YAW:
            ApplyRotation(YAW);
            break;

        case STATE_ROTATE_PITCH:
            ApplyRotation(PITCH);
            break;

        case STATE_ROTATE_ROLL:
            ApplyRotation(ROLL);
            break;
        case STATE_CREATE:
            m_iState = STATE_HOVERING;
        }
        break;
    }



    m_bCloning = false;
    m_iState = STATE_HOVERING;
    m_vStartCursorPos.Clear();
}


/*====================
  CEntityTool::PrimaryDown

  Default left mouse button down action
  ====================*/
void    CEntityTool::PrimaryDown()
{
    CalcToolProperties();

    switch (m_iOverMode)
    {
    case ENTITY_MODE_NORMAL:
        switch(le_entityEditMode)
        {
        case ENTITY_CREATE:
            Create();
            break;

        case ENTITY_SELECT:
            StartSelect();
            break;

        case ENTITY_TRANSLATE:
            StartTranslateXY();
            break;

        case ENTITY_TRANSLATE_Z:
            StartTransform(STATE_TRANSLATE_Z);
            break;

        case ENTITY_SCALE:
            StartTransform(STATE_SCALE_UNIFORM);
            break;

        case ENTITY_ROTATE_YAW:
            StartTransform(STATE_ROTATE_YAW);
            break;

        case ENTITY_ROTATE_PITCH:
            StartTransform(STATE_ROTATE_PITCH);
            break;

        case ENTITY_ROTATE_ROLL:
            StartTransform(STATE_ROTATE_ROLL);
            break;
        }   
        break;
    case ENTITY_MODE_TREE:
        switch(le_entityEditMode)
        {
        case ENTITY_CREATE:
            TreeCreate();
            break;
        case ENTITY_SELECT:
            StartTreeSelect();
            break;
        
        case ENTITY_TRANSLATE:
            StartTreeTranslateXY();
            break;

        case ENTITY_TRANSLATE_Z:
            StartTransform(STATE_TRANSLATE_Z);
            break;

        case ENTITY_SCALE:
            StartTransform(STATE_SCALE_UNIFORM);
            break;

        case ENTITY_ROTATE_YAW:
            StartTransform(STATE_ROTATE_YAW);
            break;

        case ENTITY_ROTATE_PITCH:
            StartTransform(STATE_ROTATE_PITCH);
            break;

        case ENTITY_ROTATE_ROLL:
            StartTransform(STATE_ROTATE_ROLL);
            break;
        }
        break;
    }
}
    


/*====================
  CEntityTool::SecondaryDown

  Default right mouse button down action
  ====================*/
void    CEntityTool::SecondaryDown()
{
    if (m_bCloning)
    {
        m_bCloning = false;
        Delete();
        m_setSelection = m_setOldSelection;

        EntitySelection.Trigger(XtoA(!m_setSelection.empty()));
    }

    // Cancel current action
    m_vTranslate = m_vTrueTranslate = CVec3f(0.0f, 0.0f, 0.0f);
    m_vRotation = m_vTrueRotation = CVec3f(0.0f, 0.0f, 0.0f);
    m_fScale = m_fTrueScale = 1.0f;

    m_iState = STATE_HOVERING;

    // TODO: show context menu
}


/*====================
  CEntityTool::TertiaryDown

  Scroll wheel up action
  ====================*/
void    CEntityTool::TertiaryDown()
{
    try
    {
        for (uiset_it it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
        {
            if (*it != INVALID_INDEX)
                Editor.GetWorld().GetEntity(*it, true)->DecrementSeed();
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityTool::TertiaryDown() -"), NO_THROW);
    }
}


/*====================
  CEntityTool::QuaternaryDown

  Scroll wheel down action
  ====================*/
void    CEntityTool::QuaternaryDown()
{
    try
    {
        for (uiset_it it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
        {
            if (*it != INVALID_INDEX)
                Editor.GetWorld().GetEntity(*it, true)->IncrementSeed();
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityTool::TertiaryDown() -"), NO_THROW);
    }
}


/*====================
  CEntityTool::Cancel
  ====================*/
void    CEntityTool::Cancel()
{
    m_setSelection.clear();
    EntitySelection.Trigger(XtoA(!m_setSelection.empty()));
    le_entitySelectionLock = false;
}


/*====================
  CEntityTool::Delete
  ====================*/
void    CEntityTool::Delete()
{
    if (!m_setSelection.empty())
    {
        uiset set(m_setSelection); // copy!

        for (uiset::iterator it(set.begin()); it != set.end(); ++it)
        {
            Editor.GetWorld().UnlinkEntity(*it);
            Editor.GetWorld().DeleteEntity(*it);

            map<uint, CWorldEntityEx>::iterator findit(g_WorldEntData.find(*it));
            if (findit != g_WorldEntData.end())
            {
                vector<PoolHandle> &vPathBlockers(findit->second.GetPathBlockers());

                vector<PoolHandle>::const_iterator citEnd(vPathBlockers.end());
                for (vector<PoolHandle>::const_iterator cit(vPathBlockers.begin()); cit != citEnd; ++cit)
                    Editor.GetWorld().ClearPath(*cit);

                vPathBlockers.clear();

                g_WorldEntData.erase(findit);
            }
        }

        m_setSelection.clear();
        m_uiHoverEnt = INVALID_INDEX;

        EntitySelection.Trigger(XtoA(false));
    }
    else if (m_uiHoverEnt != INVALID_INDEX && le_entityHoverDelete)
    {
        m_setSelection.erase(m_uiHoverEnt);
        Editor.GetWorld().UnlinkEntity(m_uiHoverEnt);
        Editor.GetWorld().DeleteEntity(m_uiHoverEnt);

        map<uint, CWorldEntityEx>::iterator findit(g_WorldEntData.find(m_uiHoverEnt));
        if (findit != g_WorldEntData.end())
        {
            vector<PoolHandle> &vPathBlockers(findit->second.GetPathBlockers());

            vector<PoolHandle>::const_iterator citEnd(vPathBlockers.end());
            for (vector<PoolHandle>::const_iterator cit(vPathBlockers.begin()); cit != citEnd; ++cit)
                Editor.GetWorld().ClearPath(*cit);

            vPathBlockers.clear();

            g_WorldEntData.erase(findit);
        }

        m_uiHoverEnt = INVALID_INDEX;

        EntitySelection.Trigger(XtoA(false));
    }
}


/*====================
  CEntityTool::CalcToolProperties
  ====================*/
void     CEntityTool::CalcToolProperties()
{
    bool bFullTrace(m_iState == STATE_HOVERING && !le_entitySelectionLock);

    STraceInfo trace;
    if (Editor.TraceCursor(trace, bFullTrace ? SURF_HULL | SURF_BLOCKER : TRACE_TERRAIN))
    {
        m_v3EndPos = trace.v3EndPos;

        if (trace.uiEntityIndex != INVALID_INDEX)
        {
            CWorldEntity *pWorldEnt(Editor.GetWorld().GetEntity(trace.uiEntityIndex));
            if(pWorldEnt)
            {
                tstring sEntType(_T(""));
                sEntType = pWorldEnt->GetType();
                if(m_iOverMode == ENTITY_MODE_TREE && sEntType != _T("Prop_Tree"))
                    trace.uiEntityIndex = INVALID_INDEX;
            }
        }

        if (bFullTrace)
            m_uiHoverEnt = trace.uiEntityIndex;

        m_bValidPosition = true;

    }
    else
    {
        m_bValidPosition = false;
        m_v3EndPos.Clear();

        if (bFullTrace)
            m_uiHoverEnt = INVALID_INDEX;
    }

    if (!bFullTrace && le_entitySelectionLock)
        m_uiHoverEnt = INVALID_INDEX;
}

/*====================
  CSpawnTool::CreateSpawn
  ====================*/
//uint  CSpawnTool::CreateSpawn(float fX, float fY)
//{
//  try
//  {
//      uint uiNewSpawn(Editor.GetWorld().AllocateNewEntity());
//      CWorldEntity *pNewSpawn(Editor.GetWorld().GetEntity(uiNewSpawn, true));
//
//      pNewSpawn->SetPosition(fX, fY, Editor.GetWorld().GetTerrainHeight(fX, fY));
//      
//      if (m_iOverMode == SPAWN_HEROSPAWNPOINT)
//      {
//          pNewSpawn->SetModelHandle(g_ResourceManager.Register(_T("/tools/m.mdf"), RES_MODEL));
//          pNewSpawn->SetModelPath(_T("/tools/m.mdf"));
//
//          pNewSpawn->SetType(_T("Trigger_SpawnPoint"));
//          pNewSpawn->SetTeam(SPAWN_TEAM_LEGION);
//          pNewSpawn->SetSkin(g_ResourceManager.GetSkin(pNewSpawn->GetModelHandle(), _T("green")));
//
//          IUnitDefinition *pDefinition(EntityRegistry.GetDefinition<IUnitDefinition>(_T("Trigger_SpawnPoint")));
//          if (pDefinition != NULL)
//          {
//              Editor.GetWorld().UnlinkEntity(pNewSpawn->GetIndex());
//              pNewSpawn->SetScale(pDefinition->GetPreGlobalScale(0) * pDefinition->GetModelScale(0));
//              pNewSpawn->SetModelHandle(g_ResourceManager.Register(_T("/tools/m.mdf"), RES_MODEL));
//              pNewSpawn->SetSkin(g_ResourceManager.GetSkin(pNewSpawn->GetModelHandle(), _T("green")));
//              g_ResourceManager.PrecacheSkin(pNewSpawn->GetModelHandle(), pNewSpawn->GetSkin());
//          }
//      }
//      if (m_iOverMode == SPAWN_CREEPPATH)
//      {
//          pNewSpawn->SetModelHandle(g_ResourceManager.Register(le_spawnModel, RES_MODEL));
//          pNewSpawn->SetModelPath(le_spawnModel);
//          pNewSpawn->SetType(_T("Entity_CreepSpawner"));
//          pNewSpawn->SetTeam(le_spawnTeam);
//          
//          //if (le_spawnTeam == SPAWN_TEAM_HELLBOURNE)
//          //{
//          //  pNewSpawn->SetSkin(g_ResourceManager.GetSkin(pNewSpawn->GetModelHandle(), _T("default")));
//          //  uint uiWayPoint(++m_BadCreepPath[le_spawnCreepPath].uiWayPoints);
//          //  m_BadCreepPath[le_spawnCreepPath].WayPoint[uiWayPoint].pWaypointEntity = pNewSpawn;
//
//          //  if (m_BadCreepPath[le_spawnCreepPath].uiWayPoints == 1)
//          //  {
//          //      //m_BadCreepPath[le_spawnCreepPath];
//          //      pNewSpawn->SetName(_T("bad_creep") + le_spawnCreepPath);    
//          //      //pNewSpawn->SetProperty(_T("target0"), _T("bad_wypt_") + le_spawnCreepPath + _T("_") + m_BadCreepPath[le_spawnLane].uiWayPoints);
//          //      pNewSpawn->SetProperty(_T("target0"), _T("");
//          //      pNewSpawn->SetProperty(_T("target1"), _T("bad_melee") + le_spawnCreepPath);
//          //      pNewSpawn->SetProperty(_T("target2"), _T("bad_ranged") + le_spawnCreepPath);
//          //      pNewSpawn->SetProperty(_T("target3"), _T("good_creep") + le_spawnCreepPath);
//          //      pNewSpawn->SetProperty(_T("target4"), _T(""));
//          //  }
//          //  else 
//          //  { 
//          //      pNewSpawn->SetName(_T("bad_wypt_") + le_spawnCreepPath + _T("_") + m_BadCreepPath[le_spawnLane].uiWayPoints);   
//          //      pNewSpawn->SetProperty(_T("target0"), _T("bad_wypt_") + le_spawnCreepPath + _T("_") + (m_BadCreepPath[le_spawnLane].uiWayPoints + 1));
//          //      pNewSpawn->SetProperty(_T("target1"), _T(""));
//          //      pNewSpawn->SetProperty(_T("target2"), _T(""));
//          //      pNewSpawn->SetProperty(_T("target3"), _T(""));
//          //      pNewSpawn->SetProperty(_T("target4"), _T(""));
//          //  }
//          //}
//          //else if (le_spawnTeam == SPAWN_TEAM_LEGION)
//          //{
//          //  pNewSpawn->SetSkin(g_ResourceManager.GetSkin(pNewSpawn->GetModelHandle(), _T("yellow")));
//          //  ++le_spawnCreepPathWypt;
//
//          //  if (le_spawnCreepPathWypt == 1)
//          //  {
//          //      pNewSpawn->SetName(_T("good_creep") + le_spawnCreepPath);   
//          //      pNewSpawn->SetProperty(_T("target0"), _T("good_wypt_") + le_spawnCreepPath + _T("_") + le_spawnCreepPathWypt);
//          //      pNewSpawn->SetProperty(_T("target1"), _T("good_melee") + le_spawnCreepPath);
//          //      pNewSpawn->SetProperty(_T("target2"), _T("good_ranged") + le_spawnCreepPath);
//          //      pNewSpawn->SetProperty(_T("target3"), _T("bad_creep") + le_spawnCreepPath);
//          //      pNewSpawn->SetProperty(_T("target4"), _T(""));
//          //  }
//          //  else 
//          //  { 
//          //      pNewSpawn->SetName(_T("good_wypt_") + le_spawnCreepPath + _T("_") + le_spawnCreepPathWypt); 
//          //      pNewSpawn->SetProperty(_T("target0"), _T("good_wypt_") + le_spawnCreepPath + _T("_") + (le_spawnCreepPathWypt + 1));
//          //      pNewSpawn->SetProperty(_T("target1"), _T(""));
//          //      pNewSpawn->SetProperty(_T("target2"), _T(""));
//          //      pNewSpawn->SetProperty(_T("target3"), _T(""));
//          //      pNewSpawn->SetProperty(_T("target4"), _T(""));
//          //  }
//          //}
//
//          /*IUnitDefinition *pDefinition(EntityRegistry.GetDefinition<IUnitDefinition>(_T("Entity_CreepSpawner")));
//          if (pDefinition != NULL)
//          {
//              Editor.GetWorld().UnlinkEntity(pNewSpawn->GetIndex());
//              pNewSpawn->SetScale(pDefinition->GetPreGlobalScale(0) * pDefinition->GetModelScale(0));
//              pNewSpawn->SetModelHandle(g_ResourceManager.Register(_T("/tools/m.mdf"), RES_MODEL));
//              if (le_spawnTeam == SPAWN_TEAM_HELLBOURNE)
//                  pNewSpawn->SetSkin(g_ResourceManager.GetSkin(pNewSpawn->GetModelHandle(), _T("default")));
//              else if (le_spawnTeam == SPAWN_TEAM_LEGION)
//                  pNewSpawn->SetSkin(g_ResourceManager.GetSkin(pNewSpawn->GetModelHandle(), _T("yellow")));
//              g_ResourceManager.PrecacheSkin(pNewSpawn->GetModelHandle(), pNewSpawn->GetSkin());
//          }*/
//      }
//      if (m_iOverMode == SPAWN_CREATURESPAWN)
//      {
//          pNewSpawn->SetModelHandle(g_ResourceManager.Register(le_spawnModel, RES_MODEL));
//          pNewSpawn->SetModelPath(le_spawnModel);
//
//          pNewSpawn->SetType(_T("Trigger_SpawnPoint"));
//          pNewSpawn->SetTeam(SPAWN_TEAM_NEUTRAL);
//          pNewSpawn->SetSkin(g_ResourceManager.GetSkin(pNewSpawn->GetModelHandle(), _T("yellow")));
//
//          /*IUnitDefinition *pDefinition(EntityRegistry.GetDefinition<IUnitDefinition>(le_spawnType));
//          if (pDefinition != NULL)
//          {
//              Editor.GetWorld().UnlinkEntity(pNewSpawn->GetIndex());
//              pNewSpawn->SetScale(pDefinition->GetPreGlobalScale(0) * pDefinition->GetModelScale(0));
//              pNewSpawn->SetModelHandle(g_ResourceManager.Register(_T("/tools/m.mdf"), RES_MODEL));
//              pNewSpawn->SetSkin(g_ResourceManager.GetSkin(pNewSpawn->GetModelHandle(), _T("green")));
//              g_ResourceManager.PrecacheSkin(pNewSpawn->GetModelHandle(), pNewSpawn->GetSkin());
//          }*/
//      }
//      /*if (m_iOverMode == SPAWN_MODE_NORMAL)
//      {
//          pNewSpawn->SetModelHandle(g_ResourceManager.Register(le_spawnModel, RES_MODEL));
//          pNewSpawn->SetModelPath(le_spawnModel);
//
//          pNewSpawn->SetType(le_spawnType);
//
//          IUnitDefinition *pDefinition(EntityRegistry.GetDefinition<IUnitDefinition>(le_spawnType));
//          if (pDefinition != NULL)
//          {
//              Editor.GetWorld().UnlinkEntity(pNewSpawn->GetIndex());
//              pNewSpawn->SetScale(pDefinition->GetPreGlobalScale(0) * pDefinition->GetModelScale(0));
//              pNewSpawn->SetModelHandle(g_ResourceManager.Register(pDefinition->GetModelPath(0), RES_MODEL));
//              pNewSpawn->SetSkin(g_ResourceManager.GetSkin(pNewSpawn->GetModelHandle(), pDefinition->GetSkin(0)));
//              g_ResourceManager.PrecacheSkin(pNewSpawn->GetModelHandle(), pNewSpawn->GetSkin());
//          }
//      }
//      else if (m_iOverMode == SPAWN_MODE_TREE)
//      {
//          pNewSpawn->SetModelHandle(g_ResourceManager.Register(le_spawnModelDelta, RES_MODEL));
//          pNewSpawn->SetModelPath(le_spawnModelDelta);
//      }*/
//
//      Editor.GetWorld().LinkEntity(uiNewSpawn, LINK_SURFACE | LINK_MODEL, SURF_PROP);
//      return uiNewSpawn;
//  }
//  catch (CException &ex)
//  {
//      ex.Process(_T("CSpawnTool::CreateSpawn() - "), NO_THROW);
//      return INVALID_INDEX;
//  }
//}


/*====================
  CEntityTool::TranslateXY
  ====================*/
void    CEntityTool::TranslateXY()
{
    if (Input.GetCursorPos() == m_vStartCursorPos)
        return;

    if (m_bSnapCursor) // to delay the first update until the mouse moves
    {
        if (le_entityCenterMode == CENTER_HOVER || le_entityCenterMode == CENTER_INDIVIDUAL && m_uiHoverEnt != INVALID_INDEX)
        {
            CWorldEntity *pEntity(Editor.GetWorld().GetEntity(m_uiHoverEnt));
            if (pEntity != NULL)
                SnapCursor(CVec3f(pEntity->GetPosition().x, pEntity->GetPosition().y, Editor.GetWorld().GetTerrainHeight(pEntity->GetPosition().x, pEntity->GetPosition().y)));
        }
        else
        {
            CVec3f vCenter(SelectionCenter());
            SnapCursor(CVec3f(vCenter.x, vCenter.y, Editor.GetWorld().GetTerrainHeight(vCenter.x, vCenter.y)));
        }

        m_bSnapCursor = false;
        return;
    }

    m_vTrueTranslate = m_v3EndPos;

    if (le_entitySnap)
    {
        if (le_entitySnapAbsolute)
        {
            m_vTranslate.x = ROUND(m_vTrueTranslate.x / le_entityPositionSnap) * le_entityPositionSnap;
            m_vTranslate.y = ROUND(m_vTrueTranslate.y / le_entityPositionSnap) * le_entityPositionSnap;
        }
        else
        {
            m_vTranslate.x = m_vTrueTranslate.x - fmod(m_vTrueTranslate.x, le_entityPositionSnap);
            m_vTranslate.y = m_vTrueTranslate.y - fmod(m_vTrueTranslate.y, le_entityPositionSnap);
        }

        m_vTranslate.z = m_vTrueTranslate.z;
    }
    else
    {
        m_vTranslate = m_vTrueTranslate;
    }

    m_vStartCursorPos.Clear();
}


/*====================
  CEntityTool::TranslateZ
  ====================*/
void    CEntityTool::TranslateZ()
{
    CVec2f pos = Input.GetCursorPos();

    float dY = pos.y - m_vOldCursorPos.y;

    if (dY != 0.0f)
    {
        m_vTrueTranslate.z -= dY * le_entityRaiseSensitivity;

        if (le_entitySnap)
        {
            if (le_entitySnapAbsolute)
                m_vTranslate.z = ROUND(m_vTrueTranslate.z / le_entityHeightSnap) * le_entityHeightSnap;
            else
                m_vTranslate.z = m_vTrueTranslate.z - fmod(m_vTrueTranslate.z, le_entityHeightSnap);
        }
        else
            m_vTranslate.z = m_vTrueTranslate.z;
    }
}


/*====================
  CEntityTool::RotateYaw
  ====================*/
void    CEntityTool::RotateYaw()
{
    CVec2f pos = Input.GetCursorPos();

    float dX = pos.x - m_vOldCursorPos.x;

    if (dX != 0.0f)
    {
        m_vTrueRotation[YAW] += dX * le_entityRotateSensitivity;
        m_vTrueRotation[YAW] = fmod(m_vTrueRotation[YAW], 360.0f);

        if (le_entitySnap && m_iOverMode != ENTITY_MODE_TREE)
            m_vRotation[YAW] = m_vTrueRotation[YAW] - fmod(m_vTrueRotation[YAW], le_entityAngleSnap);
        else
            m_vRotation[YAW] = m_vTrueRotation[YAW];
    }
}


/*====================
  CEntityTool::RotatePitch
  ====================*/
void    CEntityTool::RotatePitch()
{
    CVec2f pos = Input.GetCursorPos();

    float dY = pos.y - m_vOldCursorPos.y;

    if (dY != 0.0f)
    {
        m_vTrueRotation[PITCH] -= dY * le_entityRotateSensitivity;
        m_vTrueRotation[PITCH] = fmod(m_vTrueRotation[PITCH], 360.0f);

        if (le_entitySnap && m_iOverMode != ENTITY_MODE_TREE)
            m_vRotation[PITCH] = m_vTrueRotation[PITCH] - fmod(m_vTrueRotation[PITCH], le_entityAngleSnap);
        else
            m_vRotation[PITCH] = m_vTrueRotation[PITCH];
    }
}


/*====================
  CEntityTool::RotateRoll
  ====================*/
void    CEntityTool::RotateRoll()
{
    CVec2f pos = Input.GetCursorPos();

    float dX = pos.x - m_vOldCursorPos.x;

    if (dX != 0.0f)
    {
        m_vTrueRotation[ROLL] += dX * le_entityRotateSensitivity;
        m_vTrueRotation[ROLL] = fmod(m_vTrueRotation[ROLL], 360.0f);

        if (le_entitySnap && m_iOverMode != ENTITY_MODE_TREE)
            m_vRotation[ROLL] = m_vTrueRotation[ROLL] - fmod(m_vTrueRotation[ROLL], le_entityAngleSnap);
        else
            m_vRotation[ROLL] = m_vTrueRotation[ROLL];
    }
}


/*====================
  CEntityTool::ScaleUniform
  ====================*/
void    CEntityTool::ScaleUniform()
{
    CVec2f pos = Input.GetCursorPos();

    float dY = pos.y - m_vOldCursorPos.y;

    if (dY != 0.0f)
    {
        m_fTrueScale -= dY * le_entityScaleSensitivity;
        m_fTrueScale = CLAMP(m_fTrueScale, 0.01f, 1000.0f);

        if (le_entitySnap)
            m_fScale = m_fTrueScale - fmod(m_fTrueScale, le_entityScaleSnap);
        else
            m_fScale = m_fTrueScale;
    }
}


/*====================
  CEntityTool::Create
  ====================*/
void    CEntityTool::Create()
{
    if (m_uiHoverEnt == INVALID_INDEX)
    {
        if (!m_bValidPosition)
            return;

        if (!m_bModifier2)
        {
            m_setSelection.clear();
            EntitySelection.Trigger(XtoA(!m_setSelection.empty()));
        }

        float fX = m_v3EndPos.x;
        float fY = m_v3EndPos.y;

        if (le_entitySnap)
        {
            fX = fX - fmod(fX, Editor.GetWorld().GetScale());
            fY = fY - fmod(fY, Editor.GetWorld().GetScale());
        }

        uint uiIndex(CreateEntity(m_v3EndPos.x, m_v3EndPos.y));
        if (uiIndex != INVALID_INDEX && !m_bModifier1 && !m_bModifier3)
        {
            m_setSelection.insert(uiIndex);
            EntitySelection.Trigger(XtoA(!m_setSelection.empty()));
        }

        m_iState = STATE_HOVERING;
    }
    else
    {
        if (m_bModifier2)
        {
            if (m_setSelection.find(m_uiHoverEnt) == m_setSelection.end())
                m_setSelection.insert(m_uiHoverEnt);
            else
                m_setSelection.erase(m_uiHoverEnt);
        }
        else if (m_bModifier3)
        {
            m_setSelection.erase(m_uiHoverEnt);
        }
        else
        {
            m_setSelection.clear();
            m_setSelection.insert(m_uiHoverEnt);
        }

        EntitySelection.Trigger(XtoA(!m_setSelection.empty()));

        m_iState = STATE_HOVERING;
    }
}


/*====================
  CEntityTool::StartSelect
  ====================*/
void    CEntityTool::StartSelect()
{
    if (m_uiHoverEnt == INVALID_INDEX)
    {
        m_iState = STATE_SELECT;
        m_vStartCursorPos = Input.GetCursorPos();
    }
    else if (m_uiHoverEnt != INVALID_INDEX)
    {

        if (m_bModifier2)
        {
            if (m_setSelection.find(m_uiHoverEnt) == m_setSelection.end())
                m_setSelection.insert(m_uiHoverEnt);
            else
                m_setSelection.erase(m_uiHoverEnt);

            EntitySelection.Trigger(XtoA(!m_setSelection.empty()));
        }
        else if (m_bModifier3)
        {
            m_setSelection.erase(m_uiHoverEnt);
            EntitySelection.Trigger(XtoA(!m_setSelection.empty()));
        }
        else
        {
            m_setSelection.clear();
            m_setSelection.insert(m_uiHoverEnt);
            EntitySelection.Trigger(XtoA(!m_setSelection.empty()));
        }

        m_iState = STATE_HOVERING;
        m_vStartCursorPos = Input.GetCursorPos();
    }
}


/*====================
  CEntityTool::StartTranslateXY
  ====================*/
void    CEntityTool::StartTranslateXY()
{
    try
    {
        if (le_entitySelectionLock)
        {
            if (m_setSelection.empty())
                return;

            if (m_bModifier1 && (!m_bModifier2 && !m_bModifier3))
            {
                m_setOldSelection = m_setSelection;
                CloneSelection();
                m_bCloning = true;
            }
            else
            {
                m_bCloning = false;
            }

            m_iState = STATE_TRANSLATE_XY;

            if ((le_entityCenterMode == CENTER_HOVER || le_entityCenterMode == CENTER_INDIVIDUAL) && Editor.GetWorld().GetEntity(m_uiHoverEnt, true) != NULL)
                m_vTranslate = Editor.GetWorld().GetEntity(m_uiHoverEnt, true)->GetPosition();
            else
                m_vTranslate = SelectionCenter();

            m_bSnapCursor = true;

            m_vTranslate.z = Editor.GetWorld().GetTerrainHeight(m_vTranslate.x, m_vTranslate.y);

            m_vTrueTranslate = m_vTranslate;
            m_vStartCursorPos = Input.GetCursorPos();
            return;
        }

        if (m_uiHoverEnt == INVALID_INDEX)
        {
            m_iState = STATE_SELECT;
            m_vStartCursorPos = Input.GetCursorPos();
        }
        else if (m_uiHoverEnt != INVALID_INDEX)
        {
            if (m_bModifier2)
            {
                if (m_setSelection.find(m_uiHoverEnt) == m_setSelection.end())
                    m_setSelection.insert(m_uiHoverEnt);
                else
                    m_setSelection.erase(m_uiHoverEnt);

                EntitySelection.Trigger(XtoA(!m_setSelection.empty()));
            }
            else if (m_bModifier3)
            {
                m_setSelection.erase(m_uiHoverEnt);
                EntitySelection.Trigger(XtoA(!m_setSelection.empty()));
            }
            else
            {
                if (m_setSelection.find(m_uiHoverEnt) == m_setSelection.end())
                {
                    m_setSelection.clear();
                    m_setSelection.insert(m_uiHoverEnt);
                    EntitySelection.Trigger(XtoA(!m_setSelection.empty()));
                }

                if (m_bModifier1 && (!m_bModifier2 && !m_bModifier3))
                {
                    m_setOldSelection = m_setSelection;
                    CloneSelection();
                    m_bCloning = true;
                }
                else
                {
                    m_bCloning = false;
                }

                m_iState = STATE_TRANSLATE_XY;

                if (le_entityCenterMode == CENTER_HOVER || le_entityCenterMode == CENTER_INDIVIDUAL)
                    m_vTranslate = Editor.GetWorld().GetEntity(m_uiHoverEnt, true)->GetPosition();
                else
                    m_vTranslate = SelectionCenter();

                m_vTranslate.z = Editor.GetWorld().GetTerrainHeight(m_vTranslate.x, m_vTranslate.y);

                m_bSnapCursor = true;

                m_vTrueTranslate = m_vTranslate;
                m_vStartCursorPos = Input.GetCursorPos();
            }
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityTool::StartTranslateXY() - "), NO_THROW);
        m_vTranslate.Clear();
        m_vTrueTranslate.Clear();
    }
}


/*====================
  CEntityTool::StartTransform
  ====================*/
void    CEntityTool::StartTransform(int iState)
{
    if (le_entitySelectionLock)
    {
        if (m_setSelection.empty())
            return;

        if (m_bModifier1 && (!m_bModifier2 && !m_bModifier3))
        {
            m_setOldSelection = m_setSelection;
            CloneSelection();
            m_bCloning = true;
        }
        else
        {
            m_bCloning = false;
        }

        m_iState = iState;

        m_vTranslate = m_vTrueTranslate = CVec3f(0.0f, 0.0f, 0.0f);
        m_vRotation = m_vTrueRotation = CVec3f(0.0f, 0.0f, 0.0f);
        m_fScale = m_fTrueScale = 1.0f;

        m_vStartCursorPos = Input.GetCursorPos();

        return;
    }

    if (m_uiHoverEnt == INVALID_INDEX)
    {
        m_iState = STATE_SELECT;
        m_vStartCursorPos = Input.GetCursorPos();
    }
    else if (m_uiHoverEnt != INVALID_INDEX)
    {
        if (m_bModifier2)
        {
            if (m_setSelection.find(m_uiHoverEnt) == m_setSelection.end())
                m_setSelection.insert(m_uiHoverEnt);
            else
                m_setSelection.erase(m_uiHoverEnt);

            EntitySelection.Trigger(XtoA(!m_setSelection.empty()));
        }
        else if (m_bModifier3)
        {
            m_setSelection.erase(m_uiHoverEnt);
            EntitySelection.Trigger(XtoA(!m_setSelection.empty()));
        }
        else
        {
            if (m_setSelection.find(m_uiHoverEnt) == m_setSelection.end())
            {
                m_setSelection.clear();
                m_setSelection.insert(m_uiHoverEnt);
                EntitySelection.Trigger(XtoA(!m_setSelection.empty()));
            }

            if (m_bModifier1 && (!m_bModifier2 && !m_bModifier3))
            {
                m_setOldSelection = m_setSelection;
                CloneSelection();
                m_bCloning = true;
            }
            else
            {
                m_bCloning = false;
            }

            m_iState = iState;
        }

        m_vTranslate = m_vTrueTranslate = CVec3f(0.0f, 0.0f, 0.0f);
        m_vRotation = m_vTrueRotation = CVec3f(0.0f, 0.0f, 0.0f);
        m_fScale = m_fTrueScale = 1.0f;

        m_vStartCursorPos = Input.GetCursorPos();
    }
}


/*====================
  CEntityTool::ApplySelect
  ====================*/
void    CEntityTool::ApplySelect()
{
    if (!(m_bModifier2 || m_bModifier3))
    {
        m_setSelection.clear();
    }

    CRectf rect(MIN(m_vStartCursorPos.x, Input.GetCursorPos().x),
        MIN(m_vStartCursorPos.y, Input.GetCursorPos().y),
        MAX(m_vStartCursorPos.x, Input.GetCursorPos().x),
        MAX(m_vStartCursorPos.y, Input.GetCursorPos().y));

    WorldEntList &vEntities(Editor.GetWorld().GetEntityList());
    WorldEntList_cit cit(vEntities.begin()), citEnd(vEntities.end());
    for (; cit != citEnd; ++cit)
    {
        CWorldEntity *pWorldEnt(Editor.GetWorld().GetEntityByHandle(*cit));
        
        if (m_iOverMode == ENTITY_MODE_TREE && pWorldEnt && Editor.GetCamera().IsPointInScreenRect(pWorldEnt->GetPosition(), rect) && pWorldEnt->GetType() == _T("Prop_Tree"))
        {
            if (m_bModifier2)
                m_setSelection.insert(pWorldEnt->GetIndex());
            else if (m_bModifier3)
                m_setSelection.erase(pWorldEnt->GetIndex());
            else
                m_setSelection.insert(pWorldEnt->GetIndex());
        }
        else if (pWorldEnt && Editor.GetCamera().IsPointInScreenRect(pWorldEnt->GetPosition(), rect))
        {
            if (m_bModifier2)
                m_setSelection.insert(pWorldEnt->GetIndex());
            else if (m_bModifier3)
                m_setSelection.erase(pWorldEnt->GetIndex());
            else
                m_setSelection.insert(pWorldEnt->GetIndex());
        }
    }

    EntitySelection.Trigger(XtoA(!m_setSelection.empty()));
}


/*====================
  CEntityTool::UpdateHoverSelection
  ====================*/
void    CEntityTool::UpdateHoverSelection()
{
    m_setHoverSelection.clear();

    if (m_iState != STATE_SELECT)
        return;

    CRectf rect(MIN(m_vStartCursorPos.x, Input.GetCursorPos().x),
        MIN(m_vStartCursorPos.y, Input.GetCursorPos().y),
        MAX(m_vStartCursorPos.x, Input.GetCursorPos().x),
        MAX(m_vStartCursorPos.y, Input.GetCursorPos().y));

    WorldEntList &vEntities(Editor.GetWorld().GetEntityList());
    WorldEntList_cit cit(vEntities.begin()), citEnd(vEntities.end());
    for (; cit != citEnd; ++cit)
    {
        CWorldEntity *pEnt(Editor.GetWorld().GetEntityByHandle(*cit));
        if (m_iOverMode == ENTITY_MODE_TREE && pEnt && Editor.GetCamera().IsPointInScreenRect(pEnt->GetPosition(), rect) && pEnt->GetType() == _T("Prop_Tree"))
            m_setHoverSelection.insert(pEnt->GetIndex());
        else if (pEnt && Editor.GetCamera().IsPointInScreenRect(pEnt->GetPosition(), rect))
            m_setHoverSelection.insert(pEnt->GetIndex());
    }
}


/*====================
  CEntityTool::ApplyTranslateXY
  ====================*/
void    CEntityTool::ApplyTranslateXY()
{
    try
    {
        CVec3f v3Center;
        if (le_entityCenterMode == CENTER_AVERAGE || m_uiHoverEnt == INVALID_INDEX)
            v3Center = SelectionCenter();
        else
            v3Center = Editor.GetWorld().GetEntity(m_uiHoverEnt)->GetPosition();

        for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
        {
            CWorldEntity *pEntity(Editor.GetWorld().GetEntity(*it));
            if (pEntity == NULL)
                continue;

            CVec3f vDiff = pEntity->GetPosition() - v3Center;

            float fzOffset = v3Center.z - Editor.GetWorld().GetTerrainHeight(v3Center.x, v3Center.y);

            if (m_bModifier2)
                pEntity->SetPosition(m_vTranslate.x + vDiff.x, m_vTranslate.y + vDiff.y, pEntity->GetPosition().z);
            else
                pEntity->SetPosition(m_vTranslate.x + vDiff.x, m_vTranslate.y + vDiff.y, m_vTranslate.z + vDiff.z + fzOffset);

            Editor.GetWorld().UnlinkEntity(*it);
            Editor.GetWorld().LinkEntity(*it, LINK_SURFACE|LINK_MODEL, SURF_PROP);

            map<uint, CWorldEntityEx>::iterator findit(g_WorldEntData.find(pEntity->GetIndex()));

            if (findit != g_WorldEntData.end())
            {
                vector<PoolHandle> &vPathBlockers(findit->second.GetPathBlockers());

                vector<PoolHandle>::const_iterator citEnd(vPathBlockers.end());
                for (vector<PoolHandle>::const_iterator cit(vPathBlockers.begin()); cit != citEnd; ++cit)
                    Editor.GetWorld().ClearPath(*cit);

                vPathBlockers.clear();
            
                const vector<CConvexPolyhedron> &cWorldSurfs(pEntity->GetWorldSurfsRef());
                for (vector<CConvexPolyhedron>::const_iterator cit(cWorldSurfs.begin()); cit != cWorldSurfs.end(); ++cit)
                    Editor.GetWorld().BlockPath(vPathBlockers, NAVIGATION_UNIT, *cit, 24.0f);

                // Hack for tree blockers
                if (pEntity->GetType().compare(_T("Prop_Tree")) == 0)
                    vPathBlockers.push_back(Editor.GetWorld().BlockPath(NAVIGATION_UNIT, pEntity->GetPosition().xy() - CVec2f(50.0f), 100.0f, 100.0f));
            }
        }

        m_vTranslate.Clear();
        m_vTrueTranslate.Clear();
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityTool::ApplyTranslateXY() - "), NO_THROW);
        m_vTranslate.Clear();
        m_vTrueTranslate.Clear();
    }
}


/*====================
  CEntityTool::ApplyTranslateZ
  ====================*/
void    CEntityTool::ApplyTranslateZ()
{
    for (uiset::iterator it = m_setSelection.begin(); it != m_setSelection.end(); ++it)
    {
        CWorldEntity *pEntity(Editor.GetWorld().GetEntity(*it));
        if (pEntity == NULL)
            continue;

        CVec3f vOrigin = pEntity->GetPosition();
        vOrigin.z += m_vTranslate.z;
        pEntity->SetPosition(vOrigin);

        Editor.GetWorld().UnlinkEntity(*it);
        Editor.GetWorld().LinkEntity(*it, LINK_SURFACE|LINK_MODEL, SURF_PROP);
    }

    m_vTranslate.Clear();
    m_vTrueTranslate.Clear();
}


/*====================
  CEntityTool::ApplyRotation
  ====================*/
void    CEntityTool::ApplyRotation(EEulerComponent eDirection, const CVec3f &v3Center, bool bAdjustOrigin)
{
    for (uiset::iterator it = m_setSelection.begin(); it != m_setSelection.end(); ++it)
    {
        CWorldEntity *pEntity(Editor.GetWorld().GetEntity(*it));
        if (pEntity == NULL)
            continue;

        if (bAdjustOrigin)
        {
            CVec3f v3Diff(pEntity->GetPosition() - v3Center);
            v3Diff.xy().Rotate(m_vRotation[YAW]);
            pEntity->SetPosition(v3Center + v3Diff);
        }

        pEntity->AdjustAngle(eDirection, m_vRotation[eDirection]);

        Editor.GetWorld().UnlinkEntity(*it);
        Editor.GetWorld().LinkEntity(*it, LINK_SURFACE|LINK_MODEL, SURF_PROP);
    }
}


/*====================
  CEntityTool::ApplyRotation
  ====================*/
void    CEntityTool::ApplyRotation(EEulerComponent eDirection)
{
    try
    {
        if (le_entityCenterMode == CENTER_HOVER && m_uiHoverEnt != INVALID_INDEX)
            ApplyRotation(eDirection, Editor.GetWorld().GetEntity(m_uiHoverEnt, true)->GetPosition(), true);
        else if (le_entityCenterMode == CENTER_AVERAGE)
            ApplyRotation(eDirection, SelectionCenter(), true);
        else
            ApplyRotation(eDirection, V_ZERO, false);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityTool::ApplyRotatePitch() - "), NO_THROW);
    }

    m_vRotation.Clear();
}


/*====================
  CEntityTool::ApplyScaleUniform
  ====================*/
void    CEntityTool::ApplyScaleUniform(const CVec3f &v3Center, bool bAdjustOrigin)
{
    for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
    {
        CWorldEntity *pEntity(Editor.GetWorld().GetEntity(*it));
        if (pEntity == NULL)
            continue;

        if (bAdjustOrigin)
        {
            CVec3f v3Diff(pEntity->GetPosition() - v3Center);
            CVec3f v3Origin(v3Center + v3Diff * m_fScale);

            pEntity->SetPosition(v3Origin);
        }

        pEntity->AdjustScale(m_fScale);

        Editor.GetWorld().UnlinkEntity(*it);
        Editor.GetWorld().LinkEntity(*it, LINK_SURFACE|LINK_MODEL, SURF_PROP);
    }
}


/*====================
  CEntityTool::ApplyScaleUniform
  ====================*/
void    CEntityTool::ApplyScaleUniform()
{
    try
    {
        if (le_entityCenterMode == CENTER_HOVER && m_uiHoverEnt != INVALID_INDEX)
            ApplyScaleUniform(Editor.GetWorld().GetEntity(m_uiHoverEnt, true)->GetPosition(), true);
        else if (le_entityCenterMode == CENTER_AVERAGE)
            ApplyScaleUniform(SelectionCenter(), true);
        else
            ApplyScaleUniform(V_ZERO, false);

        m_fScale = 1.0f;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityTool::ApplyScaleUniform() - "), NO_THROW);
    }
}


/*====================
  CEntityTool::Frame
  ====================*/
void    CEntityTool::Frame(float fFrameTime)
{
    CalcToolProperties();
    switch (m_iOverMode)
    {
    case ENTITY_MODE_NORMAL:
        switch (m_iState)
        {
        case STATE_HOVERING:
            Hovering();
            break;
    
        case STATE_TRANSLATE_XY:
            TranslateXY();
            break;

        case STATE_TRANSLATE_Z:
            TranslateZ();
            break;

        case STATE_ROTATE_YAW:
            RotateYaw();
            break;
    
        case STATE_ROTATE_PITCH:
            RotatePitch();
            break;

        case STATE_ROTATE_ROLL:
            RotateRoll();
            break;
    
        case STATE_SCALE_UNIFORM:
            ScaleUniform();
            break;
        }
        break;
    case ENTITY_MODE_TREE:
        switch (m_iState)
        {
        case STATE_TRANSLATE_XY:
            TreeTranslateXY();
            break;

        case STATE_CREATE:
            if (m_bModifier2 && m_uiHoverEnt == INVALID_INDEX)
            TreeCreate();
            break;

        case STATE_TRANSLATE_Z:
            TranslateZ();
            break;

        case STATE_ROTATE_YAW:
            RotateYaw();
            break;
    
        case STATE_ROTATE_PITCH:
            RotatePitch();
            break;

        case STATE_ROTATE_ROLL:
            RotateRoll();
            break;

        }
    }

    m_vOldCursorPos = Input.GetCursorPos();

    UpdateHoverSelection();
}


/*====================
  CEntityTool::SnapCursor
  ====================*/
void    CEntityTool::SnapCursor(const CVec3f &vOrigin)
{
    CVec2f  pos;

    if (Editor.GetCamera().WorldToScreen(vOrigin, pos))
    {
        K2System.SetMousePos(INT_ROUND(pos.x), INT_ROUND(pos.y));
        Input.SetCursorPos(pos.x, pos.y);
        m_vOldCursorPos = pos;
    }
}


/*====================
  CEntityTool::CreateEntity
  ====================*/
uint    CEntityTool::CreateEntity(float fX, float fY)
{
    try
    {
        uint uiNewEntity(Editor.GetWorld().AllocateNewEntity());
        CWorldEntity *pNewEntity(Editor.GetWorld().GetEntity(uiNewEntity, true));

        pNewEntity->SetPosition(fX, fY, Editor.GetWorld().GetTerrainHeight(fX, fY));
        pNewEntity->SetSkin(g_ResourceManager.GetSkin(pNewEntity->GetModelHandle(), le_entitySkin));
        
        if (m_iOverMode == ENTITY_MODE_NORMAL)
        {
            pNewEntity->SetModelHandle(g_ResourceManager.Register(le_entityModel, RES_MODEL));
            pNewEntity->SetModelPath(le_entityModel);

            pNewEntity->SetType(le_entityType);

            IUnitDefinition *pDefinition(EntityRegistry.GetDefinition<IUnitDefinition>(le_entityType));
            if (pDefinition != NULL)
            {
                Editor.GetWorld().UnlinkEntity(pNewEntity->GetIndex());
                pNewEntity->SetScale(pDefinition->GetPreGlobalScale(0) * pDefinition->GetModelScale(0));
                pNewEntity->SetModelHandle(g_ResourceManager.Register(pDefinition->GetModelPath(0), RES_MODEL));
                pNewEntity->SetSkin(g_ResourceManager.GetSkin(pNewEntity->GetModelHandle(), pDefinition->GetSkin(0)));
                g_ResourceManager.PrecacheSkin(pNewEntity->GetModelHandle(), pNewEntity->GetSkin());
            }
        }
        else if (m_iOverMode == ENTITY_MODE_TREE)
        {
            pNewEntity->SetModelHandle(g_ResourceManager.Register(le_entityModelDelta, RES_MODEL));
            pNewEntity->SetModelPath(le_entityModelDelta);
        }

        Editor.GetWorld().LinkEntity(uiNewEntity, LINK_SURFACE | LINK_MODEL, SURF_PROP);
        return uiNewEntity;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityTool::CreateEntity() - "), NO_THROW);
        return INVALID_INDEX;
    }
}


/*====================
  CEntityTool::SelectionCenter
  ====================*/
CVec3f  CEntityTool::SelectionCenter()
{
    if (m_setSelection.empty())
        return V_ZERO;

    CVec3f v3Center(V_ZERO);
    for (uiset::iterator it = m_setSelection.begin(); it != m_setSelection.end(); ++it)
    {
        CWorldEntity *pEntity(Editor.GetWorld().GetEntity(*it));
        if (pEntity == NULL)
            continue;

        v3Center += pEntity->GetPosition();
    }

    v3Center /= float(m_setSelection.size());
    return v3Center;
}


/*====================
  CEntityTool::GetScale
  ====================*/
float   CEntityTool::GetScale(uint uiIndex)
{
    try
    {
        float fScale(Editor.GetWorld().GetEntity(uiIndex, true)->GetScale());
        if (m_setSelection.find(uiIndex) != m_setSelection.end())
            fScale *= m_fScale;

        return fScale;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityTool::GetScale() - "), NO_THROW);
        return 1.0f;
    }
}


/*====================
  CEntityTool::GetPosition
  ====================*/
CVec3f  CEntityTool::GetPosition(uint uiIndex)
{
    CVec3f v3Center;

    try
    {
        if (m_setSelection.find(uiIndex) == m_setSelection.end())
            return Editor.GetWorld().GetEntity(uiIndex, true)->GetPosition();

        CWorldEntity *pEntity(Editor.GetWorld().GetEntity(uiIndex, true));

        switch (le_entityCenterMode)
        {
        case CENTER_HOVER:
            if (m_uiHoverEnt != INVALID_INDEX)
                v3Center = Editor.GetWorld().GetEntity(m_uiHoverEnt, true)->GetPosition();
            else
                v3Center = pEntity->GetPosition();
            break;

        case CENTER_AVERAGE:
            v3Center = SelectionCenter();
            break;

        case CENTER_INDIVIDUAL:
            if (m_uiHoverEnt != INVALID_INDEX && m_iState == STATE_TRANSLATE_XY)
                v3Center = Editor.GetWorld().GetEntity(m_uiHoverEnt, true)->GetPosition();
            else
                v3Center = pEntity->GetPosition();
            break;
        }

        CVec3f v3Diff(pEntity->GetPosition() - v3Center);
        v3Diff.xy().Rotate(m_vRotation[YAW]);
        v3Diff *= m_fScale;

        CVec3f v3Origin(v3Diff);
        if (m_iState == STATE_TRANSLATE_XY)
        {
            if (!m_bModifier2)
            {
                v3Origin += m_vTranslate;
                float fzOffset = v3Center.z - Editor.GetWorld().GetTerrainHeight(v3Center.x, v3Center.y);
                v3Origin.z += fzOffset;
            }
            else
            {
                v3Origin.xy() += m_vTranslate.xy();
                v3Origin.z += v3Center.z;
            }
        }
        else
        {
            v3Origin += m_vTranslate;
            v3Origin += v3Center;
        }

        return v3Origin;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityTool::GetOrigin() - "), NO_THROW);
        return V_ZERO;
    }
}


/*====================
  CEntityTool::GetAngles
  ====================*/
CVec3f  CEntityTool::GetAngles(uint uiIndex)
{
    try
    {
        CVec3f v3Angles(Editor.GetWorld().GetEntity(uiIndex, true)->GetAngles());
        if (m_setSelection.find(uiIndex) != m_setSelection.end())
            v3Angles += m_vRotation;

        return  v3Angles;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityToll::GetAngles() - "), NO_THROW);
        return V_ZERO;
    }
}


/*====================
  CEntityTool::CloneSelection
  ====================*/
void    CEntityTool::CloneSelection()
{
    uiset setNew;

    for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
    {
        try
        {
            uint uiNewEntity(Editor.GetWorld().AllocateNewEntity());
            CWorldEntity *pNewEntity(Editor.GetWorld().GetEntity(uiNewEntity, true));

            uint uiOldEntity(*it);
            CWorldEntity *pOldEntity(Editor.GetWorld().GetEntity(uiOldEntity, true));

            pNewEntity->Clone(*pOldEntity);
            pNewEntity->SetIndex(uiNewEntity);

            Editor.GetWorld().LinkEntity(uiNewEntity, LINK_SURFACE|LINK_MODEL, SURF_PROP);

            map<uint, CWorldEntityEx>::iterator findit(g_WorldEntData.find(uiNewEntity));

            if (findit == g_WorldEntData.end())
            {
                g_WorldEntData.insert(pair<uint, CWorldEntityEx>(uiNewEntity, CWorldEntityEx()));
                findit = g_WorldEntData.find(uiNewEntity);
            }

            if (findit != g_WorldEntData.end())
            {
                vector<PoolHandle> &vPathBlockers(findit->second.GetPathBlockers());

                vector<PoolHandle>::const_iterator citEnd(vPathBlockers.end());
                for (vector<PoolHandle>::const_iterator cit(vPathBlockers.begin()); cit != citEnd; ++cit)
                    Editor.GetWorld().ClearPath(*cit);

                vPathBlockers.clear();
            
                const vector<CConvexPolyhedron> &cWorldSurfs(pNewEntity->GetWorldSurfsRef());
                for (vector<CConvexPolyhedron>::const_iterator cit(cWorldSurfs.begin()); cit != cWorldSurfs.end(); ++cit)
                    Editor.GetWorld().BlockPath(vPathBlockers, NAVIGATION_UNIT, *cit, 24.0f);

                // Hack for tree blockers
                if (pNewEntity->GetType().compare(_T("Prop_Tree")) == 0)
                    vPathBlockers.push_back(Editor.GetWorld().BlockPath(NAVIGATION_UNIT, pNewEntity->GetPosition().xy() - CVec2f(50.0f), 100.0f, 100.0f));
            }

            setNew.insert(uiNewEntity);

            if (uiOldEntity == m_uiHoverEnt)
                m_uiHoverEnt = uiNewEntity;
        }
        catch (CException &ex)
        {
            ex.Process(_T("CEntityTool::CloneSelection() - "), NO_THROW);
        }
    }

    m_setSelection = setNew;

    EntitySelection.Trigger(XtoA(!m_setSelection.empty()));
}


/*====================
  CEntityTool::GetSelectionRect
  ====================*/
CRectf  CEntityTool::GetSelectionRect()
{
    return CRectf(MIN(m_vStartCursorPos.x, Input.GetCursorPos().x),
                MIN(m_vStartCursorPos.y, Input.GetCursorPos().y),
                MAX(m_vStartCursorPos.x, Input.GetCursorPos().x),
                MAX(m_vStartCursorPos.y, Input.GetCursorPos().y));
}


/*====================
  CEntityTool::Draw
  ====================*/
void    CEntityTool::Draw()
{
    if (le_entityDrawBrushCoords)
    {
        CFontMap *pFontMap(g_ResourceManager.GetFontMap(m_hFont));

        if (m_uiHoverEnt != INVALID_INDEX && Editor.GetWorld().GetEntity(m_uiHoverEnt) != NULL)
        {
            CWorldEntity *pWorldEnt(Editor.GetWorld().GetEntity(m_uiHoverEnt));

            Draw2D.SetColor(0.0f, 0.0f, 0.0f);
            Draw2D.String(4.0f, Draw2D.GetScreenH() - pFontMap->GetMaxHeight() - 1.0f, Draw2D.GetScreenW(), pFontMap->GetMaxHeight(), ParenStr(XtoA(pWorldEnt->GetPosition())), m_hFont);
            Draw2D.SetColor(1.0f, 1.0f, 1.0f);
            Draw2D.String(3.0f, Draw2D.GetScreenH() - pFontMap->GetMaxHeight() - 2.0f, Draw2D.GetScreenW(), pFontMap->GetMaxHeight(), ParenStr(XtoA(pWorldEnt->GetPosition())), m_hFont);
        }
        else
        {
            Draw2D.SetColor(0.0f, 0.0f, 0.0f);
            Draw2D.String(4.0f, Draw2D.GetScreenH() - pFontMap->GetMaxHeight() - 1.0f, Draw2D.GetScreenW(), pFontMap->GetMaxHeight(), XtoA(m_v3EndPos), m_hFont);
            Draw2D.SetColor(1.0f, 1.0f, 1.0f);
            Draw2D.String(3.0f, Draw2D.GetScreenH() - pFontMap->GetMaxHeight() - 2.0f, Draw2D.GetScreenW(), pFontMap->GetMaxHeight(), XtoA(m_v3EndPos), m_hFont);
        }
    }

    if (IsSelectionActive())
    {
        CVec4f  v4Border(1.0f, 1.0f, 1.0f, 1.0f);
        CVec4f  v4Fill(0.3f, 0.7f, 1.0f, 0.2f);

        CRectf rec(GetSelectionRect());

        Draw2D.SetColor(v4Fill);
        Draw2D.Rect(rec.left, rec.top, rec.GetWidth(), rec.GetHeight());

        Draw2D.RectOutline(rec, 1.0f, v4Border);
    }

}


/*====================
  CEntityTool::GroundSelection
  ====================*/
void    CEntityTool::GroundSelection()
{
    for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
    {
        CWorldEntity *pEntity(Editor.GetWorld().GetEntity(*it));
        if (pEntity == NULL)
            continue;

        Editor.GetWorld().UnlinkEntity(*it);
        CVec3f v3Origin(pEntity->GetPosition());
        v3Origin.z = Editor.GetWorld().GetTerrainHeight(v3Origin.x, v3Origin.y);
        pEntity->SetPosition(v3Origin);
        Editor.GetWorld().LinkEntity(*it, LINK_SURFACE|LINK_MODEL, SURF_PROP);
    }
}


/*====================
  CEntityTool::StraightenSelection
  ====================*/
void    CEntityTool::StraightenSelection()
{
    for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
    {
        CWorldEntity *pEntity(Editor.GetWorld().GetEntity(*it));
        if (pEntity == NULL)
            continue;

        Editor.GetWorld().UnlinkEntity(*it);
        pEntity->SetAngles(V_ZERO);
        Editor.GetWorld().LinkEntity(*it, LINK_SURFACE|LINK_MODEL, SURF_PROP);
    }
}


/*====================
  CEntityTool::ResetScaleSelection
  ====================*/
void    CEntityTool::ResetScaleSelection()
{
    for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
    {
        CWorldEntity *pEntity(Editor.GetWorld().GetEntity(*it));
        if (pEntity == NULL)
            continue;

        Editor.GetWorld().UnlinkEntity(*it);
        pEntity->SetScale(1.0f);
        Editor.GetWorld().LinkEntity(*it, LINK_SURFACE|LINK_MODEL, SURF_PROP);
    }
}


/*====================
  CEntityTool::IsSelectionLock
  ====================*/
bool    CEntityTool::IsSelectionLock()
{
    return le_entitySelectionLock;
}


/*====================
  CEntityTool::GetSelectionName
  ====================*/
tstring CEntityTool::GetSelectionName()
{
    if (m_setSelection.empty())
        return _T("");

    tstring sName(Editor.GetWorld().GetEntity(*m_setSelection.begin())->GetName());

    for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
    {
        CWorldEntity *pEntity(Editor.GetWorld().GetEntity(*it));
        if (pEntity == NULL)
            continue;

        if (pEntity->GetName() != sName)
            return _T("");
    }

    return sName;
}


/*====================
  CEntityTool::GetSelectionTeam
  ====================*/
int     CEntityTool::GetSelectionTeam()
{
    if (m_setSelection.empty())
        return le_entityTeam;
    
    int iTeam(Editor.GetWorld().GetEntity(*m_setSelection.begin())->GetTeam());
    for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
    {
        CWorldEntity *pEntity(Editor.GetWorld().GetEntity(*it));
        if (pEntity == NULL)
            continue;

        if (pEntity->GetTeam() != iTeam)
            return -1;
    }

    return iTeam;
}


/*====================
  CEntityTool::GetSelectionModel
  ====================*/
tstring CEntityTool::GetSelectionModel()
{
    if (m_iOverMode == ENTITY_MODE_NORMAL)
    {
        if (m_setSelection.empty())
            return le_entityModel;
    }
    else if (m_iOverMode == ENTITY_MODE_TREE)
    {
        if (m_setSelection.empty())
            return le_entityModelDelta;
    }

    tstring sModel;
//  if (m_iOverMode == ENTITY_MODE_NORMAL)
    sModel = g_ResourceManager.GetPath(Editor.GetWorld().GetEntity(*m_setSelection.begin())->GetModelHandle());
//  else if (m_iOverMode == ENTITY_MODE_TREE)
//  sModel = le_entityModelDelta;

    
    for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
    {
        CWorldEntity *pEntity(Editor.GetWorld().GetEntity(*it));
        if (pEntity == NULL)
            continue;

        if (g_ResourceManager.GetPath(pEntity->GetModelHandle()) != sModel)
            return _T("");
    }

    return sModel;
}


/*====================
  CEntityTool::GetSelectionSkin
  ====================*/
tstring CEntityTool::GetSelectionSkin()
{
    if (m_setSelection.empty())
        return le_entitySkin;

    CWorldEntity *pEntity(Editor.GetWorld().GetEntity(*m_setSelection.begin()));
    if (pEntity == NULL)
        return TSNULL;

    CModel *pModel(g_ResourceManager.GetModel(pEntity->GetModelHandle()));
    if (pModel == NULL)
        return TSNULL;

    IModel *pIModel(pModel->GetModelFile());
    if (pIModel == NULL)
        return TSNULL;

    tstring sSkin(pIModel->GetSkin(pEntity->GetSkin())->GetName());
    
    for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
    {
        CWorldEntity *pEntity(Editor.GetWorld().GetEntity(*it));
        if (pEntity == NULL)
            continue;

        CModel *pModel(g_ResourceManager.GetModel(pEntity->GetModelHandle()));
        if (pModel == NULL)
            continue;

        IModel *pIModel(pModel->GetModelFile());
        if (pIModel == NULL)
            continue;
        
        if (pIModel->GetSkin(pEntity->GetSkin())->GetName() != sSkin)
            return TSNULL;
    }

    return sSkin;
}


/*====================
  CEntityTool::GetSelectionType
  ====================*/
tstring CEntityTool::GetSelectionType()
{
    if (m_setSelection.empty())
        return le_entityType;

    tstring sType(Editor.GetWorld().GetEntity(*m_setSelection.begin())->GetType());
    
    for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
    {
        CWorldEntity *pEntity(Editor.GetWorld().GetEntity(*it));
        if (pEntity == NULL)
            continue;

        if (pEntity->GetType() != sType)
            return _T("");
    }

    return sType;
}


/*====================
  CEntityTool::IsSelectionType
  ====================*/
bool    CEntityTool::IsSelectionType(const tstring &sType)
{
    if (m_setSelection.empty())
        return false;

    uint uiType0(ENTITY_BASE_TYPE0_NULL);
    uint uiType1(ENTITY_BASE_TYPE1_NULL);
    uint uiType2(ENTITY_BASE_TYPE2_NULL);
    uint uiType3(ENTITY_BASE_TYPE3_NULL);
    uint uiType4(ENTITY_BASE_TYPE4_NULL);

    tstring sLowerType(LowerString(sType));

    if (TStringCompare(sLowerType, _T("unit")) == 0)
    {
        uiType1 = ENTITY_BASE_TYPE1_UNIT;
    }
    else if (TStringCompare(sLowerType, _T("affector")) == 0)
    {
        uiType1 = ENTITY_BASE_TYPE1_AFFECTOR;
    }
    
    for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
    {
        CWorldEntity *pEntity(Editor.GetWorld().GetEntity(*it));
        if (pEntity == NULL)
            continue;

        uint uiBaseType(EntityRegistry.GetBaseType(pEntity->GetType()));

        if (uiType0 != ENTITY_BASE_TYPE0_NULL && GET_ENTITY_BASE_TYPE0(uiBaseType) != uiType0)
            return false;
        if (uiType1 != ENTITY_BASE_TYPE1_NULL && GET_ENTITY_BASE_TYPE1(uiBaseType) != uiType1)
            return false;
        if (uiType2 != ENTITY_BASE_TYPE2_NULL && GET_ENTITY_BASE_TYPE2(uiBaseType) != uiType2)
            return false;
        if (uiType3 != ENTITY_BASE_TYPE3_NULL && GET_ENTITY_BASE_TYPE3(uiBaseType) != uiType3)
            return false;
        if (uiType4 != ENTITY_BASE_TYPE4_NULL && GET_ENTITY_BASE_TYPE4(uiBaseType) != uiType4)
            return false;
    }

    return true;
}


/*====================
  CEntityTool::GetSelectionProperty
  ====================*/
tstring CEntityTool::GetSelectionProperty(const tstring &sName)
{
    if (m_setSelection.empty())
        return _T("");

    tstring sValue(Editor.GetWorld().GetEntity(*m_setSelection.begin())->GetProperty(sName));

    for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
    {
        CWorldEntity *pEntity(Editor.GetWorld().GetEntity(*it));
        if (pEntity == NULL)
            continue;

        if (pEntity->GetProperty(sName) != sValue)
            return _T("");
    }

    NormalizeLineBreaks(sValue);

    return sValue;
}


/*====================
  CEntityTool::GetWorldIndexFromName
  ====================*/
int     CEntityTool::GetWorldIndexFromName(const tstring &sName)
{
    WorldEntList &vEntities(Editor.GetWorld().GetEntityList());
    for (WorldEntList_it it(vEntities.begin()), itEnd(vEntities.end()); it != itEnd; ++it)
    {
        CWorldEntity *pEntity(Editor.GetWorld().GetEntityByHandle(*it));

        if (pEntity != NULL && pEntity->GetName() == sName)
            return pEntity->GetIndex();
    }

    return -1;
}

/*====================
  CEntityTool::SetSelectionName
  ====================*/
void    CEntityTool::SetSelectionName(const tstring &sName)
{
    for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
    {
        CWorldEntity *pEntity(Editor.GetWorld().GetEntity(*it));
        if (pEntity == NULL)
            continue;

        pEntity->SetName(sName);
    }
}


/*====================
  CEntityTool::SetSelectionTeam
  ====================*/
void    CEntityTool::SetSelectionTeam(int iTeam)
{
    for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
    {
        CWorldEntity *pEntity(Editor.GetWorld().GetEntity(*it));
        if (pEntity == NULL)
            continue;

        pEntity->SetTeam(iTeam);
    }
}


/*====================
  CEntityTool::SetSelectionModel
  ====================*/
void    CEntityTool::SetSelectionModel(const tstring &sModel)
{
    ResHandle hModel(g_ResourceManager.Register(sModel, RES_MODEL));
    if (hModel == INVALID_RESOURCE)
        return;

    for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
    {
        CWorldEntity *pEntity(Editor.GetWorld().GetEntity(*it));
        if (pEntity == NULL)
            continue;

        CModel *pModel(g_ResourceManager.GetModel(pEntity->GetModelHandle()));
        if (pModel == NULL)
            continue;

        IModel *pIModel(pModel->GetModelFile());
        if (pIModel == NULL)
            continue;

        tstring sSkin(pIModel->GetSkin(pEntity->GetSkin())->GetName());

        Editor.GetWorld().UnlinkEntity(pEntity->GetIndex());
        pEntity->SetModelHandle(hModel);
        pEntity->SetModelPath(sModel);
        Editor.GetWorld().LinkEntity(pEntity->GetIndex(), LINK_SURFACE|LINK_MODEL, SURF_PROP);

        pEntity->SetSkin(g_ResourceManager.GetSkin(pEntity->GetModelHandle(), sSkin));
    }

    EntitySelectionModel.Trigger(TSNULL);
}


/*====================
  CEntityTool::SetSelectionSkin
  ====================*/
void    CEntityTool::SetSelectionSkin(const tstring &sSkin)
{
    for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
    {
        CWorldEntity *pEntity(Editor.GetWorld().GetEntity(*it));
        if (pEntity == NULL)
            continue;

        pEntity->SetSkin(g_ResourceManager.GetSkin(pEntity->GetModelHandle(), sSkin));
    }

    EntitySelectionSkin.Trigger(TSNULL);
}


/*====================
  CEntityTool::SetSelectionType
  ====================*/
void    CEntityTool::SetSelectionType(const tstring &sType)
{
    if (sType.empty())
        return;

    for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
    {
        CWorldEntity *pEntity(Editor.GetWorld().GetEntity(*it));
        if (pEntity == NULL)
            continue;

        pEntity->SetType(sType);

        IUnitDefinition *pDefinition(EntityRegistry.GetDefinition<IUnitDefinition>(sType));
        if (pDefinition != NULL)
        {
            Editor.GetWorld().UnlinkEntity(pEntity->GetIndex());
            pEntity->SetScale(pDefinition->GetPreGlobalScale(0) * pDefinition->GetModelScale(0));
            pEntity->SetModelHandle(g_ResourceManager.Register(pDefinition->GetModelPath(0), RES_MODEL));
            pEntity->SetSkin(g_ResourceManager.GetSkin(pEntity->GetModelHandle(), pDefinition->GetSkin(0)));
            g_ResourceManager.PrecacheSkin(pEntity->GetModelHandle(), pEntity->GetSkin());
            Editor.GetWorld().LinkEntity(pEntity->GetIndex(), LINK_SURFACE|LINK_MODEL, SURF_PROP);
        }
    }
}


/*====================
  CEntityTool::SetSelectionProperty
  ====================*/
void    CEntityTool::SetSelectionProperty(const tstring &sName, const tstring &sValue)
{
    for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
    {
        CWorldEntity *pEntity(Editor.GetWorld().GetEntity(*it));
        if (pEntity == NULL)
            continue;

        pEntity->SetProperty(sName, NormalizeLineBreaks(sValue, _T("\n")));
    }
}


/*====================
  CEntityTool::SelectByModel
  ====================*/
void    CEntityTool::SelectByModel(const tstring &sModel)
{
    m_setSelection.clear();

    WorldEntList &vEntities(Editor.GetWorld().GetEntityList());
    for (WorldEntList_it it(vEntities.begin()), itEnd(vEntities.end()); it != itEnd; ++it)
    {
        CWorldEntity *pEntity(Editor.GetWorld().GetEntityByHandle(*it));

        if (pEntity != NULL && g_ResourceManager.GetPath(pEntity->GetModelHandle()) == sModel)
            m_setSelection.insert(pEntity->GetIndex());
    }
}


/*====================
  CEntityTool::SetEditMode
  ====================*/
void    CEntityTool::SetEditMode(const tstring &sValue)
{
    if (sValue == _T("create"))
    {
        le_entityEditMode = ENTITY_CREATE;
        EntityEditMode.Trigger(_T("Create"));
    }
    else if (sValue == _T("select"))
    {
        le_entityEditMode = ENTITY_SELECT;
        EntityEditMode.Trigger(_T("Select"));
    }
    else if (sValue == _T("translate") || sValue == _T("translate_xy"))
    {
        le_entityEditMode = ENTITY_TRANSLATE;
        EntityEditMode.Trigger(_T("Translate XY"));
    }
    else if (sValue == _T("translate_z"))
    {
        le_entityEditMode = ENTITY_TRANSLATE_Z;
        EntityEditMode.Trigger(_T("Translate Z"));
    }
    else if (sValue == _T("rotate") || sValue == _T("rotate_yaw"))
    {
        le_entityEditMode = ENTITY_ROTATE_YAW;
        EntityEditMode.Trigger(_T("Rotate Yaw"));
    }
    else if (sValue == _T("rotate_pitch"))
    {
        le_entityEditMode = ENTITY_ROTATE_PITCH;
        EntityEditMode.Trigger(_T("Rotate Pitch"));
    }
    else if (sValue == _T("rotate_roll"))
    {
        le_entityEditMode = ENTITY_ROTATE_ROLL;
        EntityEditMode.Trigger(_T("Rotate Roll"));
    }
    else if (sValue == _T("scale"))
    {
        le_entityEditMode = ENTITY_SCALE;
        EntityEditMode.Trigger(_T("Scale"));
    }
}


/*--------------------
  cmdEntityEditMode
  --------------------*/
UI_VOID_CMD(SetEntityEditMode, 1)
{
    try
    {
        if (vArgList.size() < 1)
        {
            Console << _T("syntax: entityeditmode create|select|translate|rotate|scale") << newl;
            return;
        }

        GET_TOOL(Entity, ENTITY)->SetEditMode(vArgList[0]->Evaluate());
    }
    catch (CException &ex)
    {
        ex.Process(_T("cmdSetEntityEditMode() - "), NO_THROW);
    }

    return;
}


/*--------------------
  cmdEntityCenterMode
  --------------------*/
UI_VOID_CMD(SetEntityCenterMode, 1)
{
    if (vArgList.size() < 1)
    {
        Console << _T("syntax: entitycentermode average|hover|individual") << newl;
        return;
    }

    tstring sValue(vArgList[0]->Evaluate());

    if (sValue == _T("average"))
    {
        le_entityCenterMode = CENTER_AVERAGE;
        return;
    }
    else if (sValue == _T("hover"))
    {
        le_entityCenterMode = CENTER_HOVER;
        return;
    }
    else if (sValue == _T("individual"))
    {
        le_entityCenterMode = CENTER_INDIVIDUAL;
        return;
    }
}


/*--------------------
  cmdEntityGround
  --------------------*/
UI_VOID_CMD(EntityGround, 0)
{
    try
    {
        GET_TOOL(Entity, ENTITY)->GroundSelection();
    }
    catch (CException &ex)
    {
        ex.Process(_T("cmdEntityStraighten() - "), NO_THROW);
    }
}


/*--------------------
  cmdEntityStraighten
  --------------------*/
UI_VOID_CMD(EntityStraighten, 0)
{
    try
    {
        GET_TOOL(Entity, ENTITY)->StraightenSelection();
    }
    catch (CException &ex)
    {
        ex.Process(_T("cmdEntityStraighten() - "), NO_THROW);
    }
}


/*--------------------
  cmdEntityResetScale
  --------------------*/
UI_VOID_CMD(EntityResetScale, 0)
{
    try
    {
        GET_TOOL(Entity, ENTITY)->ResetScaleSelection();
    }
    catch (CException &ex)
    {
        ex.Process(_T("cmdEntityStraighten() - "), NO_THROW);
    }
}


/*--------------------
  GetSelectionName
  --------------------*/
UI_CMD(GetSelectionName, 0)
{
    tstring sSelectionName;

    try
    {
        sSelectionName = GET_TOOL(Entity, ENTITY)->GetSelectionName();
    }
    catch (CException &ex)
    {
        ex.Process(_T("fnGetSelectionName() - "), NO_THROW);
    }

    return sSelectionName;
}


/*--------------------
  GetSelectionTeam
  --------------------*/
UI_CMD(GetSelectionTeam, 0)
{
    int iTeam(-1);

    try
    {
        iTeam = GET_TOOL(Entity, ENTITY)->GetSelectionTeam();
    }
    catch (CException &ex)
    {
        ex.Process(_T("GetSelectionTeam() - "), NO_THROW);
    }

    return XtoA(iTeam);
}


/*--------------------
  GetSelectionModel
  --------------------*/
UI_CMD(GetSelectionModel, 0)
{
    tstring sSelectionModel;

    try
    {
        sSelectionModel = GET_TOOL(Entity, ENTITY)->GetSelectionModel();
    }
    catch (CException &ex)
    {
        ex.Process(_T("fnGetSelectionModel() - "), NO_THROW);
    }

    return sSelectionModel;
}

/*--------------------
  GetTreePath
  --------------------*/
UI_CMD(GetTreePath, 0)
{
    tstring sTreePath = GET_TOOL(Entity, ENTITY)->GetSelectionTreeDef();
    return sTreePath;
}



/*--------------------
  GetSelectionSkin
  --------------------*/
UI_CMD(GetSelectionSkin, 0)
{
    tstring sSelectionSkin;

    try
    {
        sSelectionSkin = GET_TOOL(Entity, ENTITY)->GetSelectionSkin();
    }
    catch (CException &ex)
    {
        ex.Process(_T("fnGetSelectionModel() - "), NO_THROW);
    }

    return sSelectionSkin;
}


/*--------------------
  GetSelectionType
  --------------------*/
UI_CMD(GetSelectionType, 0)
{
    tstring sSelectionType;

    try
    {
        sSelectionType = GET_TOOL(Entity, ENTITY)->GetSelectionType();
    }
    catch (CException &ex)
    {
        ex.Process(_T("fnGetSelectionType() - "), NO_THROW);
    }

    return sSelectionType;
}


/*--------------------
  IsSelectionType
  --------------------*/
UI_CMD(IsSelectionType, 1)
{
    bool bIsSelectionType(false);

    try
    {
        bIsSelectionType = GET_TOOL(Entity, ENTITY)->IsSelectionType(vArgList[0]->Evaluate());
    }
    catch (CException &ex)
    {
        ex.Process(_T("IsSelectionType() - "), NO_THROW);
    }

    return XtoA(bIsSelectionType);
}


/*--------------------
  GetSelectionProperty
  --------------------*/
UI_CMD(GetSelectionProperty, 1)
{
    tstring sSelectionProperty;

    try
    {
        if (vArgList.size() < 1)
            EX_MESSAGE(_T("syntax: GetSelectionProperty <key>"));

        sSelectionProperty = GET_TOOL(Entity, ENTITY)->GetSelectionProperty(vArgList[0]->Evaluate());
    }
    catch (CException &ex)
    {
        ex.Process(_T("fnGetSelectionName() - "), NO_THROW);
    }

    return sSelectionProperty;
}


/*--------------------
  GetWorldIndexFromName
  --------------------*/
UI_CMD(GetWorldIndexFromName, 0)
{
    tstring sIndex(_T("-1"));

    try
    {
        if (vArgList.size() < 1)
            EX_MESSAGE(_T("syntax: GetWorldIndexFromName <key>"));

        sIndex = XtoA(GET_TOOL(Entity, ENTITY)->GetWorldIndexFromName(vArgList[0]->Evaluate()));
    }
    catch (CException &ex)
    {
        ex.Process(_T("fnGetSelectionName() - "), NO_THROW);
    }

    return sIndex;
}

/*--------------------
  SetSelectionName
  --------------------*/
UI_VOID_CMD(SetSelectionName, 1)
{
    try
    {
        if (vArgList.size() < 1)
            EX_MESSAGE(_T("syntax: SetSelectionName <name>"));

        GET_TOOL(Entity, ENTITY)->SetSelectionName(vArgList[0]->Evaluate());
    }
    catch (CException &ex)
    {
        ex.Process(_T("cmdSetSelectionName() - "), NO_THROW);
    }
}


/*--------------------
  SetSelectionTeam
  --------------------*/
UI_VOID_CMD(SetSelectionTeam, 1)
{
    try
    {
        if (vArgList.size() < 1)
            EX_MESSAGE(_T("syntax: SetSelectionTeam <team>"));

        GET_TOOL(Entity, ENTITY)->SetSelectionTeam(AtoI(vArgList[0]->Evaluate()));
    }
    catch (CException &ex)
    {
        ex.Process(_T("cmdSetSelectionTeam() - "), NO_THROW);
    }
}


/*--------------------
  SetSelectionModel
  --------------------*/
UI_VOID_CMD(SetSelectionModel, 1)
{
    try
    {
        if (vArgList.size() < 1)
            EX_MESSAGE(_T("syntax: SetSelectionModel <model>"));

        GET_TOOL(Entity, ENTITY)->SetSelectionModel(vArgList[0]->Evaluate());
    }
    catch (CException &ex)
    {
        ex.Process(_T("cmdSetSelectionModel() - "), NO_THROW);
    }
}


/*--------------------
  SetSelectionSkin
  --------------------*/
UI_VOID_CMD(SetSelectionSkin, 1)
{
    try
    {
        if (vArgList.size() < 1)
            EX_MESSAGE(_T("syntax: SetSelectionSkin <skin>"));

        GET_TOOL(Entity, ENTITY)->SetSelectionSkin(vArgList[0]->Evaluate());
    }
    catch (CException &ex)
    {
        ex.Process(_T("cmdSetSelectionSkin() - "), NO_THROW);
    }
}


/*--------------------
  SetSelectionType
  --------------------*/
UI_VOID_CMD(SetSelectionType, 1)
{
    try
    {
        if (vArgList.size() < 1)
            EX_MESSAGE(_T("syntax: SetSelectionType <type name>"));

        GET_TOOL(Entity, ENTITY)->SetSelectionType(vArgList[0]->Evaluate());
    }
    catch (CException &ex)
    {
        ex.Process(_T("cmdSetSelectionType() - "), NO_THROW);
    }
}


/*--------------------
  SetSelectionProperty
  --------------------*/
UI_VOID_CMD(SetSelectionProperty, 2)
{
    try
    {
        if (vArgList.size() < 2)
            EX_MESSAGE(_T("syntax: SetSelectionProperty <name> <value>"));

        GET_TOOL(Entity, ENTITY)->SetSelectionProperty(vArgList[0]->Evaluate(), vArgList[1]->Evaluate());
    }
    catch (CException &ex)
    {
        ex.Process(_T("SetSelectionProperty() - "), NO_THROW);
    }
}


/*--------------------
  SelectByModel
  --------------------*/
UI_VOID_CMD(SelectByModel, 1)
{
    try
    {
        if (vArgList.size() < 1)
            EX_MESSAGE(_T("syntax: SelectByModel <value>"));

        GET_TOOL(Entity, ENTITY)->SelectByModel(vArgList[0]->Evaluate());
    }
    catch (CException &ex)
    {
        ex.Process(_T("SelectByModel() - "), NO_THROW);
    }
}


/*--------------------
  ClearSelection
  --------------------*/
UI_VOID_CMD(ClearSelection, 0)
{
    GET_TOOL(Entity, ENTITY)->ClearSelection();
}


/*--------------------
  ToggleEntityTreeMode
  --------------------*/
UI_VOID_CMD(ToggleEntityTreeMode, 0)
{
    GET_TOOL(Entity, ENTITY)->ToggleTreeMode();
}


/*--------------------
  EnableTreeMode
  --------------------*/
UI_VOID_CMD(EnableTreeMode, 0)
{
    GET_TOOL(Entity, ENTITY)->EnableTreeMode();
}


/*--------------------
  DisableTreeMode
  --------------------*/
UI_VOID_CMD(DisableTreeMode, 0)
{
    GET_TOOL(Entity, ENTITY)->DisableTreeMode();
}


/*====================
  CEntityTool::TreeCreate
  ====================*/
void    CEntityTool::TreeCreate()
{
    if (m_uiHoverEnt == INVALID_INDEX)
    {
        if (!m_bValidPosition)
            return;

        if (!m_bModifier2)
        {
            m_setSelection.clear();
            EntitySelection.Trigger(XtoA(!m_setSelection.empty()));
        }

        float fX = m_v3EndPos.x;
        float fY = m_v3EndPos.y;

        if (le_entitySnap)
        {
            fX = fX - fmod(fX, Editor.GetWorld().GetScale());
            fY = fY - fmod(fY, Editor.GetWorld().GetScale());
        }

        //snap positioning 
        float creationX(m_v3EndPos.x);
        float creationY(m_v3EndPos.y);
        if (le_entitySnap)
        {
            if (le_entitySnapAbsolute)
            {
                creationX = ROUND(m_v3EndPos.x / le_entityPositionSnap) * le_entityPositionSnap;
                creationY = ROUND(m_v3EndPos.y / le_entityPositionSnap) * le_entityPositionSnap;
            }
        }

        uivector result;
        CBBoxf box = CBBoxf(-32.0f, 32.0f);
        box.Offset(CVec3f(creationX,creationY,0));
        Editor.GetWorld().GetEntitiesInRegion(result, box, 0);

        for (uivector::iterator it = result.begin(); it < result.end(); ++it)
        {
            if (Editor.GetWorld().GetEntity(*it)->GetType().compare(_T("Prop_Tree")) == 0)
                return;
        }
        
        ResHandle TreeDefinitionHandle = g_ResourceManager.Register(le_entityTreeDefinition, RES_TREE);
        CTreeDefinitionResource * TreeDefinition = static_cast<CTreeDefinitionResource*>(g_ResourceManager.Get(TreeDefinitionHandle));
        le_entityModelDelta = TreeDefinition->GetTreeModelPath();

        uint uiIndex(CreateEntity(creationX, creationY));
        if (uiIndex != INVALID_INDEX && !m_bModifier1 && !m_bModifier3)
        {
            m_setSelection.insert(uiIndex);
            EntitySelection.Trigger(XtoA(!m_setSelection.empty()));
        }
        Editor.GetWorld().GetEntity(uiIndex)->SetType(_T("Prop_Tree"));

        Editor.GetWorld().GetEntity(uiIndex)->SetTreeDefinition(TreeDefinitionHandle);

        float tmin = TreeDefinition->GetTreeScaleMin();
        float tmax = TreeDefinition->GetTreeScaleMax();

        Editor.GetWorld().GetEntity(uiIndex)->AdjustAngle(YAW, M_Randnum(1.0f, 360.0f));
        Editor.GetWorld().GetEntity(uiIndex)->SetScale(M_Randnum(tmin,tmax));
        

        Editor.GetWorld().UnlinkEntity(uiIndex);
        Editor.GetWorld().LinkEntity(uiIndex, LINK_SURFACE|LINK_MODEL, SURF_PROP);
        m_iState = STATE_CREATE;
    }
    else
    {
        if (m_bModifier2)
        {                           
            if (Editor.GetWorld().GetEntity(m_uiHoverEnt)->GetType().compare(_T("Prop_Tree")) == 0)         
            {
                if (m_setSelection.find(m_uiHoverEnt) == m_setSelection.end())
                    m_setSelection.insert(m_uiHoverEnt);
                else
                    m_setSelection.erase(m_uiHoverEnt);
            }   
        }
        else if (m_bModifier3)
        {
            m_setSelection.erase(m_uiHoverEnt);
        }
        else
        {
            if (Editor.GetWorld().GetEntity(m_uiHoverEnt)->GetType().compare(_T("Prop_Tree")) == 0)         
            {
            m_setSelection.clear();
            m_setSelection.insert(m_uiHoverEnt);
            }
        }

        EntitySelection.Trigger(XtoA(!m_setSelection.empty()));

        m_iState = STATE_CREATE;
    }
}


/*====================
  CEntityTool::Render
  ====================*/
void    CEntityTool::Render()
{
}


/*====================
  CEntityTool::ToggleTreeMode
  ====================*/
void    CEntityTool::ToggleTreeMode()
{
    if (m_iOverMode == ENTITY_MODE_TREE)
    {
        m_iOverMode = ENTITY_MODE_NORMAL;
        
        le_entitySnapAbsolute = SnapAbsoluteDelta;
        le_entitySnap = SnapDelta;
        le_entityPositionSnap = PositionSnapDelta;
        le_entityHeightSnap = HeightSnapDelta;
        le_entityAngleSnap = AngleSnapDelta;
        le_entityScaleSnap = ScaleSnapDelta;

    }
    else
    {
        m_iOverMode = ENTITY_MODE_TREE;

        SnapAbsoluteDelta = le_entitySnapAbsolute;
        SnapDelta = le_entitySnap;
        PositionSnapDelta = le_entityPositionSnap;
        HeightSnapDelta = le_entityHeightSnap;
        AngleSnapDelta = le_entityAngleSnap;
        ScaleSnapDelta = le_entityScaleSnap;
        le_entitySnap = true;
        le_entityPositionSnap = 64.0;
    }

    le_treeMode = m_iOverMode;
}


/*====================
  CEntityTool::EnableTreeMode
  ====================*/
void    CEntityTool::EnableTreeMode()
{
    m_iOverMode = ENTITY_MODE_TREE;

    SnapAbsoluteDelta = le_entitySnapAbsolute;
    SnapDelta = le_entitySnap;
    PositionSnapDelta = le_entityPositionSnap;
    HeightSnapDelta = le_entityHeightSnap;
    AngleSnapDelta = le_entityAngleSnap;
    ScaleSnapDelta = le_entityScaleSnap;
    le_entitySnap = true;
    le_entityPositionSnap = 64.0;

    le_treeMode = m_iOverMode;
}


/*====================
  CEntityTool::DisableTreeMode
  ====================*/
void    CEntityTool::DisableTreeMode()
{
    m_iOverMode = ENTITY_MODE_NORMAL;
    
    le_entitySnapAbsolute = SnapAbsoluteDelta;
    le_entitySnap = SnapDelta;
    le_entityPositionSnap = PositionSnapDelta;
    le_entityHeightSnap = HeightSnapDelta;
    le_entityAngleSnap = AngleSnapDelta;
    le_entityScaleSnap = ScaleSnapDelta;

    le_treeMode = m_iOverMode;
}


/*====================
  CEntityTool::StartTreeSelect
  ====================*/
void    CEntityTool::StartTreeSelect()
{
        if (m_uiHoverEnt == INVALID_INDEX)
    {
        m_iState = STATE_SELECT;
        m_vStartCursorPos = Input.GetCursorPos();
    }
    else if (m_uiHoverEnt != INVALID_INDEX)
    {               
        if (Editor.GetWorld().GetEntity(m_uiHoverEnt)->GetType().compare(_T("Prop_Tree")) != 0)         
                return;
            
        if (m_bModifier2)
        {
            if (m_setSelection.find(m_uiHoverEnt) == m_setSelection.end())
                m_setSelection.insert(m_uiHoverEnt);
            else
                m_setSelection.erase(m_uiHoverEnt);

            EntitySelection.Trigger(XtoA(!m_setSelection.empty()));
        }
        else if (m_bModifier3)
        {
            m_setSelection.erase(m_uiHoverEnt);
            EntitySelection.Trigger(XtoA(!m_setSelection.empty()));
        }
        else
        {
            m_setSelection.clear();
            m_setSelection.insert(m_uiHoverEnt);
            EntitySelection.Trigger(XtoA(!m_setSelection.empty()));
        }
        
//      CWorldEntity * pEntity = Editor.GetWorld().GetEntity(m_uiHoverEnt);
        m_iState = STATE_HOVERING;
        m_vStartCursorPos = Input.GetCursorPos();
    }
}


/*====================
  CEntityTool::ApplyTreeSelect
  ====================*/
void CEntityTool::ApplyTreeSelect()
{
if (!(m_bModifier2 || m_bModifier3))
    {
        m_setSelection.clear();
    }

    CRectf rect(MIN(m_vStartCursorPos.x, Input.GetCursorPos().x),
        MIN(m_vStartCursorPos.y, Input.GetCursorPos().y),
        MAX(m_vStartCursorPos.x, Input.GetCursorPos().x),
        MAX(m_vStartCursorPos.y, Input.GetCursorPos().y));

    WorldEntList &vEntities(Editor.GetWorld().GetEntityList());
    WorldEntList_cit cit(vEntities.begin()), citEnd(vEntities.end());
    for (; cit != citEnd; ++cit)
    {
        CWorldEntity *pWorldEnt(Editor.GetWorld().GetEntityByHandle(*cit));
        
        if (pWorldEnt && Editor.GetCamera().IsPointInScreenRect(pWorldEnt->GetPosition(), rect))
        {
            if (pWorldEnt->GetType().compare(_T("Prop_Tree")) == 0)
            {
                if (m_bModifier2)
                    m_setSelection.insert(pWorldEnt->GetIndex());
                else if (m_bModifier3)
                    m_setSelection.erase(pWorldEnt->GetIndex());
                else
                    m_setSelection.insert(pWorldEnt->GetIndex());
            }
        }
    }

    EntitySelection.Trigger(XtoA(!m_setSelection.empty()));
}


/*====================
  CEntityTool::StartTreeTranslateXY
  ====================*/
void    CEntityTool::StartTreeTranslateXY()
{
    try
    {
        if (le_entitySelectionLock)
        {
            if (m_setSelection.empty())
                return;

            if (m_bModifier1 && (!m_bModifier2 && !m_bModifier3))
            {
                m_setOldSelection = m_setSelection;
                CloneSelection();
                m_bCloning = true;
            }
            else
            {
                m_bCloning = false;
            }

            m_iState = STATE_TRANSLATE_XY;

            if ((le_entityCenterMode == CENTER_HOVER || le_entityCenterMode == CENTER_INDIVIDUAL) && Editor.GetWorld().GetEntity(m_uiHoverEnt, true) != NULL)
                m_vTranslate = Editor.GetWorld().GetEntity(m_uiHoverEnt, true)->GetPosition();
            else
                m_vTranslate = SelectionCenter();

            m_bSnapCursor = true;

            m_vTranslate.z = Editor.GetWorld().GetTerrainHeight(m_vTranslate.x, m_vTranslate.y);

            m_vTrueTranslate = m_vTranslate;
            m_vStartCursorPos = Input.GetCursorPos();
            return;
        }

        if (m_uiHoverEnt == INVALID_INDEX)
        {
            m_iState = STATE_SELECT;
            m_vStartCursorPos = Input.GetCursorPos();
        }
        else if (m_uiHoverEnt != INVALID_INDEX)
        {
            if (Editor.GetWorld().GetEntity(m_uiHoverEnt)->GetType().compare(_T("Prop_Tree")) != 0)
                return;

            if (m_bModifier2)
            {
                if (m_setSelection.find(m_uiHoverEnt) == m_setSelection.end())
                    m_setSelection.insert(m_uiHoverEnt);
                else
                    m_setSelection.erase(m_uiHoverEnt);

                EntitySelection.Trigger(XtoA(!m_setSelection.empty()));
            }
            else if (m_bModifier3)
            {
                m_setSelection.erase(m_uiHoverEnt);
                EntitySelection.Trigger(XtoA(!m_setSelection.empty()));
            }
            else
            {
                if (m_setSelection.find(m_uiHoverEnt) == m_setSelection.end())
                {
                    m_setSelection.clear();
                    m_setSelection.insert(m_uiHoverEnt);
                    EntitySelection.Trigger(XtoA(!m_setSelection.empty()));
                }

                if (m_bModifier1 && (!m_bModifier2 && !m_bModifier3))
                {
                    m_setOldSelection = m_setSelection;
                    CloneSelection();
                    m_bCloning = true;
                }
                else
                {
                    m_bCloning = false;
                }

                m_iState = STATE_TRANSLATE_XY;

                if (le_entityCenterMode == CENTER_HOVER || le_entityCenterMode == CENTER_INDIVIDUAL)
                    m_vTranslate = Editor.GetWorld().GetEntity(m_uiHoverEnt, true)->GetPosition();
                else
                    m_vTranslate = SelectionCenter();

                m_vTranslate.z = Editor.GetWorld().GetTerrainHeight(m_vTranslate.x, m_vTranslate.y);

                m_bSnapCursor = true;

                m_vTrueTranslate = m_vTranslate;
                m_vStartCursorPos = Input.GetCursorPos();
            }
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityTool::StartTranslateXY() - "), NO_THROW);
        m_vTranslate.Clear();
        m_vTrueTranslate.Clear();
    }
}


/*====================
  CEntityTool::ApplyTreeTranslateXY
  ====================*/
void    CEntityTool::ApplyTreeTranslateXY()
{
    try
    {
        CVec3f v3Center;
        if (le_entityCenterMode == CENTER_AVERAGE || m_uiHoverEnt == INVALID_INDEX)
            v3Center = SelectionCenter();
        else
            v3Center = Editor.GetWorld().GetEntity(m_uiHoverEnt)->GetPosition();

        for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
        {
            CWorldEntity *pEntity(Editor.GetWorld().GetEntity(*it));
            if (pEntity == NULL)
                continue;

            CVec3f vDiff = pEntity->GetPosition() - v3Center;

            float fzOffset = v3Center.z - Editor.GetWorld().GetTerrainHeight(v3Center.x, v3Center.y);

            if (m_bModifier2)
                pEntity->SetPosition(m_vTranslate.x + vDiff.x, m_vTranslate.y + vDiff.y, pEntity->GetPosition().z);
            else
                pEntity->SetPosition(m_vTranslate.x + vDiff.x, m_vTranslate.y + vDiff.y, m_vTranslate.z + vDiff.z + fzOffset);

            Editor.GetWorld().UnlinkEntity(*it);
            Editor.GetWorld().LinkEntity(*it, LINK_SURFACE|LINK_MODEL, SURF_PROP);

            map<uint, CWorldEntityEx>::iterator findit(g_WorldEntData.find(pEntity->GetIndex()));

            if (findit != g_WorldEntData.end())
            {
                vector<PoolHandle> &vPathBlockers(findit->second.GetPathBlockers());

                vector<PoolHandle>::const_iterator citEnd(vPathBlockers.end());
                for (vector<PoolHandle>::const_iterator cit(vPathBlockers.begin()); cit != citEnd; ++cit)
                    Editor.GetWorld().ClearPath(*cit);

                vPathBlockers.clear();
            
                const vector<CConvexPolyhedron> &cWorldSurfs(pEntity->GetWorldSurfsRef());
                for (vector<CConvexPolyhedron>::const_iterator cit(cWorldSurfs.begin()); cit != cWorldSurfs.end(); ++cit)
                    Editor.GetWorld().BlockPath(vPathBlockers, NAVIGATION_UNIT, *cit, 24.0f);

                // Hack for tree blockers
                if (pEntity->GetType().compare(_T("Prop_Tree")) == 0)
                    vPathBlockers.push_back(Editor.GetWorld().BlockPath(NAVIGATION_UNIT, pEntity->GetPosition().xy() - CVec2f(50.0f), 100.0f, 100.0f));
            }
        }

        m_vTranslate.Clear();
        m_vTrueTranslate.Clear();
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityTool::ApplyTranslateXY() - "), NO_THROW);
        m_vTranslate.Clear();
        m_vTrueTranslate.Clear();
    }
}


/*====================
  CEntityTool::ApplyTreeTranslateZ
  ====================*/
void    CEntityTool::ApplyTreeTranslateZ()
{
}


/*====================
  CEntityTool::TreeTranslateXY
  ====================*/
void    CEntityTool::TreeTranslateXY()
{
    if (Input.GetCursorPos() == m_vStartCursorPos)
        return;

    if (m_bSnapCursor) // to delay the first update until the mouse moves
    {
        if (le_entityCenterMode == CENTER_HOVER || le_entityCenterMode == CENTER_INDIVIDUAL && m_uiHoverEnt != INVALID_INDEX)
        {
            CWorldEntity *pEntity(Editor.GetWorld().GetEntity(m_uiHoverEnt));
            if (pEntity != NULL)
                SnapCursor(CVec3f(pEntity->GetPosition().x, pEntity->GetPosition().y, Editor.GetWorld().GetTerrainHeight(pEntity->GetPosition().x, pEntity->GetPosition().y)));
        }
        else
        {
            CVec3f vCenter(SelectionCenter());
            SnapCursor(CVec3f(vCenter.x, vCenter.y, Editor.GetWorld().GetTerrainHeight(vCenter.x, vCenter.y)));
        }

        m_bSnapCursor = false;
        return;
    }

    m_vTrueTranslate = m_v3EndPos;

    if (le_entitySnap)
    {
        if (le_entitySnapAbsolute)
        {
            m_vTranslate.x = ROUND(m_vTrueTranslate.x / le_entityPositionSnap) * le_entityPositionSnap;
            m_vTranslate.y = ROUND(m_vTrueTranslate.y / le_entityPositionSnap) * le_entityPositionSnap;
        }
        else
        {
            m_vTranslate.x = m_vTrueTranslate.x - fmod(m_vTrueTranslate.x, le_entityPositionSnap);
            m_vTranslate.y = m_vTrueTranslate.y - fmod(m_vTrueTranslate.y, le_entityPositionSnap);
        }

        m_vTranslate.z = m_vTrueTranslate.z;
    }
    else
    {
        m_vTranslate = m_vTrueTranslate;
    }

    m_vStartCursorPos.Clear();
}


/*====================
  CEntityTool::GetSelectionTreeDef
  ====================*/
tstring CEntityTool::GetSelectionTreeDef()
{
    if (m_setSelection.empty())
        return le_entityTreeDefinition;
    
    ResHandle TreeHandle = Editor.GetWorld().GetEntity(*m_setSelection.begin())->GetTreeDefinition();
    CTreeDefinitionResource * TreeDefinition;
    tstring sTreePath;
    

    if (TreeHandle != NULL)
    {
        TreeDefinition = static_cast<CTreeDefinitionResource*>(g_ResourceManager.Get(TreeHandle));
        sTreePath = TreeDefinition->GetPath();
    }
    else
        sTreePath = le_entityTreeDefinition;

    
    for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
    {
        CWorldEntity *pEntity(Editor.GetWorld().GetEntity(*it));
        if (pEntity == NULL)
            continue;

        if (pEntity->GetTreeDefinition() != NULL)
        {
            CTreeDefinitionResource * TreeDefinition2 = static_cast<CTreeDefinitionResource*>(g_ResourceManager.Get(pEntity->GetTreeDefinition()));
            if (sTreePath != TreeDefinition2->GetPath())
                return _T("");
        }
    }

    return sTreePath;
}


/*====================
  CEntityTool::SetSelectionTreeDef
  ====================*/
void    CEntityTool::SetSelectionTreeDef(const tstring &sTreePath)
{
    ResHandle hTreeDef(g_ResourceManager.Register(sTreePath, RES_TREE));
    if (hTreeDef == INVALID_RESOURCE)
        return;

    for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
    {
        CWorldEntity *pEntity(Editor.GetWorld().GetEntity(*it));
        if (pEntity == NULL)
            continue;

        ResHandle hTreeDef2 = pEntity->GetTreeDefinition();
        if (hTreeDef2 == NULL)
            continue;

        Editor.GetWorld().UnlinkEntity(pEntity->GetIndex());
        pEntity->SetTreeDefinition(hTreeDef);
        pEntity->SetModelPath(static_cast<CTreeDefinitionResource*>(g_ResourceManager.Get(pEntity->GetTreeDefinition()))->GetTreeModelPath());
        pEntity->SetModelHandle(g_ResourceManager.Register(static_cast<CTreeDefinitionResource*>(g_ResourceManager.Get(pEntity->GetTreeDefinition()))->GetTreeModelPath(), RES_MODEL));
        float tmin = static_cast<CTreeDefinitionResource*>(g_ResourceManager.Get(pEntity->GetTreeDefinition()))->GetTreeScaleMin();
        float tmax = static_cast<CTreeDefinitionResource*>(g_ResourceManager.Get(pEntity->GetTreeDefinition()))->GetTreeScaleMax();
        pEntity->AdjustAngle(YAW, M_Randnum(1.0f, 360.0f));
        pEntity->SetScale(M_Randnum(tmin,tmax));
        Editor.GetWorld().LinkEntity(pEntity->GetIndex(), LINK_SURFACE|LINK_MODEL, SURF_PROP);

    }

    EntitySelectionModel.Trigger(TSNULL);
}

/*--------------------
  SetSelectionTreeDef
  --------------------*/
UI_VOID_CMD(SetSelectionTreeDef, 1)
{
    if (vArgList.size() < 1)
        EX_MESSAGE(_T("syntax: SetSelectionTreeDef <treedef>"));
    GET_TOOL(Entity, ENTITY)->SetSelectionTreeDef(vArgList[0]->Evaluate());
}