// (C)2005 S2 Games
// c_occludertool.cpp
//
// Occluder Tool
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "editor_common.h"

#include "editor.h"

#include "c_occludertool.h"

#include "../k2/c_brush.h"
#include "../k2/c_world.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_vid.h"
#include "../k2/c_input.h"
#include "../k2/c_draw2d.h"
#include "../k2/intersection.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_fontmap.h"
#include "../k2/c_uitrigger.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
CVAR_INT    (le_occluderEditMode,           OCCLUDER_CREATE);
CVAR_INT    (le_occluderCenterMode,         CENTER_AVERAGE);
CVAR_BOOL   (le_occluderSnap,               false);
CVAR_FLOAT  (le_occluderVertexDistance,     22.0f);
CVAR_BOOL   (le_occluderHoverDelete,        true);
CVAR_FLOAT  (le_occluderRaiseSensitivity,   2.5f);
CVAR_FLOAT  (le_occluderRotateSensitivity,  1.25f);
CVAR_FLOAT  (le_occluderScaleSensitivity,   0.05f);
CVAR_FLOAT  (le_occluderHeightSnap,         10.0f);
CVAR_FLOAT  (le_occluderAngleSnap,          45.0f);
CVAR_FLOAT  (le_occluderScaleSnap,          0.5f);
CVAR_INT    (le_occluderVertexSelectSize,   16);
CVAR_BOOL   (le_occluderFade,               false);
CVAR_FLOAT  (le_occluderAlpha,              0.5f);
CVAR_BOOLF  (le_occluderDrawBrushCoords,    true,   CVAR_SAVECONFIG);

UI_TRIGGER  (OccluderEditMode);

// FIXME: Handle this better
extern ResHandle    g_hOccluderMaterial;
//=============================================================================

/*====================
  COccluderTool::COccluderTool()
  ====================*/
COccluderTool::COccluderTool() :
ITool(TOOL_OCCLUDER, _T("occluder")),
m_bCloning(false),
m_vTranslate(0.0f, 0.0f, 0.0f),
m_fScale(1.0f),
m_vRotation(0.0f, 0.0f, 0.0f),
m_uiHoverIndex(INVALID_INDEX),
m_uiHoverOccluder(INVALID_INDEX),
m_uiWorkingOccluder(INVALID_INDEX),
m_eOccluderSelectMode(SELECT_FACE),
m_hLineMaterial(g_ResourceManager.Register(_T("/core/materials/line.material"), RES_MATERIAL)),
m_hOccluderMaterial(g_ResourceManager.Register(_T("/core/materials/occluder.material"), RES_MATERIAL)),
m_bValidPosition(false),
m_hFont(g_ResourceManager.LookUpName(_T("system_medium"), RES_FONTMAP))
{
    OccluderEditMode.Trigger(_T("Create"));
}


/*====================
  COccluderTool::PrimaryUp

  Left mouse button up action
  ====================*/
void    COccluderTool::PrimaryUp()
{
    if (m_bCloning && m_vStartCursorPos == Input.GetCursorPos())
    {
        m_bCloning = false;
        Delete();

        // Cancel current action
        m_vTranslate = m_vTrueTranslate = CVec3f(0.0f, 0.0f, 0.0f);
        m_vRotation = m_vTrueRotation = CVec3f(0.0f, 0.0f, 0.0f);
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
    case STATE_ROTATE_YAW:
    case STATE_ROTATE_PITCH:
    case STATE_ROTATE_ROLL:
    case STATE_SCALE:
        ApplyTransform();
        break;
    }

    m_bCloning = false;
    m_iState = STATE_HOVERING;
    m_vStartCursorPos.Clear();
}


/*====================
  COccluderTool::PrimaryDown

  Default left mouse button down action
  ====================*/
void    COccluderTool::PrimaryDown()
{
    CalcToolProperties();

    switch(le_occluderEditMode)
    {
    case OCCLUDER_CREATE:
        Create();
        break;

    case OCCLUDER_SELECT:
        StartSelect();
        break;

    case OCCLUDER_TRANSLATE_XY:
        StartTranslateXY();
        break;

    case OCCLUDER_TRANSLATE_Z:
        StartTransform(STATE_TRANSLATE_Z);
        break;

    case OCCLUDER_ROTATE_YAW:
        StartTransform(STATE_ROTATE_YAW);
        break;

    case OCCLUDER_ROTATE_PITCH:
        StartTransform(STATE_ROTATE_PITCH);
        break;

    case OCCLUDER_ROTATE_ROLL:
        StartTransform(STATE_ROTATE_ROLL);
        break;

    case OCCLUDER_SCALE:
        StartTransform(STATE_SCALE);
        break;
    }
}

/*====================
  COccluderTool::SecondaryDown

  Default right mouse button down action
  ====================*/
void    COccluderTool::SecondaryDown()
{
    if (m_bCloning)
    {
        m_bCloning = false;
        Delete();
        m_setSelection = m_setOldSelection;
    }

    // Cancel current action
    m_vTranslate = m_vTrueTranslate = CVec3f(0.0f, 0.0f, 0.0f);
    m_vRotation = m_vTrueRotation = CVec3f(0.0f, 0.0f, 0.0f);
    m_fScale = m_fTrueScale = 1.0f;
    m_iState = STATE_HOVERING;

    // TODO: show context menu
}


/*====================
  COccluderTool::Cancel
  ====================*/
void    COccluderTool::Cancel()
{
    m_setSelection.clear();
    m_uiWorkingOccluder = INVALID_INDEX;
}


/*====================
  COccluderTool::GetOccluder
  ====================*/
COccluder*  COccluderTool::GetOccluder(uint uiIndex)
{
    return Editor.GetWorld().GetOccluder(uiIndex);
}


/*====================
  COccluderTool::DeleteOccluder
  ====================*/
void    COccluderTool::DeleteOccluder(uint uiIndex)
{
    Editor.GetWorld().DeleteOccluder(uiIndex);
}


/*====================
  COccluderTool::DeleteVertex
  ====================*/
void    COccluderTool::DeleteVertex()
{
    try
    {
        COccluder *pOccluder(GetOccluder(m_uiWorkingOccluder));

        if (!m_setSelection.empty())
        {
            while (!m_setSelection.empty())
            {
                uint uiVertex(*m_setSelection.begin());

                RemoveOccluderVertex(uiVertex, *pOccluder);
                m_setSelection.erase(uiVertex);
                UpdateOccluderSelectionSetDeletion(m_setSelection, uiVertex);
            }

            m_uiHoverIndex = INVALID_INDEX;
        }
        else if (m_uiHoverIndex != INVALID_INDEX && le_occluderHoverDelete)
        {
            m_setSelection.erase(m_uiHoverIndex);
            DeleteOccluder(m_uiHoverIndex);
            UpdateOccluderSelectionSetDeletion(m_setSelection, m_uiHoverIndex);
            m_uiHoverIndex = INVALID_INDEX;
        }

        if (pOccluder->GetNumPoints() < 3)
        {
            DeleteOccluder(m_uiWorkingOccluder);
            m_uiWorkingOccluder = INVALID_INDEX;
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("COccluderTool::DeleteVertex() - "), NO_THROW);
    }
}


/*====================
  COccluderTool::Delete
  ====================*/
