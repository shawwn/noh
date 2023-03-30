#ifndef __SCENELIST_T__
#define __SCENELIST_T__

#include "../shared/c_sceneentity.h"

//scenelist, scenefacelist, and scenelightlist are needed by gl_scene, so we expose them here
typedef struct scenelist_s
{
	SSceneObject		obj;	//must be the first field in this struct!
	bool				cull;  //if true, we do not draw this object
	bool				cull_shadow; //if true, we do not draw this object into the shadowmap
	struct scenelist_s	*next;
} scenelist_t;

#endif // __SCENELIST_T__