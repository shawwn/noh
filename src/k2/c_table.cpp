// (C)2007 S2 Games
// c_table.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_table.h"
#include "c_draw2d.h"
#include "c_widgetstyle.h"
#include "c_uicmd.h"
#include "c_uiscript.h"
#include "c_texture.h"
#include "c_interface.h"
#include "c_bitmap.h"
#include "c_uimanager.h"
#include "c_resourcemanager.h"
//=============================================================================

uint g_uiCompareCol;

/*====================
  CompareColByText
  ====================*/
bool CompareColByText(TableRow vsRow1, TableRow vsRow2)
{
    // Always push the header to the top...
    if (vsRow1.bHeader)
        return true;

    if (vsRow2.bHeader)
        return false;

    // If it's not a header, compare by column text
    if (g_uiCompareCol < vsRow1.vsCol.size() && g_uiCompareCol < vsRow2.vsCol.size())
    {
        tstring sString1 = StripColorCodes(vsRow1.vsCol[g_uiCompareCol]);
        tstring sString2 = StripColorCodes(vsRow2.vsCol[g_uiCompareCol]);

        int iCompareResult(CompareNoCase(sString1, sString2));

        return (iCompareResult == -1 ? true : false);
    }

    return false;
}


/*====================
  CompareColByTextReverse
  ====================*/
bool CompareColByTextReverse(TableRow vsRow1, TableRow vsRow2)
{
    // Always push the header to the top...
    if (vsRow1.bHeader)
        return true;

    if (vsRow2.bHeader)
        return false;

    // If it's not a header, compare by column text
    if (g_uiCompareCol < vsRow1.vsCol.size() && g_uiCompareCol < vsRow2.vsCol.size())
    {
        tstring sString1 = vsRow1.vsCol[g_uiCompareCol];
        tstring sString2 = vsRow2.vsCol[g_uiCompareCol];

        int iCompareResult(CompareNoCase(sString1, sString2));

        return (iCompareResult == 1 ? true : false);
    }

    return true;
}


/*====================
  CompareColByValue
  ====================*/
bool CompareColByValue(TableRow vsRow1, TableRow vsRow2)
{
    // Always push the header to the top...
    if (vsRow1.bHeader)
        return true;

    if (vsRow2.bHeader)
        return false;

    // If it's not a header, compare by column value
    if (g_uiCompareCol < vsRow1.vsCol.size() && g_uiCompareCol < vsRow2.vsCol.size())
    {
        float fValue1 = AtoF(StripColorCodes(vsRow1.vsCol[g_uiCompareCol]));
        float fValue2 = AtoF(StripColorCodes(vsRow2.vsCol[g_uiCompareCol]));

        return (fValue1 < fValue2);
    }

    return false;
}


/*====================
  CompareByColValueReverse
  ====================*/
bool CompareColByValueReverse(TableRow vsRow1, TableRow vsRow2)
{
    // Always push the header to the top...
    if (vsRow1.bHeader)
        return true;

    if (vsRow2.bHeader)
        return false;

    // If it's not a header, compare by column value
    if (g_uiCompareCol < vsRow1.vsCol.size() && g_uiCompareCol < vsRow2.vsCol.size())
    {
        float fValue1 = AtoF(StripColorCodes(vsRow1.vsCol[g_uiCompareCol]));
        float fValue2 = AtoF(StripColorCodes(vsRow2.vsCol[g_uiCompareCol]));

        return (fValue1 > fValue2);
    }

    return true;
}


/*====================
  CTable::~CTable
  ====================*/
CTable::~CTable()
{
    SAFE_DELETE(m_pScrollbar);
}

    //MikeG added commands Outline OutlineColor Outlineoffset Shadowoffsetx Shadowoffsety
/*====================
  CTable::CTable
  ====================*/
