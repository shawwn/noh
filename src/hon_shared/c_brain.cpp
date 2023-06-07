// (C)2008 S2 Games
// c_brain.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "hon_shared_common.h"

#include "i_unitentity.h"
#include "c_brain.h"

#include "i_behavior.h"
#include "c_bstop.h"
#include "c_bhold.h"
#include "c_bmove.h"
#include "c_battack.h"
#include "c_bability.h"
#include "c_bdoubleactivateability.h"
#include "c_bfollow.h"
#include "c_bdropitem.h"
#include "c_bgiveitem.h"
#include "c_btouch.h"
#include "c_bguard.h"
#include "c_bwander.h"
#include "c_baggressivewander.h"
#include "c_battackmove.h"
#include "c_baggro.h"
#include "c_bevent.h"
#include "c_bguardfollow.h"
#include "c_bfollowguard.h"
#include "c_bsentry.h"
#include "c_bassist.h"
#include "c_battackfollow.h"
#include "i_ActionState.h"
#include "c_asAttacking.h"
#include "c_asCasting.h"
#include "c_asMoving.h"
#include "i_waypoint.h"

#include "../k2/c_buffer.h"
//=============================================================================

/*====================
  CBrain::CBrain
  ====================*/
CBrain::CBrain() :
m_uiProcessed(0)
{
    ClearAllFlags();
    MemManager.Set(m_pActionStates, 0, sizeof(m_pActionStates));
}


/*====================
  CBrain::~CBrain
  ====================*/
CBrain::~CBrain()
{
    if (!(m_uiFlags & BS_READY))
        return;

    // Delete any active behavior
    for (BehaviorDeque::iterator it(m_Brain.begin()), itEnd(m_Brain.end()); it != itEnd; ++it)
        K2_DELETE(*it);

    SAFE_DELETE(m_pActionStates[ASID_ATTACKING]);
    SAFE_DELETE(m_pActionStates[ASID_CASTING]);
    SAFE_DELETE(m_pActionStates[ASID_MOVING]);
}


/*====================
  CBrain::CopyFrom
  ====================*/
void    CBrain::CopyFrom(const CBrain &brain)
{
    assert((m_uiFlags & BS_READY) && (brain.m_uiFlags & BS_READY));
    if (!(m_uiFlags & BS_READY) || !(brain.m_uiFlags & BS_READY))
    {
        Console.Warn << "Brain not ready." << newl;
        return;
    }

    static_cast<CASAttacking*>(m_pActionStates[ASID_ATTACKING])->CopyFrom(static_cast<CASAttacking*>(brain.m_pActionStates[ASID_ATTACKING]));
    static_cast<CASCasting*>(m_pActionStates[ASID_CASTING])->CopyFrom(static_cast<CASCasting*>(brain.m_pActionStates[ASID_CASTING]));
    static_cast<CASMoving*>(m_pActionStates[ASID_MOVING])->CopyFrom(static_cast<CASMoving*>(brain.m_pActionStates[ASID_MOVING]));

    // delete current behaviors.
    for (BehaviorDeque::iterator it = m_Brain.begin(); it != m_Brain.end(); ++it)
    {
        IBehavior* pBehavior(*it);
        pBehavior->EndBehavior();
        K2_DELETE(pBehavior);
    }
    m_Brain.clear();

    // clone incoming behaviors.
    for (BehaviorDeque::const_iterator it = brain.m_Brain.begin(); it != brain.m_Brain.end(); ++it)
    {
        const IBehavior* pBehavior(*it);
        m_Brain.push_back(pBehavior->Clone(this, m_pUnit));
    }

    // clone the command deque.
    m_Commands.clear();
    for (CommandDeque::const_iterator it = brain.m_Commands.begin(); it != brain.m_Commands.end(); ++it)
    {
        const SUnitCommand& cmd(*it);
        m_Commands.push_back(cmd);
    }

    m_uiFlags = brain.m_uiFlags;
    m_uiProcessed = brain.m_uiProcessed;
}


/*====================
  CBrain::Init
  ====================*/
