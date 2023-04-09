// (C)2007 S2 Games
// c_scenestats.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_scenestats.h"
#include "c_draw2d.h"
#include "c_input.h"
#include "c_fontmap.h"
#include "c_material.h"
#include "c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CSceneStats SceneStats; // the global singleton

const TCHAR *g_aszSceneStatModeNames[] =
{
    _T("Totals"),
    _T("Phase"),
    _T("Batch Type"),
    _T("Combined")
};

const TCHAR *g_aszSceneStatPhaseNames[] =
{
    _T("Color"),
    _T("Shadow")
};

const TCHAR *g_aszSceneStatBatchTypeNames[] =
{
    _T("Debug"),
    _T("Effect"),
    _T("Foliage"),
    _T("Static Mesh"),
    _T("Dynamic Mesh"),
    _T("Scenepoly"),
    _T("Terrain"),
    _T("Tree Billboard"),
    _T("Tree Branch"),
    _T("Tree Frond"),
    _T("Tree Leaf")
};
//=============================================================================


/*====================
  CSceneStats::CSceneStats
  ====================*/
CSceneStats::CSceneStats() :
m_bActive(false),
m_bDraw(false),
m_bRecord(false),
m_eMode(SS_TOTALS)
{
}


/*====================
  CSceneStats::~CSceneStats
  ====================*/
CSceneStats::~CSceneStats()
{
    m_bRecord = false;
    m_bActive = false;
}


/*====================
  CSceneStats::RecordBatch
  ====================*/
void    CSceneStats::RecordBatch(int iVerts, int iTris, EMaterialPhase ePhase, ESceneStatBatchType eBatchType)
{
    int iPhaseIndex(ePhase == PHASE_SHADOW ? 1 : 0);

    m_iBatches += 1;
    m_iVerts += iVerts;
    m_iTris += iTris;

    m_aiPhaseBatches[iPhaseIndex] += 1;
    m_aiPhaseVerts[iPhaseIndex] += iVerts;
    m_aiPhaseTris[iPhaseIndex] += iTris;

    m_aiBatchTypeBatches[eBatchType] += 1;
    m_aiBatchTypeVerts[eBatchType] += iVerts;
    m_aiBatchTypeTris[eBatchType] += iTris;

    m_aiCombinedBatches[iPhaseIndex][eBatchType] += 1;
    m_aiCombinedVerts[iPhaseIndex][eBatchType] += iVerts;
    m_aiCombinedTris[iPhaseIndex][eBatchType] += iTris;
}


/*====================
  CSceneStats::ResetFrame
  ====================*/
void    CSceneStats::ResetFrame()
{
    m_iBatches = 0;
    m_iVerts = 0;
    m_iTris = 0;

    MemManager.Set(m_aiPhaseBatches, 0, sizeof(m_aiPhaseBatches));
    MemManager.Set(m_aiPhaseVerts, 0, sizeof(m_aiPhaseVerts));
    MemManager.Set(m_aiPhaseTris, 0, sizeof(m_aiPhaseTris));

    MemManager.Set(m_aiBatchTypeBatches, 0, sizeof(m_aiBatchTypeBatches));
    MemManager.Set(m_aiBatchTypeVerts, 0, sizeof(m_aiBatchTypeVerts));
    MemManager.Set(m_aiBatchTypeTris, 0, sizeof(m_aiBatchTypeTris));

    MemManager.Set(m_aiCombinedBatches, 0, sizeof(m_aiCombinedBatches));
    MemManager.Set(m_aiCombinedVerts, 0, sizeof(m_aiCombinedVerts));
    MemManager.Set(m_aiCombinedTris, 0, sizeof(m_aiCombinedTris));
}


/*====================
  CSceneStats::Frame
  ====================*/
void    CSceneStats::Frame()
{
    if (!m_bActive)
        return;

    // Steal all input while the profiler window is active
    while (!Input.IsEmpty())
    {
        const SIEvent &event = Input.Pop();
        switch(event.eType)
        {
        case INPUT_AXIS:
            break;

        case INPUT_BUTTON:
            // Filter out key releases
            if (event.cAbs.fValue == 0)
                break;

            switch(event.uID.btn)
            {
            case BUTTON_F1:
                m_bActive = false;
                break;
            case BUTTON_ESC:
                m_bDraw = m_bActive = false;
                break;
            default:
                break;
            }
            break;

        case INPUT_CHARACTER:
            if (event.uID.chr >= 32)
            {
                TCHAR key = event.uID.chr;

                if ((key >= '0' && key <= '9') || (key >= 'a' && key <= 'f'))
                {
                    int i = key >= '0' && key <= '9' ? key - '0' : key - 'a' + 10;

                    if (i >= 1 && i <= NUM_SCENESTAT_MODES)
                        m_eMode = ESceneStatMode(i - 1);
                }
            }
            break;

        default:
            break;
        }
    }
}


