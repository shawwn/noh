// (C)2008 S2 Games
// i_waypoint.h
//
//=============================================================================
#ifndef __I_WAYPOINT_H__
#define __I_WAYPOINT_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_visualentity.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
DECLARE_ENTITY_DEFINITION_XML_PROCESSOR(IWaypoint, Waypoint, waypoint)
//=============================================================================

//=============================================================================
// CWaypointDefinition
//=============================================================================
class CWaypointDefinition : public IEntityDefinition
{
    DECLARE_DEFINITION_TYPE_INFO

    ENT_DEF_RESOURCE_PROPERTY(Model, Model)
    ENT_DEF_PROPERTY(ModelScale, float)

protected:
    virtual void    PrecacheV(EPrecacheScheme eScheme, const tstring &sModifier)
    {
        IEntityDefinition::PrecacheV(eScheme, sModifier);

        PRECACHE_GUARD
            PrecacheModel();
        PRECACHE_GUARD_END
    }

public:
    ~CWaypointDefinition()  {}
    CWaypointDefinition() :
    IEntityDefinition(&g_allocatorWaypoint)
    {}

    IEntityDefinition*  GetCopy() const { return K2_NEW(ctx_Game,   CWaypointDefinition)(*this); }
};
//=============================================================================

//=============================================================================
// IWaypoint
//=============================================================================
class IWaypoint : public IVisualEntity
{
    DECLARE_ENTITY_DESC
    
    SUB_ENTITY_ACCESSOR(IWaypoint, Waypoint)

public:
    typedef CWaypointDefinition TDefinition;

protected:
    uint        m_uiOwnerIndex;
    uint        m_uiUnitIndex;
    
public:
    ~IWaypoint()    {}
    IWaypoint();
    
    virtual void        Baseline();
    virtual void        GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
    virtual bool        ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);

    virtual void        Copy(const IGameEntity &B);

    virtual CSkeleton*  AllocateSkeleton();
    virtual void        Spawn();
    virtual bool        ServerFrameCleanup();
    virtual bool        AddToScene(const CVec4f &v4Color, int iFlags);

    void                SetOwnerIndex(uint uiOwnerIndex)    { m_uiOwnerIndex = uiOwnerIndex; }
    uint                GetOwnerIndex() const               { return m_uiOwnerIndex; }

    void                SetUnitIndex(uint uiUnitIndex)      { m_uiUnitIndex = uiUnitIndex; }
    uint                GetUnitIndex() const                { return m_uiUnitIndex; }

    ENTITY_DEFINITION_RESOURCE_ACCESSOR(Model)
    ENTITY_DEFINITION_ACCESSOR(float, ModelScale)
};
//=============================================================================

#endif //__I_WAYPOINT_H__
