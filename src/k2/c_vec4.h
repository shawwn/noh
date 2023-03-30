// (C)2005 S2 Games
// c_vec4.h
//=============================================================================
#ifndef __C_VEC4_H__
#define __C_VEC4_H__

//=============================================================================
// Headers
//=============================================================================
#include <math.h>
#include <float.h>
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
template <class T> class CVec2;
template <class T> class CVec3;
//=============================================================================

//=============================================================================
// SVec4<T>
//
// aggregate version of CVec4 for statically initialized data
//=============================================================================
template <class T>
struct SVec4
{
    T   x, y, z, w;
};
//=============================================================================

//=============================================================================
// CVec4<T>
//=============================================================================
template <class T> class CVec4;
template <class T> CVec4<T> CrossProduct(const CVec4<T> &a, const CVec4<T> &b);
template <class T> T        DotProduct(const CVec4<T> &a, const CVec4<T> &b);
template <class T> T        Distance(const CVec4<T> &a, const CVec4<T> &b);
template <class T>
class CVec4
{
public:
    CVec4() {}
    CVec4(T _x, T _y, T _z, T _w) : x(_x), y(_y), z(_z), w(_w)      {}
    CVec4(const T a[4]) : x(a[X]), y(a[Y]), z(a[Z]), w(a[W])        {}
    CVec4(const SVec4<T> &v) : x(v.x), y(v.y), z(v.z), w(v.w)       {}
    CVec4(const CVec3<T> &v, T _w) : x(v.x), y(v.y), z(v.z), w(_w)  {}
    CVec4(dword dw);
    CVec4(T _xyzw) : x(_xyzw), y(_xyzw), z(_xyzw), w(_xyzw)         {}

    const T &operator[](int i) const        { return *((&x)+i); };
    T &operator[](int i)                    { return *((&x)+i); };

    bool operator==(const CVec4 &b) const   { return (x == b.x && y == b.y && z == b.z && w == b.w); }
    bool operator==(T b) const              { return (x == b && y == b && z == b && w == b); }
    bool operator!=(const CVec4 &b) const   { return (x != b.x || y != b.y || z != b.z || w != b.w); }
    bool operator!=(T b) const              { return (x != b || y != b || z != b || w != b); }

    void Set(T x2, T y2, T z2, T w2)        { x = x2; y = y2; z = z2; w = w2; }
    void Clear()                            { x = 0.0f; y = 0.0f; z = 0.0f; w = 0.0f; }

    bool IsValid() const                    { return _finite(x) && _finite(y) && _finite(z) && _finite(w); }

    CVec4<T> &operator+=(const CVec4<T> &b) { x += b.x; y += b.y; z += b.z; w += b.w; return *this; }
    CVec4<T> &operator-=(const CVec4<T> &b) { x -= b.x; y -= b.y; z -= b.z; w -= b.w; return *this; }
    CVec4<T> &operator*=(const CVec4<T> &b) { x *= b.x; y *= b.y; z *= b.z; w *= b.w; return *this; }
    CVec4<T> &operator/=(const CVec4<T> &b) { x /= b.x; y /= b.y; z /= b.z; w /= b.w; return *this; }
    CVec4<T> &operator+=(T s)               { x += s; y += s; z += s; w += s; return *this; }
    CVec4<T> &operator-=(T s)               { x -= s; y -= s; z -= s; w -= s; return *this; }
    CVec4<T> &operator*=(T s)               { x *= s; y *= s; z *= s; w *= s; return *this; }
    CVec4<T> &operator/=(T s)               { x /= s; y /= s; z /= s; w /= s; return *this; }

    CVec4<T> operator+(const CVec4<T> &b) const { return CVec4<T>(x + b.x, y + b.y, z + b.z, w + b.w); }
    CVec4<T> operator-(const CVec4<T> &b) const { return CVec4<T>(x - b.x, y - b.y, z - b.z, w - b.w); }
    CVec4<T> operator*(const CVec4<T> &b) const { return CVec4<T>(x * b.x, y * b.y, z * b.z, w * b.w); }
    CVec4<T> operator/(const CVec4<T> &b) const { return CVec4<T>(x / b.x, y / b.y, z / b.z, w / b.w); }
    CVec4<T> operator*(T s) const           { return CVec4<T>(x * s, y * s, z * s, w * s); }
    CVec4<T> operator/(T s) const           { return CVec4<T>(x / s, y / s, z / s, w / s); }

    CVec4<T> operator-() const              { return CVec4<T>(-x, -y, -z, -w); }

