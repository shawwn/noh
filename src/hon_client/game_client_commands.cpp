// (C)2006 S2 Games
// game_client_commands.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_client_common.h"

#include "c_gameclient.h"
#include "c_clientcommander.h"
#include "c_gameinterfacemanager.h"

#include "../hon_shared/c_teaminfo.h"
#include "../hon_shared/i_unitentity.h"
#include "../hon_shared/i_entityitem.h"
#include "../hon_shared/i_shopentity.h"
#include "../hon_shared/c_shopdefinition.h"
#include "../hon_shared/i_buildingentity.h"
#include "../hon_shared/i_heroentity.h"
#include "../hon_shared/i_entityability.h"
#include "../hon_shared/i_areaaffector.h"
#include "../hon_shared/c_linearaffector.h"
#include "../hon_shared/i_projectile.h"

#include "../hon_client/c_gameclient.h"

#include "../k2/c_eventcmd.h"
#include "../k2/c_buffer.h"
#include "../k2/c_vid.h"
#include "../k2/c_camera.h"
#include "../k2/c_bitmap.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_sample.h"
#include "../k2/c_input.h"
#include "../k2/c_uicmd.h"
#include "../k2/c_uitrigger.h"
#include "../k2/c_function.h"
#include "../k2/c_statestring.h"
#include "../k2/i_listwidget.h"
#include "../k2/c_chatmanager.h"
#include "../k2/c_date.h"
#include "../k2/c_xmldoc.h"
#include "../k2/c_resourcemanager.h"
#include "../k2/c_resourceinfo.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
UI_TRIGGER(KarmaList);
EXTERN_CVAR_STRING(cg_current_weather);
//=============================================================================

/*--------------------
  Purchase
  --------------------*/
CMD(Purchase)
{
    if (vArgList.empty())
        return false;

    return GameClient.Purchase(AtoI(vArgList[0]));
}

UI_VOID_CMD(Purchase, 1)
{
    cmdPurchase(vArgList[0]->Evaluate());
}


/*--------------------
  Purchase2
  --------------------*/
CMD(Purchase2)
{
    if (vArgList.empty())
        return false;

    ushort unID(EntityRegistry.LookupID(vArgList[0]));
    if (unID == INVALID_ENT_TYPE)
    {
        Console << _T("Invalid item: ") << vArgList[0] << newl;
        return false;
    }

    CClientCommander *pCommander(GameClient.GetClientCommander());
    if (pCommander == nullptr)
        return false;

    uint uiUnitIndex(pCommander->GetSelectedControlEntityIndex());
    if (uiUnitIndex == INVALID_INDEX)
        return false;

    CBufferFixed<7> buffer;
    buffer << GAME_CMD_PURCHASE2 << uiUnitIndex << unID;
    GameClient.SendGameData(buffer, true);
    
    return true;
}

UI_VOID_CMD(Purchase2, 1)
{
    cmdPurchase2(vArgList[0]->Evaluate());
}


/*--------------------
  PurchaseRecipe
  --------------------*/
CMD(PurchaseRecipe)
{
    CClientCommander *pCommander(GameClient.GetClientCommander());
    if (pCommander == nullptr)
        return false;

    uint uiUnitIndex(pCommander->GetSelectedControlEntityIndex());
    if (uiUnitIndex == INVALID_INDEX)
        return false;

    ushort unID(EntityRegistry.LookupID(GameClient.GetActiveRecipe()));
    if (unID == INVALID_ENT_TYPE)
    {
        Console << _T("Invalid item: ") << GameClient.GetActiveRecipe() << newl;
        return false;
    }

    CBufferFixed<7> buffer;
    buffer << GAME_CMD_PURCHASE2 << uiUnitIndex << unID;
    GameClient.SendGameData(buffer, true);

    return true;
}

UI_VOID_CMD(PurchaseRecipe, 0)
{
    cmdPurchaseRecipe();
}


/*--------------------
  PurchaseComponent
  --------------------*/
CMD(PurchaseComponent)
{
    if (vArgList.empty())
        return false;
    
    return GameClient.PurchaseComponent(AtoI(vArgList[0]), vArgList.size() > 1 ? AtoI(vArgList[1]) : 0);
}

UI_VOID_CMD(PurchaseComponent, 1)
{
    cmdPurchaseComponent(vArgList[0]->Evaluate());
}


/*--------------------
  PurchaseAllComponents
  --------------------*/
CMD(PurchaseAllComponents)
{
    if (vArgList.empty())
        return false;
    
    return GameClient.PurchaseAllComponents(vArgList[0]);
}

UI_VOID_CMD(PurchaseAllComponents, 1)
{
    cmdPurchaseAllComponents(vArgList[0]->Evaluate());
}


/*--------------------
  GetActiveRecipe
  --------------------*/
UI_CMD(GetActiveRecipe, 0)
{
    return GameClient.GetActiveRecipe();
}


/*--------------------
  PurchaseUsedIn
  --------------------*/
CMD(PurchaseUsedIn)
{
    if (vArgList.empty())
        return false;

    return GameClient.PurchaseUsedIn(AtoI(vArgList[0]));
}

UI_VOID_CMD(PurchaseUsedIn, 1)
{
    cmdPurchaseUsedIn(vArgList[0]->Evaluate());
}


/*--------------------
  Sell
  --------------------*/
CMD(Sell)
{
    if (vArgList.empty())
        return false;

    GameClient.ItemSell(AtoI(vArgList[0]));
    return true;
}

UI_VOID_CMD(Sell, 0)
{
    GameClient.ItemSell(vArgList.empty() ? -1 : vArgList[0]->EvaluateInteger());
}


/*--------------------
  Disaassemble
  --------------------*/
CMD(Disaassemble)
{
    if (vArgList.empty())
        return false;

    GameClient.ItemSell(AtoI(vArgList[0]));
    return true;
}

UI_VOID_CMD(Disaassemble, 1)
{
    GameClient.ItemSell(vArgList[0]->EvaluateInteger());
}


/*--------------------
  LevelUpAbility
  --------------------*/
CMD(LevelUpAbility)
{
    if (vArgList.empty())
        return false;

    byte ySlot(byte(AtoI(vArgList[0])));
    if (ySlot >= MAX_INVENTORY)
    {
        Console << _T("Invalid slot: ") << ySlot << newl;
        return false;
    }

    return GameClient.LevelupAbility(ySlot);
}

UI_VOID_CMD(LevelUpAbility, 1)
{
    cmdLevelUpAbility(vArgList[0]->Evaluate());
}


/*--------------------
  Levelup
  --------------------*/
UI_VOID_CMD(Levelup, 1)
{
    GameClient.SetLevelup(AtoB(vArgList[0]->Evaluate()));
}



/*--------------------
  PrimaryAction
  --------------------*/
CMD(PrimaryAction)
{
    if (vArgList.empty())
        return false;

    int iSlot(AtoI(vArgList[0]));
    if (iSlot < INVENTORY_START_BACKPACK || iSlot > INVENTORY_END_BACKPACK)
        return false;

    GameClient.PrimaryAction(iSlot);
    return true;
}

UI_VOID_CMD(PrimaryAction, 1)
{
    cmdPrimaryAction(vArgList[0]->Evaluate());
}



/*--------------------
  SecondaryAction
  --------------------*/
CMD(SecondaryAction)
{
    if (vArgList.empty())
        return false;

    int iSlot(AtoI(vArgList[0]));
    if (iSlot < INVENTORY_START_BACKPACK || iSlot > INVENTORY_END_BACKPACK)
        return false;

    GameClient.SecondaryAction(iSlot);
    return true;
}

UI_VOID_CMD(SecondaryAction, 1)
{
    cmdSecondaryAction(vArgList[0]->Evaluate());
}


/*--------------------
  ItemPlace
  --------------------*/
CMD(ItemPlace)
{
    if (vArgList.empty())
        return false;

    int iSlot(AtoI(vArgList[0]));
    if (iSlot < INVENTORY_START_BACKPACK || iSlot > INVENTORY_END_BACKPACK)
        return false;

    GameClient.ItemPlace(iSlot);
    return true;
}

UI_VOID_CMD(ItemPlace, 1)
{
    cmdItemPlace(vArgList[0]->Evaluate());
}


/*--------------------
  PrimaryActionStash
  --------------------*/
CMD(PrimaryActionStash)
{
    if (vArgList.empty())
        return false;

    int iSlot(AtoI(vArgList[0]));
    if (iSlot < 0 || iSlot >= INVENTORY_STASH_SIZE)
        return false;

    GameClient.PrimaryActionStash(iSlot);
    return true;
}

UI_VOID_CMD(PrimaryActionStash, 1)
{
    cmdPrimaryActionStash(vArgList[0]->Evaluate());
}


/*--------------------
  SecondaryActionStash
  --------------------*/
CMD(SecondaryActionStash)
{
    if (vArgList.empty())
        return false;

    int iSlot(AtoI(vArgList[0]));
    if (iSlot < 0 || iSlot >= INVENTORY_STASH_SIZE)
        return false;

    GameClient.SecondaryActionStash(iSlot);
    return true;
}

UI_VOID_CMD(SecondaryActionStash, 1)
{
    cmdSecondaryActionStash(vArgList[0]->Evaluate());
}


/*--------------------
  ItemPlaceStash
  --------------------*/
CMD(ItemPlaceStash)
{
    if (vArgList.empty())
        return false;

    int iSlot(AtoI(vArgList[0]));
    if (iSlot < 0 || iSlot >= INVENTORY_STASH_SIZE)
        return false;

    GameClient.ItemPlaceStash(iSlot);
    return true;
}

UI_VOID_CMD(ItemPlaceStash, 1)
{
    cmdItemPlaceStash(vArgList[0]->Evaluate());
}


/*--------------------
  ItemPlaceHero
  --------------------*/
CMD(ItemPlaceHero)
{
    GameClient.ItemPlaceHero();
    return true;
}

UI_VOID_CMD(ItemPlaceHero, 0)
{
    cmdItemPlaceHero();
}


/*--------------------
  ItemPlaceSelected
  --------------------*/
CMD(ItemPlaceSelected)
{
    if (vArgList.empty())
        return false;

    int iSlot(AtoI(vArgList[0]));
    if (iSlot < 0 || iSlot >= INVENTORY_STASH_SIZE)
        return false;

    GameClient.ItemPlaceSelected(iSlot);
    return true;
}

UI_VOID_CMD(ItemPlaceSelected, 1)
{
    cmdItemPlaceSelected(vArgList[0]->Evaluate());
}


/*--------------------
  ItemPlaceEntity
  --------------------*/
CMD(ItemPlaceEntity)
{
    if (vArgList.empty())
        return false;

    uint uiIndex(AtoUI(vArgList[0]));
    GameClient.ItemPlaceEntity(uiIndex);
    return true;
}

UI_VOID_CMD(ItemPlaceEntity, 1)
{
    cmdItemPlaceEntity(vArgList[0]->Evaluate());
}



/*--------------------
  Team
  --------------------*/
CMD(Team)
{
    if (vArgList.empty())
        return false;

    uint uiSlot(-1);
    if (vArgList.size() > 1)
        uiSlot = AtoI(vArgList[1]);

    CBufferFixed<9> buffer;
    buffer << GAME_CMD_CHANGE_TEAM << AtoI(vArgList[0]) << uiSlot;
    GameClient.SendGameData(buffer, true);
    return true;
}

UI_VOID_CMD(Team, 1)
{
    if (vArgList.size() > 1)
        cmdTeam(vArgList[0]->Evaluate(), vArgList[1]->Evaluate());
    else
        cmdTeam(vArgList[0]->Evaluate());
}

/*--------------------
  AssignFirstBanTeam
  --------------------*/
CMD(AssignFirstBanTeam)
{
    if (vArgList.empty())
        return false;

    CBufferFixed<5> buffer;
    buffer << GAME_CMD_ASSIGN_FIRST_BAN_TEAM << AtoI(vArgList[0]);
    GameClient.SendGameData(buffer, true);
    return true;
}

UI_VOID_CMD(AssignFirstBanTeam, 1)
{
    if (vArgList.size() > 1)
        cmdAssignFirstBanTeam(vArgList[0]->Evaluate(), vArgList[1]->Evaluate());
    else
        cmdAssignFirstBanTeam(vArgList[0]->Evaluate());
}

/*--------------------
  ToggleMenu
  --------------------*/
UI_VOID_CMD(ToggleMenu, 0)
{
    GameClient.ToggleMenu();
}


/*--------------------
  HideMenu
  --------------------*/
UI_VOID_CMD(HideMenu, 0)
{
    GameClient.HideMenu();
}


/*--------------------
  ToggleLobby
  --------------------*/
UI_VOID_CMD(ToggleLobby, 0)
{
    GameClient.ToggleMenu();
}


/*--------------------
  HideLobby
  --------------------*/
UI_VOID_CMD(HideLobby, 0)
{
    GameClient.HideMenu();
}


/*--------------------
  Cancel
  --------------------*/
UI_VOID_CMD(Cancel, 0)
{
    GameClient.Cancel();
}


/*--------------------
  OrderMove
  --------------------*/
UI_VOID_CMD(OrderMove, 0)
{
    CClientCommander *pCommander(GameClient.GetClientCommander());
    if (pCommander == nullptr)
        return;
    pCommander->SetCommanderState(COMSTATE_MOVE);
}


/*--------------------
  OrderStop
  --------------------*/
UI_VOID_CMD(OrderStop, 0)
{
    CBufferFixed<1> buffer;
    buffer << GAME_CMD_ORDER_STOP;
    GameClient.SendGameData(buffer, true);
}


/*--------------------
  OrderHold
  --------------------*/
UI_VOID_CMD(OrderHold, 0)
{
    byte yQueue(QUEUE_NONE);

    CClientCommander *pCommander(GameClient.GetClientCommander());
    if (pCommander != nullptr)
        yQueue = (pCommander->GetModifier1() ? (pCommander->GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK) : QUEUE_NONE);

    CBufferFixed<2> buffer;
    buffer << GAME_CMD_ORDER_HOLD << yQueue;
    GameClient.SendGameData(buffer, true);
}

/*--------------------
  OrderCancelAndHold
  --------------------*/
