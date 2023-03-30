//(C)2005 S2 Games
// c_vertexfoliagemap.h
//
//=============================================================================
#ifndef __C_VERTEXFOLIAGEMAP_H__
#define __C_VERTEXFOLIAGEMAP_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_worldcomponent.h"
//=============================================================================

//=============================================================================
// CVertexFoliageMap
//=============================================================================
class CVertexFoliageMap : public IWorldComponent
{
private:
    float*  m_pFoliageDensity;
    CVec3f* m_pFoliageSize;
    CVec3f* m_pFoliageVariance;

public:
    ~CVertexFoliageMap();
    CVertexFoliageMap();

    bool    Load(CArchive &archive, const CWorld *pWorld);
    bool    Generate(const CWorld *pWorld);
    void    Release();

    float   GetFoliageDensity(int iX, int iY);
    CVec3f  GetFoliageSize(int iX, int iY);
    CVec3f  GetFoliageVariance(int iX, int iY);
};
//=============================================================================

#endif //__C_VERTEXFOLIAGEMAP_H__
