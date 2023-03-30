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
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_INT    (le_cliffMode,                  CLIFF_CREATE);
CVAR_INT    (vid_terrainShowGridOld,        0);
CVAR_INT    (le_cliffClearHeight,           2);
CVAR_INT    (le_cliffVariation,             0);
CVAR_STRING (le_cliffDefinition,            "/world/cliffs/gray/gray.cliff");
CVAR_BOOL   (le_cliffDrawBrushInfluence,    true);
CVAR_FLOAT  (le_cliffBrushInfluenceAlpha,   1.0f);

UI_TRIGGER(CliffMode);
//=============================================================================

/*====================
  CCliffTool::~CCliffTool
  ====================*/
CCliffTool::~CCliffTool()
{
    SAFE_DELETE_ARRAY(m_pTempCliffMap);
}


/*====================
  CCliffTool::CCliffTool
  ====================*/
CCliffTool::CCliffTool() :
ITool(TOOL_CLIFF, _T("cliff")),
m_bWorking(false),
m_bPrimaryDown(false),
m_pTempCliffMap(NULL),
m_bSecondaryDown(false),
m_hLineMaterial(g_ResourceManager.Register(_T("/core/materials/line.material"), RES_MATERIAL)),
m_hCliffMaterial(g_ResourceManager.Register(_T("/core/materials/cliff.material"), RES_MATERIAL)),
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
    switch (le_cliffMode)
    {
    case CLIFF_PAINT:
    if (!m_bInverse)
        m_bWorking = false;
        break;
    case CLIFF_CREATE:
    case CLIFF_CLEAR:
        break;
    }
    m_bPrimaryDown = false;
}


/*====================
  CCliffTool::PrimaryDown

  Default left mouse button down action
  ====================*/
void    CCliffTool::PrimaryDown()
{
    switch (le_cliffMode)
    {
    case CLIFF_PAINT:
        m_bWorking = true;
        m_bInverse = false;
        break;
    case CLIFF_CREATE:
        SaveTempCliffMap();
        CliffRaiseOrLower(true);
        break;
    case CLIFF_CLEAR:
        SaveTempCliffMap();
        CliffClear();
        break;
    case CLIFF_VARIATION:
        CliffVariation();
        break;
    case CLIFF_DEFINITION:
        CliffDefinition();
        break;
    }
    m_bPrimaryDown = true;
}


/*====================
  CCliffTool::SecondaryUp

  Right mouse button up action
  ====================*/
void    CCliffTool::SecondaryUp()
{
    switch (le_cliffMode)
    {
    case CLIFF_PAINT:
        if (m_bInverse)
            m_bWorking = false;
        break;
    case CLIFF_CREATE:
        delete[] m_pTempCliffMap;
        break;
    }
    m_bSecondaryDown = false;
}


/*====================
  CCliffTool::SecondaryDown

  Default right mouse button down action
  ====================*/
