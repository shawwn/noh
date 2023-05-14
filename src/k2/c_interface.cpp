// (C)2005 S2 Games
// c_interface.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_interface.h"

#include "c_widgetstyle.h"
#include "c_widgettemplate.h"
#include "c_draw2d.h"
#include "c_uicmd.h"
#include "c_uimanager.h"
#include "c_uitrigger.h"
#include "c_uiform.h"
#include "c_button.h"
#include "c_resourcemanager.h"
#include "c_filechangecallback.h"
//=============================================================================

//=============================================================================
// CInterfacePackageCallback
//=============================================================================
class CInterfacePackageCallback : public IFileChangeCallback
{
private:
    tstring m_sParentName;

public:
    ~CInterfacePackageCallback() {}
    CInterfacePackageCallback(const tstring &sPath, const tstring &sParentName) :
    IFileChangeCallback(sPath),
    m_sParentName(sParentName)
    {
    }

    void    Execute();
};
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
CVAR_INTR(scene_dimensionMultiple, 32, 0, 0, 32);
//=============================================================================

/*====================
  CInterfacePackageCallback::Execute
  ====================*/
void    CInterfacePackageCallback::Execute()
{
    UIManager.DeferReload(m_sParentName);
}


/*====================
  CInterface::~CInterface
  ====================*/
CInterface::~CInterface()
{
    for (vector<CUITrigger*>::iterator it(m_vLocalTriggers.begin()); it != m_vLocalTriggers.end(); ++it)
        SAFE_DELETE(*it)

    for (PackageCallbackVector::iterator it(m_vPackageCallbacks.begin()); it != m_vPackageCallbacks.end(); ++it)
    {
        g_ResourceManager.UnregisterFileChangeCallback(*it);
        SAFE_DELETE(*it);
    }

    for (WidgetStyleMap_it it(m_mapStyles.begin()); it != m_mapStyles.end(); ++it)
        SAFE_DELETE(it->second);

    for (WidgetTemplateMap_it it(m_mapTemplates.begin()); it != m_mapTemplates.end(); ++it)
        SAFE_DELETE(it->second);

    if (!UIManager.IsReleased())
        UIManager.LostInterface(this);

    // Destroy these things while CInterface is still valid
    m_pInterface = NULL;
    m_pParent = NULL;

    WidgetPointerVector vChildren(m_vChildren);
    for (WidgetPointerVector_it itChild(vChildren.begin()), itEnd(vChildren.end()); itChild != itEnd; ++itChild)
        SAFE_DELETE(*itChild);
}


/*====================
  CInterface::CInterface
  ====================*/
CInterface::CInterface(const CWidgetStyle& style) :
IWidget(NULL, NULL, WIDGET_INTERFACE, style),
m_sFilename(_T("")),
m_pCurrentTemplate(NULL),
m_pActiveWidget(NULL),
m_pHoverWidget(NULL),
m_pExclusiveWidget(NULL),
m_pDefaultActiveWidget(NULL),
m_bAlwaysUpdate(style.GetPropertyBool(_T("alwaysupdate"), false)),
m_bTemp(style.GetPropertyBool(_T("temp"), false)),
m_bSnapToParent(false),
m_iParentSnapAt(0),
m_bSnapToGrid(style.GetPropertyBool(_T("snaptogrid"))),
m_iGridSquares(style.GetPropertyInt(_T("gridsize"), 10)),
m_iGridSnapAt(style.GetPropertyInt(_T("snapdistance"), m_iGridSquares / 2)),
m_bAnchored(false),
m_v2Anchor(V2_ZERO),
m_sSceneX(style.GetProperty(_T("scenex"), _T("0"))),
m_sSceneY(style.GetProperty(_T("sceney"), _T("0"))),
m_sSceneWidth(style.GetProperty(_T("scenewidth"), _T("100%"))),
m_sSceneHeight(style.GetProperty(_T("sceneheight"), _T("100%")))
{
    m_pInterface = this;

    SetFlags(WFLAG_VISIBLE);
    SetFlagsRecursive(WFLAG_PROCESS_CURSOR);

    SetWidth(Draw2D.GetScreenW());
    SetHeight(Draw2D.GetScreenH());
    SetFlags(WFLAG_NO_DRAW);

    // Scene
    m_recSceneArea.SetSizeX(ceil(GetSizeFromString(m_sSceneWidth, Draw2D.GetScreenW(), Draw2D.GetScreenH())));
    m_recSceneArea.SetSizeY(ceil(GetSizeFromString(m_sSceneHeight, Draw2D.GetScreenH(), Draw2D.GetScreenW())));
    m_recSceneArea.MoveToX(floor(GetPositionFromString(m_sSceneX, Draw2D.GetScreenW(), Draw2D.GetScreenH())));
    m_recSceneArea.MoveToY(floor(GetPositionFromString(m_sSceneY, Draw2D.GetScreenH(), Draw2D.GetScreenW())));

    m_recSceneArea.SetSizeX(MIN<float>(CEIL_MULTIPLE(M_CeilPow2(int(scene_dimensionMultiple)), m_recSceneArea.GetWidth()), Draw2D.GetScreenW()));
    m_recSceneArea.SetSizeY(MIN<float>(CEIL_MULTIPLE(M_CeilPow2(int(scene_dimensionMultiple)), m_recSceneArea.GetHeight()), Draw2D.GetScreenH()));
}


