// (C)2005 S2 Games
// c_lighttool.cpp
//
// Light Tool
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "editor_common.h"

#include "editor.h"

#include "c_lighttool.h"

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
#include "../k2/c_worldlight.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
CVAR_INT    (le_lightEditMode, LIGHT_CREATE);
CVAR_INT    (le_lightCenterMode, CENTER_AVERAGE);
CVAR_BOOL   (le_lightSnap, false);
CVAR_FLOAT  (le_lightVertexDistance, 22.0f);
CVAR_BOOL   (le_lightHoverDelete, true);
CVAR_FLOAT  (le_lightRaiseSensitivity, 1.5f);
CVAR_FLOAT  (le_lightRotateSensitivity, 1.25f);
CVAR_FLOAT  (le_lightScaleSensitivity, 0.05f);
CVAR_FLOAT  (le_lightHeightSnap, 10.0f);
CVAR_FLOAT  (le_lightAngleSnap, 45.0f);
CVAR_FLOAT  (le_lightScaleSnap, 0.5f);
CVAR_INT    (le_lightVertexSelectSize, 16);
CVAR_BOOL   (le_lightDrawRings, true);

UI_TRIGGER  (LightEditMode);
UI_TRIGGER  (LightSelection);
UI_TRIGGER  (LightSelectionFalloff);
//=============================================================================

/*====================
  CLightTool::CLightTool()
  ====================*/
CLightTool::CLightTool() :
ITool(TOOL_LIGHT, _T("light")),
m_bCloning(false),
m_vTranslate(0.0f, 0.0f, 0.0f),
m_fScale(1.0f),
m_vRotation(0.0f, 0.0f, 0.0f),
m_hLineMaterial(g_ResourceManager.Register(_T("/core/materials/line.material"), RES_MATERIAL)),
m_bValidPosition(false),
m_uiHoverIndex(INVALID_INDEX)
{
    for (int n = 0; n < NUM_CIRCLE_SEGMENTS + 1; ++n)
    {
        m_fCosTable[n] = cos((2.0f * M_PI) * (float(n)/NUM_CIRCLE_SEGMENTS));
        m_fSinTable[n] = sin((2.0f * M_PI) * (float(n)/NUM_CIRCLE_SEGMENTS));
    }

    LightEditMode.Trigger(_T("Create"));
}


/*====================
  CLightTool::PrimaryUp

  Left mouse button up action
  ====================*/
void    CLightTool::PrimaryUp()
{
    if (m_bCloning && m_vStartCursorPos == Input.GetCursorPos())
    {
        m_bCloning = false;
        Delete();

        // Cancel current action
        m_vTranslate = m_vTrueTranslate = V_ZERO;
        m_vRotation = m_vTrueRotation = V_ZERO;
        m_fScale = m_fTrueScale = 1.0f;
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
    case STATE_SCALE:
        ApplyTransform();
        break;
    }

    m_bCloning = false;
    m_iState = STATE_HOVERING;
    m_vStartCursorPos.Clear();
}


/*====================
  CLightTool::PrimaryDown

  Default left mouse button down action
  ====================*/
void    CLightTool::PrimaryDown()
{
    CalcToolProperties();

    switch(le_lightEditMode)
    {
    case LIGHT_CREATE:
        Create();
        break;

    case LIGHT_SELECT:
        StartSelect();
        break;

    case LIGHT_TRANSLATE_XY:
        StartTranslateXY();
        break;

    case LIGHT_TRANSLATE_Z:
        StartTransform(STATE_TRANSLATE_Z);
        break;

    case LIGHT_SCALE:
        StartTransform(STATE_SCALE);
        break;
    }
}


/*====================
  CLightTool::SecondaryDown

  Default right mouse button down action
  ====================*/
void    CLightTool::SecondaryDown()
{
    if (m_bCloning)
    {
        m_bCloning = false;
        Delete();
        m_setSelection = m_setOldSelection;
    }

    // Cancel current action
    m_vTranslate = m_vTrueTranslate = V_ZERO;
    m_vRotation = m_vTrueRotation = V_ZERO;
    m_fScale = m_fTrueScale = 1.0f;
    m_iState = STATE_HOVERING;

    // TODO: show context menu
}


/*====================
  CLightTool::Cancel
  ====================*/
void    CLightTool::Cancel()
{
    m_setSelection.clear();
    LightSelection.Trigger(XtoA(!m_setSelection.empty()));
}


/*====================
  CLightTool::GetLightPosition
  ====================*/
CVec3f  CLightTool::GetLightPosition(uint uiIndex)
{
    try
    {
        CWorldLight *pLight(Editor.GetWorld().GetLight(uiIndex));

        if (pLight == nullptr)
            EX_ERROR(_T("Invalid light index: ") + XtoA(uiIndex));

        return pLight->GetPosition();
    }
    catch (CException &ex)
    {
        ex.Process(_T("CLightTool::GetLightPosition() - "), NO_THROW);
        return V_ZERO;
    }
}


/*====================
  CLightTool::Delete
  ====================*/
void    CLightTool::Delete()
{
    if (!m_setSelection.empty())
    {
        while (!m_setSelection.empty())
        {
            uint uiIndex(*m_setSelection.begin());
            Editor.GetWorld().DeleteLight(uiIndex);
            m_setSelection.erase(uiIndex);
        }

        m_uiHoverIndex = INVALID_INDEX;

        LightSelection.Trigger(XtoA(false));
    }
    else if (m_uiHoverIndex != INVALID_INDEX && le_lightHoverDelete)
    {
        m_setSelection.erase(m_uiHoverIndex);
        Editor.GetWorld().DeleteLight(m_uiHoverIndex);
        m_uiHoverIndex = INVALID_INDEX;

        LightSelection.Trigger(XtoA(!m_setSelection.empty()));
    }
}


/*====================
  CLightTool::CursorLightTrace

  Traces against all light in the world
  ====================*/
