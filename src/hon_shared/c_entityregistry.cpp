// (C)2006 S2 Games
// c_entityregistry.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_entityregistry.h"

#include "i_projectile.h"
#include "i_heroentity.h"
#include "c_herodefinition.h"
#include "c_gameinfo.h"
#include "i_entityitem.h"
#include "c_itemdefinition.h"
#include "c_scriptthread.h"

#include "../k2/c_entitysnapshot.h"
#include "../k2/c_resourcemanager.h"
#include "../k2/c_resourceinfo.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CEntityRegistry &g_EntityRegistry(*CEntityRegistry::GetInstance());
SINGLETON_INIT(CEntityRegistry);
//=============================================================================


/*====================
  IEntityAllocator::~IEntityAllocator
  ====================*/
IEntityAllocator::~IEntityAllocator()
{
    SAFE_DELETE(m_pBaseline);
}


/*====================
  IEntityAllocator::IEntityAllocator
  ====================*/
IEntityAllocator::IEntityAllocator(const tstring &sName, ushort unID) :
m_sName(sName),
m_unID(unID),
m_pBaseline(nullptr)
{
    EntityRegistry.Register(this);
}


/*====================
  CDynamicEntityAllocator::CDynamicEntityAllocator
  ====================*/
CDynamicEntityAllocator::CDynamicEntityAllocator(const tstring &sName, ushort unTypeID, ResHandle hDefinition, IBaseEntityAllocator *pBaseAllocator) :
m_sName(sName),
m_unTypeID(unTypeID),
m_hDefinition(hDefinition),
m_pBaseAllocator(pBaseAllocator)
{
}


/*====================
  CEntityRegistry::~CEntityRegistry
  ====================*/
CEntityRegistry::~CEntityRegistry()
{
    for (map<tstring, CScriptThread *>::iterator it(m_mapDefinitions.begin()), itEnd(m_mapDefinitions.end()); it != itEnd; ++it)
        K2_DELETE(it->second);
}


/*====================
  CEntityRegistry::CEntityRegistry
  ====================*/
CEntityRegistry::CEntityRegistry() :
m_uiModifierCount(1), // 0 is invalid as a modifier ID
m_uiCooldownTypeCount(0)
{
}


/*====================
  CEntityRegistry::RegisterModifier
  ====================*/
uint    CEntityRegistry::RegisterModifier(const tstring &sModifier)
{
    if (sModifier.empty())
        return INVALID_INDEX;

    map<tstring, uint>::iterator itFind(m_mapModifiers.find(sModifier));
    if (itFind != m_mapModifiers.end())
        return itFind->second;

    uint uiModifierID(m_uiModifierCount);
    m_mapModifiers[sModifier] = uiModifierID;
    ++m_uiModifierCount;
    return uiModifierID;
}


/*====================
  CEntityRegistry::RegisterCooldownType
  ====================*/
uint    CEntityRegistry::RegisterCooldownType(const tstring &sCooldownType)
{
    if (sCooldownType.empty())
        return 0;

    map<tstring, uint>::iterator itFind(m_mapCooldownTypes.find(sCooldownType));
    if (itFind != m_mapCooldownTypes.end())
        return itFind->second;

    uint uiCooldownID(m_uiCooldownTypeCount + 1);
    m_mapCooldownTypes[sCooldownType] = uiCooldownID;
    ++m_uiCooldownTypeCount;
    return uiCooldownID;
}


/*====================
  CEntityRegistry::Register
  ====================*/
void    CEntityRegistry::Register(IEntityAllocator* pAllocator)
{
    try
    {
        // Make sure that the name is not in use
        EntAllocatorNameMap_it  itFindName(m_mapAllocatorNames.find(LowerString(pAllocator->GetName())));
        if (itFindName != m_mapAllocatorNames.end())
            EX_FATAL(_T("Entity with this name already has an allocator registered: ") + pAllocator->GetName());

        // Assign an ID to this entity and associate it with the allocator
        m_mapAllocatorNames.insert(EntAllocatorNameEntry(LowerString(pAllocator->GetName()), pAllocator));
        m_mapAllocatorIDs.insert(EntAllocatorIDEntry(pAllocator->GetID(), pAllocator));
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityRegistry::Register() - "), NO_THROW);
    }
}


