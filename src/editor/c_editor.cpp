// (C)2005 S2 Games
// c_editor.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "editor_common.h"

#include "c_editor.h"
#include "c_toolbox.h"
#include "c_entitytool.h"
#include "c_deformtool.h"
#include "c_painttool.h"
#include "c_occludertool.h"
#include "c_lighttool.h"
#include "c_foliagetool.h"
#include "c_lightmaptool.h"
#include "c_soundtool.h"
#include "c_blockertool.h"
#include "c_clifftool.h"
#include "c_watertool.h"

#include "../hon_shared/c_entityregistry.h"
#include "../hon_shared/c_entitydefinitionresource.h"
#include "../hon_shared/i_unitdefinition.h"

#include "../k2/c_hostclient.h"
#include "../k2/c_world.h"
#include "../k2/c_camera.h"
#include "../k2/c_input.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_input.h"
#include "../k2/c_inputstate.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_actionregistry.h"
#include "../k2/c_vid.h"
#include "../k2/c_worldentity.h"
#include "../k2/c_brush.h"
#include "../k2/c_worldlight.h"
#include "../k2/c_scenelight.h"
#include "../k2/c_uimanager.h"
#include "../k2/c_uitrigger.h"
#include "../k2/c_bitmap.h"
#include "../k2/c_texture.h"
#include "../k2/c_skeleton.h"
#include "../k2/c_model.h"
#include "../k2/c_k2model.h"
#include "../k2/c_draw2d.h"
#include "../k2/c_soundmanager.h"
#include "../k2/c_DebugRenderer.h"
#include "../k2/c_fontmap.h"
#include "../k2/s_tile.h"

#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
CVAR_FLOAT  (le_camSpeed,               1000.0f);
CVAR_FLOAT  (le_camFov,                 53.75f);
CVAR_FLOAT  (le_camAspect,              1.515f);
CVAR_FLOAT  (le_camWeightX,             1.0f);
CVAR_FLOAT  (le_camWeightY,             3.0f);
CVAR_FLOAT  (le_shiftCameraDistance,    125.0f);
CVAR_FLOATF (le_yawSpeed,               0.5f,               CVAR_SAVECONFIG);
CVAR_FLOATF (le_pitchSpeed,             0.5f,               CVAR_SAVECONFIG);
CVAR_BOOL   (le_drawEntities,           true);
CVAR_BOOL   (le_drawLights,             true);
CVAR_INT    (le_loadBrushes,            32);
CVAR_STRING (le_brushPath,              "/core/brushes/standard");
CVAR_BOOL   (le_useOccluders,           true);
CVAR_BOOL   (le_drawOccluders,          false);
CVAR_BOOL   (le_camDebug,               false);
CVAR_BOOL   (le_globalScript,           false);
CVAR_STRING (le_currentScript,          "");
CVAR_STRING (le_scriptData,             "");
CVAR_STRING (le_rulerModel,             "/heroes/femmeflame/model.mdf");
CVAR_FLOAT  (le_rulerScale,             1.75f);
CVAR_BOOL   (le_drawNavGrid,            false);
CVAR_INT    (le_drawNavGridDownsize,    0);
CVAR_FLOAT  (le_pathRadius,             0.0f);
CVAR_INT    (le_pathDisplaySize,        32);
CVAR_FLOAT  (le_pathRange,              0.0f);
CVAR_STRINGF    (le_worldNewTexture,        "/world/terrain/textures/g1_ground_d.tga",      CVAR_SAVECONFIG);
CVAR_STRINGF    (le_worldNewNormal,         "/world/terrain/textures/g1_ground_n.tga",      CVAR_SAVECONFIG);

INPUT_STATE_BOOL(MouseLook);

INPUT_STATE_BOOL(MoveForward);
INPUT_STATE_BOOL(MoveBack);
INPUT_STATE_BOOL(MoveLeft);
INPUT_STATE_BOOL(MoveRight);
INPUT_STATE_BOOL(MoveUp);
INPUT_STATE_BOOL(MoveDown);
INPUT_STATE_BOOL(ShowScale);

extern CCvar<float> le_occluderAlpha;

extern CUITrigger BrushImage;

UI_TRIGGER(Minimap);
UI_TRIGGER(ClearScriptList);
UI_TRIGGER(ScriptList);
UI_TRIGGER(ImportedFiles);
UI_TRIGGER(ClearImportedFiles);

map<uint, CWorldEntityEx>   g_WorldEntData;
//=============================================================================

/*====================
  CEditor::~CEditor
  ====================*/
CEditor::~CEditor()
{
    if (m_pCamera != NULL)
        K2_DELETE(m_pCamera);
    SAFE_DELETE(m_pMinimapBitmap); // UTTAR
}


/*====================
  CEditor::CEditor
  ====================*/
CEditor::CEditor() : IGame(_T("editor")),
m_pCamera(NULL),
m_v3CamAngles(-45.0f, 0.0f, 315.0f),
m_pHostClient(NULL),
m_pWorld(NULL),

m_hLineMaterial(g_ResourceManager.Register(_T("/core/materials/line.material"), RES_MATERIAL)),
m_hOccluderMaterial(g_ResourceManager.Register(_T("/core/materials/occluder.material"), RES_MATERIAL)),
m_hMinimapReference(INVALID_RESOURCE),
m_hMinimapTexture(INVALID_RESOURCE),
m_pMinimapBitmap(NULL),
m_v2PathStart(0.0f, 0.0f),
m_v2PathEnd(0.0f, 0.0f),
m_bPathStartValid(false),
m_bPathEndValid(false),
m_hPath(INVALID_POOL_HANDLE),
m_bRuler(false)
{
    float fMaxUInt((float)(uint)-1);
    ICvar::SetFloat(_T("gfx_fogFar"), fMaxUInt);
    ICvar::SetFloat(_T("gfx_fogNear"), fMaxUInt - 10000.0f);
    scene_farClip = fMaxUInt;
    scene_worldFarClip = fMaxUInt;
}


/*====================
  CEditor::SetNullTiles
  ====================*/
void    CEditor::SetNullTiles()
{
    ResHandle rhNewTex = g_ResourceManager.Register(K2_NEW(ctx_Editor,   CTexture)(le_worldNewTexture, TEXTURE_2D, 0, TEXFMT_A8R8G8B8), RES_TEXTURE);
    ResHandle rhNewNorm = g_ResourceManager.Register(K2_NEW(ctx_Editor,   CTexture)(le_worldNewNormal, TEXTURE_2D, 0, TEXFMT_NORMALMAP), RES_TEXTURE);

    if (rhNewTex == INVALID_RESOURCE && rhNewNorm == INVALID_RESOURCE)
        return;

    STile *pRegion(NULL);

    // Clip the brush
    CRecti  recClippedBrush(0, 0, Editor.GetWorld().GetTileWidth(), Editor.GetWorld().GetTileHeight());

    // Clip the brush against the world
    if (!Editor.GetWorld().ClipRect(recClippedBrush, TILE_SPACE))
        return;

    // Get the region
    pRegion = K2_NEW_ARRAY(ctx_Editor, STile, recClippedBrush.GetArea());
    if (pRegion == NULL)
        EX_ERROR(_T("Failed to allocate region"));

    if (!Editor.GetWorld().GetRegion(WORLD_TILE_MATERIAL_MAP, recClippedBrush, pRegion, 0))
        EX_ERROR(_T("Failed to retrieve region"));

    int iRegionIndex(0);

    for (int yy(0); yy < recClippedBrush.GetHeight(); ++yy)
    {
        for (int xx(0); xx < recClippedBrush.GetWidth(); ++xx)
        {
            if (rhNewTex != INVALID_RESOURCE)
                pRegion[iRegionIndex].hDiffuse = rhNewTex;

            if (rhNewNorm != INVALID_RESOURCE)
                pRegion[iRegionIndex].hNormalmap = rhNewNorm;

            Vid.Notify(VID_NOTIFY_TERRAIN_SHADER_MODIFIED, xx, yy, 0, &Editor.GetWorld());

            ++iRegionIndex;
        }
    }

    // Apply the modified region
    if (!Editor.GetWorld().SetRegion(WORLD_TILE_MATERIAL_MAP, recClippedBrush, pRegion, 0))
        EX_ERROR(_T("SetRegion() failed"));

    if(pRegion)
        K2_DELETE_ARRAY(pRegion);
}

