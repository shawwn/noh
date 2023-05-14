// (C)2006 S2 Games
// c_gameinterfacemanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_client_common.h"

#include "c_gameinterfacemanager.h"
#include "c_clientcommander.h"

#include "../hon_shared/c_replaymanager.h"
#include "../hon_shared/c_entitychest.h"
#include "../hon_shared/i_buildingentity.h"
#include "../hon_shared/i_gadgetentity.h"
#include "../hon_shared/i_heroentity.h"
#include "../hon_shared/i_entityability.h"
#include "../hon_shared/i_entityitem.h"
#include "../hon_shared/i_shopentity.h"
#include "../hon_shared/c_shopdefinition.h"
#include "../hon_shared/i_behavior.h"

#include "../k2/c_uimanager.h"
#include "../k2/c_function.h"
#include "../k2/c_vid.h"
#include "../k2/c_eventmanager.h"
#include "../k2/c_uicmd.h"
#include "../k2/c_interface.h"
#include "../k2/c_actionregistry.h"
#include "../k2/c_voicemanager.h"
#include "../k2/c_chatmanager.h"
#include "../k2/c_httpmanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
map<ResHandle, uint>    CGameInterfaceManager::s_mapInterfaceReferenceCount;

CVAR_UINTF  (cg_interfaceFPS,               20,     CVAR_SAVECONFIG);
CVAR_BOOLF  (cg_constrainCursor,            true,   CVAR_SAVECONFIG);
CVAR_BOOL   (ui_forceUpdate,                false);
CVAR_STRING (cg_forceInterface,             "");
CVAR_BOOL   (cg_shopShowNothing,            false);
CVAR_BOOLF  (cg_displayLevelup,             false,  CVAR_SAVECONFIG);
CVAR_BOOLF  (cg_displayAllies,              true,   CVAR_SAVECONFIG);
CVAR_BOOLF  (cg_24hourClock,                false,  CVAR_SAVECONFIG);
CVAR_UINTF  (cg_endGameInterfaceDelay,      21000,  CVAR_GAMECONFIG);
CVAR_STRING (cg_avatarHero, "");
CVAR_BOOLF  (cg_tooltipFlavor,              true,   CVAR_SAVECONFIG);

UI_TRIGGER(ReplayList);

int g_iSortValue(0);
uint g_uiHeroSortValue(-1);
bool g_bHeroSortByValue(false);
bool g_bHeroSortDesc(false);
//=============================================================================

/*====================
  CompareByStatValuesAsc
  ====================*/
template<class T>
static bool CompareByStatValuesAsc(const pair<int, T> elem1, const pair<int, T> elem2)
{
    return elem1.second < elem2.second;
}


/*====================
  CompareByStatValuesDesc
  ====================*/
template <class T>
static bool CompareByStatValuesDesc(const pair<int, T> elem1, const pair<int, T> elem2)
{
    return elem1.second > elem2.second;
}

/*====================
  CompareStatsAsc
  ====================*/
static bool CompareStatsAsc(const tsvector &elem1, const tsvector &elem2)
{
    int iCompareResult;

    if (int(elem1.size()) <= g_iSortValue || elem1[g_iSortValue].empty())
        return false;

    if (int(elem2.size()) <= g_iSortValue || elem2[g_iSortValue].empty())
        return true;

    iCompareResult = CompareNoCase(elem1[g_iSortValue], elem2[g_iSortValue]);

    return iCompareResult < 0;
}

/*====================
  CompareStatsDesc
  ====================*/
static bool CompareStatsDesc(const tsvector &elem1, const tsvector &elem2)
{
    int iCompareResult;

    if (int(elem1.size()) <= g_iSortValue || elem1[g_iSortValue].empty())
        return false;

    if (int(elem2.size()) <= g_iSortValue || elem2[g_iSortValue].empty())
        return true;

    iCompareResult = CompareNoCase(elem1[g_iSortValue], elem2[g_iSortValue]);

    return iCompareResult > 0;
}

/*====================
  CompareStatsValueAsc
  ====================*/
static bool CompareStatsValueAsc(const tsvector &elem1, const tsvector &elem2)
{
    if (int(elem1.size()) <= g_iSortValue || elem1[g_iSortValue].empty())
        return false;

    if (int(elem2.size()) <= g_iSortValue || elem2[g_iSortValue].empty())
        return true;

    return AtoF(elem1[g_iSortValue]) < AtoF(elem2[g_iSortValue]);
}

/*====================
  CompareStatsValueDesc
  ====================*/
static bool CompareStatsValueDesc(const tsvector &elem1, const tsvector &elem2)
{
    if (int(elem1.size()) <= g_iSortValue || elem1[g_iSortValue].empty())
        return false;

    if (int(elem2.size()) <= g_iSortValue || elem2[g_iSortValue].empty())
        return true;

    return AtoF(elem1[g_iSortValue]) > AtoF(elem2[g_iSortValue]);
}


/*====================
  CSmartGameUITrigger::~CSmartGameUITrigger
  ====================*/
CSmartGameUITrigger::~CSmartGameUITrigger()
{
    for (uint ui(0); ui < m_uiCount; ++ui)
    {
        if (m_vTriggerOwner[ui])
            SAFE_DELETE(m_vTriggers[ui]);
    }
}


/*====================
  CSmartGameUITrigger::CSmartGameUITrigger
  ====================*/
CSmartGameUITrigger::CSmartGameUITrigger(const tstring &sName, uint uiCount) :
m_uiCount(uiCount),
m_bDumb(false),
m_vTriggers(uiCount, nullptr),
m_vTriggerOwner(uiCount, false),
m_vbValue(uiCount, false),
m_vunValue(uiCount, 0),
m_vuiValue(uiCount, 0),
m_vfValue(uiCount, 0.0f),
m_vsValue(uiCount, TSNULL),
m_vbtValue(uiCount, CBuildText(INVALID_INDEX, 0)),
m_vmltValue(uiCount, CBuildMultiLevelText(INVALID_INDEX, 0, 0)),
m_vvValue(uiCount),
m_vUpdateSequence(uiCount, uint(-1))
{
    assert((m_uiCount) > 0);

    for (uint uiIndex(0); uiIndex < m_uiCount; ++uiIndex)
    {
        tstring sNameIndex(sName + ((uiCount > 1) ? XtoA(uiIndex) : TSNULL));
        m_vTriggers[uiIndex] = UITriggerRegistry.GetUITrigger(sNameIndex);
        if (m_vTriggers[uiIndex] == nullptr)
        {
            m_vTriggers[uiIndex] = K2_NEW(ctx_GameClient,   CUITrigger)(sNameIndex);
            m_vTriggerOwner[uiIndex] = true;
        }
    }
}


/*====================
  XtoA
  ====================*/
tstring     XtoA(const CBuildText &cText)
{
    tstring sOut;

    const tstring &sIn(GameClient.GetEntityString(cText.GetString()));
    uint uiIndex(cText.GetIndex());

    for (tstring::const_iterator it(sIn.begin()), itEnd(sIn.end()); it != itEnd; ++it)
    {
        if (*it == _T('%'))
            sOut.push_back(_T('%'));

        if (*it == _T('{'))
        {
            tstring sToken;

            if (it != itEnd)
                ++it;
            while (it != itEnd && *it != _T('}'))
            {
                sToken += *it;
                ++it;
            }

            if (!sToken.empty())
            {
                tsvector vTokens(TokenizeString(sToken, _T(',')));
                if (!vTokens.empty())
                    sOut += vTokens[MIN<uint>(uiIndex, uint(vTokens.size()) - 1)];
            }

            continue;
        }

        sOut.push_back(*it);
    }

    return sOut;
}


/*====================
  XtoA
  ====================*/
tstring     XtoA(const CBuildMultiLevelText &cText)
{
    tstring sOut;

    const tstring &sIn(GameClient.GetEntityString(cText.GetString()));
    uint uiMarkIndex(cText.GetMarkIndex());
    uint uiMaxIndex(cText.GetMaxIndex());

    for (tstring::const_iterator it(sIn.begin()), itEnd(sIn.end()); it != itEnd; ++it)
    {
        if (*it == _T('{'))
        {
            tstring sToken;

            if (it != itEnd)
                ++it;
            while (it != itEnd && *it != _T('}'))
            {
                sToken += *it;
                ++it;
            }

            if (!sToken.empty())
            {
                tsvector vTokens(TokenizeString(sToken, _T(',')));

                if (vTokens.size() == 1)
                {
                    sOut += vTokens[0];
                }
                else
                {
                    if (uiMarkIndex >= 1)
                        sOut += _T("^v") + vTokens[0] + _T("^*");
                    else
                        sOut += vTokens[0];

                    for (uint uiIndex(1); uiIndex <= uiMaxIndex; ++uiIndex)
                    {
                        sOut += _T('/');

                        if (uiMarkIndex >= uiIndex + 1)
                            sOut += _T("^v") + vTokens[MIN<uint>(uiIndex, uint(vTokens.size()) - 1)] + _T("^*");
                        else
                            sOut += vTokens[MIN<uint>(uiIndex, uint(vTokens.size()) - 1)];
                    }
                }
            }

            continue;
        }

        sOut.push_back(*it);
    }

    return sOut;
}


/*====================
  CSmartGameUITrigger::Trigger
  ====================*/
#define TRIGGERFN(type, pre) \
void    CSmartGameUITrigger::Trigger(type pre##Value, uint uiIndex, uint uiUpdateSequence) \
{ \
    if (uiIndex >= m_vTriggers.size() || m_vTriggers[uiIndex] == nullptr) \
        return; \
\
    if (!m_bDumb && m_v##pre##Value[uiIndex] == pre##Value && m_vUpdateSequence[uiIndex] == uiUpdateSequence) \
        return; \
\
    m_v##pre##Value[uiIndex] = pre##Value; \
    m_vTriggers[uiIndex]->Trigger(XtoA(pre##Value)); \
    m_vUpdateSequence[uiIndex] = uiUpdateSequence; \
}

TRIGGERFN(bool, b)
TRIGGERFN(short, un)
TRIGGERFN(ushort, un)
TRIGGERFN(int, ui)
TRIGGERFN(uint, ui)
TRIGGERFN(float, f)
TRIGGERFN(const tstring&, s)
TRIGGERFN(const CBuildText&, bt)
TRIGGERFN(const CBuildMultiLevelText&, mlt)
#undef TRIGGERFN

void    CSmartGameUITrigger::Trigger(const tsvector& vValue, uint uiIndex, uint uiUpdateSequence)
{
    if (uiIndex >= m_vTriggers.size() || m_vTriggers[uiIndex] == nullptr)
        return;

    if (!m_bDumb && m_vvValue[uiIndex] == vValue && m_vUpdateSequence[uiIndex] == uiUpdateSequence)
        return;

    m_vvValue[uiIndex] = vValue;
    m_vTriggers[uiIndex]->Trigger(vValue);
    m_vUpdateSequence[uiIndex] = uiUpdateSequence;
}


/*====================
  CSmartGameUITrigger::Execute
  ====================*/
void    CSmartGameUITrigger::Execute(const tstring &sScript, uint uiIndex)
{
    if (uiIndex >= m_vTriggers.size() || m_vTriggers[uiIndex] == nullptr)
        return;

    m_vTriggers[uiIndex]->Execute(sScript);
}



/*====================
  CGameInterfaceManager::~CGameInterfaceManager
  ====================*/
CGameInterfaceManager::~CGameInterfaceManager()
{
    Host.GetHTTPManager()->ReleaseRequest(m_pStatsRequest);
    Host.GetHTTPManager()->ReleaseRequest(m_pMatchInfoRequest);
    Host.GetHTTPManager()->ReleaseRequest(m_pTournamentRequest);
    Host.GetHTTPManager()->ReleaseRequest(m_pRecentMatchesRequest);

    UIManager.ClearOverlayInterfaces();

    for (int i(0); i < NUM_UITRIGGERS; ++i)
        SAFE_DELETE(m_vTriggers[i]);

    for (map<ResHandle, uint>::iterator it(s_mapInterfaceReferenceCount.begin()); it != s_mapInterfaceReferenceCount.end(); ) // NOLINT(modernize-use-auto)
    {
        if (it->second == 0)
        {
            STL_ERASE(s_mapInterfaceReferenceCount, it);
            continue;
        }

        --(it->second);
        if (it->second == 0)
        {
            //UIManager.UnloadInterface(it->first);
            STL_ERASE(s_mapInterfaceReferenceCount, it);
            continue;
        }

        ++it;
    }

    Input.SetCursorRecenter(CURSOR_GAME, BOOL_NOT_SET);
    Input.SetCursorConstrained(CURSOR_GAME, BOOL_NOT_SET);
    Input.SetCursorHidden(CURSOR_GAME, BOOL_NOT_SET);
    Input.SetCursorFrozen(CURSOR_GAME, BOOL_NOT_SET);
}


/*====================
  CGameInterfaceManager::CGameInterfaceManager
  ====================*/
CGameInterfaceManager::CGameInterfaceManager() :
m_bCursorHidden(false),
m_bDisplayShop(false),
m_bLockShop(false),
m_vTriggers(NUM_UITRIGGERS, nullptr),
m_sMainInterface(_T("main")),
m_uiUpdateSequence(0),
m_uiLastUpdateSequence(0),
m_eCurrentInterface(CG_INTERFACE_INVALID),
m_uiLastBuildingAttackAlertTime(0),
m_bIsLoggedIn(false),
m_uiLastUpdateTime(0),
m_bEntitiesLoaded(false),
m_uiLoadPos(0),
m_iReplayURLTesting(-1),

m_pStatsRequest(nullptr),
m_pMatchInfoRequest(nullptr),
m_pTournamentRequest(nullptr),
m_pRecentMatchesRequest(nullptr),
m_uiPrevPhase(GAME_PHASE_INVALID),
m_uiPrevPhaseTime(INVALID_TIME),
m_uiScoreState(0),
m_uiQueuedMatchInfoRequest(INVALID_INDEX)
{
    AddTrigger(UITRIGGER_HOST_TIME, _T("HostTime"));
    AddTrigger(UITRIGGER_CAN_LEAVE, _T("CanLeave"));
    AddTrigger(UITRIGGER_GAME_PHASE, _T("GamePhase"));
    AddTrigger(UITRIGGER_TMM_GAME_PHASE, _T("TMMGamePhase"));
    AddTrigger(UITRIGGER_LAG, _T("Lag"));
    AddTrigger(UITRIGGER_STATS_STATUS, _T("StatsStatus"));

    AddTrigger(UITRIGGER_MAIN_LOGIN_STATUS, _T("LoginStatus"));
    AddTrigger(UITRIGGER_MAIN_UPDATER_STATUS, _T("UpdaterStatus"));
    AddTrigger(UITRIGGER_MAIN_CHANGE_PASSWORD_STATUS, _T("ChangePasswordStatus"));
    AddTrigger(UITRIGGER_MAIN_ACCOUNT_INFO, _T("AccountInfo"));
    AddTrigger(UITRIGGER_MAIN_PLAYER_STATS, _T("PlayerStats"));
    AddTrigger(UITRIGGER_MAIN_LOCAL_PLAYER_STATS, _T("LocalPlayerStats"));
    AddTrigger(UITRIGGER_MAIN_GAMELIST_STATUS, _T("GameListStatus"));
    AddTrigger(UITRIGGER_MAIN_LOCAL_SERVER_AVAILABLE, _T("LocalServerAvailable"));
    AddTrigger(UITRIGGER_MAIN_DEV, _T("Dev"));
    AddTrigger(UITRIGGER_MAIN_PLAYERS_ONLINE, _T("PlayersOnline"));
    AddTrigger(UITRIGGER_MAIN_PLAYERS_INGAME, _T("PlayersInGame"));
    AddTrigger(UITRIGGER_MAIN_COUNTDOWN, _T("Countdown"));

    AddTrigger(UITRIGGER_FOLLOW_STATUS, _T("ChatFollowStatus"));

    AddTrigger(UITRIGGER_PLAYER_INFO, _T("PlayerInfo"));
    AddTrigger(UITRIGGER_PLAYER_GOLD, _T("PlayerGold"));
    AddTrigger(UITRIGGER_PLAYER_CAN_SHOP, _T("PlayerCanShop"));
    AddTrigger(UITRIGGER_PLAYER_SCORE, _T("PlayerScore"));

    AddTrigger(UITRIGGER_SHOP_ACTIVE, _T("ShopActive"));
    AddTrigger(UITRIGGER_SHOP_TYPE, _T("ShopType"));
    AddTrigger(UITRIGGER_SHOP_HEADER, _T("ShopHeader"));
    AddTrigger(UITRIGGER_SHOP_NAME, _T("ShopName"));
    AddTrigger(UITRIGGER_SHOP_DESCRIPTION, _T("ShopDescription"));
    AddTrigger(UITRIGGER_SHOP_ICON, _T("ShopIcon"));
    AddTrigger(UITRIGGER_SHOP_KEY, _T("ShopKey"), MAX_SHOPS);
    AddTrigger(UITRIGGER_SHOP_ITEM, _T("ShopItem"), MAX_SHOP_ITEMS);
    AddTrigger(UITRIGGER_SHOP_ITEM_TOOLTIP, _T("ShopItemTooltip"), MAX_SHOP_ITEMS);
    AddTrigger(UITRIGGER_SHOP_ITEM_TYPE, _T("ShopItemType"), MAX_SHOP_ITEMS);

    AddTrigger(UITRIGGER_RECIPE_ITEM, _T("RecipeItem"));
    AddTrigger(UITRIGGER_RECIPE_ITEM_TYPE, _T("RecipeItemType"));
    AddTrigger(UITRIGGER_RECIPE_COMPONENT, _T("RecipeComponent"), MAX_RECIPE_COMPONENTS);
    AddTrigger(UITRIGGER_RECIPE_COMPONENT_0_SUB_COMPONENT, _T("RecipeComponent0SubComponent"), MAX_RECIPE_COMPONENTS);
    AddTrigger(UITRIGGER_RECIPE_COMPONENT_1_SUB_COMPONENT, _T("RecipeComponent1SubComponent"), MAX_RECIPE_COMPONENTS);
    AddTrigger(UITRIGGER_RECIPE_COMPONENT_2_SUB_COMPONENT, _T("RecipeComponent2SubComponent"), MAX_RECIPE_COMPONENTS);
    AddTrigger(UITRIGGER_RECIPE_COMPONENT_3_SUB_COMPONENT, _T("RecipeComponent3SubComponent"), MAX_RECIPE_COMPONENTS);
    AddTrigger(UITRIGGER_RECIPE_USEDIN, _T("RecipeUsedIn"), MAX_RECIPE_USEDIN);

    AddTrigger(UITRIGGER_RECIPE_ITEM_TOOLTIP, _T("RecipeItemTooltip"));
    AddTrigger(UITRIGGER_RECIPE_COMPONENT_TOOLTIP, _T("RecipeComponentTooltip"), MAX_RECIPE_COMPONENTS);
    AddTrigger(UITRIGGER_RECIPE_COMPONENT_0_SUB_TOOLTIP, _T("RecipeComponent0SubTooltip"), MAX_RECIPE_COMPONENTS);
    AddTrigger(UITRIGGER_RECIPE_COMPONENT_1_SUB_TOOLTIP, _T("RecipeComponent1SubTooltip"), MAX_RECIPE_COMPONENTS);
    AddTrigger(UITRIGGER_RECIPE_COMPONENT_2_SUB_TOOLTIP, _T("RecipeComponent2SubTooltip"), MAX_RECIPE_COMPONENTS);
    AddTrigger(UITRIGGER_RECIPE_COMPONENT_3_SUB_TOOLTIP, _T("RecipeComponent3SubTooltip"), MAX_RECIPE_COMPONENTS);
    AddTrigger(UITRIGGER_RECIPE_USEDIN_TOOLTIP, _T("RecipeUsedInTooltip"), MAX_RECIPE_USEDIN);

    AddTrigger(UITRIGGER_RECIPE_HAS_BACK_HISTORY, _T("RecipeHasBackHistory"));
    AddTrigger(UITRIGGER_RECIPE_HAS_FORWARD_HISTORY, _T("RecipeHasForwardHistory"));

    AddTrigger(UITRIGGER_HERO_INDEX, _T("HeroIndex"));
    AddTrigger(UITRIGGER_HERO_NAME, _T("HeroName"));
    AddTrigger(UITRIGGER_HERO_ICON, _T("HeroIcon"));
    AddTrigger(UITRIGGER_HERO_PORTRAIT, _T("HeroPortrait"));
    AddTrigger(UITRIGGER_HERO_LEVEL, _T("HeroLevel"));
    AddTrigger(UITRIGGER_HERO_EXPERIENCE, _T("HeroExperience"));
    AddTrigger(UITRIGGER_HERO_HEALTH, _T("HeroHealth"));
    AddTrigger(UITRIGGER_HERO_MANA, _T("HeroMana"));
    AddTrigger(UITRIGGER_HERO_HEALTHREGEN, _T("HeroHealthRegen"));
    AddTrigger(UITRIGGER_HERO_MANAREGEN, _T("HeroManaRegen"));
    AddTrigger(UITRIGGER_HERO_STATUS, _T("HeroStatus"));
    AddTrigger(UITRIGGER_HERO_RESPAWN, _T("HeroRespawn"));
    AddTrigger(UITRIGGER_HERO_BUYBACK, _T("HeroBuyBack"));
    AddTrigger(UITRIGGER_HERO_BUYBACK_COST, _T("HeroBuyBackCost"));

    AddTrigger(UITRIGGER_HERO_INVENTORY_EXISTS, _T("HeroInventoryExists"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_HERO_INVENTORY_ICON, _T("HeroInventoryIcon"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_HERO_INVENTORY_CAN_ACTIVATE, _T("HeroInventoryCanActivate"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_HERO_INVENTORY_STATUS, _T("HeroInventoryStatus"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_HERO_INVENTORY_COOLDOWN, _T("HeroInventoryCooldown"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_HERO_INVENTORY_CHARGES, _T("HeroInventoryCharges"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_HERO_INVENTORY_DESCRIPTION, _T("HeroInventoryDescription"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_HERO_INVENTORY_HOTKEYS, _T("HeroInventoryHotkeys"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_HERO_INVENTORY_AURA, _T("HeroInventoryAura"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_HERO_INVENTORY_STATUS_EFFECT, _T("HeroInventoryStatusEffect"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_HERO_INVENTORY_STATUS_EFFECTB, _T("HeroInventoryStatusEffectB"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_HERO_INVENTORY_TRIGGERED_EFFECT, _T("HeroInventoryTriggeredEffect"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_HERO_INVENTORY_INTERFACE, _T("HeroInventoryInterface"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_HERO_INVENTORY_DURATION, _T("HeroInventoryDuration"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_HERO_INVENTORY_PASSIVE_EFFECT, _T("HeroInventoryPassiveEffect"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_HERO_INVENTORY_HAS_TIMER, _T("HeroInventoryHasTimer"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_HERO_INVENTORY_TIMER, _T("HeroInventoryTimer"), MAX_INVENTORY);

    AddTrigger(UITRIGGER_ALLY_DISPLAY, _T("AllyDisplay"));
    AddTrigger(UITRIGGER_ALLY_EXISTS, _T("AllyExists"), MAX_ALLY_HEROES);
    AddTrigger(UITRIGGER_ALLY_INDEX, _T("AllyIndex"), MAX_ALLY_HEROES);
    AddTrigger(UITRIGGER_ALLY_PLAYER_INFO, _T("AllyPlayerInfo"), MAX_ALLY_HEROES);
    AddTrigger(UITRIGGER_ALLY_HERO_INFO, _T("AllyHeroInfo"), MAX_ALLY_HEROES);
    AddTrigger(UITRIGGER_ALLY_HEALTH, _T("AllyHealth"), MAX_ALLY_HEROES);
    AddTrigger(UITRIGGER_ALLY_MANA, _T("AllyMana"), MAX_ALLY_HEROES);
    AddTrigger(UITRIGGER_ALLY_STATUS, _T("AllyStatus"), MAX_ALLY_HEROES);
    AddTrigger(UITRIGGER_ALLY_RESPAWN, _T("AllyRespawn"), MAX_ALLY_HEROES);
    AddTrigger(UITRIGGER_ALLY_DAMAGE, _T("AllyDamage"), MAX_ALLY_HEROES);
    AddTrigger(UITRIGGER_ALLY_ARMOR, _T("AllyArmor"), MAX_ALLY_HEROES);
    AddTrigger(UITRIGGER_ALLY_MAGIC_ARMOR, _T("AllyMagicArmor"), MAX_ALLY_HEROES);
    AddTrigger(UITRIGGER_ALLY_MOVE_SPEED, _T("AllyMoveSpeed"), MAX_ALLY_HEROES);
    AddTrigger(UITRIGGER_ALLY_ATTACK_SPEED, _T("AllyAttackSpeed"), MAX_ALLY_HEROES);
    AddTrigger(UITRIGGER_ALLY_STRENGTH, _T("AllyStrength"), MAX_ALLY_HEROES);
    AddTrigger(UITRIGGER_ALLY_AGILITY, _T("AllyAgility"), MAX_ALLY_HEROES);
    AddTrigger(UITRIGGER_ALLY_INTELLIGENCE, _T("AllyIntelligence"), MAX_ALLY_HEROES);
    AddTrigger(UITRIGGER_ALLY_GOLD, _T("AllyGold"), MAX_ALLY_HEROES);
    AddTrigger(UITRIGGER_ALLY_DISCONNECTED, _T("AllyDisconnected"), MAX_ALLY_HEROES);
    AddTrigger(UITRIGGER_ALLY_AFK, _T("AllyAFK"), MAX_ALLY_HEROES);
    AddTrigger(UITRIGGER_ALLY_DISCONNECT_TIME, _T("AllyDisconnectTime"), MAX_ALLY_HEROES);
    AddTrigger(UITRIGGER_ALLY_LOADING_PERCENT, _T("AllyLoadingPercent"), MAX_ALLY_HEROES);
    AddTrigger(UITRIGGER_ALLY_CONTROL_SHARING, _T("AllyControlSharing"), MAX_ALLY_HEROES);
    AddTrigger(UITRIGGER_ALLY_NO_HELP, _T("AllyNoHelp"), MAX_ALLY_HEROES);
    AddTrigger(UITRIGGER_ALLY_VOICE, _T("AllyVoice"), MAX_ALLY_HEROES);

    AddTrigger(UITRIGGER_ALLY_ABILITY_0_INFO, _T("AllyAbility0Info"), MAX_ALLY_HEROES);
    AddTrigger(UITRIGGER_ALLY_ABILITY_0_COOLDOWN, _T("AllyAbility0Cooldown"), MAX_ALLY_HEROES);

    AddTrigger(UITRIGGER_ALLY_ABILITY_1_INFO, _T("AllyAbility1Info"), MAX_ALLY_HEROES);
    AddTrigger(UITRIGGER_ALLY_ABILITY_1_COOLDOWN, _T("AllyAbility1Cooldown"), MAX_ALLY_HEROES);

    AddTrigger(UITRIGGER_ALLY_ABILITY_2_INFO, _T("AllyAbility2Info"), MAX_ALLY_HEROES);
    AddTrigger(UITRIGGER_ALLY_ABILITY_2_COOLDOWN, _T("AllyAbility2Cooldown"), MAX_ALLY_HEROES);

    AddTrigger(UITRIGGER_ALLY_ABILITY_3_INFO, _T("AllyAbility3Info"), MAX_ALLY_HEROES);
    AddTrigger(UITRIGGER_ALLY_ABILITY_3_COOLDOWN, _T("AllyAbility3Cooldown"), MAX_ALLY_HEROES);

    AddTrigger(UITRIGGER_ALLY_ABILITY_4_INFO, _T("AllyAbility4Info"), MAX_ALLY_HEROES);
    AddTrigger(UITRIGGER_ALLY_ABILITY_4_COOLDOWN, _T("AllyAbility4Cooldown"), MAX_ALLY_HEROES);

    AddTrigger(UITRIGGER_ACTIVE_INDEX, _T("ActiveIndex"));
    AddTrigger(UITRIGGER_ACTIVE_NAME, _T("ActiveName"));
    AddTrigger(UITRIGGER_ACTIVE_ICON, _T("ActiveIcon"));
    AddTrigger(UITRIGGER_ACTIVE_PORTRAIT, _T("ActivePortrait"));
    AddTrigger(UITRIGGER_ACTIVE_MODEL, _T("ActiveModel"));
    AddTrigger(UITRIGGER_ACTIVE_EFFECT, _T("ActiveEffect"));
    AddTrigger(UITRIGGER_ACTIVE_STATUS, _T("ActiveStatus"));
    AddTrigger(UITRIGGER_ACTIVE_LEVEL, _T("ActiveLevel"));
    AddTrigger(UITRIGGER_ACTIVE_EXPERIENCE, _T("ActiveExperience"));
    AddTrigger(UITRIGGER_ACTIVE_DAMAGE, _T("ActiveDamage"));
    AddTrigger(UITRIGGER_ACTIVE_ARMOR, _T("ActiveArmor"));
    AddTrigger(UITRIGGER_ACTIVE_MAGIC_ARMOR, _T("ActiveMagicArmor"));
    AddTrigger(UITRIGGER_ACTIVE_MOVE_SPEED, _T("ActiveMoveSpeed"));
    AddTrigger(UITRIGGER_ACTIVE_ATTACK_SPEED, _T("ActiveAttackSpeed"));
    AddTrigger(UITRIGGER_ACTIVE_CAST_SPEED, _T("ActiveCastSpeed"));
    AddTrigger(UITRIGGER_ACTIVE_ATTACK_RANGE, _T("ActiveAttackRange"));
    AddTrigger(UITRIGGER_ACTIVE_ATTACK_COOLDOWN, _T("ActiveAttackCooldown"));
    AddTrigger(UITRIGGER_ACTIVE_STRENGTH, _T("ActiveStrength"));
    AddTrigger(UITRIGGER_ACTIVE_AGILITY, _T("ActiveAgility"));
    AddTrigger(UITRIGGER_ACTIVE_INTELLIGENCE, _T("ActiveIntelligence"));
    AddTrigger(UITRIGGER_ACTIVE_ATTRIBUTES, _T("ActiveAttributes"));
    AddTrigger(UITRIGGER_ACTIVE_HEALTH, _T("ActiveHealth"));
    AddTrigger(UITRIGGER_ACTIVE_MANA, _T("ActiveMana"));
    AddTrigger(UITRIGGER_ACTIVE_HEALTHREGEN, _T("ActiveHealthRegen"));
    AddTrigger(UITRIGGER_ACTIVE_MANAREGEN, _T("ActiveManaRegen"));
    AddTrigger(UITRIGGER_ACTIVE_LIFETIME, _T("ActiveLifetime"));
    AddTrigger(UITRIGGER_ACTIVE_HAS_INVENTORY, _T("ActiveHasInventory"));
    AddTrigger(UITRIGGER_ACTIVE_HAS_ATTRIBUTES, _T("ActiveHasAttributes"));
    AddTrigger(UITRIGGER_ACTIVE_PLAYER_INFO, _T("ActivePlayerInfo"));
    AddTrigger(UITRIGGER_ACTIVE_ILLUSION, _T("ActiveIllusion"));

    AddTrigger(UITRIGGER_ACTIVE_ATTACK_INFO, _T("ActiveAttackInfo"));
    AddTrigger(UITRIGGER_ACTIVE_DEFENSE_INFO, _T("ActiveDefenseInfo"));
    AddTrigger(UITRIGGER_ACTIVE_ATTRIBUTE_INFO, _T("ActiveAttributeInfo"));

    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_EXISTS, _T("ActiveInventoryExists"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_ICON, _T("ActiveInventoryIcon"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_RECIPE, _T("ActiveInventoryRecipe"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_CAN_ACTIVATE, _T("ActiveInventoryCanActivate"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_STATUS, _T("ActiveInventoryStatus"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_STATE, _T("ActiveInventoryState"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_COOLDOWN, _T("ActiveInventoryCooldown"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_CHARGES, _T("ActiveInventoryCharges"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_DESCRIPTION, _T("ActiveInventoryDescription"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_HOTKEYS, _T("ActiveInventoryHotkeys"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_AURA, _T("ActiveInventoryAura"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_STATUS_EFFECT, _T("ActiveInventoryStatusEffect"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_STATUS_EFFECTB, _T("ActiveInventoryStatusEffectB"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_TRIGGERED_EFFECT, _T("ActiveInventoryTriggeredEffect"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_TRIGGERED_EFFECT_DESCRIPTION, _T("ActiveInventoryTriggeredEffectDescription"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_INTERFACE, _T("ActiveInventoryInterface"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_DURATION, _T("ActiveInventoryDuration"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_DURATION_PERCENT, _T("ActiveInventoryDurationPercent"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_PASSIVE_EFFECT, _T("ActiveInventoryPassiveEffect"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_DESCRIPTION_A, _T("ActiveInventoryDescriptionA"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_DESCRIPTION_B, _T("ActiveInventoryDescriptionB"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_EFFECT_DESCRIPTION, _T("ActiveInventoryEffectDescription"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_HAS_TIMER, _T("ActiveInventoryHasTimer"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_TIMER, _T("ActiveInventoryTimer"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_ACTIVATE_COST, _T("ActiveInventoryActivateCost"), MAX_INVENTORY);

    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_STATUS, _T("ActiveInventoryMultiLevelStatus"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_DESCRIPTION, _T("ActiveInventoryMultiLevelDescription"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_CAN_ACTIVATE, _T("ActiveInventoryMultiLevelCanActivate"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_COOLDOWN, _T("ActiveInventoryMultiLevelCooldown"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_STATUS_EFFECT, _T("ActiveInventoryMultiLevelStatusEffect"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_STATUS_EFFECTB, _T("ActiveInventoryMultiLevelStatusEffectB"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_AURA, _T("ActiveInventoryMultiLevelAura"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_TRIGGERED_EFFECT, _T("ActiveInventoryMultiLevelTriggeredEffect"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_TRIGGERED_EFFECT_DESCRIPTION, _T("ActiveInventoryMultiLevelTriggeredEffectDescription"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_DESCRIPTION_A, _T("ActiveInventoryMultiLevelDescriptionA"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_DESCRIPTION_B, _T("ActiveInventoryMultiLevelDescriptionB"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_EFFECT_DESCRIPTION, _T("ActiveInventoryMultiLevelEffectDescription"), MAX_INVENTORY);

    AddTrigger(UITRIGGER_ATTACK_MODIFIERS, _T("AttackModifier"), MAX_ATTACK_MODIFIERS);

    AddTrigger(UITRIGGER_STASH_EXISTS, _T("StashExists"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_STASH_ICON, _T("StashIcon"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_STASH_RECIPE, _T("StashRecipe"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_STASH_CAN_ACTIVATE, _T("StashCanActivate"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_STASH_STATUS, _T("StashStatus"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_STASH_COOLDOWN, _T("StashCooldown"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_STASH_CHARGES, _T("StashCharges"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_STASH_DESCRIPTION, _T("StashDescription"), MAX_INVENTORY);

    AddTrigger(UITRIGGER_SELECTED_VISIBLE, _T("SelectedVisible"), MAX_SELECTED_UNITS);
    AddTrigger(UITRIGGER_SELECTED_INDEX, _T("SelectedIndex"), MAX_SELECTED_UNITS);
    AddTrigger(UITRIGGER_SELECTED_NAME, _T("SelectedName"), MAX_SELECTED_UNITS);
    AddTrigger(UITRIGGER_SELECTED_ICON, _T("SelectedIcon"), MAX_SELECTED_UNITS);
    AddTrigger(UITRIGGER_SELECTED_COLOR, _T("SelectedColor"), MAX_SELECTED_UNITS);
    AddTrigger(UITRIGGER_SELECTED_ACTIVE, _T("SelectedActive"), MAX_SELECTED_UNITS);
    AddTrigger(UITRIGGER_SELECTED_LEVEL, _T("SelectedLevel"), MAX_SELECTED_UNITS);
    AddTrigger(UITRIGGER_SELECTED_TYPE, _T("SelectedType"), MAX_SELECTED_UNITS);
    AddTrigger(UITRIGGER_SELECTED_EXPERIENCE, _T("SelectedExperience"));
    AddTrigger(UITRIGGER_SELECTED_DAMAGE, _T("SelectedDamage"));
    AddTrigger(UITRIGGER_SELECTED_ARMOR, _T("SelectedArmor"));
    AddTrigger(UITRIGGER_SELECTED_MAGIC_ARMOR, _T("SelectedMagicArmor"));
    AddTrigger(UITRIGGER_SELECTED_MOVE_SPEED, _T("SelectedMoveSpeed"));
    AddTrigger(UITRIGGER_SELECTED_ATTACK_SPEED, _T("SelectedAttackSpeed"));
    AddTrigger(UITRIGGER_SELECTED_CAST_SPEED, _T("SelectedCastSpeed"));
    AddTrigger(UITRIGGER_SELECTED_ATTACK_RANGE, _T("SelectedAttackRange"));
    AddTrigger(UITRIGGER_SELECTED_ATTACK_COOLDOWN, _T("SelectedAttackCooldown"));
    AddTrigger(UITRIGGER_SELECTED_STRENGTH, _T("SelectedStrength"));
    AddTrigger(UITRIGGER_SELECTED_AGILITY, _T("SelectedAgility"));
    AddTrigger(UITRIGGER_SELECTED_INTELLIGENCE, _T("SelectedIntelligence"));
    AddTrigger(UITRIGGER_SELECTED_ATTRIBUTES, _T("SelectedAttributes"));
    AddTrigger(UITRIGGER_SELECTED_HEALTH, _T("SelectedHealth"), MAX_SELECTED_UNITS);
    AddTrigger(UITRIGGER_SELECTED_MANA, _T("SelectedMana"), MAX_SELECTED_UNITS);
    AddTrigger(UITRIGGER_SELECTED_HEALTHREGEN, _T("SelectedHealthRegen"));
    AddTrigger(UITRIGGER_SELECTED_MANAREGEN, _T("SelectedManaRegen"));
    AddTrigger(UITRIGGER_SELECTED_LIFETIME, _T("SelectedLifetime"));
    AddTrigger(UITRIGGER_SELECTED_HAS_INVENTORY, _T("SelectedHasInventory"));
    AddTrigger(UITRIGGER_SELECTED_HAS_ATTRIBUTES, _T("SelectedHasAttributes"));
    AddTrigger(UITRIGGER_SELECTED_PLAYER_INFO, _T("SelectedPlayerInfo"), MAX_SELECTED_UNITS);
    AddTrigger(UITRIGGER_SELECTED_ILLUSION, _T("SelectedIllusion"), MAX_SELECTED_UNITS);

    AddTrigger(UITRIGGER_SELECTED_ATTACK_INFO, _T("SelectedAttackInfo"));
    AddTrigger(UITRIGGER_SELECTED_DEFENSE_INFO, _T("SelectedDefenseInfo"));
    AddTrigger(UITRIGGER_SELECTED_ATTRIBUTE_INFO, _T("SelectedAttributeInfo"), MAX_SELECTED_UNITS);

    AddTrigger(UITRIGGER_SELECTED_INVENTORY_EXISTS, _T("SelectedInventoryExists"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_SELECTED_INVENTORY_ICON, _T("SelectedInventoryIcon"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_SELECTED_INVENTORY_RECIPE, _T("SelectedInventoryRecipe"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_SELECTED_INVENTORY_CAN_ACTIVATE, _T("SelectedInventoryCanActivate"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_SELECTED_INVENTORY_STATUS, _T("SelectedInventoryStatus"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_SELECTED_INVENTORY_COOLDOWN, _T("SelectedInventoryCooldown"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_SELECTED_INVENTORY_CHARGES, _T("SelectedInventoryCharges"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_SELECTED_INVENTORY_DESCRIPTION, _T("SelectedInventoryDescription"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_SELECTED_INVENTORY_AURA, _T("SelectedInventoryAura"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_SELECTED_INVENTORY_STATUS_EFFECT, _T("SelectedInventoryStatusEffect"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_SELECTED_INVENTORY_STATUS_EFFECTB, _T("SelectedInventoryStatusEffectB"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_SELECTED_INVENTORY_TRIGGERED_EFFECT, _T("SelectedInventoryTriggeredEffect"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_SELECTED_INVENTORY_PASSIVE_EFFECT, _T("SelectedInventoryPassiveEffect"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_SELECTED_INVENTORY_INTERFACE, _T("SelectedInventoryInterface"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_SELECTED_INVENTORY_STATE, _T("SelectedInventoryState"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_SELECTED_INVENTORY_HAS_TIMER, _T("SelectedInventoryHasTimer"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_SELECTED_INVENTORY_TIMER, _T("SelectedInventoryTimer"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_SELECTED_INVENTORY_DURATION, _T("SelectedInventoryDuration"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_SELECTED_INVENTORY_DURATION_PERCENT, _T("SelectedInventoryDurationPercent"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_SELECTED_INVENTORY_ACTIVATE_COST, _T("SelectedInventoryActivateCost"), MAX_INVENTORY);

    AddTrigger(UITRIGGER_ALT_INFO_0_PLAYER, _T("AltInfo0Player"));
    AddTrigger(UITRIGGER_ALT_INFO_0_NAME, _T("AltInfo0Name"));
    AddTrigger(UITRIGGER_ALT_INFO_0_TEAM, _T("AltInfo0Team"));
    AddTrigger(UITRIGGER_ALT_INFO_0_COLOR, _T("AltInfo0Color"));
    AddTrigger(UITRIGGER_ALT_INFO_0_HAS_HEALTH, _T("AltInfo0HasHealth"));
    AddTrigger(UITRIGGER_ALT_INFO_0_HEALTH_PERCENT, _T("AltInfo0HealthPercent"));
    AddTrigger(UITRIGGER_ALT_INFO_0_HEALTH_LERP, _T("AltInfo0HealthLerp"));
    AddTrigger(UITRIGGER_ALT_INFO_0_HAS_MANA, _T("AltInfo0HasMana"));
    AddTrigger(UITRIGGER_ALT_INFO_0_MANA_PERCENT, _T("AltInfo0ManaPercent"));
    AddTrigger(UITRIGGER_ALT_INFO_0_LEVEL, _T("AltInfo0Level"));
    AddTrigger(UITRIGGER_ALT_INFO_0_ITEMS, _T("AltInfo0Items"));
    AddTrigger(UITRIGGER_ALT_INFO_0_SHIELD, _T("AltInfo0Shield"));
    AddTrigger(UITRIGGER_ALT_INFO_0_MAX_SHIELD, _T("AltInfo0MaxShield"));
    AddTrigger(UITRIGGER_ALT_INFO_0_SHIELD_PERCENT, _T("AltInfo0ShieldPercent"));

    AddTrigger(UITRIGGER_ALT_INFO_1_PLAYER, _T("AltInfo1Player"));
    AddTrigger(UITRIGGER_ALT_INFO_1_NAME, _T("AltInfo1Name"));
    AddTrigger(UITRIGGER_ALT_INFO_1_TEAM, _T("AltInfo1Team"));
    AddTrigger(UITRIGGER_ALT_INFO_1_COLOR, _T("AltInfo1Color"));
    AddTrigger(UITRIGGER_ALT_INFO_1_HAS_HEALTH, _T("AltInfo1HasHealth"));
    AddTrigger(UITRIGGER_ALT_INFO_1_HEALTH_PERCENT, _T("AltInfo1HealthPercent"));
    AddTrigger(UITRIGGER_ALT_INFO_1_HEALTH_LERP, _T("AltInfo1HealthLerp"));
    AddTrigger(UITRIGGER_ALT_INFO_1_HAS_MANA, _T("AltInfo1HasMana"));
    AddTrigger(UITRIGGER_ALT_INFO_1_MANA_PERCENT, _T("AltInfo1ManaPercent"));
    AddTrigger(UITRIGGER_ALT_INFO_1_LEVEL, _T("AltInfo1Level"));
    AddTrigger(UITRIGGER_ALT_INFO_1_SHIELD, _T("AltInfo1Shield"));
    AddTrigger(UITRIGGER_ALT_INFO_1_MAX_SHIELD, _T("AltInfo1MaxShield"));
    AddTrigger(UITRIGGER_ALT_INFO_1_SHIELD_PERCENT, _T("AltInfo1ShieldPercent"));
    AddTrigger(UITRIGGER_ALT_INFO_1_IS_IN_EXP_RANGE, _T("AltInfo1IsInExpRange"));

    AddTrigger(UITRIGGER_ALT_INFO_2_PLAYER, _T("AltInfo2Player"));
    AddTrigger(UITRIGGER_ALT_INFO_2_NAME, _T("AltInfo2Name"));
    AddTrigger(UITRIGGER_ALT_INFO_2_TEAM, _T("AltInfo2Team"));
    AddTrigger(UITRIGGER_ALT_INFO_2_COLOR, _T("AltInfo2Color"));
    AddTrigger(UITRIGGER_ALT_INFO_2_HAS_HEALTH, _T("AltInfo2HasHealth"));
    AddTrigger(UITRIGGER_ALT_INFO_2_HEALTH_PERCENT, _T("AltInfo2HealthPercent"));
    AddTrigger(UITRIGGER_ALT_INFO_2_HEALTH_LERP, _T("AltInfo2HealthLerp"));
    AddTrigger(UITRIGGER_ALT_INFO_2_HAS_MANA, _T("AltInfo2HasMana"));
    AddTrigger(UITRIGGER_ALT_INFO_2_MANA_PERCENT, _T("AltInfo2ManaPercent"));
    AddTrigger(UITRIGGER_ALT_INFO_2_LEVEL, _T("AltInfo2Level"));
    AddTrigger(UITRIGGER_ALT_INFO_2_SHIELD, _T("AltInfo2Shield"));
    AddTrigger(UITRIGGER_ALT_INFO_2_MAX_SHIELD, _T("AltInfo2MaxShield"));
    AddTrigger(UITRIGGER_ALT_INFO_2_SHIELD_PERCENT, _T("AltInfo2ShieldPercent"));
    AddTrigger(UITRIGGER_ALT_INFO_2_IS_IN_EXP_RANGE, _T("AltInfo2IsInExpRange"));

    AddTrigger(UITRIGGER_ALT_INFO_3_PLAYER, _T("AltInfo3Player"));
    AddTrigger(UITRIGGER_ALT_INFO_3_NAME, _T("AltInfo3Name"));
    AddTrigger(UITRIGGER_ALT_INFO_3_TEAM, _T("AltInfo3Team"));
    AddTrigger(UITRIGGER_ALT_INFO_3_COLOR, _T("AltInfo3Color"));
    AddTrigger(UITRIGGER_ALT_INFO_3_HAS_HEALTH, _T("AltInfo3HasHealth"));
    AddTrigger(UITRIGGER_ALT_INFO_3_HEALTH_PERCENT, _T("AltInfo3HealthPercent"));
    AddTrigger(UITRIGGER_ALT_INFO_3_HEALTH_LERP, _T("AltInfo3HealthLerp"));
    AddTrigger(UITRIGGER_ALT_INFO_3_HAS_MANA, _T("AltInfo3HasMana"));
    AddTrigger(UITRIGGER_ALT_INFO_3_MANA_PERCENT, _T("AltInfo3ManaPercent"));
    AddTrigger(UITRIGGER_ALT_INFO_3_LEVEL, _T("AltInfo3Level"));
    AddTrigger(UITRIGGER_ALT_INFO_3_SHIELD, _T("AltInfo3Shield"));
    AddTrigger(UITRIGGER_ALT_INFO_3_MAX_SHIELD, _T("AltInfo3MaxShield"));
    AddTrigger(UITRIGGER_ALT_INFO_3_SHIELD_PERCENT, _T("AltInfo3ShieldPercent"));
    AddTrigger(UITRIGGER_ALT_INFO_3_IS_IN_EXP_RANGE, _T("AltInfo3IsInExpRange"));

    AddTrigger(UITRIGGER_SCOREBOARD_PLAYER, _T("ScoreboardPlayer"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SCOREBOARD_PLAYER_RESPAWN, _T("ScoreboardPlayerRespawn"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SCOREBOARD_TEAM, _T("ScoreboardTeam"), 3);

    AddTrigger(UITRIGGER_ITEM_CURSOR_VISIBLE, _T("ItemCursorVisible"));
    AddTrigger(UITRIGGER_ITEM_CURSOR_ICON, _T("ItemCursorIcon"));
    AddTrigger(UITRIGGER_ITEM_CURSOR_POSITION, _T("ItemCursorPosition"));

    AddTrigger(UITRIGGER_TOOL_TARGETING_ENTITY, _T("ToolTargetingEntity"));

    AddTrigger(UITRIGGER_BASE_HEALTH, _T("BaseHealth"), MAX_DISPLAY_TEAMS);
    AddTrigger(UITRIGGER_BASE_HEALTH_VISIBLE, _T("BaseHealthVisible"), MAX_DISPLAY_TEAMS);

    AddTrigger(UITRIGGER_TIME_OF_DAY, _T("TimeOfDay"));
    AddTrigger(UITRIGGER_DAYTIME, _T("DayTime"));
    AddTrigger(UITRIGGER_MATCH_TIME, _T("MatchTime"));
    AddTrigger(UITRIGGER_MATCH_ID, _T("MatchID"));

    AddTrigger(UITRIGGER_SCOREBOARD_CHANGE, _T("ScoreboardChange"));

    AddTrigger(UITRIGGER_MENU_PLAYER_INFO, _T("MenuPlayerInfo"), MAX_DISPLAY_PLAYERS);

    AddDumbTrigger(UITRIGGER_EVENT_FIRST_KILL, _T("EventFirstKill"));
    AddDumbTrigger(UITRIGGER_EVENT_MULTI_KILL, _T("EventMultiKill"));
    AddDumbTrigger(UITRIGGER_EVENT_KILL_STREAK, _T("EventKillStreak"));
    AddDumbTrigger(UITRIGGER_EVENT_TEAM_WIPE, _T("EventTeamWipe"));
    AddDumbTrigger(UITRIGGER_EVENT_TOWER_DENY, _T("EventTowerDeny"));
    AddDumbTrigger(UITRIGGER_EVENT_COURIER_KILL, _T("EventCourierKill"));
    AddDumbTrigger(UITRIGGER_EVENT_MEGA_CREEPS, _T("EventMegaCreeps"));
    AddDumbTrigger(UITRIGGER_EVENT_RIVAL, _T("EventRival"));
    AddDumbTrigger(UITRIGGER_EVENT_SMACKDOWN, _T("EventSmackdown"));
    AddDumbTrigger(UITRIGGER_EVENT_HUMILIATION, _T("EventHumiliation"));
    AddDumbTrigger(UITRIGGER_EVENT_PAYBACK, _T("EventPayback"));
    AddDumbTrigger(UITRIGGER_EVENT_RAGE_QUIT, _T("EventRageQuit"));
    AddDumbTrigger(UITRIGGER_EVENT_VICTORY, _T("EventVictory"));
    AddDumbTrigger(UITRIGGER_EVENT_DEFEAT, _T("EventDefeat"));

    AddTrigger(UITRIGGER_LOBBY_STATUS, _T("LobbyStatus"));
    AddTrigger(UITRIGGER_LOBBY_TEAM_INFO, _T("LobbyTeamInfo"), 3);
    AddTrigger(UITRIGGER_LOBBY_SPECTATORS, _T("LobbySpectators"));
    AddTrigger(UITRIGGER_LOBBY_PLAYER_INFO, _T("LobbyPlayerInfo"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_LOBBY_BUDDY, _T("LobbyBuddy"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_LOBBY_VOICE, _T("LobbyVoice"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_LOBBY_GAME_INFO, _T("LobbyGameInfo"));
    AddTrigger(UITRIGGER_LOBBY_REFEREE, _T("LobbyReferee"), MAX_TOTAL_REFEREES);
    AddTrigger(UITRIGGER_LOBBY_PLAYER_LIST, _T("LobbyPlayerList"), 32);
    AddTrigger(UITRIGGER_LOBBY_PLAYER_LIST_SIZE, _T("LobbyPlayerListSize"));
    AddTrigger(UITRIGGER_LOBBY_COUNTDOWN, _T("LobbyCountDown"));
    AddTrigger(UITRIGGER_LOBBY_PRIVATE, _T("LobbyPrivate"));
    AddTrigger(UITRIGGER_LOBBY_NO_STATS, _T("LobbyNoStats"));
    AddTrigger(UITRIGGER_LOBBY_NO_LEAVERS, _T("LobbyNoLeavers"));

    AddTrigger(UITRIGGER_HERO_SELECT_HERO_LIST, _T("HeroSelectHeroList"), MAX_HERO_LIST);
    AddTrigger(UITRIGGER_HERO_SELECT_HERO_ALT_AVATAR_LIST, _T("HeroSelectHeroAltAvatarList"), MAX_ALT_AVATAR_LIST);
    AddTrigger(UITRIGGER_HERO_SELECT_HERO_HAS_AVATARS, _T("HeroSelectHeroHasAvatars"));
    AddTrigger(UITRIGGER_HERO_SELECT_PLAYER_INFO, _T("HeroSelectPlayerInfo"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_HERO_SELECT_HERO_INFO, _T("HeroSelectHeroInfo"));
    AddTrigger(UITRIGGER_HERO_SELECT_HERO_ABILITY_INFO, _T("HeroSelectHeroAbilityInfo"), MAX_HERO_ABILITY_INFO);
    AddTrigger(UITRIGGER_HERO_SELECT_TIMER, _T("HeroSelectTimer"));
    AddTrigger(UITRIGGER_HERO_SELECT_INFO, _T("HeroSelectInfo"));
    AddTrigger(UITRIGGER_HERO_SELECT_HAS_EXTRA_TIME, _T("HeroSelectHasExtraTime"));
    AddTrigger(UITRIGGER_HERO_SELECT_EXTRA_TIME, _T("HeroSelectExtraTime"), MAX_DISPLAY_TEAMS);
    AddTrigger(UITRIGGER_HERO_SELECT_USING_EXTRA_TIME, _T("HeroSelectUsingExtraTime"), MAX_DISPLAY_TEAMS);
    AddTrigger(UITRIGGER_HERO_SELECT_NEED_AVATAR_SELECTION, _T("HeroSelectNeedAvatarSelection"));
    AddTrigger(UITRIGGER_HERO_SELECT_COINS, _T("HeroSelectCoins"));

    AddTrigger(UITRIGGER_ENDGAME, _T("EndGame"));
    AddTrigger(UITRIGGER_ENDGAME_INTERFACE_DISPLAY, _T("EndGameInterfaceDisplay"));
    AddTrigger(UITRIGGER_ENDGAME_MATCH_INFO, _T("EndGameMatchInfo"));
    AddTrigger(UITRIGGER_ENDGAME_PLAYER_STATS, _T("EndGamePlayerStats"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_ENDGAME_PLAYER_DETAIL_STATS, _T("EndGamePlayerDetailStats"));
    AddTrigger(UITRIGGER_ENDGAME_PLAYER_DETAIL_INVENTORY, _T("EndGamePlayerDetailInventory"), MAX_INVENTORY);
    AddTrigger(UITRIGGER_ENDGAME_PLAYER_DETAIL_ABILITY_HISTORY, _T("EndGamePlayerDetailAbilityHistory"));
    AddTrigger(UITRIGGER_ENDGAME_PLAYER_DETAIL_KILLS, _T("EndGamePlayerDetailKills"));
    AddTrigger(UITRIGGER_ENDGAME_PLAYER_DETAIL_ASSISTS, _T("EndGamePlayerDetailAssists"));
    AddTrigger(UITRIGGER_ENDGAME_PLAYER_DETAIL_DEATHS, _T("EndGamePlayerDetailDeaths"));
    AddTrigger(UITRIGGER_ENDGAME_TEAM_STATS, _T("EndGameTeamStats"), 3);
    AddTrigger(UITRIGGER_ENDGAME_TIMER, _T("EndGameTimer"));

    AddTrigger(UITRIGGER_CONNECTION_STATUS, _T("ConnectionStatus"));

    AddTrigger(UITRIGGER_VOTE_TYPE, _T("VoteType"));
    AddTrigger(UITRIGGER_VOTE_TIME, _T("VoteTime"));
    AddTrigger(UITRIGGER_VOTE_SHOW, _T("VoteShow"));
    AddTrigger(UITRIGGER_VOTE_PROGRESS, _T("VoteProgress"));
    AddTrigger(UITRIGGER_VOTED, _T("Voted"));
    AddTrigger(UITRIGGER_VOTE_PERMISSIONS, _T("VotePermissions"));
    AddTrigger(UITRIGGER_VOTE_KICK_PERMISSIONS, _T("VoteKickPermissions"));

    AddTrigger(UITRIGGER_SPECTATOR_TEAMINFO, _T("SpectatorTeamInfo"), 2);
    AddTrigger(UITRIGGER_SPECTATOR_PLAYER, _T("SpectatorPlayer"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SPECTATOR_PLAYER_HEALTH_PERCENT, _T("SpectatorPlayerHealthPercent"), MAX_DISPLAY_PLAYERS);

    AddTrigger(UITRIGGER_SPECTATOR_HERO_EXISTS, _T("SpectatorHeroExists"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SPECTATOR_HERO_INDEX, _T("SpectatorHeroIndex"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SPECTATOR_HERO_PLAYER_INFO, _T("SpectatorHeroPlayerInfo"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SPECTATOR_HERO_HERO_INFO, _T("SpectatorHeroHeroInfo"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SPECTATOR_HERO_HEALTH, _T("SpectatorHeroHealth"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SPECTATOR_HERO_MANA, _T("SpectatorHeroMana"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SPECTATOR_HERO_STATUS, _T("SpectatorHeroStatus"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SPECTATOR_HERO_RESPAWN, _T("SpectatorHeroRespawn"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SPECTATOR_HERO_DAMAGE, _T("SpectatorHeroDamage"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SPECTATOR_HERO_ARMOR, _T("SpectatorHeroArmor"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SPECTATOR_HERO_MAGIC_ARMOR, _T("SpectatorHeroMagicArmor"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SPECTATOR_HERO_MOVE_SPEED, _T("SpectatorHeroMoveSpeed"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SPECTATOR_HERO_ATTACK_SPEED, _T("SpectatorHeroAttackSpeed"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SPECTATOR_HERO_STRENGTH, _T("SpectatorHeroStrength"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SPECTATOR_HERO_AGILITY, _T("SpectatorHeroAgility"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SPECTATOR_HERO_INTELLIGENCE, _T("SpectatorHeroIntelligence"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SPECTATOR_HERO_GOLD, _T("SpectatorHeroGold"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SPECTATOR_HERO_DISCONNECTED, _T("SpectatorHeroDisconnected"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SPECTATOR_HERO_DISCONNECT_TIME, _T("SpectatorHeroDisconnectTime"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SPECTATOR_HERO_LOADING_PERCENT, _T("SpectatorHeroLoadingPercent"), MAX_DISPLAY_PLAYERS);

    AddTrigger(UITRIGGER_SPECTATOR_HERO_ABILITY_0_INFO, _T("SpectatorHeroAbility0Info"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SPECTATOR_HERO_ABILITY_0_COOLDOWN, _T("SpectatorHeroAbility0Cooldown"), MAX_DISPLAY_PLAYERS);

    AddTrigger(UITRIGGER_SPECTATOR_HERO_ABILITY_1_INFO, _T("SpectatorHeroAbility1Info"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SPECTATOR_HERO_ABILITY_1_COOLDOWN, _T("SpectatorHeroAbility1Cooldown"), MAX_DISPLAY_PLAYERS);

    AddTrigger(UITRIGGER_SPECTATOR_HERO_ABILITY_2_INFO, _T("SpectatorHeroAbility2Info"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SPECTATOR_HERO_ABILITY_2_COOLDOWN, _T("SpectatorHeroAbility2Cooldown"), MAX_DISPLAY_PLAYERS);

    AddTrigger(UITRIGGER_SPECTATOR_HERO_ABILITY_3_INFO, _T("SpectatorHeroAbility3Info"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SPECTATOR_HERO_ABILITY_3_COOLDOWN, _T("SpectatorHeroAbility3Cooldown"), MAX_DISPLAY_PLAYERS);

    AddTrigger(UITRIGGER_SPECTATOR_HERO_ABILITY_4_INFO, _T("SpectatorHeroAbility4Info"), MAX_DISPLAY_PLAYERS);
    AddTrigger(UITRIGGER_SPECTATOR_HERO_ABILITY_4_COOLDOWN, _T("SpectatorHeroAbility4Cooldown"), MAX_DISPLAY_PLAYERS);

    AddTrigger(UITRIGGER_KEY_MODIFIER1, _T("KeyModifier1"));
    AddTrigger(UITRIGGER_KEY_MODIFIER2, _T("KeyModifier2"));

    // *** OLD ***
    AddTrigger(UITRIGGER_BUILDING_ATTACK_ALERT, _T("BuildingAttackAlert"));

    AddTrigger(UITRIGGER_TEAM, _T("Team"));

    AddTrigger(UITRIGGER_GAME_MATCH_ID, _T("GameMatchID"));

    AddTrigger(UITRIGGER_KILL_NOTIFICATION, _T("KillNotification"));

    AddTrigger(UITRIGGER_VOICECHAT_TALKING, _T("VoiceChatTalking"), MAX_DISPLAY_PLAYERSPERTEAM);

    AddTrigger(UITRIGGER_REPLAY_NAME, _T("ReplayName"));
    AddTrigger(UITRIGGER_REPLAY_TIME, _T("ReplayTime"));
    AddTrigger(UITRIGGER_REPLAY_ENDTIME, _T("ReplayEndTime"));
    AddTrigger(UITRIGGER_REPLAY_FRAME, _T("ReplayFrame"));
    AddTrigger(UITRIGGER_REPLAY_ENDFRAME, _T("ReplayEndFrame"));
    AddTrigger(UITRIGGER_REPLAY_SPEED, _T("ReplaySpeed"));
    AddTrigger(UITRIGGER_REPLAY_PLAYING, _T("ReplayPlaying"));
    AddTrigger(UITRIGGER_REPLAY_PAUSED, _T("ReplayPaused"));

    AddTrigger(UITRIGGER_REPLAY_INFO_GAME, _T("ReplayInfoGame"));
    AddTrigger(UITRIGGER_REPLAY_INFO_PLAYER, _T("ReplayInfoPlayer"), MAX_DISPLAY_PLAYERS);

    AddTrigger(UITRIGGER_REPLAY_URL_STATUS, _T("ReplayURLStatus"));

    AddTrigger(UITRIGGER_PREVIEW_MAP_NAME, _T("PreviewMapName"));
    AddTrigger(UITRIGGER_PREVIEW_MAP_SIZE, _T("PreviewMapSize"));

    AddTrigger(UITRIGGER_COMMAND_ENABLED_MOVE, _T("CommandEnabledMove"));
    AddTrigger(UITRIGGER_COMMAND_ENABLED_ATTACK, _T("CommandEnabledAttack"));
    AddTrigger(UITRIGGER_COMMAND_ENABLED_STOP, _T("CommandEnabledStop"));
    AddTrigger(UITRIGGER_COMMAND_ENABLED_HOLD, _T("CommandEnabledHold"));
    AddTrigger(UITRIGGER_COMMAND_ENABLED_PATROL, _T("CommandEnabledPatrol"));

    AddTrigger(UITRIGGER_MATCH_INFO_SUMMARY, _T("MatchInfoSummary"));
    AddTrigger(UITRIGGER_MATCH_INFO_PLAYER, _T("MatchInfoPlayer"), MAX_DISPLAY_PLAYERSPERTEAM);
    AddTrigger(UITRIGGER_MATCH_INFO_ENTRY, _T("MatchEntry"));
    AddDumbTrigger(UITRIGGER_MATCH_INFO_ENTRY_FINISHED, _T("MatchEntriesFinished"));

    AddTrigger(UITRIGGER_TOURNAMENT_INFO, _T("TournamentInfo"));
    AddTrigger(UITRIGGER_TOURNAMENTS_FOR_ACCOUNT, _T("TournamentsForAccount"));

    AddTrigger(UITRIGGER_TOURNAMENT_INFO_RETURN, _T("TournamentInfoReturn"));
    AddTrigger(UITRIGGER_TOURNAMENT_INFO_TEAM1_PLAYERS_RETURN, _T("TournamentInfoTeam1PlayersReturn"));
    AddTrigger(UITRIGGER_TOURNAMENT_INFO_TEAM2_PLAYERS_RETURN, _T("TournamentInfoTeam2PlayersReturn")); 
    AddTrigger(UITRIGGER_TOURNAMENTS_FOR_ACCOUNT_RETURN, _T("TournamentsForAccountReturn"));

    AddDumbTrigger(UITRIGGER_ENTITY_DEFINITIONS_LOADED, _T("EntityDefinitionsLoaded"));
    AddTrigger(UITRIGGER_ENTITY_DEFINITIONS_PROGRESS, _T("EntityDefinitionsProgress"));

    AddDumbTrigger(UITRIGGER_COMPENDIUM_CLEAR_INFO, _T("CompendiumClearInfo"));
    AddTrigger(UITRIGGER_COMPENDIUM_HERO_INFO, _T("CompendiumHeroInfo"));
    AddTrigger(UITRIGGER_COMPENDIUM_DETAILED_HERO_INFO, _T("CompendiumDetailedHeroInfo"));
    
    AddTrigger(UITRIGGER_SYSTEM_DATE, _T("SystemDate"));
    AddTrigger(UITRIGGER_SYSTEM_WEEKDAY, _T("SystemWeekday"));
    AddTrigger(UITRIGGER_SYSTEM_TIME, _T("SystemTime"));
    AddTrigger(UITRIGGER_GOLD_REPORT, _T("GoldReport"));

    for (auto & m_vLastGameStatsPlayer : m_vLastGameStatsPlayers)
    {
        for (auto & x : m_vLastGameStatsPlayer)
        {
            x.clear();
            x.resize(57);
        }
    }
}


/*====================
  CGameInterfaceManager::AddTrigger
  ====================*/
void    CGameInterfaceManager::AddTrigger(uint uiTriggerID, const tstring &sName, uint uiCount)
{
    assert(uiTriggerID < m_vTriggers.size());

    SAFE_DELETE(m_vTriggers[uiTriggerID]);
    m_vTriggers[uiTriggerID] = K2_NEW(ctx_GameClient,   CSmartGameUITrigger)(sName, uiCount);
}


/*====================
  CGameInterfaceManager::AddDumbTrigger
  ====================*/
void    CGameInterfaceManager::AddDumbTrigger(uint uiTriggerID, const tstring &sName, uint uiCount)
{
    assert(uiTriggerID < m_vTriggers.size());

    SAFE_DELETE(m_vTriggers[uiTriggerID]);
    m_vTriggers[uiTriggerID] = K2_NEW(ctx_GameClient,   CSmartGameUITrigger)(sName, uiCount);
    m_vTriggers[uiTriggerID]->MakeDumb();
}


/*====================
  CGameInterfaceManager::LoadMainInterfaces
  ====================*/
void    CGameInterfaceManager::LoadMainInterfaces()
{
    PROFILE("CGameInterfaceManager::LoadMainInterfaces");

    tsvector vInterfaceList;
#if TKTK // TKTK 2023: Should we try to get the fe2 UI working?
    vInterfaceList.emplace_back(_T("/ui/fe2/main.interface"));
    vInterfaceList.emplace_back(_T("/ui/fe2/loading.interface"));
    vInterfaceList.emplace_back(_T("/ui/fe2/loading_matchmaking_preload.interface"));
    vInterfaceList.emplace_back(_T("/ui/fe2/loading_matchmaking_connecting.interface"));
#else
    vInterfaceList.emplace_back(_T("/ui/main.interface"));
    vInterfaceList.emplace_back(_T("/ui/loading.interface"));
    vInterfaceList.emplace_back(_T("/ui/game_loading.interface"));
#endif
    vInterfaceList.emplace_back(_T("/ui/main_popup.interface"));

    for (tstring & sFilename : vInterfaceList)
    {
        ResHandle hInterface(UIManager.LoadInterface(sFilename));
        if (hInterface != INVALID_RESOURCE)
            ++s_mapInterfaceReferenceCount[hInterface];
    }

    UIManager.AddOverlayInterface(_T("main_popup"));
}


/*====================
  CGameInterfaceManager::LoadGameInterface
  ====================*/
ResHandle   CGameInterfaceManager::LoadGameInterface(const tstring &sName)
{
    ResHandle hInterface(UIManager.LoadInterface(sName));
    if (hInterface != INVALID_RESOURCE)
        s_mapInterfaceReferenceCount[hInterface]++;

    return hInterface;
}


/*====================
  CGameInterfaceManager::LoadGameInterfaces
  ====================*/
void    CGameInterfaceManager::LoadGameInterfaces()
{
    GameClient.AddResourceToLoadingQueue(CLIENT_RESOURCE_UNTRACKED, _T("/ui/game.interface"), RES_INTERFACE);
    GameClient.AddResourceToLoadingQueue(CLIENT_RESOURCE_UNTRACKED, _T("/ui/game_lobby.interface"), RES_INTERFACE);
    GameClient.AddResourceToLoadingQueue(CLIENT_RESOURCE_UNTRACKED, _T("/ui/game_hero_select.interface"), RES_INTERFACE);
    GameClient.AddResourceToLoadingQueue(CLIENT_RESOURCE_UNTRACKED, _T("/ui/game_hero_loading.interface"), RES_INTERFACE);
    GameClient.AddResourceToLoadingQueue(CLIENT_RESOURCE_UNTRACKED, _T("/ui/game_menu.interface"), RES_INTERFACE);
    GameClient.AddResourceToLoadingQueue(CLIENT_RESOURCE_UNTRACKED, _T("/ui/game_spectator.interface"), RES_INTERFACE);
    GameClient.AddResourceToLoadingQueue(CLIENT_RESOURCE_UNTRACKED, _T("/ui/game_replay_control.interface"), RES_INTERFACE);
}


/*====================
  CGameInterfaceManager::UpdateLobby
  ====================*/
void    CGameInterfaceManager::UpdateLobby()
{
    PROFILE("CGameInterfaceManager::UpdateLobby");

    CPlayer *pLocalPlayer(GameClient.GetLocalPlayer());

    bool bIsHost(pLocalPlayer != nullptr && pLocalPlayer->HasFlags(PLAYER_FLAG_HOST));

    // Lobby status
    static tsvector vStatus(4);
    vStatus[0] = XtoA(GameClient.GetGamePhase() == GAME_PHASE_IDLE && GameClient.IsConnected());    // Is server idle TODO: Handle this better, somehow...
    vStatus[1] = XtoA(bIsHost); // Is host
    vStatus[2] = XtoA(Game.GetGameInfo() != nullptr && Game.GetGameInfo()->HasGameOptions(GAME_OPTION_AUTOBALANCE_TEAMS)); // Don't show locking for autobalance mode
    vStatus[3] = XtoA(Game.GetGameInfo() != nullptr && !Game.GetGameInfo()->HasGameOptions(GAME_OPTION_TOURNAMENT_RULES)); // Don't allow tournament rules games to be made public.
    Trigger(UITRIGGER_LOBBY_STATUS, vStatus);

    static tsvector vCountdown(2);
    if (GameClient.GetPhaseEndTime() == INVALID_TIME)
    {
        vCountdown[0] = _T("-1");
        vCountdown[1] = _T("false");
    }
    else
    {
        vCountdown[0] = XtoA(GameClient.GetRemainingPhaseTime());
        vCountdown[1] = XtoA((GameClient.GetRemainingPhaseTime() / 1000) != ((GameClient.GetRemainingPhaseTime() + GameClient.GetServerFrameLength()) / 1000));
    }
    Trigger(UITRIGGER_LOBBY_COUNTDOWN, vCountdown);

    Trigger(UITRIGGER_LOBBY_PRIVATE, int(GameClient.GetServerAccess()));
    Trigger(UITRIGGER_LOBBY_NO_STATS, (GameClient.GetHostFlags() & HOST_SERVER_NO_STATS) != 0);
    Trigger(UITRIGGER_LOBBY_NO_LEAVERS, (GameClient.GetHostFlags() & HOST_SERVER_NO_LEAVER) != 0);

    static tsvector vPlayerInfo(83);
    static tsvector vReferee(2);
    static tsvector vTeamInfo(6);

    bool bRosterChanged(false);

    bool bAllowTeamChanges(Game.GetGamePhase() == GAME_PHASE_WAITING_FOR_PLAYERS && Game.GetPhaseEndTime() == INVALID_TIME);

    // Spectators
    uint uiRefereeCount(0);
    CTeamInfo *pTeam(GameClient.GetTeam(TEAM_SPECTATOR));
    if (pTeam != nullptr)
    {
        // Team info
        vTeamInfo[0] = XtoA(pLocalPlayer != nullptr && bAllowTeamChanges && pTeam->CanJoinTeam(pLocalPlayer->GetClientNumber()));   // Can join
        vTeamInfo[1] = XtoA(pTeam->GetNumClients());                                                            // Player count
        vTeamInfo[2] = XtoA(GameClient.GetMaxSpectators());                                                     // Max players
        vTeamInfo[3] = XtoA(0);                                                                                 // Win chance
        vTeamInfo[4] = XtoA(IsFirstBanButtonVisible());                                                         // First Ban button visible
        vTeamInfo[5] = XtoA(Game.GetGameInfo()->GetFirstBanTeam());                                             // First Ban team
        Trigger(UITRIGGER_LOBBY_TEAM_INFO, vTeamInfo, 0);

        if (pTeam->IsRosterChanged())
        {
            bRosterChanged = true;
            pTeam->AckowledgeRosterChange();

            for (auto & it : vPlayerInfo)
                it.clear();
            vPlayerInfo[0] = _T("-1");
            Trigger(UITRIGGER_LOBBY_SPECTATORS, vPlayerInfo);

            // Player info
            for (uint uiTeamIndex(0); uiTeamIndex < pTeam->GetTeamSize(); ++uiTeamIndex)
            {
                CPlayer *pPlayer(pTeam->GetPlayer(uiTeamIndex));
                if (pPlayer == nullptr || pPlayer->IsDisconnected())
                    continue;

                vPlayerInfo[0] = XtoA(pPlayer->GetClientNumber());                                                                  // Client number
                vPlayerInfo[1] = pPlayer->GetName();                                                                                // Name
                vPlayerInfo[2] = XtoA(pPlayer->GetColor());                                                                         // Color
                vPlayerInfo[3] = XtoA(pPlayer->HasFlags(PLAYER_FLAG_HOST));                                                         // Is host
                vPlayerInfo[4] = XtoA(pPlayer->CanBeKicked() && pLocalPlayer != nullptr && pLocalPlayer->CanKick());                   // Can be kicked
                vPlayerInfo[5] = XtoA(pPlayer->IsReferee());                                                                        // Is referee
                vPlayerInfo[6] = XtoA(pLocalPlayer != nullptr && pLocalPlayer->HasFlags(PLAYER_FLAG_HOST) && pPlayer != pLocalPlayer); // Can be promoted
                vPlayerInfo[7] = pPlayer->GetColorName();                                                                           // Color name
                vPlayerInfo[8] = XtoA(pPlayer == pLocalPlayer);                                                                     // Is local player
                vPlayerInfo[9] = XtoA(pPlayer->GetAccountID());                                                                     // Account ID
                Trigger(UITRIGGER_LOBBY_SPECTATORS, vPlayerInfo);

                // Referees
                if (pPlayer->IsReferee())
                {
                    vReferee[0] = XtoA(pPlayer->GetClientNumber());
                    vReferee[1] = pPlayer->GetName();
                    Trigger(UITRIGGER_LOBBY_REFEREE, vReferee, uiRefereeCount);
                    ++uiRefereeCount;
                }
            }

            // Trigger empty settings if not all ref slots are full
            for (auto & it : vReferee)
                it.clear();
            vReferee[0] = _T("-1");
            for (uint ui(uiRefereeCount); ui < MAX_TOTAL_REFEREES; ++ui)
                Trigger(UITRIGGER_LOBBY_REFEREE, vReferee, ui);
        }
    }
    else
    {
        // Team info
        vTeamInfo[0] = XtoA(false);                                                                             // Can join
        vTeamInfo[1] = XtoA(0);                                                                                 // Player count
        vTeamInfo[2] = XtoA(0);                                                                                 // Max players
        vTeamInfo[3] = XtoA(0);                                                                                 // Win chance
        vTeamInfo[4] = XtoA(0);                                                                                 // First Ban button visible
        vTeamInfo[5] = XtoA(0);                                                                                 // First Ban team
        Trigger(UITRIGGER_LOBBY_TEAM_INFO, vTeamInfo, 0);

        // Trigger empty settings if not all ref slots are full
        for (tsvector_it it(vReferee.begin()); it != vReferee.end(); ++it)
            it->clear();
        vReferee[0] = _T("-1");
        for (uint ui(uiRefereeCount); ui < MAX_TOTAL_REFEREES; ++ui)
            Trigger(UITRIGGER_LOBBY_REFEREE, vReferee, ui);
    }

    // Teams
    uint uiPlayerIndex(0);
    for (uint uiTeam(1); uiTeam <= 2; ++uiTeam)
    {
        CTeamInfo *pTeam(GameClient.GetTeam(uiTeam));
        if (pTeam == nullptr)
        {
            // Team info
            vTeamInfo[0] = XtoA(false);                         // Can join
            vTeamInfo[1] = XtoA(0);                             // Player count
            vTeamInfo[2] = XtoA(0);                             // Max players
            vTeamInfo[3] = XtoA(0);                             // Win chance
            vTeamInfo[4] = XtoA(0);                             // First Ban button visible
            vTeamInfo[5] = XtoA(0);                             // First Ban team
            Trigger(UITRIGGER_LOBBY_TEAM_INFO, vTeamInfo, uiTeam);

            // Player info
            for (uint uiTeamIndex(0); uiTeamIndex < MAX_DISPLAY_PLAYERSPERTEAM; ++uiTeamIndex)
            {
                for (tsvector_it it(vPlayerInfo.begin()); it != vPlayerInfo.end(); ++it)
                    it->clear();
                vPlayerInfo[0] = _T("-2");
                vPlayerInfo[2] = XtoA(CPlayer::GetColor(uiPlayerIndex));
                vPlayerInfo[7] = CPlayer::GetColorName(uiPlayerIndex);
                vPlayerInfo[15] = XtoA(false);
                vPlayerInfo[17] = XtoA(pLocalPlayer ? pLocalPlayer->HasFlags(PLAYER_FLAG_HOST) : false);
                vPlayerInfo[21] = XtoA(Game.GetGameInfo() != nullptr && Game.GetGameInfo()->HasGameOptions(GAME_OPTION_AUTOBALANCE_TEAMS));
                Trigger(UITRIGGER_LOBBY_PLAYER_INFO, vPlayerInfo, uiPlayerIndex);
                ++uiPlayerIndex;

                Trigger(UITRIGGER_LOBBY_BUDDY, -1, uiPlayerIndex);
                Trigger(UITRIGGER_LOBBY_VOICE, -2, uiPlayerIndex);
            }

            continue;
        }

        if (pTeam->IsRosterChanged())
        {
            pTeam->AckowledgeRosterChange();
            bRosterChanged = true;
        }

        bool bCanJoinTeam(pLocalPlayer == nullptr ? false : bAllowTeamChanges && pTeam->CanJoinTeam(pLocalPlayer->GetClientNumber()));

        // Team info
        vTeamInfo[0] = XtoA(bCanJoinTeam);                                      // Can join
        vTeamInfo[1] = XtoA(pTeam->GetNumClients());                            // Player count
        vTeamInfo[2] = XtoA(GameClient.GetTeamSize());                          // Max players
        vTeamInfo[3] = XtoA(pTeam->GetWinChance());                             // Win chance
        vTeamInfo[4] = XtoA(IsFirstBanButtonVisible());                         // First Ban button visible
        vTeamInfo[5] = XtoA(Game.GetGameInfo()->GetFirstBanTeam());             // First Ban team
        Trigger(UITRIGGER_LOBBY_TEAM_INFO, vTeamInfo, uiTeam);

        // Player info
        for (uint uiTeamIndex(0); uiTeamIndex < MAX_DISPLAY_PLAYERSPERTEAM; ++uiTeamIndex)
        {
            CPlayer *pPlayer(pTeam->GetPlayer(uiTeamIndex));
            if (pPlayer == nullptr)
            {
                for (tsvector_it it(vPlayerInfo.begin()); it != vPlayerInfo.end(); ++it)
                    it->clear();
                vPlayerInfo[0] = uiTeamIndex < pTeam->GetTeamSize() ? _T("-1") : _T("-2");
                vPlayerInfo[2] = XtoA(CPlayer::GetColor(uiPlayerIndex));
                vPlayerInfo[7] = CPlayer::GetColorName(uiPlayerIndex);
                vPlayerInfo[15] = XtoA(pTeam->IsSlotLocked(uiTeamIndex));
                vPlayerInfo[17] = XtoA(pLocalPlayer ? pLocalPlayer->HasFlags(PLAYER_FLAG_HOST) : false);
                vPlayerInfo[21] = XtoA(Game.GetGameInfo() != nullptr && Game.GetGameInfo()->HasGameOptions(GAME_OPTION_AUTOBALANCE_TEAMS));
                Trigger(UITRIGGER_LOBBY_PLAYER_INFO, vPlayerInfo, uiPlayerIndex);
                ++uiPlayerIndex;

                Trigger(UITRIGGER_LOBBY_BUDDY, -1, uiPlayerIndex);
                Trigger(UITRIGGER_LOBBY_VOICE, -2, uiPlayerIndex);
                continue;
            }

            vPlayerInfo[0] = XtoA(pPlayer->GetClientNumber());                                                                  // Client number
            vPlayerInfo[1] = pPlayer->GetName();                                                                                // Name
            vPlayerInfo[2] = XtoA(pPlayer->GetColor());                                                                         // Color
            vPlayerInfo[3] = XtoA(pPlayer->HasFlags(PLAYER_FLAG_HOST));                                                         // Is host
            vPlayerInfo[4] = XtoA(pPlayer->CanBeKicked() && pLocalPlayer != nullptr && pLocalPlayer->CanKick());                   // Can be kicked
            vPlayerInfo[5] = XtoA(pPlayer->IsReferee());                                                                        // Is referee
            vPlayerInfo[6] = XtoA(pLocalPlayer != nullptr && pLocalPlayer->HasFlags(PLAYER_FLAG_HOST) && pPlayer != pLocalPlayer); // Can be promoted
            vPlayerInfo[7] = pPlayer->GetColorName();                                                                           // Color
            vPlayerInfo[8] = XtoA(pPlayer == pLocalPlayer);                                                                     // Is local player
            vPlayerInfo[9] = XtoA(pPlayer->GetAccountID());                                                                     // Account ID
            vPlayerInfo[10] = XtoA(pPlayer->GetRank());                                                                         // Rank
            vPlayerInfo[11] = XtoA((GameClient.GetHostFlags() & HOST_SERVER_NO_STATS) == 0);                                    // Point Change
            vPlayerInfo[12] = XtoA(pPlayer->GetAdjustedMatchWinValue());                                                        // Points for a win
            vPlayerInfo[13] = XtoA(pPlayer->GetAdjustedMatchLossValue());                                                       // Points for a loss
            vPlayerInfo[14] = XtoA(pPlayer->GetSkillDifferenceAdjustment());                                                    // Adjustment for over skilled players
            vPlayerInfo[15] = XtoA(pTeam->IsSlotLocked(uiTeamIndex));                                                           // Locked slot
            vPlayerInfo[16] = XtoA(bCanJoinTeam);                                                                               // Can join team
            vPlayerInfo[17] = XtoA(pLocalPlayer ? pLocalPlayer->HasFlags(PLAYER_FLAG_HOST) : false);                            // Is local player host
            vPlayerInfo[18] = XtoA(pLocalPlayer ? pLocalPlayer->GetTeam() == uiTeam: false);                                    // Is local player on this team
            vPlayerInfo[19] = XtoA(pPlayer->HasFlags(PLAYER_FLAG_STAFF));
            vPlayerInfo[20] = XtoA(pPlayer->HasFlags(PLAYER_FLAG_PREMIUM));
            vPlayerInfo[21] = XtoA(Game.GetGameInfo() != nullptr && Game.GetGameInfo()->HasGameOptions(GAME_OPTION_AUTOBALANCE_TEAMS));    // Don't show locking for autobalance mode
                //Extra Account info
            vPlayerInfo[22] = XtoA(pPlayer->GetAccountWins());
            vPlayerInfo[23] = XtoA(pPlayer->GetAccountLosses());
            vPlayerInfo[24] = XtoA(pPlayer->GetAccountDisconnects());
            vPlayerInfo[25] = XtoA(pPlayer->GetAccountKills());
            vPlayerInfo[26] = XtoA(pPlayer->GetAccountAssists());
            vPlayerInfo[27] = XtoA(pPlayer->GetAccountDeaths());
            vPlayerInfo[28] = XtoA(0);  // Old EM Trigger // Now a Free Param !!!! Use first
            vPlayerInfo[29] = XtoA(pPlayer->GetAccountExpMin());
            vPlayerInfo[30] = XtoA(pPlayer->GetAccountGoldMin());
            vPlayerInfo[31] = XtoA(pPlayer->GetAccountWardsMin());

            int iParamX(0);
            CHeroDefinition *pHeroDef(0);
            for(int iMatchNo(0); iMatchNo < 7; ++iMatchNo)
            {
                pHeroDef = EntityRegistry.GetDefinition<CHeroDefinition>(pPlayer->GetAccountRecentMatchHeroID(iMatchNo));
                if (pHeroDef)
                {
                    vPlayerInfo[32 + (++iParamX)] = pHeroDef->GetIconPath(0);
                    vPlayerInfo[32 + (++iParamX)] = pPlayer->GetAccountRecentMatchWin(iMatchNo) == 1 ? _T("^884Win") : _T("^522Lose");
                    vPlayerInfo[32 + (++iParamX)] = XtoA(pPlayer->GetAccountRecentMatchKills(iMatchNo));
                    vPlayerInfo[32 + (++iParamX)] = XtoA(pPlayer->GetAccountRecentMatchDeaths(iMatchNo));
                    vPlayerInfo[32 + (++iParamX)] = XtoA(pPlayer->GetAccountRecentMatchAssists(iMatchNo));
                }
                else
                {
                    vPlayerInfo[32 + (++iParamX)] = TSNULL;
                    vPlayerInfo[32 + (++iParamX)] = TSNULL;
                    vPlayerInfo[32 + (++iParamX)] = XtoA(0);
                    vPlayerInfo[32 + (++iParamX)] = XtoA(0);
                    vPlayerInfo[32 + (++iParamX)] = XtoA(0);
                }
            }

            for(int iHeroNo(0); iHeroNo < 5; ++iHeroNo)
            {
                pHeroDef = EntityRegistry.GetDefinition<CHeroDefinition>(pPlayer->GetAccountFavHeroID(iHeroNo));
                if (pHeroDef)
                {
                    vPlayerInfo[32 + (++iParamX)] = pHeroDef->GetIconPath(0);
                    vPlayerInfo[32 + (++iParamX)] = XtoA(INT_CEIL(pPlayer->GetAccountFavHeroPlayedPercent(iHeroNo) * 100.0f)) + _T("%");
                }
                else
                {
                    vPlayerInfo[32 + (++iParamX)] = TSNULL;
                    vPlayerInfo[32 + (++iParamX)] = XtoA(0);
                }
            } 
            vPlayerInfo[78] = XtoA(pPlayer->GetAccountRecentMatchWin(7)); // This is a bool checking for there first game

            vPlayerInfo[79] = Host.GetChatSymbolTexturePath(pPlayer->GetChatSymbol());
            vPlayerInfo[80] = Host.GetChatNameColorTexturePath(pPlayer->GetChatNameColor());
            vPlayerInfo[81] = Host.GetChatNameColorString(pPlayer->GetChatNameColor());
            vPlayerInfo[82] = Host.GetAccountIconTexturePath(pPlayer->GetAccountIcon());
            // (83) params

            Trigger(UITRIGGER_LOBBY_PLAYER_INFO, vPlayerInfo, uiPlayerIndex);

            // Buddy status: -1 disabled, 0 not buddy, 1 is buddy
            if (!ChatManager.IsConnected() || pPlayer == pLocalPlayer)
                Trigger(UITRIGGER_LOBBY_BUDDY, -1, uiPlayerIndex);
            else if (ChatManager.IsBuddy(pPlayer->GetName()))
                Trigger(UITRIGGER_LOBBY_BUDDY, 1, uiPlayerIndex);
            else
                Trigger(UITRIGGER_LOBBY_BUDDY, 0, uiPlayerIndex);

            // Voice status: -2 not ally, -1 muted, 0 silent, 1 talking
            if (pLocalPlayer != nullptr && pLocalPlayer->GetTeam() != pPlayer->GetTeam())
                Trigger(UITRIGGER_LOBBY_VOICE, -2, uiPlayerIndex);
            else if (VoiceManager.IsClientMuted(pPlayer->GetClientNumber()))
                Trigger(UITRIGGER_LOBBY_VOICE, -1, uiPlayerIndex);
            else if (VoiceManager.IsTalking(pPlayer->GetClientNumber()))
                Trigger(UITRIGGER_LOBBY_VOICE, 1, uiPlayerIndex);
            else
                Trigger(UITRIGGER_LOBBY_VOICE, 0, uiPlayerIndex);

            ++uiPlayerIndex;
        }
    }

    // Match settings
    static tstring sHost;
    static tsvector vPlayerList(20);

    const PlayerMap &mapPlayers(GameClient.GetPlayerMap());

    Trigger(UITRIGGER_LOBBY_PLAYER_LIST_SIZE, uint(mapPlayers.size()));
    
    uint uiPlayerMapIndex(0);
    sHost.clear();
    for (PlayerMap_cit it(mapPlayers.begin()); it != mapPlayers.end(); ++it)
    {
        CPlayer *pPlayer(it->second);

        if (pPlayer->HasFlags(PLAYER_FLAG_HOST))
        {
            if (!sHost.empty())
                sHost += _T(", ");
            sHost += pPlayer->GetName();
        }

        vPlayerList[0] = XtoA(pPlayer->GetClientNumber());
        vPlayerList[1] = pPlayer->GetName();
        vPlayerList[2] = XtoA(pPlayer->GetColor());
        vPlayerList[3] = XtoA(pPlayer->HasFlags(PLAYER_FLAG_HOST));
        vPlayerList[4] = XtoA(pPlayer->HasFlags(PLAYER_FLAG_LOADING));
        vPlayerList[5] = XtoA(int(pPlayer->GetTeam()));
        vPlayerList[6] = XtoA(pPlayer->IsReferee());
        vPlayerList[7] = XtoA(pPlayer->GetLoadingProgress());
        vPlayerList[8] = XtoA(pPlayer->HasFlags(PLAYER_FLAG_STAFF));
        vPlayerList[9] = XtoA(pPlayer->HasFlags(PLAYER_FLAG_PREMIUM));
        vPlayerList[10] = XtoA(bIsHost && pPlayer->CanBeKicked());
        vPlayerList[11] = XtoA(pPlayer->GetAccountID());
        vPlayerList[12] = Host.GetChatSymbolTexturePath(pPlayer->GetChatSymbol());
        vPlayerList[13] = Host.GetChatNameColorTexturePath(pPlayer->GetChatNameColor());
        vPlayerList[14] = Host.GetChatNameColorString(pPlayer->GetChatNameColor());
        vPlayerList[15] = Host.GetChatNameColorIngameString(pPlayer->GetChatNameColor());
        vPlayerList[16] = Host.GetAccountIconTexturePath(pPlayer->GetAccountIcon());
        vPlayerList[17] = XtoA(pPlayer == pLocalPlayer);
        vPlayerList[18] = XtoA(true); // Active slot
        vPlayerList[19] = XtoA(pPlayer->HasFlags(PLAYER_FLAG_LOADING) ? 1 : 2); // Connection status
        Trigger(UITRIGGER_LOBBY_PLAYER_LIST, vPlayerList, uiPlayerMapIndex++);
    }

    // Fill remaining expected slots with null entries
    CGameInfo *pGameInfo(Game.GetGameInfo());
    if (pGameInfo != nullptr && pGameInfo->HasFlags(GAME_FLAG_ARRANGED))
    {
        CTeamInfo *pTeam1(Game.GetTeam(TEAM_1));
        CTeamInfo *pTeam2(Game.GetTeam(TEAM_2));

        uint uiExpectedPlayers((pTeam1 != nullptr ? pTeam1->GetTeamSize() : 0) + (pTeam2 != nullptr ? pTeam2->GetTeamSize() : 0));

        for (uint ui(uiPlayerMapIndex); ui < uiExpectedPlayers; ++ui)
        {
            for (tsvector_it it(vPlayerList.begin()); it != vPlayerList.end(); ++it)
                it->clear();

            vPlayerList[0] = _CTS("-1");
            vPlayerList[2] = _CTS("white");
            vPlayerList[17] = XtoA(false);
            vPlayerList[18] = XtoA(true); // Active slot
            vPlayerList[19] = XtoA(0);  // Connection status
            Trigger(UITRIGGER_LOBBY_PLAYER_LIST, vPlayerList, uiPlayerMapIndex++);
        }
    }

    // Clear remaining entries
    for (tsvector_it it(vPlayerList.begin()); it != vPlayerList.end(); ++it)
        it->clear();

    vPlayerList[0] = _CTS("-1");
    vPlayerList[2] = _CTS("white");
    vPlayerList[18] = XtoA(false); // Active slot

    for (uint ui(uiPlayerMapIndex); ui < 32; ++ui)
        Trigger(UITRIGGER_LOBBY_PLAYER_LIST, vPlayerList, ui);

    static tsvector vGameInfo(12);
    vGameInfo[0] = pGameInfo == nullptr ? TSNULL : CGameInfo::GetGameModeString(pGameInfo->GetGameMode());
    vGameInfo[1] = pGameInfo == nullptr ? TSNULL : CGameInfo::GetGameOptionsString(pGameInfo->GetGameOptions());
    vGameInfo[2] = pGameInfo == nullptr ? TSNULL : pGameInfo->HasFlags(GAME_FLAG_SOLO) ? GameClient.GetGameMessage(_CTS("game_info_practice")) : pGameInfo->GetServerName();
    vGameInfo[3] = GameClient.GetWorldPointer() == nullptr ? TSNULL : GameClient.GetWorldPointer()->GetFancyName();
    vGameInfo[4] = XtoA(GameClient.GetConnectedClientCount());
    vGameInfo[5] = pGameInfo == nullptr ? TSNULL : pGameInfo->HasFlags(GAME_FLAG_SOLO) ? XtoA(1) : XtoA(pGameInfo->GetTeamSize() * 2 + pGameInfo->GetMaxSpectators() + pGameInfo->GetMaxReferees());
    vGameInfo[6] = pLocalPlayer == nullptr ? TSNULL : XtoA(pLocalPlayer->GetPing());
    vGameInfo[7] = pGameInfo == nullptr ? TSNULL : pGameInfo->GetGameName();
    vGameInfo[8] = XtoA(pGameInfo == nullptr ? -1 : int(pGameInfo->GetMatchID()));
    vGameInfo[9] = sHost;
    vGameInfo[10] = XtoA(Game.GetGameInfo() != nullptr && Game.GetGameInfo()->HasGameOptions(GAME_OPTION_AUTOBALANCE_TEAMS));  // Don't show locking for autobalance mode
    vGameInfo[11] = GameClient.GetServerVersion();
    Trigger(UITRIGGER_LOBBY_GAME_INFO, vGameInfo);
}


/*====================
  CGameInterfaceManager::UpdateHeroSelect
  ====================*/
void    CGameInterfaceManager::UpdateHeroSelect()
{
    PROFILE("CGameInterfaceManager::UpdateHeroSelect");

    uint uiRemainingTime(GameClient.GetRemainingPhaseTime());

    if (m_uiPrevPhase == GAME_PHASE_INVALID || m_uiPrevPhaseTime == INVALID_TIME || 
        m_uiPrevPhase != GameClient.GetGamePhase() ||
        m_uiPrevPhaseTime < uiRemainingTime)
    {
        m_uiPrevPhase = GameClient.GetGamePhase();
        m_uiPrevPhaseTime = uiRemainingTime + GameClient.GetServerFrameLength();
    }

    assert(m_uiPrevPhase != GAME_PHASE_INVALID && m_uiPrevPhaseTime != INVALID_TIME);

    bool bSecondElapsed(false);
    if ((uiRemainingTime / 1000) != (m_uiPrevPhaseTime / 1000))
    {
        m_uiPrevPhaseTime = uiRemainingTime;
        bSecondElapsed = true;
    }

    // Update phase timer cvar
    static tsvector vTimer(4);
    vTimer[0] = XtoA(uiRemainingTime);
    vTimer[1] = XtoA(GameClient.GetPhaseDuration());
    vTimer[2] = XtoA(GameClient.HasFlags(GAME_FLAG_FINAL_HERO_SELECT) && (GameClient.GetGamePhase() <= GAME_PHASE_HERO_SELECT));
    vTimer[3] = XtoA(bSecondElapsed);
    Trigger(UITRIGGER_HERO_SELECT_TIMER, vTimer);

    CPlayer *pLocalPlayer(GameClient.GetLocalPlayer());
    if (pLocalPlayer == nullptr)
        return;

    // Match settings
    static tsvector vGeneralInfo(16);
    CGameInfo *pGameInfo(Game.GetGameInfo());
    if (pGameInfo == nullptr)
    {
        for (tsvector_it it(vGeneralInfo.begin()); it != vGeneralInfo.end(); ++it)
            it->clear();
    }
    else
    {
        vGeneralInfo[0] = CGameInfo::GetGameModeString(pGameInfo->GetGameMode());
        vGeneralInfo[1] = CGameInfo::GetGameOptionsString(pGameInfo->GetGameOptions());
        switch (pGameInfo->GetGameMode())
        {
        case GAME_MODE_SINGLE_DRAFT:
            vGeneralInfo[2] = _T("sd");
            break;
        case GAME_MODE_RANDOM_DRAFT:
            vGeneralInfo[2] = _T("rd");
            break;
        case GAME_MODE_BANNING_DRAFT:
            vGeneralInfo[2] = _T("bd");
            break;
        case GAME_MODE_CAPTAINS_DRAFT:
            vGeneralInfo[2] = _T("cd");
            break;
        case GAME_MODE_CAPTAINS_MODE:
            vGeneralInfo[2] = _T("cm");
            break;
        case GAME_MODE_BANNING_PICK:
            vGeneralInfo[2] = _T("bp");
            break;
        default:
            vGeneralInfo[2] = _T("ap");
            break;
        }
        vGeneralInfo[3] = XtoA(pGameInfo->HasGameOptions(GAME_OPTION_NO_REPICK));
        vGeneralInfo[4] = XtoA(pGameInfo->HasGameOptions(GAME_OPTION_NO_SWAP));
        vGeneralInfo[5] = XtoA(pLocalPlayer->HasFlags(PLAYER_FLAG_CAN_PICK));
        vGeneralInfo[6] = XtoA(pLocalPlayer->CanSwap());
        vGeneralInfo[7] = XtoA(pLocalPlayer->HasSelectedHero());
        vGeneralInfo[8] = XtoA(GameClient.HasFlags(GAME_FLAG_FINAL_HERO_SELECT));
        vGeneralInfo[9] = XtoA(pLocalPlayer->HasFlags(PLAYER_FLAG_IS_CAPTAIN));
        vGeneralInfo[10] = XtoA(pGameInfo->GetRepickCost());
        vGeneralInfo[11] = XtoA(pGameInfo->GetRandomBonus());
        vGeneralInfo[12] = XtoA(pLocalPlayer->HasFlags(PLAYER_FLAG_HAS_REPICKED));
        vGeneralInfo[13] = XtoA(pLocalPlayer->GetTeam());
        vGeneralInfo[14] = XtoA(pLocalPlayer->GetTeamIndex());
        vGeneralInfo[15] = XtoA(GameClient.GetGamePhase());
    }
    Trigger(UITRIGGER_HERO_SELECT_INFO, vGeneralInfo);

    // Players
    static tsvector vPlayerInfo(29);

    Trigger(UITRIGGER_HERO_SELECT_HAS_EXTRA_TIME, pGameInfo->GetGameMode() == GAME_MODE_BANNING_PICK || pGameInfo->GetGameMode() == GAME_MODE_CAPTAINS_MODE);

    uint uiPlayerIndex(0);
    for (uint uiTeam(1); uiTeam <= 2; ++uiTeam)
    {
        CTeamInfo *pTeam(GameClient.GetTeam(uiTeam));
        if (pTeam == nullptr)
            continue;

        Trigger(UITRIGGER_HERO_SELECT_EXTRA_TIME, pTeam->GetStat(TEAM_STAT_TOWER_KILLS), uiTeam - 1);
        Trigger(UITRIGGER_HERO_SELECT_USING_EXTRA_TIME, pTeam->GetStat(TEAM_STAT_TOWER_DENIES) > 0, uiTeam - 1);

        // Player info
        for (uint uiTeamIndex(0); uiTeamIndex < MAX_DISPLAY_PLAYERSPERTEAM; ++uiTeamIndex)
        {
            CPlayer *pPlayer(GameClient.GetPlayer(pTeam->GetClientIDFromTeamIndex(uiTeamIndex)));
            if (pPlayer == nullptr)
            {
                for (tsvector_it it(vPlayerInfo.begin()); it != vPlayerInfo.end(); ++it)
                    it->clear();
                vPlayerInfo[0] = _T("-1");      // Team index
                vPlayerInfo[2] = _T("-1");      // Client number
                Trigger(UITRIGGER_HERO_SELECT_PLAYER_INFO, vPlayerInfo, uiPlayerIndex);
                ++uiPlayerIndex;
                continue;
            }

            bool bIsPotentialHero(false);
            CHeroDefinition *pHeroDef(EntityRegistry.GetDefinition<CHeroDefinition>(pPlayer->GetSelectedHero()));
            CHeroDefinition *pBaseHeroDef(pHeroDef);
            if (pHeroDef == nullptr)
            {
                pHeroDef = (EntityRegistry.GetDefinition<CHeroDefinition>(pPlayer->GetPotentialHero()));
                if (pHeroDef != nullptr)
                    bIsPotentialHero = true;
            }

            if (pHeroDef != nullptr && pPlayer->HasSelectedAvatar())
                pHeroDef = static_cast<CHeroDefinition*>(pHeroDef->GetModifiedDefinition(pPlayer->GetSelectedAvatar()));

            vPlayerInfo[0] = XtoA(uiTeamIndex);                                             // Team index
            vPlayerInfo[1] = XtoA(pPlayer == pLocalPlayer);                                 // Is local client
            vPlayerInfo[2] = XtoA(pPlayer->GetClientNumber());                              // Client number
            vPlayerInfo[3] = pPlayer->GetName();                                            // Player name
            vPlayerInfo[4] = XtoA(pPlayer->GetColor());                                     // Player color
            vPlayerInfo[5] = XtoA(pHeroDef ? pHeroDef->GetDisplayName() : TSNULL);          // Hero name
            vPlayerInfo[6] = XtoA(pHeroDef ? pHeroDef->GetIconPath(0) : TSNULL);            // Hero icon
            vPlayerInfo[7] = XtoA(pPlayer->CanRepick());                                    // Can repick
            vPlayerInfo[8] = XtoA(pLocalPlayer->CanSwap() && pPlayer->CanSwap());           // Can swap
            vPlayerInfo[9] = XtoA(pLocalPlayer->CanSwap(uiTeamIndex));                      // Swap offered
            vPlayerInfo[10] = XtoA(pPlayer->HasFlags(PLAYER_FLAG_READY));                   // Is ready
            vPlayerInfo[11] = XtoA(pPlayer->GetTeam() == pLocalPlayer->GetTeam());          // Is team mate
            vPlayerInfo[12] = XtoA(pLocalPlayer->HasSwapRequest(uiTeamIndex));              // Swap requested
            vPlayerInfo[13] = XtoA(pHeroDef ? pHeroDef->GetName() : TSNULL);                // Hero type ID
            vPlayerInfo[14] = XtoA(pPlayer->IsCurrentPicker());                             // Highlight player
            vPlayerInfo[15] = XtoA(pPlayer->HasSelectedHero() || bIsPotentialHero || pPlayer->GetBlindPick());          // Has a hero to display
            vPlayerInfo[16] = XtoA(pPlayer->IsHeroLocked());                                // Hero locked
            vPlayerInfo[17] = XtoA(pPlayer->GetGold());                                     // Gold
            vPlayerInfo[18] = XtoA(pPlayer->GetLoadingProgress());                          // Loading Progress
            vPlayerInfo[19] = XtoA(pPlayer->HasFlags(PLAYER_FLAG_LOADED_HEROES));           // Finished Loading
            vPlayerInfo[20] = XtoA(VoiceManager.IsTalking(pPlayer->GetClientNumber()));     // Is talking
            vPlayerInfo[21] = XtoA(VoiceManager.IsClientMuted(pPlayer->GetClientNumber())); // Is muted
            vPlayerInfo[22] = XtoA(pPlayer->GetAdjustedMatchWinValue());                    // Points for a win
            vPlayerInfo[23] = XtoA(pPlayer->GetAdjustedMatchLossValue());                   // Points for a loss
            vPlayerInfo[24] = XtoA(pPlayer->GetAccountID());                                // Account ID
            vPlayerInfo[25] = XtoA(bIsPotentialHero);                                       // Is Potential Hero
            vPlayerInfo[26] = XtoA(pPlayer->HasSelectedHero());                             // Has selected a hero
            vPlayerInfo[27] = XtoA(pBaseHeroDef ? pBaseHeroDef->HasAltAvatars() : false);   // Has alt avatars
            vPlayerInfo[28] = XtoA(pHeroDef == nullptr && pPlayer->GetBlindPick());            // Blind pick?
            Trigger(UITRIGGER_HERO_SELECT_PLAYER_INFO, vPlayerInfo, uiPlayerIndex);

            ++uiPlayerIndex;
        }
    }

    // Hero list
    static tsvector vHero(34);

    uint uiHeroCount(0);
    for (uint uiListIndex(0); uiListIndex < NUM_HERO_LISTS; ++uiListIndex)
    {
        const HeroList &vHeroList(GameClient.GetHeroList(uiListIndex));

        // For SD, only process ally lists if player is on a team
        if (GameClient.GetGameMode() == GAME_MODE_SINGLE_DRAFT)
        {
            if ((pLocalPlayer->GetTeam() == 1 || pLocalPlayer->GetTeam() == 2) &&
                (uiListIndex / 5) + 1 != pLocalPlayer->GetTeam())
            {
                uiHeroCount += 3;
                continue;
            }
        }

        // Default is to display max possible heroes per group
        uint uiEnd(MAX_HERO_LIST / 6);

        // For RD, process the entire first (and only) list
        if (GameClient.GetGameMode() == GAME_MODE_RANDOM_DRAFT || GameClient.GetGameMode() == GAME_MODE_CAPTAINS_DRAFT || GameClient.GetGameMode() == GAME_MODE_BANNING_DRAFT)
            uiEnd = INT_SIZE(vHeroList.size());
        else if (GameClient.GetGameMode() == GAME_MODE_SINGLE_DRAFT)
            uiEnd = 3;

        for (uint uiHeroIndex(0); uiHeroIndex < uiEnd; ++uiHeroIndex)
        {
            ushort unHeroID(uiHeroIndex >= vHeroList.size() ? INVALID_ENT_TYPE : vHeroList[uiHeroIndex].first);
            byte yStatus(uiHeroIndex >= vHeroList.size() ? HERO_LIST_NOT_AVAILABLE : vHeroList[uiHeroIndex].second);

            CHeroDefinition *pDefinition(EntityRegistry.GetDefinition<CHeroDefinition>(unHeroID));
            if (pDefinition == nullptr)
            {
                for (tsvector_it it(vHero.begin()); it != vHero.end(); ++it)
                    it->clear();
            }
            else
            {
                vHero[0] = EntityRegistry.LookupName(unHeroID);     // Name
                vHero[1] = pDefinition->GetIconPath(0);             // Icon
                
                if (yStatus == HERO_LIST_BANNED)
                    vHero[2] = _CTS("-1");                          // banned
                else if (yStatus == HERO_LIST_PICKED)
                    vHero[2] = _CTS("-2");                          // picked
                else if (yStatus == pLocalPlayer->GetTeam() || yStatus == HERO_LIST_AVAILABLE_ALL)
                    vHero[2] = _CTS("1");                           // available
                else
                    vHero[2] = _CTS("0");                           // not available

                vHero[3] = _CTS("true");                            // Is valid
                
                if (GameClient.GetGamePhase() == GAME_PHASE_HERO_BAN)
                    vHero[4] = XtoA(pLocalPlayer->HasFlags(PLAYER_FLAG_IS_CAPTAIN) && pLocalPlayer->HasFlags(PLAYER_FLAG_CAN_PICK));    // Can be selected
                else if (GameClient.GetGamePhase() == GAME_PHASE_HERO_SELECT)
                    vHero[4] = XtoA(!pLocalPlayer->HasSelectedHero() && pLocalPlayer->HasFlags(PLAYER_FLAG_CAN_PICK));  // Can be selected
                else
                    vHero[4] = XtoA(!pLocalPlayer->HasSelectedHero());  // Can be selected

                vHero[5] = pDefinition->GetDisplayName();
                vHero[8] = XtoA(IHeroEntity::AdjustArmor(pDefinition->GetArmor(0), pDefinition->GetAgility()));
                vHero[9] = XtoA(pDefinition->GetStrength());
                vHero[10] = XtoA(pDefinition->GetAgility());
                vHero[11] = XtoA(pDefinition->GetIntelligence());
                switch (pDefinition->GetPrimaryAttribute())
                {
                case ATTRIBUTE_STRENGTH:
                    vHero[6] = XtoA(pDefinition->GetAttackDamageMin(0) + pDefinition->GetStrength());
                    vHero[7] = XtoA(pDefinition->GetAttackDamageMax(0) + pDefinition->GetStrength());
                    vHero[12] = _CTS("strength");
                    break;
                case ATTRIBUTE_AGILITY:
                    vHero[6] = XtoA(pDefinition->GetAttackDamageMin(0) + pDefinition->GetAgility());
                    vHero[7] = XtoA(pDefinition->GetAttackDamageMax(0) + pDefinition->GetAgility());
                    vHero[12] = _CTS("agility");
                    break;
                case ATTRIBUTE_INTELLIGENCE:
                    vHero[6] = XtoA(pDefinition->GetAttackDamageMin(0) + pDefinition->GetIntelligence());
                    vHero[7] = XtoA(pDefinition->GetAttackDamageMax(0) + pDefinition->GetIntelligence());
                    vHero[12] = _CTS("intelligence");
                    break;
                }

                CAbilityDefinition *pAbility(nullptr);

#define ABILITY_INFO(index) \
                pAbility = EntityRegistry.GetDefinition<CAbilityDefinition>(pDefinition->GetInventory##index(0)); \
                if (pAbility != nullptr) \
                { \
                    vHero[13 + ((index) * 4)] = pAbility->GetDisplayName(); \
                    vHero[14 + ((index) * 4)] = pAbility->GetIconPath(0); \
                    vHero[15 + ((index) * 4)] = XtoA(pAbility->GetManaCost(0)); \
                    vHero[16 + ((index) * 4)] = XtoA(pAbility->GetCooldownTime(0)); \
                }

                ABILITY_INFO(0)
                ABILITY_INFO(1)
                ABILITY_INFO(2)
                ABILITY_INFO(3)
#undef ABILITY_INFO

                vHero[30] = LowerString(pDefinition->GetTeam());

                vHero[31] = XtoA(pDefinition->HasAltAvatars());

                vHero[32] = XtoA(uiListIndex);
                vHero[33] = XtoA(uiHeroIndex);
            }

            if (GameClient.GetGamePhase() == GAME_PHASE_HERO_BAN)
                vHero[29] = _CTS("BanHero");
            else if (GameClient.GetGamePhase() >= GAME_PHASE_HERO_SELECT)
                vHero[29] = _CTS("SpawnHero");
            else
                vHero[29] = _CTS("DraftHero");

            Trigger(UITRIGGER_HERO_SELECT_HERO_LIST, vHero, uiHeroCount + uiHeroIndex);
        }

        uiHeroCount += uiEnd;
    }

    // Avatar list
    static tsvector vAvatar(16);
    uint uiAvatarIndex(0);

    ushort unAvatarHero(INVALID_ENT_TYPE);
    byte yAvatarStatus(HERO_LIST_UNKNOWN);

    if (!cg_avatarHero.empty())
    {
        unAvatarHero = EntityRegistry.LookupID(cg_avatarHero);
        //yAvatarStatus = pLocalPlayer->HasSelectedHero() && pLocalPlayer->GetSelectedHero() == unAvatarHero ? HERO_LIST_AVAILABLE_ALL : HERO_LIST_NOT_AVAILABLE;
        yAvatarStatus = GameClient.GetHeroStatus(unAvatarHero);
    }

    CHeroDefinition *pBaseDefinition(EntityRegistry.GetDefinition<CHeroDefinition>(unAvatarHero));
    if (pBaseDefinition != nullptr)
    {
        // Base definition
        {
            bool bAvailable(true);
            bool bCanSelect
            (
                bAvailable
                &&
                (
                    (
                        !pLocalPlayer->HasSelectedHero()
                        &&
                        (
                            yAvatarStatus == pLocalPlayer->GetTeam()
                            ||
                            yAvatarStatus == HERO_LIST_AVAILABLE_ALL
                        )
                    )
                    ||
                    pLocalPlayer->GetSelectedHero() == unAvatarHero
                )
            );

            vAvatar[0] = _T("Base");                                                            // Name
            vAvatar[1] = _T("true");                                                            // Available
            vAvatar[2] = pBaseDefinition->GetDisplayName();                                     // Display name
            vAvatar[3] = pBaseDefinition->GetIconPath(0);                                       // Icon
            vAvatar[4] = pBaseDefinition->GetPreviewModelPath();                                // Model
            vAvatar[5] = pBaseDefinition->GetPassiveEffectPath(0);                              // Passive Effect
            vAvatar[6] = XtoA(pBaseDefinition->GetPreviewPos());                                // Pos
            vAvatar[7] = XtoA(pBaseDefinition->GetPreviewAngles());                             // Angles
            vAvatar[8] = XtoA(pBaseDefinition->GetPreviewScale());                              // Scale
            vAvatar[9] = XtoA(bCanSelect);                                                      // Can Select

            if (yAvatarStatus == HERO_LIST_BANNED)
                vAvatar[10] = _CTS("-1");                           // banned
            else if (yAvatarStatus == HERO_LIST_PICKED)
                vAvatar[10] = _CTS("-2");                           // picked
            else if (yAvatarStatus == pLocalPlayer->GetTeam() || yAvatarStatus == HERO_LIST_AVAILABLE_ALL)
                vAvatar[10] = _CTS("1");                            // available
            else
                vAvatar[10] = _CTS("0");                            // not available

            vAvatar[11] = XtoA(pLocalPlayer->HasSelectedHero() &&
                pLocalPlayer->HasSelectedAvatar() &&
                pLocalPlayer->GetSelectedHero() == unAvatarHero && 
                pLocalPlayer->GetSelectedAvatar() == 0);                                        // Selected

            vAvatar[12] = XtoA(pLocalPlayer->HasSelectedHero());                                // Hero selected
            vAvatar[13] = XtoA(pBaseDefinition->GetName());                                     // Hero Type Name
            vAvatar[14] = pBaseDefinition->GetDisplayName();                                    // Upgrade display name
            vAvatar[15] = XtoA(0);                                                              // Upgrade cost

            Trigger(UITRIGGER_HERO_SELECT_HERO_ALT_AVATAR_LIST, vAvatar, uiAvatarIndex);

            ++uiAvatarIndex;
        }

        const EntityModifierMap &mapModifiers(pBaseDefinition->GetModifiers());

        for (EntityModifierMap::const_iterator it(mapModifiers.begin()); it != mapModifiers.end() && uiAvatarIndex < MAX_ALT_AVATAR_LIST; ++it)
        {
            CHeroDefinition *pAltDefinition(static_cast<CHeroDefinition*>(it->second));

            if (!pAltDefinition->GetAltAvatar())
                continue;

            bool bAvailable(Game.CanAccessAltAvatar(pBaseDefinition->GetName(), EntityRegistry.LookupModifierKey(pAltDefinition->GetModifierID())));
            bool bCanSelect
            (
                bAvailable
                &&
                (
                    (
                        !pLocalPlayer->HasSelectedHero()
                        &&
                        (
                            yAvatarStatus == pLocalPlayer->GetTeam()
                            ||
                            yAvatarStatus == HERO_LIST_AVAILABLE_ALL
                        )
                    )
                    ||
                    pLocalPlayer->GetSelectedHero() == unAvatarHero
                )
            );

            const tstring &sModifierName(EntityRegistry.LookupModifierKey(pAltDefinition->GetModifierID()));

            const SAvatarInfo *pAvatarInfo(GameClient.GetClient()->GetAvatarInfo(pAltDefinition->GetName() + _T(".") + sModifierName));

#if 0
            SAvatarInfo avatarInfo;

            if (pAvatarInfo == nullptr)
            {
                avatarInfo.sName = pAltDefinition->GetName() + _T(".") + sModifierName;
                avatarInfo.sCName = pAltDefinition->GetName() + _T(".") + sModifierName;
                avatarInfo.uiCost = 0;

                pAvatarInfo = &avatarInfo;
            }
#else
            if (pAvatarInfo == nullptr)
                continue;
#endif

            vAvatar[0] = sModifierName;                                                         // Name
            vAvatar[1] = XtoA(bAvailable);                                                      // Available
            vAvatar[2] = pAltDefinition->GetDisplayName();                                      // Display name
            vAvatar[3] = pAltDefinition->GetIconPath(0);                                        // Icon
            vAvatar[4] = pAltDefinition->GetPreviewModelPath();                                 // Model
            vAvatar[5] = pAltDefinition->GetPassiveEffectPath(0);                               // Passive Effect
            vAvatar[6] = XtoA(pAltDefinition->GetPreviewPos());                                 // Pos
            vAvatar[7] = XtoA(pAltDefinition->GetPreviewAngles());                              // Angles
            vAvatar[8] = XtoA(pAltDefinition->GetPreviewScale());                               // Scale
            vAvatar[9] = XtoA(bCanSelect);                                                      // Can Select

            if (yAvatarStatus == HERO_LIST_BANNED)
                vAvatar[10] = _CTS("-1");                           // banned
            else if (yAvatarStatus == HERO_LIST_PICKED)
                vAvatar[10] = _CTS("-2");                           // picked
            else if (yAvatarStatus == pLocalPlayer->GetTeam() || yAvatarStatus == HERO_LIST_AVAILABLE_ALL)
                vAvatar[10] = _CTS("1");                            // available
            else
                vAvatar[10] = _CTS("0");                            // not available

            vAvatar[11] = XtoA(pLocalPlayer->HasSelectedHero() &&
                pLocalPlayer->HasSelectedAvatar() &&
                pLocalPlayer->GetSelectedHero() == unAvatarHero && 
                pLocalPlayer->GetSelectedAvatar() == it->first);                                // Selected

            vAvatar[12] = XtoA(pLocalPlayer->HasSelectedHero());                                // Hero selected
            vAvatar[13] = pAltDefinition->GetName();                                            // Hero Type Name
            vAvatar[14] = pAvatarInfo->sCName;                                                  // Upgrade display name
            vAvatar[15] = XtoA(pAvatarInfo->uiCost);                                            // Upgrade cost

            Trigger(UITRIGGER_HERO_SELECT_HERO_ALT_AVATAR_LIST, vAvatar, uiAvatarIndex);

            ++uiAvatarIndex;
        }

        Trigger(UITRIGGER_HERO_SELECT_HERO_HAS_AVATARS, uiAvatarIndex > 1);
    }
    else
    {
        Trigger(UITRIGGER_HERO_SELECT_HERO_HAS_AVATARS, false);
    }

    // Clear remaining slots
    for (; uiAvatarIndex < MAX_ALT_AVATAR_LIST; ++uiAvatarIndex)
    {
        for (uint i(0); i < uint(vAvatar.size()); ++i)
            vAvatar[i] = TSNULL;

        Trigger(UITRIGGER_HERO_SELECT_HERO_ALT_AVATAR_LIST, vAvatar, uiAvatarIndex);
    }

    Trigger(UITRIGGER_HERO_SELECT_NEED_AVATAR_SELECTION, pLocalPlayer->HasSelectedHero() && !pLocalPlayer->HasSelectedAvatar() && GameClient.GetGamePhase() == GAME_PHASE_HERO_SELECT);

    Trigger(UITRIGGER_HERO_SELECT_COINS, GameClient.GetClient()->GetCoins());
}


/*====================
  CGameInterfaceManager::UpdateScores
  ====================*/
void    CGameInterfaceManager::UpdateScores()
{
    PROFILE("CGameInterfaceManager::UpdateScores");

    static tsvector vPlayer(9);
    static tsvector vTeam(5);

    for (uint uiTeam(0); uiTeam < MAX_DISPLAY_TEAMS; ++uiTeam)
    {
        CTeamInfo *pTeam(GameClient.GetTeam(uiTeam + 1));
        if (pTeam == nullptr)
            continue;

        int iTotalPlayers(0);

        uint uiTotalKills(0);
        uint uiTotalDeaths(0);

        for (uint uiPlayer(0); uiPlayer < MAX_DISPLAY_PLAYERSPERTEAM; ++uiPlayer)
        {
            CPlayer *pClient(GameClient.GetPlayer(pTeam->GetClientIDFromTeamIndex(uiPlayer)));

            if (pClient == nullptr)
                continue;

            IHeroEntity *pHero(pClient->GetHero());
            if (pHero == nullptr)
            {
                vPlayer[0] = pClient->GetName();
                vPlayer[1] = _T("No hero");
                vPlayer[2] = TSNULL;
                vPlayer[3] = XtoA(pClient->GetColor());
                vPlayer[4] = TSNULL;
                vPlayer[5] = XtoA(pClient->GetStat(PLAYER_STAT_HERO_KILLS));
                vPlayer[6] = XtoA(pClient->GetStat(PLAYER_STAT_DEATHS));
                vPlayer[7] = XtoA(pClient->GetStat(PLAYER_STAT_ASSISTS));
                vPlayer[8] = _T("false");

                Trigger(UITRIGGER_SCOREBOARD_PLAYER_RESPAWN, 0u, uiTeam * MAX_DISPLAY_PLAYERSPERTEAM + iTotalPlayers);
            }
            else
            {
                vPlayer[0] = pClient->GetName();
                vPlayer[1] = pHero->GetDisplayName();
                vPlayer[2] = pHero->GetIconPath();
                vPlayer[3] = XtoA(pClient->GetColor());
                vPlayer[4] = XtoA(pHero->GetLevel());
                vPlayer[5] = XtoA(pClient->GetStat(PLAYER_STAT_HERO_KILLS));
                vPlayer[6] = XtoA(pClient->GetStat(PLAYER_STAT_DEATHS));
                vPlayer[7] = XtoA(pClient->GetStat(PLAYER_STAT_ASSISTS));
                vPlayer[8] = XtoA(pHero->GetStatus() == ENTITY_STATUS_ACTIVE);

                uint uiRespawnTime(pHero->GetRespawnTime() != INVALID_TIME ? MAX(int(pHero->GetRemainingRespawnTime() - GameClient.GetServerFrameLength()), 0) : 0);

                Trigger(UITRIGGER_SCOREBOARD_PLAYER_RESPAWN, uint(INT_CEIL(MsToSec(uiRespawnTime))), uiTeam * MAX_DISPLAY_PLAYERSPERTEAM + iTotalPlayers);
            }

            uiTotalKills += pClient->GetStat(PLAYER_STAT_HERO_KILLS);
            uiTotalDeaths += pClient->GetStat(PLAYER_STAT_DEATHS);

            Trigger(UITRIGGER_SCOREBOARD_PLAYER, vPlayer, uiTeam * MAX_DISPLAY_PLAYERSPERTEAM + iTotalPlayers);
            ++iTotalPlayers;
        }

        for (int iPlayer(iTotalPlayers); iPlayer < MAX_DISPLAY_PLAYERSPERTEAM; ++iPlayer)
        {
            vPlayer[0] = TSNULL;
            vPlayer[1] = TSNULL;
            vPlayer[2] = TSNULL;
            vPlayer[3] = TSNULL;
            vPlayer[4] = TSNULL;
            vPlayer[5] = TSNULL;
            vPlayer[6] = TSNULL;
            vPlayer[7] = TSNULL;
            vPlayer[8] = TSNULL;

            Trigger(UITRIGGER_SCOREBOARD_PLAYER, vPlayer, uiTeam * MAX_DISPLAY_PLAYERSPERTEAM + iTotalPlayers);
            ++iTotalPlayers;
        }

        vTeam[0] = pTeam->GetName();
        vTeam[1] = XtoA(uiTotalKills);
        vTeam[2] = XtoA(uiTotalDeaths);
        vTeam[3] = XtoA(pTeam->GetCurrentTowerCount());
        vTeam[4] = XtoA(pTeam->GetStartingTowerCount());
        Trigger(UITRIGGER_SCOREBOARD_TEAM, vTeam, pTeam->GetTeamID());
    }
}


/*====================
  CGameInterfaceManager::UpdateVoiceChat
  ====================*/
void    CGameInterfaceManager::UpdateVoiceChat()
{
    CPlayer *pLocalPlayer(Game.GetLocalPlayer());
    if (pLocalPlayer == nullptr)
        return;

    CTeamInfo *pTeam(Game.GetTeam(pLocalPlayer->GetTeam()));
    if (pTeam == nullptr || !pTeam->IsActiveTeam())
        return;

    for (uint uiPlayerTeamIndex(0); uiPlayerTeamIndex < MAX_DISPLAY_PLAYERSPERTEAM; ++uiPlayerTeamIndex)
    {
        CPlayer *pPlayer(pTeam->GetPlayer(uiPlayerTeamIndex));

        tsvector vParams(5);

        if (pPlayer == nullptr)
        {
            vParams[0] = _T("0");
            vParams[1] = _T("invisible");
            vParams[2] = _T("");
            vParams[3] = _T("-1");
            vParams[4] = _T("-1");
        }
        else
        {
            vParams[0] = XtoA(VoiceManager.IsTalking(pPlayer->GetClientNumber()), true);
            vParams[1] = XtoA(pPlayer->GetColor());
            vParams[2] = pPlayer->GetName();
            vParams[3] = XtoA(pPlayer->GetClientNumber());
            vParams[4] = XtoA(pPlayer->GetTeam());
        }

        Trigger(UITRIGGER_VOICECHAT_TALKING, vParams, uiPlayerTeamIndex);
    }
}


/*====================
  CGameInterfaceManager::UpdateCursor
  ====================*/
void    CGameInterfaceManager::UpdateCursor()
{
    // Default mouse behavior
    Input.SetCursorRecenter(CURSOR_GAME, BOOL_FALSE);

    if (Vid.IsFullScreen())
        Input.SetCursorConstrained(CURSOR_GAME, cg_constrainCursor ? BOOL_TRUE : BOOL_FALSE);
    else
        Input.SetCursorConstrained(CURSOR_GAME, cg_constrainCursor && m_eCurrentInterface >= CG_INTERFACE_GAME ? BOOL_TRUE : BOOL_FALSE);

    if (m_bCursorHidden)
        Input.SetCursorHidden(CURSOR_GAME, BOOL_TRUE);
    else
        Input.SetCursorHidden(CURSOR_GAME, BOOL_FALSE);

    Input.SetCursorFrozen(CURSOR_GAME, BOOL_FALSE);

    Trigger(UITRIGGER_TOOL_TARGETING_ENTITY, GameClient.GetClientCommander()->IsTargetingEntity());

    // Update cursor state
    switch (m_eCurrentInterface)
    {
    case CG_INTERFACE_GAME:
    case CG_INTERFACE_GAME_SPECTATOR:
        UpdateItemCursor();
        break;

    default:
        break;
    }
}


EXTERN_CVAR_BOOL(cg_dev);

/*====================
  CGameInterfaceManager::Update
  ====================*/
void    CGameInterfaceManager::Update()
{
    PROFILE("CGameInterfaceManager::Update");

    if (ui_forceUpdate || m_eCurrentInterface != GameClient.GetCurrentInterface() || GameClient.InterfaceNeedsUpdate())
        ForceUpdate();

    m_eCurrentInterface = GameClient.GetCurrentInterface();
    if (!cg_forceInterface.empty())
    {
        m_eCurrentInterface = CG_INTERFACE_INVALID;
        UIManager.SetActiveInterface(cg_forceInterface);
    }

    CGameInfo *pGameInfo(GameClient.GetGameInfo());

    Trigger(UITRIGGER_HOST_TIME, Host.GetTime());
    Trigger(UITRIGGER_MAIN_DEV, bool(cg_dev) || (pGameInfo != nullptr && pGameInfo->HasFlags(GAME_FLAG_SOLO) && !pGameInfo->GetNoDev()));

    UpdateCursor();

    // Check load queue for entries
    if (m_vLoadQueue.size() > 0)
    {
        uint uiLoadEnd((uint)m_vLoadQueue.size());

        uint uiStartTime(K2System.Milliseconds());

        while (K2System.Milliseconds() - uiStartTime < 50 && m_uiLoadPos < uiLoadEnd)
        {
            g_ResourceManager.Register(m_vLoadQueue[m_uiLoadPos], RES_ENTITY_DEF);
            m_uiLoadPos++;
        }

        if (m_uiLoadPos >= m_vLoadQueue.size())
        {
            GameClient.PostProcessEntities();

            m_bEntitiesLoaded = true;
            m_vLoadQueue.clear();

            Trigger(UITRIGGER_ENTITY_DEFINITIONS_LOADED, TSNULL);
        }
        else
        {
            tsvector vProgress(2);
            vProgress[0] = XtoA(m_uiLoadPos);
            vProgress[1] = XtoA(INT_SIZE(m_vLoadQueue.size()));

            Trigger(UITRIGGER_ENTITY_DEFINITIONS_PROGRESS, vProgress);
        }
    }

    ProcessStatsRequest();
    ProcessMatchInfoRequest();
    ProcessTournamentRequest();
    ProcessRecentMatchesRequest();

#if 0
    // Limit interface updates to cg_interfaceFPS (triggers only, not sliding/fading, etc)
    uint uiMS(1000 / cg_interfaceFPS);

    if (m_uiLastUpdateTime + uiMS > Host.GetTime())
        return;

    m_uiLastUpdateTime = Host.GetTime() / uiMS * uiMS; // Round down to the nearest uiMS (acts like an accumulator)
#endif

    uint uiTeam(0);
    CPlayer *pLocalPlayer(GameClient.GetLocalPlayer());
    if (pLocalPlayer != nullptr)
        uiTeam = pLocalPlayer->GetTeam();

    ChatManager.SetPrivateGame(GameClient.GetServerAccess() != ACCESS_PUBLIC);
    ChatManager.SetHost(pLocalPlayer != nullptr && pLocalPlayer->HasFlags(PLAYER_FLAG_HOST));

    switch (m_eCurrentInterface)
    {
    case CG_INTERFACE_INVALID:
        break;

    case CG_INTERFACE_MAIN:
        UIManager.SetActiveInterface(m_sMainInterface);
        UpdateLogin();
        UpdateLobby();
        UpdateChangePassword();
        UpdateReplayInfo();
        break;

    case CG_INTERFACE_LOADING:
        UIManager.SetActiveInterface(_T("game_loading"));
        UpdateLogin();
        UpdateLobby();
        break;

    case CG_INTERFACE_LOBBY:
        UIManager.SetActiveInterface(_T("game_lobby"));
        UpdateLogin();
        UpdateLobby();
        UpdateVoiceChat();
        break;

    case CG_INTERFACE_HERO_SELECT:
        UIManager.SetActiveInterface(_T("game_hero_select"));
        UpdateLogin();
        UpdateHeroSelect();
        UpdateLobby();
        break;

    case CG_INTERFACE_HERO_LOADING:
        UIManager.SetActiveInterface(_T("game_hero_loading"));
        UpdateLogin();
        UpdateHeroSelect();
        break;

    case CG_INTERFACE_GAME:
    case CG_INTERFACE_GAME_SPECTATOR:
        {
            Trigger(UITRIGGER_TIME_OF_DAY, Game.GetTimeOfDay());
            Trigger(UITRIGGER_DAYTIME, !Game.IsNight());
            tsvector vMatchTime(2);
            if (GameClient.GetGamePhase() == GAME_PHASE_PRE_MATCH)
                vMatchTime[0] = XtoA(Game.GetRemainingPhaseTime());
            else if (GameClient.GetGamePhase() == GAME_PHASE_ENDED)
                vMatchTime[0] = XtoA(Game.GetRemainingPhaseTime());
            else
                vMatchTime[0] = XtoA(Game.GetMatchTime());
            vMatchTime[1] = XtoA(GameClient.GetGamePhase() == GAME_PHASE_PRE_MATCH);
            Trigger(UITRIGGER_MATCH_TIME, vMatchTime);

            UpdateLogin();
            UpdateCommander();

            if (m_eCurrentInterface == CG_INTERFACE_GAME_SPECTATOR)
            {
                if (!pLocalPlayer->GetInterface().empty())
                    UIManager.SetActiveInterface(pLocalPlayer->GetInterface());
                else
                    UIManager.SetActiveInterface(_T("game_spectator"));

                if (!pLocalPlayer->GetOverlayInterface().empty())
                {
                    UIManager.AddOverlayInterface(pLocalPlayer->GetOverlayInterface());
                }

                UpdateSpectatorTeams();
                UpdateSpectatorPlayers();
                UpdateSpectatorHeroes();
                UpdateSpectatorSelectedUnits();
                UpdateSpectatorVoiceChat();

                // Shop
                Trigger(UITRIGGER_SHOP_ACTIVE, m_bDisplayShop);
                Trigger(UITRIGGER_PLAYER_CAN_SHOP, true);
                
                if (m_bDisplayShop)
                    UpdateShop();
            }
            else
            {
                if (!pLocalPlayer->GetInterface().empty())
                    UIManager.SetActiveInterface(pLocalPlayer->GetInterface());
                else
                    UIManager.SetActiveInterface(_T("game"));

                if (!pLocalPlayer->GetOverlayInterface().empty())
                {
                    UIManager.AddOverlayInterface(pLocalPlayer->GetOverlayInterface());
                }

                UpdatePlayer();
                UpdateHero();
                UpdateAllies();
                UpdateVoiceChat();
            }

            UpdateSelectedUnits();
            UpdateScores();
            UpdateGameMenu();
            UpdateVote();
        }
        break;

    default:
        Console.Warn << _T("CGameClient::Frame() - Invalid interface") << newl;
        break;
    }

    if (Game.GetGameTime() - m_uiLastBuildingAttackAlertTime > cg_buildingAttackAlertTime)
    {
        tsvector vParams;
        vParams.emplace_back(_T("false"));
        vParams.emplace_back(_T(""));
        Trigger(UITRIGGER_BUILDING_ATTACK_ALERT, vParams);
    }

    Trigger(UITRIGGER_ENDGAME, Game.GetWinningTeam() != TEAM_INVALID);

    if ((m_eCurrentInterface == CG_INTERFACE_GAME || m_eCurrentInterface == CG_INTERFACE_GAME_SPECTATOR) && Game.GetGamePhase() == GAME_PHASE_ENDED)
    {
        UpdateGameOver();
    }

    //if ((m_eCurrentInterface == CG_INTERFACE_GAME || m_eCurrentInterface == CG_INTERFACE_GAME_SPECTATOR) && GameClient.GetShowMenu())
    //{
    //  if (UIManager.AddOverlayInterface(_T("game_menu")))
    //  {
    //      // Ensure replay controls stay in front of the menu
    //      // This is only run when the overlay is first shown.
    //      UIManager.BringOverlayToFront(_T("game_replay_control"));
    //      ForceUpdate();
    //  }

    //  UpdateGameMenu();
    //  UpdateLogin();
    //  UpdateChangePassword();
    //}
    //else
    //{
    //  UIManager.RemoveOverlayInterface(_T("game_menu"));
    //}

    if ((m_eCurrentInterface == CG_INTERFACE_GAME || m_eCurrentInterface == CG_INTERFACE_GAME_SPECTATOR) && GameClient.GetShowMenu())
    {
        if (UIManager.AddOverlayInterface(m_sMainInterface))
        {
            // Ensure replay controls stay in front of the menu
            // This is only run when the overlay is first shown.
            UIManager.BringOverlayToFront(_T("game_replay_control"));
            ForceUpdate();
        }
        
        UpdateGameMenu();
        UpdateVote();
        //UIManager.AddOverlayInterface(m_sMainInterface);
        //UpdateLogin();
        //UpdateChangePassword();
    }
    else
    {
        UIManager.RemoveOverlayInterface(m_sMainInterface);
    }

    if (ReplayManager.IsPlaying() && m_eCurrentInterface > CG_INTERFACE_LOADING)
    {
        Input.SetCursorRecenter(CURSOR_GAME, BOOL_FALSE);
        Input.SetCursorConstrained(CURSOR_GAME, cg_constrainCursor ? BOOL_TRUE : BOOL_FALSE);
        Input.SetCursorHidden(CURSOR_GAME, BOOL_FALSE);
        Input.SetCursorFrozen(CURSOR_GAME, BOOL_FALSE);

        if (UIManager.AddOverlayInterface(_T("game_replay_control")))
        {
            // This is only run when the overlay is first shown.
            ForceUpdate();
        }

        UpdateReplay();
    }
#if 0
    else
    {
        UIManager.ClearOverlayInterface();
    }
#endif

    Trigger(UITRIGGER_TEAM, uiTeam);

    // Connection status
    static tsvector vsConnectionStatus(2);
    vsConnectionStatus[0] = XtoA(GameClient.IsConnected());
    vsConnectionStatus[1] = GameClient.GetStateString(STATE_STRING_SERVER_INFO).GetString(_T("svr_name"));
    Trigger(UITRIGGER_CONNECTION_STATUS, vsConnectionStatus);

    // Following status
    Trigger(UITRIGGER_FOLLOW_STATUS, ChatManager.GetFollowing());

    // Scoreboard changes
    Trigger(UITRIGGER_SCOREBOARD_CHANGE, GetScoreState());

    m_uiLastUpdateSequence = m_uiUpdateSequence;

    // Update date and time
    CDate date(true);
    Trigger(UITRIGGER_SYSTEM_DATE, date.GetDateString(DATE_MONTH_FIRST));
    Trigger(UITRIGGER_SYSTEM_WEEKDAY, date.GetWeekdayString(WEEKDAY_SHORT));
    Trigger(UITRIGGER_SYSTEM_TIME, date.GetTimeString(TIME_NO_SECONDS | (cg_24hourClock ? 0 : TIME_TWELVE_HOUR)));
}


/*====================
  CGameInterfaceManager::UpdateLogin
  ====================*/
void    CGameInterfaceManager::UpdateLogin()
{
    const CClientAccount &account(GameClient.GetAccount());

    static tsvector vLoginStatus(6);
    switch (account.GetStatus())
    {
        case CLIENT_LOGIN_OFFLINE: vLoginStatus[0] = _T("offline"); break;
        case CLIENT_LOGIN_WAITING: vLoginStatus[0] = _T("waiting"); break;
        case CLIENT_LOGIN_SUCCESS: vLoginStatus[0] = _T("success"); break;
        case CLIENT_LOGIN_FAILURE: vLoginStatus[0] = _T("failure"); break;
        case CLIENT_LOGIN_EXPIRED: vLoginStatus[0] = _T("expired"); break;
    }

    vLoginStatus[1] = account.GetStatusDescription();
    vLoginStatus[2] = XtoA(account.IsLoggedIn());
    vLoginStatus[3] = XtoA(account.GetPasswordExpiration());
    vLoginStatus[4] = XtoA(m_bIsLoggedIn != account.IsLoggedIn());
    vLoginStatus[5] = XtoA(uint(account.GetUpdaterStatus()));
    Trigger(UITRIGGER_MAIN_LOGIN_STATUS, vLoginStatus);

    Trigger(UITRIGGER_MAIN_UPDATER_STATUS, uint(account.GetUpdaterStatus()));
    
    m_bIsLoggedIn = account.IsLoggedIn();

    static tsvector vAccountInfo(13);
    vAccountInfo[0] = XtoA(account.GetAccountID());
    vAccountInfo[1] = account.GetNickname();
    vAccountInfo[2] = XtoA(account.GetAccountType());
    vAccountInfo[3] = XtoA(account.GetLevel());
    vAccountInfo[4] = XtoA(account.GetGames());
    vAccountInfo[5] = XtoA(account.GetDisconnects());
    vAccountInfo[6] = XtoA(account.IsLeaver());
    vAccountInfo[7] = XtoA(account.WillBeLeaver());
    vAccountInfo[8] = XtoA(account.GetLeaverPercent());
    vAccountInfo[9] = XtoA(account.GetNextLeaverPercent());
    vAccountInfo[10] = XtoA(account.GetNextLeaverThreshold());
    vAccountInfo[11] = XtoA(account.IsTrialExpired());
    vAccountInfo[12] = XtoA(account.GetNoStatsGames());
    Trigger(UITRIGGER_MAIN_ACCOUNT_INFO, vAccountInfo);
    
    bool bList;
    uint uiProcessed;
    uint uiTotal;
    uint uiResponses;
    uint uiVisible;
    bool bWorking(GameClient.GetGameListStatus(bList, uiProcessed, uiTotal, uiResponses, uiVisible));

    static tsvector vGameListStatus(6);
    vGameListStatus[0] = XtoA(bWorking);
    vGameListStatus[1] = XtoA(bList);
    vGameListStatus[2] = XtoA(uiProcessed);
    vGameListStatus[3] = XtoA(uiTotal);
    vGameListStatus[4] = XtoA(uiResponses);
    vGameListStatus[5] = XtoA(uiVisible);

    Trigger(UITRIGGER_MAIN_GAMELIST_STATUS, vGameListStatus);

#ifdef K2_CLIENT
    Trigger(UITRIGGER_MAIN_LOCAL_SERVER_AVAILABLE, false);
#else
    Trigger(UITRIGGER_MAIN_LOCAL_SERVER_AVAILABLE, true);
#endif

    Trigger(UITRIGGER_MAIN_PLAYERS_ONLINE, GameClient.GetNumPlayersOnline());
    Trigger(UITRIGGER_MAIN_PLAYERS_INGAME, GameClient.GetNumPlayersInGame());

    Trigger(UITRIGGER_MAIN_COUNTDOWN, Host.GetTime());

    Trigger(UITRIGGER_PREVIEW_MAP_NAME, m_sPreviewMapName);
    Trigger(UITRIGGER_PREVIEW_MAP_SIZE, m_iPreviewMapSize);

    if (m_iReplayURLTesting == 0 && !m_sTestReplayURL.empty())
    {
        m_fileTestReplayURL.Open(m_sTestReplayURL, FILE_HTTP_GETSIZE);

        if (m_fileTestReplayURL.IsOpen())
        {
            m_iReplayURLTesting = 1; // Tested

            if (m_fileTestReplayURL.ErrorEncountered())
                m_bReplayURLValid = false;
            else if (m_fileTestReplayURL.GetBufferSize() != m_uiTestReplayURLSize)
                m_bReplayURLValid = false;
            else
                m_bReplayURLValid = true;

            m_fileTestReplayURL.Close();
        }
    }
    else if (m_iReplayURLTesting != 1)
        m_bReplayURLValid = false;

    int iReplayURLStatus(0);

    if (m_iReplayURLTesting == -1)
        iReplayURLStatus = 0;
    else if (m_iReplayURLTesting == 0)
        iReplayURLStatus = 1;
    else if (m_iReplayURLTesting == 1)
    {
        if (m_bReplayURLValid)
            iReplayURLStatus = 2;
        else 
            iReplayURLStatus = 3;
    }
    else if (m_iReplayURLTesting == 2)
        iReplayURLStatus = 4;

    Trigger(UITRIGGER_REPLAY_URL_STATUS, iReplayURLStatus);
    
    CGameInfo *pGameInfo(Game.GetGameInfo());

    // Don't display lobbies in arranged matches
    if (pGameInfo != nullptr && pGameInfo->HasFlags(GAME_FLAG_ARRANGED) && Game.GetGamePhase() == GAME_PHASE_WAITING_FOR_PLAYERS)
        Trigger(UITRIGGER_GAME_PHASE, GAME_PHASE_IDLE);
    else
        Trigger(UITRIGGER_GAME_PHASE, Game.GetGamePhase());

    if (pGameInfo != nullptr && pGameInfo->HasFlags(GAME_FLAG_ARRANGED))
        Trigger(UITRIGGER_TMM_GAME_PHASE, Game.GetGamePhase());
    else
        Trigger(UITRIGGER_TMM_GAME_PHASE, GAME_PHASE_IDLE);
}


/*====================
  CGameInterfaceManager::UpdateChangePassword
  ====================*/
void    CGameInterfaceManager::UpdateChangePassword()
{
    const CClientAccount &account(GameClient.GetAccount());

    static tsvector vChangePasswordStatus(2);
    
    switch (account.GetChangePasswordStatus())
    {
        case CLIENT_CHANGE_PASSWORD_UNUSED: vChangePasswordStatus[0] = _T("unused"); break;
        case CLIENT_CHANGE_PASSWORD_WAITING: vChangePasswordStatus[0] = _T("waiting"); break;
        case CLIENT_CHANGE_PASSWORD_SUCCESS: vChangePasswordStatus[0] = _T("success"); break;
        case CLIENT_CHANGE_PASSWORD_FAILURE: vChangePasswordStatus[0] = _T("failure"); break;
    }
    
    vChangePasswordStatus[1] = account.GetChangePasswordStatusDescription();
    Trigger(UITRIGGER_MAIN_CHANGE_PASSWORD_STATUS, vChangePasswordStatus);  
}


/*====================
  CGameInterfaceManager::UpdateAccountInfo
  ====================*/
void    CGameInterfaceManager::UpdateAccountInfo()
{
}


/*====================
  CGameInterfaceManager::UpdateGameOver
  ====================*/
void    CGameInterfaceManager::UpdateGameOver()
{
    PROFILE("CGameInterfaceManager::UpdateGameOver");

    Trigger(UITRIGGER_ENDGAME_INTERFACE_DISPLAY, GameClient.GetGameTime() > GameClient.GetPhaseStartTime() && (GameClient.GetGameTime() - GameClient.GetPhaseStartTime()) >= cg_endGameInterfaceDelay);

    // Timer
    tsvector vTimer(2);
    vTimer[0] = XtoA(Game.GetRemainingPhaseTime());
    vTimer[1] = XtoA(Game.GetPhaseDuration());
    Trigger(UITRIGGER_ENDGAME_TIMER, vTimer);

    // Match info
    tsvector vMatchInfo(6);
    CGameInfo *pGameInfo(GameClient.GetGameInfo());
    if (pGameInfo == nullptr)
    {
        for (tsvector_it it(vMatchInfo.begin()); it != vMatchInfo.end(); ++it)
            it->clear();
    }
    else
    {
        vMatchInfo[0] = XtoA(pGameInfo->GetMatchID());
        vMatchInfo[1] = pGameInfo->GetServerDate();
        vMatchInfo[2] = pGameInfo->GetServerTime();
        vMatchInfo[3] = pGameInfo->GetServerName();
        vMatchInfo[4] = XtoA(pGameInfo->GetMatchLength());
        vMatchInfo[5] = XtoA(GameClient.GetWinningTeam());
    }
    Trigger(UITRIGGER_ENDGAME_MATCH_INFO, vMatchInfo);

    // Team stats
    for (uint uiTeam(1); uiTeam <= 2; ++uiTeam)
    {
        static tsvector vTeamStats(27);
        for (tsvector_it it(vTeamStats.begin()); it != vTeamStats.end(); ++it)
            it->clear();

        CTeamInfo *pTeam(GameClient.GetTeam(uiTeam));
        if (pTeam == nullptr)
            continue;

        vTeamStats[0] = XtoA(pGameInfo == nullptr ? 0 : pGameInfo->GetMatchLength());  // Time
        vTeamStats[1] = XtoA(pTeam->GetTotalExperience());                          // Experience
        vTeamStats[2] = XtoA(pTeam->GetTotalDeaths());                              // Deaths
        vTeamStats[3] = XtoA(pTeam->GetTotalHeroKills());                           // Hero Kills
        vTeamStats[4] = XtoA(pTeam->GetTotalHeroDamage());                          // Hero Damage
        vTeamStats[5] = XtoA(pTeam->GetTotalHeroAssists());                         // Hero Assists
        vTeamStats[6] = XtoA(pTeam->GetTotalHeroBounty());                          // Hero Bounty
        vTeamStats[7] = XtoA(pTeam->GetTotalCreepKills());                          // Creep Kills
        vTeamStats[8] = XtoA(pTeam->GetTotalCreepDamage());                         // Creep Damage
        vTeamStats[9] = XtoA(pTeam->GetTotalCreepBounty());                         // Creep Bounty
        vTeamStats[10] = XtoA(pTeam->GetTotalDenies());                             // Creep Denies
        vTeamStats[11] = XtoA(pTeam->GetTotalNeutralKills());                       // Neutral Kills
        vTeamStats[12] = XtoA(pTeam->GetTotalNeutralDamage());                      // Neutral Damage
        vTeamStats[13] = XtoA(pTeam->GetTotalNeutralBounty());                      // Neutral Bounty
        vTeamStats[14] = XtoA(pTeam->GetTotalBuildingKills());                      // Building Kills
        vTeamStats[15] = XtoA(pTeam->GetTotalBuildingDamage());                     // Building Damage
        vTeamStats[16] = XtoA(pTeam->GetTotalBuildingBounty());                     // Building Bounty
        vTeamStats[17] = XtoA(pTeam->GetTotalGoldEarned());                         // Gold earned
        vTeamStats[18] = XtoA(pTeam->GetTotalGoldLost());                           // Gold lost
        vTeamStats[19] = XtoA(pTeam->GetTotalGoldSpent());                          // Gold spent
        vTeamStats[20] = XtoA(pTeam->GetTotalActionCount());                        // APM
        vTeamStats[21] = XtoA(pTeam->GetTotalBuyBacks());                           // Buy backs
        vTeamStats[22] = XtoA(pTeam->GetTotalDeniedExperience());                   // Denied Experience
        vTeamStats[23] = XtoA(pTeam->GetTotalHeroExperience());                     // Experience from hero kills
        vTeamStats[24] = XtoA(pTeam->GetTotalCreepExperience());                    // Experience creep kills
        vTeamStats[25] = XtoA(pTeam->GetTotalNeutralExperience());                  // Experience neutral kills
        vTeamStats[26] = XtoA(pTeam->GetTotalBuildingExperience());                 // Experience building kills
        Trigger(UITRIGGER_ENDGAME_TEAM_STATS, vTeamStats, uiTeam);

        // Player stats
        for (uint uiPlayerTeamIndex(0); uiPlayerTeamIndex < MAX_DISPLAY_PLAYERSPERTEAM; ++uiPlayerTeamIndex)
        {
            static tsvector vPlayerStats(12);
            for (tsvector_it it(vPlayerStats.begin()); it != vPlayerStats.end(); ++it)
                it->clear();
            vPlayerStats[0] = _T("-1");

            CPlayer *pPlayer(pTeam->GetPlayer(uiPlayerTeamIndex));
            if (pPlayer != nullptr)
            {
                vPlayerStats[0] = XtoA(pPlayer->GetClientNumber());             // Client number
                vPlayerStats[1] = XtoA(pPlayer == GameClient.GetLocalPlayer()); // Is local player
                vPlayerStats[2] = pPlayer->GetName();                           // Name

                if (pPlayer->HasSelectedHero())
                {
                    CHeroDefinition *pHeroDef(EntityRegistry.GetDefinition<CHeroDefinition>(pPlayer->GetSelectedHero()));
                    IHeroEntity *pHero(pPlayer->GetHero());
                    vPlayerStats[3] = pHeroDef ? pHeroDef->GetDisplayName() : TSNULL;   // Hero name
                    vPlayerStats[4] = pHeroDef ? pHeroDef->GetIconPath(0) : TSNULL;     // Hero icon
                    vPlayerStats[5] = pHero ? XtoA(pHero->GetLevel()) : TSNULL;         // Level
                }

                CGameStats *pStats(pPlayer->GetStats());
                if (pStats != nullptr)
                {
                    vPlayerStats[6] = XtoA(pStats->GetHeroKills());     // Hero kills
                    vPlayerStats[7] = XtoA(pStats->GetDeaths());        // Deaths
                    vPlayerStats[8] = XtoA(pStats->GetCreepKills());    // Creep kills
                    vPlayerStats[9] = XtoA(pStats->GetDenies());        // Denies
                    vPlayerStats[10] = XtoA(pStats->GetExperience());   // Experience
                    vPlayerStats[11] = XtoA(pStats->GetTimePlayed());   // Time
                }
            }

            Trigger(UITRIGGER_ENDGAME_PLAYER_STATS, vPlayerStats, uiPlayerTeamIndex + (uiTeam - 1) * MAX_DISPLAY_PLAYERSPERTEAM);
        }
    }
}


/*====================
  CGameInterfaceManager::UpdateGameMenu
  ====================*/
void    CGameInterfaceManager::UpdateGameMenu()
{
    PROFILE("CGameInterfaceManager::UpdateGameMenu");

    static tsvector vPlayerInfo(18);

    CPlayer *pLocalPlayer(Game.GetLocalPlayer());
    if (pLocalPlayer == nullptr)
        return;

    Trigger(UITRIGGER_CAN_LEAVE, Game.CanLeave(pLocalPlayer->GetTeam()));

    for (int iTeam(1); iTeam <= 2; ++iTeam)
    {
        CTeamInfo *pTeam(Game.GetTeam(iTeam));
        if (pTeam != nullptr)
        {
            for (uint uiTeamIndex(0); uiTeamIndex < MAX_DISPLAY_PLAYERSPERTEAM; ++uiTeamIndex)
            {
                CPlayer *pPlayer(pTeam->GetPlayer(uiTeamIndex));
                if (pPlayer == nullptr)
                {
                    vPlayerInfo[0] = _T("-1");
                    Trigger(UITRIGGER_MENU_PLAYER_INFO, vPlayerInfo, ((iTeam - 1) * MAX_DISPLAY_PLAYERSPERTEAM) + uiTeamIndex);
                    continue;
                }

                uint uiTimeUntilKickAllowed = 0;
                if (pPlayer->GetLastVoteKickTime() != INVALID_TIME && pPlayer->GetLastVoteKickTime() + g_voteKickCooldownTime >= Game.GetGameTime())
                    uiTimeUntilKickAllowed = (pPlayer->GetLastVoteKickTime() + g_voteKickCooldownTime) - Game.GetGameTime();

                bool bIsAlly(pPlayer->GetTeam() == pLocalPlayer->GetTeam());
                IHeroEntity *pHero(pPlayer->GetHero());
                vPlayerInfo[0] = XtoA(pPlayer->GetClientNumber());                                                      // Client number
                vPlayerInfo[1] = GetInlineColorString<tstring>(pPlayer->GetColor());                                    // Color
                vPlayerInfo[2] = pPlayer->GetName();                                                                    // Name
                vPlayerInfo[3] = XtoA(bIsAlly);                                                                         // Is ally
                vPlayerInfo[4] = XtoA(bIsAlly && pLocalPlayer->HasSharedFullControl(pPlayer->GetClientNumber()));       // Sharing full control
                vPlayerInfo[5] = XtoA(bIsAlly && pLocalPlayer->HasSharedPartialControl(pPlayer->GetClientNumber()));    // Sharing partial control
                vPlayerInfo[6] = pHero ? pHero->GetIconPath() : TSNULL;                                                 // Hero icon
                vPlayerInfo[7] = pPlayer->GetTerminationTime();                                                         // Termination time
                vPlayerInfo[8] = XtoA(pPlayer->IsDisconnected());                                                       // Is disconnected
                vPlayerInfo[9] = XtoA(pPlayer->HasFlags(PLAYER_FLAG_TERMINATED));                                       // Is terminated
                vPlayerInfo[10] = XtoA(pPlayer->GetClientNumber() == pLocalPlayer->GetClientNumber());                  // Is local client
                vPlayerInfo[11] = XtoA(pPlayer->GetAccountID());                                                        // Account ID
                vPlayerInfo[12] = XtoA(pPlayer->GetVote());                                                             // Vote
                vPlayerInfo[13] = XtoA(pPlayer->HasFlags(PLAYER_FLAG_LOADING) ? pPlayer->GetLoadingProgress() : 1.0f);  // Loading progress
                vPlayerInfo[14] = XtoA(pHero ? pHero->GetDisplayName() : TSNULL);                                       // Hero name
                vPlayerInfo[15] = XtoA(pPlayer->HasFlags(PLAYER_FLAG_HOST));                                            // Is host
                vPlayerInfo[16] = XtoA(pPlayer->HasFlags(PLAYER_FLAG_IS_AFK));                                          // Is AFK
                vPlayerInfo[17] = XtoA(uiTimeUntilKickAllowed);                                                         // Kick cooldown
                Trigger(UITRIGGER_MENU_PLAYER_INFO, vPlayerInfo, ((iTeam - 1) * MAX_DISPLAY_PLAYERSPERTEAM) + uiTeamIndex);
            }
        }
    }
}


/*====================
  CGameInterfaceManager::BuildText
  ====================*/
void    CGameInterfaceManager::BuildText(const tstring &sIn, uint uiIndex, tstring &sOut)
{
    sOut.clear();

    for (tstring::const_iterator it(sIn.begin()), itEnd(sIn.end()); it != itEnd; ++it)
    {
        if (*it == _T('{'))
        {
            tstring sToken;

            if (it != itEnd)
                ++it;
            while (it != itEnd && *it != _T('}'))
            {
                sToken += *it;
                ++it;
            }

            if (!sToken.empty())
            {
                tsvector vTokens(TokenizeString(sToken, _T(',')));
                if (!vTokens.empty())
                    sOut += vTokens[MIN<uint>(uiIndex, uint(vTokens.size()) - 1)];
            }

            continue;
        }

        sOut.push_back(*it);
    }
}


/*====================
  CGameInterfaceManager::BuildMultiLevelText
  ====================*/
void    CGameInterfaceManager::BuildMultiLevelText(const tstring &sIn, uint uiMarkIndex, uint uiMaxIndex, tstring &sOut)
{
    sOut.clear();

    for (tstring::const_iterator it(sIn.begin()), itEnd(sIn.end()); it != itEnd; ++it)
    {
        if (*it == _T('{'))
        {
            tstring sToken;

            if (it != itEnd)
                ++it;
            while (it != itEnd && *it != _T('}'))
            {
                sToken += *it;
                ++it;
            }

            if (!sToken.empty())
            {
                tsvector vTokens(TokenizeString(sToken, _T(',')));

                if (vTokens.size() == 1)
                {
                    sOut += vTokens[0];
                }
                else
                {
                    if (uiMarkIndex >= 1)
                        sOut += _T("^v") + vTokens[0] + _T("^*");
                    else
                        sOut += vTokens[0];

                    for (uint uiIndex(1); uiIndex <= uiMaxIndex; ++uiIndex)
                    {
                        sOut += _T('/');

                        if (uiMarkIndex >= uiIndex + 1)
                            sOut += _T("^v") + vTokens[MIN<uint>(uiIndex, uint(vTokens.size()) - 1)] + _T("^*");
                        else
                            sOut += vTokens[MIN<uint>(uiIndex, uint(vTokens.size()) - 1)];
                    }
                }
            }

            continue;
        }

        sOut.push_back(*it);
    }
}


/*====================
  CGameInterfaceManager::BuildBonusesString
  ====================*/
void    CGameInterfaceManager::BuildBonusesString(ISlaveEntity *pSlave, tstring &sStr, int &iLines)
{
    sStr.clear();
    iLines = 0;

    static tsmapts s_mapTokens;

    #define ADD_FLOAT_BONUS(property, iMinPrecision, iMaxPrecision, label, scale, conversion) \
    { \
        float f##property(pSlave->Get##property()); \
        if (f##property != 0.0f) \
        { \
            s_mapTokens.clear(); \
            s_mapTokens[_CTS("value")] = (f##property >= 0.0f ? _T("+") : TSNULL) + XtoA(conversion(f##property * scale), 0, 0, iMinPrecision, iMaxPrecision); \
            sStr += GameClient.GetGameMessage(label, s_mapTokens) + _CTS("\n"); \
            ++iLines; \
        } \
    }

    #define ADD_FLOAT_BONUS_NOPREFIX(property, iMinPrecision, iMaxPrecision, label, scale, conversion) \
    { \
        float f##property(pSlave->Get##property()); \
        if (f##property != 0.0f) \
        { \
            s_mapTokens.clear(); \
            s_mapTokens[_CTS("value")] = XtoA(conversion(f##property * scale), 0, 0, iMinPrecision, iMaxPrecision); \
            sStr += GameClient.GetGameMessage(label, s_mapTokens) + _CTS("\n"); \
            ++iLines; \
        } \
    }

    #define ADD_FLOAT_BONUS_PAIR(property1, iMinPrecision1, iMaxPrecision1, label, scale1, conversion1, property2, iMinPrecision2, iMaxPrecision2, scale2, conversion2) \
    { \
        float f##property1(pSlave->Get##property1()); \
        float f##property2(pSlave->Get##property2()); \
        if (f##property1 != 0.0f) \
        { \
            s_mapTokens.clear(); \
            s_mapTokens[_CTS("value1")] = XtoA(conversion1(f##property1 * scale1), 0, 0, iMinPrecision1, iMaxPrecision1); \
            s_mapTokens[_CTS("value2")] = XtoA(conversion2(f##property2 * scale2), 0, 0, iMinPrecision2, iMaxPrecision2); \
            sStr += GameClient.GetGameMessage(label, s_mapTokens) + _CTS("\n"); \
            ++iLines; \
        } \
    }

    #define ADD_BOOL_BONUS(property, label) \
    { \
        bool b##property(pSlave->Get##property()); \
        if (b##property) \
        { \
            sStr += GameClient.GetGameMessage(label) + _CTS("\n"); \
            ++iLines; \
        } \
    }

    ADD_FLOAT_BONUS(Strength, 0, 0, _CTS("str_bonus"), 1.0f, floor);
    ADD_FLOAT_BONUS(Agility, 0, 0, _CTS("agi_bonus"), 1.0f, floor);
    ADD_FLOAT_BONUS(Intelligence, 0, 0, _CTS("int_bonus"), 1.0f, floor);
    ADD_FLOAT_BONUS(MaxHealth, 0, 0, _CTS("max_health_bonus"), 1.0f, floor);
    ADD_FLOAT_BONUS(MaxHealthMultiplier, 0, 0, _CTS("max_health_mult_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(MaxMana, 0, 0, _CTS("max_mana_bonus"), 1.0f, floor);
    ADD_FLOAT_BONUS(MaxManaMultiplier, 0, 0, _CTS("max_mana_mult_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(BaseDamageMultiplier, 0, 0, _CTS("base_damage_mult_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(Damage, 0, 0, _CTS("damage_bonus"), 1.0f, floor);
    ADD_FLOAT_BONUS(TotalDamageMultiplier, 0, 0, _CTS("damage_mult_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(MoveSpeed, 0, 0, _CTS("move_speed_bonus"), 1.0f, floor);
    ADD_FLOAT_BONUS(MoveSpeedMultiplier, 0, 2, _CTS("move_speed_mult_bonus"), 100.0f, float);
    ADD_FLOAT_BONUS(SlowResistance, 0, 0, _CTS("slow_resistance_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS_NOPREFIX(MoveSpeedSlow, 0, 2, _CTS("move_speed_slow_bonus"), 100.0f, float);
    ADD_FLOAT_BONUS(AttackSpeed, 0, 0, _CTS("attack_speed_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(AttackSpeedMultiplier, 0, 0, _CTS("attack_speed_mult_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS_NOPREFIX(AttackSpeedSlow, 0, 0, _CTS("attack_speed_slow_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(CastSpeed, 0, 0, _CTS("cast_speed_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(CooldownSpeed, 0, 0, _CTS("cooldown_speed_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS_NOPREFIX(ReducedCooldowns, 0, 0, _CTS("reduced_cooldowns_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS_NOPREFIX(IncreasedCooldowns, 0, 0, _CTS("increased_cooldowns_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(Armor, 0, 2, _CTS("armor_bonus"), 1.0f, float);
    ADD_FLOAT_BONUS(MagicArmor, 0, 2, _CTS("magic_armor_bonus"), 1.0f, float);
    ADD_FLOAT_BONUS(HealthRegenPercent, 0, 2, _CTS("health_regen_percent_bonus"), 100.0f, float);
    ADD_FLOAT_BONUS(HealthRegen, 0, 2, _CTS("health_regen_bonus"), 1.0f, float);
    ADD_FLOAT_BONUS(HealthRegenMultiplier, 0, 0, _CTS("health_regen_mult_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(ManaRegenPercent, 0, 2, _CTS("mana_regen_percent_bonus"), 100.0f, float);
    ADD_FLOAT_BONUS(ManaRegen, 0, 2, _CTS("mana_regen_bonus"), 1.0f, float);
    ADD_FLOAT_BONUS(ManaRegenMultiplier, 0, 0, _CTS("mana_regen_mult_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS_PAIR(
        DeflectionChance, 0, 0, _CTS("deflection_bonus"), 100.0f, ROUND,
        Deflection, 0, 0, 1.0f, floor);

    if (pSlave->GetEvasionRanged() != 0.0f || pSlave->GetEvasionMelee() != 0.0f)
    {
        s_mapTokens.clear();
        if (pSlave->GetEvasionRanged() == pSlave->GetEvasionMelee())
        {
            s_mapTokens[_CTS("value")] = XtoA(INT_ROUND(pSlave->GetEvasionRanged() * 100.0f));
            sStr += GameClient.GetGameMessage(_CTS("evasion_bonus"), s_mapTokens) + _CTS("\n"); \
            ++iLines;
        }
        else
        {
            if (pSlave->GetEvasionMelee() != 0.0f)
            {
                s_mapTokens[_CTS("value")] = XtoA(INT_ROUND(pSlave->GetEvasionMelee() * 100.0f));
                sStr += GameClient.GetGameMessage(_CTS("melee_evasion_bonus"), s_mapTokens) + _CTS("\n"); \
                ++iLines;
            }

            if (pSlave->GetEvasionRanged() != 0.0f)
            {
                s_mapTokens[_CTS("value")] = XtoA(INT_ROUND(pSlave->GetEvasionRanged() * 100.0f));
                sStr += GameClient.GetGameMessage(_CTS("ranged_evasion_bonus"), s_mapTokens) + _CTS("\n"); \
                ++iLines;
            }
        }
    }

    ADD_FLOAT_BONUS_NOPREFIX(MissChance, 0, 0, _CTS("miss_chance_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(LifeSteal, 0, 0, _CTS("lifesteal_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS_PAIR(
        CriticalChance, 0, 0, _CTS("critical_bonus"), 100.0f, ROUND,
        CriticalMultiplier, 1, 2, 1.0f, float);
    ADD_FLOAT_BONUS(IncomingDamageMultiplier, 0, 0, _CTS("incoming_damage_mult_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(DebuffDurationMultiplier, 0, 0, _CTS("debuff_duration_mult_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(HealMultiplier, 0, 0, _CTS("heal_mult_bonus"), 100.0f, ROUND);

    ADD_BOOL_BONUS(Stunned, _CTS("stunned_bonus"));
    ADD_BOOL_BONUS(Silenced, _CTS("silenced_bonus"));
    ADD_BOOL_BONUS(Perplexed, _CTS("perplexed_bonus"));
    ADD_BOOL_BONUS(Disarmed, _CTS("disarmed_bonus"));
    ADD_BOOL_BONUS(Immobilized, _CTS("immobilized_bonus"));
    ADD_BOOL_BONUS(Immobilized2, _CTS("immobilized_bonus"));
    ADD_BOOL_BONUS(Restrained, _CTS("restrained_bonus"));
    ADD_BOOL_BONUS(Sighted, _CTS("sighted_bonus"));
    ADD_BOOL_BONUS(Revealed, _CTS("revealed_bonus"));
    
    if (pSlave->GetRevealType() != 0)
    {
        s_mapTokens.clear();
        s_mapTokens[_CTS("value")] = XtoA(INT_ROUND(pSlave->GetRevealRange()));
        sStr += GameClient.GetGameMessage(_CTS("reveal_bonus"), s_mapTokens) + _CTS("\n"); \
        ++iLines;
    }

    if (pSlave->GetStealthType() != 0)
    {
        s_mapTokens.clear();
        s_mapTokens[_CTS("value")] = XtoA(MsToSec(pSlave->GetFadeTime()), 0, 0, 0, 2);
        sStr += GameClient.GetGameMessage(_CTS("stealth_bonus"), s_mapTokens) + _CTS("\n"); \
        ++iLines;
    }

    ADD_BOOL_BONUS(Unitwalking, _CTS("unitwalking_bonus"));
    ADD_BOOL_BONUS(Treewalking, _CTS("treewalking_bonus"));
    ADD_BOOL_BONUS(Cliffwalking, _CTS("cliffwalking_bonus"));
    ADD_BOOL_BONUS(Buildingwalking, _CTS("buildingwalking_bonus"));

    if (pSlave->GetImmunityType() != 0)
    {
        s_mapTokens.clear();
        s_mapTokens[_CTS("type")] = Game.GetEffectTypeString(pSlave->GetImmunityType());
        sStr += GameClient.GetGameMessage(_CTS("immunity_bonus"), s_mapTokens) + _CTS("\n"); \
        ++iLines;
    }

    ADD_BOOL_BONUS(Invulnerable, _CTS("invulnerable_bonus"));

    if (pSlave->IsState())
    {
        IEntityState *pState(pSlave->GetAsState());

        if (pState->GetDispelOnDamage())
        {
            sStr += GameClient.GetGameMessage(_CTS("dispel_on_damage_bonus")) + _CTS("\n");
            ++iLines;
        }
        if (pState->GetDispelOnAction())
        {
            sStr += GameClient.GetGameMessage(_CTS("dispel_on_action_bonus")) + _CTS("\n");
            ++iLines;
        }
    }

    ADD_BOOL_BONUS(TrueStrike, _CTS("truestrike_bonus"));

    ADD_FLOAT_BONUS_NOPREFIX(HealthRegenReduction, 0, 0, _CTS("health_regen_reduction_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS_NOPREFIX(ManaRegenReduction, 0, 0, _CTS("mana_regen_reduction_bonus"), 100.0f, ROUND);

    tstring sOnFrame;
    BuildText(pSlave->GetEffectDescription(ACTION_SCRIPT_FRAME), MAX(1u, pSlave->GetLevel()) - 1, sOnFrame);

    if (!sOnFrame.empty())
    {
        sStr += sOnFrame + _CTS("\n");
        ++iLines;
    }

    #undef ADD_FLOAT_BONUS
    #undef ADD_FLOAT_BONUS_NOPREFIX
    #undef ADD_FLOAT_BONUS_PAIR
    #undef ADD_BOOL_BONUS
}


/*====================
  CGameInterfaceManager::BuildBonusesString
  ====================*/
void    CGameInterfaceManager::BuildBonusesString(CStateDefinition *pDefinition, uint uiLevel, uivector &vModifierKeys, tstring &sStr, int &iLines)
{
    // Find and activate all exclusive and conditional modifiers
    uint uiModifier(0);

    uiModifier |= pDefinition->GetModifierBits(vModifierKeys);

    // Search this entity
    const EntityModifierMap &mapModifiers(pDefinition->GetModifiers());
    for (EntityModifierMap::const_iterator cit(mapModifiers.begin()), citEnd(mapModifiers.end()); cit != citEnd; ++cit)
    {
        IEntityDefinition *pModifier(cit->second);

        if (!pModifier->GetExclusive() && pModifier->GetCondition().empty())
            continue;

        uiModifier |= cit->first;
    }

    IEntityDefinition *pModifiedDefinition(pDefinition->GetModifiedDefinition(uiModifier));
    if (pModifiedDefinition != nullptr)
        pDefinition = static_cast<CStateDefinition *>(pModifiedDefinition);

    sStr.clear();
    iLines = 0;

    uint uiIndex(MAX(1u, uiLevel) - 1);

    static tsmapts s_mapTokens;

    #define ADD_FLOAT_BONUS(property, iMinPrecision, iMaxPrecision, label, scale, conversion) \
    { \
        float f##property(pDefinition->Get##property(uiIndex)); \
        if (f##property != 0.0f) \
        { \
            s_mapTokens.clear(); \
            s_mapTokens[_CTS("value")] = (f##property >= 0.0f ? _CTS("+") : TSNULL) + XtoA(conversion(f##property * scale), 0, 0, iMinPrecision, iMaxPrecision); \
            sStr += GameClient.GetGameMessage(label, s_mapTokens) + _CTS("\n"); \
            ++iLines; \
        } \
    }

    #define ADD_FLOAT_BONUS_NOPREFIX(property, iMinPrecision, iMaxPrecision, label, scale, conversion) \
    { \
        float f##property(pDefinition->Get##property(uiIndex)); \
        if (f##property != 0.0f) \
        { \
            s_mapTokens.clear(); \
            s_mapTokens[_CTS("value")] = XtoA(conversion(f##property * scale), 0, 0, iMinPrecision, iMaxPrecision); \
            sStr += GameClient.GetGameMessage(label, s_mapTokens) + _CTS("\n"); \
            ++iLines; \
        } \
    }

    #define ADD_FLOAT_BONUS_PAIR(property1, iMinPrecision1, iMaxPrecision1, label, scale1, conversion1, property2, iMinPrecision2, iMaxPrecision2, scale2, conversion2) \
    { \
        float f##property1(pDefinition->Get##property1(uiIndex)); \
        float f##property2(pDefinition->Get##property2(uiIndex)); \
        if (f##property1 != 0.0f) \
        { \
            s_mapTokens.clear(); \
            s_mapTokens[_CTS("value1")] = XtoA(conversion1(f##property1 * scale1), 0, 0, iMinPrecision1, iMaxPrecision1); \
            s_mapTokens[_CTS("value2")] = XtoA(conversion2(f##property2 * scale2), 0, 0, iMinPrecision2, iMaxPrecision2); \
            sStr += GameClient.GetGameMessage(label, s_mapTokens) + _CTS("\n"); \
            ++iLines; \
        } \
    }

    #define ADD_BOOL_BONUS(property, label) \
    { \
        bool b##property(pDefinition->Get##property(uiIndex)); \
        if (b##property) \
        { \
            sStr += GameClient.GetGameMessage(label) + _CTS("\n"); \
            ++iLines; \
        } \
    }

    #define ADD_FLOAT_PROGRESSIVE_BONUS(property, iMinPrecision, iMaxPrecision, label, label2, scale, conversion) \
    { \
        s_mapTokens.clear(); \
        float f##property(pDefinition->Get##property(uiIndex)); \
        if (f##property != 0.0f) \
        { \
            s_mapTokens[_CTS("value")] = (f##property >= 0.0f ? _CTS("+") : TSNULL) + XtoA(conversion(f##property * scale), 0, 0, iMinPrecision, iMaxPrecision); \
            sStr += GameClient.GetGameMessage(label, s_mapTokens) + _CTS("\n"); \
            ++iLines; \
        } \
        float f##property##PerCharge(pDefinition->Get##property##PerCharge(uiIndex)); \
        if (f##property##PerCharge != 0.0f) \
        { \
            s_mapTokens[_CTS("value_per_charge")] = (f##property##PerCharge >= 0.0f ? _CTS("+") : TSNULL) + XtoA(conversion(f##property##PerCharge * scale), 0, 0, iMinPrecision, iMaxPrecision); \
            sStr += GameClient.GetGameMessage(label2, s_mapTokens) + _CTS("\n"); \
            ++iLines; \
        } \
    }

    ADD_FLOAT_PROGRESSIVE_BONUS(Strength, 0, 0, _CTS("str_bonus"), _CTS("str_bonus_percharge"), 1.0f, floor);
    ADD_FLOAT_PROGRESSIVE_BONUS(Agility, 0, 0, _CTS("agi_bonus"), _CTS("agi_bonus_percharge"), 1.0f, floor);
    ADD_FLOAT_PROGRESSIVE_BONUS(Intelligence, 0, 0, _CTS("int_bonus"), _CTS("int_bonus_percharge"), 1.0f, floor);
    ADD_FLOAT_BONUS(MaxHealth, 0, 0, _CTS("max_health_bonus"), 1.0f, floor);
    ADD_FLOAT_BONUS(MaxHealthMultiplier, 0, 0, _CTS("max_health_mult_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(MaxMana, 0, 0, _CTS("max_mana_bonus"), 1.0f, floor);
    ADD_FLOAT_BONUS(MaxManaMultiplier, 0, 0, _CTS("max_mana_mult_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(BaseDamageMultiplier, 0, 0, _CTS("base_damage_mult_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(Damage, 0, 0, _CTS("damage_bonus"), 1.0f, floor);
    ADD_FLOAT_BONUS(DamagePerCharge, 0, 0, _CTS("damage_bonus_per_charge"), 1.0f, floor);
    ADD_FLOAT_BONUS(TotalDamageMultiplier, 0, 0, _CTS("damage_mult_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(MoveSpeed, 0, 0, _CTS("move_speed_bonus"), 1.0f, floor);
    ADD_FLOAT_BONUS(MoveSpeedMultiplier, 0, 2, _CTS("move_speed_mult_bonus"), 100.0f, float);
    ADD_FLOAT_BONUS(MoveSpeedMultiplierPerCharge, 0, 2, _CTS("move_speed_mult_bonus_per_charge"), 100.0f, float);
    ADD_FLOAT_BONUS(SlowResistance, 0, 0, _CTS("slow_resistance_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS_NOPREFIX(MoveSpeedSlow, 0, 2, _CTS("move_speed_slow_bonus"), 100.0f, float);
    ADD_FLOAT_BONUS_NOPREFIX(MoveSpeedSlowPerCharge, 0, 2, _CTS("move_speed_slow_bonus_per_charge"), 100.0f, float);
    ADD_FLOAT_BONUS(AttackSpeed, 0, 0, _CTS("attack_speed_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(AttackSpeedPerCharge, 0, 0, _CTS("attack_speed_bonus_per_charge"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(AttackSpeedMultiplier, 0, 0, _CTS("attack_speed_mult_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS_NOPREFIX(AttackSpeedSlow, 0, 0, _CTS("attack_speed_slow_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(CastSpeed, 0, 0, _CTS("cast_speed_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(CooldownSpeed, 0, 0, _CTS("cooldown_speed_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS_NOPREFIX(ReducedCooldowns, 0, 0, _CTS("reduced_cooldowns_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS_NOPREFIX(IncreasedCooldowns, 0, 0, _CTS("increased_cooldowns_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(Armor, 0, 2, _CTS("armor_bonus"), 1.0f, float);
    ADD_FLOAT_BONUS(ArmorPerCharge, 0, 2, _CTS("armor_bonus_per_charge"), 1.0f, float);
    ADD_FLOAT_BONUS(MagicArmor, 0, 2, _CTS("magic_armor_bonus"), 1.0f, float);
    ADD_FLOAT_BONUS(MagicArmorPerCharge, 0, 2, _CTS("magic_armor_bonus_per_charge"), 1.0f, float);
    ADD_FLOAT_BONUS(HealthRegenPercent, 0, 2, _CTS("health_regen_percent_bonus"), 100.0f, float);
    ADD_FLOAT_BONUS(HealthRegen, 0, 2, _CTS("health_regen_bonus"), 1.0f, float);
    ADD_FLOAT_BONUS(HealthRegenMultiplier, 0, 0, _CTS("health_regen_mult_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(ManaRegenPercent, 0, 2, _CTS("mana_regen_percent_bonus"), 100.0f, float);
    ADD_FLOAT_BONUS(ManaRegen, 0, 2, _CTS("mana_regen_bonus"), 1.0f, float);
    ADD_FLOAT_BONUS(ManaRegenMultiplier, 0, 0, _CTS("mana_regen_mult_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS_PAIR(
        DeflectionChance, 0, 0, _CTS("deflection_bonus"), 100.0f, ROUND,
        Deflection, 0, 0, 1.0f, floor);

    if (pDefinition->GetEvasionRanged(uiIndex) != 0.0f || pDefinition->GetEvasionMelee(uiIndex) != 0.0f)
    {
        s_mapTokens.clear();
        if (pDefinition->GetEvasionRanged(uiIndex) == pDefinition->GetEvasionMelee(uiIndex))
        {
            s_mapTokens[_CTS("value")] = XtoA(INT_ROUND(pDefinition->GetEvasionRanged(uiIndex) * 100.0f));
            sStr += GameClient.GetGameMessage(_CTS("evasion_bonus"), s_mapTokens) + _CTS("\n"); \
            ++iLines;
        }
        else
        {
            if (pDefinition->GetEvasionMelee(uiIndex) != 0.0f)
            {
                s_mapTokens[_CTS("value")] = XtoA(INT_ROUND(pDefinition->GetEvasionMelee(uiIndex) * 100.0f));
                sStr += GameClient.GetGameMessage(_CTS("melee_evasion_bonus"), s_mapTokens) + _CTS("\n"); \
                ++iLines;
            }

            if (pDefinition->GetEvasionRanged(uiIndex) != 0.0f)
            {
                s_mapTokens[_CTS("value")] = XtoA(INT_ROUND(pDefinition->GetEvasionRanged(uiIndex) * 100.0f));
                sStr += GameClient.GetGameMessage(_CTS("ranged_evasion_bonus"), s_mapTokens) + _CTS("\n"); \
                ++iLines;
            }
        }
    }

    ADD_FLOAT_BONUS_NOPREFIX(MissChance, 0, 0, _CTS("miss_chance_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(LifeSteal, 0, 0, _CTS("lifesteal_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS_PAIR(
        CriticalChance, 0, 0, _CTS("critical_bonus"), 100.0f, ROUND,
        CriticalMultiplier, 1, 2, 1.0f, float);
    ADD_FLOAT_BONUS(IncomingDamageMultiplier, 0, 0, _CTS("incoming_damage_mult_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(DebuffDurationMultiplier, 0, 0, _CTS("debuff_duration_mult_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(HealMultiplier, 0, 0, _CTS("heal_mult_bonus"), 100.0f, ROUND);

    ADD_BOOL_BONUS(Stunned, _CTS("stunned_bonus"));
    ADD_BOOL_BONUS(Silenced, _CTS("silenced_bonus"));
    ADD_BOOL_BONUS(Perplexed, _CTS("perplexed_bonus"));
    ADD_BOOL_BONUS(Disarmed, _CTS("disarmed_bonus"));
    ADD_BOOL_BONUS(Immobilized, _CTS("immobilized_bonus"));
    ADD_BOOL_BONUS(Immobilized2, _CTS("immobilized_bonus"));
    ADD_BOOL_BONUS(Restrained, _CTS("restrained_bonus"));
    ADD_BOOL_BONUS(Sighted, _CTS("sighted_bonus"));
    ADD_BOOL_BONUS(Revealed, _CTS("revealed_bonus"));

    ADD_FLOAT_BONUS_NOPREFIX(RevealRange, 0, 0, _CTS("reveal_bonus"), 1.0f, ROUND);

    if (pDefinition->GetStealthType(uiIndex) != 0)
    {
        s_mapTokens.clear();
        s_mapTokens[_CTS("value")] = XtoA(MsToSec(pDefinition->GetFadeTime(uiIndex)), 0, 0, 0, 2);
        sStr += GameClient.GetGameMessage(_CTS("stealth_bonus"), s_mapTokens) + _CTS("\n"); \
        ++iLines;
    }

    ADD_BOOL_BONUS(Unitwalking, _CTS("unitwalking_bonus"));
    ADD_BOOL_BONUS(Treewalking, _CTS("treewalking_bonus"));
    ADD_BOOL_BONUS(Cliffwalking, _CTS("cliffwalking_bonus"));
    ADD_BOOL_BONUS(Buildingwalking, _CTS("buildingwalking_bonus"));

    if (pDefinition->GetImmunityType(uiIndex) != 0)
    {
        s_mapTokens.clear();
        s_mapTokens[_CTS("type")] = Game.GetEffectTypeString(pDefinition->GetImmunityType(uiIndex));
        sStr += GameClient.GetGameMessage(_CTS("immunity_bonus"), s_mapTokens) + _CTS("\n"); \
        ++iLines;
    }

    ADD_BOOL_BONUS(Invulnerable, _CTS("invulnerable_bonus"));
    ADD_BOOL_BONUS(DispelOnDamage, _CTS("dispel_on_damage_bonus"));
    ADD_BOOL_BONUS(DispelOnAction, _CTS("dispel_on_action_bonus"));

    ADD_BOOL_BONUS(TrueStrike, _CTS("truestrike_bonus"));

    ADD_FLOAT_BONUS_NOPREFIX(HealthRegenReduction, 0, 0, _CTS("health_regen_reduction_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS_NOPREFIX(ManaRegenReduction, 0, 0, _CTS("mana_regen_reduction_bonus"), 100.0f, ROUND);

    tstring sOnFrame;
    BuildText(pDefinition->GetEffectDescription(ACTION_SCRIPT_FRAME), uiIndex, sOnFrame);

    if (!sOnFrame.empty())
    {
        sStr += sOnFrame + _CTS("\n");
        ++iLines;
    }

    #undef ADD_FLOAT_BONUS
    #undef ADD_FLOAT_BONUS_NOPREFIX
    #undef ADD_FLOAT_BONUS_PAIR
    #undef ADD_BOOL_BONUS
    #undef ADD_FLOAT_PROGRESSIVE_BONUS
}


/*====================
  CGameInterfaceManager::BuildMultiLevelBonusesString
  ====================*/
void    CGameInterfaceManager::BuildMultiLevelBonusesString(ISlaveEntity *pSlave, tstring &sStr, int &iLines)
{
    ISlaveDefinition *pDefinition(pSlave->GetDefinition<ISlaveDefinition>());
    if (pDefinition == nullptr)
        return;

    uint uiLevel(pSlave->GetLevel());
    uint uiMaxLevel(pSlave->GetMaxLevel());

    uivector vModifierKeys;
    BuildMultiLevelBonusesString(pDefinition, uiLevel, uiMaxLevel, vModifierKeys, sStr, iLines);
}


/*====================
  CGameInterfaceManager::BuildMultiLevelBonusesString
  ====================*/
void    CGameInterfaceManager::BuildMultiLevelBonusesString(ISlaveDefinition *pDefinition, uint uiLevel, uint uiMaxLevel, uivector &vModifierKeys, tstring &sStr, int &iLines)
{
    // Find and activate all exclusive modifiers
    uint uiModifier(0);

    uiModifier |= pDefinition->GetModifierBits(vModifierKeys);

    // Search this entity
    const EntityModifierMap &mapModifiers(pDefinition->GetModifiers());
    for (EntityModifierMap::const_iterator cit(mapModifiers.begin()), citEnd(mapModifiers.end()); cit != citEnd; ++cit)
    {
        IEntityDefinition *pModifier(cit->second);

        if (!pModifier->GetExclusive() && pModifier->GetCondition().empty())
            continue;

        uiModifier |= cit->first;
    }

    IEntityDefinition *pModifiedDefinition(pDefinition->GetModifiedDefinition(uiModifier));
    if (pModifiedDefinition != nullptr)
        pDefinition = static_cast<ISlaveDefinition *>(pModifiedDefinition);

    uint uiLevelIndex(MAX(1u, uiLevel) - 1);

    sStr.clear();
    iLines = 0;

    static tsmapts s_mapTokens;

    #define ADD_FLOAT_BONUS(property, iMinPrecision, iMaxPrecision, label, scale, conversion) \
    { \
        bool bConstant(true); \
        float f##property(pDefinition->Get##property(0)); \
        for (uint uiIndex(1); uiIndex < uiMaxLevel; ++uiIndex) \
        { \
            if (pDefinition->Get##property(uiIndex) != f##property) \
                bConstant = false; \
        } \
        if (bConstant) \
        { \
            if (f##property != 0.0f) \
            { \
                s_mapTokens.clear(); \
                s_mapTokens[_CTS("value")] = (f##property >= 0.0f ? _CTS("+") : TSNULL) + XtoA(conversion(f##property * scale), 0, 0, iMinPrecision, iMaxPrecision); \
                sStr += GameClient.GetGameMessage(label, s_mapTokens) + _CTS("\n"); \
                ++iLines; \
            } \
        } \
        else \
        { \
            s_mapTokens.clear(); \
            tstring sTemp; \
            if (f##property >= 0.0f) \
                sTemp += _T('+'); \
            if (uiLevel >= 1) \
                sTemp += _CTS("^v") + XtoA(conversion(f##property * scale), 0, 0, iMinPrecision, iMaxPrecision) + _CTS("^*"); \
            else \
                sTemp += XtoA(conversion(f##property * scale), 0, 0, iMinPrecision, iMaxPrecision); \
            for (uint uiIndex(1); uiIndex < uiMaxLevel; ++uiIndex) \
            { \
                sTemp += _T('/'); \
                if (uiLevel >= uiIndex + 1) \
                    sTemp += _CTS("^v") + XtoA(conversion(pDefinition->Get##property(uiIndex) * scale), 0, 0, iMinPrecision, iMaxPrecision) + _CTS("^*"); \
                else \
                    sTemp += XtoA(conversion(pDefinition->Get##property(uiIndex) * scale), 0, 0, iMinPrecision, iMaxPrecision); \
            } \
            s_mapTokens[_CTS("value")] = sTemp; \
            sStr += GameClient.GetGameMessage(label, s_mapTokens) + _CTS("\n"); \
            ++iLines; \
        } \
    }

    #define ADD_FLOAT_BONUS_NOPREFIX(property, iMinPrecision, iMaxPrecision, label, scale, conversion) \
    { \
        bool bConstant(true); \
        float f##property(pDefinition->Get##property(0)); \
        for (uint uiIndex(1); uiIndex < uiMaxLevel; ++uiIndex) \
        { \
            if (pDefinition->Get##property(uiIndex) != f##property) \
                bConstant = false; \
        } \
        if (bConstant) \
        { \
            if (f##property != 0.0f) \
            { \
                s_mapTokens.clear(); \
                s_mapTokens[_CTS("value")] = XtoA(conversion(f##property * scale), 0, 0, iMinPrecision, iMaxPrecision); \
                sStr += GameClient.GetGameMessage(label, s_mapTokens) + _CTS("\n"); \
                ++iLines; \
            } \
        } \
        else \
        { \
            s_mapTokens.clear(); \
            tstring sTemp; \
            if (uiLevel >= 1) \
                sTemp += _CTS("^v") + XtoA(conversion(f##property * scale), 0, 0, iMinPrecision, iMaxPrecision) + _CTS("^*"); \
            else \
                sTemp += XtoA(conversion(f##property * scale), 0, 0, iMinPrecision, iMaxPrecision); \
            for (uint uiIndex(1); uiIndex < uiMaxLevel; ++uiIndex) \
            { \
                sTemp += _T('/'); \
                if (uiLevel >= uiIndex + 1) \
                    sTemp += _CTS("^v") + XtoA(conversion(pDefinition->Get##property(uiIndex) * scale), 0, 0, iMinPrecision, iMaxPrecision) + _CTS("^*"); \
                else \
                    sTemp += XtoA(conversion(pDefinition->Get##property(uiIndex) * scale), 0, 0, iMinPrecision, iMaxPrecision); \
            } \
            s_mapTokens[_CTS("value")] = sTemp; \
            sStr += GameClient.GetGameMessage(label, s_mapTokens) + _CTS("\n"); \
            ++iLines; \
        } \
    }

    #define ADD_FLOAT_BONUS_PAIR(property1, iMinPrecision1, iMaxPrecision1, label, scale1, conversion1, property2, iMinPrecision2, iMaxPrecision2, scale2, conversion2) \
    { \
        bool bConstant1(true); \
        float f##property1(pDefinition->Get##property1(0)); \
        for (uint uiIndex(1); uiIndex < uiMaxLevel; ++uiIndex) \
        { \
            if (pDefinition->Get##property1(uiIndex) != f##property1) \
                bConstant1 = false; \
        } \
        bool bConstant2(true); \
        float f##property2(pDefinition->Get##property2(0)); \
        for (uint uiIndex(1); uiIndex < uiMaxLevel; ++uiIndex) \
        { \
            if (pDefinition->Get##property2(uiIndex) != f##property2) \
                bConstant2 = false; \
        } \
        if (bConstant1 && bConstant2) \
        { \
            if (f##property1 != 0.0f) \
            { \
                s_mapTokens.clear(); \
                s_mapTokens[_CTS("value1")] = XtoA(conversion1(f##property1 * scale1), 0, 0, iMinPrecision1, iMaxPrecision1); \
                s_mapTokens[_CTS("value2")] = XtoA(conversion2(f##property2 * scale2), 0, 0, iMinPrecision2, iMaxPrecision2); \
                sStr += GameClient.GetGameMessage(label, s_mapTokens) + _CTS("\n"); \
                ++iLines; \
            } \
        } \
        else \
        { \
            s_mapTokens.clear(); \
            tstring sValue1, sValue2; \
            if (bConstant1) \
            { \
                sValue1 = XtoA(conversion1(f##property1 * scale1), 0, 0, iMinPrecision1, iMaxPrecision1); \
            } \
            else \
            { \
                if (uiLevel >= 1) \
                    sValue1 += _CTS("^v") + XtoA(conversion1(f##property1 * scale1), 0, 0, iMinPrecision1, iMaxPrecision1) + _CTS("^*"); \
                else \
                    sValue1 += XtoA(conversion1(f##property1 * scale1), 0, 0, iMinPrecision1, iMaxPrecision1); \
                for (uint uiIndex(1); uiIndex < uiMaxLevel; ++uiIndex) \
                { \
                    sValue1 += _T('/'); \
                    if (uiLevel >= uiIndex + 1) \
                        sValue1 += _CTS("^v") + XtoA(conversion1(pDefinition->Get##property1(uiIndex) * scale1), 0, 0, iMinPrecision1, iMaxPrecision1) + _CTS("^*"); \
                    else \
                        sValue1 += XtoA(conversion1(pDefinition->Get##property1(uiIndex) * scale1), 0, 0, iMinPrecision1, iMaxPrecision1); \
                } \
            } \
            if (bConstant2) \
            { \
                sValue2 += XtoA(conversion2(f##property2 * scale2), 0, 0, iMinPrecision2, iMaxPrecision2); \
            } \
            else \
            { \
                if (uiLevel >= 1) \
                    sValue2 += _CTS("^v") + XtoA(conversion2(f##property2 * scale2), 0, 0, iMinPrecision2, iMaxPrecision2) + _CTS("^*"); \
                else \
                    sValue2 += XtoA(conversion2(f##property2 * scale2), 0, 0, iMinPrecision2, iMaxPrecision2); \
                for (uint uiIndex(1); uiIndex < uiMaxLevel; ++uiIndex) \
                { \
                    sValue2 += _T('/'); \
                    if (uiLevel >= uiIndex + 1) \
                        sValue2 += _CTS("^v") + XtoA(conversion2(pDefinition->Get##property2(uiIndex) * scale2), 0, 0, iMinPrecision2, iMaxPrecision2) + _CTS("^*"); \
                    else \
                        sValue2 += XtoA(conversion2(pDefinition->Get##property2(uiIndex) * scale2), 0, 0, iMinPrecision2, iMaxPrecision2); \
                } \
            } \
            s_mapTokens[_CTS("value1")] = sValue1; \
            s_mapTokens[_CTS("value2")] = sValue2; \
            sStr += GameClient.GetGameMessage(label, s_mapTokens) + _CTS("\n"); \
            ++iLines; \
        } \
    }

    #define ADD_BOOL_BONUS(property, label) \
    { \
        bool b##property(pDefinition->Get##property(uiLevelIndex)); \
        if (b##property) \
        { \
            sStr += GameClient.GetGameMessage(label) + _CTS("\n"); \
            ++iLines; \
        } \
    }

    #define ADD_FLOAT_PROGRESSIVE_BONUS(property, iMinPrecision, iMaxPrecision, label, label2, scale, conversion) \
    { \
        s_mapTokens.clear(); \
        bool bConstant(true); \
        float f##property(pDefinition->Get##property(0)); \
        float f##property##PerLevel(pDefinition->Get##property##PerLevel()); \
        for (uint uiIndex(1); uiIndex < uiMaxLevel; ++uiIndex) \
        { \
            if (pDefinition->Get##property(uiIndex) != f##property) \
                bConstant = false; \
        } \
        if (bConstant) \
        { \
            f##property += f##property##PerLevel * uiLevelIndex; \
            if (f##property != 0.0f) \
            { \
                s_mapTokens[_CTS("value")] = (f##property >= 0.0f ? _CTS("+") : TSNULL) + XtoA(conversion(f##property * scale), 0, 0, iMinPrecision, iMaxPrecision); \
                sStr += GameClient.GetGameMessage(label, s_mapTokens) + _CTS("\n"); \
                ++iLines; \
            } \
        } \
        else \
        { \
            tstring sTemp; \
            if (f##property >= 0.0f) \
                sTemp += _T('+'); \
            if (uiLevel >= 1) \
                sTemp += _CTS("^v") + XtoA(conversion(f##property * scale), 0, 0, iMinPrecision, iMaxPrecision) + _CTS("^*"); \
            else \
                sTemp += XtoA(conversion(f##property * scale), 0, 0, iMinPrecision, iMaxPrecision); \
            for (uint uiIndex(1); uiIndex < uiMaxLevel; ++uiIndex) \
            { \
                sTemp += _T('/'); \
                if (uiLevel >= uiIndex + 1) \
                    sTemp += _CTS("^v") + XtoA(conversion(pDefinition->Get##property(uiIndex) * scale), 0, 0, iMinPrecision, iMaxPrecision) + _CTS("^*"); \
                else \
                    sTemp += XtoA(conversion(pDefinition->Get##property(uiIndex) * scale), 0, 0, iMinPrecision, iMaxPrecision); \
            } \
            s_mapTokens[_CTS("value")] = sTemp; \
            sStr += GameClient.GetGameMessage(label, s_mapTokens) + _CTS("\n"); \
            ++iLines; \
        } \
        s_mapTokens.clear(); \
        bool bConstantPerCharge(true); \
        float f##property##PerCharge(pDefinition->Get##property##PerCharge(0)); \
        for (uint uiIndex(1); uiIndex < uiMaxLevel; ++uiIndex) \
        { \
            if (pDefinition->Get##property##PerCharge(uiIndex) != f##property##PerCharge) \
                bConstantPerCharge = false; \
        } \
        if (bConstantPerCharge) \
        { \
            if (f##property##PerCharge != 0.0f) \
            { \
                s_mapTokens[_CTS("value_per_charge")] = (f##property##PerCharge >= 0.0f ? _CTS("+") : TSNULL) + XtoA(conversion(f##property##PerCharge * scale), 0, 0, iMinPrecision, iMaxPrecision); \
                sStr += GameClient.GetGameMessage(label2, s_mapTokens) + _CTS("\n"); \
                ++iLines; \
            } \
        } \
        else \
        { \
            tstring sTemp; \
            if (f##property##PerCharge >= 0.0f) \
                sTemp += _T('+'); \
            if (uiLevel >= 1) \
                sTemp += _CTS("^v") + XtoA(conversion(f##property##PerCharge * scale), 0, 0, iMinPrecision, iMaxPrecision) + _CTS("^*"); \
            else \
                sTemp += XtoA(conversion(f##property##PerCharge * scale), 0, 0, iMinPrecision, iMaxPrecision); \
            for (uint uiIndex(1); uiIndex < uiMaxLevel; ++uiIndex) \
            { \
                sTemp += _T('/'); \
                if (uiLevel >= uiIndex + 1) \
                    sTemp += _CTS("^v") + XtoA(conversion(pDefinition->Get##property##PerCharge(uiIndex) * scale), 0, 0, iMinPrecision, iMaxPrecision) + _CTS("^*"); \
                else \
                    sTemp += XtoA(conversion(pDefinition->Get##property##PerCharge(uiIndex) * scale), 0, 0, iMinPrecision, iMaxPrecision); \
            } \
            s_mapTokens[_CTS("value_per_charge")] = sTemp; \
            sStr += GameClient.GetGameMessage(label2, s_mapTokens) + _CTS("\n"); \
            ++iLines; \
        } \
    }

    ADD_FLOAT_PROGRESSIVE_BONUS(Strength, 0, 0, _CTS("str_bonus"), _CTS("str_bonus_per_charge"), 1.0f, floor);
    ADD_FLOAT_PROGRESSIVE_BONUS(Agility, 0, 0, _CTS("agi_bonus"), _CTS("agi_bonus_per_charge"), 1.0f, floor);
    ADD_FLOAT_PROGRESSIVE_BONUS(Intelligence, 0, 0, _CTS("int_bonus"), _CTS("int_bonus_per_charge"), 1.0f, floor);
    ADD_FLOAT_BONUS(MaxHealth, 0, 0, _CTS("max_health_bonus"), 1.0f, floor);
    ADD_FLOAT_BONUS(MaxHealthMultiplier, 0, 0, _CTS("max_health_mult_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(MaxMana, 0, 0, _CTS("max_mana_bonus"), 1.0f, floor);
    ADD_FLOAT_BONUS(MaxManaMultiplier, 0, 0, _CTS("max_mana_mult_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(BaseDamageMultiplier, 0, 0, _CTS("base_damage_mult_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(Damage, 0, 0, _CTS("damage_bonus"), 1.0f, floor);
    ADD_FLOAT_BONUS(DamagePerCharge, 0, 0, _CTS("damage_bonus_per_charge"), 1.0f, floor);
    ADD_FLOAT_BONUS(TotalDamageMultiplier, 0, 0, _CTS("damage_mult_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(MoveSpeed, 0, 0, _CTS("move_speed_bonus"), 1.0f, floor);
    ADD_FLOAT_BONUS(MoveSpeedMultiplier, 0, 2, _CTS("move_speed_mult_bonus"), 100.0f, float);
    ADD_FLOAT_BONUS(MoveSpeedMultiplierPerCharge, 0, 2, _CTS("move_speed_mult_bonus_per_charge"), 100.0f, float);
    ADD_FLOAT_BONUS(SlowResistance, 0, 0, _CTS("slow_resistance_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS_NOPREFIX(MoveSpeedSlow, 0, 2, _CTS("move_speed_slow_bonus"), 100.0f, float);
    ADD_FLOAT_BONUS_NOPREFIX(MoveSpeedSlowPerCharge, 0, 2, _CTS("move_speed_slow_bonus_per_charge"), 100.0f, float);
    ADD_FLOAT_BONUS(AttackSpeed, 0, 0, _CTS("attack_speed_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(AttackSpeedPerCharge, 0, 0, _CTS("attack_speed_bonus_per_charge"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(AttackSpeedMultiplier, 0, 0, _CTS("attack_speed_mult_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS_NOPREFIX(AttackSpeedSlow, 0, 0, _CTS("attack_speed_slow_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(CastSpeed, 0, 0, _CTS("cast_speed_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(CooldownSpeed, 0, 0, _CTS("cooldown_speed_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS_NOPREFIX(ReducedCooldowns, 0, 0, _CTS("reduced_cooldowns_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS_NOPREFIX(IncreasedCooldowns, 0, 0, _CTS("increased_cooldowns_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(Armor, 0, 2, _CTS("armor_bonus"), 1.0f, float);
    ADD_FLOAT_BONUS(ArmorPerCharge, 0, 2, _CTS("armor_bonus_per_charge"), 1.0f, float);
    ADD_FLOAT_BONUS(MagicArmor, 0, 2, _CTS("magic_armor_bonus"), 1.0f, float);
    ADD_FLOAT_BONUS(MagicArmorPerCharge, 0, 2, _CTS("magic_armor_bonus_per_charge"), 1.0f, float);
    ADD_FLOAT_BONUS(HealthRegenPercent, 0, 2, _CTS("health_regen_percent_bonus"), 100.0f, float);
    ADD_FLOAT_BONUS(HealthRegen, 0, 2, _CTS("health_regen_bonus"), 1.0f, float);
    ADD_FLOAT_BONUS(HealthRegenMultiplier, 0, 0, _CTS("health_regen_mult_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(ManaRegenPercent, 0, 2, _CTS("mana_regen_percent_bonus"), 100.0f, float);
    ADD_FLOAT_BONUS(ManaRegen, 0, 2, _CTS("mana_regen_bonus"), 1.0f, float);
    ADD_FLOAT_BONUS(ManaRegenMultiplier, 0, 0, _CTS("mana_regen_mult_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS_PAIR(
        DeflectionChance, 0, 0, _CTS("deflection_bonus"), 100.0f, ROUND,
        Deflection, 0, 0, 1.0f, floor);

    {
        bool bSplit(false);
        for (uint uiIndex(0); uiIndex < uiMaxLevel; ++uiIndex)
        {
            if (pDefinition->GetEvasionRanged(uiIndex) != pDefinition->GetEvasionMelee(uiIndex))
                bSplit = true;
        }

        if (bSplit)
        {
            ADD_FLOAT_BONUS_NOPREFIX(EvasionRanged, 0, 0, _CTS("ranged_evasion_bonus"), 100.0f, ROUND);
            ADD_FLOAT_BONUS_NOPREFIX(EvasionMelee, 0, 0, _CTS("melee_evasion_bonus"), 100.0f, ROUND);
        }
        else
        {
            ADD_FLOAT_BONUS_NOPREFIX(EvasionRanged, 0, 0, _CTS("evasion_bonus"), 100.0f, ROUND);
        }
    }

    ADD_FLOAT_BONUS_NOPREFIX(MissChance, 0, 0, _CTS("miss_chance_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(LifeSteal, 0, 0, _CTS("lifesteal_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS_PAIR(
        CriticalChance, 0, 0, _CTS("critical_bonus"), 100.0f, ROUND,
        CriticalMultiplier, 1, 2, 1.0f, float);
    ADD_FLOAT_BONUS(IncomingDamageMultiplier, 0, 0, _CTS("incoming_damage_mult_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(DebuffDurationMultiplier, 0, 0, _CTS("debuff_duration_mult_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS(HealMultiplier, 0, 0, _CTS("heal_mult_bonus"), 100.0f, ROUND);

    ADD_BOOL_BONUS(Stunned, _CTS("stunned_bonus"));
    ADD_BOOL_BONUS(Silenced, _CTS("silenced_bonus"));
    ADD_BOOL_BONUS(Perplexed, _CTS("perplexed_bonus"));
    ADD_BOOL_BONUS(Disarmed, _CTS("disarmed_bonus"));
    ADD_BOOL_BONUS(Immobilized, _CTS("immobilized_bonus"));
    ADD_BOOL_BONUS(Immobilized2, _CTS("immobilized_bonus"));
    ADD_BOOL_BONUS(Restrained, _CTS("restrained_bonus"));
    ADD_BOOL_BONUS(Sighted, _CTS("sighted_bonus"));
    ADD_BOOL_BONUS(Revealed, _CTS("revealed_bonus"));

    ADD_FLOAT_BONUS_NOPREFIX(RevealRange, 0, 0, _CTS("reveal_bonus"), 1.0f, ROUND);

    //ADD_FLOAT_BONUS_NOPREFIX(FadeTime, 0, 2, _CTS("stealth_bonus"), 0.001f, float);
    if (pDefinition->GetStealthType(uiLevelIndex) != 0)
    {
        bool bConstant(true);
        float fFadeTime(pDefinition->GetFadeTime(0));
        for (uint uiIndex(1); uiIndex < uiMaxLevel; ++uiIndex)
        {
            if (pDefinition->GetFadeTime(uiIndex) != fFadeTime)
                bConstant = false;
        }
        if (bConstant)
        {
            s_mapTokens.clear();
            s_mapTokens[_CTS("value")] = XtoA(fFadeTime * 0.001f, 0, 0, 0, 2);
            sStr += GameClient.GetGameMessage(_CTS("stealth_bonus"), s_mapTokens) + _CTS("\n");
            ++iLines;
        }
        else
        {
            s_mapTokens.clear();
            tstring sTemp;
            if (uiLevel >= 1)
                sTemp += _CTS("^v") + XtoA(fFadeTime * 0.001f, 0, 0, 0, 2) + _CTS("^*");
            else
                sTemp += XtoA(fFadeTime * 0.001f, 0, 0, 0, 2);
            for (uint uiIndex(1); uiIndex < uiMaxLevel; ++uiIndex)
            {
                sTemp += _T('/');
                if (uiLevel >= uiIndex + 1)
                    sTemp += _CTS("^v") + XtoA(pDefinition->GetFadeTime(uiIndex) * 0.001f, 0, 0, 0, 2) + _CTS("^*");
                else
                    sTemp += XtoA(pDefinition->GetFadeTime(uiIndex) * 0.001f, 0, 0, 0, 2);
            }
            s_mapTokens[_CTS("value")] = sTemp;
            sStr += GameClient.GetGameMessage(_CTS("stealth_bonus"), s_mapTokens) + _CTS("\n");
            ++iLines;
        }
    }

    ADD_BOOL_BONUS(Unitwalking, _CTS("unitwalking_bonus"));
    ADD_BOOL_BONUS(Treewalking, _CTS("treewalking_bonus"));
    ADD_BOOL_BONUS(Cliffwalking, _CTS("cliffwalking_bonus"));
    ADD_BOOL_BONUS(Buildingwalking, _CTS("buildingwalking_bonus"));

    if (pDefinition->GetImmunityType(uiLevelIndex) != 0)
    {
        s_mapTokens.clear();
        s_mapTokens[_CTS("type")] = Game.GetEffectTypeString(pDefinition->GetImmunityType(uiLevelIndex));
        sStr += GameClient.GetGameMessage(_CTS("immunity_bonus"), s_mapTokens) + _CTS("\n");
        ++iLines;
    }

    ADD_BOOL_BONUS(Invulnerable, _CTS("invulnerable_bonus"));

    if (GET_ENTITY_BASE_TYPE2(pDefinition->GetAllocator()->GetBaseType()) == ENTITY_BASE_TYPE2_STATE)
    {
        CStateDefinition *pStateDefinition(static_cast<CStateDefinition *>(pDefinition));

        if (pStateDefinition->GetDispelOnDamage(uiLevelIndex))
        {
            sStr += GameClient.GetGameMessage(_CTS("dispel_on_damage_bonus")) + _CTS("\n");
            ++iLines;
        }
        if (pStateDefinition->GetDispelOnAction(uiLevelIndex))
        {
            sStr += GameClient.GetGameMessage(_CTS("dispel_on_action_bonus")) + _CTS("\n");
            ++iLines;
        }
    }

    ADD_BOOL_BONUS(TrueStrike, _CTS("truestrike_bonus"));

    ADD_FLOAT_BONUS_NOPREFIX(HealthRegenReduction, 0, 0, _CTS("health_regen_reduction_bonus"), 100.0f, ROUND);
    ADD_FLOAT_BONUS_NOPREFIX(ManaRegenReduction, 0, 0, _CTS("mana_regen_reduction_bonus"), 100.0f, ROUND);

    tstring sOnFrame;
    BuildMultiLevelText(pDefinition->GetEffectDescription(ACTION_SCRIPT_FRAME), uiLevel, MAX(1u, uiMaxLevel) - 1, sOnFrame);

    if (!sOnFrame.empty())
    {
        sStr += sOnFrame + _CTS("\n");
        ++iLines;
    }

    #undef ADD_FLOAT_BONUS
    #undef ADD_FLOAT_BONUS_NOPREFIX
    #undef ADD_FLOAT_BONUS_PAIR
    #undef ADD_BOOL_BONUS
    #undef ADD_FLOAT_PROGRESSIVE_BONUS
}


#define BUILD_FLOAT_PROPERTY(definition, var, property, iMinPrecision, iMaxPrecision) \
tstring var; \
{ \
    bool bConstant(true); \
    float f##property(definition->Get##property(0)); \
    for (uint uiIndex(1); uiIndex < uiMaxLevel; ++uiIndex) \
    { \
        if (definition->Get##property(uiIndex) != f##property) \
            bConstant = false; \
    } \
    if (bConstant) \
    { \
        if (f##property != 0.0f) \
            var = XtoA(f##property, 0, 0, iMinPrecision, iMaxPrecision); \
    } \
    else \
    { \
        if (uiLevel >= 1) \
            var = _CTS("^v") + XtoA(f##property, 0, 0, iMinPrecision, iMaxPrecision) + _CTS("^*"); \
        else \
            var = XtoA(f##property, 0, 0, iMinPrecision, iMaxPrecision); \
        for (uint uiIndex(1); uiIndex < uiMaxLevel; ++uiIndex) \
        { \
            var += _T('/'); \
            if (uiLevel >= uiIndex + 1) \
                var += _CTS("^v") + XtoA(definition->Get##property(uiIndex), 0, 0, iMinPrecision, iMaxPrecision) + _CTS("^*"); \
            else \
                var += XtoA(definition->Get##property(uiIndex), 0, 0, iMinPrecision, iMaxPrecision); \
        } \
    } \
}


struct STriggedEffect
{
    EEntityActionScript eScript;
    tstring sLabel;
};

static STriggedEffect s_cTriggers[] =
{
    { ACTION_SCRIPT_ATTACK, _T("script_on_attack") },
    { ACTION_SCRIPT_ATTACK_PRE_IMPACT, _T("script_on_attack_pre_impact") },
    { ACTION_SCRIPT_ATTACK_PRE_DAMAGE, _T("script_on_attack_pre_damage") },
    { ACTION_SCRIPT_ATTACK_IMPACT, _T("script_on_attack_impact") },
    { ACTION_SCRIPT_ATTACKED_PRE_IMPACT, _T("script_on_attacked_pre_impact") },
    { ACTION_SCRIPT_ATTACKED_PRE_DAMAGE, _T("script_on_attacked_pre_damage") },
    { ACTION_SCRIPT_ATTACKED_POST_IMPACT, _T("script_on_attacked_post_impact") },
    { ACTION_SCRIPT_ACTIVATE_PRE_IMPACT, _T("script_on_activate_pre_impact") },
    { ACTION_SCRIPT_ACTIVATE_IMPACT, _T("script_on_activate_impact") },
    { ACTION_SCRIPT_ABILITY_IMPACT, _T("script_on_ability_impact") },
    { ACTION_SCRIPT_DAMAGE, _T("script_on_damage") },
    { ACTION_SCRIPT_DAMAGED, _T("script_on_damaged") },
    { ACTION_SCRIPT_KILL, _T("script_on_kill") },
    { ACTION_SCRIPT_KILLED, _T("script_on_killed") },
    { ACTION_SCRIPT_ASSIST, _T("script_on_assist") },
    { NUM_ACTION_SCRIPTS, _T("") }
};


/*====================
  CGameInterfaceManager::UpdateActiveInventory
  ====================*/
void    CGameInterfaceManager::UpdateActiveInventory(IUnitEntity *pUnit, int iStartSlot, int iEndSlot)
{
    PROFILE("CGameInterfaceManager::UpdateActiveInventory");

    CClientCommander *pCommander(GameClient.GetClientCommander());
    if (pCommander == nullptr)
        return;

    CPlayer *pLocalPlayer(GameClient.GetLocalPlayer());

    if (pUnit == nullptr)
    {
        for (int iSlot(iStartSlot); iSlot <= iEndSlot; ++iSlot)
        {
            Trigger(UITRIGGER_ACTIVE_INVENTORY_EXISTS, false, iSlot);
            Trigger(UITRIGGER_ACTIVE_INVENTORY_INTERFACE, _CTS("single"), iSlot);
        }

        return;
    }

    uivector vModifierKeys;
    pUnit->GetSlaveModifiers(vModifierKeys);

    for (int iSlot(iStartSlot), iDisplaySlot(iStartSlot); iSlot <= iEndSlot; ++iSlot)
    {
        ISlaveEntity *pSlave(pUnit->GetSlave(iSlot));
        IEntityTool *pTool(pUnit->GetTool(iSlot));
        IEntityItem *pItem(pUnit->GetItem(iSlot));

        if (iSlot < INVENTORY_START_STATES || iSlot > INVENTORY_END_STATES)
            iDisplaySlot = iSlot;

        if (pSlave != nullptr && pSlave->IsAbility() && pSlave->GetAsAbility()->GetSubSlot() != -1)
        {
            int iSubSlot(pSlave->GetAsAbility()->GetSubSlot());
            pSlave = pUnit->GetSlave(iSubSlot);
            pTool = pUnit->GetTool(iSubSlot);
        }

        if (pSlave == nullptr)
        {
            Trigger(UITRIGGER_ACTIVE_INVENTORY_EXISTS, false, iDisplaySlot);
            Trigger(UITRIGGER_ACTIVE_INVENTORY_INTERFACE, _CTS("single"), iDisplaySlot);
            ++iDisplaySlot;
            continue;
        }

        IEntityState *pState(pSlave->GetAsState());
        if (pState != nullptr && pState->GetIsHidden())
            continue;

        ISlaveDefinition *pDefinition(pSlave->GetActiveDefinition<ISlaveDefinition>());

        Trigger(UITRIGGER_ACTIVE_INVENTORY_EXISTS, true, iDisplaySlot);

        if (pSlave->IsAbility() && !pSlave->GetAsAbility()->GetInterface().empty())
            Trigger(UITRIGGER_ACTIVE_INVENTORY_INTERFACE, pSlave->GetAsAbility()->GetInterface(), iDisplaySlot);
        else
            Trigger(UITRIGGER_ACTIVE_INVENTORY_INTERFACE, _CTS("single"), iDisplaySlot);

        bool bCanActive(pTool != nullptr && pTool->GetActionType() != TOOL_ACTION_PASSIVE && !(pTool->GetLevel() < 1 && pTool->GetMaxLevel() > 0));

        Trigger(UITRIGGER_ACTIVE_INVENTORY_ICON, pSlave->GetIconPath(), iDisplaySlot);
        Trigger(UITRIGGER_ACTIVE_INVENTORY_RECIPE, pTool == nullptr || (!pTool->HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED) && pTool->IsItem()), iDisplaySlot);
        Trigger(UITRIGGER_ACTIVE_INVENTORY_CAN_ACTIVATE, bCanActive, iDisplaySlot);

        // normal status, active, silenced, low mana, in use, level, can level up, max level
        static tsvector vStatus(12);
        vStatus[0] = XtoA(pTool ? pTool->CanActivate() : false);
        vStatus[1] = XtoA(pTool != nullptr && (pTool->HasFlag(ENTITY_TOOL_FLAG_TOGGLE_ACTIVE) || pTool->HasFlag(ENTITY_TOOL_FLAG_CHANNEL_ACTIVE)));
        vStatus[2] = XtoA(pTool ? (pTool->IsDisabled() || (pUnit->IsStunned() && !pTool->GetNoStun())) : false);
        vStatus[3] = XtoA(pTool != nullptr && !pUnit->IsFreeCast() && ((pTool->GetManaCost() != 0.0f && pTool->GetManaCost() > pUnit->GetMana()) || (pTool->GetTriggeredManaCost() != 0.0f && pTool->GetTriggeredManaCost() > pUnit->GetMana()) || pTool->HasFlag(ENTITY_TOOL_FLAG_INVALID_COST) || !pTool->CheckTriggeredCost()) && pUnit->GetStatus() == ENTITY_STATUS_ACTIVE);
        vStatus[4] = XtoA(pTool != nullptr && pTool->HasFlag(ENTITY_TOOL_FLAG_IN_USE));
        vStatus[5] = XtoA(pSlave->GetLevel());
        vStatus[6] = XtoA(pTool != nullptr && pTool->CanLevelUp());

        if (pSlave->IsState())
            vStatus[7] = pSlave->GetAsState()->GetDisplayLevel() ? _CTS("1") : _CTS("0");
        else if (pTool == nullptr || (!pTool->HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED) && pTool->IsItem()))
            vStatus[7] = _CTS("0");
        else
            vStatus[7] = XtoA(pTool->GetMaxLevel());

        vStatus[8] = XtoA(pCommander->GetActiveSlot() == iSlot);
        vStatus[9] = XtoA(pTool != nullptr && pTool->IsItem() && pTool->GetAsItem()->GetAllowSharing());
        vStatus[10] = XtoA(pTool != nullptr && pTool->IsItem() && pTool->GetAsItem()->IsBorrowed());
        vStatus[11] = XtoA(pLocalPlayer != nullptr && pUnit->GetTeam() == pLocalPlayer->GetTeam());

        Trigger(UITRIGGER_ACTIVE_INVENTORY_STATUS, vStatus, iDisplaySlot);

        // State specific info
        static tsvector vState(4);
        for (uint ui(0); ui < vState.size(); ++ui)
            vState[ui].clear();

        if (pState != nullptr)
        {
            vState[0] = _CTS("true");
            vState[1] = XtoA(GameClient.IsDebuff(pState->GetEffectType()));
            vState[2] = XtoA(GameClient.IsBuff(pState->GetEffectType()));
            vState[3] = XtoA(pState->IsAuraInvalid());
        }

        Trigger(UITRIGGER_ACTIVE_INVENTORY_STATE, vState, iDisplaySlot);

        // Cooldown
        static tsvector vCooldown(3);
        vCooldown[0] = XtoA(pTool ? pTool->GetRemainingCooldownTime() : 0);
        vCooldown[1] = XtoA(pTool ? pTool->GetTooltipCooldownTime() : 0);
        vCooldown[2] = XtoA(pTool ? pTool->GetRemainingCooldownPercent() : 0.0f);
        Trigger(UITRIGGER_ACTIVE_INVENTORY_COOLDOWN, vCooldown, iDisplaySlot);

        if (pTool != nullptr && pTool->GetTimer() != INVALID_TIME)
        {
            uint uiTimer;
            if (pTool->GetTimer() > Game.GetGameTime())
                uiTimer = pTool->GetTimer() - Game.GetGameTime();
            else
                uiTimer = 0;

            Trigger(UITRIGGER_ACTIVE_INVENTORY_HAS_TIMER, true, iDisplaySlot);
            Trigger(UITRIGGER_ACTIVE_INVENTORY_TIMER, uiTimer, iDisplaySlot);
        }
        else
        {
            Trigger(UITRIGGER_ACTIVE_INVENTORY_HAS_TIMER, false, iDisplaySlot);
            Trigger(UITRIGGER_ACTIVE_INVENTORY_TIMER, 0, iDisplaySlot);
        }

        // Charges
        Trigger(UITRIGGER_ACTIVE_INVENTORY_CHARGES, pSlave->GetCharges(), iDisplaySlot);

        static tsmapts s_mapTokens;

        static tsvector vDescription(21);

        if (pSlave->IsItem() && !pSlave->GetAsItem()->HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED))
        {
            s_mapTokens.clear();
            s_mapTokens[_CTS("name")] = pSlave->GetDisplayName();
            vDescription[0] = GameClient.GetGameMessage(_CTS("item_recipe"), s_mapTokens);
        }
        else
        {
            vDescription[0] = pSlave->GetDisplayName();
        }

        Trigger(UITRIGGER_ACTIVE_INVENTORY_DESCRIPTION_A, CBuildText(cg_tooltipFlavor ? pSlave->GetDescriptionIndex() : INVALID_INDEX, MAX(1u, pSlave->GetLevel()) - 1), iDisplaySlot);
        Trigger(UITRIGGER_ACTIVE_INVENTORY_DESCRIPTION_B, CBuildText(pSlave->GetDescription2Index(), MAX(1u, pSlave->GetLevel()) - 1), iDisplaySlot);

        vDescription[2] = XtoA(pTool ? pTool->GetManaCost() : 0.0f);
        vDescription[3] = XtoA(pTool != nullptr && pTool->IsItem() ? pTool->GetAsItem()->GetValue() : 0);

        vDescription[4].clear();
        vDescription[5].clear();

        if (pTool != nullptr && bCanActive)
        {
            if (pTool->GetActionType() == TOOL_ACTION_TOGGLE)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_toggle"));
            else if (pTool->GetActionType() == TOOL_ACTION_NO_TARGET)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_no_target"));
            else if (pTool->GetActionType() == TOOL_ACTION_TARGET_POSITION)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_position"));
            else if (pTool->GetActionType() == TOOL_ACTION_TARGET_ENTITY)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_unit"));
            else if (pTool->GetActionType() == TOOL_ACTION_GLOBAL)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_global"));
            else if (pTool->GetActionType() == TOOL_ACTION_TARGET_SELF)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_self"));
            else if (pTool->GetActionType() == TOOL_ACTION_FACING)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_facing"));
            else if (pTool->GetActionType() == TOOL_ACTION_SELF_POSITION)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_self_position"));
            else if (pTool->GetActionType() == TOOL_ACTION_ATTACK)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_attack"));
            else if (pTool->GetActionType() == TOOL_ACTION_ATTACK_TOGGLE)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_attack_toggle"));
            else if (pTool->GetActionType() == TOOL_ACTION_TARGET_DUAL)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_dual"));
            else if (pTool->GetActionType() == TOOL_ACTION_TARGET_DUAL_POSITION)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_dual_position"));
            else if (pTool->GetActionType() == TOOL_ACTION_TARGET_VECTOR)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_vector"));
            else if (pTool->GetActionType() == TOOL_ACTION_TARGET_CURSOR)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_cursor"));
        }
        else
        {
            vDescription[6].clear();
        }

        if (pTool != nullptr)
        {
            vDescription[7] = Game.GetTargetSchemeDisplayName(pTool->GetTargetScheme());
            vDescription[8] = Game.GetEffectTypeString(pTool->GetCastEffectType());
            vDescription[9] = XtoA(pTool->GetRange(), 0, 0, 0, 1);
            vDescription[10] = XtoA(pTool->GetTargetRadius(), 0, 0, 0, 1);
            vDescription[11] = XtoA(bCanActive);

            if (pItem != nullptr && !pItem->CanUse())
            {
                CPlayer *pPurchaser(GameClient.GetPlayer(pItem->GetPurchaserClientNumber()));

                s_mapTokens.clear();
                s_mapTokens[_CTS("name")] = pPurchaser ? pPurchaser->GetName() : TSNULL;
                vDescription[12] = GameClient.GetGameMessage(_CTS("activate_item_no_share"), s_mapTokens);
            }
            else if (pItem != nullptr && pItem->IsBorrowed())
            {
                CPlayer *pPurchaser(GameClient.GetPlayer(pItem->GetPurchaserClientNumber()));

                s_mapTokens.clear();
                s_mapTokens[_CTS("name")] = pPurchaser ? pPurchaser->GetName() : TSNULL;
                vDescription[12] = GameClient.GetGameMessage(_CTS("activate_item_borrowed"), s_mapTokens);
            }
            else if (!(pTool->GetLevel() < 1 && pTool->GetMaxLevel() > 0))
            {
                if (pTool->GetActionType() != TOOL_ACTION_PASSIVE)
                    vDescription[12] = GameClient.GetGameMessage(pTool->IsAbility() ? _CTS("activate_ability") : _CTS("activate_item"));
                else if (pTool->IsAbility())
                    vDescription[12] = GameClient.GetGameMessage(_CTS("activate_passive"));
                else
                    vDescription[12] = TSNULL;
            }
            else
                vDescription[12] = TSNULL;

            Trigger(UITRIGGER_ACTIVE_INVENTORY_EFFECT_DESCRIPTION, CBuildText(pTool->GetEffectDescriptionIndex(ACTION_SCRIPT_IMPACT), MAX(1u, pTool->GetLevel()) - 1), iDisplaySlot);

            vDescription[14] = cg_tooltipFlavor ? pTool->GetTooltipFlavorText() : TSNULL;

            if (pTool->GetIsChanneling())
                vDescription[17] = XtoA(pTool->GetChannelTime());
            else
                vDescription[17].clear();

            vDescription[18] = XtoA(pTool ? pTool->GetActiveManaCost() : 0.0f);
            vDescription[19] = XtoA(pTool ? pTool->GetTriggeredManaCost() : 0.0f);
            vDescription[20] = XtoA(pTool ? pTool->GetTooltipCooldownTime() : 0);
        }
        else
        {
            vDescription[7].clear();
            vDescription[8].clear();
            vDescription[9].clear();
            vDescription[10].clear();
            vDescription[11].clear();
            vDescription[12].clear();
            vDescription[13].clear();
            vDescription[14].clear();
            vDescription[17].clear();
            vDescription[18].clear();
            vDescription[19].clear();
            vDescription[20].clear();
        }

        vDescription[15].clear();

        Trigger(UITRIGGER_ACTIVE_INVENTORY_DESCRIPTION, vDescription, iDisplaySlot);

        Trigger(UITRIGGER_ACTIVE_INVENTORY_DURATION, pState != nullptr && pState->GetExpireTime() != INVALID_TIME ? pState->GetExpireTime() - Game.GetGameTime() : 0, iDisplaySlot);
        Trigger(UITRIGGER_ACTIVE_INVENTORY_DURATION_PERCENT, pState != nullptr && pState->GetExpireTime() != INVALID_TIME ? pState->GetRemainingLifetimePercent() : 0.0f, iDisplaySlot);

        // Build passive bonus list
        static tsvector vPassiveEffect(2);

        int iLines(0);
        BuildBonusesString(pSlave, vPassiveEffect[0], iLines);
        vPassiveEffect[1] = XtoA(iLines);

        Trigger(UITRIGGER_ACTIVE_INVENTORY_PASSIVE_EFFECT, vPassiveEffect, iDisplaySlot);

        if (pTool != nullptr)
        {
            static tsvector vHotkeys(3);

            IBaseInput *pAction(ActionRegistry.GetAction(_CTS("ActivateTool")));

            int iAbilitySlot(-1);
            IEntityAbility *pAbility(pUnit->GetAbility(iSlot));
            if (pAbility != nullptr && pAbility->GetKeySlot() != -1)
                iAbilitySlot = pAbility->GetKeySlot();

            if (iAbilitySlot == -1)
                iAbilitySlot = iSlot;

            tstring sParam(XtoA(iAbilitySlot));

            int iIndex(1);

            const ButtonActionMap &lButton(ActionRegistry.GetButtonActionMap(BINDTABLE_GAME));
            for (ButtonActionMap::const_iterator it(lButton.begin()); it != lButton.end() && iIndex < 3; ++it)
            {
                if (it->first == BUTTON_INVALID)
                    continue;

                for (BindModMap::const_iterator itBind(it->second.begin()); itBind != it->second.end() && iIndex < 3; ++itBind)
                {
                    if (itBind->second.GetAction() == pAction && itBind->second.GetParam() == sParam)
                        vHotkeys[iIndex++] = Input.GetBindString(it->first, itBind->first);
                }
            }

            vHotkeys[0] = XtoA(iIndex - 1);

            Trigger(UITRIGGER_ACTIVE_INVENTORY_HOTKEYS, vHotkeys, iDisplaySlot);
        }

        static tsvector vAura(7);
        for (int i(0); i < 7; ++i)
            vAura[i].clear();

        if (pTool != nullptr)
        {
            if (pTool->GetLevel() > 0 || pTool->GetMaxLevel() == 0)
            {
                if (pDefinition != nullptr)
                {
                    const AuraList &cAuraList(pDefinition->GetAuraList());

                    if (!cAuraList.empty())
                    {
                        CStateDefinition *pAuraStateDef(EntityRegistry.GetDefinition<CStateDefinition>(cAuraList.front().GetStateName(pTool->GetLevel())));

                        if (pAuraStateDef != nullptr)
                        {
                            if (cAuraList.front().GetNoTooltip())
                                vAura[0] = _CTS("false");
                            else
                                vAura[0] = _CTS("true");

                            int iLines(0);
                            BuildBonusesString(pAuraStateDef, pTool->GetLevel(), vModifierKeys, vAura[1], iLines);
                            vAura[2] = XtoA(iLines);

                            vAura[3] = _CTS("(Aura Stack Type)");
                            vAura[4] = Game.GetTargetSchemeDisplayName(cAuraList.front().GetTargetScheme(pTool->GetLevel()));
                            vAura[5] = Game.GetEffectTypeString(cAuraList.front().GetEffectType(pTool->GetLevel()));
                            vAura[6] = cAuraList.front().GetRadius(pTool->GetLevel()) >= 9999.0f ? GameClient.GetGameMessage(_CTS("aura_range_global")) : XtoA(INT_ROUND(cAuraList.front().GetRadius(pTool->GetLevel())));
                        }
                    }
                }
            }
        }

        Trigger(UITRIGGER_ACTIVE_INVENTORY_AURA, vAura, iDisplaySlot);

        static tsvector vStatusEffect(3);
        for (uint ui(0); ui < vStatusEffect.size(); ++ui)
            vStatusEffect[ui].clear();

        if (pTool != nullptr)
        {
            if (pTool->GetLevel() > 0 || pTool->GetMaxLevel() == 0)
            {
                CStateDefinition *pStateDef(EntityRegistry.GetDefinition<CStateDefinition>(pTool->GetStatusEffectTooltip()));
                if (pStateDef != nullptr)
                {
                    vStatusEffect[0] = pTool->GetStatusEffectHeader();

                    int iLines(0);
                    BuildBonusesString(pStateDef, pTool->GetLevel(), vModifierKeys, vStatusEffect[1], iLines);
                    vStatusEffect[2] = XtoA(iLines);
                }
            }
        }

        Trigger(UITRIGGER_ACTIVE_INVENTORY_STATUS_EFFECT, vStatusEffect, iDisplaySlot);

        for (uint ui(0); ui < vStatusEffect.size(); ++ui)
            vStatusEffect[ui].clear();

        if (pTool != nullptr)
        {
            if (pTool->GetLevel() > 0 || pTool->GetMaxLevel() == 0)
            {
                CStateDefinition *pStateDef(EntityRegistry.GetDefinition<CStateDefinition>(pTool->GetStatusEffectTooltip2()));
                if (pStateDef != nullptr)
                {
                    vStatusEffect[0] = pTool->GetStatusEffectHeader2();

                    int iLines(0);
                    BuildBonusesString(pStateDef, pTool->GetLevel(), vModifierKeys, vStatusEffect[1], iLines);
                    vStatusEffect[2] = XtoA(iLines);
                }
            }
        }

        Trigger(UITRIGGER_ACTIVE_INVENTORY_STATUS_EFFECTB, vStatusEffect, iDisplaySlot);

        static tsvector vTriggeredEffect(3);
        uint uiTriggerIndex(0);
        uint uiDisplayIndex(0);
        const uint NUM_DISPLAY_TRIGGERS(1);

        if (pTool != nullptr)
        {
            if (pTool->GetLevel() > 0 || pTool->GetMaxLevel() == 0)
            {
                while (s_cTriggers[uiTriggerIndex].eScript != NUM_ACTION_SCRIPTS && uiDisplayIndex < NUM_DISPLAY_TRIGGERS)
                {
                    uint uiString(pDefinition->GetEffectDescriptionIndex(s_cTriggers[uiTriggerIndex].eScript));
                    if (uiString == INVALID_INDEX)
                    {
                        ++uiTriggerIndex;
                        continue;
                    }

                    for (int i(0); i < 3; ++i)
                        vTriggeredEffect[i].clear();

                    vTriggeredEffect[0] = _CTS("true");
                    vTriggeredEffect[1] = GameClient.GetGameMessage(s_cTriggers[uiTriggerIndex].sLabel);
                    Trigger(UITRIGGER_ACTIVE_INVENTORY_TRIGGERED_EFFECT_DESCRIPTION, CBuildText(uiString, MAX(1u, pSlave->GetLevel()) - 1), iDisplaySlot);

                    Trigger(UITRIGGER_ACTIVE_INVENTORY_TRIGGERED_EFFECT, vTriggeredEffect, iDisplaySlot);
                    ++uiDisplayIndex;
                    ++uiTriggerIndex;
                }
            }
        }

        for (; uiDisplayIndex < NUM_DISPLAY_TRIGGERS; ++uiDisplayIndex)
        {
            for (int i(0); i < 3; ++i)
                vTriggeredEffect[i].clear();

            Trigger(UITRIGGER_ACTIVE_INVENTORY_TRIGGERED_EFFECT, vTriggeredEffect, iDisplaySlot);
            Trigger(UITRIGGER_ACTIVE_INVENTORY_TRIGGERED_EFFECT_DESCRIPTION, CBuildText(INVALID_INDEX, 0), iDisplaySlot);
        }

        uint uiString(pDefinition->GetEffectDescriptionIndex(ACTION_SCRIPT_ACTIVATE_COST));
        if (uiString != INVALID_INDEX)
            Trigger(UITRIGGER_ACTIVE_INVENTORY_ACTIVATE_COST, CBuildText(uiString, MAX(1u, pSlave->GetLevel()) - 1), iDisplaySlot);
        else
            Trigger(UITRIGGER_ACTIVE_INVENTORY_ACTIVATE_COST, CBuildText(INVALID_INDEX, 0), iDisplaySlot);

        ++iDisplaySlot;
    }
}


/*====================
  CGameInterfaceManager::UpdateHeroInventory
  ====================*/
void    CGameInterfaceManager::UpdateHeroInventory(IUnitEntity *pUnit, int iStartSlot, int iEndSlot)
{
    PROFILE("CGameInterfaceManager::UpdateHeroInventory");

    CClientCommander *pCommander(GameClient.GetClientCommander());
    if (pCommander == nullptr)
        return;

    if (pUnit == nullptr)
    {
        for (int iSlot(iStartSlot); iSlot <= iEndSlot; ++iSlot)
        {
            Trigger(UITRIGGER_HERO_INVENTORY_EXISTS, false, iSlot);
            Trigger(UITRIGGER_HERO_INVENTORY_INTERFACE, _CTS("single"), iSlot);
        }

        return;
    }

    uivector vModifierKeys;
    pUnit->GetSlaveModifiers(vModifierKeys);

    for (int iSlot(iStartSlot), iDisplaySlot(iStartSlot); iSlot <= iEndSlot; ++iSlot)
    {
        ISlaveEntity *pSlave(pUnit->GetSlave(iSlot));
        IEntityTool *pTool(pUnit->GetTool(iSlot));

        if (iSlot < INVENTORY_START_STATES || iSlot > INVENTORY_END_STATES)
            iDisplaySlot = iSlot;

        if (pSlave != nullptr && pSlave->IsAbility() && pSlave->GetAsAbility()->GetSubSlot() != -1)
        {
            int iSubSlot(pSlave->GetAsAbility()->GetSubSlot());
            pSlave = pUnit->GetSlave(iSubSlot);
            pTool = pUnit->GetTool(iSubSlot);
        }

        if (pSlave == nullptr)
        {
            Trigger(UITRIGGER_HERO_INVENTORY_EXISTS, false, iDisplaySlot);
            Trigger(UITRIGGER_HERO_INVENTORY_INTERFACE, _CTS("single"), iDisplaySlot);
            ++iDisplaySlot;
            continue;
        }

        IEntityState *pState(pSlave->GetAsState());
        if (pState != nullptr && pState->GetIsHidden())
            continue;

        ISlaveDefinition *pDefinition(pSlave->GetDefinition<ISlaveDefinition>());

        Trigger(UITRIGGER_HERO_INVENTORY_EXISTS, true, iDisplaySlot);

        if (pSlave->IsAbility() && !pSlave->GetAsAbility()->GetInterface().empty())
            Trigger(UITRIGGER_HERO_INVENTORY_INTERFACE, pSlave->GetAsAbility()->GetInterface(), iDisplaySlot);
        else
            Trigger(UITRIGGER_HERO_INVENTORY_INTERFACE, _CTS("single"), iDisplaySlot);

        bool bCanActive(pTool != nullptr && pTool->GetActionType() != TOOL_ACTION_PASSIVE && !(pTool->GetLevel() < 1 && pTool->GetMaxLevel() > 0));

        Trigger(UITRIGGER_HERO_INVENTORY_ICON, pSlave->GetIconPath(), iDisplaySlot);
        Trigger(UITRIGGER_HERO_INVENTORY_CAN_ACTIVATE, bCanActive, iDisplaySlot);

        // normal status, active, silenced, low mana, in use, level, can level up, max level
        static tsvector vStatus(11);
        vStatus[0] = XtoA(pTool ? pTool->CanActivate() : false);
        vStatus[1] = XtoA(pTool != nullptr && (pTool->HasFlag(ENTITY_TOOL_FLAG_TOGGLE_ACTIVE) || pTool->HasFlag(ENTITY_TOOL_FLAG_CHANNEL_ACTIVE)));
        vStatus[2] = XtoA(pTool ? (pTool->IsDisabled() || (pUnit->IsStunned() && !pTool->GetNoStun())) : false);
        vStatus[3] = XtoA(pTool != nullptr && pTool->GetManaCost() != 0.0f && !pUnit->IsFreeCast() && pTool->GetManaCost() > pUnit->GetMana() && pUnit->GetStatus() == ENTITY_STATUS_ACTIVE);
        vStatus[4] = XtoA(pTool != nullptr && pTool->HasFlag(ENTITY_TOOL_FLAG_IN_USE));
        vStatus[5] = XtoA(pSlave->GetLevel());
        vStatus[6] = XtoA(pTool != nullptr && pTool->CanLevelUp());

        if (pSlave->IsState())
            vStatus[7] = pSlave->GetAsState()->GetDisplayLevel() ? _CTS("1") : _CTS("0");
        else if (pTool == nullptr || (!pTool->HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED) && pTool->IsItem()))
            vStatus[7] = _CTS("0");
        else
            vStatus[7] = XtoA(pTool->GetMaxLevel());

        vStatus[8] = XtoA(pCommander->GetActiveSlot() == iSlot);
        vStatus[9] = XtoA(pTool != nullptr && pTool->IsItem() && pTool->GetAsItem()->GetAllowSharing());
        vStatus[10] = XtoA(pTool != nullptr && pTool->IsItem() && pTool->GetAsItem()->IsBorrowed());

        Trigger(UITRIGGER_HERO_INVENTORY_STATUS, vStatus, iDisplaySlot);

        static tsvector vCooldown(3);
        vCooldown[0] = XtoA(pTool ? pTool->GetRemainingCooldownTime() : 0);
        vCooldown[1] = XtoA(pTool ? pTool->GetTooltipCooldownTime() : 0);
        vCooldown[2] = XtoA(pTool ? pTool->GetRemainingCooldownPercent() : 0.0f);
        Trigger(UITRIGGER_HERO_INVENTORY_COOLDOWN, vCooldown, iDisplaySlot);
        Trigger(UITRIGGER_HERO_INVENTORY_CHARGES, pSlave->GetCharges(), iDisplaySlot);

        if (pTool && pTool->GetTimer() != INVALID_TIME)
        {
            uint uiTimer;
            if (pTool->GetTimer() > Game.GetGameTime())
                uiTimer = pTool->GetTimer() - Game.GetGameTime();
            else
                uiTimer = 0;

            Trigger(UITRIGGER_HERO_INVENTORY_HAS_TIMER, pTool->GetTimer() != INVALID_TIME, iDisplaySlot);
            Trigger(UITRIGGER_HERO_INVENTORY_TIMER, uiTimer, iDisplaySlot);
        }
        else
        {
            Trigger(UITRIGGER_HERO_INVENTORY_HAS_TIMER, false, iDisplaySlot);
            Trigger(UITRIGGER_HERO_INVENTORY_TIMER, 0, iDisplaySlot);
        }

        static tsvector vDescription(21);
        vDescription[0] = pSlave->GetDisplayName();

        if (pSlave->IsItem() && !pSlave->GetAsItem()->HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED))
            vDescription[0] += _CTS(" Recipe");

        BuildText(cg_tooltipFlavor ? pSlave->GetDescription() : TSNULL, MAX(1u, pSlave->GetLevel()) - 1, vDescription[1]);
        BuildText(pSlave->GetDescription2(), MAX(1u, pSlave->GetLevel()) - 1, vDescription[16]);

        vDescription[2] = XtoA(pTool ? pTool->GetManaCost() : 0.0f);
        vDescription[3] = XtoA(pTool != nullptr && pTool->IsItem() ? pTool->GetAsItem()->GetValue() : 0);

        vDescription[4].clear();
        vDescription[5].clear();

        if (pTool != nullptr && bCanActive)
        {
            if (pTool->GetActionType() == TOOL_ACTION_TOGGLE)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_toggle"));
            else if (pTool->GetActionType() == TOOL_ACTION_NO_TARGET)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_no_target"));
            else if (pTool->GetActionType() == TOOL_ACTION_TARGET_POSITION)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_position"));
            else if (pTool->GetActionType() == TOOL_ACTION_TARGET_ENTITY)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_unit"));
            else if (pTool->GetActionType() == TOOL_ACTION_GLOBAL)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_global"));
            else if (pTool->GetActionType() == TOOL_ACTION_TARGET_SELF)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_self"));
            else if (pTool->GetActionType() == TOOL_ACTION_FACING)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_facing"));
            else if (pTool->GetActionType() == TOOL_ACTION_SELF_POSITION)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_self_position"));
            else if (pTool->GetActionType() == TOOL_ACTION_ATTACK)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_attack"));
            else if (pTool->GetActionType() == TOOL_ACTION_ATTACK_TOGGLE)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_attack_toggle"));
            else if (pTool->GetActionType() == TOOL_ACTION_TARGET_DUAL)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_dual"));
            else if (pTool->GetActionType() == TOOL_ACTION_TARGET_DUAL_POSITION)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_dual_position"));
            else if (pTool->GetActionType() == TOOL_ACTION_TARGET_VECTOR)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_vector"));
            else if (pTool->GetActionType() == TOOL_ACTION_TARGET_CURSOR)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_cursor"));
        }
        else
        {
            vDescription[6].clear();
        }

        if (pTool != nullptr)
        {
            vDescription[7] = Game.GetTargetSchemeDisplayName(pTool->GetTargetScheme());
            vDescription[8] = Game.GetEffectTypeString(pTool->GetCastEffectType());
            vDescription[9] = XtoA(pTool->GetRange(), 0, 0, 0, 1);
            vDescription[10] = XtoA(pTool->GetTargetRadius(), 0, 0, 0, 1);
            vDescription[11] = XtoA(bCanActive);

            if (!(pTool->GetLevel() < 1 && pTool->GetMaxLevel() > 0))
            {
                if (pTool->GetActionType() != TOOL_ACTION_PASSIVE)
                    vDescription[12] = pTool->IsAbility() ? _CTS("Click to activate this ability") : _CTS("Click to activate this item");
                else if (pTool->IsAbility())
                    vDescription[12] = _CTS("^gThis ability is passive");
                else
                    vDescription[12] = TSNULL;
            }
            else
                vDescription[12] = TSNULL;

            BuildText(pTool->GetEffectDescription(ACTION_SCRIPT_IMPACT), MAX(1u, pTool->GetLevel()) - 1, vDescription[13]);
            vDescription[14] = cg_tooltipFlavor ? pTool->GetTooltipFlavorText() : TSNULL;

            if (pTool->GetIsChanneling())
                vDescription[17] = XtoA(pTool->GetChannelTime());
            else
                vDescription[17].clear();

            vDescription[18] = XtoA(pTool ? pTool->GetActiveManaCost() : 0.0f);
            vDescription[19] = XtoA(pTool ? pTool->GetTriggeredManaCost() : 0.0f);
            vDescription[20] = XtoA(pTool ? pTool->GetTooltipCooldownTime() : 0);
        }
        else
        {
            vDescription[7].clear();
            vDescription[8].clear();
            vDescription[9].clear();
            vDescription[10].clear();
            vDescription[11].clear();
            vDescription[12].clear();
            vDescription[13].clear();
            vDescription[14].clear();
            vDescription[17].clear();
            vDescription[18].clear();
            vDescription[19].clear();
            vDescription[20].clear();
        }

        vDescription[15].clear();

        Trigger(UITRIGGER_HERO_INVENTORY_DESCRIPTION, vDescription, iDisplaySlot);

        Trigger(UITRIGGER_HERO_INVENTORY_DURATION, pState != nullptr && pState->GetExpireTime() != INVALID_TIME ? pState->GetExpireTime() - Game.GetGameTime() : 0, iDisplaySlot);

        // Build passive bonus list
        static tsvector vPassiveEffect(2);

        int iLines(0);
        BuildBonusesString(pSlave, vPassiveEffect[0], iLines);
        vPassiveEffect[1] = XtoA(iLines);

        Trigger(UITRIGGER_HERO_INVENTORY_PASSIVE_EFFECT, vPassiveEffect, iDisplaySlot);

        if (pTool != nullptr)
        {
            static tsvector vHotkeys(3);

            IBaseInput *pAction(ActionRegistry.GetAction(_CTS("ActivateTool")));
            tstring sParam(XtoA(iSlot));

            int iIndex(1);

            const ButtonActionMap &lButton(ActionRegistry.GetButtonActionMap(BINDTABLE_GAME));
            for (ButtonActionMap::const_iterator it(lButton.begin()); it != lButton.end() && iIndex < 3; ++it)
            {
                if (it->first == BUTTON_INVALID)
                    continue;

                for (BindModMap::const_iterator itBind(it->second.begin()); itBind != it->second.end() && iIndex < 3; ++itBind)
                {
                    if (itBind->second.GetAction() == pAction && itBind->second.GetParam() == sParam)
                        vHotkeys[iIndex++] = Input.GetBindString(it->first, itBind->first);
                }
            }

            vHotkeys[0] = XtoA(iIndex - 1);

            Trigger(UITRIGGER_HERO_INVENTORY_HOTKEYS, vHotkeys, iDisplaySlot);
        }

        static tsvector vAura(7);
        for (int i(0); i < 7; ++i)
            vAura[i].clear();

        if (pTool != nullptr)
        {
            if (pTool->GetLevel() > 0 || pTool->GetMaxLevel() == 0)
            {
                if (pDefinition != nullptr)
                {
                    const AuraList &cAuraList(pDefinition->GetAuraList());

                    if (!cAuraList.empty())
                    {
                        CStateDefinition *pAuraStateDef(EntityRegistry.GetDefinition<CStateDefinition>(cAuraList.front().GetStateName(pTool->GetLevel())));

                        if (pAuraStateDef != nullptr)
                        {
                            vAura[0] = _CTS("true");

                            int iLines(0);
                            BuildBonusesString(pAuraStateDef, pTool->GetLevel(), vModifierKeys, vAura[1], iLines);
                            vAura[2] = XtoA(iLines);

                            vAura[3] = _CTS("(Aura Stack Type)");
                            vAura[4] = Game.GetTargetSchemeDisplayName(cAuraList.front().GetTargetScheme(pTool->GetLevel()));
                            vAura[5] = Game.GetEffectTypeString(cAuraList.front().GetEffectType(pTool->GetLevel()));
                            vAura[6] = cAuraList.front().GetRadius(pTool->GetLevel()) >= 9999.0f ? _CTS("Global") : XtoA(INT_ROUND(cAuraList.front().GetRadius(pTool->GetLevel())));
                        }
                    }
                }
            }
        }

        Trigger(UITRIGGER_HERO_INVENTORY_AURA, vAura, iDisplaySlot);

        static tsvector vStatusEffect(3);
        for (uint ui(0); ui < vStatusEffect.size(); ++ui)
            vStatusEffect[ui].clear();

        if (pTool != nullptr)
        {
            if (pTool->GetLevel() > 0 || pTool->GetMaxLevel() == 0)
            {
                CStateDefinition *pStateDef(EntityRegistry.GetDefinition<CStateDefinition>(pTool->GetStatusEffectTooltip()));
                if (pStateDef != nullptr)
                {
                    vStatusEffect[0] = pTool->GetStatusEffectHeader();

                    int iLines(0);
                    BuildBonusesString(pStateDef, pTool->GetLevel(), vModifierKeys, vStatusEffect[1], iLines);
                    vStatusEffect[2] = XtoA(iLines);
                }
            }
        }

        Trigger(UITRIGGER_HERO_INVENTORY_STATUS_EFFECT, vStatusEffect, iDisplaySlot);

        for (uint ui(0); ui < vStatusEffect.size(); ++ui)
            vStatusEffect[ui].clear();

        if (pTool != nullptr)
        {
            if (pTool->GetLevel() > 0 || pTool->GetMaxLevel() == 0)
            {
                CStateDefinition *pStateDef(EntityRegistry.GetDefinition<CStateDefinition>(pTool->GetStatusEffectTooltip2()));
                if (pStateDef != nullptr)
                {
                    vStatusEffect[0] = pTool->GetStatusEffectHeader2();

                    int iLines(0);
                    BuildBonusesString(pStateDef, pTool->GetLevel(), vModifierKeys, vStatusEffect[1], iLines);
                    vStatusEffect[2] = XtoA(iLines);
                }
            }
        }

        Trigger(UITRIGGER_HERO_INVENTORY_STATUS_EFFECTB, vStatusEffect, iDisplaySlot);

        static tsvector vTriggeredEffect(3);
        uint uiTriggerIndex(0);
        uint uiDisplayIndex(0);

        const uint NUM_DISPLAY_TRIGGERS(1);

        if (pTool != nullptr)
        {
            if (pTool->GetLevel() > 0 || pTool->GetMaxLevel() == 0)
            {
                while (s_cTriggers[uiTriggerIndex].eScript != NUM_ACTION_SCRIPTS && uiDisplayIndex < NUM_DISPLAY_TRIGGERS)
                {
                    const tstring &sDescription(pDefinition->GetEffectDescription(s_cTriggers[uiTriggerIndex].eScript));
                    if (sDescription.empty())
                    {
                        ++uiTriggerIndex;
                        continue;
                    }

                    for (int i(0); i < 3; ++i)
                        vTriggeredEffect[i].clear();

                    vTriggeredEffect[0] = _CTS("true");
                    vTriggeredEffect[1] = GameClient.GetGameMessage(s_cTriggers[uiTriggerIndex].sLabel);
                    BuildText(sDescription, MAX(1u, pSlave->GetLevel()) - 1, vTriggeredEffect[2]);

                    Trigger(UITRIGGER_HERO_INVENTORY_TRIGGERED_EFFECT, vTriggeredEffect, iDisplaySlot);
                    ++uiDisplayIndex;
                    ++uiTriggerIndex;
                }
            }
        }

        for (; uiDisplayIndex < NUM_DISPLAY_TRIGGERS; ++uiDisplayIndex)
        {
            for (int i(0); i < 3; ++i)
                vTriggeredEffect[i].clear();

            Trigger(UITRIGGER_HERO_INVENTORY_TRIGGERED_EFFECT, vTriggeredEffect, iDisplaySlot);
        }

        ++iDisplaySlot;
    }
}


/*====================
  CGameInterfaceManager::UpdateActiveAttackModifiers
  ====================*/
void    CGameInterfaceManager::UpdateActiveAttackModifiers(IUnitEntity *pUnit)
{
    static tsvector vAttackModifier(5);

    if (pUnit == nullptr)
    {
        for (tsvector_it it(vAttackModifier.begin()); it != vAttackModifier.end(); ++it)
            it->clear();
        vAttackModifier[4] = _T("-1");

        for (uint uiIndex(0); uiIndex < MAX_ATTACK_MODIFIERS; ++uiIndex)
            Trigger(UITRIGGER_ATTACK_MODIFIERS, vAttackModifier, uiIndex);

        return;
    }

    uint uiAttackModifiers(0);

    for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
    {
        ISlaveEntity *pSlave(pUnit->GetSlave(iSlot));

        if (pSlave == nullptr)
            continue;
        if (!pSlave->IsActive())
            continue;

        uint uiModBit(pSlave->GetModifierBit(EntityRegistry.RegisterModifier(_T("attack"))));
        if (uiModBit != 0)
        {
            ISlaveDefinition *pAttackModifierDefinition(pSlave->GetDefinition<ISlaveDefinition>(uiModBit));
            if (pAttackModifierDefinition != nullptr)
            {
                vAttackModifier[0] = pAttackModifierDefinition->GetDisplayName();
                vAttackModifier[1] = pAttackModifierDefinition->GetIconPath(pSlave->GetLevel());
                vAttackModifier[2] = pAttackModifierDefinition->GetDescription();
                vAttackModifier[3] = XtoA(pSlave->HasAttackModPriority());
                vAttackModifier[4] = XtoA(pSlave->GetSlot());

                Trigger(UITRIGGER_ATTACK_MODIFIERS, vAttackModifier, uiAttackModifiers);
                ++uiAttackModifiers;
            }
        }
    }

    // Clear unused attack modifier buttons
    for (tsvector_it it(vAttackModifier.begin()); it != vAttackModifier.end(); ++it)
        it->clear();
    vAttackModifier[4] = _T("-1");

    for (uint uiIndex(uiAttackModifiers); uiIndex < MAX_ATTACK_MODIFIERS; ++uiIndex)
        Trigger(UITRIGGER_ATTACK_MODIFIERS, vAttackModifier, uiIndex);
}


/*====================
  CGameInterfaceManager::UpdateLevelUp
  ====================*/
void    CGameInterfaceManager::UpdateLevelUp(IUnitEntity *pUnit, int iStartSlot, int iEndSlot)
{
    PROFILE("CGameInterfaceManager::UpdateLevelUp");

    if (pUnit == nullptr)
        return;

    CClientCommander *pCommander(GameClient.GetClientCommander());
    if (pCommander == nullptr)
        return;

    uivector vModifierKeys;
    pUnit->GetSlaveModifiers(vModifierKeys);

    for (int iSlot(iStartSlot); iSlot <= iEndSlot; ++iSlot)
    {
        IEntityTool *pTool(pUnit->GetTool(iSlot));
        if (pTool == nullptr)
        {
            continue;
        }

        IToolDefinition *pDefinition(pTool->GetDefinition<IToolDefinition>());
        if (pDefinition == nullptr)
            continue;

        uint uiLevel(pTool->GetLevel());
        uint uiMaxLevel(pDefinition->GetMaxLevel());

        Trigger(UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_CAN_ACTIVATE, pDefinition->GetActionType() != TOOL_ACTION_PASSIVE, iSlot);

        // normal status, active, silenced, low mana, in use, level, can level up, max level
        static tsvector vStatus(9);
        vStatus[5] = XtoA(pTool->GetLevel());
        vStatus[6] = XtoA(pTool->CanLevelUp());

        if (!pTool->HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED) && pTool->IsItem())
            vStatus[7] = _CTS("0");
        else
            vStatus[7] = XtoA(pTool->GetMaxLevel());

        Trigger(UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_STATUS, vStatus, iSlot);

        tstring sCooldownTime;
        {
            bool bConstant(true);
            uint uiCooldownTime(pDefinition->GetCooldownTime(0));

            for (uint uiIndex(1); uiIndex < uiMaxLevel; ++uiIndex)
            {
                if (pDefinition->GetCooldownTime(uiIndex) != uiCooldownTime)
                    bConstant = false;
            }

            if (bConstant)
            {
                if (uiCooldownTime != 0)
                    sCooldownTime = XtoA(MsToSec(uiCooldownTime), 0, 0, 0, 3);
            }
            else
            {
                if (uiLevel >= 1)
                    sCooldownTime = _CTS("^v") + XtoA(MsToSec(uiCooldownTime), 0, 0, 0, 3) + _CTS("^*");
                else
                    sCooldownTime = XtoA(MsToSec(uiCooldownTime), 0, 0, 0, 3);

                for (uint uiIndex(1); uiIndex < uiMaxLevel; ++uiIndex)
                {
                    sCooldownTime += _T('/');

                    if (uiLevel >= uiIndex + 1)
                        sCooldownTime += _CTS("^v") + XtoA(MsToSec(pDefinition->GetCooldownTime(uiIndex)), 0, 0, 0, 3) + _CTS("^*");
                    else
                        sCooldownTime += XtoA(MsToSec(pDefinition->GetCooldownTime(uiIndex)), 0, 0, 0, 3);
                }
            }
        }

        Trigger(UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_COOLDOWN, sCooldownTime, iSlot);

        static tsvector vDescription(19);
        vDescription[0] = pDefinition->GetDisplayName();

        Trigger(UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_DESCRIPTION_A, CBuildMultiLevelText(cg_tooltipFlavor ? pDefinition->GetDescriptionIndex() : INVALID_INDEX, uiLevel, MAX(1u, uiMaxLevel) - 1), iSlot);
        Trigger(UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_DESCRIPTION_B, CBuildMultiLevelText(pDefinition->GetDescription2Index(), uiLevel, MAX(1u, uiMaxLevel) - 1), iSlot);

        BUILD_FLOAT_PROPERTY(pDefinition, sManaCost, ManaCost, 0, 2);
        vDescription[2] = sManaCost;

        // Build passive bonus list
        int iLines(0);
        BuildMultiLevelBonusesString(pTool, vDescription[4], iLines);
        vDescription[5] = XtoA(iLines);

        if (pDefinition->GetActionType() == TOOL_ACTION_TOGGLE)
            vDescription[6] = GameClient.GetGameMessage(_CTS("action_toggle"));
        else if (pDefinition->GetActionType() == TOOL_ACTION_NO_TARGET)
            vDescription[6] = GameClient.GetGameMessage(_CTS("action_no_target"));
        else if (pDefinition->GetActionType() == TOOL_ACTION_TARGET_POSITION)
            vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_position"));
        else if (pDefinition->GetActionType() == TOOL_ACTION_TARGET_ENTITY)
            vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_unit"));
        else if (pDefinition->GetActionType() == TOOL_ACTION_GLOBAL)
            vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_global"));
        else if (pDefinition->GetActionType() == TOOL_ACTION_TARGET_SELF)
            vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_self"));
        else if (pDefinition->GetActionType() == TOOL_ACTION_FACING)
            vDescription[6] = GameClient.GetGameMessage(_CTS("action_facing"));
        else if (pDefinition->GetActionType() == TOOL_ACTION_SELF_POSITION)
            vDescription[6] = GameClient.GetGameMessage(_CTS("action_self_position"));
        else if (pDefinition->GetActionType() == TOOL_ACTION_ATTACK)
            vDescription[6] = GameClient.GetGameMessage(_CTS("action_attack"));
        else if (pDefinition->GetActionType() == TOOL_ACTION_ATTACK_TOGGLE)
            vDescription[6] = GameClient.GetGameMessage(_CTS("action_attack_toggle"));
        else if (pTool->GetActionType() == TOOL_ACTION_TARGET_DUAL)
            vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_dual"));
        else if (pTool->GetActionType() == TOOL_ACTION_TARGET_DUAL_POSITION)
            vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_dual_position"));
        else if (pTool->GetActionType() == TOOL_ACTION_TARGET_VECTOR)
            vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_vector"));
        else if (pDefinition->GetActionType() == TOOL_ACTION_TARGET_CURSOR)
            vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_cursor"));

        vDescription[7] = Game.GetTargetSchemeDisplayName(pDefinition->GetTargetScheme(0));
        vDescription[8] = Game.GetEffectTypeString(pDefinition->GetCastEffectType(0));

        BUILD_FLOAT_PROPERTY(pDefinition, sRange, Range, 0, 0);
        vDescription[9] = sRange;

        BUILD_FLOAT_PROPERTY(pDefinition, sTargetRadius, TargetRadius, 0, 0);
        vDescription[10] = sTargetRadius;

        vDescription[11] = XtoA(pDefinition->GetActionType() != TOOL_ACTION_PASSIVE);

        if (pTool->GetLevel() == pTool->GetMaxLevel())
            vDescription[12] = GameClient.GetGameMessage(_CTS("ability_levelup_max"));
        else if (pTool->CanLevelUp())
            vDescription[12] = GameClient.GetGameMessage(_CTS("ability_levelup_available"));
        else
            vDescription[12] = GameClient.GetGameMessage(_CTS("ability_levelup_unavailable"));

        Trigger(UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_EFFECT_DESCRIPTION, CBuildMultiLevelText(pDefinition->GetEffectDescriptionIndex(ACTION_SCRIPT_IMPACT), uiLevel, MAX(1u, uiMaxLevel) - 1), iSlot);

        vDescription[14] = cg_tooltipFlavor ? pDefinition->GetTooltipFlavorText() : TSNULL;

        if (pTool->GetIsChanneling())
        {
            tstring sChannelTime;
            {
                bool bConstant(true);
                uint uiChannelTime(pDefinition->GetChannelTime(0));

                for (uint uiIndex(1); uiIndex < uiMaxLevel; ++uiIndex)
                {
                    if (pDefinition->GetChannelTime(uiIndex) != uiChannelTime)
                        bConstant = false;
                }

                if (bConstant)
                {
                    if (uiChannelTime != 0)
                        sChannelTime = XtoA(MsToSec(uiChannelTime), 0, 0, 0, 3);
                }
                else
                {
                    if (uiLevel >= 1)
                        sChannelTime = _CTS("^v") + XtoA(MsToSec(uiChannelTime), 0, 0, 0, 3) + _CTS("^*");
                    else
                        sChannelTime = XtoA(MsToSec(uiChannelTime), 0, 0, 0, 3);

                    for (uint uiIndex(1); uiIndex < uiMaxLevel; ++uiIndex)
                    {
                        sChannelTime += _T('/');

                        if (uiLevel >= uiIndex + 1)
                            sChannelTime += _CTS("^v") + XtoA(MsToSec(pDefinition->GetChannelTime(uiIndex)), 0, 0, 0, 3) + _CTS("^*");
                        else
                            sChannelTime += XtoA(MsToSec(pDefinition->GetChannelTime(uiIndex)), 0, 0, 0, 3);
                    }
                }
            }

            vDescription[16] = sChannelTime;

        }
        else
            vDescription[16].clear();

        BUILD_FLOAT_PROPERTY(pDefinition, sActiveManaCost, ActiveManaCost, 0, 2);
        vDescription[17] = sActiveManaCost;

        BUILD_FLOAT_PROPERTY(pDefinition, sTriggeredManaCost, TriggeredManaCost, 0, 2);
        vDescription[18] = sTriggeredManaCost;

        Trigger(UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_DESCRIPTION, vDescription, iSlot);

        static tsvector vAura(7);
        for (int i(0); i < 7; ++i)
            vAura[i].clear();

        const AuraList &cAuraList(pDefinition->GetAuraList());

        if (!cAuraList.empty())
        {
            CStateDefinition *pAuraStateDef(EntityRegistry.GetDefinition<CStateDefinition>(cAuraList.front().GetStateName(0)));

            if (pAuraStateDef != nullptr)
            {
                if (cAuraList.front().GetNoTooltip())
                    vAura[0] = _CTS("false");
                else
                    vAura[0] = _CTS("true");

                int iLines(0);
                BuildMultiLevelBonusesString(pAuraStateDef, pTool->GetLevel(), pTool->GetMaxLevel(), vModifierKeys, vAura[1], iLines);
                vAura[2] = XtoA(iLines);

                vAura[3] = _CTS("(Aura Stack Type)");
                vAura[4] = Game.GetTargetSchemeDisplayName(cAuraList.front().GetTargetScheme(0));
                vAura[5] = Game.GetEffectTypeString(cAuraList.front().GetEffectType(0));
                vAura[6] = cAuraList.front().GetRadius(0) >= 9999.0f ? GameClient.GetGameMessage(_CTS("aura_range_global")) : XtoA(INT_ROUND(cAuraList.front().GetRadius(0)));
            }
        }

        Trigger(UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_AURA, vAura, iSlot);

        static tsvector vStatusEffect(3);

        {
            for (uint ui(0); ui < vStatusEffect.size(); ++ui)
                vStatusEffect[ui].clear();

            CStateDefinition *pStateDef(EntityRegistry.GetDefinition<CStateDefinition>(pDefinition->GetStatusEffectTooltip(0)));
            if (pStateDef != nullptr)
            {
                vStatusEffect[0] = pDefinition->GetStatusEffectHeader();

                int iLines(0);
                BuildMultiLevelBonusesString(pStateDef, pTool->GetLevel(), pTool->GetMaxLevel(), vModifierKeys, vStatusEffect[1], iLines);
                vStatusEffect[2] = XtoA(iLines);
            }

            Trigger(UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_STATUS_EFFECT, vStatusEffect, iSlot);
        }

        {
            for (uint ui(0); ui < vStatusEffect.size(); ++ui)
                vStatusEffect[ui].clear();

            CStateDefinition *pStateDef(EntityRegistry.GetDefinition<CStateDefinition>(pDefinition->GetStatusEffectTooltip2(0)));
            if (pStateDef != nullptr)
            {
                vStatusEffect[0] = pDefinition->GetStatusEffectHeader2();

                int iLines(0);
                BuildMultiLevelBonusesString(pStateDef, pTool->GetLevel(), pTool->GetMaxLevel(), vModifierKeys, vStatusEffect[1], iLines);
                vStatusEffect[2] = XtoA(iLines);
            }

            Trigger(UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_STATUS_EFFECTB, vStatusEffect, iSlot);
        }

        static tsvector vTriggeredEffect(3);
        uint uiTriggerIndex(0);
        uint uiDisplayIndex(0);

        const uint NUM_DISPLAY_TRIGGERS(1);

        while (s_cTriggers[uiTriggerIndex].eScript != NUM_ACTION_SCRIPTS && uiDisplayIndex < NUM_DISPLAY_TRIGGERS)
        {
            uint uiString(pDefinition->GetEffectDescriptionIndex(s_cTriggers[uiTriggerIndex].eScript));
            if (uiString == INVALID_INDEX)
            {
                ++uiTriggerIndex;
                continue;
            }

            for (int i(0); i < 3; ++i)
                vTriggeredEffect[i].clear();

            vTriggeredEffect[0] = _CTS("true");
            vTriggeredEffect[1] = GameClient.GetGameMessage(s_cTriggers[uiTriggerIndex].sLabel);

            Trigger(UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_TRIGGERED_EFFECT_DESCRIPTION, CBuildMultiLevelText(uiString, uiLevel, MAX(1u, uiMaxLevel) - 1), iSlot);

            Trigger(UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_TRIGGERED_EFFECT, vTriggeredEffect, iSlot);
            ++uiDisplayIndex;
            ++uiTriggerIndex;
        }

        for (; uiDisplayIndex < NUM_DISPLAY_TRIGGERS; ++uiDisplayIndex)
        {
            for (int i(0); i < 3; ++i)
                vTriggeredEffect[i].clear();

            Trigger(UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_TRIGGERED_EFFECT, vTriggeredEffect, iSlot);
            Trigger(UITRIGGER_ACTIVE_INVENTORY_MULTI_LEVEL_TRIGGERED_EFFECT_DESCRIPTION, CBuildMultiLevelText(INVALID_INDEX, 0, 0), iSlot);
        }
    }
}


/*====================
  CGameInterfaceManager::UpdateSelectedInventory
  ====================*/
void    CGameInterfaceManager::UpdateSelectedInventory(IUnitEntity *pUnit, int iStartSlot, int iEndSlot)
{
    PROFILE("CGameInterfaceManager::UpdateSelectedInventory");

    if (pUnit == nullptr)
    {
        for (int iSlot(iStartSlot); iSlot <= iEndSlot; ++iSlot)
        {
            Trigger(UITRIGGER_SELECTED_INVENTORY_EXISTS, false, iSlot);
            Trigger(UITRIGGER_SELECTED_INVENTORY_INTERFACE, _CTS("single"), iSlot);
        }

        return;
    }

    uivector vModifierKeys;
    pUnit->GetSlaveModifiers(vModifierKeys);

    for (int iSlot(iStartSlot), iDisplaySlot(iStartSlot); iSlot <= iEndSlot; ++iSlot)
    {
        ISlaveEntity *pSlave(pUnit->GetSlave(iSlot));
        IEntityTool *pTool(pUnit->GetTool(iSlot));
        IEntityItem *pItem(pUnit->GetItem(iSlot));

        if (iSlot < INVENTORY_START_STATES || iSlot > INVENTORY_END_STATES)
            iDisplaySlot = iSlot;

        if (pSlave == nullptr)
        {
            Trigger(UITRIGGER_SELECTED_INVENTORY_EXISTS, false, iDisplaySlot);
            Trigger(UITRIGGER_SELECTED_INVENTORY_INTERFACE, _CTS("single"), iDisplaySlot);
            ++iDisplaySlot;
            continue;
        }

        IEntityState *pState(pSlave->GetAsState());
        if (pState != nullptr && pState->GetIsHidden())
            continue;

        ISlaveDefinition *pDefinition(pSlave->GetDefinition<ISlaveDefinition>());

        Trigger(UITRIGGER_SELECTED_INVENTORY_EXISTS, true, iDisplaySlot);

        if (pSlave->IsAbility() && !pSlave->GetAsAbility()->GetInterface().empty())
            Trigger(UITRIGGER_SELECTED_INVENTORY_INTERFACE, pSlave->GetAsAbility()->GetInterface(), iDisplaySlot);
        else
            Trigger(UITRIGGER_SELECTED_INVENTORY_INTERFACE, _CTS("single"), iDisplaySlot);

        bool bCanActive(pTool != nullptr && pTool->GetActionType() != TOOL_ACTION_PASSIVE && !(pTool->GetLevel() < 1 && pTool->GetMaxLevel() > 0));

        Trigger(UITRIGGER_SELECTED_INVENTORY_ICON, pSlave->GetIconPath(), iDisplaySlot);
        Trigger(UITRIGGER_SELECTED_INVENTORY_RECIPE, pTool == nullptr || (!pTool->HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED) && pTool->IsItem()), iDisplaySlot);
        Trigger(UITRIGGER_SELECTED_INVENTORY_CAN_ACTIVATE, bCanActive, iDisplaySlot);

        // active, silenced, low mana, level
        static tsvector vStatus(9);
        vStatus[0] = XtoA(pTool != nullptr && (pTool->HasFlag(ENTITY_TOOL_FLAG_TOGGLE_ACTIVE) || pTool->HasFlag(ENTITY_TOOL_FLAG_CHANNEL_ACTIVE)));
        vStatus[1] = XtoA(pTool ? (pTool->IsDisabled() || (pUnit->IsStunned() && !pTool->GetNoStun())) : false);
        vStatus[2] = XtoA(pTool != nullptr && !pUnit->IsFreeCast() && ((pTool->GetManaCost() != 0.0f && pTool->GetManaCost() > pUnit->GetMana()) || (pTool->GetTriggeredManaCost() != 0.0f && pTool->GetTriggeredManaCost() > pUnit->GetMana()) || pTool->HasFlag(ENTITY_TOOL_FLAG_INVALID_COST) || !pTool->CheckTriggeredCost()) && pUnit->GetStatus() == ENTITY_STATUS_ACTIVE);
        vStatus[3] = XtoA(pSlave->GetLevel());
        
        if (pSlave->IsState())
            vStatus[4] = pSlave->GetAsState()->GetDisplayLevel() ? _CTS("1") : _CTS("0");
        else if (pTool == nullptr || (!pTool->HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED) && pTool->IsItem()))
            vStatus[4] = _CTS("0");
        else
            vStatus[4] = XtoA(pTool->GetMaxLevel());
        
        vStatus[5] = XtoA(pTool ? pTool->CanActivate() : false);
        vStatus[6] = XtoA(pTool != nullptr && pTool->HasFlag(ENTITY_TOOL_FLAG_IN_USE));
        vStatus[7] = XtoA(pTool != nullptr && pTool->IsItem() && pTool->GetAsItem()->GetAllowSharing());
        vStatus[8] = XtoA(pTool != nullptr && pTool->IsItem() && pTool->GetAsItem()->IsBorrowed());

        Trigger(UITRIGGER_SELECTED_INVENTORY_STATUS, vStatus, iDisplaySlot);

        // State specific info
        static tsvector vState(4);
        for (uint ui(0); ui < vState.size(); ++ui)
            vState[ui].clear();

        if (pState != nullptr)
        {
            vState[0] = _CTS("true");
            vState[1] = XtoA(GameClient.IsDebuff(pState->GetEffectType()));
            vState[2] = XtoA(GameClient.IsBuff(pState->GetEffectType()));
            vState[3] = XtoA(pState->IsAuraInvalid());
        }

        Trigger(UITRIGGER_SELECTED_INVENTORY_STATE, vState, iDisplaySlot);

        // Cooldown
        static tsvector vCooldown(3);
        vCooldown[0] = XtoA(pTool ? pTool->GetRemainingCooldownTime() : 0);
        vCooldown[1] = XtoA(pTool ? pTool->GetTooltipCooldownTime() : 0);
        vCooldown[2] = XtoA(pTool ? pTool->GetRemainingCooldownPercent() : 0.0f);
        Trigger(UITRIGGER_SELECTED_INVENTORY_COOLDOWN, vCooldown, iDisplaySlot);

        if (pTool != nullptr && pTool->GetTimer() != INVALID_TIME)
        {
            uint uiTimer;
            if (pTool->GetTimer() > Game.GetGameTime())
                uiTimer = pTool->GetTimer() - Game.GetGameTime();
            else
                uiTimer = 0;

            Trigger(UITRIGGER_SELECTED_INVENTORY_HAS_TIMER, true, iDisplaySlot);
            Trigger(UITRIGGER_SELECTED_INVENTORY_TIMER, uiTimer, iDisplaySlot);
        }
        else
        {
            Trigger(UITRIGGER_SELECTED_INVENTORY_HAS_TIMER, false, iDisplaySlot);
            Trigger(UITRIGGER_SELECTED_INVENTORY_TIMER, 0, iDisplaySlot);
        }

        Trigger(UITRIGGER_SELECTED_INVENTORY_CHARGES, pSlave->GetCharges(), iDisplaySlot);

        static tsvector vDescription(21);
        static tsmapts s_mapTokens;

        if (pSlave->IsItem() && !pSlave->GetAsItem()->HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED))
        {
            s_mapTokens.clear();
            s_mapTokens[_CTS("name")] = pSlave->GetDisplayName();
            vDescription[0] = GameClient.GetGameMessage(_CTS("item_recipe"), s_mapTokens);
        }
        else
        {
            vDescription[0] = pSlave->GetDisplayName();
        }

        BuildText(cg_tooltipFlavor ? pSlave->GetDescription() : TSNULL, MAX(1u, pSlave->GetLevel()) - 1, vDescription[1]);
        BuildText(pSlave->GetDescription2(), MAX(1u, pSlave->GetLevel()) - 1, vDescription[16]);

        vDescription[2] = XtoA(pTool ? pTool->GetManaCost() : 0.0f);
        vDescription[3] = XtoA(pTool != nullptr && pTool->IsItem() ? pTool->GetAsItem()->GetValue() : 0);

        // Build passive bonus list
        vDescription[4].clear();
        vDescription[5].clear();

        if (pTool != nullptr)
        {
            if (pTool->GetActionType() == TOOL_ACTION_TOGGLE)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_toggle"));
            else if (pTool->GetActionType() == TOOL_ACTION_NO_TARGET)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_no_target"));
            else if (pTool->GetActionType() == TOOL_ACTION_TARGET_POSITION)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_position"));
            else if (pTool->GetActionType() == TOOL_ACTION_TARGET_ENTITY)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_unit"));
            else if (pTool->GetActionType() == TOOL_ACTION_GLOBAL)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_global"));
            else if (pTool->GetActionType() == TOOL_ACTION_TARGET_SELF)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_self"));
            else if (pTool->GetActionType() == TOOL_ACTION_FACING)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_facing"));
            else if (pTool->GetActionType() == TOOL_ACTION_SELF_POSITION)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_self_position"));
            else if (pTool->GetActionType() == TOOL_ACTION_ATTACK)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_attack"));
            else if (pTool->GetActionType() == TOOL_ACTION_ATTACK_TOGGLE)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_attack_toggle"));
            else if (pTool->GetActionType() == TOOL_ACTION_TARGET_DUAL)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_dual"));
            else if (pTool->GetActionType() == TOOL_ACTION_TARGET_DUAL_POSITION)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_dual_position"));
            else if (pTool->GetActionType() == TOOL_ACTION_TARGET_VECTOR)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_vector"));
            else if (pTool->GetActionType() == TOOL_ACTION_TARGET_CURSOR)
                vDescription[6] = GameClient.GetGameMessage(_CTS("action_target_cursor"));
        }
        else
        {
            vDescription[6].clear();
        }

        if (pTool != nullptr)
        {
            vDescription[7] = Game.GetTargetSchemeDisplayName(pTool->GetTargetScheme());
            vDescription[8] = Game.GetEffectTypeString(pTool->GetCastEffectType());
            vDescription[9] = XtoA(pTool->GetRange(), 0, 0, 0, 1);
            vDescription[10] = XtoA(pTool->GetTargetRadius(), 0, 0, 0, 1);
            vDescription[11] = XtoA(pTool->GetActionType() != TOOL_ACTION_PASSIVE && !(pTool->GetLevel() < 1 && pTool->GetMaxLevel() > 0));

            if (pItem != nullptr && !pItem->CanUse())
            {
                CPlayer *pPurchaser(GameClient.GetPlayer(pItem->GetPurchaserClientNumber()));

                s_mapTokens.clear();
                s_mapTokens[_CTS("name")] = pPurchaser ? pPurchaser->GetName() : TSNULL;
                vDescription[12] = GameClient.GetGameMessage(_CTS("activate_item_no_share"), s_mapTokens);
            }
            else if (pItem != nullptr && pItem->IsBorrowed())
            {
                CPlayer *pPurchaser(GameClient.GetPlayer(pItem->GetPurchaserClientNumber()));

                s_mapTokens.clear();
                s_mapTokens[_CTS("name")] = pPurchaser ? pPurchaser->GetName() : TSNULL;
                vDescription[12] = GameClient.GetGameMessage(_CTS("activate_item_borrowed"), s_mapTokens);
            }
            else
                vDescription[12] = TSNULL;

            BuildText(pTool->GetEffectDescription(ACTION_SCRIPT_IMPACT), MAX(1u, pTool->GetLevel()) - 1, vDescription[13]);

            vDescription[14] = cg_tooltipFlavor ? pTool->GetTooltipFlavorText() : TSNULL;

            if (pTool->GetIsChanneling())
                vDescription[17] = XtoA(pTool->GetChannelTime());
            else
                vDescription[17].clear();

            vDescription[18] = XtoA(pTool ? pTool->GetActiveManaCost() : 0.0f);
            vDescription[19] = XtoA(pTool ? pTool->GetTriggeredManaCost() : 0.0f);
            vDescription[20] = XtoA(pTool ? pTool->GetTooltipCooldownTime() : 0);
        }
        else
        {
            vDescription[7].clear();
            vDescription[8].clear();
            vDescription[9].clear();
            vDescription[10].clear();
            vDescription[11].clear();
            vDescription[12].clear();
            vDescription[13].clear();
            vDescription[14].clear();
            vDescription[17].clear();
            vDescription[18].clear();
            vDescription[19].clear();
            vDescription[20].clear();
        }

        Trigger(UITRIGGER_SELECTED_INVENTORY_DESCRIPTION, vDescription, iDisplaySlot);

        Trigger(UITRIGGER_SELECTED_INVENTORY_DURATION, pState != nullptr && pState->GetExpireTime() != INVALID_TIME ? pState->GetExpireTime() - Game.GetGameTime() : 0, iDisplaySlot);
        Trigger(UITRIGGER_SELECTED_INVENTORY_DURATION_PERCENT, pState != nullptr && pState->GetExpireTime() != INVALID_TIME ? pState->GetRemainingLifetimePercent() : 0.0f, iDisplaySlot);

        // Build passive bonus list
        static tsvector vPassiveEffect(2);

        int iLines(0);
        BuildBonusesString(pSlave, vPassiveEffect[0], iLines);
        vPassiveEffect[1] = XtoA(iLines);

        Trigger(UITRIGGER_SELECTED_INVENTORY_PASSIVE_EFFECT, vPassiveEffect, iDisplaySlot);

        if (pTool != nullptr)
        {
            static tsvector vAura(7);
            for (int i(0); i < 7; ++i)
                vAura[i].clear();

            if (pTool->GetLevel() > 0 || pTool->GetMaxLevel() == 0)
            {
                if (pDefinition != nullptr)
                {
                    const AuraList &cAuraList(pDefinition->GetAuraList());

                    if (!cAuraList.empty())
                    {
                        CStateDefinition *pAuraStateDef(EntityRegistry.GetDefinition<CStateDefinition>(cAuraList.front().GetStateName(pTool->GetLevel())));

                        if (pAuraStateDef != nullptr)
                        {
                            vAura[0] = _CTS("true");

                            int iLines(0);
                            BuildBonusesString(pAuraStateDef, pTool->GetLevel(), vModifierKeys, vAura[1], iLines);
                            vAura[2] = XtoA(iLines);

                            vAura[3] = _CTS("(Aura Stack Type)");
                            vAura[4] = Game.GetTargetSchemeDisplayName(cAuraList.front().GetTargetScheme(pTool->GetLevel()));
                            vAura[5] = Game.GetEffectTypeString(cAuraList.front().GetEffectType(pTool->GetLevel()));
                            vAura[6] = cAuraList.front().GetRadius(pTool->GetLevel()) >= 9999.0f ? GameClient.GetGameMessage(_CTS("aura_range_global")) : XtoA(INT_ROUND(cAuraList.front().GetRadius(pTool->GetLevel())));
                        }
                    }
                }
            }

            Trigger(UITRIGGER_SELECTED_INVENTORY_AURA, vAura, iDisplaySlot);
        }

        if (pTool != nullptr)
        {
            static tsvector vStatusEffect(3);
            for (uint ui(0); ui < vStatusEffect.size(); ++ui)
                vStatusEffect[ui].clear();

            if (pTool->GetLevel() > 0 || pTool->GetMaxLevel() == 0)
            {
                CStateDefinition *pStateDef(EntityRegistry.GetDefinition<CStateDefinition>(pTool->GetStatusEffectTooltip()));
                if (pStateDef != nullptr)
                {
                    vStatusEffect[0] = pTool->GetStatusEffectHeader();

                    int iLines(0);
                    BuildBonusesString(pStateDef, pTool->GetLevel(), vModifierKeys, vStatusEffect[1], iLines);
                    vStatusEffect[2] = XtoA(iLines);
                }
            }

            Trigger(UITRIGGER_SELECTED_INVENTORY_STATUS_EFFECT, vStatusEffect, iDisplaySlot);

            for (uint ui(0); ui < vStatusEffect.size(); ++ui)
                vStatusEffect[ui].clear();

            if (pTool->GetLevel() > 0 || pTool->GetMaxLevel() == 0)
            {
                CStateDefinition *pStateDef(EntityRegistry.GetDefinition<CStateDefinition>(pTool->GetStatusEffectTooltip2()));
                if (pStateDef != nullptr)
                {
                    vStatusEffect[0] = pTool->GetStatusEffectHeader2();

                    int iLines(0);
                    BuildBonusesString(pStateDef, pTool->GetLevel(), vModifierKeys, vStatusEffect[1], iLines);
                    vStatusEffect[2] = XtoA(iLines);
                }
            }

            Trigger(UITRIGGER_SELECTED_INVENTORY_STATUS_EFFECTB, vStatusEffect, iDisplaySlot);
        }

        if (pTool == nullptr || pTool->GetLevel() > 0 || pTool->GetMaxLevel() == 0)
        {
            static tsvector vTriggeredEffect(3);
            uint uiTriggerIndex(0);
            uint uiDisplayIndex(0);

            const uint NUM_DISPLAY_TRIGGERS(1);

            while (s_cTriggers[uiTriggerIndex].eScript != NUM_ACTION_SCRIPTS && uiDisplayIndex < NUM_DISPLAY_TRIGGERS)
            {
                const tstring &sDescription(pDefinition->GetEffectDescription(s_cTriggers[uiTriggerIndex].eScript));
                if (sDescription.empty())
                {
                    ++uiTriggerIndex;
                    continue;
                }

                for (int i(0); i < 3; ++i)
                    vTriggeredEffect[i].clear();

                vTriggeredEffect[0] = _CTS("true");
                vTriggeredEffect[1] = GameClient.GetGameMessage(s_cTriggers[uiTriggerIndex].sLabel);
                BuildText(sDescription, MAX(1u, pSlave->GetLevel()) - 1, vTriggeredEffect[2]);

                Trigger(UITRIGGER_SELECTED_INVENTORY_TRIGGERED_EFFECT, vTriggeredEffect, iDisplaySlot);
                ++uiDisplayIndex;
                ++uiTriggerIndex;
            }

            for (; uiDisplayIndex < NUM_DISPLAY_TRIGGERS; ++uiDisplayIndex)
            {
                for (int i(0); i < 3; ++i)
                    vTriggeredEffect[i].clear();

                Trigger(UITRIGGER_SELECTED_INVENTORY_TRIGGERED_EFFECT, vTriggeredEffect, iDisplaySlot);
            }
        }

        uint uiString(pDefinition->GetEffectDescriptionIndex(ACTION_SCRIPT_ACTIVATE_COST));
        if (uiString != INVALID_INDEX)
            Trigger(UITRIGGER_SELECTED_INVENTORY_ACTIVATE_COST, CBuildText(uiString, MAX(1u, pSlave->GetLevel()) - 1), iDisplaySlot);
        else
            Trigger(UITRIGGER_SELECTED_INVENTORY_ACTIVATE_COST, CBuildText(INVALID_INDEX, 0), iDisplaySlot);

        ++iDisplaySlot;
    }
}


/*====================
  CGameInterfaceManager::UpdateCommander
  ====================*/
void    CGameInterfaceManager::UpdateCommander()
{
    CClientCommander *pCommander(GameClient.GetClientCommander());
    if (pCommander == nullptr)
        return;

    Trigger(UITRIGGER_KEY_MODIFIER1, pCommander->GetModifier1());
    Trigger(UITRIGGER_KEY_MODIFIER2, pCommander->GetModifier2());
}


/*====================
  CGameInterfaceManager::UpdatePlayer
  ====================*/
void    CGameInterfaceManager::UpdatePlayer()
{
    PROFILE("CGameInterfaceManager::UpdatePlayer");

    CGameInfo *pGameInfo(Game.GetGameInfo());
    Trigger(UITRIGGER_STATS_STATUS, g_aStatsStatusNames[pGameInfo != nullptr ? pGameInfo->GetStatsStatus() : STATS_NULL]);

    CPlayer *pPlayer(GameClient.GetLocalPlayer());
    if (pPlayer == nullptr)
        return;
    CClientCommander *pCommander(GameClient.GetClientCommander());
    if (pCommander == nullptr)
        return;

    static tsvector vPlayerInfo(2);
    vPlayerInfo[0] = pPlayer->GetName();
    vPlayerInfo[1] = XtoA(pPlayer->GetColor());
    Trigger(UITRIGGER_PLAYER_INFO, vPlayerInfo);

    // Gold
    Trigger(UITRIGGER_PLAYER_GOLD, pPlayer->GetGold());

    IHeroEntity* pHero(pPlayer->GetHero());
    uint    uiMatchTime(Game.GetMatchTime());
    uint    uiBuyBackCost(0);
    uint    uiDeathCost(0);

    if (Game.GetGamePhase() > GAME_PHASE_ACTIVE)
        uiMatchTime = Game.GetFinalMatchTime();

    float fGoldPerMin = 0.0f;
    if (floor(MsToSec(uiMatchTime)) > 0.0f)
        fGoldPerMin = float(int(pPlayer->GetGoldEarned()) - int(pPlayer->GetStat(PLAYER_STAT_STARTING_GOLD))) / SecToMin(floorf(MsToSec(uiMatchTime)));

    if (pHero)
    {
        uiBuyBackCost = pHero->GetBuyBackCost();
        uiDeathCost = pHero->GetDeathGoldCost();
    }

    // Gold Report
    static tsvector vGoldReport(8);

    vGoldReport[0] = XtoA(pPlayer->GetStat(PLAYER_STAT_PLAYER_KILL_GOLD)); // Param0
    vGoldReport[1] = XtoA(pPlayer->GetStat(PLAYER_STAT_PLAYER_ASSIST_GOLD)); // Param1
    vGoldReport[2] = XtoA(pPlayer->GetStat(PLAYER_STAT_BUILDING_GOLD)); // Param2
    vGoldReport[3] = XtoA(pPlayer->GetStat(PLAYER_STAT_DEATH_GOLD)); // Param3
    vGoldReport[4] = XtoA(pPlayer->GetStat(PLAYER_STAT_CREEP_GOLD)); // Param4
    vGoldReport[5] = XtoA(uiBuyBackCost); // Param5
    vGoldReport[6] = XtoA(uiDeathCost); // Param6
    vGoldReport[7] = XtoA(fGoldPerMin, 0, 0, 0, 0); // Param7
        
    Trigger(UITRIGGER_GOLD_REPORT, vGoldReport);

    // Shop button
    Trigger(UITRIGGER_SHOP_ACTIVE, m_bDisplayShop);

    IUnitEntity *pSelectedUnit(pCommander->GetSelectedControlEntity());
    if (pSelectedUnit == nullptr)
    {
        Trigger(UITRIGGER_PLAYER_CAN_SHOP, false);
    }
    else
    {
        Trigger(UITRIGGER_PLAYER_CAN_SHOP, pSelectedUnit->GetCanCarryItems() && !pSelectedUnit->GetShopAccess().empty());
    }

    if (m_bDisplayShop)
        UpdateShop();

    // Stats
    static tsvector vScore(6);
    vScore[0] = XtoA(pPlayer->GetStat(PLAYER_STAT_HERO_KILLS));
    vScore[1] = XtoA(pPlayer->GetStat(PLAYER_STAT_DEATHS));
    vScore[2] = XtoA(pPlayer->GetStat(PLAYER_STAT_ASSISTS));
    vScore[3] = XtoA(pPlayer->GetStat(PLAYER_STAT_CREEP_KILLS));
    vScore[4] = XtoA(pPlayer->GetStat(PLAYER_STAT_NEUTRAL_KILLS));
    vScore[5] = XtoA(pPlayer->GetStat(PLAYER_STAT_DENIES));
    Trigger(UITRIGGER_PLAYER_SCORE, vScore);

    // Team 1 base health
    CTeamInfo *pTeam(Game.GetTeam(1));
    if (pTeam != nullptr)
    {
        IBuildingEntity *pBase(GameClient.GetBuildingEntity(pTeam->GetBaseBuildingIndex()));
        if (pBase != nullptr)
        {
            Trigger(UITRIGGER_BASE_HEALTH, pTeam->GetBaseHealthPercent(), 0);
            Trigger(UITRIGGER_BASE_HEALTH_VISIBLE, pBase->GetLastDamageTime() + 5000 > Game.GetGameTime(), 0);
        }
    }

    // Team 2 base health
    pTeam = GameClient.GetTeam(2);
    if (pTeam != nullptr)
    {
        IBuildingEntity *pBase(GameClient.GetBuildingEntity(pTeam->GetBaseBuildingIndex()));
        if (pBase != nullptr)
        {
            Trigger(UITRIGGER_BASE_HEALTH, pTeam->GetBaseHealthPercent(), 1);
            Trigger(UITRIGGER_BASE_HEALTH_VISIBLE, pBase->GetLastDamageTime() + 5000 > Game.GetGameTime(), 1);
        }
    }

    UpdateActiveUnit(pCommander->GetSelectedControlEntity());

    UpdateStash(pPlayer->GetHero(), pSelectedUnit);
    UpdateActiveInventory(pPlayer->GetHero(), INVENTORY_START_STASH, INVENTORY_END_STASH);
}


/*====================
  CGameInterfaceManager::UpdateHero
  ====================*/
void    CGameInterfaceManager::UpdateHero()
{
    PROFILE("CGameInterfaceManager::UpdateHero");

    CPlayer *pPlayer(GameClient.GetLocalPlayer());
    if (pPlayer == nullptr)
        return;

    IHeroEntity *pHero(pPlayer->GetHero());
    Trigger(UITRIGGER_HERO_INDEX, pHero ? pHero->GetIndex() : INVALID_INDEX);
    Trigger(UITRIGGER_HERO_NAME, pHero ? pHero->GetDisplayName() : TSNULL);
    Trigger(UITRIGGER_HERO_ICON, pHero ? pHero->GetIconPath() : TSNULL);
    Trigger(UITRIGGER_HERO_PORTRAIT, pHero ? pHero->GetPortraitPath() : TSNULL);
    Trigger(UITRIGGER_HERO_STATUS, pHero ? pHero->GetStatus() == ENTITY_STATUS_ACTIVE : false);
    static tsvector vExperience(4);
    vExperience[0] = XtoA(pHero ? pHero->GetExperience() : 0.0f);
    vExperience[1] = XtoA(pHero ? pHero->GetExperienceForNextLevel() - pHero->GetExperienceForCurrentLevel() : 0.0f);
    vExperience[2] = XtoA(pHero ? pHero->GetPercentNextLevel() : 1.0f);
    vExperience[3] = XtoA(pHero ? pHero->GetExperience() - pHero->GetExperienceForCurrentLevel() : 0.0f);
    Trigger(UITRIGGER_HERO_EXPERIENCE, vExperience);

    static tsvector vHealth(3);
    vHealth[0] = XtoA(pHero ? pHero->GetHealth() : 0.0f);
    vHealth[1] = XtoA(pHero ? pHero->GetMaxHealth() : 0.0f);
    vHealth[2] = XtoA(pHero ? pHero->GetHealthPercent() : 1.0f);
    Trigger(UITRIGGER_HERO_HEALTH, vHealth);

    static tsvector vMana(3);
    vMana[0] = XtoA(pHero ? pHero->GetMana() : 0.0f);
    vMana[1] = XtoA(pHero ? pHero->GetMaxMana() : 0.0f);
    vMana[2] = XtoA(pHero ? pHero->GetManaPercent() : 1.0f);
    Trigger(UITRIGGER_HERO_MANA, vMana);

    static tsvector vHealthRegen(2);
    vHealthRegen[0] = XtoA(pHero ? pHero->GetBaseHealthRegen() : 0.0f);
    vHealthRegen[1] = XtoA(pHero ? pHero->GetHealthRegen() : 0.0f);
    Trigger(UITRIGGER_HERO_HEALTHREGEN, vHealthRegen);

    static tsvector vManaRegen(2);
    vManaRegen[0] = XtoA(pHero ? pHero->GetBaseManaRegen() : 0.0f);
    vManaRegen[1] = XtoA(pHero ? pHero->GetManaRegen() : 0.0f);
    Trigger(UITRIGGER_HERO_MANAREGEN, vManaRegen);

    static tsvector vRespawn(3);
    if (pHero != nullptr && pHero->GetRespawnTime() != INVALID_TIME)
    {
        vRespawn[0] = XtoA(MAX(int(pHero->GetRemainingRespawnTime() - GameClient.GetServerFrameLength()), 0));
        vRespawn[1] = XtoA(pHero->GetRespawnDuration());
        vRespawn[2] = XtoA(pHero->GetRespawnPercent());
    }
    else
    {
        vRespawn[0] = TSNULL;
        vRespawn[1] = _CTS("0");
        vRespawn[2] = _CTS("1.0");
    }
    Trigger(UITRIGGER_HERO_RESPAWN, vRespawn);

    static tsvector vLevel(2);
    vLevel[0] = XtoA(pHero ? pHero->GetLevel() : 0);
    vLevel[1] = XtoA(pHero ? pHero->GetAvailablePoints() : 0);
    Trigger(UITRIGGER_HERO_LEVEL, vLevel);

    Trigger(UITRIGGER_HERO_BUYBACK, pHero ? pHero->GetStatus() == ENTITY_STATUS_CORPSE || pHero->GetStatus() == ENTITY_STATUS_DORMANT : false);
    Trigger(UITRIGGER_HERO_BUYBACK_COST, pHero ? pHero->GetBuyBackCost() : 0);

#if 0 // This is the wrong place for this...
    bool bForced(false);

    if (pHero != nullptr && pHero->GetBrain().GetCurrentBehavior() != nullptr && pHero->GetBrain().GetCurrentBehavior()->IsForced())
        bForced = true;

    Trigger(UITRIGGER_COMMAND_ENABLED_MOVE, pHero ? !bForced && !pHero->IsImmobilized() : false);
    Trigger(UITRIGGER_COMMAND_ENABLED_ATTACK, pHero ? !bForced && !pHero->IsDisarmed() : false);
    Trigger(UITRIGGER_COMMAND_ENABLED_STOP, pHero ? !bForced : false);
    Trigger(UITRIGGER_COMMAND_ENABLED_HOLD, pHero ? !bForced : false);
    Trigger(UITRIGGER_COMMAND_ENABLED_PATROL, pHero ? !bForced && !pHero->IsImmobilized() : false);
#else
    Trigger(UITRIGGER_COMMAND_ENABLED_MOVE, true);
    Trigger(UITRIGGER_COMMAND_ENABLED_ATTACK, true);
    Trigger(UITRIGGER_COMMAND_ENABLED_STOP, true);
    Trigger(UITRIGGER_COMMAND_ENABLED_HOLD, true);
    Trigger(UITRIGGER_COMMAND_ENABLED_PATROL, true);
#endif

    UpdateHeroInventory(pHero, INVENTORY_START_STATES, INVENTORY_END_STATES);
}


/*====================
  CGameInterfaceManager::UpdateAllies
  ====================*/
void    CGameInterfaceManager::UpdateAllies()
{
    PROFILE("CGameInterfaceManager::UpdateAllies");

    CPlayer *pLocalPlayer(GameClient.GetLocalPlayer());
    if (pLocalPlayer == nullptr)
        return;

    CTeamInfo *pTeam(GameClient.GetTeam(pLocalPlayer->GetTeam()));
    if (pTeam == nullptr)
        return;

    Trigger(UITRIGGER_ALLY_DISPLAY, bool(cg_displayAllies));

    uint uiTotalPlayers(0);
    for (uint uiPlayer(0); uiPlayer < MAX_DISPLAY_PLAYERSPERTEAM; ++uiPlayer)
    {
        CPlayer *pClient(GameClient.GetPlayer(pTeam->GetClientIDFromTeamIndex(uiPlayer)));

        if (pClient == nullptr || pClient == pLocalPlayer || pClient->HasFlags(PLAYER_FLAG_TERMINATED))
            continue;

        IHeroEntity *pHero(pClient->GetHero());
        if (pHero == nullptr)
            continue;

        Trigger(UITRIGGER_ALLY_EXISTS, true, uiTotalPlayers);
        Trigger(UITRIGGER_ALLY_INDEX, pHero->GetIndex(), uiTotalPlayers);

        static tsvector vPlayerInfo(3);
        vPlayerInfo[0] = pClient->GetName();
        vPlayerInfo[1] = XtoA(pClient->GetColor());
        vPlayerInfo[2] = XtoA(pClient->GetClientNumber());
        Trigger(UITRIGGER_ALLY_PLAYER_INFO, vPlayerInfo, uiTotalPlayers);

        static tsvector vHeroInfo(3);
        vHeroInfo[0] = pHero->GetDisplayName();
        vHeroInfo[1] = pHero->GetIconPath();
        vHeroInfo[2] = XtoA(pHero->GetLevel());
        Trigger(UITRIGGER_ALLY_HERO_INFO, vHeroInfo, uiTotalPlayers);

        static tsvector vHealth(3);
        vHealth[0] = XtoA(pHero->GetHealth());
        vHealth[1] = XtoA(pHero->GetMaxHealth());
        vHealth[2] = XtoA(pHero->GetHealthPercent());
        Trigger(UITRIGGER_ALLY_HEALTH, vHealth, uiTotalPlayers);

        static tsvector vMana(3);
        vMana[0] = XtoA(pHero->GetMana());
        vMana[1] = XtoA(pHero->GetMaxMana());
        vMana[2] = XtoA(pHero->GetManaPercent());
        Trigger(UITRIGGER_ALLY_MANA, vMana, uiTotalPlayers);

        Trigger(UITRIGGER_ALLY_STATUS, pHero->GetStatus() == ENTITY_STATUS_ACTIVE, uiTotalPlayers);

        static tsvector vRespawn(3);
        if (pHero != nullptr && pHero->GetRespawnTime() != INVALID_TIME)
        {
            vRespawn[0] = XtoA(MAX(int(pHero->GetRemainingRespawnTime() - GameClient.GetServerFrameLength()), 0));
            vRespawn[1] = XtoA(pHero->GetRespawnDuration());
            vRespawn[2] = XtoA(pHero->GetRespawnPercent());
        }
        else
        {
            vRespawn[0] = TSNULL;
            vRespawn[1] = _CTS("0");
            vRespawn[2] = _CTS("1.0");
        }
        Trigger(UITRIGGER_ALLY_RESPAWN, vRespawn, uiTotalPlayers);

        static tsvector vDamage(2);
        vDamage[0] = XtoA(pHero->GetAdjustedAttackDamageMin() + pHero->GetBonusDamage());
        vDamage[1] = XtoA(pHero->GetAdjustedAttackDamageMax() + pHero->GetBonusDamage());
        Trigger(UITRIGGER_ALLY_DAMAGE, vDamage, uiTotalPlayers);

        Trigger(UITRIGGER_ALLY_ARMOR, pHero->GetArmor(), uiTotalPlayers);
        Trigger(UITRIGGER_ALLY_MAGIC_ARMOR, pHero->GetMagicArmor(), uiTotalPlayers);
        Trigger(UITRIGGER_ALLY_MOVE_SPEED, pHero->GetMoveSpeed(), uiTotalPlayers);
        Trigger(UITRIGGER_ALLY_ATTACK_SPEED, pHero->GetAttackSpeed(), uiTotalPlayers);
        Trigger(UITRIGGER_ALLY_STRENGTH, pHero->GetStrength(), uiTotalPlayers);
        Trigger(UITRIGGER_ALLY_AGILITY, pHero->GetAgility(), uiTotalPlayers);
        Trigger(UITRIGGER_ALLY_INTELLIGENCE, pHero->GetIntelligence(), uiTotalPlayers);

        Trigger(UITRIGGER_ALLY_GOLD, pClient->GetGold(), uiTotalPlayers);

        Trigger(UITRIGGER_ALLY_DISCONNECTED, pClient->IsDisconnected(), uiTotalPlayers);
        Trigger(UITRIGGER_ALLY_DISCONNECT_TIME, MAX(int(pClient->GetTerminationTime() - GameClient.GetGameTime()), 0), uiTotalPlayers);
        Trigger(UITRIGGER_ALLY_LOADING_PERCENT, pClient->HasFlags(PLAYER_FLAG_LOADING) ? pClient->GetLoadingProgress() : 1.0f, uiTotalPlayers);
        Trigger(UITRIGGER_ALLY_AFK, pClient->HasFlags(PLAYER_FLAG_IS_AFK), uiTotalPlayers);

        static tsvector vSharing(2);
        uint uiSharing(0);
        if (pClient->HasSharedPartialControl(pLocalPlayer->GetClientNumber()))
            uiSharing += 1;
        if (pClient->HasSharedFullControl(pLocalPlayer->GetClientNumber()))
            uiSharing += 2;
        uint uiShared(0);
        if (pLocalPlayer->HasSharedPartialControl(pClient->GetClientNumber()))
            uiShared += 1;
        if (pLocalPlayer->HasSharedFullControl(pClient->GetClientNumber()))
            uiShared += 2;
        vSharing[0] = XtoA(uiSharing);
        vSharing[1] = XtoA(uiShared);
        Trigger(UITRIGGER_ALLY_CONTROL_SHARING, vSharing, uiTotalPlayers);

        Trigger(UITRIGGER_ALLY_NO_HELP, pLocalPlayer->GetNoHelp(pClient), uiTotalPlayers);

        ////
        static tsvector vVoice(2);
        vVoice[0] = XtoA(VoiceManager.IsTalking(pClient->GetClientNumber()));
        vVoice[1] = XtoA(VoiceManager.IsClientMuted(pClient->GetClientNumber()));

        Trigger(UITRIGGER_ALLY_VOICE, vVoice, uiTotalPlayers);
        ////

        UpdateAllyAbility(pHero, INVENTORY_START_ABILITIES + 0, UITRIGGER_ALLY_ABILITY_0_INFO, uiTotalPlayers);
        UpdateAllyAbility(pHero, INVENTORY_START_ABILITIES + 1, UITRIGGER_ALLY_ABILITY_1_INFO, uiTotalPlayers);
        UpdateAllyAbility(pHero, INVENTORY_START_ABILITIES + 2, UITRIGGER_ALLY_ABILITY_2_INFO, uiTotalPlayers);
        UpdateAllyAbility(pHero, INVENTORY_START_ABILITIES + 3, UITRIGGER_ALLY_ABILITY_3_INFO, uiTotalPlayers);
        UpdateAllyAbility(pHero, INVENTORY_START_ABILITIES + 4, UITRIGGER_ALLY_ABILITY_4_INFO, uiTotalPlayers);

        ++uiTotalPlayers;
    }

    for (uint uiPlayer(uiTotalPlayers); uiPlayer < MAX_ALLY_HEROES; ++uiPlayer)
    {
        Trigger(UITRIGGER_ALLY_EXISTS, false, uiTotalPlayers);
        ++uiTotalPlayers;
    }
}


/*====================
  CGameInterfaceManager::UpdateAllyAbility
  ====================*/
void    CGameInterfaceManager::UpdateAllyAbility(IUnitEntity *pUnit, int iSlot, EGameUITrigger eTrigger, uint uiDisplaySlot)
{
    PROFILE("CGameInterfaceManager::UpdateAllyAbility");

    IEntityTool *pTool(pUnit->GetTool(iSlot));

    if (pTool != nullptr && pTool->IsAbility() && pTool->GetAsAbility()->GetSubSlot() != -1)
    {
        int iSubSlot(pTool->GetAsAbility()->GetSubSlot());
        pTool = pUnit->GetTool(iSubSlot);
    }

    if (pTool == nullptr)
    {
        static tsvector vInfoClear(12);
        vInfoClear[0] = _CTS("false");

        Trigger(eTrigger, vInfoClear, iSlot);
        return;
    }

    static tsvector vInfo(13);
    vInfo[0] = _CTS("true");
    vInfo[1] = XtoA(pTool->GetLevel() < 1 && pTool->GetMaxLevel() > 0);
    vInfo[2] = XtoA(pTool->CanActivate());
    vInfo[3] = XtoA(pTool->HasFlag(ENTITY_TOOL_FLAG_TOGGLE_ACTIVE));
    vInfo[4] = XtoA(pTool->IsDisabled() || (pUnit->IsStunned() && !pTool->GetNoStun()));
    vInfo[5] = XtoA(pTool->GetManaCost() != 0.0f && pTool->GetManaCost() > pUnit->GetMana() && pUnit->GetStatus() == ENTITY_STATUS_ACTIVE);
    vInfo[6] = XtoA(pTool->GetLevel());
    //vInfo[7] = XtoA(pTool->GetRemainingCooldownTime());
    //vInfo[8] = XtoA(pTool->GetCooldownTime());
    //vInfo[9] = XtoA(pTool->GetRemainingCooldownPercent());
    vInfo[10] = pTool->GetDisplayName();
    vInfo[11] = pTool->GetIconPath();
    vInfo[12] = XtoA(pTool->GetActionType() == TOOL_ACTION_PASSIVE);
    Trigger(eTrigger, vInfo, uiDisplaySlot);

    Trigger(eTrigger + 1, INT_CEIL(pTool->GetRemainingCooldownTime() / 1000.0f), uiDisplaySlot);
}


/*====================
  CGameInterfaceManager::UpdateActiveUnit
  ====================*/
void    CGameInterfaceManager::UpdateActiveUnit(IUnitEntity *pUnit)
{
    PROFILE("CGameInterfaceManager::UpdateActiveUnit");

    IHeroEntity *pHero(pUnit ? pUnit->GetAsHero() : nullptr);
    //IGadgetEntity *pGadget(pUnit ? pUnit->GetAsGadget() : nullptr);
    CPlayer *pOwner(pUnit ? pUnit->GetOwnerPlayer() : nullptr);

    Trigger(UITRIGGER_ACTIVE_INDEX, pUnit ? pUnit->GetIndex() : INVALID_INDEX);
    Trigger(UITRIGGER_ACTIVE_NAME, pUnit ? pUnit->GetDisplayName() : TSNULL);
    Trigger(UITRIGGER_ACTIVE_ICON, pUnit ? pUnit->GetIconPath() : TSNULL);
    Trigger(UITRIGGER_ACTIVE_PORTRAIT, pUnit ? pUnit->GetPortraitPath() : TSNULL);
    Trigger(UITRIGGER_ACTIVE_MODEL, pUnit ? pUnit->GetModelPath() : TSNULL);
    Trigger(UITRIGGER_ACTIVE_EFFECT, pUnit ? pUnit->GetPassiveEffectPath() : TSNULL);
    Trigger(UITRIGGER_ACTIVE_STATUS, pUnit ? pUnit->GetStatus() == ENTITY_STATUS_ACTIVE : false);
    Trigger(UITRIGGER_ACTIVE_ILLUSION, pUnit ? pUnit->IsIllusion() : false);

    static tsvector vPlayerInfo(2);
    vPlayerInfo[0] = pOwner ? pOwner->GetName() : TSNULL;
    vPlayerInfo[1] = XtoA(pOwner ? pOwner->GetColor() : WHITE);
    Trigger(UITRIGGER_ACTIVE_PLAYER_INFO, vPlayerInfo);

    static tsvector vHealth(4);
    vHealth[0] = XtoA(pUnit ? pUnit->GetHealth() : 0.0f);
    vHealth[1] = XtoA(pUnit ? pUnit->GetMaxHealth() : 0.0f);
    vHealth[2] = XtoA(pUnit ? pUnit->GetHealthPercent() : 0.0f);
    vHealth[3] = XtoA(pUnit ? pUnit->GetHealthShadow() : 0.0f);
    Trigger(UITRIGGER_ACTIVE_HEALTH, vHealth);

    static tsvector vMana(4);
    vMana[0] = XtoA(pUnit ? pUnit->GetMana() : 0.0f);
    vMana[1] = XtoA(pUnit ? pUnit->GetMaxMana() : 0.0f);
    vMana[2] = XtoA(pUnit ? pUnit->GetManaPercent() : 0.0f);
    vMana[3] = XtoA(pUnit ? pUnit->GetManaShadow() : 0.0f);
    Trigger(UITRIGGER_ACTIVE_MANA, vMana);

    static tsvector vHealthRegen(2);
    vHealthRegen[0] = XtoA(pUnit ? pUnit->GetBaseHealthRegen() : 0.0f);
    vHealthRegen[1] = XtoA(pUnit ? pUnit->GetHealthRegen() : 0.0f);
    Trigger(UITRIGGER_ACTIVE_HEALTHREGEN, vHealthRegen);

    static tsvector vManaRegen(2);
    vManaRegen[0] = XtoA(pUnit ? pUnit->GetBaseManaRegen() : 0.0f);
    vManaRegen[1] = XtoA(pUnit ? pUnit->GetManaRegen() : 0.0f);
    Trigger(UITRIGGER_ACTIVE_MANAREGEN, vManaRegen);

    static tsvector vLevel(4);
    vLevel[0] = XtoA(pUnit ? pUnit->GetLevel() : 0);
    vLevel[1] = XtoA(pHero ? pHero->GetAvailablePoints() : 0);
    vLevel[2] = XtoA(cg_displayLevelup);
    vLevel[3] = XtoA(pUnit ? (pUnit->IsPet() || pUnit->IsGadget() || pUnit->IsHero()) : false);
    Trigger(UITRIGGER_ACTIVE_LEVEL, vLevel);

    static tsvector vExperience(5);
    vExperience[0] = XtoA(pHero != nullptr);
    vExperience[1] = XtoA(pHero ? pHero->GetExperience() : 0.0f);
    vExperience[2] = XtoA(pHero ? pHero->GetExperienceForNextLevel() - pHero->GetExperienceForCurrentLevel() : 0.0f);
    vExperience[3] = XtoA(pHero ? pHero->GetPercentNextLevel() : 1.0f);
    vExperience[4] = XtoA(pHero ? pHero->GetExperience() - pHero->GetExperienceForCurrentLevel() : 0.0f);
    Trigger(UITRIGGER_ACTIVE_EXPERIENCE, vExperience);

    static tsvector vDamage(3);
    vDamage[0] = XtoA(pUnit ? pUnit->GetAttackDamageMin() : 0.0f);
    vDamage[1] = XtoA(pUnit ? pUnit->GetAttackDamageMax() : 0.0f);
    vDamage[2] = XtoA(pUnit ? ((pUnit->GetAttackDamageMin() + pUnit->GetAttackDamageMax()) * 0.5f * pUnit->GetBaseDamageMultiplier() + pUnit->GetBonusDamage()) * pUnit->GetTotalDamageMultiplier() - ((pUnit->GetAttackDamageMin() + pUnit->GetAttackDamageMax()) * 0.5f) : 0.0f);
    Trigger(UITRIGGER_ACTIVE_DAMAGE, vDamage);

    static tsvector vArmor(3);
    vArmor[0] = XtoA(pUnit ? pUnit->GetBaseArmor() : 0.0f);
    vArmor[1] = XtoA(pUnit ? pUnit->GetArmor() : 0.0f);
    vArmor[2] = XtoA(pUnit ? Game.GetArmorDamageAdjustment(pUnit->GetArmorType(), pUnit->GetArmor()) : 0.0f);
    Trigger(UITRIGGER_ACTIVE_ARMOR, vArmor);

    static tsvector vMagicArmor(3);
    vMagicArmor[0] = XtoA(pUnit ? pUnit->GetBaseMagicArmor() : 0.0f);
    vMagicArmor[1] = XtoA(pUnit ? pUnit->GetMagicArmor() : 0.0f);
    vMagicArmor[2] = XtoA(pUnit ? Game.GetArmorDamageAdjustment(pUnit->GetMagicArmorType(), pUnit->GetMagicArmor()) : 0.0f);
    Trigger(UITRIGGER_ACTIVE_MAGIC_ARMOR, vMagicArmor);

    static tsvector vMoveSpeed(2);
    vMoveSpeed[0] = XtoA(pUnit ? pUnit->GetBaseMoveSpeed() : 0.0f);
    vMoveSpeed[1] = XtoA(pUnit ? pUnit->GetMoveSpeed() : 0.0f);
    Trigger(UITRIGGER_ACTIVE_MOVE_SPEED, vMoveSpeed);

    static tsvector vAttackSpeed(2);
    vAttackSpeed[0] = XtoA(pUnit ? pUnit->GetBaseAttackSpeed() * 100.0f : 0.0f);
    vAttackSpeed[1] = XtoA(pUnit ? pUnit->GetAttackSpeed() * 100.0f : 0.0f);
    Trigger(UITRIGGER_ACTIVE_ATTACK_SPEED, vAttackSpeed);

    static tsvector vCastSpeed(2);
    vCastSpeed[0] = XtoA(pUnit ? pUnit->GetBaseCastSpeed() * 100.0f : 0.0f);
    vCastSpeed[1] = XtoA(pUnit ? pUnit->GetCastSpeed() * 100.0f : 0.0f);
    Trigger(UITRIGGER_ACTIVE_CAST_SPEED, vCastSpeed);

    static tsvector vAttackRange(2);
    vAttackRange[0] = XtoA(pUnit ? pUnit->GetBaseAttackRange() : 0.0f);
    vAttackRange[1] = XtoA(pUnit ? pUnit->GetAttackRange() : 0.0f);
    Trigger(UITRIGGER_ACTIVE_ATTACK_RANGE, vAttackRange);

    static tsvector vAttackCooldown(2);
    vAttackCooldown[0] = XtoA(pUnit ? pUnit->GetAttackCooldown() : 0.0f);
    vAttackCooldown[1] = XtoA(pUnit ? INT_CEIL(pUnit->GetAdjustedAttackCooldown() / float(Game.GetServerFrameLength())) * Game.GetServerFrameLength() : 0.0f);
    Trigger(UITRIGGER_ACTIVE_ATTACK_COOLDOWN, vAttackCooldown);

    static tsvector vStrength(2);
    vStrength[0] = XtoA(pHero ? pHero->GetBaseStrength() : 0.0f);
    vStrength[1] = XtoA(pHero ? pHero->GetStrength() : 0.0f);
    Trigger(UITRIGGER_ACTIVE_STRENGTH, vStrength);

    static tsvector vAgility(2);
    vAgility[0] = XtoA(pHero ? pHero->GetBaseAgility() : 0.0f);
    vAgility[1] = XtoA(pHero ? pHero->GetAgility() : 0.0f);
    Trigger(UITRIGGER_ACTIVE_AGILITY, vAgility);

    static tsvector vIntelligence(3);
    vIntelligence[0] = XtoA(pHero ? pHero->GetBaseIntelligence() : 0.0f);
    vIntelligence[1] = XtoA(pHero ? pHero->GetIntelligence() : 0.0f);
    Trigger(UITRIGGER_ACTIVE_INTELLIGENCE, vIntelligence);

    static tsvector vAttributes(4);
    vAttributes[0] = XtoA(pHero ? pHero->GetStrength() : 0.0f);
    vAttributes[1] = XtoA(pHero ? pHero->GetAgility() : 0.0f);
    vAttributes[2] = XtoA(pHero ? pHero->GetIntelligence() : 0.0f);
    vAttributes[3] = XtoA(pHero ? pHero->GetPrimaryAttribute() : ATTRIBUTE_INVALID);
    Trigger(UITRIGGER_ACTIVE_ATTRIBUTES, vAttributes);

    static tsvector vLifetime(3);
    vLifetime[0] = XtoA(pUnit ? pUnit->GetRemainingLifetime() : 0);
    vLifetime[1] = XtoA(pUnit ? pUnit->GetActualLifetime() : 0);
    vLifetime[2] = XtoA(pUnit ? pUnit->GetRemainingLifetimePercent() : 0.0f);
    Trigger(UITRIGGER_ACTIVE_LIFETIME, vLifetime);

    Trigger(UITRIGGER_ACTIVE_HAS_INVENTORY, pUnit ? pUnit->GetCanCarryItems() : false);
    Trigger(UITRIGGER_ACTIVE_HAS_ATTRIBUTES, pHero != nullptr);

    static tsvector vAttackInfo(1);
    vAttackInfo[0] = pUnit ? Game.GetAttackTypeDisplayName(pUnit->GetAttackType()) : TSNULL;
    Trigger(UITRIGGER_ACTIVE_ATTACK_INFO, vAttackInfo);

    static tsvector vDefenseInfo(1);
    vDefenseInfo[0] = pUnit ? Game.GetAttackTypeDisplayName(pUnit->GetAttackType()) : TSNULL;
    Trigger(UITRIGGER_ACTIVE_DEFENSE_INFO, vDefenseInfo);

    static tsvector vAttributeInfo(12);
    vAttributeInfo[0] = pUnit ? pUnit->GetCombatType() : TSNULL;

    vAttributeInfo[1].clear();
    if (pHero != nullptr)
    {
        if (pHero->GetPrimaryAttribute() == ATTRIBUTE_STRENGTH)
            vAttributeInfo[1] = GameClient.GetGameMessage(_CTS("attribute_strength"));
        else if (pHero->GetPrimaryAttribute() == ATTRIBUTE_AGILITY)
            vAttributeInfo[1] = GameClient.GetGameMessage(_CTS("attribute_agility"));
        else if (pHero->GetPrimaryAttribute() == ATTRIBUTE_INTELLIGENCE)
            vAttributeInfo[1] = GameClient.GetGameMessage(_CTS("attribute_intelligence"));
    }

    vAttributeInfo[2] = XtoA(pHero ? pHero->GetStrengthPerLevel() : 0.0f);
    vAttributeInfo[3] = XtoA(pHero ? pHero->GetAgilityPerLevel() : 0.0f);
    vAttributeInfo[4] = XtoA(pHero ? pHero->GetIntelligencePerLevel() : 0.0f);

    static tsmapts s_mapTokens;

    // Strength bonuses
    {
        int iLines(0);

        vAttributeInfo[5].clear();

        if (pHero != nullptr && pHero->GetPrimaryAttribute() == ATTRIBUTE_STRENGTH)
        {
            s_mapTokens[_CTS("value")] = XtoA(INT_FLOOR(1.0f), FMT_SIGN);
            vAttributeInfo[5] += GameClient.GetGameMessage(_CTS("attribute_damage_bonus"), s_mapTokens) + _CTS("\n");
            ++iLines;
        }

        s_mapTokens[_CTS("value")] = XtoA(INT_FLOOR(hero_hpPerStr), FMT_SIGN);
        vAttributeInfo[5] += GameClient.GetGameMessage(_CTS("attribute_health_bonus"), s_mapTokens) + _CTS("\n");
        ++iLines;

        s_mapTokens[_CTS("value")] = XtoA(hero_hpRegenPerStr, FMT_SIGN, 0, 0, 2);
        vAttributeInfo[5] += GameClient.GetGameMessage(_CTS("attribute_health_regen_bonus"), s_mapTokens) + _CTS("\n");
        ++iLines;

        vAttributeInfo[6] = XtoA(iLines);
    }

    // Agility bonuses
    {
        int iLines(0);

        vAttributeInfo[7].clear();

        if (pHero != nullptr && pHero->GetPrimaryAttribute() == ATTRIBUTE_AGILITY)
        {
            s_mapTokens[_CTS("value")] = XtoA(INT_FLOOR(1.0f), FMT_SIGN);
            vAttributeInfo[7] += GameClient.GetGameMessage(_CTS("attribute_damage_bonus"), s_mapTokens) + _CTS("\n");
            ++iLines;
        }

        s_mapTokens[_CTS("value")] = XtoA(hero_attackSpeedPerAgi * 100.0f, FMT_SIGN, 0, 0, 0);
        vAttributeInfo[7] += GameClient.GetGameMessage(_CTS("attribute_attack_speed_bonus"), s_mapTokens) + _CTS("\n");
        ++iLines;

        s_mapTokens[_CTS("value")] = XtoA(hero_armorPerAgi, FMT_SIGN, 0, 0, 2);
        vAttributeInfo[7] += GameClient.GetGameMessage(_CTS("attribute_armor_bonus"), s_mapTokens) + _CTS("\n");
        ++iLines;

        vAttributeInfo[8] = XtoA(iLines);
    }

    // Intelligence bonuses
    {
        int iLines(0);

        vAttributeInfo[9].clear();

        if (pHero != nullptr && pHero->GetPrimaryAttribute() == ATTRIBUTE_INTELLIGENCE)
        {
            s_mapTokens[_CTS("value")] = XtoA(INT_FLOOR(1.0f), FMT_SIGN);
            vAttributeInfo[9] += GameClient.GetGameMessage(_CTS("attribute_damage_bonus"), s_mapTokens) + _CTS("\n");
            ++iLines;
        }

        s_mapTokens[_CTS("value")] = XtoA(INT_FLOOR(hero_mpPerInt), FMT_SIGN);
        vAttributeInfo[9] += GameClient.GetGameMessage(_CTS("attribute_mana_bonus"), s_mapTokens) + _CTS("\n");
        ++iLines;

        s_mapTokens[_CTS("value")] = XtoA(hero_mpRegenPerInt, FMT_SIGN, 0, 0, 2);
        vAttributeInfo[9] += GameClient.GetGameMessage(_CTS("attribute_mana_regen_bonus"), s_mapTokens) + _CTS("\n");
        ++iLines;

        vAttributeInfo[10] = XtoA(iLines);
    }

    if (pHero != nullptr)
    {
        if (pHero->GetPrimaryAttribute() == ATTRIBUTE_STRENGTH)
            vAttributeInfo[11] = _CTS("strength");
        else if (pHero->GetPrimaryAttribute() == ATTRIBUTE_AGILITY)
            vAttributeInfo[11] = _CTS("agility");
        else if (pHero->GetPrimaryAttribute() == ATTRIBUTE_INTELLIGENCE)
            vAttributeInfo[11] = _CTS("intelligence");
        else
            vAttributeInfo[11] = _CTS("none");
    }
    else
        vAttributeInfo[11] = _CTS("none");

    Trigger(UITRIGGER_ACTIVE_ATTRIBUTE_INFO, vAttributeInfo);

    UpdateActiveInventory(pUnit, INVENTORY_START_ABILITIES, INVENTORY_END_ABILITIES);
    UpdateActiveInventory(pUnit, INVENTORY_START_STATES, INVENTORY_END_STATES);
    UpdateActiveInventory(pUnit, INVENTORY_START_BACKPACK, INVENTORY_END_BACKPACK);
    UpdateActiveAttackModifiers(pUnit);
    UpdateLevelUp(pUnit, INVENTORY_START_ABILITIES, INVENTORY_END_ABILITIES);
}


/*====================
  CGameInterfaceManager::UpdateSelectedUnits
  ====================*/
void    CGameInterfaceManager::UpdateSelectedUnits()
{
    PROFILE("CGameInterfaceManager::UpdateSelectedUnits");

    CClientCommander *pCommander(GameClient.GetClientCommander());
    if (pCommander == nullptr)
        return;

    // Single selection
    IUnitEntity *pUnit(pCommander->GetSelectedInfoEntity());
    if (pUnit != nullptr)
    {
        CPlayer *pLocalPlayer(GameClient.GetLocalPlayer());

        UpdateSelectedUnit(pUnit);

        bool bUpdated(false);
        if (pUnit->IsBuilding())
        {
            CTeamInfo *pTeam(Game.GetTeam(pUnit->GetTeam()));
            if (pTeam != nullptr)
            {
                IUnitEntity *pBase(Game.GetUnitEntity(pTeam->GetBaseBuildingIndex()));
                if (pBase != nullptr)
                {
                    UpdateActiveInventory(pBase, INVENTORY_START_SHARED_ABILITIES, INVENTORY_END_SHARED_ABILITIES);
                    bUpdated = true;
                }
            }
        }
        
        if (!bUpdated)
            UpdateActiveInventory(pUnit, INVENTORY_START_SHARED_ABILITIES, INVENTORY_END_SHARED_ABILITIES);

        if (pLocalPlayer != nullptr && pLocalPlayer->GetTeam() == TEAM_SPECTATOR)
            UpdateSelectedInventory(pUnit, INVENTORY_START_ABILITIES, INVENTORY_END_ABILITIES);

        UpdateSelectedInventory(pUnit, INVENTORY_START_BACKPACK, INVENTORY_END_BACKPACK);
        UpdateSelectedInventory(pUnit, INVENTORY_START_STATES, INVENTORY_END_STATES);

        for (uint ui(1); ui < MAX_SELECTED_UNITS; ++ui)
            Trigger(UITRIGGER_SELECTED_VISIBLE, false, ui);

        return;
    }

    const uiset &setInfoSelection(pCommander->GetSelectedInfoEntities());

    // No info-selection
    if (setInfoSelection.empty())
    {
        const uiset &setControlSelection(pCommander->GetSelectedControlEntities());

        if (setControlSelection.size() == 1)
        {
            for (uint ui(0); ui < MAX_SELECTED_UNITS; ++ui)
                Trigger(UITRIGGER_SELECTED_VISIBLE, false, ui);

            // Shared abilities
            IUnitEntity *pUnit(pCommander->GetSelectedControlEntity());
            if (pUnit != nullptr)
            {
                bool bUpdated(false);
                CTeamInfo *pTeam(Game.GetTeam(pUnit->GetTeam()));
                if (pTeam != nullptr)
                {
                    IUnitEntity *pBase(Game.GetUnitEntity(pTeam->GetBaseBuildingIndex()));
                    if (pBase != nullptr)
                    {
                        UpdateActiveInventory(pBase, INVENTORY_START_SHARED_ABILITIES, INVENTORY_END_SHARED_ABILITIES);
                        bUpdated = true;
                    }
                }
                
                if (!bUpdated)
                    UpdateActiveInventory(pUnit, INVENTORY_START_SHARED_ABILITIES, INVENTORY_END_SHARED_ABILITIES);

                return;
            }
        }
        else
        {
            // Multi-control selection
            uint uiIndex(0);
            for (uiset_cit cit(setControlSelection.begin()); cit != setControlSelection.end(); ++cit)
            {
                IUnitEntity *pUnit(Game.GetUnitEntity(*cit));
                if (pUnit == nullptr)
                    continue;

                UpdateSelectedUnit(pUnit, uiIndex);
                ++uiIndex;
            }
            for (uint ui(uiIndex); ui < MAX_SELECTED_UNITS; ++ui)
                Trigger(UITRIGGER_SELECTED_VISIBLE, false, ui);
        }

        return;
    }

    // Multi-info selection
    uint uiIndex(0);
    for (uiset_cit cit(setInfoSelection.begin()); cit != setInfoSelection.end(); ++cit)
    {
        IUnitEntity *pUnit(Game.GetUnitEntity(*cit));
        if (pUnit == nullptr)
            continue;

        UpdateSelectedUnit(pUnit, uiIndex);
        ++uiIndex;
    }

    for (uint ui(uiIndex); ui < MAX_SELECTED_UNITS; ++ui)
        Trigger(UITRIGGER_SELECTED_VISIBLE, false, ui);
}


/*====================
  CGameInterfaceManager::BuildingAttackAlert

  OMG, WE'RE UNDER ATTACK!
  ====================*/
void    CGameInterfaceManager::BuildingAttackAlert(const tstring &sName)
{
    m_uiLastBuildingAttackAlertTime = Game.GetGameTime();
    tsvector vParams;
    vParams.emplace_back(_CTS("true"));
    vParams.emplace_back(sName);
    Trigger(UITRIGGER_BUILDING_ATTACK_ALERT, vParams);
}


/*====================
  CGameInterfaceManager::UpdateSelectedUnit
  ====================*/
void    CGameInterfaceManager::UpdateSelectedUnit(IUnitEntity *pUnit, uint uiIndex)
{
    PROFILE("CGameInterfaceManager::UpdateSelectedUnit");

    CPlayer *pLocalPlayer(GameClient.GetLocalPlayer());
    if (pLocalPlayer == nullptr)
        return;

    CClientCommander *pCommander(GameClient.GetClientCommander());
    if (pCommander == nullptr)
        return;

    IHeroEntity *pHero(pUnit ? pUnit->GetAsHero() : nullptr);
    IGadgetEntity *pGadget(pUnit ? pUnit->GetAsGadget() : nullptr);
    CPlayer *pOwner(pUnit ? pUnit->GetOwnerPlayer() : nullptr);
    CTeamInfo *pTeam(pUnit ? Game.GetTeam(pUnit->GetTeam()) : nullptr);

    Trigger(UITRIGGER_SELECTED_VISIBLE, pUnit != nullptr, uiIndex);
    Trigger(UITRIGGER_SELECTED_INDEX, pUnit ? pUnit->GetIndex() : INVALID_INDEX, uiIndex);
    Trigger(UITRIGGER_SELECTED_NAME, pUnit ? pUnit->GetDisplayName() : TSNULL, uiIndex);
    Trigger(UITRIGGER_SELECTED_ICON, pUnit ? pUnit->GetIconPath() : TSNULL, uiIndex);
    Trigger(UITRIGGER_SELECTED_ILLUSION, pUnit ? pUnit->IsIllusion() : false, uiIndex);

    if (pUnit == nullptr)
        Trigger(UITRIGGER_SELECTED_TYPE, TSNULL, uiIndex);
    else if (pUnit->IsBuilding())
        Trigger(UITRIGGER_SELECTED_TYPE, _CTS("building"), uiIndex);
    else if (pUnit->IsHero())
        Trigger(UITRIGGER_SELECTED_TYPE, _CTS("hero"), uiIndex);
    else if (pUnit->IsCreep())
        Trigger(UITRIGGER_SELECTED_TYPE, _CTS("creep"), uiIndex);
    else if (pUnit->IsGadget())
        Trigger(UITRIGGER_SELECTED_TYPE, _CTS("gadget"), uiIndex);
    else if (pUnit->IsPet())
        Trigger(UITRIGGER_SELECTED_TYPE, _CTS("pet"), uiIndex);
    else
        Trigger(UITRIGGER_SELECTED_TYPE, _CTS("unit"), uiIndex);

    if (pUnit != nullptr)
    {
        CVec4f v4Color;

        if (pUnit->IsNeutral())
            v4Color = WHITE;
        else if (pLocalPlayer->IsEnemy(pUnit) && pUnit->GetTeam())
            v4Color = RED;
        else if (pUnit->GetOwnerClientNumber() == pLocalPlayer->GetClientNumber())
            v4Color = LIME;
        else
            v4Color = YELLOW;

        Trigger(UITRIGGER_SELECTED_COLOR, XtoA(v4Color), uiIndex);
    }
    else
    {
        Trigger(UITRIGGER_SELECTED_COLOR, XtoA(WHITE), uiIndex);
    }

    Trigger(UITRIGGER_SELECTED_ACTIVE, pUnit == pCommander->GetSelectedControlEntity(), uiIndex);

    static tsvector vHealth(3);
    vHealth[0] = XtoA(pUnit ? pUnit->GetHealth() : 0.0f);
    vHealth[1] = XtoA(pUnit ? pUnit->GetMaxHealth() : 0.0f);
    vHealth[2] = XtoA(pUnit ? (pUnit->HasUnitFlags(UNIT_FLAG_INVULNERABLE) ? -1.0f : pUnit->GetHealthPercent()) : 0.0f);
    Trigger(UITRIGGER_SELECTED_HEALTH, vHealth, uiIndex);

    static tsvector vMana(3);
    vMana[0] = XtoA(pUnit ? pUnit->GetMana() : 0.0f);
    vMana[1] = XtoA(pUnit ? pUnit->GetMaxMana() : 0.0f);
    vMana[2] = XtoA(pUnit ? pUnit->GetManaPercent() : 0.0f);
    Trigger(UITRIGGER_SELECTED_MANA, vMana, uiIndex);

    static tsvector vLevel(2);
    vLevel[0] = XtoA(pUnit ? pUnit->GetLevel() : 0);
    vLevel[1] = XtoA(pUnit ? (pUnit->IsPet() || pUnit->IsGadget() || pUnit->IsHero() || pUnit->IsTargetType(_CTS("Tower"), pUnit)) : false);
    Trigger(UITRIGGER_SELECTED_LEVEL, vLevel, uiIndex);

    static tsvector vPlayerInfo(2);
    vPlayerInfo[0] = pOwner ? pOwner->GetName() : pTeam ? pTeam->GetName() : TSNULL;
    vPlayerInfo[1] = XtoA(pOwner ? pOwner->GetColor() : pTeam ? pTeam->GetColor() : WHITE);
    Trigger(UITRIGGER_SELECTED_PLAYER_INFO, vPlayerInfo, uiIndex);

    if (uiIndex > 0)
        return;

    static tsvector vHealthRegen(2);
    vHealthRegen[0] = XtoA(pUnit ? pUnit->GetBaseHealthRegen() : 0.0f);
    vHealthRegen[1] = XtoA(pUnit ? pUnit->GetHealthRegen() : 0.0f);
    Trigger(UITRIGGER_SELECTED_HEALTHREGEN, vHealthRegen);

    static tsvector vManaRegen(2);
    vManaRegen[0] = XtoA(pUnit ? pUnit->GetBaseManaRegen() : 0.0f);
    vManaRegen[1] = XtoA(pUnit ? pUnit->GetManaRegen() : 0.0f);
    Trigger(UITRIGGER_SELECTED_MANAREGEN, vManaRegen);

    static tsvector vExperience(4);
    vExperience[0] = XtoA(pHero != nullptr);
    vExperience[1] = XtoA(pHero ? pHero->GetExperience() : 0.0f);
    vExperience[2] = XtoA(pHero ? pHero->GetExperienceForNextLevel() : 0.0f);
    vExperience[3] = XtoA(pHero ? pHero->GetPercentNextLevel() : 1.0f);
    Trigger(UITRIGGER_SELECTED_EXPERIENCE, vExperience);

    static tsvector vDamage(3);
    vDamage[0] = XtoA(pUnit ? pUnit->GetAttackDamageMin() : 0.0f);
    vDamage[1] = XtoA(pUnit ? pUnit->GetAttackDamageMax() : 0.0f);
    vDamage[2] = XtoA(pUnit ? ((pUnit->GetAttackDamageMin() + pUnit->GetAttackDamageMax()) * 0.5f * pUnit->GetBaseDamageMultiplier() + pUnit->GetBonusDamage()) * pUnit->GetTotalDamageMultiplier() - ((pUnit->GetAttackDamageMin() + pUnit->GetAttackDamageMax()) * 0.5f) : 0.0f);
    Trigger(UITRIGGER_SELECTED_DAMAGE, vDamage);

    static tsvector vArmor(3);
    vArmor[0] = XtoA(pUnit ? pUnit->GetBaseArmor() : 0.0f);
    vArmor[1] = XtoA(pUnit ? pUnit->GetArmor() : 0.0f);
    vArmor[2] = XtoA(pUnit ? Game.GetArmorDamageAdjustment(pUnit->GetArmorType(), pUnit->GetArmor()) : 0.0f);
    Trigger(UITRIGGER_SELECTED_ARMOR, vArmor);

    static tsvector vMagicArmor(3);
    vMagicArmor[0] = XtoA(pUnit ? pUnit->GetBaseMagicArmor() : 0.0f);
    vMagicArmor[1] = XtoA(pUnit ? pUnit->GetMagicArmor() : 0.0f);
    vMagicArmor[2] = XtoA(pUnit ? Game.GetArmorDamageAdjustment(pUnit->GetMagicArmorType(), pUnit->GetMagicArmor()) : 0.0f);
    Trigger(UITRIGGER_SELECTED_MAGIC_ARMOR, vMagicArmor);

    static tsvector vMoveSpeed(2);
    if (!pUnit || !pUnit->GetIsMobile())
    {
        vMoveSpeed[0] = XtoA(0.0f);
        vMoveSpeed[1] = XtoA(0.0f);
    }
    else
    {
        vMoveSpeed[0] = XtoA(pUnit->GetBaseMoveSpeed());
        vMoveSpeed[1] = XtoA(pUnit->GetMoveSpeed());
    }
    Trigger(UITRIGGER_SELECTED_MOVE_SPEED, vMoveSpeed);

    static tsvector vAttackSpeed(2);
    vAttackSpeed[0] = XtoA(pUnit ? pUnit->GetBaseAttackSpeed() * 100.0f : 0.0f);
    vAttackSpeed[1] = XtoA(pUnit ? pUnit->GetAttackSpeed() * 100.0f : 0.0f);
    Trigger(UITRIGGER_SELECTED_ATTACK_SPEED, vAttackSpeed);

    static tsvector vCastSpeed(2);
    vCastSpeed[0] = XtoA(pUnit ? pUnit->GetBaseCastSpeed() * 100.0f : 0.0f);
    vCastSpeed[1] = XtoA(pUnit ? pUnit->GetCastSpeed() * 100.0f : 0.0f);
    Trigger(UITRIGGER_SELECTED_CAST_SPEED, vCastSpeed);

    static tsvector vAttackRange(2);
    vAttackRange[0] = XtoA(pUnit ? pUnit->GetBaseAttackRange() : 0.0f);
    vAttackRange[1] = XtoA(pUnit ? pUnit->GetAttackRange() : 0.0f);
    Trigger(UITRIGGER_SELECTED_ATTACK_RANGE, vAttackRange);

    static tsvector vAttackCooldown(2);
    vAttackCooldown[0] = XtoA(pUnit ? pUnit->GetAttackCooldown() : 0.0f);
    vAttackCooldown[1] = XtoA(pUnit ? INT_CEIL(pUnit->GetAdjustedAttackCooldown() / float(Game.GetServerFrameLength())) * Game.GetServerFrameLength() : 0.0f);
    Trigger(UITRIGGER_SELECTED_ATTACK_COOLDOWN, vAttackCooldown);

    static tsvector vStrength(2);
    vStrength[0] = XtoA(pHero ? pHero->GetBaseStrength() : 0.0f);
    vStrength[1] = XtoA(pHero ? pHero->GetStrength() : 0.0f);
    Trigger(UITRIGGER_SELECTED_STRENGTH, vStrength);

    static tsvector vAgility(2);
    vAgility[0] = XtoA(pHero ? pHero->GetBaseAgility() : 0.0f);
    vAgility[1] = XtoA(pHero ? pHero->GetAgility() : 0.0f);
    Trigger(UITRIGGER_SELECTED_AGILITY, vAgility);

    static tsvector vIntelligence(3);
    vIntelligence[0] = XtoA(pHero ? pHero->GetBaseIntelligence() : 0.0f);
    vIntelligence[1] = XtoA(pHero ? pHero->GetIntelligence() : 0.0f);
    Trigger(UITRIGGER_SELECTED_INTELLIGENCE, vIntelligence);

    static tsvector vAttributes(3);
    vAttributes[0] = XtoA(pHero ? pHero->GetStrength() : 0.0f);
    vAttributes[1] = XtoA(pHero ? pHero->GetAgility() : 0.0f);
    vAttributes[2] = XtoA(pHero ? pHero->GetIntelligence() : 0.0f);
    Trigger(UITRIGGER_SELECTED_ATTRIBUTES, vAttributes);

    static tsvector vLifetime(3);
    vLifetime[0] = XtoA(pGadget ? pGadget->GetRemainingLifetime() : 0);
    vLifetime[1] = XtoA(pGadget ? pGadget->GetLifetime() : 0);
    vLifetime[2] = XtoA(pGadget ? pGadget->GetRemainingLifetimePercent() : 0.0f);
    Trigger(UITRIGGER_SELECTED_LIFETIME, vLifetime);

    Trigger(UITRIGGER_SELECTED_HAS_INVENTORY, pUnit ? pUnit->GetCanCarryItems() : false);
    Trigger(UITRIGGER_SELECTED_HAS_ATTRIBUTES, pHero != nullptr);

    static tsvector vAttackInfo(1);
    vAttackInfo[0] = pUnit ? Game.GetAttackTypeDisplayName(pUnit->GetAttackType()) : TSNULL;
    Trigger(UITRIGGER_SELECTED_ATTACK_INFO, vAttackInfo);

    static tsvector vDefenseInfo(1);
    vDefenseInfo[0] = pUnit ? Game.GetAttackTypeDisplayName(pUnit->GetAttackType()) : TSNULL;
    Trigger(UITRIGGER_SELECTED_DEFENSE_INFO, vDefenseInfo);

    static tsvector vAttributeInfo(5);
    vAttributeInfo[0] = pUnit ? pUnit->GetCombatType() : TSNULL;

    vAttributeInfo[1].clear();
    if (pHero != nullptr)
    {
        if (pHero->GetPrimaryAttribute() == ATTRIBUTE_STRENGTH)
            vAttributeInfo[1] = GameClient.GetGameMessage(_CTS("attribute_strength"));
        else if (pHero->GetPrimaryAttribute() == ATTRIBUTE_AGILITY)
            vAttributeInfo[1] = GameClient.GetGameMessage(_CTS("attribute_agility"));
        else if (pHero->GetPrimaryAttribute() == ATTRIBUTE_INTELLIGENCE)
            vAttributeInfo[1] = GameClient.GetGameMessage(_CTS("attribute_intelligence"));
    }

    vAttributeInfo[2] = XtoA(pHero ? pHero->GetStrengthPerLevel() : 0.0f);
    vAttributeInfo[3] = XtoA(pHero ? pHero->GetAgilityPerLevel() : 0.0f);
    vAttributeInfo[4] = XtoA(pHero ? pHero->GetIntelligencePerLevel() : 0.0f);

    Trigger(UITRIGGER_SELECTED_ATTRIBUTE_INFO, vAttributeInfo, uiIndex);
}


/*====================
  CGameInterfaceManager::UpdateReplayInfo
  ====================*/
void    CGameInterfaceManager::UpdateReplayInfo()
{
    PROFILE("CGameInterfaceManager::UpdateReplayInfo");

    static tsvector vGameInfo(11);
    const CXMLNode& cGameInfo(m_cReplayInfo.GetGameInfo());
    vGameInfo[0] = cGameInfo.GetProperty(_CTS("matchid"));
    vGameInfo[1] = K2_Version(cGameInfo.GetProperty(_CTS("version")));
    vGameInfo[2] = cGameInfo.GetProperty(_CTS("date"));

    uint uiMatchLength(cGameInfo.GetPropertyInt(_CTS("matchlength"), 0));
    uint uiHours(uiMatchLength / MS_PER_HR);
    uint uiMinutes((uiMatchLength % MS_PER_HR) / MS_PER_MIN);
    uint uiSeconds((uiMatchLength % MS_PER_MIN) / MS_PER_SEC);

    vGameInfo[3] = XtoA(uiHours);
    vGameInfo[4] = XtoA(uiMinutes);
    vGameInfo[5] = XtoA(uiSeconds);

    vGameInfo[6] = cGameInfo.GetProperty(_CTS("mapfancyname"));
    vGameInfo[7] = cGameInfo.GetProperty(_CTS("gamemode"));
    vGameInfo[8] = cGameInfo.GetProperty(_CTS("gameoptions"));
    vGameInfo[9] = cGameInfo.GetProperty(_CTS("winner"));
    vGameInfo[10] = XtoA(FileManager.IsCompatVersionSupported(cGameInfo.GetProperty(_CTS("version"))));
    Trigger(UITRIGGER_REPLAY_INFO_GAME, vGameInfo);

    // Player info
    static tsvector vPlayerInfo(5);

    // Teams
    uint uiPlayerIndex(0);
    for (uint uiTeam(1); uiTeam <= 2; ++uiTeam)
    {
        // Player info
        for (uint uiTeamIndex(0); uiTeamIndex < MAX_DISPLAY_PLAYERSPERTEAM; ++uiTeamIndex)
        {
            const CXMLNode& cPlayerProperties(m_cReplayInfo.GetPlayerInfo(uiPlayerIndex));

            vPlayerInfo[0] = cPlayerProperties.GetProperty(_CTS("name"));           // Name
            vPlayerInfo[1] = XtoA(CPlayer::GetColor(uiPlayerIndex));                                    // Color
            vPlayerInfo[2] = CPlayer::GetColorName(uiPlayerIndex);                                      // Color
            vPlayerInfo[3] = cPlayerProperties.GetProperty(_CTS("heroname"));       // Hero Name
            vPlayerInfo[4] = cPlayerProperties.GetProperty(_CTS("heroicon"));       // Hero Icon
            Trigger(UITRIGGER_REPLAY_INFO_PLAYER, vPlayerInfo, uiPlayerIndex);
            ++uiPlayerIndex;
        }
    }
}


/*====================
  CGameInterfaceManager::UpdateReplay
  ====================*/
void    CGameInterfaceManager::UpdateReplay()
{
    PROFILE("CGameInterfaceManager::UpdateReplay");

    Trigger(UITRIGGER_REPLAY_TIME, GameClient.GetGameTime() - ReplayManager.GetBeginTime());
    Trigger(UITRIGGER_REPLAY_ENDTIME, ReplayManager.GetEndTime() - ReplayManager.GetBeginTime());
    Trigger(UITRIGGER_REPLAY_FRAME, ReplayManager.GetFrame());
    Trigger(UITRIGGER_REPLAY_ENDFRAME, ReplayManager.GetEndFrame());
    Trigger(UITRIGGER_REPLAY_SPEED, ReplayManager.GetPlaybackSpeed());
    Trigger(UITRIGGER_REPLAY_PLAYING, !ReplayManager.IsPaused());
    Trigger(UITRIGGER_REPLAY_PAUSED, ReplayManager.IsPaused());

    CPlayer *pClient(GameClient.GetPlayer(GameClient.GetLocalClientNum()));
    if (pClient == nullptr)
        return;

    Trigger(UITRIGGER_REPLAY_NAME, pClient->GetName());
}


/*====================
  CGameInterfaceManager::UpdateItemCursor
  ====================*/
void    CGameInterfaceManager::UpdateItemCursor()
{
    PROFILE("CGameInterfaceManager::UpdateItemCursor");

    IGameEntity *pEntity(GameClient.GetEntity(GameClient.GetItemCursorIndex()));
    if (!pEntity || !pEntity->IsItem())
    {
        Trigger(UITRIGGER_ITEM_CURSOR_VISIBLE, false);
        return;
    }

    // if the item cursor is over an ally, hide it.
    IUnitEntity *pTarget(Game.GetUnitEntity(GameClient.GetClientCommander()->GetHoverEntity()));
    if (pTarget != nullptr && pTarget->IsTargetType(TARGET_TRAIT_ALLY, GameClient.GetClientCommander()->GetSelectedControlEntity()))
    {
        Trigger(UITRIGGER_ITEM_CURSOR_VISIBLE, false);
        return;
    }


    IEntityItem *pItem(pEntity->GetAsItem());

    Trigger(UITRIGGER_ITEM_CURSOR_VISIBLE, true);
    Trigger(UITRIGGER_ITEM_CURSOR_ICON, pItem->GetIconPath());

    CVec2f v2CursorPos(Input.GetCursorPos());
    static tsvector vParams(2);
    vParams[0] = XtoA(v2CursorPos.x);
    vParams[1] = XtoA(v2CursorPos.y);

    Trigger(UITRIGGER_ITEM_CURSOR_POSITION, vParams);

    Input.SetCursorHidden(CURSOR_GAME, BOOL_TRUE);
}


/*====================
  CGameInterfaceManager::UpdateShopItemTooltip
  ====================*/
void    CGameInterfaceManager::UpdateShopItemTooltip(const tstring &sItem, IUnitEntity *pControlUnit, uint uiTrigger, uint uiSlot, bool bPurchaseRecipe, bool bOwned, bool bCarry, bool bAccess, bool bLocal)
{
    CPlayer *pLocalPlayer(GameClient.GetLocalPlayer());

    static tsvector vTooltip(48);

    CItemDefinition *pDefinition(EntityRegistry.GetDefinition<CItemDefinition>(sItem));

    if (pDefinition == nullptr)
    {
        for (uint ui(0); ui < vTooltip.size(); ++ui)
            vTooltip[ui].clear();

        Trigger(uiTrigger, vTooltip, uiSlot);
        return;
    }

    uivector vModifierKeys;

    // Find and activate all exclusive and conditional modifiers
    uint uiModifier(0);

    // Search this entity
    const EntityModifierMap &mapModifiers(pDefinition->GetModifiers());
    for (EntityModifierMap::const_iterator cit(mapModifiers.begin()), citEnd(mapModifiers.end()); cit != citEnd; ++cit)
    {
        IEntityDefinition *pModifier(cit->second);

        if (!pModifier->GetExclusive() && pModifier->GetCondition().empty())
            continue;

        uiModifier |= cit->first;
    }

    IEntityDefinition *pModifiedDefinition(pDefinition->GetModifiedDefinition(uiModifier));
    if (pModifiedDefinition != nullptr)
        pDefinition = static_cast<CItemDefinition *>(pModifiedDefinition);

    if (pDefinition == nullptr)
    {
        Trigger(uiTrigger, TSNULL, uiSlot);
        return;
    }

    uint uiLevel(0);
    uint uiMaxLevel(pDefinition->GetMaxLevel());
    uint uiIndex(MIN(1u, uiLevel) - 1);

    if (pDefinition->IsRecipe() && Input.IsButtonDown(BUTTON_CTRL))
        bPurchaseRecipe = true;

    vTooltip[0] = bPurchaseRecipe ? pDefinition->GetDisplayName() + _CTS(" Recipe") : pDefinition->GetDisplayName();

    if (bPurchaseRecipe)
    {
        if (pDefinition->GetAutoAssemble())
            vTooltip[1] = GameClient.GetGameMessage(_CTS("auto_assemble"));
        else
            vTooltip[1] = XtoA(pDefinition->GetCost());
    }
    else
        vTooltip[1] = XtoA(pDefinition->GetTotalCost());

    vTooltip[2] = XtoA(uiLevel);
    vTooltip[3] = XtoA(pDefinition->GetMaxLevel());

    BuildMultiLevelText(cg_tooltipFlavor ? pDefinition->GetDescription() : TSNULL, uiLevel, MAX(1u, uiMaxLevel) - 1, vTooltip[4]);
    BuildMultiLevelText(pDefinition->GetDescription2(), uiLevel, MAX(1u, uiMaxLevel) - 1, vTooltip[34]);

    // Build passive bonus list
    int iLines(0);
    BuildMultiLevelBonusesString(pDefinition, uiLevel, pDefinition->GetMaxLevel(), vModifierKeys, vTooltip[5], iLines);
    vTooltip[6] = XtoA(iLines);

    vTooltip[7] = XtoA(pDefinition->GetActionType() != TOOL_ACTION_PASSIVE);

    if (pDefinition->GetActionType() == TOOL_ACTION_TOGGLE)
        vTooltip[8] = GameClient.GetGameMessage(_CTS("action_toggle"));
    else if (pDefinition->GetActionType() == TOOL_ACTION_NO_TARGET)
        vTooltip[8] = GameClient.GetGameMessage(_CTS("action_no_target"));
    else if (pDefinition->GetActionType() == TOOL_ACTION_TARGET_POSITION)
        vTooltip[8] = GameClient.GetGameMessage(_CTS("action_target_position"));
    else if (pDefinition->GetActionType() == TOOL_ACTION_TARGET_ENTITY)
        vTooltip[8] = GameClient.GetGameMessage(_CTS("action_target_unit"));
    else if (pDefinition->GetActionType() == TOOL_ACTION_GLOBAL)
        vTooltip[8] = GameClient.GetGameMessage(_CTS("action_target_global"));
    else if (pDefinition->GetActionType() == TOOL_ACTION_TARGET_SELF)
        vTooltip[8] = GameClient.GetGameMessage(_CTS("action_target_self"));
    else if (pDefinition->GetActionType() == TOOL_ACTION_FACING)
        vTooltip[8] = GameClient.GetGameMessage(_CTS("action_facing"));
    else if (pDefinition->GetActionType() == TOOL_ACTION_SELF_POSITION)
        vTooltip[8] = GameClient.GetGameMessage(_CTS("action_self_position"));
    else if (pDefinition->GetActionType() == TOOL_ACTION_ATTACK)
        vTooltip[8] = GameClient.GetGameMessage(_CTS("action_attack"));
    else if (pDefinition->GetActionType() == TOOL_ACTION_ATTACK_TOGGLE)
        vTooltip[8] = GameClient.GetGameMessage(_CTS("action_attack_toggle"));
    else if (pDefinition->GetActionType() == TOOL_ACTION_TARGET_DUAL)
        vTooltip[8] = GameClient.GetGameMessage(_CTS("action_target_dual"));
    else if (pDefinition->GetActionType() == TOOL_ACTION_TARGET_DUAL_POSITION)
        vTooltip[8] = GameClient.GetGameMessage(_CTS("action_target_dual_position"));
    else if (pDefinition->GetActionType() == TOOL_ACTION_TARGET_VECTOR)
        vTooltip[8] = GameClient.GetGameMessage(_CTS("action_target_vector"));
    else if (pDefinition->GetActionType() == TOOL_ACTION_TARGET_CURSOR)
        vTooltip[8] = GameClient.GetGameMessage(_CTS("action_target_cursor"));

    vTooltip[9] = Game.GetTargetSchemeDisplayName(pDefinition->GetTargetScheme(uiIndex));
    vTooltip[10] = Game.GetEffectTypeString(pDefinition->GetCastEffectType(uiIndex));

    BUILD_FLOAT_PROPERTY(pDefinition, sRange, Range, 0, 0);
    vTooltip[11] = sRange;

    BUILD_FLOAT_PROPERTY(pDefinition, sTargetRadius, TargetRadius, 0, 0);
    vTooltip[12] = sTargetRadius;

    BUILD_FLOAT_PROPERTY(pDefinition, sManaCost, ManaCost, 0, 2);
    vTooltip[13] = sManaCost;

    tstring sCooldownTime;
    {
        bool bConstant(true);
        uint uiCooldownTime(pDefinition->GetCooldownTime(0));

        for (uint uiIndex(1); uiIndex < uiMaxLevel; ++uiIndex)
        {
            if (pDefinition->GetCooldownTime(uiIndex) != uiCooldownTime)
                bConstant = false;
        }

        if (bConstant)
        {
            if (uiCooldownTime != 0)
                sCooldownTime = XtoA(MsToSec(uiCooldownTime), 0, 0, 0, 3);
        }
        else
        {
            if (uiLevel >= 1)
                sCooldownTime = _CTS("^v") + XtoA(MsToSec(uiCooldownTime), 0, 0, 0, 3) + _CTS("^*");
            else
                sCooldownTime = XtoA(MsToSec(uiCooldownTime), 0, 0, 0, 3);

            for (uint uiIndex(1); uiIndex < uiMaxLevel; ++uiIndex)
            {
                sCooldownTime += _T('/');

                if (uiLevel >= uiIndex + 1)
                    sCooldownTime += _CTS("^v") + XtoA(MsToSec(pDefinition->GetCooldownTime(uiIndex)), 0, 0, 0, 3) + _CTS("^*");
                else
                    sCooldownTime += XtoA(MsToSec(pDefinition->GetCooldownTime(uiIndex)), 0, 0, 0, 3);
            }
        }
    }
    vTooltip[14] = sCooldownTime;

    BuildMultiLevelText(pDefinition->GetEffectDescription(ACTION_SCRIPT_IMPACT), uiLevel, MAX(1u, uiMaxLevel) - 1, vTooltip[15]);

    uint uiTriggerIndex(0);
    uint uiDisplayIndex(0);

    const uint NUM_DISPLAY_TRIGGERS(1);

    while (s_cTriggers[uiTriggerIndex].eScript != NUM_ACTION_SCRIPTS && uiDisplayIndex < NUM_DISPLAY_TRIGGERS)
    {
        const tstring &sDescription(pDefinition->GetEffectDescription(s_cTriggers[uiTriggerIndex].eScript));
        if (sDescription.empty())
        {
            ++uiTriggerIndex;
            continue;
        }

        vTooltip[16].clear();
        vTooltip[17].clear();
        vTooltip[18].clear();

        vTooltip[16] = _CTS("true");
        vTooltip[17] = GameClient.GetGameMessage(s_cTriggers[uiTriggerIndex].sLabel);
        BuildMultiLevelText(sDescription, uiLevel, MAX(1u, uiMaxLevel) - 1, vTooltip[18]);

        ++uiDisplayIndex;
        ++uiTriggerIndex;
    }

    for (; uiDisplayIndex < NUM_DISPLAY_TRIGGERS; ++uiDisplayIndex)
    {
        vTooltip[16].clear();
        vTooltip[17].clear();
        vTooltip[18].clear();
    }

    // Status effect
    {
        vTooltip[19].clear();
        vTooltip[20].clear();
        vTooltip[21].clear();

        CStateDefinition *pStateDef(EntityRegistry.GetDefinition<CStateDefinition>(pDefinition->GetStatusEffectTooltip(uiIndex)));
        if (pStateDef != nullptr)
        {
            vTooltip[19] = pDefinition->GetStatusEffectHeader();

            int iLines(0);
            BuildMultiLevelBonusesString(pStateDef, uiLevel, uiMaxLevel, vModifierKeys, vTooltip[20], iLines);
            vTooltip[21] = XtoA(iLines);
        }
    }

    // Status effect
    {
        vTooltip[31].clear();
        vTooltip[32].clear();
        vTooltip[33].clear();

        CStateDefinition *pStateDef(EntityRegistry.GetDefinition<CStateDefinition>(pDefinition->GetStatusEffectTooltip2(uiIndex)));
        if (pStateDef != nullptr)
        {
            vTooltip[31] = pDefinition->GetStatusEffectHeader2();

            int iLines(0);
            BuildMultiLevelBonusesString(pStateDef, uiLevel, uiMaxLevel, vModifierKeys, vTooltip[32], iLines);
            vTooltip[33] = XtoA(iLines);
        }
    }

    vTooltip[22].clear();
    vTooltip[23].clear();
    vTooltip[24].clear();
    vTooltip[25].clear();
    vTooltip[26].clear();
    vTooltip[27].clear();
    vTooltip[28].clear();

    const AuraList &cAuraList(pDefinition->GetAuraList());

    if (!cAuraList.empty())
    {
        CStateDefinition *pAuraStateDef(EntityRegistry.GetDefinition<CStateDefinition>(cAuraList.front().GetStateName(uiLevel)));

        if (pAuraStateDef != nullptr)
        {
            vTooltip[22] = _CTS("true");

            int iLines(0);
            BuildMultiLevelBonusesString(pAuraStateDef, uiLevel, uiMaxLevel, vModifierKeys, vTooltip[23], iLines);
            vTooltip[24] = XtoA(iLines);

            vTooltip[25] = _CTS("(Aura Stack Type)");
            vTooltip[26] = Game.GetTargetSchemeDisplayName(cAuraList.front().GetTargetScheme(uiLevel));
            vTooltip[27] = Game.GetEffectTypeString(cAuraList.front().GetEffectType(uiLevel));
            vTooltip[28] = cAuraList.front().GetRadius(uiLevel) >= 9999.0f ? GameClient.GetGameMessage(_CTS("aura_range_global")) : XtoA(INT_ROUND(cAuraList.front().GetRadius(uiLevel)));
        }
    }

    vTooltip[29] = cg_tooltipFlavor ? pDefinition->GetTooltipFlavorText() : TSNULL;
    vTooltip[30].clear();

    if (pDefinition->GetIsChanneling())
    {
        tstring sChannelTime;
        {
            bool bConstant(true);
            uint uiChannelTime(pDefinition->GetChannelTime(0));

            for (uint uiIndex(1); uiIndex < uiMaxLevel; ++uiIndex)
            {
                if (pDefinition->GetChannelTime(uiIndex) != uiChannelTime)
                    bConstant = false;
            }

            if (bConstant)
            {
                if (uiChannelTime != 0)
                    sChannelTime = XtoA(MsToSec(uiChannelTime), 0, 0, 0, 3);
            }
            else
            {
                if (uiLevel >= 1)
                    sChannelTime = _CTS("^v") + XtoA(MsToSec(uiChannelTime), 0, 0, 0, 3) + _CTS("^*");
                else
                    sChannelTime = XtoA(MsToSec(uiChannelTime), 0, 0, 0, 3);

                for (uint uiIndex(1); uiIndex < uiMaxLevel; ++uiIndex)
                {
                    sChannelTime += _T('/');

                    if (uiLevel >= uiIndex + 1)
                        sChannelTime += _CTS("^v") + XtoA(MsToSec(pDefinition->GetChannelTime(uiIndex)), 0, 0, 0, 3) + _CTS("^*");
                    else
                        sChannelTime += XtoA(MsToSec(pDefinition->GetChannelTime(uiIndex)), 0, 0, 0, 3);
                }
            }
        }

        vTooltip[35] = sChannelTime;

    }
    else
        vTooltip[35].clear();

    BUILD_FLOAT_PROPERTY(pDefinition, sActiveManaCost, ActiveManaCost, 0, 2);
    vTooltip[36] = sActiveManaCost;

    if (pDefinition->IsRecipe() && !bPurchaseRecipe && !Input.IsButtonDown(BUTTON_CTRL))
        vTooltip[37] = GameClient.GetGameMessage(_CTS("shop_action_view_recipe"));
    else if (!pDefinition->IsRecipe() && !bPurchaseRecipe && Input.IsButtonDown(BUTTON_SHIFT))
        vTooltip[37] = GameClient.GetGameMessage(_CTS("shop_action_view_item"));
    else if (pDefinition->GetAutoAssemble())
        vTooltip[37] = GameClient.GetGameMessage(_CTS("shop_action_auto_assemble"));
    else if (bOwned)
        vTooltip[37] = GameClient.GetGameMessage(_CTS("shop_action_already_purchased"));
    else if (!bCarry)
        vTooltip[37] = GameClient.GetGameMessage(_CTS("shop_action_cannot_shop"));
    else if (bAccess && !bPurchaseRecipe && pLocalPlayer->GetGold() < pDefinition->GetCost())
        vTooltip[37] = GameClient.GetGameMessage(_CTS("shop_action_cannot_afford_item"));
    else if (bAccess && bPurchaseRecipe && pLocalPlayer->GetGold() < pDefinition->GetCost())
        vTooltip[37] = GameClient.GetGameMessage(_CTS("shop_action_cannot_afford_recipe"));
    else if (bAccess && !bPurchaseRecipe && bLocal)
        vTooltip[37] = GameClient.GetGameMessage(_CTS("shop_action_purchase_item"));
    else if (bAccess && !bPurchaseRecipe && !bLocal)
        vTooltip[37] = GameClient.GetGameMessage(_CTS("shop_action_purchase_item_stash"));
    else if (bAccess && bPurchaseRecipe)
        vTooltip[37] = GameClient.GetGameMessage(_CTS("shop_action_purchase_recipe"));
    else
        vTooltip[37] = GameClient.GetGameMessage(_CTS("shop_action_unavailable"));

    BUILD_FLOAT_PROPERTY(pDefinition, sTriggeredManaCost, TriggeredManaCost, 0, 2);
    vTooltip[38] = sTriggeredManaCost;

    if (pDefinition->IsRecipe())
    {
        const tsvector &vComponentList(pDefinition->GetComponents(0));
        vTooltip[39] = XtoA(INT_SIZE(vComponentList.size()) + (pDefinition->GetAutoAssemble() ? 0 : 1));

        int iSlot(0);

        static uivector vComponents;
        vComponents.clear();

        for (tsvector_cit it(vComponentList.begin()); it != vComponentList.end() && iSlot < 4; ++it)
        {
            CItemDefinition *pItemDefinition(EntityRegistry.GetDefinition<CItemDefinition>(*it));
            if (pItemDefinition == nullptr)
                continue;

            ushort unTypeID(pItemDefinition->GetTypeID());
            if (unTypeID == INVALID_ENT_TYPE)
                continue;

            bool bFound(false);
            for (uint uiItemSlot(INVENTORY_START_BACKPACK); uiItemSlot <= INVENTORY_END_BACKPACK; ++uiItemSlot)
            {
                if (pControlUnit == nullptr)
                    continue;

                IEntityItem *pItem(pControlUnit->GetItem(uiItemSlot));
                if (pItem == nullptr)
                    continue;
                if (pItem->GetType() != unTypeID)
                    continue;
                if (!pItem->HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED))
                    continue;

                bool bUsed(false);
                for (uivector_it it(vComponents.begin()); it != vComponents.end(); ++it)
                {
                    if (*it == uiItemSlot)
                    {
                        bUsed = true;
                        break;
                    }
                }
                if (bUsed)
                    continue;

                vComponents.emplace_back(uiItemSlot);
                bFound = true;
                break;
            }

            vTooltip[40 + iSlot * 2 + 0] = (bFound ? _T("^g") : _T("^v")) + pItemDefinition->GetDisplayName();
            vTooltip[40 + iSlot * 2 + 1] = XtoA(pItemDefinition->GetTotalCost(0));
            ++iSlot;
        }

        if (!pDefinition->GetAutoAssemble() && iSlot < 4)
        {
            ushort unTypeID(pDefinition->GetTypeID());

            bool bFound(false);
            for (uint uiItemSlot(INVENTORY_START_BACKPACK); uiItemSlot <= INVENTORY_END_BACKPACK; ++uiItemSlot)
            {
                if (pControlUnit == nullptr)
                    continue;

                IEntityItem *pItem(pControlUnit->GetItem(uiItemSlot));
                if (pItem == nullptr)
                    continue;
                if (pItem->GetType() != unTypeID)
                    continue;
                if (pItem->HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED))
                    continue;

                bFound = true;
                break;
            }

            vTooltip[40 + iSlot * 2 + 0] = (bFound ? _T("^g") : _T("^v")) + pDefinition->GetDisplayName() + _CTS(" Recipe");
            vTooltip[40 + iSlot * 2 + 1] = XtoA(pDefinition->GetCost());
            ++iSlot;
        }

        for (; iSlot < 4; ++iSlot)
        {
            vTooltip[40 + iSlot * 2 + 0].clear();
            vTooltip[40 + iSlot * 2 + 1].clear();
        }
    }
    else
    {
        vTooltip[39] = _CTS("0");

        for (int iSlot(0); iSlot < 4; ++iSlot)
        {
            vTooltip[40 + iSlot * 2 + 0].clear();
            vTooltip[40 + iSlot * 2 + 1].clear();
        }
    }

    Trigger(uiTrigger, vTooltip, uiSlot);
}


/*====================
  CGameInterfaceManager::UpdateShopItem
  ====================*/
void    CGameInterfaceManager::UpdateShopItem(const tstring &sItem, bool bAccess, bool bLocal, uint uiStockRemaining, uint uiCooldownTime, bool bPurchaseRecipe, bool bOwned, bool bComponent, bool bCarry, bool bUsedIn, ushort unShop, int iSlot, tsvector &vItem)
{
    CPlayer *pLocalPlayer(GameClient.GetLocalPlayer());

    if (pLocalPlayer == nullptr || sItem.empty())
    {
        vItem[0] = _CTS("false");
        return;
    }

    CItemDefinition *pDef(EntityRegistry.GetDefinition<CItemDefinition>(sItem));
    if (pDef == nullptr)
    {
        vItem[0] = _CTS("false");
        return;
    }

    vItem[0] = _CTS("true");
    vItem[1] = pDef->GetDisplayName();
    vItem[2] = cg_tooltipFlavor ? pDef->GetDescription() : TSNULL;
    vItem[3] = pDef->GetIconPath(0);
    vItem[4] = (!Input.IsButtonDown(BUTTON_CTRL) && !bPurchaseRecipe) ? XtoA(pDef->GetTotalCost()) : (pDef->GetAutoAssemble() ? GameClient.GetGameMessage(_CTS("auto_assemble")) : XtoA(pDef->GetCost()));
    vItem[5] = XtoA(pDef->GetTotalCost());
    vItem[6] = XtoA(bAccess || pDef->GetAutoAssemble() || bOwned);

    if (pDef->IsRecipe() && !bPurchaseRecipe && !Input.IsButtonDown(BUTTON_CTRL))
        vItem[7] = GameClient.GetGameMessage(_CTS("shop_action_view_recipe"));
    else if (!pDef->IsRecipe() && !bPurchaseRecipe && Input.IsButtonDown(BUTTON_SHIFT))
        vItem[7] = GameClient.GetGameMessage(_CTS("shop_action_view_item"));
    else if (pDef->GetAutoAssemble())
        vItem[7] = GameClient.GetGameMessage(_CTS("shop_action_auto_assemble"));
    else if (bOwned)
        vItem[7] = GameClient.GetGameMessage(_CTS("shop_action_already_purchased"));
    else if (!bCarry)
        vItem[7] = GameClient.GetGameMessage(_CTS("shop_action_cannot_shop"));
    else if (bAccess && !bPurchaseRecipe && pLocalPlayer->GetGold() < pDef->GetCost())
        vItem[7] = GameClient.GetGameMessage(_CTS("shop_action_cannot_afford_item"));
    else if (bAccess && bPurchaseRecipe && pLocalPlayer->GetGold() < pDef->GetCost())
        vItem[7] = GameClient.GetGameMessage(_CTS("shop_action_cannot_afford_recipe"));
    else if (bAccess && !bPurchaseRecipe)
        vItem[7] = GameClient.GetGameMessage(_CTS("shop_action_purchase_item"));
    else if (bAccess && bPurchaseRecipe)
        vItem[7] = GameClient.GetGameMessage(_CTS("shop_action_purchase_recipe"));
    else
        vItem[7] = GameClient.GetGameMessage(_CTS("shop_action_unavailable"));

    vItem[8] = XtoA(pDef->IsRecipe());

    if (unShop == INVALID_ENT_TYPE)
        unShop = GameClient.GetShop(sItem);

    CShopDefinition *pShop(EntityRegistry.GetDefinition<CShopDefinition>(unShop));
    if (pShop != nullptr)
    {
        vItem[9] = pShop->GetIconPath();
        vItem[10] = pShop->GetDisplayName();
    }
    else
    {
        vItem[9] = _CTS("$invis");
        vItem[10] = _CTS("???");
    }

    vItem[11] = XtoA(bOwned);
    vItem[12] = XtoA(bComponent);

    if (iSlot != -1)
    {
        IBaseInput *pAction(ActionRegistry.GetAction(_CTS("Shop")));
        tstring sParam(XtoA(iSlot));

        bool bFound(false);

        const ButtonActionMap &lButton(ActionRegistry.GetButtonActionMap(BINDTABLE_GAME_SHOP));
        for (ButtonActionMap::const_iterator it(lButton.begin()); it != lButton.end() && !bFound; ++it)
        {
            if (it->first == BUTTON_INVALID)
                continue;

            for (BindModMap::const_iterator itBind(it->second.begin()); itBind != it->second.end(); ++itBind)
            {
                if (itBind->second.GetAction() == pAction && itBind->second.GetParam() == sParam)
                {
                    vItem[13] = Input.GetBindString(it->first, itBind->first);
                    bFound = true;
                    break;
                }
            }
        }
    }
    else
    {
        vItem[13].clear();
    }

    if (pDef->IsRecipe())
    {
        const tsvector &vComponents(pDef->GetComponents(0));
        vItem[14] = XtoA(INT_SIZE(vComponents.size()));
    }

    vItem[15] = XtoA(bAccess && !bLocal);
    vItem[16] = XtoA(bUsedIn);
    vItem[17] = XtoA(pDef->GetAutoAssemble());
    vItem[18] = XtoA((pDef->IsRecipe() && !bPurchaseRecipe && !Input.IsButtonDown(BUTTON_CTRL)) || (!pDef->IsRecipe() && !bPurchaseRecipe && Input.IsButtonDown(BUTTON_SHIFT)));
    vItem[19] = (uiStockRemaining != -1 ? XtoA(uiStockRemaining) : TSNULL);
    vItem[20] = XtoA(pDef->GetRestockDelay() > 0 ? (float(uiCooldownTime) / float(pDef->GetRestockDelay())) : 0.0f);
    vItem[21] = XtoA(uiCooldownTime);
    vItem[22] = XtoA(pDef->GetInitialCharges());
    vItem[23] = XtoA(pDef->GetNew());
    
    IHeroEntity *pHero(pLocalPlayer->GetHero());

    if (pHero != nullptr)
    {
        CHeroDefinition *pHeroDef(EntityRegistry.GetDefinition<CHeroDefinition>(pHero->GetType()));
        vItem[24] = XtoA(pHeroDef->HasGoodItem(pDef->GetName()));
    }

    vItem[25] = XtoA(bAccess);
}


/*====================
  CGameInterfaceManager::UpdateShopRecipe
  ====================*/
void    CGameInterfaceManager::UpdateShopRecipe(const tstring &sRecipe, IUnitEntity *pControlUnit, tsvector &vItem, uint uiSubIndex)
{
    PROFILE("CGameInterfaceManager::UpdateShopRecipe");

    CItemDefinition *pDef(EntityRegistry.GetDefinition<CItemDefinition>(sRecipe));
    if (pDef == nullptr)
    {
        vItem[0] = _CTS("false");

        for (uint uiSlot(0); uiSlot < MAX_RECIPE_COMPONENTS; ++uiSlot)
            Trigger(UITRIGGER_RECIPE_COMPONENT, vItem, uiSlot);
        for (uint uiSlot(0); uiSlot < MAX_RECIPE_USEDIN; ++uiSlot)
            Trigger(UITRIGGER_RECIPE_USEDIN, vItem, uiSlot);

        return;
    }

    static uivector vComponents;
    vComponents.clear();

    uint uiComponentList(0);
    const tsvector &vComponentList(pDef->GetComponents(uiComponentList));

    uint uiComponentTrigger((uiSubIndex == INVALID_INDEX) ? UITRIGGER_RECIPE_COMPONENT : (UITRIGGER_RECIPE_COMPONENT_0_SUB_COMPONENT + MIN(uiSubIndex, MAX_RECIPE_COMPONENTS)));
    uint uiTooltipTrigger((uiSubIndex == INVALID_INDEX) ? UITRIGGER_RECIPE_COMPONENT_TOOLTIP : (UITRIGGER_RECIPE_COMPONENT_0_SUB_TOOLTIP + MIN(uiSubIndex, MAX_RECIPE_COMPONENTS)));

    uint uiSlot(0);
    int iKeySlot(0);
    for (; uiSlot < vComponentList.size() && uiSlot < MAX_RECIPE_COMPONENTS; ++uiSlot)
    {
        const tstring &sItem(vComponentList[uiSlot]);

        ushort unTypeID(EntityRegistry.LookupID(sItem));
        if (unTypeID == INVALID_ENT_TYPE)
            continue;

        bool bFound(false);
        for (uint uiItemSlot(INVENTORY_START_BACKPACK); uiItemSlot <= INVENTORY_END_BACKPACK; ++uiItemSlot)
        {
            if (pControlUnit == nullptr)
                continue;

            IEntityItem *pItem(pControlUnit->GetItem(uiItemSlot));
            if (pItem == nullptr)
                continue;
            if (pItem->GetType() != unTypeID)
                continue;
            if (!pItem->HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED))
                continue;

            bool bUsed(false);
            for (uivector_it it(vComponents.begin()); it != vComponents.end(); ++it)
            {
                if (*it == uiItemSlot)
                {
                    bUsed = true;
                    break;
                }
            }
            if (bUsed)
                continue;

            vComponents.emplace_back(uiItemSlot);
            bFound = true;
            break;
        }

        UpdateShopItem(
            sItem,
            GameClient.CanAccessItem(sItem),
            GameClient.CanAccessItemLocal(sItem),
            GameClient.GetItemStock(sItem),
            GameClient.GetItemRestockTime(sItem),
            false,
            bFound,
            true,
            pControlUnit != nullptr ? pControlUnit->GetCanCarryItems() : true,
            false,
            INVALID_ENT_TYPE,
            iKeySlot++,
            vItem
        );

        Trigger(uiComponentTrigger, vItem, uiSlot);

        UpdateShopItemTooltip(
            sItem,
            pControlUnit,
            uiTooltipTrigger,
            uiSlot,
            false,
            bFound,
            pControlUnit != nullptr ? pControlUnit->GetCanCarryItems() : true,
            GameClient.CanAccessItem(sItem),
            GameClient.CanAccessItemLocal(sItem)
        );

        //if (uiSubIndex == INVALID_INDEX)
        //  UpdateShopRecipe(sItem, pControlUnit, vItem, uiSlot);
    }

    // List recipe for this item also
    if (pDef->IsRecipe() && !pDef->GetAutoAssemble())
    {
        if (uiSlot < MAX_RECIPE_COMPONENTS)
        {
            const tstring &sItem(sRecipe);

            ushort unTypeID(EntityRegistry.LookupID(sItem));
            if (unTypeID != INVALID_ENT_TYPE)
            {
                bool bFound(false);
                for (uint uiItemSlot(INVENTORY_START_BACKPACK); uiItemSlot <= INVENTORY_END_BACKPACK; ++uiItemSlot)
                {
                    if (pControlUnit == nullptr)
                        continue;

                    IEntityItem *pItem(pControlUnit->GetItem(uiItemSlot));
                    if (pItem == nullptr)
                        continue;
                    if (pItem->GetType() != unTypeID)
                        continue;
                    if (pItem->HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED))
                        continue;

                    vComponents.emplace_back(uiItemSlot);
                    bFound = true;
                    break;
                }

                UpdateShopItem(
                    sItem,
                    GameClient.CanAccessItem(sItem),
                    GameClient.CanAccessItemLocal(sItem),
                    GameClient.GetItemStock(sItem),
                    GameClient.GetItemRestockTime(sItem),
                    true,
                    bFound,
                    false,
                    pControlUnit != nullptr ? pControlUnit->GetCanCarryItems() : true,
                    false,
                    INVALID_ENT_TYPE,
                    iKeySlot++,
                    vItem
                );
                Trigger(uiComponentTrigger, vItem, uiSlot);

                UpdateShopItemTooltip(sItem,
                    pControlUnit,
                    uiTooltipTrigger,
                    uiSlot,
                    true,
                    bFound,
                    pControlUnit != nullptr ? pControlUnit->GetCanCarryItems() : true,
                    GameClient.CanAccessItem(sItem),
                    GameClient.CanAccessItemLocal(sItem)
                );
                ++uiSlot;
            }
        }
    }
    else if (!pDef->IsRecipe() && cg_shopShowNothing)
    {
        vItem[0] = _CTS("true");
        vItem[1] = _CTS("^vNothing");
        vItem[2].clear();
        vItem[3] = _CTS("/items/icons/scroll.tga");
        vItem[4].clear();
        vItem[5].clear();
        vItem[6] = _CTS("false");
        vItem[7].clear();
        vItem[8].clear();
        vItem[9] = _CTS("$invis");
        vItem[10].clear();
        vItem[11] = _CTS("false");
        vItem[12] = _CTS("false");
        vItem[13].clear();

        if (uiSlot < MAX_RECIPE_COMPONENTS)
        {
            Trigger(uiComponentTrigger, vItem, uiSlot);
            ++uiSlot;
        }
    }

    vItem[0] = _CTS("false");
    for (; uiSlot < MAX_RECIPE_COMPONENTS; ++uiSlot)
        Trigger(uiComponentTrigger, vItem, uiSlot);

    // Used in
    static vector<ushort> vUsedIn;
    vUsedIn.clear();

    GameClient.GetUsedIn(GameClient.GetActiveRecipe(), vUsedIn);

    uiSlot = 0;
    for (; uiSlot < vUsedIn.size() && uiSlot < MAX_RECIPE_USEDIN; ++uiSlot)
    {
        const tstring &sItem(EntityRegistry.LookupName(vUsedIn[uiSlot]));

        UpdateShopItem(
            sItem,
            GameClient.CanAccessItem(sItem),
            GameClient.CanAccessItemLocal(sItem),
            GameClient.GetItemStock(sItem),
            GameClient.GetItemRestockTime(sItem),
            false,
            false,
            true,
            pControlUnit != nullptr ? pControlUnit->GetCanCarryItems() : true,
            true,
            INVALID_ENT_TYPE,
            iKeySlot++,
            vItem
        );
        Trigger(UITRIGGER_RECIPE_USEDIN, vItem, uiSlot);

        UpdateShopItemTooltip(
            sItem,
            pControlUnit,
            UITRIGGER_RECIPE_USEDIN_TOOLTIP,
            uiSlot,
            false,
            false,
            pControlUnit != nullptr ? pControlUnit->GetCanCarryItems() : true,
            GameClient.CanAccessItem(sItem),
            GameClient.CanAccessItemLocal(sItem)
        );
    }

    if (vUsedIn.empty() && cg_shopShowNothing)
    {
        vItem[0] = _CTS("true");
        vItem[1] = _CTS("^vNothing");
        vItem[2].clear();
        vItem[3] = _CTS("/items/icons/scroll.tga");
        vItem[4].clear();
        vItem[5].clear();
        vItem[6] = _CTS("false");
        vItem[7].clear();
        vItem[8].clear();
        vItem[9] = _CTS("$invis");
        vItem[10].clear();
        vItem[11] = _CTS("false");
        vItem[12] = _CTS("false");
        vItem[13].clear();

        if (uiSlot < MAX_RECIPE_USEDIN)
        {
            Trigger(UITRIGGER_RECIPE_USEDIN, vItem, uiSlot);
            ++uiSlot;
        }
    }

    vItem[0] = _CTS("false");
    for (; uiSlot < MAX_RECIPE_USEDIN; ++uiSlot)
        Trigger(UITRIGGER_RECIPE_USEDIN, vItem, uiSlot);
}


/*====================
  CGameInterfaceManager::UpdateShop
  ====================*/
void    CGameInterfaceManager::UpdateShop()
{
    PROFILE("CGameInterfaceManager::UpdateShop");

    static tsvector vItem(26);

    if (!m_bDisplayShop)
        return;

    // Sanity checks
    CPlayer *pPlayer(GameClient.GetLocalPlayer());
    if (pPlayer == nullptr)
        return;
    CClientCommander *pCommander(GameClient.GetClientCommander());
    if (pCommander == nullptr)
        return;
    IUnitEntity *pControlUnit(pCommander->GetSelectedControlEntity());

    const tstring &sActiveShop(GameClient.GetActiveShop());
    bool bActiveShop(!sActiveShop.empty());
    bool bActiveRecipe(!GameClient.GetActiveRecipe().empty());

    // Assign shortcut keys
    uint uiSlot(0);
    for (; uiSlot < MAX_SHOPS; ++uiSlot)
    {
        if (bActiveShop)
        {
            Trigger(UITRIGGER_SHOP_KEY, TSNULL, uiSlot);
            continue;
        }

        IBaseInput *pAction(ActionRegistry.GetAction(_CTS("Shop")));
        tstring sParam(XtoA(uiSlot));

        bool bFound(false);

        const ButtonActionMap &lButton(ActionRegistry.GetButtonActionMap(BINDTABLE_GAME_SHOP));
        for (ButtonActionMap::const_iterator it(lButton.begin()); it != lButton.end() && !bFound; ++it)
        {
            if (it->first == BUTTON_INVALID)
                continue;

            for (BindModMap::const_iterator itBind(it->second.begin()); itBind != it->second.end(); ++itBind)
            {
                if (itBind->second.GetAction() == pAction && itBind->second.GetParam() == sParam)
                {
                    Trigger(UITRIGGER_SHOP_KEY, Input.GetBindString(it->first, itBind->first), uiSlot);
                    bFound = true;
                    break;
                }
            }
        }
    }

    // Retrieve shop definition
    ushort unShop(EntityRegistry.LookupID(sActiveShop));
    CShopDefinition *pShop(EntityRegistry.GetDefinition<CShopDefinition>(unShop));

    if (pShop == nullptr)
    {
        Trigger(UITRIGGER_SHOP_TYPE, TSNULL);
        Trigger(UITRIGGER_SHOP_NAME, TSNULL);
        Trigger(UITRIGGER_SHOP_DESCRIPTION, TSNULL);
        Trigger(UITRIGGER_SHOP_HEADER, TSNULL);
        Trigger(UITRIGGER_SHOP_ICON, TSNULL);

        vItem[0] = _CTS("false");

        for (uint uiSlot(0); uiSlot < MAX_SHOP_ITEMS; ++uiSlot)
            Trigger(UITRIGGER_SHOP_ITEM, vItem, uiSlot);

        Trigger(UITRIGGER_RECIPE_ITEM, vItem);

        return;
    }

    // Basic shop info triggers
    Trigger(UITRIGGER_SHOP_TYPE, sActiveShop);
    Trigger(UITRIGGER_SHOP_NAME, pShop->GetDisplayName());
    Trigger(UITRIGGER_SHOP_DESCRIPTION, pShop->GetDescription());
    Trigger(UITRIGGER_SHOP_HEADER, pShop->GetHeaderPath());
    Trigger(UITRIGGER_SHOP_ICON, pShop->GetIconPath());

    bool bShopAccess(GameClient.CanAccessShop(GameClient.GetActiveShop()));
    bool bShopLocal(GameClient.CanAccessLocalShop(GameClient.GetActiveShop()));
    bool bCarry(pControlUnit != nullptr ? pControlUnit->GetCanCarryItems() : true);

    const tsvector &vItems(pShop->GetItems());

    const tstring &sRestrictItemAccess(pControlUnit != nullptr ? pControlUnit->GetRestrictItemAccess() : TSNULL);

    // Items
    uiSlot = 0;
    for (; uiSlot < vItems.size() && uiSlot < MAX_SHOP_ITEMS; ++uiSlot)
    {
        const tstring &sItem(vItems[uiSlot]);

        bool bItemAccess(pShop->GetRecommendedItems() ? GameClient.CanAccessItem(sItem) : bShopAccess);
        bool bItemLocal(pShop->GetRecommendedItems() ? GameClient.CanAccessItemLocal(sItem) : bShopLocal);

        if (!sRestrictItemAccess.empty())
        {
            const tsvector &vsRestrictItemAccess(TokenizeString(sRestrictItemAccess, _T(' ')));
            tsvector_cit it(vsRestrictItemAccess.begin()), itEnd(vsRestrictItemAccess.end());
            while (it != itEnd)
            {
                if (*it == sItem)
                    break;

                ++it;
            }

            if (it == itEnd)
                bItemAccess = false;
        }

        UpdateShopItem(sItem, bItemAccess, bItemLocal, GameClient.GetItemStock(sItem), GameClient.GetItemRestockTime(sItem), false, false, false, bCarry, false, unShop, bActiveRecipe ? -1 : int(uiSlot), vItem);
        Trigger(UITRIGGER_SHOP_ITEM, vItem, uiSlot);

        UpdateShopItemTooltip(
            sItem,
            pControlUnit,
            UITRIGGER_SHOP_ITEM_TOOLTIP,
            uiSlot,
            false,
            false,
            bCarry,
            pShop->GetRecommendedItems() ? GameClient.CanAccessItem(sItem) : bShopAccess,
            pShop->GetRecommendedItems() ? GameClient.CanAccessItemLocal(sItem) : bShopLocal
        );

        Trigger(UITRIGGER_SHOP_ITEM_TYPE, sItem, uiSlot);
    }

    vItem[0] = _CTS("false");

    for (; uiSlot < MAX_SHOP_ITEMS; ++uiSlot)
        Trigger(UITRIGGER_SHOP_ITEM, vItem, uiSlot);

    // Recipe
    const tstring &sRecipe(GameClient.GetActiveRecipe());
    UpdateShopItem(
        sRecipe,
        GameClient.CanAccessItem(sRecipe),
        GameClient.CanAccessItemLocal(sRecipe),
        GameClient.GetItemStock(sRecipe),
        GameClient.GetItemRestockTime(sRecipe),
        false,
        false,
        false,
        bCarry,
        false,
        INVALID_ENT_TYPE,
        -1,
        vItem
    );
    Trigger(UITRIGGER_RECIPE_ITEM, vItem);

    UpdateShopItemTooltip(
        sRecipe,
        pControlUnit,
        UITRIGGER_RECIPE_ITEM_TOOLTIP,
        0,
        false,
        false,
        bCarry,
        GameClient.CanAccessItem(sRecipe),
        GameClient.CanAccessItemLocal(sRecipe)
    );

    Trigger(UITRIGGER_RECIPE_ITEM_TYPE, sRecipe);

    UpdateShopRecipe(sRecipe, pControlUnit, vItem, INVALID_INDEX);
}


/*====================
  CGameInterfaceManager::UpdateStash
  ====================*/
void    CGameInterfaceManager::UpdateStash(IUnitEntity *pUnit, IUnitEntity *pControlUnit)
{
    PROFILE("CGameInterfaceManager::UpdateStash");

    if (pUnit == nullptr)
    {
        for (int iSlot(0); iSlot < INVENTORY_STASH_SIZE; ++iSlot)
            Trigger(UITRIGGER_STASH_EXISTS, false, iSlot);

        return;
    }

    bool bCanAccess(pControlUnit != nullptr && pControlUnit->GetCanCarryItems() && !pControlUnit->GetShopAccess().empty());

    for (int iSlot(0); iSlot < INVENTORY_STASH_SIZE; ++iSlot)
    {
        IEntityItem *pItem(pUnit->GetItem(INVENTORY_START_STASH + iSlot));
        if (pItem == nullptr)
        {
            // available, level, max level
            tsvector vStatus(3);
            vStatus[0] = XtoA(bCanAccess);
            vStatus[1] = _CTS("0");
            vStatus[2] = _CTS("0");

            Trigger(UITRIGGER_STASH_STATUS, vStatus, iSlot);

            Trigger(UITRIGGER_STASH_EXISTS, false, iSlot);
            continue;
        }

        Trigger(UITRIGGER_STASH_EXISTS, true, iSlot);

        Trigger(UITRIGGER_STASH_ICON, pItem->GetIconPath(), iSlot);
        Trigger(UITRIGGER_STASH_RECIPE, !pItem->HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED), iSlot);
        Trigger(UITRIGGER_STASH_CAN_ACTIVATE, pItem->GetActionType() != TOOL_ACTION_PASSIVE, iSlot);

        // available, level, max level
        static tsvector vStatus(3);
        vStatus[0] = XtoA(pItem->CanAccess(pControlUnit));
        vStatus[1] = XtoA(pItem->GetLevel());

        if (!pItem->HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED))
            vStatus[2] = _CTS("0");
        else
            vStatus[2] = XtoA(pItem->GetMaxLevel());

        Trigger(UITRIGGER_STASH_STATUS, vStatus, iSlot);

        static tsvector vCooldown(3);
        vCooldown[0] = XtoA(pItem->GetRemainingCooldownTime());
        vCooldown[1] = XtoA(pItem->GetCooldownTime());
        vCooldown[2] = XtoA(pItem->GetRemainingCooldownPercent());
        Trigger(UITRIGGER_STASH_COOLDOWN, vCooldown, iSlot);
        Trigger(UITRIGGER_STASH_CHARGES, pItem->GetCharges(), iSlot);

        static tsvector vDescription(4);
        vDescription[0] = pItem->GetDisplayName();
        vDescription[1] = cg_tooltipFlavor ? pItem->GetDescription() : TSNULL;
        vDescription[2] = XtoA(pItem->GetManaCost());
        vDescription[3] = XtoA(pItem->GetValue());
        Trigger(UITRIGGER_STASH_DESCRIPTION, vDescription, iSlot);
    }
}


/*====================
  CGameInterfaceManager::UpdateVote
  ====================*/
void    CGameInterfaceManager::UpdateVote()
{
    PROFILE("CGameInterfaceManager::UpdateVote");

    CGameInfo *pGameInfo(GameClient.GetGameInfo());

    if (pGameInfo == nullptr)
        return;

    CPlayer *pLocalPlayer(GameClient.GetLocalPlayer());
    if (pLocalPlayer == nullptr)
        return;

    CTeamInfo *pLocalTeam(GameClient.GetTeam(pLocalPlayer->GetTeam()));
    if (pLocalTeam == nullptr)
        return;

    const map<uint, CTeamInfo*> mapTeams(GameClient.GetTeams());
    bool bPaused(false);

    for (map<uint, CTeamInfo*>::const_iterator it(mapTeams.begin()); it != mapTeams.end(); it++)
    {
        if (!it->second->IsActiveTeam())
            continue;

        if (it->second->HasFlags(TEAM_FLAG_PAUSED))
        {
            bPaused = true;
            break;
        }
    }

    bool bVoteInProgress(pGameInfo->GetActiveVoteType() != VOTE_TYPE_INVALID);
    bool bOtherTeamConceding((pGameInfo->GetActiveVoteType() == VOTE_TYPE_CONCEDE && pGameInfo->GetVoteTarget() != pLocalPlayer->GetTeam()));
    bool bBeingKicked(((pGameInfo->GetActiveVoteType() == VOTE_TYPE_KICK || pGameInfo->GetActiveVoteType() == VOTE_TYPE_KICK_AFK) && pGameInfo->GetVoteTarget() == pLocalPlayer->GetClientNumber()));
    bool bOtherTeamPausing((pGameInfo->GetActiveVoteType() == VOTE_TYPE_PAUSE && pGameInfo->GetVoteTarget() != pLocalPlayer->GetTeam()));

    CPlayer *pAFKKickTarget = nullptr;
    if (pGameInfo->GetActiveVoteType() == VOTE_TYPE_KICK_AFK)
        pAFKKickTarget = Game.GetPlayerFromClientNumber(pGameInfo->GetVoteTarget());
    bool bOtherTeamAFKKicking(pAFKKickTarget != nullptr && pAFKKickTarget->GetTeam() != pLocalPlayer->GetTeam());

    uint uiCoolDown(pLocalPlayer->GetLastVoteCallTime() == INVALID_TIME ? INVALID_TIME : GameClient.GetGameTime() - pLocalPlayer->GetLastVoteCallTime());
    if (uiCoolDown >= g_voteCooldownTime)
        uiCoolDown = 0;
    else
        uiCoolDown = g_voteCooldownTime - uiCoolDown;

    static tsvector vVotePermissions(8);
    vVotePermissions[0] = XtoA(bVoteInProgress);
    vVotePermissions[1] = XtoA(uiCoolDown);
    vVotePermissions[2] = XtoA(GameClient.GetMatchTime() >= g_voteRemakeTimeLimit ? 0 : (g_voteRemakeTimeLimit - GameClient.GetMatchTime() > 0 ? 1 : 0));
    vVotePermissions[3] = XtoA(GameClient.GetMatchTime() >= g_voteAllowConcedeTime ? 0 : (g_voteAllowConcedeTime - GameClient.GetMatchTime() > 0 ? 1 : 0));
    vVotePermissions[4] = XtoA(bVoteInProgress && !bOtherTeamConceding && !bBeingKicked && !bOtherTeamPausing && !bOtherTeamAFKKicking);
    vVotePermissions[5] = XtoA(pLocalTeam->GetRemainingPauses() > 0);
    vVotePermissions[6] = XtoA(bPaused);
    vVotePermissions[7] = XtoA(pLocalTeam->HasFlags(TEAM_FLAG_CAN_UNPAUSE));

    Trigger(UITRIGGER_VOTE_PERMISSIONS, vVotePermissions);

    static tsvector vVoteKickPermissions(2);
    vVoteKickPermissions[0] = XtoA(bVoteInProgress);
    vVoteKickPermissions[1] = XtoA(uiCoolDown);

    Trigger(UITRIGGER_VOTE_KICK_PERMISSIONS, vVoteKickPermissions);

    static tsvector vVoteType(2);

    if (!bVoteInProgress)
    {
        vVoteType[0] = TSNULL;
        vVoteType[1] = TSNULL;      
        Trigger(UITRIGGER_VOTE_TYPE, vVoteType);
        return;
    }

    switch (pGameInfo->GetActiveVoteType())
    {
    case VOTE_TYPE_CONCEDE:
        vVoteType[0] = _CTS("vote_concede");
        vVoteType[1] = TSNULL;
        Trigger(UITRIGGER_VOTE_TYPE, vVoteType);
        break;

    case VOTE_TYPE_REMAKE:
        vVoteType[0] = _CTS("vote_remake");
        vVoteType[1] = TSNULL;
        Trigger(UITRIGGER_VOTE_TYPE, vVoteType);
        break;

    case VOTE_TYPE_KICK:
        {
            CPlayer *pPlayer(GameClient.GetPlayer(pGameInfo->GetVoteTarget()));
            if (pPlayer == nullptr)
                break;
            vVoteType[0] = _CTS("vote_kick");
            vVoteType[1] = pPlayer ? (GetInlineColorString<tstring>(pPlayer->GetColor()) + pPlayer->GetName()) : TSNULL;
            Trigger(UITRIGGER_VOTE_TYPE, vVoteType);
        }
        break;

    case VOTE_TYPE_KICK_AFK:
        {
            CPlayer *pPlayer(GameClient.GetPlayer(pGameInfo->GetVoteTarget()));
            if (pPlayer == nullptr)
                break;          
            vVoteType[0] = _CTS("vote_kick_afk");
            vVoteType[1] = pPlayer ? (GetInlineColorString<tstring>(pPlayer->GetColor()) + pPlayer->GetName()) : TSNULL;
            Trigger(UITRIGGER_VOTE_TYPE, vVoteType);
        }
        break;

    case VOTE_TYPE_PAUSE:
        vVoteType[0] = _CTS("vote_pause");
        vVoteType[1] = TSNULL;
        Trigger(UITRIGGER_VOTE_TYPE, vVoteType);
        break;
    }

    Trigger(UITRIGGER_VOTE_TIME, pGameInfo->GetVoteEndTime() != INVALID_TIME ? pGameInfo->GetVoteEndTime() - GameClient.GetGameTime() : INVALID_TIME);

    Trigger(UITRIGGER_VOTED, pLocalPlayer->GetVote());

    static tsvector vVoteProgress(3);
    vVoteProgress[0] = XtoA(pGameInfo->GetYesVotes());
    vVoteProgress[1] = XtoA(pGameInfo->GetVotesRequired());
    vVoteProgress[2] = XtoA(pGameInfo->GetYesVotes() / float(pGameInfo->GetVotesRequired()));
    Trigger(UITRIGGER_VOTE_PROGRESS, vVoteProgress);
}


/*====================
  CGameInterfaceManager::UpdateSpectatorTeam
  ====================*/
void    CGameInterfaceManager::UpdateSpectatorTeam(uint uiTeam, uint uiIndex)
{
    PROFILE("CGameInterfaceManager::UpdateSpectatorTeams");

    static tsvector vTeamInfo(10);

    CTeamInfo *pTeam(Game.GetTeam(uiTeam));
    if (pTeam == nullptr)
    {
        for (uint ui(0); ui < vTeamInfo.size(); ++ui)
            vTeamInfo[ui].clear();

        return;
    }

    vTeamInfo[0] = XtoA(pTeam->GetTeamStat(PLAYER_STAT_HERO_KILLS));
    vTeamInfo[1] = XtoA(pTeam->GetTeamStat(PLAYER_STAT_DEATHS));
    vTeamInfo[2] = XtoA(INT_FLOOR(pTeam->GetExperienceEarned()));
    vTeamInfo[3] = XtoA(pTeam->GetGoldEarned());
    vTeamInfo[4] = XtoA(pTeam->GetTeamStat(PLAYER_STAT_CREEP_KILLS));
    vTeamInfo[5] = XtoA(pTeam->GetTeamStat(PLAYER_STAT_DENIES));
    vTeamInfo[6] = XtoA(pTeam->GetStat(TEAM_STAT_TOWER_DENIES));
    vTeamInfo[7] = XtoA(pTeam->GetCurrentTowerCount());
    vTeamInfo[8] = XtoA(pTeam->GetCurrentRangedCount());
    vTeamInfo[9] = XtoA(pTeam->GetCurrentMeleeCount());

    Trigger(UITRIGGER_SPECTATOR_TEAMINFO, vTeamInfo, uiIndex);

    IBuildingEntity *pBase(GameClient.GetBuildingEntity(pTeam->GetBaseBuildingIndex()));
    if (pBase != nullptr)
    {
        Trigger(UITRIGGER_BASE_HEALTH, pTeam->GetBaseHealthPercent(), uiIndex);
        Trigger(UITRIGGER_BASE_HEALTH_VISIBLE, pBase->GetLastDamageTime() + 5000 > Game.GetGameTime(), uiIndex);
    }
}


/*====================
  CGameInterfaceManager::UpdateSpectatorTeams
  ====================*/
void    CGameInterfaceManager::UpdateSpectatorTeams()
{
    PROFILE("CGameInterfaceManager::UpdateSpectatorTeams");

    UpdateSpectatorTeam(1, 0);
    UpdateSpectatorTeam(2, 1);
}


/*====================
  CGameInterfaceManager::SaveSpectatorPlayers
  ====================*/
void    CGameInterfaceManager::SaveSpectatorPlayers()
{
    static tsvector vPlayer(17);

    for (uint uiTeam(0); uiTeam < MAX_DISPLAY_TEAMS; ++uiTeam)
    {
        CTeamInfo *pTeam(GameClient.GetTeam(uiTeam + 1));
        if (pTeam == nullptr)
            continue;

        int iTotalPlayers(0);

        for (uint uiPlayer(0); uiPlayer < MAX_DISPLAY_PLAYERSPERTEAM; ++uiPlayer)
        {
            CPlayer *pClient(GameClient.GetPlayer(pTeam->GetClientIDFromTeamIndex(uiPlayer)));

            if (pClient == nullptr)
                continue;

            uint uiIndex(uiTeam * MAX_DISPLAY_PLAYERSPERTEAM + iTotalPlayers);

            m_vSavedPlayer[uiIndex].resize(17);

            IHeroEntity *pHero(pClient->GetHero());
            if (pHero == nullptr)
            {
                vPlayer[0] = pClient->GetName();
                vPlayer[1] = _T("No hero");
                vPlayer[2] = TSNULL;
                vPlayer[3] = XtoA(pClient->GetColor());
                vPlayer[4] = TSNULL;
                vPlayer[5] = XtoA(pClient->GetStat(PLAYER_STAT_HERO_KILLS));
                vPlayer[6] = XtoA(pClient->GetStat(PLAYER_STAT_DEATHS));
                vPlayer[7] = XtoA(pClient->GetStat(PLAYER_STAT_ASSISTS));
                vPlayer[8] = _T("false");
                vPlayer[9] = TSNULL;
                vPlayer[10] = TSNULL;
                vPlayer[11] = TSNULL;
                vPlayer[12] = TSNULL;
                vPlayer[13] = TSNULL;
                vPlayer[14] = TSNULL;
                vPlayer[15] = TSNULL;
                vPlayer[16] = TSNULL;

                m_vSavedPlayer[uiIndex] = vPlayer;
            }
            else
            {
                uint uiMatchTime(Game.GetFinalMatchTime());

                vPlayer[0] = pClient->GetName();
                vPlayer[1] = pHero->GetDisplayName();
                vPlayer[2] = pHero->GetIconPath();
                vPlayer[3] = XtoA(pClient->GetColor());
                vPlayer[4] = XtoA(pHero->GetLevel());
                vPlayer[5] = XtoA(pClient->GetStat(PLAYER_STAT_HERO_KILLS));
                vPlayer[6] = XtoA(pClient->GetStat(PLAYER_STAT_DEATHS));
                vPlayer[7] = XtoA(pClient->GetStat(PLAYER_STAT_ASSISTS));
                vPlayer[8] = XtoA(pHero->GetStatus() == ENTITY_STATUS_ACTIVE);

                if (floor(MsToSec(uiMatchTime)) > 0.0f)
                {
                    vPlayer[9] = XtoA(floor(pHero->GetExperience()) / SecToMin(floorf(MsToSec(uiMatchTime))), 0, 0, 0, 1);
                    vPlayer[10] = XtoA(float(int(pClient->GetGoldEarned()) - int(pClient->GetStat(PLAYER_STAT_STARTING_GOLD))) / SecToMin(floorf(MsToSec(uiMatchTime))), 0, 0, 0, 1);
                }
                else
                {
                    vPlayer[9] = _CTS("0");
                    vPlayer[10] = _CTS("0");
                }
                vPlayer[11] = XtoA(pClient->GetGold());
                vPlayer[12] = XtoA(pClient->GetStat(PLAYER_STAT_GOLD_SPENT));
                vPlayer[13] = XtoA(INT_FLOOR(pClient->GetFloatStat(PLAYER_STAT_HERO_DAMAGE)));
                vPlayer[14] = XtoA(INT_FLOOR(pClient->GetFloatStat(PLAYER_STAT_BUILDING_DAMAGE)));
                vPlayer[15] = XtoA(pClient->GetStat(PLAYER_STAT_CREEP_KILLS) + pClient->GetStat(PLAYER_STAT_NEUTRAL_KILLS));
                vPlayer[16] = XtoA(pClient->GetStat(PLAYER_STAT_DENIES));
            }

            m_vSavedPlayer[uiIndex] = vPlayer;
            ++iTotalPlayers;
        }

        for (int iPlayer(iTotalPlayers); iPlayer < MAX_DISPLAY_PLAYERSPERTEAM; ++iPlayer)
        {
            uint uiIndex(uiTeam * MAX_DISPLAY_PLAYERSPERTEAM + iTotalPlayers);

            m_vSavedPlayer[uiIndex].resize(17);

            vPlayer[0] = TSNULL;
            vPlayer[1] = TSNULL;
            vPlayer[2] = TSNULL;
            vPlayer[3] = TSNULL;
            vPlayer[4] = TSNULL;
            vPlayer[5] = TSNULL;
            vPlayer[6] = TSNULL;
            vPlayer[7] = TSNULL;
            vPlayer[8] = TSNULL;
            vPlayer[9] = TSNULL;
            vPlayer[10] = TSNULL;
            vPlayer[11] = TSNULL;
            vPlayer[12] = TSNULL;
            vPlayer[13] = TSNULL;
            vPlayer[14] = TSNULL;
            vPlayer[15] = TSNULL;
            vPlayer[16] = TSNULL;

            m_vSavedPlayer[uiIndex] = vPlayer;
            ++iTotalPlayers;
        }
    }
}


/*====================
  CGameInterfaceManager::UpdateSpectatorPlayers
  ====================*/
void    CGameInterfaceManager::UpdateSpectatorPlayers()
{
    PROFILE("CGameInterfaceManager::UpdateSpectatorPlayers");

    static tsvector vPlayer(17);

    for (uint uiTeam(0); uiTeam < MAX_DISPLAY_TEAMS; ++uiTeam)
    {
        CTeamInfo *pTeam(GameClient.GetTeam(uiTeam + 1));
        if (pTeam == nullptr)
            continue;

        int iTotalPlayers(0);

        for (uint uiPlayer(0); uiPlayer < MAX_DISPLAY_PLAYERSPERTEAM; ++uiPlayer)
        {
            CPlayer *pClient(GameClient.GetPlayer(pTeam->GetClientIDFromTeamIndex(uiPlayer)));

            if (pClient == nullptr)
                continue;

            uint uiIndex(uiTeam * MAX_DISPLAY_PLAYERSPERTEAM + iTotalPlayers);

            IHeroEntity *pHero(pClient->GetHero());
            if (pHero == nullptr)
            {
                vPlayer[0] = pClient->GetName();
                vPlayer[1] = _T("No hero");
                vPlayer[2] = TSNULL;
                vPlayer[3] = XtoA(pClient->GetColor());
                vPlayer[4] = TSNULL;
                vPlayer[5] = XtoA(pClient->GetStat(PLAYER_STAT_HERO_KILLS));
                vPlayer[6] = XtoA(pClient->GetStat(PLAYER_STAT_DEATHS));
                vPlayer[7] = XtoA(pClient->GetStat(PLAYER_STAT_ASSISTS));
                vPlayer[8] = _T("false");
                vPlayer[9] = TSNULL;
                vPlayer[10] = TSNULL;
                vPlayer[11] = TSNULL;
                vPlayer[12] = TSNULL;
                vPlayer[13] = TSNULL;
                vPlayer[14] = TSNULL;
                vPlayer[15] = TSNULL;
                vPlayer[16] = TSNULL;

                Trigger(UITRIGGER_SPECTATOR_PLAYER_HEALTH_PERCENT, 0.0f, uiIndex);
            }
            else
            {
                uint uiMatchTime(Game.GetFinalMatchTime());

                if (uiMatchTime != INVALID_TIME)
                {
                    vPlayer = m_vSavedPlayer[uiIndex];
                }
                else
                {
                    vPlayer[0] = pClient->GetName();
                    vPlayer[1] = pHero->GetDisplayName();
                    vPlayer[2] = pHero->GetIconPath();
                    vPlayer[3] = XtoA(pClient->GetColor());
                    vPlayer[4] = XtoA(pHero->GetLevel());
                    vPlayer[5] = XtoA(pClient->GetStat(PLAYER_STAT_HERO_KILLS));
                    vPlayer[6] = XtoA(pClient->GetStat(PLAYER_STAT_DEATHS));
                    vPlayer[7] = XtoA(pClient->GetStat(PLAYER_STAT_ASSISTS));
                    vPlayer[8] = XtoA(pHero->GetStatus() == ENTITY_STATUS_ACTIVE);

                    uiMatchTime = Game.GetMatchTime();

                    if (floor(MsToSec(uiMatchTime)) > 0.0f)
                    {
                        vPlayer[9] = XtoA(floor(pHero->GetExperience()) / SecToMin(floorf(MsToSec(uiMatchTime))), 0, 0, 0, 1);
                        vPlayer[10] = XtoA(float(int(pClient->GetGoldEarned()) - int(pClient->GetStat(PLAYER_STAT_STARTING_GOLD))) / SecToMin(floorf(MsToSec(uiMatchTime))), 0, 0, 0, 1);
                    }
                    else
                    {
                        vPlayer[9] = _CTS("0");
                        vPlayer[10] = _CTS("0");
                    }
                    vPlayer[11] = XtoA(pClient->GetGold());
                    vPlayer[12] = XtoA(pClient->GetStat(PLAYER_STAT_GOLD_SPENT));
                    vPlayer[13] = XtoA(INT_FLOOR(pClient->GetFloatStat(PLAYER_STAT_HERO_DAMAGE)));
                    vPlayer[14] = XtoA(INT_FLOOR(pClient->GetFloatStat(PLAYER_STAT_BUILDING_DAMAGE)));
                    vPlayer[15] = XtoA(pClient->GetStat(PLAYER_STAT_CREEP_KILLS) + pClient->GetStat(PLAYER_STAT_NEUTRAL_KILLS));
                    vPlayer[16] = XtoA(pClient->GetStat(PLAYER_STAT_DENIES));
                }

                Trigger(UITRIGGER_SPECTATOR_PLAYER_HEALTH_PERCENT, pHero->GetHealthPercent(), uiIndex);
            }

            Trigger(UITRIGGER_SPECTATOR_PLAYER, vPlayer, uiIndex);
            ++iTotalPlayers;
        }

        for (int iPlayer(iTotalPlayers); iPlayer < MAX_DISPLAY_PLAYERSPERTEAM; ++iPlayer)
        {
            vPlayer[0] = TSNULL;
            vPlayer[1] = TSNULL;
            vPlayer[2] = TSNULL;
            vPlayer[3] = TSNULL;
            vPlayer[4] = TSNULL;
            vPlayer[5] = TSNULL;
            vPlayer[6] = TSNULL;
            vPlayer[7] = TSNULL;
            vPlayer[8] = TSNULL;
            vPlayer[9] = TSNULL;
            vPlayer[10] = TSNULL;
            vPlayer[11] = TSNULL;
            vPlayer[12] = TSNULL;
            vPlayer[13] = TSNULL;
            vPlayer[14] = TSNULL;
            vPlayer[15] = TSNULL;
            vPlayer[16] = TSNULL;

            Trigger(UITRIGGER_SPECTATOR_PLAYER, vPlayer, uiTeam * MAX_DISPLAY_PLAYERSPERTEAM + iTotalPlayers);
            ++iTotalPlayers;
        }
    }
}


/*====================
  CGameInterfaceManager::UpdateSpectatorHeroes
  ====================*/
void    CGameInterfaceManager::UpdateSpectatorHeroes()
{
    PROFILE("CGameInterfaceManager::UpdateSpectatorHeroes");

    for (uint uiTeam(0); uiTeam < MAX_DISPLAY_TEAMS; ++uiTeam)
    {
        CTeamInfo *pTeam(GameClient.GetTeam(uiTeam + 1));
        if (pTeam == nullptr)
            continue;

        uint uiTotalPlayers(0);
        for (uint uiPlayer(0); uiPlayer < MAX_DISPLAY_PLAYERSPERTEAM; ++uiPlayer)
        {
            CPlayer *pClient(GameClient.GetPlayer(pTeam->GetClientIDFromTeamIndex(uiPlayer)));
            
            if (pClient == nullptr ||  pClient->HasFlags(PLAYER_FLAG_TERMINATED))
                continue;

            IHeroEntity *pHero(pClient->GetHero());
            if (pHero == nullptr)
                continue;

            uint uiIndex(uiTeam * MAX_DISPLAY_PLAYERSPERTEAM + uiTotalPlayers);

            Trigger(UITRIGGER_SPECTATOR_HERO_EXISTS, true, uiIndex);
            Trigger(UITRIGGER_SPECTATOR_HERO_INDEX, pHero->GetIndex(), uiIndex);

            static tsvector vPlayerInfo(3);
            vPlayerInfo[0] = pClient->GetName();
            vPlayerInfo[1] = XtoA(pClient->GetColor());
            vPlayerInfo[2] = XtoA(pClient->GetClientNumber());
            Trigger(UITRIGGER_SPECTATOR_HERO_PLAYER_INFO, vPlayerInfo, uiIndex);

            static tsvector vHeroInfo(3);
            vHeroInfo[0] = pHero->GetDisplayName();
            vHeroInfo[1] = pHero->GetIconPath();
            vHeroInfo[2] = XtoA(pHero->GetLevel());
            Trigger(UITRIGGER_SPECTATOR_HERO_HERO_INFO, vHeroInfo, uiIndex);

            static tsvector vHealth(3);
            vHealth[0] = XtoA(pHero->GetHealth());
            vHealth[1] = XtoA(pHero->GetMaxHealth());
            vHealth[2] = XtoA(pHero->GetHealthPercent());
            Trigger(UITRIGGER_SPECTATOR_HERO_HEALTH, vHealth, uiIndex);

            static tsvector vMana(3);
            vMana[0] = XtoA(pHero->GetMana());
            vMana[1] = XtoA(pHero->GetMaxMana());
            vMana[2] = XtoA(pHero->GetManaPercent());
            Trigger(UITRIGGER_SPECTATOR_HERO_MANA, vMana, uiIndex);

            Trigger(UITRIGGER_SPECTATOR_HERO_STATUS, pHero->GetStatus() == ENTITY_STATUS_ACTIVE, uiIndex);

            static tsvector vRespawn(3);
            if (pHero != nullptr && pHero->GetRespawnTime() != INVALID_TIME)
            {
                vRespawn[0] = XtoA(MAX(int(pHero->GetRemainingRespawnTime() - GameClient.GetServerFrameLength()), 0));
                vRespawn[1] = XtoA(pHero->GetRespawnDuration());
                vRespawn[2] = XtoA(pHero->GetRespawnPercent());
            }
            else
            {
                vRespawn[0] = TSNULL;
                vRespawn[1] = _CTS("0");
                vRespawn[2] = _CTS("1.0");
            }
            Trigger(UITRIGGER_SPECTATOR_HERO_RESPAWN, vRespawn, uiIndex);

            static tsvector vDamage(2);
            vDamage[0] = XtoA(pHero->GetAdjustedAttackDamageMin() + pHero->GetBonusDamage());
            vDamage[1] = XtoA(pHero->GetAdjustedAttackDamageMax() + pHero->GetBonusDamage());
            Trigger(UITRIGGER_SPECTATOR_HERO_DAMAGE, vDamage, uiIndex);

            Trigger(UITRIGGER_SPECTATOR_HERO_ARMOR, pHero->GetArmor(), uiIndex);
            Trigger(UITRIGGER_SPECTATOR_HERO_MAGIC_ARMOR, pHero->GetMagicArmor(), uiIndex);
            Trigger(UITRIGGER_SPECTATOR_HERO_MOVE_SPEED, pHero->GetMoveSpeed(), uiIndex);
            Trigger(UITRIGGER_SPECTATOR_HERO_ATTACK_SPEED, pHero->GetAttackSpeed(), uiIndex);
            Trigger(UITRIGGER_SPECTATOR_HERO_STRENGTH, pHero->GetStrength(), uiIndex);
            Trigger(UITRIGGER_SPECTATOR_HERO_AGILITY, pHero->GetAgility(), uiIndex);
            Trigger(UITRIGGER_SPECTATOR_HERO_INTELLIGENCE, pHero->GetIntelligence(), uiIndex);

            Trigger(UITRIGGER_SPECTATOR_HERO_GOLD, pClient->GetGold(), uiIndex);

            Trigger(UITRIGGER_SPECTATOR_HERO_DISCONNECTED, pClient->IsDisconnected(), uiIndex);
            Trigger(UITRIGGER_SPECTATOR_HERO_DISCONNECT_TIME, MAX(int(pClient->GetTerminationTime() - GameClient.GetGameTime()), 0), uiIndex);
            Trigger(UITRIGGER_SPECTATOR_HERO_LOADING_PERCENT, pClient->HasFlags(PLAYER_FLAG_LOADING) ? pClient->GetLoadingProgress() : 1.0f, uiIndex);

            UpdateAllyAbility(pHero, INVENTORY_START_ABILITIES + 0, UITRIGGER_SPECTATOR_HERO_ABILITY_0_INFO, uiIndex);
            UpdateAllyAbility(pHero, INVENTORY_START_ABILITIES + 1, UITRIGGER_SPECTATOR_HERO_ABILITY_1_INFO, uiIndex);
            UpdateAllyAbility(pHero, INVENTORY_START_ABILITIES + 2, UITRIGGER_SPECTATOR_HERO_ABILITY_2_INFO, uiIndex);
            UpdateAllyAbility(pHero, INVENTORY_START_ABILITIES + 3, UITRIGGER_SPECTATOR_HERO_ABILITY_3_INFO, uiIndex);
            UpdateAllyAbility(pHero, INVENTORY_START_ABILITIES + 4, UITRIGGER_SPECTATOR_HERO_ABILITY_4_INFO, uiIndex);

            ++uiTotalPlayers;
        }

        for (uint uiPlayer(uiTotalPlayers); uiPlayer < MAX_DISPLAY_PLAYERSPERTEAM; ++uiPlayer)
        {
            Trigger(UITRIGGER_SPECTATOR_HERO_EXISTS, false, uiTeam * MAX_DISPLAY_PLAYERSPERTEAM + uiTotalPlayers);
            ++uiTotalPlayers;
        }
    }
}


/*====================
  CGameInterfaceManager::UpdateSpectatorSelectedUnits
  ====================*/
void    CGameInterfaceManager::UpdateSpectatorSelectedUnits()
{
    PROFILE("CGameInterfaceManager::UpdateSpectatorSelectedUnits");
}


/*====================
  CGameInterfaceManager::UpdateSpectatorVoiceChat
  ====================*/
void    CGameInterfaceManager::UpdateSpectatorVoiceChat()
{
    PROFILE("CGameInterfaceManager::UpdateSpectatorVoiceChat");
}

/*====================
  CGameInterfaceManager::IsFirstBanButtonVisible
  ====================*/
bool    CGameInterfaceManager::IsFirstBanButtonVisible() const
{
    CGameInfo* pGameInfo(Game.GetGameInfo());
    if (pGameInfo == nullptr)
        return false;

    if (pGameInfo->GetGamePhase() <= GAME_PHASE_WAITING_FOR_PLAYERS)
    {
        if (pGameInfo->GetGameOptions() & GAME_OPTION_TOURNAMENT_RULES)
        {
            if (pGameInfo->GetGameMode() == GAME_MODE_BANNING_PICK ||
                pGameInfo->GetGameMode() == GAME_MODE_BANNING_DRAFT)
            {
                return true;
            }
        }
    }

    return false;
}


/*====================
  CGameInterfaceManager::ForceUpdate
  ====================*/
void    CGameInterfaceManager::ForceUpdate()
{
    ++m_uiUpdateSequence;
}


/*====================
  CGameInterfaceManager::RequestPlayerStats
  ====================*/
void    CGameInterfaceManager::RequestPlayerStats(int iAccountID)
{
    Host.GetHTTPManager()->ReleaseRequest(m_pStatsRequest);
    m_pStatsRequest = nullptr;

    if (iAccountID < 0)
    {
        static tsvector vStats(38);

        m_iRequestedStatsAccountID = -1;

        for (tsvector_it it(vStats.begin()); it != vStats.end(); ++it)
            it->clear();

        GameClient.GetInterfaceManager()->Trigger(UITRIGGER_MAIN_PLAYER_STATS, vStats);
        return;
    }

    m_pStatsRequest = Host.GetHTTPManager()->SpawnRequest();
    if (m_pStatsRequest == nullptr)
        return;

    m_iRequestedStatsAccountID = iAccountID;

    m_pStatsRequest->SetTargetURL(K2System.GetMasterServerAddress() + "/client_requester.php");
    m_pStatsRequest->AddVariable(_T("f"), _T("get_all_stats"));
    m_pStatsRequest->AddVariable(_T("account_id[0]"), iAccountID);
    m_pStatsRequest->SendPostRequest();
}


/*====================
  CGameInterfaceManager::RequestPlayerStats
  ====================*/
void    CGameInterfaceManager::RequestPlayerStats(const tstring &sName)
{
    Host.GetHTTPManager()->ReleaseRequest(m_pStatsRequest);
    m_pStatsRequest = nullptr;

    m_iRequestedStatsAccountID = -1;

    m_pStatsRequest = Host.GetHTTPManager()->SpawnRequest();
    if (m_pStatsRequest == nullptr)
        return;

    m_pStatsRequest->SetTargetURL(K2System.GetMasterServerAddress() + "/client_requester.php");
    m_pStatsRequest->AddVariable(_T("f"), _T("nick2id"));
    m_pStatsRequest->AddVariable(_T("nickname[0]"), sName);
    m_pStatsRequest->SendPostRequest();
}


/*====================
  CGameInterfaceManager::RequestMatchInfo
  ====================*/
void    CGameInterfaceManager::RequestMatchInfo(uint uiMatchID)
{
    if (Host.GetActiveClient() == nullptr)
        return;

    if (m_pRecentMatchesRequest != nullptr)
    {
        Console << _T("Queueing match info for Match ") << uiMatchID << newl;

        m_uiQueuedMatchInfoRequest = uiMatchID;

        return;
    }

    m_uiQueuedMatchInfoRequest = INVALID_INDEX;

    Console << _T("Requesting match info for Match ") << uiMatchID << newl;

    tsvector vSummaryStats(32);
    tsvector vPlayerStats(57);

    vSummaryStats[0] = _T("main_stats_retrieving_match");
    vSummaryStats[21] = _T("1");

    Trigger(UITRIGGER_MATCH_INFO_SUMMARY, vSummaryStats);

    vPlayerStats[1] = _T("0");

    for (uint i(0); i < MAX_DISPLAY_PLAYERSPERTEAM; i++)
        GameClient.GetInterfaceManager()->Trigger(UITRIGGER_MATCH_INFO_PLAYER, vPlayerStats, i);

    for (uint i(0); i < MAX_DISPLAY_TEAMS; i++)
    {
        m_vCurrentGameStatsPlayers[i].clear();
        m_vCurrentGameStatsPlayers[i].resize(MAX_DISPLAY_PLAYERSPERTEAM);
    }

    if (m_vLastGameStatsSummary.size() > 1 && AtoI(m_vLastGameStatsSummary[1]) == uiMatchID)
    {
        m_vCurrentGameStatsSummary = m_vLastGameStatsSummary;
        Trigger(UITRIGGER_MATCH_INFO_SUMMARY, m_vLastGameStatsSummary);

        for (uint i(0); i < MAX_DISPLAY_TEAMS; i++)
        {
            for (uint x(0); x < MAX_DISPLAY_PLAYERSPERTEAM; x++)
            {
                if (m_vLastGameStatsPlayers[i][x].size() < 1 || m_vLastGameStatsPlayers[i][x][0].empty())
                    continue;

                m_vCurrentGameStatsPlayers[i][x] = m_vLastGameStatsPlayers[i][x];
                GameClient.GetInterfaceManager()->Trigger(UITRIGGER_MATCH_INFO_PLAYER, m_vLastGameStatsPlayers[i][x], x);
            }
        }

        return;
    }

    Host.GetHTTPManager()->ReleaseRequest(m_pMatchInfoRequest);
    m_pMatchInfoRequest = Host.GetHTTPManager()->SpawnRequest();
    if (m_pMatchInfoRequest == nullptr)
        return;

    if (uiMatchID != 0 && uiMatchID != -1)
    {
        m_pMatchInfoRequest->SetTargetURL(K2System.GetMasterServerAddress() + "/client_requester.php");
        m_pMatchInfoRequest->AddVariable(_T("f"), _T("get_match_stats"));
        m_pMatchInfoRequest->AddVariable(_T("match_id[0]"), uiMatchID);
        m_pMatchInfoRequest->AddVariable(_T("cookie"), Host.GetActiveClient()->GetCookie());
        m_pMatchInfoRequest->SendPostRequest();
    }
}


/*====================
  CGameInterfaceManager::RequestTournamentInfo
  ====================*/
void    CGameInterfaceManager::RequestTournamentInfo(uint uiTournamentID)
{
    if (Host.GetActiveClient() == nullptr)
        return;

    Host.GetHTTPManager()->ReleaseRequest(m_pTournamentRequest);
    m_pTournamentRequest = Host.GetHTTPManager()->SpawnRequest();
    if (m_pTournamentRequest == nullptr)
        return;

    m_pTournamentRequest->SetType(GET_TOURNAMENT_INFO);
    m_pTournamentRequest->SetTargetURL(_T("tournaments.heroesofnewerth.com/tourn_requester.php"));
    m_pTournamentRequest->AddVariable(_T("f"), _T("get_tournament_info"));
    m_pTournamentRequest->AddVariable(_T("tourn_id"), uiTournamentID);
    //m_pTournamentRequest->AddVariable(_T("cookie"), Host.GetActiveClient()->GetCookie());
    m_pTournamentRequest->SendPostRequest();
}


/*====================
  CGameInterfaceManager::RequestTournamentsForAccount
  ====================*/
void    CGameInterfaceManager::RequestTournamentsForAccount(uint uiAccountID)
{
    if (Host.GetActiveClient() == nullptr)
        return;

    Host.GetHTTPManager()->ReleaseRequest(m_pTournamentRequest);
    m_pTournamentRequest = Host.GetHTTPManager()->SpawnRequest();
    if (m_pTournamentRequest == nullptr)
        return;

    m_pTournamentRequest->SetType(GET_TOURNAMENTS_FOR_ACCOUNT);
    m_pTournamentRequest->SetTargetURL(_T("tournaments.heroesofnewerth.com/tourn_requester.php"));
    m_pTournamentRequest->AddVariable(_T("f"), _T("get_tournaments_for_account"));
    m_pTournamentRequest->AddVariable(_T("account_id"), uiAccountID);
    //m_pTournamentRequest->AddVariable(_T("account_id"), Host.GetActiveClient()->GetAccountID());
    //m_pTournamentRequest->AddVariable(_T("cookie"), Host.GetActiveClient()->GetCookie());
    m_pTournamentRequest->SendPostRequest();
}


/*====================
  CGameInterfaceManager::RequestRecentMatches
  ====================*/
void    CGameInterfaceManager::RequestRecentMatches()
{
    if (Host.GetActiveClient() == nullptr)
        return;

    Console << _T("Requesting recent matches") << newl;

    tsvector vSummaryStats(32);

    vSummaryStats[0] = _T("main_stats_retrieving_matches");
    vSummaryStats[21] = _T("0");
    Trigger(UITRIGGER_MATCH_INFO_SUMMARY, vSummaryStats);

    Host.GetHTTPManager()->ReleaseRequest(m_pRecentMatchesRequest);
    m_pRecentMatchesRequest = Host.GetHTTPManager()->SpawnRequest();
    if (m_pRecentMatchesRequest == nullptr)
        return;

    m_pRecentMatchesRequest->SetTargetURL(K2System.GetMasterServerAddress() + "/client_requester.php");
    m_pRecentMatchesRequest->AddVariable(_T("f"), _T("grab_last_matches"));
    m_pRecentMatchesRequest->AddVariable(_T("account_id"), Host.GetActiveClient()->GetAccountID());
    m_pRecentMatchesRequest->SendPostRequest();
}


/*====================
  CGameInterfaceManager::ClearMatchInfo
  ====================*/
void    CGameInterfaceManager::ClearMatchInfo()
{
    tsvector vPlayerStats(57);
    tsvector vSummaryStats(32);

    for (uint i(0); i < MAX_DISPLAY_PLAYERSPERTEAM; i++)
        Trigger(UITRIGGER_MATCH_INFO_PLAYER, vPlayerStats, i);

    Trigger(UITRIGGER_MATCH_INFO_SUMMARY, vSummaryStats);
    Trigger(UITRIGGER_MATCH_INFO_ENTRY, TSNULL);
}


/*====================
  CGameInterfaceManager::ProcessStatsRequest
  ====================*/
void    CGameInterfaceManager::ProcessStatsRequest()
{
    if (m_pStatsRequest == nullptr || m_pStatsRequest->IsActive())
        return;

    if (!m_pStatsRequest->WasSuccessful())
    {
        Console.Warn << _T("PlayerStatsRequest: Error in response") << newl;
        Host.GetHTTPManager()->ReleaseRequest(m_pStatsRequest);
        m_pStatsRequest = nullptr;
        m_iRequestedStatsAccountID = -1;
        return;
    }

    const CPHPData phpResponse(m_pStatsRequest->GetResponse());

    Host.GetHTTPManager()->ReleaseRequest(m_pStatsRequest);
    m_pStatsRequest = nullptr;

    if (!phpResponse.IsValid())
    {
        Console.Warn << _T("PlayerStatsRequest: Bad data") << newl;
        m_iRequestedStatsAccountID = -1;
        return;
    }

    if (!phpResponse.GetBool(_T("0")))
    {
        Console.Warn << _T("PlayerStatsRequest: Server error") << newl;
        m_iRequestedStatsAccountID = -1;
        return;
    }

    const CPHPData *pClanData(phpResponse.GetVar(_T("clan_info")));
    tstring sClanName;
    tstring sRank;

    if (pClanData != nullptr && pClanData->IsValid())
    {
        pClanData = pClanData->GetVar(0);

        if (pClanData != nullptr)
        {
            sClanName = pClanData->GetString(_T("name"));
            sRank = pClanData->GetString(_T("rank"));
        }
    }


    const CPHPData *pStatsData(phpResponse.GetVar(_T("all_stats")));
    if (pStatsData == nullptr || !pStatsData->IsValid())
    {
        if (m_iRequestedStatsAccountID != -1)
        {
            Console.Warn << _T("PlayerStatsRequest: Missing data") << newl;
            RequestPlayerStats(-1);
            return;
        }
        else
        {
            const CPHPData *pIDData(phpResponse.GetVar(1));

            if (pIDData == nullptr || !pIDData->IsValid())
            {
                Console.Warn << _T("PlayerStatsRequest: Missing data") << newl;
                RequestPlayerStats(-1);
                return;
            }

            RequestPlayerStats(pIDData->GetInteger());
            return;
        }
    }

    pStatsData = pStatsData->GetVar(XtoA(m_iRequestedStatsAccountID));
    if (pStatsData == nullptr || !pStatsData->IsValid())
    {
        Console << _T("PlayerStatsRequest: Missing stats for account #") << m_iRequestedStatsAccountID << newl;
        return;
    }

    if (ChatManager.IsRetrievingStats())
    {
        ChatManager.SetRetrievingStats(false);

        ChatManager.AddIRCChatMessage(CHAT_MESSAGE_ADD, ChatManager.Translate(_T("chat_stats_header"), _T("name"), pStatsData->GetString(_T("nickname"))));

        tsmapts mapStats;

        float fMatches = AtoI(pStatsData->GetString(_T("acc_games_played")));

        mapStats.insert(pair<tstring, tstring>(_T("level"), pStatsData->GetString(_T("level"))));
        mapStats.insert(pair<tstring, tstring>(_T("wins"), pStatsData->GetString(_T("acc_wins"))));
        mapStats.insert(pair<tstring, tstring>(_T("losses"), pStatsData->GetString(_T("acc_losses"))));
        mapStats.insert(pair<tstring, tstring>(_T("winpercent"), fMatches > 0 ? XtoA(INT_ROUND((AtoF(pStatsData->GetString(_T("acc_wins"))) / fMatches) * 100)) + _T("%") : _T("100%")));
        mapStats.insert(pair<tstring, tstring>(_T("games"), XtoA(AtoI(pStatsData->GetString(_T("acc_games_played"))))));
        mapStats.insert(pair<tstring, tstring>(_T("disconnects"), pStatsData->GetString(_T("acc_discos"))));
        mapStats.insert(pair<tstring, tstring>(_T("leavepercent"), (fMatches > 0 ? XtoA(INT_ROUND((AtoI(pStatsData->GetString(_T("acc_discos"))) / fMatches) * 100)) + _T("%") : _T("0%"))));
        mapStats.insert(pair<tstring, tstring>(_T("kills"), pStatsData->GetString(_T("acc_herokills"))));
        mapStats.insert(pair<tstring, tstring>(_T("deaths"), pStatsData->GetString(_T("acc_deaths"))));
        mapStats.insert(pair<tstring, tstring>(_T("assists"), pStatsData->GetString(_T("acc_heroassists"))));
        mapStats.insert(pair<tstring, tstring>(_T("avgkills"), (fMatches > 0 ? XtoA(AtoF(pStatsData->GetString(_T("acc_herokills"))) / fMatches, 0, 0, 2) : _T("0"))));
        mapStats.insert(pair<tstring, tstring>(_T("avgdeaths"), (fMatches > 0 ? XtoA(AtoF(pStatsData->GetString(_T("acc_deaths"))) / fMatches, 0, 0, 2) : _T("0"))));
        mapStats.insert(pair<tstring, tstring>(_T("avgassists"), (fMatches > 0 ? XtoA(AtoF(pStatsData->GetString(_T("acc_heroassists"))) / fMatches, 0, 0, 2) : _T("0"))));
        mapStats.insert(pair<tstring, tstring>(_T("expmin"), (SecToMin(AtoF(pStatsData->GetString(_T("acc_secs")))) > 0 ? XtoA(AtoF(pStatsData->GetString(_T("acc_exp"))) / SecToMin(AtoF(pStatsData->GetString(_T("acc_secs")))), 0, 0, 2) : _T("0"))));
        mapStats.insert(pair<tstring, tstring>(_T("psr"), pStatsData->GetString(_T("acc_pub_skill"))));
        mapStats.insert(pair<tstring, tstring>(_T("clan_name"), sClanName));

        ChatManager.AddIRCChatMessage(CHAT_MESSAGE_ADD, ChatManager.Translate(_T("chat_stats_body1"), mapStats));
        ChatManager.AddIRCChatMessage(CHAT_MESSAGE_ADD, ChatManager.Translate(_T("chat_stats_body2"), mapStats));
    }
    else
    {
        static tsvector vStats(38);
        vStats[0] = pStatsData->GetString(_T("acc_wins"));
        vStats[1] = pStatsData->GetString(_T("acc_losses"));
        vStats[2] = pStatsData->GetString(_T("acc_avg_score"));
        vStats[3] = pStatsData->GetString(_T("acc_herokills"));
        vStats[4] = pStatsData->GetString(_T("acc_herodmg"));
        vStats[5] = pStatsData->GetString(_T("acc_herokillsgold"));
        vStats[6] = pStatsData->GetString(_T("acc_heroassists"));
        vStats[7] = pStatsData->GetString(_T("acc_deaths"));
        vStats[8] = pStatsData->GetString(_T("acc_goldlost2death"));
        vStats[9] = pStatsData->GetString(_T("acc_secs_dead"));
        vStats[10] = pStatsData->GetString(_T("acc_teamcreepkills"));
        vStats[11] = pStatsData->GetString(_T("acc_teamcreepdmg"));
        vStats[12] = pStatsData->GetString(_T("acc_teamcreepgold"));
        vStats[13] = pStatsData->GetString(_T("acc_neutralcreepkills"));
        vStats[14] = pStatsData->GetString(_T("acc_neutralcreepdmg"));
        vStats[15] = pStatsData->GetString(_T("acc_neutralcreepgold"));
        vStats[16] = pStatsData->GetString(_T("acc_bdmg"));
        vStats[17] = pStatsData->GetString(_T("acc_razed"));
        vStats[18] = pStatsData->GetString(_T("acc_bgold"));
        vStats[19] = pStatsData->GetString(_T("acc_denies"));
        vStats[20] = pStatsData->GetString(_T("acc_exp_denied"));
        vStats[21] = pStatsData->GetString(_T("acc_gold"));
        vStats[22] = pStatsData->GetString(_T("acc_gold_spent"));
        vStats[23] = pStatsData->GetString(_T("acc_exp"));
        vStats[24] = pStatsData->GetString(_T("acc_actions"));
        vStats[25] = pStatsData->GetString(_T("acc_secs"));
        vStats[26] = pStatsData->GetString(_T("acc_discos"));
        vStats[27] = pStatsData->GetString(_T("nickname"));
        vStats[28] = sClanName;
        vStats[29] = sRank;
        vStats[30] = pStatsData->GetString(_T("level"));
        vStats[31] = pStatsData->GetString(_T("create_date"));
        vStats[32] = pStatsData->GetString(_T("last_activity"));
        vStats[33] = pStatsData->GetString(_T("minXP"));
        vStats[34] = pStatsData->GetString(_T("maxXP"));
        vStats[35] = pStatsData->GetString(_T("acc_pub_skill"));
        vStats[36] = sClanName;
        vStats[37] = pStatsData->GetString(_T("acc_em_played"));
        

        if (Host.GetActiveClient() != nullptr && Host.GetActiveClient()->GetAccountID() == m_iRequestedStatsAccountID)
            Trigger(UITRIGGER_MAIN_LOCAL_PLAYER_STATS, vStats);

        Trigger(UITRIGGER_MAIN_PLAYER_STATS, vStats);
    }

    m_iRequestedStatsAccountID = -1;
}


/*====================
  CGameInterfaceManager::ProcessMatchInfoRequest
  ====================*/
void    CGameInterfaceManager::ProcessMatchInfoRequest()
{
    if (m_pMatchInfoRequest == nullptr || m_pMatchInfoRequest->IsActive())
        return;

    Console << _T("Processing match info request") << newl;

    tsvector vSummaryStats(32);

    vSummaryStats[21] = _T("1");

    if (!m_pMatchInfoRequest->WasSuccessful())
    {
        Console.Warn << _T("RequestMatchInfo: Failed") << newl;
        Host.GetHTTPManager()->ReleaseRequest(m_pMatchInfoRequest);
        m_pMatchInfoRequest = nullptr;

        vSummaryStats[0] = _T("main_stats_failed_retrieval");
        Trigger(UITRIGGER_MATCH_INFO_SUMMARY, vSummaryStats);
        return;
    }

    const CPHPData phpResponse(m_pMatchInfoRequest->GetResponse());

    // The response is now stored in the PHP array object, so the request can be released
    Host.GetHTTPManager()->ReleaseRequest(m_pMatchInfoRequest);
    m_pMatchInfoRequest = nullptr;

    if (!phpResponse.IsValid() || !phpResponse.GetBool(_T("0")))
    {
        Console.Warn << _T("RequestMatchInfo: Bad data") << newl;
        vSummaryStats[0] = _T("main_stats_failed_retrieval");
        Trigger(UITRIGGER_MATCH_INFO_SUMMARY, vSummaryStats);
        return;
    }

    const CPHPData *pSummary(phpResponse.GetVar(_T("match_summ")));
    if (pSummary != nullptr)
        pSummary = pSummary->GetVar(0);
    if (pSummary == nullptr)
    {
        Console.Warn << _T("RequestMatchInfo: No summary data") << newl;
        vSummaryStats[0] = _T("main_stats_does_not_exist");
        Trigger(UITRIGGER_MATCH_INFO_SUMMARY, vSummaryStats);
        return;
    }

    // Player stats
    const CPHPData *pPlayerData(phpResponse.GetVar(_T("match_player_stats")));
    if (pPlayerData != nullptr)
        pPlayerData = pPlayerData->GetVar(0);
    if (pPlayerData == nullptr)
    {
        Console.Warn << _T("RequestMatchInfo: No player data") << newl;
        vSummaryStats[0] = _T("main_stats_failed_retrieval");
        Trigger(UITRIGGER_MATCH_INFO_SUMMARY, vSummaryStats);
        return;
    }

    tsvector vPlayerStats(57);
    for (uint i(0); i < MAX_DISPLAY_TEAMS; ++i)
    {
        m_vCurrentGameStatsPlayers[i].clear();
        m_vCurrentGameStatsPlayers[i].resize(MAX_DISPLAY_PLAYERSPERTEAM);
    }

    for (uint i(0); i < pPlayerData->GetSize(); i++)
    {
        const CPHPData *pPlayer(pPlayerData->GetVar(i));

        if (pPlayer == nullptr)
            continue;

        if (pPlayer->GetString(_T("team")).empty())
            continue;

        const float fTime = pPlayer->GetInteger(_T("secs")) / 60.0f;        
        const float fTimeEarningExp = pPlayer->GetInteger(_T("time_earning_exp")) / 60.0f;

        vPlayerStats[0] = pPlayer->GetString(_T("nickname"));
        vPlayerStats[1] = pPlayer->GetString(_T("team"));
        vPlayerStats[2] = pPlayer->GetString(_T("position"));
        vPlayerStats[3] = pPlayer->GetString(_T("level"));
        vPlayerStats[4] = pPlayer->GetString(_T("herokills"));
        vPlayerStats[5] = XtoA(pPlayer->GetInteger(_T("teamcreepkills")) + pPlayer->GetInteger(_T("neutralcreepkills")));
        vPlayerStats[6] = fTimeEarningExp != 0.0f ? XtoA(pPlayer->GetInteger(_T("exp")) / fTimeEarningExp, 0, 0, 1) : _T("0");
        vPlayerStats[7] = fTime != 0.0f ? XtoA(pPlayer->GetInteger(_T("gold")) / fTime, 0, 0, 1) : _T("0");
        vPlayerStats[8] = fTime != 0.0f ? XtoA(pPlayer->GetInteger(_T("actions")) / fTime, 0, 0, 1) : _T("0");
        vPlayerStats[9] = pPlayer->GetString(_T("denies"));

        vPlayerStats[17] = pPlayer->GetString(_T("deaths"));
        vPlayerStats[18] = pPlayer->GetString(_T("heroassists"));

        CHeroDefinition *pHeroDef(EntityRegistry.GetDefinition<CHeroDefinition>(pPlayer->GetString(_T("cli_name"))));

        vPlayerStats[10] = XtoA(pHeroDef ? pHeroDef->GetIconPath(0) : TSNULL);          // Hero icon
        vPlayerStats[19] = XtoA(pHeroDef ? pHeroDef->GetDisplayName() : TSNULL);        // Hero name
        vPlayerStats[20] = XtoA(pHeroDef ? pHeroDef->GetName() : TSNULL);               // Hero type ID (Hero_Armadon)
            
        // Inventory data
        for (uint x(11); x < 17; ++x)
            vPlayerStats[x] = TSNULL;

        const CPHPData *pInventory(phpResponse.GetVar(_T("inventory")));
        if (pInventory != nullptr)
            pInventory = pInventory->GetVar(0);

        if (pInventory != nullptr)
        {
            const CPHPData *pItem(pInventory->GetVar(pPlayer->GetString(_T("account_id"))));
            if (pItem != nullptr)
            {
                for (uint x = 1; x < 7; ++x)
                {
                    CItemDefinition *pItemDef(EntityRegistry.GetDefinition<CItemDefinition>(pItem->GetString(_T("slot_") + XtoA(x))));

                    if (pItemDef != nullptr)
                        vPlayerStats[10 + x] = pItemDef->GetIconPath(0);
                }
            }
        }

        // Performance
        vPlayerStats[21] = pPlayer->GetString(_T("perf_amm_team_rating"));
        vPlayerStats[22] = pPlayer->GetString(_T("perf_amm_team_rating_delta"));
        vPlayerStats[23] = pPlayer->GetString(_T("perf_victory_exp"));
        vPlayerStats[24] = pPlayer->GetString(_T("perf_victory_gc"));
        vPlayerStats[25] = pPlayer->GetString(_T("perf_first_exp"));
        vPlayerStats[26] = pPlayer->GetString(_T("perf_first_gc"));
        vPlayerStats[27] = pPlayer->GetString(_T("perf_quick_exp"));
        vPlayerStats[28] = pPlayer->GetString(_T("perf_quick_gc"));
        vPlayerStats[29] = pPlayer->GetString(_T("perf_consec_played"));
        vPlayerStats[30] = pPlayer->GetString(_T("perf_consec_exp"));
        vPlayerStats[31] = pPlayer->GetString(_T("perf_consec_gc"));
        vPlayerStats[32] = pPlayer->GetString(_T("perf_annihilation_exp"));
        vPlayerStats[33] = pPlayer->GetString(_T("perf_annihilation_gc"));
        vPlayerStats[34] = pPlayer->GetString(_T("perf_bloodlust_exp"));
        vPlayerStats[35] = pPlayer->GetString(_T("perf_bloodlust_gc"));
        vPlayerStats[36] = pPlayer->GetString(_T("perf_ks15_exp"));
        vPlayerStats[37] = pPlayer->GetString(_T("perf_ks15_gc"));
        vPlayerStats[38] = pPlayer->GetString(_T("perf_wins"));
        vPlayerStats[39] = pPlayer->GetString(_T("perf_wins_delta"));
        vPlayerStats[40] = pPlayer->GetString(_T("perf_wins_gc"));
        vPlayerStats[41] = pPlayer->GetString(_T("perf_herokills"));
        vPlayerStats[42] = pPlayer->GetString(_T("perf_herokills_delta"));
        vPlayerStats[43] = pPlayer->GetString(_T("perf_herokills_gc"));
        vPlayerStats[44] = pPlayer->GetString(_T("perf_heroassists"));
        vPlayerStats[45] = pPlayer->GetString(_T("perf_heroassists_delta"));
        vPlayerStats[46] = pPlayer->GetString(_T("perf_heroassists_gc"));
        vPlayerStats[47] = pPlayer->GetString(_T("perf_wards"));
        vPlayerStats[48] = pPlayer->GetString(_T("perf_wards_delta"));
        vPlayerStats[49] = pPlayer->GetString(_T("perf_wards_gc"));
        vPlayerStats[50] = pPlayer->GetString(_T("perf_smackdown"));
        vPlayerStats[51] = pPlayer->GetString(_T("perf_smackdown_delta"));
        vPlayerStats[52] = pPlayer->GetString(_T("perf_smackdown_gc"));
        vPlayerStats[53] = pPlayer->GetString(_T("perf_level"));
        vPlayerStats[54] = pPlayer->GetString(_T("perf_level_exp"));
        vPlayerStats[55] = pPlayer->GetString(_T("perf_level_delta"));
        vPlayerStats[56] = pPlayer->GetString(_T("perf_level_gc"));

        int iPos(CLAMP(pPlayer->GetInteger(_T("position")) - ((pPlayer->GetInteger(_T("team")) - 1) * MAX_DISPLAY_PLAYERSPERTEAM), 0, MAX_DISPLAY_PLAYERSPERTEAM));

        m_vCurrentGameStatsPlayers[pPlayer->GetInteger(_T("team")) - 1][iPos] = vPlayerStats;
        Trigger(UITRIGGER_MATCH_INFO_PLAYER, vPlayerStats, iPos);
    }

    // Team stats
    const CPHPData *pTeams(phpResponse.GetVar(_T("team_summ")));
    if (pTeams != nullptr)
        pTeams = pTeams->GetVar(0);
    if (pTeams == nullptr)
    {
        Console.Warn << _T("RequestMatchInfo: No team data") << newl;
        vSummaryStats[0] = _T("main_stats_failed_retrieval");
        Trigger(UITRIGGER_MATCH_INFO_SUMMARY, vSummaryStats);
        return;
    }

    int iWinner(0);
    if (pTeams->GetVar(_T("1")) != nullptr && pTeams->GetVar(_T("2")) != nullptr)
    {
        if (pTeams->GetVar(_T("1"))->GetInteger(_T("tm_wins")) > pTeams->GetVar(_T("2"))->GetInteger(_T("tm_wins")))
            iWinner = 1;
        else if (pTeams->GetVar(_T("1"))->GetInteger(_T("tm_wins")) < pTeams->GetVar(_T("2"))->GetInteger(_T("tm_wins")))
            iWinner = 2;
    }

    tstring sUrl(pSummary->GetString(_T("url")));
    int iSize(pSummary->GetInteger(_T("size"), 0));

    if (iSize == 0)
        sUrl.clear();

    tstring sPath(!sUrl.empty() ? _T("~/replays/") + Filename_StripPath(sUrl) : TSNULL);
    
    bool bFileExists;

    if (!sPath.empty())
    {
        struct _stat stat;
        if (FileManager.Stat(sPath, stat))
            bFileExists = size_t(stat.st_size) == size_t(iSize);
        else
            bFileExists = false;
    }
    else
        bFileExists = false;

    vSummaryStats[1] = pSummary->GetString(_T("match_id"));
    vSummaryStats[2] = pSummary->GetString(_T("date"));
    vSummaryStats[3] = pSummary->GetString(_T("time"));
    vSummaryStats[4] = pSummary->GetString(_T("name"));
    vSummaryStats[5] = XtoA((pSummary->GetInteger(_T("time_played")) * 1000));
    vSummaryStats[6] = pSummary->GetString(_T("ap"));
    vSummaryStats[7] = pSummary->GetString(_T("alt_pick"));
    vSummaryStats[8] = pSummary->GetString(_T("dm"));
    vSummaryStats[9] = pSummary->GetString(_T("em"));
    vSummaryStats[10] = pSummary->GetString(_T("ar"));
    vSummaryStats[11] = pSummary->GetString(_T("nl"));
    vSummaryStats[12] = pSummary->GetString(_T("nm"));
    vSummaryStats[13] = pSummary->GetString(_T("rd"));
    vSummaryStats[14] = pSummary->GetString(_T("shuf"));
    vSummaryStats[15] = pSummary->GetString(_T("sd"));
    vSummaryStats[16] = pSummary->GetString(_T("mname"));
    vSummaryStats[17] = XtoA(iWinner);
    vSummaryStats[18] = K2_Version(pSummary->GetString(_T("version")));
    vSummaryStats[19] = sUrl;
    vSummaryStats[20] = XtoA(iSize);
    vSummaryStats[22] = XtoA(bFileExists);
    vSummaryStats[23] = sPath;
    vSummaryStats[24] = XtoA(FileManager.IsCompatVersionSupported(pSummary->GetString(_T("version"))));
    vSummaryStats[25] = _CTS("false");
    vSummaryStats[26] = pSummary->GetString(_T("bd"));
    vSummaryStats[27] = pSummary->GetString(_T("bp"));
    vSummaryStats[28] = pSummary->GetString(_T("ab"));
    vSummaryStats[29] = phpResponse.GetString(_T("points"));
    vSummaryStats[30] = phpResponse.GetString(_T("selected_upgrades"));
    vSummaryStats[31] = pSummary->GetString(_T("cas"));

    m_vCurrentGameStatsSummary = vSummaryStats;

    Trigger(UITRIGGER_MATCH_INFO_SUMMARY, vSummaryStats);

    if (bFileExists)
    {
        m_iReplayURLTesting = 2; // Already downloaded
    }
    else
    {
        if (iSize > 0)
        {
            m_bReplayURLValid = false;
            m_iReplayURLTesting = 0; // Testing
            m_sTestReplayURL = sUrl;
            m_uiTestReplayURLSize = uint(iSize);
        }
        else
        {
            m_bReplayURLValid = false;
            m_iReplayURLTesting = -1;
            m_sTestReplayURL.clear();
            m_uiTestReplayURLSize = 0;
        }
    }
}


/*====================
  CGameInterfaceManager::ProcessTournamentRequest
  ====================*/
void    CGameInterfaceManager::ProcessTournamentRequest()
{
    if (m_pTournamentRequest == nullptr || m_pTournamentRequest->IsActive())
        return;

    if (!m_pTournamentRequest->WasSuccessful())
    {
        Host.GetHTTPManager()->ReleaseRequest(m_pTournamentRequest);
        m_pTournamentRequest = nullptr;
        Console.Warn << _T("RequestTournamentData: Error in response") << newl;
        return;
    }

    const CPHPData phpResponse(m_pTournamentRequest->GetResponse());

    uint uiType(m_pTournamentRequest->GetType());
    Host.GetHTTPManager()->ReleaseRequest(m_pTournamentRequest);
    m_pTournamentRequest = nullptr;

    if (!phpResponse.IsValid())
    {
        Console.Warn << _T("RequestTournamentData: Bad data") << newl;
        return;
    }

    switch (uiType)
    {
    case GET_TOURNAMENT_INFO:
        {
            const CPHPData *pTournInfo(phpResponse.GetVar(_T("tourn_info"))->GetVar(0));
            if (pTournInfo == nullptr)
            {
                Console.Warn << _T("RequestTournamentData: No information for given tournament.") << newl;
                return;
            }

            tsvector vParams;
            vParams.emplace_back(pTournInfo->GetString(_T("tourn_id")));
            vParams.emplace_back(pTournInfo->GetString(_T("tourn_name")));
            vParams.emplace_back(pTournInfo->GetString(_T("tourn_admins")));
            vParams.emplace_back(pTournInfo->GetString(_T("tourn_nextmatchname")));
            vParams.emplace_back(pTournInfo->GetString(_T("tourn_nextmatchdate")));
            vParams.emplace_back(pTournInfo->GetString(_T("tourn_nextmatchtime")));
            vParams.emplace_back(pTournInfo->GetString(_T("tourn_bracket")));
            vParams.emplace_back(pTournInfo->GetString(_T("tourn_pool")));
            vParams.emplace_back(pTournInfo->GetString(_T("tourn_num_current")));
            vParams.emplace_back(pTournInfo->GetString(_T("tourn_num_count")));
            vParams.emplace_back(pTournInfo->GetString(_T("tourn_id_prev")));
            vParams.emplace_back(pTournInfo->GetString(_T("tourn_id_next")));
            vParams.emplace_back(pTournInfo->GetString(_T("tourn_matchtimestart")));
            
            vParams.emplace_back(pTournInfo->GetString(_T("map")));
            vParams.emplace_back(pTournInfo->GetString(_T("gamemode")));
            vParams.emplace_back(pTournInfo->GetString(_T("allheroes")));
            vParams.emplace_back(pTournInfo->GetString(_T("randomhero")));
            vParams.emplace_back(pTournInfo->GetString(_T("easymode")));
            
            vParams.emplace_back(pTournInfo->GetString(_T("advancedoptions")));
            vParams.emplace_back(pTournInfo->GetString(_T("noherorepick")));
            vParams.emplace_back(pTournInfo->GetString(_T("noheroswap")));
            vParams.emplace_back(pTournInfo->GetString(_T("noagiheroes")));
            vParams.emplace_back(pTournInfo->GetString(_T("nointheroes")));
            vParams.emplace_back(pTournInfo->GetString(_T("nostrheroes")));
            vParams.emplace_back(pTournInfo->GetString(_T("dropitems")));
            vParams.emplace_back(pTournInfo->GetString(_T("nopowerups")));
            vParams.emplace_back(pTournInfo->GetString(_T("norespawntimer")));
            vParams.emplace_back(pTournInfo->GetString(_T("altheropicking")));
            vParams.emplace_back(pTournInfo->GetString(_T("dupliateheroes")));
            vParams.emplace_back(pTournInfo->GetString(_T("reverseheroselect")));
            vParams.emplace_back(pTournInfo->GetString(_T("allowveto")));

            vParams.emplace_back(ChatManager.GetTournamentAddress(pTournInfo->GetInteger(_T("tourn_id"))));
                            
            const CPHPData *pTournInfoTeam1AccountList(pTournInfo->GetVar(_T("team1_account_list")));
            const CPHPData *pTournInfoTeam1PlayerList(pTournInfo->GetVar(_T("team1_name_list")));
            if (pTournInfoTeam1AccountList != nullptr && pTournInfoTeam1PlayerList != nullptr)
            {               
                uint uiTournInfoPlayerListArrayNum(0);
                const CPHPData *pTournInfoTeam1Account(pTournInfoTeam1AccountList->GetVar(uiTournInfoPlayerListArrayNum));
                const CPHPData *pTournInfoTeam1Player(pTournInfoTeam1PlayerList->GetVar(uiTournInfoPlayerListArrayNum));
                while (pTournInfoTeam1Account != nullptr && pTournInfoTeam1Player != nullptr)
                {
                    tsvector vParams;
                    vParams.emplace_back(pTournInfoTeam1Account->GetString());
                    vParams.emplace_back(pTournInfoTeam1Player->GetString());
                    
                    GameClient.GetInterfaceManager()->Trigger(UITRIGGER_TOURNAMENT_INFO_TEAM1_PLAYERS_RETURN, vParams);
                    
                    pTournInfoTeam1Account = pTournInfoTeam1AccountList->GetVar(++uiTournInfoPlayerListArrayNum);
                    pTournInfoTeam1Player = pTournInfoTeam1PlayerList->GetVar(uiTournInfoPlayerListArrayNum);
                }                               
            }               
            
            const CPHPData *pTournInfoTeam2AccountList(pTournInfo->GetVar(_T("team2_account_list")));
            const CPHPData *pTournInfoTeam2PlayerList(pTournInfo->GetVar(_T("team2_name_list")));
            if (pTournInfoTeam2AccountList != nullptr && pTournInfoTeam2PlayerList != nullptr)
            {               
                uint uiTournInfoPlayerListArrayNum(0);
                const CPHPData *pTournInfoTeam2Account(pTournInfoTeam2AccountList->GetVar(uiTournInfoPlayerListArrayNum));
                const CPHPData *pTournInfoTeam2Player(pTournInfoTeam2PlayerList->GetVar(uiTournInfoPlayerListArrayNum));
                while (pTournInfoTeam2Account != nullptr && pTournInfoTeam2Player != nullptr)
                {
                    tsvector vParams;
                    vParams.emplace_back(pTournInfoTeam2Account->GetString());
                    vParams.emplace_back(pTournInfoTeam2Player->GetString());
                    
                    GameClient.GetInterfaceManager()->Trigger(UITRIGGER_TOURNAMENT_INFO_TEAM2_PLAYERS_RETURN, vParams);
                    
                    pTournInfoTeam2Account = pTournInfoTeam2AccountList->GetVar(++uiTournInfoPlayerListArrayNum);
                    pTournInfoTeam2Player = pTournInfoTeam2PlayerList->GetVar(uiTournInfoPlayerListArrayNum);
                }                               
            }               
            
            GameClient.GetInterfaceManager()->Trigger(UITRIGGER_TOURNAMENT_INFO_RETURN, vParams);
            break;
        }

    case GET_TOURNAMENTS_FOR_ACCOUNT:
        {
            const CPHPData *phpTournsForAccount(phpResponse.GetVar(_T("tourns_for_account")));

            if (phpTournsForAccount == nullptr)
            {
                Console.Warn << _T("RequestTournamentData: No tournaments for selected account.") << newl;
                return;
            }

            uint uiTournListArrayNum(0);
            const CPHPData *pTourn(phpTournsForAccount->GetVar(uiTournListArrayNum));
            while (pTourn != nullptr)
            {
                tsvector vParams;
                vParams.emplace_back(pTourn->GetString(_T("tourn_id")));
                vParams.emplace_back(pTourn->GetString(_T("tourn_name")));
                vParams.emplace_back(pTourn->GetString(_T("tourn_num")));

                GameClient.GetInterfaceManager()->Trigger(UITRIGGER_TOURNAMENTS_FOR_ACCOUNT_RETURN, vParams);

                pTourn = phpTournsForAccount->GetVar(++uiTournListArrayNum);
            }
            break;
        }
    }
}


/*====================
  CGameInterfaceManager::ProcessRecentMatchesRequest
  ====================*/
void    CGameInterfaceManager::ProcessRecentMatchesRequest()
{
    if (m_pRecentMatchesRequest == nullptr || m_pRecentMatchesRequest->IsActive())
        return;

    Console << _T("Processing recent matches request") << newl;

    ClearMatchInfo();

    if (m_vLastGameStatsSummary.size() > 1 && AtoI(m_vLastGameStatsSummary[1]) != 0 && AtoI(m_vLastGameStatsSummary[1]) != -1)
        Trigger(UITRIGGER_MATCH_INFO_ENTRY, m_vLastGameStatsSummary[1]);

    if (!m_pRecentMatchesRequest->WasSuccessful())
    {
        Console.Warn << _T("RequestRecentMatches: Failed request") << newl;
        Host.GetHTTPManager()->ReleaseRequest(m_pRecentMatchesRequest);
        m_pRecentMatchesRequest = nullptr;
        Trigger(UITRIGGER_MATCH_INFO_ENTRY_FINISHED, TSNULL);
        return;
    }

    const CPHPData phpResponse(m_pRecentMatchesRequest->GetResponse());

    Host.GetHTTPManager()->ReleaseRequest(m_pRecentMatchesRequest);
    m_pRecentMatchesRequest = nullptr;

    if (!phpResponse.IsValid())
    {
        Console.Warn << _T("RequestRecentMatches: Bad data") << newl;
        Trigger(UITRIGGER_MATCH_INFO_ENTRY_FINISHED, TSNULL);
        return;
    }

    if (!phpResponse.GetBool(_T("0")))
    {
        Console.Warn << _T("RequestRecentMatches: Server error") << newl;
        Trigger(UITRIGGER_MATCH_INFO_ENTRY_FINISHED, TSNULL);
        return;
    }

    const CPHPData *phpMatches(phpResponse.GetVar(_T("last_stats")));
    if (phpMatches == nullptr)
    {
        Console.Warn << _T("RequestRecentMatches: No match data") << newl;
        Trigger(UITRIGGER_MATCH_INFO_ENTRY_FINISHED, TSNULL);
        return;
    }

    Trigger(UITRIGGER_MATCH_INFO_ENTRY, TSNULL);

    for (uint i(0); i < phpMatches->GetSize(); i++)
    {
        if (m_vLastGameStatsSummary.size() > 1 && AtoI(phpMatches->GetKeyName(i)) == AtoI(m_vLastGameStatsSummary[1]))
            continue;

        Trigger(UITRIGGER_MATCH_INFO_ENTRY, phpMatches->GetKeyName(i));
    }

    Trigger(UITRIGGER_MATCH_INFO_ENTRY_FINISHED, TSNULL);

    if (m_uiQueuedMatchInfoRequest != INVALID_INDEX)
    {
        RequestMatchInfo(m_uiQueuedMatchInfoRequest);
    }
}


/*====================
  CGameInterfaceManager::ToggleShopInterface
  ====================*/
void    CGameInterfaceManager::ToggleShopInterface()
{
    if (m_bLockShop)
        return;

    m_bDisplayShop = !m_bDisplayShop;

#if 1
    GameClient.SetActiveShop(TSNULL);

    if (m_bDisplayShop)
        GameClient.GetClientCommander()->SetDefaultActiveShop();
#else
    if (m_bDisplayShop && GameClient.GetActiveShop().empty())
        GameClient.GetClientCommander()->SetDefaultActiveShop();
#endif
}


/*====================
  CGameInterfaceManager::SetShopVisible
  ====================*/
void    CGameInterfaceManager::SetShopVisible(bool b, bool bForce)
{
    if (m_bLockShop && !bForce)
        return;

    m_bDisplayShop = b;

#if 1
    GameClient.SetActiveShop(TSNULL);

    if (m_bDisplayShop)
        GameClient.GetClientCommander()->SetDefaultActiveShop();
#else
    if (m_bDisplayShop && GameClient.GetActiveShop().empty())
        GameClient.GetClientCommander()->SetDefaultActiveShop();
#endif
}


/*====================
  CGameInterfaceManager::SetReplayInfo
  ====================*/
void    CGameInterfaceManager::SetReplayInfo(const tstring &sReplay)
{
    m_cReplayInfo = CReplayInfo();

    CArchive cArchive(sReplay, ARCHIVE_READ);
    if (!cArchive.IsOpen())
        return;

    CFileHandle hReplayInfo(_T("ReplayInfo"), FILE_READ, cArchive);
    if (!hReplayInfo.IsOpen())
        return;

    XMLManager.Process(hReplayInfo, _T("replayinfo"), &m_cReplayInfo);
}


/*====================
  CGameInterfaceManager::RefreshReplayList
  ====================*/
void    CGameInterfaceManager::RefreshReplayList()
{
    tsvector vFileList;
    FileManager.GetFileList(_T("~/replays/"), _T("*.honreplay"), false, vFileList, true);
    FileManager.GetFileList(_T("/replays/"), _T("*.honreplay"), false, vFileList, true);

    for (tsvector_it it(vFileList.begin()); it != vFileList.end(); ++it)
        ReplayList.Trigger(*it);
}


/*====================
  CGameInterfaceManager::RegisterEntityDefinitions
  ====================*/
void    CGameInterfaceManager::RegisterEntityDefinitions()
{
    if (m_bEntitiesLoaded)
    {
        Trigger(UITRIGGER_ENTITY_DEFINITIONS_LOADED, TSNULL);
        return;
    }

    // Load stringtables before registering entities
    GameClient.LoadStringTables();

    // Make sure entity definitions are loaded
    GameClient.RegisterGameMechanics(_T("/base.gamemechanics"));
    if (!GameClient.FetchGameMechanics())
        Console.Err << _T("Missing game mechanics!") << newl;

    // Add dynamic entity definitions to the load queue
    m_vLoadQueue.clear();
    FileManager.GetFileList(_T("/"), _T("*.entity"), true, m_vLoadQueue);
}


/*====================
  CGameInterfaceManager::UpdateHeroCompendium
  ====================*/
void    CGameInterfaceManager::UpdateHeroCompendium()
{
    PROFILE("CGameInterfaceManager::UpdateHeroCompendium");

    if (!m_bEntitiesLoaded)
        return;

    vector<ushort> vHeroes;

    g_EntityRegistry.GetHeroList(_T("Legion"), vHeroes);
    g_EntityRegistry.GetHeroList(_T("Hellbourne"), vHeroes);

    Trigger(UITRIGGER_COMPENDIUM_CLEAR_INFO, TSNULL);

    tsvector vParams(37 + MAX_SHOP_ITEMS);
    Trigger(UITRIGGER_COMPENDIUM_DETAILED_HERO_INFO, vParams);

    vector<tsvector> vHeroDefinitions;

    for (vector<ushort>::iterator it(vHeroes.begin()); it != vHeroes.end(); it++)
    {
        CHeroDefinition *pHero(g_EntityRegistry.GetDefinition<CHeroDefinition>(*it));

        if (pHero == nullptr)
            continue;

        tsvector vDefinition(29);
        EAttribute ePrimary(pHero->GetPrimaryAttribute());
        tstring sAttrib;

        float fAttributeDamageBonus(0.0f);

        switch (ePrimary)
        {
        case ATTRIBUTE_STRENGTH:
            sAttrib = _T("strength");
            fAttributeDamageBonus = pHero->GetStrength();
            break;

        case ATTRIBUTE_AGILITY:
            sAttrib = _T("agility");
            fAttributeDamageBonus = pHero->GetAgility();
            break;

        case ATTRIBUTE_INTELLIGENCE:
            sAttrib = _T("intelligence");
            fAttributeDamageBonus = pHero->GetIntelligence();
            break;
        }

        vDefinition[0] = pHero->GetName();
        vDefinition[1] = pHero->GetDisplayName();
        vDefinition[2] = GameClient.GetGameMessage(_T("attribute_") + sAttrib);
        vDefinition[3] = XtoA(pHero->GetStrength(), 0, 0, 0);
        vDefinition[4] = XtoA(pHero->GetStrengthPerLevel(), 0, 0, 1);
        vDefinition[5] = XtoA(pHero->GetAgility(), 0, 0, 0);
        vDefinition[6] = XtoA(pHero->GetAgilityPerLevel(), 0, 0, 1);
        vDefinition[7] = XtoA(pHero->GetIntelligence(), 0, 0, 0);
        vDefinition[8] = XtoA(pHero->GetIntelligencePerLevel(), 0, 0, 1);
        vDefinition[9] = XtoA(IHeroEntity::AdjustArmor(pHero->GetArmor(0), pHero->GetAgility()), 0, 0, 2);
        vDefinition[10] = XtoA(pHero->GetMagicArmor(0), 0, 0, 2);
        vDefinition[11] = XtoA(pHero->GetMoveSpeed(0), 0, 0, 0);
        vDefinition[12] = XtoA(pHero->GetAttackDamageMin(0) + fAttributeDamageBonus, 0, 0, 0);
        vDefinition[13] = XtoA(pHero->GetAttackDamageMax(0) + fAttributeDamageBonus, 0, 0, 0);
        vDefinition[14] = pHero->GetIconPath(0);

        CAbilityDefinition *pAbility(g_EntityRegistry.GetDefinition<CAbilityDefinition>(pHero->GetInventory0(0)));
        if (pAbility != nullptr)
            vDefinition[15] = pAbility->GetIconPath(0);

        pAbility = g_EntityRegistry.GetDefinition<CAbilityDefinition>(pHero->GetInventory1(0));
        if (pAbility != nullptr)
            vDefinition[16] = pAbility->GetIconPath(0);

        pAbility = g_EntityRegistry.GetDefinition<CAbilityDefinition>(pHero->GetInventory2(0));
        if (pAbility != nullptr)
            vDefinition[17] = pAbility->GetIconPath(0);

        pAbility = g_EntityRegistry.GetDefinition<CAbilityDefinition>(pHero->GetInventory3(0));
        if (pAbility != nullptr)
            vDefinition[18] = pAbility->GetIconPath(0);

        vDefinition[19] = _T("/ui/common/primary_") + sAttrib + _T(".tga");
        vDefinition[20] = XtoA(Game.GetArmorDamageAdjustment(pHero->GetArmorType(0), IHeroEntity::AdjustArmor(pHero->GetArmor(0), pHero->GetAgility())) * 100, 0, 0, 1);
        vDefinition[21] = XtoA(Game.GetArmorDamageAdjustment(pHero->GetMagicArmorType(0), pHero->GetMagicArmor(0)) * 100, 0, 0, 1);

        float fAvgDamage = (pHero->GetAttackDamageMin(0) + pHero->GetAttackDamageMax(0)) / 2.0f;
        float fAtksPerSecond = pHero->GetAttackCooldown(0) != 0 ? (1.0f / MsToSec(pHero->GetAttackCooldown(0))) : 1.0f;
        float fDPS = fAvgDamage / fAtksPerSecond;

        vDefinition[22] = XtoA(fDPS, 0, 0, 2);
        vDefinition[23] = XtoA(pHero->GetAttackRange(0));
        vDefinition[24] = XtoA(pHero->GetAttackType(0));        
        vDefinition[25] = XtoA(pHero->GetTeam());       
        vDefinition[26] = XtoA(AddEscapeChars(pHero->GetDescription()));
        vDefinition[27] = XtoA(pHero->GetAttackDamageMin(0));
        vDefinition[28] = XtoA(pHero->GetAttackDamageMax(0));
        

        vHeroDefinitions.emplace_back(vDefinition);
    }

    if (g_uiHeroSortValue < 25)
    {
        g_iSortValue = g_uiHeroSortValue;

        if (g_bHeroSortByValue)
        {
            if (g_bHeroSortDesc)
                sort(vHeroDefinitions.begin(), vHeroDefinitions.end(), CompareStatsValueDesc);
            else
                sort(vHeroDefinitions.begin(), vHeroDefinitions.end(), CompareStatsValueAsc);
        }
        else
        {
            if (g_bHeroSortDesc)
                sort(vHeroDefinitions.begin(), vHeroDefinitions.end(), CompareStatsDesc);
            else
                sort(vHeroDefinitions.begin(), vHeroDefinitions.end(), CompareStatsAsc);
        }
    }

    for (vector<tsvector>::iterator it(vHeroDefinitions.begin()); it != vHeroDefinitions.end(); it++)
        Trigger(UITRIGGER_COMPENDIUM_HERO_INFO, *it);
}


/*====================
  CGameInterfaceManager::ShowHeroCompendiumInfo
  ====================*/
void    CGameInterfaceManager::ShowHeroCompendiumInfo(const tstring &sHero)
{
    tsvector vParams(37 + MAX_SHOP_ITEMS);
    Trigger(UITRIGGER_COMPENDIUM_DETAILED_HERO_INFO, vParams);

    CHeroDefinition *pHero(g_EntityRegistry.GetDefinition<CHeroDefinition>(sHero));

    if (pHero == nullptr)
        return;

    EAttribute ePrimary(pHero->GetPrimaryAttribute());
    tstring sAttrib;
    float fAttributeDamageBonus(0.0f);

    switch (ePrimary)
    {
    case ATTRIBUTE_STRENGTH:
        sAttrib = _T("strength");
        fAttributeDamageBonus = pHero->GetStrength();
        break;

    case ATTRIBUTE_AGILITY:
        sAttrib = _T("agility");
        fAttributeDamageBonus = pHero->GetAgility();
        break;

    case ATTRIBUTE_INTELLIGENCE:
        sAttrib = _T("intelligence");
        fAttributeDamageBonus = pHero->GetIntelligence();
        break;
    }

    vParams[0] = sHero;
    vParams[1] = pHero->GetDisplayName();
    vParams[2] = pHero->GetIconPath(0);
    vParams[3] = XtoA(pHero->GetAttackDamageMin(0) + fAttributeDamageBonus, 0, 0, 0);
    vParams[4] = XtoA(pHero->GetAttackDamageMax(0) + fAttributeDamageBonus, 0, 0, 0);
    vParams[5] = XtoA(pHero->GetAttackRange(0), 0, 0, 0);
    vParams[6] = pHero->GetAttackCooldown(0) != 0 ? XtoA(1.0f / MsToSec(pHero->GetAttackCooldown(0)), 0, 0, 2) : _T("0.00");
    vParams[7] = XtoA(IHeroEntity::AdjustMaxHealth(pHero->GetMaxHealth(0), pHero->GetStrength()), 0, 0, 0);
    vParams[8] = XtoA(IHeroEntity::AdjustMaxMana(pHero->GetMaxMana(0), pHero->GetIntelligence()), 0, 0, 0);
    vParams[9] = pHero->GetDescription();
    
    CAbilityDefinition *pAbility(g_EntityRegistry.GetDefinition<CAbilityDefinition>(pHero->GetInventory0(0)));
    if (pAbility != nullptr)
    {
        vParams[10] = pAbility->GetIconPath(0);
        vParams[11] = pAbility->GetDisplayName();
        vParams[12] = pAbility->GetDescription();
    }

    pAbility = g_EntityRegistry.GetDefinition<CAbilityDefinition>(pHero->GetInventory1(0));
    if (pAbility != nullptr)
    {
        vParams[13] = pAbility->GetIconPath(0);
        vParams[14] = pAbility->GetDisplayName();
        vParams[15] = pAbility->GetDescription();
    }

    pAbility = g_EntityRegistry.GetDefinition<CAbilityDefinition>(pHero->GetInventory2(0));
    if (pAbility != nullptr)
    {
        vParams[16] = pAbility->GetIconPath(0);
        vParams[17] = pAbility->GetDisplayName();
        vParams[18] = pAbility->GetDescription();
    }

    pAbility = g_EntityRegistry.GetDefinition<CAbilityDefinition>(pHero->GetInventory3(0));
    if (pAbility != nullptr)
    {
        vParams[19] = pAbility->GetIconPath(0);
        vParams[20] = pAbility->GetDisplayName();
        vParams[21] = pAbility->GetDescription();
    }
    
    vParams[22] = XtoA(pHero->GetMoveSpeed(0));  // Move speed
    vParams[23] = XtoA(IHeroEntity::AdjustArmor(pHero->GetArmor(0), pHero->GetAgility())); // Armor (factors in agi)
    vParams[24] = XtoA(pHero->GetMagicArmor(0)); // Magic armor
    vParams[25] = XtoA(Game.GetArmorDamageAdjustment(pHero->GetArmorType(0), IHeroEntity::AdjustArmor(pHero->GetArmor(0), pHero->GetAgility())) * 100); // Armor damage reduction percentage (factors in agi)
    vParams[26] = XtoA(Game.GetArmorDamageAdjustment(pHero->GetMagicArmorType(0), pHero->GetMagicArmor(0)) * 100); // Magic armor damage reduction percentage
    vParams[27] = XtoA(pHero->GetStrength());
    vParams[28] = XtoA(pHero->GetStrengthPerLevel());
    vParams[29] = XtoA(pHero->GetAgility());
    vParams[30] = XtoA(pHero->GetAgilityPerLevel());
    vParams[31] = XtoA(pHero->GetIntelligence());
    vParams[32] = XtoA(pHero->GetIntelligencePerLevel());
    vParams[33] = XtoA(pHero->GetAttackRange(0));

    for (int i(0); i < MAX_SHOP_ITEMS; i++)
    {
        CItemDefinition *pItem(g_EntityRegistry.GetDefinition<CItemDefinition>(pHero->GetRecommendedItem(i)));

        if (pItem == nullptr)
            continue;

        vParams[34 + i] = pItem->GetIconPath(0);
    }
    
    vParams[54] = pHero->GetAnnouncerSoundPath();
    vParams[55] = Game.GetAttackTypeDisplayName(pHero->GetAttackType(0));
    vParams[56] = sAttrib;

    Trigger(UITRIGGER_COMPENDIUM_DETAILED_HERO_INFO, vParams);
}


/*====================
  CGameInterfaceManager::SetPreviewMap
  ====================*/
void    CGameInterfaceManager::SetPreviewMap(const tstring &sMap)
{
    CArchive cMapArchive(_T("/maps/") + sMap + _T(".s2z"), ARCHIVE_READ);
    CWorld cWorld(WORLDHOST_NULL);

    // Load the main config file
    CFileHandle hWorldConfig(_T("WorldConfig"), FILE_READ, cMapArchive);
    if (!hWorldConfig.IsOpen())
        EX_ERROR(_T("World has no config file"));
    XMLManager.Process(hWorldConfig, _T("world"), &cWorld);

    m_sPreviewMapName = cWorld.GetFancyName();
    m_iPreviewMapSize = cWorld.GetMaxPlayers();

    cWorld.Free();
    cMapArchive.Close();
}


/*====================
  CGameInterfaceManager::StoreEndGameStats
  ====================*/
void    CGameInterfaceManager::StoreEndGameStats()
{
    m_vLastGameStatsSummary.clear();
    m_vLastGameStatsSummary.resize(32);

    for (int i(0); i < MAX_DISPLAY_TEAMS; i++)
    {
        for (int x(0); x < MAX_DISPLAY_PLAYERSPERTEAM; x++)
        {
            m_vLastGameStatsPlayers[i][x].clear();
            m_vLastGameStatsPlayers[i][x].resize(57);
        }
    }

    CGameInfo *pGameInfo(GameClient.GetGameInfo());
    if (pGameInfo == nullptr)
        return;

    m_vLastGameStatsSummary[1] = XtoA(pGameInfo->GetMatchID());
    m_vLastGameStatsSummary[2] = pGameInfo->GetServerDate();
    m_vLastGameStatsSummary[3] = pGameInfo->GetServerTime();
    m_vLastGameStatsSummary[4] = pGameInfo->GetServerName();
    m_vLastGameStatsSummary[5] = XtoA(pGameInfo->GetMatchLength());
    m_vLastGameStatsSummary[17] = XtoA(GameClient.GetWinningTeam());
    m_vLastGameStatsSummary[18] = K2_Version(FileManager.GetCompatVersion());
    m_vLastGameStatsSummary[20] = XtoA(0);
    m_vLastGameStatsSummary[22] = _T("false");
    m_vLastGameStatsSummary[24] = _T("true");
    m_vLastGameStatsSummary[25] = _T("true");
}


/*====================
  CGameInterfaceManager::ClearEndGameStats()
  ====================*/
void    CGameInterfaceManager::ClearEndGameStats()
{
    m_vLastGameStatsSummary.clear();
    m_vLastGameStatsSummary.resize(32);
}


/*====================
  CGameInterfaceManager::StoreEndGamePlayerStats
  ====================*/
void    CGameInterfaceManager::StoreEndGamePlayerStats(CPlayer *pPlayer)
{
    if (pPlayer == nullptr)
        return;

    if (pPlayer->GetTeam() < TEAM_1 || pPlayer->GetTeam() > TEAM_2)
        return;

    CTeamInfo *pTeam(GameClient.GetTeam(pPlayer->GetTeam()));

    if (pTeam == nullptr)
        return;
    
    int iTeam = CLAMP((int)(pPlayer->GetTeam() - 1), 0, MAX_DISPLAY_TEAMS);
    int iIndex = CLAMP((int)pTeam->GetTeamIndexFromClientID(pPlayer->GetClientNumber()), 0, MAX_DISPLAY_PLAYERSPERTEAM);

    IHeroEntity *pHero(pPlayer->GetHero());
    if (pHero == nullptr)
        return;

    CGameStats *pStats(pPlayer->GetStats());
    if (pStats == nullptr)
        return;

    CGameInfo *pGameInfo(GameClient.GetGameInfo());
    if (pGameInfo == nullptr)
        return;

    float fTime = pGameInfo->GetMatchLength() / 60000.0f;

    m_vLastGameStatsPlayers[iTeam][iIndex][0] = pPlayer->GetName();
    m_vLastGameStatsPlayers[iTeam][iIndex][1] = XtoA(pPlayer->GetTeam());
    m_vLastGameStatsPlayers[iTeam][iIndex][2] = XtoA(pPlayer->GetPlayerIndex());
    m_vLastGameStatsPlayers[iTeam][iIndex][3] = XtoA(pHero->GetLevel());
    m_vLastGameStatsPlayers[iTeam][iIndex][4] = XtoA(pStats->GetHeroKills());
    m_vLastGameStatsPlayers[iTeam][iIndex][17] = XtoA(pStats->GetDeaths());
    m_vLastGameStatsPlayers[iTeam][iIndex][18] = XtoA(pStats->GetHeroAssists());
    m_vLastGameStatsPlayers[iTeam][iIndex][5] = XtoA(pStats->GetCreepKills());
    m_vLastGameStatsPlayers[iTeam][iIndex][6] = fTime != 0.0f ? XtoA(pStats->GetExperience() / fTime, 0, 0, 1) : _T("0");
    m_vLastGameStatsPlayers[iTeam][iIndex][7] = fTime != 0.0f ? XtoA(pStats->GetGoldEarned() / fTime, 0, 0, 1) : _T("0");
    m_vLastGameStatsPlayers[iTeam][iIndex][8] = fTime != 0.0f ? XtoA(pStats->GetActionCount() / fTime, 0, 0, 1) : _T("0");
    m_vLastGameStatsPlayers[iTeam][iIndex][9] = XtoA(pStats->GetDenies());
    m_vLastGameStatsPlayers[iTeam][iIndex][10] = pHero->GetIconPath();

    for (int i(INVENTORY_START_BACKPACK); i <= INVENTORY_END_BACKPACK; i++)
    {
        if (pHero->GetItem(i) == nullptr)
            continue;

        m_vLastGameStatsPlayers[iTeam][iIndex][11 + (i - INVENTORY_START_BACKPACK)] = pHero->GetItem(i)->GetIconPath();
    }
}


/*====================
  CGameInterfaceManager::SortStatsAsc
  ====================*/
void    CGameInterfaceManager::SortStatsAsc(int iParam, int iTeam)
{
    tsvector vPlayerStats(57);

    vPlayerStats[1] = XtoA(iTeam);

    for (uint i(0); i < MAX_DISPLAY_PLAYERSPERTEAM; i++)
        GameClient.GetInterfaceManager()->Trigger(UITRIGGER_MATCH_INFO_PLAYER, vPlayerStats, i);

    g_iSortValue = iParam;

    sort(m_vCurrentGameStatsPlayers[iTeam - 1].begin(), m_vCurrentGameStatsPlayers[iTeam - 1].end(), CompareStatsAsc);

    for (uint x(0); x < MAX_DISPLAY_PLAYERSPERTEAM; x++)
    {
        if (m_vCurrentGameStatsPlayers[iTeam - 1][x].size() == 0)
            continue;

        Trigger(UITRIGGER_MATCH_INFO_PLAYER, m_vCurrentGameStatsPlayers[iTeam - 1][x], x);
    }
}


/*====================
  CGameInterfaceManager::SortStatsDesc
  ====================*/
void    CGameInterfaceManager::SortStatsDesc(int iParam, int iTeam)
{
    tsvector vPlayerStats(57);

    vPlayerStats[1] = XtoA(iTeam);

    for (uint i(0); i < MAX_DISPLAY_PLAYERSPERTEAM; i++)
        GameClient.GetInterfaceManager()->Trigger(UITRIGGER_MATCH_INFO_PLAYER, vPlayerStats, i);

    g_iSortValue = iParam;

    sort(m_vCurrentGameStatsPlayers[iTeam - 1].begin(), m_vCurrentGameStatsPlayers[iTeam - 1].end(), CompareStatsDesc);

    for (uint x(0); x < MAX_DISPLAY_PLAYERSPERTEAM; x++)
    {
        if (m_vCurrentGameStatsPlayers[iTeam - 1][x].size() == 0)
            continue;

        Trigger(UITRIGGER_MATCH_INFO_PLAYER, m_vCurrentGameStatsPlayers[iTeam - 1][x], x);
    }
}


/*====================
  CGameInterfaceManager::SortStatsByValueAsc
  ====================*/
void    CGameInterfaceManager::SortStatsByValueAsc(int iParam, int iTeam)
{
    tsvector vPlayerStats(57);

    vPlayerStats[1] = XtoA(iTeam);

    for (uint i(0); i < MAX_DISPLAY_PLAYERSPERTEAM; i++)
        GameClient.GetInterfaceManager()->Trigger(UITRIGGER_MATCH_INFO_PLAYER, vPlayerStats, i);

    g_iSortValue = iParam;

    sort(m_vCurrentGameStatsPlayers[iTeam - 1].begin(), m_vCurrentGameStatsPlayers[iTeam - 1].end(), CompareStatsValueAsc);

    for (uint x(0); x < MAX_DISPLAY_PLAYERSPERTEAM; x++)
    {
        if (m_vCurrentGameStatsPlayers[iTeam - 1][x].size() == 0)
            continue;

        Trigger(UITRIGGER_MATCH_INFO_PLAYER, m_vCurrentGameStatsPlayers[iTeam - 1][x], x);
    }
}


/*====================
  CGameInterfaceManager::SortStatsByValueDesc
  ====================*/
void    CGameInterfaceManager::SortStatsByValueDesc(int iParam, int iTeam)
{
    tsvector vPlayerStats(57);

    vPlayerStats[1] = XtoA(iTeam);

    for (uint i(0); i < MAX_DISPLAY_PLAYERSPERTEAM; i++)
        GameClient.GetInterfaceManager()->Trigger(UITRIGGER_MATCH_INFO_PLAYER, vPlayerStats, i);

    g_iSortValue = iParam;

    sort(m_vCurrentGameStatsPlayers[iTeam - 1].begin(), m_vCurrentGameStatsPlayers[iTeam - 1].end(), CompareStatsValueDesc);

    for (uint x(0); x < MAX_DISPLAY_PLAYERSPERTEAM; x++)
    {
        if (m_vCurrentGameStatsPlayers[iTeam - 1][x].size() == 0)
            continue;

        Trigger(UITRIGGER_MATCH_INFO_PLAYER, m_vCurrentGameStatsPlayers[iTeam - 1][x], x);
    }
}


/*====================
  CGameInterfaceManager::LoadEntityDefinition
  ====================*/
void    CGameInterfaceManager::LoadEntityDefinition(const tstring &sPath)
{
    ResHandle hDefinition(g_ResourceManager.Register(sPath, RES_ENTITY_DEF));
    CEntityDefinitionResource *pDefRes(g_ResourceManager.Get<CEntityDefinitionResource>(hDefinition));
    if (pDefRes != nullptr)
        pDefRes->PostProcess();
}


/*====================
  CGameInterfaceManager::GetModifiedHeroDefinitionFromPath
  ====================*/
CHeroDefinition*    CGameInterfaceManager::GetModifiedHeroDefinitionFromPath(const tstring &sPath, const tstring &sAvatar)
{
    LoadEntityDefinition(sPath); // TODO: Probably require the interface to call this

    ResHandle hDefinition(g_ResourceManager.LookUpPath(sPath));
    CEntityDefinitionResource *pDefRes(g_ResourceManager.Get<CEntityDefinitionResource>(hDefinition));
    if (pDefRes != nullptr)
    {
        CHeroDefinition *pDefinition(pDefRes->GetDefinition<CHeroDefinition>());
        if (pDefinition != nullptr)
        {
            if (pDefinition != nullptr && !sAvatar.empty())
            {
                uint uiModifier(EntityRegistry.LookupModifierKey(sAvatar));
                uint uiModifierBit(pDefinition->GetModifierBit(uiModifier));

                pDefinition = static_cast<CHeroDefinition*>(pDefinition->GetModifiedDefinition(uiModifierBit));
            }

            return pDefinition;
        }
        else
            return nullptr;
    }
    else
        return nullptr;
}


/*====================
  CGameInterfaceManager::GetModifiedHeroDefinition
  ====================*/
CHeroDefinition*    CGameInterfaceManager::GetModifiedHeroDefinition(const tstring &sHeroName, const tstring &sAvatar)
{
    CHeroDefinition *pDefinition(EntityRegistry.GetDefinition<CHeroDefinition>(sHeroName));
    if (pDefinition != nullptr)
    {
        if (pDefinition != nullptr && !sAvatar.empty())
        {
            uint uiModifier(EntityRegistry.LookupModifierKey(sAvatar));
            uint uiModifierBit(pDefinition->GetModifierBit(uiModifier));

            pDefinition = static_cast<CHeroDefinition*>(pDefinition->GetModifiedDefinition(uiModifierBit));
        }

        return pDefinition;
    }
    else
        return nullptr;
}


/*====================
  CGameInterfaceManager::GetHeroIconPathFromHeroDefinition
  ====================*/
const tstring&  CGameInterfaceManager::GetHeroIconPathFromHeroDefinition(const tstring &sPath, const tstring &sAvatar)
{
    CHeroDefinition *pDefinition(GetModifiedHeroDefinitionFromPath(sPath, sAvatar));
    if (pDefinition != nullptr)
        return pDefinition->GetIconPath(0);
    else
        return TSNULL;
}


/*====================
  CGameInterfaceManager::GetHeroIconPathFromProduct
  ====================*/
const tstring&  CGameInterfaceManager::GetHeroIconPathFromProduct(const tstring &sProduct)
{
    const tstring &sHeroName(sProduct.substr(0, sProduct.find_first_of(_T('.'))));
    const tstring &sAvatar(sProduct.substr(sProduct.find_first_of(_T('.')) + 1));

    CHeroDefinition *pDefinition(GetModifiedHeroDefinition(sHeroName, sAvatar));
    if (pDefinition != nullptr)
        return pDefinition->GetIconPath(0);
    else
        return TSNULL;
}


/*====================
  CGameInterfaceManager::GetHeroPreviewModelPathFromProduct
  ====================*/
const tstring&  CGameInterfaceManager::GetHeroPreviewModelPathFromProduct(const tstring &sProduct)
{
    const tstring &sHeroName(sProduct.substr(0, sProduct.find_first_of(_T('.'))));
    const tstring &sAvatar(sProduct.substr(sProduct.find_first_of(_T('.')) + 1));

    CHeroDefinition *pDefinition(GetModifiedHeroDefinition(sHeroName, sAvatar));
    if (pDefinition != nullptr)
        return pDefinition->GetPreviewModelPath();
    else
        return TSNULL;
}


/*====================
  CGameInterfaceManager::GetHeroPassiveEffectPathFromProduct
  ====================*/
const tstring&  CGameInterfaceManager::GetHeroPassiveEffectPathFromProduct(const tstring &sProduct)
{
    const tstring &sHeroName(sProduct.substr(0, sProduct.find_first_of(_T('.'))));
    const tstring &sAvatar(sProduct.substr(sProduct.find_first_of(_T('.')) + 1));

    CHeroDefinition *pDefinition(GetModifiedHeroDefinition(sHeroName, sAvatar));
    if (pDefinition != nullptr)
        return pDefinition->GetPassiveEffectPath(0);
    else
        return TSNULL;
}


/*====================
  CGameInterfaceManager::GetHeroPreviewPosFromProduct
  ====================*/
CVec3f  CGameInterfaceManager::GetHeroPreviewPosFromProduct(const tstring &sProduct)
{
    const tstring &sHeroName(sProduct.substr(0, sProduct.find_first_of(_T('.'))));
    const tstring &sAvatar(sProduct.substr(sProduct.find_first_of(_T('.')) + 1));

    CHeroDefinition *pDefinition(GetModifiedHeroDefinition(sHeroName, sAvatar));
    if (pDefinition != nullptr)
        return pDefinition->GetPreviewPos();
    else
        return V3_ZERO;
}


/*====================
  CGameInterfaceManager::GetHeroPreviewAnglesFromProduct
  ====================*/
CVec3f  CGameInterfaceManager::GetHeroPreviewAnglesFromProduct(const tstring &sProduct)
{
    const tstring &sHeroName(sProduct.substr(0, sProduct.find_first_of(_T('.'))));
    const tstring &sAvatar(sProduct.substr(sProduct.find_first_of(_T('.')) + 1));

    CHeroDefinition *pDefinition(GetModifiedHeroDefinition(sHeroName, sAvatar));
    if (pDefinition != nullptr)
        return pDefinition->GetPreviewAngles();
    else
        return V3_ZERO;
}


/*====================
  CGameInterfaceManager::GetHeroPreviewScaleFromProduct
  ====================*/
float   CGameInterfaceManager::GetHeroPreviewScaleFromProduct(const tstring &sProduct)
{
    const tstring &sHeroName(sProduct.substr(0, sProduct.find_first_of(_T('.'))));
    const tstring &sAvatar(sProduct.substr(sProduct.find_first_of(_T('.')) + 1));

    CHeroDefinition *pDefinition(GetModifiedHeroDefinition(sHeroName, sAvatar));
    if (pDefinition != nullptr)
        return pDefinition->GetPreviewScale();
    else
        return 0.0f;
}


/*====================
  CGameInterfaceManager::IsReplayCompatible
  ====================*/
bool    CGameInterfaceManager::IsReplayCompatible()
{
    return FileManager.IsCompatVersionSupported(m_cReplayInfo.GetGameInfo().GetProperty(_CTS("version")));
}


/*====================
  CGameInterfaceManager::GetReplayVersion
  ====================*/
tstring CGameInterfaceManager::GetReplayVersion()
{
    return m_cReplayInfo.GetGameInfo().GetProperty(_CTS("version"));
}


/*--------------------
  MultiSelectHoverEntity
  --------------------*/
UI_VOID_CMD(MultiSelectHoverEntity, 1)
{
    uint uiTarget(AtoI(vArgList[0]->Evaluate()));

    IUnitEntity *pEntity(GameClient.GetUnitEntity(uiTarget));
    if (pEntity == nullptr || !pEntity->GetIsSelectable())
        uiTarget = INVALID_INDEX;
}


static void     GetAbilityInfo(CAbilityDefinition *pAbility, int iSlot)
{
    static tsvector vHeroAbilityInfo(7);
    if (pAbility != nullptr)
    {
        uint uiLevel(0);
        uint uiMaxLevel(pAbility->GetMaxLevel());

        vHeroAbilityInfo[0] = _T("true");
        vHeroAbilityInfo[1] = pAbility->GetIconPath(0);
        vHeroAbilityInfo[2] = pAbility->GetDisplayName();

        CGameInterfaceManager::BuildMultiLevelText(pAbility->GetDescription(), 0, MAX(1u, uiMaxLevel) - 1, vHeroAbilityInfo[3]);

        vHeroAbilityInfo[4] = pAbility->GetActionType();

        BUILD_FLOAT_PROPERTY(pAbility, sManaCost, ManaCost, 0, 2);
        vHeroAbilityInfo[5] = sManaCost;

        tstring sCooldownTime;
        {
            bool bConstant(true);
            uint uiCooldownTime(pAbility->GetCooldownTime(0));

            for (uint uiIndex(1); uiIndex < uiMaxLevel; ++uiIndex)
            {
                if (pAbility->GetCooldownTime(uiIndex) != uiCooldownTime)
                    bConstant = false;
            }

            if (bConstant)
            {
                if (uiCooldownTime != 0)
                    sCooldownTime = XtoA(MsToSec(uiCooldownTime), 0, 0, 0, 3);
            }
            else
            {
                if (uiLevel >= 1)
                    sCooldownTime = _T("^v") + XtoA(MsToSec(uiCooldownTime), 0, 0, 0, 3) + _T("^*");
                else
                    sCooldownTime = XtoA(MsToSec(uiCooldownTime), 0, 0, 0, 3);

                for (uint uiIndex(1); uiIndex < uiMaxLevel; ++uiIndex)
                {
                    sCooldownTime += _T('/');

                    if (uiLevel >= uiIndex + 1)
                        sCooldownTime += _T("^v") + XtoA(MsToSec(pAbility->GetCooldownTime(uiIndex)), 0, 0, 0, 3) + _T("^*");
                    else
                        sCooldownTime += XtoA(MsToSec(pAbility->GetCooldownTime(uiIndex)), 0, 0, 0, 3);
                }
            }
        }
        vHeroAbilityInfo[6] = sCooldownTime;
    }
    else
    {
        vHeroAbilityInfo[0] = _T("false");
    }

    GameClient.GetInterfaceManager()->Trigger(UITRIGGER_HERO_SELECT_HERO_ABILITY_INFO, vHeroAbilityInfo, iSlot);
}


/*--------------------
  GetHeroInfo
  --------------------*/
UI_VOID_CMD(GetHeroInfo, 1)
{
    static tsvector vHeroInfo(24);

    CHeroDefinition *pHero(EntityRegistry.GetDefinition<CHeroDefinition>(vArgList[0]->Evaluate()));
    if (pHero == nullptr)
    {
        vHeroInfo[0] = _T("false");
        GameClient.GetInterfaceManager()->Trigger(UITRIGGER_HERO_SELECT_HERO_INFO, vHeroInfo);
        return;
    }

    vHeroInfo[0] = _T("true");
    vHeroInfo[1] = pHero->GetIconPath(0);
    vHeroInfo[2] = pHero->GetDisplayName();
    vHeroInfo[3] = pHero->GetDescription();
    switch (pHero->GetPrimaryAttribute())
    {
    case ATTRIBUTE_STRENGTH:
        vHeroInfo[4] = _T("strength");
        vHeroInfo[15] = XtoA(pHero->GetAttackDamageMin(0) + pHero->GetStrength());
        vHeroInfo[16] = XtoA(pHero->GetAttackDamageMax(0) + pHero->GetStrength());
        break;
    case ATTRIBUTE_AGILITY:
        vHeroInfo[4] = _T("agility");
        vHeroInfo[15] = XtoA(pHero->GetAttackDamageMin(0) + pHero->GetAgility());
        vHeroInfo[16] = XtoA(pHero->GetAttackDamageMax(0) + pHero->GetAgility());
        break;
    case ATTRIBUTE_INTELLIGENCE:
        vHeroInfo[4] = _T("intelligence");
        vHeroInfo[15] = XtoA(pHero->GetAttackDamageMin(0) + pHero->GetIntelligence());
        vHeroInfo[16] = XtoA(pHero->GetAttackDamageMax(0) + pHero->GetIntelligence());
        break;
    }
    vHeroInfo[5] = XtoA(pHero->GetStrength());
    vHeroInfo[6] = XtoA(pHero->GetStrengthPerLevel());
    vHeroInfo[7] = XtoA(pHero->GetAgility());
    vHeroInfo[8] = XtoA(pHero->GetAgilityPerLevel());
    vHeroInfo[9] = XtoA(pHero->GetIntelligence());
    vHeroInfo[10] = XtoA(pHero->GetIntelligencePerLevel());
    vHeroInfo[11] = XtoA(pHero->GetMoveSpeed(0));
    vHeroInfo[12] = Game.GetAttackTypeDisplayName(pHero->GetAttackType(0));
    vHeroInfo[13] = XtoA(pHero->GetAttackRange(0));
    vHeroInfo[14] = XtoA(pHero->GetAttackCooldown(0));
    vHeroInfo[17] = XtoA(IHeroEntity::AdjustArmor(pHero->GetArmor(0), pHero->GetAgility()));
    vHeroInfo[18] = XtoA(_T("TODO: magic armor"));
    vHeroInfo[19] = XtoA(IHeroEntity::AdjustMaxHealth(pHero->GetMaxHealth(0), pHero->GetStrength()));
    vHeroInfo[20] = XtoA(_T("TODO: health regen"));
    vHeroInfo[21] = XtoA(IHeroEntity::AdjustMaxMana(pHero->GetMaxMana(0), pHero->GetIntelligence()));
    vHeroInfo[22] = XtoA(_T("TODO: mana regen"));
    const CAttackType *pAttackType(GameClient.GetAttackType(pHero->GetAttackType(0)));
    vHeroInfo[23] = pAttackType ? pAttackType->GetName() : TSNULL;

    GameClient.GetInterfaceManager()->Trigger(UITRIGGER_HERO_SELECT_HERO_INFO, vHeroInfo);

#define GET_ABILITY_INFO(n) \
{\
    CAbilityDefinition *pAbility(EntityRegistry.GetDefinition<CAbilityDefinition>(pHero->GetInventory##n(0))); \
    GetAbilityInfo(pAbility, n); \
}

    GET_ABILITY_INFO(0)
    GET_ABILITY_INFO(1)
    GET_ABILITY_INFO(2)
    GET_ABILITY_INFO(3)
}


/*--------------------
  GetDetailPlayerStats
  --------------------*/
UI_VOID_CMD(GetDetailPlayerStats, 1)
{
    static tsvector vStats(45);
    for (tsvector_it it(vStats.begin()); it != vStats.end(); ++it)
        it->clear();

    CPlayer *pPlayer(GameClient.GetPlayer(vArgList[0]->EvaluateInteger()));
    CGameStats *pStats(pPlayer ? pPlayer->GetStats() : nullptr);
    if (pPlayer == nullptr || pStats == nullptr)
    {
        GameClient.GetInterfaceManager()->Trigger(UITRIGGER_ENDGAME_PLAYER_DETAIL_STATS, vStats);
        return;
    }

    vStats[0] = XtoA(pStats->GetTimePlayed());          // Time
    vStats[1] = XtoA(pStats->GetExperience());          // Experience
    vStats[2] = XtoA(pStats->GetDeaths());              // Deaths
    vStats[3] = XtoA(pStats->GetHeroKills());           // Hero Kills
    vStats[4] = XtoA(pStats->GetHeroDamage());          // Hero Damage
    vStats[5] = XtoA(pStats->GetHeroAssists());         // Hero Assists
    vStats[6] = XtoA(pStats->GetHeroBounty());          // Hero Bounty
    vStats[7] = XtoA(pStats->GetCreepKills());          // Creep Kills
    vStats[8] = XtoA(pStats->GetCreepDamage());         // Creep Damage
    vStats[9] = XtoA(pStats->GetCreepBounty());         // Creep Bounty
    vStats[10] = XtoA(pStats->GetDenies());             // Creep Denies
    vStats[11] = XtoA(pStats->GetNeutralKills());       // Neutral Kills
    vStats[12] = XtoA(pStats->GetNeutralDamage());      // Neutral Damage
    vStats[13] = XtoA(pStats->GetNeutralBounty());      // Neutral Bounty
    vStats[14] = XtoA(pStats->GetBuildingKills());      // Building Kills
    vStats[15] = XtoA(pStats->GetBuildingDamage());     // Building Damage
    vStats[16] = XtoA(pStats->GetBuildingBounty());     // Building Bounty
    vStats[17] = XtoA(pStats->GetGoldEarned());         // Gold earned
    vStats[18] = XtoA(pStats->GetGoldLost());           // Gold lost
    vStats[19] = XtoA(pStats->GetGoldSpent());          // Gold spent
    vStats[20] = XtoA(pStats->GetActionCount());        // APM
    vStats[21] = XtoA(pStats->GetBuyBacks());           // Buy backs
    vStats[22] = XtoA(pStats->GetDeniedExperience());   // Denied Experience
    vStats[41] = XtoA(pStats->GetHeroExperience());     // Experience from hero kills
    vStats[42] = XtoA(pStats->GetCreepExperience());    // Experience from creep kills
    vStats[43] = XtoA(pStats->GetNeutralExperience());  // Experience from neutral kills
    vStats[44] = XtoA(pStats->GetBuildingExperience()); // Experience from building kills

    // Ability upgrades
    static tsvector vAbility(3);
    vAbility[0] = _T("0");
    GameClient.GetInterfaceManager()->Trigger(UITRIGGER_ENDGAME_PLAYER_DETAIL_ABILITY_HISTORY, vAbility);
    const CGameStats::AbilityUpgradeLog &vAbilityUpgrades(pStats->GetAbilityUpgradeLog());
    for (CGameStats::AbilityUpgradeLog_cit it(vAbilityUpgrades.begin()); it != vAbilityUpgrades.end(); ++it)
    {
        vAbility[0] = XtoA(it->yLevel);
        CAbilityDefinition *pDefinition(EntityRegistry.GetDefinition<CAbilityDefinition>(it->unAbilityTypeID));
        vAbility[1] = XtoA(pDefinition ? pDefinition->GetDisplayName() : TSNULL);
        vAbility[2] = XtoA(it->uiTimeStamp);
        GameClient.GetInterfaceManager()->Trigger(UITRIGGER_ENDGAME_PLAYER_DETAIL_ABILITY_HISTORY, vAbility);
    }

    static tsvector vKillEvent(2);

    // Kills
    vKillEvent[0] = _T("-1");
    GameClient.GetInterfaceManager()->Trigger(UITRIGGER_ENDGAME_PLAYER_DETAIL_KILLS, vKillEvent);
    const CGameStats::HeroKillLog &vKills(pStats->GetKillLog());
    for (CGameStats::HeroKillLog_cit it(vKills.begin()); it != vKills.end(); ++it)
    {
        CPlayer *pPlayer(GameClient.GetPlayer(it->iVictim));
        vKillEvent[0] = pPlayer == nullptr ? TSNULL : pPlayer->GetName();
        vKillEvent[1] = XtoA(it->uiTimeStamp);
        GameClient.GetInterfaceManager()->Trigger(UITRIGGER_ENDGAME_PLAYER_DETAIL_KILLS, vKillEvent);
    }

    // Assists
    vKillEvent[0] = _T("-1");
    GameClient.GetInterfaceManager()->Trigger(UITRIGGER_ENDGAME_PLAYER_DETAIL_ASSISTS, vKillEvent);
    const CGameStats::KillLogVector &vAssists(pStats->GetAssistLog());
    for (CGameStats::KillLogVector_cit it(vAssists.begin()); it != vAssists.end(); ++it)
    {
        CPlayer *pPlayer(GameClient.GetPlayer(it->second));
        vKillEvent[0] = pPlayer == nullptr ? TSNULL : pPlayer->GetName();
        vKillEvent[1] = XtoA(it->first);
        GameClient.GetInterfaceManager()->Trigger(UITRIGGER_ENDGAME_PLAYER_DETAIL_ASSISTS, vKillEvent);
    }

    // Deaths
    vKillEvent[0] = _T("-1");
    GameClient.GetInterfaceManager()->Trigger(UITRIGGER_ENDGAME_PLAYER_DETAIL_DEATHS, vKillEvent);
    const CGameStats::KillLogVector &vDeaths(pStats->GetDeathLog());
    for (CGameStats::KillLogVector_cit it(vDeaths.begin()); it != vDeaths.end(); ++it)
    {
        CPlayer *pPlayer(GameClient.GetPlayer(it->second));
        vKillEvent[0] = pPlayer == nullptr ? TSNULL : pPlayer->GetName();
        vKillEvent[1] = XtoA(it->first);
        GameClient.GetInterfaceManager()->Trigger(UITRIGGER_ENDGAME_PLAYER_DETAIL_DEATHS, vKillEvent);
    }

    // Final hero stats
    IHeroEntity *pHero(pPlayer->GetHero());
    if (pHero != nullptr)
    {
        vStats[23] = XtoA(pHero->GetDisplayName());             // Hero name
        vStats[24] = XtoA(pHero->GetIconPath());                // Hero icon
        vStats[25] = XtoA(pHero->GetLevel());                   // Hero level
        vStats[26] = XtoA(pHero->GetMaxHealth());               // Health
        vStats[27] = XtoA(pHero->GetMaxMana());                 // Mana
        vStats[28] = XtoA(pHero->GetAttackDamageMin());         // Min damage
        vStats[29] = XtoA(pHero->GetAttackDamageMax());         // Max damage
        vStats[30] = XtoA(pHero->GetBaseDamageMultiplier());    // Damage multiplier
        vStats[31] = XtoA(pHero->GetBonusDamage());             // Bonus damage
        vStats[32] = XtoA(pHero->GetMoveSpeed());               // Speed
        vStats[33] = XtoA(pHero->GetBaseAttackSpeed());         // Base Attack speed
        vStats[34] = XtoA(pHero->GetAttackSpeed());             // AttackSpeed
        vStats[35] = XtoA(pHero->GetIntelligence());            // Int
        vStats[36] = XtoA(pHero->GetAgility());                 // Agi
        vStats[37] = XtoA(pHero->GetStrength());                // Str
        vStats[38] = XtoA(pHero->GetBaseArmor());               // Base Armor
        vStats[39] = XtoA(pHero->GetArmor());                   // Armor
        vStats[40] = XtoA(Game.GetArmorDamageAdjustment(pHero->GetArmorType(), pHero->GetArmor())); // Armor Damage Reduction

        for (int iSlot(INVENTORY_START_BACKPACK); iSlot <= INVENTORY_END_BACKPACK; ++iSlot)
        {
            static tsvector vInventory(2);
            IEntityItem *pItem(pHero->GetItem(iSlot));
            if (pItem == nullptr)
            {
                for (tsvector_it it(vInventory.begin()); it != vInventory.end(); ++it)
                    it->clear();
            }
            else
            {
                vInventory[0] = XtoA(pItem->GetTypeName()); // Type name
                vInventory[1] = XtoA(pItem->GetIconPath()); // Icon
            }
            GameClient.GetInterfaceManager()->Trigger(UITRIGGER_ENDGAME_PLAYER_DETAIL_INVENTORY, vInventory, iSlot - INVENTORY_START_BACKPACK);
        }
    }

    GameClient.GetInterfaceManager()->Trigger(UITRIGGER_ENDGAME_PLAYER_DETAIL_STATS, vStats);
}


/*--------------------
  GetPlayerStats
  --------------------*/
UI_VOID_CMD(GetPlayerStats, 1)
{
    GameClient.GetInterfaceManager()->RequestPlayerStats(vArgList[0]->EvaluateInteger());
}


/*--------------------
  GetPlayerStatsName
  --------------------*/
UI_VOID_CMD(GetPlayerStatsName, 1)
{
    GameClient.GetInterfaceManager()->RequestPlayerStats(vArgList[0]->Evaluate());
}

CMD(GetPlayerStatsName)
{
    if (vArgList.size() < 1)
        return false;

    GameClient.GetInterfaceManager()->RequestPlayerStats(vArgList[0]);
    return true;
}


/*--------------------
  SetReplayInfo
  --------------------*/
UI_VOID_CMD(SetReplayInfo, 1)
{
    GameClient.GetInterfaceManager()->SetReplayInfo(vArgList[0]->Evaluate());
}


/*--------------------
  GetMatchInfo
  --------------------*/
UI_VOID_CMD(GetMatchInfo, 1)
{
    GameClient.GetInterfaceManager()->RequestMatchInfo(vArgList[0]->EvaluateInteger());
}

CMD(GetMatchInfo)
{
    if (vArgList.size() < 1)
        return false;

    GameClient.GetInterfaceManager()->RequestMatchInfo(AtoI(vArgList[0]));
    return true;
}


/*--------------------
  GetRecentMatches
  --------------------*/
UI_VOID_CMD(GetRecentMatches, 0)
{
    GameClient.GetInterfaceManager()->RequestRecentMatches();
}

CMD(GetRecentMatches)
{
    GameClient.GetInterfaceManager()->RequestRecentMatches();
    return true;
}


/*--------------------
  RegisterEntityDefinitions
  --------------------*/
UI_VOID_CMD(RegisterEntityDefinitions, 0)
{
    GameClient.GetInterfaceManager()->RegisterEntityDefinitions();
}

CMD(RegisterEntityDefinitions)
{
    GameClient.GetInterfaceManager()->RegisterEntityDefinitions();
    return true;
}


/*--------------------
  ClearMatchInfo
  --------------------*/
UI_VOID_CMD(ClearMatchInfo, 0)
{
    GameClient.GetInterfaceManager()->ClearMatchInfo();
}

CMD(ClearMatchInfo)
{
    GameClient.GetInterfaceManager()->ClearMatchInfo();
    return true;
}


/*--------------------
  RefreshReplayList
  --------------------*/
UI_VOID_CMD(RefreshReplayList, 0)
{
    GameClient.GetInterfaceManager()->RefreshReplayList();
}


/*--------------------
  SetPreviewMap
  --------------------*/
UI_VOID_CMD(SetPreviewMap, 1)
{
    GameClient.GetInterfaceManager()->SetPreviewMap(vArgList[0]->Evaluate());
}

/*--------------------
  GetTournamentInfo
  --------------------*/
UI_VOID_CMD(GetTournamentInfo, 1)
{
    GameClient.GetInterfaceManager()->RequestTournamentInfo(AtoUI(vArgList[0]->Evaluate()));
}

/*--------------------
  GetTournamentsForAccount
  --------------------*/
UI_VOID_CMD(GetTournamentsForAccount, 1)
{
    GameClient.GetInterfaceManager()->RequestTournamentsForAccount(AtoUI(vArgList[0]->Evaluate()));
}

/*--------------------
  SortStats
  --------------------*/
UI_VOID_CMD(SortStatsAsc, 2)
{
    GameClient.GetInterfaceManager()->SortStatsAsc(AtoI(vArgList[0]->Evaluate()), AtoI(vArgList[1]->Evaluate()));
}

UI_VOID_CMD(SortStatsDesc, 2)
{
    GameClient.GetInterfaceManager()->SortStatsDesc(AtoI(vArgList[0]->Evaluate()), AtoI(vArgList[1]->Evaluate()));
}


/*--------------------
  SortStatsValue
  --------------------*/
UI_VOID_CMD(SortStatsValueAsc, 2)
{
    GameClient.GetInterfaceManager()->SortStatsByValueAsc(AtoI(vArgList[0]->Evaluate()), AtoI(vArgList[1]->Evaluate()));
}

UI_VOID_CMD(SortStatsValueDesc, 2)
{
    GameClient.GetInterfaceManager()->SortStatsByValueDesc(AtoI(vArgList[0]->Evaluate()), AtoI(vArgList[1]->Evaluate()));
}


/*--------------------
  SortHeroes
  --------------------*/
UI_VOID_CMD(SortHeroesAsc, 1)
{
    g_uiHeroSortValue = AtoI(vArgList[0]->Evaluate());
    g_bHeroSortByValue = false;
    g_bHeroSortDesc = false;

    GameClient.GetInterfaceManager()->UpdateHeroCompendium();
}

UI_VOID_CMD(SortHeroesDesc, 1)
{
    g_uiHeroSortValue = AtoI(vArgList[0]->Evaluate());
    g_bHeroSortByValue = false;
    g_bHeroSortDesc = true;

    GameClient.GetInterfaceManager()->UpdateHeroCompendium();
}


/*--------------------
  SortHeroesValue
  --------------------*/
UI_VOID_CMD(SortHeroesValueAsc, 1)
{
    g_uiHeroSortValue = AtoI(vArgList[0]->Evaluate());
    g_bHeroSortByValue = true;
    g_bHeroSortDesc = false;

    GameClient.GetInterfaceManager()->UpdateHeroCompendium();
}

UI_VOID_CMD(SortHeroesValueDesc, 1)
{
    g_uiHeroSortValue = AtoI(vArgList[0]->Evaluate());
    g_bHeroSortByValue = true;
    g_bHeroSortDesc = true;

    GameClient.GetInterfaceManager()->UpdateHeroCompendium();
}


/*--------------------
  UpdateHeroCompendium
  --------------------*/
UI_VOID_CMD(UpdateHeroCompendium, 0)
{
    g_uiHeroSortValue = -1;
    GameClient.GetInterfaceManager()->UpdateHeroCompendium();
}

CMD(UpdateHeroCompendium)
{
    g_uiHeroSortValue = -1;
    GameClient.GetInterfaceManager()->UpdateHeroCompendium();
    return true;
}


/*--------------------
  ShowHeroCompendiumInfo
  --------------------*/
UI_VOID_CMD(ShowHeroCompendiumInfo, 1)
{
    GameClient.GetInterfaceManager()->ShowHeroCompendiumInfo(vArgList[0]->Evaluate());
}

CMD(ShowHeroCompendiumInfo)
{
    if (vArgList.size() < 1)
        return false;

    GameClient.GetInterfaceManager()->ShowHeroCompendiumInfo(vArgList[0]);
    return true;
}


/*--------------------
StartInfoReplay
  --------------------*/
UI_VOID_CMD(StartInfoReplay, 1)
{
    GameClient.GetInterfaceManager()->SetReplayInfo(vArgList[0]->Evaluate());

    if (GameClient.GetInterfaceManager()->IsReplayCompatible())
        Console.Execute(_T("StartReplay ") + QuoteStr(vArgList[0]->Evaluate()));
    else
        Console.Execute(_T("DownloadCompat ") + GameClient.GetInterfaceManager()->GetReplayVersion());
}


/*--------------------
  ClearEndGameStats
  --------------------*/
UI_VOID_CMD(ClearEndGameStats, 0)
{
    GameClient.GetInterfaceManager()->ClearEndGameStats();
}

CMD(ClearEndGameStats)
{
    GameClient.GetInterfaceManager()->ClearEndGameStats();
    return true;
}


/*--------------------
  LoadEntityDefinition
  --------------------*/
UI_VOID_CMD(LoadEntityDefinition, 1)
{
    GameClient.GetInterfaceManager()->LoadEntityDefinition(vArgList[0]->Evaluate());
}

CMD(LoadEntityDefinition)
{
    if (vArgList.size() < 1)
        return false;

    GameClient.GetInterfaceManager()->LoadEntityDefinition(vArgList[0]);
    return true;
}


/*--------------------
  GetHeroIconPathFromHeroDefinition
  --------------------*/
UI_CMD(GetIconPathFromHeroDefinition, 1)
{
    return GameClient.GetInterfaceManager()->GetHeroIconPathFromHeroDefinition(vArgList[0]->Evaluate(), vArgList.size() > 1 ? vArgList[1]->Evaluate() : TSNULL);
}

FUNCTION(GetHeroIconPathFromHeroDefinition)
{
    if (vArgList.size() < 1)
        return TSNULL;

    return GameClient.GetInterfaceManager()->GetHeroIconPathFromHeroDefinition(vArgList[0], vArgList.size() > 1 ? vArgList[1] : TSNULL);
}


/*--------------------
  GetHeroIconPathFromProduct
  --------------------*/
UI_CMD(GetHeroIconPathFromProduct, 1)
{
    return GameClient.GetInterfaceManager()->GetHeroIconPathFromProduct(vArgList[0]->Evaluate());
}

FUNCTION(GetHeroIconPathFromProduct)
{
    if (vArgList.size() < 1)
        return TSNULL;

    return GameClient.GetInterfaceManager()->GetHeroIconPathFromProduct(vArgList[0]);
}


/*--------------------
  GetHeroPreviewModelPathFromProduct
  --------------------*/
UI_CMD(GetHeroPreviewModelPathFromProduct, 1)
{
    return GameClient.GetInterfaceManager()->GetHeroPreviewModelPathFromProduct(vArgList[0]->Evaluate());
}

FUNCTION(GetHeroPreviewModelPathFromProduct)
{
    if (vArgList.size() < 1)
        return TSNULL;

    return GameClient.GetInterfaceManager()->GetHeroPreviewModelPathFromProduct(vArgList[0]);
}


/*--------------------
  GetHeroPassiveEffectPathFromProduct
  --------------------*/
UI_CMD(GetHeroPassiveEffectPathFromProduct, 1)
{
    return GameClient.GetInterfaceManager()->GetHeroPassiveEffectPathFromProduct(vArgList[0]->Evaluate());
}

FUNCTION(GetHeroPassiveEffectPathFromProduct)
{
    if (vArgList.size() < 1)
        return TSNULL;

    return GameClient.GetInterfaceManager()->GetHeroPassiveEffectPathFromProduct(vArgList[0]);
}


/*--------------------
  GetHeroPreviewPosFromProduct
  --------------------*/
UI_CMD(GetHeroPreviewPosFromProduct, 1)
{
    return XtoA(GameClient.GetInterfaceManager()->GetHeroPreviewPosFromProduct(vArgList[0]->Evaluate()));
}

FUNCTION(GetHeroPreviewPosFromProduct)
{
    if (vArgList.size() < 1)
        return TSNULL;

    return XtoA(GameClient.GetInterfaceManager()->GetHeroPreviewPosFromProduct(vArgList[0]));
}


/*--------------------
  GetHeroPreviewAnglesFromProduct
  --------------------*/
UI_CMD(GetHeroPreviewAnglesFromProduct, 1)
{
    return XtoA(GameClient.GetInterfaceManager()->GetHeroPreviewAnglesFromProduct(vArgList[0]->Evaluate()));
}

FUNCTION(GetHeroPreviewAnglesFromProduct)
{
    if (vArgList.size() < 1)
        return TSNULL;

    return XtoA(GameClient.GetInterfaceManager()->GetHeroPreviewAnglesFromProduct(vArgList[0]));
}


/*--------------------
  GetHeroPreviewScaleFromProduct
  --------------------*/
UI_CMD(GetHeroPreviewScaleFromProduct, 1)
{
    return XtoA(GameClient.GetInterfaceManager()->GetHeroPreviewScaleFromProduct(vArgList[0]->Evaluate()));
}

FUNCTION(GetHeroPreviewScaleFromProduct)
{
    if (vArgList.size() < 1)
        return TSNULL;

    return XtoA(GameClient.GetInterfaceManager()->GetHeroPreviewScaleFromProduct(vArgList[0]));
}

