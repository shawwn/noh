// (C)2005 S2 Games
// c_draw2d.cpp
//
// 2D Drawing functions
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_draw2d.h"
#include "c_vid.h"

#include "c_fontmap.h"

#include "c_uitextureregistry.h"
#include "c_filemanager.h"
#include "c_resourcemanager.h"
#include "c_texture.h"

#include "c_cmd.h"

#undef pDraw
#undef Draw
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CDraw2D *pDraw2D = CDraw2D::GetInstance();
CDraw2D &Draw2D = *pDraw2D;

CVAR_FLOAT(draw_scale, 1.0f);

SINGLETON_INIT(CDraw2D)
//=============================================================================

/*====================
  CDraw2D::CDraw2D
  ====================*/
CDraw2D::CDraw2D() :
m_v4CurrentBGColor(WHITE),
m_v4CurrentFGColor(WHITE),
m_hGlow(INVALID_RESOURCE)
{
}


/*====================
  CDraw2D::~CDraw2D
  ====================*/
CDraw2D::~CDraw2D()
{
}


/*====================
  CDraw2D::Quad
  ====================*/
void    CDraw2D::Quad(const CVec2f& v1, const CVec2f& v2, const CVec2f& v3, const CVec2f& v4,
                      const CVec2f& t1, const CVec2f& t2, const CVec2f& t3, const CVec2f& t4, ResHandle hTexture, int iFlags)
{
    if (hTexture == INVALID_RESOURCE)
        Console.Warn << _T("Draw2D::Rect() - Invalid texture handle") << newl;

    Vid.Add2dQuad(v1, v2, v3, v4, t1, t2, t3, t4, hTexture, iFlags);
}


/*====================
  CDraw2D::Quad
  ====================*/
void    CDraw2D::Quad(const CVec2f& v1, const CVec2f& v2, const CVec2f& v3, const CVec2f& v4, int iFlags)
{
    Quad(v1, v2, v3, v4, CVec2f(0.0f, 0.0f), CVec2f(0.0f, 1.0f), CVec2f(1.0f, 0.0f), CVec2f(1.0f, 1.0f), g_ResourceManager.GetWhiteTexture(), iFlags);
}


/*====================
  CDraw2D::Rect
  ====================*/
void    CDraw2D::Rect(float x, float y, float w, float h, float s1, float t1, float s2, float t2, ResHandle hTexture, int iFlags)
{
    if (hTexture == INVALID_RESOURCE)
        Console.Warn << _T("Draw2D::Rect() - Invalid texture handle") << newl;

    Vid.Add2dRect(x, y, w, h, s1, t1, s2, t2, hTexture, iFlags);
}


/*====================
  CDraw2D::Rect
  ====================*/
void    CDraw2D::Rect(float x, float y, float w, float h, int iFlags /*= 0*/)
{
    Rect(x, y, w, h, 0.0f, 0.0f, 1.0f, 1.0f, g_ResourceManager.GetWhiteTexture(), iFlags);
}


/*====================
  CDraw2D::RectOutline
  ====================*/
void    CDraw2D::RectOutline(const CRectf &rect, float fThickness)
{
    if (fThickness < 1.0f)
        return;

    CRectf recCopy(rect);
    recCopy.Normalize();
    Rect(recCopy.left, recCopy.top, recCopy.GetWidth(), fThickness);
    Rect(recCopy.left, recCopy.bottom - fThickness, recCopy.GetWidth(), fThickness);
    Rect(recCopy.left, recCopy.top, fThickness, recCopy.GetHeight());
    Rect(recCopy.right - fThickness, recCopy.top, fThickness, recCopy.GetHeight());
}

void    CDraw2D::RectOutline(const CRectf &rect, float fThickness, CVec4f v4Color)
{
    CVec4f v4OldColor(m_v4CurrentColor);
    SetColor(v4Color);
    RectOutline(rect, fThickness);
    SetColor(v4OldColor);
}


/*====================
  CDraw2D:FilledRect
  ====================*/
void    CDraw2D::FilledRect(const CRectf &rect, float fThickness, CVec4f v4BorderColor, CVec4f v4FillColor)
{
    CVec4f v4OldColor(m_v4CurrentColor);
    SetColor(v4FillColor);
    Rect(rect.left, rect.top, rect.GetWidth(), rect.GetHeight());
    SetColor(v4OldColor);
    RectOutline(rect, fThickness, v4BorderColor);
}


/*====================
  CDraw2D:Line
  ====================*/
void    CDraw2D::Line(const CVec2f& v1, const CVec2f& v2, const CVec4f& v4Color1, const CVec4f& v4Color2, int iFlags)
{
    Vid.Add2dLine(v1, v2, v4Color1, v4Color2, iFlags);
}


/*====================
  CDraw2D::Clear
  ====================*/
void    CDraw2D::Clear()
{
    Vid.Clear();
}


/*====================
  CDraw2D::String
  ====================*/
