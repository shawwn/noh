// (C)2008 S2 Games
// c_clifftool.cpp
//
// Cliff tool
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "editor_common.h"

#include "editor.h"

#include "c_clifftool.h"
#include "../k2/c_cliffdefinitionresource.h"

#include "../k2/c_brush.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_vid.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_world.h"
#include "../k2/c_uicmd.h"
#include "../k2/c_draw2d.h"
#include "../k2/c_fontmap.h"
#include "../k2/c_uitrigger.h"
#include "../k2/c_bitmap.h"
#include "../k2/c_input.h"
#include "../k2/c_resourcemanager.h"
#include "../k2/c_rampresource.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_INT    (le_cliffMode,                                      CLIFF_CREATE);
CVAR_INT    (le_cliffClearHeight,                               0);
CVAR_UINT   (le_cliffVariation,                                 0);
CVAR_STRING (le_cliffDefinition,                                "/world/cliffs/legion/Legion.cliff");
CVAR_BOOL   (le_cliffDrawBrushInfluence,                        true);
CVAR_BOOL   (le_cliffRandom,                                    false);
CVAR_FLOAT  (le_cliffBrushInfluenceAlpha,                       1.0f);


UI_TRIGGER(CliffMode);

//=============================================================================

/*====================
  CCliffTool::CCliffTool
  ====================*/
CCliffTool::CCliffTool() :
ITool(TOOL_CLIFF, _T("cliff")),
m_bWorking(false),
m_bPrimaryDown(false),
m_bSecondaryDown(false),
m_bFirstLoop(true),
m_uiCliffDrewMap(0),
m_uiMapSize(0),
m_iOldX(-1),
m_iOldY(-1),
m_iXCliffCenter(0),
m_iYCliffCenter(0),
m_iXOffset(0),
m_iYOffset(0),
m_hLineMaterial(g_ResourceManager.Register(_T("/core/materials/line.material"), RES_MATERIAL)),
m_hCliffMaterial(g_ResourceManager.Register(_T("/core/materials/cliff.material"), RES_MATERIAL)),
m_bValidPosition(false),
m_sLastCliffDef(TSNULL),
m_CliffDefinitionHandle(INVALID_RESOURCE),
m_hFont(g_ResourceManager.LookUpName(_T("system_medium"), RES_FONTMAP))
{
    CliffMode.Trigger(_T("Raise/Lower"));
}

#define FRONT_ROTATION_T 270
#define FRONT_ROTATION_B 90
#define FRONT_ROTATION_L 0
#define FRONT_ROTATION_R 180
#define OC_ROTATION_TR 0
#define OC_ROTATION_TL 90
#define OC_ROTATION_BL 180
#define OC_ROTATION_BR 270
#define IC_ROTATION_TR 270
#define IC_ROTATION_TL 0
#define IC_ROTATION_BL 90
#define IC_ROTATION_BR 180
#define WEDGE_ROTATION_TL 0
#define WEDGE_ROTATION_TR 90
#define WEDGE_ROTATION_BL 180
#define WEDGE_ROTATION_BR 270
#define OCT1_ROTATION_TR 0
#define OCT1_ROTATION_TL 90
#define OCT1_ROTATION_BL 180
#define OCT1_ROTATION_BR 270
#define OCT2_ROTATION_TR 0
#define OCT2_ROTATION_TL 90
#define OCT2_ROTATION_BL 180
#define OCT2_ROTATION_BR 270
#define ICT1_ROTATION_TL 0
#define ICT1_ROTATION_BL 90
#define ICT1_ROTATION_BR 180
#define ICT1_ROTATION_TR 270
#define ICT2_ROTATION_BR 0
#define ICT2_ROTATION_TR 90
#define ICT2_ROTATION_TL 180
#define ICT2_ROTATION_BL 270
#define FT1_ROTATION_BL 90
#define FT1_ROTATION_TL 0
#define FT1_ROTATION_TR 270
#define FT1_ROTATION_BR 180
#define FT2_ROTATION_TR 0 
#define FT2_ROTATION_TL 90
#define FT2_ROTATION_BL 180
#define FT2_ROTATION_BR 270
#define ICS_ROTATION_TL 180
#define ICS_ROTATION_TR 90
#define ICS_ROTATION_BR 0
#define ICS_ROTATION_BL 270
#define WT_ROTATION_TR 0
#define WT_ROTATION_TL 90
#define WT_ROTATION_BL 180
#define WT_ROTATION_BR 270
#define WS_ROTATION_BR 0
#define WS_ROTATION_TR 90
#define WS_ROTATION_TL 180
#define WS_ROTATION_BL 270


/*====================
  CCliffTool::PrimaryUp

  Left mouse button up action
  ====================*/
void    CCliffTool::PrimaryUp()
{
    if(le_cliffMode == CLIFF_PAINT && !m_bInverse)
        m_bWorking = false;

    m_bPrimaryDown = false;

}


/*====================
  CCliffTool::PrimaryDown

  Default left mouse button down action
  ====================*/
void    CCliffTool::PrimaryDown()
{
    m_bPrimaryDown = true;

    if(m_bSecondaryDown)
        return;

    m_bFirstLoop = true;

    if (m_uiCliffDrewMap)
    {
        K2_DELETE_ARRAY(m_uiCliffDrewMap);
        m_uiCliffDrewMap = 0;
    }

    m_uiMapSize = (Editor.GetWorld().GetVertCliffMapArea() / 32) + 1;

    m_uiCliffDrewMap = K2_NEW_ARRAY(ctx_Editor, uint, m_uiMapSize);

    for(uint uiMap(0); uiMap < m_uiMapSize; ++uiMap)
    {
        m_uiCliffDrewMap[uiMap] = 0;
    }

    if(le_cliffMode == CLIFF_PAINT)
    {
        m_bWorking = true;
        m_bInverse = false;
    }

    if(!m_bValidPosition)
        return;

    switch (le_cliffMode)
    {
    case CLIFF_CREATE:
        CliffDraw();
        break;
    case CLIFF_WC3:
        m_iCliffHeight = CliffMapGet() + 1;
        CliffClear();
        break;
    case CLIFF_CLEAR:
        m_iCliffHeight = le_cliffClearHeight;
        CliffClear();
        break;
    case CLIFF_VARIATION:
        CliffVariation();
        break;
    case CLIFF_RAMPCREATE:
        RampPlace();
        break;
    }

    CalcToolProperties();
    int iCliffSize(Editor.GetWorld().GetCliffSize());
    m_iOldX = (m_iXOffset * iCliffSize - iCliffSize / 2);
    m_iOldY = (m_iYOffset * iCliffSize - iCliffSize / 2);
}


/*====================
  CCliffTool::SecondaryUp

  Right mouse button up action
  ====================*/
void    CCliffTool::SecondaryUp()
{
    if(le_cliffMode == CLIFF_PAINT && m_bInverse)
        m_bWorking = false;

    m_bSecondaryDown = false;
}


/*====================
  CCliffTool::SecondaryDown

  Default right mouse button down action
  ====================*/
void    CCliffTool::SecondaryDown()
{
    m_bSecondaryDown = true;

    if(m_bPrimaryDown)
        return;

    m_bFirstLoop = true;

    if (m_uiCliffDrewMap)
    {
        K2_DELETE_ARRAY(m_uiCliffDrewMap);
        m_uiCliffDrewMap = 0;
    }

    m_uiMapSize = (Editor.GetWorld().GetVertCliffMapArea() / 32) + 1;

    m_uiCliffDrewMap = K2_NEW_ARRAY(ctx_Editor, uint, m_uiMapSize);

    for(uint uiMap(0); uiMap < m_uiMapSize; ++uiMap)
    {
        m_uiCliffDrewMap[uiMap] = 0;
    }

    if(le_cliffMode == CLIFF_PAINT)
    {
        m_bWorking = true;
        m_bInverse = true;
    }

    if(!m_bValidPosition)
        return;

    switch (le_cliffMode)
    {
    case CLIFF_CREATE:
        CliffDraw();
        break;
    case CLIFF_WC3:
        m_iCliffHeight = CliffMapGet() - 1;
        CliffClear();
        break;
    case CLIFF_CLEAR:
        break;
    case CLIFF_VARIATION:
        break;
    case CLIFF_PAINT:
        break;
    case CLIFF_RAMPCREATE:
        RampErase();
        break;
    }

    CalcToolProperties();
    int iCliffSize(Editor.GetWorld().GetCliffSize());
    m_iOldX = (m_iXOffset * iCliffSize - iCliffSize / 2);
    m_iOldY = (m_iYOffset * iCliffSize - iCliffSize / 2);
}


/*====================
  CCliffTool::TertiaryUp

  Middle mouse button up action
  ====================*/
void    CCliffTool::TertiaryUp() {}


/*====================
  CCliffTool::TertiaryDown

  Middle mouse button down action
  ====================*/
void    CCliffTool::TertiaryDown() {}


/*====================
  CCliffTool::QuaternaryUp

  Scroll wheel up action
  ====================*/
void    CCliffTool::QuaternaryUp() {}


/*====================
  CCliffTool::QuaternaryDown

  Scroll wheel down action
  ====================*/
void    CCliffTool::QuaternaryDown() {}


/*====================
  CCliffTool::Cancel
  ====================*/
void    CCliffTool::Cancel() {}


/*====================
  CCliffTool::Delete
  ====================*/
void    CCliffTool::Delete() {}


/*====================
  CCliffTool::CalcToolProperties
  ====================*/
void     CCliffTool::CalcToolProperties()
{
    STraceInfo trace;
    //Get Brush
    CBrush *pBrush = CBrush::GetCurrentBrush();

    if (Editor.TraceCursor(trace, TRACE_TERRAIN) && pBrush)
    {
        int iCliffSize(Editor.GetWorld().GetCliffSize());
        float fWorldScale(Editor.GetWorld().GetScale());

        // Clip against the brush data
        CRecti  recBrush;
        pBrush->ClipBrush(recBrush);

        float fBrushCenterX((recBrush.left + recBrush.right) * iCliffSize / 2.0f);
        float fBrushCenterY((recBrush.top + recBrush.bottom) * iCliffSize / 2.0f);

        float fTestX((trace.v3EndPos.x - fBrushCenterX * fWorldScale) / (iCliffSize * fWorldScale));
        float fTestY((trace.v3EndPos.y - fBrushCenterY * fWorldScale) / (iCliffSize * fWorldScale));

        m_iXOffset = fTestX < 0.0f ? INT_FLOOR(fTestX) : INT_CEIL(fTestX);
        m_iYOffset = fTestY < 0.0f ? INT_FLOOR(fTestY) : INT_CEIL(fTestY);

        m_iXCliffCenter = INT_ROUND((trace.v3EndPos.x / fWorldScale) / (iCliffSize));
        m_iYCliffCenter = INT_ROUND((trace.v3EndPos.y / fWorldScale) / (iCliffSize));

        fBrushCenterX = (recBrush.left + recBrush.right) / 2.0f;
        fBrushCenterY = (recBrush.top + recBrush.bottom) / 2.0f;

        fTestX = (trace.v3EndPos.x - fBrushCenterX * fWorldScale) / (fWorldScale);
        fTestY = (trace.v3EndPos.y - fBrushCenterY * fWorldScale) / (fWorldScale);

        m_iXPaint = INT_ROUND(fTestX);
        m_iYPaint = INT_ROUND(fTestY);

        m_bValidPosition = true;
        m_v3EndPos = trace.v3EndPos;
    }
    else
    {
        m_iXOffset = 0;
        m_iYOffset = 0;
        m_iXCliffCenter = 0;
        m_iYCliffCenter = 0;
        m_iXPaint = 0;
        m_iXPaint = 0;
        m_v3EndPos.Clear();
        m_bValidPosition = false;
    }
    
    if (le_cliffDefinition != m_sLastCliffDef)
    {
        m_CliffDefinitionHandle = g_ResourceManager.Register(le_cliffDefinition, RES_CLIFFDEF);
        m_sLastCliffDef = le_cliffDefinition;
    }
}


/*====================
  CCliffTool::Enter
  ====================*/
void    CCliffTool::Enter()
{
}


/*====================
  CCliffTool::CliffModify
  ====================*/
void    CCliffTool::CliffModify(byte *pRegion, const CRecti &recArea, const CBrush &brush, bool bAdd)
{
    int iBrushSize(brush.GetBrushSize());

    int iRegionIndex(0);

    for (int y(0); y < recArea.GetHeight(); ++y)
    {
        for (int x(0); x < recArea.GetWidth(); ++x)
        {
            if (brush[BRUSH_INDEX(x, y)])
                pRegion[iRegionIndex] = bAdd ? 1 : 0;
            
            ++iRegionIndex;
        }
    }
}


/*====================
  CCliffTool::PaintCliff
  ====================*/
void    CCliffTool::PaintCliff(float fFrameTime)
{
    byte *pRegion(nullptr);

    try
    {
        CBrush *pBrush(CBrush::GetCurrentBrush());
        if (pBrush == nullptr)
            EX_ERROR(_T("No brush selected"));

        int iX = m_iXPaint;
        int iY = m_iYPaint;

        // Clip against the brush data
        CRecti  recClippedBrush;
        if (!pBrush->ClipBrush(recClippedBrush))
            EX_WARN(_T("Bad Brush Clip"));

        // Clip the brush against the world
        recClippedBrush.Shift(iX, iY);
        if (!Editor.GetWorld().ClipRect(recClippedBrush, TILE_SPACE))
            EX_WARN(_T("Bad World Clip"));

        // Get the region
        pRegion = K2_NEW_ARRAY(ctx_Editor, byte, recClippedBrush.GetArea());//m_tmpByteMap;//

        if (pRegion == nullptr)
        {
            EX_ERROR(_T("Failed to allocate region"));
        }

        if (!Editor.GetWorld().GetRegion(WORLD_TILE_CLIFF_MAP, recClippedBrush, pRegion))
        {
            if(pRegion)
            {   K2_DELETE_ARRAY(pRegion); pRegion = 0;  }
            EX_ERROR(_T("Failed to retrieve region"));
        }

        // Perform the operation
        recClippedBrush.Shift(-iX, -iY);

        ClampCliffRectToGrid(&recClippedBrush);

        CliffModify(pRegion, recClippedBrush, *pBrush, !m_bInverse);

        // Apply the modified region
        recClippedBrush.Shift(iX, iY);
        if (!Editor.GetWorld().SetRegion(WORLD_TILE_CLIFF_MAP, recClippedBrush, pRegion))
        {
            if(pRegion)
            {   K2_DELETE_ARRAY(pRegion); pRegion = 0;  }
            EX_ERROR(_T("SetRegion failed"));
        }

        // Notify the video drivers about the update
        for (int y(recClippedBrush.top); y <= recClippedBrush.bottom; ++y)
        {
            for (int x = recClippedBrush.left; x <= recClippedBrush.right; ++x)
            {
                Vid.Notify(VID_NOTIFY_TERRAIN_SHADER_MODIFIED, x, y, 0, &Editor.GetWorld());
                Vid.Notify(VID_NOTIFY_TERRAIN_NORMAL_MODIFIED, x, y, 0, &Editor.GetWorld());
            }
        }

        if(pRegion)
        {   K2_DELETE_ARRAY(pRegion); pRegion = 0;  }
    }
    catch (CException &ex)
    {
        if (pRegion)
        {   K2_DELETE_ARRAY(pRegion); pRegion = 0; }

        ex.Process(_T("CCliffTool::PaintCliff() - "), NO_THROW);
    }
}


/*====================
  CCliffTool::Frame
 ====================*/
void    CCliffTool::Frame(float fFrameTime)
{
    CalcToolProperties();

    if(!m_bValidPosition)
        return;

    int iCliffSize(Editor.GetWorld().GetCliffSize());

    if(!Input.IsButtonDown(BUTTON_MOUSEL))
        m_bPrimaryDown = false;

    if(!Input.IsButtonDown(BUTTON_MOUSER))
        m_bSecondaryDown = false;

    if(!m_bPrimaryDown && !m_bSecondaryDown)
    {
        m_iOldX = -1;
        m_iOldY = -1;
        return;
    }

    if(m_iOldX == (m_iXOffset * iCliffSize - iCliffSize / 2) && m_iOldY == (m_iYOffset * iCliffSize - iCliffSize / 2))
    {
        if(le_cliffMode == CLIFF_PAINT)
            if (m_bPrimaryDown || m_bSecondaryDown)
                PaintCliff(0);
        return;
    }
    
    if(m_bPrimaryDown || m_bSecondaryDown)
    {
        m_iOldX = (m_iXOffset * iCliffSize - iCliffSize / 2);
        m_iOldY = (m_iYOffset * iCliffSize - iCliffSize / 2);
    }
    else
    {
        m_iOldX = -1;
        m_iOldY = -1;
    }

    switch (le_cliffMode)
    {
    case CLIFF_CREATE:
        CliffDraw();
        break;
    case CLIFF_WC3:
        CliffClear();
        break;
    case CLIFF_CLEAR:
        m_iCliffHeight = le_cliffClearHeight;
        CliffClear();
        break;
    case CLIFF_VARIATION:
        CliffVariation();
        break;
    case CLIFF_PAINT:
        PaintCliff(0);
        break;
    case CLIFF_RAMPCREATE:
        if (m_bPrimaryDown)
            RampPlace();
        if (m_bSecondaryDown)
            RampErase();
        break;
    }
}


/*====================
  CCliffTool::Draw
  ====================*/
void    CCliffTool::Draw()
{
    if (le_cliffDrawBrushInfluence)
    {
        CFontMap *pFontMap(g_ResourceManager.GetFontMap(m_hFont));

        Draw2D.SetColor(0.0f, 0.0f, 0.0f);
        Draw2D.String(4.0f, Draw2D.GetScreenH() - pFontMap->GetMaxHeight() - 1.0f, Draw2D.GetScreenW(), pFontMap->GetMaxHeight(), XtoA(m_v3EndPos), m_hFont);
        Draw2D.SetColor(1.0f, 1.0f, 1.0f);
        Draw2D.String(3.0f, Draw2D.GetScreenH() - pFontMap->GetMaxHeight() - 2.0f, Draw2D.GetScreenW(), pFontMap->GetMaxHeight(), XtoA(m_v3EndPos), m_hFont);
    }
}


/*====================
  CCliffTool::Render
  ====================*/