void    CCliffTool::SecondaryDown()
{
    switch (le_cliffMode)
    {
    case CLIFF_PAINT:
        m_bWorking = true;
        m_bInverse = true;
        break;
    case CLIFF_CREATE:
        SaveTempCliffMap();
        CliffRaiseOrLower(false);
        break;
    }
    m_bSecondaryDown = true;
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

    if (Editor.TraceCursor(trace, TRACE_TERRAIN))
    {
        m_iX = Editor.GetWorld().GetTileFromCoord(trace.v3EndPos.x);
        m_iY = Editor.GetWorld().GetTileFromCoord(trace.v3EndPos.y);

        CBrush *pBrush(CBrush::GetCurrentBrush());
        if (pBrush != NULL)
        {
            int iBrushScale(le_cliffMode == CLIFF_PAINT ? 1 : Editor.GetWorld().GetCliffSize());

            // Get brush dimensions
            CRecti recBrush;
            pBrush->ClipBrush(recBrush);

            // Brush centers after scaling
            float fBrushCenterX((recBrush.left + recBrush.right) * iBrushScale / 2.0f);
            float fBrushCenterY((recBrush.top + recBrush.bottom) * iBrushScale / 2.0f);

            float fTestX((trace.v3EndPos.x - fBrushCenterX * Editor.GetWorld().GetScale()) / (iBrushScale * Editor.GetWorld().GetScale()));
            float fTestY((trace.v3EndPos.y - fBrushCenterY * Editor.GetWorld().GetScale()) / (iBrushScale * Editor.GetWorld().GetScale()));

            m_iBrushX = fTestX < 0.0f ? INT_FLOOR(fTestX) : INT_CEIL(fTestX);
            m_iBrushY = fTestY < 0.0f ? INT_FLOOR(fTestY) : INT_CEIL(fTestY);
        }
        else
        {
            m_iBrushX = -1;
            m_iBrushY = -1;
        }

        m_v3EndPos = trace.v3EndPos;
    }
    else
    {
        m_iX = -1;
        m_iY = -1;
        m_iBrushX = -1;
        m_iBrushY = -1;

        m_v3EndPos.Clear();
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
    
    if (bAdd)
    {
        for (int y(0); y < recArea.GetHeight(); ++y)
        {
            for (int x(0); x < recArea.GetWidth(); ++x)
            {
                if (brush[BRUSH_INDEX(x, y)])
                    pRegion[iRegionIndex] = 1;
                
                ++iRegionIndex;
            }
        }
    }
    else
    {
        for (int y(0); y < recArea.GetHeight(); ++y)
        {
            for (int x(0); x < recArea.GetWidth(); ++x)
            {
                if (brush[BRUSH_INDEX(x, y)])
                    pRegion[iRegionIndex] = 0;
                
                ++iRegionIndex;
            }
        }
    }
}


/*====================
  CCliffTool::PaintCliff
  ====================*/
void    CCliffTool::PaintCliff(float fFrameTime)
{
    byte *pRegion(NULL);

    try
    {
        CBrush *pBrush(CBrush::GetCurrentBrush());
        if (pBrush == NULL)
            EX_ERROR(_T("No brush selected"));

        if (!Editor.GetWorld().IsInBounds(m_iX, m_iY, TILE_SPACE))
            EX_WARN(_T("Out of bounds coordinate"));

        // Clip against the brush data
        CRecti  recClippedBrush;
        if (!pBrush->ClipBrush(recClippedBrush))
            return;

        // Clip the brush against the world
        recClippedBrush.Shift(m_iX - pBrush->GetBrushSize() / 2, m_iY - pBrush->GetBrushSize() / 2);
        if (!Editor.GetWorld().ClipRect(recClippedBrush, TILE_SPACE))
            return;

        // Get the region
        pRegion = new byte[recClippedBrush.GetArea()];
        if (pRegion == NULL)
            EX_ERROR(_T("Failed to allocate region"));

        if (!Editor.GetWorld().GetRegion(WORLD_TILE_CLIFF_MAP, recClippedBrush, pRegion))
            EX_ERROR(_T("Failed to retrieve region"));

        // Perform the operation
        recClippedBrush.Shift(-(m_iX - pBrush->GetBrushSize() / 2), -(m_iY - pBrush->GetBrushSize() / 2));

        CliffModify(pRegion, recClippedBrush, *pBrush, !m_bInverse);

        // Apply the modified region
        recClippedBrush.Shift(m_iX - pBrush->GetBrushSize() / 2, m_iY - pBrush->GetBrushSize() / 2 + 1);
        if (!Editor.GetWorld().SetRegion(WORLD_TILE_CLIFF_MAP, recClippedBrush, pRegion))
            EX_ERROR(_T("SetRegion failed"));

        // Notify the video drivers about the update
        for (int y(recClippedBrush.top); y <= recClippedBrush.bottom; ++y)
        {
            for (int x = recClippedBrush.left; x <= recClippedBrush.right; ++x)
            {
                Vid.Notify(VID_NOTIFY_TERRAIN_SHADER_MODIFIED, x, y, 0, &Editor.GetWorld());
                Vid.Notify(VID_NOTIFY_TERRAIN_NORMAL_MODIFIED, x, y, 0, &Editor.GetWorld());
            }
        }

        delete[] pRegion;
    }
    catch (CException &ex)
    {
        if (pRegion != NULL)
            delete[] pRegion;

        ex.Process(_T("CCliffTool::PaintCliff() - "), NO_THROW);
    }
}


/*====================
  CCliffTool::Frame
 ====================*/
void    CCliffTool::Frame(float fFrameTime)
{
    CalcToolProperties();

    if (m_bWorking && m_iX != -1 && m_iY != -1)
        PaintCliff(fFrameTime);
    switch (le_cliffMode)
    {
    case CLIFF_CREATE:
        if (m_bPrimaryDown && m_bModifier2)
            CliffRaiseOrLower(true);
        if (m_bSecondaryDown && m_bModifier2)
            CliffRaiseOrLower(false);
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
//  byte alpha0 = 177;
//  byte alpha1 = 177;

    if (m_iX < 0 || m_iY < 0)
        return;

    SSceneFaceVert poly[1024];
    MemManager.Set(poly, 0, sizeof(poly));
    int p = 0;

    if (le_cliffMode == CLIFF_PAINT)
    {
        CBrush *pBrush = CBrush::GetCurrentBrush();
        float fTileSize = Editor.GetWorld().GetScale();

        if (!pBrush)
            return;

        int iX = m_iX, iY = m_iY + 1;

        for (int y = 0; y < pBrush->GetBrushSize(); ++y)
        {
            for (int x = 0; x < pBrush->GetBrushSize(); ++x)
            {
                int i = x + y * pBrush->GetBrushSize();

                // left
                if ((x > 0 && (*pBrush)[i] && !(*pBrush)[i - 1]) || (x == 0 && (*pBrush)[i]))
                {
                    int dX = iX + x - pBrush->GetBrushSize()/2;
                    int dY = iY + y - pBrush->GetBrushSize()/2;

                    if (Editor.GetWorld().IsInBounds(dX, dY, GRID_SPACE) && Editor.GetWorld().IsInBounds(dX, dY + 1, GRID_SPACE))
                    {
                        poly[p].vtx[0] = dX * fTileSize;
                        poly[p].vtx[1] = dY * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(dX, dY);
                        SET_VEC4(poly[p].col, 255, 255, 255, 255);
                        ++p;

                        poly[p].vtx[0] = dX * fTileSize;
                        poly[p].vtx[1] = (dY + 1) * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(dX, (dY + 1));
                        SET_VEC4(poly[p].col, 255, 255, 255, 255);
                        ++p;
                    }
                }

                // right
                if ((x < pBrush->GetBrushSize() - 1 && (*pBrush)[i] && !(*pBrush)[i + 1]) || (x == pBrush->GetBrushSize() - 1 && (*pBrush)[i]))
                {
                    int dX = iX + x - pBrush->GetBrushSize()/2;
                    int dY = iY + y - pBrush->GetBrushSize()/2;

                    if (Editor.GetWorld().IsInBounds(dX + 1, dY, GRID_SPACE) && Editor.GetWorld().IsInBounds(dX + 1, dY + 1, GRID_SPACE))
                    {
                        poly[p].vtx[0] = (dX + 1) * fTileSize;
                        poly[p].vtx[1] = dY * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint((dX + 1), dY);
                        SET_VEC4(poly[p].col, 255, 255, 255, 255);
                        ++p;

                        poly[p].vtx[0] = (dX + 1) * fTileSize;
                        poly[p].vtx[1] = (dY + 1) * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint((dX + 1), (dY+ 1));
                        SET_VEC4(poly[p].col, 255, 255, 255, 255);
                        ++p;
                    }
                }

                // top
                if ((y > 0 && (*pBrush)[i] && !(*pBrush)[i - pBrush->GetBrushSize()]) || (y == 0 && (*pBrush)[i]))
                {
                    int dX = iX + x - pBrush->GetBrushSize()/2;
                    int dY = iY + y - pBrush->GetBrushSize()/2;

                    if (Editor.GetWorld().IsInBounds(dX, dY, GRID_SPACE) && Editor.GetWorld().IsInBounds(dX + 1, dY, GRID_SPACE))
                    {
                        poly[p].vtx[0] = dX * fTileSize;
                        poly[p].vtx[1] = dY * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(dX, dY);
                        SET_VEC4(poly[p].col, 255, 255, 255, 255);
                        ++p;

                        poly[p].vtx[0] = (dX + 1) * fTileSize;
                        poly[p].vtx[1] = dY * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint((dX + 1), dY);
                        SET_VEC4(poly[p].col, 255, 255, 255, 255);
                        ++p;
                    }
                }

                // bottom
                if ((y < pBrush->GetBrushSize() - 1 && (*pBrush)[i] && !(*pBrush)[i+pBrush->GetBrushSize()]) || (y == pBrush->GetBrushSize() - 1 && (*pBrush)[i]))
                {
                    int dX = iX + x - pBrush->GetBrushSize()/2;
                    int dY = iY + y - pBrush->GetBrushSize()/2;

                    if (Editor.GetWorld().IsInBounds(dX + 1, dY + 1, GRID_SPACE) && Editor.GetWorld().IsInBounds(dX, dY + 1, GRID_SPACE))
                    {
                        poly[p].vtx[0] = (dX + 1) * fTileSize;
                        poly[p].vtx[1] = (dY + 1) * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint((dX + 1), (dY+ 1));
                        SET_VEC4(poly[p].col, 255, 255, 255, 255);
                        ++p;

                        poly[p].vtx[0] = dX * fTileSize;
                        poly[p].vtx[1] = (dY + 1) * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(dX, (dY + 1));
                        SET_VEC4(poly[p].col, 255, 255, 255, 255);
                        ++p;
                    }
                }
            }
        }

        if (p > 0)
            SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
    }

    if (le_cliffMode == CLIFF_CREATE || CLIFF_CLEAR)
    {
        MemManager.Set(poly, 0, sizeof(poly));
        p = 0;
        CBrush *pBrush = CBrush::GetCurrentBrush();
        float fTileSize = Editor.GetWorld().GetScale();

        if (!pBrush)
            return;

        int iCliffSize(Editor.GetWorld().GetCliffSize());

        int iX = m_iBrushX * iCliffSize - iCliffSize / 2;
        int iY = m_iBrushY * iCliffSize - iCliffSize / 2;

        for (int y = 0; y < pBrush->GetBrushSize() * iCliffSize; ++y)
        {
            for (int x = 0; x < pBrush->GetBrushSize() * iCliffSize; ++x)
            {
                int i = x / iCliffSize + y / iCliffSize * pBrush->GetBrushSize();

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
                        SET_VEC4(poly[p].col, 255, 255, 255, 255);
                        ++p;

                        poly[p].vtx[0] = dX * fTileSize;
                        poly[p].vtx[1] = (dY + 1) * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(dX, (dY + 1));
                        SET_VEC4(poly[p].col, 255, 255, 255, 255);
                        ++p;
                    }
                }
                
                // right
                if (((x < pBrush->GetBrushSize() * iCliffSize - 1 && (*pBrush)[i] && !(*pBrush)[i + 1]) || (x == pBrush->GetBrushSize() - 1 && (*pBrush)[i])) && x % iCliffSize == 3)
                {
                    int dX = iX + x;
                    int dY = iY + y;

                    if (Editor.GetWorld().IsInBounds(dX + 1, dY, GRID_SPACE) && Editor.GetWorld().IsInBounds(dX + 1, dY + 1, GRID_SPACE))
                    {
                        poly[p].vtx[0] = (dX + 1) * fTileSize;
                        poly[p].vtx[1] = dY * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint((dX + 1), dY);
                        SET_VEC4(poly[p].col, 255, 255, 255, 255);
                        ++p;

                        poly[p].vtx[0] = (dX + 1) * fTileSize;
                        poly[p].vtx[1] = (dY + 1) * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint((dX + 1), (dY+ 1));
                        SET_VEC4(poly[p].col, 255, 255, 255, 255);
                        ++p;
                    }
                }
                
                // top
                if (((y > 0 && (*pBrush)[i] && !(*pBrush)[i - pBrush->GetBrushSize()]) || (y == 0 && (*pBrush)[i])) && y % iCliffSize == 0)
                {
                    int dX = iX + x;
                    int dY = iY + y;

                    if (Editor.GetWorld().IsInBounds(dX, dY, GRID_SPACE) && Editor.GetWorld().IsInBounds(dX + 1, dY, GRID_SPACE))
                    {
                        poly[p].vtx[0] = dX * fTileSize;
                        poly[p].vtx[1] = dY * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(dX, dY);
                        SET_VEC4(poly[p].col, 255, 255, 255, 255);
                        ++p;

                        poly[p].vtx[0] = (dX + 1) * fTileSize;
                        poly[p].vtx[1] = dY * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint((dX + 1), dY);
                        SET_VEC4(poly[p].col, 255, 255, 255, 255);
                        ++p;
                    }
                }

                // bottom
                if (((y < pBrush->GetBrushSize() * iCliffSize - 1 && (*pBrush)[i] && !(*pBrush)[i+pBrush->GetBrushSize()]) || (y == pBrush->GetBrushSize() * iCliffSize - 1 && (*pBrush)[i])) && y % iCliffSize == 3)
                {
                    int dX = iX + x;
                    int dY = iY + y;

                    if (Editor.GetWorld().IsInBounds(dX + 1, dY + 1, GRID_SPACE) && Editor.GetWorld().IsInBounds(dX, dY + 1, GRID_SPACE))
                    {
                        poly[p].vtx[0] = (dX + 1) * fTileSize;
                        poly[p].vtx[1] = (dY + 1) * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint((dX + 1), (dY+ 1));
                        SET_VEC4(poly[p].col, 255, 255, 255, 255);
                        ++p;

                        poly[p].vtx[0] = dX * fTileSize;
                        poly[p].vtx[1] = (dY + 1) * fTileSize;
                        poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(dX, (dY + 1));
                        SET_VEC4(poly[p].col, 255, 255, 255, 255);
                        ++p;
                    }
                } 
            }
        }
        
    }
    
    if (p > 0)
    SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
}


/*====================
  CCliffTool::CliffClear
  ====================*/
