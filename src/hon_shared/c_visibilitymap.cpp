// (C)2008 S2 Games
// c_visibilitymap.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_visibilitymap.h"

#include "../k2/c_bitmap.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

/*====================
  CVisibilityMap::~CVisibilityMap
  ====================*/
CVisibilityMap::~CVisibilityMap()
{
    SAFE_DELETE_ARRAY(m_pCells);
}


/*====================
  CVisibilityMap::CVisibilityMap
  ====================*/
CVisibilityMap::CVisibilityMap() :
m_pCells(NULL)
{
}


/*====================
  CVisibilityMap::Initialize
  ====================*/
void    CVisibilityMap::Initialize(uint uiWidth, uint uiHeight)
{
    SAFE_DELETE_ARRAY(m_pCells);

    m_uiWidth = uiWidth;
    m_uiHeight = uiHeight;

    m_pCells = K2_NEW_ARRAY(ctx_Game, byte, m_uiWidth*m_uiHeight);

    MemManager.Set(m_pCells, 0, m_uiWidth * m_uiHeight);
}


/*====================
  CVisibilityMap::Clear
  ====================*/
void    CVisibilityMap::Clear(byte yValue)
{
    if (m_pCells != NULL)
        MemManager.Set(m_pCells, yValue, m_uiWidth * m_uiHeight);
}


/*====================
  CVisibilityMap::AddVision
  ====================*/
void    CVisibilityMap::AddVision(const CRectui &recRegion, const byte *pBuffer, uint uiBufferSpan, byte yVision)
{
    assert(recRegion.left >= 0);
    assert(recRegion.top >= 0);
    assert(recRegion.right <= m_uiWidth);
    assert(recRegion.bottom <= m_uiHeight);

    const byte *pSrc(pBuffer);
    byte *pDst(&m_pCells[GetCellIndex(recRegion.left, recRegion.top)]);

    uint uiRegionWidth(recRegion.GetWidth());
    uint uiRegionHeight(recRegion.GetHeight());

    uint uiSrcSpan(uiBufferSpan - uiRegionWidth);
    uint uiDstSpan(m_uiWidth - uiRegionWidth);
    
    for (uint uiY(0); uiY < uiRegionHeight; ++uiY, pDst += uiDstSpan, pSrc += uiSrcSpan)
    {
        for (uint uiX(0); uiX < uiRegionWidth; ++uiX, ++pDst, ++pSrc)
        {
            *pDst |= *pSrc & yVision;
        }
    }
}


/*====================
  CVisibilityMap::RemoveVision
  ====================*/
void    CVisibilityMap::RemoveVision(const CRectui &recRegion, const byte *pBuffer, uint uiBufferSpan, byte yVision)
{
    const byte *pSrc(pBuffer);
    byte *pDst(&m_pCells[GetCellIndex(recRegion.left, recRegion.top)]);

    uint uiRegionWidth(recRegion.GetWidth());
    uint uiRegionHeight(recRegion.GetHeight());

    uint uiSrcSpan(uiBufferSpan - uiRegionWidth);
    uint uiDstSpan(m_uiWidth - uiRegionWidth);
    
    for (uint uiY(0); uiY < uiRegionHeight; ++uiY, pDst += uiDstSpan, pSrc += uiSrcSpan)
    {
        for (uint uiX(0); uiX < uiRegionWidth; ++uiX, ++pDst, ++pSrc)
        {
            if (*pSrc)
                *pDst &= ~(*pSrc & yVision);
        }
    }
}


/*====================
  CVisibilityMap::FillBitmap
  ====================*/
void    CVisibilityMap::FillBitmap(CBitmap &cBmp, byte yBlocked, byte yUnblocked)
{
    if (cBmp.GetWidth() != m_uiWidth || cBmp.GetHeight() != m_uiHeight || cBmp.GetBMPType() != BITMAP_ALPHA)
        return;

    const byte *pCell(m_pCells);
    byte *pBuffer(cBmp.GetBuffer());
    uint uiArea(m_uiHeight * m_uiWidth);

    for (uint ui(0); ui < uiArea; ++ui, ++pCell, ++pBuffer)
        *pBuffer = *pCell ? yBlocked : yUnblocked;
}
