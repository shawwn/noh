// (C)2007 S2 Games
// c_statechlorophilic.h
//
//=============================================================================
#ifndef __C_STATECHLOROPHILIC_H__
#define __C_STATECHLOROPHILIC_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateChlorophilic
//=============================================================================
class CStateChlorophilic : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, HealthPerSecond)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, Chlorophilic);



public:
    ~CStateChlorophilic()   {}
    CStateChlorophilic();

    void    StateFrame();
    void    Activated();

};
//=============================================================================

#endif //__C_STATECHLOROPHILIC_H__
