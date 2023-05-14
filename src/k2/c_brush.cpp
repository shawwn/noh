// (C)2005 S2 Games
// c_brush.cpp
//
// Brush loading, attributes, etc.
// Has the ability to dynamically load brushes at different sizes, but all
// the brushes are stored as 32x32 currently, so we'll have to settle.
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_brush.h"
#include "c_bitmap.h"
#include "c_console.h"
#include "c_uicmd.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CBrush  *   CBrush::s_pBrushes[MAX_BRUSHES];
int         CBrush::s_iCurrentBrush(0);
int         CBrush::s_iNumBrushes(0);
//=============================================================================


/*====================
  CBrush::CBrush

  Loads a brush from the bitmap stored in m_sFilename
  ====================*/
CBrush::CBrush() :
m_cBrushData(nullptr),
m_iBrushSize(0),
m_sFilename(_T(""))
{
}


/*====================
  CBrush::CBrush

  Loads a brush from the bitmap stored in sFilename
  ====================*/
CBrush::CBrush(const tstring &sFilename) :
m_cBrushData(nullptr),
m_iBrushSize(0),
m_sFilename(_T(""))
{
    Load(sFilename);
}


/*====================
  CBrush::~CBrush
  ====================*/
CBrush::~CBrush()
{
    if (m_cBrushData)
    {
        K2_DELETE_ARRAY(m_cBrushData);
        m_cBrushData = nullptr;
    }
}


/*====================
  CBrush::Load

  Loads a brush from the bitmap stored in m_sFilename
  ====================*/
bool    CBrush::Load(const tstring &sFileName)
{
    CBitmap bmp;

    try
    {
        if (sFileName.empty())
            throw CException(_T("No file name"), E_WARNING);

        if (!bmp.Load(sFileName))
            throw CException(_T("Failed to load bitmap"), E_WARNING);

        if (bmp.GetWidth() != bmp.GetHeight())
            throw CException(_T("Brush is not square"), E_WARNING);

        m_sFilename = sFileName;

        int size = (bmp.GetWidth() + 2) * (bmp.GetHeight() + 2);
        m_iBrushSize = (bmp.GetWidth() + 2);
        m_cBrushData = K2_NEW_ARRAY(ctx_Editor, byte, size);

        for (int y = 0; y < bmp.GetHeight() + 2 && y < MAX_BRUSH_SIZE; ++y)
            for (int x = 0; x < bmp.GetWidth() + 2 && x < MAX_BRUSH_SIZE; ++x)
                m_cBrushData[y * m_iBrushSize + x] = 0;

        for (int y = 0; y < bmp.GetHeight() - 2 && y < MAX_BRUSH_SIZE; ++y)
        {
            for (int x = 0; x < bmp.GetWidth() - 2 && x < MAX_BRUSH_SIZE; ++x)
            {
                bvec4_t color;
                bmp.GetColor(x, y, color);
                byte i = color[0];
                if (i < 10)
                    i = 0;
                m_cBrushData[(y + 1) * m_iBrushSize + (x + 1)] = i;
            }
        }

        bmp.Free();
        return true;
    }
    catch (CException &ex)
    {
        bmp.Free();
        ex.Process(_TS("CBrush::Load(") + sFileName + _TS(") - "), NO_THROW);
        return false;
    }
}


/*====================
  CBrush::ClipBrush
  ====================*/
bool    CBrush::ClipBrush(CRecti &recClip)
{
    int iMinX = m_iBrushSize;
    int iMaxX = -m_iBrushSize;
    int iMinY = m_iBrushSize;
    int iMaxY = -m_iBrushSize;

    for (int y(0); y < m_iBrushSize; ++y)
    {
        for (int x(0); x < m_iBrushSize; ++x)
        {
            if (m_cBrushData[y * m_iBrushSize + x] == 0)
                continue;

            if (iMinX > x) iMinX = x;
            if (iMinY > y) iMinY = y;
            if (iMaxX < x) iMaxX = x;
            if (iMaxY < y) iMaxY = y;
        }
    }

    recClip.Set(iMinX, iMinY, iMaxX + 1, iMaxY + 1);
    if (!recClip.IsNormalized())
        return false;

    return true;
}


/*====================
  CBrush::SelectBrush
  ====================*/
void    CBrush::SelectBrush(int iBrush)
{
    if (iBrush < 0 || iBrush >= MAX_BRUSHES || !s_pBrushes[iBrush])
        return;

    CBrush::s_iCurrentBrush = iBrush;
}


/*====================
  CBrush::GetCurrentBrush
  ====================*/
CBrush* CBrush::GetCurrentBrush()
{
    return s_pBrushes[s_iCurrentBrush];
}


/*====================
  CBrush::GetBrush
  ====================*/
CBrush* CBrush::GetBrush(int iBrush)
{
    if (iBrush < 0 || iBrush >= MAX_BRUSHES || !s_pBrushes[iBrush])
        return nullptr;

    return s_pBrushes[iBrush];
}


/*====================
  CBrush::Load

  Loads a brush from the bitmap stored in m_sFilename
  ====================*/
bool    CBrush::Load(int iBrush, const tstring &sFilename)
{
    if (iBrush < 0 || iBrush >= MAX_BRUSHES)
        return false;

    if (s_pBrushes[iBrush])
    {
        K2_DELETE(s_pBrushes[iBrush]);
        s_pBrushes[iBrush] = nullptr;
        --s_iNumBrushes;
    }

    s_pBrushes[iBrush] = K2_NEW(ctx_Editor,  CBrush)(sFilename);
    return true;
}


/*--------------------
  GetCurrentBrush
  --------------------*/
UI_CMD(GetCurrentBrush, 0)
{
    return XtoA(CBrush::GetCurrentBrushIndex());
}


/*--------------------
  GetCurrentBrushImage
  --------------------*/
UI_CMD(GetCurrentBrushImage, 0)
{
    CBrush *pBrush(CBrush::GetCurrentBrush());

    if (pBrush)
        return pBrush->GetFilename();
    else
        return _T("");
}
