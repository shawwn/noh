// (C)2005 S2 Games
// intersection.cpp
//
// geometry intersection routines
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_vec3.h"
#include "intersection.h"
#include "k2_mathlib.h"
#include "c_sphere.h"
#include "c_mesh.h"
#include "c_convexpolyhedron.h"
#include "c_plane2.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define	M_CROSS(a,b,out) M_CrossProduct(b,a,out)
//=============================================================================


/*====================
  I_ProjectTriangle
  ====================*/
void	I_ProjectTriangle(const vec3_t rkD, const vec3_t apkTri[3], float *rfMin, float *rfMax)
{
	float fDot;

    *rfMin = M_DotProduct(rkD, apkTri[0]);
    *rfMax = *rfMin;

    fDot = M_DotProduct(rkD, apkTri[1]);
    if ( fDot < *rfMin )
        *rfMin = fDot;
    else if ( fDot > *rfMax )
        *rfMax = fDot;

    fDot = M_DotProduct(rkD, apkTri[2]);
    if ( fDot < *rfMin )
        *rfMin = fDot;
    else if ( fDot > *rfMax )
        *rfMax = fDot;
}


/*====================
  I_ProjectBox
  ====================*/
void	I_ProjectBox(const vec3_t rkD, const obb_t *rkBox, float *rfMin, float *rfMax)
{
    float fDdC = M_DotProduct(rkD, rkBox->center);
    float fR =
        rkBox->extents[0]*ABS(M_DotProduct(rkD,rkBox->axis[0])) +
		rkBox->extents[1]*ABS(M_DotProduct(rkD,rkBox->axis[1])) +
		rkBox->extents[2]*ABS(M_DotProduct(rkD,rkBox->axis[2]));

    *rfMin = fDdC - fR;
    *rfMax = fDdC + fR;
}


/*====================
  I_NoBoxTriIntersect
  ====================*/
bool	I_NoBoxTriIntersect(float fTMax, float fSpeed, float fMin0, float fMax0,
							float fMin1, float fMax1, float *rfTFirst, float *rfTLast)
{
    float fInvSpeed, fT;

    if ( fMax1 < fMin0 )  // C1 initially on left of C0
    {
        if ( fSpeed <= 0.0 )
        {
            // intervals moving apart
            return true;
        }

        fInvSpeed = 1.0f / fSpeed;

        fT = (fMin0 - fMax1)*fInvSpeed;
        if ( fT > *rfTFirst )
            *rfTFirst = fT;
        if ( *rfTFirst > fTMax )
            return true;

        fT = (fMax0 - fMin1)*fInvSpeed;
        if ( fT < *rfTLast )
            *rfTLast = fT;
        if ( *rfTFirst > *rfTLast )
            return true;
    }
    else if (fMax0 < fMin1)  // C1 initially on right of C0
    {
        if (fSpeed >= 0.0f)
        {
            // intervals moving apart
            return true;
        }

        fInvSpeed = 1.0f / fSpeed;

        fT = (fMax0 - fMin1)*fInvSpeed;
        if ( fT > *rfTFirst )
            *rfTFirst = fT;
        if ( *rfTFirst > fTMax )
            return true;

        fT = (fMin0 - fMax1)*fInvSpeed;
        if ( fT < *rfTLast )
            *rfTLast = fT;
        if ( *rfTFirst > *rfTLast )
            return true;
    }
    else  // C0 and C1 overlap
    {
        if ( fSpeed > 0.0f )
        {
            fT = (fMax0 - fMin1)/fSpeed;
            if ( fT < *rfTLast )
                *rfTLast = fT;
            if ( *rfTFirst > *rfTLast )
                return true;
        }
        else if ( fSpeed < 0.0f )
        {
            fT = (fMin0 - fMax1)/fSpeed;
            if ( fT < *rfTLast )
                *rfTLast = fT;
            if ( *rfTFirst > *rfTLast )
                return true;
        }
    }

    return false;
}


/*====================
  I_BoxTriIntersect
  ====================*/
bool	I_BoxTriIntersect(const vec3_t apkTri[3], const vec3_t bPos, const vec3_t bext,
						  const vec3_t rkBoxVel, float fTMax, float *rfTFirst, float *rfTLast)
{
	int i, i0, i1;
    float fMin0, fMax0, fMin1, fMax1, fSpeed;
    vec3_t kD, akE[3];
	vec3_t kW;
	obb_t rkBox;

	SET_VEC3(rkBox.axis[0], 1, 0, 0);
	SET_VEC3(rkBox.axis[1], 0, 1, 0);
	SET_VEC3(rkBox.axis[2], 0, 0, 1);
	M_CopyVec3(bPos, rkBox.center);
	M_CopyVec3(bext, rkBox.extents);

	M_CopyVec3(rkBoxVel, kW);

    *rfTFirst = 0.0f;
    *rfTLast = FAR_AWAY;

    // test direction of triangle normal
	M_SubVec3(apkTri[1], apkTri[0], akE[0]);
	M_SubVec3(apkTri[2], apkTri[0], akE[1]);

	M_CROSS(akE[0], akE[1], kD);
    fMin0 = M_DotProduct(kD, apkTri[0]);
    fMax0 = fMin0;

    I_ProjectBox(kD,&rkBox,&fMin1,&fMax1);
	fSpeed = M_DotProduct(kD, kW);

    if ( I_NoBoxTriIntersect(fTMax,fSpeed,fMin0,fMax0,fMin1,fMax1,rfTFirst,rfTLast) )
        return false;

    // test direction of box faces
    for (i = 0; i < 3; ++i)
    {
		float fDdC;

		M_CopyVec3(rkBox.axis[i], kD);

        I_ProjectTriangle(kD,apkTri,&fMin0,&fMax0);
		fDdC = M_DotProduct(kD, rkBox.center);

        fMin1 = fDdC - rkBox.extents[i];
        fMax1 = fDdC + rkBox.extents[i];
		fSpeed = M_DotProduct(kD, kW);

        if ( I_NoBoxTriIntersect(fTMax,fSpeed,fMin0,fMax0,fMin1,fMax1,rfTFirst,
             rfTLast) )
        {
            return false;
        }
    }

    // test direction of triangle - box edge cross products
	M_SubVec3(akE[1], akE[0], akE[2]);

	for (i0 = 0; i0 < 3; ++i0)
	{
		for (i1 = 0; i1 < 3; ++i1)
		{
			M_CROSS(akE[i0], rkBox.axis[i1], kD);
			I_ProjectTriangle(kD, apkTri, &fMin0, &fMax0);
			I_ProjectBox(kD, &rkBox, &fMin1, &fMax1);
			fSpeed = M_DotProduct(kD, kW);
			if (I_NoBoxTriIntersect(fTMax, fSpeed, fMin0, fMax0, fMin1, fMax1, rfTFirst, rfTLast))
			{
				return false;
			}
		}
	}

    return true;
}


/*====================
  I_LineBoundsIntersect
  ====================*/
