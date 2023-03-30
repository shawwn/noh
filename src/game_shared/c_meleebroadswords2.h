// (C)2006 S2 Games
// c_meleebroadswords2.h
//
//=============================================================================
#ifndef __C_MELEEBROADSWORDS2_H__
#define __C_MELEEBROADSWORDS2_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_meleeitem.h"
//=============================================================================

//=============================================================================
// CMeleeBroadswords
//=============================================================================
class CMeleeBroadswords2 : public IMeleeItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Melee, Broadswords2);

public:
	~CMeleeBroadswords2()	{}
	CMeleeBroadswords2() :
	IMeleeItem(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_MELEEBROADSWORDS2_H__
