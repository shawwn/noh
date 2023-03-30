// (C)2006 S2 Games
// c_modelemitter.h
//
//=============================================================================
#ifndef __C_MODELEMITTER_H__
#define __C_MODELEMITTER_H__

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
class CSkeleton;
//=============================================================================

//=============================================================================
// CModelEmitterDef
//=============================================================================
class CModelEmitterDef : public IEmitterDef
{
private:
	// Emitter Properties
	tstring						m_sName;
	tstring						m_sOwner;
	CRangei						m_riLife;
	CRangei						m_riExpireLife;
	CRangei						m_riTimeNudge;
	CRangei						m_riDelay;
	bool						m_bLoop;
	EDirectionalSpace			m_eDirectionalSpace;
	tstring						m_sBone;
	CVec3f						m_v3Pos;
	CVec3f						m_v3Offset;
	CTemporalPropertyv3			m_tv3Color;
	CTemporalPropertyf			m_tfAlpha;
	CTemporalPropertyRangef		m_trfPitch;
	CTemporalPropertyRangef		m_trfRoll;
	CTemporalPropertyRangef		m_trfYaw;
	CTemporalPropertyRangef		m_trfScale;
	CTemporalPropertyRangef		m_trfParam0;
	CTemporalPropertyRangef		m_trfParam1;
	CTemporalPropertyRangef		m_trfParam2;
	CTemporalPropertyRangef		m_trfParam3;
	ResHandle					m_hModel;
	SkinHandle					m_hSkin;
	ResHandle					m_hMaterial;
	tstring						m_sAnim;
	bool						m_bParentModel;
	bool						m_bParentSkeleton;
	tsvector						m_vEmitters;
	
	vector<IEmitterDef *>		m_vEmitterDefs;

public:
	virtual ~CModelEmitterDef();
	CModelEmitterDef
	(
		const tstring &sName,
		const tstring &sOwner,
		const CRangei &riLife,
		const CRangei &riExpireLife,
		const CRangei &riTimeNudge,
		const CRangei &riDelay,
		bool bLoop,
		EDirectionalSpace eDirectionalSpace,
		const tstring &sBone,
		const CVec3f &v3Pos,
		const CVec3f &v3Offset,
		const CTemporalPropertyv3 &tv3Color,
		const CTemporalPropertyf &tfAlpha,
		const CTemporalPropertyRangef &trfPitch,
		const CTemporalPropertyRangef &trfRoll,
		const CTemporalPropertyRangef &trfYaw,
		const CTemporalPropertyRangef &trfScale,
		const CTemporalPropertyRangef &trfParam0,
		const CTemporalPropertyRangef &trfParam1,
		const CTemporalPropertyRangef &trfParam2,
		const CTemporalPropertyRangef &trfParam3,
		ResHandle hModel,
		SkinHandle hSkin,
		ResHandle hMaterial,
		const tstring &sAnim,
		bool bParentModel,
		bool bParentSkeleton,
		const tsvector &vEmitters
	);

	IEmitter*	Spawn(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner);

	const tstring&	GetName() const		{ return m_sName; }
	const tstring&	GetOwner() const	{ return m_sOwner; }

	int			GetLife() const			{ return m_riLife; }
	int			GetExpireLife() const	{ return m_riExpireLife; }
	int			GetTimeNudge() const	{ return m_riTimeNudge; }
	int			GetDelay() const		{ return m_riDelay; }
	bool		GetLoop() const			{ return m_bLoop; }

	EDirectionalSpace	GetDirectionalSpace() const		{ return m_eDirectionalSpace; }
	const tstring&		GetBone() const					{ return m_sBone; }
	const CVec3f&		GetPos() const					{ return m_v3Pos; }
	const CVec3f&		GetOffset() const				{ return m_v3Offset; }

	const CTemporalPropertyv3&		GetColor() const			{ return m_tv3Color; }
	const CTemporalPropertyf&		GetAlpha() const			{ return m_tfAlpha; }
	
	CTemporalPropertyf				GetPitch() const			{ return m_trfPitch; }
	CTemporalPropertyf				GetRoll() const				{ return m_trfRoll; }
	CTemporalPropertyf				GetYaw() const				{ return m_trfYaw; }
	CTemporalPropertyf				GetScale() const			{ return m_trfScale; }
	CTemporalPropertyf				GetParam0() const			{ return m_trfParam0; }
	CTemporalPropertyf				GetParam1() const			{ return m_trfParam1; }
	CTemporalPropertyf				GetParam2() const			{ return m_trfParam2; }
	CTemporalPropertyf				GetParam3() const			{ return m_trfParam3; }

	ResHandle						GetModel() const			{ return m_hModel; }
	SkinHandle						GetSkin() const				{ return m_hSkin; }
	ResHandle						GetMaterial() const			{ return m_hMaterial; }
	const tstring&					GetAnim() const				{ return m_sAnim; }

	bool							GetParentModel() const		{ return m_bParentModel; }
	bool							GetParentSkeleton() const	{ return m_bParentSkeleton; }
	const tsvector&					GetEmitters() const			{ return m_vEmitters; }

	void		AddEmitterDef(IEmitterDef *pEmitterDef);
	const vector<IEmitterDef *>&	GetEmitterDefs() const	{ return m_vEmitterDefs; }
};
//=============================================================================

//=============================================================================
// CModelEmitter
//=============================================================================
class CModelEmitter : public IEmitter
{
private:
	// Emitter Properties
	CTemporalPropertyv3		m_tv3Color;
	CTemporalPropertyf		m_tfAlpha;
	CTemporalPropertyf		m_tfPitch;
	CTemporalPropertyf		m_tfRoll;
	CTemporalPropertyf		m_tfYaw;
	CTemporalPropertyf		m_tfScale;
	CTemporalPropertyf		m_tfParam0;
	CTemporalPropertyf		m_tfParam1;
	CTemporalPropertyf		m_tfParam2;
	CTemporalPropertyf		m_tfParam3;

	ResHandle				m_hModel;
	SkinHandle				m_hSkin;
	ResHandle				m_hMaterial;
		
	tstring					m_sAnim;
	
	bool					m_bParentModel;
	bool					m_bParentSkeleton;
	IEmitter				*m_pImbeddedEmitter;
	CSkeleton				*m_pSkeleton;
	bool					m_bVisible;

	bool	UpdateEmbeddedEmitter(uint uiMilliseconds, ParticleTraceFn_t pfnTrace, bool bExpire);

public:
	virtual ~CModelEmitter();
	CModelEmitter(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner, const CModelEmitterDef &eSettings);

	bool	Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace);

	uint	GetNumEntities();
	bool	GetEntity(uint uiIndex, CSceneEntity &outEntity);

	CSkeleton*	GetCustomSkeleton()		{ return m_pSkeleton; }

	uint	GetNumEmitters();
	IEmitter*	GetEmitter(uint uiIndex);

	virtual void	Expire(uint uiMilliseconds);
};
//=============================================================================

#endif	//__C_MODELEMITTER_H__
