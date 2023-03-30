// (C)2005 S2 Games
// c_sphere.h
//=============================================================================
#ifndef __C_SPHERE_H__
#define __C_SPHERE_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_vec3.h"
//=============================================================================

//=============================================================================
// CSphere
//=============================================================================
class CSphere
{
private:
	CVec3f	m_vCenter;
	float	m_fRadius;

public:
	~CSphere() {}

	// Constructors
	CSphere() {}

	CSphere(const CVec3f &vCenter, float fRadius) :
	m_vCenter(vCenter),
	m_fRadius(fRadius)
	{
	}

	// Accessors
	const CVec3f&	GetCenter() const	{ return m_vCenter; }
	float			GetRadius() const	{ return m_fRadius; }

	// Mutators
	void			SetCenter(const CVec3f &vCenter) { m_vCenter = vCenter; }
	void			SetRadius(float fRadius) { m_fRadius = fRadius; }
};
//=============================================================================
#endif // __C_SPHERE_H__