    const vec_t*    vec4() const            { return reinterpret_cast<const vec_t *>(this);   }
    vec_t*          vec4()                  { return reinterpret_cast<vec_t *>(this);   }
    const CVec3<T>& xyz() const             { return *reinterpret_cast<const CVec3<T> *>(this); }
    CVec3<T>&       xyz()                   { return *reinterpret_cast<CVec3<T> *>(this); }
    const CVec2<T>& xy() const              { return *reinterpret_cast<const CVec2<T> *>(this); }
    CVec2<T>&       xy()                    { return *reinterpret_cast<CVec2<T> *>(this); }

    CVec4<T> GetInverse() const             { return CVec4<T>(-x, -y, -z, -w); }
    T Length() const                        { return sqrt(x * x + y * y + z * z + w * w); }

    inline dword    GetAsDWord() const;
    inline dword    GetAsDWordGL() const;

    inline T Normalize();
    inline CVec4<T> Direction() const;
    inline CVec4<T> &Invert();

    inline bool InBounds(const CVec4<T> &vecMin, const CVec4<T> &vecMax) const;
    inline void AddToBounds(CVec4<T> &vecMin, CVec4<T> &vecMax) const;

    // Public to allow direct access
    T   x, y, z, w;
};
//=============================================================================

// Export an instance of CVec4<float>
// See http://support.microsoft.com/default.aspx?scid=kb;EN-US;168958
#ifdef _WIN32
#pragma warning (disable : 4231)
K2_EXTERN template class K2_API CVec4<float>;
#pragma warning (default : 4231)
#endif
//=============================================================================
// Member functions
//=============================================================================

/*====================
  CVec4<T>::CVec4
  ====================*/
template <>
inline
CVec4<float>::CVec4(dword dw) :
#if BYTE_ORDER == LITTLE_ENDIAN
x(((dw >> 16) & 0xff) / 255.0f),
y(((dw >> 8) & 0xff) / 255.0f),
z(((dw) & 0xff) / 255.0f),
w(((dw >> 24) & 0xff) / 255.0f)
#else
x(((dw >> 8) & 0xff) / 255.0f),
y(((dw >> 16) & 0xff) / 255.0f),
z(((dw >> 24) & 0xff) / 255.0f),
w(((dw) & 0xff) / 255.0f)
#endif
{
}


/*====================
  CVec4<T>::Normalize
  ====================*/
template <class T>
T   CVec4<T>::Normalize()
{
    float   flLength, flInvLength;

    flLength = sqrt(x * x + y * y + z * z + w * w);
    if (flLength == 0.0f)
    {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
        w = 0.0f;
        return 0.0f;
    }

    flInvLength = 1.0f / flLength;
    x *= flInvLength;
    y *= flInvLength;
    z *= flInvLength;
    w *= flInvLength;

    return flLength;
}

/*====================
  CVec4<T>::Direction
  ====================*/
template <class T>
CVec4<T> CVec4<T>::Direction() const
{
    CVec4<T> temp(*this);
    temp.Normalize();
    return temp;
}

/*====================
  CVec4<T>::Invert
  ====================*/
template <class T>
CVec4<T> &CVec4<T>::Invert()
{
    x = -x;
    y = -y;
    z = -z;
    w = -w;
    return *this;
}

/*====================
  CVec4<T>::InBounds
  ====================*/
template <class T>
bool    CVec4<T>::InBounds(const CVec4<T> &vecMin, const CVec4<T> &vecMax) const
{
    if (x < vecMin.x || y < vecMin.y || z < vecMin.z || w < vecMin.w ||
        x > vecMax.x || y > vecMax.y || z > vecMax.z || w > vecMin.w)
        return false;

    return true;
}

/*====================
  CVec4<T>::AddToBounds
  ====================*/
template <class T>
void    CVec4<T>::AddToBounds(CVec4<T> &vecMin, CVec4<T> &vecMax) const
{
    if (x < vecMin.x)
        vecMin.x = x;
    if (y < vecMin.y)
        vecMin.y = y;
    if (z < vecMin.z)
        vecMin.z = z;
    if (w < vecMin.w)
        vecMin.w = w;

    if (x > vecMax.x)
        vecMax.x = x;
    if (y > vecMax.y)
        vecMax.y = y;
    if (z > vecMax.z)
        vecMax.z = z;
    if (w > vecMax.w)
        vecMax.w = w;
}


/*====================
  CVec4<T>::GetAsDWord
  ====================*/
