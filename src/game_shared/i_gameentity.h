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

#include "../k2/c_entitysnapshot.h"
#include "../k2/s_traceinfo.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class IGameEntity;
class IPropEntity;
class IPlayerEntity;
class IProjectile;
class ILight;
class IGadgetEntity;
class IBuildingEntity;
class INpcEntity;
class IPetEntity;
class IPropEntity;
class ICombatEntity;
class IVisualEntity;
class IInventoryItem;
class ITriggerEntity;
class IEntityState;
class CWorldEntity;
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
	CCvar<type, type>	m_cvar##name; \
public: \
	const CCvar<type, type>&	Get##name() const	{ return m_cvar##name; }

#define INIT_ENTITY_CVAR(name, def)	m_cvar##name(sName + _T("_") _T(#name), def, CVAR_GAMECONFIG | CVAR_TRANSMIT)
#define DEFINE_ENTITY_CVAR(data, type, ent, name, def)	CCvar<data, data>	C##type##ent::s_cvar##name(_T(#type) _T("_") _T(#ent) _T("_") _T(#name), def, CVAR_GAMECONFIG | CVAR_TRANSMIT);

#define ENTITY_CVAR_ACCESSOR(type, name, def) \
	type	Get##name() const	{ if (m_pEntityConfig == NULL) return def; return m_pEntityConfig->Get##name(); }

typedef map<uint, IGameEntity*>		EntMap;
typedef EntMap::iterator			EntMap_it;

enum EEntityHitByType
{
	ENTITY_HIT_BY_MELEE,
	ENTITY_HIT_BY_RANGED,
	// AOE type needed?

	NUM_ENTITY_HIT_BY_TYPES
};
//=============================================================================

//=============================================================================
// IGameEntity
//=============================================================================
class IGameEntity
{
private:
	static vector<SDataField>	*s_pvFields;

	IGameEntity();

protected:
	// Cvar settings
	START_ENTITY_BASE_CONFIG
	END_ENTITY_BASE_CONFIG

	CEntityConfig*	m_pEntityConfig;

	// Identity
	ushort				m_unType;
	uint				m_uiIndex;
	tstring				m_sTypeName;
	uint				m_uiUniqueID;

	uint				m_uiNetFlags;	// Server controlled, communicated to the client

	bool				m_bDelete;
	uint				m_uiFrame;
	bool				m_bValid;

public:
	virtual ~IGameEntity()	{}
	IGameEntity(CEntityConfig *pConfig);

	// Accessors
	ushort				GetType() const					{ return m_unType; }
	void				SetType(ushort unType)			{ m_unType = unType; }
	bool				IsType(ushort unType) const		{ return m_unType == unType; }

	const tstring&		GetTypeName() const						{ return m_sTypeName; }
	void				SetTypeName(const tstring &sTypeName)	{ m_sTypeName = sTypeName; }

	uint				GetUniqueID() const				{ return m_uiUniqueID; }
	void				SetUniqueID(uint uiUniqueID)	{ m_uiUniqueID = uiUniqueID; }

	void				SetDelete(bool bDelete)			{ m_bDelete = bDelete; }
	bool				GetDelete() const				{ return m_bDelete; }

	uint				GetFrame() const				{ return m_uiFrame; }
	void				SetFrame(uint uiFrame)			{ m_uiFrame = uiFrame; }

	bool				IsValid() const					{ return m_bValid; }
	void				Validate()						{ m_bValid = true; }
	void				Invalidate()					{ m_bValid = false; }
	
	virtual bool		IsProp() const					{ return false; }
	virtual bool		IsPlayer() const				{ return false; }
	virtual bool		IsProjectile() const			{ return false; }
	virtual bool		IsLight() const					{ return false; }
	virtual bool		IsGadget() const				{ return false; }
	virtual bool		IsBuilding() const				{ return false; }
	virtual bool		IsNpc() const					{ return false; }
	virtual bool		IsPet() const					{ return false; }
	virtual bool		IsCombat() const				{ return false; }
	virtual bool		IsVisual() const				{ return false; }
	virtual bool		IsState() const					{ return false; }
	virtual bool		IsInventoryItem() const			{ return false; }
	virtual bool		IsTrigger() const				{ return false; }
		
	virtual bool		IsStatic() const				{ return false; }

