// (C)2005 S2 Games
// c_modelallcatorregistry.h
//
//=============================================================================
#ifndef __C_MODELALLOCATORREGISTRY_H__
#define __C_MODELALLOCATORREGISTRY_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_singleton.h"
//=============================================================================

//=============================================================================
// Declarartions
//=============================================================================
class IModelAllocator;
class IModel;
//=============================================================================

//=============================================================================
// CModelAllocatorRegistry
//=============================================================================
class CModelAllocatorRegistry
{
    SINGLETON_DEF(CModelAllocatorRegistry)

private:
    map<tstring, IModelAllocator*>  m_mapAllocators;

public:
    void    Register(const tstring &sType, IModelAllocator *pAllocator);
    IModel* Allocate(const tstring &sType);
};
//=============================================================================

#endif //__C_MODELALLOCATORREGISTRY_H__
