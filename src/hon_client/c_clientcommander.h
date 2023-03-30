// (C)2006 S2 Games
// c_clientcommander.h
//
//=============================================================================
#ifndef __C_CLIENTCOMMANDER_H__
#define __C_CLIENTCOMMANDER_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/c_camera.h"
#include "../k2/c_input.h"
#include "../k2/c_gamebind.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CPlayer;

extern CGameBindImpulse gamebindCommanderPing;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum ECommanderState
{
	COMSTATE_HOVER = 0,
	COMSTATE_SELECT,
	
	COMSTATE_MOVE,
	COMSTATE_ATTACK,
	COMSTATE_PATROL,
	COMSTATE_TARGET_ENTITY,
	COMSTATE_TARGET_AREA,
	COMSTATE_TARGET_DUAL,
	COMSTATE_TARGET_DUAL_POSITION,
	COMSTATE_TARGET_VECTOR
};

struct SSelectionEntry
{
	uint index;
	uint type;

	SSelectionEntry()
		: index(-1)
		, type(-1)
	{
	}

	SSelectionEntry(uint index, uint type)
		: index(index)
		, type(type)
	{
	}

	bool operator ==(const SSelectionEntry& entry) const
	{
		return (index == entry.index);
	}

	bool operator <(const SSelectionEntry& entry) const
	{
		return index < entry.index;
	}
};

struct SSelectionSet
{
	typedef set<SSelectionEntry>	Set;
	Set			setEntries;
	uint		uiFocus;
};

const int MAX_PLAYER_WAYPOINTS(8);
const uint MAX_SAVED_SELECTION_SETS(10);

const uint HOVER_TRACE(SURF_HULL | SURF_FOLIAGE | SURF_NOT_SOLID | SURF_INTANGIBLE | SURF_BLOCKER | SURF_SHIELD | SURF_PROP | SURF_DEAD | SURF_CORPSE | SURF_STATIC | SURF_RENDER | SURF_NOSELECT);
const uint ACTIVATE_TRACE(SURF_HULL | SURF_FOLIAGE | SURF_NOT_SOLID | SURF_INTANGIBLE | SURF_BLOCKER | SURF_SHIELD | SURF_PROP | SURF_DEAD | SURF_CORPSE | SURF_RENDER | SURF_NOSELECT);
const uint ACTIVATE_TRACE_POSITION(SURF_HULL | SURF_FOLIAGE | SURF_NOT_SOLID | SURF_INTANGIBLE | SURF_BLOCKER | SURF_SHIELD | SURF_PROP | SURF_DEAD | SURF_CORPSE | SURF_RENDER | SURF_NOSELECT | SURF_UNIT | SURF_BUILDING | SURF_ITEM);
//=============================================================================

//=============================================================================
// CClientCommander
//=============================================================================
class CClientCommander
{
private:
	list<uint>				m_vPendingPets;

	CPlayer*				m_pPlayer;
	CCamera*				m_pCamera;
	CVec2f					m_v2Cursor;

	bool					m_bModifier1;	// Shift
	bool					m_bModifier2;	// Ctrl
	bool					m_bModifier3;	// NA
	bool					m_bModifier4;	// Alt
	bool					m_bFrontQueueModifier;
	bool					m_bAlternateModifier;

	ECommanderState			m_eState;
	CVec2f					m_v2StartCursorPos;
	CVec3f					m_v3StartCursorPos;
	CVec3f					m_v3TraceEndPos;
	uint					m_uiHoverEnt;
	uint					m_uiFirstEntityIndex;
	uiset					m_setInfoSelection;
	uiset					m_setControlSelection;
	uiset					m_setTransmittedControlSelection;
	uiset					m_setHoverSelection;
	SSelectionSet			m_aSavedSelection[MAX_SAVED_SELECTION_SETS];

	int						m_iActiveSlot;
	bool					m_bControlSelection;
	uint					m_uiActiveControlEntity;

	uint					m_uiLastSelectionSetRecallIndex;
	uint					m_uiLastSelectionSetRecallTime;

	uint					m_uiLastPrimaryTime;
	uint					m_uiSelectTime;
	uint					m_uiLastSelectTime;
	uint					m_uiLastSetControlTime;
	uint					m_uiLastSetControlEntity;
	uint					m_uiLastVoiceEntity;
	uint					m_uiVoiceSequence;

	// UI feedback sounds
	ResHandle				m_hHoverSound;

	// Cursors
	ResHandle				m_hArrow;
	ResHandle				m_hArrowAlly;
	ResHandle				m_hArrowEnemy;