bool	I_LineBoundsIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CBBoxf &bbBounds, float &fFraction, CPlane *pPlaneHit, bool *pbStartInSurface)
{
	if (bbBounds.Contains(v3Start))
	{
		fFraction = 0;

		if (pbStartInSurface)
			*pbStartInSurface = true;

		if (pPlaneHit)
			*pPlaneHit = CPlane(Normalize(v3End - v3Start), v3Start);

		return true;
	}

	CVec3f v3Delta(v3End - v3Start);

	// Delta is zero and we already tested the bounds
	if (v3Delta.x == 0.0f && v3Delta.y == 0.0f && v3Delta.z == 0.0f)
		return false;

	const CVec3f &v3Min(bbBounds.GetMin());
	const CVec3f &v3Max(bbBounds.GetMax());

	float fEnter(-FAR_AWAY);
	float fExit(FAR_AWAY);
	uint uiFirstAxis(-1);

	// Get times of overlapping axes
	// The boxes only intersect when all extents overlap
	for (uint i(0); i < 3; ++i)
	{
		if (v3Start[i] <= v3Min[i]) // Line starting on the negative side of the bounds
		{
			if (v3Delta[i] <= 0.0f)
				return false; // moving away from each other

			float t0((v3Min[i] - v3Start[i]) / v3Delta[i]);
			
			if (t0 >= fEnter)
			{
				fEnter = t0;
				uiFirstAxis = i;
			}
			if (fEnter > 1.0f)
				return false; // Line never reaches bounds
			
			float t1((v3Max[i] - v3Start[i]) / v3Delta[i]);
			if (t1 < fExit)
				fExit = t1;
			if (fEnter > fExit)
				return false; // Line leaves before it entered in a different axis
		}
		else if (v3Start[i] >= v3Max[i]) // Line starting on positive side of bounds
		{
			if (v3Delta[i] >= 0.0f)
				return false; // moving away from each other

			float t0((v3Max[i] - v3Start[i]) / v3Delta[i]);

			if (t0 > 1.0f)
				return false; // Line never reaches bounds
			if (t0 >= fEnter)
			{
				fEnter = t0;
				uiFirstAxis = i;
			}
			
			float t1((v3Min[i] - v3Start[i]) / v3Delta[i]);
			
			if (t1 < fExit)
				fExit = t1;
			if (fEnter > fExit)
				return false; // Line leaves before it entered in a different axis
		}
		else // A and B starting overlapped
		{
			// Only adjust fExit
			if (v3Delta[i] > 0.0f)
			{
				float t1((v3Max[i] - v3Start[i]) / v3Delta[i]);
				
				if (t1 < fExit)
					fExit = t1;
				if (fEnter > fExit)
					return false; // Line leaves before it entered in a different axis
			}
			else if (v3Delta[i] < 0.0f)
			{
				float t1((v3Min[i] - v3Start[i]) / v3Delta[i]);
				
				if (t1 < fExit)
					fExit = t1;
				if (fEnter > fExit)
					return false; // Line leaves before it entered in a different axis
			}
		}
	}

	if (fEnter < fFraction)
	{
		if (pbStartInSurface && (uiFirstAxis == uint(-1) || fEnter < -0.01f))
			*pbStartInSurface = true;

		if (uiFirstAxis == uint(-1))
		{
			return false;
		}
		else if (pPlaneHit)
		{
			CVec3f v3Axis
			(
				uiFirstAxis == 0 ? 1.0f : 0.0f,
				uiFirstAxis == 1 ? 1.0f : 0.0f,
				uiFirstAxis == 2 ? 1.0f : 0.0f
			);

			if (DotProduct(v3Delta, v3Axis) < 0.0f)
				*pPlaneHit = CPlane(v3Axis, LERP(fFraction, v3Start, v3End));
			else
				*pPlaneHit = CPlane(-v3Axis, LERP(fFraction, v3Start, v3End));
		}

		fFraction = MAX(fEnter, 0.0f);

		return true;
	}

	return false;
}


/*====================
  I_LineBoundsOverlap
  ====================*/
bool	I_LineBoundsOverlap(const CVec3f &v3Start, const CVec3f &v3End, const CBBoxf &bbBounds, float &fEnter, float &fExit)
{
	CVec3f v3Delta(v3End - v3Start);

	const CVec3f &v3Min(bbBounds.GetMin());
	const CVec3f &v3Max(bbBounds.GetMax());

	fEnter = 0.0f;
	fExit = 1.0f;

	// Get times of overlapping axes
	// The boxes only intersect when all extents overlap
	for (uint i(0); i < 3; ++i)
	{
		if (v3Start[i] <= v3Min[i]) // Line starting on the negative side of the bounds
		{
			if (v3Delta[i] <= 0.0f)
				return false; // moving away from each other

			float t0((v3Min[i] - v3Start[i]) / v3Delta[i]);
			
			if (t0 >= fEnter)
				fEnter = t0;

			if (fEnter > 1.0f)
				return false; // Line never reaches bounds
			
			float t1((v3Max[i] - v3Start[i]) / v3Delta[i]);
			if (t1 < fExit)
				fExit = t1;
			if (fEnter > fExit)
				return false; // Line leaves before it entered in a different axis
		}
		else if (v3Start[i] >= v3Max[i]) // Line starting on positive side of bounds
		{
			if (v3Delta[i] >= 0.0f)
				return false; // moving away from each other

			float t0((v3Max[i] - v3Start[i]) / v3Delta[i]);

			if (t0 > 1.0f)
				return false; // Line never reaches bounds
			if (t0 >= fEnter)
				fEnter = t0;
			
			float t1((v3Min[i] - v3Start[i]) / v3Delta[i]);
			
			if (t1 < fExit)
				fExit = t1;
			if (fEnter > fExit)
				return false; // Line leaves before it entered in a different axis
		}
		else // A and B starting overlapped
		{
			// Only adjust fExit
			if (v3Delta[i] > 0.0f)
			{
				float t1((v3Max[i] - v3Start[i]) / v3Delta[i]);
				
				if (t1 < fExit)
					fExit = t1;
				if (fEnter > fExit)
					return false; // Line leaves before it entered in a different axis
			}
			else if (v3Delta[i] < 0.0f)
			{
				float t1((v3Min[i] - v3Start[i]) / v3Delta[i]);
				
				if (t1 < fExit)
					fExit = t1;
				if (fEnter > fExit)
					return false; // Line leaves before it entered in a different axis
			}
		}
	}

	return true;
}


/*====================
  I_MovingBoundsBoundsOverlap

  Bounds A is moving
  ====================*/
bool	I_MovingBoundsBoundsOverlap(const CVec3f &v3Start, const CVec3f &v3End, const CBBoxf &bbBoundsA, const CBBoxf &bbBoundsB, float &fEnter, float &fExit)
{
	CVec3f v3Delta(v3End - v3Start);

	const CVec3f &v3MinA(bbBoundsA.GetMin());
	const CVec3f &v3MaxA(bbBoundsA.GetMax());
	const CVec3f &v3MinB(bbBoundsB.GetMin());
	const CVec3f &v3MaxB(bbBoundsB.GetMax());

	fEnter = 0.0f;
	fExit = 1.0f;

	// Get times of overlapping axes
	// The boxes only intersect when all extents overlap
	for (uint i(0); i < 3; ++i)
	{
		if (v3MaxA[i] <= v3MinB[i]) // A starting on the negative side of the B
		{
			if (v3Delta[i] <= 0.0f)
				return false; // moving away from each other

			float t0((v3MinB[i] - v3MaxA[i]) / v3Delta[i]);
			
			if (t0 >= fEnter)
				fEnter = t0;

			if (fEnter > 1.0f)
				return false; // A never reaches B
			
			float t1((v3MaxB[i] - v3MinA[i]) / v3Delta[i]);
			if (t1 < fExit)
				fExit = t1;
			if (fEnter > fExit)
				return false; // A leaves before it entered in a different axis
		}
		else if (v3MinA[i] >= v3MaxB[i]) // A starting on positive side of B
		{
			if (v3Delta[i] >= 0.0f)
				return false; // moving away from each other

			float t0((v3MaxB[i] - v3MinA[i]) / v3Delta[i]);

			if (t0 > 1.0f)
				return false; // A never reaches B
			if (t0 >= fEnter)
				fEnter = t0;
			
			float t1((v3MinB[i] - v3MaxA[i]) / v3Delta[i]);
			
			if (t1 < fExit)
				fExit = t1;
			if (fEnter > fExit)
				return false; // A leaves before it entered in a different axis
		}
		else // A and B starting overlapped
		{
			// Only adjust fExit
			if (v3Delta[i] > 0.0f)
			{
				float t1((v3MaxB[i] - v3MinA[i]) / v3Delta[i]);
				
				if (t1 < fExit)
					fExit = t1;
				if (fEnter > fExit)
					return false; // A leaves before it entered in a different axis
			}
			else if (v3Delta[i] < 0.0f)
			{
				float t1((v3MinB[i] - v3MaxA[i]) / v3Delta[i]);
				
				if (t1 < fExit)
					fExit = t1;
				if (fEnter > fExit)
					return false; // A leaves before it entered in a different axis
			}
		}
	}

	return true;
}


/*====================
  I_RayTriIntersect
  ====================*/
