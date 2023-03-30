// (C)2006 S2 Games
// c_shadowmap.h
//
//=============================================================================
#ifndef __C_SHADOWMAP_H__
#define __C_SHADOWMAP_H__

//=============================================================================
// Headers
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
class CCamera;

enum EShadowmapType
{
	SHADOWMAP_R32F = 0,
	SHADOWMAP_DEPTH,
	SHADOWMAP_VARIANCE,
	NUM_SHADOWMAP_TYPES
};

const char * const ShadowmapTypeName[] =
{
	"SHADOWMAP_R32F",
	"SHADOWMAP_DEPTH",
	"SHADOWMAP_VARIANCE",
};
//=============================================================================

//=============================================================================
// CShadowmap
//=============================================================================
class CShadowmap
{
private:
	bool						m_bActive;
	int							m_iShadowmapType;	
	int							m_iShadowmap;
	IDirect3DTexture9			*m_pShadowmap;
	IDirect3DTexture9			*m_pDepthShadowmap;
	IDirect3DSurface9			*m_pShadowmapSurface;
	IDirect3DSurface9			*m_pDSShadow;		// Depth-stencil buffer for rendering to shadow map

public:
	~CShadowmap();
	CShadowmap();

	int		GetShadowmapIndex()		{ return m_iShadowmap; }
	bool	GetActive()				{ return m_bActive; }
	int		GetType()				{ return m_iShadowmapType; }

	void	Initialize();
	void	Release();

	void	Setup(const CCamera &cCamera);
	void	Render();
};

extern CShadowmap	g_Shadowmap;
extern CCvar<bool>	vid_shadows;
extern CCvar<float>	vid_shadowSlopeBias;
extern CCvar<float>	vid_shadowDepthBias;
extern CCvar<float>	vid_shadowLeak;
extern CCvar<bool>	vid_shadowBackface;
extern CCvar<bool>	vid_shadows;
extern CCvar<float>	vid_shadowFalloffDistance;
extern CCvar<float>	vid_shadowDrawDistance;

extern CCvar<int>	vid_shadowmapSize;
extern CCvar<int>	vid_shadowmapType;
extern CCvar<int>	vid_shadowmapFilterWidth;
extern CCvar<bool>	vid_shadowmapMagFilter;
//=============================================================================
#endif //__C_SHADOWMAP_H__
