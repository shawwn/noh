// (C)2006 S2 Games
// i_skilldeploy.h
//
//=============================================================================
#ifndef __I_SKILLDEPLOY_H__
#define __I_SKILLDEPLOY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_skillitem.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// ISkillDeploy
//=============================================================================
class ISkillDeploy : public ISkillItem
{
protected:
	START_ENTITY_CONFIG(ISkillItem)
		DECLARE_ENTITY_CVAR(tstring, GadgetName)
		DECLARE_ENTITY_CVAR(CVec3f, Offset)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

public:
	virtual ~ISkillDeploy()	{}
	ISkillDeploy(CEntityConfig *pConfig) :
	ISkillItem(pConfig),
	m_pEntityConfig(pConfig)
	{}

	bool			IsDeploySkill() const	{ return true; }

	virtual void	Activate()	{ ActivatePrimary(GAME_BUTTON_STATUS_DOWN | GAME_BUTTON_STATUS_PRESSED); }
	virtual void	Impact();

	static void		ClientPrecache(CEntityConfig *pConfig);
	static void		ServerPrecache(CEntityConfig *pConfig);

	// Settings
	ENTITY_CVAR_ACCESSOR(tstring, GadgetName, _T(""))
	ENTITY_CVAR_ACCESSOR(CVec3f, Offset, V3_ZERO)

	TYPE_NAME("Gadget Deployment")
};
//=============================================================================

#endif //__I_SKILLDEPLOY_H__
