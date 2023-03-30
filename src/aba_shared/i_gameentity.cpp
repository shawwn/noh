// (C)2005 S2 Games
// i_gameentity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_gameentity.h"

#include "c_player.h"
#include "i_visualentity.h"
#include "i_projectile.h"
#include "i_areaaffector.h"
#include "i_light.h"
#include "i_unitentity.h"
#include "i_heroentity.h"
#include "i_creepentity.h"
#include "i_gadgetentity.h"
#include "i_buildingentity.h"
#include "i_propentity.h"
#include "c_teaminfo.h"
#include "i_entitystate.h"
#include "i_entityability.h"
#include "i_entityitem.h"
#include "c_entityregistry.h"

#include "../k2/c_sceneentity.h"
#include "../k2/c_skeleton.h"
#include "../k2/c_eventscript.h"
#include "../k2/c_entitysnapshot.h"
#include "../k2/c_host.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_networkresourcemanager.h"
#include "../k2/c_worldentity.h"
#include "../k2/c_model.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_texture.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENTITY_DESC(IGameEntity, 1)
{
    s_cDesc.pFieldTypes = K2_NEW(g_heapTypeVector,   TypeVector)();

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_unModifierBits"), TYPE_SHORT, 16, 0));
}
//=============================================================================


/*====================
  IGameEntity::CEntityConfig::CEntityConfig
  ====================*/
IGameEntity::CEntityConfig::CEntityConfig(const tstring &sName)
{
}


/*====================
  IGameEntity::IGameEntity
  ====================*/
IGameEntity::IGameEntity(CEntityConfig *pConfig) :
m_pEntityConfig(pConfig),
m_pDefinition(NULL),
m_hDefinition(INVALID_RESOURCE),

m_unType(0),
m_uiIndex(INVALID_INDEX),
m_uiUniqueID(INVALID_INDEX),
m_unModifierBits(0),
m_uiActiveModifierKey(INVALID_INDEX),

m_bDelete(false),
m_uiFrame(uint(-1)),
m_bValid(false)
{
}


/*====================
  IGameEntity::Baseline
  ====================*/
void    IGameEntity::Baseline()
{
    m_unModifierBits = 0;
}


/*====================
  IGameEntity::GetSnapshot
  ====================*/
void    IGameEntity::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
    snapshot.WriteField(m_unModifierBits);
}


/*====================
  IGameEntity::ReadSnapshot
  ====================*/