template <>
inline
dword   CVec4<float>::GetAsDWord() const
{
#if BYTE_ORDER == LITTLE_ENDIAN
    return 
    (dword)(
        (CLAMP(INT_FLOOR(w * 255.0f), 0, 255) << 24) |
        (CLAMP(INT_FLOOR(x * 255.0f), 0, 255) << 16) |
        (CLAMP(INT_FLOOR(y * 255.0f), 0, 255) << 8) |
        (CLAMP(INT_FLOOR(z * 255.0f), 0, 255))
    );
#else
    return 
    (dword)(
        (CLAMP(INT_FLOOR(z * 255.0f), 0, 255) << 24) |
        (CLAMP(INT_FLOOR(y * 255.0f), 0, 255) << 16) |
        (CLAMP(INT_FLOOR(x * 255.0f), 0, 255) << 8) |
        (CLAMP(INT_FLOOR(w * 255.0f), 0, 255))
    );
#endif
}


/*====================
  CVec4<T>::GetAsDWordGL
  ====================*/
template <>
inline
dword   CVec4<float>::GetAsDWordGL() const
{
#if BYTE_ORDER == LITTLE_ENDIAN
    return 
    (dword)(
        (CLAMP(INT_FLOOR(w * 255.0f), 0, 255) << 24) |
        (CLAMP(INT_FLOOR(z * 255.0f), 0, 255) << 16) |
        (CLAMP(INT_FLOOR(y * 255.0f), 0, 255) << 8) |
        (CLAMP(INT_FLOOR(x * 255.0f), 0, 255))
    );
#else
    return 
    (dword)(
        (CLAMP(INT_FLOOR(x * 255.0f), 0, 255) << 24) |
        (CLAMP(INT_FLOOR(y * 255.0f), 0, 255) << 16) |
        (CLAMP(INT_FLOOR(z * 255.0f), 0, 255) << 8) |
        (CLAMP(INT_FLOOR(w * 255.0f), 0, 255))
    );
#endif
}


//=============================================================================
// Friend functions
//=============================================================================

/*====================
  CrossProduct
  ====================*/
template <class T>
inline CVec4<T> CrossProduct(const CVec4<T> &a, const CVec4<T> &b)
{
#if 0 // TODO
    return CVec4<T>(
        a.x,
        a.y,
        a.z,
        a.w
    );
#endif
}

/*====================
  DotProduct
  ====================*/
template <class T>
inline T    DotProduct(const CVec4<T> &a, const CVec4<T> &b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

/*====================
  Distance
  ====================*/
template <class T>
inline T    Distance(const CVec4<T> &a, const CVec4<T> &b)
{
    T   dx = a.x - b.x;
    T   dy = a.y - b.y;
    T   dz = a.z - b.z;
    T   dw = a.w - b.w;

    return static_cast<T>(sqrt(dx * dx + dy * dy + dz * dz + dw * dw));
}

/*====================
  DistanceSq
  ====================*/
template <class T>
inline T    DistanceSq(const CVec4<T> &a, const CVec4<T> &b)
{
    T   dx = a.x - b.x;
    T   dy = a.y - b.y;
    T   dz = a.z - b.z;
    T   dw = a.w - b.w;

    return dx * dx + dy * dy + dz * dz + dw * dw;
}


/*====================
  Compare
  ====================*/
template <class T>
bool    Compare(const CVec4<T> &a, const CVec4<T> &b)
{
    if (fabs(a.x - b.x) > 0.001 ||
        fabs(a.y - b.y) > 0.001 ||
        fabs(a.z - b.z) > 0.001 ||
        fabs(a.w - b.w) > 0.001)
    {
        return false;
    }
    return true;
}


/*====================
  Compare
  ====================*/
template <class T>
bool    Compare(const CVec4<T> &a, const CVec4<T> &b, float fEpsilon)
{
    if (fabs(a.x - b.x) > fEpsilon ||
        fabs(a.y - b.y) > fEpsilon ||
        fabs(a.z - b.z) > fEpsilon ||
        fabs(a.w - b.w) > fEpsilon)
    {
        return false;
    }
    return true;
}

//=============================================================================
// Non-template functions
//=============================================================================
typedef SVec4<float>    SVec4f;
typedef SVec4<int>      SVec4i;
typedef SVec4<byte>     SVec4b;
typedef SVec4<LONGLONG> SVec4ll;

typedef CVec4<float>    CVec4f;
typedef CVec4<int>      CVec4i;
typedef CVec4<byte>     CVec4b;
typedef CVec4<LONGLONG> CVec4ll;

// These are free, they just look ugly :)
// Only needed until all vec3_t's are converted
#define CVec4_cast(v) (*(CVec4f*)(&v[X]))
#define vec4_cast(v) (*(vec4_t*)(&v))

extern const CVec4f g_vecZero4;
//=============================================================================

#endif // __C_VEC4_H__
