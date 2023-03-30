// (C)2005 S2 Games
// c_filehttp.h
//
// CFileHTTP
// Provides access to a url as a file over http
//=============================================================================
#ifndef __C_FILEHTTP__
#define __C_FILEHTTP__

//=============================================================================
// Headers
//=============================================================================
#include "c_file.h"

typedef void CURL;
typedef void CURLM;

void    cURL_Initialize();
void    cURL_Shutdown();
int     cURL_Frame();
//=============================================================================

const int MAX_HTTP_REQUESTS  = 20;

enum httpResult_e
{
    HTTP_REQUESTED = 0,
    HTTP_FAILED,
    HTTP_SUCCESS
};

struct SByteBuffer
{
    byte    *buf;
    size_t  max_size;
    size_t  size;
};

struct SHTTPProgress
{
    float           fPercent;
    double          dSize;
    double          dDownloaded;
};

struct SHTTPRequest
{
    httpResult_e    eResult;
    CURL*           handle;
    string          sUrl;
    SByteBuffer     buf;
    bool            bBufIsFileHandle;
    SHTTPProgress   progress;
    int             iMode;
    uint            uiTotalSize;
    char*           pURL;
    CFile*          pFile;
};

typedef map<string, SHTTPRequest>       HTTPRequestMap;
typedef pair<string, SHTTPRequest>      HTTPRequestEntry;
typedef HTTPRequestMap::iterator        HTTPRequestMap_it;
//=============================================================================
// CFileHTTP
//=============================================================================
class CFileHTTP : public CFile
{
private:
    bool    m_bOpen;
    bool    m_bUploaded;
    bool    m_bError;

    CFile*  m_pFile;
    size_t  m_zResumePos;

    char*   m_szPostData;

    SHTTPProgress   m_Progress;

public:
    K2_API  CFileHTTP();
    K2_API  ~CFileHTTP();

    K2_API  CFile*  GetFileTarget()                         { return m_pFile; }

    K2_API  void    SetFileTarget(CFile *pFile)             { m_pFile = pFile; }
    K2_API  void    SetFileTarget(CFileHandle &file)        { m_pFile = file.m_pFile; }
    K2_API  void    SetResumePos(size_t zOffset)            { m_zResumePos = zOffset; }
    K2_API  void    SetPostData(const string &sData);

    K2_API  bool    IsInProgress(const tstring &sURL);
    K2_API  bool    Open(const tstring &sURL, int iMode);
    K2_API  void    StopTransfer(const tstring &sURL);
    K2_API  void    Close();
    K2_API  bool    IsOpen() const                          { return m_bOpen; }
    K2_API  bool    DoneUpload() const                      { return m_bUploaded; }
    K2_API  bool    ErrorEncountered() const                { return m_bError; }

    K2_API  double  GetHTTPSize(const string &sPath);
    K2_API  SHTTPProgress   GetProgress()                   { return m_Progress; }
    K2_API  float   GetProgressPercent()                    { return m_Progress.fPercent; }

    K2_API  tstring ReadLine();
    K2_API  uint    Read(char *pBuffer, uint uiBufferSize) const;
    K2_API  size_t  Write(const void *pBuffer, size_t iBufferSize);

    K2_API  const char* GetBuffer(uint &uiSize);

    K2_API  bool    WriteByte(char c)                           { return false; }
    K2_API  bool    WriteInt16(short c, bool bUseBigEndian)     { return false; }
    K2_API  bool    WriteInt32(int c, bool bUseBigEndian)       { return false; }
    K2_API  bool    WriteInt64(LONGLONG c, bool bUseBigEndian)  { return false; }
    K2_API  bool    WriteString(const string &sText)            { return false; }
    K2_API  bool    WriteString(const wstring &sText)           { return false; }
};
#endif
//=============================================================================
