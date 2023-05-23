// (C)2005 S2 Games
// mv_main.cpp
//
// Modelviewer
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "mv_common.h"

#include "mv.h"

#include "../k2/c_hostclient.h"
#include "../k2/c_world.h"
#include "../k2/c_vid.h"
#include "../k2/c_input.h"
#include "../k2/c_action.h"
#include "../k2/c_inputstate.h"
#include "../k2/c_actionregistry.h"
#include "../k2/c_boundingbox.h"
#include "../k2/c_convexhull.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_skeleton.h"
#include "../k2/c_host.h"
#include "../k2/c_effect.h"
#include "../k2/c_effectthread.h"
#include "../k2/c_particlesystem.h"
#include "../k2/c_eventscript.h"
#include "../k2/c_clientgamelib.h"
#include "../k2/c_eventcmd.h"
#include "../k2/c_draw2d.h"
#include "../k2/intersection.h"
#include "../k2/c_soundmanager.h"
#include "../k2/c_texture.h"
#include "../k2/c_sceneentitymodifier.h"
#include "../k2/c_model.h"
#include "../k2/c_k2model.h"
#include "../k2/c_mesh.h"
#include "../k2/c_sample.h"
#include "../k2/c_uimanager.h"
#include "../k2/c_interface.h"
#include "../k2/c_combobox.h"
#include "../k2/c_widgetstyle.h"
#include "../k2/c_listitem.h"
#include "../k2/c_label.h"
#include "../k2/c_uicmd.h"
#include "../k2/c_eventmanager.h"
#include "../k2/c_function.h"
#include "../k2/c_bitmap.h"
#include "../k2/c_xmldoc.h"
#include "../k2/c_bitmapresource.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
//#define PATH_ANALYZER

CVAR_FLOAT(mv_camYaw, 180.0f);
CVAR_FLOAT(mv_camPitch, 0.0f);
CVAR_FLOAT(mv_camRoll, 0.0f);
CVAR_FLOAT(mv_camFov, 90.0f);

CVAR_FLOAT(mv_camDistance, 100.0f);

CVAR_BOOL(cam_freeLook, false);
CVAR_FLOAT(mv_camSpeed, 3000.0f);

CVAR_BOOL(mv_selectionRing, false);
CVAR_FLOAT(mv_selectionRingSize, 48.0f);

CVAR_STRING(model, "");
CVAR_FLOAT(model_x, 2048.0f);
CVAR_FLOAT(model_y, 2048.0f);
CVAR_FLOAT(model_z, 0.0f);
CVAR_FLOAT(model_yaw, 0.0f);
CVAR_FLOAT(model_yawSpeed, 0.0f);
CVAR_FLOAT(model_pitch, 0.0f);
CVAR_FLOAT(model_roll, 0.0f);
CVAR_FLOAT(model_scale, 1.0f);
CVAR_FLOAT(model_effectScale, 1.0f);
CVAR_VEC4(model_color, CVec4f(1.0f, 1.0f, 1.0f, 1.0f));
CVAR_VEC3(model_teamColor, CVec3f(0.0f, 0.0f, 1.0f));
CVAR_VEC4(model_param, CVec4f(0.0f, 0.0f, 0.0f, 0.0f));
CVAR_FLOAT(model_circleSpeed, 0.0f);

CVAR_INT(model_numX, 1);
CVAR_INT(model_numY, 1);
CVAR_INT(model_numZ, 1);

CVAR_STRING(anim, "");

CVAR_VEC3(test_dir, CVec3f(0.0f, 0.2f, 1.0f));
CVAR_VEC3(test_point, CVec3f(10.0f, 0.0f, 100.0f));

CVAR_STRING(material, "");
CVAR_STRING(skin, "default");

CVAR_BOOL   (mv_poseCharacter,      false);
CVAR_BOOL   (mv_poseObject,         true);
CVAR_FLOAT  (mv_animateSpeed,       1.0f);
CVAR_BOOL   (mv_fogofWar,           false);

CVAR_VEC3F  (mv_bgColor,            CVec3f(0.2f, 0.2f, 0.2f),       CVAR_SAVECONFIG);
CVAR_BOOL   (mv_ground,             false);
CVAR_STRING (mv_groundMap,          "ground");
CVAR_INTF   (mv_animFrame,          0,      CVAR_READONLY);
CVAR_INTF   (mv_animTime,           0,      CVAR_READONLY);
CVAR_FLOAT  (mv_animFraction,       0.0f);
CVAR_INTF   (mv_activeEffects,      0,      CVAR_READONLY);

CVAR_STRING (mv_interface,          "mv_main");

SMv mv;
mvres_t res;

CVec3f  g_vCamOrigin(0.0f, 0.0f, 0.0f);
CVec3f  g_vCamLookAt(0.0f, 0.0f, 0.0f);

CSkeleton   g_Skeleton;
CBBoxf      g_bbModelBounds;

ResHandle           g_hEffect(INVALID_RESOURCE);

bool    g_bShowInterface(true);

int     g_iNumPoints;
CVec2f  *g_av2Point;

// Input States
INPUT_STATE_BOOL(MoveForward);
INPUT_STATE_BOOL(MoveBackward);
INPUT_STATE_BOOL(MoveLeft);
INPUT_STATE_BOOL(MoveRight);
INPUT_STATE_BOOL(MoveUp);
INPUT_STATE_BOOL(MoveDown);
INPUT_STATE_BOOL(TurnLeft);
INPUT_STATE_BOOL(TurnRight);
INPUT_STATE_BOOL(PrimaryButton);
INPUT_STATE_BOOL(SecondaryButton);
INPUT_STATE_BOOL(ModifierButton);
INPUT_STATE_BOOL(ModifierShift);
INPUT_STATE_BOOL(ModifierCtrl);
INPUT_STATE_BOOL(ModifierAlt);
INPUT_STATE_BOOL(DisableEffect);
INPUT_STATE_BOOL(HideEffect);

INPUT_STATE_BOOL(AddBlocker);
INPUT_STATE_BOOL(ClearBlocker);

CHostClient     *g_pHostClient(nullptr);
CWorld          *g_pWorld(nullptr);

UI_TRIGGER(AnimList);

void    CL_RasterCircle(byte *pBuffer, uint uiBufferSpan, int x0, int y0, int radius);
void    CL_RasterCircle2(byte *pBuffer, uint uiBufferSpan, int x0, int y0, int radius);
void    CL_RasterRect(byte *pBuffer, uint uiBufferSpan, int x0, int y0, int x1, int y1);

#ifdef PATH_ANALYZER
void    MV_DrawPath();
#endif

UI_TRIGGER(Ctrl);
UI_TRIGGER(Shift);
UI_TRIGGER(Alt);
//=============================================================================

/*--------------------
  cmdNoSun
  --------------------*/
CMD(NoSun)
{
    ICvar::SetVec3(_T("scene_entitySunColor"), 0.0f, 0.0f, 0.0f);
    ICvar::SetVec3(_T("scene_terrainSunColor"), 0.0f, 0.0f, 0.0f);
    return true;
}


void    MV_CenterCamera()
{
    g_vCamOrigin.Clear();
    g_vCamLookAt.Clear();

    mv_camFov.Reset();

    mv_camYaw.Reset();
    mv_camPitch.Reset();
    mv_camRoll.Reset();

    model_x.Reset();
    model_y.Reset();
    model_z.Reset();
    model_yaw.Reset();
    model_pitch.Reset();
    model_roll.Reset();

    mv_camDistance.Reset();

    mv.camera.SetAngles(mv_camPitch, mv_camRoll, mv_camYaw);

    if (mv.hActiveModel != INVALID_RESOURCE)
    {
        CBBoxf bbModel(g_ResourceManager.GetModelBounds(mv.hActiveModel));
        g_vCamLookAt = (bbModel.GetMin() + bbModel.GetMax()) * 0.5f + CVec3f(model_x, model_y, model_z);
        mv_camDistance = MAX(bbModel.GetDim(X) * 1.333f, bbModel.GetDim(Z) * 1.333f) * model_scale;
    }
    else
    {
        g_vCamLookAt = CVec3f(model_x, model_y, model_z);
    }

    g_vCamOrigin = g_vCamLookAt + mv.camera.GetViewAxis(FORWARD) * -mv_camDistance;
}


/*--------------------
  Center
  --------------------*/
CMD(Center)
{
    MV_CenterCamera();
    return true;
}


/*--------------------
  Center
  --------------------*/
UI_VOID_CMD(Center, 0)
{
    cmdCenter();
}


void    MV_CenterCameraHon()
{
    g_vCamOrigin.Clear();
    g_vCamLookAt.Clear();

    mv_camFov = 53.5f;

    mv_camYaw.Reset();
    mv_camPitch = -56.0f;
    mv_camRoll.Reset();

    model_x.Reset();
    model_y.Reset();
    model_z.Reset();
    model_yaw.Reset();
    model_pitch.Reset();
    model_roll.Reset();

    mv_camDistance = 1650.0f;

    mv.camera.SetAngles(mv_camPitch, mv_camRoll, mv_camYaw);

    g_vCamLookAt = CVec3f(model_x, model_y, model_z);

    g_vCamOrigin = g_vCamLookAt + mv.camera.GetViewAxis(FORWARD) * -mv_camDistance;

    ICvar::SetFloat(_T("scene_sunAltitude"), 50.9016f);
    ICvar::SetFloat(_T("scene_sunAzimuth"), 32.4590f);
}


/*--------------------
  CenterHon
  --------------------*/
CMD(CenterHon)
{
    MV_CenterCameraHon();
    return true;
}


/*--------------------
  CenterHon
  --------------------*/
UI_VOID_CMD(CenterHon, 0)
{
    cmdCenter();
}


/*--------------------
  PlayAnim
  --------------------*/
CMD(PlayAnim)
{
    if (vArgList.size() < 1)
        return false;

    g_Skeleton.StartAnim(
        vArgList[0],                                        // Name
        g_pHostClient->GetTime(),                           // Start time
        (vArgList.size() > 1) ? AtoI(vArgList[1]) : 0,      // Channel
        (vArgList.size() > 2) ? AtoI(vArgList[2]) : -1);    // Blend time
    return true;
}

UI_VOID_CMD(PlayAnim, 1)
{
    g_Skeleton.StartAnim(vArgList[0]->Evaluate(), g_pHostClient->GetTime(), 0);
}

/*--------------------
 SetAnimSpeed
  --------------------*/
CMD(SetAnimSpeed)
{
    if (vArgList.size() < 1)
        return false;

    g_Skeleton.SetAnimSpeed(AtoF(vArgList[0]), (vArgList.size() > 1) ? AtoI(vArgList[1]) : 0);
    return true;
}


/*--------------------
 SetAnimSpeed
  --------------------*/
UI_VOID_CMD(SetAnimSpeed, 1)
{
    cmdSetAnimSpeed(vArgList[0]->Evaluate());
}


/*--------------------
 SetAnimTime
  --------------------*/
CMD(SetAnimTime)
{
    if (vArgList.size() < 1)
        return false;

    g_Skeleton.SetAnimTime(AtoF(vArgList[0]), (vArgList.size() > 1) ? AtoI(vArgList[1]) : 0);
    return true;
}


/*--------------------
 SetAnimTime
  --------------------*/
UI_VOID_CMD(SetAnimTime, 1)
{
    cmdSetAnimTime(vArgList[0]->Evaluate());
}


/*--------------------
  StartEffect  <effect> <channel>
  --------------------*/
CMD(StartEffect)
{
    if (vArgList.size() < 1)
        return false;
    
    int iChannel(vArgList.size() > 1 ? AtoI(vArgList[1]) : -1);

    g_hEffect = g_ResourceManager.Register(vArgList[0], RES_EFFECT);

    CEffect *pEffect(g_ResourceManager.GetEffect(g_hEffect));

    if (pEffect)
    {
        if (iChannel == -1)
        {
            for (int i(NUM_MV_EFFECTS - 1); i >= 0; --i)
            {
                if (!mv.apEffectThread[i])
                {
                    iChannel = i;
                    break;
                }
            }
        }

        if (iChannel == -1)
            return false;

        if (mv.apEffectThread[iChannel])
        {
            K2_DELETE(mv.apEffectThread[iChannel]);
            mv.apEffectThread[iChannel] = nullptr;
        }

        CEffectThread   *pEffectThread(pEffect->SpawnThread(g_pHostClient->GetTime()));

        if (!pEffectThread)
            return false;

        pEffectThread->SetCamera(&mv.camera);
        pEffectThread->SetWorld(g_pWorld);

        pEffectThread->SetSourceSkeleton(&g_Skeleton);
        pEffectThread->SetSourceModel(g_ResourceManager.GetModel(mv.hActiveModel));
        pEffectThread->SetTargetSkeleton(nullptr);
        pEffectThread->SetTargetModel(nullptr);

        mv.apEffectThread[iChannel] = pEffectThread;

        return true;
    }
    else
    {
        return false;
    }
}


/*--------------------
  PlayEffect  <effect> <channel>
  --------------------*/
UI_VOID_CMD(PlayEffect,2)
{
    cmdStartEffect(vArgList[0]->Evaluate(), vArgList[1]->Evaluate());
}


/*--------------------
  StopEffects
  --------------------*/
CMD(StopEffects)
{
    for (int i(0); i < NUM_MV_EFFECTS; i++)
    {
        if (mv.apEffectThread[i])
        {
            K2_DELETE(mv.apEffectThread[i]);
            mv.apEffectThread[i] = nullptr;
        }
    }

    return true;
}


/*--------------------
  ExpireEffects
  --------------------*/
CMD(ExpireEffects)
{
    for (int i(0); i < NUM_MV_EFFECTS; i++)
    {
        if (mv.apEffectThread[i] != nullptr)
        {
            if (mv.apEffectThread[i]->IsDeferred())
                mv.apEffectThread[i]->Expire(g_pHostClient->GetTime());
            else if (!mv.apEffectThread[i]->IsPersistent())
                SAFE_DELETE(mv.apEffectThread[i]);
        }
    }

    return true;
}


/*--------------------
  StopEffect <channel>
  --------------------*/
EVENT_CMD(StopEffect)
{
    if (vArgList.size() < 1)
        return false;

    Console << _T("StopEffect ") << vArgList[0] << newl;

    int iChannel(AtoI(vArgList[0]));

    if (iChannel >= 0 && iChannel < NUM_MV_EFFECTS)
    {
        if (mv.apEffectThread[iChannel])
        {
            K2_DELETE(mv.apEffectThread[iChannel]);
            mv.apEffectThread[iChannel] = nullptr;
        }
    }

    return true;
}


/*--------------------
  StartEffect <effect> <channel>
  --------------------*/
EVENT_CMD(StartEffect)
{
    if (vArgList.size() < 1)
        return false;

    if (vArgList.size() > 1)
        Console << _T("StartEffect ") << vArgList[0] << _T(" ") << vArgList[1] << newl;
    else
        Console << _T("StartEffect ") << vArgList[0] << newl;

    int iChannel(vArgList.size() > 1 ? AtoI(vArgList[1]) : -1);

    g_hEffect = g_ResourceManager.Register(vArgList[0], RES_EFFECT);

    CEffect *pEffect(g_ResourceManager.GetEffect(g_hEffect));

    if (pEffect)
    {
        if (iChannel == -1)
        {
            for (int i(NUM_MV_EFFECTS - 1); i >= 0; --i)
            {
                if (!mv.apEffectThread[i])
                {
                    iChannel = i;
                    break;
                }
            }
        }

        if (iChannel == -1)
            return false;

        if (mv.apEffectThread[iChannel])
        {
            K2_DELETE(mv.apEffectThread[iChannel]);
            mv.apEffectThread[iChannel] = nullptr;
        }

        CEffectThread   *pEffectThread(pEffect->SpawnThread(g_pHostClient->GetTime() + iTimeNudge));

        if (!pEffectThread)
            return false;

        pEffectThread->SetCamera(&mv.camera);
        pEffectThread->SetWorld(g_pWorld);

        pEffectThread->SetSourceSkeleton(&g_Skeleton);
        pEffectThread->SetSourceModel(g_ResourceManager.GetModel(mv.hActiveModel));
        pEffectThread->SetTargetSkeleton(nullptr);
        pEffectThread->SetTargetModel(nullptr);
        
        // Update entity attachment information
        pEffectThread->SetSourcePos(CVec3f(model_x, model_y, model_z));
        pEffectThread->SetSourceAxis(CAxis(model_pitch, model_roll, model_yaw));
        pEffectThread->SetSourceScale(model_scale);

        if (pEffectThread->GetUseEntityEffectScale())
            pEffectThread->SetSourceEffectScale(model_effectScale / model_scale);
        else
            pEffectThread->SetSourceEffectScale(1.0f);

        pEffectThread->SetTargetPos(CVec3f(2048.0f + 50.0f * sin(MsToSec(g_pHostClient->GetTime())), 2048.0f + 50.0f * cos(MsToSec(g_pHostClient->GetTime())), 0.0f));
        pEffectThread->SetTargetAxis(CAxis(0.0f, 0.0f, 0.0f));
        pEffectThread->SetTargetScale(1.0f);
        pEffectThread->SetTargetEffectScale(1.0f);

        pEffectThread->SetSourceSkeleton(&g_Skeleton);
        pEffectThread->SetSourceModel(g_ResourceManager.GetModel(mv.hActiveModel));

        pEffectThread->SetActive(true);

        if (pEffectThread->Execute(g_pHostClient->GetTime() + iTimeNudge))
        {
            // Effect finished, so delete it
            K2_DELETE(pEffectThread);
            return true;
        }

        mv.apEffectThread[iChannel] = pEffectThread;

        return true;
    }
    else
    {
        return false;
    }
}


