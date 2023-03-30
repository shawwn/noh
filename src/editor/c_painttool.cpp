// (C)2005 S2 Games
// c_painttool.cpp
//
// Terrain painter
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "editor_common.h"

#include "editor.h"

#include "c_painttool.h"

#include "../k2/c_brush.h"
#include "../k2/c_world.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_vid.h"
#include "../k2/c_materialbrush.h"
#include "../k2/s_tile.h"
#include "../k2/c_texture.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_draw2d.h"
#include "../k2/c_fontmap.h"
#include "../k2/c_uitrigger.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_FLOAT	(le_paintR,								1.0f);
CVAR_FLOAT	(le_paintG,								0.0f);
CVAR_FLOAT	(le_paintB,								0.0f);
CVAR_FLOAT	(le_paintA,								1.0f);
CVAR_FLOAT	(le_paintR2,							1.0f);
CVAR_FLOAT	(le_paintG2,							1.0f);
CVAR_FLOAT	(le_paintB2,							1.0f);
CVAR_FLOAT	(le_paintA2,							1.0f);
CVAR_FLOAT	(le_paintBrushStrength,					128.0f);
CVAR_BOOL	(le_paintDrawBrushInfluence,			true);
CVAR_FLOAT	(le_paintBrushInfluenceAlpha,			1.0f);
CVAR_INT	(le_paintMode,							PAINT_COLOR);
CVAR_STRING	(le_paintDiffuseTexture,				"/world/terrain/textures/grass1_d.tga");
CVAR_STRING	(le_paintNormalmapTexture,				"/world/terrain/textures/grass1_n.tga");
CVAR_STRING	(le_paintMaterial,						"/world/terrain/materials/default.material");
CVAR_INT	(le_paintLayer,							0);
CVAR_BOOL	(le_showNullTiles,						0);
CVAR_BOOL	(le_flipNullTiles,						0);
CVAR_FLOAT	(le_paintTextureScale,					8.0f);
CVAR_FLOAT	(le_paintTextureRotation,				0.0f);
CVAR_FLOAT	(le_paintTextureOffsetX,				0.0f);
CVAR_FLOAT	(le_paintTextureOffsetY,				0.0f);
CVAR_FLOAT	(le_paintTextureBrushInfluenceAlpha,	1.0f);
CVAR_FLOAT	(le_paintTexelAlpha,					1.0f);
CVAR_FLOAT	(le_paintTexelAlpha2,					0.0f);
CVAR_BOOLF	(le_paintDrawBrushCoords,				true,	CVAR_SAVECONFIG);
CVAR_BOOLF	(le_paintDrawInfo,						true,	CVAR_SAVECONFIG);

UI_TRIGGER(PaintMode);
UI_TRIGGER(PaintDiffuseTexture);
UI_TRIGGER(PaintNormalmapTexture);
//=============================================================================

/*====================
  CPaintTool::CPaintTool()
  ====================*/
CPaintTool::CPaintTool() :
ITool(TOOL_PAINT, _T("paint")),
m_pMaterialBrush(NULL),
m_hMaterial(INVALID_RESOURCE),
m_hDiffuse(INVALID_RESOURCE),
m_hNormalmap(INVALID_RESOURCE),
m_hLineMaterial(g_ResourceManager.Register(_T("/core/materials/line.material"), RES_MATERIAL)),
m_hFont(g_ResourceManager.LookUpName(_T("system_medium"), RES_FONTMAP)),
m_bWorking(false),
m_iX(0),
m_iLastTimer(0),
m_iY(0),
m_bGrid(0),
m_iTexelX(0),
m_iTexelY(0),
m_iGridWidth(0),
m_iGridHeight(0),
m_rhTemp(0),
m_bFlip(false),
m_bValidPosition(false),
m_bInverse(false)
{
	PaintMode.Trigger(_T("Color"));

	le_paintMaterial.SetModified(true);
	le_paintDiffuseTexture.SetModified(true);
	le_paintNormalmapTexture.SetModified(true);
}


/*====================
  CPaintTool::PrimaryUp
  Left mouse button up action
  ====================*/
void	CPaintTool::PrimaryUp()
{
	if (!m_bInverse)
		m_bWorking = false;
}


/*====================
  CPaintTool::PrimaryDown
  Default left mouse button down action
  ====================*/
void	CPaintTool::PrimaryDown()
{
	CalcToolProperties();

	// Sample current value on CTRL
	// Sample current value and break on ALT
	if (m_bModifier3 || m_bModifier2)
	{
		CWorld &oWorld(Editor.GetWorld());
		const float fTileSize(float(oWorld.GetScale()));

		int iTileX(oWorld.GetTileFromCoord(m_v3EndPos.x));
		int iTileY(oWorld.GetTileFromCoord(m_v3EndPos.y));

		float fLerps[2] =
		{
			FRAC(m_v3EndPos.x / fTileSize),
			FRAC(m_v3EndPos.y / fTileSize)
		};

		switch (le_paintMode)
		{
		case PAINT_COLOR:
			{
				CVec4b v4Colors[4] =
				{
					oWorld.GetGridColor(iTileX, iTileY),
					oWorld.GetGridColor(iTileX + 1, iTileY),
					oWorld.GetGridColor(iTileX, iTileY + 1),
					oWorld.GetGridColor(iTileX + 1, iTileY + 1)
				};

				byte yReds[4] =
				{
					v4Colors[0][R],
					v4Colors[1][R],
					v4Colors[2][R],
					v4Colors[3][R]
				};
				byte yPCFRed(PCF(fLerps, yReds));

				byte yGreens[4] =
				{
					v4Colors[0][G],
					v4Colors[1][G],
					v4Colors[2][G],
					v4Colors[3][G]
				};
				byte yPCFGreen(PCF(fLerps, yGreens));

				byte yBlues[4] =
				{
					v4Colors[0][B],
					v4Colors[1][B],
					v4Colors[2][B],
					v4Colors[3][B]
				};
				byte yPCFBlue(PCF(fLerps, yBlues));

				le_paintR = yPCFRed / 255.0f;
				le_paintG = yPCFGreen / 255.0f;
				le_paintB = yPCFBlue / 255.0f;
			}
			break;
		case PAINT_MATERIAL:
			break;
		case PAINT_TEXTURE:
			le_paintMaterial = g_ResourceManager.GetPath(oWorld.GetTileMaterial(iTileX, iTileY, le_paintLayer));
			le_paintNormalmapTexture = g_ResourceManager.GetPath(oWorld.GetTileNormalmapTexture(iTileX, iTileY, le_paintLayer));
			le_paintDiffuseTexture = g_ResourceManager.GetPath(oWorld.GetTileDiffuseTexture(iTileX, iTileY, le_paintLayer));
			break;
		case PAINT_ALPHA:
			break;
		}

		if (m_bModifier3)
			return;
	}

	m_bWorking = true;
	m_bInverse = false;
}


