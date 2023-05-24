// (C)2023 S3 Games
// c_gameclient.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "bet_client_common.h"

#include "../k2/c_hostclient.h"
#include "../k2/c_uimanager.h"
#include "../k2/c_input.h"
#include "../k2/c_inputstate.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_DebugRenderer.h"
#include "../k2/c_actionregistry.h"
#include "../k2/c_resourcemanager.h"
#include "../k2/c_skeleton.h"
#include "../k2/c_model.h"
#include "../k2/c_vid.h"
#include "../k2/c_camera.h"
#include "../k2/c_soundmanager.h"
#include "../k2/c_sample.h"
#include "../k2/c_draw2d.h"
#include "../k2/c_fontmap.h"

#include "c_gameclient.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
CVAR_FLOAT  (bet_camSpeed,               1000.0f);
CVAR_FLOAT  (bet_camFov,                 53.75f);
CVAR_FLOAT  (bet_camAspect,              1.515f);
CVAR_FLOAT  (bet_camWeightX,             1.0f);
CVAR_FLOAT  (bet_camWeightY,             3.0f);
CVAR_FLOAT  (bet_shiftCameraDistance,    125.0f);
CVAR_FLOATF (bet_yawSpeed,               0.5f,               CVAR_SAVECONFIG);
CVAR_FLOATF (bet_pitchSpeed,             0.5f,               CVAR_SAVECONFIG);
CVAR_BOOL   (bet_drawEntities,           true);
CVAR_BOOL   (bet_camDebug,               false);
CVAR_STRING (bet_music,                  _T("/music/HoleInOne.mp3"));

INPUT_STATE_BOOL(MouseLook);

INPUT_STATE_BOOL(MoveForward);
INPUT_STATE_BOOL(MoveBack);
INPUT_STATE_BOOL(MoveLeft);
INPUT_STATE_BOOL(MoveRight);
INPUT_STATE_BOOL(MoveUp);
INPUT_STATE_BOOL(MoveDown);

INPUT_STATE_BOOL(Start);

EXTERN_CVAR_FLOAT(scene_nearClip);
EXTERN_CVAR_FLOAT(scene_farClip);
//=============================================================================


/*====================
  CWorldEntityEx::~CWorldEntityEx
  ====================*/
CWorldEntityEx::~CWorldEntityEx()
{
    SAFE_DELETE(m_pSkeleton);
}


/*====================
  CWorldEntityEx::CWorldEntityEx
  ====================*/
CWorldEntityEx::CWorldEntityEx()
: m_pSkeleton(nullptr)
{
}


/*====================
  CGameClient::~CGameClient
  ====================*/
CGameClient::~CGameClient()
{
    SAFE_DELETE(m_pCamera);
    Console << _T("Beatopia GameClient released") << newl;
}


/*====================
  CGameClient::CGameClient
  ====================*/
CGameClient::CGameClient()
: IGame(_T("client"))
, m_pCamera(nullptr)
, m_v3CamAngles(-45.0f, 0.0f, 315.0f)
{
    scene_nearClip = 0.1f;
    scene_farClip = 30000.0f;
}


/*====================
  CGameClient::Init
  ====================*/
