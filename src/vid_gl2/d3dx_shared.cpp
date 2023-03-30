// (C)2008 S2 Games
// c_gfxtextures.cpp
//
// These function implementations have misc. origins or were hand-made when needed...
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"
//=============================================================================

/*====================
  D3DXMatrixMultiply
  ====================*/
D3DXMATRIX*	D3DXMatrixMultiply(D3DXMATRIX *pOut, const D3DXMATRIX *pM1, const D3DXMATRIX *pM2)
{
	D3DXMATRIX M1 = *pM2;
	D3DXMATRIX M2 = *pM1;

	float* pfOut = (float*)pOut;
	float* pfM1 = (float*)&M1;
	float* pfM2 = (float*)&M2;

    pfOut[0] = pfM1[0]*pfM2[0] + pfM1[4]*pfM2[1] + pfM1[8]*pfM2[2] 
                    + pfM1[12]*pfM2[3];
    pfOut[1] = pfM1[1]*pfM2[0] + pfM1[5]*pfM2[1] + pfM1[9]*pfM2[2] 
                    + pfM1[13]*pfM2[3];
    pfOut[2] = pfM1[2]*pfM2[0] + pfM1[6]*pfM2[1] + pfM1[10]*pfM2[2] 
                    + pfM1[14]*pfM2[3];
    pfOut[3] = pfM1[3]*pfM2[0] + pfM1[7]*pfM2[1] + pfM1[11]*pfM2[2] 
                    + pfM1[15]*pfM2[3];

    pfOut[4] = pfM1[0]*pfM2[4] + pfM1[4]*pfM2[5] + pfM1[8]*pfM2[6] 
                    + pfM1[12]*pfM2[7];
    pfOut[5] = pfM1[1]*pfM2[4] + pfM1[5]*pfM2[5] + pfM1[9]*pfM2[6] 
                    + pfM1[13]*pfM2[7];
    pfOut[6] = pfM1[2]*pfM2[4] + pfM1[6]*pfM2[5] + pfM1[10]*pfM2[6] 
                    + pfM1[14]*pfM2[7];
    pfOut[7] = pfM1[3]*pfM2[4] + pfM1[7]*pfM2[5] + pfM1[11]*pfM2[6] 
                    + pfM1[15]*pfM2[7];

    pfOut[8] = pfM1[0]*pfM2[8] + pfM1[4]*pfM2[9] + pfM1[8]*pfM2[10] 
                    + pfM1[12]*pfM2[11];
    pfOut[9] = pfM1[1]*pfM2[8] + pfM1[5]*pfM2[9] + pfM1[9]*pfM2[10] 
                    + pfM1[13]*pfM2[11];
    pfOut[10] = pfM1[2]*pfM2[8] + pfM1[6]*pfM2[9] + pfM1[10]*pfM2[10] 
                    + pfM1[14]*pfM2[11];
    pfOut[11] = pfM1[3]*pfM2[8] + pfM1[7]*pfM2[9] + pfM1[11]*pfM2[10] 
                    + pfM1[15]*pfM2[11];

    pfOut[12] = pfM1[0]*pfM2[12] + pfM1[4]*pfM2[13] + pfM1[8]*pfM2[14] 
                    + pfM1[12]*pfM2[15];
    pfOut[13] = pfM1[1]*pfM2[12] + pfM1[5]*pfM2[13] + pfM1[9]*pfM2[14] 
                    + pfM1[13]*pfM2[15];
    pfOut[14] = pfM1[2]*pfM2[12] + pfM1[6]*pfM2[13] + pfM1[10]*pfM2[14] 
                    + pfM1[14]*pfM2[15];
    pfOut[15] = pfM1[3]*pfM2[12] + pfM1[7]*pfM2[13] + pfM1[11]*pfM2[14] 
                    + pfM1[15]*pfM2[15];

	return pOut;
}


/*====================
  D3DXMatrixTranslation
  ====================*/
D3DXMATRIX*	D3DXMatrixTranslation(D3DXMATRIX *pOut, float x, float y, float z)
{
	D3DXMatrixIdentity(pOut);
	pOut->m[3][0] = x;
	pOut->m[3][1] = y;
	pOut->m[3][2] = z;
	return pOut;
}


/*====================
  D3DXMatrixScaling
  ====================*/
D3DXMATRIX*	D3DXMatrixScaling(D3DXMATRIX *pOut, float sx, float sy, float sz)
{
	D3DXMatrixIdentity(pOut);
	pOut->m[0][0] = sx;
	pOut->m[1][1] = sy;
	pOut->m[2][2] = sz;
	return pOut;
}