void    CDraw2D::String(float x, float y, const tstring &sStr, ResHandle hFont)
{
    try
    {
        CVec4f v4OldColor(GetCurrentColor());

        // Get the fontmap to be used for rendering this string
        CFontMap *pFontMap(g_ResourceManager.GetFontMap(hFont));
        if (pFontMap == NULL)
            throw CException(_T("Failed to acquire font"), E_WARNING);

        // Get the color keys and a clean string
        StrColorMap vColors;
        tstring sClean(StripColorCodes(sStr, vColors));
        StrColorMap::iterator itColors(vColors.begin());

        // Draw each character
        float xPos(x);
        for (size_t z(0); z < sClean.length(); ++z)
        {
            // Check for color changes
            if (itColors != vColors.end())
            {
                if (itColors->first == z)
                {
                    // Do nothing if the color code is glow only
                    if (itColors->second.w != 3.14f)
                    {
                        if (itColors->second.x < 0.0f)
                        {
                            SetColor(v4OldColor);
                        }
                        else if (itColors->second.w > 3.14f)
                        {
                            CVec4f v4NewColor(itColors->second[0], itColors->second[1], itColors->second[2], v4OldColor[3]);
                            SetColor(v4NewColor);
                        }
                        else
                        {
                            //Set up using the colors from the iterator, but
                            //alpha value from the original color, so we can
                            //fade out text that has color codes in it.
                            CVec4f v4NewColor(itColors->second[0], itColors->second[1], itColors->second[2], v4OldColor[3]);
                            SetColor(v4NewColor);
                        }
                    }
                    ++itColors;
                }
            }

            float fKerning(z > 0 ? pFontMap->GetKerning(sClean[z - 1], sClean[z]) : 0.0f);
            const SCharacterMapInfo *pCharInfo(pFontMap->GetCharMapInfo(sClean[z]));
            if (pCharInfo == NULL)
            {
                Rect(xPos, y, pFontMap->GetMaxAdvance(), pFontMap->GetMaxHeight(), GUI_STRING);
                xPos += pFontMap->GetMaxAdvance();
                continue;
            }

            Rect(xPos + pCharInfo->m_fBearingX + fKerning, y + (pFontMap->GetMaxAscender() - pCharInfo->m_fBearingY), pCharInfo->m_fWidth, pCharInfo->m_fHeight, pCharInfo->m_fS1, pCharInfo->m_fT1,
                pCharInfo->m_fS2, pCharInfo->m_fT2, pFontMap->GetMapHandle(), GUI_STRING);

            xPos += pCharInfo->m_fAdvance + fKerning;
        }

        SetColor(v4OldColor);

        if (sStr.find(_T("^?")) != sStr.npos)
            StringGlow(x, y, sStr, hFont, vColors);

    }
    catch (CException &ex)
    {
        ex.Process(_TS("CDraw2D::String(") + sStr + _TS(") - "), NO_THROW);
    }
}

void    CDraw2D::String(float x, float y, float w, float h, const tstring &sStr, ResHandle hFont, int iFlags, size_t zSelectionStart, size_t zSelectionEnd, float fStartingXOffset)
{
    try
    {
        CVec4f v4OldColor(GetCurrentColor());
        StrColorMap::iterator itColors;

        // Retrieve the font map
        CFontMap *pFontMap(g_ResourceManager.GetFontMap(hFont));
        if (pFontMap == NULL)
            throw CException(_T("Failed to acquire font"), E_WARNING);

        // Get the color keys and a clean string
        StrColorMap vColors;
        tstring sClean(StripColorCodes(sStr, vColors));
        itColors = vColors.begin();

        float fXPos(x);
        float fYPos(y);

        int iSkipChars(0);

        // Centering
        // FIXME: Handle centering properly for wrapped text
        // FIXME: Centered, unwrapped text can leave the bounds of the drawing area.
        if (iFlags & DRAW_STRING_CENTER)
            fXPos += ceil((w - pFontMap->GetStringWidth(sStr)) / 2.0f);
        else if (iFlags & DRAW_STRING_RIGHT)
            fXPos += ceil(w - pFontMap->GetStringWidth(sStr));

        if (iFlags & DRAW_STRING_VCENTER)
            fYPos += ceil((h - pFontMap->GetMaxHeight()) / 2.0f);
        else if (iFlags & DRAW_STRING_BOTTOM)
            fYPos += ceil(h - pFontMap->GetMaxHeight());

        fXPos += fStartingXOffset;

        // Draw each character
        for (size_t z(0); z < sClean.length(); ++z)
        {
            // Check for color changes
            if (itColors != vColors.end())
            {
                if (itColors->first == z)
                {
                    // Do nothing if the color code is glow only
                    if (itColors->second.w != 3.14f)
                    {
                        if (itColors->second.x < 0.0f || iFlags & DRAW_STRING_NOCOLORCODES)
                        {
                            SetColor(v4OldColor);
                        }
                        else if (itColors->second.w > 3.14f)
                        {
                            CVec4f v4NewColor(itColors->second[0], itColors->second[1], itColors->second[2], v4OldColor[3]);
                            SetColor(v4NewColor);
                        }
                        else
                        {
                            //Set up using the colors from the iterator, but
                            //alpha value from the original color, so we can
                            //fade out text that has color codes in it.
                            CVec4f v4NewColor(itColors->second[0], itColors->second[1], itColors->second[2], v4OldColor[3]);
                            SetColor(v4NewColor);
                        }
                    }
                    ++itColors;
                }
            }

            if (iSkipChars > 0)
            {
                --iSkipChars;
                continue;
            }

            // Check for smileys
            if (iFlags & DRAW_STRING_SMILEYS)
            {
                ResHandle hSmileyTexture(INVALID_RESOURCE);
                float fSize;
                int iLength(1);

                for (map<tstring, ResHandle>::iterator it(m_mapSmileys.begin()), itEnd(m_mapSmileys.end()); it != itEnd; ++it)
                {
                    if (sClean.length() < z + it->first.length())
                        continue;

                    if (sClean.compare(z, it->first.length(), it->first) == 0)
                    {
                        hSmileyTexture = it->second;
                        iLength = int(it->first.length());
                        break;
                    }
                }

                if (hSmileyTexture != INVALID_RESOURCE)
                {
                    fSize = pFontMap->GetMaxHeight();
                    Rect(fXPos, y, fSize, fSize, hSmileyTexture, GUI_STRING);

                    // Skip the next character, as it was part of the
                    // smiley, and we don't want to draw it.
                    fXPos += fSize;
                    iSkipChars = iLength - 1;
                    continue;
                }
            }

            float fKerning(z > 0 ? pFontMap->GetKerning(sClean[z - 1], sClean[z]) : 0.0f);
            const SCharacterMapInfo *pCharInfo(pFontMap->GetCharMapInfo(sClean[z]));
            if (pCharInfo == NULL)
            {
                Rect(fXPos, y, pFontMap->GetMaxAdvance(), pFontMap->GetMaxHeight(), GUI_STRING);
                fXPos += pFontMap->GetMaxAdvance();
                continue;
            }

            // Check for a width overflow
            float fWidth(pCharInfo->m_fWidth);
            float fS2(pCharInfo->m_fS2);
            if (fXPos + fKerning + pCharInfo->m_fAdvance > x + w)
            {
                if (iFlags & DRAW_STRING_WRAP)
                {
                    // Move to the next line
                    // Use MaxHeight instead of the char height to
                    // prevent text overlapping, but only use 2/3
                    // of it to also prevent a massive gap. 
                    //MikeG seems like a waste to do float mul on every char that goes over the size 
                    // on the screen and it makes the spaces to small. =)
                    fYPos += pFontMap->GetMaxHeight();//((pFontMap->GetMaxHeight() / 3) * 2);
                    fXPos = x;
                }
                else
                {
                    if (pCharInfo->m_fWidth > 0.0f)
                    {
                        // Clamp the width
                        fWidth += fXPos + fKerning - x - w;
                        fS2 = pCharInfo->m_fS1 + (pCharInfo->m_fS2 - pCharInfo->m_fS1) * (fWidth / pCharInfo->m_fWidth);
                    }
                }
            }

            // Clamp the height
            float fHeight(pCharInfo->m_fHeight);
            float fT1(pCharInfo->m_fT1);
            if (fYPos + fHeight > y + h)
            {
                if (pCharInfo->m_fHeight > 0.0f)
                {
                    fHeight = (y + h) - fYPos;
                    fT1 = pCharInfo->m_fT2 + (pCharInfo->m_fT1 - pCharInfo->m_fT2) * (fHeight / pCharInfo->m_fHeight);
                }
            }

            if (fXPos < x)
            {
                fXPos += pCharInfo->m_fAdvance + fKerning;
                continue;
            }

            if (zSelectionEnd != size_t(-1) && zSelectionStart != size_t(-1) && z < zSelectionEnd && z >= zSelectionStart)
            {
                CVec4f v4Old(GetCurrentColor());

                SetColor(m_v4CurrentBGColor);
                Draw2D.Rect(fXPos, fYPos, pCharInfo->m_fAdvance, pFontMap->GetMaxHeight(), g_ResourceManager.GetWhiteTexture(), GUI_STRING);
                SetColor(m_v4CurrentFGColor);
                Rect(fXPos + fKerning + pCharInfo->m_fBearingX, fYPos + (pFontMap->GetMaxAscender() - pCharInfo->m_fBearingY), fWidth, fHeight, pCharInfo->m_fS1, fT1, fS2, pCharInfo->m_fT2, pFontMap->GetMapHandle(), GUI_STRING);
                SetColor(v4Old);
            }
            else
            {
                Rect(fXPos + fKerning + pCharInfo->m_fBearingX, fYPos + (pFontMap->GetMaxAscender() - pCharInfo->m_fBearingY), fWidth, fHeight, pCharInfo->m_fS1, fT1, fS2, pCharInfo->m_fT2, pFontMap->GetMapHandle(), GUI_STRING);
            }

            fXPos += pCharInfo->m_fAdvance + fKerning;

            if (fXPos > x + w)
                break;
        }

        SetColor(v4OldColor);

        if (sStr.find(_T("^?")) != sStr.npos)
            StringGlow(x, y, w, h, sStr, hFont, iFlags, fStartingXOffset, vColors);
    }
    catch (CException &ex)
    {
        ex.Process(_TS("CDraw2D::String(") + sStr + _TS(") - "), NO_THROW);
    }
}

