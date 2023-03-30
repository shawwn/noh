// (C)2005 S2 Games
// c_filehttp.cpp
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_filehttp.h"
#include "curl/curl.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
K2_API HTTPRequestMap   g_mapHTTPRequests;

K2_API CURLM*           g_pcURLMulti = NULL;

CVAR_UINT(net_httpConnectTimeout, 15);
//=============================================================================

/*====================
  cURL_Initialize
  ====================*/
void    cURL_Initialize()
{
    if (g_pcURLMulti)
        cURL_Shutdown();

    curl_global_init(CURL_GLOBAL_ALL);

    g_pcURLMulti = curl_multi_init();

    // TODO: Pipelining causing crashes w/ updater. Try
    //       it again later with a newer build of libcURL?
//  curl_multi_setopt(g_pcURLMulti, CURLMOPT_PIPELINING, 1);
}


/*====================
  cURL_Shutdown
  ====================*/
void    cURL_Shutdown()
{
    HTTPRequestMap_it it;

    if (g_pcURLMulti == NULL)
        return;

    for (it = g_mapHTTPRequests.begin(); it != g_mapHTTPRequests.end(); it++)
        if ((*it).second.handle != NULL && g_pcURLMulti != NULL)
            curl_multi_remove_handle(g_pcURLMulti, (*it).second.handle);

    curl_multi_cleanup(g_pcURLMulti);
    g_pcURLMulti = NULL;

    for (it = g_mapHTTPRequests.begin(); it != g_mapHTTPRequests.end(); it++)
    {
        if ((*it).second.handle != NULL)
            curl_easy_cleanup((*it).second.handle);
    
        SAFE_DELETE((*it).second.buf.buf);
        SAFE_DELETE((*it).second.pURL);
    }

    g_mapHTTPRequests.clear();

    curl_global_cleanup();
}


/*====================
  cURL_Frame
  ====================*/
int     cURL_Frame()
{
    int iNumHandles(0);
    int iNumMsgs(0);
    CURLMsg *pMsg(NULL);
    httpResult_e result;
    CURLMcode multResult;

    if (g_pcURLMulti == NULL)
        return -1;

    multResult = curl_multi_perform(g_pcURLMulti, &iNumHandles);

    if (multResult != CURLM_OK && multResult != CURLM_CALL_MULTI_PERFORM)
        return (int)multResult;

    pMsg = curl_multi_info_read(g_pcURLMulti, &iNumMsgs);

    while (pMsg != NULL)
    {
        if (pMsg->msg == CURLMSG_DONE)
        {
            if (pMsg->data.result == CURLE_OK)
                result = HTTP_SUCCESS;
            else
                result = HTTP_FAILED;

            for (HTTPRequestMap_it it(g_mapHTTPRequests.begin()); it != g_mapHTTPRequests.end(); it++)
            {
                if ((*it).second.handle != NULL && (*it).second.handle == pMsg->easy_handle)
                {
                    if (result == HTTP_SUCCESS)
                    {
                        double dSize;

                        if ((*it).second.iMode & FILE_HTTP_UPLOAD)
                            curl_easy_getinfo((*it).second.handle, CURLINFO_CONTENT_LENGTH_UPLOAD, &dSize);
                        else
                            curl_easy_getinfo((*it).second.handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &dSize);

                        (*it).second.uiTotalSize = uint(dSize);
                    }
                    else if (result == HTTP_FAILED)
                    {
#if defined(__APPLE__) && (MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_3)
                        Console << _T("^r[HTTP] Error encountered on file ") << (*it).second.sUrl << _T(": ") << pMsg->data.result << newl;
#else
                        const char *pError;

                        pError = curl_easy_strerror(pMsg->data.result);
                        Console << _T("^r[HTTP] Error encountered on file ") << (*it).second.sUrl << _T(": ") << pError << newl;
#endif
                    }

                    (*it).second.eResult = result;
                    (*it).second.handle = NULL;

                    CURL *pHandle = pMsg->easy_handle;
                    curl_multi_remove_handle(g_pcURLMulti, pHandle);
                    curl_easy_cleanup(pHandle);

                    break;
                }
            }
        }

        pMsg = curl_multi_info_read(g_pcURLMulti, &iNumMsgs);
    }

    return (int)multResult;
}


