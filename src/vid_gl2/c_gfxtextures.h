// (C)2008 S2 Games
// c_gfxtextures.h
//
//=============================================================================
#ifndef __C_GFXTEXTURES_H__
#define __C_GFXTEXTURES_H__

//=============================================================================
// Definitions
//=============================================================================
class CProcedural;

void	GL_OpenTextureArchive(bool bNoReload);
void	GL_CloseTextureArchive();
bool	GL_TextureExists(const tstring &sFilename, uint uiTextureFlags);
void	GL_GetTextureList(const tstring &sPath, const tstring &sSearch, tsvector &vResult);

struct STextureMapEntry
{
	GLuint			uiTextureID;
	GLuint			uiTextureID2;
	ResHandle		hTexture;

	STextureMapEntry() {}
	STextureMapEntry(GLuint _uiTextureID, GLuint _uiTextureID2, ResHandle _hTexture) :
	uiTextureID(_uiTextureID), uiTextureID2(_uiTextureID2), hTexture(_hTexture) {}
};

typedef	map<tstring, STextureMapEntry>	TextureMap;

EXTERN_CVAR_FLOAT(vid_textureGammaCorrect);
//=============================================================================

//=============================================================================
// CGfxTextures
//=============================================================================
class CGfxTextures
{
	SINGLETON_DEF(CGfxTextures)

private:
	int				m_iWhite;
	int				m_iBlack;
	DefinitionMap	m_mapTextureDefinitions;

	inline	GLuint RegisterTextureID() { GLuint texID; glGenTextures(1, &texID); return texID; }

	void	UploadBitmap2D(const CBitmap &bitmap, int iTextureFlags, ETextureFormat eFormat, GLuint texLevel, bool genMipmaps);
	void	UploadBitmapCube(const CBitmap bitmap[], int iTextureFlags, ETextureFormat eFormat, GLuint texLevel);
	void	UploadBitmapVolume(const CBitmap bitmap[], int iDepth, int iTextureFlags, ETextureFormat eFormat, GLuint texLevel);

	void	GetBitmapInfo(const CBitmap &bitmap, const ETextureFormat &eFormat, GLint &internalFormat, GLenum &format, GLenum &type);
	bool	ResizeBitmap(const CBitmap &bitmap, CBitmap &bmpResized, int iTextureFlags);

	uint	RegisterArchivedTexture(CTexture *pTexture, const tstring &sSuffix = TSNULL);
	uint	RegisterCachedTexture(CTexture *pTexture);
	uint	RegisterCachedTexture(CTexture *pTexture, const tstring &sReference, const tstring &sSuffix);

public:
	~CGfxTextures();

	void	Init();
	void	Shutdown();

	int		GetWhiteTexture()			{ return m_iWhite; }
	int		GetBlackTexture()			{ return m_iBlack; }

	int		RegisterTexture(CTexture *pTexture);
	void	UnregisterTexture(const tstring &sName);

	GLuint	RegisterProcedural(const CProcedural &cProcedural, GLuint &uiTextureID2);
	void	Update2DTexture(const CBitmap &bitmap, int iTextureFlags, ETextureFormat eFormat, uint texID);
	uint	Register2DTexture(const CBitmap &bitmap, int iTextureFlags, ETextureFormat eFormat, const tstring &sName);
	uint	RegisterCubeTexture(const CBitmap bitmaps[], int iTextureFlags, ETextureFormat eFormat, const tstring &sName);
	uint	RegisterVolumeTexture(const CBitmap bitmaps[], int iDepth, int iTextureFlags, ETextureFormat eFormat, const tstring &sName);

	uint	RegisterRenderTargetTexture(const tstring &sName, int iSizeX, int iSizeY, GLint iInternalFormat, bool bMipmap);
	uint	RegisterDynamicTexture(const tstring &sName, int iSizeX, int iSizeY, GLint iInternalFormat, bool bMipmap);

	bool	ArchivedTextureExists(const tstring &sFilename, uint uiTextureFlags);

	void	TextureDefine(const string &sName, const string &sDefinition);
	void	TextureUndefine(const string &sName);
	string	GetTextureDefinitionString();

	void	SetDefaultTexParameters2D(int iTextureFlags);
	void	SetDefaultTexParametersCube(int iTextureFlags);
	void	SetDefaultTexParametersVolume(int iTextureFlags);

	TextureMap mapTextures;
};
extern CGfxTextures *GfxTextures;
//=============================================================================

#endif //__C_GFXTEXTURES_H__
