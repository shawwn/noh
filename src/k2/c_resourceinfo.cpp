// (C)2010 S2 Games
// c_resourceinfo.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_resourceinfo.h"
#include "c_resourcemanager.h"
#include "i_resourcelibrary.h"
#include "i_resource.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CResourceInfo&  g_ResourceInfo(*CResourceInfo::GetInstance());

SINGLETON_INIT(CResourceInfo)

CVAR_BOOL(res_debugContext, false);
//=============================================================================

//=============================================================================
// Private Macros
//=============================================================================

/*====================
  ForEachResInfo
  ====================*/
#define ForEachResInfo(resPtrDecl, infoPtrDecl)                                                     \
    for (ResInfoMap::iterator                                                                       \
        FOREACH_it(m_mapResInfo.begin()),                                                           \
        FOREACH_itEnd(m_mapResInfo.end());                                                          \
        FOREACH_it != FOREACH_itEnd;                                                                \
        ++FOREACH_it)                                                                               \
        if (resPtrDecl = g_ResourceManager.LookUpHandle(FOREACH_it->first))                         \
            if (infoPtrDecl = &FOREACH_it->second)


/*====================
  ForEachResChain
  ====================*/
#define ForEachResChain(stkParentsDecl, depthDecl, resPtrDecl, infoPtrDecl)                         \
    {                                                                                               \
        class FOREACH_Iterator : public IResourceGraphIterator                                      \
        {                                                                                           \
            void    IterateResource(stkParentsDecl, depthDecl, resPtrDecl, infoPtrDecl)             \

#define ForEachResChainEnd_DoAllToplevel()                                                          \
        };                                                                                          \
        IterateToplevel(FOREACH_Iterator());                                                        \
    }
#define ForEachResChainEnd_DoContext(sCtxName, pCurCtx)                                             \
        };                                                                                          \
        IterateContext(FOREACH_Iterator(), sCtxName, pCurCtx);                                      \
    }

#define ForEachResChain_ForEachParent(stkParents, resPtrDecl, infoPtrDecl)                          \
    for (ResInfoStack::const_reverse_iterator                                                       \
        FORPARENT_it((stkParents).rbegin()),                                                        \
        FORPARENT_itEnd((stkParents).rend());                                                       \
        FORPARENT_it != FORPARENT_itEnd;                                                            \
        ++FORPARENT_it)                                                                             \
        if (resPtrDecl = (*FORPARENT_it).first)                                                     \
            if (infoPtrDecl = (*FORPARENT_it).second)


/*====================
  ForEachResInfo
  ====================*/
#define ForEachResHandle(resContainerType, resContainer, resPtrDecl, infoPtrDecl)                   \
    for (resContainerType::iterator                                                                 \
        FOREACH_it((resContainer).begin()),                                                         \
        FOREACH_itEnd((resContainer).end());                                                        \
        FOREACH_it != FOREACH_itEnd;                                                                \
        ++FOREACH_it)                                                                               \
        if (resPtrDecl = g_ResourceManager.LookUpHandle(*FOREACH_it))                                   \
            if (infoPtrDecl = g_ResourceInfo.LookupInfo(*FOREACH_it))


/*====================
  ForEachResContext
  ====================*/
#define ForEachResContext(resCtxNamePtrDecl, resCtxPtrDecl)                                         \
    for (ResContextMap::iterator                                                                    \
        FOREACH_it(g_ResourceInfo.m_mapResContexts.begin()),                                        \
        FOREACH_itEnd(g_ResourceInfo.m_mapResContexts.end());                                       \
        FOREACH_it != FOREACH_itEnd;                                                                \
        ++FOREACH_it)                                                                               \
        if (resCtxNamePtrDecl = &(*FOREACH_it).first)                                               \
            if (resCtxPtrDecl = &(*FOREACH_it).second)

//=============================================================================

//=============================================================================
// CGraphResource
//=============================================================================
CGraphResource*     CGraphResource::s_pCurrentParent;

/*====================
  CGraphResource::~CGraphResource
  ====================*/
CGraphResource::~CGraphResource()
{
    Done();
}

/*====================
  CGraphResource::CGraphResource
  ====================*/
CGraphResource::CGraphResource()
: m_pPrevParent(nullptr)
, m_hResource(INVALID_RESOURCE)
, m_bValid(true)
, m_bDone(false)
, m_bLinking(false)
{
    // store the current parent.
    m_pPrevParent = s_pCurrentParent;
    
    // make ourselves the current parent.
    s_pCurrentParent = this;
}