/*====================
  HTTP_WriteFileCallback
  ====================*/
size_t  HTTP_WriteFileCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
    size_t written(0);
    size_t realsize = size * nmemb;
    CFile *pFile = static_cast<CFile *>(data);

    if (pFile != NULL && pFile->IsOpen())
        written = pFile->Write(ptr, realsize);

    return written;
}


/*====================
  HTTP_ReadFileCallback
  ====================*/
size_t  HTTP_ReadFileCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
    int written(0);
    size_t realsize = size * nmemb;
    CFile *pFile = static_cast<CFile *>(data);

    if (pFile->IsOpen() && realsize > 0)
        written = pFile->Read((char *)ptr, INT_SIZE(realsize));

    return (size_t)written;
}


/*====================
  HTTP_WriteMemoryCallback
  ====================*/
size_t  HTTP_WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
    size_t realsize = size * nmemb;
    SByteBuffer *buf = static_cast<SByteBuffer *>(data);

    size_t oursize = realsize;
    if (realsize > buf->max_size - buf->size)
    {
        oursize = realsize - (buf->max_size - buf->size);
        buf->buf = static_cast<byte*>(realloc(buf->buf, buf->max_size + oursize));
        buf->max_size += oursize;
    }

    if (buf->buf)
    {
        MemManager.Copy(&(buf->buf[buf->size]), ptr, realsize);
        buf->size += realsize;
    }
    return realsize;
}


/*====================
  HTTP_ProgressCallback
  ====================*/
int HTTP_ProgressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
    if (clientp != NULL)
    {
        SHTTPProgress *dComplete = (SHTTPProgress *)(clientp);

        if (dltotal != 0)
        {
            dComplete->fPercent = float(dlnow / dltotal);
            dComplete->dDownloaded = dlnow;
            dComplete->dSize = dltotal;
        }
        else if (ultotal != 0)
        {
            dComplete->fPercent = float(ulnow / ultotal);
            dComplete->dDownloaded = ulnow;
            dComplete->dSize = ultotal;
        }
        else
        {
            dComplete->fPercent = 0.0f;
            dComplete->dDownloaded = 0;
            dComplete->dSize = 0;
        }
    }

    return 0;
}


/*====================
  CFileHTTP::CFileHTTP
  ====================*/
CFileHTTP::CFileHTTP() :
m_pFile(NULL)
{
}


/*====================
  CFileHTTP::~CFileHTTP
  ====================*/
CFileHTTP::~CFileHTTP()
{
    if (m_pBuffer)
        K2_DELETE_ARRAY(m_pBuffer);
}


/*====================
  CFileHTTP::GetHTTPSize
  ====================*/
double  CFileHTTP::GetHTTPSize(const string &sPath)
{
    double dSize(0);
    CURL *curl = curl_easy_init();

    if (curl == NULL)
        return 0;
    
    // use IPv4
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

    curl_easy_setopt(curl, CURLOPT_URL, sPath.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1l);

    curl_easy_perform(curl);

    curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &dSize);

    curl_easy_cleanup(curl);

    return dSize;
}


static int HTTP_DebugCallback(CURL *pCurlEasy, curl_infotype type, char *pString, size_t zSize, void *pData)
{
    CBufferDynamic buffer(uint(zSize + 1));
    buffer.Write(pString, uint(zSize));
    buffer << byte(0);

    switch (type)
    {
    case CURLINFO_TEXT:
        Console.Net << L"CURL: " << buffer.Get();
        break;
    case CURLINFO_HEADER_IN:
        Console.Net << L"CURL (header in): " << NormalizeLineBreaks(buffer.Get());
        break;
    case CURLINFO_HEADER_OUT:
        Console.Net << L"CURL (header out): " << NormalizeLineBreaks(buffer.Get());
        break;
    case CURLINFO_DATA_IN:
        Console.Net << L"CURL (data in): " << NormalizeLineBreaks(buffer.Get());
        break;
    case CURLINFO_DATA_OUT:
        Console.Net << L"CURL (data out): " << NormalizeLineBreaks(buffer.Get());
        break;
    }
    return 0;
}


