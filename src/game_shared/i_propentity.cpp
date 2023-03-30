// (C)2006 S2 Games
// i_propentity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_propentity.h"

#include "../k2/c_model.h"
#include "../k2/c_k2model.h"
#include "../k2/c_worldentity.h"
#include "../k2/c_skeleton.h"
#include "../k2/c_world.h"
#include "../k2/c_texture.h"
#include "../k2/intersection.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
extern CCvar<float>		p_stepHeight;

//=============================================================================
// Definitions
//=============================================================================
vector<SDataField>*	IPropEntity::s_pvFields;
//=============================================================================

/*====================
  IPropEntity::CEntityConfig::CEntityConfig
  ====================*/
IPropEntity::CEntityConfig::CEntityConfig(const tstring &sName) :
IVisualEntity::CEntityConfig(sName),
INIT_ENTITY_CVAR(MinimapColor, CVec3f(1.0f, 1.0f, 1.0f)),
INIT_ENTITY_CVAR(SelectSoundPath, _T(""))
{
}


/*====================
  IPropEntity::IPropEntity
  ====================*/
IPropEntity::IPropEntity(CEntityConfig *pConfig) :
IVisualEntity(pConfig),
m_pEntityConfig(pConfig)
{
}


/*====================
  IPropEntity::~IPropEntity
  ====================*/
IPropEntity::~IPropEntity()
{
	if (m_uiWorldIndex != INVALID_INDEX && Game.WorldEntityExists(m_uiWorldIndex))
	{
		Game.UnlinkEntity(m_uiWorldIndex);
		Game.DeleteWorldEntity(m_uiWorldIndex);
	}
}


/*====================
  IPropEntity::GetTypeVector
  ====================*/
const vector<SDataField>&	IPropEntity::GetTypeVector()
{
	if (!s_pvFields)
	{
		s_pvFields = K2_NEW(global,   vector<SDataField>)();
		s_pvFields->clear();
		const vector<SDataField> &vBase(IVisualEntity::GetTypeVector());
		s_pvFields->insert(s_pvFields->begin(), vBase.begin(), vBase.end());

		s_pvFields->push_back(SDataField(_T("m_fScale"), FIELD_PUBLIC, TYPE_FLOAT));
		s_pvFields->push_back(SDataField(_T("m_hModel"), FIELD_PUBLIC, TYPE_RESHANDLE));
	}

	return *s_pvFields;
}


/*====================
  IPropEntity::Spawn
  ====================*/
