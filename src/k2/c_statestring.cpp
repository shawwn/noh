// (C)2005 S2 Games
// c_statestring.pp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_statestring.h"
#include "c_buffer.h"
#include "c_eventmanager.h"
//=============================================================================

/*====================
  CStateString::CStateString
  ====================*/
CStateString::CStateString() :
m_iModifiedCount(0)
{
}

CStateString::CStateString(const IBuffer &buffer) :
m_iModifiedCount(0)
{
	Set(buffer);
}


/*====================
  CStateString::GetValue
  ====================*/
tstring	CStateString::GetValue(const tstring &sState) const
{
	smaps_cit it(m_mapProperties.find(TStringToUTF8(sState)));
	if (it == m_mapProperties.end())
		return TSNULL;

	return UTF8ToTString(it->second);
}


/*====================
  CStateString::HasState
  ====================*/
bool	CStateString::HasState(const tstring &sState) const
{
	smaps_cit it(m_mapProperties.find(TStringToUTF8(sState)));
	if (it == m_mapProperties.end())
		return false;

	return true;
}


/*====================
  CStateString::ForEachState
  ====================*/
void	CStateString::ForEachState(StateStringCallBack pCallBack, bool bLoadJob) const
{
	if (pCallBack == NULL)
		return;

	if (Host.IsInGame() || !bLoadJob)
	{
		for (smaps_cit it(m_mapProperties.begin()); it != m_mapProperties.end(); ++it)
			pCallBack(it->first, it->second);
	}
	else
	{
		class CReadStateStringFunctions : public CLoadJob<smaps>::IFunctions
		{
		private:
			StateStringCallBack m_pCallBack;

		public:
			CReadStateStringFunctions(StateStringCallBack pCallBack) :
			m_pCallBack(pCallBack)
			{}

			float	Frame(smaps_it &it, float f) const
			{
				//SetTitle(_T("Reading state string update"));
				SetProgress(f);
				return 0.0f;
			}
			float	PostFrame(smaps_it &it, float f) const	{ m_pCallBack(it->first, it->second); ++it; return 1.0f; }
		};
		CReadStateStringFunctions fnReadStateString(pCallBack);
		smaps mapProperties(m_mapProperties);
		CLoadJob<smaps>	job(mapProperties, &fnReadStateString, LOADING_DISPLAY_INTERFACE);
		IModalDialog::Show(_T("loading"));
		job.Execute(m_mapProperties.size());
	}
}


/*====================
  CStateString::Validate
  ====================*/
void	CStateString::Validate()
{
	PROFILE("CStateString::Validate");

	smaps_it it(m_mapProperties.begin());
	smaps_it itEnd(m_mapProperties.end());

	uint ui(0);
	uint uiNumProperties(uint(m_mapProperties.size()));

	while (it != itEnd)
	{
		if (ui >= uiNumProperties)
		{
			K2System.Error(_T("CStateString::GetDifference: m_mapProperties corrupted"));
			break;
		}

		++ui;
		++it;
	}
}


/*====================
  CStateString::GetDifference
  ====================*/
void	CStateString::GetDifference(CStateString &ss)
{
	PROFILE("CStateString::GetDifference");

	smaps &mapDiff(ss.GetPropertyMap());

	smaps_it it(m_mapProperties.begin());
	smaps_it itEnd(m_mapProperties.end());

	uint ui(0);
	uint uiNumProperties(uint(m_mapProperties.size()));

	while (it != itEnd)
	{
		if (ui >= uiNumProperties)
		{
			K2System.Error(_T("CStateString::GetDifference: m_mapProperties corrupted"));
			break;
		}

		smaps_it itFind(mapDiff.find(it->first));

		if (itFind != mapDiff.end() && itFind->second == it->second)
			mapDiff.erase(itFind);

		++ui;
		++it;
	}
}


/*====================
  CStateString::Modify
  ====================*/
void	CStateString::Modify(const IBuffer &buffer)
{
	string sKey;
	string sValue;
	bool bReadingKey(true);

	uint uiReadPos(0);
	const char *pBuffer(buffer.Get());
	
	while (uiReadPos < buffer.GetLength())
	{
		char c(pBuffer[uiReadPos]);
		++uiReadPos;

		if (c == STATE_STRING_SEPERATOR)
		{
			if (bReadingKey)
			{
				bReadingKey = false;
				continue;
			}

			m_mapProperties[sKey] = sValue;
			bReadingKey = true;
			sKey.clear();
			sValue.clear();
			continue;
		}

		if (bReadingKey)
			sKey += c;
		else
			sValue += c;
	}
}


/*====================
  CStateString::Set
  ====================*/
void	CStateString::Set(const string &sBuffer)
{
	m_mapProperties.clear();

	string sKey;
	string sValue;
	bool bReadingKey(true);
	
	for (string::const_iterator cit(sBuffer.begin()); cit != sBuffer.end(); ++cit)
	{
		char c(*cit);

		if (c == STATE_STRING_SEPERATOR)
		{
			if (bReadingKey)
			{
				bReadingKey = false;
				continue;
			}

			m_mapProperties[sKey] = sValue;
			bReadingKey = true;
			sKey.clear();
			sValue.clear();
			continue;
		}

		if (bReadingKey)
			sKey += c;
		else
			sValue += c;
	}
}


/*====================
  CStateString::AppendToBuffer
  ====================*/
void	CStateString::AppendToBuffer(IBuffer &buffer) const
{
	for (smaps_cit it(m_mapProperties.begin()); it != m_mapProperties.end(); ++it)
		buffer << it->first << STATE_STRING_SEPERATOR << it->second << STATE_STRING_SEPERATOR;
}

void	CStateString::AppendToBuffer(string &sBuffer) const
{
	for (smaps_cit it(m_mapProperties.begin()); it != m_mapProperties.end(); ++it)
	{
		sBuffer += it->first;
		sBuffer += STATE_STRING_SEPERATOR;
		sBuffer += it->second;
		sBuffer += STATE_STRING_SEPERATOR;
	}
}