void    CCliffTool::CliffClear()
{
    //Alter Cliff Height Map
    CliffMapSet(le_cliffClearHeight);

    //Alter Definition Map
    DefinitionMapSet();
    
    //Alter Variation Map
    VariationMapSet();

    //Get Brush
    CBrush *pBrush = CBrush::GetCurrentBrush();

    // Clip against the brush data
    CRecti  modifiedArea;
    if (!pBrush->ClipBrush(modifiedArea))
        return;

    // Clip the brush against the world
    int m_iXOffset = (m_iX + Editor.GetWorld().GetCliffSize() / 2) / Editor.GetWorld().GetCliffSize();
    int m_iYOffset = (m_iY + Editor.GetWorld().GetCliffSize() / 2) / Editor.GetWorld().GetCliffSize() + 1;
    modifiedArea.Shift(m_iXOffset - pBrush->GetBrushSize() / 2, m_iYOffset - pBrush->GetBrushSize() / 2);

    //Clip brush against cliff map
    modifiedArea.right = MIN(Editor.GetWorld().GetCliffGridWidth(), modifiedArea.right);
    modifiedArea.bottom = MIN(Editor.GetWorld().GetCliffGridHeight(), modifiedArea.bottom);

    //Make sure all tiles conform to height minimum
    modifiedArea.left = CLAMP(modifiedArea.left, 0, Editor.GetWorld().GetGridWidth() / Editor.GetWorld().GetCliffSize());
    modifiedArea.right = CLAMP(modifiedArea.right , 0, Editor.GetWorld().GetGridWidth() / Editor.GetWorld().GetCliffSize() + 1);
    modifiedArea.top = CLAMP(modifiedArea.top, 0, Editor.GetWorld().GetGridHeight() / Editor.GetWorld().GetCliffSize());
    modifiedArea.bottom = CLAMP(modifiedArea.bottom, 0, Editor.GetWorld().GetGridHeight() / Editor.GetWorld().GetCliffSize() + 1);

    CRecti enforcedArea = modifiedArea;

    //Enforce Height
    for (int y(modifiedArea.top); y <= modifiedArea.bottom; y++)
    {
        for (int x(modifiedArea.left); x <= modifiedArea.right; x++)
        {
            EnforceHeight(x,y, enforcedArea);
        }
    }

    //Calculate Tiles
    for (int x(enforcedArea.left - 1); x <= enforcedArea.right + 1; x++)
    {
        for (int y(enforcedArea.top - 1); y <= enforcedArea.bottom + 1; y++)
        {
            CalculateTile(x,y);
        }
    }

    //Calculate Blockers
    CalculateCliffBlockers(enforcedArea);
}


/*====================
  CCliffTool::CliffDefinition
  ====================*/
void    CCliffTool::CliffDefinition()
{
    DefinitionMapSet();

    //Get Brush
    CBrush *pBrush = CBrush::GetCurrentBrush();

    // Clip against the brush data
    CRecti  modifiedArea;
    if (!pBrush->ClipBrush(modifiedArea))
        return;

    // Clip the brush against the world
    int m_iXOffset = (m_iX + Editor.GetWorld().GetCliffSize() / 2) / Editor.GetWorld().GetCliffSize();
    int m_iYOffset = (m_iY + Editor.GetWorld().GetCliffSize() / 2) / Editor.GetWorld().GetCliffSize() + 1;
    modifiedArea.Shift(m_iXOffset - pBrush->GetBrushSize() / 2, m_iYOffset - pBrush->GetBrushSize() / 2);

    //Clip brush against cliff map
    modifiedArea.right = MIN(Editor.GetWorld().GetCliffGridWidth(), modifiedArea.right);
    modifiedArea.bottom = MIN(Editor.GetWorld().GetCliffGridHeight(), modifiedArea.bottom);

    //Make sure all tiles conform to height minimum
    modifiedArea.left = CLAMP(modifiedArea.left, 0, Editor.GetWorld().GetGridWidth() / Editor.GetWorld().GetCliffSize());
    modifiedArea.right = CLAMP(modifiedArea.right , 0, Editor.GetWorld().GetGridWidth() / Editor.GetWorld().GetCliffSize() + 1);
    modifiedArea.top = CLAMP(modifiedArea.top, 0, Editor.GetWorld().GetGridHeight() / Editor.GetWorld().GetCliffSize());
    modifiedArea.bottom = CLAMP(modifiedArea.bottom, 0, Editor.GetWorld().GetGridHeight() / Editor.GetWorld().GetCliffSize() + 1);

    //Calculate Tiles
    for (int x(modifiedArea.left - 1); x <= modifiedArea.right + 1; x++)
    {
        for (int y(modifiedArea.top - 1); y <= modifiedArea.bottom + 1; y++)
        {
            CalculateTile(x,y);
        }
    }
}


/*====================
  CCliffTool::CliffVariation
  ====================*/
void    CCliffTool::CliffVariation()
{
    VariationMapSet();

    //Get Brush
    CBrush *pBrush = CBrush::GetCurrentBrush();

    // Clip against the brush data
    CRecti  modifiedArea;
    if (!pBrush->ClipBrush(modifiedArea))
        return;

    // Clip the brush against the world
    int m_iXOffset = (m_iX + Editor.GetWorld().GetCliffSize() / 2) / Editor.GetWorld().GetCliffSize();
    int m_iYOffset = (m_iY + Editor.GetWorld().GetCliffSize() / 2) / Editor.GetWorld().GetCliffSize() + 1;
    modifiedArea.Shift(m_iXOffset - pBrush->GetBrushSize() / 2, m_iYOffset - pBrush->GetBrushSize() / 2);

    //Clip brush against cliff map
    modifiedArea.right = MIN(Editor.GetWorld().GetCliffGridWidth(), modifiedArea.right);
    modifiedArea.bottom = MIN(Editor.GetWorld().GetCliffGridHeight(), modifiedArea.bottom);

    //Make sure all tiles conform to height minimum
    modifiedArea.left = CLAMP(modifiedArea.left, 0, Editor.GetWorld().GetGridWidth() / Editor.GetWorld().GetCliffSize());
    modifiedArea.right = CLAMP(modifiedArea.right , 0, Editor.GetWorld().GetGridWidth() / Editor.GetWorld().GetCliffSize() + 1);
    modifiedArea.top = CLAMP(modifiedArea.top, 0, Editor.GetWorld().GetGridHeight() / Editor.GetWorld().GetCliffSize());
    modifiedArea.bottom = CLAMP(modifiedArea.bottom, 0, Editor.GetWorld().GetGridHeight() / Editor.GetWorld().GetCliffSize() + 1);

    //Calculate Tiles
    for (int x(modifiedArea.left - 1); x <= modifiedArea.right + 1; x++)
    {
        for (int y(modifiedArea.top - 1); y <= modifiedArea.bottom + 1; y++)
        {
            CalculateTile(x,y);
        }
    }
}


/*====================
  CCliffTool::GetCliffGridIndex
  ====================*/
uint    CCliffTool::GetCliffGridIndex(int x, int y)
{
    return (y * Editor.GetWorld().GetCliffGridWidth()) + x;
}


/*====================
  CCliffTool::GetSavedRegion
  ====================*/
bool    CCliffTool::GetSavedRegion(const CRecti &recArea, void *pDest)
{
    int iCliffGridWidth(Editor.GetWorld().GetCliffGridWidth());

    assert(recArea.right <= iCliffGridWidth + 1);
    assert(recArea.left >= 0);
    assert(recArea.bottom <= Editor.GetWorld().GetCliffTileHeight() + 1);
    assert(recArea.top >= 0);
    assert(recArea.GetArea() > 0);

    int *pCliffDest(static_cast<int*>(pDest));
    int *pCliffSource = &m_pTempCliffMap [GetCliffGridIndex(recArea.left, recArea.top)];

    for (int y(0); y < recArea.GetHeight(); ++y, pCliffSource += iCliffGridWidth, pCliffDest += recArea.GetWidth())
        MemManager.Copy(pCliffDest, pCliffSource, sizeof(int) * recArea.GetWidth());

    return true;
}


/*====================
  CCliffTool::CliffMapAdd
  ====================*/
void    CCliffTool::CliffMapAdd(int iCliffHeight)
{
    // Get Brush
    CBrush *pBrush = CBrush::GetCurrentBrush();
    if (pBrush == NULL)
        return;

    // Clip against the brush data
    CRecti  recClippedBrush;
    if (!pBrush->ClipBrush(recClippedBrush))
        return;

    // Clip the brush against the world
    recClippedBrush.Shift(m_iBrushX, m_iBrushY);
    recClippedBrush.Crop(0, 0, Editor.GetWorld().GetCliffGridWidth(), Editor.GetWorld().GetCliffGridHeight());
    if (!recClippedBrush.IsNormalized())
        return;

    // Get the region
    int *pRegion = new int[recClippedBrush.GetArea()];
    int *pSavedRegion = new int[recClippedBrush.GetArea()];
    Editor.GetWorld().GetRegion(WORLD_VERT_CLIFF_MAP, recClippedBrush, pRegion);
    GetSavedRegion(recClippedBrush, pSavedRegion);

    // Perform the operation
    recClippedBrush.Shift(-m_iBrushX, -m_iBrushY);

    int iBrushSize(pBrush->GetBrushSize());

    int iRegionIndex(0);
    for (int y(0); y < recClippedBrush.GetHeight(); ++y)
    {
        for (int x(0); x < recClippedBrush.GetWidth(); ++x)
        {
            if (pRegion[iRegionIndex] == pSavedRegion[iRegionIndex] &&
                (*pBrush)[(recClippedBrush.left + x) + ((recClippedBrush.top + y) * iBrushSize)] != 0)
                pRegion[iRegionIndex] += iCliffHeight;
            
            ++iRegionIndex;
        }
    }

    recClippedBrush.Shift(m_iBrushX, m_iBrushY);

    Editor.GetWorld().SetRegion(WORLD_VERT_CLIFF_MAP, recClippedBrush, pRegion);
    
    delete[] pRegion;
    delete[] pSavedRegion;
}


