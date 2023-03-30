// (C)2010 S2 Games
// c_watertool.cpp
//
// Water tool
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "editor_common.h"

#include "editor.h"

#include "c_watertool.h"

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
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_INT	(le_waterMode,										WATER_PAINT);
CVAR_FLOAT	(le_waterHeight,									10.0f);

UI_TRIGGER(WaterMode);

//=============================================================================

/*====================
  CWaterTool::CWaterTool
  ====================*/
CWaterTool::CWaterTool() :
ITool(TOOL_WATER, _T("water")),
m_bWorking(false),
m_bPrimaryDown(false),
m_bSecondaryDown(false),
m_bFirstLoop(true),
m_uiWaterDrewMap(0),
m_uiMapSize(0),
m_iOldX(-1),
m_iOldY(-1),
m_iXWaterCenter(0),
m_iYWaterCenter(0),
m_iXOffset(0),
m_iYOffset(0),
m_hLineMaterial(g_ResourceManager.Register(_T("/core/materials/line.material"), RES_MATERIAL)),
m_hWaterMaterial(g_ResourceManager.Register(_T("/tools/water.material"), RES_MATERIAL)),
m_bValidPosition(false),
m_hFont(g_ResourceManager.LookUpName(_T("system_medium"), RES_FONTMAP))
{
	WaterMode.Trigger(_T("Paint"));
}


/*====================
  CWaterTool::PrimaryUp

  Left mouse button up action
  ====================*/
void	CWaterTool::PrimaryUp()
{
	if(le_waterMode == WATER_PAINT && !m_bInverse)
		m_bWorking = false;

	m_bPrimaryDown = false;

}


/*====================
  CWaterTool::PrimaryDown

  Default left mouse button down action
  ====================*/
void	CWaterTool::PrimaryDown()
{
	m_bPrimaryDown = true;

	if(m_bSecondaryDown)
		return;

	m_bFirstLoop = true;

	if (m_uiWaterDrewMap)
	{
		K2_DELETE_ARRAY(m_uiWaterDrewMap);
		m_uiWaterDrewMap = 0;
	}

	m_uiMapSize = (Editor.GetWorld().GetVertCliffMapArea() / 32) + 1;

	m_uiWaterDrewMap = K2_NEW_ARRAY(ctx_Editor, uint, m_uiMapSize);

	for(uint uiMap(0); uiMap < m_uiMapSize; ++uiMap)
	{
		m_uiWaterDrewMap[uiMap] = 0;
	}

	if(le_waterMode == WATER_PAINT)
	{
		m_bWorking = true;
		m_bInverse = false;
	}

	if(!m_bValidPosition)
		return;

	switch (le_waterMode)
	{
	case WATER_PAINT:
		WaterDraw();
		break;
	}

	CalcToolProperties();
	int iWaterSize(Editor.GetWorld().GetCliffSize());
	m_iOldX = (m_iXOffset * iWaterSize - iWaterSize / 2);
	m_iOldY = (m_iYOffset * iWaterSize - iWaterSize / 2);
}


/*====================
  CWaterTool::SecondaryUp

  Right mouse button up action
  ====================*/
void	CWaterTool::SecondaryUp()
{
	if(le_waterMode == WATER_PAINT && m_bInverse)
		m_bWorking = false;

	m_bSecondaryDown = false;
}


/*====================
  CWaterTool::SecondaryDown

  Default right mouse button down action
  ====================*/
void	CWaterTool::SecondaryDown()
{
	m_bSecondaryDown = true;

	if(m_bPrimaryDown)
		return;

	m_bFirstLoop = true;

	if (m_uiWaterDrewMap)
	{
		K2_DELETE_ARRAY(m_uiWaterDrewMap);
		m_uiWaterDrewMap = 0;
	}

	m_uiMapSize = (Editor.GetWorld().GetVertCliffMapArea() / 32) + 1;

	m_uiWaterDrewMap = K2_NEW_ARRAY(ctx_Editor, uint, m_uiMapSize);

	for(uint uiMap(0); uiMap < m_uiMapSize; ++uiMap)
	{
		m_uiWaterDrewMap[uiMap] = 0;
	}

	if(le_waterMode == WATER_PAINT)
	{
		m_bWorking = true;
		m_bInverse = true;
	}

	if(!m_bValidPosition)
		return;

	switch (le_waterMode)
	{
	case WATER_PAINT:
		WaterDraw();
		break;
	}

	CalcToolProperties();
	int iWaterSize(Editor.GetWorld().GetCliffSize());
	m_iOldX = (m_iXOffset * iWaterSize - iWaterSize / 2);
	m_iOldY = (m_iYOffset * iWaterSize - iWaterSize / 2);
}


