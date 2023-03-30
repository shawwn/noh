// (C)2005 S2 Games
// c_textbuffer.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_textbuffer_scrollbar.h"
#include "c_textbuffer.h"
#include "c_interface.h"
#include "c_uicmd.h"
#include "c_widgetstyle.h"
#include "c_uimanager.h"
#include "c_cmd.h"
#include "c_draw2d.h"
#include "c_fontmap.h"
#include "c_uitrigger.h"
#include "c_memmanager.h"
#include "c_socket.h"
#include "c_chatmanager.h"
#include "c_resourcemanager.h"
//=============================================================================

/*====================
  CTextBuffer::~CTextBuffer
  ====================*/
CTextBuffer::~CTextBuffer()
{
    SAFE_DELETE(m_pScrollbar);
}


/*====================
  CTextBuffer::CTextBuffer
  ====================*/
CTextBuffer::CTextBuffer(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style) :
IWidget(pInterface, pParent, WIDGET_TEXTBUFFER, style),
m_bShadow(style.GetPropertyBool(_T("shadow"), false)),
m_fShadowOffset(style.GetPropertyFloat(_T("shadowoffset"), 1.0f)),
m_bWrap(style.GetPropertyBool(_T("wrap"), true)),
m_bAnchorBottom(style.GetPropertyBool(_T("anchorbottom"), false)),
m_bExteriorScrollbars(style.GetPropertyBool(_T("exteriorscrollbars"), false)),
m_bUseSmileys(style.GetPropertyBool(_T("usesmileys"), false)),
m_bScrollbarPlaceholder(style.GetPropertyBool(_T("scrollbarplaceholder"), false)),
m_bUseScrollbar(style.GetPropertyBool(_T("usescrollbar"), true)),
m_v4TextColor(GetColorFromString(style.GetProperty(_T("textcolor"), _T("silver")))),
m_v4ShadowColor(GetColorFromString(style.GetProperty(_T("shadowcolor"), _T("black")))),
m_fShadowOffsetX(style.GetPropertyFloat(_T("shadowoffsetx"), 0.0f)),
m_fShadowOffsetY(style.GetPropertyFloat(_T("shadowoffsety"), 0.0f)),
m_bOutline(style.GetPropertyBool(_T("outline"), false)),
m_v4OutlineColor(GetColorFromString(style.GetProperty(_T("outlinecolor"), _T("#000000")))),
m_iOutlineOffset(style.GetPropertyFloat(_T("outlineoffset"), 1)),
m_iMaxLines(style.GetPropertyInt(_T("maxlines"), -1)),
m_iFadeLength(style.GetPropertyInt(_T("fadetime"), -1)),
m_fScrollbarSize(GetSizeFromString(style.GetProperty(_T("scrollbarsize"), _T("16")), pParent->GetWidth(), pParent->GetHeight())),
m_pScrollbar(NULL),
m_sFile(style.GetProperty(_T("file"), TSNULL)),
m_vSelStart(uint(-1),uint(-1)),
m_vStart(0,0),
m_vInputPos(0,0),
m_bEditable(style.GetPropertyBool(_T("editable"), false)),
m_hFontMap(INVALID_RESOURCE),
m_pFontMap(NULL),
m_bLinkToChat(style.GetPropertyBool(_T("linktochat"), false)),
m_bIsGameChat(style.GetPropertyBool(_T("isgamechat"), false)),
m_sChannelName(style.GetProperty(_T("chatchannel"), TSNULL))
{

    if (m_fShadowOffsetX==0.0f && m_fShadowOffsetY==0.0f)
    {
        m_fShadowOffsetX=m_fShadowOffset;
        m_fShadowOffsetY=m_fShadowOffset;
    }

    if (m_bEditable)
        SetFlags(WFLAG_INTERACTIVE);

    // Load the font
    SetFont(style.GetProperty(_T("font"), _T("system_medium")));

    // Text Alignment
    const tstring sLeft(_T("left"));
    const tstring &sTextAlign(style.GetProperty(_T("textalign"), sLeft));
    if (sTextAlign == _T("left"))
        m_eTextAlignment = ALIGN_LEFT;
    if (sTextAlign == _T("center"))
        m_eTextAlignment = ALIGN_CENTER;
    else if (sTextAlign == _T("right"))
        m_eTextAlignment = ALIGN_RIGHT;

    if (style.HasProperty(_T("y")) && !style.HasProperty(_T("height")))
    {
        if (m_pFontMap)
            m_recArea.SetSizeY(m_pFontMap->GetMaxHeight());
    }

    if (style.HasProperty(_T("x")) && !style.HasProperty(_T("width")))
    {
        if (m_pFontMap && !m_vsText.empty())
            m_recArea.SetSizeX(m_pFontMap->GetStringWidth(m_vsText.front()));
    }


    if (!m_bAnchorBottom)
        m_vStart[Y] = 0;
    else
        m_vStart[Y] = 1;

    //Create a copy of the style and remove unneeded properties
    //so we can setup a style for our scrollbar
    CWidgetStyle styleCopy(style);
    styleCopy.RemoveProperty(_T("name"));
    styleCopy.RemoveProperty(_T("group"));
    styleCopy.RemoveProperty(_T("onselect"));
    styleCopy.RemoveProperty(_T("onframe"));
    styleCopy.RemoveProperty(_T("ontrigger"));
    styleCopy.RemoveProperty(_T("onshow"));
    styleCopy.RemoveProperty(_T("onhide"));
    styleCopy.RemoveProperty(_T("onenable"));
    styleCopy.RemoveProperty(_T("ondisable"));
    styleCopy.RemoveProperty(_T("onchange"));
    styleCopy.RemoveProperty(_T("onslide"));
    styleCopy.RemoveProperty(_T("onselect"));
    styleCopy.RemoveProperty(_T("onclick"));
    styleCopy.RemoveProperty(_T("ondoubleclick"));
    styleCopy.RemoveProperty(_T("onrightclick"));
    styleCopy.RemoveProperty(_T("onfocus"));
    styleCopy.RemoveProperty(_T("onlosefocus"));
    styleCopy.RemoveProperty(_T("onload"));
    styleCopy.RemoveProperty(_T("form"));
    styleCopy.RemoveProperty(_T("data"));
    styleCopy.RemoveProperty(_T("watch"));
    styleCopy.RemoveProperty(_T("ontrigger"));

    for (int i(0); i < 10; ++i)
    {
        styleCopy.RemoveProperty(_T("watch") + XtoA(i));
        styleCopy.RemoveProperty(_T("ontrigger") + XtoA(i));
    }

    styleCopy.RemoveProperty(_T("valign"));
    styleCopy.SetProperty(_T("align"), _T("right"));

    styleCopy.SetProperty(_T("visible"), m_bScrollbarPlaceholder);
    styleCopy.SetProperty(_T("color"), _T("white"));
    styleCopy.SetProperty(_T("handlecolor"), _T("white"));
    styleCopy.SetProperty(_T("slotcolor"), _T("white"));
    styleCopy.SetProperty(_T("texture"), style.GetProperty(_T("scrolltexture"), _T("/ui/elements/standardscroll.tga")));

    styleCopy.SetProperty(_T("x"), (m_bExteriorScrollbars ? XtoA(m_fScrollbarSize) : _T("0.0f")));
    styleCopy.SetProperty(_T("y"), 0.0f);
    styleCopy.SetProperty(_T("width"), m_fScrollbarSize);
    styleCopy.SetProperty(_T("height"), _T("100%"));
    styleCopy.SetProperty(_T("vertical"), true);

    if (!GetName().empty())
        styleCopy.SetProperty(_T("name"), GetName() + _T("_scroll"));

    m_pScrollbar = K2_NEW(ctx_Widgets,  CTextBufferScrollbar)(m_pInterface, this, styleCopy);

    m_pScrollbar->SetMaxValue(0.00001f);
    m_pScrollbar->SetValue(0.00001f);

    AddChild(m_pScrollbar);

    // Default Text
    m_sDefaultText = style.GetProperty(_T("content"), TSNULL);
    SetText(m_sDefaultText);

    // File processing
    if (!m_sFile.empty())
    {
        tstring sText;
        CFile *pFile = FileManager.GetFile(m_sFile, FILE_READ | FILE_TEXT | FILE_ASCII | FILE_ALLOW_CUSTOM);

        if (pFile != NULL)
        {
            while (pFile->IsOpen() && !pFile->IsEOF())
            {
                sText = pFile->ReadLine();
                AddText(sText, Host.GetTime());
            }

            pFile->Close();
            SAFE_DELETE(pFile);
        }
    }
    
    if (m_vsText.size() == 0)
        AddText(TSNULL, Host.GetTime());

    if (m_bLinkToChat)
        ChatManager.AddWidgetReference(this, m_bIsGameChat, m_sChannelName);

    if (IsAbsoluteVisible())
        DO_EVENT(WEVENT_SHOW)
}