UI_VOID_CMD(OrderCancelAndHold, 0)
{
    byte yQueue(QUEUE_NONE);

    CClientCommander *pCommander(GameClient.GetClientCommander());
    if (pCommander != nullptr)
        yQueue = (pCommander->GetModifier1() ? (pCommander->GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK) : QUEUE_NONE);

    CBufferFixed<2> buffer;
    buffer << GAME_CMD_ORDER_CANCEL_AND_HOLD << yQueue;
    GameClient.SendGameData(buffer, true);
}


/*--------------------
  OrderAttack
  --------------------*/
UI_VOID_CMD(OrderAttack, 0)
{
    CClientCommander *pCommander(GameClient.GetClientCommander());
    if (pCommander == nullptr)
        return;
    pCommander->SetCommanderState(COMSTATE_ATTACK);
}


/*--------------------
  OrderPatrol
  --------------------*/
UI_VOID_CMD(OrderPatrol, 0)
{
    CClientCommander *pCommander(GameClient.GetClientCommander());
    if (pCommander == nullptr)
        return;
    pCommander->SetCommanderState(COMSTATE_PATROL);
}


/*--------------------
  SelectAvatar
  --------------------*/
CMD(SelectAvatar)
{
    if (vArgList.empty())
    {
        Console << _T("No hero avatar specified.") << newl;
        return false;
    }
    
    CBufferDynamic buffer;
    buffer << GAME_CMD_SELECT_AVATAR << TStringToUTF8(vArgList[0]) << byte(0);
    GameClient.SendGameData(buffer, true);
    return true;
}

UI_VOID_CMD(SelectAvatar, 1)
{
    cmdSelectAvatar(vArgList[0]->Evaluate());
}


/*--------------------
  SpawnHero
  --------------------*/
CMD(SpawnHero)
{
    if (vArgList.empty())
    {
        Console << _T("No hero specified.") << newl;
        return false;
    }

    ushort unHeroID(EntityRegistry.LookupID(vArgList[0]));
    if (unHeroID == INVALID_ENT_TYPE)
    {
        Console << _T("Hero not found: ") << vArgList[0] << newl;
        return false;
    }

    CBufferFixed<3> buffer;
    buffer << GAME_CMD_SELECT_HERO << unHeroID;
    GameClient.SendGameData(buffer, true);
    return true;
}

UI_VOID_CMD(SpawnHero, 1)
{
    cmdSpawnHero(vArgList[0]->Evaluate());
}

/*--------------------
  PotentialHero
  --------------------*/
CMD(PotentialHero)
{
    if (vArgList.empty())
    {
        Console << _T("No hero specified.") << newl;
        return false;
    }

    ushort unHeroID(EntityRegistry.LookupID(vArgList[0]));
    if (unHeroID == INVALID_ENT_TYPE)
    {
        Console << _T("Hero not found: ") << vArgList[0] << newl;
        return false;
    }

    CBufferFixed<3> buffer;
    buffer << GAME_CMD_SELECT_POTENTIAL_HERO << unHeroID;
    GameClient.SendGameData(buffer, true);
    return true;
}

UI_VOID_CMD(PotentialHero, 1)
{
    cmdPotentialHero(vArgList[0]->Evaluate());
}

/*--------------------
  RandomHero
  --------------------*/
CMD(RandomHero)
{
    CBufferFixed<1> buffer;
    buffer << GAME_CMD_RANDOM_HERO;
    GameClient.SendGameData(buffer, true);
    return true;
}

UI_VOID_CMD(RandomHero, 0)
{
    cmdRandomHero();
}


/*--------------------
  RequestKick
  --------------------*/
CMD(RequestKick)
{
    if (vArgList.empty())
        return false;

    int iClientNumber(-1);

    const PlayerMap &mapPlayers(GameClient.GetPlayerMap());
    for (PlayerMap_cit itPlayer(mapPlayers.begin()); itPlayer != mapPlayers.end(); ++itPlayer)
    {
        if (CompareNoCase(itPlayer->second->GetName(), vArgList[0]) == 0)
        {
            iClientNumber = itPlayer->first;
            break;
        }
    }

    if (iClientNumber == -1)
    {
        iClientNumber = AtoI(vArgList[0]);
        if (iClientNumber == 0 && vArgList[0] != _T("0"))
            iClientNumber = -1;
    }

    if (iClientNumber == -1)
        return false;

    CBufferFixed<5> buffer;
    buffer << GAME_CMD_KICK << iClientNumber;
    GameClient.SendGameData(buffer, true);
    return true;
}

UI_VOID_CMD(Kick, 1)
{
    cmdRequestKick(vArgList[0]->Evaluate());
}


/*--------------------
  PromoteRef
  --------------------*/
CMD(PromoteRef)
{
    if (vArgList.empty())
        return false;

    int iClientNumber(-1);

    const PlayerMap &mapPlayers(GameClient.GetPlayerMap());
    for (PlayerMap_cit itPlayer(mapPlayers.begin()); itPlayer != mapPlayers.end(); ++itPlayer)
    {
        if (CompareNoCase(itPlayer->second->GetName(), vArgList[0]) == 0)
        {
            iClientNumber = itPlayer->first;
            break;
        }
    }

    if (iClientNumber == -1)
    {
        iClientNumber = AtoI(vArgList[0]);
        if (iClientNumber == 0 && vArgList[0] != _T("0"))
            iClientNumber = -1;
    }

    if (iClientNumber == -1)
        return false;

    CBufferFixed<5> buffer;
    buffer << GAME_CMD_PROMOTE_REFEREE << iClientNumber;
    GameClient.SendGameData(buffer, true);
    return true;
}

UI_VOID_CMD(PromoteRef, 1)
{
    cmdPromoteRef(vArgList[0]->Evaluate());
}


/*--------------------
  DemoteRef
  --------------------*/
CMD(DemoteRef)
{
    if (vArgList.empty())
        return false;

    int iClientNumber(-1);

    const PlayerMap &mapPlayers(GameClient.GetPlayerMap());
    for (PlayerMap_cit itPlayer(mapPlayers.begin()); itPlayer != mapPlayers.end(); ++itPlayer)
    {
        if (CompareNoCase(itPlayer->second->GetName(), vArgList[0]) == 0)
        {
            iClientNumber = itPlayer->first;
            break;
        }
    }

    if (iClientNumber == -1)
    {
        iClientNumber = AtoI(vArgList[0]);
        if (iClientNumber == 0 && vArgList[0] != _T("0"))
            iClientNumber = -1;
    }

    if (iClientNumber == -1)
        return false;

    CBufferFixed<5> buffer;
    buffer << GAME_CMD_DEMOTE_REFEREE << iClientNumber;
    GameClient.SendGameData(buffer, true);
    return true;
}

UI_VOID_CMD(DemoteRef, 1)
{
    cmdDemoteRef(vArgList[0]->Evaluate());
}


/*--------------------
  BanHero
  --------------------*/
CMD(BanHero)
{
    if (vArgList.empty())
        return false;

    ushort unHeroID(EntityRegistry.LookupID(vArgList[0]));

    CBufferFixed<3> buffer;
    buffer << GAME_CMD_BAN_HERO << unHeroID;
    GameClient.SendGameData(buffer, true);
    return true;
}

UI_VOID_CMD(BanHero, 1)
{
    cmdBanHero(vArgList[0]->Evaluate());
}


/*--------------------
  DraftHero
  --------------------*/
CMD(DraftHero)
{
    if (vArgList.empty())
        return false;

    ushort unHeroID(AtoN(vArgList[0]));

    CBufferFixed<3> buffer;
    buffer << GAME_CMD_DRAFT_HERO << unHeroID;
    GameClient.SendGameData(buffer, true);
    return true;
}

UI_VOID_CMD(DraftHero, 1)
{
    cmdDraftHero(vArgList[0]->Evaluate());
}


bool    HandleGameSlashCommands(const tstring sText)
/*--------------------
  HandleGameSlashCommands
  --------------------*/
{
    tsvector vsTokens;
    vsTokens = TokenizeString(sText, ' ');      

    // handle the different slash commands specific for in game use
    if (CompareNoCase(vsTokens[0], ChatManager.Translate(_T("chat_command_roll"))) == 0)
    {
        ChatManager.AddChatHistory(sText);
        
        if (vsTokens.size() == 2)
        {
            if (AtoF(vsTokens[1]) <= 0 || AtoF(vsTokens[1]) > 32767)
            {
                ChatManager.AddIRCChatMessage(CHAT_MESSAGE_ADD, ChatManager.Translate(_T("chat_command_roll_help")));
                return false;
            }
            else
            {               
                const uint uiRand(M_Randnum(1, AtoI(vsTokens[1])));
                
                const tstring sRollMessage = ChatManager.Translate(_T("chat_roll_message"), _T("player"), Game.GetLocalPlayer()->GetName(), _T("low"), _T("1"), _T("high"), XtoA(AtoI(vsTokens[1])), _T("number"), XtoA(uiRand));
                
                if (!sRollMessage.empty())
                {                   
                    CBufferDynamic buffer;
                    buffer << GAME_CMD_CHAT_ROLL << TStringToUTF8(sRollMessage.substr(0,150)) << byte(0);
                    GameClient.SendGameData(buffer, true);              
                    return true;                                
                }
                else
                {
                    ChatManager.AddIRCChatMessage(CHAT_MESSAGE_ADD, ChatManager.Translate(_T("chat_command_roll_help")));
                    return false;
                }
            }
        }   
        else if (vsTokens.size() >= 3)
        {               
            if (AtoF(vsTokens[1]) <= 0 || AtoF(vsTokens[2]) <= 0 || AtoF(vsTokens[2]) <= AtoF(vsTokens[1]) || AtoF(vsTokens[2]) > 32767)
            {
                ChatManager.AddIRCChatMessage(CHAT_MESSAGE_ADD, ChatManager.Translate(_T("chat_command_roll_help")));
                return false;
            }                           
        
            const uint uiRand(M_Randnum(AtoI(vsTokens[1]), AtoI(vsTokens[2])));
            
            const tstring sRollMessage = ChatManager.Translate(_T("chat_roll_message"), _T("player"), Game.GetLocalPlayer()->GetName(), _T("low"), XtoA(AtoI(vsTokens[1])), _T("high"), XtoA(AtoI(vsTokens[2])), _T("number"), XtoA(uiRand));
            
            if (!sRollMessage.empty())
            {
                CBufferDynamic buffer;
                buffer << GAME_CMD_CHAT_ROLL << TStringToUTF8(sRollMessage.substr(0,150)) << byte(0);
                GameClient.SendGameData(buffer, true);              
                return true;            
            }
            else
            {
                ChatManager.AddIRCChatMessage(CHAT_MESSAGE_ADD, ChatManager.Translate(_T("chat_command_roll_help")));
                return false;
            }                           
        }
        else
        {
            ChatManager.AddIRCChatMessage(CHAT_MESSAGE_ADD, ChatManager.Translate(_T("chat_command_roll_help")));
            return false;
        }   
    }
    else if (CompareNoCase(vsTokens[0], ChatManager.Translate(_T("chat_command_emote"))) == 0 || CompareNoCase(vsTokens[0], ChatManager.Translate(_T("chat_command_emote_short"))) == 0)
    {
        ChatManager.AddChatHistory(sText);

        if (vsTokens.size() >= 2)
        {       
            const tstring sEmoteMessage = Game.GetLocalPlayer()->GetName() + _T(" ") + ConcatinateArgs(vsTokens.begin() + 1, vsTokens.end());
                        
            if (!sEmoteMessage.empty())
            {
                CBufferDynamic buffer;
                buffer << GAME_CMD_CHAT_EMOTE << TStringToUTF8(sEmoteMessage.substr(0,150)) << byte(0);
                GameClient.SendGameData(buffer, true);              
                return true;            
            }
            else
            {
                ChatManager.AddIRCChatMessage(CHAT_MESSAGE_ADD, ChatManager.Translate(_T("chat_command_emote_help")));
                return false;
            }                           
        }
        else
        {
            ChatManager.AddIRCChatMessage(CHAT_MESSAGE_ADD, ChatManager.Translate(_T("chat_command_emote_help")));
            return false;
        }   
    }
    else if (CompareNoCase(vsTokens[0], ChatManager.Translate(_T("chat_command_matchup"))) == 0 || CompareNoCase(vsTokens[0], ChatManager.Translate(_T("chat_command_matchup_short"))) == 0)
    {           
        if (GameClient.GetCurrentGamePointer()->GetGamePhase() >= GAME_PHASE_PRE_MATCH && GameClient.GetCurrentGamePointer()->GetGamePhase() <= GAME_PHASE_ENDED)
        {
            ChatManager.AddChatHistory(sText);
            
            tstring sMatchupInfo;
            
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, _T("Current Matchup:"));
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, _T("----------------------------"));
            
            const PlayerMap &mapPlayers(GameClient.GetCurrentGamePointer()->GetPlayerMap());
            
            for (PlayerMap_cit it(mapPlayers.begin()); it != mapPlayers.end(); ++it)
            {
                sMatchupInfo = TSNULL;
                
                if (it->second->GetTeam() == TEAM_SPECTATOR)
                    continue;

                if (it->second->GetSelectedHero() == INVALID_ENT_TYPE)
                    continue;
                                
                CHeroDefinition *pHeroDef(EntityRegistry.GetDefinition<CHeroDefinition>(it->second->GetSelectedHero()));
                
                if (pHeroDef == nullptr)
                    continue;
                            
                if (it->second->GetTeam() == TEAM_1)
                {
                    sMatchupInfo += _T("Team 1: ");
                }
                else if (it->second->GetTeam() == TEAM_2)
                {
                    sMatchupInfo += _T("Team 2: ");
                }
                else if (it->second->IsReferee())
                {
                    sMatchupInfo += _T("Referee: ");
                }
                else if (it->second->GetTeam() == TEAM_SPECTATOR)
                {
                    sMatchupInfo += _T("Spectator: ");
                }
                    
                sMatchupInfo += GetInlineColorString<tstring>(it->second->GetColor()) + _T("Level: ") + XtoA(it->second->GetHero()->GetLevel()) + _T(" ") + it->second->GetName() + _T(" (") + pHeroDef->GetDisplayName() + _T(")^*") + newl;
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMatchupInfo);
            }       
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, _T("----------------------------"));
            return true;
        }
        else
        {
            ChatManager.AddIRCChatMessage(CHAT_MESSAGE_ADD, ChatManager.Translate(_T("chat_command_matchup_help")));
            return false;
        }
    }
    else if (CompareNoCase(vsTokens[0], ChatManager.Translate(_T("chat_command_gameinfo"))) == 0 || CompareNoCase(vsTokens[0], ChatManager.Translate(_T("chat_command_gameinfo_short"))) == 0)
    {
        ChatManager.AddChatHistory(sText);
        
        CGameInfo *pGameInfo(Game.GetGameInfo());
                
        if (pGameInfo == nullptr)
            return false;
    
        ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, _T("Current Game Info:"));
        ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, _T("----------------------------"));
        ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, _T("Match Name: ") + pGameInfo->GetGameName());
        ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, _T("Match ID: ") + XtoA(pGameInfo == nullptr ? -1 : int(pGameInfo->GetMatchID())));
        
        const PlayerMap &mapPlayers(GameClient.GetPlayerMap());
        tstring sHost(_T(""));
        
        for (PlayerMap_cit it(mapPlayers.begin()); it != mapPlayers.end(); ++it)
        {
            if (it->second->HasFlags(PLAYER_FLAG_HOST))
            {
                sHost = it->second->GetName();
                break;
            }
        }
        
        ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, _T("Host: ") + sHost);
        ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, _T("Server Name: ") + pGameInfo->GetServerName());
        ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, _T("Server Version: ") + GameClient.GetStateString(STATE_STRING_SERVER_INFO).GetString(_T("svr_version")));
        ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, _T("Min/Max PSR: ") + XtoA(pGameInfo->GetMinPSR()) + _T(" / ") + XtoA(pGameInfo->GetMaxPSR()));
        ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, _T("Current Players: " ) + XtoA(GameClient.GetConnectedClientCount()) + _T(" / ") + (pGameInfo->HasFlags(GAME_FLAG_SOLO) ? XtoA(1) : XtoA(pGameInfo->GetTeamSize() * 2)));
        ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, _T("Map: ") + (GameClient.GetWorldPointer() == nullptr ? TSNULL : GameClient.GetWorldPointer()->GetFancyName()));
        ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, _T("Game Mode: ") + CGameInfo::GetGameModeString(pGameInfo->GetGameMode()));
        ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, _T("Game Options: ") + CGameInfo::GetGameOptionsString(pGameInfo->GetGameOptions()));
        
        tstring sOtherOptions(_T("Other Options: "));
        bool bNoStats(false);
        bool bPrivateGame(false);
        bool bNoLeavers(false);
    
        if ((GameClient.GetHostFlags() & HOST_SERVER_NO_STATS) != 0)
        {
            bNoStats = true;
            sOtherOptions += _T("No Stats");
        }

        if (int(GameClient.GetServerAccess()))
        {
            bPrivateGame = true;
            if (bNoStats)
                sOtherOptions += _T(", ");
                
            sOtherOptions += _T("Private Game");
        }
        
        if ((GameClient.GetHostFlags() & HOST_SERVER_NO_LEAVER) != 0)
        {
            bNoLeavers = true;
            if (bNoStats || bPrivateGame)
                sOtherOptions += _T(", ");
            
            sOtherOptions += _T("No Leavers");
        }
        
        if(!bNoStats && !bPrivateGame && !bNoLeavers)
            sOtherOptions += _T("None");
                    
        ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sOtherOptions);            
        ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, _T("----------------------------"));       
        return true;
    }
    else if (CompareNoCase(vsTokens[0], ChatManager.Translate(_T("chat_command_ping"))) == 0)
    {
        ChatManager.AddChatHistory(sText);
        
        CPlayer *pLocalPlayer(GameClient.GetLocalPlayer());

        if (pLocalPlayer == nullptr)
            return false;
            
        ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, ChatManager.Translate(_T("chat_current_ping"), _T("ping"), XtoA(pLocalPlayer->GetPing())));
        return true;
    }
    else if(CompareNoCase(vsTokens[0], ChatManager.Translate(_T("chat_command_ping_all"))) == 0 ||
        CompareNoCase(vsTokens[0], ChatManager.Translate(_T("chat_command_ping_all_short"))) == 0)
    {
        ChatManager.AddChatHistory(sText);
        
        CBufferFixed<1> buffer;
        buffer << GAME_CMD_PING_ALL;
        GameClient.SendGameData(buffer, true);              
    }
    else if (CompareNoCase(vsTokens[0], ChatManager.Translate(_T("chat_command_weather"))) == 0)
    {
        ChatManager.AddChatHistory(sText);

        vector<SWeatherInfo> vWeather;
        GameClient.CalcAvailableWeatherEffects(vWeather);

        if (vsTokens.size() >= 2)
        {
            const tstring  sWeatherName(ConcatinateArgs(vsTokens.begin() + 1, vsTokens.end()));
            const tstring& sWeatherLower(LowerString(sWeatherName));
            for (size_t i(0); i < vWeather.size(); ++i)
            {
                SWeatherInfo& cInfo(vWeather[i]);

                if ((cInfo.sKeyName == sWeatherLower)
                    || LowerString(cInfo.sLocalizedName) == sWeatherLower)
                {
                    cg_current_weather = cInfo.sLocalizedName;
                    cg_current_weather.SetModified(true);
                    ChatManager.AddIRCChatMessage(CHAT_MESSAGE_ADD, ChatManager.Translate(_T("chat_command_weather_selected"), _T("name"), cInfo.sLocalizedName));
                    return true;
                }
            }
        }

        // Show list of weather effects
        ChatManager.AddIRCChatMessage(CHAT_MESSAGE_ADD, ChatManager.Translate(_T("chat_command_weather_intro")));
        for (size_t i(0); i < vWeather.size(); ++i)
        {
            SWeatherInfo& cInfo(vWeather[i]);

            // Display each weather command
            ChatManager.AddIRCChatMessage(CHAT_MESSAGE_ADD, ChatManager.Translate(_T("chat_command_weather_entry"), _T("name"), cInfo.sLocalizedName));
        }
        return true;
    }
    // If they entered a command, pass it to IRC
    else if (sText[0] == _T('/'))
    {
        ChatManager.SubmitChatMessage(sText, uint(-1));
        return true;
    }
    
    return true;
}


