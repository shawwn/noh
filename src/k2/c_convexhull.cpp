// (C)2005 S2 Games
// c_convexhull.cpp
//
// Convex hulls are the 3d space enclosed by a set of planes
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_convexhull.h"
#include "c_boundingbox.h"
#include "c_matrix3x3.h"
//=============================================================================

CVAR_FLOAT(hull_epsilon, 0.004f);
CVAR_FLOAT(hull_denom, 0.0001f);

/*====================
  CConvexHull::CConvexHull
  ====================*/
CConvexHull::CConvexHull()
{
}


/*====================
  CConvexHull::CConvexHull
  ====================*/
CConvexHull::CConvexHull(const vector<CPlane> &vPlanes) :
m_vPlanes(vPlanes)
{
}


/*====================
  CConvexHull::CConvexHull
  ====================*/
CConvexHull::CConvexHull(const CBBoxf &bbox)
{
    m_vPlanes.reserve(6);
    m_vPlanes.push_back(CPlane(-1.0f,  0.0f,  0.0f, -bbox.GetMin()[X]));
    m_vPlanes.push_back(CPlane( 1.0f,  0.0f,  0.0f,  bbox.GetMax()[X]));
    m_vPlanes.push_back(CPlane( 0.0f, -1.0f,  0.0f, -bbox.GetMin()[Y]));
    m_vPlanes.push_back(CPlane( 0.0f,  1.0f,  0.0f,  bbox.GetMax()[Y]));
    m_vPlanes.push_back(CPlane( 0.0f,  0.0f, -1.0f, -bbox.GetMin()[Z]));
    m_vPlanes.push_back(CPlane( 0.0f,  0.0f,  1.0f,  bbox.GetMax()[Z]));
}


/*====================
  CConvexHull::AddPlanes
  ====================*/
void    CConvexHull::AddPlanes(const vector<CPlane> &vPlanes)
{
    m_vPlanes.reserve(m_vPlanes.size() + vPlanes.size());
    for (vector<CPlane>::const_iterator it = vPlanes.begin(); it != vPlanes.end(); ++it)
        m_vPlanes.push_back(*it);
}


/*====================
  CConvexHull::AddPlane
  ====================*/
void    CConvexHull::AddPlane(const CPlane &plPlane)
{
    m_vPlanes.push_back(plPlane);
}


/*====================
  CConvexHull::GetPoints

  WARNING: This is O(n^3) so use sparingly
  ====================*/
void    CConvexHull::GetPoints(vector<CVec3f> &vPoints)
{
    if (m_vPlanes.size() < 3) // can't have any points with less that 3 planes
        return;
    
    vector<CPlane>::const_iterator itEnd(m_vPlanes.end());
    for (vector<CPlane>::const_iterator it1(m_vPlanes.begin()); it1 != itEnd; ++it1)
    {
        for (vector<CPlane>::const_iterator it2(it1 + 1); it2 != itEnd; ++it2)
        {
            for (vector<CPlane>::const_iterator it3(it2 + 1); it3 != itEnd; ++it3)
            {
                // Find the intersection point of the planes it1, it2, and it3
#if 0
                CMatrix3x3f M(it1->normal.x, it1->normal.y, it1->normal.z,
                              it2->normal.x, it2->normal.y, it2->normal.z,
                              it3->normal.x, it3->normal.y, it3->normal.z);

                CVec3f      D(it1->dist, it2->dist, it3->dist);

                if (M.Invert())
                {
                    CVec3f v = Multiply(M, D);

                    if (ABS(it1->Distance(v)) > hull_epsilon || ABS(it2->Distance(v)) > hull_epsilon || ABS(it3->Distance(v)) > hull_epsilon)
                    {
                        float s1 = it1->Distance(v);
                        float s2 = it2->Distance(v);
                        float s3 = it3->Distance(v);
                        //__asm int 0x03;
                    }

                    if (Contains(v, it1, it2, it3))
                        vPoints.push_back(v);
                }
#else
                CVec3f v3cp23(CrossProduct(it2->v3Normal, it3->v3Normal));
                float fDenom(DotProduct(it1->v3Normal, v3cp23));

                if (ABS(fDenom) > hull_denom)
                {
                    CVec3f v((v3cp23 * it1->fDist +
                        CrossProduct(it3->v3Normal, it1->v3Normal) * it2->fDist +
                        CrossProduct(it1->v3Normal, it2->v3Normal) * it3->fDist) / fDenom);

                    if (Contains(v, it1, it2, it3))
                        vPoints.push_back(v);
                }
#endif
            }
        }
    }
}


/*====================
  CConvexHull::Contains
  ====================*/
bool    CConvexHull::Contains(const CVec3f &vPoint)
{
    for (vector<CPlane>::const_iterator it = m_vPlanes.begin(); it != m_vPlanes.end(); ++it)
    {
        if (it->Distance(vPoint) > hull_epsilon)
            return false;
    }

    return true;
}


/*====================
  CConvexHull::Contains

  Ignores the parent planes of the point
  ====================*/
bool    CConvexHull::Contains(const CVec3f &vPoint,
                              vector<CPlane>::const_iterator it1,
                              vector<CPlane>::const_iterator it2,
                              vector<CPlane>::const_iterator it3)
{
    for (vector<CPlane>::const_iterator it = m_vPlanes.begin(); it != m_vPlanes.end(); ++it)
    {
        if (it == it1 || it == it2 || it == it3)
            continue;

        if (it->Distance(vPoint) > hull_epsilon)
            return false;
    }

    return true;
}
