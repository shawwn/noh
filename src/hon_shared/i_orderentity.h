// (C)2009 S2 Games
// i_orderentity.h
//
//=============================================================================
#ifndef __I_ORDERENTITY_H__
#define __I_ORDERENTITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gameentity.h"
#include "c_orderdefinition.h"
#include "c_entitydefinitionresource.h"
//=============================================================================

//=============================================================================
// IOrderEntity
//=============================================================================
class IOrderEntity : public IGameEntity
{
	DECLARE_ENTITY_DESC

public:
	typedef COrderDefinition TDefinition;

protected:
	uint				m_uiLevel;
	uint				m_uiOwnerIndex;
	uint				m_auiProxyUID[4];
	float				m_fParam;
	bool				m_bComplete;
	bool				m_bCancel;

public:
	virtual ~IOrderEntity()	{}
	IOrderEntity();

	SUB_ENTITY_ACCESSOR(IOrderEntity, Order)

	virtual bool	IsServerEntity() const				{ return true; }
	
	virtual void	Baseline();
	virtual void	GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
	virtual bool	ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);

	static void		ClientPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier);
	static void		ServerPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier);

	void			SetOwnerIndex(uint uiIndex)			{ m_uiOwnerIndex = uiIndex; }
	uint			GetOwnerIndex() const				{ return m_uiOwnerIndex; }
	IUnitEntity*	GetOwner() const					{ return Game.GetUnitEntity(m_uiOwnerIndex); }

	void			SetProxyUID(uint uiIndex, uint uiUID)	{ m_auiProxyUID[uiIndex] = uiUID; }
	uint			GetProxyUID(uint uiIndex) const			{ return m_auiProxyUID[uiIndex]; }
	IGameEntity*	GetProxy(uint uiIndex) const			{ return Game.GetEntityFromUniqueID(m_auiProxyUID[uiIndex]); }

	void			SetParam(float fParam)				{ m_fParam = fParam; }
	float			GetParam() const					{ return m_fParam; }

	void			SetCancel(bool bCancel)				{ m_bCancel = bCancel; }
	bool			GetCancel() const					{ return m_bCancel; }

	void			SetComplete(bool bComplete)			{ m_bComplete = bComplete; }
	bool			GetComplete() const					{ return m_bComplete; }

	virtual void	Spawn();

	virtual bool	ServerFrameSetup();
	virtual bool	ServerFrameMovement();
	virtual bool	ServerFrameAction();
	virtual bool	ServerFrameCleanup();

	void			ExecuteActionScript(EEntityActionScript eScript, IUnitEntity *pTarget, const CVec3f &v3Target);
	
	uint			GetLevel() const			{ return m_uiLevel; }
	void			SetLevel(uint uiLevel)		{ m_uiLevel = uiLevel; }

	bool			IsActive() const			{ return true; }

	void			UpdateModifiers(const uivector &vModifiers);
	
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, TriggerRange)
};
//=============================================================================

#endif //__I_ORDERENTITY_H__
