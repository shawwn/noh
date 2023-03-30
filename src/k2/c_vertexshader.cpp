// (C)2005 S2 Games
// c_vertexshader.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_vertexshader.h"
#include "i_resourcelibrary.h"
#include "c_vid.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
IResource*	AllocVertexShader(const tstring &sPath);
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
IResourceLibrary	g_ResLibVertexShader(RES_VERTEX_SHADER, _T("Vertex Shaders"), CVertexShader::ResTypeName(), false, AllocVertexShader);
//=============================================================================

/*====================
  AllocVertexShader
  ====================*/
IResource*	AllocVertexShader(const tstring &sPath)
{
	return K2_NEW(ctx_Resources,  CVertexShader)(sPath);
}


/*====================
  CVertexShader::CVertexShader
  ====================*/
CVertexShader::CVertexShader(const tstring &sPath) :
IResource(sPath, TSNULL),
m_iIndex(-1),
m_iShaderFlags(0)
{
}

CVertexShader::CVertexShader(const tstring &sName, int iShaderFlags) :
IResource(TSNULL, sName),
m_iIndex(-1),
m_iShaderFlags(iShaderFlags)
{
}


/*====================
  CVertexShader::Load
  ====================*/
int		CVertexShader::Load(uint uiIgnoreFlags, const char *pData, uint uiSize)
{
	PROFILE("CVertexShader::Load");

	try
	{	
		// Dedicated servers don't need vertex shader files so skip this and save some memory
		if (K2System.IsDedicatedServer() || K2System.IsServerManager())
			return false;
	
		if (!m_sPath.empty())
			Console.Res << "Loading VertexShader " << SingleQuoteStr(m_sPath) << newl;
		else if (!m_sName.empty())
			Console.Res << "Loading VertexShader " << SingleQuoteStr(m_sName) << newl;
		else
			Console.Res << "Loading Unknown VertexShader" << newl;

		Vid.RegisterVertexShader(this);
	}
	catch (CException &ex)
	{
		ex.Process(_TS("CVertexShader::Load(") + m_sName + _TS(") - "), NO_THROW);
		return RES_LOAD_FAILED;
	}

	return 0;
}


/*====================
  CVertexShader::Free
  ====================*/
void	CVertexShader::Free()
{
	Vid.UnregisterVertexShader(this);
}
