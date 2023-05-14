// (C)2006 S2 Games
// i_gadgetentity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_gadgetentity.h"
#include "i_entitytool.h"
#include "c_teaminfo.h"
#include "c_player.h"
#include "c_bsentry.h"

#include "../k2/c_model.h"
#include "../k2/c_worldentity.h"
#include "../k2/c_texture.h"
#include "../k2/c_sceneentity.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_skeleton.h"
#include "../k2/c_eventscript.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
uint                IGadgetEntity::s_uiBaseType(ENTITY_BASE_TYPE_GADGET);


DEFINE_ENTITY_DESC(IGadgetEntity, 1)
{
    s_cDesc.pFieldTypes = K2_NEW(ctx_Game,   TypeVector)();
    s_cDesc.pFieldTypes->clear();
    const TypeVector &vBase(IUnitEntity::GetTypeVector());
    s_cDesc.pFieldTypes->insert(s_cDesc.pFieldTypes->begin(), vBase.begin(), vBase.end());

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiMountIndex"), TYPE_GAMEINDEX, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiCharges"), TYPE_INT, 7, 0));
}
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
CVAR_FLOATF (g_gadgetIconSizeScale,         2.0f,   CVAR_GAMECONFIG);
//=============================================================================

/*====================
  IGadgetEntity::IGadgetEntity
  ====================*/
IGadgetEntity::IGadgetEntity() :
m_uiMountIndex(INVALID_INDEX),
m_uiCharges(0),
m_uiAuraSourceUID(INVALID_INDEX),
m_uiAuraTime(INVALID_TIME)
{
}


/*====================
  IGadgetEntity::Baseline
  ====================*/
void    IGadgetEntity::Baseline()
{
    IUnitEntity::Baseline();

    m_uiMountIndex = INVALID_INDEX;
    m_uiCharges = 0;
}


/*====================
  IGadgetEntity::GetSnapshot
  ====================*/
void    IGadgetEntity::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
    IUnitEntity::GetSnapshot(snapshot, uiFlags);

    snapshot.WriteGameIndex(m_uiMountIndex);
    snapshot.WriteField(m_uiCharges);
}


/*====================
  IGadgetEntity::ReadSnapshot
  ====================*/
bool    IGadgetEntity::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
    try
    {
        if (!IUnitEntity::ReadSnapshot(snapshot, 1))
            return false;

        snapshot.ReadGameIndex(m_uiMountIndex);
        snapshot.ReadField(m_uiCharges);
    }
    catch (CException &ex)
    {
        ex.Process(_T("IGadgetEntity::ReadSnapshot() - "), NO_THROW);
        return false;
    }

    return true;
}


/*====================
  IGadgetEntity::Spawn
  ====================*/
void    IGadgetEntity::Spawn()
{
    IUnitEntity::Spawn();
    
    // Abilities match the level of the gadget
    for (uint ui(INVENTORY_START_ABILITIES); ui <= INVENTORY_END_ABILITIES; ++ui)
    {
        if (m_apInventory[ui] == nullptr)
            continue;

        m_apInventory[ui]->SetLevel(GetLevel());
    }

    if (Game.IsServer())
        m_cBrain.AddBehavior(K2_NEW(ctx_Game,   CBSentry), false);

    SetLifetime(Game.GetGameTime(), GetLifetime());
}


/*====================
  IGadgetEntity::ServerFrameThink
  ====================*/
bool    IGadgetEntity::ServerFrameThink()
{
    // Issue default behavior (Sentry)
    if (m_cBrain.IsEmpty() && m_cBrain.GetCommandsPending() == 0)
        m_cBrain.AddBehavior(K2_NEW(ctx_Game,   CBSentry), false);

    return IUnitEntity::ServerFrameThink();
}


/*====================
  IGadgetEntity::ServerFrameCleanup
  ====================*/
bool    IGadgetEntity::ServerFrameCleanup()
{
    if (!IUnitEntity::ServerFrameCleanup())
        return false;

    if (IsAuraInvalid())
        return false;

    return true;
}