/*====================
  CEditor::Init
  ====================*/
bool    CEditor::Init(CHostClient *pHostClient)
{
    PROFILE("CEditor::Init");

    try
    {
        SetGamePointer();

        m_pCamera = K2_NEW(ctx_Editor,   CCamera);
        if (m_pCamera == NULL)
            EX_ERROR(_T("Failed to allocate camera"));

        m_v3TargetCamAngles = m_v3CamAngles;
        m_v3TargetCamPosition = CVec3f(1000.0f, 1000.0f, 1000.0f);

        m_pCamera->DefaultCamera(float(Vid.GetScreenW()), float(Vid.GetScreenH()));
        m_pCamera->SetOrigin(m_v3TargetCamPosition);

        m_pCamera->AddFlags(CAM_NO_DEPTH_CLEAR);

        m_pHostClient = pHostClient;
        if (m_pHostClient == NULL)
            EX_ERROR(_T("Inalid CHostClient"));

        m_pWorld = m_pHostClient->GetWorld();
        if (m_pWorld == NULL)
            EX_ERROR(_T("Invalid CWorld from host"));
        
        m_pWorld->SetHostType(WORLDHOST_BOTH);

        m_pCamera->SetWorld(m_pWorld);

        // Setup actions
        ActionRegistry.BindAxis(BINDTABLE_GAME, AXIS_MOUSE_X, BIND_MOD_NONE,        _T("CameraYaw"));
        ActionRegistry.BindAxis(BINDTABLE_GAME, AXIS_MOUSE_Y, BIND_MOD_NONE,        _T("CameraPitch"));

        ActionRegistry.BindButton(BINDTABLE_GAME, EButton('W'), BIND_MOD_NONE,      _T("MoveForward"));
        ActionRegistry.BindButton(BINDTABLE_GAME, EButton('S'), BIND_MOD_NONE,      _T("MoveBack"));

        ActionRegistry.BindButton(BINDTABLE_GAME, EButton('A'), BIND_MOD_NONE,      _T("MoveLeft"));
        ActionRegistry.BindButton(BINDTABLE_GAME, EButton('D'), BIND_MOD_NONE,      _T("MoveRight"));

        ActionRegistry.BindButton(BINDTABLE_GAME, BUTTON_UP, BIND_MOD_NONE,         _T("MoveForward"));
        ActionRegistry.BindButton(BINDTABLE_GAME, BUTTON_DOWN, BIND_MOD_NONE,       _T("MoveBack"));

        ActionRegistry.BindButton(BINDTABLE_GAME, BUTTON_LEFT, BIND_MOD_NONE,       _T("MoveLeft"));
        ActionRegistry.BindButton(BINDTABLE_GAME, BUTTON_RIGHT, BIND_MOD_NONE,      _T("MoveRight"));

        ActionRegistry.BindButton(BINDTABLE_GAME, BUTTON_SPACE, BIND_MOD_NONE,      _T("MoveUp"));
        ActionRegistry.BindButton(BINDTABLE_GAME, EButton('C'), BIND_MOD_NONE,      _T("MoveDown"));

        ActionRegistry.BindButton(BINDTABLE_GAME, EButton('Q'), BIND_MOD_NONE,      _T("TurnLeft"));
        ActionRegistry.BindButton(BINDTABLE_GAME, EButton('E'), BIND_MOD_NONE,      _T("TurnRight"));

        ActionRegistry.BindButton(BINDTABLE_GAME, BUTTON_MOUSEL, BIND_MOD_NONE,     _T("ToolPrimary"));
        ActionRegistry.BindButton(BINDTABLE_GAME, BUTTON_MOUSER, BIND_MOD_NONE,     _T("ToolSecondary"));
        ActionRegistry.BindButton(BINDTABLE_GAME, EButton('O'), BIND_MOD_NONE,      _T("ToolTertiary"));
        ActionRegistry.BindButton(BINDTABLE_GAME, EButton('P'), BIND_MOD_NONE,      _T("ToolQuaternary"));
        ActionRegistry.BindButton(BINDTABLE_GAME, BUTTON_SHIFT, BIND_MOD_NONE,      _T("ToolModifier1"));
        ActionRegistry.BindButton(BINDTABLE_GAME, BUTTON_CTRL, BIND_MOD_NONE,       _T("ToolModifier2"));
        ActionRegistry.BindButton(BINDTABLE_GAME, BUTTON_ALT, BIND_MOD_NONE,        _T("ToolModifier3"));
        ActionRegistry.BindButton(BINDTABLE_GAME, BUTTON_MOUSEM, BIND_MOD_NONE,     _T("MouseLook"));

        ActionRegistry.BindImpulse(BINDTABLE_GAME, BUTTON_MISC4, BIND_MOD_NONE,     _T("PathSetStart"));
        ActionRegistry.BindImpulse(BINDTABLE_GAME, BUTTON_MISC6, BIND_MOD_NONE,     _T("PathSetEnd"));
        ActionRegistry.BindImpulse(BINDTABLE_GAME, BUTTON_MISC5, BIND_MOD_NONE,     _T("PathClear"));

        ActionRegistry.BindImpulse(BINDTABLE_GAME, BUTTON_ESC, BIND_MOD_NONE,       _T("Cancel"));
        ActionRegistry.BindImpulse(BINDTABLE_GAME, BUTTON_DEL, BIND_MOD_NONE,       _T("Delete"));

        ActionRegistry.BindImpulse(BINDTABLE_GAME, BUTTON_WHEELUP, BIND_MOD_NONE,   _T("MoveIn"));
        ActionRegistry.BindImpulse(BINDTABLE_GAME, BUTTON_WHEELDOWN, BIND_MOD_NONE, _T("MoveOut"));

        ActionRegistry.BindImpulse(BINDTABLE_GAME, EButton('Z'), BIND_MOD_NONE, _T("Cmd"), _T("Toggle le_entitySnap"));
        ActionRegistry.BindImpulse(BINDTABLE_GAME, EButton('L'), BIND_MOD_NONE, _T("Cmd"), _T("Toggle le_entitySelectionLock"));

        ActionRegistry.BindButton(BINDTABLE_GAME, EButton('F'), BIND_MOD_NONE,      _T("ShowScale"));

        ActionRegistry.BindButton(BINDTABLE_GAME, EButton('R'), BIND_MOD_NONE,      _T("Ruler"));

        ActionRegistry.BindImpulse(BINDTABLE_GAME, BUTTON_BACKSPACE, BIND_MOD_NONE, _T("Cmd"), _T("Center"));

        m_hMinimapReference = g_ResourceManager.Register(_T("!minimap_texture"), RES_REFERENCE);
        g_ResourceManager.UpdateReference(m_hMinimapReference, g_ResourceManager.GetBlackTexture());

        tsvector vFileList;
        FileManager.GetFileList(_T("/"), _T("*.entity"), true, vFileList);
        for (tsvector_it it(vFileList.begin()); it != vFileList.end(); ++it)
            g_ResourceManager.Register(*it, RES_ENTITY_DEF);

        // Load brushes
        for (int i(0); i < le_loadBrushes; ++i)
        {
            tstring sFileName(le_brushPath + _T("/brush") + XtoA(i + 1) + _T(".tga"));
            CBrush::Load(i, sFileName);
        }

        // Intialize tools
        #define REGISTER_TOOL(name) if (!ToolBox.Register(K2_NEW(ctx_Editor, C##name##Tool))) EX_ERROR(_T("Failed to allocate") _T(#name) _T(" tool"))
        REGISTER_TOOL(Deform);
        REGISTER_TOOL(Paint);
        REGISTER_TOOL(Entity);
        REGISTER_TOOL(Foliage);
        REGISTER_TOOL(Lightmap);
        REGISTER_TOOL(Light);
        REGISTER_TOOL(Occluder);
        REGISTER_TOOL(Sound);
        REGISTER_TOOL(Blocker);
        REGISTER_TOOL(Cliff);
        REGISTER_TOOL(Water);
        #undef REGISTER_TOOL

        {
            PROFILE("Load interface");
            // Load interface
            UIManager.LoadInterface(_T("/ui/le_main.interface"));
            UIManager.SetActiveInterface(_T("le_main"));
        }
        
        ToolBox.SelectTool(TOOL_ENTITY);
        GET_TOOL(Entity, ENTITY)->SetEditMode(_T("select"));

        BrushImage.Trigger(CBrush::GetCurrentBrush()->GetFilename());

        // Start with a blank world
        m_pWorld->New(_T("default"), 9, 32.0f, 4, 16.0f);

        SetNullTiles();

        UpdateMinimapTexture();
        UpdateScripts();
        UpdateImportedFiles();

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEditor::Init() - "), NO_THROW);
        return false;
    }
}


