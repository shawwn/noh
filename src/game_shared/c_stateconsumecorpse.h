// (C)2006 S2 Games
// c_stateconsumecorpse.h
//
//=============================================================================
#ifndef __C_STATECONSUMECORPSE_H__
#define __C_STATECONSUMECORPSE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateConsumeCorpse
//=============================================================================
class CStateConsumeCorpse : public IEntityState
{
private:
	static vector<SDataField>	*s_pvFields;

	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(uint, MaxCorpses)
		DECLARE_ENTITY_CVAR(uint, CorpseDecayTime)
		DECLARE_ENTITY_CVAR(float, CorpseMultScale)
		DECLARE_ENTITY_CVAR(float, CorpseMultDamage)
		DECLARE_ENTITY_CVAR(float, CorpseAddDamage)
		DECLARE_ENTITY_CVAR(float, CorpseMultHealth)
		DECLARE_ENTITY_CVAR(float, CorpseAddHealth)
		DECLARE_ENTITY_CVAR(float, CorpseMultHealthRegen)
		DECLARE_ENTITY_CVAR(float, CorpseAddHealthRegen)
	END_ENTITY_CONFIG
	
	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, ConsumeCorpse);

	float	m_fCorpsesEaten;
	uint	m_uiLerpTime;

public:
	~CStateConsumeCorpse()	{}
	
	CStateConsumeCorpse() :
	IEntityState(GetEntityConfig()),
	m_pEntityConfig(GetEntityConfig()),
	m_fCorpsesEaten(0.0f),
	m_uiLerpTime(INVALID_TIME)
	{}

	void	StateFrame();

	// Network
	GAME_SHARED_API void	Baseline();
	GAME_SHARED_API void	GetSnapshot(CEntitySnapshot &snapshot) const;
	GAME_SHARED_API bool	ReadSnapshot(CEntitySnapshot &snapshot);

	static const vector<SDataField>&	GetTypeVector();

	ENTITY_CVAR_ACCESSOR(uint, MaxCorpses, 3)
	ENTITY_CVAR_ACCESSOR(uint, CorpseDecayTime, 15000)
	ENTITY_CVAR_ACCESSOR(float, CorpseMultScale, 0.15f)
	ENTITY_CVAR_ACCESSOR(float, CorpseMultDamage, 0.15f)
	ENTITY_CVAR_ACCESSOR(float, CorpseAddDamage, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, CorpseMultHealth, 0.15f)
	ENTITY_CVAR_ACCESSOR(float, CorpseAddHealth, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, CorpseMultHealthRegen, 0.15f)
	ENTITY_CVAR_ACCESSOR(float, CorpseAddHealthRegen, 0.0f)

	void	ConsumeCorpse()			{ m_fCorpsesEaten = MIN(m_fCorpsesEaten + 1.0f, float(GetMaxCorpses())); }
};
//=============================================================================

#endif //__C_STATECONSUMECORPSE_H__