/*====================
  IGadgetEntity::Copy
  ====================*/
void    IGadgetEntity::Copy(const IGameEntity &B)
{
    IUnitEntity::Copy(B);

    const IGadgetEntity *pB(B.GetAsGadget());

    if (pB == nullptr)
        return;

    const IGadgetEntity &C(*pB);

    m_uiMountIndex = C.m_uiMountIndex;
    m_uiCharges = C.m_uiCharges;
}


/*====================
  IGadgetEntity::SetLevel
  ====================*/
void    IGadgetEntity::SetLevel(uint uiLevel)
{
    if (m_uiLevel != uiLevel)
    {
        m_uiLevel = uiLevel;

        // Abilities match the level of the gadget
        for (uint ui(INVENTORY_START_ABILITIES); ui <= INVENTORY_END_ABILITIES; ++ui)
        {
            if (m_apInventory[ui] == nullptr)
                continue;

            m_apInventory[ui]->SetLevel(GetLevel());
        }
    }
}

/*====================
  IGadgetEntity::DrawOnMap
  ====================*/
void    IGadgetEntity::DrawOnMap(CUITrigger &minimap, CPlayer *pLocalPlayer) const
{
    if (!IsVisibleOnMap(pLocalPlayer))
        return;

    CBufferFixed<40> buffer;

    buffer << GetPosition().x / Game.GetWorldWidth();
    buffer << 1.0f - (GetPosition().y / Game.GetWorldHeight());
    
    buffer << GetMapIconSize(pLocalPlayer) * g_gadgetIconSizeScale << GetMapIconSize(pLocalPlayer) * g_gadgetIconSizeScale;
    
    CVec4f v4Color(GetMapIconColor(pLocalPlayer));
    buffer << v4Color[R];
    buffer << v4Color[G];
    buffer << v4Color[B];
    buffer << v4Color[A];

    buffer << GetMapIcon(pLocalPlayer);

    buffer << (GetHoverOnMap() ? GetIndex() : uint(-1));

    minimap.Execute(_T("icon"), buffer);
}


/*====================
  IGadgetEntity::UpdateModifiers

  Inherit owner modifiers every frame
  JCG - Do we want this to be optional?
  ====================*/
void    IGadgetEntity::UpdateModifiers()
{
    SetModifierBits(0);

    m_vModifierKeys = m_vPersistentModifierKeys;

    IUnitEntity *pOwner(GetOwner());
    if (pOwner != nullptr)
    {
        pOwner->UpdateModifiers();
        const uivector &vOwnerModifiers(pOwner->GetModifierKeys());
        m_vModifierKeys.insert(m_vModifierKeys.end(), vOwnerModifiers.begin(), vOwnerModifiers.end());
    }

    if (m_uiActiveModifierKey != INVALID_INDEX)
        m_vModifierKeys.push_back(m_uiActiveModifierKey);

    // Gather modifier keys from slaves
    for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
    {
        ISlaveEntity *pSlave(GetSlave(iSlot));
        if (pSlave == nullptr)
            continue;

        uint uiModifierKey(pSlave->GetModifierKey());
        if (uiModifierKey != INVALID_INDEX && uiModifierKey != 0)
            m_vModifierKeys.push_back(uiModifierKey);

        uint uiModifierKey2(pSlave->GetModifierKey2());
        if (uiModifierKey2 != INVALID_INDEX && uiModifierKey2 != 0)
            m_vModifierKeys.push_back(uiModifierKey2);
    }

    SetModifierBits(GetModifierBits() | GetModifierBits(m_vModifierKeys));

    // Each tool of this entity receives all of their master's modifiers
    // States receive their modifiers from their inflictor at application time
    for (int iSlot(0); iSlot < INVENTORY_START_STATES; ++iSlot)
    {
        IEntityTool *pTool(GetTool(iSlot));
        if (pTool == nullptr)
            continue;

        pTool->UpdateModifiers(m_vModifierKeys);
    }
}
