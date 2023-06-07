// (C)2006 S2 Games
// c_playercommander.h
//
//=============================================================================
#ifndef __C_PLAYERCOMMANDER_H__
#define __C_PLAYERCOMMANDER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_playerentity.h"
//=============================================================================

//=============================================================================
// CPlayerCommander
//=============================================================================
class CPlayerCommander : public IPlayerEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Player, Commander);

    static CCvarf   s_cvarCamZoomSpeed;
    static CCvarf   s_cvarCamMinHeight;
    static CCvarf   s_cvarCamMaxHeight;
    GAME_SHARED_API static CCvarf   s_cvarCamFOV;
    
    CCamera*    m_pCamera;
    uiset       m_setSelection;

public:
    ~CPlayerCommander();
    CPlayerCommander();

    bool                    HasAltInfo() const                      { return false; }

    void                    ReadClientSnapshot(const CClientSnapshot &snapshot);

    void                    Spawn();
    bool                    ServerFrame();

    CVec3f                  GetCameraPosition(const CVec3f &v3PlayerPos, const CVec3f &v3PlayerAngles);
    CVec3f                  GetCameraAngles(const CVec3f &v3InputAngles);
    virtual CVec3f          GetTargetPosition(float fRange, float fMinRange = 0.0f);

    GAME_SHARED_API virtual void    DrawViewBox(class CUITrigger &minimap, CCamera &camera);

    void                    SetupCamera(CCamera &camera, const CVec3f &v3InputPosition, const CVec3f &v3InputAngles);
    CSkeleton*              AllocateSkeleton()                      { return NULL; }
    void                    UpdateSkeleton(bool bPose)              {}
    bool                    AddToScene(const CVec4f &v4Color, int iFlags)   { return false; }
    void                    AttachModel()                           {}

    void                    Link()                                  {}
    void                    Unlink()                                {}

    float                   Damage(float fDamage, int iFlags, IVisualEntity *pAttacker, ushort unDamagingObjectID, bool bFeedback)  { return 0.0f; }

    GAME_SHARED_API ushort  GiveGold(ushort unGold, bool bUseTax = false, bool bUseIncomeMod = false);

    bool                    SpendMana(float fCost);
    float                   GetMana() const;
    float                   GetMaxMana() const;

    void                    Move(const CClientSnapshot &snapshot);

    CPlayerCommander*       GetAsCommander()                        { return this; }

    GAME_SHARED_API void    ZoomIn(CClientSnapshot *pSnapshot);
    GAME_SHARED_API void    ZoomOut(CClientSnapshot *pSnapshot);

    float                   GetFov() const                      { return s_cvarCamFOV; }
    const uiset&            GetSelection() const                { return m_setSelection; }

    GAME_SHARED_API void    DoCommandPositional(ECommanderOrder eCommand, const CVec3f &v3Pos);
    GAME_SHARED_API void    DoCommandEntity(ECommanderOrder eCommand, uint uiEntIndex);

    void    Copy(const IGameEntity &B);

    GAME_SHARED_API virtual bool    AIShouldTarget()            { return false; }
};
//=============================================================================
#endif //__C_PLAYERCOMMANDER_H__
