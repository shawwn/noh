// (C)2008 S2 Games
// c_visblockertool.cpp
//
// VisBlocker tool
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "editor_common.h"

#include "editor.h"

#include "c_visblockertool.h"

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
CVAR_BOOL   (le_visblockerDrawBrushInfluence,       true);
CVAR_FLOAT  (le_visblockerBrushInfluenceAlpha,      1.0f);

UI_TRIGGER(VisBlockerMode);
//=============================================================================

/*====================
  CVisBlockerTool::CVisBlockerTool
  ====================*/
CVisBlockerTool::CVisBlockerTool() :
ITool(TOOL_VISBLOCKER, _T("visblocker")),
m_bWorking(false),
m_hLineMaterial(g_ResourceManager.Register(_T("/core/materials/line.material"), RES_MATERIAL)),
m_hVisBlockerMaterial(g_ResourceManager.Register(_T("/core/materials/visblocker.material"), RES_MATERIAL)),
m_hFont(g_ResourceManager.LookUpName(_T("system_medium"), RES_FONTMAP))
{
    VisBlockerMode.Trigger(_T("Modify"));
}


/*====================
  CVisBlockerTool::PrimaryUp

  Left mouse button up action
  ====================*/
void    CVisBlockerTool::PrimaryUp()
{
    if (!m_bInverse)
        m_bWorking = false;
}


/*====================
  CVisBlockerTool::PrimaryDown

  Default left mouse button down action
  ====================*/
void    CVisBlockerTool::PrimaryDown()
{
    m_bWorking = true;
    m_bInverse = false;
}


/*====================
  CVisBlockerTool::SecondaryUp

  Right mouse button up action
  ====================*/
void    CVisBlockerTool::SecondaryUp()
{
    if (m_bInverse)
        m_bWorking = false;
}


/*====================
  CVisBlockerTool::SecondaryDown

  Default right mouse button down action - not used in this case
  ====================*/
void    CVisBlockerTool::SecondaryDown()
{
    CalcToolProperties();
    m_bWorking = true;
    m_bInverse = true;
}


/*====================
  CVisBlockerTool::TertiaryUp

  Middle mouse button up action
  ====================*/
void    CVisBlockerTool::TertiaryUp() {}


/*====================
  CVisBlockerTool::TertiaryDown

  Middle mouse button down action
  ====================*/
void    CVisBlockerTool::TertiaryDown() {}


/*====================
  CVisBlockerTool::QuaternaryUp

  Scroll wheel up action
  ====================*/
void    CVisBlockerTool::QuaternaryUp() {}


/*====================
  CVisBlockerTool::QuaternaryDown

  Scroll wheel down action
  ====================*/
void    CVisBlockerTool::QuaternaryDown() {}


/*====================
  CVisBlockerTool::Cancel
  ====================*/
void    CVisBlockerTool::Cancel() {}


/*====================
  CVisBlockerTool::Delete
  ====================*/
void    CVisBlockerTool::Delete() {}



/*====================
  CVisBlockerTool::CalcToolProperties
  ====================*/
void     CVisBlockerTool::CalcToolProperties()
{
    STraceInfo trace;

    if (Editor.TraceCursor(trace, TRACE_TERRAIN))
    {
        m_iX = Editor.GetWorld().GetTileFromCoord(trace.v3EndPos[X]);
        m_iY = Editor.GetWorld().GetTileFromCoord(trace.v3EndPos[Y]);
        m_v3EndPos = trace.v3EndPos;
    }
    else
    {
        m_iX = -1;
        m_iY = -1;
        m_v3EndPos.Clear();
    }
}


/*====================
  CVisBlockerTool::VisBlockerModify
  ====================*/
void    CVisBlockerTool::VisBlockerModify(byte *pRegion, const CRecti &recArea, const CBrush &brush, bool bAdd)
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
  CVisBlockerTool::PaintVisBlocker
  ====================*/
void    CVisBlockerTool::PaintVisBlocker(float fFrameTime)
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

        if (!Editor.GetWorld().GetRegion(WORLD_TILE_VISBLOCKER_MAP, recClippedBrush, pRegion))
            EX_ERROR(_T("Failed to retrieve region"));

        // Perform the operation
        recClippedBrush.Shift(-(m_iX - pBrush->GetBrushSize() / 2), -(m_iY - pBrush->GetBrushSize() / 2));

        VisBlockerModify(pRegion, recClippedBrush, *pBrush, !m_bInverse);

        // Apply the modified region
        recClippedBrush.Shift(m_iX - pBrush->GetBrushSize() / 2, m_iY - pBrush->GetBrushSize() / 2);
        if (!Editor.GetWorld().SetRegion(WORLD_TILE_VISBLOCKER_MAP, recClippedBrush, pRegion))
            EX_ERROR(_T("SetRegion failed"));

        delete[] pRegion;
    }
    catch (CException &ex)
    {
        if (pRegion != NULL)
            delete[] pRegion;

        ex.Process(_T("CVisBlockerTool::PaintVisBlocker() - "), NO_THROW);
    }
}


/*====================
  CVisBlockerTool::Frame
 ====================*/
void    CVisBlockerTool::Frame(float fFrameTime)
{
    CalcToolProperties();

    if (m_bWorking && m_iX != -1 && m_iY != -1)
        PaintVisBlocker(fFrameTime);
}


/*====================
  CVisBlockerTool::Draw
  ====================*/
