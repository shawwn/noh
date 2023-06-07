// (C)2006 S2 Games
// c_clientcommander.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_client_common.h"

#include "c_gameclient.h"
#include "c_clientcommander.h"

#include "../game_shared/c_teaminfo.h"

#include "../k2/c_camera.h"
#include "../k2/c_clientsnapshot.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_uitrigger.h"
#include "../k2/c_draw2d.h"
#include "../k2/c_worldentity.h"
#include "../k2/c_input.h"
#include "../k2/c_vid.h"
#include "../k2/c_soundmanager.h"
#include "../k2/c_sample.h"
#include "../k2/c_stringtable.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
UI_TRIGGER(CommanderSelection);

CVAR_UINTF( cg_cmdrDoubleSelectTime,    200,    CVAR_SAVECONFIG);
CVAR_FLOATF(cg_cmdrTransparentWidth,    400.0f, CVAR_SAVECONFIG);
//=============================================================================

/*====================
  CClientCommander::~CClientCommander
  ====================*/
CClientCommander::~CClientCommander()
{
}


/*====================
  CClientCommander::CClientCommander
  ====================*/
CClientCommander::CClientCommander() :
m_pCamera(NULL),
m_bModifier1(false),
m_bModifier2(false),
m_bModifier3(false),
m_eState(COMSTATE_HOVER),
m_eOrder(CMDR_ORDER_AUTO),
m_v3TraceEndPos(V3_ZERO),
m_uiHoverEnt(INVALID_INDEX),

m_uiLastSelectTime(0),

m_bOverrideSnapcast(false),

m_hSelectedSound(INVALID_RESOURCE),
m_hMultiSelectedSound(INVALID_RESOURCE),
m_hHoverSound(INVALID_RESOURCE)
{
}


/*====================
  CClientCommander::LoadResources
  ====================*/
void    CClientCommander::LoadResources(ResHandle hClientSoundsStringTable)
{
    CStringTable *pClientSounds(g_ResourceManager.GetStringTable(hClientSoundsStringTable));

    m_hSelectedSound = g_ResourceManager.Register(K2_NEW(global,   CSample)(pClientSounds->Get(_T("commander_select")), SND_2D), RES_SAMPLE);
    m_hMultiSelectedSound = g_ResourceManager.Register(K2_NEW(global,   CSample)(pClientSounds->Get(_T("commander_multiselect")), SND_2D), RES_SAMPLE);
    m_hHoverSound = g_ResourceManager.Register(K2_NEW(global,   CSample)(pClientSounds->Get(_T("commander_mouseover")), SND_2D), RES_SAMPLE);
}


/*====================
  CClientCommander::PrepareClientSnapshot
  ====================*/
