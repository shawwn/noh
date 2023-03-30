// (C)2009 S2 Games
// c_glslpreprocessor.h
//
//=============================================================================
#ifndef __C_GLSLPREPROCESSOR_H__
#define __C_GLSLPREPROCESSOR_H__

//=============================================================================
// CGLBranchTracker
//=============================================================================
class CBranchTracker
{
private:
	vector<unsigned char> m_vCondition;
	// first 3 bits used:
	//   bit0: parent active
	//   bit1: this branch active
	//   bit2: active block for this branch hit (needed for elif)
	
public:
	void Reset()
	{
		m_vCondition.clear();
	}

	bool Active()
	{
		if (m_vCondition.empty())
			return true;
		return (m_vCondition.back() & 0x3) == 0x3;
	}
	
	void If(bool bCondition)
	{
		m_vCondition.push_back(Active() ? 0x1 : 0x0);
		if (bCondition)
			m_vCondition.back() |= 0x6;
	}
	
	void Elif(bool bCondition)
	{
		if (m_vCondition.empty())
			EX_ERROR(_T("#elif without #if"));
		if (m_vCondition.back() & 0x4)
			m_vCondition.back() &= 0x5;
		else if (bCondition)
			m_vCondition.back() |= 0x6;
	}
	
	void Else()
	{
		if (m_vCondition.empty())
			EX_ERROR(_T("#else without #if"));
		if (m_vCondition.back() & 0x4)
			m_vCondition.back() &= 0x5;
		else
			m_vCondition.back() |= 0x6;
	}
	
	void Endif()
	{
		if (m_vCondition.empty())
			EX_ERROR(_T("#endif without #if"));
		m_vCondition.pop_back();
	}
};

//=============================================================================
// CGLSLPreprocessor
//=============================================================================
class CGLSLPreprocessor
{
private:
	CBranchTracker		m_cBT;
	map<string,string>	m_mapDefines;
	string				m_sProcessedSource;
	int					m_iLineNum;
	vector<int>			m_viValue;
	vector<char>		m_vcOp;
	
	string GetIdentifier(const string &sLine, string::size_type &zPos);
	string DoSubstitutions(const string &sLine, string::size_type zPos = 0, bool bMissingIsError = false);
	void PerformOperation(char cOp);
	void AddOp(char cOp);
	bool Evaluate(const string &sExpression);
	
	void ProcessDirective(const string &sLine);
	void ProcessLine(const string &sLine);
	
public:
	void AddSource(const char *source);
	char* AllocSourceArray();
};
//=============================================================================

#endif
