// (C)2005 S2 Games
// c_fontmap.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_fontmap.h"
#include "c_fontface.h"
#include "i_resourcelibrary.h"
#include "c_bitmap.h"
#include "c_texture.h"
#include "c_vid.h"
#include "c_resourcemanager.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SYNTHESIS_H
#include <freetype/ftglyph.h>
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CVAR_BOOL(	font_writeBitmaps,	false);
CVAR_BOOL(	font_kerning,		false);

IResourceLibrary	g_ResLibFontMap(RES_FONTMAP, _T("Fonts"), CFontMap::ResTypeName(), true, NULL);

CCharacterRange g_aCharacterRanges[] =
{
	CCharacterRange(FONT_RANGE_LATIN,			0x0000, 0x024f),
	CCharacterRange(FONT_RANGE_GREEK,			0x0370, 0x03ff),
	CCharacterRange(FONT_RANGE_CYRILLIC,		0x0400, 0x052f),
	CCharacterRange(FONT_RANGE_ARMENIAN,		0x0530, 0x058f),
	CCharacterRange(FONT_RANGE_HEBREW,			0x0590, 0x05ff),
	CCharacterRange(FONT_RANGE_ARABIC,			0x0600, 0x06ff),
	CCharacterRange(FONT_RANGE_DEVANGARI,		0x0900, 0x097F),
	CCharacterRange(FONT_RANGE_BENGALI,			0x0980, 0x09FF),
	CCharacterRange(FONT_RANGE_GURMUKHI,		0x0A00, 0x0A7F),
	CCharacterRange(FONT_RANGE_GUJARATI,		0x0A80, 0x0AFF),
	CCharacterRange(FONT_RANGE_THAI,			0x0E00, 0x0E7F),
	CCharacterRange(FONT_RANGE_HANGUL_JAMO,		0x1100, 0x11ff),
	CCharacterRange(FONT_RANGE_TAGALOG,			0x1700, 0x171F),
	CCharacterRange(FONT_RANGE_MONGOLIAN,		0x1800, 0x18AF),
	CCharacterRange(FONT_RANGE_LATIN_ADDITION,	0x1E00, 0x1EFF),
	CCharacterRange(FONT_RANGE_HIRAGANA,		0x3040, 0x309f),
	CCharacterRange(FONT_RANGE_KATAKANA,		0x30a0, 0x30ff),
	CCharacterRange(FONT_RANGE_CJK,				0x4e00, 0x9fff),
	CCharacterRange(FONT_RANGE_HANGUL_SYLLABLE,	0xac00, 0xd7a3)
};
//=============================================================================

/*====================
  CFontMap::CFontMap
  ====================*/
CFontMap::CFontMap(const tstring &sName, int iSize, int iStyle, uint uiCharacters, ResHandle hFontFace, bool bDynamicResize, int iBaseResolution, const tstring &sAxis) :
IResource(TSNULL, sName),
m_hFontFace(hFontFace),
m_hTextureMap(INVALID_RESOURCE),
m_iFaceSize(iSize),
m_iStyle(iStyle),
m_uiCharacterRanges(uiCharacters),
m_iNumCharacters(0),
m_iNumValidGlyphs(0),

m_iPenX(0),
m_iPenY(0),

m_pBitmap(NULL),

m_bDynamicResize(bDynamicResize),
m_iBaseResolution(iBaseResolution),
m_sAxis(sAxis)
{
}


/*====================
  CFontMap::AllocateBitmap
  ====================*/
