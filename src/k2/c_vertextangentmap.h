// (C)2005 S2 Games
// c_vertextangentmap.h
//
//=============================================================================
#ifndef __C_VERTEXTANGENTMAP_H__
#define __C_VERTEXTANGENTMAP_H__

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
// CVertexTangentMap
//=============================================================================
class CVertexTangentMap : public IWorldComponent
{
private:
    CVec3f*         m_pVertexTangents;

    CVec3f  GetFaceTangent(const CVec3f &v0, const CVec3f &v1, const CVec3f &v2, const CVec2f &t0, const CVec2f t1, const CVec2f t2);
    void    CalculateVertexTangent(int iX, int iY);

public:
    ~CVertexTangentMap();
    CVertexTangentMap(EWorldComponent eComponent);

    bool    Save(CArchive &archive)     { return true; }
    bool    Load(CArchive &archive, const CWorld *pWorld);
    bool    Generate(const CWorld *pWorld);
    void    Release();
    void    Update(const CRecti &recArea);

    bool    GetRegion(int iStartX, int iStartY, int iWidth, int iHeight, void *pDest, int iDestSize, int iLayer) const;
    bool    SetRegion(int iStartX, int iStartY, int iWidth, int iHeight, void *pSource, int iSourceSize, int iLayer);

    K2_API const CVec3f&    GetVertexTangent(int iX, int iY);
};
//=============================================================================

#endif //__C_VERTEXTANGENTMAP_H__
