// (C)2005 S2 Games
// c_deformtool.cpp
//
// Terrain deformer
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "editor_common.h"

#include "editor.h"

#include "c_deformtool.h"

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
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_INT	(le_deformMode,					DEFORM_ADD);
CVAR_FLOAT	(le_deformBrushStrength,		128.0f);
CVAR_FLOAT	(le_deformFlattenHeight,		0.0f);
CVAR_FLOAT	(le_deformCutHeight,			0.0f);
CVAR_FLOAT	(le_deformClearHeight,			0.0f);
CVAR_FLOAT	(le_deformSliceHeight,			0.0f);
CVAR_FLOAT	(le_deformNoiseOctave,			1.0f);
CVAR_INT	(le_deformHarmonics,			6);
CVAR_BOOL	(le_deformDrawBrushInfluence,	true);
CVAR_FLOAT	(le_deformBrushInfluenceAlpha,	1.0f);
CVAR_BOOLF	(le_deformDrawBrushCoords,		true,	CVAR_SAVECONFIG);
CVAR_INT	(le_deformStamp,				0);
CVAR_FLOAT	(le_deformStampHeight,			0.0f);
CVAR_FLOAT	(le_deformStampScale,			128.0f);
CVAR_BOOL	(le_deformLockCliffVerts,		true);

UI_TRIGGER(DeformMode);
//=============================================================================


/*====================
  CDeformTool::CDeformTool
  ====================*/
CDeformTool::CDeformTool() :
ITool(TOOL_DEFORM, _T("deform")),
m_bWorking(false),
m_hLineMaterial(g_ResourceManager.Register(_T("/core/materials/line.material"), RES_MATERIAL)),
m_hFont(g_ResourceManager.LookUpName(_T("system_medium"), RES_FONTMAP)),
m_iOldX(0),
m_iOldY(0),
m_iXOffset(0),
m_iYOffset(0),
m_bValidPosition(false)
{
	DeformMode.Trigger(_T("Raise/Lower"));
}


/*====================
  CDeformTool::PrimaryUp

  Left mouse button up action
  ====================*/
void	CDeformTool::PrimaryUp()
{
	if (!m_bInverse)
		m_bWorking = false;
}


/*====================
  CDeformTool::PrimaryDown

  Default left mouse button down action
  ====================*/
void	CDeformTool::PrimaryDown()
{
	CalcToolProperties();

	m_bWorking = true;
	m_bInverse = false;

	// Set le_deformFlattenHeight
	STraceInfo trace;

	if (m_bValidPosition)
		le_deformFlattenHeight = Editor.GetWorld().GetTerrainHeight(m_v3EndPos.x, m_v3EndPos.y);

	m_fRX = (( (long)K2_RAND((double)0,(double)0x7ffffff) & 0x7fff) / (float)0x7fff) * 10000.0f;//K2_RAND( M_Randnum(0.0f, 10000.0f);
	m_fRY = (( (long)K2_RAND((double)0,(double)0x7ffffff) & 0x7fff) / (float)0x7fff) * 10000.0f;
}


/*====================
  CDeformTool::SecondaryUp

  Right mouse button up action
  ====================*/
void	CDeformTool::SecondaryUp()
{
	if (m_bInverse)
		m_bWorking = false;
}


/*====================
  CDeformTool::SecondaryDown

  Default right mouse button down action - not used in this case
  ====================*/
void	CDeformTool::SecondaryDown()
{
	CalcToolProperties();

	m_bWorking = true;
	m_bInverse = true;

	m_fRX = (( (long)K2_RAND((double)0,(double)0x7ffffff) & 0x7fff) / (float)0x7fff) * 10000.0f;//K2_RAND( M_Randnum(0.0f, 10000.0f);
	m_fRY = (( (long)K2_RAND((double)0,(double)0x7ffffff) & 0x7fff) / (float)0x7fff) * 10000.0f;
}


/*====================
  CDeformTool::TertiaryUp

  Middle mouse button up action
  ====================*/
void	CDeformTool::TertiaryUp() {}


/*====================
  CDeformTool::TertiaryDown

  Middle mouse button down action
  ====================*/
void	CDeformTool::TertiaryDown() {}


/*====================
  CDeformTool::QuaternaryUp

  Scroll wheel up action
  ====================*/
void	CDeformTool::QuaternaryUp() {}


/*====================
  CDeformTool::QuaternaryDown

  Scroll wheel down action
  ====================*/
void	CDeformTool::QuaternaryDown() {}


/*====================
  CDeformTool::Cancel
  ====================*/
void	CDeformTool::Cancel() {}


/*====================
  CDeformTool::Delete
  ====================*/
void	CDeformTool::Delete() {}



/*====================
  CDeformTool::CalcToolProperties
  ====================*/
void	 CDeformTool::CalcToolProperties()
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

		m_iXOffset = fTestX < 0.0f ? INT_FLOOR(fTestX) : INT_CEIL(fTestX);
		m_iYOffset = fTestY < 0.0f ? INT_FLOOR(fTestY) : INT_CEIL(fTestY);

		m_bValidPosition = true;

		m_v3EndPos = trace.v3EndPos;
	}
	else
	{
		m_bValidPosition = false;
		m_iXOffset = 0;
		m_iYOffset = 0;
		m_v3EndPos.Clear();
	}

	if (m_iXOffset != m_iOldX && m_iYOffset != m_iOldY)
	{
		m_iOldX = m_iXOffset;
		m_iOldY = m_iYOffset;
	}
}


/*====================
  CDeformTool::TerrainAdd
  ====================*/
void	CDeformTool::TerrainAdd(float *pRegion, CRecti &recArea, CBrush &brush, float fScale)
{
	int iBrushSize(brush.GetBrushSize());

	int iRegionIndex(0);
	for (int y(0); y < recArea.GetHeight(); ++y)
	{
		for (int x(0); x < recArea.GetWidth(); ++x)
		{	
			if (CanPantVert(x, y))
				pRegion[iRegionIndex] += brush[BRUSH_INDEX(x, y)] * fScale;

			++iRegionIndex;
		}
	}
}

/*====================
  CDeformTool::CanPantVert
  ====================*/
