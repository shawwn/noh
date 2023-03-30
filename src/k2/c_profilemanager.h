// (C)2005 S2 Games
// c_profilemanager.h
//
// Class based hierarchical ingame profiler
//=============================================================================
#ifndef __C_PROFILEMANAGER_H__
#define __C_PROFILEMANAGER_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"

#include "c_profilenode.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#ifdef K2_PROFILE
#define PROFILE(s)          CProfileSample  _sample(_T(s), PROFILE_INHERIT)
#define PROFILE_EX(s, t)    CProfileSample  _sample(_T(s), t)
#define START_PROFILE(s)    { PROFILE(s);
#define END_PROFILE         }
#else
#define PROFILE(s)
#define PROFILE_EX(s, t)
#define START_PROFILE(s)
#define END_PROFILE
#endif

#define GAME_PROFILE(s)         CProfileSample  _sample(s, PROFILE_INHERIT)
#define GAME_PROFILE_EX(s, t)   CProfileSample  _sample(s, t)
//=============================================================================

//=============================================================================
// CProfileManager
//=============================================================================
class CProfileManager
{
private:
    CProfileNode    *m_pRootNode;
    CProfileNode    *m_pCurrentNode;
    
    const CProfileNode  *m_pPrintNode;

    bool            m_bActive;
    bool            m_bDraw;

public:
    CProfileManager();

    ~CProfileManager();

    K2_API bool StartSample(const TCHAR *szNode, eProfileType eType);
    K2_API void EndSample();

    bool    IsActive()                  { return m_bActive; }
    void    SetActive(bool bActive)     { m_bActive = bActive; }
    void    SetDraw(bool bDraw)         { m_bDraw = bDraw; }

    K2_API void ResetFrame(eProfileType eType);

    K2_API void Frame();
    K2_API void Draw();

    void    Print();
    void    PrintMax();
    void    ResetMax();
    void    SetNode(TCHAR ch);
};

extern K2_API CProfileManager ProfileManager;

#include "c_profilesample.h"
//=============================================================================

#endif // __C_PROFILEMANAGER_H__
