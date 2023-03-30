// (C)2005 S2 Games
// c_uiform.h
//
//=============================================================================
#ifndef __C_UIFORM_H__
#define __C_UIFORM_H__

//=============================================================================
// Declarations
//=============================================================================
class CHTTPRequest;
class CUITrigger;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EFormMethod
{
    FORM_METHOD_GET,
    FORM_METHOD_POST
};
//=============================================================================

//=============================================================================
// CUIForm
//=============================================================================
class CUIForm
{
private:
    CHTTPManager*           m_pHTTPManager;
    CHTTPRequest*           m_pRequest;

    tstring                 m_sName;
    tstring                 m_sTargetHost;
    tstring                 m_sTargetURI;
    CUITrigger*             m_pStatusTrigger;
    CUITrigger*             m_pResultTrigger;
    tsmapts                 m_mapVariables;
    map<tstring, IWidget*>  m_mapWidgetVariables;
    map<tstring, uint>      m_mapResults;

    EFormMethod             m_eMethod;
    bool                    m_bUseSSL;

    CUIForm();

public:
    K2_API ~CUIForm();
    K2_API CUIForm(CHTTPManager *pHTTPManager, const tstring &sName);

    const tstring&  GetName()   { return m_sName; }

    void            SetTargetHost(const tstring &sTargetHost)       { m_sTargetHost = sTargetHost; }
    void            SetTargetURI(const tstring &sTargetURI)         { m_sTargetURI = sTargetURI; }
    void            SetMethod(EFormMethod eMethod)                  { m_eMethod = eMethod; }
    void            SetUseSSL(bool bUseSSL)                         { m_bUseSSL = bUseSSL; }

    void            SetStatusTrigger(const tstring &sName);
    void            SetResultTrigger(const tstring &sName);
    void            SetResultParam(uint uiParam, const tstring &sVariable)      { m_mapResults[sVariable] = uiParam; }
    void            AddWidgetVariable(const tstring &sName, IWidget *pWidget)   { if (pWidget != NULL) m_mapWidgetVariables[sName] = pWidget; }

    void    Submit(const tsvector &vParams);
    void    Frame();
    void    ProcessResponse();
};
//=============================================================================

#endif //__C_UIFORM_H__
