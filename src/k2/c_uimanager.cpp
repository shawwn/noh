// (C)2005 S2 Games
// c_uimanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_uimanager.h"
#include "c_xmlproc_interface.h"
#include "c_interface.h"
#include "c_uicmd.h"
#include "c_uimanager.h"
#include "c_interfaceresource.h"
#include "c_xmlmanager.h"
#include "c_cmd.h"
#include "c_filemanager.h"
#include "c_vid.h"
#include "c_draw2d.h"
#include "c_function.h"
#include "c_input.h"
#include "c_action.h"
#include "c_uitrigger.h"
#include "c_clientlogin.h"
#include "c_hostclient.h"
#include "c_chatmanager.h"
#include "c_stringtable.h"
#include "c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_BOOLEX     (ui_drawGrid,           false,  CVAR_SAVECONFIG, nullptr);
CVAR_BOOL       (ui_draw,               true);
CVAR_BOOL       (ui_debugHoverWidget,   false);
CVAR_BOOL       (ui_reloadInterfaces,   false);
CVAR_BOOL       (ui_debugInterface,     false);
CVAR_STRING     (ui_highlightWidgets,   "");

SINGLETON_INIT(CUIManager);
CUIManager *g_pUIManager(CUIManager::GetInstance());

UI_TRIGGER(ShowCCPanel);
UI_TRIGGER(HideCCPanel);
UI_TRIGGER(ToggleCCPanel);
UI_TRIGGER(CCStatisticsVisible);
//=============================================================================

/*====================
 CUIManager::~CUIManager
 ====================*/
CUIManager::~CUIManager()
{
}


/*====================
  CUIManager::CUIManager
  ====================*/
CUIManager::CUIManager() :
m_bRefreshCursor(false),
m_hStringTable(INVALID_RESOURCE)
{
    m_itActiveInterface = m_mapInterfaces.end();
    m_itSavedActiveInterface = m_mapInterfaces.end();
}


/*====================
  CUIManager::Initialize
  ====================*/
void    CUIManager::Initialize()
{
    m_hStringTable = g_ResourceManager.Register(_T("/stringtables/interface.str"), RES_STRINGTABLE);
}


/*====================
  CUIManager::LoadInterface
  ====================*/
ResHandle   CUIManager::LoadInterface(const tstring &sFilename)
{
#ifdef K2_PROFILE
    static tsvector s_vNames;
    s_vNames.push_back(_T("CUIManager::LoadInterface") + ParenStr(sFilename));

    //PROFILE(s_vNames.back().c_str());
#endif

    // Register the interface in the resource manager
    ResHandle hInterface(g_ResourceManager.Register(sFilename, RES_INTERFACE));
    if (hInterface == INVALID_RESOURCE)
        return hInterface;

    CInterfaceResource *pInterface(g_ResourceManager.GetInterface(hInterface));
    if (pInterface->GetInterface() == nullptr)
    {
        Console.Warn << _T("Failed loading interface: ") << sFilename << newl;
        return hInterface;
    }

    // Add it to the map and resolve collisions
    tstring sName(pInterface->GetInterface()->GetName());
    InterfaceMap_it itFind(m_mapInterfaces.find(sName));
    if (itFind != m_mapInterfaces.end() && hInterface != itFind->second)
    {
        Console.Warn << _T("Interface ") << sName << _T(" [")
                    << GetInterface(itFind->second)->GetFilename() << _T("] replaced by [")
                    << sFilename << _T("]") << newl;

        g_ResourceManager.Unregister(itFind->second, UNREG_RESERVE_HANDLE);
    }

    m_mapInterfaces[sName] = hInterface;
    //m_itActiveInterface = m_mapInterfaces.find(sName);

    return hInterface;
}


/*====================
  CUIManager::UnloadInterface
  ====================*/
void    CUIManager::UnloadInterface(ResHandle hInterface)
{
    CInterface *pInterface(GetInterface(hInterface));
    if (pInterface == nullptr)
        return;

    UnloadInterface(pInterface->GetName());
}


/*====================
  CUIManager::UnloadInterface
  ====================*/
void    CUIManager::UnloadInterface(const tstring &sName)
{
    InterfaceMap_it itFind(m_mapInterfaces.find(sName));
    if (itFind == m_mapInterfaces.end())
    {
        Console.Warn << _T("Interface ") << sName << _T(" not found") << newl;
        return;
    }

    if (m_itActiveInterface == itFind)
        m_itActiveInterface = m_mapInterfaces.end();
    if (m_itSavedActiveInterface == itFind)
        m_itSavedActiveInterface = m_mapInterfaces.end();

    RemoveOverlayInterface(sName);

    g_ResourceManager.Unregister(itFind->second, UNREG_RESERVE_HANDLE);
    m_mapInterfaces.erase(itFind);
}


/*====================
  CUIManager::ReloadInterface
  ====================*/
void    CUIManager::ReloadInterface(const tstring &sName)
{
    InterfaceMap_cit itFind(m_mapInterfaces.find(sName));
    if (itFind == m_mapInterfaces.end())
    {
        Console.Warn << _T("Interface ") << sName << _T(" not found") << newl;
        return;
    }

    g_ResourceManager.Reload(itFind->second);
}


/*====================
  CUIManager::GetInterface
  ====================*/
CInterface* CUIManager::GetInterface(ResHandle handle) const
{
    CInterfaceResource *pInterface(g_ResourceManager.GetInterface(handle));
    if (pInterface == nullptr)
        return nullptr;

    return pInterface->GetInterface();
}

CInterface* CUIManager::GetInterface(const tstring &sName) const
{
    InterfaceMap_cit itFind(m_mapInterfaces.find(sName));
    if (itFind == m_mapInterfaces.end())
        return nullptr;

    return GetInterface(itFind->second);
}


/*====================
  CUIManager::GetActiveInterface
  ====================*/
CInterface* CUIManager::GetActiveInterface() const
{
    if (m_itActiveInterface == m_mapInterfaces.end())
        return nullptr;

    return GetInterface(m_itActiveInterface->second);
}


/*====================
  CUIManager::GetSavedActiveInterface
  ====================*/
CInterface* CUIManager::GetSavedActiveInterface() const
{
    if (m_itSavedActiveInterface == m_mapInterfaces.end())
        return nullptr;

    return GetInterface(m_itSavedActiveInterface->second);
}


/*====================
  CUIManager::IsOverlayInterface
  ====================*/
bool    CUIManager::IsOverlayInterface(const tstring &sName)
{
    for(OverlayList_it it(m_lOverlayInterfaces.begin()); it != m_lOverlayInterfaces.end(); it++)
        if (CompareNoCase((*it)->first, sName) == 0)
            return true;

    return false;
}


