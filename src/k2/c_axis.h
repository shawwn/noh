// (C)2005 S2 Games
// c_axis.h
//=============================================================================
#ifndef __C_AXIS_H__
#define __C_AXIS_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"

#include "k2_mathlib.h"
#include "c_vec3.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define CAxis_cast(a) (*(CAxis*)(&((a)[0])))

#define AXIS_IDENTITY   CAxis(0.0f, 0.0f, 0.0f)
//=============================================================================

//=============================================================================
// CAxis
//=============================================================================
class CAxis
{
private:
    CVec3f m_Axis[3];

public:
    ~CAxis()
    {}

    // Constructors
    CAxis() {}

    CAxis(const CVec3f &v3Right, const CVec3f &v3Forward, const CVec3f &v3Up)
    {
        m_Axis[RIGHT] = v3Right;
        m_Axis[FORWARD] = v3Forward;
        m_Axis[UP] = v3Up;
    }

    CAxis(float fPitch, float fRoll, float fYaw)    { Set(fPitch, fRoll, fYaw); }
    explicit CAxis(const CVec3f &v3Angles)          { Set(v3Angles); }

    // Accessors
    const CVec3f &operator[](int i) const   { return m_Axis[i]; };
    CVec3f &operator[](int i)               { return m_Axis[i]; };

    CVec3f&         Right()                 { return m_Axis[RIGHT]; }
    CVec3f&         Forward()               { return m_Axis[FORWARD]; }
    CVec3f&         Up()                    { return m_Axis[UP]; }

    const CVec3f&   Right() const           { return m_Axis[RIGHT]; }
    const CVec3f&   Forward() const         { return m_Axis[FORWARD]; }
    const CVec3f&   Up() const              { return m_Axis[UP]; }

    CVec2f          Right2d() const         { CVec2f v2Right(m_Axis[RIGHT].x, m_Axis[RIGHT].y); v2Right.Normalize(); return v2Right; }
    CVec2f          Forward2d() const       { CVec2f v2Fwd(m_Axis[FORWARD].x, m_Axis[FORWARD].y); v2Fwd.Normalize(); return v2Fwd; }

    // Utilities
    void Set(const CVec3f &v3Right, const CVec3f &v3Forward, const CVec3f &v3Up)
    {
        m_Axis[RIGHT] = v3Right;
        m_Axis[FORWARD] = v3Forward;
        m_Axis[UP] = v3Up;
    }

    void Set(float pitch, float roll, float yaw)
    {
        if (pitch == 0.0f && roll == 0.0f && yaw == 0.0f)
        {
            m_Axis[0].x = 1.0f;
            m_Axis[0].y = 0.0f;
            m_Axis[0].z = 0.0f;

            m_Axis[1].x = 0.0f;
            m_Axis[1].y = 1.0f;
            m_Axis[1].z = 0.0f;

            m_Axis[2].x = 0.0f;
            m_Axis[2].y = 0.0f;
            m_Axis[2].z = 1.0f;
            return;
        }

        float sr, cr, sp, cp, sy, cy;

        M_SinCos(DEG2RAD(roll), sr, cr);
        M_SinCos(DEG2RAD(pitch), sp, cp);
        M_SinCos(DEG2RAD(yaw), sy, cy);

        float sr_sp = sr * sp;
        float neg_cr_sp = -cr * sp;

        m_Axis[0].x = cr * cy + sr_sp * -sy;
        m_Axis[0].y = cr * sy + sr_sp * cy;
        m_Axis[0].z = -sr * cp;

        m_Axis[1].x = cp * -sy;
        m_Axis[1].y = cp * cy;
        m_Axis[1].z = sp;

        m_Axis[2].x = sr * cy + neg_cr_sp * -sy;
        m_Axis[2].y = sr * sy + neg_cr_sp * cy;
        m_Axis[2].z = cr * cp;
    }

    void    Set(const CVec3f &angles)
    {
        Set(angles[PITCH], angles[ROLL], angles[YAW]);
    }

    void    SetFromForwardVec(const CVec3f &vec)
    {
        m_Axis[FORWARD] = vec;
        m_Axis[FORWARD].Normalize();

        m_Axis[UP].Set(0.0f, 0.0f, 1.0f);
        m_Axis[RIGHT] = CrossProduct(m_Axis[FORWARD], m_Axis[UP]);
        if (m_Axis[RIGHT].Length() == 0.0f)
            m_Axis[RIGHT].Set(1.0f, 0.0f, 0.0f);
        m_Axis[RIGHT].Normalize();

        m_Axis[UP] = CrossProduct(m_Axis[RIGHT], m_Axis[FORWARD]);
        m_Axis[UP].Normalize();
    }

    void Clear()
    {
        m_Axis[RIGHT].Set(1.0f, 0.0f, 0.0f);
        m_Axis[FORWARD].Set(0.0f, 1.0f, 0.0f);
        m_Axis[UP].Set(0.0f, 0.0f, 1.0f);
    }

