// (C)2005 S2 Games
// c_boundingcone.cpp
//
// Bounding cone used to light camera construction
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_boundingcone.h"
//=============================================================================

/*====================
  CBoundingCone::CBoundingCone
  ====================*/
CBoundingCone::CBoundingCone(const vector<CVec3f> &vPoints, const CVec3f &vApex, const CVec3f &vDirection)
{
    m_fNear = FAR_AWAY;
    m_fFar = 0.0f;
    m_fFov = 0.0f;
    m_vApex = vApex;
    m_vDirection = Normalize(vDirection);

    for (vector<CVec3f>::const_iterator it = vPoints.begin(); it != vPoints.end(); ++it)
    {
        CVec3f  vPoint(*it - m_vApex);

        float fAngle = acos(DotProduct(m_vDirection, Normalize(vPoint)));
        float fDistance = DotProduct(m_vDirection, vPoint);

        if (fAngle > m_fFov)
            m_fFov = fAngle;

        if (fDistance < m_fNear)
            m_fNear = fDistance;

        if (fDistance > m_fFar)
            m_fFar = fDistance;
    }
}

#if 1
/*====================
  CBoundingCone::CBoundingCone
  ====================*/
CBoundingCone::CBoundingCone(const vector<CVec3f> &vPoints, const CVec3f &vApex) :
m_vApex(vApex)
{
    float   fBestFov = FAR_AWAY;
    CVec3f  vBestDirection(0.0f, 0.0f, 0.0f);
    float   fBestFar = 0.0f;
    float   fBestNear = 0.0f;

    for (vector<CVec3f>::const_iterator it1 = vPoints.begin(); it1 != vPoints.end(); ++it1)
    {
        CVec3f  vDirection1(Normalize(*it1 - m_vApex));

        for (vector<CVec3f>::const_iterator it2 = it1 + 1; it2 != vPoints.end(); ++it2)
        {
            CVec3f  vDirection2(Normalize(*it2 - m_vApex));
            CVec3f  vDirection(Normalize(vDirection1 + vDirection2));

            float fFov = 0.0f;
            float fNear = FAR_AWAY;
            float fFar = 0.0f;

            for (vector<CVec3f>::const_iterator it = vPoints.begin(); it != vPoints.end(); ++it)
            {
                CVec3f  vPoint(*it - m_vApex);

                float fAngle = acos(DotProduct(vDirection, Normalize(vPoint)));

                if (fAngle > fFov)
                    fFov = fAngle;

                if (fFov > fBestFov)
                    break; // stop now cause we're already not as good as the best one

                float fDistance = DotProduct(vDirection, vPoint);

                if (fDistance < fNear)
                    fNear = fDistance;

                if (fDistance > fFar)
                    fFar = fDistance;
            }

            if (fFov < fBestFov)
            {
                fBestFov = fFov;
                vBestDirection = vDirection;
                fBestFar = fFar;
                fBestNear = fNear;
            }
        }
    }

    //assert(fBestFov != FAR_AWAY);

    m_fFov = fBestFov;
    m_vDirection = vBestDirection;
    m_fFar = fBestFar;
    m_fNear = fBestNear;
}
#else
/*====================
  CBoundingCone::CBoundingCone
  ====================*/
CBoundingCone::CBoundingCone(const vector<CVec3f> &vPoints, const CVec3f &vApex) :
m_vApex(vApex)
{
    CVec3f  vDirection(0.0f, 0.0f, 0.0f);

    for (vector<CVec3f>::const_iterator it = vPoints.begin(); it != vPoints.end(); ++it)
    {
        vDirection += *it - m_vApex;
    }

    vDirection.Normalize();

    float fFov = 0.0f;
    float fNear = FAR_AWAY;
    float fFar = 0.0f;

    for (vector<CVec3f>::const_iterator it = vPoints.begin(); it != vPoints.end(); ++it)
    {
        CVec3f  vPoint(*it - m_vApex);

        float fAngle = acos(DotProduct(vDirection, Normalize(vPoint)));

        if (fAngle > fFov)
            fFov = fAngle;

        float fDistance = DotProduct(vDirection, vPoint);

        if (fDistance < fNear)
            fNear = fDistance;

        if (fDistance > fFar)
            fFar = fDistance;
    }


    m_fFov = fFov;
    m_vDirection = vDirection;
    m_fFar = fFar;
    m_fNear = fNear;
}
#endif



