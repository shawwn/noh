// (C)2005 S2 Games
// d3d9g_util.h
//
// Direct3D
//=============================================================================
#ifndef __D3D9_UTIL_H__
#define __D3D9_UTIL_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/c_vec3.h"
//=============================================================================

class CMaterial;

typedef pair<tstring, tstring> StringPair;

void    D3D_TransformPoints(vector<CVec3f> &vPoints, D3DXMATRIXA16 *mTransform);
CVec3f  D3D_TransformPoint(const CVec3f &vPoint, const D3DXMATRIXA16 &mTransform);
CVec3f  D3D_TransformNormal(const CVec3f &vNormal, const D3DXMATRIXA16 &mTransform);

void    D3D_DrawIndexedPrimitive(D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount);
void    D3D_DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount);

DWORD   D3D_Color(const CVec4f &v4Color);

int     ParseDefinitions(const tstring &sString, vector<StringPair> &v);

DWORD   D3D_DWORD(float fValue);

CMaterial&  D3D_GetMaterial(ResHandle hMaterial);

void    D3D_AxisToMatrix(const CAxis &Axis, D3DXMATRIXA16 *tm);
void    D3D_TransformToMatrix(const matrix43_t *transform, D3DXMATRIXA16 *tm);

void    D3D_WaitForQuery();

#endif // __D3D9_MAIN_H__