/*====================
  D3DXMatrixInverse
  ====================*/
D3DXMATRIX* D3DXMatrixInverse(D3DXMATRIX *pOut, float *pDeterminant, const D3DXMATRIX *pM)
{
	D3DXMATRIX M = *pM;

	pOut->_11 = M._23*M._34*M._42 - M._24*M._33*M._42 + M._24*M._32*M._43 - M._22*M._34*M._43 - M._23*M._32*M._44 + M._22*M._33*M._44;
	pOut->_12 = M._14*M._33*M._42 - M._13*M._34*M._42 - M._14*M._32*M._43 + M._12*M._34*M._43 + M._13*M._32*M._44 - M._12*M._33*M._44;
	pOut->_13 = M._13*M._24*M._42 - M._14*M._23*M._42 + M._14*M._22*M._43 - M._12*M._24*M._43 - M._13*M._22*M._44 + M._12*M._23*M._44;
	pOut->_14 = M._14*M._23*M._32 - M._13*M._24*M._32 - M._14*M._22*M._33 + M._12*M._24*M._33 + M._13*M._22*M._34 - M._12*M._23*M._34;
	pOut->_21 = M._24*M._33*M._41 - M._23*M._34*M._41 - M._24*M._31*M._43 + M._21*M._34*M._43 + M._23*M._31*M._44 - M._21*M._33*M._44;
	pOut->_22 = M._13*M._34*M._41 - M._14*M._33*M._41 + M._14*M._31*M._43 - M._11*M._34*M._43 - M._13*M._31*M._44 + M._11*M._33*M._44;
	pOut->_23 = M._14*M._23*M._41 - M._13*M._24*M._41 - M._14*M._21*M._43 + M._11*M._24*M._43 + M._13*M._21*M._44 - M._11*M._23*M._44;
	pOut->_24 = M._13*M._24*M._31 - M._14*M._23*M._31 + M._14*M._21*M._33 - M._11*M._24*M._33 - M._13*M._21*M._34 + M._11*M._23*M._34;
	pOut->_31 = M._22*M._34*M._41 - M._24*M._32*M._41 + M._24*M._31*M._42 - M._21*M._34*M._42 - M._22*M._31*M._44 + M._21*M._32*M._44;
	pOut->_32 = M._14*M._32*M._41 - M._12*M._34*M._41 - M._14*M._31*M._42 + M._11*M._34*M._42 + M._12*M._31*M._44 - M._11*M._32*M._44;
	pOut->_33 = M._12*M._24*M._41 - M._14*M._22*M._41 + M._14*M._21*M._42 - M._11*M._24*M._42 - M._12*M._21*M._44 + M._11*M._22*M._44;
	pOut->_34 = M._14*M._22*M._31 - M._12*M._24*M._31 - M._14*M._21*M._32 + M._11*M._24*M._32 + M._12*M._21*M._34 - M._11*M._22*M._34;
	pOut->_41 = M._23*M._32*M._41 - M._22*M._33*M._41 - M._23*M._31*M._42 + M._21*M._33*M._42 + M._22*M._31*M._43 - M._21*M._32*M._43;
	pOut->_42 = M._12*M._33*M._41 - M._13*M._32*M._41 + M._13*M._31*M._42 - M._11*M._33*M._42 - M._12*M._31*M._43 + M._11*M._32*M._43;
	pOut->_43 = M._13*M._22*M._41 - M._12*M._23*M._41 - M._13*M._21*M._42 + M._11*M._23*M._42 + M._12*M._21*M._43 - M._11*M._22*M._43;
	pOut->_44 = M._12*M._23*M._31 - M._13*M._22*M._31 + M._13*M._21*M._32 - M._11*M._23*M._32 - M._12*M._21*M._33 + M._11*M._22*M._33;

	float determinant =
	M._14 * M._23 * M._32 * M._41-M._13 * M._24 * M._32 * M._41-M._14 * M._22 * M._33 * M._41+M._12 * M._24    * M._33 * M._41+
	M._13 * M._22 * M._34 * M._41-M._12 * M._23 * M._34 * M._41-M._14 * M._23 * M._31 * M._42+M._13 * M._24    * M._31 * M._42+
	M._14 * M._21 * M._33 * M._42-M._11 * M._24 * M._33 * M._42-M._13 * M._21 * M._34 * M._42+M._11 * M._23    * M._34 * M._42+
	M._14 * M._22 * M._31 * M._43-M._12 * M._24 * M._31 * M._43-M._14 * M._21 * M._32 * M._43+M._11 * M._24    * M._32 * M._43+
	M._12 * M._21 * M._34 * M._43-M._11 * M._22 * M._34 * M._43-M._13 * M._22 * M._31 * M._44+M._12 * M._23    * M._31 * M._44+
	M._13 * M._21 * M._32 * M._44-M._11 * M._23 * M._32 * M._44-M._12 * M._21 * M._33 * M._44+M._11 * M._22    * M._33 * M._44;
	
	for(int x = 0; x < 4; x++)
		for(int y = 0; y < 4; y++)
			pOut->m[x][y] /= determinant;

	if(pDeterminant)
		*pDeterminant = determinant;
    return pOut;
}


