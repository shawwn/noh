// (C)2005 S2 Games
// c_camera.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_camera.h"
//=============================================================================

/*====================
  CCamera::SetTarget
  ====================*/
void	CCamera::SetTarget(const CVec3f &target)
{
	CVec3f dir = target - m_v3Origin;
	dir.Normalize();
	m_ViewAxis.SetFromForwardVec(dir);
}


/*====================
  CCamera::CalcFovY
  ====================*/
void	CCamera::CalcFovY()
{
	float a, x;

	x = m_fWidth / tan(DEG2RAD(m_fFovX * 0.5f));
	a = atan(m_fHeight / x);
	a = RAD2DEG(a) * 2.0f;

	m_fFovY = a;
	m_fAspect = m_fWidth / m_fHeight;
}


/*====================
  CCamera::CalcFovX
  ====================*/
void	CCamera::CalcFovX()
{
	float a, y;

	y = m_fHeight / tan(DEG2RAD(m_fFovY * 0.5f));
	a = atan(m_fWidth / y);
	a = RAD2DEG(a) * 2.0f;

	m_fFovX = a;
	m_fAspect = m_fWidth / m_fHeight;
}


/*====================
  CCamera::SetFovFromAspect
  ====================*/
void	CCamera::SetFovFromAspect(float fStdFovX, float fStdAspect, float fWeightX, float fWeightY)
{
	// First calculate the fovY from the fovX in the original aspect ratio
	m_fFovX = fStdFovX;
	m_fFovY = RAD2DEG(atan(tan(DEG2RAD(m_fFovX * 0.5f)) / fStdAspect)) * 2.0f;

	float Ax(2.0f * tan(DEG2RAD(m_fFovX * 0.5f)));
	float Ay(2.0f * tan(DEG2RAD(m_fFovY * 0.5f)));

	// Now find a solution in the current aspect ratio with the same gameplay area
	// weighted for the importance of each direction
	m_fAspect = m_fWidth / m_fHeight;

	float lX(fWeightX / (fWeightX + fWeightY));
	float lY(fWeightY / (fWeightX + fWeightY));

	float wX(Ax / m_fAspect);	// Fully weighted for x
	float xY(Ay);				// Fully weighted for y

	float y(pow(wX, lX) * pow(xY, lY)); // Weighted geometric mean
	float x(y * m_fAspect);

	m_fFovX = RAD2DEG(atan(x * 0.5f)) * 2.0f;
	m_fFovY = RAD2DEG(atan(y * 0.5f)) * 2.0f;
}


/*====================
  CCamera::IsPointInFront
  ====================*/
bool	CCamera::IsPointInFront(const CVec3f vec) const
{
	return DotProduct(m_ViewAxis.Forward(), vec) > 0.0f;
}


/*====================
  CCamera::ConstructRay

  Calculates the direction of a ray originating at the camera origin and
  passing through space down the window coord winx, winy
  ====================*/
CVec3f	CCamera::ConstructRay(float fWinX, float fWinY) const
{
	CVec3f result;
	float wx, wy;

	// Right side of window
	wx = tan(DEG2RAD(m_fFovX) * 0.5f);
	wx *= ((fWinX / m_fWidth) - 0.5f) * 2.0f;

	// Bottom side of window
	wy = -tan(DEG2RAD(m_fFovY) * 0.5f);
	wy *= ((fWinY / m_fHeight) - 0.5f) * 2.0f;

	for (int n = 0; n < 3; ++n)
		result[n] = m_ViewAxis[RIGHT][n] * wx + m_ViewAxis[UP][n] * wy + m_ViewAxis[FORWARD][n];

	result.Normalize();
	return result;
}


/*====================
  CCamera::ConstructRay2
  ====================*/
CVec3f	CCamera::ConstructRay2(float fX, float fY) const
{
	CVec3f result;
	float wx, wy;

	// Right side of window
	wx = tan(DEG2RAD(m_fFovX) * 0.5f);
	wx *= fX;

	// Bottom side of window
	wy = -tan(DEG2RAD(m_fFovY) * 0.5f);
	wy *= fY;

	for (int n = 0; n < 3; ++n)
		result[n] = m_ViewAxis[RIGHT][n] * wx + m_ViewAxis[UP][n] * wy + m_ViewAxis[FORWARD][n];

	result.Normalize();
	return result;
}


/*====================
  CCamera::DefaultCamera
  ====================*/
void	CCamera::DefaultCamera(float fScreenW, float fScreenH)
{
	m_pWorld = NULL;

	m_fX = m_fY = 0;
	m_fWidth = fScreenW;
	m_fHeight = fScreenH;

	SetFovXCalc(90.0f);
	ClearOrigin();
	SetAngles(V_ZERO);

	m_fTime = 0.0f;

	ClearFlags();
	m_iMode = 0;

	ClearFogBounds();

	m_fZNear = 0.0f;
	m_fZFar = 0.0f;

	m_plReflect = CPlane(0.0f, 0.0f, 1.0f, 0.0f);
}