void    CBrain::Init()
{
    if (m_uiFlags & BS_READY)
        return;

    m_pActionStates[ASID_ATTACKING] = K2_NEW(ctx_Game,    CASAttacking)(*this);
    m_pActionStates[ASID_CASTING] = K2_NEW(ctx_Game,    CASCasting)(*this);
    m_pActionStates[ASID_MOVING] = K2_NEW(ctx_Game,    CASMoving)(*this);

    for (int i(0); i < ASID_COUNT; ++i)
    {
        if (!m_pActionStates[i])
            return;
    }

    // Brain is ready if all states were allocated
    SetFlags(BS_READY);
}


/*====================
  CBrain::FrameThink
  ====================*/
void    CBrain::FrameThink()
{
    if (!(m_uiFlags & BS_READY))
        return;

    for (CommandDeque::iterator it(m_Commands.begin()); it != m_Commands.end(); )
    {
        SUnitCommand &cCmd(*it);

        if (!m_Brain.empty())
        {
            IBehavior *pBehavior(m_Brain.front());
            if (pBehavior->IsForced() && !cCmd.bForced)
            {
                ++it;
                continue;
            }
        }
        
        if (ProcessCommand(cCmd))
        {
            it = m_Commands.erase(it);
            break;
        }
        else
        {
            it = m_Commands.erase(it);
            continue;
        }
    }

    //
    // Setup
    //

    if (!m_Brain.empty())
    {
        IBehavior *pBehavior(m_Brain.front());

        if (pBehavior->Validate())
        {
            if (pBehavior->GetFlags() & BSR_NEW)
                pBehavior->BeginBehavior();

            if (~pBehavior->GetFlags() & BSR_NEW)
                pBehavior->ThinkFrame();
        }

        // Allow movement of next behavior in queue to process
        if (m_Brain.size() > 1 && pBehavior->GetInheritMovement())
        {
            IBehavior *pBehavior2(m_Brain[1]);

            if (pBehavior2->Validate())
            {
                if (pBehavior2->GetFlags() & BSR_NEW)
                    pBehavior2->BeginBehavior();

                if (~pBehavior2->GetFlags() & BSR_NEW)
                    pBehavior2->ThinkFrame();
            }
        }
    }
    else
    {
        SetMoving(false);
    }
}


/*====================
  CBrain::FrameMovement
  ====================*/
void    CBrain::FrameMovement()
{
    if (!(m_uiFlags & BS_READY))
        return;

    //
    // Movement
    //

    if (m_Brain.size() > 0)
    {
        IBehavior *pBehavior(m_Brain.front());

        if (pBehavior->Validate())
        {
            if (~pBehavior->GetFlags() & BSR_NEW)
                pBehavior->MovementFrame();
        }

        // Allow movement of next behavior in queue to process
        if (m_Brain.size() > 1 && pBehavior->GetInheritMovement())
        {
            IBehavior *pBehavior2(m_Brain[1]);

            if (pBehavior2->Validate())
            {
                if (~pBehavior2->GetFlags() & BSR_NEW)
                    pBehavior2->MovementFrame();
            }
        }
    }

    uint uiProcessed(0);

    for (int i(0); i < ASID_COUNT; ++i)
    {
        if (m_pActionStates[i]->GetFlags() & ASR_ACTIVE)
        {
            if (!m_pActionStates[i]->ContinueStateMovement())
                m_pActionStates[i]->EndState(3);
            else
                uiProcessed = 1;
        }
    }

    m_uiProcessed = uiProcessed;
}


/*====================
  CBrain::FrameAction
  ====================*/
void    CBrain::FrameAction()
{
    if (!(m_uiFlags & BS_READY))
        return;

    //
    // Actions
    //

    if (m_Brain.size() > 0)
    {
        IBehavior *pBehavior(m_Brain.front());

        if (pBehavior->Validate())
        {
            if (~pBehavior->GetFlags() & BSR_NEW)
                pBehavior->ActionFrame();
        }

        // Allow movement of next behavior in queue to process
        if (m_Brain.size() > 1 && pBehavior->GetInheritMovement())
        {
            IBehavior *pBehavior2(m_Brain[1]);

            if (pBehavior2->Validate())
            {
                if (~pBehavior2->GetFlags() & BSR_NEW)
                    pBehavior2->ActionFrame();
            }
        }
    }

    uint uiProcessed(0);

    for (int i(0); i < ASID_COUNT; ++i)
    {
        if (m_pActionStates[i]->GetFlags() & ASR_ACTIVE)
        {
            if (!m_pActionStates[i]->ContinueStateAction())
                m_pActionStates[i]->EndState(3);
            else
                uiProcessed = 1;
        }
    }

    m_uiProcessed = uiProcessed;
}


