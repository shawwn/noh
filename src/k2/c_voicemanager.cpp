// (C)2009 S2 Games
// c_voicemanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "speex/speex.h"
#include "speex/speex_preprocess.h"
#include "c_socket.h"
#include "c_soundmanager.h"
#include "c_sample.h"
#include "c_voicemanager.h"
#include "c_voiceuser.h"
#include "c_uitrigger.h"
#include "c_uicmd.h"
#include "c_cmd.h"
#include "c_voiceserver.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
SINGLETON_INIT(CVoiceManager)
CVoiceManager& VoiceManager(*CVoiceManager::GetInstance());

CVAR_BOOLF  (voice_disabled,    false,      CVAR_SAVECONFIG);
CVAR_BOOLF  (voice_pushToTalk,  true,       CVAR_SAVECONFIG);
CVAR_FLOATR (voice_audioDampen, 0.4f,       CVAR_SAVECONFIG,    0.0f,   1.0f);
CVAR_FLOATR (voice_volume,      1.0f,       CVAR_SAVECONFIG,    0.0f,   1.0f);
#ifdef _WIN32
CVAR_FLOATR (voice_micVolume,   1.0f,       0,                  0.0f,   1.0f);
CVAR_BOOL   (voice_micBoost,    false);
#else
CVAR_FLOATR (voice_micGainDB,   0.0f,       CVAR_SAVECONFIG | CVAR_VALUERANGE,  -20.0f, 20.0f);
#endif
CVAR_FLOATR (voice_autoGainLevel,   32768.0f, CVAR_VALUERANGE, 1.0f, 32768.0f); // range speex enforces
CVAR_FLOATF (voice_micOnLevel,  20.0f,      CVAR_SAVECONFIG);
CVAR_UINTF  (voice_micOnTime,   1000,       CVAR_SAVECONFIG);


UI_TRIGGER(VoiceLevel);
//=============================================================================

/*====================
  CVoiceManager::~CVoiceManager
  ====================*/
CVoiceManager::~CVoiceManager()
{
    speex_bits_destroy(m_Bits);
    speex_encoder_destroy(m_EncoderState);

    speex_preprocess_state_destroy(m_Preprocess);

    for (VoiceUserMap_it it(m_mapVoiceUsers.begin()); it != m_mapVoiceUsers.end(); it++)
        SAFE_DELETE(it->second);

    SAFE_DELETE(m_Bits);
}


/*====================
  CVoiceManager::CVoiceManager
  ====================*/
CVoiceManager::CVoiceManager() :
m_EncoderState(nullptr),
m_Preprocess(nullptr),
m_uiFrameSize(0),
m_uiBytesPerFrame(0),
m_uiLastReadPos(0),
m_uiRecordingLength(0),
m_uiFrameNumber(256),
m_uiLastTalkTime(0),
m_yVoiceID(0),
m_sockVoice(_T("VoiceManager")),
m_bConnected(false),
m_bRecording(false),
m_bTestingVoiceLevel(false),
m_uiLastSend(INVALID_TIME),
m_bTalkPushed(false),
m_bLaneTalkPushed(false),
m_bUserTalking(false),
m_bRecievedData(false),
m_Bits(nullptr),
m_uiControlAccumulator(0),
m_iClientNum(-1)
#ifdef linux
,m_fGain(1.0f)
#endif
{
}


/*====================
  CVoiceManager::Init
  ====================*/
