// (C)2007 S2 Games
// c_meleeworkerclaws.h
//
//=============================================================================
#ifndef __C_MELEEWORKERCLAWS_H__
#define __C_MELEEWORKERCLAWS_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_meleeitem.h"
//=============================================================================

//=============================================================================
// CMeleeWorkerClaws
//=============================================================================
class CMeleeWorkerClaws : public IMeleeItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Melee, WorkerClaws);

public:
	~CMeleeWorkerClaws()	{}
	CMeleeWorkerClaws() :
	IMeleeItem(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_MELEEWORKERCLAWS_H__