bool	I_RayTriIntersect(const CVec3f &orig, const CVec3f &dir, const CVec3f &vert0, const CVec3f &vert1, const CVec3f &vert2, float &fDistance)
{
	CVec3f edge1, edge2, tvec, pvec, qvec;
	float det, inv_det;
	float u, v;

	// find vectors for two edges sharing vert0
	edge1 = vert1 - vert0;
	edge2 = vert2 - vert0;

	// begin calculating determinant - also used to calculate U parameter
	pvec = CrossProduct(dir, edge2);

	// if determinant is near zero, ray lies in plane of triangle
	det = DotProduct(edge1, pvec);

	// the non - culling branch
	if (det > -EPSILON && det < EPSILON)
		return false;
	inv_det = 1.0f / det;

	// calculate distance from vert0 to ray origin
	tvec = orig - vert0;

	// calculate U parameter and test bounds
	u = DotProduct(tvec, pvec) * inv_det;
	if (u < 0.0f || u > 1.0f)
		return false;

	// prepare to test V parameter
	qvec = CrossProduct(tvec, edge1);

	// calculate V parameter and test bounds
	v = DotProduct(dir, qvec) * inv_det;
	if (v < 0.0f || u + v > 1.0f)
		return false;

	// calculate t, ray intersects triangle
	fDistance = DotProduct(edge2, qvec) * inv_det;

	if (fDistance < 0.0f)
		return false;

	return true;
}


/*====================
  I_LineMeshIntersect
  ====================*/
bool	I_LineMeshIntersect(const CVec3f &start, const CVec3f &end, const CMesh *mesh, float &fFraction, int &face_hit)
{
	int facehit = -1;
	uint *face;

	for (int tri = 0; tri < mesh->numFaces; ++tri)
	{
		face = mesh->faceList[tri];
		CVec3f &p1 = CVec3_cast(mesh->verts[face[0]]);
		CVec3f &p2 = CVec3_cast(mesh->verts[face[1]]);
		CVec3f &p3 = CVec3_cast(mesh->verts[face[2]]);

		if (I_LineTriangleIntersect(start, end, p1, p2, p3, fFraction))
		{
			facehit = tri;
		}
	}

	if (facehit != -1)
	{
		face_hit = facehit;
		return true;
	}

	return false;
}


/*==========================
  I_RayTriIntersect
  from http://www.acm.org/jgt/papers/MollerTrumbore97/
  ==========================*/
bool	I_RayTriIntersect(const vec3_t orig, const vec3_t dir, vec3_t vert0, vec3_t vert1, vec3_t vert2, float *fraction)
{
	float edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
	float det,inv_det;
	float u,v;

	// find vectors for two edges sharing vert0
	M_SubVec3(vert1, vert0, edge1);
	M_SubVec3(vert2, vert0, edge2);

	// begin calculating determinant - also used to calculate U parameter
	M_CrossProduct(dir, edge2, pvec);

	// if determinant is near zero, ray lies in plane of triangle
	det = M_DotProduct(edge1, pvec);

	// the non - culling branch
	if (det > -EPSILON && det < EPSILON)
		return false;
	inv_det = 1.0f / det;

	// calculate distance from vert0 to ray origin
	M_SubVec3(orig, vert0, tvec);

	// calculate U parameter and test bounds
	u = M_DotProduct(tvec, pvec) * inv_det;
	if (u < 0.0f || u > 1.0f)
		return false;

	// prepare to test V parameter
	M_CrossProduct(tvec, edge1, qvec);

	// calculate V parameter and test bounds
	v = M_DotProduct(dir, qvec) * inv_det;
	if (v < 0.0f || u + v > 1.0f)
		return false;

	// calculate t, ray intersects triangle
	*fraction = M_DotProduct(edge2, qvec) * inv_det;

	if (*fraction < 0)
		return false;

	return true;
}


/*==========================
  I_SphereBoundsIntersect
  ==========================*/
bool	I_SphereBoundsIntersect(const CSphere &sSphere, const CBBoxf &bbBox)
{
	float fDeltaMin(0.0f);
	const CVec3f &vCenter = sSphere.GetCenter();
	const CVec3f &vMin = bbBox.GetMin();
	const CVec3f &vMax = bbBox.GetMax();

	if (vCenter.x < vMin.x)
		fDeltaMin += SQR(vCenter.x - vMin.x);
	else if (vCenter.x > vMax.x)
		fDeltaMin += SQR(vCenter.x - vMax.x);

	if (vCenter.y < vMin.y)
		fDeltaMin += SQR(vCenter.y - vMin.y);
	else if (vCenter.y > vMax.y)
		fDeltaMin += SQR(vCenter.y - vMax.y);

	if (vCenter.z < vMin.z)
		fDeltaMin += SQR(vCenter.z - vMin.z);
	else if (vCenter.z > vMax.z)
		fDeltaMin += SQR(vCenter.z - vMax.z);

	return fDeltaMin <= SQR(sSphere.GetRadius());
}


/*==========================
  I_CircleRectIntersect
  ==========================*/
bool	I_CircleRectIntersect(const CVec2f &v2Center, float fRadius, const CRectf &rect)
{
	float fDeltaMin(0.0f);

	if (v2Center.x < rect.left)
		fDeltaMin += SQR(v2Center.x - rect.left);
	else if (v2Center.x > rect.right)
		fDeltaMin += SQR(v2Center.x - rect.right);

	if (v2Center.y < rect.top)
		fDeltaMin += SQR(v2Center.y - rect.top);
	else if (v2Center.y > rect.bottom)
		fDeltaMin += SQR(v2Center.y - rect.bottom);

	return fDeltaMin <= SQR(fRadius);
}


/*==========================
  I_LineTriangleIntersect
  from http://www.acm.org/jgt/papers/MollerTrumbore97/
  ==========================*/
bool	I_LineTriangleIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CVec3f &v1, const CVec3f &v2, const CVec3f &v3, float &fFraction)
{
	CVec3f v3Delta(v3End - v3Start);

	// find vectors for two edges sharing v1
	CVec3f v3Edge1(v2 - v1);
	CVec3f v3Edge2(v3 - v1);

	// begin calculating determinant - also used to calculate U parameter
	CVec3f v3P(CrossProduct(v3Delta, v3Edge2));

	// if determinant is near zero, line lies in plane of triangle
	float fDet(DotProduct(v3Edge1, v3P));

	// the non-culling branch
	if (fDet > -EPSILON && fDet < EPSILON)
		return false;

	// calculate distance from vert0 to ray origin
	CVec3f v3T(v3Start - v1);

	// calculate U parameter and test bounds
	float fU(DotProduct(v3T, v3P) / fDet);
	if (fU < 0.0f || fU > 1.0f)
		return false;

	// prepare to test V parameter
	CVec3f v3Q(CrossProduct(v3T, v3Edge1));

	// calculate V parameter and test bounds
	float fV(DotProduct(v3Delta, v3Q) / fDet);
	if (fV < 0.0f || fU + fV > 1.0f)
		return false;

	// calculate t, ray intersects triangle
	float fNewFraction(DotProduct(v3Edge2, v3Q) / fDet);
	if (fNewFraction < fFraction && fNewFraction >= 0.0f)
	{
		fFraction = fNewFraction;
		return true;
	}

	return false;
}


/*==========================
  I_LineQuadIntersect

  v1->v3 is the shared edge
  ==========================*/
bool	I_LineQuadIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CVec3f &v1, const CVec3f &v2, const CVec3f &v3, const CVec3f &v4, float &fFraction)
{
	CVec3f v3Delta(v3End - v3Start);

	// find vectors for two edges sharing v1
	CVec3f v3Edge1(v2 - v1);
	CVec3f v3Edge2(v3 - v1);
	CVec3f v3Edge3(v4 - v1);

	// begin calculating determinant - also used to calculate U parameter
	CVec3f v3P(CrossProduct(v3Delta, v3Edge2));

	float fDet1(DotProduct(v3Edge1, v3P));
	float fDet2(DotProduct(v3Edge3, v3P));

	// calculate distance from v1 to line start
	CVec3f v3T(v3Start - v1);

	float fUDot(DotProduct(v3T, v3P));

	// if determinant is near zero, line lies in plane of triangle
	if (fDet1 <= -EPSILON || fDet1 >= EPSILON)
	{
		// calculate U parameter and test bounds
		float fU(fUDot / fDet1);
		if (fU >= 0.0f && fU <= 1.0f)
		{
			// prepare to test V parameter
			CVec3f v3Q(CrossProduct(v3T, v3Edge1));

			// calculate V parameter and test bounds
			float fV(DotProduct(v3Delta, v3Q) / fDet1);
			if (fV >= 0.0f && fU + fV <= 1.0f)
			{
				// calculate t, ray intersects triangle
				float fNewFraction(DotProduct(v3Edge2, v3Q) / fDet1);
				if (fNewFraction < fFraction && fNewFraction >= 0.0f)
				{
					fFraction = fNewFraction;
					return true;
				}
			}
		}
	}

	// if determinant is near zero, line lies in plane of triangle
	if (fDet2 <= -EPSILON || fDet2 >= EPSILON)
	{
		// calculate U parameter and test bounds
		float fU(fUDot / fDet2);
		if (fU >= 0.0f && fU <= 1.0f)
		{
			// prepare to test V parameter
			CVec3f v3Q(CrossProduct(v3T, v3Edge3));

			// calculate V parameter and test bounds
			float fV(DotProduct(v3Delta, v3Q) / fDet2);
			if (fV >= 0.0f && fU + fV <= 1.0f)
			{
				// calculate t, ray intersects triangle
				float fNewFraction(DotProduct(v3Edge2, v3Q) / fDet2);
				if (fNewFraction < fFraction && fNewFraction >= 0.0f)
				{
					fFraction = fNewFraction;
					return true;
				}
			}
		}
	}

	return false;
}


