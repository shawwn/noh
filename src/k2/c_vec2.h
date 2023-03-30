// (C)2005 S2 Games
// c_vec2.h
//=============================================================================
#ifndef __C_VEC2_H__
#define __C_VEC2_H__

//=============================================================================
// Headers
//=============================================================================
#include <math.h>
#include <float.h>
//=============================================================================

//=============================================================================
// SVec2<T>
//
// struct version of CVec2 for statically initialized data
//=============================================================================
template <class T>
struct SVec2
{
	T	x, y;
};
//=============================================================================

//=============================================================================
// CVec2<T>
//=============================================================================
template <class T> class CVec2;
template <class T> CVec2<T>	CrossProduct(const CVec2<T> &a, const CVec2<T> &b);
template <class T> T		DotProduct(const CVec2<T> &a, const CVec2<T> &b);
template <class T> T		Distance(const CVec2<T> &a, const CVec2<T> &b);
template <class T>
class CVec2
{
public:
	CVec2()												{}
	CVec2(T _x, T _y)			: x(_x), y(_y)			{}
	CVec2(const T xy[2])		: x(xy[X]), y(xy[Y])	{}
	CVec2(const SVec2<T> &v)	: x(v.x), y(v.y)		{}
	CVec2(T _xy)				: x(_xy), y(_xy)		{}

				operator T*()						{ return reinterpret_cast<T*>(this); }
				operator const T*() const			{ return reinterpret_cast<const T*>(this); }

	const T&	operator[](int i) const				{ return *((&x) + i); }
	T&			operator[](int i)					{ return *((&x) + i); }
	const T&	operator[](uint ui) const			{ return *((&x) + ui); }
	T&			operator[](uint ui)					{ return *((&x) + ui); }

	bool operator==(const CVec2 &b) const	{ return (x == b.x && y == b.y); }
	bool operator!=(const CVec2 &b) const	{ return (x != b.x || y != b.y); }

	void Set(T x2, T y2)					{ x = x2; y = y2; }
	void Clear()							{ x = 0.0f; y = 0.0f; }

	bool IsValid() const					{ return _finite(x) && _finite(y); }

	CVec2<T> &operator+=(const CVec2<T> &b)	{ x += b.x; y += b.y; return *this; }
	CVec2<T> &operator-=(const CVec2<T> &b)	{ x -= b.x; y -= b.y; return *this; }
	CVec2<T> &operator*=(const CVec2<T> &b)	{ x *= b.x; y *= b.y; return *this; }
	CVec2<T> &operator/=(const CVec2<T> &b)	{ x /= b.x; y /= b.y; return *this; }
	CVec2<T> &operator+=(T s)				{ x += s; y += s; return *this; }
    CVec2<T> &operator-=(T s)				{ x -= s; y -= s; return *this; }
	CVec2<T> &operator*=(T s)				{ x *= s; y *= s; return *this; }
    CVec2<T> &operator/=(T s)				{ x /= s; y /= s; return *this; }

	CVec2<T> operator+(const CVec2<T> &b) const	{ return CVec2<T>(x + b.x, y + b.y); }
	CVec2<T> operator-(const CVec2<T> &b) const	{ return CVec2<T>(x - b.x, y - b.y); }
	CVec2<T> operator*(T s) const			{ return CVec2<T>(x * s, y * s); }
	CVec2<T> operator/(T s) const			{ return CVec2<T>(x / s, y / s); }

	CVec2<T> operator-() const				{ return CVec2<T>(-x, -y); }

	const vec2_t*	vec2() const			{ return reinterpret_cast<const vec2_t *>(this);   }

	CVec2<T> GetInverse() const				{ return CVec2<T>(-x, -y); }
	T Length() const						{ return sqrt(x * x + y * y); }
	T LengthSq() const						{ return x * x + y * y; }

	T Normalize()
	{
		float	flLength(sqrt(x * x + y * y));

		if (flLength == 0.0f)
		{
			x = 0.0f;
			y = 0.0f;
			return 0.0f;
		}

		x /= flLength;
		y /= flLength;

		return flLength;
	}

	CVec2<T> Direction() const
	{
		CVec2<T> temp(*this);
		temp.Normalize();
		return temp;
	}

	const CVec2<T>&	SetLength(float fLength)
	{
		Normalize();
		x *= fLength;
		y *= fLength;
		return *this;
	}

	CVec2<T> &Invert()
	{
		x = -x;
		y = -y;
		return *this;
	}

	CVec2<T>&	Rotate(float fAngle)
	{
		fAngle = DEG2RAD(-fAngle);
		T _x = x * cos(fAngle) + y * sin(fAngle);
		T _y = x * -sin(fAngle) + y * cos(fAngle);

		x = _x;
		y = _y;
		return *this;
	}

	CVec2<T>&	Clip(const CVec2<T> &n, float fFudge = 1.000f)
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

