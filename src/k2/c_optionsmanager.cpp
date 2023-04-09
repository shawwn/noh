// (C)2007 S2 Games
// c_optionsmanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_optionsmanager.h"
#include "c_uitrigger.h"
#include "c_uicmd.h"
#include "c_soundmanager.h"
#include "c_vid.h"
#include "c_input.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
COptionsManager g_OptionsManager; // the global singleton

UI_TRIGGER  (OptionsChangeVideo);

#ifdef linux
CVAR_STRING     (options_display,           "");
#endif
#ifdef __APPLE__
CVAR_INT        (options_display,           -1);
#endif
ARRAY_CVAR_UINT (options_resolution,        _T("0,0"));
CVAR_INT        (options_bpp,               0);
CVAR_INT        (options_refreshRate,       0);
CVAR_BOOL       (options_fullscreen,        false);
CVAR_BOOL       (options_exclusive,         true);
CVAR_BOOL       (options_framequeuing,      false);
CVAR_BOOL       (options_vsync,             false);
ARRAY_CVAR_UINT (options_antialiasing,      _T("0,0"));

CVAR_STRING (options_modelQuality, "high");
CVAR_INT    (options_textureSize, 0);
CVAR_INT    (options_shaderQuality, 0);
CVAR_INT    (options_shadowQuality, 0);
CVAR_INT    (options_effectQuality, 0);
CVAR_INT    (options_soundQuality, 0);
CVAR_INT    (options_sound_driver, 0);
CVAR_INT    (options_sound_recording_driver, 0);
CVAR_INT    (options_sound_numChannels, 0);
CVAR_BOOL   (options_sound_disable, false);
CVAR_STRING (options_sound_output, "");

extern CCvar<float>     scene_farClip;
extern CCvar<float>     scene_entityDrawDistance;
extern CCvar<float>     scene_foliageDrawDistance;
extern CCvar<float>     sound_masterVolume;
extern CCvar<float>     sound_sfxVolume;
extern CCvar<float>     sound_interfaceVolume;
extern CCvar<float>     sound_musicVolume;
extern CCvar<bool>      sound_voiceMicMuted;
extern CCvar<float>     sound_voiceChatVolume;
extern CCvar<int>       sound_driver;
extern CCvar<int>       sound_recording_driver;
extern CCvar<int>       sound_mixrate;
extern CCvar<bool>      sound_disable;
extern CCvar<int>       sound_maxVariations;
extern CCvar<int>       sound_resampler;
extern CCvar<int>       sound_numChannels;
extern CCvar<tstring>   sound_output;
extern CCvar<float>     input_mouseSensitivity;
extern CCvar<bool>      input_mouseInvertY;
extern CCvar<float>     cam_smoothPositionHalfLife;
extern CCvar<float>     cam_smoothAnglesHalfLife;
extern CCvar<bool>      ui_reloadInterfaces;
EXTERN_CVAR_FLOAT(cam_fov);
EXTERN_CVAR_STRING(model_quality);

EXTERN_CVAR_INT(input_joyDeviceID);

EXTERN_CVAR_FLOAT(input_joySensitivityX);
EXTERN_CVAR_FLOAT(input_joySensitivityY);
EXTERN_CVAR_FLOAT(input_joySensitivityZ);
EXTERN_CVAR_FLOAT(input_joySensitivityR);
EXTERN_CVAR_FLOAT(input_joySensitivityU);
EXTERN_CVAR_FLOAT(input_joySensitivityV);

EXTERN_CVAR_FLOAT(input_joyDeadZoneX);
EXTERN_CVAR_FLOAT(input_joyDeadZoneY);
EXTERN_CVAR_FLOAT(input_joyDeadZoneZ);
EXTERN_CVAR_FLOAT(input_joyDeadZoneR);
EXTERN_CVAR_FLOAT(input_joyDeadZoneU);
EXTERN_CVAR_FLOAT(input_joyDeadZoneV);

EXTERN_CVAR_FLOAT(input_joyGainX);
EXTERN_CVAR_FLOAT(input_joyGainY);
EXTERN_CVAR_FLOAT(input_joyGainZ);
EXTERN_CVAR_FLOAT(input_joyGainR);
EXTERN_CVAR_FLOAT(input_joyGainU);
EXTERN_CVAR_FLOAT(input_joyGainV);

EXTERN_CVAR_BOOL(input_joyInvertX);
EXTERN_CVAR_BOOL(input_joyInvertY);
EXTERN_CVAR_BOOL(input_joyInvertZ);
EXTERN_CVAR_BOOL(input_joyInvertR);
EXTERN_CVAR_BOOL(input_joyInvertU);
EXTERN_CVAR_BOOL(input_joyInvertV);

EXTERN_CVAR_BOOL(input_joyControlCursor);
EXTERN_CVAR_FLOAT(input_joyCursorSpeed);

