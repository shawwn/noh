// (C)2005 S2 Games
// c_plane.h
//
// A plane in the form Ax + By + Cz = D
//=============================================================================
#ifndef __C_PLANE_H__
#define __C_PLANE_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_vec3.h"
#include "c_matrix3x3.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const float PLANE_EPSILON = 0.002f;

enum EPlaneTest
{
    PLANE_POSITIVE = 0,
    PLANE_NEGATIVE,
    PLANE_INTERSECTS
};
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CAxis;
//=============================================================================

//=============================================================================
// CPlane
//=============================================================================
class CPlane
{
public:
    CPlane() {}
    CPlane(float x, float y, float z, float d) : v3Normal(x, y, z), fDist(d) {};
    CPlane(const CVec3f &_v3Normal, float _fDist) : v3Normal(_v3Normal), fDist(_fDist)  {}

    CPlane(const CVec3f &a, const CVec3f &b, const CVec3f &c)
    {
        CalcPlane(a, b, c);
    }

    CPlane(const CVec3f &a, const CVec3f &b, const CVec3f &c, bool bNormalized)
    {
        if (bNormalized)
            CalcPlaneNormalized(a, b, c);
        else
            CalcPlane(a, b, c);

    }

    CPlane(const CVec3f &n, const CVec3f &p)
    {
        v3Normal = n;
        fDist = DotProduct(n, p);
    }

    bool    operator==(const CPlane &b) const   { return v3Normal == b.v3Normal && fDist == b.fDist; }
    bool    operator!=(const CPlane &b) const   { return v3Normal != b.v3Normal || fDist != b.fDist; }

    CPlane  operator-() const                   { return CPlane(-v3Normal.x, -v3Normal.y, -v3Normal.z, -fDist); }

    void    Set(float x, float y, float z, float _fDist)
    {
        v3Normal.Set(x, y, z);
        this->fDist = _fDist;
    }

    float   Normalize()
    {
        float fLength = v3Normal.Normalize();
        fDist /= fLength;

        return fLength;
    }

    void    CalcPlane(const CVec3f &a, const CVec3f &b, const CVec3f &c)
    {
        v3Normal.x = a.y*(b.z - c.z) + b.y*(c.z - a.z) + c.y*(a.z - b.z);
        v3Normal.y = a.z*(b.x - c.x) + b.z*(c.x - a.x) + c.z*(a.x - b.x);
        v3Normal.z = a.x*(b.y - c.y) + b.x*(c.y - a.y) + c.x*(a.y - b.y);

        fDist = a.x*(b.y * c.z - c.y * b.z) + b.x*(c.y * a.z - a.y * c.z) + c.x*(a.y * b.z - b.y * a.z);
    }

    void    CalcPlaneNormalized(const CVec3f &a, const CVec3f &b, const CVec3f &c)
    {
        v3Normal = CrossProduct(b - a, c - a);
        v3Normal.Normalize();
        fDist = DotProduct(v3Normal, a);
    }

    void    Scale(float fScale) { fDist = DotProduct(v3Normal * (fDist * fScale), v3Normal); }

    float   GetHeight(float fX, float fY) const  // assumes the plane is normalized
    {
        // Ax + By + Cz = D --> Cz = -Ax - By + D --> z = (-Ax - By + D)/C
        return (-v3Normal.x * fX - v3Normal.y * fY + fDist) / v3Normal.z;
    }

    CVec3f  Project(const CVec2f &v2Pos) const  // assumes the plane is normalized
    {
        // Ax + By + Cz = D --> Cz = -Ax - By + D --> z = (-Ax - By + D)/C
        return CVec3f(v2Pos.x, v2Pos.y, (-v3Normal.x * v2Pos.x - v3Normal.y * v2Pos.y + fDist) / v3Normal.z);
    }

    EPlaneTest  Side(const CVec3f &v) const
    {
        float fDistance = DotProduct(v3Normal, v) - fDist;

        if (fDistance < -PLANE_EPSILON)
            return PLANE_NEGATIVE;
        else if (fDistance > PLANE_EPSILON)
            return PLANE_POSITIVE;
        else
            return PLANE_INTERSECTS;
    }

