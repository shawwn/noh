// (C)2005 S2 Games
// c_image.h
//
//=============================================================================
#ifndef __C_IMAGE_H__
#define __C_IMAGE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"
//=============================================================================

//=============================================================================
// CImage
//=============================================================================
class CImage : public IWidget
{
public:
    ~CImage()   {}
    K2_API CImage(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style);

    virtual void        MouseUp(EButton button, const CVec2f &v2CursorPos);
    virtual void        MouseDown(EButton button, const CVec2f &v2CursorPos);
};
//=============================================================================

#endif //__C_IMAGE_H__
