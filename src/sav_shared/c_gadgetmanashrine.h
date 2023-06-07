// (C)2006 S2 Games
// c_gadgetmanashrine.h
//
//=============================================================================
#ifndef __C_GADGETMANASHRINE_H__
#define __C_GADGETMANASHRINE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gadgetentity.h"
//=============================================================================

//=============================================================================
// CGadgetManaShrine
//=============================================================================
class CGadgetManaShrine : public IGadgetEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Gadget, ManaShrine);

public:
    ~CGadgetManaShrine()    {}
    CGadgetManaShrine() :
    IGadgetEntity(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_GADGETMANASHRINE_H__
