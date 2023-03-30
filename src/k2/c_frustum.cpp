// (C)2005 S2 Games
// c_frustum.cpp
//
// Frustum class
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_frustum.h"
#include "c_sphere.h"
#include "c_camera.h"
//=============================================================================

/*====================
  CFrustum::CFrustum
  ====================*/
CFrustum::CFrustum()
{
}


/*====================
  CFrustum::CFrustum
  ====================*/
CFrustum::CFrustum(const CVec3f &vOrigin, const CAxis &aAxis, float fFovX, float fFovY, float fNear, float fFar) :
m_vOrigin(vOrigin),
m_fFovX(fFovX),
m_fFovY(fFovY),
m_fNear(fNear),
m_fFar(fFar),
m_Axis(aAxis)
{
	CalcPlanes();
}


/*====================
  CFrustum::CFrustum
  ====================*/
CFrustum::CFrustum(const vector<CVec3f> &vPoints, const CVec3f &vOrigin, const CVec3f &vDirection) :
m_vOrigin(vOrigin),
m_fFovX(90.0f),
m_fFovY(90.0f),
m_fNear(0.1f),
m_fFar(100.0f)
{
	CVec3f vFrustumForward(vDirection);

	CVec3f vFrustumRight(CrossProduct(vFrustumForward, CVec3f(0.0f, 0.0f, -1.0f)));
	if (vFrustumRight.Length() < 0.001f)
		vFrustumRight = Normalize(CrossProduct(vFrustumForward, CVec3f(0.0f, 1.0f, 0.0f)));
	else
		vFrustumRight.Normalize();

	CVec3f vFrustumUp(Normalize(CrossProduct(vFrustumRight, vFrustumForward)));

	m_Axis.Set(vFrustumRight, vFrustumForward, vFrustumUp);

	// Calculate fFovY
	float fWorstFovY(-FAR_AWAY);

	for (vector<CVec3f>::const_iterator it(vPoints.begin()); it != vPoints.end(); ++it)
	{
		CVec3f p3(*it + vFrustumRight);
		CPlane plTestPlane(vOrigin, *it, p3, true);

		float fAngle = acos(ABS(DotProduct(plTestPlane.v3Normal, vFrustumUp)));

		if (fAngle > fWorstFovY)
			fWorstFovY = fAngle;
	}
	m_fFovY = 2.0f * RAD2DEG(fWorstFovY) + 0.001f;

	// Calculate fFovX
	float fWorstFovX(-FAR_AWAY);

	for (vector<CVec3f>::const_iterator it(vPoints.begin()); it != vPoints.end(); ++it)
	{
		CVec3f p3(*it + vFrustumUp);
		CPlane plTestPlane(vOrigin, *it, p3, true);

		float fAngle = acos(ABS(DotProduct(plTestPlane.v3Normal, vFrustumRight)));

		if (fAngle > fWorstFovX)
			fWorstFovX = fAngle;
	}
	m_fFovX = 2.0f * RAD2DEG(fWorstFovX) + 0.001f;

	// Calculate the near and far plane
	CPlane plViewPlane(vFrustumForward, vOrigin);
	float fBestFar = -FAR_AWAY;
	float fBestNear = FAR_AWAY;

	for (vector<CVec3f>::const_iterator it = vPoints.begin(); it != vPoints.end(); ++it)
	{
		float fDistance = plViewPlane.Distance(*it);

		if (fDistance < fBestNear)
			fBestNear = fDistance;

		if (fDistance > fBestFar)
			fBestFar = fDistance;
	}

	m_fNear = MAX(0.001f, fBestNear);
	m_fFar = MIN(FAR_AWAY, fBestFar);

	CalcPlanes();
}


/*====================
  CFrustum::CFrustum
  ====================*/
