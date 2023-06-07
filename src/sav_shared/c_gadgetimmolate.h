// (C)2006 S2 Games
// c_gadgetimmolate.h
//
//=============================================================================
#ifndef __C_GADGETIMMOLATE_H__
#define __C_GADGETIMMOLATE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gadgetentity.h"
//=============================================================================

//=============================================================================
// CGadgetImmolate
//=============================================================================
class CGadgetImmolate : public IGadgetEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Gadget, Immolate);

public:
    ~CGadgetImmolate()  {}
    CGadgetImmolate() :
    IGadgetEntity(GetEntityConfig())
    {}

    bool                    ServerFrame();

    GAME_SHARED_API virtual bool    AIShouldTarget()            { return false; }
};
//=============================================================================

#endif //__C_GADGETIMMOLATE_H__