void    COccluderTool::Delete()
{
    switch (m_eOccluderSelectMode)
    {
    case SELECT_VERTEX:
        if (m_uiWorkingOccluder != INVALID_INDEX)
            DeleteVertex();
        break;

    case SELECT_EDGE:
        break;

    case SELECT_FACE:
        if (!m_setSelection.empty())
        {
            while (!m_setSelection.empty())
            {
                uint uiIndex(*m_setSelection.begin());

                DeleteOccluder(uiIndex);
                m_setSelection.erase(uiIndex);
                UpdateOccluderSelectionSetDeletion(m_setSelection, uiIndex);
            }

            m_uiHoverIndex = INVALID_INDEX;
        }
        else if (m_uiHoverIndex != INVALID_INDEX && le_occluderHoverDelete)
        {
            m_setSelection.erase(m_uiHoverIndex);
            DeleteOccluder(m_uiHoverIndex);
            UpdateOccluderSelectionSetDeletion(m_setSelection, m_uiHoverIndex);
            m_uiHoverIndex = INVALID_INDEX;
        }
        break;
    }
}


/*====================
  COccluderTool::CursorOccluderTraceFace

  Traces against all occluders in the world
  ====================*/
bool     COccluderTool::CursorOccluderTraceFace()
{
    try
    {
        MemManager.Set(&m_Trace, 0, sizeof(SOccluderTrace));
        m_Trace.fFraction = FAR_AWAY;
        m_Trace.uiIndex = INVALID_INDEX;
        m_Trace.uiVertex = INVALID_INDEX;

        CVec3f v3Dir(Editor.GetCamera().ConstructRay(Input.GetCursorPos()));

        const OccluderMap   &mapOccluders(Editor.GetWorld().GetOccluderMap());
        for (OccluderMap::const_iterator it(mapOccluders.begin()); it != mapOccluders.end(); ++it)
        {
            CVec3f v3A(it->second->GetPoint(0));

            for (uint uiPoint(0); uiPoint < it->second->GetNumPoints() - 2; ++uiPoint)
            {
                CVec3f v3B(it->second->GetPoint(uiPoint + 1));
                CVec3f v3C(it->second->GetPoint(uiPoint + 2));

                float fFrac;
                if (I_RayTriIntersect(Editor.GetCamera().GetOrigin(), v3Dir, v3A, v3B, v3C, fFrac))
                {
                    if (fFrac < m_Trace.fFraction)
                    {
                        m_Trace.fFraction = fFrac;
                        m_Trace.uiIndex = it->first;
                    }
                }
            }
        }

        // Set vEndPos, iVertex, and iEdge if we actually hit something
        if (m_Trace.uiIndex != INVALID_INDEX)
        {
            m_Trace.v3EndPos = M_PointOnLine(Editor.GetCamera().GetOrigin(), v3Dir, m_Trace.fFraction);
            m_Trace.pOccluder = GetOccluder(m_Trace.uiIndex);
            m_Trace.v3Offset = m_Trace.v3EndPos - m_Trace.pOccluder->GetPoint(0);
            return true;
        }

        // No hit, do a trace against the terrain
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
    catch (CException &ex)
    {
        ex.Process(_T("COccluderTool::CursorOccluderTraceFace() - "), NO_THROW);
        return false;
    }
}


/*====================
  COccluderTool::CursorOccluderTraceVertex
  ====================*/
bool     COccluderTool::CursorOccluderTraceVertex(uint uiIndex)
{
    try
    {
        if (uiIndex == INVALID_INDEX)
            return CursorOccluderTraceFace();

        // Clear the trace results
        MemManager.Set(&m_Trace, 0, sizeof(SOccluderTrace));
        m_Trace.fFraction = FAR_AWAY;
        m_Trace.uiIndex = INVALID_INDEX;
        m_Trace.uiVertex = INVALID_INDEX;

        CVec2f v2CursorPos(Input.GetCursorPos());

        // Check each vertex of the occluder
        COccluder *pOccluder(GetOccluder(uiIndex));
        for (uint uiPoint(0); uiPoint < pOccluder->GetNumPoints(); ++uiPoint)
        {
            CVec2f v2ScreenPos;
            if (!Editor.GetCamera().WorldToScreen(pOccluder->GetPoint(uiPoint), v2ScreenPos))
                continue;

            if (ABS(v2CursorPos.x - v2ScreenPos.x) < le_occluderVertexSelectSize &&
                ABS(v2CursorPos.y - v2ScreenPos.y) < le_occluderVertexSelectSize &&
                Distance(v2CursorPos, v2ScreenPos) < m_Trace.fFraction)
            {
                m_Trace.uiIndex = uiIndex;
                m_Trace.uiVertex = uiPoint;
                m_Trace.fFraction = Distance(v2CursorPos, v2ScreenPos);
                m_Trace.v3EndPos = pOccluder->GetPoint(uiPoint);
            }
        }

        if (m_Trace.uiIndex == INVALID_INDEX)
            return CursorOccluderTraceFace();

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("COccluderTool::CursorOccluderTraceVertex() - "), NO_THROW);
        return false;
    }
}


/*====================
  COccluderTool::CalcToolProperties
  ====================*/
void     COccluderTool::CalcToolProperties()
{
    try
    {
        if (m_iState == STATE_HOVERING)
        {
            bool bRet;

            switch (m_eOccluderSelectMode)
            {
            case SELECT_VERTEX:
                bRet = CursorOccluderTraceVertex(m_uiWorkingOccluder);
                break;

            case SELECT_EDGE:
                bRet = CursorOccluderTraceFace();
                break;

            default:
            case SELECT_FACE:
                bRet = CursorOccluderTraceFace();
                break;
            }

            if (bRet)
            {
                m_v3EndPos = m_Trace.v3EndPos;

                switch (m_eOccluderSelectMode)
                {
                case SELECT_VERTEX:
                    m_uiHoverIndex = m_Trace.uiVertex;
                    break;

                case SELECT_EDGE:
                    m_uiHoverIndex = m_Trace.uiEdge;
                    break;

                case SELECT_FACE:
                default:
                    m_uiHoverIndex = m_Trace.uiIndex;
                    break;
                }

                m_uiHoverOccluder = m_Trace.uiIndex;
                m_bValidPosition = true;
            }
            else
            {
                m_bValidPosition = false;

                m_v3EndPos.Clear();
                m_uiHoverOccluder = m_uiHoverIndex = INVALID_INDEX;

                MemManager.Set(&m_Trace, 0, sizeof(m_Trace));
                m_Trace.uiIndex = INVALID_INDEX;
                m_Trace.uiVertex = INVALID_INDEX;
            }
        }
        else
        {
            switch (m_eOccluderSelectMode)
            {
            case SELECT_VERTEX:
                if (m_uiWorkingOccluder != INVALID_INDEX)
                {
                    COccluder *pOccluder(GetOccluder(m_uiWorkingOccluder));
                    CPlane plane(pOccluder->GetPlane());

                    CVec3f v3Dir(Editor.GetCamera().ConstructRay(Input.GetCursorPos()));
                    CVec3f v3Point;
                    float fFract(M_RayPlaneIntersect(Editor.GetCamera().GetOrigin(), v3Dir, plane, v3Point));

                    if (plane.IsValid() && fFract > 0.0f)
                    {
                        m_v3EndPos = v3Point;

                        MemManager.Set(&m_Trace, 0, sizeof(m_Trace));
                        m_Trace.uiIndex = INVALID_INDEX;
                        m_Trace.uiVertex = INVALID_INDEX;
                    }
                    else
                    {
                        m_v3EndPos.Clear();
                        m_uiHoverOccluder = m_uiHoverIndex = INVALID_INDEX;

                        MemManager.Set(&m_Trace, 0, sizeof(m_Trace));
                        m_Trace.uiIndex = INVALID_INDEX;
                        m_Trace.uiVertex = INVALID_INDEX;
                    }
                }
                else
                {
                    STraceInfo trace;

                    if (Editor.TraceCursor(trace, TRACE_TERRAIN))
                    {
                        m_v3EndPos = trace.v3EndPos;

                        MemManager.Set(&m_Trace, 0, sizeof(m_Trace));
                        m_Trace.uiIndex = INVALID_INDEX;
                        m_Trace.uiVertex = INVALID_INDEX;
                    }
                    else
                    {
                        m_v3EndPos.Clear();
                        m_uiHoverOccluder = m_uiHoverIndex = INVALID_INDEX;

                        MemManager.Set(&m_Trace, 0, sizeof(m_Trace));
                        m_Trace.uiIndex = INVALID_INDEX;
                        m_Trace.uiVertex = INVALID_INDEX;
                    }
                }
                break;

            case SELECT_EDGE:
                m_uiHoverIndex = m_Trace.uiEdge;
                break;

            case SELECT_FACE:
            default:
                {
                    STraceInfo trace;

                    if (Editor.TraceCursor(trace, TRACE_TERRAIN))
                    {
                        m_v3EndPos = trace.v3EndPos;

                        MemManager.Set(&m_Trace, 0, sizeof(m_Trace));
                        m_Trace.uiIndex = INVALID_INDEX;
                        m_Trace.uiVertex = INVALID_INDEX;
                    }
                    else
                    {
                        m_v3EndPos.Clear();
                        m_uiHoverOccluder = m_uiHoverIndex = INVALID_INDEX;

                        MemManager.Set(&m_Trace, 0, sizeof(m_Trace));
                        m_Trace.uiIndex = INVALID_INDEX;
                        m_Trace.uiVertex = INVALID_INDEX;
                    }
                }
                break;
            }

        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("COccluderTool::CalcToolProperties() - "), NO_THROW);
    }
}