/*--------------------
  eventPlaySound
  --------------------*/
EVENT_CMD(PlaySound)
{
    if (vArgList.size() < 1)
        return false;

    if (vArgList.size() > 7 && (AtoF(vArgList[7]) * 1000 >= M_Randnum(0, 1000)))
        return true;

    ResHandle hSample(g_ResourceManager.Register(K2_NEW(ctx_ModelViewer,  CSample)(vArgList[0], 0), RES_SAMPLE));

    if (hSample == INVALID_RESOURCE)
        return false;

    int iChannel;
    if (vArgList.size() > 3)
        iChannel = AtoI(vArgList[3]);
    else
        iChannel = -1;

    if (iChannel == -1)
    {
        for (iChannel = NUM_MV_SOUNDS - 1; iChannel >=0; iChannel--)
            if (mv.ahSounds[iChannel] == INVALID_INDEX)
                break;
    }

    if (iChannel < 0 || iChannel >= NUM_MV_SOUNDS)
        return false;

    if (mv.ahSounds[iChannel] != INVALID_INDEX)
        K2SoundManager.StopHandle(mv.ahSounds[iChannel]);

    CVec3f v3Pos(model_x, model_y, model_z);
    CVec3f v3Zero(0.0, 0.0, 0.0);
    mv.ahSounds[iChannel] = K2SoundManager.PlaySFXSound(hSample, &v3Pos, &v3Zero, vArgList.size() > 2 ? AtoF(vArgList[2]) : 1.0, vArgList.size() > 1 ? AtoF(vArgList[1]) : -1, -1, 128, 0, vArgList.size() > 4 ? AtoI(vArgList[4]) : 0, vArgList.size() > 5 ? AtoI(vArgList[5]) : 0, vArgList.size() > 6 ? AtoI(vArgList[6]) : 0);

    return true;
}


/*--------------------
  eventPlaySoundLooping
  --------------------*/
EVENT_CMD(PlaySoundLooping)
{
    if (vArgList.size() < 1)
        return false;

    ResHandle hSample(g_ResourceManager.Register(K2_NEW(ctx_ModelViewer,  CSample)(vArgList[0], 0), RES_SAMPLE));

    if (hSample == INVALID_RESOURCE)
        return false;

    int iChannel;
    if (vArgList.size() > 3)
        iChannel = AtoI(vArgList[3]);
    else
        iChannel = -1;

    if (iChannel == -1)
    {
        for (iChannel = NUM_MV_SOUNDS - 1; iChannel >=0; iChannel--)
            if (mv.ahSounds[iChannel] == INVALID_INDEX)
                break;
    }

    bool bOverride(vArgList.size() > 6 ? AtoB(vArgList[6]) : true);

    if (iChannel < 0 || iChannel >= NUM_MV_SOUNDS || (!bOverride && mv.ahSounds[iChannel] != INVALID_INDEX))
        return false;

    if (mv.ahSounds[iChannel] != INVALID_INDEX)
        K2SoundManager.StopHandle(mv.ahSounds[iChannel]);

    CVec3f v3Pos(model_x, model_y, model_z);
    CVec3f v3Zero(0.0, 0.0, 0.0);
    mv.ahSounds[iChannel] = K2SoundManager.PlaySFXSound(hSample, &v3Pos, &v3Zero, vArgList.size() > 2 ? AtoF(vArgList[2]) : 1.0, vArgList.size() > 1 ? AtoF(vArgList[1]) : -1, -1, 0, SND_LOOP, vArgList.size() > 4 ? AtoI(vArgList[4]) : 0, 0, vArgList.size() > 5 ? AtoI(vArgList[5]) : 0, vArgList.size() > 7 ? AtoI(vArgList[7]) : 0, vArgList.size() > 8 ? AtoF(vArgList[8]) : 1.0f, vArgList.size() > 9 ? AtoF(vArgList[9]) : 1.0f, vArgList.size() > 10 ? AtoI(vArgList[10]) : 0);

    return true;
}


/*--------------------
  eventPlaySoundLinear
  --------------------*/
EVENT_CMD(PlaySoundLinear)
{
    if (vArgList.size() < 1)
        return false;

    if (vArgList.size() > 7 && (AtoF(vArgList[7]) * 1000 >= M_Randnum(0, 1000)))
        return true;

    ResHandle hSample(g_ResourceManager.Register(K2_NEW(ctx_ModelViewer,  CSample)(vArgList[0], 0), RES_SAMPLE));

    if (hSample == INVALID_RESOURCE)
        return false;

    int iChannel;
    if (vArgList.size() > 4)
        iChannel = AtoI(vArgList[4]);
    else
        iChannel = -1;

    if (iChannel == -1)
    {
        for (iChannel = NUM_MV_SOUNDS - 1; iChannel >=0; iChannel--)
            if (mv.ahSounds[iChannel] == INVALID_INDEX)
                break;
    }

    if (iChannel < 0 || iChannel >= NUM_MV_SOUNDS)
        return false;

    if (mv.ahSounds[iChannel] != INVALID_INDEX)
        K2SoundManager.StopHandle(mv.ahSounds[iChannel]);

    CVec3f v3Pos(model_x, model_y, model_z);
    CVec3f v3Zero(0.0, 0.0, 0.0);
    mv.ahSounds[iChannel] = K2SoundManager.PlaySFXSound(hSample, &v3Pos, &v3Zero, vArgList.size() > 3 ? AtoF(vArgList[3]) : 1.0, vArgList.size() > 1 ? AtoF(vArgList[1]) : -1, -1, 128, SND_LINEARFALLOFF, vArgList.size() > 5 ? AtoI(vArgList[5]) : 0, vArgList.size() > 6 ? AtoI(vArgList[6]) : 0, vArgList.size() > 7 ? AtoI(vArgList[7]) : 0, 0, 1.0f, 1.0f, 0, vArgList.size() > 2 ? AtoF(vArgList[2]) : -1.0f);

    return true;
}


/*--------------------
  eventPlaySoundLoopingLinear
  --------------------*/
EVENT_CMD(PlaySoundLoopingLinear)
{
    if (vArgList.size() < 1)
        return false;

    ResHandle hSample(g_ResourceManager.Register(K2_NEW(ctx_ModelViewer,  CSample)(vArgList[0], 0), RES_SAMPLE));

    if (hSample == INVALID_RESOURCE)
        return false;

    int iChannel;
    if (vArgList.size() > 3)
        iChannel = AtoI(vArgList[3]);
    else
        iChannel = -1;

    if (iChannel == -1)
    {
        for (iChannel = NUM_MV_SOUNDS - 1; iChannel >=0; iChannel--)
            if (mv.ahSounds[iChannel] == INVALID_INDEX)
                break;
    }

    bool bOverride(vArgList.size() > 6 ? AtoB(vArgList[6]) : true);

    if (iChannel < 0 || iChannel >= NUM_MV_SOUNDS || (!bOverride && mv.ahSounds[iChannel] != INVALID_INDEX))
        return false;

    if (mv.ahSounds[iChannel] != INVALID_INDEX)
        K2SoundManager.StopHandle(mv.ahSounds[iChannel]);

    CVec3f v3Pos(model_x, model_y, model_z);
    CVec3f v3Zero(0.0, 0.0, 0.0);
    mv.ahSounds[iChannel] = K2SoundManager.PlaySFXSound(hSample, &v3Pos, &v3Zero, vArgList.size() > 2 ? AtoF(vArgList[2]) : 1.0, vArgList.size() > 1 ? AtoF(vArgList[1]) : -1, -1, 0, true, vArgList.size() > 4 ? AtoI(vArgList[4]) : 0, 0, vArgList.size() > 5 ? AtoI(vArgList[5]) : 0, vArgList.size() > 7 ? AtoI(vArgList[7]) : 0, vArgList.size() > 8 ? AtoF(vArgList[8]) : 1.0f, vArgList.size() > 9 ? AtoF(vArgList[9]) : 1.0f, vArgList.size() > 10 ? AtoI(vArgList[10]) : 0);

    return true;
}


/*--------------------
  eventStopSound
  --------------------*/
EVENT_CMD(StopSound)
{
    if (vArgList.size() < 1)
        return false;

    int iChannel(AtoI(vArgList[0]));
    if (iChannel < 0 || iChannel >= NUM_MV_SOUNDS)
        return false;

    K2SoundManager.StopHandle(mv.ahSounds[iChannel]);
    mv.ahSounds[iChannel] = INVALID_INDEX;

    return true;
}


/*--------------------
  CameraPitch
  --------------------*/
ACTION_AXIS(CameraPitch)
{
    if (cam_freeLook)
    {
        if (PrimaryButton)
        {
            mv_camPitch -= fDelta * 0.2f;
            mv_camRoll = 0;
        }

        if (ModifierButton)
            g_vCamOrigin -= mv.camera.GetViewAxis(UP) * fDelta;
    }
    else
    {
        if (PrimaryButton)
        {
            if (ModifierCtrl)
                model_pitch += fDelta * 0.5f;
            else
                mv_camPitch += fDelta * 0.5f;
        }

        if (ModifierButton || SecondaryButton)
        {
            if (ModifierCtrl)
            {
                CVec3f vModelOrigin(model_x, model_y, model_z);

                vModelOrigin -= mv.camera.GetViewAxis(UP) * fDelta * 0.5f;

                model_x = vModelOrigin.x;
                model_y = vModelOrigin.y;
                model_z = vModelOrigin.z;
            }
            else
            {
                g_vCamLookAt += mv.camera.GetViewAxis(UP) * fDelta * 0.5f;
            }
        }
    }
}


/*--------------------
  CameraYaw
  --------------------*/
ACTION_AXIS(CameraYaw)
{
    if (cam_freeLook)
    {
        if (PrimaryButton)
        {
            mv_camYaw -= fDelta * 0.2f;
            mv_camRoll = 0;
        }

        if (ModifierButton)
            g_vCamOrigin += mv.camera.GetViewAxis(RIGHT) * fDelta;
    }
    else
    {
        if (PrimaryButton)
        {
            if (ModifierCtrl)
                model_yaw += fDelta * 0.5f;
            else
                mv_camYaw += fDelta * 0.5f;
        }

        if (ModifierButton || SecondaryButton)
        {
            if (ModifierCtrl)
            {
                CVec3f vModelOrigin(model_x, model_y, model_z);

                vModelOrigin += mv.camera.GetViewAxis(RIGHT) * fDelta * 0.5f;

                model_x = vModelOrigin.x;
                model_y = vModelOrigin.y;
                model_z = vModelOrigin.z;
            }
            else
            {
                g_vCamLookAt -= mv.camera.GetViewAxis(RIGHT) * fDelta * 0.5f;
            }
        }
    }
}


/*--------------------
  ZoomIn
  --------------------*/
ACTION_IMPULSE(ZoomIn)
{
    mv_camDistance -= mv_camDistance * 0.2f;
}


/*--------------------
  ZoomOut
  --------------------*/
ACTION_IMPULSE(ZoomOut)
{
    mv_camDistance += mv_camDistance * 0.2f;
}


/*--------------------
  ToggleViewerUI
  --------------------*/
ACTION_IMPULSE(ToggleViewerUI)
{
    g_bShowInterface = !g_bShowInterface;
    if (g_bShowInterface)
        UIManager.GetActiveInterface()->Show();
    else
        UIManager.GetActiveInterface()->Hide();
}


/*====================
  MV_StopAllSounds
  ====================*/
void    MV_StopAllSounds()
{
    for (int i(0); i < NUM_MV_SOUNDS; i++)
    {
        if (mv.ahSounds[i] != INVALID_INDEX)
        {
            K2SoundManager.StopHandle(mv.ahSounds[i]);
            mv.ahSounds[i] = INVALID_INDEX;
        }
    }
}


/*====================
  MV_UpdateSounds
  ====================*/
void    MV_UpdateSounds(const CVec3f &v3Pos, const CVec3f &v3Vel)
{
    for (int i(0); i < NUM_MV_SOUNDS; i++)
    {
        if (mv.ahSounds[i] != INVALID_INDEX)
        {
            if (!K2SoundManager.UpdateHandle(mv.ahSounds[i], v3Pos, v3Vel))
                mv.ahSounds[i] = INVALID_INDEX;
        }
    }
}


/*====================
  MV_ParticleTrace
  ====================*/
bool    MV_ParticleTrace(const CVec3f &v3Start, const CVec3f &v3End, CVec3f &v3EndPos, CVec3f &v3Normal)
{
    if (v3Start.z < 0.0f)
    {
        v3EndPos = v3Start;
        v3Normal = Normalize(v3End - v3Start);
        return true;
    }
    else if (v3End.z < 0.0f)
    {
        float fFraction(v3Start.z / (v3Start.z - v3End.z));

        v3EndPos = LERP(fFraction, v3Start, v3End);
        v3Normal = CVec3f(0.0f, 0.0f, 1.0f);

        v3EndPos += v3Normal * 0.01f;

        return true;
    }

    v3EndPos = v3End;   
    return false;
}


/*====================
  MV_UpdateAnimListUI
  ====================*/
void    MV_UpdateAnimListUI(ResHandle hModel)
{
    CInterface *pInterface(UIManager.GetInterface(_T("mv_main")));

    if (hModel == INVALID_RESOURCE || pInterface == nullptr)
        return;

    // Clear list
    AnimList.Trigger(_T("*clear"));

    int iAnimIndex(0);
    for (;;)
    {
        tstring sAnimName(g_ResourceManager.GetAnimName(hModel, iAnimIndex));
        ++iAnimIndex;
        if (sAnimName.empty())
            break;

        AnimList.Trigger(sAnimName);
    }
}


/*====================
  CL_Init
  ====================*/
