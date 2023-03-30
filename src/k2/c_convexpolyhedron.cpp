// (C)2005 S2 Games
// c_convexpolyhedron.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_convexpolyhedron.h"
#include "c_plane.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
//=============================================================================


/*====================
  CConvexPolyhedron::CConvexPolyhedron

  Constructor for generating a surface from an AABB
  ====================*/
CConvexPolyhedron::CConvexPolyhedron(const CBBoxf &bb) :
m_bbBounds(bb),
m_vPlanes(6),
m_vEdges(0),
m_vPoints(8)
{
    const CVec3f &v3Min(bb.GetMin());
    const CVec3f &v3Max(bb.GetMax());

    // Planes
    m_vPlanes[0] = CPlane(-1.0f,  0.0f,  0.0f, -v3Min.x);
    m_vPlanes[1] = CPlane(1.0f,  0.0f,  0.0f, v3Max.x);
    m_vPlanes[2] = CPlane(0.0f, -1.0f,  0.0f, -v3Min.y);
    m_vPlanes[3] = CPlane(0.0f,  1.0f,  0.0f, v3Max.y);
    m_vPlanes[4] = CPlane(0.0f,  0.0f, -1.0f, -v3Min.z);
    m_vPlanes[5] = CPlane(0.0f,  0.0f,  1.0f, v3Max.z);

    // Points
    for (int i(0); i < 8; ++i)
    {
        m_vPoints[i] = CVec3f
        (
            (i & BIT(0)) ? v3Min.x : v3Max.x,
            (i & BIT(1)) ? v3Min.y : v3Max.y,
            (i & BIT(2)) ? v3Min.z : v3Max.z
        );
    }

    // Box axes (x, y, z)
    AddAxis(CVec3f(1.0f, 0.0f, 0.0f));
    AddAxis(CVec3f(0.0f, 1.0f, 0.0f));
    AddAxis(CVec3f(0.0f, 0.0f, 1.0f));
}


/*====================
  CConvexPolyhedron::CConvexPolyhedron

  Constructor for generating a surface for a terrain tile triangle
  ====================*/
CConvexPolyhedron::CConvexPolyhedron(const CVec3f &v1, const CVec3f &v2, const CVec3f &v3) :
m_vPlanes(4),
m_vEdges(3),
m_vPoints(3)
{
    m_vExtents.reserve(5);

    // Planes
    CVec3f v3Up(0.0f, 0.0f, 1.0f);

    m_vPlanes[0] = CPlane(v1, v2, v3, true);
    m_vPlanes[1] = CPlane(v2, v1, v1 - v3Up, true);
    m_vPlanes[2] = CPlane(v3, v2, v2 - v3Up, true);
    m_vPlanes[3] = CPlane(v1, v3, v3 - v3Up, true);

    // Points
    m_vPoints[0] = v1;
    m_vPoints[1] = v2;
    m_vPoints[2] = v3;

    // Edges
    m_vEdges[0] = CEdge(v1, v2);
    m_vEdges[1] = CEdge(v2, v3);
    m_vEdges[2] = CEdge(v3, v1);

    m_bbBounds.AddPoint(m_vPoints[0]);
    m_bbBounds.AddPoint(m_vPoints[1]);
    m_bbBounds.AddPoint(m_vPoints[2]);

    CalcExtents();
}


/*====================
  CConvexPolyhedron::CConvexPolyhedron

  Constructor for generating a surface for a frustum
  ====================*/
