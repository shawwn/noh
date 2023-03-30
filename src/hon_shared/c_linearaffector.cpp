// (C)2009 S2 Games
// c_linearaffector.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_linearaffector.h"

#include "i_unitentity.h"
#include "i_bitentity.h"

#include "../k2/c_hostclient.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_resourcemanager.h"
#include "../k2/c_resourceinfo.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
EXTERN_CVAR_BOOL(d_drawAreaAffectors);
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
uint			CLinearAffector::s_uiBaseType(ENTITY_BASE_TYPE_LINEAR_AFFECTOR);
ResHandle		CLinearAffector::s_hDebugMaterial(INVALID_RESOURCE);

DEFINE_ENTITY_DESC(CLinearAffector, 1)
{
	s_cDesc.pFieldTypes = K2_NEW(ctx_Game,    TypeVector)();
	s_cDesc.pFieldTypes->clear();
	const TypeVector &vBase(IVisualEntity::GetTypeVector());
	s_cDesc.pFieldTypes->insert(s_cDesc.pFieldTypes->begin(), vBase.begin(), vBase.end());

	s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiLevel"), TYPE_INT, 5, 0));
	s_cDesc.pFieldTypes->push_back(SDataField(_T("m_v3TargetPosition"), TYPE_ROUNDPOS3D, 0, 0));
}
//=============================================================================

/*====================
  CLinearAffector::CLinearAffector
  ====================*/
CLinearAffector::CLinearAffector() :
m_uiOwnerIndex(INVALID_INDEX),
m_uiAttachTargetUID(INVALID_INDEX),
m_uiFirstTargetIndex(INVALID_INDEX),
m_uiCreationTime(INVALID_TIME),
m_uiLastImpactTime(INVALID_TIME),
m_uiTotalImpactCount(0),
m_uiLevel(1),
m_uiChainCount(0)
{
}


/*====================
  CLinearAffector::Baseline
  ====================*/
void	CLinearAffector::Baseline()
{
	IVisualEntity::Baseline();

	m_uiLevel = 1;
	m_v3TargetPosition.Clear();
}


/*====================
  CLinearAffector::GetSnapshot
  ====================*/
void	CLinearAffector::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
	IVisualEntity::GetSnapshot(snapshot, uiFlags);

	snapshot.WriteField(m_uiLevel);
	snapshot.WriteRoundPos3D(m_v3TargetPosition);
}


/*====================
  CLinearAffector::ReadSnapshot
  ====================*/
bool	CLinearAffector::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
	if (!IVisualEntity::ReadSnapshot(snapshot, 1))
		return false;

	snapshot.ReadField(m_uiLevel);
	snapshot.ReadRoundPos3D(m_v3TargetPosition);

	return true;
}


/*====================
  CLinearAffector::Copy
  ====================*/
void	CLinearAffector::Copy(const IGameEntity &B)
{
	IVisualEntity::Copy(B);

	const CLinearAffector *pB(B.GetAsLinearAffector());
	if (!pB)
		return;
	const CLinearAffector &C(*pB);

	m_uiOwnerIndex =			C.m_uiOwnerIndex;
	m_uiCreationTime =			C.m_uiCreationTime;
	m_uiLastImpactTime =		C.m_uiLastImpactTime;
	m_uiTotalImpactCount =		C.m_uiTotalImpactCount;
	m_uiIntervalCount =			C.m_uiIntervalCount;
	m_uiLevel =					C.m_uiLevel;
	m_v3TargetPosition =		C.m_v3TargetPosition;
}


/*====================
  CLinearAffector::Spawn
  ====================*/
