// (C)2006 S2 Games
// c_meleeworkerhammer.h
//
//=============================================================================
#ifndef __C_MELEEWORKERHAMMER_H__
#define __C_MELEEWORKERHAMMER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_meleeitem.h"
//=============================================================================

//=============================================================================
// CMeleeWorkerHammer
//=============================================================================
class CMeleeWorkerHammer : public IMeleeItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Melee, WorkerHammer);

public:
    ~CMeleeWorkerHammer()   {}
    CMeleeWorkerHammer() :
    IMeleeItem(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_MELEEWORKERHAMMER_H__