/*====================
  I_LineSurfaceIntersect
  ====================*/
bool	I_LineSurfaceIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CConvexPolyhedron &surface, float &fFraction, CPlane *pPlaneHit, bool *pbStartInSurface)
{
	int idx(-1);
	float fEnter(-1.0f), fLeave(1.0f);
	bool bStartOut(false), bGetOut(false);

	for (uint n(0); n < surface.GetNumPlanes(); ++n)
	{
		float t;

		const CPlane &plane(surface.GetPlane(n));

		// Find distance from plane at start point
		float t0(DotProduct(v3Start, plane.v3Normal) - plane.fDist);
		// Find distance from plane at end point
		float t1(DotProduct(v3End, plane.v3Normal) - plane.fDist);

		if (t0 > 0)
			bStartOut = true;
		if (t1 > 0)
			bGetOut = true;

		if (t0 > 0 && t1 > 0) // no intersection
			return false;

		if (t0 <= 0 && t1 <= 0)	// inside plane
			continue;
		
		if (t0 == t1)   // parallel to plane, no intersection
			continue;

		if (t0 > t1)	// t0 is positive, going towards the surface
		{
			t = t0 / (t0 - t1);
			if (t > fEnter)
			{
				fEnter = t;
				idx = n;
			}
		}
		else			// t1 is positive, going away from the surface
		{
			t = t0 / (t0 - t1);
			if (t < fLeave)
				fLeave = t;
		}
	}

	if (!bStartOut)
	{
		if (pbStartInSurface)
			*pbStartInSurface = true;
		
		fFraction = 0.0f;

		if (pPlaneHit)
			*pPlaneHit = CPlane(Normalize(v3End - v3Start), v3Start);

		return true;
	}

	if (fEnter < fLeave)
	{
		if (fEnter > -1.0f && fEnter < fFraction && idx != -1)
		{
			if (fEnter < 0.0f)
				fEnter = 0.0f;

			fFraction = fEnter;

			if (pPlaneHit)
				*pPlaneHit = surface.GetPlane(idx);

			return true;
		}
	}

	return false;
}


/*====================
  I_LineTerrainIntersect
  ====================*/
bool	I_LineTerrainIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CPlane aPlanes[], float &fFraction, CPlane *pPlaneHit, bool *pbStartInSurface)
{
	int idx(-1);
	float fEnter(-1.0f), fLeave(1.0f);
	bool bStartOut(false), bGetOut(false);

	for (uint n(0); n < 4; ++n)
	{
		float t;

		const CPlane &plane(aPlanes[n]);

		// Find distance from plane at start point
		float t0(DotProduct(v3Start, plane.v3Normal) - plane.fDist);
		// Find distance from plane at end point
		float t1(DotProduct(v3End, plane.v3Normal) - plane.fDist);

		if (t0 > 0)
			bStartOut = true;
		if (t1 > 0)
			bGetOut = true;

		if (t0 > 0 && t1 > 0) // no intersection
			return false;

		if (t0 <= 0 && t1 <= 0)	// inside plane
			continue;
		
		if (t0 == t1)   // parallel to plane, no intersection
			continue;

		if (t0 > t1)	// t0 is positive, going towards the surface
		{
			t = t0 / (t0 - t1);
			if (t > fEnter)
			{
				fEnter = t;
				idx = n;
			}
		}
		else			// t1 is positive, going away from the surface
		{
			t = t0 / (t0 - t1);
			if (t < fLeave)
				fLeave = t;
		}
	}

	if (!bStartOut)
	{
		if (pbStartInSurface)
			*pbStartInSurface = true;
		
		fFraction = 0.0f;

		if (pPlaneHit)
			*pPlaneHit = CPlane(Normalize(v3End - v3Start), v3Start);

		return true;
	}

	if (fEnter < fLeave)
	{
		if (fEnter > -1.0f && fEnter < fFraction && idx != -1)
		{
			if (fEnter < 0.0f)
				fEnter = 0.0f;

			fFraction = fEnter;

			if (pPlaneHit)
				*pPlaneHit = aPlanes[idx];

			return true;
		}
	}

	return false;
}


/*====================
  I_LineBlockerIntersect
  ====================*/
bool	I_LineBlockerIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CPlane aPlanes[], float &fFraction, CPlane *pPlaneHit, bool *pbStartInSurface)
{
	int idx(-1);
	float fEnter(-1.0f), fLeave(1.0f);
	bool bStartOut(false), bGetOut(false);

	for (uint n(0); n < 3; ++n)
	{
		float t;

		const CPlane &plane(aPlanes[n]);

		// Find distance from plane at start point
		float t0(DotProduct(v3Start, plane.v3Normal) - plane.fDist);
		// Find distance from plane at end point
		float t1(DotProduct(v3End, plane.v3Normal) - plane.fDist);

		if (t0 > 0)
			bStartOut = true;
		if (t1 > 0)
			bGetOut = true;

		if (t0 > 0 && t1 > 0) // no intersection
			return false;

		if (t0 <= 0 && t1 <= 0)	// inside plane
			continue;
		
		if (t0 == t1)   // parallel to plane, no intersection
			continue;

		if (t0 > t1)	// t0 is positive, going towards the surface
		{
			t = t0 / (t0 - t1);
			if (t > fEnter)
			{
				fEnter = t;
				idx = n;
			}
		}
		else			// t1 is positive, going away from the surface
		{
			t = t0 / (t0 - t1);
			if (t < fLeave)
				fLeave = t;
		}
	}

	if (!bStartOut)
	{
		if (pbStartInSurface)
			*pbStartInSurface = true;
		
		fFraction = 0.0f;

		if (pPlaneHit)
			*pPlaneHit = CPlane(Normalize(v3End - v3Start), v3Start);

		return true;
	}

	if (fEnter < fLeave)
	{
		if (fEnter > -1.0f && fEnter < fFraction && idx != -1)
		{
			if (fEnter < 0.0f)
				fEnter = 0.0f;

			fFraction = fEnter;

			if (pPlaneHit)
				*pPlaneHit = aPlanes[idx];

			return true;
		}
	}

	return false;
}


/*====================
  I_MovingBoundsBoundsIntersect

  Bounds A is moving
  ====================*/