CFrustum::CFrustum(const vector<CVec3f> &vPoints, const CVec3f &vOrigin, const CAxis &aAxis) :
m_vOrigin(vOrigin),
m_fFovX(90.0f),
m_fFovY(90.0f),
m_fNear(0.1f),
m_fFar(100.0f)
{
	const CVec3f &vFrustumForward(aAxis[FORWARD]);
	const CVec3f &vFrustumRight(aAxis[RIGHT]);
	const CVec3f &vFrustumUp(aAxis[UP]);

	m_Axis = aAxis;

	// Calculate fFovY
	float fWorstFovY(-FAR_AWAY);

	for (vector<CVec3f>::const_iterator it(vPoints.begin()); it != vPoints.end(); ++it)
	{
		CVec3f p3(*it + vFrustumRight);
		CPlane plTestPlane(vOrigin, *it, p3, true);

		float fAngle = acos(ABS(DotProduct(plTestPlane.v3Normal, vFrustumUp)));

		if (fAngle > fWorstFovY)
			fWorstFovY = fAngle;
	}
	m_fFovY = 2.0f * RAD2DEG(fWorstFovY) + 0.001f;

	// Calculate fFovX
	float fWorstFovX(-FAR_AWAY);

	for (vector<CVec3f>::const_iterator it(vPoints.begin()); it != vPoints.end(); ++it)
	{
		CVec3f p3(*it + vFrustumUp);
		CPlane plTestPlane(vOrigin, *it, p3, true);

		float fAngle = acos(ABS(DotProduct(plTestPlane.v3Normal, vFrustumRight)));

		if (fAngle > fWorstFovX)
			fWorstFovX = fAngle;
	}
	m_fFovX = 2.0f * RAD2DEG(fWorstFovX) + 0.001f;

	// Calculate the near and far plane
	CPlane plViewPlane(vFrustumForward, vOrigin);
	float fBestFar = -FAR_AWAY;
	float fBestNear = FAR_AWAY;

	for (vector<CVec3f>::const_iterator it = vPoints.begin(); it != vPoints.end(); ++it)
	{
		float fDistance = plViewPlane.Distance(*it);

		if (fDistance < fBestNear)
			fBestNear = fDistance;

		if (fDistance > fBestFar)
			fBestFar = fDistance;
	}

	m_fNear = MAX(0.001f, fBestNear);
	m_fFar = MIN(FAR_AWAY, fBestFar);

	CalcPlanes();
}


#if 0
/*====================
  CFrustum::CFrustum

  Calculates the frustum of best fit from a set of points and the origin of the frustum
  ====================*/