EXTERN_CVAR_INT(input_joyCursorX);
EXTERN_CVAR_INT(input_joyCursorY);

EXTERN_CVAR_BOOL(voice_pushToTalk);
EXTERN_CVAR_BOOL(voice_disabled);
EXTERN_CVAR_FLOAT(voice_audioDampen);
EXTERN_CVAR_FLOAT(voice_volume);

UI_TRIGGER(AxisXExists);
UI_TRIGGER(AxisX);
UI_TRIGGER(AxisXPos);
UI_TRIGGER(AxisXNeg);

UI_TRIGGER(AxisYExists);
UI_TRIGGER(AxisY);
UI_TRIGGER(AxisYPos);
UI_TRIGGER(AxisYNeg);

UI_TRIGGER(AxisZExists);
UI_TRIGGER(AxisZ);
UI_TRIGGER(AxisZPos);
UI_TRIGGER(AxisZNeg);

UI_TRIGGER(AxisRExists);
UI_TRIGGER(AxisR);
UI_TRIGGER(AxisRPos);
UI_TRIGGER(AxisRNeg);

UI_TRIGGER(AxisUExists);
UI_TRIGGER(AxisU);
UI_TRIGGER(AxisUPos);
UI_TRIGGER(AxisUNeg);

UI_TRIGGER(AxisVExists);
UI_TRIGGER(AxisV);
UI_TRIGGER(AxisVPos);
UI_TRIGGER(AxisVNeg);
//=============================================================================


/*====================
  COptionsManager::~COptionsManager
  ====================*/
COptionsManager::~COptionsManager()
{
}


/*====================
  COptionsManager::COptionsManager
  ====================*/
COptionsManager::COptionsManager() :
m_bLoaded(false),
m_bNeedChangeVideo(false),
m_bNeedTextureReload(false),
m_bNeedSoundRestart(false),
m_bNeedModelReload(false)
{
}


/*====================
  COptionsManager::Frame
  ====================*/
void    COptionsManager::Frame()
{
    // Video Mode
    if (
#ifdef linux
        options_display.GetValue() != vid_display.GetValue() ||
        options_vsync.GetValue() != ICvar::GetBool(_T("gl_swapInterval")) ||
#endif
#ifdef __APPLE__
        options_display.GetValue() != vid_display.GetValue() ||
        options_exclusive != ICvar::GetBool(_T("gl_exclusive")) ||
        options_vsync.GetValue() != ICvar::GetBool(_T("gl_swapInterval")) ||
#endif
        options_resolution.GetValue() != vid_resolution.GetValue() ||
        options_bpp.GetValue() != vid_bpp.GetValue() ||
        options_refreshRate.GetValue() != vid_refreshRate.GetValue() ||
        options_fullscreen.GetValue() != vid_fullscreen.GetValue() ||
        options_antialiasing.GetValue() != vid_antialiasing.GetValue()
#ifdef _WIN32
        || options_exclusive != ICvar::GetBool(_T("d3d_exclusive"))
        || options_vsync.GetValue() != ICvar::GetBool(_T("d3d_presentInterval"))
        || options_framequeuing.GetValue() != (ICvar::GetInteger(_T("d3d_flush")) == 0)
#endif
        )
        m_bNeedChangeVideo = true;
    else
        m_bNeedChangeVideo = false;

    if (options_modelQuality.GetValue() != model_quality.GetValue())
        m_bNeedModelReload = true;
    else
        m_bNeedModelReload = false;

    if (options_textureSize.GetValue() != m_iSavedTextureSize)
        m_bNeedTextureReload = true;
    else
        m_bNeedTextureReload = false;

    if (options_shaderQuality == m_iSavedShaderQuality)
        options_shaderQuality.SetModified(false);
    if (options_shadowQuality == m_iSavedShadowQuality)
        options_shadowQuality.SetModified(false);

    if (options_sound_output.GetString() != sound_output.GetString() ||
        options_sound_driver.GetValue() != sound_driver.GetValue() || 
        options_sound_recording_driver.GetValue() != sound_recording_driver.GetValue() ||
        options_soundQuality.GetValue() != m_iSavedSoundQuality ||
        options_sound_disable.GetValue() != sound_disable.GetValue() ||
        options_sound_numChannels.GetValue() != sound_numChannels.GetValue())
        m_bNeedSoundRestart = true;
    else
        m_bNeedSoundRestart = false;

    OptionsChangeVideo.Trigger(XtoA(m_bNeedChangeVideo));

    // Joystick data
    AxisXExists.Trigger(XtoA(K2System.JoystickHasAxis(input_joyDeviceID, AXIS_JOY_X)));
    AxisX.Trigger(XtoA(Input.GetAxisState(AXIS_JOY_X)));
    AxisXPos.Trigger(XtoA(Input.IsButtonDown(BUTTON_JOY_X_POS)));
    AxisXNeg.Trigger(XtoA(Input.IsButtonDown(BUTTON_JOY_X_NEG)));

    AxisYExists.Trigger(XtoA(K2System.JoystickHasAxis(input_joyDeviceID, AXIS_JOY_Y)));
    AxisY.Trigger(XtoA(Input.GetAxisState(AXIS_JOY_Y)));
    AxisYPos.Trigger(XtoA(Input.IsButtonDown(BUTTON_JOY_Y_POS)));
    AxisYNeg.Trigger(XtoA(Input.IsButtonDown(BUTTON_JOY_Y_NEG)));

    AxisZExists.Trigger(XtoA(K2System.JoystickHasAxis(input_joyDeviceID, AXIS_JOY_Z)));
    AxisZ.Trigger(XtoA(Input.GetAxisState(AXIS_JOY_Z)));
    AxisZPos.Trigger(XtoA(Input.IsButtonDown(BUTTON_JOY_Z_POS)));
    AxisZNeg.Trigger(XtoA(Input.IsButtonDown(BUTTON_JOY_Z_NEG)));

    AxisRExists.Trigger(XtoA(K2System.JoystickHasAxis(input_joyDeviceID, AXIS_JOY_R)));
    AxisR.Trigger(XtoA(Input.GetAxisState(AXIS_JOY_R)));
    AxisRPos.Trigger(XtoA(Input.IsButtonDown(BUTTON_JOY_R_POS)));
    AxisRNeg.Trigger(XtoA(Input.IsButtonDown(BUTTON_JOY_R_NEG)));

    AxisUExists.Trigger(XtoA(K2System.JoystickHasAxis(input_joyDeviceID, AXIS_JOY_U)));
    AxisU.Trigger(XtoA(Input.GetAxisState(AXIS_JOY_U)));
    AxisUPos.Trigger(XtoA(Input.IsButtonDown(BUTTON_JOY_U_POS)));
    AxisUNeg.Trigger(XtoA(Input.IsButtonDown(BUTTON_JOY_U_NEG)));

    AxisVExists.Trigger(XtoA(K2System.JoystickHasAxis(input_joyDeviceID, AXIS_JOY_V)));
    AxisV.Trigger(XtoA(Input.GetAxisState(AXIS_JOY_V)));
    AxisVPos.Trigger(XtoA(Input.IsButtonDown(BUTTON_JOY_V_POS)));
    AxisVNeg.Trigger(XtoA(Input.IsButtonDown(BUTTON_JOY_V_NEG)));
}