/*====================
  D3DXMatrixTranspose
  ====================*/
D3DXMATRIX*	D3DXMatrixTranspose(D3DXMATRIX * pOut, const D3DXMATRIX * pM)
{
	int x, y;
	D3DXMATRIX M = *pM;
	for (x = 0; x < 4; x++)
	{
		for (y = 0; y < 4; y++)
		{
			pOut->m[x][y] = M.m[y][x];
		}
	}
	return pOut;
}


/*====================
  D3DXVec4Transform
  ====================*/
D3DXVECTOR4*	D3DXVec4Transform(D3DXVECTOR4 *pOut, const D3DXVECTOR4 *pV, const D3DXMATRIX *pM)
{
	D3DXVECTOR4 V = *pV;
	pOut->x = pM->m[0][0] * V.x + pM->m[1][0] * V.y + pM->m[2][0] * V.z + pM->m[3][0] * V.w;
	pOut->y = pM->m[0][1] * V.x + pM->m[1][1] * V.y + pM->m[2][1] * V.z + pM->m[3][1] * V.w;
	pOut->z = pM->m[0][2] * V.x + pM->m[1][2] * V.y + pM->m[2][2] * V.z + pM->m[3][2] * V.w;
	pOut->w = pM->m[0][3] * V.x + pM->m[1][3] * V.y + pM->m[2][3] * V.z + pM->m[3][3] * V.w;
	return pOut;
}


/*====================
  D3DXVec3Transform
  ====================*/
D3DXVECTOR3*	D3DXVec3Transform(D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV, const D3DXMATRIX *pM)
{
	D3DXVECTOR3 V = *pV;
	pOut->x = pM->m[0][0] * V.x + pM->m[1][0] * V.y + pM->m[2][0] * V.z + pM->m[3][0];
	pOut->y = pM->m[0][1] * V.x + pM->m[1][1] * V.y + pM->m[2][1] * V.z + pM->m[3][1];
	pOut->z = pM->m[0][2] * V.x + pM->m[1][2] * V.y + pM->m[2][2] * V.z + pM->m[3][2];
	return pOut;
}


/*====================
  D3DXVec3TransformNormal
  ====================*/
D3DXVECTOR3*	D3DXVec3TransformNormal(D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV, const D3DXMATRIX *pM)
{
	D3DXVECTOR3 V = *pV;
	pOut->x = pM->m[0][0] * V.x + pM->m[1][0] * V.y + pM->m[2][0] * V.z;
	pOut->y = pM->m[0][1] * V.x + pM->m[1][1] * V.y + pM->m[2][1] * V.z;
	pOut->z = pM->m[0][2] * V.x + pM->m[1][2] * V.y + pM->m[2][2] * V.z;
	return pOut;
}


/*====================
  D3DXMatrixOrthoOffCenterRH
  ====================*/
D3DXMATRIX*	D3DXMatrixOrthoOffCenterRH(D3DXMATRIX *pOut, float l, float r, float b, float t, float zn, float zf)
{
	D3DXMatrixIdentity(pOut);
	pOut->m[0][0] = 2.0f / (r - l);
	pOut->m[1][1] = 2.0f / (t - b);
	pOut->m[2][2] = 1.0f / (zn -zf);
	pOut->m[3][0] = -1.0f -2.0f * l / (r - l);
	pOut->m[3][1] = 1.0f + 2.0f * t / (b - t);
	pOut->m[3][2] = zn / (zn -zf);
	return pOut;
}


/*====================
  D3DXVec3Normalize
  ====================*/
D3DXVECTOR3*	D3DXVec3Normalize(D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV)
{
	float fLength(sqrt(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z));
	pOut->x = pV->x / fLength;
	pOut->y = pV->y / fLength;
	pOut->z = pV->z / fLength;

	return pOut;
}
