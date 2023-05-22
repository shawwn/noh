// (C)2005 S2 Games
// k2_mathlib.h
//
//=============================================================================
#ifndef __K2_MATHLIB_H__
#define __K2_MATHLIB_H__

//=============================================================================
// Headers
//=============================================================================
#include <math.h>
#include <algorithm>
#include "k2_randlib.h"

#if defined(linux) || defined(__APPLE__)
#if defined(__cplusplus) && !defined(isfinite) 
#define _finite std::isfinite
#define _isnan std::isnan
#else
#define _finite isfinite
#define _isnan isnan
#endif
#endif
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//inline double     K2_RAND(double lo, double hi);
//=============================================================================

//=============================================================================
// CRandomSequenceGenerator
//=============================================================================
class K2_API CRandomSequenceGenerator
{
private:
    uint    m_uiRegister;
    int     m_iTapA;
    int     m_iTapB;
    int     m_iTapC;
    int     m_iTapD;

    uint    Tap(int n)                              { return ((m_uiRegister & (1 << (n - 1))) << (32 - n)); }
    void    Advance()
    {
        for (int i(0); i < 32; ++i)
        {
            uint uiTap(0);
            if (m_iTapD != -1)
                uiTap = Tap(m_iTapA) ^ Tap(m_iTapB) ^ Tap(m_iTapC) ^ Tap(m_iTapD);
            else if (m_iTapC != -1)
                uiTap = Tap(m_iTapA) ^ Tap(m_iTapB) ^ Tap(m_iTapC);
            else if (m_iTapB != -1)
                uiTap = Tap(m_iTapA) ^ Tap(m_iTapB);
            else if (m_iTapA != -1)
                uiTap = Tap(m_iTapA);

            m_uiRegister >>= 1;
            m_uiRegister |= uiTap;
        }
    }

public:
    ~CRandomSequenceGenerator()                     {}
    CRandomSequenceGenerator() : m_uiRegister(0)    {}
    CRandomSequenceGenerator(uint uiSeed, int A = 3, int B = 5, int C = 7, int D = 11) :
    m_uiRegister(uiSeed * 1664525 + 1013904223),
    m_iTapA(A),
    m_iTapB(B),
    m_iTapC(C),
    m_iTapD(D)
    {
        if (m_iTapA == -1)
        {
            m_iTapA = m_iTapB;
            m_iTapB = m_iTapC;
            m_iTapC = m_iTapD;
            m_iTapD = -1;
        }
        if (m_iTapB == -1)
        {
            m_iTapB = m_iTapC;
            m_iTapC = m_iTapD;
            m_iTapD = -1;
        }
        if (m_iTapC == -1)
        {
            m_iTapC = m_iTapD;
            m_iTapD = -1;
        }
    }

    void    Seed(uint uiSeed)                       { m_uiRegister = uiSeed * 1664525 + 1013904223; }
    void    SetRegister(uint uiValue)               { m_uiRegister = uiValue; }

    float   GetFloat(float fMin, float fMax)
    {
        float fResult((m_uiRegister & 0x7fff) / float(0x7fff));
        Advance();
        return fResult * (fMax - fMin) + fMin;
    }
    int     GetInt(int iMin, int iMax)
    {
        int iResult(m_uiRegister % (iMax - iMin + 1) + iMin);
        Advance();
        return iResult;
    }
    uint    GetUInt(uint uiMin, uint uiMax)
    {
        uint uiResult(m_uiRegister % (uiMax - uiMin + 1) + uiMin);
        Advance();
        return uiResult;
    }
};
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#ifndef M_PI
#define M_PI    3.14159265358979323846f
#endif

#ifndef M_E
#define M_E     2.71828182845904523536f
#endif

#ifndef DEG2RAD
#define DEG2RAD(a) ((a) * (M_PI / 180.0f))
#endif
#ifndef RAD2DEG
#define RAD2DEG(a) ((a) * (180.0f / M_PI))
#endif

#define DEGSIN(a) (sin(DEG2RAD(a)))
#define DEGCOS(a) (cos(DEG2RAD(a)))

#define WORD2ANGLE(a) (((float)(a) / 65536) * 360)
#define ANGLE2WORD(a) (word)(((int)((a) * 65536.0 / 360.0)) & 65535)
#define BYTE2ANGLE(a) (((float)(a) / 256) * 360)
#define ANGLE2BYTE(a) (byte)(((int)((a) * 256.0 / 360.0)) & 255)
#define COORD2SHORT(a) ((short)(a * 2))
#define SHORT2COORD(a) ((float)(a * 0.5))

