#ifndef __D3DX_SHARED_H__
#define __D3DX_SHARED_H__

// Defines
#include <limits.h>

#define D3DX_DEFAULT            ((UINT) -1)
#define D3DX_DEFAULT_NONPOW2    ((UINT) -2)
#define D3DX_DEFAULT_float      FLT_MAX
#define D3DX_FROM_FILE          ((UINT) -3)
#define D3DFMT_FROM_FILE        ((D3DFORMAT) -3)

#ifndef D3DXINLINE
#ifdef _MSC_VER
  #if (_MSC_VER >= 1200)
  #define D3DXINLINE __forceinline
  #else
  #define D3DXINLINE __inline
  #endif
#else
  #define D3DXINLINE inline
#endif
#endif

#define D3DX_PI    ((float)  3.141592654f)
#define D3DX_1BYPI ((float)  0.318309886f)

#define D3DXToRadian( degree ) ((degree) * (D3DX_PI / 180.0f))
#define D3DXToDegree( radian ) ((radian) * (180.0f / D3DX_PI))

typedef unsigned int D3DCOLOR;

// maps unsigned 8 bits/channel to D3DCOLOR
#if BYTE_ORDER == LITTLE_ENDIAN
#define D3DCOLOR_ARGB(a,r,g,b) \
    ((D3DCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#else
#define D3DCOLOR_ARGB(a,r,g,b) \
    ((D3DCOLOR)((((b)&0xff)<<24)|(((g)&0xff)<<16)|(((r)&0xff)<<8)|((a)&0xff)))
#endif
#define D3DCOLOR_RGBA(r,g,b,a) D3DCOLOR_ARGB(a,r,g,b)
#define D3DCOLOR_XRGB(r,g,b)   D3DCOLOR_ARGB(0xff,r,g,b)

#define D3DCOLOR_XYUV(y,u,v)   D3DCOLOR_ARGB(0xff,y,u,v)
#define D3DCOLOR_AYUV(a,y,u,v) D3DCOLOR_ARGB(a,y,u,v)

// maps floating point channels (0.0f to 1.0f range) to D3DCOLOR
#define D3DCOLOR_COLORVALUE(r,g,b,a) \
    D3DCOLOR_RGBA((DWORD)((r)*255.f),(DWORD)((g)*255.f),(DWORD)((b)*255.f),(DWORD)((a)*255.f))

#ifndef D3DVECTOR_DEFINED
typedef struct _D3DVECTOR {
    float x;
    float y;
    float z;
} D3DVECTOR;
#define D3DVECTOR_DEFINED
#endif

#ifndef D3DCOLORVALUE_DEFINED
typedef struct _D3DCOLORVALUE {
    float r;
    float g;
    float b;
    float a;
} D3DCOLORVALUE;
#define D3DCOLORVALUE_DEFINED
#endif

#ifndef D3DRECT_DEFINED
typedef struct _D3DRECT {
    long x1;
    long y1;
    long x2;
    long y2;
} D3DRECT;
#define D3DRECT_DEFINED
#endif

#ifndef D3DMATRIX_DEFINED
typedef struct _D3DMATRIX {
    union {
        struct {
            float        _11, _12, _13, _14;
            float        _21, _22, _23, _24;
            float        _31, _32, _33, _34;
            float        _41, _42, _43, _44;

        };
        float m[4][4];
    };
} D3DMATRIX;
#define D3DMATRIX_DEFINED
#endif

typedef struct _D3DVIEWPORT9 {
    unsigned long       X;
    unsigned long       Y;            /* Viewport Top left */
    unsigned long       Width;
    unsigned long       Height;       /* Viewport Dimensions */
    float				MinZ;         /* Min/max of clip Volume */
    float				MaxZ;
} D3DVIEWPORT9;

//===========================================================================
//
// Vectors
//
//===========================================================================

//--------------------------
// 2D Vector
//--------------------------
typedef struct D3DXVECTOR2
{
public:
    D3DXVECTOR2() {};
    D3DXVECTOR2( const float * );
    D3DXVECTOR2( float x, float y );

    // casting
    operator float* ();
    operator const float* () const;

    // assignment operators
    D3DXVECTOR2& operator += ( const D3DXVECTOR2& );
    D3DXVECTOR2& operator -= ( const D3DXVECTOR2& );
    D3DXVECTOR2& operator *= ( float );
    D3DXVECTOR2& operator /= ( float );

    // unary operators
    D3DXVECTOR2 operator + () const;
    D3DXVECTOR2 operator - () const;

    // binary operators
    D3DXVECTOR2 operator + ( const D3DXVECTOR2& ) const;
    D3DXVECTOR2 operator - ( const D3DXVECTOR2& ) const;
    D3DXVECTOR2 operator * ( float ) const;
    D3DXVECTOR2 operator / ( float ) const;

    friend D3DXVECTOR2 operator * ( float, const D3DXVECTOR2& );

    bool operator == ( const D3DXVECTOR2& ) const;
    bool operator != ( const D3DXVECTOR2& ) const;


public:
    float x, y;
} D3DXVECTOR2, *LPD3DXVECTOR2;

//--------------------------
// 3D Vector
//--------------------------
typedef struct D3DXVECTOR3 : public D3DVECTOR
{
public:
    D3DXVECTOR3() {};
    D3DXVECTOR3( const float * );
    D3DXVECTOR3( const D3DVECTOR& );
    D3DXVECTOR3( float x, float y, float z );

    // casting
    operator float* ();
    operator const float* () const;

    // assignment operators
    D3DXVECTOR3& operator += ( const D3DXVECTOR3& );
    D3DXVECTOR3& operator -= ( const D3DXVECTOR3& );
    D3DXVECTOR3& operator *= ( float );
    D3DXVECTOR3& operator /= ( float );

    // unary operators
    D3DXVECTOR3 operator + () const;
    D3DXVECTOR3 operator - () const;

    // binary operators
    D3DXVECTOR3 operator + ( const D3DXVECTOR3& ) const;
    D3DXVECTOR3 operator - ( const D3DXVECTOR3& ) const;
    D3DXVECTOR3 operator * ( float ) const;
    D3DXVECTOR3 operator / ( float ) const;

    friend D3DXVECTOR3 operator * ( float, const struct D3DXVECTOR3& );

    bool operator == ( const D3DXVECTOR3& ) const;
    bool operator != ( const D3DXVECTOR3& ) const;

} D3DXVECTOR3, *LPD3DXVECTOR3;

//--------------------------
// 4D Vector
//--------------------------
typedef struct D3DXVECTOR4
{
public:
    D3DXVECTOR4() {};
    D3DXVECTOR4( const float* );
    D3DXVECTOR4( const D3DVECTOR& xyz, float w );
    D3DXVECTOR4( float x, float y, float z, float w );

    // casting
    operator float* ();
    operator const float* () const;

    // assignment operators
    D3DXVECTOR4& operator += ( const D3DXVECTOR4& );
    D3DXVECTOR4& operator -= ( const D3DXVECTOR4& );
    D3DXVECTOR4& operator *= ( float );
    D3DXVECTOR4& operator /= ( float );

    // unary operators
    D3DXVECTOR4 operator + () const;
    D3DXVECTOR4 operator - () const;

    // binary operators
    D3DXVECTOR4 operator + ( const D3DXVECTOR4& ) const;
    D3DXVECTOR4 operator - ( const D3DXVECTOR4& ) const;
    D3DXVECTOR4 operator * ( float ) const;
    D3DXVECTOR4 operator / ( float ) const;

    friend D3DXVECTOR4 operator * ( float, const D3DXVECTOR4& );

    bool operator == ( const D3DXVECTOR4& ) const;
    bool operator != ( const D3DXVECTOR4& ) const;

public:
    float x, y, z, w;
} D3DXVECTOR4, *LPD3DXVECTOR4;

//===========================================================================
//
// Matrices
//
//===========================================================================
typedef struct D3DXMATRIX : public D3DMATRIX
{
public:
    D3DXMATRIX() {};
    D3DXMATRIX( const float * );
    D3DXMATRIX( const D3DMATRIX& );
    D3DXMATRIX( float _11, float _12, float _13, float _14,
                float _21, float _22, float _23, float _24,
                float _31, float _32, float _33, float _34,
                float _41, float _42, float _43, float _44 );


    // access grants
    float& operator () ( uint Row, uint Col );
    float  operator () ( uint Row, uint Col ) const;

    // casting operators
    operator float* ();
    operator const float* () const;

    // assignment operators
    D3DXMATRIX& operator *= ( const D3DXMATRIX& );
    D3DXMATRIX& operator += ( const D3DXMATRIX& );
    D3DXMATRIX& operator -= ( const D3DXMATRIX& );
    D3DXMATRIX& operator *= ( float );
    D3DXMATRIX& operator /= ( float );

    // unary operators
    D3DXMATRIX operator + () const;
    D3DXMATRIX operator - () const;

    // binary operators
    D3DXMATRIX operator * ( const D3DXMATRIX& ) const;
    D3DXMATRIX operator + ( const D3DXMATRIX& ) const;
    D3DXMATRIX operator - ( const D3DXMATRIX& ) const;
    D3DXMATRIX operator * ( float ) const;
    D3DXMATRIX operator / ( float ) const;

    friend D3DXMATRIX operator * ( float, const D3DXMATRIX& );

    bool operator == ( const D3DXMATRIX& ) const;
    bool operator != ( const D3DXMATRIX& ) const;

} D3DXMATRIX, *LPD3DXMATRIX;

//---------------------------------------------------------------------------
// Aligned Matrices
//
// This class helps keep matrices 16-byte aligned as preferred by P4 cpus.
// It aligns matrices on the stack and on the heap or in global scope.
// It does this using __declspec(align(16)) which works on VC7 and on VC 6
// with the processor pack. Unfortunately there is no way to detect the 
// latter so this is turned on only on VC7. On other compilers this is the
// the same as D3DXMATRIX.
//
// Using this class on a compiler that does not actually do the alignment
// can be dangerous since it will not expose bugs that ignore alignment.
// E.g if an object of this class in inside a struct or class, and some code
// memcopys data in it assuming tight packing. This could break on a compiler
// that eventually start aligning the matrix.
//---------------------------------------------------------------------------
typedef struct _D3DXMATRIXA16 : public D3DXMATRIX
{
    _D3DXMATRIXA16() {}
    _D3DXMATRIXA16( const float * );
    _D3DXMATRIXA16( const D3DMATRIX& );
    _D3DXMATRIXA16( float _11, float _12, float _13, float _14,
                    float _21, float _22, float _23, float _24,
                    float _31, float _32, float _33, float _34,
                    float _41, float _42, float _43, float _44 );

    // new operators
    void* operator new   ( size_t );
    void* operator new[] ( size_t );

    // delete operators
    void operator delete   ( void* );   // These are NOT virtual; Do not 
    void operator delete[] ( void* );   // cast to D3DXMATRIX and delete.
    
    // assignment operators
    _D3DXMATRIXA16& operator = ( const D3DXMATRIX& );

} _D3DXMATRIXA16;

#if _MSC_VER >= 1300  // VC7
#define D3DX_ALIGN16 __declspec(align(16))
#else
#define D3DX_ALIGN16  // Earlier compiler may not understand this, do nothing.
#endif

typedef D3DX_ALIGN16 _D3DXMATRIXA16 D3DXMATRIXA16, *LPD3DXMATRIXA16;

//===========================================================================
//
// Colors
//
//===========================================================================

typedef struct D3DXCOLOR
{
public:
    D3DXCOLOR() {}
    D3DXCOLOR( unsigned long argb );
    D3DXCOLOR( const float * );
    D3DXCOLOR( const D3DCOLORVALUE& );
    D3DXCOLOR( float r, float g, float b, float a );

    // casting
    operator unsigned long () const;

    operator float* ();
    operator const float* () const;

    operator D3DCOLORVALUE* ();
    operator const D3DCOLORVALUE* () const;

    operator D3DCOLORVALUE& ();
    operator const D3DCOLORVALUE& () const;

    // assignment operators
    D3DXCOLOR& operator += ( const D3DXCOLOR& );
    D3DXCOLOR& operator -= ( const D3DXCOLOR& );
    D3DXCOLOR& operator *= ( float );
    D3DXCOLOR& operator /= ( float );

    // unary operators
    D3DXCOLOR operator + () const;
    D3DXCOLOR operator - () const;

    // binary operators
    D3DXCOLOR operator + ( const D3DXCOLOR& ) const;
    D3DXCOLOR operator - ( const D3DXCOLOR& ) const;
    D3DXCOLOR operator * ( float ) const;
    D3DXCOLOR operator / ( float ) const;

    friend D3DXCOLOR operator * ( float, const D3DXCOLOR& );

    bool operator == ( const D3DXCOLOR& ) const;
    bool operator != ( const D3DXCOLOR& ) const;

    float r, g, b, a;
} D3DXCOLOR, *LPD3DXCOLOR;

// TODO: Function definitions we need
D3DXMATRIX* D3DXMatrixMultiply(D3DXMATRIX *pOut, const D3DXMATRIX *pM1, const D3DXMATRIX *pM2);
D3DXMATRIX* D3DXMatrixMultiplyTranspose(D3DXMATRIX *pOut, const D3DXMATRIX *pM1, const D3DXMATRIX *pM2);
D3DXMATRIX* D3DXMatrixInverse(D3DXMATRIX *pOut, float *pDeterminant, const D3DXMATRIX *pM);
D3DXMATRIX* D3DXMatrixTranslation(D3DXMATRIX *pOut, float x, float y, float z);
D3DXMATRIX* D3DXMatrixScaling(D3DXMATRIX *pOut, float sx, float sy, float sz);
D3DXMATRIX* D3DXMatrixTranspose(D3DXMATRIX *pOut, const D3DXMATRIX *pM);

D3DXVECTOR4* D3DXVec4Transform(D3DXVECTOR4 *pOut, const D3DXVECTOR4 *pV, const D3DXMATRIX *pM);
D3DXVECTOR3* D3DXVec3TransformNormal(D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV, const D3DXMATRIX *pM);
D3DXMATRIX* D3DXMatrixOrthoOffCenterRH(D3DXMATRIX *pOut, float l, float r, float b, float t, float zn, float zf);
D3DXVECTOR3* D3DXVec3Normalize(D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV);
D3DXVECTOR3* D3DXVec3Transform(D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV, const D3DXMATRIX *pM);

#include "d3dx_shared.inl"
#endif//__D3DX_SHARED_H__
