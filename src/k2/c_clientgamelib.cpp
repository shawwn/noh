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
    if (m_pGameLib != NULL)
        K2System.FreeLibrary(m_pGameLib);
}


/*====================
  CClientGameLib::CClientGameLib
  ====================*/
CClientGameLib::CClientGameLib(const tstring &sLibName) :
m_pGameLib(NULL),
m_bValid(false),
m_iMajorVersion(0),
m_iMinorVersion(0),
m_fnSetGamePointer(NULL),
m_fnInit(NULL),
m_fnStartLoadingWorld(NULL),
m_fnFinishedLoadingWorld(NULL),
m_fnSpawnNextWorldEntity(NULL),

m_fnStartLoadingResources(NULL),
m_fnLoadNextResource(NULL),
m_fnIsFinishedLoadingResources(NULL),
m_fnGetResourceLoadingProgress(NULL),

m_fnIsSpawningEntities(NULL),
m_fnIsFinishedSpawningEntities(NULL),
m_fnGetEntitySpawningProgress(NULL),

m_fnStartLoadingEntityResources(NULL),
m_fnLoadNextEntityResource(NULL),
m_fnIsFinishedLoadingEntityResources(NULL),
m_fnGetEntityResourceLoadingProgress(NULL),

m_fnPreFrame(NULL),
m_fnFrame(NULL),
m_fnShutdown(NULL),
m_fnProcessSnapshot(NULL),
m_fnProcessGameData(NULL),
m_fnDropNotify(NULL),
m_fnReinitialize(NULL),
m_fnLoadAllResources(NULL)
{
    try
    {
        m_pGameLib = K2System.LoadLibrary(sLibName);
        if (m_pGameLib == NULL)
            EX_ERROR(_T("Couldn't load client library: ") + sLibName);

        // Initialize the game library
        FnInitGameDLL   *pfnInitGameDLL((FnInitGameDLL*)K2System.GetProcAddress(m_pGameLib, _T("InitLibrary")));
        if (pfnInitGameDLL == NULL)
            EX_ERROR(_T("Couldn't find entry function InitLibrary()"));
        pfnInitGameDLL(*this);

        // Validate API
        if (m_fnInit == NULL)
            EX_ERROR(_T("Client API is missing Init() function"));
        if (m_fnFrame == NULL)
            EX_ERROR(_T("Client API is missing Frame() function"));
        if (m_fnShutdown == NULL)
            EX_ERROR(_T("Client API is missing Shutdown() function"));

        m_bValid = true;
    }
    catch (CException &ex)
    {
        if (m_pGameLib != NULL)
        {
            K2System.FreeLibrary(m_pGameLib);
            m_pGameLib = NULL;
        }

        ex.Process(_T("CClientGameLib::CClientGameLib() - "), NO_THROW);
    }
}
