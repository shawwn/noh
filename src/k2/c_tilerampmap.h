// (C)2010 S2 Games
// c_tilerampmap.h
//
//=============================================================================
#ifndef __C_TILERAMPMAP_H__
#define __C_TILERAMPMAP_H__

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
// CTileRampMap
//=============================================================================
class CTileRampMap : public IWorldComponent
{
private:
    byte*           m_pTileRamps;

public:
    ~CTileRampMap();
    CTileRampMap(EWorldComponent eComponent);

    bool    Load(CArchive &archive, const CWorld *pWorld);
    bool    Generate(const CWorld *pWorld);
    void    Release();
    bool    Serialize(IBuffer *pBuffer);

    bool    GetRegion(const CRecti &recArea, void *pDest, int iLayer) const;
    bool    SetRegion(const CRecti &recArea, void *pSource, int iLayer);

    byte*   GetTileRampMap() const      { return m_pTileRamps; }

    K2_API byte GetRamp(int x, int y);
    K2_API byte GetRampByTile(int x, int y);
    K2_API void SetRamp(int x, int y, byte yRamp);
    K2_API void SetRampByTile(int x, int y, byte yRamp);
};
//=============================================================================

#endif //__C_TILERAMPMAP_H__
