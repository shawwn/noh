// (C)2007 S2 Games
// i_spellsummon.h
//
//=============================================================================
#ifndef __I_SPELLSUMMON_H__
#define __I_SPELLSUMMON_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CGameEvent;
//=============================================================================

//=============================================================================
// ISpellSummon
//=============================================================================
class ISpellSummon : public ISpellItem
{
protected:
	START_ENTITY_CONFIG(ISpellItem)
		DECLARE_ENTITY_CVAR(tstring, PetName)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

public:
	virtual ~ISpellSummon()	{}
	ISpellSummon(CEntityConfig *pConfig);

	virtual bool	ImpactPosition(const CVec3f &v3target, CGameEvent &evImpact);

	static void		ClientPrecache(CEntityConfig *pConfig);
	static void		ServerPrecache(CEntityConfig *pConfig);

	ENTITY_CVAR_ACCESSOR(tstring, PetName, _T(""))

	TYPE_NAME("Sumoning Spell")
};
//=============================================================================

#endif //__I_SPELLITEM_H__
