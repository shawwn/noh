// (C)2005 S2 Games
// c_uiscript.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_uiscript.h"
#include "c_uicmd.h"
#include "c_uicmdregistry.h"
#include "c_uimanager.h"
#include "c_interface.h"
#include "parser.h"
#include "evaluator.h"
#include "c_function.h"
#include "c_uitriggerregistry.h"
#include "c_uitrigger.h"
#include "c_table.h"
#include "c_uiscripttoken.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
SINGLETON_INIT(CUIScript)
CUIScript *g_pUIScript(CUIScript::GetInstance());

CVAR_BOOL(ui_debugScript, false);
//=============================================================================

/*====================
  CUIScript::CUIScript
  ====================*/
CUIScript::CUIScript() :
m_pActiveWidget(NULL)
{
}


/*====================
  CUIScript::Evaluate
  ====================*/
tstring CUIScript::Evaluate(IWidget *pCaller, const tstring &sScript, const tsvector &vParams)
{
    PROFILE_EX("CUIScript::Evaluate", PROFILE_LEAF);

    if (ui_debugScript)
        Console << pCaller->GetInterface()->GetName() << _T(": ") << sScript << newl;

    if (sScript.empty())
        return _T("");
 
    m_pActiveWidget = pCaller;
    return ExpressionEvaluator.Evaluate(sScript, pCaller, vParams);
}


/*--------------------
  UICall
  --------------------*/
CMD(UICall)
{
    try
    {
        if (vArgList.size() < 2)
            EX_MESSAGE(_T("syntax: UICall <widget> <uicmd>"));

        IWidget *pWidget(NULL);

        if (vArgList[0] == _T("*"))
            pWidget = UIManager.GetActiveInterface();
        else
            pWidget = UIManager.FindWidget(vArgList[0]);

        if (!pWidget)
            EX_MESSAGE(_TS("Widget ") + SingleQuoteStr(vArgList[0]) + _T(" not found"));

        UIScript.Evaluate(pWidget, ConcatinateArgs(vArgList.begin() + 1, vArgList.end()));
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("UICall - "), NO_THROW);
        return false;
    }
}


/*--------------------
  Call
  --------------------*/
UI_CMD(Call, 2)
{
    try
    {
        if (pThis == NULL)
            return TSNULL;

        if (!pThis->GetInterface())
            EX_MESSAGE(_T("No interface"));

        // Set event parameters on the target widget
        // properly, so that #param# works in UICall
        tstring sWidgetName(vArgList[0]->Evaluate());
        IWidget *pWidget(pThis->GetInterface()->GetWidget(sWidgetName));

        if (!pWidget)
            EX_MESSAGE(_TS("Widget ") + SingleQuoteStr(sWidgetName) + _T(" not found"));

        pWidget->SetEventParam(pThis->GetEventParam());

        return UIScript.Evaluate(pWidget, vArgList[1]->Evaluate());
    }
    catch (CException &ex)
    {
        ex.Process(_T("Call - "), NO_THROW);
        return TSNULL;
    }
}

/*--------------------
  RefCall
  --------------------*/
UI_CMD(RefCall, 2)
{
    try
    {
        if (pThis == NULL)
            return TSNULL;

        CUICmd *pCmd(CUICmdRegistry::GetInstance()->GetUICmd(vArgList[0]->Evaluate()));

        if (pCmd == NULL)
            return TSNULL;

        return pCmd->Execute(pThis, ScriptTokenVector(vArgList.begin() + 1, vArgList.end()));
    }
    catch (CException &ex)
    {
        ex.Process(_T("RefCall - "), NO_THROW);
        return TSNULL;
    }
}


/*--------------------
  GroupCall
  --------------------*/
UI_VOID_CMD(GroupCall, 2)
{
    if (pThis == NULL)
        return;

    if (!pThis->GetInterface())
        return;

    // Set event parameters on the target widget
    // properly, so that #param# works in UICall
    tstring sGroupName(vArgList[0]->Evaluate());
    WidgetGroup *pGroup(pThis->GetInterface()->GetGroup(sGroupName));

    if (pGroup == NULL)
    {
        Console.Warn << _T("Group ") << SingleQuoteStr(sGroupName) << _T(" not found") << newl;
        return;
    }

    for (WidgetGroup_it itWidget(pGroup->begin()); itWidget != pGroup->end(); ++itWidget)
    {
        (*itWidget)->SetEventParam(pThis->GetEventParam());
        UIScript.Evaluate(*itWidget, vArgList[1]->Evaluate());
    }
}


/*--------------------
  GroupCount
  --------------------*/
UI_CMD(GroupCount, 1)
{
    if (pThis == NULL)
        return TSNULL;

    if (!pThis->GetInterface())
        return TSNULL;

    // Set event parameters on the target widget
    // properly, so that #param# works in UICall
    tstring sGroupName(vArgList[0]->Evaluate());
    WidgetGroup *pGroup(pThis->GetInterface()->GetGroup(sGroupName));

    if (pGroup == NULL)
    {
        Console.Warn << _T("Group ") << SingleQuoteStr(sGroupName) << _T(" not found") << newl;
        return TSNULL;
    }

    return XtoA((uint)pGroup->size());
}


/*--------------------
  Evaluate
  --------------------*/
UI_CMD(Evaluate, 1)
{
    try
    {
        if (pThis == NULL)
            return TSNULL;

        pThis->SetEventParam(pThis->GetEventParam());

        return UIScript.Evaluate(pThis, vArgList[0]->Evaluate());
    }
    catch (CException &ex)
    {
        ex.Process(_T("Call - "), NO_THROW);
        return TSNULL;
    }
}


/*--------------------
  Debug
  --------------------*/
UI_CMD(Debug, 0)
{
    try
    {
        tsvector vMessages;
        for (size_t i = 0; i < vArgList.size(); ++i)
        {
            vMessages.push_back(vArgList[i]->Evaluate());
            const tstring &sMessage(vMessages.back());
            if (i == 0)
                Console.UI << _T("^c[DEBUG]^*:   ^g") << sMessage << newl;
            else
                Console.UI << _T("^c[DEBUG-") << (uint)i << _T("]^*: ^g") << sMessage << newl;
        }

        assert(false);

        if (!vArgList.empty())
            return vMessages[0];

        return _T("1");
    }
    catch (CException &ex)
    {
        ex.Process(_T("DebugBreak - "), NO_THROW);
        return TSNULL;
    }
}


