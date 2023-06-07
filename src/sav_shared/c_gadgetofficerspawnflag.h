// (C)2007 S2 Games
// c_gadgetofficerspawnflag.h
//
//=============================================================================
#ifndef __C_GADGETOFFICERSPAWNFLAG_H__
#define __C_GADGETOFFICERSPAWNFLAG_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gadgetentity.h"
//=============================================================================

//=============================================================================
// CGadgetOfficerSpawnFlag
//=============================================================================
class CGadgetOfficerSpawnFlag : public IGadgetEntity
{
private:
    DECLARE_ENT_ALLOCATOR(Gadget, OfficerSpawnFlag);
    static CEntityConfig    s_EntityConfig;

public:
    ~CGadgetOfficerSpawnFlag()  {}
    CGadgetOfficerSpawnFlag();

    void    Spawn();
    void    Kill(IGameEntity *pAttacker = NULL);
};
//=============================================================================

#endif __C_GADGETOFFICERSPAWNFLAG_H__
