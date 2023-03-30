// (C)2008 S2 Games
// c_controlleremitter.h
//
//=============================================================================
#ifndef __C_CONTROLLEREMITTER_H__
#define __C_CONTROLLEREMITTER_H__

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

enum EDirectionType
{
	DIRTYPE_LOCK = 0,
};

enum EPositionType
{
	POSTYPE_LOCK = 0,
};
//=============================================================================

//=============================================================================
// CControllerEmitterDef
//=============================================================================
class CControllerEmitterDef : public IEmitterDef
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
	CTemporalPropertyRangef		m_trfPitch;
	CTemporalPropertyRangef		m_trfRoll;
	CTemporalPropertyRangef		m_trfYaw;
	CTemporalPropertyRangef		m_trfScale;
	tsvector						m_vEmitters;

	bool						m_bLookAt;
	CVec3f						m_v3LookAtPos;
	CVec3f						m_v3LookAtOffset;
	tstring						m_sLookAtOwner;
	tstring						m_sLookAtBone;
	EDirectionalSpace			m_eLookAtDirectionalSpace;

	vector<IEmitterDef *>		m_vEmitterDefs;

public:
	virtual ~CControllerEmitterDef();
	CControllerEmitterDef
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
		const CTemporalPropertyRangef &trfPitch,
		const CTemporalPropertyRangef &trfRoll,
		const CTemporalPropertyRangef &trfYaw,
		const CTemporalPropertyRangef &trfScale,
		const tsvector &vEmitters,
		bool bLookAt,
		const CVec3f &v3LookAtPos,
		const CVec3f &v3LookAtOffset,
		const tstring &sLookAtOwner,
		const tstring &sLookAtBone,
		EDirectionalSpace eLookAtDirectionalSpace
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

	CTemporalPropertyf			GetPitch() const			{ return m_trfPitch; }
	CTemporalPropertyf			GetRoll() const				{ return m_trfRoll; }
	CTemporalPropertyf			GetYaw() const				{ return m_trfYaw; }
	CTemporalPropertyf			GetScale() const			{ return m_trfScale; }

	const tsvector&				GetEmitters() const			{ return m_vEmitters; }

	bool						GetLookAt() const			{ return m_bLookAt; }
	const CVec3f&				GetLookAtPos() const		{ return m_v3LookAtPos; }
	const CVec3f&				GetLookAtOffset() const		{ return m_v3LookAtOffset; }
	const tstring&				GetLookAtOwner() const		{ return m_sLookAtOwner; }
	const tstring&				GetLookAtBone() const		{ return m_sLookAtBone; }
	EDirectionalSpace			GetLookAtDirectionalSpace() const	{ return m_eLookAtDirectionalSpace; }

	void		AddEmitterDef(IEmitterDef *pEmitterDef);
	const vector<IEmitterDef *>&	GetEmitterDefs() const	{ return m_vEmitterDefs; }
};
//=============================================================================

//=============================================================================
// CControllerEmitter
//=============================================================================
class CControllerEmitter : public IEmitter
{
private:
	// Emitter Properties
	CTemporalPropertyf		m_tfPitch;
	CTemporalPropertyf		m_tfRoll;
	CTemporalPropertyf		m_tfYaw;
	CTemporalPropertyf		m_tfScale;

	bool					m_bSpawned;
	EDirectionType			m_eDirectionType;
	EPositionType			m_ePositionType;

	IEmitter				*m_pImbeddedEmitter;

	bool					m_bLookAt;
	CVec3f					m_v3LookAtPos;
	CVec3f					m_v3LookAtOffset;
	IEmitter				*m_pLookAtOwner;
	tstring					m_sLookAtBone;
	EDirectionalSpace		m_eLookAtDirectionalSpace;

	bool	UpdateEmbeddedEmitter(uint uiMilliseconds, ParticleTraceFn_t pfnTrace, bool bExpire);

public:
	virtual ~CControllerEmitter();
	CControllerEmitter(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner, const CControllerEmitterDef &eSettings);

	bool	Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace);

	uint	GetNumEmitters();
	IEmitter*	GetEmitter(uint uiIndex);
};
//=============================================================================

#endif	//__C_CONTROLLEREMITTER_H__