/*====================
  CEditor::LoadWorld
  ====================*/
bool    CEditor::LoadWorld(const tstring &sWorldName)
{
    try
    {
        if (!m_pWorld->StartLoad(sWorldName))
            EX_ERROR(_T("World load failed"));

        while (m_pWorld->IsLoading())
            if (!m_pWorld->LoadNextComponent())
                EX_ERROR(_T("World load failed"));

        g_WorldEntData.clear();

        m_pWorld->AnalyzeTerrain();

        WorldEntList &vEntities(m_pWorld->GetEntityList());
        for (WorldEntList_cit cit(vEntities.begin()), citEnd(vEntities.end()); cit != citEnd; ++cit)
        {
            CWorldEntity *pWorldEnt(m_pWorld->GetEntityByHandle(*cit));
            if (pWorldEnt == NULL)
                continue;

            pWorldEnt->SetModelHandle(g_ResourceManager.Register(pWorldEnt->GetModelPath(), RES_MODEL));
            pWorldEnt->SetSkin(g_ResourceManager.GetSkin(pWorldEnt->GetModelHandle(), pWorldEnt->GetSkinName()));

            tstring sType(pWorldEnt->GetType());
            if (sType == _T("Entity_NeutralCampSpawner"))
                sType = pWorldEnt->GetProperty(_T("target0"));

            IUnitDefinition *pDefinition(EntityRegistry.GetDefinition<IUnitDefinition>(sType));
            if (pDefinition != NULL)
            {
                pWorldEnt->SetScale(pDefinition->GetPreGlobalScale(0) * pDefinition->GetModelScale(0));
                pWorldEnt->SetModelHandle(g_ResourceManager.Register(pDefinition->GetModelPath(0), RES_MODEL));
                pWorldEnt->SetSkin(g_ResourceManager.GetSkin(pWorldEnt->GetModelHandle(), pDefinition->GetSkin(0)));
            }

            g_ResourceManager.PrecacheSkin(pWorldEnt->GetModelHandle(), pWorldEnt->GetSkin());

            m_pWorld->LinkEntity(pWorldEnt->GetIndex(), LINK_SURFACE | LINK_MODEL, SURF_PROP);

            map<uint, CWorldEntityEx>::iterator findit(g_WorldEntData.find(pWorldEnt->GetIndex()));

            if (findit == g_WorldEntData.end())
            {
                g_WorldEntData.insert(pair<uint, CWorldEntityEx>(pWorldEnt->GetIndex(), CWorldEntityEx()));
                findit = g_WorldEntData.find(pWorldEnt->GetIndex());
            }

            const vector<CConvexPolyhedron> &cWorldSurfs(pWorldEnt->GetWorldSurfsRef());
            for (vector<CConvexPolyhedron>::const_iterator cit(cWorldSurfs.begin()); cit != cWorldSurfs.end(); ++cit)
                m_pWorld->BlockPath(findit->second.GetPathBlockers(), NAVIGATION_UNIT, *cit, 24.0f);

            // Hack for tree blockers
            if (pWorldEnt->GetType() == _T("Prop_Tree"))
                findit->second.GetPathBlockers().push_back(m_pWorld->BlockPath(NAVIGATION_UNIT, pWorldEnt->GetPosition().xy() - CVec2f(50.0f), 100.0f, 100.0f));
        }

        UpdateMinimapTexture();
        UpdateScripts();
        UpdateImportedFiles();
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEditor::LoadWorld() - "), NO_THROW);
        return false;
    }
}


/*====================
  CEditor::DrawEntities
  ====================*/
void    CEditor::DrawEntities()
{
    PROFILE("CEditor::DrawEntities");

    if (!le_drawEntities)
        return;

    CEntityTool *pTool(NULL);
    if (ToolBox.IsToolSelected(TOOL_ENTITY))
        pTool = static_cast<CEntityTool*>(ToolBox.GetCurrentTool());

    CSceneEntity sc;

    WorldEntList &vEntities(m_pWorld->GetEntityList());
    for (WorldEntList_cit cit(vEntities.begin()), citEnd(vEntities.end()); cit != citEnd; ++cit)
    {
        CWorldEntity *pEnt(m_pWorld->GetEntityByHandle(*cit));
        if (pEnt == NULL)
            continue;

        uint uiIndex(pEnt->GetIndex());

        sc.Clear();

        if (pTool != NULL)
        {

            sc.scale = pTool->GetScale(uiIndex);
            sc.angle = pTool->GetAngles(uiIndex);
            sc.SetPosition(pTool->GetPosition(uiIndex));
            sc.hRes = pEnt->GetModelHandle();
            sc.hSkin = pEnt->GetSkin();
            sc.objtype = OBJTYPE_MODEL;
            sc.flags = 0;
            sc.color = BLACK;

            if ((pTool->GetHoverEntity() == uiIndex || pTool->IsHoverSelected(uiIndex)))
            {
                sc.flags |= SCENEENT_SHOW_WIRE | SCENEENT_SOLID_COLOR;

                if (!pTool->IsSelectionLock())
                    sc.color[G] = 2.0f;
            }

            if (pTool->IsSelected(uiIndex))
            {
                sc.flags |= SCENEENT_SHOW_BOUNDS | SCENEENT_SOLID_COLOR;
                if (pTool->IsSelectionLock())
                    sc.color = CVec4f(2.0f, 0.0f, 2.0f, 1.0f);
                else
                    sc.color[R] = 2.0f;
            }
        }
        else
        {
            sc.scale =  pEnt->GetScale();
            sc.angle = pEnt->GetAngles();
            sc.SetPosition(pEnt->GetPosition());
            sc.hRes = pEnt->GetModelHandle();
            sc.hSkin = pEnt->GetSkin();
            sc.objtype = OBJTYPE_MODEL;
            sc.flags = 0;
            sc.color = BLACK;
        }

        if (pEnt->GetType() == _T("Prop_Cliff") || pEnt->GetType() == _T("Prop_Cliff2"))
            sc.flags |= SCENEENT_TERRAIN_TEXTURES;

        CModel *pModel(g_ResourceManager.GetModel(sc.hRes));
        if (pModel != NULL)
        {
            sc.flags |= SCENEENT_USE_AXIS;
            sc.axis = CAxis(sc.angle);

            CBBoxf bbBounds(pModel->GetBounds());
            bbBounds.Transform(sc.GetPosition(), sc.axis, sc.scale);

            sc.bounds = bbBounds;
            sc.flags |= SCENEENT_USE_BOUNDS;
        }

        SSceneEntityEntry &cEntry(SceneManager.AddEntity(sc));

        if (pModel != NULL && pModel->GetModelFile() && pModel->GetModelFile()->GetType() == MODEL_K2 && (!cEntry.bCull || !cEntry.bCullShadow))
        {
            CK2Model *pK2Model(static_cast<CK2Model *>(pModel->GetModelFile()));

            if (pK2Model->GetNumAnims() > 0)
            {
                map<uint, CWorldEntityEx>::iterator findit(g_WorldEntData.find(pEnt->GetIndex()));

                if (findit == g_WorldEntData.end())
                {
                    g_WorldEntData.insert(pair<uint, CWorldEntityEx>(pEnt->GetIndex(), CWorldEntityEx()));
                    findit = g_WorldEntData.find(pEnt->GetIndex());
                }

                if (!findit->second.GetSkeleton())
                {
                    findit->second.SetSkeleton(K2_NEW(ctx_Editor,   CSkeleton)());
                }
                
                CSkeleton *pSkeleton(findit->second.GetSkeleton());

                pSkeleton->SetModel(cEntry.cEntity.hRes);

                tstring sAnim(pEnt->GetProperty(_T("anim"), _T("idle")));

                if (!sAnim.empty())
                {
                    if (pSkeleton->GetCurrentAnimName(0) != sAnim)
                        pSkeleton->StartAnim(sAnim, m_pHostClient->GetTime(), 0);

                    if (!cEntry.bCull || !cEntry.bCullShadow)
                        pSkeleton->Pose(m_pHostClient->GetTime());
                    else
                        pSkeleton->PoseLite(m_pHostClient->GetTime());

                    cEntry.cEntity.skeleton = pSkeleton;
                }

                if (pSkeleton)
                    pSkeleton->ClearEvents();
            }
        }
    }
}