/*====================
  CInterface::RegisterStyle
  ====================*/
void    CInterface::RegisterStyle(const tstring &sName, CWidgetStyle *pStyle)
{
    try
    {
        WidgetStyleMap_it itFind(m_mapStyles.find(sName));
        if (itFind != m_mapStyles.end())
            EX_ERROR(_T("Style ") + QuoteStr(sName) + _T(" is already registered"));

        m_mapStyles[sName] = pStyle;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CInterface::RegisterStyle() - "), NO_THROW);
    }
}


/*====================
  CInterface::GetStyle
  ====================*/
CWidgetStyle*   CInterface::GetStyle(const tstring &sName)
{
    WidgetStyleMap_it itFind(m_mapStyles.find(sName));
    if (itFind == m_mapStyles.end())
        return NULL;

    return itFind->second;
}


/*====================
  CInterface::RegisterTemplate
  ====================*/
void    CInterface::RegisterTemplate(CWidgetTemplate *pTemplate)
{
    try
    {
        if (pTemplate == NULL)
            EX_ERROR(_T("NULL template"));

        WidgetTemplateMap_it itFind(m_mapTemplates.find(pTemplate->GetName()));
        if (itFind != m_mapTemplates.end())
            EX_ERROR(_T("Template ") + QuoteStr(pTemplate->GetName()) + _T(" is already registered"));

        m_mapTemplates[pTemplate->GetName()] = pTemplate;
        m_pCurrentTemplate = pTemplate;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CInterface::RegisterTemplate() - "), NO_THROW);
    }
}


/*====================
  CInterface::GetTemplate
  ====================*/
CWidgetTemplate*    CInterface::GetTemplate(const tstring &sName)
{
    WidgetTemplateMap_it itFind(m_mapTemplates.find(sName));
    if (itFind == m_mapTemplates.end())
        return NULL;

    return itFind->second;
}


/*====================
  CInterface::AddWidget
  ====================*/
void    CInterface::AddWidget(IWidget *pWidget)
{
    if (m_vFreeWidgetIDs.empty())
    {
        pWidget->SetID(INT_SIZE(m_vWidgets.size()));
        m_vWidgets.push_back(pWidget);
    }
    else
    {
        pWidget->SetID(m_vFreeWidgetIDs.back());
        m_vFreeWidgetIDs.pop_back();
        assert(m_vWidgets[pWidget->GetID()] == NULL);
        m_vWidgets[pWidget->GetID()] = pWidget;
    }

    const tstring &sName(pWidget->GetName());
    if (!sName.empty())
    {
        if (m_mapWidgets.find(sName) != m_mapWidgets.end())
        {
            Console.Warn << _T("Widget ") << QuoteStr(sName) << _T(" already exists in a loaded interface") << newl;
            pWidget->SetName(sName + _T("_DUPLICATE_") + XtoA(rand() % 10000, FMT_PADZERO, 4));
        }

        m_mapWidgets[sName] = pWidget->GetID();
    }
}


