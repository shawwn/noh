// (C)2007 S2 Games
// c_gadgetbeastspawnportal.h
//
//=============================================================================
#ifndef __C_GADGETBEASTSPAWNPORTAL_H__
#define __C_GADGETBEASTSPAWNPORTAL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_officerspawnflag.h"
//=============================================================================

//=============================================================================
// CGadgetBeastSpawnPortal
//=============================================================================
class CGadgetBeastSpawnPortal : public IOfficerSpawnFlag
{
private:
    DECLARE_ENT_ALLOCATOR2(Gadget, BeastSpawnPortal);

public:
    ~CGadgetBeastSpawnPortal() {}
    CGadgetBeastSpawnPortal() : IOfficerSpawnFlag(GetEntityConfig()) {}
};
//=============================================================================

#endif //__C_GADGETBEASTSPAWNPORTAL_H__
