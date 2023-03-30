// (C)2006 S2 Games
// c_gadgetshieldgenerator.h
//
//=============================================================================
#ifndef __C_GADGETSHIELDGENERATOR_H__
#define __C_GADGETSHIELDGENERATOR_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gadgetentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EGadgetShieldCounters
{
	SHIELD_COUNTER_SHOTS_DEFLECTED
};
//=============================================================================

//=============================================================================
// CGadgetShieldGenerator
//=============================================================================
class CGadgetShieldGenerator : public IGadgetEntity
{
private:
	START_ENTITY_CONFIG(IGadgetEntity)
		DECLARE_ENTITY_CVAR(tstring, ShieldSurfaceModelPath)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(Gadget, ShieldGenerator)

	uint		m_uiShieldSurfaceWorldIndex;
	ResHandle	m_hShieldSurfaceModel;
	uiset		m_setReflected;

public:
	~CGadgetShieldGenerator();
	CGadgetShieldGenerator();

	void	Baseline();
	void	Spawn();

	void	Link();
	void	Unlink();

	bool	Impact(STraceInfo &trace, IVisualEntity *pSource);

	void	Copy(const IGameEntity &B);

	static void			ClientPrecache(CEntityConfig *pConfig);
	static void			ServerPrecache(CEntityConfig *pConfig);

	void				Hit(CVec3f v3Pos, CVec3f v3Angle, EEntityHitByType eHitBy = ENTITY_HIT_BY_RANGED)	{}
};
//=============================================================================

#endif //__C_GADGETSHIELDGENERATOR_H__
