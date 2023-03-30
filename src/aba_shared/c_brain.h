// (C)2007 S2 Games
// c_brain.h
//
//=============================================================================
#ifndef __C_BRAIN_H__
#define __C_BRAIN_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_ActionState.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class IBehavior;
class IActionState;
class IUnitEntity;
class CBufferDynamic;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
struct SUnitCommand
{
    EUnitCommand    eCommandID;
    CVec2f          v2Dest;
    int             iClientNumber;
    uint            uiIndex;
    uint            uiParam;
    byte            yQueue;
    bool            bDefault;
    bool            bForced;
    uint            uiForcedDuration;
    bool            bRestricted;
    ushort          unOrderEnt;
    uint            uiLevel;
    bool            bShared;
    float           fValue0;
    CVec2f          v2Delta;
    uint            uiOrderSequence;
    uint            uiDuration;
    bool            bDirectPathing;

    SUnitCommand() :
    eCommandID(UNITCMD_INVALID),
    v2Dest(V2_ZERO),
    iClientNumber(-1),
    uiIndex(INVALID_INDEX),
    uiParam(uint(-1)),
    yQueue(QUEUE_NONE),
    bForced(false),
    bDefault(false),
    uiForcedDuration(INVALID_TIME),
    bRestricted(false),
    unOrderEnt(INVALID_ENT_TYPE),
    uiLevel(1),
    bShared(false),
    fValue0(0.0f),
    v2Delta(V2_ZERO),
    uiOrderSequence(uint(-1)),
    uiDuration(INVALID_TIME),
    bDirectPathing(false)
    {
    }

    SUnitCommand(EUnitCommand _eCommandID) :
    eCommandID(_eCommandID),
    v2Dest(V2_ZERO),
    iClientNumber(-1),
    uiIndex(INVALID_INDEX),
    uiParam(uint(-1)),
    yQueue(QUEUE_NONE),
    bForced(false),
    bDefault(false),
    uiForcedDuration(INVALID_TIME),
    bRestricted(false),
    unOrderEnt(INVALID_ENT_TYPE),
    uiLevel(1),
    bShared(false),
    fValue0(0.0f),
    v2Delta(V2_ZERO),
    uiOrderSequence(uint(-1)),
    uiDuration(INVALID_TIME),
    bDirectPathing(false)
    {
    }
};

typedef deque<IBehavior*>   BehaviorDeque;
typedef deque<SUnitCommand> CommandDeque;

// Brain State Flags
#define BS_READY BIT(0)
#define BS_PROCESSED BIT(1)
#define BS_MOVING BIT(2)
//=============================================================================

//=============================================================================
// CBrain
//=============================================================================
class CBrain
{
private:
    IActionState*               m_pActionStates[ASID_COUNT];
    BehaviorDeque               m_Brain;
    CommandDeque                m_Commands; // Only one unqueued command is processed per frame
    IUnitEntity*                m_pUnit;

    uint m_uiFlags;
    uint m_uiProcessed;

    void ClearBrainDeque();
    inline void SetFlags(uint uiFlags)      { m_uiFlags |= uiFlags; }
    inline void ClearFlags(uint uiFlags)    { m_uiFlags &= ~uiFlags; }
    inline void ClearAllFlags()             { m_uiFlags = 0; }

    bool    ProcessCommand(const SUnitCommand &cmd);

public:
    CBrain();
    ~CBrain();

    void    CopyFrom(const CBrain &brain);

    void    SetUnit(IUnitEntity *pUnit) { m_pUnit = pUnit; }

    GAME_SHARED_API void    Init();
    GAME_SHARED_API void    FrameThink();
    GAME_SHARED_API void    FrameMovement();
    GAME_SHARED_API void    FrameAction();
    GAME_SHARED_API void    FrameCleanup();
    GAME_SHARED_API void    Killed();
    GAME_SHARED_API void    Damaged(IUnitEntity *pAttacker);
    GAME_SHARED_API void    Assist(IUnitEntity *pAlly, IUnitEntity *pAttacker);
    GAME_SHARED_API void    Moved();
    GAME_SHARED_API void    Aggro(IUnitEntity *pTarget, uint uiDuration, uint uiDelay, bool bReaggroBlock);

    GAME_SHARED_API void    AddCommand(EUnitCommand eCommandID, byte yQueue, const CVec2f &v2Dest, uint uiIndex, uint uiParam = uint(-1), bool bDefault = false, int iClientNum = -1, bool bForced = false, uint uiForcedDuration = INVALID_TIME, bool bRestricted = false, ushort unOrderEnt = INVALID_ENT_TYPE, uint uiLevel = 1, bool bShared = false, float fValue0 = 0.0f, const CVec2f &v2Delta = V2_ZERO, uint uiOrderSequence = uint(-1));
    GAME_SHARED_API void    AddCommand(const SUnitCommand &cCmd);
    GAME_SHARED_API void    AddBehavior(IBehavior *pBehavior, byte yQueue);

    // State Manipulation
    IActionState*       GetActionState(eActionStateIDs eASID)       { return m_pActionStates[eASID]; }
    const IActionState* GetActionState(eActionStateIDs eASID) const { return m_pActionStates[eASID]; }

    IActionState*   AttemptActionState(uint uiStateID, uint uiPriority);
    uint            EndActionStates(uint uiPriority);
    
    IUnitEntity*    GetUnit()   { return m_pUnit; }

    void            SetMoving(bool bMoving)     { if (bMoving) m_uiFlags |= BS_MOVING; else m_uiFlags &= ~BS_MOVING; }
    bool            GetMoving() const           { return (m_uiFlags & BS_MOVING) != 0; }
    
    bool            IsEmpty() const             { return m_Brain.size() == 0; }
    uint            GetBehaviorsPending() const { if (m_Brain.size() > 0) return uint(m_Brain.size() - 1); else return 0; }
    uint            GetCommandsPending() const  { return uint(m_Commands.size()); }
    IBehavior*      GetCurrentBehavior() const  { if (m_Brain.size() > 0) return m_Brain.front(); else return NULL; }

    uint            GetCurrentAttackStateTarget() const;
    uint            GetCurrentAttackBehaviorTarget() const;

    GAME_SHARED_API bool    HasOrder(uint uiOrderSequence);

    GAME_SHARED_API bool    IsCurrentBehaviorChanneling() const;
};
//=============================================================================

#endif // __C_BRAIN_H__
