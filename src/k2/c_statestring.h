// (C)2005 S2 Games
// c_statestring.h
//
//=============================================================================
#ifndef __C_STATESTRING_H__
#define __C_STATESTRING_H__

//=============================================================================
// Definitions
//=============================================================================
const char STATE_STRING_SEPERATOR(_T('\xff'));

typedef void (*StateStringCallBack)(const string &sStateUTF8, const string &sValueUTF8);

inline void	PrintStateAndValue(const string &sState, const string &sValue)
{
	Console << sCyan << UTF8ToTString(sState) << sNoColor << _T(": ") << sGreen << UTF8ToTString(sValue) << newl;
}
//=============================================================================

//=============================================================================
// CStateString
//=============================================================================
class CStateString
{
private:
	smaps	m_mapProperties;
	int		m_iModifiedCount;

	K2_API tstring	GetValue(const tstring &sState) const;

protected:
	smaps&			GetPropertyMap()								{ return m_mapProperties; }

public:
	~CStateString()	{}
	K2_API CStateString();
	CStateString(const IBuffer &buffer);

	void					Clear()									{ m_mapProperties.clear(); m_iModifiedCount = 0; }

	bool					IsEmpty() const							{ return m_mapProperties.empty(); }
	bool					HasState(const tstring &sState) const;

	K2_API void				ForEachState(StateStringCallBack pCallBack, bool bLoadJob) const;

	int						GetInt(const tstring &sState) const		{ return AtoI(GetValue(sState)); }
	float					GetFloat(const tstring &sState) const	{ return AtoF(GetValue(sState)); }
	bool					GetBool(const tstring &sState) const	{ return AtoB(GetValue(sState)); }
	tstring					GetString(const tstring &sState) const	{ return GetValue(sState); }
	
	inline void				Set(const IBuffer &buffer)				{ m_mapProperties.clear(); Modify(buffer); }
	K2_API void				Set(const string &sBuffer);

	void					Modify(const IBuffer &buffer);

	K2_API void				AppendToBuffer(IBuffer &buffer) const;
	K2_API void				AppendToBuffer(string &sBuffer) const;

	K2_API void				GetDifference(CStateString &ss);
	K2_API void				Validate();

	void					Print()									{ ForEachState(PrintStateAndValue, false); }

	template<class T>
	inline
	bool	Set(const tstring &sState, const T &_value)
	{
		string sValue(XtoS(_value));
		if (sState.find(STATE_STRING_SEPERATOR) != tstring::npos ||
			sValue.find(STATE_STRING_SEPERATOR) != tstring::npos)
		{
			Console.Warn << _T("CStateString::Set() failed because the state or value string contained a reserved character") << newl
				<< _T("State: ") << sState << _T(" Value: ") << sValue << newl;
			return false;
		}

		// Append the state
		m_mapProperties[TStringToUTF8(sState)] = sValue;
		return true;
	}
};

template<>
inline
bool	CStateString::Set<tstring>(const tstring &sState, const tstring &sValue)
{
	if (sState.find(STATE_STRING_SEPERATOR) != tstring::npos ||
		sValue.find(STATE_STRING_SEPERATOR) != tstring::npos)
	{
		Console.Warn << _T("CStateString::Set() failed because the state or value string contained a reserved character") << newl
			<< _T("State: ") << sState << _T(" Value: ") << sValue << newl;
		return false;
	}

	// Append the state
	m_mapProperties[TStringToUTF8(sState)] = TStringToUTF8(sValue);
	return true;
}
//=============================================================================
#endif //__C_STATESTRING_H__