void	CLinearAffector::Spawn()
{
	IVisualEntity::Spawn();

	m_uiCreationTime = Game.GetGameTime();
	m_uiTotalImpactCount = 0;
	m_uiIntervalCount = 0;
	m_uiLastImpactTime = INVALID_TIME;

	SetEffect(0, GetEffect());

	// Spawn event
	if (Game.IsServer())
	{
		CLinearAffectorDefinition *pDefinition(GetDefinition<CLinearAffectorDefinition>(GetModifierBits()));
		if (pDefinition != NULL)
			pDefinition->ExecuteActionScript(ACTION_SCRIPT_SPAWN, this, GetOwner(), this, NULL, GetPosition(), NULL, GetLevel());
	}

	K2_WITH_GAME_RESOURCE_SCOPE()
		s_hDebugMaterial = g_ResourceManager.Register(_T("/shared/materials/area_affector.material"), RES_MATERIAL);
}


/*====================
  CLinearAffector::GetPotentialImpactCount
  ====================*/
uint	CLinearAffector::GetPotentialImpactCount(uint uiTargetUID, uint uiTotalRemainingImpacts)
{
	if (uiTotalRemainingImpacts == 0)
		return 0;

	// Get previous impact count
	uint uiPrevImpacts(0);
	map<uint, uint>::iterator itFind(m_mapImpacts.find(uiTargetUID));
	if (itFind != m_mapImpacts.end())
		uiPrevImpacts = itFind->second;

	// Determine maximum possible impacts
	uint uiPossibleImapcts(uiTotalRemainingImpacts);
	if (GetMaxImpactsPerTarget() > 0)
	{
		if (uiPrevImpacts >= GetMaxImpactsPerTarget())
			return 0;

		uiPossibleImapcts = MIN(uiPossibleImapcts, GetMaxImpactsPerTarget() - uiPrevImpacts);
	}

	if (GetMaxImpactsPerTargetPerInterval() > 0)
		uiPossibleImapcts = MIN(uiPossibleImapcts, GetMaxImpactsPerTargetPerInterval());

	return uiPossibleImapcts;
}


/*====================
  CLinearAffector::ImpactEntity
  ====================*/
void	CLinearAffector::ImpactEntity(IUnitEntity *pTarget)
{
	if (pTarget == NULL)
		return;

	//Console << _T("CLinearAffector::ImpactEntity ") << Game.GetGameTime() << _T(" ") << pTarget->GetIndex() << newl;

	// Impact effect
	ResHandle hImpactEffect(GetImpactEffect());
	if (hImpactEffect != INVALID_RESOURCE)
	{
		CGameEvent evImpact;
		evImpact.SetSourceEntity(pTarget->GetIndex());
		evImpact.SetEffect(hImpactEffect);
		Game.AddEvent(evImpact);
	}

	// Bridge/Link effect
	ResHandle hBridgeEffect(GetBridgeEffect());
	ResHandle hLinkEffect(GetLinkEffect());
	if (GetChainCount() > 0 && hLinkEffect != INVALID_RESOURCE)
	{
		IUnitEntity *pAttachTarget(GetAttachTarget());
		if (pAttachTarget != NULL)
		{
			CGameEvent evBridge;
			evBridge.SetSourceEntity(pAttachTarget->GetIndex());
			evBridge.SetTargetEntity(pTarget->GetIndex());
			evBridge.SetEffect(hLinkEffect);
			Game.AddEvent(evBridge);
		}
	}
	else if (hBridgeEffect != INVALID_RESOURCE)
	{
		IUnitEntity *pAttachTarget(GetOwner());
		if (pAttachTarget != NULL)
		{
			CGameEvent evBridge;
			evBridge.SetSourceEntity(pAttachTarget->GetIndex());
			evBridge.SetTargetEntity(pTarget->GetIndex());
			evBridge.SetEffect(hBridgeEffect);
			Game.AddEvent(evBridge);
		}
	}
	
	map<uint, uint>::iterator itFind(m_mapImpacts.find(pTarget->GetUniqueID()));
	if (itFind == m_mapImpacts.end())
		m_mapImpacts.insert(pair<uint, uint>(pTarget->GetUniqueID(), 1));
	else
		++itFind->second;

	++m_uiTotalImpactCount;

	// Impact event
	CLinearAffectorDefinition *pDefinition(GetDefinition<CLinearAffectorDefinition>(GetModifierBits()));
	if (pDefinition != NULL)
		pDefinition->ExecuteActionScript(ACTION_SCRIPT_IMPACT, this, GetOwner(), this, pTarget, pTarget->GetPosition(), NULL, GetLevel());
}