void    CCliffTool::Render()
{
    SSceneFaceVert poly[1024];
    MemManager.Set(poly, 0, sizeof(poly));
    int p = 0;

    if (le_cliffMode == CLIFF_PAINT)
    {
        CBrush *pBrush = CBrush::GetCurrentBrush();
        float fTileSize = Editor.GetWorld().GetScale();

        if (!pBrush)
            return;

        int iX = m_iXPaint, iY = m_iYPaint;

        for (int y = 0; y < pBrush->GetBrushSize(); ++y)
        {
            for (int x = 0; x < pBrush->GetBrushSize(); ++x)
            {
                int i = x + y * pBrush->GetBrushSize();

                if (p >= 1024) // restart batch if we overflow
                {
                    SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_POINTLIST | POLY_NO_DEPTH_TEST);
                    SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
                    MemManager.Set(poly, 0, sizeof(poly));
                    p = 0;
                }

                // left
                if ((x > 0 && (*pBrush)[i] && !(*pBrush)[i - 1]) || (x == 0 && (*pBrush)[i]))
                {
                    int dX = iX + x;
                    int dY = iY + y;

                    if (Editor.GetWorld().IsInBounds(dX, dY, GRID_SPACE) && Editor.GetWorld().IsInBounds(dX, dY + 1, GRID_SPACE))
                    {
                        poly[p].vtx[0] = dX * fTileSize;
                        poly[p].vtx[1] = dY * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(dX, dY);
                        SET_VEC4(poly[p].col, 0, 176, 0, 255);
                        ++p;

                        poly[p].vtx[0] = dX * fTileSize;
                        poly[p].vtx[1] = (dY + 1) * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(dX, (dY + 1));
                        SET_VEC4(poly[p].col, 0, 176, 0, 255);
                        ++p;
                    }
                }

                if (p >= 1024) // restart batch if we overflow
                {
                    SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_POINTLIST | POLY_NO_DEPTH_TEST);
                    SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
                    MemManager.Set(poly, 0, sizeof(poly));
                    p = 0;
                }

                // right
                if ((x < pBrush->GetBrushSize() - 1 && (*pBrush)[i] && !(*pBrush)[i + 1]) || (x == pBrush->GetBrushSize() - 1 && (*pBrush)[i]))
                {
                    int dX = iX + x;
                    int dY = iY + y;

                    if (Editor.GetWorld().IsInBounds(dX + 1, dY, GRID_SPACE) && Editor.GetWorld().IsInBounds(dX + 1, dY + 1, GRID_SPACE))
                    {
                        poly[p].vtx[0] = (dX + 1) * fTileSize;
                        poly[p].vtx[1] = dY * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint((dX + 1), dY);
                        SET_VEC4(poly[p].col, 0, 176, 0, 255);
                        ++p;

                        poly[p].vtx[0] = (dX + 1) * fTileSize;
                        poly[p].vtx[1] = (dY + 1) * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint((dX + 1), (dY+ 1));
                        SET_VEC4(poly[p].col, 0, 176, 0, 255);
                        ++p;
                    }
                }

                if (p >= 1024) // restart batch if we overflow
                {
                    SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_POINTLIST | POLY_NO_DEPTH_TEST);
                    SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
                    MemManager.Set(poly, 0, sizeof(poly));
                    p = 0;
                }

                // top
                if ((y > 0 && (*pBrush)[i] && !(*pBrush)[i - pBrush->GetBrushSize()]) || (y == 0 && (*pBrush)[i]))
                {
                    int dX = iX + x;
                    int dY = iY + y;

                    if (Editor.GetWorld().IsInBounds(dX, dY, GRID_SPACE) && Editor.GetWorld().IsInBounds(dX + 1, dY, GRID_SPACE))
                    {
                        poly[p].vtx[0] = dX * fTileSize;
                        poly[p].vtx[1] = dY * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(dX, dY);
                        SET_VEC4(poly[p].col, 0, 176, 0, 255);
                        ++p;

                        poly[p].vtx[0] = (dX + 1) * fTileSize;
                        poly[p].vtx[1] = dY * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint((dX + 1), dY);
                        SET_VEC4(poly[p].col, 0, 176, 0, 255);
                        ++p;
                    }
                }

                if (p >= 1024) // restart batch if we overflow
                {
                    SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_POINTLIST | POLY_NO_DEPTH_TEST);
                    SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
                    MemManager.Set(poly, 0, sizeof(poly));
                    p = 0;
                }

                // bottom
                if ((y < pBrush->GetBrushSize() - 1 && (*pBrush)[i] && !(*pBrush)[i+pBrush->GetBrushSize()]) || (y == pBrush->GetBrushSize() - 1 && (*pBrush)[i]))
                {
                    int dX = iX + x;
                    int dY = iY + y;

                    if (Editor.GetWorld().IsInBounds(dX + 1, dY + 1, GRID_SPACE) && Editor.GetWorld().IsInBounds(dX, dY + 1, GRID_SPACE))
                    {
                        poly[p].vtx[0] = (dX + 1) * fTileSize;
                        poly[p].vtx[1] = (dY + 1) * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint((dX + 1), (dY+ 1));
                        SET_VEC4(poly[p].col, 0, 176, 0, 255);
                        ++p;

                        poly[p].vtx[0] = dX * fTileSize;
                        poly[p].vtx[1] = (dY + 1) * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(dX, (dY + 1));
                        SET_VEC4(poly[p].col, 0, 176, 0, 255);
                        ++p;
                    }
                }
            }
        }
    }

    
    if (le_cliffMode == CLIFF_CREATE || le_cliffMode == CLIFF_CLEAR || le_cliffMode == CLIFF_WC3 || le_cliffMode == CLIFF_VARIATION || le_cliffMode == CLIFF_DEFINITION)
    {
        MemManager.Set(poly, 0, sizeof(poly));
        p = 0;
        CBrush *pBrush = CBrush::GetCurrentBrush();
        float fTileSize = Editor.GetWorld().GetScale();

        if (!pBrush)
            return;

        int iCliffSize(Editor.GetWorld().GetCliffSize());
        int iX(m_iXOffset * iCliffSize - iCliffSize / 2);
        int iY(m_iYOffset * iCliffSize - iCliffSize / 2);
        int iBrushSize(pBrush->GetBrushSize());

        for (int y = 0; y < iBrushSize * iCliffSize; ++y)
        {
            for (int x = 0; x < iBrushSize * iCliffSize; ++x)
            {
                int i = x / iCliffSize + y / iCliffSize * iBrushSize;
                
                if (p >= 1024) // restart batch if we overflow
                {
                    SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_POINTLIST | POLY_NO_DEPTH_TEST);
                    SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
                    MemManager.Set(poly, 0, sizeof(poly));
                    p = 0;
                }

                // left
                if (((x > 0 && (*pBrush)[i] && !(*pBrush)[i - 1]) || (x == 0 && (*pBrush)[i])) && x % iCliffSize == 0)
                {
                    int dX = iX + x;
                    int dY = iY + y;

                    if (Editor.GetWorld().IsInBounds(dX, dY, GRID_SPACE) && Editor.GetWorld().IsInBounds(dX, dY + 1, GRID_SPACE))
                    {
                        poly[p].vtx[0] = dX * fTileSize;
                        poly[p].vtx[1] = dY * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(dX, dY);
                        SET_VEC4(poly[p].col, 0, 170, 0, 255);
                        ++p;

                        poly[p].vtx[0] = dX * fTileSize;
                        poly[p].vtx[1] = (dY + 1) * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(dX, (dY + 1));
                        SET_VEC4(poly[p].col, 0, 170, 0, 255);
                        ++p;
                    }
                }

                if (p >= 1024) // restart batch if we overflow
                {
                    SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_POINTLIST | POLY_NO_DEPTH_TEST);
                    SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
                    MemManager.Set(poly, 0, sizeof(poly));
                    p = 0;
                }
                
                //// right
                if (((x < iBrushSize * iCliffSize - 1 && (*pBrush)[i] && !(*pBrush)[i + 1]) || (x == iBrushSize - 1 && (*pBrush)[i])) && x % iCliffSize == 3)
                {
                    int dX = iX + x;
                    int dY = iY + y;

                    if (Editor.GetWorld().IsInBounds(dX + 1, dY, GRID_SPACE) && Editor.GetWorld().IsInBounds(dX + 1, dY + 1, GRID_SPACE))
                    {
                        poly[p].vtx[0] = (dX + 1) * fTileSize;
                        poly[p].vtx[1] = dY * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint((dX + 1), dY);
                        SET_VEC4(poly[p].col, 0, 170, 0, 255);
                        ++p;

                        poly[p].vtx[0] = (dX + 1) * fTileSize;
                        poly[p].vtx[1] = (dY + 1) * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint((dX + 1), (dY+ 1));
                        SET_VEC4(poly[p].col, 0, 170, 0, 255);
                        ++p;
                    }
                }

                if (p >= 1024) // restart batch if we overflow
                {
                    SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_POINTLIST | POLY_NO_DEPTH_TEST);
                    SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
                    MemManager.Set(poly, 0, sizeof(poly));
                    p = 0;
                }
                
                // top
                if (((y > 0 && (*pBrush)[i] && !(*pBrush)[i - iBrushSize]) || (y == 0 && (*pBrush)[i])) && y % iCliffSize == 0)
                {
                    int dX = iX + x;
                    int dY = iY + y;

                    if (Editor.GetWorld().IsInBounds(dX, dY, GRID_SPACE) && Editor.GetWorld().IsInBounds(dX + 1, dY, GRID_SPACE))
                    {
                        poly[p].vtx[0] = dX * fTileSize;
                        poly[p].vtx[1] = dY * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(dX, dY);
                        SET_VEC4(poly[p].col, 0, 170, 0, 255);
                        ++p;

                        poly[p].vtx[0] = (dX + 1) * fTileSize;
                        poly[p].vtx[1] = dY * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint((dX + 1), dY);
                        SET_VEC4(poly[p].col, 0, 170, 0, 255);
                        ++p;
                    }
                }

                if (p >= 1024) // restart batch if we overflow
                {
                    SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_POINTLIST | POLY_NO_DEPTH_TEST);
                    SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
                    MemManager.Set(poly, 0, sizeof(poly));
                    p = 0;
                }

                // bottom
                if (((y < iBrushSize * iCliffSize - 1 && (*pBrush)[i] && !(*pBrush)[i + iBrushSize]) || (y == iBrushSize * iCliffSize - 1 && (*pBrush)[i])) && y % iCliffSize == 3)
                {
                    int dX = iX + x;
                    int dY = iY + y;

                    if (Editor.GetWorld().IsInBounds(dX + 1, dY + 1, GRID_SPACE) && Editor.GetWorld().IsInBounds(dX, dY + 1, GRID_SPACE))
                    {
                        poly[p].vtx[0] = (dX + 1) * fTileSize;
                        poly[p].vtx[1] = (dY + 1) * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint((dX + 1), (dY+ 1));
                        SET_VEC4(poly[p].col, 0, 170, 0, 255);
                        ++p;

                        poly[p].vtx[0] = dX * fTileSize;
                        poly[p].vtx[1] = (dY + 1) * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(dX, (dY + 1));
                        SET_VEC4(poly[p].col, 0, 170, 0, 255);
                        ++p;
                    }
                } 
            }
        }
    }

    if (le_cliffMode == CLIFF_RAMPCREATE)
    {
        MemManager.Set(poly, 0, sizeof(poly));
        p = 0;

        //Coordinates
        int iCliffSize(Editor.GetWorld().GetCliffSize());
        float fTileSize(Editor.GetWorld().GetScale());
        
        int iX = m_iXCliffCenter * iCliffSize;
        int iY = m_iYCliffCenter * iCliffSize;
        
        float fRampLeftWidth(2);
        float fRampRightWidth(2);

        CVec4b vColor(255, 0, 0, 255);
        CRectf rectRampPlace(
            (iX - (iCliffSize / 2)) * fTileSize, 
            (iY - (iCliffSize / 2)) * fTileSize, 
            (iX + iCliffSize - (iCliffSize / 2)) * fTileSize, 
            (iY + iCliffSize - (iCliffSize / 2)) * fTileSize);

        ushort unRampType(CanPlaceRamp(m_iXCliffCenter, m_iYCliffCenter));

        if (unRampType & RAMP_FLAG_SET && !(unRampType & RAMP_FLAG_EX_REMOVABLE))
        {
            vColor = CVec4b(50, 205, 50, 255);
            rectRampPlace = CRectf((iX + iCliffSize) * fTileSize, (iY - (iCliffSize * fRampLeftWidth) + iCliffSize) * fTileSize, (iX - iCliffSize) * fTileSize, (iY + (iCliffSize * fRampRightWidth) - iCliffSize) * fTileSize);
        }
        else if (unRampType & RAMP_FLAG_EX_REMOVABLE)
        {
            vColor = CVec4b(240, 90, 0, 255);
            rectRampPlace = CRectf((iX + iCliffSize) * fTileSize, (iY - (iCliffSize * fRampLeftWidth) + iCliffSize) * fTileSize, (iX - iCliffSize) * fTileSize, (iY + (iCliffSize * fRampRightWidth) - iCliffSize) * fTileSize);
        }

        if (rectRampPlace.left < 0)
            rectRampPlace.left = 0;
        if (rectRampPlace.top < 0)
            rectRampPlace.top = 0;
        if (rectRampPlace.right > Editor.GetWorld().GetWorldWidth())
            rectRampPlace.right = Editor.GetWorld().GetWorldWidth();
        if (rectRampPlace.bottom > Editor.GetWorld().GetWorldHeight())
            rectRampPlace.bottom = Editor.GetWorld().GetWorldHeight();

        // left
        poly[p].vtx[0] = rectRampPlace.left;
        poly[p].vtx[1] = rectRampPlace.top;
        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(INT_ROUND(rectRampPlace.left / fTileSize), INT_ROUND(rectRampPlace.top / fTileSize));
        poly[p].col = vColor; 
        ++p;

        poly[p].vtx[0] = rectRampPlace.left;
        poly[p].vtx[1] = rectRampPlace.bottom;
        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(INT_ROUND(rectRampPlace.left / fTileSize), INT_ROUND(rectRampPlace.bottom / fTileSize)); 
        poly[p].col = vColor;
        ++p;
        
        // right
        poly[p].vtx[0] = rectRampPlace.right;
        poly[p].vtx[1] = rectRampPlace.top;
        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(INT_ROUND(rectRampPlace.right / fTileSize), INT_ROUND(rectRampPlace.top / fTileSize));
        poly[p].col = vColor;
        ++p;

        poly[p].vtx[0] = rectRampPlace.right;
        poly[p].vtx[1] = rectRampPlace.bottom;
        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(INT_ROUND(rectRampPlace.right / fTileSize), INT_ROUND(rectRampPlace.bottom / fTileSize)); 
        poly[p].col = vColor;
        ++p;
        
        // top
        poly[p].vtx[0] = rectRampPlace.left;
        poly[p].vtx[1] = rectRampPlace.top;
        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(INT_ROUND(rectRampPlace.left / fTileSize), INT_ROUND(rectRampPlace.top / fTileSize));
        poly[p].col = vColor;
        ++p;

        poly[p].vtx[0] = rectRampPlace.right;
        poly[p].vtx[1] = rectRampPlace.top;
        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(INT_ROUND(rectRampPlace.right / fTileSize), INT_ROUND(rectRampPlace.top / fTileSize));
        poly[p].col = vColor;
        ++p;

        // bottom
        poly[p].vtx[0] = rectRampPlace.left;
        poly[p].vtx[1] = rectRampPlace.bottom;
        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(INT_ROUND(rectRampPlace.left / fTileSize), INT_ROUND(rectRampPlace.bottom / fTileSize));
        poly[p].col = vColor;
        ++p;

        poly[p].vtx[0] = rectRampPlace.right;
        poly[p].vtx[1] = rectRampPlace.bottom;
        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(INT_ROUND(rectRampPlace.right / fTileSize), INT_ROUND(rectRampPlace.bottom / fTileSize));
        poly[p].col = vColor;
        ++p;
        } 

    
    if (p > 0)
    {
        SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_POINTLIST | POLY_NO_DEPTH_TEST);
        SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
    }
    
}


/*--------------------
  cmdSetCliffMode
  --------------------*/
UI_VOID_CMD(SetCliffMode, 1)
{
    if (vArgList.size() < 1)
    {
        Console << _T("syntax: SetCliffMode create|paint|clear|variation|createramp|warriorcliff3") << newl;
        return;
    }

    tstring sValue(vArgList[0]->Evaluate());

    if (sValue == _T("create"))
    {
        le_cliffMode = CLIFF_CREATE;
        CliffMode.Trigger(_T("Raise/Lower"));
        return;
    }
    else if (sValue == _T("paint"))
    {
        le_cliffMode = CLIFF_PAINT;
        CliffMode.Trigger(_T("Paint"));
        return;
    }
    else if (sValue == _T("clear"))
    {
        le_cliffMode = CLIFF_CLEAR;
        CliffMode.Trigger(_T("Clear"));
        return;
    }
    else if (sValue == _T("variation"))
    {
        le_cliffMode = CLIFF_VARIATION;
        CliffMode.Trigger(_T("Variation"));
        return;
    }
    else if (sValue == _T("createramp"))
    {
        le_cliffMode = CLIFF_RAMPCREATE;
        CliffMode.Trigger(_T("Create Ramp"));
        return;
    }
    else if (sValue == _T("warriorcliff3"))
    {
        le_cliffMode = CLIFF_WC3;
        CliffMode.Trigger(_T("Warrior Cliff 3 Mode"));
        return;
    }

    return;
}


/*====================
  CCliffTool::CliffClear
  ====================*/
void    CCliffTool::CliffClear()
{
    //Alter Cliff Height Map
    CliffMapSet(m_iCliffHeight);

    //Get Brush
    CBrush *pBrush = CBrush::GetCurrentBrush();

    // Clip against the brush data
    CRecti  modifiedArea;
    if (!pBrush->ClipBrush(modifiedArea))
        return;

    // Clip the brush against the world
    modifiedArea.Shift(m_iXOffset, m_iYOffset);
    modifiedArea.Crop(0, 0, Editor.GetWorld().GetCliffGridWidth(), Editor.GetWorld().GetCliffGridHeight());
    if (!modifiedArea.IsNormalized())
        return;

    ClampCliffRectToGrid(&modifiedArea);

    CRecti enforcedArea = modifiedArea;

    //Enforce Height
    bool bEnforced(false);
    for (int y(modifiedArea.top); y < modifiedArea.bottom; y++)
    {
        for (int x(modifiedArea.left); x < modifiedArea.right; x++)
        {
            bEnforced = EnforceHeight(x, y, enforcedArea) == true ? true : bEnforced;
        }
    }

    for (int x(enforcedArea.left - 1); x <= enforcedArea.right; ++x)
    {
        for (int y(enforcedArea.top - 1); y <= enforcedArea.bottom; ++y)
        {
            if (!bEnforced && (x == enforcedArea.right ||  y == enforcedArea.bottom))
                continue;

            CalculateTile(x, y);
            CheckForRampsToErase(x, y);
        }
    }

    CalculateCliffBlockers(enforcedArea);
}


/*====================
  CCliffTool::CliffDraw
  ====================*/
void    CCliffTool::CliffDraw()
{
    //Alter Cliff Height Map
    CliffMapAdd(m_bPrimaryDown ? 1 : -1);

    //Get Brush
    CBrush *pBrush = CBrush::GetCurrentBrush();

    // Clip against the brush data
    CRecti  modifiedArea;
    if (!pBrush->ClipBrush(modifiedArea))
        return;

    // Clip the brush against the world
    modifiedArea.Shift(m_iXOffset, m_iYOffset);
    modifiedArea.Crop(0, 0, Editor.GetWorld().GetCliffGridWidth(), Editor.GetWorld().GetCliffGridHeight());
    if (!modifiedArea.IsNormalized())
        return;

    ClampCliffRectToGrid(&modifiedArea);

    CRecti enforcedArea = modifiedArea;

    //Enforce Height
    bool bEnforced(false);
    for (int y(modifiedArea.top); y < modifiedArea.bottom; y++)
    {
        for (int x(modifiedArea.left); x < modifiedArea.right; x++)
        {
            bEnforced = EnforceHeight(x, y, enforcedArea) == true ? true : bEnforced;
        }
    }

    for (int x(enforcedArea.left - 1); x <= enforcedArea.right; ++x)
    {
        for (int y(enforcedArea.top - 1); y <= enforcedArea.bottom; ++y)
        {
            if (!bEnforced && (x == enforcedArea.right ||  y == enforcedArea.bottom))
                continue;

                //check for ramps
            CalculateTile(x, y);
            CheckForRampsToErase(x, y);
        }
    }

    CalculateCliffBlockers(enforcedArea);
}


/*====================
  CCliffTool::CanBlockVert
  ====================*/
bool CCliffTool::CanBlockVert(int iX, int iY)
{
        //Check titles in a 3x3 square
    CRecti rArea(iX - 1, iY - 1, iX + 1, iY + 1);

        //Clamp again
    Editor.GetWorld().ClipRect(rArea, TILE_SPACE);

    byte *pRegion = K2_NEW_ARRAY(ctx_Editor, byte, rArea.GetArea());

        //Get the Region
    Editor.GetWorld().GetRegion(WORLD_TILE_CLIFF_MAP, rArea, pRegion);

        //Check for cliff tiles
    for(int x(0); x < rArea.GetArea(); ++x)
    {
        if(pRegion[x] == 1)
            return true;    //Found a cliff tile block this vert
    }

        //No verts found
    return false;
}


/*====================
  CCliffTool::CheckForRampsToErase
  ====================*/
void    CCliffTool::CheckForRampsToErase(int iX, int iY)
{
    //Static Brush
    CRecti  modifiedArea(iX - 1, iY - 1, iX + 2, iY + 2);

    ClampCliffRectToGrid(&modifiedArea);

    for (int x(modifiedArea.left); x <= modifiedArea.right; ++x)
    {
        for (int y(modifiedArea.top); y <= modifiedArea.bottom; ++y)
        {
            if ((x == modifiedArea.left || x == modifiedArea.right || y == modifiedArea.top || y == modifiedArea.bottom))
            {
                if (Editor.GetWorld().GetRampTile(x, y) & RAMP_FLAG_DIAGONAL)
                    RampErase(x, y);
            }
            else
            {
                RampErase(x, y);
            }
        }
    }
}


/*====================
  CCliffTool::RampErase
  ====================*/
void    CCliffTool::RampErase()
{
    RampErase(m_iXCliffCenter, m_iYCliffCenter);
}