bool    CL_Init(CHostClient *pHostClient)
{
    PROFILE("CL_Init");

    MemManager.Set(&mv, 0, sizeof(SMv));
    mv.hActiveModel = INVALID_RESOURCE;

    g_pHostClient = pHostClient;
    if (pHostClient == nullptr)
        return false;

    g_pWorld = g_pHostClient->GetWorld();
    if (g_pWorld == nullptr)
        EX_ERROR(_T("Invalid CWorld from host"));

#ifdef PATH_ANALYZER
    g_pWorld->SetHostType(WORLDHOST_BOTH);
#else
    g_pWorld->SetHostType(WORLDHOST_CLIENT);
#endif
    
    // Setup actions
    ActionRegistry.BindAxis(BINDTABLE_GAME, AXIS_MOUSE_X, BIND_MOD_NONE,    _T("CameraYaw"));
    ActionRegistry.BindAxis(BINDTABLE_GAME, AXIS_MOUSE_Y, BIND_MOD_NONE,    _T("CameraPitch"));

    ActionRegistry.BindButton(BINDTABLE_GAME,   EButton('W'), BIND_MOD_NONE,    _T("MoveForward"));
    ActionRegistry.BindButton(BINDTABLE_GAME,   EButton('S'), BIND_MOD_NONE,    _T("MoveBackward"));

    ActionRegistry.BindButton(BINDTABLE_GAME,   EButton('A'), BIND_MOD_NONE,    _T("MoveLeft"));
    ActionRegistry.BindButton(BINDTABLE_GAME,   EButton('D'), BIND_MOD_NONE,    _T("MoveRight"));

#ifdef GAME_KELLOGS
    ActionRegistry.BindButton(BINDTABLE_GAME, BUTTON_SPACE, BIND_MOD_NONE,      _T("ResetCamera"));
#else
    ActionRegistry.BindButton(BINDTABLE_GAME, BUTTON_SPACE, BIND_MOD_NONE,      _T("MoveUp"));
#endif
    ActionRegistry.BindButton(BINDTABLE_GAME,   EButton('C'), BIND_MOD_NONE,    _T("MoveDown"));

    ActionRegistry.BindButton(BINDTABLE_GAME,   EButton('Q'), BIND_MOD_NONE,    _T("TurnLeft"));
    ActionRegistry.BindButton(BINDTABLE_GAME,   EButton('E'), BIND_MOD_NONE,    _T("TurnRight"));

    ActionRegistry.BindButton(BINDTABLE_GAME,   EButton('T'), BIND_MOD_NONE,    _T("Cmd"), _T("ExpireEffects"));
    ActionRegistry.BindButton(BINDTABLE_GAME,   EButton('R'), BIND_MOD_NONE,    _T("Cmd"), _T("Repeat"));

    ActionRegistry.BindButton(BINDTABLE_GAME,   BUTTON_MOUSEL, BIND_MOD_NONE,   _T("PrimaryButton"));
    ActionRegistry.BindButton(BINDTABLE_GAME,   BUTTON_MOUSER, BIND_MOD_NONE,   _T("SecondaryButton"));
    ActionRegistry.BindButton(BINDTABLE_GAME,   BUTTON_SHIFT, BIND_MOD_NONE,    _T("ModifierShift"));
    ActionRegistry.BindButton(BINDTABLE_GAME,   BUTTON_CTRL, BIND_MOD_NONE, _T("ModifierCtrl"));
    ActionRegistry.BindButton(BINDTABLE_GAME,   BUTTON_ALT, BIND_MOD_NONE,      _T("ModifierAlt"));
    ActionRegistry.BindButton(BINDTABLE_GAME,   BUTTON_MOUSEM, BIND_MOD_NONE,   _T("ModifierButton"));

    ActionRegistry.BindImpulse(BINDTABLE_GAME, BUTTON_WHEELUP, BIND_MOD_NONE,   _T("ZoomIn"));
    ActionRegistry.BindImpulse(BINDTABLE_GAME, BUTTON_WHEELDOWN, BIND_MOD_NONE, _T("ZoomOut"));
    ActionRegistry.BindImpulse(BINDTABLE_GAME, BUTTON_BACKSPACE, BIND_MOD_NONE, _T("Cmd"), _T("Center"));
    ActionRegistry.BindImpulse(BINDTABLE_GAME, BUTTON_BACKSPACE, BIND_MOD_CTRL, _T("Cmd"), _T("CenterHon"));

    ActionRegistry.BindImpulse(BINDTABLE_GAME, EButton('U'), BIND_MOD_NONE, _T("Cmd"), _T("set gfx_textures 0"));
    ActionRegistry.BindImpulse(BINDTABLE_GAME, EButton('I'), BIND_MOD_NONE, _T("Cmd"), _T("set gfx_textures 1"));
    ActionRegistry.BindImpulse(BINDTABLE_GAME, EButton('O'), BIND_MOD_NONE, _T("Cmd"), _T("set gfx_textures 2"));
    ActionRegistry.BindImpulse(BINDTABLE_GAME, EButton('P'), BIND_MOD_NONE, _T("Cmd"), _T("set gfx_textures 3"));

    ActionRegistry.BindButton(BINDTABLE_GAME,   EButton('1'), BIND_MOD_NONE,    _T("DisableEffect"));

    ActionRegistry.BindImpulse(BINDTABLE_GAME, BUTTON_TAB, BIND_MOD_NONE,   _T("ToggleViewerUI"));

#ifdef PATH_ANALYZER
    ActionRegistry.BindImpulse(BINDTABLE_GAME, BUTTON_MISC4, BIND_MOD_NONE,     _T("PathSetStart"));
    ActionRegistry.BindImpulse(BINDTABLE_GAME, BUTTON_MISC6, BIND_MOD_NONE,     _T("PathSetEnd"));
    ActionRegistry.BindImpulse(BINDTABLE_GAME, BUTTON_MISC5, BIND_MOD_NONE,     _T("PathClear"));
    ActionRegistry.BindButton(BINDTABLE_GAME, EButton('B'), BIND_MOD_NONE,      _T("AddBlocker"));
    ActionRegistry.BindButton(BINDTABLE_GAME, EButton('N'), BIND_MOD_NONE,      _T("ClearBlocker"));
    ActionRegistry.BindImpulse(BINDTABLE_GAME, BUTTON_COMMA, BIND_MOD_NONE,     _T("Cmd"), _T("Inc mv_pathStep -1"));
    ActionRegistry.BindImpulse(BINDTABLE_GAME, BUTTON_PERIOD, BIND_MOD_NONE,    _T("Cmd"), _T("Inc mv_pathStep 1"));
    ActionRegistry.BindImpulse(BINDTABLE_GAME, BUTTON_MISC2, BIND_MOD_NONE,     _T("Cmd"), _T("Set mv_pathStep 0"));
#endif

    ICvar::SetString(_T("scene_drawSky"), _T("0"));
    ICvar::SetString(_T("scene_sunAltitude"), _T("30"));
    ICvar::SetString(_T("scene_sunAzimuth"), _T("90"));
    ICvar::SetString(_T("scene_bgColor"), mv_bgColor.GetString());
    ICvar::SetString(_T("gfx_fogType"), _T("0"));
    ICvar::SetString(_T("scene_nearclip"), _T("1.0"));

    // Load UI
    UIManager.LoadInterface(_T("/ui/mv_main.interface"));

    Console.ExecuteScript(_T("/mv.cfg"));
    model.SetModified(true);

    for (int i(0); i < NUM_MV_SOUNDS; i++)
        mv.ahSounds[i] = INVALID_INDEX;

    for (int i(0); i < NUM_MV_EFFECTS; i++)
        mv.apEffectThread[i] = nullptr;

    MV_CenterCamera();
    return true;
}


/*====================
  MV_AddSelectionRingToScene
  ====================*/
void    MV_AddSelectionRingToScene()
{
    CSceneEntity sceneEntity;
    sceneEntity.Clear();

    sceneEntity.width = mv_selectionRingSize;
    sceneEntity.height = mv_selectionRingSize;
    sceneEntity.scale = 1.0f;
    sceneEntity.SetPosition(CVec3f(model_x, model_y, model_z));
    sceneEntity.objtype = OBJTYPE_GROUNDSPRITE;
    sceneEntity.hRes = g_ResourceManager.Register(_T("/shared/materials/selection_indicator.material"), RES_MATERIAL);
    sceneEntity.flags = SCENEENT_SOLID_COLOR;
    sceneEntity.color = LIME;

    SceneManager.AddEntity(sceneEntity);
}



/*====================
  CL_Frame
  ====================*/