bool CDeformTool::CanPantVert(int iX, int iY)
{
		//Cvar Override
	if (le_deformLockCliffVerts == false)
		return true;
	
		//Get The Brush
	CBrush *pBrush = CBrush::GetCurrentBrush();

	if (!pBrush)
		return true;

		// Clip against the brush data
	CRecti	recBrush;
	pBrush->ClipBrush(recBrush);

		//Add the rects true offset
	iY += (m_iYOffset + recBrush.top < 0 ? 0 : m_iYOffset + recBrush.top);
	iX += (m_iXOffset + recBrush.left < 0 ? 0 :  m_iXOffset + recBrush.left);

		//Check titles in a 2x2 square
	CRecti rArea(iX, iY, iX + 2, iY + 2);

	Editor.GetWorld().ClipRect(rArea, TILE_SPACE);

	rArea.Shift(-1, -1);

		//Get the Region
	byte* pCliffMap(Editor.GetWorld().GetCliffMap());

		//Check for cliff tiles
	for(int x(rArea.left); x < rArea.right; ++x)
	{
		for(int y(rArea.top); y < rArea.bottom; ++y)
		{
			if (x < 0)
				continue;

			if (y < 0)
				continue;

			if(pCliffMap[x + (y * Editor.GetWorld().GetTileWidth())] == 1)
				return false;	//Found a cliff tile cant edit this vert
		}
	}

		//No verts found
	return true;
}

/*====================
  CDeformTool::TerrainFlatten
  ====================*/
void	CDeformTool::TerrainFlatten(float *pRegion, CRecti &recArea, CBrush &brush, float fScale, float fFlattenHeight)
{
	int iBrushSize(brush.GetBrushSize());

	int iRegionIndex(0);
	for (int y(0); y < recArea.GetHeight(); ++y)
	{
		for (int x(0); x < recArea.GetWidth(); ++x)
		{
			if (CanPantVert(x, y))
			{
				float fDelta = brush[BRUSH_INDEX(x, y)] * fScale;

				if (pRegion[iRegionIndex] > fFlattenHeight)
				{
					pRegion[iRegionIndex] -= fDelta;

					if(pRegion[iRegionIndex] < fFlattenHeight)
						pRegion[iRegionIndex] = fFlattenHeight;
				}
				else if (pRegion[iRegionIndex] < fFlattenHeight)
				{
					pRegion[iRegionIndex] += fDelta;

					if(pRegion[iRegionIndex] > fFlattenHeight)
						pRegion[iRegionIndex] = fFlattenHeight;
				}
			}
			++iRegionIndex;
		}
	}
}


/*====================
  CDeformTool::TerrainSmooth
  ====================*/
void	CDeformTool::TerrainSmooth(float *pRegion, CRecti &recArea, CBrush &brush, float fScale)
{
	#define REGION_INDEX(x, y) ((x) + ((y) * recArea.GetWidth()))

	int iBrushSize(brush.GetBrushSize());
	float *pRegionClean = K2_NEW_ARRAY(ctx_Editor, float, recArea.GetArea());

	MemManager.Copy(pRegionClean, pRegion, sizeof(float) * recArea.GetArea());

	for (int y(1); y < recArea.GetHeight() - 1; ++y)
	{
		for (int x(1); x < recArea.GetWidth() - 1; ++x)
		{
			float fLerp = brush[BRUSH_INDEX(x, y)] * fScale;

			fLerp = CLAMP(fLerp, 0.0f, 1.0f);
			if (!fLerp)
				continue;

			float fAverage	=	pRegionClean[REGION_INDEX(x - 1, y - 1)] +
								pRegionClean[REGION_INDEX(x, y - 1)] +
								pRegionClean[REGION_INDEX(x + 1, y - 1)] +
								pRegionClean[REGION_INDEX(x - 1, y)] +
								pRegionClean[REGION_INDEX(x, y)] +
								pRegionClean[REGION_INDEX(x + 1, y)] +
								pRegionClean[REGION_INDEX(x - 1, y + 1)] +
								pRegionClean[REGION_INDEX(x, y + 1)] +
								pRegionClean[REGION_INDEX(x + 1, y + 1)];

			fAverage /= 9.0f;

			if (CanPantVert(x, y))
				pRegion[REGION_INDEX(x, y)] = LERP(fLerp, pRegion[REGION_INDEX(x, y)], fAverage);
		}
	}

	K2_DELETE_ARRAY(pRegionClean);

	#undef REGION_INDEX
}


/*====================
  CDeformTool::TerrainCut
  ====================*/
void	CDeformTool::TerrainCut(float *pRegion, CRecti &recArea, CBrush &brush, float fScale, float fCutHeight)
{
	int iBrushSize(brush.GetBrushSize());

	int iRegionIndex(0);
	for (int y(0); y < recArea.GetHeight(); ++y)
	{
		for (int x(0); x < recArea.GetWidth(); ++x)
		{
			if (brush[BRUSH_INDEX(x, y)] > 0 && CanPantVert(x, y))
			{
				float target = fCutHeight - brush[BRUSH_INDEX(x, y)] * fScale;
				if (pRegion[iRegionIndex] > target)
					pRegion[iRegionIndex] = target;
			}
			++iRegionIndex;
		}
	}
}


/*====================
  CDeformTool::TerrainSlice
  ====================*/
void	CDeformTool::TerrainSlice(float *pRegion, CRecti &recArea, CBrush &brush, float fSliceHeight)
{
	int iBrushSize(brush.GetBrushSize());

	int iRegionIndex(0);
	for (int y(0); y < recArea.GetHeight(); ++y)
	{
		for (int x(0); x < recArea.GetWidth(); ++x)
		{
			if (brush[BRUSH_INDEX(x, y)] > 0 && pRegion[iRegionIndex] > fSliceHeight && CanPantVert(x, y))
			{
					pRegion[iRegionIndex] = fSliceHeight;
			}
			++iRegionIndex;
		}
	}
}


/*====================
  CDeformTool::TerrainClear
  ====================*/
void	CDeformTool::TerrainClear(float *pRegion, CRecti &recArea, CBrush &brush)
{
	int iBrushSize(brush.GetBrushSize());

	int iRegionIndex(0);
	for (int y(0); y < recArea.GetHeight(); ++y)
	{
		for (int x(0); x < recArea.GetWidth(); ++x)
		{
			if (brush[BRUSH_INDEX(x, y)] && CanPantVert(x, y))
				pRegion[iRegionIndex] = le_deformClearHeight;

			++iRegionIndex;
		}
	}
}