/*====================
  CWaterTool::TertiaryUp

  Middle mouse button up action
  ====================*/
void	CWaterTool::TertiaryUp() {}


/*====================
  CWaterTool::TertiaryDown

  Middle mouse button down action
  ====================*/
void	CWaterTool::TertiaryDown() {}


/*====================
  CWaterTool::QuaternaryUp

  Scroll wheel up action
  ====================*/
void	CWaterTool::QuaternaryUp() {}


/*====================
  CWaterTool::QuaternaryDown

  Scroll wheel down action
  ====================*/
void	CWaterTool::QuaternaryDown() {}


/*====================
  CWaterTool::Cancel
  ====================*/
void	CWaterTool::Cancel() {}


/*====================
  CWaterTool::Delete
  ====================*/
void	CWaterTool::Delete() {}


/*====================
  CWaterTool::CalcToolProperties
  ====================*/
void	 CWaterTool::CalcToolProperties()
{
	STraceInfo trace;
	//Get Brush
	CBrush *pBrush = CBrush::GetCurrentBrush();

	if (Editor.TraceCursor(trace, TRACE_TERRAIN) && pBrush)
	{
		int iWaterSize(Editor.GetWorld().GetCliffSize());
		float fWorldScale(Editor.GetWorld().GetScale());

		// Clip against the brush data
		CRecti	recBrush;
		pBrush->ClipBrush(recBrush);

		float fBrushCenterX(((recBrush.left + recBrush.right) * iWaterSize) / 2.0f);
		float fBrushCenterY(((recBrush.top + recBrush.bottom) * iWaterSize) / 2.0f);

		float fTestX((trace.v3EndPos.x - fBrushCenterX * fWorldScale) / (iWaterSize * fWorldScale));
		float fTestY((trace.v3EndPos.y - fBrushCenterY * fWorldScale) / (iWaterSize * fWorldScale));

		m_iXOffset = fTestX < 0.0f ? INT_FLOOR(fTestX) : INT_CEIL(fTestX);
		m_iYOffset = fTestY < 0.0f ? INT_FLOOR(fTestY) : INT_CEIL(fTestY);

		m_iXWaterCenter = INT_ROUND((trace.v3EndPos.x / fWorldScale) / (iWaterSize));
		m_iYWaterCenter = INT_ROUND((trace.v3EndPos.y / fWorldScale) / (iWaterSize));

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
		m_iXWaterCenter = 0;
		m_iYWaterCenter = 0;
		m_iXPaint = 0;
		m_iXPaint = 0;
		m_v3EndPos.Clear();
		m_bValidPosition = false;
	}
}


/*====================
  CWaterTool::Enter
  ====================*/
void	CWaterTool::Enter()
{
}


/*====================
  CWaterTool::Frame
 ====================*/
void	CWaterTool::Frame(float fFrameTime)
{
	CalcToolProperties();

	if(!m_bValidPosition)
		return;

	int iWaterSize(Editor.GetWorld().GetCliffSize());

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
	
	if(m_bPrimaryDown || m_bSecondaryDown)
	{
		m_iOldX = (m_iXOffset * iWaterSize );
		m_iOldY = (m_iYOffset * iWaterSize);
	}
	else
	{
		m_iOldX = -1;
		m_iOldY = -1;
	}

	switch (le_waterMode)
	{
	case WATER_PAINT:
		WaterDraw();
		break;
	}
}


/*====================
  CWaterTool::Draw
  ====================*/