/*====================
  CUIManager::IsOverlayInterface
  ====================*/
bool    CUIManager::IsOverlayInterface(CInterface *pInterface)
{
    for(OverlayList_it it(m_lOverlayInterfaces.begin()); it != m_lOverlayInterfaces.end(); it++)
        if (GetInterface((*it)->second) == pInterface)
            return true;

    return false;
}


/*====================
  CUIManager::IsOverlayInterface
  ====================*/
bool    CUIManager::IsOverlayInterface(ResHandle hInterface)
{
    for(OverlayList_it it(m_lOverlayInterfaces.begin()); it != m_lOverlayInterfaces.end(); it++)
        if ((*it)->second == hInterface)
            return true;

    return false;
}


/*====================
  CUIManager::SetActiveInterface
  ====================*/
void    CUIManager::SetActiveInterface(const tstring &sName)
{
    if (m_itActiveInterface == m_mapInterfaces.end() && sName.empty())
        return;

    InterfaceMap_it itFind(m_mapInterfaces.find(sName));
    if (itFind == m_mapInterfaces.end())
    {
            m_itActiveInterface = itFind;
            Console.Warn << _T("CUIManager::SetActiveInterface() - Interface ") << sName << _T(" not found") << newl;
            return;
    }

    if (m_itActiveInterface != itFind)
    {       
        if (m_itActiveInterface != m_mapInterfaces.end() && GetInterface(m_itActiveInterface->second))
        {
            CInterface *pInterface(GetActiveInterface());
            if (pInterface != nullptr)
                pInterface->DoEvent(WEVENT_HIDE);
        }

        Input.SetCursor(CURSOR_UI, INVALID_RESOURCE);
        Input.SetCursorConstrained(CURSOR_UI, BOOL_NOT_SET);
        Input.SetCursorConstraint(CURSOR_UI, CRectf(0.0f, 0.0f, 0.0f, 0.0f));
        Input.SetCursorFrozen(CURSOR_UI, BOOL_NOT_SET);
        Input.SetCursorHidden(CURSOR_UI, BOOL_NOT_SET);
        Input.SetCursorRecenter(CURSOR_UI, BOOL_NOT_SET);

        m_itActiveInterface = itFind;
        CInterface *pInterface(GetActiveInterface());
        if (pInterface != nullptr)
        {
            if  (ui_debugInterface)
                Console.UI << _T("Show - ") << sName << newl;

            pInterface->DoEvent(WEVENT_SHOW);
        }
    }
}


/*====================
  CUIManager::AddOverlayInterface
  ====================*/
bool    CUIManager::AddOverlayInterface(const tstring &sName)
{
    InterfaceMap_it itFind(m_mapInterfaces.find(sName));
    if (!sName.empty() && itFind == m_mapInterfaces.end())
    {
        Console.Warn << _T("CUIManager::SetOverlayInterface() - Interface ") << sName << _T(" not found") << newl;
        return false;
    }

    if (itFind == m_itActiveInterface)
        return false;

    bool bFound(false);
    for (OverlayList_it it(m_lOverlayInterfaces.begin()); it != m_lOverlayInterfaces.end(); it++)
    {
        if ((*it) == itFind)
        {
            bFound = true;
            break;
        }
    }

    if (!bFound)
    {
        m_lOverlayInterfaces.push_back(itFind);
        
        CInterface *pInterface(GetInterface(itFind->second));

        if (pInterface != nullptr)
        {
            pInterface->SetAlwaysUpdate(true);
            pInterface->DoEvent(WEVENT_SHOW);
        }
    }

    return !bFound;
}


/*====================
  CUIManager::RemoveOverlayInterface
  ====================*/
void    CUIManager::RemoveOverlayInterface(const tstring &sName)
{
    InterfaceMap_it itFind(m_mapInterfaces.find(sName));
    if (itFind == m_mapInterfaces.end())
        return;

    for (OverlayList_it it(m_lOverlayInterfaces.begin()); it != m_lOverlayInterfaces.end(); it++)
    {
        if ((*it) == itFind)
        {
            CInterface *pInterface(GetInterface((*it)->second));

            if (pInterface != nullptr)
            {
                pInterface->DoEvent(WEVENT_HIDE);
                pInterface->SetAlwaysUpdate(false);
            }

            m_lOverlayInterfaces.erase(it);
            break;
        }
    }
}


/*====================
  CUIManager::ClearOverlayInterfaces
  ====================*/
void    CUIManager::ClearOverlayInterfaces()
{
    for (OverlayList_it it(m_lOverlayInterfaces.begin()); it != m_lOverlayInterfaces.end(); it++)
    {
        CInterface *pInterface(GetInterface((*it)->second));

        if (pInterface != nullptr)
        {
            pInterface->DoEvent(WEVENT_HIDE);
            pInterface->SetAlwaysUpdate(false);
        }
    }

    m_lOverlayInterfaces.clear();
}


/*====================
  CUIManager::UnloadTempInterfaces
  ====================*/
void    CUIManager::UnloadTempInterfaces()
{
    InterfaceMap_it it(m_mapInterfaces.begin());

    while (it != m_mapInterfaces.end())
    {
        CInterface *pInterface(GetInterface(it->second));
        if (pInterface == nullptr || !pInterface->GetTemp())
        {
            ++it;
            continue;
        }

        UnloadInterface(it->first);

        // Start search from the beginning of the map again
        it = m_mapInterfaces.begin();
    }
}


/*====================
  CUIManager::BringOverlayToFront
  ====================*/
void    CUIManager::BringOverlayToFront(const tstring &sName)
{
    InterfaceMap_it itFind(m_mapInterfaces.find(sName));
    if (!sName.empty() && itFind == m_mapInterfaces.end())
    {
        Console.Warn << _T("CUIManager::BringOverlayToFront() - Interface ") << sName << _T(" not found") << newl;
        return;
    }

    if (itFind == m_itActiveInterface)
        return;

    for (OverlayList_it it(m_lOverlayInterfaces.begin()); it != m_lOverlayInterfaces.end(); it++)
    {
        if ((*it) == itFind)
        {
            m_lOverlayInterfaces.erase(it);
            m_lOverlayInterfaces.push_back(itFind);
            break;
        }
    }
}


/*====================
  CUIManager::PrintInterfaceList
  ====================*/
void    CUIManager::PrintInterfaceList() const
{
    Console << _T("Loaded interfaces") << newl
            << _T("-----------------") << newl;
    for (InterfaceMap_cit it(m_mapInterfaces.begin()); it != m_mapInterfaces.end(); ++it)
    {
        CInterface *pInterface(GetInterface(it->second));
        Console << (it == m_itActiveInterface ? _T(" * ") : _T("   "))
                << pInterface->GetName()
                << _T("  [") << pInterface->GetFilename() << _T("]") << newl;
    }
}