void    CDraw2D::String(float x, float y, float w, float h, const tstring &sStr, ResHandle hFont, uivector vWrappingBreakList, fvector vLineCentering, int iFlags, bool bShadow, bool bOutline, float fXOffset, float fYOffset, CVec4f v4ShadowColor)
{
    try
    {
        CVec4f v4OldColor(GetCurrentColor());
        StrColorMap::iterator itColors;

        // Retrieve the font map
        CFontMap *pFontMap(g_ResourceManager.GetFontMap(hFont));
        if (pFontMap == NULL)
            throw CException(_T("Failed to acquire font"), E_WARNING);

        // Get the color keys and a clean string
        StrColorMap vColors;
        tstring sClean(StripColorCodes(sStr, vColors));
        itColors = vColors.begin();

        float fXPos(x);
        float fYPos(y);

        int iSkipChars(0);
        float fStrWidth(0.0f);

        fvector::iterator itLineCentering(vLineCentering.begin());

        if (!vLineCentering.empty())
            fStrWidth = (*itLineCentering);
        else
            fStrWidth = pFontMap->GetStringWidth(sClean);

        // Centering
        if (iFlags & DRAW_STRING_CENTER)
            fXPos += ceil((w - fStrWidth) / 2.0f);
        else if (iFlags & DRAW_STRING_RIGHT)
            fXPos += ceil(w - fStrWidth);

        //Center/bottom vertial align according to the size of the text and the height
        if (iFlags & DRAW_STRING_VCENTER)
            fYPos += ceil(((h - pFontMap->GetMaxHeight()) - (vWrappingBreakList.size() * pFontMap->GetMaxHeight())) / 2.0f);
        else if (iFlags & DRAW_STRING_BOTTOM)
            fYPos += ceil(((h - pFontMap->GetMaxHeight()) - (vWrappingBreakList.size() * pFontMap->GetMaxHeight())));


        uivector::iterator itWrapStart(vWrappingBreakList.begin());

        // Draw each character
        for (size_t z(0); z < sClean.length(); ++z)
        {
            // Check for color changes
            if (itColors != vColors.end())
            {
                if (itColors->first == z)
                {
                    // Do nothing if the color code is glow only
                    if (itColors->second.w != 3.14f)
                    {
                        if (itColors->second.x < 0.0f || iFlags & DRAW_STRING_NOCOLORCODES)
                        {
                            SetColor(v4OldColor);
                        }
                        else if (itColors->second.w > 3.14f)
                        {
                            CVec4f v4NewColor(itColors->second[0], itColors->second[1], itColors->second[2], v4OldColor[3]);
                            SetColor(v4NewColor);
                        }
                        else
                        {
                            //Set up using the colors from the iterator, but
                            //alpha value from the original color, so we can
                            //fade out text that has color codes in it.
                            CVec4f v4NewColor(itColors->second[0], itColors->second[1], itColors->second[2], v4OldColor[3]);
                            SetColor(v4NewColor);
                        }
                    }
                    ++itColors;
                }
            }

            if (iSkipChars > 0)
            {
                --iSkipChars;
                continue;
            }

            // Check for smileys
            if (iFlags & DRAW_STRING_SMILEYS)
            {
                ResHandle hSmileyTexture(INVALID_RESOURCE);
                float fSize;
                int iLength(1);

                for (map<tstring, ResHandle>::iterator it(m_mapSmileys.begin()), itEnd(m_mapSmileys.end()); it != itEnd; ++it)
                {
                    if (sClean.length() < z + it->first.length())
                        continue;

                    if (sClean.compare(z, it->first.length(), it->first) == 0)
                    {
                        hSmileyTexture = it->second;
                        iLength = int(it->first.length());
                        break;
                    }
                }

                if (hSmileyTexture != INVALID_RESOURCE)
                {
                    fSize = pFontMap->GetMaxHeight();
                    Rect(fXPos, y, fSize, fSize, hSmileyTexture, GUI_STRING);

                    // Skip the next character, as it was part of the
                    // smiley, and we don't want to draw it.
                    fXPos += fSize;
                    iSkipChars = iLength - 1;
                    continue;
                }
            }

            float fKerning(z > 0 ? pFontMap->GetKerning(sClean[z - 1], sClean[z]) : 0.0f);
            const SCharacterMapInfo *pCharInfo(pFontMap->GetCharMapInfo(sClean[z]));
            if (pCharInfo == NULL)
            {
                Rect(fXPos, y, pFontMap->GetMaxAdvance(), pFontMap->GetMaxHeight(), GUI_STRING);
                fXPos += pFontMap->GetMaxAdvance();
                continue;
            }

            // Check for a width overflow
            float fWidth(pCharInfo->m_fWidth);
            float fS2(pCharInfo->m_fS2);
            if (fXPos + fWidth > x + w)
            {
                fWidth = (x + w) - (fXPos + fKerning);
                fS2 = pCharInfo->m_fS1 + (pCharInfo->m_fS2 - pCharInfo->m_fS1) * (fWidth / pCharInfo->m_fWidth);
            }

            // Clamp the height
            float fHeight(pCharInfo->m_fHeight);
            float fT1(pCharInfo->m_fT1);
            if (fYPos + fHeight > y + h)
            {
                fHeight = (y + h) - fYPos;
                fT1 = pCharInfo->m_fT2 + (pCharInfo->m_fT1 - pCharInfo->m_fT2) * (fHeight / pCharInfo->m_fHeight);
            }

            if (fXPos < x)
            {
                fXPos = x;
            }

            if (fYPos < y)
            {
                fYPos = y;
            }
            
            if (bShadow && !IsTokenSeparator(sClean[z]))
            {
                CVec4f v4Old(GetCurrentColor());
                SetColor(v4ShadowColor);
                Rect(fXPos + fKerning + pCharInfo->m_fBearingX + fXOffset, fYPos + (pFontMap->GetMaxAscender() - pCharInfo->m_fBearingY) + fYOffset, fWidth, fHeight, pCharInfo->m_fS1, fT1, fS2, pCharInfo->m_fT2, pFontMap->GetMapHandle(), GUI_STRING);
                SetColor(v4Old);

            }
            else if (bOutline && !IsTokenSeparator(sClean[z]))
            {
                CVec4f v4Old(GetCurrentColor());
                SetColor(v4ShadowColor);
                Rect(fXPos + fKerning + pCharInfo->m_fBearingX - fXOffset, fYPos + (pFontMap->GetMaxAscender() - pCharInfo->m_fBearingY) - fYOffset, fWidth + (fXOffset * 2.0f), fHeight + (fYOffset * 2.0f), pCharInfo->m_fS1, fT1, fS2, pCharInfo->m_fT2, pFontMap->GetMapHandle(), GUI_STRING);
                SetColor(v4Old);
            }

            if (!IsTokenSeparator(sClean[z]))
                Rect(fXPos + fKerning + pCharInfo->m_fBearingX, fYPos + (pFontMap->GetMaxAscender() - pCharInfo->m_fBearingY), fWidth, fHeight, pCharInfo->m_fS1, fT1, fS2, pCharInfo->m_fT2, pFontMap->GetMapHandle(), GUI_STRING);
            
            fXPos += pCharInfo->m_fAdvance + fKerning;

            if (vWrappingBreakList.size() > 0)
            {
                for (uivector::iterator itWrap(itWrapStart); itWrap != vWrappingBreakList.end(); ++itWrap)
                {
                    if ((*itWrap) == z)
                    {
                        itWrapStart = itWrap + 1;
                        fYPos += pFontMap->GetMaxHeight();
                        fXPos = x;

                        if (vLineCentering.size() > 0)
                        {
                            if (itLineCentering != vLineCentering.end())
                            {
                                itLineCentering++;
                                fStrWidth =(*itLineCentering);
                            }
                            else
                            {
                                fStrWidth =(*itLineCentering);
                            }
                            if (iFlags & DRAW_STRING_CENTER)
                                fXPos += ceil((w - fStrWidth) / 2.0f);
                            else if (iFlags & DRAW_STRING_RIGHT)
                                fXPos += ceil(w - fStrWidth);
                        }
                        break;
                    }
                }
            }

            if (fXPos > x + w)
                break;
        }

        SetColor(v4OldColor);

        if (sStr.find(_T("^?")) != sStr.npos)
            StringGlow(x, y, w, h, sStr, hFont, vWrappingBreakList, vLineCentering, iFlags, vColors);
    }
    catch (CException &ex)
    {
        ex.Process(_TS("CDraw2D::String(") + sStr + _TS(") - "), NO_THROW);
    }
}