/*====================
  CDeformTool::TerrainReduce
  ====================*/
void	CDeformTool::TerrainReduce(float *pRegion, CRecti &recArea, CBrush &brush, float fScale)
{
	int iBrushSize(brush.GetBrushSize());

	int iRegionIndex(0);
	for (int y(0); y < recArea.GetHeight(); ++y)
	{
		for (int x(0); x < recArea.GetWidth(); ++x)
		{
			if (CanPantVert(x, y))
			{
				if (pRegion[iRegionIndex] > 0.0f)
				{
					pRegion[iRegionIndex] -= brush[BRUSH_INDEX(x, y)] * fScale;

					if (pRegion[iRegionIndex] < 0.0f)
						pRegion[iRegionIndex] = 0.0f;
				}
				else if(pRegion[iRegionIndex] < 0.0f)
				{
					pRegion[iRegionIndex] += brush[BRUSH_INDEX(x, y)] * fScale;

					if (pRegion[iRegionIndex] > 0.0f)
						pRegion[iRegionIndex] = 0.0f;
				}
			}

			++iRegionIndex;
		}
	}
}


/*====================
  CDeformTool::TerrainNoise
  ====================*/
void	CDeformTool::TerrainNoise(float *pRegion, CRecti &recArea, CBrush &brush, float fScale, float fRX, float fRY, float fOctave)
{
	int iBrushSize(brush.GetBrushSize());

	int iRegionIndex(0);
	for (int y(0); y < recArea.GetHeight(); ++y)
	{
		for (int x(0); x < recArea.GetWidth(); ++x)
		{
			if (CanPantVert(x, y))
			{
				pRegion[iRegionIndex] += brush[BRUSH_INDEX(x, y)] * fScale * M_Noise2((fRX + x) / fOctave, (fRY + y) / fOctave);
			}
			++iRegionIndex;
		}
	}
}


/*====================
  CDeformTool::TerrainHarmonic
  ====================*/
void	CDeformTool::TerrainHarmonic(float *pRegion, CRecti &recArea, CBrush &brush, float fScale, float fRX, float fRY, int iHarmonics)
{
	int iBrushSize(brush.GetBrushSize());
	fScale *= 0.075f;

	int iRegionIndex(0);
	for (int y(0); y < recArea.GetHeight(); ++y)
	{
		for (int x(0); x < recArea.GetWidth(); ++x)
		{
			if (CanPantVert(x, y))
			{
				for (int h = 0; h < iHarmonics; ++h)
				{
					float fHarmonic = pow(2.0f, h);
					
					pRegion[iRegionIndex] += brush[BRUSH_INDEX(x, y)] * fScale * M_Noise2((fRX + x) / fHarmonic, (fRY + y) / fHarmonic) * fHarmonic;
				}
			}

			++iRegionIndex;
		}
	}
}


/*====================
  CDeformTool::TerrainClone
  ====================*/
void	CDeformTool::TerrainClone(float *pRegion, CRecti &recArea, CBrush &brush, float fScale, int iTileX, int iTileY)
{
	int iBrushSize(brush.GetBrushSize());

	int iGridWidth(Editor.GetWorld().GetGridWidth());
	int iGridHeight(Editor.GetWorld().GetGridHeight());

	int iRegionIndex(0);
	for (int y(0); y < recArea.GetHeight(); ++y)
	{
		for (int x(0); x < recArea.GetWidth(); ++x)
		{
			if (CanPantVert(x, y))
			{
				float fTarget(Editor.GetWorld().GetGridPoint(iGridWidth - (iTileX + x) - 1, iGridHeight - (iTileY + y) - 1));

				if (pRegion[iRegionIndex] > fTarget)
				{
					pRegion[iRegionIndex] -= brush[BRUSH_INDEX(x, y)] * fScale;

					if (pRegion[iRegionIndex] < fTarget)
						pRegion[iRegionIndex] = fTarget;
				}
				else if(pRegion[iRegionIndex] < fTarget)
				{
					pRegion[iRegionIndex] += brush[BRUSH_INDEX(x, y)] * fScale;

					if (pRegion[iRegionIndex] > fTarget)
						pRegion[iRegionIndex] = fTarget;
				}
			}

			++iRegionIndex;
		}
	}
}

const int STAMP_SIZE(9);

