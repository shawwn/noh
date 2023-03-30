// (C)2008 S2 Games
// c_gamedefinition.h
//
//=============================================================================
#ifndef __C_GAMEDEFINITION_H__
#define __C_GAMEDEFINITION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitydefinition.h"
#include "c_entityregistry.h"

#include "../k2/i_xmlprocessor.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
DECLARE_ENTITY_DEFINITION_XML_PROCESSOR(CGameInfo, GameInfo, game)
//=============================================================================

//=============================================================================
// CGameInfoDefinition
//=============================================================================
class CGameInfoDefinition : public IEntityDefinition
{
    DECLARE_DEFINITION_TYPE_INFO

    ENT_DEF_PROPERTY(StartingGold, uint)
    ENT_DEF_PROPERTY(RepickCost, uint)
    ENT_DEF_PROPERTY(RandomBonus, uint)
    ENT_DEF_PROPERTY(HeroPoolSize, uint)
    ENT_DEF_PROPERTY(BanCount, uint)
    ENT_DEF_PROPERTY(ExtraTime, uint)

    ENT_DEF_PROPERTY(AlternatePicks, bool)

    ENT_DEF_PROPERTY(GoldPerTick, uint)
    ENT_DEF_PROPERTY(IncomeInterval, uint)
    ENT_DEF_PROPERTY(ExperienceMultiplier, float)
    ENT_DEF_PROPERTY(TowerDenyGoldMultiplier, float)

    ENT_DEF_PROPERTY(NoLobby, bool)
    ENT_DEF_PROPERTY(NoHeroSelect, bool)
    ENT_DEF_PROPERTY(NoDev, bool)

protected:
    virtual void    PrecacheV(EPrecacheScheme eScheme, const tstring &sModifier)
    {
        IEntityDefinition::PrecacheV(eScheme, sModifier);

        PRECACHE_GUARD
            // ...
        PRECACHE_GUARD_END
    }

    virtual void    GetPrecacheListV(EPrecacheScheme eScheme, const tstring &sModifier, HeroPrecacheList &deqPrecache)
    {
        IEntityDefinition::GetPrecacheListV(eScheme, sModifier, deqPrecache);

        PRECACHE_GUARD
            // ...
        PRECACHE_GUARD_END
    }

public:
    ~CGameInfoDefinition()  {}
    CGameInfoDefinition() :
    IEntityDefinition(&g_allocatorGameInfo)
    {}

    IEntityDefinition*  GetCopy() const { return K2_NEW(ctx_Game,    CGameInfoDefinition)(*this); }

    virtual void    PostProcess()
    {
        if (m_bPostProcessing)
            return;

        IEntityDefinition::PostProcess();

        m_bPostProcessing = true;

        m_bPostProcessing = false;
    }

    void    ImportDefinition(IEntityDefinition *pOtherDefinition);
};
//=============================================================================

#endif //__C_GAMEDEFINITION_H__
