// (C)2005 S2 Games
// math_inlines.h
//=============================================================================
#ifndef __MATH_INLINES_H__
#define __MATH_INLINES_H__

/*====================
  M_Identity
  ====================*/
inline
void    M_Identity(matrix43_t *t)
{
    t->axis[0][0] = 1;
    t->axis[0][1] = 0;
    t->axis[0][2] = 0;
    t->axis[1][0] = 0;
    t->axis[1][1] = 1;
    t->axis[1][2] = 0;
    t->axis[2][0] = 0;
    t->axis[2][1] = 0;
    t->axis[2][2] = 1;
    t->pos[0] = 0;
    t->pos[1] = 0;
    t->pos[2] = 0;
}


/*====================
  M_MultiplyMatrix
  ====================*/
inline
void    M_MultiplyMatrix(const matrix43_t *a, const matrix43_t *b, matrix43_t *out)
{
    out->axis[0][0] = a->axis[0][0] * b->axis[0][0] + a->axis[1][0] * b->axis[0][1] + a->axis[2][0] * b->axis[0][2];
    out->axis[0][1] = a->axis[0][1] * b->axis[0][0] + a->axis[1][1] * b->axis[0][1] + a->axis[2][1] * b->axis[0][2];
    out->axis[0][2] = a->axis[0][2] * b->axis[0][0] + a->axis[1][2] * b->axis[0][1] + a->axis[2][2] * b->axis[0][2];

    out->axis[1][0] = a->axis[0][0] * b->axis[1][0] + a->axis[1][0] * b->axis[1][1] + a->axis[2][0] * b->axis[1][2];
    out->axis[1][1] = a->axis[0][1] * b->axis[1][0] + a->axis[1][1] * b->axis[1][1] + a->axis[2][1] * b->axis[1][2];
    out->axis[1][2] = a->axis[0][2] * b->axis[1][0] + a->axis[1][2] * b->axis[1][1] + a->axis[2][2] * b->axis[1][2];

    out->axis[2][0] = a->axis[0][0] * b->axis[2][0] + a->axis[1][0] * b->axis[2][1] + a->axis[2][0] * b->axis[2][2];
    out->axis[2][1] = a->axis[0][1] * b->axis[2][0] + a->axis[1][1] * b->axis[2][1] + a->axis[2][1] * b->axis[2][2];
    out->axis[2][2] = a->axis[0][2] * b->axis[2][0] + a->axis[1][2] * b->axis[2][1] + a->axis[2][2] * b->axis[2][2];

    out->pos[0] = a->axis[0][0] * b->pos[0] + a->axis[1][0] * b->pos[1] + a->axis[2][0] * b->pos[2] + a->pos[0];
    out->pos[1] = a->axis[0][1] * b->pos[0] + a->axis[1][1] * b->pos[1] + a->axis[2][1] * b->pos[2] + a->pos[1];
    out->pos[2] = a->axis[0][2] * b->pos[0] + a->axis[1][2] * b->pos[1] + a->axis[2][2] * b->pos[2] + a->pos[2];
}


/*====================
  M_TransformPoint

  this does the same thing as multiplying a 4x3 matrix by a vector
  ====================*/
inline
void    M_TransformPoint(const vec3_t point, const vec3_t pos, const vec3_t axis[3], vec3_t out)
{
    vec3_t tmp;

    tmp[0] = point[0] * axis[0][0] + point[1] * axis[1][0] + point[2] * axis[2][0];  //M_DotProduct(point, axis[0]);
    tmp[1] = point[0] * axis[0][1] + point[1] * axis[1][1] + point[2] * axis[2][1];  //M_DotProduct(point, axis[1]);
    tmp[2] = point[0] * axis[0][2] + point[1] * axis[1][2] + point[2] * axis[2][2];  //M_DotProduct(point, axis[2]);

    out[0] = tmp[0] + pos[0];
    out[1] = tmp[1] + pos[1];
    out[2] = tmp[2] + pos[2];
}


/*====================
  M_TransformPointInverse

  inversely transforms the point
  ====================*/
