// (C)2010 S2 Games
// c_httprequest.h
//
//=============================================================================
#ifndef __C_HTTPREQUEST_H__
#define __C_HTTPREQUEST_H__

//=============================================================================
// Declarations
//=============================================================================
class CHTTPManager;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EHTTPPostStatus
{
	HTTP_REQUEST_IDLE,
	HTTP_REQUEST_SENDING,
	HTTP_REQUEST_SUCCESS,
	HTTP_REQUEST_ERROR
};
//=============================================================================

//=============================================================================
// CHTTPRequest
//=============================================================================
class CHTTPRequest
{
private:
	typedef pair<string, string>		StringPair;
	typedef vector<StringPair>			StringPairVector;
	typedef StringPairVector::iterator	StringPairVector_it;
	
	CHTTPManager*			m_pHTTPManager;

	uint					m_uiID;
	void*					m_pCurlEasy;
	
	string					m_sURL;
	char*					m_pErrorBuffer;
	CBufferDynamic			m_bufferResponse;
	EHTTPPostStatus			m_eStatus;
	StringPairVector		m_vVariables;
	wstring					m_sResponse;

	bool					m_bReleaseOnCompletion;
	uint					m_uiTimeStamp;
	uint					m_uiTimeout;
	uint					m_uiConnectTimeout;
	uint					m_uiLowSpeedLimit;
	uint					m_uiLowSpeedTime;

	// This 'type' is provided for the utility of whatever is making use of the request
	// and has no effect on the request itself
	uint					m_uiType;

	K2_API void	SendRequest(const string &sURL, bool bPost, bool bSSL);

	CHTTPRequest();

public:
	~CHTTPRequest();
	CHTTPRequest(CHTTPManager *pHTTPManager, void *pCurlEasy, uint uiID);

	inline uint		GetID() const												{ return m_uiID; }
	inline void*	GetCURL() const												{ return m_pCurlEasy; }

	inline uint		GetType() const												{ return m_uiType; }
	inline void		SetType(uint uiType)										{ m_uiType = uiType; }

	void			Completed();
	void			Failed();

	void			SetTimeStamp(uint uiTimeStamp)								{ m_uiTimeStamp = uiTimeStamp; }
	uint			GetTimeStamp() const										{ return m_uiTimeStamp; }

	inline void		SetTargetURL(const wstring &sURL)							{ m_sURL = WideToSingle(sURL); }
	inline void		SetTargetURL(const string &sURL)							{ m_sURL = sURL; }

	inline void		SetReleaseOnCompletion(bool b)								{ m_bReleaseOnCompletion = b; }
	inline bool		GetReleaseOnCompletion() const								{ return m_bReleaseOnCompletion; }

	inline void		ClearVariables()											{ m_vVariables.clear(); }
	inline void		AddVariable(const wstring &sName, const string &sValue)		{ m_vVariables.push_back(StringPair(URLEncode(sName), URLEncode(sValue))); }
	inline void		AddVariable(const wstring &sName, const wstring &sValue)	{ m_vVariables.push_back(StringPair(URLEncode(sName), URLEncode(WStringToUTF8(sValue)))); }
	inline void		AddVariable(const wstring &sName, uint uiVal)				{ m_vVariables.push_back(StringPair(URLEncode(sName), URLEncode(XtoS(uiVal)))); }
	inline void		AddVariable(const wstring &sName, int iVal)					{ m_vVariables.push_back(StringPair(URLEncode(sName), URLEncode(XtoS(iVal)))); }
	inline void		AddVariable(const wstring &sName, float fVal)				{ m_vVariables.push_back(StringPair(URLEncode(sName), URLEncode(XtoS(fVal)))); }

	inline void		SetTimeout(uint uiSeconds)									{ m_uiTimeout = uiSeconds; }
	inline void		SetConnectTimeout(uint uiSeconds)							{ m_uiConnectTimeout = uiSeconds; }
	inline void		SetLowSpeedTimeout(uint uiRate, uint uiSeconds)				{ m_uiLowSpeedLimit = uiRate; m_uiLowSpeedTime = uiSeconds; }
	
	inline void		SendSecurePostRequest()										{ return SendRequest("https://" + m_sURL, true, true); }
	inline void		SendPostRequest()											{ return SendRequest("http://" + m_sURL, true, false); }
	inline void		SendSecureRequest()											{ return SendRequest("https://" + m_sURL, false, true); }
	inline void		SendRequest()												{ return SendRequest("http://" + m_sURL, false, false); }
	void			Reset();

	K2_API void		Wait();

	inline uint				GetNumVariables()									{ return INT_SIZE(m_vVariables.size()); }
	inline const wstring&	GetResponse() const									{ return m_sResponse; }
	inline EHTTPPostStatus	GetStatus() const									{ return m_eStatus; }
	inline wstring			GetErrorBuffer() const								{ return m_pErrorBuffer ? SingleToWide(m_pErrorBuffer) : WSNULL; }

	inline bool		IsActive() const											{ return m_eStatus == HTTP_REQUEST_SENDING; }
	inline bool		WasSuccessful() const										{ return m_eStatus == HTTP_REQUEST_SUCCESS; }
	inline bool		IsFailed() const											{ return m_eStatus == HTTP_REQUEST_ERROR; }
};
//=============================================================================

#endif //__C_HTTPREQUEST_H__
