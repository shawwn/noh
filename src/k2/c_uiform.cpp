// (C)2008 S2 Games
// c_uiform.cpp
// 
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_uiform.h"
#include "c_uicmd.h"
#include "c_uitriggerregistry.h"
#include "c_uitrigger.h"
#include "c_uimanager.h"
#include "c_interface.h"
#include "c_phpdata.h"
#include "c_httpmanager.h"
#include "c_httprequest.h"
//=============================================================================

/*====================
  CUIForm::~CUIForm
  ====================*/
CUIForm::~CUIForm()
{
    m_pHTTPManager->ReleaseRequest(m_pRequest);
}


/*====================
  CUIForm::CUIForm
  ====================*/
CUIForm::CUIForm(CHTTPManager *pHTTPManager, const tstring &sName) :
m_pHTTPManager(pHTTPManager),
m_pRequest(NULL),

m_sName(sName),
m_pStatusTrigger(NULL),
m_pResultTrigger(NULL),
m_eMethod(FORM_METHOD_POST),
m_bUseSSL(false)
{
}


/*====================
  CUIForm::SetStatusTrigger
  ====================*/
void    CUIForm::SetStatusTrigger(const tstring &sName)
{
    if (sName.empty())
        return;

    CUITrigger *pTrigger(UITriggerRegistry.GetUITrigger(sName));
    if (pTrigger == NULL)
    {
        Console.Warn << _T("Could not find trigger ") << QuoteStr(sName) << _T(" for form ") << QuoteStr(m_sName) << newl;
        return;
    }

    m_pStatusTrigger = pTrigger;
}


/*====================
  CUIForm::SetResultTrigger
  ====================*/
void    CUIForm::SetResultTrigger(const tstring &sName)
{
    if (sName.empty())
        return;

    CUITrigger *pTrigger(UITriggerRegistry.GetUITrigger(sName));
    if (pTrigger == NULL)
    {
        Console.Warn << _T("Could not find trigger ") << QuoteStr(sName) << _T(" for form ") << QuoteStr(m_sName) << newl;
        return;
    }

    m_pResultTrigger = pTrigger;
}


/*====================
  CUIForm::Submit
  ====================*/
void    CUIForm::Submit(const tsvector &vParams)
{
    m_pHTTPManager->ReleaseRequest(m_pRequest);
    m_pRequest = m_pHTTPManager->SpawnRequest();
    if (m_pRequest == NULL)
        return;

    m_pRequest->SetTargetURL(m_sTargetHost + m_sTargetURI);
    
    for (tsmapts_it it(m_mapVariables.begin()); it != m_mapVariables.end(); ++it)
        m_pRequest->AddVariable(it->first, it->second);
    
    for (map<tstring, IWidget*>::iterator it(m_mapWidgetVariables.begin()); it != m_mapWidgetVariables.end(); ++it)
        m_pRequest->AddVariable(it->first, it->second->GetValue());

    for (tsvector_cit it(vParams.begin()); it != vParams.end(); ++it)
    {
        const tstring &sName(*it);
        ++it;
        if (it == vParams.end())
            break;
        const tstring &sValue(*it);
        
        m_pRequest->AddVariable(sName, sValue);
    }

    switch (m_eMethod)
    {
    case FORM_METHOD_GET:
        if (m_bUseSSL)
            m_pRequest->SendSecureRequest();
        else
            m_pRequest->SendRequest();
        break;
    case FORM_METHOD_POST:
        if (m_bUseSSL)
            m_pRequest->SendSecurePostRequest();
        else
            m_pRequest->SendPostRequest();
        break;
    }
}


/*====================
  CUIForm::Frame
  ====================*/
void    CUIForm::Frame()
{
    if (m_pRequest == NULL)
        return;

    if (m_pStatusTrigger != NULL)
        m_pStatusTrigger->Trigger(XtoW(m_pRequest->GetStatus()));

    if (m_pRequest->IsActive())
        return;

    if (m_pRequest->WasSuccessful())
        ProcessResponse();

    m_pHTTPManager->ReleaseRequest(m_pRequest);
    m_pRequest = NULL;
}


/*====================
  CUIForm::ProcessResponse
  ====================*/
void    CUIForm::ProcessResponse()
{
    if (m_pResultTrigger == NULL)
        return;

    //Console.UI << m_pRequest->GetResponse() << newl;
    CPHPData phpResponse(m_pRequest->GetResponse());

    if (!phpResponse.IsValid())
        return;

    tsvector vParams;
    for (map<tstring, uint>::iterator it(m_mapResults.begin()); it != m_mapResults.end(); ++it)
    {
        if (vParams.size() <= it->second)
            vParams.resize(it->second + 1);

        const tstring &sName(it->first);
        size_t zPos(0);
        const CPHPData *pVar(&phpResponse);
        
        for(;;)
        {
            size_t zOldPos(zPos);
            zPos = sName.find(_T("."), zPos);
            pVar = pVar->GetVar(sName.substr(zOldPos, zPos));
            if (pVar == NULL || zPos == tstring::npos)
                break;

            ++zPos;
        }

        vParams[it->second] = pVar ? pVar->GetString() : TSNULL;
    }

    m_pResultTrigger->Trigger(vParams);
}


/*--------------------
  SubmitForm
  --------------------*/
UI_VOID_CMD(SubmitForm, 1)
{
    CInterface *pInterface(pThis->GetInterface());
    if (pInterface == NULL)
    {
        Console.Warn << _T("SubmitForm: No active interface") << newl;
        return;
    }

    CUIForm *pForm(pInterface->GetForm(vArgList[0]->Evaluate()));
    if (pForm == NULL)
        return;

    tsvector vParams;
    for (ScriptTokenVector_cit it(vArgList.begin() + 1); it != vArgList.end(); ++it)
        vParams.push_back((*it)->Evaluate());

    pForm->Submit(vParams);
}
