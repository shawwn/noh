// (C)2007 S2 Games
// i_gadgetsentry.h
//
//=============================================================================
#ifndef __I_GADGETSENTRY_H__
#define __I_GADGETSENTRY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gadgetentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EGadgetSentryCounters
{
	SENTRY_COUNTER_SIGHTINGS,
	SENTRY_COUNTER_SIEGE_SIGHTINGS,
	SENTRY_COUNTER_REVEALS
};
//=============================================================================

//=============================================================================
// IGadgetSentry
//=============================================================================
class IGadgetSentry : public IGadgetEntity
{
protected:
	START_ENTITY_CONFIG(IGadgetEntity)
		DECLARE_ENTITY_CVAR(tstring, SightedEffectPath)
		DECLARE_ENTITY_CVAR(float, ExperiencePerSighting)
		DECLARE_ENTITY_CVAR(float, ExperiencePerSiegeSighting)
		DECLARE_ENTITY_CVAR(float, ExperiencePerReveal)
		DECLARE_ENTITY_CVAR(float, ExperiencePrincipal)
	END_ENTITY_CONFIG
	
	CEntityConfig*	m_pEntityConfig;

	iset			m_setNeighbors;
	map<uint, uint>	m_mapSightList;

	float			m_fDeploymentExpAccumulator;

public:
	virtual ~IGadgetSentry()	{}
	IGadgetSentry(CEntityConfig *pConfig) :
	IGadgetEntity(pConfig),
	m_pEntityConfig(pConfig),
	m_fDeploymentExpAccumulator(0.0f)
	{
		for (int i(0); i < MAX_GADGET_COUNTERS; ++i)
			m_auiCounter[i] = 0;

		m_vCounterLabels.push_back(_T("Total sightings"));
		m_vCounterLabels.push_back(_T("Siege sighted"));
		m_vCounterLabels.push_back(_T("Revealed"));
	}

	IGadgetSentry*			GetAsSentryGadget()			{ return this; }
	const IGadgetSentry*	GetAsSentryGadget() const	{ return this; }

	virtual void	Baseline();

	virtual void	Spawn();

	virtual float	GiveDeploymentExperience();
	virtual bool	ServerFrame();

	uint	GetLastSeenTime(uint uiUniqueIndex);
	void	EntitySpotted(uint uiUniqueIndex, uint uiTime);

	ENTITY_CVAR_ACCESSOR(tstring, SightedEffectPath, SNULL)
};
//=============================================================================

#endif //__I_GADGETSENTRY_H__
