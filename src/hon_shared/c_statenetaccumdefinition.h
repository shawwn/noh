// (C)2010 S2 Games
// c_statenetaccumdefinition.h
//
//=============================================================================
#ifndef __C_STATENETACCUMDEFINITION_H__
#define __C_STATENETACCUMDEFINITION_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_statedefinition.h"

#include "../k2/i_xmlprocessor.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
DECLARE_ENTITY_DEFINITION_XML_PROCESSOR(IEntityStateNetAccum, StateNetAccum, statenetaccum)
//=============================================================================

//=============================================================================
// CStateNetAccumDefinition
//=============================================================================
class CStateNetAccumDefinition : public CStateDefinition
{
public:
    ~CStateNetAccumDefinition() {}
    CStateNetAccumDefinition() :
    CStateDefinition(&g_allocatorStateNetAccum)
    {}

    IEntityDefinition*  GetCopy() const { return K2_NEW(g_heapResources, CStateNetAccumDefinition)(*this); }
};
//=============================================================================

#endif //__C_STATENETACCUMDEFINITION_H__