/*====================
  CLinearAffector::Impact
  ====================*/
void	CLinearAffector::Impact(vector<IUnitEntity*> &vTargets)
{
	// Interval event
	CLinearAffectorDefinition *pDefinition(GetDefinition<CLinearAffectorDefinition>(GetModifierBits()));
	if (pDefinition != NULL)
		pDefinition->ExecuteActionScript(ACTION_SCRIPT_INTERVAL, this, GetOwner(), this, NULL, GetPosition(), NULL, GetLevel());
	
	SubSegmentImpacts();

	if (vTargets.empty() && m_uiFirstTargetIndex == INVALID_INDEX && GetTargetSelection() != TARGET_SELECT_RANDOM_POSITION)
		return;

	// Determine total possible impacts
	uint uiRemainingImpacts(UINT_MAX);
	if (GetMaxTotalImpacts() > 0)
	{
		if (m_uiTotalImpactCount >= GetMaxTotalImpacts())
			uiRemainingImpacts = 0;
		else
			uiRemainingImpacts = GetMaxTotalImpacts() - m_uiTotalImpactCount;
	}

	if (GetMaxImpactsPerInterval() > 0)
		uiRemainingImpacts = MIN(uiRemainingImpacts, GetMaxImpactsPerInterval());

	// Always hit the first target if it hasn't been hit yet
	if (m_uiFirstTargetIndex != INVALID_INDEX)
	{
		IUnitEntity *pTarget(Game.GetUnitEntity(m_uiFirstTargetIndex));
		m_uiFirstTargetIndex = INVALID_INDEX;
		if (pTarget != NULL && Game.IsValidTarget(GetTargetScheme(), GetEffectType(), GetOwner(), pTarget))
		{
			ImpactEntity(pTarget);
			if (uiRemainingImpacts <= 1)
				return;
			--uiRemainingImpacts;
		}
	}

	vector<IUnitEntity*> vImpacts;

	// Sort the list
	switch (GetTargetSelection())
	{
	case TARGET_SELECT_ALL:
		for (vector<IUnitEntity*>::iterator it(vTargets.begin()); it != vTargets.end(); ++it)
		{
			IUnitEntity *pTarget(*it);

			uint uiNumImpacts(GetPotentialImpactCount(pTarget->GetUniqueID(), uiRemainingImpacts));
			for (uint uiImpacts(0); uiImpacts < uiNumImpacts; ++uiImpacts)
				vImpacts.push_back(pTarget);
			
			uiRemainingImpacts -= uiNumImpacts;
			if (uiRemainingImpacts == 0)
				break;
		}
		break;

	case TARGET_SELECT_CLOSEST:
	case TARGET_SELECT_FURTHEST:
		// Sort vTargets
		for (uint uiPass(0); uiPass < vTargets.size(); ++uiPass)
		{
			float fShortestDistance(FAR_AWAY);
			uint uiClosestIndex(-1);
			for (uint ui(uiPass); ui < vTargets.size(); ++ui)
			{
				float fDistance(DistanceSq(vTargets[ui]->GetPosition().xy(), GetPosition().xy()));
				if (fDistance < fShortestDistance)
				{
					fShortestDistance = fDistance;
					uiClosestIndex = ui;
				}
			}

			IUnitEntity *pTemp(vTargets[uiPass]);
			vTargets[uiPass] = vTargets[uiClosestIndex];
			vTargets[uiClosestIndex] = pTemp;
		}

		if (GetTargetSelection() == TARGET_SELECT_FURTHEST)
			std::reverse(vTargets.begin(), vTargets.end());

		// Fill impact vector
		for (vector<IUnitEntity*>::iterator it(vTargets.begin()); it != vTargets.end(); ++it)
		{
			IUnitEntity *pTarget(*it);

			uint uiNumImpacts(GetPotentialImpactCount(pTarget->GetUniqueID(), uiRemainingImpacts));
			for (uint uiImpacts(0); uiImpacts < uiNumImpacts; ++uiImpacts)
				vImpacts.push_back(pTarget);
			
			uiRemainingImpacts -= uiNumImpacts;
			if (uiRemainingImpacts == 0)
				break;
		}
		break;

	case TARGET_SELECT_RANDOM:
		{
			// Fill a vector with all possible impacts
			vector<IUnitEntity*> vPotentialImpacts;
			for (vector<IUnitEntity*>::iterator it(vTargets.begin()); it != vTargets.end(); ++it)
			{
				IUnitEntity *pTarget(*it);

				uint uiNumImpacts(GetPotentialImpactCount(pTarget->GetUniqueID(), uiRemainingImpacts));
				for (uint uiImpacts(0); uiImpacts < uiNumImpacts; ++uiImpacts)
					vPotentialImpacts.push_back(pTarget);
			}

			std::random_shuffle(vPotentialImpacts.begin(), vPotentialImpacts.end());
			
			if (uiRemainingImpacts < vPotentialImpacts.size())
				vPotentialImpacts.resize(uiRemainingImpacts);
			
			vImpacts.insert(vImpacts.end(), vPotentialImpacts.begin(), vPotentialImpacts.end());
		}
		break;

	case TARGET_SELECT_RANDOM_POSITION:
		{
			CLinearAffectorDefinition *pDefinition(GetDefinition<CLinearAffectorDefinition>(GetModifierBits()));
			if (pDefinition == NULL)
				break;

			if (uiRemainingImpacts == -1)
				uiRemainingImpacts = 1;
			for (uint ui(0); ui < uiRemainingImpacts; ++ui)
			{
				CVec2f v2Dir(GetTargetPosition().xy() - GetPosition().xy());
				float fLength(v2Dir.Normalize());
				CVec2f v2Position(GetPosition().xy() + (v2Dir * fLength * M_Randnum(0.0f, 1.0f)) + (M_RandomPointInCircle() * GetRadius()));
				pDefinition->ExecuteActionScript(ACTION_SCRIPT_IMPACT, this, GetOwner(), this, NULL, Game.GetTerrainPosition(v2Position), NULL, GetLevel());
			}
		}
		break;
	}

	// Do the impacts
	for (vector<IUnitEntity*>::iterator it(vImpacts.begin()); it != vImpacts.end(); ++it)
		ImpactEntity(*it);
}


