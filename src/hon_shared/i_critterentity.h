// (C)2008 S2 Games
// i_critterentity.h
//
//=============================================================================
#ifndef __I_CRITTERENTITY_H__
#define __I_CRITTERENTITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_unitentity.h"
#include "c_critterdefinition.h"
//=============================================================================

//=============================================================================
// ICritterEntity
//=============================================================================
class ICritterEntity : public IUnitEntity
{
    DECLARE_ENTITY_DESC

public:
    typedef CCritterDefinition TDefinition;
    
protected:
    CVec3f              m_v3SpawnPosition;

public:
    virtual ~ICritterEntity()   {}
    ICritterEntity();

    SUB_ENTITY_ACCESSOR(ICritterEntity, Critter)

    virtual void        Baseline();
    virtual void        GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
    virtual bool        ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);
    virtual void        Copy(const IGameEntity &B);

    virtual bool        ServerFrameThink();

    virtual void        Spawn();
    virtual void        Die(IUnitEntity *pAttacker = nullptr, ushort unKillingObjectID = INVALID_ENT_TYPE);
};
//=============================================================================

#endif //__I_CRITTERENTITY_H__