/*====================
  CGraphResource::SetDebugPath
  ====================*/
void    CGraphResource::SetDebugPath(const tstring &sPath)
{
#ifdef _DEBUG
    m_sPath = sPath;
#endif
}


/*====================
 CGraphResource::SetHandle
 ====================*/
void    CGraphResource::SetHandle(ResHandle hResource)
{
    m_hResource = hResource;
}


/*====================
  CGraphResource::LinkChildren
  ====================*/
void    CGraphResource::LinkChildren(ResHandle hResource)
{
    if (hResource == INVALID_RESOURCE)
        return;

    SetHandle(hResource);
    m_bLinking = true;
}


/*====================
  CGraphResource::Reset
  ====================*/
void    CGraphResource::Reset()
{
    m_setChildren.clear();
}


/*====================
 CGraphResource::Cancel
 ====================*/
void    CGraphResource::Cancel()
{
    m_bValid = false;
    m_setChildren.clear();
}


/*====================
  CGraphResource::Done
  ====================*/
void    CGraphResource::Done()
{
    if (m_bDone)
        return;
    m_bDone = true;

    // restore the previous parent.
    s_pCurrentParent = m_pPrevParent;

    // if we were canceled or our handle is invalid, then do nothing.
    if (!m_bValid || m_hResource == INVALID_RESOURCE)
        return;

    bool bIsToplevel(false);
    if (s_pCurrentParent == nullptr)
        bIsToplevel = true;
    if (m_bLinking)
        bIsToplevel = false;

    // verify that our own handle was not somehow added to our list of children.
    for (ResourceSet::iterator it(m_setChildren.begin()), itEnd(m_setChildren.end()); it != itEnd;)
    {
        ResHandle hChild(*it);
        assert(hChild != INVALID_RESOURCE);

        assert(hChild !=  m_hResource);
        if (hChild == m_hResource)
            m_setChildren.erase(it++);
        else
            ++it;
    }

    CResourceInfo::SResInfo* pInfo(g_ResourceInfo.InfoAlloc(m_hResource));

    // update our stored list of child resources.
    //g_ResourceInfo.InfoClearChildren(m_hResource, m_setChildren);
    g_ResourceInfo.InfoAddChildren(m_hResource, m_setChildren);

    // don't add ourselves to the parent if we were just linking children to a handle.
    if (!m_bLinking)
    {
        // add our handle as a child of the parent resource.
        if (s_pCurrentParent != nullptr)
            s_pCurrentParent->m_setChildren.insert(m_hResource);

        // add to the active context.
        if (bIsToplevel)
        {
            pInfo->SetFlags(CResourceInfo::RI_TOPLEVEL);
            g_ResourceInfo.InfoAddToActiveContext(m_hResource);
        }
    }
}


/*====================
  CGraphResource::LinkExistingChild
  ====================*/
void    CGraphResource::LinkExistingChild(ResHandle hResource)
{
    assert(hResource != INVALID_RESOURCE);
    if (hResource == INVALID_RESOURCE)
        return;

    if (s_pCurrentParent == nullptr)
    {
        g_ResourceInfo.InfoAddToActiveContext(hResource);
        return;
    }

    s_pCurrentParent->m_setChildren.insert(hResource);
}
//=============================================================================


//=============================================================================
// CResourceInfo
//=============================================================================

/*====================
  CResourceInfo::InfoAlloc
  ====================*/
CResourceInfo::SResInfo*    CResourceInfo::InfoAlloc(ResHandle hRes)
{
    assert(hRes != INVALID_RESOURCE);
    if (hRes == INVALID_RESOURCE)
        return nullptr;

    return &m_mapResInfo[hRes];
}


/*====================
  CResourceInfo::InfoFree
  ====================*/
void    CResourceInfo::InfoFree(ResHandle hRes)
{
    ResInfoMap::iterator itFind(m_mapResInfo.find(hRes));
    assert(itFind != m_mapResInfo.end());
    if (itFind == m_mapResInfo.end())
        return;

    m_mapResInfo.erase(itFind);
}


/*====================
  CResourceInfo::InfoClearChildren
  ====================*/
void    CResourceInfo::InfoClearChildren(ResHandle hRes)
{
    SResInfo* pInfo(LookupInfo(hRes));
    assert(pInfo != nullptr);
    if (pInfo == nullptr)
        return;

    pInfo->setChildren.clear();
}


/*====================
  CResourceInfo::InfoAddChildren
  ====================*/
