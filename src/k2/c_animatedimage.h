// (C)2009 S2 Games
// c_animatedimage.h
//
//=============================================================================
#ifndef __C_ANIMATEDIMAGE_H__
#define __C_ANIMATEDIMAGE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"
//=============================================================================

//=============================================================================
// CAnimatedImage
//=============================================================================
class CAnimatedImage : public IWidget
{
	vector<ResHandle>	m_vTextures;

	int					m_iMSperFrame;
	int					m_iNumFrames;
	int					m_iStartFrame;
	int					m_iNumLoopFrames;
	int					m_iLoopbackFrame;
	bool				m_bLoop;

	uint				m_uiStartTime;
	int					m_iOffsetTime;
	float				m_fSpeed;
	uint				m_uiForceLength;

	bool				ComputeAnimFrame(int iTime, int &iLoFrame, int &iHiFrame, float &fLerpAmt, uint uiForceLength);

public:
	~CAnimatedImage()	{}
	K2_API CAnimatedImage(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style);

	virtual void		MouseUp(EButton button, const CVec2f &v2CursorPos);
	virtual void		MouseDown(EButton button, const CVec2f &v2CursorPos);

	virtual void		Frame(uint uiFrameLength, bool bProcessFrame);

	virtual void		StartAnim(float fSpeed, uint uiForceLength);
};
//=============================================================================

#endif //__C_ANIMATEDIMAGE_H__
