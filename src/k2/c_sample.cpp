// (C)2005 S2 Games
// c_sample.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_sample.h"
#include "i_resourcelibrary.h"
#include "c_soundmanager.h"
#include "c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
IResource*	AllocSample(const tstring &sPath);
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
IResourceLibrary	g_ResLibSample(RES_SAMPLE, _T("Sound Samples"), CSample::ResTypeName(), false, AllocSample);
EXTERN_CVAR_INT(sound_maxVariations);
//=============================================================================

/*====================
  AllocSample
  ====================*/
IResource*	AllocSample(const tstring &sPath)
{
	return K2_NEW(ctx_Sound,  CSample)(sPath);
}


/*====================
  CSample::~CSample
  ====================*/
CSample::~CSample()
{
	Free();
}


/*====================
  CSample::CSample
  ====================*/
CSample::CSample(const tstring &sPath) :
IResource(sPath, TSNULL),
m_iSoundFlags(0),
m_bRandomSample(false),
m_pSampleData(NULL),
m_uiLastRandom(uint(-1))
{
}

CSample::CSample(FMOD::Sound *pSound) :
IResource(TSNULL, TSNULL),
m_iSoundFlags(0),
m_bRandomSample(false),
m_pSampleData(pSound),
m_uiLastRandom(uint(-1))
{
}

CSample::CSample(const tstring &sPath, int iSoundFlags) :
IResource(sPath, TSNULL),
m_iSoundFlags(iSoundFlags),
m_bRandomSample(false),
m_pSampleData(NULL),
m_uiLastRandom(uint(-1))
{
}

CSample::CSample(const tstring &sPath, int iSoundFlags, FMOD::Sound *pSound) :
IResource(sPath, TSNULL),
m_iSoundFlags(iSoundFlags),
m_bRandomSample(false),
m_pSampleData(pSound),
m_uiLastRandom(uint(-1))
{
}

/*====================
  CSample::Load
  ====================*/
int		CSample::Load(uint uiIgnoreFlags, const char *pData, uint uiSize)
{
	PROFILE("CSample::Load");

	try
	{
		size_t zPos(m_sPath.find(_T('%')));
		if (zPos != tstring::npos)
		{
			m_bRandomSample = true;
			m_vhSamples.clear();
		}

		if (m_bRandomSample)
		{
			if (m_vhSamples.empty())
			{
				const tstring &sFirstPart(m_sPath.substr(0, zPos));
				const tstring &sLastPart(m_sPath.substr(zPos + 1));

				m_bRandomSample = true;
				m_vhSamples.clear();

				int iSamples(1);

				tstring sPath(sFirstPart + XtoA(iSamples) + sLastPart);
				FMOD::Sound *pSound(K2SoundManager.LoadSample(sPath, m_iSoundFlags | SND_QUIETFAIL));

				while (pSound != NULL) 
				{
					m_vhSamples.push_back(g_ResourceManager.Register(K2_NEW(ctx_Sound,  CSample)(sPath, m_iSoundFlags, pSound), RES_SAMPLE));
					
					iSamples++;

					sPath = sFirstPart + XtoA(iSamples) + sLastPart;
					pSound = K2SoundManager.LoadSample(sPath, m_iSoundFlags | SND_QUIETFAIL);
				}

				if (m_vhSamples.empty())
					return RES_LOAD_FAILED;
			}
		}
		else
		{
			if (!m_sPath.empty())
				Console.Res << "Loading ^bSample^* " << SingleQuoteStr(m_sPath) << newl;
			else if (!m_sName.empty())
				Console.Res << "Loading ^bSample^* " << SingleQuoteStr(m_sName) << newl;
			else if (m_pSampleData != NULL && m_pData == NULL)
				Console.Res << "Loading ^bPrecreated Sample^*" << newl;
			else
				Console.Res << "Loading ^bUnknown Sample^*" << newl;

			if (m_pSampleData == NULL)
			{
				if (!m_sPath.empty())
					m_pSampleData = K2SoundManager.LoadSample(m_sPath, m_iSoundFlags);
				else if (!m_sName.empty())
					m_pSampleData = K2SoundManager.LoadSample(m_sName, m_iSoundFlags);

				if (m_pSampleData == NULL)
					return RES_LOAD_FAILED;
			}
		}
	}
	catch (CException &ex)
	{
		ex.Process(_TS("CSample::Load(") + m_sName + _TS(") - "), NO_THROW);
		return RES_LOAD_FAILED;
	}

	return 0;
}


/*====================
  CSample::Free
  ====================*/
void	CSample::Free()
{
	if (m_bRandomSample)
	{
		m_vhSamples.clear();
	}
	else if (m_pSampleData)
	{
		K2SoundManager.FreeSample(m_pSampleData);
	}
	m_pSampleData = NULL;
}


/*====================
  CSample::GetSampleData
  ====================*/
FMOD::Sound*	CSample::GetSampleData() const
{
	if (m_bRandomSample)
	{
		if (!m_vhSamples.empty())
		{
			uint uiRandom(M_Randnum(0u, INT_SIZE(m_vhSamples.size() - 1)));

			if (m_vhSamples.size() == 1 && uiRandom == m_uiLastRandom)
				uiRandom = M_Randnum(0u, INT_SIZE(m_vhSamples.size() - 1));
			else if (m_vhSamples.size() > 2)
			{
				while (uiRandom == m_uiLastRandom)
					uiRandom = M_Randnum(0u, INT_SIZE(m_vhSamples.size() - 1));
			}

			m_uiLastRandom = uiRandom;

			CSample *pSample(g_ResourceManager.GetSample(m_vhSamples[uiRandom]));
			if (pSample != NULL)
				return pSample->GetSampleData();
			else
				return NULL;
		}
		else
			return NULL;
	}
	else
		return m_pSampleData;
}

