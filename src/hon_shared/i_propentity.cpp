// (C)2006 S2 Games
// i_propentity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "hon_shared_common.h"

#include "i_propentity.h"

#include "../k2/c_model.h"
#include "../k2/c_k2model.h"
#include "../k2/c_worldentity.h"
#include "../k2/c_skeleton.h"
#include "../k2/c_world.h"
#include "../k2/c_texture.h"
#include "../k2/intersection.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENTITY_DESC(IPropEntity, 1)
{
    s_cDesc.pFieldTypes = K2_NEW(ctx_Game,   TypeVector)();
    s_cDesc.pFieldTypes->clear();
    const TypeVector &vBase(IVisualEntity::GetTypeVector());
    s_cDesc.pFieldTypes->insert(s_cDesc.pFieldTypes->begin(), vBase.begin(), vBase.end());

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_fScale"), TYPE_FLOAT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_hModel"), TYPE_RESHANDLE, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_hSkin"), TYPE_INT, 3, 0));
}
//=============================================================================

/*====================
  IPropEntity::CEntityConfig::CEntityConfig
  ====================*/
IPropEntity::CEntityConfig::CEntityConfig(const tstring &sName) :
IVisualEntity::CEntityConfig(sName),
INIT_ENTITY_CVAR(OcclusionRadius, 0.0f)
{
}


/*====================
  IPropEntity::IPropEntity
  ====================*/
IPropEntity::IPropEntity(CEntityConfig *pConfig) :
m_pEntityConfig(pConfig),
m_hModel(INVALID_RESOURCE),
m_hSkin(0)
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
  IPropEntity::Baseline
  ====================*/
void    IPropEntity::Baseline()
{
    IVisualEntity::Baseline();

    m_fScale = 1.0f;
    m_hModel = INVALID_RESOURCE;
    m_hSkin = 0;
}


/*====================
  IPropEntity::GetSnapshot
  ====================*/
void    IPropEntity::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
    IVisualEntity::GetSnapshot(snapshot, uiFlags);
    
    snapshot.WriteField(m_fScale);
    snapshot.WriteResHandle(m_hModel);
    snapshot.WriteField(m_hSkin);
}


/*====================
  IPropEntity::ReadSnapshot
  ====================*/
bool    IPropEntity::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
    if (!IVisualEntity::ReadSnapshot(snapshot, 1))
        return false;

    snapshot.ReadField(m_fScale);
    snapshot.ReadResHandle(m_hModel);
    snapshot.ReadField(m_hSkin);

    return true;
}


/*====================
  IPropEntity::Spawn
  ====================*/