bool	I_MovingBoundsBoundsIntersect(const CVec3f &v3Start, const CVec3f &v3Delta, const CBBoxf &bbBoundsA, const CBBoxf &bbBoundsB, float &fFraction, CPlane *pPlaneHit, bool *pbStartInSurface)
{
	if (IntersectBounds(bbBoundsA, bbBoundsB))
	{
		fFraction = 0;
		
		if (pbStartInSurface)
			*pbStartInSurface = true;

		if (pPlaneHit)
			*pPlaneHit = CPlane(Normalize(v3Delta), v3Start);

		return true;
	}

	const CVec3f &v3MinA(bbBoundsA.GetMin());
	const CVec3f &v3MaxA(bbBoundsA.GetMax());
	const CVec3f &v3MinB(bbBoundsB.GetMin());
	const CVec3f &v3MaxB(bbBoundsB.GetMax());

	float fEnter(0.0f);
	float fExit(1.0f);
	uint uiFirstAxis(-1);

	// Get times of overlapping axes
	// The boxes only intersect when all extents overlap
	
	//
	// x-axis
	//
	if (v3MaxA.x <= v3MinB.x) // A starting on the negative side of the B
	{
		if (v3Delta.x <= 0.0f)
			return false; // moving away from each other

		float t0((v3MinB.x - v3MaxA.x) / v3Delta.x);
		
		if (t0 >= fEnter)
		{
			fEnter = t0;
			uiFirstAxis = 0;
		}
		if (fEnter > 1.0f)
			return false; // A never reaches B
		
		float t1((v3MaxB.x - v3MinA.x) / v3Delta.x);
		if (t1 < fExit)
			fExit = t1;
		if (fEnter > fExit)
			return false; // A leaves before it entered in a different axis
	}
	else if (v3MinA.x >= v3MaxB.x) // A starting on positive side of B
	{
		if (v3Delta.x >= 0.0f)
			return false; // moving away from each other

		float t0((v3MaxB.x - v3MinA.x) / v3Delta.x);

		if (t0 > 1.0f)
			return false; // A never reaches B
		if (t0 >= fEnter)
		{
			fEnter = t0;
			uiFirstAxis = 0;
		}
		
		float t1((v3MinB.x - v3MaxA.x) / v3Delta.x);
		
		if (t1 < fExit)
			fExit = t1;
		if (fEnter > fExit)
			return false; // A leaves before it entered in a different axis
	}
	else // A and B starting overlapped
	{
		// Only adjust fExit
		if (v3Delta.x > 0.0f)
		{
			float t1((v3MaxB.x - v3MinA.x) / v3Delta.x);
			
			if (t1 < fExit)
				fExit = t1;
			if (fEnter > fExit)
				return false; // A leaves before it entered in a different axis
		}
		else if (v3Delta.x < 0.0f)
		{
			float t1((v3MinB.x - v3MaxA.x) / v3Delta.x);
			
			if (t1 < fExit)
				fExit = t1;
			if (fEnter > fExit)
				return false; // A leaves before it entered in a different axis
		}
	}

	//
	// y-axis
	//
	if (v3MaxA.y <= v3MinB.y) // A starting on the negative side of the B
	{
		if (v3Delta.y <= 0.0f)
			return false; // moving away from each other

		float t0((v3MinB.y - v3MaxA.y) / v3Delta.y);
		
		if (t0 >= fEnter)
		{
			fEnter = t0;
			uiFirstAxis = 1;
		}
		if (fEnter > 1.0f)
			return false; // A never reaches B
		
		float t1((v3MaxB.y - v3MinA.y) / v3Delta.y);
		if (t1 < fExit)
			fExit = t1;
		if (fEnter > fExit)
			return false; // A leaves before it entered in a different axis
	}
	else if (v3MinA.y >= v3MaxB.y) // A starting on positive side of B
	{
		if (v3Delta.y >= 0.0f)
			return false; // moving away from each other

		float t0((v3MaxB.y - v3MinA.y) / v3Delta.y);

		if (t0 > 1.0f)
			return false; // A never reaches B
		if (t0 >= fEnter)
		{
			fEnter = t0;
			uiFirstAxis = 1;
		}
		
		float t1((v3MinB.y - v3MaxA.y) / v3Delta.y);
		
		if (t1 < fExit)
			fExit = t1;
		if (fEnter > fExit)
			return false; // A leaves before it entered in a different axis
	}
	else // A and B starting overlapped
	{
		// Only adjust fExit
		if (v3Delta.y > 0.0f)
		{
			float t1((v3MaxB.y - v3MinA.y) / v3Delta.y);
			
			if (t1 < fExit)
				fExit = t1;
			if (fEnter > fExit)
				return false; // A leaves before it entered in a different axis
		}
		else if (v3Delta.y < 0.0f)
		{
			float t1((v3MinB.y - v3MaxA.y) / v3Delta.y);
			
			if (t1 < fExit)
				fExit = t1;
			if (fEnter > fExit)
				return false; // A leaves before it entered in a different axis
		}
	}

	//
	// z-axis
	//
	if (v3MaxA.z <= v3MinB.z) // A starting on the negative side of the B
	{
		if (v3Delta.z <= 0.0f)
			return false; // moving away from each other

		float t0((v3MinB.z - v3MaxA.z) / v3Delta.z);
		
		if (t0 >= fEnter)
		{
			fEnter = t0;
			uiFirstAxis = 2;
		}
		if (fEnter > 1.0f)
			return false; // A never reaches B
		
		float t1((v3MaxB.z - v3MinA.z) / v3Delta.z);
		if (t1 < fExit)
			fExit = t1;
		if (fEnter > fExit)
			return false; // A leaves before it entered in a different axis
	}
	else if (v3MinA.z >= v3MaxB.z) // A starting on positive side of B
	{
		if (v3Delta.z >= 0.0f)
			return false; // moving away from each other

		float t0((v3MaxB.z - v3MinA.z) / v3Delta.z);

		if (t0 > 1.0f)
			return false; // A never reaches B
		if (t0 >= fEnter)
		{
			fEnter = t0;
			uiFirstAxis = 2;
		}
		
		float t1((v3MinB.z - v3MaxA.z) / v3Delta.z);
		
		if (t1 < fExit)
			fExit = t1;
		if (fEnter > fExit)
			return false; // A leaves before it entered in a different axis
	}
	else // A and B starting overlapped
	{
		// Only adjust fExit
		if (v3Delta.z > 0.0f)
		{
			float t1((v3MaxB.z - v3MinA.z) / v3Delta.z);
			
			if (t1 < fExit)
				fExit = t1;
			if (fEnter > fExit)
				return false; // A leaves before it entered in a different axis
		}
		else if (v3Delta.z < 0.0f)
		{
			float t1((v3MinB.z - v3MaxA.z) / v3Delta.z);
			
			if (t1 < fExit)
				fExit = t1;
			if (fEnter > fExit)
				return false; // A leaves before it entered in a different axis
		}
	}

	if (uiFirstAxis == uint(-1))
	{
		if (pbStartInSurface)
			*pbStartInSurface = true;

		if (pPlaneHit)
			*pPlaneHit = CPlane(Normalize(v3Delta), v3Start);

		fFraction = 0.0f;

		return true;
	}

	if (fEnter < fFraction)
	{
		fFraction = MAX(fEnter, 0.0f);

		if (pPlaneHit)
		{
			CVec3f v3Axis
			(
				uiFirstAxis == 0 ? 1.0f : 0.0f,
				uiFirstAxis == 1 ? 1.0f : 0.0f,
				uiFirstAxis == 2 ? 1.0f : 0.0f
			);

			if (DotProduct(v3Delta, v3Axis) < 0.0f)
				*pPlaneHit = CPlane(v3Axis, v3Start + v3Delta * fFraction);
			else
				*pPlaneHit = CPlane(-v3Axis, v3Start + v3Delta * fFraction);
		}

		return true;
	}

	return false;
}


/*====================
  I_MovingBoundsSurfaceIntersect

  Derived from the method explained in http://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
  ====================*/
bool	I_MovingBoundsSurfaceIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CBBoxf &bbBounds, const CConvexPolyhedron &surface, float &fFraction, CPlane *pPlaneHit, bool *pbStartInSurface)
{
	uint uiNumAxes(surface.GetNumAxes());

	CVec3f v3Delta(v3End - v3Start);

	float fEnter(0.0f);
	float fExit(1.0f);
	uint uiFirstAxis(-1);

	for (uint n(0); n < uiNumAxes; ++n)
	{
		const SExtents &sExtents(surface.GetExtents(n));
		
		// Length of v3Delta along the current test axis
		float fDelta(DotProduct(v3Delta, sExtents.v3Axis));

		float fBoxMin, fBoxMax;
		M_CalcAxisExtents(sExtents.v3Axis, bbBounds, fBoxMin, fBoxMax);

		if (fBoxMax <= sExtents.fMin) // box starting on the negative side of the surface
		{
			if (fDelta <= 0.0f)
				return false; // moving away from each other

			float t0((sExtents.fMin - fBoxMax) / fDelta);
			
			if (t0 >= fEnter)
			{
				fEnter = t0;
				uiFirstAxis = n;
			}
			if (fEnter > 1.0f)
				return false; // box never reaches surface
			
			float t1((sExtents.fMax - fBoxMin) / fDelta);
			if (t1 < fExit)
				fExit = t1;
			if (fEnter > fExit)
				return false; // box leaves before it entered in a different axis
		}
		else if (fBoxMin >= sExtents.fMax) // box starting on positive side of surface
		{
			if (fDelta >= 0.0f)
				return false; // moving away from each other

			float t0((sExtents.fMax - fBoxMin) / fDelta);

			if (t0 > 1.0f)
				return false; // box never reaches surface
			if (t0 >= fEnter)
			{
				fEnter = t0;
				uiFirstAxis = n;
			}
			
			float t1((sExtents.fMin - fBoxMax) / fDelta);
			
			if (t1 < fExit)
				fExit = t1;
			if (fEnter > fExit)
				return false; // box leaves before it entered in a different axis
		}
		else // box and surface starting overlapped
		{
			// Only adjust fExit
			if (fDelta > 0.0f)
			{
				float t1((sExtents.fMax - fBoxMin) / fDelta);
				
				if (t1 < fExit)
					fExit = t1;
				if (fEnter > fExit)
					return false; // box leaves before it entered in a different axis
			}
			else if (fDelta < 0.0f)
			{
				float t1((sExtents.fMin - fBoxMax) / fDelta);
				
				if (t1 < fExit)
					fExit = t1;
				if (fEnter > fExit)
					return false; // box leaves before it entered in a different axis
			}
		}
	}

	if (uiFirstAxis == uint(-1))
	{
		if (pbStartInSurface)
			*pbStartInSurface = true;

		if (pPlaneHit)
			*pPlaneHit = CPlane(Normalize(v3End - v3Start), v3Start);

		fFraction = 0.0f;

		return true;
	}

	if (fEnter < fFraction)
	{
		fFraction = MAX(fEnter, 0.0f);

		if (pPlaneHit)
		{
			CVec3f v3Axis(surface.GetExtents(uiFirstAxis).v3Axis);

			if (DotProduct(v3Delta, v3Axis) < 0.0f)
				*pPlaneHit = CPlane(v3Axis, LERP(fFraction, v3Start, v3End));
			else
				*pPlaneHit = CPlane(-v3Axis, LERP(fFraction, v3Start, v3End));
		}

		return true;
	}

	return false;
}