/*====================
  CEntityRegistry::RegisterDynamicEntity
  ====================*/
ushort  CEntityRegistry::RegisterDynamicEntity(const tstring &sName, ResHandle hDefinition, IBaseEntityAllocator *pAllocator)
{
    // Check for collisions
    DynamicEntityNameMap::iterator itFind(m_mapDynamicNames.find(sName));
    if (itFind != m_mapDynamicNames.end())
    {
        if (hDefinition == INVALID_RESOURCE)
        {
            // Clear by inserting a dummy entry
            ushort unTypeID(itFind->second.GetTypeID());
            CDynamicEntityAllocator entry(sName, unTypeID, INVALID_RESOURCE, nullptr);

            m_mapDynamicNames.erase(itFind);
            m_mapDynamicTypeIDs.erase(m_mapDynamicTypeIDs.find(unTypeID));

            m_mapDynamicNames.insert(DynamicEntityNameEntry(sName, entry));
            m_mapDynamicTypeIDs.insert(DynamicEntityTypeIDEntry(unTypeID, entry));

            return unTypeID;
        }
        else if (itFind->second.GetDefinitionHandle() == INVALID_RESOURCE)
        {
            // Insert a new entry
            ushort unTypeID(itFind->second.GetTypeID());
            CDynamicEntityAllocator entry(sName, unTypeID, hDefinition, pAllocator);

            m_mapDynamicNames.erase(itFind);
            m_mapDynamicTypeIDs.erase(m_mapDynamicTypeIDs.find(unTypeID));

            m_mapDynamicNames.insert(DynamicEntityNameEntry(sName, entry));
            m_mapDynamicTypeIDs.insert(DynamicEntityTypeIDEntry(unTypeID, entry));

            return unTypeID;
        }
        else
        {
            Console << _T("CEntityRegistry::RegisterDynamicEntity() - Duplicate name: ") << sName << newl;
            return itFind->second.GetTypeID();
        }
    }

    ushort unTypeID(Entity_Dynamic + ushort(m_mapDynamicNames.size()));
    CDynamicEntityAllocator entry(sName, unTypeID, hDefinition, pAllocator);
    m_mapDynamicNames.insert(DynamicEntityNameEntry(sName, entry));
    m_mapDynamicTypeIDs.insert(DynamicEntityTypeIDEntry(unTypeID, entry));

    return unTypeID;
}

void    CEntityRegistry::RegisterDynamicEntity(ushort unTypeID, ResHandle hDefinition)
{
    CEntityDefinitionResource *pResource(g_ResourceManager.Get<CEntityDefinitionResource>(hDefinition));
    if (pResource == nullptr)
        return;
    IEntityDefinition *pDefinition(pResource->GetDefinition<IEntityDefinition>());
    if (pDefinition == nullptr)
        return;

    const tstring &sName(pResource->GetName());
    CDynamicEntityAllocator entry(sName, unTypeID, hDefinition, pDefinition->GetAllocator());

    if (m_mapDynamicNames.find(sName) != m_mapDynamicNames.end())
        m_mapDynamicNames.erase(sName);

    if (m_mapDynamicTypeIDs.find(unTypeID) != m_mapDynamicTypeIDs.end())
        m_mapDynamicTypeIDs.erase(unTypeID);

    m_mapDynamicNames.insert(DynamicEntityNameEntry(sName, entry));
    m_mapDynamicTypeIDs.insert(DynamicEntityTypeIDEntry(unTypeID, entry));

    pDefinition->SetTypeID(unTypeID);
}


/*====================
  CEntityRegistry::Allocate
  ====================*/
IGameEntity*    CEntityRegistry::Allocate(ushort unType)
{
    try
    {
        if (unType >= Entity_Dynamic)
            return AllocateDynamicEntity(unType);

        EntAllocatorIDMap_it    itFind(m_mapAllocatorIDs.find(unType));
        if (itFind == m_mapAllocatorIDs.end())
            EX_ERROR(_T("No allocator found for entity type: ") + XtoA(unType, FMT_PADZERO, 6, 16));

        IGameEntity *pNewEnt(itFind->second->Allocate());
        return pNewEnt;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityRegistry::Allocate() - "), NO_THROW);
        return nullptr;
    }
}