/*====================
  CFileHTTP::Open
  ====================*/
bool    CFileHTTP::Open(const tstring &sPath, int iMode)
{
    string sPathSingle(TStringToUTF8(sPath));

    m_iMode = iMode;

    m_bOpen = false;
    m_bUploaded = false;
    m_bError = false;

    if (m_iMode & FILE_BLOCK)
    {
        CURLcode res;
        SByteBuffer buf;
        CURL *curl = NULL;
        int start = K2System.Milliseconds();

        if (m_iMode & FILE_HTTP_GETSIZE)
        {
            m_uiSize = uint(GetHTTPSize(sPathSingle));
            m_bOpen = true;
            return true;
        }

        if ((m_iMode & FILE_HTTP_UPLOAD || m_iMode & FILE_HTTP_WRITETOFILE) && m_pFile == NULL)
            return false;

        buf.buf = NULL;
        buf.max_size = 0;
        buf.size = 0;

        curl = curl_easy_init();
        if (!curl)
        {
            Console.Err << "Curl has not been initialized yet!" << newl;
            return false;
        }
        
        // use IPv4
        curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

        MemManager.Set(&m_Progress, 0, sizeof(SHTTPProgress));

        // Use all encoding options
        curl_easy_setopt(curl, CURLOPT_ENCODING, "");
        curl_easy_setopt(curl, CURLOPT_URL, sPathSingle.c_str());
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, true);
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1l);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1l);

        char szErrorBuffer[CURL_ERROR_SIZE];
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, (void *)szErrorBuffer);

        //curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30l);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, long(net_httpConnectTimeout));

        // If we're below 50 bytes/sec for more than
        // 25 seconds, time out the request.
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 10l);
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 25l);

        if (m_iMode & FILE_HTTP_RESUME)
            curl_easy_setopt(curl, CURLOPT_RESUME_FROM, m_zResumePos);

        if (m_iMode & FILE_HTTP_UPLOAD)
        {
            // get our data from this function
            curl_easy_setopt(curl, CURLOPT_READFUNCTION, HTTP_ReadFileCallback);
            // we pass our file pointer to the callback function
            curl_easy_setopt(curl, CURLOPT_READDATA, (void *)(m_pFile));

            curl_easy_setopt(curl, CURLOPT_UPLOAD, 1l);
            curl_easy_setopt(curl, CURLOPT_INFILESIZE, m_pFile->GetBufferSize());
            curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1l);
            curl_easy_setopt(curl, CURLOPT_FTP_FILEMETHOD, CURLFTPMETHOD_MULTICWD);
            
            // disable EPSV (use PASV)
            curl_easy_setopt(curl, CURLOPT_FTP_USE_EPSV, 0l);
            
            curl_easy_setopt(curl, CURLOPT_FTP_RESPONSE_TIMEOUT, 5l);
            //curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 1l);
            //curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1l);

            if (m_iMode & FILE_FTP_ACTIVE)
                curl_easy_setopt(curl, CURLOPT_FTPPORT, "-");
            else
                curl_easy_setopt(curl, CURLOPT_FTPPORT, NULL);
            
            m_bUploaded = true;
        }
        else if (m_iMode & FILE_HTTP_WRITETOFILE)
        {
            // send all data to this function
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HTTP_WriteFileCallback);
            // we pass our file pointer to the callback function
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)(m_pFile));

            m_bOpen = true;
        }
        else
        {
            // send all data to this function
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HTTP_WriteMemoryCallback);
            // we pass our 'chunk' struct to the callback function
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, static_cast<void *>(&buf));

            m_bOpen = true;
        }


        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            Console.Net << _T("[cURL] ") << szErrorBuffer << newl;
            m_bError = true;
        }
        else
        {
            if (!(m_iMode & FILE_HTTP_UPLOAD) && !(m_iMode & FILE_HTTP_WRITETOFILE))
            {
                if (buf.size > 0)
                {
                    m_pBuffer = K2_NEW_ARRAY(ctx_Net, char, buf.size);
                    MemManager.Copy(m_pBuffer, buf.buf, buf.size);
                    m_uiSize = INT_SIZE(buf.size);
                }
                else
                    SAFE_DELETE_ARRAY(m_pBuffer);
            }

            m_bError = false;
        }

        curl_easy_cleanup(curl);

        Console.Dev << "HTTP request took " << K2System.Milliseconds() - start << "ms" << newl;
        return true;
    }
    else if (g_pcURLMulti != NULL)
    {
        CURL *curl = NULL;
        SHTTPRequest sRequest;

        HTTPRequestMap_it it(g_mapHTTPRequests.find(sPathSingle));
        if (it != g_mapHTTPRequests.end())
        {
            m_Progress = (*it).second.progress;
            m_iMode = (*it).second.iMode;
            m_uiSize = (*it).second.uiTotalSize;
            m_pFile = (*it).second.pFile;

            if ((*it).second.eResult != HTTP_REQUESTED)
            {
                if (!(m_iMode & FILE_HTTP_UPLOAD))
                    m_bOpen = true;
                else
                    m_bUploaded = true;
            }

            if ((*it).second.eResult == HTTP_FAILED)
                m_bError = true;
            else if ((*it).second.eResult == HTTP_REQUESTED) // still getting it
                return true; // not here yet

            if (!(*it).second.bBufIsFileHandle && (*it).second.buf.buf != NULL && (*it).second.buf.size > 0)
            {
                m_pBuffer = K2_NEW_ARRAY(ctx_Net, char, (*it).second.buf.size);
                MemManager.Copy(m_pBuffer, (*it).second.buf.buf, (*it).second.buf.size);
                SAFE_DELETE((*it).second.buf.buf);
                (*it).second.buf.size = 0;
            }

            SAFE_DELETE((*it).second.pURL);

            STL_ERASE(g_mapHTTPRequests, it);

            return true;
        }

        if ((m_iMode & FILE_HTTP_UPLOAD || m_iMode & FILE_HTTP_WRITETOFILE) && m_pFile == NULL)
            return false;

        sRequest.buf.buf = NULL;
        sRequest.buf.max_size = 0;
        sRequest.buf.size = 0;
        sRequest.uiTotalSize = 0;
        sRequest.iMode = m_iMode;
        sRequest.pFile = NULL;

        curl = curl_easy_init();
        if (!curl)
        {
            Console.Err << "Error!  curl has not been initialized yet!" << newl;
            return false;
        }
        
        // use IPv4
        curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

        sRequest.sUrl = sPathSingle;
        sRequest.handle = curl;

        sRequest.pURL = K2_NEW_ARRAY(ctx_Net, char, sRequest.sUrl.length()+1);
        STRCPY_S(sRequest.pURL, sRequest.sUrl.length() + 1, sRequest.sUrl.c_str());

        curl_easy_setopt(sRequest.handle, CURLOPT_URL, sRequest.pURL);

        sRequest.eResult = HTTP_REQUESTED;

        sRequest.bBufIsFileHandle = ((m_iMode & FILE_HTTP_WRITETOFILE) != 0);
        MemManager.Set(&sRequest.progress, 0, sizeof(SHTTPProgress));

        m_Progress = sRequest.progress;

        // If we're below 10 bytes/sec for more than
        // 25 seconds, time out the request.
        curl_easy_setopt(sRequest.handle, CURLOPT_LOW_SPEED_LIMIT, 10l);
        curl_easy_setopt(sRequest.handle, CURLOPT_LOW_SPEED_TIME, 25l);

        if (!(m_iMode & FILE_HTTP_GETSIZE))
        {
            // A blank string means allow all encoding options
            curl_easy_setopt(sRequest.handle, CURLOPT_ENCODING, "");

            if (m_iMode & FILE_HTTP_RESUME)
                curl_easy_setopt(sRequest.handle, CURLOPT_RESUME_FROM, m_zResumePos);
        }

        curl_easy_setopt(sRequest.handle, CURLOPT_FAILONERROR, 1l);

        if (m_iMode & FILE_HTTP_GETSIZE)
        {
            // only grab the header
            curl_easy_setopt(sRequest.handle, CURLOPT_NOBODY, 1l);
        }
        else if (m_iMode & FILE_HTTP_UPLOAD)
        {
            // get our data from this function
            curl_easy_setopt(sRequest.handle, CURLOPT_READFUNCTION, HTTP_ReadFileCallback);
            // we pass our file pointer to the callback function
            curl_easy_setopt(sRequest.handle, CURLOPT_READDATA, (void *)(m_pFile));

            curl_easy_setopt(sRequest.handle, CURLOPT_UPLOAD, 1l);
            curl_easy_setopt(sRequest.handle, CURLOPT_INFILESIZE, m_pFile->GetBufferSize());
            curl_easy_setopt(sRequest.handle, CURLOPT_FTP_CREATE_MISSING_DIRS, 2l);
            
            // disable EPSV (use PASV)
            curl_easy_setopt(curl, CURLOPT_FTP_USE_EPSV, 0l);

            sRequest.pFile = m_pFile;
        }
        else if (m_iMode & FILE_HTTP_WRITETOFILE)
        {
            /* send all data to this function  */
            curl_easy_setopt(sRequest.handle, CURLOPT_WRITEFUNCTION, HTTP_WriteFileCallback);
            /* we pass our file pointer to the callback function */
            curl_easy_setopt(sRequest.handle, CURLOPT_WRITEDATA, (void *)(m_pFile));
            

            sRequest.pFile = m_pFile;
        }
        else
        {
            /* send all data to this function  */
            curl_easy_setopt(sRequest.handle, CURLOPT_WRITEFUNCTION, HTTP_WriteMemoryCallback);
            /* we pass our 'chunk' struct to the callback function */
            curl_easy_setopt(sRequest.handle, CURLOPT_WRITEDATA, (void *)&sRequest.buf);
        }

        CURLMcode result = curl_multi_add_handle(g_pcURLMulti, sRequest.handle);

        if (result != CURLM_OK && result != CURLM_CALL_MULTI_PERFORM)
        {
            SAFE_DELETE(sRequest.pURL);
            return false;
        }

        g_mapHTTPRequests.insert(HTTPRequestEntry(sRequest.sUrl, sRequest));

        HTTPRequestMap_it findit = g_mapHTTPRequests.find(sRequest.sUrl);

        if (findit != g_mapHTTPRequests.end())
        {
            // Setup progress callback
            curl_easy_setopt((*findit).second.handle, CURLOPT_NOPROGRESS, 0l);
            curl_easy_setopt((*findit).second.handle, CURLOPT_PROGRESSDATA, &(*findit).second.progress);
            curl_easy_setopt((*findit).second.handle, CURLOPT_PROGRESSFUNCTION, HTTP_ProgressCallback);
        }
        
        return true;
    }

    return false;
}


