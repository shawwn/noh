#ifndef __OCCLUDER_H__
#define __OCCLUDER_H__

#define _MAX_OCCLUDER_POINTS 16

typedef struct occluder_s
{
    int     numpoints;
    CVec3f  points[_MAX_OCCLUDER_POINTS];
}
occluder_t;

#endif // __OCCLUDER_H__
