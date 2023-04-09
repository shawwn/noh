// (C)2005 S2 Games
// c_vec3.h
//=============================================================================
#ifndef __C_VEC3_H__
#define __C_VEC3_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"

#include <math.h>
#include <float.h>
//=============================================================================

template <class T> class CVec2;

//=============================================================================
// SVec3<T>
//
// struct version of CVec3 for statically initialized data
//=============================================================================
template <class T>
struct SVec3
{
    T   x, y, z;
};
//=============================================================================

//=============================================================================
// CVec3<T>
//=============================================================================
template <class T> class CVec3;
template <class T> CVec3<T> CrossProduct(const CVec3<T> &a, const CVec3<T> &b);
template <class T> T        DotProduct(const CVec3<T> &a, const CVec3<T> &b);
template <class T> T        Distance(const CVec3<T> &a, const CVec3<T> &b);
template <class T>
class CVec3
{
public:
    CVec3()                                                 {}
    CVec3(T _x, T _y, T _z) : x(_x), y(_y), z(_z)           {}
    CVec3(const T xyz[3]) : x(xyz[X]), y(xyz[Y]), z(xyz[Z]) {}
    CVec3(const SVec3<T> &v) : x(v.x), y(v.y), z(v.z)       {}
    CVec3(const CVec2<T> &v, T _z) : x(v.x), y(v.y), z(_z)  {}
    CVec3(const CVec3<T> &v3Dir, T length) : x(v3Dir.x * length), y(v3Dir.y * length), z(v3Dir.z * length)  {}
    CVec3(T _xyz) : x(_xyz), y(_xyz), z(_xyz)               {}

                operator T*()                       { return reinterpret_cast<T*>(this); }
                operator const T*() const           { return reinterpret_cast<const T*>(this); }

    const T&    operator[](int i) const             { return *((&x) + i); }
    T&          operator[](int i)                   { return *((&x) + i); }
    const T&    operator[](uint ui) const           { return *((&x) + ui); }
    T&          operator[](uint ui)                 { return *((&x) + ui); }

    bool        operator==(const CVec3 &b) const    { return (x == b.x && y == b.y && z == b.z); }
    bool        operator==(T b) const               { return (x == b && y == b && z == b); }
    bool        operator!=(const CVec3 &b) const    { return (x != b.x || y != b.y || z != b.z); }
    bool        operator!=(T b) const               { return (x != b || y != b || z != b); }

    CVec3<T>&   Set(T x2, T y2, T z2)               { x = x2; y = y2; z = z2; return *this; }
    CVec3<T>&   Set(const CVec3<T> &v3Dir, T length) { x = v3Dir.x * length; y = v3Dir.y * length; z = v3Dir.z * length; return *this; }

    CVec3<T>&   Clear()                             { x = 0.0f; y = 0.0f; z = 0.0f; return *this; }

    bool        IsValid() const                     { return _finite(x) && _finite(y) && _finite(z); }

    CVec3<T>&   operator+=(const CVec3<T> &b)       { x += b.x; y += b.y; z += b.z; return *this; }
    CVec3<T>&   operator-=(const CVec3<T> &b)       { x -= b.x; y -= b.y; z -= b.z; return *this; }
    CVec3<T>&   operator*=(const CVec3<T> &b)       { x *= b.x; y *= b.y; z *= b.z; return *this; }
    CVec3<T>&   operator/=(const CVec3<T> &b)       { x /= b.x; y /= b.y; z /= b.z; return *this; }
    CVec3<T>&   operator+=(T s)                     { x += s; y += s; z += s; return *this; }
    CVec3<T>&   operator-=(T s)                     { x -= s; y -= s; z -= s; return *this; }
    CVec3<T>&   operator*=(T s)                     { x *= s; y *= s; z *= s; return *this; }
    CVec3<T>&   operator/=(T s)                     { x /= s; y /= s; z /= s; return *this; }

