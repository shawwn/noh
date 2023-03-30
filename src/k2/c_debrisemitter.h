// (C)2008 S2 Games
// c_debrisemitter.h
//
//=============================================================================
#ifndef __C_DEBRISEMITTER_H__
#define __C_DEBRISEMITTER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_emitter.h"
#include "c_temporalproperty.h"
#include "c_temporalpropertyrange.h"
#include "c_range.h"
#include "c_skeleton.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
class CParticleSystem;
//=============================================================================

//=============================================================================
// CDebrisEmitterDef
//=============================================================================
class CDebrisEmitterDef : public IEmitterDef
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
	tsvector						m_vEmitters;

	vector<IEmitterDef *>		m_vEmitterDefs;

	CTemporalPropertyRangef		m_rfGravity;
	CTemporalPropertyRangef		m_rfMinSpeed;
	CTemporalPropertyRangef		m_rfMaxSpeed;
	CTemporalPropertyRangef		m_rfMinAcceleration;
	CTemporalPropertyRangef		m_rfMaxAcceleration;
	CTemporalPropertyRangef		m_rfMinAngle;
	CTemporalPropertyRangef		m_rfMaxAngle;
	CTemporalPropertyRangef		m_rfMinInheritVelocity;
	CTemporalPropertyRangef		m_rfMaxInheritVelocity;
	CTemporalPropertyRangef		m_rfLimitInheritVelocity;
	CVec3f						m_v3Dir;
	float						m_fDrag;
	float						m_fFriction;
	CTemporalPropertyv3			m_tv3OffsetSphere;
	CTemporalPropertyv3			m_tv3OffsetCube;
	CTemporalPropertyRangef		m_rfMinOffsetDirection;
	CTemporalPropertyRangef		m_rfMaxOffsetDirection;
	CTemporalPropertyRangef		m_rfMinOffsetRadial;
	CTemporalPropertyRangef		m_rfMaxOffsetRadial;
	CTemporalPropertyRangef		m_rfMinOffsetRadialAngle;
	CTemporalPropertyRangef		m_rfMaxOffsetRadialAngle;
	bool						m_bCollide;
	CTemporalPropertyRangef		m_rfMinRotationSpeed;
	CTemporalPropertyRangef		m_rfMaxRotationSpeed;
	float						m_fBounce;
	float						m_fReflect;
	bool						m_bAnimPose;
	bool						m_bUseAnim;

public:
	virtual ~CDebrisEmitterDef();
	CDebrisEmitterDef
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
		const tsvector &vEmitters,
		const CTemporalPropertyRangef &rfGravity,
		const CTemporalPropertyRangef &rfMinSpeed,
		const CTemporalPropertyRangef &rfMaxSpeed,
		const CTemporalPropertyRangef &rfMinAcceleration,
		const CTemporalPropertyRangef &rfMaxAcceleration,
		const CTemporalPropertyRangef &rfMinAngle,
		const CTemporalPropertyRangef &rfMaxAngle,
		const CTemporalPropertyRangef &rfMinInheritVelocity,
		const CTemporalPropertyRangef &rfMaxInheritVelocity,
		const CTemporalPropertyRangef &rfLimitInheritVelocity,
		const CVec3f &v3Dir,
		float fDrag,
		float fFriction,
		const CTemporalPropertyv3 &tv3OffsetSphere,
		const CTemporalPropertyv3 &tv3OffsetCube,
		const CTemporalPropertyRangef &rfMinOffsetDirection,
		const CTemporalPropertyRangef &rfMaxOffsetDirection,
		const CTemporalPropertyRangef &rfMinOffsetRadial,
		const CTemporalPropertyRangef &rfMaxOffsetRadial,
		const CTemporalPropertyRangef &rfMinOffsetRadialAngle,
		const CTemporalPropertyRangef &rfMaxOffsetRadialAngle,
		bool bCollide,
		const CTemporalPropertyRangef &rfMinRotationSpeed,
		const CTemporalPropertyRangef &rfMaxRotationSpeed,
		float fBounce,
		float fReflect,
		bool bAnimPose,
		bool bUseAnim
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

	const tsvector&					GetEmitters() const			{ return m_vEmitters; }

	void		AddEmitterDef(IEmitterDef *pEmitterDef);
	const vector<IEmitterDef *>&	GetEmitterDefs() const	{ return m_vEmitterDefs; }

	CTemporalPropertyf		GetGravity() const				{ return m_rfGravity; }
	CTemporalPropertyf		GetMinSpeed() const				{ return m_rfMinSpeed; }
	CTemporalPropertyf		GetMaxSpeed() const				{ return m_rfMaxSpeed; }
	CTemporalPropertyf		GetMinAcceleration() const		{ return m_rfMinAcceleration; }
	CTemporalPropertyf		GetMaxAcceleration() const		{ return m_rfMaxAcceleration; }
	CTemporalPropertyf		GetMinAngle() const				{ return m_rfMinAngle; }
	CTemporalPropertyf		GetMaxAngle() const				{ return m_rfMaxAngle; }
	CTemporalPropertyf		GetMinInheritVelocity() const	{ return m_rfMinInheritVelocity; }
	CTemporalPropertyf		GetMaxInheritVelocity() const	{ return m_rfMaxInheritVelocity; }
	CTemporalPropertyf		GetLimitInheritVelocity() const	{ return m_rfLimitInheritVelocity; }
	const CVec3f&			GetDir() const					{ return m_v3Dir; }
	float					GetDrag() const					{ return m_fDrag; }
	float					GetFriction() const				{ return m_fFriction; }
	CTemporalPropertyv3		GetOffsetSphere() const			{ return m_tv3OffsetSphere; }
	CTemporalPropertyv3		GetOffsetCube() const			{ return m_tv3OffsetCube; }
	CTemporalPropertyf		GetMinOffsetDirection() const	{ return m_rfMinOffsetDirection; }
	CTemporalPropertyf		GetMaxOffsetDirection() const	{ return m_rfMaxOffsetDirection; }
	CTemporalPropertyf		GetMinOffsetRadial() const		{ return m_rfMinOffsetRadial; }
	CTemporalPropertyf		GetMaxOffsetRadial() const		{ return m_rfMaxOffsetRadial; }
	CTemporalPropertyf		GetMinOffsetRadialAngle() const	{ return m_rfMinOffsetRadialAngle; }
	CTemporalPropertyf		GetMaxOffsetRadialAngle() const	{ return m_rfMaxOffsetRadialAngle; }
	bool					GetCollide() const				{ return m_bCollide; }
	CTemporalPropertyf		GetMinRotationSpeed() const		{ return m_rfMinRotationSpeed; }
	CTemporalPropertyf		GetMaxRotationSpeed() const		{ return m_rfMaxRotationSpeed; }
	float					GetBounce() const				{ return m_fBounce; }
	float					GetReflect() const				{ return m_fReflect; }
	bool					GetAnimPose() const				{ return m_bAnimPose; }
	bool					GetUseAnim() const				{ return m_bUseAnim; }
};
//=============================================================================

