// (C)2005 S2 Games
// intersection.h
//
// geometry intersection routines
//=============================================================================
#ifndef __INTERSECTION_H__
#define __INTERSECTION_H__

#include "c_vec3.h"
#include "c_boundingbox.h"

typedef struct obb_s
{
	vec3_t	center;
	vec3_t	axis[3];
	float	extents[3];
}
obb_t;

class CSphere;
class CConvexPolyhedron;
class CPlane2;

K2_API bool	I_BoxTriIntersect(const vec3_t apkTri[3], const vec3_t bPos, const vec3_t bext,
						   const vec3_t rkBoxVel, float fTMax, float *rfTFirst, float *rfTLast);

K2_API bool	I_LineMeshIntersect(const CVec3f &start, const CVec3f &end, const class CMesh *mesh,
							float &fFraction, int &face_hit);

K2_API bool	I_LineBoundsIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CBBoxf &bbBounds, float &fFraction, CPlane *pPlaneHit = NULL, bool *pbStartInSurface = NULL);

K2_API bool	I_IntersectBoxWithSurface(const CVec3f &start, const CVec3f &end, struct linkedSurface_s *surf, float &fraction);

K2_API bool	I_RayTriIntersect(const CVec3f &orig, const CVec3f &dir, const CVec3f &vert0, const CVec3f &vert1, const CVec3f &vert2, float &fDistance);

K2_API bool I_RayTriIntersect(const vec3_t orig, const vec3_t dir, vec3_t vert0, vec3_t vert1, vec3_t vert2, float *fraction);

K2_API bool I_SphereBoundsIntersect(const CSphere &sSphere, const CBBoxf &bbBox);

K2_API bool	I_CircleRectIntersect(const CVec2f &v2Center, float fRadius, const CRectf &rect);

K2_API bool	I_LineTriangleIntersect(const CVec3f &start, const CVec3f &end, const CVec3f &v1, const CVec3f &v2, const CVec3f &v3, float &fFraction);

K2_API bool	I_LineQuadIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CVec3f &v1, const CVec3f &v2, const CVec3f &v3, const CVec3f &v4, float &fFraction);

K2_API bool	I_LineSurfaceIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CConvexPolyhedron &surface, float &fFraction, CPlane *pPlaneHit = NULL, bool *pbStartInSurface = NULL);

K2_API bool	I_LineTerrainIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CPlane aPlanes[], float &fFraction, CPlane *pPlaneHit = NULL, bool *pbStartInSurface = NULL);

K2_API bool	I_MovingBoundsBoundsIntersect(const CVec3f &v3Start, const CVec3f &v3Delta, const CBBoxf &bbBoundsA, const CBBoxf &bbBoundsB, float &fFraction, CPlane *pPlaneHit = NULL, bool *pbStartInSurface = NULL);

K2_API bool	I_MovingBoundsSurfaceIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CBBoxf &bbBounds, const CConvexPolyhedron &surface, float &fFraction, CPlane *pPlaneHit = NULL, bool *pbStartInSurface = NULL);

K2_API bool	I_MovingBoundsTerrainIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CBBoxf &bbBounds, const CPlane aPlanes[], const CVec3f &v1, const CVec3f &v2, const CVec3f &v3, float &fFraction, CPlane *pPlaneHit = NULL, bool *pbStartInSurface = NULL);

K2_API bool	I_LinePlaneIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CPlane &plane, float &fFraction);

K2_API bool	I_LineDoubleSidedPlaneIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CPlane &plane, float &fFraction);

K2_API bool	I_LineDoubleSidedPlaneIntersect(const CVec2f &v2Start, const CVec2f &v2End, const CPlane2 &plane, float &fFraction);

K2_API bool	I_BoundsSurfaceIntersect(const CBBoxf &bbBounds, const CConvexPolyhedron &surface);

K2_API bool	I_SurfaceSurfaceIntersect(const CConvexPolyhedron &cSurface1, const CConvexPolyhedron &cSurface2);

K2_API bool	I_LineBoundsOverlap(const CVec3f &v3Start, const CVec3f &v3End, const CBBoxf &bbBounds, float &fEnter, float &fExit);

K2_API bool	I_MovingBoundsBoundsOverlap(const CVec3f &v3Start, const CVec3f &v3End, const CBBoxf &bbBoundsA, const CBBoxf &bbBoundsB, float &fEnter, float &fExit);

K2_API bool	I_LineBlockerIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CPlane aPlanes[], float &fFraction, CPlane *pPlaneHit = NULL, bool *pbStartInSurface = NULL);

K2_API bool	I_MovingBoundsBlockerIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CBBoxf &bbBounds, const CVec3f &v3Diag, const CVec3f &v1, const CVec3f &v2, const CVec3f &v3, float &fFraction, CPlane *pPlaneHit = NULL, bool *pbStartInSurface = NULL);
//=============================================================================
#endif //__INTERSECTION_H__
