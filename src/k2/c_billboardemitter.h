// (C)2006 S2 Games
// c_billboardemitter.h
//
//=============================================================================
#ifndef __C_BILLBOARDEMITTER_H__
#define __C_BILLBOARDEMITTER_H__

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
// CBillboardEmitterDef
//=============================================================================
class CBillboardEmitterDef : public IEmitterDef
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
	CTemporalPropertyf			m_tfAlpha;
	CTemporalPropertyRangef		m_trfPitch;
	CTemporalPropertyRangef		m_trfRoll;
	CTemporalPropertyRangef		m_trfYaw;
	CTemporalPropertyRangef		m_trfWidth;
	CTemporalPropertyRangef		m_trfHeight;
	CTemporalPropertyRangef		m_trfScale;
	CTemporalPropertyRangef		m_trfFrame;
	CTemporalPropertyRangef		m_trfParam;
	float						m_fDepthBias;
	uint						m_uiFlags;
	ResHandle					m_hMaterial;
	EDirectionalSpace			m_eDirectionalSpace;
	CTemporalPropertyRangef		m_trfScaleU;
	CTemporalPropertyRangef		m_trfScaleV;
	CTemporalPropertyRangef		m_trfOffsetU;
	CTemporalPropertyRangef		m_trfOffsetV;

public:
	virtual ~CBillboardEmitterDef();
	CBillboardEmitterDef
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
		const CTemporalPropertyf &tfAlpha,
		const CTemporalPropertyRangef &trfPitch,
		const CTemporalPropertyRangef &trfRoll,
		const CTemporalPropertyRangef &trfYaw,
		const CTemporalPropertyRangef &trfWidth,
		const CTemporalPropertyRangef &trfHeight,
		const CTemporalPropertyRangef &trfScale,
		const CTemporalPropertyRangef &trfFrame,
		const CTemporalPropertyRangef &trfParam,
		float fDepthBias,
		uint uiFlags,
		ResHandle hMaterial,
		EDirectionalSpace eDirectionalSpace,
		const CTemporalPropertyRangef &trfScaleU,
		const CTemporalPropertyRangef &trfScaleV,
		const CTemporalPropertyRangef &trfOffsetU,
		const CTemporalPropertyRangef &trfOffsetV
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

	const CTemporalPropertyv3&		GetColor() const			{ return m_tv3Color; }
	const CTemporalPropertyf&		GetAlpha() const			{ return m_tfAlpha; }
	
	CTemporalPropertyf				GetPitch() const			{ return m_trfPitch; }
	CTemporalPropertyf				GetRoll() const				{ return m_trfRoll; }
	CTemporalPropertyf				GetYaw() const				{ return m_trfYaw; }

	CTemporalPropertyf				GetWidth() const			{ return m_trfWidth; }
	CTemporalPropertyf				GetHeight() const			{ return m_trfHeight; }
	CTemporalPropertyf				GetScale() const			{ return m_trfScale; }
	CTemporalPropertyf				GetFrame() const			{ return m_trfFrame; }
	CTemporalPropertyf				GetParam() const			{ return m_trfParam; }

	float							GetDepthBias() const		{ return m_fDepthBias; }

	uint							GetFlags() const			{ return m_uiFlags; }

	ResHandle						GetMaterial() const			{ return m_hMaterial; }
	
	EDirectionalSpace				GetDirectionalSpace() const	{ return m_eDirectionalSpace; }

	CTemporalPropertyf				GetScaleU() const			{ return m_trfScaleU; }
	CTemporalPropertyf				GetScaleV() const			{ return m_trfScaleV; }
	CTemporalPropertyf				GetOffsetU() const			{ return m_trfOffsetU; }
	CTemporalPropertyf				GetOffsetV() const			{ return m_trfOffsetV; }
};

//=============================================================================
// CBillboardEmitter
//=============================================================================
class CBillboardEmitter : public IEmitter
{
private:
	// Emitter Properties
	CTemporalPropertyv3		m_tv3Color;
	CTemporalPropertyf		m_tfAlpha;
	CTemporalPropertyf		m_tfPitch;
	CTemporalPropertyf		m_tfRoll;
	CTemporalPropertyf		m_tfYaw;
	CTemporalPropertyf		m_tfWidth;
	CTemporalPropertyf		m_tfHeight;
	CTemporalPropertyf		m_tfScale;
	CTemporalPropertyf		m_tfFrame;
	CTemporalPropertyf		m_tfParam;
	CTemporalPropertyf		m_tfScaleU;
	CTemporalPropertyf		m_tfScaleV;
	CTemporalPropertyf		m_tfOffsetU;
	CTemporalPropertyf		m_tfOffsetV;

	float					m_fDepthBias;
	uint					m_uiFlags;
	ResHandle				m_hMaterial;

public:
	virtual ~CBillboardEmitter();
	CBillboardEmitter(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner, const CBillboardEmitterDef &eSettings);

	bool	Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace);

	uint	GetNumBillboards();
	bool	GetBillboard(uint uiIndex, SBillboard &outBillboard);
};
//=============================================================================
#endif	//__C_BILLBOARDEMITTER_H__