void    CL_Frame()
{
    PROFILE("CL_Frame");

    vector<int> vTest;
    vTest.push_back(M_Randnum(0, 9999));

    if (!IModalDialog::IsActive())
        UIManager.SetActiveInterface(mv_interface);

    if (model.IsModified())
    {
        PROFILE_EX("Model", eProfileType(666));

        if (model.empty())
            mv.hActiveModel = INVALID_RESOURCE;
        else
            mv.hActiveModel = g_ResourceManager.Register(model, RES_MODEL);

        g_Skeleton.SetModel(mv.hActiveModel);
        MV_CenterCamera();
        model.SetModified(false);
        // stop effects
        for (int i(0); i < NUM_MV_EFFECTS; i++)
        {
            if (mv.apEffectThread[i])
            {
                K2_DELETE(mv.apEffectThread[i]);
                mv.apEffectThread[i] = nullptr;
            }
        }
        MV_UpdateAnimListUI(mv.hActiveModel);
        MV_StopAllSounds();

        g_bbModelBounds = g_ResourceManager.GetModelBounds(mv.hActiveModel);
    }

    if (mv_bgColor.IsModified())
    {
        ICvar::SetString(_T("scene_bgColor"), mv_bgColor.GetString());

        mv_bgColor.SetModified(false);
    }

    if (mv_ground.IsModified())
    {
        if (mv_ground)
        {
            g_pWorld->Free();

            g_pWorld->StartLoad(mv_groundMap);
            while (g_pWorld->IsLoading())
                g_pWorld->LoadNextComponent();
            ICvar::SetString(_T("scene_bgColor"), mv_bgColor.GetString());
            ICvar::SetString(_T("scene_drawSky"), _T("0"));
            ICvar::SetString(_T("scene_sunAltitude"), _T("30"));
            ICvar::SetString(_T("scene_sunAzimuth"), _T("90"));
        }
        else
            g_pWorld->Free();

        mv_ground.SetModified(false);
    }

    //model_x = 50.0f * sin(g_pHostClient->GetTime() * SEC_PER_MS);
    //model_y = 50.0f * cos(g_pHostClient->GetTime() * SEC_PER_MS);

    mv.frametime = MsToSec(Host.GetFrameLength());
    mv.lastFrame = g_pHostClient->GetTime();

    Input.ExecuteBinds(BINDTABLE_GAME, 0);

    if (PrimaryButton || SecondaryButton || ModifierButton)
    {
        Input.SetCursorFrozen(CURSOR_GAME, BOOL_TRUE);
        Input.SetCursorHidden(CURSOR_GAME, BOOL_TRUE);
    }
    else
    {
        Input.SetCursorFrozen(CURSOR_GAME, BOOL_FALSE);
        Input.SetCursorHidden(CURSOR_GAME, BOOL_FALSE);
    }

    SceneManager.ClearBackground();
    SceneManager.Clear();

    //
    // Setup the camera
    //

    {
        PROFILE("Camera");

        mv.camera.DefaultCamera(float(Vid.GetScreenW()), float(Vid.GetScreenH()));
        mv.camera.SetTime(g_pHostClient->GetTime() * SEC_PER_MS);

        CAxis aAxis(model_pitch, model_roll, model_yaw);

        mv.camera.SetReflect(CPlane(aAxis.Up(), CVec3f(model_x, model_y, model_z)));
        mv.camera.SetAngles(CVec3f(mv_camPitch, mv_camRoll, mv_camYaw));

        if (cam_freeLook)
        {
            CVec3f vVelocity(0.0f, 0.0f, 0.0f);

            if (MoveForward)
                vVelocity += mv.camera.GetViewAxis(FORWARD) * mv_camSpeed;
            if (MoveBackward)
                vVelocity -= mv.camera.GetViewAxis(FORWARD) * mv_camSpeed;

            if (MoveRight)
                vVelocity += mv.camera.GetViewAxis(RIGHT) * mv_camSpeed;
            if (MoveLeft)
                vVelocity -= mv.camera.GetViewAxis(RIGHT) * mv_camSpeed;

            if (MoveUp)
                vVelocity += mv.camera.GetViewAxis(UP) * mv_camSpeed;
            if (MoveDown)
                vVelocity -= mv.camera.GetViewAxis(UP) * mv_camSpeed;

            g_vCamOrigin += vVelocity * mv.frametime;
            mv.camera.SetOrigin(g_vCamOrigin);
            g_vCamLookAt = g_vCamOrigin + mv.camera.GetViewAxis(FORWARD) * mv_camDistance;
        }
        else
        {
            g_vCamOrigin = g_vCamLookAt;
            g_vCamOrigin += mv.camera.GetViewAxis(FORWARD) * -mv_camDistance;
            mv.camera.SetOrigin(g_vCamOrigin);
        }

        mv.camera.SetFovXCalc(mv_camFov);

        if (mv_fogofWar)
            mv.camera.AddFlags(CAM_FOG_OF_WAR);
        else
            mv.camera.RemoveFlags(CAM_FOG_OF_WAR);

        mv.camera.AddFlags(CAM_SHADOW_SCENE_BOUNDS);
        mv.camera.SetShadowMaxFov(110.0f);

        mv.camera.SetShadowBias(MAX(Distance(mv.camera.GetOrigin(), CVec3f(model_x, model_y, model_z)) + (g_bbModelBounds.GetDim(X) + g_bbModelBounds.GetDim(Y) + g_bbModelBounds.GetDim(Z)) / 3.0f - 500.0f, 0.0f));

        CInterface *pActiveInterface(UIManager.GetActiveInterface());
        if (pActiveInterface != nullptr)
        {
            float fWidth(pActiveInterface->GetSceneWidth());
            float fHeight(pActiveInterface->GetSceneHeight());
            
            mv.camera.SetX(pActiveInterface->GetSceneX() + (pActiveInterface->GetSceneWidth() - fWidth) * 0.5f);
            mv.camera.SetY(pActiveInterface->GetSceneY());
            mv.camera.SetWidth(fWidth);
            mv.camera.SetHeight(fHeight);
        }
        else
        {
            mv.camera.SetX(0.0f);
            mv.camera.SetY(0.0f);
            mv.camera.SetWidth(Vid.GetScreenW());
            mv.camera.SetHeight(Vid.GetScreenH());
        }

    }

    // Update sounds
    MV_UpdateSounds(CVec3f(model_x, model_y, model_z), V3_ZERO);
    K2SoundManager.SetListenerPosition(mv.camera.GetOrigin(), V3_ZERO, mv.camera.GetViewAxis(FORWARD), mv.camera.GetViewAxis(UP), false);

    if (!cam_freeLook)
        K2SoundManager.SetCenter(g_vCamLookAt);

    //
    // Render the model
    //

    CSceneEntity cEntity;

    if (!model.empty())
    {
        float fCircle(model_circleSpeed * MsToSec(Host.GetTime()) * 360.0f);                

        cEntity.objtype = OBJTYPE_MODEL;
        cEntity.hRes = mv.hActiveModel;
        cEntity.hSkin = g_ResourceManager.GetSkin(cEntity.hRes, skin);
        cEntity.scale = model_scale;
        cEntity.color = model_color;
        cEntity.teamcolor = CVec4f(model_teamColor, 1.0f);
        cEntity.SetPosition(CVec3f(model_x, model_y, model_z));
        cEntity.angle[YAW] = model_yaw + model_yawSpeed * MsToSec(Host.GetTime()) - fCircle;
        cEntity.angle[PITCH] = model_pitch;
        cEntity.angle[ROLL] = model_roll;
        cEntity.flags = SCENEENT_SOLID_COLOR;
        cEntity.s1 = model_param[0];
        cEntity.t1 = model_param[1];
        cEntity.s2 = model_param[2];
        cEntity.t2 = model_param[3];

        if (g_pWorld->IsLoaded())
            cEntity.flags |= SCENEENT_TERRAIN_TEXTURES;

        if (model_circleSpeed > 0.0f)
            cEntity.SetPosition(cEntity.GetPosition() + CVec3f(256.0f * DEGSIN(fCircle), 256.0f * DEGCOS(fCircle), 0.0f));


        if (!material.empty())
        {
            cEntity.hSkin = g_ResourceManager.Register(material, RES_MATERIAL);
            cEntity.flags |= SCENEENT_SINGLE_MATERIAL;
        }

        if (mv_poseObject)
        {
            g_Skeleton.Pose(g_pHostClient->GetTime(), 0.0f, 0.0f);

            int iAnimTime;
            float fFrame, fFraction;

            g_Skeleton.ComputeAnimFrame(g_pHostClient->GetTime(), 0, iAnimTime, fFrame, fFraction);

            mv_animFrame = INT_FLOOR(fFrame);
            mv_animTime = iAnimTime;
            mv_animFraction = fFraction;

            cEntity.skeleton = &g_Skeleton;
        }

        if (g_Skeleton.CheckEvents())
        {
            tstring sOldDir(FileManager.GetWorkingDirectory());
            FileManager.SetWorkingDirectory(Filename_GetPath(g_ResourceManager.GetPath(mv.hActiveModel)));

            const vector<SAnimEventCmd> &vEventCmds(g_Skeleton.GetEventCmds());

            for (vector<SAnimEventCmd>::const_iterator it(vEventCmds.begin()); it != vEventCmds.end(); ++it)
                EventScript.Execute(it->sCmd, it->iTimeNudge);

            g_Skeleton.ClearEvents();

            FileManager.SetWorkingDirectory(sOldDir);
        }

        
    }

    CVec3f v3CameraOffset(V3_ZERO);
    CVec3f v3CameraAngleOffset(V3_ZERO);

    //
    // Update and render active effects
    //
    {
        PROFILE("Effects");

        mv_activeEffects = 0;

        for (int i(0); i < NUM_MV_EFFECTS; i++)
        {
            CEffectThread* pEffectThread(mv.apEffectThread[i]);
            if (pEffectThread == nullptr)
                continue;

            pEffectThread->SetActive(!DisableEffect);
            pEffectThread->SetSourceVisibility(!HideEffect);
            pEffectThread->SetWorld(g_pWorld);

            // Update entity attachment information
            pEffectThread->SetSourcePos(CVec3f(model_x, model_y, model_z));
            pEffectThread->SetSourceAxis(CAxis(model_pitch, model_roll, model_yaw));
            pEffectThread->SetSourceScale(model_scale);

            if (pEffectThread->GetUseEntityEffectScale())
                pEffectThread->SetSourceEffectScale(model_effectScale / model_scale);
            else
                pEffectThread->SetSourceEffectScale(1.0f);

            pEffectThread->SetTargetPos(CVec3f(2048.0f + 50.0f * sin(MsToSec(g_pHostClient->GetTime())), 2048.0f + 50.0f * cos(MsToSec(g_pHostClient->GetTime())), 0.0f));
            pEffectThread->SetTargetAxis(CAxis(0.0f, 0.0f, 0.0f));
            pEffectThread->SetTargetScale(1.0f);
            pEffectThread->SetTargetEffectScale(1.0f);

            pEffectThread->SetSourceSkeleton(&g_Skeleton);
            pEffectThread->SetSourceModel(g_ResourceManager.GetModel(mv.hActiveModel));
            
            if (pEffectThread->Execute(g_pHostClient->GetTime()))
            {
                // Effect finished, so delete it
                K2_DELETE(pEffectThread);
                mv.apEffectThread[i] = nullptr;
            }
            else
            {
                v3CameraOffset += pEffectThread->GetCameraOffset();
                v3CameraAngleOffset += pEffectThread->GetCameraAngleOffset();

                // Update and render all particles systems associated with this effect thread
                const InstanceMap &mapInstances(pEffectThread->GetInstances());

                for (InstanceMap::const_iterator psit(mapInstances.begin()); psit != mapInstances.end(); ++psit)
                {
                    IEffectInstance *pParticleSystem = psit->second;

                    pParticleSystem->Update(g_pHostClient->GetTime(), MV_ParticleTrace);

                    if (!pParticleSystem->IsDead())
                    {
                        if (pParticleSystem->IsParticleSystem())
                            SceneManager.AddParticleSystem(static_cast<CParticleSystem *>(pParticleSystem), true);
                        else if (pParticleSystem->IsModifier())
                        {
                            static_cast<CSceneEntityModifier *>(pParticleSystem)->Modify(cEntity);
                        }
                    }
                }

                pEffectThread->Cleanup();

                ++mv_activeEffects;
            }
        }
    }

    SceneManager.PrepCamera(mv.camera);

    if (!model.empty())
    {
        CVec3f v3Pos(cEntity.GetPosition());

        CModel *pModel(g_ResourceManager.GetModel(cEntity.hRes));
        if (pModel != nullptr)
        {
            cEntity.flags |= SCENEENT_USE_AXIS;
            cEntity.axis = CAxis(cEntity.angle);

            CBBoxf bbBounds(pModel->GetBounds());
            bbBounds.Transform(cEntity.GetPosition(), cEntity.axis, cEntity.scale);

            cEntity.bounds = bbBounds;
            cEntity.flags |= SCENEENT_USE_BOUNDS;
        }

        CVec3f v3MaxOffset
        (
            cEntity.bounds.GetDim(X) * (model_numX - 1),
            cEntity.bounds.GetDim(Y) * (model_numY - 1),
            cEntity.bounds.GetDim(Z) * (model_numZ - 1)
        );

        v3Pos -= v3MaxOffset * 0.5f;

        for (int iX(0); iX < model_numX; ++iX)
        {
            for (int iY(0); iY < model_numY; ++iY)
            {
                for (int iZ(0); iZ < model_numZ; ++iZ)
                {
                    CVec3f v3Offset
                    (
                        cEntity.bounds.GetDim(X) * iX,
                        cEntity.bounds.GetDim(Y) * iY,
                        cEntity.bounds.GetDim(Z) * iZ
                    );

                    cEntity.SetPosition(v3Pos + v3Offset);

                    SceneManager.AddEntity(cEntity, false);
                }
            }
        }
    }

    if (mv_selectionRing)
        MV_AddSelectionRingToScene();

#if 0 // debug stuff
    Draw2D.SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    ResHandle hMaterial(g_ResourceManager.Register(_T("/core/materials/line.material"), RES_MATERIAL));

    static CVec3f vOrigin(-0.21125072, 1.4371842, 1.0210210);

    if (ModifierAlt)
    {
        const CVec3f &v3CamOrigin(mv.camera.GetOrigin());

        CVec3f v3CursorDir(mv.camera.ConstructRay(Input.GetCursorPos()));

        CVec3f v3Trace(v3CamOrigin + v3CursorDir * 1000.0f);

        CPlane plInf0(0.0f, 0.0f, -1.0f, -1.0210210);
        CPlane plInf1(0.0f, 0.0f, 1.0f, 1.0210210);

        float fFrac(1.0f);
        if (I_LinePlaneIntersect(v3CamOrigin, v3Trace, plInf0, fFrac) ||
            I_LinePlaneIntersect(v3CamOrigin, v3Trace, plInf1, fFrac))
        {
            if (fFrac < 1.0f)
                vOrigin = LERP(fFrac, v3CamOrigin, v3Trace);
        }
    }

    vector<CVec3f> vPoints(8);

    vPoints[0] = CVec3f(-1.0000521f, 1.0000215f, 1.0000000f);
    vPoints[1] = CVec3f(-0.048574682f, 0.048574682f, 0.00000000f);
    vPoints[2] = CVec3f(1.0000077f, 0.99999243f, 0.99999988f);
    vPoints[3] = CVec3f(0.048570041f, 0.048571203f, -1.0899137e-007f);
    vPoints[4] = CVec3f(-0.99995846f, -0.99990350f, 1.0000000f);
    vPoints[5] = CVec3f(-0.048572373f, -0.048565399f, -1.4532185e-007f);
    vPoints[6] = CVec3f(1.0001101f, -0.99993527f, 1.0000000f);
    vPoints[7] = CVec3f(0.048572373f, -0.048566561f, -1.4532185e-007f);

    CFrustum shadowFrustum(vPoints, vOrigin);

    float fFovX = DEG2RAD(shadowFrustum.GetFovX());
    float fFovY = DEG2RAD(shadowFrustum.GetFovY());

    float fTanX = tan(fFovX * 0.5f);
    float fTanY = tan(fFovY * 0.5f);

    float wl = -fTanX;
    float wr = fTanX;
    float wb = -fTanY;
    float wt = fTanY;

    CVec3f v3Right(shadowFrustum.GetAxis(RIGHT));
    CVec3f v3Dir(shadowFrustum.GetAxis(FORWARD));
    CVec3f v3Up(shadowFrustum.GetAxis(UP));

    CVec3f v3Offset(shadowFrustum.GetOrigin() + v3Dir);

    CVec3f av3BoundingPoints[4];
    av3BoundingPoints[0] = v3Right * wr + v3Up * wt + v3Offset; // top - right point
    av3BoundingPoints[1] = v3Right * wl + v3Up * wt + v3Offset; // top - left point
    av3BoundingPoints[2] = v3Right * wl + v3Up * wb + v3Offset; // bottom - left point
    av3BoundingPoints[3] = v3Right * wr + v3Up * wb + v3Offset; // bottom - right point


    SSceneFaceVert poly[1024];
    MemManager.Set(poly, 0, sizeof(poly));
    int p = 0;

    for (vector<CVec3f>::iterator it = vPoints.begin(); it != vPoints.end(); ++it)
    {
        CVec3_cast(poly[p].vtx) = *it;
        poly[p].col[0] = poly[p].col[1] = poly[p].col[2] = poly[p].col[3] = 255;

        poly[p].col[1] *= int(shadowFrustum.Touches(*it));
        poly[p].col[2] *= int(shadowFrustum.Touches(*it));
        ++p;
    }

    CVec3_cast(poly[p].vtx) = vOrigin;
    poly[p].col[0] = 255;
    poly[p].col[1] = 0;
    poly[p].col[2] = 0;
    poly[p].col[3] = 255;
    ++p;

    if (p > 0)
        SceneManager.AddPoly(p, poly, hMaterial, POLY_POINTLIST | POLY_NO_DEPTH_TEST);

    SSceneFaceVert plane[4];
    MemManager.Set(plane, 0, sizeof(plane));
    int q = 0;

    CVec3_cast(plane[q].vtx) = CVec3f(10.0f, 10.0f, 1.0210210);
    plane[q].col[0] = 0;
    plane[q].col[1] = 128;
    plane[q].col[2] = 0;
    plane[q].col[3] = 255;
    ++q;

    CVec3_cast(plane[q].vtx) = CVec3f(-10.0f, 10.0f, 1.0210210);
    plane[q].col[0] = 0;
    plane[q].col[1] = 128;
    plane[q].col[2] = 0;
    plane[q].col[3] = 255;
    ++q;

    CVec3_cast(plane[q].vtx) = CVec3f(-10.0f, -10.0f, 1.0210210);
    plane[q].col[0] = 0;
    plane[q].col[1] = 128;
    plane[q].col[2] = 0;
    plane[q].col[3] = 255;
    ++q;

    CVec3_cast(plane[q].vtx) = CVec3f(10.0f, -10.0f, 1.0210210);
    plane[q].col[0] = 0;
    plane[q].col[1] = 128;
    plane[q].col[2] = 0;
    plane[q].col[3] = 255;
    ++q;

    //if (q > 0)
    //  SceneManager.AddPoly(q, plane, hMaterial, POLY_DOUBLESIDED);

    SSceneFaceVert lines[14];
    MemManager.Set(lines, 0, sizeof(lines));
    int l = 0;

    CVec3_cast(lines[l].vtx) = vOrigin;
    lines[l].col[0] = 255;
    lines[l].col[1] = 0;
    lines[l].col[2] = 0;
    lines[l].col[3] = 255;
    ++l;

    CVec3_cast(lines[l].vtx) = vOrigin + v3Dir * 100.0f;
    lines[l].col[0] = 255;
    lines[l].col[1] = 0;
    lines[l].col[2] = 0;
    lines[l].col[3] = 255;
    ++l;

    CVec3_cast(lines[l].vtx) = vOrigin;
    lines[l].col[0] = 0;
    lines[l].col[1] = 255;
    lines[l].col[2] = 0;
    lines[l].col[3] = 255;
    ++l;

    CVec3_cast(lines[l].vtx) = vOrigin + shadowFrustum.GetAxis(RIGHT) * 100.0f;
    lines[l].col[0] = 0;
    lines[l].col[1] = 255;
    lines[l].col[2] = 0;
    lines[l].col[3] = 255;
    ++l;

    CVec3_cast(lines[l].vtx) = vOrigin;
    lines[l].col[0] = 0;
    lines[l].col[1] = 0;
    lines[l].col[2] = 255;
    lines[l].col[3] = 255;
    ++l;

    CVec3_cast(lines[l].vtx) = vOrigin + shadowFrustum.GetAxis(UP) * 100.0f;
    lines[l].col[0] = 0;
    lines[l].col[1] = 0;
    lines[l].col[2] = 255;
    lines[l].col[3] = 255;
    ++l;

    CVec3_cast(lines[l].vtx) = vOrigin;
    lines[l].col[0] = 0;
    lines[l].col[1] = 255;
    lines[l].col[2] = 255;
    lines[l].col[3] = 255;
    ++l;

    CVec3_cast(lines[l].vtx) = vOrigin + Normalize(av3BoundingPoints[0] - vOrigin) * 100.0f;
    lines[l].col[0] = 0;
    lines[l].col[1] = 255;
    lines[l].col[2] = 255;
    lines[l].col[3] = 255;
    ++l;

    CVec3_cast(lines[l].vtx) = vOrigin;
    lines[l].col[0] = 0;
    lines[l].col[1] = 255;
    lines[l].col[2] = 255;
    lines[l].col[3] = 255;
    ++l;

    CVec3_cast(lines[l].vtx) = vOrigin + Normalize(av3BoundingPoints[1] - vOrigin) * 100.0f;
    lines[l].col[0] = 0;
    lines[l].col[1] = 255;
    lines[l].col[2] = 255;
    lines[l].col[3] = 255;
    ++l;

    CVec3_cast(lines[l].vtx) = vOrigin;
    lines[l].col[0] = 0;
    lines[l].col[1] = 255;
    lines[l].col[2] = 255;
    lines[l].col[3] = 255;
    ++l;

    CVec3_cast(lines[l].vtx) = vOrigin + Normalize(av3BoundingPoints[2] - vOrigin) * 100.0f;
    lines[l].col[0] = 0;
    lines[l].col[1] = 255;
    lines[l].col[2] = 255;
    lines[l].col[3] = 255;
    ++l;

    CVec3_cast(lines[l].vtx) = vOrigin;
    lines[l].col[0] = 0;
    lines[l].col[1] = 255;
    lines[l].col[2] = 255;
    lines[l].col[3] = 255;
    ++l;

    CVec3_cast(lines[l].vtx) = vOrigin + Normalize(av3BoundingPoints[3] - vOrigin) * 100.0f;
    lines[l].col[0] = 0;
    lines[l].col[1] = 255;
    lines[l].col[2] = 255;
    lines[l].col[3] = 255;
    ++l;

    if (l > 0)
        SceneManager.AddPoly(l, lines, hMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
#endif

#if 0 // debug stuff
    Draw2D.SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    ResHandle hMaterial(g_ResourceManager.Register(_T("/core/materials/line.material"), RES_MATERIAL));

    CRectf recSelection[4] =
    {
        CRectf(0.5f, 0.0f, 1.0f, 0.5f),
        CRectf(0.0f, 0.0f, 0.5f, 0.5f),
        CRectf(0.0f, 0.5f, 0.5f, 1.0f),
        CRectf(0.5f, 0.5f, 1.0f, 1.0f)
    };

    CVec4b v4Color[4] =
    {
        CVec4b(255, 0, 0, 255),
        CVec4b(0, 255, 0, 255),
        CVec4b(0, 0, 255, 255),
        CVec4b(255, 255, 0, 255)
    };

    float fFovX = DEG2RAD(mv.camera.GetFovX());
    float fFovY = DEG2RAD(mv.camera.GetFovY());

    float fTanX = tan(fFovX * 0.5f);
    float fTanY = tan(fFovY * 0.5f);

    CVec2f v2Tan[4] =
    {
        CVec2f(tan(fFovX * 0.25f), tan(fFovY * 0.25f)),
        CVec2f(-tan(fFovX * 0.25f), tan(fFovY * 0.25f)),
        CVec2f(-tan(fFovX * 0.25f), -tan(fFovY * 0.25f)),
        CVec2f(tan(fFovX * 0.25f), -tan(fFovY * 0.25f))
    };

    CVec3f v3CamRight(mv.camera.GetViewAxis(RIGHT));
    CVec3f v3CamDir(mv.camera.GetViewAxis(FORWARD));
    CVec3f v3CamUp(mv.camera.GetViewAxis(UP));

    CVec3f v3CamOffset(mv.camera.GetOrigin() + v3CamDir);

    for (int iQuad(0); iQuad < 4; ++iQuad)
    {
        CVec3f v3Forward(Normalize(v3CamRight * v2Tan[iQuad].x + v3CamUp * v2Tan[iQuad].y + v3CamDir));

        vector<CVec3f> vPoints(4);

        float wl = fTanX * ((recSelection[iQuad].left - 0.5f) * 2.0f);
        float wr = fTanX * ((recSelection[iQuad].right - 0.5f) * 2.0f);
        float wt = -fTanY * ((recSelection[iQuad].top - 0.5f) * 2.0f);
        float wb = -fTanY * ((recSelection[iQuad].bottom - 0.5f) * 2.0f);

        vPoints[0] = v3CamRight * wr + v3CamUp * wt + v3CamOffset;  // top - right point
        vPoints[1] = v3CamRight * wl + v3CamUp * wt + v3CamOffset;  // top - left point
        vPoints[2] = v3CamRight * wl + v3CamUp * wb + v3CamOffset;  // bottom - left point
        vPoints[3] = v3CamRight * wr + v3CamUp * wb + v3CamOffset;  // bottom - right point

        CVec3f vFrustumForward(v3Forward);
        CVec3f vFrustumRight(Normalize(CrossProduct(vFrustumForward, mv.camera.GetViewAxis(UP))));
        CVec3f vFrustumUp(Normalize(CrossProduct(vFrustumRight, vFrustumForward)));

        CAxis aAxis(vFrustumRight, vFrustumForward, vFrustumUp);

        CFrustum splitFrustum(vPoints, mv.camera.GetOrigin(), aAxis);

        float fFovX0 = DEG2RAD(splitFrustum.GetFovX());
        float fFovY0 = DEG2RAD(splitFrustum.GetFovY());

        float fTanX0 = tan(fFovX0 * 0.5f);
        float fTanY0 = tan(fFovY0 * 0.5f);

        float wl0 = -fTanX0;
        float wr0 = fTanX0;
        float wt0 = fTanY0;
        float wb0 = -fTanY0;

        CVec3f v3Right(splitFrustum.GetAxis(RIGHT));
        CVec3f v3Dir(splitFrustum.GetAxis(FORWARD));
        CVec3f v3Up(splitFrustum.GetAxis(UP));

        CVec3f v3Offset(splitFrustum.GetOrigin() + v3Dir);

        CVec3f vPlanePoints[4];
        vPlanePoints[0] = v3Right * wr0 + v3Up * wt0 + v3Offset;    // top - right point
        vPlanePoints[1] = v3Right * wl0 + v3Up * wt0 + v3Offset;    // top - left point
        vPlanePoints[2] = v3Right * wl0 + v3Up * wb0 + v3Offset;    // bottom - left point
        vPlanePoints[3] = v3Right * wr0 + v3Up * wb0 + v3Offset;    // bottom - right point

        SSceneFaceVert plane[4];
        MemManager.Set(plane, 0, sizeof(plane));
        int q = 0;

        for (int i(0); i < 4; ++i)
        {
            CVec3_cast(plane[q].vtx) = vPlanePoints[i];
            plane[q].col = v4Color[iQuad];
            ++q;
        }

        if (q > 0)
            SceneManager.AddPoly(q, plane, hMaterial, POLY_DOUBLESIDED | POLY_WIREFRAME);
    }
#endif

#if 0
    ResHandle hMaterial(g_ResourceManager.Register(_T("/core/materials/line.material"), RES_MATERIAL));
    CVec3f v3Origin(2048.0f, 2048.0f, 0.0f);
    CVec3f v3Dir(Normalize(CVec3f(sin(MsToSec(g_pHostClient->GetTime())), cos(MsToSec(g_pHostClient->GetTime())), 0.0f)));

    SSceneFaceVert poly[256];
    MemManager.Set(poly, 0, sizeof(poly));
    int p = 0;

    for (int i(0); i < 256; ++i)
    {
        CVec3_cast(poly[p].vtx) = v3Origin + M_RotatePointAroundAxis(test_point, v3Dir, i / 256.0f * 360.0f);
        poly[p].col[0] = 255;
        poly[p].col[1] = 0;
        poly[p].col[2] = 0;
        poly[p].col[3] = 255;
        ++p;
    }

    if (p > 0)
        SceneManager.AddPoly(p, poly, hMaterial, POLY_POINTLIST | POLY_NO_DEPTH_TEST);

    SSceneFaceVert lines[4];
    MemManager.Set(lines, 0, sizeof(lines));
    int l = 0;

    CVec3_cast(lines[l].vtx) = v3Origin;
    lines[l].col[0] = 255;
    lines[l].col[1] = 0;
    lines[l].col[2] = 0;
    lines[l].col[3] = 255;
    ++l;

    CVec3_cast(lines[l].vtx) = v3Origin + v3Dir * DotProduct(CVec3f(test_point), v3Dir);
    lines[l].col[0] = 255;
    lines[l].col[1] = 0;
    lines[l].col[2] = 0;
    lines[l].col[3] = 255;
    ++l;

    CVec3_cast(lines[l].vtx) = v3Origin + v3Dir * DotProduct(CVec3f(test_point), v3Dir);
    lines[l].col[0] = 255;
    lines[l].col[1] = 0;
    lines[l].col[2] = 0;
    lines[l].col[3] = 255;
    ++l;

    CVec3_cast(lines[l].vtx) = v3Origin + M_RotatePointAroundAxis(test_point, v3Dir, MsToSec(g_pHostClient->GetTime()) * 360.0f);
    lines[l].col[0] = 255;
    lines[l].col[1] = 0;
    lines[l].col[2] = 0;
    lines[l].col[3] = 255;
    ++l;

    if (l > 0)
        SceneManager.AddPoly(l, lines, hMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
#endif
    
    mv.camera.SetAngles(CVec3f(mv_camPitch, mv_camRoll, mv_camYaw) + v3CameraAngleOffset);
    mv.camera.AddOffset(v3CameraOffset);

    if (mv_fogofWar)
    {
        //float fX(model_x);
        //float fY(model_y);
        //float fRange(500.0f);

        int iMultisample(ICvar::GetInteger(_T("vid_fogofwarmapMultisample")));
        int iSize(ICvar::GetInteger(_T("vid_fogofwarmapSize")));

        if (iMultisample > 0)
            iSize <<= iMultisample;
        else if (iMultisample < 0)
            iSize >>= -iMultisample;

        CBitmap bmp(iSize, iSize, BITMAP_ALPHA);
        byte *pBuffer(bmp.GetBuffer());

        MemManager.Set(pBuffer, 0, bmp.GetSize());

        //CL_RasterCircle(pBuffer, iSize, iSize / 2, iSize / 2, iSize / 4);
        CL_RasterCircle2(pBuffer, iSize, iSize / 2, iSize / 2, iSize / 3);
        //CL_RasterRect(pBuffer, iSize, 2, 2, iSize - 4, iSize - 4);

        Vid.UpdateFogofWar(bmp);
        Vid.RenderFogofWar(0.0f, true, 1.0f);
    }
    else
    {
        Vid.RenderFogofWar(1.0f, false, 0.0f);
    }
    
    SceneManager.Render();

#if 0
    Draw2D.RectOutline(0.0f, 0.0f, Draw2D.GetScreenW() / 2.0f, Draw2D.GetScreenH() / 2.0f, 1.0f, CVec4f(1.0f, 1.0f, 1.0f, 1.0f));
    Draw2D.RectOutline(Draw2D.GetScreenW() / 2.0f, 0.0f, Draw2D.GetScreenW() / 2.0f, Draw2D.GetScreenH() / 2.0f, 1.0f, CVec4f(1.0f, 1.0f, 1.0f, 1.0f));
    Draw2D.RectOutline(0.0f, Draw2D.GetScreenH() / 2.0f, Draw2D.GetScreenW() / 2.0f, Draw2D.GetScreenH() / 2.0f, 1.0f, CVec4f(1.0f, 1.0f, 1.0f, 1.0f));
    Draw2D.RectOutline(Draw2D.GetScreenW() / 2.0f, Draw2D.GetScreenH() / 2.0f, Draw2D.GetScreenW() / 2.0f, Draw2D.GetScreenH() / 2.0f, 1.0f, CVec4f(1.0f, 1.0f, 1.0f, 1.0f));
#endif

    for (int i(0); i < g_iNumPoints; ++i)
        Draw2D.Rect(g_av2Point[i].x * Draw2D.GetScreenH(), g_av2Point[i].y * Draw2D.GetScreenH(), 1.0f, 1.0f);

#ifdef PATH_ANALYZER
    MV_DrawPath();
#endif

    Ctrl.Trigger(XtoA(ModifierCtrl));
    Shift.Trigger(XtoA(ModifierShift));
    Alt.Trigger(XtoA(ModifierAlt));
}


/*====================
  CL_RasterHLine
  ====================*/
inline void CL_RasterHLine(byte *pBuffer, uint uiBufferSpan, int x0, int x1, int y)
{
#define SET_PIXEL(x, y)     pBuffer[(y) * uiBufferSpan + (x)] = 255

    for (; x0 <= x1; ++x0)
        SET_PIXEL(x0, y);

#undef SET_PIXEL
}


/*====================
  CL_RasterCircle
  ====================*/
void    CL_RasterCircle(byte *pBuffer, uint uiBufferSpan, int x0, int y0, int radius)
{
#define SET_PIXEL(x, y)     pBuffer[(y) * uiBufferSpan + (x)] = 255

    int f = 1 - radius;
    int ddF_x = 0;
    int ddF_y = -2 * radius;
    int x = 0;
    int y = radius;

    SET_PIXEL(x0, y0 + radius);
    SET_PIXEL(x0, y0 - radius);
    SET_PIXEL(x0 + radius, y0);
    SET_PIXEL(x0 - radius, y0);

    while(x < y) 
    {
        if(f >= 0) 
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }

        x++;
        ddF_x += 2;
        f += ddF_x + 1;    
        SET_PIXEL(x0 + x, y0 + y);
        SET_PIXEL(x0 - x, y0 + y);
        SET_PIXEL(x0 + x, y0 - y);
        SET_PIXEL(x0 - x, y0 - y);
        SET_PIXEL(x0 + y, y0 + x);
        SET_PIXEL(x0 - y, y0 + x);
        SET_PIXEL(x0 + y, y0 - x);
        SET_PIXEL(x0 - y, y0 - x);
    }
#undef SET_PIXEL
}


/*====================
  CL_RasterCircle2
  ====================*/
void    CL_RasterCircle2(byte *pBuffer, uint uiBufferSpan, int x0, int y0, int radius)
{
#define SET_PIXEL(x, y)     pBuffer[(y) * uiBufferSpan + (x)] = 255

    int f = 1 - radius;
    int ddF_x = 0;
    int ddF_y = -2 * radius;
    int x = 0;
    int y = radius;

    SET_PIXEL(x0, y0 + radius);
    SET_PIXEL(x0, y0 - radius);
    CL_RasterHLine(pBuffer, uiBufferSpan, x0 - radius, x0 + radius, y0);

    while (x < y) 
    {
        if (f >= 0) 
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }

        x++;
        ddF_x += 2;
        f += ddF_x + 1;

        CL_RasterHLine(pBuffer, uiBufferSpan, x0 - x, x0 + x, y0 + y);
        CL_RasterHLine(pBuffer, uiBufferSpan, x0 - x, x0 + x, y0 - y);
        CL_RasterHLine(pBuffer, uiBufferSpan, x0 - y, x0 + y, y0 + x);
        CL_RasterHLine(pBuffer, uiBufferSpan, x0 - y, x0 + y, y0 - x);
    }
#undef SET_PIXEL
}


