// (C)2005 S2 Games
// c_orthofrustum.cpp
//
// Othro Frustum class
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_orthofrustum.h"
//=============================================================================

/*====================
  COrthoFrustum::COrthoFrustum
  ====================*/
COrthoFrustum::COrthoFrustum(const vector<CVec3f> &vPoints, const CVec3f &vOrigin, const CVec3f &vDirection)
{
}


/*====================
  COrthoFrustum::COrthoFrustum

  Calculates the frustum of best fit from a set of points and the origin of the frustum
  ====================*/
COrthoFrustum::COrthoFrustum(const vector<CVec3f> &vPoints, const CVec3f &vDirection, float fSlideBack) :
m_vOrigin(0.0f, 0.0f, 0.0f),
m_fWidth(0.0f),
m_fHeight(0.0f),
m_fNear(0.1f),
m_fFar(100.0f)
{
    CVec3f vCenter(0.0f, 0.0f, 0.0f);

    // Calculate the near and far plane
    for (vector<CVec3f>::const_iterator it = vPoints.begin(); it != vPoints.end(); ++it)
        vCenter += *it;

    vCenter /= float(vPoints.size());

    m_vOrigin = vCenter;

    CVec3f vFrustumForward(vDirection);

    // vFrustumRight is a ray of the line between the intersection of the planes p1 and p2
    CVec3f vFrustumRight(Normalize(CrossProduct(vFrustumForward, CVec3f(0.0f, 0.0f, 1.0f))));

    // Orthogonalize forward and right
    CVec3f v3Proj(vFrustumRight * DotProduct(vFrustumForward, vFrustumRight));
    vFrustumRight -= v3Proj;
    vFrustumRight.Normalize();

    CVec3f vFrustumUp(Normalize(CrossProduct(vFrustumRight, vFrustumForward)));

    m_Axis.Set(vFrustumRight, vFrustumForward, vFrustumUp);

    // Calculate the width
    CPlane plWidthPlane(vFrustumRight, vCenter);

    float fWorstWidth = -FAR_AWAY;

    for (vector<CVec3f>::const_iterator it = vPoints.begin(); it != vPoints.end(); ++it)
    {
        float fDistance = ABS(plWidthPlane.Distance(*it));

        if (fDistance > fWorstWidth)
            fWorstWidth = fDistance;
    }
    m_fWidth = 2.0f * fWorstWidth;

    // Calculate the height
    CPlane plHeightPlane(vFrustumUp, vCenter);

    float fWorstHeight = -FAR_AWAY;

    for (vector<CVec3f>::const_iterator it = vPoints.begin(); it != vPoints.end(); ++it)
    {
        float fDistance = ABS(plHeightPlane.Distance(*it));

        if (fDistance > fWorstHeight)
            fWorstHeight = fDistance;
    }
    m_fHeight = 2.0f * fWorstHeight;

    CPlane plViewPlane(vFrustumForward, vCenter);

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

    m_vOrigin -= vDirection * fSlideBack;
    m_fNear = fBestNear;

    m_fFar = fBestFar + fSlideBack;

    CalcPlanes();
}


/*====================
  COrthoFrustum::CalcPlanes
  ====================*/
void    COrthoFrustum::CalcPlanes()
{
    const CVec3f &v3ViewRight(m_Axis[RIGHT]);
    const CVec3f &v3ViewDir(m_Axis[FORWARD]);
    const CVec3f &v3ViewUp(m_Axis[UP]);

    // Top plane
    m_Planes[0].v3Normal = -v3ViewUp;
    m_Planes[0].fDist = DotProduct(-v3ViewUp, M_PointOnLine(m_vOrigin, v3ViewUp, m_fHeight / 2.0f));

    // Left plane
    m_Planes[1].v3Normal = v3ViewRight;
    m_Planes[1].fDist = DotProduct(v3ViewRight, M_PointOnLine(m_vOrigin, v3ViewRight, -m_fWidth / 2.0f));

    // Bottom plane
    m_Planes[2].v3Normal = v3ViewUp;
    m_Planes[2].fDist = DotProduct(v3ViewUp, M_PointOnLine(m_vOrigin, v3ViewUp, -m_fHeight / 2.0f));

    // Right plane
    m_Planes[3].v3Normal = -v3ViewRight;
    m_Planes[3].fDist = DotProduct(-v3ViewRight, M_PointOnLine(m_vOrigin, v3ViewRight, m_fWidth / 2.0f));

    // Far plane
    m_Planes[4].v3Normal = -v3ViewDir;
    m_Planes[4].fDist = DotProduct(-v3ViewDir, M_PointOnLine(m_vOrigin, v3ViewDir, m_fFar));

    // Near plane
    m_Planes[5].v3Normal = v3ViewDir;
    m_Planes[5].fDist = DotProduct(v3ViewDir, M_PointOnLine(m_vOrigin, v3ViewDir, m_fNear));

    // TODO: Compute frustum bounding box
    //m_bbBounds.Clear();
}


/*====================
  COrthoFrustum::Touches

  Test if frustum touches an oriented bounding box
  ====================*/
bool    COrthoFrustum::Touches(const CBBoxf &bb, const CVec3f &v3Pos, const CAxis &axis) const
{
    for (int n(0); n < 6; ++n)
    {
        if (M_OBBOnPlaneSide(bb, v3Pos, axis, m_Planes[n]) == PLANE_NEGATIVE)
            return false;
    }

    return true;
}


/*====================
  COrthoFrustum::Touches

  Test if frustum touches a point
  ====================*/
bool    COrthoFrustum::Touches(const CVec3f &vPoint) const
{
    for (int n = 0; n < 6; ++n)
    {
        if (m_Planes[n].Side(vPoint) == PLANE_NEGATIVE)
            return false;
    }

    return true;
}
