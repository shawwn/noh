// (C)2005 S2 Games
// c_uimanager.h
//
//=============================================================================
#ifndef __C_UIMANAGER_H__
#define __C_UIMANAGER_H__

//=============================================================================
// Declarations
//=============================================================================
class CUIManager;
class IWidget;
class CInterface;

extern K2_API class CUIManager *g_pUIManager;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef map<tstring, ResHandle>         InterfaceMap;
typedef InterfaceMap::iterator          InterfaceMap_it;
typedef InterfaceMap::const_iterator    InterfaceMap_cit;

typedef list<InterfaceMap_it>           OverlayList;
typedef OverlayList::iterator           OverlayList_it;

#ifndef K2_EXPORTS
#define UIManager (*g_pUIManager)
#else
#define UIManager (*CUIManager::GetInstance())
#endif

enum EDestructionStep
{
    UI_DESTROY_TOTAL,
    UI_DESTROY_REFERENCES,
    UI_DESTROY_REMOVE_FROM_INTERFACE,
    UI_DESTROY_NOTIFY_PARENT,
    UI_DESTROY_NOTIFY_WATCHERS,

    UI_DESTROY_NUM_STEPS
};
//=============================================================================

//=============================================================================
// CUIManager
//=============================================================================
class CUIManager
{
    SINGLETON_DEF(CUIManager)

private:
    InterfaceMap        m_mapInterfaces;
    InterfaceMap_it     m_itActiveInterface;
    InterfaceMap_it     m_itSavedActiveInterface;

    ResHandle           m_hStringTable;

    OverlayList         m_lOverlayInterfaces;

    bool                m_bRefreshCursor;

    sset                m_setDeferedReloads;

public:
    ~CUIManager();

    void                Initialize();

    K2_API ResHandle    LoadInterface(const tstring &sFilename);
    K2_API void         UnloadInterface(ResHandle hInterface);
    K2_API void         UnloadInterface(const tstring &sName);
    K2_API void         ReloadInterface(const tstring &sName);
    K2_API CInterface*  GetInterface(ResHandle handle) const;
    K2_API CInterface*  GetInterface(const tstring &sName) const;

    K2_API void         ResizeAllInterfaces(float fWidth, float fHeight);
    K2_API void         ResizeInterface(const tstring &sInterface, float fWidth, float fHeight);

    void                DeferReload(const tstring &sName)   { m_setDeferedReloads.insert(sName); }
    void                ProcessDeferedReloads();

    K2_API CInterface*  GetActiveInterface() const;
    K2_API void         SetActiveInterface(const tstring &sName);

    K2_API CInterface*  GetSavedActiveInterface() const;

    K2_API bool         IsOverlayInterface(CInterface *pInterface);
    K2_API bool         IsOverlayInterface(const tstring &sName);
    K2_API bool         IsOverlayInterface(ResHandle hInterface);
    K2_API bool         AddOverlayInterface(const tstring &sName);
    K2_API void         RemoveOverlayInterface(const tstring &sName);
    K2_API void         ClearOverlayInterfaces();
    K2_API void         BringOverlayToFront(const tstring &sName);

    K2_API void         LostInterface(CInterface *pInterface);

    void                PrintInterfaceList() const;

    K2_API IWidget*     FindWidget(const tstring &sName);
    K2_API uint         FindWidgetsByWildcards(vector<IWidget*>& vWidgets, const tstring &sWildcards, bool bSearchActiveInterfaceOnly = false);

    K2_API void         ProcessInput();
    K2_API bool         ProcessInputAxis(EAxis axis, float fValue);
    K2_API bool         ProcessInputCursor(const CVec2f &v2CursorPos);
    K2_API bool         ProcessInputMouseButton(const CVec2f &v2CursorPos, EButton button, float fValue);
    K2_API bool         ProcessInputButton(EButton button, bool bDown, bool bPressed);
    K2_API bool         ProcessInputChar(TCHAR c);
    K2_API uint         GetMinimapHoverUnit();

    K2_API tstring      GetCopyString();
    K2_API void         PasteString(const tstring &sString);

    K2_API void         Frame(uint uiFrameLength);
    void                DrawGrid();
    
    K2_API void         Render(ResHandle hInterface, const CVec2f &v2Pos = V2_ZERO);

    void                RefreshCursor()             { m_bRefreshCursor = true; }

    K2_API bool         NeedsRefresh();
    K2_API void         ResetRefresh();

    K2_API tstring      Translate(const tstring &sKey, const tsmapts &mapTokens = SMAPS_EMPTY);
    K2_API ResHandle    GetTranslationStringTable() const { return m_hStringTable; }

    K2_API void         UnloadTempInterfaces();
};
//=============================================================================

#endif //__C_UIMANAGER_H__
