// (C)2005 S2 Games
// c_fontmap.h
//
//=============================================================================
#ifndef __C_FONTMAP_H__
#define __C_FONTMAP_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_resource.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
struct FT_FaceRec_;
class CBitmap;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EFontCharacterRange
{
	FONT_RANGE_LATIN,
	FONT_RANGE_GREEK,
	FONT_RANGE_CYRILLIC,
	FONT_RANGE_ARMENIAN,
	FONT_RANGE_HEBREW,
	FONT_RANGE_ARABIC,
	FONT_RANGE_DEVANGARI,
	FONT_RANGE_BENGALI,
	FONT_RANGE_GURMUKHI,
	FONT_RANGE_GUJARATI,
	FONT_RANGE_THAI,
	FONT_RANGE_HANGUL_JAMO,
	FONT_RANGE_TAGALOG,
	FONT_RANGE_MONGOLIAN,
	FONT_RANGE_LATIN_ADDITION,
	FONT_RANGE_HIRAGANA,
	FONT_RANGE_KATAKANA,
	FONT_RANGE_HANGUL_SYLLABLE,
	FONT_RANGE_CJK,

	NUM_CHARACTER_RANGES
};

class CCharacterRange
{
private:
	EFontCharacterRange	m_eRange;
	uint				m_uiFirstChar;
	uint				m_uiLastChar;
	uint				m_uiNumChars;

	CCharacterRange();

public:
	~CCharacterRange()	{}
	CCharacterRange(EFontCharacterRange eRange, uint uiFirstChar, uint uiLastChar) :
	m_eRange(eRange),
	m_uiFirstChar(uiFirstChar),
	m_uiLastChar(uiLastChar),
	m_uiNumChars(uiLastChar - uiFirstChar + 1)
	{}

	uint	GetRange() const		{ return m_eRange; }
	uint	GetFirstChar() const	{ return m_uiFirstChar; }
	uint	GetLastChar() const		{ return m_uiLastChar; }
	uint	GetNumChars() const		{ return m_uiNumChars; }
};

const int FONT_STYLE_ITALIC			(BIT(0));
const int FONT_STYLE_BOLD			(BIT(1));
const int FONT_STYLE_FIXED_WIDTH	(BIT(2));

struct SCharacterMapInfo
{
	float	m_fWidth;
	float	m_fHeight;
	float	m_fAdvance;
	float	m_fBearingX;
	float	m_fBearingY;
	float	m_fS1;
	float	m_fT1;
	float	m_fS2;
	float	m_fT2;
};

typedef vector<SCharacterMapInfo>	CharInfoMap;
//=============================================================================

//=============================================================================
// CFontMap
//=============================================================================
class CFontMap : public IResource
{
private:
	FT_FaceRec_*	m_pFace;
	ResHandle		m_hFontFace;
	int				m_iFaceSize;
	int				m_iStyle;
	uint			m_uiCharacterRanges;
	int				m_iNumCharacters;
	int				m_iNumValidGlyphs;

	int				m_iMaxAdvance;
	int				m_iMaxHeight;
	int				m_iMaxAscender;
	int				m_iFixedAdvance;

	int				m_iMaxBitmapHeight;
	int				m_iMaxBitmapWidth;

	int				m_iPenX;
	int				m_iPenY;

	CharInfoMap			m_vCharInfo[NUM_CHARACTER_RANGES];
	SCharacterMapInfo	m_MissingCharInfo;

	ResHandle		m_hTextureMap;
	CBitmap*		m_pBitmap;

	bool			m_bDynamicResize;
	int				m_iBaseResolution;
	tstring			m_sAxis;

	bool	AllocateBitmap(int iTotalWidth);
	void	AdvancePen(int iWidth);
	int		PointsToPixels(int iPoints)		{ return INT_ROUND(iPoints / 64.0f); }
	int		PointsToPixelsCeil(int iPoints)	{ return INT_CEIL(iPoints / 64.0f); }
	int		AddCharacterRange(uint uiFirstChar, uint uiLastChar, int iCurrentTotalWidth);

	K2_API const SCharacterMapInfo*	ActuallyGetCharMapInfo(uint uiIndex);

public:
	~CFontMap()	{}
	CFontMap(const tstring &sName, int iSize, int iStyle, uint uiChracters, ResHandle hFontFace, bool bDynamicResize, int iBaseResolution, const tstring &sAxis);

	K2_API	virtual uint			GetResType() const			{ return RES_FONTMAP; }
	K2_API	virtual const tstring&	GetResTypeName() const		{ return ResTypeName(); }
	K2_API	static const tstring&	ResTypeName()				{ static tstring sTypeName(_T("{fontmap}")); return sTypeName; }

	int		Load(uint uiIgnoreFlags, const char *pData, uint uiSize);
	void	Free();

	float	GetMaxAdvance() const	{ return float(m_iMaxAdvance); }
	float	GetMaxHeight() const	{ return float(m_iMaxHeight); }
	float	GetMaxAscender() const	{ return float(m_iMaxAscender); }
	float	GetFixedAdvance() const	{ return float(m_iFixedAdvance); }

	K2_API float	GetStringWidth(const tstring &sStr);

	void							AddCharacter(uint uiChar, const CCharacterRange &charRange);
	inline const SCharacterMapInfo*	GetCharMapInfo(char c)		{ return ActuallyGetCharMapInfo(static_cast<byte>(c)); }
	inline const SCharacterMapInfo*	GetCharMapInfo(wchar_t c)	{ return ActuallyGetCharMapInfo(static_cast<ushort>(c)); }
	K2_API float					GetKerning(uint uiLeftChar, uint uiRightChar);
	ResHandle						GetMapHandle() const	{ return m_hTextureMap; }
};
//=============================================================================

#endif //__C_FONTMAP_H__
