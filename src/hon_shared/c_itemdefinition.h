// (C)2008 S2 Games
// c_itemdefinition.h
//
//=============================================================================
#ifndef __C_ITEMDEFINITION_H__
#define __C_ITEMDEFINITION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_tooldefinition.h"

#include "../k2/i_xmlprocessor.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
DECLARE_ENTITY_DEFINITION_XML_PROCESSOR(IEntityItem, Item, item)
//=============================================================================

//=============================================================================
// CItemDefinition
//=============================================================================
class CItemDefinition : public IToolDefinition
{
	DECLARE_DEFINITION_TYPE_INFO

	ENT_DEF_LOCALIZED_STRING_PROPERTY(ShopFlavorText)
	ENT_DEF_PROPERTY(Cost, int)
	ENT_DEF_PROPERTY(InitialCharges, uint)
	ENT_DEF_PROPERTY(DropOnDeath, bool)
	ENT_DEF_PROPERTY(Unkillable, bool)
	ENT_DEF_PROPERTY(DestroyOnEmpty, bool)
	ENT_DEF_PROPERTY(Rechargeable, bool)
	ENT_DEF_PROPERTY(AutoRecharge, bool)
	ENT_DEF_PROPERTY(NoSell, bool)
	ENT_DEF_PROPERTY(NoStash, bool)
	ENT_DEF_PROPERTY(NoDrop, bool)
	ENT_DEF_PROPERTY(NoPurchaseOwner, bool)
	ENT_DEF_PROPERTY(AllowDisassemble, bool)
	ENT_DEF_PROPERTY(AllowSharing, bool)
	ENT_DEF_PROPERTY(AllowTransfer, bool)
	ENT_DEF_STRING_VECTOR_ARRAY_PROPERTY(Components)
	ENT_DEF_PROPERTY(AutoAssemble, bool)
	ENT_DEF_PROPERTY(InitialStock, uint)
	ENT_DEF_PROPERTY(MaxStock, uint)
	ENT_DEF_PROPERTY(RestockDelay, uint)
	ENT_DEF_PROPERTY(New, bool)
	ENT_DEF_PROPERTY(BindOnPickup, bool)
	ENT_DEF_STRING_PROPERTY(Category)

protected:
	virtual void	PrecacheV(EPrecacheScheme eScheme, const tstring &sModifier)
	{
		IToolDefinition::PrecacheV(eScheme, sModifier);

		PRECACHE_GUARD
			// ...
		PRECACHE_GUARD_END
	}

	virtual void	GetPrecacheListV(EPrecacheScheme eScheme, const tstring &sModifier, HeroPrecacheList &deqPrecache)
	{
		IToolDefinition::GetPrecacheListV(eScheme, sModifier, deqPrecache);

		PRECACHE_GUARD
			// ...
		PRECACHE_GUARD_END
	}

public:
	~CItemDefinition()	{}
	CItemDefinition() :
	IToolDefinition(&g_allocatorItem)
	{}

	IEntityDefinition*	GetCopy() const	{ return K2_NEW(ctx_Game,    CItemDefinition)(*this); }

	virtual void	PostProcess()
	{
		if (m_bPostProcessing)
			return;

		IToolDefinition::PostProcess();

		m_bPostProcessing = true;

		PRECACHE_LOCALIZED_STRING(ShopFlavorText, shop_flavor);

		m_bPostProcessing = false;
	}

	virtual void	ImportDefinition(IEntityDefinition *pOtherDefinition);

	bool	IsRecipe()	{ return GetComponentsSize() > 0; }

	GAME_SHARED_API uint	GetTotalCost(uint uiIndex = 0) const;
	GAME_SHARED_API uint	Assemble(IUnitEntity *pOwner, int iSlot) const;
};
//=============================================================================

#endif //__C_ITEMDEFINITION_H__
