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
	NUM_SHADOWMAP_TYPES
};

const char * const ShadowmapTypeName[] =
{
	"SHADOWMAP_R32F",
	"SHADOWMAP_DEPTH"
};
//=============================================================================

//=============================================================================
// CShadowmap
//=============================================================================
class CShadowmap
{
private:
	bool						m_bActive;
	EShadowmapType				m_eShadowmapType;

	GLuint						m_uiFrameBufferObject;
	GLuint						m_uiShadowTexture;
	GLuint						m_uiDepthRenderBuffer;

public:
	~CShadowmap();
	CShadowmap();

	int		GetShadowmapIndex()		{ return m_uiShadowTexture; }
	bool	GetActive()				{ return m_bActive; }
	EShadowmapType	GetType()		{ return m_eShadowmapType; }

	void	Initialize(EShadowmapType eType);
	void	Release();

	void	Render(const CCamera &camera);
};

extern CShadowmap	g_Shadowmap;

EXTERN_CVAR_BOOL(vid_shadows);
EXTERN_CVAR_FLOAT(vid_shadowSlopeBias);
EXTERN_CVAR_FLOAT(vid_shadowDepthBias);
EXTERN_CVAR_FLOAT(vid_shadowLeak);
EXTERN_CVAR_BOOL(vid_shadowBackface);
EXTERN_CVAR_FLOAT(vid_shadowFalloffDistance);
EXTERN_CVAR_FLOAT(vid_shadowDrawDistance);

EXTERN_CVAR_INT(vid_shadowmapSize);
EXTERN_CVAR_INT(vid_shadowmapType);
EXTERN_CVAR_INT(vid_shadowmapFilterWidth);
EXTERN_CVAR_BOOL(vid_shadowmapMagFilter);
//=============================================================================
#endif //__C_SHADOWMAP_H__
