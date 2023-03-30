// (C)2007 S2 Games
// c_traceremitter.h
//
//=============================================================================
#ifndef __C_TRACEREMITTER_H__
#define __C_TRACEREMITTER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_emitter.h"
#include "c_temporalproperty.h"
#include "c_temporalpropertyrange.h"
#include "c_range.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
class CParticleSystem;
//=============================================================================

//=============================================================================
// CTracerEmitterDef
//=============================================================================
class CTracerEmitterDef : public IEmitterDef
{
private:
	// Emitter Properties
	CRangei						m_riLife;
	CRangei						m_riExpireLife;
	CRangei						m_riTimeNudge;
	CRangei						m_riDelay;
	bool						m_bLoop;
	tstring						m_sOwnerA;
	tstring						m_sOwnerB;
	tstring						m_sBoneA;
	tstring						m_sBoneB;
	CVec3f						m_v3PosA;
	CVec3f						m_v3PosB;
	CTemporalPropertyv3			m_tv3Color;
	CTemporalPropertyf			m_tfAlpha;
	CTemporalPropertyRangef		m_trfWidth;
	CTemporalPropertyRangef		m_trfLength;
	CTemporalPropertyRangef		m_trfSpeed;
	CTemporalPropertyRangef		m_trfAcceleration;
	CTemporalPropertyRangef		m_trfTaper;
	CTemporalPropertyRangef		m_trfTile;
	CTemporalPropertyRangef		m_trfFrame;
	CTemporalPropertyRangef		m_trfParam;
	ResHandle					m_hMaterial;

public:
	virtual ~CTracerEmitterDef();
	CTracerEmitterDef
	(
		const CRangei &riLife,
		const CRangei &riExpireLife,
		const CRangei &riTimeNudge,
		const CRangei &riDelay,
		bool bLoop,
		const tstring &sOwnerA,
		const tstring &sOwnerB,
		const tstring &sBoneA,
		const tstring &sBoneB,
		const CVec3f &v3PosA,
		const CVec3f &v3PosB,
		const CTemporalPropertyv3 &tv3Color,
		const CTemporalPropertyf &tfAlpha,
		const CTemporalPropertyRangef &trfWidth,
		const CTemporalPropertyRangef &trfLength,
		const CTemporalPropertyRangef &trfSpeed,
		const CTemporalPropertyRangef &trfAcceleration,
		const CTemporalPropertyRangef &trfTaper,
		const CTemporalPropertyRangef &trfTile,
		const CTemporalPropertyRangef &trfFrame,
		const CTemporalPropertyRangef &trfParam,
		ResHandle hMaterial
	);

	IEmitter*	Spawn(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner);

	int			GetLife() const			{ return m_riLife; }
	int			GetExpireLife() const	{ return m_riExpireLife; }
	int			GetTimeNudge() const	{ return m_riTimeNudge; }
	int			GetDelay() const		{ return m_riDelay; }
	bool		GetLoop() const			{ return m_bLoop; }

	const tstring&	GetOwnerA() const		{ return m_sOwnerA; }
	const tstring&	GetOwnerB() const		{ return m_sOwnerB; }
	const tstring&	GetBoneA() const		{ return m_sBoneA; }
	const tstring&	GetBoneB() const		{ return m_sBoneB; }
	const CVec3f&	GetPosA() const			{ return m_v3PosA; }
	const CVec3f&	GetPosB() const			{ return m_v3PosB; }

	const CTemporalPropertyv3&		GetColor() const			{ return m_tv3Color; }
	const CTemporalPropertyf&		GetAlpha() const			{ return m_tfAlpha; }

	CTemporalPropertyf		GetWidth() const		{ return m_trfWidth; }
	CTemporalPropertyf		GetLength() const		{ return m_trfLength; }
	CTemporalPropertyf		GetSpeed() const		{ return m_trfSpeed; }
	CTemporalPropertyf		GetAcceleration() const	{ return m_trfAcceleration; }
	CTemporalPropertyf		GetTaper() const		{ return m_trfTaper; }
	CTemporalPropertyf		GetTile() const			{ return m_trfTile; }
	CTemporalPropertyf		GetFrame() const		{ return m_trfFrame; }
	CTemporalPropertyf		GetParam() const		{ return m_trfParam; }

	ResHandle	GetMaterial() const			{ return m_hMaterial; }
};

//=============================================================================
// CTracerEmitter
//=============================================================================
class CTracerEmitter : public IEmitter
{
private:
	CVec3f		m_v3LastPosA;
	CVec3f		m_v3LastPosB;

	float		m_fBeamStartPos;

	// Emitter Properties
	IEmitter	*m_pOwnerA;
	IEmitter	*m_pOwnerB;
	tstring		m_sBoneA;
	tstring		m_sBoneB;
	CVec3f		m_v3PosA;
	CVec3f		m_v3PosB;

	CTemporalPropertyv3		m_tv3Color;
	CTemporalPropertyf		m_tfAlpha;
	CTemporalPropertyf		m_tfWidth;
	CTemporalPropertyf		m_tfLength;
	CTemporalPropertyf		m_tfSpeed;
	CTemporalPropertyf		m_tfAcceleration;
	CTemporalPropertyf		m_tfTaper;
	CTemporalPropertyf		m_tfTile;
	CTemporalPropertyf		m_tfFrame;
	CTemporalPropertyf		m_tfParam;

	ResHandle	m_hMaterial;

public:
	virtual ~CTracerEmitter();
	CTracerEmitter(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner, const CTracerEmitterDef &eSettings);

	bool	Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace);

	uint	GetNumBeams();
	bool	GetBeam(uint uiIndex, SBeam &outBeam);

	virtual void	OnDelete(IEmitter *pEmitter);
};

//=============================================================================
#endif	//__C_TRACEREMITTER_H__
