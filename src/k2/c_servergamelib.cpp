// (C)2008 S2 Games
// c_servergamelib.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_servergamelib.h"
//=============================================================================

/*====================
  CServerGameLib::~CServerGameLib
  ====================*/
CServerGameLib::~CServerGameLib()
{
    if (m_pGameLib != NULL)
    {
        Shutdown();
        K2System.FreeLibrary(m_pGameLib);
    }
}

#ifdef GAME_SHARED_LIB
void    InitLibrary(CServerGameLib &GameLib);
#endif

/*====================
  CServerGameLib::CServerGameLib
  ====================*/
CServerGameLib::CServerGameLib(const tstring &sLibPath) :
m_bValid(false),
m_pGameLib(NULL),
m_iMajorVersion(0),
m_iMinorVersion(0),
m_fnSetGamePointer(NULL),
m_fnInit(NULL),
m_fnFrame(NULL),
m_fnLoadWorld(NULL),
m_fnAddClient(NULL),
m_fnRemoveClient(NULL),
m_fnGetMaxClients(NULL),
m_fnProcessClientSnapshot(NULL),
m_fnProcessGameData(NULL),
m_fnGetSnapshot(NULL),
m_fnShutdown(NULL),
m_fnGetMatchTime(NULL),
m_fnReauthClient(NULL),
m_fnStartReplay(NULL),
m_fnStopReplay(NULL),
m_fnStateStringChanged(NULL),
m_fnUnloadWorld(NULL),
m_fnGetEntity(NULL)
{
    // Load the game library
    m_pGameLib = K2System.LoadLibrary(sLibPath);
    if (m_pGameLib == NULL)
    {
        Console.Server << _T("Couldn't load server library: ") << sLibPath << newl;
        return;
    }

#ifndef GAME_SHARED_LIB
    // Initialize the game library
    FnInitServerGameLib *pfnInitGameLib((FnInitServerGameLib*)K2System.GetProcAddress(m_pGameLib, _T("InitLibrary")));
    if (pfnInitGameLib == NULL)
    {
        Console.Server << _T("Couldn't find entry function \"InitLibrary()\"") << newl;
        return;
    }

    pfnInitGameLib(*this);
#else
    InitLibrary(*this);
#endif

    // Validate API
    #define CHECK_FUNCTION(fn) \
    if (m_fn##fn == NULL) \
    { \
        Console.Server << _T("Server API is missing: ") _T(#fn) _T("()") << newl; \
        m_bValid = false; \
    }
    
    m_bValid = true;
    CHECK_FUNCTION(SetGamePointer)
    CHECK_FUNCTION(Init)
    CHECK_FUNCTION(Frame)
    CHECK_FUNCTION(LoadWorld)
    CHECK_FUNCTION(AddClient)
    CHECK_FUNCTION(RemoveClient)
    CHECK_FUNCTION(ClientTimingOut)
    CHECK_FUNCTION(GetMaxClients)
    CHECK_FUNCTION(ProcessClientSnapshot)
    CHECK_FUNCTION(ProcessGameData)
    CHECK_FUNCTION(GetSnapshot)
    CHECK_FUNCTION(Shutdown)
    CHECK_FUNCTION(GetMatchTime)
    CHECK_FUNCTION(ReauthClient)
    CHECK_FUNCTION(StartReplay)
    CHECK_FUNCTION(StopReplay)
    CHECK_FUNCTION(StateStringChanged)
    CHECK_FUNCTION(UnloadWorld)
    CHECK_FUNCTION(GetEntity)

    #undef CHECK_FUNCTION
}

