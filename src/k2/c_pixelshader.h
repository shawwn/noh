// (C)2005 S2 Games
// c_pixelshader.h
//
//=============================================================================
#ifndef __C_PIXEL_SHADER_H__
#define __C_PIXEL_SHADER_H__

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

// pixel shader flags
const int PS_GUI_PRECACHE       (BIT(0));

//=============================================================================

//=============================================================================
// CPixelShader
//=============================================================================
class CPixelShader : public IResource
{
private:
    int     m_iIndex;       // internal index set by the renderer
    int     m_iShaderFlags;

public:
    K2_API ~CPixelShader();
    K2_API CPixelShader(const tstring &sPath);
    K2_API CPixelShader(const tstring &sName, int iShaderFlags);

    K2_API  virtual uint            GetResType() const          { return RES_PIXEL_SHADER; }
    K2_API  virtual const tstring&  GetResTypeName() const      { return ResTypeName(); }
    K2_API  static const tstring&   ResTypeName()               { static tstring sTypeName(_T("{pixelshader}")); return sTypeName; }

    void    SetIndex(int iIndex)    { m_iIndex = iIndex; }
    int     GetIndex()              { return m_iIndex; }

    int     GetShaderFlags() const      { return m_iShaderFlags; }

    int     Load(uint uiIgnoreFlags, const char *pData, uint uiSize);
    void    Free();
};
//=============================================================================
#endif //__C_PIXEL_SHADER_H__