/*====================
  CTextBuffer::ReloadFile
  ====================*/
void    CTextBuffer::ReloadFile()
{
    // Default Text
    SetText(m_sDefaultText);

    // File processing
    if (!m_sFile.empty())
    {
        tstring sText;
        CFile *pFile = FileManager.GetFile(m_sFile, FILE_READ | FILE_TEXT);

        if (pFile != NULL)
        {
            while (pFile->IsOpen() && !pFile->IsEOF())
            {
                sText = pFile->ReadLine();
                AddText(sText, Host.GetTime());
            }

            pFile->Close();
            SAFE_DELETE(pFile);
        }
    }
}


/*====================
  CTextBuffer::Char
  ====================*/
bool    CTextBuffer::Char(TCHAR c)
{
    if (unsigned(c) < _T(' ') || !m_bEditable || m_vInputPos[Y] >= m_vsText.size() || m_vInputPos[X] > m_vsText[m_vInputPos[Y]].length())
        return true;

    if (m_vSelStart != CVec2ui(uint(-1), uint(-1)))
    {
        CVec2ui vMin;
        CVec2ui vMax;

        if (m_vSelStart[Y] < m_vInputPos[Y])
        {
            vMin = m_vSelStart;
            vMax = m_vInputPos;
        }
        else if (m_vSelStart[Y] > m_vInputPos[Y])
        {
            vMin = m_vInputPos;
            vMax = m_vSelStart;
        }
        else
        {
            vMin = CVec2ui(MIN(m_vInputPos[X], m_vSelStart[X]), m_vInputPos[Y]);
            vMax = CVec2ui(MAX(m_vInputPos[X], m_vSelStart[X]), m_vInputPos[Y]);
        }

        for (uint i(vMin[Y]); i <= vMax[Y]; i++)
        {
            size_t zStart(0);
            size_t zEnd(m_vsText[i].length());

            if (i == vMin[Y])
                zStart = vMin[X];

            if (i == vMax[Y])
                zEnd = vMax[X];

            m_vsText[i].erase(zStart, zEnd - zStart);

            if (m_vsText[i].empty() && i != vMin[Y])
            {
                m_vsText.erase(m_vsText.begin() + i);
                i--;
                vMax[Y]--;
            }
        }

        m_vInputPos = vMin;
        m_vSelStart = CVec2ui(uint(-1), uint(-1));
    }

    if (m_vInputPos[X] < INT_SIZE(m_vsText[m_vInputPos[Y]].length()))
        m_vsText[m_vInputPos[Y]].insert(m_vInputPos[X], 1, c);
    else
        m_vsText[m_vInputPos[Y]].append(1, c);

    m_vInputPos[X]++;

    if (m_vStart[X] > m_vInputPos[X])
        m_vStart[X] = 0;

    if (m_vInputPos[Y] < m_vStart[Y])
        m_vStart[Y] = m_vInputPos[Y];

    float fWidth(0.0f);
    for (uint i(m_vInputPos[X]); i >= m_vStart[X] && i != uint(-1); i--)
    {
        fWidth += m_pFontMap->GetCharMapInfo(m_vsText[m_vInputPos[Y]][i])->m_fAdvance;

        if (fWidth > GetWidth())
        {
            m_vStart[X] = i + 1;
            break;
        }
    }

    DO_EVENT_RETURN(WEVENT_CHANGE, true)

    return true;
}


/*====================
  CTextBox::ButtonDown
  ====================*/
