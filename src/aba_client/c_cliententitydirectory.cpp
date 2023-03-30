// (C)2006 S2 Games
// c_cliententitydirectory.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_client_common.h"

#include "c_cliententitydirectory.h"
#include "c_cliententity.h"
#include "c_gameclient.h"

#include "../aba_shared/c_teaminfo.h"
#include "../aba_shared/c_player.h"
#include "../aba_shared/i_entitystate.h"
#include "../aba_shared/i_unitentity.h"
#include "../aba_shared/i_entitytool.h"
#include "../aba_shared/i_bitentity.h"

#include "../k2/c_scenemanager.h"
#include "../k2/c_fontmap.h"
#include "../k2/c_draw2d.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_BOOLF	(cg_debugEntities,	false,	CONEL_DEV);

UnitList CClientEntityDirectory::s_lUnits;
CClientEntityDirectory::ClientEntityPool CClientEntityDirectory::s_pClientEntityPool(256);
//=============================================================================

/*====================
  CClientEntityDirectory::Clear
  ====================*/
void	CClientEntityDirectory::Clear()
{
	for (EntMap_it it(m_mapEntities.begin()); it != m_mapEntities.end(); ++it)
		SAFE_DELETE(it->second);

	m_mapEntities.clear();
	m_lMonoEntities.clear();

	for (ClientEntMap_it it(m_mapClientEntities.begin()); it != m_mapClientEntities.end(); ++it)
	{
		CClientEntity *pClientEntity(GetByHandle(it->second));

		pClientEntity->Free();
		DeleteByHandle(it->second);
	}

	m_mapClientEntities.clear();

	m_vBitBuffer.clear();
	m_vBitEntities.clear();

	m_hFirstClientEntity = INVALID_POOL_HANDLE;
	m_hFirstLocalClientEntity = INVALID_POOL_HANDLE;
}


/*====================
  CClientEntityDirectory::ClearBitEntities
  ====================*/
void	CClientEntityDirectory::ClearBitEntities()
{
	m_vBitBuffer.clear();
	m_vBitEntities.clear();
}


/*====================
  CClientEntityDirectory::Allocate
  ====================*/
IGameEntity*	CClientEntityDirectory::Allocate(uint uiIndex, ushort unType)
{
	try
	{
		if (m_mapEntities.find(uiIndex) != m_mapEntities.end())
			EX_ERROR(_T("Entity #") + XtoA(uiIndex) + _T(" is already allocated"));

		if (m_mapClientEntities.find(uiIndex) != m_mapClientEntities.end())
			EX_ERROR(_T("Entity #") + XtoA(uiIndex) + _T(" is already allocated"));

		IGameEntity *pNewEntity(EntityRegistry.Allocate(unType));
		if (pNewEntity == NULL)
			EX_ERROR(_T("Allocation failed"));

		pNewEntity->SetIndex(uiIndex);

		if (pNewEntity->IsVisual() && !pNewEntity->IsBit())
		{
			if (cg_debugEntities)
				Console << _T("Allocated new client entity #") << uiIndex << _T(" ") << ParenStr(EntityRegistry.LookupName(unType)) << newl;

			PoolHandle hNewClientEntity(Allocate(CClientEntity()));

			CClientEntity *pNewClientEntity(GetByHandle(hNewClientEntity));
			pNewClientEntity->Initialize(pNewEntity->GetAsVisual());
			m_mapClientEntities[uiIndex] = hNewClientEntity;

			ClientEntMap_it itFind(m_mapClientEntities.find(uiIndex));
			if (itFind != m_mapClientEntities.begin())
			{
				--itFind;

				CClientEntity *pPrevClientEntity(GetByHandle(itFind->second));

				pNewClientEntity->SetNextClientEntity(pPrevClientEntity->GetNextClientEntity());
				pPrevClientEntity->SetNextClientEntity(pNewClientEntity);
			}
			else
			{
				pNewClientEntity->SetNextClientEntity(GetByHandle(m_hFirstClientEntity));
				m_hFirstClientEntity = hNewClientEntity;
			}
			
			if (IsLocalEntity(uiIndex))
			{
				if (m_hFirstLocalClientEntity == INVALID_POOL_HANDLE)
					m_hFirstLocalClientEntity = hNewClientEntity;
				else
				{
					CClientEntity *pFirstLocalClientEntity(GetByHandle(m_hFirstLocalClientEntity));

					if (uiIndex < pFirstLocalClientEntity->GetIndex())
						m_hFirstLocalClientEntity = hNewClientEntity;
				}
			}
			
			if (m_mapClientEntities[uiIndex] == INVALID_POOL_HANDLE)
				EX_ERROR(_T("Allocation failed"));
		}
		else
		{
			if (cg_debugEntities)
				Console << _T("Allocated new entity #") << uiIndex << _T(" ") << ParenStr(EntityRegistry.LookupName(unType)) << newl;

			m_mapEntities[uiIndex] = pNewEntity;

			if (!IsLocalEntity(uiIndex) && !pNewEntity->IsBit())
				m_lMonoEntities.push_back(pNewEntity);

			if (m_mapEntities[uiIndex] == NULL)
				EX_ERROR(_T("Allocation failed"));
		}

		return pNewEntity;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CEntityDirectory::Allocate() - "), NO_THROW);
		return NULL;
	}
}