IGameEntity*    CEntityRegistry::Allocate(const tstring &sName)
{
    try
    {
        EntAllocatorNameMap_it  itFind(m_mapAllocatorNames.find(LowerString(sName)));
        if (itFind == m_mapAllocatorNames.end())
            EX_ERROR(_T("No allocator found for entity: ") + sName);

        IGameEntity *pNewEnt(itFind->second->Allocate());
        return pNewEnt;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityRegistry::Allocate() - "), NO_THROW);
        return nullptr;
    }
}


/*====================
  CEntityRegistry::AllocateDynamicEntity
  ====================*/
IGameEntity*    CEntityRegistry::AllocateDynamicEntity(const tstring &sName, uint uiBaseType)
{
    PROFILE("CEntityRegistry::AllocateDynamicEntity");

    DynamicEntityNameMap::iterator itFind(m_mapDynamicNames.find(sName));
    if (itFind == m_mapDynamicNames.end())
        return nullptr;

    // Allocator type must fully contain the desired base type
    uint uiType(itFind->second.GetBaseType());

    if ((GET_ENTITY_BASE_TYPE0(uiBaseType) != 0 && GET_ENTITY_BASE_TYPE0(uiType) != GET_ENTITY_BASE_TYPE0(uiBaseType)) || 
        (GET_ENTITY_BASE_TYPE1(uiBaseType) != 0 && GET_ENTITY_BASE_TYPE1(uiType) != GET_ENTITY_BASE_TYPE1(uiBaseType)) ||
        (GET_ENTITY_BASE_TYPE2(uiBaseType) != 0 && GET_ENTITY_BASE_TYPE2(uiType) != GET_ENTITY_BASE_TYPE2(uiBaseType)) ||
        (GET_ENTITY_BASE_TYPE3(uiBaseType) != 0 && GET_ENTITY_BASE_TYPE3(uiType) != GET_ENTITY_BASE_TYPE3(uiBaseType)) ||
        (GET_ENTITY_BASE_TYPE4(uiBaseType) != 0 && GET_ENTITY_BASE_TYPE4(uiType) != GET_ENTITY_BASE_TYPE4(uiBaseType)))
    {
        Console << _T("Type mismatch for entity: ") << sName << newl;
        return nullptr;
    }

    return itFind->second.Allocate();
}

IGameEntity*    CEntityRegistry::AllocateDynamicEntity(ushort unTypeID, uint uiBaseType)
{
    PROFILE("CEntityRegistry::AllocateDynamicEntity");

    DynamicEntityTypeIDMap::iterator itFind(m_mapDynamicTypeIDs.find(unTypeID));
    if (itFind == m_mapDynamicTypeIDs.end())
        return nullptr;

    // Allocator type must fully contain the desired base type
    uint uiType(itFind->second.GetBaseType());

    if ((GET_ENTITY_BASE_TYPE0(uiBaseType) != 0 && GET_ENTITY_BASE_TYPE0(uiType) != GET_ENTITY_BASE_TYPE0(uiBaseType)) || 
        (GET_ENTITY_BASE_TYPE1(uiBaseType) != 0 && GET_ENTITY_BASE_TYPE1(uiType) != GET_ENTITY_BASE_TYPE1(uiBaseType)) ||
        (GET_ENTITY_BASE_TYPE2(uiBaseType) != 0 && GET_ENTITY_BASE_TYPE2(uiType) != GET_ENTITY_BASE_TYPE2(uiBaseType)) ||
        (GET_ENTITY_BASE_TYPE3(uiBaseType) != 0 && GET_ENTITY_BASE_TYPE3(uiType) != GET_ENTITY_BASE_TYPE3(uiBaseType)) ||
        (GET_ENTITY_BASE_TYPE4(uiBaseType) != 0 && GET_ENTITY_BASE_TYPE4(uiType) != GET_ENTITY_BASE_TYPE4(uiBaseType)))
    {
        Console << _T("Type mismatch for entity type: ") << unTypeID << newl;
        return nullptr;
    }

    return itFind->second.Allocate();
}


/*====================
  CEntityRegistry::GetTypeVector
  ====================*/
