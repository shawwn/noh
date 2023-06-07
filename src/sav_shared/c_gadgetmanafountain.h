// (C)2006 S2 Games
// c_gadgetmanafountain.h
//
//=============================================================================
#ifndef __C_GADGETMANAFOUNTAIN_H__
#define __C_GADGETMANAFOUNTAIN_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gadgetusable.h"
//=============================================================================

//=============================================================================
// CGadgetManaFountain
//=============================================================================
class CGadgetManaFountain : public IGadgetUsable
{
private:
    START_ENTITY_CONFIG(IGadgetUsable)
        DECLARE_ENTITY_CVAR(float, ManaRefillPercent)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(Gadget, ManaFountain);

public:
    ~CGadgetManaFountain()  {}
    CGadgetManaFountain() :
    IGadgetUsable(GetEntityConfig()),
    m_pEntityConfig(GetEntityConfig())
    {}

    bool    UseEffect(IGameEntity *pActivator);
};
//=============================================================================

#endif //__C_GADGETMANAFOUNTAIN_H__
