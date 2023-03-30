// (C)2005 S2 Games
// c_uitriggerregistry.h
//
//=============================================================================
#ifndef __C_UITRIGGERREGISTRY_H__
#define __C_UITRIGGERREGISTRY_H__

//=============================================================================
// Declarations
//=============================================================================
class CUITrigger;
class CUIWatcher;

extern K2_API class CUITriggerRegistry *g_pUITriggerRegistry;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef map<tstring, CUITrigger*>   UITriggerMap;
typedef UITriggerMap::iterator      UITriggerMap_it;

typedef list<CUIWatcher*>           UIWatcherList;
typedef UIWatcherList::iterator     UIWatcherList_it;
typedef map<tstring, UIWatcherList> UIWatcherMap;
typedef UIWatcherMap::iterator      UIWatcherMap_it;

#ifdef K2_EXPORTS
#define UITriggerRegistry   (*CUITriggerRegistry::GetInstance())
#else
#define UITriggerRegistry   (*g_pUITriggerRegistry)
#endif
//=============================================================================

//=============================================================================
// CUITriggerRegistry
//=============================================================================
class CUITriggerRegistry
{
    SINGLETON_DEF(CUITriggerRegistry);

private:
    UITriggerMap        m_mapUITriggers;
    UIWatcherMap        m_mapWatchers;

public:
    ~CUITriggerRegistry()   {}

    void                    Register(CUITrigger *pUITrigger);
    void                    Unregister(CUITrigger *pUITrigger);

    void                    Register(CUIWatcher *pUIWatcher);
    void                    Unregister(CUIWatcher *pUIWatcher);

    inline CUITrigger*      GetUITrigger(const tstring &sUITrigger);

    const UITriggerMap&     GetUITriggerMap()   { return m_mapUITriggers; }

    inline bool             Exists(const tstring &sUITrigger);
};
//=============================================================================

/*====================
  CUITriggerRegistry::Exists
  ====================*/
bool    CUITriggerRegistry::Exists(const tstring &sUITrigger)
{
    UITriggerMap::iterator find = m_mapUITriggers.find(sUITrigger);

    if (find == m_mapUITriggers.end())
        return false;

    return true;
}


/*====================
  CUITriggerRegistry::GetUITrigger
  ====================*/
CUITrigger  *CUITriggerRegistry::GetUITrigger(const tstring &sUITrigger)
{
    UITriggerMap::iterator find = m_mapUITriggers.find(sUITrigger);

    if (find == m_mapUITriggers.end())
        return NULL;
    else
        return find->second;
}
//=============================================================================
#endif //__C_UITRIGGERREGISTRY_H__