inline
void    M_TransformPointInverse(const vec3_t point, const vec3_t pos, const vec3_t axis[3], vec3_t out)
{
    vec3_t tmp;

    tmp[0] = point[0] - pos[0];
    tmp[1] = point[1] - pos[1];
    tmp[2] = point[2] - pos[2];

    out[0] = M_DotProduct(tmp, axis[0]);
    out[1] = M_DotProduct(tmp, axis[1]);
    out[2] = M_DotProduct(tmp, axis[2]);
}


/*====================
  M_RotatePoint
  ====================*/
inline
void    M_RotatePoint(const vec3_t point, const vec3_t axis[3], vec3_t out)
{
    vec3_t tmp;

    tmp[0] = point[0] * axis[0][0] + point[1] * axis[1][0] + point[2] * axis[2][0];  //M_DotProduct(point, axis[0]);
    tmp[1] = point[0] * axis[0][1] + point[1] * axis[1][1] + point[2] * axis[2][1];  //M_DotProduct(point, axis[1]);
    tmp[2] = point[0] * axis[0][2] + point[1] * axis[1][2] + point[2] * axis[2][2];  //M_DotProduct(point, axis[2]);

    M_CopyVec3(tmp, out);
}


/*====================
  M_PointInBounds
  ====================*/
inline
bool    M_PointInBounds(const CVec3f &point, const CVec3f &bmin, const CVec3f &bmax)
{
    if ((point.x < bmin.x) || (point.x > bmax.x) ||
        (point.y < bmin.y) || (point.y > bmax.y) ||
        (point.z < bmin.z) || (point.z > bmax.z))
        return false;

    return true;
}


/*====================
  M_ClearBounds
  ====================*/
inline
void    M_ClearBounds(vec3_t bmin, vec3_t bmax)
{
    bmin[0] = 999999;
    bmin[1] = 999999;
    bmin[2] = 999999;
    bmax[0] = -999999;
    bmax[1] = -999999;
    bmax[2] = -999999;
}

inline
void    M_ClearBounds(CVec3f &bmin, CVec3f &bmax)
{
    bmin.x = 999999;
    bmin.y = 999999;
    bmin.z = 999999;
    bmax.x = -999999;
    bmax.y = -999999;
    bmax.z = -999999;
}


/*====================
  M_ClearRect
  ====================*/
inline
void    M_ClearRect(vec3_t bmin, vec3_t bmax)
{
    bmin[0] = 999999;
    bmin[1] = 999999;
    bmax[0] = -999999;
    bmax[1] = -999999;
}


/*====================
  M_AddPointToBounds
  ====================*/
inline
void    M_AddPointToBounds(const vec3_t point, vec3_t bmin, vec3_t bmax)
{
    if (point[0] < bmin[0])
        bmin[0] = point[0];
    if (point[1] < bmin[1])
        bmin[1] = point[1];
    if (point[2] < bmin[2])
        bmin[2] = point[2];

    if (point[0] > bmax[0])
        bmax[0] = point[0];
    if (point[1] > bmax[1])
        bmax[1] = point[1];
    if (point[2] > bmax[2])
        bmax[2] = point[2];
}


/*====================
  M_AddPointToRect
  ====================*/
inline
void    M_AddPointToRect(const vec2_t point, vec2_t bmin, vec2_t bmax)
{
    if (point[0] < bmin[0])
        bmin[0] = point[0];
    if (point[1] < bmin[1])
        bmin[1] = point[1];

    if (point[0] > bmax[0])
        bmax[0] = point[0];
    if (point[1] > bmax[1])
        bmax[1] = point[1];
}


/*====================
  M_RectInRect
  ====================*/
inline
bool    M_RectInRect(const vec2_t bmin1, const vec2_t bmax1, const vec2_t bmin2, const vec2_t bmax2)
{
    if (bmin1[0] >= bmin2[0] && bmax1[0] <= bmax2[0])
        if (bmin1[1] >= bmin2[1] && bmax1[1] <= bmax2[1])
            return true;

    return false;
}