bool    CTextBuffer::ButtonDown(EButton button)
{
    if (m_vsText.empty() || !m_bEditable)
        return false;

    switch (button)
    {
    case BUTTON_BACKSPACE:
        if (m_vSelStart == CVec2ui(uint(-1), uint(-1)))
        {
            if (m_vInputPos[X] > 0 || m_vInputPos[Y] > 0)
                m_vInputPos[X]--;

            if (m_vInputPos[X] >= 0 && m_vInputPos[X] < m_vStart[X])
                m_vStart[X] = m_vInputPos[X];
        }

        // fall through to BUTTON_DEL
    case BUTTON_DEL:
        if (m_vSelStart != CVec2ui(uint(-1), uint(-1)))
        {
            CVec2ui vMin;
            CVec2ui vMax;

            if (m_vSelStart[Y] < m_vInputPos[Y])
            {
                vMin = m_vSelStart;
                vMax = m_vInputPos;
            }
            else if (m_vSelStart[Y] > m_vInputPos[Y])
            {
                vMin = m_vInputPos;
                vMax = m_vSelStart;
            }
            else
            {
                vMin = CVec2ui(MIN(m_vInputPos[X], m_vSelStart[X]), m_vInputPos[Y]);
                vMax = CVec2ui(MAX(m_vInputPos[X], m_vSelStart[X]), m_vInputPos[Y]);
            }

            for (uint i(vMin[Y]); i <= vMax[Y]; i++)
            {
                size_t zStart(0);
                size_t zEnd(m_vsText[i].length());

                if (i == vMin[Y])
                    zStart = vMin[X];

                if (i == vMax[Y])
                    zEnd = vMax[X];

                m_vsText[i].erase(zStart, zEnd - zStart);

                if (m_vsText[i].empty() && i != vMin[Y])
                {
                    m_vsText.erase(m_vsText.begin() + i);
                    i--;
                    vMax[Y]--;
                }
            }

            m_vInputPos = vMin;
            m_vSelStart = CVec2ui(uint(-1), uint(-1));

            if (m_vStart[X] > m_vInputPos[X])
                m_vStart[X] = m_vInputPos[X];

            if (m_vInputPos[Y] < m_vStart[Y])
                m_vStart[Y] = m_vInputPos[Y];

            DO_EVENT_RETURN(WEVENT_CHANGE, true)

            break;
        }

        if (m_vInputPos[Y] >= 0 && m_vInputPos[Y] < m_vsText.size() && (m_vInputPos[X] <= m_vsText[m_vInputPos[Y]].length() || m_vInputPos[X] == uint(-1)))
        {
            if (m_vInputPos[X] >= 0 && m_vInputPos[X] < m_vsText[m_vInputPos[Y]].length())
                m_vsText[m_vInputPos[Y]].erase(m_vInputPos[X], 1);
            else if (m_vInputPos[X] == uint(-1) && m_vInputPos[Y] != uint(-1))
            {
                if (m_vsText[m_vInputPos[Y]].empty() && m_vsText.size() > 1)
                {
                    m_vsText.erase(m_vsText.begin() + m_vInputPos[Y]);
                    m_vInputPos[Y]--;
                    m_vInputPos[X] = INT_SIZE(m_vsText[m_vInputPos[Y]].length());

                    if (m_vInputPos[Y] < m_vStart[Y])
                        m_vStart[Y] = m_vInputPos[Y];
                }
                else if (m_vsText.size() > 1)
                {
                    tstring sString(m_vsText[m_vInputPos[Y]]);
                    
                    m_vsText.erase(m_vsText.begin() + m_vInputPos[Y]);
                    
                    m_vInputPos[Y]--;
                    m_vInputPos[X] = INT_SIZE(m_vsText[m_vInputPos[Y]].length());
                    m_vsText[m_vInputPos[Y]].append(sString);

                    if (m_vInputPos[Y] < m_vStart[Y])
                        m_vStart[Y] = m_vInputPos[Y];
                }
            }
            else if (m_vInputPos[X] == m_vsText[m_vInputPos[Y]].length() && m_vInputPos[Y] < m_vsText.size() - 1)
            {
                tstring sString(m_vsText[m_vInputPos[Y] + 1]);
                m_vsText.erase(m_vsText.begin() + (m_vInputPos[Y] + 1));
                m_vsText[m_vInputPos[Y]].append(sString);
            }
        }

        DO_EVENT_RETURN(WEVENT_CHANGE, true)

        break;

    case BUTTON_ESC:
        m_vInputPos = CVec2ui(0, 0);
        m_vStart = CVec2ui(0, 0);
        m_vSelStart = CVec2ui(uint(uint(-1)), uint(uint(-1)));
        m_pInterface->SetActiveWidget(NULL);
        break;

    case BUTTON_HOME:
        if (Input.IsShiftDown() && m_vSelStart == CVec2ui(uint(-1), uint(-1)) && m_vInputPos[X] != 0)
            m_vSelStart = m_vInputPos;
        else if (!Input.IsShiftDown())
            m_vSelStart = CVec2ui(uint(-1), uint(-1));

        m_vInputPos[X] = 0;
        m_vStart[X] = 0;
        break;

    case BUTTON_END:
        {
            if (Input.IsShiftDown() && m_vSelStart == CVec2ui(uint(-1), uint(-1)) && m_vInputPos[Y] < m_vsText.size() && m_vInputPos[X] < m_vsText[m_vInputPos[Y]].length())
                m_vSelStart = m_vInputPos;
            else if (!Input.IsShiftDown())
                m_vSelStart = CVec2ui(uint(-1), uint(-1));

            if (m_vInputPos[Y] < m_vsText.size())
                m_vInputPos[X] = INT_SIZE(m_vsText[m_vInputPos[Y]].length());

            m_vStart[X] = 0;

            float fWidth(0.0f);
            for (uint i(m_vInputPos[X]); i >= m_vStart[X] && i != uint(-1); i--)
            {
                fWidth += m_pFontMap->GetCharMapInfo(m_vsText[m_vInputPos[Y]][i])->m_fAdvance;

                if (fWidth > GetWidth())
                {
                    m_vStart[X] = i + 1;
                    break;
                }
            }
        }
        break;

    case BUTTON_LEFT:
        if (Input.IsShiftDown() && m_vSelStart == CVec2ui(uint(-1), uint(-1)) && (m_vInputPos[X] > 0 || m_vInputPos[Y] > 0))
            m_vSelStart = m_vInputPos;
        else if (!Input.IsShiftDown())
            m_vSelStart = CVec2ui(uint(-1), uint(-1));

        if (m_vInputPos[X] > 0 || m_vInputPos[Y] > 0)
            m_vInputPos[X]--;

        if (m_vInputPos[X] == uint(-1))
        {
            m_vInputPos[Y]--;

            if (m_vInputPos[Y] < m_vsText.size())
                m_vInputPos[X] = INT_SIZE(m_vsText[m_vInputPos[Y]].length());

            if (m_vInputPos[Y] < m_vStart[Y])
                m_vStart[Y] = m_vInputPos[Y];
            
            float fWidth(0.0f);
            for (uint i(m_vInputPos[X]); i >= m_vStart[X] && i != uint(-1); i--)
            {
                fWidth += m_pFontMap->GetCharMapInfo(m_vsText[m_vInputPos[Y]][i])->m_fAdvance;

                if (fWidth > GetWidth())
                {
                    m_vStart[X] = i + 1;
                    break;
                }
            }
        }

        if (m_vInputPos[X] < m_vStart[X])
            m_vStart[X] = m_vInputPos[X];

        break;

    case BUTTON_RIGHT:
        if (Input.IsShiftDown() && m_vSelStart == CVec2ui(uint(-1), uint(-1)) && (
            (m_vInputPos[Y] < m_vsText.size() && m_vInputPos[X] < m_vsText[m_vInputPos[Y]].length()) ||
            (m_vInputPos[Y] < m_vsText.size() - 1 && m_vInputPos[X] == m_vsText[m_vInputPos[Y]].length())))
            m_vSelStart = m_vInputPos;
        else if (!Input.IsShiftDown())
            m_vSelStart = CVec2ui(uint(-1), uint(-1));

        if ((m_vInputPos[Y] < m_vsText.size() && m_vInputPos[X] < m_vsText[m_vInputPos[Y]].length()) ||
            (m_vInputPos[Y] < m_vsText.size() - 1 && m_vInputPos[X] == m_vsText[m_vInputPos[Y]].length()))
            m_vInputPos[X]++;

        if (m_vInputPos[Y] < m_vsText.size() - 1 && m_vInputPos[X] == m_vsText[m_vInputPos[Y]].length() + 1)
        {
            m_vInputPos[Y]++;
            m_vInputPos[X] = 0;
            m_vStart[X] = 0;

            float fHeight(0.0f);
            float fStep(m_pFontMap->GetMaxHeight());
            for (uint i(m_vInputPos[Y]); i >= m_vStart[Y] && i != uint(-1); i--)
            {
                fHeight += fStep;

                if (fHeight > GetHeight())
                {
                    m_vStart[Y] = i + 1;
                    break;
                }
            }
        }
        else
        {
            float fWidth(0.0f);
            for (uint i(m_vInputPos[X]); i >= m_vStart[X] && i != uint(-1); i--)
            {
                fWidth += m_pFontMap->GetCharMapInfo(m_vsText[m_vInputPos[Y]][i])->m_fAdvance;

                if (fWidth > GetWidth())
                {
                    m_vStart[X] = i + 1;
                    break;
                }
            }
        }
        break;

    case BUTTON_UP:
        ScrollUp();
        break;

    case BUTTON_DOWN:
        ScrollDown();
        break;

    case BUTTON_ENTER:
        if (m_vSelStart != CVec2ui(uint(-1), uint(-1)))
        {
            CVec2ui vMin;
            CVec2ui vMax;

            if (m_vSelStart[Y] < m_vInputPos[Y])
            {
                vMin = m_vSelStart;
                vMax = m_vInputPos;
            }
            else if (m_vSelStart[Y] > m_vInputPos[Y])
            {
                vMin = m_vInputPos;
                vMax = m_vSelStart;
            }
            else
            {
                vMin = CVec2ui(MIN(m_vInputPos[X], m_vSelStart[X]), m_vInputPos[Y]);
                vMax = CVec2ui(MAX(m_vInputPos[X], m_vSelStart[X]), m_vInputPos[Y]);
            }

            for (uint i(vMin[Y]); i <= vMax[Y]; i++)
            {
                size_t zStart(0);
                size_t zEnd(m_vsText[i].length());

                if (i == vMin[Y])
                    zStart = vMin[X];

                if (i == vMax[Y])
                    zEnd = vMax[X];

                m_vsText[i].erase(zStart, zEnd - zStart);

                if (m_vsText[i].empty() && i != vMin[Y])
                {
                    m_vsText.erase(m_vsText.begin() + i);
                    i--;
                    vMax[Y]--;
                }
            }

            m_vInputPos = vMin;
            m_vSelStart = CVec2ui(uint(-1), uint(-1));

            if (m_vStart[X] > m_vInputPos[X])
                m_vStart[X] = m_vInputPos[X];

            if (m_vInputPos[Y] < m_vStart[Y])
                m_vStart[Y] = m_vInputPos[Y];
        }

        tstring sString;

        if (m_vInputPos[Y] < m_vsText.size())
        {
            if (m_vInputPos[X] < m_vsText[m_vInputPos[Y]].length())
            {
                sString = m_vsText[m_vInputPos[Y]].substr(m_vInputPos[X], m_vsText[m_vInputPos[Y]].length() - m_vInputPos[X]);
                m_vsText[m_vInputPos[Y]].erase(m_vInputPos[X], m_vsText[m_vInputPos[Y]].length() - m_vInputPos[X]);
            }

            m_vInputPos[Y]++;
            m_vInputPos[X] = 0;

            m_vStart[X] = 0;

            float fHeight(0.0f);
            float fStep(m_pFontMap->GetMaxHeight());
            for (uint i(m_vInputPos[Y]); i >= m_vStart[Y] && i != uint(-1); i--)
            {
                fHeight += fStep;

                if (fHeight > GetHeight())
                {
                    m_vStart[Y] = i + 1;
                    break;
                }
            }

            m_vsText.insert(m_vsText.begin() + m_vInputPos[Y], sString);
            m_viFadeTime.insert(m_viFadeTime.begin() + m_vInputPos[Y], Host.GetTime() + m_iFadeLength);
            m_vFadeColor.insert(m_vFadeColor.begin() + m_vInputPos[Y], m_v4TextColor);
            m_vFadeShadowColor.insert(m_vFadeShadowColor.begin() + m_vInputPos[Y], m_v4ShadowColor);
            m_vFadeOutlineColor.insert(m_vFadeOutlineColor.begin() + m_vInputPos[Y], m_v4OutlineColor);
        }

        DO_EVENT_RETURN(WEVENT_CHANGE, true)

        break;
    }

    return true;
}