CConvexPolyhedron::CConvexPolyhedron(const CVec3f &v3Origin, const CAxis &aAxis, float fFovX, float fFovY, float fFar) :
m_vPlanes(5),
m_vEdges(8),
m_vPoints(5)
{
    PROFILE("CConvexPolyhedron::CConvexPolyhedron");

    const CVec3f &v3ViewRight(aAxis[RIGHT]);
    const CVec3f &v3ViewDir(aAxis[FORWARD]);
    const CVec3f &v3ViewUp(aAxis[UP]);
    const CVec3f &v3Eye(v3Origin);

    fFovX = DEG2RAD(fFovX);
    fFovY = DEG2RAD(fFovY);

    float fTanX = tan(fFovX * 0.5f);
    float fTanY = tan(fFovY * 0.5f);

    float wl = -fTanX;
    float wr = fTanX;
    float wb = -fTanY;
    float wt = fTanY;

    CVec3f v3Offset(v3Eye + v3ViewDir);

    CVec3f av3BoundingPoints[4];
    av3BoundingPoints[0] = v3ViewRight * wr + v3ViewUp * wt + v3Offset; // top - right point
    av3BoundingPoints[1] = v3ViewRight * wl + v3ViewUp * wt + v3Offset; // top - left point
    av3BoundingPoints[2] = v3ViewRight * wl + v3ViewUp * wb + v3Offset; // bottom - left point
    av3BoundingPoints[3] = v3ViewRight * wr + v3ViewUp * wb + v3Offset; // bottom - right point

    m_vPlanes[0].CalcPlaneNormalized(av3BoundingPoints[0], av3BoundingPoints[1], v3Eye); // top
    m_vPlanes[1].CalcPlaneNormalized(av3BoundingPoints[1], av3BoundingPoints[2], v3Eye); // left
    m_vPlanes[2].CalcPlaneNormalized(av3BoundingPoints[2], av3BoundingPoints[3], v3Eye); // bottom
    m_vPlanes[3].CalcPlaneNormalized(av3BoundingPoints[3], av3BoundingPoints[0], v3Eye); // right

    // Far plane
    m_vPlanes[4].v3Normal = v3ViewDir;
    m_vPlanes[4].fDist = DotProduct(v3ViewDir, M_PointOnLine(v3Eye, v3ViewDir, fFar));

    CVec3f av3Edges[4];
    av3Edges[0] = Normalize(av3BoundingPoints[0] - v3Eye);
    av3Edges[1] = Normalize(av3BoundingPoints[1] - v3Eye);
    av3Edges[2] = Normalize(av3BoundingPoints[2] - v3Eye);
    av3Edges[3] = Normalize(av3BoundingPoints[3] - v3Eye);

    // Points
    m_vPoints[0] = v3Origin;
    m_vPoints[1] = M_PointOnLine(v3Eye, av3Edges[0], fFar / DotProduct(av3Edges[0], v3ViewDir));
    m_vPoints[2] = M_PointOnLine(v3Eye, av3Edges[1], fFar / DotProduct(av3Edges[1], v3ViewDir));
    m_vPoints[3] = M_PointOnLine(v3Eye, av3Edges[2], fFar / DotProduct(av3Edges[2], v3ViewDir));
    m_vPoints[4] = M_PointOnLine(v3Eye, av3Edges[3], fFar / DotProduct(av3Edges[3], v3ViewDir));

    // Edges
    m_vEdges[0] = CEdge(m_vPoints[0], m_vPoints[1]);
    m_vEdges[1] = CEdge(m_vPoints[0], m_vPoints[2]);
    m_vEdges[2] = CEdge(m_vPoints[0], m_vPoints[3]);
    m_vEdges[3] = CEdge(m_vPoints[0], m_vPoints[4]);
    m_vEdges[4] = CEdge(m_vPoints[1], m_vPoints[2]);
    m_vEdges[5] = CEdge(m_vPoints[2], m_vPoints[3]);
    m_vEdges[6] = CEdge(m_vPoints[3], m_vPoints[4]);
    m_vEdges[7] = CEdge(m_vPoints[4], m_vPoints[1]);

    vector<CVec3f>::iterator itEnd(m_vPoints.end());
    for (vector<CVec3f>::iterator it(m_vPoints.begin()); it != itEnd; ++it)
        m_bbBounds.AddPoint(*it);

    CalcExtents();
}


/*====================
  CConvexPolyhedron::AddPlane
  ====================*/
void    CConvexPolyhedron::AddPlane(const CPlane &plane)
{
    vector<CPlane>::iterator itEnd(m_vPlanes.end());
    for (vector<CPlane>::iterator it(m_vPlanes.begin()); it != itEnd; ++it)
    {
        if (1.0f - DotProduct(plane.v3Normal, it->v3Normal) < 0.01f && plane.fDist <= it->fDist)
            return;
    }

    m_vPlanes.push_back(plane);
}


/*====================
  CConvexPolyhedron::Transform
  ====================*/
void    CConvexPolyhedron::Transform(const CVec3f &v3Pos, const CAxis &axis, float fScale)
{
    for (vector<CPlane>::iterator itPlane(m_vPlanes.begin()); itPlane != m_vPlanes.end(); ++itPlane)
        itPlane->Transform(v3Pos, axis, fScale);
    
    m_bbBounds.Clear();
    for (vector<CVec3f>::iterator itPoint(m_vPoints.begin()); itPoint != m_vPoints.end(); ++itPoint)
    {
        *itPoint = TransformPoint(*itPoint, axis, v3Pos, fScale);
        m_bbBounds.AddPoint(*itPoint);
    }
    
    for (vector<CEdge>::iterator itEdge(m_vEdges.begin()); itEdge != m_vEdges.end(); ++itEdge)
        itEdge->Transform(axis, v3Pos, fScale);

    CalcExtents();
}


/*====================
  CConvexPolyhedron::GetTransformed
  ====================*/
