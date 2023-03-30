// (C)2009 S2 Games
// c_plane2.h
//
// A plane in 2d space the form Ax + By = D
//=============================================================================
#ifndef __C_PLANE2_H__
#define __C_PLANE2_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_vec2.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const float PLANE2_EPSILON = 0.002f;

enum EPlane2Test
{
	PLANE2_POSITIVE = 0,
	PLANE2_NEGATIVE,
	PLANE2_INTERSECTS
};
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

//=============================================================================
// CPlane2
//=============================================================================
class CPlane2
{
public:
	CPlane2() {}
	CPlane2(float x, float y, float d) : v2Normal(x, y), fDist(d) {};
	CPlane2(const CVec2f &_v2Normal, float _fDist) : v2Normal(_v2Normal), fDist(_fDist)	{}

#if 0
	CPlane2(const CVec2f &a, const CVec2f &b)
	{
		CalcPlane(a, b);
	}

	CPlane2(const CVec2f &a, const CVec2f &b, bool bNormalized)
	{
		if (bNormalized)
			CalcPlaneNormalized(a, b);
		else
			CalcPlane(a, b);

	}

	CPlane2(const CVec2f &n, const CVec2f &p)
	{
		v2Normal = n;
		fDist = DotProduct(n, p);
	}
#endif

	bool	operator==(const CPlane2 &b) const	{ return v2Normal == b.v2Normal && fDist == b.fDist; }
	bool	operator!=(const CPlane2 &b) const	{ return v2Normal != b.v2Normal || fDist != b.fDist; }

	CPlane2	operator-() const					{ return CPlane2(-v2Normal.x, -v2Normal.y, -fDist); }

	void	Set(float x, float y, float _fDist)
	{
		v2Normal.Set(x, y);
		this->fDist = _fDist;
	}

	float	Normalize()
	{
		float fLength(v2Normal.Normalize());
		fDist /= fLength;

		return fLength;
	}

	void	CalcPlane(const CVec2f &a, const CVec2f &b, const CVec2f &c)
	{
#if 0
		v2Normal.x = a.y*(b.z - c.z) + b.y*(c.z - a.z) + c.y*(a.z - b.z);
		v2Normal.y = a.z*(b.x - c.x) + b.z*(c.x - a.x) + c.z*(a.x - b.x);
		v2Normal.z = a.x*(b.y - c.y) + b.x*(c.y - a.y) + c.x*(a.y - b.y);

		fDist = a.x*(b.y * c.z - c.y * b.z) + b.x*(c.y * a.z - a.y * c.z) + c.x*(a.y * b.z - b.y * a.z);
#endif
	}

	void	CalcPlaneNormalized(const CVec2f &a, const CVec2f &b, const CVec2f &c)
	{
#if 0
		v2Normal = CrossProduct(b - a, c - a);
		v2Normal.Normalize();
		fDist = DotProduct(v2Normal, a);
#endif
	}

	void	Scale(float fScale)	{ fDist = DotProduct(v2Normal * (fDist * fScale), v2Normal); }

#if 0
	EPlaneTest	Side(const CVec2f &v) const
	{
		float fDistance = DotProduct(v2Normal, v) - fDist;

		if (fDistance < -PLANE2_EPSILON)
			return PLANE_NEGATIVE;
		else if (fDistance > PLANE2_EPSILON)
			return PLANE_POSITIVE;
		else
			return PLANE_INTERSECTS;
	}

	EPlaneTest	Side(const vector<CVec2f> &vPoints) const
	{
		int iInside(0);
		int iOutside(0);

		for (vector<CVec2f>::const_iterator it(vPoints.begin()); it != vPoints.end(); ++it)
		{
			float fDistance = DotProduct(v2Normal, *it) - fDist;

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
#endif

#if 0
	EPlaneTest	Side(const CRectf &bbBox) const
	{
		const CVec2f &vMin(bbBox.GetMin());
		const CVec2f &vMax(bbBox.GetMax());

		CVec2f	v2Neg, v2Pos; // negative and positive vertices

		if (v2Normal.x > 0)
		{
			v2Pos.x = vMax.x;
			v2Neg.x = vMin.x;
		}
		else
		{
			v2Pos.x = vMin.x;
			v2Neg.x = vMax.x;
		}

		if (v2Normal.y > 0)
		{
			v2Pos.y = vMax.y;
			v2Neg.y = vMin.y;
		}
		else
		{
			v2Pos.y = vMin.y;
			v2Neg.y = vMax.y;
		}

		if (DotProduct(v2Normal, v2Pos) - fDist < 0)   // outside plane
			return PLANE_POSITIVE;
		else if (DotProduct(v2Normal, v2Neg) - fDist > 0)    // inside plane
			return PLANE_NEGATIVE;
		else
			return PLANE_INTERSECTS;   // intersects plane
	}
#endif

	bool	IsInside(const vector<CVec2f> &vPoints) const
	{
		for (vector<CVec2f>::const_iterator it(vPoints.begin()); it != vPoints.end(); ++it)
		{
			float fDistance = DotProduct(v2Normal, *it) - fDist;

			if (fDistance > PLANE_EPSILON)
				return false;
		}

		return true;
	}

	CVec2f	NearestPoint(const CVec2f &v2p) const
	{
		// Assumes the plane is normalized
		return v2p - v2Normal * (DotProduct(v2Normal, v2p) - fDist);
	}

	float	Distance(const CVec2f &v) const
	{
		// Assumes the plane is normalized
		return DotProduct(v2Normal, v) - fDist;
	}

	bool	IsValid() const				{ return v2Normal.IsValid() && (v2Normal.x != 0.0f || v2Normal.y != 0.0f) && _finite(fDist); }

	K2_API void	Transform(const CVec2f &pos, const class CAxis &axis, float fScale);

	void	Transform(const CVec2f &v2Pos)	{ fDist += DotProduct(v2Pos, v2Normal); }

	// Public for easy access
	CVec2f	v2Normal;
	float	fDist;
};
//=============================================================================

#endif	//__C_PLANE2_H__