	ResHandle				m_hPingAlly;
	ResHandle				m_hPingEnemy;

	ResHandle				m_hActionValid;
	ResHandle				m_hActionInvalid;
	ResHandle				m_hActionAllyValid;
	ResHandle				m_hActionAllyInvalid;
	ResHandle				m_hActionEnemyValid;
	ResHandle				m_hActionEnemyInvalid;

	ResHandle				m_hScrollDown;
	ResHandle				m_hScrollLeft;
	ResHandle				m_hScrollLeftDown;
	ResHandle				m_hScrollLeftUp;
	ResHandle				m_hScrollRight;
	ResHandle				m_hScrollRightDown;
	ResHandle				m_hScrollRightUp;
	ResHandle				m_hScrollUp;

	ResHandle				m_hCurrentCursor;
	
	ResHandle				m_hShop;

	uint					m_uiCameraWatchIndex;

	tstring					m_sClickCmd;

	CVec3f					m_v3VectorTargetPos;
	bool					m_bVectorTargeting;

	uint					m_uiLastDoubleActivateTime;
	IEntityTool*			m_pLastDoubleActivateTool;


	struct SSelectedPlayerWaypoint
	{
		bool	bActive;
		byte	yOrder;
		uint	uiOrderEntIndex;
		CVec3f	v3OrderPos;
		uint	uiEvent;
	
		SSelectedPlayerWaypoint() :
		bActive(false),
		yOrder(CMDR_ORDER_CLEAR),
		uiOrderEntIndex(INVALID_INDEX),
		v3OrderPos(V3_ZERO),
		uiEvent(INVALID_INDEX)
		{
		}
	};

	SSelectedPlayerWaypoint		m_aSelectedPlayerWaypoints[MAX_PLAYER_WAYPOINTS];

	CRectf		GetSelectionRect();
	CRectf		GetWorldSelection();
	void		UpdateCursorTrace(const CVec2f &v2Cursor, uint uiIgnoreSurface = HOVER_TRACE, uint uiIgnoreEntity = INVALID_INDEX);
	void		UpdateHoverSelection();

	bool		IsSelectionActive()				{ return m_eState == COMSTATE_SELECT; }

	void		StartSelect(const CVec2f &v2Cursor);
	void		ApplySelect(const CVec2f &v2Cursor);
	bool		SelectionContainsHeroes() const;

	void		UpdateSelectedPlayerWaypoints();
	
	void		PlaySelectSound(uint uiIndex);
	void		ProximitySearch(const CVec3f &v3Pos);

	void		UpdateSelection();

	bool		CanDoubleActivate();
	void		CancelDoubleActivate();
	void		UpdateDoubleActivate();

public:
	~CClientCommander()	{}
	CClientCommander();

	void	LoadResources(ResHandle hClientSoundsStringTable);

	void	SetPlayer(CPlayer *pPlayer)								{ m_pPlayer = pPlayer; }
	void	SetCamera(CCamera *pCamera)								{ m_pCamera = pCamera; }

	void			SetCommanderState(ECommanderState eState)	{ m_eState = eState; }
	ECommanderState	GetCommanderState() const					{ return m_eState; }
	const CVec3f&	GetTracePosition() const					{ return m_v3TraceEndPos; }
	float			GetCameraDistance() const;

	void	Reinitialize();
	void	Frame();
	void	Draw();
	
	// Actions
	void	PrimaryDown(const CVec2f &v2Cursor);
	void	PrimaryUp(const CVec2f &v2Cursor);
	void	SecondaryDown(const CVec2f &v2Cursor);
	void	SecondaryUp(const CVec2f &v2Cursor);
	bool	Cancel(const CVec2f &v2Cursor);

	void	SetModifier1(bool bValue)			{ m_bModifier1 = bValue; }
	void	SetModifier2(bool bValue)			{ m_bModifier2 = bValue; }
	void	SetModifier3(bool bValue)			{ m_bModifier3 = bValue; }
	void	SetModifier4(bool bValue)			{ m_bModifier4 = bValue; }
	void	SetFrontQueueModifier(bool bValue)	{ m_bFrontQueueModifier = bValue; }
	void	SetAlternateModifier(bool bValue)	{ m_bAlternateModifier = bValue; }

	bool	GetModifier1()						{ return m_bModifier1; }
	bool	GetModifier2()						{ return m_bModifier2; }
	bool	GetModifier3()						{ return m_bModifier3; }
	bool	GetModifier4()						{ return m_bModifier4; }
	bool	GetFrontQueueModifier()				{ return m_bFrontQueueModifier; }
	bool	GetAlternateModifier()				{ return m_bAlternateModifier; }