void    CVoiceManager::Init()
{
    int iOn(1);
    int iOff(0);
    int iSampleRate(VOICE_SAMPLE_RATE);
    float fGain(voice_autoGainLevel);

    m_Bits = K2_NEW(ctx_Voice,  SpeexBits)();

    m_sockVoice.Init(K2_SOCKET_UDP);

    speex_bits_init(m_Bits);
    m_EncoderState = speex_encoder_init(speex_lib_get_mode(SPEEX_MODEID_NB));

    speex_encoder_ctl(m_EncoderState, SPEEX_SET_SAMPLING_RATE, &iSampleRate);
    speex_encoder_ctl(m_EncoderState, SPEEX_SET_VAD, &iOn);
    speex_encoder_ctl(m_EncoderState, SPEEX_SET_DTX, &iOn);
    speex_encoder_ctl(m_EncoderState, SPEEX_GET_FRAME_SIZE, &m_uiFrameSize);

    m_Preprocess = speex_preprocess_state_init(m_uiFrameSize, VOICE_SAMPLE_RATE);

    speex_preprocess_ctl(m_Preprocess, SPEEX_PREPROCESS_SET_DENOISE, &iOff);
    speex_preprocess_ctl(m_Preprocess, SPEEX_PREPROCESS_SET_AGC, &iOn);
    speex_preprocess_ctl(m_Preprocess, SPEEX_PREPROCESS_SET_VAD, &iOn);
    speex_preprocess_ctl(m_Preprocess, SPEEX_PREPROCESS_SET_DEREVERB, &iOn);
    speex_preprocess_ctl(m_Preprocess, SPEEX_PREPROCESS_SET_AGC_LEVEL, &fGain);

    UpdateMicSettings();

    K2SoundManager.StartRecording(VOICE_SAMPLE_RATE, VOICE_SAMPLE_SIZE);

    if (K2SoundManager.GetRecordTarget() == nullptr)
    {
        m_bRecording = false;
        return;
    }

    m_bRecording = true;

    m_uiBytesPerFrame = INT_CEIL((SOUND_SAMPLE_BYTES) * float(m_uiFrameSize));
    m_uiRecordingLength = K2SoundManager.GetSampleLength(K2SoundManager.GetRecordTarget());
    m_uiLastReadPos = K2SoundManager.GetRecordingPos();
    
#ifndef _WIN32
    m_fGain = powf(10.0, voice_micGainDB/20);
#endif
}


/*====================
  CVoiceManager::Stop
  ====================*/
void    CVoiceManager::Stop()
{
    for (VoiceUserMap_it it(m_mapVoiceUsers.begin()); it != m_mapVoiceUsers.end(); it++)
        it->second->Stop();

    K2SoundManager.StopRecording();

    m_bRecording = false;
}

/*====================
  CVoiceManager::Restart
  ====================*/
void    CVoiceManager::Restart()
{
    for (VoiceUserMap_it it(m_mapVoiceUsers.begin()); it != m_mapVoiceUsers.end(); it++)
        it->second->Restart();

    K2SoundManager.StartRecording(VOICE_SAMPLE_RATE, VOICE_SAMPLE_SIZE);

    if (K2SoundManager.GetRecordTarget() == nullptr)
    {
        m_bRecording = false;
        return;
    }

    m_bRecording = true;

    m_uiBytesPerFrame = INT_CEIL((SOUND_SAMPLE_BYTES) * float(m_uiFrameSize));
    m_uiRecordingLength = K2SoundManager.GetSampleLength(K2SoundManager.GetRecordTarget());
    m_uiLastReadPos = K2SoundManager.GetRecordingPos();
}


/*====================
  CVoiceManager::Frame
  ====================*/
void    CVoiceManager::Frame()
{
#ifdef _WIN32
    if (voice_micVolume.IsModified())
    {
        K2System.SetMicVolume(voice_micVolume);
        voice_micVolume = K2System.GetMicVolume();
        voice_micVolume.SetModified(false);
    }

    if (voice_micBoost.IsModified())
    {
        K2System.SetMicBoost(voice_micBoost);
        voice_micBoost = K2System.GetMicBoost();
        voice_micBoost.SetModified(false);
    }
#else
    if (voice_micGainDB.IsModified())
    {
        m_fGain = powf(10.0f, voice_micGainDB / 20.0f);
        voice_micGainDB.SetModified(false);
    }
#endif

    if (voice_autoGainLevel.IsModified())
    {
        float fGain(voice_autoGainLevel);
        speex_preprocess_ctl(m_Preprocess,  SPEEX_PREPROCESS_SET_AGC_LEVEL, &fGain);
        speex_preprocess_ctl(m_Preprocess,  SPEEX_PREPROCESS_GET_AGC_LEVEL, &fGain);
        voice_autoGainLevel = fGain;
        voice_autoGainLevel.SetModified(false);
    }

    if (voice_disabled)
        return;

    if (voice_volume.IsModified())
    {
        K2SoundManager.SetVoiceVolume(voice_volume);
        voice_volume.SetModified(false);
    }

    if (!m_bConnected)
    {
        if (m_bRecording)
        {
            if (m_bTestingVoiceLevel)
                EncodeFrame();
            else
                m_uiLastReadPos = K2SoundManager.GetRecordingPos();
        }

        return;
    }

    if (m_bRecording)
        EncodeFrame();

    NetFrame();
    DecodeFrame();
}


