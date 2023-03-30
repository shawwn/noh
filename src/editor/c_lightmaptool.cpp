// (C)2005 S2 Games
// c_lightmaptool.cpp
//
// Lightmap Tool
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "editor_common.h"

#include "editor.h"

#include "c_lightmaptool.h"

#include "../k2/c_brush.h"
#include "../k2/c_world.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_vid.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_FLOAT(le_lightmapR, 1.0f);
CVAR_FLOAT(le_lightmapG, 0.0f);
CVAR_FLOAT(le_lightmapB, 0.0f);
CVAR_FLOAT(le_lightmapA, 1.0f);
CVAR_FLOAT(le_lightmapR2, 1.0f);
CVAR_FLOAT(le_lightmapG2, 1.0f);
CVAR_FLOAT(le_lightmapB2, 1.0f);
CVAR_FLOAT(le_lightmapA2, 1.0f);
CVAR_FLOAT(le_lightmapBrushStrength, 50.0f);
//=============================================================================

/*====================
  CLightmapTool::CLightmapTool()
  ====================*/
CLightmapTool::CLightmapTool() :
ITool(TOOL_LIGHTMAP, _T("lightmap")),
m_bValidPosition(false),
m_bWorking(false),
m_bInverse(false)
{
}


/*====================
  CLightmapTool::PrimaryUp

  Left mouse button up action
  ====================*/
void	CLightmapTool::PrimaryUp()
{
	if (!m_bInverse)
		m_bWorking = false;
}

/*====================
  CLightmapTool::PrimaryDown

  Default left mouse button down action
  ====================*/
void	CLightmapTool::PrimaryDown()
{
	m_bWorking = true;
	m_bInverse = false;
	CalcToolProperties();
}

/*====================
  CLightmapTool::SecondaryUp

  Right mouse button up action
  ====================*/
void	CLightmapTool::SecondaryUp()
{
	if (m_bInverse)
		m_bWorking = false;
}

/*====================
  CLightmapTool::SecondaryDown

  Default right mouse button down action - not used in this case
  ====================*/
void	CLightmapTool::SecondaryDown()
{
	m_bWorking = true;
	m_bInverse = true;
	CalcToolProperties();
}


/*====================
  CLightmapTool::CalcToolProperties
  ====================*/
void	 CLightmapTool::CalcToolProperties()
{
	STraceInfo trace;
	CBrush *pBrush = CBrush::GetCurrentBrush();

	if (Editor.TraceCursor(trace, TRACE_TERRAIN) && pBrush)
	{
		// Clip against the brush data
		CRecti	recBrush;
		pBrush->ClipBrush(recBrush);

		float fBrushCenterX((recBrush.left + recBrush.right) / 2.0f);
		float fBrushCenterY((recBrush.top + recBrush.bottom) / 2.0f);

		float fTestX((trace.v3EndPos.x - fBrushCenterX * Editor.GetWorld().GetScale()) / (Editor.GetWorld().GetScale()));
		float fTestY((trace.v3EndPos.y - fBrushCenterY * Editor.GetWorld().GetScale()) / (Editor.GetWorld().GetScale()));

		m_iX = fTestX < 0.0f ? INT_FLOOR(fTestX) : INT_CEIL(fTestX);
		m_iY = fTestY < 0.0f ? INT_FLOOR(fTestY) : INT_CEIL(fTestY);

		m_bValidPosition = true;

		//m_v3EndPos = trace.v3EndPos;
	}
	else
	{
		m_bValidPosition = false;
		m_iX = 0;
		m_iY = 0;
		//m_v3EndPos.Clear();
	}

	/*if (Editor.TraceCursor(trace, TRACE_TERRAIN))
	{
		m_iX = Editor.GetWorld().GetVertFromCoord(trace.v3EndPos.x);
		m_iY = Editor.GetWorld().GetVertFromCoord(trace.v3EndPos.y);
	}
	else
	{
		m_iX = 0;
		m_iY = 0;
	}*/

	if (m_bInverse)
	{
		m_vecColor[R] = le_lightmapR2;
		m_vecColor[G] = le_lightmapG2;
		m_vecColor[B] = le_lightmapB2;
		m_vecColor[A] = le_lightmapA2;
	}
	else
	{
		m_vecColor[R] = le_lightmapR;
		m_vecColor[G] = le_lightmapG;
		m_vecColor[B] = le_lightmapB;
		m_vecColor[A] = le_lightmapA;
	}
}