void        CDraw2D::String(float x, float y, float w, float h, const tsvector &vStr,
                            ResHandle hFont, int iFlags, const ColorVector *pColors)
{
    try
    {
        CVec4f v4OldColor(GetCurrentColor());

        // Retrieve the font map
        CFontMap *pFontMap(g_ResourceManager.GetFontMap(hFont));
        if (pFontMap == NULL)
            EX_WARN(_T("Failed to acquire font"));

        tsvector_cit itBegin(vStr.begin());
        tsvector_cit itEnd(vStr.end());
        size_t zCount(vStr.size());

        ColorVector::const_iterator itColor;
        if (pColors != NULL)
            itColor = pColors->begin();

        // Break apart strings that need to wrap
        tsvector vsSplitStrings;
        ColorVector vSplitColors;
        if (iFlags & DRAW_STRING_WRAP)
        {
            for (tsvector_cit it(vStr.begin()); it != vStr.end(); ++it)
            {
                if (pColors != NULL)
                {
                    WrapString(*it, pFontMap, w, vsSplitStrings, &(*itColor), &vSplitColors);
                    ++itColor;
                }
                else
                {
                    WrapString(*it, pFontMap, w, vsSplitStrings);
                }

            }

            itBegin = vsSplitStrings.begin();
            itEnd = vsSplitStrings.end();
            zCount = vsSplitStrings.size();
            itColor = vSplitColors.begin();
        }

        // Determine starting point
        float fYOffset(0);
        if (iFlags & DRAW_STRING_ANCHOR_BOTTOM)
            fYOffset = h - (zCount * pFontMap->GetMaxHeight());
        
        if (iFlags & DRAW_STRING_VCENTER)
            fYOffset = ceil((h - (zCount * pFontMap->GetMaxHeight())) / 2.0f);

        // Draw each line
        for (tsvector_cit it(itBegin); it != itEnd; ++it)
        {
            if (fYOffset < 0.0f)
            {
                fYOffset += pFontMap->GetMaxHeight();
                if (pColors != NULL)
                    ++itColor;
                continue;
            }

            if (pColors != NULL)
            {
                SetColor(*itColor);
                ++itColor;
            }

            String(x, y + fYOffset, w, pFontMap->GetMaxHeight(), *it, hFont, (iFlags & ~DRAW_STRING_VCENTER));
            fYOffset += pFontMap->GetMaxHeight();
            if (fYOffset >= h)
                break;
        }

        if (pColors != NULL)
            SetColor(v4OldColor);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CDraw2D::String() - "), NO_THROW);
    }
}