/*====================
  CVoiceManager::EncodeFrame
  ====================*/
void    CVoiceManager::EncodeFrame()
{
    CSample *pSample(K2SoundManager.GetRecordTarget());

    if (pSample == nullptr)
        return;

    uint uiPos = K2SoundManager.GetRecordingPos();
    uint uiLength(0);

    if (uiPos >= m_uiLastReadPos)
        uiLength = uiPos - m_uiLastReadPos;
    else
        uiLength = (m_uiRecordingLength - m_uiLastReadPos) + uiPos;

    uint uiFramesToRead(uiLength / m_uiBytesPerFrame);

    bool bProcess(false);

    if (uiFramesToRead > 0)
        bProcess = K2SoundManager.GetSampleData(pSample, (byte *)m_pInputBuffer, uiFramesToRead * m_uiBytesPerFrame, m_uiLastReadPos);

    if (bProcess)
        ProcessFrames(uiFramesToRead);
    else
        m_uiFrameNumber += uiFramesToRead;

    m_uiLastReadPos += (m_uiBytesPerFrame * uiFramesToRead);
    m_uiLastReadPos %= m_uiRecordingLength;
}

/*====================
  CVoiceManager::ProcessFrames
  ====================*/
void    CVoiceManager::ProcessFrames(uint uiNumFrames)
{
#ifndef _WIN32
    for (uint i(0); i < uiNumFrames * m_uiBytesPerFrame; ++i)
    {
        m_pInputBuffer[i] = static_cast<short>(CLAMP(m_pInputBuffer[i] * m_fGain, float(SHRT_MIN), float(SHRT_MAX)));
    }
#endif
    
    for (uint i(0); i < uiNumFrames && i <= 255; i++)
    {
        byte yRead(0);
        int iActivity;

        iActivity = speex_preprocess_run(m_Preprocess, &m_pInputBuffer[i * m_uiFrameSize]);
        speex_bits_reset(m_Bits);
        iActivity &= speex_encode_int(m_EncoderState, &m_pInputBuffer[i * m_uiFrameSize], m_Bits);

        int iLevel(0);
        if (m_bTestingVoiceLevel || !voice_pushToTalk)
        {
            speex_preprocess_ctl(m_Preprocess, SPEEX_PREPROCESS_GET_AGC_LOUDNESS, &iLevel);
            VoiceLevel.Trigger(XtoA(MIN(iLevel * 2, 100)));
        }

        if (!iActivity || m_bTestingVoiceLevel || 
            (voice_pushToTalk && !m_bTalkPushed && !m_bLaneTalkPushed) ||
            (!voice_pushToTalk && iLevel < voice_micOnLevel && Host.GetTime() - m_uiLastTalkTime > voice_micOnTime))
        {
            m_uiFrameNumber++;
            continue;
        }

        if (voice_pushToTalk || iLevel >= voice_micOnLevel)
            m_uiLastTalkTime = Host.GetTime();

        yRead = speex_bits_write(m_Bits, m_pOutputBuffer, VOICE_OUTPUT_BUFFER_SIZE);

        if (m_packSend.GetLength() + 7 + yRead >= MAX_PACKET_SIZE)
        {
            Console.Warn << _T("Voice: Packet overflow") << newl;

            m_uiFrameNumber++;
            continue;
        }

        m_packSend << m_yVoiceID << m_uiFrameNumber << byte(m_bLaneTalkPushed ? VOICE_TARGET_SET_LANE : VOICE_TARGET_SET_TEAM) << yRead;
        m_packSend.Write(m_pOutputBuffer, yRead);

        m_uiFrameNumber++;
    }
}

/*====================
  CVoiceManager::NetFrame
  ====================*/
