// (C)2005 S2 Games
// c_spawntool.h
//
//=============================================================================
#ifndef __C_SPAWNTOOL_H__
#define __C_SPAWNTOOL_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_toolbox.h"
#include "i_tool.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum ESpawnEditMode
{
	SPAWN_HEROSPAWNPOINT = 0,
	SPAWN_CREEPPATH,
	SPAWN_CREATURESPAWN	
};

enum ESpawnTeam
{
	SPAWN_TEAM_PASSIVE = 255,
	SPAWN_TEAM_NEUTRAL = 254,
	SPAWN_TEAM_LEGION = 1,
	SPAWN_TEAM_HELLBOURNE = 2
};
//=============================================================================

#define	MAX_CREEP_WAYPOINTS	256
#define	MAX_CREEP_SPAWNS 10

//=============================================================================
// CCreepPath
//=============================================================================
class CCreepStartingPoint
{

public:

	CVec2f	Loc;
	CWorldEntity *pWaypointEntity;
	CWorldEntity *pMelee;
	CWorldEntity *pRanged;
	CWorldEntity *pOpposingTeam;

	CCreepStartingPoint() :
	Loc(CVec2f(0.0f, 0.0f)),
	pWaypointEntity(0),
	pMelee(0),
	pRanged(0),
	pOpposingTeam(0)
	{};

};

class CCreepWayPoint
{

public:

	CVec2f	Loc;
	CWorldEntity *pWaypointEntity;

	CCreepWayPoint() :
	Loc(CVec2f(0.0f, 0.0f)),
	pWaypointEntity(0)
	{};

};

class CCreepPath
{

public:

	uint					uiWayPoints;
	CCreepStartingPoint		CreepStartingPoint;
	CCreepWayPoint			WayPoint[MAX_CREEP_WAYPOINTS];

	CCreepPath() :
	uiWayPoints(0)
	{};

};


//=============================================================================
// CSpawnTool
// Spawn and regulation creation
//=============================================================================
class CSpawnTool : public ITool
{
private:

	CCreepPath		m_BadCreepPath[MAX_CREEP_SPAWNS];
	CCreepPath		m_GoodCreepPath[MAX_CREEP_SPAWNS];

	CVec3f			m_v3EndPos;
	CVec3f			m_v3StartPos;
	CVec2f			m_vStartCursorPos;
	CVec2f			m_vOldCursorPos;
	uint			m_uiHoverEnt;
	uiset			m_setSelection;
	int				m_iState;
	CVec3f			m_vTranslate;
	CVec3f			m_vTrueTranslate;
	float			m_fScale;
	float			m_fTrueScale;
	CVec3f			m_vRotation;
	CVec3f			m_vTrueRotation;
	bool			m_bSnapCursor;
	bool			m_bCloning;
	int				m_iOverMode; //Tree Mode, Regular Mode
	uiset			m_setOldSelection;
	uiset			m_setHoverSelection;
	ResHandle		m_hModel;
	ResHandle		m_hFont;
	ResHandle		m_hLineMaterial;

	bool	SnapAbsoluteDelta;
	bool	SnapDelta;
	float	PositionSnapDelta;
	float	HeightSnapDelta;
	float	AngleSnapDelta;
	float	ScaleSnapDelta;

	void		SnapCursor(const CVec3f &vOrigin);
	void		CloneSelection();
	uint		CreateSpawn(float fX, float fY);
	CVec3f		SelectionCenter();
	void		UpdateHoverSelection();

	void		Hovering()		{}
	void		TranslateXY();
	void		TranslateZ();
	void		RotateYaw();
	void		RotatePitch();
	void		RotateRoll();
	void		ScaleUniform();

	void		Create();
	void		Split();

	void		StartSelect();
	void		StartTranslateXY();
	void		StartTransform(int iState);

	//Tree Functions
	void		StartTreeSelect();
	void		ApplyTreeSelect();
	void		StartTreeTranslateXY();
	void		ApplyTreeTranslateXY();
	void		TreeTranslateXY();
	void		ApplyTreeTranslateZ();
	void		TreeCreate(); 
	void		Render(); //empty, used to draw the brush

	void		ApplySelect();
	void		ApplyTranslateXY();
	void		ApplyTranslateZ();
	void		ApplyRotation(EEulerComponent eDirection);
	void		ApplyRotation(EEulerComponent eDirection, const CVec3f &v3Center, bool bAdjustOrigin);
	void		ApplyScaleUniform();
	void		ApplyScaleUniform(const CVec3f &v3Center, bool bAdjustOrigin);

public:
	virtual ~CSpawnTool();
	CSpawnTool();

	void	CalcToolProperties();


	void	PrimaryUp();
	void	PrimaryDown();
	void	SecondaryUp()		{}
	void	SecondaryDown();
	void	TertiaryUp()		{}
	void	TertiaryDown();
	void	QuaternaryUp()		{}
	void	QuaternaryDown();

	void	Cancel();
	void	Delete();

	void	Frame(float fFrameTime);

	void	Draw();

	uint	GetHoverSpawn()					{ return m_uiHoverEnt; }
	bool	IsSelected(uint uiIndex)			{ return m_setSelection.find(uiIndex) != m_setSelection.end(); }
	bool	IsHoverSelected(uint uiIndex)		{ return m_setHoverSelection.find(uiIndex) != m_setHoverSelection.end(); }
	bool	IsSelectionLock();
	void	Deselect(uint uiIndex)				{ m_setSelection.erase(uiIndex); }
	bool	IsSelectionActive()					{ return m_iState == STATE_SELECT; }
	void	ToggleTreeMode();
	CRectf	GetSelectionRect();
	void	ClearSelection()					{ m_setSelection.clear(); }

	void	GroundSelection();
	void	StraightenSelection();
	void	ResetScaleSelection();

	// Current transformation
	float	GetScale(uint uiIndex);
	CVec3f	GetPosition(uint uiIndex);
	CVec3f	GetAngles(uint uiIndex);

	tstring	GetSelectionName();
	tstring	GetSelectionState();
	int		GetSelectionTeam();
	tstring	GetSelectionModel();
	tstring GetSelectionTreeDef();
	tstring	GetSelectionSkin();
	tstring GetSelectionType();
	tstring	GetSelectionProperty(const tstring &sName);
	bool	IsSelectionType(const tstring &sType);

	int		GetWorldIndexFromName(const tstring &sName);

	void	SetSelectionName(const tstring &sName);
	void	SetSelectionState(const tstring &sState);
	void	SetSelectionTeam(int iTeam);
	void	SetSelectionModel(const tstring &sModel);
	void	SetSelectionTreeDef(const tstring &sModel);

	void	SetSelectionSkin(const tstring &sSkin);
	void	SetSelectionType(const tstring &sType);
	void	SetSelectionProperty(const tstring &sKey, const tstring &sValue);

	void	SelectByModel(const tstring &sModel);

};
//=============================================================================

#endif //__C_SPAWNTOOL_H__