/*====================
  CInterface::AddWidgetToGroup
  ====================*/
void    CInterface::AddWidgetToGroup(IWidget *pWidget)
{
    const tstring &sGroupName(pWidget->GetGroupName());
    if (sGroupName.empty())
        return;

    WidgetGroup* pGroup(NULL);
    WidgetGroupMap_it itGroup(m_mapGroups.find(sGroupName));
    if (itGroup == m_mapGroups.end())
    {
        pGroup = K2_NEW(ctx_Widgets,  WidgetGroup);
        if (pGroup == NULL)
        {
            Console.Err << _T("Failed to allocate new widget group") << newl;
            return;
        }

        m_mapGroups.insert(WidgetGroupMapEntry(sGroupName, pGroup));
    }
    else
    {
        pGroup = itGroup->second;
    }

    if (pGroup == NULL)
        return;

    pGroup->insert(pWidget);
}


/*====================
  CInterface::RemoveWidgetFromGroup
  ====================*/
void    CInterface::RemoveWidgetFromGroup(IWidget *pWidget)
{
    const tstring &sGroupName(pWidget->GetGroupName());
    if (sGroupName.empty())
        return;

    WidgetGroupMap_it itGroup(m_mapGroups.find(sGroupName));
    if (itGroup == m_mapGroups.end())
        return;

    WidgetGroup* pGroup(itGroup->second);
    if (pGroup == NULL)
        return;

    pGroup->erase(pWidget);
}


/*====================
  CInterface::RemoveWidget
  ====================*/
void    CInterface::RemoveWidget(IWidget *pWidget)
{
    if (pWidget == NULL)
        return;

    if (m_pActiveWidget == pWidget)
        m_pActiveWidget = NULL;
    if (m_pHoverWidget == pWidget)
        m_pHoverWidget = NULL;
    if (m_pExclusiveWidget == pWidget)
        m_pExclusiveWidget = NULL;
    if (m_pDefaultActiveWidget == pWidget)
        m_pDefaultActiveWidget = NULL;

    uint uiID(pWidget->GetID());
    if (uiID >= m_vWidgets.size())
        return;

    m_vWidgets[uiID] = NULL;
    m_vFreeWidgetIDs.push_back(uiID);

    const tstring &sName(pWidget->GetName());
    if (!sName.empty())
        m_mapWidgets.erase(sName);

    if (!pWidget->GetGroupName().empty())
        RemoveWidgetFromGroup(pWidget);
}


/*====================
  CInterface::ShowGroup
  ====================*/
void    CInterface::ShowGroup(const tstring &sGroupName)
{
    WidgetGroup *pGroup(GetGroup(sGroupName));
    if (pGroup == NULL)
    {
        Console.Warn << _T("Widget group ") << QuoteStr(sGroupName) << _T(" not found") << newl;
        return;
    }

    for (WidgetGroup_it itWidget(pGroup->begin()); itWidget != pGroup->end(); ++itWidget)
        (*itWidget)->Show();
}


/*====================
  CInterface::HideGroup
  ====================*/
void    CInterface::HideGroup(const tstring &sGroupName)
{
    WidgetGroup *pGroup(GetGroup(sGroupName));
    if (pGroup == NULL)
    {
        Console.Warn << _T("Widget group ") << QuoteStr(sGroupName) << _T(" not found") << newl;
        return;
    }

    for (WidgetGroup_it itWidget(pGroup->begin()); itWidget != pGroup->end(); ++itWidget)
        (*itWidget)->Hide();
}


/*====================
  CInterface::ShowOnly
  ====================*/
void    CInterface::ShowOnly(const tstring &sWidgetName)
{
    IWidget* pWidget(GetWidget(sWidgetName));
    if (pWidget == NULL)
    {
        Console.Warn << _T("Widget ") << QuoteStr(sWidgetName) << _T(" not found") << newl;
        return;
    }

    tstring sGroupName(pWidget->GetGroupName());
    WidgetGroup *pGroup(GetGroup(sGroupName));
    if (pGroup == NULL)
    {
        Console.Warn << _T("Widget group ") << QuoteStr(sGroupName) << _T(" not found") << newl;
        return;
    }

    for (WidgetGroup_it itWidget(pGroup->begin()); itWidget != pGroup->end(); ++itWidget)
    {
        if (*itWidget == pWidget)
            continue;

        (*itWidget)->Hide();
    }

    pWidget->Show();
}