void    CVoiceManager::NetFrame()
{
    if (m_bConnected)
    {
        if (m_uiLastSend == INVALID_TIME)
            m_uiLastSend = Host.GetTime();

        if (Host.GetTime() - m_uiLastSend >= VOICE_PACKET_DELAY_MS)
        {
            m_uiControlAccumulator += Host.GetTime() - m_uiLastSend;

            // Once every second, send a control command at the start of the packet.
            // This indicates to the server what port we're sending/recieving on (in case of NAT) and that we're active.
            if (m_packSend.GetLength() == 0 && m_uiControlAccumulator >= 1000)
            {
                m_packSend << byte(0) << m_yVoiceID;
                m_uiControlAccumulator = 0;
            }

            if (m_packSend.GetLength() > 0)
                m_sockVoice.SendPacket(m_packSend);             

            m_uiLastSend = Host.GetTime();
            m_packSend.Clear();
        }
    }
    else
        m_packSend.Clear();

    CPacket pkt;

    while (m_sockVoice.ReceivePacket(pkt) > 0)
    {
        m_bRecievedData = true;

        while (!pkt.DoneReading())
        {
            byte yVoiceID(pkt.ReadByte());
            uint uiSequence(pkt.ReadInt());
            byte yLength(pkt.ReadByte());

            if (pkt.GetUnreadLength() < yLength || pkt.HasFaults())
            {
                Console.Warn << _T("Voice: Invalid voice packet recieved.") << newl;
                break;
            }

            CVoiceUser *pUser(GetVoiceUser(yVoiceID));

            if (pUser == nullptr)
            {
                Console.Warn << _T("Voice: Voice user ") << yVoiceID << _T(" not found.") << newl;
                pkt.Advance(yLength);
                continue;
            }

            pUser->AddFrame(uiSequence, (byte *)pkt.GetBuffer(), yLength);
            pkt.Advance(yLength);
        }
    }
}

/*====================
  CVoiceManager::DecodeFrame
  ====================*/
void    CVoiceManager::DecodeFrame()
{
    m_setTalking.clear();

    for (VoiceUserMap_it it(m_mapVoiceUsers.begin()); it != m_mapVoiceUsers.end(); it++)
    {
        it->second->DecodeFrame();
                //MikeG Fix for Mute bug
        if (it->second->IsTalking() && !it->second->IsMuted())
            m_setTalking.insert(it->second->GetClientNum());
    }

    if (m_iClientNum != -1)
    {
        uint uiTime(VOICE_TALK_DURATION);

        if (!voice_pushToTalk)
            uiTime = voice_micOnTime;

        if (Host.GetTime() - m_uiLastTalkTime < uiTime)
            m_setTalking.insert(m_iClientNum);
    }       

    if (m_setTalking.size() > 0)
    {
        if (!m_bUserTalking || voice_audioDampen.IsModified())
        {
            m_bUserTalking = true;

            K2SoundManager.SetInterfaceVolume(voice_audioDampen);
            K2SoundManager.SetSFXVolume(voice_audioDampen);
            K2SoundManager.SetMusicVolume(voice_audioDampen);

            voice_audioDampen.SetModified(false);
        }
    }
    else
    {
        m_bUserTalking = false;

        K2SoundManager.SetInterfaceVolume(1.0f);
        K2SoundManager.SetSFXVolume(1.0f);
        K2SoundManager.SetMusicVolume(1.0f);
    }
}

/*====================
  CVoiceManager::AddClient
  ====================*/
void    CVoiceManager::AddClient(uint uiClientID, byte yVoiceID)
{
    if (m_mapVoiceUsers.find(yVoiceID) != m_mapVoiceUsers.end())
        return;

    m_mapVoiceUsers.insert(VoiceUserPair(yVoiceID, K2_NEW(ctx_Voice,  CVoiceUser)(uiClientID, IsClientMuted(uiClientID))));
}

/*====================
  CVoiceManager::RemoveClient
  ====================*/
void    CVoiceManager::RemoveClient(byte yVoiceID)
{
    VoiceUserMap_it findit(m_mapVoiceUsers.find(yVoiceID));

    if (findit == m_mapVoiceUsers.end())
        return;

    SAFE_DELETE(findit->second);
    m_mapVoiceUsers.erase(findit);
}

/*====================
  CVoiceManager::Disconnect
  ====================*/
void    CVoiceManager::Disconnect()
{
    m_bConnected = false;
    m_bRecievedData = false;
    m_bUserTalking = false;
    m_uiLastSend = INVALID_TIME;
    m_uiFrameNumber = 256;
    m_uiLastTalkTime = 0;
    m_iClientNum = -1;

    m_setMuted.clear();

    for (VoiceUserMap_it it(m_mapVoiceUsers.begin()); it != m_mapVoiceUsers.end(); it++)
        SAFE_DELETE(it->second);

    m_mapVoiceUsers.clear();
}

