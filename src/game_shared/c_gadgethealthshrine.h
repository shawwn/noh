// (C)2006 S2 Games
// c_gadgethealthshrine.h
//
//=============================================================================
#ifndef __C_GADGETHEALTHSHRINE_H__
#define __C_GADGETHEALTHSHRINE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gadgetentity.h"
//=============================================================================

//=============================================================================
// CGadgetHealthShrine
//=============================================================================
class CGadgetHealthShrine : public IGadgetEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Gadget, HealthShrine);

public:
    ~CGadgetHealthShrine()  {}
    CGadgetHealthShrine() :
    IGadgetEntity(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_GADGETHEALTHSHRINE_H__