/*====================
  COptionsManager::LoadCurrentSettings
  ====================*/
void    COptionsManager::LoadCurrentSettings()
{
    options_textureSize = DetermineTextureSize();
    options_shaderQuality = DetermineShaderQuality();
    options_shadowQuality = DetermineShadowQuality();
    options_effectQuality = DetermineEffectQuality();
    options_soundQuality = DetermineSoundQuality();

#ifdef linux
    options_display.Set(vid_display.GetValue());
    options_vsync = ICvar::GetBool(_T("gl_swapInterval"));
#endif
#ifdef __APPLE__
    options_display = vid_display.GetValue();
    options_bpp = vid_bpp.GetValue();
    options_exclusive = ICvar::GetBool(_T("gl_exclusive"));
    options_vsync = ICvar::GetBool(_T("gl_swapInterval"));
#endif
    options_resolution.Set(vid_resolution.GetValue());
    options_bpp = vid_bpp.GetValue();
    options_refreshRate = vid_refreshRate.GetValue();
    options_fullscreen = vid_fullscreen.GetValue();
    
#ifdef _WIN32
    options_exclusive = ICvar::GetBool(_T("d3d_exclusive"));
    options_framequeuing = ICvar::GetInteger(_T("d3d_flush")) == 0;
    options_vsync = ICvar::GetBool(_T("d3d_presentInterval"));
#endif
    options_antialiasing.Set(vid_antialiasing.GetValue());
    m_iSavedTextureFiltering = ICvar::GetInteger(_T("vid_textureFiltering"));
    m_sSavedAspect = vid_aspect;
    m_fSavedBrightness = vid_gamma;
    m_bSavedFoliage = ICvar::GetBool(_T("gfx_foliage"));
    m_bSavedDynamicLights = ICvar::GetBool(_T("vid_dynamicLights"));

    options_modelQuality = model_quality.GetValue();

    m_iSavedTextureSize = options_textureSize;
    m_iSavedShaderQuality = options_shaderQuality;
    m_iSavedShadowQuality = options_shadowQuality;
    m_iSavedEffectQuality = options_effectQuality;

    m_fSavedMasterVolume = sound_masterVolume;
    m_fSavedSoundEffectsVolume = sound_sfxVolume;
    m_fSavedSoundInterfaceVolume = sound_interfaceVolume;
    m_fSavedMusicVolume = sound_musicVolume;
    m_fSavedVoiceChatVolume = sound_voiceChatVolume;
    m_bSavedVoiceMicMuted = sound_voiceMicMuted;
    options_sound_driver = sound_driver.GetValue();
    options_sound_recording_driver = sound_recording_driver.GetValue();
    options_sound_numChannels = sound_numChannels.GetValue();
    m_iSavedSoundQuality = options_soundQuality;
    options_sound_disable = sound_disable.GetValue();
    options_sound_output = sound_output.GetString();
        
    m_fSavedMouseSensitivity = input_mouseSensitivity;
    m_bSavedMouseInvertY = input_mouseInvertY;
    m_fSavedPositionSmooth = cam_smoothPositionHalfLife;
    m_fSavedAngleSmooth = cam_smoothAnglesHalfLife;
    m_fSavedFov = cam_fov;

    m_iSavedjoyDeviceID = input_joyDeviceID;
    m_bSavedjoyControlCursor = input_joyControlCursor;
    m_iSavedjoyCursorX = input_joyCursorX;
    m_iSavedjoyCursorY = input_joyCursorY;
    m_fSavedjoyCursorSpeed = input_joyCursorSpeed;
    m_fSavedjoySensitivityX = input_joySensitivityX;
    m_fSavedjoySensitivityY = input_joySensitivityY;
    m_fSavedjoySensitivityZ = input_joySensitivityZ;
    m_fSavedjoySensitivityR = input_joySensitivityR;
    m_fSavedjoySensitivityU = input_joySensitivityU;
    m_fSavedjoySensitivityV = input_joySensitivityV;
    m_fSavedjoyGainX = input_joyGainX;
    m_fSavedjoyGainY = input_joyGainY;
    m_fSavedjoyGainZ = input_joyGainZ;
    m_fSavedjoyGainR = input_joyGainR;
    m_fSavedjoyGainU = input_joyGainU;
    m_fSavedjoyGainV = input_joyGainV;
    m_fSavedjoyDeadZoneX = input_joyDeadZoneX;
    m_fSavedjoyDeadZoneY = input_joyDeadZoneY;
    m_fSavedjoyDeadZoneZ = input_joyDeadZoneZ;
    m_fSavedjoyDeadZoneR = input_joyDeadZoneR;
    m_fSavedjoyDeadZoneU = input_joyDeadZoneU;
    m_fSavedjoyDeadZoneV = input_joyDeadZoneV;
    m_bSavedjoyInvertX = input_joyInvertX;
    m_bSavedjoyInvertY = input_joyInvertY;
    m_bSavedjoyInvertZ = input_joyInvertZ;
    m_bSavedjoyInvertR = input_joyInvertR;
    m_bSavedjoyInvertU = input_joyInvertU;
    m_bSavedjoyInvertV = input_joyInvertV;

    m_bSavedVoiceDisabled = voice_disabled;
    m_bSavedVoicePushToTalk = voice_pushToTalk;
    m_fSavedVoiceAudioDampen = voice_audioDampen;
    m_fSavedVoiceVolume = voice_volume;

    ICvar::SetModified(_T("vid_textureFiltering"), false);

    vid_aspect.SetModified(false);
    
    ICvar::SetModified(_T("vid_shaderLightingQuality"), false);
    ICvar::SetModified(_T("vid_shaderFalloffQuality"), false);
    ICvar::SetModified(_T("vid_shaderFogQuality"), false);
    ICvar::SetModified(_T("vid_shaderSmoothSelfOcclude"), false);
    
    ICvar::SetModified(_T("vid_shadows"), false);
    ICvar::SetModified(_T("vid_shadowmapSize"), false);

    options_shaderQuality.SetModified(false);
    options_shadowQuality.SetModified(false);
    options_effectQuality.SetModified(false);

    m_bLoaded = true;
}


