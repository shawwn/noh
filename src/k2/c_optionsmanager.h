// (C)2007 S2 Games
// c_optionsmanager.h
//
//=============================================================================
#ifndef __C_OPTIONSMANAGER_H__
#define __C_OPTIONSMANAGER_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// COptionsManager
//=============================================================================
class COptionsManager
{
private:
    bool    m_bLoaded;

    bool    m_bNeedChangeVideo;
    bool    m_bNeedTextureReload;
    bool    m_bNeedShaderReload;
    bool    m_bNeedSoundRestart;
    bool    m_bNeedModelReload;

    // Video
    bool    m_bSavedVerticalSync;
    int     m_iSavedTextureFiltering;
    tstring m_sSavedAspect;
    float   m_fSavedBrightness;
    float   m_fSavedPositionSmooth;
    float   m_fSavedAngleSmooth;
    float   m_fSavedFov;

    // Graphics
    int     m_iSavedModelQuality;
    int     m_iSavedTextureSize;
    int     m_iSavedShaderQuality;
    int     m_iSavedShadowQuality;
    int     m_iSavedEffectQuality;
    bool    m_bSavedFoliage;
    bool    m_bSavedDynamicLights;

    // Sound
    float   m_fSavedMasterVolume;
    float   m_fSavedSoundEffectsVolume;
    float   m_fSavedSoundInterfaceVolume;
    float   m_fSavedMusicVolume;
    float   m_fSavedVoiceChatVolume;
    bool    m_bSavedVoiceMicMuted;
    int     m_iSavedSoundQuality;

    // Input
    float   m_fSavedMouseSensitivity;
    float   m_fSavedMeleeSmoothing;
    bool    m_bSavedMouseInvertY;

    int     m_iSavedjoyDeviceID;
    bool    m_bSavedjoyControlCursor;
    int     m_iSavedjoyCursorX;
    int     m_iSavedjoyCursorY;
    float   m_fSavedjoyCursorSpeed;
    float   m_fSavedjoySensitivityX;
    float   m_fSavedjoySensitivityY;
    float   m_fSavedjoySensitivityZ;
    float   m_fSavedjoySensitivityR;
    float   m_fSavedjoySensitivityU;
    float   m_fSavedjoySensitivityV;
    float   m_fSavedjoyGainX;
    float   m_fSavedjoyGainY;
    float   m_fSavedjoyGainZ;
    float   m_fSavedjoyGainR;
    float   m_fSavedjoyGainU;
    float   m_fSavedjoyGainV;
    float   m_fSavedjoyDeadZoneX;
    float   m_fSavedjoyDeadZoneY;
    float   m_fSavedjoyDeadZoneZ;
    float   m_fSavedjoyDeadZoneR;
    float   m_fSavedjoyDeadZoneU;
    float   m_fSavedjoyDeadZoneV;
    bool    m_bSavedjoyInvertX;
    bool    m_bSavedjoyInvertY;
    bool    m_bSavedjoyInvertZ;
    bool    m_bSavedjoyInvertR;
    bool    m_bSavedjoyInvertU;
    bool    m_bSavedjoyInvertV;

    // Voice
    bool    m_bSavedVoiceDisabled;
    bool    m_bSavedVoicePushToTalk;
    float   m_fSavedVoiceAudioDampen;
    float   m_fSavedVoiceVolume;

    void    SetTextureSizeCvars(int iTextureDetail);
    void    SetShaderQualityCvars(int iShaderQuality);
    void    SetShadowQualityCvars(int iShadowQuality);
    void    SetEffectQualityCvars(int iEffectQuality);
    void    SetSoundQualityCvars(int iSoundQuality);

    int     DetermineTextureSize();
    int     DetermineShaderQuality();
    int     DetermineShadowQuality();
    int     DetermineEffectQuality();
    int     DetermineSoundQuality();

public:
    ~COptionsManager();
    COptionsManager();

    void    Frame();
    void    LoadCurrentSettings();
    void    RestoreCurrentSettings();
    void    ResetAll();
    void    Apply();
};

extern K2_API COptionsManager g_OptionsManager;
//=============================================================================

#endif // __C_OPTIONSMANAGER_H__