CFrustum::CFrustum(const vector<CVec3f> &vPoints, const CVec3f &vOrigin) :
m_vOrigin(vOrigin),
m_fFovX(90.0f),
m_fFovY(90.0f),
m_fNear(0.1f),
m_fFar(100.0f)
{
	// Find the best plane that seperates all points from the z = 1 plane (based on vUp(0,0,-1))
	// the best plane will contain both the a point in the set and vOrigin and have
	CPlane	plBestPlane(0.0f, 0.0f, 1.0f, 0.0f);
	CVec3f	vBestRight, vBestUp, vBestForward;
	float	fWorstZSlope = -FAR_AWAY;
	vector<CVec3f>::const_iterator itBest;

	for (vector<CVec3f>::const_iterator it = vPoints.begin(); it != vPoints.end(); ++it)
	{
		// construct a basis between vOrigin and *it that prevents a z = 0 plane of the basis
		// from intersecting the z = 1 plane of the points coordinate system before seperating
		// all points from the z = 1 plane (for shadow frustum purposes)

		CVec3f vForward(Normalize(*it - vOrigin));
		CVec3f vRight;

		// pl1 has the normal vForward containing point *it
		CPlane pl1(vForward, *it);

		// pl2 is the z = *it.z plane
		CPlane pl2(0.0f, 0.0f, -1.0f, -it->z);

		// vRight is a ray of the line between the intersection of the planes p1 and p2
		vRight = Normalize(CrossProduct(pl1.normal, pl2.normal));

		float fZSlope = (it->z - vOrigin.z) / Distance(it->xy(), vOrigin.xy());

		CVec3f vUp(CrossProduct(vRight, vForward));

		CVec3f p2(*it + vRight);

		CPlane plTestPlane(vOrigin, p2, *it, true);

		if (fZSlope > 0.0f)
			continue;

		if (fZSlope > fWorstZSlope)
		{
			itBest = it;
			fWorstZSlope = fZSlope;
			plBestPlane = plTestPlane;
			vBestForward = vForward;
			vBestRight = vRight;
		}
	}

	CPlane	plBestPlane2(0.0f, 0.0f, 1.0f, 0.0f);
	float	fWorstAngle = -FAR_AWAY;
	vector<CVec3f>::const_iterator itBest2;

	// Find the complement to plBestPlane that encloses all points from the top
	for (vector<CVec3f>::const_iterator it = vPoints.begin(); it != vPoints.end(); ++it)
	{
		CVec3f vForward(Normalize(*it - vOrigin));

		CVec3f p3(*it + vBestRight);

		CPlane plTestPlane(vOrigin, *it, p3, true);

		float fDot = DotProduct(plBestPlane.normal, plTestPlane.normal);

		float fAngle = DEG2RAD(180.0f) - acos(fDot);

		if (fAngle > fWorstAngle)
		{
			fWorstAngle = fAngle;

			itBest2 = it;
			plBestPlane2 = plTestPlane;
		}
	}

	CVec3f vFrustumForward(Normalize(plBestPlane.normal + plBestPlane2.normal));
	CVec3f vFrustumRight(Normalize(vBestRight));
	CVec3f vFrustumUp(Normalize(CrossProduct(vFrustumRight, vFrustumForward)));

	m_Axis.Set(vFrustumRight, vFrustumForward, vFrustumUp);

	m_fFovY = RAD2DEG(fWorstAngle) + 0.001f;

	CPlane plViewPlane(vFrustumForward, vOrigin);

	// Calculate fFovX
	float fWorstFovX = -FAR_AWAY;

	for (vector<CVec3f>::const_iterator it = vPoints.begin(); it != vPoints.end(); ++it)
	{
		CVec3f p3(*it + vFrustumUp);
		CPlane plTestPlane(vOrigin, *it, p3, true);

		float fAngle = acos(ABS(DotProduct(plTestPlane.normal, vFrustumRight)));

		if (fAngle > fWorstFovX)
			fWorstFovX = fAngle;
	}
	m_fFovX = 2.0f * RAD2DEG(fWorstFovX) + 0.001f;

	float fBestFar = -FAR_AWAY;
	float fBestNear = FAR_AWAY;

	// Calculate the near and far plane
	for (vector<CVec3f>::const_iterator it = vPoints.begin(); it != vPoints.end(); ++it)
	{
		float fDistance = plViewPlane.Distance(*it);

		if (fDistance < fBestNear)
			fBestNear = fDistance;

		if (fDistance > fBestFar)
			fBestFar = fDistance;
	}

	m_fNear = MAX(0.001f, fBestNear);
	m_fFar = MIN(FAR_AWAY, fBestFar);

	const int MAX_TIGHTEN(4);
	int iNumTighten(0);

	while (Tighten(vPoints) > 0.01 && iNumTighten < MAX_TIGHTEN) { ++iNumTighten; }

	CalcPlanes();
}
#else
/*====================
  CFrustum::CFrustum

  Calculates the frustum of best fit from a set of points and the origin of the frustum
  ====================*/
