// (C)2005 S2 Games
// c_modelallocatorregistry.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_modelallocatorregistry.h"
#include "i_modelallocator.h"
//=============================================================================
SINGLETON_INIT(CModelAllocatorRegistry)

/*====================
  CModelAllocatorRegistry::CModelAllocatorRegistry
  ====================*/
CModelAllocatorRegistry::CModelAllocatorRegistry()
{
}


/*====================
  CModelAllocatorRegistry::Register
  ====================*/
void    CModelAllocatorRegistry::Register(const tstring &sType, IModelAllocator *pAllocator)
{
    map<tstring, IModelAllocator*>::iterator findit(m_mapAllocators.find(sType));
    if (findit != m_mapAllocators.end())
    {
        Console.Warn << _T("ModelAllocator of type ") << sType << _T(" is already registered.") << newl;
        return;
    }

    m_mapAllocators[sType] = pAllocator;
}


/*====================
  CModelAllocatorRegistry::Allocate
  ====================*/
IModel* CModelAllocatorRegistry::Allocate(const tstring &sType)
{
    map<tstring, IModelAllocator*>::iterator findit(m_mapAllocators.find(sType));
    if (findit == m_mapAllocators.end())
    {
        Console.Warn << _T("ModelAllocator of type ") << sType << _T(" is not registered.") << newl;
        return nullptr;
    }

    return findit->second->Allocate();
}