const float TOLERANCE(0.00001f);

template<class T>
const T& MIN(const T& _Left, const T& _Right)
{
    // return smaller of _Left and _Right
    return (_Right < _Left ? _Right : _Left);
}

template<class T>
const T& MAX(const T& _Left, const T& _Right)
{
    // return larger of _Left and _Right
    return (_Left < _Right ? _Right : _Left);
}
//=============================================================================

typedef struct matrix43_s
{
    vec3_t          axis[3];
    vec3_t          pos;
}
matrix43_t;

const matrix43_t g_identity43 =
{
    {
        { 1, 0, 0 },
        { 0, 1, 0 },
        { 0, 0, 1 }
    },

    { 0, 0, 0 }
};

inline  void    M_SinCos(float a, float &s, float &c);

//=============================================================================
// Templates
//=============================================================================

/*====================
  SIGN
  ====================*/
template<class T>
inline
int SIGN(T a)
{
    return a < 0 ? -1 : 1;
}


/*====================
  LERP
  ====================*/
template<class T>
inline
T   LERP(float f, T low, T hi)
{
    return static_cast<T>(low + (hi - low) * f);
}


/*====================
  ILERP

  Inverse of the lerp function
  ====================*/
template<class T>
inline
float   ILERP(T f, T low, T hi)
{
    return float(f - low)/(hi - low);
}


/*====================
  CLAMP
  ====================*/
template<class T>
inline
T   CLAMP(T v, T low, T hi)
{
    return v < low ? low : v > hi ? hi : v;
}


/*====================
  ABS
  ====================*/
template<class T>
inline
T   ABS(T x)
{
    return x < 0 ? -x : x;
}


/*====================
  SWAP
  ====================*/
template<class T>
inline
void    SWAP(T &a, T &b)
{
    T temp = a;
    a = b;
    b = temp;
}


/*====================
  ROUND
  ====================*/
inline
float   ROUND(float f)
{
    return floor(f + 0.5f);
}


/*====================
  SQR
  ====================*/
template<class T>
inline
T   SQR(T x)
{
    return x * x;
}


/*====================
  FRAC
  ====================*/
inline
float   FRAC(float f)
{
    return f - floor(f);
}


/*====================
  PCF

  Percentage Closer Filter
  ====================*/
template<class T>
inline
T   PCF(const float *lerps, const T *values)
{
    return LERP(lerps[1], LERP(lerps[0], values[0], values[1]), LERP(lerps[0], values[2], values[3]));
}


/*====================
  POPCOUNT
  
  Bit population count
  http://en.wikipedia.org/wiki/Hamming_weight
  ====================*/
inline
uint    POPCOUNT(uint x)
{
    x -= (x >> 1) & 0x55555555;                     // Put count of each 2 bits into those 2 bits
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333); // Put count of each 4 bits into those 4 bits 
    x = (x + (x >> 4)) & 0x0f0f0f0f;                // Put count of each 8 bits into those 8 bits 
    return (x * 0x01010101) >> 24;                  // Returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ... 
}


const uint WORDBITS(sizeof(uint) * 8);
/*====================
  LZC

  Leading Zero Count
  ====================*/
inline
uint    LZC(uint x)
{
    x |= (x>>1);
    x |= (x>>2);
    x |= (x>>4);
    x |= (x>>8);
    x |= (x>>16);

    return (WORDBITS - POPCOUNT(x));
}


/*====================
  TZC

  Trailing Zero Count
  ====================*/
inline
uint    TZC(uint x)
{
    return POPCOUNT((x & uint(-int(x))) - 1);
}


/*====================
  TOGGLE
  ====================*/
template<class T>
inline
T   TOGGLE(T a)
{
    return (a + 1) & 0x1;
}


/*====================
  CHANCE
  ====================*/
inline
bool    CHANCE(float f)
{
    if (f <= 0.0f)
        return false;
    if (f >= 1.0f)
        return true;

    //float r((rand() & 0x7fff) / (float)0x7fff);
    //return r <= f;

    double r(K2_RAND(0.0, 1.0));
    return r <= (double)f;
}