/*====================
  COptionsManager::RestoreCurrentSettings
  ====================*/
void    COptionsManager::RestoreCurrentSettings()
{
    vid_gamma = m_fSavedBrightness;

    ICvar::SetInteger(_T("vid_textureFiltering"), m_iSavedTextureFiltering);
    ICvar::SetBool(_T("gfx_foliage"), m_bSavedFoliage);
    ICvar::SetBool(_T("vid_dynamicLights"), m_bSavedDynamicLights);

    vid_aspect = m_sSavedAspect;

    options_textureSize = m_iSavedTextureSize;
    options_shaderQuality = m_iSavedShaderQuality;
    options_shadowQuality = m_iSavedShadowQuality;
    options_effectQuality = m_iSavedEffectQuality;
    
    sound_masterVolume = m_fSavedMasterVolume;
    sound_sfxVolume = m_fSavedSoundEffectsVolume;
    sound_interfaceVolume = m_fSavedSoundInterfaceVolume;
    sound_musicVolume = m_fSavedMusicVolume;
    sound_voiceChatVolume = m_fSavedVoiceChatVolume;
    sound_voiceMicMuted = m_bSavedVoiceMicMuted;

    input_mouseSensitivity = m_fSavedMouseSensitivity;
    input_mouseInvertY = m_bSavedMouseInvertY;
    cam_smoothPositionHalfLife = m_fSavedPositionSmooth;
    cam_smoothAnglesHalfLife = m_fSavedAngleSmooth;
    cam_fov = m_fSavedFov;

    input_joyDeviceID = m_iSavedjoyDeviceID;
    input_joyControlCursor = m_bSavedjoyControlCursor;
    input_joyCursorX = m_iSavedjoyCursorX;
    input_joyCursorY = m_iSavedjoyCursorY;
    input_joyCursorSpeed = m_fSavedjoyCursorSpeed;
    input_joySensitivityX = m_fSavedjoySensitivityX;
    input_joySensitivityY = m_fSavedjoySensitivityY;
    input_joySensitivityZ = m_fSavedjoySensitivityZ;
    input_joySensitivityR = m_fSavedjoySensitivityR;
    input_joySensitivityU = m_fSavedjoySensitivityU;
    input_joySensitivityV = m_fSavedjoySensitivityV;
    input_joyGainX = m_fSavedjoyGainX;
    input_joyGainY = m_fSavedjoyGainY;
    input_joyGainZ = m_fSavedjoyGainZ;
    input_joyGainR = m_fSavedjoyGainR;
    input_joyGainU = m_fSavedjoyGainU;
    input_joyGainV = m_fSavedjoyGainV;
    input_joyDeadZoneX = m_fSavedjoyDeadZoneX;
    input_joyDeadZoneY = m_fSavedjoyDeadZoneY;
    input_joyDeadZoneZ = m_fSavedjoyDeadZoneZ;
    input_joyDeadZoneR = m_fSavedjoyDeadZoneR;
    input_joyDeadZoneU = m_fSavedjoyDeadZoneU;
    input_joyDeadZoneV = m_fSavedjoyDeadZoneV;
    input_joyInvertX = m_bSavedjoyInvertX;
    input_joyInvertY = m_bSavedjoyInvertY;
    input_joyInvertZ = m_bSavedjoyInvertZ;
    input_joyInvertR = m_bSavedjoyInvertR;
    input_joyInvertU = m_bSavedjoyInvertU;
    input_joyInvertV = m_bSavedjoyInvertV;

    voice_disabled = m_bSavedVoiceDisabled;
    voice_pushToTalk = m_bSavedVoicePushToTalk;
    voice_audioDampen = m_fSavedVoiceAudioDampen;
    voice_volume = m_fSavedVoiceVolume;

    SetTextureSizeCvars(options_textureSize);
    SetShadowQualityCvars(options_shadowQuality);
    SetShaderQualityCvars(options_shaderQuality);
    SetEffectQualityCvars(options_effectQuality);
    SetSoundQualityCvars(options_soundQuality);

    Frame();

    m_bNeedChangeVideo = false;
    m_bNeedTextureReload = false;
    m_bNeedShaderReload = false;
    m_bNeedSoundRestart = false;
    m_bNeedModelReload = false;

    LoadCurrentSettings();
}


