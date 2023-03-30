// (C)2005 S2 Games
// i_modelallocator.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_modelallocator.h"
#include "c_modelallocatorregistry.h"
//=============================================================================


/*====================
  IModelAllocator::~IModelAllocator
  ====================*/
IModelAllocator::~IModelAllocator()
{
}


/*====================
  IModelAllocator::IModelAllocator
  ====================*/
IModelAllocator::IModelAllocator(const tstring &sType) :
m_sType(sType)
{
    CModelAllocatorRegistry::GetInstance()->Register(m_sType, this);
}
