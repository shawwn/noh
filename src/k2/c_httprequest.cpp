// (C)2010 S2 Games
// c_httprequest.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "k2_include_windows.h"
#include "curl/curl.h"

#include "c_httprequest.h"
#include "c_httpmanager.h"
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
CVAR_UINTF(http_defaultTimeout,         30,     CVAR_SAVECONFIG);
CVAR_UINTF(http_defaultConnectTimeout,  15,     CVAR_SAVECONFIG);
CVAR_UINTF(http_defaultLowSpeedLimit,   10,     CVAR_SAVECONFIG);
CVAR_UINTF(http_defaultLowSpeedTimeout, 25,     CVAR_SAVECONFIG);
CVAR_BOOLF(http_useCompression,         true,   CVAR_SAVECONFIG);
CVAR_BOOLF(http_printDebugInfo,         false,  CVAR_SAVECONFIG);
//=============================================================================

/*====================
  CHTTPRequest::~CHTTPRequest
  ====================*/
CHTTPRequest::~CHTTPRequest()
{
    K2_DELETE(m_pErrorBuffer);
}


/*====================
  CHTTPRequest::CHTTPRequest
  ====================*/
CHTTPRequest::CHTTPRequest(CHTTPManager *pHTTPManager, void *pCurlEasy, uint uiID) :
m_pHTTPManager(pHTTPManager),

m_uiID(uiID),
m_pCurlEasy(pCurlEasy),
m_eStatus(HTTP_REQUEST_IDLE),
m_pErrorBuffer(nullptr),

m_bReleaseOnCompletion(false),
m_uiTimeout(http_defaultTimeout),
m_uiConnectTimeout(http_defaultConnectTimeout),
m_uiLowSpeedLimit(http_defaultLowSpeedLimit),
m_uiLowSpeedTime(http_defaultLowSpeedTimeout),

m_uiType(-1)
{
}


/*====================
  CHTTPRequest::Completed
  ====================*/
void    CHTTPRequest::Completed()
{
    m_bufferResponse << byte(0);
    m_sResponse = UTF8ToTString(m_bufferResponse.Get());

    m_eStatus = HTTP_REQUEST_SUCCESS;
    m_bufferResponse.Clear();
    m_bufferResponse.Resize(0);
}


/*====================
  CHTTPRequest::Failed
  ====================*/
void    CHTTPRequest::Failed()
{
    m_eStatus = HTTP_REQUEST_ERROR;
    m_bufferResponse.Clear();
    m_bufferResponse.Resize(0);
}


/*====================
  CHTTPRequest::SendRequest
  ====================*/
size_t  CURLWriteMemoryCallback(void *pSrc, size_t zSize, size_t zBlocks, void *pDest)
{
    size_t zTotalSize(zSize * zBlocks);
    CBufferDynamic *pBuffer(static_cast<CBufferDynamic*>(pDest));

    pBuffer->Append(pSrc, INT_SIZE(zTotalSize));

    return zTotalSize;
}

int CURLDebugCallback(CURL *pCurlEasy, curl_infotype type, char *pString, size_t zSize, void *pData)
{
    CBufferDynamic buffer(uint(zSize + 1));
    buffer.Write(pString, uint(zSize));
    buffer << byte(0);

    switch (type)
    {
    case CURLINFO_TEXT:
        Console.Net << _T("CURL: ") << buffer.Get();
        break;
    case CURLINFO_HEADER_IN:
        Console.Net << _T("CURL (header in): ") << NormalizeLineBreaks(buffer.Get());
        break;
    case CURLINFO_HEADER_OUT:
        Console.Net << _T("CURL (header out): ") << NormalizeLineBreaks(buffer.Get());
        break;
    case CURLINFO_DATA_IN:
        Console.Net << _T("CURL (data in): ") << NormalizeLineBreaks(buffer.Get()) << newl;
        break;
    case CURLINFO_DATA_OUT:
        Console.Net << _T("CURL (data out): ") << NormalizeLineBreaks(buffer.Get()) << newl;
        break;
    case CURLINFO_SSL_DATA_IN:
        Console.Net << _T("CURL (SSL data in): ") << NormalizeLineBreaks(buffer.Get()) << newl;
        break;
    case CURLINFO_SSL_DATA_OUT:
        Console.Net << _T("CURL (SSL data out): ") << NormalizeLineBreaks(buffer.Get()) << newl;
        break;
    case CURLINFO_END:
        K2_UNREACHABLE();
        break;
    }
    return 0;
}