void        CDraw2D::String(float x, float y, float w, float h, const tsvector &vStr,
                            CVec2f vfStartingOffset, ResHandle hFont, const ColorVector *pColors, int iFlags, CVec2ui v2SelStart, CVec2ui v2SelEnd)
{
    try
    {
        CVec4f v4OldColor(GetCurrentColor());

        // Retrieve the font map
        CFontMap *pFontMap(g_ResourceManager.GetFontMap(hFont));
        if (pFontMap == NULL)
            EX_WARN(_T("Failed to acquire font"));

        tsvector_cit itBegin(vStr.begin());
        tsvector_cit itEnd(vStr.end());
        size_t zCount(vStr.size());

        ColorVector::const_iterator itColor;
        if (pColors != NULL)
            itColor = pColors->begin();

        // Break apart strings that need to wrap
        tsvector vsSplitStrings;
        ColorVector vSplitColors;
        if (iFlags & DRAW_STRING_WRAP)
        {
            for (tsvector_cit it(vStr.begin()); it != vStr.end(); ++it)
            {
                if (pColors != NULL)
                {
                     WrapString(*it, pFontMap, w, vsSplitStrings, &(*itColor), &vSplitColors);
                    ++itColor;
                }
                else
                {
                    WrapString(*it, pFontMap, w, vsSplitStrings);
                }

            }

            itBegin = vsSplitStrings.begin();
            itEnd = vsSplitStrings.end();
            zCount = vsSplitStrings.size();
            itColor = vSplitColors.begin();
        }

        // Determine starting point
        float fYOffset(vfStartingOffset[Y]);
        float fXOffset(vfStartingOffset[X]);

        if (iFlags & DRAW_STRING_ANCHOR_BOTTOM)
            fYOffset = vfStartingOffset[Y] + (h - (zCount * pFontMap->GetMaxHeight()));

        uint uiLineCount(0);

        // Draw each line
        for (tsvector_cit it(itBegin); it != itEnd; ++it)
        {
            if (fYOffset < 0.0f)
            {
                fYOffset += pFontMap->GetMaxHeight();
                if (pColors != NULL)
                    ++itColor;

                uiLineCount++;
                continue;
            }

            if (pColors != NULL)
            {
                SetColor(*itColor);
                ++itColor;
            }

            if (uiLineCount < v2SelStart[Y] || uiLineCount > v2SelEnd[Y])
                String(x, y + fYOffset, w, h, *it, hFont, iFlags, 0, 0, fXOffset);
            else
                String(x, y + fYOffset, w, h, *it, hFont, iFlags, (uiLineCount == v2SelStart[Y] ? v2SelStart[X] : 0), (uiLineCount == v2SelEnd[Y] ? v2SelEnd[X] : it->length()), fXOffset);

            fYOffset += pFontMap->GetMaxHeight();
            if (fYOffset >= h)
                break;

            uiLineCount++;
        }

        if (pColors != NULL)
            SetColor(v4OldColor);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CDraw2D::String() - "), NO_THROW);
    }
}