void    CClientCommander::PrepareClientSnapshot(CClientSnapshot &snapshot)
{
    float fFrameTime(MsToSec(GameClient.GetFrameLength()));

    snapshot.SetButton(GAME_CMDR_BUTTON_EDGESCROLL_LEFT, Input.GetCursorPos().x == 0);
    snapshot.SetButton(GAME_CMDR_BUTTON_EDGESCROLL_UP, Input.GetCursorPos().y == 0);
    snapshot.SetButton(GAME_CMDR_BUTTON_EDGESCROLL_RIGHT, Input.GetCursorPos().x == Vid.GetScreenW() - 1);
    snapshot.SetButton(GAME_CMDR_BUTTON_EDGESCROLL_DOWN, Input.GetCursorPos().y == Vid.GetScreenH() - 1);

    CVec2f v2Cursor(Input.GetCursorPos());
    v2Cursor.x /= Vid.GetScreenW();
    v2Cursor.y /= Vid.GetScreenH();
    snapshot.SetCursorPosition(v2Cursor);

    CVec3f v3Position(snapshot.GetCameraPosition());
    
    CVec3f v3Velocity(V3_ZERO);
    if (snapshot.IsButtonDown(GAME_BUTTON_FORWARD | GAME_CMDR_BUTTON_EDGESCROLL_UP))
        v3Velocity += CVec3f(0.0f, 1.0f, 0.0f);
    if (snapshot.IsButtonDown(GAME_BUTTON_BACK | GAME_CMDR_BUTTON_EDGESCROLL_DOWN))
        v3Velocity -= CVec3f(0.0f, 1.0f, 0.0f);
    if (snapshot.IsButtonDown(GAME_BUTTON_RIGHT | GAME_CMDR_BUTTON_EDGESCROLL_RIGHT))
        v3Velocity += CVec3f(1.0f, 0.0f, 0.0f);
    if (snapshot.IsButtonDown(GAME_BUTTON_LEFT | GAME_CMDR_BUTTON_EDGESCROLL_LEFT))
        v3Velocity -= CVec3f(1.0f, 0.0f, 0.0f);
    v3Velocity.Normalize();
    v3Velocity *= m_pPlayerCommander->GetSpeed();
    v3Position += v3Velocity * fFrameTime;

    // HACK: using a temp camera to calculate fovy
    CCamera camera;
    camera.SetWidth(float(Vid.GetScreenW()));
    camera.SetHeight(float(Vid.GetScreenH()));
    camera.SetFovXCalc(m_pPlayerCommander->GetFov());

    // FIXME: This works pretty well, but is imperfect
    float fWidth(tan(DEG2RAD(m_pPlayerCommander->GetFov() / 2.0f)) * v3Position.z * (1.0f + tan(DEG2RAD(m_pPlayerCommander->GetCamPitch() + 90.0f))));
    float fTop(tan(DEG2RAD(camera.GetFovY() / 2.0f)) * v3Position.z * (1.0f + tan(DEG2RAD(m_pPlayerCommander->GetCamPitch() + 90.0f))));
    float fBottom(tan(DEG2RAD(camera.GetFovY() / 2.0f)) * v3Position.z * (1.0f - tan(DEG2RAD(m_pPlayerCommander->GetCamPitch() + 90.0f))));
    if (v3Position.x < fWidth)
        v3Position.x = fWidth;
    if (v3Position.x > Game.GetWorldWidth() - fWidth)
        v3Position.x = Game.GetWorldWidth() - fWidth;
    if (v3Position.y > Game.GetWorldHeight() - fTop)
        v3Position.y = Game.GetWorldHeight() - fTop;
    if (v3Position.y < fBottom)
        v3Position.y = fBottom;

    snapshot.SetCameraPosition(v3Position);

    snapshot.SetSelection(m_setSelection);

    if (m_pPlayerCommander->GetCurrentItem() != NULL)
    {
        if (!m_pPlayerCommander->GetCurrentItem()->IsReady() || m_pPlayerCommander->GetMana() < m_pPlayerCommander->GetCurrentItem()->GetManaCost())
            snapshot.SelectItem(-1);        
    }
}


/*====================
  CClientCommander::Spawn
  ====================*/
void    CClientCommander::Spawn()
{
}


/*====================
  CClientCommander::Frame
  ====================*/
void    CClientCommander::Frame()
{
    // TODO: Set our selection to match the server (or prediction)
    // We need a good way to send the selection set in the
    // player commander entity (variable length array)
    //m_setSelection = m_pPlayerCommander->GetSelection();

    UpdateCursorTrace();
    UpdateHoverSelection();
    UpdateSelectedPlayerWaypoints();
    SetOverrideSnapcast(false);
}


/*====================
  CClientCommander::GetSelectionRect
  ====================*/
CRectf  CClientCommander::GetSelectionRect()
{
    CVec2f v2Start, v2End;

    if (!m_pCamera->WorldToScreen(m_v3StartCursorPos, v2Start))
        v2Start = m_v3StartCursorPos.x < m_v3TraceEndPos.x?CVec2f(-1.f, m_pCamera->GetHeight()+1.f):CVec2f(m_pCamera->GetWidth()+1.f, m_pCamera->GetHeight()+1.f);
    m_pCamera->WorldToScreen(m_v3TraceEndPos, v2End);

    CVec2f v2Min(
        MIN(v2Start.x, v2End.x),
        MIN(v2Start.y, v2End.y));
    CVec2f v2Max(
        MAX(v2Start.x, v2End.x),
        MAX(v2Start.y, v2End.y));

    return CRectf(v2Min, v2Max);
}


/*====================
  CClientCommander::GetWorldSelection
  ====================*/