/*--------------------
  AllChat
  --------------------*/
CMD(AllChat)
{
    if (vArgList.empty())
        return false;

    const tstring &sText(ConcatinateArgs(vArgList));
    
    if (sText.empty())
        return false;
        
    if (sText[0] == _T('/'))
    {
        return HandleGameSlashCommands(sText);      
    }

    ChatManager.AddChatHistory(sText);
    
    CBufferDynamic buffer;
    buffer << GAME_CMD_CHAT_ALL << TStringToUTF8(sText.substr(0, 150)) << byte(0);
    GameClient.SendGameData(buffer, true);

    ChatManager.PlaySound(_T("SentChannelMessage"));

    return true;
}

UI_VOID_CMD(AllChat, 1)
{
    cmdAllChat(vArgList[0]->Evaluate());
}


/*--------------------
  TeamChat
  --------------------*/
CMD(TeamChat)
{
    if (vArgList.empty())
        return false;

    const tstring &sText(ConcatinateArgs(vArgList));

    if (sText.empty())
        return false;

    if (sText[0] == _T('/'))
    {
        return HandleGameSlashCommands(sText);      
    }

    ChatManager.AddChatHistory(sText);

    CPlayer *pPlayer(Game.GetLocalPlayer());

    if (pPlayer == nullptr)
        return false;

    CTeamInfo *pTeam(Game.GetTeam(pPlayer->GetTeam()));

    if (pTeam == nullptr || pTeam->GetTeamID() == TEAM_INVALID)
    {
        ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, _T("^900* You cannot team chat, you are not currently on a team!"));
        return true;
    }

    CBufferDynamic buffer;
    buffer << GAME_CMD_CHAT_TEAM << TStringToUTF8(sText.substr(0, 150)) << byte(0);
    GameClient.SendGameData(buffer, true);

    ChatManager.PlaySound(_T("SentChannelMessage"));

    return true;
}

UI_VOID_CMD(TeamChat, 1)
{
    cmdTeamChat(vArgList[0]->Evaluate());
}


/*--------------------
  LocalChat
  --------------------*/
CMD(LocalChat)
{
    if (vArgList.empty())
        return false;

    const tstring &sText(ConcatinateArgs(vArgList));

    if (sText.empty())
        return false;

    ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sText);

    return true;
}

UI_VOID_CMD(LocalChat, 1)
{
    cmdLocalChat(vArgList[0]->Evaluate());
}


/*--------------------
  SelectHero
  --------------------*/
UI_VOID_CMD(SelectHero, 0)
{
    CPlayer *pPlayer(GameClient.GetLocalPlayer());
    if (pPlayer == nullptr)
        return;

    CClientCommander *pCommander(GameClient.GetClientCommander());
    if (pCommander == nullptr)
        return;

    pCommander->SelectEntity(pPlayer->GetHeroIndex());
}


/*--------------------
  eventStartEffect
  --------------------*/
EVENT_CMD(StartEffect)
{
    if (vArgList.size() < 1)
        return false;

    GameClient.StartEffect(vArgList[0], vArgList.size() > 1 ? AtoI(vArgList[1]) : -1, iTimeNudge);
    return true;
}


/*--------------------
  eventStopEffect
  --------------------*/
EVENT_CMD(StopEffect)
{
    if (vArgList.size() < 1)
        return false;

    GameClient.StopEffect(AtoI(vArgList[0]) + NUM_EFFECT_CHANNELS + MAX_INVENTORY);
    return true;
}


/*--------------------
  eventPlaySound <filename> <falloff> <volume> <channel> <fadein ms> <fadeout start time ms> <fadeout ms> <frequency>
  --------------------*/
EVENT_CMD(PlaySound)
{
    if (vArgList.size() < 1)
        return false;

    if (vArgList.size() > 7 && (AtoF(vArgList[7]) * 1000 >= M_Randnum(0, 1000)))
        return true;


    GameClient.PlaySound
    (
        vArgList[0],
        vArgList.size() > 3 ? AtoI(vArgList[3]) : -1,
        vArgList.size() > 1 ? AtoF(vArgList[1]) : -1,
        vArgList.size() > 2 ? AtoF(vArgList[2]) : 1.0f,
        0,
        vArgList.size() > 4 ? AtoI(vArgList[4]) : 0,
        vArgList.size() > 5 ? AtoI(vArgList[5]) : 0,
        vArgList.size() > 6 ? AtoI(vArgList[6]) : 0
    );
    return true;
}


/*--------------------
  eventPlaySoundLooping

  PlaySoundLooping <filename> <falloff> <volume> <channel> <fadein ms> <fadeout ms> <override channel> <speed up time> <speed 1> <speed 2> <slow down time>
  --------------------*/
EVENT_CMD(PlaySoundLooping)
{
    if (vArgList.size() < 1)
        return false;

    GameClient.PlaySound
    (
        vArgList[0],
        vArgList.size() > 3 ? AtoI(vArgList[3]) : -1,
        vArgList.size() > 1 ? AtoF(vArgList[1]) : -1,
        vArgList.size() > 2 ? AtoF(vArgList[2]) : 1.0f,
        SND_LOOP,
        vArgList.size() > 4 ? AtoI(vArgList[4]) : 0,
        0,
        vArgList.size() > 5 ? AtoI(vArgList[5]) : 0,
        vArgList.size() > 6 ? AtoB(vArgList[6]) : true,
        vArgList.size() > 7 ? AtoI(vArgList[7]) : 0,
        vArgList.size() > 8 ? AtoF(vArgList[8]) : 1.0f,
        vArgList.size() > 9 ? AtoF(vArgList[9]) : 1.0f,
        vArgList.size() > 10 ? AtoI(vArgList[10]) : 0
    );
    return true;
}


/*--------------------
  eventPlaySoundStationary
  --------------------*/
EVENT_CMD(PlaySoundStationary)
{
    if (vArgList.size() < 1)
        return false;

    GameClient.PlaySoundStationary
    (
        vArgList[0],
        vArgList.size() > 3 ? AtoI(vArgList[3]) : -1,
        vArgList.size() > 1 ? AtoF(vArgList[1]) : -1,
        vArgList.size() > 2 ? AtoF(vArgList[2]) : 1.0f
    );
    return true;
}


/*--------------------
  eventPlaySoundLinear <filename> <falloffstart> <falloffend> <volume> <channel> <fadein ms> <fadeout start time ms> <fadeout ms> <frequency>
  --------------------*/
EVENT_CMD(PlaySoundLinear)
{
    if (vArgList.size() < 1)
        return false;

    if (vArgList.size() > 7 && (AtoF(vArgList[7]) * 1000 >= M_Randnum(0, 1000)))
        return true;

    GameClient.PlaySound
    (
        vArgList[0],
        vArgList.size() > 4 ? AtoI(vArgList[4]) : -1,
        vArgList.size() > 1 ? AtoF(vArgList[1]) : -1,
        vArgList.size() > 3 ? AtoF(vArgList[3]) : 1.0f,
        SND_LINEARFALLOFF,
        vArgList.size() > 5 ? AtoI(vArgList[5]) : 0,
        vArgList.size() > 6 ? AtoI(vArgList[6]) : 0,
        vArgList.size() > 7 ? AtoI(vArgList[7]) : 0,
        true,
        0,
        1.0f,
        1.0f,
        0,
        vArgList.size() > 2 ? AtoF(vArgList[2]) : -1
    );

    return true;
}


/*--------------------
  eventPlaySoundLoopingLinear

  PlaySoundLoopingLinear <filename> <falloffstart> <falloffend> <volume> <channel> <fadein ms> <fadeout ms> <override channel> <speed up time> <speed 1> <speed 2> <slow down time>
  --------------------*/