/*====================
  CPaintTool::SecondaryUp
  Right mouse button up action
  ====================*/
void	CPaintTool::SecondaryUp()
{
	le_showNullTiles = false;

	if (m_bInverse)
		m_bWorking = false;
}


/*====================
  CPaintTool::SecondaryDown

  Default right mouse button down action - not used in this case
  ====================*/
void	CPaintTool::SecondaryDown()
{
	if(le_paintLayer == 0 && le_paintMode == PAINT_TEXTURE)
	{
		m_iGridWidth = 0;
		m_iGridHeight = 0;
		le_showNullTiles = true;
	}

	m_bWorking = true;
	m_bInverse = true;
	CalcToolProperties();
}


/*====================
 CPaintTool::TertiaryUp

 Middle mouse button up action
 ====================*/
void	CPaintTool::TertiaryUp()
{
}


/*====================
  CPaintTool::TertiaryDown

  Middle mouse button down action
  ====================*/
void	CPaintTool::TertiaryDown()
{
}


/*====================
  CPaintTool::QuaternaryUp

  Scroll wheel up action
  ====================*/
void	CPaintTool::QuaternaryUp()
{
}


/*====================
  CPaintTool::QuaternaryDown

  Scroll wheel down action
  ====================*/
void	CPaintTool::QuaternaryDown()
{
}


/*====================
  CPaintTool::Cancel
  ====================*/
void	CPaintTool::Cancel()
{
}


/*====================
  CPaintTool::Delete
  ====================*/
void	CPaintTool::Delete()
{
}


/*====================
  CPaintTool::CalcToolProperties
  ====================*/
void	 CPaintTool::CalcToolProperties()
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

		fTestX = (trace.v3EndPos.x - fBrushCenterX * Editor.GetWorld().GetScale()) / (Editor.GetWorld().GetScale());
		fTestY = (trace.v3EndPos.y - fBrushCenterY * Editor.GetWorld().GetScale()) / (Editor.GetWorld().GetScale());

		m_iTexelX = INT_ROUND(fTestX);
		m_iTexelY = INT_ROUND(fTestY);

		m_bValidPosition = true;

		m_v3EndPos = trace.v3EndPos;
	}
	else
	{
		m_bValidPosition = false;
		m_iX = 0;
		m_iY = 0;
		m_v3EndPos.Clear();
	}

	/*if (Editor.TraceCursor(trace, TRACE_TERRAIN))
	{
		m_iX = Editor.GetWorld().GetVertFromCoord(trace.v3EndPos.x);
		m_iY = Editor.GetWorld().GetVertFromCoord(trace.v3EndPos.y);
		m_iTexelX = Editor.GetWorld().GetTexelFromCoord(trace.v3EndPos.x);
		m_iTexelY = Editor.GetWorld().GetTexelFromCoord(trace.v3EndPos.y);
		m_v3EndPos = trace.v3EndPos;
		m_bValidPosition = true;
	}
	else
	{
		m_bValidPosition = false;
		m_iX = -1;
		m_iY = -1;
		m_iTexelX = -1;
		m_iTexelY = -1;
		m_v3EndPos.Clear();
	}*/

	if (m_bInverse)
	{
		m_vecColor[0] = le_paintR2;
		m_vecColor[1] = le_paintG2;
		m_vecColor[2] = le_paintB2;
		m_vecColor[3] = le_paintA2;
	}
	else
	{
		m_vecColor[0] = le_paintR;
		m_vecColor[1] = le_paintG;
		m_vecColor[2] = le_paintB;
		m_vecColor[3] = le_paintA;
	}

	if (le_paintMaterial.IsModified())
	{
		m_hMaterial = g_ResourceManager.Register(le_paintMaterial, RES_MATERIAL);
		le_paintMaterial.SetModified(false);
	}

	if (le_paintDiffuseTexture.IsModified())
	{
		m_hDiffuse = g_ResourceManager.Register(K2_NEW(ctx_Editor,   CTexture)(le_paintDiffuseTexture, TEXTURE_2D, 0, TEXFMT_A8R8G8B8), RES_TEXTURE);
		PaintDiffuseTexture.Trigger(le_paintDiffuseTexture);
		le_paintDiffuseTexture.SetModified(false);

		tstring sShortName(Filename_GetName(le_paintDiffuseTexture));

		if (sShortName.length() >= 2 && sShortName.substr(sShortName.length() - 2, tstring::npos) == _T("_d"))
		{
			tstring sStripped(Filename_StripExtension(le_paintDiffuseTexture));
			le_paintNormalmapTexture = sStripped.substr(0, sStripped.length() - 2) + _T("_n") + _T(".") + Filename_GetExtension(le_paintDiffuseTexture);
			le_paintNormalmapTexture.SetModified(true);
		}
		else
		{
			le_paintNormalmapTexture = _T("$flat_dull");
			le_paintNormalmapTexture.SetModified(true);
		}
	}

	if (le_paintNormalmapTexture.IsModified())
	{
		m_hNormalmap = g_ResourceManager.Register(K2_NEW(ctx_Editor,   CTexture)(le_paintNormalmapTexture, TEXTURE_2D, 0, TEXFMT_NORMALMAP), RES_TEXTURE);
		PaintNormalmapTexture.Trigger(le_paintNormalmapTexture);
		le_paintNormalmapTexture.SetModified(false);
	}

