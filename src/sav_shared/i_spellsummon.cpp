// (C)2007 S2 Games
// i_spellsummon.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_spellsummon.h"
#include "i_petentity.h"
//=============================================================================

/*====================
  ISpellSummon::CEntityConfig::CEntityConfig
  ====================*/
ISpellSummon::CEntityConfig::CEntityConfig(const tstring &sName) :
ISpellItem::CEntityConfig(sName),
INIT_ENTITY_CVAR(PetName, _T(""))
{
}


/*====================
  ISpellSummon::ISpellSummon
  ====================*/
ISpellSummon::ISpellSummon(CEntityConfig *pConfig) :
ISpellItem(pConfig),

m_pEntityConfig(pConfig)
{
}


/*====================
  ISpellSummon::ImpactPosition
  ====================*/
bool    ISpellSummon::ImpactPosition(const CVec3f &v3Target, CGameEvent &evImpact)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;

    ISpellItem::ImpactPosition(v3Target, evImpact);

    // Spawn the pet
    IGameEntity *pNewEnt(Game.AllocateEntity(GetPetName()));
    if (pNewEnt == NULL || pNewEnt->GetAsPet() == NULL)
    {
        Console.Warn << _T("Failed to summon pet: ") << GetPetName() << newl;
        return false;
    }

    IPetEntity *pPet(pNewEnt->GetAsPet());

    pPet->SetOwnerUID(pOwner->GetUniqueID());
    pPet->SetTeam(pOwner->GetTeam());
    pPet->SetPosition(v3Target);
    pPet->SetAngles(pOwner->GetAngles());
    pPet->Spawn();
    pPet->SetPetMode(PETMODE_DEFENSIVE);
    pPet->PlayerCommand(PETCMD_RETURN, INVALID_INDEX, V3_ZERO);

    if (pOwner && pOwner->IsPlayer())
    {
        uint uiOldPet(pOwner->GetAsPlayerEnt()->GetPetIndex());
        IGameEntity *pOldEnt(Game.GetEntity(uiOldPet));

        if (pOldEnt && pOldEnt->IsPet() && pOldEnt->GetAsPet()->GetStatus() == ENTITY_STATUS_ACTIVE)
            pOldEnt->Kill();

        pOwner->GetAsPlayerEnt()->SetPetIndex(pPet->GetIndex());

        pOwner->GiveExperience(m_pEntityConfig->GetCastExperience(), pPet->GetPosition());

        // "teach" owner to control the pet
        pOwner->GetAsPlayerEnt()->GiveItem(INVENTORY_PETCMD_ATTACK, PetCommand_Attack);
        pOwner->GetAsPlayerEnt()->GiveItem(INVENTORY_PETCMD_FOLLOW, PetCommand_Follow);
        pOwner->GetAsPlayerEnt()->GiveItem(INVENTORY_PETCMD_MOVE, PetCommand_Move);
    }

    return true;
}


/*====================
  ISpellSummon::ClientPrecache
  ====================*/
void    ISpellSummon::ClientPrecache(CEntityConfig *pConfig)
{
    ISpellItem::ClientPrecache(pConfig);

    if (!pConfig)
        return;
    
    if (!pConfig->GetPetName().empty())
        EntityRegistry.ClientPrecache(EntityRegistry.LookupID(pConfig->GetPetName()));
}


/*====================
  ISpellSummon::ServerPrecache
  ====================*/
void    ISpellSummon::ServerPrecache(CEntityConfig *pConfig)
{
    ISpellItem::ServerPrecache(pConfig);

    if (!pConfig)
        return;
    
    if (!pConfig->GetPetName().empty())
        EntityRegistry.ServerPrecache(EntityRegistry.LookupID(pConfig->GetPetName()));
}
