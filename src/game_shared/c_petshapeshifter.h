// (C)2007 S2 Games
// c_petshapeshifter.h
//
//=============================================================================
#ifndef __C_PETSHAPESHIFTER_H__
#define __C_PETSHAPESHIFTER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_petentity.h"
//=============================================================================

//=============================================================================
// CPetShapeShifter
//=============================================================================
class CPetShapeShifter : public IPetEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Pet, ShapeShifter);

public:
	~CPetShapeShifter()	{}
	CPetShapeShifter() : IPetEntity(GetEntityConfig()) {}
};
//=============================================================================

#endif //__C_PETSHAPESHIFTER_H__
