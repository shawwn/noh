// (C)2005 S2 Games
// c_worldentitylist.h
//
//=============================================================================
#ifndef __C_WORLDENTITYLIST_H__
#define __C_WORLDENTITYLIST_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_worldcomponent.h"
#include "c_recyclepool.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CWorldEntity;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef vector<PoolHandle>				WorldEntList;
typedef WorldEntList::iterator			WorldEntList_it;
typedef WorldEntList::const_iterator	WorldEntList_cit;
typedef vector<PoolHandle>				WorldEntVector;
typedef WorldEntVector::iterator		WorldEntVector_it;
typedef WorldEntVector::const_iterator	WorldEntVector_cit;

const uint DEFAULT_WORLDENTS(8192);
//=============================================================================

//=============================================================================
// CWorldEntityList
//=============================================================================
class CWorldEntityList : public IWorldComponent
{
private:
	CRecyclePool<CWorldEntity> m_poolWorldEntities;
	WorldEntList			m_vEntities;
	uint					m_uiMinFreeIndex;

public:
	K2_API ~CWorldEntityList();
	K2_API CWorldEntityList(EWorldComponent eComponent);

	bool	Load(CArchive &archive, const CWorld *pWorld);
	bool	Generate(const CWorld *pWorld);
	void	Release();
	bool	Serialize(IBuffer *pBuffer);
	void	Restore(CArchive &archive);

	K2_API uint				AllocateNewEntity(uint uiIndex = INVALID_INDEX);
	K2_API CWorldEntity*	GetEntity(uint uiIndex, bool bThrow = false);
	K2_API CWorldEntity*	GetEntityByHandle(PoolHandle hHandle);
	K2_API PoolHandle		GetHandleByEntity(CWorldEntity *pEntity);
	K2_API void				FreeEntity(PoolHandle hHandle);
	K2_API WorldEntList&	GetEntityList()								{ return m_vEntities; }
	K2_API void				DeleteEntity(uint uiIndex);
	K2_API bool				Exists(uint uiIndex);
};
//=============================================================================

//=============================================================================
// Inline functions
//=============================================================================

/*====================
  CWorldEntityList::GetEntity
  ====================*/
inline
CWorldEntity* CWorldEntityList::GetEntityByHandle(PoolHandle hHandle)
{
	return m_poolWorldEntities.GetReferenceByHandle(hHandle);
}


/*====================
  CWorldEntityList::GetHandleByEntity
  ====================*/
inline
PoolHandle	CWorldEntityList::GetHandleByEntity(CWorldEntity *pEntity)
{
	return m_poolWorldEntities.GetHandleByReference(pEntity);
}


/*====================
  CWorldEntityList::GetEntity
  ====================*/
inline
CWorldEntity*	CWorldEntityList::GetEntity(uint uiIndex, bool bThrow)
{
	try
	{
		if (uiIndex == INVALID_INDEX)
			return NULL;

		if (uiIndex >= m_vEntities.size() || m_vEntities[uiIndex] == INVALID_POOL_HANDLE)
			EX_ERROR(_T("Entity with index ") + XtoA(uiIndex) + _T(" not found"));

		return m_poolWorldEntities.GetReferenceByHandle(m_vEntities[uiIndex]);
	}
	catch (CException &ex)
	{
		ex.Process(_T("CWorldEntityList::GetEntity() - "), bThrow);
		return NULL;
	}
}
//=============================================================================

#endif //__C_WORLDENTITYLIST_H__