CFrustum::CFrustum(const vector<CVec3f> &vPoints, const CVec3f &vOrigin) :
m_vOrigin(vOrigin),
m_fFovX(90.0f),
m_fFovY(90.0f),
m_fNear(0.1f),
m_fFar(100.0f)
{
	PROFILE("CFrustum::CFrustum");

	static vector<CVec3f> vPolarPoints;
	vPolarPoints.clear();
	vPolarPoints.reserve(vPoints.size());
	
	vector<CVec3f>::const_iterator itPointsEnd(vPoints.end());
	
	for (vector<CVec3f>::const_iterator it(vPoints.begin()); it != itPointsEnd; ++it)
		vPolarPoints.push_back(M_CartesianToPolar(*it - vOrigin));

	vector<CVec3f>::const_iterator itPolarPointsEnd(vPolarPoints.end());

	// Calculate the mininum containing sector for theta
	float fMinTheta(0.0f);
	float fMinThetaDelta(FAR_AWAY);
	for (vector<CVec3f>::const_iterator it(vPolarPoints.begin()); it != itPolarPointsEnd; ++it)
	{
		float fTestAngle(it->x);
		float fDelta(-FAR_AWAY);

		for (vector<CVec3f>::const_iterator it2(vPolarPoints.begin()); it2 != itPolarPointsEnd; ++it2)
		{
			float fAngle(it2->x - fTestAngle);

			while (fAngle < 0.0f) fAngle += 2.0f * M_PI;

			if (fAngle > fDelta)
				fDelta = fAngle;

			if (fDelta >= fMinThetaDelta)
				break;
		}

		if (fDelta < fMinThetaDelta)
		{
			fMinThetaDelta = fDelta;
			fMinTheta = fTestAngle;
		}
	}

	// Calculate the mininum containing sector for phi
	float fMinPhi(0.0f);
	float fMinPhiDelta(FAR_AWAY);
	for (vector<CVec3f>::const_iterator it(vPolarPoints.begin()); it != itPolarPointsEnd; ++it)
	{
		float fTestAngle(it->y);
		float fDelta(-FAR_AWAY);

		for (vector<CVec3f>::const_iterator it2(vPolarPoints.begin()); it2 != itPolarPointsEnd; ++it2)
		{
			float fAngle(it2->y - fTestAngle);

			while (fAngle < 0.0f) fAngle += 2.0f * M_PI;

			if (fAngle > fDelta)
				fDelta = fAngle;

			if (fDelta >= fMinPhiDelta)
				break;
		}

		if (fDelta < fMinPhiDelta)
		{
			fMinPhiDelta = fDelta;
			fMinPhi = fTestAngle;
		}
	}

	CVec3f vFrustumForward;

	if (fMinThetaDelta > M_PI || fMinPhiDelta > M_PI)
	{
		// TODO: This means the first algorithm failed
		// I'm not sure why it's not working all the time,
		// but in the mean time we just default to the old algorithm

		// Find the best plane that seperates all points from the z = 1 plane (based on vUp(0,0,-1))
		// the best plane will contain both a point in the set and vOrigin
		CPlane	plBestPlane(0.0f, 0.0f, 1.0f, 0.0f);
		CVec3f	vBestRight, vBestUp, vBestForward;
		float	fWorstZSlope = -FAR_AWAY;
		vector<CVec3f>::const_iterator itBest;

		for (vector<CVec3f>::const_iterator it(vPoints.begin()); it != vPoints.end(); ++it)
		{
			// construct a basis between vOrigin and *it that prevents a z = 0 plane of the basis
			// from intersecting the z = 1 plane of the points coordinate system before seperating
			// all points from the z = 1 plane (for shadow frustum purposes)

			CVec3f vForward(Normalize(*it - vOrigin));
			CVec3f vRight;

			// pl1 has the normal vForward containing point *it
			CPlane pl1(vForward, *it);

			// pl2 is the z = *it.z plane
			CPlane pl2(0.0f, 0.0f, -1.0f, -it->z);

			// vRight is a ray of the line between the intersection of the planes p1 and p2
			vRight = Normalize(CrossProduct(pl1.v3Normal, pl2.v3Normal));

			float fZSlope = (it->z - vOrigin.z) / Distance(it->xy(), vOrigin.xy());

			CVec3f vUp(CrossProduct(vRight, vForward));

			CVec3f p2(*it + vRight);

			CPlane plTestPlane(vOrigin, p2, *it, true);

			if (fZSlope > 0.0f)
				continue;

			if (fZSlope > fWorstZSlope)
			{
				itBest = it;
				fWorstZSlope = fZSlope;
				plBestPlane = plTestPlane;
				vBestForward = vForward;
				vBestRight = vRight;
			}
		}

		CPlane	plBestPlane2(0.0f, 0.0f, 1.0f, 0.0f);
		float	fWorstAngle = -FAR_AWAY;
		vector<CVec3f>::const_iterator itBest2;

		// Find the complement to plBestPlane that encloses all points from the top
		for (vector<CVec3f>::const_iterator it(vPoints.begin()); it != vPoints.end(); ++it)
		{
			CVec3f vForward(Normalize(*it - vOrigin));

			CVec3f p3(*it + vBestRight);

			CPlane plTestPlane(vOrigin, *it, p3, true);

			float fAngle = DEG2RAD(180.0f) - acos(DotProduct(plBestPlane.v3Normal, plTestPlane.v3Normal));

			if (fAngle > fWorstAngle)
			{
				fWorstAngle = fAngle;

				itBest2 = it;
				plBestPlane2 = plTestPlane;
			}
		}

		vFrustumForward = Normalize(plBestPlane.v3Normal + plBestPlane2.v3Normal);
	}
	else
	{
		float fMidTheta(fMinTheta + fMinThetaDelta / 2.0f);
		float fMidPhi(fMinPhi + fMinPhiDelta / 2.0f);

		vFrustumForward = M_PolarToCartesian(CVec3f(fMidTheta, fMidPhi, 1.0f));
	}

	CVec3f vFrustumRight(CrossProduct(vFrustumForward, CVec3f(0.0f, 0.0f, -1.0f)));
	if (vFrustumRight.LengthSq() < 0.00001f)
		vFrustumRight = Normalize(CrossProduct(vFrustumForward, CVec3f(0.0f, 1.0f, 0.0f)));
	else
		vFrustumRight.Normalize();

	CVec3f vFrustumUp(Normalize(CrossProduct(vFrustumRight, vFrustumForward)));

	m_Axis.Set(vFrustumRight, vFrustumForward, vFrustumUp);

	// Calculate fFovY, fFovX, and near and far plane
	float fWorstFovY(-FAR_AWAY);
	float fWorstFovX(-FAR_AWAY);
	
	CPlane plViewPlane(vFrustumForward, vOrigin);
	float fBestFar = -FAR_AWAY;
	float fBestNear = FAR_AWAY;

	for (vector<CVec3f>::const_iterator it(vPoints.begin()); it != itPointsEnd; ++it)
	{
		// fFovY
		{
			CVec3f p3(*it + vFrustumRight);
			CPlane plTestPlane(vOrigin, *it, p3, true);

			float fAngle = acos(ABS(DotProduct(plTestPlane.v3Normal, vFrustumUp)));

			if (fAngle > fWorstFovY)
				fWorstFovY = fAngle;
		}

		// fFovX
		{
			CVec3f p3(*it + vFrustumUp);
			CPlane plTestPlane(vOrigin, *it, p3, true);

			float fAngle = acos(ABS(DotProduct(plTestPlane.v3Normal, vFrustumRight)));

			if (fAngle > fWorstFovX)
				fWorstFovX = fAngle;
		}

		// near and far plane
		{
			float fDistance(plViewPlane.Distance(*it));

			if (fDistance < fBestNear)
				fBestNear = fDistance;

			if (fDistance > fBestFar)
				fBestFar = fDistance;
		}
	}
	m_fFovY = 2.0f * RAD2DEG(fWorstFovY) + 0.001f;
	m_fFovX = 2.0f * RAD2DEG(fWorstFovX) + 0.001f;

	m_fNear = MAX(0.001f, fBestNear);
	m_fFar = MIN(FAR_AWAY, fBestFar);

	CalcPlanes();
}
#endif


