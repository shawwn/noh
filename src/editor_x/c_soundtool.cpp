// (C)2007 S2 Games
// c_soundtool.cpp
//
// Sound Tool
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "editor_common.h"

#include "editor.h"

#include "c_soundtool.h"

#include "../k2/c_brush.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_vid.h"
#include "../k2/c_input.h"
#include "../k2/c_draw2d.h"
#include "../k2/intersection.h"
#include "../k2/c_function.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_world.h"
#include "../k2/c_uitrigger.h"
#include "../k2/c_worldsound.h"
#include "../k2/c_soundmanager.h"
#include "../k2/c_sample.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
CVAR_INT    (le_soundEditMode, SOUND_CREATE);
CVAR_INT    (le_soundCenterMode, CENTER_AVERAGE);
CVAR_BOOL   (le_soundSnap, false);
CVAR_BOOL   (le_soundHoverDelete, true);
CVAR_FLOAT  (le_soundRaiseSensitivity, 1.5f);
CVAR_FLOAT  (le_soundHeightSnap, 10.0f);
CVAR_INT    (le_soundVertexSelectSize, 16);
CVAR_STRING (le_soundModel, "/world/props/widgets/ambientsound/ambientsound.mdf");

CVAR_STRINGF(cg_worldAmbientSound,              "",         CVAR_WORLDCONFIG);
CVAR_FLOATF (cg_worldAmbientSoundVolume,        1.0,        CVAR_WORLDCONFIG);

UI_TRIGGER  (SoundEditMode);
UI_TRIGGER  (SoundSelection);
UI_TRIGGER  (SoundSelectionVolume);
//=============================================================================

/*====================
  CSoundTool::CSoundTool()
  ====================*/
CSoundTool::CSoundTool() :
ITool(TOOL_SOUND, _T("sound")),
m_bCloning(false),
m_vTranslate(0.0f, 0.0f, 0.0f),
m_hLineMaterial(g_ResourceManager.Register(_T("/core/materials/line.material"), RES_MATERIAL)),
m_uiHoverIndex(INVALID_INDEX),
m_hWorldAmbientSound(INVALID_INDEX),
m_hSoundModel(g_ResourceManager.Register(le_soundModel, RES_MODEL)),
m_pSoundModelSkeleton(NULL)
{
    SoundEditMode.Trigger(_T("Create"));
}


/*====================
  CSoundTool::PrimaryUp

  Left mouse button up action
  ====================*/
void    CSoundTool::PrimaryUp()
{
    if (m_bCloning && m_vStartCursorPos == Input.GetCursorPos())
    {
        m_bCloning = false;
        Delete();

        // Cancel current action
        m_vTranslate = m_vTrueTranslate = V_ZERO;
        m_iState = STATE_HOVERING;

        m_setSelection = m_setOldSelection;
        m_vStartCursorPos.Clear();
        return;
    }

    // Apply the operation
    switch(m_iState)
    {
    case STATE_SELECT:
        ApplySelect();
        break;

    case STATE_TRANSLATE_XY:
    case STATE_TRANSLATE_Z:
        ApplyTransform();
        break;
    }

    m_bCloning = false;
    m_iState = STATE_HOVERING;
    m_vStartCursorPos.Clear();
}


/*====================
  CSoundTool::PrimaryDown

  Default left mouse button down action
  ====================*/
void    CSoundTool::PrimaryDown()
{
    CalcToolProperties();

    switch(le_soundEditMode)
    {
    case SOUND_CREATE:
        Create();
        break;

    case SOUND_SELECT:
        StartSelect();
        break;

    case SOUND_TRANSLATE_XY:
        StartTranslateXY();
        break;

    case SOUND_TRANSLATE_Z:
        StartTransform(STATE_TRANSLATE_Z);
        break;
    }
}


/*====================
  CSoundTool::SecondaryDown

  Default right mouse button down action
  ====================*/
void    CSoundTool::SecondaryDown()
{
    if (m_bCloning)
    {
        m_bCloning = false;
        Delete();
        m_setSelection = m_setOldSelection;
    }

    // Cancel current action
    m_vTranslate = m_vTrueTranslate = V_ZERO;
    m_iState = STATE_HOVERING;

    // TODO: show context menu
}


/*====================
  CSoundTool::Cancel
  ====================*/
void    CSoundTool::Cancel()
{
    m_setSelection.clear();
    SoundSelection.Trigger(XtoA(!m_setSelection.empty()));
}


/*====================
  CSoundTool::GetSoundPosition
  ====================*/
CVec3f  CSoundTool::GetSoundPosition(uint uiIndex)
{
    try
    {
        CWorldSound *pSound(Editor.GetWorld().GetSound(uiIndex));

        if (pSound == NULL)
            EX_ERROR(_T("Invalid sound index: ") + XtoA(uiIndex));

        return pSound->GetPosition();
    }
    catch (CException &ex)
    {
        ex.Process(_T("CSoundTool::GetSoundPosition() - "), NO_THROW);
        return V_ZERO;
    }
}


/*====================
  CSoundTool::Delete
  ====================*/
void    CSoundTool::Delete()
{
    if (!m_setSelection.empty())
    {
        while (!m_setSelection.empty())
        {
            uint uiIndex(*m_setSelection.begin());
            StopSound(uiIndex);
            Editor.GetWorld().DeleteSound(uiIndex);
            m_setSelection.erase(uiIndex);
        }

        m_uiHoverIndex = INVALID_INDEX;

        SoundSelection.Trigger(XtoA(false));
    }
    else if (m_uiHoverIndex != INVALID_INDEX && le_soundHoverDelete)
    {
        m_setSelection.erase(m_uiHoverIndex);
        StopSound(m_uiHoverIndex);
        Editor.GetWorld().DeleteSound(m_uiHoverIndex);
        m_uiHoverIndex = INVALID_INDEX;

        SoundSelection.Trigger(XtoA(!m_setSelection.empty()));
    }
}


/*====================
  CSoundTool::CursorSoundTrace

  Traces against all sounds in the world
  ====================*/