void    CCliffTool::RampErase(int iX, int iY)
{
    if ( iX < 0 || iY < 0 || iX > Editor.GetWorld().GetCliffTileWidth() - 1 || iY > Editor.GetWorld().GetCliffTileHeight() - 1)
        return;

    //Static Brush
    CRecti  modifiedArea(iX - 1, iY - 1, iX, iY);
    ushort  unRampFlags(Editor.GetWorld().GetRampTile(iX, iY));

    if (!(unRampFlags & RAMP_FLAG_SET))
        return;

    Editor.GetWorld().SetRampTile(iX, iY, 0);

    if (unRampFlags & RAMP_FLAG_DIAGONAL && unRampFlags & RAMP_FLAG_LEFT_RIGHT_INCLINE && unRampFlags & RAMP_FLAG_INVERT_INCLINE)
    {
        modifiedArea.Stretch(1, 1);
        modifiedArea.Shift(-1, -1);
    }
    else if (unRampFlags & RAMP_FLAG_DIAGONAL && unRampFlags & RAMP_FLAG_LEFT_RIGHT_INCLINE && !(unRampFlags & RAMP_FLAG_INVERT_INCLINE))
    {
        modifiedArea.Stretch(1, 1);
    }
    else if (unRampFlags & RAMP_FLAG_DIAGONAL && !(unRampFlags & RAMP_FLAG_LEFT_RIGHT_INCLINE) && unRampFlags & RAMP_FLAG_INVERT_INCLINE)
    {
        modifiedArea.Stretch(1, 1);
        modifiedArea.Shift(0, -1);
    }
    else if (unRampFlags & RAMP_FLAG_DIAGONAL && !(unRampFlags & RAMP_FLAG_LEFT_RIGHT_INCLINE) && !(unRampFlags & RAMP_FLAG_INVERT_INCLINE))
    {
        modifiedArea.Stretch(1, 1);
        modifiedArea.Shift(-1, 0);
    }

    ClampCliffRectToGrid(&modifiedArea);

    for (int x(modifiedArea.left); x <= modifiedArea.right; ++x)
    {
        for (int y(modifiedArea.top); y <= modifiedArea.bottom; ++y)
        {
            CalculateTile(x, y);
        }
    }

    ushort unRampTileLeft(0);
    ushort unRampTileRight(0);

    if (unRampFlags & RAMP_FLAG_DIAGONAL)
    {
        if (unRampFlags & RAMP_FLAG_LEFT_RIGHT_INCLINE && unRampFlags & RAMP_FLAG_INVERT_INCLINE)
        {
            unRampTileLeft = Editor.GetWorld().GetRampTile(iX - 1, iY + 1);
            unRampTileRight = Editor.GetWorld().GetRampTile(iX + 1, iY - 1);
            if (unRampTileRight & RAMP_FLAG_SET && unRampTileLeft & RAMP_FLAG_SET)
            {
                RampErase(iX + 1, iY - 1);
                RampErase(iX - 1, iY + 1);
            }
            else if (unRampTileLeft & RAMP_FLAG_SET)
            {
                unRampTileLeft = CanPlaceRamp(iX - 1, iY + 1);
                unRampTileLeft &= ~RAMP_FLAG_EX_REMOVABLE;
                RampPlaceModel(iX - 1, iY + 1, unRampTileLeft);
            }
            else if (unRampTileRight & RAMP_FLAG_SET)
            {
                unRampTileRight = CanPlaceRamp(iX + 1, iY - 1);
                unRampTileRight &= ~RAMP_FLAG_EX_REMOVABLE;
                RampPlaceModel(iX + 1, iY - 1, unRampTileRight);
            }
        }
        else if (unRampFlags & RAMP_FLAG_LEFT_RIGHT_INCLINE && !(unRampFlags & RAMP_FLAG_INVERT_INCLINE))
        {
            unRampTileLeft = Editor.GetWorld().GetRampTile(iX + 1, iY - 1);
            unRampTileRight = Editor.GetWorld().GetRampTile(iX - 1, iY + 1);
            if (unRampTileRight & RAMP_FLAG_SET && unRampTileLeft & RAMP_FLAG_SET)
            {
                RampErase(iX + 1, iY - 1);
                RampErase(iX - 1, iY + 1);
            }
            else if (unRampTileLeft & RAMP_FLAG_SET)
            {
                unRampTileLeft = CanPlaceRamp(iX + 1, iY - 1);
                unRampTileLeft &= ~RAMP_FLAG_EX_REMOVABLE;
                RampPlaceModel(iX + 1, iY - 1, unRampTileLeft);
            }
            else if (unRampTileRight & RAMP_FLAG_SET)
            {
                unRampTileRight = CanPlaceRamp(iX - 1, iY + 1);
                unRampTileRight &= ~RAMP_FLAG_EX_REMOVABLE;
                RampPlaceModel(iX - 1, iY + 1, unRampTileRight);
            }
        }
        else if (!(unRampFlags & RAMP_FLAG_LEFT_RIGHT_INCLINE) && unRampFlags & RAMP_FLAG_INVERT_INCLINE)
        {
            unRampTileLeft = Editor.GetWorld().GetRampTile(iX - 1, iY - 1);
            unRampTileRight = Editor.GetWorld().GetRampTile(iX + 1, iY + 1);
            if (unRampTileRight & RAMP_FLAG_SET && unRampTileLeft & RAMP_FLAG_SET)
            {
                RampErase(iX - 1, iY - 1);
                RampErase(iX + 1, iY + 1);
            }
            else if (unRampTileLeft & RAMP_FLAG_SET)
            {
                unRampTileLeft = CanPlaceRamp(iX - 1, iY - 1);
                unRampTileLeft &= ~RAMP_FLAG_EX_REMOVABLE;
                RampPlaceModel(iX - 1, iY - 1, unRampTileLeft);
            }
            else if (unRampTileRight & RAMP_FLAG_SET)
            {
                unRampTileRight = CanPlaceRamp(iX + 1, iY + 1);
                unRampTileRight &= ~RAMP_FLAG_EX_REMOVABLE;
                RampPlaceModel(iX + 1, iY + 1, unRampTileRight);
            }
        }
        else if (!(unRampFlags & RAMP_FLAG_LEFT_RIGHT_INCLINE) && !(unRampFlags & RAMP_FLAG_INVERT_INCLINE))
        {
            unRampTileLeft = Editor.GetWorld().GetRampTile(iX + 1, iY + 1);
            unRampTileRight = Editor.GetWorld().GetRampTile(iX - 1, iY - 1);
            if (unRampTileRight & RAMP_FLAG_SET && unRampTileLeft & RAMP_FLAG_SET)
            {
                RampErase(iX + 1, iY + 1);
                RampErase(iX - 1, iY - 1);
            }
            else if (unRampTileLeft & RAMP_FLAG_SET)
            {
                unRampTileLeft = CanPlaceRamp(iX + 1, iY + 1);
                unRampTileLeft &= ~RAMP_FLAG_EX_REMOVABLE;
                RampPlaceModel(iX + 1, iY + 1, unRampTileLeft);
            }
            else if (unRampTileRight & RAMP_FLAG_SET)
            {
                unRampTileRight = CanPlaceRamp(iX - 1, iY - 1);
                unRampTileRight &= ~RAMP_FLAG_EX_REMOVABLE;
                RampPlaceModel(iX - 1, iY - 1, unRampTileRight);
            }
        }
    }
    else
    {
        if (unRampFlags & RAMP_FLAG_LEFT_RIGHT_INCLINE && unRampFlags & RAMP_FLAG_INVERT_INCLINE)
        {
            unRampTileLeft = Editor.GetWorld().GetRampTile(iX, iY + 1);
            unRampTileRight = Editor.GetWorld().GetRampTile(iX, iY - 1);

            if (unRampTileLeft & RAMP_FLAG_SET)
            {
                unRampTileLeft |= RAMP_FLAG_SECOND_INWARD;
                RampPlaceModel(iX, iY + 1, unRampTileLeft);
            }

            if (unRampTileRight & RAMP_FLAG_SET)
            {
                unRampTileRight |= RAMP_FLAG_FIRST_INWARD;
                RampPlaceModel(iX, iY - 1, unRampTileRight);
            }
        }
        else if (!(unRampFlags & RAMP_FLAG_LEFT_RIGHT_INCLINE) && !(unRampFlags & RAMP_FLAG_INVERT_INCLINE))
        {
            unRampTileLeft = Editor.GetWorld().GetRampTile(iX + 1, iY);
            unRampTileRight = Editor.GetWorld().GetRampTile(iX - 1, iY);

            if (unRampTileLeft & RAMP_FLAG_SET)
            {
                unRampTileLeft |= RAMP_FLAG_SECOND_INWARD;
                RampPlaceModel(iX + 1, iY, unRampTileLeft);
            }

            if (unRampTileRight & RAMP_FLAG_SET)
            {
                unRampTileRight |= RAMP_FLAG_FIRST_INWARD;
                RampPlaceModel(iX - 1, iY, unRampTileRight);
            }
        }
        else if (unRampFlags & RAMP_FLAG_LEFT_RIGHT_INCLINE && !(unRampFlags & RAMP_FLAG_INVERT_INCLINE))
        {
            unRampTileLeft = Editor.GetWorld().GetRampTile(iX, iY - 1);
            unRampTileRight = Editor.GetWorld().GetRampTile(iX, iY + 1);

            if (unRampTileLeft & RAMP_FLAG_SET)
            {
                unRampTileLeft |= RAMP_FLAG_SECOND_INWARD;
                RampPlaceModel(iX, iY - 1, unRampTileLeft);
            }

            if (unRampTileRight & RAMP_FLAG_SET)
            {
                unRampTileRight |= RAMP_FLAG_FIRST_INWARD;
                RampPlaceModel(iX, iY + 1, unRampTileRight);
            }
        }
        else if (!(unRampFlags & RAMP_FLAG_LEFT_RIGHT_INCLINE) && unRampFlags & RAMP_FLAG_INVERT_INCLINE)
        {
            unRampTileLeft = Editor.GetWorld().GetRampTile(iX - 1, iY);
            unRampTileRight = Editor.GetWorld().GetRampTile(iX + 1, iY);

            if (unRampTileLeft & RAMP_FLAG_SET)
            {
                unRampTileLeft |= RAMP_FLAG_SECOND_INWARD;
                RampPlaceModel(iX - 1, iY, unRampTileLeft);
            }

            if (unRampTileRight & RAMP_FLAG_SET)
            {
                unRampTileRight |= RAMP_FLAG_FIRST_INWARD;
                RampPlaceModel(iX + 1, iY, unRampTileRight);
            }
        }
    }
    CalculateCliffBlockers(modifiedArea);
}


/*====================
  CCliffTool::ClampCliffRectToGrid
  ====================*/
void CCliffTool::ClampCliffRectToGrid(CRecti *rArea)
{
    if(rArea->left >= rArea->right)
        rArea->right = rArea->left + 1;
    if(rArea->top >= rArea->bottom)
        rArea->bottom = rArea->top + 1;

    rArea->right = MIN(rArea->right, (Editor.GetWorld().GetGridWidth() / Editor.GetWorld().GetCliffSize()) + 1);
    rArea->left = MAX(rArea->left, 0);
    rArea->top = MAX(rArea->top, 0);
    rArea->bottom = MIN(rArea->bottom, (Editor.GetWorld().GetGridHeight() / Editor.GetWorld().GetCliffSize()) + 1);
}


/*====================
  CCliffTool::ClampRectToGrid
  ====================*/
void CCliffTool::ClampRectToGrid(CRecti *rArea)
{
    if(rArea->left > rArea->right)
        rArea->right = rArea->left + 1;
    if(rArea->top > rArea->bottom)
        rArea->bottom = rArea->top + 1;

    rArea->right = MIN(rArea->right, Editor.GetWorld().GetGridWidth());
    rArea->left = MAX(rArea->left, 0);
    rArea->top = MAX(rArea->top, 0);
    rArea->bottom = MIN(rArea->bottom, Editor.GetWorld().GetGridHeight());
}


/*====================
  CCliffTool::CliffVariation
  ====================*/
void    CCliffTool::CliffVariation()
{
    //Get Brush
    CBrush *pBrush = CBrush::GetCurrentBrush();

    // Clip against the brush data
    CRecti  modifiedArea;
    if (!pBrush->ClipBrush(modifiedArea))
        return;

    // Clip the brush against the world
    modifiedArea.Shift(m_iXOffset - 1, m_iYOffset - 1);

    if (!modifiedArea.IsNormalized())
        return;

    //Clip brush against cliff map
    ClampCliffRectToGrid(&modifiedArea);

    //Calculate Tiles
    for (int x(modifiedArea.left); x <= modifiedArea.right; ++x)
    {
        for (int y(modifiedArea.top); y <= modifiedArea.bottom; ++y)
        {
            ushort  unRampFlags(Editor.GetWorld().GetRampTile(x, y));
            if (!(unRampFlags & RAMP_FLAG_SET))
                CalculateTile(x,y);
        }
    }

    modifiedArea.Stretch(4, 4);
    modifiedArea.Shift(-1, -1);
    ClampCliffRectToGrid(&modifiedArea);

    //Calculate Ramps
    for (int x(modifiedArea.left); x < modifiedArea.right; ++x)
    {
        for (int y(modifiedArea.top); y < modifiedArea.bottom; ++y)
        {
            ushort  unRampFlags(Editor.GetWorld().GetRampTile(x, y));
            if (unRampFlags & RAMP_FLAG_SET)
            {
                if (unRampFlags & RAMP_FLAG_DIAGONAL)
                {
                    Editor.GetWorld().SetRampTile(x, y, 0);
                    ushort unRampType(CanPlaceRamp(x, y));
                    if (unRampType & RAMP_FLAG_EX_DOUBLE_LEFT)
                    {
                        Editor.GetWorld().SetRampTile(x, y, unRampType);
                        int iSideRampX(0), iSideRampY(0);

                        if (unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE && unRampType & RAMP_FLAG_INVERT_INCLINE)
                        {
                            iSideRampX = -1;
                            iSideRampY = 1;
                        }
                        if (unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE && !(unRampType & RAMP_FLAG_INVERT_INCLINE))
                        {
                            iSideRampX = 1;
                            iSideRampY = -1;
                        }
                        if (!(unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE) && unRampType & RAMP_FLAG_INVERT_INCLINE)
                        {
                            iSideRampX = -1;
                            iSideRampY = -1;
                        }
                        if (!(unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE) && !(unRampType & RAMP_FLAG_INVERT_INCLINE))
                        {
                            iSideRampX = 1;
                            iSideRampY = 1;
                        }
                        ushort unRampTmpType(CanPlaceRamp(x + iSideRampX, y + iSideRampY));

                        RampPlaceModel(x + iSideRampX, y + iSideRampY, unRampTmpType);
                    }
                    
                    if (unRampType & RAMP_FLAG_EX_DOUBLE_RIGHT)
                    {
                        Editor.GetWorld().SetRampTile(x, y, unRampType);
                        int iSideRampX(0), iSideRampY(0);

                        if (unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE && unRampType & RAMP_FLAG_INVERT_INCLINE)
                        {
                            iSideRampX = 1;
                            iSideRampY = -1;
                        }
                        if (unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE && !(unRampType & RAMP_FLAG_INVERT_INCLINE))
                        {
                            iSideRampX = -1;
                            iSideRampY = 1;
                        }
                        if (!(unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE) && unRampType & RAMP_FLAG_INVERT_INCLINE)
                        {
                            iSideRampX = 1;
                            iSideRampY = 1;
                        }
                        if (!(unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE) && !(unRampType & RAMP_FLAG_INVERT_INCLINE))
                        {
                            iSideRampX = -1;
                            iSideRampY = -1;
                        }
                        ushort unRampTmpType(CanPlaceRamp(x + iSideRampX, y + iSideRampY));

                        RampPlaceModel(x + iSideRampX, y + iSideRampY, unRampTmpType);
                    }

                    RampPlaceModel(x, y, unRampType);
                }
                else if(x != modifiedArea.left || x != modifiedArea.right || y != modifiedArea.top || y != modifiedArea.bottom)
                {
                    RampPlaceModel(x, y, unRampFlags);
                }
            }
        }
    }
}


/*====================
  CCliffTool::CliffMapSet
  ====================*/
void    CCliffTool::CliffMapSet(int iCliffHeight)
{
    //Get Cliff Map
    int* pVertCliffMap = Editor.GetWorld().GetVertCliffMap();

    //Get Brush
    CBrush *pBrush = CBrush::GetCurrentBrush();

    // Clip against the brush data
    CRecti  recClippedBrush;
    if (!pBrush->ClipBrush(recClippedBrush))
        return;

    CRecti unMovedClippedBrush(recClippedBrush);

    ClampCliffRectToGrid(&unMovedClippedBrush);

    // Clip the brush against the world
    recClippedBrush.Shift(m_iXOffset, m_iYOffset);
    recClippedBrush.Crop(0, 0, Editor.GetWorld().GetCliffGridWidth(), Editor.GetWorld().GetCliffGridHeight());
    if (!recClippedBrush.IsNormalized())
        return;

    int iOldLeft(recClippedBrush.left), iOldTop(recClippedBrush.top);

    ClampCliffRectToGrid(&recClippedBrush);

    iOldLeft = recClippedBrush.left - iOldLeft;
    iOldTop = recClippedBrush.top - iOldTop;

    for (int x(0); x < recClippedBrush.GetWidth(); x++) 
    {
        for (int y(0); y < recClippedBrush.GetHeight(); y++)
        {
            if ((*pBrush)[(unMovedClippedBrush.left + x + iOldLeft) + ((unMovedClippedBrush.top + y + iOldTop) * pBrush->GetBrushSize())] > 0)
            {
                pVertCliffMap[Editor.GetWorld().GetVertCliff(recClippedBrush.left + x, recClippedBrush.top + y)] = iCliffHeight;
            }
        }
    }
}


/*====================
  CCliffTool::CliffMapAdd
  ====================*/
void    CCliffTool::CliffMapAdd(int iCliffHeight)
{
    //Get Cliff Map
    int* pVertCliffMap = Editor.GetWorld().GetVertCliffMap();

    //Get Brush
    CBrush *pBrush = CBrush::GetCurrentBrush();

    // Clip against the brush data
    CRecti  recClippedBrush;
    if (!pBrush->ClipBrush(recClippedBrush))
        return;

    CRecti unMovedClippedBrush(recClippedBrush);

    ClampCliffRectToGrid(&unMovedClippedBrush);

    // Clip the brush against the world
    recClippedBrush.Shift(m_iXOffset, m_iYOffset);

    int iOldLeft(recClippedBrush.left), iOldTop(recClippedBrush.top);

    ClampCliffRectToGrid(&recClippedBrush);
    recClippedBrush.Crop(0, 0, Editor.GetWorld().GetCliffGridWidth(), Editor.GetWorld().GetCliffGridHeight());
    if (!recClippedBrush.IsNormalized())
        return;

    iOldLeft = recClippedBrush.left - iOldLeft;
    iOldTop = recClippedBrush.top - iOldTop;

    for (int x(0); x < recClippedBrush.GetWidth(); x++) 
    {
        for (int y(0); y < recClippedBrush.GetHeight(); y++)
        {
            if ((*pBrush)[(unMovedClippedBrush.left + x + iOldLeft) + ((unMovedClippedBrush.top + y + iOldTop) * pBrush->GetBrushSize())] > 0)
            {
                uint uiCliffLoc = Editor.GetWorld().GetVertCliff(recClippedBrush.left + x, recClippedBrush.top + y);
                if ((m_uiCliffDrewMap[uiCliffLoc / 32] & BIT(uiCliffLoc % 32)) == 0) 
                {
                    pVertCliffMap[uiCliffLoc] += iCliffHeight;
                    m_uiCliffDrewMap[uiCliffLoc / 32] |= BIT(uiCliffLoc % 32);
                }
            }
        }
    }
}


/*====================
  CCliffTool::GetHeightMapAbsolute
  ====================*/
int CCliffTool::CliffMapGet()
{
    //Get Cliff Map
    int* pVertCliffMap = Editor.GetWorld().GetVertCliffMap();
    return pVertCliffMap[Editor.GetWorld().GetVertCliff(m_iXCliffCenter, m_iYCliffCenter)];
}


/*====================
  CCliffTool::EnforceHeight
  ====================*/
bool CCliffTool::EnforceHeight(int x, int y, CRecti &enforcedRect)
{
    if (x < 0 || x > Editor.GetWorld().GetGridWidth() / Editor.GetWorld().GetCliffSize() || y < 0 || y > Editor.GetWorld().GetGridHeight() / Editor.GetWorld().GetCliffSize())
        return false;

    if (x < 0) x = 0;
    if (x > Editor.GetWorld().GetGridWidth() / Editor.GetWorld().GetCliffSize()) x = Editor.GetWorld().GetGridWidth() / Editor.GetWorld().GetCliffSize();
    if (y < 0) y = 0;
    if (y > Editor.GetWorld().GetGridHeight() / Editor.GetWorld().GetCliffSize()) y = Editor.GetWorld().GetGridHeight() / Editor.GetWorld().GetCliffSize();

    //Get Cliff Map
    int* pVertCliffMap = Editor.GetWorld().GetVertCliffMap();
    
    int maxHeight = pVertCliffMap[Editor.GetWorld().GetVertCliff(x,y)] + 2;
    int minHeight = pVertCliffMap[Editor.GetWorld().GetVertCliff(x,y)] - 2;

    std::vector<CVec2i> modifiedCoords;
    bool bReturn(false);

    for (int ix = max(0, x - 1); ix <= min(x + 1, Editor.GetWorld().GetGridWidth() / Editor.GetWorld().GetCliffSize()); ix++)
    {
        for (int iy = max(0, y - 1); iy <= min(y + 1, Editor.GetWorld().GetGridHeight() / Editor.GetWorld().GetCliffSize()); iy++)
        {

            if (pVertCliffMap[Editor.GetWorld().GetVertCliff(ix,iy)] > maxHeight)
            {
                pVertCliffMap[Editor.GetWorld().GetVertCliff(ix,iy)] = maxHeight;
                enforcedRect.AddPoint(CVec2i(ix,iy));
                modifiedCoords.push_back(CVec2i(ix,iy));
                bReturn = true;
            }
            if (pVertCliffMap[Editor.GetWorld().GetVertCliff(ix,iy)] < minHeight)
            {
                pVertCliffMap[Editor.GetWorld().GetVertCliff(ix,iy)] = minHeight;
                enforcedRect.AddPoint(CVec2i(ix,iy));
                modifiedCoords.push_back(CVec2i(ix,iy));
                bReturn = true;
            }
        }
    }

    for(size_t i(0); i < modifiedCoords.size(); i++)
        EnforceHeight(modifiedCoords[i].x, modifiedCoords[i].y, enforcedRect);

    return bReturn;
}


/*====================
  CCliffTool::CalculateHiddenTiles
  ====================*/
void    CCliffTool::CalculateHiddenTiles(CRecti CliffSquare, bool bCliff)
{

    byte *pRegion = K2_NEW_ARRAY(ctx_Editor, byte, CliffSquare.GetArea());

    for (uint x(0); (int)x < CliffSquare.GetArea(); x++)
    {
            pRegion[x] = bCliff;
    }

    if (!Editor.GetWorld().SetRegion(WORLD_TILE_CLIFF_MAP, CliffSquare, pRegion))
    {
        if (pRegion)
        {   K2_DELETE_ARRAY(pRegion); pRegion = 0;  }
        return;
    }

    if (pRegion)
    {   K2_DELETE_ARRAY(pRegion); pRegion = 0;  }

    // Notify the video drivers about the update
    for (int y(CliffSquare.top); y <= CliffSquare.bottom; ++y)
    {
        for (int x = CliffSquare.left; x <= CliffSquare.right; ++x)
        {
            Vid.Notify(VID_NOTIFY_TERRAIN_SHADER_MODIFIED, x, y, 0, &Editor.GetWorld());
            Vid.Notify(VID_NOTIFY_TERRAIN_NORMAL_MODIFIED, x, y, 0, &Editor.GetWorld());
        }
    }
}


/*====================
  CCliffTool::CalculateTile
  ====================*/