/*====================
  CCliffTool::CliffMapSet
  ====================*/
void    CCliffTool::CliffMapSet(int iCliffHeight)
{
    // Get Brush
    CBrush *pBrush = CBrush::GetCurrentBrush();
    if (pBrush == NULL)
        return;

    // Clip against the brush data
    CRecti  recClippedBrush;
    if (!pBrush->ClipBrush(recClippedBrush))
        return;

    // Clip the brush against the world
    recClippedBrush.Shift(m_iBrushX, m_iBrushY);
    recClippedBrush.Crop(0, 0, Editor.GetWorld().GetCliffGridWidth(), Editor.GetWorld().GetCliffGridHeight());
    if (!recClippedBrush.IsNormalized())
        return;

    // Get the region
    int *pRegion = new int[recClippedBrush.GetArea()];
    int *pSavedRegion = new int[recClippedBrush.GetArea()];
    Editor.GetWorld().GetRegion(WORLD_VERT_CLIFF_MAP, recClippedBrush, pRegion);
    GetSavedRegion(recClippedBrush, pSavedRegion);

    // Perform the operation
    recClippedBrush.Shift(-m_iBrushX, -m_iBrushY);

    int iBrushSize(pBrush->GetBrushSize());

    int iRegionIndex(0);
    for (int y(0); y < recClippedBrush.GetHeight(); ++y)
    {
        for (int x(0); x < recClippedBrush.GetWidth(); ++x)
        {
            if (pRegion[iRegionIndex] == pSavedRegion[iRegionIndex] &&
                (*pBrush)[(recClippedBrush.left + x) + ((recClippedBrush.top + y) * iBrushSize)] != 0)
                pRegion[iRegionIndex] = iCliffHeight;
            
            ++iRegionIndex;
        }
    }

    recClippedBrush.Shift(m_iBrushX, m_iBrushY);

    Editor.GetWorld().SetRegion(WORLD_VERT_CLIFF_MAP, recClippedBrush, pRegion);
    
    delete[] pRegion;
    delete[] pSavedRegion;
}


/*====================
  CCliffTool::DefinitionMapSet
  ====================*/
void    CCliffTool::DefinitionMapSet()
{
    // Get current cliff definition
    ResHandle hCliff(g_ResourceManager.Register(le_cliffDefinition, RES_CLIFFDEF));

    CRecti recArea(0, 0, Editor.GetWorld().GetCliffTileWidth(), Editor.GetWorld().GetCliffTileHeight());

    // Get the region
    uint *pRegion = new uint[recArea.GetArea()];
    //Editor.GetWorld().GetRegion(WORLD_VARIATION_CLIFF_MAP, recArea, pRegion, 0);

    Editor.GetWorld().AddCliffDef(hCliff);

    int iRegionIndex(0);
    for (int y(0); y < recArea.GetHeight(); ++y)
    {
        for (int x(0); x < recArea.GetWidth(); ++x)
        {
            pRegion[iRegionIndex] = hCliff;
            ++iRegionIndex;
        }
    }

    Editor.GetWorld().SetRegion(WORLD_VARIATION_CLIFF_MAP, recArea, pRegion, 0);
    
    delete[] pRegion;
}


/*====================
  CCliffTool::VariationMapSet
  ====================*/
void    CCliffTool::VariationMapSet()
{
    CRecti recArea(0, 0, Editor.GetWorld().GetCliffTileWidth(), Editor.GetWorld().GetCliffTileHeight());

    // Get the region
    uint *pRegion = new uint[recArea.GetArea()];
    //Editor.GetWorld().GetRegion(WORLD_VARIATION_CLIFF_MAP, recArea, pRegion, 0);

    int iRegionIndex(0);
    for (int y(0); y < recArea.GetHeight(); ++y)
    {
        for (int x(0); x < recArea.GetWidth(); ++x)
        {
            pRegion[iRegionIndex] = le_cliffVariation;
            ++iRegionIndex;
        }
    }

    Editor.GetWorld().SetRegion(WORLD_VARIATION_CLIFF_MAP, recArea, pRegion, 0);
    
    delete[] pRegion;
}


/*====================
  CCliffTool::CliffRaiseOrLower
  ====================*/
void    CCliffTool::CliffRaiseOrLower(bool bRaise)
{
    int iCliffHeight(bRaise ? 1 : -1);

    // Alter Cliff Height Map
    CliffMapAdd(iCliffHeight);

    // Alter Definition Map
    DefinitionMapSet();
    
    // Alter Variation Map
    VariationMapSet();

    // Get Brush
    CBrush *pBrush = CBrush::GetCurrentBrush();

    // Clip against the brush data
    CRecti  modifiedArea;
    if (!pBrush->ClipBrush(modifiedArea))
        return;

    // Clip the brush against the world
    modifiedArea.Shift(m_iBrushX, m_iBrushY);

    // Clip brush against cliff map
    modifiedArea.Crop(0, 0, Editor.GetWorld().GetCliffGridWidth(), Editor.GetWorld().GetCliffGridHeight());
    if (!modifiedArea.IsNormalized())
        return;

    CRecti enforcedArea = modifiedArea;

    // Enforce Height
    for (int y(modifiedArea.top); y <= modifiedArea.bottom; y++)
    {
        for (int x(modifiedArea.left); x <= modifiedArea.right; x++)
        {
            EnforceHeight(x, y, enforcedArea);
        }
    }

    // Calculate Tiles
    for (int x(enforcedArea.left - 1); x <= enforcedArea.right + 1; x++)
    {
        for (int y(enforcedArea.top - 1); y <= enforcedArea.bottom + 1; y++)
        {
            CalculateTile(x,y);
        }
    }

    // Calculate Blockers
    CalculateCliffBlockers(enforcedArea);
}


/*====================
  CCliffTool::EnforceHeight
  ====================*/
