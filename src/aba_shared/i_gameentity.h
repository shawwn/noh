// (C)2005 S2 Games
// i_gameentity.h
//
//=============================================================================
#ifndef __I_GAMEENTITY_H__
#define __I_GAMEENTITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_entityevent.h"
#include "c_entitydefinitionresource.h"

#include "../k2/c_entitysnapshot.h"
#include "../k2/s_traceinfo.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class IGameEntity;
class IPropEntity;
class CPlayer;
class IProjectile;
class IAffector;
class CLinearAffector;
class ILight;
class IGadgetEntity;
class IBuildingEntity;
class IPropEntity;
class IUnitEntity;
class IHeroEntity;
class ICreepEntity;
class IPetEntity;
class IVisualEntity;
class ISlaveEntity;
class IEntityTool;
class IEntityAbility;
class IEntityAbilityAttribute;
class IEntityItem;
class IEntityState;
class CWorldEntity;
class INeutralEntity;
class ICritterEntity;
class IOrderEntity;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define START_ENTITY_BASE_CONFIG \
public: \
class CEntityConfig \
{ \
private: \
    CEntityConfig();

#define END_ENTITY_BASE_CONFIG \
public: \
    CEntityConfig(const tstring &sName); \
}; \
protected:

#define START_ENTITY_CONFIG(parent) \
public: \
class CEntityConfig : public parent::CEntityConfig \
{ \
private: \
    CEntityConfig();

#define END_ENTITY_CONFIG \
public: \
    CEntityConfig(const tstring &sName); \
}; \
protected:

#define DECLARE_ENTITY_CVAR(type, name) \
private: \
    CCvar<type, type>   m_cvar##name; \
public: \
    const CCvar<type, type>&    Get##name() const   { return m_cvar##name; }

#define DECLARE_ENTITY_ARRAY_CVAR(type, name) \
private: \
    CArrayCvar<type>    m_cvar##name; \
public: \
    const CArrayCvar<type>& Get##name() const   { return m_cvar##name; }

#define INIT_ENTITY_CVAR(name, def) m_cvar##name(sName + _T("_") _T(#name), def, CVAR_GAMECONFIG | CVAR_TRANSMIT)

#define ENTITY_CVAR_ACCESSOR(type, name) \
virtual type    Get##name() const \
{ \
    if (m_pEntityConfig == NULL) \
        return GetDefaultEmptyValue<type>(); \
\
    return m_pEntityConfig->Get##name().GetValue(); \
}

#define ENTITY_ARRAY_CVAR_ACCESSOR(type, name) \
    virtual type    Get##name(uint uiIndex) const   { if (m_pEntityConfig == NULL) return GetDefaultEmptyValue<type>(); return m_pEntityConfig->Get##name().GetValue(uiIndex); } \
    uint            Get##name##Size() const         { if (m_pEntityConfig == NULL) return 0; return m_pEntityConfig->Get##name().GetSize(); }

