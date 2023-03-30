#ifndef __OCCLUDERLIST_T__
#define __OCCLUDERLIST_T__

#include "occluder_t.h"

typedef struct occluderlist_s
{
	occluder_t				occluder;
	bool					cull;
	struct occluderlist_s	*next;
} occluderlist_t;

#endif // __OCCLUDERLIST_T__