// (C)2007 S2 Games
// c_blockertool.cpp
//
// Terrain blocker
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "editor_common.h"

#include "editor.h"

#include "c_blockertool.h"

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
CVAR_BOOL	(le_blockerDrawBrushInfluence,		true);
CVAR_FLOAT	(le_blockerBrushInfluenceAlpha,		1.0f);

UI_TRIGGER(BlockerMode);
//=============================================================================

/*====================
  CBlockerTool::CBlockerTool
  ====================*/
CBlockerTool::CBlockerTool() :
ITool(TOOL_BLOCKER, _T("blocker")),
m_bWorking(false),
m_hLineMaterial(g_ResourceManager.Register(_T("/core/materials/line.material"), RES_MATERIAL)),
m_hBlockerMaterial(g_ResourceManager.Register(_T("/core/materials/blocker.material"), RES_MATERIAL)),
m_hFont(g_ResourceManager.LookUpName(_T("system_medium"), RES_FONTMAP))
{
	BlockerMode.Trigger(_T("Modify"));
}


/*====================
  CBlockerTool::PrimaryUp

  Left mouse button up action
  ====================*/
void	CBlockerTool::PrimaryUp()
{
	if (!m_bInverse)
		m_bWorking = false;
}


/*====================
  CBlockerTool::PrimaryDown

  Default left mouse button down action
  ====================*/
void	CBlockerTool::PrimaryDown()
{
	m_bWorking = true;
	m_bInverse = false;
}


/*====================
  CBlockerTool::SecondaryUp

  Right mouse button up action
  ====================*/
void	CBlockerTool::SecondaryUp()
{
	if (m_bInverse)
		m_bWorking = false;
}


/*====================
  CBlockerTool::SecondaryDown

  Default right mouse button down action - not used in this case
  ====================*/
void	CBlockerTool::SecondaryDown()
{
	CalcToolProperties();
	m_bWorking = true;
	m_bInverse = true;
}


/*====================
  CBlockerTool::TertiaryUp

  Middle mouse button up action
  ====================*/
void	CBlockerTool::TertiaryUp() {}


/*====================
  CBlockerTool::TertiaryDown

  Middle mouse button down action
  ====================*/
void	CBlockerTool::TertiaryDown() {}


/*====================
  CBlockerTool::QuaternaryUp

  Scroll wheel up action
  ====================*/
void	CBlockerTool::QuaternaryUp() {}


/*====================
  CBlockerTool::QuaternaryDown

  Scroll wheel down action
  ====================*/
void	CBlockerTool::QuaternaryDown() {}


/*====================
  CBlockerTool::Cancel
  ====================*/
void	CBlockerTool::Cancel() {}


/*====================
  CBlockerTool::Delete
  ====================*/
void	CBlockerTool::Delete() {}



/*====================
  CBlockerTool::CalcToolProperties
  ====================*/
