// (C)2007 S2 Games
// c_gadgetdemocharge.h
//
//=============================================================================
#ifndef __C_GADGETDEMOCHARGE_H__
#define __C_GADGETDEMOCHARGE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gadgetentity.h"
//=============================================================================

//=============================================================================
// CGadgetDemoCharge
//=============================================================================
class CGadgetDemoCharge : public IGadgetEntity
{
private:
    START_ENTITY_CONFIG(IGadgetEntity)
        DECLARE_ENTITY_CVAR(float, BlastRadius)
        DECLARE_ENTITY_CVAR(float, BlastDamage)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(Gadget, DemoCharge);

public:
    ~CGadgetDemoCharge()    {}
    CGadgetDemoCharge();
    
    virtual void        Spawn();

    virtual bool        ServerFrame();
};
//=============================================================================

#endif //__C_GADGETDEMOCHARGE_H__