	CVec2<T>&	Project(const CVec2<T> &a)
	{
		*this = a * (DotProduct(a, *this) / DotProduct(a, a));

		return *this;
	}

	inline bool InBounds(const CVec2<T> &vecMin, const CVec2<T> &vecMax) const;
	inline void AddToBounds(CVec2<T> &vecMin, CVec2<T> &vecMax) const;

	// Public to allow direct access
	T	x, y;
};
//=============================================================================

//=============================================================================
// Export an instance of CVec3<float>
// See http://support.microsoft.com/default.aspx?scid=kb;EN-US;168958
//=============================================================================
#ifdef _WIN32
#pragma warning (disable : 4231)
K2_EXTERN template class K2_API CVec2<float>;
#pragma warning (default : 4231)
#endif
//=============================================================================

//=============================================================================
// Member functions
//=============================================================================

/*====================
  CVec2<T>::InBounds
  ====================*/
template <class T>
bool	CVec2<T>::InBounds(const CVec2<T> &vecMin, const CVec2<T> &vecMax) const
{
	if (x < vecMin.x || y < vecMin.y ||
		x > vecMax.x || y > vecMax.y)
		return false;

	return true;
}

/*====================
  CVec2<T>::AddToBounds
  ====================*/
template <class T>
void	CVec2<T>::AddToBounds(CVec2<T> &vecMin, CVec2<T> &vecMax) const
{
	if (x < vecMin.x)
		vecMin.x = x;
	if (y < vecMin.y)
		vecMin.y = y;

	if (x > vecMax.x)
		vecMax.x = x;
	if (y > vecMax.y)
		vecMax.y = y;
}

//=============================================================================
// Friend functions
//=============================================================================

#if 0 // TODO: CrossProduct
/*====================
  CrossProduct
  ====================*/
template <class T>
inline CVec2<T> CrossProduct(const CVec2<T> &a, const CVec2<T> &b)
{

	return CVec2<T>(
		a.x,
		a.y
	);
}
#endif


/*====================
  DotProduct
  ====================*/
template <class T>
inline T	DotProduct(const CVec2<T> &a, const CVec2<T> &b)
{
	return a.x * b.x + a.y * b.y;
}


/*====================
  Normalize
  ====================*/
template <class T>
inline CVec2<T>	Normalize(const CVec2<T> &a)
{
	float flLength = sqrt(a.x * a.x + a.y * a.y);

	if (flLength == 0.0f)
	{
		return CVec2<T>(0, 0);
	}

	float flInvLength = 1.0f / flLength;
	return CVec2<T>(a.x * flInvLength, a.y * flInvLength);
}


/*====================
  Distance
  ====================*/
template <class T>
inline T	Distance(const CVec2<T> &a, const CVec2<T> &b)
{
	T	dx = a.x - b.x;
	T	dy = a.y - b.y;

	return static_cast<T>(sqrt(dx * dx + dy * dy));
}

/*====================
  DistanceSq
  ====================*/
template <class T>
inline T	DistanceSq(const CVec2<T> &a, const CVec2<T> &b)
{
	T	dx = a.x - b.x;
	T	dy = a.y - b.y;

	return dx * dx + dy * dy;
}


/*====================
  Compare
  ====================*/
template <class T>
bool	Compare(const CVec2<T> &a, const CVec2<T> &b)
{
	if (fabs(a.x - b.x) > 0.001 ||
		fabs(a.y - b.y) > 0.001)
	{
		return false;
	}
	return true;
}


/*====================
  Compare
  ====================*/
template <class T>
bool	Compare(const CVec2<T> &a, const CVec2<T> &b, float fEpsilon)
{
	if (fabs(a.x - b.x) > fEpsilon ||
		fabs(a.y - b.y) > fEpsilon)
	{
		return false;
	}
	return true;
}

//=============================================================================
// Non-template functions
//=============================================================================
typedef SVec2<float>	SVec2f;
typedef SVec2<int>		SVec2i;
typedef SVec2<short>	SVec2s;
typedef SVec2<byte>		SVec2b;
typedef SVec2<LONGLONG>	SVec2ll;

typedef CVec2<float>	CVec2f;
typedef CVec2<int>		CVec2i;
typedef CVec2<uint>		CVec2ui;
typedef CVec2<short>	CVec2s;
typedef CVec2<ushort>	CVec2us;
typedef CVec2<byte>		CVec2b;
typedef CVec2<LONGLONG>	CVec2ll;

#define CVec2_cast(v) (*(CVec2f*)(&((v)[X])))
#define vec2_cast(v) (*(vec2_t*)(&(v)))

/*====================
  M_PointOnLine
  ====================*/
inline
CVec2f M_PointOnLine(const CVec2f &origin, const CVec2f &dir, float t)
{
	return origin + dir * t;
}

#endif // __C_VEC2_H__