/*====================
  CUIManager::FindWidget
  ====================*/
IWidget*    CUIManager::FindWidget(const tstring &sName)
{
    CInterface *pInterface(GetActiveInterface());
    IWidget *pWidget(nullptr);

    if (!pInterface || !(pWidget = pInterface->GetWidget(sName)))
    {
        for (InterfaceMap_cit it(m_mapInterfaces.begin()); it != m_mapInterfaces.end(); ++it)
        {
            pInterface = GetInterface(it->second);

            if (pInterface)
                pWidget = pInterface->GetWidget(sName);

            if (pWidget)
                break;
        }
    }

    return pWidget;
}


/*====================
  CUIManager::FindWidgetsByWildcards
  ====================*/
uint    CUIManager::FindWidgetsByWildcards(vector<IWidget*>& vWidgets, const tstring &sWildcards, bool bSearchActiveInterfaceOnly)
{
    if (bSearchActiveInterfaceOnly)
    {
        CInterface *pInterface(GetActiveInterface());
        if (!pInterface)
            return 0;

        return pInterface->FindWidgetsByWildcards(vWidgets, sWildcards);
    }


    uint uiMatches(0);
    for (InterfaceMap_cit it(m_mapInterfaces.begin()); it != m_mapInterfaces.end(); ++it)
    {
        CInterface* pInterface = GetInterface(it->second);

        if (pInterface)
            uiMatches += pInterface->FindWidgetsByWildcards(vWidgets, sWildcards);
    }

    return uiMatches;;
}


/*====================
  CUIManager::ProcessInput
  ====================*/
void    CUIManager::ProcessInput()
{
    PROFILE("CUIManager::ProcessInput");

    if (!m_lOverlayInterfaces.empty() && m_itSavedActiveInterface == m_mapInterfaces.end())
    {
        m_itSavedActiveInterface = m_itActiveInterface;

        bool bBlocked(false);

        for (OverlayList::reverse_iterator rit(m_lOverlayInterfaces.rbegin()); rit != m_lOverlayInterfaces.rend(); rit++)
        {
            m_itActiveInterface = *rit;

            CInterface *pInterface(GetActiveInterface());

            if (pInterface != nullptr && pInterface->GetActiveWidget() != nullptr)
                Input.ExecuteBinds(BINDTABLE_UI, 0);

            ProcessInput();

            if (pInterface->HasFlags(WFLAG_BLOCK_INPUT))
            {
                bBlocked = true;
                break;
            }
        }

        m_itActiveInterface = m_itSavedActiveInterface;
        m_itSavedActiveInterface = m_mapInterfaces.end();

        if (bBlocked)
            return;

        CInterface *pInterface(GetActiveInterface());

        if (pInterface != nullptr && pInterface->GetActiveWidget() != nullptr)
            Input.ExecuteBinds(BINDTABLE_UI, 0);

        Input.ExecuteBinds(BINDTABLE_UI, 0);
    }
    else if (m_lOverlayInterfaces.empty())
    {
        Input.ExecuteBinds(BINDTABLE_UI, 0);
    }

    if (GetActiveInterface() == nullptr)
        return;

    InputDeque deqUnused;

    // Step through the pending input and remove anything the UI wants
    while (!Input.IsEmpty() && m_itActiveInterface != m_mapInterfaces.end() && !IsReleased())
    {
        SIEvent ev(Input.Pop());
        bool bUsed(false);

        switch (ev.eType)
        {
        case INPUT_AXIS:
            bUsed = ProcessInputAxis(ev.uID.axis, ev.cDelta.fValue);
            break;

        case INPUT_CURSOR:
            bUsed = ProcessInputCursor(ev.cAbs.v2Cursor);
            break;

        case INPUT_BUTTON:
            // Don't remove up events because other things may looking for one to stop a drag or such (editor)
            if ((ev.uID.btn == BUTTON_MOUSEL ||
                ev.uID.btn == BUTTON_MOUSER ||
                ev.uID.btn == BUTTON_MOUSEM ||
                ev.uID.btn == BUTTON_MOUSEX1 ||
                ev.uID.btn == BUTTON_MOUSEX2 ||
                ev.uID.btn == BUTTON_WHEELUP ||
                ev.uID.btn == BUTTON_WHEELDOWN ||
                ev.uID.btn == BUTTON_WHEELLEFT ||
                ev.uID.btn == BUTTON_WHEELRIGHT) &&
                !Input.IsCursorFrozen() &&
                !Input.IsCursorRecenter())
            {
                bUsed = ProcessInputMouseButton(ev.cAbs.v2Cursor, ev.uID.btn, ev.cAbs.fValue);
            }
            else
            {
                bUsed = ProcessInputButton(ev.uID.btn, ev.cAbs.fValue > 0.0f, ev.cDelta.fValue == 1.0f) && ev.cAbs.fValue != 0.0f;
            }
            break;

        case INPUT_CHARACTER:
            bUsed = ProcessInputChar(ev.uID.chr);

            // UTTAR: See comment in CInput::AddEvent(TCHAR c)
            //if(!bUsed && !Console.IsActive() && (Input.IsCtrlDown()/* || Input.IsAltDown()*/))
            //  bUsed = true;

            break;

        default:
            break;
        }

        if (!bUsed)
            deqUnused.push_back(ev);
    }

    for (InputDeque::iterator it(deqUnused.begin()); it != deqUnused.end(); ++it)
        Input.Push(*it);

    if (m_bRefreshCursor)
    {
        ProcessInputCursor(Input.GetCursorPos());
        m_bRefreshCursor = false;
    }
}


/*====================
  CUIManager::ProcessInputAxis
  ====================*/
bool    CUIManager::ProcessInputAxis(EAxis axis, float fValue)
{
    if (GetActiveInterface()->ProcessInputAxis(axis, fValue))
        return true;

    return false;
}

/*====================
  CUIManager::LostInterface
  ====================*/
void    CUIManager::LostInterface(CInterface *pInterface)
{
    if (m_itActiveInterface != m_mapInterfaces.end() && GetInterface(m_itActiveInterface->second) == pInterface)
        m_itActiveInterface = m_mapInterfaces.end();

    RemoveOverlayInterface(pInterface->GetName());
}

/*====================
  CUIManager::ProcessInputCursor
  ====================*/
bool    CUIManager::ProcessInputCursor(const CVec2f &v2Pos)
{
    CInterface *pInterface(GetActiveInterface());
    if (pInterface != nullptr && pInterface->ProcessInputCursor(v2Pos))
        return true;

    return false;
}


/*====================
  CUIManager::ProcessInputMouseButton
  ====================*/
