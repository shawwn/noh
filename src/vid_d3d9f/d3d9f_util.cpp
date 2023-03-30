// (C)2005 S2 Games
// d3d9f_util.cpp
//
// Direct3D utility functions
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "d3d9f_util.h"
#include "d3d9f_main.h"
#include "d3d9f_state.h"
#include "d3d9f_shader.h"
#include "d3d9f_material.h"

#include "../k2/c_console.h"
#include "../k2/c_system.h"
#include "../k2/c_vec3.h"
#include "../k2/c_mesh.h"
#include "../k2/c_material.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

/*====================
  ParseDefinitions
  ====================*/
int		ParseDefinitions(const tstring &sString, vector<StringPair> &v)
{
	size_t	zCurrentPos = 0;
	size_t	zNextPos = 0;

	do
	{
		zNextPos = sString.find_first_of(_T(';'), zCurrentPos);

		size_t	zSplitPos = sString.find_first_of(_T("=;"), zCurrentPos);

		if (zSplitPos == tstring::npos)
			v.push_back(StringPair(sString.substr(zCurrentPos), _T("")));
		else if (sString[zSplitPos] == _T(';'))
			v.push_back(StringPair(sString.substr(zCurrentPos, zSplitPos - zCurrentPos), _T("")));
		else
			v.push_back(StringPair(sString.substr(zCurrentPos, zSplitPos - zCurrentPos), sString.substr(zSplitPos + 1, zNextPos - zSplitPos - 1)));

		zCurrentPos = zNextPos + 1;

	} while (zNextPos != tstring::npos);

	return int(v.size());
}


/*====================
  D3D_TransformPoints
  ====================*/
void	D3D_TransformPoints(vector<CVec3f> &vPoints, D3DXMATRIXA16 *mTransform)
{
	for (vector<CVec3f>::iterator it = vPoints.begin(); it != vPoints.end(); ++it)
	{
		D3DXVECTOR4	v(it->x, it->y, it->z, 1.0f);

		D3DXVec4Transform(&v, &v, mTransform);

		it->x = v.x/v.w;
		it->y = v.y/v.w;
		it->z = v.z/v.w;
	}
}


/*====================
  D3D_TransformPoint
  ====================*/
CVec3f	D3D_TransformPoint(const CVec3f &vPoint, const D3DXMATRIXA16 &mTransform)
{
	D3DXVECTOR4	v(vPoint.x, vPoint.y, vPoint.z, 1.0f);

	D3DXVec4Transform(&v, &v, &mTransform);

	return CVec3f(v.x/v.w, v.y/v.w, v.z/v.w);
}


/*====================
  D3D_TransformNormal
  ====================*/
CVec3f	D3D_TransformNormal(const CVec3f &vNormal, const D3DXMATRIXA16 &mTransform)
{
	D3DXVECTOR3	v(vNormal.x, vNormal.y, vNormal.z);

	D3DXVec3TransformNormal(&v, &v, &mTransform);

	return CVec3f(v.x, v.y, v.z);
}


/*====================
  D3D_DrawIndexedPrimitive
  ====================*/
void	D3D_DrawIndexedPrimitive(D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount)
{
	PROFILE("D3D_DrawIndexedPrimitive");
	g_pd3dDevice->DrawIndexedPrimitive(Type, BaseVertexIndex, MinIndex, NumVertices, StartIndex, PrimitiveCount);
}


/*====================
  D3D_DrawPrimitive
  ====================*/
void	D3D_DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	PROFILE("D3D_DrawPrimitive");
	g_pd3dDevice->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}


#if 0
/*====================
  D3D_DrawBox

  This assumes that the transformation matrices are already setup correctly
  ====================*/