/*====================
  I_TestAxis
  ====================*/
static bool	I_TestAxis(const CVec3f &v3Axis, float fMin, float fMax, const CVec3f &v3Delta, const CBBoxf &bbBounds, float &fEnter, float &fExit, bool &bFirst)
{
	bFirst = false;

	float fDelta(DotProduct(v3Delta, v3Axis));

	float fBoxMin, fBoxMax;
	M_CalcAxisExtents(v3Axis, bbBounds, fBoxMin, fBoxMax);

	if (fBoxMax <= fMin) // box starting on the negative side of the surface
	{
		if (fDelta <= 0.0f)
			return false; // moving away from each other

		float t0((fMin - fBoxMax) / fDelta);
		
		if (t0 >= fEnter)
		{
			fEnter = t0;
			bFirst = true;
		}
		if (fEnter > 1.0f)
			return false; // box never reaches surface
		
		float t1((fMax - fBoxMin) / fDelta);
		if (t1 < fExit)
			fExit = t1;
		if (fEnter > fExit)
			return false; // box leaves before it entered in a different axis
	}
	else if (fBoxMin >= fMax) // box starting on positive side of surface
	{
		if (fDelta >= 0.0f)
			return false; // moving away from each other

		float t0((fMax - fBoxMin) / fDelta);

		if (t0 > 1.0f)
			return false; // box never reaches surface
		if (t0 >= fEnter)
		{
			fEnter = t0;
			bFirst = true;
		}
		
		float t1((fMin - fBoxMax) / fDelta);
		
		if (t1 < fExit)
			fExit = t1;
		if (fEnter > fExit)
			return false; // box leaves before it entered in a different axis
	}
	else // box and surface starting overlapped
	{
		// Only adjust fExit
		if (fDelta > 0.0f)
		{
			float t1((fMax - fBoxMin) / fDelta);
			
			if (t1 < fExit)
				fExit = t1;
			if (fEnter > fExit)
				return false; // box leaves before it entered in a different axis
		}
		else if (fDelta < 0.0f)
		{
			float t1((fMin - fBoxMax) / fDelta);
			
			if (t1 < fExit)
				fExit = t1;
			if (fEnter > fExit)
				return false; // box leaves before it entered in a different axis
		}
	}

	return true;
}


/*====================
  I_AddAxis

  Assumes good normals and no duplicates
  ====================*/
static bool	I_AddAxis(const CVec3f &v3Axis, const CVec3f vPoints[], CVec3f vAxes[], uint &uiNumAxes, const CVec3f &v3Delta, const CBBoxf &bbBounds, float &fEnter, float &fExit, uint &uiFirstAxis)
{
	// Extents
	float fMin(FAR_AWAY);
	float fMax(-FAR_AWAY);

	float fDist0(DotProduct(v3Axis, vPoints[0]));
	if (fDist0 < fMin)
		fMin = fDist0;
	if (fDist0 > fMax)
		fMax = fDist0;

	float fDist1(DotProduct(v3Axis, vPoints[1]));
	if (fDist1 < fMin)
		fMin = fDist1;
	if (fDist1 > fMax)
		fMax = fDist1;

	float fDist2(DotProduct(v3Axis, vPoints[2]));
	if (fDist2 < fMin)
		fMin = fDist2;
	if (fDist2 > fMax)
		fMax = fDist2;

	bool bFirst;
	if (!I_TestAxis(v3Axis, fMin, fMax, v3Delta, bbBounds, fEnter, fExit, bFirst))
		return false;

	if (bFirst)
		uiFirstAxis = uiNumAxes;

	vAxes[uiNumAxes++] = v3Axis;

	return true;
}


/*====================
  I_TestXAxis
  ====================*/
static bool	I_TestXAxis(float fMin, float fMax, float fDelta, float fBoxMin, float fBoxMax, float &fEnter, float &fExit, bool &bFirst)
{
	bFirst = false;

	if (fBoxMax <= fMin) // box starting on the negative side of the surface
	{
		if (fDelta <= 0.0f)
			return false; // moving away from each other

		float t0((fMin - fBoxMax) / fDelta);
		
		if (t0 >= fEnter)
		{
			fEnter = t0;
			bFirst = true;
		}
		if (fEnter > 1.0f)
			return false; // box never reaches surface
		
		float t1((fMax - fBoxMin) / fDelta);
		if (t1 < fExit)
			fExit = t1;
		if (fEnter > fExit)
			return false; // box leaves before it entered in a different axis
	}
	else if (fBoxMin >= fMax) // box starting on positive side of surface
	{
		if (fDelta >= 0.0f)
			return false; // moving away from each other

		float t0((fMax - fBoxMin) / fDelta);

		if (t0 > 1.0f)
			return false; // box never reaches surface
		if (t0 >= fEnter)
		{
			fEnter = t0;
			bFirst = true;
		}
		
		float t1((fMin - fBoxMax) / fDelta);
		
		if (t1 < fExit)
			fExit = t1;
		if (fEnter > fExit)
			return false; // box leaves before it entered in a different axis
	}
	else // box and surface starting overlapped
	{
		// Only adjust fExit
		if (fDelta > 0.0f)
		{
			float t1((fMax - fBoxMin) / fDelta);
			
			if (t1 < fExit)
				fExit = t1;
			if (fEnter > fExit)
				return false; // box leaves before it entered in a different axis
		}
		else if (fDelta < 0.0f)
		{
			float t1((fMin - fBoxMax) / fDelta);
			
			if (t1 < fExit)
				fExit = t1;
			if (fEnter > fExit)
				return false; // box leaves before it entered in a different axis
		}
	}

	return true;
}


/*====================
  I_AddXAxis

  Assumes good normals and no duplicates
  ====================*/
static bool	I_AddXAxis(const CVec3f vPoints[], CVec3f vAxes[], uint &uiNumAxes, float fDelta, const CBBoxf &bbBounds, float &fEnter, float &fExit, uint &uiFirstAxis)
{
	// Extents
	float fMin(FAR_AWAY);
	float fMax(-FAR_AWAY);

	if (vPoints[0].x < fMin)
		fMin = vPoints[0].x;
	if (vPoints[0].x > fMax)
		fMax = vPoints[0].x;

	if (vPoints[1].x < fMin)
		fMin = vPoints[1].x;
	if (vPoints[1].x > fMax)
		fMax = vPoints[1].x;

	if (vPoints[2].x < fMin)
		fMin = vPoints[2].x;
	if (vPoints[2].x > fMax)
		fMax = vPoints[2].x;

	bool bFirst;
	if (!I_TestXAxis(fMin, fMax, fDelta, bbBounds.GetMin().x, bbBounds.GetMax().x, fEnter, fExit, bFirst))
		return false;

	if (bFirst)
		uiFirstAxis = uiNumAxes;

	vAxes[uiNumAxes++] = CVec3f(1.0f, 0.0f, 0.0f);

	return true;
}


