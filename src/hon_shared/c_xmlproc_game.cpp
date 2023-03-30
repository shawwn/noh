// (C)2008 S2 Games
// c_xmlproc_game.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_gamedefinition.h"
#include "c_gameinfo.h"
#include "c_entitydefinitionresource.h"

#include "../k2/c_xmlprocroot.h"
//=============================================================================

DEFINE_DEFINITION_TYPE_INFO(CGameInfoDefinition, ENTITY_BASE_TYPE_GAME_INFO, GameInfo)

START_ENTITY_DEFINITION_XML_PROCESSOR(CGameInfo, GameInfo)
    READ_ENTITY_DEFINITION_PROPERTY(StartingGold, startinggold)
    READ_ENTITY_DEFINITION_PROPERTY(RepickCost, repickcost)
    READ_ENTITY_DEFINITION_PROPERTY(RandomBonus, randombonus)
    READ_ENTITY_DEFINITION_PROPERTY(HeroPoolSize, heropoolsize)
    READ_ENTITY_DEFINITION_PROPERTY(BanCount, bancount)
    READ_ENTITY_DEFINITION_PROPERTY(ExtraTime, extratime)
    
    READ_ENTITY_DEFINITION_PROPERTY(AlternatePicks, alternatepicks)

    READ_ENTITY_DEFINITION_PROPERTY(GoldPerTick, goldpertick)
    READ_ENTITY_DEFINITION_PROPERTY(IncomeInterval, incomeinterval)
    READ_ENTITY_DEFINITION_PROPERTY(ExperienceMultiplier, experiencemultiplier)
    READ_ENTITY_DEFINITION_PROPERTY(TowerDenyGoldMultiplier, towerdenygoldmultiplier)

    READ_ENTITY_DEFINITION_PROPERTY(NoLobby, nolobby)
    READ_ENTITY_DEFINITION_PROPERTY(NoHeroSelect, noheroselect)
    READ_ENTITY_DEFINITION_PROPERTY(NoDev, nodev)
END_ENTITY_DEFINITION_XML_PROCESSOR(GameInfo, game)

ENTITY_DEF_MERGE_START(CGameInfoDefinition, IEntityDefinition)
    MERGE_PROPERTY(StartingGold)
    MERGE_PROPERTY(RepickCost)
    MERGE_PROPERTY(RandomBonus)
    MERGE_PROPERTY(HeroPoolSize)
    MERGE_PROPERTY(BanCount)
    MERGE_PROPERTY(ExtraTime)

    MERGE_PROPERTY(AlternatePicks)

    MERGE_PROPERTY(GoldPerTick)
    MERGE_PROPERTY(IncomeInterval)
    MERGE_PROPERTY(ExperienceMultiplier)
    MERGE_PROPERTY(TowerDenyGoldMultiplier)

    MERGE_PROPERTY(NoLobby)
    MERGE_PROPERTY(NoHeroSelect)
    MERGE_PROPERTY(NoDev)
ENTITY_DEF_MERGE_END
