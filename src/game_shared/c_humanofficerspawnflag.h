// (C)2007 S2 Games
// c_humanofficerspawnflag.h
//
//=============================================================================
#ifndef __C_HUMANOFFICERSPAWNFLAG_H__
#define __C_HUMANOFFICERSPAWNFLAG_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_officerspawnflag.h"
//=============================================================================

//=============================================================================
// CGadgetHumanOfficerSpawnFlag
//=============================================================================
class CGadgetHumanOfficerSpawnFlag : public IOfficerSpawnFlag
{
private:
	DECLARE_ENT_ALLOCATOR2(Gadget, HumanOfficerSpawnFlag);

public:
	~CGadgetHumanOfficerSpawnFlag() {}
	CGadgetHumanOfficerSpawnFlag() : IOfficerSpawnFlag(GetEntityConfig()) {}
};
//=============================================================================

#endif //__C_HUMANOFFICERSPAWNFLAG_H__