void	D3D_AddBox(const CBBoxf &bbBox, const CVec4f &vColor)
{
	SPositionVertex *pVertices;

	if (FAILED(g_pVBBox->Lock(0, 8 * sizeof(SPositionVertex), (void**)&pVertices, D3DLOCK_NOSYSLOCK | D3DLOCK_DISCARD)))
		return;

	pVertices[0].v[0] = bbBox.GetMin()[0];
	pVertices[0].v[1] = bbBox.GetMin()[1];
	pVertices[0].v[2] = bbBox.GetMax()[2];

	pVertices[1].v[0] = bbBox.GetMin()[0];
	pVertices[1].v[1] = bbBox.GetMax()[1];
	pVertices[1].v[2] = bbBox.GetMax()[2];

	pVertices[2].v[0] = bbBox.GetMax()[0];
	pVertices[2].v[1] = bbBox.GetMax()[1];
	pVertices[2].v[2] = bbBox.GetMax()[2];

	pVertices[3].v[0] = bbBox.GetMax()[0];
	pVertices[3].v[1] = bbBox.GetMin()[1];
	pVertices[3].v[2] = bbBox.GetMax()[2];

	pVertices[4].v[0] = bbBox.GetMin()[0];
	pVertices[4].v[1] = bbBox.GetMin()[1];
	pVertices[4].v[2] = bbBox.GetMin()[2];

	pVertices[5].v[0] = bbBox.GetMin()[0];
	pVertices[5].v[1] = bbBox.GetMax()[1];
	pVertices[5].v[2] = bbBox.GetMin()[2];

	pVertices[6].v[0] = bbBox.GetMax()[0];
	pVertices[6].v[1] = bbBox.GetMax()[1];
	pVertices[6].v[2] = bbBox.GetMin()[2];

	pVertices[7].v[0] = bbBox.GetMax()[0];
	pVertices[7].v[1] = bbBox.GetMin()[1];
	pVertices[7].v[2] = bbBox.GetMin()[2];

	g_pVBBox->Unlock();

	g_bObjectColor = true;
	g_vObjectColor = vColor;

	D3D_PushMaterial(g_SimpleMaterial3D, PHASE_COLOR, VERTEX_POSITION, 0.0f);

	D3D_SetStreamSource(0, g_pVBBox, 0, sizeof(SPositionVertex));
	D3D_SetIndices(g_pIBBox);

	D3D_DrawIndexedPrimitive(D3DPT_LINELIST, 0, 0, 8, 0, 12);

	g_bObjectColor = false;

	D3D_PopMaterial();
}
#endif


/*====================
  D3D_DWORD
  ====================*/
DWORD	D3D_DWORD(float fValue)
{
	return *(DWORD*)&fValue;
}


/*====================
  D3D_GetMaterial
  ====================*/
CMaterial&	D3D_GetMaterial(ResHandle hMaterial)
{
	return *g_ResourceManager.GetMaterial(hMaterial);
}


/*====================
  D3D_AxisToMatrix
  ====================*/
void	D3D_AxisToMatrix(const CAxis &axis, D3DXMATRIXA16 *tm)
{
	// construct the transformation matrix
	(*tm)[0] = axis[RIGHT][X];
	(*tm)[1] = axis[RIGHT][Y];
	(*tm)[2] = axis[RIGHT][Z];
	(*tm)[3] = 0.0f;

	(*tm)[4] = axis[FORWARD][X];
	(*tm)[5] = axis[FORWARD][Y];
	(*tm)[6] = axis[FORWARD][Z];
	(*tm)[7] = 0.0f;

	(*tm)[8] = axis[UP][X];
	(*tm)[9] = axis[UP][Y];
	(*tm)[10] = axis[UP][Z];
	(*tm)[11] = 0.0f;

	(*tm)[12] = 0.0f;
	(*tm)[13] = 0.0f;
	(*tm)[14] = 0.0f;
	(*tm)[15] = 1.0f;
}


/*====================
  D3D_AxisToMatrix
  ====================*/
void	D3D_TransformToMatrix(const matrix43_t *transform, D3DXMATRIXA16 *tm)
{
	//construct the transformation matrix
	(*tm)[0] = transform->axis[0][0];
	(*tm)[1] = transform->axis[0][1];
	(*tm)[2] = transform->axis[0][2];
	(*tm)[3] = 0.0f;
	(*tm)[4] = transform->axis[1][0];
	(*tm)[5] = transform->axis[1][1];
 	(*tm)[6] = transform->axis[1][2];
 	(*tm)[7] = 0.0f;
 	(*tm)[8] = transform->axis[2][0];
 	(*tm)[9] = transform->axis[2][1];
 	(*tm)[10] = transform->axis[2][2];
 	(*tm)[11] = 0.0f;
 	(*tm)[12] = transform->pos[0];
 	(*tm)[13] = transform->pos[1];
 	(*tm)[14] = transform->pos[2];
 	(*tm)[15] = 1.0f;
}


/*====================
  D3D_Color
  ====================*/
DWORD	D3D_Color(const CVec4f &v4Color)
{
    return D3DCOLOR_ARGB
	(
		CLAMP(INT_FLOOR(v4Color[3] * 255.0f), 0, 255),
		CLAMP(INT_FLOOR(v4Color[0] * 255.0f), 0, 255),
		CLAMP(INT_FLOOR(v4Color[1] * 255.0f), 0, 255),
		CLAMP(INT_FLOOR(v4Color[2] * 255.0f), 0, 255)
	);
}


/*====================
  D3D_WaitForQuery
  ====================*/
void	D3D_WaitForQuery()
{
	uint uiStartQuery(K2System.Milliseconds());
	while (g_pQueueLimitQuery->GetData(NULL, 0, D3DGETDATA_FLUSH) == S_FALSE)
	{
		if (K2System.Milliseconds() - uiStartQuery > 500)
		{
			Console.Video << _T("Query reset") << newl;

			SAFE_RELEASE(g_pQueueLimitQuery);
			g_DeviceCaps.bQuery = SUCCEEDED(g_pd3dDevice->CreateQuery(D3DQUERYTYPE_EVENT, &g_pQueueLimitQuery));
			break;
		}
	}
}