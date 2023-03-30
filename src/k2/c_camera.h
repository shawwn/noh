// (C)2005 S2 Games
// c_camera.h
//
//=============================================================================
#ifndef __C_CAMERA_H__
#define __C_CAMERA_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_vec3.h"
#include "c_axis.h"
#include "c_rect.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CWorld;
//=============================================================================

//=============================================================================
// Constants
//=============================================================================
const int	CAM_WIREFRAME_TERRAIN	(BIT(0));
const int	CAM_NO_TERRAIN			(BIT(1));
const int	CAM_NO_COLORMAP			(BIT(2));
const int	CAM_NO_SHADERMAP		(BIT(3));
const int	CAM_NO_SHADERMAP2		(BIT(4));
const int	CAM_SHOW_BBOXES			(BIT(5));
const int	CAM_NO_TEXTURE			(BIT(6));
const int	CAM_NO_WORLDOBJECTS		(BIT(7));
const int	CAM_NO_SKY				(BIT(8));
const int	CAM_NO_LIGHTING			(BIT(9));
const int	CAM_NO_OCCLUDERS		(BIT(10));
const int	CAM_ORTHO				(BIT(11));
const int	CAM_INVERSEPROJECTION	(BIT(12));
const int	CAM_INFINITE			(BIT(13));
const int	CAM_FIRST_PERSON		(BIT(14));
const int	CAM_NO_SHADOWS			(BIT(15));
const int	CAM_SHADOW_UNIFORM		(BIT(16));
const int	CAM_FOG_OF_WAR			(BIT(17));
const int	CAM_NO_FOG				(BIT(18));
const int	CAM_SHADOW_PARALLEL		(BIT(19));
const int	CAM_SHADOW_NOSLIDEBACK	(BIT(20));
const int	CAM_CUBEPROJECTION		(BIT(21));
const int	CAM_SHADOW_NO_FALLOFF	(BIT(22));
const int	CAM_SHADOW_SCENE_BOUNDS	(BIT(23));
const int	CAM_NO_POST				(BIT(24));
const int	CAM_NO_DEPTH_CLEAR		(BIT(25));
const int	CAM_DEPTH_COMPRESS		(BIT(26));
const int	CAM_NO_REFLECTIONS		(BIT(27));
const int	CAM_NO_CULL				(BIT(28));
const int	CAM_NO_SCENE_BUFFER		(BIT(29));
const int	CAM_NO_CLIFFS			(BIT(30));
const int	CAM_NO_WORLD			(CAM_NO_TERRAIN | CAM_NO_WORLDOBJECTS | CAM_NO_SKY);
//=============================================================================

//=============================================================================
// CCamera
//=============================================================================
class CCamera
{
private:
	CAxis	m_ViewAxis;
	CVec3f	m_v3Origin;
	CVec3f  m_v3Angles;
	
	float	m_fX, m_fY, m_fWidth, m_fHeight;
	float	m_fFovX, m_fFovY;
	float	m_fAspect;

	float	m_fTime;
	int		m_iFlags;
	int		m_iMode;

	float	m_fFogNear, m_fFogFar;
	float	m_fZNear, m_fZFar;
	float	m_fOrthoWidth, m_fOrthoHeight;

	float	m_fLodDistance;
	float	m_fShadowBias;
	float	m_fShadowMaxFov;

	CPlane	m_plReflect;
	CVec3f	m_v3Center;

	CWorld	*m_pWorld;

public:
	~CCamera()	{}
	CCamera() :
	m_fX(0.0f), m_fY(0.0f), m_fWidth(0.0f), m_fHeight(0.0f),
	m_ViewAxis(0.0f, 0.0f, 0.0f),
	m_v3Origin(V3_ZERO),
	m_v3Angles(V3_ZERO),
	m_fLodDistance(0.0f),
	m_fShadowBias(0.0f),
	m_fShadowMaxFov(180.0f),
	m_fZNear(0.0f),
	m_fZFar(0.0f),
	m_plReflect(0.0f, 0.0f, 1.0f, 0.0f),
	m_v3Center(0.0f, 0.0f, 0.0f)
	{
	}

	void			SetX(float x)							{ m_fX = x; }
	void			SetY(float y)							{ m_fY = y; }
	void			SetWidth(float w)						{ m_fWidth = w; }
	void			SetHeight(float h)						{ m_fHeight = h; }
	void			SetFovXCalc(float fFov)					{ m_fFovX = fFov; CalcFovY(); }
	void			SetFovYCalc(float fFov)					{ m_fFovY = fFov; CalcFovX(); }
	void			SetFovX(float fFov)						{ m_fFovX = fFov; }
	void			SetFovY(float fFov)						{ m_fFovY = fFov; }
	void			SetAspect(float fAspect)				{ m_fAspect = fAspect; }
	void			SetTime(float fTime)					{ m_fTime = fTime; }
	void			SetFogBounds(float fNear, float fFar)	{ m_fFogNear = fNear; m_fFogFar = fFar; }
	void			ClearFogBounds()						{ m_fFogNear = m_fFogFar = 0; }
	void			SetZNear(float fNear)					{ m_fZNear = fNear; }
	void			SetZFar(float fFar)						{ m_fZFar = fFar; }
	void			SetZBounds(float fNear, float fFar)		{ m_fZNear = fNear; m_fZFar = fFar; }
	void			SetOrthoWidth(float fOrthoWidth)		{ m_fOrthoWidth = fOrthoWidth; }
	void			SetOrthoHeight(float fOrthoHeight)		{ m_fOrthoHeight = fOrthoHeight; }
	void			SetLodDistance(float fLodDistance)		{ m_fLodDistance = fLodDistance; }
	void			SetWorld(CWorld *pWorld)				{ m_pWorld = pWorld; }
	void			SetShadowBias(float fShadowBias)		{ m_fShadowBias = fShadowBias; }
	void			SetShadowMaxFov(float fShadowMaxFov)	{ m_fShadowMaxFov = fShadowMaxFov; }
	void			SetReflect(const CPlane &plReflect)		{ m_plReflect = plReflect; }
	void			SetCenter(const CVec3f &v3Center)		{ m_v3Center = v3Center; }

