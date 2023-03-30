// (C)2010 S2 Games
// c_httpmanager.h
//
//=============================================================================
#ifndef __C_HTTPMANAGER_H__
#define __C_HTTPMANAGER_H__

//=============================================================================
// Declarations
//=============================================================================
class CHTTPRequest;
class CPHPData;
typedef void CURL;
typedef void CURLM;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint INVALID_REQUEST_INDEX(-1);

typedef list<CHTTPRequest*>         HTTPRequestList;
typedef HTTPRequestList::iterator   HTTPRequestList_it;
//=============================================================================

//=============================================================================
// CHTTPManager
//=============================================================================
class CHTTPManager
{
private:
    typedef vector<CHTTPRequest*>   RequestVector;
    typedef RequestVector::iterator RequestVector_it;

    typedef deque<CHTTPRequest*>    RequestDeque;
    typedef RequestDeque::iterator  RequestDeque_it;

    typedef map<CURL*, CHTTPRequest*>   CurlResultMap;
    typedef pair<CURL*, CHTTPRequest*>  CurlResultMap_pair;
    typedef CurlResultMap::iterator     CurlResultMap_it;

    CURLM*          m_pCurlMulti;

    CurlResultMap   m_mapResults;

    RequestVector   m_vRequests;
    RequestDeque    m_deqReleasedRequests;
    uivector        m_vAvailableRequestIDs;

    string          m_sUserAgent;

public:
    ~CHTTPManager();
    CHTTPManager();

    const string&   GetUserAgent() const    { return m_sUserAgent; }

    bool            Initialize();
    void            Frame();
    void            Shutdown();

    K2_API CHTTPRequest*    SpawnRequest();
    void                    SendRequest(CHTTPRequest *pRequest);
    void                    KillRequest(CHTTPRequest *pRequest);
    K2_API void             ReleaseRequest(CHTTPRequest *pRequest);

    void            PrintUsage() const;
};
//=============================================================================
#endif  //__C_HTTPMANAGER_H__