CConvexPolyhedron   CConvexPolyhedron::GetTransformed(const CVec3f &v3Pos, const CAxis &axis, float fScale)
{
    CConvexPolyhedron   polyOut(*this);
    polyOut.Transform(v3Pos, axis, fScale);
    return polyOut;
}


/*====================
  CConvexPolyhedron::AddPoint
  ====================*/
uint    CConvexPolyhedron::AddPoint(const CVec3f &v3Point)
{
    for (vector<CVec3f>::iterator it(m_vPoints.begin()); it != m_vPoints.end(); ++it)
    {
        if (v3Point == *it)
            return uint(it - m_vPoints.begin());
    }

    m_vPoints.push_back(v3Point);
    return uint(m_vPoints.size() - 1);
}


/*====================
  CConvexPolyhedron::AddEdge
  ====================*/
void    CConvexPolyhedron::AddEdge(const CEdge &cEdge)
{
    for (vector<CEdge>::iterator it(m_vEdges.begin()); it != m_vEdges.end(); ++it)
    {
        if (M_LinePointDistance(it->v3Point, it->v3Normal, cEdge.v3Point) < 0.01f &&
            1.0f - DotProduct(it->v3Normal, cEdge.v3Normal) < 0.01f)
            return;
    }

    m_vEdges.push_back(cEdge);
}


/*====================
  CConvexPolyhedron::AddTri
  ====================*/
void    CConvexPolyhedron::AddTri(uint i0, uint i1, uint i2)
{
    m_vTriList.push_back(i0);
    m_vTriList.push_back(i1);
    m_vTriList.push_back(i2);
}


/*====================
  CConvexPolyhedron::AddAxis
  ====================*/
bool    CConvexPolyhedron::AddAxis(const CVec3f &v3Axis)
{
    SExtents sExtents;

    if (fabs(1.0f - v3Axis.LengthSq()) > 0.1f) // Check for malformed normal
        return false;

    // Skip duplicated axes
    vector<SExtents>::iterator itFind(m_vExtents.begin()), itEnd(m_vExtents.end());
    for (; itFind != itEnd; ++itFind)
    {
        if (itFind->v3Axis == v3Axis || itFind->v3Axis == -v3Axis)
            break;
    }

    if (itFind != itEnd)
        return false;

    sExtents.v3Axis = v3Axis;
    M_CalcAxisExtents(sExtents.v3Axis, m_vPoints, sExtents.fMin, sExtents.fMax);
    m_vExtents.push_back(sExtents);
    return true;
}


/*====================
  CConvexPolyhedron::CalcExtents

  Calculate extents for moving bounds collisions
  ====================*/
void    CConvexPolyhedron::CalcExtents()
{
    m_vExtents.clear();

    // Box axes (x, y, z)
    AddAxis(CVec3f(1.0f, 0.0f, 0.0f));
    AddAxis(CVec3f(0.0f, 1.0f, 0.0f));
    AddAxis(CVec3f(0.0f, 0.0f, 1.0f));

    // The planes of this polyhedron
    vector<CPlane>::iterator itEnd(m_vPlanes.end());
    for (vector<CPlane>::iterator it(m_vPlanes.begin()); it != itEnd; ++it)
        AddAxis(it->v3Normal);

    // Edge cross-product planes
    CVec3f  vAABBEdges[3];
    vAABBEdges[0] = CVec3f(1.0f, 0.0f, 0.0f);
    vAABBEdges[1] = CVec3f(0.0f, 1.0f, 0.0f);
    vAABBEdges[2] = CVec3f(0.0f, 0.0f, 1.0f);

    for (uint iAABBEdge(0); iAABBEdge < 3; ++iAABBEdge)
    {
        const CVec3f &v3B(vAABBEdges[iAABBEdge]);

        for (uint iSurfaceEdge(0); iSurfaceEdge < m_vEdges.size(); ++iSurfaceEdge)
        {
            const CVec3f &v3A(m_vEdges[iSurfaceEdge].v3Normal);

            if (v3A == v3B || v3A == -v3B)
                continue; // Parallel

            AddAxis(Normalize(CrossProduct(v3A, v3B)));
        }
    }
}


/*====================
  CConvexPolyhedron::AddFace
  ====================*/
void    CConvexPolyhedron::AddFace(const CVec3f &v0, const CVec3f &v1, const CVec3f &v2)
{
    AddPlane(CPlane(v0, v1, v2, true));

    AddEdge(CEdge(v0, v1));
    AddEdge(CEdge(v1, v2));
    AddEdge(CEdge(v2, v0));

    uint i0(AddPoint(v0));
    uint i1(AddPoint(v1));
    uint i2(AddPoint(v2));

    m_vTriList.push_back(i0);
    m_vTriList.push_back(i1);
    m_vTriList.push_back(i2);
}