/*====================
  COccluderTool::CreateOccluder
  ====================*/
uint    COccluderTool::CreateOccluder(const CVec3f &v3Pos)
{
    uint uiIndex(Editor.GetWorld().AllocateNewOccluder());

    COccluder *pNewOccluder(GetOccluder(uiIndex));

    pNewOccluder->SetNumPoints(4);
    pNewOccluder->SetPoint(0, M_PointOnLine(v3Pos, Editor.GetCamera().GetViewAxis(RIGHT), -200.0f));
    pNewOccluder->SetPoint(1, M_PointOnLine(pNewOccluder->GetPoint(0), V_UP, 400.0f));
    pNewOccluder->SetPoint(2, M_PointOnLine(pNewOccluder->GetPoint(1), Editor.GetCamera().GetViewAxis(RIGHT), 400.0f));
    pNewOccluder->SetPoint(3, M_PointOnLine(pNewOccluder->GetPoint(2), V_UP, -400.0f));

    return uiIndex;
}


/*====================
  COccluderTool::Hovering
  ====================*/
void    COccluderTool::Hovering()
{
}


/*====================
  COccluderTool::TranslateXY
  ====================*/
void    COccluderTool::TranslateXY()
{
    if (Input.GetCursorPos() != m_vStartCursorPos)
    {
        if (m_bSnapCursor) // to delay the first update until the mouse moves
        {
            switch (m_eOccluderSelectMode)
            {
            case SELECT_VERTEX:
                {
                    CVec3f vSnap;

                    if (le_occluderCenterMode == CENTER_HOVER || le_occluderCenterMode == CENTER_INDIVIDUAL && m_uiHoverIndex != INVALID_INDEX)
                        vSnap = GetOccluderOrigin(m_uiHoverIndex);
                    else
                        vSnap = SelectionCenter();

                    SnapCursor(vSnap);

                    m_bSnapCursor = false;
                }
                break;
            case SELECT_FACE:
                {
                    CVec3f vSnap;

                    if (le_occluderCenterMode == CENTER_HOVER || le_occluderCenterMode == CENTER_INDIVIDUAL && m_uiHoverIndex != INVALID_INDEX)
                        vSnap = GetOccluderOrigin(m_uiHoverIndex);
                    else
                        vSnap = SelectionCenter();

                    vSnap.z = Editor.GetWorld().GetTerrainHeight(vSnap.x, vSnap.y);
                    SnapCursor(vSnap);

                    m_bSnapCursor = false;
                }
                break;
            }
            return;
        }

        m_vTrueTranslate = m_v3EndPos;

        if (le_occluderSnap)
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
  COccluderTool::TranslateZ
  ====================*/
void    COccluderTool::TranslateZ()
{
    CVec2f pos = Input.GetCursorPos();

    float dY = pos.y - m_vOldCursorPos.y;

    if (dY != 0.0f)
    {
        m_vTrueTranslate.z -= dY * le_occluderRaiseSensitivity;

        if (le_occluderSnap)
            m_vTranslate.z = m_vTrueTranslate.z - fmod(m_vTrueTranslate.z, le_occluderHeightSnap);
        else
            m_vTranslate.z = m_vTrueTranslate.z;
    }
}


/*====================
  COccluderTool::RotateYaw
  ====================*/
void    COccluderTool::RotateYaw()
{
    CVec2f pos = Input.GetCursorPos();

    float dX = pos.x - m_vOldCursorPos.x;

    if (dX != 0.0f)
    {
        m_vTrueRotation[YAW] += dX * le_occluderRotateSensitivity;
        m_vTrueRotation[YAW] = fmod(m_vTrueRotation[YAW], 360.0f);

        if (le_occluderSnap)
            m_vRotation[YAW] = m_vTrueRotation[YAW] - fmod(m_vTrueRotation[YAW], le_occluderAngleSnap);
        else
            m_vRotation[YAW] = m_vTrueRotation[YAW];
    }
}


/*====================
  COccluderTool::RotatePitch
  ====================*/
void    COccluderTool::RotatePitch()
{
    CVec2f pos = Input.GetCursorPos();

    float dY = pos.y - m_vOldCursorPos.y;

    if (dY != 0.0f)
    {
        m_vTrueRotation[PITCH] -= dY * le_occluderRotateSensitivity;
        m_vTrueRotation[PITCH] = fmod(m_vTrueRotation[PITCH], 360.0f);

        if (le_occluderSnap)
            m_vRotation[PITCH] = m_vTrueRotation[PITCH] - fmod(m_vTrueRotation[PITCH], le_occluderAngleSnap);
        else
            m_vRotation[PITCH] = m_vTrueRotation[PITCH];
    }
}


/*====================
  COccluderTool::RotateRoll
  ====================*/
void    COccluderTool::RotateRoll()
{
    CVec2f pos = Input.GetCursorPos();

    float dX = pos.x - m_vOldCursorPos.x;

    if (dX != 0.0f)
    {
        m_vTrueRotation[ROLL] += dX * le_occluderRotateSensitivity;
        m_vTrueRotation[ROLL] = fmod(m_vTrueRotation[ROLL], 360.0f);

        if (le_occluderSnap)
            m_vRotation[ROLL] = m_vTrueRotation[ROLL] - fmod(m_vTrueRotation[ROLL], le_occluderAngleSnap);
        else
            m_vRotation[ROLL] = m_vTrueRotation[ROLL];
    }
}


/*====================
  COccluderTool::Scale
  ====================*/
void    COccluderTool::Scale()
{
    CVec2f pos = Input.GetCursorPos();

    float dY = pos.y - m_vOldCursorPos.y;

    if (dY != 0.0f)
    {
        m_fTrueScale -= dY * le_occluderScaleSensitivity;
        m_fTrueScale = CLAMP(m_fTrueScale, 0.01f, 1000.0f);

        if (le_occluderSnap)
            m_fScale = m_fTrueScale - fmod(m_fTrueScale, le_occluderScaleSnap);
        else
            m_fScale = m_fTrueScale;
    }
}


/*====================
  COccluderTool::Create
  ====================*/
void    COccluderTool::Create()
{
    switch (m_eOccluderSelectMode)
    {
    case SELECT_VERTEX:
        if (m_uiHoverOccluder != INVALID_INDEX && m_uiHoverOccluder != m_uiWorkingOccluder)
        {
            m_uiWorkingOccluder = m_uiHoverOccluder;
            m_setSelection.clear();
        }
        else
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
                }
                else if (m_bModifier3)
                {
                    m_setSelection.erase(m_uiHoverIndex);
                }
                else
                {
                    if (m_setSelection.find(m_uiHoverIndex) == m_setSelection.end())
                    {
                        m_setSelection.clear();
                        m_setSelection.insert(m_uiHoverIndex);
                    }
                }

                m_iState = STATE_HOVERING;
                m_vStartCursorPos = Input.GetCursorPos();
            }
        }
        break;
    case SELECT_FACE:
        if (m_uiHoverIndex == INVALID_INDEX)
        {
            if (!m_bValidPosition)
                return;

            m_setSelection.clear();

            CVec3f  vPos(m_v3EndPos);

            if (le_occluderSnap)
            {
                vPos.x -= fmod(vPos.x, Editor.GetWorld().GetScale());
                vPos.y -= fmod(vPos.y, Editor.GetWorld().GetScale());
                vPos.z -= fmod(vPos.z, Editor.GetWorld().GetScale());
            }

            uint uiIndex(CreateOccluder(vPos));

            if (uiIndex != INVALID_INDEX)
            {
                switch (m_eOccluderSelectMode)
                {
                case SELECT_VERTEX:
                    break;

                case SELECT_EDGE:
                    break;

                case SELECT_FACE:
                    m_setSelection.insert(uiIndex);
                    break;
                }
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

            m_iState = STATE_HOVERING;
        }
        break;
    }
}


