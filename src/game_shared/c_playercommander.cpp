// (C)2006 S2 Games
// c_playercommander.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_playercommander.h"
#include "c_teaminfo.h"

#include "../k2/c_camera.h"
#include "../k2/c_clientsnapshot.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_uitrigger.h"
#include "../k2/c_draw2d.h"
#include "../k2/c_worldentity.h"
#include "../k2/intersection.h"
#include "../k2/c_vid.h"
#include "../k2/c_soundmanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Player, Commander);

CCvarf	CPlayerCommander::s_cvarCamMinHeight(	_T("Commander_CamMinHeight"),	50.0f,		CVAR_GAMECONFIG | CVAR_TRANSMIT);
CCvarf	CPlayerCommander::s_cvarCamMaxHeight(	_T("Commander_CamMaxHeight"),	1000.0f,	CVAR_GAMECONFIG | CVAR_TRANSMIT);
CCvarf	CPlayerCommander::s_cvarCamZoomSpeed(	_T("Commander_CamZoomSpeed"),	50.0f,		CVAR_GAMECONFIG | CVAR_TRANSMIT);
CCvarf	CPlayerCommander::s_cvarCamFOV(			_T("Commander_CamFOV"),			90.0f,		CVAR_GAMECONFIG | CVAR_TRANSMIT);
//=============================================================================

/*====================
  CPlayerCommander::~CPlayerCommander
  ====================*/
CPlayerCommander::~CPlayerCommander()
{
	SAFE_DELETE(m_pCamera);
}


/*====================
  CPlayerCommander::CPlayerCommander
  ====================*/
CPlayerCommander::CPlayerCommander() :
IPlayerEntity(GetEntityConfig()),

m_pCamera(K2_NEW(global,   CCamera){}/*====================
  CPlayerCommander::GetCameraPosition
  ====================*/CVec3fCPlayerCommander::GetCameraPosition)(const CVec3f &v3PlayerPos, const CVec3f &v3PlayerAngles)
{
	CVec3f v3Target(v3PlayerPos);

#if 0
	v3Target.z = Game.GetTerrainHeight(v3Target.x, v3Target.y);
#else
	v3Target.z = Game.GetGroundLevel();
#endif

	CAxis axis(GetCameraAngles(v3PlayerAngles));
	return M_PointOnLine(v3Target, axis.Forward(), -m_v3Position.z);
}


/*====================
  CPlayerCommander::GetCameraAngles
  ====================*/
CVec3f	CPlayerCommander::GetCameraAngles(const CVec3f &v3InputAngles)
{
	return CVec3f(m_pEntityConfig->GetCamPitch(), 0.0f, 0.0f);
}


/*====================
  CPlayerCommander::GetTargetPosition
  ====================*/
CVec3f	CPlayerCommander::GetTargetPosition(float fRange, float fMinRange)
{
	// Set up a virtual camera window using relative cursor coords
	float fWidth(1024.0f);
	float fHeight(768.0f);
	CVec2f v2Cursor(m_v2Cursor);
	v2Cursor.x *= fWidth;
	v2Cursor.y *= fHeight;

	CCamera camera;
	camera.SetWidth(fWidth);
	camera.SetHeight(fHeight);
	camera.SetFovXCalc(GetFov());
	camera.SetAngles(GetCameraAngles(GetViewAngles()));

	CVec3f v3Start(GetCameraPosition(m_v3Position, GetViewAngles()));
	CVec3f v3Dir(camera.ConstructRay(v2Cursor));
	CVec3f v3End(M_PointOnLine(v3Start, v3Dir, FAR_AWAY));

	STraceInfo trace;
	Game.TraceLine(trace, v3Start, v3End, TRACE_TERRAIN);
	return trace.v3EndPos;
}


/*====================
  CPlayerCommander::DrawViewBox
  ====================*/