CRectf  CClientCommander::GetWorldSelection()
{
    return CRectf(
        MIN(m_v3StartCursorPos.x, m_v3TraceEndPos.x),
        MIN(m_v3StartCursorPos.y, m_v3TraceEndPos.y),
        MAX(m_v3StartCursorPos.x, m_v3TraceEndPos.x), 
        MAX(m_v3StartCursorPos.y, m_v3TraceEndPos.y)
        );
}


/*====================
  CClientCommander::UpdateHoverSelection
  ====================*/
void    CClientCommander::UpdateHoverSelection()
{
    m_setHoverSelection.clear();

    if (m_uiHoverEnt != INVALID_INDEX)
        m_setHoverSelection.insert(m_uiHoverEnt);

    if (m_eState != COMSTATE_SELECT)
        return;

    CRectf rect(GetWorldSelection());

    WorldEntMap &mapEntities(Game.GetWorldEntityMap());
    
    // First just get a potential set
    set<IVisualEntity*> setPotential;
    for (WorldEntMap::iterator it(mapEntities.begin()); it != mapEntities.end(); ++it)
    {
        IVisualEntity *pEntity(GameClient.GetClientEntityCurrent(GameClient.GetGameIndexFromWorldIndex(it->first)));
        if (pEntity == NULL)
            continue;
        if (!pEntity->IsSelectable())
            continue;

        if (rect.Contains(pEntity->GetPosition().xy()))
            setPotential.insert(pEntity);
    }

    // Add clients on this team to the set
    for (set<IVisualEntity*>::iterator it(setPotential.begin()); it != setPotential.end(); ++it)
    {
        IVisualEntity *pEntity(*it);
        if (!pEntity->IsPlayer() && !pEntity->IsPet())
            continue;
        if (m_pPlayerCommander->LooksLikeEnemy(pEntity))
            continue;
        m_setHoverSelection.insert(pEntity->GetIndex());
    }
    if (!m_setHoverSelection.empty())
        return;

    // Add enemy clients to the set
    for (set<IVisualEntity*>::iterator it(setPotential.begin()); it != setPotential.end(); ++it)
    {
        IVisualEntity *pEntity(*it);
        if (!pEntity->IsPlayer())
            continue;
        if (pEntity->GetTeam() != 0)
            continue;
        m_setHoverSelection.insert(pEntity->GetIndex());
    }
    if (!m_setHoverSelection.empty())
        return;

    // Add NPCs
    for (set<IVisualEntity*>::iterator it(setPotential.begin()); it != setPotential.end(); ++it)
    {
        IVisualEntity *pEntity(*it);
        if (!pEntity->IsNpc())
            continue;

        m_setHoverSelection.insert(pEntity->GetIndex());
    }
    if (!m_setHoverSelection.empty())
        return;

    if (setPotential.size())
    {
        if (setPotential.size() == 1)
            m_setHoverSelection.insert((*setPotential.begin())->GetIndex());
        else
            m_setHoverSelection.insert((*setPotential.rbegin())->GetIndex());
    }
}


/*====================
  CClientCommander::Draw
  ====================*/
void    CClientCommander::Draw()
{
    if (IsSelectionActive())
    {
        CVec4f  v4Border(1.0f, 1.0f, 1.0f, 1.0f);
        CVec4f  v4Fill(0.3f, 0.7f, 1.0f, 0.2f);

        CRectf rec(GetSelectionRect());

        Draw2D.SetColor(v4Fill);
        Draw2D.Rect(rec.left, rec.top, rec.GetWidth(), rec.GetHeight());

        Draw2D.RectOutline(rec, 1.0f, v4Border);
    }
}


/*====================
  CClientCommander::UpdateCursorTrace
  ====================*/