void    CCliffTool::CalculateTile(int iXC, int iYC)
{

    if (iXC < 0 || iYC < 0 || iXC > Editor.GetWorld().GetGridWidth() / Editor.GetWorld().GetCliffSize() - 1 || iYC > Editor.GetWorld().GetGridHeight() / Editor.GetWorld().GetCliffSize() - 1)
        return;

    //Get Cliff Map
    int* pVertCliffMap = Editor.GetWorld().GetVertCliffMap();

    //Get Cliff Definition 
    uint* pDefinitionMap = Editor.GetWorld().GetTileCliffDefinitionMap();
    pDefinitionMap[Editor.GetWorld().GetTileCliff(iXC, iYC)] = Editor.GetWorld().AddCliffDef(m_CliffDefinitionHandle);
    CCliffDefinitionResource* CliffDefinition = static_cast<CCliffDefinitionResource*>(g_ResourceManager.Get(m_CliffDefinitionHandle));

    //Get Cliff Variation 
    uint* pVariationMap = Editor.GetWorld().GetTileCliffVariationMap(); 
    uint uiVariation = le_cliffRandom ?  (uint)K2_RAND(0, 3) : le_cliffVariation;
    pVariationMap[Editor.GetWorld().GetTileCliff(iXC, iYC)] = uiVariation;

    //Get Cliff tile size
    int iCliffSize = Editor.GetWorld().GetCliffSize();

    //declarations
    int iXX = 0;
    int iYY = 0;
    int iZZ = 0;
    CBBoxf box;
    uivector vresult;

    //Find the lowest vert
    int lowestVert = 1;
    int lowestVertHeight = GetCliffVertHeight(iXC,iYC + 1);

    if (pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC + 1,iYC + 1)] < lowestVertHeight)
    {
        lowestVertHeight = pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC + 1,iYC + 1)];
        lowestVert = 2;
    }
    if (pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC,iYC)] < lowestVertHeight)
    {
        lowestVertHeight = pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC,iYC)];
        lowestVert = 3;
    }
    if (pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC + 1,iYC)] < lowestVertHeight)
    {
        lowestVertHeight = pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC + 1,iYC)];
        lowestVert = 4;
    }

    //Find the highest vert
    int highestVertHeight = pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC,iYC + 1)];
    if (pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC + 1, iYC + 1)] > highestVertHeight)
        highestVertHeight = pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC + 1, iYC + 1)];
    if (pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC,iYC)] > highestVertHeight)
        highestVertHeight = pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC,iYC)];
    if (pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC + 1, iYC)] > highestVertHeight)
        highestVertHeight = pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC + 1, iYC)];

    unsigned int cliffCombination = 0;
    //Gather information about surrounding verts
    if (pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC,iYC + 1)] == lowestVertHeight) 
        cliffCombination += 1;
    if (pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC + 1, iYC + 1)] == lowestVertHeight) 
        cliffCombination += 10;
    if (pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC,iYC)] == lowestVertHeight) 
        cliffCombination += 100;
    if (pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC + 1, iYC)] == lowestVertHeight) 
        cliffCombination += 1000;
    if (pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC,iYC + 1)] == lowestVertHeight + 1) 
        cliffCombination += 2;
    if (pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC + 1, iYC + 1)] == lowestVertHeight + 1) 
        cliffCombination += 20;
    if (pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC,iYC)] == lowestVertHeight + 1) 
        cliffCombination += 200;
    if (pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC + 1, iYC)] == lowestVertHeight + 1) 
        cliffCombination += 2000;
    if (pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC,iYC + 1)] == lowestVertHeight + 2) 
        cliffCombination += 3;
    if (pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC + 1, iYC + 1)] == lowestVertHeight + 2) 
        cliffCombination += 30;
    if (pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC,iYC)] == lowestVertHeight + 2) 
        cliffCombination += 300;
    if (pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC + 1, iYC)] == lowestVertHeight + 2) 
        cliffCombination += 3000;

        //Paint Cliff
        CRecti CliffSquare(0, 0, iCliffSize, iCliffSize);
        CliffSquare.MoveTo(iXC * iCliffSize ,iYC * iCliffSize);

        ClampRectToGrid(&CliffSquare);

        CRecti heightRect(iXC * iCliffSize, iYC * iCliffSize, iXC * iCliffSize + iCliffSize + 1, iYC * iCliffSize + iCliffSize + 1);

        heightRect.left = CLAMP(heightRect.left, 0, Editor.GetWorld().GetGridWidth());
        heightRect.right = CLAMP(heightRect.right, 0, Editor.GetWorld().GetGridWidth());
        heightRect.bottom = CLAMP(heightRect.bottom, 0, Editor.GetWorld().GetGridWidth());
        heightRect.top = CLAMP(heightRect.top, 0, Editor.GetWorld().GetGridWidth());

        float * fpRegion = K2_NEW_ARRAY(ctx_Editor, float, heightRect.GetArea());
        
        if(!fpRegion)
            return;

        //Adjust Height Map
        for (int x(0); x < heightRect.GetWidth(); x++)
        {
            for (int y(0); y < heightRect.GetHeight(); y++)
            {
                fpRegion[x + y * (heightRect.GetWidth())] = CalculateHeightVertex(heightRect.left + x, heightRect.top + y);
            }
        }

        Editor.GetWorld().SetRegion(WORLD_VERT_HEIGHT_MAP, heightRect, fpRegion);

        if(fpRegion)
        {   K2_DELETE_ARRAY(fpRegion); fpRegion = 0;    }

        iXX = iXC * Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        iYY = iYC * Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        iZZ = lowestVertHeight * Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        
        CliffDelete(iXC, iYC);

        tstring InnerCornerPathPerm(TSNULL);
        tstring OuterCornerPathPerm(TSNULL);
        tstring WedgePathPerm(TSNULL);
        tstring FrontPathPerm(TSNULL);
        tstring InnerCornerPath256Perm(TSNULL);
        tstring OuterCornerPath256Perm(TSNULL);
        tstring WedgePath256Perm(TSNULL);
        tstring FrontPath256Perm(TSNULL);
        tstring InnerCornerTransition1PathPerm(TSNULL);
        tstring InnerCornerTransition2PathPerm(TSNULL);
        tstring OuterCornerTransition1PathPerm(TSNULL);
        tstring OuterCornerTransition2PathPerm(TSNULL);
        tstring FrontTransition1PathPerm(TSNULL);
        tstring FrontTransition2PathPerm(TSNULL);
        tstring InnerCornerSimplePathPerm(TSNULL);
        tstring WedgeTransitionPathPerm(TSNULL);
        tstring WedgeShiftPathPerm(TSNULL);

        //Rotation vertexes
        int ICrv(0);
        int OCrv(0);
        int Wrv(0);
        int Frv(0);
        int IC256rv(0);
        int OC256rv(0);
        int W256rv(0);
        int F256rv(0);
        int ICT1rv(0);
        int ICT2rv(0);
        int OCT1rv(0);
        int OCT2rv(0);
        int FT1rv(0);
        int FT2rv(0);
        int ICSrv(0);
        int WTrv(0);
        int WSrv(0);

        //Default Rotation Offset
        float ICdr(0.0f);
        float OCdr(0.0f);
        float Wdr(0.0f);
        float Fdr(0.0f);
        float IC256dr(0.0f);
        float OC256dr(0.0f);
        float W256dr(0.0f);
        float F256dr(0.0f);
        float ICT1dr(0.0f);
        float ICT2dr(0.0f);
        float OCT1dr(0.0f);
        float OCT2dr(0.0f);
        float FT1dr(0.0f);
        float FT2dr(0.0f);
        float ICSdr(0.0f);
        float WTdr(0.0f);
        float WSdr(0.0f);

        if (CliffDefinition && CliffDefinition->GetInnerCorner().GetVariation(uiVariation) != nullptr) // TKTK: This null check was added because it seems to be null sometimes
        {
            //Cliff Paths
            InnerCornerPathPerm = CliffDefinition->GetInnerCorner().GetVariation(uiVariation)->GetPiecePath();
            OuterCornerPathPerm = CliffDefinition->GetOuterCorner().GetVariation(uiVariation)->GetPiecePath();
            WedgePathPerm = CliffDefinition->GetWedge().GetVariation(uiVariation)->GetPiecePath();
            FrontPathPerm = CliffDefinition->GetFront().GetVariation(uiVariation)->GetPiecePath();
            InnerCornerPath256Perm = CliffDefinition->GetInnerCorner256().GetVariation(uiVariation)->GetPiecePath();
            OuterCornerPath256Perm = CliffDefinition->GetOuterCorner256().GetVariation(uiVariation)->GetPiecePath();
            WedgePath256Perm = CliffDefinition->GetWedge256().GetVariation(uiVariation)->GetPiecePath();
            FrontPath256Perm = CliffDefinition->GetFront256().GetVariation(uiVariation)->GetPiecePath();
            InnerCornerTransition1PathPerm = CliffDefinition->GetInnerCornerTransition1().GetVariation(uiVariation)->GetPiecePath();
            InnerCornerTransition2PathPerm = CliffDefinition->GetInnerCornerTransition2().GetVariation(uiVariation)->GetPiecePath();
            OuterCornerTransition1PathPerm = CliffDefinition->GetOuterCornerTransition1().GetVariation(uiVariation)->GetPiecePath();
            OuterCornerTransition2PathPerm = CliffDefinition->GetOuterCornerTransition2().GetVariation(uiVariation)->GetPiecePath();
            FrontTransition1PathPerm = CliffDefinition->GetFrontTransition1().GetVariation(uiVariation)->GetPiecePath();
            FrontTransition2PathPerm = CliffDefinition->GetFrontTransition2().GetVariation(uiVariation)->GetPiecePath();
            InnerCornerSimplePathPerm = CliffDefinition->GetInnerCornerSimple().GetVariation(uiVariation)->GetPiecePath();
            WedgeTransitionPathPerm = CliffDefinition->GetWedgeTransition().GetVariation(uiVariation)->GetPiecePath();
            WedgeShiftPathPerm = CliffDefinition->GetWedgeShift().GetVariation(uiVariation)->GetPiecePath();

            //Rotation vertexes
            ICrv = CliffDefinition->GetInnerCorner().GetVariation(uiVariation)->GetRotationVertex();
            OCrv = CliffDefinition->GetOuterCorner().GetVariation(uiVariation)->GetRotationVertex();
            Wrv = CliffDefinition->GetWedge().GetVariation(uiVariation)->GetRotationVertex();
            Frv = CliffDefinition->GetFront().GetVariation(uiVariation)->GetRotationVertex();
            IC256rv = CliffDefinition->GetInnerCorner256().GetVariation(uiVariation)->GetRotationVertex();
            OC256rv = CliffDefinition->GetOuterCorner256().GetVariation(uiVariation)->GetRotationVertex();
            W256rv = CliffDefinition->GetWedge256().GetVariation(uiVariation)->GetRotationVertex();
            F256rv = CliffDefinition->GetFront256().GetVariation(uiVariation)->GetRotationVertex();
            ICT1rv = CliffDefinition->GetInnerCornerTransition1().GetVariation(uiVariation)->GetRotationVertex();
            ICT2rv = CliffDefinition->GetInnerCornerTransition2().GetVariation(uiVariation)->GetRotationVertex();
            OCT1rv = CliffDefinition->GetOuterCornerTransition1().GetVariation(uiVariation)->GetRotationVertex();
            OCT2rv = CliffDefinition->GetOuterCornerTransition2().GetVariation(uiVariation)->GetRotationVertex();
            FT1rv = CliffDefinition->GetFrontTransition1().GetVariation(uiVariation)->GetRotationVertex();
            FT2rv = CliffDefinition->GetFrontTransition2().GetVariation(uiVariation)->GetRotationVertex();
            ICSrv = CliffDefinition->GetInnerCornerSimple().GetVariation(uiVariation)->GetRotationVertex();
            WTrv = CliffDefinition->GetWedgeTransition().GetVariation(uiVariation)->GetRotationVertex();
            WSrv = CliffDefinition->GetWedgeShift().GetVariation(uiVariation)->GetRotationVertex();

            //Default Rotation Offset
            ICdr = CliffDefinition->GetInnerCorner().GetVariation(uiVariation)->GetDefaultRotation();
            OCdr = CliffDefinition->GetOuterCorner().GetVariation(uiVariation)->GetDefaultRotation();
            Wdr = CliffDefinition->GetWedge().GetVariation(uiVariation)->GetDefaultRotation();
            Fdr = CliffDefinition->GetFront().GetVariation(uiVariation)->GetDefaultRotation();
            IC256dr = CliffDefinition->GetInnerCorner256().GetVariation(uiVariation)->GetDefaultRotation();
            OC256dr = CliffDefinition->GetOuterCorner256().GetVariation(uiVariation)->GetDefaultRotation();
            W256dr = CliffDefinition->GetWedge256().GetVariation(uiVariation)->GetDefaultRotation();
            F256dr = CliffDefinition->GetFront256().GetVariation(uiVariation)->GetDefaultRotation();
            ICT1dr = CliffDefinition->GetInnerCornerTransition1().GetVariation(uiVariation)->GetDefaultRotation();
            ICT2dr = CliffDefinition->GetInnerCornerTransition2().GetVariation(uiVariation)->GetDefaultRotation();
            OCT1dr = CliffDefinition->GetOuterCornerTransition1().GetVariation(uiVariation)->GetDefaultRotation();
            OCT2dr = CliffDefinition->GetOuterCornerTransition2().GetVariation(uiVariation)->GetDefaultRotation();
            FT1dr = CliffDefinition->GetFrontTransition1().GetVariation(uiVariation)->GetDefaultRotation();
            FT2dr = CliffDefinition->GetFrontTransition2().GetVariation(uiVariation)->GetDefaultRotation();
            ICSdr = CliffDefinition->GetInnerCornerSimple().GetVariation(uiVariation)->GetDefaultRotation();
            WTdr = CliffDefinition->GetWedgeTransition().GetVariation(uiVariation)->GetDefaultRotation();
            WSdr = CliffDefinition->GetWedgeShift().GetVariation(uiVariation)->GetDefaultRotation();
        }
        else
        {
            cliffCombination = 1111;
        }

        switch (cliffCombination) 
        {
        case 1111:
            CalculateHiddenTiles(CliffSquare, false);
            break;
        case 1112: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), OC_ROTATION_TL + OCdr, OuterCornerPathPerm, OCrv);
            break;
        case 1113:
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), OC_ROTATION_TL + OC256dr, OuterCornerPath256Perm, OC256rv);
            break;
        case 1121: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), OC_ROTATION_TR + OCdr, OuterCornerPathPerm, OCrv);
            break;
        case 1122: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), FRONT_ROTATION_B + Fdr, FrontPathPerm, Frv);
            break;
        case 1123:
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), FT1_ROTATION_TL + FT1dr, FrontTransition1PathPerm, FT1rv); 
            break;
        case 1131:
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), OC_ROTATION_TR + OC256dr, OuterCornerPath256Perm, OC256rv);
            break;
        case 1132: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), FT2_ROTATION_TR + FT2dr, FrontTransition2PathPerm, FT2rv);
            break;
        case 1133:
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), FRONT_ROTATION_B + F256dr, FrontPath256Perm, F256rv);
            break;
        case 1211: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), OC_ROTATION_BL + OCdr, OuterCornerPathPerm, OCrv);
            break;
        case 1212: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), FRONT_ROTATION_R + Fdr, FrontPathPerm, Frv);
            break;
        case 1213:
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), FT2_ROTATION_TL + FT2dr, FrontTransition2PathPerm, FT2rv);
            break;
        case 1221:
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), WEDGE_ROTATION_BR + Wdr, WedgePathPerm, Wrv);
            break;
        case 1222:
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), IC_ROTATION_BR + ICdr, InnerCornerPathPerm, ICrv);
            break;
        case 1223:
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), ICS_ROTATION_TL + ICSdr, InnerCornerSimplePathPerm, ICSrv);
            break;
        case 1231: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), WT_ROTATION_TR + WTdr, WedgeTransitionPathPerm, WTrv);
            break;
        case 1232: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), ICT2_ROTATION_TR + ICT2dr, InnerCornerTransition2PathPerm, ICT2rv);
            break;
        case 1233: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), OCT2_ROTATION_TL + OCT2dr, OuterCornerTransition2PathPerm, OCT2rv);
            break;
        case 1311:
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), OC_ROTATION_BL + OC256dr, OuterCornerPath256Perm, OC256rv);
            break;
        case 1312: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), FT1_ROTATION_BL + FT1dr, FrontTransition1PathPerm, FT1rv);
            break;
        case 1313:
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), FRONT_ROTATION_R + F256dr, FrontPath256Perm, F256rv);
            break;
        case 1321: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), WT_ROTATION_BL + WTdr, WedgeTransitionPathPerm, WTrv);
            break;
        case 1322: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), ICT1_ROTATION_BL + ICT1dr, InnerCornerTransition1PathPerm, ICT1rv);
            break;
        case 1323:
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), OCT1_ROTATION_TL + OCT1dr, OuterCornerTransition1PathPerm, OCT1rv);
            break;
        case 1331:
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), WEDGE_ROTATION_BR + W256dr, WedgePath256Perm, W256rv);
            break;
        case 1332: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), WS_ROTATION_TL + WSdr, WedgeShiftPathPerm, WSrv);
            break;
        case 1333:
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), IC_ROTATION_BR + IC256dr, InnerCornerPath256Perm, IC256rv);
            break;
        case 2111: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), OC_ROTATION_BR + OCdr, OuterCornerPathPerm, OCrv);
            break;
        case 2112:
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), WEDGE_ROTATION_BL + Wdr, WedgePathPerm, Wrv);
            break;
        case 2113: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), WT_ROTATION_TL + WTdr, WedgeTransitionPathPerm, WTrv);
            break;
        case 2121:
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), FRONT_ROTATION_L + Fdr, FrontPathPerm, Frv);
            break;
        case 2122:
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), IC_ROTATION_BL + ICdr, InnerCornerPathPerm, ICrv);
            break;
        case 2123: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), ICT1_ROTATION_TL + ICT1dr, InnerCornerTransition1PathPerm, ICT1rv);
            break;
        case 2131: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), FT1_ROTATION_TR + FT1dr, FrontTransition1PathPerm, FT1rv);
            break;
        case 2132: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), ICS_ROTATION_TR + ICSdr, InnerCornerSimplePathPerm, ICSrv);
            break;
        case 2133: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), OCT1_ROTATION_TR + OCT1dr, OuterCornerTransition1PathPerm, OCT1rv);
            break;
        case 2211: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), FRONT_ROTATION_T + Fdr, FrontPathPerm, Frv);
            break;
        case 2212:
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), IC_ROTATION_TR + ICdr, InnerCornerPathPerm, ICrv);
            break;
        case 2213: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), ICT2_ROTATION_TL + ICT2dr, InnerCornerTransition2PathPerm, ICT2rv);
            break;
        case 2221:
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), IC_ROTATION_TL + ICdr, InnerCornerPathPerm, ICrv);
            break;
        case 2231:
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), ICT1_ROTATION_TR + ICT1dr, InnerCornerTransition1PathPerm, ICT1rv);
            break;
        case 2311: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), FT2_ROTATION_BL + FT2dr, FrontTransition2PathPerm, FT2rv);
            break;
        case 2312:
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), ICS_ROTATION_BL + ICSdr, InnerCornerSimplePathPerm, ICSrv);
            break;
        case 2313: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), OCT2_ROTATION_BL + OCT2dr, OuterCornerTransition2PathPerm, OCT2rv);
            break;
        case 2321:
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), ICT2_ROTATION_BL + ICT2dr, InnerCornerTransition2PathPerm, ICT2rv);
            break;
        case 2331:
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), WS_ROTATION_BR + WSdr, WedgeShiftPathPerm, WSrv);
            break;
        case 3111:
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), OC_ROTATION_BR + OC256dr, OuterCornerPath256Perm, OC256rv);
            break;
        case 3112: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), WT_ROTATION_BR + WTdr, WedgeTransitionPathPerm, WTrv);
            break;
        case 3113:
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), WEDGE_ROTATION_BL + W256dr, WedgePath256Perm, W256rv);
            break;
        case 3121:
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), FT2_ROTATION_BR + FT2dr, FrontTransition2PathPerm, FT2rv);
            break;
        case 3122:
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), ICT2_ROTATION_BR + ICT2dr, InnerCornerTransition2PathPerm, ICT2rv);
            break;
        case 3123: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), WS_ROTATION_TR + WSdr, WedgeShiftPathPerm, WSrv);
            break;
        case 3131:
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), FRONT_ROTATION_L + F256dr, FrontPath256Perm, F256rv);
            break;
        case 3132: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), OCT2_ROTATION_TR + OCT2dr, OuterCornerTransition2PathPerm, OCT2rv);
            break;
        case 3133:
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), IC_ROTATION_BL + IC256dr, InnerCornerPath256Perm, IC256rv);
            break;
        case 3211:
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), FT1_ROTATION_BR + FT1dr, FrontTransition1PathPerm, FT1rv);
            break;
        case 3212: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), ICT1_ROTATION_BR + ICT1dr, InnerCornerTransition1PathPerm, ICT1rv);
            break;
        case 3213: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), WS_ROTATION_BL + WSdr, WedgeShiftPathPerm, WSrv);
            break;
        case 3221: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), ICS_ROTATION_BR + ICSdr, InnerCornerSimplePathPerm, ICSrv);
            break;
        case 3231: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), OCT1_ROTATION_BR + OCT1dr, OuterCornerTransition1PathPerm, OCT1rv);
            break;
        case 3311: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), FRONT_ROTATION_T + F256dr, FrontPath256Perm, F256rv);
            break;
        case 3312: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), OCT1_ROTATION_BL + OCT1dr, OuterCornerTransition1PathPerm, OCT1rv);
            break;
        case 3313: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), IC_ROTATION_TR + IC256dr, InnerCornerPath256Perm, IC256rv);
            break;
        case 3321: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), OCT2_ROTATION_BR + OCT2dr, OuterCornerTransition2PathPerm, OCT2rv);
            break;
        case 3331: 
            CalculateHiddenTiles(CliffSquare, true);
            CliffCreate(CVec3f(iXX, iYY, iZZ), IC_ROTATION_TL + IC256dr, InnerCornerPath256Perm, IC256rv);
            break;
        }
}


/*====================
  CCliffTool::RotationAdjustment
  ====================*/
void CCliffTool::RotationAdjustment(uint uiEntity, int iRotationVertex)
{
    CWorldEntity *pEntity(Editor.GetWorld().GetEntity(uiEntity));
    CVec3f vAdjustedPosition = pEntity->GetPosition();

    if (iRotationVertex == 1)
    {
        if (pEntity->GetAngles().z == 0)
        {
            vAdjustedPosition.y += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        }
        else if (pEntity->GetAngles().z == 180)
        {
            vAdjustedPosition.x += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        }
        else if (pEntity->GetAngles().z == 270)
        {
            vAdjustedPosition.y += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
            vAdjustedPosition.x += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        }
    }
    else if (iRotationVertex == 2)
    {
        if (pEntity->GetAngles().z == 90)
        {
            vAdjustedPosition.y += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        }
        else if (pEntity->GetAngles().z == 270)
        {
            vAdjustedPosition.x += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        }
        else if (pEntity->GetAngles().z == 0)
        {
            vAdjustedPosition.y += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
            vAdjustedPosition.x += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        }
    }
    else if (iRotationVertex == 3)
    {
        if (pEntity->GetAngles().z == 270)
        {
            vAdjustedPosition.y += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        }
        else if (pEntity->GetAngles().z == 90)
        {
            vAdjustedPosition.x += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        }
        else if (pEntity->GetAngles().z == 180)
        {
            vAdjustedPosition.x += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
            vAdjustedPosition.y += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        }
    }
    else if (iRotationVertex == 4)
    {
        if (pEntity->GetAngles().z == 180)
        {
            vAdjustedPosition.y += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        }
        else if (pEntity->GetAngles().z == 0)
        {
            vAdjustedPosition.x += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        }
        else if (pEntity->GetAngles().z == 90)
        {
            vAdjustedPosition.x += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
            vAdjustedPosition.y += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        }
    }
    else if (iRotationVertex == 5)
    {
        if (pEntity->GetAngles().z == 0)
        {
            vAdjustedPosition.x += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
            vAdjustedPosition.y += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        }
        else if (pEntity->GetAngles().z == 90)
        {
            vAdjustedPosition.y += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        }
        else if (pEntity->GetAngles().z == 180)
        {
            vAdjustedPosition.x += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        }
        else if (pEntity->GetAngles().z == 270)
        {
            vAdjustedPosition.x += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
            vAdjustedPosition.y += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        }
    }
    else if (iRotationVertex == 6)
    {
        if (pEntity->GetAngles().z == 0)
        {
            vAdjustedPosition.x += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        }
        else if (pEntity->GetAngles().z == 90)
        {
            vAdjustedPosition.x += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
            vAdjustedPosition.y += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        }
        else if (pEntity->GetAngles().z == 180)
        {
            vAdjustedPosition.x += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
            vAdjustedPosition.y += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        }
        else if (pEntity->GetAngles().z == 270)
        {
            vAdjustedPosition.y += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        }
    }
    else if (iRotationVertex == 7)
    {
        if (pEntity->GetAngles().z == 0)
        {
            vAdjustedPosition.y += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize() * 2;
        }
        else if (pEntity->GetAngles().z == 180)
        {
            vAdjustedPosition.x += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        }
        else if (pEntity->GetAngles().z == 270)
        {
            vAdjustedPosition.y += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
            vAdjustedPosition.x += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize() * 2;
        }
    }
    else if (iRotationVertex == 8)
    {
        if (pEntity->GetAngles().z == 0)
        {
            vAdjustedPosition.x += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
            vAdjustedPosition.y += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize() * 2;
        }
        else if (pEntity->GetAngles().z == 90)
        {
            vAdjustedPosition.y += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        }
        else if (pEntity->GetAngles().z == 270)
        {
            vAdjustedPosition.x += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize() * 2;
        }
    }

    pEntity->SetPosition(vAdjustedPosition);
}


/*====================
  CCliffTool::RampPlace
  ====================*/
