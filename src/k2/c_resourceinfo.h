// (C)2010 S2 Games
// c_resourceinfo.h
//
//=============================================================================
#ifndef __C_RESOURCEINFO_H__
#define __C_RESOURCEINFO_H__

//=============================================================================
// Headers
//=============================================================================
//=============================================================================

//=============================================================================
// Resource Context Macros
//=============================================================================

// activate a resource context for the remainder of the current scope.
#define K2_RESOURCE_CONTEXT(sResourceContextName)                                                   \
    CResourceScope  K2_PP_CAT(cActivateResContext_, __LINE__)(sResourceContextName)

// creates a new scope, then activates a resource context for the duration of that scope.
#define K2_WITH_RESOURCE_SCOPE(sResourceContextName)                                                \
    if (const CResourceScope& K2_PP_CAT(cActivateResContext_, __LINE__) = CResourceScope(sResourceContextName))

// game scope.
#define K2_GAME_RESOURCE_CONTEXT()                  K2_RESOURCE_CONTEXT(g_ResourceInfo.GetGameContext())
#define K2_WITH_GAME_RESOURCE_SCOPE()               K2_WITH_RESOURCE_SCOPE(g_ResourceInfo.GetGameContext())

// global scope.
#define K2_GLOBAL_RESOURCE_CONTEXT()                K2_RESOURCE_CONTEXT(_T("global"))
#define K2_WITH_GLOBAL_RESOURCE_SCOPE()             K2_WITH_RESOURCE_SCOPE(_T("global"))
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
extern K2_API class CResourceInfo& g_ResourceInfo;

class CGraphResource;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef hash_set<ResHandle>     ResourceSet;
typedef uint                    ResCtx;
//=============================================================================

//=============================================================================
// CGraphResource
//=============================================================================
class CGraphResource
{
private:
    static CGraphResource*  s_pCurrentParent;

    // for debugging purposes only.
#ifdef _DEBUG
    tstring             m_sPath;
#endif

    CGraphResource*     m_pPrevParent;
    ResourceSet         m_setChildren;
    ResHandle           m_hResource;
    bool                m_bValid;
    bool                m_bDone;
    bool                m_bLinking;

public:
    ~CGraphResource();
    CGraphResource();

    // for debugging purposes only.
    void            SetDebugPath(const tstring &sPath);

    void            SetHandle(ResHandle hResource);
    void            LinkChildren(ResHandle hResource);

    void            Reset();
    void            Cancel();
    void            Done();

    static void     LinkExistingChild(ResHandle hResource);
};
//=============================================================================

//=============================================================================
// CResourceInfo
//=============================================================================
class CResourceInfo
{
private:
    SINGLETON_DEF(CResourceInfo)

    //*********************
    // Friend Classes
    //*********************

    friend class CResourceScope;

    //*********************
    // Private Declarations
    //*********************

    struct SResInfo;
    struct SResContext;
    class IResourceGraphIterator;

    //*********************
    // Private Definitions
    //*********************

    static const uint RI_TOPLEVEL   = BIT(0);
    static const uint RI_ORPAHNED   = BIT(1);
    static const uint RI_TOUCHED    = BIT(2);
    static const uint RI_MATCHED    = BIT(3);
    static const uint RI_RELEVANT   = BIT(4);

    typedef pair<IResource*, SResInfo*>     ResInfoPair;
    typedef vector<ResInfoPair>             ResInfoStack;
    typedef hash_map<tstring, bool>         ResCategoryLoadMap;

    //*********************
    // Internal Interfaces
    //*********************

    // CGraphResource
    friend class CGraphResource;
    SResInfo*           InfoAlloc(ResHandle hRes);
    void                InfoFree(ResHandle hRes);
    void                InfoClearChildren(ResHandle hRes);
    void                InfoAddChildren(ResHandle hRes, const ResourceSet& setChildren);
    void                InfoAddToActiveContext(ResHandle hRes);

    // CResourceManager
    friend class CResourceManager;
    void                OnResourceUnregistered(ResHandle hRes);

    //*********************
    // Private Definitions
    //*********************

    // SResInfo
    struct SResInfo
    {
        ResourceSet     setChildren;
        uint            uiFlags;

        SResInfo()
            : uiFlags(0)
        {}

        bool            HasFlags(uint uiInfoFlags) const    { return (this->uiFlags & uiInfoFlags) != 0; }
        void            ClearFlags(uint uiInfoFlags)        { this->uiFlags &= ~uiInfoFlags; }
        void            SetFlags(uint uiInfoFlags)          { this->uiFlags |= uiInfoFlags; }
    };
    typedef hash_map<ResHandle, SResInfo>       ResInfoMap;

    // SResContext
    struct SResContext
    {
        ResourceSet     setToplevelResources;
        int             iActivationCount;
        bool            bDelete;

        SResContext()
            : iActivationCount(0)
            , bDelete(false)
        {}
    };
    typedef hash_map<tstring, SResContext>      ResContextMap;
    typedef vector<ResContextMap::iterator>     ResContextStack;

    // IResourceGraphIterator
    class IResourceGraphIterator
    {
        friend class CResourceInfo;
        void DoResource(ResInfoStack& stkParents, uint uiDepth, IResource* pRes, SResInfo* pInfo);

    public:
        virtual void IterateResource(const ResInfoStack& stkParents, uint uiDepth, IResource* pRes, SResInfo* pInfo)=0;
    };
    void                IterateToplevel(const IResourceGraphIterator& pIterator);
    void                IterateContext(const IResourceGraphIterator& pIterator, const tstring &sCtxName, SResContext* pCtx);