bool     CSoundTool::CursorSoundTrace(CSoundTraceResult &result)
{
    result.Clear();
    CVec2f v2CursorPos(Input.GetCursorPos());

    WorldSoundsMap& mapSounds(Editor.GetWorld().GetSoundsMap());
    for (WorldSoundsMap_it it(mapSounds.begin()); it != mapSounds.end(); ++it)
    {
        CWorldSound &sound(*it->second);

        CVec2f v2ScreenPos;
        if (Editor.GetCamera().WorldToScreen(sound.GetPosition(), v2ScreenPos))
        {
            if (ABS(v2CursorPos.x - v2ScreenPos.x) < le_soundVertexSelectSize &&
                ABS(v2CursorPos.y - v2ScreenPos.y) < le_soundVertexSelectSize &&
                Distance(v2CursorPos, v2ScreenPos) < result.fFraction)
            {
                result.uiIndex = sound.GetIndex();
                result.fFraction = Distance(v2CursorPos, v2ScreenPos);
                result.v3EndPos = sound.GetPosition();
                result.pSound = &sound;
            }
        }
    }

    // Do a trace against the terrain seeing we didn't hit anything
    if (result.uiIndex == INVALID_INDEX)
    {
        m_Trace.uiIndex = INVALID_INDEX;

        CVec3f v3Dir(Editor.GetCamera().ConstructRay(v2CursorPos));
        CVec3f v3End(M_PointOnLine(Editor.GetCamera().GetOrigin(), v3Dir, FAR_AWAY));

        STraceInfo  trace;
        if (Editor.GetWorld().TraceLine(trace, Editor.GetCamera().GetOrigin(), v3End, TRACE_TERRAIN))
        {
            m_Trace.v3EndPos = trace.v3EndPos;
            m_Trace.fFraction = trace.fFraction;
            return true;
        }

        return false;
    }

    return true;
}


/*====================
  CSoundTool::CalcToolProperties
  ====================*/
void     CSoundTool::CalcToolProperties()
{
    if (m_iState == STATE_HOVERING)
    {
        if (CursorSoundTrace(m_Trace))
        {
            m_iX = Editor.GetWorld().GetVertFromCoord(m_Trace.v3EndPos.x);
            m_iY = Editor.GetWorld().GetVertFromCoord(m_Trace.v3EndPos.y);

            m_v3EndPos = m_Trace.v3EndPos;
            m_uiHoverIndex = m_Trace.uiIndex;
        }
        else
        {
            m_iX = -1;
            m_iY = -1;

            m_v3EndPos.Clear();
            m_uiHoverIndex = INVALID_INDEX;
            m_Trace.Clear();
        }
    }
    else
    {
        STraceInfo trace;
        if (Editor.TraceCursor(trace, TRACE_TERRAIN))
        {
            m_iX = Editor.GetWorld().GetVertFromCoord(trace.v3EndPos[0]);
            m_iY = Editor.GetWorld().GetVertFromCoord(trace.v3EndPos[1]);

            m_v3EndPos = trace.v3EndPos;
            m_Trace.Clear();
        }
        else
        {
            m_iX = -1;
            m_iY = -1;

            m_v3EndPos.Clear();
            m_uiHoverIndex = INVALID_INDEX;
            m_Trace.Clear();
        }
    }
}


/*====================
  CSoundTool::CreateSound
  ====================*/
uint    CSoundTool::CreateSound(const CVec3f &v3Pos)
{
    try
    {
        uint uiNewSound(Editor.GetWorld().AllocateNewSound());
        CWorldSound *pNewSound(Editor.GetWorld().GetSound(uiNewSound, true));

        pNewSound->SetPosition(v3Pos);
        pNewSound->SetSound(_T(""));
        pNewSound->SetVolume(1.0f);
        pNewSound->SetFalloff(250.0f);
        pNewSound->SetLoopDelay(0, 0);

        StartSound(uiNewSound);

        return uiNewSound;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CSoundTool::CreateSound() - "), NO_THROW);
        return INVALID_INDEX;
    }
}


/*====================
  CSoundTool::Hovering
  ====================*/
void    CSoundTool::Hovering()
{
}


/*====================
  CSoundTool::TranslateXY
  ====================*/
void    CSoundTool::TranslateXY()
{
    if (Input.GetCursorPos() != m_vStartCursorPos)
    {
        if (m_bSnapCursor) // to delay the first update until the mouse moves
        {
            CVec3f v3Snap;

            if (le_soundCenterMode == CENTER_HOVER || le_soundCenterMode == CENTER_INDIVIDUAL && m_uiHoverIndex != INVALID_INDEX)
                v3Snap = GetSoundPosition(m_uiHoverIndex);
            else
                v3Snap = SelectionCenter();

            v3Snap.z = Editor.GetWorld().GetTerrainHeight(v3Snap.x, v3Snap.y);
            SnapCursor(v3Snap);
            m_bSnapCursor = false;
            return;
        }

        m_vTrueTranslate = m_v3EndPos;

        if (le_soundSnap)
        {
            m_vTranslate.x = m_vTrueTranslate.x - fmod(m_vTrueTranslate.x, Editor.GetWorld().GetScale());
            m_vTranslate.y = m_vTrueTranslate.y - fmod(m_vTrueTranslate.y, Editor.GetWorld().GetScale());
            m_vTranslate.z = m_vTrueTranslate.z;
        }
        else
            m_vTranslate = m_vTrueTranslate;

        m_vStartCursorPos.Clear();
    }
}


/*====================
  CSoundTool::TranslateZ
  ====================*/
void    CSoundTool::TranslateZ()
{
    CVec2f pos = Input.GetCursorPos();

    float dY = pos.y - m_vOldCursorPos.y;

    if (dY != 0.0f)
    {
        m_vTrueTranslate.z -= dY * le_soundRaiseSensitivity;

        if (le_soundSnap)
            m_vTranslate.z = m_vTrueTranslate.z - fmod(m_vTrueTranslate.z, le_soundHeightSnap);
        else
            m_vTranslate.z = m_vTrueTranslate.z;
    }
}


/*====================
  CSoundTool::Create
  ====================*/
void    CSoundTool::Create()
{
    if (m_uiHoverIndex == INVALID_INDEX)
    {
        if (m_iX < 0 || m_iY < 0)
            return;

        if (!m_bModifier2)
        {
            m_setSelection.clear();
            SoundSelection.Trigger(XtoA(!m_setSelection.empty()));
        }

        CVec3f  vPos = m_v3EndPos;

        if (le_soundSnap)
        {
            vPos.x -= fmod(vPos.x, Editor.GetWorld().GetScale());
            vPos.y -= fmod(vPos.y, Editor.GetWorld().GetScale());
            vPos.z -= fmod(vPos.z, Editor.GetWorld().GetScale());
        }

        vPos.z += 150.0f;

        uint uiSound = CreateSound(vPos);
        if (uiSound != INVALID_INDEX && !m_bModifier1 && !m_bModifier3)
        {
            m_setSelection.insert(uiSound);
            SoundSelection.Trigger(XtoA(!m_setSelection.empty()));
        }

        m_iState = STATE_HOVERING;
    }
    else
    {
        if (m_bModifier2)
        {
            if (m_setSelection.find(m_uiHoverIndex) == m_setSelection.end())
                m_setSelection.insert(m_uiHoverIndex);
            else
                m_setSelection.erase(m_uiHoverIndex);
        }
        else if (m_bModifier3)
        {
            m_setSelection.erase(m_uiHoverIndex);
        }
        else
        {
            m_setSelection.clear();
            m_setSelection.insert(m_uiHoverIndex);
        }

        SoundSelection.Trigger(XtoA(!m_setSelection.empty()));

        m_iState = STATE_HOVERING;
    }
}