/*====================
  CVoiceManager::Connect
  ====================*/
void    CVoiceManager::Connect(byte yVoiceID, const tstring &sAddress, ushort unPort, int iClientNum)
{
    m_yVoiceID = yVoiceID;
    m_sockVoice.SetSendAddr(sAddress, unPort);
    m_bConnected = true;
    m_iClientNum = iClientNum;
}

/*====================
  CVoiceManager::GetVoiceUser
  ====================*/
CVoiceUser* CVoiceManager::GetVoiceUser(byte yVoiceID)
{
    VoiceUserMap_it findit(m_mapVoiceUsers.find(yVoiceID));

    if (findit == m_mapVoiceUsers.end())
        return nullptr;

    return findit->second;
}

/*====================
  CVoiceManager::StartLevelTest
  ====================*/
void    CVoiceManager::StartLevelTest()
{
    m_bTestingVoiceLevel = true;
}

/*====================
  CVoiceManager::StopLevelTest
  ====================*/
void    CVoiceManager::StopLevelTest()
{
    m_bTestingVoiceLevel = false;
}

/*====================
  CVoiceManager::UpdateMicSettings
  ====================*/
void    CVoiceManager::UpdateMicSettings()
{
#ifdef _WIN32
    voice_micVolume = K2System.GetMicVolume();
    voice_micBoost = K2System.GetMicBoost();

    voice_micVolume.SetModified(false);
    voice_micBoost.SetModified(false);
#endif
}

/*====================
  CVoiceManager::MuteClient
  ====================*/
void    CVoiceManager::MuteClient(int iClientNum)
{
    m_setMuted.insert(iClientNum);

    CVoiceUser *pUser(nullptr);
    VoiceUserMap_it it(m_mapVoiceUsers.begin());

    while (it != m_mapVoiceUsers.end() && pUser == nullptr)
    {
        if (it->second->GetClientNum() != iClientNum)
        {
            it++;
            continue;
        }

        pUser = it->second;
    }

    if (pUser == nullptr)
        return;

    pUser->SetMuted(true);
}

/*====================
  CVoiceManager::UnmuteClient
  ====================*/
void    CVoiceManager::UnmuteClient(int iClientNum)
{
    m_setMuted.erase(iClientNum);

    CVoiceUser *pUser(nullptr);
    VoiceUserMap_it it(m_mapVoiceUsers.begin());

    while (it != m_mapVoiceUsers.end() && pUser == nullptr)
    {
        if (it->second->GetClientNum() != iClientNum)
        {
            it++;
            continue;
        }

        pUser = it->second;
    }

    if (pUser == nullptr)
        return;

    pUser->SetMuted(false);
}


/*--------------------
  uiVoiceStartTest
  --------------------*/
UI_VOID_CMD(VoiceStartTest, 0)
{
    VoiceManager.StartLevelTest();
}


/*--------------------
  uiVoiceStopTest
  --------------------*/
UI_VOID_CMD(VoiceStopTest, 0)
{
    VoiceManager.StopLevelTest();
}


/*--------------------
  uiUpdateMicSettings
  --------------------*/
UI_VOID_CMD(UpdateMicSettings, 0)
{
    VoiceManager.UpdateMicSettings();
}


/*--------------------
  uiVoiceMute
  --------------------*/
UI_VOID_CMD(VoiceMute, 1)
{
    VoiceManager.MuteClient(AtoI(vArgList[0]->Evaluate()));
}


/*--------------------
  uiVoiceUnmute
  --------------------*/
UI_VOID_CMD(VoiceUnmute, 1)
{
    VoiceManager.UnmuteClient(AtoI(vArgList[0]->Evaluate()));
}


/*--------------------
  uiToggleVoiceMute
  --------------------*/
UI_VOID_CMD(ToggleVoiceMute, 1)
{
    int iClientNum(AtoI(vArgList[0]->Evaluate()));

    if (VoiceManager.IsClientMuted(iClientNum))
        VoiceManager.UnmuteClient(iClientNum);
    else
        VoiceManager.MuteClient(iClientNum);
}


/*--------------------
  uiIsVoiceMuted
  --------------------*/
UI_CMD(IsVoiceMuted, 1)
{
    return XtoA(VoiceManager.IsClientMuted(AtoI(vArgList[0]->Evaluate())), true);
}