CTable::CTable(CInterface* pInterface, IWidget* pParent, const CWidgetStyle& style) :
IWidget(pInterface, pParent, WIDGET_TABLE, style),
m_sRowHeight(style.GetProperty(_T("rowheight"), _T("20"))),
m_sColWidth(style.GetProperty(_T("colwidth"), _T("20"))),
m_sDataOffset(style.GetProperty(_T("dataoffset"), _T("1"))),
m_bExpandable(style.GetPropertyBool(_T("expandable"))),
m_v4HorizontalBorderColor(GetColorFromString(style.GetProperty(_T("bordercolor"), _T("silver")))),
m_v4VerticalBorderColor(GetColorFromString(style.GetProperty(_T("bordercolor"), _T("silver")))),
m_v4DataColor(GetColorFromString(style.GetProperty(_T("datacolor"), _T("silver")))),
m_v4AltRowColor(m_v4Color),
m_v4AltDataColor(m_v4DataColor),
m_v4HeadingColor(m_v4Color),
m_v4HeadingDataColor(m_v4DataColor),
m_hFontMap(g_ResourceManager.LookUpName(style.GetProperty(_T("font"), _T("system_medium")), RES_FONTMAP)),
m_bHasHeadings(false),
m_sHeadingID(style.GetProperty(_T("headingid"))),
m_sOnClear(style.GetProperty(_T("onclear"), _T(""))),
m_v2LastCell(CVec2ui(uint(-1), uint(-1))),
m_uiDoubleClickTime(style.GetPropertyInt(_T("doubleclicktime"), 400)),
m_uiLastClickTime(-1),
m_v2LastClickedCell(CVec2ui(uint(-1), uint(-1))),
m_iDrawFlags(0),
m_uiSortCol(style.GetPropertyInt(_T("sortcol"), -1)),
m_bSortReverse(false),
m_bSortByValue(false),
m_uiLastUniqueID(-1),
m_bShadow(style.GetPropertyBool(_T("shadow"), false)),
m_v4ShadowColor(GetColorFromString(style.GetProperty(_T("shadowcolor"), _T("#000000")))),
m_fShadowOffset(style.GetPropertyFloat(_T("shadowoffset"), 1.0f)),
m_fShadowOffsetX(style.GetPropertyFloat(_T("shadowoffsetx"), 0.0f)),
m_fShadowOffsetY(style.GetPropertyFloat(_T("shadowoffsety"), 0.0f)),
m_bOutline(style.GetPropertyBool(_T("outline"), false)),
m_v4OutlineColor(GetColorFromString(style.GetProperty(_T("outlinecolor"), _T("#000000")))),
m_iOutlineOffset(style.GetPropertyFloat(_T("outlineoffset"), 1)),
m_bUseScrollbar(style.GetPropertyBool(_T("usescrollbar"), false)),
m_fScrollbarSize(GetSizeFromString(style.GetProperty(_T("scrollbarsize"), _T("16")), pParent->GetWidth(), pParent->GetHeight())),
m_pScrollbar(NULL),
m_bAutoSort(style.GetPropertyBool(_T("autosort"), true))
{

    if (m_fShadowOffsetX==0.0f && m_fShadowOffsetY==0.0f)
    {
        m_fShadowOffsetX=m_fShadowOffset;
        m_fShadowOffsetY=m_fShadowOffset;
    }

    SetFlags(WFLAG_INTERACTIVE);
    SetFlagsRecursive(WFLAG_PROCESS_CURSOR);

    // Text Alignment
    const tstring sLeft(_T("left"));
    const tstring &sTextAlign(style.GetProperty(_T("textalign"), sLeft));
    if (sTextAlign == _T("center"))
        m_iDrawFlags |= DRAW_STRING_CENTER;
    else if (sTextAlign == _T("right"))
        m_iDrawFlags |= DRAW_STRING_RIGHT;

    const tstring sTop(_T("top"));
    const tstring &sTextVAlign(style.GetProperty(_T("textvalign"), sTop));
    if (sTextVAlign == _T("center"))
        m_iDrawFlags |= DRAW_STRING_VCENTER;
    else if (sTextVAlign == _T("bottom"))
        m_iDrawFlags |= DRAW_STRING_BOTTOM;

    // Columns
    const tstring &sCols(style.GetProperty(_T("cols")));
    tsvector vCols(TokenizeString(sCols, _T(',')));
    if (vCols.size() == 1)
    {
        int iCount(AtoI(sCols));
        for (int i(0); i < iCount; ++i)
            m_vsColSize.push_back(m_sColWidth);
    }
    else
    {
        m_vsColSize = vCols;
    }

    m_uiBaseCols = INT_SIZE(m_vsColSize.size());

    // Rows
    const tstring &sRows(style.GetProperty(_T("rows")));
    tsvector vRows(TokenizeString(sRows, _T(',')));
    if (vRows.size() == 1)
    {
        int iCount(AtoI(sRows));
        for (int i(0); i < iCount; ++i)
            m_vsRowSize.push_back(m_sRowHeight);
    }
    else
    {
        m_vsRowSize = vRows;
    }
    m_uiBaseRows = INT_SIZE(m_vsRowSize.size());

    if (m_bUseScrollbar)
        {
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
        styleCopy.RemoveProperty(_T("watch"));
        styleCopy.RemoveProperty(_T("ontrigger"));

        for (int i(0); i < 10; ++i)
        {
            styleCopy.RemoveProperty(_T("watch") + XtoA(i));
            styleCopy.RemoveProperty(_T("ontrigger") + XtoA(i));
        }

        styleCopy.RemoveProperty(_T("valign"));
        styleCopy.SetProperty(_T("align"), _T("right"));

        styleCopy.SetProperty(_T("visible"), false);
        styleCopy.SetProperty(_T("color"), _T("white"));
        styleCopy.SetProperty(_T("handlecolor"), _T("white"));
        styleCopy.SetProperty(_T("slotcolor"), _T("white"));
        styleCopy.SetProperty(_T("texture"), style.GetProperty(_T("scrolltexture"), _T("/ui/elements/standardscroll.tga")));
        styleCopy.SetProperty(_T("step"), style.GetPropertyFloat(_T("scrollstep"), 1.0f));

        styleCopy.SetProperty(_T("x"), 0.0f);
        styleCopy.SetProperty(_T("y"), 0.0f);
        styleCopy.SetProperty(_T("width"), m_fScrollbarSize);
        styleCopy.SetProperty(_T("height"), _T("100%"));
        styleCopy.SetProperty(_T("vertical"), true);

        if (!GetName().empty())
            styleCopy.SetProperty(_T("name"), GetName() + _T("_scroll"));

        m_pScrollbar = K2_NEW(ctx_Widgets,  CTableScrollbar)(m_pInterface, this, styleCopy);

        m_pScrollbar->SetValue(0.0f);

        AddChild(m_pScrollbar);
    }

    // Colors
    if (style.HasProperty(_T("headingcolor")))
        m_v4HeadingColor = GetColorFromString(style.GetProperty(_T("headingcolor")));
    if (style.HasProperty(_T("headingdatacolor")))
        m_v4HeadingDataColor = GetColorFromString(style.GetProperty(_T("headingdatacolor")));
    if (style.HasProperty(_T("altrowcolor")))
        m_v4AltRowColor = GetColorFromString(style.GetProperty(_T("altrowcolor")));
    if (style.HasProperty(_T("altdatacolor")))
        m_v4AltDataColor = GetColorFromString(style.GetProperty(_T("altdatacolor")));
    if (style.HasProperty(_T("horizontalbordercolor")))
        m_v4HorizontalBorderColor = GetColorFromString(style.GetProperty(_T("horizontalbordercolor")));
    if (style.HasProperty(_T("verticalbordercolor")))
        m_v4VerticalBorderColor = GetColorFromString(style.GetProperty(_T("verticalbordercolor")));

    m_sOrigHeight = GetBaseHeight();
    m_sOrigWidth = GetBaseWidth();

    // Header
    if (style.HasProperty(_T("headings")))
    {
        m_bHasHeadings = true;
        ++m_uiBaseRows;
        m_vsRowSize.push_back(m_sRowHeight);
        const tstring &sHeadings(style.GetProperty(_T("headings")));
        m_vHeadings = TokenizeString(sHeadings, _T(','));
        for (tsvector_it it(m_vHeadings.begin()); it != m_vHeadings.end(); ++it)
            *it = UIManager.Translate(*it);
        Data(m_sHeadingID, m_vHeadings, true);
    }

    while (m_vRowColors.size() < m_uiBaseRows)
    {
        if (m_vRowColors.size() % 2 == 0 && m_bHasHeadings)
        {
            m_vRowColors.push_back(m_v4Color);
            m_vRowDataColors.push_back(m_v4DataColor);
        }
        else if (m_vRowColors.size() % 2 == 1 && !m_bHasHeadings)
        {
            m_vRowColors.push_back(m_v4Color);
            m_vRowDataColors.push_back(m_v4DataColor);
        }
        else
        {
            m_vRowColors.push_back(m_v4AltRowColor);
            m_vRowDataColors.push_back(m_v4AltDataColor);
        }
    }

    RecalculateSize();
}


