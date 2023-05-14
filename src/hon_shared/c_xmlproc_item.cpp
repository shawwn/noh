// (C)2008 S2 Games
// c_xmlproc_item.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_itemdefinition.h"
#include "i_entityitem.h"

#include "../k2/c_xmlprocroot.h"
//=============================================================================

DEFINE_DEFINITION_TYPE_INFO(CItemDefinition, ENTITY_BASE_TYPE_ITEM, Item)

START_ENTITY_DEFINITION_XML_PROCESSOR(IEntityItem, Item)
    IToolDefinition::ReadSettings(pDefinition, node, bMod);

    pDefinition->SetShopFlavorTextPriority(pDefinition->GetPriority());

    READ_ENTITY_DEFINITION_PROPERTY(Cost, cost)
    READ_ENTITY_DEFINITION_PROPERTY(InitialCharges, initialcharges)
    READ_ENTITY_DEFINITION_PROPERTY(DropOnDeath, dropondeath)
    READ_ENTITY_DEFINITION_PROPERTY(Unkillable, unkillable)
    READ_ENTITY_DEFINITION_PROPERTY(DestroyOnEmpty, destroyonempty)
    READ_ENTITY_DEFINITION_PROPERTY(Rechargeable, rechargeable)
    READ_ENTITY_DEFINITION_PROPERTY(AutoRecharge, autorecharge)
    READ_ENTITY_DEFINITION_PROPERTY(NoSell, nosell)
    READ_ENTITY_DEFINITION_PROPERTY(NoStash, nostash)
    READ_ENTITY_DEFINITION_PROPERTY(NoDrop, nodrop)
    READ_ENTITY_DEFINITION_PROPERTY(AllowDisassemble, allowdisassemble)
    READ_ENTITY_DEFINITION_PROPERTY(AllowSharing, allowsharing)
    READ_ENTITY_DEFINITION_PROPERTY(AllowTransfer, allowtransfer)
    READ_ENTITY_DEFINITION_PROPERTY(Components, components)
    READ_ENTITY_DEFINITION_PROPERTY(AutoAssemble, autoassemble)
    READ_ENTITY_DEFINITION_PROPERTY(InitialStock, initialstock)
    READ_ENTITY_DEFINITION_PROPERTY(MaxStock, maxstock)
    READ_ENTITY_DEFINITION_PROPERTY(RestockDelay, restockdelay)
    READ_ENTITY_DEFINITION_PROPERTY(New, new)
    READ_ENTITY_DEFINITION_PROPERTY(BindOnPickup, bindonpickup)
    READ_ENTITY_DEFINITION_PROPERTY(Category, category)
END_ENTITY_DEFINITION_XML_PROCESSOR(Item, item)

ENTITY_DEF_MERGE_START(CItemDefinition, IToolDefinition)
    MERGE_LOCALIZED_STRING_PROPERTY(ShopFlavorText);
    MERGE_PROPERTY(Cost)
    MERGE_PROPERTY(InitialCharges)
    MERGE_PROPERTY(DropOnDeath)
    MERGE_PROPERTY(Unkillable)
    MERGE_PROPERTY(DestroyOnEmpty)
    MERGE_PROPERTY(Rechargeable)
    MERGE_PROPERTY(AutoRecharge)
    MERGE_PROPERTY(NoSell)
    MERGE_PROPERTY(NoStash)
    MERGE_PROPERTY(NoDrop)
    MERGE_PROPERTY(AllowDisassemble)
    MERGE_PROPERTY(AllowSharing)
    MERGE_PROPERTY(AllowTransfer)
    MERGE_STRING_VECTOR_ARRAY_PROPERTY(Components)
    MERGE_PROPERTY(AutoAssemble)
    MERGE_PROPERTY(InitialStock)
    MERGE_PROPERTY(MaxStock)
    MERGE_PROPERTY(RestockDelay)
    MERGE_PROPERTY(New)
    MERGE_PROPERTY(BindOnPickup)
    MERGE_STRING_PROPERTY(Category)