void    CResourceInfo::InfoAddChildren(ResHandle hRes, const ResourceSet& setChildren)
{
    PROFILE("CResourceInfo::InfoAddChildren");

    SResInfo* pInfo(LookupInfo(hRes));
    assert(pInfo != nullptr);
    if (pInfo == nullptr)
        return;

    // merge.
    for (ResourceSet::const_iterator it(setChildren.begin()), itEnd(setChildren.end());
        it != itEnd;
        ++it)
    {
        ResHandle hChild(*it);
        pInfo->setChildren.insert(hChild);
    }
}


/*====================
  CResourceInfo::InfoAddToActiveContext
  ====================*/
void    CResourceInfo::InfoAddToActiveContext(ResHandle hRes)
{
    PROFILE("CResourceInfo::InfoAddToActiveContext");

    SResInfo* pInfo(LookupInfo(hRes));
    assert(pInfo != nullptr);
    if (pInfo == nullptr)
        return;

    assert(!m_stkPushedContexts.empty());
    if (m_stkPushedContexts.empty())
        return;

    ResContextMap::iterator itActiveCtx     (m_stkPushedContexts.back());
    const tstring&          sActiveCtxName  (itActiveCtx->first);
    SResContext*            pActiveCtx      (&itActiveCtx->second);

    if (pActiveCtx->setToplevelResources.insert(hRes).second && res_debugContext)
    {
        Console << _T("Protected by context '") << sActiveCtxName << _T("': ");
        g_ResourceManager.PrintResource(Console.DefaultStream(), hRes);
        Console << newl;
    }
}


/*====================
  CResourceInfo::OnResourceUnregistered
  ====================*/
void    CResourceInfo::OnResourceUnregistered(ResHandle hRes)
{
    InfoFree(hRes);
}


/*====================
  CResourceInfo::~CResourceInfo
  ====================*/
CResourceInfo::~CResourceInfo()
{
}


/*====================
  CResourceInfo::CResourceInfo
  ====================*/
CResourceInfo::CResourceInfo()
: m_sGameContextDivision(_T("client"))
, m_sGameContextCategory(_T("world"))
, m_bGameContextDirty(true)
, m_bGameContextEnabled(false)
{
}


/*====================
  CResourceInfo::IResourceGraphIterator::DoResource
  ====================*/
void    CResourceInfo::IResourceGraphIterator::DoResource(CResourceInfo::ResInfoStack& stkParents, uint uiDepth, IResource* pRes, SResInfo* pInfo)
{
    IterateResource(stkParents, uiDepth, pRes, pInfo);

    if (!pInfo->setChildren.empty())
    {
        stkParents.push_back(std::make_pair(pRes, pInfo));
        ++uiDepth;
        ForEachResHandle(ResourceSet, pInfo->setChildren, IResource* pChildRes, SResInfo* pChildInfo)
        {
            DoResource(stkParents, uiDepth, pChildRes, pChildInfo);
        }
        --uiDepth;
        stkParents.pop_back();
    }
}


/*====================
  CResourceInfo::IterateToplevel
  ====================*/
void    CResourceInfo::IterateToplevel(const IResourceGraphIterator& vIterator)
{
    IResourceGraphIterator* pIterator = const_cast<IResourceGraphIterator*>(&vIterator);
    ResInfoStack stkParents;
    ForEachResInfo(IResource* pRes, SResInfo* pInfo)
    {
        if (pInfo->HasFlags(RI_TOPLEVEL))
        {
            assert(stkParents.empty());
            pIterator->DoResource(stkParents, 0, pRes, pInfo);
            assert(stkParents.empty());
        }
    }
}


/*====================
  CResourceInfo::IterateContext
  ====================*/
void    CResourceInfo::IterateContext(const IResourceGraphIterator& vIterator, const tstring &sCtxName, SResContext* pCtx)
{
    IResourceGraphIterator* pIterator = const_cast<IResourceGraphIterator*>(&vIterator);
    if (pCtx == nullptr)
        pCtx = LookupContext(sCtxName);

    assert(pCtx != nullptr);
    if (pCtx == nullptr)
        return;

    ResInfoStack stkParents;
    ForEachResHandle(ResourceSet, pCtx->setToplevelResources,
        IResource* pRes, SResInfo* pInfo)
    {
        assert(stkParents.empty());
        pIterator->DoResource(stkParents, 0, pRes, pInfo);
        assert(stkParents.empty());
    }
}


/*====================
  CResourceInfo::LookupInfo
  ====================*/