/*====================
  CBrain::FrameCleanup
  ====================*/
void    CBrain::FrameCleanup()
{
    if (!(m_uiFlags & BS_READY))
        return;

    //
    // Cleanup
    //

    if (m_Brain.size() > 0)
    {
        IBehavior *pBehavior(m_Brain.front());

        if (pBehavior->Validate())
        {
            if (~pBehavior->GetFlags() & BSR_NEW)
                pBehavior->CleanupFrame();
        }

        // Allow movement of next behavior in queue to process
        if (m_Brain.size() > 1 && pBehavior->GetInheritMovement())
        {
            IBehavior *pBehavior2(m_Brain[1]);

            if (pBehavior2->Validate())
            {
                if (~pBehavior2->GetFlags() & BSR_NEW)
                    pBehavior2->CleanupFrame();
            }
        }

        if (pBehavior->GetFlags() & BSR_END)
        {
            pBehavior->EndBehavior();

            // behavior may leave a lingering action to be completed (e.g. attack committed but target dead and behavior complete)
            K2_DELETE(pBehavior);
            m_Brain.pop_front();

            // Reset next behavior
            if (m_Brain.size() > 0 && m_Brain.front()->ShouldReset())
                m_Brain.front()->Reset();
        }
    }

    uint uiProcessed(0);

    for (int i(0); i < ASID_COUNT; ++i)
    {
        if (m_pActionStates[i]->GetFlags() & ASR_ACTIVE)
        {
            if (!m_pActionStates[i]->ContinueStateCleanup())
                m_pActionStates[i]->EndState(3);
            else
                uiProcessed = 1;
        }
    }

    m_uiProcessed = uiProcessed;
}


/*====================
  CBrain::ClearBrainDeque
  ====================*/
void    CBrain::ClearBrainDeque()
{
    if (!(m_uiFlags & BS_READY))
        return;

    BehaviorDeque::const_iterator cit(m_Brain.begin()), citEnd(m_Brain.end());

    // End all behaviors at the front of the queue
    if (cit != citEnd)
        (*cit)->EndBehavior();

    // Dealloc behaviors
    for (; cit != citEnd; ++cit)
        K2_DELETE(*cit);

    // Clear entries
    m_Brain.clear();
}


/*====================
  CBrain::ProcessCommand

  Return whether or not the command was processed.
  The order will be deleted no matter what, however,
  a unit is only allowed to process one command per
  frame.
  ====================*/
