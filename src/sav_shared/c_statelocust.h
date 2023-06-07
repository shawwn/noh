// (C)2007 S2 Games
// c_statelocust.h
//
//=============================================================================
#ifndef __C_STATELOCUST_H__
#define __C_STATELOCUST_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateLocust
//=============================================================================
class CStateLocust : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, Locust);

public:
    ~CStateLocust() {}
    CStateLocust();
};
//=============================================================================

#endif //__C_STATELOCUST_H__