void    CCliffTool::EnforceHeight(int x, int y, CRecti &enforcedRect)
{
    if (x < 0 || x > Editor.GetWorld().GetGridWidth() / Editor.GetWorld().GetCliffSize() || y < 0 || y > Editor.GetWorld().GetGridHeight() / Editor.GetWorld().GetCliffSize())
        return;

    //Get Cliff Map
    int* pVertCliffMap = Editor.GetWorld().GetVertCliffMap();
    
    int maxHeight = pVertCliffMap[Editor.GetWorld().GetVertCliff(x,y)] + 2;
    int minHeight = pVertCliffMap[Editor.GetWorld().GetVertCliff(x,y)] - 2;

    std::vector<CVec2i> modifiedCoords;
    modifiedCoords.reserve(10);

    for (int ix = max(0, x - 1); ix <= min(x + 1, Editor.GetWorld().GetGridWidth() / Editor.GetWorld().GetCliffSize()); ix++)
    {
        for (int iy = max(0, y - 1); iy <= min(y + 1, Editor.GetWorld().GetGridHeight() / Editor.GetWorld().GetCliffSize()); iy++)
        {

            if (pVertCliffMap[Editor.GetWorld().GetVertCliff(ix,iy)] > maxHeight)
            {
                pVertCliffMap[Editor.GetWorld().GetVertCliff(ix,iy)] = maxHeight;
                enforcedRect.AddPoint(CVec2i(ix,iy));
                modifiedCoords.push_back(CVec2i(ix,iy));
            }
            if (pVertCliffMap[Editor.GetWorld().GetVertCliff(ix,iy)] < minHeight)
            {
                pVertCliffMap[Editor.GetWorld().GetVertCliff(ix,iy)] = minHeight;
                enforcedRect.AddPoint(CVec2i(ix,iy));
                modifiedCoords.push_back(CVec2i(ix,iy));
            }
        }
    }

    for(size_t i(0); i < modifiedCoords.size(); i++)
        EnforceHeight(modifiedCoords[i].x, modifiedCoords[i].y, enforcedRect);

    return;
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
    ResHandle CliffDefinitionHandle = Editor.GetWorld().GetCliffDefHandle(pDefinitionMap[Editor.GetWorld().GetTileCliff(iXC, iYC)]);
    CCliffDefinitionResource * CliffDefinition = static_cast<CCliffDefinitionResource*>(g_ResourceManager.Get(CliffDefinitionHandle));

    //Get Cliff Variation
    uint* pVariationMap = Editor.GetWorld().GetTileCliffVariationMap();
    int iVariation = (int)pVariationMap[Editor.GetWorld().GetTileCliff(iXC, iYC)];

    //Get Cliff tile size
    int iCliffSize = Editor.GetWorld().GetCliffSize();

    //declarations
    int iXX = 0;
    int iYY = 0;
    int iZZ = 0;
    CBBoxf box;
    uivector vresult;

    //Cliff Paths
    #define InnerCornerPathPerm CliffDefinition->GetInnerCorner().GetVariation(iVariation)->GetPiecePath()
    #define OuterCornerPathPerm CliffDefinition->GetOuterCorner().GetVariation(iVariation)->GetPiecePath()
    #define WedgePathPerm CliffDefinition->GetWedge().GetVariation(iVariation)->GetPiecePath()
    #define FrontPathPerm CliffDefinition->GetFront().GetVariation(iVariation)->GetPiecePath()
    #define InnerCornerPath256Perm CliffDefinition->GetInnerCorner256().GetVariation(iVariation)->GetPiecePath()
    #define OuterCornerPath256Perm CliffDefinition->GetOuterCorner256().GetVariation(iVariation)->GetPiecePath()
    #define WedgePath256Perm CliffDefinition->GetWedge256().GetVariation(iVariation)->GetPiecePath()
    #define FrontPath256Perm CliffDefinition->GetFront256().GetVariation(iVariation)->GetPiecePath()
    #define InnerCornerTransition1PathPerm CliffDefinition->GetInnerCornerTransition1().GetVariation(iVariation)->GetPiecePath()
    #define InnerCornerTransition2PathPerm CliffDefinition->GetInnerCornerTransition2().GetVariation(iVariation)->GetPiecePath()
    #define OuterCornerTransition1PathPerm CliffDefinition->GetOuterCornerTransition1().GetVariation(iVariation)->GetPiecePath()
    #define OuterCornerTransition2PathPerm CliffDefinition->GetOuterCornerTransition2().GetVariation(iVariation)->GetPiecePath()
    #define FrontTransition1PathPerm CliffDefinition->GetFrontTransition1().GetVariation(iVariation)->GetPiecePath()
    #define FrontTransition2PathPerm CliffDefinition->GetFrontTransition2().GetVariation(iVariation)->GetPiecePath()
    #define InnerCornerSimplePathPerm CliffDefinition->GetInnerCornerSimple().GetVariation(iVariation)->GetPiecePath()
    #define WedgeTransitionPathPerm CliffDefinition->GetWedgeTransition().GetVariation(iVariation)->GetPiecePath()
    #define WedgeShiftPathPerm CliffDefinition->GetWedgeShift().GetVariation(iVariation)->GetPiecePath()

    //Rotation vertexes
    #define ICrv CliffDefinition->GetInnerCorner().GetVariation(iVariation)->GetRotationVertex()
    #define OCrv CliffDefinition->GetOuterCorner().GetVariation(iVariation)->GetRotationVertex()
    #define Wrv CliffDefinition->GetWedge().GetVariation(iVariation)->GetRotationVertex()
    #define Frv CliffDefinition->GetFront().GetVariation(iVariation)->GetRotationVertex()
    #define IC256rv CliffDefinition->GetInnerCorner256().GetVariation(iVariation)->GetRotationVertex()
    #define OC256rv CliffDefinition->GetOuterCorner256().GetVariation(iVariation)->GetRotationVertex()
    #define W256rv CliffDefinition->GetWedge256().GetVariation(iVariation)->GetRotationVertex()
    #define F256rv CliffDefinition->GetFront256().GetVariation(iVariation)->GetRotationVertex()
    #define ICT1rv CliffDefinition->GetInnerCornerTransition1().GetVariation(iVariation)->GetRotationVertex()
    #define ICT2rv CliffDefinition->GetInnerCornerTransition2().GetVariation(iVariation)->GetRotationVertex()
    #define OCT1rv CliffDefinition->GetOuterCornerTransition1().GetVariation(iVariation)->GetRotationVertex()
    #define OCT2rv CliffDefinition->GetOuterCornerTransition2().GetVariation(iVariation)->GetRotationVertex()
    #define FT1rv CliffDefinition->GetFrontTransition1().GetVariation(iVariation)->GetRotationVertex()
    #define FT2rv CliffDefinition->GetFrontTransition2().GetVariation(iVariation)->GetRotationVertex()
    #define ICSrv CliffDefinition->GetInnerCornerSimple().GetVariation(iVariation)->GetRotationVertex()
    #define WTrv CliffDefinition->GetWedgeTransition().GetVariation(iVariation)->GetRotationVertex()
    #define WSrv CliffDefinition->GetWedgeShift().GetVariation(iVariation)->GetRotationVertex()

    //Default Rotation Offset
    #define ICdr CliffDefinition->GetInnerCorner().GetVariation(iVariation)->GetDefaultRotation()
    #define OCdr CliffDefinition->GetOuterCorner().GetVariation(iVariation)->GetDefaultRotation()
    #define Wdr CliffDefinition->GetWedge().GetVariation(iVariation)->GetDefaultRotation()
    #define Fdr CliffDefinition->GetFront().GetVariation(iVariation)->GetDefaultRotation()
    #define IC256dr CliffDefinition->GetInnerCorner256().GetVariation(iVariation)->GetDefaultRotation()
    #define OC256dr CliffDefinition->GetOuterCorner256().GetVariation(iVariation)->GetDefaultRotation()
    #define W256dr CliffDefinition->GetWedge256().GetVariation(iVariation)->GetDefaultRotation()
    #define F256dr CliffDefinition->GetFront256().GetVariation(iVariation)->GetDefaultRotation()
    #define ICT1dr CliffDefinition->GetInnerCornerTransition1().GetVariation(iVariation)->GetDefaultRotation()
    #define ICT2dr CliffDefinition->GetInnerCornerTransition2().GetVariation(iVariation)->GetDefaultRotation()
    #define OCT1dr CliffDefinition->GetOuterCornerTransition1().GetVariation(iVariation)->GetDefaultRotation()
    #define OCT2dr CliffDefinition->GetOuterCornerTransition2().GetVariation(iVariation)->GetDefaultRotation()
    #define FT1dr CliffDefinition->GetFrontTransition1().GetVariation(iVariation)->GetDefaultRotation()
    #define FT2dr CliffDefinition->GetFrontTransition2().GetVariation(iVariation)->GetDefaultRotation()
    #define ICSdr CliffDefinition->GetInnerCornerSimple().GetVariation(iVariation)->GetDefaultRotation()
    #define WTdr CliffDefinition->GetWedgeTransition().GetVariation(iVariation)->GetDefaultRotation()
    #define WSdr CliffDefinition->GetWedgeShift().GetVariation(iVariation)->GetDefaultRotation()

    //Find the lowest vert
    int lowestVert = 1;
    int lowestVertHeight = pVertCliffMap[Editor.GetWorld().GetVertCliff(iXC,iYC + 1)];

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

        byte * pRegion = new byte[256];
        for (int x(0); x < 256; x++)
        {
            if (highestVertHeight == 0 && lowestVertHeight == 0)
                pRegion[x] = 0;
            else
                pRegion[x] = 1;
        }

        CliffSquare.left = CLAMP(CliffSquare.left, 0, Editor.GetWorld().GetTileWidth() - 1);
        CliffSquare.right = CLAMP(CliffSquare.right, 1, Editor.GetWorld().GetTileWidth() - 1);
        CliffSquare.bottom = CLAMP(CliffSquare.bottom, 1, Editor.GetWorld().GetTileHeight() - 1);
        CliffSquare.top = CLAMP(CliffSquare.top, 0, Editor.GetWorld().GetTileHeight() - 1);

        if (!Editor.GetWorld().SetRegion(WORLD_TILE_CLIFF_MAP, CliffSquare, pRegion))
                EX_ERROR(_T("SetRegion failed"));

        // Notify the video drivers about the update
        for (int y(CliffSquare.top); y <= CliffSquare.bottom; ++y)
        {
            for (int x = CliffSquare.left; x <= CliffSquare.right; ++x)
            {
                Vid.Notify(VID_NOTIFY_TERRAIN_SHADER_MODIFIED, x, y, 0, &Editor.GetWorld());
                Vid.Notify(VID_NOTIFY_TERRAIN_NORMAL_MODIFIED, x, y, 0, &Editor.GetWorld());
            }
        }   

        //Prepare undo rect
        CRecti CliffSquareUndo(0, 0, iCliffSize, iCliffSize);
        CliffSquareUndo.MoveTo(iXC * iCliffSize,iYC * iCliffSize);

        CliffSquareUndo.left = CLAMP(CliffSquareUndo.left, 0, Editor.GetWorld().GetTileWidth() - 1);
        CliffSquareUndo.right = CLAMP(CliffSquareUndo.right, 1, Editor.GetWorld().GetTileWidth() - 1);
        CliffSquareUndo.bottom = CLAMP(CliffSquareUndo.bottom, 1, Editor.GetWorld().GetTileHeight() - 1);
        CliffSquareUndo.top = CLAMP(CliffSquareUndo.top, 0, Editor.GetWorld().GetTileHeight() - 1);

        CRecti heightRect(iXC * iCliffSize, iYC * iCliffSize, iXC * iCliffSize + iCliffSize + 1, iYC * iCliffSize + iCliffSize + 1);

        float * fpRegion = new float[heightRect.GetArea()];
        for (int x(0); x < heightRect.GetArea(); x++)
            fpRegion[x] = 0;

        //Adjust Height Map
        for (int x(0); x < heightRect.GetWidth(); x++)
        {
            for (int y(0); y < heightRect.GetHeight(); y++)
            {
                fpRegion[x + y * (heightRect.GetWidth())] = CalculateHeightVertex(heightRect.left + x, heightRect.top + y);
            }
        }

        Editor.GetWorld().SetRegion(WORLD_VERT_HEIGHT_MAP, heightRect, fpRegion);

        delete[] fpRegion;

        pRegion = new byte[128];
        for (int x(0); x < 128; x++)
            pRegion[x] = 0;

        //Blocker stuff
        CRecti blockerUndoSquare(iXC * iCliffSize, iYC * iCliffSize, iXC * iCliffSize + iCliffSize + 1, iYC * iCliffSize + iCliffSize + 1);
//      byte * pBlockerRegion;

        iXX = iXC * Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        iYY = iYC * Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        iZZ = lowestVertHeight * Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        
        CliffDelete(iXC, iYC);
        switch (cliffCombination) 
        {
        case 1111:
            if (!Editor.GetWorld().SetRegion(WORLD_TILE_CLIFF_MAP, CliffSquareUndo, pRegion))
                    EX_ERROR(_T("SetRegion failed"));
            break;
        case 1112:
            CliffCreate(iXX, iYY, iZZ, OC_ROTATION_TL + OCdr, OuterCornerPathPerm, OCrv);
            break;
        case 1113:
            CliffCreate(iXX, iYY, iZZ, OC_ROTATION_TL + OC256dr, OuterCornerPath256Perm, OC256rv);
            break;
        case 1121:
            CliffCreate(iXX, iYY, iZZ, OC_ROTATION_TR + OCdr, OuterCornerPathPerm, OCrv);
            break;
        case 1122:
            CliffCreate(iXX, iYY, iZZ, FRONT_ROTATION_B + Fdr, FrontPathPerm, Frv);
            break;
        case 1123:
            CliffCreate(iXX, iYY, iZZ, FT1_ROTATION_TL + FT1dr, FrontTransition1PathPerm, FT1rv); 
            break;
        case 1131:
            CliffCreate(iXX, iYY, iZZ, OC_ROTATION_TR + OC256dr, OuterCornerPath256Perm, OC256rv);
            break;
        case 1132: 
            CliffCreate(iXX, iYY, iZZ, FT2_ROTATION_TR + FT2dr, FrontTransition2PathPerm, FT2rv);
            break;
        case 1133:
            CliffCreate(iXX, iYY, iZZ, FRONT_ROTATION_B + F256dr, FrontPath256Perm, F256rv);
            break;
        case 1211:
            CliffCreate(iXX, iYY, iZZ, OC_ROTATION_BL + OCdr, OuterCornerPathPerm, OCrv);
            break;
        case 1212:
            CliffCreate(iXX, iYY, iZZ, FRONT_ROTATION_R + Fdr, FrontPathPerm, Frv);
            break;
        case 1213:
            CliffCreate(iXX, iYY, iZZ, FT2_ROTATION_TL + FT2dr, FrontTransition2PathPerm, FT2rv);
            break;
        case 1221:
            CliffCreate(iXX, iYY, iZZ, WEDGE_ROTATION_BR + Wdr, WedgePathPerm, Wrv);
            break;
        case 1222:
            CliffCreate(iXX, iYY, iZZ, IC_ROTATION_BR + ICdr, InnerCornerPathPerm, ICrv);
            break;
        case 1223:
            CliffCreate(iXX, iYY, iZZ, ICS_ROTATION_TL + ICSdr, InnerCornerSimplePathPerm, ICSrv);
            break;
        case 1231: 
            CliffCreate(iXX, iYY, iZZ, WT_ROTATION_TR + WTdr, WedgeTransitionPathPerm, WTrv);
            break;
        case 1232: 
            CliffCreate(iXX, iYY, iZZ, ICT2_ROTATION_TR + ICT2dr, InnerCornerTransition2PathPerm, ICT2rv);
            break;
        case 1233: 
            CliffCreate(iXX, iYY, iZZ, OCT2_ROTATION_TL + OCT2dr, OuterCornerTransition2PathPerm, OCT2rv);
            break;
        case 1311:
            CliffCreate(iXX, iYY, iZZ, OC_ROTATION_BL + OC256dr, OuterCornerPath256Perm, OC256rv);
            break;
        case 1312: 
            CliffCreate(iXX, iYY, iZZ, FT1_ROTATION_BL + FT1dr, FrontTransition1PathPerm, FT1rv);
            break;
        case 1313:
            CliffCreate(iXX, iYY, iZZ, FRONT_ROTATION_R + F256dr, FrontPath256Perm, F256rv);
            break;
        case 1321: 
            CliffCreate(iXX, iYY, iZZ, WT_ROTATION_BL + WTdr, WedgeTransitionPathPerm, WTrv);
            break;
        case 1322: 
            CliffCreate(iXX, iYY, iZZ, ICT1_ROTATION_BL + ICT1dr, InnerCornerTransition1PathPerm, ICT1rv);
            break;
        case 1323:
            CliffCreate(iXX, iYY, iZZ, OCT1_ROTATION_TL + OCT1dr, OuterCornerTransition1PathPerm, OCT1rv);
            break;
        case 1331:
            CliffCreate(iXX, iYY, iZZ, WEDGE_ROTATION_BR + W256dr, WedgePath256Perm, W256rv);
            break;
        case 1332: 
            CliffCreate(iXX, iYY, iZZ, WS_ROTATION_TL + WSdr, WedgeShiftPathPerm, WSrv);
            break;
        case 1333:
            CliffCreate(iXX, iYY, iZZ, IC_ROTATION_BR + IC256dr, InnerCornerPath256Perm, IC256rv);
            break;
        case 2111:
            CliffCreate(iXX, iYY, iZZ, OC_ROTATION_BR + OCdr, OuterCornerPathPerm, OCrv);
            break;
        case 2112:
            CliffCreate(iXX, iYY, iZZ, WEDGE_ROTATION_BL + Wdr, WedgePathPerm, Wrv);
            break;
        case 2113: 
            CliffCreate(iXX, iYY, iZZ, WT_ROTATION_TL + WTdr, WedgeTransitionPathPerm, WTrv);
            break;
        case 2121:
            CliffCreate(iXX, iYY, iZZ, FRONT_ROTATION_L + Fdr, FrontPathPerm, Frv);
            break;
        case 2122:
            CliffCreate(iXX, iYY, iZZ, IC_ROTATION_BL + ICdr, InnerCornerPathPerm, ICrv);
            break;
        case 2123: 
            CliffCreate(iXX, iYY, iZZ, ICT1_ROTATION_TL + ICT1dr, InnerCornerTransition1PathPerm, ICT1rv);
            break;
        case 2131: 
            CliffCreate(iXX, iYY, iZZ, FT1_ROTATION_TR + FT1dr, FrontTransition1PathPerm, FT1rv);
            break;
        case 2132: 
            CliffCreate(iXX, iYY, iZZ, ICS_ROTATION_TR + ICSdr, InnerCornerSimplePathPerm, ICSrv);
            break;
        case 2133: 
            CliffCreate(iXX, iYY, iZZ, OCT1_ROTATION_TR + OCT1dr, OuterCornerTransition1PathPerm, OCT1rv);
            break;
        case 2211:
            CliffCreate(iXX, iYY, iZZ, FRONT_ROTATION_T + Fdr, FrontPathPerm, Frv);
            break;
        case 2212:
            CliffCreate(iXX, iYY, iZZ, IC_ROTATION_TR + ICdr, InnerCornerPathPerm, ICrv);
            break;
        case 2213: 
            CliffCreate(iXX, iYY, iZZ, ICT2_ROTATION_TL + ICT2dr, InnerCornerTransition2PathPerm, ICT2rv);
            break;
        case 2221:
            CliffCreate(iXX, iYY, iZZ, IC_ROTATION_TL + ICdr, InnerCornerPathPerm, ICrv);
            break;
        case 2231:
            CliffCreate(iXX, iYY, iZZ, ICT1_ROTATION_TR + ICT1dr, InnerCornerTransition1PathPerm, ICT1rv);
            break;
        case 2311: 
            CliffCreate(iXX, iYY, iZZ, FT2_ROTATION_BL + FT2dr, FrontTransition2PathPerm, FT2rv);
            break;
        case 2312:
            CliffCreate(iXX, iYY, iZZ, ICS_ROTATION_BL + ICSdr, InnerCornerSimplePathPerm, ICSrv);
            break;
        case 2313: 
            CliffCreate(iXX, iYY, iZZ, OCT2_ROTATION_BL + OCT2dr, OuterCornerTransition2PathPerm, OCT2rv);
            break;
        case 2321:
            CliffCreate(iXX, iYY, iZZ, ICT2_ROTATION_BL + ICT2dr, InnerCornerTransition2PathPerm, ICT2rv);
            break;
        case 2331:
            CliffCreate(iXX, iYY, iZZ, WS_ROTATION_BR + WSdr, WedgeShiftPathPerm, WSrv);
            break;
        case 3111:
            CliffCreate(iXX, iYY, iZZ, OC_ROTATION_BR + OC256dr, OuterCornerPath256Perm, OC256rv);
            break;
        case 3112: 
            CliffCreate(iXX, iYY, iZZ, WT_ROTATION_BR + WTdr, WedgeTransitionPathPerm, WTrv);
            break;
        case 3113:
            CliffCreate(iXX, iYY, iZZ, WEDGE_ROTATION_BL + W256dr, WedgePath256Perm, W256rv);
            break;
        case 3121:
            CliffCreate(iXX, iYY, iZZ, FT2_ROTATION_BR + FT2dr, FrontTransition2PathPerm, FT2rv);
            break;
        case 3122:
            CliffCreate(iXX, iYY, iZZ, ICT2_ROTATION_BR + ICT2dr, InnerCornerTransition2PathPerm, ICT2rv);
            break;
        case 3123: 
            CliffCreate(iXX, iYY, iZZ, WS_ROTATION_TR + WSdr, WedgeShiftPathPerm, WSrv);
            break;
        case 3131:
            CliffCreate(iXX, iYY, iZZ, FRONT_ROTATION_L + F256dr, FrontPath256Perm, F256rv);
            break;
        case 3132: 
            CliffCreate(iXX, iYY, iZZ, OCT2_ROTATION_TR + OCT2dr, OuterCornerTransition2PathPerm, OCT2rv);
            break;
        case 3133:
            CliffCreate(iXX, iYY, iZZ, IC_ROTATION_BL + IC256dr, InnerCornerPath256Perm, IC256rv);
            break;
        case 3211:
            CliffCreate(iXX, iYY, iZZ, FT1_ROTATION_BR + FT1dr, FrontTransition1PathPerm, FT1rv);
            break;
        case 3212: 
            CliffCreate(iXX, iYY, iZZ, ICT1_ROTATION_BR + ICT1dr, InnerCornerTransition1PathPerm, ICT1rv);
            break;
        case 3213: 
            CliffCreate(iXX, iYY, iZZ, WS_ROTATION_BL + WSdr, WedgeShiftPathPerm, WSrv);
            break;
        case 3221: 
            CliffCreate(iXX, iYY, iZZ, ICS_ROTATION_BR + ICSdr, InnerCornerSimplePathPerm, ICSrv);
            break;
        case 3231: 
            CliffCreate(iXX, iYY, iZZ, OCT1_ROTATION_BR + OCT1dr, OuterCornerTransition1PathPerm, OCT1rv);
            break;
        case 3311: 
            CliffCreate(iXX, iYY, iZZ, FRONT_ROTATION_T + F256dr, FrontPath256Perm, F256rv);
            break;
        case 3312: 
            CliffCreate(iXX, iYY, iZZ, OCT1_ROTATION_BL + OCT1dr, OuterCornerTransition1PathPerm, OCT1rv);
            break;
        case 3313: 
            CliffCreate(iXX, iYY, iZZ, IC_ROTATION_TR + IC256dr, InnerCornerPath256Perm, IC256rv);
            break;
        case 3321: 
            CliffCreate(iXX, iYY, iZZ, OCT2_ROTATION_BR + OCT2dr, OuterCornerTransition2PathPerm, OCT2rv);
            break;
        case 3331: 
            CliffCreate(iXX, iYY, iZZ, IC_ROTATION_TL + IC256dr, InnerCornerPath256Perm, IC256rv);
            break;
        }
    delete[] pRegion;

    #undef InnerCornerPathPerm 
    #undef OuterCornerPathPerm 
    #undef WedgePathPerm 
    #undef FrontPathPerm 
    #undef InnerCornerPath256Perm 
    #undef OuterCornerPath256Perm 
    #undef WedgePath256Perm 
    #undef FrontPath256Perm 
    #undef InnerCornerTransition1PathPerm 
    #undef InnerCornerTransition2PathPerm 
    #undef OuterCornerTransition1PathPerm 
    #undef OuterCornerTransition2PathPerm 
    #undef FrontTransition1PathPerm 
    #undef FrontTransition2PathPerm 
    #undef InnerCornerSimplePathPerm 
    #undef WedgeTransitionPathPerm 
    #undef WedgeShiftPathPerm 

    #undef ICrv
    #undef OCrv
    #undef Wrv
    #undef Frv 
    #undef IC256rv
    #undef OC256rv 
    #undef W256rv
    #undef F256rv
    #undef ICT1rv 
    #undef ICT2rv
    #undef OCT1rv 
    #undef OCT2rv
    #undef FT1rv
    #undef FT2rv
    #undef ICSrv
    #undef WTrv
    #undef WSrv

    #undef ICdr
    #undef OCdr
    #undef Wdr
    #undef Fdr
    #undef IC256dr
    #undef OC256dr 
    #undef W256dr
    #undef F256dr 
    #undef ICT1dr 
    #undef ICT2dr 
    #undef OCT1dr 
    #undef OCT2dr 
    #undef FT1dr
    #undef FT2dr 
    #undef ICSdr 
    #undef WTdr 
    #undef WSdr
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
            vAdjustedPosition.y += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
    //  if (pEntity->GetAngles().z == 90)
    //      vAdjustedPosition.y -= Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        if (pEntity->GetAngles().z == 180)
//      {
            vAdjustedPosition.x += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
//          vAdjustedPosition.y -= Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
//      }
        if (pEntity->GetAngles().z == 270)
        {
            vAdjustedPosition.y += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
            vAdjustedPosition.x += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        }
    }

    if (iRotationVertex == 2)
    {
        if (pEntity->GetAngles().z == 90)
            vAdjustedPosition.y += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
    //  if (pEntity->GetAngles().z == 180)
    //      vAdjustedPosition.y -= Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        if (pEntity->GetAngles().z == 270)
    //  {
            vAdjustedPosition.x += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
    //      vAdjustedPosition.y -= Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
    //  }
        if (pEntity->GetAngles().z == 0)
        {
            vAdjustedPosition.y += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
            vAdjustedPosition.x += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        }
    }

    if (iRotationVertex == 3)
    {
        if (pEntity->GetAngles().z == 270)
            vAdjustedPosition.y += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
    //  if (pEntity->GetAngles().z == 0)
    //      vAdjustedPosition.y -= Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        if (pEntity->GetAngles().z == 90)
    //  {
            vAdjustedPosition.x += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
    //      vAdjustedPosition.y -= Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
    //  }
        if (pEntity->GetAngles().z == 180)
        {
            vAdjustedPosition.x += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
            vAdjustedPosition.y += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        }
    }

    if (iRotationVertex == 4)
    {
        if (pEntity->GetAngles().z == 180)
            vAdjustedPosition.y += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
    //  if (pEntity->GetAngles().z == 270)
    //      vAdjustedPosition.y -= Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        if (pEntity->GetAngles().z == 0)
    //  {
            vAdjustedPosition.x += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
    //      vAdjustedPosition.y -= Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
    //  }
        if (pEntity->GetAngles().z == 90)
        {
            vAdjustedPosition.x += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
            vAdjustedPosition.y += Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize();
        }
    }
    
        pEntity->SetPosition(vAdjustedPosition);
}