	GAME_SHARED_API IPropEntity*		GetAsProp();
	GAME_SHARED_API IPlayerEntity*		GetAsPlayerEnt();
	GAME_SHARED_API IProjectile*		GetAsProjectile();
	GAME_SHARED_API ILight*				GetAsLight();
	GAME_SHARED_API IGadgetEntity*		GetAsGadget();
	GAME_SHARED_API IBuildingEntity*	GetAsBuilding();
	GAME_SHARED_API INpcEntity*			GetAsNpc();
	GAME_SHARED_API IPetEntity*			GetAsPet();
	GAME_SHARED_API ICombatEntity*		GetAsCombatEnt();
	GAME_SHARED_API IVisualEntity*		GetAsVisualEnt();
	GAME_SHARED_API IEntityState*		GetAsState();
	GAME_SHARED_API IInventoryItem*		GetAsInventoryItem();
	GAME_SHARED_API ITriggerEntity*		GetAsTrigger();

	GAME_SHARED_API const IPropEntity*		GetAsProp() const;
	GAME_SHARED_API const IPlayerEntity*	GetAsPlayerEnt() const;
	GAME_SHARED_API const IProjectile*		GetAsProjectile() const;
	GAME_SHARED_API const ILight*			GetAsLight() const;
	GAME_SHARED_API const IGadgetEntity*	GetAsGadget() const;
	GAME_SHARED_API const IBuildingEntity*	GetAsBuilding() const;
	GAME_SHARED_API const INpcEntity*		GetAsNpc() const;
	GAME_SHARED_API const IPetEntity*		GetAsPet() const;
	GAME_SHARED_API const ICombatEntity*	GetAsCombatEnt() const;
	GAME_SHARED_API const IVisualEntity*	GetAsVisualEnt() const;
	GAME_SHARED_API const IEntityState*		GetAsState() const;
	GAME_SHARED_API const IInventoryItem*	GetAsInventoryItem() const;
	GAME_SHARED_API const ITriggerEntity*	GetAsTrigger() const;

	uint				GetIndex() const									{ return m_uiIndex; }
	void				SetIndex(uint uiIndex)								{ m_uiIndex = uiIndex; }

	// Net Flags
	void				SetNetFlags(uint uiFlags)							{ m_uiNetFlags |= uiFlags; }
	void				RemoveNetFlags(uint uiFlags)						{ m_uiNetFlags &= ~uiFlags; }
	void				ClearNetFlags()										{ m_uiNetFlags = 0; }
	bool				HasNetFlags(uint uiFlags) const						{ return (m_uiNetFlags & uiFlags) != 0; }
	bool				HasAllNetFlags(uint uiFlags) const					{ return (m_uiNetFlags & uiFlags) == uiFlags; }
	
	// Network
	GAME_SHARED_API virtual void	Baseline();
	GAME_SHARED_API virtual void	GetSnapshot(CEntitySnapshot &snapshot) const;
	GAME_SHARED_API virtual bool	ReadSnapshot(CEntitySnapshot &snapshot);

	static const vector<SDataField>&	GetTypeVector();

	virtual int			GetPrivateClient()									{ return -1; }

	static void			ClientPrecache(CEntityConfig *pConfig);
	static void			ServerPrecache(CEntityConfig *pConfig);
	
	virtual void		ApplyWorldEntity(const CWorldEntity &ent)			{}

	// Actions
	virtual void		Spawn()												{}
	virtual void		GameStart()											{}
	virtual void		WarmupStart()										{}
	virtual bool		Reset()												{ return false; } // false for delete
	virtual bool		ServerFrame()										{ return true; }
	virtual float		Damage(float fDamage, int iFlags, IVisualEntity *pAttacker = NULL, ushort unDamagingObjectID = INVALID_ENT_TYPE, bool bFeedback = true) { return 0.0f; }
	virtual void		Hit(CVec3f v3Pos, CVec3f v3Angle, EEntityHitByType eHitBy = ENTITY_HIT_BY_RANGED)					{}
	virtual void		KillReward(IGameEntity *pKiller)					{}
	virtual void		Kill(IVisualEntity *pAttacker = NULL, ushort unKillingObjectID = INVALID_ENT_TYPE)					{}
	virtual bool		Use(IGameEntity *pActivator)						{ return false; }
	virtual void		Touch(IGameEntity *pActivator)						{}
	virtual bool		CanTakeDamage(int iFlags, IVisualEntity *pAttacker = NULL) { return false; }

	virtual void		Copy(const IGameEntity &B);

	// Visual
	GAME_SHARED_API virtual bool		AddToScene(const CVec4f &v4Color, int iFlags);

	virtual const tstring&		GetEntityName() const						{ return SNULL; }
	virtual const tstring&		GetEntityDescription() const				{ return SNULL; }
};
//=============================================================================

#endif //__I_GAMEENTITY_H__
