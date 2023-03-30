// (C)2006 S2 Games
// c_serverentitydirectory.h
//
//=============================================================================
#ifndef __C_SERVERENTITYDIRECTORY_H__
#define __C_SERVERENTITYDIRECTORY_H__

//=============================================================================
// Headers
//=============================================================================
#include "../aba_shared/i_entitydirectory.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CSnapshot;
class IUnitEntity;
class CStateBlock;

typedef vector<IBitEntity *>		BitEntVector;
typedef BitEntVector::iterator		BitEntVector_it;

typedef set<IBitEntity *>			BitEntSet;
typedef BitEntSet::iterator			BitEntSet_it;
//=============================================================================

//=============================================================================
// CServerEntityDirectory
//=============================================================================
class CServerEntityDirectory : public IEntityDirectory
{
	typedef map<uint, IGameEntity *>	EntUIDMap;
	typedef EntUIDMap::iterator			EntUIDMap_it;

private:
	uint			m_uiNextUniqueID;
	uint			m_uiLastGameIndex;

	EntMap			m_mapEntities;
	EntUIDMap		m_mapUniqueIDs;
	EntMap			m_mapFrameEntities;
	UnitList		m_lUnits;

	BitEntSet		m_vTestBitEntVisible[2];
	uivector		m_vBitBuffer[3];
	BitEntVector	m_vBitEntities;

	EntMap_it		m_itLastEntity;

	vector<uint>	m_vAvailableIndexes;

	uint			GetNewEntIndex(uint uiMinIndex);
	void			CloseIndex(uint uiIndex)		{ m_vAvailableIndexes[uiIndex >> 5] &= ~(1 << (uiIndex & 31)); }
	void			OpenIndex(uint uiIndex)			{ m_vAvailableIndexes[uiIndex >> 5] |= (1 << (uiIndex & 31)); }

public:
	~CServerEntityDirectory();
	CServerEntityDirectory();

	void			Clear();
	IGameEntity*	Allocate(ushort unType, uint uiMinIndex = INVALID_INDEX);
	IGameEntity*	Allocate(const tstring &sName, uint uiMinIndex = INVALID_INDEX)		{ return Allocate(EntityRegistry.LookupID(sName), uiMinIndex); }
	void			Delete(uint uiIndex);

	IGameEntity*	AllocateDynamicEntity(const tstring &sName, uint uiMinIndex, uint uiBaseType);
	IGameEntity*	AllocateDynamicEntity(ushort unTypeID, uint uiMinIndex, uint uiBaseType);

	IGameEntity*	GetEntity(uint uiIndex);
	IGameEntity*	GetEntityFromUniqueID(uint uiUniqueID);
	uint			GetGameIndexFromUniqueID(uint uiUniqueID);
	IGameEntity*	GetFirstEntity();
	IGameEntity*	GetNextEntity(IGameEntity *pEntity);

	IVisualEntity*	GetEntityFromName(const tstring &sName);
	IVisualEntity*	GetNextEntityFromName(IVisualEntity *pEntity);

	virtual void	GetEntities(uivector &vResult, ushort unType);

	void			GetSnapshot(CSnapshot &snapshot);
	void			GameStart();
	void			MatchStart();
	void			FlushStats();
	void			Spawn();
	void			FrameSetup();
	void			FrameThink();
	void			FrameMovement();
	void			FrameAction();
	void			FrameCleanup();
	void			BackgroundFrame();

	uint			GetNumEntities() const		{ return uint(m_mapEntities.size()); }
	EntMap&			GetEntMap()					{ return m_mapEntities; }
	const UnitList&	GetUnitList()				{ return m_lUnits; }

	void			WriteBitEntityMap(CStateBlock &block);

	virtual void	ActivateBitEntity(uint uiIndex);
	virtual void	DeactivateBitEntity(uint uiIndex);

	virtual void	ActivateBitEntities();
	virtual void	DeactivateBitEntities();

	virtual void	ClearBitEntities();

	virtual void	UpdateDefinitions(ushort unType);
};
//=============================================================================

#endif //__C_SERVERENTITYDIRECTORY_H__
