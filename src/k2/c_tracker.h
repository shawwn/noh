// (C)2006 S2 Games
// c_tracker.h
//
//=============================================================================
#ifndef __C_TRACKER_H__
#define __C_TRACKER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_temporalproperty.h"
#include "c_temporalpropertyrange.h"
#include "c_simpleparticle.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum ETrackType
{
	TRACK_DISTANCE = 0,
	TRACK_ANGULAR,
	TRACK_GRAVITY,
	TRACK_CGRAVITY,
	TRACK_TARGET,
	TRACK_LERP
};
//=============================================================================

//=============================================================================
// CTracker
//=============================================================================
class CTracker
{
private:
	bool		m_bActive;
	uint		m_uiStartTime;
	float		m_fStartDistance;
	int			m_iLife;
	float		m_fTrackSpeed;

	CVec3f		m_v3Pos;
	CVec3f		m_v3Velocity;
	CVec3f		m_v3Dir;
	float		m_fAcceleration;
	float		m_fScale;

	CTemporalPropertyv3	m_tv3Color;
	CTemporalPropertyf	m_tfAlpha;
	CTemporalPropertyf	m_tfWidth;
	CTemporalPropertyf	m_tfHeight;
	CTemporalPropertyf	m_tfScale;
	CTemporalPropertyf	m_tfAngle;
	CTemporalPropertyf	m_tfPitch;
	CTemporalPropertyf	m_tfYaw;
	CTemporalPropertyf	m_tfFrame;
	CTemporalPropertyf	m_tfParam;

	float		m_fS1, m_fS2;
	float		m_fT1, m_fT2;

	CVec2f		m_v2Center;

	uint		m_uiFlags;
	IEmitter	*m_pImbeddedEmitter;

public:
	~CTracker();
	CTracker() : m_bActive(false), m_pImbeddedEmitter(NULL) {};

	void Spawn
	(
		uint uiStartTime,
		int iLife,
		const CVec3f &v3Pos,
		const CVec3f &v3Velocity,
		const CVec3f &v3Dir,
		float fAcceleration,
		float fScale,
		const CVec3f &v3Target,
		float fTrackSpeed,
		IEmitter *pImbeddedEmitter,
		const CSimpleParticleDef &settings
	);

	void	AddToStartTime(uint uiPauseTime) { m_uiStartTime += uiPauseTime; }
	bool	IsDead(uint uiLastUpdateTime, bool bExpired) { return m_iLife == -1 ? bExpired : uiLastUpdateTime > m_uiStartTime + m_iLife; }

	void	Update(float fDeltaTime, const CVec3f &v3Acceleration, float fDrag, float fFriction, const CVec3f &v3Target, ETrackType eTrackType, bool bDistanceLife);

	void	GetBillboard(uint uiMilliseconds, bool bDistanceLife, const CVec3f &v3Target, SBillboard &outBillboard);

	bool	IsActive()				{ return m_bActive; }
	void	SetActive(bool bActive)	{ m_bActive = bActive; }

	float	GetLerp(uint uiMilliseconds, bool bDistanceLife, const CVec3f &v3Target) const;
	float	GetTime(uint uiMilliseconds) const			{ return (uiMilliseconds - m_uiStartTime) * SEC_PER_MS; }
	float	GetScale(float fLerp, float fTime) const	{ return m_tfScale.Evaluate(fLerp, fTime) * m_fScale; }
	float	GetRoll(float fLerp, float fTime) const		{ return m_tfAngle.Evaluate(fLerp, fTime); }
	float	GetPitch(float fLerp, float fTime) const	{ return m_tfPitch.Evaluate(fLerp, fTime); }
	float	GetYaw(float fLerp, float fTime) const		{ return m_tfYaw.Evaluate(fLerp, fTime); }
	
	const CVec3f&	GetPos() const			{ return m_v3Pos; }
	const CVec3f&	GetDir() const			{ return m_v3Dir; }
	const CVec3f&	GetVelocity() const		{ return m_v3Velocity; }
	int				GetLife() const			{ return m_iLife; }
	uint			GetFlags() const		{ return m_uiFlags; }
	
	IEmitter*		GetImbeddedEmitter()	{ return m_pImbeddedEmitter; }
	void			SetImbeddedEmitter(IEmitter *pEmitter)	{ m_pImbeddedEmitter = pEmitter; }
	void			SetPos(const CVec3f &v3Pos)				{ m_v3Pos = v3Pos; }
};
//=============================================================================

#endif	//__C_TRACKER_H__
