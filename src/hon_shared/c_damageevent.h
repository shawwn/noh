// (C)2008 S2 Games
// c_damageevent.h
//
//=============================================================================
#ifndef __C_DAMAGEEVENT_H__
#define __C_DAMAGEEVENT_H__

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CDamageEvent
//=============================================================================
class CDamageEvent
{
private:
	ESuperType	m_eSuperType;
	uint		m_uiEffectType;
	float		m_fAttemptedDamage;
	float		m_fAppliedDamage;
	float		m_fDeflection;
	float		m_fArmorPierce;
	float		m_fMagicArmorPierce;

	uint		m_uiTargetIndex;
	uint		m_uiAttackerIndex;
	uint		m_uiInflictorIndex;
	
	int			m_iFlags;

public:
	~CDamageEvent()	{}
	CDamageEvent() :
	m_eSuperType(SUPERTYPE_INVALID),
	m_uiEffectType(0),
	m_fAttemptedDamage(0.0f),
	m_fAppliedDamage(0.0f),
	m_fDeflection(0.0f),
	m_fArmorPierce(0.0f),
	m_fMagicArmorPierce(0.0f),
	m_uiTargetIndex(INVALID_INDEX),
	m_uiAttackerIndex(INVALID_INDEX),
	m_uiInflictorIndex(INVALID_INDEX),
	m_iFlags(0)
	{}

	void		SetFlag(int iFlag)					{ m_iFlags |= iFlag; }
	bool		HasFlag(int iFlag) const			{ return (m_iFlags & iFlag) != 0; }

	void		SetSuperType(ESuperType eType)		{ m_eSuperType = eType; }
	void		SetEffectType(uint uiEffectType)	{ m_uiEffectType = uiEffectType; }
	void		SetAmount(float fDamage)			{ m_fAttemptedDamage = fDamage; }
	
	void		SetTargetIndex(uint uiIndex)		{ m_uiTargetIndex = uiIndex; }
	void		SetAttackerIndex(uint uiIndex)		{ m_uiAttackerIndex = uiIndex; }
	void		SetInflictorIndex(uint uiIndex)		{ m_uiInflictorIndex = uiIndex; }

	void		AddDeflection(float fAmount)		{ m_fDeflection = MAX(fAmount, m_fDeflection); }
	void		SetDeflection(float fAmount)		{ m_fDeflection = fAmount; }

	void		SetArmorPierce(float fPierce)		{ m_fArmorPierce = fPierce; }
	void		SetMagicArmorPierce(float fPierce)	{ m_fMagicArmorPierce = fPierce; }

	ESuperType	GetSuperType() const				{ return m_eSuperType; }
	uint		GetEffectType() const				{ return m_uiEffectType; }
	float		GetAttemptedDamage() const			{ return m_fAttemptedDamage; }
	float		GetAppliedDamage() const			{ return m_fAppliedDamage; }
	float		GetDeflection() const				{ return m_fDeflection; }
	uint		GetTargetIndex() const				{ return m_uiTargetIndex; }
	uint		GetAttackerIndex() const			{ return m_uiAttackerIndex; }
	uint		GetInflictorIndex() const			{ return m_uiInflictorIndex; }
	float		GetArmorPierce() const				{ return m_fArmorPierce; }
	float		GetMagicArmorPierce() const			{ return m_fMagicArmorPierce; }

	GAME_SHARED_API void	ApplyDamage();
};
//=============================================================================

#endif //__C_DAMAGEEVENT_H__