/*====================
  CTextBuffer::RenderWidget
  ====================*/
void    CTextBuffer::RenderWidget(const CVec2f &v2Origin, float fFade)
{
    PROFILE("CTextBuffer::RenderWidget");

    if (!HasFlags(WFLAG_VISIBLE))
        return;

    IWidget::RenderWidget(v2Origin, fFade);

    if (m_vsText.empty() && !m_bScrollbarPlaceholder)
        return;

    int iFlags(0);
    float fX(v2Origin.x);
    float fY(v2Origin.y);
    float fWidth(m_recArea.GetWidth());
    float fHeight(m_recArea.GetHeight());

    float fXOffset(m_pFontMap->GetStringWidth(m_vsText[m_vInputPos[Y]].substr(0, m_vStart[X])));
    float fYOffset(m_vStart[Y] * m_pFontMap->GetMaxHeight());

    if (m_bAnchorBottom)
        fYOffset *= -1;

    CVec2ui v2SelStart(uint(-1),uint(-1));
    CVec2ui v2SelEnd(uint(-1),uint(-1));

    if (m_vSelStart != CVec2ui(uint(-1),uint(-1)))
    {
        if (m_vSelStart[Y] < m_vInputPos[Y])
        {
            v2SelStart = m_vSelStart;
            v2SelEnd = m_vInputPos;
        }
        else if (m_vSelStart[Y] > m_vInputPos[Y])
        {
            v2SelStart = m_vInputPos;
            v2SelEnd = m_vSelStart;
        }
        else
        {
            v2SelStart = CVec2ui(MIN(m_vInputPos[X], m_vSelStart[X]), m_vInputPos[Y]);
            v2SelEnd = CVec2ui(MAX(m_vInputPos[X], m_vSelStart[X]), m_vInputPos[Y]);
        }
    }

    CVec2f v2Offset(-fXOffset, -fYOffset);

    if (fWidth == 0.0f)
        fWidth = Draw2D.GetScreenW() - fX;

    if (fHeight == 0.0f)
        fHeight = Draw2D.GetScreenH() - fY;

    if (m_eTextAlignment == ALIGN_CENTER)
        iFlags |= DRAW_STRING_CENTER;

    if (m_bAnchorBottom)
        iFlags |= DRAW_STRING_ANCHOR_BOTTOM;

    if (m_bUseSmileys)
        iFlags |= DRAW_STRING_SMILEYS;

    if (m_bShadow)
    {
        //Remove color codes in the text for the shadow
        iFlags |= DRAW_STRING_NOCOLORCODES;
        Draw2D.String(fX + m_fShadowOffsetX, fY + m_fShadowOffsetY, fWidth - (m_bExteriorScrollbars ? 0.0f : m_fScrollbarSize), fHeight, m_vsText, v2Offset, m_hFontMap, &m_vFadeShadowColor, iFlags, CVec2ui(uint(-1), uint(-1)), CVec2ui(uint(-1), uint(-1)));
    }
    if (m_bOutline)
    {
        //Remove color codes in the text for the shadow
        iFlags |= DRAW_STRING_NOCOLORCODES;
        Draw2D.String(fX + m_iOutlineOffset, fY, fWidth - (m_bExteriorScrollbars ? 0.0f : m_fScrollbarSize), fHeight, m_vsText, v2Offset, m_hFontMap, &m_vFadeOutlineColor, iFlags, CVec2ui(uint(-1), uint(-1)), CVec2ui(uint(-1), uint(-1)));
        Draw2D.String(fX - m_iOutlineOffset, fY, fWidth - (m_bExteriorScrollbars ? 0.0f : m_fScrollbarSize), fHeight, m_vsText, v2Offset, m_hFontMap, &m_vFadeOutlineColor, iFlags, CVec2ui(uint(-1), uint(-1)), CVec2ui(uint(-1), uint(-1)));
        Draw2D.String(fX, fY + m_iOutlineOffset, fWidth - (m_bExteriorScrollbars ? 0.0f : m_fScrollbarSize), fHeight, m_vsText, v2Offset, m_hFontMap, &m_vFadeOutlineColor, iFlags, CVec2ui(uint(-1), uint(-1)), CVec2ui(uint(-1), uint(-1)));
        Draw2D.String(fX, fY - m_iOutlineOffset, fWidth - (m_bExteriorScrollbars ? 0.0f : m_fScrollbarSize), fHeight, m_vsText, v2Offset, m_hFontMap, &m_vFadeOutlineColor, iFlags, CVec2ui(uint(-1), uint(-1)), CVec2ui(uint(-1), uint(-1)));
    }

    iFlags &= ~DRAW_STRING_NOCOLORCODES;

    Draw2D.String(fX, fY, fWidth - (m_bExteriorScrollbars ? 0.0f : m_fScrollbarSize), fHeight, m_vsText, v2Offset, m_hFontMap, &m_vFadeColor, iFlags, v2SelStart, v2SelEnd);

    // Render input position
    // render insertion point
    if (m_pInterface->GetActiveWidget() == this)
    {
        if (m_bEditable && (int((Host.GetSystemTime() / 1000.0f) * 8.0f) & 0x01) && m_vInputPos[Y] >= 0 && m_vInputPos[Y] < m_vsText.size())
        {
            Draw2D.SetColor(GetFadedColor(m_v4TextColor, fFade));
            Draw2D.Rect(fX + m_pFontMap->GetStringWidth(m_vsText[m_vInputPos[Y]].substr(m_vStart[X], m_vInputPos[X] - m_vStart[X])), fY + (m_pFontMap->GetMaxHeight() - 2.0f) + ((m_vInputPos[Y] - m_vStart[Y]) * m_pFontMap->GetMaxHeight()), m_pFontMap->GetCharMapInfo(_T(' '))->m_fAdvance, 2);
        }
    }
}


