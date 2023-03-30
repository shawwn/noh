// (C)2005 S2 Games
// c_editor.h
//
//=============================================================================
#ifndef __C_EDITOR_H__
#define __C_EDITOR_H__

//=============================================================================
// Headers
//=============================================================================
#include "../aba_shared/i_game.h"

#include "../k2/c_host.h"
#include "../k2/c_hostclient.h"
#include "../k2/c_bitmap.h"
#include "../k2/c_skeleton.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CHostClient;
class CWorld;
class CCamera;
class COccluder;

struct STraceInfo;

enum EEditorResourceType
{
	RES_TREE = NUM_GAME_RESOURCE_TYPES,
};
//=============================================================================

//=============================================================================
// CWorldEntityEx
//=============================================================================
class CWorldEntityEx
{
private:
	CSkeleton	*m_pSkeleton;
	vector<PoolHandle>	m_vPathBlockers;

public:
	~CWorldEntityEx()
	{
		SAFE_DELETE(m_pSkeleton);
	}

	CWorldEntityEx() :
	m_pSkeleton(NULL)
	{
	}

	CSkeleton*	GetSkeleton()						{ return m_pSkeleton; }
	void		SetSkeleton(CSkeleton *pSkeleton)	{ m_pSkeleton = pSkeleton; }

	vector<PoolHandle>&	GetPathBlockers()			{ return m_vPathBlockers; }
};

extern map<uint, CWorldEntityEx>	g_WorldEntData;
//=============================================================================

//=============================================================================
// CEditor
//=============================================================================
class CEditor : public IGame
{
private:
	CCamera*		m_pCamera;
	CVec3f			m_v3CamAngles;
	
	CVec3f			m_v3TargetCamPosition;
	CVec3f			m_v3TargetCamAngles;
	
	float			m_fCameraShift;

	vector<CVec3f>  m_vRulerPoints;

	CHostClient*	m_pHostClient;
	CWorld*			m_pWorld;

	ResHandle		m_hLineMaterial;
	ResHandle		m_hOccluderMaterial;

	ResHandle		m_hMinimapReference;
	ResHandle		m_hMinimapTexture;
	CBitmap*		m_pMinimapBitmap;

	CVec2f			m_v2PathStart;
	CVec2f			m_v2PathEnd;
	bool			m_bPathStartValid;
	bool			m_bPathEndValid;

	PoolHandle		m_hPath;

	bool			m_bRuler;
	CVec3f			m_v3RulerStart;
	CVec3f			m_v3RulerEnd;

	void	DrawEntities();
	void	AddLights();
	void	AddOccluders();
	void	DrawOccluderPoly(const COccluder &occluder);
	void	DrawOccluders();
	void	UpdateMinimap();
	void	DrawNavGrid();

public:
	~CEditor();
	CEditor();

	////
	void			SetGamePointer()						{ IGame::SetCurrentGamePointer(this); }
	ResHandle		RegisterModel(const tstring&)			{ return INVALID_RESOURCE; }
	ResHandle		RegisterEffect(const tstring&)			{ return INVALID_RESOURCE; }
	ResHandle		RegisterIcon(const tstring&)			{ return INVALID_RESOURCE; }
	void			Precache(ushort, EPrecacheScheme)		{}
	void			Precache(const tstring&, EPrecacheScheme) {}
	CStateString&	GetStateString(uint)					{ static CStateString ss; return ss; }
	CStateBlock&	GetStateBlock(uint)						{ static CStateBlock block; return block; }
	uint			GetServerFrame(void)					{ return 0; }
	uint			GetServerTime(void) const				{ return INVALID_TIME; }
	uint			GetPrevServerTime(void)					{ return INVALID_TIME; }
	uint			GetServerFrameLength(void)				{ return INVALID_TIME; }
	////

	bool			Init(CHostClient *pHostClient);
	void			Frame();

	uint			GetFrameCount() const	{ return m_pHostClient->GetFrameCount(); }
	float			GetFrameSeconds() const	{ return MsToSec(Host.GetFrameLength()); }

	CHostClient&	GetClient()				{ return *m_pHostClient; }
	CWorld&			GetWorld()				{ return *m_pWorld; }
	CCamera&		GetCamera()				{ return *m_pCamera; }

	bool			LoadWorld(const tstring &sWorldName);

	bool			TraceCursor(STraceInfo &result, int iIgnoreSurface);
	bool			TracePoint(STraceInfo &result, int iIgnoreSurface, CVec2f point);

	void			UpdateCamera();
	void			CenterCamera(const CVec3f &v3Pos);
	void			SetCameraPosition(const CVec3f &v3Pos);
	void			SetCameraAngles(const CVec3f &v3Angles);
	void			ShiftCamera(float fShift)		{ m_fCameraShift += fShift; }
	void			AdjustCameraPitch(float fPitch);
	void			AdjustCameraYaw(float fYaw);

	void			SetTextureScale(float fScale);
	void			SetFancyName(const tstring &sFancyName);
	
	void			UpdateMinimapTexture();
	void			RenderMinimapBitmap(CBitmap &cMinimap);

	void			UpdateScripts();
	void			UpdateImportedFiles();

	void			PathSetStart();
	void			PathSetEnd();
	void			PathClear();
	void			PathSpam();
	void			Path(const CVec2f &v2Start, const CVec2f &v2End);

	void			DebugRender();

	bool			GetLookAtPoint(CVec3f &v3Pos);

	void			RulerStart();
	void			RulerEnd();
	bool			IsRulerActive()		{ return m_bRuler; }
	CVec3f			GetRulerStart()		{ return m_v3RulerStart; }
	CVec3f			GetRulerEnd()		{ return m_v3RulerEnd; }
	void			RulerPointCreate();

};
//=============================================================================

#endif //__C_EDITOR_H__