/*====================
  CSoundTool::StartSelect
  ====================*/
void    CSoundTool::StartSelect()
{
    if (m_uiHoverIndex == INVALID_INDEX)
    {
        m_iState = STATE_SELECT;
        m_vStartCursorPos = Input.GetCursorPos();
    }
    else
    {
        if (m_bModifier2)
        {
            if (m_setSelection.find(m_uiHoverIndex) == m_setSelection.end())
                m_setSelection.insert(m_uiHoverIndex);
            else
                m_setSelection.erase(m_uiHoverIndex);

            SoundSelection.Trigger(XtoA(!m_setSelection.empty()));
        }
        else if (m_bModifier3)
        {
            m_setSelection.erase(m_uiHoverIndex);
            SoundSelection.Trigger(XtoA(!m_setSelection.empty()));
        }
        else
        {
            if (m_setSelection.find(m_uiHoverIndex) == m_setSelection.end())
            {
                m_setSelection.clear();
                m_setSelection.insert(m_uiHoverIndex);
                SoundSelection.Trigger(XtoA(!m_setSelection.empty()));
            }
        }

        m_iState = STATE_HOVERING;
        m_vStartCursorPos = Input.GetCursorPos();
    }
}


/*====================
  CSoundTool::StartTranslateXY
  ====================*/
