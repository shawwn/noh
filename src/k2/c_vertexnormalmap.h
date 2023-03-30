// (C)2005 S2 Games
// c_vertexnormalmap.h
//
//=============================================================================
#ifndef __C_VERTEXNORMALMAP_H__
#define __C_VERTEXNORMALMAP_H__

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
// CVertexNormalMap
//=============================================================================
class CVertexNormalMap : public IWorldComponent
{
private:
    CVec3f*         m_pVertexNormals;

    void    CalculateVertexNormal(int iX, int iY);

public:
    ~CVertexNormalMap();
    CVertexNormalMap(EWorldComponent eComponent);

    bool    Save(CArchive &archive)     { return true; }
    bool    Load(CArchive &archive, const CWorld *pWorld);
    bool    Generate(const CWorld *pWorld);
    void    Release();
    void    Update(const CRecti &recArea);

    bool    GetRegion(int iStartX, int iStartY, int iWidth, int iHeight, void *pDest, int iDestSize, int iLayer) const;
    bool    SetRegion(int iStartX, int iStartY, int iWidth, int iHeight, void *pSource, int iSourceSize, int iLayer);

    K2_API const CVec3f&    GetVertexNormal(int iX, int iY);
};
//=============================================================================

#endif //__C_VERTEXNORMALMAP_H__
