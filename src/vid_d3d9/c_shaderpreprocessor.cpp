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

#include "c_shadercache.h"

#include "../k2/c_cmd.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CVAR_BOOL(vid_debugShaderPreprocessor, false);

CShaderPreprocessor     g_ShaderPreprocessor;
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
void    CShaderPreprocessor::Define(const string &sName, const string &sDefinition)
{
    m_mapDefinitions[sName] = sDefinition;
}


/*====================
  CShaderPreprocessor::Undefine
  ====================*/
void    CShaderPreprocessor::Undefine(const string &sName)
{
    m_mapDefinitions.erase(sName);
}


/*====================
  CShaderPreprocessor::GetDefinition
  ====================*/
string  CShaderPreprocessor::GetDefinition(const string &sName)
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
void    CShaderPreprocessor::UndefineAll()
{
    m_mapDefinitions.clear();
}


/*====================
  CShaderPreprocessor::AllocMacroArray

  Free this value please when you're done using it!
  ====================*/
D3DXMACRO*  CShaderPreprocessor::AllocMacroArray()
{
    D3DXMACRO   *pMacros = K2_NEW_ARRAY(ctx_D3D9, D3DXMACRO, m_mapDefinitions.size()+1);

    int q = 0;
    for (DefinitionMap::iterator it(m_mapDefinitions.begin()); it != m_mapDefinitions.end(); ++it)
    {
        pMacros[q].Name = it->first.c_str();
        pMacros[q].Definition = it->second.c_str();
        ++q;

        if (vid_debugShaderPreprocessor && !it->first.empty())
        {
            if (it->second.empty())
                Console.Dev << QuoteStr(StringToTString(it->first)) << _T(";");
            else
                Console.Dev << QuoteStr(StringToTString(it->first)) << _T("=") << QuoteStr(StringToTString(it->second)) << _T(";");
        }
    }

    if (vid_debugShaderPreprocessor && m_mapDefinitions.size() > 0)
        Console.Dev << newl;

    pMacros[q].Name = pMacros[q].Definition = NULL;

    return pMacros;
}


/*====================
  CShaderPreprocessor::GetDefinitionHeaderString
  ====================*/
string  CShaderPreprocessor::GetDefinitionHeaderString()
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
string  CShaderPreprocessor::GetDefinitionString()
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