bool CGameClient::Init(CHostClient *pHostClient)
{
    PROFILE("CGameClient::Init");

    try
    {
        SetGamePointer();

        m_pHostClient = pHostClient;
        if (m_pHostClient == nullptr)
            EX_ERROR(_T("Inalid CHostClient"));

        m_pWorld = m_pHostClient->GetWorld();
        if (m_pWorld == nullptr)
            EX_ERROR(_T("Invalid CWorld from host"));

        m_pWorld->SetHostType(WORLDHOST_CLIENT);

        m_v3TargetCamPosition = CVec3f(1000.0f, 1000.0f, 1000.0f);
        m_pCamera = K2_NEW(ctx_Editor,   CCamera);
        m_pCamera->DefaultCamera(float(Vid.GetScreenW()), float(Vid.GetScreenH()));
        m_pCamera->SetOrigin(m_v3TargetCamPosition);
        m_pCamera->AddFlags(CAM_NO_DEPTH_CLEAR);
        m_pCamera->SetWorld(m_pWorld);

        int iSoundFlags(SND_2D | SND_COMPRESSED | SND_STREAM);
        ResHandle hSound(g_ResourceManager.Register(K2_NEW(ctx_Client,   CSample)(bet_music, iSoundFlags), RES_SAMPLE));
        if (hSound == INVALID_INDEX)
            EX_ERROR(_T("Invalid music"));
        m_hMusic = hSound;

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
#if TKTK
        ActionRegistry.BindButton(BINDTABLE_GAME, BUTTON_MOUSEM, BIND_MOD_NONE,     _T("MouseLook"));
#else
        ActionRegistry.BindButton(BINDTABLE_GAME, BUTTON_TAB, BIND_MOD_NONE,     _T("MouseLook"));
#endif

        ActionRegistry.BindImpulse(BINDTABLE_GAME, BUTTON_MISC4, BIND_MOD_NONE,     _T("PathSetStart"));
        ActionRegistry.BindImpulse(BINDTABLE_GAME, BUTTON_MISC6, BIND_MOD_NONE,     _T("PathSetEnd"));
        ActionRegistry.BindImpulse(BINDTABLE_GAME, BUTTON_MISC5, BIND_MOD_NONE,     _T("PathClear"));

        ActionRegistry.BindImpulse(BINDTABLE_GAME, BUTTON_ESC, BIND_MOD_NONE,       _T("Cancel"));
        ActionRegistry.BindImpulse(BINDTABLE_GAME, BUTTON_DEL, BIND_MOD_NONE,       _T("Delete"));

        ActionRegistry.BindImpulse(BINDTABLE_GAME, BUTTON_WHEELUP, BIND_MOD_NONE,   _T("MoveIn"));
        ActionRegistry.BindImpulse(BINDTABLE_GAME, BUTTON_WHEELDOWN, BIND_MOD_NONE, _T("MoveOut"));

        ActionRegistry.BindButton(BINDTABLE_GAME, EButton('F'), BIND_MOD_NONE,      _T("ShowScale"));

        ActionRegistry.BindButton(BINDTABLE_GAME, EButton('R'), BIND_MOD_NONE,      _T("Ruler"));

        ActionRegistry.BindImpulse(BINDTABLE_GAME, BUTTON_BACKSPACE, BIND_MOD_NONE, _T("Cmd"), _T("Center"));
        ActionRegistry.BindImpulse(BINDTABLE_GAME, BUTTON_SPACE,     BIND_MOD_NONE, _T("Cmd"), _T("Start"));

#if 0
        m_hMinimapReference = g_ResourceManager.Register(_T("!minimap_texture"), RES_REFERENCE);
        g_ResourceManager.UpdateReference(m_hMinimapReference, g_ResourceManager.GetBlackTexture());

        tsvector vFileList;
        FileManager.GetFileList(_T("/"), _T("*.entity"), true, vFileList);
        for (tsvector_it it(vFileList.begin()); it != vFileList.end(); ++it)
            g_ResourceManager.Register(*it, RES_ENTITY_DEF);
#endif

        {
            PROFILE("Load interface");
            // Load interface
            UIManager.LoadInterface(_T("/ui/main.interface"));
            UIManager.SetActiveInterface(_T("main"));
        }

        Console.ExecuteScript(_T("/init.cfg"));

        // Start with a blank world
        m_pWorld->New(_T("default"), 9, 32.0f, 4, 16.0f);

#if 0
        SetNullTiles();

        UpdateMinimapTexture();
        UpdateScripts();
        UpdateImportedFiles();
#endif

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CGameClient::Init() - "), NO_THROW);
        return false;
    }
}

/*====================
  CGameClient::Start
  ====================*/
void    CGameClient::Start()
{
    if (CSample* pSample = g_ResourceManager.GetSample(m_hMusic); pSample)
    {
        m_hMusicPlaying = K2SoundManager.Play2DSound(pSample, 1.0f, CHANNEL_MUSIC);
    }
}

/*====================
  CGameClient::UpdateCamera
  ====================*/