void        CDraw2D::String(float x, float y, float w, float h, const tsvector &vStr,
                            float fStartingYOffset, ResHandle hFont, int iFlags)
{
    try
    {
        // Retrieve the font map
        CFontMap *pFontMap(g_ResourceManager.GetFontMap(hFont));
        if (pFontMap == NULL)
            EX_WARN(_T("Failed to acquire font"));

        size_t zCount(vStr.size());

        // Determine starting point
        float fYOffset(fStartingYOffset);

        if (iFlags & DRAW_STRING_ANCHOR_BOTTOM)
            fYOffset = fStartingYOffset + (h - (zCount * pFontMap->GetMaxHeight()));

        // Draw each line
        for (tsvector_cit it(vStr.begin()); it != vStr.end(); ++it)
        {
            if (fYOffset < 0.0f)
            {
                fYOffset += pFontMap->GetMaxHeight();
                continue;
            }

            String(x, y + fYOffset, w, h, *it, hFont, iFlags);
            fYOffset += pFontMap->GetMaxHeight();

            if (fYOffset + pFontMap->GetMaxHeight() >= h)
                break;
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CDraw2D::String() - "), NO_THROW);
    }
}

void    CDraw2D::String(float x, float y, float w, float h, const tsvector &vStr,
                           const ColorVector &vColors, ResHandle hFont, int iFlags)
{
    try
    {
        if (vColors.size() != vStr.size())
            EX_WARN(_T("Color and string vectors are different sizes"));

        String(x, y, w, h, vStr, hFont, iFlags, &vColors);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CDraw2D::String() - "), NO_THROW);
    }
}




/*====================
  CDraw2D::StringGlow
  ====================*/
void    CDraw2D::StringGlow(float x, float y, const tstring &sStr, ResHandle hFont, StrColorMap &vColors)
{
    try
    {
        bool  bGlow(false);
        if (m_hGlow == INVALID_RESOURCE)
            m_hGlow = g_ResourceManager.Register(K2_NEW(ctx_Effects,   CTexture)(_T("$glow"), TEXTURE_2D, 0, TEXFMT_A8R8G8B8), RES_TEXTURE);
            
        if (m_hGlow == INVALID_RESOURCE)
            throw CException(_T("Failed to load Glow effect"), E_WARNING);

        CVec4f v4OldColor(GetCurrentColor());

        // Get the fontmap to be used for rendering this string
        CFontMap *pFontMap(g_ResourceManager.GetFontMap(hFont));
        if (pFontMap == NULL)
            throw CException(_T("Failed to acquire font"), E_WARNING);

        // Get the color keys and a clean string
        tstring sClean(StripColorCodes(sStr));
        StrColorMap::iterator itColors(vColors.begin());

        // Draw each character
        float xPos(x);
        for (size_t z(0); z < sClean.length(); ++z)
        {
            // Check for color changes
            if (itColors != vColors.end())
            {
                if (itColors->first == z)
                {
                    if (itColors->second.x < 0.0f)
                        SetColor(v4OldColor);
                    else if (itColors->second.w == 3.14f)
                    {
                        bGlow = !bGlow;
                    }
                    else if (itColors->second.w > 3.14f)
                    {
                        bGlow = !bGlow;
                        CVec4f v4NewColor(itColors->second[0], itColors->second[1], itColors->second[2], v4OldColor[3]);
                        SetColor(v4NewColor);
                    }
                    else
                    {
                        //Set up using the colors from the iterator, but
                        //alpha value from the original color, so we can
                        //fade out text that has color codes in it.
                        CVec4f v4NewColor(itColors->second[0], itColors->second[1], itColors->second[2], v4OldColor[3]);
                        SetColor(v4NewColor);
                    }
                    ++itColors;
                }
            }

            float fKerning(z > 0 ? pFontMap->GetKerning(sClean[z - 1], sClean[z]) : 0.0f);
            const SCharacterMapInfo *pCharInfo(pFontMap->GetCharMapInfo(sClean[z]));
            if (pCharInfo == NULL)
            {
                Rect(xPos, y, pFontMap->GetMaxAdvance(), pFontMap->GetMaxHeight(), GUI_STRING);
                xPos += pFontMap->GetMaxAdvance();
                continue;
            }

            if (bGlow)
            {
                float fSizeW = (pCharInfo->m_fWidth * (3.0f + fabs(cosf((Host.GetTime() * 0.001f)) * 0.75f)));
                float fSizeH = (pFontMap->GetMaxHeight() * (1.5f + fabs(cosf((Host.GetTime() * 0.001f)) * 0.5f)));
                CVec4f v4Cur(GetCurrentColor());
                SetColor(v4Cur.x, v4Cur.y, v4Cur.z, (fabs(cosf((Host.GetTime() - y - (y * 2.0f)) * 0.0009f)) * 0.40f) + 0.3f);
                Rect(xPos + fKerning + pCharInfo->m_fBearingX - (fSizeW * 0.5f), y + (pFontMap->GetMaxAscender() - pCharInfo->m_fBearingY) - (fSizeH * 0.5f), pCharInfo->m_fWidth + fSizeW, pCharInfo->m_fHeight + fSizeH, m_hGlow, GUI_STRING);
                SetColor(v4Cur);
            }

            xPos += pCharInfo->m_fAdvance + fKerning;
        }

        SetColor(v4OldColor);
    }
    catch (CException &ex)
    {
        ex.Process(_TS("CDraw2D::StringGlow(") + sStr + _TS(") - "), NO_THROW);
    }
}

void    CDraw2D::StringGlow(float x, float y, float w, float h, const tstring &sStr, ResHandle hFont, int iFlags, float fStartingXOffset, StrColorMap &vColors)
{
    try
    {
        bool  bGlow(false);
        if (m_hGlow == INVALID_RESOURCE)
            m_hGlow = g_ResourceManager.Register(K2_NEW(ctx_Effects,   CTexture)(_T("$glow"), TEXTURE_2D, 0, TEXFMT_A8R8G8B8), RES_TEXTURE);
            
        if (m_hGlow == INVALID_RESOURCE)
            throw CException(_T("Failed to load Glow effect"), E_WARNING);

        CVec4f v4OldColor(GetCurrentColor());
        StrColorMap::iterator itColors;

        // Retrieve the font map
        CFontMap *pFontMap(g_ResourceManager.GetFontMap(hFont));
        if (pFontMap == NULL)
            throw CException(_T("Failed to acquire font"), E_WARNING);

        tstring sClean(StripColorCodes(sStr));
        itColors = vColors.begin();

        float fXPos(x);
        float fYPos(y);

        int iSkipChars(0);

        // Centering
        // FIXME: Handle centering properly for wrapped text
        // FIXME: Centered, unwrapped text can leave the bounds of the drawing area.
        if (iFlags & DRAW_STRING_CENTER)
            fXPos += ceil((w - pFontMap->GetStringWidth(sStr)) / 2.0f);
        else if (iFlags & DRAW_STRING_RIGHT)
            fXPos += ceil(w - pFontMap->GetStringWidth(sStr));

        if (iFlags & DRAW_STRING_VCENTER)
            fYPos += ceil((h - pFontMap->GetMaxHeight()) / 2.0f);
        else if (iFlags & DRAW_STRING_BOTTOM)
            fYPos += ceil(h - pFontMap->GetMaxHeight());

        fXPos += fStartingXOffset;

        // Draw each character
        for (size_t z(0); z < sClean.length(); ++z)
        {
            // Check for color changes
            if (itColors != vColors.end())
            {
                if (itColors->first == z)
                {
                    if (itColors->second.x < 0.0f || iFlags & DRAW_STRING_NOCOLORCODES)
                        SetColor(v4OldColor);
                    else if (itColors->second.w == 3.14f)
                    {
                        bGlow = !bGlow;
                    }
                    else if (itColors->second.w > 3.14f)
                    {
                        bGlow = !bGlow;
                        CVec4f v4NewColor(itColors->second[0], itColors->second[1], itColors->second[2], v4OldColor[3]);
                        SetColor(v4NewColor);
                    }
                    else
                    {
                        //Set up using the colors from the iterator, but
                        //alpha value from the original color, so we can
                        //fade out text that has color codes in it.
                        CVec4f v4NewColor(itColors->second[0], itColors->second[1], itColors->second[2], v4OldColor[3]);
                        SetColor(v4NewColor);
                    }
                    ++itColors;
                }
            }

            if (iSkipChars > 0)
            {
                --iSkipChars;
                continue;
            }

            float fKerning(z > 0 ? pFontMap->GetKerning(sClean[z - 1], sClean[z]) : 0.0f);
            const SCharacterMapInfo *pCharInfo(pFontMap->GetCharMapInfo(sClean[z]));
            if (pCharInfo == NULL)
            {
                Rect(fXPos, y, pFontMap->GetMaxAdvance(), pFontMap->GetMaxHeight(), GUI_STRING);
                fXPos += pFontMap->GetMaxAdvance();
                continue;
            }

            // Check for a width overflow
            float fWidth(pCharInfo->m_fWidth);
            float fS2(pCharInfo->m_fS2);
            if (fXPos + fKerning + pCharInfo->m_fAdvance > x + w)
            {
                if (iFlags & DRAW_STRING_WRAP)
                {
                    // Move to the next line
                    // Use MaxHeight instead of the char height to
                    // prevent text overlapping, but only use 2/3
                    // of it to also prevent a massive gap. 
                    //MikeG seems like a waste to do float mul on every char that goes over the size 
                    // on the screen and it makes the spaces to small. =)
                    fYPos += pFontMap->GetMaxHeight();//((pFontMap->GetMaxHeight() / 3) * 2);
                    fXPos = x;
                }
                else
                {
                    if (pCharInfo->m_fWidth > 0.0f)
                    {
                        // Clamp the width
                        fWidth += fXPos + fKerning - x - w;
                        fS2 = pCharInfo->m_fS1 + (pCharInfo->m_fS2 - pCharInfo->m_fS1) * (fWidth / pCharInfo->m_fWidth);
                    }
                }
            }

            // Clamp the height
            float fHeight(pCharInfo->m_fHeight);
            float fT1(pCharInfo->m_fT1);
            if (fYPos + fHeight > y + h)
            {
                if (pCharInfo->m_fHeight > 0.0f)
                {
                    fHeight = (y + h) - fYPos;
                    fT1 = pCharInfo->m_fT2 + (pCharInfo->m_fT1 - pCharInfo->m_fT2) * (fHeight / pCharInfo->m_fHeight);
                }
            }

            if (fXPos < x)
            {
                fXPos += pCharInfo->m_fAdvance + fKerning;
                continue;
            }

            if (bGlow)
            {
                float fSizeW = (fWidth * (3.0f + fabs(cosf((Host.GetTime() * 0.001f)) * 0.75f)));
                float fSizeH = (pFontMap->GetMaxHeight() * (1.5f + fabs(cosf((Host.GetTime() * 0.001f)) * 0.5f)));
                CVec4f v4Cur(GetCurrentColor());
                SetColor(v4Cur.x, v4Cur.y, v4Cur.z, (fabs(cosf((Host.GetTime() - (fYPos - y) - ((fYPos - y) * 2.0f)) * 0.0009f)) * 0.40f) + 0.3f);
                Rect(fXPos + fKerning + pCharInfo->m_fBearingX - (fSizeW * 0.5f), fYPos + (pFontMap->GetMaxAscender() - pCharInfo->m_fBearingY) - (fSizeH * 0.5f), fWidth + fSizeW, fHeight + fSizeH, m_hGlow, GUI_STRING);
                SetColor(v4Cur);
            }

            fXPos += pCharInfo->m_fAdvance + fKerning;

            if (fXPos > x + w)
                break;
        }

        SetColor(v4OldColor);
    }
    catch (CException &ex)
    {
        ex.Process(_TS("CDraw2D::StringGlow(") + sStr + _TS(") - "), NO_THROW);
    }
}

void    CDraw2D::StringGlow(float x, float y, float w, float h, const tstring &sStr, ResHandle hFont, uivector vWrappingBreakList, fvector vLineCentering, int iFlags, StrColorMap &vColors)
{
    try
    {
        
        bool  bGlow(false);
        if (m_hGlow == INVALID_RESOURCE)
            m_hGlow = g_ResourceManager.Register(K2_NEW(ctx_Effects,   CTexture)(_T("$glow"), TEXTURE_2D, 0, TEXFMT_A8R8G8B8), RES_TEXTURE);
            
        if (m_hGlow == INVALID_RESOURCE)
            throw CException(_T("Failed to load Glow effect"), E_WARNING);

        CVec4f v4OldColor(GetCurrentColor());
        StrColorMap::iterator itColors;

        // Retrieve the font map
        CFontMap *pFontMap(g_ResourceManager.GetFontMap(hFont));
        if (pFontMap == NULL)
            throw CException(_T("Failed to acquire font"), E_WARNING);

        // Get the color keys and a clean string
        tstring sClean(StripColorCodes(sStr));
        itColors = vColors.begin();

        float fXPos(x);
        float fYPos(y);

        int iSkipChars(0);
        float fStrWidth(0.0f);

        fvector::iterator itLineCentering(vLineCentering.begin());

        if (!vLineCentering.empty())
            fStrWidth = (*itLineCentering);
        else
            fStrWidth = pFontMap->GetStringWidth(sClean);

        // Centering
        if (iFlags & DRAW_STRING_CENTER)
            fXPos += ceil((w - fStrWidth) / 2.0f);
        else if (iFlags & DRAW_STRING_RIGHT)
            fXPos += ceil(w - fStrWidth);

        //Center/bottom vertial align according to the size of the text and the height
        if (iFlags & DRAW_STRING_VCENTER)
            fYPos += ceil(((h - pFontMap->GetMaxHeight()) - (vWrappingBreakList.size() * pFontMap->GetMaxHeight())) / 2.0f);
        else if (iFlags & DRAW_STRING_BOTTOM)
            fYPos += ceil(((h - pFontMap->GetMaxHeight()) - (vWrappingBreakList.size() * pFontMap->GetMaxHeight())));


        uivector::iterator itWrapStart(vWrappingBreakList.begin());

        // Draw each character
        for (size_t z(0); z < sClean.length(); ++z)
        {
            // Check for color changes
            if (itColors != vColors.end())
            {
                if (itColors->first == z)
                {
                    if (itColors->second.x < 0.0f || iFlags & DRAW_STRING_NOCOLORCODES)
                        SetColor(v4OldColor);
                    else if (itColors->second.w == 3.14f)
                    {
                        bGlow = !bGlow;
                    }
                    else if (itColors->second.w > 3.14f)
                    {
                        bGlow = !bGlow;
                        CVec4f v4NewColor(itColors->second[0], itColors->second[1], itColors->second[2], v4OldColor[3]);
                        SetColor(v4NewColor);
                    }
                    else
                    {
                        //Set up using the colors from the iterator, but
                        //alpha value from the original color, so we can
                        //fade out text that has color codes in it.
                        CVec4f v4NewColor(itColors->second[0], itColors->second[1], itColors->second[2], v4OldColor[3]);
                        SetColor(v4NewColor);
                    }
                    ++itColors;
                }
            }

            if (iSkipChars > 0)
            {
                --iSkipChars;
                continue;
            }

            float fKerning(z > 0 ? pFontMap->GetKerning(sClean[z - 1], sClean[z]) : 0.0f);
            const SCharacterMapInfo *pCharInfo(pFontMap->GetCharMapInfo(sClean[z]));
            if (pCharInfo == NULL)
            {
                Rect(fXPos, y, pFontMap->GetMaxAdvance(), pFontMap->GetMaxHeight(), GUI_STRING);
                fXPos += pFontMap->GetMaxAdvance();
                continue;
            }

            // Check for a width overflow
            float fWidth(pCharInfo->m_fWidth);
            float fS2(pCharInfo->m_fS2);
            if (fXPos + fWidth > x + w)
            {
                fWidth = (x + w) - (fXPos + fKerning);
                fS2 = pCharInfo->m_fS1 + (pCharInfo->m_fS2 - pCharInfo->m_fS1) * (fWidth / pCharInfo->m_fWidth);
            }

            // Clamp the height
            float fHeight(pCharInfo->m_fHeight);
            float fT1(pCharInfo->m_fT1);
            if (fYPos + fHeight > y + h)
            {
                fHeight = (y + h) - fYPos;
                fT1 = pCharInfo->m_fT2 + (pCharInfo->m_fT1 - pCharInfo->m_fT2) * (fHeight / pCharInfo->m_fHeight);
            }

            if (fXPos < x)
            {
                fXPos = x;//pCharInfo->m_fAdvance + fKerning;
            }

            if (fYPos < y)
            {
                fYPos = y;//pCharInfo->m_fHeight;
            }

            if (bGlow && !IsTokenSeparator(sClean[z]))
            {
                float fSizeW = (fWidth * (3.0f + fabs(cosf((Host.GetTime() * 0.001f)) * 0.75f)));
                float fSizeH = (pFontMap->GetMaxHeight() * (1.5f + fabs(cosf((Host.GetTime() * 0.001f)) * 0.5f)));
                CVec4f v4Cur(GetCurrentColor());
                SetColor(v4Cur.x, v4Cur.y, v4Cur.z, (fabs(cosf((Host.GetTime() - (fYPos - y) - ((fYPos - y) * 2.0f)) * 0.0009f)) * 0.40f) + 0.3f);
                Rect(fXPos + fKerning + pCharInfo->m_fBearingX - (fSizeW * 0.5f), fYPos + (pFontMap->GetMaxAscender() - pCharInfo->m_fBearingY) - (fSizeH * 0.5f), fWidth + fSizeW, fHeight + fSizeH, m_hGlow, GUI_STRING);
                SetColor(v4Cur);
            }

            fXPos += pCharInfo->m_fAdvance + fKerning;

            if (vWrappingBreakList.size() > 0)
            {
                for (uivector::iterator itWrap(itWrapStart); itWrap != vWrappingBreakList.end(); ++itWrap)
                {
                    if ((*itWrap) == z)
                    {
                        itWrapStart = itWrap + 1;
                        fYPos += pFontMap->GetMaxHeight();
                        fXPos = x;

                        if (vLineCentering.size() > 0)
                        {
                            if (itLineCentering != vLineCentering.end())
                            {
                                itLineCentering++;
                                fStrWidth =(*itLineCentering);
                            }
                            else
                            {
                                fStrWidth =(*itLineCentering);
                            }
                            if (iFlags & DRAW_STRING_CENTER)
                                fXPos += ceil((w - fStrWidth) / 2.0f);
                            else if (iFlags & DRAW_STRING_RIGHT)
                                fXPos += ceil(w - fStrWidth);
                        }
                        break;
                    }
                }
            }

            if (fXPos > x + w)
                break;
        }

        SetColor(v4OldColor);
    }
    catch (CException &ex)
    {
        ex.Process(_TS("CDraw2D::StringGlow(") + sStr + _TS(") - "), NO_THROW);
    }
}

/*====================
  CDraw2D::SetColor
  ====================*/
void    CDraw2D::SetColor(const CVec4f &v4Color)
{
    m_v4CurrentColor = v4Color;
    Vid.SetColor(m_v4CurrentColor);
}


/*====================
  CDraw2D::GetScreenW
  ====================*/
float   CDraw2D::GetScreenW()
{
    return Vid.GetScreenW() / draw_scale;
}


/*====================
  CDraw2D::GetScreenH
  ====================*/
float   CDraw2D::GetScreenH()
{
    return Vid.GetScreenH() / draw_scale;
}


/*====================
  CDraw2D::RegisterSmiley
  ====================*/
void    CDraw2D::RegisterSmiley(const tstring &sSmiley, const tstring &sPath)
{
    ResHandle hSmiley;

    UITextureRegistry.Register(sPath, 0, hSmiley);

    m_mapSmileys[sSmiley] = hSmiley;
}


/*--------------------
  RegisterSmiley
  --------------------*/
CMD(RegisterSmiley)
{
    if (vArgList.size() < 2)
        return false;

    Draw2D.RegisterSmiley(vArgList[0], vArgList[1]);

    return true;
}