/*====================
  CFrustum::CalcPlanes

  see www.cs.unc.edu/~hoff/research
  ====================*/
void	CFrustum::CalcPlanes()
{
	const CVec3f &v3ViewRight(m_Axis[RIGHT]);
	const CVec3f &v3ViewDir(m_Axis[FORWARD]);
	const CVec3f &v3ViewUp(m_Axis[UP]);
	const CVec3f &v3Eye(m_vOrigin);
	CVec3f vecInvViewDir(v3ViewDir.GetInverse());

	float fFovX(DEG2RAD(m_fFovX));
	float fFovY(DEG2RAD(m_fFovY));

	float fTanX(tan(fFovX * 0.5f));
	float fTanY(tan(fFovY * 0.5f));

	CVec3f v3Offset(v3Eye + v3ViewDir);

	CVec3f av3BoundingPoints[4];
	av3BoundingPoints[0] = v3ViewRight * fTanX + v3ViewUp * fTanY + v3Offset;	// top - right point
	av3BoundingPoints[1] = v3ViewRight * -fTanX + v3ViewUp * fTanY + v3Offset;	// top - left point
	av3BoundingPoints[2] = v3ViewRight * -fTanX + v3ViewUp * -fTanY + v3Offset;	// bottom - left point
	av3BoundingPoints[3] = v3ViewRight * fTanX + v3ViewUp * -fTanY + v3Offset;	// bottom - right point

	m_Planes[0].CalcPlaneNormalized(av3BoundingPoints[1], av3BoundingPoints[0], v3Eye); // top
	m_Planes[1].CalcPlaneNormalized(av3BoundingPoints[2], av3BoundingPoints[1], v3Eye); // left
	m_Planes[2].CalcPlaneNormalized(av3BoundingPoints[3], av3BoundingPoints[2], v3Eye); // bottom
	m_Planes[3].CalcPlaneNormalized(av3BoundingPoints[0], av3BoundingPoints[3], v3Eye); // right

	// Far plane
	if (NUM_FRUSTUM_PLANES > 4)
	{
		m_Planes[4].v3Normal = vecInvViewDir;
		m_Planes[4].fDist = DotProduct(vecInvViewDir, M_PointOnLine(v3Eye, v3ViewDir, m_fFar));
	}

	// Near plane
	if (NUM_FRUSTUM_PLANES > 5)
	{
		m_Planes[5].v3Normal = v3ViewDir;
		m_Planes[5].fDist = DotProduct(v3ViewDir, M_PointOnLine(v3Eye, v3ViewDir, m_fNear));
	}

	// Compute frustum bounding box
	m_bbBounds.Clear();
	
	CVec3f av3Edges[4];
	av3Edges[0] = Normalize(av3BoundingPoints[0] - v3Eye);
	av3Edges[1] = Normalize(av3BoundingPoints[1] - v3Eye);
	av3Edges[2] = Normalize(av3BoundingPoints[2] - v3Eye);
	av3Edges[3] = Normalize(av3BoundingPoints[3] - v3Eye);

	// near clip plane points
	m_bbBounds.AddPoint(M_PointOnLine(v3Eye, av3Edges[0], m_fNear / DotProduct(av3Edges[0], v3ViewDir)));
	m_bbBounds.AddPoint(M_PointOnLine(v3Eye, av3Edges[1], m_fNear / DotProduct(av3Edges[1], v3ViewDir)));
	m_bbBounds.AddPoint(M_PointOnLine(v3Eye, av3Edges[2], m_fNear / DotProduct(av3Edges[2], v3ViewDir)));
	m_bbBounds.AddPoint(M_PointOnLine(v3Eye, av3Edges[3], m_fNear / DotProduct(av3Edges[3], v3ViewDir)));

	// far clip plane points
	m_bbBounds.AddPoint(M_PointOnLine(v3Eye, av3Edges[0], m_fFar / DotProduct(av3Edges[0], v3ViewDir)));
	m_bbBounds.AddPoint(M_PointOnLine(v3Eye, av3Edges[1], m_fFar / DotProduct(av3Edges[1], v3ViewDir)));
	m_bbBounds.AddPoint(M_PointOnLine(v3Eye, av3Edges[2], m_fFar / DotProduct(av3Edges[2], v3ViewDir)));
	m_bbBounds.AddPoint(M_PointOnLine(v3Eye, av3Edges[3], m_fFar / DotProduct(av3Edges[3], v3ViewDir)));
}


