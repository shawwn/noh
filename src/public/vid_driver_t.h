// (C)2005 S2 Games
// SVidDriver.h
//
//=============================================================================
#ifndef __VID_DRIVER_T__
#define __VID_DRIVER_T__

//=============================================================================
// Declarations
//=============================================================================
struct SVidMode;
struct SAAMode;
class CTexture;
class CVertexShader;
class CPixelShader;
class CModel;
class CBitmap;
class CCamera;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EVidNotifyMessage
{
	VID_NOTIFY_NEW_WORLD = 1,
	//a new world has been loaded
	VID_NOTIFY_TERRAIN_COLOR_MODIFIED,
	//the terrain colormap or dynamap has been modified
	//param1: xpos   param2: ypos
	//param3: bool indicating if ALL terrain should be rebuilt (if so, first 2 params are ignored)
	VID_NOTIFY_TERRAIN_VERTEX_MODIFIED,
	//world.grid has been modified
	//param1: xpos   param2: ypos
	//param3: bool indicating if ALL terrain should be rebuilt (if so, first 2 params are ignored)
	VID_NOTIFY_TERRAIN_SHADER_MODIFIED,
	//a shader on the terrain has been changed
	//param1: xpos   param2: ypos
	//param3: bool indicating if ALL terrain should be rebuilt (if so, first 2 params are ignored)
	VID_NOTIFY_TERRAIN_TEXCOORD_MODIFIED,
	//unused
	VID_NOTIFY_TERRAIN_NORMAL_MODIFIED,
	//a normal on the terrain has been changed
	//param1: xpos   param2: ypos
	//param3: bool indicating if ALL terrain should be rebuilt (if so, first 2 params are ignored)
	VID_NOTIFY_TERRAIN_TEXEL_ALPHA_MODIFIED,
	VID_NOTIFY_WORLD_DESTROYED,
	//the world has been destroyed
	VID_NOTIFY_FOLIAGE_DENSITY_MODIFIED,
	//...
	VID_NOTIFY_FOLIAGE_SIZE_MODIFIED,
	//...
	VID_NOTIFY_FOLIAGE_TEXTURE_MODIFIED,
	//...
	VID_NOTIFY_TEXTURE_FILTERING_SETTINGS_MODIFIED,
	//texture filtering settings modified
	VID_NOTIFY_X11_EVENT,
	//pass along X11 events that the renderer needs to know about
	VID_NOTIFY_UPDATE_SHADERS,
	//update shaders
	VID_NOTIFY_RELOAD_SHADER_CACHE,
	//reload shader cache
	VID_NOTIFY_ADD_CLIFF,
	VID_NOTIFY_REMOVE_CLIFF,
	VID_NOTIFY_REBUILD_CLIFFS,
	VID_NOTIFY_FOG_OF_WAR
};

enum ETextureFilteringModes
{
	TEXTUREFILTERING_NONE = 0,
	TEXTUREFILTERING_BILINEAR,
	TEXTUREFILTERING_TRILINEAR,
	TEXTUREFILTERING_ANISOTROPIC2,
	TEXTUREFILTERING_ANISOTROPIC4,
	TEXTUREFILTERING_ANISOTROPIC6,
	TEXTUREFILTERING_ANISOTROPIC8,
	TEXTUREFILTERING_ANISOTROPIC12,
	TEXTUREFILTERING_ANISOTROPIC16,
	TEXTUREFILTERING_ANISOTROPIC32,

	NUM_TEXTUREFILTERING_MODES
};

// This structure holds pointers to all fundamental graphics functions
struct SVidDriver
{
	tstring			sDriverName;

	int				(*Init)();
	void			(*Start)();
	int				(*SetMode)();
	bool			(*GetMode)(int iMode, SVidMode *pVidMode);
	bool			(*GetCurrentMode)(SVidMode *pVidMode);
	bool			(*IsFullScreen)();
	void			(*Shutdown)();
	
	bool			(*TextureFilteringModeAvailable)(ETextureFilteringModes eMode);

	// Rendering
	void			(*BeginFrame)();
	void			(*EndFrame)();
	void			(*RenderScene)(class CCamera &camera);
	void			(*Add2dRect)(float x, float y, float w, float h, float s1, float t1, float s2, float t2, ResHandle hTexture, int iFlags);
	void			(*Add2dQuad)(const CVec2f& v1, const CVec2f& v2, const CVec2f& v3, const CVec2f& v4,
									const CVec2f& t1, const CVec2f& t2, const CVec2f& t3, const CVec2f& t4, ResHandle hTexture, int iFlags);
	void			(*Add2dLine)(const CVec2f& v1, const CVec2f& v2, const CVec4f &v4Color1, const CVec4f &v4Color2, int iFlags);
	void			(*AddPoint)(const CVec3f &v3Point, const CVec4f &v4Color);
	void			(*AddLine)(const CVec3f &v3Start, const CVec3f &v3End, const CVec4f &v4Color);
	void			(*SetColor)(CVec4f v4Color);
	void			(*Notify)(enum EVidNotifyMessage eMsg, int iParam1, int iParam2, int iParam3, void *pData, const tstring &sResContext);
	void			(*GetFrameBuffer)(CBitmap &bmp);
	CVec2f			(*ProjectVertex)(const class CCamera &cam, const CVec3f &vecVertex);

	void*			(*GetHWnd)();
	void			(*Clear)();

	void			(*OpenTextureArchive)(bool bNoReload);
	void			(*CloseTextureArchive)();
	void			(*GetTextureList)(const tstring &sPath, const tstring &sSearch, tsvector &vResult);
	bool			(*TextureExists)(const tstring &sFilename, uint uiTextureFlags);

	int				(*RegisterTexture)(CTexture *pTexture);
	void			(*UnregisterTexture)(CTexture *pTexture);

	int				(*RegisterVertexShader)(CVertexShader *pVertexShader);
	void			(*UnregisterVertexShader)(CVertexShader *pVertexShader);

	int				(*RegisterPixelShader)(CPixelShader *pPixelShader);
	void			(*UnregisterPixelShader)(CPixelShader *pPixelShader);

	void			(*RegisterShaderPair)(CVertexShader *pVertexShader, CPixelShader *pPixelShader);

	int				(*RegisterModel)(CModel *pModel);
	void			(*UnregisterModel)(CModel *pModel);

	const CCamera*	(*GetCamera)();

	void			(*RenderFogofWar)(float fClear, bool bTexture, float fLerp);
	void			(*UpdateFogofWar)(const CBitmap &cBmp);

	void			(*ShowCursor)(bool bShow);
	void			(*SetCursor)(ResHandle hCursor);

	CVec4f			(*GetTextureColor)(CTexture *pTexture);

	bool			(*GetAAMode)(int iMode, SAAMode *pAAmode);
	bool			(*GetCurrentAAMode)(SAAMode *pAAmode);
};
//=============================================================================

#endif // __VID_DRIVER_T__