void	IPropEntity::Spawn()
{
	IVisualEntity::Spawn();

	if (m_uiWorldIndex == INVALID_INDEX)
		m_uiWorldIndex = Game.AllocateNewWorldEntity();

	CModel* pModel(g_ResourceManager.GetModel(m_hModel));
	if (pModel != NULL &&
		pModel->GetModelFile()->GetType() == MODEL_K2)
	{
		CK2Model* pK2Model(static_cast<CK2Model*>(pModel->GetModelFile()));
		if (pK2Model->GetNumAnims() > 0)
		{
			if (Game.IsClient())
			{
				if (!m_pSkeleton)
					m_pSkeleton = K2_NEW(global,   CSkeleton);

				m_pSkeleton->SetModel(m_hModel);
			}

			m_ayAnim[0] = m_ayAnim[1] = m_yDefaultAnim;
		}
		else
		{
			SAFE_DELETE(m_pSkeleton);
		}
	}
	else
	{
		SAFE_DELETE(m_pSkeleton);
	}

	if (Game.IsClient())
	{
		g_ResourceManager.PrecacheSkin(m_hModel, 0);

		if (!GetEntityIconPath().empty())
			g_ResourceManager.Register(K2_NEW(global,   CTexture)(GetEntityIconPath(), TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);

		if (!GetCommanderPortraitPath().empty())
			g_ResourceManager.Register(K2_NEW(global,   CTexture)(GetCommanderPortraitPath(), TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);
	}

	if (!GetHitByMeleeEffectPath().empty())
		Game.RegisterEffect(GetHitByMeleeEffectPath());

	if (!GetHitByRangedEffectPath().empty())
		Game.RegisterEffect(GetHitByRangedEffectPath());

	Link();
}


/*====================
  IPropEntity:ApplyWorldEntity
  ====================*/
void	IPropEntity::ApplyWorldEntity(const CWorldEntity &ent)
{
	IVisualEntity::ApplyWorldEntity(ent);

	m_sSurfaceType = Game.GetPropType(ent.GetModelPath());
}


/*====================
  IPropEntity::Baseline
  ====================*/
void	IPropEntity::Baseline()
{
	IVisualEntity::Baseline();

	m_fScale = 1.0f;
	m_hModel = INVALID_RESOURCE;
}


/*====================
  IPropEntity::GetSnapshot
  ====================*/
void	IPropEntity::GetSnapshot(CEntitySnapshot &snapshot) const
{
	IVisualEntity::GetSnapshot(snapshot);
	
	snapshot.AddField(m_fScale);
	snapshot.AddResHandle(m_hModel);
}


/*====================
  IPropEntity::ReadSnapshot
  ====================*/
bool	IPropEntity::ReadSnapshot(CEntitySnapshot &snapshot)
{
	if (!IVisualEntity::ReadSnapshot(snapshot))
		return false;

	snapshot.ReadNextField(m_fScale);
	snapshot.ReadNextResHandle(m_hModel);

	return true;
}


/*====================
  IPropEntity::Kill
  ====================*/
void	IPropEntity::Kill(IVisualEntity *pAttacker, ushort unKillingObjectID)
{
	Console << _T("Killing entity #") << m_uiIndex << SPACE << ParenStr(m_sName) << newl;
	SetStatus(ENTITY_STATUS_DEAD);

	tstring sMethod(_T("Unknown"));
	if (unKillingObjectID != INVALID_ENT_TYPE)
	{
		ICvar *pCvar(EntityRegistry.GetGameSetting(unKillingObjectID, _T("Name")));

		if (pCvar != NULL)
			sMethod = pCvar->GetString();
	}

	Game.RegisterTriggerParam(_T("index"), XtoA(GetIndex()));
	Game.RegisterTriggerParam(_T("attackingindex"), XtoA(pAttacker != NULL ? pAttacker->GetIndex() : INVALID_INDEX));
	Game.RegisterTriggerParam(_T("method"), sMethod);
	Game.TriggerEntityScript(GetIndex(), _T("death"));
}


/*====================
  IPropEntity::Link
  ====================*/
void	IPropEntity::Link()
{
	if (m_uiWorldIndex != INVALID_INDEX)
	{
		CWorldEntity *pWorldEnt(Game.GetWorldEntity(m_uiWorldIndex));

		if (pWorldEnt != NULL)
		{
			pWorldEnt->SetPosition(GetPosition());
			pWorldEnt->SetScale(GetScale());
			pWorldEnt->SetScale2(GetScale2());
			pWorldEnt->SetAngles(GetAngles());
			pWorldEnt->SetBounds(GetBounds());
			pWorldEnt->SetModelHandle(GetModelHandle());
			pWorldEnt->SetGameIndex(GetIndex());

			Game.LinkEntity(m_uiWorldIndex, LINK_MODEL|LINK_SURFACE, SURF_PROP);

			const vector<CConvexPolyhedron> &cWorldSurfs(pWorldEnt->GetWorldSurfsRef());
			for (vector<CConvexPolyhedron>::const_iterator cit(cWorldSurfs.begin()); cit != cWorldSurfs.end(); ++cit)
				Game.BlockPath(NAVIGATION_CLIFF, *cit, p_stepHeight);
		}
	}
}


/*====================
  IPropEntity::Unlink
  ====================*/
void	IPropEntity::Unlink()
{
	if (m_uiWorldIndex != INVALID_INDEX && Game.WorldEntityExists(m_uiWorldIndex))
	{
		CWorldEntity *pWorldEnt(Game.GetWorldEntity(m_uiWorldIndex));

		if (pWorldEnt != NULL)
		{
#if 0 // props need their own nav map
			const vector<CConvexPolyhedron> &cWorldSurfs(pWorldEnt->GetWorldSurfsRef());
			for (vector<CConvexPolyhedron>::const_iterator cit(cWorldSurfs.begin()); cit != cWorldSurfs.end(); ++cit)
			{
				const CBBoxf &bbBounds(cit->GetBounds());

				Game.FreePath(NAVIGATION_CLIFF, bbBounds.GetMin().xy(), bbBounds.GetDim(X), bbBounds.GetDim(Y));
			}
#endif

			Game.UnlinkEntity(m_uiWorldIndex);
		}
	}
}


/*====================
  IPropEntity::Copy
  ====================*/
void	IPropEntity::Copy(const IGameEntity &B)
{
	IVisualEntity::Copy(B);

	const IPropEntity *pB(B.GetAsProp());

	if (!pB)	
		return;

	const IPropEntity &C(*pB);

	m_sSurfaceType = C.m_sSurfaceType;
}


/*====================
  IPropEntity::AllocateSkeleton
  ====================*/
CSkeleton*	IPropEntity::AllocateSkeleton()
{
	return m_pSkeleton;
}
	

/*====================
  IPropEntity::ClientPrecache
  ====================*/
void	IPropEntity::ClientPrecache(CEntityConfig *pConfig)
{
	IVisualEntity::ClientPrecache(pConfig);

	if (!pConfig)
		return;

	if (!pConfig->GetSelectSoundPath().empty())
		g_ResourceManager.Register(pConfig->GetSelectSoundPath(), RES_SAMPLE);
}


/*====================
  IPropEntity::GetApproachPosition
  ====================*/
CVec3f	IPropEntity::GetApproachPosition(const CVec3f &v3Start, const CBBoxf &bbBounds)
{
	CWorldEntity *pWorldEnt(Game.GetWorldEntity(m_uiWorldIndex));
		
	if (pWorldEnt != NULL)
	{
		CBBoxf bbBoundsWorld(bbBounds);
		bbBoundsWorld.Offset(v3Start);

		float fFraction(1.0f);

		const vector<CConvexPolyhedron> &cWorldSurfs(pWorldEnt->GetWorldSurfsRef());
		for (vector<CConvexPolyhedron>::const_iterator cit(cWorldSurfs.begin()); cit != cWorldSurfs.end(); ++cit)
			I_MovingBoundsSurfaceIntersect(v3Start, m_v3Position, bbBoundsWorld, *cit, fFraction);

		return LERP(fFraction, v3Start, m_v3Position);
	}
	else
		return m_v3Position;
}
