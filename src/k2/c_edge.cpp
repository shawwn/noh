// (C)2005 S2 Games
// c_edge.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_edge.h"
#include "c_axis.h"
//=============================================================================


/*====================
  CEdge::CEdge
  ====================*/
CEdge::CEdge(const CVec3f &a, const CVec3f &b) :
v3Normal(Normalize(b - a)),
v3Point(a)
{
}


/*====================
  CEdge::Transform
  ====================*/
void    CEdge::Transform(const CAxis &aAxis, const CVec3f &v3Pos, float fScale)
{
    v3Point = TransformPoint(v3Point, aAxis, v3Pos, fScale);    // Transform point
    v3Normal = TransformPoint(v3Normal, aAxis);                 // Rotate normal
    v3Normal.Normalize();                                       // Renormalized
}
