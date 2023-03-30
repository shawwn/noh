// (C)2010 S2 Games
// c_webpanel.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"
#include "c_webpanel.h"

#include "c_httpmanager.h"
#include "c_httprequest.h"
#include "c_widgetstyle.h"
#include "c_phpdata.h"
#include "c_xmlmanager.h"
#include "c_draw2d.h"
#include "c_uicmd.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

/*====================
  CWebPanel::~CWebPanel
  ====================*/
CWebPanel::~CWebPanel()
{
    m_pHTTPManager->ReleaseRequest(m_pRequest);
}


/*====================
  CWebPanel::CWebPanel
  ====================*/
CWebPanel::CWebPanel(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style, CHTTPManager* pHTTPManager) :
CPanel(pInterface, pParent, style, WIDGET_WEBPANEL),
m_pHTTPManager(pHTTPManager),
m_pRequest(NULL),

m_eState(WEBPANEL_BLANK),
m_pStatusTrigger(NULL),
m_bUseSSL(false)
{
    assert(m_pHTTPManager != NULL);

    PROFILE("CXMLProc_webpanel::Process");

    try
    {
        if (!style.HasProperty(_T("name")))
            EX_ERROR(_T("<webpanel> has no name property"));

        tstring sURL;
        const tstring &sHost(style.GetProperty(_T("host")));
        if (sHost == _T("!masterserver"))
            sURL = StringToTString(K2System.GetMasterServerAddress()) + _T("/webpanel");
        else if (sHost == _T("!hon"))
            sURL = _T("www.heroesofnewerth.com/webpanel");
        else
        {
#if defined(K2_CLIENT)
            EX_ERROR(_TS("<webpanel> tried to set invalid host '") + sHost + _T("'"));
#else
            sURL = sHost;
#endif
        }

        SetTargetHost(sURL);
        SetTargetURI(style.GetProperty(_T("target")));
        SetStatusTrigger(style.GetProperty(_T("statustrigger")));
        SetUseSSL(style.GetPropertyBool(_T("ssl")));
    }
    catch (CException& ex)
    {
        ex.Process(_T("CXMLProc_Webpanel::Process() - "), NO_THROW);
    }
}


/*====================
  CWebPanel::SetStatusTrigger
  ====================*/
void    CWebPanel::SetStatusTrigger(const tstring &sName)
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
  CWebPanel::Submit
  ====================*/
void    CWebPanel::Submit(const tsvector &vParams)
{
    if (m_sTargetHost.empty())
    {
        Console.Warn << _T("<webpanel> '") << m_sName << _T("': SubmitWebPanel() ignored, invalid host") << newl;
        return;
    }

    if (m_sTargetURI.empty())
    {
        Console.Warn << _T("<webpanel> '") << m_sName << _T("': SubmitWebPanel() ignored, invalid target") << newl;
        return;
    }

    m_pHTTPManager->ReleaseRequest(m_pRequest);
    m_pRequest = m_pHTTPManager->SpawnRequest();
    if (m_pRequest == NULL)
        return;

    m_pRequest->SetTargetURL(m_sTargetHost + m_sTargetURI);

    for (tsvector_cit it(vParams.begin()); it != vParams.end(); ++it)
    {
        const tstring &sName(*it);
        ++it;
        if (it == vParams.end())
            break;
        const tstring &sValue(*it);
        
        m_pRequest->AddVariable(sName, sValue);
    }

    if (m_bUseSSL)
        m_pRequest->SendSecureRequest();
    else
        m_pRequest->SendRequest();
}


/*====================
  CWebPanel::Frame
  ====================*/
void    CWebPanel::Frame(uint uiFrameLength, bool bProcessFrame)
{
    IWidget::Frame(uiFrameLength, bProcessFrame);

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
  CWebPanel::ProcessResponse
  ====================*/
void    CWebPanel::ProcessResponse()
{
    //Console.UI << m_pRequest->GetResponse() << newl;
    const string& sContents(TStringToUTF8(m_pRequest->GetResponse()));

    vector<IWidget*> vChildren(GetChildList());
    for (vector<IWidget*>::iterator itChild(vChildren.begin()), itEnd(vChildren.end()); itChild != itEnd; ++itChild)
        SAFE_DELETE(*itChild);

    if (!sContents.empty())
    {
        if (!XMLManager.ReadBuffer(sContents.c_str(), (int)sContents.size(), _T("uielements"), this))
        {
            Console.Warn << _T("<webpanel> failed to read XML.  URL: ") << (m_sTargetHost + m_sTargetURI) << newl;
        }
    }

    RecalculateSize();
}


/*--------------------
  SubmitWebPanel
  --------------------*/
UI_CMD(SubmitWebPanel, 1)
{
    const tstring& sWebPanelName(vArgList[0]->Evaluate());
    IWidget* pWidget(UIManager.FindWidget(sWebPanelName));

    if (pWidget == NULL)
    {
        Console.Err << _T("SubmitWebPanel('") << sWebPanelName << _T("') - The specified web panel does not exist") << newl;
        return _T("0");
    }

    if (pWidget->GetType() != WIDGET_WEBPANEL)
    {
        Console.Err << _T("SubmitWebPanel('") << sWebPanelName << _T("') - The specified widget is not a webpanel") << newl;
        return _T("0");
    }

    CWebPanel* pWebPanel((CWebPanel*)pWidget);

    tsvector vParams;
    for (ScriptTokenVector_cit it(vArgList.begin() + 1); it != vArgList.end(); ++it)
        vParams.push_back((*it)->Evaluate());

    pWebPanel->Submit(vParams);
    return _T("1");
}