/*====================
  COptionsManager::ResetAll
  ====================*/
void    COptionsManager::ResetAll()
{
#if 0
    ICvar::Reset(_T("vid_mode"));
    ICvar::Reset(_T("vid_fullscreen"));
#ifdef _WIN32
    ICvar::Reset(_T("d3d_exclusive"));
    ICvar::Reset(_T("d3d_presentInterval"));
#endif
    ICvar::Reset(_T("vid_multisample"));
    ICvar::Reset(_T("vid_textureFiltering"));
    ICvar::Reset(_T("vid_aspect"));
    ICvar::Reset(_T("vid_gamma"));

    ICvar::Reset(_T("vid_shaderLightingQuality"));
    ICvar::Reset(_T("vid_shaderFalloffQuality"));
    ICvar::Reset(_T("vid_shaderFogQuality"));
    ICvar::Reset(_T("vid_shaderSmoothSelfOcclude"));

    ICvar::Reset(_T("vid_shadows"));
    ICvar::Reset(_T("vid_shadowmapSize"));
    ICvar::Reset(_T("vid_shadowDrawDistance"));
    ICvar::Reset(_T("vid_shadowFalloffDistance"));

    scene_farClip.Reset();
    scene_entityDrawDistance.Reset();
    scene_foliageDrawDistance.Reset();
    
    sound_masterVolume.Reset();
    sound_sfxVolume.Reset();
    sound_interfaceVolume.Reset();
    sound_musicVolume.Reset();
    sound_voiceChatVolume.Reset();
    sound_voiceMicMuted.Reset();
    sound_driver.Reset();
    sound_recording_driver.Reset();
    sound_mixrate.Reset();
    sound_maxVariations.Reset();
    sound_resampler.Reset();
    sound_numChannels.Reset();
    sound_disable.Reset();

    input_mouseSensitivity.Reset();
    input_mouseInvertY.Reset();
    cam_smoothPositionHalfLife.Reset();
    cam_smoothAnglesHalfLife.Reset();
    cam_fov.Reset();

    input_joyDeviceID.Reset();
    input_joyControlCursor.Reset();
    input_joyCursorX.Reset();
    input_joyCursorY.Reset();
    input_joyCursorSpeed.Reset();
    input_joySensitivityX.Reset();
    input_joySensitivityY.Reset();
    input_joySensitivityZ.Reset();
    input_joySensitivityR.Reset();
    input_joySensitivityU.Reset();
    input_joySensitivityV.Reset();
    input_joyGainX.Reset();
    input_joyGainY.Reset();
    input_joyGainZ.Reset();
    input_joyGainR.Reset();
    input_joyGainU.Reset();
    input_joyGainV.Reset();
    input_joyDeadZoneX.Reset();
    input_joyDeadZoneY.Reset();
    input_joyDeadZoneZ.Reset();
    input_joyDeadZoneR.Reset();
    input_joyDeadZoneU.Reset();
    input_joyDeadZoneV.Reset();
    input_joyInvertX.Reset();
    input_joyInvertY.Reset();
    input_joyInvertZ.Reset();
    input_joyInvertR.Reset();
    input_joyInvertU.Reset();
    input_joyInvertV.Reset();

    voice_disabled.Reset();
    voice_pushToTalk.Reset();
    voice_audioDampen.Reset();
    voice_volume.Reset();

    options_modelQuality = DetermineModelQuality();
    options_textureSize = DetermineTextureSize();
    options_shaderQuality = DetermineShaderQuality();
    options_shadowQuality = DetermineShadowQuality();
    options_effectQuality = DetermineEffectQuality();
    options_soundQuality = DetermineSoundQuality();

    options_modelQuality.SetModified(false);
    options_textureSize.SetModified(false);
    options_shaderQuality.SetModified(false);
    options_shadowQuality.SetModified(false);
    options_effectQuality.SetModified(false);
    options_soundQuality.SetModified(false);
#endif
}


