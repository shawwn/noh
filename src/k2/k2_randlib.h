// (C)2010 S2 Games
// k2_randlib.h
//
//=============================================================================
#ifndef __K2_RANDLIB_H__
#define __K2_RANDLIB_H__

//=============================================================================
// Headers
//=============================================================================
#include <math.h>
#include "MersenneTwister.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
extern K2_API MersenneTwister*      gMersenneTwister;
//=============================================================================

/*====================
  K2_SRAND
  ====================*/
inline void     K2_SRAND(LONGLONG seed)
{
    assert(gMersenneTwister == nullptr);
    if (gMersenneTwister == nullptr)
        gMersenneTwister = K2_NEW(ctx_Singleton,  MersenneTwister)( uint(seed & UINT_MAX) );
}

/*====================
  K2_RAND
    generates a random value in [lo, hi]
  ====================*/
inline double       K2_RAND(double lo, double hi)
{
    assert(gMersenneTwister != nullptr);
    assert(lo < hi);
    return gMersenneTwister->genrand(lo, hi);
}

/*====================
  K2_RAND_NOT_INCLUSIVE
    generates a random value in [lo, hi)
  ====================*/
inline double       K2_RAND_NOT_INCLUSIVE(double lo, double hi)
{
    assert(gMersenneTwister != nullptr);
    return gMersenneTwister->genrand2(lo, hi);
}

/*====================
  K2_RAND_UINT32
    generates a random value in [0, 0xffffffff]
  ====================*/
inline uint         K2_RAND_UINT32()
{
    assert(gMersenneTwister != nullptr);
    return gMersenneTwister->genrand_uint32();
}

#endif