inline 
bool    M_RectInRect(const CVec3f &bmin1, const CVec3f &bmax1, const CVec3f &bmin2, const CVec3f &bmax2)
{
    if (bmin1.x >= bmin2.x && bmax1.x <= bmax2.x)
        if (bmin1.y >= bmin2.y && bmax1.y <= bmax2.y)
            return true;

    return false;
}


/*====================
  M_CalcBoxExtents
  ====================*/
inline
void    M_CalcBoxExtents(const vec3_t bmin, const vec3_t bmax, vec3_t pos, vec3_t ext)
{
    ext[0] = ((bmax[0] - bmin[0]) * 0.5f);
    ext[1] = ((bmax[1] - bmin[1]) * 0.5f);
    ext[2] = ((bmax[2] - bmin[2]) * 0.5f);

    pos[0] = bmin[0] + ext[0];
    pos[1] = bmin[1] + ext[1];
    pos[2] = bmin[2] + ext[2];

}

inline
void    M_CalcBoxExtents(const CVec3f &bmin, const CVec3f &bmax, CVec3f &pos, CVec3f &ext)
{
    ext = (bmax - bmin) * 0.5f;
    pos = bmin + ext;
}


/*====================
  M_SetVec3
  ====================*/
inline
void    M_SetVec3(vec3_t out, float x, float y, float z)
{
    out[0] = x;
    out[1] = y;
    out[2] = z;
}


/*====================
  M_SetVec4
  ====================*/
inline
void    M_SetVec4(vec4_t out, float x, float y, float z, float w)
{
    out[0] = x;
    out[1] = y;
    out[2] = z;
    out[3] = w;
}


/*====================
  M_SetVec2
  ====================*/
inline
void    M_SetVec2(vec2_t out, float x, float y)
{
    out[0] = x;
    out[1] = y;
}


/*====================
  M_MultVec3
  ====================*/
inline
void M_MultVec3(const vec3_t a, float b, vec3_t out)
{
    out[0] = a[0] * b;
    out[1] = a[1] * b;
    out[2] = a[2] * b;
}


/*====================
  M_CopyVec2
  ====================*/
inline
void    M_CopyVec2(const vec2_t in, vec2_t out)
{
    out[0] = in[0];
    out[1] = in[1];
}


/*====================
  M_CopyVec3
  ====================*/
inline
void    M_CopyVec3(const vec3_t in, vec3_t out)
{
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
}


/*====================
  M_CopyVec4
  ====================*/
inline
void    M_CopyVec4(const vec4_t in, vec4_t out)
{
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
    out[3] = in[3];
}


/*====================
  M_CrossProduct
  ====================*/
inline
void    M_CrossProduct(const vec3_t a, const vec3_t b, vec3_t out)
{
    out[0] = a[1]*b[2] - a[2]*b[1];
    out[1] = a[2]*b[0] - a[0]*b[2];
    out[2] = a[0]*b[1] - a[1]*b[0];
}


/*====================
  M_GetVec2Length
  ====================*/
inline
float   M_GetVec2Length(vec2_t vec)
{
    return sqrt(vec[0]*vec[0] + vec[1]*vec[1]);
}


/*====================
  M_GetAxis

  returns a valid basis from euler angles
  uses YXZ euler angle ordering convention
  this corresponds to roll, then pitch, then yaw in the engine
  this is a RIGHT HANDED coordinate system
  ====================*/