bool    CBrain::ProcessCommand(const SUnitCommand &cmd)
{
    if (!(m_uiFlags & BS_READY))
        return false;

    EUnitCommand eCmd(cmd.eCommandID);

    // Filter invalid commands
    if (eCmd == UNITCMD_MOVE)
    {
        if (!m_pUnit->GetIsMobile())
            return false;
    }
    else if (eCmd == UNITCMD_ATTACK)
    {
        if (!m_pUnit->GetCanAttack())
            return false;
    }
    else if (eCmd == UNITCMD_ATTACKMOVE)
    {
        if (!m_pUnit->GetIsMobile())
            return false;
        else if (!m_pUnit->GetCanAttack())
            eCmd = UNITCMD_MOVE;
    }

    IBehavior *pBehavior(nullptr);

    if (cmd.yQueue == QUEUE_NONE)
        ClearBrainDeque();
    else
    {
        // Queued commands cancel default behaviors
        if (!m_Brain.empty())
        {
            IBehavior *pBehavior(m_Brain.front());
            if (pBehavior->GetDefault())
            {
                pBehavior->EndBehavior();
                K2_DELETE(pBehavior);
                m_Brain.pop_front();
            }
        }
    }

    switch (eCmd)
    {
    case UNITCMD_STOP:
        pBehavior = K2_NEW(ctx_Game,    CBStop)();
        break;

    case UNITCMD_HOLD:
        pBehavior = K2_NEW(ctx_Game,    CBHold)(cmd.uiParam);
        break;

    case UNITCMD_MOVE:
        pBehavior = K2_NEW(ctx_Game,    CBMove)();
        break;

    case UNITCMD_ATTACK:
        pBehavior = K2_NEW(ctx_Game,    CBAttack)(m_pUnit->GetArmingSequence());
        break;

    case UNITCMD_ATTACKMOVE:
        pBehavior = K2_NEW(ctx_Game,    CBAttackMove)();
        break;

    case UNITCMD_ABILITY:
        pBehavior = K2_NEW(ctx_Game,    CBAbility)(cmd.uiParam, false);
        break;

    case UNITCMD_ABILITY2:
        pBehavior = K2_NEW(ctx_Game,    CBAbility)(cmd.uiParam, true);
        break;

    case UNITCMD_DOUBLE_ACTIVATE_ABILITY:
        pBehavior = K2_NEW(ctx_Game,    CBDoubleActivateAbility)(cmd.uiParam);
        break;

    case UNITCMD_FOLLOW:
        pBehavior = K2_NEW(ctx_Game,    CBFollow)();
        break;

    case UNITCMD_DROPITEM:
        pBehavior = K2_NEW(ctx_Game,    CBDropItem)(cmd.uiParam);
        break;

    case UNITCMD_GIVEITEM:
        pBehavior = K2_NEW(ctx_Game,    CBGiveItem)(cmd.uiParam);
        break;

    case UNITCMD_TOUCH:
        pBehavior = K2_NEW(ctx_Game,    CBTouch)();
        break;

    case UNITCMD_GUARD:
        pBehavior = K2_NEW(ctx_Game,    CBGuard)();
        break;

    case UNITCMD_WANDER:
        pBehavior = K2_NEW(ctx_Game,    CBWander)();
        break;

    case UNITCMD_AGGRESSIVEWANDER:
        pBehavior = K2_NEW(ctx_Game,    CBAggressiveWander)();
        break;

    case UNITCMD_AGGRO:
        pBehavior = K2_NEW(ctx_Game,    CBAggro)();
        break;

    case UNITCMD_EVENT:
        pBehavior = K2_NEW(ctx_Game,    CBEvent)();
        break;

    case UNITCMD_GUARDFOLLOW:
        pBehavior = K2_NEW(ctx_Game,    CBGuardFollow)();
        break;

    case UNITCMD_FOLLOWGUARD:
        pBehavior = K2_NEW(ctx_Game,    CBFollowGuard)();
        break;

    case UNITCMD_SENTRY:
        pBehavior = K2_NEW(ctx_Game,    CBSentry)();
        break;

    case UNITCMD_ASSIST:
        pBehavior = K2_NEW(ctx_Game,    CBAssist)();
        break;

    case UNITCMD_ATTACKFOLLOW:
        pBehavior = K2_NEW(ctx_Game,    CBAttackFollow)();
        break;

    default:
        Console << _T("Unrecognized Unit Command") << newl;
        break;
    };

    if (pBehavior)
    {
        pBehavior->SetBrain(this);
        pBehavior->SetIssuedClientNumber(cmd.iClientNumber);
        pBehavior->SetSelf(m_pUnit);
        pBehavior->SetGoal(cmd.v2Dest);
        pBehavior->SetTarget(cmd.uiIndex);
        pBehavior->SetDefault(cmd.bDefault);
        pBehavior->SetOrderEnt(cmd.unOrderEnt);
        pBehavior->SetLevel(cmd.uiLevel);
        pBehavior->SetShared(cmd.bShared);
        pBehavior->SetDirectPathing(cmd.bDirectPathing);

        if (cmd.bForced)
        {
            pBehavior->SetForced(true);
            pBehavior->SetForcedTime(Game.GetGameTime() + cmd.uiForcedDuration);
        }

        if (cmd.bRestricted)
        {
            pBehavior->SetRestricted(true);
        }

        pBehavior->SetValue0(cmd.fValue0);
        pBehavior->SetDelta(cmd.v2Delta);
        pBehavior->SetOrderSequence(cmd.uiOrderSequence);
        
        IUnitEntity *pTargetUnit = Game.GetUnitEntity(cmd.uiIndex);
        if (pTargetUnit != nullptr)
            pBehavior->SetTargetOrderDisjointSequence(pTargetUnit->GetOrderDisjointSequence());

        if (cmd.uiDuration != INVALID_TIME)
            pBehavior->SetEndTime(Game.GetGameTime() + cmd.uiDuration);
        else
            pBehavior->SetEndTime(INVALID_TIME);

        if (cmd.yQueue == QUEUE_BACK)
        {
            IWaypoint *pWaypoint(Game.AllocateDynamicEntity<IWaypoint>(g_waypoint));

            if (pWaypoint != nullptr)
            {
                pWaypoint->SetOwnerIndex(m_pUnit->GetIndex());
                pWaypoint->SetPosition(Game.GetTerrainPosition(cmd.v2Dest));
                pWaypoint->SetUnitIndex(cmd.uiIndex);
                pBehavior->SetWaypointUID(pWaypoint->GetUniqueID());
            }
        }

        if (cmd.yQueue == QUEUE_FRONT)
        {
            if (!m_Brain.empty())
            {
                IBehavior *pFront(m_Brain.front());
                if (pFront != nullptr && pFront->IsRestricted())
                {
                    pFront->EndBehavior();

                    SAFE_DELETE(pFront);
                    m_Brain.pop_front();
                }
            }

            m_Brain.push_front(pBehavior);
        }
        else
            m_Brain.push_back(pBehavior);

        return true;
    }

    return false;
}


