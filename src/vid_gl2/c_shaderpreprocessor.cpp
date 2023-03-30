// (C)2005 S2 Games
// c_shaderpreprocessor.cpp
//
// A nice wrapper class for D3DXMACROs
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_shaderpreprocessor.h"

#include "../k2/c_cmd.h"
#include "../k2/evaluator.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CVAR_BOOL(vid_debugShaderPreprocessor, false);

CShaderPreprocessor		g_ShaderPreprocessor;
//=============================================================================

/*====================
  CShaderPreprocessor::CShaderPreprocessor
  ====================*/
CShaderPreprocessor::CShaderPreprocessor()
{
}


/*====================
  CShaderPreprocessor::~CShaderPreprocessor
  ====================*/
CShaderPreprocessor::~CShaderPreprocessor()
{
}


/*====================
  CShaderPreprocessor::Define
  ====================*/
void	CShaderPreprocessor::Define(const string &sName, const string &sDefinition)
{
	m_mapDefinitions[sName] = sDefinition;
}


/*====================
  CShaderPreprocessor::Undefine
  ====================*/
void	CShaderPreprocessor::Undefine(const string &sName)
{
	m_mapDefinitions.erase(sName);
}


/*====================
  CShaderPreprocessor::GetDefinition
  ====================*/
string	CShaderPreprocessor::GetDefinition(const string &sName)
{
	DefinitionMap::iterator findit(m_mapDefinitions.find(sName));

	if (findit != m_mapDefinitions.end())
		return findit->second;
	else
		return "";
}


/*====================
  CShaderPreprocessor::UndefineAll
  ====================*/
void	CShaderPreprocessor::UndefineAll()
{
	m_mapDefinitions.clear();
}


/*====================
  CShaderPreprocessor::AllocMacroArray

  Free this value please when you're done using it!
  ====================*/
const char*	CShaderPreprocessor::AllocMacroArray()
{
	string sMacros;

	if (g_DeviceCaps.bNonSquareMatrix)
		sMacros += "#version 120\n";
	else
		sMacros += "#version 110\n";

	for (DefinitionMap::iterator it(m_mapDefinitions.begin()); it != m_mapDefinitions.end(); ++it)
	{
		if (it->first.empty())
			continue;

		sMacros += "#define ";
		sMacros += it->first;

		if (!it->second.empty())
		{
			sMacros += " ";
			sMacros += it->second;
		}

		sMacros += "\n";

		if (vid_debugShaderPreprocessor)
		{
			if (it->second.empty())
				Console.Dev << QuoteStr(StringToTString(it->first)) << _T(";");
			else
				Console.Dev << QuoteStr(StringToTString(it->first)) << _T("=") << QuoteStr(StringToTString(it->second)) << _T(";");
		}
	}

	if (vid_debugShaderPreprocessor && m_mapDefinitions.size() > 0)
		Console.Dev << newl;

	char *szMacros = K2_NEW_ARRAY(ctx_GL2, char, sMacros.size()+1);
	STRCPY_S(szMacros, sMacros.size() + 1, sMacros.c_str());

	return szMacros;
}


/*====================
  CShaderPreprocessor::GetDefinitionHeaderString
  ====================*/
string	CShaderPreprocessor::GetDefinitionHeaderString()
{
	string s;

	s += "// Defines: ";
	s += "\n";
	s += "//";
	s += "\n";

	for (DefinitionMap::iterator it(m_mapDefinitions.begin()); it != m_mapDefinitions.end(); ++it)
	{
		if (it->first.empty())
			continue;

		s += "//   ";

		s += it->first;

		if (!it->second.empty())
		{
			s += "=";
			s += it->second;
		}

		s += "\n";
	}

	return s;
}


/*====================
  CShaderPreprocessor::GetDefinitionString
  ====================*/
string	CShaderPreprocessor::GetDefinitionString()
{
	string s;

	for (DefinitionMap::iterator it(m_mapDefinitions.begin()); it != m_mapDefinitions.end(); ++it)
	{
		if (it->first.empty())
			continue;

		if (it->second.empty())
			s += it->first + ";";
		else
			s += it->first + "='" + it->second + "';";
	}

	return s;
}


/*--------------------
  cmdShaderDefine
  --------------------*/
CMD(ShaderDefine)
{
	if (vArgList.size() < 1)
		return false;

	g_ShaderPreprocessor.Define(TStringToString(vArgList[0]), vArgList.size() > 1 ? TStringToString(vArgList[1]) : "");
	return true;
}


/*--------------------
  cmdShaderUndefine
  --------------------*/
CMD(ShaderUndefine)
{
	if (vArgList.size() < 1)
		return false;

	g_ShaderPreprocessor.Undefine(TStringToString(vArgList[0]));
	return true;
}