    CVec3<T>    operator+(const CVec3<T> &b) const  { return CVec3<T>(x + b.x, y + b.y, z + b.z); }
    CVec3<T>    operator-(const CVec3<T> &b) const  { return CVec3<T>(x - b.x, y - b.y, z - b.z); }
    CVec3<T>    operator*(const CVec3<T> &b) const  { return CVec3<T>(x * b.x, y * b.y, z * b.z); }
    CVec3<T>    operator/(const CVec3<T> &b) const  { return CVec3<T>(x / b.x, y / b.y, z / b.z); }
    CVec3<T>    operator+(T s) const                { return CVec3<T>(x + s, y + s, z + s); }
    CVec3<T>    operator-(T s) const                { return CVec3<T>(x - s, y - s, z - s); }
    CVec3<T>    operator*(T s) const                { return CVec3<T>(x * s, y * s, z * s); }
    CVec3<T>    operator/(T s) const                { return CVec3<T>(x / s, y / s, z / s); }

    CVec3<T>    operator-() const                   { return CVec3<T>(-x, -y, -z); }

    const vec3_t*   vec3() const                    { return reinterpret_cast<const vec3_t *>(this);   }
    const CVec2<T>& xy() const                      { return *reinterpret_cast<const CVec2<T> *>(this); }
    CVec2<T>&       xy()                            { return *reinterpret_cast<CVec2<T> *>(this); }

    CVec3<T>    GetInverse() const                  { return CVec3<T>(-x, -y, -z); }
    T           Length() const                      { return sqrt(x * x + y * y + z * z); }
    T           LengthSq() const                    { return x * x + y * y + z * z; }

    T           Normalize()
    {       
        float fLengthSq(x * x + y * y + z * z);

        if (fabs(fLengthSq - 1.0f) > TOLERANCE)
        {
            if (fLengthSq == 0.0f)
            {
                x = 0.0f;
                y = 0.0f;
                z = 0.0f;
                return 0.0f;
            }

            float fLength(sqrt(fLengthSq));

            x /= fLength;
            y /= fLength;
            z /= fLength;

            return fLength;
        }

        return 1.0f;
    }

    CVec3<T>    Direction() const                   { CVec3<T> v(*this); v.Normalize(); return v; }
    CVec3<T>&   Invert()                            { x = -x; y = -y; z = -z; return *this; }

    const CVec3<T>& SetLength(float fLength)
    {
        Normalize();
        x *= fLength;
        y *= fLength;
        z *= fLength;
        return *this;
    }

    const CVec3<T>& SetDirection(const CVec3<T> &v3Dir)
    {
        float fLength(Length());
        x = v3Dir.x * fLength;
        y = v3Dir.y * fLength;
        z = v3Dir.z * fLength;
        return *this;
    }

    const CVec3<T>& LerpToward(float fLerp, const CVec3<T> &vec)
    {
        fLerp = CLAMP(fLerp, 0.0f, 1.0f);

        x = LERP(fLerp, x, vec.x);
        y = LERP(fLerp, y, vec.y);
        z = LERP(fLerp, z, vec.z);
        return *this;
    }

    const CVec3<T>& LerpAnglesToward(float fLerp, const CVec3<T> &vec)
    {
        fLerp = CLAMP(fLerp, 0.0f, 1.0f);

        x = M_LerpAngle(fLerp, x, vec.x);
        y = M_LerpAngle(fLerp, y, vec.y);
        z = M_LerpAngle(fLerp, z, vec.z);
        return *this;
    }

    bool        InBounds(const CVec3<T> &vecMin, const CVec3<T> &vecMax) const
    {
        if (x < vecMin.x || y < vecMin.y || z < vecMin.z ||
            x > vecMax.x || y > vecMax.y || z > vecMax.z)
            return false;

        return true;
    }

    void        AddToBounds(CVec3<T> &vecMin, CVec3<T> &vecMax) const
    {
        if (x < vecMin.x)
            vecMin.x = x;
        if (y < vecMin.y)
            vecMin.y = y;
        if (z < vecMin.z)
            vecMin.z = z;

        if (x > vecMax.x)
            vecMax.x = x;
        if (y > vecMax.y)
            vecMax.y = y;
        if (z > vecMax.z)
            vecMax.z = z;
    }

