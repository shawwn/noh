// (C)2005 S2 Games
// c_movie_win32.cpp
//
// Bink implementation of the movie player class
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#ifdef K2_INCLUDE_BINK
#pragma comment(lib, "/lib/bink/binkw32.lib")

#include "bink.h"

#include "c_movie.h"
#include "c_filemanager.h"

#include "../k2/c_bitmap.h"
#endif
//=============================================================================

#ifdef K2_INCLUDE_BINK
/*====================
  MoviePlayer_Initialize
  ====================*/
void	MoviePlayer_Initialize()
{
	BinkSoundUseDirectSound(0);
}


/*====================
  MoviePlayer_Shutdown
  ====================*/
void	MoviePlayer_Shutdown()
{
}


/*====================
  CMovie::Load
  ====================*/
bool	CMovie::Load(const tstring &sFileName, CBitmap *pBitmap, bool bPreload)
{
	// Load the movie
	tstring sFullPath = FileManager.GetSystemPath(sFileName);

	char szFullPath[_MAX_PATH + 1];
	MemManager.Set(szFullPath, 0, _MAX_PATH + 1);
	TCHARToSingle(szFullPath, _MAX_PATH, sFullPath.c_str(), _MAX_PATH);

	HBINK hBink = BinkOpen(szFullPath, BINKALPHA | (bPreload ? BINKPRELOADALL : 0));
	m_pHandle = hBink;
	if (m_pHandle == NULL)
	{
		Console.Err << _T("CMovie::Load(): ") << sFullPath << newl
					<< BinkGetError() << newl;
		return false;
	}

	pBitmap->Alloc(M_NextPowerOfTwo(hBink->Width), M_NextPowerOfTwo(hBink->Height), (hBink->OpenFlags & BINKALPHA) ? BITMAP_RGBA : BITMAP_RGB);
	pBitmap->SetTranslucent((hBink->OpenFlags & BINKALPHA) != 0);

	// HACK: BMPType should be assumed to represent bytes per pixel
	m_iPitch =	pBitmap->GetWidth() * (pBitmap->GetBMPType() / 8);
	m_iWidth = pBitmap->GetWidth();
	m_iHeight = pBitmap->GetHeight();
	m_bAlpha = (hBink->OpenFlags & BINKALPHA) != 0;

	m_pBuffer = pBitmap->GetBuffer();
	return true;
}


/*====================
  CMovie::Unload
  ====================*/
void	CMovie::Unload()
{
	Stop();
	BinkSetSoundOnOff((HBINK)m_pHandle, 0);
	BinkClose((HBINK)m_pHandle);
}


/*====================
  CMovie::UpdateFrame
  ====================*/
void	CMovie::UpdateFrame()
{
	HBINK hBink = (HBINK)m_pHandle;

	if (BinkWait(hBink))
		return;

	BinkDoFrame(hBink);

	BinkCopyToBuffer(hBink, m_pBuffer, m_iPitch, m_iHeight, 0, 0, BINKSURFACE24R);

	if (hBink->FrameNum == hBink->Frames)
		++m_iPlayCount;

	BinkNextFrame(hBink);

	Console.Dev << _T("CMovie::GetFrame(): ") << hBink->FrameNum << _T("/") << hBink->Frames << newl;
}


/*====================
  CMovie::Stop
  ====================*/
void	CMovie::Stop()
{
	BinkPause((HBINK)m_pHandle, 1);
}


/*====================
  CMovie::Continue
  ====================*/
void	CMovie::Continue()
{
	BinkPause((HBINK)m_pHandle, 0);
}


/*====================
  CMovie::Restart
  ====================*/
void	CMovie::Restart()
{
	BinkGoto((HBINK)m_pHandle, 0, 0);
}

#endif K2_INCLUDE_BINK
