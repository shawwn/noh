// (C)2008 S2 Games
// c_gamma.cpp
//
// Gamma manager
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_gamma.h"

#include "d3d9_main.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CGamma	g_Gamma;
//=============================================================================

/*====================
  CGamma::~CGamma
  ====================*/
CGamma::~CGamma()
{
}


/*====================
  CGamma::CGamma
  ====================*/
CGamma::CGamma() :
m_fD3DGamma(0.0f),
m_fGDIGamma(0.0f),
m_bSaved(false)
{
}


/*====================
  CGamma::SaveGDIGammaRamp
  ====================*/
void	CGamma::SaveGDIGammaRamp()
{
	HDC hDC(GetDC(GetDesktopWindow()));
	m_bSaved = GetDeviceGammaRamp(hDC, &m_cSavedGDIGammaRamp) == TRUE;
	ReleaseDC(GetDesktopWindow(), hDC);
}


/*====================
  CGamma::RestoreGDIGammaRamp
  ====================*/
void	CGamma::RestoreGDIGammaRamp()
{
	if (!m_bSaved)
		return;

	HDC hDC(GetDC(GetDesktopWindow()));
	SetDeviceGammaRamp(hDC, &m_cSavedGDIGammaRamp);
	ReleaseDC(GetDesktopWindow(), hDC);
}


/*====================
  CGamma::UpdateD3DGamma
  ====================*/
void	CGamma::UpdateD3DGamma(float fGamma)
{
	if (m_fD3DGamma == fGamma)
		return;

	D3DGAMMARAMP cRamp;
	for (uint ui(0); ui < 256; ++ui)
	{
		ushort unValue(CLAMP(INT_ROUND(USHRT_MAX * pow(ui / 255.0f, 1.0f / fGamma)), 0, USHRT_MAX));

		cRamp.red[ui] = unValue;
		cRamp.green[ui] = unValue;
		cRamp.blue[ui] = unValue;
	}

	g_pd3dDevice->SetGammaRamp(0, D3DSGR_CALIBRATE, &cRamp);

	m_fD3DGamma = fGamma;
}


/*====================
  CGamma::UpdateGDIGamma
  ====================*/
void	CGamma::UpdateGDIGamma(float fGamma)
{
	if (m_fGDIGamma == fGamma)
		return;

	SGammaRamp cRamp;
	for (uint ui(0); ui < 256; ++ui)
	{
		ushort unValue(CLAMP(INT_ROUND(USHRT_MAX * pow(ui / 255.0f, 1.0f / fGamma)), 0, USHRT_MAX));

		cRamp.unRed[ui] = unValue;
		cRamp.unGreen[ui] = unValue;
		cRamp.unBlue[ui] = unValue;
	}

	SetDeviceGammaRamp(g_hDC, &cRamp);

	m_fGDIGamma = fGamma;
}


/*====================
  CGamma::Update
  ====================*/
void	CGamma::Update(float fGamma)
{
	if (g_bExclusive)
		UpdateD3DGamma(fGamma);
	else if (g_bFullscreen)
		UpdateGDIGamma(fGamma);
	else
		UpdateGDIGamma(1.0f);
}