/*====================
  CFileHTTP::StopTransfer
  ====================*/
void    CFileHTTP::StopTransfer(const tstring &sURL)
{
    HTTPRequestMap_it it;
    CURLMcode result;

    it = g_mapHTTPRequests.find(TStringToString(sURL));

    if (it != g_mapHTTPRequests.end())
    {
        if (it->second.pFile != NULL)
            it->second.pFile->Close();

        if ((*it).second.handle != NULL && g_pcURLMulti != NULL)
            result = curl_multi_remove_handle(g_pcURLMulti, (*it).second.handle);

        curl_easy_cleanup((*it).second.handle);

        SAFE_DELETE((*it).second.pURL);

        if (!(*it).second.bBufIsFileHandle && (*it).second.buf.buf != NULL)
            SAFE_DELETE((*it).second.buf.buf);

        STL_ERASE(g_mapHTTPRequests, it);
    }
}


/*====================
  CFileHTTP::Close
  ====================*/
void    CFileHTTP::Close()
{
    if (m_pBuffer)
        K2_DELETE_ARRAY(m_pBuffer);
}


/*====================
  CFileHTTP::ReadLine
  ====================*/
tstring CFileHTTP::ReadLine()
{
    if (m_iMode & FILE_WRITE)
    {
        Console.Warn << _T("Cannot read from WRITE file ") << m_sPath << newl;
        return 0;
    }

    if (m_iMode & FILE_BINARY)
    {
        Console.Warn << _T("Cannot ReadLine from BINARY file ") << m_sPath << newl;
        return 0;
    }

    if (m_bEOF)
        return 0;

    tstring sReturn;
    do
    {
        TCHAR c = *(m_pBuffer + m_uiPos);

        // look for any line break character
        if (c == '\n' || c == '\r')
        {
            // move the position marker to the start of the next line
            // we want to any consecutivce line break characters until we reach
            // more text, or another duplicate of the character that initiated
            // the break (a blank line).  This should handle any conceivable
            // text file format
            do
            {
                ++m_uiPos;
            }
            while ((*(m_pBuffer + m_uiPos) == '\n' || *(m_pBuffer + m_uiPos) == '\r') && *(m_pBuffer + m_uiPos) != c);
            break;
        }
        else
        {
            sReturn += c;
        }

        ++m_uiPos;

        if (m_uiPos == m_uiSize)
        {
            m_bEOF = true;
            break;
        }
    }
    while (true);

    return sReturn;
}


