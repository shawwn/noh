// (C)2008 S2 Games
// c_entitydefinitionresource.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_entitydefinitionresource.h"
#include "c_entityregistry.h"

#include "../k2/i_resourcelibrary.h"
#include "../k2/c_xmlmanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
IResource*  AllocEntityDefinition(const tstring &sPath)
{
    return K2_NEW(g_heapResources,    CEntityDefinitionResource)(sPath);
}

IResourceLibrary    g_ResLibEntityDefinition(RES_ENTITY_DEF, _T("Entity Definitions"), true, AllocEntityDefinition);
//=============================================================================

/*====================
  CEntityDefinitionResource::Load
  ====================*/
int     CEntityDefinitionResource::Load(uint uiIgnoreFlags, const char *pData, uint uiSize)
{
    PROFILE("CEntityDefinitionResource::Load");

    Console.Res << _T("Loading ^970Entity definition^*: ") << m_sPath << newl;

    // Process the XML
    if (!XMLManager.ReadBuffer(pData, uiSize, _T(""), this))
    {
        Console.Warn << _T("CEntityDefinitionResource::Load(") + m_sPath + _T(") - couldn't read XML") << newl;
        return RES_LOAD_FAILED;
    }

    return 0;
}


/*====================
  CEntityDefinitionResource::PostLoad
  ====================*/
void    CEntityDefinitionResource::Free()
{
    if (m_pDefinition != NULL)
    {
        m_unTypeID = EntityRegistry.RegisterDynamicEntity(GetName(), INVALID_RESOURCE, NULL);
        m_pDefinition->SetName(GetName());
        m_pDefinition->SetTypeID(m_unTypeID);
    }
}


/*====================
  CEntityDefinitionResource::PostLoad
  ====================*/
void    CEntityDefinitionResource::PostLoad()
{
    if (m_pDefinition != NULL)
    {
        m_unTypeID = EntityRegistry.RegisterDynamicEntity(GetName(), GetHandle(), m_pDefinition->GetAllocator());
        m_pDefinition->SetName(GetName());
        m_pDefinition->SetTypeID(m_unTypeID);
    }
}


/*====================
  CEntityDefinitionResource::Reloaded
  ====================*/
void    CEntityDefinitionResource::Reloaded()
{
    PostProcess();

    if (m_pDefinition != NULL)
        Game.UpdateDefinitions(m_unTypeID);
}
