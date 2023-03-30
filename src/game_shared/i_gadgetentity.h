// (C)2006 S2 Games
// i_gadgetentity.h
//
//=============================================================================
#ifndef __I_GADGETENTITY_H__
#define __I_GADGETENTITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gameentity.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class IGadgetSentry;
class IGadgetPortal;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint MAX_GADGET_COUNTERS(3);
//=============================================================================

//=============================================================================
// IGadgetEntity
//=============================================================================
class IGadgetEntity : public IVisualEntity
{
private:
	static vector<SDataField>	*s_pvFields;

	IGadgetEntity();

protected:
	START_ENTITY_CONFIG(IVisualEntity)
		DECLARE_ENTITY_CVAR(uint, Lifetime)
		DECLARE_ENTITY_CVAR(uint, CorpseTime)
		DECLARE_ENTITY_CVAR(tstring, StateName)
		DECLARE_ENTITY_CVAR(float, StateRadius)
		DECLARE_ENTITY_CVAR(uint, StateDuration)
		DECLARE_ENTITY_CVAR(bool, StateTargetEnemy)
		DECLARE_ENTITY_CVAR(bool, StateTargetAlly)
		DECLARE_ENTITY_CVAR(uint, BuildTime)
		DECLARE_ENTITY_CVAR(bool, IsInvulnerable)
		DECLARE_ENTITY_CVAR(float, ExperiencePerMinute)
	END_ENTITY_CONFIG
	
	CEntityConfig*	m_pEntityConfig;

	int				m_iOwnerClientNumber;
	uint			m_uiOwnerIndex;
	uint			m_uiSpawnTime;
	ushort			m_unDamageID;
	CVec3f			m_v3ViewAngles;

	float			m_fTotalExperience;
	uint			m_auiCounter[MAX_GADGET_COUNTERS];
	svector			m_vCounterLabels;

	bool	m_bAccessed;	// Client side
	uiset	m_setAccessors;	// Server side

public:
	virtual ~IGadgetEntity();
	IGadgetEntity(CEntityConfig *pConfig);

	GAME_SHARED_API static const vector<SDataField>&	GetTypeVector();

	virtual int			GetPrivateClient()				{ return m_iOwnerClientNumber; }

	virtual bool		IsGadget() const				{ return true; }
	
	virtual const IGadgetSentry*	GetAsSentryGadget() const	{ return NULL; }
	virtual const IGadgetPortal*	GetAsPortalGadget() const	{ return NULL; }

	virtual IGadgetSentry*			GetAsSentryGadget()			{ return NULL; }
	virtual IGadgetPortal*			GetAsPortalGadget()			{ return NULL; }

	void				Accessed()						{ m_bAccessed = true; }

	virtual bool		IsSelectable() const			{ return true; }
	
	virtual bool			HasAltInfo() const			{ return true; }
	virtual const tstring&	GetAltInfoName() const		{ return GetEntityName(); }

	virtual void		Baseline();
	virtual void		GetSnapshot(CEntitySnapshot &snapshot) const;
	virtual bool		ReadSnapshot(CEntitySnapshot &snapshot);

	void				SetOwner(uint uiIndex)						{ m_uiOwnerIndex = uiIndex; }
	uint				GetOwnerIndex() const						{ return m_uiOwnerIndex; }
	void				SetOwnerClientNumber(int iClient)			{ m_iOwnerClientNumber = iClient; }
	int					GetOwnerClientNumber() const				{ return m_iOwnerClientNumber; }

	void				SetViewAngles(const CVec3f &v3Angles)		{ m_v3ViewAngles = v3Angles; }
	void				SetViewAngles(float p, float r, float y)	{ m_v3ViewAngles.Set(p, r, y); }
	CVec3f				GetViewAngles() const						{ return m_v3ViewAngles; }
	float				GetViewAngle(EEulerComponent e) const		{ return m_v3ViewAngles[e]; }

	void				SetDamageID(ushort unID)					{ m_unDamageID = unID; }
	ushort				GetDamageID() const							{ return m_unDamageID; }

	uint				GetSpawnTime() const						{ return m_uiSpawnTime; }

	virtual CSkeleton*	AllocateSkeleton();

	virtual bool		AIShouldTarget()			{ return !GetIsInvulnerable(); }

	virtual void		Spawn();
	virtual float		Damage(float fDamage, int iFlags, IVisualEntity *pAttacker = NULL, ushort unDamagingObjectID = INVALID_ENT_TYPE, bool bFeedback = true);
	virtual void		Kill(IVisualEntity *pAttacker = NULL, ushort unKillingObjectID = INVALID_ENT_TYPE);

	virtual uint		GetRemainingLifetime() const;
	virtual float		GetRemainingLifetimePercent() const	{ if (m_pEntityConfig->GetLifetime() == 0) return 1.0f; else return GetRemainingLifetime() / float(m_pEntityConfig->GetLifetime()); }
	virtual void		Link();
	virtual void		Unlink();

	virtual bool		CanSpawn();
	virtual void		SpawnPreview();

	virtual float		GiveDeploymentExperience();
	virtual	bool		ServerFrame();

	virtual void		UpdateSkeleton(bool bPose);
	virtual bool		AddToScene(const CVec4f &v4Color, int iFlags);

	GAME_SHARED_API static bool		TestSpawn(ResHandle hModel, const CVec3f &v3Position, const CVec3f &v3Angles, float fScale);

	GAME_SHARED_API virtual void	Copy(const IGameEntity &B);

	static void			ClientPrecache(CEntityConfig *pConfig);
	static void			ServerPrecache(CEntityConfig *pConfig);

	float			GetExperienceAccumulator() const	{ return m_fTotalExperience; }
	const tstring&	GetCounterLabel(uint uiIndex) const	{ if (uiIndex >= INT_SIZE(m_vCounterLabels.size())) return SNULL; return m_vCounterLabels[uiIndex]; }
	uint			GetCounterValue(uint uiIndex) const	{ return m_auiCounter[CLAMP(uiIndex, 0u, MAX_GADGET_COUNTERS - 1)]; }
	void			IncrementCounter(uint uiIndex)		{ ++m_auiCounter[CLAMP(uiIndex, 0u, MAX_GADGET_COUNTERS - 1)]; }

	ENTITY_CVAR_ACCESSOR(bool, IsInvulnerable, false)
	ENTITY_CVAR_ACCESSOR(uint, Lifetime, 0)
};
//=============================================================================

#endif //__I_GADGETENTITY_H__