void     CClientCommander::UpdateCursorTrace()
{
    bool bFullTrace(m_eState == COMSTATE_HOVER);
    bool bPlayHoverSound(false);

    STraceInfo trace;
    STraceInfo traceTrans;
    STraceInfo traceWorking;
    CVec3f v3Dir(GameClient.GetCamera()->ConstructRay(Input.GetCursorPos()));
    CVec3f v3End(M_PointOnLine(GameClient.GetCamera()->GetOrigin(), v3Dir, FAR_AWAY));
    uiset setIgnored;
    CVec3f v3TransStart(GameClient.GetCamera()->GetOrigin());

    // Do a trace around the cursor to determine what should and shouldn't be transparent
    traceTrans.uiEntityIndex = 0;
    Game.TraceBox(traceWorking, v3TransStart, v3End, CBBoxf(-cg_cmdrTransparentWidth, cg_cmdrTransparentWidth), TRACE_COMMANDER_TRANSPARENT);

    while (traceTrans.uiEntityIndex != INVALID_INDEX)
    {
        Game.TraceBox(traceTrans, v3TransStart, v3End, CBBoxf(-cg_cmdrTransparentWidth, cg_cmdrTransparentWidth), SURF_IGNORE | SURF_BLOCKER | SURF_TERRAIN);

        v3TransStart = traceTrans.v3EndPos;

        CWorldEntity *pWorld(GameClient.GetWorldEntity(traceTrans.uiEntityIndex));

        if (pWorld == NULL)
            break;

        setIgnored.insert(traceTrans.uiEntityIndex);
        pWorld->SetSurfFlags(pWorld->GetSurfFlags() | SURF_IGNORE);

        if (traceTrans.uiEntityIndex == traceWorking.uiEntityIndex)
        {
            Game.TraceBox(traceWorking, v3TransStart, v3End, CBBoxf(-cg_cmdrTransparentWidth, cg_cmdrTransparentWidth), TRACE_COMMANDER_TRANSPARENT);
            continue;
        }
            
        IVisualEntity *pEnt(Game.GetVisualEntity(pWorld->GetGameIndex()));

        if (pEnt != NULL)
        {
            pEnt->AddClientRenderFlags(ECRF_HALFTRANSPARENT);
            GameClient.AddTransparentEntity(pEnt->GetIndex());
        }
    }

    for (uiset::iterator it(setIgnored.begin()); it != setIgnored.end(); it++)
    {
        CWorldEntity *pWorld(GameClient.GetWorldEntity(*it));

        if (pWorld != NULL)
            pWorld->SetSurfFlags(pWorld->GetSurfFlags() & ~SURF_IGNORE);
    }

    if (Game.TraceLine(trace, GameClient.GetCamera()->GetOrigin(), v3End, bFullTrace ? TRACE_COMMANDER_SNAPCAST : TRACE_TERRAIN))
    {
        m_v3TraceEndPos = trace.v3EndPos;

        if (bFullTrace)
        {
            uint uiHoverEnt(Game.GetGameIndexFromWorldIndex(trace.uiEntityIndex));

            if (uiHoverEnt != INVALID_INDEX && uiHoverEnt != m_uiHoverEnt)
                bPlayHoverSound = true;
            
            m_uiHoverEnt = uiHoverEnt;
        }
    }
    else
    {
        m_v3TraceEndPos.Clear();

        if (bFullTrace)
            m_uiHoverEnt = INVALID_INDEX;
    }

    if (!bFullTrace)
        m_uiHoverEnt = INVALID_INDEX;

    IVisualEntity *pEntity(GameClient.GetClientEntityCurrent(m_uiHoverEnt));

    if (!pEntity)
        m_uiHoverEnt = INVALID_INDEX;
    else if (!pEntity->IsSelectable())
        m_uiHoverEnt = INVALID_INDEX;

    if (bPlayHoverSound && m_uiHoverEnt != INVALID_INDEX && m_hHoverSound != INVALID_RESOURCE)
        K2SoundManager.Play2DSound(m_hHoverSound);
}


/*====================
  CClientCommander::PrimaryDown
  ====================*/
void    CClientCommander::PrimaryDown()
{
    UpdateCursorTrace();

    if (GameClient.GetCurrentSnapshot()->GetSelectedItem() != byte(-1))
    {
        GameClient.GetCurrentSnapshot()->SetActivate(GameClient.GetCurrentSnapshot()->GetSelectedItem());
        //GameClient.GetCurrentSnapshot()->SelectItem(-1);
        return;
    }

    switch (m_eState)
    {
    case COMSTATE_HOVER:
        StartSelect();
        break;

    case COMSTATE_SELECT:
        break;

    case COMSTATE_BUILD:
        break;
    }
}


/*====================
  CClientCommander::PrimaryUp
  ====================*/