/*====================
  CTextBuffer::Frame
  ====================*/
void    CTextBuffer::Frame(uint uiFrameLength, bool bProcessFrame)
{
    IWidget::Frame(uiFrameLength, bProcessFrame);

    for (uint uiVecPos(0); uiVecPos < m_viFadeTime.size(); ++uiVecPos)
    {
        m_vFadeColor[uiVecPos] = GetFadedColor(m_v4TextColor, GetCurrentFade());
        m_vFadeShadowColor[uiVecPos] = GetFadedColor(m_v4ShadowColor, GetCurrentFade());
        m_vFadeOutlineColor[uiVecPos] = GetFadedColor(m_v4OutlineColor, GetCurrentFade());
    }

    if (!bProcessFrame || !HasFlags(WFLAG_VISIBLE) || !HasFlags(WFLAG_ENABLED) || m_iFadeLength == -1)
        return;

    for (uint uiVecPos(0); uiVecPos < m_viFadeTime.size(); ++uiVecPos)
    {
        if (m_viFadeTime[uiVecPos] <= Host.GetTime())
        {
            //If the item is completely faded, remove it from the string vector
            m_viFadeTime.erase(m_viFadeTime.begin() + uiVecPos);
            m_vFadeColor.erase(m_vFadeColor.begin() + uiVecPos);
            m_vFadeShadowColor.erase(m_vFadeShadowColor.begin() + uiVecPos);
            m_vFadeOutlineColor.erase(m_vFadeOutlineColor.begin() + uiVecPos);
            m_vsText.erase(m_vsText.begin() + uiVecPos);

            UpdateScrollbars();

            //Move to the previous value, so that we won't skip over the
            //one that took the place of the current value with the erase.
            --uiVecPos;
            continue;
        }

        if (m_viFadeTime[uiVecPos] - Host.GetTime() < 2000)
        {
            m_vFadeColor[uiVecPos][A] *= (m_viFadeTime[uiVecPos] - Host.GetTime()) / 2000.0f;
            m_vFadeShadowColor[uiVecPos][A] *= (m_viFadeTime[uiVecPos] - Host.GetTime()) / 2000.0f;
            m_vFadeOutlineColor[uiVecPos][A] *= (m_viFadeTime[uiVecPos] - Host.GetTime()) / 2000.0f;
        }
    }
}


/*====================
  CTextBuffer::WrapAddString
  ====================*/
uint    CTextBuffer::WrapAddString(const tstring &sOriginalStr, uint uiInsertTime, tstring sStartingColorCode, uint uiInsertPos)
{
    tstring sStr;
    tstring sNextLine;
    size_t zNewLinePos;
    uint uiLastInsert(uiInsertPos);

    //Make sure we include the color code
    sStr = sStartingColorCode + sOriginalStr;

    if (uiLastInsert == uint(-1))
        uiLastInsert = INT_SIZE(m_vsText.size());

    if ((zNewLinePos = sStr.find(TLINEBREAK)) != tstring::npos)
    {
        sNextLine = sStr.substr(zNewLinePos + TLINEBREAK.length(), sStr.length() - (zNewLinePos + TLINEBREAK.length()));
        sStr = sStr.substr(0, zNewLinePos);
    }

    if (m_bWrap)
    {
        // Get the width of the area
        float fWidth(m_recArea.GetWidth() - (m_bExteriorScrollbars ? 0.0f : m_fScrollbarSize));

        // Do a simple worst case check to quickly reject some strings and check for short strings that will fit
        if (m_pFontMap->GetMaxAdvance() * sStr.length() < fWidth || sStr.length() < 2 || m_pFontMap->GetStringWidth(sStr) < fWidth)
        {
            m_viFadeTime.insert(m_viFadeTime.begin() + uiLastInsert, uiInsertTime + m_iFadeLength);
            m_vFadeColor.insert(m_vFadeColor.begin() + uiLastInsert, m_v4TextColor);
            m_vFadeShadowColor.insert(m_vFadeShadowColor.begin() + uiLastInsert, m_v4ShadowColor);
            m_vFadeOutlineColor.insert(m_vFadeOutlineColor.begin() + uiLastInsert, m_v4OutlineColor);
            m_vsText.insert(m_vsText.begin() + uiLastInsert, sStr);
            return uiLastInsert;
        }

        // Search backwards from the character that would overflow in a worst case
        // scenario (all characters are maximum width) for the first white space
        // character, to avoid some unnecesary comparisons
        size_t zStartSearch(1);

        zStartSearch = INT_FLOOR(fWidth / m_pFontMap->GetMaxAdvance());

        // If the widget is too small to print 1 character, bail out
        if (zStartSearch == 0)
            return uiLastInsert;

        for ( ; zStartSearch != 0; --zStartSearch)
        {
            if (IsTokenSeparator(sStr[zStartSearch]) && !IsTokenSeparator(sStr[zStartSearch - 1]))
                break;
        }

        // Find a good place to break
        size_t zBreakPos(0);
        size_t zOverflowPos(0);
        if (zStartSearch > 0)
        {
            float fCurrentWidth(m_pFontMap->GetStringWidth(sStr.substr(0, zStartSearch)));

            for (size_t z(zStartSearch); z < sStr.length(); ++z)
            {
                if (IsTokenSeparator(sStr[z]) && !IsTokenSeparator(sStr[z - 1]))
                    zBreakPos = z + 1;

                fCurrentWidth += m_pFontMap->GetCharMapInfo(sStr[z])->m_fAdvance;
                if (fCurrentWidth >= fWidth)
                {
                    zOverflowPos = z - 1;
                    break;
                }
            }
        }

        if (zBreakPos == 0)
        {
            // Didn't find any white space to break on, so just split mid-token
            // Pass the last found color code as well, so the color carries over

            // If the overflow pos is 0, the widget is too small, return to avoid a stack overflow
            if (zOverflowPos == 0)
                return uiLastInsert;

            m_viFadeTime.insert(m_viFadeTime.begin() + uiLastInsert, uiInsertTime + m_iFadeLength);
            m_vFadeColor.insert(m_vFadeColor.begin() + uiLastInsert, m_v4TextColor);
            m_vFadeShadowColor.insert(m_vFadeShadowColor.begin() + uiLastInsert, m_v4ShadowColor);
            m_vFadeOutlineColor.insert(m_vFadeOutlineColor.begin() + uiLastInsert, m_v4OutlineColor);
            m_vsText.insert(m_vsText.begin() + uiLastInsert, sStartingColorCode + sStr.substr(0, zOverflowPos));
            uiLastInsert = WrapAddString(sStr.substr(zOverflowPos), uiInsertTime, GetLastColorCode(sStr.substr(0, zOverflowPos)), uiLastInsert + 1);

            // Add any text after a newline that was encountered
            if (!sNextLine.empty())
                uiLastInsert = WrapAddString(sNextLine, uiInsertTime, _T(""), uiLastInsert + 1);
        }
        else
        {
            // Break in the middle of some white space
            // Pass the last found color code as well, so the color carries over
            m_viFadeTime.insert(m_viFadeTime.begin() + uiLastInsert, uiInsertTime + m_iFadeLength);
            m_vFadeColor.insert(m_vFadeColor.begin() + uiLastInsert, m_v4TextColor);
            m_vFadeShadowColor.insert(m_vFadeShadowColor.begin() + uiLastInsert, m_v4ShadowColor);
            m_vFadeOutlineColor.insert(m_vFadeOutlineColor.begin() + uiLastInsert, m_v4OutlineColor);
            m_vsText.insert(m_vsText.begin() + uiLastInsert, sStr.substr(0, zBreakPos));
            uiLastInsert = WrapAddString(sStr.substr(zBreakPos), uiInsertTime, GetLastColorCode(sStr.substr(0, zBreakPos)), uiLastInsert + 1);

            // Add any text after a newline that was encountered
            if (!sNextLine.empty())
                uiLastInsert = WrapAddString(sNextLine, uiInsertTime, _T(""), uiLastInsert + 1);
        }
    }
    else
    {
        m_viFadeTime.insert(m_viFadeTime.begin() + uiLastInsert, uiInsertTime + m_iFadeLength);
        m_vFadeColor.insert(m_vFadeColor.begin() + uiLastInsert, m_v4TextColor);
        m_vFadeShadowColor.insert(m_vFadeShadowColor.begin() + uiLastInsert, m_v4ShadowColor);
        m_vFadeOutlineColor.insert(m_vFadeOutlineColor.begin() + uiLastInsert, m_v4OutlineColor);
        m_vsText.insert(m_vsText.begin() + uiLastInsert, sStr);

        // Add any text after a newline that was encountered
        if (!sNextLine.empty())
            uiLastInsert = WrapAddString(sNextLine, uiInsertTime, TSNULL, uiLastInsert + 1);
    }

    return uiLastInsert;
}