/*====================
  COccluderTool::Split
  ====================*/
void    COccluderTool::Split()
{
    try
    {
        if (m_eOccluderSelectMode != SELECT_VERTEX)
            EX_WARN(_T("Can only split verts"));

        if (m_uiWorkingOccluder == INVALID_INDEX)
            EX_ERROR(_T("Invalid occluder selection"));

        COccluder *pOccluder(GetOccluder(m_uiWorkingOccluder));
        uiset set(m_setSelection);

        // search for pairs of adjacent vertices that are selected and split them
        for (uint ui(0); ui < pOccluder->GetNumPoints(); ++ui)
        {
            if (m_setSelection.find(ui) != m_setSelection.end() &&
                m_setSelection.find(ui + 1) != m_setSelection.end())
            {
                SplitOccluderVertices(ui, ui + 1, *pOccluder);
                UpdateOccluderSelectionSetAddition(m_setSelection, ui + 1);
                UpdateOccluderSelectionSetAddition(set, ui + 1);
                set.insert(ui + 1);
            }
        }

        m_setSelection = set;
    }
    catch (CException &ex)
    {
        ex.Process(_T("COccluderTool::Split() - "), NO_THROW);
    }
}


/*====================
  COccluderTool::StartSelect
  ====================*/
void    COccluderTool::StartSelect()
{
    switch (m_eOccluderSelectMode)
    {
    case SELECT_VERTEX:
        if (m_uiHoverOccluder != INVALID_INDEX && m_uiHoverOccluder != m_uiWorkingOccluder)
        {
            m_uiWorkingOccluder = m_uiHoverOccluder;
            m_setSelection.clear();
        }
        else
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
                }
                else if (m_bModifier3)
                {
                    m_setSelection.erase(m_uiHoverIndex);
                }
                else
                {
                    if (m_setSelection.find(m_uiHoverIndex) == m_setSelection.end())
                    {
                        m_setSelection.clear();
                        m_setSelection.insert(m_uiHoverIndex);
                    }
                }

                m_iState = STATE_HOVERING;
                m_vStartCursorPos = Input.GetCursorPos();
            }
        }
        break;

    case SELECT_FACE:
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
            }
            else if (m_bModifier3)
            {
                m_setSelection.erase(m_uiHoverIndex);
            }
            else
            {
                if (m_setSelection.find(m_uiHoverIndex) == m_setSelection.end())
                {
                    m_setSelection.clear();
                    m_setSelection.insert(m_uiHoverIndex);
                }
            }

            m_iState = STATE_HOVERING;
            m_vStartCursorPos = Input.GetCursorPos();
        }
        break;
    }
}


/*====================
  COccluderTool::StartTranslateXY
  ====================*/
void    COccluderTool::StartTranslateXY()
{
    switch (m_eOccluderSelectMode)
    {
    case SELECT_VERTEX:
        if (m_uiHoverOccluder != INVALID_INDEX && m_uiHoverOccluder != m_uiWorkingOccluder)
        {
            m_uiWorkingOccluder = m_uiHoverOccluder;
            m_setSelection.clear();
        }
        else
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
                }
                else if (m_bModifier3)
                {
                    m_setSelection.erase(m_uiHoverIndex);
                }
                else
                {
                    if (m_setSelection.find(m_uiHoverIndex) == m_setSelection.end())
                    {
                        m_setSelection.clear();
                        m_setSelection.insert(m_uiHoverIndex);
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

                    if (le_occluderCenterMode == CENTER_HOVER || le_occluderCenterMode == CENTER_INDIVIDUAL)
                    {
                        m_vTranslate = GetOccluderOrigin(m_uiHoverIndex);
                    }
                    else
                        m_vTranslate = SelectionCenter();

                    m_bSnapCursor = true;
                }

                m_vTrueTranslate = m_vTranslate;
                m_vStartCursorPos = Input.GetCursorPos();
            }
        }
        break;
    case SELECT_FACE:
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
            }
            else if (m_bModifier3)
            {
                m_setSelection.erase(m_uiHoverIndex);
            }
            else
            {
                if (m_setSelection.find(m_uiHoverIndex) == m_setSelection.end())
                {
                    m_setSelection.clear();
                    m_setSelection.insert(m_uiHoverIndex);
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

                if (le_occluderCenterMode == CENTER_HOVER || le_occluderCenterMode == CENTER_INDIVIDUAL)
                {
                    m_vTranslate = GetOccluderOrigin(m_uiHoverIndex);
                }
                else
                    m_vTranslate = SelectionCenter();

                m_bSnapCursor = true;
            }

            m_vTranslate.z = Editor.GetWorld().GetTerrainHeight(m_vTranslate.x, m_vTranslate.y);

            m_vTrueTranslate = m_vTranslate;
            m_vStartCursorPos = Input.GetCursorPos();
        }
        break;
    }
}


/*====================
  COccluderTool::StartTransform
  ====================*/