bool    CUIManager::ProcessInputMouseButton(const CVec2f &v2CursorPos, EButton button, float fValue)
{
    CInterface *pInterface(GetActiveInterface());
    if (pInterface != nullptr && pInterface->ProcessInputMouseButton(v2CursorPos, button, fValue))
        return true;
    return false;
}


/*====================
  CUIManager::ProcessInputButton
  ====================*/
bool    CUIManager::ProcessInputButton(EButton eButton, bool bDown, bool bPressed)
{
    if (bPressed && GetActiveInterface()->GetActiveWidget() == nullptr)
    {
        CInterface *pInterface(GetActiveInterface());
        if (pInterface != nullptr && pInterface->ProcessHotKeys(eButton))
            return true;
    }

    IWidget *pWidget(GetActiveInterface()->GetActiveWidget());
    if (pWidget == nullptr || !pWidget->IsAbsoluteEnabled() || !pWidget->IsAbsoluteVisible())
        return false;

    if (!bDown)
    {
        if (pWidget->ButtonUp(eButton))
            return true;
    }
    else
    {
        if (pWidget->ButtonDown(eButton))
            return true;

        if (eButton == BUTTON_TAB && !g_pInput->IsButtonDown(BUTTON_SHIFT))
        {
            IWidget *pNewWidget;
            pNewWidget = GetActiveInterface()->GetNextTabWidget(pWidget->GetTabOrder());

            if (pNewWidget != nullptr)
            {
                CInterface *pInterface(GetActiveInterface());
                if (pInterface != nullptr)
                    pInterface->SetActiveWidget(pNewWidget);
                return true;
            }

            return false;
        }
        else if (eButton == BUTTON_TAB)
        {
            IWidget *pNewWidget;
            pNewWidget = GetActiveInterface()->GetPrevTabWidget(pWidget->GetTabOrder());

            if (pNewWidget != nullptr)
            {
                CInterface *pInterface(GetActiveInterface());
                if (pInterface != nullptr)
                    pInterface->SetActiveWidget(pNewWidget);
                return true;
            }

            return false;
        }
    }

    return false;
}


/*====================
  CUIManager::ProcessInputChar
  ====================*/
bool    CUIManager::ProcessInputChar(TCHAR c)
{
    CInterface *pInterface(GetActiveInterface());
    if (pInterface == nullptr)
        return false;

    IWidget *pWidget(pInterface->GetActiveWidget());
    if (pWidget == nullptr || !pWidget->IsAbsoluteVisible())
        return false;

    if (c == 9) //tab
        pWidget->DoEvent(WEVENT_TAB);

    if (pWidget->Char(c))
        return true;
    

    return false;
}


/*====================
  CUIManager::GetMinimapHoverUnit
  ====================*/
uint    CUIManager::GetMinimapHoverUnit()
{
    CInterface *pInterface(GetActiveInterface());
    if (pInterface != nullptr)
        return pInterface->GetMinimapHoverUnit();

    return -1;
}


/*====================
  CUIManager::GetCopyString
  ====================*/
tstring CUIManager::GetCopyString()
{
    if (GetActiveInterface() == nullptr)
        return _T("");

    IWidget *pWidget(GetActiveInterface()->GetActiveWidget());

    if (pWidget == nullptr)
        return _T("");

    return pWidget->GetCopyString();
}


/*====================
  CUIManager::PasteString
  ====================*/
void    CUIManager::PasteString(const tstring &sString)
{
    if (GetActiveInterface() == nullptr)
        return;

    IWidget *pWidget(GetActiveInterface()->GetActiveWidget());

    if (pWidget == nullptr)
        return;

    pWidget->PasteString(sString);
}


/*====================
  CUIManager::Frame
  ====================*/