void	CPlayerCommander::DrawViewBox(CUITrigger &minimap, CCamera &camera)
{
	CBufferFixed<48> buffer;

	CVec3f v3Start(camera.GetOrigin());
	CPlane plane(V_UP, 0.0f);

	CVec3f	v3Dir(camera.ConstructRay(0.0f, 0.0f));
	v3Dir *= FAR_AWAY;
	CVec3f v3End(v3Start + v3Dir);
	float fFraction(1.0f);
	I_LinePlaneIntersect(v3Start, v3End, plane, fFraction);
	CVec3f v3TL(LERP(fFraction, v3Start, v3End));

	v3Dir = camera.ConstructRay(float(Vid.GetScreenW() - 1), 0.0f);
	v3Dir *= FAR_AWAY;
	v3End = v3Start + v3Dir;
	fFraction = 1.0f;
	I_LinePlaneIntersect(v3Start, v3End, plane, fFraction);
	CVec3f v3TR(LERP(fFraction, v3Start, v3End));

	v3Dir = camera.ConstructRay(0.0f, float(Vid.GetScreenH() - 1));
	v3Dir *= FAR_AWAY;
	v3End = v3Start + v3Dir;
	fFraction = 1.0f;
	I_LinePlaneIntersect(v3Start, v3End, plane, fFraction);
	CVec3f v3BL(LERP(fFraction, v3Start, v3End));

	v3Dir = camera.ConstructRay(float(Vid.GetScreenW() - 1), float(Vid.GetScreenH() - 1));
	v3Dir *= FAR_AWAY;
	v3End = v3Start + v3Dir;
	fFraction = 1.0f;
	I_LinePlaneIntersect(v3Start, v3End, plane, fFraction);
	CVec3f v3BR(LERP(fFraction, v3Start, v3End));

	buffer.Clear();
	buffer << v3TL.x / Game.GetWorldWidth() << 1.0f - v3TL.y / Game.GetWorldHeight()
		<< v3TR.x / Game.GetWorldWidth() << 1.0f - v3TR.y / Game.GetWorldHeight()
		<< 1.0f << 1.0f << 1.0f << 1.0f
		<< 1.0f << 1.0f << 1.0f << 1.0f;
	minimap.Execute(_T("line"), buffer);

	buffer.Clear();
	buffer << v3TR.x / Game.GetWorldWidth() << 1.0f - v3TR.y / Game.GetWorldHeight()
		<< v3BR.x / Game.GetWorldWidth() << 1.0f - v3BR.y / Game.GetWorldHeight()
		<< 1.0f << 1.0f << 1.0f << 1.0f
		<< 1.0f << 1.0f << 1.0f << 1.0f;
	minimap.Execute(_T("line"), buffer);

	buffer.Clear();
	buffer << v3TL.x / Game.GetWorldWidth() << 1.0f - v3TL.y / Game.GetWorldHeight()
		<< v3BL.x / Game.GetWorldWidth() << 1.0f - v3BL.y / Game.GetWorldHeight()
		<< 1.0f << 1.0f << 1.0f << 1.0f
		<< 1.0f << 1.0f << 1.0f << 1.0f;
	minimap.Execute(_T("line"), buffer);

	buffer.Clear();
	buffer << v3BL.x / Game.GetWorldWidth() << 1.0f - v3BL.y / Game.GetWorldHeight()
		<< v3BR.x / Game.GetWorldWidth() << 1.0f - v3BR.y / Game.GetWorldHeight()
		<< 1.0f << 1.0f << 1.0f << 1.0f
		<< 1.0f << 1.0f << 1.0f << 1.0f;
	minimap.Execute(_T("line"), buffer);
}


/*====================
  CPlayerCommander::SetupCamera
  ====================*/
void	CPlayerCommander::SetupCamera(CCamera &camera, const CVec3f &v3InputPosition, const CVec3f &v3InputAngles)
{
	camera.RemoveFlags(CAM_FIRST_PERSON);
	camera.SetFovXCalc(GetFov());

	// Determine camera position and angles
	camera.SetAngles(GetCameraAngles(v3InputAngles));
	camera.SetOrigin(GetCameraPosition(v3InputPosition, v3InputAngles));

	camera.SetLodDistance(m_v3Position.z);

	*m_pCamera = camera;
}


/*====================
  CPlayerCommander::ReadClientSnapshot
  ====================*/
void	CPlayerCommander::ReadClientSnapshot(const CClientSnapshot &snapshot)
{
	if (Game.GetGamePhase() == GAME_PHASE_ENDED)
		return;

	m_v3Position = snapshot.GetCameraPosition();
	m_v2Cursor = snapshot.GetCursorPosition();
	m_setSelection = snapshot.GetSelection();
	m_ySelectedItem = snapshot.GetSelectedItem();
	m_uiSpellTargetIndex = snapshot.GetSelectedEntity();

	// Building rotation
	IBuildingEntity *pBuilding(Game.GetBuildingEntity(GetPreviewBuildingIndex()));
	if (pBuilding != NULL)
		pBuilding->SetAngles(snapshot.GetAngles());

	Move(snapshot);
}


/*====================
  CPlayerCommander::GiveGold
  ====================*/
ushort	CPlayerCommander::GiveGold(ushort unGold, bool bUseTax, bool bUseIncomeMod)
{
	if (Game.GetGamePhase() == GAME_PHASE_WARMUP)
		return 0;

	CEntityTeamInfo *pTeam(Game.GetTeam(GetTeam()));
	if (pTeam == NULL)
		return 0;

	pTeam->GiveGold(unGold);
	return unGold;
}


/*====================
  CPlayerCommander::SpendMana
  ====================*/
bool	CPlayerCommander::SpendMana(float fCost)
{
	CEntityTeamInfo *pTeam(Game.GetTeam(GetTeam()));
	if (pTeam == NULL)
		return false;

	return pTeam->SpendMana(fCost);
}


/*====================
  CPlayerCommander::GetMana
  ====================*/
float	CPlayerCommander::GetMana() const
{
	CEntityTeamInfo *pTeam(Game.GetTeam(GetTeam()));
	if (pTeam == NULL)
		return false;

	return pTeam->GetMana();
}