ENTITY_DEF_MERGE_END


/*====================
  CItemDefinition::GetTotalCost
  ====================*/
uint    CItemDefinition::GetTotalCost(uint uiIndex) const
{
    uint uiCost(GetCost());
    
    if (uiIndex >= GetComponentsSize())
        return uiCost;

    const tsvector &vComponents(GetComponents(uiIndex));
    for (tsvector_cit it(vComponents.begin()); it != vComponents.end(); ++it)
    {
        CItemDefinition *pComponentDef(EntityRegistry.GetDefinition<CItemDefinition>(*it));
        if (pComponentDef == nullptr)
            continue;

        uiCost += pComponentDef->GetTotalCost();
    }

    return uiCost;
}


/*====================
  CItemDefinition::Assemble
  ====================*/
uint    CItemDefinition::Assemble(IUnitEntity *pOwner, int iSlot) const
{
    static vector<IEntityItem*> s_vComponentItems;

    if (pOwner == nullptr)
        return -1;

    bool bComplete(false);

    // Determine domain
    bool bInStash(iSlot >= INVENTORY_START_STASH && iSlot <= INVENTORY_STASH_PROVISIONAL);
    uint uiStartSlot(bInStash ? INVENTORY_START_STASH : INVENTORY_START_BACKPACK);
    uint uiEndSlot(bInStash ? INVENTORY_STASH_PROVISIONAL : INVENTORY_BACKPACK_PROVISIONAL);

    IEntityItem *pBaseItem(pOwner->GetItem(iSlot));
    int iPurchaserClientNumber(pBaseItem != nullptr ? pBaseItem->GetPurchaserClientNumber() : -1);

    // Check each component list
    uint uiIndex(0);
    for (; uiIndex < GetComponentsSize(); ++uiIndex)
    {
        s_vComponentItems.clear();

        // Look for each component in the inventory
        const tsvector &vComponents(GetComponents(uiIndex));
        for (tsvector_cit it(vComponents.begin()); it != vComponents.end(); ++it)
        {
            ushort unTypeID(EntityRegistry.LookupID(*it));
            if (unTypeID == INVALID_ENT_TYPE)
            {
                Console.Warn << _T("Bad component ") << QuoteStr(*it) << _T(" in recipe: ") << GetName() << newl;
                break;
            }
            
            // Check each slot for this item
            bool bFound(false);
            for (uint uiSlot(uiStartSlot); uiSlot <= uiEndSlot; ++uiSlot)
            {
                IEntityItem *pItem(pOwner->GetItem(uiSlot));
                if (pItem == nullptr || pItem->GetType() != unTypeID || !pItem->HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED))
                    continue;
                if (pItem->GetPurchaserClientNumber() != -1 && pItem->GetPurchaserClientNumber() != iPurchaserClientNumber)
                    continue;
                
                bool bUsed(false);
                for (vector<IEntityItem*>::iterator it(s_vComponentItems.begin()); it != s_vComponentItems.end(); ++it)
                {
                    if ((*it)->GetSlot() == uiSlot)
                    {
                        bUsed = true;
                        break;
                    }
                }
                if (bUsed)
                    continue;

                s_vComponentItems.push_back(pItem);
                bFound = true;
                break;
            }

            if (!bFound)
                break;
        }

        if (s_vComponentItems.size() == vComponents.size())
        {
            bComplete = true;
            break;
        }
    }

    if (!bComplete)
        return -1;
    
    for (vector<IEntityItem*>::iterator it(s_vComponentItems.begin()); it != s_vComponentItems.end(); ++it)
    {
        IEntityItem *pItem(*it);
        if (pItem->GetMaxCharges() != 0 && pItem->GetCharges() > 1 && pItem->GetRechargeable())
            pItem->RemoveCharge();
        else
            pOwner->RemoveItem(pItem->GetSlot());
    }

    return uiIndex;
}
