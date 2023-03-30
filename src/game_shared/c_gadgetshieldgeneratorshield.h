// (C)2006 S2 Games
// c_gadgetshieldgeneratorshield.h
//
//=============================================================================
#ifndef __C_GADGETSHIELDGENERATORSHIELD_H__
#define __C_GADGETSHIELDGENERATORSHIELD_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gadgetentity.h"
//=============================================================================

//=============================================================================
// CGadgetShieldGeneratorShield
//=============================================================================
class CGadgetShieldGeneratorShield : public IGadgetEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Gadget, ShieldGeneratorShield);

public:
    ~CGadgetShieldGeneratorShield() {}
    CGadgetShieldGeneratorShield();

    virtual void        Link();
};
//=============================================================================

#endif //__C_GADGETSHIELDGENERATORSHIELD_H__