    //*********************
    // Member Variables
    //*********************

    ResInfoMap          m_mapResInfo;
    ResContextMap       m_mapResContexts;
    ResContextStack     m_stkPushedContexts;
    ResCategoryLoadMap  m_mapResCategoriesLoaded;

    // the current game resource division: 'client' or 'server'
    tstring             m_sGameContextDivision;
    
    // the current game resource category: 'world' or 'hero'
    tstring             m_sGameContextCategory;

    // the current game resource context.
    mutable tstring     m_sGameContext;
    mutable bool        m_bGameContextDirty;

    // whether the game context is enabled.
    bool                m_bGameContextEnabled;

    //*********************
    // Private Methods
    //*********************

    SResInfo*           LookupInfo(ResHandle hRes, bool bAllocate = false);
    SResContext*        LookupContext(const tstring &sCtxName, bool bAllocate = false);
    uint                DeleteContext(const ResContextMap::iterator& itContext);

    //*********************
    // Public Interface
    //*********************
public:
    ~CResourceInfo();

    // resource context methods.
    void                ActivateContextPushTop(const tstring &sCtxName);
    bool                DeactivateContextPopTop(const tstring &sVerifyCtxName);

    K2_API uint         DeleteContext(const tstring &sCtxName);
    // DeleteContext returns one of the following:
    static const uint   RESCTX_DELETED                  = 1; // the specified context was deleted.
    static const uint   RESCTX_SCHEDULED_FOR_DELETION   = 2; // the specified context is currently active, and will be deleted when it goes inactive.
    static const uint   RESCTX_ALREADY_DELETING         = 3; // the specified context is currently active, and was already scheduled for deletion when it goes inactive.
    // (a return value of 0 means "failed to delete the specified context")

    // resource graph methods.
    void                CalcOrphanedResources();

    // printing.
    K2_API void         PrintContext(CConsoleStream& cStream, const tstring &sCtxName, const tstring &sWildcardChildren = _T("*"), SResContext* pCtx = nullptr);

    // various console commands.
    K2_API bool         ExecCommand(const tstring &sCommand, const tsvector& vArgs = tsvector());
    K2_API bool         ExecCommandLine(const tstring &sCommandLine);

    // game resource context methods.
    K2_API bool             GetGameContextEnabled() const                       { return m_bGameContextEnabled; }
    K2_API void             SetGameContextEnabled(bool bEnabled)                { if (bEnabled != m_bGameContextEnabled) { m_bGameContextEnabled = bEnabled; m_bGameContextDirty = true; } }
    //      the current game resource division: 'client' or 'server'
    K2_API const tstring&   GetGameContextDivision() const                      { return m_sGameContextDivision; }
    K2_API void             SetGameContextDivision(const tstring &sCur)         { if (m_sGameContextDivision != sCur) { m_sGameContextDivision = sCur; m_bGameContextDirty = true; } }
    //      the current game resource category: 'world' or 'hero'
    K2_API const tstring&   GetGameContextCategory() const                      { return m_sGameContextCategory; }
    K2_API void             SetGameContextCategory(const tstring &sCur)         { if (m_sGameContextCategory != sCur) { m_sGameContextCategory = sCur; m_bGameContextDirty = true; } }
    K2_API void             SetGameContextCategoryLoaded(const tstring &sCur, bool bSet)    { m_mapResCategoriesLoaded[sCur] = bSet; }
    K2_API bool             WasGameContextCategoryLoaded(const tstring &sCur)   { if (m_mapResCategoriesLoaded.find(sCur) == m_mapResCategoriesLoaded.end()) { return false; } return m_mapResCategoriesLoaded[sCur]; }
    //      the current game resource context.
    K2_API const tstring&   GetGameContext() const;
};
//=============================================================================


/*====================
  CResourceInfo::GetGameContext
  ====================*/
inline
const tstring&  CResourceInfo::GetGameContext() const
{
    if (m_bGameContextDirty)
    {
        if (m_bGameContextEnabled)
            m_sGameContext = m_sGameContextDivision + _T(":curgame_") + m_sGameContextCategory;
        else
            m_sGameContext = _T("global");
        m_bGameContextDirty = false;
    }
    return m_sGameContext;
}


//=============================================================================
// CResourceScope
//
//  a helper object.  Its constructor activates the specified context (creating
//  it if necessary), and its destructor deactivates that context.
//=============================================================================
class CResourceScope
{
private:
    CResourceInfo::ResContextMap::iterator      m_itContext;

public:
    CResourceScope(const tstring &sResourceContextName)
    {
        assert(!sResourceContextName.empty());
        assert(sResourceContextName.find(_T(" ")) == tstring::npos);

        g_ResourceInfo.ExecCommandLine(_TS("context push ") + sResourceContextName);
        m_itContext = g_ResourceInfo.m_mapResContexts.find(sResourceContextName);
        assert(m_itContext != g_ResourceInfo.m_mapResContexts.end());
    }

    ~CResourceScope()
    {
        if (m_itContext == g_ResourceInfo.m_mapResContexts.end())
            return;

        const tstring &sCtxName(m_itContext->first);
        g_ResourceInfo.ExecCommandLine(_TS("context pop ") + sCtxName);
    }

    // this is necessary for the following usage:
    //
    //      if (CResourceScope cScope)
    //
    // in other words, this allows if-statement-scoping, which is common
    // for certain macros.
    operator bool() const       { return true; }
};
//=============================================================================

#endif //__C_RESOURCEINFO_H__