void	 CBlockerTool::CalcToolProperties()
{
	STraceInfo trace;

	if (Editor.TraceCursor(trace, TRACE_TERRAIN))
	{
		m_iX = Editor.GetWorld().GetVertFromCoord(trace.v3EndPos[X]);
		m_iY = Editor.GetWorld().GetVertFromCoord(trace.v3EndPos[Y]);
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
  CBlockerTool::BlockerModify
  ====================*/
void	CBlockerTool::BlockerModify(byte *pRegion, const CRecti &recArea, const CBrush &brush, bool bAdd)
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
  CBlockerTool::BlockerTerrain
  ====================*/
void	CBlockerTool::BlockerTerrain(float fFrameTime)
{
	byte *pRegion(NULL);

	try
	{
		CBrush *pBrush(CBrush::GetCurrentBrush());
		if (pBrush == NULL)
			EX_ERROR(_T("No brush selected"));

		if (!Editor.GetWorld().IsInBounds(m_iX, m_iY, GRID_SPACE))
			EX_WARN(_T("Out of bounds coordinate"));

		// Clip against the brush data
		CRecti	recClippedBrush;
		if (!pBrush->ClipBrush(recClippedBrush))
			return;

		// Clip the brush against the world
		recClippedBrush.Shift(m_iX - pBrush->GetBrushSize() / 2, m_iY - pBrush->GetBrushSize() / 2);
		if (!Editor.GetWorld().ClipRect(recClippedBrush, GRID_SPACE))
			return;

		// Get the region
		pRegion = K2_NEW_ARRAY(global, byte, recClippedBrush.GetArea());
		if (pRegion == NULL)
			EX_ERROR(_T("Failed to allocate region"));

		if (!Editor.GetWorld().GetRegion(WORLD_VERT_BLOCKER_MAP, recClippedBrush, pRegion))
			EX_ERROR(_T("Failed to retrieve region"));

		// Perform the operation
		recClippedBrush.Shift(-(m_iX - pBrush->GetBrushSize() / 2), -(m_iY - pBrush->GetBrushSize() / 2));

		BlockerModify(pRegion, recClippedBrush, *pBrush, !m_bInverse);

		// Apply the modified region
		recClippedBrush.Shift(m_iX - pBrush->GetBrushSize() / 2, m_iY - pBrush->GetBrushSize() / 2);
		if (!Editor.GetWorld().SetRegion(WORLD_VERT_BLOCKER_MAP, recClippedBrush, pRegion))
			EX_ERROR(_T("SetRegion failed"));

		K2_DELETE_ARRAY(pRegion);
	}
	catch (CException &ex)
	{
		if (pRegion != NULL)
			K2_DELETE_ARRAY(pRegion);

		ex.Process(_T("CBlockerTool::BlockerTerrain() - "), NO_THROW);
	}
}


/*====================
  CBlockerTool::Frame
 ====================*/
void	CBlockerTool::Frame(float fFrameTime)
{
	CalcToolProperties();

	if (m_bWorking && m_iX != -1 && m_iY != -1)
		BlockerTerrain(fFrameTime);
}


/*====================
  CBlockerTool::Draw
  ====================*/
void	CBlockerTool::Draw()
{
	if (le_blockerDrawBrushInfluence)
	{
		CFontMap *pFontMap(g_ResourceManager.GetFontMap(m_hFont));

		Draw2D.SetColor(0.0f, 0.0f, 0.0f);
		Draw2D.String(4.0f, Draw2D.GetScreenH() - pFontMap->GetMaxHeight() - 1.0f, Draw2D.GetScreenW(), pFontMap->GetMaxHeight(), XtoA(m_v3EndPos), m_hFont);
		Draw2D.SetColor(1.0f, 1.0f, 1.0f);
		Draw2D.String(3.0f, Draw2D.GetScreenH() - pFontMap->GetMaxHeight() - 2.0f, Draw2D.GetScreenW(), pFontMap->GetMaxHeight(), XtoA(m_v3EndPos), m_hFont);
	}
}


/*====================
  CBlockerTool::Render
  ====================*/
void	CBlockerTool::Render()
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
	byte *pBlockerMap(Editor.GetWorld().GetBlockerMap());

	int iIndex(iBeginY * iGridWidth + iBeginX);
	int iSpan(iGridWidth - (iEndX - iBeginX));

	CVec3f	v1(0.0f, 0.0f, 0.0f);
	CVec3f	v2(0.0f, iBeginY * fTileSize, 0.0f);
	CVec3f	v3(0.0f, 0.0f, 0.0f);
	CVec3f	v4(0.0f, iBeginY * fTileSize, 0.0f);

	byte yBlocker1(0);
	byte yBlocker2(0);
	byte yBlocker3(0);
	byte yBlocker4(0);

	SSceneFaceVert poly[1024];
	MemManager.Set(poly, 0, sizeof(poly));
	int p(0);

	for (int iY(iBeginY); iY < iEndY; ++iY, iIndex += iSpan - 1)
	{
		// Reset X values
		v4.x = v3.x = iBeginX * fTileSize;

		// Shift Y values
		v3.y = v1.y = v2.y;
		v4.y = v2.y += fTileSize;

		// New Z Values
		v3.z = pHeightMap[iIndex];
		v4.z = pHeightMap[iIndex + iGridWidth];

		// New Blocker Values
		yBlocker3 = pBlockerMap[iIndex];
		yBlocker4 = pBlockerMap[iIndex + iGridWidth];

		++iIndex;

		for (int iX(iBeginX); iX < iEndX; ++iX, ++iIndex)
		{
			// Shift X values
			v1.x = v2.x = v3.x;
			v4.x = v3.x += fTileSize;
	
			// Shift Z Vavues
			v1.z = v3.z;
			v2.z = v4.z;

			// New Z values
			v3.z = pHeightMap[iIndex];
			v4.z = pHeightMap[iIndex + iGridWidth];

			// Shift Blocker Values
			yBlocker1 = yBlocker3;
			yBlocker2 = yBlocker4;

			// New Blocker values
			yBlocker3 = pBlockerMap[iIndex];
			yBlocker4 = pBlockerMap[iIndex + iGridWidth];


			if (pSplitMap[iY * iTileWidth + iX] == SPLIT_NEG)
			{
				if (yBlocker1 & yBlocker2 & yBlocker3)
				{
					poly[p].vtx = v1;
					SET_VEC4(poly[p].col, 255, 0, 0, 128);
					++p;

					poly[p].vtx = v3;
					SET_VEC4(poly[p].col, 255, 0, 0, 128);
					++p;

					poly[p].vtx = v2;
					SET_VEC4(poly[p].col, 255, 0, 0, 128);
					++p;
				}

				if (yBlocker2 & yBlocker3 & yBlocker4)
				{
					poly[p].vtx = v2;
					SET_VEC4(poly[p].col, 255, 0, 0, 128);
					++p;

					poly[p].vtx = v3;
					SET_VEC4(poly[p].col, 255, 0, 0, 128);
					++p;

					poly[p].vtx = v4;
					SET_VEC4(poly[p].col, 255, 0, 0, 128);
					++p;
				}
			}
			else
			{
				if (yBlocker1 & yBlocker2 & yBlocker4)
				{
					poly[p].vtx = v1;
					SET_VEC4(poly[p].col, 255, 0, 0, 128);
					++p;

					poly[p].vtx = v4;
					SET_VEC4(poly[p].col, 255, 0, 0, 128);
					++p;

					poly[p].vtx = v2;
					SET_VEC4(poly[p].col, 255, 0, 0, 128);
					++p;
				}

				if (yBlocker1 & yBlocker3 & yBlocker4)
				{
					poly[p].vtx = v1;
					SET_VEC4(poly[p].col, 255, 0, 0, 128);
					++p;

					poly[p].vtx = v3;
					SET_VEC4(poly[p].col, 255, 0, 0, 128);
					++p;

					poly[p].vtx = v4;
					SET_VEC4(poly[p].col, 255, 0, 0, 128);
					++p;
				}
			}

			if (p >= 1020) // restart batch if we overflow
			{
				SceneManager.AddPoly(p, poly, m_hBlockerMaterial, POLY_TRILIST);
				MemManager.Set(poly, 0, sizeof(poly));
				p = 0;
			}
		}
	}

	if (p > 0)
		SceneManager.AddPoly(p, poly, m_hBlockerMaterial, POLY_TRILIST);

	CBrush *pBrush(CBrush::GetCurrentBrush());

	if (!le_blockerDrawBrushInfluence || !pBrush || m_iX < 0 || m_iY < 0)
		return;

	int iX = m_iX, iY = m_iY;

	p = 0;

	for (int y = 0; y < pBrush->GetBrushSize() - 1; ++y)
	{
		for (int x = 0; x < pBrush->GetBrushSize() - 1; ++x)
		{
			if (p >= 1024) // restart batch if we overflow
			{
				SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
				MemManager.Set(poly, 0, sizeof(poly));
				p = 0;
			}

			int i = x + y * pBrush->GetBrushSize();
			int dX = iX + x - pBrush->GetBrushSize() / 2;
			int dY = iY + y - pBrush->GetBrushSize() / 2;
			if (!Editor.GetWorld().IsInBounds(dX, dY, GRID_SPACE))
				continue;

			// left
			if (dY < Editor.GetWorld().GetGridHeight() - 1)
			{
				byte alpha0 = INT_FLOOR((*pBrush)[i] * le_blockerBrushInfluenceAlpha);
				byte alpha1 = INT_FLOOR((*pBrush)[i + pBrush->GetBrushSize()] * le_blockerBrushInfluenceAlpha);

				if (alpha0 || alpha1)
				{
					poly[p].vtx[0] = dX * fTileSize;
					poly[p].vtx[1] = dY * fTileSize;
					poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(dX, dY);
					SET_VEC4(poly[p].col, 0, 255, 0, alpha0);
					++p;

					poly[p].vtx[0] = dX * fTileSize;
					poly[p].vtx[1] = (dY + 1) * fTileSize;
					poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(dX, (dY + 1));
					SET_VEC4(poly[p].col, 0, 255, 0, alpha1);
					++p;
				}
			}

			if (p >= 1024) // restart batch if we overflow
			{
				SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
				MemManager.Set(poly, 0, sizeof(poly));
				p = 0;
			}

			// top
			if (dX < Editor.GetWorld().GetGridWidth() - 1)
			{
				byte alpha0 = INT_FLOOR((*pBrush)[i] * le_blockerBrushInfluenceAlpha);
				byte alpha1 = INT_FLOOR((*pBrush)[i + 1] * le_blockerBrushInfluenceAlpha);

				if (alpha0 || alpha1)
				{
					poly[p].vtx[0] = dX * fTileSize;
					poly[p].vtx[1] = dY * fTileSize;
					poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(dX, dY);
					SET_VEC4(poly[p].col, 0, 255, 0, alpha0);
					++p;

					poly[p].vtx[0] = (dX + 1) * fTileSize;
					poly[p].vtx[1] = dY * fTileSize;
					poly[p].vtx[2] = Editor.GetWorld().GetGridPoint((dX + 1), dY);
					SET_VEC4(poly[p].col, 0, 255, 0, alpha1);
					++p;
				}
			}
		}
	}

	if (p > 0)
		SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
}