void    CHTTPRequest::SendRequest(const string &sURL, bool bPost, bool bSSL)
{
    PROFILE("CHTTPRequest::SendRequest");
    string sFinalURL(sURL);

    if (bPost && m_vVariables.empty())
    {
        Console.Net << _T("CHTTPRequest::SendRequest() - POST request has no parameters") << newl;
        return;
    }

    m_bufferResponse.Clear();
    m_sResponse.clear();

    // Assemble POST string
    if (bPost)
    {
#ifdef __APPLE__
        // hack to work around buggy network monitoring when managed account is used: it is changing the last 2 chars in the post data to "Pr"
        // add a dummy variable/value at the end so none of the actual data gets corrupted
        AddVariable(_T("neverusethisvar"), _T("xx"));
#endif
        string sVarString;
        for (StringPairVector_it it(m_vVariables.begin()); it != m_vVariables.end(); ++it) {
            const auto& sKey(it->first);
            const auto& sVal(it->second);

            // TKTK 2023: special case for the new HoN servers: make f be a query parameter
            if (sKey == "f") {
                sFinalURL += (sFinalURL.find('?') != string::npos) ? "&" : "?";
                sFinalURL += sKey;
                sFinalURL += "=";
                sFinalURL += sVal;
            } else {
                sVarString += (sVarString.empty() ? "" : "&");
                sVarString += sKey;
                sVarString += "=";
                sVarString += sVal;
            }
        }

        curl_easy_setopt(m_pCurlEasy, CURLOPT_POST, 1l);
        curl_easy_setopt(m_pCurlEasy, CURLOPT_COPYPOSTFIELDS, sVarString.c_str());
        curl_easy_setopt(m_pCurlEasy, CURLOPT_POSTFIELDSIZE, -1l);
    }

    // Use IPv4
    curl_easy_setopt(m_pCurlEasy, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

    // Set target URL
    curl_easy_setopt(m_pCurlEasy, CURLOPT_URL, sFinalURL.c_str());

    // User Agent
    curl_easy_setopt(m_pCurlEasy, CURLOPT_USERAGENT, m_pHTTPManager->GetUserAgent().c_str());

    // Set timeout parameters
    curl_easy_setopt(m_pCurlEasy, CURLOPT_TIMEOUT, m_uiTimeout);
    curl_easy_setopt(m_pCurlEasy, CURLOPT_LOW_SPEED_LIMIT, m_uiLowSpeedLimit);
    curl_easy_setopt(m_pCurlEasy, CURLOPT_LOW_SPEED_TIME, m_uiLowSpeedTime);
    curl_easy_setopt(m_pCurlEasy, CURLOPT_CONNECTTIMEOUT, m_uiConnectTimeout);

    // Debugging
    if (http_printDebugInfo)
    {
        curl_easy_setopt(m_pCurlEasy, CURLOPT_VERBOSE, 1l);
        curl_easy_setopt(m_pCurlEasy, CURLOPT_DEBUGFUNCTION, CURLDebugCallback);
    }

    // Error buffer
    m_pErrorBuffer = K2_NEW_ARRAY(ctx_Net, char, CURL_ERROR_SIZE+1);
    MemManager.Set(m_pErrorBuffer, 0, CURL_ERROR_SIZE + 1);
    curl_easy_setopt(m_pCurlEasy, CURLOPT_ERRORBUFFER, (void*)m_pErrorBuffer);

    // SSL
    if (bSSL)
    {
        curl_easy_setopt(m_pCurlEasy, CURLOPT_SSL_VERIFYPEER, 1l);
        curl_easy_setopt(m_pCurlEasy, CURLOPT_SSL_VERIFYHOST, 2l);
#ifdef _WIN32
        string sCertPath(WCSToMBS(FileManager.GetSystemPath(_T(":/ca-bundle.crt"))));
#else
        string sCertPath(TStringToNative(FileManager.GetSystemPath(_T(":/ca-bundle.crt"))));
//        curl_easy_setopt(m_pCurlEasy, CURLOPT_CAPATH, _T("/etc/ssl/certs"));
#endif
        curl_easy_setopt(m_pCurlEasy, CURLOPT_CAINFO, sCertPath.c_str());
    }

    // Compression
    if (http_useCompression)
        curl_easy_setopt(m_pCurlEasy, CURLOPT_ENCODING, "gzip,deflate");

    // Output
    curl_easy_setopt(m_pCurlEasy, CURLOPT_WRITEFUNCTION, CURLWriteMemoryCallback);
    curl_easy_setopt(m_pCurlEasy, CURLOPT_WRITEDATA, static_cast<void*>(&m_bufferResponse));

    m_eStatus = HTTP_REQUEST_SENDING;
    m_pHTTPManager->SendRequest(this);
}


/*====================
  CHTTPRequest::Reset
  ====================*/
void    CHTTPRequest::Reset()
{
    PROFILE("CHTTPRequest::Reset");

    m_eStatus = HTTP_REQUEST_IDLE;
    m_sURL.clear();
    m_vVariables.clear();
    m_bufferResponse.Clear();
    m_sResponse.clear();
    K2_DELETE(m_pErrorBuffer);
    m_pErrorBuffer = nullptr;
    m_bReleaseOnCompletion = false;
    
    m_uiType = -1;

    m_uiTimeout = http_defaultTimeout;
    m_uiConnectTimeout = http_defaultConnectTimeout;
    m_uiLowSpeedLimit = http_defaultLowSpeedLimit;
    m_uiLowSpeedTime = http_defaultLowSpeedTimeout;
}


/*====================
  CHTTPRequest::Wait
  ====================*/
void    CHTTPRequest::Wait()
{
    while (m_eStatus == HTTP_REQUEST_SENDING)
    {
        m_pHTTPManager->Frame();
        K2System.Sleep(1);
    }
}