#define ENTITY_MULTI_LEVEL_CVAR_ACCESSOR(type, name) \
virtual type    Get##name() const \
{ \
    if (m_pEntityConfig == NULL || m_pEntityConfig->Get##name().GetSize() == 0) \
        return GetDefaultEmptyValue<type>(); \
    if (!IsActive()) \
        return GetDefaultEmptyValue<type>(); \
\
    return m_pEntityConfig->Get##name().GetValue(MIN(MAX(1u, GetLevel()) - 1, m_pEntityConfig->Get##name().GetSize() - 1)); \
}

// DECLARE_ENTITY_DESC
#define DECLARE_ENTITY_DESC \
private: \
    static SEntityDesc  s_cDesc; \
    static void     InitFieldTypes(); \
public: \
    static const SEntityDesc*   GetStaticTypeDesc()     { return &s_cDesc; } \
    static uint GetVersion()                            { return s_cDesc.uiVersion; } \
    static void InitTypeDesc(CEntitySnapshot *pBaseline); \
    GAME_SHARED_API static const TypeVector&    GetTypeVector();

// DEFINE_ENTITY_DESC
#define DEFINE_ENTITY_DESC(type, version) \
SEntityDesc     type::s_cDesc; \
void    type::InitTypeDesc(CEntitySnapshot *pBaseline) \
{ \
    GetTypeVector(); \
    s_cDesc.uiSize = CEntitySnapshot::CalcSnapshotSize(s_cDesc.pFieldTypes); \
    s_cDesc.pBaseline = pBaseline; \
    s_cDesc.uiVersion = version; \
} \
const TypeVector&   type::GetTypeVector() \
{ \
    if (!s_cDesc.pFieldTypes) \
        InitFieldTypes(); \
    return *s_cDesc.pFieldTypes; \
} \
void    type::InitFieldTypes()


// DECLARE_SUB_ENTITY_ACCESSOR
#define DECLARE_SUB_ENTITY_ACCESSOR(type, name) \
virtual bool                Is##name() const    { return false; } \
virtual class type*         GetAs##name()       { return NULL; } \
virtual const class type*   GetAs##name() const { return NULL; }

// SUB_ENTITY_ACCESSOR
#define SUB_ENTITY_ACCESSOR(type, name) \
private: \
    static uint     s_uiBaseType; \
public: \
    bool                    Is##name() const    { return true; } \
    class type*             GetAs##name()       { return this; } \
    const class type*       GetAs##name() const { return this; } \
    static const tstring&   GetBaseTypeName()   { static const tstring *sBaseTypeName = K2_NEW(g_heapSharedGame2,   tstring)(_T(#name)); return *sBaseTypeName; } \
    static uint             GetBaseType()       { return s_uiBaseType; }

// ENTITY_DEFINITION_ACCESSOR
#define ENTITY_DEFINITION_ACCESSOR(type, name) \
virtual type    Get##name() const \
{ \
    if (m_pDefinition == NULL) \
        return GetDefaultEmptyValue<type>(); \
\
    return static_cast<TDefinition *>(m_pDefinition)->Get##name(); \
}

// MUTABLE_ATTRIBUTE
#define MUTABLE_ATTRIBUTE(type, name) \
virtual type    Get##name() const \
{ \
    if (m_pMorphState != NULL && m_pMorphState->GetApply##name##Morph()) \
        return m_pMorphState->GetMorph##name(); \
\
    if (m_pDefinition == NULL) \
        return GetDefaultEmptyValue<type>(); \
\
    return static_cast<TDefinition *>(m_pDefinition)->Get##name(); \
}

// BASE_ENTITY_DEFINITION_ACCESSOR
#define BASE_ENTITY_DEFINITION_ACCESSOR(type, name) \
virtual type    GetBase##name() const \
{ \
    if (m_pDefinition == NULL) \
        return GetDefaultEmptyValue<type>(); \
\
    return static_cast<TDefinition *>(m_pDefinition)->Get##name(); \
}

// ENTITY_DEFINITION_RESOURCE_ACCESSOR
#define ENTITY_DEFINITION_RESOURCE_ACCESSOR(name) \
virtual ResHandle   Get##name() const \
{ \
    if (m_pDefinition == NULL) \
        return INVALID_RESOURCE; \
\
    return static_cast<TDefinition *>(m_pDefinition)->Get##name(); \
}

// ENTITY_DEFINITION_LOCALIZED_STRING_ACCESSOR
#define ENTITY_DEFINITION_LOCALIZED_STRING_ACCESSOR(name) \
virtual const tstring&  Get##name() const \
{ \
    if (m_pDefinition == NULL) \
        return TSNULL; \
\
    return static_cast<TDefinition *>(m_pDefinition)->Get##name(); \
} \
virtual uint    Get##name##Index() const \
{ \
    if (m_pDefinition == NULL) \
        return INVALID_INDEX; \
\
    return static_cast<TDefinition *>(m_pDefinition)->Get##name##Index(); \
}

// ENTITY_DEFINITION_ARRAY_ACCESSOR
#define ENTITY_DEFINITION_ARRAY_ACCESSOR(type, name) \
virtual type    Get##name(uint uiIndex) const \
{ \
    if (m_pDefinition == NULL) \
        return GetDefaultEmptyValue<type>(); \
\
    return static_cast<TDefinition *>(m_pDefinition)->Get##name(MIN(uiIndex, Get##name##Size() - 1)); \
} \
\
virtual uint    Get##name##Size() const \
{ \
    if (m_pDefinition == NULL) \
        return 0; \
\
    return static_cast<TDefinition *>(m_pDefinition)->Get##name##Size(); \
}

// MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR
#define MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(type, name) \
virtual type    Get##name() const \
{ \
    if (!IsActive()) \
        return GetDefaultEmptyValue<type>(); \
    if (m_pDefinition == NULL) \
        return GetDefaultEmptyValue<type>(); \
\
    return static_cast<TDefinition *>(m_pDefinition)->Get##name(MAX(1u, GetLevel()) - 1); \
}

// MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR_PASSIVE
#define MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR_PASSIVE(type, name) \
virtual type    Get##name() const \
{ \
    if (m_pDefinition == NULL) \
        return GetDefaultEmptyValue<type>(); \
\
    return static_cast<TDefinition *>(m_pDefinition)->Get##name(MAX(1u, GetLevel()) - 1); \
}

// MULTI_LEVEL_ENTITY_DEFINITION_RESOURCE_ACCESSOR
#define MULTI_LEVEL_ENTITY_DEFINITION_RESOURCE_ACCESSOR(name) \
virtual ResHandle   Get##name() const \
{ \
    if (m_pDefinition == NULL) \
        return INVALID_RESOURCE; \
\
    return static_cast<TDefinition *>(m_pDefinition)->Get##name(MAX(1u, GetLevel()) - 1); \
} \
virtual const tstring&  Get##name##Path() const \
{ \
    if (m_pDefinition == NULL) \
        return TSNULL; \
\
    return static_cast<TDefinition *>(m_pDefinition)->Get##name##Path(MAX(1u, GetLevel()) - 1); \
}

// ENTITY_DEFINITION_TEMPORAL_ACCESSOR
#define ENTITY_DEFINITION_TEMPORAL_ACCESSOR(type, name) \
virtual type    Get##name() const \
{ \
    if (!IsActive()) \
        return GetDefaultEmptyValue<type>(); \
    if (m_pDefinition == NULL) \
        return GetDefaultEmptyValue<type>(); \
\
    TDefinition *pDefinition(static_cast<TDefinition *>(m_pDefinition)); \
    CTemporalProperty<type> tp(pDefinition->Get##name##Start(), pDefinition->Get##name##End(), pDefinition->Get##name##Mid(), pDefinition->Get##name##MidPos(), pDefinition->Get##name##Speed()); \
    uint uiStartTime(m_uiCreationTime + pDefinition->Get##name##Delay()); \
    uint uiTime(MAX(Game.GetGameTime(), uiStartTime) - uiStartTime); \
    uint uiDuration(pDefinition->Get##name##Duration() > 0 ? pDefinition->Get##name##Duration() : GetLifetime()); \
    float fLerp(uiDuration > 0 ? CLAMP(uiTime / float(uiDuration), 0.0f, 1.0f) : 1.0f); \
    return tp.Evaluate(fLerp, MsToSec(uiTime)); \
}

// MULTI_LEVEL_ENTITY_DEFINITION_TEMPORAL_ACCESSOR
#define MULTI_LEVEL_ENTITY_DEFINITION_TEMPORAL_ACCESSOR(type, name) \
virtual type    Get##name() const \
{ \
    if (!IsActive()) \
        return GetDefaultEmptyValue<type>(); \
    if (m_pDefinition == NULL) \
        return GetDefaultEmptyValue<type>(); \
\
    TDefinition *pDefinition(static_cast<TDefinition *>(m_pDefinition)); \
    uint ui(MAX(1u, GetLevel()) - 1); \
    CTemporalProperty<type> tp(pDefinition->Get##name##Start(ui), pDefinition->Get##name##End(ui), pDefinition->Get##name##Mid(ui), pDefinition->Get##name##MidPos(ui), pDefinition->Get##name##Speed(ui)); \
    uint uiStartTime(m_uiCreationTime + pDefinition->Get##name##Delay(ui)); \
    uint uiTime(MAX(Game.GetGameTime(), uiStartTime) - uiStartTime); \
    uint uiDuration(pDefinition->Get##name##Duration(ui) > 0 ? pDefinition->Get##name##Duration(ui) : GetLifetime()); \
    float fLerp(uiDuration > 0 ? CLAMP(uiTime / float(uiDuration), 0.0f, 1.0f) : 1.0f); \
    return tp.Evaluate(fLerp, MsToSec(uiTime)); \
}

// MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_TEMPORAL_ACCESSOR
#define MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_TEMPORAL_ACCESSOR(type, name) \
MULTI_LEVEL_ENTITY_DEFINITION_TEMPORAL_ACCESSOR(TDefinition, type, name) \
MULTI_LEVEL_ENTITY_DEFINITION_TEMPORAL_ACCESSOR(TDefinition, type, Morph##name) \
ENTITY_DEFINITION_ACCESSOR(bool, Apply##name##Morph)

// ENTITY_DEFINITION_MUTATION_ACCESSOR
#define ENTITY_DEFINITION_MUTATION_ACCESSOR(type, name) \
ENTITY_DEFINITION_ACCESSOR(type, Morph##name) \
ENTITY_DEFINITION_ACCESSOR(bool, Apply##name##Morph)

// MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR
#define MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(type, name) \
MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(type, Morph##name) \
ENTITY_DEFINITION_ACCESSOR(bool, Apply##name##Morph)

// PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR
#define PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(type, name) \
virtual type    Get##name() const \
{ \
    if (!IsActive()) \
        return GetDefaultEmptyValue<type>(); \
    if (m_pDefinition == NULL) \
        return GetDefaultEmptyValue<type>(); \
\
    uint uiLevelIndex(MAX(1u, GetLevel()) - 1); \
    return static_cast<TDefinition *>(m_pDefinition)->Get##name(uiLevelIndex) + \
        uiLevelIndex * static_cast<TDefinition *>(m_pDefinition)->Get##name##PerLevel() + \
        GetCharges() * static_cast<TDefinition *>(m_pDefinition)->Get##name##PerCharge(uiLevelIndex); \
}

// PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_TEMPORAL_ACCESSOR
#define PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_TEMPORAL_ACCESSOR(type, name) \
ENTITY_DEFINITION_TEMPORAL_ACCESSOR(type, name##PerLevel) \
MULTI_LEVEL_ENTITY_DEFINITION_TEMPORAL_ACCESSOR(type, name##PerCharge) \
virtual type    Get##name() const \
{ \
    if (!IsActive()) \
        return GetDefaultEmptyValue<type>(); \
    if (m_pDefinition == NULL) \
        return GetDefaultEmptyValue<type>(); \
\
    TDefinition *pDefinition(static_cast<TDefinition *>(m_pDefinition)); \
    uint ui(MAX(1u, GetLevel()) - 1); \
    CTemporalProperty<type> tp(pDefinition->Get##name##Start(ui), pDefinition->Get##name##End(ui), pDefinition->Get##name##Mid(ui), pDefinition->Get##name##MidPos(ui), pDefinition->Get##name##Speed(ui)); \
    uint uiStartTime(m_uiCreationTime + pDefinition->Get##name##Delay(ui)); \
    uint uiTime(MAX(Game.GetGameTime(), uiStartTime) - uiStartTime); \
    uint uiDuration(pDefinition->Get##name##Duration(ui) > 0 ? pDefinition->Get##name##Duration(ui) : GetLifetime()); \
    float fLerp(uiDuration > 0 ? CLAMP(uiTime / float(uiDuration), 0.0f, 1.0f) : 1.0f); \
\
    uint uiLevel(MAX(1u, GetLevel()) - 1); \
    return tp.Evaluate(fLerp, MsToSec(uiTime)) + \
        uiLevel * Get##name##PerLevel() + \
        GetCharges() * Get##name##PerCharge(); \
}

// PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_TEMPORAL_ACCESSOR
#define PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_TEMPORAL_ACCESSOR(type, name) \
PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_TEMPORAL_ACCESSOR(type, name) \
PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_TEMPORAL_ACCESSOR(type, Morph##name) \
ENTITY_DEFINITION_ACCESSOR(bool, Apply##name##Morph)

struct SModifierEntry
{
    int iPriority;
    uint uiGameIndex;
};

struct SEntityDesc
{
    TypeVector*             pFieldTypes;
    uint                    uiSize;
    CEntitySnapshot*        pBaseline;
    uint                    uiVersion;
};

const uint SNAPSHOT_ENEMY   (BIT(0));
const uint SNAPSHOT_HIDDEN  (BIT(1));

const uint TEAM_1           (1);
const uint TEAM_2           (2);
const uint TEAM_SPECTATOR   (0);
const uint TEAM_INVALID     (-4);   // When adding new hard coded teams, keep TEAM_INVALID as the lowest negative value
const uint TEAM_NEUTRAL     (-2);
const uint TEAM_PASSIVE     (-1);
//=============================================================================

inline tstring      GetTeamNameString(uint uiTeam)
{
    if (uiTeam == TEAM_1)
        return _T("Legion");
    if (uiTeam == TEAM_2)
        return _T("Hellbourne");
    if (uiTeam == TEAM_SPECTATOR)
        return _T("Spectator");
    if (uiTeam == TEAM_INVALID)
        return _T("Invalid");
    if (uiTeam == TEAM_NEUTRAL)
        return _T("Neutral");
    if (uiTeam == TEAM_PASSIVE)
        return _T("Passive");

    return _T("Unknown");
}

inline tstring      GetTeamColorString(uint uiTeam)
{
    if (uiTeam == TEAM_1)
        return GetInlineColorString<tstring>(GetColorFromString(_T("#0042FF")));
    if (uiTeam == TEAM_2)
        return GetInlineColorString<tstring>(GetColorFromString(_T("#E55BB0")));
    
    return _T("^888");
}

//=============================================================================
// IGameEntity
//=============================================================================
class IGameEntity
{
    DECLARE_ENTITY_DESC

public:
    typedef IEntityDefinition TDefinition;

protected:
    IEntityDefinition       *m_pDefinition;

public:
    virtual void        UpdateDefinition()
    {
        CEntityDefinitionResource *pResource(g_ResourceManager.Get<CEntityDefinitionResource>(m_hDefinition));
        if (pResource == NULL)
            m_pDefinition = NULL;
        else
            m_pDefinition = pResource->GetDefinition<IEntityDefinition>(GetModifierBits());
    }

private:
    IGameEntity();

protected:
    // Cvar settings
    START_ENTITY_BASE_CONFIG
    END_ENTITY_BASE_CONFIG

    CEntityConfig*      m_pEntityConfig;
    ResHandle           m_hDefinition;

    // Identity
    ushort              m_unType;
    uint                m_uiIndex;
    tstring             m_sTypeName;
    const SEntityDesc*  m_pTypeDesc;
    uint                m_uiUniqueID;
    ushort              m_unModifierBits;
    uint                m_uiActiveModifierKey;
    uivector            m_vModifierKeys;
    uivector            m_vPersistentModifierKeys;

    bool                m_bDelete;
    uint                m_uiFrame;
    bool                m_bValid;

public:
    virtual ~IGameEntity()  {}
    IGameEntity(CEntityConfig *pConfig);

    // Accessors
    ushort              GetType() const                             { return m_unType; }
    void                SetType(ushort unType)                      { m_unType = unType; }
    bool                IsType(ushort unType) const                 { return m_unType == unType; }

    const tstring&      GetTypeName() const                         { return m_sTypeName; }
    void                SetTypeName(const tstring &sTypeName)       { m_sTypeName = sTypeName; }

    const SEntityDesc*  GetTypeDesc() const                         { return m_pTypeDesc; }
    void                SetTypeDesc(const SEntityDesc *pTypeDesc)   { m_pTypeDesc = pTypeDesc; }

    void                SetDefinitionHandle(ResHandle hDefinition)  { m_hDefinition = hDefinition; }
    ResHandle           GetDefinitionHandle() const                 { return m_hDefinition; }

    uint                GetUniqueID() const                         { return m_uiUniqueID; }
    void                SetUniqueID(uint uiUniqueID)                { m_uiUniqueID = uiUniqueID; }

    void                SetDelete(bool bDelete)                     { m_bDelete = bDelete; }
    bool                GetDelete() const                           { return m_bDelete; }

    uint                GetFrame() const                            { return m_uiFrame; }
    void                SetFrame(uint uiFrame)                      { m_uiFrame = uiFrame; }

    bool                IsValid() const                             { return m_bValid; }
    void                Validate()                                  { m_bValid = true; }
    void                Invalidate()                                { m_bValid = false; }
    
    virtual bool        IsStatic() const                { return false; }
    virtual bool        IsServerEntity() const          { return false; }

    virtual void            GetExtendedData(IBuffer &buffer) const  {}
    GAME_SHARED_API void    SendExtendedData(int iClient) const;
    virtual void            ReadExtendedData(CPacket &pkt)          {}

    DECLARE_SUB_ENTITY_ACCESSOR(IPropEntity, Prop)
    DECLARE_SUB_ENTITY_ACCESSOR(CGameInfo, GameInfo)
    DECLARE_SUB_ENTITY_ACCESSOR(CTeamInfo, TeamInfo)
    DECLARE_SUB_ENTITY_ACCESSOR(CGameStats, Stats)
    DECLARE_SUB_ENTITY_ACCESSOR(CPlayer, Player)
    DECLARE_SUB_ENTITY_ACCESSOR(IProjectile, Projectile)
    DECLARE_SUB_ENTITY_ACCESSOR(IAffector, Affector)
    DECLARE_SUB_ENTITY_ACCESSOR(CLinearAffector, LinearAffector)
    DECLARE_SUB_ENTITY_ACCESSOR(ILight, Light)
    DECLARE_SUB_ENTITY_ACCESSOR(IGadgetEntity, Gadget)
    DECLARE_SUB_ENTITY_ACCESSOR(IBuildingEntity, Building)
    DECLARE_SUB_ENTITY_ACCESSOR(IUnitEntity, Unit)
    DECLARE_SUB_ENTITY_ACCESSOR(IHeroEntity, Hero)
    DECLARE_SUB_ENTITY_ACCESSOR(ICreepEntity, Creep)
    DECLARE_SUB_ENTITY_ACCESSOR(IPetEntity, Pet)
    DECLARE_SUB_ENTITY_ACCESSOR(IVisualEntity, Visual)
    DECLARE_SUB_ENTITY_ACCESSOR(ISlaveEntity, Slave)
    DECLARE_SUB_ENTITY_ACCESSOR(IEntityTool, Tool)
    DECLARE_SUB_ENTITY_ACCESSOR(IEntityAbility, Ability)
    DECLARE_SUB_ENTITY_ACCESSOR(IEntityAbilityAttribute, AbilityAttribute)
    DECLARE_SUB_ENTITY_ACCESSOR(IEntityItem, Item)
    DECLARE_SUB_ENTITY_ACCESSOR(IEntityState, State)
    DECLARE_SUB_ENTITY_ACCESSOR(IMorphState, MorphState)
    DECLARE_SUB_ENTITY_ACCESSOR(ITemporalState, TemporalState)
    DECLARE_SUB_ENTITY_ACCESSOR(IBitEntity, Bit)
    DECLARE_SUB_ENTITY_ACCESSOR(IPowerupEntity, Powerup)
    DECLARE_SUB_ENTITY_ACCESSOR(INeutralEntity, Neutral)
    DECLARE_SUB_ENTITY_ACCESSOR(IShopEntity, Shop)
    DECLARE_SUB_ENTITY_ACCESSOR(ICritterEntity, Critter)
    DECLARE_SUB_ENTITY_ACCESSOR(IOrderEntity, Order)
    DECLARE_SUB_ENTITY_ACCESSOR(CEntityNeutralCampSpawner, NeutralCampSpawner)
    DECLARE_SUB_ENTITY_ACCESSOR(CEntityChest, Chest)

    template <class T>
    T*  GetBaseDefinition() const
    {
        CEntityDefinitionResource *pResource(g_ResourceManager.Get<CEntityDefinitionResource>(m_hDefinition));
        if (pResource == NULL)
            return NULL;

        return pResource->GetDefinition<T>();
    }

    template <class T>
    T*  GetDefinition() const
    {
        CEntityDefinitionResource *pResource(g_ResourceManager.Get<CEntityDefinitionResource>(m_hDefinition));
        if (pResource == NULL)
            return NULL;

        return pResource->GetDefinition<T>(m_unModifierBits);
    }

    template <class T>
    T*  GetDefinition(ushort unModifierBits) const
    {
        CEntityDefinitionResource *pResource(g_ResourceManager.Get<CEntityDefinitionResource>(m_hDefinition));
        if (pResource == NULL)
            return NULL;

        return pResource->GetDefinition<T>(unModifierBits);
    }

    template <class T>
    T*  GetActiveDefinition() const
    {
        if (m_pDefinition == NULL)
            return NULL;
        else
            return static_cast<T*>(m_pDefinition);
    }

    // Cast to leaf entity type
    template <class T> T*       GetAs()                 { if (m_unType == T::GetEntityType()) return static_cast<T*>(this); else return NULL; }
    template <class T> const T* GetAs() const           { if (m_unType == T::GetEntityType()) return static_cast<const T*>(this); else return NULL; }
    template <class T> bool     IsType()                { return (m_unType == T::GetEntityType()); }

    uint                GetIndex() const                { return m_uiIndex; }
    void                SetIndex(uint uiIndex)          { m_uiIndex = uiIndex; }

    // Network
    GAME_SHARED_API virtual void    Baseline();
    GAME_SHARED_API virtual void    GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
    GAME_SHARED_API virtual bool    ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);

    virtual int             GetPrivateClient()                              { return -1; }
    virtual IUnitEntity*    GetOwner() const                                { return NULL; }
    virtual IUnitEntity*    GetMasterOwner() const;
    virtual IGameEntity*    GetProxy(uint uiIndex) const                    { return NULL; }
    ushort                  GetModifierBits() const                         { return m_unModifierBits; }
    void                    SetModifierBits(ushort unModifierBits)          { m_unModifierBits = unModifierBits; UpdateDefinition(); }
    uint                    GetActiveModifierKey() const                    { return m_uiActiveModifierKey; }
    void                    SetActiveModifierKey(uint uiModifierKey)        { m_uiActiveModifierKey = uiModifierKey; }
    GAME_SHARED_API void    SetActiveModifierKey(const tstring &sModifierKey);
    const uivector&         GetModifierKeys() const                         { return m_vModifierKeys; }
    const uivector&         GetPersistentModifierKeys() const               { return m_vPersistentModifierKeys; }
    void                    SetPersistentModifierKeys(const uivector &vModifierKeys)    { m_vPersistentModifierKeys = vModifierKeys; }
    GAME_SHARED_API bool    HasModifier(const tstring &sModifier) const;
    virtual uint            GetLevel() const                                { return 0; }

    static void         ClientPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme)             {}
    static void         ServerPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme)             {}
    static void         PostProcess(CEntityConfig *pConfig)                 {}
    
    virtual void        ApplyWorldEntity(const CWorldEntity &ent)           {}

    // Actions
    virtual void        Spawn()                                             {}
    virtual void        GameStart()                                         {}
    virtual void        MatchStart()                                        {}
    virtual void        FlushStats()                                        {}
    virtual void        MatchRemake()                                       {}
    virtual bool        Use(IGameEntity *pActivator)                        { return false; }
    virtual void        Touch(IGameEntity *pActivator)                      {}
    virtual void        Trigger(IGameEntity *pActivator)                    {}

    virtual bool        ServerFrameSetup()      { return true; }    // Update visibility, apply auras
    virtual bool        ServerFrameThink()      { return true; }    // Update AI behaviors
    virtual bool        ServerFrameMovement()   { return true; }    // Movement
    virtual bool        ServerFrameAction()     { return true; }    // Process actions
    virtual bool        ServerFrameCleanup()    { return true; }    // Kill units, expire states

    virtual void        Copy(const IGameEntity &B);

    virtual void        SnapshotUpdate()        {}

    // Visual
    GAME_SHARED_API virtual bool    AddToScene(const CVec4f &v4Color, int iFlags)   { return true; }

    GAME_SHARED_API uint            GetModifierBit(uint uiModifierID);
    GAME_SHARED_API uint            GetModifierBits(const uivector &vModifiers);

    GAME_SHARED_API bool            MorphDynamicType(ushort unType);

    GAME_SHARED_API void            GetActiveExclusiveModifiers(IUnitEntity *pUnit, map<uint, SModifierEntry> &mapActiveModifiers, int iPriorityAdjust);
};
//=============================================================================

#endif //__I_GAMEENTITY_H__
