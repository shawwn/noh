// (C)2006 S2 Games
// c_entityregistry.h
//
//=============================================================================
#ifndef __C_ENTITYREGISTRY_H__
#define __C_ENTITYREGISTRY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gameentity.h"
#include "i_baseentityallocator.h"

#include "../k2/c_entitysnapshot.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class IEntityAllocator;
class CDynamicEntityAllocator;
class CScriptThread;

extern GAME_SHARED_API class CEntityRegistry &g_EntityRegistry;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#ifdef GAME_SHARED_EXPORTS
#define EntityRegistry (*CEntityRegistry::GetInstance())
#else
#define EntityRegistry g_EntityRegistry
#endif

#define DECLARE_ENT_ALLOCATOR(type, name) \
	friend class IGameEntity; \
	GAME_SHARED_API static CEntityAllocator<C##type##name>	s_Allocator; \
	static ushort GetEntityType()			{ return s_Allocator.GetID(); }

#define DEFINE_ENT_ALLOCATOR(type, name) \
	CEntityAllocator<C##type##name>	C##type##name::s_Allocator(_T(#type) _T("_") _T(#name), type##_##name);

#define DECLARE_ENT_ALLOCATOR2(type, name) \
	friend class IGameEntity; \
	friend class CEntityAllocator2<C##type##name>; \
	GAME_SHARED_API static CEntityAllocator2<C##type##name> s_Allocator; \
	static CEntityConfig s_EntityConfig; \
	static CEntityConfig *GetEntityConfig()	{ return &s_EntityConfig; } \
	public: \
	static ushort GetEntityType()			{ return s_Allocator.GetID(); } \
	protected:

#define DEFINE_ENT_ALLOCATOR2(type, name) \
	C##type##name::CEntityConfig		C##type##name::s_EntityConfig(_T(#type) _T("_") _T(#name)); \
	CEntityAllocator2<C##type##name>	C##type##name::s_Allocator(_T(#type) _T("_") _T(#name), type##_##name);

#define DECLARE_ENT_ALLOCATOR3(name)		static	CEntityAllocator<C##name>	s_Allocator;
#define DEFINE_ENT_ALLOCATOR3(name, id)		CEntityAllocator<C##name>	C##name::s_Allocator(_T(#name), id);

#define DECLARE_ENT_ALLOCATOR4(name) \
	friend class IGameEntity; \
	friend class CEntityAllocator2<C##name>; \
	GAME_SHARED_API static CEntityAllocator2<C##name> s_Allocator; \
	static CEntityConfig s_EntityConfig; \
	static CEntityConfig *GetEntityConfig()	{ return &s_EntityConfig; } \
	static ushort GetEntityType()			{ return s_Allocator.GetID(); }

#define DEFINE_ENT_ALLOCATOR4(name) \
	C##name::CEntityConfig			C##name::s_EntityConfig(_T(#name)); \
	CEntityAllocator2<C##name>		C##name::s_Allocator(_T(#name), name);

#define DECLARE_ENT_ALLOCATOR5(c) \
	friend class IGameEntity; \
	friend class CEntityAllocator2<c>; \
	GAME_SHARED_API static CEntityAllocator2<c> s_Allocator; \
	static CEntityConfig s_EntityConfig; \
	static CEntityConfig *GetEntityConfig()	{ return &s_EntityConfig; } \
	static ushort GetEntityType()			{ return s_Allocator.GetID(); }

