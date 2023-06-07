// (C)2006 S2 Games
// c_clientcommander.h
//
//=============================================================================
#ifndef __C_CLIENTCOMMANDER_H__
#define __C_CLIENTCOMMANDER_H__

//=============================================================================
// Headers
//=============================================================================
#include "../game_shared/c_playercommander.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CPlayerCommander;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum ECommanderState
{
    COMSTATE_HOVER = 0,
    COMSTATE_SELECT,
    COMSTATE_BUILD
};

const int MAX_PLAYER_WAYPOINTS(8);
//=============================================================================

//=============================================================================
// CClientCommander
//=============================================================================
class CClientCommander
{
private:
    CPlayerCommander*       m_pPlayerCommander;
    CCamera*                m_pCamera;
    CVec2f                  m_v2Cursor;

    bool                    m_bModifier1;
    bool                    m_bModifier2;
    bool                    m_bModifier3;

    ECommanderState         m_eState;
    ECommanderOrder         m_eOrder;
    CVec3f                  m_v3StartCursorPos;
    CVec3f                  m_v3TraceEndPos;
    uint                    m_uiHoverEnt;
    uiset                   m_setSelection;
    uiset                   m_setHoverSelection;

    uint                    m_uiLastSelectTime;

    bool                    m_bOverrideSnapcast;

    // UI feedback sounds
    ResHandle               m_hSelectedSound;
    ResHandle               m_hMultiSelectedSound;
    ResHandle               m_hHoverSound;

    struct SSelectedPlayerWaypoint
    {
        bool    bActive;
        byte    yOrder;
        uint    uiOrderEntIndex;
        CVec3f  v3OrderPos;
        uint    uiEvent;
    
        SSelectedPlayerWaypoint() :
        bActive(false),
        yOrder(CMDR_ORDER_CLEAR),
        uiOrderEntIndex(INVALID_INDEX),
        v3OrderPos(V3_ZERO),
        uiEvent(INVALID_INDEX)
        {
        }
    };

    SSelectedPlayerWaypoint     m_aSelectedPlayerWaypoints[MAX_PLAYER_WAYPOINTS];

    CRectf      GetSelectionRect();
    CRectf      GetWorldSelection();
    void        UpdateCursorTrace();
    void        UpdateHoverSelection();

    bool        IsSelectionActive()             { return m_eState == COMSTATE_SELECT; }

    void        StartSelect();
    void        ApplySelect();

    void        UpdateSelectedPlayerWaypoints();

public:
    ~CClientCommander();
    CClientCommander();

    void    SetOverrideSnapcast(bool bOverride)                     { m_bOverrideSnapcast = bOverride; }
    bool    GetOverideSnapscast() const                             { return m_bOverrideSnapcast; }

    void    LoadResources(ResHandle hClientSoundsStringTable);

    void    SetPlayerCommander(CPlayerCommander *pPlayerCommander)  { m_pPlayerCommander = pPlayerCommander; }
    void    SetCamera(CCamera *pCamera)                             { m_pCamera = pCamera; }

    void    PrepareClientSnapshot(CClientSnapshot &snapshot);

    ECommanderState         GetCommanderState() const               { return m_eState; }
    CVec3f                  GetTracePosition() const                { return m_v3TraceEndPos; }
    float                   GetCameraDistance() const;

    void    Spawn();
    void    Frame();
    void    Draw();
    
    // Actions
    void    PrimaryDown();
    void    PrimaryUp();
    void    SecondaryDown();
    void    SecondaryUp();
    bool    Cancel();

    void    SetModifier1(bool bValue)   { m_bModifier1 = bValue; }
    void    SetModifier2(bool bValue)   { m_bModifier2 = bValue; }
    void    SetModifier3(bool bValue)   { m_bModifier3 = bValue; }

    bool    GetModifier1()              { return m_bModifier1; }
    bool    GetModifier2()              { return m_bModifier2; }
    bool    GetModifier3()              { return m_bModifier3; }

    bool    IsSelected(uint uiIndex)            { return m_setSelection.find(uiIndex) != m_setSelection.end(); }
    bool    IsHoverSelected(uint uiIndex)       { return m_setHoverSelection.find(uiIndex) != m_setHoverSelection.end(); }

    void    SetCursorPos(const CVec2f &v2Pos)   { m_v2Cursor = v2Pos; }

    void    GiveOrder(ECommanderOrder eOrder, uint uiTargetIndex);
    void    GiveOrder(ECommanderOrder eOrder, const CVec3f &v3Pos);
    void    StartCommand();
    void    Ping();

    // Building
    void    StartBuilding()                     { Cancel(); m_eState = COMSTATE_BUILD; }
    bool    StopBuilding()                      { if (m_eState == COMSTATE_HOVER) return false; m_eState = COMSTATE_HOVER; return true;}

    uint    GetSelectedEntity();
    void    ClearSelection()                    { m_setSelection.clear(); }
    void    SelectEntity(uint uiIndex, bool bClear = true);
    void    SelectSquad(uint uiSquad, bool bClear = true);

    const uiset&    GetSelectedEntities() const { return m_setSelection; }
};
//=============================================================================

#endif //__C_PLAYERCOMMANDER_H__