    void        Decompose(CVec3<T> &v3Dir, float &fLength) const
    {
        fLength = sqrt(x * x + y * y + z * z);
        
        if (fLength == 0.0f)
        {
            v3Dir.x = 0.0f;
            v3Dir.y = 0.0f;
            v3Dir.z = 0.0f;
            return;
        }
        
        v3Dir.x = x / fLength;
        v3Dir.y = y / fLength;
        v3Dir.z = z / fLength;
    }

    CVec3<T>&   Clip(const CVec3<T> &n, float fFudge = 1.000f)
    {
        float fDot(DotProduct(*this, n));

        // Fudge fDot
        if (fDot < 0.0f)
            fDot *= fFudge;
        else
            fDot /= fFudge;

        *this -= n * fDot;

        return *this;
    }

    CVec3<T>&   Project(const CVec3<T> &a)
    {
        *this = a * (DotProduct(a, *this)/DotProduct(a, a));

        return *this;
    }

    CVec3<T>&   ScaleAdd(const CVec3<T> &b, float f)
    {
        x += b.x * f; y += b.y * f; z += b.z * f; return *this;
    }

    // Public to allow direct access
    T   x, y, z;
};
//=============================================================================

// Export an instance of CVec3<float>
// See http://support.microsoft.com/default.aspx?scid=kb;EN-US;168958
#ifdef _WIN32
#pragma warning (disable : 4231)
K2_EXTERN template class K2_API CVec3<float>;
#pragma warning (default : 4231)
#endif

typedef SVec3<float>    SVec3f;
typedef SVec3<int>      SVec3i;
typedef SVec3<byte>     SVec3b;
typedef SVec3<LONGLONG> SVec3ll;

typedef CVec3<float>    CVec3f;
typedef CVec3<int>      CVec3i;
typedef CVec3<byte>     CVec3b;
typedef CVec3<LONGLONG> CVec3ll;
//=============================================================================
// Friend functions
//=============================================================================

/*====================
  CrossProduct
  ====================*/
template <class T>
inline CVec3<T> CrossProduct(const CVec3<T> &a, const CVec3<T> &b)
{
    return CVec3<T>(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}


/*====================
  DotProduct
  ====================*/
template <class T>
inline T    DotProduct(const CVec3<T> &a, const CVec3<T> &b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}


/*====================
  Normalize
  ====================*/
template <class T>
inline CVec3<T> Normalize(const CVec3<T> &a)
{
    float fLength = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);

    if (fLength == 0.0f)
    {
        return CVec3f(0.0f, 0.0f, 0.0f);
    }

    return CVec3f(a.x / fLength, a.y / fLength, a.z / fLength);
}


/*====================
  Normalize
  ====================*/
template <class T>
inline CVec3<T> Normalize(const CVec3<T> &a, T &_Length)
{
    float fLength = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);

    if (fLength == 0.0f)
    {
        return CVec3f(0.0f, 0.0f, 0.0f);
    }

    _Length = T(fLength);

    return CVec3f(a.x / fLength, a.y / fLength, a.z / fLength);
}


/*====================
  Length
  ====================*/
template <class T>
inline T    Length(const CVec3<T> &a)
{
    return sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}


/*====================
  LengthSq
  ====================*/
template <class T>
inline T    LengthSq(const CVec3<T> &a)
{
    return (a.x * a.x + a.y * a.y + a.z * a.z);
}


/*====================
  Distance
  ====================*/
template <class T>
inline T    Distance(const CVec3<T> &a, const CVec3<T> &b)
{
    T   dx = a.x - b.x;
    T   dy = a.y - b.y;
    T   dz = a.z - b.z;

    return static_cast<T>(sqrt(dx * dx + dy * dy + dz * dz));
}


/*====================
  DistanceSq
  ====================*/
template <class T>
inline T    DistanceSq(const CVec3<T> &a, const CVec3<T> &b)
{
    T   dx = a.x - b.x;
    T   dy = a.y - b.y;
    T   dz = a.z - b.z;

    return dx * dx + dy * dy + dz * dz;
}