IGameEntity*	CClientEntityDirectory::Allocate(uint uiIndex, const tstring &sTypeName)
{
	return Allocate(uiIndex, EntityRegistry.LookupID(sTypeName));
}


/*====================
  CClientEntityDirectory::AllocateLocal
  ====================*/
IGameEntity*	CClientEntityDirectory::AllocateLocal(ushort unType)
{
	uint uiIndex(0x10000);
	while (m_mapEntities.find(uiIndex) != m_mapEntities.end() ||
		m_mapClientEntities.find(uiIndex) != m_mapClientEntities.end())
		++uiIndex;
	return Allocate(uiIndex, unType);
}

IGameEntity*	CClientEntityDirectory::AllocateLocal(const tstring &sTypeName)
{
	return AllocateLocal(EntityRegistry.LookupID(sTypeName));
}


/*====================
  CClientEntityDirectory::Delete
  ====================*/
void	CClientEntityDirectory::Delete(uint uiIndex)
{
	EntMap_it itFind(m_mapEntities.find(uiIndex));
	ClientEntMap_it itFind2(m_mapClientEntities.find(uiIndex));
	if (itFind == m_mapEntities.end() && itFind2 == m_mapClientEntities.end())
	{
		Console.Warn << _T("Tried to delete entity #") << uiIndex << _T(", which does not exist") << newl;
		return;
	}

	if (itFind2 != m_mapClientEntities.end())
	{
		CClientEntity *pClientEntity(GetByHandle(itFind2->second));

		pClientEntity->PassEffects();

		if (cg_debugEntities)
			Console << _T("Deleting client entity #") << uiIndex << _T(" ") << ParenStr(EntityRegistry.LookupName(pClientEntity->GetType())) << newl;

		if (IsLocalEntity(uiIndex))
		{
			if (m_hFirstLocalClientEntity == GetHandle(pClientEntity))
			{
				CClientEntity *pNextClientEntity(pClientEntity->GetNextClientEntity());
				if (pNextClientEntity != NULL)
					m_hFirstLocalClientEntity = GetHandle(pNextClientEntity);
				else
					m_hFirstLocalClientEntity = INVALID_POOL_HANDLE;
			}
		}

		if (itFind2 != m_mapClientEntities.begin())
		{
			ClientEntMap_it itPrev(itFind2);
			--itPrev;

			CClientEntity *pPrevClientEntity(GetByHandle(itPrev->second));
			pPrevClientEntity->SetNextClientEntity(pClientEntity->GetNextClientEntity());
		}
		else
		{
			CClientEntity *pNextClientEntity(pClientEntity->GetNextClientEntity());
			if (pNextClientEntity != NULL)
				m_hFirstClientEntity = GetHandle(pNextClientEntity);
			else
				m_hFirstClientEntity = INVALID_POOL_HANDLE;
		}

		pClientEntity->Free();

		DeleteByHandle(itFind2->second);
		m_mapClientEntities.erase(itFind2);
	}
	else if (itFind != m_mapEntities.end())
	{
		if (cg_debugEntities)
			Console << _T("Deleting entity #") << uiIndex << _T(" ") << ParenStr(EntityRegistry.LookupName(itFind->second->GetType())) << newl;

		if (!IsLocalEntity(itFind->first) && !itFind->second->IsBit())
			m_lMonoEntities.remove(itFind->second);

		// Update attached entity states and inventory
		if (itFind->second->IsState())
		{
			IEntityState *pState(itFind->second->GetAsState());

			CClientEntity *pOwner(GetClientEntity(pState->GetOwnerIndex()));
			if (pOwner != NULL && 
				pOwner->GetNextEntity() != NULL && 
				pOwner->GetPrevEntity() != NULL && 
				pOwner->GetCurrentEntity() != NULL && 
				pOwner->GetNextEntity()->IsUnit())
			{
				if (pOwner->GetNextEntity()->GetAsUnit() != NULL) pOwner->GetNextEntity()->GetAsUnit()->RemoveState(pState);
				if (pOwner->GetPrevEntity()->GetAsUnit() != NULL) pOwner->GetPrevEntity()->GetAsUnit()->RemoveState(pState);
				if (pOwner->GetCurrentEntity()->GetAsUnit() != NULL) pOwner->GetCurrentEntity()->GetAsUnit()->RemoveState(pState);
			}
		}
		else if (itFind->second->IsTool())
		{
			IEntityTool *pItem(itFind->second->GetAsTool());

			CClientEntity *pOwner(GetClientEntity(pItem->GetOwnerIndex()));
			if (pOwner != NULL && pOwner->GetNextEntity() != NULL && pOwner->GetNextEntity()->IsUnit())
			{
				if (pOwner->GetNextEntity()->GetAsUnit() != NULL &&
					pOwner->GetNextEntity()->GetAsUnit()->GetTool(pItem->GetSlot()) == pItem)
					pOwner->GetNextEntity()->GetAsUnit()->SetInventorySlot(pItem->GetSlot(), NULL);
				if (pOwner->GetPrevEntity() != NULL && 
					pOwner->GetPrevEntity()->GetAsUnit() != NULL &&
					pOwner->GetPrevEntity()->GetAsUnit()->GetTool(pItem->GetSlot()) == pItem)
					pOwner->GetPrevEntity()->GetAsUnit()->SetInventorySlot(pItem->GetSlot(), NULL);
				if (pOwner->GetCurrentEntity() != NULL && 
					pOwner->GetCurrentEntity()->GetAsUnit() != NULL && 
					pOwner->GetCurrentEntity()->GetAsUnit()->GetTool(pItem->GetSlot()) == pItem)
					pOwner->GetCurrentEntity()->GetAsUnit()->SetInventorySlot(pItem->GetSlot(), NULL);
			}
		}

		K2_DELETE(itFind->second);
		m_mapEntities.erase(itFind);
	}
}