/*====================
  CL_RasterRect
  ====================*/
void    CL_RasterRect(byte *pBuffer, uint uiBufferSpan, int x0, int y0, int x1, int y1)
{
    for (; y0 <= y1; ++y0)
        CL_RasterHLine(pBuffer, uiBufferSpan, x0, x1, y0);
}


/*====================
  CL_Shutdown
  ====================*/
void    CL_Shutdown()
{
    for (int i(0); i < NUM_MV_EFFECTS; i++)
    {
        if (mv.apEffectThread[i])
        {
            K2_DELETE(mv.apEffectThread[i]);
            mv.apEffectThread[i] = nullptr;
        }
    }
    MV_StopAllSounds();
}


/*====================
  CL_ProcessSnapshot
  ====================*/
bool    CL_ProcessSnapshot(CSnapshot &snapshot)
{
    return true;
}


/*====================
  CL_FileDropNotify
  ====================*/
void    CL_FileDropNotify(const tsvector &vsFiles)
{
    if (vsFiles.empty())
        return;

    for (tsvector_cit it(vsFiles.begin()); it != vsFiles.end(); ++it)
    {
        tstring sRoot(K2System.GetRootDir());
        tstring sModel(FileManager.SanitizePath(*it, false));
        if (sModel.substr(0, sRoot.length()) != sRoot)
            return;

        sModel = sModel.substr(sRoot.length());
        sModel = sModel.substr(sModel.find_first_of(_T("/")));

        if (CompareNoCase(Filename_GetExtension(sModel), _T("mdf")) == 0)
        {
            model = sModel;
            model.SetModified(true);
            Console.AddInputHistory(_TS("Set model \"") + sModel + _TS("\""));
        }
        else if (CompareNoCase(Filename_GetExtension(sModel), _T("effect")) == 0)
        {
            cmdStartEffect(sModel);
            Console.AddInputHistory(_TS("StartEffect \"") + sModel + _TS("\""));
        }
    }
}


/*====================
  InitLibrary
  ====================*/
K2_DLL_EXPORT void InitLibrary(CClientGameLib &GameLib)
{
    GameLib.SetName(K2System.GetGameName());
    GameLib.SetTypeName(_T("modelviewer"));
    GameLib.SetMajorVersion(1);
    GameLib.SetMinorVersion(1);
    GameLib.AssignInitFn(CL_Init);
    GameLib.AssignFrameFn(CL_Frame);
    GameLib.AssignShutdownFn(CL_Shutdown);
    GameLib.AssignSnapshotFn(CL_ProcessSnapshot);
    GameLib.AssignDropNotifyFn(CL_FileDropNotify);
}


/* Period parameters */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0df   /* constant vector a */
#define UPPER_MASK 0x80000000 /* most significant w-r bits */
#define LOWER_MASK 0x7fffffff /* least significant r bits */

/* Tempering parameters */
#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define TEMPERING_SHIFT_U(y)  (y >> 11)
#define TEMPERING_SHIFT_S(y)  (y << 7)
#define TEMPERING_SHIFT_T(y)  (y << 15)
#define TEMPERING_SHIFT_L(y)  (y >> 18)

static unsigned long mt[N]; /* the array for the state vector  */
static int mti=N+1; /* mti==N+1 means mt[N] is not initialized */

/* Initializing the array with a seed */
void sgenrand(unsigned long seed)
{
    int i;

    for (i=0;i<N;i++) {
         mt[i] = seed & 0xffff0000;
         seed = 69069 * seed + 1;
         mt[i] |= (seed & 0xffff0000) >> 16;
         seed = 69069 * seed + 1;
    }
    mti = N;
}


/*====================
  genrand
  ====================*/