	bool	GetPingKeyDown() const				{ return ((gamebindCommanderPing.GetButton1Down() && gamebindCommanderPing.GetModifier1Down()) || (gamebindCommanderPing.GetButton2Down() && gamebindCommanderPing.GetModifier2Down())); }
	bool	GetPingEnabled() const				{ return (GameClient.GetItemCursorIndex() == INVALID_INDEX && GetCommanderState() == COMSTATE_HOVER && (gamebindCommanderPing.GetModifier1Down() || gamebindCommanderPing.GetModifier2Down())); }
	bool	GetPingModifiersDown() const		{ return (gamebindCommanderPing.GetBothModifiers() && (gamebindCommanderPing.GetModifier1Down() || gamebindCommanderPing.GetModifier2Down())); }

	bool	IsSelected(uint uiIndex)			{ return (m_setInfoSelection.find(uiIndex) != m_setInfoSelection.end() || m_setControlSelection.find(uiIndex) != m_setControlSelection.end());}
	bool	IsHoverSelected(uint uiIndex)		{ return m_setHoverSelection.find(uiIndex) != m_setHoverSelection.end(); }
	bool	IsDragSelecting()					{ return m_eState == COMSTATE_SELECT && m_v2StartCursorPos != Input.GetCursorPos(); }
	uint	GetHoverEntity() const				{ return m_uiHoverEnt; }

	void	SetCursorPos(const CVec2f &v2Pos)	{ m_v2Cursor = v2Pos; }

	void	GiveOrder(ECommanderOrder eOrder, uint uiTargetIndex, byte yQueue, uint uiParam = 0);
	void	GiveOrder(ECommanderOrder eOrder, const CVec2f &v2Pos, byte yQueue, uint uiParam = 0);

	void	StartCommand(const CVec2f &v2Pos, byte yQueue, ECommanderOrder eOrder = CMDR_ORDER_AUTO, uint uiParam = 0);
	void	ActivateTool(int iSlot, bool bSecondary, IUnitEntity *pUnit, bool bClick);
	void	Cast(const CVec2f &v2Pos, const CVec2f &v2Delta, bool bUpdateTrace = true);
	void	DoubleActivate();
	void	Ping();

	uint			GetSelectedControlEntityIndex();
	IUnitEntity*	GetSelectedControlEntity()			{ return GameClient.GetUnitEntity(GetSelectedControlEntityIndex()); }

	uint			GetSelectedInfoEntityIndex();
	IUnitEntity*	GetSelectedInfoEntity()				{ return GameClient.GetUnitEntity(GetSelectedInfoEntityIndex()); }

	void			SelectEntity(uint uiIndex);

	void			SaveSelectionSet(uint uiIndex);
	void			AddToSelectionSet(uint uiIndex);
	void			RemoveFromSelectionSet(uint uiIndex);
	void			RecallSelectionSet(uint uiIndex);
	uint			ReplaceUnitInSelectionSets(uint uiUnitIndex, uint uiUnitType);

	const uiset&	GetSelectedInfoEntities() const		{ return m_setInfoSelection; }
	const uiset&	GetSelectedControlEntities() const	{ return m_setControlSelection; }
	const uiset&	GetHoveredEntities() const			{ return m_setHoverSelection; }

	void			NextUnit(bool bCentered = false);
	void			PrevUnit(bool bCentered = false);
	void			NextInventoryUnit(bool bCentered = false);
	void			PrevInventoryUnit(bool bCentered = false);

	void			SetControlUnit(int iSlot, bool bCentered = false);
	void			DeselectUnit(int iSlot);

	void			PrepareClientSnapshot(CClientSnapshot &snapshot);

	int				GetActiveSlot() const				{ return m_iActiveSlot; }

	void			ResetVoiceSequence()				{ m_uiVoiceSequence = 0; }

	void			ValidateSelections();

	bool			MinimapPrimaryClick(const CVec2f &v2Pos);
	bool			MinimapSecondaryClick(const CVec2f &v2Pos);

	uint			GetActiveControlEntity() const		{ return m_uiActiveControlEntity; }

	void			StartClickCmdPos(const tstring &sCmd);

	void			PrintErrorMessage(const tstring &sMessage);
	void			SetDefaultActiveShop();

	bool			IsTargetingEntity() const;

	bool			GetActiveUnits() const;

	void			ForceResendGameplayOptions();
	void			TransmitGameplayOptions();

	void			OnPetAdded(uint uiPetIndex);
};
//=============================================================================

#endif //__C_CLIENTCOMMANDER_H__