/*====================
  CLinearAffector::SubSegmentImpacts
  ====================*/
void	CLinearAffector::SubSegmentImpacts()
{
	if (GetSubSegmentLength() <= 0.0f)
		return;

	CVec2f v2Segment(GetTargetPosition().xy() - GetPosition().xy());
	float fLength(v2Segment.Normalize());
	fLength = CLAMP(fLength, GetMinLength(), GetMaxLength());
	CVec2f v2Dir(v2Segment);
	v2Segment *= fLength;
	CVec3f v3EndPoint(Game.GetTerrainPosition(GetPosition().xy() + v2Segment));

	for (float fSegment(GetSubSegmentOffset()); fSegment <= fLength; fSegment += GetSubSegmentLength())
	{
		// Impact event
		CLinearAffectorDefinition *pDefinition(GetDefinition<CLinearAffectorDefinition>(GetModifierBits()));
		if (pDefinition != NULL)
			pDefinition->ExecuteActionScript(ACTION_SCRIPT_IMPACT, this, GetOwner(), this, NULL, Game.GetTerrainPosition(GetPosition().xy() + v2Dir * fSegment), NULL, GetLevel());
	}
}


/*====================
  CLinearAffector::ServerFrameSetup
  ====================*/
bool	CLinearAffector::ServerFrameSetup()
{
	if (!GetPersist() && m_uiAttachTargetUID != INVALID_INDEX)
	{
		IVisualEntity *pTarget(Game.GetVisualEntity(Game.GetGameIndexFromUniqueID(m_uiAttachTargetUID)));
		if (pTarget == NULL || pTarget->GetStatus() != ENTITY_STATUS_ACTIVE)
			return false;
	}

	return true;
}