bool    IGameEntity::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
    try
    {
        Validate();
        snapshot.ReadField(m_unModifierBits);
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("IGameEntity::ReadSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  IGameEntity::Copy
  ====================*/
void    IGameEntity::Copy(const IGameEntity &B)
{
    m_unType            = B.m_unType;
    m_uiFrame           = B.m_uiFrame;
    m_unModifierBits    = B.m_unModifierBits;
    m_pDefinition       = B.m_pDefinition;
}


/*====================
  IGameEntity::GetMasterOwner
  ====================*/
IUnitEntity*    IGameEntity::GetMasterOwner() const
{
    if (GetOwner() == NULL)
        return NULL;

    return GetOwner()->GetMasterOwner();
}


/*====================
  IGameEntity::GetModifierBit
  ====================*/
uint    IGameEntity::GetModifierBit(uint uiModifierID)
{
    CEntityDefinitionResource *pResource(g_ResourceManager.Get<CEntityDefinitionResource>(m_hDefinition));
    if (pResource == NULL)
        return 0;

    // Access root definition
    IEntityDefinition *pDefinition(pResource->GetDefinition<IEntityDefinition>());
    if (pDefinition == NULL)
        return 0;

    return pDefinition->GetModifierBit(uiModifierID);
}


/*====================
  IGameEntity::GetModifierBits
  ====================*/
uint    IGameEntity::GetModifierBits(const uivector &vModifiers)
{
    CEntityDefinitionResource *pResource(g_ResourceManager.Get<CEntityDefinitionResource>(m_hDefinition));
    if (pResource == NULL)
        return 0;

    // Access root definition
    IEntityDefinition *pDefinition(pResource->GetDefinition<IEntityDefinition>());
    if (pDefinition == NULL)
        return 0;

    return pDefinition->GetModifierBits(vModifiers);
}


/*====================
  IGameEntity::MorphDynamicType

  Change this entity to a different dynamic type
  with the same base type
  ====================*/
bool    IGameEntity::MorphDynamicType(ushort unType)
{
    const CDynamicEntityAllocator *pOldAllocator(EntityRegistry.GetDynamicAllocator(m_unType));
    if (pOldAllocator == NULL)
        return false;

    const CDynamicEntityAllocator *pNewAllocator(EntityRegistry.GetDynamicAllocator(unType));
    if (pNewAllocator == NULL)
        return false;

    m_sTypeName = pNewAllocator->GetName();
    m_unType = pNewAllocator->GetTypeID();
    m_hDefinition = pNewAllocator->GetDefinitionHandle();
    
    UpdateDefinition();

    return true;
}


/*====================
  IGameEntity::SendExtendedData
  ====================*/
void    IGameEntity::SendExtendedData(int iClient) const
{
    CBufferDynamic buffer;
    GetExtendedData(buffer);

    CBufferStatic bufferSend(buffer.GetLength() + 7);
    bufferSend << GAME_CMD_EXT_ENTITY_DATA << GetIndex() << ushort(buffer.GetLength()) << buffer;

    // HACK: Send these packets right away for now, so the reliable packet doesn't overflow
    //Game.SendGameData(iClient, bufferSend, true);
    Game.SendReliablePacket(iClient, bufferSend);
}


/*====================
  IGameEntity::HasModifier
  ====================*/
bool    IGameEntity::HasModifier(const tstring &sModifier) const
{
    uint uiModifier(EntityRegistry.RegisterModifier(sModifier));
    if (GetActiveModifierKey() == uiModifier)
        return true;

    return find(m_vModifierKeys.begin(), m_vModifierKeys.end(), uiModifier) != m_vModifierKeys.end();
}


/*====================
  IGameEntity::GetActiveExclusiveModifiers
  ====================*/
void    IGameEntity::GetActiveExclusiveModifiers(IUnitEntity *pUnit, map<uint, SModifierEntry> &mapActiveModifiers, int iPriorityAdjust)
{
    IEntityDefinition *pDefinition(GetBaseDefinition<IEntityDefinition>());
    if (pDefinition == NULL)
        return;

    const map<ushort, IEntityDefinition*> &mapModifiers(pDefinition->GetModifiers());
    for (map<ushort, IEntityDefinition*>::const_iterator cit(mapModifiers.begin()), citEnd(mapModifiers.end()); cit != citEnd; ++cit)
    {
        IEntityDefinition *pModifier(cit->second);

        if (!pModifier->GetExclusive())
            continue;

        // Check conditions
        const tstring &sCondition(pModifier->GetCondition());
        if (pUnit != NULL && !sCondition.empty())
        {
            tsvector vsTypes(TokenizeString(sCondition, _T(' ')));

            tsvector_cit itType(vsTypes.begin()), itTypeEnd(vsTypes.end());
            for (; itType != itTypeEnd; ++itType)
            {
                if (!itType->empty() && (*itType)[0] == _T('!'))
                {
                    if (pUnit->IsTargetType(itType->substr(1), pUnit))
                        break;
                }
                else
                {
                    if (!pUnit->IsTargetType(*itType, pUnit))
                        break;
                }
            }

            // Not active if we searched the entire vector, so skip this modifier
            if (itType != itTypeEnd)
                continue;
        }

        map<uint, SModifierEntry>::iterator itFind(mapActiveModifiers.find(pModifier->GetModifierID()));
        if (itFind != mapActiveModifiers.end())
        {
            if ((pModifier->GetPriority() + iPriorityAdjust) > itFind->second.iPriority)
            {
                SModifierEntry sEntry;
                sEntry.iPriority = pModifier->GetPriority() + iPriorityAdjust;
                sEntry.uiGameIndex = GetIndex();

                mapActiveModifiers[pModifier->GetModifierID()] = sEntry;
            }
        }
        else
        {
            SModifierEntry sEntry;
            sEntry.iPriority = pModifier->GetPriority() + iPriorityAdjust;
            sEntry.uiGameIndex = GetIndex();

            mapActiveModifiers[pModifier->GetModifierID()] = sEntry;
        }
    }
}


/*====================
  IGameEntity::SetActiveModifierKey
  ====================*/
void    IGameEntity::SetActiveModifierKey(const tstring &sModifierKey)
{
    m_uiActiveModifierKey = EntityRegistry.RegisterModifier(sModifierKey);
}