bool	CFontMap::AllocateBitmap(int iTotalWidth)
{
	try
	{
		int iWidth(M_CeilPow2(iTotalWidth));
		int iHeight(M_CeilPow2(m_iMaxBitmapHeight));
		int iBestWidth(iWidth);
		int iBestHeight(iHeight);
		int iSmallestSize(iWidth * iHeight);
		int iSmallestAspect(iWidth + iHeight);
		while (iWidth > m_iMaxBitmapWidth)
		{
			int iRows(INT_CEIL(iTotalWidth / float(iWidth - m_iMaxBitmapWidth)));

			iHeight = M_CeilPow2(iRows * m_iMaxBitmapHeight);
			int iAspect(iWidth + iHeight);
			int iSize(iWidth * iHeight);
			if (iSize <= iSmallestSize && iAspect < iSmallestAspect && iWidth <= 4096 && iHeight <= 4096)
			{
				iSmallestAspect = iAspect;
				iSmallestSize = iSize;
				iBestWidth = iWidth;
				iBestHeight = iHeight;
			}
			iWidth >>= 1;
		}

		int iTexWidth(iBestWidth);
		int iTexHeight(iBestHeight);

		if (iTexWidth * iTexHeight > SQR(4096))
			EX_ERROR(_T("Bitmap for this font is too large: ") + XtoA(iTexWidth) + _T("x") + XtoA(iTexHeight));

		m_pBitmap = K2_NEW(ctx_Resources,  CBitmap)(iTexWidth, iTexHeight, BITMAP_RGBA);
		if (m_pBitmap == NULL)
			EX_ERROR(_T("Failed to allocate texture"));

		// Initial texture state
		for (int i(0); i < m_pBitmap->GetSize(); i += BITMAP_RGBA)
		{
			m_pBitmap->GetBuffer()[i] = 255;
			m_pBitmap->GetBuffer()[i + 1] = 255;
			m_pBitmap->GetBuffer()[i + 2] = 255;
			m_pBitmap->GetBuffer()[i + 3] = 0;
		}
		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CFontMap::AllocTexture() - "));
		return false;
	}
}


/*====================
  CFontMap::AdvancePen
  ====================*/
void	CFontMap::AdvancePen(int iWidth)
{
	// Advance the position of the "pen" on the map
	m_iPenX += iWidth;
	if (m_iPenX + m_iMaxBitmapWidth > m_pBitmap->GetWidth())
	{
		m_iPenX = 0;
		m_iPenY += m_iMaxBitmapHeight;
	}
}


/*====================
  CFontMap::AddCharacter
  ====================*/
void	CFontMap::AddCharacter(uint uiChar, const CCharacterRange &charRange)
{
	try
	{
		// Find the character glyph in the face
		uint uiGlyphIndex(FT_Get_Char_Index(m_pFace, uiChar));
		if (uiGlyphIndex == 0)
			return;

		// Load the glyph
		if (FT_Load_Glyph(m_pFace, uiGlyphIndex, FT_LOAD_DEFAULT) != 0)
			EX_ERROR(_T("FT_Load_Glyph() failed"));

		// Apply styles
		//*
		if (m_iStyle & FONT_STYLE_ITALIC)
			FT_GlyphSlot_Oblique(m_pFace->glyph);
		if (m_iStyle & FONT_STYLE_BOLD)
			FT_GlyphSlot_Embolden(m_pFace->glyph);

		// Render glyph if we aren't a bitmap
		if (m_pFace->glyph->format != FT_GLYPH_FORMAT_BITMAP)
		{
			// Render an anti-aliased bitmap into font->glyph
			if (FT_Render_Glyph(m_pFace->glyph, FT_RENDER_MODE_NORMAL) != 0)
				EX_ERROR(_T("Failed to render glyph"));
		}
		/**/

		if (m_pFace->glyph->bitmap.rows > m_iMaxBitmapHeight)
			EX_ERROR(_T("Glyph taller than MaxBitmapHeight"));

		// Copy the newly rendered glyph to the main texture map
		for (int iRow(0); iRow < m_pFace->glyph->bitmap.rows; ++iRow)
		{
			for (int iCol(0); iCol < m_pFace->glyph->bitmap.width; ++iCol)
			{
				int iTexMapX(m_iPenX + iCol);
				int iTexMapY(m_iPenY + iRow);
				int iTexMapIndex(((iTexMapX + iTexMapY * m_pBitmap->GetWidth()) * 4) + 3);

				int uiGlyphIndex(iCol + iRow * m_pFace->glyph->bitmap.width);

				if (iTexMapIndex < 0 || iTexMapIndex >= m_pBitmap->GetSize())
					continue;

				m_pBitmap->GetBuffer()[iTexMapIndex] = m_pFace->glyph->bitmap.buffer[uiGlyphIndex];
			}
		}

		// Store the character info
		SCharacterMapInfo info;
		info.m_fWidth = float(m_pFace->glyph->bitmap.width);
		info.m_fHeight = float(m_pFace->glyph->bitmap.rows);
		info.m_fAdvance = float(PointsToPixels(m_pFace->glyph->metrics.horiAdvance));
		info.m_fBearingX = float(PointsToPixels(m_pFace->glyph->metrics.horiBearingX));
		info.m_fBearingY = float(PointsToPixels(m_pFace->glyph->metrics.horiBearingY));
		info.m_fS1 = float(m_iPenX) / m_pBitmap->GetWidth();
		info.m_fT1 = (m_iPenY + info.m_fHeight) / m_pBitmap->GetHeight();
		info.m_fS2 = (m_iPenX + info.m_fWidth) / m_pBitmap->GetWidth();
		info.m_fT2 = float(m_iPenY) / m_pBitmap->GetHeight();

		m_vCharInfo[charRange.GetRange()][uiChar - charRange.GetFirstChar()] = info;

		AdvancePen(m_pFace->glyph->bitmap.width);

		if (m_iPenY >= m_pBitmap->GetHeight())
			EX_ERROR(_T("Out of room on texture map!"));
	}
	catch (CException &ex)
	{
		ex.Process(_TS("CFontMap::AddCharacter(") + XtoA(uiChar) + _TS(") - "), NO_THROW);
	}
}