void CCliffTool::RampPlace()
{
    int     iX(m_iXCliffCenter);
    int     iY(m_iYCliffCenter);

    ushort unRampType(CanPlaceRamp(iX, iY));

    if (unRampType & RAMP_FLAG_DIAGONAL)
    {
        if (unRampType & RAMP_FLAG_EX_DOUBLE_LEFT)
        {
            Editor.GetWorld().SetRampTile(iX, iY, unRampType);
            int iSideRampX(0), iSideRampY(0);

            if (unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE && unRampType & RAMP_FLAG_INVERT_INCLINE)
            {
                iSideRampX = -1;
                iSideRampY = 1;
            }
            if (unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE && !(unRampType & RAMP_FLAG_INVERT_INCLINE))
            {
                iSideRampX = 1;
                iSideRampY = -1;
            }
            if (!(unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE) && unRampType & RAMP_FLAG_INVERT_INCLINE)
            {
                iSideRampX = -1;
                iSideRampY = -1;
            }
            if (!(unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE) && !(unRampType & RAMP_FLAG_INVERT_INCLINE))
            {
                iSideRampX = 1;
                iSideRampY = 1;
            }
            ushort unRampTmpType(CanPlaceRamp(iX + iSideRampX, iY + iSideRampY));

            RampPlaceModel(iX + iSideRampX, iY + iSideRampY, unRampTmpType);
        }
        
        if (unRampType & RAMP_FLAG_EX_DOUBLE_RIGHT)
        {
            Editor.GetWorld().SetRampTile(iX, iY, unRampType);
            int iSideRampX(0), iSideRampY(0);

            if (unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE && unRampType & RAMP_FLAG_INVERT_INCLINE)
            {
                iSideRampX = 1;
                iSideRampY = -1;
            }
            if (unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE && !(unRampType & RAMP_FLAG_INVERT_INCLINE))
            {
                iSideRampX = -1;
                iSideRampY = 1;
            }
            if (!(unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE) && unRampType & RAMP_FLAG_INVERT_INCLINE)
            {
                iSideRampX = 1;
                iSideRampY = 1;
            }
            if (!(unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE) && !(unRampType & RAMP_FLAG_INVERT_INCLINE))
            {
                iSideRampX = -1;
                iSideRampY = -1;
            }
            ushort unRampTmpType(CanPlaceRamp(iX + iSideRampX, iY + iSideRampY));

            RampPlaceModel(iX + iSideRampX, iY + iSideRampY, unRampTmpType);
        }

        RampPlaceModel(iX, iY, unRampType);
    }
    else
    {
        RampPlaceModel(iX, iY, unRampType);
    }

}


/*====================
  CCliffTool::RampPlaceModel
  ====================*/