float g_fStamp[][STAMP_SIZE * STAMP_SIZE] =
{
	{
		0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f,
		0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f,
		0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f,
		0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f,
		0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f,
		0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f,
		0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f,
		0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f,
		0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f, 0.000f,
	},

	{
		8.0f / 8.0f, 8.0f / 8.0f, 8.0f / 8.0f, 8.0f / 8.0f, 8.0f / 8.0f, 8.0f / 8.0f, 8.0f / 8.0f, 8.0f / 8.0f, 8.0f / 8.0f,
		7.0f / 8.0f, 7.0f / 8.0f, 7.0f / 8.0f, 7.0f / 8.0f, 7.0f / 8.0f, 7.0f / 8.0f, 7.0f / 8.0f, 7.0f / 8.0f, 7.0f / 8.0f,
		6.0f / 8.0f, 6.0f / 8.0f, 6.0f / 8.0f, 6.0f / 8.0f, 6.0f / 8.0f, 6.0f / 8.0f, 6.0f / 8.0f, 6.0f / 8.0f, 6.0f / 8.0f,
		5.0f / 8.0f, 5.0f / 8.0f, 5.0f / 8.0f, 5.0f / 8.0f, 5.0f / 8.0f, 5.0f / 8.0f, 5.0f / 8.0f, 5.0f / 8.0f, 5.0f / 8.0f,
		4.0f / 8.0f, 4.0f / 8.0f, 4.0f / 8.0f, 4.0f / 8.0f, 4.0f / 8.0f, 4.0f / 8.0f, 4.0f / 8.0f, 4.0f / 8.0f, 4.0f / 8.0f,
		3.0f / 8.0f, 3.0f / 8.0f, 3.0f / 8.0f, 3.0f / 8.0f, 3.0f / 8.0f, 3.0f / 8.0f, 3.0f / 8.0f, 3.0f / 8.0f, 3.0f / 8.0f,
		2.0f / 8.0f, 2.0f / 8.0f, 2.0f / 8.0f, 2.0f / 8.0f, 2.0f / 8.0f, 2.0f / 8.0f, 2.0f / 8.0f, 2.0f / 8.0f, 2.0f / 8.0f,
		1.0f / 8.0f, 1.0f / 8.0f, 1.0f / 8.0f, 1.0f / 8.0f, 1.0f / 8.0f, 1.0f / 8.0f, 1.0f / 8.0f, 1.0f / 8.0f, 1.0f / 8.0f,
		0.0f / 8.0f, 0.0f / 8.0f, 0.0f / 8.0f, 0.0f / 8.0f, 0.0f / 8.0f, 0.0f / 8.0f, 0.0f / 8.0f, 0.0f / 8.0f, 0.0f / 8.0f
	},

	{
		0.0f / 8.0f, 0.0f / 8.0f, 0.0f / 8.0f, 0.0f / 8.0f, 0.0f / 8.0f, 0.0f / 8.0f, 0.0f / 8.0f, 0.0f / 8.0f, 0.0f / 8.0f,
		1.0f / 8.0f, 1.0f / 8.0f, 1.0f / 8.0f, 1.0f / 8.0f, 1.0f / 8.0f, 1.0f / 8.0f, 1.0f / 8.0f, 1.0f / 8.0f, 1.0f / 8.0f,
		2.0f / 8.0f, 2.0f / 8.0f, 2.0f / 8.0f, 2.0f / 8.0f, 2.0f / 8.0f, 2.0f / 8.0f, 2.0f / 8.0f, 2.0f / 8.0f, 2.0f / 8.0f,
		3.0f / 8.0f, 3.0f / 8.0f, 3.0f / 8.0f, 3.0f / 8.0f, 3.0f / 8.0f, 3.0f / 8.0f, 3.0f / 8.0f, 3.0f / 8.0f, 3.0f / 8.0f,
		4.0f / 8.0f, 4.0f / 8.0f, 4.0f / 8.0f, 4.0f / 8.0f, 4.0f / 8.0f, 4.0f / 8.0f, 4.0f / 8.0f, 4.0f / 8.0f, 4.0f / 8.0f,
		5.0f / 8.0f, 5.0f / 8.0f, 5.0f / 8.0f, 5.0f / 8.0f, 5.0f / 8.0f, 5.0f / 8.0f, 5.0f / 8.0f, 5.0f / 8.0f, 5.0f / 8.0f,
		6.0f / 8.0f, 6.0f / 8.0f, 6.0f / 8.0f, 6.0f / 8.0f, 6.0f / 8.0f, 6.0f / 8.0f, 6.0f / 8.0f, 6.0f / 8.0f, 6.0f / 8.0f,
		7.0f / 8.0f, 7.0f / 8.0f, 7.0f / 8.0f, 7.0f / 8.0f, 7.0f / 8.0f, 7.0f / 8.0f, 7.0f / 8.0f, 7.0f / 8.0f, 7.0f / 8.0f,
		8.0f / 8.0f, 8.0f / 8.0f, 8.0f / 8.0f, 8.0f / 8.0f, 8.0f / 8.0f, 8.0f / 8.0f, 8.0f / 8.0f, 8.0f / 8.0f, 8.0f / 8.0f
	},
	
	{
		8.0f / 8.0f, 7.0f / 8.0f, 6.0f / 8.0f, 5.0f / 8.0f, 4.0f / 8.0f, 3.0f / 8.0f, 2.0f / 8.0f, 1.0f / 8.0f, 0.0f / 8.0f,
		8.0f / 8.0f, 7.0f / 8.0f, 6.0f / 8.0f, 5.0f / 8.0f, 4.0f / 8.0f, 3.0f / 8.0f, 2.0f / 8.0f, 1.0f / 8.0f, 0.0f / 8.0f,
		8.0f / 8.0f, 7.0f / 8.0f, 6.0f / 8.0f, 5.0f / 8.0f, 4.0f / 8.0f, 3.0f / 8.0f, 2.0f / 8.0f, 1.0f / 8.0f, 0.0f / 8.0f,
		8.0f / 8.0f, 7.0f / 8.0f, 6.0f / 8.0f, 5.0f / 8.0f, 4.0f / 8.0f, 3.0f / 8.0f, 2.0f / 8.0f, 1.0f / 8.0f, 0.0f / 8.0f,
		8.0f / 8.0f, 7.0f / 8.0f, 6.0f / 8.0f, 5.0f / 8.0f, 4.0f / 8.0f, 3.0f / 8.0f, 2.0f / 8.0f, 1.0f / 8.0f, 0.0f / 8.0f,
		8.0f / 8.0f, 7.0f / 8.0f, 6.0f / 8.0f, 5.0f / 8.0f, 4.0f / 8.0f, 3.0f / 8.0f, 2.0f / 8.0f, 1.0f / 8.0f, 0.0f / 8.0f,
		8.0f / 8.0f, 7.0f / 8.0f, 6.0f / 8.0f, 5.0f / 8.0f, 4.0f / 8.0f, 3.0f / 8.0f, 2.0f / 8.0f, 1.0f / 8.0f, 0.0f / 8.0f,
		8.0f / 8.0f, 7.0f / 8.0f, 6.0f / 8.0f, 5.0f / 8.0f, 4.0f / 8.0f, 3.0f / 8.0f, 2.0f / 8.0f, 1.0f / 8.0f, 0.0f / 8.0f,
		8.0f / 8.0f, 7.0f / 8.0f, 6.0f / 8.0f, 5.0f / 8.0f, 4.0f / 8.0f, 3.0f / 8.0f, 2.0f / 8.0f, 1.0f / 8.0f, 0.0f / 8.0f
	},

	{
		0.0f / 8.0f, 1.0f / 8.0f, 2.0f / 8.0f, 3.0f / 8.0f, 4.0f / 8.0f, 5.0f / 8.0f, 6.0f / 8.0f, 7.0f / 8.0f, 8.0f / 8.0f,
		0.0f / 8.0f, 1.0f / 8.0f, 2.0f / 8.0f, 3.0f / 8.0f, 4.0f / 8.0f, 5.0f / 8.0f, 6.0f / 8.0f, 7.0f / 8.0f, 8.0f / 8.0f,
		0.0f / 8.0f, 1.0f / 8.0f, 2.0f / 8.0f, 3.0f / 8.0f, 4.0f / 8.0f, 5.0f / 8.0f, 6.0f / 8.0f, 7.0f / 8.0f, 8.0f / 8.0f,
		0.0f / 8.0f, 1.0f / 8.0f, 2.0f / 8.0f, 3.0f / 8.0f, 4.0f / 8.0f, 5.0f / 8.0f, 6.0f / 8.0f, 7.0f / 8.0f, 8.0f / 8.0f,
		0.0f / 8.0f, 1.0f / 8.0f, 2.0f / 8.0f, 3.0f / 8.0f, 4.0f / 8.0f, 5.0f / 8.0f, 6.0f / 8.0f, 7.0f / 8.0f, 8.0f / 8.0f,
		0.0f / 8.0f, 1.0f / 8.0f, 2.0f / 8.0f, 3.0f / 8.0f, 4.0f / 8.0f, 5.0f / 8.0f, 6.0f / 8.0f, 7.0f / 8.0f, 8.0f / 8.0f,
		0.0f / 8.0f, 1.0f / 8.0f, 2.0f / 8.0f, 3.0f / 8.0f, 4.0f / 8.0f, 5.0f / 8.0f, 6.0f / 8.0f, 7.0f / 8.0f, 8.0f / 8.0f,
		0.0f / 8.0f, 1.0f / 8.0f, 2.0f / 8.0f, 3.0f / 8.0f, 4.0f / 8.0f, 5.0f / 8.0f, 6.0f / 8.0f, 7.0f / 8.0f, 8.0f / 8.0f,
		0.0f / 8.0f, 1.0f / 8.0f, 2.0f / 8.0f, 3.0f / 8.0f, 4.0f / 8.0f, 5.0f / 8.0f, 6.0f / 8.0f, 7.0f / 8.0f, 8.0f / 8.0f
	}
};


