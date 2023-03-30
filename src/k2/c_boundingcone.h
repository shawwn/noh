// (C)2005 S2 Games
// c_boundingcone.h
//=============================================================================
#ifndef __C_BOUNDINGCONE_H__
#define __C_BOUNDINGCONE_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"
#include "c_vec.h"
//=============================================================================


//=============================================================================
// CBoundingCone
//=============================================================================
class CBoundingCone
{
	CVec3f	m_vApex;
	CVec3f	m_vDirection;
	float	m_fFov;
	float	m_fNear;
	float	m_fFar;

public:
	K2_API	CBoundingCone(const vector<CVec3f> &vPoints, const CVec3f &vApex, const CVec3f &vDirection);
	K2_API	CBoundingCone(const vector<CVec3f> &vPoints, const CVec3f &vApex);

	const CVec3f&	GetApex()		{ return m_vApex; }
	const CVec3f&	GetDirection()	{ return m_vDirection; }
	float			GetFov()		{ return 2.0f * RAD2DEG(m_fFov); }
	float			GetNear()		{ return m_fNear; }
	float			GetFar()		{ return m_fFar; }
};
//=============================================================================
#endif // __C_BOUNDINGCONE_H__
