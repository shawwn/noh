// (C)2007 S2 Games
// c_gadgetentangletrap.h
//
//=============================================================================
#ifndef __C_ENTANGLETTRAP_H__
#define __C_ENTANGLETTRAP_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gadgetentity.h"
//=============================================================================

//=============================================================================
// CGadgetEntangleTrap
//=============================================================================
class CGadgetEntangleTrap : public IGadgetEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Gadget, EntangleTrap)

public:
    ~CGadgetEntangleTrap()  {}
    CGadgetEntangleTrap() :
    IGadgetEntity(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_ENTANGLETTRAP_H__