/*====================
  CDeformTool::TerrainStamp
  ====================*/
void	CDeformTool::TerrainStamp(float *pRegion, CRecti &recArea)
{
	int iRegionIndex(0);
	for (int y(recArea.top); y < recArea.bottom; ++y)
	{
		for (int x(recArea.left); x < recArea.right; ++x)
		{
			if(CanPantVert(x, y))
			{
				pRegion[iRegionIndex] = g_fStamp[le_deformStamp][y * STAMP_SIZE + x] * le_deformStampScale + le_deformStampHeight;
			}

			++iRegionIndex;
		}
	}
}


/*====================
  CDeformTool::DeformTerrain
  ====================*/
void	CDeformTool::DeformTerrain(float fFrameTime)
{
	float *pRegion(NULL);

	try
	{
		CBrush *pBrush(CBrush::GetCurrentBrush());
		if (pBrush == NULL)
			EX_ERROR(_T("No brush selected"));

		//if (!Editor.GetWorld().IsInBounds(m_iX, m_iY, GRID_SPACE))
		//	EX_WARN(_T("Out of bounds coordinate"));

		// Clip against the brush data
		CRecti	recClippedBrush;
		int d(-STAMP_SIZE / 2);

		if (le_deformMode == DEFORM_STAMP)
		{
			recClippedBrush.Set(d, d, d + STAMP_SIZE, d + STAMP_SIZE);
		}
		else
		{
			if (!pBrush->ClipBrush(recClippedBrush))
				return;
		}

		// Clip the brush against the world
		if (le_deformMode == DEFORM_STAMP)
		{
			recClippedBrush.Shift(m_iXOffset, m_iYOffset);
			if (!Editor.GetWorld().ClipRect(recClippedBrush, GRID_SPACE))
				return;
		}
		else
		{
			recClippedBrush.Shift(m_iXOffset, m_iYOffset);
			if (!Editor.GetWorld().ClipRect(recClippedBrush, GRID_SPACE))
				return;
		}

		// Get the region
		pRegion = K2_NEW_ARRAY(ctx_Editor, float, recClippedBrush.GetArea());
		if (pRegion == NULL)
			EX_ERROR(_T("Failed to allocate region"));

		if (!Editor.GetWorld().GetRegion(WORLD_VERT_HEIGHT_MAP, recClippedBrush, pRegion))
			EX_ERROR(_T("Failed to retrieve region"));

		//m_iXOffset = recClippedBrush.left;
		//m_iYOffset = recClippedBrush.top;

		// Perform the operation
		if (le_deformMode == DEFORM_STAMP)
			recClippedBrush.Shift(-(m_iXOffset + d), -(m_iYOffset + d));
		else
			recClippedBrush.Shift(-(m_iXOffset), -(m_iYOffset));
		
		float fScale(fFrameTime * le_deformBrushStrength / 50.0f);
		float fInverse(m_bInverse ? -1.0f : 1.0f);
		switch(le_deformMode)
		{
		case DEFORM_ADD:
			TerrainAdd(pRegion, recClippedBrush, *pBrush, fScale * fInverse);
			break;

		case DEFORM_FLATTEN:
			TerrainFlatten(pRegion, recClippedBrush, *pBrush, fScale, le_deformFlattenHeight);
			break;

		case DEFORM_SMOOTH:
			TerrainSmooth(pRegion, recClippedBrush, *pBrush, fScale / 50.0f);
			break;

		case DEFORM_CUT:
			TerrainCut(pRegion, recClippedBrush, *pBrush, fScale, le_deformCutHeight);
			break;

		case DEFORM_SLICE:
			TerrainSlice(pRegion, recClippedBrush, *pBrush, le_deformSliceHeight);
			break;

		case DEFORM_CLEAR:
			TerrainFlatten(pRegion, recClippedBrush, *pBrush, fScale, le_deformClearHeight);
			break;

		case DEFORM_REDUCE:
			TerrainReduce(pRegion, recClippedBrush, *pBrush, fScale * fInverse);
			break;

		case DEFORM_NOISE:
			TerrainNoise(pRegion, recClippedBrush, *pBrush, fScale * fInverse, m_fRX + m_iXOffset, m_fRY + m_iYOffset, le_deformNoiseOctave);
			break;

		case DEFORM_HARMONIC:
			TerrainHarmonic(pRegion, recClippedBrush, *pBrush, fScale * fInverse, m_fRX + m_iXOffset, m_fRY + m_iYOffset, le_deformHarmonics);
			break;

		case DEFORM_CLONE:
			TerrainClone(pRegion, recClippedBrush, *pBrush, fScale * fInverse, m_iXOffset - pBrush->GetBrushSize() / 2, m_iYOffset - pBrush->GetBrushSize() / 2);
			break;

		case DEFORM_STAMP:
			TerrainStamp(pRegion, recClippedBrush);
			m_bWorking = false;
			break;
		}

		// Apply the modified region
		if (le_deformMode == DEFORM_STAMP)
			recClippedBrush.Shift(m_iXOffset + d, m_iYOffset + d);
		else
			recClippedBrush.Shift(m_iXOffset, m_iYOffset);

		if (!Editor.GetWorld().SetRegion(WORLD_VERT_HEIGHT_MAP, recClippedBrush, pRegion))
			EX_ERROR(_T("SetRegion failed"));

		// Notify the video drivers about the update
		for (int y(recClippedBrush.top); y < recClippedBrush.bottom; ++y)
		{
			for (int x(recClippedBrush.left); x < recClippedBrush.right; ++x)
				Vid.Notify(VID_NOTIFY_TERRAIN_VERTEX_MODIFIED, x, y, 0, &Editor.GetWorld());
		}

		for (int y(recClippedBrush.top - 1); y < recClippedBrush.bottom; ++y)
		{
			for (int x(recClippedBrush.left - 1); x < recClippedBrush.right; ++x)
				Vid.Notify(VID_NOTIFY_TERRAIN_SHADER_MODIFIED, x, y, 0, &Editor.GetWorld());
		}

		for (int y(recClippedBrush.top - 2); y < recClippedBrush.bottom + 3; ++y)
		{
			for (int x(recClippedBrush.left - 2); x < recClippedBrush.right + 3; ++x)
				Vid.Notify(VID_NOTIFY_TERRAIN_NORMAL_MODIFIED, x, y, 0, &Editor.GetWorld());
		}

		K2_DELETE_ARRAY(pRegion);
	}
	catch (CException &ex)
	{
		if (pRegion != NULL)
			K2_DELETE_ARRAY(pRegion);

		ex.Process(_T("CDeformTool::DeformTerrain() - "), NO_THROW);
	}
}