/*====================
  CTextBuffer::ScrollUp
  ====================*/
void    CTextBuffer::ScrollUp()
{
    if (Input.IsShiftDown() && m_vSelStart == CVec2ui(uint(-1), uint(-1)) && m_vInputPos[Y] != 0)
        m_vSelStart = m_vInputPos;
    else if (!Input.IsShiftDown())
        m_vSelStart = CVec2ui(uint(-1), uint(-1));

    if (m_vInputPos[Y] <= 0)
        return;

    m_vInputPos[Y]--;

    if (m_vInputPos[Y] < m_vStart[Y])
        m_vStart[Y] = m_vInputPos[Y];

    if (m_vInputPos[X] > INT_SIZE(m_vsText[m_vInputPos[Y]].length()))
        m_vInputPos[X] = INT_SIZE(m_vsText[m_vInputPos[Y]].length());
}


/*====================
  CTextBuffer::ScrollDown
  ====================*/
void    CTextBuffer::ScrollDown()
{
    if (Input.IsShiftDown() && m_vSelStart == CVec2ui(uint(-1), uint(-1)) && m_vInputPos[Y] < m_vsText.size() - 1)
        m_vSelStart = m_vInputPos;
    else if (!Input.IsShiftDown())
        m_vSelStart = CVec2ui(uint(-1), uint(-1));

    if (m_vInputPos[Y] >= m_vsText.size() - 1)
        return;

    m_vInputPos[Y]++;

    float fHeight(0.0f);
    float fStep(m_pFontMap->GetMaxHeight());
    for (uint i(m_vInputPos[Y]); i >= m_vStart[Y] && i != uint(-1); i--)
    {
        fHeight += fStep;

        if (fHeight > GetHeight())
        {
            m_vStart[Y] = i + 1;
            return;
        }
    }

    if (m_vInputPos[X] > INT_SIZE(m_vsText[m_vInputPos[Y]].length()))
        m_vInputPos[X] = INT_SIZE(m_vsText[m_vInputPos[Y]].length());

    if (m_vStart[X] > m_vInputPos[X])
        m_vStart[X] = m_vInputPos[X];
}


/*====================
  CTextBuffer::SetText
  ====================*/
void    CTextBuffer::SetText(const tstring &sStr)   
{
    if (m_pFontMap == NULL)
        return;

    m_viFadeTime.clear();
    m_vFadeColor.clear();
    m_vFadeShadowColor.clear();
    m_vFadeOutlineColor.clear();
    m_vsText.clear();

    m_pScrollbar->SetMaxValue(0.00001f);
    m_pScrollbar->SetValue(0.00001f);

    m_pScrollbar->Disable();

    if (!m_bScrollbarPlaceholder)
        m_pScrollbar->Hide();
    else
        m_pScrollbar->Show();

    if (!sStr.empty())
        AddText(sStr, Host.GetTime());
    else
        AddText(TSNULL, Host.GetTime());

    m_vInputPos = CVec2ui(0,0);
    m_vStart = CVec2ui(0,0);
    m_vSelStart = CVec2ui(uint(-1),uint(-1));
}


/*====================
  CTextBuffer::UpdateScrollbars
  ====================*/
void    CTextBuffer::UpdateScrollbars()
{
    bool bAtBottom(false);

    if (m_bUseScrollbar)
    {
        if (INT_ROUND(m_pScrollbar->GetValueFloat()) == INT_ROUND(m_pScrollbar->GetMaxValue()))
            bAtBottom = true;

        float fHeight(m_vsText.size() * (m_pFontMap == NULL ? 0.0f : m_pFontMap->GetMaxHeight()));
        //Increase the range of the scrollbar and scroll as appropriate
        if (fHeight > GetHeight())
        {
            m_pScrollbar->SetMaxValue(ceil(((m_vsText.size() * m_pFontMap->GetMaxHeight()) - GetHeight()) / m_pFontMap->GetMaxHeight()));

            if (!m_pScrollbar->HasFlags(WFLAG_VISIBLE) || m_bScrollbarPlaceholder)
            {
                m_pScrollbar->Show();
                m_pScrollbar->Enable();

                if (m_bAnchorBottom)
                    m_pScrollbar->SetValue(m_pScrollbar->GetMaxValue());
                else
                    m_pScrollbar->SetValue(0.0f);
            }
        }
        else
        {
            //If we don't set max value to something other
            //than 0, we will run into a divide by 0 issue,
            //so just set it very close to 0.
            m_pScrollbar->SetMaxValue(0.00001f);
            m_pScrollbar->SetValue(0.00001f);

            m_pScrollbar->Disable();

            if (!m_bScrollbarPlaceholder)
                m_pScrollbar->Hide();
            else
                m_pScrollbar->Show();
        }

        //Only scroll if we're bottom anchored and scrolled to the bottom
        if (m_bAnchorBottom && bAtBottom)
            m_pScrollbar->SetValue(m_pScrollbar->GetMaxValue());
    }
    else
        m_pScrollbar->Hide();
}