void	CWaterTool::Draw()
{
//	if (le_waterDrawBrushInfluence)
	{
		CFontMap *pFontMap(g_ResourceManager.GetFontMap(m_hFont));

		Draw2D.SetColor(0.0f, 0.0f, 0.0f);
		Draw2D.String(4.0f, Draw2D.GetScreenH() - pFontMap->GetMaxHeight() - 1.0f, Draw2D.GetScreenW(), pFontMap->GetMaxHeight(), XtoA(m_v3EndPos), m_hFont);
		Draw2D.SetColor(1.0f, 1.0f, 1.0f);
		Draw2D.String(3.0f, Draw2D.GetScreenH() - pFontMap->GetMaxHeight() - 2.0f, Draw2D.GetScreenW(), pFontMap->GetMaxHeight(), XtoA(m_v3EndPos), m_hFont);
	}
}


/*====================
  CWaterTool::Render
  ====================*/
void	CWaterTool::Render()
{
	int*	pVertCliffMap(Editor.GetWorld().GetVertCliffMap());
	
	if (!pVertCliffMap)
		return;

	SSceneFaceVert poly[1024];
	MemManager.Set(poly, 0, sizeof(poly));
	int p = 0;
	
	if (le_waterMode == WATER_PAINT)
	{
		MemManager.Set(poly, 0, sizeof(poly));
		p = 0;
		CBrush *pBrush = CBrush::GetCurrentBrush();
		CRecti recClippedBrush;


		float fTileSize = Editor.GetWorld().GetScale();

		if (!pBrush)
			return;

		if (!pBrush->ClipBrush(recClippedBrush))
			return;

		int iWaterSize(Editor.GetWorld().GetCliffSize());
		int iX(m_iXOffset);
		int iY(m_iYOffset);
		int iBrushSize(pBrush->GetBrushSize());
		int iWaterHeight(pVertCliffMap[Editor.GetWorld().GetVertCliff(m_iXWaterCenter, m_iYWaterCenter)]);
		float fCliffHeight = iWaterHeight * fTileSize * iWaterSize + le_waterHeight;

		for (int y = 0; y < iBrushSize; ++y)
		{
			for (int x = 0; x < iBrushSize; ++x)
			{
				int iXLoc = (x + iX) * iWaterSize - (iWaterSize - iWaterSize / 2);
				int iYLoc = (y + iY) * iWaterSize - (iWaterSize - iWaterSize / 2);

				if (iXLoc < -2 || iXLoc > Editor.GetWorld().GetTileWidth() - 1)
					continue;

				if (iYLoc < -2 || iYLoc > Editor.GetWorld().GetTileHeight() - 1)
					continue;

				int iCurrentCliffHeight(pVertCliffMap[Editor.GetWorld().GetVertCliff((iXLoc + iWaterSize) / iWaterSize, (iYLoc + iWaterSize) / iWaterSize)]);

				//if (x + iX < 0 || x + iX > Editor.GetWorld().GetVertCliffMapWidth() - 1)
				//	x = x - iX;

				//if (y + iY < 0 || y + iY > Editor.GetWorld().GetVertCliffMapHeight() - 1)
				//	y = y - iY;

				int iBrushIndex = x + y * iBrushSize;
				int iBrushIndexLeft = x + 1 + y * iBrushSize;
				int iBrushIndexRight = x - 1 + y * iBrushSize;
				int iBrushIndexFront = x + (y + 1) * iBrushSize;
				int iBrushIndexBack = x + (y - 1) * iBrushSize;

				int iBrushSides(0);

				if ((*pBrush)[iBrushIndexLeft] > 0)
					++iBrushSides;
				if ((*pBrush)[iBrushIndexRight] > 0)
					++iBrushSides;
				if ((*pBrush)[iBrushIndexFront] > 0)
					++iBrushSides;
				if ((*pBrush)[iBrushIndexBack] > 0)
					++iBrushSides;

					
				if (p >= 1020) // restart batch if we overflow
				{
					SceneManager.AddPoly(p, poly, m_hWaterMaterial, POLY_TRILIST);
					MemManager.Set(poly, 0, sizeof(poly));
					p = 0;
				}

				byte yBrushIndexValue((*pBrush)[iBrushIndex]);

				CVec3f v3PolyLoc(0.0f, 0.0f, 0.0f);

				if (yBrushIndexValue > 0 && iCurrentCliffHeight <= iWaterHeight)
				{
					
					v3PolyLoc.x = (iXLoc + iWaterSize) * fTileSize;
					v3PolyLoc.y = iYLoc * fTileSize;
					v3PolyLoc.x = CLAMP(v3PolyLoc.x, 0.0f, Editor.GetWorld().GetWorldWidth());
					v3PolyLoc.y = CLAMP(v3PolyLoc.y, 0.0f, Editor.GetWorld().GetWorldHeight());
					v3PolyLoc.z = fCliffHeight;

					poly[p].vtx.Set(v3PolyLoc.x, v3PolyLoc.y, v3PolyLoc.z);
					poly[p].tex[0] = 1.0f;
					poly[p].tex[1] = 0.0f;
					SET_VEC4(poly[p].col, 255, 255, 255, 255);
					++p;

					v3PolyLoc.x = (iXLoc + iWaterSize) * fTileSize;
					v3PolyLoc.y = (iYLoc + iWaterSize) * fTileSize;
					v3PolyLoc.x = CLAMP(v3PolyLoc.x, 0.0f, Editor.GetWorld().GetWorldWidth());
					v3PolyLoc.y = CLAMP(v3PolyLoc.y, 0.0f, Editor.GetWorld().GetWorldHeight());
					v3PolyLoc.z = fCliffHeight;

					poly[p].vtx.Set(v3PolyLoc.x, v3PolyLoc.y, v3PolyLoc.z);
					poly[p].tex[0] = 1.0f;
					poly[p].tex[1] = 1.0f;
					SET_VEC4(poly[p].col, 255, 255, 255, 255);
					++p;

					v3PolyLoc.x = iXLoc * fTileSize;
					v3PolyLoc.y = iYLoc * fTileSize;
					v3PolyLoc.x = CLAMP(v3PolyLoc.x, 0.0f, Editor.GetWorld().GetWorldWidth());
					v3PolyLoc.y = CLAMP(v3PolyLoc.y, 0.0f, Editor.GetWorld().GetWorldHeight());
					v3PolyLoc.z = fCliffHeight;

					poly[p].vtx.Set(v3PolyLoc.x, v3PolyLoc.y, v3PolyLoc.z);
					poly[p].tex[0] = 0.0f;
					poly[p].tex[1] = 0.0f;
					SET_VEC4(poly[p].col, 255, 255, 255, 255);
					++p;


					v3PolyLoc.x = (iXLoc + iWaterSize) * fTileSize;
					v3PolyLoc.y = (iYLoc + iWaterSize) * fTileSize;
					v3PolyLoc.x = CLAMP(v3PolyLoc.x, 0.0f, Editor.GetWorld().GetWorldWidth());
					v3PolyLoc.y = CLAMP(v3PolyLoc.y, 0.0f, Editor.GetWorld().GetWorldHeight());
					v3PolyLoc.z = fCliffHeight;

					poly[p].vtx.Set(v3PolyLoc.x, v3PolyLoc.y, v3PolyLoc.z);
					poly[p].tex[0] = 1.0f;
					poly[p].tex[1] = 1.0f;
					SET_VEC4(poly[p].col, 255, 255, 255, 255);
					++p;


					v3PolyLoc.x = iXLoc * fTileSize;
					v3PolyLoc.y = (iYLoc + iWaterSize) * fTileSize;
					v3PolyLoc.x = CLAMP(v3PolyLoc.x, 1.0f, Editor.GetWorld().GetWorldWidth());
					v3PolyLoc.y = CLAMP(v3PolyLoc.y, 1.0f, Editor.GetWorld().GetWorldHeight());
					v3PolyLoc.z = fCliffHeight;

					poly[p].vtx.Set(v3PolyLoc.x, v3PolyLoc.y, v3PolyLoc.z);
					poly[p].tex[0] = 0.0f; 
					poly[p].tex[1] = 1.0f;
					SET_VEC4(poly[p].col, 255, 255, 255, 255);
					++p;


					v3PolyLoc.x = iXLoc * fTileSize;
					v3PolyLoc.y = iYLoc * fTileSize;
					v3PolyLoc.x = CLAMP(v3PolyLoc.x, 0.0f, Editor.GetWorld().GetWorldWidth());
					v3PolyLoc.y = CLAMP(v3PolyLoc.y, 0.0f, Editor.GetWorld().GetWorldHeight());
					v3PolyLoc.z = fCliffHeight;

					poly[p].vtx.Set(v3PolyLoc.x, v3PolyLoc.y, v3PolyLoc.z);
					poly[p].tex[0] = 0.0f;
					poly[p].tex[1] = 0.0f;
					SET_VEC4(poly[p].col, 255, 255, 255, 255);
					++p;
				}
				else if (yBrushIndexValue > 0 || iBrushSides > 0)
				{
					int iCCHeightR(pVertCliffMap[Editor.GetWorld().GetVertCliff((iXLoc) / iWaterSize, (iYLoc + iWaterSize) / iWaterSize)]);
					int iCCHeightB(pVertCliffMap[Editor.GetWorld().GetVertCliff((iXLoc + iWaterSize) / iWaterSize, (iYLoc) / iWaterSize)]);
					int iCCHeightF(pVertCliffMap[Editor.GetWorld().GetVertCliff((iXLoc + iWaterSize) / iWaterSize, (iYLoc + iWaterSize * 2) / iWaterSize)]);
					int iCCHeightL(pVertCliffMap[Editor.GetWorld().GetVertCliff((iXLoc + iWaterSize  * 2) / iWaterSize, (iYLoc + iWaterSize) / iWaterSize)]);

					if (iBrushSides >= 3 || ((*pBrush)[iBrushIndexRight] > 0 && (*pBrush)[iBrushIndexFront] > 0 && iCCHeightR <= iWaterHeight && iCCHeightF <= iWaterHeight))
					{
						v3PolyLoc.x = (iXLoc + iWaterSize) * fTileSize;
						v3PolyLoc.y = (iYLoc + iWaterSize) * fTileSize;
						v3PolyLoc.x = CLAMP(v3PolyLoc.x, 0.0f, Editor.GetWorld().GetWorldWidth());
						v3PolyLoc.y = CLAMP(v3PolyLoc.y, 0.0f, Editor.GetWorld().GetWorldHeight());
						v3PolyLoc.z = fCliffHeight;

						poly[p].vtx.Set(v3PolyLoc.x, v3PolyLoc.y, v3PolyLoc.z);
						poly[p].tex[0] = 1.0f;
						poly[p].tex[1] = 1.0f;
						SET_VEC4(poly[p].col, 255, 255, 255, 255);
						++p;


						v3PolyLoc.x = iXLoc * fTileSize;
						v3PolyLoc.y = (iYLoc + iWaterSize) * fTileSize;
						v3PolyLoc.x = CLAMP(v3PolyLoc.x, 1.0f, Editor.GetWorld().GetWorldWidth());
						v3PolyLoc.y = CLAMP(v3PolyLoc.y, 1.0f, Editor.GetWorld().GetWorldHeight());
						v3PolyLoc.z = fCliffHeight;

						poly[p].vtx.Set(v3PolyLoc.x, v3PolyLoc.y, v3PolyLoc.z);
						poly[p].tex[0] = 0.0f; 
						poly[p].tex[1] = 1.0f;
						SET_VEC4(poly[p].col, 255, 255, 255, 255);
						++p;


						v3PolyLoc.x = iXLoc * fTileSize;
						v3PolyLoc.y = iYLoc * fTileSize;
						v3PolyLoc.x = CLAMP(v3PolyLoc.x, 0.0f, Editor.GetWorld().GetWorldWidth());
						v3PolyLoc.y = CLAMP(v3PolyLoc.y, 0.0f, Editor.GetWorld().GetWorldHeight());
						v3PolyLoc.z = fCliffHeight;

						poly[p].vtx.Set(v3PolyLoc.x, v3PolyLoc.y, v3PolyLoc.z);
						poly[p].tex[0] = 0.0f;
						poly[p].tex[1] = 0.0f;
						SET_VEC4(poly[p].col, 255, 255, 255, 255);
						++p;
					}
					else if ((*pBrush)[iBrushIndexRight] > 0 && (*pBrush)[iBrushIndexBack] > 0 && iCCHeightR <= iWaterHeight && iCCHeightB <= iWaterHeight)
					{	
						v3PolyLoc.x = iXLoc * fTileSize;
						v3PolyLoc.y = iYLoc * fTileSize;
						v3PolyLoc.x = CLAMP(v3PolyLoc.x, 0.0f, Editor.GetWorld().GetWorldWidth());
						v3PolyLoc.y = CLAMP(v3PolyLoc.y, 0.0f, Editor.GetWorld().GetWorldHeight());
						v3PolyLoc.z = fCliffHeight;

						poly[p].vtx.Set(v3PolyLoc.x, v3PolyLoc.y, v3PolyLoc.z);
						poly[p].tex[0] = 0.0f;
						poly[p].tex[1] = 0.0f;
						SET_VEC4(poly[p].col, 255, 255, 255, 255);
						++p;

						v3PolyLoc.x = (iXLoc + iWaterSize) * fTileSize;
						v3PolyLoc.y = iYLoc * fTileSize;
						v3PolyLoc.x = CLAMP(v3PolyLoc.x, 1.0f, Editor.GetWorld().GetWorldWidth());
						v3PolyLoc.y = CLAMP(v3PolyLoc.y, 1.0f, Editor.GetWorld().GetWorldHeight());
						v3PolyLoc.z = fCliffHeight;

						poly[p].vtx.Set(v3PolyLoc.x, v3PolyLoc.y, v3PolyLoc.z);
						poly[p].tex[0] = 1.0f; 
						poly[p].tex[1] = 0.0f;
						SET_VEC4(poly[p].col, 255, 255, 255, 255);
						++p;
						
						v3PolyLoc.x = iXLoc * fTileSize;
						v3PolyLoc.y = (iYLoc + iWaterSize) * fTileSize;
						v3PolyLoc.x = CLAMP(v3PolyLoc.x, 0.0f, Editor.GetWorld().GetWorldWidth());
						v3PolyLoc.y = CLAMP(v3PolyLoc.y, 0.0f, Editor.GetWorld().GetWorldHeight());
						v3PolyLoc.z = fCliffHeight;

						poly[p].vtx.Set(v3PolyLoc.x, v3PolyLoc.y, v3PolyLoc.z);
						poly[p].tex[0] = 0.0f;
						poly[p].tex[1] = 1.0f;
						SET_VEC4(poly[p].col, 255, 255, 255, 255);
						++p;
					}

					if (iBrushSides >= 3 || ((*pBrush)[iBrushIndexLeft] > 0 && (*pBrush)[iBrushIndexBack] > 0 && iCCHeightL <= iWaterHeight && iCCHeightB <= iWaterHeight))
					{
						v3PolyLoc.x = (iXLoc + iWaterSize) * fTileSize;
						v3PolyLoc.y = iYLoc * fTileSize;
						v3PolyLoc.x = CLAMP(v3PolyLoc.x, 0.0f, Editor.GetWorld().GetWorldWidth());
						v3PolyLoc.y = CLAMP(v3PolyLoc.y, 0.0f, Editor.GetWorld().GetWorldHeight());
						v3PolyLoc.z = fCliffHeight;

						poly[p].vtx.Set(v3PolyLoc.x, v3PolyLoc.y, v3PolyLoc.z);
						poly[p].tex[0] = 1.0f;
						poly[p].tex[1] = 0.0f;
						SET_VEC4(poly[p].col, 255, 255, 255, 255);
						++p;

						v3PolyLoc.x = (iXLoc + iWaterSize) * fTileSize;
						v3PolyLoc.y = (iYLoc + iWaterSize) * fTileSize;
						v3PolyLoc.x = CLAMP(v3PolyLoc.x, 0.0f, Editor.GetWorld().GetWorldWidth());
						v3PolyLoc.y = CLAMP(v3PolyLoc.y, 0.0f, Editor.GetWorld().GetWorldHeight());
						v3PolyLoc.z = fCliffHeight;

						poly[p].vtx.Set(v3PolyLoc.x, v3PolyLoc.y, v3PolyLoc.z);
						poly[p].tex[0] = 1.0f;
						poly[p].tex[1] = 1.0f;
						SET_VEC4(poly[p].col, 255, 255, 255, 255);
						++p;

						v3PolyLoc.x = iXLoc * fTileSize;
						v3PolyLoc.y = iYLoc * fTileSize;
						v3PolyLoc.x = CLAMP(v3PolyLoc.x, 0.0f, Editor.GetWorld().GetWorldWidth());
						v3PolyLoc.y = CLAMP(v3PolyLoc.y, 0.0f, Editor.GetWorld().GetWorldHeight());
						v3PolyLoc.z = fCliffHeight;

						poly[p].vtx.Set(v3PolyLoc.x, v3PolyLoc.y, v3PolyLoc.z);
						poly[p].tex[0] = 0.0f;
						poly[p].tex[1] = 0.0f;
						SET_VEC4(poly[p].col, 255, 255, 255, 255);
						++p;
					}
					else if ((*pBrush)[iBrushIndexLeft] > 0 && (*pBrush)[iBrushIndexFront] > 0 && iCCHeightL <= iWaterHeight && iCCHeightF <= iWaterHeight)
					{
						
						v3PolyLoc.x = iXLoc * fTileSize;
						v3PolyLoc.y = (iYLoc + iWaterSize) * fTileSize;
						v3PolyLoc.x = CLAMP(v3PolyLoc.x, 0.0f, Editor.GetWorld().GetWorldWidth());
						v3PolyLoc.y = CLAMP(v3PolyLoc.y, 0.0f, Editor.GetWorld().GetWorldHeight());
						v3PolyLoc.z = fCliffHeight;

						poly[p].vtx.Set(v3PolyLoc.x, v3PolyLoc.y, v3PolyLoc.z);
						poly[p].tex[0] = 0.0f;
						poly[p].tex[1] = 1.0f;
						SET_VEC4(poly[p].col, 255, 255, 255, 255);
						++p;

						v3PolyLoc.x = (iXLoc + iWaterSize) * fTileSize;
						v3PolyLoc.y = iYLoc * fTileSize;
						v3PolyLoc.x = CLAMP(v3PolyLoc.x, 0.0f, Editor.GetWorld().GetWorldWidth());
						v3PolyLoc.y = CLAMP(v3PolyLoc.y, 0.0f, Editor.GetWorld().GetWorldHeight());
						v3PolyLoc.z = fCliffHeight;

						poly[p].vtx.Set(v3PolyLoc.x, v3PolyLoc.y, v3PolyLoc.z);
						poly[p].tex[0] = 1.0f;
						poly[p].tex[1] = 0.0f;
						SET_VEC4(poly[p].col, 255, 255, 255, 255);
						++p;

						v3PolyLoc.x = (iXLoc + iWaterSize) * fTileSize;
						v3PolyLoc.y = (iYLoc + iWaterSize) * fTileSize;
						v3PolyLoc.x = CLAMP(v3PolyLoc.x, 0.0f, Editor.GetWorld().GetWorldWidth());
						v3PolyLoc.y = CLAMP(v3PolyLoc.y, 0.0f, Editor.GetWorld().GetWorldHeight());
						v3PolyLoc.z = fCliffHeight;

						poly[p].vtx.Set(v3PolyLoc.x, v3PolyLoc.y, v3PolyLoc.z);
						poly[p].tex[0] = 1.0f;
						poly[p].tex[1] = 1.0f;
						SET_VEC4(poly[p].col, 255, 255, 255, 255);
						++p;
					}
				}
			}
		}
	}
	
	if (p > 0)
	{
		SceneManager.AddPoly(p, poly, m_hWaterMaterial, POLY_TRILIST);
	}
	
}


/*--------------------
  cmdSetWaterMode
  --------------------*/
UI_VOID_CMD(SetWaterMode, 1)
{
	if (vArgList.size() < 1)
	{
		Console << _T("syntax: SetWaterMode paint") << newl;
		return;
	}

	tstring sValue(vArgList[0]->Evaluate());

	if (sValue == _T("paint"))
	{
		le_waterMode = WATER_PAINT;
		WaterMode.Trigger(_T("Paint"));
		return;
	}

	return;
}


/*====================
  CWaterTool::WaterDraw
  ====================*/
void	CWaterTool::WaterDraw()
{
}



/*====================
  CWaterTool::ClampWaterRectToGrid
  ====================*/
void CWaterTool::ClampWaterRectToGrid(CRecti *rArea)
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
  CWaterTool::ClampRectToGrid
  ====================*/
void CWaterTool::ClampRectToGrid(CRecti *rArea)
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