void    CUIManager::Frame(uint uiFrameLength)
{
    PROFILE("CUIManager:Frame");

    if (ui_reloadInterfaces)
    {
        ui_reloadInterfaces = false;
        Console.Execute(_T("ReloadInterfaces"));
    }

    if (GetActiveInterface() == nullptr)
    {
        if (!m_lOverlayInterfaces.empty())
        {
            PROFILE("Overlay Frame");

            m_itSavedActiveInterface = m_itActiveInterface;

            for (OverlayList_it it(m_lOverlayInterfaces.begin()); it != m_lOverlayInterfaces.end(); it++)
            {
                m_itActiveInterface = *it;
                Frame(uiFrameLength);
            }

            m_itActiveInterface = m_itSavedActiveInterface;
            m_itSavedActiveInterface = m_mapInterfaces.end();
        }

        return;
    }

    {
        PROFILE("Frame");
        GetActiveInterface()->Frame(uiFrameLength, true);
        if (GetActiveInterface()->NeedsPurge())
        {
            PROFILE("Purge");
            GetActiveInterface()->Purge();
        }
    }

    if (ui_draw)
    {
        PROFILE("Render");
        GetActiveInterface()->UnsetFlags(WFLAG_RENDER_TOP);
        GetActiveInterface()->Render(CVec2f(0.0f, 0.0f), WIDGET_RENDER_BOTTOM, 1.0f);
        //GetActiveInterface()->SetFlags(WFLAG_RENDER_TOP);
        //GetActiveInterface()->Render(CVec2f(0.0f, 0.0f), WIDGET_RENDER_TOP);
    }

    if (ui_drawGrid && GetActiveInterface()->IsGridSnapEnabled())
        DrawGrid();

    const tstring &sHighlightWidgets(ui_highlightWidgets);
    if (!sHighlightWidgets.empty())
    {
        vector<IWidget*> vWidgets;
        GetActiveInterface()->FindWidgetsByWildcards(vWidgets, sHighlightWidgets);
        for (size_t i = 0; i < vWidgets.size(); ++i)
        {
            IWidget* pWidget(vWidgets[i]);
            CRectf rect(pWidget->GetRect());
            rect.MoveTo(pWidget->GetAbsolutePos());

            CVec4f v4Color(LIME);
            Draw2D.SetColor(BLACK);
            Draw2D.String(rect.left + 1.0f, rect.top + 1.0f, pWidget->GetName(), g_ResourceManager.LookUpName(_T("system_medium"), RES_FONTMAP));
            Draw2D.SetColor(v4Color);
            Draw2D.String(rect.left, rect.top, pWidget->GetName(), g_ResourceManager.LookUpName(_T("system_medium"), RES_FONTMAP));
            Draw2D.RectOutline(rect, 1);
        }
    }

    if (ui_debugHoverWidget && GetActiveInterface()->GetHoverWidget())
    {
        deque<IWidget*> deqWidgets;

        IWidget *pWidget(GetActiveInterface()->GetHoverWidget());
        while (pWidget != nullptr)
        {
            deqWidgets.push_front(pWidget);
            pWidget = pWidget->GetParent();
        }

        for (deque<IWidget*>::iterator it(deqWidgets.begin()); it != deqWidgets.end(); ++it)
        {
            pWidget = *it;
            CRectf rect(pWidget->GetRect());
            rect.MoveTo(pWidget->GetAbsolutePos());

            CVec4f v4Color(GREEN);
            if (it - deqWidgets.begin() == deqWidgets.size() - 1)
                v4Color = LIME;
            Draw2D.SetColor(BLACK);
            Draw2D.String(rect.left + 1.0f, rect.top + 1.0f, pWidget->GetName(), g_ResourceManager.LookUpName(_T("system_small"), RES_FONTMAP));
            Draw2D.SetColor(v4Color);
            Draw2D.String(rect.left, rect.top, pWidget->GetName(), g_ResourceManager.LookUpName(_T("system_small"), RES_FONTMAP));
            Draw2D.RectOutline(rect, 1);
        }

        tstring s;
        CVec2f v2Cursor(Input.GetCursorPos());
        v2Cursor.x += 15.0f;
        Draw2D.SetColor(BLACK);
        Draw2D.String(v2Cursor.x + 1.0f, v2Cursor.y + 1.0f, pWidget->GetName(), g_ResourceManager.LookUpName(_T("system_small"), RES_FONTMAP));
        Draw2D.SetColor(LIME);
        Draw2D.String(v2Cursor.x, v2Cursor.y, pWidget->GetName(), g_ResourceManager.LookUpName(_T("system_small"), RES_FONTMAP));

        v2Cursor.y += 10.0f;
        s = _T("x: ") + XtoA(pWidget->GetRect().left) + _T("  y: ") + XtoA(pWidget->GetRect().right);
        Draw2D.SetColor(BLACK);
        Draw2D.String(v2Cursor.x + 1.0f, v2Cursor.y + 1.0f, s, g_ResourceManager.LookUpName(_T("system_small"), RES_FONTMAP));
        Draw2D.SetColor(LIME);
        Draw2D.String(v2Cursor.x, v2Cursor.y, s, g_ResourceManager.LookUpName(_T("system_small"), RES_FONTMAP));

        v2Cursor.y += 10.0f;
        s = _T("w: ") + XtoA(pWidget->GetRect().GetWidth()) + _T("  h: ") + XtoA(pWidget->GetRect().GetHeight());
        Draw2D.SetColor(BLACK);
        Draw2D.String(v2Cursor.x + 1.0f, v2Cursor.y + 1.0f, s, g_ResourceManager.LookUpName(_T("system_small"), RES_FONTMAP));
        Draw2D.SetColor(LIME);
        Draw2D.String(v2Cursor.x, v2Cursor.y, s, g_ResourceManager.LookUpName(_T("system_small"), RES_FONTMAP));
    }


    if (!m_lOverlayInterfaces.empty() && m_itSavedActiveInterface == m_mapInterfaces.end())
    {
        PROFILE("Overlay Frame");

        m_itSavedActiveInterface = m_itActiveInterface;
        
        OverlayList vOverlays(m_lOverlayInterfaces);
        for (OverlayList_it it(vOverlays.begin()); it != vOverlays.end(); ++it)
        {
            m_itActiveInterface = *it;
            Frame(uiFrameLength);
        }

        m_itActiveInterface = m_itSavedActiveInterface;
        m_itSavedActiveInterface = m_mapInterfaces.end();
    }

    if (GetActiveInterface()->GetHoverWidget() != nullptr && GetActiveInterface()->GetHoverWidget()->GetType() != WIDGET_INTERFACE)
        Input.SetCursorHidden(CURSOR_UI, BOOL_FALSE);
    else
        Input.SetCursorHidden(CURSOR_UI, BOOL_NOT_SET);

    if (GetActiveInterface()->GetHoverWidget() != nullptr && GetActiveInterface()->GetHoverWidget()->GetType() == WIDGET_TEXTBOX)
        Input.SetCursor(CURSOR_UI, g_ResourceManager.Register(_T("/core/cursors/text.cursor"), RES_K2CURSOR));
    else
        Input.SetCursor(CURSOR_UI, INVALID_RESOURCE);
}


/*====================
  CUIManager::Render
  ====================*/
void    CUIManager::Render(ResHandle hInterface, const CVec2f &v2Pos)
{
    CInterfaceResource *pInterfaceResource(g_ResourceManager.GetInterface(hInterface));

    if (!pInterfaceResource)
        return;

    CInterface *pInterface(pInterfaceResource->GetInterface());

    if (!pInterface)
        return;

    pInterface->Render(v2Pos, WIDGET_RENDER_BOTTOM, 1.0f);
    //pInterface->Render(v2Pos, WIDGET_RENDER_TOP, 1.0f);
}


/*====================
  CUIManager::DrawGrid
  ====================*/
void    CUIManager::DrawGrid()
{
    float fScreenW = Draw2D.GetScreenW();
    float fScreenH = Draw2D.GetScreenH();

    int iNumPixelsX = int(fScreenW / GetActiveInterface()->GetNumGridSquares());
    int iNumPixelsY = int(fScreenH / GetActiveInterface()->GetNumGridSquares());

    Draw2D.SetColor(RED);

    for (int i(1); i <= GetActiveInterface()->GetNumGridSquares(); ++i)
    {
        Draw2D.Rect(float(i * iNumPixelsX), 0.0f, 1.0f, fScreenH);
        Draw2D.Rect(0.0f, float(i * iNumPixelsY), fScreenW, 1.0f);
    }
}


/*====================
  CUIManager::ResizeAllInterfaces
  ====================*/
void    CUIManager::ResizeAllInterfaces(float fWidth, float fHeight)
{
    InterfaceMap_cit it;

    for (it = m_mapInterfaces.begin(); it != m_mapInterfaces.end(); it++)
    {
        CInterface *pInterface(GetInterface(it->second));

        if (pInterface != nullptr)
            pInterface->ResizeInterface(fWidth, fHeight);
    }

    CInterface *pInterface(GetSavedActiveInterface());
    if (pInterface != nullptr)
    {
        pInterface->Hide();
        pInterface->Show();
    }

    for (OverlayList_it it(m_lOverlayInterfaces.begin()); it != m_lOverlayInterfaces.end(); it++)
    {
        pInterface = GetInterface((*it)->second);

        if (pInterface != nullptr)
        {
            pInterface->Hide();
            pInterface->Show();
        }
    }
}


/*====================
  CUIManager::ResizeInterface
  ====================*/
