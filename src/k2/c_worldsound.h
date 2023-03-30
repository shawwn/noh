// (C)2007 S2 Games
// c_worldsound.h
//
//=============================================================================
#ifndef __C_WORLDSOUND_H__
#define __C_WORLDSOUND_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_range.h"
//=============================================================================

//=============================================================================
// CWorldSound
//=============================================================================
class CWorldSound
{
private:
	uint	m_uiIndex;
	CVec3f	m_v3Position;
	tstring	m_sSound;
	float	m_fVolume;
	float	m_fFalloff;
	CRangei	m_riLoopDelay;

public:
	~CWorldSound()	{}
	CWorldSound()	{}

	CWorldSound& operator=(const CWorldSound &sound)
	{
		// Clone sound settings
		m_v3Position = sound.m_v3Position;
		m_sSound = sound.m_sSound;
		m_fVolume = sound.m_fVolume;
		m_fFalloff = sound.m_fFalloff;
		m_riLoopDelay = sound.m_riLoopDelay;

		return *this;
	}

	uint			GetIndex() const					{ return m_uiIndex; }
	void			SetIndex(uint uiIndex)				{ m_uiIndex = uiIndex; }

	const CVec3f&	GetPosition() const					{ return m_v3Position; }
	void			SetPosition(const CVec3f &v3Pos)	{ m_v3Position = v3Pos; }
	void			Translate(const CVec3f &v3Pos)		{ m_v3Position += v3Pos; }

	const tstring&	GetSound() const					{ return m_sSound; }
	void			SetSound(const tstring &sSound)		{ m_sSound = sSound; }

	float			GetVolume() const					{ return m_fVolume; }
	void			SetVolume(float fVolume)			{ m_fVolume = fVolume; }

	float			GetFalloff() const					{ return m_fFalloff; }
	void			SetFalloff(float fFalloff)			{ m_fFalloff = fFalloff; }

	const CRangei&	GetLoopDelay() const				{ return m_riLoopDelay; }
	void			SetLoopDelay(const CRangei &riLoopDelay)	{ m_riLoopDelay = riLoopDelay; }
	void			SetLoopDelay(int iMin, int iMax)	{ m_riLoopDelay.Set(iMin, iMax); }
};
//=============================================================================

#endif //__C_WORLDSOUND_H__