inline
void    M_GetAxis(float pitch, float roll, float yaw, vec3_t axis[3])
{
    float sr,cr, sp,cp, sy,cy;
    float sr_sp;
    float neg_cr_sp;

    M_SinCos(DEG2RAD(roll), sr, cr);
    M_SinCos(DEG2RAD(pitch), sp, cp);
    M_SinCos(DEG2RAD(yaw), sy, cy);

    /*
        this is the concatanated version of the matrix multiples (generated with the matrix.c util):

        axis[0][0] = ((cr * 1 + 0 * 0 + -sr * 0) * cy + (cr * 0 + 0 * cp + -sr * -sp) *
                        -sy + (cr * 0 + 0 * sp + -sr * cp) * 0)
        axis[0][1] = ((cr * 1 + 0 * 0 + -sr * 0) * sy + (cr * 0 + 0 * cp + -sr * -sp) *
                        cy + (cr * 0 + 0 * sp + -sr * cp) * 0)
        axis[0][2] = ((cr * 1 + 0 * 0 + -sr * 0) * 0 + (cr * 0 + 0 * cp + -sr * -sp) * 0
                        + (cr * 0 + 0 * sp + -sr * cp) * 1)

        axis[1][0] = ((0 * 1 + 1 * 0 + 0 * 0) * cy + (0 * 0 + 1 * cp + 0 * -sp) * -sy +
                        (0 * 0 + 1 * sp + 0 * cp) * 0)
        axis[1][1] = ((0 * 1 + 1 * 0 + 0 * 0) * sy + (0 * 0 + 1 * cp + 0 * -sp) * cy + (
                        0 * 0 + 1 * sp + 0 * cp) * 0)
        axis[1][2] = ((0 * 1 + 1 * 0 + 0 * 0) * 0 + (0 * 0 + 1 * cp + 0 * -sp) * 0 + (0
                        * 0 + 1 * sp + 0 * cp) * 1)

        axis[2][0] = ((sr * 1 + 0 * 0 + cr * 0) * cy + (sr * 0 + 0 * cp + cr * -sp) * -s
                        y + (sr * 0 + 0 * sp + cr * cp) * 0)
        axis[2][1] = ((sr * 1 + 0 * 0 + cr * 0) * sy + (sr * 0 + 0 * cp + cr * -sp) * cy
                     + (sr * 0 + 0 * sp + cr * cp) * 0)
        axis[2][2] = ((sr * 1 + 0 * 0 + cr * 0) * 0 + (sr * 0 + 0 * cp + cr * -sp) * 0 +
                    (sr * 0 + 0 * sp + cr * cp) * 1)
    */

    sr_sp = sr * sp;
    neg_cr_sp = -cr * sp;

    axis[0][0] = cr * cy + sr_sp * -sy;
    axis[0][1] = cr * sy + sr_sp * cy;
    axis[0][2] = -sr * cp;

    axis[1][0] = cp * -sy;
    axis[1][1] = cp * cy;
    axis[1][2] = sp;

    axis[2][0] = sr * cy + neg_cr_sp * -sy;
    axis[2][1] = sr * sy + neg_cr_sp * cy;
    axis[2][2] = cr * cp;
}


/*====================
  M_EulerToQuat

  uses YXZ euler angle ordering convention
  this corresponds to roll, then pitch, then yaw in the engine
  this is a RIGHT HANDED coordinate system
  ====================*/
inline
void    M_EulerToQuat(float fPitch, float fRoll, float fYaw, vec4_t quat)
{
    float sr, cr, sp, cp, sy, cy;

    M_SinCos(DEG2RAD(fRoll) * 0.5f, sr, cr);
    M_SinCos(DEG2RAD(fPitch) * 0.5f, sp, cp);
    M_SinCos(DEG2RAD(fYaw) * 0.5f, sy, cy);

    quat[X] = cy * sp * cr - sy * cp * sr;
    quat[Y] = cy * cp * sr + sy * sp * cr;
    quat[Z] = cy * sp * sr + sy * cp * cr;
    quat[W] = cy * cp * cr - sy * sp * sr;
}

inline
CVec4f  M_EulerToQuat(const CVec3f &v3)
{
    float sr, cr, sp, cp, sy, cy;

    M_SinCos(DEG2RAD(v3[ROLL]) * 0.5f, sr, cr);
    M_SinCos(DEG2RAD(v3[PITCH]) * 0.5f, sp, cp);
    M_SinCos(DEG2RAD(v3[YAW]) * 0.5f, sy, cy);

    return CVec4f(
        cy * sp * cr - sy * cp * sr,
        cy * cp * sr + sy * sp * cr,
        cy * sp * sr + sy * cp * cr,
        cy * cp * cr - sy * sp * sr);
}