void    CSoundTool::StartTranslateXY()
{
    if (m_uiHoverIndex == INVALID_INDEX)
    {
        m_iState = STATE_SELECT;
        m_vStartCursorPos = Input.GetCursorPos();
    }
    else if (m_uiHoverIndex != INVALID_INDEX)
    {
        if (m_bModifier2)
        {
            if (m_setSelection.find(m_uiHoverIndex) == m_setSelection.end())
                m_setSelection.insert(m_uiHoverIndex);
            else
                m_setSelection.erase(m_uiHoverIndex);

            SoundSelection.Trigger(XtoA(!m_setSelection.empty()));
        }
        else if (m_bModifier3)
        {
            m_setSelection.erase(m_uiHoverIndex);
            SoundSelection.Trigger(XtoA(!m_setSelection.empty()));
        }
        else
        {
            if (m_setSelection.find(m_uiHoverIndex) == m_setSelection.end())
            {
                m_setSelection.clear();
                m_setSelection.insert(m_uiHoverIndex);
                SoundSelection.Trigger(XtoA(!m_setSelection.empty()));
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

            if (le_soundCenterMode == CENTER_HOVER || le_soundCenterMode == CENTER_INDIVIDUAL)
                m_vTranslate = GetSoundPosition(m_uiHoverIndex);
            else
                m_vTranslate = SelectionCenter();

            m_bSnapCursor = true;
        }

        m_vTranslate.z = Editor.GetWorld().GetTerrainHeight(m_vTranslate.x, m_vTranslate.y);

        m_vTrueTranslate = m_vTranslate;
        m_vStartCursorPos = Input.GetCursorPos();
    }
}


/*====================
  CSoundTool::StartTransform
  ====================*/
void    CSoundTool::StartTransform(int iState)
{
    if (m_uiHoverIndex == INVALID_INDEX)
    {
        m_iState = STATE_SELECT;
        m_vStartCursorPos = Input.GetCursorPos();
    }
    else
    {
        if (m_bModifier2)
        {
            if (m_setSelection.find(m_uiHoverIndex) == m_setSelection.end())
                m_setSelection.insert(m_uiHoverIndex);
            else
                m_setSelection.erase(m_uiHoverIndex);

            SoundSelection.Trigger(XtoA(!m_setSelection.empty()));
        }
        else if (m_bModifier3)
        {
            m_setSelection.erase(m_uiHoverIndex);
            SoundSelection.Trigger(XtoA(!m_setSelection.empty()));
        }
        else
        {
            if (m_setSelection.find(m_uiHoverIndex) == m_setSelection.end())
            {
                m_setSelection.clear();
                m_setSelection.insert(m_uiHoverIndex);
                SoundSelection.Trigger(XtoA(!m_setSelection.empty()));
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

        m_vTranslate = m_vTrueTranslate = V_ZERO;
        m_vStartCursorPos = Input.GetCursorPos();
    }
}


/*====================
  CSoundTool::ApplySelect
  ====================*/
void    CSoundTool::ApplySelect()
{
    if (!(m_bModifier2 || m_bModifier3))
        m_setSelection.clear();

    CRectf rect(MIN(m_vStartCursorPos.x, Input.GetCursorPos().x),
        MIN(m_vStartCursorPos.y, Input.GetCursorPos().y),
        MAX(m_vStartCursorPos.x, Input.GetCursorPos().x),
        MAX(m_vStartCursorPos.y, Input.GetCursorPos().y));


    WorldSoundsMap& mapSounds(Editor.GetWorld().GetSoundsMap());
    for (WorldSoundsMap_it it(mapSounds.begin()); it != mapSounds.end(); ++it)
    {
        CWorldSound &sound(*it->second);
        if (Editor.GetCamera().IsPointInScreenRect(sound.GetPosition(), rect))
        {
            if (m_bModifier2)
                m_setSelection.insert(sound.GetIndex());
            else if (m_bModifier3)
                m_setSelection.erase(sound.GetIndex());
            else
                m_setSelection.insert(sound.GetIndex());
        }
    }

    SoundSelection.Trigger(XtoA(!m_setSelection.empty()));
}


/*====================
  CSoundTool::ApplyTransform
  ====================*/
void    CSoundTool::ApplyTransform()
{
    try
    {
        CVec3f vSelectionCenter(SelectionCenter());

        for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
        {
            CVec3f v3Center;

            switch(le_soundCenterMode)
            {
            case CENTER_HOVER:
                if (m_uiHoverIndex != INVALID_INDEX)
                    v3Center = GetSoundPosition(m_uiHoverIndex);
                else
                    v3Center = GetSoundPosition(*it);
                break;

            case CENTER_AVERAGE:
                v3Center = vSelectionCenter;
                break;

            case CENTER_INDIVIDUAL:
                if (m_uiHoverIndex != INVALID_INDEX && m_iState == STATE_TRANSLATE_XY)
                    v3Center = GetSoundPosition(m_uiHoverIndex);
                else
                    v3Center = GetSoundPosition(*it);
                break;
            }

            CWorldSound *pSound(Editor.GetWorld().GetSound(*it));
            CVec3f v3Diff(pSound->GetPosition() - v3Center);
            CVec3f v3Origin;
            if (m_iState == STATE_TRANSLATE_XY) // use m_VTranslate as the absolute position
            {
                float fzOffset(v3Center.z - Editor.GetWorld().GetTerrainHeight(v3Center.x, v3Center.y));
                v3Origin = m_vTranslate + v3Diff;
                v3Origin.z += fzOffset;
            }
            else
            {
                v3Origin = v3Center + v3Diff + m_vTranslate;
            }

            pSound->SetPosition(v3Origin);
        }

        m_vTranslate.Clear();
        m_vTrueTranslate.Clear();
    }
    catch (CException &ex)
    {
        ex.Process(_T("CSoundTool::ApplyTransform() - "), NO_THROW);
    }
}


/*====================
  CSoundTool::UpdateHoverSelection
  ====================*/
void    CSoundTool::UpdateHoverSelection()
{
    m_setHoverSelection.clear();

    if (m_iState == STATE_SELECT)
    {
        CRectf rect(MIN(m_vStartCursorPos.x, Input.GetCursorPos().x),
            MIN(m_vStartCursorPos.y, Input.GetCursorPos().y),
            MAX(m_vStartCursorPos.x, Input.GetCursorPos().x),
            MAX(m_vStartCursorPos.y, Input.GetCursorPos().y));

        WorldSoundsMap& mapSounds(Editor.GetWorld().GetSoundsMap());
        for (WorldSoundsMap_it it(mapSounds.begin()); it != mapSounds.end(); ++it)
        {
            CWorldSound &sound(*it->second);
            if (Editor.GetCamera().IsPointInScreenRect(sound.GetPosition(), rect))
                m_setHoverSelection.insert(sound.GetIndex());
        }
    }
}


/*====================
  CSoundTool::StartSound
  ====================*/
void    CSoundTool::StartSound(uint uiIndex)
{
    StopSound(uiIndex);

    CWorldSound *pSound(Editor.GetWorld().GetSound(uiIndex));

    if (!FileManager.Exists(pSound->GetSound()))
    {
        if (pSound->GetSound().find(_T("%")) == tstring::npos)
            return;

        if (!FileManager.Exists(pSound->GetSound().substr(0, pSound->GetSound().find(_T("%"), 0)) + XtoA(1) + pSound->GetSound().substr(pSound->GetSound().find(_T("%"))+1)))
            return;
    }

    m_mapSounds[uiIndex].hSound = INVALID_INDEX;
    m_mapSounds[uiIndex].uiNextStartTime = 0;
}


/*====================
  CSoundTool::StopSound
  ====================*/
void    CSoundTool::StopSound(uint uiIndex)
{
    SoundsMap_it    itFind(m_mapSounds.find(uiIndex));
    if (itFind == m_mapSounds.end())
        return;

    K2SoundManager.StopHandle(itFind->second.hSound);
    m_mapSounds.erase(itFind);
}


/*====================
  CSoundTool::SetVolume
  ====================*/
void    CSoundTool::SetVolume(uint uiIndex, float fVolume)
{
    SoundsMap_it    itFind(m_mapSounds.find(uiIndex));
    if (itFind == m_mapSounds.end())
        return;

    if (itFind->second.hSound == INVALID_INDEX)
        return;

    K2SoundManager.SetVolume(itFind->second.hSound, fVolume);
}


/*====================
  CSoundTool::UpdateSounds
  ====================*/
void    CSoundTool::UpdateSounds()
{
    for(SoundsMap_it it = m_mapSounds.begin(); it != m_mapSounds.end(); it++)
    {
        if (it->second.hSound == INVALID_INDEX && it->second.uiNextStartTime > Host.GetTime())
            continue;

        CWorldSound *pSound(Editor.GetWorld().GetSound(it->first));

        ResHandle hSound(g_ResourceManager.Register(new CSample(pSound->GetSound(), 0), RES_SAMPLE));
        if (hSound == INVALID_INDEX)
            continue;
        
        if (it->second.hSound == INVALID_INDEX)
            it->second.hSound = K2SoundManager.PlayWorldSFXSound(hSound, &pSound->GetPosition(), pSound->GetVolume(), pSound->GetFalloff());

        if (!K2SoundManager.UpdateHandle(it->second.hSound, pSound->GetPosition(), V3_ZERO))
        {
            it->second.hSound = INVALID_INDEX;
            it->second.uiNextStartTime = Host.GetTime() + pSound->GetLoopDelay();
        }
    }
}


/*====================
  CSoundTool::Enter
  ====================*/
void    CSoundTool::Enter()
{
    PlayWorldAmbientSound();
    WorldSoundsMap& mapSounds(Editor.GetWorld().GetSoundsMap());
    for (WorldSoundsMap_it it(mapSounds.begin()); it != mapSounds.end(); ++it)
        StartSound(it->first);
    SoundSelection.Trigger(XtoA(!m_setSelection.empty()));
}


/*====================
  CSoundTool::Leave
  ====================*/
void    CSoundTool::Leave()
{
    StopWorldAmbientSound();
    for (SoundsMap_it it(m_mapSounds.begin()); it != m_mapSounds.end();)
    {
        K2SoundManager.StopHandle(it->second.hSound);
        STL_ERASE(m_mapSounds, it);
    }
}


/*====================
  CSoundTool::Frame
  ====================*/
void    CSoundTool::Frame(float fFrameTime)
{
    CalcToolProperties();

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
    }

    if (le_soundModel.IsModified())
    {
        m_hSoundModel = g_ResourceManager.Register(le_soundModel, RES_MODEL);
        le_soundModel.SetModified(false);
    }

    m_vOldCursorPos = Input.GetCursorPos();

    UpdateHoverSelection();

    UpdateSounds();
}


/*====================
  CSoundTool::GetCurrentSoundPosition

  Get the current sound position accounting for any active translations
  ====================*/
CVec3f      CSoundTool::GetCurrentSoundPosition(uint uiSound)
{
    try
    {
        CWorldSound *pSound(Editor.GetWorld().GetSound(uiSound));

        if (!IsSelected(uiSound))
            return pSound->GetPosition();

        CVec3f v3Center;
        switch(le_soundCenterMode)
        {
        case CENTER_HOVER:
            if (m_uiHoverIndex != INVALID_INDEX)
                v3Center = GetSoundPosition(m_uiHoverIndex);
            else
                v3Center = GetSoundPosition(uiSound);
            break;

        case CENTER_AVERAGE:
            v3Center = SelectionCenter();
            break;

        case CENTER_INDIVIDUAL:
            if (m_uiHoverIndex != INVALID_INDEX && m_iState == STATE_TRANSLATE_XY)
                v3Center = GetSoundPosition(m_uiHoverIndex);
            else
                v3Center = GetSoundPosition(uiSound);
            break;
        }

        CVec3f v3Diff(pSound->GetPosition() - v3Center);
        CVec3f v3Origin;
        if (m_iState == STATE_TRANSLATE_XY)
        {
            float fzOffset(v3Center.z - Editor.GetWorld().GetTerrainHeight(v3Center.x, v3Center.y));
            v3Origin = m_vTranslate + v3Diff; // use m_VTranslate as the absolute position
            v3Origin.z += fzOffset;
        }
        else
        {
            v3Origin = v3Center + v3Diff + m_vTranslate;
        }

        return v3Origin;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CSoundTool::GetCurrentSoundPosition() - "), NO_THROW);
        return V_ZERO;
    }
}


/*====================
  CSoundTool::GetVolume

  Get the current sound volume
  ====================*/
float       CSoundTool::GetVolume(uint uiSound)
{
    try
    {
        CWorldSound *pSound(Editor.GetWorld().GetSound(uiSound));
        return pSound->GetVolume();
    }
    catch (CException &ex)
    {
        ex.Process(_T("CSoundTool::GetVolume() - "), NO_THROW);
        return 0.0f;
    }
}


/*====================
  CSoundTool::GetFalloff

  Get the current sound falloff
  ====================*/
float       CSoundTool::GetFalloff(uint uiSound)
{
    try
    {
        CWorldSound *pSound(Editor.GetWorld().GetSound(uiSound));
        return pSound->GetFalloff();
    }
    catch (CException &ex)
    {
        ex.Process(_T("CSoundTool::GetFalloff() - "), NO_THROW);
        return 0.0f;
    }
}


/*====================
  CSoundTool::GetLoopDelayMin

  Get the current sound loop delay
  ====================*/
int CSoundTool::GetLoopDelayMin(uint uiSound)
{
    try
    {
        CWorldSound *pSound(Editor.GetWorld().GetSound(uiSound));
        return pSound->GetLoopDelay().Min();
    }
    catch (CException &ex)
    {
        ex.Process(_T("CSoundTool::GetLoopDelayMin() - "), NO_THROW);
        return 0;
    }
}


/*====================
  CSoundTool::GetLoopDelayMax

  Get the current sound loop delay
  ====================*/
int CSoundTool::GetLoopDelayMax(uint uiSound)
{
    try
    {
        CWorldSound *pSound(Editor.GetWorld().GetSound(uiSound));
        return pSound->GetLoopDelay().Max();
    }
    catch (CException &ex)
    {
        ex.Process(_T("CSoundTool::GetLoopDelayMax() - "), NO_THROW);
        return 0;
    }
}

/*====================
  CSoundTool::GetSelectionRect
  ====================*/
CRectf  CSoundTool::GetSelectionRect()
{
    return CRectf(MIN(m_vStartCursorPos.x, Input.GetCursorPos().x),
                MIN(m_vStartCursorPos.y, Input.GetCursorPos().y),
                MAX(m_vStartCursorPos.x, Input.GetCursorPos().x),
                MAX(m_vStartCursorPos.y, Input.GetCursorPos().y));
}


/*====================
  CSoundTool::IsSelected
  ====================*/
int     CSoundTool::IsSelected(uint uiIndex)
{
    return m_setSelection.find(uiIndex) != m_setSelection.end();
}


/*====================
  CSoundTool::IsHoverSelected
  ====================*/
int     CSoundTool::IsHoverSelected(uint uiIndex)
{
    return m_setHoverSelection.find(uiIndex) != m_setHoverSelection.end();
}


/*====================
  CSoundTool::SelectNextSound
  ====================*/
void    CSoundTool::SelectNextSound()
{
    WorldSoundsMap& mapSounds(Editor.GetWorld().GetSoundsMap());

    if (mapSounds.empty())
        return;

    int iSelect(0);

    if (m_setSelection.empty())
    {
        iSelect = mapSounds.begin()->first;
    }
    else
    {
        int iCurrentSelection(*m_setSelection.begin());

        WorldSoundsMap_it it = mapSounds.find(iCurrentSelection);
        if (it == mapSounds.end())
            iSelect = mapSounds.begin()->first;
        else if ((++it) == mapSounds.end())
            iSelect = iCurrentSelection;
        else
            iSelect = it->first;
    }

    m_setSelection.clear();
    m_setSelection.insert(iSelect);
    SoundSelection.Trigger(XtoA(!m_setSelection.empty()));

    Editor.CenterCamera(mapSounds.find(iSelect)->second->GetPosition());
}


/*====================
  CSoundTool::SelectPreviousSound
  ====================*/
void    CSoundTool::SelectPreviousSound()
{
    WorldSoundsMap& mapSounds(Editor.GetWorld().GetSoundsMap());

    if (mapSounds.empty())
        return;

    int iSelect(0);

    if (m_setSelection.empty())
    {
        iSelect = mapSounds.rbegin()->first;
    }
    else
    {
        int iCurrentSelection(*m_setSelection.begin());

        WorldSoundsMap_it it = mapSounds.find(iCurrentSelection);
        if (it == mapSounds.end())
            iSelect = mapSounds.rbegin()->first;
        else if (it == mapSounds.begin())
            iSelect = iCurrentSelection;
        else
            iSelect = (--it)->first;
    }

    m_setSelection.clear();
    m_setSelection.insert(iSelect);
    SoundSelection.Trigger(XtoA(!m_setSelection.empty()));

    Editor.CenterCamera(mapSounds.find(iSelect)->second->GetPosition());
}


/*====================
  CSoundTool::Draw
  ====================*/
void    CSoundTool::Draw()
{
    if (IsSelectionActive())
    {
        CVec4f  v4Border(1.0f, 1.0f, 1.0f, 1.0f);
        CVec4f  v4Fill(0.3f, 0.7f, 1.0f, 0.2f);

        CRectf rec(GetSelectionRect());

        Draw2D.SetColor(v4Fill);
        Draw2D.Rect(rec.left, rec.top, rec.GetWidth(), rec.GetHeight());

        Draw2D.RectOutline(rec, 1, v4Border);
    }
}


/*====================
  CSoundTool::DrawSound
  ====================*/
void    CSoundTool::DrawSound(uint uiIndex)
{
    try
    {
        CWorldSound *pSound(Editor.GetWorld().GetSound(uiIndex));

        CVec3f  vPosition = IsSelected(uiIndex) ? GetCurrentSoundPosition(uiIndex) : pSound->GetPosition();

        //
        // Draw Point
        //

        SSceneFaceVert poly;

        MemManager.Set(&poly, 0, sizeof(poly));

        CVec3_cast(poly.vtx) = vPosition;

        CSceneEntity sc;
        sc.SetPosition(vPosition);
        sc.hRes = m_hSoundModel;
        sc.objtype = OBJTYPE_MODEL;
        sc.flags = 0;
        sc.color = BLACK;

        if (IsSelected(uiIndex) && (IsHoverSelected(uiIndex) || m_uiHoverIndex == uiIndex))
        {
            sc.flags = SCENEENT_SOLID_COLOR;
            sc.color = CVec4f(2.0f, 0.0f, 2.0f, 1.0f);
            SET_VEC4(poly.col, 255, 0, 255, 255);
        }
        else if (IsHoverSelected(uiIndex) || m_uiHoverIndex == uiIndex)
        {
            sc.flags = SCENEENT_SOLID_COLOR;
            sc.color = CVec4f(0.5f, 0.5f, 0.5f, 1.0f);
            SET_VEC4(poly.col, 128, 128, 128, 255);
        }
        else if (IsSelected(uiIndex))
        {
            sc.flags = SCENEENT_SOLID_COLOR;
            sc.color = CVec4f(2.0f, 2.0f, 0.0f, 1.0f);
            SET_VEC4(poly.col, 255, 255, 0, 255);
        }
        else
        {
            SET_VEC4(poly.col, 0, 0, 255, 255);
        }

        SceneManager.AddPoly(1, &poly, m_hLineMaterial, POLY_POINTLIST | POLY_NO_DEPTH_TEST);

        if (m_hSoundModel == INVALID_INDEX)
            return;

        CModel *pModel(g_ResourceManager.GetModel(sc.hRes));

        if (pModel && pModel->GetModelFile() && pModel->GetModelFile()->GetType() == MODEL_K2)
        {
            CK2Model *pK2Model(static_cast<CK2Model *>(pModel->GetModelFile()));

            if (pK2Model->GetNumAnims() > 0)
            {
                if (!m_pSoundModelSkeleton)
                    m_pSoundModelSkeleton = new CSkeleton();

                m_pSoundModelSkeleton->SetModel(sc.hRes);
                if (m_pSoundModelSkeleton->GetCurrentAnimName(0) != _T("idle"))
                    m_pSoundModelSkeleton->StartAnim(_T("idle"), Host.GetTime(), 0);

                m_pSoundModelSkeleton->Pose(Host.GetTime());

                sc.skeleton = m_pSoundModelSkeleton;
            }
        }

        SceneManager.AddEntity(sc);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CSoundTool::DrawSound() - "), NO_THROW);
    }
}


/*====================
  CSoundTool::Render
  ====================*/
void    CSoundTool::Render()
{
    WorldSoundsMap& mapSounds(Editor.GetWorld().GetSoundsMap());
    for (WorldSoundsMap_it it(mapSounds.begin()); it != mapSounds.end(); ++it)
        DrawSound(it->first);
}


/*====================
  CSoundTool::SelectionCenter
  ====================*/
CVec3f  CSoundTool::SelectionCenter()
{
    CVec3f  vCenter(V_ZERO);

    for (uiset::iterator it = m_setSelection.begin(); it != m_setSelection.end(); ++it)
        vCenter += GetSoundPosition(*it);

    vCenter /= float(m_setSelection.size());

    return vCenter;
}


/*====================
  CSoundTool::IsSelectionActive
  ====================*/
bool    CSoundTool::IsSelectionActive()
{
    return m_iState == STATE_SELECT;
}


/*====================
  CSoundTool::SnapCursor
  ====================*/
void    CSoundTool::SnapCursor(const CVec3f &vOrigin)
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
  CSoundTool::PlayWorldAmbientSound
  ====================*/
void    CSoundTool::PlayWorldAmbientSound(void)
{
    ResHandle hSample(g_ResourceManager.Register(new CSample(cg_worldAmbientSound, SND_2D), RES_SAMPLE));

    if (hSample == INVALID_INDEX)
        return;

    if (m_hWorldAmbientSound != INVALID_INDEX)
        StopWorldAmbientSound();

    m_hWorldAmbientSound = K2SoundManager.Play2DSFXSound(hSample, cg_worldAmbientSoundVolume, -1, 128, true);
}



/*====================
  CSoundTool::StopWorldAmbientSound
  ====================*/
void    CSoundTool::StopWorldAmbientSound(void)
{
    if (m_hWorldAmbientSound == INVALID_INDEX)
        return;

    K2SoundManager.StopHandle(m_hWorldAmbientSound);
    m_hWorldAmbientSound = INVALID_INDEX;
}


/*====================
  CSoundTool::SetWorldAmbientSoundVolume
  ====================*/
void    CSoundTool::SetWorldAmbientSoundVolume(float fVolume)
{
    if (m_hWorldAmbientSound == INVALID_INDEX)
        return;

    K2SoundManager.SetVolume(m_hWorldAmbientSound, fVolume);
}


/*====================
  CSoundTool::CloneSelection
  ====================*/
void    CSoundTool::CloneSelection()
{
    try
    {
        uiset setNew;

        for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
        {
            uint uiOldSound(*it);
            CWorldSound *pOldSound(Editor.GetWorld().GetSound(uiOldSound));

            uint uiNewSound(CreateSound());
            if (uiNewSound == INVALID_INDEX)
                EX_ERROR(_T("Failed to create new sound"));

            CWorldSound *pNewSound(Editor.GetWorld().GetSound(uiNewSound));
            *pNewSound = *pOldSound;

            setNew.insert(uiNewSound);
            if (uiOldSound == m_uiHoverIndex)
                m_uiHoverIndex = uiNewSound;

            StartSound(uiNewSound);
        }

        m_setSelection = setNew;
        SoundSelection.Trigger(XtoA(!m_setSelection.empty()));
    }
    catch (CException &ex)
    {
        ex.Process(_T("CSoundTool::CloneSelection() - "), NO_THROW);
    }
}


/*====================
  CSoundTool::SetSelectionSound
  ====================*/
void    CSoundTool::SetSelectionSound(const tstring &sSound)
{
    try
    {
        for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
        {
            CWorldSound *pSound(Editor.GetWorld().GetSound(*it));
            pSound->SetSound(sSound);
            StartSound(*it);
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CSoundTool::SetSelectionSound() - "), NO_THROW);
    }
}


/*====================
  CSoundTool::SetSelectionVolume
  ====================*/
void    CSoundTool::SetSelectionVolume(float fVolume)
{
    try
    {
        for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
        {
            CWorldSound *pSound(Editor.GetWorld().GetSound(*it));
            pSound->SetVolume(fVolume);
            SetVolume(*it, fVolume);
        }
        SoundSelectionVolume.Trigger(XtoA(!m_setSelection.empty()));
    }
    catch (CException &ex)
    {
        ex.Process(_T("CSoundTool::SetVolume() - "), NO_THROW);
    }
}


/*====================
  CSoundTool::SetSelectionFalloff
  ====================*/
void    CSoundTool::SetSelectionFalloff(float fFalloff)
{
    try
    {
        for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
        {
            CWorldSound *pSound(Editor.GetWorld().GetSound(*it));
            pSound->SetFalloff(fFalloff);
            StartSound(*it);
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CSoundTool::SetSelectionFalloff() - "), NO_THROW);
    }
}

/*====================
  CSoundTool::SetSelectionLoopDelayMin
  ====================*/
void    CSoundTool::SetSelectionLoopDelayMin(int iMin)
{
    try
    {
        for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
        {
            CWorldSound *pSound(Editor.GetWorld().GetSound(*it));
            int iMax = pSound->GetLoopDelay().Max();
            pSound->SetLoopDelay(iMin, iMin > iMax ? iMin : iMax);

        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CSoundTool::SetSelectionLoopDelayMin() - "), NO_THROW);
    }
}


/*====================
  CSoundTool::SetSelectionLoopDelayMax
  ====================*/
void    CSoundTool::SetSelectionLoopDelayMax(int iMax)
{
    try
    {
        for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
        {
            CWorldSound *pSound(Editor.GetWorld().GetSound(*it));
            int iMin(pSound->GetLoopDelay().Min());
            pSound->SetLoopDelay(iMax < iMin ? iMax : iMin, iMax);
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CSoundTool::SetSelectionLoopDelayMax() - "), NO_THROW);
    }
}


//=============================================================================
// Console Functions
//=============================================================================

/*--------------------
  GetSelectionSound
  --------------------*/
UI_CMD(GetSelectionSound, 0)
{
    tstring sSound(_T(""));

    try
    {
        const uiset &setSelection(GET_TOOL(Sound, SOUND)->GetSelectionSet());
        for (uiset::const_iterator it(setSelection.begin()); it != setSelection.end(); ++it)
        {
            CWorldSound *pSound(Editor.GetWorld().GetSound(*it));
            sSound = pSound->GetSound();
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("fnGetSelectionSound() - "), NO_THROW);
    }

    return sSound;
}


/*--------------------
  GetSelectionVolume
  --------------------*/
UI_CMD(GetSelectionVolume, 0)
{
    float fVolume(0.0f);

    try
    {
        const uiset &setSelection(GET_TOOL(Sound, SOUND)->GetSelectionSet());
        for (uiset::const_iterator it(setSelection.begin()); it != setSelection.end(); ++it)
        {
            CWorldSound *pSound(Editor.GetWorld().GetSound(*it));
            fVolume += pSound->GetVolume();
        }
        fVolume /= float(setSelection.size());
    }
    catch (CException &ex)
    {
        ex.Process(_T("fnGetSelectionVolume() - "), NO_THROW);
    }

    return XtoA(fVolume);
}


/*--------------------
  GetSelectionFalloff
  --------------------*/
UI_CMD(GetSelectionFalloff, 0)
{
    float fFalloff(0.0f);

    try
    {
        const uiset &setSelection(GET_TOOL(Sound, SOUND)->GetSelectionSet());
        for (uiset::const_iterator it(setSelection.begin()); it != setSelection.end(); ++it)
        {
            CWorldSound *pSound(Editor.GetWorld().GetSound(*it));
            fFalloff += pSound->GetFalloff();
        }
        fFalloff /= float(setSelection.size());
    }
    catch (CException &ex)
    {
        ex.Process(_T("fnGetSelectionFalloff() - "), NO_THROW);
    }

    return XtoA(fFalloff);
}

/*--------------------
  GetSelectionLoopDelayMin
  --------------------*/
UI_CMD(GetSelectionLoopDelayMin, 0)
{
    int iLoopDelay(0);

    try
    {
        const uiset &setSelection(GET_TOOL(Sound, SOUND)->GetSelectionSet());
        for (uiset::const_iterator it(setSelection.begin()); it != setSelection.end(); ++it)
        {
            CWorldSound *pSound(Editor.GetWorld().GetSound(*it));
            iLoopDelay += pSound->GetLoopDelay().Min();
        }
        if (setSelection.size() > 0)
            iLoopDelay /= int(setSelection.size());
    }
    catch (CException &ex)
    {
        ex.Process(_T("fnGetSelectionLoopDelayMin() - "), NO_THROW);
    }

    return XtoA(iLoopDelay);
}


/*--------------------
  GetSelectionLoopDelayMax
  --------------------*/
UI_CMD(GetSelectionLoopDelayMax, 0)
{
    int iLoopDelay(0);

    try
    {
        const uiset &setSelection(GET_TOOL(Sound, SOUND)->GetSelectionSet());
        for (uiset::const_iterator it(setSelection.begin()); it != setSelection.end(); ++it)
        {
            CWorldSound *pSound(Editor.GetWorld().GetSound(*it));
            iLoopDelay += pSound->GetLoopDelay().Max();
        }
        if (setSelection.size() > 0)
            iLoopDelay /= int(setSelection.size());
    }
    catch (CException &ex)
    {
        ex.Process(_T("fnGetSelectionLoopDelayMax() - "), NO_THROW);
    }

    return XtoA(iLoopDelay);
}



/*--------------------
  GetSelectionDetalis
  --------------------*/
UI_CMD(GetSelectionDetails, 0)
{
    tstring sDetails(_T("No sounds selected"));

    try
    {
        const uiset &setSelection(GET_TOOL(Sound, SOUND)->GetSelectionSet());
        if (setSelection.size() == 1)
            sDetails = _T("Selected sound: ") + XtoA(*setSelection.begin());
        else if (setSelection.size() > 1)
        {
            sDetails = _T("Selected sounds: ") + XtoA(*setSelection.begin());
            for (uiset::const_iterator it((++setSelection.begin())); it != setSelection.end(); ++it)
            {
                sDetails += _T(", ") + XtoA(*it);
            }
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("fnGetSelectionDetails() - "), NO_THROW);
    }

    return sDetails;
}


/*--------------------
  SetSelectionSound
  --------------------*/
UI_VOID_CMD(SetSelectionSound, 1)
{
    try
    {
        if (vArgList.size() < 1)
            EX_MESSAGE(_T("syntax: SetSelectionSound <filename>"));

        GET_TOOL(Sound, SOUND)->SetSelectionSound(vArgList[0]->Evaluate());
    }
    catch (CException &ex)
    {
        ex.Process(_T("cmdSetSelectionSound() - "), NO_THROW);
    }
}



/*--------------------
  SetSelectionVolume
  --------------------*/
UI_VOID_CMD(SetSelectionVolume, 1)
{
    try
    {
        if (vArgList.size() < 1)
            EX_MESSAGE(_T("syntax: SetSelectionVolume <volume>"));

        GET_TOOL(Sound, SOUND)->SetSelectionVolume(AtoF(vArgList[0]->Evaluate()));
    }
    catch (CException &ex)
    {
        ex.Process(_T("cmdSetSelectionVolume() - "), NO_THROW);
    }
}


/*--------------------
  SetSelectionFalloff
  --------------------*/
UI_VOID_CMD(SetSelectionFalloff, 1)
{
    try
    {
        if (vArgList.size() < 1)
            EX_MESSAGE(_T("syntax: SetSelectionFalloff <falloff>"));

        GET_TOOL(Sound, SOUND)->SetSelectionFalloff(AtoF(vArgList[0]->Evaluate()));
    }
    catch (CException &ex)
    {
        ex.Process(_T("cmdSetSelectionFalloff() - "), NO_THROW);
    }
}


/*--------------------
  SetSelectionLoopDelayMin
  --------------------*/
UI_VOID_CMD(SetSelectionLoopDelayMin, 1)
{
    try
    {
        if (vArgList.size() < 1)
            EX_MESSAGE(_T("syntax: SetSelectionLoopDelayMin <min>"));

        GET_TOOL(Sound, SOUND)->SetSelectionLoopDelayMin(AtoI(vArgList[0]->Evaluate()));
    }
    catch (CException &ex)
    {
        ex.Process(_T("cmdSetSelectionLoopDelayMin() - "), NO_THROW);
    }
}


/*--------------------
  SetSelectionLoopDelayMax
  --------------------*/
UI_VOID_CMD(SetSelectionLoopDelayMax, 1)
{
    try
    {
        if (vArgList.size() < 1)
            EX_MESSAGE(_T("syntax: SetSelectionLoopDelayMax <max>"));

        GET_TOOL(Sound, SOUND)->SetSelectionLoopDelayMax(AtoI(vArgList[0]->Evaluate()));
    }
    catch (CException &ex)
    {
        ex.Process(_T("cmdSetSelectionLoopDelayMax() - "), NO_THROW);
    }
}


/*--------------------
  cmdSelectNextSound
  --------------------*/
UI_VOID_CMD(SelectNextSound, 0)
{
    GET_TOOL(Sound, SOUND)->SelectNextSound();
}


/*--------------------
  cmdSelectPreviousSound
  --------------------*/
UI_VOID_CMD(SelectPreviousSound, 0)
{
    GET_TOOL(Sound, SOUND)->SelectPreviousSound();
}


/*--------------------
  cmdSoundEditMode
  --------------------*/
UI_VOID_CMD(SoundEditMode, 1)
{
    if (vArgList.size() < 1)
    {
        Console << _T("syntax: SoundEditMode create|select|translate|scale") << newl;
        return;
    }

    tstring sValue(vArgList[0]->Evaluate());

    if (sValue == _T("create"))
    {
        le_soundEditMode = SOUND_CREATE;
        SoundEditMode.Trigger(_T("Create"));
        return;
    }
    else if (sValue == _T("select"))
    {
        le_soundEditMode = SOUND_SELECT;
        SoundEditMode.Trigger(_T("Select"));
        return;
    }
    else if (sValue == _T("translate") || sValue == _T("translate_xy"))
    {
        le_soundEditMode = SOUND_TRANSLATE_XY;
        SoundEditMode.Trigger(_T("Translate XY"));
        return;
    }
    else if (sValue == _T("translate_z"))
    {
        le_soundEditMode = SOUND_TRANSLATE_Z;
        SoundEditMode.Trigger(_T("Translate Z"));
        return;
    }
    /*else if (sValue == _T("rotate") || sValue == _T("rotate_yaw"))
    {
        le_soundEditMode = SOUND_ROTATE_YAW;
        return;
    }
    else if (sValue == _T("rotate_pitch"))
    {
        le_soundEditMode = SOUND_ROTATE_PITCH;
        return;
    }
    else if (sValue == _T("rotate_roll"))
    {
        le_soundEditMode = SOUND_ROTATE_ROLL;
        return;
    }
    else if (sValue == _T("scale"))
    {
        le_soundEditMode = SOUND_SCALE;
        SoundEditMode.Trigger(_T("Scale"));
        return;
    }*/
}


/*--------------------
  cmdSoundCenterMode
  --------------------*/
UI_VOID_CMD(SoundCenterMode, 1)
{
    if (vArgList.size() < 1)
    {
        Console << _T("syntax: SoundCenterMode average|hover|individual") << newl;
        return;
    }

    tstring sValue(vArgList[0]->Evaluate());

    if (sValue == _T("average"))
    {
        le_soundCenterMode = CENTER_AVERAGE;
        return;
    }
    else if (sValue == _T("hover"))
    {
        le_soundCenterMode = CENTER_HOVER;
        return;
    }
    else if (sValue == _T("individual"))
    {
        le_soundCenterMode = CENTER_INDIVIDUAL;
        return;
    }
}


/*--------------------
  cmdStartWorldAmbientSound
  --------------------*/
UI_VOID_CMD(StartWorldAmbientSound, 0)
{
    GET_TOOL(Sound, SOUND)->PlayWorldAmbientSound();
}


/*--------------------
  cmdSetWorldAmbientSoundVolume
  --------------------*/
UI_VOID_CMD(SetWorldAmbientSoundVolume, 1)
{
    if (vArgList.size() < 1)
    {
        Console << _T("syntax: SetWorldAmbientSoundVolume <volume>") << newl;
        return;
    }

    GET_TOOL(Sound, SOUND)->SetWorldAmbientSoundVolume(AtoF(vArgList[0]->Evaluate()));
}