/*====================
  CEditor::AddLights

  Add dynamic lights to the scene
  ====================*/
void    CEditor::AddLights()
{
    if (!le_drawLights)
        return;

    CLightTool *pTool(NULL);
    if (ToolBox.IsToolSelected(TOOL_LIGHT))
        pTool = static_cast<CLightTool*>(ToolBox.GetCurrentTool());

    WorldLightsMap& mapLights(Editor.GetWorld().GetLightsMap());
    for (WorldLightsMap_it it(mapLights.begin()); it != mapLights.end(); ++it)
    {
        uint uiIndex(it->second->GetIndex());

        CSceneLight scLight
        (
            pTool ? pTool->GetCurrentLightPosition(uiIndex) : it->second->GetPosition(),
            it->second->GetColor(),
            pTool ? pTool->GetStartFalloff(uiIndex) :  it->second->GetFalloffStart(),
            pTool ? pTool->GetEndFalloff(uiIndex) : it->second->GetFalloffEnd()
        );

        SceneManager.AddLight(scLight);
    }
}


/*====================
  CEditor::AddOccluders
  ====================*/
void    CEditor::AddOccluders()
{
    if (!le_useOccluders)
        return;

    OccluderMap& mapOccluders(Editor.GetWorld().GetOccluderMap());
    for (OccluderMap::const_iterator it(mapOccluders.begin()); it != mapOccluders.end(); ++it)
        SceneManager.AddOccluder(*it->second);
}


/*====================
  CEditor::DrawOccluderPoly
  ====================*/
void    CEditor::DrawOccluderPoly(const COccluder &occluder)
{
    SSceneFaceVert poly[MAX_OCCLUDER_POINTS];

    MemManager.Set(poly, 0, sizeof(poly));

    for (uint n(0); n < occluder.GetNumPoints(); ++n)
    {
        M_CopyVec3(vec3_cast(occluder.GetPoint(n)), poly[n].vtx);
        SET_VEC4(poly[n].col, 255, 255, 255, 255);
    }

    SceneManager.AddPoly(occluder.GetNumPoints(), poly, m_hLineMaterial, POLY_DOUBLESIDED | POLY_WIREFRAME | POLY_NO_DEPTH_TEST);

    for (uint n(0); n < occluder.GetNumPoints(); ++n)
        SET_VEC4(poly[n].col, 255, 255, 255, CLAMP(INT_ROUND(255 * le_occluderAlpha), 0, 255));

    SceneManager.AddPoly(occluder.GetNumPoints(), poly, m_hOccluderMaterial, POLY_DOUBLESIDED);
}


/*====================
  CEditor::DrawOccluders

  Draws occluders
  ====================*/
void    CEditor::DrawOccluders()
{
    if (!le_drawOccluders)
        return;

    const OccluderMap   &mapOccluders(m_pWorld->GetOccluderMap());
    for (OccluderMap::const_iterator it(mapOccluders.begin()); it != mapOccluders.end(); ++it)
    {
        if (!ToolBox.GetCurrentTool() || ToolBox.GetCurrentTool()->GetName() != _T("occluder")) // the occluder tool has special rendering for these
            DrawOccluderPoly(*it->second);
    }
}



/*====================
  CEditor::Frame
  ====================*/
void    CEditor::Frame()
{
    PROFILE("CEditor::Frame");

    UIManager.SetActiveInterface(_T("le_main"));

    Input.ExecuteBinds(BINDTABLE_GAME, 0);

    if (MouseLook)
    {
        Input.SetCursorFrozen(CURSOR_GAME, BOOL_TRUE);
        Input.SetCursorHidden(CURSOR_GAME, BOOL_TRUE);
    }
    else
    {
        Input.SetCursorFrozen(CURSOR_GAME, BOOL_FALSE);
        Input.SetCursorHidden(CURSOR_GAME, BOOL_FALSE);
    }

    DebugRenderer.ClearLists();

    m_pWorld->UpdateNavigation();

    SceneManager.Clear();

    Vid.RenderFogofWar(1.0f, false, 0.0f);

    UpdateCamera();

    K2SoundManager.SetListenerPosition(m_pCamera->GetOrigin(), V3_ZERO, m_pCamera->GetViewAxis(FORWARD), m_pCamera->GetViewAxis(UP), false);

    SceneManager.ClearBackground();

    SceneManager.PrepCamera(*m_pCamera);

    SceneManager.DrawSky(*m_pCamera, MsToSec(Host.GetFrameLength()));

    ITool *pTool(ToolBox.GetCurrentTool());

    if (pTool)
        pTool->Frame(GetFrameSeconds());

    AddOccluders();
    DrawOccluders();
    DrawEntities();
    AddLights();

    if (ShowScale)
    {
        STraceInfo trace;

        if (Editor.TraceCursor(trace, TRACE_TERRAIN))
        {
            CSceneEntity sc;
            sc.Clear();

            sc.scale = le_rulerScale;
            sc.SetPosition(trace.v3EndPos);
            sc.hRes = g_ResourceManager.Register(le_rulerModel, RES_MODEL);
            sc.objtype = OBJTYPE_MODEL;
            sc.flags = 0;

            SceneManager.AddEntity(sc);
        }
    }

    if (pTool)
        pTool->Render();

    if (m_bRuler)
    {
        STraceInfo cTrace;
        if (Editor.TraceCursor(cTrace, TRACE_TERRAIN))
        {
            m_v3RulerEnd = cTrace.v3EndPos;

            for (vector<CVec3f>::const_iterator it = m_vRulerPoints.begin(); it != m_vRulerPoints.end(); ++it)
            {
                if (it != m_vRulerPoints.begin())
                {
                    CSceneEntity sceneEntity;

                    sceneEntity.objtype = OBJTYPE_BEAM;
                    sceneEntity.scale = 8.0f;
                    sceneEntity.height = 1.0f;
                    sceneEntity.hRes = g_ResourceManager.Register(_T("/core/materials/ruler.material"), RES_MATERIAL);
                    sceneEntity.color = YELLOW;

                    sceneEntity.angle = *it;
                    --it;
                    sceneEntity.SetPosition(*it);
                    ++it;
                    SceneManager.AddEntity(sceneEntity);
                }
            }
            
            CSceneEntity sceneEntity2;

            sceneEntity2.objtype = OBJTYPE_BEAM;
            sceneEntity2.scale = 8.0f;
            sceneEntity2.height = 1.0f;
            sceneEntity2.hRes = g_ResourceManager.Register(_T("/core/materials/ruler.material"), RES_MATERIAL);
            sceneEntity2.color = YELLOW;

            sceneEntity2.angle = m_v3RulerEnd;
            sceneEntity2.SetPosition(m_v3RulerStart);

            SceneManager.AddEntity(sceneEntity2);

        }
    }

    DrawNavGrid();
    
    SceneManager.Render();

    if (m_bRuler)
    {
        CVec2f  v2ScreenPos;
        if (m_pCamera->WorldToScreen(m_v3RulerEnd, v2ScreenPos))
        {
            float cumulativeDistance = 0.0f;

            for (unsigned int i = 0; i + 1 <= m_vRulerPoints.size(); ++i)
            {
                if (i + 1 >= m_vRulerPoints.size())
                    break;
                cumulativeDistance += Distance(m_vRulerPoints[i].xy(),m_vRulerPoints[i + 1].xy());
            }
            cumulativeDistance += Distance(m_v3RulerStart.xy(), m_v3RulerEnd.xy());
            tstring sString(XtoA(cumulativeDistance));
            ResHandle hFont(g_ResourceManager.LookUpName(_T("system_medium"), RES_FONTMAP));

            CFontMap *pFontMap(g_ResourceManager.GetFontMap(hFont));

            float fStringWidth(pFontMap->GetStringWidth(sString));
            float fStringHeight(pFontMap->GetMaxHeight());

            v2ScreenPos -= CVec2f(fStringWidth, fStringHeight);

            Draw2D.SetColor(CVec4f(0.0f, 0.0f, 0.0f, 1.0f));
            Draw2D.String(floor(v2ScreenPos.x) + 1.0f, floor(v2ScreenPos.y) + 1.0f, sString, hFont);

            Draw2D.SetColor(CVec4f(1.0f, 1.0f, 0.0f, 1.0f));
            Draw2D.String(floor(v2ScreenPos.x), floor(v2ScreenPos.y), sString, hFont);
        }
    }

    if (pTool)
        pTool->Draw();

    DebugRender();
    
    DebugRenderer.Frame(m_pCamera);

    UpdateMinimap();
}