/*====================
  COptionsManager::Apply
  ====================*/
void    COptionsManager::Apply()
{
    Frame();

    if (options_textureSize.IsModified())
        SetTextureSizeCvars(options_textureSize);
    
    if (options_shaderQuality.IsModified())
        SetShaderQualityCvars(options_shaderQuality);

    if (options_shadowQuality.IsModified())
        SetShadowQualityCvars(options_shadowQuality);
    
    if (options_effectQuality.IsModified())
        SetEffectQualityCvars(options_effectQuality);

    if (ICvar::IsModified(_T("vid_shaderLightingQuality")) ||
        ICvar::IsModified(_T("vid_shaderFalloffQuality")) ||
        ICvar::IsModified(_T("vid_shaderFogQuality")) ||
        ICvar::IsModified(_T("vid_shaderSmoothSelfOcclude")) ||
        ICvar::IsModified(_T("vid_shadows")))
        m_bNeedShaderReload = true;

    if (m_bNeedChangeVideo)
    {
#ifdef linux
        vid_display.Set(options_display.GetValue());
        ICvar::SetInteger(_T("gl_swapInterval"), options_vsync ? 1 : 0);
#endif
#ifdef __APPLE__
        vid_display = options_display.GetValue();
        ICvar::SetBool(_T("gl_exclusive"), options_exclusive);
        ICvar::SetInteger(_T("gl_swapInterval"), options_vsync ? 1 : 0);
#endif
        vid_resolution.Set(options_resolution.GetValue());
        vid_bpp = int(options_bpp);
        vid_refreshRate = int(options_refreshRate);
        vid_fullscreen = bool(options_fullscreen);
        
#ifdef _WIN32
        ICvar::SetBool(_T("d3d_exclusive"), options_exclusive);
        ICvar::SetInteger(_T("d3d_flush"), options_framequeuing ? 0 : 1);
        ICvar::SetInteger(_T("d3d_presentInterval"), options_vsync ? 1 : 0);
#endif

        vid_antialiasing.Set(options_antialiasing.GetValue());

        Vid.ChangeMode(-1);
    }

    if (m_bNeedTextureReload)
        Console.Execute(_T("ReloadTextures"));

    if (m_bNeedShaderReload)
        Console.Execute(_T("ReloadShaders"));

    if (m_bNeedSoundRestart)
    {
        SetSoundQualityCvars(options_soundQuality);
        sound_output = options_sound_output.GetString();
        sound_driver = options_sound_driver.GetValue();
        sound_recording_driver = options_sound_recording_driver.GetValue();
        sound_numChannels = options_sound_numChannels.GetValue();
        sound_disable = options_sound_disable.GetValue();
        K2SoundManager.Restart();
    }

    if (m_bNeedModelReload)
    {
        model_quality = options_modelQuality.GetValue();

        Console.Execute(_T("ReloadModels"));
    }

    m_bNeedChangeVideo = false;
    m_bNeedTextureReload = false;
    m_bNeedShaderReload = false;
    m_bNeedSoundRestart = false;
    m_bNeedModelReload = false;
    
    options_shaderQuality.SetModified(false);
    options_shadowQuality.SetModified(false);
    options_effectQuality.SetModified(false);

    options_soundQuality.SetModified(false);

    Host.SaveConfig();
}