inline int INT_ROUND(float f)               { return f >= 0.0f ? static_cast<int>(f + 0.5f) : static_cast<int>(f - 0.5f); }
inline int INT_FLOOR(float f)               { return static_cast<int>(f); }
inline int INT_CEIL(float f)                { return static_cast<int>(ceil(f)); }
inline bool FLOAT_EQUALS(float a, float b)  { return fabs(a - b) <= 0.00001f; }
inline byte BYTE_ROUND(float f)             { return byte(CLAMP(static_cast<int>(f + 0.5f), 0, 255)); }


/*====================
  CEIL_MULTIPLE

  Round up to a nearest multiple of a power of 2
  ====================*/
template<uint M>
inline
uint    CEIL_MULTIPLE(uint x)
{
    return (x + (M - 1)) & ~(M - 1);
}


/*====================
  CEIL_MULTIPLE

  Round up to a nearest multiple of M
  ====================*/
inline
uint    CEIL_MULTIPLE(uint M, uint x)
{
    return (x + (M - 1)) & ~(M - 1);
}


/*====================
  FLOOR_MULTIPLE

  Round down to a nearest multiple of a power of 2
  ====================*/
template<uint M>
inline
uint    FLOOR_MULTIPLE(uint x)
{
    return x & ~(M - 1);
}


//=============================================================================

#include "c_vec.h"
#include "k2_constants.h"


/*====================
  DECAY
  ====================*/
template<class T>
inline
T   DECAY(T start, T target, float fHalfLife, float fTime)
{
    if (start == target)
        return target;
    else if (fTime == 0.0f)
        return start;
    else if (fHalfLife == 0.0f)
        return target;
    else
        return T(target + (start - target) * exp(-(LN2 / fHalfLife) * fTime));
}


#include "c_boundingbox.h"
#include "c_plane.h"
#include "c_axis.h"
#include "c_rect.h"
#include "c_matrix4x3.h"

const float     EPSILON = 1.19209e-007f;
const float     QUAT_EPSILON = 1.19209e-007f;

#define SET_VEC3(vec, x, y, z)      ( vec[0] = (x), vec[1] = (y), vec[2] = (z))
#define SET_VEC4(vec, x, y, z, w)   ( vec[0] = (x), vec[1] = (y), vec[2] = (z), vec[3] = (w))

#define MAKEMATRIX(m, m00, m01, m02, m10, m11, m12, m20, m21, m22) \
    m[0][0] = m00; \
    m[0][1] = m01; \
    m[0][2] = m02; \
    m[1][0] = m10; \
    m[1][1] = m11; \
    m[1][2] = m12; \
    m[2][0] = m20; \
    m[2][1] = m21; \
    m[2][2] = m22;

#define M_SubVec3(a, b, out)    ((out[0]) = (a[0]) - (b[0]), (out[1]) = (a[1]) - (b[1]), (out[2]) = (a[2]) - (b[2]))
#define M_DotProduct(a,b)       ((a[0]) * (b[0]) + (a[1]) * (b[1]) + (a[2]) * (b[2]))
#define M_DotProduct2(a,b)      ((a[0]) * (b[0]) + (a[1]) * (b[1]))

K2_API float    M_GetDistanceSq(const CVec3f &v3A, const CVec3f &v3B);
K2_API void     M_SetVec3(vec3_t out, float x, float y, float z);
K2_API void     M_SetVec2(vec3_t out, float x, float y);
K2_API void     M_ClearVec3(vec3_t out);
K2_API bool     M_CompareVec3(const vec3_t a, const vec3_t b, float fEpsilon);
K2_API bool     M_CompareVec4(const vec4_t a, const vec4_t b, float fEpsilon);

inline  void    M_MultVec3(const vec3_t a, float b, vec3_t out);
K2_API  float   M_Normalize(vec3_t out);
K2_API  float   M_NormalizeVec2(vec2_t out);
inline  void    M_AddPointToBounds(const vec3_t point, vec3_t bmin, vec3_t bmax);
inline  void    M_ClearBounds(vec3_t bmin, vec3_t bmax);