    CAxis   operator*(const CAxis &b) const
    {
        CAxis c;
        c.m_Axis[X][X] = m_Axis[X][X] * b.m_Axis[X][X] + m_Axis[Y][X] * b.m_Axis[X][Y] + m_Axis[Z][X] * b.m_Axis[X][Z];
        c.m_Axis[X][Y] = m_Axis[X][Y] * b.m_Axis[X][X] + m_Axis[Y][Y] * b.m_Axis[X][Y] + m_Axis[Z][Y] * b.m_Axis[X][Z];
        c.m_Axis[X][Z] = m_Axis[X][Z] * b.m_Axis[X][X] + m_Axis[Y][Z] * b.m_Axis[X][Y] + m_Axis[Z][Z] * b.m_Axis[X][Z];

        c.m_Axis[Y][X] = m_Axis[X][X] * b.m_Axis[Y][X] + m_Axis[Y][X] * b.m_Axis[Y][Y] + m_Axis[Z][X] * b.m_Axis[Y][Z];
        c.m_Axis[Y][Y] = m_Axis[X][Y] * b.m_Axis[Y][X] + m_Axis[Y][Y] * b.m_Axis[Y][Y] + m_Axis[Z][Y] * b.m_Axis[Y][Z];
        c.m_Axis[Y][Z] = m_Axis[X][Z] * b.m_Axis[Y][X] + m_Axis[Y][Z] * b.m_Axis[Y][Y] + m_Axis[Z][Z] * b.m_Axis[Y][Z];

        c.m_Axis[Z][X] = m_Axis[X][X] * b.m_Axis[Z][X] + m_Axis[Y][X] * b.m_Axis[Z][Y] + m_Axis[Z][X] * b.m_Axis[Z][Z];
        c.m_Axis[Z][Y] = m_Axis[X][Y] * b.m_Axis[Z][X] + m_Axis[Y][Y] * b.m_Axis[Z][Y] + m_Axis[Z][Y] * b.m_Axis[Z][Z];
        c.m_Axis[Z][Z] = m_Axis[X][Z] * b.m_Axis[Z][X] + m_Axis[Y][Z] * b.m_Axis[Z][Y] + m_Axis[Z][Z] * b.m_Axis[Z][Z];

        return c;
    }
};
//=============================================================================

/*====================
  TransformPoint
  ====================*/
inline
CVec3f  TransformPoint(const CVec3f &point, const CAxis &axis)
{
    CVec3f result(
        point.x * axis.Right().x + point.y * axis.Forward().x + point.z * axis.Up().x,
        point.x * axis.Right().y + point.y * axis.Forward().y + point.z * axis.Up().y,
        point.x * axis.Right().z + point.y * axis.Forward().z + point.z * axis.Up().z);
    return result;
}


/*====================
  TransformPoint
  ====================*/
inline
CVec3f  TransformPoint(const CVec3f &point, const CAxis &axis, const CVec3f &pos)
{
    CVec3f result(
        point.x * axis.Right().x + point.y * axis.Forward().x + point.z * axis.Up().x,
        point.x * axis.Right().y + point.y * axis.Forward().y + point.z * axis.Up().y,
        point.x * axis.Right().z + point.y * axis.Forward().z + point.z * axis.Up().z);
    result += pos;
    return result;
}

inline CVec3f   operator*(const CAxis &axis, const CVec3f &v3)  { return TransformPoint(v3, axis); }
inline CVec3f   operator*(const CVec3f &v3, const CAxis &axis)  { return TransformPoint(v3, axis); }


/*====================
  TransformPoint
  ====================*/
inline
CVec3f  TransformPoint(const CVec3f &point, const CAxis &axis, const CVec3f &pos, float fScale)
{
    CVec3f v3ScaledPoint(point * fScale);

    CVec3f result(
        v3ScaledPoint.x * axis.Right().x + v3ScaledPoint.y * axis.Forward().x + v3ScaledPoint.z * axis.Up().x,
        v3ScaledPoint.x * axis.Right().y + v3ScaledPoint.y * axis.Forward().y + v3ScaledPoint.z * axis.Up().y,
        v3ScaledPoint.x * axis.Right().z + v3ScaledPoint.y * axis.Forward().z + v3ScaledPoint.z * axis.Up().z);
    result += pos;
    return result;
}


/*====================
  M_TransformPointInverse
  ====================*/
inline
CVec3f  TransformPointInverse(const CVec3f &point, const CVec3f &pos, const CAxis &axis)
{
    CVec3f temp(point - pos);

    return CVec3f(DotProduct(temp, axis[0]), DotProduct(temp, axis[1]), DotProduct(temp, axis[2]));
}


/*====================
  TransformPointInverse
  ====================*/
inline
CVec3f  TransformPointInverse(const CVec3f &point, const CAxis &axis)
{
    return CVec3f(DotProduct(point, axis[0]), DotProduct(point, axis[1]), DotProduct(point, axis[2]));
}



/*====================
  GetAxisFromForwardVec
  ====================*/
inline
CAxis   GetAxisFromForwardVec(const CVec3f &vec)
{
    CVec3f  forward(vec);
    forward.Normalize();

    CVec3f  up(0.0f, 0.0f, 1.0f);
    CVec3f  right(CrossProduct(forward, up));
    if (right.LengthSq() == 0.0f)
        right.Set(1.0f, 0.0f, 0.0f);
    right.Normalize();

    up = CrossProduct(right, forward);
    up.Normalize();

    return CAxis(right, forward, up);
}


/*====================
  GetAxisFromUpVec
  ====================*/
inline
CAxis   GetAxisFromUpVec(const CVec3f &vec)
{
    CVec3f  up(vec);
    up.Normalize();

    CVec3f  forward(1.0f, 0.0f, 0.0f);
    CVec3f  right(CrossProduct(forward, up));
    if (right.LengthSq() == 0.0f)
    {
        CVec3f  forward2(0.0f, 1.0f, 0.0f);
        right = CrossProduct(forward2, up);
        if (right.LengthSq() == 0.0f)
            right.Set(1.0f, 0.0f, 0.0f);
    }
    right.Normalize();

    forward = CrossProduct(up, right);
    forward.Normalize();

    return CAxis(right, forward, up);
}
//=============================================================================
#endif //__C_AXIS_H__