/*====================
  CTextBuffer::AddText
  ====================*/
void    CTextBuffer::AddText(const tstring &sStr, uint uiInsertTime)
{ 
    if (m_pFontMap == NULL)
        return;

    WrapAddString(sStr, uiInsertTime);

    //Erase elements to make sure we don't go over our line limit
    while (m_vsText.size() > m_iMaxLines && m_iMaxLines != uint(-1))
    {
        m_viFadeTime.erase(m_viFadeTime.begin());
        m_vFadeColor.erase(m_vFadeColor.begin());
        m_vFadeShadowColor.erase(m_vFadeShadowColor.begin());
        m_vFadeOutlineColor.erase(m_vFadeOutlineColor.begin());
        m_vsText.erase(m_vsText.begin());
        m_pScrollbar->SetMaxValue(m_pScrollbar->GetMaxValue() - 1);

        if (m_pScrollbar->GetValueFloat() > m_pScrollbar->GetMaxValue())
            m_pScrollbar->SetValue(m_pScrollbar->GetMaxValue());
    }

    UpdateScrollbars();

    DO_EVENT(WEVENT_CHANGE)
}


/*====================
  CTextBuffer::SetFont
  ====================*/
void    CTextBuffer::SetFont(const tstring &sFont)
{
    m_hFontMap = g_ResourceManager.LookUpName(sFont, RES_FONTMAP); 
    m_pFontMap = g_ResourceManager.GetFontMap(m_hFontMap);
}


/*====================
  CTextBuffer::MouseDown
  ====================*/
void    CTextBuffer::MouseDown(EButton button, const CVec2f &v2CursorPos)
{
    if (button == BUTTON_WHEELUP)
        m_pScrollbar->MinButtonCommand();
    else if (button == BUTTON_WHEELDOWN)
        m_pScrollbar->MaxButtonCommand();
    if (button != BUTTON_MOUSEL || !Contains(v2CursorPos))
        return;

    DO_EVENT(WEVENT_CLICK)

    if (m_vsText.empty() || !m_bEditable)
        return;

    if (Input.IsShiftDown() && m_vSelStart == CVec2ui(uint(-1),uint(-1)))
        m_vSelStart = m_vInputPos;
    else if (!Input.IsShiftDown())
        m_vSelStart = CVec2ui(uint(-1),uint(-1));

    float fPos(GetY());
    uint uiRow(m_vStart[Y]);
    uint uiCol(m_vStart[X]);

    while (fPos < v2CursorPos[Y])
    {
        fPos += m_pFontMap->GetMaxHeight();
        uiRow++;
    }

    if (uiRow > 0)
        uiRow--;

    if (uiRow >= m_vsText.size() && m_vsText.size() != 0)
        uiRow = INT_SIZE(m_vsText.size()) - 1;

    fPos = GetX();

    while (fPos < v2CursorPos[X] && uiCol < m_vsText[uiRow].length())
    {
        fPos += m_pFontMap->GetCharMapInfo(m_vsText[uiRow][uiCol])->m_fAdvance;
        uiCol++;
    }

    if (fPos < v2CursorPos[X])
        uiCol = INT_SIZE(m_vsText[uiRow].length());
    else if (uiCol > 0)
        uiCol--;

    m_vInputPos = CVec2ui(uiCol, uiRow);

    if (m_vStart[X] > m_vInputPos[X])
    {
        m_vStart[X] = 0;
        fPos = 0.0f;

        for (uint i(m_vInputPos[X]); i >= m_vStart[X] && i != uint(-1); i--)
        {
            fPos += m_pFontMap->GetCharMapInfo(m_vsText[m_vInputPos[Y]][i])->m_fAdvance;

            if (fPos > GetWidth())
            {
                m_vStart[X] = i + 1;
                break;
            }
        }
    }

    if (m_vStart[Y] > m_vInputPos[Y])
    {
        m_vStart[Y] = 0;
        fPos = 0.0f;
        
        float fStep(m_pFontMap->GetMaxHeight());

        for (uint i(m_vInputPos[Y]); i >= m_vStart[Y] && i != uint(-1); i--)
        {
            fPos += fStep;

            if (fPos > GetHeight())
            {
                m_vStart[Y] = i + 1;
                break;
            }
        }
    }

    if (m_vInputPos == m_vSelStart)
        m_vSelStart = CVec2ui(uint(-1),uint(-1));
}


/*====================
  CTextBuffer::AddTextAtPos
  ====================*/
CVec2ui CTextBuffer::AddTextAtPos(const tstring &sStr, CVec2ui v2Pos)
{
    tstring sStrCopy(sStr);
    size_t zPos;(sStrCopy.find(TLINEBREAK));
    CVec2ui v2LastPos;

    NormalizeLineBreaks(sStrCopy);
    zPos = sStrCopy.find(TLINEBREAK);

    if (zPos == tstring::npos)
    {
        m_vsText[v2Pos[Y]].insert(v2Pos[X], sStrCopy);
        v2LastPos[Y] = v2Pos[Y];
        v2LastPos[X] = v2Pos[X] + sStrCopy.length();
    }
    else
    {
        m_vsText[v2Pos[Y]].insert(v2Pos[X], sStrCopy.substr(0, zPos));
        sStrCopy.erase(0, zPos + TLINEBREAK.length());
        v2LastPos[Y] = WrapAddString(sStrCopy, Host.GetTime(), GetLastColorCode(m_vsText[v2Pos[Y]]), v2Pos[Y] + 1);
        v2LastPos[X] = INT_SIZE(m_vsText[v2LastPos[Y]].length());
    }

    //Erase elements to make sure we don't go over our line limit
    while (m_vsText.size() > m_iMaxLines && m_iMaxLines != uint(-1))
    {
        m_viFadeTime.erase(m_viFadeTime.begin());
        m_vFadeColor.erase(m_vFadeColor.begin());
        m_vFadeShadowColor.erase(m_vFadeShadowColor.begin());
        m_vFadeOutlineColor.erase(m_vFadeOutlineColor.begin());
        m_vsText.erase(m_vsText.begin());
        m_pScrollbar->SetMaxValue(m_pScrollbar->GetMaxValue() - 1);

        if (m_pScrollbar->GetValueFloat() > m_pScrollbar->GetMaxValue())
            m_pScrollbar->SetValue(m_pScrollbar->GetMaxValue());
    }

    UpdateScrollbars();

    return v2LastPos;
}

/*====================
  CTextBuffer::ClearText
  ====================*/