void    CClientCommander::PrimaryUp()
{
    switch (m_eState)
    {
    case COMSTATE_HOVER:
        break;

    case COMSTATE_SELECT:
        ApplySelect();
        m_eState = COMSTATE_HOVER;
        m_v3StartCursorPos.Clear();
        break;

    case COMSTATE_BUILD:
        break;
    }
}


/*====================
  CClientCommander::SecondaryDown
  ====================*/
void    CClientCommander::SecondaryDown()
{
    if (GameClient.GetCurrentSnapshot()->GetSelectedItem() != byte(-1))
    {
        GameClient.GetCurrentSnapshot()->SelectItem(-1);
        return;
    }

    UpdateCursorTrace();

    switch (m_eState)
    {
    case COMSTATE_HOVER:
        StartCommand();
        break;

    case COMSTATE_SELECT:
        break;

    case COMSTATE_BUILD:
        GameClient.SetBuildingRotate(true);
        break;
    }
}


/*====================
  CClientCommander::SecondaryUp
  ====================*/
void    CClientCommander::SecondaryUp()
{
    switch (m_eState)
    {
    case COMSTATE_HOVER:
        break;

    case COMSTATE_SELECT:
        break;

    case COMSTATE_BUILD:
        GameClient.SetBuildingRotate(false);
        break;
    }
}


/*====================
  CClientCommander::Cancel
  ====================*/
bool    CClientCommander::Cancel()
{
    if (GameClient.GetCurrentSnapshot()->GetSelectedItem() != byte(-1))
    {
        GameClient.GetCurrentSnapshot()->SelectItem(-1);
        return true;
    }

    if (m_eState != COMSTATE_HOVER)
    {
        m_eState = COMSTATE_HOVER;
        return true;
    }

    if (!m_setSelection.empty())
    {
        m_setSelection.clear();
        CommanderSelection.Trigger(XtoA(!m_setSelection.empty()));
        return true;
    }

    return false;
}


/*====================
  CClientCommander::StartSelect
  ====================*/
void    CClientCommander::StartSelect()
{
    if (m_uiHoverEnt == INVALID_INDEX)
    {
        m_eState = COMSTATE_SELECT;
        m_v3StartCursorPos = m_v3TraceEndPos;
    }
    else if (m_uiHoverEnt != INVALID_INDEX)
    {
        if (m_bModifier1)
        {
            m_setSelection.insert(m_uiHoverEnt);
            CommanderSelection.Trigger(XtoA(!m_setSelection.empty()));
        }
        else if (m_bModifier2)
        {
            if (m_setSelection.find(m_uiHoverEnt) == m_setSelection.end())
                m_setSelection.insert(m_uiHoverEnt);
            else
                m_setSelection.erase(m_uiHoverEnt);

            CommanderSelection.Trigger(XtoA(!m_setSelection.empty()));
        }
        else if (!(m_setSelection.size() == 1 && *m_setSelection.begin() == m_uiHoverEnt))
        {
            m_setSelection.clear();
            m_setSelection.insert(m_uiHoverEnt);
            CommanderSelection.Trigger(XtoA(!m_setSelection.empty()));

            K2SoundManager.Play2DSound(m_hSelectedSound);

            IVisualEntity *pSelectedEntity(GameClient.GetClientEntityCurrent(m_uiHoverEnt));

            if (pSelectedEntity->GetTeam() == m_pPlayerCommander->GetTeam() && pSelectedEntity->IsBuilding() || pSelectedEntity->IsProp())
            {
                tstring sFeedbackSound;
                if (pSelectedEntity->IsBuilding())
                {
                    if (pSelectedEntity->GetAsBuilding()->GetStatus() == ENTITY_STATUS_SPAWNING)
                        sFeedbackSound = pSelectedEntity->GetAsBuilding()->GetSelectConstructionSoundPath();
                    else
                        sFeedbackSound = pSelectedEntity->GetAsBuilding()->GetSelectSoundPath();
                }
                else
                    sFeedbackSound = pSelectedEntity->GetAsProp()->GetSelectSoundPath();
                if (!sFeedbackSound.empty())
                {
                    ResHandle hSample(g_ResourceManager.Register(sFeedbackSound, RES_SAMPLE));
                    if (hSample != INVALID_RESOURCE)
                        K2SoundManager.Play2DSound(hSample);
                }
            }
        }

        m_eState = COMSTATE_HOVER;
        m_v3StartCursorPos = m_v3TraceEndPos;
    }
}


