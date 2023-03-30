// (C)2007 S2 Games
// c_meleeconjurerclaws.h
//
//=============================================================================
#ifndef __C_MELEECONJURERCLAWS_H__
#define __C_MELEECONJURERCLAWS_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_meleeitem.h"
//=============================================================================

//=============================================================================
// CMeleeConjurerClaws
//=============================================================================
class CMeleeConjurerClaws : public IMeleeItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Melee, ConjurerClaws);

public:
	~CMeleeConjurerClaws()	{}
	CMeleeConjurerClaws() :
	IMeleeItem(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_MELEECONJURERCLAWS_H__
