// (C)2005 S2 Games
// c_VertexCliffMap.h
//
//=============================================================================
#ifndef __C_VERTEXCLIFFMAP_H__
#define __C_VERTEXCLIFFMAP_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_worldcomponent.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//class CWorld;
//=============================================================================

//=============================================================================
// CVertexCliffMap
//=============================================================================
class CVertexCliffMap : public IWorldComponent
{
private:
    int             *m_pVertexCliff;
    int             m_iCliffMapWidth;
    int             m_iCliffMapHeight;

    void    CalculateVertexNormal(int iX, int iY);

public:
    ~CVertexCliffMap();
    CVertexCliffMap(EWorldComponent eComponent);

    bool    Load(CArchive &archive, const CWorld *pWorld);
    bool    Generate(const CWorld *pWorld);
    void    Release();
    void    Update(const CRecti &recArea);
    bool    Serialize(IBuffer *pBuffer);

    int     GetVertCliffMapWidth() { return m_iCliffMapWidth; }
    int     GetVertCliffMapHeight() { return m_iCliffMapWidth; }

    bool    GetRegion(const CRecti &recArea, void *pDest, int iLayer) const;
    bool    SetRegion(const CRecti &recArea, void *pSource, int iLayer);
    

    K2_API const uint   GetVertexCliff(int iX, int iY);
    int*    GetVertexCliffMap()         { return m_pVertexCliff; }

    //K2_API byte   GetCliff(int x, int y);
};
//=============================================================================

#endif //__C_VERTEXCLIFFMAP_H__
