// (C)2005 S2 Games
// s_traceinfo.h
//=============================================================================
#ifndef __S_TRACEINFO_H__
#define __S_TRACEINFO_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_vec3.h"
//=============================================================================

//these TRACE_* defines tell the trace function which surfaces to ignore
const uint  TRACE_PLAYER_MOVEMENT       (SURF_MODEL | SURF_PROJECTILE | SURF_FOLIAGE | SURF_NOT_SOLID | SURF_SHIELD | SURF_DEAD | SURF_CORPSE | SURF_ITEM);
const uint  TRACE_CAMERA                (SURF_MODEL | SURF_PLAYER | SURF_PROJECTILE | SURF_FOLIAGE | SURF_NOT_SOLID | SURF_SHIELD | SURF_DEAD | SURF_CORPSE | SURF_ITEM | SURF_GADGET | SURF_BLOCKER);
const uint  TRACE_PROJECTILE            (SURF_FOLIAGE | SURF_NOT_SOLID | SURF_DEAD | SURF_CORPSE | SURF_HULL | SURF_INTANGIBLE | SURF_ITEM | SURF_BLOCKER);
const uint  TRACE_SNAPCAST              (SURF_MODEL | SURF_FOLIAGE | SURF_NOT_SOLID | SURF_INTANGIBLE | SURF_ITEM | SURF_BLOCKER | SURF_SHIELD);
const uint  TRACE_COMMANDER_SNAPCAST    (SURF_HULL | SURF_FOLIAGE | SURF_NOT_SOLID | SURF_INTANGIBLE | SURF_ITEM | SURF_BLOCKER | SURF_SHIELD | SURF_PROP);
const uint  TRACE_COMMANDER_TRANSPARENT (SURF_IGNORE | SURF_BLOCKER | SURF_PROP | SURF_FOLIAGE | SURF_NOT_SOLID | SURF_INTANGIBLE | SURF_SHIELD | SURF_TERRAIN);
const uint  TRACE_TERRAIN               (SURF_STATIC | SURF_DYNAMIC | SURF_BLOCKER | SURF_RENDER);
const uint  TRACE_NOCLIP                (0xffffffff);
const uint  TRACE_ALL_SURFACES          (0);

//=============================================================================
// STraceInfo
//=============================================================================
struct STraceInfo
{
    float       fFraction;      // amount box / ray traveled before hitting something
    CVec3f      v3EndPos;       // point where the box / ray stopped
    int         iGridX, iGridY; // grid square where the box / ray stopped
 
    uint        uiSurfFlags;    // surface flags
    uint        uiEntityIndex;
    uint        uiSurfaceIndex;

    CPlane      plPlane;        // impact plane

    bool        bStartedInSurface;  // the trace started inside the surface
    bool        bEmbedded;      // the trace is entirely inside the surface
    bool        bHit;
};
//=============================================================================

#endif //__S_TRACEINFO_H__