inline
void    M_EulerToQuat2(float sr, float cr, float sp, float cp, float sy, float cy, vec4_t quat)
{
    quat[X] = cy * sp * cr - sy * cp * sr;
    quat[Y] = cy * cp * sr + sy * sp * cr;
    quat[Z] = cy * sp * sr + sy * cp * cr;
    quat[W] = cy * cp * cr - sy * sp * sr;
}


/*====================
  M_AxisAngleToQuat

  Axis is assumed to be normalized
  ====================*/
inline
CVec4f  M_AxisAngleToQuat(const CVec3f &vAxis, float fAngle)
{
    float fSin, fCos;
    M_SinCos(DEG2RAD(fAngle * 0.5f), fSin, fCos);
 
    return CVec4f(vAxis.x * fSin, vAxis.y * fSin, vAxis.z * fSin, fCos);
}


/*====================
  M_MultQuat

  Multiplying q1 with q2 applies the rotation q2 to q1
  ====================*/
inline
CVec4f  M_MultQuat(const CVec4f &q1, const CVec4f &q2)
{
    return CVec4f
    (
        q2.w * q1.x + q2.x * q1.w + q2.y * q1.z - q2.z * q1.y,
        q2.w * q1.y + q2.y * q1.w + q2.z * q1.x - q2.x * q1.z,
        q2.w * q1.z + q2.z * q1.w + q2.x * q1.y - q2.y * q1.x,
        q2.w * q1.w - q2.x * q1.x - q2.y * q1.y - q2.z * q1.z
    );
}


/*====================
  M_QuatNormalize

  Multiplying q1 with q2 applies the rotation q2 to q1
  ====================*/