/*====================
  CCliffTool::CliffDelete
  ====================*/
void CCliffTool::CliffDelete(int iX, int iY)
{
    //Delete the old piece
    CBBoxf box;
    uivector vresult;
    box = CBBoxf(CVec3f(iX * Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize() + 1, iY * Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize() + 1, -FAR_AWAY),
                 CVec3f(iX * Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize() + 1, iY * Editor.GetWorld().GetScale() * Editor.GetWorld().GetCliffSize() + 1, FAR_AWAY));
    Editor.GetWorld().GetEntitiesInRegion(vresult, box, 0);
    for (uivector::iterator it = vresult.begin(); it < vresult.end(); it++)
    {
        if (Editor.GetWorld().GetEntity(*it)->GetType() == _T("Prop_Cliff"))
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
uint    CCliffTool::CliffCreate(float fX, float fY, float fz, float iRotation, tstring tModel, int iRotationVertex)
{
    while (iRotation >= 360)
        iRotation -= 360;
    try
    {
        uint uiNewEntity(Editor.GetWorld().AllocateNewEntity());
        CWorldEntity *pNewEntity(Editor.GetWorld().GetEntity(uiNewEntity, true));

        pNewEntity->SetAngles(CVec3f(0,0,iRotation));
        pNewEntity->SetPosition(fX, fY, fz);
        pNewEntity->SetModelHandle(g_ResourceManager.Register(tModel, RES_MODEL));
        pNewEntity->SetModelPath(tModel);
        pNewEntity->SetType(_T("Prop_Cliff"));
        pNewEntity->SetScale(Editor.GetWorld().GetScale() / 32);
        
        Editor.GetWorld().LinkEntity(uiNewEntity, LINK_SURFACE | LINK_MODEL, SURF_PROP);
        
        // Adjust location to compensate for yaw rotation
        RotationAdjustment(uiNewEntity, iRotationVertex);
        //vAdjustedPosition.z = Editor.GetWorld().GetTerrainHeight(vAdjustedPosition.x, vAdjustedPosition.y);

        Editor.GetWorld().LinkEntity(uiNewEntity, LINK_SURFACE | LINK_MODEL, SURF_PROP);

        return uiNewEntity;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CCliffTool::CliffCreate() - "), NO_THROW);
        return INVALID_INDEX;
    }
}


/*====================
  CCliffTool::CalculateHeightVertex
  ====================*/
float   CCliffTool::CalculateHeightVertex(int iX, int iY)
{
    // Get Height Map
    //float* pHeightMap = Editor.GetWorld().GetHeightMap();

    //Get Cliff Map
    int* pVertCliffMap = Editor.GetWorld().GetVertCliffMap();

    int iCliffSize = Editor.GetWorld().GetCliffSize();

    int iXC = -1;
    int iXC2 = -1;
    int iYC = -1;
    int iYC2 = -1;
    if (iX % iCliffSize < iCliffSize / 2)
        iXC = iX / iCliffSize;
    if (iX % iCliffSize == iCliffSize / 2)
    {
        iXC = iX / iCliffSize;
        iXC2 = iX / iCliffSize + 1;
    }
    if (iX % iCliffSize > iCliffSize / 2)
        iXC = iX / iCliffSize + 1;
    if (iY % iCliffSize < iCliffSize / 2)
        iYC = iY / iCliffSize;
    if (iY % iCliffSize == iCliffSize / 2)
    {
        iYC = iY / iCliffSize;
        iYC2 = iY / iCliffSize + 1;
    }
    if (iY % iCliffSize > iCliffSize / 2)
        iYC = iY / iCliffSize + 1;

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
    // Get Cliff Size
    int iCliffSize = Editor.GetWorld().GetCliffSize();

    // Get Cliff Map
    int* pVertCliffMap = Editor.GetWorld().GetVertCliffMap();

    // First Pass: Removing Blockers
    for (int x(MAX(scanArea.left - 1, 0)); x < MIN(scanArea.right + 1, Editor.GetWorld().GetCliffGridWidth()); x++)
    {
        for (int y(MAX(scanArea.top - 1, 0)); y < MIN(scanArea.bottom + 1, Editor.GetWorld().GetCliffGridHeight()); y++)
        {
            //Get Cliff Height
            int iCliffHeight = pVertCliffMap[Editor.GetWorld().GetVertCliff(x,y)];
            
            if (pVertCliffMap[Editor.GetWorld().GetVertCliff(x + 1, y)] == iCliffHeight &&
                pVertCliffMap[Editor.GetWorld().GetVertCliff(x + 1, y + 1)] == iCliffHeight &&
                pVertCliffMap[Editor.GetWorld().GetVertCliff(x, y + 1)] == iCliffHeight)
            {
                byte * pBlockerRegion = new byte[256];
                for (int i(0); i < 256; i++)
                    pBlockerRegion[i] = 0;

                CRecti blockerSquare(x * iCliffSize, y * iCliffSize, x * iCliffSize + iCliffSize + 1, y * iCliffSize + iCliffSize + 1);
                blockerSquare.left = CLAMP(blockerSquare.left, 0, Editor.GetWorld().GetGridWidth() - 1);
                blockerSquare.right = CLAMP(blockerSquare.right, 0, Editor.GetWorld().GetGridWidth() - 1);
                blockerSquare.bottom = CLAMP(blockerSquare.bottom, 0, Editor.GetWorld().GetGridHeight() - 1);
                blockerSquare.top = CLAMP(blockerSquare.top, 0, Editor.GetWorld().GetGridHeight() - 1);

                if (!Editor.GetWorld().SetRegion(WORLD_VERT_BLOCKER_MAP, blockerSquare, pBlockerRegion))
                    EX_ERROR(_T("SetRegion failed"));   
                delete[] pBlockerRegion;
            }
        }
    }

    // Second Pass: Adding blockers
    for (int x(MAX(scanArea.left - 2, 0)); x < MIN(scanArea.right + 2, Editor.GetWorld().GetCliffGridWidth()); x++)
    {
        for (int y(MAX(scanArea.top - 2, 0)); y < MIN(scanArea.bottom + 2, Editor.GetWorld().GetCliffGridHeight()); y++)
        {
            //Get Cliff Height
            int iCliffHeight = pVertCliffMap[Editor.GetWorld().GetVertCliff(x, y)];

            if(!(pVertCliffMap[Editor.GetWorld().GetVertCliff(x + 1, y)] == iCliffHeight &&
               pVertCliffMap[Editor.GetWorld().GetVertCliff(x + 1, y + 1)] == iCliffHeight &&
               pVertCliffMap[Editor.GetWorld().GetVertCliff(x, y + 1)] == iCliffHeight))
            {
                byte * pBlockerRegion = new byte[256];
                for (int i(0); i < 256; i++)
                    pBlockerRegion[i] = 1;
                
                CRecti blockerSquare(x * iCliffSize, y * iCliffSize, x * iCliffSize + iCliffSize + 1, y * iCliffSize + iCliffSize + 1);
                blockerSquare.left = CLAMP(blockerSquare.left, 0, Editor.GetWorld().GetGridWidth() - 1);
                blockerSquare.right = CLAMP(blockerSquare.right, 0, Editor.GetWorld().GetGridWidth() - 1);
                blockerSquare.bottom = CLAMP(blockerSquare.bottom, 0, Editor.GetWorld().GetGridHeight() - 1);
                blockerSquare.top = CLAMP(blockerSquare.top, 0, Editor.GetWorld().GetGridHeight() - 1);

                if (!Editor.GetWorld().SetRegion(WORLD_VERT_BLOCKER_MAP, blockerSquare, pBlockerRegion))
                    EX_ERROR(_T("SetRegion failed"));   
                delete[] pBlockerRegion;
            }
        }   
    }   
}


/*====================
  CCliffTool::SaveTempCliffMap
  ====================*/
void    CCliffTool::SaveTempCliffMap()
{
    if (m_pTempCliffMap == NULL) // HACK: Resize when map changes
        m_pTempCliffMap = K2_NEW_ARRAY(global, int, Editor.GetWorld().GetCliffGridWidth() * Editor.GetWorld().GetCliffGridHeight());

    int *pVertCliffMap(Editor.GetWorld().GetVertCliffMap());
    memcpy(m_pTempCliffMap, pVertCliffMap, sizeof(int) * Editor.GetWorld().GetCliffGridWidth() * Editor.GetWorld().GetCliffGridHeight());
}


/*====================
  CCliffTool::CliffMapRandom
  ====================*/
void    CCliffTool::CliffMapRandom()
{
    CRecti recArea(0, 0, Editor.GetWorld().GetCliffGridWidth(), Editor.GetWorld().GetCliffGridHeight());

    // Get the region
    int *pRegion = new int[recArea.GetArea()];
    Editor.GetWorld().GetRegion(WORLD_VERT_CLIFF_MAP, recArea, pRegion);

    int iRegionIndex(0);
    for (int y(0); y < recArea.GetHeight(); ++y)
    {
        for (int x(0); x < recArea.GetWidth(); ++x)
        {
            pRegion[iRegionIndex] = M_Randnum(0, 2);
            ++iRegionIndex;
        }
    }

    Editor.GetWorld().SetRegion(WORLD_VERT_CLIFF_MAP, recArea, pRegion);
    
    delete[] pRegion;
}


/*====================
  CCliffTool::CliffRandom
  ====================*/
void    CCliffTool::CliffRandom()
{
    CliffMapRandom();

    DefinitionMapSet();

    VariationMapSet();

    CRecti modifiedArea(0, 0, Editor.GetWorld().GetCliffGridWidth(), Editor.GetWorld().GetCliffGridHeight());

    CRecti enforcedArea = modifiedArea;

    // Enforce Height
    for (int y(modifiedArea.top); y <= modifiedArea.bottom; y++)
    {
        for (int x(modifiedArea.left); x <= modifiedArea.right; x++)
        {
            EnforceHeight(x,y, enforcedArea);
        }
    }

    // Calculate Tiles
    for (int x(enforcedArea.left - 1); x <= enforcedArea.right + 1; x++)
    {
        for (int y(enforcedArea.top - 1); y <= enforcedArea.bottom + 1; y++)
        {
            CalculateTile(x,y);
        }
    }

    // Calculate Blockers
    CalculateCliffBlockers(enforcedArea);
}


/*--------------------
  cmdSetCliffMode
  --------------------*/
UI_VOID_CMD(SetCliffMode, 1)
{
    if (vArgList.size() < 1)
    {
        Console << _T("syntax: SetCliffMode create|select") << newl;
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
    else if (sValue == _T("definition"))
    {
        le_cliffMode = CLIFF_DEFINITION;
        CliffMode.Trigger(_T("Cliff"));
        return;
    }
    else if (sValue == _T("variation"))
    {
        le_cliffMode = CLIFF_VARIATION;
        CliffMode.Trigger(_T("Variation"));
        return;
    }

    return;
}


/*--------------------
  CliffRandom
  --------------------*/
CMD(CliffRandom)
{
    GET_TOOL(Cliff, CLIFF)->CliffRandom();
    return true;
}