/*====================
  COptionsManager::SetTextureSizeCvars

  3 levels
  ====================*/
void    COptionsManager::SetTextureSizeCvars(int iTextureDetail)
{
    switch (iTextureDetail)
    {
    case 0:
        ICvar::SetInteger(_T("vid_textureDownsize"), 0);
        //ICvar::SetInteger(_T("vid_textureCompression"), true);
        break;
    case 1:
        ICvar::SetInteger(_T("vid_textureDownsize"), 1);
        //ICvar::SetInteger(_T("vid_textureCompression"), true);
        break;
    case 2:
        ICvar::SetInteger(_T("vid_textureDownsize"), 2);
        //ICvar::SetInteger(_T("vid_textureCompression"), true);
        break;
    case 3:
        ICvar::SetInteger(_T("vid_textureDownsize"), 3);
        //ICvar::SetInteger(_T("vid_textureCompression"), true);
        break;
    }
}


/*====================
  COptionsManager::SetShaderQualityCvars
  
  3 levels
  ====================*/
void    COptionsManager::SetShaderQualityCvars(int iShaderQuality)
{
    switch (iShaderQuality)
    {
    case 0:
        ICvar::SetInteger(_T("vid_shaderLightingQuality"), 0);
        ICvar::SetInteger(_T("vid_shaderFalloffQuality"), 0);
        ICvar::SetInteger(_T("vid_shaderFogQuality"), 0);
        ICvar::SetInteger(_T("vid_shaderSmoothSelfOcclude"), 1);
        break;
    case 1:
        ICvar::SetInteger(_T("vid_shaderLightingQuality"), 1);
        ICvar::SetInteger(_T("vid_shaderFalloffQuality"), 1);
        ICvar::SetInteger(_T("vid_shaderFogQuality"), 0);
        ICvar::SetInteger(_T("vid_shaderSmoothSelfOcclude"), 1);
        break;
    case 2:
        ICvar::SetInteger(_T("vid_shaderLightingQuality"), 2);
        ICvar::SetInteger(_T("vid_shaderFalloffQuality"), 1);
        ICvar::SetInteger(_T("vid_shaderFogQuality"), 1);
        ICvar::SetInteger(_T("vid_shaderSmoothSelfOcclude"), 1);
        break;
    }
}


/*====================
  COptionsManager::SetShadowQualityCvars

  5 Levels
  ====================*/
void    COptionsManager::SetShadowQualityCvars(int iShadowQuality)
{
    switch (iShadowQuality)
    {
    case 0:
        ICvar::SetBool(_T("vid_shadows"), 1);
        ICvar::SetInteger(_T("vid_shadowmapSize"), 4096);
        break;
    case 1:
        ICvar::SetBool(_T("vid_shadows"), 1);
        ICvar::SetInteger(_T("vid_shadowmapSize"), 2048);
        break;
    case 2:
        ICvar::SetBool(_T("vid_shadows"), 1);
        ICvar::SetInteger(_T("vid_shadowmapSize"), 1024);
        break;
    case 3:
        ICvar::SetBool(_T("vid_shadows"), 1);
        ICvar::SetInteger(_T("vid_shadowmapSize"), 512);
        break;
    case 4:
        ICvar::SetBool(_T("vid_shadows"), 0);
        ICvar::SetInteger(_T("vid_shadowmapSize"), 256);
        break;
    }
}


/*====================
  COptionsManager::SetEffectQualityCvars

  2 Levels
  ====================*/
void    COptionsManager::SetEffectQualityCvars(int iEffectQuality)
{
    switch (iEffectQuality)
    {
    case 0:
        ICvar::SetInteger(_T("scene_effectQuality"), 0);
        break;
    case 1:
        ICvar::SetInteger(_T("scene_effectQuality"), 1);
        break;
    }
}


/*====================
  COptionsManager::SetSoundQualityCvars
  
  4 levels
  ====================*/
void    COptionsManager::SetSoundQualityCvars(int iSoundQuality)
{
    switch (iSoundQuality)
    {
    case 0:
        sound_mixrate = 44100;
        sound_maxVariations = 16;
        sound_resampler = 2;
        break;
    case 1:
        sound_mixrate = 22050;
        sound_maxVariations = 8;
        sound_resampler = 1;
        break;
    case 2:
        sound_mixrate = 11025;
        sound_maxVariations = 2;
        sound_resampler = 1;
    }
}


