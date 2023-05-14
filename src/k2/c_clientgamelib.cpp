// (C)2006 S2 Games
// c_clientgamelib.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_clientgamelib.h"
//=============================================================================

/*====================
  CClientGameLib::~CClientGameLib
  ====================*/
CClientGameLib::~CClientGameLib()
{
    if (m_pGameLib != nullptr)
        K2System.FreeLibrary(m_pGameLib);
}


/*====================
  CClientGameLib::CClientGameLib
  ====================*/
CClientGameLib::CClientGameLib(const tstring &sLibName) :
m_pGameLib(nullptr),
m_bValid(false),
m_iMajorVersion(0),
m_iMinorVersion(0),
m_fnSetGamePointer(nullptr),
m_fnInit(nullptr),
m_fnStartLoadingWorld(nullptr),
m_fnFinishedLoadingWorld(nullptr),
m_fnSpawnNextWorldEntity(nullptr),

m_fnStartLoadingResources(nullptr),
m_fnLoadNextResource(nullptr),
m_fnIsFinishedLoadingResources(nullptr),
m_fnGetResourceLoadingProgress(nullptr),

m_fnIsSpawningEntities(nullptr),
m_fnIsFinishedSpawningEntities(nullptr),
m_fnGetEntitySpawningProgress(nullptr),

m_fnStartLoadingEntityResources(nullptr),
m_fnLoadNextEntityResource(nullptr),
m_fnIsFinishedLoadingEntityResources(nullptr),
m_fnGetEntityResourceLoadingProgress(nullptr),

m_fnPreFrame(nullptr),
m_fnFrame(nullptr),
m_fnShutdown(nullptr),
m_fnProcessSnapshot(nullptr),
m_fnProcessGameData(nullptr),
m_fnDropNotify(nullptr),
m_fnReinitialize(nullptr),
m_fnLoadAllResources(nullptr)
{
    try
    {
        m_pGameLib = K2System.LoadLibrary(sLibName);
        if (m_pGameLib == nullptr)
            EX_ERROR(_T("Couldn't load client library: ") + sLibName);

        // Initialize the game library
        FnInitGameDLL   *pfnInitGameDLL((FnInitGameDLL*)K2System.GetProcAddress(m_pGameLib, _T("InitLibrary")));
        if (pfnInitGameDLL == nullptr)
            EX_ERROR(_T("Couldn't find entry function InitLibrary()"));
        pfnInitGameDLL(*this);

        // Validate API
        if (m_fnInit == nullptr)
            EX_ERROR(_T("Client API is missing Init() function"));
        if (m_fnFrame == nullptr)
            EX_ERROR(_T("Client API is missing Frame() function"));
        if (m_fnShutdown == nullptr)
            EX_ERROR(_T("Client API is missing Shutdown() function"));

        m_bValid = true;
    }
    catch (CException &ex)
    {
        if (m_pGameLib != nullptr)
        {
            K2System.FreeLibrary(m_pGameLib);
            m_pGameLib = nullptr;
        }

        ex.Process(_T("CClientGameLib::CClientGameLib() - "), NO_THROW);
    }
}
