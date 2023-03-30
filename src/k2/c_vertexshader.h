// (C)2005 S2 Games
// c_vertexshader.h
//
//=============================================================================
#ifndef __C_VERTEXSHADER_H__
#define __C_VERTEXSHADER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_resource.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================

// vertex shader flags
const int VS_GUI_PRECACHE		(BIT(0));

//=============================================================================

//=============================================================================
// CVertexShader
//=============================================================================
class CVertexShader : public IResource
{
private:
	int		m_iIndex;		// internal index set by the renderer
	int		m_iShaderFlags;

public:
	K2_API ~CVertexShader()	{}
	K2_API CVertexShader(const tstring &sPath);
	K2_API CVertexShader(const tstring &sName, int iShaderFlags);

	K2_API	virtual uint			GetResType() const			{ return RES_VERTEX_SHADER; }
	K2_API	virtual const tstring&	GetResTypeName() const		{ return ResTypeName(); }
	K2_API	static const tstring&	ResTypeName()				{ static tstring sTypeName(_T("{vertexshader}")); return sTypeName; }

	void	SetIndex(int iIndex)	{ m_iIndex = iIndex; }
	int		GetIndex()				{ return m_iIndex; }

	int		GetShaderFlags() const		{ return m_iShaderFlags; }

	int		Load(uint uiIgnoreFlags, const char *pData, uint uiSize);
	void	Free();
};
//=============================================================================
#endif //__C_VERTEXSHADER_H__