void    CTextBuffer::ClearText()
{
    m_viFadeTime.clear();
    m_vFadeColor.clear();
    m_vFadeShadowColor.clear();
    m_vFadeOutlineColor.clear();
    m_vsText.clear();
    
    m_pScrollbar->SetMaxValue(0.00001f);
    m_pScrollbar->SetValue(0.00001f);

    m_pScrollbar->Disable();

    if (!m_bScrollbarPlaceholder)
        m_pScrollbar->Hide();
    else
        m_pScrollbar->Show();

    m_vInputPos = CVec2ui(0,0);
    m_vStart = CVec2ui(0,0);
    m_vSelStart = CVec2ui(uint(-1),uint(-1));

    AddText(TSNULL, Host.GetTime());
}


/*====================
  CTextBuffer::VerticalScrollbarChange
  ====================*/
void    CTextBuffer::VerticalScrollbarChange(float fNewValue)
{
    if (m_bAnchorBottom)
        m_vStart[Y] = INT_ROUND(m_pScrollbar->GetMaxValue() - fNewValue);
    else
        m_vStart[Y] = INT_ROUND(fNewValue);
}


/*====================
  CTextBuffer::GetCopyString
  ====================*/
tstring CTextBuffer::GetCopyString()
{
    if (m_vSelStart == CVec2ui(uint(-1),uint(-1)))
        return _T("");

    CVec2ui vMin;
    CVec2ui vMax;
    tstring sCopy;

    if (m_vSelStart[Y] < m_vInputPos[Y])
    {
        vMin = m_vSelStart;
        vMax = m_vInputPos;
    }
    else if (m_vSelStart[Y] > m_vInputPos[Y])
    {
        vMin = m_vInputPos;
        vMax = m_vSelStart;
    }
    else
    {
        vMin = CVec2ui(MIN(m_vInputPos[X], m_vSelStart[X]), m_vInputPos[Y]);
        vMax = CVec2ui(MAX(m_vInputPos[X], m_vSelStart[X]), m_vInputPos[Y]);
    }

    for (uint i(vMin[Y]); i <= vMax[Y]; i++)
    {
        size_t zStart(0);
        size_t zEnd(m_vsText[i].length());

        if (i == vMin[Y])
            zStart = vMin[X];

        if (i == vMax[Y])
            zEnd = vMax[X];

        if (i != vMin[Y])
            sCopy += TLINEBREAK;

        sCopy += m_vsText[i].substr(zStart, zEnd - zStart);
    }

    NormalizeLineBreaks(sCopy);

    return sCopy;
}

/*====================
  CTextBuffer::PasteString
  ====================*/
void    CTextBuffer::PasteString(const tstring &sString)
{
    if (!m_bEditable)
        return;

    if (m_vSelStart != CVec2ui(uint(-1),uint(-1)))
    {
        CVec2ui vMin;
        CVec2ui vMax;

        if (m_vSelStart[Y] < m_vInputPos[Y])
        {
            vMin = m_vSelStart;
            vMax = m_vInputPos;
        }
        else if (m_vSelStart[Y] > m_vInputPos[Y])
        {
            vMin = m_vInputPos;
            vMax = m_vSelStart;
        }
        else
        {
            vMin = CVec2ui(MIN(m_vInputPos[X], m_vSelStart[X]), m_vInputPos[Y]);
            vMax = CVec2ui(MAX(m_vInputPos[X], m_vSelStart[X]), m_vInputPos[Y]);
        }

        for (uint i(vMin[Y]); i <= vMax[Y]; i++)
        {
            size_t zStart(0);
            size_t zEnd(m_vsText[i].length());

            if (i == vMin[Y])
                zStart = vMin[X];

            if (i == vMax[Y])
                zEnd = vMax[X];

            m_vsText[i].erase(zStart, zEnd - zStart);

            if (m_vsText[i].empty() && i != vMin[Y])
            {
                m_vsText.erase(m_vsText.begin() + i);
                i--;
                vMax[Y]--;
            }
        }
        
        m_vSelStart = CVec2ui(uint(-1), uint(-1));
        m_vInputPos = vMin;
    }

    m_vInputPos = AddTextAtPos(sString, m_vInputPos);

    if (m_vStart[X] > m_vInputPos[X])
        m_vStart[X] = m_vInputPos[X];

    if (m_vInputPos[Y] < m_vStart[Y])
        m_vStart[Y] = m_vInputPos[Y];

    float fSize(0.0f);
    float fStep(m_pFontMap->GetMaxHeight());

    for (uint i(m_vInputPos[Y]); i >= m_vStart[Y] && i != uint(-1); i--)
    {
        fSize += fStep;

        if (fSize > GetHeight())
        {
            m_vStart[Y] = i + 1;
            break;
        }
    }

    fSize = 0.0f;

    for (uint i(m_vInputPos[X]); i >= m_vStart[X] && i != uint(-1); i--)
    {
        fSize += m_pFontMap->GetCharMapInfo(m_vsText[m_vInputPos[Y]][i])->m_fAdvance;

        if (fSize > GetWidth())
        {
            m_vStart[X] = i + 1;
            break;
        }
    }

    UpdateScrollbars();

    DO_EVENT(WEVENT_CHANGE)
}

/*--------------------
  TextBufferCmd
  --------------------*/
UI_VOID_CMD(TextBufferCmd, 1)
{
    //The way args are passed to us brings all of them
    //improperly tokenized, so we have to do it ourselves.
    //Also, erase the command from the new tsvector so it
    //does not get concatinated into the messages to be added.
    if (!pThis || pThis->GetType() != WIDGET_TEXTBUFFER)
        return;

    tstring sInput(vArgList[0]->Evaluate());
    tsvector vTokens(TokenizeString(sInput, _T(' ')));

    if (vTokens.empty())
        return;

    tstring sCommand(vTokens[0]);

    vTokens.erase(vTokens.begin());
    sInput = ConcatinateArgs(vTokens);

    if (CompareNoCase(sCommand, _T("Add")) == 0)
        static_cast<CTextBuffer *>(pThis)->AddText(sInput, Host.GetTime());
    else if (CompareNoCase(sCommand, _T("Set")) == 0)
        static_cast<CTextBuffer *>(pThis)->SetText(sInput);
    else if (CompareNoCase(sCommand, _T("Clear")) == 0)
        static_cast<CTextBuffer *>(pThis)->ClearText();
}


/*--------------------
  AddBufferText
  --------------------*/
UI_VOID_CMD(AddBufferText, 1)
{
    if (!pThis || pThis->GetType() != WIDGET_TEXTBUFFER)
        return;

    static_cast<CTextBuffer *>(pThis)->AddText(vArgList[0]->Evaluate(), vArgList.size() > 1 ? AtoI(vArgList[1]->Evaluate()) : Host.GetTime());
}


/*--------------------
  SetBufferText
  --------------------*/
UI_VOID_CMD(SetBufferText, 1)
{
    if (!pThis || pThis->GetType() != WIDGET_TEXTBUFFER)
        return;

    static_cast<CTextBuffer *>(pThis)->SetText(vArgList[0]->Evaluate());
}


/*--------------------
  ClearBufferText
  --------------------*/
UI_VOID_CMD(ClearBufferText, 0)
{
    if (!pThis || pThis->GetType() != WIDGET_TEXTBUFFER)
        return;

    static_cast<CTextBuffer *>(pThis)->ClearText();
}


/*--------------------
  ReloadFile
  --------------------*/
UI_VOID_CMD(ReloadFile, 0)
{
    if (!pThis || pThis->GetType() != WIDGET_TEXTBUFFER)
        return;

    static_cast<CTextBuffer *>(pThis)->ReloadFile();
}