/*====================
  CCamera::WorldToScreen
  ====================*/
bool	CCamera::WorldToScreen(const CVec3f &vecWorld, CVec2f &vecScreen) const
{
	float fCenterX(m_fWidth * 0.5f);
	float fCenterY(m_fHeight * 0.5f);

	CVec3f	v3Local(vecWorld - m_v3Origin);

	CVec3f	v3Transformed;
	v3Transformed.x = DotProduct(v3Local, m_ViewAxis[RIGHT]);
	v3Transformed.y = DotProduct(v3Local, m_ViewAxis[UP]);
	v3Transformed.z = DotProduct(v3Local, m_ViewAxis[FORWARD]);

	// make sure z is not negative
	if (v3Transformed.z < 0.001f)
		return false;

	vecScreen.x = fCenterX + fCenterX / v3Transformed.z * v3Transformed.x / tan(DEG2RAD(m_fFovX * 0.5f)) + 0.5f;
	vecScreen.y = fCenterY - fCenterY / v3Transformed.z * v3Transformed.y / tan(DEG2RAD(m_fFovY * 0.5f)) + 0.5f;

	return true;
}


/*====================
  CCamera::WorldToScreenBack
  ====================*/
void	CCamera::WorldToScreenBack(const CVec3f &vecWorld, CVec2f &vecScreen) const
{
	float fCenterX(m_fWidth * 0.5f);
	float fCenterY(m_fHeight * 0.5f);

	CVec3f	v3Local(vecWorld - m_v3Origin);

	CVec3f	v3Transformed;
	v3Transformed.x = DotProduct(v3Local, m_ViewAxis[RIGHT]);
	v3Transformed.y = DotProduct(v3Local, m_ViewAxis[UP]);
	v3Transformed.z = DotProduct(v3Local, m_ViewAxis[FORWARD]);

	// make sure z is not 0
	if (ABS(v3Transformed.z) < 0.001f)
	{
		vecScreen = V2_ZERO;
		return;
	}
	else if (v3Transformed.z > 0.0f)
	{
		vecScreen.x = fCenterX + fCenterX / v3Transformed.z * v3Transformed.x / tan(DEG2RAD(m_fFovX * 0.5f)) + 0.5f;
		vecScreen.y = fCenterY - fCenterY / v3Transformed.z * v3Transformed.y / tan(DEG2RAD(m_fFovY * 0.5f)) + 0.5f;
	}
	else
	{
		vecScreen.x = fCenterX + fCenterX / -v3Transformed.z * v3Transformed.x / tan(DEG2RAD(m_fFovX * 0.5f)) + 0.5f;
		vecScreen.y = fCenterY - fCenterY / -v3Transformed.z * v3Transformed.y / tan(DEG2RAD(m_fFovY * 0.5f)) + 0.5f;
	}
}


/*====================
  CCamera::WorldToView
  ====================*/
CVec2f	CCamera::WorldToView(const CVec3f &v3World) const
{
	CVec3f v3Local(v3World - m_v3Origin);
	
	CVec3f v3Transformed
	(
		DotProduct(v3Local, m_ViewAxis[RIGHT]),
		DotProduct(v3Local, m_ViewAxis[UP]),
		ABS(DotProduct(v3Local, m_ViewAxis[FORWARD]))
	);

	return CVec2f
	(
		v3Transformed.x / tan(DEG2RAD(m_fFovX * 0.5f)) / v3Transformed.z,
		v3Transformed.y / tan(DEG2RAD(m_fFovY * 0.5f)) / v3Transformed.z
	);
}


/*====================
  CCamera::WorldToViewRotate
  ====================*/
CVec2f	CCamera::WorldToViewRotate(const CVec3f &v3Dir) const
{
	CVec2f v3Transformed
	(
		DotProduct(v3Dir, m_ViewAxis[RIGHT]),
		DotProduct(v3Dir, m_ViewAxis[UP])
	);

	return CVec2f
	(
		v3Transformed.x / tan(DEG2RAD(m_fFovX * 0.5f)),
		v3Transformed.y / tan(DEG2RAD(m_fFovY * 0.5f))
	);
}


/*====================
  CCamera::IsPointInScreenRect
  ====================*/
bool	CCamera::IsPointInScreenRect(const CVec3f &vOrigin, const CRectf &rect)
{
	CVec2f pos;

	if (WorldToScreen(vOrigin, pos) && rect.Contains(pos))
		return true;
	else
		return false;
}


/*====================
  CCamera::AddOffset
  ====================*/
void	CCamera::AddOffset(const CVec3f &v3Offset)
{
	m_v3Origin += m_ViewAxis[RIGHT] * v3Offset.x;
	m_v3Origin += m_ViewAxis[UP] * v3Offset.y;
	m_v3Origin += m_ViewAxis[FORWARD] * v3Offset.z;
}