#define DEFINE_ENT_ALLOCATOR5(c, name) \
	c::CEntityConfig			c::s_EntityConfig(_T(#name)); \
	CEntityAllocator2<c>		c::s_Allocator(_T(#name), name);

#define DEFINE_ENTITY(base, name) \
	class CGameEntity_##name : public base \
	{ \
	private: \
		DECLARE_ENT_ALLOCATOR5(CGameEntity_##name); \
	public: \
		~CGameEntity_##name()	{} \
		CGameEntity_##name() : base(GetEntityConfig()) {} \
	}; \
	DEFINE_ENT_ALLOCATOR5(CGameEntity_##name, name);

typedef map<ushort, IEntityAllocator*>		EntAllocatorIDMap;
typedef pair<ushort, IEntityAllocator*>		EntAllocatorIDEntry;
typedef EntAllocatorIDMap::iterator			EntAllocatorIDMap_it;
typedef EntAllocatorIDMap::const_iterator	EntAllocatorIDMap_cit;

typedef map<tstring, IEntityAllocator*>		EntAllocatorNameMap;
typedef pair<tstring, IEntityAllocator*>	EntAllocatorNameEntry;
typedef EntAllocatorNameMap::iterator		EntAllocatorNameMap_it;

typedef map<tstring, CDynamicEntityAllocator>	DynamicEntityNameMap;
typedef pair<tstring, CDynamicEntityAllocator>	DynamicEntityNameEntry;
typedef map<ushort, CDynamicEntityAllocator>	DynamicEntityTypeIDMap;
typedef pair<ushort, CDynamicEntityAllocator>	DynamicEntityTypeIDEntry;

typedef DynamicEntityTypeIDMap::iterator		DynamicEntityTypeIDMap_it;
typedef DynamicEntityTypeIDMap::const_iterator	DynamicEntityTypeIDMap_cit;
//=============================================================================

//=============================================================================
// IEntityAllocator
//=============================================================================
class IEntityAllocator
{
protected:
	tstring				m_sName;
	ushort				m_unID;
	CEntitySnapshot*	m_pBaseline;

	IEntityAllocator();

public:
	virtual ~IEntityAllocator();
	IEntityAllocator(const tstring &sName, ushort unID);

	ushort					GetID() const			{ return m_unID; }
	const tstring&			GetName() const			{ return m_sName; }
	const CEntitySnapshot*	GetBaseline() const		{ return m_pBaseline; }

	virtual IGameEntity*				Allocate() const = 0;
	virtual const TypeVector*			GetTypeVector() const = 0;
	virtual const SEntityDesc*			GetTypeDesc() const = 0;
	virtual uint						GetVersion() const = 0;
	virtual void						ClientPrecache(EPrecacheScheme eScheme, const tstring &sModifier) const = 0;
	virtual void						ServerPrecache(EPrecacheScheme eScheme, const tstring &sModifier) const = 0;
	virtual void						PostProcess() const = 0;
	virtual IGameEntity::CEntityConfig*	GetEntityConfig() const = 0;

	virtual ICvar*						GetGameSetting(const tstring &sSetting) const
	{
		return ICvar::GetCvar(m_sName + _T("_") + sSetting);
	}
};
//=============================================================================

//=============================================================================
// CEntityAllocator<T>
//=============================================================================
template <class T>
class CEntityAllocator : public IEntityAllocator
{
private:
	CEntityAllocator();

public:
	~CEntityAllocator()	{}
	CEntityAllocator(const tstring &sName, ushort unID) : IEntityAllocator(sName, unID)
	{
		IGameEntity *pEntBaseline(Allocate());

		m_pBaseline = K2_NEW(ctx_Game,    CEntitySnapshot)();
		m_pBaseline->SetIndex(INVALID_INDEX);
		m_pBaseline->SetType(pEntBaseline->GetType());
		m_pBaseline->SetFieldTypes(GetTypeVector(), CEntitySnapshot::CalcSnapshotSize(GetTypeVector()));
		m_pBaseline->SetAllFields();
		m_pBaseline->SetUniqueID(-1);
		m_pBaseline->SetPublicSequence(-1);
		pEntBaseline->GetSnapshot(*m_pBaseline, 0);

		K2_DELETE(pEntBaseline);

		T::InitTypeDesc(m_pBaseline);
	}

	virtual IGameEntity*				Allocate() const;
	virtual const TypeVector*			GetTypeVector() const	{ return &T::GetTypeVector(); }
	virtual const SEntityDesc*			GetTypeDesc() const		{ return T::GetStaticTypeDesc(); }
	virtual uint						GetVersion() const		{ return T::GetVersion(); }
	virtual void						ClientPrecache(EPrecacheScheme eScheme, const tstring &sModifier) const	{ T::ClientPrecache(NULL, eScheme, sModifier); }
	virtual void						ServerPrecache(EPrecacheScheme eScheme, const tstring &sModifier) const	{ T::ServerPrecache(NULL, eScheme, sModifier); }
	virtual void						PostProcess() const		{ T::PostProcess(NULL); }
	virtual IGameEntity::CEntityConfig*	GetEntityConfig() const	{ return NULL; }
};
//=============================================================================

/*====================
  CEntityAllocator::Allocate
  ====================*/
template <class T>
IGameEntity*	CEntityAllocator<T>::Allocate() const
{
	try
	{
		T* pNewEnt(K2_NEW(ctx_Game,    T)());
		if (pNewEnt == NULL)
			EX_ERROR(_T("Allocation failed"));

		pNewEnt->SetTypeName(m_sName);
		pNewEnt->SetType(m_unID);
		pNewEnt->SetTypeDesc(T::GetStaticTypeDesc());
		pNewEnt->Baseline();
		return pNewEnt;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CEntityAllocator::Allocate() - "), NO_THROW);
		return NULL;
	}
}


//=============================================================================
// CEntityAllocator2<T>
//=============================================================================
template <class T>
class CEntityAllocator2 : public IEntityAllocator
{
private:
	CEntityAllocator2();

public:
	~CEntityAllocator2()	{}
	CEntityAllocator2(const tstring &sName, ushort unID) : IEntityAllocator(sName, unID)
	{
		IGameEntity *pEntBaseline(Allocate());

		m_pBaseline = K2_NEW(ctx_Game,    CEntitySnapshot)();
		m_pBaseline->SetIndex(INVALID_INDEX);
		m_pBaseline->SetType(pEntBaseline->GetType());
		m_pBaseline->SetFieldTypes(GetTypeVector(), CEntitySnapshot::CalcSnapshotSize(GetTypeVector()));
		m_pBaseline->SetAllFields();
		m_pBaseline->SetUniqueID(-1);
		m_pBaseline->SetPublicSequence(-1);
		pEntBaseline->GetSnapshot(*m_pBaseline, 0);

		K2_DELETE(pEntBaseline);

		T::InitTypeDesc(m_pBaseline);
	}

	virtual IGameEntity*				Allocate() const;
	virtual const TypeVector*			GetTypeVector() const	{ return &T::GetTypeVector(); }
	virtual const SEntityDesc*			GetTypeDesc() const		{ return T::GetStaticTypeDesc(); }
	virtual uint						GetVersion() const		{ return T::GetVersion(); }
	virtual void						ClientPrecache(EPrecacheScheme eScheme, const tstring &sModifier) const	{ T::ClientPrecache(T::GetEntityConfig(), eScheme, sModifier); }
	virtual void						ServerPrecache(EPrecacheScheme eScheme, const tstring &sModifier) const	{ T::ServerPrecache(T::GetEntityConfig(), eScheme, sModifier); }
	virtual void						PostProcess() const		{ T::PostProcess(T::GetEntityConfig()); }
	virtual IGameEntity::CEntityConfig*	GetEntityConfig() const	{ return T::GetEntityConfig(); }
};
//=============================================================================

/*====================
  CEntityAllocator2::Allocate
  ====================*/
template <class T>
IGameEntity*	CEntityAllocator2<T>::Allocate() const
{
	try
	{
		T* pNewEnt(K2_NEW(ctx_Game,    T)());
		if (pNewEnt == NULL)
			EX_ERROR(_T("Allocation failed"));

		pNewEnt->SetTypeName(m_sName);
		pNewEnt->SetType(m_unID);
		pNewEnt->SetTypeDesc(T::GetStaticTypeDesc());
		pNewEnt->Baseline();
		return pNewEnt;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CEntityAllocator::Allocate() - "), NO_THROW);
		return NULL;
	}
}


//=============================================================================
// CBaseEntityAllocator
//=============================================================================
template <class T>
class CBaseEntityAllocator : public IBaseEntityAllocator
{
private:

public:
	~CBaseEntityAllocator()	{}
	CBaseEntityAllocator() :
	IBaseEntityAllocator(T::GetBaseTypeName(), T::GetBaseType())
	{
		IGameEntity *pEntBaseline(Allocate());
		pEntBaseline->Baseline();

		m_pBaseline = K2_NEW(ctx_Game,    CEntitySnapshot)();
		m_pBaseline->SetIndex(INVALID_INDEX);
		m_pBaseline->SetType(Entity_Dynamic);
		m_pBaseline->SetFieldTypes(GetTypeVector(), CEntitySnapshot::CalcSnapshotSize(GetTypeVector()));
		m_pBaseline->SetAllFields();
		m_pBaseline->SetUniqueID(-1);
		m_pBaseline->SetPublicSequence(-1);
		pEntBaseline->GetSnapshot(*m_pBaseline, 0);

		K2_DELETE(pEntBaseline);

		T::InitTypeDesc(m_pBaseline);
	}

	virtual IGameEntity*			Allocate() const		{ return K2_NEW(ctx_Game, T); }
	virtual const TypeVector*		GetTypeVector() const	{ return &T::GetTypeVector(); }
	virtual const SEntityDesc*		GetTypeDesc() const		{ return T::GetStaticTypeDesc(); }
	virtual uint					GetVersion() const		{ return T::GetVersion(); }
	virtual void					ClientPrecache(EPrecacheScheme eScheme, const tstring &sModifier) const	{ T::ClientPrecache(NULL, eScheme, sModifier); }
	virtual void					ServerPrecache(EPrecacheScheme eScheme, const tstring &sModifier) const	{ T::ServerPrecache(NULL, eScheme, sModifier); }
	virtual void					PostProcess() const		{ T::PostProcess(NULL); }
};
//=============================================================================


//=============================================================================
// CDynamicEntityAllocator
//=============================================================================
class CDynamicEntityAllocator
{
private:
	tstring					m_sName;
	ushort					m_unTypeID;
	ResHandle				m_hDefinition;
	IBaseEntityAllocator*	m_pBaseAllocator;

public:
	~CDynamicEntityAllocator()	{}
	CDynamicEntityAllocator(const tstring &sName, ushort unTypeID, ResHandle hDefinition, IBaseEntityAllocator *pBaseAllocator);
	
	const tstring&				GetName() const				{ return m_sName; }
	ushort						GetTypeID() const			{ return m_unTypeID; }
	ResHandle					GetDefinitionHandle() const	{ return m_hDefinition; }

	const IBaseEntityAllocator*	GetBaseAllocator() const	{ return m_pBaseAllocator; }
	const TypeVector*			GetTypeVector() const		{ return m_pBaseAllocator != NULL ? m_pBaseAllocator->GetTypeVector() : NULL; }
	const SEntityDesc*			GetTypeDesc() const			{ return m_pBaseAllocator != NULL ? m_pBaseAllocator->GetTypeDesc() : NULL; }
	uint						GetVersion() const			{ return m_pBaseAllocator != NULL ? m_pBaseAllocator->GetVersion() : 0; }
	const tstring&				GetBaseTypeName() const		{ return m_pBaseAllocator != NULL ? m_pBaseAllocator->GetBaseTypeName() : TSNULL; }
	uint						GetBaseType() const			{ return m_pBaseAllocator != NULL ? m_pBaseAllocator->GetBaseType() : ENTITY_BASE_TYPE_INVALID; }
	const CEntitySnapshot*		GetBaseline() const			{ return m_pBaseAllocator != NULL ? m_pBaseAllocator->GetBaseline() : NULL; }
	void						ClientPrecache(EPrecacheScheme eScheme, const tstring &sModifier) const		{ if (m_pBaseAllocator != NULL) m_pBaseAllocator->ClientPrecache(eScheme, sModifier); }
	void						ServerPrecache(EPrecacheScheme eScheme, const tstring &sModifier) const		{ if (m_pBaseAllocator != NULL) m_pBaseAllocator->ServerPrecache(eScheme, sModifier); }
	void						PostProcess() const			{ if (m_pBaseAllocator != NULL) m_pBaseAllocator->PostProcess(); }

	IGameEntity*	Allocate() const
	{
		if (m_pBaseAllocator == NULL)
		{
			Console << _T("Allocate failed for entity type: ") << m_sName << newl;
			return NULL;
		}

		IGameEntity *pNewEnt(m_pBaseAllocator->Allocate());
		if (pNewEnt == NULL)
		{
			Console << _T("Allocate failed for entity type: ") << m_sName << newl;
			return NULL;
		}

		pNewEnt->SetTypeName(m_sName);
		pNewEnt->SetType(m_unTypeID);
		pNewEnt->SetTypeDesc(m_pBaseAllocator->GetTypeDesc());
		pNewEnt->SetDefinitionHandle(m_hDefinition);
		pNewEnt->UpdateDefinition();
		pNewEnt->Baseline();
		return pNewEnt;
	}
};
//=============================================================================


//=============================================================================
// CEntityRegistry
//=============================================================================
class CEntityRegistry
{
	SINGLETON_DEF(CEntityRegistry);

private:
	EntAllocatorIDMap			m_mapAllocatorIDs;
	EntAllocatorNameMap			m_mapAllocatorNames;

	DynamicEntityNameMap		m_mapDynamicNames;
	DynamicEntityTypeIDMap		m_mapDynamicTypeIDs;

	uint						m_uiModifierCount;
	map<tstring, uint>			m_mapModifiers;
	
	uint						m_uiCooldownTypeCount;
	map<tstring, uint>			m_mapCooldownTypes;

	map<tstring, CScriptThread *>	m_mapDefinitions;

public:
	~CEntityRegistry();

	GAME_SHARED_API uint			RegisterModifier(const tstring &sModifier);
	GAME_SHARED_API uint			RegisterCooldownType(const tstring &sCooldownType);
	void							Register(IEntityAllocator* pAllocator);
	ushort							RegisterDynamicEntity(const tstring &sName, ResHandle hDefinition, IBaseEntityAllocator *pAllocator);
	GAME_SHARED_API void			RegisterDynamicEntity(ushort unTypeID, ResHandle hDefinition);

	GAME_SHARED_API ushort			LookupID(const tstring &sName);
	GAME_SHARED_API const tstring&	LookupName(ushort unID);
	GAME_SHARED_API const tstring&	LookupModifierKey(uint uiID) const;
	GAME_SHARED_API uint			LookupModifierKey(const tstring &sModifier) const;

	GAME_SHARED_API uint			GetBaseType(ushort unTypeID);
	uint							GetBaseType(const tstring &sName)	{ return GetBaseType(LookupID(sName)); }

	const IEntityAllocator*							GetAllocator(ushort unID);
	GAME_SHARED_API const CDynamicEntityAllocator*	GetDynamicAllocator(ushort unID);
	uint											GetNumTypes()	{ return uint(m_mapAllocatorIDs.size()); }

	GAME_SHARED_API IGameEntity*	Allocate(ushort unID);
	GAME_SHARED_API IGameEntity*	Allocate(const tstring &sName);

	GAME_SHARED_API IGameEntity*	AllocateDynamicEntity(const tstring &sName, uint uiBaseType);
	GAME_SHARED_API IGameEntity*	AllocateDynamicEntity(ushort unTypeID, uint uiBaseType = ENTITY_BASE_TYPE_ENTITY);

	GAME_SHARED_API const TypeVector*	GetTypeVector(ushort unType) const;
	GAME_SHARED_API const SEntityDesc*	GetTypeDesc(ushort unType) const;
	GAME_SHARED_API void				ServerPrecache(ushort unType, EPrecacheScheme eScheme, const tstring &sModifier) const;
	GAME_SHARED_API void				ClientPrecache(ushort unType, EPrecacheScheme eScheme, const tstring &sModifier) const;
	GAME_SHARED_API void				PostProcess(ushort unType) const;

	GAME_SHARED_API ICvar*	GetGameSetting(ushort unType, const tstring &sSetting) const;

	GAME_SHARED_API void	GetEntityList(map<ushort, tstring> &mapEntities) const;

	GAME_SHARED_API void	WriteDynamicEntities(IBuffer &buffer);

	GAME_SHARED_API void	GetHeroList(const tstring &sTeamName, vector<ushort> &vHeroes, EAttribute eAttribute = ATTRIBUTE_INVALID);
	GAME_SHARED_API void	GetAutoRecipeList(vector<ushort> &vRecipes);
	GAME_SHARED_API void	GetShopList(vector<ushort> &vShops);
	GAME_SHARED_API void	GetItemList(vector<ushort> &vItems);

	EntAllocatorIDMap&		GetAllocators()			{ return m_mapAllocatorIDs; }
	DynamicEntityTypeIDMap&	GetDynamicAllocators()	{ return m_mapDynamicTypeIDs; }

	GAME_SHARED_API CEntityDefinitionResource*	GetEntityDef(ResHandle hEntDef);

	template <class T>
	T*	GetDefinition(ushort unTypeID)
	{
		if (unTypeID == INVALID_ENT_TYPE)
			return NULL;

		DynamicEntityTypeIDMap::iterator itFind(m_mapDynamicTypeIDs.find(unTypeID));
		if (itFind == m_mapDynamicTypeIDs.end())
			return NULL;

		// Type 1 must fully contain Type 2
		uint uiType1(itFind->second.GetBaseType());
		uint uiType2(T::GetBaseType());

		if ((GET_ENTITY_BASE_TYPE0(uiType2) != 0 && GET_ENTITY_BASE_TYPE0(uiType1) != GET_ENTITY_BASE_TYPE0(uiType2)) || 
			(GET_ENTITY_BASE_TYPE1(uiType2) != 0 && GET_ENTITY_BASE_TYPE1(uiType1) != GET_ENTITY_BASE_TYPE1(uiType2)) ||
			(GET_ENTITY_BASE_TYPE2(uiType2) != 0 && GET_ENTITY_BASE_TYPE2(uiType1) != GET_ENTITY_BASE_TYPE2(uiType2)) ||
			(GET_ENTITY_BASE_TYPE3(uiType2) != 0 && GET_ENTITY_BASE_TYPE3(uiType1) != GET_ENTITY_BASE_TYPE3(uiType2)) ||
			(GET_ENTITY_BASE_TYPE4(uiType2) != 0 && GET_ENTITY_BASE_TYPE4(uiType1) != GET_ENTITY_BASE_TYPE4(uiType2)))
			return NULL;

		CEntityDefinitionResource *pResource(GetEntityDef(itFind->second.GetDefinitionHandle()));
		if (pResource == NULL)
			return NULL;

		return pResource->GetDefinition<T>();
	}

	template <class T> T*	GetDefinition(const tstring &sName)	{ return GetDefinition<T>(LookupID(sName)); }

	GAME_SHARED_API CScriptThread*	NewScriptThread(const tstring &sName);
	GAME_SHARED_API CScriptThread*	GetScriptDefinition(const tstring &sName);
	GAME_SHARED_API void			PrecacheScripts();
};
//=============================================================================

#endif //__C_ENTITYREGISTRY_H__
