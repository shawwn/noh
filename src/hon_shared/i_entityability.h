// (C)2008 S2 Games
// i_entityabiliy.h
//
//=============================================================================
#ifndef __I_ENTITYABILITY_H__
#define __I_ENTITYABILITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitytool.h"
#include "c_abilitydefinition.h"
//=============================================================================

//=============================================================================
// IEntityAbility
//=============================================================================
class IEntityAbility : public IEntityTool
{
	DECLARE_ENTITY_DESC

public:
	typedef CAbilityDefinition TDefinition;
	
public:
	virtual ~IEntityAbility()	{}
	IEntityAbility();

	SUB_ENTITY_ACCESSOR(IEntityAbility, Ability)

	// Network
	GAME_SHARED_API virtual void	Baseline();
	GAME_SHARED_API virtual void	GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
	GAME_SHARED_API virtual bool	ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);

	void	LevelUp();
	bool	CanLevelUp() const;

	virtual bool	IsDisabled() const;
	virtual bool	CanOrder();
	virtual bool	CanActivate();

	virtual ResHandle	GetEffect();

	ENTITY_DEFINITION_ARRAY_ACCESSOR(uint, RequiredLevel)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(const tstring&, Interface)
	ENTITY_DEFINITION_ACCESSOR(int, SubSlot)
	ENTITY_DEFINITION_ACCESSOR(int, KeySlot)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, NoSilence)
};
//=============================================================================

#endif //__I_ENTITYABILITY_H__