/*====================
  CLinearAffector::ServerFrameMovement
  ====================*/
bool	CLinearAffector::ServerFrameMovement()
{
	if (!IVisualEntity::ServerFrameMovement())
		return false;

	IVisualEntity *pTarget(Game.GetVisualEntity(Game.GetGameIndexFromUniqueID(m_uiAttachTargetUID)));
	if (pTarget != NULL)
	{
		SetPosition(pTarget->GetPosition());
		SetAngles(pTarget->GetAngles());
	}

	return true;
}


/*====================
  CLinearAffector::ServerFrameAction
  ====================*/
bool	CLinearAffector::ServerFrameAction()
{
	if (!IVisualEntity::ServerFrameAction())
		return false;

	// Frame event
	CLinearAffectorDefinition *pDefinition(GetDefinition<CLinearAffectorDefinition>(GetModifierBits()));
	if (pDefinition != NULL)
		pDefinition->ExecuteActionScript(ACTION_SCRIPT_FRAME, this, GetOwner(), this, NULL, GetPosition(), NULL, GetLevel());

	bool bImpact(false);

	// Determin if we'll impact this frame
	if ((GetMaxTotalImpacts() == 0 || m_uiTotalImpactCount < GetMaxTotalImpacts()) &&
		Game.GetGameTime() >= m_uiCreationTime + GetImpactDelay() &&
		(GetMaxIntervals() == 0 || m_uiIntervalCount < GetMaxIntervals()))
	{
		if (GetImpactInterval() == 0)
		{
			bImpact = true;
		}
		else
		{
			if (m_uiLastImpactTime == INVALID_TIME)
			{
				bImpact = true;
				m_uiLastImpactTime = Game.GetGameTime();
			}

			while (Game.GetGameTime() - m_uiLastImpactTime >= GetImpactInterval())
			{
				bImpact = true;
				m_uiLastImpactTime += GetImpactInterval();
				if (GetMaxTotalImpacts() > 0 && m_uiTotalImpactCount >= GetMaxTotalImpacts())
					break;
			}
		}
	}

	if (!bImpact)
		return true;

	++m_uiIntervalCount;

	float fRadius(GetRadius());
	CVec2f v2Segment(GetTargetPosition().xy() - GetPosition().xy());
	float fLength(v2Segment.Normalize());
	fLength = CLAMP(fLength, GetMinLength(), GetMaxLength());
	v2Segment *= fLength;
	CVec2f v2EndPoint(Game.GetTerrainPosition(GetPosition().xy() + v2Segment));
	CVec3f v3MidPoint(Game.GetTerrainPosition(GetPosition().xy() + v2Segment * 0.5f));

	CAxis axisSource(GetAngles());
	CVec2f v2Forward(axisSource.Forward2d());

	bool bDestroyTrees(GetDestroyTrees());
	IUnitEntity *pOwner(GetOwner());
	uint uiTargetScheme(GetTargetScheme());
	uint uiEffectType(GetEffectType());

	uint uiIgnoreFlags(bDestroyTrees ? REGION_UNITS_AND_TREES : REGION_UNIT);

	// Find all entities within the outer radius
	static uivector vEntities;
	vEntities.clear();
	Game.GetEntitiesInRadius(vEntities, v3MidPoint.xy(), fLength + fRadius, uiIgnoreFlags);

	static vector<IUnitEntity*> vTargets;
	vTargets.clear();
	for (uivector_it it(vEntities.begin()), itEnd(vEntities.end()); it != itEnd; ++it)
	{
		uint uiTargetIndex(Game.GetGameIndexFromWorldIndex(*it));
		if (uiTargetIndex == INVALID_INDEX)
			continue;

		IUnitEntity *pTarget(Game.GetUnitEntity(uiTargetIndex));
		if (pTarget == NULL)
			continue;

		// Check distance to line
		if (M_DistanceSqToSegment2d(GetPosition().xy(), v2EndPoint, pTarget->GetPosition().xy()) > SQR(fRadius - pTarget->GetBoundsRadius()))
			continue;

		if (pTarget->IsBit())
		{
			if (!bDestroyTrees || pTarget->GetType() != Prop_Tree)
				continue;

			IBitEntity *pTargetBit(pTarget->GetAsBit());
			
			CWorldEntity *pWorldEnt(Game.GetWorldEntity(pTargetBit->GetWorldIndex()));
			if (pWorldEnt == NULL)
				continue;
			
			pTargetBit->Die(pOwner);
		}
		else
		{
			// Check targeting rules
			if (!Game.IsValidTarget(uiTargetScheme, uiEffectType, pOwner, pTarget))
				continue;

			// Execute frame script, this hits all valid targets every frame, ignoring impact rules
			if (pDefinition != NULL)
				pDefinition->ExecuteActionScript(ACTION_SCRIPT_FRAME, this, pOwner, this, pTarget, pTarget->GetPosition(), NULL, GetLevel());

			// Skip first target (added in Impact)
			if (pTarget->GetIndex() == m_uiFirstTargetIndex)
				continue;

			// Filter already impacted entities
			if (GetMaxImpactsPerTarget() > 0)
			{
				map<uint, uint>::iterator itFind(m_mapImpacts.find(pTarget->GetUniqueID()));
				if (itFind != m_mapImpacts.end() && itFind->second > 0)
				{
					if (itFind->second >= GetMaxImpactsPerTarget())
						continue;
				}
			}

			vTargets.push_back(pTarget);
		}
	}

	// Impact with potential targets
	Impact(vTargets);

	return true;
}


