// (C)2008 S2 Games
// c_visibilitymap.h
//
//=============================================================================
#ifndef __C_VISIBILITYMAP_H__
#define __C_VISIBILITYMAP_H__

//=============================================================================
// Headers
//=============================================================================
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CBitmap;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CVisibilityMap
//=============================================================================
class CVisibilityMap
{
private:
    uint    m_uiWidth;
    uint    m_uiHeight;

    byte    *m_pCells;

    uint    GetCellIndex(uint uiX, uint uiY) const  { return (uiY * m_uiHeight + uiX); }

public:
    GAME_SHARED_API ~CVisibilityMap();
    GAME_SHARED_API CVisibilityMap();

    uint                    GetWidth() const        { return m_uiWidth; }
    uint                    GetHeight() const       { return m_uiHeight; }

    GAME_SHARED_API void    Initialize(uint uiWidth, uint uiHeight);
    GAME_SHARED_API void    Clear(byte yValue = 0);

    GAME_SHARED_API void    AddVision(const CRectui &recRegion, const byte *pBuffer, uint uiBufferSpan, byte yVision);
    GAME_SHARED_API void    RemoveVision(const CRectui &recRegion, const byte *pBuffer, uint uiBufferSpan, byte yVision);

    GAME_SHARED_API void    FillBitmap(CBitmap &cBmp, byte yBlocked, byte yUnblocked);

    inline bool             IsVisible(uint uiX, uint uiY) const { return m_pCells[GetCellIndex(uiX, uiY)] != 0; }
    inline byte             GetVision(uint uiX, uint uiY) const { return m_pCells[GetCellIndex(uiX, uiY)]; }
};
//=============================================================================

#endif //__C_VISIBILITYMAP_H__
