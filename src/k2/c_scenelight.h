// (C) 2005 S2 Games
// c_scenelight.h
//
//=============================================================================
#ifndef __C_SCENELIGHT_H__
#define __C_SCENELIGHT_H__

//=============================================================================
// CSceneLight
//=============================================================================
class CSceneLight
{
private:
	CVec3f	m_v3Position;
	CVec3f	m_v3Color;
	float	m_fFalloffStart;
	float	m_fFalloffEnd;

public:
	~CSceneLight()	{}
	CSceneLight(const CVec3f &v3Pos, const CVec3f &v3Color, float fFalloffStart, float fFalloffEnd);
	CSceneLight(const CSceneLight &light);

	const CVec3f&	GetPosition() const				{ return m_v3Position; }
	const CVec3f&	GetColor() const				{ return m_v3Color; }
	float	GetColor(EColorComponent e)				{ return m_v3Color[e]; }
	float	GetFalloffStart() const					{ return m_fFalloffStart; }
	float	GetFalloffEnd() const					{ return m_fFalloffEnd; }

	void	SetPosition(const CVec3f &v3Pos)		{ m_v3Position = v3Pos; }
	void	SetColor(const CVec3f &v3Color)			{ m_v3Color = v3Color; }
	void	SetColor(float fR, float fG, float fB)	{ m_v3Color.Set(fR, fG, fB); }
	void	SetColor(EColorComponent e, float f)	{ m_v3Color[e] = f; }
	void	SetFalloffStart(float fStart)			{ m_fFalloffStart = fStart; }
	void	SetFalloffEnd(float fEnd)				{ m_fFalloffEnd = fEnd; }
	void	ScaleFalloff(float fScale)				{ m_fFalloffStart *= fScale; m_fFalloffEnd *= fScale; }
};
//=============================================================================

/*====================
  CSceneLight::CSceneLight
  ====================*/
inline
CSceneLight::CSceneLight(const CVec3f &v3Pos = V_ZERO,
						 const CVec3f &v3Color = CVec3f(1.0f, 1.0f, 1.0f),
						 float fFalloffStart = 0.0f,
						 float fFalloffEnd = 1000.0f) :
m_v3Position(v3Pos),
m_v3Color(v3Color),
m_fFalloffStart(fFalloffStart),
m_fFalloffEnd(fFalloffEnd)
{
}

inline
CSceneLight::CSceneLight(const CSceneLight &light) :
m_v3Position(light.m_v3Position),
m_v3Color(light.m_v3Color),
m_fFalloffStart(light.m_fFalloffStart),
m_fFalloffEnd(light.m_fFalloffEnd)
{
}

#endif //__C_SCENELIGHT_H__