/*====================
  I_AddYAxis

  Assumes good normals and no duplicates
  ====================*/
static bool	I_AddYAxis(const CVec3f vPoints[], CVec3f vAxes[], uint &uiNumAxes, float fDelta, const CBBoxf &bbBounds, float &fEnter, float &fExit, uint &uiFirstAxis)
{
	// Extents
	float fMin(FAR_AWAY);
	float fMax(-FAR_AWAY);

	if (vPoints[0].y < fMin)
		fMin = vPoints[0].y;
	if (vPoints[0].y > fMax)
		fMax = vPoints[0].y;

	if (vPoints[1].y < fMin)
		fMin = vPoints[1].y;
	if (vPoints[1].y > fMax)
		fMax = vPoints[1].y;

	if (vPoints[2].y < fMin)
		fMin = vPoints[2].y;
	if (vPoints[2].y > fMax)
		fMax = vPoints[2].y;

	bool bFirst;
	if (!I_TestXAxis(fMin, fMax, fDelta, bbBounds.GetMin().y, bbBounds.GetMax().y, fEnter, fExit, bFirst))
		return false;

	if (bFirst)
		uiFirstAxis = uiNumAxes;

	vAxes[uiNumAxes++] = CVec3f(0.0f, 1.0f, 0.0f);

	return true;
}


/*====================
  I_AddZAxis

  Assumes good normals and no duplicates
  ====================*/
static bool	I_AddZAxis(const CVec3f vPoints[], CVec3f vAxes[], uint &uiNumAxes, float fDelta, const CBBoxf &bbBounds, float &fEnter, float &fExit, uint &uiFirstAxis)
{
	// Extents
	float fMin(FAR_AWAY);
	float fMax(-FAR_AWAY);

	if (vPoints[0].z < fMin)
		fMin = vPoints[0].z;
	if (vPoints[0].z > fMax)
		fMax = vPoints[0].z;

	if (vPoints[1].z < fMin)
		fMin = vPoints[1].z;
	if (vPoints[1].z > fMax)
		fMax = vPoints[1].z;

	if (vPoints[2].z < fMin)
		fMin = vPoints[2].z;
	if (vPoints[2].z > fMax)
		fMax = vPoints[2].z;

	bool bFirst;
	if (!I_TestXAxis(fMin, fMax, fDelta, bbBounds.GetMin().z, bbBounds.GetMax().z, fEnter, fExit, bFirst))
		return false;

	if (bFirst)
		uiFirstAxis = uiNumAxes;

	vAxes[uiNumAxes++] = CVec3f(0.0f, 0.0f, 1.0f);

	return true;
}


/*====================
  I_AddAxis2
  ====================*/
static bool	I_AddAxis2(const CVec3f &v3Axis, const CVec3f vPoints[], CVec3f vAxes[], uint &uiNumAxes, const CVec3f &v3Delta, const CBBoxf &bbBounds, float &fEnter, float &fExit, uint &uiFirstAxis)
{
	if (v3Axis.LengthSq() < 0.5f) // Check for malformed normal
		return true;

	// Skip duplicated axes
	uint ui(0);
	for (; ui != uiNumAxes; ++ui)
	{
		if (vAxes[ui] == v3Axis || vAxes[ui] == -v3Axis)
			return true;
	}

	// Extents
	float fMin(FAR_AWAY);
	float fMax(-FAR_AWAY);

	float fDist0(DotProduct(v3Axis, vPoints[0]));
	if (fDist0 < fMin)
		fMin = fDist0;
	if (fDist0 > fMax)
		fMax = fDist0;

	float fDist1(DotProduct(v3Axis, vPoints[1]));
	if (fDist1 < fMin)
		fMin = fDist1;
	if (fDist1 > fMax)
		fMax = fDist1;

	float fDist2(DotProduct(v3Axis, vPoints[2]));
	if (fDist2 < fMin)
		fMin = fDist2;
	if (fDist2 > fMax)
		fMax = fDist2;

	bool bFirst;
	if (!I_TestAxis(v3Axis, fMin, fMax, v3Delta, bbBounds, fEnter, fExit, bFirst))
		return false;

	if (bFirst)
		uiFirstAxis = uiNumAxes;

	vAxes[uiNumAxes++] = v3Axis;

	return true;
}


/*====================
  I_MovingBoundsTerrainIntersect

  Derived from the method explained in http://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
  v1, v2, and v3 need to be in special order v1->v2->v3 forms a right triangle with v2 being the right angle vertex
  ====================*/
bool	I_MovingBoundsTerrainIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CBBoxf &bbBounds, const CPlane aPlanes[], const CVec3f &v1, const CVec3f &v2, const CVec3f &v3, float &fFraction, CPlane *pPlaneHit, bool *pbStartInSurface)
{
	CVec3f	vAxes[12];
	uint	uiNumAxes(0);

	// Points
	CVec3f	vPoints[3] = { v1, v2, v3 };
	CVec3f	v3Delta(v3End - v3Start);

	float fEnter(0.0f);
	float fExit(1.0f);
	uint uiFirstAxis(-1);

	// Box axes (x, y, z)
	if (!I_AddXAxis(vPoints, vAxes, uiNumAxes, v3Delta.x, bbBounds, fEnter, fExit, uiFirstAxis))
		return false;
	if (!I_AddYAxis(vPoints, vAxes, uiNumAxes, v3Delta.y, bbBounds, fEnter, fExit, uiFirstAxis))
		return false;
	if (!I_AddZAxis(vPoints, vAxes, uiNumAxes, v3Delta.z, bbBounds, fEnter, fExit, uiFirstAxis))
		return false;

	// The planes of this polyhedron
	if (aPlanes[0].v3Normal.z != 1.0f || aPlanes[0].v3Normal.x != 0.0f || aPlanes[0].v3Normal.y != 0.0f)
		if (!I_AddAxis(aPlanes[0].v3Normal, vPoints, vAxes, uiNumAxes, v3Delta, bbBounds, fEnter, fExit, uiFirstAxis))
			return false;
	if (!I_AddAxis(aPlanes[1].v3Normal, vPoints, vAxes, uiNumAxes, v3Delta, bbBounds, fEnter, fExit, uiFirstAxis))
		return false;

	// Edge cross-product planes
	CVec3f	vEdges[3] =
	{
		Normalize(v2 - v1),
		Normalize(v3 - v2),
		Normalize(v1 - v3)
	};

	// Do to the way the vertexes are ordered vEdge[0].x and vEdge[1].y are always zero
	if (vEdges[0].y != 0.0f && vEdges[0].z != 0.0f) // Parallel
		if (!I_AddAxis2(Normalize(CVec3f(0.0f, vEdges[0].z, -vEdges[0].y)), vPoints, vAxes, uiNumAxes, v3Delta, bbBounds, fEnter, fExit, uiFirstAxis))
			return false; 
	
	if (vEdges[2].x != 1.0f && vEdges[2].x != -1.0f && vEdges[2].y != 0.0f && vEdges[2].z != 0.0f) // Parallel
		if (!I_AddAxis2(Normalize(CVec3f(0.0f, vEdges[2].z, -vEdges[2].y)), vPoints, vAxes, uiNumAxes, v3Delta, bbBounds, fEnter, fExit, uiFirstAxis))
			return false;
	
	if (vEdges[1].x != 0.0f && vEdges[1].z != 0.0f) // Parallel
		if (!I_AddAxis2(Normalize(CVec3f(-vEdges[1].z, 0.0f, vEdges[1].x)), vPoints, vAxes, uiNumAxes, v3Delta, bbBounds, fEnter, fExit, uiFirstAxis))
			return false;
	
	if (vEdges[2].x != 0.0f && vEdges[2].y != 1.0f && vEdges[2].y != -1.0f && vEdges[2].z != 0.0f) // Parallel
		if (!I_AddAxis2(Normalize(CVec3f(-vEdges[2].z, 0.0f, vEdges[2].x)), vPoints, vAxes, uiNumAxes, v3Delta, bbBounds, fEnter, fExit, uiFirstAxis))
			return false;
	
	if (vEdges[2].x != 0.0f && vEdges[2].y != 0.0f && vEdges[2].z != 1.0f && vEdges[2].z != -1.0f) // Parallel
		if (!I_AddAxis2(Normalize(CVec3f(vEdges[2].y, -vEdges[2].x, 0.0f)), vPoints, vAxes, uiNumAxes, v3Delta, bbBounds, fEnter, fExit, uiFirstAxis))
			return false;

	if (uiFirstAxis == uint(-1))
	{
		if (pbStartInSurface)
			*pbStartInSurface = true;

		if (pPlaneHit)
			*pPlaneHit = CPlane(Normalize(v3End - v3Start), v3Start);

		fFraction = 0.0f;

		return true;
	}

	if (fEnter < fFraction)
	{
		fFraction = MAX(fEnter, 0.0f);

		if (pPlaneHit)
		{
			CVec3f v3Axis(vAxes[uiFirstAxis]);

			if (DotProduct(v3Delta, v3Axis) < 0.0f)
				*pPlaneHit = CPlane(v3Axis, LERP(fFraction, v3Start, v3End));
			else
				*pPlaneHit = CPlane(-v3Axis, LERP(fFraction, v3Start, v3End));
		}

		return true;
	}

	return false;
}


