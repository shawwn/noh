// (C)2009 S2 Games
// c_modelpanel.h
//
//=============================================================================
#ifndef __C_EFFECTPANEL_H__
#define __C_EFFECTPANEL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"
#include "c_camera.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CUICmd;
class ICvar;
class CSkeleton;
class CEffectThread;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CEffectPanel
//=============================================================================
class CEffectPanel : public IWidget
{
protected:
    struct SEffect
    {
        CEffectThread   *pEffectThread;

        CVec3f          v3EffectPos;
        CVec3f          v3EffectAngles;
        float           fEffectScale;

        SEffect() :
        pEffectThread(nullptr),
        v3EffectPos(0.0f, 0.0f, 0.0f),
        v3EffectAngles(0.0f, 0.0f, 0.0f),
        fEffectScale(1.0f)
        {}
    };

    map<int, SEffect>   m_mapEffects;

    CCamera         m_Camera;

    CVec3f          m_v3SunColor;
    CVec3f          m_v3AmbientColor;
    float           m_fSunAltitude;
    float           m_fSunAzimuth;

    bool            m_bFog;
    CVec3f          m_v3FogColor;
    float           m_fFogNear;
    float           m_fFogFar;
    float           m_fFogScale;
    float           m_fFogDensity;

    bool            m_bFovY;

    float           m_fCameraNear;
    float           m_fCameraFar;

    CVec2f          m_v2SceneSize;

    void            UpdateEffect(CEffectThread *&pEffectThread, const CVec3f &v3EffectPos, const CVec3f &v3EffectAngles, float fEffectScale);

public:
    ~CEffectPanel();
    CEffectPanel(CInterface* pInterface, IWidget* pParent, const CWidgetStyle& style);

    void    RenderWidget(const CVec2f &vOrigin, float fFade);

    void    StartEffect(int iChannel, const tstring &sEffect, const CVec3f &v3EffectPos, const CVec3f &v3Color);

    void    RecalculateSize();

    virtual void        MouseUp(EButton button, const CVec2f &v2CursorPos);
    virtual void        MouseDown(EButton button, const CVec2f &v2CursorPos);
};
//=============================================================================

#endif //__C_EFFECTPANEL_H__