/*====================
  CClientCommander::ApplySelect
  ====================*/
void    CClientCommander::ApplySelect()
{
    if (!m_bModifier1 && !m_bModifier2)
    {
        m_setSelection.clear();
    }

    bool bEmpty(m_setSelection.empty());

    UpdateHoverSelection();
    for (uiset_it it(m_setHoverSelection.begin()); it != m_setHoverSelection.end(); ++it)
    {
        if (m_bModifier2 && m_setSelection.find(*it) != m_setSelection.end())
            m_setSelection.erase(*it);
        else
            m_setSelection.insert(*it);
    }

    CommanderSelection.Trigger(XtoA(!m_setSelection.empty()));

    if (bEmpty && m_setSelection.size() == 1)
    {
        K2SoundManager.Play2DSound(m_hSelectedSound);

        IVisualEntity *pSelectedEntity(GameClient.GetClientEntityCurrent(*m_setSelection.begin()));

        if (pSelectedEntity->GetTeam() == m_pPlayerCommander->GetTeam() && pSelectedEntity->IsBuilding() || pSelectedEntity->IsProp())
        {
            tstring sFeedbackSound;
            if (pSelectedEntity->IsBuilding())
            {
                if (pSelectedEntity->GetAsBuilding()->GetStatus() == ENTITY_STATUS_SPAWNING)
                    sFeedbackSound = pSelectedEntity->GetAsBuilding()->GetSelectConstructionSoundPath();
                else
                    sFeedbackSound = pSelectedEntity->GetAsBuilding()->GetSelectSoundPath();
            }
            else
                sFeedbackSound = pSelectedEntity->GetAsProp()->GetSelectSoundPath();
            if (!sFeedbackSound.empty())
            {
                ResHandle hSample(g_ResourceManager.Register(sFeedbackSound, RES_SAMPLE));
                if (hSample != INVALID_RESOURCE)
                {
                    K2SoundManager.Play2DSound(hSample);
                }
            }
        }
    }
    else if (bEmpty && m_setSelection.size())
    {
        K2SoundManager.Play2DSound(m_hMultiSelectedSound);
    }
}


/*====================
  CClientCommander::GiveOrder
  ====================*/
void    CClientCommander::GiveOrder(ECommanderOrder eOrder, uint uiTargetIndex)
{
    CBufferFixed<7> buffer;
    buffer << GAME_CMD_ENT_COMMAND << byte(eOrder) << uiTargetIndex << byte(GetModifier1() ? 1 : 0);
    GameClient.SendGameData(buffer, true);
}

void    CClientCommander::GiveOrder(ECommanderOrder eOrder, const CVec3f &v3Pos)
{
    CGameEvent ev;
    ev.SetSourcePosition(v3Pos);

    switch (m_eOrder)
    {
    default:
    case CMDR_ORDER_MOVE:
        ev.SetEffect(g_ResourceManager.Register(_T("/shared/effects/move_indicator.effect"), RES_EFFECT));
        break;

    case CMDR_ORDER_ATTACK:     
        ev.SetEffect(g_ResourceManager.Register(_T("/shared/effects/attack_indicator.effect"), RES_EFFECT));
        break;
    }

    ev.Spawn();
    Game.AddEvent(ev);

    CBufferFixed<15> buffer;
    buffer << GAME_CMD_POS_COMMAND << byte(eOrder) << v3Pos << byte(GetModifier1() ? 1 : 0);
    GameClient.SendGameData(buffer, true);
}


/*====================
  CClientCommander::StartCommand
  ====================*/
void    CClientCommander::StartCommand()
{
    uiset::const_iterator itEnd(m_setSelection.end());
    uiset::const_iterator it(m_setSelection.begin());

    for (; it != itEnd; ++it)
    {
        IVisualEntity *pEnt(GameClient.GetClientEntityCurrent(*it));

        if (pEnt == NULL)
            continue;

        if (pEnt->IsPlayer() || pEnt->IsPet())
            break;
    }

    if (it == itEnd)
        return;

    if (m_eOrder == CMDR_ORDER_AUTO)
    {
        if (m_uiHoverEnt != INVALID_INDEX)
        {
            IVisualEntity *pEnt(GameClient.GetClientEntityCurrent(m_uiHoverEnt));

            if (pEnt && m_pPlayerCommander->IsEnemy(pEnt))
                m_eOrder = CMDR_ORDER_ATTACK;
            else
                m_eOrder = CMDR_ORDER_MOVE;
        }
        else
            m_eOrder = CMDR_ORDER_MOVE;
    }

    if (m_uiHoverEnt != INVALID_INDEX)
        GiveOrder(m_eOrder, m_uiHoverEnt);
    else
        GiveOrder(m_eOrder, m_v3TraceEndPos);
    
    m_eOrder = CMDR_ORDER_AUTO;
}


