// (C)2008 S2 Games
// c_shopdefinition.h
//
//=============================================================================
#ifndef __C_SHOPDEFINITION_H__
#define __C_SHOPDEFINITION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitydefinition.h"

#include "../k2/i_xmlprocessor.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
DECLARE_ENTITY_DEFINITION_XML_PROCESSOR(IShopEntity, Shop, shop)
//=============================================================================

//=============================================================================
// CShopDefinition
//=============================================================================
class CShopDefinition : public IEntityDefinition
{
    DECLARE_DEFINITION_TYPE_INFO

    ENT_DEF_LOCALIZED_STRING_PROPERTY(DisplayName)
    ENT_DEF_LOCALIZED_STRING_PROPERTY(Description)
    ENT_DEF_STRING_PROPERTY(Style)
    ENT_DEF_RESOURCE_PROPERTY(Header, Icon)
    ENT_DEF_RESOURCE_PROPERTY(Icon, Icon)
    ENT_DEF_PROPERTY(AllowRemoteAccess, bool)
    ENT_DEF_PROPERTY(Order, int)
    ENT_DEF_PROPERTY(Slot, int)
    ENT_DEF_PROPERTY(RecommendedItems, bool)

    tsvector        m_vsItems;

public:
    ~CShopDefinition()  {}
    CShopDefinition() :
    IEntityDefinition(&g_allocatorShop)
    {}

    IEntityDefinition*  GetCopy() const { return K2_NEW(g_heapResources,    CShopDefinition)(*this); }

    virtual void    Precache(EPrecacheScheme eScheme)
    {
        IEntityDefinition::Precache(eScheme);

        PRECACHE_GUARD
            PrecacheHeader();
            PrecacheIcon();
        PRECACHE_GUARD_END
    }

    virtual void    GetPrecacheList(EPrecacheScheme eScheme, HeroPrecacheList &deqPrecache)
    {
        IEntityDefinition::GetPrecacheList(eScheme, deqPrecache);

        PRECACHE_GUARD
            deqPrecache.push_back(SHeroPrecache(GetName(), eScheme));
        PRECACHE_GUARD_END
    }

    virtual void    PostProcess()
    {
        if (m_bPostProcessing)
            return;

        IEntityDefinition::PostProcess();

        m_bPostProcessing = true;

        PRECACHE_LOCALIZED_STRING(DisplayName, name);
        PRECACHE_LOCALIZED_STRING(Description, description);

        m_bPostProcessing = false;
    }

    void            AddItem(const tstring &sItem)   { m_vsItems.push_back(sItem); }
    void            SetItems(const tsvector &vList) { m_vsItems = vList; }

    const tstring   GetItem(uint uiSlot) const      { return uiSlot >= 0 && uiSlot < m_vsItems.size() ? m_vsItems[uiSlot] : TSNULL; }
    const tsvector& GetItems() const                { return m_vsItems; }

    void            ClearItems()                    { m_vsItems.clear(); }
};
//=============================================================================

#endif //__C_SHOPDEFINITION_H__