/*====================
  CClientEntityDirectory::GetClientEntity
  ====================*/
CClientEntity*	CClientEntityDirectory::GetClientEntity(uint uiIndex)
{
	ClientEntMap_it itFind(m_mapClientEntities.find(uiIndex));
	if (itFind == m_mapClientEntities.end())
		return NULL;

	return GetByHandle(itFind->second);
}


/*====================
  CClientEntityDirectory::GetClientEntityCurrent
  ====================*/
IVisualEntity*	CClientEntityDirectory::GetClientEntityCurrent(uint uiIndex)
{
	ClientEntMap_it itFind(m_mapClientEntities.find(uiIndex));
	if (itFind == m_mapClientEntities.end())
		return NULL;

	return GetByHandle(itFind->second)->GetCurrentEntity();
}


/*====================
  CClientEntityDirectory::GetClientEntityPrev
  ====================*/
IVisualEntity*	CClientEntityDirectory::GetClientEntityPrev(uint uiIndex)
{
	ClientEntMap_it itFind(m_mapClientEntities.find(uiIndex));
	if (itFind == m_mapClientEntities.end())
		return NULL;

	return GetByHandle(itFind->second)->GetPrevEntity();
}


/*====================
  CClientEntityDirectory::GetClientEntityNext
  ====================*/
