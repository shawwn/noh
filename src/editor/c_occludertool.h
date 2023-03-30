// (C)2005 S2 Games
// c_occludertool.h
//
//=============================================================================
#ifndef __C_OCCLUDERTOOL_H__
#define __C_OCCLUDERTOOL_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_toolbox.h"
#include "i_tool.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CBrush;
class COccluder;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EOccluderEditMode
{
	OCCLUDER_CREATE = 0,
	OCCLUDER_SELECT,
	OCCLUDER_TRANSLATE_XY,
	OCCLUDER_TRANSLATE_Z,
	OCCLUDER_ROTATE_YAW,
	OCCLUDER_ROTATE_PITCH,
	OCCLUDER_ROTATE_ROLL,
	OCCLUDER_SCALE,
};

enum EOccluderSelectMode
{
	SELECT_VERTEX = 0,
	SELECT_EDGE,
	SELECT_FACE
};

struct SOccluderTrace
{
	uint		uiIndex;
	uint		uiVertex;
	uint		uiEdge;
	CVec3f		v3EndPos;
	float		fFraction;
	CVec3f		v3Offset;
	CVec3f		v3EndPosPlane;	// End pos in the plane of the current occluder
    COccluder*	pOccluder;
};
//=============================================================================

//=============================================================================
// COccluderTool
//=============================================================================
class COccluderTool : public ITool
{
private:
	bool			m_bValidPosition;
	bool			m_bCloning;
	SOccluderTrace	m_Trace;
	uiset			m_setSelection;
	uiset			m_setOldSelection;
	uiset			m_setHoverSelection;
	CVec2f			m_vStartCursorPos;
	CVec2f			m_vOldCursorPos;
	int				m_iState;
	CVec3f			m_vTranslate;
	CVec3f			m_vTrueTranslate;
	float			m_fScale;
	float			m_fTrueScale;
	CVec3f			m_vRotation;
	CVec3f			m_vTrueRotation;
	bool			m_bSnapCursor;
	CVec3f			m_v3EndPos;

	uint			m_uiHoverIndex; // can be a vertex index, an edge index, or an occluder index
	uint			m_uiHoverOccluder;
	uint			m_uiWorkingOccluder; // working occluder for vertex editing

	EOccluderSelectMode		m_eOccluderSelectMode;

	ResHandle		m_hLineMaterial;
	ResHandle		m_hOccluderMaterial;
	ResHandle		m_hFont;

	bool		CursorOccluderTraceFace();
	bool		CursorOccluderTraceVertex(uint uiIndex);
	void		UpdateOccluderSelectionSetDeletion(uiset &set, uint uiIndex);
	void		UpdateOccluderSelectionSetAddition(uiset &set, uint uiIndex);
	CVec3f		GetOccluderOrigin(uint uiIndex);
	CVec3f		SelectionCenter();
	void		SnapCursor(const CVec3f &vOrigin);
	void		RemoveOccluderVertex(size_t zVertex, COccluder &occluder);
	void		SplitOccluderVertices(size_t zVertex0, size_t zVertex1, COccluder &occluder);
	void		CloneSelection();

	void		Hovering();
	void		TranslateXY();
	void		TranslateZ();
	void		RotateYaw();
	void		RotatePitch();
	void		RotateRoll();
	void		Scale();

	void		Create();

	void		StartSelect();
	void		StartTranslateXY();
	void		StartTransform(int iState);

	void		ApplySelect();
	void		ApplyTransform();

	void		DrawOccluderPoly(uint uiIndex);

	void		UpdateHoverSelection();

	// OccluderMap management
	COccluder*	GetOccluder(uint uiIndex);

	// Delete funcions
	void		DeleteVertex();
	void		DeleteOccluder(uint uiIndex);

public:
	COccluderTool();
	virtual ~COccluderTool()				{}

	void		CalcToolProperties();
	uint		CreateOccluder(const CVec3f &vPos);

	void		PrimaryUp();
	void		PrimaryDown();
	void		SecondaryUp()		{}
	void		SecondaryDown();
	void		TertiaryUp()		{}
	void		TertiaryDown()		{}
	void		QuaternaryUp()		{}
	void		QuaternaryDown()	{}

	void		Cancel();
	void		Delete();

	void		Frame(float fFrameTime);

	bool		IsSelectionActive()					{ return m_iState == STATE_SELECT; }
	CRectf		GetSelectionRect();
	int			IsSelected(uint uiIndex);
	int			IsHoverSelected(uint uiIndex);
	void		Split();

	void		Draw();
	void		Render();

	// Current transformation
	CVec3f		GetVertexPosition(uint uiOccluder, uint uiVertex);

	void		SetOccluderSelectMode(EOccluderSelectMode eSelectMode);
};
//=============================================================================

#endif //__C_OCCLUDERTOOL_H__