CResourceInfo::SResInfo*    CResourceInfo::LookupInfo(ResHandle hRes, bool bAllocate)
{
    if (hRes == INVALID_RESOURCE)
        return nullptr;

    ResInfoMap::iterator itFind(m_mapResInfo.find(hRes));
    if (itFind == m_mapResInfo.end())
    {
        if (!bAllocate)
            return nullptr;

        itFind = m_mapResInfo.insert(std::make_pair(hRes, SResInfo())).first;
    }

    SResInfo& sResInfo(itFind->second);
    return &sResInfo;
}


/*====================
  CResourceInfo::LookupContext
  ====================*/
CResourceInfo::SResContext* CResourceInfo::LookupContext(const tstring &sCtxName, bool bAllocate)
{
    assert(!sCtxName.empty());
    if (sCtxName.empty())
        return nullptr;

    ResContextMap::iterator itFind(m_mapResContexts.find(sCtxName));
    if (itFind == m_mapResContexts.end())
    {
        if (!bAllocate)
            return nullptr;

        itFind = m_mapResContexts.insert(std::make_pair(sCtxName, SResContext())).first;
    }

    SResContext& sResCtx(itFind->second);
    return &sResCtx;
}


/*====================
  CResourceInfo::DeleteContext
  ====================*/
uint    CResourceInfo::DeleteContext(const ResContextMap::iterator& itContext)
{
    if (itContext == m_mapResContexts.end())
        return false;

    SResContext* pCtx(&itContext->second);

    // if the context is active, then schedule the context for deletion next time it becomes inactive.
    if (pCtx->iActivationCount > 0)
    {
        // already scheduled for deletion?
        if (pCtx->bDelete)
            return RESCTX_ALREADY_DELETING;

        pCtx->bDelete = true;
        return RESCTX_SCHEDULED_FOR_DELETION;
    }

    // delete the context.
    m_mapResContexts.erase(itContext);
    CalcOrphanedResources();
    return RESCTX_DELETED;
}


/*====================
  CResourceInfo::ActivateContextPushTop
  ====================*/
void    CResourceInfo::ActivateContextPushTop(const tstring &sCtxName)
{
    // the context name should be set!
    assert(!sCtxName.empty());
    if (sCtxName.empty())
        return;

    // allocate the context if necessary.
    SResContext* pContext(LookupContext(sCtxName, true));
    assert(pContext != nullptr);
    if (pContext == nullptr)
        return;

    ResContextMap::iterator itFind(m_mapResContexts.find(sCtxName));
    assert(itFind != m_mapResContexts.end());
    if (itFind == m_mapResContexts.end())
        return;

    SResContext* pCtx(&itFind->second);
    pCtx->iActivationCount++;

    m_stkPushedContexts.push_back(itFind);
}


/*====================
  CResourceInfo::DeactivateContextPopTop
  ====================*/
bool    CResourceInfo::DeactivateContextPopTop(const tstring &sCtxName)
{
    SResContext* pContext(LookupContext(sCtxName));
    assert(pContext != nullptr);
    if (pContext == nullptr)
        return false;

    assert(!m_stkPushedContexts.empty());
    if (m_stkPushedContexts.empty())
    {
        Console.Warn << _T("Tried to deactivate resource context, but none were active!") << newl;
        return false;
    }

    ResContextMap::iterator& itActiveCtx(m_stkPushedContexts.back());
    SResContext* pActiveCtx(&itActiveCtx->second);
    assert(sCtxName == m_stkPushedContexts.back()->first);
    if (sCtxName != m_stkPushedContexts.back()->first)
    {
        Console.Warn << _T("Tried to deactivate resource context '") << sCtxName << _T("', but it wasn't the toplevel active context!") << newl;
        return false;
    }

    assert(pActiveCtx->iActivationCount > 0);
    if (pActiveCtx->iActivationCount <= 0)
        return false;

    // deactivate the context.
    m_stkPushedContexts.pop_back();
    pActiveCtx->iActivationCount--;

    // if the context was scheduled for deletion, and it just became inactive, then delete it.
    if (pActiveCtx->bDelete && pActiveCtx->iActivationCount == 0)
    {
        if (DeleteContext(sCtxName) != RESCTX_DELETED)
            assert(!"programmer error");
    }
    // ***WARNING***: accessing variables below this line (such as itActiveCtx, sActiveCtxName, pActiveCtx, etc)
    // could result in a crash! (we might have deleted the context.)
    return true;
}


/*====================
  CResourceInfo::DeleteContext
  ====================*/
uint    CResourceInfo::DeleteContext(const tstring &sCtxName)
{
    ResContextMap::iterator itFind(m_mapResContexts.find(sCtxName));
    assert(itFind != m_mapResContexts.end());
    return DeleteContext(itFind);
}