/*====================
  CClientCommander::Ping
  ====================*/
void    CClientCommander::Ping()
{
    UpdateCursorTrace();

    if (Host.GetTime() - GameClient.GetLastPingTime() > MIN_PING_TIME)
    {
        CBufferFixed<10> buffer;
        buffer << GAME_CMD_MINIMAP_PING << m_v3TraceEndPos.x / GameClient.GetWorldWidth() << m_v3TraceEndPos.y / GameClient.GetWorldHeight() << byte(-1);
        GameClient.SendGameData(buffer, false);
        GameClient.SetLastPingTime(Host.GetTime());
    }
}


/*====================
  CClientCommander::GetSelectedEntity
  ====================*/
uint    CClientCommander::GetSelectedEntity()
{
    if (m_setSelection.size() == 1)
        return *m_setSelection.begin();
    else
        return INVALID_INDEX;
}


/*====================
  CClientCommander::SelectEntity
  ====================*/
void    CClientCommander::SelectEntity(uint uiIndex, bool bClear)
{
    if (uiIndex == INVALID_INDEX)
        return;

    // If there is a spell ready to cast, do that instead
    IInventoryItem *pItem(m_pPlayerCommander->GetCurrentItem());
    if (pItem != NULL &&
        pItem->IsSpell() &&
        pItem->GetAsSpell()->IsSnapcast())
    {
        ISpellItem *pSpell(pItem->GetAsSpell());
        IVisualEntity *pTarget(GameClient.GetClientEntityCurrent(uiIndex));
        if (pTarget == NULL)
            return;
        if (!pSpell->IsValidTarget(pTarget, false))
            return;

        GameClient.GetCurrentSnapshot()->SetSelectedEntity(uiIndex);
        GameClient.GetCurrentSnapshot()->SetActivate(pSpell->GetSlot());
        SetOverrideSnapcast(true);
        return;
    }

    // If SelectEntity was recently called for this same entity, center the camera on it
    if (Host.GetTime() - m_uiLastSelectTime < cg_cmdrDoubleSelectTime &&
        m_setSelection.size() == 1 &&
        *m_setSelection.begin() == uiIndex)
    {
        IVisualEntity *pEntity(GameClient.GetClientEntityCurrent(uiIndex));
        CVec3f v3Target(V_ZERO);
        if (pEntity != NULL)
            v3Target = pEntity->GetPosition();
        v3Target.z = GameClient.GetCurrentSnapshot()->GetCameraPosition().z;
        GameClient.GetCurrentSnapshot()->SetCameraPosition(v3Target);
        return;
    }

    if (bClear)
        m_setSelection.clear();
    m_setSelection.insert(uiIndex);
    m_uiLastSelectTime = Host.GetTime();
}


/*====================
  CClientCommander::SelectSquad
  ====================*/
void    CClientCommander::SelectSquad(uint uiSquad, bool bClear)
{
    if (uiSquad == INVALID_INDEX)
        return;

    if (bClear)
        m_setSelection.clear();

    CEntityTeamInfo *pTeam(GameClient.GetTeam(m_pPlayerCommander->GetTeam()));
    if (pTeam == NULL)
        return;

    for (uint uiMember(0); uiMember < pTeam->GetSquadSize(uiSquad); ++uiMember)
        m_setSelection.insert(pTeam->GetSquadMemberIndex(uiSquad, uiMember));
}


/*====================
  CClientCommander::UpdateSelectedPlayerWaypoints
  ====================*/