/*====================
  CLinearAffector::ServerFrameCleanup
  ====================*/
bool	CLinearAffector::ServerFrameCleanup()
{
	if (GetLifetime() != 0 && GetLifetime() != uint(-1) && Game.GetGameTime() - m_uiCreationTime > GetLifetime())
		return false;
	if (GetLifetime() == 0 && m_uiIntervalCount > 0)
		return false;
	if (GetMaxIntervals() != 0 && m_uiIntervalCount >= GetMaxIntervals())
		return false;

	return IVisualEntity::ServerFrameCleanup();
}


/*====================
  CLinearAffector::AddToScene
  ====================*/
bool	CLinearAffector::AddToScene(const CVec4f &v4Color, int iFlags)
{
	if (m_v3AxisAngles != m_v3Angles)
	{
		m_aAxis.Set(m_v3Angles);
		m_v3AxisAngles = m_v3Angles;
	}

	// If the cvar is false or it isn't a practice/replay game, return as it can be used to reveal abilities in game it shouldn't be able to
	if (!d_drawAreaAffectors || (Host.GetActiveClient() != NULL && !(Host.GetActiveClient()->GetPractice() || Host.IsReplay())))
		return true;

	float fRadius(GetRadius());
	CVec2f v2Segment(GetTargetPosition().xy() - GetPosition().xy());
	float fLength(v2Segment.Normalize());
	fLength = CLAMP(fLength, GetMinLength(), GetMaxLength());
	CVec2f v2Dir(v2Segment);
	v2Segment *= fLength;
	CVec3f v3EndPoint(Game.GetTerrainPosition(GetPosition().xy() + v2Segment));

	for (float fSegment(0.0f); fSegment <= fLength; fSegment += GetSubSegmentLength())
	{
		CSceneEntity scSegment;
		scSegment.Clear();
		scSegment.width = GetSubSegmentLength();
		scSegment.height = GetSubSegmentLength();
		scSegment.scale = 1.0f;
		scSegment.SetPosition(Game.GetTerrainPosition(GetPosition().xy() + v2Dir * fSegment));
		scSegment.angle = GetAngles();
		scSegment.objtype = OBJTYPE_GROUNDSPRITE;
		scSegment.hRes = s_hDebugMaterial;
		scSegment.flags = SCENEENT_SOLID_COLOR;
		scSegment.color = YELLOW;
		SceneManager.AddEntity(scSegment);
	}

	CSceneEntity scOrigin;
	scOrigin.Clear();
	scOrigin.width = fRadius;
	scOrigin.height = fRadius;
	scOrigin.scale = 1.0f;
	scOrigin.SetPosition(GetPosition());
	scOrigin.angle = GetAngles();
	scOrigin.objtype = OBJTYPE_GROUNDSPRITE;
	scOrigin.hRes = s_hDebugMaterial;
	scOrigin.flags = SCENEENT_SOLID_COLOR;
	scOrigin.color = LIME;
	SceneManager.AddEntity(scOrigin);

	CSceneEntity scTarget;
	scTarget.Clear();
	scTarget.width = fRadius;
	scTarget.height = fRadius;
	scTarget.scale = 1.0f;
	scTarget.SetPosition(v3EndPoint);
	scTarget.angle = GetAngles();
	scTarget.objtype = OBJTYPE_GROUNDSPRITE;
	scTarget.hRes = s_hDebugMaterial;
	scTarget.flags = SCENEENT_SOLID_COLOR;
	scTarget.color = RED;
	SceneManager.AddEntity(scTarget);

	return true;
}