/*====================
  CFontMap::AddCharacterRange
  ====================*/
int		CFontMap::AddCharacterRange(uint uiFirstChar, uint uiLastChar, int iCurrentTotalWidth)
{
	for (uint uiIndex(uiFirstChar); uiIndex <= uiLastChar; ++uiIndex)
	{
		// Find the character glyph in the face
		uint uiGlyphIndex(FT_Get_Char_Index(m_pFace, uiIndex));
		if (uiGlyphIndex == 0)
			continue;

		// Load the glyph
		if (FT_Load_Glyph(m_pFace, uiGlyphIndex, FT_LOAD_DEFAULT) != 0)
			continue;

		// Apply styles
		if (m_iStyle & FONT_STYLE_ITALIC)
			FT_GlyphSlot_Oblique(m_pFace->glyph);
		if (m_iStyle & FONT_STYLE_BOLD)
			FT_GlyphSlot_Embolden(m_pFace->glyph);

		// Render glyph if we aren't a bitmap
		if (m_pFace->glyph->format != FT_GLYPH_FORMAT_BITMAP)
		{
			// Render an anti-aliased bitmap into font->glyph
			if (FT_Render_Glyph(m_pFace->glyph, FT_RENDER_MODE_NORMAL) != 0)
			{
				Console.Err << _T("Failed to render glyph") << newl;
				continue;
			}
		}

		//int iWidth(PointsToPixels(m_pFace->glyph->metrics.horiAdvance));
		//int iHeight(PointsToPixels(m_pFace->glyph->metrics.vertAdvance));
		int iWidth(m_pFace->glyph->bitmap.width);
		int iHeight(m_pFace->glyph->bitmap.rows);
		m_iMaxBitmapWidth = MAX(m_iMaxBitmapWidth, iWidth);
		m_iMaxBitmapHeight = MAX(m_iMaxBitmapHeight, iHeight);
		iCurrentTotalWidth += iWidth;

		++m_iNumValidGlyphs;
	}

	return iCurrentTotalWidth;
}


/*====================
  CFontMap::Load
  ====================*/