/*====================
  CBrain::AddCommand
  ====================*/
void    CBrain::AddCommand(EUnitCommand eCommandID, byte yQueue, const CVec2f &v2Dest, uint uiIndex, uint uiParam, bool bDefault, int iClientNum, bool bForced, uint uiForcedDuration, bool bRestricted, ushort unOrderEnt, uint uiLevel, bool bShared, float fValue0, const CVec2f &v2Delta, uint uiOrderSequence)
{
    if (!(m_uiFlags & BS_READY))
        return;

    SUnitCommand cCmd;
    cCmd.eCommandID = eCommandID;
    cCmd.v2Dest = v2Dest;
    cCmd.iClientNumber = iClientNum;
    cCmd.uiIndex = uiIndex;
    cCmd.uiParam = uiParam;
    cCmd.yQueue = yQueue;
    cCmd.bDefault = bDefault;
    cCmd.bForced = bForced;
    cCmd.uiForcedDuration = uiForcedDuration;
    cCmd.bRestricted = bRestricted;
    cCmd.unOrderEnt = unOrderEnt;
    cCmd.uiLevel = uiLevel;
    cCmd.bShared = bShared;
    cCmd.fValue0 = fValue0;
    cCmd.v2Delta = v2Delta;
    cCmd.uiOrderSequence = uiOrderSequence;

    m_Commands.push_back(cCmd);
}


/*====================
  CBrain::AddCommand
  ====================*/
void    CBrain::AddCommand(const SUnitCommand &cCmd)
{
    if (!(m_uiFlags & BS_READY))
        return;

    m_Commands.push_back(cCmd);
}


/*====================
  CBrain::AddBehavior
  ====================*/
void    CBrain::AddBehavior(IBehavior *pBehavior, byte yQueue)
{
    if (!(m_uiFlags & BS_READY))
    {
        K2_DELETE(pBehavior);
        return;
    }

    if (yQueue == QUEUE_NONE)
        ClearBrainDeque();

    if (pBehavior)
    {
        pBehavior->SetBrain(this);
        pBehavior->SetSelf(m_pUnit);

        if (yQueue == QUEUE_FRONT)
            m_Brain.push_front(pBehavior);
        else
            m_Brain.push_back(pBehavior);
    }
}


/*====================
  CBrain::AttemptActionState
  ====================*/
IActionState*   CBrain::AttemptActionState(uint uiStateID, uint uiPriority)
{
    if (!(m_uiFlags & BS_READY))
        return nullptr;

    for (int i(0); i < ASID_COUNT; ++i)
    {
        if (m_pActionStates[i]->GetFlags() & ASR_ACTIVE)
        {
            if (uiStateID == i)
            {
                return m_pActionStates[i];
            }
            else if (!m_pActionStates[i]->EndState(uiPriority))
            {
                return m_pActionStates[i];
            }
        }
    }

    if (~m_pActionStates[uiStateID]->GetFlags() & ASR_ACTIVE)
    {
        if (!m_pActionStates[uiStateID]->BeginState())
            return nullptr;
    }

    return m_pActionStates[uiStateID];
}


/*====================
  CBrain::EndActionStates
  ====================*/