/*====================
  CLinearAffector::UpdateModifiers
  ====================*/
void	CLinearAffector::UpdateModifiers(const uivector &vModifiers)
{
	m_vModifierKeys = vModifiers;

	uint uiModifierBits(0);
	if (m_uiActiveModifierKey != INVALID_INDEX)
		uiModifierBits |= GetModifierBit(m_uiActiveModifierKey);

	SetModifierBits(uiModifierBits | GetModifierBits(vModifiers));

	// Activate conditional modifiers
	IUnitEntity *pOwner(GetOwner());
	if (pOwner == NULL)
		return;

	CEntityDefinitionResource *pResource(g_ResourceManager.Get<CEntityDefinitionResource>(m_hDefinition));
	if (pResource == NULL)
		return;
	IEntityDefinition *pDefinition(pResource->GetDefinition<IEntityDefinition>());
	if (pDefinition == NULL)
		return;

	const EntityModifierMap &mapModifiers(pDefinition->GetModifiers());
	for (EntityModifierMap::const_iterator cit(mapModifiers.begin()), citEnd(mapModifiers.end()); cit != citEnd; ++cit)
	{
		const tstring &sCondition(cit->second->GetCondition());
		if (sCondition.empty())
			continue;

		tsvector vsTypes(TokenizeString(sCondition, _T(' ')));

		tsvector_cit itType(vsTypes.begin()), itTypeEnd(vsTypes.end());
		for (; itType != itTypeEnd; ++itType)
		{
			if (!itType->empty() && (*itType)[0] == _T('!'))
			{
				if (pOwner->IsTargetType(itType->substr(1), pOwner))
					break;
			}
			else
			{
				if (!pOwner->IsTargetType(*itType, pOwner))
					break;
			}
		}
		if (itType == itTypeEnd)
			SetModifierBits(GetModifierBits() | cit->first);
	}
}