/*====================
  CFrustum::Update
  ====================*/
void	CFrustum::Update(const CCamera &camera)
{
	m_vOrigin = camera.GetOrigin();
	m_Axis = camera.GetViewAxis();
	m_fFovX = camera.GetFovX();
	m_fFovY = camera.GetFovY();
	m_fNear = camera.GetZNear();
	m_fFar = camera.GetZFar();

	CalcPlanes();
}


/*====================
  CFrustum::Touches

  Test if frustum touches an axially-aligned bounding box
  ====================*/
bool	CFrustum::Touches(const CBBoxf &bbBox) const
{
	for (int n = 0; n < NUM_FRUSTUM_PLANES; ++n)
	{
		if (M_AABBOnPlaneSide(bbBox, m_Planes[n]) == PLANE_NEGATIVE)
			return false;
	}

	return true;
}



/*====================
  CFrustum::Touches

  Test if frustum touches a point
  ====================*/
bool	CFrustum::Touches(const CVec3f &vPoint) const
{
	for (int n = 0; n < NUM_FRUSTUM_PLANES; ++n)
	{
		if (m_Planes[n].Side(vPoint) == PLANE_NEGATIVE)
			return false;
	}

	return true;
}



/*====================
  CFrustum::Touches

  Test if frustum contains a sphere
  ====================*/
bool	CFrustum::Touches(const CSphere &sSphere) const
{
	const float fRadius = sSphere.GetRadius();
	const CVec3f &vCenter = sSphere.GetCenter();

	for (int n = 0; n < NUM_FRUSTUM_PLANES; ++n)
	{
		if (m_Planes[n].Distance(vCenter) < -fRadius)
			return false;
	}

	return true;
}


/*====================
  CFrustum::Touches

  Test if frustum touches an oriented bounding box
  ====================*/
bool	CFrustum::Touches(const CBBoxf &bbBox, const CAxis &aAxis, const CVec3f &v3Pos) const
{
	for (int n = 0; n < NUM_FRUSTUM_PLANES; ++n)
	{
		if (M_OBBOnPlaneSide(bbBox, v3Pos, aAxis, m_Planes[n]) == PLANE_NEGATIVE)
			return false;
	}

	return true;
}