    EPlaneTest  Side(const vector<CVec3f> &vPoints) const
    {
        int iInside(0);
        int iOutside(0);

        for (vector<CVec3f>::const_iterator it(vPoints.begin()); it != vPoints.end(); ++it)
        {
            float fDistance = DotProduct(v3Normal, *it) - fDist;

            if (fDistance < -PLANE_EPSILON)
            {
                if (iOutside > 0)
                    return PLANE_INTERSECTS;

                ++iInside;
            }
            else if (fDistance > PLANE_EPSILON)
            {
                if (iInside > 0)
                    return PLANE_INTERSECTS;

                ++iOutside;
            }
        }

        if (iInside > 0)
            return PLANE_NEGATIVE;
        else if (iOutside > 0)
            return PLANE_POSITIVE;
        else
            return PLANE_INTERSECTS;
    }

    EPlaneTest  Side(const CBBoxf &bbBox) const
    {
        const CVec3f &vMin(bbBox.GetMin());
        const CVec3f &vMax(bbBox.GetMax());

        CVec3f  v3Neg, v3Pos; // negative and positive vertices

        if (v3Normal.x > 0)
        {
            v3Pos.x = vMax.x;
            v3Neg.x = vMin.x;
        }
        else
        {
            v3Pos.x = vMin.x;
            v3Neg.x = vMax.x;
        }

        if (v3Normal.y > 0)
        {
            v3Pos.y = vMax.y;
            v3Neg.y = vMin.y;
        }
        else
        {
            v3Pos.y = vMin.y;
            v3Neg.y = vMax.y;
        }

        if (v3Normal.z > 0)
        {
            v3Pos.z = vMax.z;
            v3Neg.z = vMin.z;
        }
        else
        {
            v3Pos.z = vMin.z;
            v3Neg.z = vMax.z;
        }

        if (DotProduct(v3Normal, v3Pos) - fDist < 0)   // outside plane
            return PLANE_POSITIVE;
        else if (DotProduct(v3Normal, v3Neg) - fDist > 0)    // inside plane
            return PLANE_NEGATIVE;
        else
            return PLANE_INTERSECTS;   // intersects plane
    }

    bool    IsInside(const vector<CVec3f> &vPoints) const
    {
        for (vector<CVec3f>::const_iterator it(vPoints.begin()); it != vPoints.end(); ++it)
        {
            float fDistance = DotProduct(v3Normal, *it) - fDist;

            if (fDistance > PLANE_EPSILON)
                return false;
        }

        return true;
    }

    CVec3f  NearestPoint(const CVec3f &v3p) const
    {
        // Assumes the plane is normalized
        return v3p - v3Normal * (DotProduct(v3Normal, v3p) - fDist);
    }

    float   Distance(const CVec3f &v) const
    {
        // Assumes the plane is normalized
        return DotProduct(v3Normal, v) - fDist;
    }

    bool    IsValid() const             { return v3Normal.IsValid() && (v3Normal.x != 0.0f || v3Normal.y != 0.0f || v3Normal.z != 0.0f) && _finite(fDist); }

    K2_API void Transform(const CVec3f &pos, const class CAxis &axis, float fScale);

    void    Transform(const CVec3f &v3Pos)  { fDist += DotProduct(v3Pos, v3Normal); }

    // Public for easy access
    CVec3f  v3Normal;
    float   fDist;
};
//=============================================================================

/*====================
  Intersection
  ====================*/
inline
CVec3f Intersection(const CPlane &a, const CPlane &b, const CPlane &c)
{
    CMatrix3x3f M(a.v3Normal.x, b.v3Normal.y, c.v3Normal.z,
                  a.v3Normal.x, b.v3Normal.y, c.v3Normal.z,
                  a.v3Normal.x, b.v3Normal.y, c.v3Normal.z);

    CVec3f      D(a.fDist, b.fDist, c.fDist);

    if (M.Invert())
        return Multiply(M, D);
    else
        return CVec3f(0.0f, 0.0f, 0.0f);
}


/*====================
  Project

  Assumes the plane is normalized
  ====================*/
inline
CVec3f Project(const CPlane &a, const CVec3f &b)
{
    return CVec3f(b - a.v3Normal * a.Distance(b));
}


/*====================
  Reflect

  Assumes the plane is normalized
  ====================*/
inline
CVec3f Reflect(const CVec3f &v, const CPlane &p)
{
    return CVec3f(v - p.v3Normal * p.Distance(v) * 2.0f);
}



#endif  //__C_PLANE_H__