bool     CLightTool::CursorLightTrace(CLightTraceResult &result)
{
    result.Clear();
    CVec2f v2CursorPos(Input.GetCursorPos());

    WorldLightsMap& mapLights(Editor.GetWorld().GetLightsMap());
    for (WorldLightsMap_it it(mapLights.begin()); it != mapLights.end(); ++it)
    {
        CWorldLight &light(*it->second);

        CVec2f v2ScreenPos;
        if (Editor.GetCamera().WorldToScreen(light.GetPosition(), v2ScreenPos))
        {
            if (ABS(v2CursorPos.x - v2ScreenPos.x) < le_lightVertexSelectSize &&
                ABS(v2CursorPos.y - v2ScreenPos.y) < le_lightVertexSelectSize &&
                Distance(v2CursorPos, v2ScreenPos) < result.fFraction)
            {
                result.uiIndex = light.GetIndex();
                result.fFraction = Distance(v2CursorPos, v2ScreenPos);
                result.v3EndPos = light.GetPosition();
                result.pLight = &light;
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
  CLightTool::CalcToolProperties
  ====================*/
void     CLightTool::CalcToolProperties()
{
    CBrush *pBrush = CBrush::GetCurrentBrush();

    if (m_iState == STATE_HOVERING)
    {
        if (CursorLightTrace(m_Trace) && pBrush)
        {
            // Clip against the brush data
            CRecti  recBrush;
            pBrush->ClipBrush(recBrush);

            float fBrushCenterX((recBrush.left + recBrush.right) / 2.0f);
            float fBrushCenterY((recBrush.top + recBrush.bottom) / 2.0f);

            float fTestX((m_Trace.v3EndPos.x - fBrushCenterX * Editor.GetWorld().GetScale()) / (Editor.GetWorld().GetScale()));
            float fTestY((m_Trace.v3EndPos.y - fBrushCenterY * Editor.GetWorld().GetScale()) / (Editor.GetWorld().GetScale()));

            m_iX = fTestX < 0.0f ? INT_FLOOR(fTestX) : INT_CEIL(fTestX);
            m_iY = fTestY < 0.0f ? INT_FLOOR(fTestY) : INT_CEIL(fTestY);

            m_bValidPosition = true;

            m_v3EndPos = m_Trace.v3EndPos;
            m_uiHoverIndex = m_Trace.uiIndex;
        }
        else
        {
            m_iX = 0;
            m_iY = 0;
            m_v3EndPos.Clear();
            m_uiHoverIndex = INVALID_INDEX;
            m_Trace.Clear();
            m_bValidPosition = false;
        }
    }
    else
    {
        CBrush *pBrush = CBrush::GetCurrentBrush();

        STraceInfo trace;
        if (Editor.TraceCursor(trace, TRACE_TERRAIN) && pBrush)
        {
            // Clip against the brush data
            CRecti  recBrush;
            pBrush->ClipBrush(recBrush);

            float fBrushCenterX((recBrush.left + recBrush.right) / 2.0f);
            float fBrushCenterY((recBrush.top + recBrush.bottom) / 2.0f);

            float fTestX((trace.v3EndPos.x - fBrushCenterX * Editor.GetWorld().GetScale()) / (Editor.GetWorld().GetScale()));
            float fTestY((trace.v3EndPos.y - fBrushCenterY * Editor.GetWorld().GetScale()) / (Editor.GetWorld().GetScale()));

            m_iX = fTestX < 0.0f ? INT_FLOOR(fTestX) : INT_CEIL(fTestX);
            m_iY = fTestY < 0.0f ? INT_FLOOR(fTestY) : INT_CEIL(fTestY);

            m_bValidPosition = true;

            m_v3EndPos = trace.v3EndPos;
            m_Trace.Clear();
        }
        else
        {
            m_iX = 0;
            m_iY = 0;
            m_v3EndPos.Clear();
            m_uiHoverIndex = INVALID_INDEX;
            m_Trace.Clear();
            m_bValidPosition = false;
        }
    }




    //if (m_iState == STATE_HOVERING)
    //{
    //  if (CursorLightTrace(m_Trace))
    //  {
    //      m_iX = Editor.GetWorld().GetVertFromCoord(m_Trace.v3EndPos.x);
    //      m_iY = Editor.GetWorld().GetVertFromCoord(m_Trace.v3EndPos.y);

    //      m_v3EndPos = m_Trace.v3EndPos;
    //      m_uiHoverIndex = m_Trace.uiIndex;
    //      m_bValidPosition = true;
    //  }
    //  else
    //  {
    //      m_iX = 0;
    //      m_iY = 0;
    //      m_v3EndPos.Clear();
    //      m_uiHoverIndex = INVALID_INDEX;
    //      m_Trace.Clear();
    //      m_bValidPosition = false;
    //  }
    //}
    //else
    //{
    //  STraceInfo trace;
    //  if (Editor.TraceCursor(trace, TRACE_TERRAIN))
    //  {
    //      m_iX = Editor.GetWorld().GetVertFromCoord(trace.v3EndPos.x); //[0]);
    //      m_iY = Editor.GetWorld().GetVertFromCoord(trace.v3EndPos.y);//[1]);

    //      m_v3EndPos = trace.v3EndPos;
    //      m_Trace.Clear();
    //      m_bValidPosition = true;
    //  }
    //  else
    //  {
    //      m_iX = 0;
    //      m_iY = 0;

    //      m_v3EndPos.Clear();
    //      m_uiHoverIndex = INVALID_INDEX;
    //      m_Trace.Clear();
    //      m_bValidPosition = false;
    //  }
    //}
}


/*====================
  CLightTool::CreateLight
  ====================*/
uint    CLightTool::CreateLight(const CVec3f &v3Pos)
{
    try
    {
        uint uiNewLight(Editor.GetWorld().AllocateNewLight());
        CWorldLight *pNewLight(Editor.GetWorld().GetLight(uiNewLight, true));

        pNewLight->SetPosition(v3Pos);
        pNewLight->SetColor(CVec3f(1.0f, 1.0f, 1.0f));
        pNewLight->SetFalloffStart(0.0f);
        pNewLight->SetFalloffEnd(1000.0f);
        return uiNewLight;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CLightTool::CreateLight() - "), NO_THROW);
        return INVALID_INDEX;
    }
}


/*====================
  CLightTool::Hovering
  ====================*/
void    CLightTool::Hovering()
{
}


/*====================
  CLightTool::TranslateXY
  ====================*/
void    CLightTool::TranslateXY()
{
    if (Input.GetCursorPos() != m_vStartCursorPos)
    {
        if (m_bSnapCursor) // to delay the first update until the mouse moves
        {
            CVec3f v3Snap;

            if (le_lightCenterMode == CENTER_HOVER || le_lightCenterMode == CENTER_INDIVIDUAL && m_uiHoverIndex != INVALID_INDEX)
                v3Snap = GetLightPosition(m_uiHoverIndex);
            else
                v3Snap = SelectionCenter();

            v3Snap.z = Editor.GetWorld().GetTerrainHeight(v3Snap.x, v3Snap.y);
            SnapCursor(v3Snap);
            m_bSnapCursor = false;
            return;
        }

        m_vTrueTranslate = m_v3EndPos;

        if (le_lightSnap)
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
  CLightTool::TranslateZ
  ====================*/
void    CLightTool::TranslateZ()
{
    CVec2f pos = Input.GetCursorPos();

    float dY = pos.y - m_vOldCursorPos.y;

    if (dY != 0.0f)
    {
        m_vTrueTranslate.z -= dY * le_lightRaiseSensitivity;

        if (le_lightSnap)
            m_vTranslate.z = m_vTrueTranslate.z - fmod(m_vTrueTranslate.z, le_lightHeightSnap);
        else
            m_vTranslate.z = m_vTrueTranslate.z;
    }
}


/*====================
  CLightTool::Scale
  ====================*/
void    CLightTool::Scale()
{
    CVec2f pos = Input.GetCursorPos();

    float dY = pos.y - m_vOldCursorPos.y;

    if (dY != 0.0f)
    {
        m_fTrueScale -= dY * le_lightScaleSensitivity;
        m_fTrueScale = CLAMP(m_fTrueScale, 0.01f, 1000.0f);

        if (le_lightSnap)
            m_fScale = m_fTrueScale - fmod(m_fTrueScale, le_lightScaleSnap);
        else
            m_fScale = m_fTrueScale;
    }
}


/*====================
  CLightTool::Create
  ====================*/
void    CLightTool::Create()
{
    if (m_uiHoverIndex == INVALID_INDEX)
    {
        if (!m_bValidPosition)
            return;

        if (!m_bModifier2)
        {
            m_setSelection.clear();
            LightSelection.Trigger(XtoA(!m_setSelection.empty()));
        }

        CVec3f  vPos = m_v3EndPos;

        if (le_lightSnap)
        {
            vPos.x -= fmod(vPos.x, Editor.GetWorld().GetScale());
            vPos.y -= fmod(vPos.y, Editor.GetWorld().GetScale());
            vPos.z -= fmod(vPos.z, Editor.GetWorld().GetScale());
        }

        vPos.z += 150.0f;

        uint uiLight = CreateLight(vPos);
        if (uiLight != INVALID_INDEX && !m_bModifier1 && !m_bModifier3)
        {
            m_setSelection.insert(uiLight);
            LightSelection.Trigger(XtoA(!m_setSelection.empty()));
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

        LightSelection.Trigger(XtoA(!m_setSelection.empty()));

        m_iState = STATE_HOVERING;
    }
}


/*====================
  CLightTool::StartSelect
  ====================*/
void    CLightTool::StartSelect()
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

            LightSelection.Trigger(XtoA(!m_setSelection.empty()));
        }
        else if (m_bModifier3)
        {
            m_setSelection.erase(m_uiHoverIndex);
            LightSelection.Trigger(XtoA(!m_setSelection.empty()));
        }
        else
        {
            if (m_setSelection.find(m_uiHoverIndex) == m_setSelection.end())
            {
                m_setSelection.clear();
                m_setSelection.insert(m_uiHoverIndex);
                LightSelection.Trigger(XtoA(!m_setSelection.empty()));
            }
        }

        m_iState = STATE_HOVERING;
        m_vStartCursorPos = Input.GetCursorPos();
    }
}


/*====================
  CLightTool::StartTranslateXY
  ====================*/
void    CLightTool::StartTranslateXY()
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

            LightSelection.Trigger(XtoA(!m_setSelection.empty()));
        }
        else if (m_bModifier3)
        {
            m_setSelection.erase(m_uiHoverIndex);
            LightSelection.Trigger(XtoA(!m_setSelection.empty()));
        }
        else
        {
            if (m_setSelection.find(m_uiHoverIndex) == m_setSelection.end())
            {
                m_setSelection.clear();
                m_setSelection.insert(m_uiHoverIndex);
                LightSelection.Trigger(XtoA(!m_setSelection.empty()));
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

            if (le_lightCenterMode == CENTER_HOVER || le_lightCenterMode == CENTER_INDIVIDUAL)
                m_vTranslate = GetLightPosition(m_uiHoverIndex);
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
  CLightTool::StartTransform
  ====================*/
void    CLightTool::StartTransform(int iState)
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

            LightSelection.Trigger(XtoA(!m_setSelection.empty()));
        }
        else if (m_bModifier3)
        {
            m_setSelection.erase(m_uiHoverIndex);
            LightSelection.Trigger(XtoA(!m_setSelection.empty()));
        }
        else
        {
            if (m_setSelection.find(m_uiHoverIndex) == m_setSelection.end())
            {
                m_setSelection.clear();
                m_setSelection.insert(m_uiHoverIndex);
                LightSelection.Trigger(XtoA(!m_setSelection.empty()));
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
        m_vRotation = m_vTrueRotation = V_ZERO;
        m_fScale = m_fTrueScale = 1.0f;
        m_vStartCursorPos = Input.GetCursorPos();
    }
}


/*====================
  CLightTool::ApplySelect
  ====================*/
void    CLightTool::ApplySelect()
{
    if (!(m_bModifier2 || m_bModifier3))
        m_setSelection.clear();

    CRectf rect(MIN(m_vStartCursorPos.x, Input.GetCursorPos().x),
        MIN(m_vStartCursorPos.y, Input.GetCursorPos().y),
        MAX(m_vStartCursorPos.x, Input.GetCursorPos().x),
        MAX(m_vStartCursorPos.y, Input.GetCursorPos().y));


    WorldLightsMap& mapLights(Editor.GetWorld().GetLightsMap());
    for (WorldLightsMap_it it(mapLights.begin()); it != mapLights.end(); ++it)
    {
        CWorldLight &light(*it->second);
        if (Editor.GetCamera().IsPointInScreenRect(light.GetPosition(), rect))
        {
            if (m_bModifier2)
                m_setSelection.insert(light.GetIndex());
            else if (m_bModifier3)
                m_setSelection.erase(light.GetIndex());
            else
                m_setSelection.insert(light.GetIndex());
        }
    }

    LightSelection.Trigger(XtoA(!m_setSelection.empty()));
}


/*====================
  CLightTool::ApplyTransform
  ====================*/
void    CLightTool::ApplyTransform()
{
    try
    {
        CVec3f vSelectionCenter(SelectionCenter());

        for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
        {
            CVec3f v3Center;

            switch(le_lightCenterMode)
            {
            case CENTER_HOVER:
                if (m_uiHoverIndex != INVALID_INDEX)
                    v3Center = GetLightPosition(m_uiHoverIndex);
                else
                    v3Center = GetLightPosition(*it);
                break;

            case CENTER_AVERAGE:
                v3Center = vSelectionCenter;
                break;

            case CENTER_INDIVIDUAL:
                if (m_uiHoverIndex != INVALID_INDEX && m_iState == STATE_TRANSLATE_XY)
                    v3Center = GetLightPosition(m_uiHoverIndex);
                else
                    v3Center = GetLightPosition(*it);
                break;
            }

            CWorldLight *pLight(Editor.GetWorld().GetLight(*it));
            CVec3f v3Diff(pLight->GetPosition() - v3Center);
            CVec3f v3Origin;
            if (m_iState == STATE_TRANSLATE_XY) // use m_VTranslate as the absolute position
            {
                float fzOffset(v3Center.z - Editor.GetWorld().GetTerrainHeight(v3Center.x, v3Center.y));
                v3Origin = m_vTranslate + v3Diff * m_fScale;
                v3Origin.z += fzOffset;
            }
            else
            {
                v3Origin = v3Center + v3Diff * m_fScale + m_vTranslate;
            }

            pLight->SetPosition(v3Origin);
            pLight->ScaleFalloff(m_fScale);
        }

        m_vTranslate.Clear();
        m_vTrueTranslate.Clear();
        m_vRotation.Clear();
        m_vTrueRotation.Clear();
        m_fScale = m_fTrueScale = 1.0f;
        LightSelectionFalloff.Trigger(TSNULL);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CLightTool::ApplyTransform() - "), NO_THROW);
    }
}


/*====================
  CLightTool::UpdateHoverSelection
  ====================*/
void    CLightTool::UpdateHoverSelection()
{
    m_setHoverSelection.clear();

    if (m_iState == STATE_SELECT)
    {
        CRectf rect(MIN(m_vStartCursorPos.x, Input.GetCursorPos().x),
            MIN(m_vStartCursorPos.y, Input.GetCursorPos().y),
            MAX(m_vStartCursorPos.x, Input.GetCursorPos().x),
            MAX(m_vStartCursorPos.y, Input.GetCursorPos().y));

        WorldLightsMap& mapLights(Editor.GetWorld().GetLightsMap());
        for (WorldLightsMap_it it(mapLights.begin()); it != mapLights.end(); ++it)
        {
            CWorldLight &light(*it->second);
            if (Editor.GetCamera().IsPointInScreenRect(light.GetPosition(), rect))
                m_setHoverSelection.insert(light.GetIndex());
        }
    }
}


/*====================
  CLightTool::Frame
  ====================*/
void    CLightTool::Frame(float fFrameTime)
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

    case STATE_SCALE:
        Scale();
        break;
    }

    m_vOldCursorPos = Input.GetCursorPos();

    UpdateHoverSelection();
}


/*====================
  CLightTool::GetCurrentLightPosition

  Get the current light position accounting for any active transformations
  ====================*/
CVec3f      CLightTool::GetCurrentLightPosition(uint uiLight)
{
    try
    {
        CWorldLight *pLight(Editor.GetWorld().GetLight(uiLight));

        if (!IsSelected(uiLight))
            return pLight->GetPosition();

        CVec3f v3Center;
        switch(le_lightCenterMode)
        {
        case CENTER_HOVER:
            if (m_uiHoverIndex != INVALID_INDEX)
                v3Center = GetLightPosition(m_uiHoverIndex);
            else
                v3Center = GetLightPosition(uiLight);
            break;

        case CENTER_AVERAGE:
            v3Center = SelectionCenter();
            break;

        case CENTER_INDIVIDUAL:
            if (m_uiHoverIndex != INVALID_INDEX && m_iState == STATE_TRANSLATE_XY)
                v3Center = GetLightPosition(m_uiHoverIndex);
            else
                v3Center = GetLightPosition(uiLight);
            break;
        }

        CAxis axis(m_vRotation);
        CVec3f v3Diff(TransformPoint(pLight->GetPosition() - v3Center, axis));
        CVec3f v3Origin;
        if (m_iState == STATE_TRANSLATE_XY)
        {
            float fzOffset(v3Center.z - Editor.GetWorld().GetTerrainHeight(v3Center.x, v3Center.y));
            v3Origin = m_vTranslate + v3Diff * m_fScale; // use m_VTranslate as the absolute position
            v3Origin.z += fzOffset;
        }
        else
        {
            v3Origin = v3Center + v3Diff * m_fScale + m_vTranslate;
        }

        return v3Origin;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CLightTool::GetCurrentLightPosition() - "), NO_THROW);
        return V_ZERO;
    }
}


/*====================
  CLightTool::GetStartFalloff

  Get the current light falloff accounting for any active transformations
  ====================*/
float       CLightTool::GetStartFalloff(uint uiLight)
{
    try
    {
        CWorldLight *pLight(Editor.GetWorld().GetLight(uiLight));
        return pLight->GetFalloffStart() * m_fScale;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CLightTool::GetStartFalloff() - "), NO_THROW);
        return 0.0f;
    }
}


/*====================
  CLightTool::GetEndFalloff

  Get the current light position accounting for any active transformations
  ====================*/
float       CLightTool::GetEndFalloff(uint uiLight)
{
    try
    {
        CWorldLight *pLight(Editor.GetWorld().GetLight(uiLight));
        return pLight->GetFalloffEnd() * m_fScale;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CLightTool::GetEndFalloff() - "), NO_THROW);
        return 0.0f;
    }
}


/*====================
  CLightTool::GetSelectionRect
  ====================*/
CRectf  CLightTool::GetSelectionRect()
{
    return CRectf(MIN(m_vStartCursorPos.x, Input.GetCursorPos().x),
                MIN(m_vStartCursorPos.y, Input.GetCursorPos().y),
                MAX(m_vStartCursorPos.x, Input.GetCursorPos().x),
                MAX(m_vStartCursorPos.y, Input.GetCursorPos().y));
}


/*====================
  CLightTool::IsSelected
  ====================*/
int     CLightTool::IsSelected(uint uiIndex)
{
    return m_setSelection.find(uiIndex) != m_setSelection.end();
}


/*====================
  CLightTool::IsHoverSelected
  ====================*/
int     CLightTool::IsHoverSelected(uint uiIndex)
{
    return m_setHoverSelection.find(uiIndex) != m_setHoverSelection.end();
}


/*====================
  CLightTool::Draw
  ====================*/
void    CLightTool::Draw()
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
  CLightTool::DrawLightRings
  ====================*/
void    CLightTool::DrawLightRings(uint uiIndex)
{
    try
    {
        CWorldLight *pLight(Editor.GetWorld().GetLight(uiIndex));

        CVec3f  vPosition = IsSelected(uiIndex) ? GetCurrentLightPosition(uiIndex) : pLight->GetPosition();
        float   fStartFalloff = IsSelected(uiIndex) ? GetStartFalloff(uiIndex) : pLight->GetFalloffStart();
        float   fEndFalloff = IsSelected(uiIndex) ? GetEndFalloff(uiIndex) : pLight->GetFalloffEnd();

        //
        // Draw Point
        //

        SSceneFaceVert poly;

        MemManager.Set(&poly, 0, sizeof(poly));

        CVec3_cast(poly.vtx) = vPosition;

        if (IsSelected(uiIndex) && (IsHoverSelected(uiIndex) || m_uiHoverIndex == uiIndex))
            SET_VEC4(poly.col, 255, 0, 255, 255);
        else if (IsHoverSelected(uiIndex) || m_uiHoverIndex == uiIndex)
            SET_VEC4(poly.col, 128, 128, 128, 255);
        else if (IsSelected(uiIndex))
            SET_VEC4(poly.col, 255, 255, 0, 255);
        else
            SET_VEC4(poly.col, 0, 0, 255, 255);

        SceneManager.AddPoly(1, &poly, m_hLineMaterial, POLY_POINTLIST | POLY_NO_DEPTH_TEST);

        //
        // Draw Rings
        //

        if (le_lightDrawRings)
        {
            SSceneFaceVert circle[NUM_CIRCLE_SEGMENTS + 1];
            int p;

            if (fEndFalloff > 0.0f)
            {
                //
                // End Falloff - XY Depth Ring
                //

                p = 0;

                for (int n = 0; n < NUM_CIRCLE_SEGMENTS + 1; ++n)
                {
                    SET_VEC3(circle[p].vtx,
                        vPosition.x + m_fCosTable[n] * fEndFalloff,
                        vPosition.y + m_fSinTable[n] * fEndFalloff,
                        vPosition.z);

                    if (IsSelected(uiIndex) && (IsHoverSelected(uiIndex) || m_uiHoverIndex == uiIndex))
                        SET_VEC4(circle[p].col, 255, 0, 255, 64);
                    else if (IsHoverSelected(uiIndex) || m_uiHoverIndex == uiIndex)
                        SET_VEC4(circle[p].col, 128, 128, 128, 64);
                    else if (IsSelected(uiIndex))
                        SET_VEC4(circle[p].col, 255, 255, 0, 64);
                    else
                        SET_VEC4(circle[p].col, 0, 0, 255, 64);

                    ++p;
                }

                SceneManager.AddPoly(p, circle, m_hLineMaterial, POLY_LINESTRIP | POLY_NO_DEPTH_TEST);

                //
                // End Falloff - XY Ring
                //

                p = 0;

                for (int n = 0; n < NUM_CIRCLE_SEGMENTS + 1; ++n)
                {
                    if (IsSelected(uiIndex) && (IsHoverSelected(uiIndex) || m_uiHoverIndex == uiIndex))
                        SET_VEC4(circle[p].col, 255, 0, 255, 255);
                    else if (IsHoverSelected(uiIndex) || m_uiHoverIndex == uiIndex)
                        SET_VEC4(circle[p].col, 128, 128, 128, 255);
                    else if (IsSelected(uiIndex))
                        SET_VEC4(circle[p].col, 255, 255, 0, 255);
                    else
                        SET_VEC4(circle[p].col, 0, 0, 255, 255);

                    ++p;
                }

                SceneManager.AddPoly(p, circle, m_hLineMaterial, POLY_LINESTRIP);

                //
                // End Falloff - XZ Depth Ring
                //

                p = 0;

                for (int n = 0; n < NUM_CIRCLE_SEGMENTS + 1; ++n)
                {
                    SET_VEC3(circle[p].vtx,
                        vPosition.x + m_fCosTable[n] * fEndFalloff,
                        vPosition.y,
                        vPosition.z + m_fSinTable[n] * fEndFalloff);

                    if (IsSelected(uiIndex) && (IsHoverSelected(uiIndex) || m_uiHoverIndex == uiIndex))
                        SET_VEC4(circle[p].col, 255, 0, 255, 64);
                    else if (IsHoverSelected(uiIndex) || m_uiHoverIndex == uiIndex)
                        SET_VEC4(circle[p].col, 128, 128, 128, 64);
                    else if (IsSelected(uiIndex))
                        SET_VEC4(circle[p].col, 255, 255, 0, 64);
                    else
                        SET_VEC4(circle[p].col, 0, 0, 255, 64);

                    ++p;
                }

                SceneManager.AddPoly(p, circle, m_hLineMaterial, POLY_LINESTRIP | POLY_NO_DEPTH_TEST);

                //
                // End Falloff - XZ Ring
                //

                p = 0;

                for (int n = 0; n < NUM_CIRCLE_SEGMENTS + 1; ++n)
                {
                    if (IsSelected(uiIndex) && (IsHoverSelected(uiIndex) || m_uiHoverIndex == uiIndex))
                        SET_VEC4(circle[p].col, 255, 0, 255, 255);
                    else if (IsHoverSelected(uiIndex) || m_uiHoverIndex == uiIndex)
                        SET_VEC4(circle[p].col, 128, 128, 128, 255);
                    else if (IsSelected(uiIndex))
                        SET_VEC4(circle[p].col, 255, 255, 0, 255);
                    else
                        SET_VEC4(circle[p].col, 0, 0, 255, 255);

                    ++p;
                }

                SceneManager.AddPoly(p, circle, m_hLineMaterial, POLY_LINESTRIP);

                //
                // End Falloff - YZ Depth Ring
                //

                p = 0;

                for (int n = 0; n < NUM_CIRCLE_SEGMENTS + 1; ++n)
                {
                    SET_VEC3(circle[p].vtx,
                        vPosition.x,
                        vPosition.y + m_fCosTable[n] * fEndFalloff,
                        vPosition.z + m_fSinTable[n] * fEndFalloff);

                    if (IsSelected(uiIndex) && (IsHoverSelected(uiIndex) || m_uiHoverIndex == uiIndex))
                        SET_VEC4(circle[p].col, 255, 0, 255, 64);
                    else if (IsHoverSelected(uiIndex) || m_uiHoverIndex == uiIndex)
                        SET_VEC4(circle[p].col, 128, 128, 128, 64);
                    else if (IsSelected(uiIndex))
                        SET_VEC4(circle[p].col, 255, 255, 0, 64);
                    else
                        SET_VEC4(circle[p].col, 0, 0, 255, 64);

                    ++p;
                }

                SceneManager.AddPoly(p, circle, m_hLineMaterial, POLY_LINESTRIP | POLY_NO_DEPTH_TEST);

                //
                // End Falloff - YZ Ring
                //

                p = 0;

                for (int n = 0; n < NUM_CIRCLE_SEGMENTS + 1; ++n)
                {
                    if (IsSelected(uiIndex) && (IsHoverSelected(uiIndex) || m_uiHoverIndex == uiIndex))
                        SET_VEC4(circle[p].col, 255, 0, 255, 255);
                    else if (IsHoverSelected(uiIndex) || m_uiHoverIndex == uiIndex)
                        SET_VEC4(circle[p].col, 128, 128, 128, 255);
                    else if (IsSelected(uiIndex))
                        SET_VEC4(circle[p].col, 255, 255, 0, 255);
                    else
                        SET_VEC4(circle[p].col, 0, 0, 255, 255);

                    ++p;
                }

                SceneManager.AddPoly(p, circle, m_hLineMaterial, POLY_LINESTRIP);
            }

            if (fStartFalloff > 0.0f)
            {
                //
                // Start Falloff - XY Depth Ring
                //

                p = 0;

                for (int n = 0; n < NUM_CIRCLE_SEGMENTS + 1; ++n)
                {
                    SET_VEC3(circle[p].vtx,
                        vPosition.x + m_fCosTable[n] * fStartFalloff,
                        vPosition.y + m_fSinTable[n] * fStartFalloff,
                        vPosition.z);

                    if (IsSelected(uiIndex) && (IsHoverSelected(uiIndex) || m_uiHoverIndex == uiIndex))
                        SET_VEC4(circle[p].col, 255, 0, 255, 64);
                    else if (IsHoverSelected(uiIndex) || m_uiHoverIndex == uiIndex)
                        SET_VEC4(circle[p].col, 128, 128, 128, 64);
                    else if (IsSelected(uiIndex))
                        SET_VEC4(circle[p].col, 255, 255, 0, 64);
                    else
                        SET_VEC4(circle[p].col, 0, 0, 255, 64);

                    ++p;
                }

                SceneManager.AddPoly(p, circle, m_hLineMaterial, POLY_LINESTRIP | POLY_NO_DEPTH_TEST);

                //
                // Start Falloff - XY Ring
                //

                p = 0;

                for (int n = 0; n < NUM_CIRCLE_SEGMENTS + 1; ++n)
                {
                    if (IsSelected(uiIndex) && (IsHoverSelected(uiIndex) || m_uiHoverIndex == uiIndex))
                        SET_VEC4(circle[p].col, 255, 0, 255, 255);
                    else if (IsHoverSelected(uiIndex) || m_uiHoverIndex == uiIndex)
                        SET_VEC4(circle[p].col, 128, 128, 128, 255);
                    else if (IsSelected(uiIndex))
                        SET_VEC4(circle[p].col, 255, 255, 0, 255);
                    else
                        SET_VEC4(circle[p].col, 0, 0, 255, 255);

                    ++p;
                }

                SceneManager.AddPoly(p, circle, m_hLineMaterial, POLY_LINESTRIP);

                //
                // Start Falloff - XZ Depth Ring
                //

                p = 0;

                for (int n = 0; n < NUM_CIRCLE_SEGMENTS + 1; ++n)
                {
                    SET_VEC3(circle[p].vtx,
                        vPosition.x + m_fCosTable[n] * fStartFalloff,
                        vPosition.y,
                        vPosition.z + m_fSinTable[n] * fStartFalloff);

                    if (IsSelected(uiIndex) && (IsHoverSelected(uiIndex) || m_uiHoverIndex == uiIndex))
                        SET_VEC4(circle[p].col, 255, 0, 255, 64);
                    else if (IsHoverSelected(uiIndex) || m_uiHoverIndex == uiIndex)
                        SET_VEC4(circle[p].col, 128, 128, 128, 64);
                    else if (IsSelected(uiIndex))
                        SET_VEC4(circle[p].col, 255, 255, 0, 64);
                    else
                        SET_VEC4(circle[p].col, 0, 0, 255, 64);

                    ++p;
                }

                SceneManager.AddPoly(p, circle, m_hLineMaterial, POLY_LINESTRIP | POLY_NO_DEPTH_TEST);

                //
                // Start Falloff - XZ Ring
                //

                p = 0;

                for (int n = 0; n < NUM_CIRCLE_SEGMENTS + 1; ++n)
                {
                    if (IsSelected(uiIndex) && (IsHoverSelected(uiIndex) || m_uiHoverIndex == uiIndex))
                        SET_VEC4(circle[p].col, 255, 0, 255, 255);
                    else if (IsHoverSelected(uiIndex) || m_uiHoverIndex == uiIndex)
                        SET_VEC4(circle[p].col, 128, 128, 128, 255);
                    else if (IsSelected(uiIndex))
                        SET_VEC4(circle[p].col, 255, 255, 0, 255);
                    else
                        SET_VEC4(circle[p].col, 0, 0, 255, 255);

                    ++p;
                }

                SceneManager.AddPoly(p, circle, m_hLineMaterial, POLY_LINESTRIP);

                //
                // Start Falloff - YZ Depth Ring
                //

                p = 0;

                for (int n = 0; n < NUM_CIRCLE_SEGMENTS + 1; ++n)
                {
                    SET_VEC3(circle[p].vtx,
                        vPosition.x,
                        vPosition.y + m_fCosTable[n] * fStartFalloff,
                        vPosition.z + m_fSinTable[n] * fStartFalloff);

                    if (IsSelected(uiIndex) && (IsHoverSelected(uiIndex) || m_uiHoverIndex == uiIndex))
                        SET_VEC4(circle[p].col, 255, 0, 255, 64);
                    else if (IsHoverSelected(uiIndex) || m_uiHoverIndex == uiIndex)
                        SET_VEC4(circle[p].col, 128, 128, 128, 64);
                    else if (IsSelected(uiIndex))
                        SET_VEC4(circle[p].col, 255, 255, 0, 64);
                    else
                        SET_VEC4(circle[p].col, 0, 0, 255, 64);

                    ++p;
                }

                SceneManager.AddPoly(p, circle, m_hLineMaterial, POLY_LINESTRIP | POLY_NO_DEPTH_TEST);

                //
                // Start Falloff - YZ Ring
                //

                p = 0;

                for (int n = 0; n < NUM_CIRCLE_SEGMENTS + 1; ++n)
                {

                    if (IsSelected(uiIndex) && (IsHoverSelected(uiIndex) || m_uiHoverIndex == uiIndex))
                        SET_VEC4(circle[p].col, 255, 0, 255, 255);
                    else if (IsHoverSelected(uiIndex) || m_uiHoverIndex == uiIndex)
                        SET_VEC4(circle[p].col, 128, 128, 128, 255);
                    else if (IsSelected(uiIndex))
                        SET_VEC4(circle[p].col, 255, 255, 0, 255);
                    else
                        SET_VEC4(circle[p].col, 0, 0, 255, 255);

                    ++p;
                }

                SceneManager.AddPoly(p, circle, m_hLineMaterial, POLY_LINESTRIP);
            }
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CLightTool::DrawLightRings() - "), NO_THROW);
    }
}


/*====================
  CLightTool::Render
  ====================*/
void    CLightTool::Render()
{
    WorldLightsMap& mapLights(Editor.GetWorld().GetLightsMap());
    for (WorldLightsMap_it it(mapLights.begin()); it != mapLights.end(); ++it)
        DrawLightRings(it->first);
}


/*====================
  CLightTool::SelectionCenter
  ====================*/
CVec3f  CLightTool::SelectionCenter()
{
    CVec3f  vCenter(V_ZERO);

    for (uiset::iterator it = m_setSelection.begin(); it != m_setSelection.end(); ++it)
        vCenter += GetLightPosition(*it);

    vCenter /= float(m_setSelection.size());

    return vCenter;
}


/*====================
  CLightTool::IsSelectionActive
  ====================*/
bool    CLightTool::IsSelectionActive()
{
    return m_iState == STATE_SELECT;
}


/*====================
  CLightTool::SnapCursor
  ====================*/
void    CLightTool::SnapCursor(const CVec3f &vOrigin)
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
  CLightTool::CloneSelection
  ====================*/
void    CLightTool::CloneSelection()
{
    try
    {
        uiset setNew;

        for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
        {
            uint uiOldLight(*it);
            CWorldLight *pOldLight(Editor.GetWorld().GetLight(uiOldLight));

            uint uiNewLight(CreateLight());
            if (uiNewLight == INVALID_INDEX)
                EX_ERROR(_T("Failed to create new light"));

            CWorldLight *pNewLight(Editor.GetWorld().GetLight(uiNewLight));
            *pNewLight = *pOldLight;

            setNew.insert(uiNewLight);
            if (uiOldLight == m_uiHoverIndex)
                m_uiHoverIndex = uiNewLight;
        }

        m_setSelection = setNew;
        LightSelection.Trigger(XtoA(!m_setSelection.empty()));
    }
    catch (CException &ex)
    {
        ex.Process(_T("CLightTool::CloneSelection() - "), NO_THROW);
    }
}


/*====================
  CLightTool::SetSelectionColor
  ====================*/
void    CLightTool::SetSelectionColor(EColorComponent eColor, float fLevel)
{
    try
    {
        for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
        {
            CWorldLight *pLight(Editor.GetWorld().GetLight(*it));
            pLight->SetColor(eColor, fLevel);
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CLightTool::SetSelectionColor() - "), NO_THROW);
    }
}


/*====================
  CLightTool::SetSelectionStartFalloff
  ====================*/
void    CLightTool::SetSelectionStartFalloff(float fStartFalloff)
{
    try
    {
        for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
        {
            CWorldLight *pLight(Editor.GetWorld().GetLight(*it));
            pLight->SetFalloffStart(fStartFalloff);
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CLightTool::SetSelectionStartFalloff() - "), NO_THROW);
    }
}


/*====================
  CLightTool::SetSelectionEndFalloff
  ====================*/
void    CLightTool::SetSelectionEndFalloff(float fEndFalloff)
{
    try
    {
        for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
        {
            CWorldLight *pLight(Editor.GetWorld().GetLight(*it));
            pLight->SetFalloffEnd(fEndFalloff);
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CLightTool::SetSelectionEndFalloff() - "), NO_THROW);
    }
}


//=============================================================================
// Console Functions
//=============================================================================

/*--------------------
  GetSelectionColorR
  --------------------*/
UI_CMD(GetSelectionColorR, 0)
{
    float fRed(0.0f);

    try
    {
        const uiset &setSelection(GET_TOOL(Light, LIGHT)->GetSelectionSet());
        for (uiset::const_iterator it(setSelection.begin()); it != setSelection.end(); ++it)
        {
            CWorldLight *pLight(Editor.GetWorld().GetLight(*it));
            fRed += pLight->GetColor(R);
        }

        fRed /= float(setSelection.size());
    }
    catch (CException &ex)
    {
        ex.Process(_T("fnGetSelectionColorR() - "), NO_THROW);
    }

    return XtoA(fRed);
}


/*--------------------
  GetSelectionColorG
  --------------------*/
UI_CMD(GetSelectionColorG, 0)
{
    float fGreen(0.0f);

    try
    {
        const uiset &setSelection(GET_TOOL(Light, LIGHT)->GetSelectionSet());
        for (uiset::const_iterator it(setSelection.begin()); it != setSelection.end(); ++it)
        {
            CWorldLight *pLight(Editor.GetWorld().GetLight(*it));
            fGreen += pLight->GetColor(G);
        }

        fGreen /= float(setSelection.size());
        return XtoA(fGreen);
    }
    catch (CException &ex)
    {
        ex.Process(_T("fnGetSelectionColorG() - "), NO_THROW);
    }

    return XtoA(fGreen);
}


/*--------------------
  GetSelectionColorB
  --------------------*/
UI_CMD(GetSelectionColorB, 0)
{
    float fBlue(0.0f);

    try
    {
        const uiset &setSelection(GET_TOOL(Light, LIGHT)->GetSelectionSet());
        for (uiset::const_iterator it(setSelection.begin()); it != setSelection.end(); ++it)
        {
            CWorldLight *pLight(Editor.GetWorld().GetLight(*it));
            fBlue += pLight->GetColor(B);
        }

        fBlue /= float(setSelection.size());
        return XtoA(fBlue);
    }
    catch (CException &ex)
    {
        ex.Process(_T("fnGetSelectionColorG() - "), NO_THROW);
    }

    return XtoA(fBlue);
}


/*--------------------
  GetSelectionStartFalloff
  --------------------*/
UI_CMD(GetSelectionStartFalloff, 0)
{
    float fStartFalloff(0.0f);

    try
    {
        const uiset &setSelection(GET_TOOL(Light, LIGHT)->GetSelectionSet());
        for (uiset::const_iterator it(setSelection.begin()); it != setSelection.end(); ++it)
        {
            CWorldLight *pLight(Editor.GetWorld().GetLight(*it));
            fStartFalloff += pLight->GetFalloffStart();
        }

        fStartFalloff /= float(setSelection.size());
    }
    catch (CException &ex)
    {
        ex.Process(_T("fnGetSelectionStartFalloff() - "), NO_THROW);
    }

    return XtoA(fStartFalloff);
}


/*--------------------
  GetSelectionEndFalloff
  --------------------*/
UI_CMD(GetSelectionEndFalloff, 0)
{
    float fEndFalloff(0.0f);

    try
    {
        const uiset &setSelection(GET_TOOL(Light, LIGHT)->GetSelectionSet());
        for (uiset::const_iterator it(setSelection.begin()); it != setSelection.end(); ++it)
        {
            CWorldLight *pLight(Editor.GetWorld().GetLight(*it));
            fEndFalloff += pLight->GetFalloffEnd();
        }

        fEndFalloff /= float(setSelection.size());
    }
    catch (CException &ex)
    {
        ex.Process(_T("fnGetSelectionEndFalloff() - "), NO_THROW);
    }

    return XtoA(fEndFalloff);
}



/*--------------------
  SetSelectionColorR
  --------------------*/
UI_VOID_CMD(SetSelectionColorR, 1)
{
    try
    {
        if (vArgList.size() < 1)
            EX_MESSAGE(_T("syntax: SetSelectionColorR <red>"));

        GET_TOOL(Light, LIGHT)->SetSelectionColor(R, AtoF(vArgList[0]->Evaluate()));
    }
    catch (CException &ex)
    {
        ex.Process(_T("cmdSetSelectionEndFalloff() - "), NO_THROW);
    }
}


/*--------------------
  SetSelectionColorG
  --------------------*/
UI_VOID_CMD(SetSelectionColorG, 1)
{
    try
    {
        if (vArgList.size() < 1)
            EX_MESSAGE(_T("syntax: SetSelectionColorG <green>"));

        GET_TOOL(Light, LIGHT)->SetSelectionColor(G, AtoF(vArgList[0]->Evaluate()));
    }
    catch (CException &ex)
    {
        ex.Process(_T("cmdSetSelectionEndFalloff() - "), NO_THROW);
    }
}


/*--------------------
  SetSelectionColorB
  --------------------*/
UI_VOID_CMD(SetSelectionColorB, 1)
{
    try
    {
        if (vArgList.size() < 1)
            EX_MESSAGE(_T("syntax: SetSelectionColorB <blue>"));

        GET_TOOL(Light, LIGHT)->SetSelectionColor(B, AtoF(vArgList[0]->Evaluate()));
    }
    catch (CException &ex)
    {
        ex.Process(_T("cmdSetSelectionEndFalloff() - "), NO_THROW);
    }
}


/*--------------------
  SetSelectionStartFalloff
  --------------------*/
UI_VOID_CMD(SetSelectionStartFalloff, 1)
{
    try
    {
        if (vArgList.size() < 1)
            EX_MESSAGE(_T("syntax: SetSelectionStartFalloff <blue>"));

        GET_TOOL(Light, LIGHT)->SetSelectionStartFalloff(AtoF(vArgList[0]->Evaluate()));
    }
    catch (CException &ex)
    {
        ex.Process(_T("cmdSetSelectionEndFalloff() - "), NO_THROW);
    }
}


/*--------------------
  SetSelectionEndFalloff
  --------------------*/
UI_VOID_CMD(SetSelectionEndFalloff, 1)
{
    try
    {
        if (vArgList.size() < 1)
            EX_MESSAGE(_T("syntax: SetSelectionEndFalloff <blue>"));

        GET_TOOL(Light, LIGHT)->SetSelectionEndFalloff(AtoF(vArgList[0]->Evaluate()));
    }
    catch (CException &ex)
    {
        ex.Process(_T("cmdSetSelectionEndFalloff() - "), NO_THROW);
    }
}



/*--------------------
  cmdLightEditMode
  --------------------*/
UI_VOID_CMD(LightEditMode, 1)
{
    if (vArgList.size() < 1)
    {
        Console << _T("syntax: entityeditmode create|select|translate|rotate|scale") << newl;
        return;
    }

    tstring sValue(vArgList[0]->Evaluate());

    if (sValue == _T("create"))
    {
        le_lightEditMode = LIGHT_CREATE;
        LightEditMode.Trigger(_T("Create"));
        return;
    }
    else if (sValue == _T("select"))
    {
        le_lightEditMode = LIGHT_SELECT;
        LightEditMode.Trigger(_T("Select"));
        return;
    }
    else if (sValue == _T("translate") || sValue == _T("translate_xy"))
    {
        le_lightEditMode = LIGHT_TRANSLATE_XY;
        LightEditMode.Trigger(_T("Translate XY"));
        return;
    }
    else if (sValue == _T("translate_z"))
    {
        le_lightEditMode = LIGHT_TRANSLATE_Z;
        LightEditMode.Trigger(_T("Translate Z"));
        return;
    }
    /*else if (sValue == _T("rotate") || sValue == _T("rotate_yaw"))
    {
        le_lightEditMode = LIGHT_ROTATE_YAW;
        return;
    }
    else if (sValue == _T("rotate_pitch"))
    {
        le_lightEditMode = LIGHT_ROTATE_PITCH;
        return;
    }
    else if (sValue == _T("rotate_roll"))
    {
        le_lightEditMode = LIGHT_ROTATE_ROLL;
        return;
    }*/
    else if (sValue == _T("scale"))
    {
        le_lightEditMode = LIGHT_SCALE;
        LightEditMode.Trigger(_T("Scale"));
        return;
    }
}


/*--------------------
  cmdLightCenterMode
  --------------------*/
UI_VOID_CMD(LightCenterMode, 1)
{
    if (vArgList.size() < 1)
    {
        Console << _T("syntax: entitycentermode ") << newl;
        return;
    }

    tstring sValue(vArgList[0]->Evaluate());

    if (sValue == _T("average"))
    {
        le_lightCenterMode = CENTER_AVERAGE;
        return;
    }
    else if (sValue == _T("hover"))
    {
        le_lightCenterMode = CENTER_HOVER;
        return;
    }
    else if (sValue == _T("individual"))
    {
        le_lightCenterMode = CENTER_INDIVIDUAL;
        return;
    }
}