#if 0
	if (le_paintMaterial.IsModified())
	{
		if (m_pMaterialBrush)
		{
			delete m_pMaterialBrush;
			m_pMaterialBrush = NULL;
		}

		m_pMaterialBrush = K2_NEW(ctx_Editor,   CMaterialBrush)(_T("/brushes/materials/") + le_paintMaterial + _T(".xml"));

		le_paintMaterial.SetModified(false);
	}
#endif
}


/*====================
  CPaintTool::LerpColor
  ====================*/
void	CPaintTool::LerpColor(CVec4b *pRegion, const CRecti &recArea, const CBrush &brush, const CVec4f &v4Color, float fScale)
{
	int iBrushSize(brush.GetBrushSize());
	int iRegionIndex(0);

	for (int y(0); y < recArea.GetHeight(); ++y)
	{
		for (int x(0); x < recArea.GetWidth(); ++x)
		{
			float fLerp = CLAMP(brush[BRUSH_INDEX(x, y)] * fScale, 0.0f, 1.0f);
			if (fLerp == 0.0f)
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
  CPaintTool::LerpAlpha
  ====================*/
void	CPaintTool::LerpAlpha(CVec4b *pRegion, const CRecti &recArea, const CBrush &brush, float fAlpha, float fScale)
{
	int iBrushSize(brush.GetBrushSize());
	int iRegionIndex(0);

	for (int y(0); y < recArea.GetHeight(); ++y)
	{
		for (int x(0); x < recArea.GetWidth(); ++x)
		{
			float fLerp = CLAMP(brush[BRUSH_INDEX(x, y)] * fScale, 0.0f, 1.0f);
			if (fLerp == 0.0f)
			{
				++iRegionIndex;
				continue;
			}

			float fOutAlpha;
			fOutAlpha = LERP(fLerp, pRegion[iRegionIndex][A] / 255.0f, fAlpha);
			pRegion[iRegionIndex][A] = static_cast<byte>(CLAMP(fOutAlpha, 0.0f, 1.0f) * 255);
			++iRegionIndex;
		}
	}
}


/*====================
  CPaintTool::PaintVertex
  ====================*/
void	CPaintTool::PaintVertex(float fFrameTime)
{
	CVec4b *pRegion(NULL);

	try
	{
		CBrush *pBrush(CBrush::GetCurrentBrush());
		if (pBrush == NULL)
			EX_ERROR(_T("No brush selected"));

		//if (!Editor.GetWorld().IsInBounds(m_iX, m_iY, GRID_SPACE))
		//	EX_WARN(_T("Out of bounds coordinate"));

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
		if (!Editor.GetWorld().GetRegion(WORLD_VERT_COLOR_MAP, recClippedBrush, pRegion))
			EX_ERROR(_T("Failed to retrieve region"));

		// Perform the operation
		recClippedBrush.Shift(-m_iX, -m_iY);
		if (le_paintMode == PAINT_COLOR)
			LerpColor(pRegion, recClippedBrush, *pBrush, m_vecColor, fFrameTime * le_paintBrushStrength / 1000.0f);
		else if (le_paintMode == PAINT_TEXTURE && le_paintLayer > 0)
			LerpAlpha(pRegion, recClippedBrush, *pBrush, m_bInverse ? 0.0f : 1.0f, fFrameTime * le_paintBrushStrength / 1000.0f);

		// Apply the modified region
		recClippedBrush.Shift(m_iX, m_iY);
		if (!Editor.GetWorld().SetRegion(WORLD_VERT_COLOR_MAP, recClippedBrush, pRegion))
			EX_ERROR(_T("SetRegion() failed"));

		// Notify the video drivers about the update
		for (int y(recClippedBrush.top); y <= recClippedBrush.bottom; ++y)
		{
			for (int x = recClippedBrush.left; x <= recClippedBrush.right; ++x)
				Vid.Notify(VID_NOTIFY_TERRAIN_COLOR_MODIFIED, x, y, 0, &Editor.GetWorld());
		}

		K2_DELETE_ARRAY(pRegion);
	}
	catch (CException &ex)
	{
		if (pRegion != NULL)
			K2_DELETE_ARRAY(pRegion);

		ex.Process(_T("CPaintTool::PaintVertex() - "), NO_THROW);
	}
}


/*====================
  CPaintTool::ApplyTextureTile
  ====================*/
void	CPaintTool::ApplyTextureTile(STile *pRegion, const CRecti &recArea, const CBrush &brush, ResHandle hMaterial, ResHandle hDiffuse, ResHandle hNormalmap)
{
	int iBrushSize(brush.GetBrushSize());

	int iRegionIndex(0);
	for (int y(0); y < recArea.GetHeight(); ++y)
	{
		for (int x(0); x < recArea.GetWidth(); ++x)
		{
			if (brush[BRUSH_INDEX(x, y)] > 0)
			{
				//recClippedBrush.Shift(iX - pBrush->GetBrushSize() / 2, iY - pBrush->GetBrushSize() / 2);
				//recClippedBrush.Shift(-(iX - pBrush->GetBrushSize() / 2),  (y + (m_iY - pBrush->GetBrushSize() / 2));
				if(m_bGrid)
					m_bGrid[(x + (recArea.left + m_iTexelX)) + ((y + (recArea.top + m_iTexelY)) * m_iGridWidth)] = 1;
				pRegion[iRegionIndex].hMaterial = hMaterial;
				pRegion[iRegionIndex].hDiffuse = hDiffuse;
				pRegion[iRegionIndex].hNormalmap = hNormalmap;
			}

			++iRegionIndex;
		}
	}
}


/*====================
  CPaintTool::ApplyTextureTileStroked

  Stroked to fix the alpha edge problems
  ====================*/
void	CPaintTool::ApplyTextureTileStroked(STile *pRegion, const CRecti &recArea, const CBrush &brush, ResHandle hMaterial, ResHandle hDiffuse, ResHandle hNormalmap)
{
	int iBrushSize(brush.GetBrushSize());

	int iRegionIndex(0);
	for (int y(0); y < recArea.GetHeight(); ++y)
	{
		for (int x(0); x < recArea.GetWidth(); ++x)
		{
			if (brush[BRUSH_INDEX(x, y)] > 0 ||
				(recArea.left + x < iBrushSize - 1 && brush[BRUSH_INDEX(x + 1, y)] > 0) ||
				(recArea.top + y < iBrushSize - 1 && brush[BRUSH_INDEX(x, y + 1)] > 0) ||
				(recArea.left + x < iBrushSize - 1 && recArea.top + y < iBrushSize - 1 && brush[BRUSH_INDEX(x + 1, y + 1)] > 0))
			{
				pRegion[iRegionIndex].hMaterial = hMaterial;
				pRegion[iRegionIndex].hDiffuse = hDiffuse;
				pRegion[iRegionIndex].hNormalmap = hNormalmap;
			}

			++iRegionIndex;
		}
	}
}


#if 0
/*====================
  CPaintTool::GetMaterialTile
  ====================*/
int		CPaintTool::GetMaterialTile(short coverage, int bitTL, int bitTR, int bitBL, int bitBR)
{
	int iReturn = 0;

	if (coverage & BIT(bitTL))
		iReturn |= BIT(0);

	if (coverage & BIT(bitTR))
		iReturn |= BIT(1);

	if (coverage & BIT(bitBL))
		iReturn |= BIT(2);

	if (coverage & BIT(bitBR))
		iReturn |= BIT(3);

	return iReturn;
}


/*====================
  CPaintTool::UpdateCoverage
  ====================*/
void	CPaintTool::UpdateCoverage(short &coverage, int bitTL, int bitTR, int bitBL, int bitBR, int iTile)
{
	if (iTile & BIT(0))
		coverage |= BIT(bitTL);

	if (iTile & BIT(1))
		coverage |= BIT(bitTR);

	if (iTile & BIT(2))
		coverage |= BIT(bitBL);

	if (iTile & BIT(3))
		coverage |= BIT(bitBR);
}


/*====================
  CPaintTool::ApplyTextureMaterial
  ====================*/
void	CPaintTool::ApplyTextureMaterial(STile *pRegion, const CRecti &recArea, bool bErase, CMaterialBrush *pBrush)
{
	ResHandle		tiles[4];
	CVec2f			v2Offset(0.5f, 0.5f);

	if (!pBrush)
		return;

	short coverage(0); // bit vector of material coverage (4x4)

	// Calculate current material coverage
	int iRegionIndex = 0;
	for (int y(0); y < recArea.GetHeight(); ++y)
	{
		for (int x(0); x < recArea.GetWidth(); ++x)
		{
			int iMaterialTile = pBrush->GetTileIndex(pRegion[iRegionIndex].iDiffuseRef);

			if (iMaterialTile == 0 ||
				pRegion[iRegionIndex].fScale != pBrush->GetScale() ||
				pRegion[iRegionIndex].v2Offset != v2Offset)
			{
				++iRegionIndex;
				continue;
			}

			int iCoverageX = x / pBrush->GetScale();
			int iCoverageY = y / pBrush->GetScale();

			assert(iCoverageX == 0 || iCoverageX == 1);
			assert(iCoverageY == 0 || iCoverageY == 1);

			if (iCoverageX == 0 && iCoverageY == 0)
				UpdateCoverage(coverage, 0, 1, 4, 5, iMaterialTile);
			else if (iCoverageX == 1 && iCoverageY == 0)
				UpdateCoverage(coverage, 2, 3, 6, 7, iMaterialTile);
			else if (iCoverageX == 0 && iCoverageY == 1)
				UpdateCoverage(coverage, 8, 9, 12, 13, iMaterialTile);
			else if (iCoverageX == 1 && iCoverageY == 1)
				UpdateCoverage(coverage, 10, 11, 14, 15, iMaterialTile);

			++iRegionIndex;
		}
	}

	// set the middle 2x2 bits
	if (bErase)
	{
		coverage &= ~BIT(5);
		coverage &= ~BIT(6);
		coverage &= ~BIT(9);
		coverage &= ~BIT(10);
	}
	else
	{
		coverage |= BIT(5);
		coverage |= BIT(6);
		coverage |= BIT(9);
		coverage |= BIT(10);
	}

	// determine new tiles based on material coverage
	tiles[0] = pBrush->GetTile(GetMaterialTile(coverage, 0, 1, 4, 5));
	tiles[1] = pBrush->GetTile(GetMaterialTile(coverage, 2, 3, 6, 7));
	tiles[2] = pBrush->GetTile(GetMaterialTile(coverage, 8, 9, 12, 13));
	tiles[3] = pBrush->GetTile(GetMaterialTile(coverage, 10, 11, 14, 15));

	// write new tiles back into the region
	iRegionIndex = 0;
	for (int y(recArea.top); y < recArea.bottom; ++y)
	{
		for (int x(recArea.left); x < recArea.right; ++x)
		{
			pRegion[iRegionIndex].iDiffuseRef = tiles[(recArea.left + x) / pBrush->GetScale() + (recArea.top + y) / pBrush->GetScale() * 2];
			pRegion[iRegionIndex].fScale = float(pBrush->GetScale());
			pRegion[iRegionIndex].v2Offset = v2Offset;
			pRegion[iRegionIndex].fRotation = 0.0f;
			++iRegionIndex;
		}
	}
}
#endif

/*====================
  CPaintTool::PaintTile
  ====================*/
void	CPaintTool::PaintTile(float fFrameTime)
{
	STile *pRegion(NULL);

	try
	{
		
		int iBrushSize, iX = m_iTexelX, iY = m_iTexelY;

		CBrush *pBrush(CBrush::GetCurrentBrush());
		if (pBrush == NULL)
			EX_ERROR(_T("No brush selected"));

	//	if (!Editor.GetWorld().IsInBounds(m_iX, m_iY, TILE_SPACE))
	//		EX_WARN(_T("Out of bounds coordinate"));

		// Shift xy and set iBrushSize
		switch (static_cast<int>(le_paintMode))
		{
		case PAINT_MATERIAL:
			{
				int iSize = m_pMaterialBrush->GetScale();

				iX = iX - iX % iSize + iSize / 2;
				iY = iY - iY % iSize + iSize / 2;

				iBrushSize = 2 * iSize; // 2 * scale of current material
			}
			break;
		default:
		case PAINT_TEXTURE:
			{
				iBrushSize = pBrush->GetBrushSize();
			}
			break;
		}

		// Clip the brush
		CRecti	recClippedBrush;

		// Clip against the brush data
		if (!pBrush->ClipBrush(recClippedBrush))
			return;

		if (le_paintLayer > 0) // Expand brush by one pixel in the negative direction to fix alpha edge problems
		{
			if (recClippedBrush.left > 0)
			{
				recClippedBrush.ShiftX(-1);
				recClippedBrush.StretchX(2);
			}


			if (recClippedBrush.top > 0)
			{
				recClippedBrush.ShiftY(-1);
				recClippedBrush.StretchY(2);
			}
			iX = m_iX;
			iY = m_iY;
		}

		// Clip the brush against the world
		recClippedBrush.Shift(iX, iY);
		if (!Editor.GetWorld().ClipRect(recClippedBrush, TILE_SPACE))
			return;

		// Get the region
		pRegion = K2_NEW_ARRAY(ctx_Editor, STile, recClippedBrush.GetArea());
		if (pRegion == NULL)
			EX_ERROR(_T("Failed to allocate region"));

		if (!Editor.GetWorld().GetRegion(WORLD_TILE_MATERIAL_MAP, recClippedBrush, pRegion, le_paintLayer))
			EX_ERROR(_T("Failed to retrieve region"));

		// Perform the operation
		recClippedBrush.Shift(-iX, -iY);
		if (le_paintMode == PAINT_TEXTURE && le_paintLayer > 0 && !m_bInverse)
			ApplyTextureTileStroked(pRegion, recClippedBrush, *pBrush, m_hMaterial, m_hDiffuse, m_hNormalmap);
		else if (le_paintMode == PAINT_TEXTURE && !m_bInverse)
			ApplyTextureTile(pRegion, recClippedBrush, *pBrush, m_hMaterial, m_hDiffuse, m_hNormalmap);

		// Apply the modified region
		recClippedBrush.Shift(iX, iY);
		if (!Editor.GetWorld().SetRegion(WORLD_TILE_MATERIAL_MAP, recClippedBrush, pRegion, le_paintLayer))
			EX_ERROR(_T("SetRegion() failed"));

		// Notify the video drivers about the update
		for (int y(recClippedBrush.top); y <= recClippedBrush.bottom; ++y)
		{
			for (int x = recClippedBrush.left; x <= recClippedBrush.right; ++x)
			{
				Vid.Notify(VID_NOTIFY_TERRAIN_SHADER_MODIFIED, x, y, 0, &Editor.GetWorld());
			}
		}


		K2_DELETE_ARRAY(pRegion);
	}
	catch (CException &ex)
	{
		if (pRegion != NULL)
			K2_DELETE_ARRAY(pRegion);

		ex.Process(_T("CPaintTool::PaintTile() - "), NO_THROW);
	}
}


/*====================
  CPaintTool::LerpTexelAlpha
  ====================*/
void	CPaintTool::LerpTexelAlpha(byte *pRegion, const CRecti &recArea, const CBrush &brush, float fAlpha, float fScale)
{
	int iBrushSize(brush.GetBrushSize());
	int iRegionIndex(0);

	for (int y(0); y < recArea.GetHeight(); ++y)
	{
		for (int x(0); x < recArea.GetWidth(); ++x)
		{
			float fLerp = CLAMP(brush[BRUSH_INDEX(x, y)] * fScale, 0.0f, 1.0f);
			if (fLerp == 0.0f)
			{
				++iRegionIndex;
				continue;
			}

			float fOutAlpha;
			fOutAlpha = LERP(fLerp, pRegion[iRegionIndex] / 255.0f, fAlpha);
			pRegion[iRegionIndex] = static_cast<byte>(CLAMP(fOutAlpha, 0.0f, 1.0f) * 255);
			++iRegionIndex;
		}
	}
}


/*====================
  CPaintTool::PaintTexel
  ====================*/
void	CPaintTool::PaintTexel(float fFrameTime)
{
	byte *pRegion(NULL);

	try
	{
		CBrush *pBrush(CBrush::GetCurrentBrush());
		if (pBrush == NULL)
			EX_ERROR(_T("No brush selected"));

		//if (!Editor.GetWorld().IsInBounds(m_iTexelX, m_iTexelY, TEXEL_SPACE))
		//	EX_WARN(_T("Out of bounds coordinate"));

		// Clip the brush
		CRecti	recClippedBrush;

		// Clip against the brush data
		if (!pBrush->ClipBrush(recClippedBrush))
			return;

		// Clip the brush against the world
		recClippedBrush.Shift(m_iTexelX, m_iTexelY);
		if (!Editor.GetWorld().ClipRect(recClippedBrush, TEXEL_SPACE))
			return;

		// Get the region
		pRegion = K2_NEW_ARRAY(ctx_Editor, byte, recClippedBrush.GetArea());
		if (pRegion == NULL)
			EX_ERROR(_T("Failed to allocate region"));

		if (!Editor.GetWorld().GetRegion(WORLD_TEXEL_ALPHA_MAP, recClippedBrush, pRegion, 0))
			EX_ERROR(_T("Failed to retrieve region"));

		// Perform the operation
		recClippedBrush.Shift(-m_iTexelX, -m_iTexelY);
		LerpTexelAlpha(pRegion, recClippedBrush, *pBrush, m_bInverse ? le_paintTexelAlpha2 : le_paintTexelAlpha, fFrameTime * le_paintBrushStrength / 1000.0f);

		// Apply the modified region
		recClippedBrush.Shift(m_iTexelX, m_iTexelY);
		if (!Editor.GetWorld().SetRegion(WORLD_TEXEL_ALPHA_MAP, recClippedBrush, pRegion, 0))
			EX_ERROR(_T("SetRegion() failed"));

		// Notify the video drivers about the update
		for (int y(recClippedBrush.top); y <= recClippedBrush.bottom; ++y)
		{
			for (int x = recClippedBrush.left; x <= recClippedBrush.right; ++x)
				Vid.Notify(VID_NOTIFY_TERRAIN_TEXEL_ALPHA_MODIFIED, x, y, 0, &Editor.GetWorld());
		}


		K2_DELETE_ARRAY(pRegion);
	}
	catch (CException &ex)
	{
		if (pRegion != NULL)
			K2_DELETE_ARRAY(pRegion);

		ex.Process(_T("CPaintTool::PaintTile() - "), NO_THROW);
	}
}


/*====================
  CPaintTool::Frame
  ====================*/
void	CPaintTool::Frame(float fFrameTime)
{
	
	if(m_iGridWidth != Editor.GetWorld().GetTileWidth() || m_iGridHeight != Editor.GetWorld().GetTileHeight())
	{

		if(m_bGrid)
		{
			K2_DELETE_ARRAY(m_bGrid);
			m_bGrid = 0;
		}

		if(!m_bGrid)
		{
			m_iGridWidth = Editor.GetWorld().GetTileWidth();
			m_iGridHeight = Editor.GetWorld().GetTileHeight();
			m_bGrid = K2_NEW_ARRAY(ctx_Editor, bool, (m_iGridWidth)*(m_iGridHeight));
		}

		if(m_bGrid)
		{
			for(int x = 0; x < Editor.GetWorld().GetTileWidth(); ++x)
			{
				for(int y = 0; y < Editor.GetWorld().GetTileHeight(); ++y)
				{
					tstring sTmp(g_ResourceManager.GetPath(Editor.GetWorld().GetTileDiffuseTexture(x, y, 0)));

					if(sTmp == _T("$checker") || sTmp == _T("$smooth_checker") || sTmp == _T("$red_smooth_checker") || sTmp == _T("$blue_smooth_checker") || sTmp == _T("$green_smooth_checker") || sTmp == _T("$yellow_smooth_checker"))
						m_bGrid[x + (y * m_iGridWidth)] = 0;
					else
						m_bGrid[x + (y * m_iGridWidth)] = 1;
				}
			}
		}
	}


	CalcToolProperties();

	if (m_bWorking && m_bValidPosition)
	{
		//if (m_bModifier3)
		//{
		//	CWorld &oWorld(Editor.GetWorld());
		//	const float fTileSize(float(oWorld.GetScale()));

		//	//int iTileX(oWorld.GetTileFromCoord(m_v3EndPos.x));
		//	//int iTileY(oWorld.GetTileFromCoord(m_v3EndPos.y));

		//	float fLerps[2] =
		//	{
		//		FRAC(m_v3EndPos.x / fTileSize),
		//		FRAC(m_v3EndPos.y / fTileSize)
		//	};

		//	switch (le_paintMode)
		//	{
		//	case PAINT_COLOR:
		//		{
		//			CVec4b v4Colors[4] =
		//			{
		//				oWorld.GetGridColor(m_iX, m_iY),
		//				oWorld.GetGridColor(m_iX + 1, m_iY),
		//				oWorld.GetGridColor(m_iX, m_iY + 1),
		//				oWorld.GetGridColor(m_iX + 1, m_iY + 1)
		//			};

		//			byte yReds[4] =
		//			{
		//				v4Colors[0][R],
		//				v4Colors[1][R],
		//				v4Colors[2][R],
		//				v4Colors[3][R]
		//			};
		//			byte yPCFRed(PCF(fLerps, yReds));

		//			byte yGreens[4] =
		//			{
		//				v4Colors[0][G],
		//				v4Colors[1][G],
		//				v4Colors[2][G],
		//				v4Colors[3][G]
		//			};
		//			byte yPCFGreen(PCF(fLerps, yGreens));

		//			byte yBlues[4] =
		//			{
		//				v4Colors[0][B],
		//				v4Colors[1][B],
		//				v4Colors[2][B],
		//				v4Colors[3][B]
		//			};
		//			byte yPCFBlue(PCF(fLerps, yBlues));

		//			le_paintR = yPCFRed / 255.0f;
		//			le_paintG = yPCFGreen / 255.0f;
		//			le_paintB = yPCFBlue / 255.0f;
		//		}
		//		break;
		//	case PAINT_MATERIAL:
		//		break;
		//	case PAINT_TEXTURE:
		//		le_paintMaterial = g_ResourceManager.GetPath(oWorld.GetTileMaterial(m_iTexelX, m_iTexelY, le_paintLayer));
		//		le_paintNormalmapTexture = g_ResourceManager.GetPath(oWorld.GetTileNormalmapTexture(m_iTexelX, m_iTexelY, le_paintLayer));
		//		le_paintDiffuseTexture = g_ResourceManager.GetPath(oWorld.GetTileDiffuseTexture(m_iTexelX, m_iTexelY, le_paintLayer));
		//		break;
		//	case PAINT_ALPHA:
		//		break;
		//	}

		//	return;
		//}
		if (!m_bModifier3 && !m_bModifier2)
		{
			switch (le_paintMode)
			{
			case PAINT_COLOR:
				PaintVertex(fFrameTime);
				break;

			case PAINT_TEXTURE:
				if (!m_bInverse)
					PaintTile(fFrameTime);
				if (le_paintLayer > 0)
					PaintVertex(fFrameTime); // paint alpha as well
				break;

			case PAINT_MATERIAL:
				PaintTile(fFrameTime);
				break;

			case PAINT_ALPHA:
				PaintTexel(fFrameTime);
				break;
			}
		}
	}
}


/*====================
  DrawInfoString
  ====================*/
static void		DrawInfoString(const tstring &sString, int &iLine, CFontMap *pFontMap, ResHandle hFont)
{
	float fWidth(pFontMap->GetStringWidth(sString));
	Draw2D.SetColor(0.0f, 0.0f, 0.0f);
	Draw2D.String(Draw2D.GetScreenW() - fWidth - 3.0f, Draw2D.GetScreenH() - pFontMap->GetMaxHeight() * (iLine + 1) - 1.0f, fWidth, pFontMap->GetMaxHeight(), sString, hFont);
	Draw2D.SetColor(1.0f, 1.0f, 1.0f);
	Draw2D.String(Draw2D.GetScreenW() - fWidth - 4.0f, Draw2D.GetScreenH() - pFontMap->GetMaxHeight() * (iLine + 1) - 2.0f, fWidth, pFontMap->GetMaxHeight(), sString, hFont);
	++iLine;
}


/*====================
  CPaintTool::Draw
  ====================*/
void	CPaintTool::Draw()
{
	if(!m_bValidPosition)
		return;

	if (le_paintDrawBrushCoords)
	{
		CFontMap *pFontMap(g_ResourceManager.GetFontMap(m_hFont));

		Draw2D.SetColor(0.0f, 0.0f, 0.0f);
		Draw2D.String(4.0f, Draw2D.GetScreenH() - pFontMap->GetMaxHeight() - 1.0f, Draw2D.GetScreenW(), pFontMap->GetMaxHeight(), XtoA(m_v3EndPos), m_hFont);
		Draw2D.SetColor(1.0f, 1.0f, 1.0f);
		Draw2D.String(3.0f, Draw2D.GetScreenH() - pFontMap->GetMaxHeight() - 2.0f, Draw2D.GetScreenW(), pFontMap->GetMaxHeight(), XtoA(m_v3EndPos), m_hFont);
	}

	if (le_paintDrawInfo)
	{
		CFontMap *pFontMap(g_ResourceManager.GetFontMap(m_hFont));

		CWorld &oWorld(Editor.GetWorld());
		const float fTileSize(float(oWorld.GetScale()));
		
		int iTileX(Editor.GetWorld().GetTileFromCoord(m_v3EndPos.x));
		int iTileY(Editor.GetWorld().GetTileFromCoord(m_v3EndPos.y));

		float fLerps[2] =
		{
			FRAC(m_v3EndPos.x / fTileSize),
			FRAC(m_v3EndPos.y / fTileSize)
		};

		int iLine(0);

		//
		// Color
		//

		CVec4b v4Colors[4] =
		{
			oWorld.GetGridColor(iTileX, iTileY),
			oWorld.GetGridColor(iTileX + 1, iTileY),
			oWorld.GetGridColor(iTileX, iTileY + 1),
			oWorld.GetGridColor(iTileX + 1, iTileY + 1)
		};

		byte yReds[4] =
		{
			v4Colors[0][R],
			v4Colors[1][R],
			v4Colors[2][R],
			v4Colors[3][R]
		};
		byte yPCFRed(PCF(fLerps, yReds));

		byte yGreens[4] =
		{
			v4Colors[0][G],
			v4Colors[1][G],
			v4Colors[2][G],
			v4Colors[3][G]
		};
		byte yPCFGreen(PCF(fLerps, yGreens));

		byte yBlues[4] =
		{
			v4Colors[0][B],
			v4Colors[1][B],
			v4Colors[2][B],
			v4Colors[3][B]
		};
		byte yPCFBlue(PCF(fLerps, yBlues));

		byte yAlphas[4] =
		{
			v4Colors[0][A],
			v4Colors[1][A],
			v4Colors[2][A],
			v4Colors[3][A]
		};
		byte yPCFAlpha(PCF(fLerps, yAlphas));

		DrawInfoString(_T("Color: ") + XtoA(yPCFRed) + _T(", ") + XtoA(yPCFGreen) + _T(", ") + XtoA(yPCFBlue) + _T(", ") + XtoA(yPCFAlpha), iLine, pFontMap, m_hFont);
		DrawInfoString(_T("Material: ") + g_ResourceManager.GetPath(oWorld.GetTileMaterial(iTileX, iTileY, le_paintLayer)), iLine, pFontMap, m_hFont);
		DrawInfoString(_T("Normalmap: ") + g_ResourceManager.GetPath(oWorld.GetTileNormalmapTexture(iTileX, iTileY, le_paintLayer)), iLine, pFontMap, m_hFont);
		DrawInfoString(_T("Diffuse: ") + g_ResourceManager.GetPath(oWorld.GetTileDiffuseTexture(iTileX, iTileY, le_paintLayer)), iLine, pFontMap, m_hFont);
		DrawInfoString(_T("Layer ") + XtoA(le_paintLayer + 1), iLine, pFontMap, m_hFont);
	}
}


/*====================
  CPaintTool::Render

  Draw brush influences
  ====================*/
void	CPaintTool::Render()
{
	SSceneFaceVert poly[1024];
	MemManager.Set(poly, 0, sizeof(poly));
	int p = 0;

	// Draw un tuched tiles
	if(le_showNullTiles)
	{
		float fTileSize = Editor.GetWorld().GetScale();
		int alpha = fabs(sin((float)K2System.Milliseconds() * 0.003f) * 0.5f) * 255;
		alpha = CLAMP(alpha, 0, 128);

		for (int y = 0; y < Editor.GetWorld().GetTileHeight(); ++y)
		{
			for (int x = 0; x < Editor.GetWorld().GetTileWidth(); ++x)
			{

//				ResHandle Tmp(g_ResourceManager.LookUpPath(g_ResourceManager.GetPath(Editor.GetWorld().GetTileDiffuseTexture(x, y, 0))));
				if(m_bGrid[x + (y * m_iGridWidth)] == 0)
				{
					if (p >= 1020) // restart batch if we overflow
					{
						SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_TRILIST | POLY_NO_DEPTH_TEST);
						MemManager.Set(poly, 0, sizeof(poly));
						p = 0;
					}

					poly[p].vtx[0] = x * fTileSize;
					poly[p].vtx[1] = y * fTileSize;
					poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(x, y);
					SET_VEC4(poly[p].col, 255, 0, 0, alpha);
					++p;

					poly[p].vtx[0] = x * fTileSize;
					poly[p].vtx[1] = (y + 1) * fTileSize;
					poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(x, y + 1);
					SET_VEC4(poly[p].col, 255, 0, 0, alpha);
					++p;

					poly[p].vtx[0] = (x + 1) * fTileSize;
					poly[p].vtx[1] = (y + 1) * fTileSize;
					poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(x + 1, y + 1);
					SET_VEC4(poly[p].col, 255, 0, 0, alpha);
					++p;


					poly[p].vtx[0] = x * fTileSize;
					poly[p].vtx[1] = y * fTileSize;
					poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(x, y);
					SET_VEC4(poly[p].col, 255, 0, 0, alpha);
					++p;

					poly[p].vtx[0] = (x + 1) * fTileSize;
					poly[p].vtx[1] = (y + 1) * fTileSize;
					poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(x + 1, y + 1);
					SET_VEC4(poly[p].col, 255, 0, 0, alpha);
					++p;

					poly[p].vtx[0] = (x + 1) * fTileSize;
					poly[p].vtx[1] = y * fTileSize;
					poly[p].vtx[2] = Editor.GetWorld().GetGridPoint(x + 1, y);
					SET_VEC4(poly[p].col, 255, 0, 0, alpha);
					++p;
				}
			}
		}

		if (p > 0)
		{
			SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_TRILIST | POLY_NO_DEPTH_TEST);
			MemManager.Set(poly, 0, sizeof(poly));
			p = 0;
		}
	}


	if (!le_paintDrawBrushInfluence || !m_bValidPosition)
		return;

	if (le_paintMode == PAINT_COLOR || (le_paintMode == PAINT_TEXTURE && le_paintLayer > 0))
	{
		float fTileSize = Editor.GetWorld().GetScale();

		int iX = m_iX, iY = m_iY;
		CBrush *pBrush = CBrush::GetCurrentBrush();

		if (!pBrush)
			return;

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

				// left
				{
					int dX = iX + x;
					int dY = iY + y;

					byte alpha0 = INT_ROUND((*pBrush)[i] * le_paintBrushInfluenceAlpha);
					byte alpha1 = INT_ROUND((*pBrush)[i + pBrush->GetBrushSize()] * le_paintBrushInfluenceAlpha);

					if (Editor.GetWorld().IsInBounds(dX, dY, GRID_SPACE) &&
						Editor.GetWorld().IsInBounds(dX, dY + 1, GRID_SPACE) &&
						(alpha0 || alpha1))
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
				{
					int dX = iX + x;
					int dY = iY + y;

					byte alpha0 = INT_FLOOR((*pBrush)[i] * le_paintBrushInfluenceAlpha);
					byte alpha1 = INT_FLOOR((*pBrush)[i + 1] * le_paintBrushInfluenceAlpha);

					if (Editor.GetWorld().IsInBounds(dX, dY, GRID_SPACE) &&
						Editor.GetWorld().IsInBounds(dX + 1, dY, GRID_SPACE) &&
						(alpha0 || alpha1))
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
		{
			SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
			MemManager.Set(poly, 0, sizeof(poly));
			p = 0;
		}
	}
	else if (le_paintMode == PAINT_TEXTURE)
	{
		CBrush *pBrush = CBrush::GetCurrentBrush();
		float fTileSize = Editor.GetWorld().GetScale();

		if (!pBrush)
			return;

		int iX = m_iTexelX, iY = m_iTexelY;

		for (int y = 0; y < pBrush->GetBrushSize(); ++y)
		{
			for (int x = 0; x < pBrush->GetBrushSize(); ++x)
			{
				int i = x + y * pBrush->GetBrushSize();

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
				if ((y > 0 && (*pBrush)[i] && !(*pBrush)[i - pBrush->GetBrushSize()]) || (y == 0 && (*pBrush)[i]))
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
				if ((y < pBrush->GetBrushSize() - 1 && (*pBrush)[i] && !(*pBrush)[i+pBrush->GetBrushSize()]) || (y == pBrush->GetBrushSize() - 1 && (*pBrush)[i]))
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

		if (p > 0)
		{
			SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
			MemManager.Set(poly, 0, sizeof(poly));
			p = 0;
		}
	}
	else if (le_paintMode == PAINT_MATERIAL)
	{
		if (!m_pMaterialBrush)
			return;

		int iX = m_iTexelX, iY = m_iTexelY;
		int iSize = m_pMaterialBrush->GetScale();
		float fTileSize = Editor.GetWorld().GetScale();

		iX = iX - iX % iSize + iSize/2;
		iY = iY - iY % iSize + iSize/2;

		for (int y = 0; y < iSize; ++y)
		{
			for (int x = 0; x < iSize; ++x)
			{
				// left
				if (x == 0)
				{
					int dX = iX + x - iSize/2;
					int dY = iY + y - iSize/2;

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

				// right
				if (x == iSize - 1)
				{
					int dX = iX + x - iSize/2;
					int dY = iY + y - iSize/2;

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

				// top
				if (y == 0)
				{
					int dX = iX + x - iSize/2;
					int dY = iY + y - iSize/2;

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

				// bottom
				if (y == iSize - 1)
				{
					int dX = iX + x - iSize/2;
					int dY = iY + y - iSize/2;

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

		if (p > 0)
		{
			SceneManager.AddPoly(p, poly, m_hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
			MemManager.Set(poly, 0, sizeof(poly));
			p = 0;
		}
	}
}


/*--------------------
  cmdSetPaintMode
  --------------------*/
UI_VOID_CMD(SetPaintMode, 1)
{
	if (vArgList.size() < 1)
	{
		Console << _T("syntax: paintmode color|alpha|texture|material") << newl;
		return;
	}

	tstring sValue(vArgList[0]->Evaluate());

	if (sValue == _T("color"))
	{
		le_paintMode = PAINT_COLOR;
		PaintMode.Trigger(_T("Color"));
		return;
	}
	else if (sValue == _T("texture"))
	{
		le_paintMode = PAINT_TEXTURE;
		PaintMode.Trigger(_T("Texture"));
		return;
	}
	else if (sValue == _T("alpha"))
	{
		le_paintMode = PAINT_ALPHA;
		PaintMode.Trigger(_T("Alpha"));
		return;
	}
	else if (sValue == _T("material"))
	{
		le_paintMode = PAINT_MATERIAL;
		PaintMode.Trigger(_T("Material"));
		return;
	}
}
