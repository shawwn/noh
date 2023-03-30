// (C)2007 S2 Games
// i_spelltoggle.h
//
//=============================================================================
#ifndef __I_SPELLTOGGLE_H__
#define __I_SPELLTOGGLE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// ISpellToggle
//=============================================================================
class ISpellToggle : public ISpellItem
{
protected:
	START_ENTITY_CONFIG(ISpellItem)
		DECLARE_ENTITY_CVAR(tstring, EndAnimName)
		DECLARE_ENTITY_CVAR(uint, FinishTime)
		DECLARE_ENTITY_CVAR(float, ManaCostPerSecond)
		DECLARE_ENTITY_CVAR(tstring, SelfState)
		DECLARE_ENTITY_CVAR(uint, MaxTime)
		DECLARE_ENTITY_CVAR(tstring, ActiveEffectPath)
		DECLARE_ENTITY_CVAR(bool, AllowManaRegen)
		DECLARE_ENTITY_CVAR(tstring, ActiveIconPath)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	uint	m_uiActivationTime;
	int		m_iSelfStateSlot;

public:
	virtual ~ISpellToggle()	{}
	ISpellToggle(CEntityConfig *pConfig) :
	ISpellItem(pConfig),
	m_pEntityConfig(pConfig),
	m_uiActivationTime(0),
	m_iSelfStateSlot(-1)
	{}

	bool	IsToggleSpell() const	{ return true; }

	virtual void	ActiveFrame();
	virtual void	Deactivate();
	virtual bool	ActivatePrimary(int iButtonStatus);

	GAME_SHARED_API virtual const tstring&		GetIconImageList();

	const tstring&	GetCurrentIconPath();

	static void		ClientPrecache(CEntityConfig *pConfig);
	static void		ServerPrecache(CEntityConfig *pConfig);

	TYPE_DESCRIPTION("Toggle Spell")

	ENTITY_CVAR_ACCESSOR(tstring, EndAnimName, _T(""))
	ENTITY_CVAR_ACCESSOR(tstring, ActiveEffectPath, _T(""))
	ENTITY_CVAR_ACCESSOR(uint, FinishTime, 0)
	ENTITY_CVAR_ACCESSOR(tstring, SelfState, _T(""))
	ENTITY_CVAR_ACCESSOR(float, ManaCostPerSecond, 0.0f)
	ENTITY_CVAR_ACCESSOR(uint, MaxTime, 0)
	ENTITY_CVAR_ACCESSOR(bool, AllowManaRegen, false)
	ENTITY_CVAR_ACCESSOR(tstring, ActiveIconPath, _T(""))
};
//=============================================================================

#endif //__I_SPELLTOGGLE_H__