/*====================
  I_LinePlaneIntersect
  ====================*/
bool	I_LinePlaneIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CPlane &plane, float &fFraction)
{
	float d1 = plane.Distance(v3Start);

	// start point after plane
	if (d1 < 0.0f)
		return false;

	float d2 = plane.Distance(v3End);

	// end before plane
	if (d2 > 0.0f)
		return false;

	float fNewFraction = d1 / (d1 - d2);

	if (fNewFraction < fFraction && fNewFraction >= 0.0f)
	{
		fFraction = fNewFraction;
		return true;
	}

	return false;
}


/*====================
  I_LineDoubleSidedPlaneIntersect
  ====================*/
bool	I_LineDoubleSidedPlaneIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CPlane &plane, float &fFraction)
{
	float d1(plane.Distance(v3Start));
	float d2(plane.Distance(v3End));

	float fNewFraction(d1 / (d1 - d2));

	if (fNewFraction < fFraction && fNewFraction >= 0.0f)
	{
		fFraction = fNewFraction;
		return true;
	}

	return false;
}


/*====================
  I_LineDoubleSidedPlaneIntersect
  ====================*/
bool	I_LineDoubleSidedPlaneIntersect(const CVec2f &v2Start, const CVec2f &v2End, const CPlane2 &plane, float &fFraction)
{
	float d1(plane.Distance(v2Start));
	float d2(plane.Distance(v2End));

	float fNewFraction(d1 / (d1 - d2));

	if (fNewFraction < fFraction && fNewFraction >= 0.0f)
	{
		fFraction = fNewFraction;
		return true;
	}

	return false;
}


/*====================
  I_BoundsSurfaceIntersect

  Derived from the method explained in http://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
  ====================*/
bool	I_BoundsSurfaceIntersect(const CBBoxf &bbBounds, const CConvexPolyhedron &surface)
{
	uint uiNumAxes(surface.GetNumAxes());

	for (uint n(0); n < uiNumAxes; ++n)
	{
		const SExtents &sExtents(surface.GetExtents(n));

		float fBoxMin, fBoxMax;
		M_CalcAxisExtents(sExtents.v3Axis, bbBounds, fBoxMin, fBoxMax);

		if (fBoxMax <= sExtents.fMin)		// box on the negative side of the surface
			return false;
		else if (fBoxMin >= sExtents.fMax)	// box on positive side of surface
			return false;
	}

	return true;
}


/*====================
  I_SurfaceSurfaceIntersect
  ====================*/
bool	I_SurfaceSurfaceIntersect(const CConvexPolyhedron &cSurface1, const CConvexPolyhedron &cSurface2)
{
	if (!IntersectBounds(cSurface1.GetBounds(), cSurface2.GetBounds()))
		return false;

	// Test Surface 1 planes
	for (uint n(0); n < cSurface1.GetNumPlanes(); ++n)
	{
		const CPlane &plPlane(cSurface1.GetPlane(n));

		plPlane.Side(cSurface1.GetPoints());

		if (plPlane.Side(cSurface2.GetPoints()) == PLANE_POSITIVE)
			return false;
	}

	// Test Surface 2 planes
	for (uint n(0); n < cSurface2.GetNumPlanes(); ++n)
	{
		const CPlane &plPlane(cSurface2.GetPlane(n));

		if (plPlane.Side(cSurface1.GetPoints()) == PLANE_POSITIVE)
			return false;
	}

	// Test Cross-product planes
	for (uint n1(0); n1 < cSurface1.GetNumEdges(); ++n1)
	{
		const CVec3f &v3A(cSurface1.GetEdge(n1).v3Normal);
		const CVec3f &v3P(cSurface1.GetEdge(n1).v3Point);

		for (uint n2(0); n2 < cSurface2.GetNumEdges(); ++n2)
		{			
			const CVec3f &v3B(cSurface2.GetEdge(n2).v3Normal);

			if (v3A == v3B || v3A == -v3B)
				continue; // Parallel

			CPlane plPlane(Normalize(CrossProduct(v3A, v3B)), v3P);

			EPlaneTest test1(plPlane.Side(cSurface1.GetPoints()));
			EPlaneTest test2(plPlane.Side(cSurface1.GetPoints()));

			if (test1 != PLANE_INTERSECTS && test2 != PLANE_INTERSECTS && test1 != test2)
				return false;
		}
	}

	return true;
}


/*====================
  I_MovingBoundsBlockerIntersect

  Derived from the method explained in http://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
  v1, v2, and v3 need to be in special order v1->v2->v3 forms a right triangle with v2 being the right angle vertex
  ====================*/
bool	I_MovingBoundsBlockerIntersect(const CVec3f &v3Start, const CVec3f &v3End, const CBBoxf &bbBounds, const CVec3f &v3Diag, const CVec3f &v1, const CVec3f &v2, const CVec3f &v3, float &fFraction, CPlane *pPlaneHit, bool *pbStartInSurface)
{
	CVec3f	vAxes[12];
	uint	uiNumAxes(0);

	// Points
	CVec3f	vPoints[3] = { v1, v2, v3 };
	CVec3f	v3Delta(v3End - v3Start);

	float fEnter(0.0f);
	float fExit(1.0f);
	uint uiFirstAxis(-1);

	// Box axes (x, y, z)
	if (!I_AddXAxis(vPoints, vAxes, uiNumAxes, v3Delta.x, bbBounds, fEnter, fExit, uiFirstAxis))
		return false;
	if (!I_AddYAxis(vPoints, vAxes, uiNumAxes, v3Delta.y, bbBounds, fEnter, fExit, uiFirstAxis))
		return false;

	// The planes of this polyhedron
	if (!I_AddAxis(v3Diag, vPoints, vAxes, uiNumAxes, v3Delta, bbBounds, fEnter, fExit, uiFirstAxis))
		return false;

	// Edge cross-product planes
	CVec3f	vEdges[3] =
	{
		Normalize(v2 - v1),
		Normalize(v3 - v2),
		Normalize(v1 - v3)
	};

	// Do to the way the vertexes are ordered vEdge[0].x and vEdge[1].y and Zs are always zero
	if (vEdges[2].x != 0.0f && vEdges[2].y != 0.0f && vEdges[2].z != 1.0f && vEdges[2].z != -1.0f) // Parallel
		if (!I_AddAxis2(Normalize(CVec3f(vEdges[2].y, -vEdges[2].x, 0.0f)), vPoints, vAxes, uiNumAxes, v3Delta, bbBounds, fEnter, fExit, uiFirstAxis))
			return false;

	if (uiFirstAxis == uint(-1))
	{
		if (pbStartInSurface)
			*pbStartInSurface = true;

		if (pPlaneHit)
			*pPlaneHit = CPlane(Normalize(v3End - v3Start), v3Start);

		fFraction = 0.0f;

		return true;
	}

	if (fEnter < fFraction)
	{
		fFraction = MAX(fEnter, 0.0f);

		if (pPlaneHit)
		{
			CVec3f v3Axis(vAxes[uiFirstAxis]);

			if (DotProduct(v3Delta, v3Axis) < 0.0f)
				*pPlaneHit = CPlane(v3Axis, LERP(fFraction, v3Start, v3End));
			else
				*pPlaneHit = CPlane(-v3Axis, LERP(fFraction, v3Start, v3End));
		}

		return true;
	}

	return false;
}
