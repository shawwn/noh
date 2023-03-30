// (C)2005 S2 Games
// c_plane.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_plane.h"
#include "c_axis.h"
//=============================================================================


/*====================
  CPlane::Transform
  ====================*/
void	CPlane::Transform(const CVec3f &v3Pos, const CAxis &axis, float fScale)
{
	CVec3f v3P;

	Scale(fScale);
	v3P = v3Normal * fDist;						// Get a point on the plane
	v3P = TransformPoint(v3P, axis, v3Pos);		// Transform it
	v3Normal = TransformPoint(v3Normal, axis);	// Rotate normal
	v3Normal.Normalize();						// Probably not necessary, but may help with numerical imprecision
	fDist = DotProduct(v3P, v3Normal);			// Recalc plane distance using new normal and point
}