void CCliffTool::RampPlaceModel(int iX, int iY, ushort unRampType)
{

    int                         iCliffSize(Editor.GetWorld().GetCliffSize());
    float                       fScale(Editor.GetWorld().GetScale());
    int*                        pVertCliffMap(Editor.GetWorld().GetVertCliffMap());
    float                       fCliffWorldScale(iCliffSize * fScale);
    float                       fRampRotationRight(0.0f);
    float                       fRampRotationLeft(0.0f);
    CCliffDefinitionResource*   CliffDef(static_cast<CCliffDefinitionResource*>(g_ResourceManager.Get(m_CliffDefinitionHandle)));
    bool                        bRampsLoaded(false);
    CRampResource*              CRampDefinition(nullptr);
    tstring                     sTopModelPath(TSNULL);
    int                         iTopRotationVertex(0);
    float                       fTopRotationAdjustment(0.0f);
    tstring                     sBotModelPath(TSNULL);
    int                         iBotRotationVertex(0);
    float                       fBotRotationAdjustment(0.0f);

    if (CliffDef)
    {
        CRampDefinition = CliffDef->GetRamp();
        bRampsLoaded = CRampDefinition ? true : false;
    }

    CVec3f vRampLeftPos(0.0f, 0.0f, 0.0f);
    CVec3f vRampRightPos(0.0f, 0.0f, 0.0f);
    CRecti rectRampPlace(0, 0, 0, 0);

    if (unRampType & RAMP_FLAG_SET && !(unRampType & RAMP_FLAG_EX_REMOVABLE))
    {
        if (unRampType & RAMP_FLAG_DIAGONAL && unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE && !(unRampType & RAMP_FLAG_INVERT_INCLINE))
        {
            rectRampPlace = CRecti((iX - 1), (iY - 1), (iX + 2), (iY + 2));
            fRampRotationRight = 270.0f;
            fRampRotationLeft = 180.0f;
            vRampRightPos = CVec3f((rectRampPlace.left * iCliffSize) * fScale, ((rectRampPlace.top + 1) * iCliffSize) * fScale, (float)pVertCliffMap[Editor.GetWorld().GetVertCliff(rectRampPlace.GetMid().x, rectRampPlace.GetMid().y)] * fCliffWorldScale);
            vRampLeftPos = CVec3f((rectRampPlace.right - 2) * fScale * iCliffSize, rectRampPlace.top * fScale * iCliffSize, (float)pVertCliffMap[Editor.GetWorld().GetVertCliff(rectRampPlace.GetMid().x, rectRampPlace.GetMid().y)] * fCliffWorldScale);
        }
        else if (unRampType & RAMP_FLAG_DIAGONAL && unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE && unRampType & RAMP_FLAG_INVERT_INCLINE)
        {
            rectRampPlace = CRecti((iX - 2), (iY - 2), (iX + 1), (iY + 1));
            fRampRotationRight = 90.0f;
            fRampRotationLeft = 0.0f;
            vRampRightPos = CVec3f((rectRampPlace.right - 1) * fScale * iCliffSize, rectRampPlace.top * fScale * iCliffSize, (float)pVertCliffMap[Editor.GetWorld().GetVertCliff(rectRampPlace.GetMid().x, rectRampPlace.GetMid().y)] * fCliffWorldScale);
            vRampLeftPos = CVec3f(rectRampPlace.left * fScale * iCliffSize, (rectRampPlace.bottom - 1) * fScale * iCliffSize, (float)pVertCliffMap[Editor.GetWorld().GetVertCliff(rectRampPlace.GetMid().x, rectRampPlace.GetMid().y)] * fCliffWorldScale);
        }
        else if (unRampType & RAMP_FLAG_DIAGONAL && !(unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE) && !(unRampType & RAMP_FLAG_INVERT_INCLINE))
        {
            rectRampPlace = CRecti((iX - 2), (iY - 1), (iX + 1), (iY + 2));
            fRampRotationRight = 0.0f;
            fRampRotationLeft = 270.0f;
            vRampRightPos = CVec3f((rectRampPlace.left * iCliffSize) * fScale, (rectRampPlace.top * iCliffSize) * fScale, (float)pVertCliffMap[Editor.GetWorld().GetVertCliff(rectRampPlace.GetMid().x, rectRampPlace.GetMid().y)] * fCliffWorldScale);
            vRampLeftPos = CVec3f((rectRampPlace.left + 2) * fScale * iCliffSize, ((rectRampPlace.top + 1) * iCliffSize) * fScale,  (float)pVertCliffMap[Editor.GetWorld().GetVertCliff(rectRampPlace.GetMid().x, rectRampPlace.GetMid().y)] * fCliffWorldScale);
        }
        else if (unRampType & RAMP_FLAG_DIAGONAL && !(unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE) && unRampType & RAMP_FLAG_INVERT_INCLINE)
        {
            rectRampPlace = CRecti((iX - 1), (iY - 2), (iX + 2), (iY + 1));
            fRampRotationRight = 180.0f;
            fRampRotationLeft = 90.0f;
            vRampLeftPos = CVec3f((rectRampPlace.left * iCliffSize) * fScale, (rectRampPlace.top * iCliffSize) * fScale, (float)pVertCliffMap[Editor.GetWorld().GetVertCliff(rectRampPlace.GetMid().x, rectRampPlace.GetMid().y)] * fCliffWorldScale);
            vRampRightPos = CVec3f((rectRampPlace.left + 1) * fScale * iCliffSize, ((rectRampPlace.top + 2) * iCliffSize) * fScale, (float)pVertCliffMap[Editor.GetWorld().GetVertCliff(rectRampPlace.GetMid().x, rectRampPlace.GetMid().y)] * fCliffWorldScale);

        }
        else if (unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE && unRampType & RAMP_FLAG_INVERT_INCLINE)
        {
            rectRampPlace = CRecti((iX - 1), (iY - 1), (iX + 1), (iY + 1));
            vRampRightPos = CVec3f(rectRampPlace.left * fScale * iCliffSize, rectRampPlace.top * fScale * iCliffSize, (float)pVertCliffMap[Editor.GetWorld().GetVertCliff(rectRampPlace.GetMid().x, rectRampPlace.GetMid().y)] * fCliffWorldScale);
            vRampLeftPos = CVec3f(rectRampPlace.left * fScale * iCliffSize, (rectRampPlace.bottom - 1) * fScale * iCliffSize, (float)pVertCliffMap[Editor.GetWorld().GetVertCliff(rectRampPlace.GetMid().x, rectRampPlace.GetMid().y)] * fCliffWorldScale);
            fRampRotationRight = 0.0f;
            fRampRotationLeft = 0.0f;
        }
        else if (unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE && !(unRampType & RAMP_FLAG_INVERT_INCLINE))
        {
            rectRampPlace = CRecti((iX - 1), (iY - 1), (iX + 1), (iY + 1));
            vRampLeftPos = CVec3f(rectRampPlace.left * fScale * iCliffSize, rectRampPlace.top * fScale * iCliffSize, (float)pVertCliffMap[Editor.GetWorld().GetVertCliff(rectRampPlace.GetMid().x, rectRampPlace.GetMid().y)] * fCliffWorldScale);
            vRampRightPos = CVec3f(rectRampPlace.left * fScale * iCliffSize, (rectRampPlace.bottom - 1) * fScale * iCliffSize, (float)pVertCliffMap[Editor.GetWorld().GetVertCliff(rectRampPlace.GetMid().x, rectRampPlace.GetMid().y)] * fCliffWorldScale);
            fRampRotationRight = 180.0f;
            fRampRotationLeft = 180.0f;
        }
        else if (!(unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE) && unRampType & RAMP_FLAG_INVERT_INCLINE)
        {
            rectRampPlace = CRecti((iX - 1), (iY - 1),  (iX + 1), (iY + 1));
            vRampLeftPos = CVec3f(rectRampPlace.left * fScale * iCliffSize, rectRampPlace.top * fScale * iCliffSize, (float)pVertCliffMap[Editor.GetWorld().GetVertCliff(rectRampPlace.GetMid().x, rectRampPlace.GetMid().y)] * fCliffWorldScale);
            vRampRightPos = CVec3f((rectRampPlace.right - 1) * fScale * iCliffSize, rectRampPlace.top * fScale * iCliffSize, (float)pVertCliffMap[Editor.GetWorld().GetVertCliff(rectRampPlace.GetMid().x, rectRampPlace.GetMid().y)] * fCliffWorldScale);
            fRampRotationRight = 90.0f;
            fRampRotationLeft = 90.0f;
        }
        else if (!(unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE) && !(unRampType & RAMP_FLAG_INVERT_INCLINE))
        {
            rectRampPlace = CRecti((iX - 1), (iY - 1), (iX + 1), (iY + 1));
            vRampRightPos = CVec3f((rectRampPlace.left * iCliffSize) * fScale, (rectRampPlace.top * iCliffSize) * fScale, (float)pVertCliffMap[Editor.GetWorld().GetVertCliff(rectRampPlace.GetMid().x, rectRampPlace.GetMid().y)] * fCliffWorldScale);
            vRampLeftPos = CVec3f((rectRampPlace.right - 1) * fScale * iCliffSize, rectRampPlace.top * fScale * iCliffSize, (float)pVertCliffMap[Editor.GetWorld().GetVertCliff(rectRampPlace.GetMid().x, rectRampPlace.GetMid().y)] * fCliffWorldScale);
            fRampRotationRight = 270.0f;
            fRampRotationLeft = 270.0f;
        }

        CliffDelete(rectRampPlace);

        if (bRampsLoaded)
        {
            if (unRampType & RAMP_FLAG_FIRST_INWARD)
            {
                sTopModelPath = CRampDefinition->GetRampType1().sTopPath;
                iTopRotationVertex = CRampDefinition->GetRampType1().iTopRotationVertex;
                fTopRotationAdjustment = CRampDefinition->GetRampType1().fTopDefRot;
            }
            else if (unRampType & RAMP_FLAG_FIRST_OUTWARD)
            {
                sTopModelPath = CRampDefinition->GetRampType2().sTopPath;
                iTopRotationVertex = CRampDefinition->GetRampType2().iTopRotationVertex;
                fTopRotationAdjustment = CRampDefinition->GetRampType2().fTopDefRot;
            }

            if (unRampType & RAMP_FLAG_SECOND_INWARD)
            {
                sBotModelPath = CRampDefinition->GetRampType1().sBotPath;
                iBotRotationVertex = CRampDefinition->GetRampType1().iBotRotationVertex;
                fBotRotationAdjustment = CRampDefinition->GetRampType1().fBotDefRot;
            }
            else if (unRampType & RAMP_FLAG_SECOND_OUTWARD)
            {
                sBotModelPath = CRampDefinition->GetRampType2().sBotPath;
                iBotRotationVertex = CRampDefinition->GetRampType2().iBotRotationVertex;
                fBotRotationAdjustment = CRampDefinition->GetRampType2().fBotDefRot;
            }

            //Left Ramp model
            if (IsRampFlagFirstSet(unRampType))
                RampCreate(vRampLeftPos, fRampRotationLeft + fTopRotationAdjustment, sTopModelPath, iTopRotationVertex);

            //Right ramp model
            if (IsRampFlagSecondSet(unRampType))
                RampCreate(vRampRightPos, fRampRotationRight + fBotRotationAdjustment, sBotModelPath, iBotRotationVertex);
        }

        //Terrain
        // Start Hidden Tile Edit
        CRecti rectTerrainEdit(rectRampPlace.left * iCliffSize, rectRampPlace.top * iCliffSize, rectRampPlace.right * iCliffSize, rectRampPlace.bottom * iCliffSize);

        byte*   pHiddenTileRegion = K2_NEW_ARRAY(ctx_Editor, byte, rectTerrainEdit.GetArea());

        if(!pHiddenTileRegion)
            return;

        Editor.GetWorld().GetRegion(WORLD_TILE_CLIFF_MAP, rectTerrainEdit, pHiddenTileRegion);
        
        for (int x(0); x < rectTerrainEdit.GetWidth(); x++)
        {
            for (int y(0); y < rectTerrainEdit.GetHeight(); y++)
            {
                int iCliffHide(0);

                if (!bRampsLoaded)
                {
                    iCliffHide = 0;
                }
                else if (unRampType & RAMP_FLAG_DIAGONAL && unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE && !(unRampType & RAMP_FLAG_INVERT_INCLINE))
                {
                    if (y < iCliffSize && x < iCliffSize )
                        iCliffHide = 0;
                    else if (y < iCliffSize && IsRampFlagFirstSet(unRampType))
                        iCliffHide = 1; 
                    else if (x < iCliffSize && IsRampFlagSecondSet(unRampType))
                        iCliffHide = 1; 
                }
                else if (unRampType & RAMP_FLAG_DIAGONAL && unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE && unRampType & RAMP_FLAG_INVERT_INCLINE)
                {
                    if (y > rectTerrainEdit.GetHeight() - iCliffSize - 1 && x > rectTerrainEdit.GetWidth() - iCliffSize - 1)
                        iCliffHide = 0;
                    else if (y > rectTerrainEdit.GetHeight() - iCliffSize - 1 && IsRampFlagFirstSet(unRampType))
                        iCliffHide = 1; 
                    else if (x > rectTerrainEdit.GetWidth() - iCliffSize - 1 && IsRampFlagSecondSet(unRampType))
                        iCliffHide = 1; 
                }
                else if (unRampType & RAMP_FLAG_DIAGONAL && !(unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE) && !(unRampType & RAMP_FLAG_INVERT_INCLINE))
                {
                    if (y < iCliffSize && x > rectTerrainEdit.GetWidth() - iCliffSize - 1)
                        iCliffHide = 0;
                    else if (y < iCliffSize && IsRampFlagSecondSet(unRampType))
                        iCliffHide = 1; 
                    else if (x > rectTerrainEdit.GetWidth() - iCliffSize - 1 && IsRampFlagFirstSet(unRampType))
                        iCliffHide = 1; 
                }
                else if (unRampType & RAMP_FLAG_DIAGONAL && !(unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE) && unRampType & RAMP_FLAG_INVERT_INCLINE)
                {
                    if (y > rectTerrainEdit.GetHeight() - iCliffSize - 1 && x < iCliffSize)
                        iCliffHide = 0;
                    else if (y > rectTerrainEdit.GetHeight() - iCliffSize - 1 && IsRampFlagSecondSet(unRampType))
                        iCliffHide = 1; 
                    else if (x < iCliffSize && IsRampFlagFirstSet(unRampType))
                        iCliffHide = 1; 
                }
                else if (unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE && !(unRampType & RAMP_FLAG_INVERT_INCLINE))
                {
                    if (y < iCliffSize && IsRampFlagFirstSet(unRampType))
                        iCliffHide = 1; 
                    else if (y > rectTerrainEdit.GetHeight() - iCliffSize - 1 && IsRampFlagSecondSet(unRampType))
                        iCliffHide = 1; 
                }
                else if (!(unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE) && unRampType & RAMP_FLAG_INVERT_INCLINE)
                {
                    if (x < iCliffSize && IsRampFlagFirstSet(unRampType))
                        iCliffHide = 1; 
                    else if (x > rectTerrainEdit.GetWidth() - iCliffSize - 1 && IsRampFlagSecondSet(unRampType))
                        iCliffHide = 1; 
                }
                else if (unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE && unRampType & RAMP_FLAG_INVERT_INCLINE)
                {
                    if (y < iCliffSize && IsRampFlagSecondSet(unRampType))
                        iCliffHide = 1; 
                    else if (y > rectTerrainEdit.GetHeight() - iCliffSize - 1 && IsRampFlagFirstSet(unRampType))
                        iCliffHide = 1; 
                }
                else if (!(unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE) && !(unRampType & RAMP_FLAG_INVERT_INCLINE))
                {
                    if (x < iCliffSize && IsRampFlagSecondSet(unRampType))
                        iCliffHide = 1; 
                    else if (x > rectTerrainEdit.GetWidth() - iCliffSize - 1 && IsRampFlagFirstSet(unRampType))
                        iCliffHide = 1; 
                }

                pHiddenTileRegion[x + y * rectTerrainEdit.GetWidth()] = iCliffHide;
            }
        }
                
        Editor.GetWorld().SetRegion(WORLD_TILE_CLIFF_MAP, rectTerrainEdit, pHiddenTileRegion);
        
        for (int x(0); x < rectTerrainEdit.GetWidth(); x++)
        {
            for (int y(0); y < rectTerrainEdit.GetHeight(); y++)
            {
                Vid.Notify(VID_NOTIFY_TERRAIN_SHADER_MODIFIED, x + rectTerrainEdit.left, y + rectTerrainEdit.top, 0, &Editor.GetWorld());
                Vid.Notify(VID_NOTIFY_TERRAIN_NORMAL_MODIFIED, x + rectTerrainEdit.left, y + rectTerrainEdit.top, 0, &Editor.GetWorld());
            }
        }

        if(pHiddenTileRegion)
            K2_DELETE_ARRAY(pHiddenTileRegion);
        // End Hidden Tile Edit


            // Strech to get the outer corners.
        rectTerrainEdit.Stretch(1, 1);

        // Start Height Edit
        float*  pHeightRegion = K2_NEW_ARRAY(ctx_Editor, float, rectTerrainEdit.GetArea()); 

        if (!pHeightRegion)
            return;

        Editor.GetWorld().GetRegion(WORLD_VERT_HEIGHT_MAP, rectTerrainEdit, pHeightRegion);

        int iCliffHeight(pVertCliffMap[Editor.GetWorld().GetVertCliff(rectRampPlace.GetMid().x, rectRampPlace.GetMid().y)]);

        for (int x(0); x < rectTerrainEdit.GetWidth(); x++)
        {
            for (int y(0); y < rectTerrainEdit.GetHeight(); y++)
            {
                int iRegionIndex(x + y * rectTerrainEdit.GetWidth());
                if (unRampType & RAMP_FLAG_DIAGONAL && unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE && !(unRampType & RAMP_FLAG_INVERT_INCLINE))
                {   
                    int iXpY(x + y);
                    float fRampLow(iCliffHeight * fScale * iCliffSize);
                    float fRampHigh((iCliffHeight + 1) * fScale * iCliffSize);
                    int iWidth(rectTerrainEdit.GetWidth() - 1);
                    int iHeight(rectTerrainEdit.GetHeight() - 1);
                    int iHpW(iWidth + iHeight);
                    float fDiagonalPos(((iHpW - iXpY) / 2.0f) - iCliffSize);
                    
                    if ((IsRampFlagFirstSet(unRampType) || IsRampFlagSecondSet(unRampType)) && 
                            (y <= iCliffSize && x <= iCliffSize))
                    {
                        pHeightRegion[iRegionIndex] = fRampHigh;
                    }
                    else if ((unRampType & RAMP_FLAG_EX_SECOND_DIAGONAL || unRampType & RAMP_FLAG_EX_FIRST_DIAGONAL) && 
                            (iXpY < iCliffSize * 2))
                    {
                        pHeightRegion[iRegionIndex] = fRampHigh;
                    }
                    else if (IsRampFlagFirstSet(unRampType) && y == 0 && x < iWidth - iCliffSize - 1)
                    {
                        pHeightRegion[iRegionIndex] = fRampHigh;
                    }
                    else if (IsRampFlagSecondSet(unRampType) && x == 0 && y < iHeight - iCliffSize - 1)
                    {
                        pHeightRegion[iRegionIndex] = fRampHigh;
                    }
                    else if (IsRampFlagFirstSet(unRampType) && y == 0)
                    {
                        pHeightRegion[iRegionIndex] = fRampLow;
                    }
                    else if (IsRampFlagSecondSet(unRampType) && x == 0)
                    {
                        pHeightRegion[iRegionIndex] = fRampLow;
                    }
                    else if (((iXpY > iCliffSize * 2) && (iXpY <= iHpW - iCliffSize * 2)) && 
                            (unRampType & RAMP_FLAG_EX_FIRST_DIAGONAL && unRampType & RAMP_FLAG_EX_SECOND_DIAGONAL))
                    {
                        pHeightRegion[iRegionIndex] = (fDiagonalPos * fScale) + fRampLow;
                    }
                    else if (y <= iCliffSize && unRampType & RAMP_FLAG_EX_FIRST_DIAGONAL)
                    {
                        pHeightRegion[iRegionIndex] = (fDiagonalPos * fScale) + fRampLow;
                    }
                    else if (x <= iCliffSize && unRampType & RAMP_FLAG_EX_SECOND_DIAGONAL)
                    {
                        pHeightRegion[iRegionIndex] = (fDiagonalPos * fScale) + fRampLow;
                    }
                    else if (y <= iCliffSize && IsRampFlagFirstSet(unRampType))
                    {
                        pHeightRegion[iRegionIndex] = (iWidth - x) * (fScale / 2.0f) + fRampLow;
                    }
                    else if (x <= iCliffSize && IsRampFlagSecondSet(unRampType))
                    {
                        pHeightRegion[iRegionIndex] = (iHeight - y) * (fScale / 2.0f) + fRampLow;
                    }
                    else if ((IsRampFlagSecondSet(unRampType)) && (iXpY <= iHpW - iCliffSize * 2 && iXpY > iCliffSize * 2))
                    {
                        pHeightRegion[iRegionIndex] = fDiagonalPos * fScale + fRampLow;
                    }
                    else if ((IsRampFlagFirstSet(unRampType)) && (iXpY <= iHpW - iCliffSize * 2 && iXpY > iCliffSize * 2))
                    {
                        pHeightRegion[iRegionIndex] = fDiagonalPos * fScale + fRampLow;
                    }
                    else    
                    {
                        pHeightRegion[iRegionIndex] = fRampLow;
                    }

                }
                else if (unRampType & RAMP_FLAG_DIAGONAL && unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE && unRampType & RAMP_FLAG_INVERT_INCLINE)
                {
                    int iXpY(x + y);
                    float fDiagonalPos((iXpY / 2.0f) - iCliffSize);
                    float fRampLow(iCliffHeight * fScale * iCliffSize);
                    float fRampHigh((iCliffHeight + 1) * fScale * iCliffSize);
                    int iWidth(rectTerrainEdit.GetWidth() - 1);
                    int iHeight(rectTerrainEdit.GetHeight() - 1);
                    int iHpW(iWidth + iHeight);
                    
                    if ((IsRampFlagFirstSet(unRampType) || IsRampFlagSecondSet(unRampType)) && 
                            (y >= iHeight - iCliffSize && x >= iWidth - iCliffSize))
                    {
                        pHeightRegion[iRegionIndex] = fRampHigh;
                    }
                    else if ((unRampType & RAMP_FLAG_EX_SECOND_DIAGONAL || unRampType & RAMP_FLAG_EX_FIRST_DIAGONAL) && 
                            (iXpY > iHpW - iCliffSize * 2))
                    {
                        pHeightRegion[iRegionIndex] = fRampHigh;
                    }
                    else if (IsRampFlagFirstSet(unRampType) && y == iHeight && x >= iCliffSize + 2)
                    {
                        pHeightRegion[iRegionIndex] = fRampHigh;
                    }
                    else if (IsRampFlagSecondSet(unRampType) && x == iWidth && y >= iCliffSize + 2)
                    {
                        pHeightRegion[iRegionIndex] = fRampHigh;
                    }
                    else if (IsRampFlagFirstSet(unRampType) && y == iHeight)
                    {
                        pHeightRegion[iRegionIndex] = fRampLow;
                    }
                    else if (IsRampFlagSecondSet(unRampType) && x == iWidth)
                    {
                        pHeightRegion[iRegionIndex] = fRampLow;
                    }
                    else if (((iXpY <= iHpW - iCliffSize * 2) && (iXpY > iCliffSize * 2)) && 
                            (unRampType & RAMP_FLAG_EX_FIRST_DIAGONAL && unRampType & RAMP_FLAG_EX_SECOND_DIAGONAL))
                    {
                        pHeightRegion[iRegionIndex] = (fDiagonalPos * fScale) + fRampLow;
                    }
                    else if (y >= iHeight - iCliffSize && unRampType & RAMP_FLAG_EX_FIRST_DIAGONAL)
                    {
                        pHeightRegion[iRegionIndex] = (fDiagonalPos * fScale) + fRampLow;
                    }
                    else if (x >= iWidth - iCliffSize && unRampType & RAMP_FLAG_EX_SECOND_DIAGONAL)
                    {
                        pHeightRegion[iRegionIndex] = (fDiagonalPos * fScale) + fRampLow;
                    }
                    else if (y >= iHeight - iCliffSize && IsRampFlagFirstSet(unRampType))
                    {
                        pHeightRegion[iRegionIndex] = x * (fScale / 2.0f) + fRampLow;
                    }
                    else if (x >= iWidth - iCliffSize && IsRampFlagSecondSet(unRampType))
                    {
                        pHeightRegion[iRegionIndex] = y * (fScale / 2.0f) + fRampLow;
                    }
                    else if ((IsRampFlagSecondSet(unRampType)) && (iXpY > iCliffSize * 2 && iXpY <= iHpW - iCliffSize * 2))
                    {
                        pHeightRegion[iRegionIndex] = fDiagonalPos * fScale + fRampLow;
                    }
                    else if ((IsRampFlagFirstSet(unRampType)) && (iXpY > iCliffSize * 2 && iXpY <= iHpW - iCliffSize * 2))
                    {
                        pHeightRegion[iRegionIndex] = fDiagonalPos * fScale + fRampLow;
                    }
                    else    
                    {
                        pHeightRegion[iRegionIndex] = fRampLow;
                    }

                }
                else if (unRampType & RAMP_FLAG_DIAGONAL && !(unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE) && !(unRampType & RAMP_FLAG_INVERT_INCLINE))
                {   
                    int iWidth(rectTerrainEdit.GetWidth() - 1);
                    int iHeight(rectTerrainEdit.GetHeight() - 1);
                    int iXpY(x + (iHeight - y));
                    float fRampLow(iCliffHeight * fScale * iCliffSize);
                    float fRampHigh((iCliffHeight + 1) * fScale * iCliffSize);
                    int iHpW(iWidth + iHeight);
                    float fDiagonalPos((iXpY / 2.0f) - iCliffSize);
                    
                    if ((IsRampFlagFirstSet(unRampType) || IsRampFlagSecondSet(unRampType)) && 
                            (x >= iHeight - iCliffSize && y <= iCliffSize))
                    {
                        pHeightRegion[iRegionIndex] = fRampHigh;
                    }
                    else if ((unRampType & RAMP_FLAG_EX_SECOND_DIAGONAL || unRampType & RAMP_FLAG_EX_FIRST_DIAGONAL) && 
                            (iXpY > iHpW - iCliffSize * 2))
                    {
                        pHeightRegion[iRegionIndex] = fRampHigh;
                    }
                    else if (IsRampFlagFirstSet(unRampType) && x == iWidth && y <= iHeight - iCliffSize - 2)
                    {
                        pHeightRegion[iRegionIndex] = fRampHigh;
                    }
                    else if (IsRampFlagSecondSet(unRampType) && y == 0 && x >= iCliffSize + 2)
                    {
                        pHeightRegion[iRegionIndex] = fRampHigh;
                    }
                    else if (IsRampFlagFirstSet(unRampType) && x == iWidth)
                    {
                        pHeightRegion[iRegionIndex] = fRampLow;
                    }
                    else if (IsRampFlagSecondSet(unRampType) && y == 0)
                    {
                        pHeightRegion[iRegionIndex] = fRampLow;
                    }
                    else if (((iXpY > iCliffSize * 2) && (iXpY <= iHpW - iCliffSize * 2)) && 
                            (unRampType & RAMP_FLAG_EX_FIRST_DIAGONAL && unRampType & RAMP_FLAG_EX_SECOND_DIAGONAL))
                    {
                        pHeightRegion[iRegionIndex] = (fDiagonalPos * fScale) + fRampLow;
                    }
                    else if (y <= iCliffSize && unRampType & RAMP_FLAG_EX_SECOND_DIAGONAL)
                    {
                        pHeightRegion[iRegionIndex] = (fDiagonalPos * fScale) + fRampLow;
                    }
                    else if (x >= iWidth - iCliffSize && unRampType & RAMP_FLAG_EX_FIRST_DIAGONAL)
                    {
                        pHeightRegion[iRegionIndex] = (fDiagonalPos * fScale) + fRampLow;
                    }
                    else if (x >= iWidth - iCliffSize && IsRampFlagFirstSet(unRampType))
                    {
                        pHeightRegion[iRegionIndex] = (iHeight - y) * (fScale / 2.0f) + fRampLow;
                    }
                    else if (y <= iCliffSize && IsRampFlagSecondSet(unRampType))
                    {
                        pHeightRegion[iRegionIndex] = x * (fScale / 2.0f) + fRampLow;
                    }
                    else if ((IsRampFlagFirstSet(unRampType)) && (iXpY < iHpW - iCliffSize * 2 && iXpY > iCliffSize * 2))
                    {
                        pHeightRegion[iRegionIndex] = fDiagonalPos * fScale + fRampLow;
                    }
                    else if ((IsRampFlagSecondSet(unRampType)) && (iXpY < iHpW - iCliffSize * 2 && iXpY > iCliffSize * 2))
                    {
                        pHeightRegion[iRegionIndex] = fDiagonalPos * fScale + fRampLow;
                    }
                    else    
                    {
                        pHeightRegion[iRegionIndex] = fRampLow;
                    }

                }
                else if (unRampType & RAMP_FLAG_DIAGONAL && !(unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE) && unRampType & RAMP_FLAG_INVERT_INCLINE)
                {
                    int iWidth(rectTerrainEdit.GetWidth() - 1);
                    int iHeight(rectTerrainEdit.GetHeight() - 1);
                    int iXpY((iWidth - x) + y);
                    float fRampLow(iCliffHeight * fScale * iCliffSize);
                    float fRampHigh((iCliffHeight + 1) * fScale * iCliffSize);
                    int iHpW(iWidth + iHeight);
                    float fDiagonalPos((iXpY / 2.0f) - iCliffSize);
                    
                    if ((IsRampFlagFirstSet(unRampType) || IsRampFlagSecondSet(unRampType)) && 
                            (x <= iCliffSize && y >= iWidth - iCliffSize))
                    {
                        pHeightRegion[iRegionIndex] = fRampHigh;
                    }
                    else if ((unRampType & RAMP_FLAG_EX_SECOND_DIAGONAL || unRampType & RAMP_FLAG_EX_FIRST_DIAGONAL) && 
                            (iXpY > iHpW - iCliffSize * 2))
                    {
                        pHeightRegion[iRegionIndex] = fRampHigh;
                    }
                    else if (IsRampFlagFirstSet(unRampType) && x == 0 && y >= iCliffSize + 2)
                    {
                        pHeightRegion[iRegionIndex] = fRampHigh;
                    }
                    else if (IsRampFlagSecondSet(unRampType) && y == iHeight && x <= iWidth - iCliffSize - 2)
                    {
                        pHeightRegion[iRegionIndex] = fRampHigh;
                    }
                    else if (IsRampFlagFirstSet(unRampType) && x == 0)
                    {
                        pHeightRegion[iRegionIndex] = fRampLow;
                    }
                    else if (IsRampFlagSecondSet(unRampType) && y == iHeight)
                    {
                        pHeightRegion[iRegionIndex] = fRampLow;
                    }
                    else if (((iXpY > iCliffSize * 2) && (iXpY <= iHpW - iCliffSize * 2)) && 
                            (unRampType & RAMP_FLAG_EX_FIRST_DIAGONAL && unRampType & RAMP_FLAG_EX_SECOND_DIAGONAL))
                    {
                        pHeightRegion[iRegionIndex] = (fDiagonalPos * fScale) + fRampLow;
                    }
                    else if (y >= iHeight - iCliffSize && unRampType & RAMP_FLAG_EX_SECOND_DIAGONAL)
                    {
                        pHeightRegion[iRegionIndex] = (fDiagonalPos * fScale) + fRampLow;
                    }
                    else if (x <= iCliffSize && unRampType & RAMP_FLAG_EX_FIRST_DIAGONAL)
                    {
                        pHeightRegion[iRegionIndex] = (fDiagonalPos * fScale) + fRampLow;
                    }
                    else if (x <= iCliffSize && IsRampFlagFirstSet(unRampType))
                    {
                        pHeightRegion[iRegionIndex] = y * (fScale / 2.0f) + fRampLow;
                    }
                    else if (y >= iHeight - iCliffSize && IsRampFlagSecondSet(unRampType))
                    {
                        pHeightRegion[iRegionIndex] = (iWidth - x) * (fScale / 2.0f) + fRampLow;
                    }
                    else if ((IsRampFlagFirstSet(unRampType)) && (iXpY < iHpW - iCliffSize * 2 && iXpY > iCliffSize * 2))
                    {
                        pHeightRegion[iRegionIndex] = fDiagonalPos * fScale + fRampLow;
                    }
                    else if ((IsRampFlagSecondSet(unRampType)) && (iXpY < iHpW - iCliffSize * 2 && iXpY > iCliffSize * 2))
                    {
                        pHeightRegion[iRegionIndex] = fDiagonalPos * fScale + fRampLow;
                    }
                    else    
                    {
                        pHeightRegion[iRegionIndex] = fRampLow;
                    }
                }

                else if (unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE && unRampType & RAMP_FLAG_INVERT_INCLINE)
                {
                    float fCliffExtra(0.0f);
                    if ((y < iCliffSize && IsRampFlagSecondSet(unRampType)) || 
                        (y > rectTerrainEdit.GetHeight() - 1 - iCliffSize && IsRampFlagFirstSet(unRampType)))
                        fCliffExtra = fScale * 0.5f; 

                    if (x == rectTerrainEdit.GetWidth() - 1 && !((unRampType & RAMP_FLAG_SECOND_OUTWARD && y < iCliffSize / 2) || (unRampType & RAMP_FLAG_FIRST_OUTWARD && y > rectTerrainEdit.GetWidth() - 1 - iCliffSize / 2)))
                        pHeightRegion[x + y * rectTerrainEdit.GetWidth()] = (iCliffHeight + 1) * fScale * iCliffSize;
                    else if ((((x < iCliffSize + 1 && (y < 2 || y > rectTerrainEdit.GetHeight() - 3)) || x == 0) && fCliffExtra != 0.0f) || 
                        (unRampType & RAMP_FLAG_SECOND_OUTWARD && y < iCliffSize / 2) || (unRampType & RAMP_FLAG_FIRST_OUTWARD && y > rectTerrainEdit.GetWidth() - 1 - iCliffSize / 2))
                        pHeightRegion[x + y * rectTerrainEdit.GetWidth()] = iCliffHeight * fScale * iCliffSize;
                    else
                        pHeightRegion[x + y * rectTerrainEdit.GetWidth()] = x * (fScale / 2.0f) + (iCliffHeight * fScale * iCliffSize) + fCliffExtra;
                }
                else if (!(unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE) && unRampType & RAMP_FLAG_INVERT_INCLINE)
                {
                    float fCliffExtra(0.0f);
                    if ((x < iCliffSize && IsRampFlagFirstSet(unRampType)) || 
                        (x > rectTerrainEdit.GetWidth() - 1 - iCliffSize && IsRampFlagSecondSet(unRampType)))
                        fCliffExtra = fScale * 0.5f;

                    if (y == rectTerrainEdit.GetHeight() - 1 && !((unRampType & RAMP_FLAG_FIRST_OUTWARD && x < iCliffSize / 2) || (unRampType & RAMP_FLAG_SECOND_OUTWARD && x > rectTerrainEdit.GetHeight() - 1 - iCliffSize / 2)))
                        pHeightRegion[x + y * rectTerrainEdit.GetWidth()] = (iCliffHeight + 1) * fScale * iCliffSize;
                    else if (((y < iCliffSize + 1 && (x < 2 || x > rectTerrainEdit.GetWidth() - 3)) || y == 0) && fCliffExtra != 0.0f || 
                        (unRampType & RAMP_FLAG_FIRST_OUTWARD && x < iCliffSize / 2) || (unRampType & RAMP_FLAG_SECOND_OUTWARD && x > rectTerrainEdit.GetHeight() - 1 - iCliffSize / 2))
                        pHeightRegion[x + y * rectTerrainEdit.GetWidth()] = iCliffHeight * fScale * iCliffSize;
                    else
                        pHeightRegion[x + y * rectTerrainEdit.GetWidth()] = y * (fScale / 2.0f) + (iCliffHeight * fScale * iCliffSize) + fCliffExtra;
                }
                else if (unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE && !(unRampType & RAMP_FLAG_INVERT_INCLINE))
                {
                    float fCliffExtra(0.0f);
                    if ((y < iCliffSize && IsRampFlagFirstSet(unRampType)) || 
                        (y > rectTerrainEdit.GetHeight() - 1 - iCliffSize && IsRampFlagSecondSet(unRampType)))
                        fCliffExtra = fScale * 0.5f;

                    if (x == 0 && !((unRampType & RAMP_FLAG_FIRST_OUTWARD && y < iCliffSize / 2) || (unRampType & RAMP_FLAG_SECOND_OUTWARD && y > rectTerrainEdit.GetWidth() - 1 - iCliffSize / 2)))
                        pHeightRegion[x + y * rectTerrainEdit.GetWidth()] = (rectTerrainEdit.GetWidth() - 1 - x) * (fScale / 2.0f) + (iCliffHeight * fScale * iCliffSize);
                    else if ((x > iCliffSize - 1 && (y < 2 || y > rectTerrainEdit.GetHeight() - 3) || x == rectTerrainEdit.GetWidth() - 1) && fCliffExtra != 0.0f || 
                        (unRampType & RAMP_FLAG_FIRST_OUTWARD && y < iCliffSize / 2) || (unRampType & RAMP_FLAG_SECOND_OUTWARD && y > rectTerrainEdit.GetWidth() - 1 - iCliffSize / 2))
                        pHeightRegion[x + y * rectTerrainEdit.GetWidth()] = iCliffHeight * fScale * iCliffSize;
                    else
                        pHeightRegion[x + y * rectTerrainEdit.GetWidth()] = (rectTerrainEdit.GetWidth() - 1 - x) * (fScale / 2.0f) + (iCliffHeight * fScale * iCliffSize) + fCliffExtra;
                }
                else if (!(unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE) && !(unRampType & RAMP_FLAG_INVERT_INCLINE))
                {
                    float fCliffExtra(0.0f);
                    if ((x < iCliffSize && IsRampFlagSecondSet(unRampType)) || 
                        (x > rectTerrainEdit.GetWidth() - 1 - iCliffSize && IsRampFlagFirstSet(unRampType)))
                        fCliffExtra = fScale * 0.5f; 

                    if (y == 0 && !((unRampType & RAMP_FLAG_SECOND_OUTWARD && x < iCliffSize / 2) || (unRampType & RAMP_FLAG_FIRST_OUTWARD && x > rectTerrainEdit.GetHeight() - 1 - iCliffSize / 2)))
                        pHeightRegion[x + y * rectTerrainEdit.GetWidth()] = (rectTerrainEdit.GetHeight() - 1 - y) * (fScale / 2.0f) + (iCliffHeight * fScale * iCliffSize);
                    else if ((y > iCliffSize - 1 && (x < 2 || x > rectTerrainEdit.GetWidth() - 3) || y == rectTerrainEdit.GetHeight() - 1) && fCliffExtra != 0.0f || 
                        (unRampType & RAMP_FLAG_SECOND_OUTWARD && x < iCliffSize / 2) || (unRampType & RAMP_FLAG_FIRST_OUTWARD && x > rectTerrainEdit.GetHeight() - 1 - iCliffSize / 2))
                        pHeightRegion[x + y * rectTerrainEdit.GetWidth()] = iCliffHeight * fScale * iCliffSize;
                    else
                        pHeightRegion[x + y * rectTerrainEdit.GetWidth()] = (rectTerrainEdit.GetHeight() - 1 - y) * (fScale / 2.0f) + (iCliffHeight * fScale * iCliffSize) + fCliffExtra;
                }
            }
        }

        Editor.GetWorld().SetRegion(WORLD_VERT_HEIGHT_MAP, rectTerrainEdit, pHeightRegion);

        for (int x(0); x < rectTerrainEdit.GetWidth(); x++)
        {
            for (int y(0); y < rectTerrainEdit.GetHeight(); y++)
            {
                Vid.Notify(VID_NOTIFY_TERRAIN_VERTEX_MODIFIED, x + rectTerrainEdit.left, y + rectTerrainEdit.top, 0, &Editor.GetWorld());
            }
        }

        if(pHeightRegion)
            K2_DELETE_ARRAY(pHeightRegion);
        // End Hight Edit

        Editor.GetWorld().SetRampTile(iX, iY, unRampType);

            // Set the Ramp Tiles left and right to reflect the current set one.
        ushort unRampTileLeft(0);
        ushort unRampTileRight(0);
        if (unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE && unRampType & RAMP_FLAG_INVERT_INCLINE)
        {
            unRampTileLeft = Editor.GetWorld().GetRampTile(iX, iY + 1);
            unRampTileRight = Editor.GetWorld().GetRampTile(iX, iY - 1);

            if (unRampTileLeft & RAMP_FLAG_SET)
            {
                if ((unRampType & RAMP_FLAG_FIRST_INWARD))
                    unRampTileLeft |= RAMP_FLAG_SECOND_INWARD;
                else
                    unRampTileLeft &= ~RAMP_FLAG_SECOND_INWARD;
                Editor.GetWorld().SetRampTile(iX, iY + 1, unRampTileLeft);
            }

            if (unRampTileRight & RAMP_FLAG_SET)
            {
                if ((unRampType & RAMP_FLAG_SECOND_INWARD)) 
                    unRampTileRight |= RAMP_FLAG_FIRST_INWARD;
                else
                    unRampTileRight &= ~RAMP_FLAG_FIRST_INWARD;
                Editor.GetWorld().SetRampTile(iX, iY - 1, unRampTileRight);
            }
        }
        else if (!(unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE) && !(unRampType & RAMP_FLAG_INVERT_INCLINE))
        {
            unRampTileLeft = Editor.GetWorld().GetRampTile(iX + 1, iY);
            unRampTileRight = Editor.GetWorld().GetRampTile(iX - 1, iY);

            if (unRampTileLeft & RAMP_FLAG_SET)
            {
                if ((unRampType & RAMP_FLAG_FIRST_INWARD))
                    unRampTileLeft |= RAMP_FLAG_SECOND_INWARD;
                else
                    unRampTileLeft &= ~RAMP_FLAG_SECOND_INWARD;
                Editor.GetWorld().SetRampTile(iX + 1, iY, unRampTileLeft);
            }

            if (unRampTileRight & RAMP_FLAG_SET)
            {
                if ((unRampType & RAMP_FLAG_SECOND_INWARD)) 
                    unRampTileRight |= RAMP_FLAG_FIRST_INWARD;
                else
                    unRampTileRight &= ~RAMP_FLAG_FIRST_INWARD;
                Editor.GetWorld().SetRampTile(iX - 1, iY, unRampTileRight);
            }
        }
        else if (unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE && !(unRampType & RAMP_FLAG_INVERT_INCLINE))
        {
            unRampTileLeft = Editor.GetWorld().GetRampTile(iX, iY - 1);
            unRampTileRight = Editor.GetWorld().GetRampTile(iX, iY + 1);

            if (unRampTileLeft & RAMP_FLAG_SET)
            {
                if ((unRampType & RAMP_FLAG_FIRST_INWARD))
                    unRampTileLeft |= RAMP_FLAG_SECOND_INWARD;
                else
                    unRampTileLeft &= ~RAMP_FLAG_SECOND_INWARD;
                Editor.GetWorld().SetRampTile(iX, iY - 1, unRampTileLeft);
            }

            if (unRampTileRight & RAMP_FLAG_SET)
            {
                if ((unRampType & RAMP_FLAG_SECOND_INWARD)) 
                    unRampTileRight |= RAMP_FLAG_FIRST_INWARD;
                else
                    unRampTileRight &= ~RAMP_FLAG_FIRST_INWARD;
                Editor.GetWorld().SetRampTile(iX, iY + 1, unRampTileRight);
            }
        }
        else if (!(unRampType & RAMP_FLAG_LEFT_RIGHT_INCLINE) && unRampType & RAMP_FLAG_INVERT_INCLINE)
        {
            unRampTileLeft = Editor.GetWorld().GetRampTile(iX - 1, iY);
            unRampTileRight = Editor.GetWorld().GetRampTile(iX + 1, iY);

            if (unRampTileLeft & RAMP_FLAG_SET)
            {
                if ((unRampType & RAMP_FLAG_FIRST_INWARD))
                    unRampTileLeft |= RAMP_FLAG_SECOND_INWARD;
                else
                    unRampTileLeft &= ~RAMP_FLAG_SECOND_INWARD;
                Editor.GetWorld().SetRampTile(iX - 1, iY, unRampTileLeft);
            }

            if (unRampTileRight & RAMP_FLAG_SET)
            {
                if ((unRampType & RAMP_FLAG_SECOND_INWARD)) 
                    unRampTileRight |= RAMP_FLAG_FIRST_INWARD;
                else
                    unRampTileRight &= ~RAMP_FLAG_FIRST_INWARD;
                Editor.GetWorld().SetRampTile(iX + 1, iY, unRampTileRight);
            }
        }

        CalculateCliffBlockers(rectRampPlace);
    }
}