EVENT_CMD(PlaySoundLoopingLinear)
{
    if (vArgList.size() < 1)
        return false;

    GameClient.PlaySound
    (
        vArgList[0],
        vArgList.size() > 4 ? AtoI(vArgList[4]) : -1,
        vArgList.size() > 1 ? AtoF(vArgList[1]) : -1,
        vArgList.size() > 3 ? AtoF(vArgList[3]) : 1.0f,
        SND_LINEARFALLOFF | SND_LOOP,
        vArgList.size() > 5 ? AtoI(vArgList[5]) : 0,
        0,
        vArgList.size() > 6 ? AtoI(vArgList[6]) : 0,
        vArgList.size() > 7 ? AtoB(vArgList[7]) : true,
        vArgList.size() > 8 ? AtoI(vArgList[8]) : 0,
        vArgList.size() > 9 ? AtoF(vArgList[9]) : 1.0f,
        vArgList.size() > 10 ? AtoF(vArgList[10]) : 1.0f,
        vArgList.size() > 11 ? AtoI(vArgList[11]) : 0,
        vArgList.size() > 2 ? AtoF(vArgList[2]) : -1
    );
    return true;
}


/*--------------------
  eventPlaySoundStationaryLinear
  --------------------*/
EVENT_CMD(PlaySoundStationaryLinear)
{
    if (vArgList.size() < 1)
        return false;

    GameClient.PlaySoundStationary
    (
        vArgList[0],
        vArgList.size() > 3 ? AtoI(vArgList[3]) : -1,
        vArgList.size() > 1 ? AtoF(vArgList[1]) : -1,
        vArgList.size() > 2 ? AtoF(vArgList[2]) : 1.0f
    );
    return true;
}


/*--------------------
  eventStopSound
  --------------------*/
EVENT_CMD(StopSound)
{
    if (vArgList.size() < 1)
        return false;

    GameClient.StopSound(AtoI(vArgList[0]));
    return true;
}


/*--------------------
  PrecacheSound
  --------------------*/
EVENT_CMD(PrecacheSound)
{
    assert(!"is PrecacheSound ever used?");
    if (!vArgList.empty())
    {
        Console << _T("Precaching ") << vArgList[0] << newl;
        K2_WITH_GAME_RESOURCE_SCOPE()
            g_ResourceManager.Register(K2_NEW(ctx_Sound,   CSample)(vArgList[0], 0), RES_SAMPLE);
    }
    return true;
}


/*--------------------
  MinimapLeftClick
  --------------------*/
CMD(MinimapLeftClick)
{
    if (vArgList.size() < 2)
        return false;

    CClientCommander *pCommander(GameClient.GetClientCommander());

    if (pCommander == nullptr)
        return false;

    CVec2f v2Pos(AtoF(vArgList[0]) * GameClient.GetWorldWidth(), AtoF(vArgList[1]) * GameClient.GetWorldHeight());

    bool bUnitPingFailed(false);

    if (GameClient.GetMinimapHoverUnit() != -1 && pCommander->GetPingEnabled() && pCommander->GetPingKeyDown())
    {
        CPlayer* pLocalPlayer(Game.GetLocalPlayer());

        if (!pLocalPlayer)
            bUnitPingFailed = true;

        if (!bUnitPingFailed && pLocalPlayer->GetTeam() == TEAM_SPECTATOR)
            bUnitPingFailed = true;

        IUnitEntity *pHover(GameClient.GetUnitEntity(GameClient.GetMinimapHoverUnit()));

        if (!bUnitPingFailed && !(pHover && 
            ((pHover->IsBuilding() && !pHover->GetAsBuilding()->GetNoAltClickPing() && (pHover->GetTeam() == pLocalPlayer->GetTeam() || pLocalPlayer->IsEnemy(pHover)) && !pHover->GetAsBuilding()->GetIsShop() && !pHover->GetAsBuilding()->GetNoAltClickPing()) || 
            (pHover->IsHero() && pLocalPlayer->IsEnemy(pHover)))))
        {
            bUnitPingFailed = true;
        }

        if (!bUnitPingFailed)
        {
            pCommander->Ping();
            return true;        // Return if Alt ping a unit
        }
    }

    if (pCommander->GetPingEnabled() && pCommander->GetPingKeyDown())
    {
        // Return if Alt ping a unit
        pCommander->Ping();
        return true;
    }

    pCommander->MinimapPrimaryClick(v2Pos);
    
    return true;
}


#if 0
/*--------------------
  MinimapDraw
  --------------------*/
CMD(MinimapDraw)
{
    if (vArgList.size() < 2)
        return false;

    CBufferFixed<9> buffer;
    buffer << GAME_CMD_MINIMAP_DRAW << AtoF(vArgList[0]) << AtoF(vArgList[1]);
    GameClient.SendGameData(buffer, false);

    return true;
}
#endif


/*--------------------
  MinimapPing
  --------------------*/
CMD(MinimapPing)
{
    if (vArgList.size() < 2)
        return false;

    GameClient.MinimapPing(byte(AtoF(vArgList[0]) * UCHAR_MAX), byte(AtoF(vArgList[1]) * UCHAR_MAX));
    return true;
}


/*--------------------
  ActivateTool
  --------------------*/
UI_VOID_CMD(ActivateTool, 1)
{
    if (GameClient.GetClientCommander() == nullptr)
        return;
    GameClient.GetClientCommander()->ActivateTool(AtoI(vArgList[0]->Evaluate()), false, GameClient.GetClientCommander()->GetSelectedControlEntity(), true);
}


/*--------------------
  ActivateSharedTool
  --------------------*/
UI_VOID_CMD(ActivateSharedTool, 1)
{
    if (GameClient.GetClientCommander() == nullptr)
        return;
    GameClient.GetClientCommander()->ActivateTool(AtoI(vArgList[0]->Evaluate()), false, GameClient.GetClientCommander()->GetSelectedInfoEntity(), true);
}


/*--------------------
  ActivateToolSecondary
  --------------------*/
UI_VOID_CMD(ActivateToolSecondary, 1)
{
    if (GameClient.GetClientCommander() == nullptr)
        return;
    GameClient.GetClientCommander()->ActivateTool(AtoI(vArgList[0]->Evaluate()), true, GameClient.GetClientCommander()->GetSelectedControlEntity(), true);
}


/*--------------------
  ActivateSharedToolSecondary
  --------------------*/
UI_VOID_CMD(ActivateSharedToolSecondary, 1)
{
    if (GameClient.GetClientCommander() == nullptr)
        return;
    GameClient.GetClientCommander()->ActivateTool(AtoI(vArgList[0]->Evaluate()), true, GameClient.GetClientCommander()->GetSelectedInfoEntity(), true);
}


/*--------------------
  Vote
  --------------------*/
CMD(Vote)
{
    if (vArgList.empty())
        return false;

    byte yVote(AtoB(vArgList[0]) ? VOTE_YES : VOTE_NO);

    CBufferFixed<2> buffer;
    buffer << GAME_CMD_VOTE << yVote;
    GameClient.SendGameData(buffer, true);
    return true;
}

UI_VOID_CMD(Vote, 1)
{
    cmdVote(vArgList[0]->Evaluate());
}


/*--------------------
  CallVote
  --------------------*/
CMD(CallVote)
{
    if (vArgList.empty())
        return false;

    EVoteType eVote(GetVoteTypeFromString(vArgList[0]));
    if (eVote == VOTE_TYPE_INVALID)
        return false;

    int iParam(-1);
    if (eVote == VOTE_TYPE_KICK_AFK || eVote == VOTE_TYPE_KICK)
    {
        if (vArgList.size() > 1)
        {
            CPlayer *pPlayer(GameClient.GetPlayerByName(vArgList[1]));
            if (pPlayer == nullptr)
                pPlayer = GameClient.GetPlayer(AtoI(vArgList[1]));
            if (pPlayer != nullptr)
                iParam = pPlayer->GetClientNumber();
        }

        if (iParam == -1)
            return false;
    }

    CBufferFixed<6> buffer;
    buffer << GAME_CMD_CALLVOTE << byte(eVote) << iParam;
    GameClient.SendGameData(buffer, true);
    return true;
}

UI_VOID_CMD(CallVote, 1)
{
    cmdCallVote(vArgList[0]->Evaluate(), vArgList.size() > 1 ? vArgList[1]->Evaluate() : TSNULL);
}


/*--------------------
  Unpause
  --------------------*/
CMD(Unpause)
{
    CBufferFixed<1> buffer;
    buffer << GAME_CMD_UNPAUSE;
    GameClient.SendGameData(buffer, true);
    return true;
}

UI_VOID_CMD(Unpause, 0)
{
    cmdUnpause();
}


/*--------------------
  Ready
  --------------------*/
CMD(Ready)
{
    CBufferFixed<1> buffer;
    buffer << GAME_CMD_READY;
    GameClient.SendGameData(buffer, true);
    return true;
}

UI_VOID_CMD(Ready, 0)
{
    cmdReady();
}


/*--------------------
  UnReady
  --------------------*/
/*
CMD(UnReady)
{
    CBufferFixed<1> buffer;
    buffer << GAME_CMD_UNREADY;
    GameClient.SendGameData(buffer, true);
    return true;
}

UI_VOID_CMD(UnReady, 0)
{
    cmdUnReady();
}
*/


/*--------------------
  RepickHero
  --------------------*/
CMD(RepickHero)
{
    CBufferFixed<1> buffer;
    buffer << GAME_CMD_REPICK_HERO;
    GameClient.SendGameData(buffer, true);
    return true;
}

UI_VOID_CMD(RepickHero, 0)
{
    cmdRepickHero();
}


/*--------------------
  SwapHeroRequest
  --------------------*/
CMD(SwapHeroRequest)
{
    if (vArgList.empty())
        return false;

    CBufferFixed<2> buffer;
    buffer << GAME_CMD_SWAP_HERO_REQUEST << byte(AtoI(vArgList[0]));
    GameClient.SendGameData(buffer, true);
    return true;
}

UI_VOID_CMD(SwapHeroRequest, 1)
{
    cmdSwapHeroRequest(vArgList[0]->Evaluate());
}


/*--------------------
  RequestMatchStart
  --------------------*/
CMD(RequestMatchStart)
{
    CPlayer *pPlayer(Game.GetLocalPlayer());
    
    if (pPlayer != nullptr && pPlayer->HasFlags(PLAYER_FLAG_HOST))
    {       
        CBufferFixed<1> buffer;
        buffer << GAME_CMD_REQUEST_MATCH_START;
        GameClient.SendGameData(buffer, true);
        return true;
    }
    
    return false;
}

UI_VOID_CMD(RequestMatchStart, 0)
{
    cmdRequestMatchStart();
}


/*--------------------
  RequestMatchCancel
  --------------------*/
CMD(RequestMatchCancel)
{
    CPlayer *pPlayer(Game.GetLocalPlayer());
    
    if (pPlayer != nullptr && pPlayer->HasFlags(PLAYER_FLAG_HOST))
    {       
        CBufferFixed<1> buffer;
        buffer << GAME_CMD_REQUEST_MATCH_CANCEL;
        GameClient.SendGameData(buffer, true);
        return true;
    }
    
    return false;
}

UI_VOID_CMD(RequestMatchCancel, 0)
{
    cmdRequestMatchCancel();
}


/*--------------------
  RayTrace
  --------------------*/
CMD(RayTrace)
{
    // Determine size of image to produce
    int iWidth(160);
    int iHeight(120);
    if (vArgList.size() >= 2)
    {
        iWidth = AtoI(vArgList[0]);
        iHeight = AtoI(vArgList[1]);
    }
    else if (!vArgList.empty())
    {
        iWidth = AtoI(vArgList[0]);
        iHeight = INT_ROUND(iWidth / Vid.GetAspect());
    }
    iWidth = CLAMP(iWidth, 10, Vid.GetScreenW());
    iHeight = CLAMP(iHeight, 10, Vid.GetScreenH());

    Console << _T("Tracing scene at a resolution of ") << iWidth << _T("x") << iHeight << newl;

    float fRatioX = Vid.GetScreenW() / static_cast<float>(iWidth);
    float fRatioY = Vid.GetScreenH() / static_cast<float>(iHeight);

    CBitmap bmp;
    bmp.Alloc(iWidth, iHeight, BITMAP_RGBA);

    uint uiMsec(K2System.Milliseconds());

    CVec4f  colors[] =
    {
        CVec4f(1.0f, 0.5f, 0.5f, 1.0f),
        CVec4f(0.0f, 1.0f, 0.5f, 1.0f),
        CVec4f(0.5f, 0.5f, 1.0f, 1.0f),
        CVec4f(0.5f, 1.0f, 1.0f, 1.0f),
        CVec4f(1.0f, 0.5f, 1.0f, 1.0f),
        CVec4f(1.0f, 1.0f, 0.5f, 1.0f)
    };

    CWorld *pWorld(GameClient.GetWorldPointer());
    CCamera *pCamera(GameClient.GetCamera());
    bool bTraceBox(vArgList.size() > 2);
    float fBoxSize(bTraceBox ? AtoF(vArgList[2]) : 0.0f);
    CBBoxf bbBounds(CVec3f(-fBoxSize, -fBoxSize, -fBoxSize), CVec3f(fBoxSize, fBoxSize, fBoxSize));
    int iIgnoreSurface(SURF_BLOCKER | SURF_TERRAIN);

    if (fBoxSize == 0.0f)
        iIgnoreSurface |= SURF_HULL | SURF_SHIELD;

    for (int y(0); y < iHeight; ++y)
    {
        for (int x(0); x < iWidth; ++x)
        {
            // Set up the trace
            CVec3f v3Dir(pCamera->ConstructRay(x * fRatioX, y * fRatioY));
            CVec3f v3End(M_PointOnLine(pCamera->GetOrigin(), v3Dir, 100000.0f));
            STraceInfo trace;

            // Perform the trace
            if (bTraceBox)
                pWorld->TraceBox(trace, pCamera->GetOrigin(), v3End, bbBounds, iIgnoreSurface);
            else
                pWorld->TraceLine(trace, pCamera->GetOrigin(), v3End, iIgnoreSurface);

            // No hit
            if (trace.fFraction >= 1.0f)
            {
                bmp.SetPixel4b(x, y, 255, 0, 0, 255);
                continue;
            }

            float fDot(DotProduct(-v3Dir, trace.plPlane.v3Normal));
            if (fDot < 0.0f)
            {
                bmp.SetPixel4b(x, y, 255, 0, 255, 255);
            }
            else if (trace.uiEntityIndex != INVALID_INDEX)
            {
                float fLight(CLAMP(fDot, 0.0f, 1.0f));
                CVec4f  v4Color(colors[trace.uiEntityIndex % 6]);
                bmp.SetPixel4b(x, y, byte(v4Color[R] * fLight * 255), byte(v4Color[G] * fLight * 255), byte(v4Color[B] * fLight * 255), 255);
            }
            else
            {
                byte yLight(static_cast<byte>(MAX(fDot, 0.0f) * 255));
                bmp.SetPixel4b(x, y, yLight, yLight, (trace.uiSurfFlags & SURF_TERRAIN) ? yLight : 0, 255);
            }
        }
    }

    Console << _T("Raytrace took ") << MsToSec(K2System.Milliseconds() - uiMsec) << _T(" seconds") << newl;
    bmp.WritePNG(_T("~/raytrace.png"));
    bmp.Free();
    return true;
}


