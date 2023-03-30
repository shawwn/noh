// (C)2008 S2 Games
// c_petdefinition.h
//
//=============================================================================
#ifndef __C_PETDEFINITION_H__
#define __C_PETDEFINITION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_unitdefinition.h"

#include "../k2/i_xmlprocessor.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
DECLARE_ENTITY_DEFINITION_XML_PROCESSOR(IPetEntity, Pet, pet)
//=============================================================================

//=============================================================================
// CPetDefinition
//=============================================================================
class CPetDefinition : public IUnitDefinition
{
	DECLARE_DEFINITION_TYPE_INFO

	ENT_DEF_ARRAY_PROPERTY(IsPersistent, bool)
	ENT_DEF_ARRAY_PROPERTY(Lifetime, uint)

public:
	~CPetDefinition()	{}
	CPetDefinition() :
	IUnitDefinition(&g_allocatorPet)
	{}

	IEntityDefinition*	GetCopy() const	{ return K2_NEW(g_heapResources,    CPetDefinition)(*this); }

	virtual void	GetPrecacheList(EPrecacheScheme eScheme, HeroPrecacheList &deqPrecache)
	{
		IUnitDefinition::GetPrecacheList(eScheme, deqPrecache);
		
		PRECACHE_GUARD
			deqPrecache.push_back(SHeroPrecache(GetName(), eScheme));
		PRECACHE_GUARD_END
	}
};
//=============================================================================

#endif //__C_PETDEFINITION_H__