void    CUIManager::ResizeInterface(const tstring &sInterface, float fWidth, float fHeight)
{
    InterfaceMap_cit it(m_mapInterfaces.find(sInterface));

    if (it != m_mapInterfaces.end())
    {
        CInterface *pInterface(GetInterface(it->second));

        if (pInterface != nullptr)
            pInterface->ResizeInterface(fWidth, fHeight);
    }
}


/*====================
  CUIManager::ProcessDeferedReloads
  ====================*/
void    CUIManager::ProcessDeferedReloads()
{
    for (sset_it it(m_setDeferedReloads.begin()); it != m_setDeferedReloads.end(); ++it)
        ReloadInterface(*it);

    m_setDeferedReloads.clear();
}


/*====================
  CUIManager::NeedsRefresh
  ====================*/
bool    CUIManager::NeedsRefresh()
{
    CInterface *pInterface(nullptr);

    if (m_itSavedActiveInterface != m_mapInterfaces.end())
        pInterface = GetInterface(m_itSavedActiveInterface->second);

    if (pInterface == nullptr && m_itActiveInterface != m_mapInterfaces.end())
        pInterface = GetInterface(m_itActiveInterface->second);

    if (pInterface != nullptr && pInterface->NeedsRefresh())
        return true;

    bool bRefresh(false);

    for (OverlayList_it it(m_lOverlayInterfaces.begin()); !bRefresh && it != m_lOverlayInterfaces.end(); it++)
    {
        pInterface = GetInterface((*it)->second);

        if (pInterface == nullptr)
            continue;

        bRefresh = pInterface->NeedsRefresh();
    }

    return bRefresh;
}


/*====================
  CUIManager::ResetRefresh
  ====================*/
void    CUIManager::ResetRefresh()
{
    CInterface *pInterface(nullptr);

    if (m_itSavedActiveInterface != m_mapInterfaces.end())
        pInterface = GetInterface(m_itSavedActiveInterface->second);

    if (pInterface == nullptr && m_itActiveInterface != m_mapInterfaces.end())
        pInterface = GetInterface(m_itActiveInterface->second);

    if (pInterface != nullptr)
        pInterface->NeedsRefresh(false);

    for (OverlayList_it it(m_lOverlayInterfaces.begin()); it != m_lOverlayInterfaces.end(); it++)
    {
        pInterface = GetInterface((*it)->second);

        if (pInterface == nullptr)
            continue;

        pInterface->NeedsRefresh(false);
    }
}


/*====================
  CUIManager::Translate
  ====================*/
tstring CUIManager::Translate(const tstring &sKey, const tsmapts &mapTokens)
{
    CStringTable *pStringTable(g_ResourceManager.GetStringTable(m_hStringTable));
    if (pStringTable == nullptr)
        return TSNULL;

    tstring sMessage(pStringTable->Get(sKey));

    if (mapTokens.empty())
        return sMessage;

    size_t zOffset(0);
    while (zOffset != tstring::npos)
    {
        size_t zStart(sMessage.find(_T('{'), zOffset));
        if (zStart == tstring::npos)
            break;
        size_t zEnd(sMessage.find(_T('}'), zStart));
        if (zEnd == tstring::npos)
            break;

        // Default parameter
        size_t zMid(sMessage.find(_T('='), zStart));
        if (zMid < zEnd)
        {
            const tstring &sToken(sMessage.substr(zStart + 1, zMid - zStart - 1));
            tsmapts_cit itFind(mapTokens.find(sToken));

            if (itFind != mapTokens.end())
            {
                const tstring &sValue(itFind->second);
                zOffset = zStart + sValue.length();
                sMessage.replace(zStart, zEnd - zStart + 1, sValue);
            }
            else
            {
                const tstring &sValue(sMessage.substr(zMid + 1, zEnd - zMid - 1));
                zOffset = zStart + sValue.length();
                sMessage.replace(zStart, zEnd - zStart + 1, sValue);
            }
            continue;
        }

        const tstring &sToken(sMessage.substr(zStart + 1, zEnd - zStart - 1));

        tsmapts_cit itFind(mapTokens.find(sToken));
        const tstring &sValue(itFind == mapTokens.end() ? TSNULL : itFind->second);
        zOffset = zStart + sValue.length();
        sMessage.replace(zStart, zEnd - zStart + 1, sValue);
    }

    return sMessage;
}


/*--------------------
  Translate
  --------------------*/
UI_CMD(Translate, 1)
{
    if (vArgList.size() == 1)
        return UIManager.Translate(vArgList[0]->Evaluate());

    tsmapts mapTokens;

    for (ScriptTokenVector_cit it(vArgList.begin() + 1); it != vArgList.end(); ++it)
    {
        ScriptTokenVector_cit itNext(it + 1);
        if (itNext == vArgList.end())
            break;

        mapTokens[(*it)->Evaluate()] = (*itNext)->Evaluate();
        it = itNext;
    }

    return UIManager.Translate(vArgList[0]->Evaluate(), mapTokens);
}


/*--------------------
  ListInterfaces
  --------------------*/
CMD(ListInterfaces)
{
    UIManager.PrintInterfaceList();
    return true;
}


/*--------------------
  LoadInterface
  --------------------*/
CMD(LoadInterface)
{
    if (vArgList.empty())
    {
        Console << _T("Syntax: LoadInterface <filename>") << newl;
        return false;
    }

    UIManager.LoadInterface(vArgList[0]);
    return true;
}


/*--------------------
  UnloadInterface
  --------------------*/
CMD(UnloadInterface)
{
    if (vArgList.empty())
    {
        Console << _T("syntax: UnloadInterface <interface name>") << newl;
        return false;
    }

    UIManager.UnloadInterface(vArgList[0]);
    return true;
}


/*--------------------
  ReloadInterface
  --------------------*/
CMD(ReloadInterface)
{
    if (vArgList.empty())
    {
        Console << _T("syntax: ReloadInterface <interface name>") << newl;
        return false;
    }

    UIManager.ReloadInterface(vArgList[0]);
    return true;
}


/*--------------------
  ShowInterface
  --------------------*/
CMD(ShowInterface)
{
    if (vArgList.empty())
        return false;

    UIManager.SetActiveInterface(vArgList[0]);
    return true;
}


/*--------------------
  EnableInterface
  --------------------*/
CMD(EnableInterface)
{
    UIManager.GetActiveInterface()->Enable();
    return true;
}


/*--------------------
  DisableInterface
  --------------------*/
CMD(DisableInterface)
{
    UIManager.GetActiveInterface()->Disable();
    return true;
}


/*--------------------
  ResizeAllInterfaces
  --------------------*/
CMD(ResizeAllInterfaces)
{
    if (vArgList.size() < 2)
        return false;

    UIManager.ResizeAllInterfaces(AtoF(vArgList[0]), AtoF(vArgList[1]));
    return true;
}