/*--------------------
  UpdateInterface
  --------------------*/
CMD(UpdateInterface)
{
    GameClient.ForceInterfaceRefresh();
    return true;
}


/*--------------------
  SelectUnit
  --------------------*/
UI_VOID_CMD(SelectUnit, 1)
{
    CClientCommander *pCommander(GameClient.GetClientCommander());
    if (pCommander == nullptr)
        return;

    pCommander->SelectEntity(vArgList[0]->EvaluateInteger());
}


/*--------------------
  SetReplayClient
  --------------------*/
CMD(SetReplayClient)
{
    if (vArgList.size() < 1)
        return false;

    GameClient.SetReplayClient(AtoI(vArgList[0]));

    return true;
}


/*--------------------
  SetReplayClient
  --------------------*/
UI_VOID_CMD(SetReplayClient, 1)
{
    cmdSetReplayClient(vArgList[0]->Evaluate());
}


/*--------------------
  NextReplayClient
  --------------------*/
CMD(NextReplayClient)
{
    GameClient.NextReplayClient();
    return true;
}

UI_VOID_CMD(NextReplayClient, 0)
{
    GameClient.NextReplayClient();
}


/*--------------------
  PrevReplayClient
  --------------------*/
CMD(PrevReplayClient)
{
    GameClient.PrevReplayClient();
    return true;
}

UI_VOID_CMD(PrevReplayClient, 0)
{
    GameClient.PrevReplayClient();
}


/*--------------------
  RequestServerStatus
  --------------------*/
CMD(RequestServerStatus)
{
    CBufferFixed<1> buffer;
    buffer << GAME_CMD_SERVER_STATUS;
    GameClient.SendGameData(buffer, true);
    return true;
}


/*--------------------
  StartClientGameEffect
  --------------------*/
CMD(StartClientGameEffect)
{
    if (vArgList.size() < 1)
        return false;

    ResHandle hEffect(INVALID_RESOURCE);
    K2_WITH_GAME_RESOURCE_SCOPE()
    {
        hEffect = g_ResourceManager.Register(vArgList[0], RES_EFFECT);
        if (hEffect == INVALID_RESOURCE)
            return false;
    }
    GameClient.StartClientGameEffect(hEffect, vArgList.size() > 1 ? AtoI(vArgList[1]) + NUM_CLIENT_GAME_EFFECT_CHANNELS : -1, 0, V3_ZERO);
    return true;
}


/*--------------------
  StopClientGameEffect
  --------------------*/
CMD(StopClientGameEffect)
{
    if (vArgList.size() < 1)
        return false;

    GameClient.StopClientGameEffect(AtoI(vArgList[0]) + NUM_CLIENT_GAME_EFFECT_CHANNELS);
    return true;
}


/*--------------------
  PlayClientGameSound

  PlayClientGameSound <filename> <volume> <channel> <fadein ms> <fadeout start time ms> <fadeout ms> <frequency>
  --------------------*/
CMD(PlayClientGameSound)
{
    if (vArgList.size() < 1)
        return false;

    if (vArgList.size() > 6 && (AtoF(vArgList[6]) * 1000 >= M_Randnum(0, 1000)))
        return true;


    GameClient.PlayClientGameSound(vArgList[0], vArgList.size() > 2 ? AtoI(vArgList[2]) : -1, vArgList.size() > 1 ? AtoF(vArgList[1]) : 1.0f, 0, vArgList.size() > 3 ? AtoI(vArgList[3]) : 0, vArgList.size() > 4 ? AtoI(vArgList[4]) : 0, vArgList.size() > 5 ? AtoI(vArgList[5]) : 0);
    return true;
}


/*--------------------
  PlayClientGameSoundLooping

  PlayClientGameSoundLooping <filename> <volume> <channel> <fadein ms> <fadeout ms> <override channel> <speed up time> <speed 1> <speed 2> <slow down time>
  --------------------*/
CMD(PlayClientGameSoundLooping)
{
    if (vArgList.size() < 1)
        return false;

    GameClient.PlayClientGameSound(vArgList[0], vArgList.size() > 2 ? AtoI(vArgList[2]) : -1, vArgList.size() > 1 ? AtoF(vArgList[1]) : 1.0f, SND_LOOP, vArgList.size() > 3 ? AtoI(vArgList[3]) : 0, 0, vArgList.size() > 4 ? AtoI(vArgList[4]) : 0, vArgList.size() > 5 ? AtoB(vArgList[5]) : true, vArgList.size() > 6 ? AtoI(vArgList[6]) : 0, vArgList.size() > 7 ? AtoF(vArgList[7]) : 1.0f, vArgList.size() > 8 ? AtoF(vArgList[8]) : 1.0f, vArgList.size() > 9 ? AtoI(vArgList[9]) : 0);
    return true;
}


/*--------------------
  StopClientGameSound
  --------------------*/
CMD(StopClientGameSound)
{
    if (vArgList.size() < 1)
        return false;

    GameClient.StopClientGameSound(AtoI(vArgList[0]));
    return true;
}


/*--------------------
  SetClientAngles
  --------------------*/
CMD(SetClientAngles)
{
    if (vArgList.size() < 3)
        return false;

    GameClient.GetCurrentSnapshot()->AdjustCameraAngles(CVec3f(AtoF(vArgList[0]),AtoF(vArgList[1]),AtoF(vArgList[2])));

    return true;
}


/*--------------------
  MinimapClick
  --------------------*/
UI_CMD(MinimapClick, 2)
{
    CClientCommander *pCommander(GameClient.GetClientCommander());
    if (pCommander == nullptr)
        return XtoA(false);

    CVec2f v2Pos(AtoF(vArgList[0]->Evaluate()) * GameClient.GetWorldWidth(), AtoF(vArgList[1]->Evaluate()) * GameClient.GetWorldHeight());

    bool bUnitPingFailed(false);

    if (GameClient.GetMinimapHoverUnit() != -1 && pCommander->GetPingEnabled() && pCommander->GetPingKeyDown())
    {
        CPlayer* pLocalPlayer(Game.GetLocalPlayer());

        if (!pLocalPlayer)
            bUnitPingFailed = true;

        if (!bUnitPingFailed && pLocalPlayer->GetTeam() == TEAM_SPECTATOR)
            bUnitPingFailed = true;

        IUnitEntity *pHover(GameClient.GetUnitEntity(GameClient.GetMinimapHoverUnit()));

        if (!bUnitPingFailed && !(pHover && 
            ((pHover->IsBuilding() && !pHover->GetAsBuilding()->GetNoAltClickPing() && (pHover->GetTeam() == pLocalPlayer->GetTeam() || pLocalPlayer->IsEnemy(pHover)) && !pHover->GetAsBuilding()->GetIsShop() && !pHover->GetAsBuilding()->GetNoAltClickPing()) || 
            (pHover->IsHero() && pLocalPlayer->IsEnemy(pHover)))))
        {
            bUnitPingFailed = true;
        }

        if (!bUnitPingFailed)
        {
            pCommander->Ping();
            return XtoA(true);
        }
    }
    
    if (pCommander->GetPingEnabled() && pCommander->GetPingKeyDown())
    {
        pCommander->Ping();
        return XtoA(true);
    }

    return XtoA(pCommander->MinimapPrimaryClick(v2Pos));
}


/*--------------------
  MinimapRightClick
  --------------------*/
UI_CMD(MinimapRightClick, 2)
{
    CClientCommander *pCommander(GameClient.GetClientCommander());
    if (pCommander == nullptr)
        return XtoA(false);

    CVec2f v2Pos(AtoF(vArgList[0]->Evaluate()) * GameClient.GetWorldWidth(), AtoF(vArgList[1]->Evaluate()) * GameClient.GetWorldHeight());

    bool bUnitPingFailed(false);

    if (GameClient.GetMinimapHoverUnit() != -1 && pCommander->GetPingEnabled() && pCommander->GetPingKeyDown())
    {
        CPlayer* pLocalPlayer(Game.GetLocalPlayer());

        if (!pLocalPlayer)
            bUnitPingFailed = true;

        if (!bUnitPingFailed && pLocalPlayer->GetTeam() == TEAM_SPECTATOR)
            bUnitPingFailed = true;

        IUnitEntity *pHover(GameClient.GetUnitEntity(GameClient.GetMinimapHoverUnit()));

        if (!bUnitPingFailed && !(pHover && 
            ((pHover->IsBuilding() && !pHover->GetAsBuilding()->GetNoAltClickPing() && (pHover->GetTeam() == pLocalPlayer->GetTeam() || pLocalPlayer->IsEnemy(pHover)) && !pHover->GetAsBuilding()->GetIsShop() && !pHover->GetAsBuilding()->GetNoAltClickPing()) || 
            (pHover->IsHero() && pLocalPlayer->IsEnemy(pHover)))))
        {
            bUnitPingFailed = true;
        }

        if (!bUnitPingFailed)
        {
            pCommander->Ping();
            return XtoA(true);
        }
    }
    
    if (pCommander->GetPingEnabled() && pCommander->GetPingKeyDown())
    {
        pCommander->Ping();
        return XtoA(true);
    }

    return XtoA(pCommander->MinimapSecondaryClick(v2Pos));
}


/*--------------------
  GetTerrainType
  --------------------*/
FUNCTION(GetTerrainType)
{
    return GameClient.GetTerrainType();
}


/*--------------------
  GetLocalClientNum
  --------------------*/
UI_CMD(GetLocalClientNum, 0)
{
    return XtoA(GameClient.GetLocalClientNum());
}


/*--------------------
  GetLocalClientNum
  --------------------*/
FUNCTION(GetLocalClientNum)
{
    return XtoA(GameClient.GetLocalClientNum());
}


/*--------------------
  GetLocalTeam
  --------------------*/
UI_CMD(GetLocalTeam, 0)
{
    if (GameClient.GetLocalPlayer() == nullptr)
        return _T("0");

    return XtoA(GameClient.GetLocalPlayer()->GetTeam());
}


/*--------------------
  GetNumClients
  --------------------*/
UI_CMD(GetNumClients, 0)
{
    if (vArgList.size() < 1)
        return XtoA(GameClient.GetConnectedClientCount());

    return XtoA(GameClient.GetConnectedClientCount(AtoI(vArgList[0]->Evaluate())));
}


/*--------------------
  GetSelectedEntity
  --------------------*/
UI_CMD(GetSelectedEntity, 0)
{
    if (GameClient.GetClientCommander() == nullptr)
        return XtoA(INVALID_INDEX);

    if (GameClient.GetClientCommander()->GetSelectedInfoEntityIndex() == INVALID_INDEX)
        return XtoA(GameClient.GetClientCommander()->GetSelectedControlEntityIndex());

    return XtoA(GameClient.GetClientCommander()->GetSelectedInfoEntityIndex());
}


/*--------------------
  GetSelectedEntity
  --------------------*/
FUNCTION(GetSelectedEntity)
{
    if (GameClient.GetClientCommander() == nullptr)
        return XtoA(INVALID_INDEX);

    if (GameClient.GetClientCommander()->GetSelectedInfoEntityIndex() == INVALID_INDEX)
        return XtoA(GameClient.GetClientCommander()->GetSelectedControlEntityIndex());

    return XtoA(GameClient.GetClientCommander()->GetSelectedInfoEntityIndex());
}


/*--------------------
  GetLocalClientNumber
  --------------------*/
UI_CMD(GetLocalClientNumber, 0)
{
    return XtoA(GameClient.GetLocalClientNum());
}


/*--------------------
  GetLocalClientNumber
  --------------------*/
FUNCTION(GetLocalClientNumber)
{
    return XtoA(GameClient.GetLocalClientNum());
}


/*--------------------
  SendCreateGameRequest
  --------------------*/
CMD(SendCreateGameRequest)
{
    if (vArgList.size() < 2)
        return false;

    GameClient.SendCreateGameRequest(vArgList[0], ConcatinateArgs(vArgList.begin() + 1, vArgList.end()));
    return true;
}

UI_VOID_CMD(SendCreateGameRequest, 2)
{
    tstring sArgs;
    for (ScriptTokenVector_cit it(vArgList.begin() + 1); it != vArgList.end(); ++it)
        sArgs += (*it)->Evaluate() + SPACE;
    cmdSendCreateGameRequest(vArgList[0]->Evaluate(), sArgs);
}


/*--------------------
  SubmitMatchComment
  --------------------*/
CMD(SubmitMatchComment)
{
    if (vArgList.empty())
        return false;

    CBufferDynamic buffer;
    buffer << GAME_CMD_SUBMIT_MATCH_COMMENT;

    tstring sComment(ConcatinateArgs(vArgList.begin(), vArgList.end()));
    buffer << TStringToUTF8(sComment.substr(0, 512)) << byte(0);
    GameClient.SendGameData(buffer, true);
    return true;
}

UI_VOID_CMD(SubmitMatchComment, 1)
{
    cmdSubmitMatchComment(vArgList[0]->Evaluate());
}


/*--------------------
  SetActiveShop
  --------------------*/
CMD(SetActiveShop)
{
    if (vArgList.empty())
        return false;

    GameClient.SetActiveShop(vArgList[0]);
    return true;
}