/*====================
  CSceneStats::Draw
  ====================*/
void    CSceneStats::Draw()
{
    if (!m_bDraw)
        return;

    PROFILE("CSceneStats::Draw");

    ResHandle hFont(g_ResourceManager.LookUpName(_T("system_medium"), RES_FONTMAP));
    CFontMap *pFontMap(g_ResourceManager.GetFontMap(hFont));
    if (pFontMap == NULL)
        return;

    const float FONT_WIDTH = pFontMap->GetFixedAdvance();
    const float FONT_HEIGHT = pFontMap->GetMaxHeight();
    const int   PANEL_WIDTH = 74;
    const float START_X = FONT_WIDTH;
    const float START_Y = FONT_HEIGHT;

    float fLines = 6.0f;

    switch (m_eMode)
    {
    case SS_TOTALS:
        break;
    case SS_PHASE:
        fLines += 2.0f;
        break;
    case SS_BATCHTYPE:
        fLines += float(NUM_SSBATCH_TYPES);
        break;
    case SS_COMBINED:
        fLines += 2.0f * float(NUM_SSBATCH_TYPES + 2);
        break;
    case NUM_SCENESTAT_MODES:
        K2_UNREACHABLE();
        break;
    }

    float fDrawY = START_Y;
    tstring sStr;

    Draw2D.SetColor(0.2f, 0.2f, 0.2f, 0.5f);
    Draw2D.Rect(START_X - 2, START_Y - 2, FONT_WIDTH * PANEL_WIDTH + 4, FONT_HEIGHT * fLines + 4);

    if (m_bActive)
    {
        Draw2D.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        Draw2D.RectOutline(START_X - 2, START_Y - 2, FONT_WIDTH * PANEL_WIDTH + 4, FONT_HEIGHT * fLines + 4, 1);
    }

    Draw2D.SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    float fDrawX(START_X);

    for (int i(0); i < NUM_SCENESTAT_MODES; ++i)
    {
        if (m_eMode == i)
            Draw2D.SetColor(0.0f, 1.0f, 0.0f, 1.0f);

        sStr = XtoA(i + 1) + _T(". ") + g_aszSceneStatModeNames[i];
        Draw2D.String(fDrawX, fDrawY, sStr, hFont);
        fDrawX += (sStr.length() + 6) * FONT_WIDTH;

        if (m_eMode == i)
            Draw2D.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    }

    fDrawY += FONT_HEIGHT * 2;

    Draw2D.String(START_X, fDrawY, _T("Type           Batches   Verts     Tris      Batches/s Verts/s   Tris/s   "), hFont);
    fDrawY += FONT_HEIGHT;

    Draw2D.String(START_X, fDrawY, _T("========================================================================="), hFont);
    fDrawY += FONT_HEIGHT;

    switch (m_eMode)
    {
    case SS_TOTALS:
        break;
    case SS_PHASE:
        {
            for (int i(0); i < 2; ++i)
            {
                sStr = XtoA(g_aszSceneStatPhaseNames[i], FMT_ALIGNLEFT, 14) + SPACE
                    + XtoA(m_aiPhaseBatches[i], FMT_ALIGNLEFT, 9) + SPACE
                    + XtoA(m_aiPhaseVerts[i], FMT_ALIGNLEFT, 9) + SPACE
                    + XtoA(m_aiPhaseTris[i], FMT_ALIGNLEFT, 9) + SPACE
                    + XtoA(INT_ROUND(m_aiPhaseBatches[i] / MsToSec(Host.GetFrameLength())), FMT_ALIGNLEFT, 9) + SPACE
                    + XtoA(INT_ROUND(m_aiPhaseVerts[i] / MsToSec(Host.GetFrameLength())), FMT_ALIGNLEFT, 9) + SPACE
                    + XtoA(INT_ROUND(m_aiPhaseTris[i] / MsToSec(Host.GetFrameLength())), FMT_ALIGNLEFT, 9) + SPACE;

                Draw2D.String(START_X, fDrawY, sStr, hFont);
                fDrawY += FONT_HEIGHT;
            }
        } break;
    case SS_BATCHTYPE:
        {
            for (int i(0); i < NUM_SSBATCH_TYPES; ++i)
            {
                sStr = XtoA(g_aszSceneStatBatchTypeNames[i], FMT_ALIGNLEFT, 14) + SPACE
                    + XtoA(m_aiBatchTypeBatches[i], FMT_ALIGNLEFT, 9) + SPACE
                    + XtoA(m_aiBatchTypeVerts[i], FMT_ALIGNLEFT, 9) + SPACE
                    + XtoA(m_aiBatchTypeTris[i], FMT_ALIGNLEFT, 9) + SPACE
                    + XtoA(INT_ROUND(m_aiBatchTypeBatches[i] / MsToSec(Host.GetFrameLength())), FMT_ALIGNLEFT, 9) + SPACE
                    + XtoA(INT_ROUND(m_aiBatchTypeVerts[i] / MsToSec(Host.GetFrameLength())), FMT_ALIGNLEFT, 9) + SPACE
                    + XtoA(INT_ROUND(m_aiBatchTypeTris[i] / MsToSec(Host.GetFrameLength())), FMT_ALIGNLEFT, 9) + SPACE;

                Draw2D.String(START_X, fDrawY, sStr, hFont);
                fDrawY += FONT_HEIGHT;
            }
        } break;
    case SS_COMBINED:
        {
            for (int i(0); i < 2; ++i)
            {
                sStr = g_aszSceneStatPhaseNames[i];
                Draw2D.String(START_X, fDrawY, sStr, hFont);
                fDrawY += FONT_HEIGHT;

                for (int j(0); j < NUM_SSBATCH_TYPES; ++j)
                {
                    sStr = XtoA(g_aszSceneStatBatchTypeNames[j], FMT_ALIGNLEFT, 14) + SPACE
                        + XtoA(m_aiCombinedBatches[i][j], FMT_ALIGNLEFT, 9) + SPACE
                        + XtoA(m_aiCombinedVerts[i][j], FMT_ALIGNLEFT, 9) + SPACE
                        + XtoA(m_aiCombinedTris[i][j], FMT_ALIGNLEFT, 9) + SPACE
                        + XtoA(INT_ROUND(m_aiCombinedBatches[i][j] / MsToSec(Host.GetFrameLength())), FMT_ALIGNLEFT, 9) + SPACE
                        + XtoA(INT_ROUND(m_aiCombinedVerts[i][j] / MsToSec(Host.GetFrameLength())), FMT_ALIGNLEFT, 9) + SPACE
                        + XtoA(INT_ROUND(m_aiCombinedTris[i][j] / MsToSec(Host.GetFrameLength())), FMT_ALIGNLEFT, 9) + SPACE;

                    Draw2D.String(START_X, fDrawY, sStr, hFont);
                    fDrawY += FONT_HEIGHT;
                }

                fDrawY += FONT_HEIGHT;
            }
        } break;
    case NUM_SCENESTAT_MODES:
        K2_UNREACHABLE();
        break;
    }

    Draw2D.SetColor(1.0f, 1.0f, 0.0f, 1.0f);

    sStr = _T("Totals         ")
        + XtoA(m_iBatches, FMT_ALIGNLEFT, 9) + SPACE
        + XtoA(m_iVerts, FMT_ALIGNLEFT, 9) + SPACE
        + XtoA(m_iTris, FMT_ALIGNLEFT, 9) + SPACE
        + XtoA(INT_ROUND(m_iBatches / MsToSec(Host.GetFrameLength())), FMT_ALIGNLEFT, 9) + SPACE
        + XtoA(INT_ROUND(m_iVerts / MsToSec(Host.GetFrameLength())), FMT_ALIGNLEFT, 9) + SPACE
        + XtoA(INT_ROUND(m_iTris / MsToSec(Host.GetFrameLength())), FMT_ALIGNLEFT, 9) + SPACE;

    Draw2D.String(START_X, fDrawY, sStr, hFont);
    fDrawY += FONT_HEIGHT;

    Draw2D.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}


/*--------------------
  cmdSceneStatsStart
  --------------------*/
CMD(SceneStatsStart)
{
    SceneStats.SetDraw(true);
    SceneStats.SetActive(true);

    return true;
}


/*--------------------
  cmdSceneStatsDraw
  --------------------*/
CMD(SceneStatsDraw)
{
    SceneStats.SetDraw(true);

    return true;
}


