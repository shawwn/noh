// (C)2005 S2 Games
// c_convexpolygon.h
//=============================================================================
#ifndef __C_CONVEXPOLYGON_H__
#define __C_CONVEXPOLYGON_H__

//=============================================================================
// Headers
//=============================================================================
#include "intersection.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
const int	MAX_CONVEXPOLYGON_VERTS(8);

struct SPolyVertex
{
	SPolyVertex() {}

	SPolyVertex(const CVec3f &_v3Pos, float _f0, float _f1) :
	v3Pos(_v3Pos), f0(_f0), f1(_f1)
	{
	}

	CVec3f	v3Pos;
	float	f0;
	float	f1;
};
//=============================================================================

//=============================================================================
// CConvexPolygon
//=============================================================================
class CConvexPolygon
{
private:
	SPolyVertex		m_aVerts[MAX_CONVEXPOLYGON_VERTS];
	uint			m_uiNumVerts;

public:
	~CConvexPolygon() {}
	CConvexPolygon() : m_uiNumVerts(0) {}
	inline CConvexPolygon(const CConvexPolygon &c);

	uint				GetNumVerts()		{ return m_uiNumVerts; }
	const SPolyVertex&	GetVertex(uint n)	{ return m_aVerts[n]; }

	inline void		AddVertex(const CVec3f &v3Point, float f0, float f1);
	inline void		ProjectXY(const CPlane &plPlane);
	inline void		Clip(const CPlane &plPlane);
};
//=============================================================================

/*====================
  CConvexPolygon::CConvexPolygon
  ====================*/
CConvexPolygon::CConvexPolygon(const CConvexPolygon &c) : 
m_uiNumVerts(c.m_uiNumVerts)
{
	MemManager.Copy(m_aVerts, c.m_aVerts, sizeof(SPolyVertex) * m_uiNumVerts);
}


/*====================
  CConvexPolygon::AddVertex
  ====================*/
void	CConvexPolygon::AddVertex(const CVec3f &v3Point, float f0, float f1)
{
	if (m_uiNumVerts >= MAX_CONVEXPOLYGON_VERTS)
		return;

	m_aVerts[m_uiNumVerts++] = SPolyVertex(v3Point, f0, f1);
}


/*====================
  CConvexPolygon::ProjectXY
  ====================*/
void	CConvexPolygon::ProjectXY(const CPlane &plPlane)
{
	for (uint uiVert(0); uiVert != m_uiNumVerts; ++uiVert)
		m_aVerts[uiVert].v3Pos.z = plPlane.GetHeight(m_aVerts[uiVert].v3Pos.x, m_aVerts[uiVert].v3Pos.y);
}


/*====================
  CConvexPolygon::Clip

  Clip everything on the positive side of the plane
  ====================*/
void	CConvexPolygon::Clip(const CPlane &plPlane)
{
	if (m_uiNumVerts < 2)
		return;

	SPolyVertex		aNewVerts[MAX_CONVEXPOLYGON_VERTS];
	uint			uiNumNewVerts(0);

	bool bPositive(plPlane.Distance(m_aVerts[0].v3Pos) > 0.0f);

	for (uint uiVert(1); uiVert != m_uiNumVerts; ++uiVert)
	{
		if (bPositive)
		{
			bPositive = plPlane.Distance(m_aVerts[uiVert].v3Pos) > 0.0f;

			if (bPositive)
				continue;
			else 
			{
				// Crossing from negative into positive
				float fFraction(1.0f);
				if (I_LineDoubleSidedPlaneIntersect(m_aVerts[uiVert - 1].v3Pos, m_aVerts[uiVert].v3Pos, plPlane, fFraction))
					aNewVerts[uiNumNewVerts++] =
						SPolyVertex
						(
							LERP(fFraction, m_aVerts[uiVert - 1].v3Pos, m_aVerts[uiVert].v3Pos),
							LERP(fFraction, m_aVerts[uiVert - 1].f0, m_aVerts[uiVert].f0),
							LERP(fFraction, m_aVerts[uiVert - 1].f1, m_aVerts[uiVert].f1)
						);

				aNewVerts[uiNumNewVerts++] = m_aVerts[uiVert];
			}
		}
		else
		{
			bPositive = plPlane.Distance(m_aVerts[uiVert].v3Pos) > 0.0f;

			if (!bPositive)
				aNewVerts[uiNumNewVerts++] = m_aVerts[uiVert];
			else
			{
				// Crossing from positive into negative
				float fFraction(1.0f);
				if (I_LineDoubleSidedPlaneIntersect(m_aVerts[uiVert - 1].v3Pos, m_aVerts[uiVert].v3Pos, plPlane, fFraction))
					aNewVerts[uiNumNewVerts++] =
						SPolyVertex
						(
							LERP(fFraction, m_aVerts[uiVert - 1].v3Pos, m_aVerts[uiVert].v3Pos),
							LERP(fFraction, m_aVerts[uiVert - 1].f0, m_aVerts[uiVert].f0),
							LERP(fFraction, m_aVerts[uiVert - 1].f1, m_aVerts[uiVert].f1)
						);
			}
		}
	}

	if (bPositive)
	{
		bPositive = plPlane.Distance(m_aVerts[0].v3Pos) > 0.0f;

		if (!bPositive)
		{
			// Crossing from negative into positive
			float fFraction(1.0f);
			if (I_LineDoubleSidedPlaneIntersect(m_aVerts[m_uiNumVerts - 1].v3Pos, m_aVerts[0].v3Pos, plPlane, fFraction))
				aNewVerts[uiNumNewVerts++] =
					SPolyVertex
					(
						LERP(fFraction, m_aVerts[m_uiNumVerts - 1].v3Pos, m_aVerts[0].v3Pos),
						LERP(fFraction, m_aVerts[m_uiNumVerts - 1].f0, m_aVerts[0].f0),
						LERP(fFraction, m_aVerts[m_uiNumVerts - 1].f1, m_aVerts[0].f1)
					);

			aNewVerts[uiNumNewVerts++] = m_aVerts[0];
		}
	}
	else
	{
		bPositive = plPlane.Distance(m_aVerts[0].v3Pos) > 0.0f;

		if (!bPositive)
			aNewVerts[uiNumNewVerts++] = m_aVerts[0];
		else
		{
			// Crossing from positive into negative
			float fFraction(1.0f);
			if (I_LineDoubleSidedPlaneIntersect(m_aVerts[m_uiNumVerts - 1].v3Pos, m_aVerts[0].v3Pos, plPlane, fFraction))
				aNewVerts[uiNumNewVerts++] = 
					SPolyVertex
					(
						LERP(fFraction, m_aVerts[m_uiNumVerts - 1].v3Pos, m_aVerts[0].v3Pos),
						LERP(fFraction, m_aVerts[m_uiNumVerts - 1].f0, m_aVerts[0].f0),
						LERP(fFraction, m_aVerts[m_uiNumVerts - 1].f1, m_aVerts[0].f1)
					);
		}
	}

	MemManager.Copy(m_aVerts, aNewVerts, sizeof(SPolyVertex) * uiNumNewVerts);
	m_uiNumVerts = uiNumNewVerts;
}

//=============================================================================
#endif //__C_CONVEXPOLYGON_H__
