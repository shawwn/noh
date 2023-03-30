// (C)2006 S2 Games
// c_statelynxfeet.h
//
//=============================================================================
#ifndef __C_STATELYNXFEET_H__
#define __C_STATELYNXFEET_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateLynxFeet
//=============================================================================
class CStateLynxFeet : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, SpeedBoost)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, LynxFeet);

public:
    ~CStateLynxFeet()   {}
    CStateLynxFeet();
};
//=============================================================================

#endif //__C_STATELYNXFEET_H__