void    COccluderTool::StartTransform(int iState)
{
    switch (m_eOccluderSelectMode)
    {
    case SELECT_VERTEX:
        if (m_uiHoverOccluder != INVALID_INDEX && m_uiHoverOccluder != m_uiWorkingOccluder)
        {
            m_uiWorkingOccluder = m_uiHoverOccluder;
            m_setSelection.clear();
        }
        else
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
                }
                else if (m_bModifier3)
                {
                    m_setSelection.erase(m_uiHoverIndex);
                }
                else
                {
                    if (m_setSelection.find(m_uiHoverIndex) == m_setSelection.end())
                    {
                        m_setSelection.clear();
                        m_setSelection.insert(m_uiHoverIndex);
                    }
                }

                m_iState = STATE_HOVERING;
                m_vStartCursorPos = Input.GetCursorPos();
            }
        }
        break;
    case SELECT_FACE:
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
            }
            else if (m_bModifier3)
            {
                m_setSelection.erase(m_uiHoverIndex);
            }
            else
            {
                if (m_setSelection.find(m_uiHoverIndex) == m_setSelection.end())
                {
                    m_setSelection.clear();
                    m_setSelection.insert(m_uiHoverIndex);
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
        break;
    }
}


/*====================
  COccluderTool::ApplySelect
  ====================*/
void    COccluderTool::ApplySelect()
{
    try
    {
        if (!(m_bModifier2 || m_bModifier3))
            m_setSelection.clear();

        CRectf rect(MIN(m_vStartCursorPos.x, Input.GetCursorPos().x),
            MIN(m_vStartCursorPos.y, Input.GetCursorPos().y),
            MAX(m_vStartCursorPos.x, Input.GetCursorPos().x),
            MAX(m_vStartCursorPos.y, Input.GetCursorPos().y));

        switch (m_eOccluderSelectMode)
        {
        case SELECT_VERTEX:
            if (m_uiWorkingOccluder != INVALID_INDEX)
            {
                COccluder *pOccluder(GetOccluder(m_uiWorkingOccluder));

                for (uint uiPoint(0); uiPoint < pOccluder->GetNumPoints(); ++uiPoint)
                {
                    if (Editor.GetCamera().IsPointInScreenRect(pOccluder->GetPoint(uiPoint), rect))
                    {
                        if (m_bModifier2)
                            m_setSelection.insert(uiPoint);
                        else if (m_bModifier3)
                            m_setSelection.erase(uiPoint);
                        else
                            m_setSelection.insert(uiPoint);
                    }
                }
            }
            break;

        case SELECT_FACE:
            const OccluderMap   &mapOccluders(Editor.GetWorld().GetOccluderMap());
            for (OccluderMap::const_iterator it(mapOccluders.begin()); it != mapOccluders.end(); ++it)
            {
                if (Editor.GetCamera().IsPointInScreenRect(GetOccluderOrigin(it->first), rect))
                {
                    if (m_bModifier2)
                        m_setSelection.insert(it->first);
                    else if (m_bModifier3)
                        m_setSelection.erase(it->first);
                    else
                        m_setSelection.insert(it->first);
                }
            }
            break;
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("COccluderToool::ApplySelect() - "), NO_THROW);
    }
}


/*====================
  COccluderTool::ApplyTransform
  ====================*/
void    COccluderTool::ApplyTransform()
{
    try
    {
        switch(m_eOccluderSelectMode)
        {
        case SELECT_VERTEX:
            if (m_uiWorkingOccluder != INVALID_INDEX)
            {
                COccluder occluder(*GetOccluder(m_uiWorkingOccluder));

                for (uiset::iterator it = m_setSelection.begin(); it != m_setSelection.end(); ++it)
                {
                    CVec3f v3Center;

                    switch(le_occluderCenterMode)
                    {
                    case CENTER_HOVER:
                        if (m_uiHoverIndex != INVALID_INDEX)
                            v3Center = GetOccluderOrigin(m_uiHoverIndex);
                        else
                            v3Center = GetOccluderOrigin(*it);
                        break;

                    case CENTER_AVERAGE:
                        v3Center = SelectionCenter();
                        break;

                    case CENTER_INDIVIDUAL:
                        if (m_uiHoverIndex != INVALID_INDEX && m_iState == STATE_TRANSLATE_XY)
                            v3Center = GetOccluderOrigin(m_uiHoverIndex);
                        else
                            v3Center = GetOccluderOrigin(*it);
                        break;
                    }

                    CAxis axis(m_vRotation[PITCH], m_vRotation[ROLL], m_vRotation[YAW]);
                    CVec3f v3Diff(occluder.GetPoint(*it) - v3Center);
                    v3Diff = TransformPoint(v3Diff, axis);

                    CVec3f v3Origin;
                    if (m_iState == STATE_TRANSLATE_XY) // use m_VTranslate as the absolute position
                        v3Origin = m_vTranslate + v3Diff * m_fScale;
                    else
                        v3Origin = v3Center + v3Diff * m_fScale + m_vTranslate;

                    occluder.SetPoint(*it, v3Origin);
                }

                *GetOccluder(m_uiWorkingOccluder) = occluder;
            }
            break;

        case SELECT_FACE:
            {
                CVec3f  v3SelectionCenter(SelectionCenter());
                CVec3f  v3HoverCenter(m_uiHoverIndex != INVALID_INDEX ? GetOccluderOrigin(m_uiHoverIndex) : V3_ZERO);

                for (uiset::iterator it = m_setSelection.begin(); it != m_setSelection.end(); ++it)
                {
                    CVec3f v3Center;

                    switch(le_occluderCenterMode)
                    {
                    case CENTER_HOVER:
                        if (m_uiHoverIndex != INVALID_INDEX)
                            v3Center = v3HoverCenter;
                        else
                            v3Center = GetOccluderOrigin(*it);
                        break;

                    case CENTER_AVERAGE:
                        v3Center = v3SelectionCenter;
                        break;

                    case CENTER_INDIVIDUAL:
                        if (m_uiHoverIndex != INVALID_INDEX && m_iState == STATE_TRANSLATE_XY)
                            v3Center = v3HoverCenter;
                        else
                            v3Center = GetOccluderOrigin(*it);
                        break;
                    }

                    COccluder *pOccluder(GetOccluder(*it));

                    for (uint uiPoint(0); uiPoint < pOccluder->GetNumPoints(); ++uiPoint)
                    {
                        CAxis axis(m_vRotation);
                        CVec3f v3Diff(pOccluder->GetPoint(uiPoint) - v3Center);
                        v3Diff = TransformPoint(v3Diff, axis);

                        CVec3f v3Origin;
                        if (m_iState == STATE_TRANSLATE_XY) // use m_VTranslate as the absolute position
                        {
                            float fzOffset = v3Center.z - Editor.GetWorld().GetTerrainHeight(v3Center.x, v3Center.y);
                            v3Origin = m_vTranslate + v3Diff * m_fScale;
                            v3Origin.z += fzOffset;
                        }
                        else
                        {
                            v3Origin = v3Center + v3Diff * m_fScale + m_vTranslate;
                        }

                        pOccluder->SetPoint(uiPoint, v3Origin);
                    }
                }
            }
            break;
        }

        m_vTranslate.Clear();
        m_vTrueTranslate.Clear();
        m_vRotation.Clear();
        m_vTrueRotation.Clear();
        m_fScale = m_fTrueScale = 1.0f;
    }
    catch (CException &ex)
    {
        ex.Process(_T("COccluderTool::ApplyTransform() - "), NO_THROW);
    }
}


/*====================
  COccluderTool::UpdateHoverSelection
  ====================*/
