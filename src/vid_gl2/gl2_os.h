// (C)2008 S2 Games
// gl2_os.h
//
//=============================================================================
#ifndef __GL2_OS_H__
#define __GL2_OS_H__

//=============================================================================
// Definitions
//=============================================================================
extern bool					g_bFullscreen;
extern bool					gl_initialized;
extern int					g_iCurrentVideoMode;
extern SVidMode				g_CurrentVidMode;
extern SVidMode				g_VidModes[MAX_VID_MODES];
extern int					g_iNumVidModes;
extern SAAMode				g_CurrentAAMode;
extern SAAMode				g_AAModes[MAX_AA_MODES];
extern int					g_iNumAAModes;
extern CCvar<bool>			*vid_overbright;
extern const float			DEFAULT_OVERBRIGHT;
// texture settings
extern GLint				g_textureMagFilter;
extern GLint				g_textureMinFilter;
extern GLint				g_textureMinFilterMipmap;
extern GLfloat				g_textureMaxAnisotropy;

struct SDeviceCaps
{
	bool	bNonSquareMatrix;
	GLint	iMaxVaryingFloats;
	bool	bTextureCompression;
};
extern SDeviceCaps g_DeviceCaps;


// In gl_main.cpp
void	GL_GetDeviceCaps();
bool	GL_GetMode(int mode, SVidMode *vidmode);
void	InitAPIs_Global(SVidDriver *vid_api);
int		GL_Global_Init();
void	GL_Global_EndFrame();
void	GL_Global_Shutdown();
void	GL_Global_Start();
void	GL_SetupTextureFilterSettings();

// In gl_win32.cpp, gl_linux.cpp and gl_mac.cpp
void	GL_Start();
void*	GL_GetHWnd();
void	GL_Shutdown();
void	GL_EndFrame();
int		GL_Init();
void	GL_SetupWindow();
int		GL_SetMode();
bool	GL_GetCurrentMode(SVidMode *pVidMode);
void	GL_SetGamma(float fGamma);
void	GL_ShowCursor(bool bShow);
void	GL_SetCursor(ResHandle hCursor);

// In gl2_linux.cpp
#ifdef linux
void	GL_X11_Event(XEvent* pEvent);
#endif
//=============================================================================

#endif //__GL2_OS_H__

