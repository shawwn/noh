// (C)2009 S2 Games
// c_animatedimage.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_animatedimage.h"

#include "c_widgetstyle.h"
#include "c_uitextureregistry.h"
#include "c_draw2d.h"
#include "c_uicmd.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

/*====================
  CAnimatedImage::CAnimatedImage
  ====================*/
CAnimatedImage::CAnimatedImage(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style) :
IWidget(pInterface, pParent, WIDGET_ANIMATEDIMAGE, style),
m_iNumFrames(style.GetPropertyInt(_T("numframes"), 0)),
m_iStartFrame(style.GetPropertyInt(_T("startframe"), 0)),
m_iLoopbackFrame(style.GetPropertyInt(_T("loopbackframe"), 0)),
m_bLoop(style.GetPropertyBool(_T("loop"), true))
{
    float fFps(style.GetPropertyFloat(_T("fps"), 20.0f));

    m_iMSperFrame = MAX(INT_FLOOR(1000.0f / fFps), 1);

    const tstring &sTextureName(style.GetProperty(_T("texture")));

    for (int iZ(0); iZ < 10000; ++iZ)
    {
        tstring sFrame(Filename_AppendSuffix(sTextureName, XtoA(iZ, FMT_PADZERO, 4)));

        if (!UITextureRegistry.TextureExists(sFrame, 0))
            break;

        m_vTextures.push_back(INVALID_RESOURCE);

        UITextureRegistry.Register(sFrame, 0, m_vTextures.back());
    }

    int iNumTextureFrames(int(m_vTextures.size()));

    // Fix up the frames
    if (m_iStartFrame == 0 && m_iNumFrames == 0)
        m_iNumFrames = iNumTextureFrames;
    else if (m_iNumFrames == -1)
        m_iNumFrames = iNumTextureFrames - m_iStartFrame;
    else if (m_iStartFrame + m_iNumFrames > int(m_vTextures.size()))
        m_iNumFrames = iNumTextureFrames - m_iStartFrame;
    
    if (m_iNumFrames <= 0)
        m_iNumFrames = 1;

    if (m_bLoop)
    {
        if (m_iLoopbackFrame == -1)
            m_iLoopbackFrame = m_iStartFrame + m_iNumFrames - 1;

        m_iLoopbackFrame = CLAMP(m_iLoopbackFrame, m_iStartFrame, m_iStartFrame + m_iNumFrames - 1);
        m_iNumLoopFrames = m_iStartFrame + m_iNumFrames - m_iLoopbackFrame;
    }

    StartAnim(1.0f, 0);
}


/*====================
  CAnimatedImage::MouseDown
  ====================*/
void    CAnimatedImage::MouseDown(EButton button, const CVec2f &v2CursorPos)
{
    if (button == BUTTON_MOUSEL)
    {
        DO_EVENT(WEVENT_MOUSELDOWN)
        DO_EVENT(WEVENT_CLICK)
        
    }
    else if (button == BUTTON_MOUSER)
    {
        DO_EVENT(WEVENT_MOUSERDOWN)
        DO_EVENT(WEVENT_RIGHTCLICK)
    }
}


/*====================
  CAnimatedImage::MouseUp
  ====================*/
void    CAnimatedImage::MouseUp(EButton button, const CVec2f &v2CursorPos)
{
    if (button == BUTTON_MOUSEL)
    {
        DO_EVENT(WEVENT_MOUSELUP)
    }
    else if (button == BUTTON_MOUSER)
    {
        DO_EVENT(WEVENT_MOUSERUP)
    }
}


/*====================
  CAnimatedImage::ComputeAnimFrame

  computes loframe, hiframe, and lerp amounts based on a given time
  non looping animations will freeze on their last frame
  looping animations will loop back to the loopbackframe specified
  time is specified in milliseconds
  ====================*/
bool    CAnimatedImage::ComputeAnimFrame(int iTime, int &iLoFrame, int &iHiFrame, float &fLerpAmt, uint uiForceLength)
{
    int iMsPerFrame(m_iMSperFrame);
    if (uiForceLength != 0)
        iMsPerFrame = uiForceLength / m_iNumFrames;
    if (iMsPerFrame == 0)
        iMsPerFrame = 1;

    if (iTime < 0)
    {
        iLoFrame = 0;
        iHiFrame = 1;
        fLerpAmt = 0.0f;

        return true;
    }

    int iFrame(iTime / iMsPerFrame);

    if (iFrame >= m_iNumFrames - 1)
    {
        if (!m_bLoop)
        {
            iLoFrame = m_iStartFrame + m_iNumFrames - 1;
            iHiFrame = m_iStartFrame + m_iNumFrames - 1;
            fLerpAmt = 0.0f;
            return false;
        }
        else
        {
            iLoFrame = ((iFrame - (m_iLoopbackFrame - m_iStartFrame)) % m_iNumLoopFrames) + m_iLoopbackFrame;
            iHiFrame = ((iFrame - (m_iLoopbackFrame - m_iStartFrame) + 1) % m_iNumLoopFrames) + m_iLoopbackFrame;

            int iTimeLo(iFrame * iMsPerFrame);
            fLerpAmt = (iTime - iTimeLo) / float(iMsPerFrame);

            return true;
        }
    }

    iLoFrame = (iFrame % m_iNumFrames) + m_iStartFrame;
    iHiFrame = ((iFrame + 1) % m_iNumFrames) + m_iStartFrame;

    int iTimeLo(iFrame * iMsPerFrame);
    fLerpAmt = (iTime - iTimeLo) / float(iMsPerFrame);

    return true;
}


/*====================
  CAnimatedImage::Frame
  ====================*/
void    CAnimatedImage::Frame(uint uiFrameLength, bool bProcessFrame)
{
    IWidget::Frame(uiFrameLength, bProcessFrame);

    if (m_vTextures.size() == 0)
        return;

    uint uiTime(Host.GetTime());
    int iAnimTime(m_fSpeed == 0.0f ? m_iOffsetTime : INT_FLOOR((uiTime - m_uiStartTime + m_iOffsetTime) * m_fSpeed));

    int iLoFrame;
    int iHiFrame;
    float fLerp;

    bool bEnd(false);
    if (!ComputeAnimFrame(iAnimTime, iLoFrame, iHiFrame, fLerp, m_uiForceLength))
        bEnd = true;

    if (bEnd)
    {
        SetFlags(WFLAG_NO_DRAW);
        m_hTexture[0] = INVALID_RESOURCE;
    }
    else
    {
        UnsetFlags(WFLAG_NO_DRAW);
        m_hTexture[0] = m_vTextures[iLoFrame];
    }
}


/*====================
  CAnimatedImage::StartAnim
  ====================*/
void    CAnimatedImage::StartAnim(float fSpeed, uint uiForceLength)
{
    m_uiStartTime = Host.GetTime();
    m_iOffsetTime = 0;
    m_fSpeed = fSpeed;
    m_uiForceLength = uiForceLength;
}


/*--------------------
  StartAnim
  --------------------*/
UI_VOID_CMD(StartAnim, 0)
{
    if (pThis == nullptr ||
        pThis->GetType() != WIDGET_ANIMATEDIMAGE)
        return;

    static_cast<CAnimatedImage*>(pThis)->StartAnim(vArgList.size() > 0 ? AtoF(vArgList[0]->Evaluate()) : 1.0f, vArgList.size() > 1 ? AtoI(vArgList[1]->Evaluate()) : 0);
}