/*====================
  CInterface::HideOnly
  ====================*/
void    CInterface::HideOnly(const tstring &sWidgetName)
{
    IWidget* pWidget(GetWidget(sWidgetName));
    if (pWidget == NULL)
    {
        Console.Warn << _T("Widget ") << QuoteStr(sWidgetName) << _T(" not found") << newl;
        return;
    }

    tstring sGroupName(pWidget->GetGroupName());
    WidgetGroup *pGroup(GetGroup(sGroupName));
    if (pGroup == NULL)
    {
        Console.Warn << _T("Widget group ") << QuoteStr(sGroupName) << _T(" not found") << newl;
        return;
    }

    for (WidgetGroup_it itWidget(pGroup->begin()); itWidget != pGroup->end(); ++itWidget)
    {
        if (*itWidget == pWidget)
            continue;

        (*itWidget)->Show();
    }

    pWidget->Hide();
}


/*====================
  CInterface::EnableGroup
  ====================*/
void    CInterface::EnableGroup(const tstring &sGroupName)
{
    WidgetGroup *pGroup(GetGroup(sGroupName));
    if (pGroup == NULL)
    {
        Console.Warn << _T("Widget group ") << QuoteStr(sGroupName) << _T(" not found") << newl;
        return;
    }

    for (WidgetGroup_it itWidget(pGroup->begin()); itWidget != pGroup->end(); ++itWidget)
        (*itWidget)->Enable();
}


/*====================
  CInterface::DisableGroup
  ====================*/
void    CInterface::DisableGroup(const tstring &sGroupName)
{
    WidgetGroup *pGroup(GetGroup(sGroupName));
    if (pGroup == NULL)
    {
        Console.Warn << _T("Widget group ") << QuoteStr(sGroupName) << _T(" not found") << newl;
        return;
    }

    for (WidgetGroup_it itWidget(pGroup->begin()); itWidget != pGroup->end(); ++itWidget)
        (*itWidget)->Disable();
}


/*====================
  CInterface::EnableOnly
  ====================*/
void    CInterface::EnableOnly(const tstring &sWidgetName)
{
    IWidget* pWidget(GetWidget(sWidgetName));
    if (pWidget == NULL)
    {
        Console.Warn << _T("Widget ") << QuoteStr(sWidgetName) << _T(" not found") << newl;
        return;
    }

    tstring sGroupName(pWidget->GetGroupName());
    WidgetGroup *pGroup(GetGroup(sGroupName));
    if (pGroup == NULL)
    {
        Console.Warn << _T("Widget group ") << QuoteStr(sGroupName) << _T(" not found") << newl;
        return;
    }

    for (WidgetGroup_it itWidget(pGroup->begin()); itWidget != pGroup->end(); ++itWidget)
    {
        if (*itWidget == pWidget)
            continue;

        (*itWidget)->Disable();
    }

    pWidget->Enable();
}


/*====================
  CInterface::DisableOnly
  ====================*/
void    CInterface::DisableOnly(const tstring &sWidgetName)
{
    IWidget* pWidget(GetWidget(sWidgetName));
    if (pWidget == NULL)
    {
        Console.Warn << _T("Widget ") << QuoteStr(sWidgetName) << _T(" not found") << newl;
        return;
    }

    tstring sGroupName(pWidget->GetGroupName());
    WidgetGroup *pGroup(GetGroup(sGroupName));
    if (pGroup == NULL)
    {
        Console.Warn << _T("Widget group ") << QuoteStr(sGroupName) << _T(" not found") << newl;
        return;
    }

    for (WidgetGroup_it itWidget(pGroup->begin()); itWidget != pGroup->end(); ++itWidget)
    {
        if (*itWidget == pWidget)
            continue;

        (*itWidget)->Enable();
    }

    pWidget->Disable();
}


