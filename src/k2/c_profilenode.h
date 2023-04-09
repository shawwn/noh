// (C)2005 S2 Games
// c_profilemanager.h
//
// Data node for CProfileManager
//=============================================================================
#ifndef __C_PROFILENODE_H__
#define __C_PROFILENODE_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
class CProfileNode;

typedef vector<const TCHAR *>   NodeNameVector;
typedef vector<CProfileNode *>  ProfileVector;

enum eProfileType
{
    PROFILE_ROOT = 0,
    PROFILE_INHERIT,
    PROFILE_LEAF,
    PROFILE_INIT,
    PROFILE_STARTCLIENT,
    PROFILE_CONNECT,
    PROFILE_WORLDLOAD,
    PROFILE_GAMELOOP,
    PROFILE_SHUTDOWN,

    PROFILE_EVALUATOR,

    PROFILE_WORLD_NEW,
    PROFILE_WORLD_LOAD,
    PROFILE_WORLD_SAVE,
    PROFILE_WORLD_PATH,

    PROFILE_GAME_SERVER_FRAME,
    PROFILE_GAME_CLIENT_PROCESS_SNAPSHOT,
    PROFILE_GAME_LOAD_INTERFACE,
    PROFILE_GAME_LOAD_WORLD,
    PROFILE_GAME_PRECACHE_ENTITIES,

    PROFILE_CLIENTSNAPSHOT
};

K2_API extern bool      g_bProfile;
//=============================================================================

//=============================================================================
// CProfileNode
//=============================================================================
class CProfileNode
{
private:
    const TCHAR     *m_szName;
    eProfileType    m_eType;

    LONGLONG        m_llStartTime;
    LONGLONG        m_llTotalTime;

    int             m_iFrameCalls;
    int             m_iFrameRecursiveCalls;
    LONGLONG        m_llFrameTime;
    
    int             m_iWorkingFrameCalls;
    int             m_iWorkingFrameRecursiveCalls;
    LONGLONG        m_llWorkingFrameTime;

    int             m_iMaxFrameCalls;
    int             m_iMaxFrameRecursiveCalls;
    LONGLONG        m_llMaxFrameTime;

    int             m_iTotalCalls;
    int             m_iFrame;

    NodeNameVector  m_vChildrenNames;
    ProfileVector   m_vChildren;
    CProfileNode    *m_pParent;

    void            StartTimer()
    {
        m_llStartTime = K2System.GetTicks();
        ++m_iTotalCalls;
        ++m_iWorkingFrameCalls;
    }

    void            EndTimer()
    {
        LONGLONG    llEndTime = K2System.GetTicks();

        m_llTotalTime += llEndTime - m_llStartTime;
        m_llWorkingFrameTime += llEndTime - m_llStartTime;
    }

public:
    ~CProfileNode();
    CProfileNode(const TCHAR *szName, eProfileType eType, CProfileNode *pParent);

    inline CProfileNode*    StartSample(const TCHAR *szName, eProfileType eType);
    inline CProfileNode*    EndSample();

    void            ResetFrame(eProfileType eType);
    void            ResetMax();

    tstring         GetName() const             { return m_szName; }
    LONGLONG        GetTotalTime() const        { return m_llTotalTime; }
    int             GetTotalCalls() const       { return m_iTotalCalls; }
    LONGLONG        GetFrameTime() const        { return m_llFrameTime; }
    int             GetFrameCalls() const       { return m_iFrameCalls; }
    
    eProfileType    GetType() const             { return m_eType; }
    int             GetFrame() const            { return m_iFrame; }
    
    LONGLONG        GetMaxFrameTime() const     { return m_llMaxFrameTime; }
    int             GetMaxFrameCalls() const    { return m_iMaxFrameCalls; }

    const TCHAR*    GetNodeNamePointer()        { return m_szName; }
    
    const CProfileNode*     GetParent() const   { return m_pParent; }
    const ProfileVector&    GetChildren() const { return m_vChildren; }

    static bool     Pred(CProfileNode *pElem1, CProfileNode *pElem2)
    {
        return pElem1->m_szName < pElem2->m_szName;
    }
};
//=============================================================================

/*====================
  CProfileNode::StartSample
  ====================*/
CProfileNode*   CProfileNode::StartSample(const TCHAR *szName, eProfileType eType)
{
    if (!CSystem::IsInitialized())
        return this;
    if (m_szName == szName)
    {
        ++m_iWorkingFrameCalls;
        ++m_iWorkingFrameRecursiveCalls;
        return this;
    }

    NodeNameVector::iterator findit(lower_bound(m_vChildrenNames.begin(), m_vChildrenNames.end(), szName));

    if (findit == m_vChildrenNames.end() || *findit != szName)
    {
        bool bProfile(g_bProfile);
        g_bProfile = false;
        
        m_vChildrenNames.push_back(szName);
        sort(m_vChildrenNames.begin(), m_vChildrenNames.end());

        m_vChildren.push_back(K2_NEW(ctx_Profile,  CProfileNode)(szName, (eType == PROFILE_INHERIT || eType == PROFILE_LEAF) ? m_eType : eType, this));
        sort(m_vChildren.begin(), m_vChildren.end(), CProfileNode::Pred);
        
        findit = lower_bound(m_vChildrenNames.begin(), m_vChildrenNames.end(), szName);
        g_bProfile = bProfile;
    }

    size_t uiIndex(findit - m_vChildrenNames.begin());

    m_vChildren[uiIndex]->StartTimer();
    return m_vChildren[uiIndex];
}


/*====================
  CProfileNode::EndSample
  ====================*/
CProfileNode*   CProfileNode::EndSample()
{
    if (!CSystem::IsInitialized())
        return this;
    if (m_iWorkingFrameRecursiveCalls > 0)
    {
        --m_iWorkingFrameRecursiveCalls;
        return this;
    }

    EndTimer();
    return m_pParent;
}

//=============================================================================
#endif // __C_PROFILENODE_H__