int		CFontMap::Load(uint uiIgnoreFlags, const char *pData, uint uiSize)
{
	PROFILE("CFontFace::Load");

	try
	{
		if (K2System.IsDedicatedServer() || K2System.IsServerManager())
			return 0; // Skip

		if (!m_sPath.empty())
			Console.Res << "Loading FontMap " << SingleQuoteStr(m_sPath) << newl;
		else if (!m_sName.empty())
			Console.Res << "Loading FontMap " << SingleQuoteStr(m_sName) << newl;
		else
			Console.Res << "Loading Unknown FontMap" << newl;

		int iSize(m_iFaceSize);

		// If in dynamic resize mode, get the current resolution and adjust the size
		if (m_bDynamicResize)
		{
			float fRelativeAdjustment;

			if (m_sAxis == _T("y"))
			{
				fRelativeAdjustment = (float(Vid.GetScreenH()) / m_iBaseResolution);
			}
			else
			{
				if (m_sAxis != _T("x"))
					Console.Warn << _T("Defaulted fontmap axis to X - unrecognized specifier.") << newl;

				fRelativeAdjustment = (float(Vid.GetScreenW()) / m_iBaseResolution);
			}
			
			if (fRelativeAdjustment < 1.0f)
				iSize = INT_CEIL(fRelativeAdjustment * iSize);
			else
				iSize = INT_FLOOR(fRelativeAdjustment * iSize);
		}

		// Check requested face size
		if (iSize < 1)
			EX_ERROR(_T("Can't create a font with a size < 1"));

		// Get the fontface resource
		CFontFace *pFontFace(g_ResourceManager.GetFontFace(m_hFontFace));
		if (pFontFace == NULL)
			EX_ERROR(_T("Couldn't retrieve the fontface resource"));
		m_pFace = pFontFace->GetFace();

		if (m_pFace == NULL)
			EX_ERROR(_T("Couldn't retrieve the face from the fontface resource"));

		// Set the size parameter
		if (FT_Set_Pixel_Sizes(m_pFace, 0, iSize) != 0)
			EX_ERROR(_T("FT_Set_Pixel_Sizes() failed"));

		// Calculate max bitmap sizes
		m_iMaxBitmapWidth = 0;
		m_iMaxBitmapHeight = 0;

		int iTotalWidth(0);

		for (uint ui(0); ui < sizeof(g_aCharacterRanges) / sizeof(CCharacterRange); ++ui)
		{
			if (m_uiCharacterRanges & BIT(g_aCharacterRanges[ui].GetRange()))
			{
				iTotalWidth += AddCharacterRange(g_aCharacterRanges[ui].GetFirstChar(), g_aCharacterRanges[ui].GetLastChar(), iTotalWidth);
				m_iNumCharacters += g_aCharacterRanges[ui].GetNumChars();
				m_vCharInfo[g_aCharacterRanges[ui].GetRange()].resize(g_aCharacterRanges[ui].GetNumChars());
			}
		}

		m_iMaxAdvance = PointsToPixels(m_pFace->max_advance_width);
		m_iMaxHeight = PointsToPixels(m_pFace->size->metrics.ascender - m_pFace->size->metrics.descender);
		m_iMaxAscender = PointsToPixels(m_pFace->size->metrics.ascender);
		m_iFixedAdvance = PointsToPixels(m_pFace->size->metrics.max_advance);
		m_iMaxBitmapWidth += 1;
		m_iMaxBitmapHeight += 1;

		// Missing character
		iTotalWidth += ceil(iSize / 2.0f);

		// Create a bitmap
		if (!AllocateBitmap(iTotalWidth))
			EX_ERROR(_T("Failed to allocate texture"));

		// Fill in a dummy entry at the start that will be used for missing characters
		m_MissingCharInfo.m_fWidth = ceil(iSize / 2.0f);
		m_MissingCharInfo.m_fHeight = iSize;
		m_MissingCharInfo.m_fAdvance = m_MissingCharInfo.m_fWidth + 2.0f;
		m_MissingCharInfo.m_fBearingX = 1.0f;
		m_MissingCharInfo.m_fBearingY = m_MissingCharInfo.m_fHeight - 1.0f;
		m_MissingCharInfo.m_fS1 = 0.0f;
		m_MissingCharInfo.m_fT1 = m_MissingCharInfo.m_fHeight / m_pBitmap->GetHeight();
		m_MissingCharInfo.m_fS2 = m_MissingCharInfo.m_fWidth / m_pBitmap->GetWidth();
		m_MissingCharInfo.m_fT2 = 0.0f;
		byte* pBitmapBuffer(m_pBitmap->GetBuffer());
		for (int y(0); y < m_iMaxHeight; ++y)
		{
			for (int x(0); x < m_iMaxAdvance; ++x)
				pBitmapBuffer[(y * m_pBitmap->GetWidth()) * 4 + (x * 4) + 3] = 255;
		}
		AdvancePen(m_iMaxAdvance);
		
		// Fill in the character info arrays
		for (uint ui(0); ui < sizeof(g_aCharacterRanges) / sizeof(CCharacterRange); ++ui)
		{
			if (m_uiCharacterRanges & BIT(g_aCharacterRanges[ui].GetRange()))
			{
				for (uint uiChar(g_aCharacterRanges[ui].GetFirstChar()); uiChar <= g_aCharacterRanges[ui].GetLastChar(); ++uiChar)
					AddCharacter(uiChar, g_aCharacterRanges[ui]);
			}
		}

		if (font_writeBitmaps)
			m_pBitmap->WritePNG(_T("~/font_") + m_sName + _T(".png"));

		// Register this texture with the resource manager
		int iTextureFlags(TEX_FULL_QUALITY | TEX_NO_MIPMAPS | TEX_NO_COMPRESS);
		CTexture *pNewTexture(K2_NEW(ctx_Resources,  CTexture)(_T("*") + tstring(pFontFace->GetPath()) + _T(" ") + XtoA(iSize) + _T(" ") + XtoA(m_iStyle, 0, 0, 16), m_pBitmap, TEXTURE_2D, iTextureFlags, TEXFMT_A8R8G8B8));
		m_hTextureMap = g_ResourceManager.Register(pNewTexture, RES_TEXTURE);
		if (m_hTextureMap == INVALID_RESOURCE)
			EX_ERROR(_T("Failed registering texture"));

		Console.Res << _T("Created FontMap: ") << m_sName << _T(" from ") << pFontFace->GetPath()
				<< _T(" at ") << iSize << _T(" pixels.") << newl;
	}
	catch (CException &ex)
	{
		SAFE_DELETE(m_pBitmap);

		ex.Process(_TS("CFontMap::Load(") + m_sName + _TS(") - "), NO_THROW);

		return RES_LOAD_FAILED;
	}

	SAFE_DELETE(m_pBitmap);

	return 0;
}