/*====================
  CEditor::RulerPointCreate
  ====================*/
void     CEditor::RulerPointCreate()
{
    STraceInfo cTrace;
    Editor.TraceCursor(cTrace, TRACE_TERRAIN);
    if (m_vRulerPoints.empty())
    m_vRulerPoints.insert(m_vRulerPoints.end(),m_v3RulerStart);
    m_vRulerPoints.insert(m_vRulerPoints.end(),cTrace.v3EndPos);
    m_v3RulerStart = cTrace.v3EndPos;
}

/*====================
  CEditor::TraceCursor
  ====================*/
bool     CEditor::TraceCursor(STraceInfo &result, int iIgnoreSurface)
{
    CVec2f v2Pos(Input.GetCursorPos());
    CVec3f vecDir(Editor.GetCamera().ConstructRay(v2Pos));
    CVec3f vecEnd(M_PointOnLine(Editor.GetCamera().GetOrigin(), vecDir, FAR_AWAY));

    return GetWorld().TraceLine(result, m_pCamera->GetOrigin(), vecEnd, iIgnoreSurface);
}

/*====================
  CEditor::TracePoint
  ====================*/
bool     CEditor::TracePoint(STraceInfo &result, int iIgnoreSurface, CVec2f point)
{
    CVec2f v2Pos(point);
    CVec3f vecDir(Editor.GetCamera().ConstructRay(v2Pos));
    CVec3f vecEnd(M_PointOnLine(Editor.GetCamera().GetOrigin(), vecDir, FAR_AWAY));

    return GetWorld().TraceLine(result, m_pCamera->GetOrigin(), vecEnd, iIgnoreSurface);
}



/*====================
  CEditor::GetLookAtPoint
  ====================*/
bool     CEditor::GetLookAtPoint(CVec3f &v3Pos)
{
    STraceInfo result;

    CVec3f v3Dir(Editor.GetCamera().GetViewAxis(FORWARD));
    CVec3f v3End(M_PointOnLine(Editor.GetCamera().GetOrigin(), v3Dir, FAR_AWAY));

    if (GetWorld().TraceLine(result, m_pCamera->GetOrigin(), v3End, TRACE_TERRAIN))
    {
        v3Pos = result.v3EndPos;
        return true;
    }

    return false;
}


/*====================
  CEditor::UpdateCamera
  ====================*/
void    CEditor::UpdateCamera()
{
    m_pCamera->SetTime(Host.GetTime() / 1000.0f);
    m_pCamera->SetAngles(m_v3CamAngles);
    m_pCamera->AddFlags(CAM_NO_CLIFFS);

    if (le_camDebug)
    {
        Console << _T("Origin: ") << m_pCamera->GetOrigin() << newl;
        Console << _T("FrameTime: ") << Editor.GetFrameSeconds() << newl;
        Console << _T("Axis[0]: ") << m_pCamera->GetViewAxis(RIGHT) << newl;
        Console << _T("Axis[1]: ") << m_pCamera->GetViewAxis(FORWARD) << newl;
        Console << _T("Axis[2]: ") << m_pCamera->GetViewAxis(UP) << newl;
    }

    CVec3f vForward;
    CVec3f vecRight;
    CVec3f v3Up;

    if (MouseLook)
    {
        vForward = m_pCamera->GetViewAxis(FORWARD);
        vecRight = m_pCamera->GetViewAxis(RIGHT);
        v3Up = m_pCamera->GetViewAxis(UP);
    }
    else
    {
        vForward = m_pCamera->GetViewAxis(FORWARD);
        vForward.z = 0.0f;
        vForward.Normalize();

        vecRight = m_pCamera->GetViewAxis(RIGHT);
        vecRight.z = 0.0f;
        vecRight.Normalize();

        v3Up = V_UP;
    }

    CVec3f vVelocity(0.0f, 0.0f, 0.0f);

    if (MoveForward)
        vVelocity += vForward * le_camSpeed;
    if (MoveBack)
        vVelocity += vForward * -le_camSpeed;
    if (MoveLeft)
        vVelocity += vecRight * -le_camSpeed;
    if (MoveRight)
        vVelocity += vecRight * le_camSpeed;
    if (MoveUp)
        vVelocity += v3Up * le_camSpeed;
    if (MoveDown)
        vVelocity += v3Up * -le_camSpeed;

    m_v3TargetCamPosition += vVelocity * Editor.GetFrameSeconds();

    if (m_fCameraShift != 0.0f)
    {
        m_v3TargetCamPosition += m_pCamera->GetViewAxis(FORWARD) * m_fCameraShift * le_shiftCameraDistance;
        m_fCameraShift = 0.0f;
    }

    CVec3f v3CamPosition(m_pCamera->GetOrigin());

    m_pCamera->SetOrigin(DECAY(m_pCamera->GetOrigin(), m_v3TargetCamPosition, 0.1f, Editor.GetFrameSeconds()));

    m_pCamera->SetWidth(float(Vid.GetScreenW()));
    m_pCamera->SetHeight(float(Vid.GetScreenH()));
    
    m_pCamera->SetFovFromAspect(le_camFov, le_camAspect, le_camWeightX, le_camWeightY);

    m_pCamera->SetReflect(CPlane(V_UP, CVec3f(0.0f, 0.0f, -92.85f)));
}


/*====================
  CEditor::CenterCamera
  ====================*/
void    CEditor::CenterCamera(const CVec3f &v3Pos)
{
    float fZ, fScale;
    const CVec3f &v3Forward = m_pCamera->GetViewAxis(FORWARD);

    // move above the point if we are below it; otherwise, stay at same height
    fZ = m_pCamera->GetOrigin(Z) < v3Pos.z + 30.0f ? v3Pos.z + 30.0f : m_pCamera->GetOrigin(Z);
    fScale = fabs((fZ - v3Pos.z) / v3Forward.z);

    m_v3TargetCamPosition = v3Pos - v3Forward * fScale;
}


/*====================
  CEditor::SetCameraPosition
  ====================*/
void    CEditor::SetCameraPosition(const CVec3f &v3Pos)
{
    m_v3TargetCamPosition = v3Pos;
}