/*====================
  CFileHTTP::Read
  ====================*/
uint    CFileHTTP::Read(char* pBuffer, uint uiBufferSize) const
{
    if (!m_bOpen)
        return 0;

    if (m_iMode & FILE_WRITE)
    {
        Console.Warn << _T("Cannot read from WRITE file ") << m_sPath << newl;
        return 0;
    }

    // Check for a buffer
    if (m_pBuffer)
    {
        // read from the buffer, and advance the position
        uint uiSize(uiBufferSize);

        if (m_uiSize - m_uiPos < uiBufferSize)
        {
            uiSize = m_uiSize - m_uiPos;
            m_bEOF = true;
        }

        MemManager.Copy(pBuffer, m_pBuffer + m_uiPos, uiSize);
        m_uiPos += uiSize;
        return uiSize;
    }

    return 0;
}


/*====================
  CFileHTTP::Write
  ====================*/
size_t  CFileHTTP::Write(const void *pBuffer, size_t iBufferSize)
{
    // can't write to this type of file
    return 0;
}


/*====================
  CFileHTTP::GetBuffer
  ====================*/
const char* CFileHTTP::GetBuffer(uint &uiSize)
{
    uiSize = m_uiSize;
    return m_pBuffer;
}


/*====================
  CFileHTTP::SetPostData
  ====================*/
void    CFileHTTP::SetPostData(const string &sData)
{
    size_t zLength((sData.length() + 1));
    m_szPostData = K2_NEW_ARRAY(ctx_Net, char, zLength);
    STRNCPY_S(m_szPostData, zLength, sData.c_str(), sData.length());
}


/*====================
  CFileHTTP::IsInProgress
  ====================*/
bool    CFileHTTP::IsInProgress(const tstring &sURL)
{
    return (g_mapHTTPRequests.find(TStringToString(sURL)) != g_mapHTTPRequests.end());
}
