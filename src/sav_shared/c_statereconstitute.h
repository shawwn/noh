// (C)2006 S2 Games
// c_statereconstitute.h
//
//=============================================================================
#ifndef __C_STATERECONSTITUTE_H__
#define __C_STATERECONSTITUTE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateReconstitute
//=============================================================================
class CStateReconstitute : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, HealthRegenMult)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, Reconstitute);

public:
    ~CStateReconstitute() {};
    CStateReconstitute();
};
//=============================================================================

#endif //__C_STATERECONSTITUTE_H__
