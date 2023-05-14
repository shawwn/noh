// (C)2008 S2 Games
// c_httpmanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_httpmanager.h"
#include "c_httprequest.h"
//#include "c_phpdata.h"
//#include "c_console.h"

#include "k2_include_windows.h"
#include "curl/curl.h"
//=============================================================================

/*====================
  CHTTPManager::CHTTPManager
  ====================*/
CHTTPManager::~CHTTPManager()
{
    Shutdown();
}


/*====================
  CHTTPManager::CHTTPManager
  ====================*/
CHTTPManager::CHTTPManager() :
m_pCurlMulti(nullptr)
{
}


/*====================
  CHTTPManager::Initialize
  ====================*/
bool    CHTTPManager::Initialize()
{
    m_sUserAgent = "S2 Games";
    m_sUserAgent += "/" + TStringToString(K2System.GetGameName());
    m_sUserAgent += "/" + TStringToString(K2System.GetVersionString());
    m_sUserAgent += "/" + TStringToString(K2System.GetBuildOSString());
    m_sUserAgent += "/" + TStringToString(K2System.GetBuildArchString());

    // Create a cURL "multi" interface to use, this only needs to be done
    // once per instance of the application
    m_pCurlMulti = curl_multi_init();
    if (m_pCurlMulti == nullptr)
        return  false;

    return true;
}


/*====================
  CHTTPManager::Frame
  ====================*/
void    CHTTPManager::Frame()
{
    PROFILE("CHTTPManager::Frame");

    // Allow cURL to update all active requests
    CURLMcode result(CURLM_CALL_MULTI_PERFORM);
    while (result == CURLM_CALL_MULTI_PERFORM)
    {
        int iProcessCount(0);
        result = curl_multi_perform(m_pCurlMulti, &iProcessCount);
        if (result != CURLM_OK && result != CURLM_CALL_MULTI_PERFORM)
            Console.Net << _T("CHTTPManager::Frame() - ") << curl_multi_strerror(result) << newl;
    }

    // Grab messages
    for (;;)
    {
        int iMessageCount(0);
        CURLMsg *pMsg(curl_multi_info_read(m_pCurlMulti, &iMessageCount));
        if (pMsg == nullptr)
            break;

        // cURL docs say this is the only valid message right now
        if (pMsg->msg != CURLMSG_DONE)
            continue;

        CurlResultMap_it itRequest(m_mapResults.find(pMsg->easy_handle));
        if (itRequest == m_mapResults.end())
            continue;

        if (pMsg->data.result == CURLE_OK)
        {
            itRequest->second->Completed();
        }
        else
        {
            Console.Net << L"Request failed: " << curl_easy_strerror(pMsg->data.result) << L" " << ParenStr(itRequest->second->GetErrorBuffer()) << newl;
            itRequest->second->Failed();
        }

        curl_multi_remove_handle(m_pCurlMulti, pMsg->easy_handle);
        
        if (itRequest->second->GetReleaseOnCompletion())
            ReleaseRequest(itRequest->second);
    }

    // Expire dormant requests
    if (!m_deqReleasedRequests.empty())
    {
        CHTTPRequest *pRequest(m_deqReleasedRequests.front());
        if (Host.GetTimeSeconds() - pRequest->GetTimeStamp() > 30)
        {
            m_deqReleasedRequests.pop_front();
            KillRequest(pRequest);
        }
    }
}


/*====================
  CHTTPManager::Shutdown
  ====================*/
void    CHTTPManager::Shutdown()
{
    // Release all active requests
    for (RequestVector_it itRequest(m_vRequests.begin()), itEnd(m_vRequests.end()); itRequest != itEnd; ++itRequest)
    {
        CHTTPRequest *pRequest(*itRequest);
        if (pRequest == nullptr)
            continue;

        curl_multi_remove_handle(m_pCurlMulti, pRequest->GetCURL());
        curl_easy_cleanup(pRequest->GetCURL());

        K2_DELETE(*itRequest);
    }
    m_vRequests.clear();

    // Release the cURL "multi" interface
    if (m_pCurlMulti != nullptr)
    {
        curl_multi_cleanup(m_pCurlMulti);
        m_pCurlMulti = nullptr;
    }
}


