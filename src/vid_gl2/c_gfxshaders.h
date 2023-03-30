// (C)2008 S2 Games
// c_gfxshaders.h
//
// Shaders
//=============================================================================
#ifndef __C_GFXSHADERS_H__
#define __C_GFXSHADERS_H__

//=============================================================================
// Headers
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const int	MAX_SHADERS(2048);

class CShaderVar;
class CShaderSampler;
class IMaterialParameter;
class CVertexShaderFileCallback;
class CPixelShaderFileCallback;

struct SVertexShaderSlot
{
	bool						bActive;
	uint						uiShader;
	CVertexShaderFileCallback	*pFileCallback;

	uint						uiCRC32;
};

struct SPixelShaderSlot
{
	bool						bActive;
	uint						uiShader;
	CPixelShaderFileCallback	*pFileCallback;

	uint						uiCRC32;
};

struct SShaderUniform
{
	tstring				sName;
	GLenum				eType;
	GLint				iLocation;
	GLenum				eTextureType;
	int					iTextureStage;
	CShaderVar*			pShaderVar;
	CShaderSampler*		pShaderSampler;
	uint				uiSubTexture;
};

struct SShaderAttribute
{
	tstring				sName;
	GLenum				eType;
	GLint				iLocation;
};

struct SShaderProgramSlot
{
	bool						bActive;
	uint						uiProgram;
	vector<SShaderAttribute>	vAttributes;
	vector<SShaderUniform>		vUniforms;
	int							iNumTextureStages;
};
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CMaterial;
class CVertexShader;
class CPixelShader;

void	D3D_InitShader();
void	GL_FrameShader();

void	GL_DestroyShader();
void	D3D_ShutdownShader();

extern SVertexShaderSlot			g_aVertexShaderSlots[MAX_SHADERS];
extern SPixelShaderSlot				g_aPixelShaderSlots[MAX_SHADERS];
extern SShaderProgramSlot			g_aShaderProgramSlots[MAX_SHADERS];

extern ResHandle					g_hNullMeshVS, g_hNullMeshPS;
extern ResHandle					g_hCloudTexture;

EXTERN_CVAR_BOOL(vid_shadowFalloff);
EXTERN_CVAR_BOOL(vid_shaderPrecache);
EXTERN_CVAR_BOOL(vid_shaderTexkill);
EXTERN_CVAR_BOOL(vid_shaderTexkillColorOnly);
EXTERN_CVAR_BOOL(vid_shaderRXGBNormalmap);
EXTERN_CVAR_INT(vid_shaderLightingQuality);
//=============================================================================

//=============================================================================
// CGfxShaders
//=============================================================================
class CGfxShaders
{
	SINGLETON_DEF(CGfxShaders)
private:
	void	UpdateDefineCvarInt(ICvar *pCvar, const string &sDefine);
	void	UpdateDefineCvarBool(CCvar<bool> *pCvar, const string &sDefine);

	bool	m_bReloadShaders;

public:
	~CGfxShaders();

	void	Init();
	void	Frame();
	void	Shutdown();

	bool	LoadVertexShader(const tstring &sFilename, int &iIndex);
	void	FreeVertexShader(int iShaderIndex);
	bool	LoadPixelShader(const tstring &sFilename, int &iIndex);
	void	FreePixelShader(int iShaderIndex);

	int		RegisterVertexShader(CVertexShader *pVertexShader);
	void	UnregisterVertexShader(CVertexShader *pVertexShader);

	int		RegisterPixelShader(CPixelShader *pPixelShader);
	void	UnregisterPixelShader(CPixelShader *pPixelShader);
	
	bool	LinkShaderProgram(int iVertexShader, int iPixelShader, int &iIndex);
	void	FreeShaderProgram(int iShaderIndex);

	void	RegisterShaderPair(CVertexShader *pVertexShader, CPixelShader *pPixelShader);
};
extern CGfxShaders *GfxShaders;
//=============================================================================

#endif //__C_GFXSHADERS_H__