IVisualEntity*	CClientEntityDirectory::GetClientEntityNext(uint uiIndex)
{
	ClientEntMap_it itFind(m_mapClientEntities.find(uiIndex));
	if (itFind == m_mapClientEntities.end())
		return NULL;

	return GetByHandle(itFind->second)->GetNextEntity();
}


/*====================
  CClientEntityDirectory::GetEntity
  ====================*/
IGameEntity*	CClientEntityDirectory::GetEntity(uint uiIndex)
{
	CClientEntity *pEnt(GetClientEntity(uiIndex));
	if (pEnt)
		return pEnt->GetCurrentEntity();

	EntMap_it itFind(m_mapEntities.find(uiIndex));
	if (itFind != m_mapEntities.end())
		return itFind->second;

	return NULL;
}


/*====================
  CClientEntityDirectory::GetEntityNext
  ====================*/
IGameEntity*	CClientEntityDirectory::GetEntityNext(uint uiIndex)
{
	CClientEntity *pEnt(GetClientEntity(uiIndex));
	if (pEnt)
		return pEnt->GetNextEntity();

	EntMap_it itFind(m_mapEntities.find(uiIndex));
	if (itFind != m_mapEntities.end())
		return itFind->second;

	return NULL;
}


/*====================
  CClientEntityDirectory::GetFirstEntity
  ====================*/
IGameEntity*	CClientEntityDirectory::GetFirstEntity()
{
	ClientEntMap_it itFind(m_mapClientEntities.begin());
	if (itFind == m_mapClientEntities.end())
		return NULL;

	return GetByHandle(itFind->second)->GetCurrentEntity();
}


/*====================
  CClientEntityDirectory::GetNextEntity
  ====================*/
IGameEntity*	CClientEntityDirectory::GetNextEntity(IGameEntity *pEntity)
{
	if (!pEntity)
		return NULL;

	ClientEntMap_it itFind(m_mapClientEntities.find(pEntity->GetIndex()));
	if (itFind == m_mapClientEntities.end())
		return NULL;

	++itFind;

	if (itFind == m_mapClientEntities.end())
		return NULL;

	return GetByHandle(itFind->second)->GetCurrentEntity();
}


/*====================
  CClientEntityDirectory::GetEntities
  ====================*/
void	CClientEntityDirectory::GetEntities(uivector &vResult, ushort unType)
{
	vResult.clear();
}


/*====================
  CClientEntityDirectory::PrepForSnapshot
  ====================*/
void	CClientEntityDirectory::PrepForSnapshot()
{
	PROFILE("CClientEntityDirectory::PrepForSnapshot");

	// Update bit entities
	uivector &vBitEntityBuffer(GameClient.GetCurrentServerSnapshot().GetStreamBitEntityBuffer(0));
	uivector &vCurrentBitEntityBuffer(GetBitEntityBuffer());
	BitEntVector &vBitEntities(GetBitEntities());
	uint uiNumFields(MIN(uint(vCurrentBitEntityBuffer.size()), uint(vBitEntityBuffer.size())));

	uint uiNumBitEntities(uint(vBitEntities.size()));
	for (uint uiGroup(0); uiGroup < uiNumFields; ++uiGroup)
	{
		uint uiOld(vCurrentBitEntityBuffer[uiGroup]);
		uint uiNew(vBitEntityBuffer[uiGroup]);

		if ((uiOld ^ uiNew) == 0)
			continue;

		for (uint uiBit(0); uiBit < 32; ++uiBit)
		{
			uint uiIndex(uiGroup * 32 + uiBit);

			if (uiIndex >= uiNumBitEntities)
				break;

			if ((uiOld ^ uiNew) & BIT(uiBit))
			{
				if (uiNew & BIT(uiBit))
					vBitEntities[uiIndex]->Activate();
				else
					vBitEntities[uiIndex]->Deactivate();
			}
		}

		vCurrentBitEntityBuffer[uiGroup] = uiNew;
	}

	CClientEntity *pEnd(GetFirstLocalClientEntity());
	for (CClientEntity *pClEnt(GetFirstClientEntity()); pClEnt != pEnd; pClEnt = pClEnt->GetNextClientEntity())
	{
		if (pClEnt->GetNextEntity()->GetFrame() != pClEnt->GetPrevEntity()->GetFrame())
			pClEnt->CopyNextToPrev();

		pClEnt->GetNextEntity()->Invalidate();
	}

	for (EntList_it it(m_lMonoEntities.begin()), itEnd(m_lMonoEntities.end()); it != itEnd; ++it)
		(*it)->Invalidate();
}