/*====================
  CInterface::GetWidget
  ====================*/
IWidget*    CInterface::GetWidget(const tstring &sWidgetName) const
{
    if (sWidgetName.empty())
        return NULL;

    WidgetMap_cit itFind(m_mapWidgets.find(sWidgetName));
    if (itFind == m_mapWidgets.end())
        return NULL;

    return m_vWidgets[itFind->second];
}


/*====================
  CInterface::FindWidgetsByWildcards
  ====================*/
uint    CInterface::FindWidgetsByWildcards(vector<IWidget*>& vWidgets, const tstring &sWildcards) const
{
    if (sWildcards.empty())
        return 0;

    uint uiMatches(0);

    for (WidgetMap_cit it(m_mapWidgets.begin()), itEnd(m_mapWidgets.end());
        it != itEnd;
        ++it)
    {
        const tstring &sWidgetName(it->first);
        IWidget* pWidget(m_vWidgets[it->second]);

        if (!pWidget)
            continue;

        // match the widget against the wildcards.
        if (EqualsWildcards(sWildcards, sWidgetName))
        {
            ++uiMatches;
            vWidgets.push_back(pWidget);
        }
    }

    return uiMatches;
}


/*====================
  CInterface::GetGroup
  ====================*/
WidgetGroup*    CInterface::GetGroup(const tstring &sGroupName) const
{
    WidgetGroupMap_cit itGroup(m_mapGroups.find(sGroupName));
    if (itGroup == m_mapGroups.end())
        return NULL;

    return itGroup->second;
}


/*====================
  CInterface::ProcessInputMouseButton
  ====================*/
bool    CInterface::ProcessInputMouseButton(const CVec2f &v2CursorPos, EButton button, float fValue)
{
    if (m_pExclusiveWidget == NULL || fValue != 1.0f)
        return IWidget::ProcessInputMouseButton(v2CursorPos, button, fValue);

    CVec2f v2RelativeCursorPos(v2CursorPos);

    if (m_pExclusiveWidget->GetParent() != NULL)
        v2RelativeCursorPos -= m_pExclusiveWidget->GetParent()->GetAbsolutePos();

    if (!m_pExclusiveWidget->ProcessInputMouseButton(v2RelativeCursorPos - m_recArea.lt(), button, fValue))
        return IWidget::ProcessInputMouseButton(v2CursorPos, button, fValue);
    else
        return true;
}


/*====================
  CInterface::ProcessInputCursor
  ====================*/
bool    CInterface::ProcessInputCursor(const CVec2f &v2CursorPos)
{
    m_v2CursorPos = v2CursorPos;

    if (!HasFlags(WFLAG_VISIBLE) || !HasFlags(WFLAG_ENABLED))
        return false;

    // Check for rollover/rolloff and send the call the appropriate functions
    IWidget *pWidget(IWidget::GetWidget(v2CursorPos - m_recArea.lt(), true));

    if (m_pExclusiveWidget) // for blocking rollover messages to other widgets (buttons while pressed, sliders)
    {
        if (pWidget == m_pExclusiveWidget)
            SetHoverWidget(pWidget);
        else
            SetHoverWidget(NULL);
    }
    else
        SetHoverWidget(pWidget);

    // Let children handle the input directly
    for (WidgetPointerVector_rit it(m_vChildren.rbegin()), itEnd(m_vChildren.rend()); it != itEnd; ++it)
    {
        if ((*it)->HasFlags(WFLAG_PROCESS_CURSOR) && (*it)->ProcessInputCursor(v2CursorPos - m_recArea.lt()))
            return true;
    }

    return false;
}


/*====================
  CInterface::SetHoverWidget
  ====================*/
void    CInterface::SetHoverWidget(IWidget *pWidget)
{
    if (pWidget != m_pHoverWidget)
    {
        if (m_pHoverWidget && m_pHoverWidget->IsEnabled())
            m_pHoverWidget->Rolloff();

        m_pHoverWidget = pWidget;

        if (m_pHoverWidget && m_pHoverWidget->IsEnabled())
            m_pHoverWidget->Rollover();
    }
}


