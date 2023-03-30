// (C)2008 S2 Games
// c_tilecliffmap.h
//
//=============================================================================
#ifndef __C_TILECLIFFMAP_H__
#define __C_TILECLIFFMAP_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_worldcomponent.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CWorld;
//=============================================================================

//=============================================================================
// CTileCliffMap
//=============================================================================
class CTileCliffMap : public IWorldComponent
{
private:
    byte*           m_pTileCliffs;

public:
    ~CTileCliffMap();
    CTileCliffMap(EWorldComponent eComponent);

    bool    Load(CArchive &archive, const CWorld *pWorld);
    bool    Generate(const CWorld *pWorld);
    void    Release();
    bool    Serialize(IBuffer *pBuffer);

    bool    GetRegion(const CRecti &recArea, void *pDest, int iLayer) const;
    bool    SetRegion(const CRecti &recArea, void *pSource, int iLayer);

    byte*   GetCliffMap() const     { return m_pTileCliffs; }

    K2_API byte GetCliff(int x, int y);
    K2_API void SetCliff(int x, int y, byte yCliff);
};
//=============================================================================

#endif //__C_TILECLIFFMAP_H__
