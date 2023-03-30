// (C)2010 S2 Games
// c_webimage.h
//
//=============================================================================
#ifndef __C_WEBIMAGE_H__
#define __C_WEBIMAGE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EWebImageState
{
    WEBIMAGE_BLANK = 0,
    WEBIMAGE_DOWNLOADING,
    WEBIMAGE_FINISHED
};
//=============================================================================

//=============================================================================
// CWebImage
//=============================================================================
class CWebImage : public IWidget
{
private:
    EWebImageState  m_eState;
    tstring         m_sURL;

public:
    ~CWebImage();
    K2_API CWebImage(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style);

    void    SetTextureURL(const tstring &sURL);

    virtual void    Frame(uint uiFrameLength, bool bProcessFrame);

    virtual void    MouseUp(EButton button, const CVec2f &v2CursorPos);
    virtual void    MouseDown(EButton button, const CVec2f &v2CursorPos);

    virtual void    SetTexture(const tstring &sTexture);
    virtual void    SetTexture(const tstring &sTexture, const tstring &sSuffix);
};
//=============================================================================

#endif //__C_WEBIMAGE_H__
