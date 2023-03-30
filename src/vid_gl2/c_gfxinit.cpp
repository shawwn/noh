// (C)2008 S2 Games
// c_gfxinit.cpp
//
// API and renderer initialization
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_gfxinit.h"

#include "c_gfxtextures.h"
#include "c_gfxshaders.h"
#include "c_gfxterrain.h"
#include "c_gfxmaterials.h"
#include "c_gfx2d.h"
#include "gl2_foliage.h"
#include "c_shadowmap.h"
#include "c_fogofwar.h"
#include "c_texturecache.h"
#include "c_scenebuffer.h"
#include "c_postbuffer.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
SINGLETON_INIT(CGfxInit)
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CGfxInit *GfxInit(CGfxInit::GetInstance());

CVAR_BOOLF(vid_checkOpenGLVersion, true, CVAR_SAVECONFIG);
EXTERN_CVAR_BOOL(vid_textureCompression);
//=============================================================================

/*====================
  CGfxInit::CGfxInit
  ====================*/
CGfxInit::CGfxInit()
{
}


/*====================
  CGfxInit::~CGfxInit
  ====================*/
CGfxInit::~CGfxInit()
{
}


/*====================
  CGfxInit::Init
  ====================*/
void	CGfxInit::Init()
{
	g_TextureCache.Initialize();
}


/*====================
  CGfxInit::Start
  ====================*/
void	CGfxInit::Start()
{
	glewInit();
	glDisable(GL_NORMALIZE);
	glDepthFunc(GL_LEQUAL);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	
	const char *szRenderer((const char*)glGetString(GL_RENDERER));
	const char *szVersion((const char*)glGetString(GL_VERSION));
	const char *szGLSLVersion((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION_ARB));
	const char *szVendor((const char*)glGetString(GL_VENDOR));
	const char *szExtensions((const char*)glGetString(GL_EXTENSIONS));
	
	Console.Video << _T("OpenGL Renderer: ") << (szRenderer ? szRenderer : "NULL") << newl;
	Console.Video << _T("OpenGL Version: ") << (szVersion ? szVersion : "NULL") << newl;
	Console.Video << _T("OpenGL Shading Language Version: ") << (szGLSLVersion ? szGLSLVersion : "NULL") << newl;
	Console.Video << _T("OpenGL Vendor: ") << (szVendor ? szVendor : "NULL") << newl;
	Console.Video << _T("OpenGL Extensions: ") << (szExtensions ? szExtensions : "NULL") << newl;

	if (vid_checkOpenGLVersion)
	{
		if (!GLEW_VERSION_1_2)
			K2System.Error(_T("OpenGL 1.2 not available."));
#define REQUIRE(x) if (!GLEW_##x) K2System.Error(_T(#x) _T(" not available."));
		REQUIRE(ARB_multitexture);
		REQUIRE(ARB_vertex_buffer_object);
		REQUIRE(ARB_shader_objects);
		REQUIRE(ARB_fragment_shader);
		REQUIRE(ARB_vertex_shader);
		REQUIRE(ARB_shading_language_100);
		REQUIRE(ARB_pixel_buffer_object);
		REQUIRE(EXT_framebuffer_object);
#undef REQUIRE

		if (!szGLSLVersion || AtoF(szGLSLVersion) < 1.10f)
			K2System.Error(_T("GLSL 1.10 not available."));
	}

	GL_GetDeviceCaps();
	
	Console.Video << _T("OpenGL Max Varying Floats: ") << g_DeviceCaps.iMaxVaryingFloats << newl;
	Console.Video << _T("Texture compression ") << (g_DeviceCaps.bTextureCompression && vid_textureCompression ? _T("Enabled") : _T("Disabled")) << newl;

	GfxTextures->Init();
	GfxShaders->Init();
	GfxMaterials->Init();
	GfxTerrain->Init();
	GL_InitFoliage();
	Gfx2D->Init();
	Gfx3D->Init();

	g_Shadowmap.Initialize(EShadowmapType(int(vid_shadowmapType)));
	g_FogofWar.Initialize();
	g_SceneBuffer.Initialize(g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight);
	g_PostBuffer.Initialize(g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight);
}