/*====================
  CResourceInfo::CalcOrphanedResources
  ====================*/
void    CResourceInfo::CalcOrphanedResources()
{
    PROFILE("CResourceInfo::CalcOrphanedResources");

    ForEachResInfo(IResource* pRes, SResInfo* pInfo)
    {
        pInfo->SetFlags(RI_ORPAHNED);
    }

    ForEachResContext(const tstring* sCtxName, SResContext* pCtx)
    {
        ForEachResChain(const ResInfoStack& stkParents, uint uiDepth, IResource* pRes, SResInfo* pInfo)
        {
            pInfo->ClearFlags(RI_ORPAHNED);
        }
        ForEachResChainEnd_DoContext(*sCtxName, pCtx);
    }
}


/*====================
  CResourceInfo::PrintContext
  ====================*/
void    CResourceInfo::PrintContext(CConsoleStream& cStream, const tstring &sCtxName, const tstring &sWildcardChildren, SResContext* pCtx)
{
    if (pCtx == nullptr)
        pCtx = LookupContext(sCtxName);

    if (pCtx == nullptr)
    {
        cStream << _T("^r[nullptr context]");
        return;
    }

    cStream << _T("^w[ResContext ^y") << sCtxName << _T("^w] (^c") << (uint)pCtx->setToplevelResources.size() << _T("^w resources) ") << newl;

    if (!sWildcardChildren.empty())
    {
        ForEachResHandle(ResourceSet, pCtx->setToplevelResources,
            IResource* pResource, SResInfo* pInfo)
        {
            if (pResource->MatchesWildcard(sWildcardChildren))
            {
                g_ResourceManager.PrintResource(cStream, pResource);
                cStream << newl;
            }
        }
    }
}


/*====================
  CResourceInfo::ExecCommand
  ====================*/
