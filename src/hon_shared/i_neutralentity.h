// (C)2008 S2 Games
// i_neutralentity.h
//
//=============================================================================
#ifndef __I_NEUTRALENTITY_H__
#define __I_NEUTRALENTITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_unitentity.h"
#include "c_neutraldefinition.h"
//=============================================================================

//=============================================================================
// INeutralEntity
//=============================================================================
class INeutralEntity : public IUnitEntity
{
    DECLARE_ENTITY_DESC

public:
    typedef CNeutralDefinition TDefinition;
    
protected:
    CVec3f              m_v3SpawnPosition;

    uint                m_uiSpawnControllerUID;

public:
    virtual ~INeutralEntity()   {}
    INeutralEntity();

    SUB_ENTITY_ACCESSOR(INeutralEntity, Neutral)

    virtual void        Baseline();
    virtual void        GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
    virtual bool        ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);
    virtual void        Copy(const IGameEntity &B);

    virtual bool        ServerFrameThink();

    virtual void        Spawn();
    virtual void        Die(IUnitEntity *pAttacker = nullptr, ushort unKillingObjectID = INVALID_ENT_TYPE);

    virtual void        OnTakeControl(IUnitEntity *pOwner);

    void                SetSpawnControllerUID(uint uiSpawnControllerUID)    { m_uiSpawnControllerUID = uiSpawnControllerUID; }
};
//=============================================================================

#endif //__I_NEUTRALENTITY_H__
