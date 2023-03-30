// (C)2005 S2 Games
// c_pixelshader.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_pixelshader.h"
#include "i_resourcelibrary.h"
#include "c_vid.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
IResource*  AllocPixelShader(const tstring &sPath);
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
IResourceLibrary    g_ResLibPixelShader(RES_PIXEL_SHADER, _T("Pixel Shaders"), CPixelShader::ResTypeName(), false, AllocPixelShader);
//=============================================================================

/*====================
  AllocPixelShader
  ====================*/
IResource*  AllocPixelShader(const tstring &sPath)
{
    return K2_NEW(ctx_Resources,  CPixelShader)(sPath);
}


/*====================
  CPixelShader::~CPixelShader
  ====================*/
CPixelShader::~CPixelShader()
{
    //Free();
}


/*====================
  CPixelShader::CPixelShader
  ====================*/
CPixelShader::CPixelShader(const tstring &sPath) :
IResource(sPath, TSNULL),
m_iIndex(-1),
m_iShaderFlags(0)
{
}

CPixelShader::CPixelShader(const tstring &sName, int iShaderFlags) :
IResource(TSNULL, sName),
m_iIndex(-1),
m_iShaderFlags(iShaderFlags)
{
}


/*====================
  CPixelShader::Load
  ====================*/
int     CPixelShader::Load(uint uiIgnoreFlags, const char *pData, uint uiSize)
{
    PROFILE("CPixelShader::Load");

    try
    {
        // Dedicated servers don't need pixel shader files so skip this and save some memory
        if (K2System.IsDedicatedServer() || K2System.IsServerManager())
            return false;
    
        if (!m_sPath.empty())
            Console.Res << "Loading PixelShader " << SingleQuoteStr(m_sPath) << newl;
        else if (!m_sName.empty())
            Console.Res << "Loading PixelShader " << SingleQuoteStr(m_sName) << newl;
        else
            Console.Res << "Loading Unknown PixelShader" << newl;

        Vid.RegisterPixelShader(this);
    }
    catch (CException &ex)
    {
        ex.Process(_TS("CPixelShader::Load(") + m_sName + _TS(") - "), NO_THROW);
        return RES_LOAD_FAILED;
    }

    return 0;
}


/*====================
  CPixelShader::Free
  ====================*/
void    CPixelShader::Free()
{
    Vid.UnregisterPixelShader(this);
}