/*====================
  CTable::UpdateScrollbar
  ====================*/
void    CTable::UpdateScrollbar()
{
    if (m_pScrollbar == NULL || !m_bUseScrollbar)
        return;

    uint uiMaxValue(INT_SIZE(m_vsRowSize.size()) + 1);

    if (uiMaxValue > m_uiBaseRows + 1)
        uiMaxValue -= m_uiBaseRows;
    else
        uiMaxValue = 1;

    m_pScrollbar->SetMaxValue(uiMaxValue);

    if (m_pScrollbar->GetValueFloat() > m_pScrollbar->GetMaxValue())
        m_pScrollbar->SetValue(0.0f);

    m_pScrollbar->Show();
}


/*====================
  CTable::SortTable
  ====================*/
void    CTable::SortTable()
{
    tablevector vNewIDs;
    tablevector::iterator datait;
    tablevector::iterator idit;

    g_uiCompareCol = m_uiSortCol;

    // Sort the data...
    if (!m_bSortByValue)
    {
        if (!m_bSortReverse)
            sort(m_vRow.begin(), m_vRow.end(), CompareColByText);
        else
            sort(m_vRow.begin(), m_vRow.end(), CompareColByTextReverse);
    }
    else
    {
        if (!m_bSortReverse)
            sort(m_vRow.begin(), m_vRow.end(), CompareColByValue);
        else
            sort(m_vRow.begin(), m_vRow.end(), CompareColByValueReverse);
    }

    // Now that we've sorted the data, we need to realign IDs
    for (datait = m_vRow.begin(); datait != m_vRow.end(); datait++)
    {
        for (idit = m_vRowIDs.begin(); idit != m_vRowIDs.end(); idit++)
        {
            if (idit->uiUniqueID == datait->uiUniqueID)
            {
                vNewIDs.push_back(*idit);
                break;
            }
        }
    }

    m_vRowIDs.clear();
    m_vRowIDs = vNewIDs;
}


/*====================
  CTable::RecalculateSize
  ====================*/
void    CTable::RecalculateSize()
{
    float fParentWidth(GetParentWidth());
    float fParentHeight(GetParentHeight());

    float fBaseWidth(0.0f);
    float fOrigWidth(GetSizeFromString(m_sOrigWidth, fParentWidth, fParentHeight));
    float fOrigHeight(GetSizeFromString(m_sOrigHeight, fParentHeight, fParentWidth));

    for (tsvector_it it(m_vsColSize.begin()); it != m_vsColSize.end(); ++it)
        fBaseWidth += GetSizeFromString(*it, fOrigWidth, fOrigHeight);

    SetBaseWidth(XtoA(fBaseWidth));

    float fBaseHeight(0.0f);
    size_t zNumRows(m_vsRowSize.size());
    uint uiNumAdded(0);
    uint uiMaxAdded(-1);

    if (m_bUseScrollbar)
        uiMaxAdded = m_uiBaseRows;

    for (uint uiRow(0); uiRow < zNumRows && uiNumAdded < uiMaxAdded; ++uiRow)
    {
        float fRowHeight = GetSizeFromString(m_vsRowSize[uiRow], fOrigHeight, fOrigWidth);

        if (!m_bUseScrollbar || (uiRow >= m_pScrollbar->GetValueFloat() || (uiRow < m_vRow.size() && m_vRow[uiRow].bHeader)) && fBaseHeight + fRowHeight <= fOrigHeight)
        {
            fBaseHeight += fRowHeight;
            uiNumAdded++;
        }
    }

    SetBaseHeight(XtoA(fBaseHeight));

    IWidget::RecalculateSize();

    UpdateScrollbar();
}


/*====================
  CTable::RenderWidget
  ====================*/