/*====================
  CCliffTool::CliffDelete
  ====================*/
void CCliffTool::CliffDelete(int iX, int iY)
{
    //Delete the old piece
    uivector vresult;
    int         iCliffWorldSize(Editor.GetWorld().GetCliffSize() * Editor.GetWorld().GetScale());
    CBBoxf      box(CBBoxf(CVec3f(iX * iCliffWorldSize + 1, iY * iCliffWorldSize + 1, -FAR_AWAY),
                           CVec3f(iX * iCliffWorldSize + 1, iY * iCliffWorldSize + 1, FAR_AWAY)));

    Editor.GetWorld().GetEntitiesInRegion(vresult, box, 0);
    
    for (uivector::iterator it = vresult.begin(); it < vresult.end(); it++)
    {
        if (Editor.GetWorld().GetEntity(*it)->GetType() == _T("Prop_Cliff") || Editor.GetWorld().GetEntity(*it)->GetType() == _T("Prop_Cliff2"))
        {
            Editor.GetWorld().UnlinkEntity(*it);
            Editor.GetWorld().DeleteEntity(*it);
        }
    }
    
    vresult.clear();
}


/*====================
  CCliffTool::CliffDelete
  ====================*/
void CCliffTool::CliffDelete(CRecti rectArea)
{
    rectArea.Stretch(-2, -2);
    rectArea.Shift(1, 1);

    //Delete the old piece
    float       fWorldScale(Editor.GetWorld().GetScale());
    float       fWorldCliffScale(fWorldScale * Editor.GetWorld().GetCliffSize());
    uivector    vresult;
    CVec3f      vMin(rectArea.left * fWorldCliffScale - fWorldScale, rectArea.top * fWorldCliffScale - fWorldScale, -FAR_AWAY);
    CVec3f      vMax(rectArea.right * fWorldCliffScale + fWorldScale, rectArea.bottom * fWorldCliffScale + fWorldScale, FAR_AWAY);
    CBBoxf      box(CBBoxf(vMin, vMax));

    Editor.GetWorld().GetEntitiesInRegion(vresult, box, 0);

    for (uivector::iterator it = vresult.begin(); it < vresult.end(); it++)
    {
        if (Editor.GetWorld().GetEntity(*it)->GetType() == _T("Prop_Cliff") || Editor.GetWorld().GetEntity(*it)->GetType() == _T("Prop_Cliff2"))
        {
            Editor.GetWorld().UnlinkEntity(*it);
            Editor.GetWorld().DeleteEntity(*it);
        }
    }

    vresult.clear();
}


/*====================
  CCliffTool::CliffCreate
  ====================*/
void    CCliffTool::CliffCreate(CVec3f vCliffPos, float fRotation, tstring tModel, int iRotationVertex)
{
    fRotation = fabs(fmod(fRotation, 360.0f));

    try
    {
        uint uiNewEntity(Editor.GetWorld().AllocateNewEntity());
        CWorldEntity *pNewEntity(Editor.GetWorld().GetEntity(uiNewEntity, true));

        pNewEntity->SetAngles(CVec3f(0.0f, 0.0f, fRotation));
        pNewEntity->SetPosition(vCliffPos);
        pNewEntity->SetModelHandle(g_ResourceManager.Register(tModel, RES_MODEL));
        pNewEntity->SetModelPath(tModel);
        pNewEntity->SetType(_T("Prop_Cliff"));
        pNewEntity->SetScale(Editor.GetWorld().GetScale() / 32.0f);
        pNewEntity->SetSkin(g_ResourceManager.GetSkin(pNewEntity->GetModelHandle(), _T("Default")));
        
        //adjust location to compensate for yaw rotation
        RotationAdjustment(uiNewEntity, iRotationVertex);

        Editor.GetWorld().LinkEntity(uiNewEntity, LINK_SURFACE | LINK_MODEL, SURF_PROP);
    }
    catch (CException &ex)
    {
        ex.Process(_T("  CCliffTool::CliffCreate() - "), NO_THROW);
    }
}


/*====================
  CCliffTool::RampCreate
  ====================*/
void    CCliffTool::RampCreate(CVec3f vRampPos, float fRotation, tstring tModel, int iRotationVertex)
{
    fRotation = fabs(fmod(fRotation, 360.0f));

    try
    {
        uint uiNewEntity(Editor.GetWorld().AllocateNewEntity());
        CWorldEntity *pNewEntity(Editor.GetWorld().GetEntity(uiNewEntity, true));

        pNewEntity->SetAngles(CVec3f(0.0f, 0.0f, fRotation));
        pNewEntity->SetPosition(vRampPos);
        pNewEntity->SetModelHandle(g_ResourceManager.Register(tModel, RES_MODEL));
        pNewEntity->SetModelPath(tModel);
        pNewEntity->SetType(_T("Prop_Cliff2"));
        pNewEntity->SetScale(Editor.GetWorld().GetScale() / 32.0f);
        pNewEntity->SetSkin(g_ResourceManager.GetSkin(pNewEntity->GetModelHandle(), _T("Default")));
        
        //adjust location to compensate for yaw rotation
        RotationAdjustment(uiNewEntity, iRotationVertex);

        Editor.GetWorld().LinkEntity(uiNewEntity, LINK_SURFACE | LINK_MODEL, SURF_PROP);

    }
    catch (CException &ex)
    {
        ex.Process(_T("  CCliffTool::RampCreate() - "), NO_THROW);
    }
}


/*====================
  CCliffTool::CalculateHeightVertex
  ====================*/
float   CCliffTool::CalculateHeightVertex(int iX, int iY)
{
    //Get Cliff Map
    int* pVertCliffMap = Editor.GetWorld().GetVertCliffMap();

    int iCliffSize = Editor.GetWorld().GetCliffSize();

    int iXC(-1);
    int iXC2(-1);
    int iYC(-1);
    int iYC2(-1);

    if (iX % iCliffSize < iCliffSize / 2)
    {
        iXC = iX / iCliffSize;
    }
    if (iX % iCliffSize == iCliffSize / 2)
    {
        iXC = iX / iCliffSize;
        iXC2 = iX / iCliffSize + 1;
    }
    if (iX % iCliffSize > iCliffSize / 2)
    {
        iXC = iX / iCliffSize + 1;
    }
    if (iY % iCliffSize < iCliffSize / 2)
    {
        iYC = iY / iCliffSize;
    }
    if (iY % iCliffSize == iCliffSize / 2)
    {
        iYC = iY / iCliffSize;
        iYC2 = iY / iCliffSize + 1;
    }
    if (iY % iCliffSize > iCliffSize / 2)
    {
        iYC = iY / iCliffSize + 1;
    }

    int iCliffHeight = pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC,iYC)];

    if (iYC2 != -1 && iXC2 != -1 && pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC2,iYC2)] > iCliffHeight)
        iCliffHeight = pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC2,iYC2)];

    if (iXC2 != -1 && pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC2,iYC)] > iCliffHeight)
        iCliffHeight = pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC2,iYC)];

    if (iYC2 != -1 && pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC,iYC2)] > iCliffHeight)
        iCliffHeight = pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC,iYC2)];

    return iCliffHeight * Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
}


/*====================
  CCliffTool::CalculateCliffBlockers
  ====================*/
void    CCliffTool::CalculateCliffBlockers(CRecti scanArea)
{
    scanArea.Stretch(4, 4);
    scanArea.Shift(-2, -2);
    scanArea.right = (scanArea.right * Editor.GetWorld().GetCliffSize());
    scanArea.left = (scanArea.left * Editor.GetWorld().GetCliffSize());
    scanArea.bottom = (scanArea.bottom * Editor.GetWorld().GetCliffSize());
    scanArea.top = (scanArea.top * Editor.GetWorld().GetCliffSize());

    ClampRectToGrid(&scanArea);
    
    byte *pRegion = K2_NEW_ARRAY(ctx_Editor, byte, scanArea.GetArea());

    uint uiAreaX(0);
    uint uiAreaY(0);
    for(int x(scanArea.left); x < scanArea.left + scanArea.GetWidth(); ++x)
    {
        for(int y(scanArea.top); y < scanArea.top + scanArea.GetHeight(); ++y)
        {
            uiAreaY = (y - scanArea.top) * scanArea.GetWidth();
            pRegion[uiAreaX + uiAreaY] = CanBlockVert(x, y);
        }
        ++uiAreaX;
    }

    Editor.GetWorld().SetRegion(WORLD_VERT_BLOCKER_MAP, scanArea, pRegion);

    if (pRegion)
    {   K2_DELETE_ARRAY(pRegion); pRegion = 0;  }
}


/*====================
  CCliffTool::GetCliffVertHeight
  ====================*/
int     CCliffTool::GetCliffVertHeight(int iXC, int iYC)
{   
    int* pVertCliffMap = Editor.GetWorld().GetVertCliffMap();
    return pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC, iYC)];
}


/*====================
  CCliffTool::CanPlaceRamp
  ====================*/
