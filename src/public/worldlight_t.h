#ifndef __WORLDLIGHT_T__
#define __WORLDLIGHT_T__

#include "../shared/c_cvar.h"

const int _MAX_WORLD_LIGHTS = 4;

typedef struct
{
	CCvar<tstring>	*type;
	CCvar<float>	*x;
	CCvar<float>	*y;
	CCvar<float>	*z;
	CCvar<float>	*r;
	CCvar<float>	*g;
	CCvar<float>	*b;
} worldLight_t;

#endif // __WORLDLIGHT_T__
