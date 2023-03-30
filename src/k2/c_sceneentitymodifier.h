// (C)2007 S2 Games
// c_sceneentitymodifier.h
//
//=============================================================================
#ifndef __C_SCENEENTITYMODIFIER_H__
#define __C_SCENEENTITYMODIFIER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_effectinstance.h"
#include "i_emitter.h"
#include "c_temporalproperty.h"
#include "c_temporalpropertyrange.h"
#include "c_range.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CModifierDef
//=============================================================================
class CModifierDef
{
private:
	CRangei						m_riLife;
	CRangei						m_riExpireLife;
	CRangei						m_riTimeNudge;
	CRangei						m_riDelay;
	bool						m_bLoop;
	CTemporalPropertyv3			m_tv3Color;
	CTemporalPropertyf			m_tfAlpha;
	CTemporalPropertyf			m_tfParam0;
	CTemporalPropertyf			m_tfParam1;
	CTemporalPropertyf			m_tfParam2;
	CTemporalPropertyf			m_tfParam3;
	ResHandle					m_hMaterial;
	CTemporalPropertyv3			m_tv3Offset;
	tstring						m_sSkin;

public:
	~CModifierDef();
	CModifierDef
	(
		const CRangei &riLife,
		const CRangei &riExpireLife,
		const CRangei &riTimeNudge,
		const CRangei &riDelay,
		bool bLoop,
		const CTemporalPropertyv3 &tv3Color,
		const CTemporalPropertyf &tfAlpha,
		const CTemporalPropertyf &tfParam0,
		const CTemporalPropertyf &tfParam1,
		const CTemporalPropertyf &tfParam2,
		const CTemporalPropertyf &tfParam3,
		ResHandle hMaterial,
		const CTemporalPropertyv3 &tv3Offset,
		const tstring	 &sSkin
	);

	int			GetLife() const			{ return m_riLife; }
	int			GetExpireLife() const	{ return m_riExpireLife; }
	int			GetTimeNudge() const	{ return m_riTimeNudge; }
	int			GetDelay() const		{ return m_riDelay; }
	bool		GetLoop() const			{ return m_bLoop; }

	const CTemporalPropertyv3&		GetColor() const			{ return m_tv3Color; }
	const CTemporalPropertyf&		GetAlpha() const			{ return m_tfAlpha; }
	const CTemporalPropertyf&		GetParam0() const			{ return m_tfParam0; }
	const CTemporalPropertyf&		GetParam1() const			{ return m_tfParam1; }
	const CTemporalPropertyf&		GetParam2() const			{ return m_tfParam2; }
	const CTemporalPropertyf&		GetParam3() const			{ return m_tfParam3; }

	ResHandle						GetMaterial() const			{ return m_hMaterial; }

	const CTemporalPropertyv3&		GetOffset() const			{ return m_tv3Offset; }

	const tstring&					GetSkin() const				{ return m_sSkin; }
};
//=============================================================================

//=============================================================================
// CSceneEntityModifier
//=============================================================================
class CSceneEntityModifier : public IEffectInstance
{
private:
	uint			m_uiStartTime;
	uint			m_uiLastUpdateTime;
	bool			m_bActive;
	bool			m_bDead;

	int			m_iLife;
	int			m_iTimeNudge;
	int			m_iDelay;
	bool		m_bLoop;

	CTemporalPropertyv3		m_tv3Color;
	CTemporalPropertyf		m_tfAlpha;
	CTemporalPropertyf		m_tfParam0;
	CTemporalPropertyf		m_tfParam1;
	CTemporalPropertyf		m_tfParam2;
	CTemporalPropertyf		m_tfParam3;
	ResHandle				m_hMaterial;
	CTemporalPropertyv3		m_tv3Offset;
	tstring					m_sSkin;

public:
	~CSceneEntityModifier();
	CSceneEntityModifier();

	CSceneEntityModifier(uint uiStartTime, CEffectThread *pEffectThread, const CModifierDef &cSettings);

	virtual bool	IsModifier() const		{ return true; }

	K2_API bool	Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace = NULL);
	K2_API bool	IsDead();

	K2_API void	Modify(CSceneEntity &cEntity) const;

	void	Expire(uint uiMilliseconds)	{}
};
//=============================================================================
#endif	//__C_SCENEENTITYMODIFIER_H__
