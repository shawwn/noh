// (C)2008 S2 Games
// c_petdefinition.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_petdefinition.h"
#include "i_petentity.h"

#include "../k2/i_resourcelibrary.h"
#include "../k2/c_xmlmanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
IResource*  AllocPetDefinition(const tstring &sPath, const char *pData, int iSize);

IResourceLibrary    g_ResLibPetDef(RES_PET_DEF, _T("PetDefs"), true, AllocPetDefinition);
//=============================================================================

/*====================
  AllocPetDefinition
  ====================*/
IResource*  AllocPetDefinition(const tstring &sPath, const char *pData, int iSize)
{
    return K2_NEW(g_heapResources,    CPetDefinition)(sPath, pData, iSize);
}


/*====================
  CPetDefinition::CPetDefinition
  ====================*/
CPetDefinition::CPetDefinition(const tstring &sPath, const char *pData, int iSize) :
IEntityDefinition(sPath, pData, iSize, &g_allocatorPet)
{
}


/*====================
  CPetDefinition::Load
  ====================*/
int     CPetDefinition::Load(uint uiIgnoreFlags)
{
    PROFILE("CPetDefinition::Load");

    Console.Res << "Loading pet definition " << SingleQuoteStr(m_sPath) << newl;

    // Process the XML
    if (!XMLManager.ReadBuffer(m_pData, m_iSize, this))
    {
        Console.Warn << _T("CPetDefinition::Load(") + m_sPath + _T(") - couldn't read XML") << newl;
        return RES_LOAD_FAILED;
    }

    SAFE_DELETE_ARRAY(m_pData);
    return 0;
}