/*====================
  CInterface::SetActiveWidget
  ====================*/
void    CInterface::SetActiveWidget(IWidget *pWidget)
{
    IWidget *pActive(m_pActiveWidget);

    m_pActiveWidget = pWidget;

    if (pActive != pWidget)
    {
        if (pActive != NULL)
        {
            pActive->LoseFocus();

            if (HasFlags(WFLAG_RELEASED))
                return;
        }

        if (pWidget != NULL)
        {
            pWidget->Focus();

            if (HasFlags(WFLAG_RELEASED))
                return;
        }
    }
}


/*====================
  CInterface::ResizeInterface
  ====================*/
void    CInterface::ResizeInterface(float fWidth, float fHeight)
{
    SetWidth(fWidth);
    SetHeight(fHeight);

    // Scene
    m_recSceneArea.SetSizeX(ceil(GetSizeFromString(m_sSceneWidth, fWidth, fHeight)));
    m_recSceneArea.SetSizeY(ceil(GetSizeFromString(m_sSceneHeight, fHeight, fWidth)));
    m_recSceneArea.MoveToX(floor(GetPositionFromString(m_sSceneX, fWidth, fHeight)));
    m_recSceneArea.MoveToY(floor(GetPositionFromString(m_sSceneY, fHeight, fWidth)));

    m_recSceneArea.SetSizeX(MIN<float>(CEIL_MULTIPLE(M_CeilPow2(int(scene_dimensionMultiple)), m_recSceneArea.GetWidth()), Draw2D.GetScreenW()));
    m_recSceneArea.SetSizeY(MIN<float>(CEIL_MULTIPLE(M_CeilPow2(int(scene_dimensionMultiple)), m_recSceneArea.GetHeight()), Draw2D.GetScreenH()));

    RecalculateSize();
}


/*====================
  CInterface::CheckSnapTargets
  ====================*/
bool    CInterface::CheckSnapTargets(CVec2f &v2Pos, IWidget *pWidget)
{
    bool bDone(false);

    for (WidgetPointerVector_it it(m_vChildren.begin()), itEnd(m_vChildren.end()); it != itEnd && !bDone; ++it)
    {
        if (*it == pWidget)
            continue;

        bDone = (*it)->CheckSnapTargets(v2Pos, pWidget);
    }

    return bDone;
}


/*====================
  CInterface::CheckSnapTo
  ====================*/
bool    CInterface::CheckSnapTo(CVec2f &v2Pos, IWidget *pWidget)
{
    bool bDone(false);

    for (WidgetPointerVector_it it(m_vChildren.begin()), itEnd(m_vChildren.end()); it != itEnd && !bDone; ++it)
    {
        if (*it == pWidget)
            continue;

        bDone = (*it)->CheckSnapTo(v2Pos, pWidget);
    }

    return bDone;
}


/*====================
  CInterface::WidgetLost
  ====================*/
void    CInterface::WidgetLost(IWidget *pWidget)
{
    RemoveWidget(pWidget);

    IWidget::WidgetLost(pWidget);
}


/*====================
  CInterface::AddCallback
  ====================*/
void    CInterface::AddCallback(const tstring &sFile)
{
    CInterfacePackageCallback* pCallback(K2_NEW(ctx_Resources,  CInterfacePackageCallback)(FileManager.SanitizePath(sFile), GetName()));
    m_vPackageCallbacks.push_back(pCallback);
    g_ResourceManager.RegisterFileChangeCallback(pCallback);
}


/*====================
  CInterface::GetNextTabWidget
  ====================*/
IWidget*    CInterface::GetNextTabWidget(uint uiOrder)
{
    IWidget *pNextWidget(NULL);
    IWidget *pFirstWidget(NULL);
    uint uiHighOrder(-1);
    uint uiLowOrder(uiOrder);

    for (WidgetTabMap_it it(m_mapTabOrder.begin()); it != m_mapTabOrder.end(); it++)
    {
        if (it->first == uint(-1) || it->first == uiOrder || !it->second->IsAbsoluteEnabled() || !it->second->IsAbsoluteVisible())
            continue;

        if (it->first < uiLowOrder)
        {
            uiLowOrder = it->first;
            pFirstWidget = it->second;
        }
        else if (it->first > uiOrder && it->first < uiHighOrder)
        {
            uiHighOrder = it->first;
            pNextWidget = it->second;
        }
    }

    return (pNextWidget != NULL ? pNextWidget : pFirstWidget);
}