/*--------------------
  ResizeInterface
  --------------------*/
CMD(ResizeInterface)
{
    if (vArgList.size() < 3)
        return false;

    UIManager.ResizeInterface(vArgList[0], AtoF(vArgList[1]), AtoF(vArgList[2]));
    return true;
}


/*--------------------
  ShowWidget
  --------------------*/
CMD(ShowWidget)
{
    try
    {
        if (vArgList.empty())
            EX_MESSAGE(_T("syntax: ShowWidget <widget name>"));

        IWidget *pWidget(UIManager.FindWidget(vArgList[0]));
        if (pWidget == nullptr)
            EX_WARN(_T("Widget ") + vArgList[0] + _T(" not found"));

        pWidget->Show();
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("cmdShowWidget() - "), NO_THROW);
        return false;
    }
}


/*--------------------
  HideWidget
  --------------------*/
CMD(HideWidget)
{
    try
    {
        if (vArgList.empty())
            EX_MESSAGE(_T("syntax: HideWidget <widget name>"));

        IWidget *pWidget(UIManager.FindWidget(vArgList[0]));
        if (pWidget == nullptr)
            EX_WARN(_T("Widget ") + vArgList[0] + _T(" not found"));

        pWidget->Hide();
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("cmdHideWidget() - "), NO_THROW);
        return false;
    }
}


/*====================
  UI_ListWidgets
  ====================*/
static void     UI_ListWidgets(const tstring &sWildcards, bool bActiveInterfaceOnly)
{
    vector<IWidget*> vWidgets;
    uint uiMatches = UIManager.FindWidgetsByWildcards(vWidgets, sWildcards, bActiveInterfaceOnly);

    if (uiMatches == 0)
    {
        Console.Std << _T("No matching widgets for wildcard ") << sWildcards << newl;
        return;
    }

    Console.Std << uiMatches << _T(" matching widgets:") << newl;

    for (uint i = 0; i < (uint)vWidgets.size(); ++i)
    {
        IWidget* pWidget(vWidgets[i]);
        Console.Std << _T("     #^c") << XtoA(i, FMT_PADZERO, 4) << _T("^*:     ")
            << pWidget->GetName() << newl;
    }
}


/*--------------------
  ListWidgets
  --------------------*/
CMD(ListWidgets)
{
    try
    {
        tstring sWildcards(_T("*"));

        if (vArgList.size() >= 1)
            sWildcards = vArgList[0];

        UI_ListWidgets(sWildcards, false);
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("cmdListWidgets() - "), NO_THROW);
        return false;
    }
}


/*--------------------
  ListWidgetsInActiveInterface
  --------------------*/
CMD(ListWidgetsInActiveInterface)
{
    try
    {
        tstring sWildcards(_T("*"));

        if (vArgList.size() >= 1)
            sWildcards = vArgList[0];

        UI_ListWidgets(sWildcards, true);
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("cmdListWidgetsInActiveInterface() - "), NO_THROW);
        return false;
    }
}


/*--------------------
  ShowCCPanel
  --------------------*/
UI_VOID_CMD(ShowCCPanel, 0)
{
    if (ChatManager.IsConnected())
    {
        ShowCCPanel.Trigger(TSNULL);
        UIManager.BringOverlayToFront(_T("cc_panel"));
    }
}

CMD(ShowCCPanel)
{
    if (ChatManager.IsConnected())
    {
        ShowCCPanel.Trigger(TSNULL);
        UIManager.BringOverlayToFront(_T("cc_panel"));
    }

    return true;
}

/*--------------------
  HideCCPanel
  --------------------*/
UI_VOID_CMD(HideCCPanel, 0)
{
    HideCCPanel.Trigger(TSNULL);
}


/*--------------------
  ToggleCCPanel
  --------------------*/
UI_VOID_CMD(ToggleCCPanel, 0)
{
    ToggleCCPanel.Trigger(TSNULL);
    UIManager.BringOverlayToFront(_T("cc_panel"));
}


/*--------------------
  ShowCCStatistics
  --------------------*/
UI_VOID_CMD(ShowCCStatistics, 1)
{
    CCStatisticsVisible.Trigger(vArgList[0]->Evaluate());
}

CMD(ShowCCStatistics)
{
    if (vArgList.size() < 1)
        return false;

    CCStatisticsVisible.Trigger(vArgList[0]);
    return true;
}


/*--------------------
  AddOverlayInterface
  --------------------*/
UI_VOID_CMD(AddOverlayInterface, 1)
{
    UIManager.AddOverlayInterface(vArgList[0]->Evaluate());
}


/*--------------------
  BringOverlayToFront
  --------------------*/
UI_VOID_CMD(BringOverlayToFront, 1)
{
    UIManager.BringOverlayToFront(vArgList[0]->Evaluate());
}


/*--------------------
  SetFocusName
  --------------------*/
UI_VOID_CMD(SetFocusName, 1)
{
    CInterface *pInterface(pThis->GetInterface());

    if (pInterface == nullptr)
        return;

    IWidget *pWidget(pInterface->GetWidget(vArgList[0]->Evaluate()));

    if (pWidget == nullptr)
        return;

    pInterface->SetActiveWidget(pWidget);
}


/*--------------------
  SetActiveWidget
  --------------------*/
UI_VOID_CMD(SetActiveWidget, 0)
{
    CInterface *pInterface(pThis->GetInterface());

    if (!pInterface)
        return;

    if (vArgList.size() == 0)
    {
        pInterface->SetActiveWidget(pThis);
        return;
    }

    tstring sWidgetName(vArgList[0]->Evaluate());

    if (sWidgetName == _T("nullptr"))
        pInterface->SetActiveWidget(nullptr);
    else
        pInterface->SetActiveWidget(pInterface->GetWidget(sWidgetName));
}


/*--------------------
  ShowGroup
  --------------------*/
UI_VOID_CMD(ShowGroup, 1)
{
    pThis->GetInterface()->ShowGroup(vArgList[0]->Evaluate());
}


/*--------------------
  HideGroup
  --------------------*/
UI_VOID_CMD(HideGroup, 1)
{
    pThis->GetInterface()->HideGroup(vArgList[0]->Evaluate());
}


/*--------------------
  ShowOnly
  --------------------*/
UI_VOID_CMD(ShowOnly, 1)
{
    pThis->GetInterface()->ShowOnly(vArgList[0]->Evaluate());
}


/*--------------------
  HideOnly
  --------------------*/
UI_VOID_CMD(HideOnly, 1)
{
    pThis->GetInterface()->HideOnly(vArgList[0]->Evaluate());
}


/*--------------------
  EnableWidget
  --------------------*/