	CVec2f			GetXY() const							{ return CVec2f(m_fX, m_fY); }
	float			GetX() const							{ return m_fX; }
	float			GetY() const							{ return m_fY; }
	float			GetWidth() const						{ return m_fWidth; }
	float			GetHeight() const						{ return m_fHeight; }
	float			GetFovX() const							{ return m_fFovX; }
	float			GetFovY() const							{ return m_fFovY; }
	float			GetAspect() const						{ return m_fAspect; }
	float			GetTime() const							{ return m_fTime; }
	float			GetFogNear() const						{ return m_fFogNear; }
	float			GetFogFar() const						{ return m_fFogFar; }
	float			GetZNear() const						{ return m_fZNear; }
	float			GetZFar() const							{ return m_fZFar; }
	float			GetOrthoWidth() const					{ return m_fOrthoWidth; }
	float			GetOrthoHeight() const					{ return m_fOrthoHeight; }
	float			GetLodDistance() const					{ return m_fLodDistance; }
	float			GetShadowBias() const					{ return m_fShadowBias; }
	float			GetShadowMaxFov() const					{ return m_fShadowMaxFov; }
	CWorld*			GetWorld() const						{ return m_pWorld; }
	const CPlane&	GetReflect() const						{ return m_plReflect; }
	const CVec3f&	GetCenter() const						{ return m_v3Center; }
	
	void			SetAngles(const CVec3f &v3Angles)					{ m_v3Angles = v3Angles; m_ViewAxis.Set(m_v3Angles); }
	void			SetAngles(float fPitch, float fRoll, float fYaw)	{ m_v3Angles = CVec3f(fPitch, fRoll, fYaw); m_ViewAxis.Set(m_v3Angles); }
	void			SetViewAxis(const CAxis &axis)						{ m_ViewAxis = axis; }
	void			ClearAxis()											{ m_ViewAxis.Clear(); }

	void			SetOrigin(const CVec3f &vOrigin)			{ m_v3Origin = vOrigin; }
	void			SetOrigin(float fX, float fY, float fZ)		{ m_v3Origin.Set(fX, fY, fZ); }
	void			ShiftOrigin(const CVec3f &vecDir)			{ m_v3Origin += vecDir; }
	void			ShiftOrigin(float fX, float fY, float fZ)	{ m_v3Origin += CVec3f(fX, fY, fZ); }
	K2_API void		AddOffset(const CVec3f &v3Offset);
	void			ClearOrigin()								{ m_v3Origin.Clear(); }

	K2_API void		SetTarget(const CVec3f &target);
	void			SetDistance(float fDist)					{ m_v3Origin -= m_ViewAxis[FORWARD] * fDist; }

	const CVec3f&	GetOrigin() const					{ return m_v3Origin; }
	float			GetOrigin(EVectorComponent e) const	{ return m_v3Origin[e]; }
	const CAxis&	GetViewAxis() const					{ return m_ViewAxis; }
	const CVec3f&	GetViewAxis(EAxisComponent e) const { return m_ViewAxis[e]; }
	const CVec3f&	GetAngles() const					{ return m_v3Angles; }

	K2_API CVec3f	ConstructRay(float winx, float winy) const;
	CVec3f			ConstructRay(const CVec2f &vec) const		{ return ConstructRay(vec.x, vec.y); }
	K2_API CVec3f	ConstructRay2(float fX, float fY) const;

	K2_API  bool	IsPointInFront(const CVec3f vec) const;
	bool			IsPointBehind(const CVec3f vec) const		{ return !IsPointInFront(vec); }

	K2_API void		SetFovFromAspect(float fStdFovX, float fStdAspect, float fWeightX, float fWeightY);
	K2_API void		CalcFovY();								// Based on fovx, width and height
	K2_API void		CalcFovX();								// Based on fovy, width and height

	K2_API void		DefaultCamera(float fScreenW, float fScreenH);
	K2_API bool		WorldToScreen(const CVec3f &vecWorld, CVec2f &vecScreen) const;
	K2_API void		WorldToScreenBack(const CVec3f &vecWorld, CVec2f &vecScreen) const;
	K2_API CVec2f	WorldToView(const CVec3f &v3World) const;
	K2_API CVec2f	WorldToViewRotate(const CVec3f &v3Dir) const;
	K2_API bool		IsPointInScreenRect(const CVec3f &vOrigin, const CRectf &rect);

	int				GetFlags() const				{ return m_iFlags; }
	void			SetFlags(int iFlags)			{ m_iFlags = iFlags; }

	bool			HasFlags(int iFlags) const		{ return (m_iFlags & iFlags) != 0; }
	bool			HasAllFlags(int iFlags) const	{ return (m_iFlags & iFlags) == iFlags; }
	void			AddFlags(int iFlags)			{ m_iFlags |= iFlags; }
	void			RemoveFlags(int iFlags)			{ m_iFlags &= ~iFlags; }
	void			ClearFlags()					{ m_iFlags = 0; }
};
//=============================================================================

#endif //__C_CAMERA_H__
