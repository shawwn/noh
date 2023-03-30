// (C)2005 S2 Games
// c_sample.h
//
//=============================================================================
#ifndef __C_SAMPLE__
#define __C_SAMPLE__

//=============================================================================
// Headers
//=============================================================================
#include "i_resource.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
namespace FMOD
{
	class Sound;
}
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
// Sound flags
const int SND_2D				(BIT(0));	// Disable 3d processing, and allow for stereo playback
const int SND_LOOP				(BIT(1));	// Looping sound
const int SND_STREAM			(BIT(2));	// Stream, instead of loading it into memory
const int SND_LINEARFALLOFF		(BIT(3));	// Linear falloff
const int SND_SQUAREDFALLOFF	(BIT(4));	// Squared falloff
const int SND_COMPRESSED		(BIT(5));	// Keep compressed in memory
const int SND_QUIETFAIL			(BIT(6));	// Don't complain about not loading
//=============================================================================

//=============================================================================
// CSample
//=============================================================================
class CSample : public IResource
{
private:
	int				m_iSoundFlags;
	bool			m_bRandomSample;

	FMOD::Sound*		m_pSampleData;
	vector<ResHandle>	m_vhSamples;
	mutable uint		m_uiLastRandom;

public:
	K2_API ~CSample();
	K2_API CSample(const tstring &sPath);
	K2_API CSample(const tstring &sPath, int iSoundFlags);
	K2_API CSample(const tstring &sPath, int iSoundFlags, FMOD::Sound *pSound);
	K2_API CSample(FMOD::Sound *pSound);

	K2_API	virtual uint			GetResType() const			{ return RES_SAMPLE; }
	K2_API	virtual const tstring&	GetResTypeName() const		{ return ResTypeName(); }
	K2_API	static const tstring&	ResTypeName()				{ static tstring sTypeName(_T("{sample}")); return sTypeName; }

	int		Load(uint uiIgnoreFlags, const char *pData, uint uiSize);
	void	Free();

	int				GetSoundFlags() const	{ return m_iSoundFlags; }
	
	FMOD::Sound*	GetSampleData() const;

	uint			GetNumSamples() const	{ return uint(m_vhSamples.size()); }
	ResHandle		GetSample(uint uiIndex) { return m_vhSamples[uiIndex]; }
};
//=============================================================================


#endif //__C_SAMPLE__
