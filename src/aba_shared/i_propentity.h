// (C)2006 S2 Games
// i_propentity.h
//
//=============================================================================
#ifndef __I_PROPENTITY_H__
#define __I_PROPENTITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_visualentity.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CWorldEntity;
class CEntitySnapshot;
//=============================================================================

//=============================================================================
// IPropEntity
//=============================================================================
class IPropEntity : public IVisualEntity
{
    DECLARE_ENTITY_DESC

protected:
    START_ENTITY_CONFIG(IVisualEntity)
        DECLARE_ENTITY_CVAR(float, OcclusionRadius)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    tstring     m_sSurfaceType;
    ResHandle   m_hModel;
    SkinHandle  m_hSkin;

    vector<PoolHandle>  m_vPathBlockers;

public:
    virtual ~IPropEntity();
    IPropEntity(CEntityConfig *pConfig);

    SUB_ENTITY_ACCESSOR(IPropEntity, Prop)

    virtual void            Baseline();
    virtual void            GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
    virtual bool            ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);
    virtual void            Copy(const IGameEntity &B);

    virtual void            Spawn();

    void                    SetModel(ResHandle hModel)              { m_hModel = hModel; }
    virtual ResHandle       GetModel() const                        { return m_hModel; }

    void                    SetSkin(SkinHandle hSkin)               { m_hSkin = hSkin; }
    virtual SkinHandle      GetSkin() const                         { return m_hSkin; }

    virtual CSkeleton*      AllocateSkeleton();

    virtual void            ApplyWorldEntity(const CWorldEntity &ent);
    virtual void            Die(IVisualEntity *pAttacker = NULL, ushort unKillingObjectID = INVALID_ENT_TYPE);

    virtual void            Link();
    virtual void            Unlink();

    // Settings
    ENTITY_CVAR_ACCESSOR(float, OcclusionRadius)

    static void             ClientPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme);

    void                    SetSurfaceType(const tstring &sSurfaceType) { m_sSurfaceType = sSurfaceType; }
    const tstring&          GetSurfaceType() const                      { return m_sSurfaceType; }

    virtual CVec3f          GetApproachPosition(const CVec3f &v3Start, const CBBoxf &bbBounds);
};
//=============================================================================

#endif //__I_PROPENTITY_H__