UI_VOID_CMD(SetActiveShop, 1)
{
    tsvector vsArgList(1);
    vsArgList[0] = vArgList[0]->Evaluate();

    cmdSetActiveShop(vsArgList);
}


/*--------------------
  SetActiveRecipe
  --------------------*/
CMD(SetActiveRecipe)
{
    if (vArgList.empty())
        return false;

    GameClient.SetActiveRecipe(vArgList[0], false);
    return true;
}

UI_VOID_CMD(SetActiveRecipe, 1)
{
    tsvector vsArgList(1);
    vsArgList[0] = vArgList[0]->Evaluate();

    cmdSetActiveRecipe(vsArgList);
}


/*--------------------
  SetControlUnit
  --------------------*/
CMD(SetControlUnit)
{
    if (vArgList.empty())
        return false;

    CClientCommander *pCommander(GameClient.GetClientCommander());
    if (pCommander == nullptr)
        return false;

    pCommander->SetControlUnit(AtoI(vArgList[0]));
    return true;
}

UI_VOID_CMD(SetControlUnit, 1)
{
    tsvector vsArgList(1);
    vsArgList[0] = vArgList[0]->Evaluate();

    cmdSetControlUnit(vsArgList);
}


/*--------------------
  DeselectUnit
  --------------------*/
CMD(DeselectUnit)
{
    if (vArgList.empty())
        return false;

    CClientCommander *pCommander(GameClient.GetClientCommander());
    if (pCommander == nullptr)
        return false;

    pCommander->DeselectUnit(AtoI(vArgList[0]));
    return true;
}

UI_VOID_CMD(DeselectUnit, 1)
{
    tsvector vsArgList(1);
    vsArgList[0] = vArgList[0]->Evaluate();

    cmdDeselectUnit(vsArgList);
}


/*--------------------
  BuyBack
  --------------------*/
CMD(BuyBack)
{
    CPlayer *pPlayer(Game.GetLocalPlayer());
    if (pPlayer == nullptr)
        return false;

    uint uiUnitIndex(pPlayer->GetHeroIndex());
    if (uiUnitIndex == INVALID_INDEX)
        return false;

    CBufferFixed<5> buffer;
    buffer << GAME_CMD_BUYBACK << uiUnitIndex;
    GameClient.SendGameData(buffer, true);
    return true;
}

UI_VOID_CMD(BuyBack, 0)
{
    cmdBuyBack();
}


/*--------------------
  GameMessage
  --------------------*/
CMD(GameMessage)
{
    if (vArgList.size() < 1)
        return false;

    GameClient.SendMessage(_T("Add ") + vArgList[0], -1);
    return true;
}

UI_VOID_CMD(GameMessage, 1)
{
    cmdGameMessage(vArgList[0]->Evaluate());
}


/*--------------------
  AddMaps
  --------------------*/
UI_VOID_CMD(AddMaps, 1)
{
    if (!pThis || !pThis->HasFlags(WFLAG_LIST))
        return;

    IListWidget *pList(static_cast<IListWidget *>(pThis));

    CXMLNode::PropertyMap mapParams;
    for (ScriptTokenVector_cit cit(vArgList.begin() + 1); cit != vArgList.end(); ++cit)
    {
        CXMLNode::Key sKey((*cit)->Evaluate());
        ++cit;
        if (cit == vArgList.end())
            break;
        mapParams[sKey] = (*cit)->Evaluate();
    }

    tsvector vFileList;
    FileManager.GetFileList(_T("/maps/"), _T("*.s2z"), true, vFileList, true);
    FileManager.GetFileList(_T("~/maps/"), _T("*.s2z"), true, vFileList, true);

    tsvector vFileOnDiskList;
    FileManager.GetFileList(_T("/maps/"), _T("worldconfig"), true, vFileOnDiskList, true);
    FileManager.GetFileList(_T("~/maps/"), _T("worldconfig"), true, vFileOnDiskList, true);
    for (const auto& sFile : vFileOnDiskList)
    {
        auto sPath(TrimRight(Filename_GetPath(sFile), _T("/")));
        vFileList.emplace_back(sPath + _T(".s2z"));
    }

    hash_set<tstring> vSeen;
    for (const auto& sFile : vFileList)
    {
        if (vSeen.find(sFile) != vSeen.end())
            continue;
        vSeen.insert(sFile);

        CArchive cMapArchive(sFile, ARCHIVE_READ);
        CWorld cWorld(WORLDHOST_NULL);

        // Load the main config file
        CFileHandle hWorldConfig(_T("WorldConfig"), FILE_READ, cMapArchive);
        if (!hWorldConfig.IsOpen())
            EX_ERROR(_T("World has no config file"));
        XMLManager.Process(hWorldConfig, _T("world"), &cWorld);

        mapParams[_T("label")] = Filename_GetName(cWorld.GetFancyName());

        if (!cWorld.GetDev() || cg_dev)
            pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), Filename_GetName(sFile), mapParams);

        cWorld.Free();
        cMapArchive.Close();
    }
}


/*--------------------
  AddWeather
  --------------------*/
UI_VOID_CMD(AddWeather, 1)
{
    if (!pThis || !pThis->HasFlags(WFLAG_LIST))
        return;

    IListWidget *pList(static_cast<IListWidget *>(pThis));
    CXMLNode::PropertyMap mapParams;
    for (ScriptTokenVector_cit cit(vArgList.begin() + 1); cit != vArgList.end(); ++cit)
    {
        CXMLNode::Key sKey((*cit)->Evaluate());
        ++cit;
        if (cit == vArgList.end())
            break;
        mapParams[sKey] = (*cit)->Evaluate();
    }

    vector<SWeatherInfo> vWeather;
    GameClient.CalcAvailableWeatherEffects(vWeather);

    for (size_t i(0); i < vWeather.size(); ++i)
    {
        SWeatherInfo& cInfo(vWeather[i]);
        mapParams[_T("label")] = cInfo.sLocalizedName;
        mapParams[_T("weather_name")] = cInfo.sKeyName;
        pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), cInfo.sLocalizedName, mapParams);
    }
}


/*--------------------
  StartWeather
  --------------------*/
UI_CMD(StartWeather, 1)
{
    const tstring& sName(vArgList[0]->Evaluate());
    uint uiIdx(GameClient.StartWeatherByName(sName));
    return XtoA(uiIdx);
}
CMD(StartWeather)
{
    if (vArgList.empty())
    {
        assert(false);
        return false;
    }

    const tstring& sName(vArgList[0]);
    uint uiIdx(GameClient.StartWeatherByName(sName));
    Console << _T("StartWeather result: ") << XtoA(uiIdx) << newl;
    return true;
}


/*--------------------
  StopWeather
  --------------------*/
UI_VOID_CMD(StopWeather, 1)
{
    GameClient.StopWeatherByName(vArgList[0]->Evaluate());
}
CMD(StopWeather)
{
    if (vArgList.empty())
    {
        assert(false);
        return false;
    }
    GameClient.StopWeatherByName(vArgList[0]);
    return true;
}


/*--------------------
  StopAllWeather
  --------------------*/
UI_VOID_CMD(StopAllWeather, 1)
{
    GameClient.StopAllWeather();
}


/*--------------------
  ShareFullControl
  --------------------*/
CMD(ShareFullControl)
{
    if (vArgList.size() < 1)
        return false;

    CBufferFixed<5> buffer;
    buffer << GAME_CMD_SHARE_FULL_CONTROL << AtoI(vArgList[0]);
    GameClient.SendGameData(buffer, true);
    return true;
}

UI_VOID_CMD(ShareFullControl, 1)
{
    cmdShareFullControl(vArgList[0]->Evaluate());
}


/*--------------------
  UnshareFullControl
  --------------------*/
CMD(UnshareFullControl)
{
    if (vArgList.size() < 1)
        return false;

    CBufferFixed<5> buffer;
    buffer << GAME_CMD_UNSHARE_FULL_CONTROL << AtoI(vArgList[0]);
    GameClient.SendGameData(buffer, true);
    return true;
}

UI_VOID_CMD(UnshareFullControl, 1)
{
    cmdUnshareFullControl(vArgList[0]->Evaluate());
}


/*--------------------
  SharePartialControl
  --------------------*/
CMD(SharePartialControl)
{
    if (vArgList.size() < 1)
        return false;

    CBufferFixed<5> buffer;
    buffer << GAME_CMD_SHARE_PARTIAL_CONTROL << AtoI(vArgList[0]);
    GameClient.SendGameData(buffer, true);
    return true;
}

UI_VOID_CMD(SharePartialControl, 1)
{
    cmdSharePartialControl(vArgList[0]->Evaluate());
}


/*--------------------
  UnsharePartialControl
  --------------------*/
CMD(UnsharePartialControl)
{
    if (vArgList.size() < 1)
        return false;

    CBufferFixed<5> buffer;
    buffer << GAME_CMD_UNSHARE_PARTIAL_CONTROL << AtoI(vArgList[0]);
    GameClient.SendGameData(buffer, true);
    return true;
}

UI_VOID_CMD(UnsharePartialControl, 1)
{
    cmdUnsharePartialControl(vArgList[0]->Evaluate());
}


/*--------------------
  CycleSharedControl
  --------------------*/
CMD(CycleSharedControl)
{
    if (vArgList.size() < 1)
        return false;

    CBufferFixed<5> buffer;
    buffer << GAME_CMD_CYCLE_SHARED_CONTROL << AtoI(vArgList[0]);
    GameClient.SendGameData(buffer, true);
    return true;
}

UI_VOID_CMD(CycleSharedControl, 1)
{
    cmdCycleSharedControl(vArgList[0]->Evaluate());
}


/*--------------------
  SetNoHelp
  --------------------*/
CMD(SetNoHelp)
{
    if (vArgList.size() < 2)
        return false;

    CBufferFixed<6> buffer;
    buffer << GAME_CMD_SET_NO_HELP << AtoI(vArgList[0]) << byte(AtoB(vArgList[1]));
    GameClient.SendGameData(buffer, true);
    return true;
}

UI_VOID_CMD(SetNoHelp, 2)
{
    cmdSetNoHelp(vArgList[0]->Evaluate(), vArgList[1]->Evaluate());
}


/*--------------------
  ToggleNoHelp
  --------------------*/
CMD(ToggleNoHelp)
{
    if (vArgList.size() < 1)
        return false;

    CPlayer *pLocalPlayer(GameClient.GetLocalPlayer());
    if (pLocalPlayer == nullptr)
        return false;

    int iClientNumber(AtoI(vArgList[0]));
    CPlayer *pAlly(GameClient.GetPlayer(iClientNumber));

    CBufferFixed<6> buffer;
    buffer << GAME_CMD_SET_NO_HELP << iClientNumber << byte(!pLocalPlayer->GetNoHelp(pAlly));
    GameClient.SendGameData(buffer, true);
    return true;
}

UI_VOID_CMD(ToggleNoHelp, 1)
{
    cmdToggleNoHelp(vArgList[0]->Evaluate());
}


/*--------------------
  GetCurrentControlUnit
  --------------------*/
UI_CMD(GetCurrentControlUnit, 0)
{
    CClientCommander *pCommander(GameClient.GetClientCommander());
    if (pCommander == nullptr)
        return TSNULL;

    return XtoA(pCommander->GetSelectedControlEntityIndex());
}


/*--------------------
  SetExclusiveModifierSlot
  --------------------*/
CMD(SetExclusiveModifierSlot)
{
    if (vArgList.size() < 2)
        return false;

    CBufferFixed<9> buffer;
    buffer << GAME_CMD_SET_ATTACK_MOD_SLOT << AtoI(vArgList[0]) << AtoI(vArgList[1]);
    GameClient.SendGameData(buffer, true);
    return true;
}

UI_VOID_CMD(SetExclusiveModifierSlot, 2)
{
    cmdSetExclusiveModifierSlot(vArgList[0]->Evaluate(), vArgList[1]->Evaluate());
}


/*--------------------
  SetMainInterface
  --------------------*/
CMD(SetMainInterface)
{
    if (vArgList.empty())
        return false;

    GameClient.GetInterfaceManager()->SetMainInterface(vArgList[0]);
    return true;
}

UI_VOID_CMD(SetMainInterface, 1)
{
    cmdSetMainInterface(vArgList[0]->Evaluate());
}


/*--------------------
  ToggleShop
  --------------------*/
UI_VOID_CMD(ToggleShop, 0)
{
    GameClient.GetInterfaceManager()->ToggleShopInterface();
}


/*--------------------
  OpenShop
  --------------------*/
UI_VOID_CMD(OpenShop, 0)
{
    GameClient.GetInterfaceManager()->SetShopVisible(true);
}


/*--------------------
  CloseShop
  --------------------*/
UI_VOID_CMD(CloseShop, 0)
{
    GameClient.GetInterfaceManager()->SetShopVisible(false);
}


/*--------------------
  SetDefaultActiveShop
  --------------------*/
UI_VOID_CMD(SetDefaultActiveShop, 0)
{
    GameClient.GetClientCommander()->SetDefaultActiveShop();
}


/*--------------------
  ToggleLevelup
  --------------------*/
UI_VOID_CMD(ToggleLevelup, 0)
{
    GameClient.GetInterfaceManager()->ToggleLevelupInterface();
}


/*--------------------
  ToggleAllies
  --------------------*/
UI_VOID_CMD(ToggleAllies, 0)
{
    GameClient.GetInterfaceManager()->ToggleAlliesInterface();
}


/*--------------------
  StartCmdClickPos
  --------------------*/
CMD(StartCmdClickPos)
{
    if (GameClient.GetClientCommander() == nullptr)
        return false;

    GameClient.GetClientCommander()->StartClickCmdPos(ConcatinateArgs(vArgList));

    return true;
}


/*--------------------
  AddUnitTypes
  --------------------*/
