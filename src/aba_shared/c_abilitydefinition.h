// (C)2008 S2 Games
// c_abilitydefinition.h
//
//=============================================================================
#ifndef __C_ABILITYDEFINITION_H__
#define __C_ABILITYDEFINITION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_tooldefinition.h"

#include "../k2/i_xmlprocessor.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
DECLARE_ENTITY_DEFINITION_XML_PROCESSOR(IEntityAbility, Ability, ability)
//=============================================================================

//=============================================================================
// CAbilityDefinition
//=============================================================================
class CAbilityDefinition : public IToolDefinition
{
	DECLARE_DEFINITION_TYPE_INFO

	ENT_DEF_ARRAY_PROPERTY(RequiredLevel, uint)
	ENT_DEF_STRING_ARRAY_PROPERTY(Interface)
	ENT_DEF_PROPERTY(SubSlot, int)
	ENT_DEF_PROPERTY(KeySlot, int)
	ENT_DEF_ARRAY_PROPERTY(NoSilence, bool)

private:

public:
	~CAbilityDefinition()	{}
	CAbilityDefinition() :
	IToolDefinition(&g_allocatorAbility)
	{}
	CAbilityDefinition(IBaseEntityAllocator *pAllocator) :
	IToolDefinition(pAllocator)
	{}

	static void		ReadSettings(CAbilityDefinition *pDefinition, const class CXMLNode &node, bool bMod);

	IEntityDefinition*	GetCopy() const	{ return K2_NEW(g_heapResources,    CAbilityDefinition)(*this); }

	virtual void	Precache(EPrecacheScheme eScheme)
	{
		IToolDefinition::Precache(eScheme);

		PRECACHE_GUARD
			// ...
		PRECACHE_GUARD_END
	}

	virtual void	GetPrecacheList(EPrecacheScheme eScheme, HeroPrecacheList &deqPrecache)
	{
		IToolDefinition::GetPrecacheList(eScheme, deqPrecache);
		
		PRECACHE_GUARD
			deqPrecache.push_back(SHeroPrecache(GetName(), eScheme));
		PRECACHE_GUARD_END
	}

	virtual void	ImportDefinition(IEntityDefinition *pOtherDefinition);
};
//=============================================================================

#endif //__C_ABILITYDEFINITION_H__