/*====================
  CDeformTool::Frame
 ====================*/
void	CDeformTool::Frame(float fFrameTime)
{
	CalcToolProperties();

	if (m_bWorking && m_bValidPosition)
		DeformTerrain(fFrameTime);
}


/*====================
  CDeformTool::Draw
  ====================*/
void	CDeformTool::Draw()
{
	if (le_deformDrawBrushCoords)
	{
		CFontMap *pFontMap(g_ResourceManager.GetFontMap(m_hFont));
		
		CVec3f v3Pos(m_v3EndPos);

		if (m_bValidPosition)
			v3Pos.z = Editor.GetWorld().GetTerrainHeight(v3Pos.x, v3Pos.y);

		Draw2D.SetColor(0.0f, 0.0f, 0.0f);
		Draw2D.String(4.0f, Draw2D.GetScreenH() - pFontMap->GetMaxHeight() - 1.0f, Draw2D.GetScreenW(), pFontMap->GetMaxHeight(), XtoA(v3Pos), m_hFont);
		Draw2D.SetColor(1.0f, 1.0f, 1.0f);
		Draw2D.String(3.0f, Draw2D.GetScreenH() - pFontMap->GetMaxHeight() - 2.0f, Draw2D.GetScreenW(), pFontMap->GetMaxHeight(), XtoA(v3Pos), m_hFont);
	}
}


/*====================
  CDeformTool::RenderBrush
  ====================*/
void	CDeformTool::RenderBrush()
{
	CBrush *pBrush = CBrush::GetCurrentBrush();
	float fTileSize = Editor.GetWorld().GetScale();

	if (!le_deformDrawBrushInfluence || !pBrush || !m_bValidPosition)
		return;

	int iX = m_iXOffset, iY = m_iYOffset;

	SSceneFaceVert poly[1024];
	MemManager.Set(poly, 0, sizeof(poly));
	int p = 0;

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
			int dX = iX + x;
			int dY = iY + y;
			if (!Editor.GetWorld().IsInBounds(dX, dY, GRID_SPACE))
				continue;

			// left
			if (dY < Editor.GetWorld().GetGridHeight() - 1)
			{
				byte yAlpha0 = INT_FLOOR((*pBrush)[i] * le_deformBrushInfluenceAlpha);
				byte yAlpha1 = INT_FLOOR((*pBrush)[i + pBrush->GetBrushSize()] * le_deformBrushInfluenceAlpha);

				if (yAlpha0 || yAlpha1)
				{
					poly[p].vtx[0] = dX * fTileSize;
					poly[p].vtx[1] = dY * fTileSize;
					poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(dX, dY);
					SET_VEC4(poly[p].col, 0, 255, 0, yAlpha0);
					++p;

					poly[p].vtx[0] = dX * fTileSize;
					poly[p].vtx[1] = (dY + 1) * fTileSize;
					poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(dX, (dY + 1));
					SET_VEC4(poly[p].col, 0, 255, 0, yAlpha1);
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
				byte yAlpha0 = INT_FLOOR((*pBrush)[i] * le_deformBrushInfluenceAlpha);
				byte yAlpha1 = INT_FLOOR((*pBrush)[i + 1] * le_deformBrushInfluenceAlpha);

				if (yAlpha0 || yAlpha1)
				{
					poly[p].vtx[0] = dX * fTileSize;
					poly[p].vtx[1] = dY * fTileSize;
					poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(dX, dY);
					SET_VEC4(poly[p].col, 0, 255, 0, yAlpha0);
					++p;

					poly[p].vtx[0] = (dX + 1) * fTileSize;
					poly[p].vtx[1] = dY * fTileSize;
					poly[p].vtx[2] = Editor.GetWorld().GetGridPoint((dX + 1), dY);
					SET_VEC4(poly[p].col, 0, 255, 0, yAlpha1);
					++p;
				}
			}
		}
	}

	if (p > 0)
		SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
}