/*====================
  COptionsManager::DetermineTextureSize
  ====================*/
int     COptionsManager::DetermineTextureSize()
{
    if (ICvar::GetInteger(_T("vid_textureDownsize")) == 0/* &&
        ICvar::GetBool(_T("vid_textureCompression")) == true*/)
        return 0;
    else if (ICvar::GetInteger(_T("vid_textureDownsize")) == 1/* &&
        ICvar::GetBool(_T("vid_textureCompression")) == true*/)
        return 1;
    else if (ICvar::GetInteger(_T("vid_textureDownsize")) == 2/* &&
        ICvar::GetBool(_T("vid_textureCompression")) == true*/)
        return 2;
    else if (ICvar::GetInteger(_T("vid_textureDownsize")) == 3/* &&
        ICvar::GetBool(_T("vid_textureCompression")) == true*/)
        return 3;
    else
        return -1;
}


/*====================
  COptionsManager::DetermineShaderQuality
  ====================*/
int     COptionsManager::DetermineShaderQuality()
{
    if (ICvar::GetInteger(_T("vid_shaderLightingQuality")) == 0 &&
        ICvar::GetInteger(_T("vid_shaderFalloffQuality")) == 0 &&
        ICvar::GetInteger(_T("vid_shaderFogQuality")) == 0 &&
        ICvar::GetInteger(_T("vid_shaderSmoothSelfOcclude")) == 1)
        return 0;
    else if (ICvar::GetInteger(_T("vid_shaderLightingQuality")) == 1 &&
        ICvar::GetInteger(_T("vid_shaderFalloffQuality")) == 1 &&
        ICvar::GetInteger(_T("vid_shaderFogQuality")) == 0 &&
        ICvar::GetInteger(_T("vid_shaderSmoothSelfOcclude")) == 1)
        return 1;
    else if (ICvar::GetInteger(_T("vid_shaderLightingQuality")) == 2 &&
        ICvar::GetInteger(_T("vid_shaderFalloffQuality")) == 1 &&
        ICvar::GetInteger(_T("vid_shaderFogQuality")) == 1 &&
        ICvar::GetInteger(_T("vid_shaderSmoothSelfOcclude")) == 1)
        return 2;
    else
        return -1;
}


/*====================
  COptionsManager::DetermineShadowQuality
  ====================*/
int     COptionsManager::DetermineShadowQuality()
{
    if (ICvar::GetBool(_T("vid_shadows")) == false)
        return 4;
    else if (ICvar::GetBool(_T("vid_shadows")) == true &&
        ICvar::GetInteger(_T("vid_shadowmapSize")) == 4096)
        return 0;
    else if (ICvar::GetBool(_T("vid_shadows")) == true &&
        ICvar::GetInteger(_T("vid_shadowmapSize")) == 2048)
        return 1;
    else if (ICvar::GetBool(_T("vid_shadows")) == true &&
        ICvar::GetInteger(_T("vid_shadowmapSize")) == 1024)
        return 2;
    else if (ICvar::GetBool(_T("vid_shadows")) == true &&
        ICvar::GetInteger(_T("vid_shadowmapSize")) == 512)
        return 3;
    else
        return -1;
}


/*====================
  COptionsManager::DetermineEffectQuality
  ====================*/
int     COptionsManager::DetermineEffectQuality()
{
    if (ICvar::GetInteger(_T("scene_effectQuality")) == 0)
        return 0;
    else if (ICvar::GetInteger(_T("scene_effectQuality")) == 1)
        return 1;
    else
        return -1;
}


/*====================
  COptionsManager::DetermineSoundQuality
  ====================*/
int     COptionsManager::DetermineSoundQuality()
{
    if (sound_mixrate == 44100 && sound_maxVariations == 16 && sound_resampler == 2)
        return 0;
    else if (sound_mixrate == 22050 && sound_maxVariations == 8 && sound_resampler == 1)
        return 1;
    else if (sound_mixrate == 11025 && sound_maxVariations == 2 && sound_resampler == 1)
        return 2;
    else
        return -1;
}


/*--------------------
  OptionsOpen
  --------------------*/
UI_VOID_CMD(OptionsOpen, 0)
{
    g_OptionsManager.LoadCurrentSettings();
}


/*--------------------
  OptionsFrame
  --------------------*/
UI_VOID_CMD(OptionsFrame, 0)
{
    g_OptionsManager.Frame();
}


/*--------------------
  OptionsCancel
  --------------------*/
UI_VOID_CMD(OptionsCancel, 0)
{
    g_OptionsManager.RestoreCurrentSettings();
}


/*--------------------
  OptionsApply
  --------------------*/
UI_VOID_CMD(OptionsApply, 0)
{
    g_OptionsManager.Apply();
}


/*--------------------
  OptionsResetAll
  --------------------*/
UI_VOID_CMD(OptionsResetAll, 0)
{
    g_OptionsManager.ResetAll();
}
