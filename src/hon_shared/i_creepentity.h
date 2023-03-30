// (C)2008 S2 Games
// i_creepentity.h
//
//=============================================================================
#ifndef __I_CREEPENTITY_H__
#define __I_CREEPENTITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_unitentity.h"
#include "c_creepdefinition.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
EXTERN_CVAR_UINT(creep_blockRepathTime);
EXTERN_CVAR_UINT(creep_blockRepathTimeExtra);
//=============================================================================

//=============================================================================
// ICreepEntity
//=============================================================================
class ICreepEntity : public IUnitEntity
{
    DECLARE_ENTITY_DESC

public:
    typedef CCreepDefinition TDefinition;

protected:
    uint    m_uiCharges;
    uint    m_uiControllerUID;

    CVec2f  m_v2Waypoint;

public:
    virtual ~ICreepEntity() {}
    ICreepEntity();

    SUB_ENTITY_ACCESSOR(ICreepEntity, Creep)

    virtual void        Baseline();
    virtual void        GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
    virtual bool        ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);
    virtual void        Copy(const IGameEntity &B);

    virtual bool        ServerFrameThink();

    virtual void        Spawn();
    virtual void        Die(IUnitEntity *pAttacker = NULL, ushort unKillingObject = INVALID_ENT_TYPE);

    virtual uint        GetInitialCharges() const       { return 0; }
    virtual byte        GetCharges() const              { return m_uiCharges; }
    virtual void        AddCharges(uint uiCharges)      { m_uiCharges += uiCharges; }
    virtual void        RemoveCharge()                  { if (m_uiCharges > 0) --m_uiCharges; }

    void                SetController(uint uiController)    { m_uiControllerUID = uiController; }
    uint                GetController() const               { return m_uiControllerUID; }
    
    virtual float       GetThreatLevel(IUnitEntity *pOther, bool bCurrentTarget);

    virtual uint        GetRepathTime() const           { return creep_blockRepathTime; }
    virtual uint        GetRepathTimeExtra() const      { return creep_blockRepathTimeExtra; }
};
//=============================================================================

#endif //__I_CREEPENTITY_H__