/*====================
  CDeformTool::RenderStamp
  ====================*/
void	CDeformTool::RenderStamp()
{
	float fTileSize = Editor.GetWorld().GetScale();

	if (!le_deformDrawBrushInfluence || !m_bValidPosition)
		return;

	int iX = m_iXOffset, iY = m_iYOffset;

	SSceneFaceVert poly[1024];
	MemManager.Set(poly, 0, sizeof(poly));
	int p = 0;

	for (int y = 0; y < STAMP_SIZE - 1; ++y)
	{
		for (int x = 0; x < STAMP_SIZE - 1; ++x)
		{
			if (p >= 1024) // restart batch if we overflow
			{
				SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
				MemManager.Set(poly, 0, sizeof(poly));
				p = 0;
			}

			int dX = iX + x - STAMP_SIZE / 2;
			int dY = iY + y - STAMP_SIZE / 2;
			if (!Editor.GetWorld().IsInBounds(dX, dY, GRID_SPACE))
				continue;

			// left
			if (dY < Editor.GetWorld().GetGridHeight() - 1)
			{
				byte yAlpha0 = 255;
				byte yAlpha1 = 255;

				if (yAlpha0 || yAlpha1)
				{
					poly[p].vtx[0] = dX * fTileSize;
					poly[p].vtx[1] = dY * fTileSize;
					poly[p].vtx[2] = g_fStamp[le_deformStamp][y * STAMP_SIZE + x] * le_deformStampScale + le_deformStampHeight;
					SET_VEC4(poly[p].col, 0, 255, 0, yAlpha0);
					++p;

					poly[p].vtx[0] = dX * fTileSize;
					poly[p].vtx[1] = (dY + 1) * fTileSize;
					poly[p].vtx[2] = g_fStamp[le_deformStamp][(y + 1) * STAMP_SIZE + x] * le_deformStampScale + le_deformStampHeight;
					SET_VEC4(poly[p].col, 0, 255, 0, yAlpha1);
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
				byte yAlpha0 = 255;
				byte yAlpha1 = 255;

				if (yAlpha0 || yAlpha1)
				{
					poly[p].vtx[0] = dX * fTileSize;
					poly[p].vtx[1] = dY * fTileSize;
					poly[p].vtx[2] = g_fStamp[le_deformStamp][y * STAMP_SIZE + x] * le_deformStampScale + le_deformStampHeight;
					SET_VEC4(poly[p].col, 0, 255, 0, yAlpha0);
					++p;

					poly[p].vtx[0] = (dX + 1) * fTileSize;
					poly[p].vtx[1] = dY * fTileSize;
					poly[p].vtx[2] = g_fStamp[le_deformStamp][y * STAMP_SIZE + x + 1] * le_deformStampScale + le_deformStampHeight;
					SET_VEC4(poly[p].col, 0, 255, 0, yAlpha1);
					++p;
				}
			}
		}
	}

	if (p > 0)
		SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
}


/*====================
  CDeformTool::Render
  ====================*/
void	CDeformTool::Render()
{
	if (le_deformMode == DEFORM_STAMP)
		RenderStamp();
	else
		RenderBrush();
}


/*--------------------
  cmdSetDeformMode
  --------------------*/
UI_VOID_CMD(SetDeformMode, 1)
{
	if (vArgList.size() < 1)
	{
		Console << _T("syntax: deformmode raiselower|add|flatten|smooth|cut|slice|clear|reduce|noise|harmonic|clone|stamp") << newl;
		return;
	}

	tstring sValue(vArgList[0]->Evaluate());

	if (sValue == _T("raiselower") || sValue == _T("add"))
	{
		le_deformMode = DEFORM_ADD;
		DeformMode.Trigger(_T("Raise/Lower"));
		return;
	}
	else if (sValue == _T("flatten"))
	{
		le_deformMode = DEFORM_FLATTEN;
		DeformMode.Trigger(_T("Flatten"));
		return;
	}
	else if (sValue == _T("smooth"))
	{
		le_deformMode = DEFORM_SMOOTH;
		DeformMode.Trigger(_T("Smooth"));
		return;
	}
	else if (sValue == _T("cut"))
	{
		le_deformMode = DEFORM_CUT;
		DeformMode.Trigger(_T("Cut"));
		return;
	}
	else if (sValue == _T("slice"))
	{
		le_deformMode = DEFORM_SLICE;
		DeformMode.Trigger(_T("Slice"));
		return;
	}
	else if (sValue == _T("clear"))
	{
		le_deformMode = DEFORM_CLEAR;
		DeformMode.Trigger(_T("Clear"));
		return;
	}
	else if (sValue == _T("reduce"))
	{
		le_deformMode = DEFORM_REDUCE;
		DeformMode.Trigger(_T("Reduce"));
		return;
	}
	else if (sValue == _T("noise"))
	{
		le_deformMode = DEFORM_NOISE;
		DeformMode.Trigger(_T("Noise"));
		return;
	}
	else if (sValue == _T("harmonic"))
	{
		le_deformMode = DEFORM_HARMONIC;
		DeformMode.Trigger(_T("Harmonic"));
		return;
	}
	else if (sValue == _T("clone"))
	{
		le_deformMode = DEFORM_CLONE;
		DeformMode.Trigger(_T("Clone"));
		return;
	}
	else if (sValue == _T("stamp"))
	{
		le_deformMode = DEFORM_STAMP;
		DeformMode.Trigger(_T("Stamp"));
		return;
	}
}


/*--------------------
  cmdImportHeightmap
  --------------------*/
CMD(ImportHeightmap)
{
	if (vArgList.size() < 3)
	{
		Console << _T("syntax: ImportHeightmap <filename> <low> <high> <pixel>") << newl;
		return false;
	}

	CBitmap heightmap(vArgList[0]);

	if (heightmap.GetWidth() != Editor.GetWorld().GetGridWidth() ||
		heightmap.GetHeight() != Editor.GetWorld().GetGridHeight())
	{
		Console << _T("Invalid dimensions, should be ") << Editor.GetWorld().GetGridWidth() << _T(", ") << Editor.GetWorld().GetGridHeight() << newl;
		return false;
	}

	float fLow(AtoF(vArgList[1])), fHigh(AtoF(vArgList[2])), fPixel(vArgList.size() > 3 ? AtoF(vArgList[3]) : 255.0f);

	CRecti recWorld(0, 0, Editor.GetWorld().GetGridWidth(), Editor.GetWorld().GetGridHeight());

	float *pRegion(K2_NEW_ARRAY(ctx_Editor, float, recWorld.GetArea()));

	if (Editor.GetWorld().GetRegion(WORLD_VERT_HEIGHT_MAP, recWorld, pRegion))
	{
		for (int iY(0); iY < heightmap.GetHeight(); ++iY)
		{
			for (int iX(0); iX < heightmap.GetWidth(); ++iX)
			{
				int iIndex(iY * heightmap.GetWidth() + iX);

				CVec4b v4Color(heightmap.GetColor(iX, iY));

				byte yMax(MAX(MAX(v4Color[R], v4Color[G]), v4Color[B]));

				pRegion[iIndex] = LERP(yMax / fPixel, fLow, fHigh);
			}
		}

		Editor.GetWorld().SetRegion(WORLD_VERT_HEIGHT_MAP, recWorld, pRegion);

		Vid.Notify(VID_NOTIFY_TERRAIN_VERTEX_MODIFIED, 0, 0, 1, &Editor.GetWorld());
	}

	K2_DELETE_ARRAY(pRegion);
	return true;
}


/*--------------------
  cmdImportHeightmapSilverback
  --------------------*/
CMD(ImportHeightmapSilverback)
{
	if (vArgList.size() < 2)
	{
		Console << _T("syntax: ImportHeightmapSavage1 <filename> <height>") << newl;
		return false;
	}
	
	float fHeight(AtoF(vArgList[1]));

	CWorld &cWorld(Editor.GetWorld());

	CFileHandle hFile(vArgList[0], FILE_READ);

	uint uiWidth(hFile.ReadInt32() + 1);
	uint uiHeight(hFile.ReadInt32() + 1);

	float *pSilverback(K2_NEW_ARRAY(ctx_Editor, float, uiWidth*uiHeight));

	for (uint iY(0); iY < uiHeight - 1; ++iY)
	{
		for (uint iX(0); iX < uiWidth - 1; ++iX)
		{
			int iIndex(iY * uiWidth + iX);
			pSilverback[iIndex] = LittleFloat(hFile.ReadFloat()) * fHeight;
		}

		int iIndex0(iY * uiWidth + uiWidth - 2);
		int iIndex1(iY * uiWidth + uiWidth - 1);

		// Duplicate last column
		pSilverback[iIndex1] = pSilverback[iIndex0];
	}

	// Duplicate last row
	for (uint iX(0); iX < uiWidth; ++iX)
	{
		int iIndex0((uiHeight - 2) * uiWidth + iX);
		int iIndex1((uiHeight - 1) * uiWidth + iX);
		
		pSilverback[iIndex1] = pSilverback[iIndex0];
	}

	CRecti recWorld(0, 0, cWorld.GetGridWidth(), cWorld.GetGridHeight());
	float *pK2(K2_NEW_ARRAY(ctx_Editor, float, recWorld.GetArea()));
	
	float fTileScaleX(float(uiWidth - 1) / cWorld.GetTileWidth());
	float fTileScaleY(float(uiHeight - 1) / cWorld.GetTileHeight());

	if (cWorld.GetRegion(WORLD_VERT_HEIGHT_MAP, recWorld, pK2))
	{
		for (int iY(0); iY < cWorld.GetGridHeight(); ++iY)
		{
			for (int iX(0); iX < cWorld.GetGridWidth(); ++iX)
			{
				int iTileX(INT_FLOOR(iX * fTileScaleX));
				int iTileY(INT_FLOOR(iY * fTileScaleY));

				float fLerps[2] =
				{
					FRAC(iX * fTileScaleX),
					FRAC(iY * fTileScaleY)
				};

				float afHeight[4] =
				{
					pSilverback[iTileY * uiWidth + iTileX],
					pSilverback[iTileY * uiWidth + iTileX + 1],
					pSilverback[(iTileY + 1) * uiWidth + iTileX],
					pSilverback[(iTileY + 1) * uiWidth + iTileX + 1]
				};

				int iIndex(iY * cWorld.GetGridWidth() + iX);
				pK2[iIndex] = PCF(fLerps, afHeight);
			}
		}

		Editor.GetWorld().SetRegion(WORLD_VERT_HEIGHT_MAP, recWorld, pK2);

		Vid.Notify(VID_NOTIFY_TERRAIN_VERTEX_MODIFIED, 0, 0, 1, &Editor.GetWorld());
	}

	K2_DELETE_ARRAY(pSilverback);
	K2_DELETE_ARRAY(pK2);

	hFile.Close();

	return true;
}


/*--------------------
  cmdExportHeightmap
  --------------------*/
CMD(ExportHeightmap)
{
	if (vArgList.size() < 3)
	{
		Console << _T("syntax: ExportHeightmap <filename> <low> <high>") << newl;
		return false;
	}

	CBitmap heightmap(Editor.GetWorld().GetGridWidth(), Editor.GetWorld().GetGridHeight(), BITMAP_RGB);

	float fLow(AtoF(vArgList[1])), fHigh(AtoF(vArgList[2]));

	CRecti recWorld(0, 0, Editor.GetWorld().GetGridWidth(), Editor.GetWorld().GetGridHeight());

	float *pRegion(K2_NEW_ARRAY(ctx_Editor, float, recWorld.GetArea()));

	if (Editor.GetWorld().GetRegion(WORLD_VERT_HEIGHT_MAP, recWorld, pRegion))
	{
		for (int iY(0); iY < heightmap.GetHeight(); ++iY)
		{
			for (int iX(0); iX < heightmap.GetWidth(); ++iX)
			{
				int iIndex(iY * heightmap.GetWidth() + iX);

				byte yHeight = CLAMP(INT_ROUND(ILERP(pRegion[iIndex], fLow, fHigh) * 255), 0, 255);

				heightmap.SetPixel4b(iX, iY, yHeight, yHeight, yHeight, 0);
			}
		}
	}

	heightmap.WritePNG(_TS("~/") + vArgList[0]);

	K2_DELETE_ARRAY(pRegion);
	return true;
}
