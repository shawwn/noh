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
    if (m_pGameLib != nullptr)
    {
        Shutdown();
        K2System.FreeLibrary(m_pGameLib);
    }
}

#ifdef GAME_SHARED_LIB
extern "C" void    InitLibrary(CServerGameLib &GameLib);
#endif

/*====================
  CServerGameLib::CServerGameLib
  ====================*/
CServerGameLib::CServerGameLib(const tstring &sLibPath) :
m_bValid(false),
m_pGameLib(nullptr),
m_iMajorVersion(0),
m_iMinorVersion(0),
m_fnSetGamePointer(nullptr),
m_fnInit(nullptr),
m_fnFrame(nullptr),
m_fnLoadWorld(nullptr),
m_fnAddClient(nullptr),
m_fnRemoveClient(nullptr),
m_fnGetMaxClients(nullptr),
m_fnProcessClientSnapshot(nullptr),
m_fnProcessGameData(nullptr),
m_fnGetSnapshot(nullptr),
m_fnShutdown(nullptr),
m_fnGetMatchTime(nullptr),
m_fnReauthClient(nullptr),
m_fnStartReplay(nullptr),
m_fnStopReplay(nullptr),
m_fnStateStringChanged(nullptr),
m_fnUnloadWorld(nullptr),
m_fnGetEntity(nullptr)
{
    // Load the game library
    m_pGameLib = K2System.LoadLibrary(sLibPath);
    if (m_pGameLib == nullptr)
    {
        Console.Server << _T("Couldn't load server library: ") << sLibPath << newl;
        return;
    }

#ifndef GAME_SHARED_LIB
    // Initialize the game library
    FnInitServerGameLib *pfnInitGameLib((FnInitServerGameLib*)K2System.GetProcAddress(m_pGameLib, _T("InitLibrary")));
    if (pfnInitGameLib == nullptr)
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
    if (m_fn##fn == nullptr) \
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