bool    CResourceInfo::ExecCommand(const tstring &sCommand, const tsvector& vArgs)
{
    PROFILE("CResourceInfo::ExecCommand");

    //*********************
    // ResourceCmdEx graph
    //*********************
    if (sCommand == _T("graph"))
    {
        if (vArgs.size() < 1)
        {
            Console << _T("^r'ResourceCmdEx graph' command failed (not enough args): specify a graph command.") << newl;
            return false;
        }
        const tstring &sGraphCmd(vArgs[0]);

        if (sGraphCmd == _T("list"))
        {
            tstring sWild(_T("*"));
            if (vArgs.size() >= 2)
                sWild = vArgs[1];

            ForEachResInfo(IResource* pRes, SResInfo* pInfo)
            {
                pInfo->ClearFlags(RI_RELEVANT | RI_MATCHED);
                if (pRes->MatchesWildcard(sWild))
                    pInfo->SetFlags(RI_MATCHED);
            }

            ForEachResChain(const ResInfoStack& stkParents, uint uiDepth, IResource* pRes, SResInfo* pInfo)
            {
                if (!pInfo->HasFlags(RI_MATCHED))
                    return;

                pInfo->SetFlags(RI_RELEVANT);
                ForEachResChain_ForEachParent(stkParents, IResource* pParentRes, SResInfo* pParentInfo)
                {
                    pParentInfo->SetFlags(RI_RELEVANT);
                }
            }
            ForEachResChainEnd_DoAllToplevel();

            ForEachResChain(const ResInfoStack& stkParents, uint uiDepth, IResource* pRes, SResInfo* pInfo)
            {
                bool bExactMatch(false);
                bool bMatched(pInfo->HasFlags(RI_MATCHED));

                if (!pInfo->HasFlags(RI_MATCHED))
                    if (bExactMatch || !pInfo->HasFlags(RI_RELEVANT))
                        return;

                CConsoleStream& cStream(Console.DefaultStream());

                if (uiDepth == 0)
                    cStream << newl;

                if (bMatched)
                    cStream << _T("^m");
                else
                    cStream << _T("^v");

                // print the indentation.
                if (uiDepth == 0)
                    cStream << _T("> ");
                else
                {
                    cStream << _T("  ");
                    for (uint i(0); i < uiDepth; ++i)
                    {
                        cStream << _T("* ");
                    }
                }

                // print the resource.
                g_ResourceManager.PrintResource(cStream, pRes,
                    false, 
                    (bMatched ? nullptr : _T("^v")));
                cStream << newl;
            }
            ForEachResChainEnd_DoAllToplevel();

            return true;
        }

        Console << _T("^r'ResourceCmdEx graph ") << sGraphCmd << _T("' failed: unknown graph command") << newl;
        return false;
    }

    //*********************
    // ResourceCmdEx context
    //*********************
    if (sCommand == _T("context"))
    {
        if (vArgs.size() < 1)
        {
            Console << _T("^r'ResourceCmdEx context' command failed (not enough args): specify a context command.") << newl;
            return false;
        }
        const tstring &sContextCmd(vArgs[0]);

        //*********************
        // ResourceCmdEx context activate <string sContextName>
        //*********************
        if (sContextCmd == _T("push") || sContextCmd == _T("activate"))
        {
            if (vArgs.size() < 2)
            {
                Console << _T("^r'ResourceCmdEx context push' command failed (not enough args): specify a context name.") << newl;
                return false;
            }
            const tstring &sContextName(vArgs[1]);

            ActivateContextPushTop(sContextName);

            return true;
        }

        //*********************
        // ResourceCmdEx context deactivate <string sContextName>
        //*********************
        if (sContextCmd == _T("pop") || sContextCmd == _T("deactivate"))
        {
            if (vArgs.size() < 2)
            {
                Console << _T("^r'ResourceCmdEx context pop' command failed (not enough args): specify a context name.") << newl;
                return false;
            }
            const tstring &sContextName(vArgs[1]);

            if (!DeactivateContextPopTop(sContextName))
            {
                Console << _T("^rFailed to pop context ") << sContextName << _T(": it is not the topmost context.") << newl;
                return false;
            }

            return true;
        }

        //*********************
        // ResourceCmdEx context delete <string sContextName>
        //*********************
        if (sContextCmd == _T("delete"))
        {
            try
            {
                if (vArgs.size() < 2)
                    EX_ERROR(_T("not enough args; specify the context you want to delete"));
                const tstring &sContextName(vArgs[1]);

                // is it a wildcard deletion?
                bool bWildcard(false);
                const tstring &sWildcard(sContextName);
                if (sContextName.find(_T('*')) != tstring::npos)
                    bWildcard = true;

                // global context.
                SResContext* pGlobalCtx(nullptr);
                if (!m_stkPushedContexts.empty())
                    pGlobalCtx = &((*m_stkPushedContexts.front()).second);

                for (ResContextMap::iterator it(m_mapResContexts.begin()), itNext, itEnd(m_mapResContexts.end()); it != itEnd; it = itNext)
                {
                    const tstring&  sCurCtxName(it->first);
                    SResContext*    pCtx(&it->second);

                    itNext = it;
                    ++itNext;

                    // never delete the global context.
                    if (pCtx == pGlobalCtx)
                        continue;

                    if (bWildcard)
                    {
                        // wildcard name match
                        if (!EqualsWildcards(sWildcard, sCurCtxName))
                            continue;
                    }
                    else
                    {
                        // exact name match
                        if (sCurCtxName != sContextName)
                            continue;
                    }

                    tstring sCurCtxNameCopy(sCurCtxName);
                    if (uint uiResult = DeleteContext(sCurCtxNameCopy))
                    {
                        if (uiResult == RESCTX_DELETED)
                            Console << _T("ResContext: deleted context '^y") << sCurCtxNameCopy << _T("^*'") << newl;
                        else if (uiResult == RESCTX_SCHEDULED_FOR_DELETION)
                            Console << _T("ResContext: scheduled context '^y") << sCurCtxNameCopy << _T("^*' to be deleted when it goes inactive.") << newl;
                        else if (uiResult == RESCTX_ALREADY_DELETING)
                            Console << _T("ResContext: context '^y") << sCurCtxNameCopy << _T("^*' was already scheduled to be deleted when it goes inactive.") << newl;
                        else
                            assert(false);
                    }
                    else
                    {
                        Console << _T("ResContext: Failed to delete '^y") << sCurCtxNameCopy << _T("^*'") << newl;
                    }
                }

                return true;
            }
            catch (CException& ex)
            {
                ex.Process(_T("'ResourceCmdEx context delete <contextName>' command: "), NO_THROW);
                return false;
            }
        }

        //*********************
        // ResourceCmdEx context copy <string sSrcContextName> <string sDstContextName>
        //*********************
        if (sContextCmd == _T("copy") || sContextCmd == _T("clone") || sContextCmd == _T("cp"))
        {
            try
            {
                if (vArgs.size() < 3)
                    EX_ERROR(_T("not enough args"));

                const tstring &sSrcContextName(vArgs[1]);
                const tstring &sDstContextName(vArgs[2]);

                SResContext* pSrcContext(LookupContext(sSrcContextName));
                if (pSrcContext == nullptr)
                    EX_ERROR(_TS("unknown source context '") + sSrcContextName + _T("'"));

                bool bReplacing(false);
                uint uiPrevResCount(0);

                SResContext* pDstContext(LookupContext(sDstContextName));
                if (pDstContext != nullptr)
                {
                    bReplacing = true;
                    uiPrevResCount = (uint)pDstContext->setToplevelResources.size();
                }
                else
                    pDstContext = LookupContext(sDstContextName, true);
                assert(pDstContext != nullptr);

                pDstContext->setToplevelResources = pSrcContext->setToplevelResources;

                if (bReplacing)
                {
                    Console << _T("ResContext: replaced '^y") << sDstContextName << _T("^*' (") << uiPrevResCount << _T(" resources)");
                    Console << _T(" with '^y") << sSrcContextName << _T("^*' (^c") << (uint)pSrcContext->setToplevelResources.size() << _T("^* resources)");
                    Console << newl;
                }
                else
                {
                    Console << _T("ResContext: created '^y") << sDstContextName << _T("^*' and copied contents");
                    Console << _T(" from '^y") << sSrcContextName << _T("^*' (^c") << (uint)pSrcContext->setToplevelResources.size() << _T("^* resources)");
                    Console << newl;
                }

                return true;
            }
            catch (CException& ex)
            {
                ex.Process(_T("'ResourceCmdEx context copy <srcContextName> <dstContextName>' command: "), NO_THROW);
                return false;
            }
            return false;
        }

        //*********************
        // ResourceCmdEx context move <string sSrcContextName> <string sDstContextName>
        //*********************
        if (sContextCmd == _T("move") || sContextCmd == _T("mv"))
        {
            try
            {
                if (vArgs.size() < 3)
                    EX_ERROR(_T("not enough args"));

                const tstring &sSrcContextName(vArgs[1]);
                const tstring &sDstContextName(vArgs[2]);

                SResContext* pSrcContext(LookupContext(sSrcContextName));
                if (pSrcContext == nullptr)
                    EX_ERROR(_TS("unknown source context '") + sSrcContextName + _T("'"));

                bool bReplacing(false);
                uint uiPrevResCount(0);

                SResContext* pDstContext(LookupContext(sDstContextName));
                if (pDstContext != nullptr)
                {
                    bReplacing = true;
                    uiPrevResCount = (uint)pDstContext->setToplevelResources.size();
                }
                else
                {
                    pDstContext = LookupContext(sDstContextName, true);
                }
                assert(pDstContext != nullptr);

                pDstContext->setToplevelResources = pSrcContext->setToplevelResources;

                if (bReplacing)
                {
                    Console << _T("ResContext: replaced '^y") << sDstContextName << _T("^*' (") << uiPrevResCount << _T(" resources)");
                    Console << _T(" with '^y") << sSrcContextName << _T("^*' (^c") << (uint)pSrcContext->setToplevelResources.size() << _T("^* resources)");
                    Console << _T(" and deleted '^y") << sSrcContextName << _T("^*'");
                    Console << newl;
                }
                else
                {
                    Console << _T("ResContext: created '^y") << sDstContextName << _T("^*', copied contents ");
                    Console << _T(" from '^y") << sSrcContextName << _T("^*' (^c") << (uint)pSrcContext->setToplevelResources.size() << _T("^* resources), ");
                    Console << _T(" and deleted '^y") << sSrcContextName << _T("^*'");
                    Console << newl;
                }

                DeleteContext(sSrcContextName);

                return true;
            }
            catch (CException& ex)
            {
                ex.Process(_T("'ResourceCmdEx context move <srcContextName> <dstContextName>' command: "), NO_THROW);
                return false;
            }
            return false;
        }

        //*********************
        // ResourceCmdEx context exists <string sContextName>
        //*********************
        if (sContextCmd == _T("exists"))
        {
            try
            {
                if (vArgs.size() < 2)
                    EX_ERROR(_T("not enough args"));

                const tstring &sContextName(vArgs[0]);

                SResContext* pSrcContext(LookupContext(sContextName));
                if (pSrcContext == nullptr)
                    Console << XtoA(_T("false"));
                else
                    Console << XtoA(_T("true"));
                Console << newl;

                return true;
            }
            catch (CException& ex)
            {
                ex.Process(_T("'ResourceCmdEx context exists <contextName>' command: "), NO_THROW);
                return false;
            }
            return false;
        }

        //*********************
        // ResourceCmdEx context list <string sWildcardContextNames>
        //*********************
        if (sContextCmd == _T("list"))
        {
            tstring sWildcardContext(_T("*"));
            tstring sWildcardContextChildren; // don't print any context resources unless specified.
            if (vArgs.size() >= 2)
                sWildcardContext = vArgs[1];
            if (vArgs.size() >= 3)
                sWildcardContextChildren = vArgs[2];

            ForEachResContext(const tstring* sCtxName, SResContext* pCtx)
            {
                if (EqualsWildcards(sWildcardContext, *sCtxName))
                    PrintContext(Console.DefaultStream(), *sCtxName, sWildcardContextChildren, pCtx);
            }

            return true;
        }

        Console << _T("^r'ResourceCmdEx context ") << sContextCmd << _T("' failed: unknown context command") << newl;
        return true;
    }

    //*********************
    // ResourceCmdEx orphans
    //*********************
    if (sCommand == _T("orphans"))
    {
        if (vArgs.size() < 1)
        {
            Console << _T("^r'ResourceCmdEx orphans' command failed (not enough args): specify an orphans command.") << newl;
            return false;
        }
        const tstring &sOrphansCmd(vArgs[0]);


        //*********************
        // ResourceCmdEx orphans list
        //*********************
        if (sOrphansCmd == _T("list"))
        {
            CalcOrphanedResources();

            ForEachResInfo(IResource* pRes, SResInfo* pInfo)
            {
                if (pInfo->HasFlags(RI_ORPAHNED))
                {
                    Console << _T("Orphaned ");
                    g_ResourceManager.PrintResource(Console.DefaultStream(), pRes);
                    Console << newl;
                }
            }

            return true;
        }


        //*********************
        // ResourceCmdEx orphans unregister
        //*********************
        if (sOrphansCmd == _T("unregister"))
        {
            CalcOrphanedResources();

            ResourceSet setUnregister;

            ForEachResInfo(IResource* pRes, SResInfo* pInfo)
            {
                switch (pRes->GetResType())
                {
                case RES_TEXTURE:
                case RES_MATERIAL:
                case RES_MODEL:
                case RES_CLIP:
                case RES_SAMPLE:
                case RES_EFFECT:
                    {
                        if (pInfo->HasFlags(RI_ORPAHNED))
                        {
                            ResHandle hRes(pRes->GetHandle());
                            setUnregister.insert(hRes);
                        }
                    }
                    break;
                }
            }

            while (!setUnregister.empty())
            {
                ResHandle hRes(*setUnregister.begin());
                g_ResourceManager.Unregister(hRes, UNREG_RESERVE_HANDLE);
                setUnregister.erase(setUnregister.begin());
            }

            return true;
        }

        Console << _T("^r'ResourceCmdEx orphans ") << sOrphansCmd << _T("' failed: unknown orphans command") << newl;
        return true;
    }

    Console << _T("^rUnknown resource context command: ") << sCommand << newl;
    return false;
}