uint    CBrain::EndActionStates(uint uiPriority)
{
    if (!(m_uiFlags & BS_READY))
        return 0;

    uint uiActive(0);

    for (int i(0); i < ASID_COUNT; ++i)
    {
        if (m_pActionStates[i]->GetFlags() & ASR_ACTIVE)
        {
            if (!m_pActionStates[i]->EndState(uiPriority))
                ++uiActive;
        }
    }

    return uiActive;
}


/*====================
  CBrain::Killed
  ====================*/
void    CBrain::Killed()
{
    if (!(m_uiFlags & BS_READY))
        return;

    ClearBrainDeque();

    m_Commands.clear();
    EndActionStates(3);
    ClearAllFlags();

    SAFE_DELETE(m_pActionStates[ASID_ATTACKING]);
    SAFE_DELETE(m_pActionStates[ASID_CASTING]);
    SAFE_DELETE(m_pActionStates[ASID_MOVING]);
}


/*====================
  CBrain::Damaged
  ====================*/
void    CBrain::Damaged(IUnitEntity *pAttacker)
{
    if (!(m_uiFlags & BS_READY))
        return;

    if (m_Brain.empty())
        return;

    IBehavior *pBehavior(m_Brain.front());
    if (pBehavior != nullptr)
        pBehavior->Damaged(pAttacker);
}


/*====================
  CBrain::Assist
  ====================*/
void    CBrain::Assist(IUnitEntity *pAlly, IUnitEntity *pAttacker)
{
    if (!(m_uiFlags & BS_READY))
        return;

    if (m_Brain.empty())
        return;

    IBehavior *pBehavior(m_Brain.front());
    if (pBehavior != nullptr)
        pBehavior->Assist(pAlly, pAttacker);
}


/*====================
  CBrain::Moved
  ====================*/
void    CBrain::Moved()
{
    if (!(m_uiFlags & BS_READY))
        return;

    if (m_Brain.empty())
        return;

    IBehavior *pBehavior(m_Brain.front());
    if (pBehavior != nullptr)
        pBehavior->Moved();
}


/*====================
  CBrain::Aggro
  ====================*/
void    CBrain::Aggro(IUnitEntity *pTarget, uint uiDuration, uint uiDelay, bool bReaggroBlock)
{
    if (!(m_uiFlags & BS_READY))
        return;

    if (m_Brain.empty())
        return;

    IBehavior *pBehavior(m_Brain.front());
    if (pBehavior != nullptr)
        pBehavior->Aggro(pTarget, uiDuration, uiDelay, bReaggroBlock);
}


/*====================
  CBrain::GetCurrentAttackStateTarget
  ====================*/
uint    CBrain::GetCurrentAttackStateTarget() const
{
    if (!(m_uiFlags & BS_READY))
        return INVALID_INDEX;

    const CASAttacking *pAttackingState(static_cast<const CASAttacking *>(GetActionState(ASID_ATTACKING)));

    if (pAttackingState->GetFlags() & ASR_ACTIVE)
        return pAttackingState->GetAttackTarget();
    else
        return INVALID_INDEX;
}


/*====================
  CBrain::GetCurrentAttackBehaviorTarget
  ====================*/
uint    CBrain::GetCurrentAttackBehaviorTarget() const
{
    if (!(m_uiFlags & BS_READY))
        return INVALID_INDEX;

    if (m_Brain.empty())
        return INVALID_INDEX;

    IBehavior *pBehavior(m_Brain.front());
    if (pBehavior != nullptr)
        return pBehavior->GetAttackTarget();
    else
        return INVALID_INDEX;
}


/*====================
  CBrain::HasOrder
  ====================*/
bool    CBrain::HasOrder(uint uiOrderSequence)
{
    for (CommandDeque::iterator it(m_Commands.begin()), itEnd(m_Commands.end()); it != itEnd; ++it)
    {
        if (it->uiOrderSequence == uiOrderSequence)
            return true;
    }
    
    for (BehaviorDeque::iterator it(m_Brain.begin()), itEnd(m_Brain.end()); it != itEnd; ++it)
    {
        if ((*it)->GetOrderSequence() == uiOrderSequence)
            return true;
    }

    return false;
}

/*====================
  CBrain::IsCurrentBehaviorChanneling
  ====================*/
bool    CBrain::IsCurrentBehaviorChanneling() const
{
    if (!m_Brain.empty())
    {
        IBehavior *pBehavior(m_Brain.front());
        if (pBehavior != nullptr)
            return pBehavior->IsChanneling();
    }

    return false;
}
