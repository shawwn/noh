// (C)2006 S2 Games
// i_propfoundation.h
//
//=============================================================================
#ifndef __I_PROPFOUNDATION_H__
#define __I_PROPFOUNDATION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_propentity.h"
//=============================================================================

//=============================================================================
// IPropFoundation
//=============================================================================
class IPropFoundation : public IPropEntity
{
protected:
	static vector<SDataField>*	s_pvFields;

	START_ENTITY_CONFIG(IPropEntity)
		DECLARE_ENTITY_CVAR(float, Radius)
	END_ENTITY_CONFIG

	CEntityConfig*				m_pEntityConfig;

	uint	m_uiBuildingIndex;
	ushort	m_unBuildingType;

public:
	~IPropFoundation()	{}
	IPropFoundation(CEntityConfig *pConfig);

	IPropFoundation*			GetAsFoundation()				{ return this; }
	const IPropFoundation*		GetAsFoundation() const			{ return this; }

	virtual bool				IsMine() const					{ return false; }

	GAME_SHARED_API static const vector<SDataField>&	GetTypeVector();
	virtual void					Baseline();
	virtual void					GetSnapshot(CEntitySnapshot &snapshot) const;
	virtual bool					ReadSnapshot(CEntitySnapshot &snapshot);

	float							GetRadius() const				{ return m_pEntityConfig->GetRadius(); }

	void							SetBuildingType(ushort unType)	{ m_unBuildingType = unType; }
	void							ClearBuildingType()				{ m_unBuildingType = 0; }
	bool							HasBuilding() const				{ return m_uiBuildingIndex != INVALID_INDEX; }
	void							SetBuildingIndex(uint uiIndex)	{ m_uiBuildingIndex = uiIndex; }
	void							ClearBuildingIndex()			{ m_uiBuildingIndex = 0; }

	GAME_SHARED_API virtual void	Spawn();
	virtual bool					AddToScene(const CVec4f &v4Color, int iFlags);

	virtual uint					HarvestGold()					{ return 0; }
	virtual uint					RaidGold()						{ return 0; }
	virtual uint					GetHarvestRate() const			{ return 0; }
	virtual uint					GetRemainingGold() const		{ return 0; }
	virtual float					GetRemainingGoldPercent() const	{ return 0.0f; }
	virtual uint					GetTotalGold() const			{ return 0; }
	virtual uint					GetRaidBonus() const			{ return 0; }
	virtual bool					CanSupportBuilding() const		{ return true; }

	void							AssignToTeam(int iTeam);

	virtual CSkeleton*				AllocateSkeleton()		{ return NULL; }

	virtual void					Copy(const IGameEntity &B);

	GAME_SHARED_API virtual bool	AIShouldTarget()			{ return false; }
};
//=============================================================================

#endif //__I_PROPFOUNDATION_H__