/*====================
  CClientEntityDirectory::CleanupEntities
  ====================*/
void	CClientEntityDirectory::CleanupEntities()
{
	PROFILE("CClientEntityDirectory::CleanupEntities");

	static uivector vRelease;
	vRelease.clear();

	for (CClientEntity *pClEnt(GetFirstClientEntity()), *pEnd(GetFirstLocalClientEntity()); pClEnt != pEnd; pClEnt = pClEnt->GetNextClientEntity())
	{
		if (!pClEnt->GetNextEntity()->IsValid())
			vRelease.push_back(pClEnt->GetIndex());
	}

	for (EntList_it it(m_lMonoEntities.begin()), itEnd(m_lMonoEntities.end()); it != itEnd; ++it)
	{
		IGameEntity *pEntity(*it);
		if (pEntity->IsValid())
			continue;

		// Make sure we clear out the client entry
		if (pEntity->GetType() == Player)
		{
			int iClientNum(static_cast<CPlayer *>(pEntity)->GetClientNumber());
			GameClient.RemoveClient(iClientNum);
		}

		vRelease.push_back(pEntity->GetIndex());
	}

	uivector_it itEnd(vRelease.end());
	for (uivector_it it(vRelease.begin()); it != itEnd; ++it)
		Delete(*it);

	for (CClientEntity *pClEnt(GetFirstClientEntity()), *pEnd(GetFirstLocalClientEntity()); pClEnt != pEnd; pClEnt = pClEnt->GetNextClientEntity())
	{
		pClEnt->GetNextEntity()->SnapshotUpdate();
		pClEnt->GetCurrentEntity()->SnapshotUpdate();
	}

	for (EntList_it it(m_lMonoEntities.begin()), itEnd(m_lMonoEntities.end()); it != itEnd; ++it)
	{
		IGameEntity *pEntity(*it);

		pEntity->SnapshotUpdate();
	}
}


/*====================
  CClientEntityDirectory::Frame
  ====================*/
void	CClientEntityDirectory::Frame(float fLerp)
{
	PROFILE("CClientEntityDirectory::Frame");

	try
	{
		CClientEntity *pEnd(GetFirstLocalClientEntity());
		for (CClientEntity *pClEnt(GetFirstClientEntity()); pClEnt != pEnd; pClEnt = pClEnt->GetNextClientEntity())
		{
			GameClient.SetCurrentEntity(pClEnt);

			pClEnt->Interpolate(fLerp);
		}

#if 0 // Not current used
		// Client entity frames
		for (ClientEntMap_it it(m_mapClientEntities.begin()); it != m_mapClientEntities.end(); ++it)
		{
			if (m_pClientEntityDirectory->IsLocalEntity(it->first))
				break; // Every entity after this will be local

			CClientEntity *pEntity(it->second);

			GameClient.SetCurrentEntity(pEntity);

			pEntity->Frame();
		}
#endif
	}
	catch (CException &ex)
	{
		ex.Process(_T("CClientEntityDirectory::Frame() -"), NO_THROW);
	}
}