K2_API  void    M_GetNormal(vec3_t nml, int x, int y);
K2_API  void    M_CrossProduct(const vec3_t a, const vec3_t b, vec3_t out);
K2_API  void    M_CopyVec2(const vec2_t in, vec2_t out);
K2_API  inline void M_CopyVec3(const vec3_t in, vec3_t out);
K2_API  EPlaneTest  M_AABBOnPlaneSide(const CBBoxf &bbBox, const CPlane &p);
K2_API  EPlaneTest  M_OBBOnPlaneSide(const CBBoxf &bbBox, const CVec3f &v3Pos, const CAxis &aAxis, const CPlane &p);
K2_API  bool    M_RayBoxIntersect(const vec3_t origin, const vec3_t dir, const vec3_t bmin, const vec3_t bmax, vec3_t out);
K2_API  void    M_AddPointToRect(const vec2_t point, vec2_t bmin, vec2_t bmax);
K2_API  void    M_ClearRect(vec2_t bmin, vec2_t bmax);
K2_API  bool    M_RectInRect(const vec2_t bmin1, const vec2_t bmax1, const vec2_t bmin2, const vec2_t bmax2);
K2_API  bool    M_RectInRect(const CVec3f &bmin1, const CVec3f &bmax1, const CVec3f &bmin2, const CVec3f &bmax2);
K2_API  void    M_CalcBoxExtents(const vec3_t bmin, const vec3_t bmax, vec3_t pos, vec3_t ext);
K2_API  void    M_CalcBoxExtents(const CVec3f &bmin, const CVec3f &bmax, CVec3f &pos, CVec3f &ext);
K2_API  float   M_RayPlaneIntersect(const CVec3f &v3Origin, const CVec3f &v3Dir, const CPlane &plPlane, CVec3f &v3Result);
K2_API  void    M_PointOnLine(const vec3_t origin, const vec3_t dir, float fFraction, vec3_t out);
inline  void    M_GetAxis(float anglex, float angley, float anglez, vec3_t axis[3]);
inline  void    M_TransformPoint(const vec3_t point, const vec3_t pos, const vec3_t axis[3], vec3_t out);
inline  void    M_TransformPointInverse(const vec3_t point, const vec3_t pos, const vec3_t axis[3], vec3_t out);
inline  void    M_RotatePoint(const vec3_t point, const vec3_t axis[3], vec3_t out);

//rotates the bounding box points and creates a new axis - aligned bounding box out of it (new box may be larger than the original)
K2_API  void    M_TransformBounds(const CVec3f &bmin, const CVec3f &bmax, const CVec3f &pos, const CAxis &axis, CVec3f &bmin_out, CVec3f &bmax_out);
K2_API  float   M_GetYawFromForwardVec2(const CVec2f &v2Forward);
K2_API  void    M_GetAnglesFromForwardVec(const CVec3f &vForward, CVec3f &vAngles);
K2_API  CVec3f  M_GetAnglesFromForwardVec(const CVec3f &vForward);
K2_API  CVec3f  M_GetForwardVecFromAngles(const CVec3f &v3Angles);
K2_API  CVec2f  M_GetForwardVec2FromYaw(float fYaw);

K2_API  float   M_Randnum(float fMin, float fMax);
K2_API  int     M_Randnum(int iMin, int iMax);
K2_API  uint    M_Randnum(uint iMin, uint iMax);

K2_API  float   M_Noise1(float x);
K2_API  float   M_Noise2(float x, float y);
K2_API  float   M_Noise3(float x, float y, float z);
K2_API  float   M_SmoothRandAngle1(float x);
K2_API  float   M_SmoothRand1(float x);
K2_API  float   M_ClampLerp(float amount, float low, float high);
K2_API  float   M_LerpAngle(float a, float low, float high);
K2_API  CVec3f  M_LerpAngles(float a, const CVec3f &v3Low, const CVec3f &v3High);
K2_API  float   M_ChangeAngle(float fAngleStep, float fStartAngle, float fEndAngle);
K2_API  float   M_DiffAngle(float fAngle1, float fAngle2);
K2_API  float   M_YawToPosition(const CVec2f &v2Pos0, const CVec2f &v2Pos1);
K2_API  float   M_YawToPosition(const CVec3f &v3Pos0, const CVec3f &v3Pos1);

K2_API  void    M_InitSimpleNoise2();
K2_API  float   M_SimpleNoise2(uint x, uint y);

K2_API  bool    M_LineBoxIntersect3d(const CVec3f &vecStart, const CVec3f &vecEnd, const CVec3f &vecBMin, const CVec3f &vecBMax, float &fFraction);
K2_API  bool    M_LineBoxIntersect3d(const CVec3f &vecStart, const CVec3f &vecEnd, const CBBoxf &bbBounds, float &fFraction);

// Matrix math (4x3, unless otherwise specified in the function name)
inline  void    M_Identity(matrix43_t *t);
inline  void    M_MultiplyMatrix(const matrix43_t *a, const matrix43_t *b, matrix43_t *out);
K2_API  void    M_BlendMatrix(const matrix43_t *a, const matrix43_t *b, float fAmount, matrix43_t *out);

