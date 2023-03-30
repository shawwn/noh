#ifndef __SCENELIGHTLIST_T__
#define __SCENELIGHTLIST_T__

typedef struct scenelightlist_s
{
	SSceneLight				light;
	bool					cull;
	struct scenelightlist_s *next;
} scenelightlist_t;

#endif // __SCENELIGHTLIST_T__