UI_VOID_CMD(AddUnitTypes, 1)
{
    if (!pThis || !pThis->HasFlags(WFLAG_LIST))
        return;

    IListWidget *pList(static_cast<IListWidget *>(pThis));
    if (pList == nullptr)
        return;

    CXMLNode::PropertyMap mapParams;
    for (ScriptTokenVector_cit cit(vArgList.begin() + 1); cit != vArgList.end(); ++cit)
    {
        CXMLNode::Key sKey((*cit)->Evaluate());
        ++cit;
        if (cit == vArgList.end())
            break;
        mapParams[sKey] = (*cit)->Evaluate();
    }

    map<ushort, tstring> mapEntities;
    EntityRegistry.GetEntityList(mapEntities);
    tsvector vEntities;
    for (map<ushort, tstring>::iterator it(mapEntities.begin()); it != mapEntities.end(); ++it)
    {
        if (it->first <= Entity_Tangible)
            continue;

        const CDynamicEntityAllocator *pAllocator(EntityRegistry.GetDynamicAllocator(it->first));
        if (pAllocator == nullptr || GET_ENTITY_BASE_TYPE1(pAllocator->GetBaseType()) != ENTITY_BASE_TYPE1_UNIT)
            continue;

        vEntities.push_back(it->second);

        ResHandle hDefinition(pAllocator->GetDefinitionHandle());
        CEntityDefinitionResource *pDefRes(g_ResourceManager.Get<CEntityDefinitionResource>(hDefinition));
        IEntityDefinition *pDefinition(pDefRes != nullptr ? pDefRes->GetDefinition<IEntityDefinition>() : nullptr);
        if (pDefinition != nullptr)
        {
            const EntityModifierMap &mapModifiers(pDefinition->GetModifiers());
            for (EntityModifierMap::const_iterator cit(mapModifiers.begin()), citEnd(mapModifiers.end()); cit != citEnd; ++cit)
            {
                if (!cit->second->GetAltAvatar())
                    continue;
                
                vEntities.push_back(it->second + _T(".") + EntityRegistry.LookupModifierKey(cit->second->GetModifierID()));
            }
        }
    }

    sort(vEntities.begin(), vEntities.end());
    for (tsvector_it it(vEntities.begin()); it != vEntities.end(); ++it)
    {
        mapParams[_T("label")] = *it;

        pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), *it, mapParams);
    }
}


/*--------------------
  AddItemTypes
  --------------------*/
UI_VOID_CMD(AddItemTypes, 1)
{
    if (!pThis || !pThis->HasFlags(WFLAG_LIST))
        return;

    IListWidget *pList(static_cast<IListWidget *>(pThis));
    if (pList == nullptr)
        return;

    CXMLNode::PropertyMap mapParams;
    for (ScriptTokenVector_cit cit(vArgList.begin() + 1); cit != vArgList.end(); ++cit)
    {
        CXMLNode::Key sKey((*cit)->Evaluate());
        ++cit;
        if (cit == vArgList.end())
            break;
        mapParams[sKey] = (*cit)->Evaluate();
    }

    map<ushort, tstring> mapEntities;
    EntityRegistry.GetEntityList(mapEntities);
    tsvector vEntities;
    for (map<ushort, tstring>::iterator it(mapEntities.begin()); it != mapEntities.end(); ++it)
    {
        if (it->first <= Entity_Tangible)
            continue;

        const CDynamicEntityAllocator *pAllocator(EntityRegistry.GetDynamicAllocator(it->first));
        if (pAllocator == nullptr || GET_ENTITY_BASE_TYPE3(pAllocator->GetBaseType()) != ENTITY_BASE_TYPE3_ITEM)
            continue;

        vEntities.push_back(it->second);
    }

    sort(vEntities.begin(), vEntities.end());
    for (tsvector_it it(vEntities.begin()); it != vEntities.end(); ++it)
    {
        mapParams[_T("label")] = *it;

        pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), *it, mapParams);
    }
}


