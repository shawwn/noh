// (C)2007 S2 Games
// i_officerspawnflag.h
//
//=============================================================================
#ifndef __I_OFFICERSPAWNFLAG_H__
#define __I_OFFICERSPAWNFLAG_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gadgetentity.h"
//=============================================================================

//=============================================================================
// IOfficerSpawnFlag
//=============================================================================
class IOfficerSpawnFlag : public IGadgetEntity
{
private:

public:
	~IOfficerSpawnFlag()	{}
	IOfficerSpawnFlag(CEntityConfig *pConfig) : IGadgetEntity(pConfig)	{}

	void	Spawn();
};
//=============================================================================

#endif //__I_OFFICERSPAWNFLAG_H__
