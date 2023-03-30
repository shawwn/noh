// (C)2009 S2 Games
// c_gamemechanicsresource.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_gamemechanicsresource.h"

#include "../k2/i_resourcelibrary.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
extern IResourceLibrary g_ResLibEntityDefinition;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
IResource*  AllocGameMechanics(const tstring &sPath)
{
    return K2_NEW(ctx_Game,    CGameMechanicsResource)(sPath);
}

IResourceLibrary    g_ResLibGameMechanics(RES_GAME_MECHANICS, _T("Game Mechanics"), CGameMechanicsResource::ResTypeName(), true, AllocGameMechanics);
//=============================================================================

/*====================
  CGameMechanicsResource::Load
  ====================*/
int     CGameMechanicsResource::Load(uint uiIgnoreFlags, const char *pData, uint uiSize)
{
    PROFILE("CGameMechanicsResource::Load");

    Console.Res << _T("Loading ^750Game Mechanics^*: ") << m_sPath << newl;

    // Process the XML
    if (!XMLManager.ReadBuffer(pData, uiSize, _T("gamemechanics"), this))
    {
        Console.Warn << _T("CGameMechanicsResource::Load(") + m_sPath + _T(") - couldn't read XML") << newl;
        return RES_LOAD_FAILED;
    }

    return 0;
}


/*====================
  CGameMechanicsResource::PostLoad
  ====================*/
void    CGameMechanicsResource::PostLoad()
{
    Game.RegisterGameMechanics(GetHandle());
    if (m_pGameMechanics != NULL)
        m_pGameMechanics->PostLoad();

    g_ResLibEntityDefinition.ReloadAll();
}