UI_VOID_CMD(EnableWidget, 0)
{
    if (!pThis)
        return;

    IWidget *pWidget(pThis);

    if (vArgList.size() > 0)
        pWidget = pWidget->GetInterface()->GetWidget(vArgList[0]->Evaluate());

    if (pWidget == nullptr)
        return;

    pWidget->Enable();
}


/*--------------------
  DisableWidget
  --------------------*/
UI_VOID_CMD(DisableWidget, 0)
{
    if (!pThis)
        return;

    IWidget *pWidget(pThis);

    if (vArgList.size() > 0)
        pWidget = pWidget->GetInterface()->GetWidget(vArgList[0]->Evaluate());

    if (pWidget == nullptr)
        return;

    pWidget->Disable();
}


/*--------------------
  EnableGroup
  --------------------*/
UI_VOID_CMD(EnableGroup, 1)
{
    pThis->GetInterface()->EnableGroup(vArgList[0]->Evaluate());
}


/*--------------------
  DisableGroup
  --------------------*/
UI_VOID_CMD(DisableGroup, 1)
{
    pThis->GetInterface()->DisableGroup(vArgList[0]->Evaluate());
}


/*--------------------
  EnableOnly
  --------------------*/
UI_VOID_CMD(EnableOnly, 1)
{
    pThis->GetInterface()->EnableOnly(vArgList[0]->Evaluate());
}


/*--------------------
  DisableOnly
  --------------------*/
UI_VOID_CMD(DisableOnly, 1)
{
    pThis->GetInterface()->DisableOnly(vArgList[0]->Evaluate());
}


/*--------------------
  FtoA
  --------------------*/
UI_CMD(FtoA, 1)
{
    uint uiWidth(vArgList.size() > 2 ? AtoI(vArgList[2]->Evaluate()) : 0);
    uint uiPrecision(vArgList.size() > 1 ? AtoI(vArgList[1]->Evaluate()) : 4);
    uint uiFlags(0);
    if (vArgList.size() > 3)
    {
        const tstring &sFlags(vArgList[3]->Evaluate());
        if (sFlags.find(_T('_')) != tstring::npos)
            uiFlags |= FMT_PADSIGN;
        if (sFlags.find(_T('+')) != tstring::npos)
            uiFlags |= FMT_SIGN;
        if (sFlags.find(_T('0')) != tstring::npos)
            uiFlags |= FMT_PADZERO;
        if (sFlags.find(_T(',')) != tstring::npos)
            uiFlags |= FMT_DELIMIT;
    }
    return XtoA(AtoF(vArgList[0]->Evaluate()), uiFlags, uiWidth, uiPrecision);
}


/*--------------------
  FtoA2
  --------------------*/
UI_CMD(FtoA2, 1)
{
    uint uiWidth(vArgList.size() > 3 ? AtoI(vArgList[3]->Evaluate()) : 0);
    int iMinPrecision(vArgList.size() > 1 ? AtoI(vArgList[1]->Evaluate()) : 4);
    int iMaxPrecision(vArgList.size() > 2 ? AtoI(vArgList[2]->Evaluate()) : 4);
    uint uiFlags(0);
    if (vArgList.size() > 4)
    {
        const tstring &sFlags(vArgList[3]->Evaluate());
        if (sFlags.find(_T('_')) != tstring::npos)
            uiFlags |= FMT_PADSIGN;
        if (sFlags.find(_T('+')) != tstring::npos)
            uiFlags |= FMT_SIGN;
        if (sFlags.find(_T('0')) != tstring::npos)
            uiFlags |= FMT_PADZERO;
        if (sFlags.find(_T(',')) != tstring::npos)
            uiFlags |= FMT_DELIMIT;
    }
    return XtoA(AtoF(vArgList[0]->Evaluate()), uiFlags, uiWidth, iMinPrecision, iMaxPrecision);
}


/*--------------------
  RefreshCursor
  --------------------*/
UI_VOID_CMD(RefreshCursor, 0)
{
    g_pUIManager->RefreshCursor();
}


/*--------------------
  WidgetExists
  --------------------*/
UI_CMD(WidgetExists, 1)
{
    CInterface *pInterface(pThis->GetInterface());

    if (pInterface == nullptr)
        return _T("0");

    return XtoA(pInterface->GetWidget(vArgList[0]->Evaluate()) != nullptr, true);
}


/*--------------------
  FtoA
  --------------------*/
FUNCTION(FtoA)
{
    if (vArgList.empty())
        return _T("");

    return XtoA(AtoF(vArgList[0]), 0, vArgList.size() > 2 ? AtoI(vArgList[2]) : 0, vArgList.size() > 1 ? AtoI(vArgList[1]) : 0);
}


/*--------------------
  Floor
  --------------------*/
FUNCTION(Floor)
{
    return XtoA(vArgList.size() > 0 ? INT_FLOOR(AtoF(vArgList[0])) : 0);
}


/*--------------------
  Ceil
  --------------------*/
FUNCTION(Ceil)
{
    return XtoA(vArgList.size() > 0 ? INT_CEIL(AtoF(vArgList[0])) : 0);
}


/*--------------------
  Choose
  --------------------*/
FUNCTION(Choose)
{
    if (vArgList.size() < 3)
        return _T("");

    int iValue(AtoI(vArgList[0]));
    int iBase(AtoI(vArgList[1]));

    if (iValue - iBase + 2 < int(vArgList.size()))
        return vArgList[iValue - iBase + 2];
    else
        return _T("");
}


/*--------------------
  GetMouseX
  --------------------*/
UI_CMD(GetMouseX, 0)
{
    return XtoA(Input.GetCursorPos().x);
}


/*--------------------
  GetMouseY
  --------------------*/
UI_CMD(GetMouseY, 0)
{
    return XtoA(Input.GetCursorPos().y);
}


/*--------------------
  GetScreenWidth
  --------------------*/
UI_CMD(GetScreenWidth, 0)
{
    return XtoA(Draw2D.GetScreenW());
}


/*--------------------
  GetScreenHeight
  --------------------*/
UI_CMD(GetScreenHeight, 0)
{
    return XtoA(Draw2D.GetScreenH());
}



/*--------------------
  UICopyInputLine
  --------------------*/
ACTION_IMPULSE(UICopyInputLine)
{
    K2System.CopyToClipboard(UIManager.GetCopyString());
}


/*--------------------
  UIPasteInputLine
  --------------------*/
ACTION_IMPULSE(UIPasteInputLine)
{
    if (K2System.IsClipboardString())
        UIManager.PasteString(K2System.GetClipboardString());
}


/*--------------------
  UIShowCCPanel
  --------------------*/
ACTION_IMPULSE(UIShowCCPanel)
{
    ToggleCCPanel.Trigger(TSNULL);
    UIManager.BringOverlayToFront(_T("cc_panel"));
}
