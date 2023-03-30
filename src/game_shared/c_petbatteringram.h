// (C)2007 S2 Games
// c_petBatteringram.h
//
//=============================================================================
#ifndef __C_PETBATTERINGRAM_H__
#define __C_PETBATTERINGRAM_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_petentity.h"
//=============================================================================

//=============================================================================
// CPetBatteringRam
//=============================================================================
class CPetBatteringRam : public IPetEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Pet, BatteringRam);

public:
    ~CPetBatteringRam() {}
    CPetBatteringRam() : IPetEntity(GetEntityConfig()) {}
};
//=============================================================================

#endif //__C_PETBATTERINGRAM_H__
