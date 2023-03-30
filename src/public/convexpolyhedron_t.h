// (C)2005 S2 Games
// complexpolyhedron_t.h
//
//=============================================================================
#ifndef __CONVEXHEDRON_T__
#define __CONVEXHEDRON_T__

#define MAX_POLYHEDRON_PLANES   128

struct SConvexPolyhedron
{
    int     numPlanes;
    plane_t planes[MAX_POLYHEDRON_PLANES + 6];  //the 6 extra planes are for the beveling step

    vec3_t  bmin;       //must be computed beforehand
    vec3_t  bmax;       //must be computed beforehand

    int     flags;

    //trimesh representation (for drawing and debugging)
    int     num_verts;
    vec3_t  *verts;
    int     numFaces;
    uivec3_t *faceList; 
};

#endif // __CONVEXHEDRON_T__
