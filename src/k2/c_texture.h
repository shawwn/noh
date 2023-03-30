// (C)2005 S2 Games
// c_texture.h
//
//=============================================================================
#ifndef __C_TEXTURE__
#define __C_TEXTURE__

//=============================================================================
// Headers
//=============================================================================
#include "i_resource.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CBitmap;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================

// texture flags
const int TEX_FULL_QUALITY		(BIT(0));	// don't resize this texture if possible
const int TEX_NO_COMPRESS		(BIT(1));	// don't use texture compression on this texture
const int TEX_NO_MIPMAPS		(BIT(2));	// don't generate mipmaps for this texture
const int TEX_NO_DISABLE		(BIT(3));	// render this texture even when textures are disabled
const int TEX_ON_DEMAND			(BIT(4));	// don't load until this texture is first used
const int TEX_MONO_AS_ALPHA		(BIT(5));	// don't load until this texture is first used

enum ETextureType
{
	TEXTURE_2D = 0,
	TEXTURE_VOLUME,
	TEXTURE_CUBE,
	NUM_TEXTURE_TYPES
};

enum ETextureFormat
{
	TEXFMT_INVALID = -1,

	TEXFMT_A8R8G8B8 = 0,
	TEXFMT_A1R5G5B5,
	TEXFMT_A4R4G4B4,
	TEXFMT_A8,
	TEXFMT_A8L8,
	TEXFMT_R16G16,
	TEXFMT_U8V8,
	TEXFMT_U16V16,
	TEXFMT_R16F,
	TEXFMT_R16G16F,
	TEXFMT_A16R16G16B16F,
	TEXFMT_R32F,
	TEXFMT_R32G32F,
	TEXFMT_A32R32G32B32F,
	TEXFMT_NORMALMAP,
	TEXFMT_NORMALMAP_RXGB,
	TEXFMT_NORMALMAP_S,

	NUM_TEXTURE_FORMATS
};

const uint RES_TEXTURE_IGNORE_ALL(BIT(0));

//=============================================================================

//=============================================================================
// CTexture
//=============================================================================
class CTexture : public IResource
{
private:
	int				m_iIndex;		// internal index set by the renderer
	int				m_iIndex2;
	ETextureType	m_eType;
	int				m_iTextureFlags;
	ETextureFormat	m_eFormat;
	const CBitmap*	m_pBitmap;
	bool			m_bTranslucent;

public:
	K2_API ~CTexture()	{}
	K2_API CTexture(const tstring &sPath);
	K2_API CTexture(const tstring &sPath, ETextureType eType, int iTextureFlags, ETextureFormat eFormat);
	K2_API CTexture(const tstring &sPath, const CBitmap *pBitmap, ETextureType eType, int iTextureFlags, ETextureFormat eFormat);

	K2_API	virtual uint			GetResType() const			{ return RES_TEXTURE; }
	K2_API	virtual const tstring&	GetResTypeName() const		{ return ResTypeName(); }
	K2_API	static const tstring&	ResTypeName()				{ static tstring sTypeName(_T("{texture}")); return sTypeName; }

	bool			HasBitmap()							{ return m_pBitmap != NULL; }
	void			SetBitmap(const CBitmap *pBitmap)	{ m_pBitmap = pBitmap; }
	const CBitmap*	GetBitmap()							{ return m_pBitmap; }

	void			SetIndex(int iIndex)				{ m_iIndex = iIndex; }
	int				GetIndex() const					{ return m_iIndex; }

	void			SetIndex2(int iIndex)				{ m_iIndex2 = iIndex; }
	int				GetIndex2() const					{ return m_iIndex2; }

	int				GetTextureFlags() const				{ return m_iTextureFlags; }
	ETextureType	GetType() const						{ return m_eType; }
	ETextureFormat	GetFormat() const					{ return m_eFormat; }

	bool			IsTranslucent() const				{ return m_bTranslucent; }
	void			SetTranslucent(bool value)			{ m_bTranslucent = value; }

	int				Load(uint uiIgnoreFlags, const char *pData, uint uiSize);
	bool			LoadNull();
	void			Free();
};
//=============================================================================
#endif //__C_TEXTURE__
