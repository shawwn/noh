// (C)2005 S2 Games
// c_convexhull.h
//=============================================================================
#ifndef __C_CONVEXHULL_H__
#define __C_CONVEXHULL_H__

//=============================================================================
// Headers
//=============================================================================
#include <math.h>
#include <float.h>
#include "k2_api.h"
#include "c_plane.h"
#include "c_vec3.h"
#include "c_boundingbox.h"
//=============================================================================

//=============================================================================
// CConvexHull
//=============================================================================
class CConvexHull
{
    vector<CPlane>  m_vPlanes;

    bool    Contains(const CVec3f &vPoint, vector<CPlane>::const_iterator it1, vector<CPlane>::const_iterator it2, vector<CPlane>::const_iterator it3);

public:
    K2_API  CConvexHull();
    K2_API  CConvexHull(const vector<CPlane> &vPlanes);
    K2_API  CConvexHull(const CBBoxf &bbox);

    K2_API  void    AddPlanes(const vector<CPlane> &vPlanes);
    K2_API  void    AddPlane(const CPlane &plPlane);

    K2_API  bool    Contains(const CVec3f &vPoint);

    const vector<CPlane>&   GetPlanes() { return m_vPlanes; }
    K2_API  void        GetPoints(vector<CVec3f> &vPoints);
};
//=============================================================================
#endif // __C_CONVEXHULL_H__