//=============================================================================
// CDebrisEmitter
//=============================================================================
class CDebrisEmitter : public IEmitter
{
private:
	struct SDebrisState
	{
		CVec3f	v3Position;
		CVec4f	v4Rotation;
		
		CVec3f	v3Velocity;
		CVec4f	v4RotationVelocity;

		CVec3f	v3Direction;
		float	fScale;
		float	fAcceleration;

		IEmitter *pImbeddedEmitter;
	};

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

	CTemporalPropertyf		m_rfGravity;
	CTemporalPropertyf		m_rfMinSpeed;
	CTemporalPropertyf		m_rfMaxSpeed;
	CTemporalPropertyf		m_rfMinAcceleration;
	CTemporalPropertyf		m_rfMaxAcceleration;
	CTemporalPropertyf		m_rfMinAngle;
	CTemporalPropertyf		m_rfMaxAngle;
	CTemporalPropertyf		m_rfMinInheritVelocity;
	CTemporalPropertyf		m_rfMaxInheritVelocity;
	CTemporalPropertyf		m_rfLimitInheritVelocity;
	CVec3f					m_v3Dir;
	float					m_fDrag;
	float					m_fFriction;
	CTemporalPropertyv3		m_tv3OffsetSphere;
	CTemporalPropertyv3		m_tv3OffsetCube;
	CTemporalPropertyf		m_rfMinOffsetDirection;
	CTemporalPropertyf		m_rfMaxOffsetDirection;
	CTemporalPropertyf		m_rfMinOffsetRadial;
	CTemporalPropertyf		m_rfMaxOffsetRadial;
	CTemporalPropertyf		m_rfMinOffsetRadialAngle;
	CTemporalPropertyf		m_rfMaxOffsetRadialAngle;
	bool					m_bCollide;
	CTemporalPropertyf		m_rfMinRotationSpeed;
	CTemporalPropertyf		m_rfMaxRotationSpeed;
	float					m_fBounce;
	float					m_fReflect;
	bool					m_bAnimPose;
	bool					m_bUseAnim;
	
	CSkeleton				*m_pSkeleton;

	vector<SDebrisState>	m_vDebrisState;
	vector<SBoneXForm>		m_vBonePose;

	float					m_fLastLerp;
	float					m_fLastTime;

	bool	UpdateEmbeddedEmitter(uint uiMilliseconds, ParticleTraceFn_t pfnTrace, IEmitter *pEmitter, SDebrisState &cDebris);
	void	UpdateSkeleton(float fDeltaTime, const CVec3f &v3Acceleration, float fDrag, float fFriction, ParticleTraceFn_t pfnTrace);

public:
	virtual ~CDebrisEmitter();
	CDebrisEmitter(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner, const CDebrisEmitterDef &eSettings);

	bool	Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace);

	uint	GetNumEntities();
	bool	GetEntity(uint uiIndex, CSceneEntity &outEntity);

	CSkeleton*	GetCustomSkeleton()		{ return m_pSkeleton; }

	uint	GetNumEmitters();
	IEmitter*	GetEmitter(uint uiIndex);
};
//=============================================================================

#endif	//__C_DEBRISEMITTER_H__