/*====================
  CResourceInfo::ExecCommandLine
  ====================*/
bool    CResourceInfo::ExecCommandLine(const tstring &sCommandLine)
{
    size_t uiPos(sCommandLine.find_first_of(_T(' ')));

    // no args?
    if (uiPos == tstring::npos)
        return ExecCommand(sCommandLine);

    // execute with args.
    tsvector vArgs;
    SplitBy(vArgs, sCommandLine, _TS(" "), uiPos + 1, SPLITBY_ERASE_EMPTY_SPLITS);
    assert(vArgs.size() >= 1);
    return ExecCommand(sCommandLine.substr(0, uiPos), vArgs);
}

//=============================================================================

/*--------------------
  ResourceCmdEx
  --------------------*/
CMD(ResourceCmdEx)
{
    if (vArgList.size() < 1)
    {
        Console << _T("ResourceCmdEx <string sCommand>") << newl;
        return false;
    }

    const tstring &sCommand(LowerString(vArgList[0]));
    tsvector vArgs(vArgList.begin() + 1, vArgList.end());
    g_ResourceInfo.ExecCommand(sCommand, vArgs);
    return true;
}


/*--------------------
  ListResourceGraph
  --------------------*/
CMD(ListResourceGraph)
{
    tstring sWildcard(_T("*"));
    if (vArgList.size() >= 1)
        sWildcard = vArgList[0];

    tsvector vArgs;
    vArgs.push_back(sWildcard);
    for (size_t i(1); i < vArgList.size(); ++i)
        vArgs.push_back(vArgList[i]);

    g_ResourceInfo.ExecCommand(_T("graph"), vArgs);
    return true;
}
