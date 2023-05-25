// (C)2005 S2 Games
// c_fontface.h
//
//=============================================================================
#ifndef __C_FONTFACE_H__
#define __C_FONTFACE_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_freetype.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
IResource*  AllocFontFace(const tstring &sPath);
//=============================================================================

//=============================================================================
// CFontFace
//=============================================================================
class CFontFace : public IFreeTypeResource
{
private:
    FT_Face     m_FTFace;

public:
    CFontFace(const tstring &sPath);

    K2_API  virtual uint            GetResType() const          { return RES_FONTFACE; }
    K2_API  virtual const tstring&  GetResTypeName() const      { return ResTypeName(); }
    K2_API  static const tstring&   ResTypeName()               { static tstring sTypeName(_T("{fontface}")); return sTypeName; }

    FT_Face GetFace() const { return m_FTFace; }

    int     Load(uint uiIgnoreFlags, const char *pData, uint uiSize);
    void    Free();
};
//=============================================================================

#endif //__C_FONTFACE_H__