void    CGameClient::UpdateCamera()
{
    m_pCamera->SetTime(Host.GetTime() / 1000.0f);
    m_pCamera->SetAngles(m_v3CamAngles);
    m_pCamera->AddFlags(CAM_NO_CLIFFS);

    if (bet_camDebug)
    {
        Console << _T("Origin: ") << m_pCamera->GetOrigin() << newl;
        Console << _T("FrameTime: ") << GameClient.GetFrameSeconds() << newl;
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
        vVelocity += vForward * bet_camSpeed;
    if (MoveBack)
        vVelocity += vForward * -bet_camSpeed;
    if (MoveLeft)
        vVelocity += vecRight * -bet_camSpeed;
    if (MoveRight)
        vVelocity += vecRight * bet_camSpeed;
    if (MoveUp)
        vVelocity += v3Up * bet_camSpeed;
    if (MoveDown)
        vVelocity += v3Up * -bet_camSpeed;

    m_v3TargetCamPosition += vVelocity * GameClient.GetFrameSeconds();

    if (m_fCameraShift != 0.0f)
    {
        m_v3TargetCamPosition += m_pCamera->GetViewAxis(FORWARD) * m_fCameraShift * bet_shiftCameraDistance;
        m_fCameraShift = 0.0f;
    }

    CVec3f v3CamPosition(m_pCamera->GetOrigin());

    m_pCamera->SetOrigin(DECAY(m_pCamera->GetOrigin(), m_v3TargetCamPosition, 0.1f, GameClient.GetFrameSeconds()));

    m_pCamera->SetWidth(float(Vid.GetScreenW()));
    m_pCamera->SetHeight(float(Vid.GetScreenH()));

    m_pCamera->SetFovFromAspect(bet_camFov, bet_camAspect, bet_camWeightX, bet_camWeightY);

    m_pCamera->SetReflect(CPlane(V_UP, CVec3f(0.0f, 0.0f, -92.85f)));
}


/*====================
  CGameClient::CenterCamera
  ====================*/
void    CGameClient::CenterCamera(const CVec3f &v3Pos)
{
    float fZ, fScale;
    const CVec3f &v3Forward = m_pCamera->GetViewAxis(FORWARD);

    // move above the point if we are below it; otherwise, stay at same height
    fZ = m_pCamera->GetOrigin(Z) < v3Pos.z + 30.0f ? v3Pos.z + 30.0f : m_pCamera->GetOrigin(Z);
    fScale = fabs((fZ - v3Pos.z) / v3Forward.z);

    m_v3TargetCamPosition = v3Pos - v3Forward * fScale;
}


/*====================
  CGameClient::SetCameraPosition
  ====================*/
void    CGameClient::SetCameraPosition(const CVec3f &v3Pos)
{
    m_v3TargetCamPosition = v3Pos;
}


/*====================
  CGameClient::SetCameraAngles
  ====================*/
void    CGameClient::SetCameraAngles(const CVec3f &v3Angles)
{
    m_v3CamAngles = v3Angles;
}


/*====================
  CGameClient::AdjustCameraPitch
  ====================*/
void    CGameClient::AdjustCameraPitch(float fPitch)
{
    if (!MouseLook)
        return;

    m_v3CamAngles[PITCH] -= fPitch * bet_pitchSpeed;
    m_v3CamAngles[PITCH] = CLAMP(m_v3CamAngles[PITCH], -89.9f, 89.9f);
    m_v3CamAngles[ROLL] = 0;
}


/*====================
  CGameClient::AdjustCameraYaw
  ====================*/
void    CGameClient::AdjustCameraYaw(float fYaw)
{
    if (!MouseLook)
        return;

    m_v3CamAngles[YAW] -= fYaw * bet_yawSpeed;

    while (m_v3CamAngles[YAW] > 360.0f) m_v3CamAngles[YAW] -= 360.0f;
    while (m_v3CamAngles[YAW] < 0.0f) m_v3CamAngles[YAW] += 360.0f;

    m_v3CamAngles[ROLL] = 0;
}


/*====================
  CGameClient::GetLookAtPoint
  ====================*/
bool     CGameClient::GetLookAtPoint(CVec3f &v3Pos)
{
    STraceInfo result;

    CVec3f v3Dir(GetCamera().GetViewAxis(FORWARD));
    CVec3f v3End(M_PointOnLine(GetCamera().GetOrigin(), v3Dir, FAR_AWAY));

    if (GetWorld().TraceLine(result, m_pCamera->GetOrigin(), v3End, TRACE_TERRAIN))
    {
        v3Pos = result.v3EndPos;
        return true;
    }

    return false;
}


/*====================
  CGameClient::LoadWorld
  ====================*/
bool    CGameClient::LoadWorld(const tstring &sWorldName)
{
    try
    {
        if (!m_pWorld->StartLoad(sWorldName))
            EX_ERROR(_T("World load failed"));

        while (m_pWorld->IsLoading())
            if (!m_pWorld->LoadNextComponent())
                EX_ERROR(_T("World load failed"));

        m_mapWorldEntData.clear();

        m_pWorld->AnalyzeTerrain();

        WorldEntList &vEntities(m_pWorld->GetEntityList());
        for (WorldEntList_cit cit(vEntities.begin()), citEnd(vEntities.end()); cit != citEnd; ++cit)
        {
            CWorldEntity *pWorldEnt(m_pWorld->GetEntityByHandle(*cit));
            if (pWorldEnt == nullptr)
                continue;

            pWorldEnt->SetModelHandle(g_ResourceManager.Register(pWorldEnt->GetModelPath(), RES_MODEL));
            pWorldEnt->SetSkin(g_ResourceManager.GetSkin(pWorldEnt->GetModelHandle(), pWorldEnt->GetSkinName()));

            tstring sType(pWorldEnt->GetType());
            if (sType == _T("Entity_NeutralCampSpawner"))
                sType = pWorldEnt->GetProperty(_T("target0"));

            Console.Script << sType << newl;

#if 0
            IUnitDefinition *pDefinition(EntityRegistry.GetDefinition<IUnitDefinition>(sType));
            if (pDefinition != nullptr)
            {
                pWorldEnt->SetScale(pDefinition->GetPreGlobalScale(0) * pDefinition->GetModelScale(0));
                pWorldEnt->SetModelHandle(g_ResourceManager.Register(pDefinition->GetModelPath(0), RES_MODEL));
                pWorldEnt->SetSkin(g_ResourceManager.GetSkin(pWorldEnt->GetModelHandle(), pDefinition->GetSkin(0)));
            }
#endif

            g_ResourceManager.PrecacheSkin(pWorldEnt->GetModelHandle(), pWorldEnt->GetSkin());

            m_pWorld->LinkEntity(pWorldEnt->GetIndex(), LINK_SURFACE | LINK_MODEL, SURF_PROP);

            map<uint, CWorldEntityEx>::iterator findit(m_mapWorldEntData.find(pWorldEnt->GetIndex()));

            if (findit == m_mapWorldEntData.end())
            {
                m_mapWorldEntData.insert(pair<uint, CWorldEntityEx>(pWorldEnt->GetIndex(), CWorldEntityEx()));
                findit = m_mapWorldEntData.find(pWorldEnt->GetIndex());
            }

            const vector<CConvexPolyhedron> &cWorldSurfs(pWorldEnt->GetWorldSurfsRef());
            for (vector<CConvexPolyhedron>::const_iterator cit(cWorldSurfs.begin()); cit != cWorldSurfs.end(); ++cit)
                m_pWorld->BlockPath(findit->second.GetPathBlockers(), NAVIGATION_UNIT, *cit, 24.0f);

            // Hack for tree blockers
            if (pWorldEnt->GetType() == _T("Prop_Tree"))
                findit->second.GetPathBlockers().push_back(m_pWorld->BlockPath(NAVIGATION_UNIT, pWorldEnt->GetPosition().xy() - CVec2f(50.0f), 100.0f, 100.0f));
        }
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CGameClient::LoadWorld() - "), NO_THROW);
        return false;
    }
}



/*====================
  CGameClient::Frame
  ====================*/
void    CGameClient::Frame()
{
    PROFILE("CGameClient::Frame");

    UIManager.SetActiveInterface(_T("main"));

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

#if 0
    AddOccluders();
    DrawOccluders();
#endif
    DrawEntities();
#if 0
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
#endif

    SceneManager.Render();

#if 0
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

    DebugRender();
#endif
    if (K2SoundManager.IsHandleActive(m_hMusicPlaying))
    {
        uint uiPos = K2SoundManager.GetTime(m_hMusicPlaying);
        uint uiLen = K2SoundManager.GetDuration(m_hMusicPlaying);
        if (uiPos != INVALID_TIME && uiLen != INVALID_TIME)
        {
            CVec2f  v2ScreenPos(0.0f, 0.0f);
            ResHandle hFont(g_ResourceManager.LookUpName(_T("system_large"), RES_FONTMAP));
            auto *pFontMap = g_ResourceManager.GetFontMap(hFont);
            if (pFontMap != nullptr)
            {
                tstring sString(XtoA(100.0f * float(uiPos) / float(uiLen), 0, 2, 2) + _T("%"));
                float fWidth(pFontMap->GetStringWidth(sString));
                Draw2D.SetColor(BLACK);
                Draw2D.String(floor(v2ScreenPos.x) + 4.0f, floor(v2ScreenPos.y) + 4.0f, fWidth, pFontMap->GetMaxHeight(), sString, hFont);
                Draw2D.SetColor(WHITE);
                Draw2D.String(floor(v2ScreenPos.x) + 6.0f, floor(v2ScreenPos.y) + 2.0f, fWidth, pFontMap->GetMaxHeight(), sString, hFont);
            }
        }
    }

    DebugRenderer.Frame(m_pCamera);

#if 0
    UpdateMinimap();
#endif
}


/*====================
  CGameClient::DrawEntities
  ====================*/
void    CGameClient::DrawEntities()
{
    PROFILE("CGameClient::DrawEntities");

    if (!bet_drawEntities)
        return;

    CSceneEntity sc;

    WorldEntList &vEntities(m_pWorld->GetEntityList());
    for (WorldEntList_cit cit(vEntities.begin()), citEnd(vEntities.end()); cit != citEnd; ++cit)
    {
        CWorldEntity *pEnt(m_pWorld->GetEntityByHandle(*cit));
        if (pEnt == nullptr)
            continue;

        uint uiIndex(pEnt->GetIndex());

        sc.Clear();
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
        if (pModel != nullptr)
        {
            sc.flags |= SCENEENT_USE_AXIS;
            sc.axis = CAxis(sc.angle);

            CBBoxf bbBounds(pModel->GetBounds());
            bbBounds.Transform(sc.GetPosition(), sc.axis, sc.scale);

            sc.bounds = bbBounds;
            sc.flags |= SCENEENT_USE_BOUNDS;
        }

        SSceneEntityEntry &cEntry(SceneManager.AddEntity(sc));

        if (pModel != nullptr && pModel->GetModelFile() && pModel->GetModelFile()->GetType() == MODEL_K2 && (!cEntry.bCull || !cEntry.bCullShadow))
        {
            CK2Model *pK2Model(static_cast<CK2Model *>(pModel->GetModelFile()));

            if (pK2Model->GetNumAnims() > 0)
            {
                map<uint, CWorldEntityEx>::iterator findit(m_mapWorldEntData.find(pEnt->GetIndex()));

                if (findit == m_mapWorldEntData.end())
                {
                    m_mapWorldEntData.insert(pair<uint, CWorldEntityEx>(pEnt->GetIndex(), CWorldEntityEx()));
                    findit = m_mapWorldEntData.find(pEnt->GetIndex());
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

                pSkeleton->ClearEvents();
            }
        }
    }
}


ResHandle CGameClient::RegisterModel(const tstring &sPath)
{
    K2_UNREACHABLE();
    return 0;
}


ResHandle CGameClient::RegisterEffect(const tstring &sPath)
{
    K2_UNREACHABLE();
    return 0;
}


ResHandle CGameClient::RegisterIcon(const tstring &sPath)
{
    K2_UNREACHABLE();
    return 0;
}


void CGameClient::Precache(const tstring &sName, EPrecacheScheme eScheme, const tstring &sModifier)
{
    K2_UNREACHABLE();
}


void CGameClient::Precache(ushort unType, EPrecacheScheme eScheme, const tstring &sModifier)
{
    K2_UNREACHABLE();
}


CStateString &CGameClient::GetStateString(uint uiID)
{
    K2_UNREACHABLE();
    static CStateString ss;
    return ss;
}


CStateBlock &CGameClient::GetStateBlock(uint uiID)
{
    K2_UNREACHABLE();
    static CStateBlock block;
    return block;
}


uint CGameClient::GetServerFrame()
{
    K2_UNREACHABLE();
    return 0;
}


uint CGameClient::GetServerTime() const
{
    K2_UNREACHABLE();
    return 0;
}


uint CGameClient::GetPrevServerTime()
{
    K2_UNREACHABLE();
    return 0;
}


uint CGameClient::GetServerFrameLength()
{
    K2_UNREACHABLE();
    return 0;
}