/*====================
  CEditor::SetCameraAngles
  ====================*/
void    CEditor::SetCameraAngles(const CVec3f &v3Angles)
{
    m_v3CamAngles = v3Angles;
}


/*====================
  CEditor::AdjustCameraPitch
  ====================*/
void    CEditor::AdjustCameraPitch(float fPitch)
{
    if (!MouseLook)
        return;

    m_v3CamAngles[PITCH] -= fPitch * le_pitchSpeed;
    m_v3CamAngles[PITCH] = CLAMP(m_v3CamAngles[PITCH], -89.9f, 89.9f);
    m_v3CamAngles[ROLL] = 0;
}


/*====================
  CEditor::AdjustCameraYaw
  ====================*/
void    CEditor::AdjustCameraYaw(float fYaw)
{
    if (!MouseLook)
        return;

    m_v3CamAngles[YAW] -= fYaw * le_yawSpeed;

    while (m_v3CamAngles[YAW] > 360.0f) m_v3CamAngles[YAW] -= 360.0f;
    while (m_v3CamAngles[YAW] < 0.0f) m_v3CamAngles[YAW] += 360.0f;

    m_v3CamAngles[ROLL] = 0;
}


/*====================
  CEditor::SetTextureScale
  ====================*/
void    CEditor::SetTextureScale(float fScale)
{
    m_pWorld->SetTextureScale(fScale);
}


/*====================
  CEditor::SetFancyName
  ====================*/
void    CEditor::SetFancyName(const tstring &sFancyName)
{
    m_pWorld->SetFancyName(sFancyName);
}


/*====================
  CEditor::UpdateImportedFiles
  ====================*/
void    CEditor::UpdateImportedFiles()
{
    ClearImportedFiles.Trigger(TSNULL);

    tsvector vFileList;

    GetWorld().GetImportedFiles(vFileList);

    for (tsvector_it it(vFileList.begin()); it != vFileList.end(); it++)
        ImportedFiles.Trigger(*it);
}


/*====================
  CEditor::UpdateScripts
  ====================*/
void    CEditor::UpdateScripts()
{
    ClearScriptList.Trigger(TSNULL);

    tsmapts &mapScripts(GetWorld().GetScriptMap());

    for (tsmapts::iterator it(mapScripts.begin()); it != mapScripts.end(); it++)
    {
        tstring sColorCode;

        if (it->second.empty())
            sColorCode = _T("^r");
        else
            sColorCode = _T("^g");

        ScriptList.Trigger(sColorCode + it->first);
    }
}


/*====================
  CEditor::UpdateMinimap
  ====================*/
void    CEditor::UpdateMinimap()
{
    PROFILE("CEditor::UpdateMinimap");

    CBufferFixed<40> buffer;

    Minimap.Execute(_T("clear"), buffer);

    buffer.Clear();
    buffer << m_pCamera->GetOrigin().x / m_pWorld->GetWorldWidth();
    buffer << 1.0f - (m_pCamera->GetOrigin().y / m_pWorld->GetWorldHeight());
    buffer << 0.016f; // Width
    buffer << 0.016f; // Height
    buffer << 1.0f; // Color R
    buffer << 1.0f; // Color G
    buffer << 1.0f; // Color B
    buffer << 1.0f; // Color A
    buffer << g_ResourceManager.GetWhiteTexture();
    buffer << uint(-1);
    Minimap.Execute(_T("icon"), buffer);

#if 0
    WorldEntList &vEntities(m_pWorld->GetEntityList());
    for (WorldEntList_cit cit(vEntities.begin()), citEnd(vEntities.end()); cit != citEnd; ++cit)
    {
        CWorldEntity *pEnt(m_pWorld->GetEntityByHandle(*cit));
        if (pEnt == NULL)
            continue;

        buffer.Clear();
        buffer << pEnt->GetPosition().x / m_pWorld->GetWorldWidth();
        buffer << 1.0f - (pEnt->GetPosition().y / m_pWorld->GetWorldHeight());
        buffer << 0.004f; // Width
        buffer << 0.004f; // Height
        buffer << 1.0f; // Color R
        buffer << 1.0f; // Color G
        buffer << 1.0f; // Color B
        buffer << 1.0f; // Color A
        buffer << g_ResourceManager.GetWhiteTexture();
        buffer << unit(-1);
        Minimap.Execute(_T("icon"), buffer);
    }
#endif
}


/*====================
  CEditor::RenderMinimapBitmap
  ====================*/
void    CEditor::RenderMinimapBitmap(CBitmap &cMinimap)
{
    map<ResHandle, CVec4b> mapColors;

    for (int iY(0); iY < cMinimap.GetHeight(); ++iY)
    {
        for (int iX(0); iX < cMinimap.GetWidth(); ++iX)
        {
            CVec3f v3Normal(Normalize(m_pWorld->GetTileNormal(iX, iY, TRIANGLE_LEFT) + m_pWorld->GetTileNormal(iX, iY, TRIANGLE_RIGHT)));

            CVec4b av4LayerColors[NUM_TERRAIN_LAYERS];

            for (int iLayer(0); iLayer < NUM_TERRAIN_LAYERS; ++iLayer)
            {
                ResHandle hTexture(m_pWorld->GetTileDiffuseTexture(iX, iY, iLayer));

                map<ResHandle, CVec4b>::iterator findit(mapColors.find(hTexture));
                if (findit == mapColors.end())
                {
                    CTexture *pTexture(g_ResourceManager.GetTexture(hTexture));

                    if (pTexture)
                    {
                        CVec4f v4Color(Vid.GetTextureColor(pTexture));

                        av4LayerColors[iLayer] = CVec4b(BYTE_ROUND(v4Color[R] * 255.0f), BYTE_ROUND(v4Color[G] * 255.0f), BYTE_ROUND(v4Color[B] * 255.0f), BYTE_ROUND(v4Color[A] * 255.0f));
                        mapColors[hTexture] = av4LayerColors[iLayer];
                    }
                    else
                    {
                        av4LayerColors[iLayer] = CVec4b(192, 192, 192, 255);
                        mapColors[hTexture] = av4LayerColors[iLayer];
                    }
                }
                else
                {
                    av4LayerColors[iLayer] = findit->second;
                }
            }

            int iTexelDensity(m_pWorld->GetTexelDensity());
            int iAlphamap(0);

            for (int iAlphaY(0); iAlphaY < iTexelDensity; ++iAlphaY)
                for (int iAlphaX(0); iAlphaX < iTexelDensity; ++iAlphaX)
                    iAlphamap += m_pWorld->GetTexelAlpha(iX * iTexelDensity + iAlphaX, iY * iTexelDensity + iAlphaY);

            if (iAlphamap != 0 && iTexelDensity != 0)
                iAlphamap /= (iTexelDensity * iTexelDensity);
            
            int iAlpha(INT_ROUND((float(m_pWorld->GetGridColor(iX, iY)[A]) +
                m_pWorld->GetGridColor(iX + 1, iY)[A] +
                m_pWorld->GetGridColor(iX, iY + 1)[A] +
                m_pWorld->GetGridColor(iX + 1, iY + 1)[A]) / 2.0f * (iAlphamap / 255.0f)));

            CVec3f v3PaintColor
            (
                float((m_pWorld->GetGridColor(iX, iY)[R] + m_pWorld->GetGridColor(iX + 1, iY)[R] + m_pWorld->GetGridColor(iX, iY + 1)[R] + m_pWorld->GetGridColor(iX + 1, iY + 1)[R]) / 4.0f) / 255.0f,
                float((m_pWorld->GetGridColor(iX, iY)[G] + m_pWorld->GetGridColor(iX + 1, iY)[G] + m_pWorld->GetGridColor(iX, iY + 1)[G] + m_pWorld->GetGridColor(iX + 1, iY + 1)[G]) / 4.0f) / 255.0f,
                float((m_pWorld->GetGridColor(iX, iY)[B] + m_pWorld->GetGridColor(iX + 1, iY)[B] + m_pWorld->GetGridColor(iX, iY + 1)[B] + m_pWorld->GetGridColor(iX + 1, iY + 1)[B]) / 4.0f) / 255.0f
            );

            CVec4b v4Color;
            v4Color[R] = byte(LERP(CLAMP(iAlpha / 255.0f, 0.0f, 1.0f), av4LayerColors[0][R], av4LayerColors[1][R]) * v3PaintColor[R]);
            v4Color[G] = byte(LERP(CLAMP(iAlpha / 255.0f, 0.0f, 1.0f), av4LayerColors[0][G], av4LayerColors[1][G]) * v3PaintColor[G]);
            v4Color[B] = byte(LERP(CLAMP(iAlpha / 255.0f, 0.0f, 1.0f), av4LayerColors[0][B], av4LayerColors[1][B]) * v3PaintColor[B]);
            v4Color[A] = LERP(CLAMP(iAlpha / 255.0f, 0.0f, 1.0f), av4LayerColors[0][A], av4LayerColors[1][A]);

            float fShade(DotProduct(v3Normal, CVec3f(0.0f, 0.0f, 1.0f)) * 0.90f);

            cMinimap.SetPixel4b(iX, iY,
                CLAMP(INT_ROUND(v4Color[R] * fShade), 0, 255),
                CLAMP(INT_ROUND(v4Color[G] * fShade), 0, 255),
                CLAMP(INT_ROUND(v4Color[B] * fShade), 0, 255),
                255);
        }
    }
}