/*====================
  CFontMap::Free
  ====================*/
void	CFontMap::Free()
{
	m_iMaxAdvance = 0;
	m_iMaxHeight = 0;
	m_iFixedAdvance = 0;

	m_iPenX = 0;
	m_iPenY = 0;

	SAFE_DELETE(m_pBitmap);
}


/*====================
  CFontMap::ActuallyGetCharMapInfo
  ====================*/
const SCharacterMapInfo*	CFontMap::ActuallyGetCharMapInfo(uint uiIndex)
{
	for (uint ui(0); ui < sizeof(g_aCharacterRanges) / sizeof(CCharacterRange); ++ui)
	{
		if (!(m_uiCharacterRanges & BIT(g_aCharacterRanges[ui].GetRange())))
			continue;

		if (uiIndex >= g_aCharacterRanges[ui].GetFirstChar() && uiIndex <= g_aCharacterRanges[ui].GetLastChar())
			return &m_vCharInfo[g_aCharacterRanges[ui].GetRange()][uiIndex - g_aCharacterRanges[ui].GetFirstChar()];
	}

	/*
	if (uiIndex < g_LatinCharacterRange.GetFirstChar())
		return &m_MissingCharInfo;
	if (uiIndex <= g_LatinCharacterRange.GetLastChar() && (m_uiCharacterRanges & BIT(g_LatinCharacterRange.GetRange())))
		return &m_vCharInfo[g_LatinCharacterRange.GetRange()][uiIndex - g_LatinCharacterRange.GetFirstChar()];
	if (uiIndex < FONT_FIRST_SIMPLIFIED_CHINESE_CHAR)
		return &m_MissingCharInfo;
	if (uiIndex <= FONT_LAST_SIMPLIFIED_CHINESE_CHAR && (m_uiCharacterRanges & BIT(FONT_RANGE_SIMPLIFIED_CHINESE)))
		return &m_vCharInfo[g_SimplifiedChineseCharacterRange.GetRange()][uiIndex - g_SimplifiedChineseCharacterRange.GetFirstChar()];
	if (uiIndex < FONT_FIRST_HANGUL_CHAR)
		return &m_MissingCharInfo;
	if (uiIndex <= FONT_LAST_HANGUL_CHAR && (m_uiCharacterRanges & BIT(FONT_RANGE_HANGUL)))
		return &m_vCharInfo[g_HangulCharacterRange.GetRange()][uiIndex - g_HangulCharacterRange.GetFirstChar()];
	/**/
	
	return &m_MissingCharInfo;
}


/*====================
  CFontMap::GetKerning
  ====================*/
float	CFontMap::GetKerning(uint uiLeftChar, uint uiRightChar)
{
	if (!font_kerning)
		return 0.0f;

	FT_Vector ftv;
	if (FT_Get_Kerning(m_pFace, FT_Get_Char_Index(m_pFace, uiLeftChar), FT_Get_Char_Index(m_pFace, uiRightChar), FT_KERNING_DEFAULT, &ftv))
		return 0.0f;

	return PointsToPixels(ftv.x);
}


/*====================
  CFontMap::GetStringWidth
  ====================*/
float	CFontMap::GetStringWidth(const tstring &sStr)
{
	float fLength(0.0f);

	tstring sClean(StripColorCodes(sStr));
	for (size_t z(0); z < sClean.length(); ++z)
	{
		const SCharacterMapInfo *pInfo(GetCharMapInfo(sClean[z]));
		if (pInfo == NULL)
			fLength += m_iMaxAdvance;
		else
			fLength += pInfo->m_fAdvance;

		if (z > 0 && font_kerning)
			fLength += GetKerning(sClean[z - 1], sClean[z]);
	}

	return fLength;
}
