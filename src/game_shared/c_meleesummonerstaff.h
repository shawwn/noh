// (C)2007 S2 Games
// c_meleesummonerstaff.h
//
//=============================================================================
#ifndef __C_MELEESUMMONERSTAFF_H__
#define __C_MELEESUMMONERSTAFF_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_meleeitem.h"
//=============================================================================

//=============================================================================
// CMeleeSummonerStaff
//=============================================================================
class CMeleeSummonerStaff : public IMeleeItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Melee, SummonerStaff);

public:
	~CMeleeSummonerStaff()	{}
	CMeleeSummonerStaff() :
	IMeleeItem(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_MELEESUMMONERSTAFF_H__