/*====================
  CHTTPManager::SpawnRequest
  ====================*/
CHTTPRequest*   CHTTPManager::SpawnRequest()
{
    PROFILE("CHTTPManager::SpawnRequest");

    if (!m_deqReleasedRequests.empty())
    {
        CHTTPRequest *pRequest(m_deqReleasedRequests.back());
        m_deqReleasedRequests.pop_back();
        m_mapResults[pRequest->GetCURL()] = pRequest;
        return pRequest;
    }

    CURL *pCurlEasy(curl_easy_init());
    if (pCurlEasy == nullptr)
        return nullptr;

    uint uiID(INT_SIZE(m_vRequests.size()));
    if (!m_vAvailableRequestIDs.empty())
        uiID = m_vAvailableRequestIDs.back();

    CHTTPRequest *pNewRequest(K2_NEW(ctx_Net,  CHTTPRequest)(this, pCurlEasy, uiID));
    if (pNewRequest == nullptr)
        return nullptr;

    if (m_vAvailableRequestIDs.empty())
    {
        m_vRequests.push_back(pNewRequest);
    }
    else
    {   
        m_vRequests[uiID] = pNewRequest;
        m_vAvailableRequestIDs.pop_back();
    }
    
    m_mapResults[pCurlEasy] = pNewRequest;
    return pNewRequest;
}


/*====================
  CHTTPManager::SendRequest
  ====================*/
void    CHTTPManager::SendRequest(CHTTPRequest *pRequest)
{
    PROFILE("CHTTPManager::SendRequest");

    CURLMcode eCode(curl_multi_add_handle(m_pCurlMulti, pRequest->GetCURL()));
    if (eCode != CURLM_OK)
    {
        Console.Net << L"Send failed: " << curl_multi_strerror(eCode) << L" " << ParenStr(pRequest->GetErrorBuffer()) << newl;
        pRequest->Failed();
    }
}


/*====================
  CHTTPManager::KillRequest
  ====================*/
void    CHTTPManager::KillRequest(CHTTPRequest *pRequest)
{
    PROFILE("CHTTPManager::KillRequest");

    if (pRequest == nullptr)
        return;

    m_mapResults.erase(pRequest->GetCURL());

    curl_multi_remove_handle(m_pCurlMulti, pRequest->GetCURL());
    curl_easy_cleanup(pRequest->GetCURL());

    uint uiID(pRequest->GetID());
    K2_DELETE(m_vRequests[uiID]);
    m_vRequests[uiID] = nullptr;
    m_vAvailableRequestIDs.push_back(uiID);
}


/*====================
  CHTTPManager::ReleaseRequest
  ====================*/
void    CHTTPManager::ReleaseRequest(CHTTPRequest *pRequest)
{
    PROFILE("CHTTPManager::ReleaseRequest");

    if (pRequest == nullptr)
        return;

    m_mapResults.erase(pRequest->GetCURL());

    curl_multi_remove_handle(m_pCurlMulti, pRequest->GetCURL());
    pRequest->Reset();
    pRequest->SetTimeStamp(Host.GetTimeSeconds());

    m_deqReleasedRequests.push_back(pRequest);
}


/*====================
  CHTTPManager::PrintUsage
  ====================*/
void    CHTTPManager::PrintUsage() const
{
    uint uiActive(INT_SIZE((m_vRequests.size() - m_deqReleasedRequests.size()) - m_vAvailableRequestIDs.size()));

    Console
        << newl
        << L"HTTP Requests" << newl
        << L"-------------" << newl
        << L"Active:        " << XtoA(uiActive, FMT_DELIMIT, 5) << " / " << XtoA(INT_SIZE(m_vRequests.size()), FMT_DELIMIT) << newl
        << L"Standby:       " << XtoA(INT_SIZE(m_deqReleasedRequests.size()), FMT_DELIMIT, 5) << newl
        << L"Dead:          " << XtoA(INT_SIZE(m_vAvailableRequestIDs.size()), FMT_DELIMIT, 5) << newl;
}