void    CVisBlockerTool::Draw()
{
    if (le_visblockerDrawBrushInfluence)
    {
        CFontMap *pFontMap(g_ResourceManager.GetFontMap(m_hFont));

        Draw2D.SetColor(0.0f, 0.0f, 0.0f);
        Draw2D.String(4.0f, Draw2D.GetScreenH() - pFontMap->GetMaxHeight() - 1.0f, Draw2D.GetScreenW(), pFontMap->GetMaxHeight(), XtoA(m_v3EndPos), m_hFont);
        Draw2D.SetColor(1.0f, 1.0f, 1.0f);
        Draw2D.String(3.0f, Draw2D.GetScreenH() - pFontMap->GetMaxHeight() - 2.0f, Draw2D.GetScreenW(), pFontMap->GetMaxHeight(), XtoA(m_v3EndPos), m_hFont);
    }
}


/*====================
  CVisBlockerTool::Render
  ====================*/
void    CVisBlockerTool::Render()
{
    int iBeginX(0);
    int iBeginY(0);
    int iEndX(Editor.GetWorld().GetTileWidth());
    int iEndY(Editor.GetWorld().GetTileHeight());

    float fTileSize(Editor.GetWorld().GetScale());
    int iGridWidth(Editor.GetWorld().GetGridWidth());
    int iTileWidth(Editor.GetWorld().GetTileWidth());

    float *pHeightMap(Editor.GetWorld().GetHeightMap());
    byte *pSplitMap(Editor.GetWorld().GetSplitMap());
    byte *pVisBlockerMap(Editor.GetWorld().GetVisBlockerMap());

    int iGridIndex(iBeginY * iGridWidth + iBeginX);
    int iGridSpan(iGridWidth - (iEndX - iBeginX));

    int iTileIndex(iBeginY * iTileWidth + iBeginX);
    int iTileSpan(iTileWidth - (iEndX - iBeginX));

    CVec3f  v1(0.0f, 0.0f, 0.0f);
    CVec3f  v2(0.0f, iBeginY * fTileSize, 0.0f);
    CVec3f  v3(0.0f, 0.0f, 0.0f);
    CVec3f  v4(0.0f, iBeginY * fTileSize, 0.0f);

    SSceneFaceVert poly[1024];
    MemManager.Set(poly, 0, sizeof(poly));
    int p(0);

    for (int iY(iBeginY); iY < iEndY; ++iY, iGridIndex += iGridSpan - 1, iTileIndex += iTileSpan)
    {
        // Reset X values
        v4.x = v3.x = iBeginX * fTileSize;

        // Shift Y values
        v3.y = v1.y = v2.y;
        v4.y = v2.y += fTileSize;

        // New Z Values
        v3.z = pHeightMap[iGridIndex];
        v4.z = pHeightMap[iGridIndex + iGridWidth];

        ++iGridIndex;

        for (int iX(iBeginX); iX < iEndX; ++iX, ++iGridIndex, ++iTileIndex)
        {
            // Shift X values
            v1.x = v2.x = v3.x;
            v4.x = v3.x += fTileSize;
    
            // Shift Z Vavues
            v1.z = v3.z;
            v2.z = v4.z;

            // New Z values
            v3.z = pHeightMap[iGridIndex];
            v4.z = pHeightMap[iGridIndex + iGridWidth];

            if (pVisBlockerMap[iTileIndex])
            {
                if (p >= 1000) // restart batch if we overflow
                {
                    SceneManager.AddPoly(p, poly, m_hVisBlockerMaterial, POLY_TRILIST | POLY_NO_DEPTH_TEST);
                    MemManager.Set(poly, 0, sizeof(poly));
                    p = 0;
                }

                if (pSplitMap[iTileIndex] == SPLIT_NEG)
                {
                    // Left triangle
                    poly[p].vtx = v1;
                    SET_VEC4(poly[p].col, 255, 255, 0, 128);
                    ++p;

                    poly[p].vtx = v3;
                    SET_VEC4(poly[p].col, 255, 255, 0, 128);
                    ++p;

                    poly[p].vtx = v2;
                    SET_VEC4(poly[p].col, 255, 255, 0, 128);
                    ++p;

                    // Right triangle
                    poly[p].vtx = v2;
                    SET_VEC4(poly[p].col, 255, 255, 0, 128);
                    ++p;

                    poly[p].vtx = v3;
                    SET_VEC4(poly[p].col, 255, 255, 0, 128);
                    ++p;

                    poly[p].vtx = v4;
                    SET_VEC4(poly[p].col, 255, 255, 0, 128);
                    ++p;
                }
                else
                {
                    // Left triangle
                    poly[p].vtx = v1;
                    SET_VEC4(poly[p].col, 255, 255, 0, 128);
                    ++p;

                    poly[p].vtx = v4;
                    SET_VEC4(poly[p].col, 255, 255, 0, 128);
                    ++p;

                    poly[p].vtx = v2;
                    SET_VEC4(poly[p].col, 255, 255, 0, 128);
                    ++p;

                    // Right triangle
                    poly[p].vtx = v1;
                    SET_VEC4(poly[p].col, 255, 255, 0, 128);
                    ++p;

                    poly[p].vtx = v3;
                    SET_VEC4(poly[p].col, 255, 255, 0, 128);
                    ++p;

                    poly[p].vtx = v4;
                    SET_VEC4(poly[p].col, 255, 255, 0, 128);
                    ++p;
                }
            }
        }
    }

    if (p > 0)
    {
        SceneManager.AddPoly(p, poly, m_hVisBlockerMaterial, POLY_TRILIST | POLY_NO_DEPTH_TEST);
        MemManager.Set(poly, 0, sizeof(poly));
    }

    p = 0;

    CBrush *pBrush = CBrush::GetCurrentBrush();

    if (!pBrush)
        return;

    int iX = m_iX, iY = m_iY;

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