const TypeVector*   CEntityRegistry::GetTypeVector(ushort unType) const
{
    try
    {
        if (unType < Entity_Dynamic)
        {
            EntAllocatorIDMap_cit citFind(m_mapAllocatorIDs.find(unType));
            if (citFind == m_mapAllocatorIDs.end())
                EX_ERROR(_T("No allocator found for entity type: ") + SHORT_HEX_TSTR(unType));

            return citFind->second->GetTypeVector();
        }
        else
        {
            DynamicEntityTypeIDMap::const_iterator citFind(m_mapDynamicTypeIDs.find(unType));
            if (citFind == m_mapDynamicTypeIDs.end())
                EX_ERROR(_T("No entry found for entity type: ") + SHORT_HEX_TSTR(unType));

            return citFind->second.GetTypeVector();
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityRegistry::GetTypeVector() - "), NO_THROW);
        return nullptr;
    }
}


/*====================
  CEntityRegistry::GetTypeDesc
  ====================*/
const SEntityDesc*  CEntityRegistry::GetTypeDesc(ushort unType) const
{
    try
    {
        if (unType < Entity_Dynamic)
        {
            EntAllocatorIDMap_cit citFind(m_mapAllocatorIDs.find(unType));
            if (citFind == m_mapAllocatorIDs.end())
                EX_ERROR(_T("No allocator found for entity type: ") + SHORT_HEX_TSTR(unType));

            return citFind->second->GetTypeDesc();
        }
        else
        {
            DynamicEntityTypeIDMap::const_iterator citFind(m_mapDynamicTypeIDs.find(unType));
            if (citFind == m_mapDynamicTypeIDs.end())
                EX_ERROR(_T("No entry found for entity type: ") + SHORT_HEX_TSTR(unType));

            return citFind->second.GetTypeDesc();
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityRegistry::GetTypeDesc() - "), NO_THROW);
        return nullptr;
    }
}


/*====================
  CEntityRegistry::ServerPrecache
  ====================*/
void    CEntityRegistry::ServerPrecache(ushort unType, EPrecacheScheme eScheme, const tstring &sModifier) const
{
    try
    {
        EntAllocatorIDMap_cit citStatic(m_mapAllocatorIDs.find(unType));
        if (citStatic != m_mapAllocatorIDs.end())
        {
            citStatic->second->ServerPrecache(eScheme, sModifier);
            return;
        }

        DynamicEntityTypeIDMap::const_iterator  citDynamic(m_mapDynamicTypeIDs.find(unType));
        if (citDynamic != m_mapDynamicTypeIDs.end())
        {
            ResHandle hDefinition(citDynamic->second.GetDefinitionHandle());
            CEntityDefinitionResource *pDefRes(g_ResourceManager.Get<CEntityDefinitionResource>(hDefinition));
            if (pDefRes == nullptr || pDefRes->GetDefinition<IEntityDefinition>() == nullptr)
                return;
            pDefRes->GetDefinition<IEntityDefinition>()->Precache(eScheme, sModifier);
            citDynamic->second.ServerPrecache(eScheme, sModifier);
        }

        // Play nice with other instances
        if (K2System.IsDedicatedServer())
            K2System.Sleep(0);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityRegistry::ServerPrecache() - "), NO_THROW);
    }
}


/*====================
  CEntityRegistry::ClientPrecache
  ====================*/
void    CEntityRegistry::ClientPrecache(ushort unType, EPrecacheScheme eScheme, const tstring &sModifier) const
{
    if (unType == INVALID_ENT_TYPE)
        return;

    EntAllocatorIDMap_cit   citFind(m_mapAllocatorIDs.find(unType));
    if (citFind != m_mapAllocatorIDs.end())
    {
        citFind->second->ClientPrecache(eScheme, sModifier);
        return;
    }

    DynamicEntityTypeIDMap::const_iterator  citDynamic(m_mapDynamicTypeIDs.find(unType));
    if (citDynamic != m_mapDynamicTypeIDs.end())
    {
        ResHandle hDefinition(citDynamic->second.GetDefinitionHandle());
        CEntityDefinitionResource *pDefRes(g_ResourceManager.Get<CEntityDefinitionResource>(hDefinition));
        if (pDefRes == nullptr || pDefRes->GetDefinition<IEntityDefinition>() == nullptr)
            return;

        pDefRes->GetDefinition<IEntityDefinition>()->Precache(eScheme, sModifier);
        citDynamic->second.ClientPrecache(eScheme, sModifier);
        return;
    }

    Console.Err << _T("No allocator found for entity type: ") << XtoA(unType, FMT_PADZERO, 6, 16) << newl;
}


/*====================
  CEntityRegistry::PostProcess
  ====================*/
void    CEntityRegistry::PostProcess(ushort unType) const
{
    EntAllocatorIDMap_cit   citFind(m_mapAllocatorIDs.find(unType));
    if (citFind != m_mapAllocatorIDs.end())
    {
        citFind->second->PostProcess();
        return;
    }

    DynamicEntityTypeIDMap::const_iterator  citDynamic(m_mapDynamicTypeIDs.find(unType));
    if (citDynamic != m_mapDynamicTypeIDs.end())
    {
        ResHandle hDefinition(citDynamic->second.GetDefinitionHandle());
        CEntityDefinitionResource *pDefRes(g_ResourceManager.Get<CEntityDefinitionResource>(hDefinition));
        if (pDefRes == nullptr || pDefRes->GetDefinition<IEntityDefinition>() == nullptr)
            return;
        pDefRes->GetDefinition<IEntityDefinition>()->PostProcess();
        citDynamic->second.PostProcess();
        return;
    }

    Console.Err << _T("No allocator found for entity type: ") << XtoA(unType, FMT_PADZERO, 6, 16) << newl;
}


/*====================
  CEntityRegistry::LookupID
  ====================*/
ushort  CEntityRegistry::LookupID(const tstring &sName)
{
    PROFILE("CEntityRegistry::LookupID");

    if (sName.empty())
        return INVALID_ENT_TYPE;

    EntAllocatorNameMap_it itFind(m_mapAllocatorNames.find(LowerString(sName)));
    if (itFind != m_mapAllocatorNames.end())
        return itFind->second->GetID();
    DynamicEntityNameMap::iterator itFindDynamic(m_mapDynamicNames.find(sName));
    if (itFindDynamic != m_mapDynamicNames.end())
        return itFindDynamic->second.GetTypeID();

    return INVALID_ENT_TYPE;
}


/*====================
  CEntityRegistry::LookupName
  ====================*/
const tstring&  CEntityRegistry::LookupName(ushort unType)
{
    PROFILE("CEntityRegistry::LookupName");

    if (unType == INVALID_ENT_TYPE)
        return TSNULL;

    EntAllocatorIDMap_it itFind(m_mapAllocatorIDs.find(unType));
    if (itFind != m_mapAllocatorIDs.end())
        return itFind->second->GetName();

    DynamicEntityTypeIDMap::iterator itFindDynamic(m_mapDynamicTypeIDs.find(unType));
    if (itFindDynamic != m_mapDynamicTypeIDs.end())
        return itFindDynamic->second.GetName();

    return TSNULL;
}


/*====================
  CEntityRegistry::LookupModifierKey

  This is not meant for use in game, it is slow!
  ====================*/
const tstring&  CEntityRegistry::LookupModifierKey(uint uiID) const
{
    for (map<tstring, uint>::const_iterator it(m_mapModifiers.begin()); it != m_mapModifiers.end(); ++ it)
    {
        if (it->second == uiID)
            return it->first;
    }

    return TSNULL;
}


/*====================
  CEntityRegistry::LookupModifierKey
  ====================*/
uint    CEntityRegistry::LookupModifierKey(const tstring &sModifier) const
{
    map<tstring, uint>::const_iterator itFind(m_mapModifiers.find(sModifier));
    if (itFind != m_mapModifiers.end())
        return itFind->second;
    else
        return INVALID_INDEX;
}


/*====================
  CEntityRegistry::GetBaseType
  ====================*/
uint    CEntityRegistry::GetBaseType(ushort unTypeID)
{
    DynamicEntityTypeIDMap::iterator itFind(m_mapDynamicTypeIDs.find(unTypeID));
    if (itFind == m_mapDynamicTypeIDs.end())
        return ENTITY_BASE_TYPE_INVALID;

    return itFind->second.GetBaseType();
}


/*====================
  CEntityRegistry::GetAllocator
  ====================*/
const IEntityAllocator* CEntityRegistry::GetAllocator(ushort unTypeID)
{
    EntAllocatorIDMap_it itFind(m_mapAllocatorIDs.find(unTypeID));
    if (itFind == m_mapAllocatorIDs.end())
        return nullptr;

    return itFind->second;
}


/*====================
  CEntityRegistry::GetDynamicAllocator
  ====================*/
const CDynamicEntityAllocator*  CEntityRegistry::GetDynamicAllocator(ushort unTypeID)
{
    DynamicEntityTypeIDMap::iterator itFind(m_mapDynamicTypeIDs.find(unTypeID));
    if (itFind == m_mapDynamicTypeIDs.end())
        return nullptr;
    
    return &(itFind->second);
}


/*====================
  CEntityRegistry::GetGameSetting
  ====================*/
ICvar*  CEntityRegistry::GetGameSetting(ushort unType, const tstring &sSetting) const
{
    try
    {
        EntAllocatorIDMap_cit   citFind(m_mapAllocatorIDs.find(unType));
        if (citFind == m_mapAllocatorIDs.end())
            EX_ERROR(_T("No allocator found for entity ID: ") + SHORT_HEX_TSTR(unType) + _T(", could not retrieve setting \"") + sSetting + _T("\""));

        return citFind->second->GetGameSetting(sSetting);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityRegistry::GetGameSetting() - "), NO_THROW);
        return nullptr;
    }
}


/*====================
  CEntityRegistry::GetEntityList
  ====================*/
void    CEntityRegistry::GetEntityList(map<ushort, tstring> &mapEntities) const
{
    for (EntAllocatorIDMap_cit cit(m_mapAllocatorIDs.begin()); cit != m_mapAllocatorIDs.end(); ++cit)
        mapEntities.insert(pair<ushort, tstring>(cit->first, cit->second->GetName()));
    for (DynamicEntityTypeIDMap::const_iterator cit(m_mapDynamicTypeIDs.begin()); cit != m_mapDynamicTypeIDs.end(); ++cit)
        mapEntities.insert(pair<ushort, tstring>(cit->first, cit->second.GetName()));
}


/*====================
  CEntityRegistry::WriteDynamicEntities
  ====================*/
void    CEntityRegistry::WriteDynamicEntities(IBuffer &buffer)
{
    buffer.Clear();

    for (DynamicEntityTypeIDMap::iterator it(m_mapDynamicTypeIDs.begin()); it != m_mapDynamicTypeIDs.end(); ++it)
        buffer << it->second.GetTypeID() << NetworkResourceManager.GetNetIndex(it->second.GetDefinitionHandle());
}


/*====================
  CEntityRegistry::GetHeroList
  ====================*/
void    CEntityRegistry::GetHeroList(const tstring &sTeamName, vector<ushort> &vHeroes, EAttribute eAttribute)
{
    for (const CDynamicEntityAllocator& it : m_mapDynamicTypeIDs | values)
    {
        if (it.GetBaseType() != ENTITY_BASE_TYPE_HERO)
            continue;
        CEntityDefinitionResource *pResource(g_ResourceManager.Get<CEntityDefinitionResource>(it.GetDefinitionHandle()));
        if (pResource == nullptr)
            continue;
        CHeroDefinition *pDefinition(pResource->GetDefinition<CHeroDefinition>());
        if (pDefinition == nullptr)
            continue;
        if (pDefinition->GetTeam().empty())
            continue;
        if (CompareNoCase(pDefinition->GetTeam(), sTeamName) != 0)
            continue;
        
        if (pDefinition->GetPrimaryAttribute() == ATTRIBUTE_INVALID)
            continue;

        if (pDefinition->GetPrimaryAttribute() == ATTRIBUTE_AGILITY && 
            (Game.HasGameOptions(GAME_OPTION_NO_AGILITY) || (eAttribute != ATTRIBUTE_INVALID && eAttribute != ATTRIBUTE_AGILITY)))
            continue;
        if (pDefinition->GetPrimaryAttribute() == ATTRIBUTE_INTELLIGENCE &&
            (Game.HasGameOptions(GAME_OPTION_NO_INTELLIGENCE) || (eAttribute != ATTRIBUTE_INVALID && eAttribute != ATTRIBUTE_INTELLIGENCE)))
            continue;
        if (pDefinition->GetPrimaryAttribute() == ATTRIBUTE_STRENGTH &&
            (Game.HasGameOptions(GAME_OPTION_NO_STRENGTH) || (eAttribute != ATTRIBUTE_INVALID && eAttribute != ATTRIBUTE_STRENGTH)))
            continue;
        
        vHeroes.push_back(it.GetTypeID());
    }
}


/*====================
  CEntityRegistry::GetAutoRecipeList
  ====================*/
void    CEntityRegistry::GetAutoRecipeList(vector<ushort> &vRecipes)
{
    for (const CDynamicEntityAllocator& it : m_mapDynamicTypeIDs | values)
    {
        if (it.GetBaseType() != ENTITY_BASE_TYPE_ITEM)
            continue;
        CEntityDefinitionResource *pResource(g_ResourceManager.Get<CEntityDefinitionResource>(it.GetDefinitionHandle()));
        if (pResource == nullptr)
            continue;
        CItemDefinition *pDefinition(pResource->GetDefinition<CItemDefinition>());
        if (pDefinition == nullptr)
            continue;
        if (!pDefinition->GetAutoAssemble())
            continue;
        
        vRecipes.push_back(it.GetTypeID());
    }
}


/*====================
  CEntityRegistry::GetShopList
  ====================*/
void    CEntityRegistry::GetShopList(vector<ushort> &vShops)
{
    for (const CDynamicEntityAllocator& it : m_mapDynamicTypeIDs | values)
    {
        if (it.GetBaseType() != ENTITY_BASE_TYPE_SHOP)
            continue;
        
        vShops.push_back(it.GetTypeID());
    }
}


/*====================
  CEntityRegistry::GetItemList
  ====================*/
void    CEntityRegistry::GetItemList(vector<ushort> &vItems)
{
    for (DynamicEntityTypeIDMap::iterator it(m_mapDynamicTypeIDs.begin()); it != m_mapDynamicTypeIDs.end(); ++it)
    {
        if (it->second.GetBaseType() != ENTITY_BASE_TYPE_ITEM)
            continue;
        
        vItems.push_back(it->second.GetTypeID());
    }
}


/*====================
  CEntityRegistry::GetEntityDef
  ====================*/
CEntityDefinitionResource*  CEntityRegistry::GetEntityDef(ResHandle hEntDef)
{
    return g_ResourceManager.Get<CEntityDefinitionResource>(hEntDef);
}


/*====================
  CEntityRegistry::NewScriptThread
  ====================*/
CScriptThread*  CEntityRegistry::NewScriptThread(const tstring &sName)
{
    map<tstring, CScriptThread *>::iterator itFind(m_mapDefinitions.find(sName));
    if (itFind != m_mapDefinitions.end())
        K2_DELETE(itFind->second);

    CScriptThread *pNewThread(K2_NEW(ctx_Game,    CScriptThread)(sName));

    m_mapDefinitions[sName] = pNewThread;

    return pNewThread;
}


/*====================
  CEntityRegistry::GetScriptDefinition
  ====================*/
CScriptThread*  CEntityRegistry::GetScriptDefinition(const tstring &sName)
{
    map<tstring, CScriptThread *>::iterator itFind(m_mapDefinitions.find(sName));
    if (itFind != m_mapDefinitions.end())
        return itFind->second;
    else
        return nullptr;
}


/*====================
  CEntityRegistry::PrecacheScripts
  ====================*/
void    CEntityRegistry::PrecacheScripts()
{
    for (map<tstring, CScriptThread *>::iterator it(m_mapDefinitions.begin()), itEnd(m_mapDefinitions.end()); it != itEnd; ++it)
        it->second->Precache(PRECACHE_ALL, _T("All"));
}


/*--------------------
  EntityList
  --------------------*/
CMD(EntityList)
{
    map<ushort, tstring> mapEntities;
    EntityRegistry.GetEntityList(mapEntities);
    for (map<ushort, tstring>::iterator it(mapEntities.begin()); it != mapEntities.end(); ++it)
        Console << _T("^960[") << XtoA(it->first, 0, 3) << _T("] ^c") << it->second << newl;

    return true;
}