void    CClientCommander::UpdateSelectedPlayerWaypoints()
{
    for (int i(0); i < MAX_PLAYER_WAYPOINTS; ++i)
        m_aSelectedPlayerWaypoints[i].bActive = false;

    for (uiset::iterator it(m_setSelection.begin()); it != m_setSelection.end(); ++it)
    {
        IVisualEntity *pEnt(GameClient.GetClientEntityCurrent(*it));
        if (pEnt == NULL)
            continue;

        if (pEnt->IsPlayer() && pEnt->GetTeam() == m_pPlayerCommander->GetTeam())
        {
            IPlayerEntity *pPlayer(pEnt->GetAsPlayerEnt());

            if (pPlayer->GetCurrentOrder() == CMDR_ORDER_CLEAR)
                continue;

            int i;
            for (i = 0; i < MAX_PLAYER_WAYPOINTS; ++i)
            {
                if (m_aSelectedPlayerWaypoints[i].uiEvent != INVALID_INDEX &&
                    m_aSelectedPlayerWaypoints[i].uiOrderEntIndex != INVALID_INDEX &&
                    m_aSelectedPlayerWaypoints[i].uiOrderEntIndex == pPlayer->GetCurrentOrderEntIndex())
                {
                    m_aSelectedPlayerWaypoints[i].bActive = true;
                    break;
                }
                else if (m_aSelectedPlayerWaypoints[i].uiEvent != INVALID_INDEX &&
                    m_aSelectedPlayerWaypoints[i].uiOrderEntIndex == INVALID_INDEX &&
                    m_aSelectedPlayerWaypoints[i].v3OrderPos == pPlayer->GetCurrentOrderPos())
                {
                    m_aSelectedPlayerWaypoints[i].bActive = true;
                    break;
                }
            }

            if (i == MAX_PLAYER_WAYPOINTS)
            {
                for (i = 0; i < MAX_PLAYER_WAYPOINTS; ++i)
                {
                    if (m_aSelectedPlayerWaypoints[i].uiEvent == INVALID_INDEX)
                    {
                        m_aSelectedPlayerWaypoints[i].bActive = true;
                        m_aSelectedPlayerWaypoints[i].uiOrderEntIndex = pPlayer->GetCurrentOrderEntIndex();
                        m_aSelectedPlayerWaypoints[i].v3OrderPos = pPlayer->GetCurrentOrderPos();

                        switch (pPlayer->GetCurrentOrder())
                        {
                        case CMDR_ORDER_MOVE:
                            {               
                                CGameEvent ev;
                                ev.SetSourcePosition(pPlayer->GetCurrentOrderPos());
                                ev.SetSourceEntity(pPlayer->GetCurrentOrderEntIndex());
                                ev.SetEffect(g_ResourceManager.Register(_T("/shared/effects/waypoint.effect"), RES_EFFECT));
                                ev.Spawn();
                                
                                m_aSelectedPlayerWaypoints[i].uiEvent = Game.AddEvent(ev);
                            }
                            break;
                        case CMDR_ORDER_ATTACK:
                            {
                                CGameEvent ev;
                                ev.SetSourcePosition(pPlayer->GetCurrentOrderPos());
                                ev.SetSourceEntity(pPlayer->GetCurrentOrderEntIndex());
                                ev.SetEffect(g_ResourceManager.Register(_T("/shared/effects/attack_waypoint.effect"), RES_EFFECT));
                                ev.Spawn();
                                
                                m_aSelectedPlayerWaypoints[i].uiEvent = Game.AddEvent(ev);
                            }
                            break;
                        default:
                            m_aSelectedPlayerWaypoints[i].uiEvent = INVALID_INDEX;
                            break;
                        }
                        break;
                    }
                }
            }
        }
    }

    for (int i(0); i < MAX_PLAYER_WAYPOINTS; ++i)
    {
        if (!m_aSelectedPlayerWaypoints[i].bActive && m_aSelectedPlayerWaypoints[i].uiEvent != INVALID_INDEX)
        {
            Game.DeleteEvent(m_aSelectedPlayerWaypoints[i].uiEvent);
            m_aSelectedPlayerWaypoints[i].uiEvent = INVALID_INDEX;
        }
    }
}


/*====================
  CClientCommander::GetCameraDistance
  ====================*/
float   CClientCommander::GetCameraDistance() const
{
    if (m_pPlayerCommander)
        return m_pPlayerCommander->GetPosition().z;
    else
        return 0.0f;
}

