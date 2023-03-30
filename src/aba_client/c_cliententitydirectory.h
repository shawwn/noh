// (C)2006 S2 Games
// c_cliententitydirectory.h
//
//=============================================================================
#ifndef __C_CLIENTENTITYDIRECTORY_H__
#define __C_CLIENTENTITYDIRECTORY_H__

//=============================================================================
// Headers
//=============================================================================
#include "../aba_shared/i_entitydirectory.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CClientEntity;
class IGameEntity;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef vector<IBitEntity *>		BitEntVector;
typedef BitEntVector::iterator		BitEntVector_it;
//=============================================================================

//=============================================================================
// CClientEntityDirectory
//=============================================================================
class CClientEntityDirectory : public IEntityDirectory
{
public:
	typedef map<uint, IGameEntity*>		EntMap;
	typedef EntMap::iterator			EntMap_it;

	typedef list<IGameEntity*>			EntList;
	typedef EntList::iterator			EntList_it;

	typedef map<uint, PoolHandle>		ClientEntMap;
	typedef ClientEntMap::iterator		ClientEntMap_it;

	typedef CRecyclePool<CClientEntity> ClientEntityPool;

private:
	EntMap			m_mapEntities;
	EntList			m_lMonoEntities;
	ClientEntMap	m_mapClientEntities;

	PoolHandle		m_hFirstClientEntity;
	PoolHandle		m_hFirstLocalClientEntity;

	uivector		m_vBitBuffer;
	BitEntVector	m_vBitEntities;

	static ClientEntityPool				s_pClientEntityPool;
	
	static UnitList	s_lUnits;

public:
	~CClientEntityDirectory()	{ Clear(); }
	CClientEntityDirectory() :
	m_hFirstClientEntity(INVALID_POOL_HANDLE),
	m_hFirstLocalClientEntity(INVALID_POOL_HANDLE)
	{}

	void			Clear();
	void			ClearBitEntities();

	IGameEntity*	Allocate(uint uiIndex, ushort unType);
	IGameEntity*	Allocate(uint uiIndex, const tstring &sName);
	IGameEntity*	AllocateLocal(ushort unType);
	IGameEntity*	AllocateLocal(const tstring &sName);

	static bool		IsLocalEntity(uint uiIndex)		{ return (uiIndex >= 0x10000); }
	
	void			Delete(uint uiIndex);
	void			Delete(CClientEntity *pEntity)	{ if (pEntity != NULL) Delete(pEntity->GetIndex()); }
	void			Delete(IGameEntity *pEntity)	{ if (pEntity != NULL) Delete(pEntity->GetIndex()); }

	CClientEntity*			GetClientEntity(uint uiIndex);
	IVisualEntity*			GetClientEntityCurrent(uint uiIndex);
	IVisualEntity*			GetClientEntityPrev(uint uiIndex);
	IVisualEntity*			GetClientEntityNext(uint uiIndex);
	IGameEntity*			GetEntityNext(uint uiIndex);
	
	virtual IGameEntity*	GetEntity(uint uiIndex);
	virtual IGameEntity*	GetFirstEntity();
	virtual IGameEntity*	GetNextEntity(IGameEntity *pEntity);

	void			GetEntities(uivector &vResult, ushort unType);
	const UnitList&	GetUnitList()									{ return s_lUnits; }

	void			PrepForSnapshot();
	void			CleanupEntities();
	void			Frame(float fLerp);
	void			PopulateScene();
	void			DrawScreen();

	ClientEntMap&			GetEntMap() { return m_mapClientEntities; }

	static inline CClientEntity*	GetByHandle(PoolHandle hHandle);
	static inline void				DeleteByHandle(PoolHandle hHandle);
	static inline PoolHandle		Allocate(const CClientEntity &cInitialState);
	static inline PoolHandle		Allocate(PoolHandle hHandle);
	static inline PoolHandle		GetHandle(CClientEntity *pClientEntity);

	CClientEntity*			GetFirstClientEntity()			{ return GetByHandle(m_hFirstClientEntity); }
	CClientEntity*			GetFirstLocalClientEntity()		{ return GetByHandle(m_hFirstLocalClientEntity); }

	uivector&				GetBitEntityBuffer()			{ return m_vBitBuffer; }
	BitEntVector&			GetBitEntities()				{ return m_vBitEntities; }

	void					AddBitEntity(IBitEntity *pBit);

	virtual void			UpdateDefinitions(ushort unType);

	void			Rewind();
};
//=============================================================================

//=============================================================================
// Inline Functions
//=============================================================================

/*====================
  CClientEntityDirectory::GetByHandle
  ====================*/
inline
CClientEntity*	CClientEntityDirectory::GetByHandle(PoolHandle hHandle)
{
	return s_pClientEntityPool.GetReferenceByHandle(hHandle);
}


/*====================
  CClientEntityDirectory::DeleteByHandle
  ====================*/
inline
void	CClientEntityDirectory::DeleteByHandle(PoolHandle hHandle)
{
	s_pClientEntityPool.Free(hHandle);
}


/*====================
  CClientEntityDirectory::Allocate
  ====================*/
inline
PoolHandle	CClientEntityDirectory::Allocate(const CClientEntity &cInitialState)
{
	return s_pClientEntityPool.New(cInitialState);
}


/*====================
  CClientEntityDirectory::Allocate
  ====================*/
inline
PoolHandle	CClientEntityDirectory::Allocate(PoolHandle hHandle)
{
	return s_pClientEntityPool.NewFromHandle(hHandle);
}


/*====================
  CClientEntityDirectory::GetHandle
  ====================*/
inline
PoolHandle	CClientEntityDirectory::GetHandle(CClientEntity *pClientEntity)
{
	return s_pClientEntityPool.GetHandleByReference(pClientEntity);
}
//=============================================================================

#endif //__C_CLIENTENTITYDIRECTORY_H__