void    CTable::RenderWidget(const CVec2f &vOrigin, float fFade)
{
    if (!HasFlags(WFLAG_VISIBLE))
        return;
    if (HasFlags(WFLAG_NO_DRAW))
        return;

    float fOrigWidth;
    float fOrigHeight;
    
    if (m_pParent != NULL)
    {
        fOrigWidth = GetSizeFromString(m_sOrigWidth, m_pParent->GetWidth(), m_pParent->GetHeight());
        fOrigHeight = GetSizeFromString(m_sOrigHeight, m_pParent->GetHeight(), m_pParent->GetWidth());
    }
    else
    {
        fOrigWidth = GetSizeFromString(m_sOrigWidth, Draw2D.GetScreenW(), Draw2D.GetScreenH());
        fOrigHeight = GetSizeFromString(m_sOrigHeight, Draw2D.GetScreenH(), Draw2D.GetScreenW());
    }

    // Background
    float fRowOffset(0.0f);
    size_t zNumRows(MIN(m_vsRowSize.size(), m_vRowColors.size()));
    uint uiNumDrawn(0);
    uint uiMaxDrawn(-1);

    if (m_bUseScrollbar)
        uiMaxDrawn = m_uiBaseRows;

    for (uint uiRow(0); uiRow < zNumRows && uiNumDrawn < uiMaxDrawn; ++uiRow)
    {
        float fRowHeight = GetSizeFromString(m_vsRowSize[uiRow], fOrigHeight, fOrigWidth);

        if (!m_bUseScrollbar || (uiRow >= m_pScrollbar->GetValueFloat() || m_vRow[uiRow].bHeader) && fRowOffset + fRowHeight <= GetHeight())
        {
            Draw2D.SetColor(GetFadedColor(m_vRowColors[uiRow], fFade));
            Draw2D.Rect(vOrigin.x, vOrigin.y + fRowOffset, GetWidth(), fRowHeight);
            fRowOffset += fRowHeight;
            uiNumDrawn++;
        }
    }

    // Data
    fRowOffset = 0.0f;
    uiNumDrawn = 0;
    zNumRows = MIN(MIN(m_vRow.size(), m_vsRowSize.size()), m_vRowDataColors.size());
    for (uint uiRow(0); uiRow < zNumRows && uiNumDrawn < uiMaxDrawn; ++uiRow)
    {
        CVec4f v4TextColor(GetFadedColor(m_vRowDataColors[uiRow], fFade));
        Draw2D.SetColor(v4TextColor);

        float fRowHeight(GetSizeFromString(m_vsRowSize[uiRow], fOrigHeight, fOrigWidth));

        TableRow &vRowData(m_vRow[uiRow]);

        if (m_bUseScrollbar && (uiRow < m_pScrollbar->GetValueFloat() && !m_vRow[uiRow].bHeader) || fRowOffset + fRowHeight > GetHeight())
            continue;

        tsvector_it itColPos(m_vsColSize.begin());
        float fColPos(0.0f);
        for (
            tsvector_it itColData(vRowData.vsCol.begin());
            itColData != vRowData.vsCol.end() && itColPos != m_vsColSize.end();
            ++itColPos, ++itColData
            )
        {
            float fColWidth = GetSizeFromString(*itColPos, fOrigWidth, fOrigHeight);
            float fOffsetX = GetSizeFromString(m_sDataOffset, fColWidth, fColWidth);
            tstring sData(*itColData);

#if 0
            if (sData.substr(0, 2) == _T("!!"))
            {
                Draw2D.Rect(
                    vOrigin.x + fColPos + fOffsetX,
                    vOrigin.y + fRowOffset,
                    fColWidth - fOffsetX, fRowHeight,
                    g_ResourceManager.Register(K2_NEW(ctx_Widgets,  CTexture)(sData.substr(2), TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE));
            }
            else if (sData.substr(0, 2) == _T("@@"))
            {
                // Same as !!, but keeps image proportions
                float fSize(MIN(fColWidth - fOffsetX, fRowHeight));

                Draw2D.Rect(
                    vOrigin.x + fColPos + fOffsetX + (((fColWidth - fOffsetX) - fSize) / 2.0f),
                    vOrigin.y + fRowOffset + ((fRowHeight - fSize) / 2.0f),
                    fSize, fSize,
                    g_ResourceManager.Register(K2_NEW(ctx_Widgets,  CTexture)(sData.substr(2), TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE));
            }
            else
#endif
            {
                if (m_bShadow)
                {
                    Draw2D.SetColor(GetFadedColor(m_v4ShadowColor, fFade));
                    Draw2D.String(
                        vOrigin.x + fColPos + fOffsetX + m_fShadowOffsetX,
                        vOrigin.y + fRowOffset + m_fShadowOffsetY,
                        fColWidth - fOffsetX, fRowHeight,
                        sData,
                        m_hFontMap, m_iDrawFlags | DRAW_STRING_NOCOLORCODES);
                    Draw2D.SetColor(GetFadedColor(v4TextColor, fFade));
                }
                if (m_bOutline)
                {
                    Draw2D.SetColor(GetFadedColor(m_v4OutlineColor, fFade));
                    Draw2D.String(vOrigin.x + fColPos + fOffsetX + m_iOutlineOffset, vOrigin.y + fRowOffset, fColWidth - fOffsetX, fRowHeight, sData,m_hFontMap, m_iDrawFlags | DRAW_STRING_NOCOLORCODES);
                    Draw2D.String(vOrigin.x + fColPos + fOffsetX - m_iOutlineOffset, vOrigin.y + fRowOffset, fColWidth - fOffsetX, fRowHeight, sData,m_hFontMap, m_iDrawFlags | DRAW_STRING_NOCOLORCODES);
                    Draw2D.String(vOrigin.x + fColPos + fOffsetX, vOrigin.y + fRowOffset + m_iOutlineOffset, fColWidth - fOffsetX, fRowHeight, sData,m_hFontMap, m_iDrawFlags | DRAW_STRING_NOCOLORCODES);
                    Draw2D.String(vOrigin.x + fColPos + fOffsetX, vOrigin.y + fRowOffset - m_iOutlineOffset, fColWidth - fOffsetX, fRowHeight, sData,m_hFontMap, m_iDrawFlags | DRAW_STRING_NOCOLORCODES);
                    Draw2D.SetColor(GetFadedColor(v4TextColor, fFade));
                }

                Draw2D.String(
                    vOrigin.x + fColPos + fOffsetX,
                    vOrigin.y + fRowOffset,
                    fColWidth - fOffsetX, fRowHeight,
                    sData,
                    m_hFontMap, m_iDrawFlags);
            }

            fColPos += fColWidth;
        }

        fRowOffset += fRowHeight;
        uiNumDrawn++;
    }

    // Borders
    float fColPos(0.0f);
    CVec4f v4VerticalBorderColor(GetFadedColor(m_v4VerticalBorderColor, fFade));

    Draw2D.Line(
        CVec2f(vOrigin.x, vOrigin.y),
        CVec2f(vOrigin.x, vOrigin.y + m_recArea.GetHeight()),
        v4VerticalBorderColor, v4VerticalBorderColor);
    for (tsvector_it it(m_vsColSize.begin()); it != m_vsColSize.end(); ++it)
    {
        float fColWidth = GetSizeFromString(*it, fOrigWidth, fOrigHeight);

        Draw2D.Line(
            CVec2f(vOrigin.x + fColPos + fColWidth, vOrigin.y),
            CVec2f(vOrigin.x + fColPos + fColWidth, vOrigin.y + m_recArea.GetHeight()),
            v4VerticalBorderColor, v4VerticalBorderColor);
        fColPos += fColWidth;
    }


    CVec4f v4HorizontalBorderColor(GetFadedColor(m_v4HorizontalBorderColor, fFade));
    fRowOffset = 0.0f;
    uiNumDrawn = 0;
    Draw2D.Line(
        CVec2f(vOrigin.x, vOrigin.y),
        CVec2f(vOrigin.x + m_recArea.GetWidth(), vOrigin.y),
        v4HorizontalBorderColor, v4HorizontalBorderColor);
    zNumRows = MIN(m_vsRowSize.size(), m_vRowColors.size());
    for (uint uiRow(0); uiRow < zNumRows && uiNumDrawn < uiMaxDrawn; ++uiRow)
    {
        float fRowHeight = GetSizeFromString(m_vsRowSize[uiRow], fOrigHeight, fOrigWidth);

        if (!m_bUseScrollbar || (uiRow >= m_pScrollbar->GetValueFloat() || m_vRow[uiRow].bHeader) && fRowOffset + fRowHeight <= GetHeight())
        {
            Draw2D.Line(
                CVec2f(vOrigin.x, vOrigin.y + fRowOffset + fRowHeight),
                CVec2f(vOrigin.x + m_recArea.GetWidth(), vOrigin.y + fRowOffset + fRowHeight),
                v4HorizontalBorderColor, v4HorizontalBorderColor);

            fRowOffset += fRowHeight;
            uiNumDrawn++;
        }
    }
}


/*====================
  CTable::GetCellFromCoord
  ====================*/
CVec2ui CTable::GetCellFromCoord(const CVec2f &v2Pos)
{
    if (!m_recArea.AltContains(v2Pos))
        return (CVec2ui(uint(-1), uint(-1)));

    CVec2ui v2Cell(0, 0);

    float fParentWidth;
    float fParentHeight;

    if (m_pParent != NULL)
    {
        fParentWidth = m_pParent->GetWidth();
        fParentHeight = m_pParent->GetHeight();
    }
    else
    {
        fParentWidth = Draw2D.GetScreenW();
        fParentHeight = Draw2D.GetScreenH();
    }

    float fRowPos(0.0f);
    float fRowOffset(0.0f);
    uint uiNumChecked(0);
    uint uiMaxChecked(-1);

    if (m_bUseScrollbar)
        uiMaxChecked = m_uiBaseRows;

    for (tsvector_it it(m_vsRowSize.begin()); it != m_vsRowSize.end() && uiNumChecked < uiMaxChecked; ++it)
    {
        float fRowHeight(GetSizeFromString(*it, GetSizeFromString(m_sOrigHeight, fParentHeight, fParentWidth), GetSizeFromString(m_sOrigWidth, fParentWidth, fParentHeight)));
        fRowPos += fRowHeight;

        if (!m_bUseScrollbar || (v2Cell.y >= m_pScrollbar->GetValueFloat() || m_vRow[v2Cell.y].bHeader) && fRowOffset + fRowHeight <= GetHeight())
        {
            fRowOffset += fRowHeight;
            if (v2Pos.y - m_recArea.top <= fRowOffset)
                break;
            uiNumChecked++;
        }

        ++v2Cell.y;
    }

    float fColPos(0.0f);
    for (tsvector_it it(m_vsColSize.begin()); it != m_vsColSize.end(); ++it)
    {
        fColPos += GetSizeFromString(*it, GetSizeFromString(m_sOrigWidth, fParentWidth, fParentHeight), GetSizeFromString(m_sOrigHeight, fParentHeight, fParentWidth));
        if (v2Pos.x - m_recArea.left <= fColPos)
            break;
        ++v2Cell.x;
    }

    return v2Cell;
}

/*====================
  CTable::GetCoordFromCell
  ====================*/
CVec2f  CTable::GetCoordFromCell(const CVec2ui &v2Pos)
{
    if (v2Pos[X] > m_vsRowSize.size() || v2Pos[Y] > m_vsRowSize.size())
        return (CVec2f(0.0f, 0.0f));

    CVec2ui v2Cell(0, 0);
    CVec2f v2Final(0.0f, 0.0f);

    float fParentWidth;
    float fParentHeight;

    if (m_pParent != NULL)
    {
        fParentWidth = m_pParent->GetWidth();
        fParentHeight = m_pParent->GetHeight();
    }
    else
    {
        fParentWidth = Draw2D.GetScreenW();
        fParentHeight = Draw2D.GetScreenH();
    }

    float fRowPos(0.0f);
    uint uiNumChecked(0);
    uint uiMaxChecked(-1);

    if (m_bUseScrollbar)
        uiMaxChecked = m_uiBaseRows;

    for (tsvector_it it(m_vsRowSize.begin()); it != m_vsRowSize.end() && uiNumChecked < uiMaxChecked; ++it)
    {
        float fRowHeight(GetSizeFromString(*it, GetSizeFromString(m_sOrigHeight, fParentHeight, fParentWidth), GetSizeFromString(m_sOrigWidth, fParentWidth, fParentHeight)));

        if (!m_bUseScrollbar || (v2Cell.y >= m_pScrollbar->GetValueFloat() || m_vRow[v2Cell.y].bHeader) && v2Final[Y] + fRowHeight <= GetHeight())
        {
            if (v2Cell.y >= v2Pos.y)
                break;
            v2Final[Y] += fRowHeight;
            uiNumChecked++;
        }

        fRowPos += fRowHeight;
        ++v2Cell.y;
    }

    for (tsvector_it it(m_vsColSize.begin()); it != m_vsColSize.end(); ++it)
    {
        if (v2Cell.x >= v2Pos.x)
            break;
        v2Final[X] += GetSizeFromString(*it, GetSizeFromString(m_sOrigWidth, fParentWidth, fParentHeight), GetSizeFromString(m_sOrigHeight, fParentHeight, fParentWidth));
        ++v2Cell.x;
    }

    return v2Final;
}


/*====================
  CTable::MouseDown
  ====================*/
void    CTable::MouseDown(EButton button, const CVec2f &v2CursorPos)
{
    if (button == BUTTON_WHEELUP && m_pScrollbar != NULL)
        m_pScrollbar->MinButtonCommand();
    else if (button == BUTTON_WHEELDOWN && m_pScrollbar != NULL)
        m_pScrollbar->MaxButtonCommand();
    
    if (button == BUTTON_WHEELUP || button == BUTTON_WHEELDOWN || !Contains(v2CursorPos))
        return;

    CVec2ui v2Cell(GetCellFromCoord(v2CursorPos));

    m_uiEventRow = v2Cell.y;
    m_uiEventCol = v2Cell.x;
    m_sEventData.clear();
    m_sEventDataID.clear();

    if (v2Cell.y < m_vRowIDs.size() && v2Cell.x < m_vRowIDs[v2Cell.y].vsCol.size())
        m_sEventDataID = m_vRowIDs[v2Cell.y].vsCol[v2Cell.x];
    if (v2Cell.y < m_vRow.size() && v2Cell.x < m_vRow[v2Cell.y].vsCol.size())
        m_sEventData = m_vRow[v2Cell.y].vsCol[v2Cell.x];

    if (v2Cell == m_v2LastClickedCell && Host.GetTime() - m_uiDoubleClickTime < m_uiLastClickTime)
    {
        m_uiLastClickTime = Host.GetTime();
        DO_EVENT(WEVENT_DOUBLECLICK)
    }
    else
    {
        m_uiLastClickTime = Host.GetTime();
        m_v2LastClickedCell = v2Cell;
        DO_EVENT(WEVENT_CLICK)
    }
}


/*====================
  CTable::GetData
  ====================*/
tstring CTable::GetData(uint uiCol, uint uiRow)
{
    if (uiRow >= INT_SIZE(m_vRow.size()))
        return _T("");

    if (uiCol > INT_SIZE(m_vRow[uiRow].vsCol.size()))
        return _T("");

    return m_vRow[uiRow].vsCol[uiCol];
}


/*====================
  CTable::SetCell
  ====================*/
void    CTable::SetCell(uint uiCol, uint uiRow, const tstring &sValue)
{
    if (uiRow >= INT_SIZE(m_vRow.size()))
    {
        m_vRow.resize(uiRow + 1);
        m_vRowIDs.resize(uiRow + 1);
    }

    if (uiCol >= INT_SIZE(m_vRow[uiRow].vsCol.size()))
    {
        m_vRow[uiRow].vsCol.resize(uiCol + 1);
        m_vRowIDs[uiRow].vsCol.resize(uiCol + 1);
    }

    m_vRow[uiRow].vsCol[uiCol] = sValue;
}


/*====================
  CTable::Data
  ====================*/
void    CTable::Data(const tstring &sID, const tsvector &vData, bool bIsHeader)
{
    TableRow row;

    row.vsCol = tsvector(vData.size(), sID);
    row.uiUniqueID = ++m_uiLastUniqueID;
    row.bHeader = bIsHeader;

    m_vRowIDs.push_back(row);

    row.vsCol = vData;

    m_vRow.push_back(row);

    if (m_bHasHeadings)
    {
        if (bIsHeader)
        {
            m_vRowColors.push_back(m_v4HeadingColor);
            m_vRowDataColors.push_back(m_v4HeadingDataColor);
        }
    }

    bool bAddedRows(false);

    while (m_vsRowSize.size() < m_uiBaseRows)
    {
        m_vsRowSize.push_back(m_sRowHeight);
        bAddedRows = true;
    }

    if (m_bExpandable)
    {
        while (m_vsRowSize.size() < m_vRow.size())
        {
            m_vsRowSize.push_back(m_sRowHeight);
            bAddedRows = true;
        }
    }

    if (bAddedRows)
    {
        while (m_vRowColors.size() < m_vsRowSize.size())
        {
            if (m_vRowColors.size() % 2 == 0 && m_bHasHeadings)
            {
                m_vRowColors.push_back(m_v4Color);
                m_vRowDataColors.push_back(m_v4DataColor);
            }
            else if (m_vRowColors.size() % 2 == 1 && !m_bHasHeadings)
            {
                m_vRowColors.push_back(m_v4Color);
                m_vRowDataColors.push_back(m_v4DataColor);
            }
            else
            {
                m_vRowColors.push_back(m_v4AltRowColor);
                m_vRowDataColors.push_back(m_v4AltDataColor);
            }
        }

        RecalculateSize();
    }

    if (m_bAutoSort && m_uiSortCol != uint(-1))
        SortTable();
}


/*====================
  CTable::AppendData
  ====================*/
void    CTable::AppendData(const tstring &sID, const tsvector &vData)
{
    if (vData.empty())
        return;

    if (m_vRow.empty())
    {
        Data(sID, tsvector(vData.begin(), vData.begin() + MIN(vData.size(), (size_t)m_uiBaseCols)));
        AppendData(sID, tsvector(vData.begin() + MIN(vData.size(), (size_t)m_uiBaseCols), vData.end()));
        return;
    }

    if (m_vRow.back().vsCol.size() + vData.size() > m_uiBaseCols)
    {
        uint uiRemainingCells(0);
        if (m_vRow.back().vsCol.size() < m_uiBaseCols)
            uiRemainingCells = m_uiBaseCols - m_vRow.back().vsCol.size();
        if (uiRemainingCells == 0)
        {
            Data(sID, tsvector(vData.begin(), vData.begin() + MIN(vData.size(), (size_t)m_uiBaseCols)));
            AppendData(sID, tsvector(vData.begin() + MIN(vData.size(), (size_t)m_uiBaseCols), vData.end()));
        }
        else
        {
            AppendData(sID, tsvector(vData.begin(), vData.begin() + MIN(vData.size(), (size_t)uiRemainingCells)));
            AppendData(sID, tsvector(vData.begin() + MIN(vData.size(), (size_t)uiRemainingCells), vData.end()));
        }
        return;
    }

    m_vRowIDs.back().vsCol.insert(m_vRowIDs.back().vsCol.end(), vData.size(), sID);
    m_vRow.back().vsCol.insert(m_vRow.back().vsCol.end(), vData.begin(), vData.end());

    // Only sort if the column we added is what we sort by
    if (m_bAutoSort && m_uiSortCol != uint(-1) && m_vRow.back().vsCol.size() - 1 < m_uiSortCol && m_vRow.back().vsCol.size() + vData.size() > m_uiSortCol)
        SortTable();
}


/*====================
  CTable::ClearData
  ====================*/
void    CTable::ClearData()
{
    m_vRowIDs.clear();
    m_vRow.clear();

    if (m_bHasHeadings)
        Data(m_sHeadingID, m_vHeadings, true);

    if (m_bExpandable)
    {
        if (m_vsRowSize.size() > m_uiBaseRows)
        {
            m_vsRowSize.erase(m_vsRowSize.begin() + m_uiBaseRows, m_vsRowSize.end());
            m_vRowColors.erase(m_vRowColors.begin() + m_uiBaseRows, m_vRowColors.end());
            m_vRowDataColors.erase(m_vRowDataColors.begin() + m_uiBaseRows, m_vRowDataColors.end());
        }

        RecalculateSize();
    }

    if (!m_sOnClear.empty())
        UIScript.Evaluate(this, m_sOnClear);
}


/*====================
  CTable::ProcessInputCursor
  ====================*/
bool    CTable::ProcessInputCursor(const CVec2f &v2CursorPos)
{
    if (!HasFlags(WFLAG_VISIBLE) || !HasFlags(WFLAG_ENABLED))
        return false;

    if (!HasFlags(WFLAG_PASSIVE_CHILDREN))
    {
        // Let children handle the input directly
        for (WidgetPointerVector_rit it(m_vChildren.rbegin()), itEnd(m_vChildren.rend()); it != itEnd; ++it)
            if ((*it)->ProcessInputCursor(v2CursorPos - m_recArea.lt()))
                return true;
    }

    CVec2ui v2Cell(GetCellFromCoord(v2CursorPos));
    if (v2Cell == m_v2LastCell && v2Cell.y < m_vRow.size() && v2Cell.x < m_vRow[v2Cell.y].vsCol.size())
        return true;

    if (m_v2LastCell.y < m_vRow.size() && m_v2LastCell.x < m_vRow[m_v2LastCell.y].vsCol.size())
    {
        m_uiEventRow = m_v2LastCell.y;
        m_uiEventCol = m_v2LastCell.x;
        m_sEventDataID = m_vRowIDs[m_v2LastCell.y].vsCol[m_v2LastCell.x];
        m_sEventData = m_vRow[m_v2LastCell.y].vsCol[m_v2LastCell.x];
        DO_EVENT_RETURN(WEVENT_MOUSEOUT, true)
    }

    if (v2Cell.y < m_vRow.size() && v2Cell.x < m_vRow[v2Cell.y].vsCol.size())
    {
        m_v2LastCell = v2Cell;
        m_uiEventRow = v2Cell.y;
        m_uiEventCol = v2Cell.x;
        m_sEventDataID = m_vRowIDs[v2Cell.y].vsCol[v2Cell.x];
        m_sEventData = m_vRow[v2Cell.y].vsCol[v2Cell.x];
        DO_EVENT_RETURN(WEVENT_MOUSEOVER, true)
        return true;
    }

    m_v2LastCell.Set(uint(-1), uint(-1));
    return false;
}


/*====================
  CTable::ProcessInputMouseButton
  ====================*/
bool    CTable::ProcessInputMouseButton(const CVec2f &v2CursorPos, EButton button, float fValue)
{
    if (!HasFlags(WFLAG_VISIBLE) || !HasFlags(WFLAG_ENABLED))
        return false;

    CVec2f v2LastCursorPos;

    if (HasSavedCursorPos())
        v2LastCursorPos = m_v2LastCursorPos;

    if (!HasFlags(WFLAG_PASSIVE_CHILDREN))
    {
        // Check children, relative to parent widget
        for (WidgetPointerVector_rit it(m_vChildren.rbegin()), itEnd(m_vChildren.rend()); it != itEnd; ++it)
        {
            if ((*it)->ProcessInputMouseButton(v2CursorPos - m_recArea.lt(), button, fValue))
                return true;
        }
    }

    if (m_eWidgetType == WIDGET_INTERFACE || HasFlags(WFLAG_NO_CLICK))
        return false;

    // Check this widget
    if (fValue == 1.0f) // down
    {
        bool bContains(false);
        float fParentWidth;
        float fParentHeight;

        if (m_pParent != NULL)
        {
            fParentWidth = m_pParent->GetWidth();
            fParentHeight = m_pParent->GetHeight();
        }
        else
        {
            fParentWidth = Draw2D.GetScreenW();
            fParentHeight = Draw2D.GetScreenH();
        }

        float fRowOffset(0.0f);
        uint uiNumChecked(0);
        uint uiMaxChecked(-1);
        uint uiPos(0);

        if (m_bUseScrollbar)
            uiMaxChecked = m_uiBaseRows;

        if (m_recArea.AltContains(v2CursorPos))
        {
            for (tsvector_it it(m_vsRowSize.begin()); it != m_vsRowSize.end() && uiNumChecked < uiMaxChecked; ++it)
            {
                float fRowHeight(GetSizeFromString(*it, GetSizeFromString(m_sOrigHeight, fParentHeight, fParentWidth), GetSizeFromString(m_sOrigWidth, fParentWidth, fParentHeight)));

                if (!m_bUseScrollbar || (uiPos >= m_pScrollbar->GetValueFloat() || m_vRow[uiPos].bHeader) && fRowOffset + fRowHeight <= GetHeight())
                {
                    fRowOffset += fRowHeight;

                    if (v2CursorPos.y - m_recArea.top <= fRowOffset)
                    {
                        bContains = true;
                        break;
                    }

                    uiNumChecked++;
                }

                uiPos++;
            }
        }

        if (bContains || (HasSavedCursorPos() && m_recArea.AltContains(v2LastCursorPos)))
        {
            // set active widget we click on so we know where to send input
            if (IsInteractive())
                m_pInterface->SetActiveWidget(this);

            switch (button)
            {
            case BUTTON_MOUSEL:
            case BUTTON_MOUSER:
            case BUTTON_MOUSEM:
            case BUTTON_MOUSEX1:
            case BUTTON_MOUSEX2:
            case BUTTON_WHEELUP:
            case BUTTON_WHEELDOWN:
                MouseDown(button, v2CursorPos);
                break;

            default:
                break;
            }

            return true;
        }
    }
    else // up
    {
        bool bExclusive(m_pInterface && m_pInterface->GetExclusiveWidget() == this);

        switch (button)
        {
        case BUTTON_MOUSEL:
        case BUTTON_MOUSER:
            MouseUp(button, v2CursorPos);
            break;

        default:
            break;
        }

        return bExclusive; // only eat exclusive up's
    }

    return false;
}


/*--------------------
  GetData
  --------------------*/
UI_CMD(GetData, 2)
{
    if(pThis->GetType() != WIDGET_TABLE)
        return _T("");

    uint uiCol(AtoI(vArgList[0]->Evaluate()));
    uint uiRow(AtoI(vArgList[1]->Evaluate()));

    return static_cast<CTable*>(pThis)->GetData(uiCol, uiRow);
}


/*--------------------
  GetCellPositionX
  --------------------*/
UI_CMD(GetCellPositionX, 2)
{
    if(pThis->GetType() != WIDGET_TABLE)
        return _T("");

    uint uiCol(AtoI(vArgList[0]->Evaluate()));
    uint uiRow(AtoI(vArgList[1]->Evaluate()));

    return XtoA(static_cast<CTable*>(pThis)->GetCoordFromCell(CVec2ui(uiCol, uiRow))[X]);
}


/*--------------------
  GetCellPositionY
  --------------------*/
UI_CMD(GetCellPositionY, 2)
{
    if(pThis->GetType() != WIDGET_TABLE)
        return _T("");

    uint uiCol(AtoI(vArgList[0]->Evaluate()));
    uint uiRow(AtoI(vArgList[1]->Evaluate()));

    return XtoA(static_cast<CTable*>(pThis)->GetCoordFromCell(CVec2ui(uiCol, uiRow))[Y]);
}


/*--------------------
  SetCell
  --------------------*/
UI_VOID_CMD(SetCell, 3)
{
    if (pThis->GetType() != WIDGET_TABLE)
        return;

    uint uiCol(AtoI(vArgList[0]->Evaluate()));
    uint uiRow(AtoI(vArgList[1]->Evaluate()));

    static_cast<CTable*>(pThis)->SetCell(uiCol, uiRow, vArgList[2]->Evaluate());
}


/*--------------------
  Data
  --------------------*/
UI_VOID_CMD(Data, 1)
{
    if(pThis->GetType() != WIDGET_TABLE)
        return;

    tstring sID(vArgList[0]->Evaluate());
    tsvector vData;
    for (ScriptTokenVector_cit cit(vArgList.begin() + 1); cit != vArgList.end(); ++ cit)
        vData.push_back((*cit)->Evaluate());

    static_cast<CTable*>(pThis)->Data(sID, vData);
}


/*--------------------
  AppendData
  --------------------*/
UI_VOID_CMD(AppendData, 1)
{
    if(pThis->GetType() != WIDGET_TABLE)
        return;

    tstring sID(vArgList[0]->Evaluate());
    tsvector vData;
    for (ScriptTokenVector_cit cit(vArgList.begin() + 1); cit != vArgList.end(); ++ cit)
        vData.push_back((*cit)->Evaluate());

    static_cast<CTable*>(pThis)->AppendData(sID, vData);
}


/*--------------------
  ClearData
  --------------------*/
UI_VOID_CMD(ClearData, 0)
{
    if(pThis->GetType() != WIDGET_TABLE)
        return;

    static_cast<CTable*>(pThis)->ClearData();
}


/*--------------------
  SortByCol
  --------------------*/
UI_VOID_CMD(SortByCol, 1)
{
    if (pThis->GetType() != WIDGET_TABLE)
        return;

    static_cast<CTable*>(pThis)->SortByCol(AtoI(vArgList[0]->Evaluate()), false, false);
}


/*--------------------
  SortByColReverse
  --------------------*/
UI_VOID_CMD(SortByColReverse, 1)
{
    if (pThis->GetType() != WIDGET_TABLE)
        return;

    static_cast<CTable*>(pThis)->SortByCol(AtoI(vArgList[0]->Evaluate()), true, false);
}


/*--------------------
  SortByColNum
  --------------------*/
UI_VOID_CMD(SortByColNum, 1)
{
    if (pThis->GetType() != WIDGET_TABLE)
        return;

    static_cast<CTable*>(pThis)->SortByCol(AtoI(vArgList[0]->Evaluate()), false, true);
}


/*--------------------
  SortByColNumReverse
  --------------------*/
UI_VOID_CMD(SortByColNumReverse, 1)
{
    if (pThis->GetType() != WIDGET_TABLE)
        return;

    static_cast<CTable*>(pThis)->SortByCol(AtoI(vArgList[0]->Evaluate()), true, true);
}
