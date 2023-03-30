// (C)2006 S2 Games
// c_meleebatteringram.h
//
//=============================================================================
#ifndef __C_MELEEBATTERINGRAM_H__
#define __C_MELEEBATTERINGRAM_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_meleeitem.h"
//=============================================================================

//=============================================================================
// CMeleeBatteringRam
//=============================================================================
class CMeleeBatteringRam : public IMeleeItem
{
private:
	START_ENTITY_CONFIG(IMeleeItem)
		DECLARE_ENTITY_CVAR(tstring, HitBuildingEffectPath)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(Melee, BatteringRam);

public:
	~CMeleeBatteringRam()	{}
	CMeleeBatteringRam() :
	IMeleeItem(GetEntityConfig()),
	m_pEntityConfig(GetEntityConfig())
	{}

	virtual void	Impact();

	int		GetPrimaryDamageFlags()		{ return DAMAGE_FLAG_SIEGE | DAMAGE_FLAG_NOTRANGED; }

	bool	ActivateSecondary(int iButtonStatus)	{ return false; }
	bool	ActivateTertiary(int iButtonStatus)		{ return false; }

	static void		ClientPrecache(CEntityConfig *pConfig);
	static void		ServerPrecache(CEntityConfig *pConfig);

	ENTITY_CVAR_ACCESSOR(tstring, HitBuildingEffectPath, _T(""));
};
//=============================================================================

#endif //__C_MELEEBATTERINGRAM_H__
