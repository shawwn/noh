#ifndef __POINTINFO_T__
#define __POINTINFO_T__

#include "../shared/shared_types.h"

typedef struct pointinfo_s
{
    float   z;              //height of terrain at this point
    vec3_t  nml;            //surface normal
}
pointinfo_t;

#endif // __POINTINFO_T__