/*====================
  CInterface::GetPrevTabWidget
  ====================*/
IWidget*    CInterface::GetPrevTabWidget(uint uiOrder)
{
    IWidget *pPrevWidget(NULL);
    IWidget *pLastWidget(NULL);
    uint uiHighOrder(uiOrder);
    uint uiLowOrder(0);

    for (WidgetTabMap_it it(m_mapTabOrder.begin()); it != m_mapTabOrder.end(); it++)
    {
        if (it->first == uint(-1) || !it->second->IsAbsoluteEnabled() || !it->second->IsAbsoluteVisible())
            continue;

        if (it->first < uiOrder && it->first >= uiLowOrder)
        {
            uiLowOrder = it->first;
            pPrevWidget = it->second;
        }
        else if (it->first > uiHighOrder)
        {
            uiHighOrder = it->first;
            pLastWidget = it->second;
        }
    }

    return (pPrevWidget != NULL ? pPrevWidget : pLastWidget);
}


/*====================
  CInterface::Frame
  ====================*/
void    CInterface::Frame(uint uiFrameLength, bool bProcessFrame)
{
    // Scene
    m_recSceneArea.SetSizeX(ceil(GetSizeFromString(m_sSceneWidth, Draw2D.GetScreenW(), Draw2D.GetScreenH())));
    m_recSceneArea.SetSizeY(ceil(GetSizeFromString(m_sSceneHeight, Draw2D.GetScreenH(), Draw2D.GetScreenW())));
    m_recSceneArea.MoveToX(floor(GetPositionFromString(m_sSceneX, Draw2D.GetScreenW(), Draw2D.GetScreenH())));
    m_recSceneArea.MoveToY(floor(GetPositionFromString(m_sSceneY, Draw2D.GetScreenH(), Draw2D.GetScreenW())));

    m_recSceneArea.SetSizeX(MIN<float>(CEIL_MULTIPLE(M_CeilPow2(int(scene_dimensionMultiple)), m_recSceneArea.GetWidth()), Draw2D.GetScreenW()));
    m_recSceneArea.SetSizeY(MIN<float>(CEIL_MULTIPLE(M_CeilPow2(int(scene_dimensionMultiple)), m_recSceneArea.GetHeight()), Draw2D.GetScreenH()));

    for (map<tstring, CUIForm*>::iterator itForm(m_mapForms.begin()); itForm != m_mapForms.end(); ++itForm)
        itForm->second->Frame();

    IWidget::Frame(uiFrameLength, bProcessFrame);

    if (m_pActiveWidget == NULL)
        m_pActiveWidget = m_pDefaultActiveWidget;
}


/*====================
  CInterface::AddForm
  ====================*/
void    CInterface::AddForm(CUIForm *pForm)
{
    if (m_mapForms.find(pForm->GetName()) != m_mapForms.end())
    {
        Console.Warn << _T("Form ") << QuoteStr(pForm->GetName()) << _T(" already exists in interface ") << m_sName << newl;
        return;
    }

    m_mapForms[pForm->GetName()] = pForm;
}


/*====================
  CInterface::GetForm
  ====================*/
CUIForm*    CInterface::GetForm(const tstring &sName)
{
    map<tstring, CUIForm*>::iterator itForm(m_mapForms.find(sName));
    if (itForm == m_mapForms.end())
    {
        Console.Warn << _T("Form ") << QuoteStr(sName) << _T(" not found in interface ") << m_sName << newl;
        return NULL;
    }

    return itForm->second;
}


/*--------------------
  Refresh
  --------------------*/
UI_VOID_CMD(Refresh, 0)
{
    if (pThis == NULL)
        return;

    pThis->GetInterface()->DoEvent(WEVENT_REFRESH);
}
