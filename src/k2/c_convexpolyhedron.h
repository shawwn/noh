// (C)2005 S2 Games
// c_convexpolyhedron.h
//=============================================================================
#ifndef __C_CONVEXPOLYHEDRON_H__
#define __C_CONVEXPOLYHEDRON_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_boundingbox.h"
#include "c_plane.h"
#include "c_edge.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CAxis;

struct SExtents
{
    CVec3f  v3Axis;
    float   fMin;
    float   fMax;
};
//=============================================================================

//=============================================================================
// CConvexPolyhedron
//=============================================================================
class CConvexPolyhedron
{
private:
    CBBoxf          m_bbBounds;

    vector<CPlane>  m_vPlanes;
    vector<CEdge>   m_vEdges;
    vector<CVec3f>  m_vPoints;

    vector<uint>    m_vTriList; // index list into m_vPoints

    vector<SExtents>    m_vExtents;

    bool            AddAxis(const CVec3f &v3Axis);

public:
    ~CConvexPolyhedron() {};
    CConvexPolyhedron() {};

    K2_API  CConvexPolyhedron(const CBBoxf &bb); // AABB
    K2_API  CConvexPolyhedron(const CVec3f &v1, const CVec3f &v2, const CVec3f &v3); // Terrain tile
    K2_API  CConvexPolyhedron(const CVec3f &v3Origin, const CAxis &aAxis, float fFovX, float fFovY, float fFar); // Frustum

    void    ReservePlanes(uint ui)      { m_vPlanes.reserve(ui); }
    void    ReservePoints(uint ui)      { m_vPoints.reserve(ui); }
    void    ReserveEdges(uint ui)       { m_vEdges.reserve(ui); }
    void    ReserveTris(uint ui)        { m_vTriList.reserve(ui); }

    K2_API void AddPlane(const CPlane &plane);
    K2_API uint AddPoint(const CVec3f &v3Point);
    K2_API void AddEdge(const CEdge &cEdge);
    K2_API void AddTri(uint i0, uint i1, uint i2);
    K2_API void AddFace(const CVec3f &v0, const CVec3f &v1, const CVec3f &v2);
    void            SetBounds(const CBBoxf &bb) { m_bbBounds = bb; }
    K2_API void CalcExtents();

    const CBBoxf&   GetBounds() const               { return m_bbBounds; }
    uint            GetNumPlanes() const            { return uint(m_vPlanes.size()); }
    uint            GetNumPoints() const            { return uint(m_vPoints.size()); }
    uint            GetNumEdges() const             { return uint(m_vEdges.size()); }
    const CPlane&   GetPlane(uint ui) const         { return m_vPlanes[ui]; }
    const CVec3f&   GetPoint(uint ui) const         { return m_vPoints[ui]; }
    const CEdge&    GetEdge(uint ui) const          { return m_vEdges[ui]; }
    const vector<uint>& GetTriList() const          { return m_vTriList; }
    const vector<CVec3f>& GetPoints() const         { return m_vPoints; }

    const SExtents& GetExtents(uint ui) const       { return m_vExtents[ui]; }
    uint            GetNumAxes() const              { return uint(m_vExtents.size()); }

    CConvexPolyhedron   GetTransformed(const CVec3f &v3Pos, const CAxis &axis, float fScale);
    K2_API void     Transform(const CVec3f &v3Pos, const CAxis &axis, float fScale);
};
//=============================================================================
#endif //__C_CONVEXPOLYHEDRON_H__
