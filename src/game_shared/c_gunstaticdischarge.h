// (C)2007 S2 Games
// c_gunstaticdischarge.h
//
//=============================================================================
#ifndef __C_GUNSTATICDISCHARGE_H__
#define __C_GUNSTATICDISCHARGE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_beamgunitem.h"
//=============================================================================

//=============================================================================
// CGunStaticDischarge
//=============================================================================
class CGunStaticDischarge : public IBeamGunItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Gun, StaticDischarge);

public:
    ~CGunStaticDischarge()  {}
    CGunStaticDischarge() :
    IBeamGunItem(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_GUNSTATICDISCHARGE_H__