/*====================
  CClientEntityDirectory::PopulateScene
  ====================*/
void	CClientEntityDirectory::PopulateScene()
{
	PROFILE("CClientEntityDirectory::PopulateScene");

	for (CClientEntity *pClEnt(GetFirstClientEntity()); pClEnt != NULL; pClEnt = pClEnt->GetNextClientEntity())
	{
		GameClient.SetCurrentEntity(pClEnt);

		pClEnt->AddToScene();
	}
}


/*====================
  CClientEntityDirectory::DrawScreen
  ====================*/
void	CClientEntityDirectory::DrawScreen()
{
	static ResHandle hFont(g_ResourceManager.LookUpName(_T("system_medium"), RES_FONTMAP));
	CFontMap *pFontMap(g_ResourceManager.GetFontMap(hFont));

	for (CClientEntity *pClEnt(GetFirstClientEntity()); pClEnt != NULL; pClEnt = pClEnt->GetNextClientEntity())
	{
		IVisualEntity *pVisual(pClEnt->GetCurrentEntity());

		tstring sText(pVisual->GetTypeName());

		CVec2f v2ScreenPos;
		if (GameClient.GetCamera()->WorldToScreen(pVisual->GetPosition(), v2ScreenPos))
		{
			CVec2f v2StringPos(ROUND(v2ScreenPos.x - pFontMap->GetStringWidth(sText) / 2.0f), ROUND(v2ScreenPos.y - pFontMap->GetMaxHeight() / 2.0f));

			Draw2D.SetColor(BLACK);
			Draw2D.String(v2StringPos.x + 2, v2StringPos.y + 2, sText, hFont);

			Draw2D.SetColor(ORANGE);
			Draw2D.String(v2StringPos.x, v2StringPos.y, sText, hFont);
		}
	}
}


/*====================
  CClientEntityDirectory::AddBitEntity
  ====================*/
void	CClientEntityDirectory::AddBitEntity(IBitEntity *pBit)
{
	m_vBitEntities.push_back(pBit);

	if (m_vBitEntities.size() > m_vBitBuffer.size() * 32)
		m_vBitBuffer.resize((m_vBitEntities.size() - 1) / 32 + 1, uint(-1));
}


/*====================
  CClientEntityDirectory::UpdateDefinitions
  ====================*/
void	CClientEntityDirectory::UpdateDefinitions(ushort unType)
{
	if (GameClient.GetClient()->GetState() < CLIENT_STATE_IN_GAME)
		return;

	Game.Precache(unType, PRECACHE_ALL);

	for (EntMap_it it(m_mapEntities.begin()), itEnd(m_mapEntities.end()); it != itEnd; ++it)
	{
		if (it->second->GetType() != unType)
			continue;
		
		it->second->UpdateDefinition();
	}

	for (ClientEntMap_it it(m_mapClientEntities.begin()), itEnd(m_mapClientEntities.end()); it != itEnd; ++it)
	{
		CClientEntity *pClientEntity(GetByHandle(it->second));

		if (pClientEntity->GetType() != unType)
			continue;

		if (pClientEntity->GetNextEntity() != NULL)
			pClientEntity->GetNextEntity()->UpdateDefinition();
		if (pClientEntity->GetPrevEntity() != NULL)
			pClientEntity->GetPrevEntity()->UpdateDefinition();
		if (pClientEntity->GetCurrentEntity() != NULL)
			pClientEntity->GetCurrentEntity()->UpdateDefinition();
	}
}


/*====================
  CClientEntityDirectory::Rewind
  ====================*/
void	CClientEntityDirectory::Rewind()
{
	CClientEntity *pEnd(GetFirstLocalClientEntity());
	for (CClientEntity *pClEnt(GetFirstClientEntity()); pClEnt != pEnd; pClEnt = pClEnt->GetNextClientEntity())
	{
		GameClient.SetCurrentEntity(pClEnt);

		pClEnt->Rewind();
	}
}
