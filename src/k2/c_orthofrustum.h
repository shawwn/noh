// (C)2005 S2 Games
// c_orthofrustum.h
//=============================================================================
#ifndef __C_ORTHOFRUSTUM_H__
#define __C_ORTHOFRUSTUM_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"
#include "c_vec.h"
#include "c_plane.h"
#include "c_boundingcone.h"
#include "c_axis.h"
//=============================================================================

//=============================================================================
// COrthoFrustum
//=============================================================================
class COrthoFrustum
{
	CVec3f	m_vOrigin;
	float	m_fWidth;
	float	m_fHeight;
	float	m_fNear;
	float	m_fFar;
	CAxis	m_Axis;

	CPlane	m_Planes[6];

	void	CalcPlanes();

public:
	K2_API	COrthoFrustum(const CVec3f &vOrigin, const CVec3f &vDirection, float fWidth, float fHeight, float fNear, float fFar);
	K2_API	COrthoFrustum(const CPlane &Planes);
	K2_API	COrthoFrustum(const vector<CVec3f> &vPoints, const CVec3f &vOrigin, const CVec3f &vDirection);
	K2_API	COrthoFrustum(const vector<CVec3f> &vPoints, const CVec3f &vDirection, float fSlideBack = 0.0f);
	K2_API	COrthoFrustum(const CBoundingCone &BoundingCone);

	const CVec3f&	GetOrigin()		{ return m_vOrigin; }
	const CAxis&	GetAxis()		{ return m_Axis; }
	float			GetWidth()		{ return m_fWidth; }
	float			GetHeight()		{ return m_fHeight; }
	float			GetNear()		{ return m_fNear; }
	float			GetFar()		{ return m_fFar; }

	K2_API	bool	Touches(const CBBoxf &bb, const CVec3f &v3Pos, const CAxis &axis) const;
	K2_API	bool	Touches(const CVec3f &vPoint) const;
};
//=============================================================================
#endif // __C_ORTHOFRUSTUM_H__