void    COccluderTool::UpdateHoverSelection()
{
    try
    {
        m_setHoverSelection.clear();

        if (m_iState == STATE_SELECT)
        {
            CRectf rect(MIN(m_vStartCursorPos.x, Input.GetCursorPos().x),
                MIN(m_vStartCursorPos.y, Input.GetCursorPos().y),
                MAX(m_vStartCursorPos.x, Input.GetCursorPos().x),
                MAX(m_vStartCursorPos.y, Input.GetCursorPos().y));

            switch (m_eOccluderSelectMode)
            {
            case SELECT_VERTEX:
                if (m_uiWorkingOccluder != INVALID_INDEX)
                {
                    COccluder *pOccluder(GetOccluder(m_uiWorkingOccluder));

                    for (uint uiPoint(0); uiPoint < pOccluder->GetNumPoints(); ++uiPoint)
                    {
                        if (Editor.GetCamera().IsPointInScreenRect(pOccluder->GetPoint(uiPoint), rect))
                            m_setHoverSelection.insert(uiPoint);
                    }
                }
                break;

            case SELECT_FACE:
                const OccluderMap   &mapOccluders(Editor.GetWorld().GetOccluderMap());
                for (OccluderMap::const_iterator it(mapOccluders.begin()); it != mapOccluders.end(); ++it)
                {
                    if (Editor.GetCamera().IsPointInScreenRect(GetOccluderOrigin(it->first), rect))
                        m_setHoverSelection.insert(it->first);
                }
                break;
            }
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("COccludetTool::UpdateHoverSelection() - "), NO_THROW);
    }
}


/*====================
  COccluderTool::Frame
  ====================*/
void    COccluderTool::Frame(float fFrameTime)
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

    case STATE_ROTATE_YAW:
        RotateYaw();
        break;

    case STATE_ROTATE_PITCH:
        RotatePitch();
        break;

    case STATE_ROTATE_ROLL:
        RotateRoll();
        break;

    case STATE_SCALE:
        Scale();
        break;
    }

    m_vOldCursorPos = Input.GetCursorPos();

    UpdateHoverSelection();
}


/*====================
  COccluderTool::GetVertexPosition

  Get the current vertex position accounting for any active transformations
  ====================*/
CVec3f      COccluderTool::GetVertexPosition(uint uiOccluder, uint uiVertex)
{
    try
    {
        COccluder *pOccluder(GetOccluder(uiOccluder));
        CVec3f v3Center(V_ZERO), v3Diff(V_ZERO), v3Origin(V_ZERO);
        CAxis axis(m_vRotation);

        switch (m_eOccluderSelectMode)
        {
        case SELECT_VERTEX:
            switch(le_occluderCenterMode)
            {
            case CENTER_HOVER:
                if (m_uiHoverIndex != INVALID_INDEX)
                    v3Center = GetOccluderOrigin(m_uiHoverIndex);
                else
                    v3Center = GetOccluderOrigin(uiVertex);
                break;

            case CENTER_AVERAGE:
                v3Center = SelectionCenter();
                break;

            case CENTER_INDIVIDUAL:
                if (m_uiHoverIndex != INVALID_INDEX && m_iState == STATE_TRANSLATE_XY)
                    v3Center = GetOccluderOrigin(m_uiHoverIndex);
                else
                    v3Center = GetOccluderOrigin(uiVertex);
                break;
            }

            v3Diff = TransformPoint(pOccluder->GetPoint(uiVertex) - v3Center, axis);
            if (m_iState == STATE_TRANSLATE_XY)
                v3Origin = m_vTranslate + v3Diff * m_fScale; // use m_VTranslate as the absolute position
            else
                v3Origin = v3Center + v3Diff * m_fScale + m_vTranslate;
            break;

        case SELECT_FACE:
        default:
            switch(le_occluderCenterMode)
            {
            case CENTER_HOVER:
                if (m_uiHoverIndex != INVALID_INDEX)
                    v3Center = GetOccluderOrigin(m_uiHoverIndex);
                else
                    v3Center = GetOccluderOrigin(uiOccluder);
                break;

            case CENTER_AVERAGE:
                v3Center = SelectionCenter();
                break;

            case CENTER_INDIVIDUAL:
                if (m_uiHoverIndex != INVALID_INDEX && m_iState == STATE_TRANSLATE_XY)
                    v3Center = GetOccluderOrigin(m_uiHoverIndex);
                else
                    v3Center = GetOccluderOrigin(uiVertex);
                break;
            }

            v3Diff = TransformPoint((pOccluder->GetPoint(uiVertex) - v3Center), axis);
            if (m_iState == STATE_TRANSLATE_XY)
            {
                float fzOffset = v3Center.z - Editor.GetWorld().GetTerrainHeight(v3Center.x, v3Center.y);
                v3Origin = m_vTranslate + v3Diff * m_fScale; // use m_VTranslate as the absolute position
                v3Origin.z += fzOffset;
            }
            else
            {
                v3Origin = v3Center + v3Diff * m_fScale + m_vTranslate;
            }
            break;
        }

        return v3Origin;
    }
    catch (CException &ex)
    {
        ex.Process(_T("COCcluderTool::GetVertexPosition() - "), NO_THROW);
        return V_ZERO;
    }
}


/*====================
  COccluderTool::GetSelectionRect
  ====================*/
CRectf  COccluderTool::GetSelectionRect()
{
    return CRectf(MIN(m_vStartCursorPos.x, Input.GetCursorPos().x),
                MIN(m_vStartCursorPos.y, Input.GetCursorPos().y),
                MAX(m_vStartCursorPos.x, Input.GetCursorPos().x),
                MAX(m_vStartCursorPos.y, Input.GetCursorPos().y));
}


/*====================
  COccluderTool::GetSelectionRect
  ====================*/
int     COccluderTool::IsSelected(uint uiIndex)
{
    return m_setSelection.find(uiIndex) != m_setSelection.end();
}


/*====================
  COccluderTool::GetSelectionRect
  ====================*/
int     COccluderTool::IsHoverSelected(uint uiIndex)
{
    return m_setHoverSelection.find(uiIndex) != m_setHoverSelection.end();
}


/*====================
  COccluderTool::Draw
  ====================*/
