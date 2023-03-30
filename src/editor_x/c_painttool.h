// (C)2005 S2 Games
// c_painttool.h
//
//=============================================================================
#ifndef __C_PAINTTOOL_H__
#define __C_PAINTTOOL_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/c_vec4.h"

#include "c_toolbox.h"
#include "i_tool.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
struct STile;
class CBrush;
class CMaterialBrush;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EPaintMode
{
	PAINT_COLOR,
	PAINT_TEXTURE,
	PAINT_MATERIAL,
	PAINT_ALPHA
};
//=============================================================================

//=============================================================================
// CPaintTool
// Terrain painter
//=============================================================================
class CPaintTool : public ITool
{
private:
	int				m_iTexelX, m_iTexelY;

	CMaterialBrush	*m_pMaterialBrush;
	ResHandle		m_hMaterial;
	ResHandle		m_hDiffuse;
	ResHandle		m_hNormalmap;
	ResHandle		m_hLineMaterial;
	ResHandle		m_hFont;
	CVec3f			m_v3EndPos;

	CVec4f	m_vecColor;
	bool	m_bWorking;
	bool	m_bInverse;

	static void	LerpColor(CVec4b *pRegion, const CRecti &recArea, const CBrush &brush, const CVec4f &v4Color, float fScale);
	static void	LerpAlpha(CVec4b *pRegion, const CRecti &recArea, const CBrush &brush, float fAlpha, float fScale);

	static void	LerpTexelAlpha(byte *pRegion, const CRecti &recArea, const CBrush &brush, float fAlpha, float fScale);

	static void	ApplyTextureTile(STile *pRegion, const CRecti &recArea, const CBrush &brush, ResHandle hMaterial, ResHandle hDiffuse, ResHandle hNormalmap);
	static void	ApplyTextureTileStroked(STile *pRegion, const CRecti &recArea, const CBrush &brush, ResHandle hMaterial, ResHandle hDiffuse, ResHandle hNormalmap);

	void	PaintVertex(float fFrameTime);
	void	PaintTile(float fFrameTime);
	void	PaintTexel(float fFrameTime);

public:
	CPaintTool();
	virtual ~CPaintTool()				{}

	void	CalcToolProperties();


	void	PrimaryUp();
	void	PrimaryDown();
	void	SecondaryUp();
	void	SecondaryDown();
	void	TertiaryUp();
	void	TertiaryDown();
	void	QuaternaryUp();
	void	QuaternaryDown();

	void	Cancel();
	void	Delete();

	void	Frame(float fFrameTime);

	void	Draw();
	void	Render();
};
//=============================================================================
#endif //__C_PAINTTOOL_H__
