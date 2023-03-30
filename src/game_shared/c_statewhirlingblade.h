// (C)2006 S2 Games
// c_statewhirlingblade.h
//
//=============================================================================
#ifndef __C_STATEWHIRLOINGBLADE_H__
#define __C_STATEWHIRLOINGBLADE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateWhirlingBlade
//=============================================================================
class CStateWhirlingBlade : public IEntityState
{
private:
    DECLARE_STATE_ALLOCATOR(WhirlingBlade);
    DECLARE_ENTITY_STATE_CVARS;

public:
    ~CStateWhirlingBlade();
    CStateWhirlingBlade();

    void    DoAttack(CMeleeAttackEvent &attack);
};
//=============================================================================

#endif //__C_STATEWHIRLOINGBLADE_H__
