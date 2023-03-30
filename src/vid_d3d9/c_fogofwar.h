// (C)2007 S2 Games
// c_fogofwar.h
//
//=============================================================================
#ifndef __C_FOGOFWAR_H__
#define __C_FOGOFWAR_H__

//=============================================================================
// Headers
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CFogofWar
//=============================================================================
class CFogofWar
{
private:
	bool						m_bValid;
	int							m_iTexturemap;
	IDirect3DTexture9			*m_pTexturemap;
	IDirect3DSurface9			*m_pTexturemapSurface;

	int							m_iDynamicTexturemap0;
	int							m_iDynamicTexturemap1;
	int							m_iNextTexturemap;

	ResHandle					m_hFogofWarTexture;
	ResHandle					m_hFogofWarTexture0;
	ResHandle					m_hFogofWarTexture1;

	float						m_fWorldWidth;
	float						m_fWorldHeight;

	uint						m_uiBMPWidth;
	uint						m_uiBMPHeight;

	uint						m_uiRTWidth;
	uint						m_uiRTHeight;

	uint						m_uiFilterWidth;
	uint						m_uiFilterHeight;

	int							m_iSize;

	void	DrawTexture(float fLerp);

public:
	~CFogofWar();
	CFogofWar();

	int		GetTextureIndex()		{ return m_iTexturemap; }

	void	Initialize(uint uiWidth, uint uiHeight, int iSize);
	void	Release();

	void	Render(float fClear, bool bTexture, float fLerp);
	void	Update(const CBitmap &cBmp);

	void	AddRect(float x, float y, float w, float h, float s1, float t1, float s2, float t2, ResHandle hTexture);

	bool	IsValid() const			{ return m_bValid; }

	uint	GetWidth() const		{ return m_uiBMPWidth; }
	uint	GetHeight() const		{ return m_uiBMPHeight; }
	int		GetSize() const			{ return m_iSize; }
};

extern CFogofWar		g_FogofWar;
//=============================================================================
#endif //__C_FOGOFWAR_H__