/*====================
  CEditor::UpdateMinimapTexture
  ====================*/
void    CEditor::UpdateMinimapTexture()
{
    if (m_hMinimapTexture != INVALID_RESOURCE)
    {
        g_ResourceManager.Unregister(m_hMinimapTexture, UNREG_DELETE_HANDLE);
        m_hMinimapTexture = INVALID_RESOURCE;
    }

    // UTTAR
    SAFE_DELETE(m_pMinimapBitmap);
    m_pMinimapBitmap = K2_NEW(ctx_Editor,   CBitmap)(m_pWorld->GetTileWidth(), m_pWorld->GetTileWidth(), BITMAP_RGB);

    RenderMinimapBitmap(*m_pMinimapBitmap);

    m_hMinimapTexture = g_ResourceManager.Register(K2_NEW(ctx_Editor,   CTexture)(_T("*editor_minimap"), m_pMinimapBitmap, TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);
    g_ResourceManager.UpdateReference(m_hMinimapReference, m_hMinimapTexture);
}


/*====================
  CEditor::PathSetStart
  ====================*/
void    CEditor::PathSetStart()
{
    STraceInfo trace;
    if (TraceCursor(trace, TRACE_TERRAIN))
    {
        m_v2PathStart = trace.v3EndPos.xy();
        m_bPathStartValid = true;
    }

    if (m_bPathStartValid && m_bPathEndValid)
    {
        if (m_hPath != INVALID_POOL_HANDLE)
            m_pWorld->FreePath(m_hPath);

        m_hPath = m_pWorld->FindPath(m_v2PathStart, le_pathRadius, INVALID_INDEX, m_v2PathEnd, le_pathRange);

        Console << _T("Path ") << m_v2PathStart << _T(" ") << m_v2PathEnd << newl;
    }
}


/*====================
  CEditor::PathSetEnd
  ====================*/
void    CEditor::PathSetEnd()
{
    STraceInfo trace;
    if (TraceCursor(trace, TRACE_TERRAIN))
    {
        m_v2PathEnd = trace.v3EndPos.xy();
        m_bPathEndValid = true;
    }

    if (m_bPathStartValid && m_bPathEndValid)
    {
        if (m_hPath != INVALID_POOL_HANDLE)
            m_pWorld->FreePath(m_hPath);

        m_hPath = m_pWorld->FindPath(m_v2PathStart, le_pathRadius, INVALID_INDEX, m_v2PathEnd, le_pathRange);

        Console << _T("Path ") << m_v2PathStart << _T(" ") << m_v2PathEnd << newl;
    }
}


/*====================
  CEditor::PathClear
  ====================*/
void    CEditor::PathClear()
{
    if (m_hPath != INVALID_POOL_HANDLE)
    {
        m_pWorld->FreePath(m_hPath);
        m_hPath = INVALID_POOL_HANDLE;
    }

    m_bPathStartValid = false;
    m_bPathEndValid = false;
}


/*====================
  CEditor::PathSpam
  ====================*/
void    CEditor::PathSpam()
{
    for (int i(0); i<10000; ++i)
    {
        m_hPath = m_pWorld->FindPath(
            CVec2f(m_pWorld->GetWorldWidth() * ((float)rand()/SHRT_MAX),
            m_pWorld->GetWorldHeight() * ((float)rand()/SHRT_MAX)),
            26.0f,
            INVALID_INDEX,
            CVec2f(m_pWorld->GetWorldWidth() * ((float)rand()/SHRT_MAX),
            m_pWorld->GetWorldHeight() * ((float)rand()/SHRT_MAX)),
            0.0f
            );

        m_pWorld->FreePath(m_hPath);
    }
}


/*====================
  CEditor::Path
  ====================*/
void    CEditor::Path(const CVec2f &v2Start, const CVec2f &v2End)
{
    if (m_hPath != INVALID_POOL_HANDLE)
        m_pWorld->FreePath(m_hPath);

    m_v2PathStart = v2Start;
    m_v2PathEnd = v2End;

    m_hPath = m_pWorld->FindPath(m_v2PathStart, le_pathRadius, INVALID_INDEX, m_v2PathEnd, le_pathRange);
}


/*====================
  CEditor::DebugRender
  ====================*/
void    CEditor::DebugRender()
{
    if (m_hPath == INVALID_POOL_HANDLE)
        return;

    CPath *pPath(m_pWorld->AccessPath(m_hPath));

    // Unsmoothed path
    if (pPath)
    {
        PathResult &vecInfoPath(pPath->GetSimpleResult());
        uint uiCount(uint(vecInfoPath.size()));

        for (uint i(0); i + 1 < uiCount; ++i)
        {
            CVec2f v2Src(vecInfoPath[i].GetPath());
            CVec2f v2End(vecInfoPath[i + 1].GetPath());

#if 1 // SHOW_GATES
            CVec2f v2PosGate, v2NegGate;
            CVec2f v2Dir(0.0f, 0.0f);

            switch (vecInfoPath[i].Direction())
            {
            case SD_NORTH: v2Dir = CVec2f(-1.0f, 0.0f); break;
            case SD_EAST: v2Dir = CVec2f(0.0f, 1.0f); break;
            case SD_SOUTH: v2Dir = CVec2f(1.0f, 0.0f); break;
            case SD_WEST: v2Dir = CVec2f(0.0f, -1.0f); break;
            }

            v2PosGate = v2Dir * vecInfoPath[i].GetRadiusPositive();
            v2PosGate += v2Src;

            v2NegGate = v2Dir * -vecInfoPath[i].GetRadiusNegative();
            v2NegGate += v2Src;

            if (vecInfoPath[i].Direction() != SD_INVALID)
            {
                DebugRenderer.AddLine(
                    CVec3f(v2PosGate.x, v2PosGate.y, m_pWorld->GetTerrainHeight(v2Src.x, v2Src.y)),
                    CVec3f(v2NegGate.x, v2NegGate.y, m_pWorld->GetTerrainHeight(v2Src.x, v2Src.y)), 
                    vecInfoPath[i].GetColor());
            }
#endif
            {
                CVec4f v4PathLineColor(0.0f, 1.0f, 0.0f, 1.0f);

                DebugRenderer.AddLine(
                    CVec3f(v2Src.x, v2Src.y, m_pWorld->GetTerrainHeight(v2Src.x, v2Src.y)),
                    CVec3f(v2End.x, v2End.y, m_pWorld->GetTerrainHeight(v2End.x, v2End.y)),
                    v4PathLineColor);
            }
        }
    }

    // Smoothed path
    if (pPath)
    {
        PathResult &vecPath(pPath->GetSmoothResult());
        uint uiCount(uint(vecPath.size()));

        for (uint uiIndex(0); uiIndex + 1 < uiCount; ++uiIndex)
        {
            CVec2f v2Src(vecPath[uiIndex].GetPath());
            CVec2f v2Dst(vecPath[uiIndex + 1].GetPath());

            DebugRenderer.AddLine(
                CVec3f(v2Src.x, v2Src.y, m_pWorld->GetTerrainHeight(v2Src.x, v2Src.y)),
                CVec3f(v2Dst.x, v2Dst.y, m_pWorld->GetTerrainHeight(v2Dst.x, v2Dst.y)),
                CVec4f(1.0f, 1.0f, 1.0f, 1.0f));
        }
    }
}


/*====================
  CEditor::DrawNavGrid
  ====================*/
void    CEditor::DrawNavGrid()
{
    if (!le_drawNavGrid)
        return;

    CNavigationMap &cNavigationMap(m_pWorld->GetNavigationMap());

    CNavGridZ *pNavGrid(cNavigationMap.PrepForSearch(uint(-1), le_drawNavGridDownsize));

    uint *pHorizontal(pNavGrid->GetHorizontal());
    uint *pVertical(pNavGrid->GetVertical());

    uint uiNavigationWidth(pNavGrid->GetWidth());
    uint uiNavigationHeight(pNavGrid->GetHeight());
    uint uiIntsPerRow(pNavGrid->GetIntsPerRow());

    STraceInfo trace;

    if (!TraceCursor(trace, TRACE_TERRAIN))
        return;

    float fGateScale(m_pWorld->GetNavigationScale());
    float fTileScale(float(1 << m_pWorld->GetNavigationSize() >> m_pWorld->GetSize() << pNavGrid->GetDownSize()) * m_pWorld->GetScale());
    int iDisplaySize(le_pathDisplaySize);
    int iDisplaySize2(int(le_pathDisplaySize) << pNavGrid->GetDownSize());

    uint uiHorizontalStartX(CLAMP(INT_ROUND(trace.v3EndPos[X] / fGateScale) - iDisplaySize2, 0, int(uiIntsPerRow * 32)));
    uint uiHorizontalStartY(CLAMP(INT_ROUND(trace.v3EndPos[Y] / fTileScale) - iDisplaySize, 0, int(uiNavigationHeight)));
    uint uiHorizontalEndX(CLAMP(INT_ROUND(trace.v3EndPos[X] / fGateScale) + iDisplaySize2, 0, int(uiIntsPerRow * 32)));
    uint uiHorizontalEndY(CLAMP(INT_ROUND(trace.v3EndPos[Y] / fTileScale) + iDisplaySize, 0, int(uiNavigationHeight) - 1));

    SSceneFaceVert poly[1024];
    MemManager.Set(poly, 0, sizeof(poly));
    int p(0);

    // Horizontal gates
    for (uint uiY(uiHorizontalStartY); uiY != uiHorizontalEndY; ++uiY)
    {
        float fY((uiY + 1) * fTileScale);

        for (uint uiX(uiHorizontalStartX); uiX != uiHorizontalEndX; ++uiX)
        {
            // Restart batch if we overflow
            if (p >= 1024)
            {
                SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
                MemManager.Set(poly, 0, sizeof(poly));
                p = 0;
            }

            float fX0((uiX) * fGateScale);
            float fX1((uiX + 1) * fGateScale);
            
            float fZ0(m_pWorld->GetTerrainHeight(fX0, fY));
            float fZ1(uiX + 1 != uiNavigationWidth ? m_pWorld->GetTerrainHeight(fX1, fY) : m_pWorld->GetTerrainHeight(fX1 - 0.001f, fY));

            if (pHorizontal[uiY * uiIntsPerRow + (uiX >> 5)] & BIT(31 - (uiX & 31)))
            {
                poly[p].vtx[0] = fX0;
                poly[p].vtx[1] = fY;
                poly[p].vtx[2] = fZ0;
                SET_VEC4(poly[p].col, 255, 255, 255, 255);
                ++p;

                poly[p].vtx[0] = fX1;
                poly[p].vtx[1] = fY;
                poly[p].vtx[2] = fZ1;
                SET_VEC4(poly[p].col, 255, 255, 255, 255);
                ++p;
            }
            else
            {
                poly[p].vtx[0] = fX0;
                poly[p].vtx[1] = fY;
                poly[p].vtx[2] = fZ0;
                SET_VEC4(poly[p].col, 255, 0, 0, 255);
                ++p;

                poly[p].vtx[0] = fX1;
                poly[p].vtx[1] = fY;
                poly[p].vtx[2] = fZ1;
                SET_VEC4(poly[p].col, 255, 0, 0, 255);
                ++p;
            }
        }
    }

    uint uiVerticalStartX(CLAMP(INT_ROUND(trace.v3EndPos[X] / fTileScale) - iDisplaySize, 0, int(uiNavigationWidth)));
    uint uiVerticalStartY(CLAMP(INT_ROUND(trace.v3EndPos[Y] / fGateScale) - iDisplaySize2, 0, int(uiIntsPerRow * 32)));
    uint uiVerticalEndX(CLAMP(INT_ROUND(trace.v3EndPos[X] / fTileScale) + iDisplaySize, 0, int(uiNavigationWidth) - 1));
    uint uiVerticalEndY(CLAMP(INT_ROUND(trace.v3EndPos[Y] / fGateScale) + iDisplaySize2, 0, int(uiIntsPerRow * 32)));

    // Vertical gates
    for (uint uiX(uiVerticalStartX); uiX != uiVerticalEndX; ++uiX)
    {
        float fX((uiX + 1) * fTileScale);

        for (uint uiY(uiVerticalStartY); uiY != uiVerticalEndY; ++uiY)
        {
            // Restart batch if we overflow
            if (p >= 1024)
            {
                SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
                MemManager.Set(poly, 0, sizeof(poly));
                p = 0;
            }

            float fY0((uiY) * fGateScale);
            float fY1((uiY + 1) * fGateScale);
            
            float fZ0(m_pWorld->GetTerrainHeight(fX, fY0));
            float fZ1(uiY + 1 != uiNavigationWidth ? m_pWorld->GetTerrainHeight(fX, fY1) : m_pWorld->GetTerrainHeight(fX, fY1 - 0.001f));

            if (pVertical[uiX * uiIntsPerRow + (uiY >> 5)] & BIT(31 - (uiY & 31)))
            {
                poly[p].vtx[0] = fX;
                poly[p].vtx[1] = fY0;
                poly[p].vtx[2] = fZ0;
                SET_VEC4(poly[p].col, 255, 255, 255, 255);
                ++p;

                poly[p].vtx[0] = fX;
                poly[p].vtx[1] = fY1;
                poly[p].vtx[2] = fZ1;
                SET_VEC4(poly[p].col, 255, 255, 255, 255);
                ++p;
            }
            else
            {
                poly[p].vtx[0] = fX;
                poly[p].vtx[1] = fY0;
                poly[p].vtx[2] = fZ0;
                SET_VEC4(poly[p].col, 255, 0, 0, 255);
                ++p;

                poly[p].vtx[0] = fX;
                poly[p].vtx[1] = fY1;
                poly[p].vtx[2] = fZ1;
                SET_VEC4(poly[p].col, 255, 0, 0, 255);
                ++p;
            }
        }
    }

    // Draw remaining polys
    if (p > 0)
    {
        SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
        p = 0;
    }
}


/*====================
  CEditor::RulerStart
  ====================*/
void    CEditor::RulerStart()
{
    STraceInfo cTrace;
    if (TraceCursor(cTrace, TRACE_TERRAIN))
    {
        m_v3RulerStart = cTrace.v3EndPos;
        m_bRuler = true;
    }
}


/*====================
  CEditor::RulerEnd
  ====================*/
void    CEditor::RulerEnd()
{
    m_bRuler = false;
    m_vRulerPoints.clear();

}



