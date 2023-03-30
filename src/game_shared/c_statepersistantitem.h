// (C)2006 S2 Games
// c_statepersistantitem.h
//
//=============================================================================
#ifndef __C_STATEPERSISTANTITEM_H__
#define __C_STATEPERSISTANTITEM_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStatePersistantItem
//=============================================================================
class CStatePersistantItem : public IEntityState
{
private:
	static vector<SDataField>	*s_pvFields;

	START_ENTITY_CONFIG(IEntityState)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, PersistantItem);

	ushort		m_unItemData;
	uint		m_uiRegenMod;
	uint		m_uiIncreaseMod;
	uint		m_uiReplenishMod;
	uint		m_uiPersistantType;

	void		UpdateItemData();

public:
	~CStatePersistantItem()	{}
	CStatePersistantItem();

	GAME_SHARED_API virtual void	Baseline();
	GAME_SHARED_API virtual void	GetSnapshot(CEntitySnapshot &snapshot) const;
	GAME_SHARED_API virtual bool	ReadSnapshot(CEntitySnapshot &snapshot);

	static const vector<SDataField>&	GetTypeVector();

	float			GetMultiplier()					{ return g_fPersistantItemTypeMultipliers[m_uiPersistantType]; }

	GAME_SHARED_API void	SetItemData(ushort unData);
};
//=============================================================================

#endif //__C_STATEPERSISTANTITEM_H__
