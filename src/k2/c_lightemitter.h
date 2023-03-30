// (C)2006 S2 Games
// c_lightemitter.h
//
//=============================================================================
#ifndef __C_LIGHTEMITTER_H__
#define __C_LIGHTEMITTER_H__

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
// CLightEmitterDef
//=============================================================================
class CLightEmitterDef : public IEmitterDef
{
private:
	// Emitter Properties
	tstring						m_sOwner;
	CRangei						m_riLife;
	CRangei						m_riExpireLife;
	CRangei						m_riTimeNudge;
	CRangei						m_riDelay;
	bool						m_bLoop;
	tstring						m_sBone;
	CVec3f						m_v3Pos;
	CVec3f						m_v3Offset;
	CTemporalPropertyv3			m_tv3Color;
	CTemporalPropertyRangef		m_trfFalloffStart;
	CTemporalPropertyRangef		m_trfFalloffEnd;
	CTemporalPropertyRangef		m_trfFlickerAmount;
	CTemporalPropertyRangef		m_trfFlickerFrequency;

public:
	virtual ~CLightEmitterDef();
	CLightEmitterDef
	(
		const tstring &sOwner,
		const CRangei &riLife,
		const CRangei &riExpireLife,
		const CRangei &riTimeNudge,
		const CRangei &riDelay,
		bool bLoop,
		const tstring &sBone,
		const CVec3f &v3Pos,
		const CVec3f &v3Offset,
		const CTemporalPropertyv3 &tv3Color,
		const CTemporalPropertyRangef &trfFalloffStart,
		const CTemporalPropertyRangef &trfFalloffEnd,
		const CTemporalPropertyRangef &trfFlickerAmount,
		const CTemporalPropertyRangef &trfFlickerFrequency
	);

	IEmitter*	Spawn(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner);

	const tstring&	GetOwner() const	{ return m_sOwner; }

	int			GetLife() const			{ return m_riLife; }
	int			GetExpireLife() const	{ return m_riExpireLife; }
	int			GetTimeNudge() const	{ return m_riTimeNudge; }
	int			GetDelay() const		{ return m_riDelay; }
	bool		GetLoop() const			{ return m_bLoop; }

	const tstring&	GetBone() const		{ return m_sBone; }
	const CVec3f&	GetPos() const		{ return m_v3Pos; }
	const CVec3f&	GetOffset() const	{ return m_v3Offset; }

	const CTemporalPropertyv3&	GetColor() const			{ return m_tv3Color; }
	CTemporalPropertyf			GetFalloffStart() const		{ return m_trfFalloffStart; }
	CTemporalPropertyf			GetFalloffEnd() const		{ return m_trfFalloffEnd; }
	CTemporalPropertyf			GetFlickerAmount() const	{ return m_trfFlickerAmount; }
	CTemporalPropertyf			GetFlickerFrequency() const	{ return m_trfFlickerFrequency; }
};

//=============================================================================
// CLightEmitter
//=============================================================================
class CLightEmitter : public IEmitter
{
private:
	CTemporalPropertyv3		m_tv3Color;
	CTemporalPropertyf		m_tfFalloffStart;
	CTemporalPropertyf		m_tfFalloffEnd;
	CTemporalPropertyf		m_tfFlickerAmount;
	CTemporalPropertyf		m_tfFlickerFrequency;

public:
	virtual ~CLightEmitter();
	CLightEmitter(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner, const CLightEmitterDef &eSettings);

	bool	Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace);

	uint	GetNumLights();
	bool	GetLight(uint uiIndex, CSceneLight &outLight);
};

//=============================================================================
#endif	//__C_LIGHTEMITTER_H__
