// (C)2007 S2 Games
// c_meleesummonerstaff2.h
//
//=============================================================================
#ifndef __C_MELEESUMMONERSTAFF2_H__
#define __C_MELEESUMMONERSTAFF2_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_meleeitem.h"
//=============================================================================

//=============================================================================
// CMeleeSummonerStaff2
//=============================================================================
class CMeleeSummonerStaff2 : public IMeleeItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Melee, SummonerStaff2);

public:
	~CMeleeSummonerStaff2()	{}
	CMeleeSummonerStaff2() :
	IMeleeItem(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_MELEESUMMONERSTAFF2_H__