#define WRITE_STRING(def, name, tag) \
if (def != nullptr && (bWriteEmpty || def->Get##name##Index() != INVALID_INDEX)) \
    hFile << TabPad(sName + _CTS(#tag), zTabStop, zColumnOffset) << EscapeWhiteSpace(def->Get##name()) << newl;

#define WRITE_STRING_MOD(basedef, def, name, tag) \
if (def != nullptr && (bWriteEmpty || def->Get##name##Index() != INVALID_INDEX) && def->Get##name() != basedef->Get##name()) \
    hFile << TabPad(sName + _CTS(#tag _T(":")) + sModifierName, zTabStop, zColumnOffset) << EscapeWhiteSpace(def->Get##name()) << newl;

#define WRITE_SCRIPT_STRING_NO_MOD \
hFile << TabPad(sName + _CTS("_") + GetActionScriptName(iScript) + _CTS("_effect"), zTabStop, zColumnOffset) << EscapeWhiteSpace(pScript->GetEffectDescription()) << newl;

#define WRITE_SCRIPT_STRING_MOD \
hFile << TabPad(sName + _CTS("_") + GetActionScriptName(iScript) + _CTS("_effect:") + sModifierName, zTabStop, zColumnOffset) << EscapeWhiteSpace(pScript->GetEffectDescription()) << newl;

#define WRITE_SCRIPT_STRINGS(basedef, def, mod) \
for (int iScript(0); iScript < NUM_ACTION_SCRIPTS; ++iScript) \
{ \
    CCombatActionScript *pBaseScript(basedef->GetActionScript(EEntityActionScript(iScript))); \
    CCombatActionScript *pScript(def->GetActionScript(EEntityActionScript(iScript))); \
    if (pScript == nullptr) \
        continue; \
    if (!bWriteEmpty && pScript->GetEffectDescription().empty()) \
        continue; \
    if (basedef != def && pBaseScript != nullptr && pScript->GetEffectDescription() == pBaseScript->GetEffectDescription()) \
        continue; \
\
    WRITE_SCRIPT_STRING_##mod \
}


/*--------------------
  GenerateEntityStringTable
  --------------------*/
CMD(GenerateEntityStringTable)
{
    bool bWriteEmpty(true);
    if (!vArgList.empty())
        bWriteEmpty = !AtoB(vArgList[0]);

    size_t zTabStop(4);
    size_t zColumnOffset(64);

    // Register existing string table
    GameClient.AddResourceToLoadingQueue(CLIENT_RESOURCE_ENTITY_STRING_TABLE, _T("/stringtables/entities.str"), RES_STRINGTABLE);
    GameClient.LoadNextResource();

    CFileHandle hFile(_CTS("/stringtables/entities_") + ICvar::GetString(_CTS("host_language")) + _CTS(".str"), FILE_WRITE | FILE_TEXT | FILE_UTF16);
    if (!hFile.IsOpen())
        return false;

    GameClient.RegisterGameMechanics(_CTS("/base.gamemechanics"));
    GameClient.FetchGameMechanics();
    CGameMechanics *pGameMechanics(GameClient.GetGameMechanics());
    if (pGameMechanics == nullptr)
        Console.Err << _CTS("Missing game mechanics!") << newl;
    else
        pGameMechanics->WriteStringTable(hFile, zTabStop, zColumnOffset);

    CGameInfo::WriteStringTable(hFile, zTabStop, zColumnOffset);

    // Register dynamic entity definitions
    tsvector vFileList;
    FileManager.GetFileList(_T("/"), _T("*.entity"), true, vFileList);
    for (tsvector_it it(vFileList.begin()); it != vFileList.end(); ++it)
        g_ResourceManager.Register(*it, RES_ENTITY_DEF);
    GameClient.PostProcessEntities();

    map<ushort, tstring> mapEntities;
    EntityRegistry.GetEntityList(mapEntities);
    for (map<ushort, tstring>::iterator itEntity(mapEntities.begin()); itEntity != mapEntities.end(); ++itEntity)
    {
        // Shops
        CShopDefinition *pShopDefinition(EntityRegistry.GetDefinition<CShopDefinition>(itEntity->first));
        if (pShopDefinition != nullptr)
        {
            const tstring &sName(pShopDefinition->GetName());

            // Base definition
            hFile << _CTS("// ") << sName << newl;

            WRITE_STRING(pShopDefinition, DisplayName, _name)
            WRITE_STRING(pShopDefinition, Description, _description)

            hFile << newl;
            continue;
        }

        // Units
        IUnitDefinition *pUnitDefinition(EntityRegistry.GetDefinition<IUnitDefinition>(itEntity->first));
        if (pUnitDefinition != nullptr)
        {
            const tstring &sName(pUnitDefinition->GetName());

            // Base definition
            hFile << _CTS("// ") << sName << newl;

            WRITE_STRING(pUnitDefinition, DisplayName, _name)
            WRITE_STRING(pUnitDefinition, Description, _description)

            WRITE_SCRIPT_STRINGS(pUnitDefinition, pUnitDefinition, NO_MOD)

            // Modifiers
            const map<uint, ushort> &mapModifiers(pUnitDefinition->GetModifierIDMap());
            for (map<uint, ushort>::const_iterator itMod(mapModifiers.begin()); itMod != mapModifiers.end(); ++itMod)
            {
                IUnitDefinition *pModifiedDefinition(static_cast<IUnitDefinition*>(pUnitDefinition->GetModifiedDefinition(pUnitDefinition->GetModifierBit(itMod->first))));
                if (pModifiedDefinition == nullptr)
                    continue;

                tstring sModifierName(EntityRegistry.LookupModifierKey(pModifiedDefinition->GetModifierID()));

                WRITE_STRING_MOD(pUnitDefinition, pModifiedDefinition, DisplayName, _name)
                WRITE_STRING_MOD(pUnitDefinition, pModifiedDefinition, Description, _description)

                WRITE_SCRIPT_STRINGS(pUnitDefinition, pModifiedDefinition, MOD)
            }

            hFile << newl;
            continue;
        }

        // Slaves
        ISlaveDefinition *pSlaveDefinition(EntityRegistry.GetDefinition<ISlaveDefinition>(itEntity->first));
        IToolDefinition *pToolDefinition(EntityRegistry.GetDefinition<IToolDefinition>(itEntity->first));
        CItemDefinition *pItemDefinition(EntityRegistry.GetDefinition<CItemDefinition>(itEntity->first));
        if (pSlaveDefinition != nullptr)
        {
            const tstring &sName(pSlaveDefinition->GetName());

            // Base definitions
            hFile << _CTS("// ") << sName << newl;
            WRITE_STRING(pSlaveDefinition, DisplayName, _name)
            WRITE_STRING(pSlaveDefinition, Description, _description)
            WRITE_STRING(pSlaveDefinition, Description2, _description2)
            WRITE_STRING(pToolDefinition, StatusEffectHeader, _effect_header)
            WRITE_STRING(pToolDefinition, StatusEffectHeader2, _effect_header2)
            WRITE_STRING(pToolDefinition, TooltipFlavorText, _tooltip_flavor)
            WRITE_STRING(pItemDefinition, ShopFlavorText, _shop_flavor)

            WRITE_SCRIPT_STRINGS(pSlaveDefinition, pSlaveDefinition, NO_MOD)

            // Modifiers
            const map<uint, ushort> &mapModifiers(pSlaveDefinition->GetModifierIDMap());
            for (map<uint, ushort>::const_iterator itMod(mapModifiers.begin()); itMod != mapModifiers.end(); ++itMod)
            {
                ISlaveDefinition *pModifiedDefinition(static_cast<ISlaveDefinition*>(pSlaveDefinition->GetModifiedDefinition(pSlaveDefinition->GetModifierBit(itMod->first))));
                if (pModifiedDefinition == nullptr)
                    continue;

                IToolDefinition *pModifiedToolDefinition(nullptr);
                if (pToolDefinition != nullptr)
                    pModifiedToolDefinition = static_cast<IToolDefinition*>(pToolDefinition->GetModifiedDefinition(pToolDefinition->GetModifierBit(itMod->first)));

                CItemDefinition *pModifiedItemDefinition(nullptr);
                if (pItemDefinition != nullptr)
                    pModifiedItemDefinition = static_cast<CItemDefinition*>(pItemDefinition->GetModifiedDefinition(pItemDefinition->GetModifierBit(itMod->first)));

                tstring sModifierName(EntityRegistry.LookupModifierKey(pModifiedDefinition->GetModifierID()));

                WRITE_STRING_MOD(pSlaveDefinition, pModifiedDefinition, DisplayName, _name)
                WRITE_STRING_MOD(pSlaveDefinition, pModifiedDefinition, Description, _description)
                WRITE_STRING_MOD(pSlaveDefinition, pModifiedDefinition, Description2, _description2)
                WRITE_STRING_MOD(pToolDefinition, pModifiedToolDefinition, StatusEffectHeader, _effect_header)
                WRITE_STRING_MOD(pToolDefinition, pModifiedToolDefinition, StatusEffectHeader2, _effect_header2)
                WRITE_STRING_MOD(pToolDefinition, pModifiedToolDefinition, TooltipFlavorText, _tooltip_flavor)
                WRITE_STRING_MOD(pItemDefinition, pModifiedItemDefinition, ShopFlavorText, _shop_flavor)

                WRITE_SCRIPT_STRINGS(pSlaveDefinition, pModifiedDefinition, MOD)
            }

            hFile << newl;
            continue;
        }
    }

    return true;
}


/*--------------------
  PrecacheAll
  --------------------*/
CMD(PrecacheAll)
{
    GameClient.PrecacheAll();
    return true;
}


/*--------------------
  RequestBalanceTeams
  --------------------*/
CMD(RequestBalanceTeams)
{
    CPlayer *pPlayer(Game.GetLocalPlayer());
    
    if (pPlayer != nullptr && pPlayer->HasFlags(PLAYER_FLAG_HOST))
    {       
        CBufferFixed<1> buffer;
        buffer << GAME_CMD_BALANCE_TEAMS;
        GameClient.SendGameData(buffer, true);
        return true;
    }
    
    return false;
}

UI_VOID_CMD(RequestBalanceTeams, 0)
{
    cmdRequestBalanceTeams();
}


/*--------------------
  RequestSwapPlayerSlots
  --------------------*/
CMD(RequestSwapPlayerSlots)
{
    if (vArgList.size() < 4)
        return false;
        
    CPlayer *pPlayer(Game.GetLocalPlayer());
    
    if (pPlayer != nullptr && pPlayer->HasFlags(PLAYER_FLAG_HOST))
    {           
        int iTeam1(AtoI(vArgList[0]));
        uint uiSlot1(AtoI(vArgList[1]));
        int iTeam2(AtoI(vArgList[2]));
        uint uiSlot2(AtoI(vArgList[3]));

        CBufferFixed<17> buffer;
        buffer << GAME_CMD_SWAP_PLAYER_SLOT << iTeam1 << uiSlot1 << iTeam2 << uiSlot2;
        GameClient.SendGameData(buffer, true);
        return true;
    }
    
    return false;
}

UI_VOID_CMD(RequestSwapPlayerSlots, 4)
{
    cmdRequestSwapPlayerSlots(vArgList[0]->Evaluate(), vArgList[1]->Evaluate(), vArgList[2]->Evaluate(), vArgList[3]->Evaluate());
}


/*--------------------
  RequestAssignHost
  --------------------*/
CMD(RequestAssignHost)
{
    if (vArgList.empty())
    {
        Console << _T("Must specify a client to be assigned as the host.") << newl;
        return false;
    }
    
    CPlayer *pPlayer(Game.GetLocalPlayer());
    
    if (pPlayer != nullptr && pPlayer->HasFlags(PLAYER_FLAG_HOST))
    {       
        uint uiClientNum(AtoI(vArgList[0]));
            
        CBufferFixed<5> buffer;
        buffer << GAME_CMD_ASSIGN_HOST << uiClientNum;
        GameClient.SendGameData(buffer, true);
        return true;                
    }
    
    return false;
}


UI_VOID_CMD(RequestAssignHost, 1)
{
    cmdRequestAssignHost(vArgList[0]->Evaluate());
}


/*--------------------
  RequestAssignSpectator
  --------------------*/
CMD(RequestAssignSpectator)
{
    if (vArgList.empty())
    {
        Console << _T("Must specify a client to be assigned as a spectator.") << newl;
        return false;
    }
    
    CPlayer *pPlayer(Game.GetLocalPlayer());
    
    if (pPlayer != nullptr && pPlayer->HasFlags(PLAYER_FLAG_HOST))
    {       
        uint uiClientNum(AtoI(vArgList[0]));
            
        CBufferFixed<5> buffer;
        buffer << GAME_CMD_ASSIGN_SPECTATOR << uiClientNum;
        GameClient.SendGameData(buffer, true);
        return true;                
    }
    
    return false;
}


UI_VOID_CMD(RequestAssignSpectator, 1)
{
    cmdRequestAssignSpectator(vArgList[0]->Evaluate());
}


/*--------------------
  RequestLockSlot
  --------------------*/
CMD(RequestLockSlot)
{
    if (vArgList.size() < 2)
        return false;
        
    CPlayer *pPlayer(Game.GetLocalPlayer());
    
    if (pPlayer != nullptr && pPlayer->HasFlags(PLAYER_FLAG_HOST))
    {           
        int iTeam(AtoI(vArgList[0]));
        uint uiSlot(AtoI(vArgList[1]));

        CBufferFixed<9> buffer;
        buffer << GAME_CMD_LOCK_SLOT << iTeam << uiSlot;
        GameClient.SendGameData(buffer, true);
        return true;
    }
    
    return false;
}

UI_VOID_CMD(RequestLockSlot, 2)
{
    cmdRequestLockSlot(vArgList[0]->Evaluate(), vArgList[1]->Evaluate());
}


/*--------------------
  RequestUnlockSlot
  --------------------*/
CMD(RequestUnlockSlot)
{
    if (vArgList.size() < 2)
        return false;

    CPlayer *pPlayer(Game.GetLocalPlayer());
    
    if (pPlayer != nullptr && pPlayer->HasFlags(PLAYER_FLAG_HOST))
    {   
        int iTeam(AtoI(vArgList[0]));
        uint uiSlot(AtoI(vArgList[1]));

        CBufferFixed<9> buffer;
        buffer << GAME_CMD_UNLOCK_SLOT << iTeam << uiSlot;
        GameClient.SendGameData(buffer, true);
        return true;
    }
    
    return false;
}

UI_VOID_CMD(RequestUnlockSlot, 2)
{
    cmdRequestUnlockSlot(vArgList[0]->Evaluate(), vArgList[1]->Evaluate());
}


/*--------------------
  RequestToggleSlotLock
  --------------------*/
CMD(RequestToggleSlotLock)
{
    if (vArgList.size() < 2)
        return false;

    CPlayer *pPlayer(Game.GetLocalPlayer());
    
    if (pPlayer != nullptr && pPlayer->HasFlags(PLAYER_FLAG_HOST))
    {   
        int iTeam(AtoI(vArgList[0]));
        uint uiSlot(AtoI(vArgList[1]));

        CBufferFixed<9> buffer;
        buffer << GAME_CMD_TOGGLE_SLOT_LOCK << iTeam << uiSlot;
        GameClient.SendGameData(buffer, true);
        return true;
    }
    
    return false;
}

UI_VOID_CMD(RequestToggleSlotLock, 2)
{
    cmdRequestToggleSlotLock(vArgList[0]->Evaluate(), vArgList[1]->Evaluate());
}

UI_TRIGGER(EventTest);

CMD(TriggerTest)
{
    EventTest.Trigger(TSNULL);
    return true;
}


/*--------------------
  GetModifier1
  --------------------*/
UI_CMD(GetModifier1, 0)
{
    CClientCommander *pCommander(GameClient.GetClientCommander());
    if (pCommander == nullptr)
        return _CTS("false");
    else
        return XtoA(pCommander->GetModifier1());
}


/*--------------------
  GetModifier2
  --------------------*/
UI_CMD(GetModifier2, 0)
{
    CClientCommander *pCommander(GameClient.GetClientCommander());
    if (pCommander == nullptr)
        return _CTS("false");
    else
        return XtoA(pCommander->GetModifier2());
}


/*--------------------
  GetModifier3
  --------------------*/
UI_CMD(GetModifier3, 0)
{
    CClientCommander *pCommander(GameClient.GetClientCommander());
    if (pCommander == nullptr)
        return _CTS("false");
    else
        return XtoA(pCommander->GetModifier3());
}


/*--------------------
  DownloadReplay
  --------------------*/
UI_VOID_CMD(DownloadReplay, 1)
{
    GameClient.DownloadReplay(vArgList[0]->Evaluate());
}

/*--------------------
  GetReplayDownloadProgress
  --------------------*/
UI_CMD(GetReplayDownloadProgress, 0)
{
    return XtoA(GameClient.GetReplayDownloadProgress());
}

/*--------------------
  ReplayDownloadErrorEncountered
  --------------------*/
UI_CMD(ReplayDownloadErrorEncountered, 0)
{
    return XtoA(GameClient.ReplayDownloadErrorEncountered(), true);
}

/*--------------------
  ReplayDownloadInProgress
  --------------------*/
UI_CMD(ReplayDownloadInProgress, 0)
{
    return XtoA(GameClient.ReplayDownloadInProgress(), true);
}


/*--------------------
  StopReplayDownload
  --------------------*/
UI_VOID_CMD(StopReplayDownload, 0)
{
    GameClient.StopReplayDownload();
}


/*--------------------
  AddPlayers
  --------------------*/
UI_VOID_CMD(AddPlayers, 1)
{
    if (!pThis || !pThis->HasFlags(WFLAG_LIST))
        return;

    IListWidget *pList(static_cast<IListWidget *>(pThis));

    CXMLNode::PropertyMap mapParams;
    for (ScriptTokenVector_cit cit(vArgList.begin() + 1); cit != vArgList.end(); ++cit)
    {
        CXMLNode::Key sKey((*cit)->Evaluate());
        ++cit;
        if (cit == vArgList.end())
            break;
        mapParams[sKey] = (*cit)->Evaluate();
    }

    for (uint uiTeam(0); uiTeam < MAX_DISPLAY_TEAMS; ++uiTeam)
    {
        CTeamInfo *pTeam(GameClient.GetTeam(uiTeam + 1));
        if (pTeam == nullptr)
            continue;

        for (uint uiPlayer(0); uiPlayer < MAX_DISPLAY_PLAYERSPERTEAM; ++uiPlayer)
        {
            CPlayer *pClient(GameClient.GetPlayer(pTeam->GetClientIDFromTeamIndex(uiPlayer)));

            if (pClient == nullptr)
                continue;

            IHeroEntity *pHero(pClient->GetHero());
            mapParams[_T("name")] = pClient->GetName();
            mapParams[_T("color")] = XtoA(pClient->GetColor());
            mapParams[_T("hero")] = pHero != nullptr ? pHero->GetDisplayName() : TSNULL;
            mapParams[_T("icon")] = pHero != nullptr ? pHero->GetIconPath() : _T("$black");

            pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), XtoA(pClient->GetClientNumber()), mapParams);
        }
    }
}


/*--------------------
  SendScriptMessage
  --------------------*/
UI_VOID_CMD(SendScriptMessage, 2)
{
    GameClient.SendScriptMessage(vArgList[0]->Evaluate(), vArgList[1]->Evaluate());
}


/*--------------------
  SendScriptMessage
  --------------------*/
CMD(SendScriptMessage)
{
    if (vArgList.size() < 2)
        return false;

    GameClient.SendScriptMessage(vArgList[0], vArgList[1]);
    return true;
}


/*--------------------
  StripClanTag
  --------------------*/
UI_CMD(StripClanTag, 1)
{
    const tstring &sName(vArgList[0]->Evaluate());
    if (sName.empty() || sName[0] != _T('['))
        return sName;

    const size_t zPos(sName.find(_T("]")));
    if (zPos == tstring::npos)
        return sName;

    return sName.substr(zPos + 1);
}


/*--------------------
  DelayHeroLoading
  --------------------*/
UI_VOID_CMD(DelayHeroLoading, 1)
{
    GameClient.DelayHeroLoading(vArgList[0]->EvaluateInteger());
}


/*--------------------
  DelayHeroLoading
  --------------------*/
CMD(DelayHeroLoading)
{
    if (vArgList.size() < 1)
        return false;

    GameClient.DelayHeroLoading(AtoI(vArgList[0]));
    return true;
}


/*--------------------
  CanKick
  --------------------*/
UI_CMD(CanKick, 1)
{
    int iClientNumber(-1);

    tstring sName(vArgList[0]->Evaluate());

    const PlayerMap &mapPlayers(GameClient.GetPlayerMap());
    for (PlayerMap_cit itPlayer(mapPlayers.begin()); itPlayer != mapPlayers.end(); ++itPlayer)
    {
        if (CompareNoCase(itPlayer->second->GetName(), sName) == 0)
        {
            iClientNumber = itPlayer->first;
            break;
        }
    }

    if (iClientNumber == -1)
    {
        iClientNumber = AtoI(sName);
        if (iClientNumber == 0 && sName != _T("0"))
            iClientNumber = -1;
    }

    if (iClientNumber == -1)
        return TSNULL;

    CPlayer *pPlayer(GameClient.GetPlayer(iClientNumber));
    if (pPlayer == nullptr)
        return TSNULL;

    return XtoA(pPlayer->CanKick());
}


/*--------------------
  GetClientNameFromClientNumber
  --------------------*/
UI_CMD(GetClientNameFromClientNumber, 1)
{
    int iClientNumber(vArgList[0]->EvaluateInteger());

    CPlayer *pPlayer(GameClient.GetPlayer(iClientNumber));
    if (pPlayer == nullptr)
        return TSNULL;

    return pPlayer->GetName();
}


/*--------------------
  GetCurrentGamePhase
  --------------------*/
UI_CMD(GetCurrentGamePhase, 0)
{
    return XtoA(GameClient.GetGamePhase());
}


/*--------------------
  IsInGame
  --------------------*/
UI_CMD(IsInGame, 0)
{
    // they are passed the waiting for players phase, but not yet in the finish game phase
    if (GameClient.GetGamePhase() > GAME_PHASE_WAITING_FOR_PLAYERS && GameClient.GetGamePhase() < GAME_PHASE_ENDED)
        return _T("1");
    else
        return _T("0");
}


/*--------------------
  SetScoreState
  --------------------*/
UI_VOID_CMD(SetScoreState, 1)
{
    CGameInterfaceManager *pInterfaceManager(GameClient.GetInterfaceManager());
    if (pInterfaceManager == nullptr)
        return;
    
    pInterfaceManager->SetScoreState(vArgList[0]->EvaluateInteger());
}


/*--------------------
  TestAltAnnouncement
  --------------------*/
CMD(TestAltAnnouncement)
{
    if (vArgList.size() == 2)
    {
        CGameInterfaceManager *pInterfaceManager(GameClient.GetInterfaceManager());
        if (pInterfaceManager == nullptr)
            return false;
        
        pInterfaceManager->Trigger(AtoI(vArgList[0]), AtoI(vArgList[1]));
    }
    else if (vArgList.size() == 3)
    {
        tsvector vMiniParams(2);
        vMiniParams[0] = vArgList[1];
        vMiniParams[1] = vArgList[2];
        
        CGameInterfaceManager *pInterfaceManager(GameClient.GetInterfaceManager());
        if (pInterfaceManager == nullptr)
            return false;
        
        pInterfaceManager->Trigger(AtoI(vArgList[0]), vMiniParams);
    }
        
    return true;
}


/*--------------------
  GetPing
  --------------------*/
UI_CMD(GetPing, 0)
{
    CPlayer *pPlayer(Game.GetLocalPlayer());
    
    if (pPlayer == nullptr)
        return TSNULL;
    else
        return XtoA(pPlayer->GetPing());
}


CMD(TestFirstKill)
{
    if (vArgList.empty())
        return false;

    CGameInterfaceManager *pInterfaceManager(GameClient.GetInterfaceManager());
    if (pInterfaceManager == nullptr)
        return false;

    tsvector vParams(2);
    vParams[0] = _T("0");
    vParams[1] = vArgList[0];

    pInterfaceManager->Trigger(UITRIGGER_EVENT_FIRST_KILL, vParams);
    return true;
}


/*--------------------
  ServerRefreshUpgrades
  --------------------*/
UI_VOID_CMD(ServerRefreshUpgrades, 0)
{
    GameClient.ServerRefreshUpgrades();
}


/*--------------------
  ServerRefreshUpgrades
  --------------------*/
CMD(ServerRefreshUpgrades)
{
    GameClient.ServerRefreshUpgrades();
    return true;
}

