// (C)2005 S2 Games
// c_uitrigger.h
//
//=============================================================================
#ifndef __C_UITRIGGER_H__
#define __C_UITRIGGER_H__

//=============================================================================
// Declarations
//=============================================================================
class IWidget;
class CUITrigger;
class CBufferStatic;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef list<IWidget *>         WatcherList;
typedef WatcherList::iterator   WatcherList_it;

// Declaration macros
#define UI_TRIGGER(name) CUITrigger name(_T(#name))
//=============================================================================

//=============================================================================
// CUITrigger
//=============================================================================
class CUITrigger
{
private:
    tstring             m_sName;
    WatcherList         m_lWatchers;
    vector<WatcherList> m_vlWatchers;
    tstring             m_sLastParam;

    // CUITriggers should not be copied
    CUITrigger(CUITrigger&);
    CUITrigger& operator=(CUITrigger&);

public:
    K2_API ~CUITrigger();
    K2_API CUITrigger(const tstring &sName);

    const tstring&  GetName()           { return m_sName; }
    const tstring&  GetLastParam()      { return m_sLastParam; }

    K2_API void AddWatcher(IWidget *pWidget, int iIndex = -1);
    K2_API void RemoveWatcher(IWidget *pWidget, int iIndex = -1);
    K2_API void Trigger(const tstring &sParam, bool bForce = false);
    K2_API void Trigger(const tsvector &vParams, bool bForce = false);

    K2_API void Hide();
    K2_API void Show();

    K2_API bool IsVisible();

    K2_API void Execute(const tstring &sCmd);
    K2_API void Execute(const tstring &sCmd, IBuffer &buffer);
};
//=============================================================================

//=============================================================================
// CUIWatcher
//=============================================================================
class CUIWatcher
{
private:
    tstring     m_sTriggerName;
    CUITrigger* m_pTrigger;
    IWidget*    m_pWidget;
    int         m_iIndex;

    CUIWatcher();

public:
    ~CUIWatcher();
    CUIWatcher(IWidget *pWidget, const tstring &sTriggerName, int iIndex = -1);

    tstring GetTriggerName()                    { return m_sTriggerName; }

    void    SetTrigger(CUITrigger *pTrigger);
};
//=============================================================================

#endif //__C_UITRIGGER_H__