// Quaternion routines for skeletal animation
K2_API  void    M_QuatToAxis(const vec4_t quat, vec3_t out[3]);
K2_API  CAxis   M_QuatToAxis(const CVec4f &v4);
K2_API  void    M_AxisToQuat(const vec3_t in[3], vec4_t out);
K2_API  CVec4f  M_AxisToQuat(const CAxis &in);
K2_API  void    M_LerpQuat(float fLerp, const vec4_t from, const vec4_t to, vec4_t result);
K2_API  CVec4f  M_LerpQuat(float fLerp, const CVec4f &v4From, const CVec4f &v4To);
K2_API  void    M_LerpQuat(float fLerp, const CVec4f &v4From, const CVec4f &v4To, CVec4f &v4Result);

K2_API  CVec3f  M_SlerpDirection(float fLerp, const CVec3f &v3A, const CVec3f &v3B);
K2_API  float   M_ArcLengthFactor(float fAngle1, float fAngle2);

K2_API  int     M_NextPowerOfTwo(int x);

K2_API  CVec3f  M_RandomDirection();
K2_API  CVec3f  M_RandomDirection(const CVec3f &v3Dir, float fAngle);
K2_API  CVec3f  M_RandomDirection(const CVec3f &v3Dir, float fAngle0, float fAngle1);
K2_API  CVec3f  M_RandomPointInSphere();
K2_API  CVec2f  M_RandomPointInCircle();

K2_API  float   M_AreaOfTriangle(const CVec3f &v3A, const CVec3f &v3B, const CVec3f &v3C);

K2_API  uint    M_BlueNoise(const CRectf &rec, int iDepth, vector<CVec2f> &v2Points);

K2_API float    M_GammaDistribution(int x, int k, float fTheta);
inline float    M_LogisticDistribution(int x, int mu, float fScale)     { return 1.0f / (1.0f + pow(M_E, -(x - mu) / fScale)); }

K2_API  void    M_CalcAxisExtents(const CVec3f &v3Axis, const vector<CVec3f> &v3Points, float &fMin, float &fMax);
K2_API  void    M_CalcAxisExtents(const CVec3f &v3Axis, const CBBoxf &bbBox, float &fMin, float &fMax);

K2_API  float   M_LinePointDistance(const CVec3f &v3Origin, const CVec3f &v3Dir, const CVec3f &v3Point);

K2_API  float   M_GetDistanceSqVec2(const vec2_t pos1, const vec2_t pos2);
K2_API  CVec2f  M_ClosestPointToSegment2d(const CVec2f &v2A, const CVec2f &v2B, const CVec2f &v2Test);
inline float    M_DistanceSqToSegment2d(const CVec2f &v2A, const CVec2f &v2B, const CVec2f &v2Test)     { return DistanceSq(M_ClosestPointToSegment2d(v2A, v2B, v2Test), v2Test); }
K2_API  bool    M_RayIntersectsLineSeg2d(const vec2_t src, const vec2_t dir, const vec2_t line_src, const vec2_t line_dest, float epsilon);
K2_API  bool    M_2dBoundsIntersect(const vec2_t bmin_a, const vec2_t bmax_a, const vec2_t bmin_b, const vec2_t bmax_b);

K2_API  CVec3f  M_RotatePointAroundAxis(const CVec3f &v3In, const CVec3f &v3Dir, float fAngle);
K2_API  CVec3f  M_RotatePointAroundLine(const CVec3f &v3In, const CVec3f &v3Start, const CVec3f &v3End, float fAngle);

K2_API  bool    M_ClipLine(const CPlane &plPlane, CVec3f &p1, CVec3f &p2);

K2_API  uint    M_GetCRC32(const byte *pBuffer, uint uiSize);

K2_API  byte    M_GetAngle8(float fAngle);
K2_API  float   M_GetAngle(byte yAngle8);

#if 0 // You have failed me for the last time
K2_API  void    M_SeedRandom(unsigned long uiSeed);
K2_API  int     M_Random(int i);
K2_API  int     M_Random(int iLow, int iHigh);
K2_API  uint    M_Random(uint uiLow, uint uiHigh);
K2_API  float   M_Random(float fLow, float fHigh);
K2_API  float   M_Random();
#endif

K2_API  void    M_Init(LONGLONG rngseed);

#include "math_inlines.h"
//=============================================================================

#endif //__K2_MATHLIB_H__