void    IPropEntity::Spawn()
{
    IVisualEntity::Spawn();

    m_bbBounds = g_ResourceManager.GetModelSurfaceBounds(GetModel());
    m_bbBounds *= m_fScale;

    if (Game.IsServer())
        NetworkResourceManager.GetNetIndex(GetModel());

    if (m_uiWorldIndex == INVALID_INDEX)
        m_uiWorldIndex = Game.AllocateNewWorldEntity();

    CModel* pModel(g_ResourceManager.GetModel(GetModel()));
    if (pModel != nullptr &&
        pModel->GetModelFile()->GetType() == MODEL_K2)
    {
        CK2Model* pK2Model(static_cast<CK2Model*>(pModel->GetModelFile()));
        if (pK2Model->GetNumAnims() > 0)
        {
            if (Game.IsClient())
            {
                if (!m_pSkeleton)
                    m_pSkeleton = K2_NEW(ctx_Game,   CSkeleton);

                m_pSkeleton->SetModel(m_hModel);
            }
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

    Link();
}


/*====================
  IPropEntity:ApplyWorldEntity
  ====================*/
void    IPropEntity::ApplyWorldEntity(const CWorldEntity &ent)
{
    m_hModel = Game.RegisterModel(ent.GetModelPath());
    m_hSkin = g_ResourceManager.GetSkin(m_hModel, ent.GetSkinName());
    m_sSurfaceType = Game.GetPropType(ent.GetModelPath());

    IVisualEntity::ApplyWorldEntity(ent);
}


/*====================
  IPropEntity::Die
  ====================*/
void    IPropEntity::Die(IVisualEntity *pAttacker, ushort unKillingObjectID)
{
    SetStatus(ENTITY_STATUS_DEAD);

    tstring sMethod(_T("Unknown"));
    if (unKillingObjectID != INVALID_ENT_TYPE)
    {
        ICvar *pCvar(EntityRegistry.GetGameSetting(unKillingObjectID, _T("Name")));

        if (pCvar != nullptr)
            sMethod = pCvar->GetString();
    }
}


/*====================
  IPropEntity::Link
  ====================*/
void    IPropEntity::Link()
{
    if (m_uiWorldIndex != INVALID_INDEX)
    {
        CWorldEntity *pWorldEnt(Game.GetWorldEntity(m_uiWorldIndex));

        if (pWorldEnt != nullptr)
        {
            pWorldEnt->SetPosition(GetPosition());
            pWorldEnt->SetScale(GetBaseScale() * GetScale());
            pWorldEnt->SetScale2(1.0f); // Change this to GetScale() if we want bounds to resize with scale changes caused by states
            pWorldEnt->SetAngles(GetAngles());
            pWorldEnt->SetBounds(GetBounds());
            pWorldEnt->SetModelHandle(GetModel());
            pWorldEnt->SetGameIndex(GetIndex());
            pWorldEnt->SetOcclusionRadius(GetOcclusionRadius());

            if (pWorldEnt->HasFlags(WE_NOT_SOLID))
            {
                Game.LinkEntity(m_uiWorldIndex, LINK_MODEL | LINK_SURFACE, SURF_PROP | SURF_STATIC | SURF_NOT_SOLID);
            }
            else
            {
                Game.LinkEntity(m_uiWorldIndex, LINK_MODEL | LINK_SURFACE, SURF_PROP | SURF_STATIC);

                const vector<CConvexPolyhedron> &cWorldSurfs(pWorldEnt->GetWorldSurfsRef());
                for (vector<CConvexPolyhedron>::const_iterator cit(cWorldSurfs.begin()); cit != cWorldSurfs.end(); ++cit)
                    Game.BlockPath(m_vPathBlockers, NAVIGATION_BUILDING, *cit, 0.0f);
            }
        }
    }
}


/*====================
  IPropEntity::Unlink
  ====================*/
void    IPropEntity::Unlink()
{
    if (m_uiWorldIndex != INVALID_INDEX && Game.WorldEntityExists(m_uiWorldIndex))
    {
        CWorldEntity *pWorldEnt(Game.GetWorldEntity(m_uiWorldIndex));

        if (pWorldEnt != nullptr)
        {
            vector<PoolHandle>::const_iterator citEnd(m_vPathBlockers.end());
            for (vector<PoolHandle>::const_iterator cit(m_vPathBlockers.begin()); cit != citEnd; ++cit)
                Game.ClearPath(*cit);

            m_vPathBlockers.clear();

            Game.UnlinkEntity(m_uiWorldIndex);
        }
    }
}


/*====================
  IPropEntity::Copy
  ====================*/
void    IPropEntity::Copy(const IGameEntity &B)
{
    IVisualEntity::Copy(B);

    const IPropEntity *pB(B.GetAsProp());

    if (!pB)    
        return;

    const IPropEntity &C(*pB);

    m_hModel = C.m_hModel;
    m_hSkin = C.m_hSkin;
    //m_sSurfaceType = C.m_sSurfaceType;
}


/*====================
  IPropEntity::AllocateSkeleton
  ====================*/
CSkeleton*  IPropEntity::AllocateSkeleton()
{
    return m_pSkeleton;
}
    

/*====================
  IPropEntity::ClientPrecache
  ====================*/
void    IPropEntity::ClientPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier)
{
    IVisualEntity::ClientPrecache(pConfig, eScheme, sModifier);

    if (!pConfig)
        return;
}


/*====================
  IPropEntity::GetApproachPosition
  ====================*/
CVec3f  IPropEntity::GetApproachPosition(const CVec3f &v3Start, const CBBoxf &bbBounds)
{
    CWorldEntity *pWorldEnt(Game.GetWorldEntity(m_uiWorldIndex));
        
    if (pWorldEnt != nullptr)
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
