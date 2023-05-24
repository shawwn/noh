// (C)2023 S3 Games
// c_gameclient.h
//
//=============================================================================
#ifndef __BET_C_GAMECLIENT_H__
#define __BET_C_GAMECLIENT_H__

//=============================================================================
// Headers
//=============================================================================
#include "../hon_shared/i_game.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CHostClient;
class CWorld;
class CSkeleton;
class CCamera;

namespace FMOD
{
    class Sound;
}
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define GameClient (*CGameClient::GetCurrentClientGamePointer())
//=============================================================================

//=============================================================================
// CWorldEntityEx
//=============================================================================
class CWorldEntityEx
{
private:
    CSkeleton   *m_pSkeleton = nullptr;
    vector<PoolHandle>  m_vPathBlockers;

public:
    ~CWorldEntityEx();

    CWorldEntityEx();

    CSkeleton*  GetSkeleton()                       { return m_pSkeleton; }
    void        SetSkeleton(CSkeleton *pSkeleton)   { m_pSkeleton = pSkeleton; }

    vector<PoolHandle>& GetPathBlockers()           { return m_vPathBlockers; }
};
//=============================================================================

//=============================================================================
// CGameClient
//=============================================================================
class CGameClient : public IGame
{
    CHostClient*    m_pHostClient = nullptr;
    CWorld*         m_pWorld = nullptr;

    CCamera*        m_pCamera;
    CVec3f          m_v3CamAngles;
    CVec3f          m_v3TargetCamPosition = CVec3f(0.0f, 0.0f, 0.0f);
    float           m_fCameraShift = 0.0f;

    map<uint, CWorldEntityEx>   m_mapWorldEntData;

    ResHandle       m_hMusic = INVALID_RESOURCE;
    SoundHandle     m_hMusicPlaying = INVALID_INDEX;

    void            DrawEntities();

public:
    ~CGameClient() override;
    CGameClient();

    void            SetGamePointer()                                { IGame::SetCurrentGamePointer(this); }
    static CGameClient* GetCurrentClientGamePointer()               { return static_cast<CGameClient*>(GetCurrentGamePointer()); }

    auto&           GetWorldEntData()                               { return m_mapWorldEntData; }

    float           GetFrameSeconds() const { return MsToSec(Host.GetFrameLength()); }

    CHostClient&    GetClient()             { return *m_pHostClient; }
    CWorld&         GetWorld()              { return *m_pWorld; }
    CCamera&        GetCamera()             { return *m_pCamera; }
    ResHandle       GetMusic()              { return m_hMusic; }

    bool            Init(CHostClient *pHostClient);
    bool            LoadWorld(const tstring &sWorldName);
    void            Frame();

    void            Start();

    void            UpdateCamera();
    void            CenterCamera(const CVec3f &v3Pos);
    void            SetCameraPosition(const CVec3f &v3Pos);
    void            SetCameraAngles(const CVec3f &v3Angles);
    void            ShiftCamera(float fShift)       { m_fCameraShift += fShift; }
    void            AdjustCameraPitch(float fPitch);
    void            AdjustCameraYaw(float fYaw);

    bool            GetLookAtPoint(CVec3f &v3Pos);

    ResHandle       RegisterModel(const tstring &sPath) override;
    ResHandle       RegisterEffect(const tstring &sPath) override;
    ResHandle       RegisterIcon(const tstring &sPath) override;
    void            Precache(const tstring &sName, EPrecacheScheme eScheme, const tstring &sModifier) override;
    void            Precache(ushort unType, EPrecacheScheme eScheme, const tstring &sModifier) override;
    CStateString&   GetStateString(uint uiID) override;
    CStateBlock&    GetStateBlock(uint uiID) override;
    uint            GetServerFrame() override;
    uint            GetServerTime() const override;
    uint            GetPrevServerTime() override;
    uint            GetServerFrameLength() override;
};
//=============================================================================

#endif  //__BET_C_GAMECLIENT_H__