void    COccluderTool::Draw()
{
    if (le_occluderDrawBrushCoords)
    {
        CFontMap *pFontMap(g_ResourceManager.GetFontMap(m_hFont));

        Draw2D.SetColor(0.0f, 0.0f, 0.0f);
        Draw2D.String(4.0f, Draw2D.GetScreenH() - pFontMap->GetMaxHeight() - 1.0f, Draw2D.GetScreenW(), pFontMap->GetMaxHeight(), XtoA(m_v3EndPos), m_hFont);
        Draw2D.SetColor(1.0f, 1.0f, 1.0f);
        Draw2D.String(3.0f, Draw2D.GetScreenH() - pFontMap->GetMaxHeight() - 2.0f, Draw2D.GetScreenW(), pFontMap->GetMaxHeight(), XtoA(m_v3EndPos), m_hFont);
    }

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
  COccluderTool::DrawOccluderPoly
  ====================*/
void    COccluderTool::DrawOccluderPoly(uint uiIndex)
{
    try
    {
        COccluder *pOccluder(GetOccluder(uiIndex));
        SSceneFaceVert poly[MAX_OCCLUDER_POINTS];
        MemManager.Set(poly, 0, sizeof(poly));

        // occluder vertex positions
        switch (m_eOccluderSelectMode)
        {
        case SELECT_VERTEX:
            for (uint uiPoint(0); uiPoint < pOccluder->GetNumPoints(); ++uiPoint)
            {
                if (m_uiWorkingOccluder == uiIndex && IsSelected(uiPoint))
                {
                    CVec3f v3Pos(GetVertexPosition(uiIndex, uiPoint));
                    M_CopyVec3(vec3_cast(v3Pos), poly[uiPoint].vtx);
                }
                else
                    M_CopyVec3(vec3_cast(pOccluder->GetPoint(uiPoint)), poly[uiPoint].vtx);
            }
            break;

        case SELECT_FACE:
            for (uint uiPoint(0); uiPoint < pOccluder->GetNumPoints(); ++uiPoint)
            {
                if (IsSelected(uiIndex))
                {
                    CVec3f v3Pos(GetVertexPosition(uiIndex, uiPoint));
                    M_CopyVec3(vec3_cast(v3Pos), poly[uiPoint].vtx);
                }
                else
                    M_CopyVec3(vec3_cast(pOccluder->GetPoint(uiPoint)), poly[uiPoint].vtx);
            }
            break;
        }

        // vertex colors
        switch (m_eOccluderSelectMode)
        {
        case SELECT_VERTEX:
            // Point Vertices
            for (uint z(0); z < pOccluder->GetNumPoints(); ++z)
            {
                if  (m_uiWorkingOccluder == uiIndex)
                {
                    if (IsSelected(z) && (IsHoverSelected(z) || m_uiHoverIndex == z))
                        SET_VEC4(poly[z].col, 255, 0, 255, 255);
                    else if (IsHoverSelected(z) || m_uiHoverIndex == z)
                        SET_VEC4(poly[z].col, 255, 255, 255, 255);
                    else if (IsSelected(z))
                        SET_VEC4(poly[z].col, 255, 255, 0, 255);
                    else
                        SET_VEC4(poly[z].col, 255, 0, 0, 255);
                }
                else
                {
                    SET_VEC4(poly[z].col, 255, 255, 255, 0);
                }
            }

            SceneManager.AddPoly(pOccluder->GetNumPoints(), poly, m_hLineMaterial,
                POLY_DOUBLESIDED | POLY_POINT | POLY_NO_DEPTH_TEST);

            // Wireframe vertices
            for (size_t z(0); z < pOccluder->GetNumPoints(); ++z)
            {
                if (m_uiWorkingOccluder == uiIndex)
                    SET_VEC4(poly[z].col, 255, 0, 0, 255);
                else
                    SET_VEC4(poly[z].col, 255, 255, 255, 255);
            }

            SceneManager.AddPoly(pOccluder->GetNumPoints(), poly, m_hLineMaterial,
                POLY_DOUBLESIDED | POLY_WIREFRAME | POLY_NO_DEPTH_TEST);
            break;

        case SELECT_FACE:
            // Wireframe vertices
            for (size_t z(0); z < pOccluder->GetNumPoints(); ++z)
            {
                if (IsSelected(uiIndex) && (IsHoverSelected(uiIndex) || m_uiHoverOccluder == uiIndex))
                    SET_VEC4(poly[z].col, 255, 255, 0, 255);
                else if (IsHoverSelected(uiIndex) || m_uiHoverIndex == uiIndex)
                    SET_VEC4(poly[z].col, 0, 255, 0, 255);
                else if (IsSelected(uiIndex))
                    SET_VEC4(poly[z].col, 255, 0, 0, 255);
                else
                    SET_VEC4(poly[z].col, 255, 255, 255, 255);
            }

            SceneManager.AddPoly(pOccluder->GetNumPoints(), poly, m_hLineMaterial,
                POLY_DOUBLESIDED | POLY_WIREFRAME | POLY_NO_DEPTH_TEST);
            break;
        }

        // Creamy filled center
        for (uint z(0); z < pOccluder->GetNumPoints(); ++z)
        {
            SET_VEC4(poly[z].col, 255, 255, 255, CLAMP(INT_ROUND(255 * le_occluderAlpha), 0, 255));

            //CPlane plane(pOccluder->GetPlane());
        }

        SceneManager.AddPoly(pOccluder->GetNumPoints(), poly, m_hOccluderMaterial, POLY_DOUBLESIDED);
    }
    catch (CException &ex)
    {
        ex.Process(_T("COccluderTool::DrawOccluderPoly() - "), NO_THROW);
    }
}


/*====================
  COccluderTool::Render
  ====================*/
void    COccluderTool::Render()
{
    const OccluderMap   &mapOccluders(Editor.GetWorld().GetOccluderMap());
    for (OccluderMap::const_iterator it(mapOccluders.begin()); it != mapOccluders.end(); ++it)
        DrawOccluderPoly(it->first);
}


/*====================
  COccluderTool::SelectionCenter
  ====================*/
CVec3f  COccluderTool::SelectionCenter()
{
    CVec3f  vCenter(0.0f, 0.0f, 0.0f);

    for (uiset::iterator it = m_setSelection.begin(); it != m_setSelection.end(); ++it)
        vCenter += GetOccluderOrigin(*it);

    vCenter /= float(m_setSelection.size());

    return vCenter;
}


/*====================
  COccluderTool::GetOccluderOrigin
  ====================*/
CVec3f  COccluderTool::GetOccluderOrigin(uint uiIndex)
{
    COccluder *pOccluder(NULL);

    try
    {
        switch (m_eOccluderSelectMode)
        {
        case SELECT_VERTEX:
            pOccluder = GetOccluder(m_uiWorkingOccluder);
            return pOccluder->GetPoint(uiIndex);
            break;

        case SELECT_FACE:
        default:
            pOccluder = GetOccluder(uiIndex);

            CVec3f v3Origin(V_ZERO);
            for (uint uiPoint(0); uiPoint < pOccluder->GetNumPoints(); ++uiPoint)
                v3Origin += pOccluder->GetPoint(uiPoint);

            v3Origin /= float(pOccluder->GetNumPoints());
            return v3Origin;
            break;
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("COccluderTool::GetOccluderOrigin() - "), NO_THROW);
        return V_ZERO;
    }
}


/*====================
  COccluderTool::UpdateOccluderSelectionSetDeletion

  we need to update the indices contained in a selection set after a deletion
  because the engine moves existing occluders down to fill in holes. Vertex
  deletion works the same way.
  ====================*/
void    COccluderTool::UpdateOccluderSelectionSetDeletion(uiset &set, uint uiIndex)
{
    uiset oldSet = set; // copy!
    uiset newSet;

    for (uiset::iterator it = set.begin(); it != set.end(); ++it)
    {
        if (*it >= uiIndex)
            newSet.insert(*it - 1);
        else
            newSet.insert(*it);
    }

    set = newSet;
}


/*====================
  COccluderTool::UpdateOccluderSelectionSetAddition

  Like the previous, but this allows us to add elements into the middle of the set
  ====================*/
void    COccluderTool::UpdateOccluderSelectionSetAddition(uiset &set, uint uiIndex)
{
    uiset oldSet = set; // copy!
    uiset newSet;

    for (uiset::iterator it(set.begin()); it != set.end(); ++it)
    {
        if (*it >= uiIndex)
            newSet.insert(*it + 1);
        else
            newSet.insert(*it);
    }

    set = newSet;
}


/*====================
  COccluderTool::SnapCursor
  ====================*/
void    COccluderTool::SnapCursor(const CVec3f &vOrigin)
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
  COccluderTool::RemoveOccluderVertex
  ====================*/
void    COccluderTool::RemoveOccluderVertex(size_t uiVertex, COccluder &occluder)
{
    if (occluder.GetNumPoints() == 0)
        return;

    CVec3f *pVerts(occluder.GetPoints());
    MemManager.Move(&pVerts[uiVertex], &pVerts[uiVertex + 1], sizeof(CVec3f) * (occluder.GetNumPoints() - uiVertex));

    occluder.SetNumPoints(occluder.GetNumPoints() - 1);
}


/*====================
  COccluderTool::SplitOccluderVertices
  ====================*/
void    COccluderTool::SplitOccluderVertices(size_t uiVertex0, size_t uiVertex1, COccluder &occluder)
{
    if (occluder.GetNumPoints() >= MAX_OCCLUDER_POINTS)
    {
        Console.Warn << _T("Cannot split occluder vert, it already has too many") << newl;
        return;
    }

    CVec3f *pVerts(occluder.GetPoints());
    MemManager.Move(&pVerts[uiVertex1 + 1], &pVerts[uiVertex1], sizeof(CVec3f) * (occluder.GetNumPoints() - uiVertex1));

    pVerts[uiVertex1] = (pVerts[uiVertex0] + pVerts[uiVertex1 + 1]) / 2.0f;
    occluder.SetNumPoints(occluder.GetNumPoints() + 1);
}


/*====================
  COccluderTool::CloneSelection
  ====================*/
void    COccluderTool::CloneSelection()
{
    try
    {
        switch (m_eOccluderSelectMode)
        {
        case SELECT_FACE:
            {
                uiset setNew;

                for (uiset::iterator it = m_setSelection.begin(); it != m_setSelection.end(); ++it)
                {
                    uint uiOldOccluder(*it);
                    COccluder *pOldOccluder(GetOccluder(uiOldOccluder));
                    uint uiNewOccluder(CreateOccluder(V_ZERO));
                    COccluder *pNewOccluder(GetOccluder(uiNewOccluder));

                    *pNewOccluder = *pOldOccluder;

                    setNew.insert(uiNewOccluder);

                    if (uiOldOccluder == m_uiHoverIndex)
                        m_uiHoverIndex = uiNewOccluder;
                }

                m_setSelection = setNew;
            }
            break;
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("COccluderTool::CloneSelection() - "), NO_THROW);
    }
}


/*====================
  COccluderTool::SetOccluderSelectMode
  ====================*/
void    COccluderTool::SetOccluderSelectMode(EOccluderSelectMode eSelectMode)
{
    if (m_eOccluderSelectMode != eSelectMode)
    {
        if (m_eOccluderSelectMode == SELECT_FACE && eSelectMode == SELECT_VERTEX)
        {
            m_eOccluderSelectMode = eSelectMode;

            if (m_setSelection.size() == 1)
            {
                uint uiWorkingOccluder(*m_setSelection.begin());
                Cancel();
                m_uiWorkingOccluder = uiWorkingOccluder;
            }
            else if (m_setSelection.find(m_uiHoverIndex) != m_setSelection.end())
            {
                Cancel();
                m_uiWorkingOccluder = m_uiHoverIndex;
            }
            else
            {
                Cancel();
            }
        }
        else if (m_eOccluderSelectMode == SELECT_VERTEX && eSelectMode == SELECT_FACE)
        {
            m_eOccluderSelectMode = eSelectMode;

            if (m_uiWorkingOccluder != INVALID_INDEX)
            {
                uint uiWorkingOccluder(m_uiWorkingOccluder);
                Cancel();
                m_setSelection.insert(uiWorkingOccluder);
            }
            else
            {
                Cancel();
            }
        }
        else
        {
            m_eOccluderSelectMode = eSelectMode;
            Cancel();
        }
    }

}



/*--------------------
  cmdOccluderEditMode
  --------------------*/
UI_VOID_CMD(OccluderEditMode, 1)
{
    if (vArgList.size() < 1)
    {
        Console << _T("syntax: entityeditmode create|select|translate|rotate|scale") << newl;
        return;
    }

    tstring sValue(vArgList[0]->Evaluate());

    if (sValue == _T("create"))
    {
        le_occluderEditMode = OCCLUDER_CREATE;
        OccluderEditMode.Trigger(_T("Create"));
        return;
    }
    else if (sValue == _T("select"))
    {
        le_occluderEditMode = OCCLUDER_SELECT;
        OccluderEditMode.Trigger(_T("Select"));
        return;
    }
    else if (sValue == _T("translate") || sValue == _T("translate_xy"))
    {
        le_occluderEditMode = OCCLUDER_TRANSLATE_XY;
        OccluderEditMode.Trigger(_T("Translate XY"));
        return;
    }
    else if (sValue == _T("translate_z"))
    {
        le_occluderEditMode = OCCLUDER_TRANSLATE_Z;
        OccluderEditMode.Trigger(_T("Translate Z"));
        return;
    }
    else if (sValue == _T("rotate") || sValue == _T("rotate_yaw"))
    {
        le_occluderEditMode = OCCLUDER_ROTATE_YAW;
        OccluderEditMode.Trigger(_T("Rotate Yaw"));
        return;
    }
    else if (sValue == _T("rotate_pitch"))
    {
        le_occluderEditMode = OCCLUDER_ROTATE_PITCH;
        OccluderEditMode.Trigger(_T("Rotate Pitch"));
        return;
    }
    else if (sValue == _T("rotate_roll"))
    {
        le_occluderEditMode = OCCLUDER_ROTATE_ROLL;
        OccluderEditMode.Trigger(_T("Rotate Roll"));
        return;
    }
    else if (sValue == _T("scale"))
    {
        le_occluderEditMode = OCCLUDER_SCALE;
        OccluderEditMode.Trigger(_T("Scale"));
        return;
    }
}


/*--------------------
  cmdOccluderCenterMode
  --------------------*/
UI_VOID_CMD(OccluderCenterMode, 1)
{
    if (vArgList.size() < 1)
    {
        Console << _T("syntax: entitycentermode average|hover|individual") << newl;
        return;
    }

    tstring sValue(vArgList[0]->Evaluate());

    if (sValue == _T("average"))
    {
        le_occluderCenterMode = CENTER_AVERAGE;
        return;
    }
    else if (sValue == _T("hover"))
    {
        le_occluderCenterMode = CENTER_HOVER;
        return;
    }
    else if (sValue == _T("individual"))
    {
        le_occluderCenterMode = CENTER_INDIVIDUAL;
        return;
    }
}


/*--------------------
  cmdOccluderSelectMode
  --------------------*/
UI_VOID_CMD(OccluderSelectMode, 1)
{
    try
    {
        if (vArgList.size() < 1)
            EX_MESSAGE(_T("syntax: OccluderSelectMode vertex|edge|face"));

        tstring sValue(vArgList[0]->Evaluate());

        if (sValue == _T("vertex"))
        {
            GET_TOOL(Occluder, OCCLUDER)->SetOccluderSelectMode(SELECT_VERTEX);
            return;
        }
        else if (sValue == _T("edge"))
        {
            GET_TOOL(Occluder, OCCLUDER)->SetOccluderSelectMode(SELECT_EDGE);
            return;
        }
        else if (sValue == _T("face"))
        {
            GET_TOOL(Occluder, OCCLUDER)->SetOccluderSelectMode(SELECT_FACE);
            return;
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("cmdOccluderSelectMode() - "), NO_THROW);
        return;
    }
}


/*--------------------
  cmdOccluderSplit
  --------------------*/
UI_VOID_CMD(OccluderSplit, 0)
{
    try
    {
        GET_TOOL(Occluder, OCCLUDER)->Split();
    }
    catch (CException &ex)
    {
        ex.Process(_T("cmdOccluderSelectMode() - "), NO_THROW);
    }
}
