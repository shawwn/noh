// (C)2006 S2 Games
// c_vertexcolormap.h
//
//=============================================================================
#ifndef __C_TEXELALPHAMAP_H__
#define __C_TEXELALPHAMAP_H__

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
// CTexelAlphaMap
//=============================================================================
class CTexelAlphaMap : public IWorldComponent
{
private:
    byte        *m_pTexelAlpha;

public:
    ~CTexelAlphaMap();
    CTexelAlphaMap(EWorldComponent eComponent);

    bool    Load(CArchive &archive, const CWorld *pWorld);
    bool    Generate(const CWorld *pWorld);
    void    Release();
    bool    Serialize(IBuffer *pBuffer);

    bool    GetRegion(const CRecti &recArea, void *pDest, int iLayer) const;
    bool    SetRegion(const CRecti &recArea, void *pSource, int iLayer);

    K2_API byte GetTexelAlpha(int iX, int iY);
    K2_API void SetAlpha(int iX, int iY, byte yAlpha);
};
//=============================================================================
#endif //__C_TEXELALPHAMAP_H__