inline
void    M_QuatNormalize(CVec4f &q)
{
    // Don't normalize if we don't have to
    float fMagSq(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
    if (fabs(fMagSq - 1.0f) > TOLERANCE)
    {
        float fMag(sqrt(fMagSq));
        q.w /= fMag;
        q.x /= fMag;
        q.y /= fMag;
        q.z /= fMag;
    }
}


/*====================
  M_ClearVec3
  ====================*/
inline void M_ClearVec3(vec3_t out)
{
    out[0] = out[1] = out[2] = 0;
}


/*====================
  M_SurfaceNormal
  ====================*/
inline
CVec3f  M_SurfaceNormal(const CVec3f &a, const CVec3f &b, const CVec3f &c)
{
    return Normalize(CrossProduct(b - a, c - a));
}


/*====================
  M_PointOnLine
  ====================*/
inline
CVec3f  M_PointOnLine(const CVec3f &origin, const CVec3f &dir, float t)
{
    return origin + dir * t;
}


/*====================
  M_DirectionFromSphericalCoords
  ====================*/
inline
CVec3f  M_DirectionFromSphericalCoords(float fPhi, float fTheta)
{
    float fSinPhi, fCosPhi, fSinTheta, fCosTheta;

    M_SinCos(fPhi, fSinPhi, fCosPhi);
    M_SinCos(fTheta, fSinTheta, fCosTheta);

    // Convert to directional vector
    return CVec3f(fCosTheta * fSinPhi, fSinTheta * fSinPhi, fCosPhi);
}


/*====================
  M_IsPow2
  ====================*/
template<class T>
inline
bool    M_IsPow2(T Num)
{
    return POPCOUNT(uint(Num)) == 1;
}


/*====================
  M_CeilPow2
  ====================*/
template<class T>
inline
T       M_CeilPow2(T Num)
{
    T i(1);

    while (i < Num)
        i <<= 1;

    return i;
}


/*====================
  M_FloorPow2
  ====================*/
template<class T>
inline
T       M_FloorPow2(T Num)
{
    T i(1);

    while (i <= Num && i != 0)
        i <<= 1;

    return i >> 1;
}


/*====================
  M_Power
  ====================*/
inline
int     M_Power(int iBase, int iExp)
{
    int x = 1;

    for (int i = 0; i < iExp; ++i)
        x *= iBase;

    return x;
}


/*====================
  M_Log2
  ====================*/
inline
int     M_Log2(int iX)
{
    int iLog(0);

    while (iX > 1)
    {
        iX >>= 1;
        ++iLog;
    }
    
    return iLog;
}


/*====================
  M_TransformPlaneInverse
  ====================*/
inline
void    M_TransformPlaneInverse(const CPlane &plane, const CVec3f &pos, const CAxis &axis, CPlane &out)
{
    // get a point on the plane and transform it
    CVec3f point = TransformPointInverse(plane.v3Normal * plane.fDist, pos, axis);

    // rotate normal
    out.v3Normal = TransformPointInverse(plane.v3Normal, V_ZERO, axis);

    // recalc plane distance using new normal and point
    out.fDist = DotProduct(point, out.v3Normal);
}


/*====================
  M_PolarToCartesian

  x = theta (counterclockwise angle on xy plane from x axis)
  y = phi (counterclockwise angle on yz plane from y axis)
  z = r
  ====================*/
inline
CVec3f  M_PolarToCartesian(const CVec3f &p)
{
    float sx, cx, sy, cy;

    M_SinCos(p.x, sx, cx);
    M_SinCos(p.y, sy, cy);

    return CVec3f(cx * sy * p.z, sx * sy * p.z, cy * p.z);
}


/*====================
  M_CartesianToPolar
  ====================*/
inline
CVec3f  M_CartesianToPolar(const CVec3f &p)
{
    float fR(p.Length());
    float fTheta(atan2(p.y, p.x));
    float fPhi(acos(p.z / fR));

    return CVec3f(fTheta, fPhi, fR);
}


/*====================
  M_CylindricalToCartesian
  ====================*/
inline
CVec3f  M_CylindricalToCartesian(const CVec3f &p)
{
    float sx, cx;

    M_SinCos(p.x, sx, cx);

    return CVec3f(cx * p.z, sx * p.z, p.y);
}


/*====================
  M_AddVec2
  ====================*/
inline
void    M_AddVec2(const vec2_t a, const vec2_t b, vec2_t out)
{
    out[0] = a[0] + b[0];
    out[1] = a[1] + b[1];
}


/*====================
  M_SubVec2
  ====================*/
inline
void    M_SubVec2(const vec2_t a, const vec2_t b, vec2_t out)
{
    out[0] = a[0] - b[0];
    out[1] = a[1] - b[1];
}


/*====================
  M_MultVec2
  ====================*/
inline
void    M_MultVec2(const vec2_t a, float b, vec2_t out)
{
    out[0] = a[0] * b;
    out[1] = a[1] * b;
}


/*====================
  M_CompareVec2
  ====================*/
inline
bool    M_CompareVec2(const vec2_t a, const vec2_t b)
{
    if (fabs(a[0] - b[0]) > 0.001f ||
        fabs(a[1] - b[1]) > 0.001f)
        return false;

    return true;
}


/*====================
  M_SmoothStep
  ====================*/
inline
float   M_SmoothStep(float fValue, float fLow, float fHigh)
{
   if (fValue <= fLow)
      return 0.0f;

   if (fValue >= fHigh)
      return 1.0f;

   // Scale / bias into [0..1] range
   fValue = (fValue - fLow) / (fHigh - fLow);

   return fValue * fValue * (3.0f - 2.0f * fValue);
}


/*====================
  M_SmoothStepN

  Smoothstep between [0..1]
  ====================*/
inline
float   M_SmoothStepN(float fValue)
{
   if (fValue <= 0.0f)
      return 0.0f;

   if (fValue >= 1.0f)
      return 1.0f;

   return fValue * fValue * (3.0f - 2.0f * fValue);
}


/*====================
  M_SinCos
  ====================*/
inline
void    M_SinCos(float a, float &s, float &c)
{
#if defined(_WIN32) && !defined(_WIN64)
    _asm
    {
        fld     a
        fsincos
        mov     ecx, c
        mov     edx, s
        fstp    dword ptr [ecx]
        fstp    dword ptr [edx]
    }
#else
    s = sin(a);
    c = cos(a);
#endif
}
//=============================================================================

#endif //__MATH_INLINES_H__
