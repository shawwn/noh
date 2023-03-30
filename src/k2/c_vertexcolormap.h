// (C)2005 S2 Games
// c_vertexcolormap.h
//
//=============================================================================
#ifndef __C_VERTEXCOLORMAP_H__
#define __C_VERTEXCOLORMAP_H__

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
// CVertexColorMap
//=============================================================================
class CVertexColorMap : public IWorldComponent
{
private:
	CVec4b*			m_pVertexColors;

public:
	~CVertexColorMap();
	CVertexColorMap(EWorldComponent eComponent);

	bool	Load(CArchive &archive, const CWorld *pWorld);
	bool	Generate(const CWorld *pWorld);
	void	Release();
	bool	Serialize(IBuffer *pBuffer);

	bool	GetRegion(const CRecti &recArea, void *pDest, int iLayer) const;
	bool	SetRegion(const CRecti &recArea, void *pSource, int iLayer);

	K2_API const CVec4b&	GetVertexColor(int x, int y);
	K2_API void	SetColor(int x, int y, CVec3b v3Color);
	K2_API void	SetColor(int x, int y, CVec4b v4Color);
};
//=============================================================================

#endif //__C_VERTEXCOLORMAP_H__