/*====================
  CLightmapTool::LerpColor
  ====================*/
void	CLightmapTool::LerpColor(CVec4b *pRegion, const CRecti &recArea, const CBrush &brush, const CVec4f &v4Color, float fScale)
{
	int iBrushSize(brush.GetBrushSize());

	int iRegionIndex(0);
	for (int y(0); y < recArea.GetHeight(); ++y)
	{
		for (int x(0); x < recArea.GetWidth(); ++x)
		{
			float fLerp = brush[BRUSH_INDEX(x, y)] * fScale;

			if (!fLerp)
			{
				++iRegionIndex;
				continue;
			}

			CVec3f v3OutColor;
			v3OutColor[R] = LERP(fLerp, pRegion[iRegionIndex][R] / 255.0f, v4Color[R]);
			v3OutColor[G] = LERP(fLerp, pRegion[iRegionIndex][G] / 255.0f, v4Color[G]);
			v3OutColor[B] = LERP(fLerp, pRegion[iRegionIndex][B] / 255.0f, v4Color[B]);
			pRegion[iRegionIndex][R] = static_cast<byte>(CLAMP(v3OutColor[R], 0.0f, 1.0f) * 255);
			pRegion[iRegionIndex][G] = static_cast<byte>(CLAMP(v3OutColor[G], 0.0f, 1.0f) * 255);
			pRegion[iRegionIndex][B] = static_cast<byte>(CLAMP(v3OutColor[B], 0.0f, 1.0f) * 255);
			++iRegionIndex;
		}
	}
}

/*====================
  CLightmapTool::PaintVertex
  ====================*/
void	CLightmapTool::PaintVertex(float fFrameTime)
{
	CVec4b *pRegion(NULL);

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
		recClippedBrush.Shift(m_iX, m_iY);
		if (!Editor.GetWorld().ClipRect(recClippedBrush, GRID_SPACE))
			return;

		// Get the region
		pRegion = K2_NEW_ARRAY(ctx_Editor, CVec4b, recClippedBrush.GetArea());
		if (pRegion == NULL)
			EX_ERROR(_T("Failed to allocate region"));

		if (!Editor.GetWorld().GetRegion(WORLD_VERT_COLOR_MAP, recClippedBrush, pRegion))
			EX_ERROR(_T("Failed to retrieve region"));

		// Perform the operation
		recClippedBrush.Shift(-m_iX, -m_iY);
		LerpColor(pRegion, recClippedBrush, *pBrush, m_vecColor, fFrameTime * le_lightmapBrushStrength / 500.0f);

		// Apply the modified region
		recClippedBrush.Shift(m_iX, m_iY);
		if (!Editor.GetWorld().SetRegion(WORLD_VERT_COLOR_MAP, recClippedBrush, pRegion))
			EX_ERROR(_T("SetRegion failed"));

			for (int y(recClippedBrush.top); y < recClippedBrush.bottom; ++y)
			{
				for (int x(recClippedBrush.left); x < recClippedBrush.right; ++x)
					Vid.Notify(VID_NOTIFY_TERRAIN_COLOR_MODIFIED, x, y, 0, &Editor.GetWorld());
			}

		K2_DELETE_ARRAY(pRegion);
	}
	catch (CException &ex)
	{
		if (pRegion != NULL)
			K2_DELETE_ARRAY(pRegion);

		ex.Process(_T("CLightMapTool::PaintVertex() - "), NO_THROW);
	}
}


/*====================
  CLightmapTool::Frame
  ====================*/
void	CLightmapTool::Frame(float fFrameTime)
{
	if (m_bWorking)
	{
		CalcToolProperties();

		PaintVertex(fFrameTime);
	}
}
