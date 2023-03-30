// (C)2009 S2 Games
// c_treedefinitionresource.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "editor_common.h"

#include "c_treedefinitionresource.h"

#include "../k2/i_resourcelibrary.h"
#include "../k2/c_xmlmanager.h"
//=============================================================================

IResource*  AllocTreeDefinition(const tstring &sPath);

//=============================================================================
// Globals
//=============================================================================
IResourceLibrary    g_ResLibTree(RES_TREE, _T("TreeDefinition"), CTreeDefinitionResource::ResTypeName(), true, AllocTreeDefinition);

//=============================================================================
// Definitions
//=============================================================================
IResource*  AllocTreeDefinition(const tstring &sPath)
{
    return K2_NEW(ctx_Resources,   CTreeDefinitionResource)(sPath);
}

/*====================
  CTreeDefinitionResource::Load
  ====================*/
int     CTreeDefinitionResource::Load(uint uiIgnoreFlags, const char *pData, uint uiSize)
{
    PROFILE("CTreeDefinitionResource::Load");

    Console.Res << _T("Loading ^970Tree definition^*: ") << m_sPath << newl;

    // Process the XML
    if (!XMLManager.ReadBuffer(pData, uiSize, _T(""), this))
    {
        Console.Warn << _T("CTreeDefinitionResource::Load(") + m_sPath + _T(") - couldn't read XML") << newl;
        return RES_LOAD_FAILED;
    }

    return 0;
}


/*====================
  CTreeDefinitionResource::PostLoad
  ====================*/
void    CTreeDefinitionResource::PostLoad()
{
}


/*====================
  CTreeDefinitionResource::Reloaded
  ====================*/
void    CTreeDefinitionResource::Reloaded()
{
}