ushort  CCliffTool::CanPlaceRamp(int iX, int iY)
{
    if (iX < 1 || iY < 1 || iX > Editor.GetWorld().GetVertCliffMapWidth() - 2 || iY > Editor.GetWorld().GetVertCliffMapWidth() - 2)
        return 0;

    int*    pVertCliffMap(Editor.GetWorld().GetVertCliffMap());
    int     iBaseHeight(pVertCliffMap[Editor.GetWorld().GetVertCliff(iX, iY)]);
    bool    bFront(true);
    bool    bBack(true);
    bool    bLeft(true);
    bool    bRight(true);
    bool    bTopLeft(true);
    bool    bTopRight(true);
    bool    bBottomLeft(true);
    bool    bBottomRight(true);
    bool    bDiagonalFront(true);
    bool    bDiagonalBack(true);
    bool    bDiagonalLeft(true);
    bool    bDiagonalRight(true);
    bool    bFlat(true);
    ushort  unRampTileFlags(Editor.GetWorld().GetRampTile(iX, iY));
    ushort  unRemoveFlag(0);

    if (unRampTileFlags & RAMP_FLAG_SET)
        unRemoveFlag |= RAMP_FLAG_EX_REMOVABLE;

    for(int iYTest(iY - 2); iYTest <= iY + 2; ++iYTest)
    {
        for(int iXTest(iX - 2); iXTest <= iX + 2; ++iXTest)
        {
            if (iXTest < 1 || iYTest < 1 || iXTest > Editor.GetWorld().GetVertCliffMapHeight() - 2 || iYTest > Editor.GetWorld().GetVertCliffMapHeight() - 2)
            {
                bFront = false;
                bBack = false;
                bLeft = false;
                bRight = false;
                bDiagonalFront = false;
                bDiagonalBack = false;
                bDiagonalLeft = false;
                bDiagonalRight = false;
                break;
            }

            if (iXTest == iX - 2)
            {
                if (iYTest == iY - 2)   
                {
                    bool bRampSet(Editor.GetWorld().GetRampTile(iXTest, iYTest) & RAMP_FLAG_SET);
                    if (bRampSet)
                    {
                        bDiagonalBack = false;
                    }
                }
                else if (iYTest == iY - 1)  
                {
                    int iTileHeight(pVertCliffMap[Editor.GetWorld().GetVertCliff(iXTest, iYTest)]);
                    byte yRampFlags(Editor.GetWorld().GetRampTile(iXTest, iYTest));
                    if (iTileHeight != iBaseHeight || yRampFlags & RAMP_FLAG_SET)
                    {
                        bDiagonalLeft = false;
                        bDiagonalBack = false;
                    }
                    if (yRampFlags & RAMP_FLAG_SET && yRampFlags & RAMP_FLAG_DIAGONAL)
                    {
                        bRight = false;
                        bBack = false;
                    }
                }
                else if (iYTest == iY)  
                {
                    byte yRampFlags(Editor.GetWorld().GetRampTile(iXTest, iYTest));
                    if (yRampFlags & RAMP_FLAG_SET && yRampFlags & RAMP_FLAG_DIAGONAL)
                    {
                        bRight = false;
                    }
                    if (yRampFlags & RAMP_FLAG_SET)
                    {
                        bDiagonalLeft = false;
                        bDiagonalBack = false;
                    }
                }
                else if (iYTest == iY + 1)  
                {
                    int iTileHeight(pVertCliffMap[Editor.GetWorld().GetVertCliff(iXTest, iYTest)]);
                    byte yRampFlags(Editor.GetWorld().GetRampTile(iXTest, iYTest));
                    if (iTileHeight != iBaseHeight || yRampFlags & RAMP_FLAG_SET)
                    {
                        bDiagonalBack = false;
                        bDiagonalLeft = false;
                    }
                    if (yRampFlags & RAMP_FLAG_SET && yRampFlags & RAMP_FLAG_DIAGONAL)
                    {
                        bRight = false;
                        bFront = false;
                    }
                }
                else if (iYTest == iY + 2)  
                {
                    bool bRampSet(Editor.GetWorld().GetRampTile(iXTest, iYTest) & RAMP_FLAG_SET);
                    if (bRampSet)
                    {
                        bDiagonalLeft = false;
                    }
                }
            }
            else if (iXTest == iX - 1)
            {
                if (iYTest == iY - 2)   
                {
                    int iTileHeight(pVertCliffMap[Editor.GetWorld().GetVertCliff(iXTest, iYTest)]);
                    byte yRampFlags(Editor.GetWorld().GetRampTile(iXTest, iYTest));
                    if (iTileHeight != iBaseHeight || yRampFlags & RAMP_FLAG_SET)
                    {
                        bDiagonalRight = false;
                        bDiagonalBack = false;
                    }
                    if (yRampFlags & RAMP_FLAG_SET && yRampFlags & RAMP_FLAG_DIAGONAL)
                    {
                        bBack = false;
                        bRight = false;
                    }
                }
                else if (iYTest == iY - 1)  // TOP LEFT
                {
                    if (Editor.GetWorld().GetRampTile(iXTest, iYTest) & RAMP_FLAG_SET)
                    {
                        bFront = false;
                        bBack = false;
                        bLeft = false;
                        bRight = false;
                        bDiagonalFront = false;
                        bDiagonalBack = false;
                    }

                    int iTileHeight(pVertCliffMap[Editor.GetWorld().GetVertCliff(iXTest, iYTest)]);
                    if (iTileHeight < iBaseHeight || iTileHeight > iBaseHeight + 1)
                    {
                        bFront = false;
                        bLeft = false;
                        bRight = false;
                        bBack = false;
                        bDiagonalFront = false;
                        bDiagonalBack = false;
                        bDiagonalLeft = false;
                        bDiagonalRight = false;
                    }

                    if (iTileHeight != iBaseHeight)
                    {
                        bTopLeft = false;
                        bRight = false;
                        bBack = false;
                        bDiagonalLeft = false;
                        bDiagonalRight = false;
                        bDiagonalBack = false;
                        bFlat = false;
                    }

                    if (iTileHeight != iBaseHeight + 1)
                    {
                        bDiagonalFront = false;
                    }
                }
                else if (iYTest == iY)  // TOP MID
                {
                    if (Editor.GetWorld().GetRampTile(iXTest, iYTest) & RAMP_FLAG_SET)
                    {
                        bLeft = false;
                        bRight = false;
                        bDiagonalLeft = false;
                        bDiagonalRight = false;
                        bDiagonalFront = false;
                        bDiagonalBack = false;
                    }

                    int iTileHeight(pVertCliffMap[Editor.GetWorld().GetVertCliff(iXTest, iYTest)]);
                    if (iTileHeight != iBaseHeight + 1)
                    {
                        bLeft = false;
                        bDiagonalFront = false;
                        bDiagonalRight = false;
                    }

                    if (iTileHeight != iBaseHeight)
                    {
                        bFront = false;
                        bRight = false;
                        bBack = false;
                        bDiagonalLeft = false;
                        bDiagonalBack = false;
                        bFlat = false;
                    }
                }
                else if (iYTest == iY + 1)  // TOP RIGHT
                {
                    if (Editor.GetWorld().GetRampTile(iXTest, iYTest) & RAMP_FLAG_SET)
                    {
                        bFront = false;
                        bBack = false;
                        bLeft = false;
                        bRight = false;
                        bDiagonalLeft = false;
                        bDiagonalRight = false;
                    }

                    int iTileHeight(pVertCliffMap[Editor.GetWorld().GetVertCliff(iXTest, iYTest)]);
                    if (iTileHeight < iBaseHeight || iTileHeight > iBaseHeight + 1)
                    {
                        bLeft = false;
                        bBack = false;
                        bRight = false;
                        bFront = false;
                        bDiagonalFront = false;
                        bDiagonalBack = false;
                        bDiagonalLeft = false;
                        bDiagonalRight = false;
                    }

                    if (iTileHeight != iBaseHeight)
                    {
                        bTopRight = false;
                        bRight = false;
                        bFront = false;
                        bDiagonalFront = false;
                        bDiagonalBack = false;
                        bDiagonalLeft = false;
                        bFlat = false;
                    }

                    if (iTileHeight != iBaseHeight + 1)
                    {
                        bDiagonalRight = false;
                    }
                }
                else if (iYTest == iY + 2)  
                {
                    int iTileHeight(pVertCliffMap[Editor.GetWorld().GetVertCliff(iXTest, iYTest)]);
                    byte yRampFlags(Editor.GetWorld().GetRampTile(iXTest, iYTest));
                    if (iTileHeight != iBaseHeight || yRampFlags & RAMP_FLAG_SET)
                    {
                        bDiagonalFront = false;
                        bDiagonalLeft = false;
                    }
                    if (yRampFlags & RAMP_FLAG_SET && yRampFlags & RAMP_FLAG_DIAGONAL)
                    {
                        bFront = false;
                        bRight = false;

                    }
                }
            }
            else if (iXTest == iX)
            {
                if (iYTest == iY - 2)   
                {
                    byte yRampFlags(Editor.GetWorld().GetRampTile(iXTest, iYTest));
                    if (yRampFlags & RAMP_FLAG_SET && yRampFlags & RAMP_FLAG_DIAGONAL)
                    {
                        bBack = false;
                    }
                    if (yRampFlags & RAMP_FLAG_SET)
                    {
                        bDiagonalRight = false;
                        bDiagonalBack = false;
                    }
                }
                else if (iYTest == iY - 1)  // MID LEFT
                {
                    if (Editor.GetWorld().GetRampTile(iXTest, iYTest) & RAMP_FLAG_SET)
                    {
                        bFront = false;
                        bBack = false;
                        bDiagonalLeft = false;
                        bDiagonalRight = false;
                        bDiagonalFront = false;
                        bDiagonalBack = false;
                    }

                    int iTileHeight(pVertCliffMap[Editor.GetWorld().GetVertCliff(iXTest, iYTest)]);
                    if (iTileHeight != iBaseHeight + 1)
                    {
                        bDiagonalFront = false;
                        bDiagonalLeft = false;
                        bFront = false;
                    }

                    if (iTileHeight != iBaseHeight)
                    {
                        bLeft = false;
                        bRight = false;
                        bBack = false;
                        bDiagonalBack = false;
                        bDiagonalRight = false;
                        bFlat = false;
                    }
                }
                else if (iYTest == iY)  // MID MID
                {
                    int iTileHeight(pVertCliffMap[Editor.GetWorld().GetVertCliff(iXTest, iYTest)]);
                    if (iTileHeight != iBaseHeight)
                    {
                        bLeft = false;
                        bRight = false;
                        bBack = false;
                        bFront = false;
                        bDiagonalBack = false;
                        bDiagonalRight = false;
                        bDiagonalFront = false;
                        bDiagonalBack = false;
                        bFlat = false;
                    }
                }
                else if (iYTest == iY + 1)  // MID RIGHT
                {
                    if (Editor.GetWorld().GetRampTile(iXTest, iYTest) & RAMP_FLAG_SET)
                    {
                        bFront = false;
                        bBack = false;
                        bDiagonalLeft = false;
                        bDiagonalRight = false;
                        bDiagonalFront = false;
                        bDiagonalBack = false;
                    }

                    int iTileHeight(pVertCliffMap[Editor.GetWorld().GetVertCliff(iXTest, iYTest)]);
                    if (iTileHeight != iBaseHeight + 1)
                    {
                        bBack = false;
                        bDiagonalRight = false;
                        bDiagonalBack = false;
                    }

                    if (iTileHeight != iBaseHeight)
                    {
                        bRight = false;
                        bLeft = false;
                        bFront = false;
                        bDiagonalLeft = false;
                        bDiagonalFront = false;
                        bFlat = false;
                    }
                }
                else if (iYTest == iY + 2)  
                {
                    byte yRampFlags(Editor.GetWorld().GetRampTile(iXTest, iYTest));
                    if (yRampFlags & RAMP_FLAG_SET && yRampFlags & RAMP_FLAG_DIAGONAL)
                    {
                        bFront = false;
                    }
                    if (yRampFlags & RAMP_FLAG_SET)
                    {
                        bDiagonalFront = false;
                        bDiagonalLeft = false;
                    }
                }
            }
            else if (iXTest == iX + 1)
            {
                if (iYTest == iY - 2)   
                {
                    int iTileHeight(pVertCliffMap[Editor.GetWorld().GetVertCliff(iXTest, iYTest)]);
                    byte yRampFlags(Editor.GetWorld().GetRampTile(iXTest, iYTest));
                    if (iTileHeight != iBaseHeight || yRampFlags & RAMP_FLAG_SET)
                    {
                        bDiagonalBack = false;
                        bDiagonalRight = false;
                    }
                    if (yRampFlags & RAMP_FLAG_SET && yRampFlags & RAMP_FLAG_DIAGONAL)
                    {
                        bBack = false;
                        bLeft = false;
                    }
                }
                else if (iYTest == iY - 1)  // BOTTOM LEFT
                {
                    if (Editor.GetWorld().GetRampTile(iXTest, iYTest) & RAMP_FLAG_SET)
                    {
                        bFront = false;
                        bBack = false;
                        bLeft = false;
                        bRight = false;
                        bDiagonalLeft = false;
                        bDiagonalRight = false;
                    }

                    int iTileHeight(pVertCliffMap[Editor.GetWorld().GetVertCliff(iXTest, iYTest)]);
                    if (iTileHeight < iBaseHeight || iTileHeight > iBaseHeight + 1)
                    {
                        bRight = false;
                        bFront = false;
                        bLeft = false;
                        bBack = false;
                        bDiagonalFront = false;
                        bDiagonalBack = false;
                        bDiagonalLeft = false;
                        bDiagonalRight = false;
                    }

                    if (iTileHeight != iBaseHeight)
                    {
                        bBottomLeft = false;
                        bLeft = false;
                        bBack = false;
                        bDiagonalRight = false;
                        bDiagonalBack = false;
                        bDiagonalFront = false;
                        bFlat = false;
                    }

                    if (iTileHeight != iBaseHeight + 1)
                    {
                        bDiagonalLeft = false;
                    }
                }
                else if (iYTest == iY)  // BOTTOM MID
                {
                    if (Editor.GetWorld().GetRampTile(iXTest, iYTest) & RAMP_FLAG_SET)
                    {
                        bLeft = false;
                        bRight = false;
                        bDiagonalLeft = false;
                        bDiagonalRight = false;
                        bDiagonalFront = false;
                        bDiagonalBack = false;
                    }

                    int iTileHeight(pVertCliffMap[Editor.GetWorld().GetVertCliff(iXTest, iYTest)]);
                    if (iTileHeight != iBaseHeight + 1)
                    {
                        bRight = false;
                        bDiagonalBack = false;
                        bDiagonalLeft = false;
                    }

                    if (iTileHeight != iBaseHeight)
                    {
                        bLeft = false;
                        bBack = false;
                        bFront = false;
                        bDiagonalFront = false;
                        bDiagonalRight = false;
                        bFlat = false;
                    }
                }
                else if (iYTest == iY + 1)  // BOTTOM RIGHT
                {
                    if (Editor.GetWorld().GetRampTile(iXTest, iYTest) & RAMP_FLAG_SET)
                    {
                        bFront = false;
                        bBack = false;
                        bLeft = false;
                        bRight = false;
                        bDiagonalFront = false;
                        bDiagonalBack = false;
                    }

                    int iTileHeight(pVertCliffMap[Editor.GetWorld().GetVertCliff(iXTest, iYTest)]);
                    if (iTileHeight < iBaseHeight || iTileHeight > iBaseHeight + 1)
                    {
                        bBack = false;
                        bRight = false;
                        bLeft = false;
                        bFront = false;
                        bDiagonalFront = false;
                        bDiagonalBack = false;
                        bDiagonalLeft = false;
                        bDiagonalRight = false;
                    }

                    if (iTileHeight != iBaseHeight)
                    {
                        bBottomRight = false;
                        bLeft = false;
                        bFront = false;
                        bDiagonalFront = false;
                        bDiagonalLeft = false;
                        bDiagonalRight = false;
                        bFlat = false;
                    }

                    if (iTileHeight != iBaseHeight + 1)
                    {
                        bDiagonalBack = false;
                    }
                }
                else if (iYTest == iY + 2)  
                {
                    int iTileHeight(pVertCliffMap[Editor.GetWorld().GetVertCliff(iXTest, iYTest)]);
                    byte yRampFlags(Editor.GetWorld().GetRampTile(iXTest, iYTest));
                    if (iTileHeight != iBaseHeight || yRampFlags & RAMP_FLAG_SET)
                    {
                        bDiagonalLeft = false;
                        bDiagonalFront = false;
                    }
                    if (yRampFlags & RAMP_FLAG_SET && yRampFlags & RAMP_FLAG_DIAGONAL)
                    {
                        bFront = false;
                        bLeft = false;
                    }
                }
            }
            else if (iXTest == iX + 2)
            {
                if (iYTest == iY - 2)   
                {
                    bool bRampSet(Editor.GetWorld().GetRampTile(iXTest, iYTest) & RAMP_FLAG_SET);
                    if (bRampSet)
                    {
                        bDiagonalRight = false;
                    }
                }
                else if (iYTest == iY - 1)  
                {
                    int iTileHeight(pVertCliffMap[Editor.GetWorld().GetVertCliff(iXTest, iYTest)]);
                    byte yRampFlags(Editor.GetWorld().GetRampTile(iXTest, iYTest));
                    if (iTileHeight != iBaseHeight || yRampFlags & RAMP_FLAG_SET)
                    {
                        bDiagonalFront = false;
                        bDiagonalRight = false;
                    }
                    if (yRampFlags & RAMP_FLAG_SET && yRampFlags & RAMP_FLAG_DIAGONAL)
                    {
                        bLeft = false;
                        bBack = false;
                    }
                }
                else if (iYTest == iY)  
                {
                    byte yRampFlags(Editor.GetWorld().GetRampTile(iXTest, iYTest));
                    if (yRampFlags & RAMP_FLAG_SET && yRampFlags & RAMP_FLAG_DIAGONAL)
                    {
                        bLeft = false;
                    }
                    if (yRampFlags & RAMP_FLAG_SET)
                    {
                        bDiagonalRight = false;
                        bDiagonalFront = false;
                    }
                }
                else if (iYTest == iY + 1)  
                {
                    int iTileHeight(pVertCliffMap[Editor.GetWorld().GetVertCliff(iXTest, iYTest)]);
                    byte yRampFlags(Editor.GetWorld().GetRampTile(iXTest, iYTest));
                    if (iTileHeight != iBaseHeight || yRampFlags & RAMP_FLAG_SET)
                    {
                        bDiagonalRight = false;
                        bDiagonalFront = false;
                    }
                    if (yRampFlags & RAMP_FLAG_SET && yRampFlags & RAMP_FLAG_DIAGONAL)
                    {
                        bLeft = false;
                        bFront = false;
                    }
                }
                else if (iYTest == iY + 2)  
                {
                    bool bRampSet(Editor.GetWorld().GetRampTile(iXTest, iYTest) & RAMP_FLAG_SET);
                    if (bRampSet)
                    {
                        bDiagonalFront = false;
                    }
                }
            }
        }
    }

        // Only 1 can be true
    if (bFront)
    {
        ushort unRampTileFlagsLeft(Editor.GetWorld().GetRampTile(iX + 1, iY));
        ushort unRampTileFlagsRight(Editor.GetWorld().GetRampTile(iX - 1, iY));
        ushort unExtraFlags(0);

        if (!(unRampTileFlagsLeft & RAMP_FLAG_SECOND_INWARD))
        {
            unExtraFlags |= RAMP_FLAG_FIRST_INWARD;
        }

        if (!(unRampTileFlagsRight & RAMP_FLAG_FIRST_INWARD))
        {
            unExtraFlags |= RAMP_FLAG_SECOND_INWARD;
        }

        if (unExtraFlags & RAMP_FLAG_FIRST_INWARD && bBottomLeft)
        {
            unExtraFlags &= ~RAMP_FLAG_FIRST_INWARD;
            unExtraFlags |= RAMP_FLAG_FIRST_OUTWARD;
        }

        if (unExtraFlags & RAMP_FLAG_SECOND_INWARD && bTopLeft)
        {
            unExtraFlags &= ~RAMP_FLAG_SECOND_INWARD;
            unExtraFlags |= RAMP_FLAG_SECOND_OUTWARD;
        }

        return ushort(RAMP_FLAG_SET | unExtraFlags | unRemoveFlag);
    }

    if (bBack)
    {
        ushort unRampTileFlagsLeft(Editor.GetWorld().GetRampTile(iX - 1, iY));
        ushort unRampTileFlagsRight(Editor.GetWorld().GetRampTile(iX + 1, iY));
        ushort unExtraFlags(0);

        if (!(unRampTileFlagsLeft & RAMP_FLAG_SECOND_INWARD))
        {
            unExtraFlags |= RAMP_FLAG_FIRST_INWARD;
        }

        if (!(unRampTileFlagsRight & RAMP_FLAG_FIRST_INWARD))
        {
            unExtraFlags |= RAMP_FLAG_SECOND_INWARD;
        }

        if (unExtraFlags & RAMP_FLAG_FIRST_INWARD && bTopRight)
        {
            unExtraFlags &= ~RAMP_FLAG_FIRST_INWARD;
            unExtraFlags |= RAMP_FLAG_FIRST_OUTWARD;
        }

        if (unExtraFlags & RAMP_FLAG_SECOND_INWARD && bBottomRight)
        {
            unExtraFlags &= ~RAMP_FLAG_SECOND_INWARD;
            unExtraFlags |= RAMP_FLAG_SECOND_OUTWARD;
        }

        return ushort(RAMP_FLAG_SET | RAMP_FLAG_INVERT_INCLINE | unExtraFlags | unRemoveFlag);
    }

    if (bLeft)
    {
        ushort unRampTileFlagsLeft(Editor.GetWorld().GetRampTile(iX, iY - 1));
        ushort unRampTileFlagsRight(Editor.GetWorld().GetRampTile(iX, iY + 1));
        ushort unExtraFlags(0);

        if (!(unRampTileFlagsLeft & RAMP_FLAG_SECOND_INWARD))
        {
            unExtraFlags |= RAMP_FLAG_FIRST_INWARD;
        }

        if (!(unRampTileFlagsRight & RAMP_FLAG_FIRST_INWARD))
        {
            unExtraFlags |= RAMP_FLAG_SECOND_INWARD;
        }

        if (unExtraFlags & RAMP_FLAG_FIRST_INWARD && bTopLeft)
        {
            unExtraFlags &= ~RAMP_FLAG_FIRST_INWARD;
            unExtraFlags |= RAMP_FLAG_FIRST_OUTWARD;
        }

        if (unExtraFlags & RAMP_FLAG_SECOND_INWARD && bTopRight)
        {
            unExtraFlags &= ~RAMP_FLAG_SECOND_INWARD;
            unExtraFlags |= RAMP_FLAG_SECOND_OUTWARD;
        }

        return ushort(RAMP_FLAG_SET | RAMP_FLAG_LEFT_RIGHT_INCLINE | unExtraFlags | unRemoveFlag);
    }

    if (bRight)
    {
        ushort unRampTileFlagsLeft(Editor.GetWorld().GetRampTile(iX, iY + 1));
        ushort unRampTileFlagsRight(Editor.GetWorld().GetRampTile(iX, iY - 1));
        ushort unExtraFlags(0);

        if (!(unRampTileFlagsLeft & RAMP_FLAG_SECOND_INWARD))
        {
            unExtraFlags |= RAMP_FLAG_FIRST_INWARD;
        }

        if (!(unRampTileFlagsRight & RAMP_FLAG_FIRST_INWARD))
        {
            unExtraFlags |= RAMP_FLAG_SECOND_INWARD;
        }

        if (unExtraFlags & RAMP_FLAG_FIRST_INWARD && bBottomRight)
        {
            unExtraFlags &= ~RAMP_FLAG_FIRST_INWARD;
            unExtraFlags |= RAMP_FLAG_FIRST_OUTWARD;
        }

        if (unExtraFlags & RAMP_FLAG_SECOND_INWARD && bBottomLeft)
        {
            unExtraFlags &= ~RAMP_FLAG_SECOND_INWARD;
            unExtraFlags |= RAMP_FLAG_SECOND_OUTWARD;
        }

        return ushort(RAMP_FLAG_SET | RAMP_FLAG_LEFT_RIGHT_INCLINE | RAMP_FLAG_INVERT_INCLINE | unExtraFlags | unRemoveFlag);
    }

    if (bDiagonalFront)
    {
        ushort unRampTileFlagsLeft(Editor.GetWorld().GetRampTile(iX + 1, iY - 1));
        ushort unRampTileFlagsRight(Editor.GetWorld().GetRampTile(iX - 1, iY + 1));
        ushort unRampTileFlagsFarLeft(Editor.GetWorld().GetRampTile(iX + 2, iY - 2));
        ushort unRampTileFlagsFarRight(Editor.GetWorld().GetRampTile(iX - 2, iY + 2));
        ushort unExtraFlags(0);

        if (!(unRampTileFlagsLeft & RAMP_FLAG_SET))
        {
            unExtraFlags |= RAMP_FLAG_FIRST_INWARD;
        }

        if (!(unRampTileFlagsRight & RAMP_FLAG_SET))
        {
            unExtraFlags |= RAMP_FLAG_SECOND_INWARD;
        }

        if (unRampTileFlagsRight & RAMP_FLAG_DIAGONAL)
        {
            unExtraFlags |= RAMP_FLAG_EX_SECOND_DIAGONAL;
        }

        if (unRampTileFlagsLeft & RAMP_FLAG_DIAGONAL)
        {
            unExtraFlags |= RAMP_FLAG_EX_FIRST_DIAGONAL;
        }

        if (unRampTileFlagsFarLeft & RAMP_FLAG_DIAGONAL && !(unRampTileFlagsLeft & RAMP_FLAG_SET))
        {
            unExtraFlags |= RAMP_FLAG_EX_DOUBLE_LEFT;
            unExtraFlags |= RAMP_FLAG_EX_FIRST_DIAGONAL;
            unExtraFlags &= ~RAMP_FLAG_FIRST_INWARD;
        }

        if (unRampTileFlagsFarRight & RAMP_FLAG_DIAGONAL && !(unRampTileFlagsRight & RAMP_FLAG_SET))
        {
            unExtraFlags |= RAMP_FLAG_EX_DOUBLE_RIGHT;
            unExtraFlags |= RAMP_FLAG_EX_SECOND_DIAGONAL;
            unExtraFlags &= ~RAMP_FLAG_SECOND_INWARD;
        }

        if (CanPlaceDiagonalRamp(iX + 1, iY + 1))
        {
            if ((!(unRampTileFlagsLeft & RAMP_FLAG_SET) || unRampTileFlagsLeft & RAMP_FLAG_DIAGONAL) && (!(unRampTileFlagsRight & RAMP_FLAG_SET) || unRampTileFlagsRight & RAMP_FLAG_DIAGONAL))
                return ushort(RAMP_FLAG_SET | RAMP_FLAG_DIAGONAL | RAMP_FLAG_LEFT_RIGHT_INCLINE | unRemoveFlag | unExtraFlags);
        }
    }

    if (bDiagonalBack)
    {
        ushort unRampTileFlagsLeft(Editor.GetWorld().GetRampTile(iX - 1, iY + 1));
        ushort unRampTileFlagsRight(Editor.GetWorld().GetRampTile(iX + 1, iY - 1));
        ushort unRampTileFlagsFarLeft(Editor.GetWorld().GetRampTile(iX - 2, iY + 2));
        ushort unRampTileFlagsFarRight(Editor.GetWorld().GetRampTile(iX + 2, iY - 2));
        ushort unExtraFlags(0);

        if (!(unRampTileFlagsLeft & RAMP_FLAG_SET))
        {
            unExtraFlags |= RAMP_FLAG_FIRST_INWARD;
        }

        if (!(unRampTileFlagsRight & RAMP_FLAG_SET))
        {
            unExtraFlags |= RAMP_FLAG_SECOND_INWARD;
        }

        if (unRampTileFlagsRight & RAMP_FLAG_DIAGONAL)
        {
            unExtraFlags |= RAMP_FLAG_EX_SECOND_DIAGONAL;
        }

        if (unRampTileFlagsLeft & RAMP_FLAG_DIAGONAL)
        {
            unExtraFlags |= RAMP_FLAG_EX_FIRST_DIAGONAL;
        }

        if (unRampTileFlagsFarLeft & RAMP_FLAG_DIAGONAL && !(unRampTileFlagsLeft & RAMP_FLAG_SET))
        {
            unExtraFlags |= RAMP_FLAG_EX_DOUBLE_LEFT;
            unExtraFlags |= RAMP_FLAG_EX_FIRST_DIAGONAL;
            unExtraFlags &= ~RAMP_FLAG_FIRST_INWARD;
        }

        if (unRampTileFlagsFarRight & RAMP_FLAG_DIAGONAL && !(unRampTileFlagsRight & RAMP_FLAG_SET))
        {
            unExtraFlags |= RAMP_FLAG_EX_DOUBLE_RIGHT;
            unExtraFlags |= RAMP_FLAG_EX_SECOND_DIAGONAL;
            unExtraFlags &= ~RAMP_FLAG_SECOND_INWARD;
        }

        if (CanPlaceDiagonalRamp(iX - 1, iY - 1))
        {
            if ((!(unRampTileFlagsLeft & RAMP_FLAG_SET) || unRampTileFlagsLeft & RAMP_FLAG_DIAGONAL) && (!(unRampTileFlagsRight & RAMP_FLAG_SET) || unRampTileFlagsRight & RAMP_FLAG_DIAGONAL))
                return ushort(RAMP_FLAG_SET | RAMP_FLAG_DIAGONAL | RAMP_FLAG_LEFT_RIGHT_INCLINE | RAMP_FLAG_INVERT_INCLINE | unRemoveFlag | unExtraFlags);
        }
    }

    if (bDiagonalLeft)
    {
        ushort unRampTileFlagsLeft(Editor.GetWorld().GetRampTile(iX + 1, iY + 1));
        ushort unRampTileFlagsRight(Editor.GetWorld().GetRampTile(iX - 1, iY - 1));
        ushort unRampTileFlagsFarLeft(Editor.GetWorld().GetRampTile(iX + 2, iY + 2));
        ushort unRampTileFlagsFarRight(Editor.GetWorld().GetRampTile(iX - 2, iY - 2));
        ushort unExtraFlags(0);

        if (!(unRampTileFlagsLeft & RAMP_FLAG_SET))
        {
            unExtraFlags |= RAMP_FLAG_FIRST_INWARD;
        }

        if (!(unRampTileFlagsRight & RAMP_FLAG_SET))
        {
            unExtraFlags |= RAMP_FLAG_SECOND_INWARD;
        }

        if (unRampTileFlagsRight & RAMP_FLAG_DIAGONAL)
        {
            unExtraFlags |= RAMP_FLAG_EX_SECOND_DIAGONAL;
        }

        if (unRampTileFlagsLeft & RAMP_FLAG_DIAGONAL)
        {
            unExtraFlags |= RAMP_FLAG_EX_FIRST_DIAGONAL;
        }

        if (unRampTileFlagsFarLeft & RAMP_FLAG_DIAGONAL && !(unRampTileFlagsLeft & RAMP_FLAG_SET))
        {
            unExtraFlags |= RAMP_FLAG_EX_DOUBLE_LEFT;
            unExtraFlags |= RAMP_FLAG_EX_FIRST_DIAGONAL;
            unExtraFlags &= ~RAMP_FLAG_FIRST_INWARD;
        }

        if (unRampTileFlagsFarRight & RAMP_FLAG_DIAGONAL && !(unRampTileFlagsRight & RAMP_FLAG_SET))
        {
            unExtraFlags |= RAMP_FLAG_EX_DOUBLE_RIGHT;
            unExtraFlags |= RAMP_FLAG_EX_SECOND_DIAGONAL;
            unExtraFlags &= ~RAMP_FLAG_SECOND_INWARD;
        }

        if (CanPlaceDiagonalRamp(iX - 1, iY + 1))
        {
            if ((!(unRampTileFlagsLeft & RAMP_FLAG_SET) || unRampTileFlagsLeft & RAMP_FLAG_DIAGONAL) && (!(unRampTileFlagsRight & RAMP_FLAG_SET) || unRampTileFlagsRight & RAMP_FLAG_DIAGONAL))
                return ushort(RAMP_FLAG_SET | RAMP_FLAG_DIAGONAL | unRemoveFlag | unExtraFlags);
        }
    }

    if (bDiagonalRight)
    {
        ushort unRampTileFlagsLeft(Editor.GetWorld().GetRampTile(iX - 1, iY - 1));
        ushort unRampTileFlagsRight(Editor.GetWorld().GetRampTile(iX + 1, iY + 1));
        ushort unRampTileFlagsFarLeft(Editor.GetWorld().GetRampTile(iX - 2, iY - 2));
        ushort unRampTileFlagsFarRight(Editor.GetWorld().GetRampTile(iX + 2, iY + 2));
        ushort unExtraFlags(0);

        if (!(unRampTileFlagsLeft & RAMP_FLAG_SET))
        {
            unExtraFlags |= RAMP_FLAG_FIRST_INWARD;
        }

        if (!(unRampTileFlagsRight & RAMP_FLAG_SET))
        {
            unExtraFlags |= RAMP_FLAG_SECOND_INWARD;
        }

        if (unRampTileFlagsRight & RAMP_FLAG_DIAGONAL)
        {
            unExtraFlags |= RAMP_FLAG_EX_SECOND_DIAGONAL;
        }

        if (unRampTileFlagsLeft & RAMP_FLAG_DIAGONAL)
        {
            unExtraFlags |= RAMP_FLAG_EX_FIRST_DIAGONAL;
        }

        if (unRampTileFlagsFarLeft & RAMP_FLAG_DIAGONAL && !(unRampTileFlagsLeft & RAMP_FLAG_SET))
        {
            unExtraFlags |= RAMP_FLAG_EX_DOUBLE_LEFT;
            unExtraFlags |= RAMP_FLAG_EX_FIRST_DIAGONAL;
            unExtraFlags &= ~RAMP_FLAG_FIRST_INWARD;
        }

        if (unRampTileFlagsFarRight & RAMP_FLAG_DIAGONAL && !(unRampTileFlagsRight & RAMP_FLAG_SET))
        {
            unExtraFlags |= RAMP_FLAG_EX_DOUBLE_RIGHT;
            unExtraFlags |= RAMP_FLAG_EX_SECOND_DIAGONAL;
            unExtraFlags &= ~RAMP_FLAG_SECOND_INWARD;
        }

        if (CanPlaceDiagonalRamp(iX + 1, iY - 1))
        {
            if ((!(unRampTileFlagsLeft & RAMP_FLAG_SET) || unRampTileFlagsLeft & RAMP_FLAG_DIAGONAL) && (!(unRampTileFlagsRight & RAMP_FLAG_SET) || unRampTileFlagsRight & RAMP_FLAG_DIAGONAL))
                return ushort(RAMP_FLAG_SET | RAMP_FLAG_DIAGONAL | RAMP_FLAG_INVERT_INCLINE | unRemoveFlag | unExtraFlags);
        }
    }

    if (bFlat)
    {
        return ushort(RAMP_FLAG_EX_FLAT);
    }

    return 0;
}


/*====================
  CCliffTool::CanPlaceDiagonalRamp
  ====================*/
bool    CCliffTool::CanPlaceDiagonalRamp(int iX, int iY)
{
    if (CanPlaceRamp(iX, iY) & RAMP_FLAG_EX_FLAT)
        return true;

    return false;
}