/*====================
  Compare
  ====================*/
template <class T>
inline bool Compare(const CVec3<T> &a, const CVec3<T> &b)
{
    if (fabs(a.x - b.x) > 0.001 ||
        fabs(a.y - b.y) > 0.001 ||
        fabs(a.z - b.z) > 0.001)
    {
        return false;
    }
    return true;
}


/*====================
  Compare
  ====================*/
template <class T>
inline bool Compare(const CVec3<T> &a, const CVec3<T> &b, float fEpsilon)
{
    if (fabs(a.x - b.x) > fEpsilon ||
        fabs(a.y - b.y) > fEpsilon ||
        fabs(a.z - b.z) > fEpsilon)
    {
        return false;
    }
    return true;
}


/*====================
  Project

  Project the vector B onto A
  ====================*/
template <class T>
inline CVec3<T> Project(const CVec3<T> &a, const CVec3<T> &b)
{
    return (a * (DotProduct(a, b)/DotProduct(a, a)));
}


/*====================
  NormalProject

  Project the vector B onto the normalized vector A
  ====================*/
template <class T>
inline CVec3<T> NormalProject(const CVec3<T> &aNormalized, const CVec3<T> &b)
{
    return (aNormalized * (DotProduct(aNormalized, b)));
}


/*====================
  Prependicular

  Prependicular component of vector B with respec to A
  ====================*/
template <class T>
inline CVec3<T> Prependicular(const CVec3<T> &a, const CVec3<T> &b)
{
    return b - (a * (DotProduct(a, b)/DotProduct(a, a)));
}


/*====================
  NormalPrependicular

  Prependicular component of vector B with respec to the normalized vector A
  ====================*/
template <class T>
inline CVec3<T> NormalPrependicular(const CVec3<T> &aNormalized, const CVec3<T> &b)
{
    return b - (aNormalized * (DotProduct(aNormalized, b)));
}



/*====================
  Clip

  turn a vector parallel to a normal
  ====================*/
template <class T>
inline CVec3<T> Clip(const CVec3<T> &a, const CVec3<T> &n, float fFudge = 1.000f)
{
    float fDot(DotProduct(a, n));

    // Fudge fDot
    if (fDot < 0.0f)
        fDot *= fFudge;
    else
        fDot /= fFudge;

    return a - n * fDot;
}


/*====================
  Reflect

  calculate a reflected vector
  ====================*/
template <class T>
inline CVec3<T> Reflect(const CVec3<T> &a, const CVec3<T> &n, float fBounce)
{
    return a - n * (DotProduct(a, n) * (1.0f + fBounce));
}


/*====================
  Decompose

  decompose a vector into length and direction
  ====================*/
template <class T>
inline void Decompose(const CVec3<T> &v3Vec, CVec3<T> &v3Dir, float &fLength)
{
    fLength = sqrt(v3Vec.x * v3Vec.x + v3Vec.y * v3Vec.y + v3Vec.z * v3Vec.z);
    
    if (fLength == 0.0f)
    {
        v3Dir.x = 0.0f;
        v3Dir.y = 0.0f;
        v3Dir.z = 0.0f;
        return;
    }
    
    v3Dir.x = v3Vec.x / fLength;
    v3Dir.y = v3Vec.y / fLength;
    v3Dir.z = v3Vec.z / fLength;
}


/*====================
  Orthogonalize

  orthogonalize vector A with respec to normalized vector B
  ====================*/
template <class T>
inline CVec3<T> Orthogonalize(const CVec3<T> &a, const CVec3<T> &b)
{
    return Normalize(a - b * DotProduct(b, a));
}


//=============================================================================
// Non-template functions
//=============================================================================

// These are free, they just look ugly :)
// Only needed until all vec3_t's are converted
#define CVec3_cast(v) (*(CVec3f*)(&((v)[X])))
#define vec3_cast(v) (*(vec3_t*)(&(v)))

struct SPointInfo
{
    CVec3f  normal;  // surface normal
    float   z;       // height of terrain at this point
};

#endif // __C_VEC3_H__
