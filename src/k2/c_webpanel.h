// (C)2010 S2 Games
// c_webpanel.h
//
//=============================================================================
#ifndef __C_WEBPANEL_H__
#define __C_WEBPANEL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"
#include "c_panel.h"
#include "c_filehttp.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CHTTPManager;
class CHTTPRequest;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EWebPanelState
{
	WEBPANEL_BLANK = 0,
	WEBPANEL_DOWNLOADING,
	WEBPANEL_FINISHED
};
//=============================================================================

//=============================================================================
// CWebPanel
//=============================================================================
class CWebPanel : public CPanel
{
private:
	CHTTPManager*			m_pHTTPManager;
	CHTTPRequest*			m_pRequest;

	tstring					m_sName;
	tstring					m_sTargetHost;
	tstring					m_sTargetURI;
	CFileHTTP				m_fileHTTP;
	EWebPanelState			m_eState;
	CUITrigger*				m_pStatusTrigger;

	bool					m_bUseSSL;

	void			ProcessResponse();

public:
	~CWebPanel();
	K2_API CWebPanel(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style, CHTTPManager* pHTTPManager);

	void			SetTargetHost(const tstring &sTargetHost)		{ m_sTargetHost = sTargetHost; }
	void			SetTargetURI(const tstring &sTargetURI)			{ m_sTargetURI = sTargetURI; }
	void			SetUseSSL(bool bUseSSL)							{ m_bUseSSL = bUseSSL; }

	void			SetStatusTrigger(const tstring &sName);

	void			Submit(const tsvector &vParams);
	virtual void	Frame(uint uiFrameLength, bool bProcessFrame);
};
//=============================================================================

#endif //__C_WEBPANEL_H__