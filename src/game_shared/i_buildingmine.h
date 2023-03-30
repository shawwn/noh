// (C)2007 S2 Games
// i_buildingmine.h
//
//=============================================================================
#ifndef __I_BUILDINGMINE_H__
#define __I_BUILDINGMINE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_buildingentity.h"
//=============================================================================

//=============================================================================
// IBuildingMine
//=============================================================================
class IBuildingMine : public IBuildingEntity
{
private:
    IBuildingMine();

protected:
    CEntityConfig*  m_pEntityConfig;

public:
    virtual ~IBuildingMine()    {}
    IBuildingMine(CEntityConfig *pConfig) :
    IBuildingEntity(pConfig)
    {}

    uint            HarvestGold();
    uint            GetIncomeAmount() const;
    virtual void    UpkeepFailed(float fFraction)   {}

    virtual void    Kill(IVisualEntity *pAttacker = NULL, ushort unKillingObjectID = INVALID_ENT_TYPE);

    virtual bool    AddToScene(const CVec4f &v4Color, int iFlags);

    bool    IsMine() const  { return true; }
};
//=============================================================================

#endif //__I_BUILDINGMINE_H__
