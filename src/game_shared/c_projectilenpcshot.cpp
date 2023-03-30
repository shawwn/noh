// (C)2006 S2 Games
// c_projectilenpcshot.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_projectilenpcshot.h"
#include "c_npcability.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Projectile, NpcShot);

vector<SDataField>*	CProjectileNpcShot::s_pvFields;
//=============================================================================

/*====================
  CProjectileNpcShot::CProjectileNpcShot
  ====================*/
CProjectileNpcShot::CProjectileNpcShot() :
IProjectile(GetEntityConfig()),

m_fEffectRadius(0.0f)
{
}


/*====================
  CProjectileNpcShot::GetTypeVector
  ====================*/
const vector<SDataField>&	CProjectileNpcShot::GetTypeVector()
{
	if (!s_pvFields)
	{
		s_pvFields = K2_NEW(global,   vector<SDataField>)();
		s_pvFields->clear();
		const vector<SDataField> &vBase(IProjectile::GetTypeVector());
		s_pvFields->insert(s_pvFields->begin(), vBase.begin(), vBase.end());
		
		s_pvFields->push_back(SDataField(_T("m_fScale"), FIELD_PUBLIC, TYPE_FLOAT));
		s_pvFields->push_back(SDataField(_T("m_hModel"), FIELD_PUBLIC, TYPE_RESHANDLE));
		
		// Effects
		for (int i(0); i < 1; ++i)
		{
			s_pvFields->push_back(SDataField(_T("m_ahEffect[") + XtoA(i) + _T("]"), FIELD_PUBLIC, TYPE_RESHANDLE));
			s_pvFields->push_back(SDataField(_T("m_ayEffectSequence[") + XtoA(i) + _T("]"), FIELD_PUBLIC, TYPE_CHAR));
		}
	}

	return *s_pvFields;
}


/*====================
  CProjectileNpcShot::GetSnapshot
  ====================*/
void	CProjectileNpcShot::GetSnapshot(CEntitySnapshot &snapshot) const
{
	IProjectile::GetSnapshot(snapshot);

	snapshot.AddField(m_fScale);
	snapshot.AddResHandle(m_hModel);
	
	// Effects
	for (int i(0); i < 1; ++i)
	{
		snapshot.AddResHandle(m_ahEffect[i]);
		snapshot.AddField(m_ayEffectSequence[i]);
	}
}


/*====================
  CProjectileNpcShot::ReadSnapshot
  ====================*/
bool	CProjectileNpcShot::ReadSnapshot(CEntitySnapshot &snapshot)
{
	try
	{
		if (!IProjectile::ReadSnapshot(snapshot))
			return false;

		snapshot.ReadNextField(m_fScale);
		snapshot.ReadNextResHandle(m_hModel);

		// Effects
		for (int i(0); i < 1; ++i)
		{
			snapshot.ReadNextResHandle(m_ahEffect[i]);
			snapshot.ReadNextField(m_ayEffectSequence[i]);
		}

		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CProjectileNpcShot::ReadSnapshot() - "), NO_THROW);
		return false;
	}
}


/*====================
  CProjectileNpcShot::Baseline
  ====================*/
void	CProjectileNpcShot::Baseline()
{
	IProjectile::Baseline();

	m_fScale = 1.0f;
	m_hModel = INVALID_INDEX;
	
	// Effects
	for (int i(0); i < 1; ++i)
	{
		m_ahEffect[i] = INVALID_INDEX;
		m_ayEffectSequence[i] = 0;
	}
}


/*====================
  CProjectileNpcShot::Killed
  ====================*/
void	CProjectileNpcShot::Killed()
{
	// Death event
	if (m_hDeathEffect != INVALID_RESOURCE)
	{
		CGameEvent evDeath;
		evDeath.SetSourcePosition(m_v3Position);
		evDeath.SetSourceAngles(m_v3Angles);
		evDeath.SetEffect(m_hDeathEffect);
		Game.AddEvent(evDeath);
	}

	if (m_pImpactEntity != NULL)
	{
		for (EffectVector::iterator itEffect(m_vTargetEffects.begin()); itEffect != m_vTargetEffects.end(); ++itEffect)
			itEffect->ApplyEffect(m_pImpactEntity->GetIndex(), m_uiOwnerIndex);

		for (EffectVector::iterator itEffect(m_vSourceEffects.begin()); itEffect != m_vSourceEffects.end(); ++itEffect)
			itEffect->ApplyEffect(m_uiOwnerIndex, m_uiOwnerIndex);
	}

	if (m_fEffectRadius > 0.0f)
	{
		CSphere	cSphere(m_v3Position, m_fEffectRadius);

		uivector	vResult;
		Game.GetEntitiesInRadius(vResult, cSphere, 0);
		for (uivector_it it(vResult.begin()); it != vResult.end(); ++it)
		{
			IGameEntity *pEnt(Game.GetEntityFromWorldIndex(*it));
			if (pEnt == NULL || pEnt == m_pImpactEntity)
				continue;

			for (EffectVector::iterator itEffect(m_vTargetEffects.begin()); itEffect != m_vTargetEffects.end(); ++itEffect)
				itEffect->ApplyEffect(pEnt->GetIndex(), m_uiOwnerIndex);

			for (EffectVector::iterator itEffect(m_vSourceEffects.begin()); itEffect != m_vSourceEffects.end(); ++itEffect)
				itEffect->ApplyEffect(m_uiOwnerIndex, m_uiOwnerIndex);
		}
	}
}
