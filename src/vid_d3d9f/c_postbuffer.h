// (C)2008 S2 Games
// c_postbuffer.h
//
//=============================================================================
#ifndef __C_POSTBUFFER_H__
#define __C_POSTBUFFER_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/c_material.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
EXTERN_CVAR_BOOL(vid_postEffects);
const bool POST_EFFECTS_MIPMAP(false);
//=============================================================================

//=============================================================================
// CPostBuffer
//=============================================================================
class CPostBuffer
{
private:
	bool					m_bActive;

	uint					m_uiWidth;
	uint					m_uiHeight;
	
	int						m_aiPostBuffer[2];
	IDirect3DTexture9		*m_apPostBuffer[2];
	IDirect3DSurface9		*m_apPostBufferSurface[2];
	ResHandle				m_ahPostBufferHandle[2];

	int						m_iActiveBuffer;

	ResHandle				m_hNullPostMaterial;
	ResHandle				m_hPostColorReference;

	void					ToggleBuffer()			{ m_iActiveBuffer = 1 - m_iActiveBuffer; }

	IDirect3DTexture9*		GetCurrentTexture()		{ return m_apPostBuffer[m_iActiveBuffer]; }
	IDirect3DSurface9*		GetCurrentSurface()		{ return m_apPostBufferSurface[m_iActiveBuffer]; }
	ResHandle				GetCurrentHandle()		{ return m_ahPostBufferHandle[m_iActiveBuffer]; }

	IDirect3DTexture9*		GetNextTexture()		{ return m_apPostBuffer[1 - m_iActiveBuffer]; }
	IDirect3DSurface9*		GetNextSurface()		{ return m_apPostBufferSurface[1 - m_iActiveBuffer]; }
	ResHandle				GetNextHandle()			{ return m_ahPostBufferHandle[1 - m_iActiveBuffer]; }

public:
	~CPostBuffer();
	CPostBuffer();

	bool	GetActive()		{ return m_bActive; }

	void	Initialize(int iWidth, int iHeight);
	void	Release();

	void	Render();

	uint					GetWidth()				{ return m_uiWidth; }
	uint					GetHeight()				{ return m_uiHeight; }
};

extern CPostBuffer	g_PostBuffer;
//=============================================================================

#endif //__C_POSTBUFFER_H__