unsigned long genrand()
{
    unsigned long y;
    static unsigned long mag01[2]={0x0, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (mti >= N) { /* generate N words at one time */
        int kk;

        if (mti == N+1)   /* if sgenrand() has not been called, */
            sgenrand(4357); /* a default initial seed is used   */

        for (kk=0;kk<N-M;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        for (;kk<N-1;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1];

        mti = 0;
    }

    y = mt[mti++];
    y ^= TEMPERING_SHIFT_U(y);
    y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
    y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
    y ^= TEMPERING_SHIFT_L(y);

    return y;
}


/*====================
  M_RandomMersenneInt
  ====================*/
int     M_RandomMersenneInt(int iLow, int iHigh)
{
    unsigned int iRange(iHigh - iLow + 1);

    if (iRange < 1)
    {
        return iLow;
    }
    else
    {
        unsigned int uiRand(genrand());
        int iOffset(uiRand % iRange);

        return (iLow + iOffset);
    }
}


/*====================
  M_RandomMersenneFloat
  ====================*/
float   M_RandomMersenneFloat(float fLow, float fHigh)
{
    float fRange(fHigh - fLow);

    if (fRange <= 0.0f)
    {
        return fLow;
    }
    else
    {
        unsigned int uiRand(genrand());
        float fOffset(static_cast<float>(uiRand) * 2.3283064370807974e-10f);

        return (fLow + fOffset * fRange);
    }
}


/*====================
  M_RandomMersenneFloat
  ====================*/
float   M_RandomMersenneFloat()
{
    unsigned int uiRand(genrand());
    return (static_cast<float>(uiRand) * 2.3283064370807974e-10f);
}


/*--------------------
  BlueNoise
  --------------------*/
CMD(BlueNoise)
{
    sgenrand(K2System.GetRandomSeed32());

    uint uiMsec(K2System.Milliseconds());

    int iSize(vArgList.size() > 0 ? AtoI(vArgList[0]) : 16);
    int iTries(vArgList.size() > 1 ? AtoI(vArgList[1]) : 16);

    CVec2f *av2Point[9] =
    {
        K2_NEW_ARRAY(ctx_ModelViewer, CVec2f, iSize),
        K2_NEW_ARRAY(ctx_ModelViewer, CVec2f, iSize),
        K2_NEW_ARRAY(ctx_ModelViewer, CVec2f, iSize),

        K2_NEW_ARRAY(ctx_ModelViewer, CVec2f, iSize),
        K2_NEW_ARRAY(ctx_ModelViewer, CVec2f, iSize),
        K2_NEW_ARRAY(ctx_ModelViewer, CVec2f, iSize),

        K2_NEW_ARRAY(ctx_ModelViewer, CVec2f, iSize),
        K2_NEW_ARRAY(ctx_ModelViewer, CVec2f, iSize),
        K2_NEW_ARRAY(ctx_ModelViewer, CVec2f, iSize)
    };

    CVec2f av2Offset[9] =
    {
        CVec2f(-1.0f, -1.0f),
        CVec2f( 0.0f, -1.0f),
        CVec2f( 1.0f, -1.0f),

        CVec2f(-1.0f,  0.0f),
        CVec2f( 0.0f,  0.0f),
        CVec2f( 1.0f,  0.0f),

        CVec2f(-1.0f,  1.0f),
        CVec2f( 0.0f,  1.0f),
        CVec2f( 1.0f,  1.0f)
    };

    for (int i(0); i < iSize; ++i)
    {
        CVec2f  v2BestPoint(0.0f, 0.0f);
        float   fBestDistance(0.0f);

        int iNumTries(iTries > 1 ? iTries * (i + 1) : iTries);
        //int iNumTries(iTries);

        for (int j(0); j < iNumTries; ++j)
        {
            CVec2f v2Point(M_RandomMersenneFloat(), M_RandomMersenneFloat());

            float fMinDistance(FAR_AWAY);

            for (int a(0); a < 9; ++a)
            {
                for (int k(0); k < i; ++k)
                {
                    float fDistance((av2Point[a][k].x - v2Point.x) * (av2Point[a][k].x - v2Point.x) + (av2Point[a][k].y - v2Point.y) * (av2Point[a][k].y - v2Point.y));

                    fMinDistance = MIN(fDistance, fMinDistance);
                }
            }

            if (fMinDistance > fBestDistance)
            {
                fBestDistance = fMinDistance;
                v2BestPoint = v2Point;

                //Console << _T("    ") << fMinDistance << _T("    ") << v2Point << newl;
            }
        }

        for (int b(0); b < 9; ++b)
            av2Point[b][i] = v2BestPoint + av2Offset[b];

        //Console << av2Point[4][i] << newl;
    }

    CFileHandle hFile(_T("~/blue_noise.h"), FILE_WRITE | FILE_TEXT);

    for (int l(0); l < iSize; ++l)
        hFile << _T("{") << XtoA(av2Point[4][l].x, 0, 0, 8) << _T("f, ") << XtoA(av2Point[4][l].y, 0, 0, 8) << _T("f},") << newl;

    Console << _T("BlueNoise saved to ") << _T("blue_noise.h") << newl;
    Console << _T("BlueNoise took ") << K2System.Milliseconds() - uiMsec << _T("ms") << newl;

    if (g_av2Point)
        K2_DELETE_ARRAY(g_av2Point);

    g_iNumPoints = iSize;
    g_av2Point = K2_NEW_ARRAY(ctx_ModelViewer, CVec2f, iSize);

    for (int m(0); m < iSize; ++m)
        g_av2Point[m] = av2Point[4][m];

    for (int c(0); c < 9; ++c)
        K2_DELETE_ARRAY(av2Point[c]);

    return true;
}


/*--------------------
  ModelMaterialList
  --------------------*/
CMD(ModelMaterialList)
{
    if (mv.hActiveModel == INVALID_RESOURCE)
        return true;

    CModel *pModel(g_ResourceManager.GetModel(mv.hActiveModel));
    if (pModel && pModel->GetModelFile()->GetType() == MODEL_K2)
    {
        Console << _T("<skin name=\"default\">") << newl;

        CK2Model* pK2Model(static_cast<CK2Model*>(pModel->GetModelFile()));
        for (uint ui(0); ui < pK2Model->GetNumMeshes(); ++ui)
        {
            CMesh *pMesh(pK2Model->GetMesh(ui));

            Console << _T("<reference mesh=\"") << pMesh->GetName() << _T("\" material=\"") << pMesh->GetDefaultShaderName() << _T(".material\" />") << newl;
        }

        Console << _T("</skin>") << newl;
    }

    return true;
}


/*--------------------
  GetTerrainType
  --------------------*/
FUNCTION(GetTerrainType)
{
    return _T("grass");
}


/*--------------------
  ReformatMaterials
  --------------------*/
CMD(ReformatMaterials)
{
    if (vArgList.size() < 1)
        return false;

    tsvector vFileList;
    FileManager.GetFileList(vArgList[0], _T("*.material"), true, vFileList, true);

    tsvector_it itEnd(vFileList.end());
    for (tsvector_it it(vFileList.begin()); it != itEnd; ++it)
    {
        CFileHandle hFile(*it, FILE_READ | FILE_BINARY);

        CXMLDoc cXml;

        uint uiLength(0);
        const char *pBuffer(hFile.GetBuffer(uiLength));

        if (cXml.ReadBuffer(pBuffer, uiLength))
            cXml.WriteFile(*it, true);
    }

    return true;
}


/*--------------------
  ReformatMaterial
  --------------------*/
CMD(ReformatMaterial)
{
    if (vArgList.size() < 1)
        return false;

    CFileHandle hFile(vArgList[0], FILE_READ | FILE_BINARY);

    CXMLDoc cXml;

    uint uiLength(0);
    const char *pBuffer(hFile.GetBuffer(uiLength));

    if (cXml.ReadBuffer(pBuffer, uiLength))
    {
        cXml.WriteFile(vArgList[0], true);
    }

    return true;
}


/*--------------------
  TestFormatFloat
  --------------------*/
CMD(TestFormatFloat)
{
    if (vArgList.size() < 3)
        return false;

    Console << XtoA(AtoF(vArgList[0]), 0, 0, AtoI(vArgList[1]), AtoI(vArgList[2])) << newl;
    return true;
}


/*--------------------
  RenameFiles
  --------------------*/
CMD(RenameFiles)
{
    if (vArgList.size() < 3)
        return false;

    tsvector vFileList;
    FileManager.GetFileList(vArgList[0], vArgList[1], true, vFileList, true);

    tsvector_it itEnd(vFileList.end());
    for (tsvector_it it(vFileList.begin()); it != itEnd; ++it)
    {
        CFileHandle hInFile(*it, FILE_READ | FILE_BINARY);
        CFileHandle hOutFile(Filename_GetPath(*it) + vArgList[2], FILE_WRITE | FILE_BINARY);

        uint uiSize;
        const char *pBuffer(hInFile.GetBuffer(uiSize));

        hOutFile.Write(pBuffer, uiSize);
    }

    return true;
}


#ifdef PATH_ANALYZER

PoolHandle      g_hPath(INVALID_POOL_HANDLE);
CVec2f          g_v2PathStart;
CVec2f          g_v2PathEnd;
bool            g_bPathStartValid;
bool            g_bPathEndValid;
const float     PATH_TILE_SIZE(16.0f);

CVAR_INT(mv_pathStep, 0);
CVAR_FLOAT(mv_pathRadius, 0.0f);
CVAR_FLOAT(mv_pathRange, 0.0f);
CVAR_FLOAT(mv_blockerSize, 0.0f);

/*--------------------
  Path
  --------------------*/
CMD(Path)
{
    if (vArgList.size() < 4)
        return false;

    CVec2f v2Start(AtoF(vArgList[0]), AtoF(vArgList[1]));
    CVec2f v2End(AtoF(vArgList[2]), AtoF(vArgList[3]));

    if (g_hPath != INVALID_POOL_HANDLE)
        g_pWorld->FreePath(g_hPath);

    g_hPath = g_pWorld->FindPath(v2Start, 0.0f, INVALID_INDEX, v2End, 0.0f);

    return true;
}


/*====================
  MV_FindPath
  ====================*/
void    MV_FindPath()
{
    if (g_bPathStartValid && g_bPathEndValid)
    {
        if (g_hPath != INVALID_POOL_HANDLE)
            g_pWorld->FreePath(g_hPath);

        if (mv_blockerSize > 0.0f)
        {
            vector<PoolHandle>  vBlockers;
            vBlockers.push_back(g_pWorld->BlockPath(NAVIGATION_CLIFF, g_v2PathEnd, mv_blockerSize, mv_blockerSize));

            g_pWorld->UpdateNavigation();

            g_hPath = g_pWorld->FindPath(g_v2PathStart, mv_pathRadius, INVALID_INDEX, g_v2PathEnd, mv_pathRange, &vBlockers);

            g_pWorld->ClearPath(vBlockers[0]);
        }
        else
        {
            g_pWorld->UpdateNavigation();

            g_hPath = g_pWorld->FindPath(g_v2PathStart, mv_pathRadius, INVALID_INDEX, g_v2PathEnd, mv_pathRange);
        }
    }
}


/*====================
  MV_DrawNavGrid
  ====================*/
void    MV_DrawNavGrid()
{
    const CVec4f GRID_COLOR(0.33f, 0.33f, 0.33f, 1.0f);

    if (!g_pWorld->IsLoaded())
        return;

    CNavigationMap &cNavigationMap(g_pWorld->GetNavigationMap());

    CNavGridZ *pNavGrid(cNavigationMap.PrepForSearch(NAVIGATION_UNIT | NAVIGATION_CLIFF, 0));

    if (pNavGrid == nullptr)
        return;

    uint *pHorizontal(pNavGrid->GetHorizontal());
    uint *pVertical(pNavGrid->GetVertical());

    uint uiNavigationWidth(pNavGrid->GetWidth());
    uint uiNavigationHeight(pNavGrid->GetHeight());
    uint uiIntsPerRow(pNavGrid->GetIntsPerRow());

    float fGateScale(PATH_TILE_SIZE);
    float fTileScale(PATH_TILE_SIZE);

    uint uiHorizontalStartX(0);
    uint uiHorizontalStartY(0);
    uint uiHorizontalEndX(CLAMP(INT_ROUND(Draw2D.GetScreenW() / PATH_TILE_SIZE), 0, int(uiIntsPerRow * 32)));
    uint uiHorizontalEndY(CLAMP(INT_ROUND(Draw2D.GetScreenH() / PATH_TILE_SIZE) - 1, 0, int(uiNavigationHeight) - 1));

    // Horizontal gates
    for (uint uiY(uiHorizontalStartY); uiY != uiHorizontalEndY; ++uiY)
    {
        float fY((uiY + 1) * fTileScale + 0.5f);

        for (uint uiX(uiHorizontalStartX); uiX != uiHorizontalEndX; ++uiX)
        {
            float fX0((uiX) * fGateScale + 0.5f);
            float fX1((uiX + 1) * fGateScale + 0.5f);
            
            if (pHorizontal[uiY * uiIntsPerRow + (uiX >> 5)] & BIT(31 - (uiX & 31)))
                Draw2D.Line(CVec2f(fX0, fY), CVec2f(fX1, fY), GRID_COLOR, GRID_COLOR);
            else
                Draw2D.Line(CVec2f(fX0, fY), CVec2f(fX1, fY), RED, RED);
        }
    }

    uint uiVerticalStartX(0);
    uint uiVerticalStartY(0);
    uint uiVerticalEndX(CLAMP(INT_ROUND(Draw2D.GetScreenW() / PATH_TILE_SIZE) - 1, 0, int(uiNavigationWidth) - 1));
    uint uiVerticalEndY(CLAMP(INT_ROUND(Draw2D.GetScreenH() / PATH_TILE_SIZE), 0, int(uiIntsPerRow * 32)));

    // Vertical gates
    for (uint uiX(uiVerticalStartX); uiX != uiVerticalEndX; ++uiX)
    {
        float fX((uiX + 1) * fTileScale + 0.5f);

        for (uint uiY(uiVerticalStartY); uiY != uiVerticalEndY; ++uiY)
        {
            float fY0((uiY) * fGateScale + 0.5f);
            float fY1((uiY + 1) * fGateScale + 0.5f);

            if (pVertical[uiX * uiIntsPerRow + (uiY >> 5)] & BIT(31 - (uiY & 31)))
                Draw2D.Line(CVec2f(fX, fY0), CVec2f(fX, fY1), GRID_COLOR, GRID_COLOR);
            else
                Draw2D.Line(CVec2f(fX, fY0), CVec2f(fX, fY1), RED, RED);
        }
    }
}


map<uint, PoolHandle>   g_mapBlockers;

/*====================
  MV_AddBlocker
  ====================*/
void    MV_AddBlocker()
{
    if (g_pWorld == nullptr)
        return;

    CVec2f v2Pos(Input.GetCursorPos() * (g_pWorld->GetNavigationScale() / PATH_TILE_SIZE));

    float fNavigationScale(g_pWorld->GetNavigationScale());

    uint uiX(INT_FLOOR(v2Pos.x / fNavigationScale));
    uint uiY(INT_FLOOR(v2Pos.y / fNavigationScale));

    if (uiX >= uint(g_pWorld->GetNavigationWidth()) || uiY >= uint(g_pWorld->GetNavigationHeight()))
        return;

    uint uiKey(uiY * g_pWorld->GetNavigationWidth() + uiX);

    if (g_mapBlockers.find(uiKey) != g_mapBlockers.end())
        return;

    float fX(uiX * fNavigationScale);
    float fY(uiY * fNavigationScale);
    float fWidth(fNavigationScale);
    float fHeight(fNavigationScale);

    g_mapBlockers[uiKey] = g_pWorld->GetNavigationMap().AddBlocker(NAVIGATION_UNIT, fX, fY, fWidth, fHeight);

    g_pWorld->UpdateNavigation();

    MV_FindPath();
}


/*====================
  MV_ClearBlocker
  ====================*/
void    MV_ClearBlocker()
{
    if (g_pWorld == nullptr)
        return;

    CVec2f v2Pos(Input.GetCursorPos() * (g_pWorld->GetNavigationScale() / PATH_TILE_SIZE));

    float fNavigationScale(g_pWorld->GetNavigationScale());

    uint uiX(INT_FLOOR(v2Pos.x / fNavigationScale));
    uint uiY(INT_FLOOR(v2Pos.y / fNavigationScale));

    if (uiX >= uint(g_pWorld->GetNavigationWidth()) || uiY >= uint(g_pWorld->GetNavigationHeight()))
        return;

    uint uiKey(uiY * g_pWorld->GetNavigationWidth() + uiX);
    
    map<uint, PoolHandle>::iterator itFind(g_mapBlockers.find(uiKey));

    if (itFind == g_mapBlockers.end())
        return;

    g_pWorld->GetNavigationMap().ClearBlocker(itFind->second);
    g_mapBlockers.erase(itFind);

    g_pWorld->UpdateNavigation();

    MV_FindPath();
}


/*====================
  MV_DrawHoverTile
  ====================*/
void    MV_DrawHoverTile()
{
    tstring s;
    CVec2f v2Cursor(Input.GetCursorPos());

    uint uiX(INT_FLOOR(v2Cursor.x / PATH_TILE_SIZE));
    uint uiY(INT_FLOOR(v2Cursor.y / PATH_TILE_SIZE));

    if (uiX >= uint(g_pWorld->GetNavigationWidth()) || uiY >= uint(g_pWorld->GetNavigationHeight()))
        return;

    if (!g_pWorld->GetNavigationMap().IsReady() || g_pWorld->GetNavigationGraph().GetNode(0, 0) == nullptr)
        return;

    CNavigationGraph &cNavGraph(g_pWorld->GetNavigationGraph());

    CSearchNode *pNode(g_pWorld->GetNavigationGraph().GetNode(uiX, uiY));

    v2Cursor.x += 32.0f;

    if (pNode->IsReset())
    {
        Draw2D.SetColor(LIME);
        Draw2D.RectOutline(CRectf(uiX * PATH_TILE_SIZE, uiY * PATH_TILE_SIZE, (uiX + 1) * PATH_TILE_SIZE, (uiY + 1) * PATH_TILE_SIZE), 1);

        Draw2D.SetColor(CVec4f(0.0f, 0.1f, 0.0f, 1.0f));
        Draw2D.Rect(CRectf(v2Cursor.x - 1, v2Cursor.y - 1, v2Cursor.x + 66, v2Cursor.y + 14), g_ResourceManager.GetWhiteTexture());

        Draw2D.SetColor(LIME);
        Draw2D.RectOutline(CRectf(v2Cursor.x - 1, v2Cursor.y - 1, v2Cursor.x + 66, v2Cursor.y + 14), 1);

        s = _T("(") + XtoA(uiX) + _T(", ") + XtoA(uiY) + _T(")");
        Draw2D.SetColor(LIME);
        Draw2D.String(v2Cursor.x, v2Cursor.y, s, g_ResourceManager.LookUpName(_T("system_small"), RES_FONTMAP));
    }
    else
    {
        CSearchNode *pDrawNode0(pNode);
        CSearchNode *pDrawNode1(pDrawNode0->ParentDirection() != SD_INVALID ? cNavGraph.FindNeighbor(pDrawNode0, pDrawNode0->ParentDirection()) : nullptr);
        while (pDrawNode1 != nullptr)
        {
            CVec2f v0, v1;

            if (pDrawNode0->ParentDirection() == SD_NORTH)
                v0 = CVec2f((cNavGraph.GetNodeX(pDrawNode0) + 0.5f) * PATH_TILE_SIZE + 0.5f, (cNavGraph.GetNodeY(pDrawNode0) + 1.0f) * PATH_TILE_SIZE + 0.5f);
            else if (pDrawNode0->ParentDirection() == SD_EAST)
                v0 = CVec2f((cNavGraph.GetNodeX(pDrawNode0) + 1.0f) * PATH_TILE_SIZE + 0.5f, (cNavGraph.GetNodeY(pDrawNode0) + 0.5f) * PATH_TILE_SIZE + 0.5f);
            else if (pDrawNode0->ParentDirection() == SD_WEST)
                v0 = CVec2f((cNavGraph.GetNodeX(pDrawNode0) + 0.0f) * PATH_TILE_SIZE + 0.5f, (cNavGraph.GetNodeY(pDrawNode0) + 0.5f) * PATH_TILE_SIZE + 0.5f);
            else if (pDrawNode0->ParentDirection() == SD_SOUTH)
                v0 = CVec2f((cNavGraph.GetNodeX(pDrawNode0) + 0.5f) * PATH_TILE_SIZE + 0.5f, (cNavGraph.GetNodeY(pDrawNode0) + 0.0f) * PATH_TILE_SIZE + 0.5f);
            else
                v0 = CVec2f((cNavGraph.GetNodeX(pDrawNode0) + 0.5f) * PATH_TILE_SIZE + 0.5f, (cNavGraph.GetNodeY(pDrawNode0) + 0.5f) * PATH_TILE_SIZE + 0.5f);

            if (pDrawNode1->ParentDirection() == SD_NORTH)
                v1 = CVec2f((cNavGraph.GetNodeX(pDrawNode1) + 0.5f) * PATH_TILE_SIZE + 0.5f, (cNavGraph.GetNodeY(pDrawNode1) + 1.0f) * PATH_TILE_SIZE + 0.5f);
            else if (pDrawNode1->ParentDirection() == SD_EAST)
                v1 = CVec2f((cNavGraph.GetNodeX(pDrawNode1) + 1.0f) * PATH_TILE_SIZE + 0.5f, (cNavGraph.GetNodeY(pDrawNode1) + 0.5f) * PATH_TILE_SIZE + 0.5f);
            else if (pDrawNode1->ParentDirection() == SD_WEST)
                v1 = CVec2f((cNavGraph.GetNodeX(pDrawNode1) + 0.0f) * PATH_TILE_SIZE + 0.5f, (cNavGraph.GetNodeY(pDrawNode1) + 0.5f) * PATH_TILE_SIZE + 0.5f);
            else if (pDrawNode1->ParentDirection() == SD_SOUTH)
                v1 = CVec2f((cNavGraph.GetNodeX(pDrawNode1) + 0.5f) * PATH_TILE_SIZE + 0.5f, (cNavGraph.GetNodeY(pDrawNode1) + 0.0f) * PATH_TILE_SIZE + 0.5f);
            else
                v1 = CVec2f((cNavGraph.GetNodeX(pDrawNode1) + 0.5f) * PATH_TILE_SIZE + 0.5f, (cNavGraph.GetNodeY(pDrawNode1) + 0.5f) * PATH_TILE_SIZE + 0.5f);

            Draw2D.Line(v0, v1, LIME, LIME);

            pDrawNode0 = pDrawNode1;
            pDrawNode1 = pDrawNode0->ParentDirection() != SD_INVALID ? cNavGraph.FindNeighbor(pDrawNode0, pDrawNode0->ParentDirection()) : nullptr;
        }

        Draw2D.SetColor(LIME);
        Draw2D.RectOutline(CRectf(uiX * PATH_TILE_SIZE, uiY * PATH_TILE_SIZE, (uiX + 1) * PATH_TILE_SIZE, (uiY + 1) * PATH_TILE_SIZE), 1);

        Draw2D.SetColor(CVec4f(0.0f, 0.1f, 0.0f, 1.0f));
        Draw2D.Rect(CRectf(v2Cursor.x - 1, v2Cursor.y - 1, v2Cursor.x + 66, v2Cursor.y + 54), g_ResourceManager.GetWhiteTexture());

        Draw2D.SetColor(LIME);
        Draw2D.RectOutline(CRectf(v2Cursor.x - 1, v2Cursor.y - 1, v2Cursor.x + 66, v2Cursor.y + 54), 1);

        s = _T("(") + XtoA(uiX) + _T(", ") + XtoA(uiY) + _T(")");
        Draw2D.SetColor(LIME);
        Draw2D.String(v2Cursor.x, v2Cursor.y, s, g_ResourceManager.LookUpName(_T("system_small"), RES_FONTMAP));

        v2Cursor.y += 10.0f;

        s = _T("g: ") + XtoA(pNode->GetCost());
        Draw2D.SetColor(LIME);
        Draw2D.String(v2Cursor.x, v2Cursor.y, s, g_ResourceManager.LookUpName(_T("system_small"), RES_FONTMAP));

        v2Cursor.y += 10.0f;

        s = _T("h: ") + XtoA(pNode->GetHeuristic() - pNode->GetCost());
        Draw2D.SetColor(LIME);
        Draw2D.String(v2Cursor.x, v2Cursor.y, s, g_ResourceManager.LookUpName(_T("system_small"), RES_FONTMAP));

        v2Cursor.y += 10.0f;

        s = _T("f: ") + XtoA(pNode->GetHeuristic());
        Draw2D.SetColor(LIME);
        Draw2D.String(v2Cursor.x, v2Cursor.y, s, g_ResourceManager.LookUpName(_T("system_small"), RES_FONTMAP));

        v2Cursor.y += 10.0f;

        s = _T("b: ") + XtoA(pNode->GetBias());
        Draw2D.SetColor(LIME);
        Draw2D.String(v2Cursor.x, v2Cursor.y, s, g_ResourceManager.LookUpName(_T("system_small"), RES_FONTMAP));
    }
}


/*====================
  MV_DrawNodes
  ====================*/
void    MV_DrawNodes()
{
    static ResHandle s_hTextures[] =
    {
        g_ResourceManager.Register(K2_NEW(ctx_ModelViewer,  CTexture)(_T("/test/textures/down.tga"), TEXTURE_2D, TEX_FULL_QUALITY | TEX_NO_COMPRESS, TEXFMT_A8R8G8B8), RES_TEXTURE),
        g_ResourceManager.Register(K2_NEW(ctx_ModelViewer,  CTexture)(_T("/test/textures/right.tga"), TEXTURE_2D, TEX_FULL_QUALITY | TEX_NO_COMPRESS, TEXFMT_A8R8G8B8), RES_TEXTURE),
        g_ResourceManager.Register(K2_NEW(ctx_ModelViewer,  CTexture)(_T("/test/textures/left.tga"), TEXTURE_2D, TEX_FULL_QUALITY | TEX_NO_COMPRESS, TEXFMT_A8R8G8B8), RES_TEXTURE),
        g_ResourceManager.Register(K2_NEW(ctx_ModelViewer,  CTexture)(_T("/test/textures/up.tga"), TEXTURE_2D, TEX_FULL_QUALITY | TEX_NO_COMPRESS, TEXFMT_A8R8G8B8), RES_TEXTURE),
        g_ResourceManager.Register(K2_NEW(ctx_ModelViewer,  CTexture)(_T("/test/textures/circle.tga"), TEXTURE_2D, TEX_FULL_QUALITY | TEX_NO_COMPRESS, TEXFMT_A8R8G8B8), RES_TEXTURE)
    };

    if (!g_pWorld->GetNavigationMap().IsReady() || g_pWorld->GetNavigationGraph().GetNode(0, 0) == nullptr)
        return;

    uint uiGridWidth(MIN(INT_ROUND(Draw2D.GetScreenW() / PATH_TILE_SIZE), g_pWorld->GetNavigationWidth()));
    uint uiGridHeight(MIN(INT_ROUND(Draw2D.GetScreenH() / PATH_TILE_SIZE), g_pWorld->GetNavigationHeight()));

    for (uint uiY(0); uiY < uiGridHeight; ++uiY)
    {
        for (uint uiX(0); uiX < uiGridWidth; ++uiX)
        {
            CSearchNode *pNode(g_pWorld->GetNavigationGraph().GetNode(uiX, uiY));

            if (!pNode->IsReset())
            {
                CRectf rec(uiX * PATH_TILE_SIZE, uiY * PATH_TILE_SIZE, (uiX + 1) * PATH_TILE_SIZE, (uiY + 1) * PATH_TILE_SIZE);

                if (pNode->GetFlags() & SN_NOTLISTED)
                    Draw2D.SetColor(WHITE);
                else
                    Draw2D.SetColor(ORANGE);

                Draw2D.Rect(rec, s_hTextures[pNode->ParentDirection()]);
            }
        }
    }

    Draw2D.SetColor(WHITE);
}


/*====================
  MV_DrawPath
  ====================*/
void    MV_DrawPath()
{
    if (g_pWorld == nullptr || !g_pWorld->IsLoaded() || &g_pWorld->GetNavigationMap() == nullptr)
        return;

    ICvar::SetBool(_T("g_PathDetail"), true);
    ICvar::SetUnsignedInteger(_T("g_PathMaxDownsize"), 0);
    ICvar::SetBool(_T("g_PathDelayReset"), true);

    if (ClearBlocker)
        MV_ClearBlocker();

    if (AddBlocker)
        MV_AddBlocker();

    if (mv_pathStep.IsModified())
    {
        if (mv_pathStep < 0)
            mv_pathStep = 0;

        if (mv_pathStep > 0)
        {
            ICvar::SetInteger(_T("g_PathBidirectionalChanges"), 0);
            ICvar::SetUnsignedInteger(_T("g_PathTimeoutNodes"), mv_pathStep);
        }
        else
        {
            ICvar::Reset(_T("g_PathBidirectionalChanges"));
            ICvar::Reset(_T("g_PathTimeoutNodes"));
        }

        mv_pathStep.SetModified(false);

        MV_FindPath();
    }

    g_pWorld->UpdateNavigation();

    MV_DrawNavGrid();

    MV_DrawNodes();

    if (g_hPath == INVALID_POOL_HANDLE)
    {
        MV_DrawHoverTile();
        return;
    }

    CPath *pPath(g_pWorld->AccessPath(g_hPath));

    float fDrawScale(PATH_TILE_SIZE / g_pWorld->GetNavigationScale());

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
            case SD_NORTH:  v2Dir = CVec2f(-1.0f, 0.0f);    break;
            case SD_EAST:   v2Dir = CVec2f(0.0f, 1.0f);     break;
            case SD_SOUTH:  v2Dir = CVec2f(1.0f, 0.0f);     break;
            case SD_WEST:   v2Dir = CVec2f(0.0f, -1.0f);    break;
            }

            v2PosGate = v2Dir * vecInfoPath[i].GetRadiusPositive();
            v2PosGate += v2Src;

            v2NegGate = v2Dir * -vecInfoPath[i].GetRadiusNegative();
            v2NegGate += v2Src;

            if (vecInfoPath[i].Direction() != SD_INVALID)
            {
                Draw2D.Line(
                    CVec2f(v2PosGate.x * fDrawScale + 0.5f, v2PosGate.y * fDrawScale + 0.5f),
                    CVec2f(v2NegGate.x * fDrawScale + 0.5f, v2NegGate.y * fDrawScale + 0.5f), 
                    vecInfoPath[i].GetColor(),
                    vecInfoPath[i].GetColor());
            }
#endif
            {
                CVec4f v4PathLineColor(0.0f, 1.0f, 0.0f, 1.0f);

                Draw2D.Line(
                    CVec2f(v2Src.x * fDrawScale + 0.5f, v2Src.y * fDrawScale + 0.5f),
                    CVec2f(v2End.x * fDrawScale + 0.5f, v2End.y * fDrawScale + 0.5f),
                    v4PathLineColor,
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

            Draw2D.Line(
                CVec2f(v2Src.x * fDrawScale + 0.5f, v2Src.y * fDrawScale + 0.5f),
                CVec2f(v2Dst.x * fDrawScale + 0.5f, v2Dst.y * fDrawScale + 0.5f),
                WHITE,
                WHITE);
        }
    }

    MV_DrawHoverTile();
}


/*====================
  MV_PathSetStart
  ====================*/
void    MV_PathSetStart()
{
    g_v2PathStart = Input.GetCursorPos() * (g_pWorld->GetNavigationScale() / PATH_TILE_SIZE);
    g_bPathStartValid = true;

    MV_FindPath();
}


/*====================
  MV_PathSetEnd
  ====================*/
void    MV_PathSetEnd()
{
    g_v2PathEnd = Input.GetCursorPos() * (g_pWorld->GetNavigationScale() / PATH_TILE_SIZE);
    g_bPathEndValid = true;

    MV_FindPath();
}


/*====================
  MV_PathClear
  ====================*/
void    MV_PathClear()
{
    if (g_hPath != INVALID_POOL_HANDLE)
    {
        g_pWorld->FreePath(g_hPath);
        g_hPath = INVALID_POOL_HANDLE;
    }

    g_bPathStartValid = false;
    g_bPathEndValid = false;
}


/*--------------------
  PathSetStart
  --------------------*/
ACTION_IMPULSE(PathSetStart)
{
    MV_PathSetStart();
}


/*--------------------
  PathSetEnd
  --------------------*/
ACTION_IMPULSE(PathSetEnd)
{
    MV_PathSetEnd();
}


/*--------------------
  PathClear
  --------------------*/
ACTION_IMPULSE(PathClear)
{
    MV_PathClear();
}

#endif


CBufferBit g_cBitTest;

CMD(WriteBits)
{
    if (vArgList.size() < 2)
        return false;

    g_cBitTest.WriteBits(AtoUI(vArgList[0]), AtoUI(vArgList[1]));
    return true;
}

CMD(ReadBits)
{
    if (vArgList.size() < 1)
        return false;

    Console << XtoA(g_cBitTest.ReadBits(AtoUI(vArgList[0]))) << newl;
    return true;
}

CMD(ClearBits)
{
    g_cBitTest.Clear();
    return true;
}


CMD(TestBits)
{
    g_cBitTest.Clear();

    sgenrand(K2System.GetRandomSeed32());

    vector<pair<uint, uint> > vValues(1000000);

    for (int i(0); i < 1000000; ++i)
    {
        pair<uint, uint> uuValue(genrand(), M_Randnum(1, 32));

        g_cBitTest.WriteBits(uuValue.first, uuValue.second);

        uuValue.first &= uint(-1) >> (32 - uuValue.second);

        vValues[i] = uuValue;
    }

    for (int i(0); i < 1000000; ++i)
    {
        if (g_cBitTest.ReadBits(vValues[i].second) != vValues[i].first)
        {
            Console << _T("FRAK!") << newl;
        }
    }

    return true;
}

CMD(TestDeltaPos)
{
    g_cBitTest.Clear();

    sgenrand(K2System.GetRandomSeed32());

    const int NUM_TESTS(100000);

    vector<short> vValues(NUM_TESTS);

    // code:
    // 1 + 5
    // 0,1 + 6
    // 0,0 + 15 

    for (int i(0); i < NUM_TESTS; ++i)
    {
        short p(M_RandomMersenneInt(SHRT_MIN, SHRT_MAX));

        // Map -2,-1,0,1,2 to 0,-1,1,-2,2
        uint uiCode(p >= 0 ? uint(p) << 1 : (uint(-p) << 1) - 1);

        if (uiCode < 32)
        {
            g_cBitTest.WriteBit(1); // 1
            g_cBitTest.WriteBits(uiCode, 5);
        }
        else if (uiCode < 32 + 64)
        {
            g_cBitTest.WriteBits(2, 2); // 0,1
            g_cBitTest.WriteBits(uiCode - 32, 6);
        }
        else
        {
            g_cBitTest.WriteBits(0, 2);
            g_cBitTest.WriteBits(uiCode, 16);
        }

        vValues[i] = p;
    }

    for (int i(0); i < NUM_TESTS; ++i)
    {
        uint uiCode(g_cBitTest.ReadBit());
        if (uiCode != 0)
            uiCode = g_cBitTest.ReadBits(5);
        else
        {
            uiCode = g_cBitTest.ReadBit();
            if (uiCode != 0)
                uiCode = g_cBitTest.ReadBits(6) + 32;
            else
                uiCode = g_cBitTest.ReadBits(16);
        }

        // Map 0,-1,1,-2,2 to -2,-1,0,1,2
        short p(uiCode & 1 ? short(-int(uiCode + 1) >> 1) : short(uiCode >> 1));

        if (p != vValues[i])
        {
            Console << _T("FRAK!") << newl;
        }
    }

    return true;
}


// need external linkage for template params
class CAbs
{
public:
    bool operator()(const pair<int, int> &a, const pair<int, int> &b)
    {
        return (ABS(a.first) < ABS(b.first));
    }
};


CMD(CalcCodes)
{
    if (vArgList.size() < 1)
        return false;

    CFileHandle hFile(vArgList[0], FILE_READ | FILE_TEXT);

    typedef pair<int, int> iipair;

    vector<iipair> vData;

    while (!hFile.IsEOF())
    {
        iipair iiData;
        tstring sLine(hFile.ReadLine());
        vector<tstring> vsFields(TokenizeString(sLine, _T(' ')));

        if (vsFields.size() >= 2)
        {
            iiData.first = AtoI(vsFields[0]);
            iiData.second = AtoI(vsFields[1]);

            if (iiData.first < 0)
                iiData.first *= -2;
            else if (iiData.first != 0)
                iiData.first = iiData.first * 2 - 1;

            vData.push_back(iiData);
        }
    }

    sort(vData.begin(), vData.end(), CAbs());

    {
        vector<iipair> vPairs;
        vPairs.push_back(iipair(1, 0));
        vPairs.push_back(iipair(1, 15));

        vector<iipair> vBestPairs;
        int iBestBits(INT_MAX);

        for (int a(0); a < 15; ++a)
        {
            vPairs[0].second = a;

            int iBits(0);

            for (vector<iipair>::iterator it(vData.begin()), itEnd(vData.end()); it != itEnd; ++it)
            {
                int iMax(0);
                int iSize(0);
                for (vector<iipair>::iterator it2(vPairs.begin()), it2End(vPairs.end()); it2 != it2End && iMax <= it->first; ++it2)
                {
                    iMax += BIT(it2->second);
                    iSize = it2->first + it2->second;
                }

                iBits += iSize * it->second;
            }

            if (iBits < iBestBits)
            {
                iBestBits = iBits;
                vBestPairs = vPairs;
            }
        }

        Console << vBestPairs[0].second;
        for (vector<iipair>::iterator it2(vBestPairs.begin() + 1), it2End(vBestPairs.end()); it2 != it2End; ++it2)
            Console << _T(",") << it2->second;
            
        Console << _T(" ") << XtoA(CEIL_MULTIPLE<8>(iBestBits) / 8, FMT_DELIMIT) << _T(" bytes") << newl;
    }

    {
        vector<iipair> vPairs;
        vPairs.push_back(iipair(1, 0));
        vPairs.push_back(iipair(2, 0));
        vPairs.push_back(iipair(2, 15));

        vector<iipair> vBestPairs;
        int iBestBits(INT_MAX);

        for (int a(0); a < 15; ++a)
        {
            vPairs[0].second = a;

            for (int b(0); b < 15; ++b)
            {
                vPairs[1].second = b;

                int iBits(0);

                for (vector<iipair>::iterator it(vData.begin()), itEnd(vData.end()); it != itEnd; ++it)
                {
                    int iMax(0);
                    int iSize(0);
                    for (vector<iipair>::iterator it2(vPairs.begin()), it2End(vPairs.end()); it2 != it2End && iMax <= it->first; ++it2)
                    {
                        iMax += BIT(it2->second);
                        iSize = it2->first + it2->second;
                    }

                    iBits += iSize * it->second;
                }

                if (iBits < iBestBits)
                {
                    iBestBits = iBits;
                    vBestPairs = vPairs;
                }

#if 0
                Console << vPairs[0].second;
                for (vector<iipair>::iterator it2(vPairs.begin() + 1), it2End(vPairs.end()); it2 != it2End; ++it2)
                    Console << _T(",") << it2->second;
                    
                Console << _T(" ") << XtoA(CEIL_MULTIPLE<8>(iBits) / 8, FMT_DELIMIT) << _T(" bytes") << newl;
#endif
            }
        }

        Console << vBestPairs[0].second;
        for (vector<iipair>::iterator it2(vBestPairs.begin() + 1), it2End(vBestPairs.end()); it2 != it2End; ++it2)
            Console << _T(",") << it2->second;
            
        Console << _T(" ") << XtoA(CEIL_MULTIPLE<8>(iBestBits) / 8, FMT_DELIMIT) << _T(" bytes") << newl;
    }

#if 0
    {
        vector<iipair> vPairs;
        vPairs.push_back(iipair(1, 0));
        vPairs.push_back(iipair(2, 0));
        vPairs.push_back(iipair(3, 0));
        vPairs.push_back(iipair(3, 15));

        vector<iipair> vBestPairs;
        int iBestBits(INT_MAX);

        for (int a(0); a < 15; ++a)
        {
            vPairs[0].second = a;

            for (int b(0); b < 15; ++b)
            {
                vPairs[1].second = b;

                for (int c(0); c < 15; ++c)
                {
                    vPairs[2].second = c;

                    int iBits(0);

                    for (vector<iipair>::iterator it(vData.begin()), itEnd(vData.end()); it != itEnd; ++it)
                    {
                        int iMax(0);
                        int iSize(0);
                        for (vector<iipair>::iterator it2(vPairs.begin()), it2End(vPairs.end()); it2 != it2End && iMax <= it->first; ++it2)
                        {
                            iMax += BIT(it2->second);
                            iSize = it2->first + it2->second;
                        }

                        iBits += iSize * it->second;
                    }

                    if (iBits < iBestBits)
                    {
                        iBestBits = iBits;
                        vBestPairs = vPairs;
                    }

                    Console << vPairs[0].second;
                    for (vector<iipair>::iterator it2(vPairs.begin() + 1), it2End(vPairs.end()); it2 != it2End; ++it2)
                        Console << _T(",") << it2->second;
                        
                    Console << _T(" ") << XtoA(CEIL_MULTIPLE<8>(iBits) / 8, FMT_DELIMIT) << _T(" bytes") << newl;
                }
            }
        }

        Console << vBestPairs[0].second;
        for (vector<iipair>::iterator it2(vBestPairs.begin() + 1), it2End(vBestPairs.end()); it2 != it2End; ++it2)
            Console << _T(",") << it2->second;
            
        Console << _T(" ") << XtoA(CEIL_MULTIPLE<8>(iBestBits) / 8, FMT_DELIMIT) << _T(" bytes") << newl;
    }


    {
        vector<iipair> vPairs;
        vPairs.push_back(iipair(1, 0));
        vPairs.push_back(iipair(2, 0));
        vPairs.push_back(iipair(3, 0));
        vPairs.push_back(iipair(4, 0));
        vPairs.push_back(iipair(4, 15));

        vector<iipair> vBestPairs;
        int iBestBits(INT_MAX);

        for (int a(0); a < 15; ++a)
        {
            vPairs[0].second = a;

            for (int b(0); b < 15; ++b)
            {
                vPairs[1].second = b;

                for (int c(0); c < 15; ++c)
                {
                    vPairs[2].second = c;

                    for (int d(0); d < 15; ++d)
                    {
                        vPairs[3].second = d;

                        int iBits(0);

                        for (vector<iipair>::iterator it(vData.begin()), itEnd(vData.end()); it != itEnd; ++it)
                        {
                            int iMax(0);
                            int iSize(0);
                            for (vector<iipair>::iterator it2(vPairs.begin()), it2End(vPairs.end()); it2 != it2End && iMax <= it->first; ++it2)
                            {
                                iMax += BIT(it2->second);
                                iSize = it2->first + it2->second;
                            }

                            iBits += iSize * it->second;
                        }

                        if (iBits < iBestBits)
                        {
                            iBestBits = iBits;
                            vBestPairs = vPairs;
                        }
                    }
                }
            }
        }

        Console << vBestPairs[0].second;
        for (vector<iipair>::iterator it2(vBestPairs.begin() + 1), it2End(vBestPairs.end()); it2 != it2End; ++it2)
            Console << _T(",") << it2->second;
            
        Console << _T(" ") << XtoA(CEIL_MULTIPLE<8>(iBestBits) / 8, FMT_DELIMIT) << _T(" bytes") << newl;
    }
#endif

    return true;
}

CMD(TestTransmitFlags)
{
    byte ayTransmitFlagTree[64];
    byte ayTransmitFlagTree2[64];
    for (uint uiTreeWidth(8); uiTreeWidth <= 16; uiTreeWidth <<= 1)
    {
        uint uiNumBytes(CEIL_MULTIPLE<4>(uiTreeWidth) >> 2);
        uint uiNumTests(1 << (uiTreeWidth - 1));

        for (uint uiTest(0); uiTest != uiNumTests; ++uiTest)
        {
            {
                MemManager.Set(ayTransmitFlagTree, 0, uiNumBytes * sizeof(byte));
                
                // Setup the transmit flag tree
                for (uint uiField(0); uiField < uiTreeWidth; ++uiField)
                    if (uiTest & BIT(uiField))
                        K2_SetVectorBit(ayTransmitFlagTree, uiTreeWidth + uiField);

                K2_SetTreeBits(ayTransmitFlagTree, uiTreeWidth);
            }

            {
                MemManager.Set(ayTransmitFlagTree2, 0, uiNumBytes * sizeof(byte));

                // Setup the transmit flag tree
                for (uint uiField(0); uiField < uiTreeWidth; ++uiField)
                    if (uiTest & BIT(uiField))
                        K2_SetTreeBit(ayTransmitFlagTree2, uiTreeWidth, uiField);
            }

            if (memcmp(ayTransmitFlagTree, ayTransmitFlagTree2, uiNumBytes) != 0)
            {
                Console << _T("Failed test #") << uiTest << newl;
                break;
            }
        }
    }

    return true;
}


#if 0
CMD(ProfileTransmitFlags)
{
    byte ayTransmitFlagTree[64];
    byte ayTransmitFlagTree2[64];

    uint uiTreeWidth(32);
    uint uiNumBytes(CEIL_MULTIPLE<4>(uiTreeWidth) >> 2);
    uint uiNumTests(10000000);

    uint uiStartTime;

    uiStartTime = K2System.Microseconds();

    for (uint uiTest(0); uiTest != uiNumTests; ++uiTest)
    {
        MemManager.Set(ayTransmitFlagTree2, 0, uiNumBytes * sizeof(byte));

        // Setup the transmit flag tree
        for (uint uiField(0); uiField < uiTreeWidth; ++uiField)
            if (uiTest & BIT(uiField))
                K2_SetTreeBit2(ayTransmitFlagTree2, uiTreeWidth, uiField);
    }

    Console << _T("Newer: ") << K2System.Microseconds() - uiStartTime << newl;

    uiStartTime = K2System.Microseconds();

    for (uint uiTest(0); uiTest != uiNumTests; ++uiTest)
    {
        // Setup the transmit flag tree
        for (uint uiField(0); uiField < uiTreeWidth; ++uiField)
            if (uiTest & BIT(uiField))
                K2_SetVectorBit(ayTransmitFlagTree, uiTreeWidth + uiField);
        K2_SetTreeBits(ayTransmitFlagTree, uiTreeWidth);
    }

    Console << _T("New: ") << K2System.Microseconds() - uiStartTime << newl;

    uiStartTime = K2System.Microseconds();

    for (uint uiTest(0); uiTest != uiNumTests; ++uiTest)
    {
        MemManager.Set(ayTransmitFlagTree2, 0, uiNumBytes * sizeof(byte));

        // Setup the transmit flag tree
        for (uint uiField(0); uiField < uiTreeWidth; ++uiField)
            if (uiTest & BIT(uiField))
                K2_SetTreeBit(ayTransmitFlagTree2, uiTreeWidth, uiField);
    }

    Console << _T("Old: ") << K2System.Microseconds() - uiStartTime << newl;

    return true;
}
#endif


CMD(RegisterCursor)
{
    if (vArgList.size() < 1)
        return false;

    ResHandle hCursor(g_ResourceManager.Register(vArgList[0], RES_K2CURSOR));
    CCursor *pCursor(g_ResourceManager.GetCursor(hCursor));
    if (pCursor == nullptr)
        return false;

    return true;
}


/*--------------------
  LoadAllModels
  --------------------*/
CMD(LoadAllModels)
{
    tsvector vFileList;
    FileManager.GetFileList(_T("/"), _T("*.mdf"), true, vFileList, true);

    for (tsvector_it it(vFileList.begin()); it != vFileList.end(); ++it)
    {
        ResHandle hModel(g_ResourceManager.Register(*it, RES_MODEL));
        g_ResourceManager.PrecacheSkin(hModel, uint(-1));
    }

    return true;
}


/*--------------------
  ConvertTexturesTGA
  --------------------*/
CMD(ConvertTexturesTGA)
{
    if (vArgList.size() < 2)
        return false;

    tsvector vFileList;
    FileManager.GetFileList(vArgList[0], vArgList[1], true, vFileList, true);

    tsvector_it itEnd(vFileList.end());
    for (tsvector_it it(vFileList.begin()); it != itEnd; ++it)
    {
        CBitmap cBitmap(*it);
        cBitmap.WriteTGA(_T("~/converted") + Filename_StripExtension(*it) + _T(".tga"));
    }

    return true;
}


/*--------------------
  FixTextures
  --------------------*/
CMD(FixTextures)
{
    if (vArgList.size() < 2)
        return false;

    tsvector vFileList;
    FileManager.GetFileList(vArgList[0], vArgList[1], true, vFileList, true);

    tsvector_it itEnd(vFileList.end());
    for (tsvector_it it(vFileList.begin()); it != itEnd; ++it)
    {
        CBitmap cBitmap(*it);
        cBitmap.Flip();
        cBitmap.WriteTGA(_T("~/fixed") + Filename_StripExtension(*it) + _T(".tga"));
    }

    return true;
}