/*====================
  CPlayerCommander::GetMaxMana
  ====================*/
float	CPlayerCommander::GetMaxMana() const
{
	CEntityTeamInfo *pTeam(Game.GetTeam(GetTeam()));
	if (pTeam == NULL)
		return false;

	return pTeam->GetMaxMana();
}


/*====================
  CPlayerCommander::Move
  ====================*/
void	CPlayerCommander::Move(const CClientSnapshot &snapshot)
{
	if (m_iCurrentAction & PLAYER_ACTION_SPELL)
		m_spellActivate.TryImpact();

	// Check for expired actions before trying to start new ones
	if (!IsIdle() && m_uiCurrentActionEndTime <= snapshot.GetTimeStamp())
	{
		int iFinishedAction(m_iCurrentAction);
		if (GetCurrentItem() != NULL)
			GetCurrentItem()->FinishedAction(iFinishedAction);
	}

	// Inventory activation
	byte yActivate(snapshot.GetActivate());
	if (yActivate != NO_SELECTION && m_apInventory[yActivate] != NULL)
		ActivatePrimary(yActivate, GAME_BUTTON_STATUS_DOWN | GAME_BUTTON_STATUS_PRESSED);
}


/*====================
  CPlayerCommander::ZoomIn
  ====================*/
void	CPlayerCommander::ZoomIn(CClientSnapshot *pSnapshot)
{
	CVec3f	v3Pos(pSnapshot->GetCameraPosition());
	v3Pos -= CVec3f(0.0f, 0.0f, s_cvarCamZoomSpeed);
	v3Pos.z = CLAMP(v3Pos.z, s_cvarCamMinHeight.GetValue(), s_cvarCamMaxHeight.GetValue());

	pSnapshot->SetCameraPosition(v3Pos);
}


/*====================
  CPlayerCommander::ZoomOut
  ====================*/
void	CPlayerCommander::ZoomOut(CClientSnapshot *pSnapshot)
{
	CVec3f	v3Pos(pSnapshot->GetCameraPosition());
	v3Pos += CVec3f(0.0f, 0.0f, s_cvarCamZoomSpeed);
	v3Pos.z = CLAMP(v3Pos.z, s_cvarCamMinHeight.GetValue(), s_cvarCamMaxHeight.GetValue());

	pSnapshot->SetCameraPosition(v3Pos);
}


/*====================
  CPlayerCommander::Spawn
  ====================*/
void	CPlayerCommander::Spawn()
{
	SetStatus(ENTITY_STATUS_ACTIVE);
	SetSquad(INVALID_SQUAD);

	// Fill inventory
	GiveItem(0, EntityRegistry.LookupID(m_pEntityConfig->GetInventory0()));
	GiveItem(1, EntityRegistry.LookupID(m_pEntityConfig->GetInventory1()));
	GiveItem(2,	EntityRegistry.LookupID(m_pEntityConfig->GetInventory2()));
	GiveItem(3, EntityRegistry.LookupID(m_pEntityConfig->GetInventory3()));
	GiveItem(4, EntityRegistry.LookupID(m_pEntityConfig->GetInventory4()));
	GiveItem(5, EntityRegistry.LookupID(m_pEntityConfig->GetInventory5()));
	GiveItem(6, EntityRegistry.LookupID(m_pEntityConfig->GetInventory6()));
	GiveItem(7, EntityRegistry.LookupID(m_pEntityConfig->GetInventory7()));
	GiveItem(8, EntityRegistry.LookupID(m_pEntityConfig->GetInventory8()));
	GiveItem(9, EntityRegistry.LookupID(m_pEntityConfig->GetInventory9()));

	if (Game.IsServer() || m_iClientNum == Game.GetLocalClientNum())
		Game.SelectItem(GetDefaultInventorySlot());

	if (!Game.IsClient() || m_iClientNum != Game.GetLocalClientNum())
		return;

	// Get a nice initial camera position
	CEntityTeamInfo *pTeam(Game.GetTeam(m_iTeam));
	if (pTeam != NULL)
	{
		IVisualEntity *pBase(Game.GetVisualEntity(pTeam->GetBaseBuildingIndex()));
		if (pBase != NULL)
			m_v3Position = pBase->GetPosition();
		m_v3Position.z = s_cvarCamMaxHeight.GetValue();
		Game.GetCurrentSnapshot()->SetCameraPosition(m_v3Position);
	}

	return;
}


/*====================
  CPlayerCommander::ServerFrame
  ====================*/
bool	CPlayerCommander::ServerFrame()
{
	IBuildingEntity *pPreviewBuilding(Game.GetBuildingEntity(GetPreviewBuildingIndex()));
	if (pPreviewBuilding != NULL)
		pPreviewBuilding->SetPosition(GetTargetPosition(FAR_AWAY));

	return true;
}


/*====================
  CPlayerCommander::Copy
  ====================*/
void	CPlayerCommander::Copy(const IGameEntity &B)
{
	IPlayerEntity::Copy(B);
}
