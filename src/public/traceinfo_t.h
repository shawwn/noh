#ifndef __TRACEINFO_T__
#define __TRACEINFO_T__

#include "../shared/shared_types.h"

typedef struct traceinfo_s
{
    float   fraction;       //amount box / ray traveled before hitting something
    vec3_t  endpos;         //point where the box / ray stopped
    int     gridx, gridy;   //grid square where the box / ray stopped

    int     flags;          //surface flags
    int     objectType;     //STATIC_OBJECT or GAME_OBJECT
    int     index;          //if objectType==STATIC_OBJECT, the index to the internal worldObjects array.  if objectType==GAME_OBJECT, the index into the g_GameServer / g_HostClient.objects array

    vec3_t  normal;         //normal to the surface
    float   dist;           //surface plane dist
//  vec3_t  normal_b;       //normal to the other surface if we hit an edge

    int     collisionType;  //see COL_ defines above

    bool    startedInside;  //the trace started inside the surface
    bool    embedded;       //the trace is entirely inside the surface

    int     dbg_numsurfs;
}
traceinfo_t;

#endif // __TRACEINFO_T__