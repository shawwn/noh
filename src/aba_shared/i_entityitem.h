// (C)2008 S2 Games
// i_entityitem.h
//
//=============================================================================
#ifndef __I_ENTITYITEM_H__
#define __I_ENTITYITEM_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitytool.h"
#include "c_itemdefinition.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
EXTERN_CVAR_STRING(g_itemRecipeIconPath);
class CEntityChest;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef vector<class IEntityItem*>  ItemVector;
typedef ItemVector::iterator        ItemVector_it;
typedef ItemVector::const_iterator  ItemVector_cit;
//=============================================================================

//=============================================================================
// IEntityItem
//=============================================================================
class IEntityItem : public IEntityTool
{
    DECLARE_ENTITY_DESC

public:
    typedef CItemDefinition TDefinition;

private:
    int     m_iPurchaserClientNumber;
    uint    m_uiPurchaseTime;
    uint    m_uiRecipeVariation;

public:
    virtual ~IEntityItem()  {}
    IEntityItem();

    SUB_ENTITY_ACCESSOR(IEntityItem, Item)

    // Network
    GAME_SHARED_API virtual void    Baseline();
    GAME_SHARED_API virtual void    GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
    GAME_SHARED_API virtual bool    ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);

    void            SetRecipeVariation(uint uiIndex)        { m_uiRecipeVariation = uiIndex; }
    uint            GetRecipeVariation() const              { return m_uiRecipeVariation; }

    void            SetPurchaseTime(uint uiTime)            { m_uiPurchaseTime = uiTime; }
    uint            GetPurchaseTime() const                 { return m_uiPurchaseTime; }

    // if uiTime or GetPurchaseTime() is INVALID_TIME, then sets purchase time to INVALID_TIME.
    // otherwise, sets purchase time to the minimum of uiTime and GetPurchaseTime().
    void            UpdatePurchaseTime(uint uiTime);

    int             GetPurchaserClientNumber() const        { return m_iPurchaserClientNumber; }
    void            SetPurchaserClientNumber(int iClient)   { m_iPurchaserClientNumber = iClient; }

    // returns true if the item is within the sellback grace period.
    virtual bool    WasPurchasedRecently() const;

    virtual void    Delete();

    virtual bool    IsActive() const                        { return CanUse() && IEntityTool::IsActive(); }
    virtual void    Spawn();
    virtual bool    ServerFrameAction();
    virtual bool    ServerFrameCleanup();

    virtual ushort  GetCharges() const          { if (!HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED)) return 0; return IEntityTool::GetCharges(); }
    virtual bool    CanDrop();

    virtual CEntityChest*       Drop(const CVec3f &v3Pos, bool bLoseOwnership);
    static CEntityChest*        Drop(ushort unID, const CVec3f &v3Pos);

    virtual int                 GetValue() const;

    virtual int                 Assemble();

    virtual EEntityToolAction   GetActionType() const   { if (!HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED)) return TOOL_ACTION_PASSIVE; return IEntityTool::GetActionType(); }
    virtual bool                IsDisabled() const;
    virtual bool                CanOrder()              { if (!HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED)) return true; return CanUse() && IEntityTool::CanOrder(); }
    virtual bool                CanActivate()           { if (!HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED)) return true; return CanUse() && IEntityTool::CanActivate(); }
    virtual bool                CanUse() const;
    virtual bool                IsBorrowed() const;
    virtual bool                IsRecipe() const;
    virtual bool                BelongsToClient(int iClientNumber) const    { return m_iPurchaserClientNumber == -1 || m_iPurchaserClientNumber == iClientNumber; }
    virtual bool                BelongsToEveryone() const                   { return m_iPurchaserClientNumber == -1; }

    virtual bool                CanStack(ushort unItemID, int iPurchaserClientNumber) const;
    virtual bool                CanStack(IEntityItem *pItem) const;
    virtual bool                CanAccess(IUnitEntity *pUnit) const;

    static void                 ClientPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme);
    static void                 ServerPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme);
    
    ENTITY_DEFINITION_ACCESSOR(const tstring&, ShopFlavorText)
    ENTITY_DEFINITION_ACCESSOR(int, Cost)
    ENTITY_DEFINITION_ACCESSOR(int, TotalCost)
    ENTITY_DEFINITION_ACCESSOR(uint, InitialCharges)
    ENTITY_DEFINITION_ACCESSOR(bool, DropOnDeath)
    ENTITY_DEFINITION_ACCESSOR(bool, Unkillable)
    ENTITY_DEFINITION_ACCESSOR(bool, DestroyOnEmpty)
    ENTITY_DEFINITION_ACCESSOR(bool, Rechargeable)
    ENTITY_DEFINITION_ACCESSOR(bool, AutoRecharge)
    ENTITY_DEFINITION_ACCESSOR(bool, NoSell)
    ENTITY_DEFINITION_ACCESSOR(bool, NoStash)
    ENTITY_DEFINITION_ACCESSOR(bool, NoDrop)
    ENTITY_DEFINITION_ACCESSOR(bool, AllowDisassemble)
    ENTITY_DEFINITION_ARRAY_ACCESSOR(const tsvector&, Components)
    ENTITY_DEFINITION_ACCESSOR(bool, AutoAssemble)
    ENTITY_DEFINITION_ACCESSOR(bool, AllowSharing)
    ENTITY_DEFINITION_ACCESSOR(bool, AllowTransfer)
    ENTITY_DEFINITION_ACCESSOR(uint, InitialStock)
    ENTITY_DEFINITION_ACCESSOR(uint, MaxStock)
    ENTITY_DEFINITION_ACCESSOR(uint, RestockDelay)
    ENTITY_DEFINITION_ACCESSOR(bool, BindOnPickup)
    ENTITY_DEFINITION_ACCESSOR(const tstring &, Category)
};
//=============================================================================

#endif //__I_ENTITYITEM_H__
