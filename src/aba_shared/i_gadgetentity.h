// (C)2008 S2 Games
// i_gadgetentity.h
//
//=============================================================================
#ifndef __I_GADGETENTITY_H__
#define __I_GADGETENTITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_unitentity.h"
#include "c_gadgetdefinition.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint MAX_GADGET_COUNTERS(3);
//=============================================================================

//=============================================================================
// IGadgetEntity
//=============================================================================
class IGadgetEntity : public IUnitEntity
{
	DECLARE_ENTITY_DESC

public:
	typedef CGadgetDefinition TDefinition;
	
protected:
	uint			m_uiMountIndex;
	uint			m_uiCharges;

	uint			m_uiAuraSourceUID;
	uint			m_uiAuraTime;

public:
	virtual ~IGadgetEntity()	{}
	IGadgetEntity();

	virtual int			GetPrivateClient()				{ return m_iOwnerClientNumber; }

	SUB_ENTITY_ACCESSOR(IGadgetEntity, Gadget)

	void				SetAuraSource(uint uiUID)		{ m_uiAuraSourceUID = uiUID; }
	void				SetAuraTime(uint uiTime)		{ m_uiAuraTime = uiTime; }
	virtual bool		IsAuraInvalid() const			{ return m_uiAuraSourceUID != INVALID_INDEX && m_uiAuraTime != Game.GetGameTime(); }

	virtual void		Baseline();
	virtual void		GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
	virtual bool		ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);

	virtual void		Spawn();

	virtual bool		ServerFrameThink();
	virtual	bool		ServerFrameCleanup();

	GAME_SHARED_API virtual void	Copy(const IGameEntity &B);

	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, Lifetime)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, InitialCharges)

	uint				GetMountIndex() const						{ return m_uiMountIndex; }
	void				SetMountIndex(uint uiIndex)					{ m_uiMountIndex = uiIndex; }

	virtual ushort		GetCharges() const				{ return m_uiCharges; }
	virtual void		SetCharges(uint uiCharges)		{ m_uiCharges = uiCharges; }
	virtual void		AddCharges(uint uiCharges)		{ m_uiCharges += uiCharges; }
	virtual void		RemoveCharge()					{ if (m_uiCharges > 0) --m_uiCharges; }

	virtual void		SetLevel(uint uiLevel);

	virtual void		UpdateModifiers();
};
//=============================================================================

#endif //__I_GADGETENTITY_H__
